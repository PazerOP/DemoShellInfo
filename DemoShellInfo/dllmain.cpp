// dllmain.cpp : Defines the entry point for the DLL application.
#include <objbase.h>
#include <propkey.h>
#include <Shlobj.h>
#include <Windows.h>

#include "DemoPropertyStorage.h"
#include "UnknownObject.h"

#include <map>
#include <string>

struct UnknownDeleter
{
	void operator()(IUnknown* unknown) const
	{
		unknown->Release();
	}
};

// {FC783E08-7B78-42FD-B7F7-931E753E9658}
static constexpr GUID s_DemoShellInfoProps = { 0xfc783e08, 0x7b78, 0x42fd, { 0xb7, 0xf7, 0x93, 0x1e, 0x75, 0x3e, 0x96, 0x58 } };
enum class DemoShellInfoProps
{
	LengthTime = 150,
	LengthTicks,
	LengthFrames,

	ServerName,
	ClientName,
};

static constexpr PROPERTYKEY PKEY_Demo_LengthTime = { s_DemoShellInfoProps, (int)DemoShellInfoProps::LengthTime };
static constexpr PROPERTYKEY PKEY_Demo_LengthTicks = { s_DemoShellInfoProps, (int)DemoShellInfoProps::LengthTicks };
static constexpr PROPERTYKEY PKEY_Demo_LengthFrames = { s_DemoShellInfoProps, (int)DemoShellInfoProps::LengthFrames };

static constexpr PROPERTYKEY PKEY_Demo_ServerName = { s_DemoShellInfoProps, (int)DemoShellInfoProps::ServerName };
static constexpr PROPERTYKEY PKEY_Demo_ClientName = { s_DemoShellInfoProps, (int)DemoShellInfoProps::ClientName };

// {C6FF826A-1EF5-4526-95DC-134BCC7B3BE3}
static constexpr GUID s_DemoShellInfoGUID = { 0xc6ff826a, 0x1ef5, 0x4526, { 0x95, 0xdc, 0x13, 0x4b, 0xcc, 0x7b, 0x3b, 0xe3 } };

static constexpr auto SECONDS_TO_TICKS = 10000000;

struct DemoHeader
{
	char m_Header[8];
	static constexpr const char EXPECTED_HEADER[] = "HL2DEMO";
	static_assert(sizeof(m_Header) == std::size(EXPECTED_HEADER));

	int m_DemoProtocol;
	int m_NetworkProtocol;

	char m_ServerName[260];
	char m_ClientName[260];
	char m_MapName[260];
	char m_GameDirectory[260];

	float m_PlaybackTime;
	int m_Ticks;
	int m_Frames;
	int m_SignonLength;
};

class TestMetadataProvider final : public UnknownObject<IPropertySetStorage, IInitializeWithStream, IPropertyStore>
{
public:
	// IPropertySetStorage
	HRESULT STDMETHODCALLTYPE Create(REFFMTID rfmtid, const CLSID *pclsid, DWORD grfFlags, DWORD grfMode, IPropertyStorage **ppprstg) override
	{
		return E_UNEXPECTED;
	}
	HRESULT STDMETHODCALLTYPE Delete(REFFMTID rfmtid) override
	{
		return E_UNEXPECTED;
	}
	HRESULT STDMETHODCALLTYPE Enum(IEnumSTATPROPSETSTG **ppenum) override
	{
		return E_UNEXPECTED;
	}
	HRESULT STDMETHODCALLTYPE Open(REFFMTID rfmtid, DWORD grfMode, IPropertyStorage **ppprstg) override
	{
		if (rfmtid == PKEY_Media_Duration.fmtid || rfmtid == PKEY_Calendar_Location.fmtid)
		{
			*ppprstg = new DemoPropertyStorage(rfmtid);
			return S_OK;
		}
		if (rfmtid == PKEY_AppUserModel_ID.fmtid)
			return E_UNEXPECTED;

		return E_UNEXPECTED;
	}

	// IInitializeWithStream
	HRESULT STDMETHODCALLTYPE Initialize(IStream* pStream, DWORD grfMode) override
	{
		STATSTG stats;
		if (auto hr = pStream->Stat(&stats, STATFLAG_NONAME); hr != S_OK)
			return hr;

		if (auto hr = pStream->Seek(CreateLargeInteger(0), STREAM_SEEK_SET, nullptr); hr != S_OK)
			return hr;

		ULONG headerRead;
		DemoHeader header;
		if (auto hr = pStream->Read(&header, sizeof(DemoHeader), &headerRead); hr != S_OK)
			return hr;

		if (memcmp(header.m_Header, header.EXPECTED_HEADER, sizeof(header.EXPECTED_HEADER)))
			return E_INVALID_PROTOCOL_FORMAT;  // Invalid/corrupted .dem file

		InitPropVariantFromUInt64(double(header.m_PlaybackTime) * SECONDS_TO_TICKS, &m_Properties[PKEY_Demo_LengthTime]);
		InitPropVariantFromInt32(header.m_Ticks, &m_Properties[PKEY_Demo_LengthTicks]);
		InitPropVariantFromInt32(header.m_Frames, &m_Properties[PKEY_Demo_LengthFrames]);

		InitPropVariantFromString(header.m_ClientName, &m_Properties[PKEY_Demo_ClientName]);

		return S_OK;
	}

	// IPropertyStore
	HRESULT STDMETHODCALLTYPE GetCount(DWORD* cProps) override
	{
		if (!cProps)
			return E_POINTER;

		*cProps = m_Properties.size();

		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetAt(DWORD iProp, PROPERTYKEY* pKey) override
	{
		if (!pKey)
			return E_POINTER;
		if (iProp > m_Properties.size())
			return E_INVALIDARG;

		auto it = m_Properties.begin();
		std::advance(it, iProp);
		*pKey = it->first;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) override
	{
		if (!pv)
			return E_POINTER;

		*pv = m_Properties.at(key);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) override
	{
		m_Properties[key] = propvar;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE Commit() override
	{
		return S_OK;
	}

protected:
	IUnknown* TryGetInterface(REFIID riid) override
	{
		if (riid == __uuidof(IPropertySetStorage))
			return static_cast<IPropertySetStorage*>(this);
		if (riid == __uuidof(IInitializeWithStream))
			return static_cast<IInitializeWithStream*>(this);
		//if (riid == __uuidof(IPropertyStore))
		//	return static_cast<IPropertyStore*>(this);

		return nullptr;
	}

private:
	std::map<PROPERTYKEY, PROPVARIANT> m_Properties;
};
static TestMetadataProvider s_TestColumnProvider;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

class ClassFactory : public UnknownObject<IClassFactory>
{
public:
	HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject) override
	{
		if (!ppvObject)
			return E_POINTER;

		if (riid == __uuidof(IPropertySetStorage))
		{
			*ppvObject = new TestMetadataProvider();
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	HRESULT LockServer(BOOL fLock) override
	{
		if (fLock)
			++m_ServerLockCount;
		else
			--m_ServerLockCount;

		return S_OK;
	}

protected:
	IUnknown* TryGetInterface(REFIID riid) override
	{
		if (riid == __uuidof(IClassFactory))
			return this;

		return nullptr;
	}

	std::atomic<int> m_ServerLockCount;
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	std::cerr << "PazerFromSilver Test";

	if (!ppv)
		return E_POINTER;

	if (riid == __uuidof(IClassFactory))
	{
		*ppv = new ClassFactory();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI DllCanUnloadNow()
{
	return S_OK;
}