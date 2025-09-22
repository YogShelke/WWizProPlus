/**********************************************************************************************************            		  Program Name          : WardWiz Communication service
Description           : This file contains functionality about the service
Author Name			: Ramkrushna Shelke                                                                        		  Date Of Creation      : 10 Jan 2014
Version No            : 1.0.0.8
Special Logic Used    : Communication using named pipes & using Shared Memory.
Modification Log      :
***********************************************************************************************************/
#include <Windows.h>
#include <tchar.h>
#include "ISpyDataManager.h"
#include "ISpyScannerBase.h"
#include "scanuk.h"
#include "scanuser.h"
#include "ConCurrentQueue.h"
#include "AVRegInfo.h"
#include "sqlite3.h"
#include "time.h"
#include <map>
#include "winioctl.h"
#include "CSecure64.h"
#include "CScannerLoad.h"

TCHAR	szSrvName[]				= L"VibraniumComSrv" ;
TCHAR	szSrvDisplayName[]		= L"Vibranium Communication Module" ;

SERVICE_STATUS					ServiceStatus = {0} ;
SERVICE_STATUS_HANDLE			hServiceStatus = NULL ;

std::vector<CString>			g_vFailedToDeleteEvenOnRestart;

std::vector<CString>			m_vCsFaildFilePath;
std::vector<CString>			m_vCsTempFileList;
std::vector<DWORD>				m_vDwCRC;
std::vector<DWORD>				m_vRollbackAttemp;
CStringArray					m_csArrRegOptions;

typedef struct _SCANNER_THREAD_CONTEXT {

	HANDLE Port;
	HANDLE Completion;

} SCANNER_THREAD_CONTEXT, *PSCANNER_THREAD_CONTEXT;

typedef struct __tagActiveScanOptions
{
	WCHAR	szUserName[100];
	ULONG	ulCallType;
	HANDLE	hFileObjectHash;
	WCHAR	wcProcessPath[1024];
	DWORD	dwScanTryCount;
}ACTIVESCANFLAGS, *PACTIVESCANFLAGS;

typedef struct __tagBlockAppOptions
{
	ULONG	ulCallerProcessId;
	WCHAR	wcProcessPath[1024];
}BLOCKAPPFLAGS, *PBLOCKAPPFLAGS;

typedef struct _tagSchedScanList
{
	WCHAR szData[MAX_PATH];
	int iStngFKey;
}SCHEDSCANLIST;

typedef struct _tagSchedScanStng
{
	int iStngID;
	int iSchedType;
	int iSchedLowBat;
	int iSchedShutDown;
	int iSchedWakeUp;
	int iSchedScanSkip;
	int SchedScanTime;
	BYTE byScanSun;
	BYTE byScanMon;
	BYTE byScanTue;
	BYTE byScanWed;
	BYTE byScanThur;
	BYTE byScanFri;
	BYTE byScanSat;
	int iScanType;
	SYSTEMTIME SchedScanDate;
}SCHEDSCANSTNG;

typedef struct _tagSchedFinishList
{
	int iSchType;
	int SchScanTime;
	int iScanType;
	SYSTEMTIME SchedDate;
}SCHEDSCANFINISH;

typedef struct _tagSchedMissedList
{
	int iSchMissID;
	int iSchTypeMiss;
	int SchScanTimeMiss;
	int iScanTypeMiss;
	SYSTEMTIME SchedDateMiss;
}SCHEDSCANMISS;

enum __TAGUSERRESPONSE
{
	NO,
	YES
}USERRESPONSE;

typedef struct _SYSDETAILS
{
	TCHAR szOsName[MAX_PATH];	
	TCHAR szRAM[MAX_PATH];
	TCHAR szHardDiskSize[MAX_PATH];
	TCHAR szProcessor[MAX_PATH];
	TCHAR szCompName[MAX_PATH];
}SYSDETAILS;

