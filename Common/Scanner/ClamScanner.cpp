/**********************************************************************************************************                     
	  Program Name          : CClamScanner.cpp
	  Description           : Implementation of Clam AV Scanner functionality 
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 20 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna           Created Wrapper class for Clam AV functionality implementation
***********************************************************************************************************/
#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif

#include <Psapi.h>
#include  <io.h>
#include "ClamScanner.h"
#include "ISpyCommunicator.h"
#include "Enumprocess.h"
#include "ITinRegWrapper.h"
#include "ISpyCriticalSection.h"
#include "CSecure64.h"
#include "WardWizDatabaseInterface.h"

#define TIMER_SCAN_STATUS			100
#define TIMER_PAUSE_HANDLER			101

#define BUFSIZE 2024
#define MAX_OUTPUT 8192

char*  g_strWWDatabaseFile = "\\VBALLREPORTS.DB";
//#define DETECTEDSTATUS	L"Detected"

//const TCHAR * DELETEDSTATUS = L"Quarantined";
//const TCHAR * FILEREPAIRED = L"Repaired";
//const TCHAR * NOTHREATSFOUND = L"No threat(s) found";
//const TCHAR * CANTREPAIRFILE = L"Can't repair";

//#define STATUSFAILED 	L"Failed"
//const unsigned int WRDWIZ_KEY = 0x5757495A;			//WWIZ

BOOL					isScanning;

#define BAIL_OUT(code) { isScanning = FALSE; return code; }
HANDLE hChildStdinWrDup = INVALID_HANDLE_VALUE, hChildStdoutRdDup = INVALID_HANDLE_VALUE;
HANDLE					m_LogMutex;
HANDLE					m_hEvtStop;
PROCESS_INFORMATION		pi;
DWORD					exitcode = 0;
CString					g_csPreviousFile = L"";
CString					g_csPreviousFilePath = L"";

//Thread initialization here
DWORD WINAPI PipeToClamAV(LPVOID lpvThreadParam);
DWORD WINAPI CalculateTotalFilesInFolder(LPVOID lpvThreadParam);
DWORD WINAPI OutputThread(LPVOID lpvThreadParam);
DWORD WINAPI MemoryScanThread(LPVOID lpvThreadParam);

UINT QuarantineThread(LPVOID lpParam);

DWORD WINAPI GetScanFilesCount(LPVOID lpParam ) ;
HANDLE m_hThread_ScanCount = NULL ;
HANDLE m_hClamAVThread = NULL;
HANDLE m_hMemScanThread = NULL;

CClamScanner::CClamScanner(void):
	 m_dwTotalScanPathCount(1)
    , m_bSendScanMessage(true)
	, m_objServ(FILESYSTEMSCAN)
	, m_objServViruEntry(VIRUSFOUNDENTRY)
	, m_iThreatsFoundCount(0)
	, m_ptrISpyDBManipulation(NULL)
	, m_dwTotalFileCount(0)
	, m_dwTotalMemScanFileCount(0)
	, m_bManualStop(false)
	, m_bRescan(false)
	, m_bFailedToLoadSignature(false)
	, m_objCom(UI_SERVER, true)
	, m_hThread_Output(NULL)
	, m_hPsApiDLL(NULL) 
	, m_dwFailedDeleteFileCount(0)
	, m_bFileFailedToDelete(false)
	, m_bMemScanCompleted(false)
	, m_iMemScanFileCount(0)
	, m_bIsClamLoaded(false)
	, m_bIsClamSuspended(false)
	, m_bISOnlyMemScan(false)
{
	m_objServ.CreateServerMemoryMappedFile();
	m_objServViruEntry.CreateServerMemoryMappedFile();

	m_pbyEncDecKey = NULL;
	m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE,sizeof(unsigned char)); 

	EnumProcessModulesWWizEx = NULL;

	m_hEventMemScanFinished = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CClamScanner::~CClamScanner(void)
{
	if(m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}

	//Ramkrushna Shelke Date: 04/02/2015, Version: 1.8.3.8
	//Issue: .In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
	if(m_hPsApiDLL != NULL)
	{
		FreeLibrary(m_hPsApiDLL);
		m_hPsApiDLL = NULL;
	}
}

DWORD WINAPI GetScanFilesCount(LPVOID lpParam )
{
	CClamScanner *pThis = (CClamScanner*)lpParam ;
	if(!pThis)
		return 1 ;

	pThis->m_iTotalFileCount = 0x00 ;

	int	iIndex = 0x00 ;
	iIndex = static_cast<int>(pThis->m_csaAllScanPaths.GetCount());
	if( !iIndex )
		return 2 ;

	for(int i = 0; i < iIndex ; i++)
	{
		pThis->EnumFolder( pThis->m_csaAllScanPaths.GetAt(i) ) ;
	}

	if( pThis->m_iTotalFileCount )
	{
		//set here the progress of Main UI

		//pThis->m_prgScanProgress.ShowWindow( SW_SHOW ) ;
		////pThis->m_ScanPercentage.ShowWindow( SW_SHOW ) ;
		//pThis->m_prgScanProgress.SetRange32( 0, pThis->m_iTotalFileCount ) ;
		pThis->m_ScanCount = true ;
	}
	
	CString csLog;
	csLog.Format(L">>> NONMEMSCAN: Total files to scan: %d", pThis->m_iTotalFileCount);
	AddLogEntry(L"%s", csLog, 0, true, SECONDLEVEL);

	pThis->SendTotalFileCount(pThis->m_iTotalFileCount);

	return 0 ;
}

DWORD WINAPI CalculateTotalFilesInFolder(LPVOID lpvThreadParam)
{
	AddLogEntry(L">>> Calculating total files in folder", 0, 0, true, FIRSTLEVEL);

	CClamScanner *pThis = (CClamScanner*)lpvThreadParam;
	if(!pThis)
		return 0;

	pThis->m_iTotalFileCount = 0;
	for(int iIndex = 0; iIndex < pThis->m_csaAllScanPaths.GetCount() ; iIndex++)
	{
		pThis->EnumFolder(pThis->m_csaAllScanPaths.GetAt(iIndex));
	}
	return 0;
}

DWORD WINAPI PipeToClamAV(LPVOID lpvThreadParam)
{
	CClamScanner *pThis = (CClamScanner*)lpvThreadParam;
	if(!pThis)
		return 0;

    HANDLE	hChildStdoutRd, hChildStdoutWr, hChildStderrWr ;

	//Name: Varada Ikhar
	//Date: 15/01/2015, Version:1.8.3.17
	//Description: In Full Scan, after completing C, D & E drive scanning it again scans C drive recursively.
	//Commented this loop.
	//for(int iIndex = 0; iIndex < pThis->m_csaAllScanPaths.GetCount() ; iIndex++)
	//{

	//issue :Excluding CHM spy dbx files Neha Gharge 9/2/2015
		CString csCommadLine = L"";
		csCommadLine += L"VBSCANNER.EXE ";

		csCommadLine += L"--keep-mbox ";
		csCommadLine += L"--stdout ";
		//csCommadLine += L"--infected ";
		csCommadLine += L"--algorithmic-detection ";
		csCommadLine += L"--max-files=500 ";
		csCommadLine += L"--max-scansize=150M ";
		csCommadLine += L"--max-recursion=50 ";
		csCommadLine += L"--max-filesize=100M ";
		csCommadLine += L"--show-progress ";
		csCommadLine += L"--recursive ";
		//csCommadLine += L"--exclude = \"\\.(manifest)$\" ";
		//csCommadLine += L"--exclude=[^\\]*\\.spy$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.spy$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.dbx$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.tbb$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.pst$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.dat$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.log$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.evt$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.nsf$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.ntf$ ";
		//csCommadLine += L"--exclude=[^\\]*\\.chm$ "; 
#if 0
		csCommadLine += L"--exclude=[^\\]*\\.clamtmp$ ";
#endif
		if(pThis->m_eScanType == QUICKSCAN)
		{
			csCommadLine += L"--kill ";
			csCommadLine += L"--memory";
		}
		else
		{
			//Name: Varada Ikhar, Date:13/01/2015, Version:1.8.3.10, Issue No:6
			//Description: D & E drive is not getting scanned in Full scan.
			for(int jIndex = 0; jIndex < pThis->m_csaAllScanPaths.GetCount() ; jIndex++) 
			{
				csCommadLine += L"--kill \"";
				CString csDriveFormat;
				csCommadLine += pThis->m_csaAllScanPaths.GetAt(jIndex);
				csCommadLine += "\"";
				csCommadLine += " ";
			}
		}

		LPTSTR pszCmdLine = csCommadLine.GetBuffer();

		if (!pszCmdLine) BAIL_OUT(-1);

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		/* Create a pipe for the child process's STDOUT */
		if(!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
			BAIL_OUT(-1);

		/* Duplicate stdout sto stderr */
		if (!DuplicateHandle(GetCurrentProcess(), hChildStdoutWr, GetCurrentProcess(), &hChildStderrWr, 0, TRUE, DUPLICATE_SAME_ACCESS))
			BAIL_OUT(-1);

		/* Duplicate the pipe HANDLE */
		if (!DuplicateHandle(GetCurrentProcess(), hChildStdoutRd, GetCurrentProcess(), &hChildStdoutRdDup, 0, FALSE, DUPLICATE_SAME_ACCESS))
			BAIL_OUT(-1);

		CloseHandle(hChildStdoutRd);

		pThis->WriteStdOut(pszCmdLine, FALSE);
		pThis->WriteStdOut(L"\r\n\r\n", FALSE);

		CString cstemp;
		if(!pThis->LaunchClamAV(pszCmdLine, hChildStdoutWr, hChildStderrWr))
		{
			//KillTimer(pThis->m_hWnd, TIMER_SCAN_STATUS);
			isScanning = FALSE;

			AddLogEntry(L"---------------------------------------------");
			AddLogEntry(L"Scanner not launched.. ");
			MessageBox(NULL, L"Scanner not launched, \nPlease reinstall application", L"Vibranium", MB_ICONINFORMATION);
			//pThis->EnableAllWindows(TRUE);
			return 0;
		}
	
		if (!pThis->SendProcessID2Protect(pi))
		{
			AddLogEntry(L"### SendProcessID2Protect::Failed to Protect scanner.exe", 0, 0, true, SECONDLEVEL);
		}

		//Varada Ikhar, Date:26/02/2015 Issue:Install 1.9.0.0 setup->perform Quick scan->'scanning started' entry is not reflecting in 'comsrv' file only for Quick scan.
		//Varada Ikhar, Date: 18/04/2015
		// Issue : 0000128 :In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
		CString csStartScan = L"";
		switch (pThis->m_eScanType)
		{
		case QUICKSCAN: 
			csStartScan = L">>> Quick scanning started...";
			break;
		case FULLSCAN: 
			csStartScan = L">>> Full scanning started...";
			break;
		case CUSTOMSCAN:
			csStartScan = L">>> Custom scanning started...";
			break;
		default:
			csStartScan = L"Scanning started...";
			break;
		}
		AddLogEntry(csStartScan, 0, 0, true, SECONDLEVEL);

		pThis->m_bSendScanMessage = true;
		BOOL freshclam = TRUE;//!_strnicmp(pszCmdLine, "WelkinScanner", 9);
		//delete pszCmdLine;
		CloseHandle(hChildStdoutWr);

		m_hEvtStop = CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD m_dwThreadId;
		//HANDLE m_hThread = CreateThread(NULL, 0, OutputThread, (LPVOID) pThis, 0, &m_dwThreadId);
		//Added By Nitin K
		//Date: 16 Jan 2015
		//In custom scan, full scan, quick scan, while Pressing the "pause" button the progress bar continues some times.
		//It should be immediately.
		pThis->m_hThread_Output = CreateThread(NULL, 0, OutputThread, (LPVOID) pThis, 0, &m_dwThreadId);

		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &exitcode);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		pi.hProcess = INVALID_HANDLE_VALUE;
		SetEvent(m_hEvtStop);

		Sleep(1000);
	//}

	OutputDebugString(L">>> PipeToClamAV Thread Finished");

    return 0;
}

