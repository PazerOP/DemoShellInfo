#pragma once

#include <Windows.h>
#include <cstdint>

inline constexpr LARGE_INTEGER CreateLargeInteger(int64_t val)
{
	LARGE_INTEGER retVal{};
	retVal.QuadPart = val;
	return retVal;
}
inline constexpr ULARGE_INTEGER CreateULargeInteger(uint64_t val)
{
	ULARGE_INTEGER retVal{};
	retVal.QuadPart = val;
	return retVal;
}