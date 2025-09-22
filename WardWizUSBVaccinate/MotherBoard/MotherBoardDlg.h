/****************************************************
*  Program Name     : MotherBoardDlg.h
*  Author Name      : Yogeshwar Rasal
*  Date Of Creation : 31st May 2016
*  Version No       : 2.0.0.17
****************************************************/
// MotherBoardDlg.h : header file for Dilog
//

#pragma once
#include "afxwin.h"
#include "sciter-x-threads.h"
#include "WardWizDatabaseInterface.h"
#include "PipeConstants.h"
#include "ISpyCommunicator.h"

class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CMotherBoardDlg dialog
class CMotherBoardDlg : public CJpegDialog
	, public CSciterBase,
	public sciter::host<CMotherBoardDlg> // Sciter host window primitives
{
// Construction
public:
	CMotherBoardDlg(CWnd* pParent = NULL);	// standard constructor
	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();

// Dialog Data
	enum { IDD = IDD_MOTHERBOARD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
public:
	//void AddLogEntry(const TCHAR *szMessage, const TCHAR *szMessage1 = NULL, const TCHAR *szMessage2 = NULL, bool isDateTime = true , DWORD dwLogLevel = 0);
	//Utility functions for Kolhapur Event
	DWORD USBVaccine();
	DWORD VaccinateUSBDrive( LPCTSTR pszRemDrive, LPTSTR lpszRetMessage );
	DWORD RenameIfFileExists( LPCTSTR pszRemDrive );
	DWORD MakeAutorunDirectory( LPCTSTR pszDirPath, bool bHideAttribute = false );

	bool EnableTaskMgr( );
	bool EnableRegedit( );
	DWORD EnableRunWindow();

	DWORD GetCurrentLoggedUserSID( LPTSTR lpUserSID );

	DWORD UnhideFilesFolders( LPCTSTR pszDirPath, bool bDeleteLNKFile = false );
	DWORD SelectedFilesFolders(LPCTSTR pszDirPath);
	bool SetDefaultExplorerSettings();

	DWORD	m_dwUnhideFiles;
	DWORD	m_dwScannedFiles;
	INT64	m_iUSBSessionId;
	bool	m_bProcessStarted;

	HANDLE	m_hThreadStartScanForUnhide;

	
	void InitializeVariablesToZero();
	void HideAllElements();
	bool CheckInExcludeList(LPCTSTR lpFile);
// Implementation
protected:
	HICON m_hIcon;

	CStringList		m_cslExcludeFiles;
	
    // Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
public:
	//afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonBrowsefolder();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedCheckUnhidefilesandfolders();
	bool SetGoogleAsDefaultBrowserInernetExplorer();
public:
	CButton m_chkEnableTaskManager;
	CButton m_chkRegistryEditor;
	CButton m_chkEnableRunWindow;
	CButton m_chkFolderOption;
	CButton m_chkUnhideFolderOption;
	CButton m_chkUSBVaccine;
	CxSkinButton m_btnBrowse;
	CButton m_chkDefaultIESettings;
	CxSkinButton m_OkButton;
	CxSkinButton m_CancelButton;

	CEdit	m_editPath;
	CString	m_csEditPath;
	CString csTypeSelected;
	CColorStatic m_CurrentFile;
	
	int		m_iUSBVaccine;
	int		m_iEnableTaskManager;
	int		m_iRegistryEditor;
	int		m_ikEnableRunWindow;
	int		m_iFolderOption;
	int		m_iUnhideFolderOption;
	int		m_iDefaultIESettings;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCheckDefaultIexploresettings();
	afx_msg void OnBnClickedCheckUsbVaccine();
	afx_msg void OnBnClickedButtonClose();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CColorStatic			m_stUSBVaccineHeader;
	CColorStatic			m_stUSBVaccineTitle;
	CColorStatic			m_stUnhideFileFolder;
	CColorStatic			m_stSelectFileFolderHeader;
	
	CxSkinButton			m_btnClose;
	CRect					m_rect;
	CColorStatic			m_stAllRightsText;
	CFont				 	m_BoldText;
	CxSkinButton			m_btnMinimize;
public :
	SCITER_VALUE			m_svVaccineStatusFnCB;
	SCITER_VALUE			m_FileStatusFnCB;
	sciter::dom::element	m_root_el;
	CString					m_csFileName;
	bool					m_bClickedNo;
	afx_msg void OnBnClickedButtonMinimize();
	afx_msg void OnClose();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	/*BEGIN_FUNCTION_MAP
		//FUNCTION_1("On_BnClickedAutoRunScan", On_OnBnClickedAutoRunScan) // On_OnBnClickedAutoRunScan()
		FUNCTION_1("On_BnClickedAutoRunScan", On_OnBnClickedTempCleanFind) // On_OnBnClickedTempCleanScan()

		FUNCTION_0("On_BnClickedAutoRunRepair", On_OnBnClickedTempCleanRemove) // On_OnBnClickedAutoRunRepair()
		//FUNCTION_0("On_BnClickedAutoRunPause_Resume", On_OnBnClickedAutoRunPause_Resume) // On_OnBnClickedAutoRunPause_Resume()
		//FUNCTION_0("On_BnClickedAutoRunRepair", On_OnBnClickedAutoRunRepair) // On_OnBnClickedAutoRunRepair()

	END_FUNCTION_MAP

	json::value  On_OnBnClickedTempCleanFind(SCITER_VALUE SetScanStatusFncb);
	json::value  On_OnBnClickedTempCleanRemove();*/


	BEGIN_FUNCTION_MAP
		FUNCTION_3("OnStartVaccineProcess", On_StartVaccineProcess)
		FUNCTION_0("OnMinimize",On_Minimize)
		FUNCTION_0("OnClickClose",On_Close)
		FUNCTION_0("SelectFileFolder", On_SelectFileFolder)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnClickedNo", OnBnClickedNo)
		FUNCTION_0("OnPauseThread", OnPauseThread)
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)
	END_FUNCTION_MAP

	json::value On_StartVaccineProcess(SCITER_VALUE svBoolUSBVaccin, SCITER_VALUE svFilePath, SCITER_VALUE svNotificationMsgCB);
	json::value On_Close();
	json::value On_Minimize();
	json::value On_SelectFileFolder();
	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value OnBnClickedNo();
	json::value OnPauseThread();
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value OnGetAppPath();
	json::value On_GetThemeID();
	json::value FunCheckInternetAccessBlock();

public:
	INT64 InsertDataToTable(const char* szQuery);
	void CallNotificationMessage(SCITER_STRING strMessageString);
	void EnterVaccinationDetails(LPCTSTR pszRemDrive, INT64 iSessionId);
	void EnterHideUnhideDetails(LPCTSTR pszDirPath, INT64 iSessionId);
public:	
	CWardWizSQLiteDatabase m_objSqlDb;
};

