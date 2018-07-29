#pragma once

struct UnknownDeleter
{
	void operator()(IUnknown* unknown) const
	{
		unknown->Release();
	}
};