typedef std::vector <SCHEDSCANLIST> SCHEDSCANLISTMAP;
typedef std::vector <SCHEDSCANSTNG> SCHEDSCANSTNGMAP;
typedef std::vector <SCHEDSCANFINISH> SCHEDSCANFINMAP;
typedef std::vector <SCHEDSCANMISS> SCHEDSCANMISSMAP;

SCHEDSCANLISTMAP				m_vSchedScanList;
SCHEDSCANSTNGMAP				m_vSchedScanStng;
SCHEDSCANFINMAP					m_vSchedScanFinish;
SCHEDSCANMISSMAP				m_vSchedScanMiss;

typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
typedef  int(*SQLITE3_PREPARE)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
typedef  int(*SQLITE3_CLOSE)(sqlite3 *);
sqlite3							*m_pdbfile;
HMODULE							m_hSQLiteDLL;
SQLITE3_OPEN					m_pSQliteOpen;
SQLITE3_PREPARE					m_pSQLitePrepare;
SQLITE3_COLUMN_COUNT			m_pSQLiteColumnCount;
SQLITE3_STEP					m_pSQLiteStep;
SQLITE3_COLUMN_TEXT				m_pSQLiteColumnText;
SQLITE3_CLOSE					m_pSQLiteClose;
bool							m_bISDBLoaded;

DWORD							m_dwDaysLeft;
DWORD							m_TotalThreatsCleaned;
TCHAR							m_szSpecificPath[512];

void WINAPI ServiceMain(DWORD dwArgC, LPTSTR *pArgV) ;
void WINAPI ServiceCtrlHandler( DWORD dwCtrlCode ) ;
BOOL UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) ;
INT64 InsertSQLiteData(const char* szQuery);
INT64 InsertSQLiteDataForStng(const char* szQuery);

HANDLE hEvent_ServiceStop = NULL;
BOOL   bStopService = FALSE;
void StopServiceByEvent() ;

void StartServiceWorkerThread(DWORD dwArgC, LPTSTR *pArgV) ;
void StartServiceWorkerThreadSEH(DWORD dwArgC, LPTSTR *pArgV);

HANDLE	hThread_SrvPrimary = NULL ;
DWORD SrvPrimaryThread( LPVOID lpParam ) ;
DWORD SrvPrimaryThreadSEH(LPVOID lpParam);

HANDLE hThread_WatchReportsAction = NULL;
HANDLE hSendUserInformationThread = NULL;
HANDLE hIndexingThread = NULL;
HANDLE hWatchDogThread = NULL;
HANDLE hActScanSetThread = NULL;
HANDLE hActiveScanEvent = NULL;
HANDLE hSchedScanThread = NULL;
HANDLE hEmailScanThread = NULL;
HANDLE hFirewallThread = NULL;
HANDLE hSchScanEvent = NULL;
HANDLE hParCtrlEvent = NULL;
HANDLE hPCtrlCompUsgEvent = NULL;
HANDLE hPCtrlINetEvent = NULL;
HANDLE hSendRegInfoEvent = NULL;
HANDLE hPCtrlINetUsgEvent = NULL;
HANDLE m_hParCtrlThread = NULL;
HANDLE m_hParCtrlCheckThread = NULL;
HANDLE m_hParCtrlInternet = NULL;
HANDLE m_hParCtrlINetCheckThread = NULL;
HANDLE	g_hPort = NULL;
HANDLE	g_hCompletion = NULL;
HANDLE	m_hThreadTempFileClnr;

DWORD WatchReportsActionThread(LPVOID lpParam);
DWORD WatchDogThread(LPVOID lpParam);
DWORD SendUserInformationThread(LPVOID lpParam);
DWORD IndexingThread(LPVOID lpParam);
DWORD HandleActiveScanSettingsThread(LPVOID lpParam);
DWORD SchedScanThread(LPVOID lpParam);
DWORD Thread_StartTempFileCleaner(LPVOID lpParam);
//*********************************************************

