// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
//#include "Constants.h"

#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
#include <shlwapi.h>      
#include "WardWizDumpCreater.h"
#include "WardwizOSVersion.h"
#include "WWizSettingsWrapper.h"

typedef enum _LOGGING_LEVEL
{
	ZEROLEVEL,
	FIRSTLEVEL,
	SECONDLEVEL,
}LOGGINGLEVEL;

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);
// TODO: reference additional headers your program requires here
