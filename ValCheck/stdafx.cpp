#include "stdafx.h"

#define LOG_FILE					_T("VIBOAVUI.LOG")

/***************************************************************************
  Function Name  : GetModulePath
  Description    : to get the module path
  Author Name    : Ram
  S.R. No        : WRDWIZVALCHECKDLL_001
  Date           : 25th June 2014
****************************************************************************/
bool GetModulePath(TCHAR *szModulePath, DWORD dwSize)
{
	if(0 == GetModuleFileName(NULL, szModulePath, dwSize))
	{
		return false;
	}

	if(_tcsrchr(szModulePath, _T('\\')))
	{
		*_tcsrchr(szModulePath, _T('\\'))= 0;
	}
	return true;
}

/***************************************************************************
  Function Name  : AddLogEntry
  Description    : Add the log in specified file
  Author Name    : Ram
  S.R. No        :WRDWIZVALCHECKDLL_002
  Date           : 25th June 2014
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
		if(dwRegistryLogLevel == -1)
		{
			dwRegistryLogLevel = GetLoggingLevel4mRegistry();
		}

		DWORD dwRet = Check4LogLevel(dwRegistryLogLevel, dwLogLevel);		
		if(dwRet == 0x1 || dwRet == 0x2)
		{
			return;
		}
		/*==================================================================*/

		if(csScanLogFullPath.GetLength() == 0)
		{
			TCHAR szModulePath[MAX_PATH] = {0};
			if(!GetModulePath(szModulePath, MAX_PATH))
			{
				MessageBox(NULL, L"Error in GetModulePath", L"Vibranium", MB_ICONERROR);
				return;
			}
			csScanLogFullPath = szModulePath;
			csScanLogFullPath += L"\\Log\\";
			CreateDirectory(csScanLogFullPath, NULL);
			csScanLogFullPath += LOG_FILE;
		}

		if(!pRtktOutFile)
			pRtktOutFile = _wfsopen(csScanLogFullPath, _T("a"), 0x40);

		if(pRtktOutFile != NULL)
		{
			CString szMessage;
			if(sFormatString && sEntry1 && sEntry2)
				szMessage.Format(sFormatString, sEntry1, sEntry2);
			else if(sFormatString && sEntry1)
				szMessage.Format(sFormatString, sEntry1);
			else if(sFormatString && sEntry2)
				szMessage.Format(sFormatString, sEntry2);
			else if(sFormatString)
				szMessage = sFormatString;

			if(isDateTime)
			{
				TCHAR tbuffer[9]= {0};
				TCHAR dbuffer[9] = {0};
				_wstrtime_s(tbuffer, 9);
				_wstrdate_s(dbuffer, 9);

				CString szOutMessage;
				szOutMessage.Format(_T("[%s %s] [VIBOVALCHCK] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
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
	catch(...)
	{
	}
}

/***************************************************************************
  Function Name  : GetLoggingLevel4mRegistry
  Description    : Read Registry entry for logging level
  Author Name    : Prajakta
  S.R. No        : WRDWIZVALCHECKDLL_003
  Date           : 25th June 2014
****************************************************************************/
DWORD GetLoggingLevel4mRegistry()
{
	DWORD dwLogLevel = 0;
	try
	{
		HKEY	h_WRDWIZAV = NULL ;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h_WRDWIZAV) != ERROR_SUCCESS)
		{
			h_WRDWIZAV = NULL;
			return 0;
		}

		DWORD dwLogLevelSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		long ReadReg = RegQueryValueEx(h_WRDWIZAV, L"dwLoggingLevel", 0 ,&dwType,(LPBYTE)&dwLogLevel, &dwLogLevelSize);
		if(ReadReg != ERROR_SUCCESS)
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
  S.R. No        :WRDWIZVALCHECKDLL_004
  Date           : 25th June 2014
****************************************************************************/
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel)
{
	DWORD dwRet = 0x0;
	__try
	{
		if(dwRegLogLevel == 1 && dwLogLevel < 1)
		{
			dwRet = 0x1;
		}
		else if(dwRegLogLevel == 2 && dwLogLevel < 2)
		{
			dwRet = 0x2;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return dwRet;
}