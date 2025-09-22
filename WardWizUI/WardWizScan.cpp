/*********************************************************************
*  Program Name: CWardWizScan.cpp
*  Description: CWardWizScan Implementation
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28 March 2016
*  Version No: 2.0.0.1
**********************************************************************/
#include "stdafx.h"
#include <Psapi.h>
#include "WardWizScan.h"
#include "WrdwizEncDecManager.h"
#include "WardWizUIDlg.h"
#include "PipeConstants.h"
#include "Enumprocess.h"
#include "WrdWizSystemInfo.h"

#define		SETFILEPATH_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 1)
#define		SETFILECOUNT_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 2)
#define		SETSCANFINISHED_EVENT_CODE		(FIRST_APPLICATION_EVENT_CODE + 16)
#define		RESTART_YES						L"YES"
#define		RESTART_NO						L"NO"
#define		WRDWIZBOOTSCN					L"VBBOOTSCN.EXE"
#define		WRDWIZEXCLUDECLONEDB			L"VBEXCLUDECLONE.DB"

DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam);
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam);
DWORD WINAPI QuarantineThread(LPVOID lpParam);
UINT PlayScanFinishedThread(LPVOID lpThis);
UINT PlayThreatsFoundThread(LPVOID lpThis);


BOOL					g_bIsScanning;

CString					g_csPreviousFile = L"";
CString					g_csPreviousFilePath = L"";
CString					g_csPreviousStatus = L"";
CString					g_csPreviousVirusFoundPath = L"";

CWardWizScan::CWardWizScan():	behavior_factory("WardWizScan")
,  m_objIPCClient(FILESYSTEMSCAN)
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
, m_bIsMultiCScanFinish(false)
, m_objSqlDb(STRDATABASEFILE)
{
	m_objIPCClient.OpenServerMemoryMappedFile();
	m_objIPCClientVirusFound.OpenServerMemoryMappedFile();
	EnumProcessModulesWWizEx = NULL;
	m_hPsApiDLL = NULL;
	LoadPSAPILibrary();
	m_hScanDllModule = NULL;
}

