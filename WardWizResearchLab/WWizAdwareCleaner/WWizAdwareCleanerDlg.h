
// WWizAdwareCleanerDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "WWizAdwareScanner.h"
#include "iTINRegWrapper.h"
#include "iSpySrvMgmt.h"
#include "WinHttpManager.h"
#include "WWizAdwareRecoverDlg.h"

#define WM_TRAYMESSAGE WM_USER
#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30

class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CWWizAdwareCleanerDlg dialog
class CWWizAdwareCleanerDlg : public CDialog,
	public CSciterBase,
	public sciter::host<CWWizAdwareCleanerDlg> // Sciter host window primitives
{
	// Construction
public:
	CWWizAdwareCleanerDlg(CWnd* pParent = NULL);	// standard constructor

	// Dialog Data
	enum { IDD = IDD_WWIZADWARECLEANER_DIALOG };

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
public:
	afx_msg void OnBnClickedButtonScan();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonClean();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonResume();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnBnClickedCheckSelectall();
	bool UnInstallService(CString csSvcName);
	void StartScanning();
	void InitializeScan();
	void LoadDB();
	void InsertData(ENUMTYPE eType, CString csThreatName, CString csPath);
	bool Check4FileFolders(CString csLocation, CString csData, CString &cs2Display);
	bool Check4Registry(CString csLocation, CString csData, CString &cs2Display);
	void DisplayResult();
	void ScanFinished();
	DWORD GetTotalNumberOfIterations();
	bool GetUsersList();
	void EmptyDirectory(TCHAR* folderPath, bool &bRestartRequired, bool bTakeBackup = true);
	bool RemoveFile(CString csPath, bool &bRestartRequired);
	bool RemoveRegistry(CString csLocation);
	HINSTANCE hinst;
public:
	HWINDOW						get_hwnd();
	HINSTANCE					get_resource_instance();
	WWizAdwareRecoverDlg		m_objWWizAdwareRecover;

public:
	CStatic						m_stStatus;
	CProgressCtrl				m_prgState;
	CStatic						m_stThreatsFound;
	CButton						m_btnScan;
	CButton						m_btnClean;
	CButton						m_btnCancel;
	CListCtrl					m_lstFoundEntries;
	CButton						m_btnPauseResume;
	HANDLE						m_hThreadScan;
	std::vector<std::wstring>	m_vecList;
	CWWizAdwareScanner			m_objAdwScan;
	bool						m_bScanning;
	DWORD						m_dwScannedCount;
	DWORD						m_dwDetectedCount;
	DWORD						m_dwRemovedCount;
	DWORD						m_dwLocationCount;
	DWORD						m_dwDataCount;
	DWORD						m_dwTotalIterations;
	CStatic						m_stScannedCount;
	CStatic						m_stThreatsCleaned;
	CButton						m_chkSelectAll;
	bool						PauseScan();
	bool						ResumeScan();
	void						HideAllElements();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	VOID RestoreWndFromTray(HWND hWnd);
	VOID ShowNotifyIcon(HWND hWnd, DWORD dwAdd, CString csMessage);
	BOOL GetDoAnimateMinimize(VOID);
	VOID GetTrayWndRect(LPRECT lpTrayRect);
	void OnLoadDatabase();
	void OnCallAdwareEntryFunction(CString csThreatName, CString csPath, CString csStatus, CString csType);
	bool CheckInternetConnection();
	void GetRecordsSEH();
	INT64 InsertDataToTable(const char* szQuery);
	bool CheckforUninstall();
	CString GetWardWizPathFromRegistry();

public:
	sciter::dom::element			m_root_el;
	SCITER_VALUE					m_svFunGetLoadStatus;
	SCITER_VALUE					m_svAddAdwareEntryCB;
	SCITER_VALUE					m_svSetScanFinishedStatus;
	SCITER_VALUE					m_svSetScanStatus;
	SCITER_VALUE					m_svSetScanPercentage;
	SCITER_VALUE					m_svOnUpdateUIStatus;
	SCITER_VALUE					m_svArrayRecords;
	SCITER_VALUE					m_svFunCheckInternet;
	SCITER_VALUE					m_svFunCheckforUninstall;

	BEGIN_FUNCTION_MAP
		FUNCTION_3("LoadDB", OnLoadDB)
		FUNCTION_0("CloseUI", OnCloseUI)
		FUNCTION_3("StartAdwareScan", OnStartAdwareScan)
		FUNCTION_0("OnClickPause", OnBtnClickPause);
		FUNCTION_0("OnClickResume", OnBtnClickResume)
		FUNCTION_2("OnCleanEntries",OnBtnClickClean)
		FUNCTION_0("OnClickMinimize",OnBtnClickMinimise)
		FUNCTION_0("OnClickReboot", OnBtnClickReboot)
		FUNCTION_0("OnStopCleaning", OnBtnStopCleaning)
		FUNCTION_0("OnCallTimer",OnBtnCallTimer)
		FUNCTION_0("OnClickUninstall", OnBtnClickUninstall)
	END_FUNCTION_MAP

	json::value OnLoadDB(SCITER_VALUE svFunGetLoadStatus, SCITER_VALUE svFunCheckInternet, SCITER_VALUE svFunCheckforUninstall);
	json::value OnCloseUI();
	json::value OnStartAdwareScan(SCITER_VALUE svAddAdwareEntryCB, SCITER_VALUE svSetScanFinishedStatus, SCITER_VALUE svSetScanPercentage);
	json::value OnBtnClickPause();
	json::value OnBtnClickResume();
	json::value OnBtnClickClean(SCITER_VALUE svArrayRecords, SCITER_VALUE svOnUpdateUIStatus);
	json::value OnBtnClickMinimise();
	json::value OnBtnClickReboot();
	json::value OnBtnStopCleaning();
	json::value OnBtnCallTimer();
	json::value OnBtnClickUninstall();
};
