#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string>

#pragma warning(disable : 4996)

inline bool ends_with(const char* str, const char* prefix, bool case_sensitive)
{
	auto str2 = &str[strlen(str) - 1];
	auto prefix2 = &prefix[strlen(prefix) - 1];

	while (prefix2 >= prefix)
	{
		bool equal;
		if (case_sensitive)
			equal = (*str2-- == *prefix2--);
		else
			equal = (::tolower(*str2--) == ::tolower(*prefix2--));

		if (!equal) return false;
	}
	return true;
}

class IniReader
{
private:
	char* filePath;
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
		if (this->filePath)
		{
			free(this->filePath);
			this->filePath = nullptr;
		}
		char buff[MAX_PATH];
		HMODULE hm = NULL;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&ends_with, &hm);
		GetModuleFileNameA(hm, buff, sizeof(buff));

		char* ptr = strrchr(buff, '\\');

		if (ptr)
			*ptr = '\0';

		if (!strncmp(filename, buff, strlen(buff)))
		{
			SetPath(filename);
			return;
		}

		this->filePath = (char*)malloc(strlen(buff) + strlen(filename) + 2);
		if (this->filePath)
			sprintf(this->filePath, "%s\\%s", buff, filename);
	}

	IniReader()
	{
		this->filePath = nullptr;
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
		if (this->filePath)
		{
			free(this->filePath);
			this->filePath = nullptr;
		}
	}

	void SetPath(const char* path)
	{
		if (this->filePath)
		{
			free(this->filePath);
			this->filePath = nullptr;
		}
		this->filePath = (char*)malloc(strlen(path) + 1);
		if (this->filePath)
			strcpy(this->filePath, path);
	}

	int ReadInt(const char* section, const char* key, int iDefaultValue)
	{
		return GetPrivateProfileIntA(section, key, iDefaultValue, filePath);
	}

	void WriteInt(const char* section, const char* key, int value)
	{
		char iBuff[32];

		sprintf(iBuff, "%d", value);
		WritePrivateProfileStringA(section, key, iBuff, filePath);
	}

	float ReadFloat(const char* section, const char* key, float flDefValue)
	{
		char flRes[32];
		char flDef[32];

		sprintf(flDef, "%f", flDefValue);
		GetPrivateProfileStringA(section, key, flDef, flRes, sizeof(flRes), filePath);

		return atof(flRes);
	}

	void WriteFloat(const char* section, const char* key, float flValue)
	{
		char flBuff[32];

		sprintf(flBuff, "%f", flValue);
		WritePrivateProfileStringA(section, key, flBuff, filePath);
	}

	const char* ReadString(const char* section, const char* key, const char* szDefaultValue)
	{
		static char buff[512];

		GetPrivateProfileStringA(section, key, szDefaultValue, buff, sizeof(buff), filePath);

		return buff;
	}

	void WriteString(const char* section, const char* key, const char* szValue)
	{
		WritePrivateProfileStringA(section, key, szValue, filePath);
	}

	bool ReadBool(const char* section, const char* key, bool bDefaultBool)
	{
		char buff[8];
		char resBuff[8];

		sprintf(buff, "%s", bDefaultBool ? "true" : "false");

		GetPrivateProfileStringA(section, key, buff, resBuff, sizeof(resBuff), filePath);
	
		size_t resBuffSize = strlen(resBuff);
		
		for (int i = 0; ; i++)
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
		WritePrivateProfileStringA(section, key, bValue ? "true" : "false", filePath);
	}

	IniSection operator[](const char* section)
	{
		return IniSection(section, this);
	};
};