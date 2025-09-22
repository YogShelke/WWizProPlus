/*********************************************************************
*  Program Name: CWardWizRegistryOpt.cpp
*  Description: CWardWizRegistryOpt Implementation
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28 March 2016
*  Version No: 2.0.0.1
**********************************************************************/

#include "stdafx.h"
#include "WardWizUI.h"
#include "WardWizRegistryOpt.h"

#define		SETREGOPTSTATUS_EVENT_CODE		(FIRST_APPLICATION_EVENT_CODE + 23)
#define		SETREGOPTCOUNT_EVENT_CODE		(FIRST_APPLICATION_EVENT_CODE + 24)
#define		SETREGOPTFINISHED_EVENT_CODE	(FIRST_APPLICATION_EVENT_CODE + 25)

char* g_strDatabasePath = ".\\VBALLREPORTS.DB";
REGOPTSCANOPTIONS	g_StatOptions = { 0 };
DWORD WINAPI ScanRepairRegistryEntriesThread(LPVOID lpParam);

// CWardWizRegistryOpt
CWardWizRegistryOpt::CWardWizRegistryOpt() : behavior_factory("WardWizRegOptimizer")
, m_objiTinServerMemMap_Client(REGISTRYOPTIMIZER)
, m_hRegOptThread(NULL)
, m_bIsMultiRegOptFinish(false)
, m_csCommandLine(L"")
{

}

CWardWizRegistryOpt::~CWardWizRegistryOpt()
{
}