bool WriteMachineID2Registry();
DWORD GetCPUID( TCHAR *pszCPUID );
DWORD GetBIOSSerialNumber(TCHAR *pszBIOSSerialNumber );
DWORD RemoveCharsIfExists(TCHAR *pszBIOSSerialNumber, int iLen, int iSize, TCHAR chRemove );

DWORD InstallService( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName) ;
DWORD UnInstallService( TCHAR *pszSrvName ) ;
DWORD StartServiceManually( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName ) ;
DWORD StopServiceManually( TCHAR *pszSrvName ) ;

DWORD QueryServiceStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus) ;
DWORD QueryServiceStartStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus) ;

DWORD SetServiceFailureAction( SC_HANDLE hService ) ;
DWORD ReadDeleteReportsValueFromRegistry();
bool DeleteOldReports(DWORD dwDays);
void OnDataReceiveCallBack(LPVOID lpParam);
void AddServiceEntryInSafeMode( );
DWORD GetMotherBoardSerialNumber(TCHAR *psMotherBoardSerialNumber );
bool GetVGAAdapterID( TCHAR *pszVGAID );
DWORD GetMACAddress(TCHAR *pMacAddress, LPTSTR lpszOldMacID = NULL);
bool CheckForMIDInRegistry();
DWORD GetBIOSSerialNumberSEH(TCHAR *pszBIOSSerialNumber );
DWORD GetMotherBoardSerialNumberSEH(TCHAR *psMotherBoardSerialNumber );
void  Check4StartUpEntries();
bool CreateRequiredDirs();
void CheckScanLevel();
bool DeleteFileFromINI();
CString GetQuarantineFolderPath();
bool TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwsizeofApplnName, DWORD &dwAttempt);

//Added by Vilas on 23 Mar 2015 for reboot repair
HANDLE	hThread_RepairViruses = NULL;
DWORD RepairVirusesThread(LPVOID lpParam);

//Added by Ram on 27 April 2015 to load signature database.
HANDLE	hThreadLoadSigDatabase = NULL;
DWORD	LoadWardWizSignatureDatabase(LPVOID lpParam);

bool ParseIniLineAndSendToRepair(LPTSTR lpszIniLine);

//Added by Vilas on 27 Mar 2015 for delete entries in WWRecover.ini on next reboot
DWORD DeleteRecoverEntriesFromIni();

//Added by Gayatri A. For OnAccess related changes
DWORD PopulateScanQueue(__in PSCANNER_THREAD_CONTEXT Context);
DWORD PopulateScanQueueSEH(__in PSCANNER_THREAD_CONTEXT Context);
void ProcessScanQueueThread(CConCurrentQueue<ACTIVESCANFLAGS>& pObjeOnAccess);
void ProcessScanQueueThreadSEH(CConCurrentQueue<ACTIVESCANFLAGS>& pObjeOnAccess);

//OnAccess changes end here

