
// stdafx.cpp : source file that includes just the standard includes
// WardWizDBUpgrade.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#define LOG_FILE					_T("WARDWIZDBUPGRADE.LOG")

/***************************************************************************
Function Name  : GetModulePath
Description    : Function returns module path
Author Name    : Vilas
Date           : 15 July 2014
SR_NO			 : SR.N0 ALUPD_0001
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
Description    : this function is called to log entries in LOG files
Author Name    : Vilas
Date           : 15 July 2014
SR_NO			 : SR.N0 ALUPD_0002
****************************************************************************/
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1, const TCHAR *sEntry2,
	bool isDateTime, DWORD dwLogLevel)
{
	try
	{
		static CString csScanLogFullPath;
		FILE *pRtktOutFile = NULL;

		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			MessageBox(NULL, L"Error in GetModulePath", L"WardWiz", MB_ICONERROR);
			return;
		}

		csScanLogFullPath = szModulePath;
		csScanLogFullPath += L"\\";
		csScanLogFullPath += LOG_FILE;


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
				szOutMessage.Format(_T("[%s %s] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
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
Function Name  : GetWardWizPathFromRegistry
Description    : function returns wardwiz app folder path from registry
Author Name    : Ram Shelke
Date           : 25th June 2014
****************************************************************************/
CString GetWardWizPathFromRegistry()
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
	return L"";
}