
#include <Windows.h>
#include <tchar.h>
#include <shlobj.h>
#include "PipeConstants.h"
#include "WinHttpManager.h"
#include "ISpyCriticalSection.h"
#include "ISpyCommunicator.h"
#include "Enumprocess.h"
#include "WardwizLangManager.h"
#include "sqlite3.h"
#include "DownloadController.h"

TCHAR	szSrvName[]				= L"VibraniumALUSrv" ;
TCHAR	szSrvDisplayName[]		= L"Vibranium Updater Module" ;

typedef struct _tagScanDetails
{
	char szData[MAX_PATH];
	BYTE  byType;
}STRUCTSCANDETAILS;

SERVICE_STATUS					ServiceStatus = {0} ;
SERVICE_STATUS_HANDLE			hServiceStatus = NULL ;


void WINAPI ServiceMain(DWORD dwArgC, LPTSTR *pArgV) ;
void WINAPI ServiceCtrlHandler( DWORD dwCtrlCode ) ;
BOOL UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) ;

HANDLE hEvent_ServiceStop = NULL ;
void StopServiceByEvent() ;

void StartServiceWorkerThread(DWORD dwArgC, LPTSTR *pArgV) ;

HANDLE	hThread_SrvPrimary = NULL ;
HANDLE hThread_AutoUpdateAfter3Hrs = NULL;
HANDLE hThreadStartUpdateManagerTask = NULL;
HANDLE hThread_WaitToStartTray = NULL;
DWORD SrvPrimaryThread( LPVOID lpParam ) ;

HANDLE	hThread_StartALUpdateProcess = NULL ;
HANDLE  hTotalFileSizeThread = NULL;
DWORD StartALUpdateProcessThread( LPVOID lpParam ) ;
DWORD GetTotalFileSizeThread(LPVOID lpParam);
DWORD StartALUpdateAfter3Hrs( LPVOID lpParam ) ;
DWORD WaitToStartTrayThread(LPVOID lpParam);
DWORD WaitToStartTrayThreadSEH();


DWORD InstallService( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName) ;
DWORD UnInstallService( TCHAR *pszSrvName ) ;
DWORD StartServiceManually( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName ) ;
DWORD StopServiceManually( TCHAR *pszSrvName ) ;

DWORD QueryServiceStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus) ;
DWORD QueryServiceStartStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus) ;

DWORD SetServiceFailureAction( SC_HANDLE hService ) ;
bool RunUtililtyInSilentModeSEH();

//typedef enum _LOGGING_LEVEL
//{
//	ZEROLEVEL,
//	FIRSTLEVEL,
//	SECONDLEVEL,
//
//}LOGGINGLEVEL;

bool GetProductID( DWORD &dwProductID);
bool GetDWORDValueFromRegistry(HKEY hMain, LPTSTR lpszSubKey, LPTSTR lpszValuneName, DWORD &dwProductID);
bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = true);
bool SendData2Service(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam = NULL, LPTSTR lpszSecondParam = NULL, bool bWait = false);
bool SendData2UI(int iMessageInfo, bool bWait = false);

/**************************************************************************************************************/
// Constant Service Name
/**************************************************************************************************************/
const CString g_csServiceNameArray[10] = {
											L"VBALUSRV.EXE",
											L"VBCOMMSRV.EXE",
											L"VBFILEPROT.SYS",
											L"VBREGPROT.SYS",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
										};

/**************************************************************************************************************/
// Constant Drivers Service Names
/**************************************************************************************************************/
const CString g_csaDriverServices[10] = {
											L"VBFILEPROT.SYS",
											L"VBREGPROT.SYS",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
										};


//Auto Live Update 
CWinHttpManager				g_objWinHttpManager;
CWinHttpManager				g_objWinHttpManagerUpDM;
std::vector<CString>		g_vUrlLists;
std::vector<CString>		g_vUrlZipLists;
DWORD						g_ExistingBytes[2];
DWORD						g_dwTotalFileSize;
DWORD						g_dwFileSize;
TCHAR						g_szAppDataFolder[MAX_PATH];
HANDLE						g_hFile;
HANDLE						g_hIniFile;
HANDLE						g_hTargetFile;
int							g_iCurrentDownloadedByte;
long						g_lTotalDownloadedBytes;
DWORD						g_dwPercentage;
std::vector<CString>		g_vServiceNameVector;
HANDLE						g_hUpdateFromUIEvent;
HANDLE						g_hUpdateFromAgentEvent;
bool						g_bEnableAutoliveUpdate;
std::vector<CString>		g_vCOMDLLNameVector;
TCHAR						g_szModificationAppPath[512];
DWORD						g_OSType;
DWORD						dwEnableProtection;


