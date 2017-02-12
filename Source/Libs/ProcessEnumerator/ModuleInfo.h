#pragma once
#include <string>
#include <winnt.h>

class CModuleInfo
{
	friend class CProcessInfo;
	friend class CModuleList;
private:
	CModuleInfo(HMODULE hModule, TCHAR *moduleName) : _hModule(hModule), _moduleName(moduleName) {};

	HMODULE _hModule;
	std::string _moduleName;

public:
	HMODULE handle() const
	{
		return _hModule;
	}

	std::string name() const
	{
		return _moduleName;
	}
};