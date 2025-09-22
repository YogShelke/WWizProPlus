// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "iTinRegWrapper.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#if _ATL_VER < 0x0700
#include <statreg.cpp>
#endif
#endif

#if _ATL_VER < 0x0700
#include <atlimpl.cpp>
#endif

#define LOG_FILE					_T("VIBOAVUI.LOG")

/***************************************************************************
Function Name  : GetModulePath
Description    : write log entry to log file
Author Name    : Neha Gharge
S.R.NO		 : WRDWIZUSBDLL_010
Date           : 25th June 2014
****************************************************************************/
bool GetModulePath(TCHAR *szModulePath, DWORD dwSize)
{
	if (0 == GetModuleFileName(NULL, szModulePath, dwSize))
	{
		return false;
	}

	if (_tcsrchr(szModulePath, _T('\\')))
	{
		*_tcsrchr(szModulePath, _T('\\')) = 0;
	}
	return true;
}
/***************************************************************************
Function Name  : AddLogEntry
Description    :  write log entry to log file
Author Name    : Neha Gharge
S.R.NO		 : WRDWIZUSBDLL_011
Date           : 25th June 2014
****************************************************************************/
void AddLogEntry(const TCHAR sFormatString[1024], const TCHAR *sEntry1, const TCHAR *sEntry2,
	bool isDateTime, DWORD dwLogLevel)
{
	try
	{

		TCHAR csScanLogFullPath[512] = { 0 };
		FILE *pRtktOutFile = NULL;
		CITinRegWrapper objReg;

		/*==================================================================*/
		/*						LOGGING LEVEL							   */
		/*==================================================================*/
		static DWORD dwRegistryLogLevel = -1;
		if (dwRegistryLogLevel == -1)
		{
			dwRegistryLogLevel = GetLoggingLevel4mRegistry();
		}

		DWORD dwRet = Check4LogLevel(dwRegistryLogLevel, dwLogLevel);
		if (dwRet == 0x1 || dwRet == 0x2)
		{
			return;
		}
		/*==================================================================*/

		if (wcslen(csScanLogFullPath) == 0)
		{
			TCHAR szModulePath[MAX_PATH] = { 0 };
			TCHAR szValue[MAX_PATH] = { 0 };
			DWORD dwSize = sizeof(szValue);
			
			/*if (!GetModulePath(szModulePath, MAX_PATH))
			{
				MessageBox(NULL, L"Error in GetModulePath", L"Vibranium", MB_ICONERROR);
				return;
			}*/

			// issue :- log encry for this module never got created since from start
			// issue resolved by lalit kumawat 7-23-2015
			DWORD dwRet = 0;

			if (csProdRegKey == EMPTY_STRING)
			{
				csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();
			}

			dwRet = objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, csProdRegKey.GetBuffer(), L"AppFolder", szModulePath, dwSize);
			
			if (dwRet) // failed to get app folder path
				return;

			wcscpy_s(csScanLogFullPath, szModulePath);
			wcscat_s(csScanLogFullPath, L"Log\\");
			CreateDirectory(csScanLogFullPath, NULL);
			wcscat_s(csScanLogFullPath, LOG_FILE);
		}

		if (!pRtktOutFile)
			pRtktOutFile = _wfsopen(csScanLogFullPath, _T("a"), 0x40);

		if (pRtktOutFile != NULL)
		{
			TCHAR  szMessage[256];
			//if(sFormatString && sEntry1 && sEntry2)

			//	szMessage.Format(sFormatString, sEntry1, sEntry2);
			//else if(sFormatString && sEntry1)
			//	szMessage.Format(sFormatString, sEntry1);
			//else if(sFormatString && sEntry2)
			//	szMessage.Format(sFormatString, sEntry2);
			if (sFormatString)
				wcscpy_s(szMessage, sFormatString);

			if (isDateTime)
			{
				TCHAR tbuffer[9] = { 0 };
				TCHAR dbuffer[9] = { 0 };
				_wstrtime_s(tbuffer, 9);
				_wstrdate_s(dbuffer, 9);

				TCHAR  szOutMessage[1024];
				wcscpy_s(szOutMessage, dbuffer);
				wcscat_s(szOutMessage, L" ");
				wcscat_s(szOutMessage, tbuffer);
				wcscat_s(szOutMessage, L" ");
				wcscat_s(szOutMessage, L"[VIBOSHELLEXT] ");
				wcscat_s(szOutMessage, szMessage);
				wcscat_s(szOutMessage, L"\n");
				fputws((LPCTSTR)szOutMessage, pRtktOutFile);
			}
			else
			{
				fputws((LPCTSTR)szMessage, pRtktOutFile);
			}
			fflush(pRtktOutFile);
			fclose(pRtktOutFile);
		}
	}
	catch (...)
	{
	}
}

/***************************************************************************
Function Name  : GetLoggingLevel4mRegistry
Description    : Read Registry entry for logging level
Author Name    : Neha Gharge
S.R.NO		 :WRDWIZUSBDLL_012
Date           : 25th June 2014
****************************************************************************/
DWORD GetLoggingLevel4mRegistry()
{
	DWORD dwLogLevel = 0;
	try
	{
		if (csProdRegKey == EMPTY_STRING)
		{
			csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();
		}

		HKEY	h_WRDWIZAV = NULL;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, csProdRegKey, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h_WRDWIZAV) != ERROR_SUCCESS)
		{
			h_WRDWIZAV = NULL;
			return 0;
		}

		DWORD dwLogLevelSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		long ReadReg = RegQueryValueEx(h_WRDWIZAV, L"dwLoggingLevel", 0, &dwType, (LPBYTE)&dwLogLevel, &dwLogLevelSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			h_WRDWIZAV = NULL;
			return 0;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in GetLoggingLevel4mRegistry", 0, 0, true, SECONDLEVEL);
	}
	return dwLogLevel;
}

/***************************************************************************
Function Name  : Check4LogLevel
Description    : Check the parameter for log entry
Author Name    : Neha Gharge
S.R.NO          :WRDWIZUSBDLL_013
Date           : 25th June 2014
****************************************************************************/
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel)
{
	DWORD dwRet = 0x0;
	__try
	{
		if (dwRegLogLevel == 1 && dwLogLevel < 1)
		{
			dwRet = 0x1;
		}
		else if (dwRegLogLevel == 2 && dwLogLevel < 2)
		{
			dwRet = 0x2;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return dwRet;
}