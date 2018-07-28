#pragma once

#include <propvarutil.h>

#include <iostream>

class PropVariantSafe final
{
public:
	PropVariantSafe()
	{
		PropVariantInit(&m_Variant);
	}
	~PropVariantSafe()
	{
		if (auto hr = PropVariantClear(&m_Variant))
		{
			std::cerr << "test";
			OutputDebugStringA("")
		}
	}

private:
	PROPVARIANT m_Variant;
};