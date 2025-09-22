/*********************************************************************\\WRDWIZALLREPORTS.DB
*  Program Name: WardWizAntirootkit.cpp
*  Description: Antirootkit Implementation
*  Author Name: Sukanya Indurkar
*  Date Of Creation: 10th May 2016
*  Version No: 2.0.0.1
**********************************************************************/
#include "stdafx.h"
#include "WardWizAntirootkit.h"
#include "WardWizDatabaseInterface.h"
#include "WardWizReports.h"

DWORD WINAPI GetPercentage(LPVOID lpvThreadParam);
DWORD WINAPI DeleteThread(LPVOID lpParam);
bool  g_bIsRootkitScanning;

#define		SETSCANSTATUS_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 20)
#define		SETSCANCOUNT_EVENT_CODE				(FIRST_APPLICATION_EVENT_CODE + 21)
#define		SETSCANFINISHED_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 22)
#define DELETEDSTATUS			L"Quarantined"

CWardWizAntirootkit::CWardWizAntirootkit() : behavior_factory("WardWizAntirootkit")
, m_objIPCRootkitClient(ROOTKIT)
, m_bStop(false)
, m_bScanningStopped(false)
, m_bIsManualStop(false)
, m_bIsShutDownPC(false)
, m_bIsMultiAScanFinish(false)
{
}


CWardWizAntirootkit::~CWardWizAntirootkit()
{
}

