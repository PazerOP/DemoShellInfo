#pragma once
#include "DemoHeader.h"
#include "DemoPropertyStorage.h"

#include <propkeydef.h>
#include <propsys.h>
#include <ShObjIdl.h>

#include <map>
#include <mutex>
#include <string>

class TestMetadataProvider final : public UnknownObject<IPropertyStoreCapabilities, IInitializeWithStream, IPropertyStore>
{
public:
	// IInitializeWithStream
	HRESULT STDMETHODCALLTYPE Initialize(IStream* pStream, DWORD grfMode) override;

	// IPropertyStore
	HRESULT STDMETHODCALLTYPE GetCount(DWORD* cProps) override;
	HRESULT STDMETHODCALLTYPE GetAt(DWORD iProp, PROPERTYKEY* pKey) override;
	HRESULT STDMETHODCALLTYPE GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) override;
	HRESULT STDMETHODCALLTYPE SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) override;
	HRESULT STDMETHODCALLTYPE Commit() override;

	// IPropertyStoreCapabilities
	IFACEMETHODIMP IsPropertyWritable(REFPROPERTYKEY key) override;

	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;

protected:
	InterfacePair TryGetInterface(REFIID riid) override;

private:
	std::unique_ptr<IStream, UnknownDeleter> m_Stream;
	//std::unique_ptr<IDestinationStreamFactory, UnknownDeleter> m_DestFactory;
	//std::unique_ptr<IPropertyStoreCache, UnknownDeleter> m_Cache;
	//IStream* m_Stream;

	static constexpr auto SECONDS_TO_TICKS = 10000000;

	DemoHeader m_Header;

	std::mutex m_Mutex;
	std::map<PROPERTYKEY, PropVariantSafe> m_Properties;
	static PROPERTYKEY HandleKeyAliases(const PROPERTYKEY& key);
};