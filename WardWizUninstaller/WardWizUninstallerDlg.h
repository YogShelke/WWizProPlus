/**********************************************************************************************************
Program Name          : WardWizUninstallerDlg.h
Description           :
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 6th Feb 2015
Version No            : 1.9.0.0
Special Logic Used    :
Modification Log      :
***********************************************************************************************************/
#pragma once

#include "CScannerLoad.h"
#include "CSecure64.h"
#include "DriverConstants.h"
#include "WardWizOSversion.h"
#include "sciter-x-threads.h"
#include "Enumprocess.h"
#include "ISpyCommunicator.h"
#include "ScannerContants.h"
#include "sqlite3.h"
#include "PipeConstants.h"

enum eENABLEDISABLE
{
	DISABLE,
	ENABLE
};

enum eBUTTONIDS
{
	CANCEL,
	CLOSE,
	NEXT
};

class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CWardWizUninstallerDlg dialog
class CWardWizUninstallerDlg : public CDialog
	, public CSciterBase,
	public sciter::host<CWardWizUninstallerDlg> // Sciter host window primitives

{
private:

	bool LoadRequiredLibrary();
	WRDWIZEMAILNFDLLUNLINK		m_lpEmailNFUnlink;
	HMODULE						m_hModuleEmailDLL;
	// Construction
public:
	CWardWizUninstallerDlg(CWnd* pParent = NULL);	// standard constructor
	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();

// Dialog Data
	enum { IDD = IDD_WARDWIZUNINSTALLER_DIALOG };

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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	sciter::dom::element		m_RootElt;
	bool						m_bUserSettings;
	bool						m_bQuarantineEntries;
	HANDLE						m_hThreadDeleteFile;
	HANDLE						m_hThreadUninstall;
	DWORD						m_dwTotalDeletedFileCount;
	DWORD						m_dwTotalFileCount;
	CString						m_csWardWizPath;
	CString						m_csAppPath;
	CString						m_csProductName;
	bool						m_bQuarantine;
	std::vector<CString>		m_vFilePathLists;
	std::vector<CString>		m_vFolderPathLists;
	bool						m_bMoveFileExFlag;
	bool						m_bPassDbMngr;
	int							m_iTotalFileCount;
	int							m_iTotalDeletedFileCount;
	bool						m_bRestartReq;
	bool						m_bUninstallInProgress;
	CStringArray				m_csArrRegistryEntriesDel;
	SCITER_VALUE				m_svFunSetPercentageCB;
	SCITER_VALUE				m_svFuntionFinished;
	SCITER_VALUE				m_svEnableDisableButtons;
	SCITER_VALUE				m_svShowWrdWizRunningCB;
	bool						bShowLightBox;
	CString						m_AppPath;
	sciter::dom::element		m_root_el;
	bool						m_bIsUserDefinedSettings;
	bool						m_bIsKeepQuarFiles;
	bool						m_bUninstallCmd = false;

public:
	void InitializeVariables();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//json::value CWardWizUninstallerDlg::On_ClickStartUninstall(SCITER_VALUE svArrSelectedOptions, SCITER_VALUE svFunSetPercentageCB, SCITER_VALUE svFuntionFinished, SCITER_VALUE svEnableDisableButtons);
	
	void EnumFolderToCountFiles(LPCTSTR pstr);
	void UnregisterComDll(CString csAppPath);
	void StartUninstallProgram();
	void StopProtectionDrivers();
	void UninstallforEPS();
	bool CloseAllAplication(bool bIsReInstall);
	void CloseApp();
	bool DeleteFiles(std::vector<CString>	&vFilePathLists);
	bool DeleteFolder(std::vector<CString>	&vFolderPathLists);
	bool DeleteRegistryKeys();
	void DeleteAllAppShortcut();
	DWORD RemoveFilesUsingSHFileOperation(TCHAR *pFolder);
	void DeleteFinished();
	void UninstallationMessage();
	void RemoveDriverRegistryKeyAndPauseDrivers();
	bool GetCloseAction4OutlookIfRunning(bool bIsReInstall);
	BOOL RegDelnode(HKEY hKeyRoot, CString csSubKey);
	BOOL RegDelnodeRecurse(HKEY hKeyRoot, CString csSubKey);
	void ShowFinishedPage();
	void EnableDisableControls(bool bEnable);
	void RestoreWindow();
	void CallNotificationMessage(SCITER_STRING strMessageString, int msgType = 2);
	void CreateUninstallerEntryinRegistry();
	bool GetNFDllUnlink();
	void UninstallFWDrivers();
	void GetFWDriverPath();
	void DeleteFWDrivers(CString csWrdWizFLTDriverPath);
	void RestoreReqRegistry();
	void StartUninstllSqlServerAndDisableIISServices();
	BOOL LaunchProcessThrCommandLine(LPTSTR pszAppPath, LPTSTR pszCmdLine);
	bool DeleteSqlServerWWIZInstanceDirectory(CString csInputFileName);
	bool GetBackupSqlServerWWIZInstanceDB(CString csSourceSqlDir, LPTSTR csDestSqlServerDir, LPTSTR csSqlWwizInstaDbLog);
	bool CreateDirectoryWwizInstanceBackup(LPTSTR lpszPath);
	BOOL IsDots(const TCHAR* str);
	void IsWow64();
	bool m_bIsWow64;
	bool bBackSpace;
public:
	DECLARE_MESSAGE_MAP()
	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnClickRestartNow", On_ClickRestartNow)
		FUNCTION_0("OnButtonClickMinimize", On_ButtonClickMinimize)
		//FUNCTION_4("OnClickStartUninstall", On_ClickStartUninstall)
		FUNCTION_5("OnClickStartUninstall", On_ClickStartUninstall)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("OnCloseCalled", OnCloseCalled)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("GetDBPath", GetDBPath)
		FUNCTION_0("GetDBPathforEPS", GetDBPathforEPS);
		FUNCTION_1("GetDecryptPasssword", GetDecryptPasssword)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)
	END_FUNCTION_MAP

	json::value On_ClickStartUninstall(SCITER_VALUE svArrSelectedOptions, SCITER_VALUE svFunSetPercentageCB, SCITER_VALUE svFuntionFinished, SCITER_VALUE svEnableDisableButtons, SCITER_VALUE svShowWrdWizRunningCB);
	json::value On_ButtonClickMinimize();
	json::value On_ClickRestartNow(); 
	json::value On_GetProductID();
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value OnGetAppPath();
	json::value OnCloseCalled();
	json::value On_GetLanguageID();
	json::value GetDBPath();
	json::value GetDBPathforEPS();
	json::value GetDecryptPasssword(SCITER_VALUE svEncryptPasssword);
	json::value On_GetThemeID();
	json::value FunCheckInternetAccessBlock();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
};
