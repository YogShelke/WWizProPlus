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

#include <string>

#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WWizSettingsWrapper.h"
#include "WardwizLangManager.h"
#include "iTINRegWrapper.h"

using namespace std;

#define LOG_FILE					_T("VIBOCOMSRV.LOG")

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);

//Add log to specified file path - vilas
void AddLogEntryWithPath(const TCHAR *lpLogFilePath, const TCHAR *sFormatString, const TCHAR *sEntry1, const TCHAR *sEntry2, bool isDateTime, DWORD dwLogLevel);

DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel); bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
CString MakeTokenisizeString(CString csFirstEntry, CString csSecondEntry, CString csThirdEntry = L"", CString csForthEntry = L"", CString csFifthEntry = L"", CString csSixthEntry = L"", DWORD dwSeventhEntry = 0);

CString GetWardWizPathFromRegistry();
bool GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt);
DWORD GetProductID();
void ErrorDescription(HRESULT hr);
std::wstring String2WString(const std::string& strToConvert);
void AddLogEntryEx(DWORD dwLogLevel, IN PWCH Message, ...);

