// WardWizExports.h : Declaration of the CWardWizExports

#pragma once
#include "resource.h"       // main symbols



#include "WWizInteropeDLL_i.h"
#include "_IWardWizExportsEvents_CP.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


// CWardWizExports

class ATL_NO_VTABLE CWardWizExports :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CWardWizExports, &CLSID_WardWizExports>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CWardWizExports>,
	public CProxy_IWardWizExportsEvents<CWardWizExports>,
	public IDispatchImpl<IWardWizExports, &IID_IWardWizExports, &LIBID_WWizInteropeDLLLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CWardWizExports()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WARDWIZEXPORTS)


BEGIN_COM_MAP(CWardWizExports)
	COM_INTERFACE_ENTRY(IWardWizExports)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CWardWizExports)
	CONNECTION_POINT_ENTRY(__uuidof(_IWardWizExportsEvents))
END_CONNECTION_POINT_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:


	void PrintMD5(BYTE md5Digest[16], char *pMd5);
	STDMETHOD(FunctionGetStringHash)(CHAR* strInputString, BSTR* bstrHash);
};

OBJECT_ENTRY_AUTO(__uuidof(WardWizExports), CWardWizExports)