BOOL	g_bIsWow64;
bool	g_bVistaOnward;
TCHAR	g_szAVPath[512];
TCHAR	g_szAllUserPath[256];
CString	g_csAppFolderName;
CString	m_csRegKeyPath;
CString	m_csRegKeyActiveScan;
CString m_cswardwizEPSPublishPath;
CString	m_csWWizProgDataEPSPublishPath;
CStringArray	m_csExludedList;

HMODULE	g_hHashDLL;
HMODULE	g_hExtractDLL;

DWORD	g_dwUnzippedCount;
DWORD	g_dwReplacedCount;

bool	g_bRequestFromUI;
bool	g_bUpToDate;
bool	g_bUpdateFailed;
bool	g_bExtractionFailed;
bool	g_bExtractionFailedUpdtMgr;
bool    g_bRestartFlag;
bool	g_bUpdateSuccess;
bool 	g_bIsALUDeleted;
DWORD	g_dwIsUptoDateCompleted;
bool	g_bIsAnyProductChanges;
long	m_lRetryCount = 0x00;
long	m_lAppStopping = 0x00;
bool	g_bIsRelayClient;
int		m_iServerCount;

typedef DWORD	(*GETFILEHASH)		(TCHAR *pFilePath, TCHAR *pFileHash);
typedef DWORD	(*UNZIPSINGLEFILE)	(TCHAR *pZipFile, TCHAR *pUnzipFile );

GETFILEHASH		GetFileHash = NULL ;
UNZIPSINGLEFILE	UnzipSingleFile = NULL;

CISpyCommunicator m_objCom(UI_SERVER, true);

void IsWow64();
bool LoadDLLs();


DWORD GetTotalUpdateFilesCount(bool bSendGUI = false);

bool  StopALUpdateProcessThread( );
bool  StopALUpdateProcessThread4UpdtMgr();
bool  ResumeALUpdateProcessThread( );
bool  SuspendALUpdateProcessThread( );

bool CheckDestFolderAndCreate( LPTSTR lpszShortPath );
bool CreateDirectoryFocefully( LPTSTR lpszPath );
bool CreateDirectoryFocefully4EPSHierarchy( LPTSTR lpszPath );
DWORD CreateDirwardwizEPSPublish();
DWORD CreateDirProgDataWardWizEPS();
bool CreateWardWizUpdatesDir( LPTSTR lpszPath );

bool ReplaceAllOriginalFiles( );
bool AddEntryToALUDelIni( LPTSTR lpszFilePath, LPTSTR lpszFileName = NULL );
bool DeleteAllDownloadedFiles( );
bool ReplaceDownloadedFiles();
bool DeleteAllRenamedFiles();
bool DeleteAllRenamedFilesSEH();

