// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <afx.h>
#include <afxwin.h>
#include <Shlwapi.h>
#include <vector>
#include "Constants.h"
#include <Wbemidl.h>
#include <comutil.h>
#include <intrin.h>
#include <cfgmgr32.h>
#include <newdev.h>
#include <setupapi.h>
#include <shlobj.h>

#include "WardWizDumpCreater.h"
#include "WardwizLangManager.h"
#include "WWizSettingsWrapper.h"

#pragma comment(lib, "Wbemuuid.lib" )
#pragma comment(lib, "comsuppw.lib" )
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
CString MakeTokenisizeString(CString csFirstEntry , CString csSecondEntry  ,CString csThirdEntry = L"",CString csForthEntry = L"",CString csFifthEntry = L"",CString csSixthEntry = L"",DWORD dwSeventhEntry = 0);

CString GetWardWizPathFromRegistry( );
DWORD GetProductID();
void ErrorDescription(HRESULT hr);
