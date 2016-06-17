// InstantCompile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <map>
#include <string>

typedef int (* FNMODULEMAIN)(int, _TCHAR* argv[]);

typedef struct T_MODULE_BASE
{
	HANDLE ModuleHandle;
	FNMODULEMAIN EntryFunctiom;
	FILETIME LastWrite;
	std::string API;

	T_MODULE_BASE()
	{
		ZeroMemory(&LastWrite, sizeof(LastWrite));
		EntryFunctiom = NULL;
		ModuleHandle = INVALID_HANDLE_VALUE;
	}
	
} T_MODULE_BASE;

std::map<std::string, T_MODULE_BASE> g_kAPI;
std::string g_strDLLBase;

void LoadDLLBase(const char * szBase)
{
	FILE * fp = fopen(szBase, "rt");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		DWORD dwFileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char * szBuffer = new char[dwFileSize+1];

		int i = 0;
		while(!feof(fp))
		{
			char cc;
			fread(&cc, 1, 1, fp);

			szBuffer[i] = cc;

			printf("%d : %x\n", i++, cc);
		}

		szBuffer[i] = 0x00;

		fclose(fp);

		g_strDLLBase = szBuffer;
		delete [] szBuffer;
	}
}

void CompileModule(T_MODULE_BASE & in)
{
	char szFile[8096];

	sprintf(szFile, g_strDLLBase.c_str(), in.API.c_str());

	FILE * fp = fopen("compile_temp.cpp", "wt");
	if(fp)
	{
		fputs(szFile, fp);
		fclose(fp);
	}
}

void ExecModule(T_MODULE_BASE & in)
{
}


int _tmain(int argc, _TCHAR* argv[])
{
	LoadDLLBase("dllbase.txt");

	while(true)
	{
		std::string in_api;

		std::cout << "Please input API : ";
		std::getline(std::cin, in_api);

		if(in_api == "exit")
			break;
		else if(in_api == "list")
		{
			printf("[%d] API\n", g_kAPI.size());

			for(auto it = g_kAPI.begin(); it != g_kAPI.end(); it++)
			{
				SYSTEMTIME kSys;
				FileTimeToSystemTime(&it->second.LastWrite, &kSys);
				printf("API : %s - Date[%04d/%02d/%02d %02d:%02d:%02d:%03d]\n", it->first.c_str(),
					kSys.wYear, kSys.wMonth, kSys.wDay, kSys.wHour, kSys.wMinute, kSys.wSecond, kSys.wMilliseconds);
			}

			continue;
		}
		else if(in_api == "1")
			in_api = "hello.cpp";

		HANDLE hFile = CreateFileA(in_api.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			printf("Could not open file [%s], error [%d]\n", in_api.c_str(), GetLastError());
		else
		{
			FILETIME kFT;
			if(GetFileTime(hFile, NULL, NULL, &kFT))
			{
				auto it = g_kAPI.find(in_api);

				// new file
				if(it == g_kAPI.end())
				{
					printf("++ New File [%s] has been added\n", in_api.c_str());

					T_MODULE_BASE kM;
					kM.API = in_api;
					CopyMemory(&kM.LastWrite, &kFT, sizeof(FILETIME));
					g_kAPI[in_api] = kM;

					CompileModule(g_kAPI[in_api]);
				}

				else
				{
					// update
					if(memcmp(&kFT, &it->second.LastWrite, sizeof(FILETIME)) != 0)
					{
						printf("!! File [%s] has been updated\n", in_api.c_str());
						CopyMemory(&it->second.LastWrite, &kFT, sizeof(FILETIME));

						CompileModule(it->second);
					}

					else
					{
						printf("Call API [%s]\n", in_api.c_str());
						ExecModule(g_kAPI[in_api]);
					}
				}
			}
			else
				printf("Failed to get file time [%s], error [%d]\n", in_api.c_str(), GetLastError());

			CloseHandle(hFile);
		}
	}

	return 0;
}

