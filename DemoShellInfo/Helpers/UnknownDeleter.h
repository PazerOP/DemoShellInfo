#pragma once

#include <memory>

struct UnknownDeleter
{
	void operator()(IUnknown* unknown) const
	{
		unknown->Release();
	}
};

template<typename T> using UnknownPtr = std::unique_ptr<T, UnknownDeleter>;