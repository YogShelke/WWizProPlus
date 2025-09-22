/**********************************************************************************************************
Program Name			: WardWizDataCrypt.cpp
Description				: Wrapper class to launch Crypt.exe
Author Name				: Nitin Kolapkar
Date Of Creation		: 5th Jun 2015
Version No				: 1.11.0.0
Special Logic Used		:
Modification Log		:
***********************************************************************************************************/
#include "WardWizDataCrypt.h"
#include "stdafx.h"
#include "WardwizLangManager.h"
#include "WardWizDumpCreater.h"
#include "ExecuteProcess.h"

PROCESS_INFORMATION		pinfo;
DWORD					ExitCode = 0;

/***********************************************************************************************
Function Name  : CWardWizDataCrypt
Description    : Constructor
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
CWardWizDataCrypt::CWardWizDataCrypt()
{
}

/***********************************************************************************************
Function Name  : ~CWardWizDataCrypt
Description    : Destructor
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
CWardWizDataCrypt::~CWardWizDataCrypt()
{
}

/***********************************************************************************************
Function Name  : LaunchDataEncDecExe
Description    : Launch the Crypt.exe as commandline with required parameters
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
bool CWardWizDataCrypt::LaunchDataEncDecExe(TCHAR *pszFileName, TCHAR *pszPass, DWORD dwKeepOriginal, DWORD dwDataCryptOpr)
{
	try
	{
		STARTUPINFO si;

		ZeroMemory(&pinfo, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.hStdOutput = NULL;
		si.hStdError = NULL;
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;



		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csDataEncDecFilePath = szModulePath;
		csDataEncDecFilePath += WRDWIZCRYPT;
		csDataEncDecFilePath += L" -NOGUI";
		if (dwDataCryptOpr == ENCRYPTION)
		{
			csDataEncDecFilePath += L" -ENC";

		}
		else if (dwDataCryptOpr == DECRYPTION)
		{
			csDataEncDecFilePath += L" -DEC";
		}
		csDataEncDecFilePath += L" -";
		csDataEncDecFilePath += pszPass;

		if (dwKeepOriginal == KEEPORIGINAL)
		{
			csDataEncDecFilePath += L" -KEEPORIGINAL";
		}
		else if (dwKeepOriginal == DELETEORIGINAL)
		{
			csDataEncDecFilePath += L" -DELETEORIGINAL";
		}

		csDataEncDecFilePath += L" -";
		csDataEncDecFilePath += pszFileName;
		LPTSTR pszCmdLine = csDataEncDecFilePath.GetBuffer();

		if (!CreateProcess(NULL, pszCmdLine,
			NULL, NULL,
			TRUE,
			CREATE_NEW_CONSOLE,
			NULL, NULL,
			&si,
			&pinfo))

		return FALSE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDataCrypt::LaunchDataEncDecExe", 0, 0, true, SECONDLEVEL);
		return FALSE;
	}
	return TRUE;
}
