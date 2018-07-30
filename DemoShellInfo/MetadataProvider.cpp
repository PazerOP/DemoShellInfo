#include "DemoPropertyStorage.h"
#include "MetadataProvider.h"

#include <objbase.h>
#include <propkey.h>
#include <Shlobj.h>
#include <strsafe.h>
#include <Windows.h>

#pragma warning(disable : 4189)

// {FC783E08-7B78-42FD-B7F7-931E753E9658}
static constexpr GUID s_DemoShellInfoProps = { 0xfc783e08, 0x7b78, 0x42fd, { 0xb7, 0xf7, 0x93, 0x1e, 0x75, 0x3e, 0x96, 0x58 } };
enum class DemoProps
{
	LengthTime = 150,
	LengthTicks,
	LengthFrames,

	ServerName,
	ClientName,
	MapName,
	GameDirectory,
};

static constexpr PROPERTYKEY PKEY_Demo_LengthTime = CreatePropertyKey(s_DemoShellInfoProps, (int)DemoProps::LengthTime);
static constexpr PROPERTYKEY PKEY_Demo_LengthTicks = CreatePropertyKey(s_DemoShellInfoProps, (int)DemoProps::LengthTicks);
static constexpr PROPERTYKEY PKEY_Demo_LengthFrames = CreatePropertyKey(s_DemoShellInfoProps, (int)DemoProps::LengthFrames);

static constexpr PROPERTYKEY PKEY_Demo_ServerName = CreatePropertyKey(s_DemoShellInfoProps, (int)DemoProps::ServerName);
static constexpr PROPERTYKEY PKEY_Demo_ClientName = CreatePropertyKey(s_DemoShellInfoProps, (int)DemoProps::ClientName);
static constexpr PROPERTYKEY PKEY_Demo_MapName = CreatePropertyKey(s_DemoShellInfoProps, (int)DemoProps::MapName);
static constexpr PROPERTYKEY PKEY_Demo_GameDirectory = CreatePropertyKey(s_DemoShellInfoProps, (int)DemoProps::GameDirectory);

#if false
// IPropertySetStorage
HRESULT STDMETHODCALLTYPE TestMetadataProvider::Create(REFFMTID rfmtid, const CLSID *pclsid, DWORD grfFlags, DWORD grfMode, IPropertyStorage **ppprstg)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::Delete(REFFMTID rfmtid)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::Enum(IEnumSTATPROPSETSTG **ppenum)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::Open(REFFMTID rfmtid, DWORD grfMode, IPropertyStorage **ppprstg)
{
	if (!ppprstg)
		return E_POINTER;

	LOCK_GUARD(m_Mutex);

	if (rfmtid == PKEY_Media_Duration.fmtid ||
		rfmtid == PKEY_Calendar_Location.fmtid ||
		rfmtid == s_DemoShellInfoProps)
	{
		auto& found = m_OpenedPropertyStorages[rfmtid];
		if (!found)
		{
			found.reset(new DemoPropertyStorage(rfmtid, this, PROPSETFLAG_NONSIMPLE));

			if (rfmtid == s_DemoShellInfoProps)
				found->SetClass(s_DemoShellInfoCLSID);
		}

		found->AddRef();
		*ppprstg = found.get();

		return S_OK;
	}

	return E_UNEXPECTED;
}
#endif

template<typename T, typename InitFunc> HRESULT TestMetadataProvider::StoreIntoCache(const T& value, InitFunc func, const PROPERTYKEY& key)
{
	PropVariantSafe variant;

	if (auto hr = func(value, &variant.Get()))
		return hr;
	if (auto hr = m_Cache->SetValueAndState(key, &variant.Get(), PSC_NORMAL))
		return hr;

	return S_OK;
}

