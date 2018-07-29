#include "DemoPropertyStorage.h"
#include "MetadataProvider.h"

#include <objbase.h>
#include <propkey.h>
#include <Shlobj.h>
#include <strsafe.h>
#include <Windows.h>

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

// IInitializeWithStream
HRESULT STDMETHODCALLTYPE TestMetadataProvider::Initialize(IStream* pStream, DWORD grfMode)
{
	LOCK_GUARD(m_Mutex);

	m_Stream = pStream;

	STATSTG stats;
	if (auto hr = pStream->Stat(&stats, STATFLAG_NONAME); hr != S_OK)
		return hr;

	if (auto hr = pStream->Seek(CreateLargeInteger(0), STREAM_SEEK_SET, nullptr); hr != S_OK)
		return hr;

	ULONG headerRead;
	if (auto hr = pStream->Read(&m_Header, sizeof(m_Header), &headerRead); hr != S_OK)
		return hr;

	if (memcmp(m_Header.m_Header, m_Header.EXPECTED_HEADER, sizeof(m_Header.EXPECTED_HEADER)))
		return E_INVALID_PROTOCOL_FORMAT;  // Invalid/corrupted .dem file

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

	return S_OK;
}

// IPropertyStore
HRESULT STDMETHODCALLTYPE TestMetadataProvider::GetCount(DWORD* cProps)
{
	if (!cProps)
		return E_POINTER;

	LOCK_GUARD(m_Mutex);

	*cProps = DWORD(m_Properties.size());

	return S_OK;
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::GetAt(DWORD iProp, PROPERTYKEY* pKey)
{
	if (!pKey)
		return E_POINTER;

	LOCK_GUARD(m_Mutex);

	if (iProp > m_Properties.size())
		return E_INVALIDARG;

	auto it = m_Properties.begin();
	std::advance(it, iProp);
	*pKey = it->first;
	return S_OK;
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
	if (!pv)
		return E_POINTER;

	LOCK_GUARD(m_Mutex);

	if (auto found = m_Properties.find(HandleKeyAliases(key)); found != m_Properties.end())
	{
		*pv = found->second.Get();
		return S_OK;
	}
	else
		return E_NOT_SET;
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
	LOCK_GUARD(m_Mutex);

	m_Properties[key].Set(propvar);
	return S_OK;
}
HRESULT STDMETHODCALLTYPE TestMetadataProvider::Commit()
{
	// Currently, the only thing we're allowing users to change is the client name
	wchar_t wBuf[DemoHeader::STRING_LENGTHS];
	if (auto hr = PropVariantToString(m_Properties.at(PKEY_Demo_ClientName).Get(), wBuf, UINT(std::size(wBuf))))
	{
		if (hr != STRSAFE_E_INSUFFICIENT_BUFFER) // We're willing to accept truncation
			return hr;
	}

	wcstombs_s(nullptr, m_Header.m_ClientName, wBuf, DemoHeader::STRING_LENGTHS - 1);

	if (auto hr = m_Stream->Seek(CreateLargeInteger(0), STREAM_SEEK_SET, nullptr))
		return hr;

	if (auto hr = m_Stream->Write(&m_Header, sizeof(m_Header), nullptr))
		return hr;

	m_Stream = nullptr;
	return S_OK;
}

IUnknown* TestMetadataProvider::TryGetInterface(REFIID riid)
{
	if (riid == __uuidof(IPropertySetStorage))
		return static_cast<IPropertySetStorage*>(this);
	if (riid == __uuidof(IInitializeWithStream))
		return static_cast<IInitializeWithStream*>(this);
	//if (riid == __uuidof(IPropertyStore))
	//	return static_cast<IPropertyStore*>(this);

	return nullptr;
}

PROPERTYKEY TestMetadataProvider::HandleKeyAliases(const PROPERTYKEY& key)
{
	if (key == PKEY_Media_Duration)
		return PKEY_Demo_LengthTime;
	if (key == PKEY_Calendar_Location)
		return PKEY_Demo_MapName;

	return key;
}