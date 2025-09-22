/**********************************************************************************************************
*	Program Name			: DownloadContext.cpp
*	Description				: Class which handles downloading with Multiple threads using Queue mechanism.
*	Author Name				: Ramkrushna Shelke
*	Date Of Creation		: 14 Jan 2017
*	Version No				: 2.6.0.121
*	Special Logic Used		:
*	Modification Log		:
***********************************************************************************************************/
#include "StdAfx.h"
#include "WinHttpManager.h"
#include "DownloadContext.h"
#include "WinHttpManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************
Function       : CDownloadContext
In Parameters  : void, 
Out Parameters :
Description    : constructor
Author		   : Ram Shelke
****************************************************************************/
CDownloadContext::CDownloadContext(void)
{
	m_dwFileSize = 0;
	m_dwFailedHeaderCnt = 0;
	m_dwCurrFileSize = 0;
	m_dwCurrReadBytes = 1;
	m_dwCurrTotalBytes =0;
	m_dwTotalQueueCnt = 0;
	m_dwByteRangeStart = 0;
	m_dwByteRangeEnd = 0;
	m_dwDownloadedSize = 0;
	m_hConnect = NULL;
	m_bResumDownload = false;
	m_dwPartNo = 0;
	::ZeroMemory(m_szLocalFilePath,sizeof(m_szLocalFilePath));
	::ZeroMemory(m_szAppPath,sizeof(m_szLocalFilePath));
	::ZeroMemory(m_strLocalFileName,sizeof(m_szLocalFilePath));
	m_bPartComplete = false;
}

/***************************************************************************
Function       : ~CDownloadContext
In Parameters  : void, 
Out Parameters :
Description    : destructor
Author		   : Ram Shelke
****************************************************************************/
CDownloadContext::~CDownloadContext(void)
{

}

