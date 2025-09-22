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

#include <shlwapi.h>
#include "PipeConstants.h"
#include "WardWizResource.h"
#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WardwizLangManager.h"
#include "WWizSettingsWrapper.h"

//============= Sciter header files ===============
#include "sciter-x.h"  // sciter headers
#include "sciter-x-api.h"
#include "sciter-x-dom.hpp"
#include "sciter-x-host-callback.h"
//=================================================

#define		SCITER_VALUE		sciter::value
#define		SCITER_STRING		sciter::string


//#if !defined(_WINHTTPX_)
#undef _WININET_
//#endif // !defined(_WININET_)

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);

bool	GetModulePath(TCHAR *szModulePath, DWORD dwSize);
CString GetWardWizPathFromRegistry( );
void	ErrorDescription(HRESULT hr);

typedef enum {
	MACHINEIDMISMATCH		= 0x05,
	INVALIDEMAILID			= 0x06,
	COUNTRYCODEINVALID		= 0x07,
	INVALIDREGNUMBER		= 0x08,
	INVALIDPRODVERSION		= 0x09,
	USERINFOUPDATEFAILD		= 0x10,
	PRODUCTEXPIRED			= 0x11,
	NULLRESPONSE			= 0x12,
	FOUNDPIRACY				= 0x13,
}REGFAILUREMSGS;
