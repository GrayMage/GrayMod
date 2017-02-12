#pragma once
#include <list>
#include <functional>
#include "ProcessInfo.h"
#include <psapi.h>

using namespace std;

class CProcessEnumerator
{
public:
	CProcessEnumerator() = delete;

	static list<CProcessInfo> processes(function<bool(CProcessInfo&)> filterFunc)
	{
		list<CProcessInfo> retLst;

		DWORD aProcesses[1024], cbNeeded, cProcesses;

		EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);

		cProcesses = cbNeeded / sizeof(DWORD);

		for (unsigned i = 0; i < cProcesses; i++)
		{
			if (aProcesses[i] == 0) continue;

			CProcessInfo pi(aProcesses[i]);
			if (filterFunc(pi)) retLst.push_back(pi);
		}

		return retLst;
	}

	static list<CProcessInfo> processes()
	{
		return processes([](auto pi) { return true; });
	}
};

