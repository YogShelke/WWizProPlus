/**********************************************************************************************************      		Program Name          : OutlookAddinApp.cpp
	Description           : This is an application class.
	Author Name			  : Ramkrushna Shelke                                                                  	  
	Date Of Creation      : 20th Aug 2014
	Version No            : 1.0.0.8
	Special Logic Used    : 
	Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "OutlookAddin.h"
#include "OutlookAddinApp.h"
#include "dlldatax.h"
#include "WardwizLangManager.h"
#include "OutlookAddin_i.c"
#include "Addin.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

typedef DWORD (*GETDAYSLEFT)		(DWORD) ;
typedef bool (*PERFORMREGISTRATION)	() ;

GETDAYSLEFT			GetDaysLeft			= NULL ;
PERFORMREGISTRATION	PerformRegistration	= NULL ;

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Addin, CAddin)
END_OBJECT_MAP()

BEGIN_MESSAGE_MAP(COutlookAddinApp, CWinApp)
	//{{AFX_MSG_MAP(COutlookAddinApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

COutlookAddinApp theApp;

/***************************************************************************
  Function Name  : COutlookAddinApp
  Description    : Cont'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0090
  Date           : 07th Aug 2014
****************************************************************************/
COutlookAddinApp::COutlookAddinApp():
		m_dwDaysLeft(0x0000)
	,	m_hRegistrationDLL(NULL)
{
}

/***************************************************************************
  Function Name  : ~COutlookAddinApp
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0091
  Date           : 07th Aug 2014
****************************************************************************/
COutlookAddinApp::~COutlookAddinApp()
{
}

/***************************************************************************
  Function Name  : InitInstance
  Description    : InitInstance overridable function 
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0092
  Date           : 07th Aug 2014
****************************************************************************/
BOOL COutlookAddinApp::InitInstance()
{
#ifdef _MERGE_PROXYSTUB
    hProxyDll = m_hInstance;
#endif

    _Module.Init(ObjectMap, m_hInstance, &LIBID_OUTLOOKADDINLib);

	m_dwSelectedLangID = m_objwardwizLangManager.GetSelectedLanguage();
	m_dwProductID = m_objwardwizLangManager.GetSelectedProductID();

	return CWinApp::InitInstance();
}

/***************************************************************************
  Function Name  : ExitInstance
  Description    : ExitInstance overridable function 
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0093
  Date           : 07th Aug 2014
****************************************************************************/
int COutlookAddinApp::ExitInstance()
{
    _Module.Term();
    return CWinApp::ExitInstance();
}

/***************************************************************************
  Function Name  : UnloadRegistrationDLL
  Description    : Function which unloads registration DLL
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0094
  Date           : 07th Aug 2014
****************************************************************************/
bool COutlookAddinApp::UnloadRegistrationDLL( )
{
	if( m_hRegistrationDLL != NULL)
	{
		FreeLibrary( m_hRegistrationDLL ) ;
		m_hRegistrationDLL = NULL;
	}
	return true;
}

/***************************************************************************
  Function Name  : GetProductSettings
  Description    : Function which get the settings from productsetting.ini file.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0095
  Date           : 07th Aug 2014
****************************************************************************/
bool COutlookAddinApp::GetProductSettings( )
{
	CWardwizLangManager objWardwizLangManager;
	m_dwSelectedLangID = objWardwizLangManager.GetSelectedLanguage();
	m_dwProductID = objWardwizLangManager.GetSelectedProductID();
	return false;
}

/***************************************************************************
  Function Name  : GetProductSettings
  Description    : Function which verifies that the product is registered or not.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0096
  Date           : 07th Aug 2014
****************************************************************************/
DWORD COutlookAddinApp::CheckiSPYAVRegistered( )
{
	DWORD	dwRet = 0x00 ;

	CString	striSPYAVPath(L"");
	striSPYAVPath = GetWardWizPathFromRegistry() ;

	CString	strRegistrationDLL = L"" ;
#ifdef MSOUTLOOK32
	strRegistrationDLL.Format( L"%sWRDWIZREGISTRATION32.DLL", striSPYAVPath ) ;
#else
	strRegistrationDLL.Format( L"%sVBREGISTRATION.DLL", striSPYAVPath ) ;
#endif

	if( !PathFileExists( strRegistrationDLL ) )
	{
		MessageBox(NULL, m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_NOT_FOUND"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	}

	if( !m_hRegistrationDLL )
	{
		m_hRegistrationDLL = LoadLibrary( strRegistrationDLL ) ;
	}

	if(!m_hRegistrationDLL)
	{
		MessageBox(NULL, m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_LOAD_FAILED"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		return dwRet;
	}

	GetDaysLeft	= (GETDAYSLEFT ) GetProcAddress( m_hRegistrationDLL, "GetDaysLeft" ) ;

	if( !GetDaysLeft )
	{
		MessageBox(NULL, m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_GET_DAYS_LEFT_NOT_FOUND"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	}

	if(GetDaysLeft != NULL)
	{
		m_dwDaysLeft = GetDaysLeft( m_dwProductID ) ;
	}
	return dwRet ;
}

/***************************************************************************
  Function Name  : ShowEvaluationExpiredMsg
  Description    : Function which Shows product expired message box
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0097
  Date           : 07th Aug 2014
****************************************************************************/
void COutlookAddinApp::ShowEvaluationExpiredMsg()
{
	MessageBox(NULL, m_objwardwizLangManager.GetString(L"IDS_EVALUATION_EXPIRED"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
}


/***************************************************************************
  Function Name  : ReloadNoofdaysLeft
  Description    : Function which reloads the days left.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0098
  Date           : 07th Aug 2014
****************************************************************************/
void COutlookAddinApp::ReloadNoofdaysLeft()
{
	m_dwDaysLeft = GetDaysLeft( m_dwProductID ) ;
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    return _Module.UnregisterServer(TRUE);
}

