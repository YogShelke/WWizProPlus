// stdafx.cpp : source file that includes just the standard includes
// iSpyComSrv.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "WardwizLangManager.h"

/***************************************************************************
  Function Name  : GetModulePath
  Description    : function returns module path 
  Author Name    : Ram Shelke
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
  Description    : Write log to file 
  Author Name    : Ram Shelke
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
				szOutMessage.Format(_T("[%s %s] [VIBOCOMSERVI] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
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
Function Name  : AddLogEntryWithPath
Description    : Write log to file
Author Name    : Vilas
Date           : 17th Dec 2015
****************************************************************************/
void AddLogEntryWithPath(const TCHAR *lpLogFilePath, const TCHAR *sFormatString, const TCHAR *sEntry1, const TCHAR *sEntry2,
	bool isDateTime, DWORD dwLogLevel)
{
	try
	{
		static CString csScanLogFullPath;
		FILE *pRtktOutFile = NULL;

		if (!lpLogFilePath)
		{
			return;
		}

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

	/*	if (csScanLogFullPath.GetLength() == 0)
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
	*/
		if (!pRtktOutFile)
			pRtktOutFile = _wfsopen(lpLogFilePath, _T("a"), 0x40);

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
				CString szOutMessage;
				szOutMessage.Format(_T("%s"), static_cast<LPCTSTR>(szMessage));
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

CString MakeTokenisizeString(CString csFirstEntry , CString csSecondEntry ,CString csThirdEntry ,CString csForthEntry ,CString csFifthEntry ,CString csSixthEntry ,DWORD dwSeventhEntry)
{
	CString csCombineString = L"";
	csCombineString.Format(L"%s#%s#%s#%s#%s#%s#%d#",csFirstEntry,csSecondEntry,csThirdEntry,csForthEntry,csFifthEntry,csSixthEntry,dwSeventhEntry);
	return csCombineString;
}

/**********************************************************************************************************
*  Function Name  :	GetWardwizRegistryDetails
*  Description    :	Read Scanning related options from registry
*  Author Name    : Gayatri A.
*  SR_NO		  :
*  Date           : 13 Sep 2016
**********************************************************************************************************/
bool GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt)
{
	try
	{
		CString csRegKeyPath;
		csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		//Get here registry setting for Active scanning.
		DWORD dwQuarantineOption = 0x00;

		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwQuarantineOption", dwQuarantineOption) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwQuarantineOption in GetWardwizRegistryDetails KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		dwQuarantineOpt = dwQuarantineOption;

		//Get here registry setting for Active scanning.
		DWORD dwHeuScan = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwHeuScan", dwHeuScan) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwQuarantineOption in GetWardwizRegistryDetails KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		dwHeuScanOpt = dwHeuScan;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::GetWardwizRegistryDetails", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
  Function Name  : GetWardWizPathFromRegistry
  Description    : function returns wardwiz app folder path from registry
  Author Name    : Ram Shelke
  Date           : 25th June 2014
****************************************************************************/
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

DWORD GetProductID()
{
	CWardwizLangManager objWardwizLangManager;
	return objWardwizLangManager.GetSelectedProductID();
}

/***************************************************************************
  Function Name  : ErrorDescription
  Description    : fucntion which returns the COM error
  Author Name    : Ram Shelke
  Date           : 25th June 2014
****************************************************************************/
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
		 AddLogEntry(L"### COM Error: %s\n", szErrMsg);
         LocalFree(szErrMsg); 
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

///***********************************************************************************************
//Function Name  : String2WString
//Description    : Helper Function to convert string to wstring.
//SR.NO		   :
//Author Name    : Gayatri A.
//Date           : 14 Jul 2016
//***********************************************************************************************/
std::wstring String2WString(const std::string& strToConvert)
{
	std::wstring wsReturn = L"";
	try
	{
		int iLen;
		int iLength = (int)strToConvert.length() + 1;
		iLen = MultiByteToWideChar(CP_ACP, 0, strToConvert.c_str(), iLength, 0, 0);
		wchar_t* wcBuffer = new wchar_t[iLen];
		MultiByteToWideChar(CP_ACP, 0, strToConvert.c_str(), iLength, wcBuffer, iLen);
		std::wstring strRetString(wcBuffer);
		delete[] wcBuffer;
		wsReturn = strRetString;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in IndexingThread", 0, 0, true, SECONDLEVEL);
		wsReturn = L"";
	}
	return wsReturn;
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