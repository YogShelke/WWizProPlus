#include "sciter-x-threads.h"
#include "DownloadConts.h"
#include "iSpyMemMapClient.h"
#include "WinHttpClient.h"
#include "WinHttpManager.h"
#include "WardWizUpdateManager.h"

// WardWizUpdateDlg.h : header file
//
#pragma once

typedef DWORD(*UNZIPFILE)			(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount);
typedef bool(*STOPUNRAROPERATION)   (void);

class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CWardWizUpdateDlg dialog
class CWardWizUpdateDlg : public CDialogEx,
	public CSciterBase,
	public sciter::host<CWardWizUpdateDlg> // Sciter host window primitives
{
// Construction
public:
	CWardWizUpdateDlg(CWnd* pParent = NULL);	// standard constructor

	HWINDOW					get_hwnd();
	HINSTANCE				get_resource_instance();
	sciter::dom::element	m_root_el;

// Dialog Data
	enum { IDD = IDD_WARDWIZUPDATE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	HICON		m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonMinimize();

	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:

	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetProductID", On_GetProductID) // On_Minimize()
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("OnMinimize", On_Minimize)

		//Updates relatedd functions
		FUNCTION_2("OnClickUpdatesFromLocalFolder", On_ClickUpdatesFromLocalFolder)//On_UpdatesNextButton
		FUNCTION_5("OnClickUpdateFromInternet", On_ClickUpdateFromInternet)//On_ClickUpdateFromInternet
		FUNCTION_0("OnPauseUpdates", On_PauseUpdates)//On_PauseResumeUpdates
		FUNCTION_0("OnResumeUpdates", On_ResumeUpdates)//On_PauseResumeUpdates
		FUNCTION_2("OnClickStopUpdates", On_ClickStopUpdates)//On_ClickStopUpdates
		FUNCTION_1("SendNotificationMessage", On_SendNotificationMessageCB)
		FUNCTION_0("OnPauaseOfflineUpdate", On_PauaseOfflineUpdate);
		FUNCTION_0("OnResumeOfflineUpdate", On_ResumeOfflineUpdate);
		FUNCTION_0("OnCloseOfflineUpdate", On_CloseOfflineUpdate);
		FUNCTION_1("SendUpdateType", Send_UpdateType);
		FUNCTION_3("onModalLoop", onModalLoop);
		FUNCTION_0("onClose", On_Close);
		FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)

	END_FUNCTION_MAP

	json::value OnGetAppPath();
	json::value On_GetThemeID();
	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value On_Minimize();
	json::value On_Close();

	//Updates related functions
	json::value On_ClickUpdatesFromLocalFolder(SCITER_VALUE svFilePath, SCITER_VALUE svNotificationMessageFromLocalCB);
	json::value On_ClickUpdateFromInternet(SCITER_VALUE m_pSetUpdateStatusCb, SCITER_VALUE m_pAddUpdateTableCb, SCITER_VALUE m_pUpdateUpdateTableCb, SCITER_VALUE m_pRowAddCb, SCITER_VALUE m_pUpdateCompleteCb);
	json::value On_PauseUpdates();
	json::value On_ResumeUpdates();
	json::value On_ClickStopUpdates(SCITER_VALUE svIsStopFrmTaskbar, SCITER_VALUE svbIsManualStop);
	json::value On_SendNotificationMessageCB(SCITER_VALUE svFunNotificationMessageCB);
	json::value On_PauaseOfflineUpdate();
	json::value On_ResumeOfflineUpdate();
	json::value On_CloseOfflineUpdate();
	json::value Send_UpdateType(SCITER_VALUE m_csUpdateType);
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value FunCheckInternetAccessBlock();

	SCITER_VALUE				m_svLiveUpdateStatusFunctionCB;
	SCITER_VALUE				m_svAddUpdateTableCb;
	SCITER_VALUE				m_svUpdateUpdateTableCb;
	SCITER_VALUE				m_svRowAddCb;
	SCITER_VALUE				m_svUpdateCompleteCb;
	SCITER_VALUE				m_svNotificationMessageFromLocalCB;
	SCITER_VALUE				m_svFunDisplayDownloadType;
	SCITER_VALUE				m_svUpdateDownloadTypeCB;
	SCITER_VALUE				m_svFunNotificationMessageCB;
	SCITER_VALUE				m_svUpdateStatusFunctionCB;
	SCITER_VALUE				m_svLiveAddUpdateTableCb;
	SCITER_VALUE				m_svLiveUpdateUpdateTableCb;
	SCITER_VALUE				m_svLiveRowAddCb;
	SCITER_VALUE				m_svLiveUpdateCompleteCb;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;

	CString ExtractRARFile(CString csInputFileName, DWORD &dwUnzipCount);
	CString GetAppFolderPath();
	CString	GetModuleFilePath();

	DWORD UpdateFromLocalFolder(std::vector<CString> &csVectInputFiles);
	DWORD CopyFromLocalFolder2InstalledFolder(std::vector<CString> &csVectInputFiles);
	DWORD ValidateDB_File(TCHAR *m_szFilePath, DWORD &dwStatus, CString &csInputPathProgramData);
	DWORD CheckForValidVersion(CString csVersionNo);
	DWORD ReadDBVersionFromReg();
	DWORD SendData2CommService(int iMesssageInfo, bool bWait);

	int ConvertBytes2KB(int iCurrentDownloadBytes);
	int ConvertBytes2MB(int iCurrentDownloadKB);
	int ConvertBytes2GB(int iCurrentDownloadMB);

	static void UpdateOpProcess(DWORD dwFlag, DWORD dwPercent, DWORD dwTotalFileSize, DWORD dwCurrentDownloadedbytes, void * param);
	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString, DWORD dwFlagUpdt);
	void EnumFolder(LPCTSTR pstr);
	void CloseALUpdateProcess();
	void OnBnClickedButtonNext();
	void CallUISetPauseStatusfunction(CString csData);
	void UpdateTimeDate();
	void InsertItem(CString csInsertItem, CString csActualStatus);
	void OnAddUpdateStatus(CString csStatus);
	void ShowUpdateCompleteMessage();
	void ShowLiveUpdate();
	void StartEPSLiveUpdateNOUI();
	void InitializeUpdateProcThread();
	void CloseUpdtNoUISrv();
	void TheadTerminationState(HANDLE hThread);

	bool LoadExtractDll();
	bool ShutDownDownload();
	bool StopUpdates(SCITER_VALUE svIsStopFrmTaskbar, SCITER_VALUE svbIsManualStop);
	bool UpdateVersionIntoRegistry();
	bool CheckForValidUpdatedFiles(CString csInputFolder, std::vector<CString> &csVectInputFiles);
	bool StartDownloading();
	bool StartALUpdateUsingALupdateService();
	bool CreateDirectoryFocefully(LPTSTR lpszPath);
	bool ValidateFileNVersion(CString csFileName, CString csVersion, DWORD dwDBVersionLength);
	bool EnumAndDeleteTempFolder(CString csInputFileName);
	bool CheckForMaxVersionInZip(CString csVersionNo);
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait);
	bool ShutDownDownloadLiveupdates();
	bool SendRequestCommon(int iRequest, bool bWait = false);
	bool PauseLiveUpdate();
	bool ResumeLiveUpdateThread();
	bool StopLiveUpdates();
	bool UpDateDowloadStatus(DWORD dwFlag, DWORD dwPercent, DWORD dwTotalFileSize, DWORD dwCurrentDownloadedbytes, void* param);

	CWardWizUpdateManager		m_objWardWizUpdateManager;
	eLIVE_UPDATE_TYPE			m_updateType;
	CStringArray				m_csFilesList;

	HANDLE						m_hThread_StartALUpdateProcess;
	HANDLE						m_hThreadStartUpdateManagerTask;
	HANDLE						m_hUpdateFromLocalFolderThread;
	HMODULE						m_hZip;
	HMODULE						m_hStopUnRarOperation;
	UNZIPFILE					m_UnzipFile;
	STOPUNRAROPERATION			m_StopUnRarOperation;

	CString						m_csInputFolderPath;
	CString						m_csVersionNo;
	CString						m_csFileName;
	CString						m_CurrentFilePath;
	DWORD						m_dwMaxVersionInZip;
	DWORD						m_dwCurrentLocalFileSize;
	INT64						m_iUpdateID;
	int							m_iTotalNoofFiles;
	bool						m_bOlderdatabase;
	bool						m_bdatabaseuptodate;
	bool						m_bOfflineUpdateStarted;
	bool						m_bAborted;
	bool						m_bisDateTime;
	bool						m_bIsStopFrmTaskbar;
	bool						m_bIsManualStop;

	//live update declaration
	CString						m_csDispTotSizeExt;
	CString						m_csTotalFileSize;
	CString						m_csDownloadPercentage;
	CString						m_csCurrentDownloadedbytes;
	CString						m_csListControlStatus;
	CString						m_csUpdateType;
	TCHAR						m_szAllUserPath[512];
	DWORD						m_dwstatusMsg;
	DWORD						m_iTotalFileSize;
	DWORD						m_iTotalFileSizeCount;
	DWORD						m_dwPercentage;
	INT64						m_iDBUpdateID;
	bool						m_bIsCheckingForUpdateEntryExistInList;
	bool						m_bDownLoadInProgress;
	bool                        m_bISalreadyDownloadAvailable;
	bool						m_bCloseUpdate;
	bool						m_bOnCloseFromMainUI;
	int							m_iCurrentDownloadBytes;
	int							m_iCount;
	int							m_iRowCount;
	int							m_iFileCount;
	int							m_idivision;
	int							m_iIntFilesSize;

};