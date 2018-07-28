#pragma once

#include "Common.h"
#include "UnknownObject.h"

#include <map>
#include <mutex>
#include <random>

#undef max

class DemoPropertyStorage : public UnknownObject<IPropertyStorage>
{
	static constexpr PROPVARIANT GetEmptyPropVariant()
	{
		PROPVARIANT retVal{};
		retVal.vt = VT_NULL;
		return retVal;
	}
public:
	DemoPropertyStorage(IID fmtid) : m_FmtID(fmtid)
	{
		static std::default_random_engine s_Random;

		if (m_FmtID == PKEY_Media_Duration.fmtid)
		{
			auto scale = s_Random() / double(s_Random.max());
			m_Properties[PKEY_Media_Duration.pid] = CreatePropVariant((scale * 100) * 10000000);
		}
		else if (m_FmtID == PKEY_Calendar_Location.fmtid)
			m_Properties[PKEY_Calendar_Location.pid] = CreatePropVariant(L"Test location");
	}

	HRESULT STDMETHODCALLTYPE ReadMultiple(ULONG cpspec, const PROPSPEC rgpspec[], PROPVARIANT rgpropvar[]) override
	{
		LOCK_GUARD(m_Mutex);

		for (ULONG i = 0; i < cpspec; i++)
		{
			assert(rgpspec[i].ulKind == PRSPEC_PROPID);
			if (auto found = m_Properties.find(rgpspec[i].propid); found != m_Properties.end())
				rgpropvar[i] = found->second;
			else
				rgpropvar[i] = GetEmptyPropVariant();
		}

		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE WriteMultiple(ULONG cpspec, const PROPSPEC rgpspec[], const PROPVARIANT rgpropvar[], PROPID propidNameFirst) override
	{
		LOCK_GUARD(m_Mutex);

		for (ULONG i = 0; i < cpspec; i++)
		{
			assert(rgpspec[i].ulKind == PRSPEC_PROPID);
			m_Properties[rgpspec[i].propid] = rgpropvar[i];
		}

		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE DeleteMultiple(ULONG cpspec, const PROPSPEC rgpspec[]) override
	{
		LOCK_GUARD(m_Mutex);

		for (ULONG i = 0; i < cpspec; i++)
		{
			assert(rgpspec[i].ulKind == PRSPEC_PROPID);
			m_Properties.erase(rgpspec[i].propid);
		}

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE ReadPropertyNames(ULONG cpropid, const PROPID rgpropid[], LPOLESTR rglpwstrName[]) override
	{
		LOCK_GUARD(m_Mutex);

		for (ULONG i = 0; i < cpropid; i++)
		{

		}

		return E_UNEXPECTED;
	}
	HRESULT STDMETHODCALLTYPE WritePropertyNames(ULONG cpropid, const PROPID rgpropid[], const LPOLESTR rglpwstrName[]) override
	{
		LOCK_GUARD(m_Mutex);

		for (ULONG i = 0; i < cpropid; i++)
			m_PropNames[rgpropid[i]] = rglpwstrName[i];

		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE DeletePropertyNames(ULONG cpropid, const PROPID rgpropid[]) override
	{
		LOCK_GUARD(m_Mutex);

		for (ULONG i = 0; i < cpropid; i++)
			m_PropNames.erase(rgpropid[i]);

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) override
	{
		LOCK_GUARD(m_Mutex);

		return E_UNEXPECTED;
	}
	HRESULT STDMETHODCALLTYPE Revert() override
	{
		LOCK_GUARD(m_Mutex);

		return E_UNEXPECTED;
	}

	HRESULT STDMETHODCALLTYPE Enum(IEnumSTATPROPSTG** ppenum) override
	{
		LOCK_GUARD(m_Mutex);

		return E_UNEXPECTED;
	}

	HRESULT STDMETHODCALLTYPE SetTimes(const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime) override
	{
		LOCK_GUARD(m_Mutex);

		return E_UNEXPECTED;
	}

	HRESULT STDMETHODCALLTYPE SetClass(REFCLSID clsid) override
	{
		LOCK_GUARD(m_Mutex);

		m_ID = clsid;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Stat(STATPROPSETSTG* pstatpsstg) override
	{
		LOCK_GUARD(m_Mutex);

		return E_UNEXPECTED;
	}

protected:
	IUnknown* TryGetInterface(REFIID riid) override
	{
		if (riid == __uuidof(IPropertyStorage))
			return static_cast<IPropertyStorage*>(this);

		return nullptr;
	}

private:
	IID m_FmtID;
	std::mutex m_Mutex;

	CLSID m_ID;
	std::map<PROPID, PROPVARIANT> m_Properties;
	std::map<PROPID, std::wstring> m_PropNames;
};