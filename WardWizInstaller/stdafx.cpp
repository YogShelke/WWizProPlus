/**********************************************************************************************************
Program Name          : stdafx.cpp
Description           : Logs for installer class
Author Name			  : Tejas Shinde
Date Of Creation      : 18 Jun 2019
Modification Log      :
***********************************************************************************************************/

#include "stdafx.h"

#define LOG_FILE				_T("VIBOINSTALLER.LOG")

/**********************************************************************************************************
*  Function Name  :	AddLogEntry
*  Description    :	Add log entry into respective log files
*  Author Name    : Tejas Shinde
*  SR_NO		  :
*  Date           : 18 Jun 2019
**********************************************************************************************************/
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
			memset(szModulePath, 0x00, MAX_PATH * sizeof(TCHAR));
			GetEnvironmentVariable(L"ALLUSERSPROFILE", szModulePath, MAX_PATH);

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
				szMessage.Format(sFormatString, sEntry1, sEntry2);
			else if (sFormatString && sEntry1)
				szMessage.Format(sFormatString, sEntry1);
			else if (sFormatString && sEntry2)
				szMessage.Format(sFormatString, sEntry2);
			else if (sFormatString)
				szMessage = sFormatString;

			if (isDateTime)
			{
				TCHAR tbuffer[9] = { 0 };
				TCHAR dbuffer[9] = { 0 };
				_wstrtime_s(tbuffer, 9);
				_wstrdate_s(dbuffer, 9);

				CString szOutMessage;
				szOutMessage.Format(_T("[%s %s] [VIBOSETUPDLL] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
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
Author Name    : Prajakta
Date           : 25th June 2014
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
		AddLogEntry(L"### Excpetion in GetLoggingLevel4mRegistry", 0, 0, true, SECONDLEVEL);
	}
	return dwLogLevel;
}

/***************************************************************************
Function Name  : Check4LogLevel
Description    : Check the parameter for log entry
Author Name    : Prajakta
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

/**********************************************************************************************************
*  Function Name  :	GetModulePath
*  Description    :	Get path where module exist
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
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
Function Name  : GetWardWizPathFromRegistry
Description    : Function to get wardwiz path from registry
Author Name    : Ram Shelke
Date           : 17th June 2014
****************************************************************************/
CString GetWardWizPathFromRegistry()
{
	HKEY	hSubKey = NULL;
	TCHAR	szModulePath[MAX_PATH] = { 0 };

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
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
	return L"";
}

/***********************************************************************************************
Function Name  : AddLogEntryEx
Description    : Extended function of addlogentry
SR.NO		   :
Author Name    : Gayatri A.
Date           : 14 Jul 2016
***********************************************************************************************/
void AddLogEntryEx(DWORD dwLogLevel, IN PWCH Message, ...)
{
	try
	{
		va_list MessageList;
		TCHAR szMessageBuffer[0xA28] = { 0 };

		// First, combine the message
		va_start(MessageList, Message);
		_vsnwprintf(szMessageBuffer, 0xA28, Message, MessageList);
		va_end(MessageList);

		AddLogEntry(L"%s", szMessageBuffer, 0, true, dwLogLevel);
	}
	catch (...)
	{
	}
}

/***************************************************************************
Function Name  : ErrorDescription
Description    : fucntion which returns the COM error
Author Name    : Ram Shelke
Date           : 25th June 2014
****************************************************************************/
void ErrorDescription(HRESULT hr)
{
	if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
		hr = HRESULT_CODE(hr);

	TCHAR szErrMsg[MAX_PATH] = { 0 };

	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&szErrMsg, 0, NULL) != 0)
	{
		AddLogEntry(L"### COM Error: %s\n", szErrMsg);
		LocalFree(szErrMsg);
	}
}