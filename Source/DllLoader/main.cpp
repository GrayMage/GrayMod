#pragma once
#include <conio.h>
#include <Windows.h>
#include <iostream>
#include <string>

#include "../Libs/ProcessEnumerator/ProcessEnumerator.h"

#define IMPORT extern "C" __declspec(dllimport)

IMPORT void inject(void*);
IMPORT void eject();

typedef HRESULT(__stdcall *PFNNTCREATETHREADEX)(PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	LPVOID ObjectAttributes,
	HANDLE ProcessHandle,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	BOOL CreateSuspended,
	DWORD dwStackSize,
	LPVOID d1,
	LPVOID d2,
	LPVOID d3);

using namespace std;

void printHelp()
{
	cout << "Usage: DllLoader <Target_process_Name or Target_process_id> <DLL full path>" << endl;
}

void terminate(int exitCode)
{
	Sleep(3000);
	exit(exitCode);
}

bool fileExists(TCHAR * file)
{
	WIN32_FIND_DATA findFileData;
	HANDLE handle = FindFirstFile(file, &findFileData);
	bool bFound = handle != INVALID_HANDLE_VALUE;
	if (bFound)
	{
		FindClose(handle);
	}
	return bFound;
}

string getInjectorPath(char *argv[])
{
	string exePath(argv[0]);
#ifdef _M_X64
	return exePath.substr(0, exePath.find_last_of("/\\") + 1) + "DllInjector64.dll";
#else
	return exePath.substr(0, exePath.find_last_of("/\\") + 1) + "DllInjector.dll";
#endif
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printHelp();
		terminate(0);
	}

	string processName(argv[1]);

	if (!fileExists(argv[2]))
	{
		cout << "Dll file not found!";
		terminate(2);
	};

	DWORD pid;
	bool bPidSpecified = true;
	try
	{
		pid = std::stol(processName);
	}
	catch (...) { bPidSpecified = false; }

	if (!bPidSpecified)
	{
		list<CProcessInfo> pl;

		cout << "Waiting for process \"" << processName << "\"..." << endl;
		while (true)
		{
			pl = CProcessEnumerator::processes([&](CProcessInfo &pi)
			{
				return pi.name() == processName;
			});

			if (!pl.empty()) break;
			Sleep(1000);
		}
		cout << "Process found" << endl;

		pid = pl.front().id();
	}

	string dllPath(argv[2]);

	string dllIjectorPath(getInjectorPath(argv));

	cout << "Using dll injector: " << dllIjectorPath << endl;

	auto pfnNtCreateThreadEx = (PFNNTCREATETHREADEX)GetProcAddress(GetModuleHandle("ntdll"), "NtCreateThreadEx");

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProc)
	{
		cout << "Failed to open process " << pid;
		terminate(3);
	}

	LPVOID pRemoteStr = VirtualAllocEx(hProc, nullptr, dllIjectorPath.size() + 1, MEM_COMMIT, PAGE_READWRITE);
	if (!pRemoteStr)
	{
		cout << "Failed to reserve region";
		CloseHandle(hProc);	
		terminate(4);
	}

	if (!WriteProcessMemory(hProc, pRemoteStr, dllIjectorPath.c_str(), dllIjectorPath.size() + 1, nullptr))
	{
		cout << "Failed to write process memory";
		VirtualFreeEx(hProc, pRemoteStr, MAX_PATH, MEM_RELEASE);
		CloseHandle(hProc);
		terminate(5);
	}
	cout << "Dll path " << dllIjectorPath << " written into remote address space" << endl;

	HANDLE hThread = nullptr;
	pfnNtCreateThreadEx(&hThread, MAXIMUM_ALLOWED, nullptr, hProc, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"), pRemoteStr, FALSE, 0, nullptr, nullptr, nullptr);
	if (!hThread)
		cout << "Failed to create injecting remote thread" << endl;
	else
	{
		cout << "Injecting remote thread created with id: " << GetThreadId(hThread) << endl;
		cout << "Waiting for it to do the work..." << endl;
	}

	WaitForSingleObject(hThread, INFINITE);
	
	DWORD hLoadedLib;
	GetExitCodeThread(hThread, &hLoadedLib);
	CloseHandle(hThread);

	cout << "Sending inject command for " << dllPath << " and calling 'void Loaded(HINSTANCE)' callback..." << endl;

	auto pTargetDll = VirtualAllocEx(hProc, nullptr, dllPath.size() + 1, MEM_COMMIT, PAGE_READWRITE);
	if(!WriteProcessMemory(hProc, pTargetDll, dllPath.c_str(), dllPath.size() + 1, nullptr))
	{
		cout << "Failed to write process memory";
		VirtualFreeEx(hProc, pTargetDll, MAX_PATH, MEM_RELEASE);
		CloseHandle(hProc);
		terminate(5);
	}

	inject(pTargetDll);
	VirtualFreeEx(hProc, pTargetDll, dllPath.size() + 1, MEM_RELEASE);
	cout << "Everything seems to be ok..." << endl;
	cout << "Type 'x' to eject dll..." << endl;
	while (_getch() != 'x') {}
	cout << "Sending eject command and calling 'void Unloading()' callback..." << endl;
	eject();

	VirtualFreeEx(hProc, pRemoteStr, dllPath.size() + 1, MEM_RELEASE);
	CloseHandle(hProc);

	cout << endl << endl << "Press any key to exit..." << endl;
	_getch();
	return 0;
}