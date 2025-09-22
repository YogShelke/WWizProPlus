/**********************************************************************************************************
Program Name          : WardWizQuickScan.cpp
Description           : This class contains logic for Quick Scan
Author Name			  : Jeena Mariam Saji
Date Of Creation      : 25 Jan 2019
Version No            : 4.1.0.0
Special Logic Used    : This code is created using logic to run multiple scans at the same time.
Modification Log      :
1. Jeena Mariam        Created WardWizQuickScan class   25th Jan 2019
***********************************************************************************************************/
#include "stdafx.h"
#include <Psapi.h>
#include "WardWizQuickScan.h"
#include "WrdwizEncDecManager.h"
#include "WardWizUIDlg.h"
#include "PipeConstants.h"
#include "Enumprocess.h"
#include "WrdWizSystemInfo.h"

#define		SETFILEPATH_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 1)
#define		SETFILECOUNT_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 2)
#define		SETSCANFINISHED_EVENT_CODE		(FIRST_APPLICATION_EVENT_CODE + 3)
#define		RESTART_YES						L"YES"
#define		RESTART_NO						L"NO"
#define		WRDWIZBOOTSCN					L"VBBOOTSCN.EXE"
#define		WRDWIZEXCLUDECLONEDB			L"VBEXCLUDECLONE.DB"

DWORD WINAPI EnumerateThreadQuick(LPVOID lpvThreadParam);
DWORD WINAPI GetTotalScanFilesCountQuick(LPVOID lpParam);
DWORD WINAPI QuarantineThreadQuick(LPVOID lpParam);
UINT PlayScanFinishedThreadQuick(LPVOID lpThis);
UINT PlayThreatsFoundThreadQuick(LPVOID lpThis);

BOOL					g_bIsScanningQuick;

CString					g_csPreviousFileQuick = L"";
CString					g_csPreviousFilePathQuick = L"";
CString					g_csPreviousStatusQuick = L"";
CString					g_csPreviousVirusFoundPathQuick = L"";

