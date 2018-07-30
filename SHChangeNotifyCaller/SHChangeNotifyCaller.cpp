// SHChangeNotifyCaller.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Shlobj.h>

#include <cassert>

int main()
{
	if (auto hr = CoInitialize(nullptr); hr != S_OK)
		return 1;

	if (auto hr = PSUnregisterPropertySchema(L"C:\\Users\\matt\\source\\repos\\DemoShellInfo\\DemoShellInfo\\DemoShellInfo.propdesc"); hr != S_OK)
		return 2;

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return 0;
}

