#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string>

#pragma warning(disable : 4996)

inline bool dummy()
{
	return false;
}

class IniReader
{
private:
	char* m_FilePath = nullptr; // Avoid stack based values(which will literally crash on path set)
public:
	class IniSection
	{
	public:
		char* section;
		IniReader* parent;

		struct IniKey
		{
			char* key;
			IniSection* section;

			IniKey(const char* keyName, IniSection* sect) : key((char*)keyName), section(sect) {};

			IniKey operator=(const char* value)
			{
				section->parent->WriteString(section->section, key, value);
				return *this;
			}

			operator const char* ()
			{
				return section->parent->ReadString(section->section, key, "");
			}

			IniKey operator=(float value)
			{
				section->parent->WriteFloat(section->section, key, value);
				return *this;
			}

			operator float()
			{
				return section->parent->ReadFloat(section->section, key, 0.0f);
			}

			IniKey operator=(int value)
			{
				section->parent->WriteInt(section->section, key, value);
				return *this;
			}

			operator int()
			{
				return section->parent->ReadInt(section->section, key, 0);
			}

			IniKey operator=(bool value)
			{
				section->parent->WriteBool(section->section, key, value);
				return *this;
			}

			operator bool()
			{
				return section->parent->ReadBool(section->section, key, false);
			}
		};

		IniSection(const char* sect, IniReader* par) : section((char*)sect), parent(par) {};

		IniKey operator[](const char* key)
		{
			return IniKey(key, this);
		}
	};

	void SetIniPath(const char* filename)
	{
		if (this->m_FilePath)
		{
			free(this->m_FilePath);
			this->m_FilePath = nullptr;
		}

		if (!strcmp(filename, ""))
		{
			filename = "default.ini";
		}
		else
		{
			char fileExt[6];
			strcpy(fileExt, &filename[strlen(filename) - 4]);

			for (int i = 0; ;)
				fileExt[i] = tolower(fileExt[i++]);

			if (strcmp(fileExt, ".ini")) // If we don't have .ini extension, just append it
				strcat(fileExt, ".ini");
		}

		char buff[MAX_PATH];
		HMODULE hm = NULL;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&dummy, &hm);
		GetModuleFileNameA(hm, buff, sizeof(buff));

		char* ptr = strrchr(buff, '\\'); // remove the executeable name from the path

		if (ptr)
			*ptr = '\0';

		if (!strncmp(filename, buff, strlen(buff)))
		{
			SetPath(filename);
			return;
		}

		this->m_FilePath = (char*)malloc(strlen(buff) + strlen(filename) + 2);
		if (this->m_FilePath)
			sprintf(this->m_FilePath, "%s\\%s", buff, filename);
	}

	IniReader()
	{
		this->m_FilePath = nullptr;
	}

	IniReader(const char* fileName)
	{
		SetIniPath(fileName);
	}
	
	IniReader(std::string fileName)
	{
		SetIniPath(fileName.c_str());
	}

	~IniReader()
	{
		if (this->m_FilePath)
		{
			free(this->m_FilePath);
			this->m_FilePath = nullptr;
		}
	}

	void SetPath(const char* path)
	{
		if (this->m_FilePath)
		{
			free(this->m_FilePath);
			this->m_FilePath = nullptr;
		}
		this->m_FilePath = (char*)malloc(strlen(path) + 1);
		if (this->m_FilePath)
			strcpy(this->m_FilePath, path);
	}

	int ReadInt(const char* section, const char* key, int iDefaultValue)
	{
		return GetPrivateProfileIntA(section, key, iDefaultValue, m_FilePath);
	}

	int ReadInteger(const char* section, const char* key, int iDefaultValue)
	{
		return ReadInt(section, key, iDefaultValue);
	}

	void WriteInt(const char* section, const char* key, int value)
	{
		char iBuff[32];

		sprintf(iBuff, "%d", value);
		WritePrivateProfileStringA(section, key, iBuff, m_FilePath);
	}

	void WriteInteger(const char* section, const char* key, int value)
	{
		WriteInt(section, key, value);
	}

	float ReadFloat(const char* section, const char* key, float flDefValue)
	{
		char flRes[32];
		char flDef[32];

		sprintf(flDef, "%f", flDefValue);
		GetPrivateProfileStringA(section, key, flDef, flRes, sizeof(flRes), m_FilePath);

		return (float)atof(flRes);
	}

	void WriteFloat(const char* section, const char* key, float flValue)
	{
		char flBuff[32];

		sprintf(flBuff, "%f", flValue);
		WritePrivateProfileStringA(section, key, flBuff, m_FilePath);
	}

	const char* ReadString(const char* section, const char* key, const char* szDefaultValue)
	{
		static char buff[512];
		memset(buff, 0, sizeof(buff));

		GetPrivateProfileStringA(section, key, szDefaultValue, buff, sizeof(buff), m_FilePath);

		return buff;
	}
	// dynamic buffer, returns the allocated string, must be freed after the use
	char* ReadString(const char* szSection, const char* szKey, const char* szDefaultValue, char** pValue)
	{
		auto stringLength = GetPrivateProfileStringA(szSection, szKey, nullptr, nullptr, 0, m_FilePath);
		if (stringLength >= 1 && pValue)
		{
			*pValue = (char*)malloc(stringLength + 1);
			GetPrivateProfileStringA(szSection, szKey, szDefaultValue, *pValue, stringLength, m_FilePath);

			return *pValue;
		}
		return nullptr;
	}

	void WriteString(const char* section, const char* key, const char* szValue)
	{
		WritePrivateProfileStringA(section, key, szValue, m_FilePath);
	}

	bool ReadBool(const char* section, const char* key, bool bDefaultBool)
	{
		char resBuff[8];

		GetPrivateProfileStringA(section, key, bDefaultBool ? "true" : "false", resBuff, sizeof(resBuff), m_FilePath);
	
		size_t resBuffSize = strlen(resBuff);
		
		for (unsigned int i = 0; ; i++)
		{
			if (i >= resBuffSize || resBuff[i] == '\0')
				break;
			
			resBuff[i] = tolower(resBuff[i]);
		} // case sensitive

		if (!strcmp(resBuff, "true"))
			return true;
		else if (!strcmp(resBuff, "false"))
			return false;

		return bDefaultBool;
	}

	void WriteBool(const char* section, const char* key, bool bValue)
	{
		WritePrivateProfileStringA(section, key, bValue ? "true" : "false", m_FilePath);
	}

	IniSection operator[](const char* section)
	{
		return IniSection(section, this);
	};
};