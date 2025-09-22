
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WardwizLangManager.h"
#include "WWizSettingsWrapper.h"

#pragma warning ( disable : 4995)

//============= Sciter header files ===============
#include "sciter-x.h"  // sciter headers
#include "sciter-x-api.h"
#include "sciter-x-dom.hpp"
#include "sciter-x-host-callback.h"
//=================================================

#define		SCITER_VALUE		sciter::value
#define		SCITER_STRING		sciter::string


DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);
CString GetWardWizPathFromRegistry();
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
