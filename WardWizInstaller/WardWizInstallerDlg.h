#include "sciter-x-threads.h"
#include "resource.h"
#include "WardWizInstaller.h"
#include "DownloadConts.h"
#include "iSpyMemMapClient.h"
#include "WinHttpClient.h"
#include "WinHttpManager.h"
#include "ScanByName.h"
#include "DownloadController.h"
#include "winioctl.h"

// WardWizInstallerDlg.h : header file
//

#pragma once

#define HTTP_RESOLVE_TIMEOUT		1000
#define HTTP_CONNECT_TIMEOUT		2000
#define HTTP_SEND_TIMEOUT			2000
#define HTTP_RECEIVE_TIMEOUT		3000
#define TIMER_SETFLEPATH_PER		50
#define TIMER_SETPERCENTAGE			400
#define TIMER_FORADVERTISEMENT		600
#define DEFAULT_RECT_WIDTH			150
#define DEFAULT_RECT_HEIGHT			30
#define WM_TRAYMESSAGE WM_USER
typedef bool(*WRDWIZPERCENTWITHFILENAME) (LPWSTR lpszFilePath);
typedef DWORD(*GETREGISTRATIONDATA)	(LPBYTE, DWORD &, DWORD dwResType, TCHAR *pResName);
typedef DWORD(*GETINSTALLATIONCODE)(LPCTSTR lpSerialNumber, LPCTSTR lpMID, LPTSTR lpInstallationCode);
typedef DWORD(*VALIDATERESPONSE)(LPTSTR lpActivationCode, BYTE bProductID, SYSTEMTIME &ServetTime, WORD &wwDaysLeft);


class CGUIInterface :
	public IGUIInterface
{
public:
	void SetDownloadedBytes(DWORD dwTotalFileLength, DWORD dwDownloadedBytes, double dTransferRate);
	void SetPercentDownload(int nPercent);
};

typedef struct _SYSDETAILS
{
	TCHAR szOsName[MAX_PATH];
	TCHAR szRAM[MAX_PATH];
	TCHAR szHardDiskSize[MAX_PATH];
	TCHAR szProcessor[MAX_PATH];
	TCHAR szCompName[MAX_PATH];
	TCHAR szLoggedInUser[MAX_PATH];
}SYSDETAILS;

