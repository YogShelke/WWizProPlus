
// WardWizMemScanDlg.h : header file
//

#pragma once
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

// CWardWizMemScanDlg dialog
class CWardWizMemScanDlg : public CDialogEx,
	public CSciterBase,
	sciter::behavior_factory,
	public sciter::host < CWardWizMemScanDlg > // Sciter host window primitives
{
	// Construction
public:
	CWardWizMemScanDlg(CWnd* pParent = NULL);	// standard constructor
	~CWardWizMemScanDlg();

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
	enum { IDD = IDD_WARDWIZMEMSCAN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();
	sciter::dom::element			m_root_el;
	static CWardWizMemScanDlg	 	*m_pThis;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("OnCloseButton", On_Close) // On_Close()
		FUNCTION_0("OnMinimize", On_Minimize) // On_Minimize()
		FUNCTION_4("OnStartFullScan", On_StartFullScan)
		FUNCTION_5("OnStartCustomScan", On_StartCustomScan)
		FUNCTION_1("OnPauseCustomScan", On_PauseCustomScan) // On_PasueResumeFullScan()	
		FUNCTION_1("OnResumeCustomScan", On_ResumeCustomScan) // On_PasueResumeFullScan()
		FUNCTION_2("OnClickScanCleanBtn", On_ClickScanCleanBtn)
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_0("onGetFolderPath", On_GetFolderPath)
		FUNCTION_0("OnCloseThreads", On_CloseThreads)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value On_GetThemeID();
	json::value On_GetFolderPath();
	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value OnGetAppPath();
	json::value On_Close();
	json::value On_Minimize();
	json::value On_CloseThreads();
	json::value On_PauseCustomScan(SCITER_VALUE svFunPauseResumeFunCB);
	json::value On_ResumeCustomScan(SCITER_VALUE svFunPauseResumeFunCB);
	json::value On_ClickScanCleanBtn(SCITER_VALUE svArrCleanEntries, SCITER_VALUE svQarantineFlag);
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value On_StartCustomScan(SCITER_VALUE svArrCustomScanSelectedEntries, SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB, SCITER_VALUE svFunNotificationMessageCB);
	//json::value On_StartCustomScan(SCITER_VALUE svArrCustomScanSelectedEntries);
	json::value On_StartFullScan(SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB, SCITER_VALUE svFunNotificationMessageCB);

public:

	int GetTaskBarWidth();
	int GetTaskBarHeight();

	void EnumFolder(LPCTSTR);
	void StartScanning();
	void EnumFolderForScanning(LPCTSTR pstr);
	void CallUISetStatusfunction(LPTSTR lpszPath);
	void CallUISetStatusfunctionSEH(LPTSTR lpszPath);
	void CallUISetFileCountfunction(CString csData, CString csCurrentFileCount);
	void ScanForSingleFile(CString);
	void CallUISetPauseStatusfunction(CString csData);
	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
	void OnTimerScan();
	void LoadRecoversDBFile();
	bool SaveInRecoverDB();
	void QuaratineEntries();
	bool GetSortedDriveList();
	bool GetDriveSize(CString  csDriveName);
	void CallUISetScanFinishedStatus(CString csData);
	void CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID);

	bool GetScanningPaths(CStringArray &csaReturn);
	bool GetAllDrivesList(CStringArray &csaReturn);
	bool CheckFileOrFolderOnRootPath(CString);
	bool Check4DBFiles();
	bool Check4ValidDBFiles(CString);
	bool PauseScan();
	bool ResumeScan();
	bool CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize);
	bool CheckEntryPresent(LPCTSTR szFileHash, LPTSTR szBackupPath);
	bool GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt);
	bool OnClickCleanButton(SCITER_VALUE svArrayCleanEntries);
	bool InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR m_csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus);
	bool QuarantineEntry(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath);
	bool BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath, LPTSTR lpszBackupFilePath);
	bool IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize);
	bool CheckIFAlreadyBackupTaken(LPCTSTR szFileHash, LPTSTR szBackupPath);
	bool GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash);
	bool WriteRegistryEntry(HKEY hRootKey, LPTSTR szKey, LPTSTR szValueName, DWORD dwData);

	DWORD HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThreatName1, DWORD dwISpyID, CString &csBackupFilePath, DWORD &dwAction);
	DWORD Encrypt_File(TCHAR *m_szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus);
	DWORD CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath);
	DWORD LoadSignatureDatabase(DWORD &dwSigCount);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	DWORD UnLoadSignatureDatabase();
	DWORD GetSelectedLanguage();
	void FunCleanup();

	CString GetQuarantineFolderPath();
	CString BrowseForFolder();
	CString GetString(CString csStringID);
	CString GetModuleFileStringPath();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	SCITER_VALUE	m_svArrCustomScanSelectedEntries;
	SCITER_VALUE	m_svAddVirusFoundEntryCB;
	SCITER_VALUE	m_svSetScanFinishedStatusCB;
	SCITER_VALUE	m_svFunNotificationMessageCB;
	SCITER_VALUE	m_svSetPauseStatusCB;
	SCITER_VALUE	m_svArrayCleanEntries;
	SCITER_VALUE	m_svVirusCount;

	HANDLE					m_hWardWizAVThread;
	HANDLE					m_hThread_ScanCount;
	HANDLE					m_hQuarantineThread;

public:
	bool					m_ScanCount;
	bool					m_bIsPathExist;
	bool					m_bFullScan;
	bool					m_bCustomscan;
	bool					m_bManualStop;

	int						m_iTotalFileCount;
	int						m_iThreatsFoundCount;

	unsigned char			*m_pbyEncDecKey;

	CStringArray			m_csaAllScanPaths;
	CString					m_csPreviousPath;
	CString					m_csCurrentFilePath;
	CString					m_csISpyID;

	DWORD					m_dwTotalFileCount;
	DWORD					m_FileScanned;
	DWORD					m_iMemScanTotalFileCount;
	DWORD					m_dwVirusFoundCount;
	DWORD					m_dwVirusCleanedCount;
	DWORD					m_dwAutoQuarOption;
	DWORD					m_wISpywareID;

	SCANTYPE				m_eScanType;
	SCANTYPE				m_eCurrentSelectedScanType;

	CTime					m_tsScanStartTime;
	CTime					m_tsScanEndTime;
	CTimeSpan				m_tsScanPauseResumeElapsedTime;

	CISpyScanner			m_objISpyScanner;
	CISpyCriticalSection	m_csQuarentineEntries;
	CISpyCriticalSection	m_csScanFile;
	CWardwizLangManager		m_objwardwizLangManager;
	CISpyDBManipulation		m_objISpyDBManipulation;
	CITinRegWrapper			m_objReg;

	ULARGE_INTEGER			m_uliFreeBytesAvailable;     // bytes disponiveis no disco associado a thread de chamada
	ULARGE_INTEGER			m_uliTotalNumberOfBytes;     // bytes no disco
	ULARGE_INTEGER			m_uliTotalNumberOfFreeBytes; // bytes livres no disco
	
	CString					m_csSortedDrive;						//sorted drive path
	DWORD64					m_dw64MaxSize;
	
	HELEMENT				self;
	SCANLEVEL				m_eScanLevel;

	bool ScanFinished();
	virtual bool on_timer(HELEMENT he);
	virtual bool on_timer(HELEMENT he, UINT_PTR extTimerId);
	bool on_timerSEH(HELEMENT he);

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
