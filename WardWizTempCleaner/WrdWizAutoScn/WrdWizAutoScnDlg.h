/***************************************************************************************************************************************
Program Name          : WrdWizAutoScnDlg.h
Description           : This class contains the functionality for Temporary File Cleaner: Save disk space by-removing temporary file(s).
It has 2 options	  : a) Find temporary files  b) Remove temporary file(s).
Author Name			  : Amol Jaware
Date Of Creation      : 02 June 2016
Version No            : 2.0.0.17
***************************************************************************************************************************************/
// WrdWizAutoScnDlg.h : header file
#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <vector>
#include <Shlwapi.h>
#include <Imagehlp.h>
#include "JpegDialog.h"
#include "sciter-x-threads.h"
#include "WardWizDatabaseInterface.h"
#include "iTINRegWrapper.h"

#pragma pack(1)
typedef struct _FILE_CACHEINFO
{
	DWORD	dwFileSize;
	DWORD	dwFileChecksum;
	//BYTE	bFileHash[0x10];
	//DWORD	dwCRC32;
	//01 5C C3 90 BF DD 3D 91 40 BE 42 63 88 E9 65 24
}FILECACHEINFO;
#pragma pack()

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
	sciter::dom::element	m_root_el;
	TCHAR	m_ApplRunningPath[512];
	TCHAR	m_szSpecificPath[512];
	TCHAR	m_szLogPath[512];
	TCHAR	m_szLogComparePath[512];
	TCHAR	m_SysDir[256];
	TCHAR	m_WinDir[256];
	INT64	m_iScanId;
	INT64	m_iScanSessionId;
	TCHAR	m_ProgPath[256];
	TCHAR	m_Progx86Path[256];

	bool	m_bX64;

	HMODULE	m_hHashDLL;
	HMODULE	m_hModuleISpyScanDLL;
	HANDLE	m_hThreadStartScan;
	HANDLE	m_hThreadRepair;
	bool	m_GenerateStarted;
	bool	m_bAllDrives;

	DWORD	m_dwScannedFiles;
	DWORD	m_dwDetectedFiles;
	DWORD	m_dwRepairedFiles;
	DWORD	m_dwFailed;
	DWORD	m_ListIndex;
	DWORD	m_dwRebootFileDeletion;
	DWORD   m_dwTotalFilesCleaned;
	DWORD   m_dwActioned;
	DWORD	m_dwTotDetectedFiles;

	DWORD	m_dwScanStartTickCount;

	SYSTEMTIME		m_StartScanTime;


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

	DWORD SearchInFileCache(DWORD dwFileSize, DWORD dwFileChecksum);
	DWORD AddFileInfoInFileCache(DWORD dwFileSize, DWORD dwFileChecksum);

	bool SaveGoodCacheToFile(LPCTSTR lpFileName);
	bool ReadGoodCacheFromFile(LPCTSTR lpFileName);

	void DeleteFileRename(LPCTSTR lpFileName);
	//void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = NULL, const TCHAR *sEntry2 = NULL,
		//bool isDateTime = true, bool bNextLine = false);

	void AddLogEntry_Compare(const TCHAR *sFormatString, const TCHAR *sEntry1 = NULL, const TCHAR *sEntry2 = NULL,
		bool isDateTime = true);

	DWORD KillSpecifiedRunningProcesses();
	DWORD SuspendProcessThreadsForcefully(DWORD dwPID, bool &bRunningFound);
	void GenerateProcessKillList();

	void StartScanning();
	void CheckFileForViruses(LPCTSTR lpFileName);
	bool CheckForAutorunInfFile(LPBYTE	lpbBuffer, DWORD dwBufSize, bool &bFound);
	bool CheckForThumbsDBFile(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);
	bool CheckForAutorunLNKFile(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);
	bool CheckforDesktopVBS(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);

	bool CheckForCantix(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);

	bool UnhideDirectory(LPCTSTR lpFileName);
	DWORD DeleteFileForcefully(LPCTSTR lpFileName, bool bDeleteReboot);
	void AddEntryinUITable(LPCTSTR lpVirusName, LPCTSTR lpFileName, CString lpAction = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"));

	void SetAllItemsInListViewasSelected(bool bSelectAll = true);

	DWORD CheckInExcludeFolder(LPCTSTR lpPath);


	void Init();
	void CalculateCRC32(const BYTE byte, DWORD &dwCrc32);
	DWORD FileCRC32Win32(HANDLE hFile, DWORD &dwCrc32);

	void FreeResource();

	bool CheckPathInExcludeFolder(LPCTSTR lpFolPath);

	void SetForegroundWindowAndBringToTop(HWND hWnd);

	bool m_bScanThreadFinished;

	//HWND	m_MainWindow;
	bool	m_bScanStarted;

	TCHAR	m_szProgressMsg[256];

	bool	m_bScanStart;
	bool	m_bRepairStart;

	//Added by Vilas on 19 / 08 / 2015
	//Issue reported by Harald during second scan
	void SetElapsedTimetoGUI();

// Implementation
protected:
	HICON m_hIcon;

	
	std::vector <FILECACHEINFO > m_vFileCacheInfo;

	LPDWORD m_pdwCrc32Table;

	CStringList	m_ExcludeFols;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonStartScan();
	void funcCleanTempFileFoundEntry(int, CString, int);

	CxSkinButton m_generateCache;
	CxSkinButton m_Close;
	afx_msg void OnBnClickedButtonClose();
	CColorStatic m_CurrentFile;
	CListCtrl m_FilesList;
	CButton m_SelectAll;
	CButton m_Browse;
	CEdit m_SpecificPath;
	CButton m_Radio_AllDrives;
	CButton m_Radio_SpecificPath;
	afx_msg void OnBnClickedButtonBrowse();
	CColorStatic m_StartTime;
	CColorStatic m_EndTime;
	afx_msg void OnBnClickedRadioWholeSystem();
	afx_msg void OnBnClickedRadioSpecificPath();
	afx_msg void OnBnClickedCheckSelectAll();
	CColorStatic m_DuplicateFilesCountText;
	
	CColorStatic m_stSelctAll;
	CxSkinButton m_btnMinimize;
	CxSkinButton m_btnCloseUI;

	
	afx_msg void OnBnClickedButtonMinimize();
	afx_msg void OnBnClickedButtonCloseui();
	CColorStatic			m_stTempFooter;
	HCURSOR					m_hButtonCursor;
	CFont					m_BoldText;
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	CColorStatic m_stTempCleanerHeader;
	afx_msg void OnClose();

	BEGIN_FUNCTION_MAP
		
		FUNCTION_2("OnBnClickedTempCleanFind", On_OnBnClickedTempCleanFind) // On_OnBnClickedTempCleanScan()
		FUNCTION_2("OnBnClickedTempFileClean", On_OnBnClickedTempFileClean) // On_OnBnClickedAutoRunRepair()
		FUNCTION_0("OnCloseButton", On_OnCloseButton); // on clsoe button and on close image cross (X)
		FUNCTION_0("PauseTempFileOperation",On_PauseTempFileOperation) // On_PauseTempFileOperation()
		FUNCTION_0("ResumeTempFileOperation",On_ResumeTempFileOperation) // On_ResumeTempFileOperation()
		FUNCTION_0("GetProductID", On_GetProductID) 
		FUNCTION_0("OnMinimize", On_Minimize) // On_Minimize()	
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)
	END_FUNCTION_MAP
	
