/**********************************************************************************************************
*	Program Name			: DownloadController.cpp
*	Description				: Class which handles download the files in parts to achieve best download 
*							  Performance.
*	Author Name				: Ramkrushna Shelke
*	Date Of Creation		: 14 Jan 2017
*	Version No				: 2.6.0.121
*	Special Logic Used		:
*	Modification Log		:
***********************************************************************************************************/
#include "StdAfx.h"
#include "DownloadController.h"
#include "DownloadConts.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD CDownloadController::m_nPoolSize = 1;

/***************************************************************************
Function       : CDownloadController
In Parameters  : void, 
Out Parameters :
Description    : constructor
Author		   : Ram Shelke
****************************************************************************/
CDownloadController::CDownloadController(void)
{
	m_objDownloadQueue.m_hQueueEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_pIGuiInterface = NULL;
	m_hCtrlThread = NULL;
	ResetInitData();
}

/***************************************************************************
Function       : ResetInitData
In Parameters  : void,
Out Parameters :
Description    : Function to initialize variables
Author		   : Ram Shelke
****************************************************************************/
void CDownloadController::ResetInitData()
{
	m_bStartController = false;
	m_bResumeDownload = false;
	m_lpDownloadInfo  = NULL;
	m_dwCurrentDownloadStatus = 0;
	m_dwCurrentFileSize = 0;
	m_nContextIndex = 0;
	m_pIExecCurrentContext = NULL;
	m_nTaskItems =   0;
	m_dwCurrDownloadedBytes = 0;
	::ZeroMemory(&m_sHeaderInfo,sizeof(STRUCT_HEADER_INFO));
	::ZeroMemory(m_szDownloadLocalPath,sizeof(m_szDownloadLocalPath));
	::ZeroMemory(m_szLocalTempDownloadPath,sizeof(m_szLocalTempDownloadPath));
	::ZeroMemory(m_szOrgFileName,sizeof(m_szOrgFileName));
	::ZeroMemory(m_szUrlPAth,sizeof(m_szUrlPAth));
	::ZeroMemory(m_szSectionHeader,sizeof(m_szSectionHeader));
	::ZeroMemory(m_szINIPath,sizeof(m_szINIPath));
	::ZeroMemory(m_szFileMD5,sizeof(m_szFileMD5));
	m_DownloaderThreadPool.ClearExecutionContext();
	m_dwCurrContentLength = 0;
	if(m_hCtrlThread)
	{
		::CloseHandle(m_hCtrlThread);
		m_hCtrlThread = NULL;
	}

}
/***************************************************************************
Function       : ~CDownloadController
In Parameters  : void, 
Out Parameters :
Description    : destructor
Author		   : Ram Shelke
****************************************************************************/
CDownloadController::~CDownloadController(void)
{
	if(m_objDownloadQueue.m_hQueueEvent)
	{
		::CloseHandle(m_objDownloadQueue.m_hQueueEvent);
		m_objDownloadQueue.m_hQueueEvent = NULL; 
	}
}

