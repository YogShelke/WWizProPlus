/****************************************************
*  Program Name: CWardWizScan.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28th March 2016
*  Version No: 2.0.0.1
****************************************************/
#pragma once

#include "ScannerContants.h"
#include "WardWizUI.h"
#include "iSpyMemMapClient.h"
#include "iTinEmailContants.h"
#include "ISpyDBManipulation.h"
#include "ExcludeFilesFolders.h"
#include <mmsystem.h>
#include "WardWizDatabaseInterface.h"

#pragma comment(lib, "winmm.lib")

class CWardWizScan :	sciter::event_handler,
						sciter::behavior_factory
{
	HELEMENT self;
public:
	CWardWizScan();
	~CWardWizScan();

	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}

	BEGIN_FUNCTION_MAP
		//Custom Scan related functions
		FUNCTION_1("OnClickShutDwonScan",On_ClickShutDwonScan)
		FUNCTION_5("OnStartCustomScan", On_StartCustomScan) // On_StartFullScan()
		FUNCTION_1("OnPauseCustomScan", On_PauseCustomScan) // On_PasueResumeFullScan()	
		FUNCTION_1("OnResumeCustomScan", On_ResumeCustomScan) // On_PasueResumeFullScan()	
		FUNCTION_1("OnStopCustomScan", On_StopCustomScan) // On_StopFullScan()	
		FUNCTION_1("LoadCutomScanINI", LoadCutomScanINI) // On_StopFullScan()	
		FUNCTION_1("AddDeleteEditCustomScanEntryToINI", AddDeleteEditCustomScanEntryToINI) // On_StopFullScan()
		FUNCTION_1("CheckIsNetworkPath",CheckForNetworkPath)
		FUNCTION_0("OnSetTimer",OnSetTimer)
		FUNCTION_0("CallContinueCustomScan", On_CallContinueCustomScan)
		//Boot Time Scan related functions
		FUNCTION_2("OnClickSetScanID", On_ClickSetScanID)
		FUNCTION_0("OnClickGetScanID", On_ClickGetScanID)
		FUNCTION_1("ScanFinishedCall", On_ScanFinishedCall)
		FUNCTION_1("GetCustomScanStatus", GetCustomScanStatus)

	END_FUNCTION_MAP

	//Custom Scan related functions
	json::value GetCustomScanStatus(SCITER_VALUE svIsScanRunning);
	json::value On_ScanFinishedCall(SCITER_VALUE svGetScanFinishedFlag);
	json::value On_ClickShutDwonScan(SCITER_VALUE svIsShutDownPC);
	json::value On_StartCustomScan(SCITER_VALUE svArrCustomScanSelectedEntries, SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB, SCITER_VALUE svFunNotificationMessageCB);
	json::value On_PauseCustomScan(SCITER_VALUE svFunPauseResumeFunCB);
	json::value On_ResumeCustomScan(SCITER_VALUE svFunPauseResumeFunCB);
	json::value On_StopCustomScan(SCITER_VALUE svbIsManualStop);
	json::value On_ClickCustomScanCleanBtn(SCITER_VALUE svArrCleanEntries, SCITER_VALUE svFunSetVirusUpdateEntryCB);
	json::value LoadCutomScanINI(SCITER_VALUE svFillCustomScanArrayCB);
	json::value AddDeleteEditCustomScanEntryToINI(SCITER_VALUE svArrCustomScanEntries);
	json::value CheckForNetworkPath(SCITER_VALUE svFilePath);
	json::value OnSetTimer();
	json::value On_CallContinueCustomScan();
	//Boot Time Scan related functions
	json::value On_ClickSetScanID(SCITER_VALUE svScanTypeID, SCITER_VALUE svResult);
	json::value On_ClickGetScanID();
public:
	
	bool GetScanningPaths(CStringArray &csaReturn);
	bool Check4DBFiles();
	bool StartScanUsingService(CStringArray &csaAllScanPaths);
	bool GetAllDrivesList(CStringArray &csaReturn);
	bool Check4ValidDBFiles(CString csDBFilePath);
	bool MakeFullTokenizedScanPath(CStringArray &csaAllScanPaths, LPTSTR szScanPath);
	bool PauseScan();
	bool ResumeScan();
	bool SendRequestCommon(int iRequest);
	bool ScanFinished();
	bool SaveLocalDatabase();
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait);
	bool ShutDownScanning();
	bool OnClickCleanButton(SCITER_VALUE svArrayCleanEntries, SCITER_VALUE setVirusUpdateEntryFunCB);
	bool SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry, DWORD dwType, bool bWait = false, bool bReconnect = false);
	bool SendFile4RepairUsingService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect);
	bool SendFile4RepairUsingService(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait = false, bool bReconnect = false);
	bool VirusFoundEntries(LPISPY_PIPE_DATA lpSpyData);
	bool SaveDBEntries();
	bool ReadUISettingFromRegistry();
	bool GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt);
	bool IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize);
	bool CheckFileIsInRepairRebootIni(CString csFilePath);
	bool GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath);
	bool CheckFileIsInRecoverIni(CString csFilePath);
	bool CheckFileIsInDeleteIni(CString csQurFilePaths);
	bool TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName);
	bool TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName);
	bool ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan = false);
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait);
	bool SendData2EPSClient(LPCTSTR szFilePath, LPCTSTR szVirusName, CString csTaskID, DWORD dwActionTaken);
	//bool LoadRequiredDllForScan();

	void StartScanning();
	void CallUISetStatusfunction(LPTSTR lpszPath);
	void CallUISetStatusfunctionSEH(LPTSTR lpszPath);
	void CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID);
	void CallUISetPauseStatusfunction(CString csData);
	void CallUISetScanFinishedStatus(CString csData);
	void CallUISetFileCountfunction(CString csData, CString csCurrentFileCount);
	void StopScan();
	void QuaratineEntries();
	void GetModuleCount();
	void LoadPSAPILibrary();
	void EnumerateProcesses();
	void EnumFolderForScanning(LPCTSTR pstr);
	void ScanForSingleFile(CString csFilePath);
	void AddEntriesInReportsDB(SCANTYPE eScanType, CString csThreatName, CString csFilePath, CString csAction);
	void EnumFolder(LPCTSTR pstr);
	void SetLastScanDateTime();
	
	DWORD CheckForDeleteFileINIEntries();
	DWORD CheckForRepairFileINIEntries();

	CString GetQuarantineFolderPath();

	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
	INT64 InsertSQLiteData(const char* szQuery);
	bool CheckFileOrFolderOnRootPath(CString csFilePath);
	void OnTimerScan();
	void CallFinishScanFunction();
	bool CheckIsScanShutdown();
