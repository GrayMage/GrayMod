#pragma once

#include <windows.h>
#include <process.h>

#pragma data_seg(".shared")
HANDLE hCmdThread = nullptr;
DWORD idCmdThread = 0;
bool bRemoteLoad = false;
#pragma data_seg()
#pragma comment(linker, "/section:.shared,rws")

HINSTANCE hDll = nullptr, hDllInjector = nullptr;

#define EXPORT extern "C" __declspec(dllexport)

#define CMD_INJECT WM_USER
#define CMD_EJECT WM_USER + 1

EXPORT void inject(void* dllPath)
{
	PostThreadMessage(idCmdThread, CMD_INJECT, 0, (LPARAM)dllPath);
}

EXPORT void eject()
{
	PostThreadMessage(idCmdThread, CMD_EJECT, 0, 0);
	WaitForSingleObject(hCmdThread, INFINITE);
}

void commandFunc(void*)
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		switch (msg.message)
		{
		case CMD_INJECT:
		{
			auto dllPath = (LPCSTR)msg.lParam;
			hDll = LoadLibrary(dllPath);
			
			if(!hDll) break;

			using TPFNLoaded = void(*)(HINSTANCE);
			
			auto pfnLoaded = (TPFNLoaded)GetProcAddress(hDll, "Loaded");
			if(pfnLoaded) pfnLoaded(hDll);
		}
			break;
		case CMD_EJECT:
			using TPFNUnloading = void(*)();
			auto pfnUnloading = (TPFNUnloading)GetProcAddress(hDll, "Unloading");
			if (pfnUnloading) pfnUnloading();

			FreeLibrary(hDll);
			FreeLibrary(hDllInjector);
			return;
		}
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		if (bRemoteLoad)
		{
			hCmdThread = (HANDLE)_beginthread((_beginthread_proc_type)commandFunc, 0, nullptr);
			idCmdThread = GetThreadId(hCmdThread);
			hDllInjector = hinstDLL;
		}
		else bRemoteLoad = true;

		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	
	return TRUE;
}