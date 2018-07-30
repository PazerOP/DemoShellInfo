#pragma once

#include <map>
#include <mutex>
#include <random>

#undef max

struct IPropertyStore;

class DemoPropertyStorage final : public UnknownObject<IPropertyStorage>
{
public:
	DemoPropertyStorage(IID fmtid, IPropertyStore* parent, DWORD flags = PROPSETFLAG_DEFAULT);
	~DemoPropertyStorage();

	HRESULT STDMETHODCALLTYPE ReadMultiple(ULONG cpspec, const PROPSPEC rgpspec[], PROPVARIANT rgpropvar[]) override;
	HRESULT STDMETHODCALLTYPE WriteMultiple(ULONG cpspec, const PROPSPEC rgpspec[], const PROPVARIANT rgpropvar[], PROPID propidNameFirst) override;
	HRESULT STDMETHODCALLTYPE DeleteMultiple(ULONG cpspec, const PROPSPEC rgpspec[]) override;

	HRESULT STDMETHODCALLTYPE ReadPropertyNames(ULONG cpropid, const PROPID rgpropid[], LPOLESTR rglpwstrName[]) override;
	HRESULT STDMETHODCALLTYPE WritePropertyNames(ULONG cpropid, const PROPID rgpropid[], const LPOLESTR rglpwstrName[]) override;
	HRESULT STDMETHODCALLTYPE DeletePropertyNames(ULONG cpropid, const PROPID rgpropid[]) override;

	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) override;
	HRESULT STDMETHODCALLTYPE Revert() override;

	HRESULT STDMETHODCALLTYPE Enum(IEnumSTATPROPSTG** ppenum) override;

	HRESULT STDMETHODCALLTYPE SetTimes(const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime) override;

	HRESULT STDMETHODCALLTYPE SetClass(REFCLSID clsid) override;

	HRESULT STDMETHODCALLTYPE Stat(STATPROPSETSTG* pstatpsstg) override;

protected:
	InterfacePair TryGetInterface(REFIID riid) override;

	PROPERTYKEY GetPropertyKey(PROPID id) const { return CreatePropertyKey(m_FmtID, id); }

private:
	IID m_FmtID;
	CLSID m_CLSID = CLSID_NULL;
	DWORD m_Flags;
	FILETIME m_ModifiedTime;
	FILETIME m_CreatedTime;
	FILETIME m_AccessedTime;

	IPropertyStore* m_Parent;
	std::mutex m_Mutex;
	std::map<PROPID, PropVariantSafe> m_SetProperties;
};