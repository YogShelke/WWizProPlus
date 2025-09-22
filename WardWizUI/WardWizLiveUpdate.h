/**********************************************************************************************************
Program Name          : WardWizLiveUpdate.cpp
Description           : This class contains the functionality for updating product.
It has 2 options
1) Update from internet.
2) Update from Local Folder.
Author Name			  : Nihar Deshpande
Date Of Creation      : 27th May 2016
Version No            : 2.0.0.14
Special Logic Used    :

Modification Log      :
1. Nihar           Created WardWizLiveUpdate class   27th May 2016
***********************************************************************************************************/
#pragma once

class CWardWizLiveUpdate
{
public:
	CWardWizLiveUpdate();
	~CWardWizLiveUpdate();

	bool						m_bIsCheckingForUpdateEntryExistInList;
	bool						m_bDownLoadInProgress;
	bool						m_bisDateTime;
	bool						m_isDownloading;
	bool                        m_bISalreadyDownloadAvailable;
	bool						m_bIsStopFrmTaskbar;
	bool						m_bIsManualStop;
	bool						m_bCloseUpdate;
	bool						m_bOnCloseFromMainUI;
	DWORD						m_dwstatusMsg;
	DWORD						m_iTotalFileSize;
	DWORD						m_iTotalFileSizeCount;
	CString						m_csDispTotSizeExt;
	DWORD						m_dwPercentage;
	int							m_iCurrentDownloadBytes;
	int							m_iCount;
	int							m_iRowCount;
	int							m_iFileCount;
	int							m_idivision;
	int							m_iIntFilesSize;
	INT64						m_iDBUpdateID;
	CString						m_csTotalFileSize;
	CString						m_csDownloadPercentage;
	CString						m_csCurrentDownloadedbytes;
	CString						m_csListControlStatus;
	SCITER_VALUE				m_svUpdateStatusFunctionCB;
	SCITER_VALUE				m_svLiveAddUpdateTableCb;
	SCITER_VALUE				m_svLiveUpdateUpdateTableCb;
	SCITER_VALUE				m_svLiveRowAddCb;
	SCITER_VALUE				m_svLiveUpdateCompleteCb;
	TCHAR						m_szAllUserPath[512];
	CString						m_csUpdateType;
	

	DWORD UpdateFromLocalFolder(std::vector<CString> &csVectInputFiles);
	DWORD CopyFromLocalFolder2InstalledFolder(std::vector<CString> &csVectInputFiles);

	void UpdateTimeDate();
	
	void InsertItem(CString csInsertItem, CString csActualStatus);
	void OnAddUpdateStatus(CString csStatus);
	void ShowUpdateCompleteMessage();

	bool StartDownloading();
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait);
	bool UpDateDowloadStatus(LPISPY_PIPE_DATA lpSpyData);
	bool SendRequestCommon(int iRequest, bool bWait = false);
	bool PauseUpdateLiveUpdate();
	bool ResumeUpdateLiveUpdate();
	bool StopLiveUpdates();
	bool ShutDownDownloadLiveupdates();
	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
	int ConvertBytes2KB(int iCurrentDownloadBytes);
	int ConvertBytes2MB(int iCurrentDownloadKB);
	int ConvertBytes2GB(int iCurrentDownloadMB);

public:

	SCITER_VALUE						m_svFunNotificationMessageCB;
	SCITER_VALUE						m_svUpdateDownloadTypeCB;
};