//Auto Live Update Global fuctions 
bool MakeDownloadUrlList();
bool MakeDownloadUrlList4EPSUpdtMgr();
DWORD GetTotalExistingBytes();
DWORD GetTotalFilesSize();
CString GetModuleFilePath();
bool CreateTempFolder();
bool IsFileMismatch();
bool StartDownloadFile(LPCTSTR szUrlPath);
bool StartDownloadZipFile(LPCTSTR szZipUrlPath);
bool CheckInternetConnection();
bool WaitForInternetConnection();
bool WaitForInternetConnection();
void FillListControl(std::vector<CString> strVector);
bool CopyDownloadedFiles2InstalledFolder();
bool GetApplicationDataFolder(TCHAR *szAppPath);
void StartUpdate();
bool SendUpdateInfoToGUI(int iMessageInfo , CString csFirstParam , CString csSecondParam , DWORD dwFirstParam , DWORD dwSecondParm);
bool SendLiveUpdateOperation2Service(DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, bool bLiveUpdateWait);
bool StartDownloadingINI(LPCTSTR szUrlPath);
bool StartDownloadingINIForZipFiles(LPCTSTR szUrlPath);
bool SendMessage2UI(int iRequest, DWORD dwUITypetoSendMessge, DWORD dwMsgType = 0, DWORD dwSecondOption = 0, bool bWait = false);
DWORD GetPercentage(int iDownloaded, int iTotalSize);
void ClearMemoryMapObjects();
void CloseTrasferDataToUIThread();
void CloseALUpdateProcess();
bool CheckAnyModuleIsInProcess();
bool IsZipFileMisMatched(LPCTSTR szFilePath, int iItemCount);
bool CheckForServiceName(int iIndex);
bool CheckForService(TCHAR *szFilePathFromVector , CString &MatchedServiceName);
bool AddServiceNameIntoVector();
bool ReadAutoliveUpdateEnableCheck();
bool WriteIntoRegistry();
bool IsLatestFile(const CString& csFileName);
bool GetFileSizeAndHash(TCHAR *pFilePath, DWORD &dwFileSize, TCHAR *pFileHash);
bool GetFilePathFromshortPath(LPTSTR lpszFilePath, LPTSTR lpszShortPath, LPTSTR lpszFileName );
void GetNumberOfDaysLeft();
bool UpdateTimeDate();
void AddServiceEntryInSafeMode( );
bool GetDomainName(LPTSTR pszDomainName, DWORD dwSize);
void PerformPostUpdateRegOperations( );
DWORD ParseRegLineForPostUpdate( LPTSTR lpLine );
bool GetFileSize(LPTSTR pFilePath, DWORD &dwFileSize);
bool RegisterComDLL();
bool AddCOMDLLNameIntoVector(CString csCOMDLLName);
void ReadValueFromDLLregisterSection();
void ReadValueFromDLLregisterSectionSEH();
bool RenameModifiedregitrypath(TCHAR *szModifiactionValue);
DWORD SendData2CommService(int iMesssageInfo, bool bWait);
bool SetEventData(HANDLE g_hUpdateFromUIEvent, LPISPY_PIPE_DATA lpSpyData);

//Added by Vilas on 24 April 2015 for second level extraction
DWORD UnzipUsingZipArchive(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath);

//Added by Neha To add utilities in start up menu.
bool RunUtililtyInSilentMode();

//Adding User, computer name and OS details
//Added by Vilas on 05 May 2015
void AddUserAndSystemInfoToLog();

//Varada Ikhar, To download ReleaseNotes.ini files
void SendMsgToShowReleaseNotesMsg();
bool ShowReleaseNotesMsg(TCHAR	szIniFilePath[512]);
bool ParseVersionString(int iDigits[4], CString& csVersion);
int CompareVersions(int iVersion1[4], int iVersion2[4]);

//Adding to remove all read only files from C:\ProgramData\WardWiz Antivirus
//Added by Vilas and Lalit on 15 May 2015
bool DeleteAllALtFilesFromProgramData();
bool SetAllFileAttributeToNormal(LPCTSTR lpFolPath, LPCTSTR lpQuarntinePath, bool bRenameFile = false);
bool SetAllALtFilesAttributeToNorml();
bool RenameFileFailedIn1940Version();
bool CheckForRenameRequired(CString csRenameFile);
DWORD CheckScanLevel();
DWORD CheckOfflineReg();
DWORD UnzipUsingShell32API(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath);
DWORD UnzipUsingShell32APISEH(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath);
HRESULT UnzipToFolder(PCWSTR pszZipFile, PCWSTR pszDestFolder);
bool LaunchTLBSEH();
bool LaunchTLB();
bool ReadProductUpdateEnableCheck();
bool GetURLPathFromFilePath(LPTSTR szZipFilePath, CString &csURLPath);
bool HandleDriverReplaceCaseOnWin10();
bool CheckForCorruption(LPTSTR szZipFilePath);
void LaunchUpdateExe();

CWardwizLangManager			m_objwardwizLangManager;
CDownloadController*		m_pDownloadController;
CDownloadController*		m_pDownloadController4UpdtMgr;

typedef std::vector <STRUCTSCANDETAILS> SCANDETAILSMAP;