/***************************************************************************************************
*  Function Name  : On_StartAntirootkitScan
*  Description    : Starts Antiroot scan
*  Author Name    : Sukanya Indurkar
*  Date			  : 10 May 2016
****************************************************************************************************/
json::value CWardWizAntirootkit::On_StartAntirootkitScan(SCITER_VALUE svAntirootkit, SCITER_VALUE svShowScanFinshedPageCB, SCITER_VALUE svAddVirusFoundEntryCB)
{
	theApp.m_bAntirootscan = true;
	theApp.m_bIsARootScanUIReceptive = true;
	m_bIsMultiAScanFinish = false;
	svAntirootkit.isolate();
	bool bIsArray = false;
	bIsArray = svAntirootkit.is_array();
	try
	{
		if (!bIsArray)
		{
			return false;
		}
		m_svFunScanningFinishedCB = svShowScanFinshedPageCB;
		m_svAddVirusFoundEntryCB = svAddVirusFoundEntryCB;
		OnBnClickedBtnScan(svAntirootkit);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_StartAntirootkitScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : On_PauseAntirootScan
*  Description    : Pause and resume Antiroot scan button onclick events.
*  Author Name    : Sukanya Indurkar
*  Date			  : 10 May 2016
****************************************************************************************************/
json::value CWardWizAntirootkit::On_PauseAntirootScan()
{
	try
	{
		{
			PauseScan();
			theApp.m_bAntirootscan = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::On_PauseAntirootScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : On_ResumeAntirootScan
*  Description    : Pause and resume Antiroot scan button onclick events.
*  Author Name    : Sukanya Indurkar
*  Date			  : 10 May 2016
****************************************************************************************************/
json::value CWardWizAntirootkit::On_ResumeAntirootScan()
{
	try
	{
		{
			ResumeScan();
			theApp.m_bAntirootscan = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::On_ResumeAntirootScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : On_StopAntirootScan
*  Description    : Stop Antiroot scan button onclick event.
*  Author Name    : Sukanya Indurkar
*  Date			  : 10 May 2016
****************************************************************************************************/
json::value CWardWizAntirootkit::On_StopAntirootScan(SCITER_VALUE svbIsManualStop)
{
	try
	{
		m_bIsManualStop = svbIsManualStop.get(false);
		StopScan();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::On_StopAntirootScan", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ClickAntirootCleanBtn
*  Description    : Clean Antiroot detected entries on button onclick event.
*  Author Name    : Sukanya Indurkar
*  Date			  : 10 May 2016
****************************************************************************************************/
json::value CWardWizAntirootkit::On_ClickAntirootCleanBtn(SCITER_VALUE RootkitScanOption, SCITER_VALUE svArrCleanVirusEntries, SCITER_VALUE svFunUpdateVirusFoundEntryCB, SCITER_VALUE svFunNotificationMsgCB)
{
	try
	{
		CString csRootkitScanOptn = RootkitScanOption.get(L"").c_str();
		OnBnClickedBtnantirootkitDelete(csRootkitScanOptn, svArrCleanVirusEntries, svFunUpdateVirusFoundEntryCB, svFunNotificationMsgCB);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::On_ClickAntirootCleanBtn", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 InsertDataToTable(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable entered", 0, 0, true, ZEROLEVEL);
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);

		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = objSqlDb.GetLastRowId();

		objSqlDb.Close();

		return iLastRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
	return 0;
}
/**********************************************************************************************************
*  Function Name  :	OnBnClickedBtnScan
*  Description    :	When rootkit scan button is clicked this function gets called.
*  Author Name    : Sukanya Indurkar
*  Date           : 10 May 2016
**********************************************************************************************************/
void CWardWizAntirootkit::OnBnClickedBtnScan(SCITER_VALUE svAntirootkit)
{
	try
	{
		if (!theApp.ShowRestartMsgOnProductUpdate())
		{
			return;
		}
		theApp.m_bIsAntirootkitScanning = true;
		m_bAntirootkitScan = true;
		m_bAntirootScanningInProgress = true;
		m_bStop = false;
		m_bScanningStopped = false;
		m_bIsShutDownPC = false;
		m_bScanningFinished = true;
		g_bIsRootkitScanning = true;
		m_bAntirootClose = false;
		m_bAntirootkitHome = false;
		m_bRedFlag = false;
		m_dwPercentage = 0;
		m_dwScannedFileFolder = 0;
		m_dwScannedCntRegistry = 0;
		m_dwScannedCnt = 0;
		m_dwThreatCntFilefolder = 0;
		m_dwThreatCnt = 0;
		m_dwThreatCntRegistry = 0;
		isAnyEntrySeletected(svAntirootkit);
		if (!StartAntirootScanUsingService())
		{
			AddLogEntry(L"### Failed to send data to service StartAntirootScanUsingService in OnBnClickedBtnScan", 0, 0, true, SECONDLEVEL);
			return;
		}
		Sleep(500);

		// Get entries from registry so that, those can be included in query..
		DWORD dwQuarantineOpt = 0x00;
		DWORD dwHeuristicOpt = 0x00;
		bool  bHeuristicOpt = false; 

		GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);

		if (dwHeuristicOpt == 0x00)
			bHeuristicOpt = true;

		// Add entries into Database..
		SCANTYPE eScanType = ANTIROOTKITSCAN;
		CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");

		csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),%d,%d,%d,%d,%d );"), eScanType, m_dwScannedCnt, m_dwThreatCnt, dwQuarantineOpt, bHeuristicOpt,0);

		CT2A ascii(csInsertQuery, CP_UTF8);

		m_iScanSessionId = InsertDataToTable(ascii.m_psz);

		DWORD m_dwThreadId = 0;
		m_hGetPercentage = ::CreateThread(NULL, 0, GetPercentage, (LPVOID) this, 0, NULL);

		CString csRKScanningStarted;
		csRKScanningStarted.Format(L">>> %s...", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANSTART"));
		AddLogEntry(csRKScanningStarted, 0, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::OnBnClickedBtnScan", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	PauseScan
*  Description    :	When you want to pause rootkit scan this function gets called.
*  Author Name    : Sukanya Indurkar
*  Date           : 10 May 2016
**********************************************************************************************************/
bool CWardWizAntirootkit::PauseScan()
{
	try
	{
		CString csPauseResumeBtnText = L"";
		if (!SendRequestCommon(PAUSE_SCAN))
		{
			AddLogEntry(L"### Failed to pause scan as Send PAUSE_SCAN request failed.", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::PauseScan", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ResumeScan
*  Description    :	After manual pausing rootkit scan this function gets called.
*  Author Name    : Sukanya Indurkar
*  Date           : 10 May 2016
**********************************************************************************************************/
bool CWardWizAntirootkit::ResumeScan()
{
	try
	{
		CString csPauseResumeBtnText = L"";
		if (!SendRequestCommon(RESUME_SCAN))
		{
			AddLogEntry(L"### Failed to pause scan as Send PAUSE_SCAN request failed.", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::ResumeScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	StopScan
*  Description    :	To stop rootkit scan this function gets called.
*  Author Name    : Sukanya Indurkar
*  Date           : 10 May 2016
**********************************************************************************************************/
void CWardWizAntirootkit::StopScan()
{
	try
	{
		m_bIsShutDownPC = false;
		ShutDownRootkitScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::StopScan", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ShutDownRootkitScanning
*  Description    :	After manual stopping rootkit scan this function gets called.
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CWardWizAntirootkit::ShutDownRootkitScanning()
{
	if (!g_bIsRootkitScanning)
	{
		return true;
	}
	try
	{
		m_bScanningStopped = false;
		CString csScanBtnText = L"";

		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = STOP_SCAN;
		szPipeData.dwValue = static_cast<DWORD>(ANTIROOTKITSCAN);
		CISpyCommunicator objCom(SERVICE_SERVER, true);

		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumAntiRootkit::ShutDownAntiRootkitScanning", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CVibraniumAntiRootkit::ShutDownAntiRootkitScanning", 0, 0, true, SECONDLEVEL);
			return false;
		}

		m_bStop = true;
		Sleep(500);

		if (m_hGetPercentage != NULL)
		{
			ResumeThread(m_hGetPercentage);
			TerminateThread(m_hGetPercentage, 0);
			m_hGetPercentage = NULL;
		}

		m_bAntirootScanningInProgress = false;	
		theApp.m_bIsAntirootkitScanning = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntiRootkit::ShutDownRootkitScanning", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SendRequestCommon
*  Description    :	Send request to comm service
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CWardWizAntirootkit::SendRequestCommon(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;
		szPipeData.dwValue = static_cast<DWORD>(ANTIROOTKITSCAN);

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to SERVICE_SERVER in CScanDlg::SendRequestCommon", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::SendRequestCommon", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	isAnyEntrySeletected
*  Description    :	Checks atleast one entry is selected or not.
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CWardWizAntirootkit::isAnyEntrySeletected(SCITER_VALUE svAntirootkit)
{
	try
	{
		m_icount = 0;
		m_bCheckFileFolder = false;
		m_bCheckProcess = false;
		m_bCheckRegistry = false;

		const SCITER_VALUE EachEntry = svAntirootkit[2];
		m_bValue = EachEntry[L"Val"].get(false);

		if (m_bValue)
		{
			m_dwGetCheckValues = 0x01;
			m_bCheckProcess = true;
			m_icount++;
		}

		const SCITER_VALUE EachEntryRegistry = svAntirootkit[1];
		m_bValue = EachEntryRegistry[L"Val"].get(false);
		if (m_bValue)
		{
			m_dwGetCheckValues = 0x02;
			m_bCheckRegistry = true;
			m_icount++;
		}

		const SCITER_VALUE EachEntryFilesFolders = svAntirootkit[0];
		m_bValue = EachEntryFilesFolders[L"Val"].get(false);
		if (m_bValue)
		{
			m_dwGetCheckValues = 0x03;
			m_bCheckFileFolder = true;
			m_icount++;
		}

		if (m_icount == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::isAnyEntrySeletected", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	StartAntirootScanUsingService
*  Description    : Start rootkit scan using service
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
bool CWardWizAntirootkit::StartAntirootScanUsingService()
{
	try
	{
		DWORD dwScanOption = 0;
		GetDWORDFromRootKitScanOptions(dwScanOption);
		m_dwRootKitOption = dwScanOption;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = START_SCAN;
		szPipeData.dwValue = static_cast<DWORD>(ANTIROOTKITSCAN);
		szPipeData.dwSecondValue = m_dwRootKitOption;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumAntirootkit::StartAntirootScanUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::StartAntirootScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;

}

/**********************************************************************************************************
*  Function Name  :	GetDWORDFromRootKitScanOptions
*  Description    : Get dword value from selected rootkit scan options
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CWardWizAntirootkit::GetDWORDFromRootKitScanOptions(DWORD &dwRegScanOpt)
{
	DWORD	dwValue = 0x00;
	DWORD	dwTemp = 0x00;
	try
	{
		for (DWORD i = 0x00; i < 0x03; i++)
		{
			switch (i)
			{
			case 0:
				if (m_bCheckProcess)
				{
					dwValue = dwTemp = 0x01;
				}
				break;
			case 1:
				if (m_bCheckRegistry)
				{
					dwTemp = 1 << i;
					dwValue = dwValue + dwTemp;
				}
				break;

			case 2:
				if (m_bCheckFileFolder)
				{
					dwTemp = 1 << i;
					dwValue = dwValue + dwTemp;
				}
				break;
			}
		}
		dwRegScanOpt = dwValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::GetDWORDFromRootKitScanOptions", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetPercentage
*  Description    :	Get all entries through shared memory which has to display on the UI
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
DWORD WINAPI GetPercentage(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizAntirootkit *pThis = (CWardWizAntirootkit*)lpvThreadParam;
		if (!pThis)
			return 0;

		wchar_t Buffer[100];
		CString csType, csPID;

		while (true)
		{
			if (pThis->m_bStop)
				break;

			ITIN_MEMMAP_DATA iTinMemMap = { 0 };
			pThis->m_objIPCRootkitClient.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));

			switch (iTinMemMap.iMessageInfo)
			{
			case SETSCANSTATUS:
			{
				CString csCurrentStatus = L"";
				csCurrentStatus = iTinMemMap.szFirstParam;
				csCurrentStatus.ReleaseBuffer();
				break;
			}

			case SETPERCENTAGE:
			{
				pThis->m_dwPercentage = iTinMemMap.dwFirstValue;
				CString csPercentage = L"";
				csPercentage.Format(L"%d", pThis->m_dwPercentage);
				pThis->OnCallSetPercentage(csPercentage);
				//AddLogEntry(L"Client side value of percentage %s",Buffer);
				break;
			}

			case SHOWVIRUSFOUND:
			{
				CString csCurrentPath(L"");
				csCurrentPath.Format(L"%s", iTinMemMap.szFirstParam);
				if (csCurrentPath.GetLength() > 0)
				{
					//OutputDebugString(csCurrentPath);
					pThis->m_dwForInsertItem = iTinMemMap.dwSecondValue;
					csPID.Format(L"%d", iTinMemMap.dwFirstValue);
					csType.Format(L"%d", iTinMemMap.dwSecondValue);

					//Ram: Temparary commented
					//pThis->InsertItem(iTinMemMap.szFirstParam, csPID, iTinMemMap.szSecondParam, DETECTEDSTATUS);
				}
				csCurrentPath.ReleaseBuffer();
				break;
			}

			case SHOWDETECTEDENTRIES:
			{
				//Ram: Temparary commented
				//pThis->m_dwType = iTinMemMap.dwSecondValue;
				//_itow_s(iTinMemMap.dwFirstValue, Buffer, 10);
				//AddLogEntry(L"Client side value of detected entries %s",Buffer);
				break;
			}

			case SHOWTOTALCOUNT:
			{
				pThis->m_dwTotalType = iTinMemMap.dwSecondValue;
				pThis->AddTotalCount(iTinMemMap.dwFirstValue);
				_itow_s(iTinMemMap.dwFirstValue, Buffer, 10);
				//AddLogEntry(L"Client side value of total entries %s",Buffer);
				break;
			}
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	InsertItem
*  Description    :	Inserting virus found entries
*  Author Name    : Sukanya Indurkar
*  SR_NO		  :
*  Date           : May 2016
**********************************************************************************************************/
void CWardWizAntirootkit::InsertItem(CString csFirstParam, CString csSecondParam, CString csThirdParam, CString csForthParam)
{
	LVITEM lvItem;
	int nItem = 0;
	int imgNbr = 0;

	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	CString csForInsertItem = L"";
	CString cstmp;
	try
	{
		if (m_dwForInsertItem == 0x01)//Process
		{
			csForInsertItem.Format(L"%d", m_dwForInsertItem);

			const sciter::value data[5] = { sciter::string(csForInsertItem), sciter::string(csSecondParam), sciter::string(csSecondParam), sciter::string(csThirdParam), sciter::string(csForthParam) };
			sciter::value arrDetectedVirusArray = sciter::value(data, 5);
			m_svAddVirusFoundEntryCB.call(arrDetectedVirusArray);
			AddEntriesInReportsDB(L"RootKit Scan", csFirstParam, csThirdParam, csForthParam);
		}
		else if (m_dwForInsertItem == 0x02)//Registry
		{
			csForInsertItem.Format(L"%d", m_dwForInsertItem);
			const sciter::value data[4] = { sciter::string(csForInsertItem), sciter::string(csThirdParam), sciter::string(csFirstParam), sciter::string(csForthParam) };
			sciter::value arr = sciter::value(data, 4);
			m_svAddVirusFoundEntryCB.call(arr);
			AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), csThirdParam, csFirstParam, csForthParam);
		}

		else if (m_dwForInsertItem == 0x03)//fileorfolders or driverpath
		{
			csForInsertItem.Format(L"%d", m_dwForInsertItem);
			const sciter::value data[3] = { sciter::string(csForInsertItem), sciter::string(csFirstParam), sciter::string(csForthParam) };
			sciter::value arr = sciter::value(data, 3);
			m_svAddVirusFoundEntryCB.call(arr);
			AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), L"NA", csFirstParam, csForthParam);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::InsertItem", 0, 0, true, SECONDLEVEL);
	}

}

/**********************************************************************************************************
*  Function Name  :	OnBnClickedBtnantirootkitDelete
*  Description    :	Antirootkit clear button onclick event 
*  Author Name    : Sukanya Indurkar
*  SR_NO		  :
*  Date           : May 2016
**********************************************************************************************************/
void CWardWizAntirootkit::OnBnClickedBtnantirootkitDelete(CString csRootkitScanOption, SCITER_VALUE svArrCleanVirusEntries, SCITER_VALUE svFunUpdateVirusFoundEntryCB, SCITER_VALUE  svFunNotificationMsgCB) // for clean button
{
	svArrCleanVirusEntries.isolate();
	bool bIsArray = false;
	m_svFunNotificationMsgCB = svFunNotificationMsgCB;
	m_svFunUpdateVirusFoundEntryCB = svFunUpdateVirusFoundEntryCB;
	bIsArray = svArrCleanVirusEntries.is_array();
	m_svArrCleanVirusEntries = svArrCleanVirusEntries;
	try
	{
		if (!bIsArray)
		{
			return;
		}
		bool bEntrySelected = false;
		int iCnt = 0;
		CString  csAction;

		if (csRootkitScanOption == "Process")
		{
			m_bProcessTabSelected = true;
			m_bRegistryTabSelected = false;
			m_bFilesFoldersTabSelected = false;
		}
		else if (csRootkitScanOption == "Registry")
		{
			m_bProcessTabSelected = false;
			m_bRegistryTabSelected = true;
			m_bFilesFoldersTabSelected = false;
		}
		else if (csRootkitScanOption == "FilesFolders")
		{
			m_bProcessTabSelected = false;
			m_bRegistryTabSelected = false;
			m_bFilesFoldersTabSelected = true;
		}


		if (m_bProcessTabSelected)
		{
			iCnt = svArrCleanVirusEntries.length();

			if (iCnt == 0)
			{
				CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_NOENTRIES"));
				goto CleanUp;
			}
			for (int i = 0; i < svArrCleanVirusEntries.length(); i++)
			{
				const SCITER_VALUE svEachEntry = svArrCleanVirusEntries[i];
				const std::wstring chPID = svEachEntry[L"PID"].get(L"");
				const std::wstring chProcessName = svEachEntry[L"ProcessName"].get(L"");
				const std::wstring chProcessPath = svEachEntry[L"ProcessPath"].get(L"");
				const std::wstring chProcessActionTaken = svEachEntry[L"ProcessActionTaken"].get(L"");
				bool bProcessValue = svEachEntry[L"selected"].get(false);


				if (bProcessValue)
				{
					if (chProcessActionTaken == DETECTEDSTATUS)
					{
						bEntrySelected = true;
						break;
					}
				}
			}
		}
		else if (m_bRegistryTabSelected)
		{
			iCnt = svArrCleanVirusEntries.length();

			if (iCnt == 0)
			{
				CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_NOENTRIES"));
				goto CleanUp;
			}
			for (int i = 0; i < svArrCleanVirusEntries.length(); i++)
			{
				const SCITER_VALUE svEachEntry = svArrCleanVirusEntries[i];
				const std::wstring chRegistryName = svEachEntry[L"RegistryName"].get(L"");
				const std::wstring chRegistryPath = svEachEntry[L"RegistryPath"].get(L"");
				const std::wstring chRegistryActionTaken = svEachEntry[L"RegistryActionTaken"].get(L"");
				bool bRegistryValue = svEachEntry[L"selected"].get(false);

				if (bRegistryValue)
				{
					if (chRegistryActionTaken == DETECTEDSTATUS)
					{
						bEntrySelected = true;
						break;
					}
				}
			}
		}
		else if (m_bFilesFoldersTabSelected)
		{
			iCnt = svArrCleanVirusEntries.length();
			if (iCnt == 0)
			{
				CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_NOENTRIES"));
				goto CleanUp;
			}
			for (int i = 0; i < svArrCleanVirusEntries.length(); i++)
			{
				const SCITER_VALUE svEachEntry = svArrCleanVirusEntries[i];
				const std::wstring chFilesFoldersPath = svEachEntry[L"FilesFoldersPath"].get(L"");
				const std::wstring chFilesFoldersActionTaken = svEachEntry[L"FilesFoldersActionTaken"].get(L"");
				bool bFilesFoldersValue = svEachEntry[L"selected"].get(false);

				if (bFilesFoldersValue)
				{
					if (chFilesFoldersActionTaken == DETECTEDSTATUS)
					{
						bEntrySelected = true;
						break;
					}
				}
			}
		}
		if (!bEntrySelected)
		{
			CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_CLEANENTRIES"));
			return;
		}

		m_hCleaningThread = ::CreateThread(NULL, 0, DeleteThread, (LPVOID) this, 0, NULL);
		Sleep(500);

	CleanUp:
		return;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::OnBnClickedBtnantirootkitDelete", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	DeleteThread
*  Description    :	Thread to clean detected entries
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
DWORD WINAPI DeleteThread(LPVOID lpParam)
{
	try
	{
		CWardWizAntirootkit *pThis = (CWardWizAntirootkit *)lpParam;
		pThis->DeleteEntries();
		return 1;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::DeleteThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	DeleteEntries
*  Description    :	Send entry to service to get clean
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Jan 2014
*  Modified by	  : Sukanya Indurkar 
**********************************************************************************************************/
void CWardWizAntirootkit::DeleteEntries()
{
	try
	{
		bool bCheck = false;
		DWORD dwCleanCount = 0;
		CString csAction;
		m_bRedFlag = true;

		TCHAR	Datebuff[9] = { 0 };
		TCHAR	csDate[9] = { 0 };
		BOOL bBackUp = 0;

		_wstrdate_s(Datebuff, 9);
		_tcscpy_s(csDate, Datebuff);

		if (!SendReportOperations2Service(RELOAD_DBENTRIES, L"", REPORTS, true))
		{
			AddLogEntry(L"### Error in CVibraniumAntiRootkit::SendRecoverOperations2Service RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
		}
		if (m_bProcessTabSelected)
		{

			CString csProcessPath, csProcessID;
			for (int i = 0; i < m_svArrCleanVirusEntries.length(); i++)
			{
				const SCITER_VALUE svEachEntry = m_svArrCleanVirusEntries[i];
				const std::wstring chPID = svEachEntry[L"PID"].get(L"");
				const std::wstring chProcessName = svEachEntry[L"ProcessName"].get(L"");
				const std::wstring chProcessPath = svEachEntry[L"ProcessPath"].get(L"");
				const std::wstring chProcessActionTaken = svEachEntry[L"ProcessActionTaken"].get(L"");
				bool bProcessValue = svEachEntry[L"selected"].get(false);

				bCheck = bProcessValue ? true : false;
				csAction = chProcessActionTaken.c_str();
				if (bCheck && csAction == DETECTEDSTATUS)
				{
					csProcessPath = chProcessName.c_str(); //process name
					csProcessID = chPID.c_str(); //process ID
					//0x01 dword for process	
					if (!SendFile4RepairUsingService(HANDLE_VIRUS_ENTRY, csProcessPath, csProcessID, L"", 0x01, ROOTKITSCAN, true))
					{
						m_svFunUpdateVirusFoundEntryCB.call(i, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"));
						AddLogEntry(L"### Error in CVibraniumAntiRootkit::SendFile4RepairUsingService HANDLE_VIRUS_ENTRY", 0, 0, true, SECONDLEVEL);
						continue;
					}
					AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), L"NA", csProcessPath, DELETEDSTATUS);
					m_svFunUpdateVirusFoundEntryCB.call(i, (SCITER_STRING)L"Deleted");//(SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"));
					dwCleanCount++;
				}
			}
		}
		if (m_bRegistryTabSelected)//no code change for registry now.
		{
			CString csServiceName, csServicePath;
			for (int i = 0; i < m_svArrCleanVirusEntries.length(); i++)
			{
				const SCITER_VALUE svEachEntry = m_svArrCleanVirusEntries[i];
				const std::wstring chRegistryName = svEachEntry[L"RegistryName"].get(L"");
				const std::wstring chRegistryPath = svEachEntry[L"RegistryPath"].get(L"");
				const std::wstring chRegistryActionTaken = svEachEntry[L"RegistryActionTaken"].get(L"");
				bool bRegistryValue = svEachEntry[L"selected"].get(false);

				bCheck = bRegistryValue ? true : false;
				csAction = chRegistryActionTaken.c_str();
				if (bCheck && csAction == DETECTEDSTATUS)
				{
					csServiceName = chRegistryName.c_str(); //registry key 
					csServicePath = chRegistryPath.c_str(); //registry value
					//0x02 for Registry	
					if (!SendFile4RepairUsingService(HANDLE_VIRUS_ENTRY, csServiceName, csServicePath, L"", 0x02, ROOTKITSCAN, true))
					{
						m_svFunUpdateVirusFoundEntryCB.call(i, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"));
						AddLogEntry(L"### Error in CVibraniumAntiRootkit::SendFile4RepairUsingService HANDLE_VIRUS_ENTRY", 0, 0, true, SECONDLEVEL);
						continue;
					}
					AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), csServiceName, csServicePath, DELETEDSTATUS);
					m_svFunUpdateVirusFoundEntryCB.call(i, (SCITER_STRING)L"Deleted");//(SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"));
					dwCleanCount++;
				}
			}
		}
		if (m_bFilesFoldersTabSelected)
		{
			CString csFileFolderPath;
			for (int i = 0; i <m_svArrCleanVirusEntries.length(); i++)
			{
				const SCITER_VALUE svEachEntry = m_svArrCleanVirusEntries[i];
				const std::wstring chFilesFoldersPath = svEachEntry[L"FilesFoldersPath"].get(L"");
				const std::wstring chFilesFoldersActionTaken = svEachEntry[L"FilesFoldersActionTaken"].get(L"");
				bool bFilesFoldersValue = svEachEntry[L"selected"].get(false);

				bCheck = bFilesFoldersValue ? true : false;
				csAction = chFilesFoldersActionTaken.c_str();
				if (bCheck && csAction == DETECTEDSTATUS)
				{
					csFileFolderPath = chFilesFoldersPath.c_str(); //file/folder path 
					//0x03 for device name or file/folder name
					if (!SendFile4RepairUsingService(HANDLE_VIRUS_ENTRY, csFileFolderPath, L"", L"", 0x03, ROOTKITSCAN, true))
					{
						m_svFunUpdateVirusFoundEntryCB.call(i, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED")); 
						AddLogEntry(L"### Error in CVibraniumAntiRootkit::SendFile4RepairUsingService HANDLE_VIRUS_ENTRY", 0, 0, true, SECONDLEVEL);
						continue;
					}
					AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), L"NA", csFileFolderPath, DELETEDSTATUS);
					m_svFunUpdateVirusFoundEntryCB.call(i, (SCITER_STRING)L"Deleted");//(SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"));
					dwCleanCount++;
				}
			}
		}

		Sleep(5);

		if (!SendFile4RepairUsingService(SAVE_REPORTS_ENTRIES, L"", L"", L"", 5, ROOTKITSCAN))
		{
			AddLogEntry(L"### Error in CVibraniumAntiRootkit::SendFile4RepairUsingService SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
		}
		m_bRedFlag = false;
		CString csTotalClean;
		csTotalClean.Format(L"%s %d", theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_CLEANING_SUCCESSFULL"), dwCleanCount);

		CallNotificationMessage(1, (SCITER_STRING)csTotalClean);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntiRootkit::DeleteEntries", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/**********************************************************************************************************
*  Function Name  :	SendFile4RepairUsingService
*  Description    :	Send file to repair using services
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CWardWizAntirootkit::SendFile4RepairUsingService(int iMessage, CString csEntryOne, CString csEntryTwo, CString csEntryThree, DWORD dwISpyID, DWORD dwScanType, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = iMessage;
	szPipeData.dwValue = dwISpyID;
	szPipeData.dwSecondValue = dwScanType;
	wcscpy_s(szPipeData.szFirstParam, csEntryOne);
	wcscpy_s(szPipeData.szSecondParam, csEntryTwo);
	wcscpy_s(szPipeData.szThirdParam, csEntryThree);
	try
	{
		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumAntiRootkit::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CVibraniumAntiRootkit::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SetRegistrykeyUsingService
*  Description    :	Set any registry key using service
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CWardWizAntirootkit::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
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
	try
	{
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CVibraniumAntiRootkit : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumAntiRootkit : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	AddDetectedCount
*  Description    :	Add detected count of files/registry/process.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CWardWizAntirootkit::AddDetectedCount(DWORD dwDetectedEntries)
{
	try
	{
		if (m_dwType == 0x01)
		{

			m_dwThreatCnt = dwDetectedEntries;
		}
		if (m_dwType == 0x02)
		{

			m_dwThreatCntRegistry = dwDetectedEntries;
		}
		if (m_dwType == 0x03)
		{
			m_dwThreatCntFilefolder = dwDetectedEntries;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::AddDetectedCount", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	CallRootkitUIFileCountfunction
*  Description    :	Counts scanned files
*  Author Name    : Sukanya Indurkar
*  SR_NO		  :
*  Date           :  May 2016
**********************************************************************************************************/
void CWardWizAntirootkit::CallRootkitUIFileCountfunction(DWORD dwTotalFileCnt, DWORD dwTotalDriveCnt, DWORD dwTotalProcCnt, DWORD dwDetectedFileCnt, DWORD dwDetectedDriveCnt, DWORD dwDetectedProcCnt)
{
	try
	{
		m_dwScannedFileFolder = dwTotalFileCnt;
		m_dwScannedCntRegistry = dwTotalDriveCnt;
		m_dwScannedCnt = dwTotalProcCnt;

		csTotalFileCnt.Format(L"%d", dwTotalFileCnt);
		csTotalDriveCnt.Format(L"%d", dwTotalDriveCnt);
		csTotalProcCnt.Format(L"%d", dwTotalProcCnt);

		csDetectedFileCnt.Format(L"%d", dwDetectedFileCnt);
		csDetectedDriveCnt.Format(L"%d", dwDetectedDriveCnt);
		csDetectedProcCnt.Format(L"%d", dwDetectedProcCnt);

		//const sciter::value data[6] = { sciter::string(csTotalFileCnt), sciter::string(csTotalDriveCnt), sciter::string(csTotalProcCnt), sciter::string(csDetectedFileCnt), sciter::string(csDetectedDriveCnt), sciter::string(csDetectedProcCnt) };
		//sciter::value arr = sciter::value(data, 6);
		//m_svUpdateFileCountCB.call(arr);

		if (theApp.m_bIsARootScanUIReceptive)
		{
			sciter::value map;
			map.set_item("one", sciter::string(csTotalFileCnt));
			map.set_item("two", sciter::string(csTotalDriveCnt));
			map.set_item("three", sciter::string(csTotalProcCnt));
			map.set_item("four", sciter::string(csDetectedFileCnt));
			map.set_item("five", sciter::string(csDetectedDriveCnt));
			map.set_item("six", sciter::string(csDetectedProcCnt));
			//Send here event
			ela = self;
			BEHAVIOR_EVENT_PARAMS params;
			params.cmd = SETSCANCOUNT_EVENT_CODE;
			params.he = params.heTarget = ela;
			params.data = map;
			ela.fire_event(params, true);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::CallRootkitUIFileCountfunction", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	AddTotalCount
*  Description    :	Add total count of files/registry/process count
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CWardWizAntirootkit::AddTotalCount(DWORD dwTotalEntries)
{
	try
	{
		if (m_dwTotalType == 0x01)
		{
			//m_dwScannedCnt = dwTotalEntries;
			////m_csScannedText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_PROC"));
			//m_csScannedEntries.Format(L": %d", m_dwScannedCnt);
			//m_stScannedEntries.SetWindowTextW(m_csScannedEntries);

		}
		if (m_dwTotalType == 0x02)
		{
			//m_dwScannedCntRegistry = dwTotalEntries;
			////m_stRegisteryText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_REG"));
			//m_csScannedEntries.Format(L": %d", m_dwScannedCntRegistry);
			//m_stScannedRegistry.SetWindowTextW(m_csScannedEntries);
		}
		if (m_dwTotalType == 0x03)
		{
			//m_dwScannedFileFolder = dwTotalEntries;
			////m_stFileText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_FILE"));
			//m_csScannedEntries.Format(L": %d", m_dwScannedFileFolder);
			//m_stScannedFilesFolders.SetWindowTextW(m_csScannedEntries);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::AddTotalCount", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	RootKitScanningStarted
*  Description    :	Open memory after server memory gets created
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May ,2014
**********************************************************************************************************/
bool CWardWizAntirootkit::RootKitScanningStarted()
{
	try
	{
		DWORD dwret = m_objIPCRootkitClient.OpenServerMemoryMappedFile();
		if (dwret)
		{
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::RootKitScanningStarted", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	RootKitScanningFinished
*  Description    :	According to manual stop and complete scan gives message box and reset controls.
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CWardWizAntirootkit::RootKitScanningFinished()
{
	try
	{
		m_bIsMultiAScanFinish = true;
		OutputDebugStringW(L"WrdWizAntirootscan UI::Scanning finished");
		if (!g_bIsRootkitScanning)
		{
			return false;
		}
		g_bIsRootkitScanning = false;
		CString csBtnText, stopBtnText;

		::WaitForSingleObject(m_hGetPercentage, 500);
		if (m_hGetPercentage != NULL)
		{
			ResumeThread(m_hGetPercentage);
			TerminateThread(m_hGetPercentage, 0);
			m_hGetPercentage = NULL;
			AddLogEntry(L"Terminate get percentage thread successfully", 0, 0, true, SECONDLEVEL);
		}

		/*if (!m_bIsManualStop)
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

		//Sleep(200);
		if (!m_bIsManualStop)
		{
			if (theApp.m_bIsARootScanUIReceptive)
			{
				if (theApp.m_bIsARootScanPageSwitched)
				{
					OnCallSetFinishStatus();
				}
				else
				{
					m_svFunScanningFinishedCB.call();
				}
			}
		}

		if (m_bScanningStopped == false)
		{
			if (m_bAntirootClose == false && m_bScanningStopped == false)
			{
				if (m_dwThreatCntFilefolder == 0 && m_dwThreatCnt == 0 && m_dwThreatCntRegistry == 0)
				{
					AddEntriesInReportsDB(L"Rootkit Scan", L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND"));
				}

				CString csRKScanningComplete;
				AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
				csRKScanningComplete.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANCOMPLETE"));
				AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
				csRKScanningComplete.Format(L">>> Total Scanned:		Processes = [%d]\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwScannedCnt, m_dwScannedCntRegistry, m_dwScannedFileFolder);
				AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
				csRKScanningComplete.Format(L">>> Total Threats Found:	Processes = [%d]\t\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwThreatCnt, m_dwThreatCntRegistry, m_dwThreatCntFilefolder);
				AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
				AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
				OutputDebugStringW(L"WrdWizAntirootscan UI::Before Message box");

				if (!SendReportOperations2Service(SAVE_REPORTS_ENTRIES, L"", REPORTS, true))
				{
					AddLogEntry(L"### Error in CVibraniumAntirootkit::SendRecoverOperations2Service RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
				}
				m_bAntirootScanningInProgress = false;
				//m_bAntirootkitCompleted = true;
				m_bScanningStopped = false;


				if (m_bIsShutDownPC == true && CheckIsScanShutdown() == true)
				{
					CEnumProcess enumproc;
					enumproc.RebootSystem(1);
				}

				return true;
			}
		}
		else if (m_bScanningStopped == true)
		{
			CString csRKScanningComplete;
			AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ROOTKITABORT"));
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> Total Scanned:		Processes = [%d]	\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwScannedCnt, m_dwScannedCntRegistry, m_dwScannedFileFolder);
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> Total Threats Found:	Processes = [%d]\t\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwThreatCnt, m_dwThreatCntRegistry, m_dwThreatCntFilefolder);
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
			m_bAntirootScanningInProgress = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::RootKitScanningFinished", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	GetWardwizRegistryDetails
*  Description    :	Read Scanning related options from registry
*  Author Name    : Gayatri A.
*  SR_NO		  :
*  Date           : 13 Sep 2016
**********************************************************************************************************/
bool CWardWizAntirootkit::GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt)
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
			AddLogEntry(L"### Failed to get registry key value for Heuristic Scan Option in CWardwizScan::GetGenXRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::GetWardwizRegistryDetails", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	GetActionIDFromAction
*  Description    : Get action Id as stored in ini file from the action text.This will be used to 
					store in DB.Will help in multi language support.
*  Author Name    : Gayatri A.
*  SR_NO		  :
*  Date           : 15 Sep 2016
**********************************************************************************************************/
CString GetActionIDFromAction(CString csMessage)
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
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_NO_FILE_FOUND";
		}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
		}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
		}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_FAILED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_FAILED";
		}

	return csLanguageID;
}
/***************************************************************************
*  Function Name  : AddEntriesInReportsDB
*  Description    : Add entries into reports
*  Author Name    : Neha gharge
*  SR_NO          :
*  Date           : 28th may 2014
*  Modification   : 28th may 2014
****************************************************************************/
void CWardWizAntirootkit::AddEntriesInReportsDB(CString csScanType, CString csThreatName, CString csFilePath, CString csAction)
{
	try
	{
		CTime ctDateTime = CTime::GetCurrentTime();
		CString csAntirootkitReportEntry = L"";
		CString csTime = ctDateTime.Format(_T("%H:%M:%S"));

		//OutputDebugString(L">>> AddinReports " + csFilePath);

		SYSTEMTIME  CurrTime = { 0 };
		GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
		CTime Time_Curr(CurrTime);
		int iMonth = Time_Curr.GetMonth();
		int iDate = Time_Curr.GetDay();
		int iYear = Time_Curr.GetYear();

		CString csDate = L"";
		csDate.Format(L"%d/%d/%d", iMonth, iDate, iYear);

		CString csDateTime = L"";
		csDateTime.Format(_T("%s %s"), csDate, csTime);
		csAntirootkitReportEntry.Format(L"%s#%s#%s#%s#%s#", csDateTime, csScanType, csThreatName, csFilePath, csAction);

		// Get entries from registry so that, those can be included in query..
		DWORD dwQuarantineOpt;
		DWORD dwHeuristicOpt;
		bool  bHeuristicOpt = false;

		GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);

		if (dwHeuristicOpt == 1)
			bHeuristicOpt = true;

		CString csActionID = GetActionIDFromAction(csAction);

		CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");

		csThreatName.Replace(L"'", L"''");
		csFilePath.Replace(L"'", L"''");

		csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%I64d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),'%s','%s','%s',NULL);"), m_iScanSessionId, csThreatName, csFilePath, csActionID);

		CT2A ascii(csInsertQuery, CP_UTF8);

		InsertDataToTable(ascii.m_psz);
		//Update Scan session information as well.
		csInsertQuery = "";
		csInsertQuery.Format(_T("UPDATE Wardwiz_ScanSessionDetails SET db_ScanSessionEndDate = Date('now'),db_ScanSessionEndTime = Datetime('now', 'localtime'),db_TotalFilesScanned = %d,db_TotalThreatsFound = %d, db_TotalThreatsCleaned = %d WHERE db_ScanSessionID = %I64d;"), m_dwScannedCnt + m_dwScannedCntRegistry + m_dwScannedFileFolder, m_dwThreatCnt, 0, m_iScanSessionId);

		CT2A Updateascii(csInsertQuery, CP_UTF8);
		InsertDataToTable(Updateascii.m_psz);
		if (!SendReportOperations2Service(REPORT_ANTIROOTKIT_ENTRY, csAntirootkitReportEntry, 0x05, true))
		{
			AddLogEntry(L"### Error in CVibraniumAntiRootkit::AddEntriesInReportsDB", 0, 0, true, SECONDLEVEL);
			return;
		}
		Sleep(5);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::AddEntriesInReportsDB", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	SendReportOperations2Service
*  Description    :	Send rootkit scan report to service, so that it get stored to DB file
*  Author Name    : Neha Ghareg
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
bool CWardWizAntirootkit::SendReportOperations2Service(int dwMessageinfo, CString csReportFileEntry, DWORD dwType, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	szPipeData.iMessageInfo = dwMessageinfo;
	_tcscpy_s(szPipeData.szFirstParam, csReportFileEntry);
	szPipeData.dwValue = dwType;
	CISpyCommunicator objCom(SERVICE_SERVER, true);
	try
	{
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumAntiRootkit::SendReportOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CVibraniumAntiRootkit::SendReportOperations2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::SendReportOperations2Service", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
Function Name  : CallNotificationMessage
Description    : Calls Light box on UI
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Sept 2016
/***************************************************************************************************/
void CWardWizAntirootkit::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
{
	try
	{
		m_svFunNotificationMsgCB.call(iMsgType, (SCITER_STRING)strMessageString);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		Sleep(300); 
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : On_ClickShutDwonScan
Description    : Function which sends bool value by shutdown checkbox.
Author Name    : Amol J.
SR_NO		   :
Date           : 07 Aug 2017
/*************************s**************************************************************************/
json::value CWardWizAntirootkit::On_ClickShutDownScan(SCITER_VALUE svIsShutDownPC)
{
	m_bIsShutDownPC = svIsShutDownPC.get(false);
	return 0;
}

/***********************************************************************************************
Function Name  : OnCallContinueARootScan
Description    : Called when scan is to be continued in backend
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 08th April 2019
***********************************************************************************************/
json::value CWardWizAntirootkit::OnCallContinueARootScan()
{
	try
	{
		theApp.m_bIsARootScanPageSwitched = true;
		theApp.m_bIsARootScanUIReceptive = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::OnCallContinueARootScan", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : OnSetTimer
Description    : Called when scan is to be continued
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 08th April 2019
***********************************************************************************************/
json::value CWardWizAntirootkit::OnSetTimer()
{
	try
	{
		theApp.m_bIsARootScanUIReceptive = true;
		if (m_bIsMultiAScanFinish)
		{
			OnCallSetFinishStatus();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::OnSetTimer", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : OnCallSetPercentage
Description    : Called to set scan percentage
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 08th April 2019
***********************************************************************************************/
void CWardWizAntirootkit::OnCallSetPercentage(CString csPercentage)
{
	try
	{
		CString csPercVal;
		csPercVal = csPercentage;
		if (theApp.m_bIsARootScanUIReceptive)
		{
			sciter::value map;
			map.set_item("one", sciter::string(csPercVal));
			map.set_item("two", L"");
			map.set_item("three", L"");
			//Send here event
			ela = self;
			BEHAVIOR_EVENT_PARAMS params;
			params.cmd = SETSCANSTATUS_EVENT_CODE;
			params.he = params.heTarget = ela;
			params.data = map;
			ela.fire_event(params, true);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::OnCallSetPercentage", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : OnCallSetFinishStatus
Description    : Called when scan is finished
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 08th April 2019
***********************************************************************************************/
void CWardWizAntirootkit::OnCallSetFinishStatus()
{
	try
	{
		sciter::value map;
		map.set_item("one", sciter::string(csTotalFileCnt));
		map.set_item("two", sciter::string(csTotalDriveCnt));
		map.set_item("three", sciter::string(csTotalProcCnt));
		map.set_item("four", sciter::string(csDetectedFileCnt));
		map.set_item("five", sciter::string(csDetectedDriveCnt));
		map.set_item("six", sciter::string(csDetectedProcCnt));

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
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::OnCallSetFinishStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : On_ScanFinishedCall
Description    : Called when scan is finished
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 08th April 2019
***********************************************************************************************/
json::value CWardWizAntirootkit::On_ScanFinishedCall(SCITER_VALUE svGetScanFinishedFlag)
{
	try
	{
		m_svGetScanFinishedFlag = svGetScanFinishedFlag;;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::On_ScanFinishedCall", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetAntirootScanStatuss
*  Description    :	To get other scan status
*  Author Name    : Akshay Patil
*  Date           : 21 Nov 2019
**********************************************************************************************************/
json::value CWardWizAntirootkit::GetAntirootScanStatus(SCITER_VALUE svIsScanRunning)
{
	try
	{
		m_svIsScanRunning = svIsScanRunning;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::GetAntirootScanStatus", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	CheckIsScanShutdown
*  Description    :	Check if any scan after shut down ticked
*  Author Name    : Akshay Patil
*  Date           : 21 Nov 2019
**********************************************************************************************************/
bool CWardWizAntirootkit::CheckIsScanShutdown()
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
		AddLogEntry(L"### Exception in CVibraniumAntirootkit::CheckIsScanShutdown", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}