/***************************************************************************************************
*  Function Name  : On_OnStartRegistryOpt
*  Description    : Accepts the request from UI and starts the Registry Optimizer
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
json::value CWardWizRegistryOpt::On_StartRegistryOpt(SCITER_VALUE svArrRegOptSelectedEntries)
{
	try
	{
		REGOPTSCANOPTIONS	StatOptions = { 0 };
		bool bIsArray = false;
		theApp.m_bIsRegOptUIReceptive = true;
		m_bIsMultiRegOptFinish = false;
		svArrRegOptSelectedEntries.isolate();
		bIsArray = svArrRegOptSelectedEntries.is_array();
		if (!bIsArray)
		{
			return false;
		}
		for (unsigned iCurrentValue = 0, count = svArrRegOptSelectedEntries.length(); iCurrentValue < count; iCurrentValue++)
		{
			const SCITER_VALUE EachEntry = svArrRegOptSelectedEntries[iCurrentValue];
			const std::wstring Key = EachEntry[L"Key"].get(L"");
			bool bValue = EachEntry[L"Val"].get(false);
			switch (iCurrentValue)
			{
			case 0:
				if (bValue)
					StatOptions.bActiveX = true;
				break;
			case 1:
				if (bValue)
					StatOptions.bUninstall = true;
				break;
			case 2:
				if (bValue)
					StatOptions.bFont = true;
				break;
			case 3:
				if (bValue)
					StatOptions.bSharedLibraries = true;
				break;
			case 4:
				if (bValue)
					StatOptions.bApplicationPaths = true;
				break;
			case 5:
				if (bValue)
					StatOptions.bHelpFiles = true;
				break;
			case 6:
				if (bValue)
					StatOptions.bStartup = true;
				break;
			case 7:
				if (bValue)
					StatOptions.bServices = true;
				break;
			case 8:
				if (bValue)
					StatOptions.bExtensions = true;
				break;
			case 9:
				if (bValue)
					StatOptions.bRootKit = true;
				break;
			case 10:
				if (bValue)
					StatOptions.bRogueApplications = true;
				break;
			case 11:
				if (bValue)
					StatOptions.bWorm = true;
				break;
			case 12:
				if (bValue)
					StatOptions.bSpywares = true;
				break;
			case 13:
				if (bValue)
					StatOptions.bAdwares = true;
				break;
			case 14:
				if (bValue)
					StatOptions.bKeyLogger = true;
				break;
			case 15:
				if (bValue)
					StatOptions.bBHO = true;
				break;
			case 16:
				if (bValue)
					StatOptions.bExplorer = true;
				break;
			case 17:
				if (bValue)
					StatOptions.bIExplorer = true;
				break;
			default:
				break;
			}
		}
		g_StatOptions = StatOptions;
		StartRegistryOptScan(&StatOptions);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::On_StartRegistryOpt", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : GetRegistryOptionList
*  Description    : Get the data(registry options) to store in member variable
*  Author Name    : Amol Jaware
*  Date			  : 26 March 2018
****************************************************************************************************/
void CWardWizRegistryOpt::GetRegistryOptionList(CString csCommandLine)
{
	try
	{
		m_csCommandLine = csCommandLine;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::GetRegistryOptionList", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_StartRegistryOpt4SchedScan
*  Description    : Start registry optimer by giving data.
*  Author Name    : Amol Jaware
*  Date			  : 26 March 2018
****************************************************************************************************/
json::value CWardWizRegistryOpt::On_StartRegistryOpt4SchedScan()
{
	try
	{
		REGOPTSCANOPTIONS	StatOptions = { 0 };
		CString csField = L"";
		int iIndex = 0;
		theApp.m_bIsRegOptUIReceptive = true;

		while (AfxExtractSubString(csField, m_csCommandLine, iIndex, _T('|')))
		{

			switch (iIndex)
			{
			case 0:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bActiveX = true;
				break;
			case 1:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bUninstall = true;
				break;
			case 2:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bFont = true;
				break;
			case 3:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bSharedLibraries = true;
				break;
			case 4:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bApplicationPaths = true;
				break;
			case 5:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bHelpFiles = true;
				break;
			case 6:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bStartup = true;
				break;
			case 7:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bServices = true;
				break;
			case 8:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bExtensions = true;
				break;
			case 9:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bRootKit = true;
				break;
			case 10:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bRogueApplications = true;
				break;
			case 11:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bWorm = true;
				break;
			case 12:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bSpywares = true;
				break;
			case 13:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bAdwares = true;
				break;
			case 14:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bKeyLogger = true;
				break;
			case 15:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bBHO = true;
				break;
			case 16:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bExplorer = true;
				break;
			case 17:
				if (csField.Compare(TEXT("1")) == 0)
					StatOptions.bIExplorer = true;
				break;
			default:
				break;
			}
			iIndex++;
		}

		g_StatOptions = StatOptions;
		//m_svFunSetRegistryScanStatusCB = svFunSetScanStatusCB;
		StartRegistryOptScan(&StatOptions);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::On_StartRegistryOpt4SchedScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value (0);
}

/***************************************************************************************************
*  Function Name  : On_OnStopRegistryOpt
*  Description    : Accepts the request from UI and starts the Registry Optimizer
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
//json::value CWardWizUIDlg::On_OnStopRegistryOpt(sciter::value progressCb, SCITER_VALUE pSetScanStatus)
json::value CWardWizRegistryOpt::On_StopRegistryOpt()
{
	try
	{
		if (StopRegistryOptScan())
			return json::value(1);
		else
			return json::value(0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::On_StopRegistryOpt", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : On_PauseRegistryOpt
*  Description    : Used for pause operation
*  Author Name    : Nitin Kolapkar
*  Date			  : 24 June 2016
****************************************************************************************************/
json::value CWardWizRegistryOpt::On_PauseRegistryOpt()
{
	try
	{
		PauseRegistryOptimizer();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::On_PauseRegistryOpt", 0, 0, true, SECONDLEVEL);
	}
	return sciter::value(0);
}

/***************************************************************************************************
*  Function Name  : On_ResumeRegistryOpt
*  Description    : Used for Resume operation
*  Author Name    : Nitin Kolapkar
*  Date			  : 24 June 2016
****************************************************************************************************/
json::value CWardWizRegistryOpt::On_ResumeRegistryOpt()
{
	try
	{
		ResumeRegistryOptimizer();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in On_ResumeRegistryOpt::On_PauseRegistryOpt", 0, 0, true, SECONDLEVEL);
	}
	return sciter::value(0);
}

void CWardWizRegistryOpt::StartRegistryOptScan(LPREGOPTSCANOPTIONS lpRegOpt)
{
	DWORD dwScanOption = 0;
	m_dwPercentage = 0x00;
	m_dwTotalEntries = 0x00;

	GetDWORDFromScanOptions(dwScanOption, lpRegOpt);
	AddLogEntry(L">>> Sending START_REGISTRY_OPTIMIZER to services", 0, 0, true, FIRSTLEVEL);
	
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));
	szPipeData.iMessageInfo = START_REGISTRY_OPTIMIZER;
	szPipeData.dwValue = dwScanOption;

	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data", 0, 0, true, SECONDLEVEL);
		return;
	}

	DWORD dwret = m_objiTinServerMemMap_Client.OpenServerMemoryMappedFile();

	if (dwret)
	{
		AddLogEntry(L"### Error in OpenServerMemoryMappedFile in CRegistryOptimizerDlg::OnBnClickedBtnScanrepair", 0, 0, true, SECONDLEVEL);
	}

	m_hRegOptThread = NULL; 
	m_hRegOptThread = ::CreateThread(NULL, 0, ScanRepairRegistryEntriesThread, (LPVOID) this, 0, NULL);
	Sleep(100);
}