public:
	bool					m_bQuickScan;
	bool					m_bFullScan;
	bool					m_bCustomscan;
	bool					m_bRedFlag;
	bool					m_bScnAborted;
	bool					m_bScanStartedStatusOnGUI;
	bool					m_bIsMemScan;
	bool					m_bStop;
	bool					m_bIsManualStop;
	bool					m_bIsCleaning;
	bool					m_bEnableSound;
	bool					m_bManualStop;
	bool					m_ScanCount;
	bool					m_bRescan;
	bool					m_bIsPathExist;
	bool					m_bIsShutDownPC;
	bool					m_bIsManualStopScan;
	bool					m_bIsMultiCScanFinish;
	SCANTYPE				m_eScanType;
	SCANTYPE				m_eCurrentSelectedScanType;
	CISpyCommunicator		m_objCom;

	CStringArray			m_csaAllScanPaths;

	DWORD					m_dwTotalFileCount;
	DWORD					m_iMemScanTotalFileCount;
	DWORD					m_FileScanned;
	DWORD					m_dwVirusFoundCount;
	DWORD					m_dwVirusCleanedCount;
	INT64					m_iScanSessionId;

	HANDLE					m_hThreadVirEntries;
	HANDLE					m_hThreadStatusEntry;

	CTime					m_tsScanStartTime;
	CTime					m_tsScanEndTime;
	CTime					m_tsScanPauseResumeTime;
	CTimeSpan				m_tsScanPauseResumeElapsedTime;

	iSpyServerMemMap_Client m_objIPCClient;
	iSpyServerMemMap_Client m_objIPCClientVirusFound;

	SCITER_VALUE m_svGetScanFinishedFlag;
	SCITER_VALUE m_svAddVirusFoundEntryCB;
	SCITER_VALUE m_svSetPauseStatusCB;
	SCITER_VALUE m_svSetScanFinishedStatusCB;
	SCITER_VALUE m_svSetScanFileCountStatusCB;
	SCITER_VALUE m_svSetVirusUpdateStatusCB;
	SCITER_VALUE m_svArrayCleanEntries;
	SCITER_VALUE m_svArrCustomScanSelectedEntries;
	SCITER_VALUE m_svFunNotificationMessageCB;
	SCITER_VALUE m_svIsScanRunning;

	HANDLE					m_hQuarantineThread;
	HANDLE					m_hThread_ScanCount;
	HANDLE					m_hWardWizAVThread;
	HMODULE					m_hScanDllModule;
	HMODULE					m_hPsApiDLL; 
	ENUMPROCESSMODULESEX	EnumProcessModulesWWizEx;
	CExcludeFilesFolders	m_objExcludeFilesFolders;
	CISpyCommunicator		m_objScanCom;
	//SCANFILE				m_lpScanFileProc;
	CString					m_csPreviousPath;
	CStringList				m_csaModuleList;
	int						m_iTotalFileCount;
	int						m_iThreatsFoundCount;
	CWardWizSQLiteDatabase  m_objSqlDb;
	CString					m_csCurrentFilePath;
	virtual bool on_timer(HELEMENT he, UINT_PTR extTimerId);
	virtual bool on_timer(HELEMENT he);
	bool on_timerSEH(HELEMENT he);
	bool AddBootScannerEntry(LPTSTR pKeyName, LPTSTR pValueName, LPTSTR pNewValue);
	virtual bool handle_timer(HELEMENT he, TIMER_PARAMS& params)
	{
		__try
		{
			if (params.timerId)
				return on_timer(he, params.timerId);
			return on_timer(he);
		}
		__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
		{
			AddLogEntry(L"### Exception in CWardwizScan::handle_timer", 0, 0, true, SECONDLEVEL);
		}
		return true;
	}
};

