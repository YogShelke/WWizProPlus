// USBDetectUIDlg.h : header file
//
#pragma once
#include "stdafx.h"
#include "JpegDialog.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "GradientStatic.h"
#include "USBDetectUI.h"
#include "DialogExpander.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "ScannerContants.h"
#include "TextProgressCtrl.h"
#include "PictureEx.h"
#include "StaticFilespec.h"
#include "ISpyDBManipulation.h"
#include "ExcludeFilesFolders.h"
#include "sciter-x-threads.h"
#include "ISpyCriticalSection.h"
#include "WardWizDatabaseInterface.h"

// CUSBDetectUIDlg dialog
class CUSBDetectUIDlg : public CJpegDialog
	, public CSciterBase
	,public sciter::host<CUSBDetectUIDlg> // Sciter host window primitives
{
	HELEMENT self;
// Construction
public:
	CUSBDetectUIDlg(CWnd* pParent = NULL);	// standard constructor
	~CUSBDetectUIDlg();

	//virtual bool subscription(HELEMENT he, UINT& event_groups)
	//{
	//	event_groups = UINT(-1);
	//	return true;
	//}
	 
	//// the only behavior_factory method:
	//virtual event_handler* create(HELEMENT he) { return this; }
	 
	//virtual void attached(HELEMENT he) {
	//	self = he; // if this is attached to windoww then he is NULL, understood, but where to keep this->root() ?
	//}
	//virtual void detached(HELEMENT he) {
	//	self = NULL;
	//}

	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();

// Dialog Data
	enum { IDD = IDD_USBDETECTUI_DIALOG };

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
	bool					m_bScanCount;
	DWORD					m_FileScanned;
	DWORD					m_dwIsAborted;
	CRect					m_rect;
	CStatic					m_stHeaderLogo;
	//int					m_iTotalFileCount;
	DWORD					m_iTotalFileCount;
	int						m_iThreatsFoundCount;
	DWORD					m_dwTotalThreatsCleaned;
	INT64					m_iScanSessionId;
	//CExpandDialog			m_ExpandDialog;
	CxSkinButton			m_btnShowHideButton;
	CColorStatic			m_stScanningText;
	CColorStatic			m_stStatusText;
	CColorStatic			m_stActualScnFileFolderName;
	CColorStatic			m_stActualStatus;
	CColorStatic			m_stFilescanned;
	CColorStatic			m_stStartTime;
	CColorStatic			m_stThreadFound;
	CColorStatic			m_stElapsedTime;
	CString					m_csFileScanned;
	CString					m_csStartUpTime;
	CStringArray			m_csaAllScanPaths;
	//CString					m_csThreadFound;
	CString					m_csElapsedTime;
	CxSkinButton			m_btnPauseResume;
	CxSkinButton			m_btnStop;
	CxSkinButton			m_btnClean;
	CListCtrl				m_lstVirusTrack;
	CColorStatic			m_stHideDetails;
	CColorStatic			m_stShowDetails;
	CColorStatic			m_stHeaderText;
	CxSkinButton			m_btnUsbClose;
	CxSkinButton			m_btnUsbMinimize;
	CColorStatic			m_stHideDetailsText;
	CColorStatic			m_stShowDetailstext;
	HMODULE					m_hScanDllModule;
	LOADSIGNATURE			m_lpLoadSigProc;
	UNLOADSIGNATURES		m_lpUnLoadSigProc;
	SCANFILE				m_lpScanFileProc;
	HBITMAP					m_bmpWardWizLogo;
	bool					m_bIsExpanded;
	CTime					m_tsScanPauseResumeTime;
	CTime					m_tsScanStartTime;
	CTimeSpan				m_tsScanPauseResumeElapsedTime;
	bool					m_bSignatureLoaded;
	CString					m_csPreviousFilePath;
	HANDLE					m_hUSBScanThread;
	HANDLE					m_hThreadUSbScanCount;
	CString					m_csEnumFolderORDriveName;
	CStringArray            m_csaEnumfolders;
	bool					m_bISScanning;
	bool					m_bQuarantineFinished;
	//int					m_dwFileScanned;
	DWORD					m_dwFileScanned;
	bool					m_bPlaySound;
	TCHAR					m_szShortPath[60];
	TCHAR					m_szUSBdetectedPath[60];
	CPictureEx				m_stUsbGifloader;
	CTextProgressCtrl		m_prgUsbProgressbar;
	CColorStatic			m_stUsbPercentage;
	CColorStatic			m_stPauseAndResumeStatus;
	bool					m_bRescan;
	bool					m_bClose;
	bool					m_bScanningAborted;	
	bool					m_bIsCleaning;
	bool					m_bIsPathExist;
	bool					m_bIsCloseFrmTaskBar;
	TCHAR					m_szCleaningShortPath[60];
	CColorStatic			m_stCleaningText;
	DWORD					dwSignatureFailedToLoad;
	//DWORD					m_dwScanOption;
	SCITER_VALUE			m_dwScanOption;
	CString					m_csUSBDriveName;
	CStaticFilespec			m_edtActualStatus;
	bool					m_bNoActionafterScanComplete;
	bool					m_bScanningCompleted;
	HANDLE					m_hQuarantineUSBThread;
	bool					m_bOnWMClose;
	CString					m_csStaticElapsedTime;
	CISpyDBManipulation		m_objDBManager;
	CISpyDBManipulation		m_objRecoverDB;
	CISpyCommunicator		m_objCom;
	CISpyCommunicator		m_objScanCom;
	CExcludeFilesFolders	m_objExcludeFilesFolders;
	CWardWizSQLiteDatabase	m_objSqlDb;
	bool					m_bIsShutDownScan;
	SCANTYPE				m_eScanType;
	bool					m_bQuickScan;
	bool					m_bFullScan;
	bool					m_bCustomscan;
	bool					m_bEPSCustomScan;
	sciter::dom::element	m_root_el;
	CISpyCriticalSection	m_crSectionPlaySound;
	int						m_iPercentage;
	SCITER_VALUE			m_svArrFileStatusDetails;
	SCITER_VALUE			m_svArrCustomScanSelectedEntries;
	void TokenizePath(CString csEnumFolderORDriveName);
	static void OnDataReceiveCallBack(LPVOID lpParam);
	bool ReadUISettingFromRegistry();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//afx_msg void OnBnClickedButtonHideShowDetailsButton();
	bool LoadRequiredDllForUSBScan();
	void EnumFolder(LPCTSTR pstr);
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonMinimize();
	bool ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID,DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan =  false);
	void InsertItem(CString strVirusName, CString strScanFileName, CString csAction, CString csISpyID);
	bool HandleVirusFoundEntries(CString strVirusName, CString strScanFileName, CString csAction, DWORD dwSpyID);
	bool ShutdownScanning(bool bClose = false);
	void EnumTotalFolder(LPCTSTR pstr);
	void HideAllElements();
	bool GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt);
	afx_msg void OnBnClickedButtonPauseResume(); 
