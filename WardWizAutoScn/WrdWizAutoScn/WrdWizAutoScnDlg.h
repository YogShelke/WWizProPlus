
// WrdWizAutoScnDlg.h : header file
//

#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "sciter-x-threads.h"
#include "TextProgressCtrl.h"
#include "ISpyCriticalSection.h"
#include "WardWizDatabaseInterface.h"
#include "iTINRegWrapper.h"
#include "PipeConstants.h"
#include "ISpyCommunicator.h"

/***************************************************************************************************
*  Class Name	  : CSciterBase
*  Description    : Sciterbase class for sciter DOM event handling
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 12 Apr 2016
****************************************************************************************************/
class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CWrdWizAutoScnDlg dialog
class CWrdWizAutoScnDlg : public CJpegDialog
	, public CSciterBase,
	public sciter::host<CWrdWizAutoScnDlg> // Sciter host window primitives
{
// Construction
public:
	CWrdWizAutoScnDlg(CWnd* pParent = NULL);	// standard constructor
	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();
	
	~CWrdWizAutoScnDlg();
// Dialog Data
	enum { IDD = IDD_WRDWIZAUTOSCN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


public:

	//CStringList m_strListProcesses;

	TCHAR	m_ApplRunningPath[512];
	TCHAR	m_szSpecificPath[512];
	TCHAR	m_szLogPath[512];
	TCHAR	m_szLogComparePath[512];
	TCHAR	m_SysDir[256];

	HMODULE	m_hHashDLL;
	HMODULE	m_hModuleISpyScanDLL;
	HANDLE	m_hThreadStartScan;
	HANDLE	m_hThreadRepair;
	bool	m_GenerateStarted;
	bool	m_bAllDrives;
	sciter::dom::element root_el;

	DWORD	m_dwScannedFiles;
	DWORD	m_dwDetectedFiles;
	DWORD	m_dwRepairedFiles;
	DWORD	m_dwFailed;
	DWORD	m_ListIndex;
	DWORD	m_dwRebootFileDeletion;
	INT64   m_iScanSessionId;
	DWORD	m_dwScanStartTickCount;

	SYSTEMTIME		m_StartScanTime;

	//Added on 02 / 09 /2015 for Progress bar
	DWORD	m_dwTotalFiles;
	HANDLE	m_hThreadFilesCount;
	CTextProgressCtrl m_prgScanProgress;

	//Added threat(s) count in GUI by Vilas on 03 / 09 / 2015
	CColorStatic m_stThreatCount;

	//Added on 20 / 10 /2015 for Correct status of Progress bar
	bool	m_bFileCountObtained;

	void SetElapsedTimetoGUI();

	void EnumFolderForFileCount(LPCTSTR lpFolPath);

	void DeleteFilesFromTEMP();
	void EnumFolderForDeletion(LPCTSTR lpFolPath);
	DWORD DeleteFileForcefully(LPCTSTR lpFolPath);

	void InitializeAllVariablesToZero();

	bool LoadMD5DLL();
	bool LoadRequiredModules();
	void FreeUsedResources();
	void HideAllElements();

	void EnumFolder(LPCTSTR lpFolPath);
	bool GenerateHashForFile(LPCTSTR lpFileName);
	DWORD GetHashForPEFile(LPCTSTR lpFileName);

	void EnumFolderForGoodChache(LPCTSTR lpFolPath);
	DWORD GenerateHashForFileForGoodChache(LPCTSTR lpFileName);

	bool SearchInGoodCache(DWORD dwFileSize, DWORD dwFileChecksum);

	bool SaveGoodCacheToFile(LPCTSTR lpFileName);
	bool ReadGoodCacheFromFile(LPCTSTR lpFileName);

	void DeleteFileRename(LPCTSTR lpFileName);
	/*void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = NULL, const TCHAR *sEntry2 = NULL,
		bool isDateTime = true);*/

	void AddLogEntry_Compare(const TCHAR *sFormatString, const TCHAR *sEntry1 = NULL, const TCHAR *sEntry2 = NULL,
		bool isDateTime = true);

	DWORD KillSpecifiedRunningProcesses();
	DWORD SuspendProcessThreadsForcefully(DWORD dwPID, bool &bRunningFound);
	void GenerateProcessKillList();

	void StartScanning();
	void CheckFileForViruses(LPCTSTR lpFileName);
	void CheckFileForVirusesSEH(LPCTSTR lpFileName);
	bool CheckForAutorunInfFile(LPBYTE	lpbBuffer, DWORD dwBufSize, bool &bFound);
	bool CheckForThumbsDBFile(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);

	//Modified for Gamerue 
	//by Vilas on 13 / 07 / 2015
	bool CheckForAutorunLNKFile(LPCTSTR lpFileName, LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound, LPTSTR lpszVirusName);
	bool CheckforDesktopVBS(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);

	bool CheckForCantix(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);

	bool UnhideDirectory(LPCTSTR lpFileName);
	DWORD DeleteFileForcefully(LPCTSTR lpFileName, bool bDeleteReboot);
	void AddEntryinListView(LPCTSTR lpVirusName, LPCTSTR lpFileName, CString lpAction = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"));

	bool m_bScanStarted;
	INT64 m_iAutoScanId;

	void SetAllItemsInListViewasSelected(bool bSelectAll = true);
	void AddUserAndSystemInfoToLog();
	TCHAR	szMessage[256];

	bool	m_bScanStart;
	bool	m_bRepairStart;
	bool	m_bRemovDriveRemoved;
	bool	m_bPause_Resume;

// Implementation
protected:
	HICON m_hIcon;

	TCHAR	m_szComputerName[128];

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonGenerateCache();
	CxSkinButton m_generateCache;
	CxSkinButton m_Close;
	afx_msg void OnBnClickedButtonClose();
	CColorStatic m_CurrentFile;
	CListCtrl m_FilesList;
	CButton m_SelectAll;
	CxSkinButton m_Browse;
	CEdit m_SpecificPath;
	CxSkinButton m_Radio_AllDrives;
	CxSkinButton m_Radio_SpecificPath;
	afx_msg void OnBnClickedButtonBrowse();
	CColorStatic m_StartTime;
	CColorStatic m_EndTime;
	afx_msg void OnBnClickedRadioWholeSystem();
	afx_msg void OnBnClickedRadioSpecificPath();
	afx_msg void OnBnClickedCheckSelectAll();
	afx_msg void OnEnChangeEditSpecificPath();
	CColorStatic m_stSelectScanPath;
	CxSkinButton m_btnAutorunminimize;
	CxSkinButton m_btnAutoRunClose;
	CColorStatic m_stSelectAll;
	HCURSOR		 m_hButtonCursor;
	CFont		 m_BoldText;
	HWND		 m_MainWindow;
	SCITER_VALUE m_svSetScanFinishedStatuscb;
	SCITER_VALUE m_svSetScanFileStatuscb;
	sciter::dom::element m_root_el;
	CString		 m_csEditPath;
	CISpyCriticalSection  m_csReports;

	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	CColorStatic m_stAutorunFooter;
	CColorStatic m_stAllDriveText;
	CColorStatic m_stSelectDrivetext;
	CColorStatic m_stAutoHeader;
	afx_msg void OnBnClickedButtonAutorunMinimize();
	afx_msg void OnBnClickedButtonAutorunClose();	
	void SetForegroundWindowAndBringToTop(HWND hWnd);
	void OnBnClickedButtonBrowsefolder();
	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnClose();

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);



public:

	BEGIN_FUNCTION_MAP
		//FUNCTION_4("On_BnClickedAutoRunScan", On_OnBnClickedAutoRunScan) // On_OnBnClickedAutoRunScan()
		FUNCTION_4("On_BnClickedAutoRunScan", On_OnBnClickedAutoRunScan) // On_OnBnClickedAutoRunScan()
		//FUNCTION_1("On_BnClickedAutoRunPause_Resume", On_OnBnClickedAutoRunPause_Resume) // On_OnBnClickedAutoRunPause_Resume
		FUNCTION_0("On_BnClickedAutoRunPause_Resume", On_OnBnClickedAutoRunPause_Resume) // On_OnBnClickedAutoRunPause_Resume
		FUNCTION_3("On_BnClickedAutoRunRepair", On_OnBnClickedAutoRunRepair) // On_OnBnClickedAutoRunRepair()
		FUNCTION_0("PauseAutorunScanOperation", On_Stop)
		FUNCTION_0("On_Close", On_Close)
		FUNCTION_0("On_Yes", OnBnClickedYesButton)
		FUNCTION_0("ResumeAutorunScanOperation", OnBnClickedNoButton)
		FUNCTION_0("GetProductID", On_GetProductID) // On_Minimize()
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("SelectFileFolder", On_SelectFileFolder)
		FUNCTION_0("OnMinimize", On_Minimize)
		FUNCTION_1("funcScanFinished", On_funcScanFinished);
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("CloseWhileNoDetectedEntry", CloseWhileNoDetectedEntry)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)
	END_FUNCTION_MAP

	//json::value  On_OnBnClickedAutoRunScan(SCITER_VALUE svbAutoRunScan, SCITER_VALUE svNewPath, SCITER_VALUE svSetScanFileStatusFunCB,SCITER_VALUE svAddAutoRunScanFoundEntriesCB);
	json::value  On_OnBnClickedAutoRunScan(SCITER_VALUE svbAutoRunScan, SCITER_VALUE svNewPath, SCITER_VALUE svAddAutoRunScanFoundEntriesCB, SCITER_VALUE svEmptyFolderNotificationCB);
	json::value On_OnBnClickedAutoRunScanSEH(SCITER_VALUE svbAutoRunScan, SCITER_VALUE svNewPath, SCITER_VALUE svAddAutoRunScanFoundEntriesCB, SCITER_VALUE svEmptyFolderNotificationCB);
	//json::value  On_OnBnClickedAutoRunPause_Resume(SCITER_VALUE svSetFileScanStatus);
	json::value  On_OnBnClickedAutoRunPause_Resume();
	json::value  On_OnBnClickedAutoRunRepair(SCITER_VALUE svrecords, SCITER_VALUE setUpdateVirusFoundentriesCB, SCITER_VALUE svUpdateRepairedFilesStatusCB);
	json::value On_Close();
	json::value On_Stop();
	json::value OnBnClickedYesButton();
	json::value OnBnClickedNoButton();
	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value On_SelectFileFolder();
	json::value On_Minimize();
	json::value On_funcScanFinished(SCITER_VALUE svFuncScanFinishedCB);
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value OnGetAppPath();
	json::value CloseWhileNoDetectedEntry();
	json::value On_GetThemeID();
	json::value FunCheckInternetAccessBlock();

	void UpdateSessionRecord();
	INT64 InsertDataToTable(const char* szQuery);
	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
	void CloseFileHandle();

	public:
		SCITER_VALUE			m_svbAutoRunScan;
		SCITER_VALUE			m_svNewPath;
		SCITER_VALUE			m_svrecords;
		SCITER_VALUE			m_setUpdateVirusFoundentriesCB;
		//SCITER_VALUE			m_svSetScanFileStatusFunCB;
		SCITER_VALUE			m_svAddAutoRunScanFoundEntriesCB;
		SCITER_VALUE			m_svSetScanFinishedStatus;
		SCITER_VALUE			m_svUpdateRepairedFilesStatusCB;
		SCITER_VALUE			m_svEmptyFolderNotificationCB;
		SCITER_VALUE			m_svFuncScanFinishedCB;
		bool					m_bIsRequestFromTaskBar;
		bool					m_bIsTempFileFoundFinished;
		bool					m_bISRepairFile;
		int						m_iRepairFileCount;
		CWardWizSQLiteDatabase	m_objSqlDb;
		HANDLE					m_hFileHandle; //= INVALID_HANDLE_VALUE;
		bool					m_bClose;
};
