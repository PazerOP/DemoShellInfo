#pragma once

#include <Unknwn.h>

#include <atomic>
#include <cassert>

template<typename... Types>
class UnknownObject : public Types...
{
public:
	UnknownObject()
	{
		AddRef();
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override final
	{
		if (!ppvObject)
			return E_POINTER;

		if (auto found = TryGetInterface(riid))
		{
			found->AddRef();
			*ppvObject = found;
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef() override final { return ++m_RefCount; }
	ULONG STDMETHODCALLTYPE Release() override final
	{
		assert(m_RefCount > 0);

		auto newVal = --m_RefCount;
		if (newVal == 0)
			delete this;

		return newVal;
	}

protected:
	virtual IUnknown* TryGetInterface(REFIID riid) = 0;

private:
	std::atomic<ULONG> m_RefCount;
};