//	afx_msg void OnBnClickedButtonClean();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnNMCustomdrawListViruslistControl(NMHDR *pNMHDR, LRESULT *pResult);
	void QuaratineUSBEntries();
	bool SendFile4RepairUsingService(int iMessage, CString csThreatPath,CString csThreatName, DWORD dwISpyID, bool bWait = false, bool bReconnect = false);
	bool ISAllItemsCleaned();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	bool SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry ,DWORD dwType, bool bWait = false, bool bReconnect = false);
	bool Check4DBFiles();
	afx_msg void OnClose();
	bool RefreshString();
	void CloseUI();
	bool USBScanningFinished();
	void GetShortFilePath(CString csFilePath);
	void SetLastScanDateTime( );
	bool SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait);
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait);
	void AddEntriesInReportsDB(CString csThreatName, CString csFilePath, CString csAction);
	bool SendReportOperations2Service(int dwMessageinfo, CString csReportFileEntry ,DWORD dwType, bool bWait);
	DWORD CheckForDeleteFileINIEntries();
	CString GetQuarantineFolderPath();
	char					m_chDriveleter;
	bool					m_bDeviceDetached;
	bool					m_bIsManualStop;
	bool					m_bUSBAutoScan;
	bool					bQuickScan;
	bool					bFullScan;
	CString					m_csTaskID;
	afx_msg LRESULT OnDriveRemovalStatus(WPARAM wParam, LPARAM lParam);
	CString m_csDriveLetter;
	int m_iDriveCompareResult;
	void DeviceRemoved();
	bool m_bMsgAbortPopup;
	bool m_bCloseMsgAbortPopup;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	bool Check4ValidDBFiles(CString csDBFolderPath);
	//added by Vilas on 07 April 2015 to handle failure cases
	bool SendFile4RepairUsingService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect);
	DWORD CheckForRepairFileINIEntries();
	//added by Neha Gharge 24 April 2015 to launch tray if it is not available
	void StartWardWizAVTray();
	CButton m_chkSelectAll;
	CColorStatic m_stSelectAll;
	afx_msg void OnBnClickedCheckSelectAll();
	bool InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus);
	bool SaveLocalDatabase();
	void CallUISetStatusfunction(CString csData, int iThreatsFoundCount, int iFileScanned);

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	bool CheckFileIsInRepairRebootIni(CString csFilePath);
	bool GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath);
	bool CheckFileIsInRecoverIni(CString csFilePath);
	bool CheckFileIsInDeleteIni(CString csQurFilePaths);
	bool TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName);
	bool TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName);
	void StatusTimer(LPTSTR lpFilePath);
	bool GetScanningPaths(CStringArray &csaReturn);
	bool GetAllDrivesList(CStringArray &csaReturn);
	bool SendData2Tray(DWORD dwMessage, DWORD dwValue, LPTSTR lpszFirstParam, bool bWait = false);

	//callback functions
	public:
		BEGIN_FUNCTION_MAP
			FUNCTION_5("USBThreatFind", OnStartUSBScan);
			FUNCTION_0("OnBnClickedButtonMinimize", On_OnBnClickedButtonMinimize);
			FUNCTION_0("OnResumeScan", On_ResumeScan);
			FUNCTION_0("OnPauseScan", On_OnPauseScan);
			FUNCTION_1("OnCloseButton", On_CloseButton);
			FUNCTION_3("OnCleanButton", On_OnCleanButton);
			FUNCTION_1("SendNotificationMessageCB", On_SendNotificationMessageCB);
			FUNCTION_0("GetProductID", On_GetProductID)
			FUNCTION_3("onModalLoop", onModalLoop)
			FUNCTION_0("GetAppPath", OnGetAppPath)
			FUNCTION_0("GetLanguageID", On_GetLanguageID)
			FUNCTION_0("PercentageTimer", On_PercentageTimer)
			FUNCTION_1("FileStatusTimer", On_FileStatusTimer)
			FUNCTION_0("GetDBPathforEPS", GetDBPathforEPS);
			FUNCTION_1("GetDecryptPasssword", GetDecryptPasssword)
			FUNCTION_0("OnGetThemeID", On_GetThemeID)
			FUNCTION_0("FunCheckInternetAccessBlock", FunCheckInternetAccessBlock)

		END_FUNCTION_MAP

		json::value OnStartUSBScan(SCITER_VALUE svAddVirusFoundEnteryCB, SCITER_VALUE svScanFinishedStatusCB, SCITER_VALUE svFilePathStatusCB, SCITER_VALUE svUSBdetectedDriveCB, SCITER_VALUE svPercentageCB);
		json::value On_OnBnClickedButtonMinimize();
		json::value On_ResumeScan();
		json::value On_OnPauseScan();
		json::value On_CloseButton(SCITER_VALUE svbIsManualStop);
		json::value On_OnCleanButton(SCITER_VALUE svArrRecords, SCITER_VALUE svUpdateThreatFoundEnteryCB, SCITER_VALUE svCleanFinishedCB);
		json::value On_SendNotificationMessageCB(SCITER_VALUE svFunNotificationMessageCB);
		json::value On_GetProductID();
		json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
		json::value OnGetAppPath();
		json::value On_GetLanguageID();
		json::value On_PercentageTimer();
		json::value On_FileStatusTimer(SCITER_VALUE svArrFileStatusDetails);
		json::value GetDBPathforEPS();
		json::value GetDecryptPasssword(SCITER_VALUE svEncryptPasssword);
		json::value On_GetThemeID();
		json::value FunCheckInternetAccessBlock();

		void StatusTimerSEH(LPTSTR csFilePath);
		INT64 InsertDataToTable(const char* szQuery);
		bool CheckFileOrFolderOnRootPath(CString csFilePath);
		bool SendData2EPSClient(LPCTSTR szFilePath, LPCTSTR szVirusName, DWORD dwActionTaken);
		bool SendScanFinishedData2EPSClient(DWORD dwIsAborted);
		bool isEPSNOUIInCommand(CString csCommandLine);
		bool isTaskIDInCommand(CString csCommandLine);
		DWORD getScanType(CString csCommandLine);
		bool CheckForWardWizCommand(CString csCommandVal);
public:
		SCITER_VALUE	m_svAddVirusFoundEnteryCB;
		SCITER_VALUE	m_svScanFinishedStatusCB;
		SCITER_VALUE	m_svFilePathStatusCB;
		SCITER_VALUE	m_svUSBdetectedDriveCB;
		SCITER_VALUE	m_svPercentageCB;
		SCITER_VALUE	m_svArrRecords;
		SCITER_VALUE	m_svUpdateThreatFoundEnteryCB;
		SCITER_VALUE	m_svvarMsgTypeCB;
		SCITER_VALUE	m_svstrMessageText;
		SCITER_VALUE	m_svFunNotificationMessageCB;
		SCITER_VALUE	m_svCleanFinishedCB;
		
		void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
};
