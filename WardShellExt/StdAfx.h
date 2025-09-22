// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__5E2121E4_0300_11D4_8D3B_444553540000__INCLUDED_)
#define AFX_STDAFX_H__5E2121E4_0300_11D4_8D3B_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning ( disable : 4005 )
#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4651 )

#define STRICT
#define WINVER          0x0500
#define _WIN32_WINNT    0x0501
#define _WIN32_IE       0x0501

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlconv.h>
#include <atlstr.h>
#include <shlobj.h>
#include <comdef.h>
#include <vector>
#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WWizSettingsWrapper.h"
#include "PipeConstants.h"
#include "TreeConstants.h"

using namespace std;

#define EMPTY_STRING _T("")

bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);

CString csProdRegKey = EMPTY_STRING;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5E2121E4_0300_11D4_8D3B_444553540000__INCLUDED)

