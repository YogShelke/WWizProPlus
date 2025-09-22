// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently
#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

//#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#pragma warning (disable:4996)

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

#include "ISpyCommunicator.h"
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
#include <cpprest/json.h>
#include <winhttp.h>
#include "SocketComm.h"
#include <vector>
#include "WWizSettings.h"
#include "EPSConstants.h"

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


//******************************************************************************
//Encryption constants
const int			WRDWIZ_KEYSIZE = 4;
const unsigned char WRDWIZ_KEY[WRDWIZ_KEYSIZE + 1] = "WWIZ";
const unsigned int WRDWIZ_KEY_SIZE = 0x10;
const unsigned int WRDWIZ_SIG_SIZE = 0x07;
const unsigned char WRDWIZ_SIG[WRDWIZ_SIG_SIZE + 1] = "WARDWIZ";
const unsigned char WRDWIZ_SIG_NEW[WRDWIZ_SIG_SIZE + 1] = "WRDWIZE";

#define		MAX_FILEPATH_LENGTH			MAX_PATH * 4

enum
{
	ESSENTIAL = 1,
	PRO,
	ELITE,
	BASIC,
};

typedef enum _LOGGING_LEVEL
{
	ZEROLEVEL,
	FIRSTLEVEL,
	SECONDLEVEL,
}LOGGINGLEVEL;

bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
CString GetWardWizPathFromRegistry();
bool AddErrorServerLog(LPTSTR lpszTaskID, LPTSTR lpszRemoteIP, PWCH Message, ...);
bool SendFailureLog(LPTSTR lpszTaskID, LPTSTR lpszRemoteIP, PWCH Message, ...);

CString GetWardWizPathFromRegistry();

//extern CWWizSettings theApp;

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


