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

#include <map>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <atltime.h>
#include <shlwapi.h>
#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WWizSettingsWrapper.h"
#include "WardwizLangManager.h"

using namespace std;

const CString WRDWIZAVPEDBNAME = L"WRDWIZHPE";
const CString WRDWIZAVJPEGDBNAME = L"WRDWIZHJPG";
const CString WRDWIZAVDOCDBNAME = L"WRDWIZHMAC"; //WRDWIZHDOC
const CString WRDWIZAVHTMLDBNAME = L"WRDWIZHHTML";
const CString WRDWIZAVXMLDBNAME = L"WRDWIZHXML";
const CString WRDWIZAVPDFDBNAME = L"WRDWIZHPDF";
const CString WRDWIZAVPHPDBNAME = L"WRDWIZHPHP";

//DB names changed from scan engine version 2.5
const CString AVPE32DBNAME = L"2";
const CString AVPE64DBNAME = L"4";

//WRDWIZAVDBNAME			L"WRDWIZAV2.DB"

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
CString GetWardWizPathFromRegistry( );
bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
void trim(char *s, const int len);