class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CWardWizInstallerDlg dialog
class CWardWizInstallerDlg : public CDialogEx,
	public CSciterBase,
	public sciter::host<CWardWizInstallerDlg> // Sciter host window primitives
{
private:
	CGUIInterface				objGUI;

// Construction
public:
	long						g_lTotalDownloadedBytes;
	int							g_iPreviousPerc;
	DWORD						g_iPercentage;

	CWardWizInstallerDlg(CWnd* pParent = NULL);	// standard constructor
	~CWardWizInstallerDlg();

	HELEMENT				self;
	HWINDOW					get_hwnd();
	HINSTANCE				get_resource_instance();
	sciter::dom::element	m_root_el;


// Dialog Data
	enum { IDD = IDD_WARDWIZINSTALLER_DIALOG };

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
	afx_msg void OnBnClickedButtonResume();
	//afx_msg void OnBnClickedCancel();
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnBnClickedButtonWindowClose();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedCheckLaunch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCancel();

	HANDLE							m_hStartWardWizSetupDwnldProc;
	HANDLE							m_hFile;
	HMODULE						    m_hModuleSetupDLL;
	HANDLE							m_hThreadinstall;
	HANDLE							m_hThreadAvUnInstallThread;
	HANDLE							m_hOfflineNextEvent;
	HANDLE							m_hSendRequestThread;
	HANDLE							g_hFile;
	HANDLE							m_hProcess;
	HANDLE							m_hTmpProcess;
	CString							m_csTmpProcess;
	DWORD							m_dwTmpProcID;

	HMODULE							m_hOfflineDLL;
	HMODULE							m_RegisterationDLL;

	SCITER_VALUE					svLangValue;
	SCITER_VALUE					m_cvSendUninstallAvDetails2UI;
	SCITER_VALUE					m_svCallFinishXMLStatus;
	SCITER_VALUE					m_svPreInstallScnStatus;
	SCITER_VALUE					m_svUpdateDownloadStatus;
	SCITER_VALUE				    m_svUninstallString;
	SCITER_VALUE					m_svNotificationCall;
	SCITER_VALUE					m_svAddVirusFoundEntryCB;
	SCITER_VALUE					m_svSetScanFinishedStatusCB;
	SCITER_VALUE					m_svFunNotificationMessageCB;
	SCITER_VALUE					m_svStatusFunctionCB;
	SCITER_VALUE					m_svFunAddVirusFoundEntryCB;
	SCITER_VALUE					m_svFunSetScanFinishedStatusCB;
	SCITER_VALUE					m_svVirusCount;
	SCITER_VALUE					m_svArrayCleanEntries;
	SCITER_VALUE					m_cvRegLocDB;
	SCITER_VALUE					m_svFunSetRegStatusCB;
	SCITER_VALUE					m_svFunSetUserDetailsCB;
	SCITER_VALUE					m_svFunSetOfflineActKeyCB;
	SCITER_VALUE					m_svFunSetNoInternetMsg;
	SCITER_VALUE					m_svGetProdIdNProdVersion;
	SCITER_VALUE					m_svGetProdIdNVersion;
	SCITER_VALUE					m_svSetSystemInfoCB;
	SCITER_VALUE					m_svInstallFinishFlagCB;
	SCITER_VALUE					m_svAlreadyDownloadStatus;
	SCITER_VALUE                    m_svSetupDownloadSuccess;
	SCITER_VALUE                    m_svSetupNotExistStatus;
	SCITER_VALUE					m_svCloseMsg;
	SCITER_VALUE                    m_svVarOnClickOverWriteCB;
	DWORD							m_svGetResponseFinalInstall;
	SCITER_VALUE                    m_bIsSetupDownloadSuccess;
	SCITER_VALUE                    m_svChkVerMsg;
	SCITER_VALUE					m_svSetupInstalledSuccess;

	static	CWardWizInstallerDlg	*m_pThis;
	ULARGE_INTEGER					m_uliFreeBytesAvailable;
	ULARGE_INTEGER					m_uliTotalNumberOfBytes;
	ULARGE_INTEGER					m_uliTotalNumberOfFreeBytes;
	TCHAR							m_szModulePath[512];
	TCHAR						    m_szBrowseFolderName[512];
	TCHAR							m_szOfflineActivationCode[23];
	DWORD							dwDaysRemain;
	TCHAR							m_szInstallCode[32];
	TCHAR							m_szMachineIDValue[0x80];
	TCHAR							m_szDealerCode[64];
	TCHAR							m_szReferenceID[64];
	TCHAR							m_szCountry[512];
	TCHAR							m_szState[512];
	TCHAR							m_szCity[512];
	TCHAR							m_szPinCode[512];
	TCHAR							m_szEngineerName[512];
	TCHAR							m_szEngineerMobNo[512];
	CString                         m_csLangType;
	CString							m_csTempTargetFilePath;
	CString							m_stActualTransferRate;
	CString							m_stActualDownloadPath;
	CString							m_edtBrowsedSetupPath;
	CString							m_csDrivePath;
	CString							m_csActualProdName;
	CString							m_csFileName;
	CString							m_csFileTargetPath;
	CString							m_csServer;
	CString							m_csUserName;
	CString							m_csPassword;
	CString							m_csResponseData;
	TCHAR							m_szTempFolderPath[512];
	TCHAR							m_szIniFilePath[512];
	TCHAR							m_szServer[MAX_PATH];
	TCHAR							m_szUsername[MAX_PATH];
	TCHAR							m_szPassword[MAX_PATH];
	bool                            m_bIsWow64;
	bool                            m_bIsWrdwizInstall;
	bool							m_binstallInProgress;
	bool                            m_bIsCloseCalled;
	bool							m_bIsProcessPaused;
	bool							m_bIsDownloadingInProgress;
	bool							m_bRequiredSpaceNotavailable;
	bool							m_bchkOpenFolder;
	bool							m_bchkLaunchExe;
	bool							m_bchkDownloadoption;
	bool							m_bIsRegFlag;
	int								m_iCurrentDownloadedByte;
	DWORD							m_dwPercentage;
	DWORD							m_dwTotalFileSize;
	CTime							m_tsStartTime;
	CTime							m_tsScanEndTime;
	WRDWIZPERCENTWITHFILENAME       m_PercentagewithFilename;
	CWinHttpManager					m_objWinHttpManager;
	bool							m_bIsManualStop;
	bool							m_bStop;
	bool						    m_bIsManualStopScan;
	bool							m_bRedFlag;
	bool							m_bIsPathExist;
	bool							m_bManualStop;
	bool							m_ScanCount;
	bool							m_bScnAborted;
	bool							m_bScanStartedStatusOnGUI;
	bool							m_bRescan;
	bool							m_bFileFailedToDelete;
	bool							m_bTryProduct;
	bool							m_bActiveProduct;
	bool							m_bOnlineActivation;
	bool							m_bOfflineActivation;
	bool							m_bisThreadCompleted;
	bool							m_bIsProxySet;
	bool							m_bIsOffline;
	int								m_iThreatsFoundCount;
	int								m_iTotalFileCount;

	int								m_iPercentage;
	CString							m_csInstallationFilePath;

	HMODULE							m_hPsApiDLL;
	SCANTYPE						m_eCurrentSelectedScanType;
	SCANTYPE						m_eScanType;
	ENUMPROCESSMODULESEX		    EnumProcessModulesWWizEx;
	HANDLE							m_hThread_ScanCount;
	HANDLE							m_hWardWizAVThread;
	HANDLE							m_hQuarantineThread;
	CISpyScanner					m_objISpyScanner;
	CISpyCriticalSection			m_csQuarentineEntries;
	CISpyCriticalSection			m_csScanFile;
	CISpyDBManipulation				m_objISpyDBManipulation;
	DWORD							m_FileScanned;
	DWORD							m_dwTotalFileCount;
	DWORD							m_iMemScanTotalFileCount;
	DWORD							m_dwVirusFoundCount;
	DWORD							m_dwVirusCleanedCount;
	DWORD							m_wISpywareID;
	DWORD							m_dwAutoQuarOption;
	CString							m_csPreviousPath;
	CString							m_csCurrentFilePath;
	CString							m_csISpyID;
	CStringList						m_csaModuleList;
	CStringArray					m_csaAllScanPaths;
	CTime							m_tsScanStartTime;
	CTimeSpan						m_tsScanPauseResumeElapsedTime;
	unsigned char					*m_pbyEncDecKey;
	AVACTIVATIONINFO				m_ActInfo;
	CString							m_csProdVersion;
	DWORD							m_dwProductId;
public:
	TCHAR							m_szMachineId[MAX_PATH];	
	CString							m_csMachineId;
	CString                         m_csSetupBit;
	CString							m_csSetupLang;
	vector<HANDLE>					v_hProcess;
	SYSDETAILS						m_objSysdetails;
	CString                         m_szWWizInstallerLocationPath;
	TCHAR							m_csWWizSetupFinalPath[MAX_PATH];
	TCHAR							m_csWWizOfflineInstallerSetupPath[MAX_PATH];
	TCHAR							m_csWWizOfflineSetupFinalPath[MAX_PATH];
	TCHAR							m_csWWizAlreadyInstallerSetupPath[MAX_PATH];
	TCHAR							m_csWWizAlreadySetupFinalPath[MAX_PATH];
	bool							m_bIsOfflineInstaller;
	CString							m_csAppPath;
	CString                         m_GetOnlineSetupResponse = L"";
	TCHAR							m_WWIZSetupExeTmpPath[MAX_PATH];
	TCHAR							m_WWizOfflineSetupFinalPath[MAX_PATH];

	BEGIN_FUNCTION_MAP
		FUNCTION_2("GetProdIdNVersion", GetProdIdNVersion)
		FUNCTION_0("OnMinimize", On_Minimize)
		FUNCTION_0("OnCloseButton", On_Close) // On_Close()
		FUNCTION_0("PauseDownload", PauseDownload)
		FUNCTION_0("ResumeDownload", ResumeDownload)
		FUNCTION_5("OnInstallWardwizSetup", StartInstallWardwizSetup);
		FUNCTION_3("CheckOtherAVProduct", CheckOtherAVProduct);
		FUNCTION_0("GetRestartPC", On_GetRestartPC);
		FUNCTION_0("ExitSetup", On_ExitSetup);
		FUNCTION_1("OnRestartForBootScan", On_RestartForBootScan)
		FUNCTION_0("OnClickResume", On_BtnClickResume)
		FUNCTION_2("LaunchAvToUninstall", On_LaunchAvToUninstall)
		FUNCTION_2("OnStartQuickScan", On_StartQuickScan)
		FUNCTION_2("OnClickScanCleanBtn", On_ClickScanCleanBtn)
		FUNCTION_3("onModalLoop", onModalLoop)
		FUNCTION_5("OnLoadInstaller", OnLoadInstaller)
		FUNCTION_1("GetString", Get_String);
		FUNCTION_0("GetDBPath", GetDBPath)
	    FUNCTION_4("OnClickTrialProduct", On_ClickTrialProduct)
	    FUNCTION_4("OnClickActivateProduct", On_ClickActivateProduct)
	    FUNCTION_3("GetOfflineActivationKey", GetOfflineActivationKey)
	    FUNCTION_2("OnClickOfflineRegistration", On_ClickOfflineRegistration)
	    FUNCTION_0("FunGetMachineId", FunGetMachineId)
	    FUNCTION_0("GetInternetConnection", On_GetInternetConnection)
	    FUNCTION_0("GetHostFileForWWRedirectionFlag", GetHostFileForWWRedirectionFlag)
	    FUNCTION_0("LaunchWwizNExitInstaller", LaunchWwizNExitInstaller)
	    FUNCTION_1("SetSelectedLanguage", SetSelectedLanguage)
	    FUNCTION_0("OnPauseQuickScan", On_PauseQuickScan)
	    FUNCTION_0("OnResumeQuickScan", On_ResumeQuickScan)
	    FUNCTION_0("OnPauseInstallation", On_PauseInstallation)
	    FUNCTION_0("OnResumeInstallation", On_ResumeInstallation)
	    FUNCTION_1("GetSystemInformation", GetSystemInformation)
		FUNCTION_0("CheckAnyUninstallationPorcessIsRunning", CheckAnyUninstallationPorcessIsRunning)
		FUNCTION_1("OnSetProxySetting", On_SetProxyServer)
		FUNCTION_0("CheckIsOffline", On_CheckIsOffline)
		FUNCTION_1("CloseWWizRunningApp", OnCloseWWizRunningApp)
		FUNCTION_1("GetResponseFinalInstall", OnGetResponseFinalInstall)
	END_FUNCTION_MAP

	json::value SetSelectedLanguage(SCITER_VALUE);
	json::value GetProdIdNVersion(SCITER_VALUE svGetProdIdNProdVersion, SCITER_VALUE svGetProdIdNVersion);
	json::value OnLoadInstaller(SCITER_VALUE svUpdateDownloadStatus, SCITER_VALUE svNotificationCall, SCITER_VALUE svProductId, SCITER_VALUE svAlreadyDownloadStatus, SCITER_VALUE svSetupDownloadSuccess);
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);
	json::value StartInstallWardwizSetup(SCITER_VALUE svInstallFinishFlag, SCITER_VALUE svbRegFlag, SCITER_VALUE svCloseMsg, SCITER_VALUE svChkVerMsg  ,SCITER_VALUE svChkInstallFinishStatus);
	json::value CheckOtherAVProduct(SCITER_VALUE svCallFinishXMLStatus, SCITER_VALUE cvSendUninstallAvDetails2UI, SCITER_VALUE svPreInstallScnStatus);
	json::value On_Close();
	json::value PauseDownload();
	json::value ResumeDownload();
	json::value On_GetRestartPC();
	json::value On_ExitSetup();
	json::value On_Minimize();
	json::value On_BtnClickResume();
	json::value On_LaunchAvToUninstall(SCITER_VALUE m_svUninstallStringFirstParam, SCITER_VALUE m_svUninstallStringSecondParam);
	json::value On_StartQuickScan(SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB);
	json::value On_ClickScanCleanBtn(SCITER_VALUE svArrCleanEntries, SCITER_VALUE svQarantineFlag);
	json::value Get_String(SCITER_VALUE svStringValue);
	json::value On_RestartForBootScan(SCITER_VALUE svBootFlag);
	json::value GetDBPath();
	json::value LaunchWwizNExitInstaller();
	json::value On_ClickTrialProduct(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetRegStatusCB, SCITER_VALUE svFunSetNoInternetMsg, SCITER_VALUE svResponse);
	json::value On_ClickActivateProduct(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetRegStatusCB, SCITER_VALUE svFunSetNoInternetMsg, SCITER_VALUE svResponse);
	json::value GetOfflineActivationKey(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetOfflineActKeyCB, SCITER_VALUE svFunSetNoInternetMsg);
	json::value On_ClickOfflineRegistration(SCITER_VALUE svOfflineActKey, SCITER_VALUE svFunSetRegStatusCB);
	json::value On_GetInternetConnection();
	json::value On_GetProxyRegistry();
	json::value GetHostFileForWWRedirectionFlag();
	json::value FunGetMachineId();
	json::value On_PauseQuickScan();
	json::value On_ResumeQuickScan();
	json::value On_PauseInstallation();
	json::value On_ResumeInstallation();
	json::value GetSystemInformation(SCITER_VALUE svSetSystemInfoCB);
	json::value CheckAnyUninstallationPorcessIsRunning(); 
	json::value On_SetProxyServer(SCITER_VALUE svArrProxyDetails);
	json::value On_CheckIsOffline();
	json::value OnCloseWWizRunningApp(SCITER_VALUE svGetResult);
	json::value OnGetResponseFinalInstall(SCITER_VALUE svGetResponseFinalInstall);

	BOOL GetDriveGeometry(DISK_GEOMETRY *pdg);
	bool SystemInfoDetails();	
	bool GetSystemDetails();
	bool InstallWardwizSetup();
	bool CheckOtherAVProductWardwizSetup();
	int GetTaskBarWidth();
	int GetTaskBarHeight();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CString GetModuleFileStringPath();
	CString GetRegLocDBPath(LPTSTR szRegUserLocDetails);
	void IsWow64();
	void CancelUI();
	void CloseUI();
	VOID ShowNotifyIcon(HWND hWnd, DWORD dwAdd, CString csMessage);
	VOID RestoreWndFromTray(HWND hWnd);
	BOOL GetDoAnimateMinimize(VOID);
	void StartInstallProgram();
	void StartCheckOtherAvProgram();
	VOID GetTrayWndRect(LPRECT lpTrayRect);
	BOOL InstallWWizSetupThrCommandLine(LPTSTR pszAppPath, LPTSTR pszCmdLine);
	HANDLE GetTmpProcessHandle(CString csTmpProcessName);

