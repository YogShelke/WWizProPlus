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

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include "resource.h"
#include <shlwapi.h>      
#include <string>
#include <stdlib.h>
#include <afxmt.h>			//To use thread synchronization classes
#include <algorithm>
#include "JpegDialog.h"
#include "DownloadConts.h"
#include "ISpyCommunicator.h"
#include "Constants.h"
#include "PipeConstants.h"
#include "TreeConstants.h"
#include "WardWizResource.h"
//#include "CrashThread.h"   //Ram: Commented as removed depedency of Crash report library
#include "WardWizDumpCreater.h"

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <shlwapi.h>  
#include <afxcontrolbars.h>

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);

bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
bool ExtractTokenString(CString csCombineString, CStringArray &csaStringsList);	

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

#define MAX_FILE_PATH 260

#define LOG_FILE					_T("WRDWIZAVUI.LOG")

//typedef enum ___SCAN_TYPE
//{
//	FULLSCAN,
//	CUSTOMSCAN,
//	QUICKSCAN,
//	USBSCAN,
//	USBDETECT
//}SCANTYPE;
//  
//typedef enum ___ACTION
//{
//	DETECTED,
//	DELETED,
//	REPAIRED,
//};
typedef struct ___ISPYREPORTS_DB
{
	TCHAR szDate[MAX_PATH];
	TCHAR szDateTime[MAX_PATH];
	TCHAR szScanType[MAX_PATH];
	TCHAR szThreatName[MAX_PATH];
	TCHAR szFilePath[MAX_PATH];
	TCHAR szAction[MAX_PATH];
}ISPYREPORTSDB;


/*===========================================================*/
/*			CONSTANTS										 */
/*===========================================================*/
const int BLUE_COLOUR = 1;
const int WHITE_COLOUR = 2; 
const int RED_COLOUR = 3; 
const COLORREF BLACK = RGB(0,0,0); 
const COLORREF WHITE = RGB(255,255,255); 
/*===========================================================*/
/*				CONSTANT STRINGS							*/
/*===========================================================*/
#define SZDETECTED		L"Threat(s) Blocked"
#define SZNOTHREAT		L"No threat found"
#define SZREPAIRED		L"Repaired"
#define SZQUARANTINED	L"Quarantined"
#define SZNOTHREATS		L"No threat(s) found"
/*===========================================================*/

/*===========================================================*/
/*				USER DEFINED MESSAGESS						*/
/*===========================================================*/
#define RELOAD_EMAILVIRUSCANDB (WM_USER + 100)
#define CHECKEMAILSCANSTATUS (WM_USER + 200)
#define CHECKLIVEUPDATE (WM_USER + 300)/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */
#define CHECKSCROLLINGTEXT	 (WM_USER + 500)
#define LAUNCHPRODEXPMSGBOX	 (WM_USER + 600)

//button size
const int BUTTON_WIDTH	= 80;
const int BUTTON_HEIGHT = 17;
const int BUTTON_WIDTH_BIG	= 150;


//buttons' IDs
const UINT ID_BUTTON_FULL_SCAN			= 10000;
const UINT ID_BUTTON_CUSTOM_SCAN		= 10001;
const UINT ID_BUTTON_QUICK_SCAN			= 10002;
const UINT ID_STATIC_SCAN				= 10003;
const UINT ID_STATIC_TOOLS			    = 10004;
const UINT ID_BUTTON_DATAENCRYPTION		= 10005;
const UINT ID_BUTTON_RECOVER			= 10006;
const UINT ID_BUTTON_REGISTYOPTIMIZER   = 10007;
const UINT ID_BUTTON_FOLDERLOCKER		= 10008;
const UINT ID_STATIC_EMAILSCAN			= 10009;
const UINT ID_BUTTON_VIRUSSCAN			= 10010;
const UINT ID_BUTTON_SPAMFILTER			= 10011;
const UINT ID_BUTTON_CONTENTFILTER		= 10012;
const UINT ID_BUTTON_SIGNATURE			= 10013;
const UINT ID_BUTTON_ANTIROOTKIT		= 10014;
const UINT ID_BUTTON_UPDATE				= 10015;
const UINT ID_BUTTON_REPORTS			= 10016;
const UINT ID_BUTTON_HOME_PAGE			= 10018;
const UINT ID_STATIC_ADMINSTRATION		= 10019;
const UINT ID_BUTTON_GENERAL_OPTION		= 10020;
const UINT ID_BUTTON_SCAN_OPTION		= 10021;
const UINT ID_BUTTON_EMAIL_OPTION		= 10022;
const UINT ID_BUTTON_UPDATE_OPTION		= 10023;
const UINT ID_BUTTON_UTILITY_OPTION		= 10024;



//buttons positions
const CPoint BTN_LOCATION(0, 0);
//const CPoint BTN_NEXT_LOCATION(140, 0);
//const CPoint BTN_LOCATION_1(82, 0);
//const CPoint BTN_LOCATION_2(2*82, 0);