public:
	json::value  On_OnBnClickedTempCleanFind(SCITER_VALUE svFunScanFinishedCB, SCITER_VALUE OnBnClickedButtonCloseCB);
	json::value  On_OnBnClickedTempFileClean(SCITER_VALUE svArrRecords, /*SCITER_VALUE svCleanTempFileFoundEntryCB,*/ SCITER_VALUE svRemovedStatusCB);
	json::value  On_OnCloseButton();
	json::value  On_PauseTempFileOperation();
	json::value  On_ResumeTempFileOperation();
	json::value On_GetProductID();
	json::value On_Minimize();
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
	json::value FunCheckInternetAccessBlock();

	void UpdateSessionRecord();

	INT64 InsertDataToTable(const char* szQuery);
	INT64 AddTempFileEntryToDB(INT64 iScanID, CString csFilePath, CString csVirusName, CString csAction);
	INT64 AddTempFileSessionEntryToDB(INT64 iScanID, int iDetectedFiles, int iRepairedFiles);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void	ShowTempFileCleaner();
	bool CWrdWizAutoScnDlg::SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait = false);
public:
	SCITER_VALUE				m_svScanStatusFncb;
	SCITER_VALUE				m_svFunAddVirusFoundEntryCB;
	SCITER_VALUE				m_svFunScanFinishedCB;
	SCITER_VALUE				m_svArrRecords;
	SCITER_VALUE				svCleanTempFileFoundEntryCB;
	SCITER_VALUE				m_svRemovedStatusCB;
	SCITER_VALUE				m_svOnBnClickedButtonCloseCB;
	bool						m_bIsTempFileFoundFinished;
	CWardWizSQLiteDatabase		m_objSqlDb;
};