//Adding User, computer name and OS details
//Added by Vilas on 05 May 2015
void AddUserAndSystemInfoToLog();
void ParseIntegrityInfoINIAndRollBackCRC();
bool AddFileInfoToINI(CString csfilePath, DWORD dwCRC, DWORD dwAttemp);
DWORD AddCheckSum(LPCTSTR lpFileName, DWORD dwByteNeedtoAdd);
bool CheckInternetConnection();
void SendUserInformation2Server(int iRegInfoType);
bool Check4OfflineActivationFlag();
bool CheckForMIDInRegistry();
bool SendMessage2Tray(int iRequest);
bool SendMessage2UI(int iRequest);
bool StopRunningOperations();
bool AddProtectionToLiveUpdateFolder();
void CheckIsOffline();
DWORD HandleVirusEntry(LPISPY_PIPE_DATA lpSpyData);
void GetFileDigits(LPCTSTR pstr, vector<int> &vec);
void MergeRecoverEntries();
void MergeIntoRecoverDB(CDataManager &objdb);
bool SaveDBFile();
bool GetSortedFileNames(vector<int> &vec);
bool LoadDataContentFromFile(CString csPathName, CDataManager &objdb);
void PopulateList(bool bCheckEntry);
DWORD ScanFile(LPISPY_PIPE_DATA lpSpyData);
bool SaveLocalDBFiles();
void StartIndexing();
bool HandleActiveScanSettings(LPISPY_PIPE_DATA lpSpyData);
bool HandleProductSettings(LPISPY_PIPE_DATA lpSpyData);
bool ReLoadSignatures();
bool ReloadRegistrationDays();
void consume(CConCurrentQueue<std::wstring>& pObjeOnAccess, unsigned int id);
bool AddEntryIntoFile(LPTSTR lpszEntry, LPTSTR lpszFilePath);
bool LoadActiveScanEntryFromFile(CConCurrentQueue<ACTIVESCANFLAGS> &onAccessQueue, LPTSTR lpszFilePath);
bool LoadActiveScanEntryFromFileSEH(CConCurrentQueue<ACTIVESCANFLAGS> &onAccessQueue, LPTSTR lpszFilePath);
bool ISExcludedPathAndExt(LPTSTR lpszFilePath);
bool ClearIndexing();
bool StartStopIndexing(DWORD dwOption);
bool ApplyAutorunScanSettings();
bool AddIntoProcessingQueue(PWCHAR usAccessedFileName, LPTSTR lpszUserName);
bool GenerateMachineID(LPTSTR szClientID, DWORD dwSize, bool bUsingHDDSerial, LPTSTR lpszHDDSerial, LPTSTR lpszOldMacID = NULL);
bool CheckForHardDiskSerialNumber(LPTSTR szClientID, LPTSTR szHDDSerial);
bool GetRegisteredUserInformation();
DWORD GetRegistrationDataFromRegistry();
DWORD DecryptRegistryData(LPBYTE lpBuffer, DWORD dwSize);
DWORD GetRegistrationDatafromFile();
CString GetModuleFileStringPath();
DWORD GetRegistrationDatafromFile(CString strUserRegFile);
bool StartScheduledScanThread();
void GetRecordsSEH();
bool LoadRequiredLibrary();
void CheckForSchedScan();
bool CheckForCompletedScan(int iSchedtime);
bool StartScheduledScanner(DWORD dwScanType, CString csScanCmdParam, int iShutDown);
void StartSchedScanProcess(int iScanID, DWORD dwScanType, int iSchedScanTypeFin, int lpSchTime, int iScanTypeFin, CString csScanCmdParam, int iShutDown);
int CheckForMonth(int intMonth, int intYear);
bool AddBootScannerEntry(LPTSTR pKeyName, LPTSTR pValueName, LPTSTR pNewValue);
bool RemoveBootScannerEntry(LPTSTR pKeyName, LPTSTR pValueName);
bool CheckForMissedDuplicate(int iSchedtime);
DWORD SendMessage2Tray(int iRequest, bool bWait);
void GetFileFoldList(int iScanID, CString &csScanCmdParam);
void GetRegOptionList(int IScanID, CString &csScanCmdParam);
bool ReadBootScannedEntries();
bool SendData2Tray(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam = NULL, LPTSTR lpszSecondParam = NULL, bool bWait = false);
bool SendAppBlock2Tray(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam, LPTSTR lpszSecondParam);
bool SendData2UI(int iMessage, DWORD dwValue, DWORD dwSeondValue, bool bWait = false);
bool ResetBootTimeScanner();
bool IsAnyTaskRunnning();
bool SendData2EPSClient(LPCTSTR szFilePath, LPCTSTR szVirusName, DWORD dwActionTaken);
bool LauchAppInUserContext(LPISPY_PIPE_DATA lpSpyData);
bool LauchApplication(LPISPY_PIPE_DATA lpSpyData);
DWORD StartEMailScanWorkerThread(LPVOID lpParam);
DWORD StartFirewallWorkerThread(LPVOID lpParam);
DWORD StartEMailScanWorkerThreadSEH(LPVOID lpParam);
DWORD StartFirewallWorkerThreadSEH(LPVOID lpParam);
void StartEmailScanThread();
void StartFirewallThread();
void StartParentalControlCheck();
void StartPCCompUsageTimeCheck();
void StartPCInternetCheck();
void StartINetAccessTimeCheck();
DWORD StartParCtrlWrkerThread(LPVOID lpParam);
DWORD StartPCCompUsageTimeCheckThread(LPVOID lpParam);
DWORD StartPCInternetTimeCheckThread(LPVOID lpParam);
DWORD StartINetAccessTimeCheckThread(LPVOID lpParam);
bool ReloadApplicationRules(LPISPY_PIPE_DATA lpSpyData);
bool ReloadApplicationRestriction();
bool UninitializeNFApi();
bool ReloadFirewallRules();
bool ReLoadParentalControlSettings(LPISPY_PIPE_DATA lpSpyData);
bool ReLoadFWControlSettings(LPISPY_PIPE_DATA lpSpyData);
bool ReLoadEmailScanSettings(LPISPY_PIPE_DATA lpSpyData);
bool StartParentalCtrlCheck();
bool StartParCtrlCompUsageCheck();
bool StartParCtrlInternetCheck();
bool StartParCtrlINetUsageCheck();
bool AddIntoAppBlockQueue(ULONG ulCallerProcID, PWCHAR usAccessedFileName);
void ProcessAppBlockThread(CConCurrentQueue<BLOCKAPPFLAGS>& pObjeAppBlock);
void ProcessAppBlockThreadSEH(CConCurrentQueue<BLOCKAPPFLAGS>& pObjeOnAccess);
bool LaunchApplicationUsingService(LPISPY_PIPE_DATA lpSpyData);
void ReLoadApplicationOnSwitchUser(LPTSTR szParam, DWORD dwSessionID);
bool OnFileQuarantineFromRecover(LPTSTR szRecoverVal);
bool ReloadExcludeDB();
SYSDETAILS GetSystemDetails();
void AddSystemDetailsToINI();
BOOL GetDriveGeometry(DISK_GEOMETRY *pdg);

