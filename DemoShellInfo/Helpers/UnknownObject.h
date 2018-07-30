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

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (!ppvObject)
			return E_POINTER;

		if (auto found = TryGetInterface(riid); found.first)
		{
			found.second->AddRef();
			*ppvObject = found.first;
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
	using InterfacePair = std::pair<void*, IUnknown*>;
	virtual InterfacePair TryGetInterface(REFIID riid) = 0;
	template<typename T, typename T2> static InterfacePair GetInterface(T2* fullType)
	{
		return std::pair<void*, IUnknown*>(static_cast<T*>(fullType), static_cast<IUnknown*>(static_cast<T*>(fullType)));
	}
	static constexpr InterfacePair NO_INTERFACE = std::make_pair<void*, IUnknown*>(nullptr, nullptr);

private:
	std::atomic<ULONG> m_RefCount;
};