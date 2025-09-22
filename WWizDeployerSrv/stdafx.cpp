#include "stdafx.h"

#define LOG_FILE					_T("WWIZDEPLOYERSRV.LOG")

/**********************************************************************************************************
*  Function Name  :	AddLogEntry
*  Description    :	Add log entry into respective log files
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1, const TCHAR *sEntry2,
	bool isDateTime, DWORD dwLogLevel)
{
	try
	{
		static CString csScanLogFullPath;
		FILE *pRtktOutFile = NULL;

		if (csScanLogFullPath.GetLength() == 0)
		{
			TCHAR szModulePath[MAX_PATH] = { 0 };
			memset(szModulePath, 0x00, MAX_PATH * sizeof(TCHAR));
			if (!GetModulePath(szModulePath, MAX_PATH))
			{
				//MessageBox(NULL, L"Error in GetModulePath", L"WardWiz", MB_ICONERROR);
				return;
			}

			//ISSUE : Log for WARDWIZUI is getting created at incorrect path.
			//NAME - Niranjan Deshak. - 23th Jan 2015
			csScanLogFullPath = szModulePath;
			csScanLogFullPath += L"\\Log\\";
			if (!PathFileExists(csScanLogFullPath))
			{
				CreateDirectory(csScanLogFullPath, NULL);
			}
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
				szOutMessage.Format(_T("[%s %s] [WWIZDEPLOYERSRV] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
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
		AddLogEntry(L"###WRDWIZCLIENTAGENT : Error in AddLogEntry", 0, 0, true, SECONDLEVEL);
	}
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