/**********************************************************************************************************
*  Function Name  :	ScanRepairRegistryEntriesThread
*  Description    :	Gets the scan percentage and total entries
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 28th April 2015
**********************************************************************************************************/
DWORD WINAPI ScanRepairRegistryEntriesThread(LPVOID lpParam)
{
	try
	{
		CWardWizRegistryOpt *pRegOptDlg = (CWardWizRegistryOpt *)lpParam;

		CString csPercent = L"";
		CString csTotalEntries = L"";
		pRegOptDlg->m_dwPercentage = 0;
		pRegOptDlg->m_dwTotalEntries = 0;
		if (!pRegOptDlg)
			return 1;

		while (true)
		{
			ITIN_MEMMAP_DATA iTinMemMap = { 0 };
			pRegOptDlg->m_objiTinServerMemMap_Client.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));
			
			pRegOptDlg->m_dwPercentage = iTinMemMap.dwFirstValue;
			pRegOptDlg->m_dwTotalEntries = iTinMemMap.dwSecondValue;
			csPercent.Format(L"%d", pRegOptDlg->m_dwPercentage);
			csTotalEntries.Format(L"%d", pRegOptDlg->m_dwTotalEntries);
			pRegOptDlg->callUISetRegistryStatusfunction(csPercent, csTotalEntries);
			pRegOptDlg->OnCallSetPercentage(csPercent, csTotalEntries);
		}

		Sleep(30);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::ScanRepairRegistryEntriesThread.", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