public:
	static void OnDataReceiveCallBack(LPVOID lpParam);
	bool SendWWizInstallationPath2UI(LPTSTR szFilePath, int iPercentage);
	bool SendWWizUninstallAvDetails2UI(LPTSTR szUninstallAvDetails);
	void SendWWizXMLFinishFlag(DWORD dwFinishFlag);
	bool SendExtractFinishedFlag(LPTSTR m_csSetupExtractTempFolderPath);
	bool StartDownloadingWardWizSetup(LPCTSTR szUrlPath);
	void SetStartupEntry(TCHAR* szDownloaderPath);
	bool GetStartupEntry();
	bool CheckInternetConnection();
	bool WaitForInternetConnection();
	bool IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize);
	bool BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath, LPTSTR lpszBackupFilePath);
	DWORD GetPercentage(int iDownloaded, int iTotalSize);
	void OnBnClickedButtonRun();
	void StartScanning();
	void GetModuleCount();
	void EnumFolder(LPCTSTR pstr);
	DWORD LoadSignatureDatabase(DWORD &dwSigCount);
	DWORD UnLoadSignatureDatabase();
	bool IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize);
	bool CheckFileOrFolderOnRootPath(CString csFilePath);
	bool ScanFinished();
	bool on_timer(HELEMENT he);
	bool on_timerSEH(HELEMENT he);
	bool OnClickCleanButton(SCITER_VALUE svArrayCleanEntries);
	bool GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash);
	bool QuarantineEntry(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath);
	bool InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus);
	bool SaveInRecoverDB();
	bool CheckIFAlreadyBackupTaken(LPCTSTR szFileHash, LPTSTR szBackupPath);
	bool CheckEntryPresent(LPCTSTR szFileHash, LPTSTR szBackupPath);
	bool CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize);
	CString GetString(CString csStringID);
	CString GetProgramFilePath();
	DWORD GetSelectedLanguage();
	DWORD HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThreatName1, DWORD dwISpyID, CString &csBackupFilePath, DWORD &dwAction);
	DWORD CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath);
	DWORD Encrypt_File(TCHAR *szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	void EnumFolderForScanning(LPCTSTR pstr);
	void EnumerateProcesses();
	void ScanForSingleFile(CString csFilePath);
	void CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID);
	void OnTimerScan();
	void UIOnTimer();
	void OnTimerSetFilePath();
	void CallUISetStatusfunction(LPTSTR lpszPath);
	void CallUISetFileCountfunction(CString csTotalFileCount, CString csCurrentFileCount);
	void CallUISetStatusfunctionSEH(LPTSTR lpszPath);
	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
	void CallUISetScanFinishedStatus(CString csData);
	void LoadPSAPILibrary();
	void WriteFileForBootScan(CString csQurFilePaths, CString csVirusName);
	void LoadRecoversDBFile();
	void PerformRegistration(SCITER_VALUE svArrUserDetails);
	void DisplayFailureMsgOnUI(CString csMessageType, CString strMessage);
	bool ReadHostFileForWWRedirection();
	DWORD GetRegisteredUserInfo(DWORD dwProdID);
	DWORD GetRegistrationDateFromServer(LPSYSTEMTIME lpServerTime, LPDWORD lpDaysRemaining);
	void AddProdRegInfoToLocal(AVACTIVATIONINFO	ActInfo, DWORD m_dwResSize);
	void DisplayFailureMessage(REGFAILUREMSGS dwRes);
	void DisplayFailureMessageForOfflineRegistration(REGFAILUREMSGS dwRes);
	bool SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName, bool bRegWait);
	DWORD AddRegistrationDataInRegistry();
	DWORD AddRegistrationDataInFile();
	DWORD SpreadRegistrationFilesInSystem();
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPBYTE byData, DWORD dwLen, bool bWait);
	DWORD GetProxySettingsFromRegistry();
	DWORD ExtractDate(TCHAR *pTime, LPSYSTEMTIME lpServerTime, LPDWORD lpdwServerDays);
	bool LoadWrdWizOfflineRegDll();
	bool WriteMachineID2Registry();
	bool GetMachineIDOnMemory();
	bool GetVGAAdapterID(TCHAR *pszVGAID);
	bool CopyFileToDestination(TCHAR *pszSource, TCHAR *pszDest);
	bool CheckDestinationPathExists(LPCTSTR DestinationPath);
	bool CheckForMachineID(const AVACTIVATIONINFO	&actInfo);
	DWORD GetCPUID(TCHAR *pszCPUID);
	DWORD GetBIOSSerialNumber(TCHAR *pszBIOSSerialNumber);
	DWORD RemoveCharsIfExists(TCHAR *pszBIOSSerialNumber, int iLen, int iSize, TCHAR chRemove);
	DWORD GetBIOSSerialNumberSEH(TCHAR *pszBIOSSerialNumber);
	DWORD GetMotherBoardSerialNumberSEH(TCHAR *psMotherBoardSerialNumber);
	DWORD GetMotherBoardSerialNumber(TCHAR *psMotherBoardSerialNumber);
	DWORD GetMACAddress(TCHAR *pMacAddress, LPTSTR lpszOldMacID = NULL);
	DWORD GetRegistrationDataFromRegistry();
	DWORD GetRegistrationDatafromFile();
	DWORD GetRegistrationDatafromFile(CString strUserRegFile);
	DWORD DecryptDataReg(LPBYTE lpBuffer, DWORD dwSize);
	DWORD AddRegDataToRegistry();
	void CopyUserLocDetails();
	void InstallFinishStatus();
	bool StartDownloadFile(LPCTSTR szUrlPath, TCHAR szTargPath[MAX_PATH]);
	bool CheckForCorruption(LPTSTR szZipFilePath);
	bool RegisterForProtection();
	bool PerformPostRegOperation();
	bool TerminateInstallThreads();	
	void ForceTerminate();
	CString CheckDownloadedSetup();
	bool CheckOfflineInternetConnection();
	bool SendInstallerLocationPath(LPTSTR m_csInstallerLocationPath);
	bool  CheckOfflineInstallerSetup();
	bool ReinstallCloseAppMsg(CString csMessageContent);
	void UnregisterComDll(CString csAppPath);
	CString CheckAlreadySetupExit();
	bool GetInstallerLocationWardwizSetup();
	bool OfflineInstallWardwizSetup();
	bool DownloadTempLocationInstallWardwizSetup();
	bool CheckVersionAppMsg(CString csFirstMessageContent, CString csSecondMessageContent, CString csAppPathContent);
	bool ParseVersionString(int iDigits[4], CString& csVersion);
	int CompareVersions(int iVersion1[4], int iVersion2[4]);
	bool SendFinishedInstallationsFlag(DWORD dwInstallFinishFlag);

	CDownloadController*		m_pDownloadController;
	CWinHttpManager				g_objWinHttpManager;

	virtual bool handle_timer(HELEMENT he, TIMER_PARAMS& params)
	{
		__try
		{
			return on_timer(he);
		}
		__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
		{
			AddLogEntry(L"### Exception in CWardwizScan::handle_timer", 0, 0, true, SECONDLEVEL);
		}
		return true;
	}
};
