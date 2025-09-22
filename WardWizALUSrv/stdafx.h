// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//


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

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <afx.h>
#include <afxwin.h>
#include <Shlwapi.h>
#include <vector>
#include <winhttp.h>

#include "ZipArchive.h"

#include "DownloadConts.h"
#include "constants.h"

#include "ITinRegWrapper.h"
#include "WardWizDumpCreater.h"
#include "WardwizLangManager.h"
#include "WWizSettingsWrapper.h"
#include "WWizSettings.h"

#define LOG_FILE					_T("VIBOALUCOMMSRV.LOG")

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

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);
bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);

DWORD GetProductID();
void ErrorDescription(HRESULT hr);

CString GetAVPathFromRegistry( LPTSTR lpszAVPath = NULL );
CString GetWardWizPathFromRegistry();
