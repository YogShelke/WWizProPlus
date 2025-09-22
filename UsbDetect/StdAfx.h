#if !defined(AFX_STDAFX_H__B3C8B51A_378C_4C46_9933_E52A1837A5A4__INCLUDED_)
#define AFX_STDAFX_H__B3C8B51A_378C_4C46_9933_E52A1837A5A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT 0x0501

#pragma warning (disable : 4995)

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define DRIVEREMOVALSCANSTATUS (WM_USER + 200)

#include <afx.h>
#include <windows.h>
#include <tchar.h>
#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WWizSettingsWrapper.h"

bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);
bool GetProductRegistryKey(TCHAR *szRegKeyValue, DWORD dwSize);

#endif // !defined(AFX_STDAFX_H__B3C8B51A_378C_4C46_9933_E52A1837A5A4__INCLUDED_)


