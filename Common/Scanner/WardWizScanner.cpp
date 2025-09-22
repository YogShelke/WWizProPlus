/**********************************************************************************************************                     
	  Program Name          : WardWizScanner.cpp
	  Description           : Implementation of WardWiz Scanner functionality 
	  Author Name			: Neha Gharge                                                                                           
	  Date Of Creation      : 4 Dec 2014
	  Version No            : 1.9.0.0
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Neha Gharge         Created Wrapper class for WardWiz functionality implementation
***********************************************************************************************************/
#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif

#include <Psapi.h>
#include "WardWizScanner.h"
#include "ISpyCommunicator.h"
#include "Enumprocess.h"
#include "ITinRegWrapper.h"
#include "ISpyCriticalSection.h"
#include "WardWizDatabaseInterface.h"
#include "WWizCRC64.h"

//#define DETECTEDSTATUS	L"Detected"
//#define DELETEDSTATUS	L"Quarantined"
//#define FILEREPAIRED	L"Repaired"WardwizReports
//#define NOTHREATSFOUND  L"No threat(s) found"
//#define STATUSFAILED 	L"Failed"

BOOL					g_isScanning;
char*				g_strDatabaseFile = ".\\VBALLREPORTS.DB";

#define		BAIL_OUT(code) { g_isScanning = FALSE; return code; }
#define		WRDWIZBOOTRECOVERDB		L"VBBOOTRECOVER.DB"
#define		WRDSCANSESSDETAILDB		L"VBSCANSESSDETAIL.DB"
#define		WRDWIZSCANDETAILSDB		L"VBSCANDETAILS.DB"

CString					g_csPreviousPath = L"";

//Thread initialization here
DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam);
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam ) ;

HANDLE g_hThread_ScanCount = NULL ;
HANDLE g_hWardWizAVThread = NULL;


/***************************************************************************
  Function Name  : CWardWizScanner(void)
  Description    : Constructor
  Author Name    : Neha Gharge.
  Date           : 4 Dec 2014
  SR_NO			 : 
****************************************************************************/
CWardWizScanner::CWardWizScanner(void):
	 m_dwTotalScanPathCount(1)
    , m_bSendScanMessage(true)
	, m_objServ(FILESYSTEMSCAN)
	, m_objServViruEntry(VIRUSFOUNDENTRY)
	, m_iThreatsFoundCount(0)
	, m_ptrISpyDBManipulation(NULL)
	, m_dwTotalFileCount(0)
	, m_bManualStop(false)
	, m_bRescan(false)
	, m_objCom(UI_SERVER, true)
	, m_hPsApiDLL(NULL)
	, m_bFileFailedToDelete(false)
	, m_dwFailedDeleteFileCount(0)
	, m_bISOnlyMemScan(false)
	, m_dwAutoQuarOption(0)
{
	m_objServ.CreateServerMemoryMappedFile();
	m_objServViruEntry.CreateServerMemoryMappedFile();

	m_pbyEncDecKey = NULL;
	m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE,sizeof(unsigned char)); 

	EnumProcessModulesWWizEx = NULL; 
}

/***************************************************************************
  Function Name  : ~CWardWizScanner(void)
  Description    : Destructor
  Author Name    : Neha Gharge.
  Date           : 4 Dec 2014
  SR_NO			 : 
****************************************************************************/
CWardWizScanner::~CWardWizScanner(void)
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

