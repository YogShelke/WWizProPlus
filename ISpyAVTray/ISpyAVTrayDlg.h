#pragma once
#include <afxcontrolbars.h>
#include "WardWizWidgetDlg.h"
#include "EmailNotifyTrayDlg.h"
#include "SystemTray.h"
#include "ISpyCommServer.h"
#include "WardWizTrayReleaseNotes.h"
#include "TrayPopUpMultiDlg.h"
#include "WardWizDatabaseInterface.h"
#include "ConCurrentQueue.h"
#include "Connections.h"
#include "Connections.h"
#include <afxcontrolbars.h>
#include "WardWizRestartClient.h"
#include "WrdWizSystemInfo.h"
#include "ExecuteProcess.h"

typedef bool (*RESETUSBVARIABLE)(void);
typedef INT_PTR(*DetectUSBFunc)(void);
//typedef INT_PTR(*FPISREMOVABLEDEVICE)(LPTSTR);

#define INSTALLATION_DEFAULT_THREAD_COUNT	0x32

class CISpyAVTrayDlg : public CDialog                   // Sciter host window primitives
{
public:
	CISpyAVTrayDlg(CWnd* pParent = NULL);	// standard constructor
	~CISpyAVTrayDlg(void);
	enum { IDD = IDD_ISPYAVTRAY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
public:
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	bool StartUSBDetectWatch();
	bool ResetUSBVariable();
	bool DisplayUpdateMessage(DWORD dwTypeOfUpdateMsg, DWORD dwMessageOption);
	void ShowEmailScanTrayPopup(LPCTSTR szThreatName,LPCTSTR szAttachmentName,LPCTSTR szSenderAddr,DWORD dwAction);
	static void OnDataReceiveCallBack(LPVOID lpParam);
	static void OnDataReceiveCallBackParCtrl(LPVOID lpParam);
	bool DisplayUpdateNowPopup(bool bRequestToClose = false);
	//bool IsRemovableDevice(LPTSTR lpDrive);
	bool AddIntoQueue(LPTSTR szThreatName, LPTSTR szFilePath, DWORD dwActionTaken);
	bool AddIntoQueueEmailScanInfo(LPTSTR szThreatName, LPTSTR szAttachmentName, LPTSTR szSenderAddr, DWORD dwActionTaken);
	bool ReInitializeQurentineDlg();
	bool ReInitializeQurentineDlg4EmailScan();
	bool LaunchAppBlockDlg(LPTSTR lpFirstParam);
	bool ShowUSBAutoScanPopUp();
public:
	CSystemTray				m_TrayIcon;
	HANDLE					m_hThread;
	static CISpyAVTrayDlg	*m_pThis;
	RESETUSBVARIABLE 		m_ResetUSBVariable;
	//FPISREMOVABLEDEVICE		m_fpISRemovableDevice;
	DetectUSBFunc	 		m_StartDialog;
	HINSTANCE 				m_hLibrary;
	DWORD					m_dwTimeReminder;
	HANDLE					m_hThreadRemider;
	TCHAR					m_szAllUserPath[512];
	CString					m_csCurrentUser;

	//Nitin K 09 July 2014
	HANDLE					m_ThreadMultiPopUp;
	int						m_iIndex;
	LPCTSTR					m_pszThreatName;
	LPCTSTR					m_pszAttachmentName;
	LPCTSTR					m_pszSenderAddr;
	DWORD					m_dwAction;
	DWORD 					m_dwNoofDays;
	BOOL					m_bFlag;
	CString					m_Key;
	CString					m_csInstalledEdition;
	CString				    m_csRegProductVer;
	CString					m_csRegDataBaseVer;
	CString					m_csUpdateDate;
	CString					m_csThreatDefCount;
	CString					m_csAppFolderName;
	bool					m_bAllowDemoEdition;
	HANDLE					m_hThreadShowPopUp;
	HANDLE					m_hThreadDetectThreat;
	HANDLE					m_hThreadDetectEmailScanThreat;
	HANDLE						m_hThreadShowRenewOption;
	CWardWizWidgetDlg			*m_pWardWizWidgetDlg;
	CWardWizTrayReleaseNotes	*m_objReleaseNotesDlg;
	HANDLE						m_hWidgetsUIThread;
	HANDLE						m_hParentalCtrlThread;
	std::thread				m_hProcessingThreads[INSTALLATION_DEFAULT_THREAD_COUNT];
	CConCurrentQueue<std::wstring> m_cInstallationQueue;
	bool					m_bIsIntProcessON;
	bool					m_IsRestartLaterActive;

	bool CompareUserName(LPTSTR csValue);
	bool WidgetActiveScanState(DWORD dwFlagState);
	void ResetVariables();
	void CheckRegistryForUpdate();/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */
	DWORD ReadRegistryEntry(CString strKey);
	bool ShowReleaseNotes();
	bool DisplayProductExpiryMsg();
	bool SetNonGenunineRegistry(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait=false);
	void ClearEntries();
	void ClearEntries4EmailScan();
	void LaunchDialog();
	void LaunchEmailTotifyTrayDialog();
	void ReadProductVersion4mRegistry();
	void GetRegisteredUserInfo();
	bool GetInformationFromINI();
	bool ShowSchedScanMissed();
	bool ShowSchedTempFileScanFinished();
	bool HandleActiveScanSettings(DWORD dwAction);
	bool ReLoadWidgetsUISettings(DWORD dwAction);
	void LaunchWidgetsUI();
	bool LaunchPasswordWnd();
	void ShowWidgetWindow();
	bool ShowPasswordWindow();
	bool StartInstallationProcessQueue();
	bool StopInstallationProcessQueue();
	bool EPSInstallClientSetup(LPISPY_PIPE_DATA lpData);
	DWORD InstallClientSetup(LPTSTR lpszTaskID, LPTSTR lpszIP, LPTSTR lpszUserName, LPTSTR lpszPassword);
	CString GetMachineNameByIP(CString csIPAddress);
	bool LaunchPasswordWndForPC(LPTSTR lpFirstParam);
	bool ShowUSBAutoScanPopUp(DWORD dwAction, LPTSTR lpFirstParam);
	bool ShowBrowserSecurityPopUp(LPTSTR lpFirstParam, LPTSTR lpSecondParam);
	bool LaunchPasswordWndForPCINet();
	CString GetCurrentUsrName(DWORD &dwSessionID);
	void CheckParCtrlPermission();
	void ReloadUserNameList();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSessionChange(UINT nSessionState, UINT nId);
	bool SendData2Service(DWORD dwMsg, bool bWait = false);
	bool SendData2Service(DWORD dwMsg, CString csParam, DWORD dwVal, bool bWait = false);
	bool ClosePasswordWndForPC();
	afx_msg void OnEndSession(BOOL bEnding);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);//Set the Registry Key Value for Menu Items using Service
	INT64 InsertDataToTable(const char* szQuery);
	CString GetSQLiteDBFilePath();
	void InsertParCtrlReport(DWORD);
	CString	GetCurrentUserName();
	void InsertUserNametoINI();
	int ShowBlockWebPopUp(LPTSTR lpszWebUrl, LPTSTR lpszWebCategory);
	bool ShowPortScanBlockPopUp();
	CString GetTSUserName(DWORD);
	bool CheckProductUptoDate();
	bool CheckProductInfo(CString &);
	bool GetProductID();
	void RefreshSysTrayIcon();
	CISpyCommunicatorServer  m_objParentalCntrlServer;
public:
	CWardWizSQLiteDatabase	m_objSqlDb;
	CISpyCriticalSection  objcriticalSection;
	CISpyCommunicator m_objCom;
};