/***************************************************************************************************
*  Function Name  : Constructor
*  Description    : CWardWizQuickScan Constructor
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
CWardWizQuickScan::CWardWizQuickScan() : behavior_factory("WardWizQuickScan")
, m_objIPCClient(FILESYSTEMSCAN)
, m_objIPCClientVirusFound(VIRUSFOUNDENTRY)
, m_objCom(SERVICE_SERVER, true)
, m_hQuarantineThread(NULL)
, m_bStop(false)
, m_bIsManualStop(false)
, m_bEnableSound(false)
, m_bManualStop(false)
, m_iThreatsFoundCount(0)
, m_objScanCom(SERVICE_SERVER, true, 2)
, m_hScanDllModule(NULL)
, m_bRescan(false)
, m_hThread_ScanCount(NULL)
, m_hWardWizAVThread(NULL)
, m_csPreviousPath(L"")
, m_bIsPathExist(false)
, m_bIsShutDownPC(false)
, m_bIsManualStopScan(false)
, m_bIsMultiQScanFinish(false)
, m_objSqlDb(STRDATABASEFILE)
{
	m_objIPCClient.OpenServerMemoryMappedFile();
	m_objIPCClientVirusFound.OpenServerMemoryMappedFile();
	EnumProcessModulesWWizEx = NULL;
	m_hPsApiDLL = NULL;
	LoadPSAPILibrary();
	m_hScanDllModule = NULL;
}

/***************************************************************************************************
*  Function Name  : Destructor
*  Description    : CWardWizQuickScan Destructor
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
CWardWizQuickScan::~CWardWizQuickScan()
{
	if (m_hPsApiDLL != NULL)
	{
		FreeLibrary(m_hPsApiDLL);
		m_hPsApiDLL = NULL;
	}
}

/***************************************************************************************************
*  Function Name  : On_StartQuickScan
*  Description    : Accepts the request from UI and starts the Quick scan
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
json::value CWardWizQuickScan::On_StartQuickScan(SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB, SCITER_VALUE svFunNotificationMessageCB)
{
	try
	{
		theApp.m_bIsScanning = true;
		theApp.m_bQuickScan = true;
		m_bIsManualStop = false;
		m_bStop = false;
		m_bIsManualStopScan = false;
		m_bIsMultiQScanFinish = false;
		m_eCurrentSelectedScanType = QUICKSCAN;
		m_svAddVirusFoundEntryCB = svFunAddVirusFoundEntryCB;
		m_svSetScanFinishedStatusCB = svFunSetScanFinishedStatusCB;
		m_svFunNotificationMessageCB = svFunNotificationMessageCB;
		StartScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::On_StartQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_PauseQuickScan
*  Description    : Accepts the request from UI and Pause Quick scan
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
json::value CWardWizQuickScan::On_PauseQuickScan(SCITER_VALUE svFunPauseResumeFunCB)
{
	try
	{
		theApp.m_bIsQuickScanUIReceptive = false;
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS_QUICK);
		}

		m_objSqlDb.Close();

		if (theApp.m_bQuickScan)
		{
			m_svSetPauseStatusCB = svFunPauseResumeFunCB;
			PauseScan();
			theApp.m_bQuickScan = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::On_PauseQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_ResumeQuickScan
*  Description    : Accepts the request from UI and Resumes quick scan
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
json::value CWardWizQuickScan::On_ResumeQuickScan(SCITER_VALUE svFunPauseResumeFunCB)
{
	try
	{
		theApp.m_bIsQuickScanUIReceptive = false;
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS_QUICK, 1000, NULL);
		}

		m_objSqlDb.Close();

		if (!theApp.m_bQuickScan)
		{
			m_svSetPauseStatusCB = svFunPauseResumeFunCB;
			ResumeScan();
			theApp.m_bQuickScan = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::On_ResumeQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_StopQuickScan
*  Description    : Accepts the request from UI and stops quick scan
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
json::value CWardWizQuickScan::On_StopQuickScan(SCITER_VALUE svbIsManualStop)
{
	try
	{
		m_bIsManualStop = svbIsManualStop.get(false);
		m_bIsManualStopScan = true;
		StopScan();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::On_StopQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ClickShutDwonScan
*  Description    : Accepts the request from UI and shut wodn after scan
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
json::value CWardWizQuickScan::On_ClickShutDwonScan(SCITER_VALUE svIsShutDownPC)
{
	try
	{
		m_bIsShutDownPC = svIsShutDownPC.get(false);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::On_ClickShutDwonScan", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	InsertSQLiteData
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
INT64 CWardWizQuickScan::InsertSQLiteData(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable entered", 0, 0, true, ZEROLEVEL);

	try
	{
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", theApp.GetModuleFilePath());
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		m_objSqlDb.SetDataBaseFilePath(dbPath.m_psz);
		m_objSqlDb.Open();

		int		iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(500);
		INT64	iIdentityId = m_objSqlDb.GetLastRowId();

		m_objSqlDb.Close();
		return iIdentityId;
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CVibraniumQuickScan::InsertSQLiteData.", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	StartScanning
*  Description    :	To start Full scan
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
void CWardWizQuickScan::StartScanning()
{
	try
	{
		m_bRedFlag = false;
		m_bIsPathExist = false;
		UINT uRetVal = 0;
		m_bQuickScan = false;
		m_bFullScan = false;
		m_bCustomscan = false;
		m_bManualStop = false;
		m_iThreatsFoundCount = 0;
		m_ScanCount = false;
		m_FileScanned = 0;
		m_csPreviousPath = L"";
		m_eScanType = m_eCurrentSelectedScanType;
		m_bScnAborted = false;
		if (!theApp.ShowRestartMsgOnProductUpdate())
		{
			return;
		}

		ReadUISettingFromRegistry();
		m_bScanStartedStatusOnGUI = true;

		if (!GetScanningPaths(m_csaAllScanPaths))
		{
			return;
		}

		//Check for DB files at installation path
		if (!Check4DBFiles())
		{
			m_bQuickScan = false;
			m_bFullScan = false;
			m_bCustomscan = false;
			return;
		}
		m_iTotalFileCount = 0;
		m_dwTotalFileCount = 0;
		m_iMemScanTotalFileCount = 0;
		m_dwVirusFoundCount = 0;
		m_dwVirusCleanedCount = 0;

		//Load Exclue DB
		m_objExcludeFilesFolders.LoadExcludeDB();

		if (m_bQuickScan == true)
		{
			m_eScanType = QUICKSCAN;
			GetModuleCount();
		}

		m_hThread_ScanCount = ::CreateThread(NULL, 0, GetTotalScanFilesCountQuick, (LPVOID) this, 0, NULL);
		Sleep(500);

		// Get entries from registry so that, those can be included in query..
		DWORD dwQuarantineOpt;
		DWORD dwHeuristicOpt;
		bool  bHeuristicOpt = false;

		GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);

		if (dwHeuristicOpt == 1)
			bHeuristicOpt = true;


		CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");

		csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),%d,%d,%d,%d,%d );"), m_eScanType, m_FileScanned, m_dwVirusFoundCount, dwQuarantineOpt, bHeuristicOpt, m_dwVirusCleanedCount);

		CT2A ascii(csInsertQuery, CP_UTF8);


		m_iScanSessionId = InsertSQLiteData(ascii.m_psz);
		DWORD m_dwThreadId = 0;
		m_hWardWizAVThread = ::CreateThread(NULL, 0, EnumerateThreadQuick, (LPVOID) this, 0, &m_dwThreadId);
		Sleep(500);
		CString csScanStarted = L"";
		switch (m_eScanType)
		{
		case QUICKSCAN:
			csScanStarted = L">>> Quick scanning started...";
			break;
		case FULLSCAN:
			csScanStarted = L">>> Full scanning started...";
			break;
		case CUSTOMSCAN:
			csScanStarted = L">>> Custom scanning started...";
			break;
		default:
			csScanStarted = L">>> Scanning started...";
			break;
		}

		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS_QUICK, 1000, NULL);  // call OnTimer function
		}
		theApp.m_bIsQuickScanUIReceptive = true;
		AddLogEntry(csScanStarted, 0, 0, true, SECONDLEVEL);
		m_tsScanStartTime = CTime::GetCurrentTime();
		m_tsScanPauseResumeElapsedTime -= m_tsScanPauseResumeElapsedTime;
		sciter::dom::element(self).start_timer(500);
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CVibraniumQuickScan::StartScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetScanningPaths
*  Description    :	Get scan path according to scanning types.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::GetScanningPaths(CStringArray &csaReturn)
{
	try
	{
		CString cScanPath;
		TCHAR	szProgFilesDir86[MAX_PATH] = { 0 };
		TCHAR	szProgFilesDir[MAX_PATH] = { 0 };
		WardWizSystemInfo	objSysInfo;

		switch (m_eScanType)
		{
		case QUICKSCAN:
			m_bQuickScan = true;
			csaReturn.RemoveAll();
			//csaReturn.Add(L"QUICKSCAN");
			if (objSysInfo.GetOSType())
			{
				GetEnvironmentVariable(TEXT("PROGRAMFILES(X86)"), szProgFilesDir86, 255);
				csaReturn.Add(szProgFilesDir86);
			}
			else
			{
				GetEnvironmentVariable(TEXT("ProgramFiles"), szProgFilesDir, 255);
				csaReturn.Add(szProgFilesDir);
			}
			break;
		case FULLSCAN:
			m_bFullScan = true;
			csaReturn.RemoveAll();
			if (!GetAllDrivesList(csaReturn))
			{
				return false;
			}
			break;
		case CUSTOMSCAN:
		{
			m_bCustomscan = true;
			bool bIsArray = false;
			m_svArrCustomScanSelectedEntries.isolate();
			bIsArray = m_svArrCustomScanSelectedEntries.is_array();
			if (!bIsArray)
			{
				return false;
			}
			csaReturn.RemoveAll();
			for (unsigned iCurrentValue = 0, count = m_svArrCustomScanSelectedEntries.length(); iCurrentValue < count; iCurrentValue++)
			{
				const SCITER_VALUE EachEntry = m_svArrCustomScanSelectedEntries[iCurrentValue];
				const std::wstring chFilePah = EachEntry[L"FilePath"].get(L"");
				bool bValue = EachEntry[L"selected"].get(false);
				if (bValue)
				{
					csaReturn.Add(chFilePah.c_str());
				}
			}
		}
		break;
		case USBSCAN:
		case USBDETECT:
			//return OnGetSelection();
			break;
		}
		if (csaReturn.GetCount() > 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::GetScanningPaths", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************
*  Function Name	 :	Check4DBFiles
*  Description		 :	Checks for signature db are exist or not
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
**********************************************************************************************************/
bool CWardWizQuickScan::Check4DBFiles()
{
	DWORD dwDBVersionLength = 0;
	TCHAR szModulePath[MAX_PATH] = { 0 };
	CWrdwizEncDecManager objWrdwizEncDecMgr;

	if (!GetModulePath(szModulePath, MAX_PATH))
	{
		return false;
	}
	CString csDBFilesFolderPath = szModulePath;
	CString csWRDDBFilesFolderPath = szModulePath;
	csDBFilesFolderPath += L"\\DB";
	csWRDDBFilesFolderPath += L"\\VBDB";
	if (!PathFileExists(csDBFilesFolderPath) && !PathFileExists(csWRDDBFilesFolderPath))
	{
		return false;
	}
	else if (!Check4ValidDBFiles(csWRDDBFilesFolderPath))
	{
		return false;
	}
	else
	{
		CStringArray csaDBFiles;
		if (theApp.m_eScanLevel == WARDWIZSCANNER)
		{
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\VIBRANIUMAV1.DB");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\VIBRANIUMAVR.DB");
		}
		else
		{
			csaDBFiles.Add(csDBFilesFolderPath + L"\\MAIN.CVD");
			csaDBFiles.Add(csDBFilesFolderPath + L"\\DAILY.CLD");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\VIBRANIUMAV1.DB");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\VIBRANIUMAVR.DB");
		}

		for (int iIndex = 0; iIndex < csaDBFiles.GetCount(); iIndex++)
		{
			if (!PathFileExists(csaDBFiles.GetAt(iIndex)))
			{
				return false;
			}
		}
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	StartScanUsingService
*  Description    :	Used named pipe to give signal to service to start scanning .
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::StartScanUsingService(CStringArray &csaAllScanPaths)
{
	try
	{
		if (csaAllScanPaths.GetCount() == 0)
		{
			return false;
		}

		TCHAR szScanPath[1000] = { 0 };
		if (!MakeFullTokenizedScanPath(csaAllScanPaths, szScanPath))
		{
			return false;
		}

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = START_SCAN;
		szPipeData.dwValue = static_cast<DWORD>(m_eScanType);
		wcscpy_s(szPipeData.szFirstParam, szScanPath);

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumQuickScan::StartScanUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CVibraniumQuickScan::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		else
		{
			g_bIsScanningQuick = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetAllDrivesList
*  Description    :	Makes list of drives present on a system.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::GetAllDrivesList(CStringArray &csaReturn)
{
	csaReturn.RemoveAll();
	bool bReturn = false;
	CString csDrive;
	int iCount = 0;

	for (char chDrive = 'A'; chDrive <= 'Z'; chDrive++)
	{
		csDrive.Format(L"%c:", chDrive);
		if (PathFileExists(csDrive))
		{
			csaReturn.Add(csDrive);
			bReturn = true;
		}
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	Check4ValidDBFiles
*  Description    :	This function will check for valid signature and valid Version length in DB files
if any mismatch found, will return false otherwise true.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::Check4ValidDBFiles(CString csDBFolderPath)
{
	try
	{
		CString csFilePath;
		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAV1);
		DWORD dwDBVersionLength = 0;
		DWORD dwDBMajorVersion = 0;
		CWrdwizEncDecManager objWrdwizEncDecMgr;
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(csFilePath, dwDBVersionLength, dwDBMajorVersion))
		{
			AddLogEntry(L"### Invalid DB found (or) may corrupted, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		//DB Version lenfth should be in between 7 and 19
		//Eg: 1.0.0.0 to 9999.9999.9999.9999
		if (!(dwDBVersionLength >= 7 && dwDBVersionLength <= 19))
		{
			AddLogEntry(L"### Invalid DB Version length, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAVR);
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(csFilePath, dwDBVersionLength, dwDBMajorVersion))
		{
			AddLogEntry(L"### Invalid DB found (or) may corrupted, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		//DB Version lenfth should be in between 7 and 19
		//Eg: 1.0.0.0 to 9999.9999.9999.9999
		if (!(dwDBVersionLength >= 7 && dwDBVersionLength <= 19))
		{
			AddLogEntry(L"### Invalid DB Version length, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::Check4ValidDBFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	MakeFullTokenizedScanPath
*  Description    :	Toknization of scan path
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::MakeFullTokenizedScanPath(CStringArray &csaAllScanPaths, LPTSTR szScanPath)
{
	try
	{
		if (!szScanPath)
		{
			return false;
		}

		if (csaAllScanPaths.GetCount() == 0)
		{
			return false;
		}

		_tcscpy(szScanPath, csaAllScanPaths.GetAt(0));
		for (int iIndex = 1; iIndex < csaAllScanPaths.GetCount(); iIndex++)
		{
			_tcscat(szScanPath, L"#");
			_tcscat(szScanPath, csaAllScanPaths.GetAt(iIndex));
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::MakeFullTokenizedScanPath", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : PauseScan
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*************************************************************************************************/
bool CWardWizQuickScan::PauseScan()
{
	try
	{
		CString csPauseResumeBtnText = L"";

		if (m_hThread_ScanCount != NULL)
		{
			::SuspendThread(m_hThread_ScanCount);
		}

		if (m_hWardWizAVThread != NULL)
		{
			::SuspendThread(m_hWardWizAVThread);
			CallUISetPauseStatusfunction(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_PAUSE"));
			AddLogEntry(L">>> Scanning Paused..", 0, 0, true, FIRSTLEVEL);
		}
		else
		{
			AddLogEntry(L"### Failed to pause scan as SuspendThread request failed.", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::PauseScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SendRequestCommon
*  Description    :	Send a pause,stop,resume scanning request to comm service.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::SendRequestCommon(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to SERVICE_SERVER in CVibraniumQuickScan::SendRequestCommon", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : ResumeScan
*  Description    : Resume scanning if user click on stop/close button and click to 'No' for stop confirmation box.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*************************************************************************************************/
bool CWardWizQuickScan::ResumeScan()
{
	try
	{
		if (m_hThread_ScanCount != NULL)
		{
			::ResumeThread(m_hThread_ScanCount);
		}

		if (m_hWardWizAVThread != NULL)
		{
			::ResumeThread(m_hWardWizAVThread);
			CallUISetPauseStatusfunction(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_RESUME"));
		}
		else
		{
			AddLogEntry(L"### Failed to pause scan as Send PAUSE_SCAN request failed.", 0, 0, true, SECONDLEVEL);
			return false;
		}
		sciter::dom::element(self).start_timer(500);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::ResumeScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : callUISetStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
void CWardWizQuickScan::CallUISetStatusfunction(LPTSTR lpszPath)
{
	__try
	{
		CallUISetStatusfunctionSEH(lpszPath);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CallUISetStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : CallUISetStatusfunctionSEH
*  Description    : Calls call_function to invoke ant UI function
Note: No need to add exception handling because
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
void CWardWizQuickScan::CallUISetStatusfunctionSEH(LPTSTR lpszPath)
{
	if (!lpszPath)
	{
		return;
	}

	CString csPath(lpszPath);
	TCHAR					m_szActiveScanFilePath[1024];
	memset(m_szActiveScanFilePath, 0, sizeof(m_szActiveScanFilePath));
	int iCount = csPath.ReverseFind('\\');
	CString csFileName = csPath.Right(csPath.GetLength() - (iCount + 1));
	CString csFolderPath;
	csFolderPath = csPath.Left(iCount);

	GetShortPathName(csFolderPath, m_szActiveScanFilePath, 60);
	CString csTempFileName = csFileName;
	iCount = csTempFileName.ReverseFind('.');
	CString csFileExt = csTempFileName.Right(csTempFileName.GetLength() - (iCount));
	csTempFileName = csTempFileName.Left(iCount);
	if (csTempFileName.GetLength() > 10)
	{
		csTempFileName = csTempFileName.Left(10);
		csFileName.Format(L"%s~%s", csTempFileName, csFileExt);
	}

	if (_tcslen(m_szActiveScanFilePath) == 0 || csFileName.GetLength() == 0)
		return;

	CString csFinalFilePath;
	csFinalFilePath.Format(L"%s\\%s", m_szActiveScanFilePath, csFileName);

	sciter::dom::element ela = self;
	BEHAVIOR_EVENT_PARAMS params;
	params.cmd = SETFILEPATH_EVENT_CODE;
	params.he = params.heTarget = ela;
	params.data = SCITER_STRING(csFinalFilePath.Trim());
	ela.fire_event(params, true);
}

/***************************************************************************************************
*  Function Name  : CallUISetVirusFoundEntryfunction
*  Description    : used to insert threat found entries
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
void CWardWizQuickScan::CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID)
{
	try
	{
		m_svAddVirusFoundEntryCB.call(SCITER_STRING(csVirusName), SCITER_STRING(csFilePath), SCITER_STRING(csActionTaken), SCITER_STRING(SpyID));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CallUISetVirusFoundEntryfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetPauseStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
void CWardWizQuickScan::CallUISetPauseStatusfunction(CString csData)
{
	try
	{
		m_svSetPauseStatusCB.call(SCITER_STRING(csData));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CallUISetPauseStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : callUISetScanFinishedStatus
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
void CWardWizQuickScan::CallUISetScanFinishedStatus(CString csData)
{
	try
	{
		if (theApp.m_bIsQuickScanUIReceptive)
		{
			CWnd *pwnd = theApp.m_pMainWnd;
			if (pwnd != NULL)
			{
				KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS_QUICK);
			}

			if (!theApp.m_bIsQScanPageSwitched)
			{
				m_svSetScanFinishedStatusCB.call(SCITER_STRING(csData));
			}
			else
			{
				CallFinishScanFunction();
			}
			sciter::dom::element(self).stop_timer();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CallUISetScanFinishedStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
void CWardWizQuickScan::CallUISetFileCountfunction(CString csTotalFileCount, CString csCurrentFileCount)
{
	try
	{
		sciter::value map;
		CString csVirusCount;
		csVirusCount.Format(L"%d", m_dwVirusFoundCount);
		map.set_item("one", sciter::string(csCurrentFileCount));
		map.set_item("two", sciter::string(csTotalFileCount));
		map.set_item("three", sciter::string(csVirusCount));
		//Send here event
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETFILECOUNT_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::callUIfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ScanFinished
*  Description    : Calls ScanFinished when scan is finished
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
****************************************************************************************************/
bool CWardWizQuickScan::ScanFinished()
{
	CString csCompleteScanning;
	CString csFileScanCount;
	CString csMsgNoFileExist(L"");
	CString csCurrentFileCount;
	m_bIsMultiQScanFinish = true;
	if (theApp.m_bIsQuickScanUIReceptive)
	{
		OnTimerScan();
	}
	if (m_eScanType == CUSTOMSCAN && m_bScnAborted)
	{
		csCurrentFileCount.Format(L"%d", m_FileScanned);
		CallUISetFileCountfunction(L"", csCurrentFileCount);
	}

	//OutputDebugString(L">>> m_hThreadStatusEntry stopped");
	Sleep(500);

	if (!m_bIsPathExist)
	{
		csMsgNoFileExist.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_INVALID_SELECTION"));
		CallNotificationMessage(1, (SCITER_STRING)csMsgNoFileExist);
	}
	if (!m_bIsManualStop)
	{
		if (m_bEnableSound)
		{
			AfxBeginThread(PlayScanFinishedThreadQuick, NULL);
		}
		AddEntriesInReportsDB(m_eScanType, L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L" IDS_MSG_SCANNING_COMPLETED"));
		// Add entries into Database..
		CString csInsertQuery = 0;
		csInsertQuery.Format(_T("UPDATE Wardwiz_ScanSessionDetails SET db_ScanSessionEndDate = Date('now'),db_ScanSessionEndTime = Datetime('now', 'localtime'),db_TotalFilesScanned = %d,db_TotalThreatsFound = %d, db_TotalThreatsCleaned = %d WHERE db_ScanSessionID = %I64d;"), m_FileScanned, m_dwVirusFoundCount, m_dwVirusCleanedCount, m_iScanSessionId);
		CT2A ascii(csInsertQuery, CP_UTF8);
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", theApp.GetModuleFilePath());
		if (!PathFileExists(csWardWizReportsPath))
		{
			m_objSqlDb.Open();
			m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			m_objSqlDb.Close();
		}
		InsertSQLiteData(ascii.m_psz);
	}
	else
	{
		AddEntriesInReportsDB(m_eScanType, L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L" IDS_STATUS_SCAN_ABORTED"));
	}
	CallUISetScanFinishedStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_INFECTEDFILES"));
	SaveLocalDatabase();
	m_tsScanEndTime = CTime::GetCurrentTime();
	CString csTime = m_tsScanEndTime.Format(_T("%H:%M:%S"));
	AddLogEntry(_T(">>> End Scan Time: %s"), csTime, 0, true, FIRSTLEVEL);
	SetLastScanDateTime();
	CString csElapsedTime;
	csElapsedTime.Format(L"%s%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ELAPSEDTIME"), csTime);
	AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	CString cstypeofscan = L"";
	DWORD ScanType;
	switch (m_eScanType)
	{
	case QUICKSCAN:	cstypeofscan = L"Quick";
		ScanType = 2;
		break;

	case FULLSCAN:	cstypeofscan = L"Full";
		ScanType = 0;
		break;

	case CUSTOMSCAN:cstypeofscan = L"Custom";
		ScanType = 1;
		break;

	default:		cstypeofscan = L"";
		break;
	}
	if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath, L"ScanType", REG_DWORD, ScanType, true))
	{
		AddLogEntry(L"### Failed in Setting Registry CVibraniumQuickScan::ScanFinished", 0, 0, true, SECONDLEVEL);
	}
	if (!m_bIsManualStop)
	{
		csCompleteScanning.Format(L">>> %s %s.", cstypeofscan, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_INFECTEDFILES"));
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		csFileScanCount.Format(L"%s%d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED"), m_FileScanned);
		csCompleteScanning.Format(L">>> %s = %d, %s = %d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_FileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_THREAD_FOUND"), m_dwVirusFoundCount);
	}
	else
	{
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		AddLogEntry(L"--------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
		csCompleteScanning.Format(L">>> %s %s.", cstypeofscan, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_ABORTED"));
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		csFileScanCount.Format(L"%s%d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED"), m_FileScanned);
		csCompleteScanning.Format(L">>> %s = %d, %s = %d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_FileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_THREAD_FOUND"), m_dwVirusFoundCount);
	}
	AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"--------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	csCompleteScanning.Format(L"%s%d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TREATFOUND"), m_dwVirusFoundCount);
	if (m_bFullScan)
	{
		if ((m_dwVirusFoundCount == 0) && (m_bScnAborted == false))
		{
			DWORD	dwbVirusFound = 0x00;
			if (m_bEnableSound)
			{
				AfxBeginThread(PlayScanFinishedThreadQuick, NULL);
			}

			if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath,
				L"VirusFound", REG_DWORD, dwbVirusFound, false))
			{
				AddLogEntry(L"### Error in Set SetRegistrykeyUsingService VirusFound in CVibraniumQuickScan::QuaratineEntries", 0, 0, true, SECONDLEVEL);
			}
		}
	}

	/*if (m_bIsManualStopScan == false)
	{
		HWND hWindow = ::FindWindow(NULL, L"VIBRANIUMUI");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
		}
	}*/
	
	m_svGetScanFinishedFlag.call();

	if (m_bIsShutDownPC == true && m_bIsManualStopScan == false && CheckIsScanShutdown() == true)
	{
		CEnumProcess enumproc;
		enumproc.RebootSystem(1);
	}
	DWORD dwTotalRebootCount = 0x00;
	if (m_iThreatsFoundCount == 0)
	{
		dwTotalRebootCount = CheckForDeleteFileINIEntries() + CheckForRepairFileINIEntries();
		if (dwTotalRebootCount)
		{
			CString csMsgToRebootSystem(L"");
			csMsgToRebootSystem.Format(L"%s %d %s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART1"), dwTotalRebootCount, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART2"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));
			//if (m_svFunNotificationMessageCB.call(2, (SCITER_STRING)csMsgToRebootSystem) == 1)
			CallNotificationMessage(2, (SCITER_STRING)csMsgToRebootSystem);
			if (theApp.m_bRetval == true)
			{
				//Write a code to restart computer.
				CEnumProcess enumproc;
				enumproc.RebootSystem(0);
			}
		}
	}
	m_bQuickScan = false;
	m_bFullScan = false;
	m_bCustomscan = false;
	theApp.m_bIsScanning = false;
	return true;
}

/**********************************************************************************************************
*  Function Name		: SaveLocalDatabase
*  Description			: Function to save local white database into hard disk
*  Author Name			: Jeena Mariam Saji
*  Date					: 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::SaveLocalDatabase()
{
	__try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SAVE_LOCALDB;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumQuickScan::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
		}

		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CVibraniumQuickScan::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
		}

		if (szPipeData.dwValue == 1)
		{
			return true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	SetRegistrykeyUsingService
*  Description    :	Set any dword value into given key into registry.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD;
	//szPipeData.hHey = HKEY_LOCAL_MACHINE;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName);
	szPipeData.dwSecondValue = dwData;

	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CVibraniumQuickScan : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumQuickScan : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	StopScan
*  Description    :	stops scanning.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
void CWardWizQuickScan::StopScan()
{
	try
	{
		ShutDownScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::StopScan", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ShutDownScanning
*  Description    :	Shut down scanning with terminating all thread safely.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::ShutDownScanning()
{
	try
	{
		if (m_hThread_ScanCount != NULL)
		{
			SuspendThread(m_hThread_ScanCount);
			TerminateThread(m_hThread_ScanCount, 0);
			m_hThread_ScanCount = NULL;
		}

		if (m_hWardWizAVThread != NULL)
		{
			SuspendThread(m_hWardWizAVThread);
			TerminateThread(m_hWardWizAVThread, 0);
			m_hWardWizAVThread = NULL;
			// Just incase db connection was not closed due to abruptly stopping scanning, close DB connection.
			m_objSqlDb.Close();
		}
		SetLastScanDateTime();
		CString cstypeofscan = L"";
		switch (m_eScanType)
		{
		case QUICKSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_QUICK");
			break;
		case FULLSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_FULL");
			break;
		case CUSTOMSCAN:	cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM");
			break;
		default:			cstypeofscan = L"";
			break;
		}

		//OutputDebugString(L">>> g_hWardWizAVThread stopped");
		Sleep(500);
		m_bScnAborted = true;
		AddLogEntry(L"-------------------------------------------------------------");
		CString csCompleteScanning;

		AddLogEntry(L"-------------------------------------------------------------");
		ScanFinished();
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CVibraniumQuickScan::ShutDownScanning", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineThread
*  Description    :	If user clicks on clean button.Quarantine thread gets called.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
DWORD WINAPI QuarantineThreadQuick(LPVOID lpParam)
{
	try
	{
		CWardWizQuickScan *pThis = (CWardWizQuickScan *)lpParam;
		pThis->QuaratineEntries();
		return 1;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	QuaratineEntries
*  Description    :	Repaires or quarantines selected files one by one.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
void CWardWizQuickScan::QuaratineEntries()
{
	AddLogEntry(L"------------------------------------------------", 0, 0, true, SECONDLEVEL);
	AddLogEntry(L">>> Quarantine started..", 0, 0, true, SECONDLEVEL);
	m_bIsCleaning = true;
	try
	{
		if (m_eCurrentSelectedScanType == QUICKSCAN)
		{
			m_bQuickScan = true;
		}
		else if (m_eCurrentSelectedScanType == FULLSCAN)
		{
			m_bFullScan = true;
		}
		else if (m_eCurrentSelectedScanType == CUSTOMSCAN)
		{
			m_bCustomscan = true;
		}
		m_bRedFlag = true;
		BOOL bCheck = FALSE;
		DWORD dwVirusCount = 0x00;
		DWORD dwCleanCount = 0;
		DWORD dwRebootRepair = 0;
		DWORD dwQuarantine = 0;
		CString csThreatName, csThreatPath, csStatus, csISpyID;
		CString csVirusEntry = L"";
		TCHAR	Datebuff[9] = { 0 };
		TCHAR	csDate[9] = { 0 };
		BOOL bBackUp = 0;

		_wstrdate_s(Datebuff, 9);
		_tcscpy_s(csDate, Datebuff);

		if (!SendRecoverOperations2Service(RELOAD_DBENTRIES, L"", RECOVER, true, true))
		{
			AddLogEntry(L"### Error in CVibraniumQuickScan::SendRecoverOperations2Service RELOAD_DBENTRIES RECOVER", 0, 0, true, SECONDLEVEL);
		}
		if (!SendRecoverOperations2Service(RELOAD_DBENTRIES, L"", REPORTS, true, true))
		{
			AddLogEntry(L"### Error in CVibraniumQuickScan::SendFile4RepairUsingService RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
		}
		dwVirusCount = m_svArrayCleanEntries.length();
		for (DWORD dwCurrentVirusEntry = 0; dwCurrentVirusEntry < dwVirusCount; dwCurrentVirusEntry++)
		{
			const SCITER_VALUE svEachEntry = m_svArrayCleanEntries[dwCurrentVirusEntry];
			const std::wstring chThreatName = svEachEntry[L"ThreatName"].get(L"");
			const std::wstring chFilePath = svEachEntry[L"FilePath"].get(L"");
			const std::wstring chActionTaken = svEachEntry[L"ActionTaken"].get(L"");
			const std::wstring chSpyID = svEachEntry[L"WardWizID"].get(L"");
			bool bValue = svEachEntry[L"selected"].get(false);
			csVirusEntry.Format(L"%d", dwCurrentVirusEntry);
			bCheck = bValue;
			csStatus = chActionTaken.c_str();
			if (bCheck && (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED") || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE")))// csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND") 
			{
				csThreatName = chThreatName.c_str();
				csThreatPath = chFilePath.c_str();
				csISpyID = chSpyID.c_str();
				DWORD dwISpyID = 0;
				dwISpyID = _wtol((LPCTSTR)csISpyID);
				if (dwISpyID >= 0)
				{
					CString csEntryState;
					csEntryState.Format(L"%s : %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_FILE"), csThreatPath);
					if (!PathFileExists(csThreatPath))
					{
						m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND"));
						AddLogEntry(L"### Error in CVibraniumQuickScan::QuaratineEntries: File %s does not exist", csThreatPath, 0, true, SECONDLEVEL);
						continue;
					}

					ISPY_PIPE_DATA szPipeData = { 0 };
					szPipeData.iMessageInfo = HANDLE_VIRUS_ENTRY;
					szPipeData.dwValue = dwISpyID;
					wcscpy_s(szPipeData.szFirstParam, csThreatPath);
					wcscpy_s(szPipeData.szSecondParam, csThreatName);
					if (m_eCurrentSelectedScanType == FULLSCAN)
					{
						wcscpy_s(szPipeData.szThirdParam, L"Full Scan");
					}
					if (m_eCurrentSelectedScanType == QUICKSCAN)
					{
						wcscpy_s(szPipeData.szThirdParam, L"Quick Scan");
					}
					if (m_eCurrentSelectedScanType == CUSTOMSCAN)
					{
						wcscpy_s(szPipeData.szThirdParam, L"Custom Scan");
					}

					bool bSendReapir = SendFile4RepairUsingService(&szPipeData, true, true);

					switch (szPipeData.dwValue)
					{
					case 0x00:
						if (dwISpyID > 0)
						{
							m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
						}
						else
						{
							m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
						}
						break;
					case 0x04:
						m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR"));
						dwRebootRepair++;
						AddLogEntry(L"### Repair on Reboot File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;

					case 0x05:
						m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE"));
						dwQuarantine++;
						AddLogEntry(L"### quarantine File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;
					case 0x08:
						m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_ALREADY_REPAIRED"));
						AddLogEntry(L"### Already Repaired File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;
					case 0x09:
						m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE"));
						AddLogEntry(L"### Low disc take a backup of file File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;
					default:
						CString csFailedValue;
						csFailedValue.Format(L"%d", szPipeData.dwValue);
						AddLogEntry(L"### Repair failed file::%s with Error ::%s", csThreatPath, csFailedValue, true, SECONDLEVEL);
						szPipeData.dwValue = 0x00;
						if (dwISpyID > 0)
						{
							m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
						}
						else
						{
							m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
						}

						AddLogEntry(L"### Repair failed File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
					}
					if (szPipeData.dwValue == 0x00)
					{
						dwCleanCount++;
					}
				}
			}
		}
		m_bIsCleaning = false;
		if (dwVirusCount > 0x00)
		{
			DWORD	dwbVirusFound = 0x01;

			if (dwCleanCount >= dwVirusCount)
				dwbVirusFound = 0x00;

			if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath,
				L"VirusFound", REG_DWORD, dwbVirusFound, false))
			{
				AddLogEntry(L"### Error in Set SetRegistrykeyUsingService VirusFound in CVibraniumQuickScan::QuaratineEntries", 0, 0, true, SECONDLEVEL);
			}
		}
		Sleep(5);
		//used 0 as Type for Saving RECOVER DB
		if (!SendFile4RepairUsingService(SAVE_RECOVERDB, L"", L"", 0, true, true))
		{
			AddLogEntry(L"### Error in CVibraniumQuickScan::SendFile4RepairUsingService SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
		}

		Sleep(5);
		//used 0 as Type for Saving RECOVER DB
		if (!SendFile4RepairUsingService(SAVE_REPORTS_ENTRIES, L"", L"", 5, true, true))
		{
			AddLogEntry(L"### Error in CVibraniumQuickScan::SendFile4RepairUsingService SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
		}

		m_bRedFlag = false;
		CString cstypeofscan = L"";
		switch (m_eScanType)
		{
		case QUICKSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_QUICK");
			break;
		case FULLSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_FULL");
			break;
		case CUSTOMSCAN:	cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM");
			break;
		default:			cstypeofscan = L"";
			break;
		}
		CString csTotalClean;
		csTotalClean.Format(L"%s %s,%s: %d", cstypeofscan, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_COMPLETED"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_CLEAN_COUNT"), dwCleanCount);
		csTotalClean.Format(L"%s %s\n\n%s: %d", cstypeofscan, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_COMPLETED"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_CLEAN_COUNT"), dwCleanCount);
		CString csMsgToRebootSystem(L"");
		CallUISetScanFinishedStatus(L"");
		if (m_hQuarantineThread != NULL)
		{
			if (TerminateThread(m_hQuarantineThread, 0) == FALSE)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to Terminate QuarantineThread in CWardWizQuickScan::CloseCleaning with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Terminated QuarantineEntries thread successfully.", 0, 0, true, FIRSTLEVEL);
			m_hQuarantineThread = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::QuaratineEntries", 0, 0, true, SECONDLEVEL);
	}
	AddLogEntry(L">>> Quarantine Finished..", 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"------------------------------------------------", 0, 0, true, SECONDLEVEL);
}

/**********************************************************************************************************
*  Function Name  :	SendRecoverOperations2Service
*  Description    :	Send a request to stored data into recover db.So that user can recover file.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry, DWORD dwType, bool bWait, bool bReconnect)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	szPipeData.iMessageInfo = dwMessageinfo;
	_tcscpy_s(szPipeData.szFirstParam, csRecoverFileEntry);
	szPipeData.dwValue = dwType;
	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CVibraniumQuickScan::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to Read data in CVibraniumQuickScan::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (szPipeData.dwValue != 1)
		{
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SendRecoverOperations2Service
*  Description    :	Sends data to service to repair a file and service reply the status to GUI
*  SR.NO		  :
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::SendFile4RepairUsingService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect)
{
	try
	{
		if (!m_objCom.SendData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumQuickScan::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (bWait)
		{
			if (!m_objCom.ReadData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CVibraniumQuickScan::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::SendFile4RepairUsingService1", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SendFile4RepairUsingService
*  Description    :	Send request to clean file to service
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::SendFile4RepairUsingService(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait, bool bReconnect)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));
	szPipeData.iMessageInfo = iMessage;
	szPipeData.dwValue = dwISpyID;
	wcscpy_s(szPipeData.szFirstParam, csThreatPath);
	wcscpy_s(szPipeData.szSecondParam, csThreatName);
	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CVibraniumQuickScan::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CVibraniumQuickScan::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (szPipeData.dwValue == 0)
		{
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	VirusFoundEntries
*  Description    :	Pipe Communication to send Virus found entries to UI
Issue: Displaying Virus found entries on UI through Pipe Communication
Date : 15-Jan-2015
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::VirusFoundEntries(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (!lpSpyData)
			return false;

		CString csCurrentPath(L"");
		CString csStatusForReports = L"";
		csCurrentPath.Format(L"%s", lpSpyData->szSecondParam);
		if (csCurrentPath.GetLength() > 0 && (g_csPreviousVirusFoundPathQuick != csCurrentPath))
		{
			CString csStatus = L"";
			switch (lpSpyData->dwSecondValue)
			{
			case FILESKIPPED:
				csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
				break;
			case FILEQURENTINED:
				csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED");
				break;
			case FILEREPAIRED:
				csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED");
				break;
			case LOWDISKSPACE:
				csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE");
				break;
			}
			csStatusForReports = csStatus;
			DWORD dwSpyID = (*(DWORD *)&lpSpyData->byData[0]);
			CString csSpyID;
			csSpyID.Format(L"%d", dwSpyID);
			CallUISetVirusFoundEntryfunction(lpSpyData->szFirstParam, csCurrentPath, csStatus, csSpyID);
			if (m_dwVirusFoundCount % 5 == 0)
			{
				if (m_bEnableSound)
				{
					AfxBeginThread(PlayThreatsFoundThreadQuick, NULL);
				}
			}
			AddLogEntry(L">>> Virus Found GUI: %s, File Path: %s", lpSpyData->szFirstParam, csCurrentPath, true, SECONDLEVEL);
		}
		g_csPreviousVirusFoundPathQuick.SetString(csCurrentPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::VirusFoundEntries", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SaveDBEntries
*  Description    :	Save recover entries in DB folder
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::SaveDBEntries()
{
	try
	{
		AddLogEntry(L"CVibraniumQuickScan::SaveDBEntries() entered!!", 0, 0, true, ZEROLEVEL);
		if (!SendFile4RepairUsingService(SAVE_RECOVERDB, L"", L"", 0, true, true))
		{
			AddLogEntry(L"### Error in CVibraniumQuickScan::SaveDBEntries SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
			return false;
		}
		Sleep(5);
		if (!SendFile4RepairUsingService(SAVE_REPORTS_ENTRIES, L"", L"", 5, true, true))
		{
			AddLogEntry(L"### Error in CVibraniumQuickScan::SaveDBEntries SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::SaveDBEntries.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetWardwizRegistryDetails
*  Description    :	Read Scanning related options from registry
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt)
{
	try
	{
		HKEY hKey;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, &hKey) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to open registry key in CWardwizScan::GetWardwizRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			return false;
		}

		DWORD dwOptionSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;

		long ReadReg = RegQueryValueEx(hKey, L"dwQuarantineOption", NULL, &dwType, (LPBYTE)&dwQuarantineOpt, &dwOptionSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to get registry key value for Quarantine Option in CVibraniumQuickScan::GetWardwizRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
		}

		ReadReg = RegQueryValueEx(hKey, L"dwHeuScan", NULL, &dwType, (LPBYTE)&dwHeuScanOpt, &dwOptionSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to get registry key value for Heuristic Scan Option in CVibraniumQuickScan::GetWardwizRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::GetWardwizRegistryDetails", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
/**********************************************************************************************************
*  Function Name  :	PlayScanFinishedThread
*  Description    :	Thread will play a sound when scan gets finished.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
UINT PlayScanFinishedThreadQuick(LPVOID lpThis)
{
	try
	{
		PlaySound(_T("ScanFinished.wav"), NULL, SND_FILENAME | SND_LOOP | SND_SYNC);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in PlayScanFinishedThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	PlayThreatsFoundThread
*  Description    :	Thread will play a sound when virus get found.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
UINT PlayThreatsFoundThreadQuick(LPVOID lpThis)
{
	try
	{
		PlaySound(_T("ThreatsFound.wav"), NULL, SND_FILENAME | SND_LOOP | SND_SYNC);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in PlayThreatsFoundThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	ReadUISettingFromRegistry
*  Description    :	Read Enable/disable sound entry from registry
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
bool CWardWizQuickScan::ReadUISettingFromRegistry()
{
	try
	{
		HKEY key;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, &key) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to open registry key in CVibraniumQuickScan::ReadUISettingFromRegistry, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			return false;
		}

		DWORD dwPlaySound;
		DWORD dwPlaySoundSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		long ReadReg = RegQueryValueEx(key, L"dwEnableSound", NULL, &dwType, (LPBYTE)&dwPlaySound, &dwPlaySoundSize);
		if (ReadReg == ERROR_SUCCESS)
		{
			dwPlaySound = (DWORD)dwPlaySound;
			if (dwPlaySound == 0)
			{
				m_bEnableSound = false;
			}
			else
			{
				m_bEnableSound = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::ReadUISettingFromRegistry", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetModuleCount
*  Description    :	Give total modules of proesses in the case of quick scan
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
void CWardWizQuickScan::GetModuleCount()
{
	try
	{
		DWORD	dwPID[0x100] = { 0 };
		DWORD	dwBytesRet = 0x00, dwProcIndex = 0x00;
		m_csaModuleList.RemoveAll();
		EnumProcesses(dwPID, 0x400, &dwBytesRet);
		dwBytesRet = dwBytesRet / sizeof(DWORD);
		m_iTotalFileCount = 0;
		for (dwProcIndex = 0; dwProcIndex < dwBytesRet; dwProcIndex++)
		{
			HMODULE		hMods[1024] = { 0 };
			HANDLE		hProcess = NULL;
			DWORD		dwModules = 0x00;
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, dwPID[dwProcIndex]);
			if (!hProcess)
				continue;
			if (EnumProcessModulesWWizEx != NULL)
			{
				if (!EnumProcessModulesWWizEx(hProcess, hMods, sizeof(hMods), &dwModules, LIST_MODULES_ALL))
				{
					DWORD error = GetLastError();
					CloseHandle(hProcess);
					continue;
				}
			}
			else
			{
				if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules))
				{
					DWORD error = GetLastError();
					CloseHandle(hProcess);
					continue;
				}
			}
			for (DWORD iModIndex = 0; iModIndex < (dwModules / sizeof(HMODULE)); iModIndex++)
			{
				TCHAR szModulePath[MAX_PATH * 2] = { 0 };
				GetModuleFileNameEx(hProcess, hMods[iModIndex], szModulePath, MAX_PATH * 2);
				if (!IsDuplicateModule(szModulePath, sizeof(szModulePath) / sizeof(TCHAR)))
				{
					m_iTotalFileCount++;
					_tcsupr_s(szModulePath, sizeof(szModulePath) / sizeof(TCHAR));
					m_csaModuleList.AddHead(szModulePath);
				}
			}
			CloseHandle(hProcess);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::GetModuleCount", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : IsDuplicateModule
Description    : Function to find duplicates modules to avoid multiple scanning.
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
****************************************************************************/
bool CWardWizQuickScan::IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize)
{
	bool bReturn = false;
	try
	{
		_tcsupr_s(szModulePath, dwSize);
		if (m_csaModuleList.Find(szModulePath, 0))
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::IsDuplicateModule", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : LoadPSAPILibrary
Description    : Load PSAPI.DLL.
For Issue: In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
****************************************************************************/
void CWardWizQuickScan::LoadPSAPILibrary()
{
	__try
	{
		TCHAR	szSystem32[256] = { 0 };
		TCHAR	szTemp[256] = { 0 };
		GetSystemDirectory(szSystem32, 255);

		ZeroMemory(szTemp, sizeof(szTemp));
		wsprintf(szTemp, L"%s\\PSAPI.DLL", szSystem32);
		if (!m_hPsApiDLL)
		{
			m_hPsApiDLL = LoadLibrary(szTemp);
		}

		if (!EnumProcessModulesWWizEx)
		{
			EnumProcessModulesWWizEx = (ENUMPROCESSMODULESEX)GetProcAddress(m_hPsApiDLL, "EnumProcessModulesEx");
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::LoadPSAPILibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTotalScanFilesCountQuick
Description    : Get total files count in case of quick scan
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
SR_NO			 :
****************************************************************************/
DWORD WINAPI GetTotalScanFilesCountQuick(LPVOID lpParam)
{
	try
	{
		CWardWizQuickScan *pThis = (CWardWizQuickScan*)lpParam;
		if (!pThis)
			return 1;
		int	iIndex = 0x00;
		iIndex = (int)pThis->m_csaAllScanPaths.GetCount();
		if (!iIndex)
			return 2;
		for (int i = 0; i < iIndex; i++)
		{
			pThis->EnumFolder(pThis->m_csaAllScanPaths.GetAt(i));
		}
		if (pThis->m_iTotalFileCount)
		{
			pThis->m_ScanCount = true;
		}
		pThis->m_dwTotalFileCount = pThis->m_iTotalFileCount;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::GetTotalScanFilesCountQuick", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : EnumerateThread
Description    : Thread to enumerarte process in case of quick scan and
files and folders in case of custom and full scan.
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
SR_NO			 :
****************************************************************************/
DWORD WINAPI EnumerateThreadQuick(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizQuickScan *pThis = (CWardWizQuickScan*)lpvThreadParam;
		if (!pThis)
			return 0;
		int	iIndex = 0x00;
		if (pThis->m_eScanType == QUICKSCAN)
		{
			pThis->EnumerateProcesses();
		}
		iIndex = (int)pThis->m_csaAllScanPaths.GetCount();
		if (!iIndex)
			return 2;
		for (int i = 0; i < iIndex; i++)
		{
			CString csPath = pThis->m_csaAllScanPaths.GetAt(i);
			if (!PathFileExists(csPath))
			{
				continue;
			}
			pThis->m_bIsPathExist = true;
			pThis->EnumFolderForScanningFull(csPath);
		}
		if (!pThis->m_bManualStop)
		{
			ITIN_MEMMAP_DATA iTinMemMap = { 0 };
			iTinMemMap.iMessageInfo = DISABLE_CONTROLS;
			iTinMemMap.dwSecondValue = pThis->m_iThreatsFoundCount;
		}
		if (pThis->m_iThreatsFoundCount == 0 && !pThis->m_bManualStop)
		{
			pThis->AddEntriesInReportsDB(pThis->m_eScanType, L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND"));
		}
		pThis->ScanFinished();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::EnumerateThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************            
*  Function Name  :	EnumerateProcesses
*  Description    :	Enumaerate processes in case of quick scan.
Changes (Ram) : Time complexity decresed as we enumerating processes and modules
to calculate file count, There is no need to enumerate it again.
kept in CStringList while file count calculation, same list is used again.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizQuickScan::EnumerateProcesses()
{
	try
	{
		CString csProcessPath(L"");
		CString csToken(L"");
		CString csTokenSytemRoot(L"");
		TCHAR szSystemDirectory[MAX_PATH] = { 0 };
		bool bSystemDirectory = false;
		bool bReplaceWindowPath = false;

		POSITION pos = m_csaModuleList.GetHeadPosition();
		while (pos != NULL)
		{
			csProcessPath = m_csaModuleList.GetNext(pos);
			int iPos = 0;
			int SlashPos = 0;
			SlashPos = csProcessPath.Find(_T("\\"), iPos);
			if (SlashPos == 0)
			{
				csToken = csProcessPath.Right(csProcessPath.GetLength() - (SlashPos + 1));
				bSystemDirectory = true;
			}
			GetWindowsDirectory(szSystemDirectory, MAX_PATH);
			if (bSystemDirectory == true)
			{
				SlashPos = 0;
				iPos = 0;
				SlashPos = csToken.Find(_T("\\"), iPos);
				csTokenSytemRoot = csToken;
				csTokenSytemRoot.Truncate(SlashPos);
				if (csTokenSytemRoot == L"SystemRoot")
				{
					bReplaceWindowPath = true;
				}
				else if (csTokenSytemRoot == L"??")
				{
					csToken.Replace(L"??\\", L"");
					csProcessPath = csToken;
				}
				bSystemDirectory = false;
			}
			if (bReplaceWindowPath == true)
			{
				csToken.Replace(csTokenSytemRoot, szSystemDirectory);
				csProcessPath = csToken;
				bReplaceWindowPath = false;
			}
			if (PathFileExists(csProcessPath))
			{
				if (m_bManualStop)
				{
					break;
				}
				ScanForSingleFileFull(csProcessPath);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::EnumerateProcesses", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	EnumFolderForScanning
*  Description    :	enumerate files of system and sent it to scan.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizQuickScan::EnumFolderForScanningFull(LPCTSTR pstr)
{
	try
	{
		if (!pstr)
			return;

		CFileFind finder;
		DWORD	dwAttributes = 0;
		CString strWildcard(pstr);
		CString csFilePath(pstr);

		//Check here is file/folder from removable device
		if (GetDriveType((LPCWSTR)strWildcard.Left(2)) == DRIVE_REMOVABLE)
		{
			//Is file/folder is hidden?
			if (FILE_ATTRIBUTE_HIDDEN == (GetFileAttributes(strWildcard) & FILE_ATTRIBUTE_HIDDEN))
			{
				//Check is file/folder on root path?
				if (CheckFileOrFolderOnRootPath(strWildcard))
				{
					SetFileAttributes(strWildcard, FILE_ATTRIBUTE_NORMAL);
				}
			}
		}

		BOOL bRet = PathIsDirectory(strWildcard);
		if (bRet == FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strWildcard[strWildcard.GetLength() - 1] != '\\')
				strWildcard += _T("\\*.*");
			else
				strWildcard += _T("*.*");
		}
		else
		{
			if (!PathFileExists(pstr))
			{
				return;
			}
			if (m_bManualStop)
			{
				return;
			}

			bool bIsSubFolderExcluded = false;
			CString csFileExt;
			int iExtRevCount = csFilePath.ReverseFind('.');
			if (iExtRevCount > 0)
			{
				csFileExt = csFilePath.Right((csFilePath.GetLength() - iExtRevCount));
				csFileExt.Trim('.');

				if (m_objExcludeFilesFolders.ISExcludedFileExt((LPTSTR)csFileExt.GetString()))
				{
					AddLogEntry(L">>> Excluded File Extension [%s] ", csFileExt, 0, true, ZEROLEVEL);
					return;
				}
			}
			if (m_objExcludeFilesFolders.ISExcludedPath((LPTSTR)pstr, bIsSubFolderExcluded))
			{
				AddLogEntry(L">>> Excluded Path [%s] ", pstr, 0, true, ZEROLEVEL);
				return;
			}

			ScanForSingleFileFull(pstr);
			return;
		}


		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;
			if (finder.IsDirectory())
			{
				CString str = finder.GetFilePath();

				bool bIsSubFolderExcluded = false;
				if (m_objExcludeFilesFolders.ISExcludedPath((LPTSTR)str.GetBuffer(), bIsSubFolderExcluded))
				{
					if (bIsSubFolderExcluded)
					{
						AddLogEntry(L">>> Excluded Path [%s] ", str, 0, true, ZEROLEVEL);
						continue;
					}
				}

				EnumFolderForScanningFull(str);
			}
			else
			{
				CString csFilePath = finder.GetFilePath();
				if (csFilePath.Trim().GetLength() > 0)
				{
					//Check here is file/folder from removable device
					if (GetDriveType((LPCWSTR)csFilePath.Left(2)) == DRIVE_REMOVABLE)
					{
						//Is file/folder is hidden?
						if (FILE_ATTRIBUTE_HIDDEN == (GetFileAttributes(csFilePath) & FILE_ATTRIBUTE_HIDDEN))
						{
							//Check is file/folder on root path?
							if (CheckFileOrFolderOnRootPath(csFilePath))
							{
								SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
							}
						}
					}

					if (PathFileExists(csFilePath))
					{
						if (m_bManualStop)
						{
							break;
						}

						bool bIsSubFolderExcluded = false;
						CString csFileExt;
						int iExtRevCount = csFilePath.ReverseFind('.');
						if (iExtRevCount > 0)
						{
							csFileExt = csFilePath.Right((csFilePath.GetLength() - iExtRevCount));
							csFileExt.Trim('.');
							if (m_objExcludeFilesFolders.ISExcludedFileExt((LPTSTR)csFileExt.GetString()))
							{
								AddLogEntry(L">>> Excluded File Extension [%s] ", csFileExt, 0, true, ZEROLEVEL);
								continue;
							}
						}
						if (m_objExcludeFilesFolders.ISExcludedPath((LPTSTR)csFilePath.GetBuffer(), bIsSubFolderExcluded))
						{
							AddLogEntry(L">>> Excluded Path [%s] ", csFilePath, 0, true, ZEROLEVEL);
							continue;
						}

						ScanForSingleFileFull(csFilePath);
					}
				}
			}
			Sleep(10);
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::EnumFolderForScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ScanForSingleFile
*  Description    :	Scan each single file .
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizQuickScan::ScanForSingleFileFull(CString csFilePath)
{
	if (csFilePath.GetLength() == 0)
		return;
	try
	{
		bool bSetStatus = false;
		bool bVirusFound = false;

		CString csVirusName(L"");
		CString csVirusPath(L"");
		DWORD dwISpywareID = 0;
		DWORD dwAction = 0x00;
		CString csCurrentFile(L"");
		CString csStatus(L"");
		m_csCurrentFilePath = csFilePath;
		if (csFilePath.Trim().GetLength() > 0)
		{
			//if (m_csPreviousPath != csFilePath)
			{
				if (PathFileExists(csFilePath))
				{
					m_bIsPathExist = true;
					DWORD dwISpyID = 0;
					TCHAR szVirusName[MAX_PATH] = { 0 };
					DWORD dwSignatureFailedToLoad = 0;
					DWORD dwActionTaken = 0x00;
					if ((CheckFileIsInRepairRebootIni(csFilePath)) ||
						(CheckFileIsInRecoverIni(csFilePath)) || (CheckFileIsInDeleteIni(csFilePath))
						)

						return;

					//Check here is file is excluded?
					bool bIsSubFolderExcluded = false;
					CString csFileExt;
					int iExtRevCount = csFilePath.ReverseFind('.');
					if (iExtRevCount > 0)
					{
						csFileExt = csFilePath.Right((csFilePath.GetLength() - iExtRevCount));
						csFileExt.Trim('.');

						if (m_objExcludeFilesFolders.ISExcludedFileExt((LPTSTR)csFileExt.GetString()))
						{
							AddLogEntry(L">>> Excluded File Extension [%s] ", csFileExt, 0, true, ZEROLEVEL);
							return;
						}
					}
					if (m_objExcludeFilesFolders.ISExcludedPath((LPTSTR)csFilePath.GetBuffer(), bIsSubFolderExcluded))
					{
						AddLogEntry(L">>> Excluded Path [%s] ", csFilePath, 0, true, ZEROLEVEL);
						return;
					}

					CString csActionID;
					m_FileScanned++;
					if (ScanFile(csFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, dwActionTaken, m_bRescan))
					{
						if (dwISpyID >= 0)
						{
							csVirusName = szVirusName;
							dwISpywareID = dwISpyID;

							CString csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
							csActionID = "IDS_FILE_SKIPPED";
							switch (dwActionTaken)
							{
							case FILESKIPPED:
								csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
								csActionID = "IDS_FILE_SKIPPED";
								break;
							case FILEQURENTINED:
								csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED");
								csActionID = "IDS_CONSTANT_THREAT_QUARANTINED";
								m_dwVirusCleanedCount++;
								break;
							case FILEREPAIRED:
								csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED");
								csActionID = "IDS_CONSTANT_THREAT_REPAIRED";
								m_dwVirusCleanedCount++;
								break;
							case LOWDISKSPACE:
								csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE");
								csActionID = "IDS_CONSTANT_THREAT_LOWDISC_SPACE";
								m_dwVirusCleanedCount++;
								break;
							case FILEREBOOTQUARENTINE:
								csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE");
								m_dwVirusCleanedCount++;
								break;
							case FILEREBOOTREPAIR:
								csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR");
								m_dwVirusCleanedCount++;
								break;

							}
							CString csISpyID = L"";
							csISpyID.Format(L"%d", dwISpyID);
							CallUISetVirusFoundEntryfunction(csVirusName, csFilePath, csStatus, csISpyID);
							m_dwVirusFoundCount++;
							// Add entries into Database..
							// Get entries from registry so that, those can be included in query..
							DWORD dwQuarantineOpt;
							DWORD dwHeuristicOpt;
							bool  bHeuristicOpt = false;

							GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);

							if (dwHeuristicOpt == 1)
								bHeuristicOpt = true;

							CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");

							csVirusName.Replace(L"'", L"''");
							csFilePath.Replace(L"'", L"''");

							csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%I64d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),'%s','%s','%s',NULL );"), m_iScanSessionId, csVirusName, csFilePath, csActionID);

							CT2A ascii(csInsertQuery, CP_UTF8);

							InsertSQLiteData(ascii.m_psz);
						}
						if (theApp.m_dwProductID == ELITE)
						{
							//Send here to EPS client
							if (!SendData2EPSClient(csFilePath, szVirusName, theApp.m_csTaskID, dwActionTaken))
							{
								AddLogEntry(L"### Failed to SendData to EPS Client in CVibraniumQuickScan::EnumFolder", 0, 0, true, SECONDLEVEL);
							}
						}
					}
					DWORD dwRet = 0;
					//DWORD dwRet = ScanFile(csFilePath.GetBuffer(), szVirusName, dwISpyID, m_eScanType, dwAction, dwSignatureFailedToLoad, m_bRescan);
					if (dwRet != 0x00)
					{
						if (dwISpyID >= 0)
						{
							csVirusName = szVirusName;
							dwISpywareID = dwISpyID;
							csStatus = csFilePath;
							bVirusFound = true;
						}
					}
				}
			}
			/*
			CallUISetStatusfunction(csFilePath);
			CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
			CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
			CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
			m_csPreviousPath = csFilePath;
			}
			*/
		}

		//virus found 
		if (bVirusFound)
		{
			bSetStatus = true;
		}
		else
		{
			bSetStatus = true;
			csStatus = csFilePath;
		}
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in void CVibraniumQuickScan::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetActionIDFromAction
*  Description    : Get action Id as stored in ini file from the action text.This will be used to
store in DB.Will help in multi language support.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
**********************************************************************************************************/
CString GetActionLangIDFromActionQuick(CString csMessage)
{
	CString csLanguageID = 0;

	if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED")) == 0)
	{
		csLanguageID = "IDS_USB_SCANNING_ABORTED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND")) == 0)
	{
		csLanguageID = "IDS_USB_SCAN_NO_THREAT_FOUND";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_DETECTED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_QUARANTINED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_REPAIRED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_ALREADY_REPAIRED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_ALREADY_REPAIRED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_NO_FILE_FOUND";
	}

	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_FAILED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_FAILED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_COMPLETED")) == 0)
	{
		csLanguageID = "IDS_MSG_SCANNING_COMPLETED";
	}
	return csLanguageID;
}

/**********************************************************************************************************
*  Function Name  :	AddEntriesInReportsDB
*  Description    :	Add entries to report DB,to display report in report tab
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizQuickScan::AddEntriesInReportsDB(SCANTYPE eScanType, CString csThreatName, CString csFilePath, CString csAction)
{
	try
	{
		SYSTEMTIME		CurrTime = { 0 };
		GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
		CTime	Time_Curr(CurrTime);
		int iMonth = Time_Curr.GetMonth();
		int iDate = Time_Curr.GetDay();
		int iYear = Time_Curr.GetYear();

		CString csDate = L"";
		csDate.Format(L"%d/%d/%d", iMonth, iDate, iYear);
		CTime ctDateTime = CTime::GetCurrentTime();
		TCHAR csScanType[0x30];
		switch (eScanType)
		{
		case FULLSCAN:
			_tcscpy(csScanType, theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FULLSCAN"));
			break;
		case CUSTOMSCAN:
			_tcscpy(csScanType, theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CUSTOMSCAN"));
			break;
		case QUICKSCAN:
			_tcscpy(csScanType, theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_QUICKSCAN"));
			break;
		case USBSCAN:
		case USBDETECT:
			_tcscpy(csScanType, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_SCAN"));
			break;
		}
		CString csTime = ctDateTime.Format(_T("%H:%M:%S"));
		CString csDateTime = L"";
		csDateTime.Format(_T("%s %s"), csDate, csTime);
		CIspyList newEntry(csDateTime, csScanType, csThreatName, csFilePath, csAction);
		CWardWizSQLiteDatabase objSqlDb(STRDATABASEFILE);
		DWORD dwQuarantineOpt;
		DWORD dwHeuristicOpt;
		bool  bHeuristicOpt = false;
		GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);
		if (dwHeuristicOpt == 1)
			bHeuristicOpt = true;
		CString csActionID = GetActionLangIDFromActionQuick(csAction);
		CString csInsertQuery = 0;
		csInsertQuery.Format(_T("UPDATE Wardwiz_ScanSessionDetails SET db_ScanSessionEndDate = Date('now'),db_ScanSessionEndTime = Datetime('now', 'localtime'),db_TotalFilesScanned = %d,db_TotalThreatsFound = %d, db_TotalThreatsCleaned = %d WHERE db_ScanSessionID = %I64d;"), m_FileScanned, m_dwVirusFoundCount, m_dwVirusCleanedCount, m_iScanSessionId);
		CT2A ascii(csInsertQuery, CP_UTF8);
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", theApp.GetModuleFilePath());
		if (!PathFileExists(csWardWizReportsPath))
		{
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			objSqlDb.Close();
		}
		InsertSQLiteData(ascii.m_psz);
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CVibraniumQuickScan::AddEntriesInReportsDB", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInRepairRebootIni
*  Description    :	Get Quarantine folder path.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizQuickScan::CheckFileIsInRepairRebootIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };
		if (GetQuarantineFolderPath(szRepairIniPath))
		{
			AddLogEntry(L"### Failed in CVibraniumQuickScan::CheckFileIsInRepairRebootIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}
		wcscat_s(szRepairIniPath, _countof(szRepairIniPath), L"\\WWRepair.ini");
		DWORD dwRepairCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRepairIniPath);
		if (!dwRepairCount)
			return bReturn;

		DWORD	i = 0x01;
		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[2048] = { 0 };
		TCHAR	szFilePath[512] = { 0 };

		swprintf_s(szFilePath, _countof(szFilePath), L"|%s|", csFilePath);
		_wcsupr(szFilePath);
		for (; i <= dwRepairCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRepairIniPath);
			_wcsupr(szValueData);
			if (wcsstr(szValueData, szFilePath) != NULL)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CheckFileIsInRepairRebootIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizQuickScan::GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath)
{
	bool	bReturn = true;
	try
	{
		TCHAR	szModulePath[MAX_PATH] = { 0 };

		GetModulePath(szModulePath, MAX_PATH);
		if (!wcslen(szModulePath))
			return bReturn;
		wcscpy_s(lpszQuarantineFolPath, MAX_PATH - 1, szModulePath);
		wcscat_s(lpszQuarantineFolPath, MAX_PATH - 1, L"\\Quarantine");
		bReturn = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : Checking file entry present in Recover ini (WWRecover.ini)
Description    : Parses WWRecover.ini and not sends to scan if file is found.
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
***********************************************************************************************/
bool CWardWizQuickScan::CheckFileIsInRecoverIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR	szRecoverIniPath[MAX_PATH] = { 0 };
		if (GetQuarantineFolderPath(szRecoverIniPath))
		{
			AddLogEntry(L"### Failed in CVibraniumQuickScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}
		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), L"\\");
		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), WRDWIZRECOVERINI);
		DWORD dwRecoverCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRecoverIniPath);
		if (!dwRecoverCount)
			return bReturn;

		DWORD	i = 0x01;
		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[512] = { 0 };

		for (; i <= dwRecoverCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			ZeroMemory(szValueData, sizeof(szValueData));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRecoverIniPath);
			if (csFilePath.CompareNoCase(szValueData) == 0x00)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CheckFileIsInRecoverIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInDeleteIni
*  Description    :	Check whether file available in delete.ini
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizQuickScan::CheckFileIsInDeleteIni(CString csQurFilePaths)
{
	bool	bReturn = false;
	TCHAR szValueName[260] = { 0 };
	DWORD dwCount = 0x00;
	TCHAR szDuplicateString[512] = { 0 };
	TCHAR szTempKey[512] = { 0 };
	TCHAR szApplnName[512] = { 0 };
	TCHAR szConcatnateDeleteString[1024] = { 0 };
	TCHAR szFileName[512] = { 0 };
	TCHAR szQuarantineFileName[512] = { 0 };
	TCHAR szVirusName[512] = { 0 };

	try
	{
		CString csDeleteFailedINIPath(L"");
		CString csQuarantineFolderPath = GetQuarantineFolderPath();
		csDeleteFailedINIPath.Format(L"%s\\WRDWIZDELETEFAIL.INI", csQuarantineFolderPath);
		DWORD dwDeleteCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csDeleteFailedINIPath);
		if (!dwDeleteCount)
			return bReturn;

		for (int i = 0x01; i <= static_cast<int>(dwDeleteCount); i++)
		{
			ZeroMemory(szTempKey, sizeof(szTempKey));
			swprintf_s(szTempKey, _countof(szTempKey), L"%lu", i);
			GetPrivateProfileString(L"DeleteFiles", szTempKey, L"", szDuplicateString, 511, csDeleteFailedINIPath);
			ZeroMemory(szApplnName, sizeof(szApplnName));
			if (!TokenizationOfParameterForrecover(szDuplicateString, szFileName, _countof(szFileName), szQuarantineFileName, _countof(szQuarantineFileName), szVirusName, _countof(szVirusName)))
			{
				AddLogEntry(L"### CVibraniumQuickScan::TokenizationOfParameterForrecover is not tokenize properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}
			if (!TokenizeIniData(szFileName, szApplnName, _countof(szApplnName)))
			{
				AddLogEntry(L"### CVibraniumQuickScan::CheckFileIsInDeleteIni::TokenizeIniData is not tokenize properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}
			CString csDuplicateString = (CString)szDuplicateString;
			CString csFileName = (CString)szApplnName;
			if (csFileName.CompareNoCase(csQurFilePaths) == 0)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CheckFileIsInDeleteIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
CString CWardWizQuickScan::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***********************************************************************************************
Function Name  : TokenizationOfParameterForrecover
Description    : Tokenize input path to get file name, quarantine name and virus name
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
***********************************************************************************************/
bool CWardWizQuickScan::TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName)
{
	TCHAR	szToken[] = L"|";
	TCHAR	*pToken = NULL;
	try
	{
		if (lpWholePath == NULL || szFileName == NULL || szQuarantinepath == NULL || szVirusName == NULL)
			return false;

		pToken = wcstok(lpWholePath, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
		{
			wcscpy_s(szFileName, (dwsizeofFileName - 1), pToken);
		}
		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szQuarantinepath, (dwsizeofquarantinefileName - 1), pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szVirusName, (dwsizeofVirusName - 1), pToken);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::TokenizationOfParameterForrecover", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	TokenizeIniData
*  Description    :	Tokenization of entries from delete file ini
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizQuickScan::TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName)
{
	TCHAR	szToken[] = L",";
	TCHAR	*pToken = NULL;
	try
	{
		if (lpszValuedata == NULL || szApplicationName == NULL)
			return false;
		pToken = wcstok(lpszValuedata, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		TCHAR	szValueApplicationName[512] = { 0 };
		if (pToken)
		{
			wcscpy_s(szValueApplicationName, _countof(szValueApplicationName), pToken);
			wcscpy_s(szApplicationName, (dwSizeofApplicationName - 1), szValueApplicationName);
		}
		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		}
		TCHAR	szAttemptCnt[16] = { 0 };
		if (pToken)
			wcscpy_s(szAttemptCnt, _countof(szAttemptCnt), pToken);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::TokenizeIniData", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	EnumFolder
*  Description    :	Enumerate each folders of system and calculate total files count.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizQuickScan::EnumFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");
		BOOL bWorking = finder.FindFile(strWildcard);
		if (bWorking)
		{
			while (bWorking)
			{
				bWorking = finder.FindNextFile();
				if (finder.IsDots())
					continue;

				// if it's a directory, recursively search it 
				if (finder.IsDirectory())
				{
					CString str = finder.GetFilePath();
					EnumFolder(str);
				}
				else
				{
					m_iTotalFileCount++;
				}
			}
		}
		else
		{
			m_iTotalFileCount++;
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ScanFile
*  Description    :	Exported funtion to scan file
Scanning using service, the aim is to keep the database in common memory location.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  : WRDWIZUSBUI_0039
**********************************************************************************************************/
bool CWardWizQuickScan::ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan)
{
	try
	{
		bool bSendFailed = false;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SCAN_FILE;
		wcscpy_s(szPipeData.szFirstParam, szFilePath);
		if (!m_objScanCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumQuickScan::ScanFile", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}
		if (!m_objScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CVibraniumQuickScan::ScanFile", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}
		if (szPipeData.dwValue == 1)
		{
			dwActionTaken = szPipeData.dwSecondValue;
			_tcscpy(szVirusName, szPipeData.szSecondParam);
			dwISpyID = (*(DWORD *)&szPipeData.byData[0]);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::ScanFile, File: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
*  Function Name  : CheckForDeleteFileINIEntries
*  Description    : check whether delete file ini present or not
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*************************************************************************************************/
DWORD CWardWizQuickScan::CheckForDeleteFileINIEntries()
{
	bool bReturn = false;
	DWORD dwCount = 0x00;
	try
	{
		CString csQuarantineFolderPath = GetQuarantineFolderPath();
		csQuarantineFolderPath.Append(L"\\WRDWIZDELETEFAIL.INI");

		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csQuarantineFolderPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CheckForDeleteFileINIEntries()", 0, 0, true, SECONDLEVEL);
	}
	return dwCount;
}

/***********************************************************************************************
*  Function Name  : CheckForRepairFileINIEntries
*  Description    : check whether repair file ini present or not
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*************************************************************************************************/
DWORD CWardWizQuickScan::CheckForRepairFileINIEntries()
{
	bool bReturn = false;
	DWORD dwCount = 0x00;
	try
	{
		CString csRepairIniPath = GetQuarantineFolderPath();
		csRepairIniPath += L"\\";
		csRepairIniPath += WRDWIZREPAIRINI;

		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csRepairIniPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CheckForRepairFileINIEntries()", 0, 0, true, SECONDLEVEL);
	}
	return dwCount;
}

/***************************************************************************************************
Function Name  : CallNotificationMessage
Description    : Calls Light box on UI
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
/***************************************************************************************************/
void CWardWizQuickScan::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
{
	try
	{
		m_svFunNotificationMessageCB.call(iMsgType, (SCITER_STRING)strMessageString);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		Sleep(300);
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : on_timer
Description    : On timer
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
/***************************************************************************************************/
bool CWardWizQuickScan::on_timer(HELEMENT he, UINT_PTR extTimerId)
{
	try
	{
		/*CallUISetStatusfunction(m_csCurrentFilePath);
		CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
		CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
		CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
		m_csPreviousPath = m_csCurrentFilePath;
		AddLogEntry(L"### Called CWardwizScan::on_timer()", 0, 0, true, SECONDLEVEL);*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::on_timer(HELEMENT he, UINT_PTR extTimerId)", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : on_timer
Description    : On timer
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
/***************************************************************************************************/
bool CWardWizQuickScan::on_timer(HELEMENT he)
{
	__try
	{
		return  on_timerSEH(he);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::on_timer", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : on_timerSEH
Description    : On timer
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
/***************************************************************************************************/
bool CWardWizQuickScan::on_timerSEH(HELEMENT he)
{
	CallUISetStatusfunction(m_csCurrentFilePath.GetBuffer());
	CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
	CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
	CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
	//m_csPreviousPath = m_csCurrentFilePath;
	return true;
}

/***************************************************************************************************
Function Name  : OnTimerScan
Description    : On timer
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
/***************************************************************************************************/
void CWardWizQuickScan::OnTimerScan()
{
	try
	{
		CallUISetStatusfunction(m_csCurrentFilePath.GetBuffer());
		CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
		CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
		CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::OnTimerScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : CheckFileOrFolderOnRootPath
Description    : Function which check IS file/Folder present on root path
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
/*************************s**************************************************************************/
bool CWardWizQuickScan::CheckFileOrFolderOnRootPath(CString csFilePath)
{
	try
	{
		int iPos = csFilePath.ReverseFind(L'\\');
		if (iPos == csFilePath.GetLength() - 1)
		{
			iPos = csFilePath.Left(csFilePath.GetLength() - 1).ReverseFind(L'\\');
		}

		if (iPos == 0x02)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CheckFileOrFolderOnRootPath, Path: %s", csFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : SetLastScanDateTime( )
Description    : Sets the Scan Date & time for Full Scan, Quick Scan & Custom Scan.
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
****************************************************************************/
void CWardWizQuickScan::SetLastScanDateTime()
{
	SYSTEMTIME  CurrTime = { 0 };

	TCHAR	szLastDtTime[256] = { 0 }, szTime[16] = { 0 };

	try
	{
		GetLocalTime(&CurrTime);
		_wstrtime_s(szTime, 15);

		swprintf_s(szLastDtTime, _countof(szLastDtTime), L"%d/%d/%d %s", CurrTime.wMonth, CurrTime.wDay, CurrTime.wYear, szTime);

		CITinRegWrapper	objReg;
		DWORD	dwRet = 0x00;

		if (!SendRegistryData2Service(SZ_STRING, theApp.m_csRegKeyPath.GetBuffer(),
			_T("LastScandt"), szLastDtTime, true))
		{
			AddLogEntry(L"### Failed to SET LastScandt CVibraniumQuickScan::SetLastScanDateTime", 0, 0, true, SECONDLEVEL);
		}
		Sleep(5);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::SetLastScanDateTime", 0, 0, true, SECONDLEVEL);
	}
}


/***********************************************************************************************
*  Function Name  : SendRegistryData2Service
*  Description    : Send request to service to set registry through pipe.
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*************************************************************************************************/
bool CWardWizQuickScan::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = dwType;
	wcscpy_s(szPipeData.szFirstParam, szKey);
	wcscpy_s(szPipeData.szSecondParam, szValue);
	if (szData)
	{
		wcscpy_s(szPipeData.szThirdParam, szData);
	}
	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CVibraniumQuickScan : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumQuickScan : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SendData2EPSClient
*  Description    :	Function which sends detected entries data to wardwiz EPS client
*  Author Name    : Jeena Mariam Saji
*  Date           : 25th Jan 2019
*  SR_NO		  : strVirusName, strScanFileName, csAction
**********************************************************************************************************/
bool CWardWizQuickScan::SendData2EPSClient(LPCTSTR szFilePath, LPCTSTR szVirusName, CString csTaskID, DWORD dwActionTaken)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SHOW_VIRUSENTRY;
		szPipeData.dwSecondValue = dwActionTaken;
		wcscpy_s(szPipeData.szFirstParam, csTaskID); //send here task ID
		wcscpy_s(szPipeData.szSecondParam, szVirusName);
		wcscpy_s(szPipeData.szThirdParam, szFilePath);

		CISpyCommunicator objCom(EPS_CLIENT_AGENT, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumQuickScan::SendData2EPSClient", 0, 0, true, SECONDLEVEL);
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::SendData2EPSClient, File: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : OnSetTimer
Description    : used to set timer
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
***********************************************************************************************/
json::value CWardWizQuickScan::OnSetTimer()
{
	try
	{
		theApp.m_bIsQuickScanUIReceptive = true;
		if (m_bIsMultiQScanFinish)
		{
			CallFinishScanFunction();
		}
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS_QUICK, 1000, NULL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::OnSetTimer", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***********************************************************************************************
Function Name  : On_CallContinueFullScan
Description    : used to continue Full scan
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
***********************************************************************************************/
json::value CWardWizQuickScan::On_CallContinueQuickScan()
{
	try
	{
		theApp.m_bIsQuickScanUIReceptive = false;
		theApp.m_bIsQScanPageSwitched = true;
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS_QUICK);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::On_CallContinueQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/**********************************************************************************************************
*  Function Name  :	CallFinishScanFunction
*  Description    :	Function to call Finish Status
*  Author Name    : Jeena Mariam Saji
*  Date           : 11th Feb 2019
**********************************************************************************************************/
void CWardWizQuickScan::CallFinishScanFunction()
{
	try
	{
		CString csCurrentFileCount;
		csCurrentFileCount.Format(L"%d", m_FileScanned);
		CString csVirusCount;
		csVirusCount.Format(L"%d", m_dwVirusFoundCount);

		sciter::value map;
		map.set_item("one", sciter::string(csCurrentFileCount));
		map.set_item("two", sciter::string(csVirusCount));

		//Send here event
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETSCANFINISHED_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CallFinishScanFunction", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	On_ScanFinishedCall
*  Description    :	Accept GetScanFinishedFlag call input from sciter
*  Author Name    : Kunal Waghmare
*  Date           : 4th Apr 2019
**********************************************************************************************************/
json::value CWardWizQuickScan::On_ScanFinishedCall(SCITER_VALUE svGetScanFinishedFlag)
{
	try
	{
		m_svGetScanFinishedFlag = svGetScanFinishedFlag;;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::On_ScanFinishedCall", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetQuickScanStatus
*  Description    :	To get other scan status
*  Author Name    : Akshay Patil
*  Date           : 21 Nov 2019
**********************************************************************************************************/
json::value CWardWizQuickScan::GetQuickScanStatus(SCITER_VALUE svIsScanRunning)
{
	try
	{
		m_svIsScanRunning = svIsScanRunning;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::GetQuickScanStatus", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	CheckIsScanShutdown
*  Description    :	Check if any scan after shut down ticked
*  Author Name    : Akshay Patil
*  Date           : 21 Nov 2019
**********************************************************************************************************/
bool CWardWizQuickScan::CheckIsScanShutdown()
{
	bool bReturn = false;
	try
	{
		SCITER_VALUE result = m_svIsScanRunning.call();
		if (result == false)
		{
			bReturn = false;
		}
		else
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumQuickScan::CheckIsScanShutdown", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
