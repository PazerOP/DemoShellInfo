#pragma once
#include "DemoHeader.h"
#include "DemoPropertyStorage.h"

#include <propkeydef.h>
#include <propsys.h>

#include <map>
#include <mutex>
#include <string>

class TestMetadataProvider final : public UnknownObject<IPropertySetStorage, IInitializeWithStream, IPropertyStore>
{
public:
	// IPropertySetStorage
	HRESULT STDMETHODCALLTYPE Create(REFFMTID rfmtid, const CLSID *pclsid, DWORD grfFlags, DWORD grfMode, IPropertyStorage **ppprstg) override;
	HRESULT STDMETHODCALLTYPE Delete(REFFMTID rfmtid) override;
	HRESULT STDMETHODCALLTYPE Enum(IEnumSTATPROPSETSTG **ppenum) override;
	HRESULT STDMETHODCALLTYPE Open(REFFMTID rfmtid, DWORD grfMode, IPropertyStorage **ppprstg) override;

	// IInitializeWithStream
	HRESULT STDMETHODCALLTYPE Initialize(IStream* pStream, DWORD grfMode) override;

	// IPropertyStore
	HRESULT STDMETHODCALLTYPE GetCount(DWORD* cProps) override;
	HRESULT STDMETHODCALLTYPE GetAt(DWORD iProp, PROPERTYKEY* pKey) override;
	HRESULT STDMETHODCALLTYPE GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) override;
	HRESULT STDMETHODCALLTYPE SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) override;
	HRESULT STDMETHODCALLTYPE Commit() override;

protected:
	IUnknown* TryGetInterface(REFIID riid) override;

private:
	IStream* m_Stream = nullptr;

	static constexpr auto SECONDS_TO_TICKS = 10000000;

	DemoHeader m_Header;

	std::map<IID, std::unique_ptr<DemoPropertyStorage, UnknownDeleter>> m_OpenedPropertyStorages;

	std::mutex m_Mutex;
	std::map<PROPERTYKEY, PropVariantSafe> m_Properties;
	static PROPERTYKEY HandleKeyAliases(const PROPERTYKEY& key);
};