DWORD WINAPI OutputThread(LPVOID lpvThreadParam)
{
	try
	{
		OutputDebugString(L">>> OutputThread Started");
		CClamScanner *pThis = (CClamScanner*)lpvThreadParam;
		if (!pThis)
			return 0;

		TCHAR msg[128];
		HANDLE Handles[2];
		Handles[0] = pi.hProcess;
		Handles[1] = m_hEvtStop;

		ResumeThread(pi.hThread);

		while (1)
		{
			DWORD dwRc = WaitForMultipleObjects(2, Handles, FALSE, 100);
			pThis->RedirectStdOutput((BOOL)lpvThreadParam);
			if ((dwRc == WAIT_OBJECT_0) || (dwRc == WAIT_OBJECT_0 + 1) || (dwRc == WAIT_FAILED))
				break;
		}
		CloseHandle(hChildStdoutRdDup);
		hChildStdoutRdDup = INVALID_HANDLE_VALUE;
		_stprintf_s(msg, 128, L"\r\nProcess exited with %d code\r\n", exitcode);
		pThis->WriteStdOut(msg, FALSE);

		//bool bFinishedScanning = false;

		//This Check is specially for scan files which are excluded by Clam
		//Because clam scanner does not scan 64bit files, so need to scan using wardwiz scanner.
		//Issue : 10. In Quick scan, If we abort the scan then, Aborted message not coming immediately, and other scan buttons in Tab not getting enabled
		//Resolved By : Nitin K. Date: 03rd - Feb - 2015
		if (pThis->m_eScanType == QUICKSCAN && !pThis->m_bManualStop)
		{
			pThis->ScanFilesExcludedByClam();
		}

		//Varada Ikhar, Date: 30/04/2015
		//Issue : In custom scan, if folder with very few number of files is given, then scanning is not getting paused on click of pause/stop/close button.
		if (!pThis->m_bManualStop)
		{
			ITIN_MEMMAP_DATA iTinMemMap = { 0 };
			iTinMemMap.iMessageInfo = DISABLE_CONTROLS;
			//Issue: Virus found count doesnt show on UI in case of fewer no of files
			//Resolved By: Nitin K Date : 15th May 2015
			iTinMemMap.dwSecondValue = pThis->m_iThreatsFoundCount;
			pThis->m_objServ.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
		}
		//Issue resolved: In Windows 7 Ultimate 32 bit full scan completed pop-up doesn’t appear.
		//As we are giving multiple drives in single instance of scan, so no need to check with scan path count.
		//if(pThis->m_eScanType == QUICKSCAN || pThis->m_eScanType == CUSTOMSCAN || pThis->m_bManualStop ||
		//pThis->m_csaAllScanPaths.GetCount() == pThis->m_dwTotalScanPathCount)
		//{
		//	bFinishedScanning = true;
		//}

		//if(bFinishedScanning)
	{
		isScanning = FALSE;

		if (pThis->m_bFailedToLoadSignature)
		{
			//tray
			pThis->SendMessage2Tray(SCAN_FINISHED_SIGNATUREFAILED);
			//Varada Ikhar, Date: 14/02/2015, Issue: Database needs to be updated.Database not valid.Message displayed on UI.
			pThis->SendMessage2UI(SCAN_FINISHED_SIGNATUREFAILED);
		}

		if (!pThis->SendMessage2UI(SCAN_FINISHED))
		{
			AddLogEntry(L"### Failed to send Scanning finished message", 0, 0, true, SECONDLEVEL);
		}

		//if scanning is not started then not need to set last scan date and time.
		if (!pThis->m_bSendScanMessage)
		{
			pThis->SetLastScanDatetime();
		}
		//pThis->ClearMemoryMapObjects();
		//In Quick Scan for some cases Add Entry for "No threat(s) found" in ReportDB not saved.
		//Name : Niranjan Deshak. Date - 10/2/2015.
		//Called AddEntriesInReportsDB("No threat(s) found") Only in case of QUICKSCAN and No Threats Found.
		//Issue: Start custom scan after it gets complete entry is not present in reports.
		//Resolved By: Nitin Kolapkar Date: 17th August 2015
		if ((pThis->m_iThreatsFoundCount == 0) && /*(pThis->m_eScanType == QUICKSCAN) &&*/ (!pThis->m_bManualStop))
		{
			pThis->AddEntriesInReportsDB(pThis->m_eScanType, L"NA", L"NA", pThis->m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND"));
			pThis->SaveInReportsDB();

		}

		if (!pThis->m_bManualStop)
		{
			//Need to set the total files scanned equal to Total file count calculate
			//This code is required because, after scanning completed UI not showing 
			//100%, even though message box getting displayed with Scanning completed.

			if (!pThis->m_bMemScanCompleted)
			{
				pThis->m_dwTotalMemScanFileCount = pThis->m_iMemScanFileCount;
			}
			else
			{
				pThis->m_dwTotalFileCount = pThis->m_iTotalFileCount;
			}
		}

		//Issue : In quick/full/custom scan scanning finished message comes twice.
		//Resolved By : Nitin K. Date: 04 - Feb - 2015
		//pThis->SetScanStatus(L"Scanning Completed");

		//Varada Ikhar, Date: 14/04/2015
		// Issue : 0000128 :In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
		CString csScanningFinished = L"";
		switch (pThis->m_eScanType)
		{
		case QUICKSCAN:
			csScanningFinished = L">>> Quick scanning finished.";
			break;
		case FULLSCAN:
			csScanningFinished = L">>> Full scanning finished.";
			break;
		case CUSTOMSCAN:
			csScanningFinished = L">>> Custom scanning finished.";
			break;
		default:
			csScanningFinished = L">>> Scanning finished.";
			break;
		}

		//Varada Ikhar, Date:11-03-2015,
		//Issue:In Quick, Full & custom scan->start scan & abort->In log file (comsrv) entry is reflecting as 'Scanning finished' instead of 'scanning aborted'
		if (!pThis->m_bManualStop)
		{
			//Varada Ikhar, Date: 14/04/2015
			// Issue : 0000128 :In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
			AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
			CString csStatus;
			csStatus.Format(L">>> Total File Scanned: %d, Threats Found: %d ", pThis->m_dwTotalFileCount, pThis->m_iThreatsFoundCount);
			AddLogEntry(L"---------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
			AddLogEntry(csStatus, 0, 0, true, SECONDLEVEL);
			AddLogEntry(L"---------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
		}
	}
	OutputDebugString(L">>> OutputThread Finished");
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CScanDlg::Output thread", 0, 0, true, SECONDLEVEL);
	}
	
	return 1;
}

UINT QuarantineThread(LPVOID lpParam)
{
	CClamScanner *pThis = (CClamScanner *) lpParam;
	//pThis->m_btnClean.EnableWindow(FALSE);
	//pThis->QuaratineEntries();
	//pThis->m_btnClean.EnableWindow(TRUE);
	return 1;
}

void CClamScanner::EnumFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(pstr);

		if (strWildcard.IsEmpty())
			return;

		if (!PathFileExists(strWildcard))
		{
			return;
		}

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");

		// start working for files
		//Custom Scan for Files/Folders changes. Enumerating the files as well for custom scan
		//Added By Nitin K. Date : 18th March 2015
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
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

void CClamScanner::WriteStdOut(LPTSTR msg, BOOL freshclam)
{
	try
	{
		::WaitForSingleObject(m_LogMutex, INFINITE);

		CString str(msg);
		CString resToken;
		int curPos = 0;

		resToken= str.Tokenize(L"\r\n",curPos);
		while (resToken != _T(""))
		{
			if(freshclam)
			{
				if(m_bFailedToLoadSignature)
				{
					AddLogEntry(L"%s",L"### Failed to Load Wardwiz Signature DataBase",L"",true,SECONDLEVEL);
					break;
				}
				else
				{
					ProcessCallBackEntries(resToken);
				}
				//send message to UI for to set text
			}
		   resToken = str.Tokenize(L"\r\n", curPos);
		};   

		ReleaseMutex(m_LogMutex);
		/* If DB is not valid issue Neha Gharge 1/1/2014*/
		if(m_bFailedToLoadSignature)
		{
			StopScan();
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in Commserv::WriteStdOut", 0, 0, true, SECONDLEVEL);
	}
}

void CClamScanner::ProcessCallBackEntries(CString csStatus)
{
	if(csStatus.GetLength() == 0)
		return;

	if (!isScanning) { return; }

	try
	{
		bool bSetStatus = false;
		bool bVirusFound = false;
		
		CString csVirusName(L"");
		CString csVirusPath(L"");
		DWORD dwISpywareID = 0;
		CString csCurrentFile(L"");
		CString csFilePath(L"");

		//int iFound = std::wstring((LPCTSTR)csStatus).rfind(L"FOUND", 1);
		if(csStatus.Right(5) == L"FOUND")
		{			
			if(GetVirusNameAndPath(csStatus, csVirusName, csVirusPath))
			{
				if(csVirusName.GetLength() > 0 && csVirusPath.GetLength() > 0)
				{
					//Clam AV detecting the files with .clamtmp extention, need to skip.
					if(csStatus.Right(8) != L".clamtmp")
					{
						if(PathFileExists(csVirusPath))
						{
							csFilePath = csVirusPath;
							bVirusFound = true;
						}
					}
				}
			}
			bSetStatus = true;
		}
		else
		{
			csCurrentFile = GetFileNameOnly(csStatus);
			if(csCurrentFile.GetLength() > 0)
			{
				if(g_csPreviousFile != csCurrentFile)
				{
					m_dwTotalFileCount++;
					csFilePath = csCurrentFile;
				}
				g_csPreviousFile = csCurrentFile;
				bSetStatus = true;
			}
			else
			{
				if(csStatus.Find(L"Infected files:") != -1)
				{
					if(m_dwTotalScanPathCount == m_csaAllScanPaths.GetCount())
					{
						
						//Name: Varada Ikhar, Date:06/01/2015, Issue No:4
						//Description: In quick/full/custom scan, incomplete message before "scanning completed" pop-up appears.
						//SetScanStatus(L"Scanning Completed"); 

						//if no threats found add entries in report

						//In Quick Scan for some cases Add Entry for "No threat(s) found" in ReportDB not saved.
						//Name : Niranjan Deshak. Date - 10/2/2015.
						//Disabled AddEntriesInReportsDB("No threat(s) found") in case of QUICKSCAN.
						/*if((m_iThreatsFoundCount == 0) && (m_eScanType != QUICKSCAN))
						{
							AddEntriesInReportsDB(m_eScanType,L"NA",L"NA",L"No threat(s) found");
						}*/
						//Issue : In Quick scan, If we abort the scan then, Aborted message not coming immediately, 
						//and other scan buttons in Tab not getting enabled
						//Resolved By : Nitin K. Date: 04 - Feb - 2015
						//resolution : flag commented because when in case of clam scanner was finished with quick scan
						//then our scanner scans the processes and modules that time stop was not functioning 
						//isScanning = FALSE;				
					}
					else
					{
						m_dwTotalScanPathCount++;
						OutputDebugString(L">>> Full Scanning is not completed still...");
					}
					SaveInReportsDB();
				}
			}
		}
		   
		if(csFilePath.Trim().GetLength() > 0)
		{
			if(g_csPreviousFilePath != csFilePath)
			{
				//Here PathFileExists, returns true even if file path is not valid.
				//This function used as replacement of PathFileExists.
				if ((_waccess(csFilePath, 0)) != -1)
				{
					//Ram: Issue Resolved, 0001134
					//Suspend here clam scanner, as it memory scanning is in progress.
					if (!m_bMemScanCompleted && !m_bIsClamSuspended)
					{
						m_bIsClamLoaded = true;
						//Suspend here the clam scanner
						SuspendClamScan();

						AddLogEntry(L">>> Wait for WardwizSCANNER to finish, Timeout: 3 minute", 0, 0, true, FIRSTLEVEL);

						//Wait here till clam scanner not resumed.
						//In General clam scanner not take more than 2 minutes to load database.
						//to kept of 2 minute buffer time to load.
						//if it get loaded and memory scan get finished, will come out immediately.
						DWORD dwTimeOut = (4 * 60 * 1000); //4 Minute ( 240000 milisconds )
						WaitForSingleObject(m_hEventMemScanFinished, dwTimeOut);
						
						AddLogEntry(L">>> After WaitForSingleObject WardwizSCANNER", 0, 0, true, FIRSTLEVEL);

						//Issue No 1150, 1161 Issue with custom scan. When we start custom scan and after some file get scanned and we go to any other feature and again come back to custom scan the pause button is not visible and any other features are not working.
						if (m_bManualStop == true)
							return;
					}
					
					/* If DB is not valid issue Neha Gharge 1/1/2014*/
					DWORD dwISpyID = 0;
					DWORD dwSignatureFailedToLoad = 0;

					TCHAR szVirusName[MAX_PATH] = {0};
					AddLogEntry(L">>> Scanning file: %s", csFilePath, 0, true, ZEROLEVEL);

					//Added by Vilas on 26 Mar 2015
					//Checking file entry present in reboot repair ini (WWRepair.ini)
					if ((!CheckFileIsInRepairRebootIni(csFilePath)) &&
						(!CheckFileIsInRecoverIni(csFilePath))			//CheckFileIsInRecoverIni Added by Vilas on 27 Mar 2015
						)
					{
						if (m_objISpyScanner.ScanFile(csFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, m_bRescan))
						{
							if (dwISpyID >= 0)
							{
								csVirusName = szVirusName;
								dwISpywareID = dwISpyID;
								csStatus = csFilePath + L": " + csVirusName + L" FOUND";
								bVirusFound = true;
							}
						}
					}
					else
					{
						//Added by Vilas on 26 Mar 2015
						//file rescan avoided because this file entry is present in reboot repair
						bVirusFound = false;
					}
					
					//Check here if it is Quick scan, as file is scanned either by clam (or) by
					//WardWiz Scanner, so no need to scan it again.
					if (m_eScanType == QUICKSCAN)
					{
						RemoveModuleFromList(csFilePath.GetBuffer());
					}

					if(dwSignatureFailedToLoad != 0)
					{
						m_bFailedToLoadSignature = true;
					}
				}
			}
			g_csPreviousFilePath = csFilePath;
		}

		if (bSetStatus)
		{
			if (m_bSendScanMessage)
			{
				if (!SendMessage2UI(SCAN_STARTED))
				{
					AddLogEntry(L"### Failed to send SCAN_STARTED message to UI", 0, 0, true, SECONDLEVEL);
				}
				m_bSendScanMessage = false;
			}

			SetScanStatus(csStatus.MakeUpper());
		}

		//virus found 
		if(bVirusFound)
		{
			bSetStatus = true;
			if (HandleVirusFoundEntries(csVirusName, csFilePath, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"), dwISpywareID))
			{
				m_iThreatsFoundCount++;
				AddEntriesInReportsDB(m_eScanType, csVirusName, csFilePath, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"));
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in void CScanDlg::ReInitialize", 0, 0, true, SECONDLEVEL);
	}
}

BOOL CClamScanner::LaunchClamAV(LPTSTR pszCmdLine, HANDLE hStdOut, HANDLE hStdErr)
{
	try
	{
		STARTUPINFO si;
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.hStdOutput = hStdOut;
		si.hStdError = hStdErr;
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

		if (!CreateProcess(NULL, pszCmdLine,
			NULL, NULL,
			TRUE,
			CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
			NULL, NULL,
			&si,
			&pi))
			return FALSE;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::LaunchClamAV", 0, 0, true, SECONDLEVEL);
		return FALSE;
	}
	return TRUE;
}

void CClamScanner::RedirectStdOutput(BOOL freshclam)
{
	try
	{

		CHAR chBuf[BUFSIZE];
		DWORD dwRead;
		DWORD dwAvail = 0;
		if (!PeekNamedPipe(hChildStdoutRdDup, NULL, 0, NULL, &dwAvail, NULL) || !dwAvail)
			return;
		if (!ReadFile(hChildStdoutRdDup, chBuf, min(BUFSIZE - 1, dwAvail), &dwRead, NULL) || !dwRead)
			return;
		chBuf[dwRead] = 0;

		const size_t cSize = strlen(chBuf)+1;
		TCHAR* szBuf = new TCHAR[cSize];
		if(szBuf == NULL)
		{
			return;//allocation failed here
		}

		size_t buffer_size = 0;
		mbstowcs_s(&buffer_size, szBuf, cSize, chBuf, cSize);

		if(szBuf != NULL)
		{
			WriteStdOut(szBuf, freshclam);
		}

		if(szBuf != NULL)
		{
			delete []szBuf;
			szBuf = NULL;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::RedirectStdOutput", 0, 0, true, SECONDLEVEL);
	}
}

bool CClamScanner::StartScanning(SCANTYPE eScanType, CStringArray &csaAllScanPaths)
{
	try
	{
		m_ScanCount = false ;
		m_iThreatsFoundCount = 0;
		m_dwTotalFileCount = 0;
		m_dwTotalMemScanFileCount = 0;
		m_bManualStop = false;
		m_bMemScanCompleted = false;
		m_bIsClamLoaded = false;
		m_bIsClamSuspended = false;
		m_bFailedToLoadSignature = false;
		m_dwFailedDeleteFileCount = 0;
		m_bFileFailedToDelete = false;

		if (isScanning) return false;
		//m_bRedFlag = false;

		UINT uRetVal = 0;

		m_csaAllScanPaths.RemoveAll();
		m_csaAllScanPaths.Append(csaAllScanPaths);
		m_vFileDuplicates.clear();
		m_csaModuleList.RemoveAll();
		ClearMemoryMapObjects();

		//Ramkrushna Shelke Date: 04/02/2015, Version: 1.8.3.8
		//Issue: .In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
		LoadPSAPILibrary();

		m_eScanType = eScanType;

		if(m_csaAllScanPaths.GetCount() == 0)
		{
			AddEntriesInReportsDB(m_eScanType, L"NA", L"NA", m_objwardwizLangManager.GetString(L"IDS_MSG_NO_FILES_FOR_SCAN"));
		}

		//ReInitializeBeforeStartScan();

		SetScanStatus(m_objwardwizLangManager.GetString(L"IDS_STATUS_SCANNER_LAUNCH"));
		SetScanStatus(m_objwardwizLangManager.GetString(L"IDS_STATUS_MEMSCANNER_LAUNCH"));
		AddLogEntry(L">>> Scanning Memory...", 0, 0, true, ZEROLEVEL);

		//DWORD m_dwCalcFilesThreadId = 0;
		//HANDLE m_hCalcTotalFilesThread = ::CreateThread(NULL, 0, CalculateTotalFilesInFolder, (LPVOID) this, 0, &m_dwCalcFilesThreadId);
		//CloseHandle(m_hCalcTotalFilesThread);
		
		//Ram: LoadSignature database here if not loaded.
		DWORD dwSigCount = 0x00;
		if (LoadSignatureDatabase(dwSigCount) != 0x00)
		{
			AddLogEntry(L"### Failed to Load SignatureDatabase", 0, 0, true, SECONDLEVEL);
		}

		//Load Reports.DB
		LoadReportsDBFile();
		LoadRecoversDBFile();

	
		m_iMemScanFileCount = 0;
		m_iTotalFileCount = 0;
		m_iMemScanFileCount = 0;
		if( m_csaAllScanPaths.GetAt(0).CompareNoCase( L"QUICKSCAN") == 0 )
		{
			m_eScanType = QUICKSCAN;
			//GetModuleCount();
		}
		else
		{
			m_hThread_ScanCount = ::CreateThread(NULL, 0, GetScanFilesCount, (LPVOID) this, 0, NULL);
			Sleep( 1000 ) ;
		}

		isScanning = TRUE;


		//Start here memory scan thread.
		DWORD m_dwMemScanTId = 0;
		m_hMemScanThread = ::CreateThread(NULL, 0, MemoryScanThread, (LPVOID)this, 0, &m_dwMemScanTId);

		DWORD m_dwThreadId = 0;
		m_hClamAVThread = ::CreateThread(NULL, 0, PipeToClamAV, (LPVOID) this, 0, &m_dwThreadId);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartScanning", 0, 0, true, SECONDLEVEL);
	}
	return true;	
}

bool CClamScanner::SetScanStatus(CString csStatus)
{
	CISpyCriticalSection  objcriticalSection;
	objcriticalSection.Lock();

	ITIN_MEMMAP_DATA iTinMemMap = {0};
	iTinMemMap.iMessageInfo = SETSCANSTATUS;

	if (!m_bMemScanCompleted)
	{
		iTinMemMap.dwFirstValue = m_dwTotalMemScanFileCount;
	}
	else
	{
		iTinMemMap.dwFirstValue = m_dwTotalFileCount;
	}

	iTinMemMap.dwSecondValue = m_iThreatsFoundCount;
	_tcscpy_s(iTinMemMap.szFirstParam, sizeof(iTinMemMap.szFirstParam), csStatus);
	m_objServ.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));

	//Ram: No need it slows down the scanner
	//Sleep(10);

	objcriticalSection.Unlock();

	return true;
}

/***************************************************************************
  Function Name  : GetModuleCount
  Description    : Function to calculate running process count with modules 
				   loaded by process.
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 24th Jan 2015
****************************************************************************/
void CClamScanner::GetModuleCount( )
{
	DWORD	dwPID[0x100] = {0} ;
	DWORD	dwBytesRet = 0x00, iProcIndex = 0x00 ;
	
	EnumProcesses(dwPID, 0x400, &dwBytesRet ) ;
	dwBytesRet = dwBytesRet/sizeof(DWORD) ;

	//First Set total number of files count.
	m_iTotalFileCount = 0;
	m_dwTotalMemScanFileCount = 0;
	m_iMemScanFileCount = 0;
	for( iProcIndex = 0; iProcIndex < dwBytesRet; iProcIndex++ )
	{
		HMODULE		hMods[1024] = {0} ;
		HANDLE		hProcess = NULL ;

		DWORD		dwModules = 0x00 ;

		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
								FALSE, dwPID[iProcIndex] ) ;
		if( !hProcess )
			continue ;

		//Ramkrushna Shelke Date: 04/02/2015, Version: 1.8.3.8
		//Issue: .In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
		if( EnumProcessModulesWWizEx != NULL)
		{
			if( !EnumProcessModulesWWizEx(hProcess, hMods, sizeof(hMods), &dwModules, LIST_MODULES_ALL ) )
			{
				DWORD error = GetLastError();
				CloseHandle( hProcess ) ;
				continue ;
			}
		}
		else
		{
			if( !EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules ) )
			{
				DWORD error = GetLastError();
				CloseHandle( hProcess ) ;
				continue ;
			}
		}
		
		//EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules) ;
		//EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &dwModules, LIST_MODULES_ALL) ;
		
		for ( DWORD iModIndex = 0; iModIndex < (dwModules / sizeof(HMODULE)); iModIndex++ )
		{
			TCHAR szModulePath[MAX_PATH * 2] = {0};
			GetModuleFileNameEx( hProcess, hMods[iModIndex], szModulePath, MAX_PATH * 2);

			CString csProcessPath(L"");
			CString csToken(L"");
			CString csTokenSytemRoot(L"");

			TCHAR szSystemDirectory[MAX_PATH] = {0};
			bool bSystemDirectory = false;
			bool bReplaceWindowPath = false;
			int iPos = 0;
			int SlashPos = 0;

			csProcessPath = szModulePath;
			SlashPos = csProcessPath.Find(_T("\\"), iPos);
			if(SlashPos == 0)
			{
				csToken = csProcessPath.Right(csProcessPath.GetLength() - (SlashPos+1));
				bSystemDirectory = true;
			}

			GetWindowsDirectory(szSystemDirectory,MAX_PATH);
			if(bSystemDirectory == true)
			{
				SlashPos = 0;
				iPos = 0;
				SlashPos = csToken.Find(_T("\\"), iPos);
				csTokenSytemRoot = csToken;
				csTokenSytemRoot.Truncate(SlashPos);
				if(csTokenSytemRoot == L"SystemRoot")
				{
					bReplaceWindowPath = true;
				}
				else if(csTokenSytemRoot == L"??")
				{
					csToken.Replace(L"??\\", L"");
					csProcessPath = csToken;
				}
				bSystemDirectory = false;
			}
			if(bReplaceWindowPath == true)
			{
				csToken.Replace(csTokenSytemRoot,szSystemDirectory);
				csProcessPath = csToken;
				bReplaceWindowPath = false;
			}

			_tcscpy(szModulePath, csProcessPath); 
			if(!IsDuplicateModule( szModulePath ))
			{
				m_iTotalFileCount++;
				m_iMemScanFileCount++;
				_tcsupr(szModulePath);
				m_csaModuleList.AddHead(szModulePath);
			}
		}													 
		CloseHandle( hProcess ) ;
	}

	m_iTotalFileCount += 6;		//Need to increase this count because, when we launch WRDWIZSCANNER, it loads 
	m_iMemScanFileCount += 6;	//LIBCLAMAV.DLL with DLL file present in "MICROSOFT.VC80.CRT" folder
								//Total loded file count is 6 so need to increase count by 6.
	if( m_iTotalFileCount )
	{
		m_ScanCount = true ;
	}

	//Send here total file count for Memory scan
	SendTotalFileCount(m_iMemScanFileCount, true);

	Sleep(1000);

	if(m_eScanType == QUICKSCAN)
	{
		//Send here total file count for without Memory scan ( Event though we 
		SendTotalFileCount(m_iTotalFileCount);
	}
}

bool CClamScanner::StopScan()
{
	if (!isScanning) {return true;}

	m_bManualStop = true;

	//Varada Ikhar, Date: 14/04/2015
	// Issue : 0000128 :In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
	CString csScanningAborted = L"";
	switch (m_eScanType)
	{
	case QUICKSCAN: 
		csScanningAborted = L">>> Quick scanning aborted.";
		break;
	case FULLSCAN:
		csScanningAborted = L">>> Full scanning aborted.";
		break;
	case CUSTOMSCAN: 
		csScanningAborted = L">>> Custom scanning aborted.";
		break;
	default: 
		csScanningAborted = L">>> Scanning aborted.";
		break;
	}

	//Varada Ikhar, Date:11-03-2015,
	//Issue:In Quick, Full & custom scan->start scan & abort->In log file (comsrv) entry is reflecting as 'Scanning finished' instead of 'scanning aborted'
	AddLogEntry(csScanningAborted, 0, 0, true, SECONDLEVEL);
	CString csStatus;
	csStatus.Format(L">>> Total File Scanned: %d, Threats Found: %d ", m_dwTotalFileCount , m_iThreatsFoundCount);
	AddLogEntry(L"---------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	AddLogEntry(csStatus, 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"---------------------------------------------------------------", 0, 0, true, SECONDLEVEL);

	//Issue No 1150, 1161 Issue with custom scan. When we start custom scan and after some file get scanned and we go to any other feature and again come back to custom scan the pause button is not visible and any other features are not working.
	if (m_hEventMemScanFinished != NULL)
	{
		SetEvent(m_hEventMemScanFinished);
	}
	//Added By Nitin K
	//Date: 16 Jan 2015
	//In custom scan, full scan, quick scan, while Pressing the "pause" button the progress bar continues some times.
	//It should be immediately.
	if(m_hThread_Output != NULL)
	{
		ResumeThread(m_hThread_Output);
	}
	if(m_hThread_ScanCount != NULL)
	{
		SuspendThread(m_hThread_ScanCount);
		TerminateThread(m_hThread_ScanCount, 0);
		m_hThread_ScanCount = NULL;
	}

	isScanning = FALSE;

	if (pi.hProcess != INVALID_HANDLE_VALUE)
	{
		TerminateProcess(pi.hProcess, -1);
	}
	
	Sleep(500);

	if(m_hClamAVThread != NULL)
	{
		SuspendThread(m_hClamAVThread);
		TerminateThread(m_hClamAVThread, 0);
		m_hClamAVThread = NULL;
	}

	//Terminate here memory scan thread.
	if (m_hMemScanThread != NULL)
	{
		SuspendThread(m_hMemScanThread);
		TerminateThread(m_hMemScanThread, 0);
		m_hMemScanThread = NULL;
	}

	//Set here the last scan status if memory scan is running.
	if (m_bISOnlyMemScan)
	{
		SetLastScanDatetime();
	}

	//scanning aborted
	AddEntriesInReportsDB(m_eScanType, L"NA", L"NA", m_objwardwizLangManager.GetString(L" IDS_STATUS_SCAN_ABORTED"));
	SaveInReportsDB();
	return true;
}

bool CClamScanner::PauseScan()
{
	AddLogEntry(L">>> Scanning Paused", 0, 0, true, FIRSTLEVEL);
	//Added By Nitin K
	//Date: 16 Jan 2015
	//In custom scan, full scan, quick scan, while Pressing the "pause" button the progress bar continues some times.
	//It should be immediately.
	if (m_hThread_Output != INVALID_HANDLE_VALUE)
	{
		SuspendThread(m_hThread_Output);
	}

	//Suspend here memory scan thread.
	if (m_hMemScanThread != INVALID_HANDLE_VALUE)
	{
		SuspendThread(m_hMemScanThread);
	}

	if (!isScanning) return false;
	if (pi.hProcess != INVALID_HANDLE_VALUE)
	{
		SuspendThread(pi.hThread);
	}
	
	return true;
}

bool CClamScanner::ResumeScan()
{
	AddLogEntry(L">>> Scanning Resumed", 0, 0, true, FIRSTLEVEL);
	//Added By Nitin K
	//Date: 16 Jan 2015
	//In custom scan, full scan, quick scan, while Pressing the "pause" button the progress bar continues some times.
	//It should be immediately.
	if (m_hThread_Output != INVALID_HANDLE_VALUE)
	{
		ResumeThread(m_hThread_Output);
	}

	if (!isScanning) return false;

	//Resume memory scan thread.
	if (m_hMemScanThread != INVALID_HANDLE_VALUE)
	{
		ResumeThread(m_hMemScanThread);
	}

	if (pi.hProcess != INVALID_HANDLE_VALUE)
	{
		ResumeThread(pi.hThread);
	}
	
	return true;
}

/***************************************************************************
Function Name  : QuarantineEntry
Description    : IF WARDWIZID = 0, files get quarantined, each entry get deleted.
				if fails its entry save into ini. So that at boot up time delete action 
				to be taken.
Author Name    : Ramkrushna Shelke 
S.R. No        :
Date           : 24th Jan 2015
Modified date  : 8-4-2015 Neha Gharge Adding entry into INI.
****************************************************************************/
bool CClamScanner::QuarantineEntry(CString csQurFilePaths, CString csVirusName)
{
	AddLogEntry(L">>> Quarantine Entry [%s]: %s", csVirusName, csQurFilePaths, true, SECONDLEVEL);
	bool bReturn = false;
	try
	{
		m_bFileFailedToDelete = false;
		SetFileAttributes(csQurFilePaths, FILE_ATTRIBUTE_NORMAL);
		bReturn = ::DeleteFile(csQurFilePaths) ? true : false;
		if(!bReturn)
		{
			CEnumProcess objEnumProcess;
			if(objEnumProcess.IsProcessRunning(csQurFilePaths, true))
			{
				AddLogEntry(L">>> Killing running Process: %s", csQurFilePaths, 0, true, SECONDLEVEL);			
				::Sleep(1000);
				SetFileAttributes(csQurFilePaths, FILE_ATTRIBUTE_NORMAL);
				bReturn = ::DeleteFile(csQurFilePaths) ? true : false;
			}
		}
		if (!bReturn)
		{
			AddLogEntry(L"Failed to Quarantine file: %s", csQurFilePaths, 0, true, SECONDLEVEL);
			if (!AddDeleteFailEntryToINI(csQurFilePaths, csVirusName))
			{
				AddLogEntry(L"### Failed into add entry into ini", 0, 0, true, FIRSTLEVEL);
			}
			m_bFileFailedToDelete = true;
			MoveFileEx(csQurFilePaths, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			bReturn = false; //quarantine is failed but we keep the record in the ini.
		}

		//}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::QuarantineFile", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

bool CClamScanner::GetVirusNameAndPath(CString csStatus, CString &csVirusName, CString &csVirusPath)
{
	bool bReturn  = false;
	try
	{
		int iLength = csStatus.GetLength();

		if(iLength == 0)
			return bReturn;

		//Handle here the false positive returned by clam scanner.
		if(csStatus.Mid(iLength - 20, 14) == L"FALSE POSITIVE")
		{
			return bReturn;
		}

		int iPivot = csStatus.ReverseFind(':');

		csVirusName = csStatus.Right((csStatus.GetLength() - iPivot) - 2);
		csVirusName = csVirusName.Left(csVirusName.GetLength() - 6);
		csVirusPath = csStatus.Left(iPivot);
		bReturn = true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetVirusNameAndPath", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

CString CClamScanner::GetFileNameOnly(CString csInputPath)
{
	CString csReturn = L"";
	try
	{
		if(csInputPath.GetLength() == 0)
			return csReturn;

		if(!IsFullString(csInputPath))
		{
			return csReturn;
		}

		//if file path is complete file path, then no need to extract exact file name
		//from this.
		if (PathFileExists(csInputPath))
		{
			csReturn = csInputPath;
			return csReturn;
		}

		int iTempPos, iFirstPos;
		iFirstPos = iTempPos = static_cast<int>(std::wstring((LPCTSTR)csInputPath).rfind(L"]"));
		if(iTempPos > 0)
		{
			if( (csInputPath[ --iTempPos] == '[')  ||
				(csInputPath[ --iTempPos] == '[')  ||
				(csInputPath[ --iTempPos] == '[')  ||
				(csInputPath[ --iTempPos] == '[')  ||
				(csInputPath[ --iTempPos] == '[') )
			{
				iFirstPos = iTempPos;
				CString csFilePath = csInputPath.Left(iFirstPos - 2);
				if(PathFileExists(csFilePath))
				{
					csReturn = csFilePath;
				}
			}
		}
		else 
		{
			CString csTemp = csInputPath.Right(6);
			csTemp.Trim();
			if(csTemp == L"OK")
			{
				csReturn = csInputPath.Left(csInputPath.GetLength() - 8);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CClamScanner::GetFileNameOnly, InputPath: %s", csInputPath, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

bool CClamScanner::IsFullString(CString csInputPath)
{
	try
	{
		if(( (csInputPath.Find(L"[") != -1) && (csInputPath.Find(L"]") != -1)) ||
			(csInputPath.Find(L"OK") > 0))
		{
			return true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in void CScanDlg::IsFullString", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

bool CClamScanner::SendMessage2UI(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(UI_SERVER, true);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CClamScanner::SendMessage2UI", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CClamScanner::HandleVirusFoundEntries(CString strVirusName, CString strScanFileName, CString csAction, DWORD dwSpyID)
{
	CISpyCriticalSection  objcriticalSection;
	objcriticalSection.Lock();

	if(!PathFileExists(strScanFileName))
	{
		AddLogEntry(L"### Path is not exist of %s",strScanFileName, 0, true, SECONDLEVEL);
		return false;
	}

	//Check here to avoid duplicate entry 
	if(CheckForDuplicates(strScanFileName))
	{
		return false;
	}
	
	AddLogEntry(L">>> Virus Found: %s, File Path: %s", strVirusName, strScanFileName, true, SECONDLEVEL);

	m_vFileDuplicates.push_back(strScanFileName);

	OutputDebugString(strScanFileName);

	//Commented this code by Nitin K.
	//Reason: Displaying Virus found entries on UI through Pipe Communication because 
	//some of the found entries were missing and those were not getting displayed on UI 
	//because previously we were displaying it through Shared memory.
	//Date : 15-Jan-2015
	/*ITIN_MEMMAP_DATA iTinMemMap = {0};
	iTinMemMap.iMessageInfo = SHOWVIRUSFOUND;
	_tcscpy(iTinMemMap.szFirstParam,strVirusName);
	_tcscpy(iTinMemMap.szSecondParam,strScanFileName);
	iTinMemMap.dwFirstValue = dwSpyID;

	m_objServViruEntry.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));*/

	//Added by Nitin K.
	//Issue: Displaying Virus found entries on UI through Pipe Communication
	//Date : 15-Jan-2015
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	szPipeData.iMessageInfo = SHOW_VIRUSENTRY;
	_tcscpy(szPipeData.szFirstParam,strVirusName);
	_tcscpy(szPipeData.szSecondParam,strScanFileName);
	szPipeData.dwValue = dwSpyID;
	if(!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		//Varada Ikhar, Date : 30/03/2015,
		//Issue : In Custom scan,if virus files are detected and without cleaning if UI is close and again if same virus files are scanned using custom scan then only one file is getting detected.
		//Description: Reason behind the issue was, Pipe is being closed (getlasterror - 232) but pipe-handle value was not invalid. Thus another call is given to try to send data.
		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CClamScanner::HandleVirusFoundEntries", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}

	Sleep(10);

	objcriticalSection.Unlock();

	return true;
}

void CClamScanner::SendTotalFileCount(DWORD dwCount, bool bIsMemOnly)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = SET_TOTALFILECOUNT;
	szPipeData.dwValue = dwCount;

	//This flag tells UI, that total file count is for Memory scan.
	if (bIsMemOnly)
	{
		szPipeData.dwSecondValue = TRUE;
	}

	//Ram:
	//Retry to send Total file count from Service to UI.
	CISpyCommunicator objCom(UI_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CClamScanner::SendTotalFileCount", 0, 0, true, SECONDLEVEL);
	}
}

//Signature changed to handle failure cases
//Modified by Vilas on 07 April 2015
//bool CClamScanner::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, DWORD dwISpyID)
DWORD CClamScanner::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThirdParam, DWORD dwISpyID, CString &csBackupFilePath, CString &csQurStatus)
{
	//bool bReturn = false;

	DWORD	dwRet = 0x00;

	try
	{

		TCHAR szAction[MAX_PATH] = { 0 };

		if ((!szThreatPath) || (!szThreatName) || (!szThirdParam))
		{
			AddLogEntry(L"### CClamScanner::HandleVirusEntry file name not available", 0, 0, true, SECONDLEVEL);

			dwRet = 0x01;
			return dwRet;
		}

		if (!PathFileExists(szThreatPath))
		{
			// Should not give failed message on UI at any case Neha Gharge 19/3/2015
			AddLogEntry(L"### CClamScanner::HandleVirusEntry No file available %s", szThreatPath, 0, true, SECONDLEVEL);
			//return bReturn;

			dwRet = 0x02;
			return dwRet;
		}
		//For files having read-only attribute
		::SetFileAttributes(szThreatPath, FILE_ATTRIBUTE_NORMAL);

		CString csQuaratineFolderPath = GetQuarantineFolderPath();
		if (!PathFileExists(csQuaratineFolderPath))
		{
			if (!CreateDirectory(csQuaratineFolderPath, NULL))
			{
				AddLogEntry(L"### CWardwizSCANNERBase::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);

				dwRet = 0x03;
			}
		}

		bool bISbackupSuccess = true;
		//DWORD value flag to show entry in Recover window (or) not.
		DWORD dwShowEntryStatus = 0;
		if (dwISpyID > 0)
		{
			//Terminate the process if its running
			CEnumProcess objEnumProcess;
			objEnumProcess.IsProcessRunning(szThreatPath, true);

			/*Added by Prajakta: to handle multiple instances of scanning & repair*/

			//Re-scan file before repairing
			DWORD dwRepairID = 0;
			DWORD dwFailedToLoadSignature = 0;
			m_bRescan = true;
			TCHAR szVirusName[MAX_PATH] = { 0 };
			_tcscpy(szVirusName, szThreatName);
			if (m_objISpyScanner.ScanFile(szThreatPath, szVirusName, dwISpyID, dwFailedToLoadSignature, m_bRescan))
			{
				//While taking a back up of any file. we have to check disk where we take a back up 
				//is having space or not.
				if (CheckForDiscSpaceAvail(csQuaratineFolderPath,szThreatPath) == 0x01)
				{
					dwRet = 0x09;
					return dwRet;
				}

				if (!BackUpBeforeQuarantineOrRepair(szThreatPath))
				{
					AddLogEntry(L"#### Failed to take backup of %s", szThreatPath, 0, true, SECONDLEVEL);
					bISbackupSuccess = false;
					dwRet = 0x07;
				}
				
				bISbackupSuccess = true;

				if (!m_objISpyScanner.RepairFile(szThreatPath, dwISpyID))
				{
					// Should not give failed message on UI at any case Neha Gharge 19/3/2015
					//If repair failed 
					//Added by Vilas on 23 Mar 2015 for reboot repair
					AddRepairEntryAfterReboot(szThreatPath, szVirusName, m_csDuplicateName, dwISpyID);
					dwShowEntryStatus = 1;

					AddLogEntry(L"### Repair on reboot file: %s", szThreatPath, 0, true, SECONDLEVEL);
					//return bReturn;

					dwRet = 0x04;
				}
				_tcscpy(szAction, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
				//bReturn = true;
			}
			else
			{
				//Need to returned from here.
				m_bRescan = false;
				dwRet = 0x08;
				return dwRet;
			}
		}
		else
		{
			//While taking a back up of any file. we have to check disk where we take a back up 
			//is having space or not.
			if (CheckForDiscSpaceAvail(csQuaratineFolderPath, szThreatPath) == 0x01)
			{
				dwRet = 0x09;
				return dwRet;
			}

			if (!BackUpBeforeQuarantineOrRepair(szThreatPath))
			{
				AddLogEntry(L"#### Failed to take backup of %s", szThreatPath, 0, true, SECONDLEVEL);
				bISbackupSuccess = false;
				dwRet = 0x07;
			}

			bISbackupSuccess = true;

			//quarantine file
			if (!QuarantineEntry(szThreatPath, szThreatName))
			{
				AddLogEntry(L"### Failed to Quarantine file: %s", szThreatPath, 0, true, SECONDLEVEL);
				dwShowEntryStatus = 1;
				//return bReturn;

				dwRet = 0x05;
			}
			_tcscpy(szAction, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
		}

		//Neha Gharge right click scan 18-3-2015
		// Third parameter need to be recieve which scan currently going on.
		//Issue 1190 report entry for this scans after USB scan is wrong
		//Neha Gharge right click scan 18-3-2015
		// Third parameter need to be receive which scan currently going on.
		//Issue 1190 report entry for this scans after USB scan is wrong
		bool bDoRecover = true;
		if (_tcscmp(szThirdParam, L"USB Scan") == 0)
		{
			bDoRecover = false;
			csQurStatus.Format(L"%s", szAction);
			//AddEntriesInReportsDB(USBSCAN, szThreatName, szThreatPath, szAction);
		}
		else if (_tcscmp(szThirdParam, L"RightClick Scan") == 0)
		{
			bDoRecover = false;
			csQurStatus.Format(L"%s", szAction);
			//AddEntriesInReportsDB(CUSTOMSCAN, szThreatName, szThreatPath, szAction);
		}
		else if (_tcscmp(szThirdParam, L"Custom Scan") == 0)
		{
			AddEntriesInReportsDB(CUSTOMSCAN, szThreatName, szThreatPath, szAction);
		}
		else if (_tcscmp(szThirdParam, L"Full Scan") == 0)
		{
			AddEntriesInReportsDB(FULLSCAN, szThreatName, szThreatPath, szAction);
		}
		else if (_tcscmp(szThirdParam, L"Quick Scan") == 0)
		{
			AddEntriesInReportsDB(QUICKSCAN, szThreatName, szThreatPath, szAction);
		}

		if (bISbackupSuccess)
		{
			csBackupFilePath = m_csDuplicateName;
			if (bDoRecover)
			{
				if (!InsertRecoverEntry(szThreatPath, m_csDuplicateName, szThreatName, dwShowEntryStatus))
				{
					AddLogEntry(L"### Error in InsertRecoverEntry", 0, 0, true, SECONDLEVEL);
					dwRet = 0x06;
				}
			}
		}

		m_bRescan = false;
	}
	catch (...)
	{
		dwRet = 0x0100;
		AddLogEntry(L"### Exception in CClamScanner::HandleVirusEntry", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
void InsertDataToTable(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable entered", 0, 0, true, ZEROLEVEL);
	try
	{
		CWardWizSQLiteDatabase objSqlDb(g_strWWDatabaseFile);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);

		objSqlDb.Close();

		return;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CClamScanner::InsertDataToTable", 0, 0, true, SECONDLEVEL);
	}
}
void CClamScanner::AddEntriesInReportsDB(SCANTYPE eScanType, CString csThreatName, CString csFilePath, CString csAction)
{
	try
	{
		SYSTEMTIME		CurrTime = {0} ;
		GetLocalTime( &CurrTime ) ;//Ram, Issue resolved:0001218
		CTime	Time_Curr( CurrTime ) ;
		int iMonth = Time_Curr.GetMonth();
		int iDate = Time_Curr.GetDay();
		int iYear = Time_Curr.GetYear();

		CString csDate = L"";
		csDate.Format(L"%d/%d/%d",iMonth,iDate,iYear);

		CTime ctDateTime = CTime::GetCurrentTime();

		TCHAR csScanType[0x30];
		switch(eScanType)
		{
		case FULLSCAN:
			_tcscpy(csScanType, m_objwardwizLangManager.GetString(L"IDS_BUTTON_FULLSCAN"));
			break;
		case CUSTOMSCAN:
			_tcscpy(csScanType, m_objwardwizLangManager.GetString(L"IDS_BUTTON_CUSTOMSCAN"));
			break;
		case QUICKSCAN:
			_tcscpy(csScanType, m_objwardwizLangManager.GetString(L"IDS_BUTTON_QUICKSCAN"));
			break;
		case USBSCAN:
		case USBDETECT:
			_tcscpy(csScanType, m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_SCAN"));
			break;

		}

		CString csTime =  ctDateTime.Format(_T("%H:%M:%S"));

		CString csDateTime = L"";
		csDateTime.Format(_T("%s %s"),csDate,csTime);

		//CString csInsertQuery = "INSERT INTO Wardwiz_ScanDetails VALUES (null,";
		//csInsertQuery.Format(_T("%d ,%s,%s,Date('now'),Datetime('now','localtime'),%s,%s,%s );"), eScanType, csDate, csTime, csThreatName, csFilePath, csAction);
		//
		//CT2A ascii(csInsertQuery, CP_UTF8);

		////Not required to create, because on Server restart we are creating table.
		////CWardWizSQLiteDatabase objSqlDb(g_strWWDatabaseFile);
		////objSqlDb.Open();
		////objSqlDb.CreateWardwizSQLiteTables();
		////objSqlDb.Close();

		//InsertDataToTable(ascii.m_psz);

		//m_ptrISpyDBManipulation->InsertEntry(newEntry, REPORTS);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CClamScanner::AddEntriesInReportsDB", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name		: InsertRecoverEntry
*  Description			: Insert entry into recover DB
*  Function Arguments	: szThreatPath, csDuplicateName, szThreatName.
							dwShowStatus = 0;	Repair/Delete Sucess
							dwShowStatus = 1; Delete failed
							dwShowStatus = 2; Repair failed
*  Author Name			: Neha Gharge
*  Date					: 4 Dec 2014
*  SR_NO				:
**********************************************************************************************************/
bool CClamScanner::InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus)
{
	CIspyList newEntry(szThreatPath, csDuplicateName, szThreatName, L"", L"", L"", dwShowStatus);
 
	if(!m_ptrISpyDBManipulation)
	{
		return false;
	}
	m_ptrISpyDBManipulation->InsertEntry(newEntry, RECOVER);
	return true;
}

bool CClamScanner::SaveInReportsDB()
{
	OutputDebugString(L">>> In SaveInReportsDB");

	bool bReturn = false;
	__try
	{
		if(!m_ptrISpyDBManipulation)
		{
			return bReturn;
		}
		bReturn = m_ptrISpyDBManipulation->SaveEntries(0x05);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return  bReturn;

	//FILE *pFile = NULL;
	//static CString csFilePath;
	//TCHAR szModulePath[MAX_PATH] = {0};
	//if(!GetModulePath(szModulePath, MAX_PATH))
	//{
	//	MessageBox(NULL,L"Error in GetModulePath", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
	//	return false;
	//}
	//csFilePath = szModulePath;
	//csFilePath += _T("\\VBREPORTS.DB");
	//	
	//m_bSuccess=0;
	//CFile wFile(csFilePath, CFile::modeCreate | CFile::modeWrite);
 //
	//// Create a storing archive
	//CArchive arStore(&wFile,CArchive::store);
	//m_objReportsDB.Serialize(arStore);
	//
	//// Close the storing archive
	//arStore.Close();
	//wFile.Close();
 //
	//if(!m_bSuccess)
	//{
	//	return true;
	//}
	//return false;
}

void CClamScanner::LoadReportsDBFile()
{
	try
	{
		if(!m_ptrISpyDBManipulation)
		{
			return;
		}
		if(!m_ptrISpyDBManipulation->LoadEntries(0x05))
		{
			AddLogEntry(L"### Failed to load entries in CClamScanner::LoadReportsDBFile", 0, 0, true, SECONDLEVEL);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CClamScanner::LoadReportsDBFile", 0, 0, true, SECONDLEVEL);
	}
}

bool CClamScanner::SetManipulationPtr(CISpyDBManipulation *pISpyDBManpt)
{
	if(!pISpyDBManpt)
	{
		return false;
	}

	if(m_ptrISpyDBManipulation == NULL)
	{
		m_ptrISpyDBManipulation = pISpyDBManpt;
	}
	return true;
}

bool CClamScanner::BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath)
{
	DWORD dwStatus=0;
	CString csEncryptFilePath,csQuarantineFolderpath = L"";
	TCHAR szQuarantineFileName[MAX_PATH]={0};
	UINT RetUnique=0;
	m_csDuplicateName = L"";
	csQuarantineFolderpath = GetQuarantineFolderPath(); 

	if(!PathFileExists(csOriginalThreatPath))
	{
		AddLogEntry(L"### CClamScanner::BackUpBeforeQuarantineOrRepair Original file not available %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
		return false;
	}
	if(!PathFileExists(csQuarantineFolderpath))
	{
		if(!CreateDirectory(csQuarantineFolderpath, NULL))
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}

	RetUnique=GetTempFileName(csQuarantineFolderpath, L"tmp", 0, szQuarantineFileName);
	if(!RetUnique)
	{
		AddLogEntry(L"### CWardwizSCANNERBase::Error in Create temporary file name", 0, 0, true, SECONDLEVEL);
		return false;
	}
	
	m_csDuplicateName = szQuarantineFileName;
			
	if(!CopyFile(csOriginalThreatPath,m_csDuplicateName,FALSE))
	{
		int error = GetLastError();
		return false;
	}
	
	if(Encrypt_File(szQuarantineFileName,dwStatus) != 0x00)
	{
		return false;
	}
	return true;

}

DWORD CClamScanner::Encrypt_File(TCHAR *szFilePath, DWORD &dwStatus)
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00 ;
	TCHAR	szExt[16] = {0} ;
	DWORD	dwLen = 0x00 ;
	LPBYTE	lpFileData = NULL ;
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;
	//m_dwEnKey = *( (DWORD * )&szpIpsy ) ;


	__try
	{
		
		if( !PathFileExists( szFilePath ) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}


		hFile = CreateFile(	szFilePath, GENERIC_READ, 0, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in opening existing file %s",szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		dwFileSize = GetFileSize( hFile, NULL ) ;
		if( !dwFileSize )
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in GetFileSize of file %s",szFilePath, 0, true, SECONDLEVEL);
			CloseHandle( hFile ) ;
			dwRet = 0x03 ;
			goto Cleanup ;
		}
		if( lpFileData )
		{
			free(lpFileData);
			lpFileData=NULL;

		}

		//lpFileData = (LPBYTE ) malloc( dwFileSize +0x08 ) ;
		lpFileData = (LPBYTE ) malloc( dwFileSize) ;
		if( !lpFileData )
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in allocating memory", 0, 0, true, SECONDLEVEL);
			CloseHandle( hFile ) ;
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		memset(lpFileData, 0x00, dwFileSize ) ;
		SetFilePointer( hFile, 0x00, NULL, FILE_BEGIN ) ;
		ReadFile( hFile, lpFileData, dwFileSize, &dwBytesRead, NULL ) ;
		if( dwFileSize != dwBytesRead )
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in ReadFile of file %s",szFilePath, 0, true, SECONDLEVEL);
			CloseHandle( hFile ) ;
			dwRet = 0x04 ;
			goto Cleanup ;
		}
		
		if(!CreateRandomKeyFromFile(hFile, dwFileSize))
		{
			AddLogEntry(L"### CWardwizSCANNERBase : Error in CreateRandomKeyFromFile", 0, 0, true, SECONDLEVEL);
			CloseHandle( hFile ) ;
			dwRet = 0x08 ;
			goto Cleanup ;
		}
		CloseHandle( hFile ) ;
		if( DecryptData( (LPBYTE)lpFileData, dwBytesRead ) )
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in DecryptData", 0, 0, true, SECONDLEVEL);
			dwRet = 0x05 ;
			goto Cleanup ;
		}
		
		::SetFileAttributes(szFilePath, FILE_ATTRIBUTE_NORMAL);

		hFileEnc = CreateFile(szFilePath, GENERIC_WRITE, 0, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFileEnc == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in opening file %s",szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x06 ;
			goto Cleanup ;
		}

		dwBytesRead = 0x00 ;
		SetFilePointer( hFileEnc, 0x00, NULL, FILE_BEGIN ) ;
		WriteFile(hFileEnc, WRDWIZ_SIG, WRDWIZ_SIG_SIZE, &dwBytesRead, NULL); // Write sig "WARDWIZ"
		if(dwBytesRead != WRDWIZ_SIG_SIZE)
			dwRet = 0x9;

		SetFilePointer( hFileEnc, (0x00 + WRDWIZ_SIG_SIZE), NULL, FILE_BEGIN ) ;
		WriteFile(hFileEnc, m_pbyEncDecKey, WRDWIZ_KEY_SIZE, &dwBytesRead, NULL); // Write Encryption key
		if(dwBytesRead != WRDWIZ_KEY_SIZE)
			dwRet = 0x9;

		SetFilePointer( hFileEnc, (0x0 + WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE), NULL, FILE_BEGIN ) ;
		WriteFile( hFileEnc, lpFileData, dwFileSize, &dwBytesRead, NULL ) ; // Write encrypted data in file
		if( dwFileSize != dwBytesRead )
			dwRet = 0x07 ;

		CloseHandle( hFileEnc ) ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup :

	if( lpFileData )
		free( lpFileData ) ;
	lpFileData = NULL ;

	if(m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}
	return dwRet ;
}

DWORD CClamScanner::DecryptData( LPBYTE lpBuffer, DWORD dwSize )
{
	__try
	{
		if( IsBadWritePtr( lpBuffer, dwSize ) )
			return 1 ;

		DWORD	iIndex = 0 ;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			//if(lpBuffer[iIndex] != 0)
			{
				lpBuffer[iIndex] ^= m_pbyEncDecKey[jIndex++];
				if (jIndex == WRDWIZ_KEY_SIZE)
				{
					jIndex = 0x00;
				}
				if (iIndex >= dwSize)
				{
					break;
				}
			}
		}	
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CClamScanner::DecryptData", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

CString CClamScanner::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = {0};
		if(!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

bool CClamScanner::SaveInRecoverDB()
{
	OutputDebugString(L">>> In SaveInReportsDB");

	bool bReturn = false;
	__try
	{
		if(!m_ptrISpyDBManipulation)
		{
			return bReturn;
		}
		bReturn = m_ptrISpyDBManipulation->SaveEntries(0x00);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return  bReturn;
}

void CClamScanner::LoadRecoversDBFile()
{
	try
	{
		if(!m_ptrISpyDBManipulation)
		{
			return;
		}
		m_ptrISpyDBManipulation->LoadEntries(0x00);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CClamScanner::LoadRecoversDBFile", 0, 0, true, SECONDLEVEL);
	}
}

//bool CClamScanner::RecoverFile(LPTSTR lpFilePath, LPTSTR lpBrowseFilePath,DWORD dwTypeofAction)
DWORD CClamScanner::RecoverFile(LPTSTR lpFilePath, LPTSTR lpBrowseFilePath, DWORD dwTypeofAction)
{
	//bool bReturn = false;
	CString csDuplicatePath;

	DWORD	dwRet = 0x00;

	//if(!RecoverCurrentFile(lpFilePath,lpBrowseFilePath,dwTypeofAction))
	dwRet = RecoverCurrentFile(lpFilePath, lpBrowseFilePath, dwTypeofAction);
	if (dwRet)
	{
		AddLogEntry(L"### File(s) not recovered : %s", lpFilePath, 0, true, SECONDLEVEL);
		//bReturn = false;
	}
	else
	{
		if(dwTypeofAction == 0)//0 is for Recover
		{
			//Action to be needed remove entry from recoverdb
			AddLogEntry(L">>> File(s) recovered : %s", lpFilePath, 0, true, FIRSTLEVEL);
			
			csDuplicatePath=m_szQuarantineFilePath;
			
			m_ptrISpyDBManipulation->m_RecoverDBEntries.RemoveContact(lpFilePath,csDuplicatePath);
			
			//bReturn = true;
			dwRet = 0x00;
		}
		else if(dwTypeofAction == 1)//1 is for Deleting the entries in RecoverDB
		{
			//Action to be needed remove entry from recoverdb
			AddLogEntry(L">>> File(s) Deleted : %s", lpFilePath, 0, true, FIRSTLEVEL);
			csDuplicatePath=m_szQuarantineFilePath;
			m_ptrISpyDBManipulation->m_RecoverDBEntries.RemoveContact(lpFilePath,csDuplicatePath);
			//m_objRecoverdb.RemoveContact(lpFilePath,csDuplicatePath);
			
			dwRet = 0x00;
			//bReturn = true;
		}
	}

	return dwRet;
}

//bool CClamScanner::RecoverCurrentFile(CString csThreatPath , CString csBrowseFilePath,DWORD dwTypeofAction)
//Signature changed because we need to send extact reason of recovered failed to GUI
DWORD CClamScanner::RecoverCurrentFile(CString csThreatPath, CString csBrowseFilePath, DWORD dwTypeofAction)
{
	AddLogEntry(L">>> Recovering File : %s", csThreatPath, 0, true, FIRSTLEVEL);

	CString csQuarantineFilePath = L"";
	TCHAR	*pFileName = NULL ;
	TCHAR  szFileName[MAX_PATH] = {0};
	TCHAR  szBrowseFilePath[MAX_PATH] = {0};
	DWORD dwStatus=0 , dwRet = 0x00;
	CFile hBrowseFile;
	// get a reference to the contacts list
	const ContactList& contacts = m_ptrISpyDBManipulation ->m_RecoverDBEntries.GetContacts();

	// iterate over all contacts add add them to the list
		
	POSITION pos = contacts.GetHeadPosition();
	
	while(pos != NULL)
	{
		const CIspyList contact = contacts.GetNext(pos);
		if(csThreatPath == contact.GetFirstEntry())
		{
			csQuarantineFilePath=contact.GetSecondEntry();

			if(dwTypeofAction == 0)
			{			
					_tcscpy(m_szQuarantineFilePath,csQuarantineFilePath);
					_tcscpy(m_szOriginalThreatPath,csThreatPath);
					
					//check here if original path is present or not if not create the full directory path
					if(!PathFileExists(m_szOriginalThreatPath))
					{
						if(!CreateFullDirectoryPath((wchar_t *)m_szOriginalThreatPath))
						{
							pFileName = wcsrchr( m_szOriginalThreatPath, '\\' ) ;
							if(!pFileName)
							{
								AddLogEntry(L"### Failed to CreateFullDirectoryPath", 0, 0, true, SECONDLEVEL);
								//return false;
								dwRet = 0x01;
								return dwRet;
							}
							_tcscpy(szFileName,pFileName);
							_tcscpy(szBrowseFilePath,csBrowseFilePath);
							_tcscat(szBrowseFilePath,szFileName);
							_tcscpy(m_szOriginalThreatPath , szBrowseFilePath);
							if(!PathFileExists(m_szOriginalThreatPath))
							{
								if (hBrowseFile.Open(m_szOriginalThreatPath, CFile::modeCreate | CFile::modeWrite) == FALSE)
								{
									//return false;
									dwRet = 0x01;
									return dwRet;
								}

								hBrowseFile.Close();
								if(!PathFileExists(m_szOriginalThreatPath))
								{
									if(!PathIsNetworkPath(m_szOriginalThreatPath))
									{
										AddLogEntry(L"### Failed to CreateFullDirectoryPath", 0, 0, true, SECONDLEVEL);
										//return false;
										dwRet = 0x01;
										return dwRet;
									}
								}
							}
						}
					}

					if(PathFileExists(csQuarantineFilePath))
					{
						dwRet = Decrypt_File(m_szQuarantineFilePath, m_szOriginalThreatPath, dwStatus);
						if(dwRet != 0x00)
						{
							//AddLogEntry(L"### CClamScanner::RecoverCurrentFile failed to Decrypt file", 0, 0, true, SECONDLEVEL);
							//return false;
							dwRet = RecoverInUseFileIfPossible(m_szQuarantineFilePath, m_szOriginalThreatPath, dwStatus);
							if (dwRet)
							{
								dwRet = 0x02;
								AddLogEntry(L"### CClamScanner::RecoverCurrentFile failed due to file in use:%s", m_szOriginalThreatPath, 0, true, SECONDLEVEL);
								return dwRet;
							}
							else
							{
								dwRet = 0x00;
								return dwRet;
							}
						}
						else
						{
							dwRet = 0x00;
							return dwRet;
						}
					}
					else
					{
						AddLogEntry(L"### Path not exists : %s", csQuarantineFilePath, 0, true, SECONDLEVEL);
						//return false;
						dwRet = 0x01;
						return dwRet;
					}
			}
			else if(dwTypeofAction ==1)
			{
				_tcscpy(m_szQuarantineFilePath,csQuarantineFilePath);

				SetFileAttributes(csQuarantineFilePath, FILE_ATTRIBUTE_NORMAL);
				if(!DeleteFile(csQuarantineFilePath))
				{
					AddLogEntry(L"### Path not exists : %s", csQuarantineFilePath, 0, true, SECONDLEVEL);
				}
				
			}
		}
		
	}	

	return dwRet;
}

DWORD CClamScanner::Decrypt_File(TCHAR* szRecoverFilePath, TCHAR* szOriginalThreatPath, DWORD &dwStatus)
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00 ;
	TCHAR	szTemp[1024] = {0} ;
	TCHAR	szExt[16] = {0} ;
	DWORD	dwLen = 0x00 ;
	LPBYTE	lpFileData = NULL ;
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;
	DWORD FlagToCreateFile;
	bool bAccessflag = false;
	//m_dwEnKey = *( (DWORD * )&szpIpsy ) ;

	__try
	{

		if( !PathFileExists( szRecoverFilePath ) )
		{
			//AfxMessageBox( TEXT("Please select file for operation") ) ;
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		::SetFileAttributes(szRecoverFilePath, FILE_ATTRIBUTE_NORMAL);
		hFile = CreateFile(	szRecoverFilePath, GENERIC_READ, 0, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		dwFileSize = GetFileSize( hFile, NULL ) ;
		if( !dwFileSize )
		{
			CloseHandle( hFile ) ;
			dwRet = 0x03 ;
			goto Cleanup ;
		}

	//	lpFileData = (LPBYTE ) malloc( dwFileSize + 1 ) ;
		lpFileData = (LPBYTE ) malloc( dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) ;
		if( !lpFileData )
		{
			CloseHandle( hFile ) ;
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		memset(lpFileData, 0x00, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE))) ;
		SetFilePointer( hFile, (0x00 + (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), NULL, FILE_BEGIN ) ;
		ReadFile( hFile, lpFileData, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), &dwBytesRead, NULL ) ;
		if( (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) != dwBytesRead )
		{
			CloseHandle( hFile ) ;
			dwRet = 0x04 ;
			goto Cleanup ;
		}
		//read key from file
		if(!ReadKeyFromEncryptedFile(hFile))
		{
			CloseHandle( hFile ) ;
			dwRet = 0x04 ;
			goto Cleanup ;
		}
		CloseHandle( hFile );

		if( DecryptData( (LPBYTE)lpFileData, dwBytesRead ) )
		{
			dwRet = 0x05 ;
			goto Cleanup ;
		}

		wcscpy(szTemp, szOriginalThreatPath ) ;
		//szTemp[ dwLen-0x05 ] = '\0' ;
		if(PathFileExists(szTemp))
		{
			SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
			if(!(DeleteFile(szTemp)))
			{
				AddLogEntry(L"### Delete existing %s File ",szTemp, 0, true, SECONDLEVEL);
				FlagToCreateFile = OPEN_EXISTING;
				bAccessflag = true;
			}
			if(bAccessflag == false)
			{
				FlagToCreateFile = OPEN_ALWAYS;
			}
		}
		else
		{
			if (PathIsNetworkPath(szTemp))
			{
				SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
				if(!(DeleteFile(szTemp)))
				{
					int err = GetLastError();
					AddLogEntry(L"### Delete existing %s File ",szTemp, 0, true, SECONDLEVEL);
					FlagToCreateFile = OPEN_EXISTING;
					bAccessflag = true;
				}
				
			}
			if(bAccessflag == false)
			{
				FlagToCreateFile = OPEN_ALWAYS;
			}
		}

		::SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
		hFileEnc = CreateFile(	szTemp, GENERIC_READ|GENERIC_WRITE, 0, NULL,
								FlagToCreateFile, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFileEnc == INVALID_HANDLE_VALUE )
		{
			int err = GetLastError();
			dwRet = 0x06 ;
			goto Cleanup ;
		}

		dwBytesRead = 0x00 ;
		WriteFile( hFileEnc, lpFileData, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), &dwBytesRead, NULL ) ;
		if( (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) != dwBytesRead )
			dwRet = 0x07 ;
		CloseHandle( hFileEnc ) ;

		SetFileAttributes(szRecoverFilePath, FILE_ATTRIBUTE_NORMAL);
		if (!(DeleteFile(szRecoverFilePath)))
		{
			AddLogEntry(L"### Quarantine %s File not deleted after decrypt data.",szRecoverFilePath, 0, true, SECONDLEVEL);
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}
Cleanup :

	if( lpFileData )
		free( lpFileData ) ;
	lpFileData = NULL ;

	if(m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}

	return dwRet ;
}

bool CClamScanner::CreateFullDirectoryPath(wchar_t *szFullPath)
{
	__try
	{
		if(!szFullPath)
		{
			return false;
		}

		if(wcslen(szFullPath) == 0)
		{
			return false;
		}

		wchar_t szPath[MAX_PATH] = {0};
		wchar_t folder[MAX_PATH] = {0};
		wchar_t *end = NULL;

		ZeroMemory(folder, MAX_PATH * sizeof(wchar_t));

		_tcscpy(szPath, szFullPath);

		end = wcschr(szPath, L'\\');
		while(end != NULL)
		{
			wcsncpy(folder, szPath, end - szPath + 1);
			if(!CreateDirectory(folder, NULL))
			{
				DWORD err = GetLastError();

				if(err != ERROR_ALREADY_EXISTS)
				{
					// do whatever handling you'd like
				}
			}
			end = wcschr(++end, L'\\');
		}

		if(PathFileExists(folder))
		{
			return true;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CClamScanner::CreateFullDirectoryPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

bool CClamScanner::CheckForDuplicates(CString csFilePath)
{
	bool bReturn = false;
	for(std::vector<CString>::iterator it = m_vFileDuplicates.begin(); it != m_vFileDuplicates.end(); ++it)
	{
		if(csFilePath == (*it))
		{
			bReturn = true;
			break;
		}
	}
	return bReturn;
}

void CClamScanner::SetLastScanDatetime()
{
	try
	{
		TCHAR szOutMessage[30] = {0};

		TCHAR tbuffer[9]= {0};
		TCHAR dbuffer[9] = {0};
		_wstrtime_s(tbuffer, 9);
		//Issue No. 599 Rajil Yadav 5/6/14
		CString  csDate;
		SYSTEMTIME  CurrTime = {0} ;
		GetLocalTime(&CurrTime); //Ram, Issue resolved:0001218
		CTime Time_Curr( CurrTime ) ;
		int iMonth = Time_Curr.GetMonth();
		int iDate = Time_Curr.GetDay();
		int iYear = Time_Curr.GetYear();
		csDate.Format(L"%d/%d/%d",iMonth, iDate, iYear);

		_stprintf(szOutMessage, _T("%s %s\0"), csDate, tbuffer);

		CITinRegWrapper	objReg;
		if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"LastScandt",
			szOutMessage) != 0)
		{
			AddLogEntry(L"### SetRegistryValueData failed in LastScandt CClamScanner::SetLastScandatetime", 0, 0, true, SECONDLEVEL);
		}

		DWORD ScanType = 0;
		switch(m_eScanType)
		{
		case FULLSCAN: ScanType = 0;
			break;
		case CUSTOMSCAN: ScanType = 1;
			break;
		case QUICKSCAN: ScanType = 2;
			break;
		case USBSCAN: ScanType = 3;
			break;
		case USBDETECT: ScanType = 4;
			break;
		}

		if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"ScanType",
			ScanType) != 0)
		{
			AddLogEntry(L"### SetRegistryValueData failed in ScanType CClamScanner::SetLastScandatetime", 0, 0, true, SECONDLEVEL);
		}

		if(m_iThreatsFoundCount > 0)
		{
			if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"VirusFound",
				0x01) != 0)
			{
				AddLogEntry(L"### SetRegistryValueData failed in VirusFound CClamScanner::SetLastScandatetime", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CClamScanner::SetLastScandatetime", 0, 0, true, SECONDLEVEL);
	}
}

//Clear here all memory map objects to avoid duplicates
void CClamScanner::ClearMemoryMapObjects()
{
	ITIN_MEMMAP_DATA iTinMemMap = {0};
	m_objServViruEntry.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
	m_objServ.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
}

//Prajakta N.
bool CClamScanner::CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize)
{
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	bool			bReturn = false;
	int				iTmp = 0x00;
	int				iIndex = 0x00, jIndex = 0x00;
	int				iRandValue = 0x00, iReadPos = 0x00;
	unsigned char	szChar = 0x0;

	iTmp = dwFileSize / WRDWIZ_KEY_SIZE;

	if(m_pbyEncDecKey == NULL)
	{
		m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE,sizeof(unsigned char)); 
	}
	for(iIndex = 0x00, jIndex = 0x00; iIndex < iTmp; iIndex++, jIndex++)
	{
		if (jIndex >= WRDWIZ_KEY_SIZE)
		{
			break;
		}

		iRandValue = rand();
		iRandValue = iRandValue % WRDWIZ_KEY_SIZE;

		iReadPos = (iIndex *  WRDWIZ_KEY_SIZE) +  iRandValue;

		DWORD dwBytesRead = 0x0;
		SetFilePointer( hFile, iReadPos, NULL, FILE_BEGIN ) ;
		ReadFile( hFile, &szChar, sizeof(BYTE), &dwBytesRead, NULL ) ;
		
		if (szChar == 0x00)
		{
			szChar = iRandValue;
		}
		m_pbyEncDecKey[jIndex] = szChar;

		if (iIndex == (iTmp - 0x01) && jIndex < WRDWIZ_KEY_SIZE)
		{
			iIndex = 0x00;
		}
	}
	return true;
}


//Prajakta N.
bool CClamScanner::ReadKeyFromEncryptedFile(HANDLE hFile)
{
	bool	bReturn = false;
	int		iReadPos = 0x0;
	DWORD   dwBytesRead = 0x0;

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return bReturn;
	}
	
	if(!IsFileAlreadyEncrypted(hFile))
	{
		return bReturn;
	}
	//read encryption key
	if(m_pbyEncDecKey == NULL)
	{
		m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE,sizeof(unsigned char)); 
	}
	memset(m_pbyEncDecKey, 0x00, WRDWIZ_KEY_SIZE * sizeof(unsigned char));
	SetFilePointer(hFile, (0x0 + WRDWIZ_SIG_SIZE), NULL, FILE_BEGIN);
	ReadFile( hFile, &m_pbyEncDecKey[0x0], WRDWIZ_KEY_SIZE * sizeof(unsigned char), &dwBytesRead, NULL ) ;
	if (dwBytesRead != WRDWIZ_KEY_SIZE)
	{
		return bReturn;
	}
	bReturn = true;
	return bReturn;
}

//Prajakta N.
bool CClamScanner::IsFileAlreadyEncrypted(HANDLE hFile)
{
	bool	bReturn = false;
	int		iReadPos = 0x0;
	DWORD   dwBytesRead = 0x0;
	unsigned char	bySigBuff[WRDWIZ_SIG_SIZE] = {0x00};

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return bReturn;
	}
	
	//check if file is already encrypted by checking existence of sig "WARDWIZ"
	SetFilePointer(hFile, iReadPos, NULL, FILE_BEGIN);
	ReadFile( hFile, &bySigBuff[0x0], WRDWIZ_SIG_SIZE * sizeof(unsigned char), &dwBytesRead, NULL ) ;
	if(dwBytesRead != WRDWIZ_SIG_SIZE)
	{
		return bReturn;
	}
	if(memcmp(&bySigBuff, WRDWIZ_SIG, WRDWIZ_SIG_SIZE) == 0)
	{
		bReturn = true;
	}
	else
	{
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : SendMessage2Tray
  Description    : Send Respective Message to tray.
  Author Name    : Neha Gharge
  S.R. No        : 
  Date           : 31 Dec 2014
****************************************************************************/
bool CClamScanner::SendMessage2Tray(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(TRAY_SERVER, false);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CClamScanner::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CClamScanner::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
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
bool CClamScanner::IsDuplicateModule( LPTSTR szModulePath )
{
	bool bReturn = false;
	try
	{
		_tcsupr(szModulePath);
		if(m_csaModuleList.Find(szModulePath, 0) != NULL)
		{
			bReturn = true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CClamScanner::IsDuplicateModule", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************            
*  Function Name  :	RemoveModuleFromList
*  Description    :	Function which removes file entry fro Modules List which is scanned by Clam.
*  Author Name    : Ram Shelke                                                                            
*  Date           : 02 Feb 2015
*  SR_NO		  : 
**********************************************************************************************************/
void CClamScanner::RemoveModuleFromList(LPTSTR pszModulePath)
{
	_tcsupr(pszModulePath);
	POSITION pos = m_csaModuleList.Find(pszModulePath, 0);
	if(pos != NULL)
	{
		m_csaModuleList.RemoveAt(pos);
	}
	else
	{
		AddLogEntry(L"### Failed to Remove entry: %s", pszModulePath, 0, true, ZEROLEVEL);
	}
}

/**********************************************************************************************************            
*  Function Name  :	ScanFilesExcludedByClam
*  Description    :	Function which scans files which are excluded by clam.
					This issue is on 
*  Author Name    : Ram Shelke                                                                            
*  Date           : 02 Feb 2015
*  SR_NO		  : 
**********************************************************************************************************/
void CClamScanner::ScanFilesExcludedByClam()
{
	DWORD dwCount = static_cast<DWORD>(m_csaModuleList.GetCount());

	if(dwCount == 0)
	{
		return;//No files excluded need to return.
	}

	CString csLog;
	csLog.Format(L">>> Remaining Files to scan: %d", dwCount);
	AddLogEntry(csLog);

	POSITION pos = m_csaModuleList.GetHeadPosition();
	//Issue : 10. In Quick scan, If we abort the scan then, Aborted message not coming immediately, and other scan buttons in Tab not getting enabled
	//Resolved By : Nitin K. Date: 03rd - Feb - 2015
	while( pos != NULL && !m_bManualStop)
	{
		CString csModulePath = m_csaModuleList.GetNext(pos);
		if(csModulePath.GetLength() > 0)
		{
			ScanForSingleFile(csModulePath);
		}
	}
}

/**********************************************************************************************************            
*  Function Name  :	ScanForSingleFile                                                     
*  Description    :	Scan each single file .
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 02 Feb 2015
*  SR_NO		  : 
**********************************************************************************************************/
void CClamScanner::ScanForSingleFile(CString csFilePath)
{
	if(csFilePath.GetLength() == 0)
		return;

	try
	{
		bool bSetStatus = false;
		bool bVirusFound = false;
		
		CString csVirusName(L"");
		CString csVirusPath(L"");
		DWORD dwISpywareID = 0;
		CString csCurrentFile(L"");
		CString csStatus(L"");

		if(csFilePath.Trim().GetLength() > 0)
		{
			if(PathFileExists(csFilePath))
			{
				DWORD dwISpyID = 0;
				TCHAR szVirusName[MAX_PATH] = {0};
				DWORD dwSignatureFailedToLoad = 0;

				if (!m_bISOnlyMemScan)
					m_dwTotalFileCount++; //File scanned count

				//Added by Vilas on 26 Mar 2015
				//Checking file entry present in reboot repair ini (WWRepair.ini)
				if (CheckFileIsInRepairRebootIni(csFilePath))
					return;

				//Added by Vilas on 27 Mar 2015
				//Checking file entry present in recover ini (WWRecover.ini)
				if (CheckFileIsInRecoverIni(csFilePath))
					return;

				//Added by Neha Gharge on 10 April 2015
				//Checking file entry present in recover ini (WWRecover.ini)
				if (CheckFileIsInDeleteIni(csFilePath))
					return;
				

				if(m_objISpyScanner.ScanFile(csFilePath, szVirusName, dwISpyID,dwSignatureFailedToLoad,m_bRescan))
				{
					if(dwISpyID >= 0)
					{
						csVirusName = szVirusName;
						dwISpywareID = dwISpyID;
						bVirusFound = true;
					}
				}
			}
		}

		//virus found 
		if(bVirusFound)
		{
			bSetStatus = true;
			if (HandleVirusFoundEntries(csVirusName, csFilePath, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"), dwISpywareID))
			{
				m_iThreatsFoundCount++;
				AddEntriesInReportsDB(m_eScanType, csVirusName, csFilePath, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"));
			}
		}
		else
		{
			bSetStatus = true;
			csStatus = csFilePath + L": OK";
		}

		if (!m_bISOnlyMemScan)
		{
			if (bSetStatus)
			{
				if (m_bSendScanMessage)
				{
					if (!SendMessage2UI(SCAN_STARTED))
					{
						AddLogEntry(L"### Failed to send SCAN_STARTED message to UI", 0, 0, true, SECONDLEVEL);
					}
					m_bSendScanMessage = false;
				}

				//Varada Ikhar, Date:24/01/2015, Adding log entry of file path at zero level.
				AddLogEntry(L">>> %s :%s", L"FilePath", csStatus, true, ZEROLEVEL);
				SetScanStatus(csStatus);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in void CClamScanner::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
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
void CClamScanner::LoadPSAPILibrary()
{
	__try
	{
		TCHAR	szSystem32[256] = {0} ;
		TCHAR	szTemp[256] = {0} ;
		GetSystemDirectory( szSystem32, 255 ) ;

		ZeroMemory( szTemp, sizeof(szTemp) ) ;
		wsprintf(szTemp, L"%s\\PSAPI.DLL", szSystem32 ) ;
		if( !m_hPsApiDLL )
		{
			m_hPsApiDLL = LoadLibrary( szTemp ) ;
		}

		if(!EnumProcessModulesWWizEx)
		{
			EnumProcessModulesWWizEx	= (ENUMPROCESSMODULESEX ) GetProcAddress( m_hPsApiDLL, "EnumProcessModulesEx" ) ;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CClamScanner::LoadPSAPILibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : AddDeleteFailEntryToINI
Description    : Add entry of files whose failed in delete.
Author Name    : Neha Gharge
S.R. No        :
Modified Date  : 04th March 2015
****************************************************************************/
bool CClamScanner::AddDeleteFailEntryToINI(CString csQurFilePaths, CString csVirusName)
{
	TCHAR szValueName[260] = { 0 };
	DWORD dwCount = 0x00;
	TCHAR szDuplicateString[512] = { 0 };
	TCHAR szTempKey[512] = { 0 };
	TCHAR szApplnName[512] = { 0 };
	TCHAR szConcatnateDeleteString[1024] = { 0 };
	TCHAR szFileName[512] = { 0 };
	TCHAR szQuarantineFileName[512] = { 0 };
	TCHAR szVirusName[512] = { 0 };
	bool	bDupFlag = false;

	try
	{

		CString csDeleteFailedINIPath(L"");
		CString csQuarantineFolderPath = GetQuarantineFolderPath();

		csDeleteFailedINIPath.Format(L"%s\\WRDWIZDELETEFAIL.INI", csQuarantineFolderPath);

		if (!PathFileExists(csQuarantineFolderPath))
		{
			if (!CreateDirectory(csQuarantineFolderPath, NULL))
			{
				AddLogEntry(L"### CGenXScanner::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
			}
		}

		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csDeleteFailedINIPath);
		m_dwFailedDeleteFileCount = dwCount;

		if (dwCount)
		{
			for (int i = 0x01; i <= static_cast<int>(dwCount); i++)
			{
				ZeroMemory(szTempKey, sizeof(szTempKey));
				swprintf_s(szTempKey, _countof(szTempKey), L"%lu", i);

				GetPrivateProfileString(L"DeleteFiles", szTempKey, L"", szDuplicateString, 511, csDeleteFailedINIPath);
				ZeroMemory(szApplnName, sizeof(szApplnName));

				if (!TokenizationOfParameterForrecover(szDuplicateString, szFileName, _countof(szFileName), szQuarantineFileName,_countof(szQuarantineFileName), szVirusName,_countof(szVirusName)))
				{
					AddLogEntry(L"### TokenizationOfParameterForrecover is not tokenize properly", 0, 0, true, FIRSTLEVEL);
					return false;
				}

				if (!TokenizeIniData(szFileName, szApplnName, _countof(szApplnName)))
				{
					AddLogEntry(L"### TokenizeIniData is not tokenize properly", 0, 0, true, FIRSTLEVEL);
					return false;
				}

				CString csDuplicateString = (CString)szDuplicateString;
				CString csFileName = (CString)szApplnName;
				if (csFileName.CompareNoCase(csQurFilePaths) == 0)
				{
					AddLogEntry(L">>> File Entry already present in deletefiles.ini", 0, 0, true, FIRSTLEVEL);
					bDupFlag = false;
					break;
				}
				else
				{
					bDupFlag = true;
				}
			}

			if (bDupFlag)
			{
				if (!MakeConcatStringforrecover(csQurFilePaths, m_csDuplicateName, csVirusName, szConcatnateDeleteString,_countof(szConcatnateDeleteString)))
				{
					AddLogEntry(L"### MakeConcatStringforrecover is not caoncatenate properly", 0, 0, true, FIRSTLEVEL);
					return false;
				}
				ZeroMemory(szValueName, sizeof(szValueName));
				swprintf_s(szValueName, _countof(szValueName), L"%lu", ++m_dwFailedDeleteFileCount);
				WritePrivateProfileString(L"DeleteFiles", szValueName, szConcatnateDeleteString, csDeleteFailedINIPath);

				ZeroMemory(szValueName, sizeof(szValueName));
				swprintf_s(szValueName, _countof(szValueName), L"%lu", m_dwFailedDeleteFileCount);
				WritePrivateProfileString(L"Count", L"Count", szValueName, csDeleteFailedINIPath);
				bDupFlag = false;
			}

		}
		else
		{
			if (!MakeConcatStringforrecover(csQurFilePaths, m_csDuplicateName, csVirusName, szConcatnateDeleteString,_countof(szConcatnateDeleteString)))
			{
				AddLogEntry(L"### MakeConcatStringforrecover is not caoncatenate properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", ++m_dwFailedDeleteFileCount);
			WritePrivateProfileString(L"DeleteFiles", szValueName, szConcatnateDeleteString, csDeleteFailedINIPath);

			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", m_dwFailedDeleteFileCount);
			WritePrivateProfileString(L"Count", L"Count", szValueName, csDeleteFailedINIPath);

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::AddDeleteFailEntryToINI", 0, 0, true, SECONDLEVEL);
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
bool CClamScanner::TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName)
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
			wcscpy_s(szValueApplicationName,_countof(szValueApplicationName),pToken);
			wcscpy_s(szApplicationName, (dwSizeofApplicationName - 1),szValueApplicationName);
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
		AddLogEntry(L"### Exception in CClamScanner::TokenizeIniData", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
Function Name  : SendProcessID2Protect
Description    : to add process protection to scanner.
Author Name    : Lalit kumawat
S.R. No        :
Date           : 3-13-015
****************************************************************************/
bool CClamScanner::SendProcessID2Protect(PROCESS_INFORMATION pi)
{
	try
	{
		if (pi.dwProcessId == 0)
		{
			AddLogEntry(L"### Process id getting 0 in CClamScanner::SendProcessID2Protect", 0, 0, true, SECONDLEVEL);
			return false;
		}

		CSecure64  objCSecure;
		int iProcessID;
		iProcessID = pi.dwProcessId;

		objCSecure.RegisterProcessIdAndCode(10, iProcessID);  // to register service for process protection

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CClamScanner::SendProcessID2Protect", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/***************************************************************************
Function Name  : AddRepairEntryAfterReboot
Description    : adds file path, virus name and virus ID in WWRepair.ini to repair on reboot.
Author Name    : Vilas Suvarnakar
S.R. No        :
Date           : 23 Mar 2015
****************************************************************************/
void CClamScanner::AddRepairEntryAfterReboot(LPCTSTR szThreatPath, LPCTSTR szThreatName, CString csDuplicateName, DWORD dwISpyID)
{
	try
	{
		TCHAR	szData[2048] = { 0 };
		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRepairIniPath))
		{
			AddLogEntry(L"### Failed in AddRepairEntryAfterReboot::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return;
		}

		//GetEnvironmentVariable(L"ALLUSERSPROFILE", szRepairIniPath, (MAX_PATH - 1));
		wcscat_s(szRepairIniPath, _countof(szRepairIniPath), L"\\WWRepair.ini");

		DWORD dwRepairCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRepairIniPath);

		DWORD	i = 0x01;
		bool	bFound = true;

		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[2048] = { 0 };

		swprintf_s(szData, _countof(szData), L"%lu|%s|%s", dwISpyID, szThreatName, szThreatPath);
		_wcsupr(szData);

		for (; i <= dwRepairCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRepairIniPath);
			_wcsupr(szValueData);
			if (memcmp(szValueData, szData, wcslen(szData)*sizeof(TCHAR)) == 0)
			{
				bFound = false;
				break;
			}
		}

		if (bFound)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwRepairCount + 1);

			wcscat_s(szData, _countof(szData), L"|");
			wcscat_s(szData, _countof(szData), csDuplicateName);
			WritePrivateProfileString(L"Files", szValueName, szData, szRepairIniPath);

			WritePrivateProfileString(L"Count", L"Count", szValueName, szRepairIniPath);
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::AddRepairEntryAfterReboot", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************
Function Name  : AddRepairEntryAfterReboot
Description    : adds file path, virus name and virus ID in WWRepair.ini to repair on reboot.
Author Name    : Vilas Suvarnakar
S.R. No        :
Date           : 23 Mar 2015
Modified Date  : 24 Mar 2015
****************************************************************************/
bool CClamScanner::RescanAndRepairFile(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szDupName, DWORD dwISpyID)
{
	bool	bReturn = true;
	
	__try
	{
	
		bool	bRescan = true;

		DWORD	dwRepairID = 0;
		DWORD	dwFailedToLoadSignature = 0;

		TCHAR	szVirusName[512] = { 0 };

		_tcscpy(szVirusName, szThreatName);

		if (m_objISpyScanner.ScanFile(szThreatPath, szVirusName, dwISpyID, dwFailedToLoadSignature, bRescan))
		{
			if (!m_objISpyScanner.RepairFile(szThreatPath, dwISpyID))
			{
				bReturn = false;
				AddLogEntry(L"### RescanAndRepairFile Failed to repair file: %s", szThreatPath, 0, true, SECONDLEVEL);

				return bReturn;
			}

			if(!UpdateRecoverEntry(szThreatPath, szDupName, szVirusName, 0))
			{
				AddLogEntry(L"### Failed to update UpdateRecoverEntry in CClamScanner::RescanAndRepairFile");
			}
		}
		else
		{
			//For Recover handling when path failed
			//Vilas s 11/4/2015
			if (!PathFileExists(szThreatPath))
			{
				if (!UpdateRecoverEntry(szThreatPath, szDupName, szVirusName, 0))
				{
					AddLogEntry(L"### Failed to update UpdateRecoverEntry in CWardwizScanner::RescanAndRepairFile");
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in CWardwizScanner::RescanAndRepairFile", 0, 0, true, SECONDLEVEL);
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
bool CClamScanner::GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath)
{
	bool	bReturn = true;
	try
	{
		TCHAR	szModulePath[MAX_PATH] = { 0 };

		GetModulePath(szModulePath, MAX_PATH);
		if (!wcslen(szModulePath))
			return bReturn;

		wcscpy(lpszQuarantineFolPath, szModulePath);
		wcscat_s(lpszQuarantineFolPath, MAX_PATH - 1, L"\\Quarantine");
		bReturn = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	UpdateRecoverEntry
*  Description    :	Function to update recover entry
*  Author Name    : Ram Shelke
*  Date           : 24 March 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CClamScanner::UpdateRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowEntryStatus)
{
	try
	{
		LoadRecoversDBFile();

		CIspyList newEntry(szThreatPath, csDuplicateName, szThreatName, L"", L"", L"", dwShowEntryStatus);
		if (!m_ptrISpyDBManipulation)
		{
			return false;
		}
		m_ptrISpyDBManipulation->EditEntry(szThreatPath, newEntry, RECOVER);

		if (!SaveInRecoverDB())
		{
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CClamScanner::UpdateRecoverEntry", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : Checking file entry present in reboot repair ini (WWRepair.ini)
Description    : Parses WWRepair.ini and sends for scan and repair
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 23 March 2015
Modified Date  : 26 March 2015
***********************************************************************************************/
bool CClamScanner::CheckFileIsInRepairRebootIni(CString csFilePath)
{
	bool	bReturn = false;

	try
	{

		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRepairIniPath))
		{
			AddLogEntry(L"### Failed in CheckFileIsInRepairRebootIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScanner::CheckFileIsInRepairRebootIni", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}


/***********************************************************************************************
Function Name  : RecoverInUseFileIfPossible
Description    : Recover file if it is in use
SR.NO		   :
Author Name    : Vilas Suvarnakar
Modified Date  : 26 March 2015
***********************************************************************************************/
DWORD CClamScanner::RecoverInUseFileIfPossible(LPCTSTR szRecoverFilePath, LPCTSTR szOriginalThreatPath, DWORD &dwStatus)
{

	DWORD	dwRet = 0x00;
	
	__try
	{
		TCHAR	szRenamedFile[512] = { 0 };

		if (RenameFile(szOriginalThreatPath, szRenamedFile))
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		DWORD	dwStatus = 0x00;

		dwRet = Decrypt_File(m_szQuarantineFilePath, m_szOriginalThreatPath, dwStatus);
		if (!dwRet)
		{
			AddRenamedFileEntryInIni(szRenamedFile);
			goto Cleanup;
		}

		SetFileAttributes(szRenamedFile, FILE_ATTRIBUTE_NORMAL);
		_wrename(szRenamedFile, szOriginalThreatPath);

		dwRet = 0x02;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x03;
		AddLogEntry(L"### Exception in CClamScanner::RecoverInUseFileIfPossible", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet;
}

/***********************************************************************************************
Function Name  : RenameFile
Description    : Rename file if it is in use
SR.NO		   :
Author Name    : Vilas Suvarnakar
Modified Date  : 26 March 2015
***********************************************************************************************/
DWORD CClamScanner::RenameFile(LPCTSTR szOriginalThreatPath, LPTSTR lpszRenamedFilePath)
{

	DWORD	dwRet = 0x00;

	__try
	{

		SetFileAttributes(szOriginalThreatPath, FILE_ATTRIBUTE_NORMAL);
		swprintf_s(lpszRenamedFilePath, 511, L"%s.WW", szOriginalThreatPath);

		if (!_wrename(szOriginalThreatPath, lpszRenamedFilePath))
			return dwRet;

		Sleep(10);

		if (!_wrename(szOriginalThreatPath, lpszRenamedFilePath))
			return dwRet;

		dwRet = 0x01;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x02;
		AddLogEntry(L"### Exception in CClamScanner::RenameFile", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}


/***************************************************************************
Function Name  : AddRenamedFileEntryInIni
Description    : adds file in use path WWRecover.ini to avoid rescan and delete on reboot
Author Name    : Vilas Suvarnakar
S.R. No        :
Date           : 27 Mar 2015
****************************************************************************/
void CClamScanner::AddRenamedFileEntryInIni(LPCTSTR lpszInUseFilePath)
{
	try
	{
		TCHAR	szData[2048] = { 0 };
		TCHAR	szRecoverIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRecoverIniPath))
		{
			AddLogEntry(L"### Failed in AddRepairEntryAfterReboot::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return;
		}

		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), L"\\");
		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), WRDWIZRECOVERINI);

		DWORD dwRepairCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRecoverIniPath);

		DWORD	i = 0x01;
		bool	bFound = true;

		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[512] = { 0 };

		for (; i <= dwRepairCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			ZeroMemory(szValueData, sizeof(szValueData));

			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRecoverIniPath);

			if (_wcsicmp(szValueData, lpszInUseFilePath) == 0)
			{
				bFound = false;
				break;
			}
		}

		if (bFound)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwRepairCount + 1);

			WritePrivateProfileString(L"Files", szValueName, lpszInUseFilePath, szRecoverIniPath);

			WritePrivateProfileString(L"Count", L"Count", szValueName, szRecoverIniPath);
		}

		MoveFileEx(NULL, lpszInUseFilePath, MOVEFILE_DELAY_UNTIL_REBOOT);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CClamScanner::AddRenamedFileEntryInIni", 0, 0, true, SECONDLEVEL);
	}
}



/***********************************************************************************************
Function Name  : Checking file entry present in Recover ini (WWRecover.ini)
Description    : Parses WWRecover.ini and not sends to scan if file is found.
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 27 March 2015
***********************************************************************************************/
bool CClamScanner::CheckFileIsInRecoverIni(CString csFilePath)
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

			if (csFilePath.CompareNoCase(szValueData) == 0x00 )
			{
				bReturn = true;
				break;
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::CheckFileIsInRecoverIni", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***********************************************************************************************
Function Name  : MakeConcatStringforrecover
Description    : concatenate string with bar to store it into delete file ini
SR.NO		   :
Author Name    : Neha Gharge
Date           : 8 April 2015
***********************************************************************************************/
bool CClamScanner::MakeConcatStringforrecover(CString csFilename, CString csQuarantinePath, CString csVirusName, TCHAR * szConcateDeleteString, DWORD dwsizeofConcatenateString)
{
	try
	{
		if (szConcateDeleteString == NULL)
			return false;

		CString csConcateDeleteString(L"");
		csConcateDeleteString.Format(L"%s|%s|%s", csFilename, csQuarantinePath, csVirusName);
		_tcscpy_s(szConcateDeleteString, (dwsizeofConcatenateString -1), csConcateDeleteString);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::MakeConcatStringforrecover", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***********************************************************************************************
Function Name  : TokenizationOfParameterForrecover
Description    : Tokenize input path to get file name, quarantine name and virus name
SR.NO		   :
Author Name    : Neha Gharge
Date           : 8 April 2015
***********************************************************************************************/
bool CClamScanner::TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName)
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
			wcscpy_s(szFileName, (dwsizeofFileName -1), pToken);
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szQuarantinepath, (dwsizeofquarantinefileName -1), pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szVirusName,(dwsizeofVirusName-1), pToken);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::TokenizationOfParameterForrecover", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInDeleteIni
*  Description    :	Check whether file available in delete.ini
*  Author Name    : Neha Gharge
*  Date           : 10 April 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CClamScanner::CheckFileIsInDeleteIni(CString csQurFilePaths)
{
	bool  bReturn = false;
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
				AddLogEntry(L"### CheckFileIsInDeleteIni::TokenizeIniData is not tokenize properly", 0, 0, true, FIRSTLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScanner::CheckFileIsInDeleteIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadSignatureDatabase
Description    : Load WardWiz Signature Datbase
SR.NO		   :
Author Name    : Ram Shelke
Date           : 27 April 2015
***********************************************************************************************/
DWORD CClamScanner::LoadSignatureDatabase(DWORD &dwSigCount)
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.LoadSignatureDatabase(dwSigCount);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CClamScanner::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***********************************************************************************************
Function Name  : UnLoadSignatureDatabase
Description    : To UnLoad WardWiz Signature Datbase
SR.NO		   :
Author Name    : Ram Shelke
Date           : 27 April 2015
***********************************************************************************************/
DWORD CClamScanner::UnLoadSignatureDatabase()
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.UnLoadSignatureDatabase();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CClamScanner::UnLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***********************************************************************************************
Function Name  : MemoryScanThread
Description    : Thread which scan Processed and module loaded by that processes
SR.NO		   :
Author Name    : Ram Shelke
Date           : 3 Dec 2015
***********************************************************************************************/
DWORD WINAPI MemoryScanThread(LPVOID lpvThreadParam)
{
	__try
	{
		CClamScanner *pThis = (CClamScanner*)lpvThreadParam;
		if (!pThis)
			return 0;

		pThis->GetModuleCount();

		if (!pThis->SendMessage2UI(SCAN_STARTED))
		{
			AddLogEntry(L"### Failed to send SCAN_STARTED message to UI", 0, 0, true, SECONDLEVEL);
		}

		pThis->m_bISOnlyMemScan = true;

		pThis->EnumerateProcesses();

		pThis->ReInitializeVariables();

		pThis->m_bISOnlyMemScan = false;

		pThis->MemoryScanFinished();

		pThis->ResumeClamScan();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in MemoryScanThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	SuspendClamScan
*  Description    :	Function to Suspend Clam Scanner
*  Author Name    : Ram Shelke
*  Date           : 3rd Dec 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CClamScanner::SuspendClamScan()
{
	//OutputDebugString(L">>> CClamScanner::SuspendClamScan");

	if (pi.hProcess != INVALID_HANDLE_VALUE)
	{
		SuspendThread(pi.hThread);
	}

	ResetEvent(m_hEventMemScanFinished);

	m_bIsClamSuspended = true;

	AddLogEntry(L">>> WardwizSCANNER suspended", 0, 0, true, ZEROLEVEL); 

	return true;
}

/**********************************************************************************************************
*  Function Name  :	ResumeClamScan
*  Description    :	Function to resume clam scanner.
*  Author Name    : Ram Shelke
*  Date           : 3rd Dec 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CClamScanner::ResumeClamScan()
{
	//OutputDebugString(L">>> CClamScanner::ResumeClamScan");

	SetEvent(m_hEventMemScanFinished);

	if (pi.hProcess != INVALID_HANDLE_VALUE)
	{
		ResumeThread(pi.hThread);
	}

	m_bIsClamSuspended = false;
	
	AddLogEntry(L">>> WardwizSCANNER Resumed", 0, 0, true, ZEROLEVEL);

	return true;
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
void CClamScanner::EnumerateProcesses()
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

				m_dwTotalMemScanFileCount++;

				AddLogEntry(L">>> %s :%s", L"FilePath", csProcessPath, true, ZEROLEVEL); //Varada Ikhar, Date:24/01/2015, Adding log entry of file path at zero level.
				SetScanStatus(csProcessPath);

				//Varada Ikhar, Date: 18/02/2015, Issue: Database needs to be updated. Database not valid.
				if (m_bFailedToLoadSignature)
				{
					AddLogEntry(L"%s", L"### Failed to Load Wardwiz Signature DataBase", L"", true, SECONDLEVEL);
					break;
				}

				if (!m_bIsClamLoaded)
				{
					//Kept slow as because clam scanner taking time to load.
					Sleep(100);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::EnumerateProcesses", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	UnLoadSignatureDatabase
*  Description    :	Function to Unload Signature database.
*  Author Name    : Neha Gharge
*  Date           : 27 April 2015
*  SR_NO		  :
**********************************************************************************************************/
void CClamScanner::MemoryScanFinished()
{
	AddLogEntry(L">>> Memory Scan Finished", 0, 0, true, FIRSTLEVEL);

	try
	{
		if (!SendMessage2UI(MEMSCANFINISHED))
		{
			AddLogEntry(L"### Failed to Send MEMSCANFINISHED Message in CWardwizScanner::MemoryScanFinished", 0, 0, true, SECONDLEVEL);
		}

		m_bMemScanCompleted = true;

		CString csScanStarted = L"";
		switch (m_eScanType)
		{
		case QUICKSCAN:
			csScanStarted = m_objwardwizLangManager.GetString(L"IDS_STATUS_QSCANNER_LAUNCH");
			break;
		case FULLSCAN:
			csScanStarted = m_objwardwizLangManager.GetString(L"IDS_STATUS_FSCANNER_LAUNCH");
			break;
		case CUSTOMSCAN:
			csScanStarted = m_objwardwizLangManager.GetString(L"IDS_STATUS_CSCANNER_LAUNCH");
			break;
		default:
			csScanStarted = m_objwardwizLangManager.GetString(L"IDS_STATUS_SCANNER_LAUNCH");
			break;
		}
		
		//Here it is required to flush Shared memory at last.
		SetScanStatus(csScanStarted);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CClamScanner::MemoryScanFinished", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : ReInitializeVariables
Description    : Function to re-initialize varibles.
SR.NO		   : 
Author Name    : Ram Shelke
Date           : 3 Dec 2015
***********************************************************************************************/
void CClamScanner::ReInitializeVariables()
{
	m_dwTotalMemScanFileCount = 0;
	m_dwTotalFileCount = 0;
}

/***************************************************************************
Function Name  : IsDriveHaveRequiredSpace
Description    : to check whether there is enough space to take a backup
percentage.
Author Name    : Neha Gharge
SR_NO		   :
Date           : 3rd Feb 2016
****************************************************************************/
bool CClamScanner::IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize)
{
	bool bReturn = false;
	bool isbSpaceAvailable = false;
	try
	{

		DWORD64 TotalNumberOfFreeBytes;
		//csDrive.Format(L"%c:", szDrive);

		if (PathFileExists(csDrive))
		{
			if (!GetDiskFreeSpaceEx((LPCWSTR)csDrive, &m_uliFreeBytesAvailable, &m_uliTotalNumberOfBytes, &m_uliTotalNumberOfFreeBytes))
			{
				isbSpaceAvailable = false;
				bReturn = false;
				AddLogEntry(L"### Failed in  GetDiskFreeSpaceEx", 0, 0, true, SECONDLEVEL);
			}

			TotalNumberOfFreeBytes = m_uliTotalNumberOfFreeBytes.QuadPart;
			TCHAR szFilePath[255] = { 0 };
			DWORD64 dwfileSize = 0;
			dwSetupFileSize = (dwSetupFileSize * iSpaceRatio) / (1024 * 1024);
			TotalNumberOfFreeBytes = TotalNumberOfFreeBytes / (1024 * 1024);
			if (dwSetupFileSize < TotalNumberOfFreeBytes)
			{
				bReturn = true;
			}
			else
			{
				bReturn = false;
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CClScanner::IsDriveHaveRequiredSpace", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************
Function Name  : CheckForDiscSpaceAvail
Description    : to check whether there is enough space to take a backup
percentage.
Author Name    : Neha Gharge
SR_NO		   :
Date           : 3rd Feb 2016
****************************************************************************/
DWORD CClamScanner::CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath)
{
	DWORD dwRet = 0x00;
	CString csDrivePath;
	try
	{
		csDrivePath = csQuaratineFolderPath;
		int iPos = csDrivePath.Find(L"\\");
		if (iPos != -1)
		{
			csDrivePath.Truncate(iPos);
		}
		HANDLE hFile = CreateFile(csThreatPath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### CClamScanner::Error in opening existing file %s for finding a size of path file", csThreatPath, 0, true, SECONDLEVEL);
		}

		DWORD dwFileSize = GetFileSize(hFile, NULL);

		CloseHandle(hFile);

		if (!IsDriveHaveRequiredSpace(csDrivePath, 1, dwFileSize))
		{
			AddLogEntry(L"### Low space to store the back up of file %s", csThreatPath, 0, true, SECONDLEVEL);
			dwRet = 0x01;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CClamScanner::CheckForDiscSpaceAvail for file %s", csThreatPath, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************
Function Name  : ScanFile
Description    : Function to scan single file using single loaded Database from service and return as per result of scan
Author Name    : Ram Shelke
SR_NO		   :
Date           : 3rd Feb 2016
****************************************************************************/
bool CClamScanner::ScanFile(LPTSTR lpszFilePath, LPTSTR szVirusName, DWORD& dwISpyID, DWORD& dwSignatureFailedToLoad, bool bRescan)
{
	return m_objISpyScanner.ScanFile(lpszFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, bRescan);
}
