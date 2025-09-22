
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif 

#define HANDLEUIREQUEST	 (WM_USER + 600)

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#pragma warning (disable:4996)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
//#include <atlstr.h>

#include "ISpyCommunicator.h"
#include "Constants.h"
#include "PipeConstants.h"
#include "TreeConstants.h"
#include "ITinRegWrapper.h"
#include "WardWizDumpCreater.h"
#include "ISpyCriticalSection.h"
#include "WWizSettingsWrapper.h"
#include "iTinEmailContants.h"
#include "ISpyDataManager.h"
#include "WardwizLangManager.h"
//alu
#include "DownloadConts.h"
#include "ZipArchive.h"
#include <winhttp.h>
#include "WWizSettings.h"

//============= Sciter header files ===============
#include "sciter-x.h"  // sciter headers
#include "sciter-x-api.h"
#include "sciter-x-dom.hpp"
#include "sciter-x-host-callback.h"
//=================================================

#define		SCITER_VALUE		sciter::value
#define		SCITER_STRING		sciter::string


#define TIMER_SCAN_STATUS			200

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars


typedef struct HeaderInfo
{
	DWORD		dwFileSize;
	HINTERNET	hSession;
	HINTERNET	hConnect;
	HINTERNET	hRequest;
	TCHAR		szHostName[MAX_PATH];
	TCHAR		szMainUrl[URL_SIZE];
	TCHAR		szBinaryName[MAX_PATH];
	TCHAR		szETag[MAX_PATH];
}STRUCT_HEADER_INFO, *LP_STRUCT_HEADER_INFO; //alu

bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
CString GetWardWizPathFromRegistry();
CString GetAVPathFromRegistry(LPTSTR lpszAVPath = NULL); //alu
//void ErrorDescription(HRESULT hr); //alu


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


#define MAX_FILE_PATH 260

#define LOG_FILE					_T("VBUPDATE.LOG")
#define STRDATABASEFILE				".\\VBALLREPORTS.DB"
