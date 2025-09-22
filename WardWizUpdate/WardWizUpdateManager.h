/**********************************************************************************************************
Program Name          : WardWizUpdateManage.h
Description           : Update manager class.
Author Name			  : Amol Jaware
Date Of Creation      : 20 Jan 2019
Version No            : 4.1.0.1
***********************************************************************************************************/
#include <Windows.h>
#include "PipeConstants.h"
#include "WinHttpManager.h"
#include "DownloadController.h"
#include "ISpyCommunicator.h"

#pragma once

typedef  void(*DOWNLOADFUNCTIONCALLBACK)(DWORD dwFlag, DWORD dwPercent, DWORD dwTotalFileSize, DWORD dwCurrentDownloadedbytes, void * param);

class CGUIInterface :
	public IGUIInterface
{
public:
	void SetDownloadedBytes(DWORD dwTotalFileLength, DWORD dwDownloadedBytes, double dTransferRate);
	void SetPercentDownload(int nPercent);
};

class CWardWizUpdateManager
{
private:
	CGUIInterface					m_objGUI;
	DOWNLOADFUNCTIONCALLBACK		m_pfnDownloadStatusCallback;

public:

	CWardWizUpdateManager();
	CWardWizUpdateManager(DOWNLOADFUNCTIONCALLBACK pfnDownloadStatusCallback);
	~CWardWizUpdateManager();


	CWardwizLangManager		m_objwardwizLangManager;
	CISpyCommunicator		m_objCom;
	CWinHttpManager			m_objWinHttpManager;
	CWinHttpManager			m_objWinHttpManagerUpDM;
	CDownloadController*	m_pDownloadController;
	CDownloadController*	m_pDownloadController4UpdtMgr;

	//vector
	std::vector<CString>	m_vUrlLists;
	std::vector<CString>	m_vUrlZipLists;
	std::vector<CString>	m_vServiceNameVector;
	std::vector<CString>	m_vCOMDLLNameVector;

	HANDLE					hTotalFileSizeThread;
	HANDLE					m_hFile;
	HANDLE					m_hIniFile;
	HANDLE					m_hTargetFile;
	HMODULE					m_hHashDLL;
	HMODULE					m_hExtractDLL;

	CStringArray			m_csExludedList;
	CString					m_csAppFolderName;
	CString					m_csRegKeyPath;
	CString					m_csRegKeyActiveScan;
	CString					m_cswardwizEPSPublishPath;
	CString					m_csWWizProgDataEPSPublishPath;
	CString					m_csTaskID = L"0";
	TCHAR					m_szAppDataFolder[MAX_PATH];
	TCHAR					m_szModificationAppPath[512];
	TCHAR					m_szAllUserPath[256];
	DWORD					m_dwUnzippedCount;
	DWORD					m_dwReplacedCount;
	DWORD					m_dwIsUptoDateCompleted;
	DWORD					m_OSType;
	DWORD					dwEnableProtection;
	DWORD					m_ExistingBytes[2];
	DWORD					m_dwTotalFileSize;
	DWORD					m_dwFileSize;
	DWORD					m_dwPercentage;
	bool					m_bUpToDate;
	bool					m_bUpdateFailed;
	bool					m_bExtractionFailed;
	bool					m_bExtractionFailedUpdtMgr;
	bool					m_bRestartFlag;
	bool					m_bUpdateSuccess;
	bool 					m_bIsALUDeleted;
	bool					m_bIsAnyProductChanges;
	bool					m_bIsRelayClient;
	bool					m_bEnableAutoliveUpdate;
	long					m_lRetryCount = 0x00;
	long					m_lAppStopping = 0x00;
	long					m_lTotalDownloadedBytes;
	bool					m_bVistaOnward;
	int						m_iServerCount;
	int						m_iCurrentDownloadedByte;

