#pragma once

#include <list>
#include <functional>
#include "ModuleInfo.h"

using namespace std;

class CModuleList : public list<CModuleInfo>
{
public:
	bool contains(function<bool(const CModuleInfo &mi)> pattern) const
	{
		for (auto &m : *this)
		{
			if (pattern(m)) return true;
		}

		return false;
	}

	CModuleInfo find(string name) const
	{
		for (auto &m : *this)
		{
			if (m.name() == name) return m;
		}

		return CModuleInfo(0, 0);
	}
};