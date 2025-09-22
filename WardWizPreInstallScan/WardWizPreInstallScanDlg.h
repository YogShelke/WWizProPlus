
// WardWizPreInstallScanDlg.h : header file
//

#pragma once
#include "stdafx.h"
#include "sciter-x-threads.h"
#include "iTINRegWrapper.h"
#include "ScannerContants.h"
#include "ISpyScanner.h"
#include "ISpyDBManipulation.h"
#include "ScanByName.h"

class CSciterBase :
	public sciter::event_handler // Sciter DOM event handling
{

};

// CWardWizPreInstallScanDlg dialog
class CWardWizPreInstallScanDlg : public CDialogEx,
	public CSciterBase,
	sciter::behavior_factory,
	public sciter::host <CWardWizPreInstallScanDlg>
{
// Construction
public:
	CWardWizPreInstallScanDlg(CWnd* pParent = NULL);	// standard constructor
	~CWardWizPreInstallScanDlg();

	HELEMENT				self;
	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}
// Dialog Data
	enum { IDD = IDD_WARDWIZPREINSTALLSCAN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();

	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();
	sciter::dom::element			m_root_el;

	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnCloseButton", On_Close) // On_Close()
		FUNCTION_3("OnStartQuickScan", On_StartQuickScan)
		FUNCTION_3("OnSysFileScan", On_SysFileScan)
		FUNCTION_1("OnPauseQuickScan", On_PauseQuickScan)
		FUNCTION_1("OnResumeQuickScan", On_ResumeQuickScan)
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_2("OnClickScanCleanBtn", On_ClickScanCleanBtn)
		FUNCTION_1("OnRestartForBootScan", On_RestartForBootScan)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value On_GetProductID();
	json::value On_RestartForBootScan(SCITER_VALUE svBootFlag);
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value On_Close();
	json::value On_PauseQuickScan(SCITER_VALUE svFunPauseResumeFunCB);
	json::value On_ResumeQuickScan(SCITER_VALUE svFunPauseResumeFunCB);
	json::value On_ClickScanCleanBtn(SCITER_VALUE svArrCleanEntries, SCITER_VALUE svQarantineFlag);
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value On_StartQuickScan(SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB);
	json::value On_SysFileScan(SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB);
	json::value On_GetThemeID();

public:
	void LoadRecoversDBFile();
	void EnumerateProcesses();
	void StartScanning();
	void ScanForSingleFile(CString csFilePath);
	void EnumFolder(LPCTSTR pstr);
	void EnumFolderForScanning(LPCTSTR pstr);
	void OnTimerScan();
	void GetModuleCount();
	void LoadPSAPILibrary();
	void QuaratineEntries();
	void WriteFileForBootScan(CString csQurFilePaths, CString csVirusName);
	void CallUISetScanFinishedStatus(CString csData);
	void CallUISetFileCountfunction(CString csData, CString csCurrentFileCount);
	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
	void CallUISetStatusfunction(LPTSTR lpszPath);
	void CallUISetStatusfunctionSEH(LPTSTR lpszPath);
	void CallUISetPauseStatusfunction(CString csData);
	void CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID);

	bool OnClickCleanButton(SCITER_VALUE svArrayCleanEntries);
	bool InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR m_csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus);
	bool QuarantineEntry(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath);
	bool BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath, LPTSTR lpszBackupFilePath);
	bool IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize);
	bool CheckIFAlreadyBackupTaken(LPCTSTR szFileHash, LPTSTR szBackupPath);
	bool CheckEntryPresent(LPCTSTR szFileHash, LPTSTR szBackupPath);
	bool CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize);
	bool IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize);
	bool CheckFileOrFolderOnRootPath(CString csFilePath);
	bool GetScanningPaths(CStringArray &csaReturn);
	bool GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash);
	bool AddBootScannerEntry(LPTSTR pKeyName, LPTSTR pValueName, LPTSTR pNewValue);
	bool SaveInRecoverDB();
	bool ScanFinished();

	int GetTaskBarWidth();
	int GetTaskBarHeight();

	DWORD HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThreatName1, DWORD dwISpyID, CString &csBackupFilePath, DWORD &dwAction);
	DWORD Encrypt_File(TCHAR *m_szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus);
	DWORD CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath);
	DWORD LoadSignatureDatabase(DWORD &dwSigCount);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	DWORD UnLoadSignatureDatabase();
	DWORD GetSelectedLanguage();
	CString GetQuarantineFolderPath();
	CString GetProgramFilePath();
	CString GetString(CString csID);
	CString GetModuleFileStringPath();
	void ForceTerminate();

	ENUMPROCESSMODULESEX	EnumProcessModulesWWizEx;

	CStringList				m_csaModuleList;
	CStringArray			m_csaAllScanPaths;

	CString					m_csCurrentFilePath;
	CString					m_csPreviousPath;
	CString					m_csISpyID;

	bool					m_bManualStop;
	bool					m_bIsPathExist;
	bool					m_bStop;
	bool					m_bIsManualStop;
	bool					m_bIsManualStopScan;
	//bool					m_bQuickScan;
	bool					m_bRedFlag;
	bool					m_ScanCount;
	bool					m_bScnAborted;
	bool					m_bScanStartedStatusOnGUI;
	bool					m_bRescan;
	bool					m_bFileFailedToDelete;

	int						m_iThreatsFoundCount;
	int						m_iTotalFileCount;
	
	unsigned char			*m_pbyEncDecKey;

	DWORD					m_FileScanned;
	DWORD					m_dwTotalFileCount;
	DWORD					m_iMemScanTotalFileCount;
	DWORD					m_dwVirusFoundCount;
	DWORD					m_dwVirusCleanedCount;
	DWORD					m_dwAutoQuarOption;
	DWORD					m_wISpywareID;
	HANDLE					m_hThread_ScanCount;
	HANDLE					m_hWardWizAVThread;
	HANDLE					m_hQuarantineThread;

	HMODULE					m_hPsApiDLL;

	SCANTYPE				m_eCurrentSelectedScanType;
	SCANTYPE				m_eScanType;

	CTime					m_tsScanStartTime;
	CTime					m_tsScanEndTime;
	CTimeSpan				m_tsScanPauseResumeElapsedTime;
	CISpyScanner			m_objISpyScanner;
	CISpyCriticalSection	m_csQuarentineEntries;
	CISpyCriticalSection	m_csScanFile;
	CWardwizLangManager		m_objwardwizLangManager;
	CISpyDBManipulation		m_objISpyDBManipulation;

	ULARGE_INTEGER			m_uliFreeBytesAvailable;     // bytes disponiveis no disco associado a thread de chamada
	ULARGE_INTEGER			m_uliTotalNumberOfBytes;     // bytes no disco
	ULARGE_INTEGER			m_uliTotalNumberOfFreeBytes; // bytes livres no disco

	SCITER_VALUE			m_svAddVirusFoundEntryCB;
	SCITER_VALUE			m_svSetScanFinishedStatusCB;
	SCITER_VALUE			m_svFunNotificationMessageCB;
	SCITER_VALUE			m_svSetPauseStatusCB;
	SCITER_VALUE			m_svSetVirusUpdateStatusCB;
	SCITER_VALUE			m_svArrayCleanEntries;
	SCITER_VALUE			m_svVirusCount;

	virtual bool on_timer(HELEMENT he);
	virtual bool on_timer(HELEMENT he, UINT_PTR extTimerId);
	bool on_timerSEH(HELEMENT he);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual bool handle_timer(HELEMENT he, TIMER_PARAMS& params)
	{
		__try
		{
			if (params.timerId)
				return on_timer(he, params.timerId);
			return on_timer(he);
		}
		__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
		{
			AddLogEntry(L"### Exception in CWardwizScan::handle_timer", 0, 0, true, SECONDLEVEL);
		}
		return true;
	}
};