// IInitializeWithStream
HRESULT STDMETHODCALLTYPE TestMetadataProvider::Initialize(IStream* pStream, DWORD grfMode)
{
	OutputDebugStringA(__FUNCSIG__ "\n");

	LOCK_GUARD(m_Mutex);

	if (m_Initialized)
		return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
	else
		m_Initialized = true;

	STATSTG stats;
	if (auto hr = pStream->Stat(&stats, STATFLAG_DEFAULT); hr != S_OK)
		return hr;

	if ((grfMode & STGM_READWRITE) != (stats.grfMode & STGM_READWRITE))
		return STG_E_ACCESSDENIED;

	// Grab a reference to the stream
	//if (!(grfMode & STGM_READWRITE))
	{
		if (grfMode & STGM_READWRITE)
		{
			m_Stream.reset(pStream);
			const auto refCount = m_Stream->AddRef();
			assert(refCount > 1);
		}
		//IStream* stream;
		//if (auto hr = pStream->QueryInterface(&stream))
		//	return hr;

		//auto str = stream->Release();
		//m_Stream.reset(stream);
	}

	m_Mode = grfMode;

	// Initialize cache
	{
		IPropertyStoreCache* cache;
		if (auto hr = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&cache)))
			return hr;

		m_Cache.reset(cache);
	}

#if true
	if (auto hr = pStream->Seek(CreateLargeInteger(0), STREAM_SEEK_SET, nullptr); hr != S_OK)
		return hr;

	ULONG headerRead;
	if (auto hr = pStream->Read(&m_Header, sizeof(m_Header), &headerRead); hr != S_OK)
		return hr;

	if (memcmp(m_Header.m_Header, m_Header.EXPECTED_HEADER, sizeof(m_Header.EXPECTED_HEADER)))
		return E_INVALID_PROTOCOL_FORMAT;  // Invalid/corrupted .dem file

#if false
	InitPropVariantFromUInt64(ULONGLONG(double(m_Header.m_PlaybackTime) * SECONDS_TO_TICKS), &m_Properties[PKEY_Demo_LengthTime].Get());
	InitPropVariantFromInt32(m_Header.m_Ticks, &m_Properties[PKEY_Demo_LengthTicks].Get());
	InitPropVariantFromInt32(m_Header.m_Frames, &m_Properties[PKEY_Demo_LengthFrames].Get());

	wchar_t buf[DemoHeader::STRING_LENGTHS];
	mbstowcs_s(nullptr, buf, m_Header.m_ClientName, std::size(buf) - 1);
	InitPropVariantFromString(buf, &m_Properties[PKEY_Demo_ClientName].Get());
	mbstowcs_s(nullptr, buf, m_Header.m_ServerName, std::size(buf) - 1);
	InitPropVariantFromString(buf, &m_Properties[PKEY_Demo_ServerName].Get());
	mbstowcs_s(nullptr, buf, m_Header.m_MapName, std::size(buf) - 1);
	InitPropVariantFromString(buf, &m_Properties[PKEY_Demo_MapName].Get());
	mbstowcs_s(nullptr, buf, m_Header.m_GameDirectory, std::size(buf) - 1);
	InitPropVariantFromString(buf, &m_Properties[PKEY_Demo_GameDirectory].Get());
#else
	if (auto hr = StoreIntoCache(ULONGLONG(double(m_Header.m_PlaybackTime) * SECONDS_TO_TICKS), InitPropVariantFromUInt64, PKEY_Demo_LengthTime))
		return hr;
	if (auto hr = StoreIntoCache(m_Header.m_Ticks, InitPropVariantFromInt32, PKEY_Demo_LengthTicks))
		return hr;
	if (auto hr = StoreIntoCache(m_Header.m_Frames, InitPropVariantFromInt32, PKEY_Demo_LengthFrames))
		return hr;

	wchar_t buf[DemoHeader::STRING_LENGTHS];
	mbstowcs_s(nullptr, buf, m_Header.m_ClientName, std::size(buf) - 1);
	if (auto hr = StoreIntoCache(buf, InitPropVariantFromString, PKEY_Demo_ClientName))
		return hr;

	mbstowcs_s(nullptr, buf, m_Header.m_ServerName, std::size(buf) - 1);
	if (auto hr = StoreIntoCache(buf, InitPropVariantFromString, PKEY_Demo_ServerName))
		return hr;

	mbstowcs_s(nullptr, buf, m_Header.m_MapName, std::size(buf) - 1);
	if (auto hr = StoreIntoCache(buf, InitPropVariantFromString, PKEY_Demo_MapName))
		return hr;

	mbstowcs_s(nullptr, buf, m_Header.m_GameDirectory, std::size(buf) - 1);
	if (auto hr = StoreIntoCache(buf, InitPropVariantFromString, PKEY_Demo_GameDirectory))
		return hr;

