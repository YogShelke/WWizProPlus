
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently
#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif


#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif 

#define HANDLEUIREQUEST	 (WM_USER + 600)
//


//#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#pragma warning (disable:4996)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include "ISpyCommunicator.h"
#include "Constants.h"
#include "PipeConstants.h"
#include "TreeConstants.h"
#include "WardWizDumpCreater.h"
#include "ISpyCriticalSection.h"
#include "WWizSettingsWrapper.h"
#include "iTinEmailContants.h"
#include "ISpyDataManager.h"
#include "ITinRegWrapper.h"
#include "AVRegInfo.h"
#include "DownloadConts.h"
#include "connection.h"
#include "restclient.h"
#include <winhttp.h>
#include "SocketComm.h"
#include <cpprest/json.h>
#include <vector>
#include "WWizSettings.h"
#include "EPSConstants.h"

using namespace std;

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
}STRUCT_HEADER_INFO, *LP_STRUCT_HEADER_INFO;

typedef struct MachineInfo
{
	TCHAR		szName[MAX_PATH];
	TCHAR		szIP[MAX_PATH];
	TCHAR		szOSName[MAX_PATH];
}STRUCT_MACHINE_INFO, *LP_STRUCT_MACHINE_INFO;


bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
CString GetWardWizPathFromRegistry();

extern CWWizSettings theApp;

#define EMPTY_STRING _T("")

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

#define LOG_FILE					_T("WRDWIZCLIENTAGENT.LOG")
#define STRDATABASEFILE				".\\WRDWIZALLREPORTS.DB"

