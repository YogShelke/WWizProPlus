
// stdafx.cpp : source file that includes just the standard includes
// WrdWizAutoScn.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"


/***************************************************************************
Function Name  : GetModulePath
Description    : Function returns module path
Author Name    : Neha G
Date           : 25th June 2014
SR_NO			 : WRDWIZUSBUI_0066
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
Description    : This function is called to log entries in LOG files
Author Name    : Neha G
Date           : 15 July 2014
SR_NO			 : WRDWIZUSBUI_0067
****************************************************************************/
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1, const TCHAR *sEntry2,
	bool isDateTime, DWORD dwLogLevel)
{
	try
	{
		static CString csScanLogFullPath;
		FILE *pRtktOutFile = NULL;

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

		if (csScanLogFullPath.GetLength() == 0)
		{
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (!GetModulePath(szModulePath, MAX_PATH))
			{
				MessageBox(NULL, L"Error in GetModulePath", L"Vibranium", MB_ICONERROR);
				return;
			}
			csScanLogFullPath = szModulePath;
			csScanLogFullPath += L"\\Log\\";
			CreateDirectory(csScanLogFullPath, NULL);
			csScanLogFullPath += LOG_FILE;
		}

		if (!pRtktOutFile)
			pRtktOutFile = _wfsopen(csScanLogFullPath, _T("a"), 0x40);

		if (pRtktOutFile != NULL)
		{
			CString szMessage;
			if (sFormatString && sEntry1 && sEntry2)
				szMessage.Format(L"%s%s%s", sFormatString, sEntry1, sEntry2);
			else if (sFormatString && sEntry1)
				szMessage.Format(L"%s%s", sFormatString, sEntry1);
			else if (sFormatString && sEntry2)
				szMessage.Format(L"%s%s", sFormatString, sEntry2);
			else if (sFormatString)
				szMessage = sFormatString;

			if (isDateTime)
			{
				TCHAR tbuffer[9] = { 0 };
				TCHAR dbuffer[9] = { 0 };
				_wstrtime_s(tbuffer, 9);
				_wstrdate_s(dbuffer, 9);

				CString szOutMessage;
				szOutMessage.Format(_T("[%s %s] [VIBOTEMPCLN] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
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
		AddLogEntry(L"###Wardwiz : Error in AddLogEntry");
	}
}


/***************************************************************************
Function Name  : GetLoggingLevel4mRegistry
Description    : Read Registry entry for logging level
Author Name    : Prajakta
Date           : 25th June 2014
SR_NO			 : WRDWIZUSBUI_0068
****************************************************************************/
DWORD GetLoggingLevel4mRegistry()
{
	DWORD dwLogLevel = 0;
	try
	{
		HKEY	h_WRDWIZAV = NULL;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h_WRDWIZAV) != ERROR_SUCCESS)
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
		AddLogEntry(L"### Exception in VibraniumTempCln::GetLoggingLevel4mRegistry", 0, 0, true, SECONDLEVEL);
	}
	return dwLogLevel;
}

/***************************************************************************
Function Name  : Check4LogLevel
Description    : Check the parameter for log entry
Author Name    : Prajakta
Date           : 25th June 2014
SR_NO			 : WRDWIZUSBUI_0069
****************************************************************************/
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel)
{
	DWORD dwRet = 0x0;
	try
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
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumTempCln::Check4LogLevel", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************
Function Name  : GetWardWizPathFromRegistry
Description    : function returns wardwiz app folder path from registry
Author Name    : Ram Shelke
Date           : 25th June 2014
****************************************************************************/
CString GetWardWizPathFromRegistry()
{
	try
	{
		HKEY	hSubKey = NULL;
		TCHAR	szModulePath[MAX_PATH] = { 0 };

		if (RegOpenKey(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), &hSubKey) != ERROR_SUCCESS)
			return L"";

		DWORD	dwSize = 511;
		DWORD	dwType = 0x00;

		RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize);
		RegCloseKey(hSubKey);
		hSubKey = NULL;

		if (_tcslen(szModulePath) > 0)
		{
			return CString(szModulePath);
		}
	}
	catch (...)
	{
	}
	return L"";
}