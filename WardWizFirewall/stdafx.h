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

#pragma warning(disable : 4005)
#pragma warning(disable : 4996)

#include <sal.h>

extern "C"
{
	#include "ptstatus.h"
}

#define	EMPTY_STRING	_T("")

#include <ntstatus.h>
#include <winsock2.h>
#include <stdio.h>

typedef long NTSTATUS;

#define WIN32_NO_STATUS

#include "ignis_dll.h"
#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WWizSettingsWrapper.h"
#include "WardwizLangManager.h"
#include "iTINRegWrapper.h"
#include "PipeConstants.h"

using namespace std;

#define UI_FLAG_BLOCK       0x00000000 // 0 as intended
#define UI_FLAG_REMEMBER    0x00000004
#define UI_FLAG_BY_PATH     0x00000008
#define UI_FLAG_BY_PROTO    0x00000010
#define UI_FLAG_BY_PORT     0x00000020
#define UI_FLAG_BY_IP       0x00000040
#define UI_FLAG_BY_MD5      0x00000080
#define UI_FLAG_BIDIRECTIONAL 0x00000100
#define UI_FLAG_ALLOW       0x01000000
#define UI_FLAG_NOTIFY      0x02000000

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
