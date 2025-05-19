#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string>
#include <vector>

#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 6387)

class IniReader
{
public:
	class IniSection
	{
	private:
		IniReader* m_parent;
		std::string m_szSection;
	public:
		class IniKey
		{
		private:
			IniSection* m_parent;
			std::string m_szKey;
			std::string m_value; // from string to another typename
		public:

			IniKey()
			{
				m_parent = nullptr;
			}
			
			~IniKey()
			{
				m_parent = nullptr;
			}

			IniKey(const char* szKey, IniSection* parent) : m_szKey(szKey), m_parent(parent) {}

			const char* getKeyName() { return m_szKey.c_str(); }
			std::string& getValue() { return m_value; }

			bool empty() { return m_value.empty() && m_parent == nullptr; }

			// Input operators

			/* Unstable at this point, use full functions instead
			IniKey& operator=(const char* value) { m_parent->m_parent->WriteString(m_parent->getSectionName(), getKeyName(), value); return *this; }
			IniKey& operator=(int value) { m_parent->m_parent->WriteInt(m_parent->getSectionName(), getKeyName(), value); return *this; }
			IniKey& operator=(float value) { m_parent->m_parent->WriteFloat(m_parent->getSectionName(), getKeyName(), value); return *this; }
			IniKey& operator=(bool value) { m_parent->m_parent->WriteBool(m_parent->getSectionName(), getKeyName(), value); return *this; }
			*/

			// Output operators

			operator const char* () { return m_value.c_str(); }
			operator int() { return atoi(m_value.c_str()); }
			operator float() { return (float)atof(m_value.c_str()); }
			operator bool() { return !stricmp(m_value.c_str(),  "true"); }
		};

		IniSection()
		{
			m_parent = nullptr;
		}

		~IniSection()
		{
			m_parent = nullptr;
			m_keys.clear();
		}

		IniSection(const char* szSection, IniReader* parent) : m_szSection(szSection), m_parent(parent) {}
	private:
		std::vector<IniKey> m_keys;
	public:
		IniKey* get(const char* szKey)
		{
			for (IniKey& key : m_keys)
			{
				if (!strcmp(key.getKeyName(), szKey))
					return &key;
			}

			return nullptr;
		}

		IniKey* add(const char* szKey)
		{
			if (IniKey* existing = get(szKey); existing->empty())
			{
				m_keys.push_back(IniKey(szKey, this)); // Don't add any more of keys
				return &m_keys.back();
			}
			else
			{
				return existing;
			}
		}

		IniKey* add(const char* szKey, std::string value)
		{
			if (IniKey *existingKey = get(szKey); existingKey->empty())
			{
				IniKey key(szKey, this);

				key.getValue() = value;

				m_keys.push_back(key);

				return &m_keys.back();
			}
			else
			{
				existingKey->getValue() = value;

				return existingKey;
			}
		}

		std::vector<IniKey>& getKeys() { return m_keys; }

		IniKey& operator[](const char* szKey)
		{
			IniKey dummy;

			for (IniKey& key : m_keys)
			{
				if (!strcmp(key.getKeyName(), szKey))
					return key;
			}

			return dummy;
		}

		bool empty() { return m_szSection.empty() && m_parent == nullptr; }

		const char* getSectionName() { return m_szSection.c_str(); }

		IniKey* begin() { return m_keys.data(); }
		const IniKey* begin() const { return m_keys.data(); }

		IniKey* end() { return m_keys.data() + m_keys.size(); }
		const IniKey* end() const { return m_keys.data() + m_keys.size(); }
	};
private:
	static inline void dummy() {};

	char* m_FilePath = nullptr; // Avoid stack based values(which will literally crash on path set)
	std::vector<IniSection> m_sections;
public:
	void SetIniPath(const char* filename)
	{
		char* str = (char*)_alloca(strlen(filename) + 16); // _malloca should be freed by _freea, so no thanks, I won't use it
		if (str)
			strcpy(str, filename);

		if (m_FilePath)
		{
			free(m_FilePath);
			m_FilePath = nullptr;
		}

		if (!strcmp(filename, "") && str)
		{
			strcpy(str, "default.ini");
		}
		else
		{
			if (const char* ext = strrchr(filename, '.'); !ext && str)
				strcat(str, ".ini");
		}

		char buff[MAX_PATH];
		HMODULE hm = NULL;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&dummy, &hm);
		GetModuleFileNameA(hm, buff, sizeof(buff));

		char* ptr = strrchr(buff, '\\'); // remove the executeable name from the path

		if (ptr)
			*ptr = '\0';

		if (strchr(filename, ':'))
		{
			SetPath(filename);
			return;
		}

