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
#include <algorithm>
#include "Constants.h"
#include "ScannerContants.h"
#include "TreeConstants.h"
#include "PipeConstants.h"
#include <Wbemidl.h>
#include <comutil.h>
#include <intrin.h>
#include <cfgmgr32.h>
#include <newdev.h>
#include <setupapi.h>
#include <shlobj.h>
#include "DownloadConts.h"
#include "WardWizDumpCreater.h"
#include "WWizSettingsWrapper.h"
#include "WardwizLangManager.h"
#include "iTINRegWrapper.h"

#pragma comment(lib, "Wbemuuid.lib" )
#pragma comment(lib, "comsuppw.lib" )
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#pragma warning(disable:4996)

#define LOG_FILE					_T("VIBOCOMSRV.LOG")

void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);

//Add log to specified file path - vilas
void AddLogEntryWithPath(const TCHAR *lpLogFilePath, const TCHAR *sFormatString, const TCHAR *sEntry1, const TCHAR *sEntry2, bool isDateTime, DWORD dwLogLevel);

DWORD GetLoggingLevel4mRegistry();
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
CString MakeTokenisizeString(CString csFirstEntry , CString csSecondEntry  ,CString csThirdEntry = L"",CString csForthEntry = L"",CString csFifthEntry = L"",CString csSixthEntry = L"",DWORD dwSeventhEntry = 0);

CString GetWardWizPathFromRegistry( );
bool GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt);
DWORD GetProductID();
void ErrorDescription(HRESULT hr);
std::wstring String2WString(const std::string& strToConvert);
void AddLogEntryEx(DWORD dwLogLevel, IN PWCH Message, ...);


//Registration failures message return from registration dll.
typedef enum {
	MACHINEIDMISMATCH = 0x05,
	INVALIDEMAILID = 0x06,
	COUNTRYCODEINVALID = 0x07,
	INVALIDREGNUMBER = 0x08,
	INVALIDPRODVERSION = 0x09,
	USERINFOUPDATEFAILD = 0x10,
	PRODUCTEXPIRED = 0x11,
	NULLRESPONSE = 0x12
}REGFAILUREMSGS;