CWardWizScan::~CWardWizScan()
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
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
json::value CWardWizScan::On_StartCustomScan(SCITER_VALUE svArrCustomScanSelectedEntries, SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB, SCITER_VALUE svFunNotificationMessageCB)
{
	try
	{
		theApp.m_bIsScanning = true;
		theApp.m_bCustomScan = true;
		m_bIsManualStop = false;
		m_bStop = false;
		m_bIsManualStopScan = false;
		m_bIsMultiCScanFinish = false;
		m_eCurrentSelectedScanType = CUSTOMSCAN;
		m_svArrCustomScanSelectedEntries = svArrCustomScanSelectedEntries;
		m_svAddVirusFoundEntryCB = svFunAddVirusFoundEntryCB;
		m_svSetScanFinishedStatusCB = svFunSetScanFinishedStatusCB;
		m_svFunNotificationMessageCB = svFunNotificationMessageCB;
		AddDeleteEditCustomScanEntryToINI(m_svArrCustomScanSelectedEntries);
		StartScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::On_StartCustomScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}


/***************************************************************************************************
*  Function Name  : On_PasueCustomScan
*  Description    : Accepts the request from UI and Pasue Custom scan
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
json::value CWardWizScan::On_PauseCustomScan(SCITER_VALUE svFunPauseResumeFunCB)
{
	try
	{
		theApp.m_bIsCustomScanUIReceptive = false;
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS);
		}

		m_objSqlDb.Close();

		if (theApp.m_bCustomScan)
		{
			m_svSetPauseStatusCB = svFunPauseResumeFunCB;
			PauseScan();
			theApp.m_bCustomScan = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::On_PasueResumeCustomScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_PasueCustomScan
*  Description    : Accepts the request from UI and Pasue Custom scan
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
json::value CWardWizScan::On_ResumeCustomScan(SCITER_VALUE svFunPauseResumeFunCB)
{
	try
	{
		theApp.m_bIsCustomScanUIReceptive = true;
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS, 1000, NULL);
		}

		m_objSqlDb.Close();

		if (!theApp.m_bCustomScan)
		{
			m_svSetPauseStatusCB = svFunPauseResumeFunCB;
			ResumeScan();
			theApp.m_bCustomScan = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::On_PasueResumeCustomScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***********************************************************************************************
Function Name  : On_StopCustomScan
Description    : used to stop Custom scan
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 2nd May 2016
***********************************************************************************************/
json::value CWardWizScan::On_StopCustomScan(SCITER_VALUE svbIsManualStop)
{
	try
	{
		m_bIsManualStop = svbIsManualStop.get(false);
		m_bIsManualStopScan = true;
		StopScan();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::On_StopCustomScan", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : On_ClickCustomScanCleanBtn
Description    : Cleans detected virus entries
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 2nd May 2016
***********************************************************************************************/
json::value CWardWizScan::On_ClickCustomScanCleanBtn(SCITER_VALUE svArrCleanEntries, SCITER_VALUE svFunSetVirusUpdateEntryCB)
{
	try
	{
		OnClickCleanButton(svArrCleanEntries, svFunSetVirusUpdateEntryCB);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::On_ClickCustomScanCleanBtn", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	InsertSQLiteData
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 CWardWizScan::InsertSQLiteData(const char* szQuery)
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
		AddLogEntry(L"### Exception in CWardwizScan::InsertSQLiteData.", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	StartScanning
*  Description    :	To start Full scan ,custom scan and quick scan accoeding m_scantype variable.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CWardWizScan::StartScanning()
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

		if (m_bQuickScan ==  true)
		{
			m_eScanType = QUICKSCAN;
			GetModuleCount();
		}

		m_hThread_ScanCount = ::CreateThread(NULL, 0, GetTotalScanFilesCount, (LPVOID) this, 0, NULL);
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
		m_hWardWizAVThread = ::CreateThread(NULL, 0, EnumerateThread, (LPVOID) this, 0, &m_dwThreadId);
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
			SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS, 1000, NULL);  // call OnTimer function
		}
		theApp.m_bIsCustomScanUIReceptive = true;
		AddLogEntry(csScanStarted, 0, 0, true, SECONDLEVEL);
		m_tsScanStartTime = CTime::GetCurrentTime();
		m_tsScanPauseResumeElapsedTime -= m_tsScanPauseResumeElapsedTime;
		sciter::dom::element(self).start_timer(500);
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CScanDlg::StartScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetScanningPaths
*  Description    :	Get scan path according to scanning types.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::GetScanningPaths(CStringArray &csaReturn)
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
		AddLogEntry(L"### Exception in CWardwizScan::GetScanningPaths", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************
*  Function Name	 :	Check4DBFiles
*  Description		 :	Checks for signature db are exist or not
*  Author Name		 : Ramkrushna Shelke
*  SR_NO		   	 :
*  Date				 : 26 Nov 2013
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
**********************************************************************************************************/
bool CWardWizScan::Check4DBFiles()
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
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::StartScanUsingService(CStringArray &csaAllScanPaths)
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
			AddLogEntry(L"### Failed to send data in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		else
		{
			g_bIsScanning = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	GetAllDrivesList
*  Description    :	Makes list of drives present on a system.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::GetAllDrivesList(CStringArray &csaReturn)
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
*  Author Name    :	Varada Ikhar
*  SR_NO		  :
*  Date           : 20 Mar 2015
**********************************************************************************************************/
bool CWardWizScan::Check4ValidDBFiles(CString csDBFolderPath)
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
		AddLogEntry(L"### Exception in CScanDlg::Check4ValidDBFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	MakeFullTokenizedScanPath
*  Description    :	Toknization of scan path
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::MakeFullTokenizedScanPath(CStringArray &csaAllScanPaths, LPTSTR szScanPath)
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
		AddLogEntry(L"### Exception in CScanDlg::MakeFullTokenizedScanPath", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : PauseScan
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    : 
*  SR_NO		  :
*  Date           : 29/04/2015
*************************************************************************************************/
bool CWardWizScan::PauseScan()
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
		AddLogEntry(L"### Exception in CWardwizScan::PauseScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SendRequestCommon
*  Description    :	Send a pause,stop,resume scanning request to comm service.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::SendRequestCommon(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to SERVICE_SERVER in CScanDlg::SendRequestCommon", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : ResumeScan
*  Description    : Resume scanning if user click on stop/close button and click to 'No' for stop confirmation box.
*  Author Name    : 
*  SR_NO		  :
*  Date           : 29/04/2015
*************************************************************************************************/
bool CWardWizScan::ResumeScan()
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
		AddLogEntry(L"### Exception in CWardwizScan::ResumeScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : callUISetStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
void CWardWizScan::CallUISetStatusfunction(LPTSTR lpszPath)
{
	__try
	{
		CallUISetStatusfunctionSEH(lpszPath);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScan::CallUISetStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallUISetStatusfunctionSEH
*  Description    : Calls call_function to invoke ant UI function
Note: No need to add exception handling because 
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
void CWardWizScan::CallUISetStatusfunctionSEH(LPTSTR lpszPath)
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
*  Author Name    : Nitin Kolapkar
*  Date			  : 21 April 2016
****************************************************************************************************/
void CWardWizScan::CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID)
{
	try
	{
		m_svAddVirusFoundEntryCB.call(SCITER_STRING(csVirusName), SCITER_STRING(csFilePath), SCITER_STRING(csActionTaken), SCITER_STRING (SpyID));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::CallUISetVirusFoundEntryfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetPauseStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
void CWardWizScan::CallUISetPauseStatusfunction(CString csData)
{
	try
	{
		m_svSetPauseStatusCB.call(SCITER_STRING(csData));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::CallUISetPauseStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetScanFinishedStatus
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
void CWardWizScan::CallUISetScanFinishedStatus(CString csData)
{
	try
	{
		if (theApp.m_bIsCustomScanUIReceptive)
		{
			CWnd *pwnd = theApp.m_pMainWnd;
			if (pwnd != NULL)
			{
				KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS);
			}

			if (!theApp.m_bIsCScanPageSwitched)
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
		AddLogEntry(L"### Exception in CWardwizScan::CallUISetScanFinishedStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
void CWardWizScan::CallUISetFileCountfunction(CString csTotalFileCount, CString csCurrentFileCount)
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
		AddLogEntry(L"### Exception in CWardwizScan::callUIfunction", 0, 0, true, SECONDLEVEL);
	}
}

bool CWardWizScan::ScanFinished()
{
	CString csCompleteScanning;
	CString csFileScanCount;
	CString csMsgNoFileExist(L"");
	CString csCurrentFileCount; 
	m_bIsMultiCScanFinish = true;
	if (theApp.m_bIsCustomScanUIReceptive)
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
			AfxBeginThread(PlayScanFinishedThread, NULL);
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
		AddLogEntry(L"### Failed in Setting Registry CWardwizScan::ScanFinished", 0, 0, true, SECONDLEVEL);
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
				AfxBeginThread(PlayScanFinishedThread, NULL);
			}
	
			if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath,
				L"VirusFound", REG_DWORD, dwbVirusFound, false))
			{
				AddLogEntry(L"### Error in Set SetRegistrykeyUsingService VirusFound in CWardwizScan::QuaratineEntries", 0, 0, true, SECONDLEVEL);
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
*  Author Name			: Ram Shelke
*  Date					: 12 Apr 2016
*  SR_NO				:
**********************************************************************************************************/
bool CWardWizScan::SaveLocalDatabase()
{
	__try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SAVE_LOCALDB;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
		}

		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CUSBDetectUIDlg::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
		}

		if (szPipeData.dwValue == 1)
		{
			return true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CScanDlg::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	SetRegistrykeyUsingService
*  Description    :	Set any dword value into given key into registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
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
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	StopScan
*  Description    :	stops scanning.
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 2nd May 2016
**********************************************************************************************************/
void CWardWizScan::StopScan()
{
	try
	{
		ShutDownScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::StopScan", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ShutDownScanning
*  Description    :	Shut down scanning with terminating all thread safely.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::ShutDownScanning()
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
		AddLogEntry(L"### Exception in CWardwizScan::ShutDownScanning", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ShutDownScanning
*  Description    :	Shut down scanning with terminating all thread safely.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::OnClickCleanButton(SCITER_VALUE svArrayCleanEntries, SCITER_VALUE setVirusUpdateEntryFunCB)
{
	try
	{
		bool bIsArray = false;
		svArrayCleanEntries.isolate();
		bIsArray = svArrayCleanEntries.is_array();
		if (!bIsArray)
		{
			return false;
		}
		m_svSetVirusUpdateStatusCB = setVirusUpdateEntryFunCB;
		m_svArrayCleanEntries = svArrayCleanEntries;
		m_hQuarantineThread = ::CreateThread(NULL, 0, QuarantineThread, (LPVOID) this, 0, NULL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::OnClickCleanButton", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineThread
*  Description    :	If user clicks on clean button.Quarantine thread gets called.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
DWORD WINAPI QuarantineThread(LPVOID lpParam)
{
	try
	{
		CWardWizScan *pThis = (CWardWizScan *)lpParam;
		pThis->QuaratineEntries();
		return 1;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	QuaratineEntries
*  Description    :	Repaires or quarantines selected files one by one.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CWardWizScan::QuaratineEntries()
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
			AddLogEntry(L"### Error in CScanDlg::SendRecoverOperations2Service RELOAD_DBENTRIES RECOVER", 0, 0, true, SECONDLEVEL);
		}
		if (!SendRecoverOperations2Service(RELOAD_DBENTRIES, L"", REPORTS, true, true))
		{
			AddLogEntry(L"### Error in CScanDlg::SendFile4RepairUsingService RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
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
						AddLogEntry(L"### Error in CScanDlg::QuaratineEntries: File %s does not exist", csThreatPath, 0, true, SECONDLEVEL);
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
				AddLogEntry(L"### Error in Set SetRegistrykeyUsingService VirusFound in CScanDlg::QuaratineEntries", 0, 0, true, SECONDLEVEL);
			}
		}
		Sleep(5);
		//used 0 as Type for Saving RECOVER DB
		if (!SendFile4RepairUsingService(SAVE_RECOVERDB, L"", L"", 0, true, true))
		{
			AddLogEntry(L"### Error in CScanDlg::SendFile4RepairUsingService SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
		}

		Sleep(5);
		//used 0 as Type for Saving RECOVER DB
		if (!SendFile4RepairUsingService(SAVE_REPORTS_ENTRIES, L"", L"", 5, true, true))
		{
			AddLogEntry(L"### Error in CScanDlg::SendFile4RepairUsingService SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
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
				csErrorMsg.Format(L"### Failed to Terminate QuarantineThread in CScanDlg::CloseCleaning with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Terminated QuarantineEntries thread successfully.", 0, 0, true, FIRSTLEVEL);
			m_hQuarantineThread = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::QuaratineEntries", 0, 0, true, SECONDLEVEL);
	}
	AddLogEntry(L">>> Quarantine Finished..", 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"------------------------------------------------", 0, 0, true, SECONDLEVEL);
}

/**********************************************************************************************************
*  Function Name  :	SendRecoverOperations2Service
*  Description    :	Send a request to stored data into recover db.So that user can recover file.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry, DWORD dwType, bool bWait, bool bReconnect)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	szPipeData.iMessageInfo = dwMessageinfo;
	_tcscpy_s(szPipeData.szFirstParam, csRecoverFileEntry);
	szPipeData.dwValue = dwType;
	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CScanDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to Read data in CScanDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
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
*  Author Name    : Vilas Suvarnakar
*  Date           : 07 April 2015
**********************************************************************************************************/
bool CWardWizScan::SendFile4RepairUsingService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect)
{
	try
	{
		if (!m_objCom.SendData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (bWait)
		{
			if (!m_objCom.ReadData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CVibraniumRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::SendFile4RepairUsingService1", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SendFile4RepairUsingService
*  Description    :	Send request to clean file to service
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::SendFile4RepairUsingService(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait, bool bReconnect)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));
	szPipeData.iMessageInfo = iMessage;
	szPipeData.dwValue = dwISpyID;
	wcscpy_s(szPipeData.szFirstParam, csThreatPath);
	wcscpy_s(szPipeData.szSecondParam, csThreatName);
	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CScanDlg::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CScanDlg::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
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
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 15-Jan-2015
**********************************************************************************************************/
bool CWardWizScan::VirusFoundEntries(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (!lpSpyData)
			return false;

		CString csCurrentPath(L"");
		CString csStatusForReports = L"";
		csCurrentPath.Format(L"%s", lpSpyData->szSecondParam);
		if (csCurrentPath.GetLength() > 0 && (g_csPreviousVirusFoundPath != csCurrentPath))
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
					AfxBeginThread(PlayThreatsFoundThread, NULL);
				}
			}
			AddLogEntry(L">>> Virus Found GUI: %s, File Path: %s", lpSpyData->szFirstParam, csCurrentPath, true, SECONDLEVEL);
		}
		g_csPreviousVirusFoundPath.SetString(csCurrentPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::VirusFoundEntries", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SaveDBEntries
*  Description    :	Save recover entries in DB folder
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardWizScan::SaveDBEntries()
{
	try
	{
		AddLogEntry(L"CWardwizScan::SaveDBEntries() entered!!", 0, 0, true, ZEROLEVEL);
		if (!SendFile4RepairUsingService(SAVE_RECOVERDB, L"", L"", 0, true, true))
		{
			AddLogEntry(L"### Error in CScanDlg::SaveDBEntries SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
			return false;
		}
		Sleep(5);
		if (!SendFile4RepairUsingService(SAVE_REPORTS_ENTRIES, L"", L"", 5, true, true))
		{
			AddLogEntry(L"### Error in CWardwizScan::SaveDBEntries SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::SaveDBEntries.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : LoadCutomScanINI
*  Description    : Loads the entry for last custom scan from INI file
*  Author Name    : Nitin K.
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
json::value CWardWizScan::LoadCutomScanINI(SCITER_VALUE svFillCustomScanArrayCB)
{
	try
	{
		CString sz_Count = L"";
		TCHAR	m_szCustomCount[256];
		TCHAR	m_szCustomINIPath[256];
		int		m_iCustomCount;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L"### CWardwizScan::AddDeleteEditCustomScanEntryToINI::GetModulePath failed", 0, 0, true, SECONDLEVEL);
			return 0;
		}
		ZeroMemory(m_szCustomINIPath, sizeof(m_szCustomINIPath));
		swprintf_s(m_szCustomINIPath, _countof(m_szCustomINIPath), L"%s\\VBSETTINGS\\VibraniumCUSTOMSCANENTRY.INI", szModulePath);
		ZeroMemory(m_szCustomCount, sizeof(m_szCustomCount));
		GetPrivateProfileString(L"Custom Scan Entry", L"Count", L"", m_szCustomCount, 255, m_szCustomINIPath);
		m_iCustomCount = 0;
		m_iCustomCount = _ttoi(m_szCustomCount);
		for (int i = 1; i <= m_iCustomCount; i++)
		{
			sz_Count = L"";
			sz_Count.Format(L"%d", i);
			GetPrivateProfileString(L"Custom Scan Entry", sz_Count, L"", m_szCustomCount, 255, m_szCustomINIPath);
			if (PathFileExists(m_szCustomCount))
			{
				svFillCustomScanArrayCB.call(m_szCustomCount);
			}
		}
		return  0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::LoadCutomScanINI", 0, 0, true, SECONDLEVEL);
	}
	return  0;
}

/***********************************************************************************************
*  Function Name  : AddDeleteEditCustomScanEntryToINI
*  Description    : Add the selected entries of custom scan into INI file for next time use
*  Author Name    : Nitin K.
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
json::value CWardWizScan::AddDeleteEditCustomScanEntryToINI(SCITER_VALUE svArrCustomScanEntries)
{
	try
	{
		bool bIsArray = false;
		svArrCustomScanEntries.isolate();
		bIsArray = svArrCustomScanEntries.is_array();
		if (!bIsArray)
		{
			return 0;
		}
		DWORD dwFailedDeleteFileCount = 0;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L"### CWardwizScan::AddDeleteEditCustomScanEntryToINI::GetModulePath failed", 0, 0, true, SECONDLEVEL);
			return 0;
		}
		CString csCustomScanINIPath = szModulePath;
		csCustomScanINIPath += L"\\VBSETTINGS";
		TCHAR szValueName[260] = { 0 };
		CString csCustomScanEntryINIPath(L"");

		csCustomScanEntryINIPath.Format(L"%s\\VibraniumCUSTOMSCANENTRY.INI", csCustomScanINIPath);
		DeleteFile(csCustomScanEntryINIPath);
		if (!PathFileExists(csCustomScanINIPath))
		{
			if (!CreateDirectory(csCustomScanINIPath, NULL))
			{
				AddLogEntry(L"### CWardwizScan::Create VBSETTINGS directory failed", 0, 0, true, SECONDLEVEL);
			}
		}
		dwFailedDeleteFileCount = 0;
		for (int iCount = 0; iCount < svArrCustomScanEntries.length(); iCount++)
		{
			const SCITER_VALUE EachEntry = svArrCustomScanEntries[iCount];
			const std::wstring chFilePah = EachEntry[L"FilePath"].get(L"");
			CString szStr = L"";
			szStr = chFilePah.c_str();
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", ++dwFailedDeleteFileCount);
			WritePrivateProfileString(L"Custom Scan Entry", szValueName, szStr, csCustomScanEntryINIPath);
		}
		ZeroMemory(szValueName, sizeof(szValueName));
		swprintf_s(szValueName, _countof(szValueName), L"%lu", svArrCustomScanEntries.length());
		WritePrivateProfileString(L"Custom Scan Entry", L"Count", szValueName, csCustomScanEntryINIPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::AddDeleteEditCustomScanEntryToINI", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetWardwizRegistryDetails
*  Description    :	Read Scanning related options from registry
*  Author Name    : Gayatri A.
*  SR_NO		  :
*  Date           : 13 Sep 2016
**********************************************************************************************************/
bool CWardWizScan::GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt)
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
			AddLogEntry(L"### Failed to get registry key value for Quarantine Option in CWardwizScan::GetWardwizRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
		}

		ReadReg = RegQueryValueEx(hKey, L"dwHeuScan", NULL, &dwType, (LPBYTE)&dwHeuScanOpt, &dwOptionSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to get registry key value for Heuristic Scan Option in CWardwizScan::GetWardwizRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::GetWardwizRegistryDetails", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
/**********************************************************************************************************
*  Function Name  :	PlayScanFinishedThread
*  Description    :	Thread will play a sound when scan gets finished.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 05 Aug 2016
**********************************************************************************************************/
UINT PlayScanFinishedThread(LPVOID lpThis)
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
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 05 Aug 2016
**********************************************************************************************************/
UINT PlayThreatsFoundThread(LPVOID lpThis)
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
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 05 Aug 2016
**********************************************************************************************************/
bool CWardWizScan::ReadUISettingFromRegistry()
{
	try
	{ 
		HKEY key;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, &key) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to open registry key in CWardwizScan::ReadUISettingFromRegistry, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScan::ReadUISettingFromRegistry", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetModuleCount
*  Description    :	Give total modules of proesses in the case of quick scan
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizScan::GetModuleCount()
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
		AddLogEntry(L"### Exception in CWardwizScan::GetModuleCount", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : IsDuplicateModule
Description    : Function to find duplicates modules to avoid multiple scanning.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 24th Jan 2015
****************************************************************************/
bool CWardWizScan::IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize)
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
		AddLogEntry(L"### Exception in CClamScanner::IsDuplicateModule", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : LoadPSAPILibrary
Description    : Load PSAPI.DLL.
For Issue: In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 04th Feb 2015
****************************************************************************/
void CWardWizScan::LoadPSAPILibrary()
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
		AddLogEntry(L"### Exception in CWardwizScan::LoadPSAPILibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTotalScanFilesCount
Description    : Get total files count in case of fullscan and custom scan
Author Name    : Neha Gharge.
Date           : 4 Dec 2015
SR_NO			 :
****************************************************************************/
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam)
{
	try
	{
		CWardWizScan *pThis = (CWardWizScan*)lpParam;
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
		AddLogEntry(L"### Exception in CWardwizScan::GetTotalScanFilesCount", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : EnumerateThread
Description    : Thread to enumerarte process in case of quick scan and
files and folders in case of custom and full scan.
Author Name    : Neha Gharge.
Date           : 4 Dec 2014
SR_NO			 :
****************************************************************************/
DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizScan *pThis = (CWardWizScan*)lpvThreadParam;
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
			pThis->EnumFolderForScanning(csPath);
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
		AddLogEntry(L"### Exception in CWardwizScan::EnumerateThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************            *  Function Name  :	EnumerateProcesses
*  Description    :	Enumaerate processes in case of quick scan.
Changes (Ram) : Time complexity decresed as we enumerating processes and modules
to calculate file count, There is no need to enumerate it again.
kept in CStringList while file count calculation, same list is used again.
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizScan::EnumerateProcesses()
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
				ScanForSingleFile(csProcessPath);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::EnumerateProcesses", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	EnumFolderForScanning
*  Description    :	enumerate files of system and sent it to scan.
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizScan::EnumFolderForScanning(LPCTSTR pstr)
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

			ScanForSingleFile(pstr);
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

				EnumFolderForScanning(str);
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

						ScanForSingleFile(csFilePath);
					}
				}
			}
			Sleep(10);
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::EnumFolderForScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ScanForSingleFile
*  Description    :	Scan each single file .
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizScan::ScanForSingleFile(CString csFilePath)
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
							bool  bHeuristicOpt	 =false;
							
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
								AddLogEntry(L"### Failed to SendData to EPS Client in CWardwizScan::EnumFolder", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in void CWardwizScan::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
	}
}


/**********************************************************************************************************
*  Function Name  :	GetActionIDFromAction
*  Description    : Get action Id as stored in ini file from the action text.This will be used to
					store in DB.Will help in multi language support.
*  Author Name    : Gayatri A.
*  SR_NO		  :
*  Date           : 15 Sep 2016
**********************************************************************************************************/
CString GetActionLangIDFromAction(CString csMessage)
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
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizScan::AddEntriesInReportsDB(SCANTYPE eScanType, CString csThreatName, CString csFilePath, CString csAction)
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
		CString csActionID = GetActionLangIDFromAction(csAction);
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
		AddLogEntry(L"### Exception in CWardwizScan::AddEntriesInReportsDB", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInRepairRebootIni
*  Description    :	Get Quarantine folder path.
*  Author Name    : Vilas Suvarnakar
*  Date           : 24 March 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizScan::CheckFileIsInRepairRebootIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };
		if (GetQuarantineFolderPath(szRepairIniPath))
		{
			AddLogEntry(L"### Failed in CWardwizScan::CheckFileIsInRepairRebootIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScan::CheckFileIsInRepairRebootIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Vilas Suvarnakar
*  Date           : 24 March 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizScan::GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath)
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
		AddLogEntry(L"### Exception in CWardwizScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : Checking file entry present in Recover ini (WWRecover.ini)
Description    : Parses WWRecover.ini and not sends to scan if file is found.
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 27 March 2015
***********************************************************************************************/
bool CWardWizScan::CheckFileIsInRecoverIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR	szRecoverIniPath[MAX_PATH] = { 0 };
		if (GetQuarantineFolderPath(szRecoverIniPath))
		{
			AddLogEntry(L"### Failed in CheckFileIsInRecoverIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScan::CheckFileIsInRecoverIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInDeleteIni
*  Description    :	Check whether file available in delete.ini
*  Author Name    : Neha Gharge
*  Date           : 10 April 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizScan::CheckFileIsInDeleteIni(CString csQurFilePaths)
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
				AddLogEntry(L"### CheckFileIsInDeleteIni::TokenizationOfParameterForrecover is not tokenize properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}
			if (!TokenizeIniData(szFileName, szApplnName, _countof(szApplnName)))
			{
				AddLogEntry(L"### CWardwizScan::CheckFileIsInDeleteIni::TokenizeIniData is not tokenize properly", 0, 0, true, FIRSTLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScan::CheckFileIsInDeleteIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
CString CWardWizScan::GetQuarantineFolderPath()
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
		AddLogEntry(L"### Exception in CWardwizScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***********************************************************************************************
Function Name  : TokenizationOfParameterForrecover
Description    : Tokenize input path to get file name, quarantine name and virus name
SR.NO		   :
Author Name    : Neha Gharge
Date           : 8 April 2015
***********************************************************************************************/
bool CWardWizScan::TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName)
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
		AddLogEntry(L"### Exception in CWardwizScan::TokenizationOfParameterForrecover", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	TokenizeIniData
*  Description    :	Tokenization of entries from delete file ini
*  Author Name    : Neha Gharge
*  Date           : 26 Feb 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizScan::TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName)
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
		AddLogEntry(L"### Exception in CWardwizScan::TokenizeIniData", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	EnumFolder
*  Description    :	Enumerate each folders of system and calculate total files count.
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizScan::EnumFolder(LPCTSTR pstr)
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
		AddLogEntry(L"### Exception in CWardwizScan::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ScanFile
*  Description    :	Exported funtion to scan file
Scanning using service, the aim is to keep the database in common memory location.
*  Author Name    : Neha Gharge, Ram Shelke
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0039
**********************************************************************************************************/
bool CWardWizScan::ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan)
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
			AddLogEntry(L"### Failed to send data in CWardwizScan::ScanFile", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}
		if (!m_objScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CWardwizScan::ScanFile", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScan::ScanFile, File: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	CheckForNetworkPath
*  Description    :	To check weather file is network file or not.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 20 Aug 2016
**********************************************************************************************************/
json::value CWardWizScan::CheckForNetworkPath(SCITER_VALUE svFilePath)
{
	try
	{
		const std::wstring  chFilePath = svFilePath.get(L"");
		if (PathIsNetworkPath((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::CheckForNetworkPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
*  Function Name  : CheckForDeleteFileINIEntries
*  Description    : check whether delete file ini present or not
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 23 Aug 2016
*************************************************************************************************/
DWORD CWardWizScan::CheckForDeleteFileINIEntries()
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
		AddLogEntry(L"### Exception in CWardwizScan::CheckForDeleteFileINIEntries()", 0, 0, true, SECONDLEVEL);
	}
	return dwCount;
}

/***********************************************************************************************
*  Function Name  : CheckForRepairFileINIEntries
*  Description    : check whether repair file ini present or not
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 10 April 2015
*************************************************************************************************/
DWORD CWardWizScan::CheckForRepairFileINIEntries()
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
		AddLogEntry(L"### Exception in CWardwizScan::CheckForRepairFileINIEntries()", 0, 0, true, SECONDLEVEL);
	}
	return dwCount;
}

/***************************************************************************************************
Function Name  : CallNotificationMessage
Description    : Calls Light box on UI
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Sept 2016
/***************************************************************************************************/
void CWardWizScan::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
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
		AddLogEntry(L"### Exception in CWardwizScan::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : on_timer
Description    : On timer
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 8th Dec 2016
/***************************************************************************************************/
bool CWardWizScan::on_timer(HELEMENT he, UINT_PTR extTimerId)
{
	try
	{
		/*CallUISetStatusfunction(m_csCurrentFilePath);
		CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
		CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
		CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
		m_csPreviousPath = m_csCurrentFilePath;
		AddLogEntry(L"### Called CNextGenExScan::on_timer()", 0, 0, true, SECONDLEVEL);*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::on_timer(HELEMENT he, UINT_PTR extTimerId)", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : on_timer
Description    : On timer
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 8th Dec 2016
/***************************************************************************************************/
bool CWardWizScan::on_timer(HELEMENT he)
{
	__try
	{
		return  on_timerSEH(he);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScan::on_timer", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : on_timerSEH
Description    : On timer
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 8th Dec 2016
/***************************************************************************************************/
bool CWardWizScan::on_timerSEH(HELEMENT he)
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
Author Name    : Amol j.
SR_NO		   :
Date           : 5th Dec 2017
/***************************************************************************************************/
void CWardWizScan::OnTimerScan()
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
		AddLogEntry(L"### Exception in CWardwizScan::OnTimerScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : CheckFileOrFolderOnRootPath
Description    : Function which check IS file/Folder present on root path
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 25th Apr 2017
/*************************s**************************************************************************/
bool CWardWizScan::CheckFileOrFolderOnRootPath(CString csFilePath)
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
		AddLogEntry(L"### Exception in CWardwizScan::CheckFileOrFolderOnRootPath, Path: %s", csFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
Function Name  : CheckFileOrFolderOnRootPath
Description    : Function which sends bool value of shutdown checkbox.
Author Name    : Amol J.
SR_NO		   :
Date           : 09h June 2017
/*************************s**************************************************************************/
json::value CWardWizScan::On_ClickShutDwonScan(SCITER_VALUE svIsShutDownPC)
{
	m_bIsShutDownPC = svIsShutDownPC.get(false);
	return 0;
}

/***************************************************************************
Function Name  : SetLastScanDateTime( )
Description    : Sets the Scan Date & time for Full Scan, Quick Scan & Custom Scan.
Author Name    : Niranjan Deshak.
S.R. No        :
Date           : 4/2/2015
****************************************************************************/
void CWardWizScan::SetLastScanDateTime()
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
			AddLogEntry(L"### Failed to SET LastScandt CWardwizScan::SetLastScanDateTime", 0, 0, true, SECONDLEVEL);
		}
		Sleep(5);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::SetLastScanDateTime", 0, 0, true, SECONDLEVEL);
	}
}


/***********************************************************************************************
*  Function Name  : SendRegistryData2Service
*  Description    : Send request to service to set registry through pipe.
*  Author Name    : Neha Gharge,Ram krushna
*  SR_NO		  :
*  Date           : 4-Feb-2015
*************************************************************************************************/
bool CWardWizScan::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait)
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
		AddLogEntry(L"### Failed to send data in CWardwizScan : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizScan : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : On_ClickSetScanID
*  Description    : Write scan value in ProductSettings.ini file.
*  Author Name    : Amol Jaware
*  Date           : 28-Sep-2017
*************************************************************************************************/
json::value CWardWizScan::On_ClickSetScanID(SCITER_VALUE svScanTypeID, SCITER_VALUE svResult)
{
	try
	{
		CString csExePath, csIniFilePath;
		CString csScanTypeID(svScanTypeID.to_string().c_str());
		CString csResult(svResult.to_string().c_str());
		CString csAppPath = theApp.m_AppPath;
		csExePath = csIniFilePath = csAppPath;
		csIniFilePath += L"VBSETTINGS";
		csIniFilePath += L"\\ProductSettings.ini";
		DWORD dwQuarantineOpt = 0x00;
		CITinRegWrapper objReg;

		WritePrivateProfileString(L"VBSETTINGS", L"BootTimeScan ", csScanTypeID, csIniFilePath);

		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwQuarantineOption", dwQuarantineOpt) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwQuarantineOption in CWardwizScan::On_ClickSetScanID, KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		if (dwQuarantineOpt == 0)
		{
			WritePrivateProfileString(L"VBSETTINGS", L"AutoQuarOption ", L"0", csIniFilePath);
		}
		else if (dwQuarantineOpt == 1)
		{
			WritePrivateProfileString(L"VBSETTINGS", L"AutoQuarOption ", L"1", csIniFilePath);
		}
		else if (dwQuarantineOpt == 2)
		{
			WritePrivateProfileString(L"VBSETTINGS", L"AutoQuarOption ", L"2", csIniFilePath);
		}


		CString csExcludeDBPath = csAppPath;
		csExcludeDBPath += WRDWIZEXCLUDECLONEDB;

		if (!PathFileExists(csExcludeDBPath))
		{
			//Create here exclude DB if not created.
			CExcludeFilesFolders objExcludeDB;
			if (!objExcludeDB.SaveEntriesInDB())
			{
				AddLogEntry(L"### Failed to SaveEntriesInDB in CWardwizScan::On_ClickSetScanID", 0, 0, true, SECONDLEVEL);
			}
		}

		csExePath += WRDWIZBOOTSCN;

		int iLength = GetShortPathName(csExePath, NULL, 0);
		if (iLength == 0x00)
		{
			AddLogEntry(L"### Failed GetShortPathName in CWardwizScan::On_ClickSetScanID", 0, 0, true, SECONDLEVEL);
			return 0;
		}
		
		TCHAR szShortPath[MAX_PATH] = { 0 };
		if (GetShortPathName(csExePath, szShortPath, iLength) == 0x00)
		{
			AddLogEntry(L"### Failed GetShortPathName in CWardwizScan::On_ClickSetScanID", 0, 0, true, SECONDLEVEL);
			return 0;
		}

		CISpyCommunicator objCom(SERVICE_SERVER, true, 2);
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		
		szPipeData.iMessageInfo = BOOT_SCANNER_OPR;
		szPipeData.dwValue = 0x01;
		wcscpy_s(szPipeData.szFirstParam, szShortPath);
		
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardwizScan::On_ClickSetScanID", 0, 0, true, SECONDLEVEL);
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardwizScan::On_ClickSetScanID", 0, 0, true, SECONDLEVEL);
		}

		if (csResult == "NULL")
			return 0;

		if (csResult == RESTART_YES)
		{
			CEnumProcess enumproc;
			enumproc.RebootSystem(0);
			return 0;
		}
		else if (csResult == RESTART_NO)
		{
			return 0;
		}
	}
	catch (...)
	{
		AddLogEntry(L"#### exception in CWardwizScan::On_ClickSetScanID", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***********************************************************************************************
*  Function Name  : On_ClickSetScanID
*  Description    : Get BootTimeScan value from ProductSettings.ini file.
*  Author Name    : Amol Jaware
*  Date           : 15-Nov-2017
*************************************************************************************************/
json::value CWardWizScan::On_ClickGetScanID()
{
	try
	{
		CString csIniFilePath;
		CString csAppPath = theApp.m_AppPath;
		csIniFilePath = csAppPath;
		csIniFilePath += L"VBSETTINGS";
		csIniFilePath += L"\\ProductSettings.ini";
		TCHAR m_szCustomCount[256];
		int m_iCustomCount = 0;

		ZeroMemory(m_szCustomCount,sizeof(m_szCustomCount));
		GetPrivateProfileString(L"VBSETTINGS", L"BootTimeScan", L"", m_szCustomCount, 255, csIniFilePath);

		m_iCustomCount = _ttoi(m_szCustomCount);
		if (m_iCustomCount == 1)
			return 1;
		else if (m_iCustomCount == 2)
			return 2;
		else
			return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::On_ClickGetScanID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	SendData2EPSClient
*  Description    :	Function which sends detected entries data to wardwiz EPS client
*  Author Name    : Ram Shelke
*  Date           : 9th March 2018
*  SR_NO		  : strVirusName, strScanFileName, csAction
**********************************************************************************************************/
bool CWardWizScan::SendData2EPSClient(LPCTSTR szFilePath, LPCTSTR szVirusName, CString csTaskID, DWORD dwActionTaken)
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
			AddLogEntry(L"### Failed to send data in CWardwizScan::SendData2EPSClient", 0, 0, true, SECONDLEVEL);
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::SendData2EPSClient, File: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	OnSetTimer
*  Description    :	Function to set timer on showing UI
*  Author Name    : Jeena Mariam Saji
*  Date           : 11th Feb 2019
**********************************************************************************************************/
json::value CWardWizScan::OnSetTimer()
{
	try
	{
		theApp.m_bIsCustomScanUIReceptive = true;
		if (m_bIsMultiCScanFinish)
		{
			CallFinishScanFunction();
		}
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS, 1000, NULL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::OnSetTimer", 0, 0, true, SECONDLEVEL);
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
json::value CWardWizScan::On_CallContinueCustomScan()
{
	try
	{
		theApp.m_bIsCustomScanUIReceptive = false;
		theApp.m_bIsCScanPageSwitched = true;
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::On_StopFullScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : CallFinishScanFunction
Description    : Called when scan is finished
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 25th Jan 2019
***********************************************************************************************/
void CWardWizScan::CallFinishScanFunction()
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
		AddLogEntry(L"### Exception in CWardwizScan::CallFinishScanFunction", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	On_ScanFinishedCall
*  Description    :	Accept GetScanFinishedFlag call input from sciter
*  Author Name    : Kunal Waghmare
*  Date           : 4th Apr 2019
**********************************************************************************************************/
json::value CWardWizScan::On_ScanFinishedCall(SCITER_VALUE svGetScanFinishedFlag)
{
	try
	{
		m_svGetScanFinishedFlag = svGetScanFinishedFlag;;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::On_ScanFinishedCall", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetCustomScanStatus
*  Description    :	To get other scan status
*  Author Name    : Akshay Patil
*  Date           : 21 Nov 2019
**********************************************************************************************************/
json::value CWardWizScan::GetCustomScanStatus(SCITER_VALUE svIsScanRunning)
{
	try
	{
		m_svIsScanRunning = svIsScanRunning;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::GetCustomScanStatus", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	CheckIsScanShutdown
*  Description    :	Check if any scan after shut down ticked
*  Author Name    : Akshay Patil
*  Date           : 21 Nov 2019
**********************************************************************************************************/
bool CWardWizScan::CheckIsScanShutdown()
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
		AddLogEntry(L"### Exception in CWardwizScan::CheckIsScanShutdown", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}