	CString GetModuleFilePath();
	DWORD	GetTotalUpdateFilesCount(bool bSendGUI = false);
	DWORD	StartALUpdateAfter3Hrs(LPVOID lpParam);
	DWORD	UpdateManagerTaskThread(LPVOID lpParam);
	DWORD	WaitToStartTrayThread(LPVOID lpParam);
	DWORD	WaitToStartTrayThreadSEH();
	DWORD	CreateDirwardwizEPSPublish();
	DWORD	CreateDirProgDataWardWizEPS();
	DWORD	GetTotalExistingBytes();
	DWORD	GetTotalFilesSize();
	DWORD	StartALUpdtProcess4UdtMgr();
	DWORD	ParseRegLineForPostUpdate(LPTSTR lpLine);
	DWORD	SendData2CommService(int iMesssageInfo, bool bWait);
	DWORD	CheckOfflineReg();
	DWORD	CheckScanLevel();
	DWORD	UnzipUsingZipArchive(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath);
	DWORD	UnzipUsingShell32API(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath);
	DWORD	UnzipUsingShell32APISEH(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath);
	DWORD	StartALUpdateProcess();
	void	FillListControl(std::vector<CString> strVector);
	void	IsWow64();
	void	StartUpdate();
	void	StartUpdate4UpdateManger();
	void	InitializeGlobalVariblesToZero();
	void	InitializeGlobalVaribles();
	void	InitGlobalVariToZero4UpdtMgr();
	void	ClearMemoryMapObjects();
	void	CloseTrasferDataToUIThread();
	void	ReadAutoliveUpdateEnableCheck();
	void	GetNumberOfDaysLeft();
	void	AddServiceEntryInSafeMode();
	void	PerformPostUpdateRegOperations();
	void	StopGetTotalFileSizeThread();
	void	RemoveProxySetting();
	void	StopLiveUpdateThread(DWORD dwValue);
	void	SetThreadPoolStatus(bool bThreadPoolStatus);
	void	CreateDir4ProgFiles();
	void	CreateDir4ProgData();
	bool	DownLoadIniFileAndParse();
	bool	DownLoadLatestFilesFromServer(DWORD& dwStatus);
	bool	DownLoadLatestFilesFromServer4EPSUpdtMgr(DWORD& dwStatus);
	bool	ExtractAllDownLoadedFiles();
	bool	DeleteFileForcefully(LPTSTR lpszFilePath);
	bool	ReplaceDownloadedFiles();
	bool	DeleteAllRenamedFiles();
	bool	DownLoadIniFileAndParse4EPS();
	bool	MergeUpdates2ProductFolder();
	bool	StopALUpdateProcessThread();
	bool	StopALUpdateProcessThread4UpdtMgr();
	bool	ResumeALUpdateProcessThread();
	bool	SuspendALUpdateProcessThread();
	bool	CheckDestFolderAndCreate(LPTSTR lpszShortPath);
	bool	CreateDirectoryFocefully(LPTSTR lpszPath);
	bool	CreateDirectoryFocefully4EPSHierarchy(LPTSTR lpszPath);
	bool	CreateWardWizUpdatesDir(LPTSTR lpszPath);
	bool	ReplaceAllOriginalFiles();
	bool	AddEntryToALUDelIni(LPTSTR lpszFilePath, LPTSTR lpszFileName = NULL);
	bool	DeleteAllDownloadedFiles();
	bool	DeleteAllRenamedFilesSEH();
	bool	MakeDownloadUrlList();
	bool	MakeDownloadUrlList4EPSUpdtMgr();
	bool	CreateTempFolder();
	bool	IsFileMismatch();
	bool	StartDownloadFile(LPCTSTR szUrlPath);
	bool	StartDownloadZipFile(LPCTSTR szZipUrlPath);
	bool	CheckInternetConnection();
	bool	WaitForInternetConnection();
	bool	CopyDownloadedFiles2InstalledFolder();
	bool	GetApplicationDataFolder(TCHAR *szAppPath);
	bool	SendUpdateInfoToGUI(int iMessageInfo, CString csFirstParam, CString csSecondParam, DWORD dwFirstParam, DWORD dwSecondParm);
	bool	SendLiveUpdateOperation2Service(DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, bool bLiveUpdateWait);
	bool	StartDownloadingINI(LPCTSTR szUrlPath);
	bool	StartDownloadingINIForZipFiles(LPCTSTR szUrlPath);
	bool	SendMessage2UI(int iRequest, DWORD dwUITypetoSendMessge, DWORD dwMsgType = 0, DWORD dwSecondOption = 0, bool bWait = false);
	bool	CheckAnyModuleIsInProcess();
	bool	IsZipFileMisMatched(LPCTSTR szFilePath, int iItemCount);
	bool	CheckForServiceName(int iIndex);
	bool	CheckForService(TCHAR *szFilePathFromVector, CString &MatchedServiceName);
	bool	AddServiceNameIntoVector();
	bool	WriteIntoRegistry();
	bool	IsLatestFile(const CString& csFileName);
	bool	GetFileSizeAndHash(TCHAR *pFilePath, DWORD &dwFileSize, TCHAR *pFileHash);
	bool	GetFilePathFromshortPath(LPTSTR lpszFilePath, LPTSTR lpszShortPath, LPTSTR lpszFileName);
	bool	UpdateTimeDate();
	bool	GetDomainName(LPTSTR pszDomainName, DWORD dwSize);
	bool	RenameModifiedregitrypath(TCHAR *szModifiactionValue);
	bool	IsDiskSpaceAvailable();
	bool	SetEventData(HANDLE g_hUpdateFromUIEvent, LPISPY_PIPE_DATA lpSpyData);
	bool	SendUpdateFinishedData2EPSClient(DWORD dwUpdtStatus);
	bool	RemoveTemporaryData();
	bool	RemovePatchIniFile();
	bool	GetProductID(DWORD &dwProductID);
	bool	GetDWORDValueFromRegistry(HKEY hMain, LPTSTR lpszSubKey, LPTSTR lpszValuneName, DWORD &dwProductID);
	bool	CheckForCorruption(LPTSTR szZipFilePath);
	bool	DeleteFolderTree(CString csFilePath, CStringArray &csExcludedList);
	bool	GetURLPathFromFilePath(LPTSTR szZipFilePath, CString &csURLPath);
	bool	ReadProductUpdateEnableCheck();
	bool	RegisterComDLL();
	bool	AddCOMDLLNameIntoVector(CString csCOMDLLName);
	bool	SetAllALtFilesAttributeToNorml();
	bool	SetAllFileAttributeToNormal(LPCTSTR lpFolPath, LPCTSTR lpQuarntinePath, bool bRenameFile);
	bool	CheckForRenameRequired(CString csRenameFile);
	bool	DeleteAllALtFilesFromProgramData();
	bool	DeleteAllALtFilesFromProgramDataSEH(LPCTSTR lpFolPath);
	bool	StopALUpdateProcess4NoUI();

	//Added by Vilas on 24 April 2015 for second level extraction
	HRESULT UnzipToFolder(PCWSTR pszZipFile, PCWSTR pszDestFolder);
	bool SetDownloadPercentage(DWORD dwFlag, DWORD dwPercent, DWORD dwTotalFileSize, DWORD dwCurrentDownloadedbytes, void	*callbackParam);
};

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

bool	GetFileSize(LPTSTR pFilePath, DWORD &dwFileSize);
bool	DeleteFileForcefully(LPTSTR lpszFilePath);

//DWORD	StartALUpdateProcessThread(LPVOID lpParam);
DWORD	GetTotalFileSizeThread(LPVOID lpParam);
DWORD	GetPercentage(int iDownloaded, int iTotalSize);