		m_FilePath = (char*)malloc(strlen(buff) + strlen(str) + 2);
		if (m_FilePath)
			sprintf(m_FilePath, "%s\\%s", buff, str);
	}

	IniReader()
	{
		m_FilePath = nullptr;
	}

	IniReader(const char* fileName)
	{
		SetIniPath(fileName);

		Parse();
	}

	IniReader(std::string fileName)
	{
		SetIniPath(fileName.c_str());

		Parse();
	}

	~IniReader()
	{
		if (m_FilePath)
		{
			free(m_FilePath);
			m_FilePath = nullptr;
		}
		m_sections.clear();
	}

	void Parse()
	{
		FILE* iniFile = fopen(m_FilePath, "r");

		if (!iniFile)
			return;

		IniSection* currentSection = nullptr;
		char line[1024];
		bool inSection = false;

		while (fgets(line, sizeof(line), iniFile)) 
		{
			char* start = line;
			while (*start == ' ' || *start == '\t')
				start++;

			char* end = start + strlen(start) - 1;
			while ((end > start) && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
			{
				*end = '\0';
				end--;
			}

			if (*start == '\0' || *start == ';' || *start == '#' || !strncmp(start, "//", 2))
				continue;

			if (*start == '[') 
			{
				end = strchr(start, ']');
				if (end != nullptr) 
				{
					*end = '\0';
					m_sections.push_back(IniSection(start + 1, this));
					currentSection = &m_sections.back();
				}
				continue;
			}

			char* delimiter = strchr(start, '=');
			if (delimiter) 
			{
				*delimiter = '\0';
				char* key = start;
				char* value = delimiter + 1;

				while (*key == ' ' || *key == '\t') key++;
				while (*value == ' ' || *value == '\t' || *value == '"') value++;

				if (char* chr = strrchr(value, '"'); chr)
					*chr = 0;

				while (*(key + strlen(key) - 1) == ' ') *(key + strlen(key) - 1) = '\0';

				IniSection::IniKey kkey(key, currentSection);
				kkey.getValue() = value;

				currentSection->getKeys().push_back(kkey);
			}
		}

		fclose(iniFile);
	}

	void SetPath(const char* path)
	{
		if (m_FilePath)
		{
			free(m_FilePath);
			m_FilePath = nullptr;
		}
		m_FilePath = (char*)malloc(strlen(path) + 1);
		if (m_FilePath)
			strcpy(m_FilePath, path);
	}

	[[nodiscard]] int ReadInt(const char* section, const char* key, int iDefaultValue)
	{
		if (IniSection* sect = get(section); sect)
		{
			if (IniSection::IniKey* pKey = sect->get(key); pKey)
				return *pKey;
		}

		return iDefaultValue;
	}

	[[nodiscard]] int ReadInteger(const char* section, const char* key, int iDefaultValue)
	{
		return ReadInt(section, key, iDefaultValue);
	}

	void WriteInt(const char* section, const char* key, int value)
	{
		char iBuff[16];

		sprintf(iBuff, "%d", value);
		WriteString(section, key, iBuff);
	}

	void WriteInteger(const char* section, const char* key, int value)
	{
		WriteInt(section, key, value);
	}

	[[nodiscard]] float ReadFloat(const char* section, const char* key, float flDefValue)
	{
		if (IniSection* sect = get(section); sect)
		{
			if (IniSection::IniKey* pKey = sect->get(key); pKey)
				return *pKey;
		}

		return flDefValue;
	}

	void WriteFloat(const char* section, const char* key, float flValue)
	{
		char flBuff[32];

		sprintf(flBuff, "%f", flValue);
		WriteString(section, key, flBuff);
	}

	// Version with std::string
	[[nodiscard]] std::string ReadString(const char* section, const char* key, std::string szDefaultValue)
	{
		if (IniSection* sect = get(section); sect)
		{
			if (IniSection::IniKey* pKey = sect->get(key); pKey)
				return pKey->getValue();
		}
		
		return szDefaultValue;
	}

	void WriteString(const char* section, const char* key, const char* szValue)
	{
		WritePrivateProfileStringA(section, key, szValue, m_FilePath);
	}

	[[nodiscard]] bool ReadBool(const char* section, const char* key, bool bDefaultBool)
	{
		if (IniSection* sect = get(section); sect)
		{
			if (IniSection::IniKey* pKey = sect->get(key); pKey)
				return *pKey;
		}

		return bDefaultBool;
	}

	void WriteBool(const char* section, const char* key, bool bValue)
	{
		WriteString(section, key, bValue ? "true" : "false");
	}

	IniSection& operator[](const char* section)
	{
		IniSection dummy;

		for (IniSection& sect : m_sections)
		{
			if (!strcmp(sect.getSectionName(), section))
				return sect;
		}

		return dummy;
	};

	IniSection *get(const char* szSection)
	{
		for (IniSection &sect : m_sections)
		{
			if (!strcmp(sect.getSectionName(), szSection))
				return &sect;
		}

		return nullptr;
	}

	IniSection* add(const char* szSection)
	{
		if (IniSection* existing = get(szSection); !existing)
		{
			IniSection section(szSection, this);

			m_sections.push_back(section);

			return &m_sections.back();
		}
		else
		{
			return existing;
		}
	}

	IniSection* add(const char* szSection, const char* szKey)
	{
		if (IniSection *existingSect = get(szSection); !existingSect)
		{
			IniSection section(szSection, this);

			section.add(szKey);

			m_sections.push_back(section);

			return &m_sections.back();
		}
		else
		{
			existingSect->add(szKey);

			return existingSect;
		}
	}

	IniSection* add(const char* szSection, const char* szKey, std::string value)
	{
		if (IniSection* existingSect = get(szSection); !existingSect)
		{
			IniSection section(szSection, this);

			section.add(szKey, value);

			m_sections.push_back(section);

			return &m_sections.back();
		}
		else
		{
			existingSect->add(szKey, value);
			return existingSect;
		}
	}

	IniSection* begin() { return m_sections.data(); }
	const IniSection* begin() const { return m_sections.data(); }

	IniSection* end() { return m_sections.data() + m_sections.size(); }
	const IniSection* end() const { return m_sections.data() + m_sections.size(); }
};

#pragma warning(pop)