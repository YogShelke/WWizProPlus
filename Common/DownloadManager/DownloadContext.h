/**********************************************************************************************************
*	Program Name			: DownloadContext.cpp
*	Description				: Class which handles downloading with Multiple threads using Queue mechanism.
*	Author Name				: Ramkrushna Shelke
*	Date Of Creation		: 14 Jan 2017
*	Version No				: 2.6.0.121
*	Special Logic Used		:
*	Modification Log		:
***********************************************************************************************************/
#pragma once
#include "iexeccontext.h"
#include "MessageQueue.h"
#include "WWizHttpManager.h"

const int MAX_READBUFF_SIZE = 65536;
class CDownloadContext : public IExecContext
{
public:
	CDownloadContext(void);
	~CDownloadContext(void);

	CMessageQueue m_objMsgQueue;
	DWORD m_dwFileSize;
	DWORD m_dwFailedHeaderCnt;
	DWORD m_dwByteRangeStart;
	DWORD m_dwByteRangeEnd;
	DWORD m_dwTotalParts;
	DWORD m_dwPartNo;
	DWORD m_dwDownloadedSize;
	TCHAR m_szLocalFilePath[MAX_PATH];
	TCHAR m_szAppPath[MAX_PATH];
	TCHAR m_strLocalFileName[MAX_PATH];
	HINTERNET m_hConnect;
	bool m_bResumDownload;

	void DumpStatistics();
	void DeleteContext();
	void GetDownloadStatus(DWORD & dwDownloaded);
	void GetHeaderInfo(TCHAR * szSourceUrl, STRUCT_HEADER_INFO &sHeaderInfo );
	bool SetHeaderInfo(STRUCT_HEADER_INFO &sHeaderInfo);
	bool Initialize(HANDLE hQueueEvent);
	bool Run(bool bLastOperation = false);
	void NotifyQueueEvent();
	bool m_bPartComplete;
private:
	CMessageQueueItem m_objCurrQueueItem;
	DWORD m_dwCurrFileSize;
	DWORD m_dwCurrReadBytes;
	DWORD m_dwCurrTotalBytes;
	DWORD m_dwTotalQueueCnt;
	CWWizHttpManager m_objwinHttpManager;

	bool EnumFolder(LPCTSTR szFolderPath);
	LPVOID Allocate (DWORD dwSize);
	void Release(LPVOID& pVPtr);
};
