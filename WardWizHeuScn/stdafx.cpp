// stdafx.cpp : source file that includes just the standard includes
// WWHeuScn.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
//#include "WardWizDumpCreater.h"

#define LOG_FILE					_T("VIBOHEUSCN.LOG")


/***************************************************************************************************
*  Function Name  : GetModulePath()
*  Description    : returns running path
*  Author Name    : Vilas
*  Date			  :	08-Sept-2014
*  SR No		  : WRDWIZHEURISTICSCANNER_0022
****************************************************************************************************/
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

/***************************************************************************************************
*  Function Name  : GetWardWizPathFromRegistry()
*  Description    : returns WardWiz install path from registry
*  Author Name    : Vilas
*  Date			  :	08-Sept-2014
*  SR No		  : WRDWIZHEURISTICSCANNER_0023
****************************************************************************************************/
CString GetWardWizPathFromRegistry( )
{
	HKEY	hSubKey = NULL ;
	TCHAR	szModulePath[MAX_PATH] = {0};

	if (RegOpenKey(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), &hSubKey) != ERROR_SUCCESS)
		return L"";

	DWORD	dwSize = 511 ;
	DWORD	dwType = 0x00 ;

	RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if(_tcslen(szModulePath) > 0)
	{
		return CString(szModulePath) ;
	}
	return L"";
}

/***************************************************************************************************
*  Function Name  : AddLogEntry()
*  Description    : Adds entry in log file
*  Author Name    : Vilas
*  Date			  :	08-Sept-2014
*  SR No		  : WRDWIZHEURISTICSCANNER_0024
****************************************************************************************************/
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
			csScanLogFullPath = GetWardWizPathFromRegistry();
			csScanLogFullPath += L"Log\\";
			if(!PathFileExists(csScanLogFullPath))
			{
				CreateDirectory(csScanLogFullPath, NULL);
			}
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
				szOutMessage.Format(_T("[%s %s] [VIBOSCANDLL] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
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

/***************************************************************************************************
*  Function Name  : trim()
*  Description    : removes spaces from input string
*  Author Name    : Vilas
*  Date			  :	08-Sept-2014
*  SR No		  : WRDWIZHEURISTICSCANNER_0025
****************************************************************************************************/
void trim(char *s, const int len)
{
	try
	{
		int end = len - 1;
		int start = 0;
		int i = 0;

		while ((start < len) && (s[start] <= ' '))
		{
			start++;
		}

		while ((start < end) && (s[end] <= ' '))
		{
			end--;
		}

		if (start > end)
		{
			memset(s, '\0', len);
			return;
		}

		for (i = 0; (i + start) <= end; i++)
		{
			s[i] = s[start + i];
		}
		memset((s + i), '\0', len - i);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in trim(char *s, const int len)", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : GetLoggingLevel4mRegistry()
  Description    : returns logging level from registry
  Author Name    : Prajakta
  Date           : 25th June 2014
  SR No		     : WRDWIZHEURISTICSCANNER_0026
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
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		AddLogEntry(L"### Exception in GetLoggingLevel4mRegistry", 0, 0, true, SECONDLEVEL);
	}
	return dwLogLevel;
}

/***************************************************************************
  Function Name  : Check4LogLevel
  Description    : Check log level
  Author Name    : Prajakta
  Date           : 25th June 2014
  SR No		     : WRDWIZHEURISTICSCANNER_0027
****************************************************************************/
DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel)
{
	DWORD dwRet = 0x0;

	try
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
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		AddLogEntry(L"### Exception in Check4LogLevel", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}