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

protected:
	InterfacePair TryGetInterface(REFIID riid) override;

private:
	UnknownPtr<IStream> m_Stream;
	UnknownPtr<IPropertyStoreCache> m_Cache;
	bool m_Initialized = false;
	//IStream* m_Stream = nullptr;
	//std::unique_ptr<IDestinationStreamFactory, UnknownDeleter> m_DestFactory;
	//IStream* m_Stream;

	DWORD m_Mode = 0;

	static constexpr auto SECONDS_TO_TICKS = 10000000;

	DemoHeader m_Header;

	template<typename T, typename InitFunc> HRESULT StoreIntoCache(const T& value, InitFunc func, const PROPERTYKEY& key);

	std::recursive_mutex m_Mutex;
	//std::map<PROPERTYKEY, PropVariantSafe> m_Properties;
	static PROPERTYKEY HandleKeyAliases(const PROPERTYKEY& key);
};