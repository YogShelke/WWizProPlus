#include "stdafx.h"
#include "WardwizScanner.h"
#include <Psapi.h>
#include "WrdwizEncDecManager.h"
#include "ISpyCommServer.h"
#include "iSpyMemMapClient.h"
#include "iTinEmailContants.h"
#include "Enumprocess.h"
#include "WrdWizSystemInfo.h"
#include <vector>

DWORD WINAPI StatusEntryThread(LPVOID lpvThreadParam);
DWORD WINAPI VirusFoundEntriesThread(LPVOID lpvThreadParam);
DWORD WINAPI QuarantineThread(LPVOID lpParam);
DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam);
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam);

std::vector<CString>vecListOfProcessToScan;

//CISpyCommunicatorServer  m_objCom(SERVERCALL, CWardwizScanner::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));

BOOL					g_bIsScanning;
CString					g_csPreviousFile = L"";
CString					g_csPreviousFilePath = L"";
CString					g_csPreviousStatus = L"";
CString					g_csPreviousVirusFoundPath = L"";

/**********************************************************************************************************
*  Function Name  :	OnDataReceiveCallBack
*  Description    :	To receive notification as soon as registration completed.
*  Author Name    : Prajkta Nanaware
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0064
**********************************************************************************************************/
void CWardwizScanner::OnDataReceiveCallBack(LPVOID lpParam)
{
	__try
	{
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		switch (lpSpyData->iMessageInfo)
		{
		case RELOAD_REGISTARTION_DAYS:
			//theApp.GetDaysLeft();
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{

	}
}
CWardwizScanner::CWardwizScanner(void):
m_objCom(SERVICE_SERVER, true,3)
, m_objIPCClient(FILESYSTEMSCAN)
, m_objIPCClientVirusFound(VIRUSFOUNDENTRY)
, m_objScanCom(SERVICE_SERVER, true,3)
{
	m_objIPCClient.OpenServerMemoryMappedFile();
	m_objIPCClientVirusFound.OpenServerMemoryMappedFile();
	EnumProcessModulesWWizEx = NULL;
	m_hPsApiDLL = NULL;
	m_bThreatDetected = false;
	LoadPSAPILibrary();

}
CWardwizScanner::~CWardwizScanner(void)
{
	if (m_hPsApiDLL != NULL)
	{
		FreeLibrary(m_hPsApiDLL);
		m_hPsApiDLL = NULL;
	}
}


/***************************************************************************
Function Name  : LoadPSAPILibrary
Description    : Load PSAPI.DLL.
For Issue: In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 04th Feb 2015
****************************************************************************/
void CWardwizScanner::LoadPSAPILibrary()
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
		AddLogEntry(L"### Exception in CGenXScan::LoadPSAPILibrary", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	VirusFoundEntriesThread
*  Description    : ThreadProc which reads data of virus found entries from shared memory and sends details on Scan dialog UI
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
DWORD WINAPI VirusFoundEntriesThread(LPVOID lpvThreadParam)
{
	//AddLogEntry(L">>> In VirusFoundEntriesThread", 0, 0, true, FIRSTLEVEL);
	CWardwizScanner *pThis = (CWardwizScanner*)lpvThreadParam;

	while (true)
	{
		if (pThis->m_bStop)
			break;
		CString csCurrentPath(L"");

		ITIN_MEMMAP_DATA iTinMemMap = { 0 };
		pThis->m_objIPCClientVirusFound.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));

		switch (iTinMemMap.iMessageInfo)
		{
		case SHOWVIRUSFOUND:
		{
			csCurrentPath.Format(L"%s", iTinMemMap.szSecondParam);
			if (csCurrentPath.GetLength() > 0 && (g_csPreviousVirusFoundPath != csCurrentPath))
			{
				OutputDebugString(csCurrentPath);
				CString csSpyID;
				csSpyID.Format(L"%d", iTinMemMap.dwFirstValue);
				//Insert Virus found entry into List control 
				//pThis->InsertItem(iTinMemMap.szFirstParam, csCurrentPath, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"), csSpyID);
				//pThis->CallUISetVirusFoundEntryfunction(iTinMemMap.szFirstParam, csCurrentPath, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"), csSpyID);
			}
			g_csPreviousVirusFoundPath.SetString(csCurrentPath);
		}
		break;
		}
		csCurrentPath.ReleaseBuffer();
		Sleep(5);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	StatusEntryThread
*  Description    :	A thread which read data from shared memory and shows on Scan dialog UI
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
DWORD WINAPI StatusEntryThread(LPVOID lpvThreadParam)
{
	//AddLogEntry(L">>> In StatusEntryThread", 0, 0, true, FIRSTLEVEL);

	CWardwizScanner *pThis = (CWardwizScanner*)lpvThreadParam;
	while (true)
	{
		if (pThis->m_bStop)
			break;
		CString csCurrentStatus(L"");

		ITIN_MEMMAP_DATA iTinMemMap = { 0 };
		pThis->m_objIPCClient.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));

		switch (iTinMemMap.iMessageInfo)
		{
		case SETSCANSTATUS:
		{
			csCurrentStatus = iTinMemMap.szFirstParam;
			pThis->m_virusFound = iTinMemMap.dwSecondValue;

			if (pThis->m_bIsMemScan)
			{
				if (pThis->m_FileScanned <= pThis->m_iMemScanTotalFileCount || pThis->m_iMemScanTotalFileCount == 0)
				{
					CString csTotalFileCount; csTotalFileCount.Format(L"%d", pThis->m_dwTotalFileCount);

					pThis->m_FileScanned = iTinMemMap.dwFirstValue;
					CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", pThis->m_FileScanned);

					//pThis->CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
					//AddLogEntry(L">>> File Scan status: %s %s", csTotalFileCount, csCurrentFileCount, true, ZEROLEVEL);
				}
			}
			else if (pThis->m_FileScanned <= pThis->m_dwTotalFileCount || pThis->m_dwTotalFileCount == 0)
			{
				pThis->m_FileScanned = iTinMemMap.dwFirstValue;
				CString csTotalFileCount; csTotalFileCount.Format(L"%d", pThis->m_dwTotalFileCount);

				pThis->m_FileScanned = iTinMemMap.dwFirstValue;
				CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", pThis->m_FileScanned);
				//pThis->CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
			}

			if (csCurrentStatus.GetLength() > 0 && (g_csPreviousStatus != csCurrentStatus))
			{
				OutputDebugString(csCurrentStatus);

				//pThis->CallUISetStatusfunction(csCurrentStatus);
				g_csPreviousStatus.SetString(csCurrentStatus);
			}
		}
		break;
		case DISABLE_CONTROLS:
			pThis->m_virusFound = iTinMemMap.dwSecondValue;

			break;
		}
		csCurrentStatus.ReleaseBuffer();
		Sleep(5);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	InvokeRecoverOperationFromCommService
*  Description    :	Send a request to stored data into recover db.So that user can recover file.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardwizScanner::InvokeRecoverOperationFromCommService(int dwMessageinfo, CString csRecoverFileEntry, DWORD dwType, bool bWait, bool bReconnect)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	szPipeData.iMessageInfo = dwMessageinfo;
	_tcscpy_s(szPipeData.szFirstParam, csRecoverFileEntry);
	szPipeData.dwValue = dwType;


	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		//AddLogEntry(L"### Failed to send data in CWardwizScanner::InvokeRecoverOperationFromCommService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			//AddLogEntry(L"### Failed to Read data in CWardwizScanner::InvokeRecoverOperationFromCommService", 0, 0, true, SECONDLEVEL);
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
*  Function Name  :	InvokeRepairFileFromCommService
*  Description    :	Sends data to service to repair a file and service reply the status to GUI
*  SR.NO		  :
*  Author Name    : Vilas Suvarnakar
*  Date           : 07 April 2015
**********************************************************************************************************/
bool CWardwizScanner::InvokeRepairFileFromCommService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect)
{
 try
	{
		if (!m_objCom.SendData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			//AddLogEntry(L"### Failed to send data in CWardwizScanner::InvokeRepairFileFromCommService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				//AddLogEntry(L"### Failed to Read data in CWardwizScanner::InvokeRepairFileFromCommService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
	//	AddLogEntry(L"### Exception in CWardwizScanner::InvokeRepairFileFromCommService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	InvokeRepairFileFromCommService
*  Description    :	Send request to clean file to service
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardwizScanner::InvokeRepairFileFromCommService(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait, bool bReconnect)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = iMessage;
	szPipeData.dwValue = dwISpyID;
	wcscpy_s(szPipeData.szFirstParam, csThreatPath);
	wcscpy_s(szPipeData.szSecondParam, csThreatName);

	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		//AddLogEntry(L"### Failed to send data in CWardwizScanner::InvokeRepairFileFromCommService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			//AddLogEntry(L"### Failed to ReadData in CWardwizScanner::InvokeRepairFileFromCommService", 0, 0, true, SECONDLEVEL);
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
*  Function Name  :	InvokeSetRegistrykeyCommService 
*  Description    :	Set any dword value into given key into registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardwizScanner::InvokeSetRegistrykeyCommService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName);
	szPipeData.dwSecondValue = dwData;

	//CISpyCommunicator objCom(SERVICE_SERVER, true);
	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		//AddLogEntry(L"### Failed to send data in CWardwizScanner : InvokeSetRegistrykeyCommService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			//AddLogEntry(L"### Failed to send data in CWardwizScanner : InvokeSetRegistrykeyCommService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}


/***************************************************************************
Function Name  : IsDuplicateModule
Description    : Function to find duplicates modules to avoid multiple scanning.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 24th Jan 2015
****************************************************************************/
bool CWardwizScanner::IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize)
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
		//AddLogEntry(L"### Exception in CClamScanner::IsDuplicateModule", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
/**********************************************************************************************************
*  Function Name  :	GetModuleCount
*  Description    :	Give total modules of proesses in the case of quick scan
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardwizScanner::GetModuleCount()
{
	try
	{
		//AddLogEntry(L"#I am in CWardwizScanner::GetModuleCount()", 0, 0, true, SECONDLEVEL);
		DWORD	dwPID[0x100] = { 0 };
		DWORD	dwBytesRet = 0x00, dwProcIndex = 0x00;
		m_csaModuleList.RemoveAll();
		EnumProcesses(dwPID, 0x400, &dwBytesRet);
		dwBytesRet = dwBytesRet / sizeof(DWORD);

		//First Set total number of files count.
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
				//if (!EnumProcessModulesWWizEx(hProcess, hMods, sizeof(hMods), &dwModules, LIST_MODULES_ALL))
				if (!EnumProcessModulesWWizEx(hProcess, hMods, sizeof(hMods), &dwModules, LIST_MODULES_DEFAULT))
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
		AddLogEntry(L"### Exception in CWardWizScan::GetModuleCount", 0, 0, true, SECONDLEVEL);
	}
}
/***************************************************************************************************
*  Function Name  : StartCustomScan
*  Description    : Starts Custom Scan functionality
*  Author Name    :
*  Date			  :
****************************************************************************************************/
bool CWardwizScanner::StartCustomScan(CStringArray &csaAllScanPaths)
{
	m_iTotalFileCount = 0;
	m_dwTotalFileCount = 0;
	m_iMemScanTotalFileCount = 0;
	m_dwVirusFoundCount = 0;

	try
	{
		m_eScanType = CUSTOMSCAN;
		m_csaAllScanPaths.Copy(csaAllScanPaths);

		GetModuleCount();

		m_hThread_ScanCount = ::CreateThread(NULL, 0, GetTotalScanFilesCount, (LPVOID) this, 0, NULL);
		Sleep(500);

		DWORD m_dwThreadId = 0;
		m_hWardWizAVThread = ::CreateThread(NULL, 0, EnumerateThread, (LPVOID) this, 0, &m_dwThreadId);


		Sleep(500);
		// suspend main thread until new thread completes
		WaitForSingleObject(m_hWardWizAVThread, 60000);

		//std::vector<CString>::iterator itrProcName;
		//for (itrProcName = vecListOfProcessToScan.begin(); itrProcName < vecListOfProcessToScan.end(); ++itrProcName)
		//{
		//	CString csProcessPath = *itrProcName;
		//	AddLogEntry(L"$$ File to be scanned is: %s ", csProcessPath, 0, true, SECONDLEVEL);

		//	ScanForSingleFile(csProcessPath);
		//}

		m_tsScanStartTime = CTime::GetCurrentTime();
		m_tsScanPauseResumeElapsedTime -= m_tsScanPauseResumeElapsedTime;
		return TRUE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartScanning", 0, 0, true, SECONDLEVEL);
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : StartQuickScan
*  Description    : Starts Quick scan functionality
*  Author Name    : 
*  Date			  : 
****************************************************************************************************/
bool CWardwizScanner::StartQuickScan()
{
	m_iTotalFileCount = 0;
	m_dwTotalFileCount = 0;
	m_iMemScanTotalFileCount = 0;
	m_dwVirusFoundCount = 0;

	try
	{
		m_eScanType = QUICKSCAN;
		if (!GetScanningPaths(m_csaAllScanPaths))
		{
			return FALSE;
		}

		//Check for DB files at installation path
		//if (!Check4DBFiles())
		//{
		//	m_bQuickScan = false;
		//	m_bFullScan = false;
		//	m_bCustomscan = false;
		//	return FALSE;
		//}

		GetModuleCount();

		//m_hThread_ScanCount = ::CreateThread(NULL, 0, GetTotalScanFilesCount, (LPVOID) this, 0, NULL);
		//Sleep(500);

		DWORD m_dwThreadId = 0, exThread =0;
		m_hWardWizAVThread = ::CreateThread(NULL, 0, EnumerateThread, (LPVOID) this, 0, &m_dwThreadId);

		Sleep(500); 
		
		// suspend main thread until new thread completes
		WaitForSingleObject(m_hWardWizAVThread, INFINITE);


		// examine create thread response
		if (!GetExitCodeThread(m_hWardWizAVThread, &exThread)) {
			printf(("GetExitCodeThread %08X\n"), GetLastError());
		}

		// close thread handle
		CloseHandle(m_hWardWizAVThread);
		//DWORD dwError = 0;

		//// examine create thread response
		//if (!GetExitCodeThread(m_hWardWizAVThread, &exThread)) {
		//	//printf(_T("GetExitCodeThread %08X\n"), GetLastError());
		//	dwError = GetLastError();
		//}

		//// close thread handle
		//CloseHandle(m_hWardWizAVThread);		// close thread handle
		//CloseHandle(m_hThread_ScanCount);


		//vecListOfProcessToScan.push_back(csProcessPath);
	/*	std::vector<CString>::iterator itrProcName;
		for (itrProcName = vecListOfProcessToScan.begin(); itrProcName < vecListOfProcessToScan.end(); ++itrProcName)
		{
			CString csProcessPath = *itrProcName;
			AddLogEntry(L"$$ Process to be scanned is: %s ", csProcessPath, 0, true, SECONDLEVEL);

			ScanForSingleFile(csProcessPath);
		}
*/
		m_tsScanStartTime = CTime::GetCurrentTime();
		m_tsScanPauseResumeElapsedTime -= m_tsScanPauseResumeElapsedTime;
		return TRUE;
}
catch (...)
{
	AddLogEntry(L"### Exception in CScanDlg::StartScanning", 0, 0, true, SECONDLEVEL);
	return FALSE;
}
return TRUE;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineFiles
*  Description    :	Repaires or quarantines selected files one by one.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
BOOL CWardwizScanner::QuarantineFiles()
{
//	AddLogEntry(L">>> Quarantine started..", 0, 0, true, SECONDLEVEL);

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

		if (!InvokeRecoverOperationFromCommService(RELOAD_DBENTRIES, L"", RECOVER, true, true))
		{
//			AddLogEntry(L"### Error in CWardwizScanner::InvokeRecoverOperationFromCommService RELOAD_DBENTRIES RECOVER", 0, 0, true, SECONDLEVEL);
		}

		if (!InvokeRecoverOperationFromCommService(RELOAD_DBENTRIES, L"", REPORTS, true, true))
		{
	//		AddLogEntry(L"### Error in CWardwizScanner::InvokeRecoverOperationFromCommService RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
		}

		dwVirusCount = static_cast<DWORD> (m_vecArrayCleanEntries.size());

		for (DWORD dwCurrentVirusEntry = 0; dwCurrentVirusEntry < dwVirusCount; dwCurrentVirusEntry++)
		{
			const ISPY_PIPE_DATA structEachEntry = m_vecArrayCleanEntries[dwCurrentVirusEntry];
			const std::wstring chThreatName = structEachEntry.szSecondParam;
			const std::wstring chFilePath = structEachEntry.szFirstParam;
			const std::wstring chActionTaken = structEachEntry.szThirdParam;
			const DWORD dwSpyID = structEachEntry.dwValue;

			//bool bValue = structEachEntry.[L"selected"].get(false);
			csVirusEntry.Format(L"%d", dwCurrentVirusEntry);

			csStatus = chActionTaken.c_str();

		//	if (bCheck && (csStatus == m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED") || csStatus == m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE")))// csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND") 
			{
				csThreatName = chThreatName.c_str();
				csThreatPath = chFilePath.c_str();

				DWORD dwISpyID = 0;
				dwISpyID = dwSpyID; 
				if (dwISpyID >= 0)
				{
					if (!PathFileExists(csThreatPath))
					{
						//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND"));
						//AddLogEntry(L"### Error in CWardwizScanner::QuaratineEntries: File %s does not exist", csThreatPath, 0, true, SECONDLEVEL);
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

					bool bSendReapir = InvokeRepairFileFromCommService(&szPipeData, true, true);

					switch (szPipeData.dwValue)
					{
					case 0x00:
						if (dwISpyID > 0)
						{
							//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
							//AddLogEntry(L"### Threat Quarantined, File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);

						}
						else
						{
							//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
							//AddLogEntry(L"### Threat Quarantined, File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						}

						break;

					case 0x04:
						//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR"));
						dwRebootRepair++;
						//AddLogEntry(L"### Repair on Reboot File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;

					case 0x05:
						//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE"));
						dwQuarantine++;
						//AddLogEntry(L"### quarantine File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;
					case 0x08:
						//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_ALREADY_REPAIRED"));
						//AddLogEntry(L"### Already Repaired File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;
					case 0x09:
						//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE"));
						//AddLogEntry(L"### Low disc take a backup of file File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;
					default:
						CString csFailedValue;
						csFailedValue.Format(L"%d", szPipeData.dwValue);
						//AddLogEntry(L"### Repair failed file::%s with Error ::%s", csThreatPath, csFailedValue, true, SECONDLEVEL);
						szPipeData.dwValue = 0x00;
						if (dwISpyID > 0)
						{
							//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
						}
						else
						{
							//m_svSetVirusUpdateStatusCB.call((SCITER_STRING)csVirusEntry, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
						}

						//AddLogEntry(L"### Repair failed File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
					}

					if (szPipeData.dwValue == 0x00)
					{
						dwCleanCount++;
					}
				}
			}
		}


		if (dwVirusCount > 0x00)
		{
			DWORD	dwbVirusFound = 0x01;

			if (dwCleanCount >= dwVirusCount)
				dwbVirusFound = 0x00;

			if (!InvokeSetRegistrykeyCommService(m_csRegKeyPath,	L"VirusFound", REG_DWORD, dwbVirusFound, false))
			{
				//AddLogEntry(L"### Error in Set SetRegistrykeyUsingService VirusFound in CWardWizScan::QuaratineEntries", 0, 0, true, SECONDLEVEL);
			}
		}

		Sleep(5);
		//used 0 as Type for Saving RECOVER DB
		if (!InvokeRepairFileFromCommService(SAVE_RECOVERDB, L"", L"", 0, true, true))
		{
		//	AddLogEntry(L"### Error in CWardWizScan::SendFile4RepairUsingService SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
		}

		Sleep(5);
		//used 0 as Type for Saving RECOVER DB
		if (!InvokeRepairFileFromCommService(SAVE_REPORTS_ENTRIES, L"", L"", 5, true, true))
		{
		//	AddLogEntry(L"### Error in CWardWizScan::SendFile4RepairUsingService SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
		}

		if (m_hQuarantineThread != NULL)
		{
			if (TerminateThread(m_hQuarantineThread, 0) == FALSE)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to Terminate QuarantineThread in CWardwizScanner::CloseCleaning with GetLastError code %d", ErrorCode);
				//AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
			//AddLogEntry(L">>> Terminated QuarantineEntries thread successfully.", 0, 0, true, FIRSTLEVEL);
			m_hQuarantineThread = NULL;
		}
		return TRUE;
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardWizScan::QuaratineEntries", 0, 0, true, SECONDLEVEL);
	}
	return FALSE;
	//AddLogEntry(L">>> Quarantine Finished..", 0, 0, true, SECONDLEVEL);
	//AddLogEntry(L"------------------------------------------------", 0, 0, true, SECONDLEVEL);
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
		CWardwizScanner *pThis = (CWardwizScanner *)lpParam;
		pThis->QuarantineFiles();
		return 1;
	}
	catch (...)
	{
	//	AddLogEntry(L"### Exception in CWardwizScanner::QuarantineThread", 0, 0, true, SECONDLEVEL);
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
		CWardwizScanner *pThis = (CWardwizScanner*)lpvThreadParam;
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
			pThis->EnumFolderForScanning(pThis->m_csaAllScanPaths.GetAt(i));
		}

		//if (!pThis->m_bManualStop)
		//{
		//	ITIN_MEMMAP_DATA iTinMemMap = { 0 };
		//	iTinMemMap.iMessageInfo = DISABLE_CONTROLS;
		//	iTinMemMap.dwSecondValue = pThis->m_iThreatsFoundCount;
		//}

		//if (pThis->m_iThreatsFoundCount == 0 && !pThis->m_bManualStop)
		//{
		//	pThis->AddEntriesInReportsDB(pThis->m_eScanType, L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND"));
		//}

		//pThis->ScanFinished();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXScan::EnumerateThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


/**********************************************************************************************************
*  Function Name  :	EnumFolderForScanning
*  Description    :	enumerate files of system and sent it to scan.
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardwizScanner::EnumFolderForScanning(LPCTSTR pstr)
{
	try
	{
		if (!pstr)
			return;

		CFileFind finder;
		DWORD	dwAttributes = 0;
		CString strWildcard(pstr);

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
			//if file does not exits then return
			if (!PathFileExists(pstr))
			{
				return;
			}

			ScanForSingleFile(pstr);
			//vecListOfProcessToScan.push_back(pstr);
			return;
		}


		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString str = finder.GetFilePath();
				EnumFolderForScanning(str);
			}
			else
			{
				CString csFilePath = finder.GetFilePath();
				if (csFilePath.Trim().GetLength() > 0)
				{
					if (PathFileExists(csFilePath))
					{
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
		AddLogEntry(L"### Exception in CGenXScan::EnumFolderForScanning", 0, 0, true, SECONDLEVEL);
	}
}
//Send the all files path # sign token
/**********************************************************************************************************
*  Function Name  :	MakeFullTokenizedScanPath
*  Description    :	Toknization of scan path
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardwizScanner::MakeFullTokenizedScanPath(CStringArray &csaAllScanPaths, LPTSTR szScanPath)
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

		wcscpy_s(szScanPath,csaAllScanPaths.GetSize(), csaAllScanPaths.GetAt(0));

		for (int iIndex = 1; iIndex < csaAllScanPaths.GetCount(); iIndex++)
		{
			wcscat_s(szScanPath, sizeof(L"#"), L"#");
			wcscat_s(szScanPath, csaAllScanPaths.GetSize(),csaAllScanPaths.GetAt(iIndex));
		}
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CCWardwizScanner::MakeFullTokenizedScanPath", 0, 0, true, SECONDLEVEL);
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
void CWardwizScanner::EnumFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");
		//AddLogEntry(L"### I am inCWardwizScanner::EnumFolder(LPCTSTR pstr)", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CGenXScan::EnumFolder", 0, 0, true, SECONDLEVEL);
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
		CWardwizScanner *pThis = (CWardwizScanner*)lpParam;

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
		AddLogEntry(L"### Exception in CGenXScan::GetTotalScanFilesCount", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


/**********************************************************************************************************
*  Function Name  :	CheckFileIsInRepairRebootIni
*  Description    :	Get Quarantine folder path.
*  Author Name    : Vilas Suvarnakar
*  Date           : 24 March 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardwizScanner::CheckFileIsInRepairRebootIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRepairIniPath))
		{
			//AddLogEntry(L"### Failed in CWardWizScan::CheckFileIsInRepairRebootIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
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
		//AddLogEntry(L"### Exception in CWardWizScan::CheckFileIsInRepairRebootIni", 0, 0, true, SECONDLEVEL);
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
bool CWardwizScanner::CheckFileIsInRecoverIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR szRecoverIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRecoverIniPath))
		{
			//AddLogEntry(L"### Failed in CheckFileIsInRecoverIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
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
		//AddLogEntry(L"### Exception in CWardWizScan::CheckFileIsInRecoverIni", 0, 0, true, SECONDLEVEL);
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
bool CWardwizScanner::CheckFileIsInDeleteIni(CString csQurFilePaths)
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
				//AddLogEntry(L"### CheckFileIsInDeleteIni::TokenizationOfParameterForrecover is not tokenize properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}

			if (!TokenizeIniData(szFileName, szApplnName, _countof(szApplnName)))
			{
				//AddLogEntry(L"### CWardWizScan::CheckFileIsInDeleteIni::TokenizeIniData is not tokenize properly", 0, 0, true, FIRSTLEVEL);
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
		//AddLogEntry(L"### Exception in CWardWizScan::CheckFileIsInDeleteIni", 0, 0, true, SECONDLEVEL);
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
bool CWardwizScanner::GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath)
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
		//AddLogEntry(L"### Exception in CWardWizScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
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
CString CWardwizScanner::GetQuarantineFolderPath()
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
		//AddLogEntry(L"### Exception in CWardWizScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
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
bool CWardwizScanner::TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName)
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
			//AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (pToken)
		{
			wcscpy_s(szFileName, (dwsizeofFileName - 1), pToken);
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			//AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szQuarantinepath, (dwsizeofquarantinefileName - 1), pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			//AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szVirusName, (dwsizeofVirusName - 1), pToken);
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardWizScan::TokenizationOfParameterForrecover", 0, 0, true, SECONDLEVEL);
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
bool CWardwizScanner::TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName)
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
			//AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
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
			//AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		}

		TCHAR	szAttemptCnt[16] = { 0 };

		if (pToken)
			wcscpy_s(szAttemptCnt, _countof(szAttemptCnt), pToken);
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardWizScan::TokenizeIniData", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ScanForSingleFile
*  Description    :	Scan each single file .
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardwizScanner::ScanForSingleFile(CString csFilePath)
{
	AddLogEntry(L" CGenXScanner::ScanForSingleFile : %s ", csFilePath, 0, true, SECONDLEVEL);
	//AddLogEntry(L"### I am in  CWardwizScanner::ScanForSingleFile--", 0, 0, true, SECONDLEVEL);
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

		if (csFilePath.Trim().GetLength() > 0)
		{
			if (m_csPreviousPath.Compare(csFilePath)!=0)
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

				CString csActionID;
				if (InvokeStartScanFromCommService(csFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, dwActionTaken, m_bRescan))
				{
						if (dwISpyID >= 0)
						{
							csVirusName = szVirusName;
							m_csVirusName = szVirusName;
							dwISpywareID = dwISpyID;
							m_bThreatDetected = true;
							m_dwVirusFoundCount++;
						}
					}
					m_FileScanned++;
				}
			}
			m_csPreviousPath = csFilePath;
		}

		//virus found 
		if (bVirusFound)
		{
			bSetStatus = true;
			return;
		}
		else
		{
			bSetStatus = true;
			csStatus = csFilePath;
		}
		return;
	}
	catch (...)
	{
		//m_objSqlDb.Close();
		AddLogEntry(L"### Exception in void CGenXScan::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
		return;
	}
}

/**********************************************************************************************************
*  Function Name  :	EnumerateProcesses
*  Description    :	Enumaerate processes in case of quick scan.
					Changes (Ram) : Time complexity decresed as we enumerating processes and modules
					to calculate file count, There is no need to enumerate it again.
					kept in CStringList while file count calculation, same list is used again.
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardwizScanner::EnumerateProcesses()
{
	try
	{
		//AddLogEntry(L"### I am in  CWardwizScanner::EnumerateProcesses()", 0, 0, true, SECONDLEVEL);
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
				//AddLogEntry(L" CWardwizScanner::EnumerateProcesses()** Before ScanForSingleFile process name is : %s ", csProcessPath, 0, true, SECONDLEVEL);
				ScanForSingleFile(csProcessPath);
				//vecListOfProcessToScan.push_back(csProcessPath);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXScan::EnumerateProcesses", 0, 0, true, SECONDLEVEL);
	}
}
/**********************************************************************************************************
*  Function Name  :	InvokeStartScanFromCommService
*  Description    :	Used named pipe to give signal to service to start scanning .
*  Author Name    : 
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
//bool CWardwizScanner::InvokeStartScanFromCommService(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan)
//{
//	try
//	{
//		AddLogEntry(L"FileName is :  %s ", szFilePath, 0, true, SECONDLEVEL);
//
//		ISPY_PIPE_DATA szPipeData = { 0 };
//		memset(&szPipeData, 0, sizeof(szPipeData));
//		szPipeData.iMessageInfo = SCAN_FILE;
//		wcscpy_s(szPipeData.szFirstParam, szFilePath);
//
//		//AddLogEntry(L" CWardwizScanner::InvokeStartScanFromCommService --szPipeData.szFirstParam is %s ", szPipeData.szFirstParam, 0, true, SECONDLEVEL);
//
//		//CISpyCommunicator objCom(SERVICE_SERVER, true);
//
//		if (!m_objScanCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
//			{
//			AddLogEntry(L"### Failed to send data in CWardwizScanner::InvokeStartScanFromCommService", 0, 0, true, SECONDLEVEL);
//			return false;
//		}
//		//AddLogEntry(L"@@@ AFTER send data in CWardwizScanner::InvokeStartScanFromCommService", 0, 0, true, SECONDLEVEL);
//
//		if (!m_objScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
//		{
//			AddLogEntry(L"### Failed to ReadData in CWardwizScanner::InvokeStartScanFromCommService", 0, 0, true, SECONDLEVEL);
//		}
//		//AddLogEntry(L"@@@ AFTER ReadData in CWardwizScanner::InvokeStartScanFromCommService", 0, 0, true, SECONDLEVEL);
//
//		if (!&szPipeData.dwValue)
//		{
//			AddLogEntry(L" CWardwizScanner::InvokeStartScanFromCommService -- !&szPipeData.dwValue ", 0, 0, true, SECONDLEVEL);
//			return false;
//		}
//		else
//		{
//			g_bIsScanning = true;
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
//	}
//	return true;
//}
bool CWardwizScanner::InvokeStartScanFromCommService(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan)
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
			AddLogEntry(L"### Failed to send data in CGenXScanner::InvokeStartScanFromCommService", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}

		if (!m_objScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CGenXScan::ScanFile", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/**********************************************************************************************************
*  Function Name  :	GetAllDrivesList
*  Description    :	Makes list of drives present on a system.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardwizScanner::GetAllDrivesList(CStringArray &csaReturn)
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
bool CWardwizScanner::Check4ValidDBFiles(CString csDBFolderPath)
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
//			AddLogEntry(L"### Invalid DB found (or) may corrupted, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		//DB Version length should be in between 7 and 19
		//Eg: 1.0.0.0 to 9999.9999.9999.9999
		if (!(dwDBVersionLength >= 7 && dwDBVersionLength <= 19))
		{
			//AddLogEntry(L"### Invalid DB Version length, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAVR);

		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(csFilePath, dwDBVersionLength, dwDBMajorVersion))
		{
			//AddLogEntry(L"### Invalid DB found (or) may corrupted, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		//DB Version length should be in between 7 and 19
		//Eg: 1.0.0.0 to 9999.9999.9999.9999
		if (!(dwDBVersionLength >= 7 && dwDBVersionLength <= 19))
		{
			//AddLogEntry(L"### Invalid DB Version length, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CScanDlg::Check4ValidDBFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/**********************************************************************************************************
*  Function Name	 :	Check4DBFiles
*  Description		 :	Checks for signature db are exist or not
*  Author Name		 :	Ramkrushna Shelke
*  SR_NO		   	 :
*  Date				 : 26 Nov 2013
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
**********************************************************************************************************/
bool CWardwizScanner::Check4DBFiles()
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
	csWRDDBFilesFolderPath += L"\\WRDWIZDATABASE";

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

		if (m_eScanLevel == WARDWIZSCANNER)
		{
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\WRDWIZAV1.DB");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\WRDWIZAVR.DB");
		}
		else
		{
			csaDBFiles.Add(csDBFilesFolderPath + L"\\MAIN.CVD");
			csaDBFiles.Add(csDBFilesFolderPath + L"\\DAILY.CLD");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\WRDWIZAV1.DB");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\WRDWIZAVR.DB");
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
*  Function Name  :	GetScanningPaths
*  Description    :	Get scan path according to scanning types.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
//bool CWardwizScanner::GetScanningPaths(CStringArray &csaReturn)
//{
//	CString cScanPath;
//
//	switch (m_eScanType)
//	{
//
//	case QUICKSCAN:
//		m_bQuickScan = true;
//		csaReturn.RemoveAll();
//		csaReturn.Add(L"QUICKSCAN");
//
//		break;
//	case FULLSCAN:
//		m_bFullScan = true;
//		csaReturn.RemoveAll();
//
//		if (!GetAllDrivesList(csaReturn))
//		{
//			return false;
//		}
//		break;
//	case CUSTOMSCAN:
//	{
//		m_bCustomscan = true;
//	
//		bool bIsArray = false;
//
//		if (m_vecCustomScanSelectedEntries.size() == 0)
//			return false;
//
//		//clear the return string first..
//		csaReturn.RemoveAll();
//
//		//iterate through the vector and setup the CString arry of paths.
//		for(auto &csFilePath :m_vecCustomScanSelectedEntries)
//		{
//			csaReturn.Add(csFilePath);
//		}
//
//	}
//	break;
//	//case USBSCAN:
//	//case USBDETECT:
//		//return OnGetSelection();
//	//	break;
//	}
//	if (csaReturn.GetCount() > 0)
//	{
//		return true;
//	}
//	return false;
//}

bool CWardwizScanner::GetScanningPaths(CStringArray &csaReturn)
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
			csaReturn.Add(L"QUICKSCAN");

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

			csaReturn.RemoveAll();
	/*		for (unsigned iCurrentValue = 0, count = m_svArrCustomScanSelectedEntries.length(); iCurrentValue < count; iCurrentValue++)
			{
				const SCITER_VALUE EachEntry = m_svArrCustomScanSelectedEntries[iCurrentValue];
				const std::wstring chFilePah = EachEntry[L"FilePath"].get(L"");
				bool bValue = EachEntry[L"selected"].get(false);
				if (bValue)
				{
					csaReturn.Add(chFilePah.c_str());
				}
			}*/
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
		AddLogEntry(L"### Exception in CGenXScanner::GetScanningPaths", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}
BOOL CWardwizScanner::StartScan(CStringArray &csaAllScanPaths)
{
	try
	{
		UINT uRetVal = 0;
		m_bQuickScan = true;
		m_bFullScan = false;
		m_bCustomscan = false;
		m_eScanType = m_eCurrentSelectedScanType;
		m_bScnAborted = false;

		if (!GetScanningPaths(m_csaAllScanPaths))
		{
			return false;
		}
		
		////Check for DB files at installation path
		//if (!Check4DBFiles())
		//{
		//	m_bQuickScan = false;
		//	m_bFullScan = false;
		//	m_bCustomscan = false;
		//	return FALSE;
		//}
		//m_dwTotalFileCount = 0;
		//m_iMemScanTotalFileCount = 0;
		
		//if (!InvokeStartScanFromCommService(m_csaAllScanPaths))
		//{
		//	m_bQuickScan = false;
		//	m_bFullScan = false;
		//	m_bCustomscan = false;
		//	return FALSE;
		//}

		//m_hThreadVirEntries = ::CreateThread(NULL, 0, VirusFoundEntriesThread, (LPVOID) this, 0, NULL);
		//Sleep(500);

		//m_hThreadStatusEntry = ::CreateThread(NULL, 0, StatusEntryThread, (LPVOID) this, 0, NULL);
		//Sleep(500);

		//m_tsScanStartTime = CTime::GetCurrentTime();
		//m_tsScanPauseResumeElapsedTime -= m_tsScanPauseResumeElapsedTime;

	}
	catch (...)
	{
	}
	return false;
}

bool CWardwizScanner::SendRequestCommon(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(SERVICE_SERVER, true);

		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			//AddLogEntry(L"### Failed to send data to SERVICE_SERVER in CWardwizScanner::SendRequestCommon", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardwizScanner::SendRequestCommon", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

BOOL CWardwizScanner::PauseScan()
{
	try
	{
		if (SendRequestCommon(PAUSE_SCAN))
		{
			//AddLogEntry(L">>> Scanning Paused..", 0, 0, true, FIRSTLEVEL);
		}
		else
		{
			//AddLogEntry(L"### Failed to pause scan as Send PAUSE_SCAN request failed.", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardwizScanner::PauseScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

BOOL CWardwizScanner::ResumeScan()
{
	try
	{
		if (SendRequestCommon(RESUME_SCAN))
		{
			//AddLogEntry(L">>> Scanning Resumed..", 0, 0, true, FIRSTLEVEL);
		}
		else
		{
			//AddLogEntry(L"### Failed to pause scan as Send RESUME_SCAN request failed.", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardwizScanner::ResumeScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
/**********************************************************************************************************
*  Function Name  :	ShutDownScanning
*  Description    :	Shut down scanning with terminating all threads safely.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CWardwizScanner::ShutDownScanning()
{
	try
	{
		m_bStop = true;

		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = STOP_SCAN;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			//AddLogEntry(L"### Failed to send data in CWardwizScanner::ShutDownScanning", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			//AddLogEntry(L"### Failed to ReadData in CWardwizScanner::ShutDownScanning", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (m_hThreadVirEntries != NULL)
		{
			SuspendThread(m_hThreadVirEntries);
			TerminateThread(m_hThreadVirEntries, 0);
			m_hThreadVirEntries = NULL;
		}

		if (m_hThreadStatusEntry != NULL)
		{
			SuspendThread(m_hThreadStatusEntry);
			TerminateThread(m_hThreadStatusEntry, 0);
			m_hThreadStatusEntry = NULL;
		}
		OutputDebugString(L">>> m_hThreadStatusEntry stopped");
		Sleep(500);
		m_bScnAborted = true;
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardwizScanner::ShutDownScanning", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
BOOL CWardwizScanner::StopScan()
{
	try
	{
		ShutDownScanning();
		return TRUE;
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardwizScanner::StopScan", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

BOOL CWardwizScanner::BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath)
{
	return TRUE;
}


BOOL CWardwizScanner::QuarantineSelectedfile()
{
	return TRUE;
}

BOOL CWardwizScanner::SearchForVirusAndQuarantine()
{
	return TRUE;
}

BOOL CWardwizScanner::RecoverOperations(int dwMessageinfo, CString csRecoverFileEntry, DWORD dwType, bool bWait, bool bReconnect)
{
	return TRUE;
}

BOOL CWardwizScanner::RepairFile(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect)
{
	return TRUE;
}

BOOL CWardwizScanner::RepairFile(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait, bool bReconnect)
{
	return TRUE;
}