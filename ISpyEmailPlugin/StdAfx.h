// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__5040DA3E_23F4_4B7B_876B_D7BBBE22EFF9__INCLUDED_)
#define AFX_STDAFX_H__5040DA3E_23F4_4B7B_876B_D7BBBE22EFF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning ( disable : 4222)
#pragma warning ( disable : 4278)


//#define STRICT
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT 0x0400
//#endif
//#define _ATL_APARTMENT_THREADED

#include <afxwin.h>
#include <afxdisp.h>

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>
//#include <atlwin.h>
#include <vector>
#include <map>
#include <sstream>
#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WWizSettingsWrapper.h"
//#include <afxcontrolbars.h>
//#include <afxcontrolbars.h>

using namespace std;


void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);

bool	GetModulePath(TCHAR *szModulePath, DWORD dwSize);
CString GetWardWizPathFromRegistry( );
CString GetISpyTempPath();
bool	ISEmailScanEnabled( );
bool	GetRegistrySetting(CString csSetting);
void	ErrorDescription(HRESULT hr);
void	trim(char *s, const int len);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

//#import "C:\Program Files (x86)\Common Files\microsoft shared\OFFICE14\mso.dll" rename_namespace("Office"), named_guids
//using namespace Office;
//
//#import "C:\Program Files (x86)\Microsoft Office\Office14\MSOUTL.olb" rename_namespace("Outlook"), named_guids, raw_interfaces_only
//using namespace Outlook;

#import "mso.dll" rename_namespace("Office"), named_guids
using namespace Office;

#import "MSOUTL.olb" rename_namespace("Outlook"), named_guids, raw_interfaces_only
using namespace Outlook;

#endif // !defined(AFX_STDAFX_H__5040DA3E_23F4_4B7B_876B_D7BBBE22EFF9__INCLUDED)

typedef std::basic_stringstream<TCHAR> tstringstream;
template<typename T> tstringstream& operator,(tstringstream& tss, T t) { tss << _T(" ") << t; return tss; }
#define OUTPUT_DEBUG_STRING(...) ::OutputDebugString((tstringstream(), _T("***"), __VA_ARGS__, _T("\n")).str().c_str());

enum __ENUMSPAMEMAILOPTION
{
	NONE,
	ALLOW,
	BLOCK
};
/*===========================================================*/
/*				CONSTANT STRINGS							*/
/*===========================================================*/
#define SZDETECTED		L"Detected"
#define SZNOTHREATS		L"No threat(s) found"
#define SZREPAIRED		L"Repaired"
#define SZQUARANTINED	L"Quarantined"
/*===========================================================*/


/*===========================================================*/
/*				USER DEFINED MESSAGESS						*/
/*===========================================================*/
#define RELOAD_EMAILVIRUSCANDB (WM_USER + 100)

/*===========================================================*/
/*					MAIL PROPERTY CONSTANTS					*/
/*===========================================================*/
#define WARDWIZSPAMPROP			L"WardWizSpam"
#define WARDWIZBODYPROP			L"WardWizBody"
#define WARDWIZNONBODYPROP		L"WardWizNonBody"
#define WARDWIZSCANPROP			L"WardWizScan"
#define WARDWIZMAILISSPAM		L"WardWizMailIsSpam"