/***************************************************************************
Function       : GetFileSizeEx
In Parameters  : TCHAR *szFilePath, 
Out Parameters : DWORD
Description    : retrive file size
Author		   : Ram Shelke
****************************************************************************/
DWORD CDownloadController::GetFileSizeEx(TCHAR *szFilePath)
{
	__try
	{
		HANDLE hFile = NULL;
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
		if ((hFile = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
		{
			DWORD dwFileSize = GetFileSize(hFile, NULL);
			if (hFile)
			{
				FlushFileBuffers(hFile);
				CloseHandle(hFile);
				hFile = NULL;
			}
			return dwFileSize;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadController::GetFileSizeEx, FilePath: [%s]", szFilePath, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function       : VerifyResumeDownloadFiles
In Parameters  : DWORD dwFileSize
Out Parameters : bool
Description    : verify resume download files
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::VerifyResumeDownloadFiles(DWORD dwFileSize)
{
	bool bResumeCancel = true;
	try
	{
		for (DWORD i = 0; i < m_nPoolSize; i++)
		{
			TCHAR szSection[MAX_PATH] = { 0 };
			wsprintf(szSection, _T("DownloadedBytesThread_%ld_%ld"), m_nPoolSize, i);
			DWORD dwDownloadedSize = GetPrivateProfileInt(m_szSectionHeader, szSection,
				0, m_szINIPath);
			CString csTemp;
			csTemp.Format(_T("%s%s_%d_%d.tmp"), m_szLocalTempDownloadPath, m_szOrgFileName,
				m_nPoolSize, i);
			DWORD dwTempFileSize = GetFileSizeEx((TCHAR *)(LPCTSTR)csTemp);
			if (dwTempFileSize < dwDownloadedSize)
			{
				bResumeCancel = false;
				break;
			}

			wsprintf(szSection, _T("SizeAlloted_%ld_%ld"), m_nPoolSize, i);
			DWORD dwAssignedSize = GetPrivateProfileInt(m_szSectionHeader,
				szSection, 0, m_szINIPath);
			if (dwAssignedSize)
			{
				DWORD dwPartSize = dwFileSize / m_nPoolSize;
				if (m_nPoolSize == 1)
				{
					if (dwAssignedSize != dwPartSize)
					{
						bResumeCancel = false;
						break;
					}
				}
				else
				{
					if (i == 0)
					{
						if (dwAssignedSize != dwPartSize)
						{
							bResumeCancel = false;
							break;
						}
					}
					else if (i == m_nPoolSize - 1)
					{
						DWORD dwByteRangeStart = ((dwFileSize / m_nPoolSize)*(i));
						dwPartSize = dwFileSize - dwByteRangeStart;
						if (dwAssignedSize != dwPartSize)
						{
							bResumeCancel = false;
							break;
						}
					}
					else
					{
						DWORD dwByteRangeStart = ((dwFileSize / m_nPoolSize)*(i));
						DWORD dwByteRangeEnd = ((dwFileSize / m_nPoolSize)*(i + 1)) - 1;
						dwPartSize = dwByteRangeEnd - dwByteRangeStart + 1;
						if (dwAssignedSize != dwPartSize)
						{
							bResumeCancel = false;
							break;
						}
					}
				}
			}
			else
			{
				bResumeCancel = false;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::VerifyResumeDownloadFiles", 0, 0, true, SECONDLEVEL);
	}
	return bResumeCancel;
}

/***************************************************************************
Function       : ResumeDownload
In Parameters  :
Out Parameters : bool
Description    : resume download starts
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::ResumeDownload()
{
	try
	{
		TCHAR szSouceUrl[URL_SIZE] = { 0 };
		GetPrivateProfileString(m_szSectionHeader, _T("UrlPath"), _T(""), szSouceUrl,
			URL_SIZE, m_szINIPath);
		m_dwCurrentFileSize = GetPrivateProfileInt(m_szSectionHeader, _T("FileSize"), 0, m_szINIPath);

		if (m_dwCurrentFileSize == 0)
		{
			return false;
		}

		CDownloadContext *pIExecCurrentContextFirst = new CDownloadContext();
		pIExecCurrentContextFirst->GetHeaderInfo(szSouceUrl, m_sHeaderInfo);
		if (m_sHeaderInfo.dwFileSize == 0)
		{
			return false;
		}
		if (m_dwCurrentFileSize != m_sHeaderInfo.dwFileSize)
		{
			return false;
		}
		bool bETAGMatch = true;
		CDownloadController::m_nPoolSize = GetPrivateProfileInt(m_szSectionHeader,
			_T("Threads"), 0, m_szINIPath);
		TCHAR szETAG[MAX_PATH] = { 0 };
		GetPrivateProfileString(m_szSectionHeader, _T("ETag"), _T(""), szETAG,
			MAX_PATH, m_szINIPath);
		if ((_tcslen(szETAG) > 0) && (_tcslen(m_sHeaderInfo.szETag) > 0))
		{
			if (_tcsstr(m_sHeaderInfo.szETag, szETAG) == NULL)
			{
				bETAGMatch = false;
			}
		}

		if ((!bETAGMatch) || (m_dwCurrentFileSize != m_sHeaderInfo.dwFileSize) || (!VerifyResumeDownloadFiles(m_dwCurrentFileSize)))
		{
			if (pIExecCurrentContextFirst)
			{
				delete pIExecCurrentContextFirst;
				pIExecCurrentContextFirst = NULL;
			}
			return false;
		}
		m_DownloaderThreadPool.CreateThreadPool(CDownloadController::m_nPoolSize);
		for (DWORD i = 0; i < m_nPoolSize; i++)
		{

			CDownloadContext *pIExecCurrentContext = NULL;
			if (i != 0)
			{
				pIExecCurrentContext = new CDownloadContext();
				if (pIExecCurrentContext)
				{
					STRUCT_HEADER_INFO sHeaderInfo = { 0 };
					pIExecCurrentContext->GetHeaderInfo(szSouceUrl, sHeaderInfo);
				}
			}
			else
			{
				pIExecCurrentContext = pIExecCurrentContextFirst;
			}

			TCHAR szSection[MAX_PATH] = { 0 };

			pIExecCurrentContext->m_dwPartNo = i;
			pIExecCurrentContext->m_dwTotalParts = m_nPoolSize;

			wcscpy_s(pIExecCurrentContext->m_strLocalFileName, m_szOrgFileName);
			wcscpy_s(pIExecCurrentContext->m_szAppPath, m_szLocalTempDownloadPath);

			wsprintf(szSection, _T("DownloadedBytesThread_%ld_%ld"), m_nPoolSize, i);
			DWORD dwDownloadedSize = GetPrivateProfileInt(m_szSectionHeader, szSection, 0, m_szINIPath);
			pIExecCurrentContext->m_dwDownloadedSize = dwDownloadedSize;

			wsprintf(szSection, _T("SizeAlloted_%ld_%ld"), m_nPoolSize, i);
			pIExecCurrentContext->m_dwFileSize = GetPrivateProfileInt(m_szSectionHeader,
				szSection, 0, m_szINIPath);

			wsprintf(szSection, _T("StartBytesRange_%ld_%ld"), m_nPoolSize, i);
			pIExecCurrentContext->m_dwByteRangeStart = GetPrivateProfileInt(m_szSectionHeader,
				szSection, 0, m_szINIPath);
			wsprintf(szSection, _T("EndBytesRange_%ld_%ld"), m_nPoolSize, i);
			pIExecCurrentContext->m_dwByteRangeEnd = GetPrivateProfileInt(m_szSectionHeader,
				szSection, 0, m_szINIPath);

			if (pIExecCurrentContext->m_dwFileSize == dwDownloadedSize)
			{
				pIExecCurrentContext->m_bPartComplete = true;
				pIExecCurrentContext->m_dwByteRangeStart = -1L;
				pIExecCurrentContext->m_dwByteRangeEnd = -1L;
			}
			else
			{
				pIExecCurrentContext->m_bPartComplete = false;
				pIExecCurrentContext->m_dwByteRangeStart += dwDownloadedSize;// + (dwDownloadedSize?1:0));
			}

			pIExecCurrentContext->m_objMsgQueue.m_dwTaskItems = DEFAULT_QUEUE_ITEMS;
			m_DownloaderThreadPool.AssignContext(pIExecCurrentContext);
			pIExecCurrentContext->m_bResumDownload = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::ResumeDownload", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function       : InitController
In Parameters  :
Out Parameters : bool
Description    : Based on COntext Object.It an also be a registry object
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::InitController(LPVOID lParam)
{
	try
	{
		LPSTRUCT_DOWNLOAD_INFO pDownloadInfo = (LPSTRUCT_DOWNLOAD_INFO)lParam;

		wcscpy_s(m_szINIPath, m_szLocalTempDownloadPath);
		_tcscpy_s(m_szOrgFileName, pDownloadInfo->szExeName);
		wcscat_s(m_szINIPath, MAX_PATH, pDownloadInfo->szExeName);
		wcscat_s(m_szINIPath, MAX_PATH, _T(".ini"));
		_stprintf_s(m_szDownloadLocalPath, _T("%s%s"), m_szDownloadLocalPath, pDownloadInfo->szExeName);

		CDownloadContext *pIExecCurrentContext = new CDownloadContext();
		pIExecCurrentContext->GetHeaderInfo(m_szUrlPAth, m_sHeaderInfo);
		if (m_sHeaderInfo.dwFileSize == 0)
		{
			return false;
		}
		if (m_dwCurrentFileSize != m_sHeaderInfo.dwFileSize)
		{
			m_dwCurrentFileSize = m_sHeaderInfo.dwFileSize;
		}

		DWORD dwFileSize = m_sHeaderInfo.dwFileSize;
		if (m_dwCurrentFileSize != 0)
		{
			if (m_dwCurrentFileSize != dwFileSize)
			{
				if (dwFileSize)
				{
					AddLogEntry(_T("### Size on server and file size mismatch!"), 0, 0, true, SECONDLEVEL);
				}
				if (pIExecCurrentContext)
				{
					delete pIExecCurrentContext;
					pIExecCurrentContext = NULL;
				}
				return false;
			}
		}
		WritePrivateProfileStringW(m_szSectionHeader, _T("UrlPath"), m_szUrlPAth,
			m_szINIPath);

		TCHAR szValue[MAX_PATH] = { 0 };
		wsprintf(szValue, _T("%d"), dwFileSize);
		WritePrivateProfileStringW(m_szSectionHeader, _T("FileSize"), szValue, m_szINIPath);
		wsprintf(szValue, _T("%d"), m_nPoolSize);
		WritePrivateProfileStringW(m_szSectionHeader, _T("Threads"), szValue, m_szINIPath);
		WritePrivateProfileStringW(m_szSectionHeader, _T("ETag"), m_sHeaderInfo.szETag, m_szINIPath);
		m_DownloaderThreadPool.CreateThreadPool(CDownloadController::m_nPoolSize);
		for (DWORD i = 0; i < m_nPoolSize; i++)
		{
			TCHAR szSection[URL_SIZE] = { 0 };
			if (i == 0)
			{
				wcscpy_s(pIExecCurrentContext->m_strLocalFileName, m_szOrgFileName);
				wcscpy_s(pIExecCurrentContext->m_szAppPath, m_szLocalTempDownloadPath);
				pIExecCurrentContext->m_dwPartNo = i;
				pIExecCurrentContext->m_dwTotalParts = m_nPoolSize;
				pIExecCurrentContext->m_dwByteRangeStart = 0;
				pIExecCurrentContext->m_dwByteRangeEnd = (dwFileSize / m_nPoolSize) - 1;
				if (m_nPoolSize == 1)
				{
					pIExecCurrentContext->m_dwFileSize = pIExecCurrentContext->m_dwByteRangeEnd + 1;
				}
				else
				{
					pIExecCurrentContext->m_dwFileSize = pIExecCurrentContext->m_dwByteRangeEnd + 1;
				}
				pIExecCurrentContext->m_objMsgQueue.m_dwTaskItems = DEFAULT_QUEUE_ITEMS;
				m_DownloaderThreadPool.AssignContext(pIExecCurrentContext);
				wsprintf(szValue, _T("%d"), pIExecCurrentContext->m_dwFileSize);
				wsprintf(szSection, _T("SizeAlloted_%ld_%ld"), m_nPoolSize, i);
				WritePrivateProfileStringW(m_szSectionHeader, szSection, szValue, m_szINIPath);

				wsprintf(szValue, _T("%d"), pIExecCurrentContext->m_dwByteRangeStart);
				wsprintf(szSection, _T("StartBytesRange_%ld_%ld"), m_nPoolSize, i);
				WritePrivateProfileStringW(m_szSectionHeader, szSection, szValue, m_szINIPath);

				wsprintf(szValue, _T("%d"), pIExecCurrentContext->m_dwByteRangeEnd);
				wsprintf(szSection, _T("EndBytesRange_%ld_%ld"), m_nPoolSize, i);
				WritePrivateProfileStringW(m_szSectionHeader, szSection, szValue, m_szINIPath);
			}
			else
			{
				CDownloadContext *pIExecCurrentContext = new CDownloadContext();
				pIExecCurrentContext->SetHeaderInfo(m_sHeaderInfo);
				pIExecCurrentContext->m_dwPartNo = i;
				pIExecCurrentContext->m_dwTotalParts = m_nPoolSize;
				pIExecCurrentContext->m_objMsgQueue.m_dwTaskItems = DEFAULT_QUEUE_ITEMS;
				if (i == m_nPoolSize - 1)
				{
					pIExecCurrentContext->m_dwByteRangeStart = ((dwFileSize / m_nPoolSize)*(i));
					pIExecCurrentContext->m_dwByteRangeEnd = dwFileSize - 1;
					pIExecCurrentContext->m_dwFileSize = (pIExecCurrentContext->m_dwByteRangeEnd
						- pIExecCurrentContext->m_dwByteRangeStart) + 1;
				}
				else
				{
					pIExecCurrentContext->m_dwByteRangeStart = ((dwFileSize / m_nPoolSize)*(i));
					pIExecCurrentContext->m_dwByteRangeEnd = ((dwFileSize / m_nPoolSize)*(i + 1)) - 1;
					pIExecCurrentContext->m_dwFileSize = pIExecCurrentContext->m_dwByteRangeEnd
						- pIExecCurrentContext->m_dwByteRangeStart + 1;
				}

				wcscpy_s(pIExecCurrentContext->m_strLocalFileName, m_szOrgFileName);
				wcscpy_s(pIExecCurrentContext->m_szAppPath, m_szLocalTempDownloadPath);
				STRUCT_HEADER_INFO sHeaderInfo = { 0 };
				pIExecCurrentContext->GetHeaderInfo(m_szUrlPAth, sHeaderInfo);
				m_DownloaderThreadPool.AssignContext(pIExecCurrentContext);
				wsprintf(szValue, _T("%d"), pIExecCurrentContext->m_dwFileSize);
				wsprintf(szSection, _T("SizeAlloted_%ld_%ld"), m_nPoolSize, i);
				WritePrivateProfileStringW(m_szSectionHeader, szSection, szValue, m_szINIPath);

				wsprintf(szValue, _T("%d"), pIExecCurrentContext->m_dwByteRangeStart);
				wsprintf(szSection, _T("StartBytesRange_%ld_%ld"), m_nPoolSize, i);
				WritePrivateProfileStringW(m_szSectionHeader, szSection, szValue, m_szINIPath);

				wsprintf(szValue, _T("%d"), pIExecCurrentContext->m_dwByteRangeEnd);
				wsprintf(szSection, _T("EndBytesRange_%ld_%ld"), m_nPoolSize, i);
				WritePrivateProfileStringW(m_szSectionHeader, szSection, szValue, m_szINIPath);
			}
		}
		m_DownloaderThreadPool.RunThreadPool();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::InitController", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/***************************************************************************
Function       : ControllerThreadProc
In Parameters  : LPVOID lpParameter, 
Out Parameters : DWORD
Description    :
Author		   : Ram Shelke
****************************************************************************/
DWORD WINAPI CDownloadController::ControllerThreadProc(LPVOID lpParameter)
{
	__try{
		CDownloadController *pDownloadController = (CDownloadController *)lpParameter;
		pDownloadController->m_dwStartTime = ::GetTickCount();
		for(DWORD i = 0;i<m_nPoolSize;i++)
		{
			pDownloadController->AssignScanUrl(pDownloadController->m_szUrlPAth, 0);
		}
		pDownloadController->NotifyAllContex();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadController::ControllerThreadProc", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function       : NotifyAllContex
In Parameters  :
Out Parameters : bool
Description    : notify all context
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::NotifyAllContex()
{
	try
	{
		m_DownloaderThreadPool.WaitForLastOperation();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::NotifyAllContex", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function       : AssignScanUrl
In Parameters  : LPCTSTR szItem, int iItemType, 
Out Parameters : bool
Description    : assign download url
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::AssignScanUrl(LPCTSTR szItem, int iItemType)
{
	try
	{
		if (m_nTaskItems >= DEFAULT_QUEUE_ITEMS)
		{
			m_nTaskItems = 0;
			m_nContextIndex++;
			m_pIExecCurrentContext = NULL;
		}

		if (m_nContextIndex >= m_nPoolSize)
		{
			m_nContextIndex = 0;
			m_nTaskItems = 0;
		}
		if (m_pIExecCurrentContext == NULL)
		{
			m_pIExecCurrentContext = (CDownloadContext *)m_DownloaderThreadPool.GetContext(m_nContextIndex);
		}
		if (m_pIExecCurrentContext)
		{
			m_pIExecCurrentContext->m_objMsgQueue.AddQueueItem(szItem, iItemType);
			m_nTaskItems++;
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::AssignScanUrl, URL: [%s]", szItem, 0, true, SECONDLEVEL);

	}
	return false;
}

/***************************************************************************
Function       : StartController
In Parameters  : LPVOID pThis, 
Out Parameters : bool
Description    : start controller
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::StartController(LPVOID lParam)
{
	bool bReturn = false;
	try
	{
		m_bStartController = true;
		m_lpDownloadInfo = (LPSTRUCT_DOWNLOAD_INFO)lParam;
		if (m_lpDownloadInfo)
		{
			_tcscpy_s(m_szUrlPAth, m_lpDownloadInfo->szMainUrl);
			_tcscpy_s(m_szDownloadLocalPath, m_lpDownloadInfo->szLocalPath);
			_tcscpy_s(m_szLocalTempDownloadPath, m_lpDownloadInfo->szLocalTempDownloadPath);
			wcscpy_s(m_szSectionHeader, m_lpDownloadInfo->szSectionName);
			m_dwCurrContentLength = m_lpDownloadInfo->dwFileSize;
			if (m_lpDownloadInfo->bCheckMD5)
			{
				if ((_tcslen(m_lpDownloadInfo->szFileMD5) > 0) && (_tcsicmp(m_lpDownloadInfo->szFileMD5, _T("NA")) != 0))
				{
					wcscpy_s(m_szFileMD5, m_lpDownloadInfo->szFileMD5);
				}
			}
			m_nPoolSize = m_lpDownloadInfo->dwDownloadThreadCount;
			m_dwCurrentFileSize = m_lpDownloadInfo->dwFileSize;
			m_dwCurrentDownloadStatus = 0;
			if (InitController(lParam))
			{
				DWORD dwThreadId = 0;
				m_hCtrlThread = CreateThread(NULL, 0, ControllerThreadProc, this, 0, &dwThreadId);
				DWORD dwReturn = 0;
				if (m_pIGuiInterface)
				{
					m_pIGuiInterface->SetPercentDownload(2);
				}
				DWORD dwTimeout = 1;
				do{
					dwReturn = WaitForSingleObject(m_hCtrlThread, dwTimeout * 1000);
					if (CWWizHttpManager::m_lStopDownload)
					{
						return false;
					}
					if (dwReturn == WAIT_TIMEOUT)
					{
						CheckDownloadStatus(dwTimeout);
						if (dwTimeout == 1)
						{
							dwTimeout = STATUS_INTERVAL;
						}
					}
					else
					{
						break;
					}

				} while (dwReturn == WAIT_TIMEOUT);
				m_dwEndTime = ::GetTickCount();
				CheckDownloadStatus(dwTimeout);

				if (m_dwCurrentDownloadStatus >= 100)
				{
					bReturn = JoinDownloadFiles();
				}
				else
				{
					CString csThreadID;
					csThreadID.Format(_T("%d"), ::GetCurrentThreadId());
					AddLogEntry(_T("### Failed to Download :[%s], ThreadID: [%s]"), m_szUrlPAth, csThreadID);
					bReturn = false;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::StartController", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function       : StopController
In Parameters  :
Out Parameters : bool
Description    : stop the controller and clean up context
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::StopController()
{
	if(!m_bStartController)
	{
		return true;
	}
	__try{
		CancelDownload();
		m_DownloaderThreadPool.StopThreadPool();
		if(m_hCtrlThread)
		{
			DWORD dwErr = ::WaitForSingleObject(m_hCtrlThread,3000);
			if(dwErr == WAIT_TIMEOUT)
			{
				::TerminateThread(m_hCtrlThread,0);
			}
			::CloseHandle(m_hCtrlThread);
			m_hCtrlThread = NULL;
		}

		for(DWORD i = 0;i<m_nPoolSize;i++)
		{
			CDownloadContext *pIExecContext = (CDownloadContext *)m_DownloaderThreadPool.GetContext(i);
			if(pIExecContext)
			{
				delete pIExecContext;
				pIExecContext = NULL;
			}
		}
		ResetInitData();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadController::StopController", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function       : CheckDownloadStatus
In Parameters  :
Out Parameters : void
Description    : check download status
Author		   : Ram Shelke
****************************************************************************/
void CDownloadController::CheckDownloadStatus(DWORD nPassedSeconds)
{
	try
	{
		TCHAR szSection[URL_SIZE] = { 0 };
		TCHAR szValue[MAX_PATH] = { 0 };
		DWORD dwTotalPerc = 0;
		DWORD dwTotalBytes = 0;
		for (DWORD i = 0; i < m_nPoolSize; i++)
		{
			if (CWWizHttpManager::m_lStopDownload)
			{
				return;
			}
			CDownloadContext *pIExecContext = (CDownloadContext *)m_DownloaderThreadPool.GetContext(i);
			if (pIExecContext)
			{
				DWORD  dwDownloaded = 0;
				pIExecContext->GetDownloadStatus(dwDownloaded);
				if (dwDownloaded)
				{
					wsprintf(szSection, _T("DownloadedBytesThread_%ld_%ld"), pIExecContext->m_dwTotalParts,
						pIExecContext->m_dwPartNo);
					wsprintf(szValue, _T("%ld"), dwDownloaded);
					dwTotalBytes += dwDownloaded;
					WritePrivateProfileStringW(m_szSectionHeader, szSection, szValue, m_szINIPath);
					if (dwDownloaded)
					{
						DWORD dwPercent = 0;
						if (pIExecContext->m_dwFileSize != 0)
						{
							dwPercent = static_cast<DWORD>(((static_cast<double>(dwDownloaded)) / pIExecContext->m_dwFileSize) * 100);
						}
						dwTotalPerc += dwPercent;
					}
				}
			}
		}
		float dTransferRateKB = 0;
		if (m_dwCurrDownloadedBytes == 0)
		{
			m_dwCurrDownloadedBytes += dwTotalBytes;
		}
		else
		{
			DWORD dwDiff = dwTotalBytes - m_dwCurrDownloadedBytes;
			dTransferRateKB = static_cast<float>(dwDiff) / (nPassedSeconds * 1024);
			m_dwCurrDownloadedBytes += dwDiff;
		}

		if (m_dwCurrentDownloadStatus < 100)
		{
			m_dwCurrentDownloadStatus = dwTotalPerc / m_nPoolSize;
			if (m_dwCurrentDownloadStatus > 0)
			{
				if (m_pIGuiInterface)
				{
					if (m_dwCurrDownloadedBytes != 0)
					{
						m_pIGuiInterface->SetDownloadedBytes(m_dwCurrentFileSize, dwTotalBytes, dTransferRateKB);
						m_pIGuiInterface->SetPercentDownload(dwTotalPerc / m_nPoolSize);
					}
					else
					{
						m_dwCurrDownloadedBytes = dwTotalBytes;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::CheckDownloadStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : VerifyDownloader
In Parameters  :
Out Parameters : bool
Description    : verify that given download file downloaded successful or not
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::VerifyDownloader()
{
	bool bSuccess = true;
	try
	{
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
		HANDLE hFile = NULL;
		DWORD dwTotalFileSize = 0;
		for (DWORD i = 0; i < m_nPoolSize; i++)
		{
			CDownloadContext *pIExecContext = (CDownloadContext *)m_DownloaderThreadPool.GetContext(i);
			if (pIExecContext)
			{
				DWORD dwFileSize = GetFileSizeEx(pIExecContext->m_szLocalFilePath);
				dwTotalFileSize += dwFileSize;
				DWORD dwDownloadFileSize = pIExecContext->m_dwFileSize;
				if (dwFileSize != dwDownloadFileSize)
				{
					bSuccess = false;
				}
				if (bSuccess == false)
				{
					break;
				}
			}
		}
		if (dwTotalFileSize != m_dwCurrentFileSize)
		{
			bSuccess = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::VerifyDownloader", 0, 0, true, SECONDLEVEL);
	}
	return bSuccess;
}

/***************************************************************************
Function       : JoinDownloadFiles
In Parameters  :
Out Parameters : void
Description    : join the multi part file into one
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::JoinDownloadFiles()
{
	bool bReturn = false;
	try
	{
		if (VerifyDownloader() == false)
		{
			AddLogEntry(_T("### Failed to verify downloaded files. size not match for all temp download parts!"), 0, 0, true, SECONDLEVEL);
			return false;
		}

		TCHAR szBuffer[4096] = { 0 };
		HANDLE hMainFile = NULL;
		HANDLE hFile = NULL;
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
		if ((hMainFile = CreateFile(m_szDownloadLocalPath, GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
		{
			bool bSuccess = false;
			for (DWORD i = 0; i < m_nPoolSize; i++)
			{
				bSuccess = false;
				CDownloadContext *pIExecContext = (CDownloadContext *)m_DownloaderThreadPool.GetContext(i);
				if (pIExecContext)
				{
					if ((hFile = CreateFile(pIExecContext->m_szLocalFilePath, GENERIC_READ,
						FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))
						!= INVALID_HANDLE_VALUE)
					{
						DWORD dwFileSize = GetFileSize(hFile, NULL);
						DWORD dwDonloadFileSize = pIExecContext->m_dwFileSize;
						if (dwFileSize == dwDonloadFileSize)
						{
							DWORD dwRead = 0;
							do
							{
								if (ReadFile(hFile, szBuffer, 4096, &dwRead, 0))
								{
									if (dwRead)
									{
										DWORD dwSize = 0;
										WriteFile(hMainFile, szBuffer, dwRead, &dwSize, NULL);
									}
								}
							} while (dwRead != 0);
							if (hFile)
							{
								FlushFileBuffers(hFile);
								CloseHandle(hFile);
								hFile = NULL;
							}
							DeleteFile(pIExecContext->m_szLocalFilePath);
							bSuccess = true;
						}
					}
				}
				if (!bSuccess)
				{
					bReturn = false;
					break;
				}
			}
			DWORD dwMainFileSize = 0;
			if (hMainFile)
			{
				FlushFileBuffers(hMainFile);
				dwMainFileSize = ::GetFileSize(hMainFile, NULL);
				CloseHandle(hMainFile);
				hMainFile = NULL;
			}

			if (bSuccess)
			{
				if (dwMainFileSize != m_dwCurrentFileSize)
				{
					DeleteFile(m_szDownloadLocalPath);
					DeleteFile(m_szINIPath);

					CString csActualSize, csCurrentFileSize;
					csActualSize.Format(L"ActualSize: [%d]", dwMainFileSize);
					csCurrentFileSize.Format(L"CurrentFileSize: [%d]", m_dwCurrentFileSize);
					AddLogEntry(_T("### File Size not matched: ActualSize: [%s] | CurrentFileSize: [%s]"), csActualSize, csCurrentFileSize, true, SECONDLEVEL);
					bReturn = false;
				}
				else
				{
					DeleteFile(m_szINIPath);
					bReturn = true;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::JoinDownloadFiles", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function       : DeleteFolderTree
In Parameters  : CString csFilePath, 
Out Parameters : bool
Description    :
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::DeleteFolderTree(CString csFilePath)
{
	try
	{
		CFileFind findfile;
		CString csOldFilePath;

		csFilePath += _T("\\*.*");
		//To Check Whether The File Is Exist Or Not
		BOOL bCheck = findfile.FindFile(csFilePath);
		if (!bCheck)
		{
			return false;
		}
		while (bCheck)
		{
			//To Find Next File In Same Directory
			bCheck = findfile.FindNextFile();
			if (findfile.IsDots())
			{
				continue;
			}

			//To get file path
			csFilePath = findfile.GetFilePath();
			csOldFilePath = csFilePath;
			{
				//To set the file attribute to archive
				DWORD dwAttrs = GetFileAttributes(csFilePath);
				if (dwAttrs != INVALID_FILE_ATTRIBUTES && dwAttrs & FILE_ATTRIBUTE_READONLY)
				{
					SetFileAttributes(csFilePath, dwAttrs ^ FILE_ATTRIBUTE_READONLY);
				}
				::DeleteFile(csFilePath);
			}
		}
		//to close handle
		findfile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::DeleteFolderTree", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function       : DeleteController
In Parameters  : void
Out Parameters : void
Description    : Function to delete self object.
Author		   : Ram Shelke
****************************************************************************/
void CDownloadController::DeleteController()
{
	__try
	{
		delete this;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadController::DeleteController", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : ProcessQueueItem
In Parameters  : void
Out Parameters : void
Description    : Function to process queued items
Author		   : Ram Shelke
****************************************************************************/
void CDownloadController::ProcessQueueItem()
{
}

/***************************************************************************
Function       : SetGUIInterface
In Parameters  : IGUIInterface *pIGUI
Out Parameters : void
Description    : Function to gets gui interface object for further communication.
Author		   : Ram Shelke
****************************************************************************/
void CDownloadController::SetGUIInterface(IGUIInterface *pIGUI)
{
	m_pIGuiInterface = pIGUI;	
}

/***************************************************************************
Function       : CancelDownload
In Parameters  : void
Out Parameters : void
Description    : Function which cancel downloading task.
Author		   : Ram Shelke
****************************************************************************/
void CDownloadController::CancelDownload()
{
	try
	{
		CWWizHttpManager::StopDownload();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::CancelDownload", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function       : StartDownload
In Parameters  : void
Out Parameters : void
Description    : Function which start downloading task.
Author		   : Ram Shelke
****************************************************************************/
void CDownloadController::StartDownload()
{
	try
	{
		CWWizHttpManager::StartDownload();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::StartDownload", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : SetThreadPoolStatus
In Parameters  : bool
Out Parameters : bool
Description    : Function which sets thread pool status to pause (or) resume
Author		   : Ram Shelke
****************************************************************************/
void CDownloadController::SetThreadPoolStatus(bool bPause)
{
	try
	{
		if (bPause)
		{
			m_DownloaderThreadPool.PauseThreadPool();
		}
		else
		{
			m_DownloaderThreadPool.ResumeThreadPool();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::SetThreadPoolStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : CheckMD5
In Parameters  : void
Out Parameters : bool
Description    : Function to check MD5 of file.
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::CheckMD5()
{
	bool bRet = false;
	return bRet;
}

/***************************************************************************
Function       : CheckForInternetConnection
In Parameters  : void
Out Parameters : bool
Description    : Function to check internet connection
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadController::CheckForInternetConnection()
{
	try
	{
		return CWWizHttpManager::CheckInternetConnection();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadController::CheckForInternetConnection", 0, 0, true, SECONDLEVEL);
	}
	return false;
}
