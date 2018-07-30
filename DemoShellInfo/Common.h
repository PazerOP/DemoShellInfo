#pragma once
#pragma warning(disable : 4100)  // 'x': unreferenced formal parameter

#include "Helpers/LargeIntegerCreator.h"
#include "Helpers/LessExtensions.h"
#include "Helpers/PropVariantSafe.h"
#include "Helpers/UnknownDeleter.h"
#include "Helpers/UnknownObject.h"

#include <propvarutil.h>

// {C6FF826A-1EF5-4526-95DC-134BCC7B3BE3}
static constexpr GUID s_DemoShellInfoCLSID = { 0xc6ff826a, 0x1ef5, 0x4526, { 0x95, 0xdc, 0x13, 0x4b, 0xcc, 0x7b, 0x3b, 0xe3 } };

inline constexpr PROPERTYKEY CreatePropertyKey(const GUID& fmtid, DWORD pid)
{
	return PROPERTYKEY{ fmtid, pid };
}

inline bool GetFileTime(FILETIME& time)
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	if (SystemTimeToFileTime(&st, &time))
		return false;

	return true;
}

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res

#define MAKE_UNIQUE_NAME(base) PP_CAT(base, __COUNTER__)

#define LOCK_GUARD(mutex) std::lock_guard<decltype(mutex)> MAKE_UNIQUE_NAME(__lock_guard)(mutex)

//#define LOCK_GUARD(mutex)