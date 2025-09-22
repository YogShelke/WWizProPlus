// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "Registry.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

//Error as cannot found.
//#include <atlimpl.cpp>


#define LOG_FILE					_T("VIBOEMAILSCAN.LOG")

void trim(char *s, const int len)
{
	__try
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
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in trim(char *s, const int len)", 0, 0, true, SECONDLEVEL);
	}
}

void ErrorDescription(HRESULT hr) 
{ 
     if(FACILITY_WINDOWS == HRESULT_FACILITY(hr)) 
         hr = HRESULT_CODE(hr); 

	 TCHAR szErrMsg[MAX_PATH] = {0}; 

     if(FormatMessage( 
       FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, 
       NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
       (LPTSTR)&szErrMsg, 0, NULL) != 0) 
     { 
         AddLogEntry(szErrMsg);
         LocalFree(szErrMsg); 
     } 
}

bool GetRegistrySetting(CString csSetting)
{
	HKEY	hSubKey = NULL ;
	DWORD	dwValue = 0;

	CString csKeyName = CWWizSettingsWrapper::GetProductRegistryKey() + CString(L"\\EmailScanSetting");
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, csKeyName, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD	dwSize = sizeof(DWORD) ;
	DWORD	dwType = REG_DWORD;

	RegQueryValueEx(hSubKey, csSetting, NULL ,&dwType,(LPBYTE)&dwValue,&dwSize);

	if(dwValue == 1)
	{
		return true;
	}
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;
	return false;
}

bool ISEmailScanEnabled( )
{
	bool bReturn = false;
	HKEY	hSubKey = NULL ;
	DWORD	dwValue = 0;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
	{
		return false;
	}
		
	DWORD	dwSize = sizeof(DWORD) ;
	DWORD	dwType = REG_DWORD;

	RegQueryValueEx(hSubKey, L"dwEmailScan", NULL ,&dwType,(LPBYTE)&dwValue,&dwSize);
	if(dwValue == 1)
	{
		bReturn = true;
	}
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	return bReturn;
}

CString GetWardWizPathFromRegistry( )
{
	HKEY	hSubKey = NULL ;
	TCHAR	szModulePath[MAX_PATH] = {0};

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
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

bool GetModulePath(TCHAR *szModulePath, DWORD dwSize)
{
	CString csProductFolderPath = GetWardWizPathFromRegistry();
	_tcscpy_s(szModulePath, dwSize, csProductFolderPath);
	return true;
}

CString GetISpyTempPath()
{
	//Path changes to temp folder as driver protection is not exist to it
	CString csReturn = L"";
	TCHAR szModulePath[MAX_PATH] = {0};
	if (GetEnvironmentVariable(L"TEMP", szModulePath, 511))
	{
		if(szModulePath && _tcslen(szModulePath) > 0)
		{
			csReturn.Format(L"%s\\Wardwiz", szModulePath);
		}
	}
	return csReturn;
}

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
				szOutMessage.Format(_T("[%s %s] [VIBOEPLUGIN] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
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
