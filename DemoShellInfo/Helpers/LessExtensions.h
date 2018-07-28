#pragma once

#include <functional>

namespace std
{
	template<> struct less<GUID>
	{
		inline constexpr bool operator()(const GUID& left, const GUID& right) const
		{
			if (left.Data1 < right.Data1)
				return true;
			else if (left.Data1 > right.Data1)
				return false;

			if (left.Data2 < right.Data2)
				return true;
			else if (left.Data2 > right.Data2)
				return false;

			if (left.Data3 < right.Data3)
				return true;
			else if (left.Data3 > right.Data3)
				return false;

			for (uint_fast8_t i = 0; i < std::size(left.Data4); i++)
			{
				if (left.Data4[i] < right.Data4[i])
					return true;
				else if (left.Data4[i] > right.Data4[i])
					return false;
			}

			return false;
		}
	};
	template<> struct less<PROPERTYKEY>
	{
		inline constexpr bool operator()(const PROPERTYKEY& left, const PROPERTYKEY& right) const
		{
			return std::less<GUID>{}(left.fmtid, right.fmtid) && left.pid < right.pid;
		}
	};
}