/***************************************************************************
Function       : GetHeaderInfo
In Parameters  : TCHAR * szSourceUrl, 
Out Parameters : STRUCT_HEADER_INFO
Description    : retrive header info
Author		   : Ram Shelke
****************************************************************************/
void CDownloadContext::GetHeaderInfo(TCHAR * szSourceUrl, STRUCT_HEADER_INFO &sHeaderInfo)
{
	try
	{
		if (CWWizHttpManager::m_lStopDownload)
		{
			return;
		}

		TCHAR szInfo[MAX_PATH] = { 0 };
		TCHAR szETAG[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH*sizeof(TCHAR);
		if (m_objwinHttpManager.Initialize(szSourceUrl))
		{
			if (m_dwPartNo == 0)
			{
				m_objwinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen);
				dwBufLen = MAX_PATH*sizeof(TCHAR);
				m_objwinHttpManager.GetHeaderInfo(WINHTTP_QUERY_ETAG, szETAG, dwBufLen);
			}
			sHeaderInfo.dwFileSize = _wtol(szInfo);
			sHeaderInfo.hSession = m_objwinHttpManager.m_hSession;
			sHeaderInfo.hConnect = m_objwinHttpManager.m_hConnect;
			sHeaderInfo.hRequest = m_objwinHttpManager.m_hRequest;
			if (_tcslen(szETAG) > 0)
			{
				_tcscpy_s(sHeaderInfo.szETag, szETAG);
			}
			_tcscpy_s(sHeaderInfo.szHostName, m_objwinHttpManager.m_szHostName);
			_tcscpy_s(sHeaderInfo.szMainUrl, m_objwinHttpManager.m_szMainUrl);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadContext::GetHeaderInfo, SourceUrl: [%s]", szSourceUrl, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : SetHeaderInfo
In Parameters  : STRUCT_HEADER_INFO sHeaderInfo, 
Out Parameters : bool
Description    :
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadContext::SetHeaderInfo(STRUCT_HEADER_INFO &sHeaderInfo)
{
	try
	{
		m_objwinHttpManager.m_hSession = sHeaderInfo.hSession;
		m_objwinHttpManager.m_hConnect = sHeaderInfo.hConnect;
		//wcscpy_s(m_objwinHttpManager.m_szHostName, sHeaderInfo.szHostName);
		//wcscpy_s(m_objwinHttpManager.m_szMainUrl, sHeaderInfo.szMainUrl);
		m_objwinHttpManager.m_bSharedSession = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadContext::SetHeaderInfo", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function       : DeleteContext
In Parameters  :
Out Parameters : void
Description    :
Author		   : Ram Shelke
****************************************************************************/
void CDownloadContext::DeleteContext()
{
	__try
	{
		delete this;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadContext::DeleteContext", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : Initialize
In Parameters  : HANDLE hQueueEvent, 
Out Parameters : bool
Description    :
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadContext::Initialize(HANDLE hQueueEvent)
{
	__try
	{
		//Do not close this handle
		m_objMsgQueue.m_hQueueEvent = hQueueEvent;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadContext::Initialize", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function       : Run
In Parameters  : bool
Out Parameters : bool
Description    : Function which run thread pool items.
Author		   : Ram Shelke
****************************************************************************/
bool CDownloadContext::Run(bool bLastOperation)
{
	try
	{
		if (!bLastOperation)
		{
			m_dwTotalQueueCnt = (DWORD)m_objMsgQueue.m_MsgQueue.size();
		}

		while (m_objMsgQueue.FetchQueueItem(m_objCurrQueueItem))
		{
			if (CWWizHttpManager::m_lStopDownload)
			{
				return false;
			}

			CString csTemp;
			csTemp.Format(_T("%s%s_%d_%d.tmp"), m_szAppPath, m_strLocalFileName, m_dwTotalParts, m_dwPartNo);
			wcscpy_s(m_szLocalFilePath, _countof(m_szLocalFilePath), csTemp);
			if (m_bResumDownload == false)
			{
				DeleteFile(m_szLocalFilePath);
			}
			m_objwinHttpManager.SetDownloadStatus(m_dwDownloadedSize);
			if (m_bPartComplete)
			{
				continue;
			}
			m_objwinHttpManager.Download(m_szLocalFilePath, m_dwByteRangeStart, m_dwByteRangeEnd);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadContext::Run", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
Function       :
In Parameters  : DWORD dwSize, 
Out Parameters : LPVOID Allocate
Description    :
Author		   : Ram Shelke
****************************************************************************/
LPVOID CDownloadContext::Allocate (DWORD dwSize)
{
	__try
	{
		return (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize));
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadContext::Allocate", 0, 0, true, SECONDLEVEL);
		return NULL;
	}
	return NULL;
}

/***************************************************************************
Function       :
In Parameters  : LPVOID& pVPtr, 
Out Parameters : void Release
Description    :
Author		   : Ram Shelke
****************************************************************************/
void CDownloadContext::Release(LPVOID& pVPtr)
{
	__try
	{
		HeapFree(GetProcessHeap(), 0, pVPtr);
		pVPtr = NULL;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadContext::Release", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : NotifyQueueEvent
In Parameters  :
Out Parameters : void
Description    :
Author		   : Ram Shelke
****************************************************************************/
void CDownloadContext::NotifyQueueEvent()
{
	__try
	{
		if (m_objMsgQueue.m_hQueueEvent)
		{
			::SetEvent(m_objMsgQueue.m_hQueueEvent);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDownloadContext::NotifyQueueEvent", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : DumpStatistics
In Parameters  :
Out Parameters : void
Description    :
Author		   : Ram Shelke
****************************************************************************/
void CDownloadContext::DumpStatistics()
{
}

/***************************************************************************
Function       : GetDownloadStatus
In Parameters  : DWORD & dwDownloaded, 
Out Parameters : void
Description    :
Author		   : Ram Shelke
****************************************************************************/
void CDownloadContext::GetDownloadStatus(DWORD & dwDownloaded)
{
	try
	{
		m_objwinHttpManager.GetDownloadStatus(dwDownloaded);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDownloadContext::GetDownloadStatus", 0, 0, true, SECONDLEVEL);
	}
}