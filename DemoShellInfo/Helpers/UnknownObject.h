#pragma once

#include <Unknwn.h>

#include <atomic>
#include <cassert>

class UnknownObjectBase
{
public:
	static uint32_t GetTotalRefCount() { return s_TotalRefCount; }

protected:
	static std::atomic<uint32_t> s_TotalRefCount;
};

template<typename... Types>
class UnknownObject : UnknownObjectBase, public Types...
{
public:
	UnknownObject() : m_RefCount(0)
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

	ULONG STDMETHODCALLTYPE AddRef() override final
	{
		++s_TotalRefCount;
		return ++m_RefCount;
	}
	ULONG STDMETHODCALLTYPE Release() override final
	{
		assert(s_TotalRefCount > 0);
		--s_TotalRefCount;

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