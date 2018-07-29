#include "ClassFactory.h"
#include "MetadataProvider.h"

#include <mutex>

class ClassFactory : public UnknownObject<IClassFactory>
{
public:
	ClassFactory();
	~ClassFactory();

	// IClassFactory
	HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject) override;
	HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) override;

	bool IsLocked() { return m_ServerLockCount > 0; }

protected:
	IUnknown * TryGetInterface(REFIID riid) override;

	std::atomic<int> m_ServerLockCount;
};

ClassFactory::ClassFactory() : m_ServerLockCount(0)
{
	LOCK_GUARD(s_ClassFactoryLifetimeMutex);
}
ClassFactory::~ClassFactory()
{
	LOCK_GUARD(s_ClassFactoryLifetimeMutex);

	assert(m_ServerLockCount == 0);
}

HRESULT STDMETHODCALLTYPE ClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppvObject)
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

HRESULT STDMETHODCALLTYPE ClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
		++m_ServerLockCount;
	else
		--m_ServerLockCount;

	return S_OK;
}

IUnknown* ClassFactory::TryGetInterface(REFIID riid)
{
	if (riid == __uuidof(IClassFactory))
		return this;

	return nullptr;
}

IClassFactory* CreateClassFactory()
{
	return new ClassFactory();
}
