#pragma once

#include <windows.h>
#include <psapi.h>
#include "ModuleList.h"

using namespace std;

class CProcessInfo
{
	friend class CProcessEnumerator;
private:
	DWORD _pid;
	string _name;
	CModuleList _modules;

public:
	explicit CProcessInfo(DWORD processId) : _pid(processId)
	{
		TCHAR szModuleName[MAX_PATH];

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, _pid);
		// Get the process name.

		if (NULL != hProcess)
		{
			HMODULE hMod[1024];
			DWORD cbNeeded;

			if (EnumProcessModules(hProcess, hMod, sizeof(hMod), &cbNeeded))
			{
				for (unsigned i = 0; i < cbNeeded / sizeof(HMODULE); i++)
				{
					GetModuleBaseName(hProcess, hMod[i], szModuleName, sizeof(szModuleName) / sizeof(TCHAR));
					_modules.push_back(CModuleInfo(hMod[i], szModuleName));
				}
			}
		}

		_name = _modules.empty() ? "<unknown>" : _modules.front().name();

		CloseHandle(hProcess);
	}

	DWORD id() const { return _pid; };
	string name() const { return _name; };
	const CModuleList& modules() const { return _modules; };
};