#endif
#endif

	return S_OK;
}

// IPropertyStore
HRESULT STDMETHODCALLTYPE TestMetadataProvider::GetCount(DWORD* cProps)
{
	return m_Cache->GetCount(cProps);
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::GetAt(DWORD iProp, PROPERTYKEY* pKey)
{
	return m_Cache->GetAt(iProp, pKey);
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
	return m_Cache->GetValue(HandleKeyAliases(key), pv);
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
	return m_Cache->SetValueAndState(key, &propvar, PSC_DIRTY);
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::Commit()
{
	OutputDebugStringA(__FUNCSIG__ "\n");

	LOCK_GUARD(m_Mutex);

	if (!m_Stream)
		return E_NOTIMPL;

	auto refCount = m_Stream->AddRef();
	m_Stream->Release();

	PropVariantSafe variant;
	PSC_STATE state;
	if (auto hr = m_Cache->GetValueAndState(PKEY_Demo_ClientName, &variant.Get(), &state))
		return hr;

	if (state != PSC_DIRTY)
		return S_OK;  // Nothing to modify here

	// Currently, the only thing we're allowing users to change is the client name
	bool truncated = false;
	wchar_t wBuf[DemoHeader::STRING_LENGTHS];
	if (auto hr = PropVariantToString(variant.Get(), wBuf, UINT(std::size(wBuf))))
	{
		if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) // We're willing to accept truncation
			truncated = true;
		else
			return hr;
	}

	wcstombs_s(nullptr, m_Header.m_ClientName, wBuf, DemoHeader::STRING_LENGTHS - 1);

	if (auto hr = m_Stream->Seek(CreateLargeInteger(0), STREAM_SEEK_SET, nullptr))
		return hr;

	if (auto hr = m_Stream->Write(&m_Header, sizeof(m_Header), nullptr))
		return hr;

	if (auto hr = m_Stream->Commit(STGC_DEFAULT))
		return hr;

	//m_Stream->Release();
	//m_Stream = nullptr;
	m_Stream.reset();

	return truncated ? INPLACE_S_TRUNCATED : S_OK;
}

IFACEMETHODIMP TestMetadataProvider::IsPropertyWritable(REFPROPERTYKEY key)
{
	OutputDebugStringA(__FUNCSIG__ "\n");

	if (key == PKEY_Demo_ClientName)
		return S_OK;

	return S_FALSE;
}

TestMetadataProvider::InterfacePair TestMetadataProvider::TryGetInterface(REFIID riid)
{
	if (riid == __uuidof(IUnknown))
		return GetInterface<IUnknown>(static_cast<IPropertyStore*>(this));
	if (riid == __uuidof(IPropertyStore))
		return GetInterface<IPropertyStore>(this);
	if (riid == __uuidof(IPropertyStoreCapabilities))
		return GetInterface<IPropertyStoreCapabilities>(this);
	if (riid == __uuidof(IInitializeWithStream))
		return GetInterface<IInitializeWithStream>(this);

	return NO_INTERFACE;
}

PROPERTYKEY TestMetadataProvider::HandleKeyAliases(const PROPERTYKEY& key)
{
	if (key == PKEY_Media_Duration)
		return PKEY_Demo_LengthTime;
	if (key == PKEY_Calendar_Location)
		return PKEY_Demo_MapName;

	return key;
}