/**********************************************************************************************************
*  Function Name  :	GetDWORDFromScanOptions
*  Description    :	Gets DWORD value from selected registry scan areas
*  SR.NO          :
*  Author Name    : Vilas
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CWardWizRegistryOpt::GetDWORDFromScanOptions(DWORD &dwRegScanOpt, LPREGOPTSCANOPTIONS lpRegOpt)
{
	DWORD	dwValue = 0x00;
	DWORD	dwTemp = 0x00;

	for (DWORD i = 0x00; i<0x12; i++)
	{
		switch (i)
		{
		case 0:
			if (lpRegOpt->bActiveX == true)
			{
				dwValue = dwTemp = 0x01;
			}
			break;
		case 1:
			if (lpRegOpt->bUninstall == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 2:
			if (lpRegOpt->bFont == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 3:
			if (lpRegOpt->bSharedLibraries == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 4:
			if (lpRegOpt->bApplicationPaths == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 5:
			if (lpRegOpt->bHelpFiles == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 6:
			if (lpRegOpt->bStartup == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 7:
			if (lpRegOpt->bServices == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 8:
			if (lpRegOpt->bExtensions == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 9:
			if (lpRegOpt->bRootKit == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 10:
			if (lpRegOpt->bRogueApplications == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 11:
			if (lpRegOpt->bWorm == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 12:
			if (lpRegOpt->bSpywares == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 13:
			if (lpRegOpt->bAdwares == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 14:
			if (lpRegOpt->bKeyLogger == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 15:
			if (lpRegOpt->bBHO == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 16:
			if (lpRegOpt->bExplorer == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		case 17:
			if (lpRegOpt->bIExplorer == true)
			{
				dwTemp = 1 << i;
				dwValue = dwValue + dwTemp;
			}
			break;
		}

	}
	dwRegScanOpt = dwValue;
}


bool CWardWizRegistryOpt::StopRegistryOptScan()
{

	if (m_hRegOptThread != NULL)
	{
		if (::TerminateThread(m_hRegOptThread, 0x00) == FALSE)
		{
			CString csErrorMsg = L"";
			DWORD ErrorCode = GetLastError();
			csErrorMsg.Format(L"### Failed to terminate ScanRepairRegistryOptimizer thread in ShutdownScanning with GetLastError code %d", ErrorCode);
			AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
		}
		m_hRegOptThread = NULL;
	}
	else
	{
		return false;
	}

	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));
	szPipeData.iMessageInfo = STOP_REGISTRY_OPTIMIZER;

	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data STOP_REGISTRY_OPTIMIZER", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to ReadData in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	EnterRegistryOptimizerDetails
*  Description    :	Function to generate query to add registry optimizer details into database
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizRegistryOpt::EnterRegistryOptimizerDetails(DWORD dwTotalFileCount, DWORD dwRepairedFileCount)
{
	CString csInsertQuery = _T("INSERT INTO Wardwiz_RegistryOptimizerDetails VALUES (null,");

	try
	{
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_RegistryOptimizerDetails VALUES (null,datetime('now','localtime'),date('now'), datetime('now','localtime'),date('now'),%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d );"), dwTotalFileCount, dwRepairedFileCount, g_StatOptions.bActiveX, g_StatOptions.bUninstall, g_StatOptions.bFont, g_StatOptions.bSharedLibraries, g_StatOptions.bApplicationPaths, g_StatOptions.bHelpFiles, g_StatOptions.bStartup, g_StatOptions.bServices, g_StatOptions.bExtensions, g_StatOptions.bRootKit, g_StatOptions.bRogueApplications, g_StatOptions.bWorm, g_StatOptions.bSpywares, g_StatOptions.bAdwares, g_StatOptions.bKeyLogger, g_StatOptions.bBHO, g_StatOptions.bExplorer, g_StatOptions.bIExplorer);

		CT2A ascii(csInsertQuery, CP_UTF8);
		CString	csWardWizModulePath = theApp.GetModuleFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);

		if (!PathFileExists(csWardWizReportsPath))
		{
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			objSqlDb.Close();
		}
		objSqlDb.Open();
		int iRows = objSqlDb.ExecDML(ascii.m_psz);
		objSqlDb.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::EnterRegistryOptimizerDetails. Query is : ", csInsertQuery, 0, true, SECONDLEVEL);
	}
}

void CWardWizRegistryOpt::ScanningStopped()
{
	try
	{
		m_bIsMultiRegOptFinish = true;
		if (theApp.m_bIsRegOptUIReceptive)
		{
			OnCallSetFinishStatus();
		}
		EnterRegistryOptimizerDetails(m_dwTotalEntries, m_dwTotalEntries);
		if (m_hRegOptThread != NULL)
		{
			if (::TerminateThread(m_hRegOptThread, 0x00) == FALSE)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to terminate ScanRepairRegistryOptimizer thread in ShutdownScanning with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
			m_hRegOptThread = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::ScanningStopped.", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetRegistryStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
void CWardWizRegistryOpt::callUISetRegistryStatusfunction(CString csPercent, CString csTotalEntries)
{
	try
	{
		m_svFunSetRegistryScanStatusCB.call(SCITER_STRING(csPercent), SCITER_STRING(csTotalEntries));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::callUISetRegistryStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	PauseRegistryOptimizer
*  Description    :	Pause RegistryOptimizer if user clicks on close button
*  SR.N0		  :
*  Author Name    : Nitin K.
*  Date           : 24th June 2016
**********************************************************************************************************/
bool CWardWizRegistryOpt::PauseRegistryOptimizer()
{
	try
	{
		if (m_hRegOptThread)
		{
			if (::SuspendThread(m_hRegOptThread) == -1)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to suspend ScanRepairRegistryOptimizer thread in CWardWizRegistryOpt::PauseRegistryOptimizer with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
		}

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = PAUSE_REGISTRY_OPTIMIZER;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data PAUSE_REGISTRY_OPTIMIZER", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CVibraniumRegistryOpt::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		AddLogEntry(L">>> Registry optimization paused.", 0, 0, true, FIRSTLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::PauseRegistryOptimizer", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ResumeRegistryOptimizer
*  Description    :	Resume RegistryOptimizer if user clicks on close button
*  SR.N0		  :
*  Author Name    : Nitin K
*  Date           : 24th June 2016
**********************************************************************************************************/
bool CWardWizRegistryOpt::ResumeRegistryOptimizer()
{
	try
	{
		if (m_hRegOptThread)
		{
			if (::ResumeThread(m_hRegOptThread) == -1)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to Resume ScanRepairRegistryOptimizer thread in CRegistryOptimizerDlg::ResumeRegistryOptimizer with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
		}

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = RESUME_REGISTRY_OPTIMIZER;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data RESUME_REGISTRY_OPTIMIZER", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CVibraniumRegistryOpt::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		AddLogEntry(L">>> Registry optimization resumed.", 0, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::ResumeRegistryOptimizer", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	On_funClickShowDetails
*  Description    :	Show details of RegistryOptimizer repqired file's if view details button clicked
*  Author Name    : Amol J.
*  Date           : 13th April 2017
**********************************************************************************************************/
json::value CWardWizRegistryOpt::On_funClickShowDetails()
{
	try
	{
		CString szFullPath;
		TCHAR m_szWindow[MAX_PATH] = { 0 };
		CString	csRegLogPath = L"";
		CString	csWardWizModulePath = theApp.GetModuleFilePath();
		HWND hWindow = ::FindWindow(NULL, L"REGISTRYOPTIMIZER.LOG - Notepad");
		if (hWindow)
		{
			BringWindowToTop(hWindow);
			SetForegroundWindow(hWindow);
			ShowWindow(hWindow, SW_RESTORE);
			return 0;
		}
		GetEnvironmentVariable(TEXT("windir"), m_szWindow, MAX_PATH);//windows 
		szFullPath.Format(L"%s\\System32\\notepad.exe", m_szWindow);
		csRegLogPath.Format(L"%s\\LOG\\REGISTRYOPTIMIZER.LOG", csWardWizModulePath);
		ShellExecute(NULL, L"open", szFullPath, (LPCWSTR)csRegLogPath, NULL, SW_SHOW);
	}
	catch (...)
	{
		AddLogEntry(L">>>### Exception in CVibraniumRegistryOpt::On_funClickShowDetails", 0, 0, true, ZEROLEVEL);
	}
	return 0;

}

/***********************************************************************************************
Function Name  : On_CallContinueFullScan
Description    : used to continue registry optimizer scan
SR.NO		   :
Author Name    : Kunal Waghmare
Date           : 25th April 2019
***********************************************************************************************/
json::value CWardWizRegistryOpt::On_CallContinueRegOptScan()
{
	try
	{
		theApp.m_bIsRegOptUIReceptive = false;
		theApp.m_bIsRegOptPageSwitched = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::On_CallContinueRegOptScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : OnCallSetPercentage
Description    : Called to set scan percentage
SR.NO		   :
Author Name    : Kunal Waghmare
Date           : 25th April 2019
***********************************************************************************************/
void CWardWizRegistryOpt::OnCallSetPercentage(CString csPercentage, CString csTotalEntries)
{
	try
	{
		if (theApp.m_bIsRegOptUIReceptive)
		{
			sciter::value map;
			map.set_item("TotalPercent", sciter::string(csPercentage));
			map.set_item("TotalRepairedCnt", sciter::string(csTotalEntries));
			
			//Send here event
			ela = self;
			BEHAVIOR_EVENT_PARAMS params;
			params.cmd = SETREGOPTSTATUS_EVENT_CODE;
			params.he = params.heTarget = ela;
			params.data = map;
			ela.fire_event(params, true);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::OnCallSetPercentage", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : OnSetTimer
Description    : Called when scan is to be continued
SR.NO		   :
Author Name    : Kunal Waghmare
Date           : 25th April 2019
***********************************************************************************************/
json::value CWardWizRegistryOpt::OnSetTimer()
{
	try
	{
		theApp.m_bIsRegOptUIReceptive = true; 
		if (m_bIsMultiRegOptFinish)
		{
			OnCallSetFinishStatus();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::OnSetTimer", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : OnCallSetFinishStatus
Description    : Called when scan is finished
SR.NO		   :
Author Name    : Kunal Waghmare
Date           : 25th April 2019
***********************************************************************************************/
void CWardWizRegistryOpt::OnCallSetFinishStatus()
{
	try
	{
		CString csTotalEntries = L"";
		csTotalEntries.Format(L"%d", m_dwTotalEntries);
		sciter::value map;
		map.set_item("TotalRepairedCnt", sciter::string(csTotalEntries));
		
		//Send here event
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETREGOPTFINISHED_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistryOpt::OnCallSetFinishStatus", 0, 0, true, SECONDLEVEL);
	}
}