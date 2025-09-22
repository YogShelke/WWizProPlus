/****************************************************
*  Program Name: CWardWizUIDlg.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28th March 2016
*  Version No: 2.0.0.1
****************************************************/
#pragma once

#include "WardWizScan.h"
#include "ISpyCommServer.h"
#include "sciter-x-threads.h"
#include "WardWizRegistryOpt.h"
#include "WardWizDataCrypt.h"
#include "WardWizRecover.h"
#include "WardWizSettings.h"
#include "WardWizAntirootkit.h"
#include "WardWizUpdates.h"
#include "WardWizReports.h"
#include "iTINRegWrapper.h"
#include "WardWizFolderLocker.h"
#include "WardWizEmailScan.h"
#include "WardWizFirewall.h"
#include "WardWizParentalControl.h"
#include "WardWizFullScan.h"
#include "WardWizQuickScan.h"
#include "WrdWizBrowserSecurity.h"

class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CWardWizUIDlg dialog
class CWardWizUIDlg : public CDialogEx,
	public CSciterBase,
	public sciter::host<CWardWizUIDlg> // Sciter host window primitives
{
// Construction
public:
	CWardWizUIDlg(CWnd* pParent = NULL);	// standard constructor
	static void OnDataReceiveCallBack(LPVOID lpParam);
// Dialog Data
	enum { IDD = IDD_WARDWIZUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
public:
	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();

	CWardWizScan			m_objWardWizScan;
	CWardWizRegistryOpt		m_objWardWizRegistryOpt;
	CWardWizDataCrypt		m_objWardWizDataCrypt;
	CWardWizRecover			m_objWardWizRecover;
	CWardWizSettings		m_objWardWizSettings;
	CWardWizAntirootkit		m_objWardWizAntirootkit;
	CWardWizUpdates			m_objWardWizUpdates;
	CWardWizReports			m_objWardWizReports;
	CWardWizFolderLocker	m_objWardWizFolderLocker;
	CWardWizEmailScan		m_objCWardWizEmailScan;
	CWardWizFirewall		m_objWardWizFirewall;
	CWardWizParentalControl		m_objCWardWizParentalControl;
	CWardWizFullScan		m_objCWardWizFullScan;
	CWardWizQuickScan		m_objCWardWizQuickScan;
	CWrdWizBrowserSecurity  m_objCWrdWizBrowserSecurity;
public:
	CRect							m_rect;
	static CWardWizUIDlg	 		*m_pThis;
	sciter::dom::element			m_root_el;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg LRESULT ShowRegMessageHandler(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMaximiseWindow(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:

	BEGIN_FUNCTION_MAP
		//Main DLG functions
		FUNCTION_0("OnClose", On_Close) // On_Close()
		FUNCTION_0("OnMinimize", On_Minimize) // On_Minimize()	
		FUNCTION_1("SendGlobalMessageboxCB", On_CallMessageBoxOnUI) // On_CallMessageBoxOnUI()	
		FUNCTION_0("GetProductID", On_GetProductID) // On_Minimize()
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_1("LoadHomepageControls",HomepageControls)
		FUNCTION_1("HomeControl", HomeControls)
		FUNCTION_1("LaunchUtilities", LaunchUtilities)
		FUNCTION_1("LaunchWardWizUpdate", LaunchWardWizUpdate)
		FUNCTION_1("ShowCheckboxValue", On_ShowCheckboxValue);
		FUNCTION_0("EnableDriverProtection", On_EnableDriverProtection);
		FUNCTION_1("OnGetRegValue", On_GetRegVal);
		FUNCTION_1("OnSetRegValue", On_SetRegVal);
		FUNCTION_1("OnToggleWidget", On_ToggleWidget);
		
		//To get support no. for indian product
		FUNCTION_0("GetSupportNo", GetSupportNo);

		//Registration related functions
		FUNCTION_0("OnClickRegisterNowBtn", On_ClickRegisterNowBtn)

			//Accounts Page related functions
			FUNCTION_0("GetRegisteredUserInfo", GetRegisteredUserInfo)

			FUNCTION_3("onModalLoop", onModalLoop)
			FUNCTION_0("GetAppPath", OnGetAppPath)
			FUNCTION_1("CallGetUIStatus", CallGetUIStatus)
			FUNCTION_0("GetDBPath", GetDBPath)
			FUNCTION_1("GetDecryptPasssword", GetDecryptPasssword)
			FUNCTION_0("GetDataBasePath", GetDataBasePath)
			FUNCTION_0("OnGetThemeID", On_GetThemeID)
			FUNCTION_0("ReadReportsDBPath", ReadReportsDBPath)
			FUNCTION_0("ReadValuesForHomePage", ReadValuesForHomePage)
			FUNCTION_0("OnCallHideUIDialog", On_CallHideUIDialog)
			FUNCTION_1("ReadCurrentValues", ReadCurrentValues)
			FUNCTION_0("OnClickRemoteAssistance", On_ClickRemoteAssistance)
			FUNCTION_1("FetchReportValue", FetchReportValue)
			FUNCTION_0("OnIsCheckOfline", On_IsCheckOfline)
			FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)

	END_FUNCTION_MAP
	
	//Sciter Variables
	SCITER_VALUE m_pSetRegistryScanStatusCB;
	SCITER_VALUE m_pMessageboxFunctionCB;
	SCITER_VALUE m_svDataToDisplayCB;
	SCITER_VALUE m_svDisabledCB;
	SCITER_VALUE m_svCallGetUIStatus;

	//Main DLG functions
	json::value On_Close();
	json::value On_Minimize();
	json::value On_CallMessageBoxOnUI(SCITER_VALUE svProcessCb);
	json::value On_ClickAccountsPage();
	json::value On_GetProductID();	
	json::value On_GetLanguageID();
	json::value LaunchUtilities(SCITER_VALUE svUtilityType);
	json::value LaunchWardWizUpdate(SCITER_VALUE svUpdateType);
	json::value On_EnableDriverProtection();
	
	json::value HomepageControls(SCITER_VALUE svDataToDisplayCB);
	json::value HomeControls(SCITER_VALUE svDataToDisplayCB);
	//Registration related functions
	json::value On_ClickRegisterNowBtn();

	//Accounts Page related functions
	json::value GetRegisteredUserInfo();

	//For Checkbox of Tooltip
	json::value On_ShowCheckboxValue(SCITER_VALUE svCheckBoxValue);

	json::value GetSupportNo();
	json::value On_IsCheckOfline();

	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value OnGetAppPath();
	json::value CallGetUIStatus(SCITER_VALUE svCallGetUIStatus);
	json::value GetDBPath();
	json::value GetDecryptPasssword(SCITER_VALUE svEncryptPasssword);
	json::value GetDataBasePath();
	json::value On_GetThemeID();
	json::value ReadReportsDBPath();
	json::value ReadValuesForHomePage();	
	json::value On_CallHideUIDialog();
	json::value On_GetRegVal(SCITER_VALUE svValueName);
	json::value On_SetRegVal(SCITER_VALUE svValueName);
	json::value On_ToggleWidget(SCITER_VALUE bToggleState);
	json::value ReadCurrentValues(SCITER_VALUE svFunCurrentValueCB);
	json::value On_ClickRemoteAssistance();
	json::value FetchReportValue(SCITER_VALUE svFunUpdateReportsValue);
	json::value FunCheckInternetAccessBlock();

public:
	bool				m_bAllowDemoEdition;
	bool				m_bNonGenuineCopy;
	bool				m_bData;
	bool                m_bLiveUpdateMsg;
	bool                m_bIsActiveProtectionON;
	bool				m_bVirusMsgDetect;
	bool				m_bIsRegInProgress;
	int					m_iProtectionStatusMsgType;
	int					m_iDisabled;
	int					m_iProtectedMsgType;
	CString				m_lpstrDate;
	CString				m_csRegProductVer;
	CString				m_csRegScanEngineVer;
	CString				m_csRegDataBaseVer;
	DWORD				m_dwvalueSType;
	CString				m_csDataEncVer;
	CString             m_szEmailId_GUI;
	CString             m_csProtectionStatusMsg;
	CString				m_csScanType;
	CString				m_csUpdateDate;
	CString				m_szvalueTM;
	TCHAR				m_csValueData[512];
	TCHAR				m_csDelearCod[512];
	TCHAR				m_csReferenceID[512];
	HANDLE				m_hThreadLaunchOprtn;
	DWORD 				 m_dwNoofDays;
	CISpyCommunicator	m_objComService;
	CISpyCommunicator	m_objComTray;
public:
	LRESULT OnUserMessagesHandleUIRequest(WPARAM wParam, LPARAM lParam);
	
	void ShowDataCryptOpr(bool bIsRequestFromExplorer);
	void CallUIMessageBox(CString csMessage, CString csMessageHeader, CString csMessageboxType);
	void ScanFinished();
	void SetTotalFileCount(LPISPY_PIPE_DATA lpSpyData);
	void ReadProductVersion4mRegistry();
	void ShowProductUpdate();
	void ShowScanPage();
	void ShowReports();
	void ShowHomepageControls(bool bEnable);
	void RegistryEntryOnUI();
	void GetDays();
	void SetNotProtectedMsg();
	bool isValidDate(int iDay, int iMonth, int iYear);
	bool GetInformationFromINI();

	void ShowRegistrationDialog();
	void ShowToolTipDialog();
	CString RemoveUnWantedPipeSymblFromPath(CString csSelectedArgumentPath);
	void StartWardWizTray();
	void OnStartUpScan();
	CString GetRegOptionsList();

	void WriteRegistryEntryofStartUpTips(SCITER_VALUE svChangeValue);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = true);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	bool SendData2Service(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam = NULL, LPTSTR lpszSecondParam = NULL, bool bWait = false);
	bool SendData2Tray(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam = NULL, LPTSTR lpszSecondParam = NULL, bool bWait = false);
	bool SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait = false);
	void ReloadHomePage();
	bool IsAnyChangeInDate();
	bool StartSheduledScan(DWORD dwScanType, DWORD dwShutDwonVal);
	bool StartSheduledScan4RegOpt(DWORD dwScanType, DWORD dwShutDwonVal, CString csRegOptList);
	bool CheckForTasksRunningUI();
	bool ClosePasswordWndForPC();
	void SendRegistryOptSchedScanData(LPVOID lpParam);
	void ReadCurrentRegValues(SCITER_VALUE svFunCurrentValueCB);
	bool CheckIsAppRunning(LPTSTR);
};