void CleanTemporaryFiles();
void StartTempFileScanning();
void EnumFolder(LPCTSTR lpFolPath);
void CheckFileForViruses(LPCTSTR lpFileName);
void AddEntryinUITable(LPCTSTR lpVirusName, LPCTSTR lpFileName);
void RemoveTempFiles();
bool SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait = false);
DWORD DeleteFileForcefully(LPCTSTR lpFileName, bool bDeleteReboot);
bool ReloadWebSecDB();
bool ReloadBrowseProtDB();
bool ReadWebSecDB();
bool ReadWWizBrowseProtDB();
bool OnReloadUserList();
bool ReloadBlkSpecWebDB();
bool ReloadMngExcDB();
bool ReloadUserAccessList();
bool OnParCtrlInitialise();
bool OnParCtrlUnInitialise();
bool OnReloadBrowserSecurity();
bool OnReloadBrowserSecurityExc();
bool OnReloadBrowserSecuritySpec();
bool OnReloadBrowserSecurityVal();
bool OnParCtrlResetValue();
void GetNumberOfDaysLeft();
bool ISIncludedFileExt(LPTSTR lpszFileExt);
bool GetFileExtRecords();
bool OnParBrowserSecUninit();
bool OnCheckInternetAccess();
bool UpdateRegistrationDetails2Server();

CConCurrentQueue<ACTIVESCANFLAGS> g_onAccessQueue;
CConCurrentQueue<BLOCKAPPFLAGS>		g_BlockApplicationQueue;
DWORD								m_dwIsOffline;
CDataManager						g_objRecoverdb;
AVACTIVATIONINFO					m_ActInfo;
CScannerLoad						g_objScanner;
std::vector<std::string>			g_vecExts;
