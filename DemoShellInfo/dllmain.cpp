// dllmain.cpp : Defines the entry point for the DLL application.

#include "ClassFactory.h"

#include <propsys.h>
#include <Shlobj.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	if (!ppv)
		return E_POINTER;

	if (rclsid == s_DemoShellInfoCLSID && riid == __uuidof(IClassFactory))
	{
		*ppv = CreateClassFactory();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI DllRegisterServer()
{
	if (auto hr = PSRegisterPropertySchema(L"DemoShellInfo.propdesc"))
		return hr;

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}
STDAPI DllUnregisterServer()
{
	if (auto hr = PSUnregisterPropertySchema(L"DemoShellInfo.propdesc"))
		return hr;

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}

STDAPI DllCanUnloadNow()
{
	auto totalRefCount = UnknownObjectBase::GetTotalRefCount();
	return totalRefCount == 0 ? S_OK : S_FALSE;
}