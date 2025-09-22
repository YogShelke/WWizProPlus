// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"
#include "WardWizResource.h"
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes
#include <shlwapi.h>      


#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include "sciter-x-types.h"
#include "sciter-x-def.h"
#include "sciter-x-dom.h"
#include "sciter-x.h"
#include "sciter-x-api.h"
#include "sciter-x-host-callback.h"
#include "sciter-x-threads.h"
#include "sciter-x-request.h"
#include "value.h"
#include "tiscript.hpp"

//Ram: commented as removed dependency of crash report library
//#include "CrashThread.h"

#define		SCITER_VALUE		sciter::value
#define		SCITER_STRING		sciter::string


#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WardwizLangManager.h"
#include "WWizSettingsWrapper.h"
#include "EPSConstants.h"
#include "PipeConstants.h"

#define LOG_FILE					_T("VIBOAVTRAY.LOG")

using namespace std;

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);

bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
CString GetWardWizPathFromRegistry();
bool AddErrorServerLog(LPTSTR lpszTaskID, LPTSTR lpszTaskTypeID, LPTSTR lpszMachineIP, PWCH Message, ...);
bool AddTaskCompletedServerLog(LPTSTR lpszTaskID, LPTSTR lpszTaskTypeID, LPTSTR lpszMachineIP, PWCH Message, ...);

vector<wstring> SplitString(wstring str, wchar delimiter);

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#pragma warning(disable : 4996)

using namespace std;


#define CHECKEMAILSCANSTATUS (WM_USER + 200)
#define CHECKLIVEUPDATE (WM_USER + 300)/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */

// defines Sciter Based Class 
class CSciterBase :
	public sciter::event_handler     // Sciter DOM event handling
{

};

enum{
	WARNING_CLIENT_RESTART = 0,
	WARNING_PC_LOCK,
	WARNING_PC_INET_LOCK,
	WARNING_FIREWALL_APP_BLOCK,
	WARNING_PAR_CTRL_APP_BLOCK,
}WARNING_POPUP_TYPE;