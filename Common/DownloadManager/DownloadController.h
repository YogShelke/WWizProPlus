/**********************************************************************************************************
*	Program Name			: DownloadController.h
*	Description				: Class which handles download the files in parts to achieve best download
*							  Performance.
*	Author Name				: Ramkrushna Shelke
*	Date Of Creation		: 14 Jan 2017
*	Version No				: 2.6.0.121
*	Special Logic Used		:
*	Modification Log		:
***********************************************************************************************************/
#pragma once
#include "DownloadContext.h"
#include "ThreadPool.h"
#include "IController.h"
#include "DownloadConts.h"
#include "MessageQueue.h"
#include "ispyCriticalSection.h"

class CDownloadController :
	public IController
{

public:
	static DWORD m_nPoolSize;
	CDownloadController(void);
	~CDownloadController(void);
	void SetThreadPoolStatus(bool bPause);
	bool InitController(LPVOID lParam);
	bool StartController(LPVOID lParam);
	bool StopController();
	void DeleteController();
	bool JoinDownloadFiles();
	bool VerifyDownloader();
	void CheckDownloadStatus(DWORD nPassedSeconds);
	static DWORD WINAPI ControllerThreadProc(LPVOID lpParameter);
	bool AssignScanUrl(LPCTSTR szItem, int iItemType);
	bool NotifyAllContex();
	TCHAR m_szDownloadLocalPath[MAX_PATH];
	TCHAR m_szLocalTempDownloadPath[MAX_PATH];
	TCHAR m_szOrgFileName[MAX_PATH];
	TCHAR m_szFileMD5[MAX_PATH];
	bool ResumeDownload();
	bool DeleteFolderTree(CString csFilePath);
	void ResetInitData();
	CMessageQueue m_objDownloadQueue;
	CMessageQueue m_objDownloadPriorityQueue;
	CMessageQueueItem m_objCurrQueueItem;
	void ProcessQueueItem();
	LPSTRUCT_DOWNLOAD_INFO m_lpDownloadInfo;
	void SetGUIInterface(IGUIInterface *pIGUI);
	void CancelDownload();
	void StartDownload();
	bool CheckForInternetConnection();
private:
	TCHAR m_szINIPath[MAX_PATH];
	TCHAR m_szUrlPAth[URL_SIZE];
	TCHAR m_szSectionHeader[MAX_PATH];
	DWORD m_dwCurrContentLength;
	DWORD m_nTaskItems;
	DWORD m_nContextIndex;
	CThreadPool m_DownloaderThreadPool;
	CDownloadContext *m_pIExecCurrentContext;
	DWORD m_dwStartTime;
	DWORD m_dwEndTime;
	STRUCT_HEADER_INFO m_sHeaderInfo;
	DWORD GetFileSizeEx(TCHAR *szFilePath);
	bool VerifyResumeDownloadFiles(DWORD dwFileSize);
	DWORD m_dwCurrentDownloadStatus;
	DWORD m_dwCurrentFileSize;
	IGUIInterface *m_pIGuiInterface; 
	DWORD m_dwCurrDownloadedBytes;
	bool m_bResumeDownload;
	HANDLE m_hCtrlThread;
	bool m_bStartController;
	bool CheckMD5();
};
