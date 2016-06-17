// InstantCompile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <map>
#include <string>

std::map<std::wstring, FILETIME> g_kAPI;

int _tmain(int argc, _TCHAR* argv[])
{
	while(true)
	{
		std::wstring in_api;

		std::wcout << TEXT("Please input API : ");
		std::getline(std::wcin, in_api);

		if(in_api == TEXT("exit"))
			break;
		else if(in_api == TEXT("list"))
		{
			wprintf(TEXT("[%d] API\n"), g_kAPI.size());

			for(auto it = g_kAPI.begin(); it != g_kAPI.end(); it++)
			{
				SYSTEMTIME kSys;
				FileTimeToSystemTime(&it->second, &kSys);
				wprintf(TEXT("API : %s - Date[%04d/%02d/%02d %02d:%02d:%02d:%03d]\n"), it->first.c_str(),
					kSys.wYear, kSys.wMonth, kSys.wDay, kSys.wHour, kSys.wMinute, kSys.wSecond, kSys.wMilliseconds);
			}

			continue;
		}

		HANDLE hFile = CreateFile(in_api.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			wprintf(TEXT("Could not open file [%s], error [%d]\n"), in_api.c_str(), GetLastError());
		else
		{
			FILETIME kFT;
			if(GetFileTime(hFile, NULL, NULL, &kFT))
			{
				auto it = g_kAPI.find(in_api);

				// new file
				if(it == g_kAPI.end())
				{
					wprintf(TEXT("++ New File [%s] has been added\n"), in_api.c_str());
					g_kAPI[in_api] = kFT;
				}

				else
				{
					// update
					if(memcmp(&kFT, &it->second, sizeof(FILETIME)) != 0)
					{
						wprintf(TEXT("!! File [%s] has been updated\n"), in_api.c_str());
						it->second = kFT;
					}

					else
						wprintf(TEXT("Call API [%s]\n"), in_api.c_str());
				}
			}
			else
				wprintf(TEXT("Failed to get file time [%s], error [%d]\n"), in_api.c_str(), GetLastError());

			CloseHandle(hFile);
		}

	}

	return 0;
}