/***************************************************************************
  Function Name  : GetTotalScanFilesCount
  Description    : Get total files count in case of fullscan and custom scan
  Author Name    : Neha Gharge.
  Date           : 4 Dec 2015
  SR_NO			 : 
****************************************************************************/
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam )
{
	CWardWizScanner *pThis = (CWardWizScanner*)lpParam ;
	if(!pThis)
		return 1 ;

	pThis->m_iTotalFileCount = 0x00 ;

	int	iIndex = 0x00 ;
	iIndex = (int)pThis->m_csaAllScanPaths.GetCount() ;
	if( !iIndex )
		return 2 ;

	for(int i = 0; i < iIndex ; i++)
	{
		pThis->EnumFolder( pThis->m_csaAllScanPaths.GetAt(i) ) ;
	}

	if( pThis->m_iTotalFileCount )
	{
		pThis->m_ScanCount = true ;
	}
	
	pThis->SendTotalFileCount();

	return 0 ;
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
		CWardWizScanner *pThis = (CWardWizScanner*)lpvThreadParam;
		if (!pThis)
			return 0;

		int	iIndex = 0x00;

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
		AddLogEntry(csStartScan, 0, 0, true, SECONDLEVEL); //Varada Ikhar, Date:24/01/2015, Adding a log entry.

		if (pThis->m_eScanType == QUICKSCAN)
		{
			pThis->EnumerateProcesses();

		}
		else
		{
			iIndex = (int)pThis->m_csaAllScanPaths.GetCount();
			if (!iIndex)
				return 2;

			for (int i = 0; i < iIndex; i++)
			{
				pThis->EnumFolderForScanning(pThis->m_csaAllScanPaths.GetAt(i));
				//Varada Ikhar, Date: 18/02/2015, Issue: Database needs to be updated.Database not valid.
				if (pThis->m_bFailedToLoadSignature)
				{
					break;
				}
			}
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

		//issue no 2 ,35 Neha Gharge 19-12-2014
		//Varada Ikhar, Date:12-02-2015, 
		//Issue Observed:If quick scan is aborted, 2 entries gets added in reports, one for scan complete and other for scan aborted.
		//pThis->m_bManualStop condition added.
		//if (pThis->m_iThreatsFoundCount == 0 && !pThis->m_bManualStop)
		//{
		//	pThis->AddEntriesInReportsDB(pThis->m_eScanType, L"NA", L"NA", pThis->m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND"));
		//}
		pThis->ShutdownScan();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizSCANNER::EnumerateThread",0,0,true,SECONDLEVEL);
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
void CWardWizScanner::EnumFolderForScanning(LPCTSTR pstr)
{
	try
	{
		//sanity check
		if (!pstr)
			return;

		CFileFind finder;
		//Issue: 1245: 	Last Scan Type reflecting "None" after completing the custom scan.
		//Resolved By: Nitin Kolapkar
		//Single file was not getting scanned in case of WardWiz scanner
		DWORD	dwAttributes = 0;
		// build a string with wildcards
		CString strWildcard(pstr);

		bool bIsSubFolderExcluded = false;
		if (m_objExcludeFilesFolders.ISExcludedPath((LPTSTR)pstr, bIsSubFolderExcluded))
		{
			AddLogEntry(L">>> Excluded Path [%s] ", pstr, 0, true, ZEROLEVEL);
			return;
		}

		// Issue no 1274 : Users folder is not getting scanned.
		//dwAttributes = GetFileAttributes(pstr);
		//if (((FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY) == dwAttributes) || (FILE_ATTRIBUTE_DIRECTORY == dwAttributes))
		//Issue no 1274 In full scan if u keep virus file in directory its not getting detected.
		BOOL bRet = PathIsDirectory(strWildcard);
		if (bRet == FILE_ATTRIBUTE_DIRECTORY)
		{
			//strWildcard += _T("\\*.*");
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

			if (m_bManualStop)
			{
				return;
			}
			ScanForSingleFile(pstr);
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
				if(csFilePath.Trim().GetLength() > 0)
				{
					if(PathFileExists(csFilePath))
					{		
						if (m_bManualStop)
						{
							break;
						}
						ScanForSingleFile(csFilePath);
					}
				}
			}
			//Varada Ikhar, Date: 18/02/2015, Issue: Database needs to be updated.Database not valid.
			if(m_bFailedToLoadSignature)
			{
				AddLogEntry(L"%s",L"### Failed to Load Wardwiz Signature DataBase",L"",true,SECONDLEVEL);
				break;
			}
			Sleep(10);
		}
		finder.Close();
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in EnumerateThread::EnumFolderForScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	EnumFolder                                                     
*  Description    :	Enumerate each folders of system and calculate total files count.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CWardWizScanner::EnumFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");

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
		AddLogEntry(L"### Exception in CWardwizScanner::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************               
*  Function Name  :	ScanForSingleFile                                                     
*  Description    :	Scan each single file .
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CWardWizScanner::ScanForSingleFile(CString csFilePath)
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
		DWORD dwAction = 0x00;
		CString csCurrentFile(L"");
		CString csStatus(L"");
		
		if(csFilePath.Trim().GetLength() > 0)
		{
			if(g_csPreviousPath != csFilePath)
			{
				if(PathFileExists(csFilePath))
				{
					DWORD dwISpyID = 0;
					TCHAR szVirusName[MAX_PATH] = {0};
					DWORD dwSignatureFailedToLoad = 0;

					//Added by Vilas on 26 and 28 March 2015 to avoid rescan of Repair and Recover files present in .ini
					if ( (CheckFileIsInRepairRebootIni(csFilePath)) ||
						(CheckFileIsInRecoverIni(csFilePath)) || (CheckFileIsInDeleteIni(csFilePath))
						)

						return;

					//Check here is file is excluded?
					bool bIsSubFolderExcluded = false;
					CString csFileExt;
					bool bFlag = false;
					int iExtRevCount = csFilePath.ReverseFind('.');
					if (iExtRevCount > 0)
					{
						CString csFileExt = csFilePath.Right((csFilePath.GetLength() - iExtRevCount));
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

					DWORD dwRet = ScanFile(csFilePath.GetBuffer(), szVirusName, dwISpyID, m_eScanType, dwAction,dwSignatureFailedToLoad, m_bRescan);
					if (dwRet != 0x00)
					{
						if(dwISpyID >= 0)
						{
							csVirusName = szVirusName;
							dwISpywareID = dwISpyID;
							//issue fix : 1137  When we do custom scan using No clam setup the threats are detected while "launching" or "starting" the scanner.
							csStatus = csFilePath;
							//csStatus = csFilePath + L": " + csVirusName + L" FOUND";
							bVirusFound = true;
						}
					}

					if (!m_bISOnlyMemScan)
						m_dwTotalFileCount++; //File scanned count

					//Varada Ikhar, Date: 18/02/2015, Issue: Database needs to be updated.Database not valid.
					if(dwSignatureFailedToLoad != 0)
					{
						m_bFailedToLoadSignature = true;
					}
				}
			}
			g_csPreviousPath = csFilePath;
		}

		//virus found 
		if(bVirusFound)
		{
			bSetStatus = true;
			//csStatus = csFilePath;
			if (HandleVirusFoundEntries(csVirusName, csFilePath, dwAction, dwISpywareID))
			{
				m_iThreatsFoundCount++;
				//AddEntriesInReportsDB(m_eScanType, csVirusName, csFilePath, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"));
			}
		}
		else
		{
			bSetStatus = true;
			csStatus = csFilePath;
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

				AddLogEntry(L">>> %s :%s", L"FilePath", csStatus, true, ZEROLEVEL); //Varada Ikhar, Date:24/01/2015, Adding log entry of file path at zero level.
				SetScanStatus(csStatus);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in void CWardwizScanner::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	StartScanning                                                     
*  Description    :	Start scanning with parameter given scan type and drivers and folder path .
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::StartScanning(SCANTYPE eScanType, CStringArray &csaAllScanPaths)
{
	try
	{
		m_ScanCount = false ;
		m_iThreatsFoundCount = 0;
		m_dwTotalFileCount = 0;
		m_bManualStop = false;
		//Varada Ikhar, Date: 18/02/2015, Issue: Database needs to be updated.Database not valid.
		m_bFailedToLoadSignature = false;
		m_dwFailedDeleteFileCount = 0;
		m_bFileFailedToDelete = false;


		if (g_isScanning) return false;
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

		//Ram: LoadSignature database here if not loaded.
		DWORD dwSigCount = 0x00;
		if (LoadSignatureDatabase(dwSigCount) != 0x00)
		{
			AddLogEntry(L"### Failed to Load SignatureDatabase", 0, 0, true, SECONDLEVEL);
		}

		//Load Reports.DB
		LoadReportsDBFile();
		LoadRecoversDBFile();

		//if(m_csaAllScanPaths.GetCount() == 0)
		//{
		//	AddEntriesInReportsDB(m_eScanType, L"NA", L"NA", m_objwardwizLangManager.GetString(L"IDS_MSG_NO_FILES_FOR_SCAN"));
		//}

		//ReInitializeBeforeStartScan();

		SetScanStatus(m_objwardwizLangManager.GetString(L"IDS_STATUS_SCANNER_LAUNCH"));
		AddLogEntry(L">>> Launching Scanner", 0, 0, true, ZEROLEVEL);

		m_iTotalFileCount = 0;
		if( m_csaAllScanPaths.GetAt(0).CompareNoCase( L"QUICKSCAN") == 0 )
		{
			m_eScanType = QUICKSCAN;
			GetModuleCount( ) ;
		}
		else
		{
			g_hThread_ScanCount = ::CreateThread(NULL, 0, GetTotalScanFilesCount, (LPVOID) this, 0, NULL);
			Sleep( 1000 ) ;
		}

		g_isScanning = TRUE;
		DWORD m_dwThreadId = 0;
		g_hWardWizAVThread = ::CreateThread(NULL , 0 ,EnumerateThread , (LPVOID) this, 0, &m_dwThreadId);
		Sleep(10 );
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::StartScanning", 0, 0, true, SECONDLEVEL);
	}
	return true;	
}


/**********************************************************************************************************                     
*  Function Name  :	SetScanStatus                                                     
*  Description    :	Set every status of scanning on UI
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::SetScanStatus(CString csStatus)
{
	CISpyCriticalSection  objcriticalSection;
	objcriticalSection.Lock();

	ITIN_MEMMAP_DATA iTinMemMap = {0};
	iTinMemMap.iMessageInfo = SETSCANSTATUS;
	iTinMemMap.dwFirstValue = m_dwTotalFileCount;
	iTinMemMap.dwSecondValue = m_iThreatsFoundCount;
	_tcscpy_s(iTinMemMap.szFirstParam, sizeof(iTinMemMap.szFirstParam), csStatus);
	m_objServ.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
	
	//Ram: No need it slows down the scanner
	//Sleep(10);

	objcriticalSection.Unlock();

	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	GetModuleCount                                                     
*  Description    :	Give total modules of proesses in the case of quick scan
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CWardWizScanner::GetModuleCount( )
{
	DWORD	dwPID[0x100] = {0} ;
	DWORD	dwBytesRet = 0x00, dwProcIndex = 0x00 ;
	
	EnumProcesses(dwPID, 0x400, &dwBytesRet ) ;
	dwBytesRet = dwBytesRet/sizeof(DWORD) ;

	//First Set total number of files count.
	m_iTotalFileCount = 0;
	for( dwProcIndex = 0; dwProcIndex < dwBytesRet; dwProcIndex++ )
	{
		HMODULE		hMods[1024] = {0} ;
		HANDLE		hProcess = NULL ;

		DWORD		dwModules = 0x00 ;

		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
								FALSE, dwPID[dwProcIndex] ) ;
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
		
		//EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &dwModules, LIST_MODULES_ALL) ;
		//EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules) ;

		for ( DWORD iModIndex = 0; iModIndex < (dwModules / sizeof(HMODULE)); iModIndex++ )
		{
			TCHAR szModulePath[MAX_PATH * 2] = {0};
			GetModuleFileNameEx( hProcess, hMods[iModIndex], szModulePath, MAX_PATH * 2);

			if(!IsDuplicateModule( szModulePath, sizeof(szModulePath)/sizeof(TCHAR) ))
			{
				m_iTotalFileCount++;
				_tcsupr_s(szModulePath, sizeof(szModulePath)/sizeof(TCHAR) );
				m_csaModuleList.AddHead(szModulePath);
			}
		}
		CloseHandle( hProcess ) ;
	}

	//Varada Ikhar, Date:04/02/2015, Version: 1.8.3.8
	//Issue:In quick scan, scanning completed pop-up comes at 98%.
	//m_iTotalFileCount += 6; //Need to increase this count because, when we launch WRDWIZSCANNER & LILCLAMAV.DLL 
							//the files count get increase by 2 and there will be percentage issue.
	
	if( m_iTotalFileCount )
	{
		m_ScanCount = true ;
	}

	SendTotalFileCount();
}

/**********************************************************************************************************                     
*  Function Name  :	StopScan                                                     
*  Description    :	Stop scan
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::StopScan()
{
	if (!g_isScanning) {return true;}

	m_bManualStop = true;
	//Varada Ikhar, Date: 18/04/2015
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
	//Varada Ikhar, Date:11-03-2015
	//Issue: In Quick, Full & custom scan->start scan & abort->In log file (comsrv) entry is reflecting as 'Scanning finished' instead of 'scanning aborted'
	if (m_bManualStop)
	{
		//Varada Ikhar, Date: 18/04/2015
		// Issue : 0000128 :In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
		AddLogEntry(csScanningAborted, 0, 0, true, SECONDLEVEL);
		CString csStatus;
		csStatus.Format(L">>> Total File Scanned: %d, Threats Found: %d", m_dwTotalFileCount, m_iThreatsFoundCount);
		AddLogEntry(L"---------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
		AddLogEntry(csStatus, 0, 0, true, SECONDLEVEL);
		AddLogEntry(L"---------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	}

	//Issue Resolved : 0001783
	//Unlock here 
	m_csScanFile.Unlock();

	if(g_hThread_ScanCount != NULL)
	{
		SuspendThread(g_hThread_ScanCount);
		TerminateThread(g_hThread_ScanCount, 0);
		g_hThread_ScanCount = NULL;
	}

	Sleep(500);

	if(g_hWardWizAVThread != NULL)
	{
		SuspendThread(g_hWardWizAVThread);
		TerminateThread(g_hWardWizAVThread, 0);
		g_hWardWizAVThread = NULL;
	}
	Sleep(500);
	
	//scanning aborted
	//AddEntriesInReportsDB(m_eScanType, L"NA", L"NA", m_objwardwizLangManager.GetString(L" IDS_STATUS_SCAN_ABORTED"));

	g_isScanning = FALSE;
	ShutdownScan();
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	PauseScan                                                     
*  Description    :	Pause scan
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::PauseScan()
{
	AddLogEntry(L">>> Scanning Paused", 0, 0, true, ZEROLEVEL);
	if (!g_isScanning) return false;

	if(g_hWardWizAVThread!= NULL)
	{
		::SuspendThread(g_hWardWizAVThread);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ResumeScan                                                     
*  Description    :	Resume scan
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::ResumeScan()
{
	AddLogEntry(L">>> Scanning Resumed", 0, 0, true, ZEROLEVEL);
	if (!g_isScanning) return false;

	if(g_hWardWizAVThread!= NULL)
	{
		::ResumeThread(g_hWardWizAVThread);
	}
	return true;
}

/**********************************************************************************************************              
*  Function Name  :	QuarantineEntry                                                     
*  Description    :	if ISPYID =0 , wardwiz scanner delete that file
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::QuarantineEntry(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath)
{
	AddLogEntry(L">>> Quarantine Entry [%s]: %s", csVirusName, csQurFilePaths, true, SECONDLEVEL);

	bool bReturn = false;
	try
	{
		m_bFileFailedToDelete = false;
		
		if (!MoveFile2TempRestartDelete(csQurFilePaths))
		{
			if (!DeleteFile(csQurFilePaths))
			{
				return bReturn;
			}
		}

		bReturn = true;
		m_bFileFailedToDelete = true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::QuarantineFile", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************               
*  Function Name  :	GetVirusNameAndPath                                                     
*  Description    :	Get virus name and path
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::GetVirusNameAndPath(CString csStatus, CString &csVirusName, CString &csVirusPath)
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
		AddLogEntry(L"### Exception in CWardwizScanner::GetVirusNameAndPath", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	SendMessage2UI                                                     
*  Description    :	Send message to UI
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::SendMessage2UI(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(UI_SERVER, true);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizScanner::SendMessage2UI", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	HandleVirusFoundEntries                                                     
*  Description    :	To display detected entry on UI..this function send information of detected filename,virus name
					to UI
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::HandleVirusFoundEntries(CString strVirusName, CString strScanFileName, DWORD &dwAction, DWORD dwSpyID)
{
	m_csHandleVirusFoundEntries.Lock();

	//if(!PathFileExists(strScanFileName))
	//{
	//	AddLogEntry(L"### Path is not exist of %s",strScanFileName, 0, true, SECONDLEVEL);
	//	return false;
	//}

	//Check here to avoid duplicate entry 
	if(CheckForDuplicates(strScanFileName))
	{
		m_csHandleVirusFoundEntries.Unlock();
		return false;
	}

	//Issue no :1299 Virus found entry was at zerolevel , it should be Secondlevel.
	AddLogEntry(L">>> Virus Found: %s, File Path: %s", strVirusName, strScanFileName, true, SECONDLEVEL);

	m_vFileDuplicates.push_back(strScanFileName);

	OutputDebugString(strScanFileName);

	/*ITIN_MEMMAP_DATA iTinMemMap = {0};
	iTinMemMap.iMessageInfo = SHOWVIRUSFOUND;
	_tcscpy(iTinMemMap.szFirstParam, strVirusName);
	_tcscpy(iTinMemMap.szSecondParam, strScanFileName);
	iTinMemMap.dwFirstValue = dwSpyID;

	m_objServViruEntry.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));*/

	//Added by Nitin K.
	//Issue: Displaying Virus found entries on UI through Pipe Communication
	//Date : 02-Feb-2015
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	szPipeData.iMessageInfo = SHOW_VIRUSENTRY;
	_tcscpy(szPipeData.szFirstParam,strVirusName);
	_tcscpy(szPipeData.szSecondParam,strScanFileName);
	szPipeData.dwValue			= 0x01;
	szPipeData.dwSecondValue = dwAction;
	(*(DWORD *)&szPipeData.byData[0]) = dwSpyID;

	if(!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		//Varada Ikhar, Date : 30/03/2015,
		//Issue : In Custom scan,if virus files are detected and without cleaning if UI is close and again if same virus files are scanned using custom scan then only one file is getting detected.
		//Description: Reason behind the issue was, Pipe is being closed (getlasterror - 232) but pipe-handle value was not invalid. Thus another call is given to try to send data.
		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizScanner::HandleVirusFoundEntries", 0, 0, true, SECONDLEVEL);
			m_csHandleVirusFoundEntries.Unlock();
			return false;
		}
	}

	//Sleep(30);

	m_csHandleVirusFoundEntries.Unlock();

	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SendTotalFileCount                                                     
*  Description    :	Sending total file cout to UI
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CWardWizScanner::SendTotalFileCount()
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = SET_TOTALFILECOUNT;
	szPipeData.dwValue = m_iTotalFileCount;

	CISpyCommunicator objCom(UI_SERVER, false);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CWardwizScanner::SendTotalFileCount", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	HandleVirusEntry                                                     
*  Description    :	When any entry comes for cleaning, wardwiz scanner take a backup store into quarantine 
					folder and keep a record into DB file
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
//bool CWardWizScanner::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName,LPCTSTR szThirdParam, DWORD dwISpyID)
//Signature changed to handle failure cases
//Modified by Vilas on 07 April 2015
DWORD CWardWizScanner::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThirdParam, DWORD dwISpyID, CString &csBackupFilePath, DWORD &dwAction)
{
	//bool bReturn = false;
	DWORD	dwRet = 0x00;

	m_csQuarentineEntries.Lock();

	try
	{
		TCHAR szAction[MAX_PATH] = { 0 };

		if ((!szThreatPath) || (!szThreatName) || (!szThirdParam))
		{
			AddLogEntry(L"### CWardwizScanner::HandleVirusEntry file name not available", 0, 0, true, SECONDLEVEL);

			dwRet = SANITYCHECKFAILED;
			m_csQuarentineEntries.Unlock();
			return dwRet;
		}

		if (!PathFileExists(szThreatPath))
		{
			// Should not give failed message on UI at any case Neha Gharge 19/3/2015
			AddLogEntry(L"### CWardwizScanner::HandleVirusEntry No file available %s", szThreatPath, 0, true, SECONDLEVEL);
			//return bReturn;

			dwRet = FILENOTEXISTS;
			m_csQuarentineEntries.Unlock();
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
				dwRet = CREATEDIRECTORYFAILED;
			}
		}

		bool bISbackupSuccess = false;

		dwAction = FILESKIPPED;

		//Check if file option is selected as skipped.
		if (m_dwAutoQuarOption == 0x00)
		{
			//Issue resolved: USB scan getting stuck in second instance
			m_csQuarentineEntries.Unlock();
			return dwRet;
		}

		TCHAR szBackupFilePath[MAX_PATH * 2] = { 0 };

		//DWORD value flag to show entry in Recover window (or) not.
		DWORD dwShowEntryStatus = 0;
		if (dwISpyID > 0 && m_dwAutoQuarOption == REPAIRE)
		{
			/*Added by Prajakta: to handle multiple instances of scanning & repair*/
			//Re-scan file before repairing
			DWORD dwRepairID = 0;
			DWORD dwFailedToLoadSignature = 0;
			m_bRescan = true;
			TCHAR szVirusName[MAX_PATH] = { 0 };
			_tcscpy(szVirusName, szThreatName);
		
			//if (m_objISpyScanner.ScanFile(szThreatPath, szVirusName, dwISpyID, dwFailedToLoadSignature, m_bRescan))
			{
				//While taking a back up of any file. we have to check disk where we take a back up 
				//is having space or not.
				if (CheckForDiscSpaceAvail(csQuaratineFolderPath, szThreatPath) == 0x01)
				{
					dwRet = LOWDISKSPACE;
					m_csQuarentineEntries.Unlock();
					return dwRet;
				}

				if (!BackUpBeforeQuarantineOrRepair(szThreatPath, szBackupFilePath))
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
					//AddRepairEntryAfterReboot(szThreatPath, szVirusName, szBackupFilePath, dwISpyID);
					dwShowEntryStatus = 1;

					//AddLogEntry(L"### Repair on reboot file: %s", szThreatPath, 0, true, SECONDLEVEL);
					//return bReturn;

					//Temporary made OFF
					dwAction = FILEREBOOTREPAIR;
					dwRet = 0x04;
				}
				else
				{
					dwAction = FILEREPAIRED;
					_tcscpy(szAction, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
				}
				//bReturn = true;
			}
			//else
			//{
			//	//Need to returned from here.
			//	m_bRescan = false;
			//	dwRet = 0x08;
			//	m_csQuarentineEntries.Unlock();
			//	return dwRet;
			//}
		}
		else if (dwISpyID == 0 || m_dwAutoQuarOption == QURENTINE)
 		{
			//While taking a back up of any file. we have to check disk where we take a back up 
			//is having space or not.
			if (CheckForDiscSpaceAvail(csQuaratineFolderPath, szThreatPath) == 0x01)
			{
				dwRet = LOWDISKSPACE;
				m_csQuarentineEntries.Unlock();
				return dwRet;
			}

			if (!BackUpBeforeQuarantineOrRepair(szThreatPath, szBackupFilePath))
			{
				AddLogEntry(L"#### Failed to take backup of %s", szThreatPath, 0, true, SECONDLEVEL);
				bISbackupSuccess = false;
				dwRet = BACKUPFILEFAILED;
			}

			bISbackupSuccess = true;
			//quarantine file
			if (!QuarantineEntry(szThreatPath, szThreatName, szBackupFilePath))
			{
				AddLogEntry(L"### Failed to Quarantine file: %s", szThreatPath, 0, true, SECONDLEVEL);
				dwShowEntryStatus = 1;
				dwRet = DELETEFILEFAILED;
				dwAction = FILEREBOOTQUARENTINE;
			}
			else
			{
				dwAction = FILEQURENTINED;
				_tcscpy(szAction, m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
			}
		}
		else
		{
			AddLogEntry(L"### Unhandled case to clean, VirusName: [%s], FilePath: [%s]", szThreatName, szThreatPath, true, SECONDLEVEL);
		}

		bool bDoRecover = true;
		//Neha Gharge right click scan 18-3-2015
		// Third parameter need to be receive which scan currently going on.
		//Issue 1190 report entry for this scans after USB scan is wrong
		if (_tcscmp(szThirdParam, L"USB Scan") == 0)
		{
			bDoRecover = false;
			//csQurStatus.Format(L"%s", szAction);
			//AddEntriesInReportsDB(USBSCAN, szThreatName, szThreatPath, szAction);
		}
		else if (_tcscmp(szThirdParam, L"RightClick Scan") == 0)
		{
			bDoRecover = false;
			//csQurStatus.Format(L"%s", szAction);
			//AddEntriesInReportsDB(CUSTOMSCAN, szThreatName, szThreatPath, szAction);
		}
		else if (_tcscmp(szThirdParam, L"Custom Scan") == 0)
		{
			//AddEntriesInReportsDB(CUSTOMSCAN, szThreatName, szThreatPath, szAction);
		}
		else if (_tcscmp(szThirdParam, L"Full Scan") == 0)
		{
			//AddEntriesInReportsDB(FULLSCAN, szThreatName, szThreatPath, szAction);
		}
		else if (_tcscmp(szThirdParam, L"Quick Scan") == 0)
		{
			//AddEntriesInReportsDB(QUICKSCAN, szThreatName, szThreatPath, szAction);
		}

		if (bISbackupSuccess)
		{
			csBackupFilePath = szBackupFilePath;
			if (bDoRecover)
			{
				LoadRecoversDBFile();

				if (!InsertRecoverEntry(szThreatPath, szBackupFilePath, szThreatName, dwShowEntryStatus))
				{
					AddLogEntry(L"### Error in InsertRecoverEntry, Path: %s | BackupPath: %s", szThreatPath, szBackupFilePath, true, SECONDLEVEL);
					dwRet = INSERTINTORECOVERFAILED;
				}

				if (!SaveInRecoverDB())
				{
					AddLogEntry(L"### Error in SaveInRecoverDB, Path: %s | BackupPath: %s", szThreatPath, szBackupFilePath, true, SECONDLEVEL);
					dwRet = SAVERECOVERDBFAILED;
				}
			}
		}
		m_bRescan = false;
	}
	catch (...)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in CWardwizScanner::HandleVirusEntry", 0, 0, true, SECONDLEVEL);
	}

	m_csQuarentineEntries.Unlock();
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	InsertData
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into 
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
void InsertData(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable entered", 0, 0, true, ZEROLEVEL);
	try
	{
		CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFile);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);

		objSqlDb.Close();

		return;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::InsertData", 0, 0, true, SECONDLEVEL);

	}
}
/**********************************************************************************************************
*  Function Name  :	GetWardwizRegistryDetails
*  Description    :	Read Scanning related options from registry
*  Author Name    : Gayatri A.
*  SR_NO		  :
*  Date           : 13 Sep 2016
**********************************************************************************************************/
bool CWardWizScanner::GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt)
{
	try
	{
		HKEY hKey;
		//take here the registry path
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		if (RegOpenKey(HKEY_LOCAL_MACHINE,csRegKeyPath, &hKey) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to open registry key in CWardwizScanner::GetWardwizRegistryDetails, Key Path %s", csRegKeyPath, 0, true, SECONDLEVEL);
			return false;
		}

		DWORD dwOptionSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;

		long ReadReg = RegQueryValueEx(hKey, L"dwQuarantineOption", NULL, &dwType, (LPBYTE)&dwQuarantineOpt, &dwOptionSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to get registry key value for Quarantine Option in CWardwizScanner::GetWardwizRegistryDetails, Key Path %s", csRegKeyPath, 0, true, SECONDLEVEL);
		}

		ReadReg = RegQueryValueEx(hKey, L"dwHeuScan", NULL, &dwType, (LPBYTE)&dwHeuScanOpt, &dwOptionSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to get registry key value for Heuristic Scan Option in CWardwizScanner::GetWardwizRegistryDetails, Key Path %s", csRegKeyPath, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::GetWardwizRegistryDetails", 0, 0, true, SECONDLEVEL);
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
CString CWardWizScanner::GetActionIDFromAction(CString csMessage)
{
	CString csLanguageID = "";
	try
	{
		CWardwizLangManager objwardwizLangManager;

		if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED")) == 0)
		{
			csLanguageID = "IDS_USB_SCANNING_ABORTED";
		}
		else if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND")) == 0)
		{
			csLanguageID = "IDS_USB_SCAN_NO_THREAT_FOUND";
		}
		else if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_DETECTED";
		}
		else if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_QUARANTINED";
		}
		else if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_REPAIRED";
		}
		else if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_NO_FILE_FOUND";
		}
		else if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
		}
		else if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
		}
		else if (csMessage.CompareNoCase(objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_FAILED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_FAILED";
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::GetActionIDFromAction, Message: %s", csMessage, 0, true, SECONDLEVEL);
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
void CWardWizScanner::AddEntriesInReportsDB(SCANTYPE eScanType, CString csThreatName, CString csFilePath, CString csAction)
{
	try
	{
		AddLogEntry(L"CWardwizScanner::AddEntriesInReportsDB entered", 0, 0, true, SECONDLEVEL);
		SYSTEMTIME		CurrTime = {0} ;
		GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
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

		//OutputDebugString(L">>> AddinReports " + csFilePath);

		CString csDateTime = L"";
		csDateTime.Format(_T("%s %s"),csDate,csTime);
		CIspyList newEntry(csDateTime,csScanType,csThreatName,csFilePath,csAction);


		if(!m_ptrISpyDBManipulation)
		{
			return;
		}

		// Get entries from registry so that, those can be included in query..
		DWORD dwQuarantineOpt = 0x00;
		DWORD dwHeuristicOpt = 0x00;
		bool  bHeuristicOpt = false;
		
		GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);

		if (dwHeuristicOpt == 0x01)
			bHeuristicOpt = true;

		//// Add entries into Database..
		//CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");

		//csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),'%s','%s','%s');"), eScanType, csThreatName, csFilePath, csAction);
		//CT2A ascii(csInsertQuery, CP_UTF8);

		////Not required to create, because on Server restart we are creating table.
		////CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFile);
		////objSqlDb.Open();
		////objSqlDb.CreateWardwizSQLiteTables();
		////objSqlDb.Close();

		//InsertData(ascii.m_psz);
		//m_ptrISpyDBManipulation->InsertEntry(newEntry, REPORTS);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::AddEntriesInReportsDB", 0, 0, true, SECONDLEVEL);
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
bool CWardWizScanner::InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus)
{
	CIspyList newEntry(szThreatPath, csDuplicateName, szThreatName, L"", L"", L"", dwShowStatus);
 
	if(!m_ptrISpyDBManipulation)
	{
		return false;
	}
	m_ptrISpyDBManipulation->InsertEntry(newEntry, RECOVER);
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SaveInReportsDB                                                     
*  Description    :	Save entry into report DB
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::SaveInReportsDB()
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
}

/**********************************************************************************************************                     
*  Function Name  :	LoadReportsDBFile                                                     
*  Description    :	Load entry into report DB
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CWardWizScanner::LoadReportsDBFile()
{
	try
	{
		if(!m_ptrISpyDBManipulation)
		{
			return;
		}
		if(!m_ptrISpyDBManipulation->LoadEntries(0x05))
		{
			AddLogEntry(L"### Failed to load entries in CWardwizScanner::LoadReportsDBFile", 0, 0, true, SECONDLEVEL);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::LoadReportsDBFile", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	SetManipulationPtr                                                     
*  Description    :	Set DB manipulation pointer.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::SetManipulationPtr(CISpyDBManipulation *pISpyDBManpt)
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

/**********************************************************************************************************                     
*  Function Name  :	BackUpBeforeQuarantineOrRepair                                                     
*  Description    :	Taking a backup before taking any action on detected files.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath, LPTSTR lpszBackupFilePath)
{
	try
	{
		if (!lpszBackupFilePath)
			return false;

		DWORD dwStatus = 0;
		CString csEncryptFilePath, csQuarantineFolderpath = L"";
		TCHAR szQuarantineFileName[MAX_PATH] = { 0 };
		UINT RetUnique = 0;

		csQuarantineFolderpath = GetQuarantineFolderPath();

		if (!PathFileExists(csOriginalThreatPath))
		{
			AddLogEntry(L"### CWardwizScanner::BackUpBeforeQuarantineOrRepair Original file not available %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
			return false;
		}

		if (!PathFileExists(csQuarantineFolderpath))
		{
			if (!CreateDirectory(csQuarantineFolderpath, NULL))
			{
				AddLogEntry(L"### CWardwizSCANNERBase::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		//Get here file hash
		TCHAR szFileHash[128] = { 0 };
		if (!GetFileHash(csOriginalThreatPath.GetBuffer(), szFileHash))
		{
			return false;
		}

		//check here if backup has is taken already.
		if (CheckIFAlreadyBackupTaken(szFileHash, lpszBackupFilePath))
		{
			return  true;
		}

		if (Encrypt_File(csOriginalThreatPath.GetBuffer(), csQuarantineFolderpath.GetBuffer(), lpszBackupFilePath, szFileHash, dwStatus) != 0x00)
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::BackUpBeforeQuarantineOrRepair, FilePath: %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	Encrypt_File                                                     
*  Description    :	Encrypt file and keep into quarantine folder as temp file.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
DWORD CWardWizScanner::Encrypt_File(TCHAR *szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus)
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00 ;
	TCHAR	szExt[16] = {0} ;
	DWORD	dwLen = 0x00 ;
	LPBYTE	lpFileData = NULL ;
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;
	
	__try
	{
		//Sanity check
		if (!szFilePath || !szQurFolderPath || !lpszFileHash || !lpszTargetFilePath)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//Check is valid paths
		if (!PathFileExists(szFilePath) || !PathFileExists(szQurFolderPath))
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

		TCHAR szTargetFilePath[MAX_FILE_PATH_LENGTH] = { 0 };
		_stprintf(szTargetFilePath, L"%s\\%s.tmp", szQurFolderPath, lpszFileHash);
		
		//copy here into output parameter
		_tcscpy(lpszTargetFilePath, szTargetFilePath);

		hFileEnc = CreateFile(lpszTargetFilePath, GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFileEnc == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in creating file %s", lpszTargetFilePath, 0, true, SECONDLEVEL);
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

/**********************************************************************************************************               
*  Function Name  :	DecryptData                                                     
*  Description    :	Encrypt/Decrypt data.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
DWORD CWardWizScanner::DecryptData( LPBYTE lpBuffer, DWORD dwSize )
{
	__try
	{
		if( IsBadWritePtr( lpBuffer, dwSize ) )
			return 1 ;

		DWORD	iIndex = 0 ;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00 || m_pbyEncDecKey == NULL)
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
		AddLogEntry(L"### Exception in CWardwizScanner::DecryptData", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

/**********************************************************************************************************               
*  Function Name  :	GetQuarantineFolderPath                                                     
*  Description    :	Get Quarantine folder path.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
CString CWardWizScanner::GetQuarantineFolderPath()
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
		AddLogEntry(L"### Exception in CWardwizScanner::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/**********************************************************************************************************                     
*  Function Name  :	SaveInRecoverDB                                                     
*  Description    :	Save all entry into recover files.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::SaveInRecoverDB()
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

/**********************************************************************************************************                     
*  Function Name  :	LoadRecoversDBFile                                                     
*  Description    :	Load all entries of recover files.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CWardWizScanner::LoadRecoversDBFile()
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
		AddLogEntry(L"### Exception in CWardwizScanner::LoadRecoversDBFile", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	RecoverFile                                                     
*  Description    :	Handle recover and delete function in recovery tab.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
//bool CWardWizScanner::RecoverFile(LPTSTR lpFilePath, LPTSTR lpBrowseFilePath,DWORD dwTypeofAction)
//Modified by Vilas on 28 March 2015
//Signature Modified to handle failure cases
DWORD CWardWizScanner::RecoverFile(LPTSTR lpFilePath, LPTSTR lpBrowseFilePath,DWORD dwTypeofAction)
{
	//bool bReturn = false;
	CString		csDuplicatePath;
	DWORD		dwRet = 0x00;

	try
	{
		LoadRecoversDBFile();

		dwRet = RecoverCurrentFile(lpFilePath, lpBrowseFilePath, dwTypeofAction);
		//if(!RecoverCurrentFile(lpFilePath,lpBrowseFilePath,dwTypeofAction))
		if (dwRet)
		{
			AddLogEntry(L"### File(s) not recovered : %s", lpFilePath, 0, true, SECONDLEVEL);
			//bReturn = false;
		}
		else
		{
			if (dwTypeofAction == 0)//0 is for Recover
			{
				//Action to be needed remove entry from recoverdb
				AddLogEntry(L">>> File(s) recovered : %s", lpFilePath, 0, true, ZEROLEVEL);

				csDuplicatePath = m_szQuarantineFilePath;

				m_ptrISpyDBManipulation->m_RecoverDBEntries.RemoveContact(lpFilePath, csDuplicatePath);
				dwRet = 0x00;
				//bReturn = true;
			}
			else if (dwTypeofAction == 1)//1 is for Deleting the entries in RecoverDB
			{
				//Action to be needed remove entry from recoverdb
				AddLogEntry(L">>> File(s) Deleted : %s", lpFilePath, 0, true, ZEROLEVEL);
				csDuplicatePath = m_szQuarantineFilePath;
				m_ptrISpyDBManipulation->m_RecoverDBEntries.RemoveContact(lpFilePath, csDuplicatePath);
				//m_objRecoverdb.RemoveContact(lpFilePath,csDuplicatePath);

				dwRet = 0x00;
				//bReturn = true;
			}
			SaveInRecoverDB();
		}

		m_objExcludeFilesFolders.ReloadExcludeDB();
	}
	catch (...)
	{
		dwRet = 0x100;
		AddLogEntry(L"### Exception in CWardwizScanner::RecoverFile", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/**********************************************************************************************************                     
*  Function Name  :	RecoverCurrentFile                                                     
*  Description    :	Recover Current file.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/

//bool CWardWizScanner::RecoverCurrentFile(CString csThreatPath , CString csBrowseFilePath,DWORD dwTypeofAction)
//Modified by Vilas on 28 March 2015
//Signature Modified to handle failure cases
DWORD CWardWizScanner::RecoverCurrentFile(CString csThreatPath, CString csBrowseFilePath, DWORD dwTypeofAction)
{
	AddLogEntry(L">>> Recovering File : %s", csThreatPath, 0, true, ZEROLEVEL);

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
						bool bDeleteBackupFile = true;
						if (CheckForDuplicateEntry(m_szOriginalThreatPath, csQuarantineFilePath.GetBuffer()))
						{
							bDeleteBackupFile = false;
						}

						dwRet = Decrypt_File(m_szQuarantineFilePath, m_szOriginalThreatPath, dwStatus, bDeleteBackupFile);
						if(dwRet != 0x00)
						{
							//AddLogEntry(L"### CWardWizScanner::RecoverCurrentFile failed to Decrypt file", 0, 0, true, SECONDLEVEL);
							//return false;

							dwRet = RecoverInUseFileIfPossible(m_szQuarantineFilePath, m_szOriginalThreatPath, dwStatus, bDeleteBackupFile);
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


	//return true;
	return dwRet;
}

/**********************************************************************************************************                     
*  Function Name  :	Decrypt_File                                                     
*  Description    :	Decrpt file.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
DWORD CWardWizScanner::Decrypt_File(TCHAR* szRecoverFilePath, TCHAR* szOriginalThreatPath, DWORD &dwStatus, bool bDeleteBackupFile)
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

		if (bDeleteBackupFile)
		{
			SetFileAttributes(szRecoverFilePath, FILE_ATTRIBUTE_NORMAL);
			if (!(DeleteFile(szRecoverFilePath)))
			{
				AddLogEntry(L"### Quarantine %s File not deleted after decrypt data.", szRecoverFilePath, 0, true, SECONDLEVEL);
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::Decrypt_File, File Path: %s, Original Path: %s", szRecoverFilePath, szOriginalThreatPath, true, SECONDLEVEL);
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

/**********************************************************************************************************                     
*  Function Name  :	CreateFullDirectoryPath                                                     
*  Description    :	If directory is not present, it will create a directory at given path.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::CreateFullDirectoryPath(wchar_t *szFullPath)
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
		AddLogEntry(L"### Exception in CWardwizScanner::CreateFullDirectoryPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckForDuplicates                                                     
*  Description    :	check for duplicate entry before inserting it into list control on UI.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::CheckForDuplicates(CString csFilePath)
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

/**********************************************************************************************************                     
*  Function Name  :	SetLastScanDatetime                                                     
*  Description    :	set last scannig date and time into registry.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CWardWizScanner::SetLastScanDatetime()
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
		GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
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
			AddLogEntry(L"### SetRegistryValueData failed in LastScandt CWardwizScanner::SetLastScandatetime", 0, 0, true, SECONDLEVEL);
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
			AddLogEntry(L"### SetRegistryValueData failed in ScanType CWardwizScanner::SetLastScandatetime", 0, 0, true, SECONDLEVEL);
		}

		if(m_iThreatsFoundCount > 0)
		{
			if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"VirusFound",
				0x01) != 0)
			{
				AddLogEntry(L"### SetRegistryValueData failed in VirusFound CWardwizScanner::SetLastScandatetime", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::SetLastScandatetime", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	ClearMemoryMapObjects                                                     
*  Description    :	Clear memory map object.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
//Clear here all memory map objects to avoid duplicates
void CWardWizScanner::ClearMemoryMapObjects()
{
	ITIN_MEMMAP_DATA iTinMemMap = {0};
	m_objServViruEntry.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
	m_objServ.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
}

//Prajakta N.
/**********************************************************************************************************                     
*  Function Name  :	CreateRandomKeyFromFile                                                     
*  Description    :	Create a random key to insert into encrypted file.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize)
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
/**********************************************************************************************************                     
*  Function Name  :	ReadKeyFromEncryptedFile                                                     
*  Description    :	Read key from encrypted file.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::ReadKeyFromEncryptedFile(HANDLE hFile)
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
/**********************************************************************************************************                     
*  Function Name  :	IsFileAlreadyEncrypted                                                     
*  Description    :	Checking whether file is already encrypted or not.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::IsFileAlreadyEncrypted(HANDLE hFile)
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

/**********************************************************************************************************                     
*  Function Name  :	ShutdownScan                                                     
*  Description    :	Shut down scanning processes.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 4 Dec 2014
*  SR_NO		  : 
**********************************************************************************************************/
bool CWardWizScanner::ShutdownScan()
{
	bool bFinishedScanning = false;
	if (m_eScanType == FULLSCAN || m_eScanType == QUICKSCAN || m_eScanType == CUSTOMSCAN || m_bManualStop ||
		m_csaAllScanPaths.GetCount() == m_dwTotalScanPathCount)
	{
		bFinishedScanning = true;
	}

	if (bFinishedScanning)
	{
		g_isScanning = FALSE;
		g_csPreviousPath = L"";

		if (!m_bManualStop)
		{
			// total files scanned count differs in AVUI and COMMSRV log files. Neha Gharge 9/2/2015
			m_dwTotalFileCount = m_iTotalFileCount;
		}

		//Varada Ikhar, Date: 18/02/2015, Issue: Database needs to be updated.Database not valid.
		if (m_bFailedToLoadSignature)
		{
			SendMessage2UI(SCAN_FINISHED_SIGNATUREFAILED); //Varada Ikhar
		}
		m_bFailedToLoadSignature = false;

		//Issue Resolved: 0001783
		//Unlock here
		m_csScanFile.Unlock();

		//issue no 2 ,35 Neha Gharge 19-12-2014
		SaveInReportsDB();
		SaveInRecoverDB();

		Sleep(1000);

		if (!SendMessage2UI(SCAN_FINISHED))
		{
			AddLogEntry(L"### Failed to send Scanning finished message", 0, 0, true, SECONDLEVEL);
		}

		//if scanning is not started then not need to set last scan date and time.
		if (!m_bSendScanMessage)
		{
			SetLastScanDatetime();
		}
		//pThis->ClearMemoryMapObjects();

		//Varada Ikhar, Date: 18/04/2015
		// Issue : 0000128 :In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
		CString csScanningFinished = L"";
		switch (m_eScanType)
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

		//Varada Ikhar, Date:11-03-2015
		//Issue: In Quick, Full & custom scan->start scan & abort->In log file (comsrv) entry is reflecting as 'Scanning finished' instead of 'scanning aborted'
		if (!m_bManualStop)
		{
			//Varada Ikhar, Date: 18/04/2015
			// Issue : 0000128 :In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
			AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
			//Varada Ikhar, Date: 24/01/2015, Adding a log entry at second level.
			CString csStatus;
			csStatus.Format(L">>> Total File Scanned: %d, Threats Found: %d", m_dwTotalFileCount, m_iThreatsFoundCount);
			AddLogEntry(L"---------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
			AddLogEntry(csStatus, 0, 0, true, SECONDLEVEL);
			AddLogEntry(L"---------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
		}
	}
	return true;
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
void CWardWizScanner::EnumerateProcesses()
{
	try
	{
		CString csProcessPath(L"");
		CString csToken(L"");
		CString csTokenSytemRoot(L"");

		TCHAR szSystemDirectory[MAX_PATH]={0};
		bool bSystemDirectory = false;
		bool bReplaceWindowPath = false;

		POSITION pos = m_csaModuleList.GetHeadPosition();
		while( pos != NULL )
		{
			csProcessPath = m_csaModuleList.GetNext(pos);

			int iPos = 0;
			int SlashPos = 0;

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

			if(PathFileExists(csProcessPath))
			{
				if (m_bManualStop)
				{
					break;
				}
				ScanForSingleFile(csProcessPath);
				//Varada Ikhar, Date: 18/02/2015, Issue: Database needs to be updated. Database not valid.
				if(m_bFailedToLoadSignature)
				{
					AddLogEntry(L"%s",L"### Failed to Load Wardwiz Signature DataBase",L"",true,SECONDLEVEL);
					break;
				}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::EnumerateProcesses", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : IsDuplicateModule
  Description    : Function to find duplicates modules to avoid multiple scanning.
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 24th Jan 2015
****************************************************************************/
bool CWardWizScanner::IsDuplicateModule( LPTSTR szModulePath, DWORD dwSize )
{
	bool bReturn = false;
	try
	{
		_tcsupr_s(szModulePath, dwSize);
		if(m_csaModuleList.Find(szModulePath, 0))
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

/***************************************************************************
  Function Name  : LoadPSAPILibrary
  Description    : Load PSAPI.DLL.
				   For Issue: In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 04th Feb 2015
****************************************************************************/
void CWardWizScanner::LoadPSAPILibrary()
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
		AddLogEntry(L"### Exception in CWardwizScanner::LoadPSAPILibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : AddDeleteFailEntryToINI
Description    : Add entry of files whose failed in delete.
Author Name    : Neha Gharge
S.R. No        :
Modified Date  : 04th March 2015
****************************************************************************/
bool CWardWizScanner::AddDeleteFailEntryToINI(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath)
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
	bool  bDupFlag = false;

	try
	{

		CString csDeleteFailedINIPath(L"");
		CString csQuarantineFolderPath = GetQuarantineFolderPath();

		csDeleteFailedINIPath.Format(L"%s\\WRDWIZDELETEFAIL.INI", csQuarantineFolderPath);

		if (!PathFileExists(csQuarantineFolderPath))
		{
			if (!CreateDirectory(csQuarantineFolderPath, NULL))
			{
				AddLogEntry(L"### CWardwizScanner::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
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
				if (!TokenizationOfParameterForrecover(szDuplicateString, szFileName, _countof(szFileName), szQuarantineFileName, _countof(szQuarantineFileName), szVirusName, _countof(szVirusName)))
				{
					AddLogEntry(L"### TokenizationOfParameterForrecover is not tokenize properly", 0, 0, true, FIRSTLEVEL);
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
					AddLogEntry(L"### File Entry already present in deletefiles.ini", 0, 0, true, FIRSTLEVEL);
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
				if (!MakeConcatStringforrecover(csQurFilePaths, csBackupFilePath, csVirusName, szConcatnateDeleteString, _countof(szConcatnateDeleteString)))
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
			if (!MakeConcatStringforrecover(csQurFilePaths, csBackupFilePath, csVirusName, szConcatnateDeleteString, _countof(szConcatnateDeleteString)))
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
bool CWardWizScanner::TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName)
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
			wcscpy_s(szApplicationName, (dwSizeofApplicationName-1), szValueApplicationName);
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		}

		TCHAR	szAttemptCnt[16] = { 0 };

		if (pToken)
			wcscpy_s(szAttemptCnt, _countof(szAttemptCnt),pToken);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::TokenizeIniData", 0, 0, true, SECONDLEVEL);
		return false;
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
void CWardWizScanner::AddRepairEntryAfterReboot(LPCTSTR szThreatPath, LPCTSTR szThreatName, CString csDuplicateName, DWORD dwISpyID)
{

	try
	{

		TCHAR	szData[2048] = { 0 };
		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRepairIniPath) )
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
			if (memcmp(szValueData, szData, wcslen(szData)*sizeof(TCHAR)) == 0 )
			{
				bFound = false;
				break;
			}
		}

		if (bFound)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwRepairCount+1);

			wcscat_s(szData, _countof(szData), L"|");
			wcscat_s(szData, _countof(szData), csDuplicateName);
			WritePrivateProfileString(L"Files", szValueName, szData, szRepairIniPath);

			WritePrivateProfileString(L"Count", L"Count", szValueName, szRepairIniPath);
		}

	}
	catch ( ... )
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
bool CWardWizScanner::RescanAndRepairFile(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szDupName, DWORD dwISpyID)
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

			if (!UpdateRecoverEntry(szThreatPath, szDupName, szVirusName, 0))
			{
				AddLogEntry(L"### Failed to update UpdateRecoverEntry in CWardwizScanner::RescanAndRepairFile");
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
bool CWardWizScanner::GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath)
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
		AddLogEntry(L"### Exception in CWardwizScanner::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInRepairRebootIni
*  Description    :	Get Quarantine folder path.
*  Author Name    : Vilas Suvarnakar
*  Date           : 24 March 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizScanner::CheckFileIsInRepairRebootIni(CString csFilePath)
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

/**********************************************************************************************************
*  Function Name  :	UpdateRecoverEntry
*  Description    :	Function to update Recover entry.
*  Author Name    : Ram
*  Date           : 24 March 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizScanner::UpdateRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowEntryStatus)
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
Function Name  : RecoverInUseFileIfPossible
Description    : Recover file if it is in use
SR.NO		   :
Author Name    : Vilas Suvarnakar
Modified Date  : 26 March 2015
***********************************************************************************************/
DWORD CWardWizScanner::RecoverInUseFileIfPossible(LPCTSTR szRecoverFilePath, LPCTSTR szOriginalThreatPath, DWORD &dwStatus, bool bDeleteBackupFile)
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

		dwRet = Decrypt_File(m_szQuarantineFilePath, m_szOriginalThreatPath, dwStatus, bDeleteBackupFile);
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
		AddLogEntry(L"### Exception in CWardwizScanner::RecoverInUseFileIfPossible", 0, 0, true, SECONDLEVEL);
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
DWORD CWardWizScanner::RenameFile(LPCTSTR szOriginalThreatPath, LPTSTR lpszRenamedFilePath)
{

	DWORD	dwRet = 0x00;

	__try
	{

		SetFileAttributes(szOriginalThreatPath, FILE_ATTRIBUTE_NORMAL);
		swprintf_s(lpszRenamedFilePath, 511, L"%s.WW", szOriginalThreatPath);

		if (PathFileExists(lpszRenamedFilePath))
		{
			SetFileAttributes(lpszRenamedFilePath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(lpszRenamedFilePath);
		}

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
		AddLogEntry(L"### Exception in CWardwizScanner::RenameFile", 0, 0, true, SECONDLEVEL);
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
void CWardWizScanner::AddRenamedFileEntryInIni(LPCTSTR lpszInUseFilePath)
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
		AddLogEntry(L"### Exception in CWardwizScanner::AddRenamedFileEntryInIni", 0, 0, true, SECONDLEVEL);
	}
}



/***********************************************************************************************
Function Name  : Checking file entry present in Recover ini (WWRecover.ini)
Description    : Parses WWRecover.ini and not sends to scan if file is found.
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 27 March 2015
***********************************************************************************************/
bool CWardWizScanner::CheckFileIsInRecoverIni(CString csFilePath)
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
bool CWardWizScanner::MakeConcatStringforrecover(CString csFilename, CString csQuarantinePath, CString csVirusName, TCHAR * szConcateDeleteString, DWORD dwsizeofConcatenateString)
{
	try
	{
		if (szConcateDeleteString == NULL)
			return false;

		CString csConcateDeleteString(L"");
		csConcateDeleteString.Format(L"%s|%s|%s",csFilename,csQuarantinePath,csVirusName);
		_tcscpy_s(szConcateDeleteString, (dwsizeofConcatenateString-1),csConcateDeleteString);

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
bool CWardWizScanner::TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName)
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
			wcscpy_s(szQuarantinepath, (dwsizeofquarantinefileName-1), pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szVirusName, (dwsizeofVirusName -1), pToken);

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
bool CWardWizScanner::CheckFileIsInDeleteIni(CString csQurFilePaths)
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

/**********************************************************************************************************
*  Function Name  :	LoadSignatureDatabase
*  Description    :	Function to load Signature database.
*  Author Name    : Neha Gharge
*  Date           : 27 April 2015
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizScanner::LoadSignatureDatabase(DWORD &dwSigCount)
{
	DWORD dwRet = 0x00;
	__try
	{
		//Get days left.
		GetDaysLeft();

		dwRet = m_objISpyScanner.LoadSignatureDatabase(dwSigCount);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CClamScanner::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	UnLoadSignatureDatabase
*  Description    :	Function to Unload Signature database.
*  Author Name    : Neha Gharge
*  Date           : 27 April 2015
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizScanner::UnLoadSignatureDatabase()
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.UnLoadSignatureDatabase();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CClamScanner::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}


/***************************************************************************
Function Name  : IsDriveHaveRequiredSpace
Description    : to check whether there is enough space to take a backup
percentage.
Author Name    : Neha Gharge
SR_NO		   :
Date           : 3rd Feb 2016
****************************************************************************/
bool CWardWizScanner::IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize)
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
		AddLogEntry(L"### Exception in CWardwizScanner::IsDriveHaveRequiredSpace", 0, 0, true, SECONDLEVEL);
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
DWORD CWardWizScanner::CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath)
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
			AddLogEntry(L"### CWardwizScanner::Error in opening existing file %s for finding a size of path file", csThreatPath, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScanner::CheckForDiscSpaceAvail for file %s", csThreatPath, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************
Function Name  : ScanFile
Description    : Function to scan single file using single loaded Database from service and return as per result of scan
Author Name    : Ram Shelke
SR_NO		   :
Date           : 25 Mar 2016
****************************************************************************/
DWORD CWardWizScanner::ScanFile(LPTSTR lpszFilePath, LPTSTR szVirusName, DWORD& dwISpyID, DWORD dwScanType, DWORD &dwAction, DWORD& dwSignatureFailedToLoad, bool bRescan)
{
	DWORD dwReturn = 0x00;

	m_csScanFile.Lock();

	try
	{
		//Issue Resolved: 0001783
		if (m_bManualStop)
		{
			m_csScanFile.Unlock();
			return dwReturn;
		}
		bool bFound = false;

		if (IsScanByFileName(lpszFilePath))
		{
			_tcscpy(szVirusName, L"USER_DEFINED");
			m_csScanFile.Unlock();
			bFound = true;
		}
		else
		{
			bFound = m_objISpyScanner.ScanFile(lpszFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, bRescan);
		}

		//if found
		if (bFound)
		{
			TCHAR szScanType[MAX_PATH] = { 0 };

			//Set as detected
			dwReturn = 0x01;

			//If a call is for indexing then no need to quarentine.
			if (m_eScanType == INDEXING)
			{
				m_csScanFile.Unlock();
				return dwReturn;
			}

			//If Number of days left 0, then no need to quarentine, simple reply as detected only.
			if (m_dwDaysLeft <= 0x00)
			{
				dwAction = FILESKIPPED;
				m_csScanFile.Unlock();
				return dwReturn;
			}

			//Check here if user has not set the AutoQuarentine option as skipped.
			switch (dwScanType)
			{
			case QUICKSCAN:
				_tcscpy_s(szScanType, L"Quick Scan");
				break;
			case FULLSCAN:
				_tcscpy_s(szScanType, L"Full Scan");
				break;
			case CUSTOMSCAN:
				_tcscpy_s(szScanType, L"Custom Scan");
				break;
			case ANTIROOTKITSCAN:
				_tcscpy_s(szScanType, L"Antirootkit scan");
				break;
			case USBSCAN:
				_tcscpy_s(szScanType, L"RightClick Scan");
				break;
			case USBDETECT:
				_tcscpy_s(szScanType, L"USB Scan");
				break;
			case ACTIVESCAN:
				_tcscpy_s(szScanType, L"Active Scan");
				break;
			}

			CString csBackupPath;
			//Send file to quarentine
			DWORD dwCleanResult = HandleVirusEntry(lpszFilePath, szVirusName, szScanType, dwISpyID, csBackupPath, dwAction);
			if (dwCleanResult != 0x00)
			{
				//temporary: if malware repair fails, dont show as detected
				//Set as detected
				dwReturn = 0x00;

				AddLogEntry(L"### Failed to HandleVirusEntry, FilePath: %s, Threat Name: %s", lpszFilePath, szVirusName, true, SECONDLEVEL);
				m_csScanFile.Unlock();
				return dwReturn;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::ScanFile for file %s", lpszFilePath, 0, true, SECONDLEVEL);
	}

	m_csScanFile.Unlock();

	return dwReturn;
}

/***************************************************************************
Function Name  : SetAutoQuarentineOption
Description    : Function to set AutoQuaretine option
Author Name    : Ram Shelke
SR_NO		   : 
Date           : 25 Mar 2016
****************************************************************************/
void CWardWizScanner::SetAutoQuarentineOption(DWORD dwOption)
{
	__try
	{
		m_dwAutoQuarOption = dwOption;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception CWardwizScanner::SetAutoQuarentineOption", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : ReloadExcludeDB
Description    : Function to reload Exclude files and folders DB
Author Name    : Ram Shelke
SR_NO		   :
Date           : 25 Mar 2016
****************************************************************************/
bool CWardWizScanner::ReloadExcludeDB()
{
	bool bReturn = false;
	__try
	{
		bReturn = m_objExcludeFilesFolders.LoadExcludeDB();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception CWardwizScanner::ReloadExcludeDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name		: GetDaysLeft
Description			: Function to get Number of days left from registration DLL.
SR.NO				:
Author Name			: Ramkrushna Shelke
Date				: 20 Jan 2014
***********************************************************************************************/
DWORD CWardWizScanner::GetDaysLeft()
{
	DWORD dwDaysLeft = 0x00;
	__try
	{
		typedef DWORD(*GETDAYSLEFT_SERVICE)(DWORD);

		GETDAYSLEFT_SERVICE	GetDaysLeft = NULL;

		TCHAR	szTemp[512] = { 0 };

		GetModuleFileName(NULL, szTemp, 511);

		TCHAR	*pTemp = wcsrchr(szTemp, '\\');
		if (pTemp)
		{
			pTemp++;
			*pTemp = '\0';
		}

		wcscat(szTemp, L"VBREGISTRATION.DLL");

		DWORD dwRet = 0x00;
		HMODULE	hMod = LoadLibrary(szTemp);

		if (hMod)
			GetDaysLeft = (GETDAYSLEFT_SERVICE)GetProcAddress(hMod, "GetDaysLeft");

		if (GetDaysLeft)
		{
			DWORD	dwProductID = 0x00;
			dwProductID = GetProductID();
			dwDaysLeft = m_dwDaysLeft = GetDaysLeft(dwProductID);
		}

		if (hMod)
		{
			FreeLibrary(hMod);
			hMod = NULL;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::GetDaysLeft", 0, 0, true, SECONDLEVEL);
	}
	return dwDaysLeft;
}


/**********************************************************************************************************
*  Function Name		: CheckIFAlreadyBackupTaken
*  Description			: Function to check whether backup already taken (or) not.
*  Function Arguments	: szFileHash
*  Author Name			: Ram Shelke
*  Date					: 2 Feb 2017
**********************************************************************************************************/
bool CWardWizScanner::CheckIFAlreadyBackupTaken(LPCTSTR szFileHash, LPTSTR szBackupPath)
{
	bool bReturn = false;
	__try
	{
		//Sanity check
		if (!szFileHash || !szBackupPath)
			return bReturn;

		if (CheckEntryPresent(szFileHash, szBackupPath))
		{
			bReturn = true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::CheckIFAlreadyBackupTaken, HASH: %s, BackupPath:%s ", szFileHash, szBackupPath, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: CheckEntryPresent
*  Description			: Function to check entry present in list
*  Function Arguments	: szFileHash
*  Author Name			: Ram Shelke
*  Date					: 2 Feb 2017
**********************************************************************************************************/
bool CWardWizScanner::CheckEntryPresent(LPCTSTR szFileHash, LPTSTR szBackupPath)
{
	bool bReturn = false;
	try
	{
		//Sanity check
		if (!szFileHash)
			return bReturn;

		const ContactList& contacts = m_ptrISpyDBManipulation->m_RecoverDBEntries.GetContacts();
		POSITION pos = contacts.GetHeadPosition();
		while (pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
			CString csSecondEntry = contact.GetSecondEntry();
			if (csSecondEntry.Find(szFileHash) > 0)
			{
				_tcscpy(szBackupPath, csSecondEntry);
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::CheckIFAlreadyBackupTaken, HASH: %s, BackupPath:%s ", szFileHash, szBackupPath, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: GetFileHash(
*  Description			: Function to get file hash
*  Function Arguments	: pFilePath (In), pFileHash(out)
*  Author Name			: Ram Shelke
*  Date					: 02 Feb 2017
**********************************************************************************************************/
bool CWardWizScanner::GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash)
{
	bool bReturn = false;
	try
	{

		if (!pFilePath || !pFileHash)
			return bReturn;

		HMODULE hHashDLL = LoadLibrary(L"VBHASH.DLL");
		if (!hHashDLL)
		{
			DWORD dwRet = GetLastError();
			AddLogEntry(L"### Failed in CWardwizScanner::GetFileHash (%s)", L"VBHASH.DLL");
			return true;
		}
		
		typedef DWORD(*GETFILEHASH)	(TCHAR *pFilePath, TCHAR *pFileHash);
		GETFILEHASH fpGetFileHash = (GETFILEHASH)GetProcAddress(hHashDLL, "GetFileHash");
		if (!fpGetFileHash)
		{
			AddLogEntry(L"### Failed in CWardwizScanner::GetFileHash Address(%s)", L"GetFileHash");
			return true;
		}

		DWORD dwRet = fpGetFileHash(pFilePath, pFileHash);
		if (dwRet == 0x00)
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::GetFileHash, FilePath: %s", pFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: CheckForDuplicateEntry
*  Description			: Function to check for duplicate entry 
*  Function Arguments	: 
*  Author Name			: Ram Shelke
*  Date					: 02 Feb 2017
**********************************************************************************************************/
bool CWardWizScanner::CheckForDuplicateEntry(TCHAR *pFilePath, TCHAR *pBackupPath)
{
	bool bReturn = false;
	try
	{
		if (!pFilePath || !pBackupPath)
			return bReturn;

		const ContactList& contacts = m_ptrISpyDBManipulation->m_RecoverDBEntries.GetContacts();
		POSITION pos = contacts.GetHeadPosition();
		while (pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
			CString csFirstEntry = contact.GetFirstEntry();
			if (csFirstEntry.CompareNoCase(pFilePath) != 0)
			{
				CString csSecondEntry = contact.GetSecondEntry();
				if (csSecondEntry.CompareNoCase(pBackupPath) == 0)
				{
					bReturn = true;
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::CheckForDuplicateEntry, FilePath: %s, BackupPath:%s", pFilePath, pBackupPath, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: ISExcludedPath
*  Description			: Function to check excluded file path
*  Function Arguments	:
*  Author Name			: Amol Jaware
*  Date					: 26 July 2017
**********************************************************************************************************/
bool CWardWizScanner::ISExcludedPath(LPTSTR lpszPath, bool &bISSubFolderExcluded)
{
	bool bReturn = false;
	try
	{
		if (!lpszPath)
			return bReturn;

		bReturn = m_objExcludeFilesFolders.ISExcludedPath(lpszPath, bISSubFolderExcluded);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::ISExcludedPath, FilePath: %s", lpszPath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: ISExcludedFileExt
*  Description			: Function to check excluded file extensions.
*  Function Arguments	:
*  Author Name			: Amol Jaware
*  Date					: 26 July 2017
**********************************************************************************************************/
bool CWardWizScanner::ISExcludedFileExt(LPTSTR lpszFileExt)
{
	bool bReturn = false;
	try
	{
		if (!lpszFileExt)
			return bReturn;

		bReturn = m_objExcludeFilesFolders.ISExcludedFileExt(lpszFileExt);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::ISExcludedFileExt, FileExt: %s", lpszFileExt, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: ReadBootRecoverEntries
*  Description			: Function which reads the boot time entries which merge into recover DB.
*  Function Arguments	: 
*  Author Name			: Ram Shelke
*  Date					: 24 OCT 2017
**********************************************************************************************************/
bool CWardWizScanner::ReadBootRecoverEntries()
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool bReturn = false;
	try
	{
		CString csBootRecDBPath = GetQuarantineFolderPath();
		csBootRecDBPath += L"\\";
		csBootRecDBPath += WRDWIZBOOTRECOVERDB;
		
		if (!PathFileExists(csBootRecDBPath))
		{
			return bReturn;
		}

		hFile = CreateFile(csBootRecDBPath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### CreateFile failed in CWardwizScanner::ReadBootRecoverEntries file: [%s]", csBootRecDBPath, 0, true, SECONDLEVEL);
			bReturn = false;
			goto Cleanup;
		}

		BOOTRECOVERENTRIES stFileData = { 0 };
		memset(&stFileData, 0x00, sizeof(stFileData));

		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);

		while (true)
		{
			DWORD dwBytesRead = 0x00;
			if (!ReadFile(hFile, &stFileData, sizeof(stFileData), &dwBytesRead, NULL))
			{
				break;
			}

			if (dwBytesRead != sizeof(stFileData))
			{
				break;
			}

			TCHAR szFilePath[0x104] = { 0 };
			TCHAR szBackupPath[0xC8] = { 0 };
			TCHAR szThreatName[0x32] = { 0 };

			std::string strFilePath(stFileData.szFilePath);
			std::string strBackupPath(stFileData.szBackupPath);
			std::string strThreatName(stFileData.szThreatName);

			std::wstring strwFilePath = String2WString(strFilePath.c_str());
			std::wstring strwBackupPath = String2WString(strBackupPath.c_str());
			std::wstring strwThreatName = String2WString(strThreatName).c_str();

			if (!InsertRecoverEntry(strwFilePath.c_str(), strwBackupPath.c_str(), strwThreatName.c_str(), 0x00))
			{
				AddLogEntry(L"### Error in InsertRecoverEntry, Path: %s | BackupPath: %s", strwFilePath.c_str(), strwBackupPath.c_str(), true, SECONDLEVEL);
			}

			if (!SaveInRecoverDB())
			{
				AddLogEntry(L"### Error in SaveInRecoverDB, Path: %s | BackupPath: %s", strwFilePath.c_str(), strwBackupPath.c_str(), true, SECONDLEVEL);
			}
		}

		//Close file handle here
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

		//remove the temporary created file
		SetFileAttributes(csBootRecDBPath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(csBootRecDBPath))
		{
			AddLogEntry(L"### Failed to delete file: [%s]", csBootRecDBPath, 0, true, SECONDLEVEL);
			bReturn = false;
			goto Cleanup;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::ReadBootRecoverEntries", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bReturn;

}

/**********************************************************************************************************
*  Function Name		: ReadBootRecoverEntries
*  Description			: Function which reads the scan session entries which merge into ScanSession table.
*  Function Arguments	: 
*  Author Name			: Ram Shelke
*  Date					: 24 OCT 2017
**********************************************************************************************************/
bool CWardWizScanner::AddScanSessionDetails(INT64 &iSessionID)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool bReturn = false;
	try
	{
		CString csScanSessionDBPath = GetWardWizPathFromRegistry();
		csScanSessionDBPath += WRDSCANSESSDETAILDB;
			
		if (!PathFileExists(csScanSessionDBPath))
		{
			return bReturn;
		}

		hFile = CreateFile(csScanSessionDBPath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### CreateFile failed in CWardwizScanner::AddScanSessionDetails file: [%s]", csScanSessionDBPath, 0, true, SECONDLEVEL);
			bReturn = false;
			goto Cleanup;
		}

		DWORD dwQuarantineOpt = 0x00;
		DWORD dwHeuristicOpt = 0x00;
		bool  bHeuristicOpt = false;

		GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);

		if (dwHeuristicOpt == 0x01)
			bHeuristicOpt = true;

		SCANSESSIONDETAILS stScanSesstion;
		memset(&stScanSesstion, 0, sizeof(stScanSesstion));

		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
		while (true)
		{
			DWORD dwBytesRead = 0x00;
			if (!ReadFile(hFile, &stScanSesstion, sizeof(stScanSesstion), &dwBytesRead, NULL))
			{
				break;
			}

			if (dwBytesRead != sizeof(stScanSesstion))
			{
				break;
			}

			SYSTEMTIME stStartTime = { 0 };;
			FileTimeToSystemTime((FILETIME*)&stScanSesstion.SessionStartDatetime, &stStartTime);

			CString csStartDate;
			csStartDate.Format(L"%d-%02d-%02d", stStartTime.wYear, stStartTime.wMonth, stStartTime.wDay);

			CString csStartDateTime;
			csStartDateTime.Format(L"%d-%02d-%02d %02d:%02d:%02d", stStartTime.wYear, stStartTime.wMonth, stStartTime.wDay, stStartTime.wHour, stStartTime.wMinute, stStartTime.wSecond);

			SYSTEMTIME stEndTime = { 0 };;
			FileTimeToSystemTime((FILETIME*)&stScanSesstion.SessionEndDate, &stEndTime);

			CString csEndDate;
			csEndDate.Format(L"%d-%02d-%02d", stEndTime.wYear, stEndTime.wMonth, stEndTime.wDay);

			CString csEndDateTime;
			csEndDateTime.Format(L"%d-%02d-%02d %02d:%02d:%02d", stEndTime.wYear, stEndTime.wMonth, stEndTime.wDay, stEndTime.wHour, stEndTime.wMinute, stEndTime.wSecond);

			CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");
			csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,'%s','%s','%s','%s',%d,%d,%d,%d,%d );"), BOOTSCANNER, csStartDate, csStartDateTime, csEndDate, csEndDateTime, stScanSesstion.TotalFilesScanned, stScanSesstion.TotalThreatsFound, dwQuarantineOpt, bHeuristicOpt, stScanSesstion.TotalThreatsCleaned);
			CT2A ascii(csInsertQuery, CP_UTF8);

			CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFile);
			objSqlDb.Open();
			objSqlDb.ExecDML(ascii.m_psz);
			iSessionID = objSqlDb.GetLastRowId();
			objSqlDb.Close();
		}

		//Close file handle here
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

		//remove the temporary created file
		SetFileAttributes(csScanSessionDBPath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(csScanSessionDBPath))
		{
			AddLogEntry(L"### Failed to delete file: [%s]", csScanSessionDBPath, 0, true, SECONDLEVEL);
			bReturn = false;
			goto Cleanup;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::CWardwizScanner::AddScanSessionDetails", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bReturn;

}

/**********************************************************************************************************
*  Function Name		: AddScanDetails
*  Description			: Function which add entries in scandetails table
*  Function Arguments	: 
*  Author Name			: Ram Shelke
*  Date					: 24 OCT 2017
**********************************************************************************************************/
bool CWardWizScanner::AddScanDetails(INT64 iSessionID)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool bReturn = false;
	try
	{
		CString csScanDetailsDBPath = GetWardWizPathFromRegistry();
		csScanDetailsDBPath += WRDWIZSCANDETAILSDB;

		if (!PathFileExists(csScanDetailsDBPath))
		{
			return bReturn;
		}

		hFile = CreateFile(csScanDetailsDBPath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### CreateFile failed in CWardwizScanner::AddScanSessionDetails file: [%s]", csScanDetailsDBPath, 0, true, SECONDLEVEL);
			bReturn = false;
			goto Cleanup;
		}

		WRDWIZSCANDETAILS stScanDetails = { 0 };
		memset(&stScanDetails, 0x00, sizeof(stScanDetails));

		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);

		while (true)
		{
			DWORD dwBytesRead = 0x00;
			if (!ReadFile(hFile, &stScanDetails, sizeof(stScanDetails), &dwBytesRead, NULL))
			{
				break;
			}

			if (dwBytesRead != sizeof(stScanDetails))
			{
				break;
			}

			CString csAction = L"IDS_FILE_SKIPPED";
			switch (stScanDetails.dwActionTaken)
			{
			case FILESKIPPED:
				csAction = L"IDS_FILE_SKIPPED";
				break;
			case FILEQURENTINED:
				csAction = L"IDS_CONSTANT_THREAT_QUARANTINED";
				break;
			case FILEREPAIRED:
				csAction = L"IDS_CONSTANT_THREAT_REPAIRED";
				break;
			case LOWDISKSPACE:
				csAction = L"IDS_CONSTANT_THREAT_LOWDISC_SPACE";
				break;
			case FILEREBOOTQUARENTINE:
				break;
			case FILEREBOOTREPAIR:
				break;
			}

			SYSTEMTIME stStartTime = { 0 };;
			if (FileTimeToSystemTime((FILETIME*)&stScanDetails.lIntstartDatetime, &stStartTime) == 0x00)
			{
				continue;
			}

			CString csStartDate;
			csStartDate.Format(L"%d-%02d-%02d", stStartTime.wYear, stStartTime.wMonth, stStartTime.wDay);

			CString csStartDateTime;
			csStartDateTime.Format(L"%d-%02d-%02d %02d:%02d:%02d", stStartTime.wYear, stStartTime.wMonth, stStartTime.wDay, stStartTime.wHour, stStartTime.wMinute, stStartTime.wSecond);

			SYSTEMTIME stEndTime = { 0 };
			if (FileTimeToSystemTime((FILETIME*)&stScanDetails.lIntEndDateTime, &stEndTime) == 0x00)
			{
				continue;
			}
			
			CString csEndDate;
			csEndDate.Format(L"%d-%02d-%02d", stEndTime.wYear, stEndTime.wMonth, stEndTime.wDay);

			CString csEndDateTime;
			csEndDateTime.Format(L"%d-%02d-%02d %02d:%02d:%02d", stEndTime.wYear, stEndTime.wMonth, stEndTime.wDay, stEndTime.wHour, stEndTime.wMinute, stEndTime.wSecond);

			CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");
			csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%I64d,'%s','%s','%s','%s','%s','%s','%s',NULL);"), iSessionID, csStartDate, csStartDateTime, csEndDate, csEndDateTime, CString(stScanDetails.szThreatName), CString(stScanDetails.szFilePath), csAction);

			CT2A ascii(csInsertQuery, CP_UTF8);

			InsertData(ascii.m_psz);
		}

		//Close file handle here
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

		//remove the temporary created file
		SetFileAttributes(csScanDetailsDBPath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(csScanDetailsDBPath))
		{
			AddLogEntry(L"### Failed to delete file: [%s]", csScanDetailsDBPath, 0, true, SECONDLEVEL);
			bReturn = false;
			goto Cleanup;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::AddScanSessionDetails", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: OnRecoverQuarantineEntries
*  Description			: Function to make files quarantine on dragging and dropping
*  Author Name			: Akshay Patil
*  Date					: 05 OCT 2018
**********************************************************************************************************/
bool CWardWizScanner::OnRecoverQuarantineEntries(LPTSTR szThreatPath)
{
	bool bReturn = false;
	try
	{
		TCHAR szBackupFilePath[MAX_PATH * 2] = { 0 };

		if (!PathFileExists(szThreatPath))
		{
			return bReturn;
		}

		CString csQuaratineFolderPath = GetQuarantineFolderPath();
		if (!PathFileExists(csQuaratineFolderPath))
		{
			if (!CreateDirectory(csQuaratineFolderPath, NULL))
			{
				AddLogEntry(L"### CWardwizScanner::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
				return bReturn;
			}
		}
	
		if (!BackUpBeforeQuarantineOrRepair(szThreatPath, szBackupFilePath))
		{
			AddLogEntry(L"#### Failed to take backup of %s", szThreatPath, 0, true, SECONDLEVEL);
			return bReturn;
		}

		if (!QuarantineEntry(szThreatPath, L"USER_DEFINED", szBackupFilePath))
		{
			AddLogEntry(L"### Failed to Quarantine file: %s", szThreatPath, 0, true, SECONDLEVEL);
			return bReturn;
		}

		if (!InsertRecoverEntry(szThreatPath, szBackupFilePath, L"USER_DEFINED", 0x00))
		{
			AddLogEntry(L"### Error in InsertRecoverEntry, Path: %s | BackupPath: %s", szThreatPath, L"", true, SECONDLEVEL);
			return bReturn;
		}

		if (!SaveInRecoverDB())
		{
			AddLogEntry(L"### Error in SaveInRecoverDB, Path: %s | BackupPath: %s", szThreatPath, szBackupFilePath, true, SECONDLEVEL);
			return bReturn;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::OnRecoverQuarantineEntries", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  : IsScanByFileName
*  Description    :	Function to check is file scan by name.
*  Author Name    : Amol Jaware
*  Date           : 05 Oct 2018
**********************************************************************************************************/
bool CWardWizScanner::IsScanByFileName(PCTSTR szFilePath)
{
	try
	{
		CString csFilePath = szFilePath;
		int iScanByNameCount = csFilePath.ReverseFind('\\');
		CString csScanByFileName = csFilePath.Right(csFilePath.GetLength() - (++iScanByNameCount));

		if (m_objScanByName.IsScanByNameFile((LPTSTR)csScanByFileName.GetString()))
			return true;
		else
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::IsScanByFileName");
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  : ReloadScanByName
*  Description    :	Function to reload scan by file name.
*  Author Name    : Amol Jaware
*  Date           : 05 Oct 2018
**********************************************************************************************************/
void CWardWizScanner::ReloadScanByName()
{
	try
	{
		m_objScanByName.LoadScanByNameDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::ReloadScanByName");
	}
	}

/**********************************************************************************************************
*  Function Name  : MoveFile2TempRestartDelete
*  Description    :	Function which moves files to temporary location and deletes on restart.
*  Author Name    : Ram Shelke
*  Date           : 30 Aug 2019
**********************************************************************************************************/
bool CWardWizScanner::MoveFile2TempRestartDelete(LPCTSTR szThreatPath)
{
	bool bReturn = false;
	try
	{
		//move original file to temp location
		TCHAR szTempFileName[MAX_PATH];
		TCHAR lpTempPathBuffer[MAX_PATH];
		UINT RetUnique = 0;
		DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
		if (!dwRetVal)
		{
			return bReturn;
		}

		RetUnique = GetTempFileName(lpTempPathBuffer, L"wwtmp", 0, szTempFileName);
		if (!RetUnique)
		{
			return bReturn;
		}

		if (!DeleteFile(szTempFileName))
		{
			return bReturn;
		}

		if (PathFileExists(szThreatPath))
		{
			if (!MoveFileEx(szThreatPath, szTempFileName, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
			{
				return bReturn;
			}

			if (!DeleteFile(szTempFileName))
			{
				MoveFileEx(szTempFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::Replace2OriginalFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}