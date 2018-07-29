#include "DemoPropertyStorage.h"

#include <propsys.h>

DemoPropertyStorage::DemoPropertyStorage(IID fmtid, IPropertyStore* parent, DWORD flags) : m_FmtID(fmtid), m_Parent(parent), m_Flags(flags)
{
	m_Parent->AddRef();

	GetFileTime(m_CreatedTime);
	m_ModifiedTime = m_AccessedTime = m_CreatedTime;
}
DemoPropertyStorage::~DemoPropertyStorage()
{
	m_Parent->Release();
}

HRESULT STDMETHODCALLTYPE DemoPropertyStorage::ReadMultiple(ULONG cpspec, const PROPSPEC rgpspec[], PROPVARIANT rgpropvar[])
{
	LOCK_GUARD(m_Mutex);

	GetFileTime(m_AccessedTime);

	for (ULONG i = 0; i < cpspec; i++)
	{
		assert(rgpspec[i].ulKind == PRSPEC_PROPID);
		if (rgpspec[i].ulKind != PRSPEC_PROPID)
			continue;

		if (auto found = m_SetProperties.find(rgpspec[i].propid); found != m_SetProperties.end())
		{
			PropVariantInit(&rgpropvar[i]);
			PropVariantCopy(&rgpropvar[i], &found->second.Get());
			continue;
		}

		if (auto hr = m_Parent->GetValue(CreatePropertyKey(m_FmtID, rgpspec[i].propid), &rgpropvar[i]))
		{
			if (hr == E_NOT_SET)
				PropVariantInit(&rgpropvar[i]);
			else
				return hr;
		}
	}

	return S_OK;
}
HRESULT STDMETHODCALLTYPE DemoPropertyStorage::WriteMultiple(ULONG cpspec, const PROPSPEC rgpspec[], const PROPVARIANT rgpropvar[], PROPID propidNameFirst)
{
	LOCK_GUARD(m_Mutex);

	GetFileTime(m_ModifiedTime);

	for (ULONG i = 0; i < cpspec; i++)
	{
		assert(rgpspec[i].ulKind == PRSPEC_PROPID);
		if (rgpspec[i].ulKind != PRSPEC_PROPID)
			continue;

		m_SetProperties[rgpspec[i].propid] = rgpropvar[i];
	}

	return S_OK;
}
HRESULT STDMETHODCALLTYPE DemoPropertyStorage::DeleteMultiple(ULONG cpspec, const PROPSPEC rgpspec[])
{
	LOCK_GUARD(m_Mutex);

	GetFileTime(m_ModifiedTime);

	for (ULONG i = 0; i < cpspec; i++)
	{
		assert(rgpspec[i].ulKind == PRSPEC_PROPID);
		if (rgpspec[i].ulKind != PRSPEC_PROPID)
			continue;

		m_SetProperties.erase(rgpspec[i].propid);
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DemoPropertyStorage::ReadPropertyNames(ULONG cpropid, const PROPID rgpropid[], LPOLESTR rglpwstrName[])
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE DemoPropertyStorage::WritePropertyNames(ULONG cpropid, const PROPID rgpropid[], const LPOLESTR rglpwstrName[])
{
	//LOCK_GUARD(m_Mutex);
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE DemoPropertyStorage::DeletePropertyNames(ULONG cpropid, const PROPID rgpropid[])
{
	//LOCK_GUARD(m_Mutex);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DemoPropertyStorage::Commit(DWORD grfCommitFlags)
{
	LOCK_GUARD(m_Mutex);

	for (auto& prop : m_SetProperties)
	{
		if (auto hr = m_Parent->SetValue(GetPropertyKey(prop.first), prop.second.Get()))
			return hr;
	}

	if (auto hr = m_Parent->Commit())
		return hr;

	return S_OK;
}
HRESULT STDMETHODCALLTYPE DemoPropertyStorage::Revert()
{
	LOCK_GUARD(m_Mutex);

	GetFileTime(m_ModifiedTime);

	m_SetProperties.clear();

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DemoPropertyStorage::Enum(IEnumSTATPROPSTG** ppenum)
{
	//LOCK_GUARD(m_Mutex);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DemoPropertyStorage::SetTimes(const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime)
{
	LOCK_GUARD(m_Mutex);

	if (pctime)
		m_CreatedTime = *pctime;
	if (patime)
		m_AccessedTime = *patime;
	if (pmtime)
		m_ModifiedTime = *pmtime;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE DemoPropertyStorage::SetClass(REFCLSID clsid)
{
	LOCK_GUARD(m_Mutex);

	m_CLSID = clsid;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE DemoPropertyStorage::Stat(STATPROPSETSTG* pstatpsstg)
{
	if (!pstatpsstg)
		return E_POINTER;

	LOCK_GUARD(m_Mutex);

	pstatpsstg->fmtid = m_FmtID;
	pstatpsstg->clsid = m_CLSID;
	pstatpsstg->grfFlags = m_Flags;
	pstatpsstg->ctime = m_CreatedTime;
	pstatpsstg->mtime = m_ModifiedTime;
	pstatpsstg->atime = m_AccessedTime;

	return S_OK;
}

IUnknown* DemoPropertyStorage::TryGetInterface(REFIID riid)
{
	if (riid == __uuidof(IPropertyStorage))
		return static_cast<IPropertyStorage*>(this);

	return nullptr;
}