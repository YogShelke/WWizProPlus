#include "WWizProdDownloader.h"

CWWizProdDownloader::CWWizProdDownloader()
{
	m_pDownloadController = new CDownloadController();
}

CWWizProdDownloader::~CWWizProdDownloader()
{
	if (m_pDownloadController != NULL)
	{
		delete m_pDownloadController;
		m_pDownloadController = NULL;
	}
}

/***************************************************************************************************
*  Function Name  : StartDownloadFile()
*  Description    : Start downloading files on by one .
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0052
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWWizProdDownloader::StartDownloadFile(LPCTSTR szUrlPath, LPTSTR lpszTargetPath)
{
	try
	{
		if (!szUrlPath)
			return false;

		if (!lpszTargetPath)
			return false;

		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		STRUCT_DOWNLOAD_INFO sDownloadInfo = { 0 };
		_tcscpy_s(sDownloadInfo.szMainUrl, URL_SIZE, szUrlPath);
		_tcscpy_s(sDownloadInfo.szSectionName, csFileName);
		_tcscpy_s(sDownloadInfo.szExeName, csFileName);

		DWORD dwThreadSize = DEFAULT_DOWNLOAD_THREAD;

		sDownloadInfo.dwDownloadThreadCount = dwThreadSize;

		TCHAR szAllUserPath[MAX_PATH] = { 0 };
		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 255);

		_stprintf_s(sDownloadInfo.szLocalTempDownloadPath, MAX_PATH, L"%s\\%s\\", szAllUserPath, L"WardWiz");
		_stprintf_s(sDownloadInfo.szLocalPath, MAX_PATH, L"%s\\%s\\", szAllUserPath, L"WardWiz");

		TCHAR szTagetPath[MAX_PATH] = { 0 };
		_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s\\%s", szAllUserPath, L"WardWiz", csFileName);
		_tcscpy(lpszTargetPath, szTagetPath);

		DWORD dwTotalFileSize = 0x00;
		TCHAR szInfo[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);

		CWWizHttpManager objWinHttpManager;
		if (!objWinHttpManager.Initialize(szUrlPath))
		{
			AddLogEntry(L"### Initialize Failed in StartDownloadFile, URLPath: [%s]", szUrlPath, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
		{
			AddLogEntry(L"### GetHeaderInfo Failed in StartDownloadFile, URLPath: [%s]", szUrlPath, 0, true, SECONDLEVEL);
			return false;
		}
		dwTotalFileSize = _wtol(szInfo);

		if (PathFileExists(szTagetPath))
		{
			SetFileAttributes(szTagetPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szTagetPath);
		}

		if (!m_pDownloadController)
			return false;

		bool bDownloadFailed = false;
		DWORD dwRetryCount = 0x00;
		do
		{
			bDownloadFailed = false;
			m_pDownloadController->ResetInitData();
			if (!m_pDownloadController->StartController(&sDownloadInfo))
			{
				AddLogEntry(L"### Retry to download: FilePath: [%s\\%s]", sDownloadInfo.szLocalTempDownloadPath, sDownloadInfo.szExeName, true, FIRSTLEVEL);
				bDownloadFailed = true;
				dwRetryCount++;
			}
		} while (bDownloadFailed && dwRetryCount < MAX_RETRY_COUNT);

		if (bDownloadFailed)
			return false;

		AddLogEntry(L">>> File Downloaded: FilePath: [%s\\%s]", sDownloadInfo.szLocalTempDownloadPath, sDownloadInfo.szExeName, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizProdDownloader::StartDownloadFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : RunUtililtyInSilentModeSEH
Description    : Run utility.exe setup with verysilent mode and add utilities in startup menu
Author Name    : Neha Gharge
SR.NO		   :
Date           : 30 Apr 2015
***********************************************************************************************/
bool CWWizProdDownloader::RunSetupInSilentMode(LPCTSTR szSetupPath)
{
	try
	{

		CString csExePath, csCommandLine, csExePathExist, csLanguage;
		if (!PathFileExists(szSetupPath))
		{
			AddLogEntry(L"### File is not available File: [%s]", szSetupPath, 0, true, FIRSTLEVEL);
			return false;
		}

		csExePath = szSetupPath;
		csLanguage += L"LANG=english";
		
		csCommandLine.Format(L"%s%s", L"/verysilent /SUPPRESSMSGBOXES /", csLanguage);

		ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);

		AddLogEntry(L">>> [OK] RunSetupInSilentMode: %s, CommandLine: %s", csExePathExist, csCommandLine, true, SECONDLEVEL);
		
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"Exception in WrdWizAluSrv::RunUtililtyInSilentMode", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}