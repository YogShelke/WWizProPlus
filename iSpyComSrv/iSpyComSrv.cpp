/**********************************************************************************************************            	  
	  Program Name          : WardWiz Communication service
	  Description           : This file contains functionality about the service 
	  Author Name			: Ramkrushna Shelke                                                                        	  
	  Date Of Creation      : 10 Jan 2014
	  Version No            : 1.0.0.8
	  Special Logic Used    : Communication using named pipes & using Shared Memory.
	  Modification Log      : 
***********************************************************************************************************/
#include "stdafx.h"
#include "iSpyComSrv.h"
#include "ISpyCommServer.h"
#include "PipeConstants.h"
#include "ISpyScannerBase.h"
#include "ISpyDBManipulation.h"
#include "iTinRegWrapper.h"
#include "iSpyRegOpt.h"
#include "ISpyCriticalSection.h"
#include "ITinRegDataOperations.h"
#include "WardwizLangManager.h"
#include "WardWizDumpCreater.h"
#include "ExecuteProcess.h"
#include "WrdWizSystemInfo.h"
#include "WardWizDataCrypt.h"
#include "WinHttpManager.h"
#include "DriverConstants.h"
#include "WWizIndexer.h"
#include "WardWizIniWrapper.h"
#include "EnumProcess.h"
#include <map>
#include <string> 
#include <winioctl.h>
#include <crtdbg.h>
#include <fltuser.h>
#include "scanuk.h"
#include "scanuser.h"
#include <thread>
#include <mutex>
#include <iostream>
#include <fstream>
#include <condition_variable>
#include <future>
#include "WardWizDatabaseInterface.h"
#include "CPUInfo.h"
#include "WardWizDatabaseInterface.h"
#include "WardWizFwClient.h"
#include "WardWizEmailClient.h"
#include "WardWizWebProtClient.h"
#include "WardWizBrowserSec.h"

CISpyScannerBase			g_objISpyScannerBase;
CISpyDBManipulation			g_objISpyDBManipulation;
CISpyCommunicatorServer		g_objCommServer(SERVICE_SERVER, &OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));
CISpyRegOpt					g_iSpyRegOpt;
CITinRegWrapper				g_objReg;
CITinRegDataOperations		g_regDataOperation;
CISpyCriticalSection		g_objISpyCriticalSection;
CISpyCriticalSection		g_objcsScanFile;
CISpyCriticalSection		g_objOnAccessScan;
CISpyCriticalSection		g_objOnAccessFileAdd;
CWardWizDataCrypt			g_objWardWizDataCrypt;
CWardWizFwClient			g_objFWClient;
CWardWizEmailClient			g_objEmailClient;
CWardWizWebProtClient		g_objWebProtClient;
CWardWizBrowserSec			g_objBrowserSec;

CString						g_csRegKeyPath;
bool						g_bCoInitializeSecurityCalled = false;
bool						g_bFilesToBeScannedReady;
bool						g_bISActiveScanDisabled	= false;
bool						g_bISFwPCEmailInitialized = false;
bool						g_bISFileExtLoaded = false;
bool						g_updateInfo = false;
DWORD						g_dwPerfLevel = 0x02;

ISPY_PIPE_DATA				g_szSpyData = { 0 };
char*						g_strDatabaseFilePath = ".\\VBALLREPORTS.DB";
HANDLE						g_hEvent = NULL;

CISpyCommunicator			g_objCom(TRAY_SERVER, false);
CISpyCommunicator			m_objComTray(TRAY_SERVER, true, 3);

static DWORD	dwTlsIndex		=	0;	// address Threads
static DWORD	dwTlsMemory		=	1;	// address of shared memory

#define SCANNER_DEFAULT_REQUEST_COUNT       5
#define SCANNER_DEFAULT_THREAD_COUNT       25
#define SCANNER_MAX_THREAD_COUNT           25
#define	MAXQUERYLENGTH						1000

#define	SCANDAILY		'1'
#define	SCANQUICK		'0'
#define	SCANFULL		'1'
#define	SCANSPEC		'2'
#define TMPFILECLNR		'4'
#define REGOPT			'5'
#define	SCANWEEKLY		'2'
#define	ISSUNDAY		 1
#define	ISMONDAY		 2
#define	ISTUESDAY		 3
#define	ISWEDNESDAY		 4
#define	ISTHURSDAY		 5
#define	ISFRIDAY		 6
#define	ISSATURDAY		 7
#define	SCANSUNDAY		'1'
#define	SCANMONDAY		'1'
#define	SCANTUESDAY		'1'
#define	SCANWEDNESDAY	'1'
#define	SCANTHURSDAY    '1'
#define	SCANFRIDAY	    '1'
#define SCANSATURDAY	'1'
#define	SCANMONTHLY		'3'

#define SESSION_MANAGER_REG		L"SYSTEM\\CurrentControlSet\\Control\\Session Manager"
#define BOOT_EXEC_REG			L"BootExecute"
#define WRDWIZBOOTSCN			L"VBBOOTSCN.EXE"
#define	WRDWIZBTSCNSESSIONDB	L"VBBTSCNSESSION.DB"
#define WSZDRIVE				L"\\\\.\\C:"

UCHAR FoulString[] = "foul";


/***********************************************************************************************
  Function Name  : _tmain
  Description    : Application start point function
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
int _tmain(int argc, _TCHAR* argv[])
{

	DWORD	nRetCode = 0x00 ;

	SERVICE_TABLE_ENTRY SrvTable[] =
	{
		{ szSrvName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	} ;

	if( argc > 0x01 )
	{
		if( _wcsicmp( argv[1], L"install") == 0 )
		{
			nRetCode  = InstallService( szSrvName, szSrvDisplayName ) ;
			//return nRetCode ;
		}

		if( _wcsicmp( argv[1], L"remove") == 0 )
		{
			nRetCode  = UnInstallService( szSrvName ) ;
		}

		if( _wcsicmp( argv[1], L"start") == 0 )
		{
			nRetCode  = StartServiceManually( szSrvName, szSrvDisplayName ) ;
		}

		if( _wcsicmp( argv[1], L"stop") == 0 )
		{
			nRetCode  = StopServiceManually( szSrvName ) ;
		}


		return nRetCode ;
	}

	StartServiceCtrlDispatcher( SrvTable ) ;

	return nRetCode ;
}

/***********************************************************************************************
  Function Name  : ServiceMain
  Description    : Service main function which gets registers with Windows SCM.
  SR.NO			 : WRDWIZCOMMSRV_43
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
void WINAPI ServiceMain(DWORD dwArgC, LPTSTR *pArgV)
{
	__try
	{
		//Adding Computer name, user name and OS information at the top of tray log file
		//Added by Vilas on 5 May 2015
		AddUserAndSystemInfoToLog();

		hServiceStatus = RegisterServiceCtrlHandler(szSrvName, ServiceCtrlHandler ) ;

		ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS ;
		ServiceStatus.dwServiceSpecificExitCode = 0 ;

		if( !UpdateServiceStatus (SERVICE_START_PENDING, NO_ERROR, 3000 ) )
		{
			if (hServiceStatus)
				UpdateServiceStatus(SERVICE_STOPPED, 0, 0) ;		
		}

		StartServiceWorkerThread( dwArgC, pArgV ) ;

		UpdateServiceStatus(SERVICE_STOPPED, 0, 0) ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ServiceCtrlHandler", 0, 0, true, SECONDLEVEL);
	}
	exit( 0 ) ;
}

/***********************************************************************************************
  Function Name  : ServiceCtrlHandler
  Description    : Service control handler function which manages with Service manual operations
  SR.NO			 : WRDWIZCOMMSRV_43
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
void WINAPI ServiceCtrlHandler( DWORD dwCtrlCode )
{
	__try
	{
		switch(dwCtrlCode)
		{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:

			UpdateServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0) ;
			StopServiceByEvent() ;
			return ;

		case SERVICE_CONTROL_INTERROGATE:
			break;

		default:
			break ;
		}
		UpdateServiceStatus(ServiceStatus.dwCurrentState, NO_ERROR, 0);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ServiceCtrlHandler", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
  Function Name  : ServiceCtrlHandler
  Description    : Service control handler function which manages with Service manual operations
  SR.NO			 : WRDWIZCOMMSRV_45
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
BOOL UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	BOOL			success = FALSE ;
	__try
	{
		static DWORD	dwCheckPoint = 1 ;

		if(dwCurrentState == SERVICE_START_PENDING )
			ServiceStatus.dwControlsAccepted = 0;
		else
			ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN ;


		ServiceStatus.dwCurrentState	= dwCurrentState ;
		ServiceStatus.dwWin32ExitCode	= dwWin32ExitCode ;
		ServiceStatus.dwWaitHint		= dwWaitHint ;

		if(	dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED )
			ServiceStatus.dwCheckPoint = 0 ;
		else
			ServiceStatus.dwCheckPoint = dwCheckPoint++;

		success = SetServiceStatus( hServiceStatus, &ServiceStatus);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in UpdateServiceStatus", 0, 0, true, SECONDLEVEL);
	}
	return success ;
}

/***********************************************************************************************
  Function Name  : StartServiceWorkerThread
  Description    : Service control handler function which manages with Service manual operations.
  SR.NO			 : WRDWIZCOMMSRV_45
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
void StartServiceWorkerThread(DWORD dwArgC, LPTSTR *pArgV)
{
	__try
	{
		StartServiceWorkerThreadSEH(dwArgC, pArgV);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in StartServiceWorkerThread", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : StartServiceWorkerThreadSEH
Description    : Service control handler function which manages with Service manual operations.
SR.NO		   : WRDWIZCOMMSRV_45
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
void StartServiceWorkerThreadSEH(DWORD dwArgC, LPTSTR *pArgV)
{
	try
	{
		//HANDLE hProcessScanQThread;
		SCANNER_THREAD_CONTEXT structThreadContext;
		PSCANNER_MESSAGE pScanMsg;
		DWORD dwThreadId;
		HRESULT hrFilterConnect;
		DWORD iThreadNum, iRequestNum;
		m_TotalThreatsCleaned = 0x00;

		hEvent_ServiceStop = CreateEvent(NULL, TRUE, FALSE, NULL) ;
		if( !hEvent_ServiceStop )
		{
			//AddToLo
			return ;
		}

		hActiveScanEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!hActiveScanEvent)
		{
			return;
		}

		if( !UpdateServiceStatus( SERVICE_RUNNING, NO_ERROR, 0 ) )
		{
			if( hEvent_ServiceStop )
			{
				CloseHandle(hEvent_ServiceStop) ;
				hEvent_ServiceStop = NULL ;
			}

			return ;
		}

		//Start here commservice late than ALU Service.
		Sleep(5 * 1000);

		g_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		if (!GetRegisteredUserInformation())
		{
			AddLogEntry(L"### Failed to get GetRegisteredUserInformation", 0, 0, true, SECONDLEVEL);
		}
		
		//Neha Gharge 16/1/2015 No of days not display at the time of installation.
		//Write here the machine ID in registry for further use.
		if (!CheckForMIDInRegistry())
		{
			AddLogEntry(L"### Failed to CheckForMIDInRegistry", 0, 0, true, SECONDLEVEL);
		}

		AddLogEntry(L">>> Before Wait for ALU service to complete the task", 0, 0, true, FIRSTLEVEL);

		//ISSUE:scan dll not getting rename during restart after updating product
		//resolved : lalit kumawat 9-16-2015
		//Isssue Resolved: 1162
		g_hEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, WWUPDATECMPLTEVENT);

		if (g_hEvent) // event does not already exist, or other problem
		{
			//OutputDebugString(L"\n>>> Waiting for event to complete ALUSrv task");
			AddLogEntry(L">>> Waiting for event to complete ALUSrv task", 0, 0, true, FIRSTLEVEL);
			DWORD dwWaitResult = WaitForSingleObject(g_hEvent, 3 * 60 * 1000);//3 minutes

			switch (dwWaitResult)
			{
				// All thread objects were signaled
			case WAIT_OBJECT_0:
				if (bStopService)
				{
					goto main_cleanup;
				}
				AddLogEntry(L">>> Signaled from ALU service as VibraniumUPDATECMPLTEVENT completed", 0, 0, true, FIRSTLEVEL);
				break;

				// An error occurred
			default:
				AddLogEntry(L">>> TimeOut for VibraniumUPDATECMPLTEVENT", 0, 0, true, FIRSTLEVEL);
				return;
			}

			CloseHandle(g_hEvent);
			g_hEvent = NULL;
		}
		else
		{
			AddLogEntry(L">>> No need to wait for event to complete ALUSrv task", 0, 0, true, FIRSTLEVEL);
		}

		AddLogEntry(L">>> After Wait for ALU service to complete the task", 0, 0, true, FIRSTLEVEL);
		

		AddServiceEntryInSafeMode();

		CheckScanLevel();

		hThread_SrvPrimary = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SrvPrimaryThread,
			NULL, 0, 0);
		Sleep(500);
		
		CSecure64  objCSecure;
		objCSecure.RegisterProcessId(WLSRV_ID_EIGHT);// Issue resolved: 0001288, duplicate ID's was present

		CScannerLoad	objCScanner;
		objCScanner.RegisterProcessId(WLSRV_ID_EIGHT);//WLSRV_ID_EIGHT to register service for process protection
		
		//Start here parental control thread.
		DWORD dwProdID = 0;
		dwProdID = GetProductID();

		LoadRequiredLibrary();

		//get here number of days left
		m_dwDaysLeft = g_objISpyScannerBase.m_objWardWizScanner.GetDaysLeft();

		if (!(dwProdID == ESSENTIAL || dwProdID == BASIC))
		{
			StartFirewallThread();

			Sleep(2 * 1000);	//Don't remove, for default rules

			StartEmailScanThread();

			Sleep(2 * 1000);

			StartParentalControlCheck();
			StartPCInternetCheck();
			StartINetAccessTimeCheck();
		}

		// issue : live update folder should be protected
		// resolve :- lalit kumawat 3-16-2015
		if (!AddProtectionToLiveUpdateFolder())
		{
			AddLogEntry(L"### Unable to protect Live update folder", 0, 0, true, SECONDLEVEL);
		}

		//Function to set autorun scanner settings.
		if (!ApplyAutorunScanSettings())
		{
			AddLogEntry(L"### function failed ApplyAutorunScanSettings", 0, 0, true, SECONDLEVEL);
		}

		if(hThread_SrvPrimary == NULL )
		{
			StopServiceByEvent() ;
		}

		//Thread to Load WardWiz signature database.
		hThreadLoadSigDatabase = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LoadWardWizSignatureDatabase,
			NULL, 0, 0);
		
		Sleep(100);
		
		//Thread to repair files present in WWRepair.ini
		hThread_RepairViruses = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RepairVirusesThread,
			NULL, 0, 0);

		Sleep(100);
		
		// resolved by lalit kumawat 7-17-2015
		// function to add CRC again to file which modifed by external resourse at system reboot time.
		ParseIntegrityInfoINIAndRollBackCRC();

		//Reports 
		hThread_WatchReportsAction = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) WatchReportsActionThread, NULL, 0, 0);
		Sleep(500);
		
		if(hThread_WatchReportsAction == NULL)
		{
			StopServiceByEvent();
		}

		//Thread to watch the required process that needs to be run all the time.
		//if any one of these will not running will get started.
		hWatchDogThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WatchDogThread,
			NULL, 0, 0);
		Sleep(50);

		StartIndexing();
		StartScheduledScanThread();

		try
		{
			// Variables Required for OnAccess Logic
			DWORD dwRequestCount = SCANNER_DEFAULT_REQUEST_COUNT;
			DWORD dwThreadCount = SCANNER_DEFAULT_THREAD_COUNT;
			HANDLE hQueueThread[SCANNER_DEFAULT_THREAD_COUNT] = { NULL };
			std::thread m_hProcessingThreads[SCANNER_DEFAULT_THREAD_COUNT];

			// Invoke logic to invoke thread for onaccess notification
			// Invoke thread to scan the file paths from the map.
			//hProcessScanQThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessScanQueueThread, NULL, 0, 0);
			//SetThreadPriority(hProcessScanQThread, THREAD_PRIORITY_ABOVE_NORMAL);

			// Add the required file paths to the map associated..
			hrFilterConnect = FilterConnectCommunicationPort(ScannerPortName, 0, NULL, 0, NULL, &g_hPort);
			if (!IS_ERROR(hrFilterConnect)) {
				//  Create a completion port to associate with this handle.
				g_hCompletion = CreateIoCompletionPort(g_hPort, NULL, 0, dwThreadCount);

				if (g_hCompletion == NULL) {
					CloseHandle(g_hPort);
				}

				structThreadContext.Port = g_hPort;
				structThreadContext.Completion = g_hCompletion;

				//  Create specified number of threads.
				for (iThreadNum = 0; iThreadNum < dwThreadCount; iThreadNum++) {

					hQueueThread[iThreadNum] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PopulateScanQueue, &structThreadContext, 0, &dwThreadId);

					if (hQueueThread[iThreadNum] == NULL) {
						//  Couldn't create thread.
						hrFilterConnect = GetLastError();
						break;
					}

					SetThreadPriority(hQueueThread[iThreadNum], THREAD_PRIORITY_NORMAL);

					m_hProcessingThreads[iThreadNum] = std::thread(ProcessScanQueueThread, std::ref(g_onAccessQueue));
					SetThreadPriority(m_hProcessingThreads[iThreadNum].native_handle(), THREAD_PRIORITY_HIGHEST);

					for (iRequestNum = 0; iRequestNum < dwRequestCount; iRequestNum++)
					{
						//  Allocate the message.
#pragma prefast(suppress:__WARNING_MEMORY_LEAK, "msg will not be leaked because it is freed in ScannerWorker")
						pScanMsg = (PSCANNER_MESSAGE)malloc(sizeof(SCANNER_MESSAGE));

						if (pScanMsg == NULL) {

							hrFilterConnect = ERROR_NOT_ENOUGH_MEMORY;
							break;
						}

						memset(&pScanMsg->Ovlp, 0, sizeof(OVERLAPPED));

						//  Request messages from the filter driver.
						hrFilterConnect = FilterGetMessage(g_hPort, &pScanMsg->MessageHeader, FIELD_OFFSET(SCANNER_MESSAGE, Ovlp), &pScanMsg->Ovlp);

						if (hrFilterConnect != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
							free(pScanMsg);
							break;
						}
					}
				}
			}

			std::thread m_hAppBlockThread;
			if (dwProdID != BASIC || dwProdID != ESSENTIAL)
			{
				m_hAppBlockThread = std::thread(ProcessAppBlockThread, std::ref(g_BlockApplicationQueue));
			}

			// Code updated because in future, we will be giving the Offline with NC in India
			CheckIsOffline();
			if (m_dwIsOffline)
			{
				//Offline Registation Thread
				hSendUserInformationThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendUserInformationThread, NULL, 0, 0);
				Sleep(50);
				if (hSendUserInformationThread == NULL)
				{
					StopServiceByEvent();
				}
			}

			hrFilterConnect = S_OK;

			if (hQueueThread[0] != NULL)
			{
				//Wait here for OnAccess threads to stop
				WaitForMultipleObjectsEx(iThreadNum, hQueueThread, TRUE, INFINITE, FALSE);
			}

			if (hEvent_ServiceStop != NULL)
			{ 
				//Wait call for service stop event
				WaitForSingleObject(hEvent_ServiceStop, INFINITE);
				if (hEvent_ServiceStop)
				{
					CloseHandle(hEvent_ServiceStop);
					hEvent_ServiceStop = NULL;
				}
			}

			//UnInitilize nf here
			UninitializeNFApi();
	
			//Terminate the thread here by adding exit call
			for (int i = 0; i < SCANNER_DEFAULT_THREAD_COUNT; ++i) {

				ACTIVESCANFLAGS	szActiveScanopt;
				memset(&szActiveScanopt, 0x00, sizeof(ACTIVESCANFLAGS));

				_tcscpy(szActiveScanopt.wcProcessPath, L"WRDWIZEXIT");
				g_onAccessQueue.push(szActiveScanopt);
			}

			//Wait here till thread exits.
			for (int i = 0; i < SCANNER_DEFAULT_THREAD_COUNT; ++i) {
				if (m_hProcessingThreads[i].joinable())
				{
					m_hProcessingThreads[i].join();
				}
			}

			if (dwProdID != BASIC || dwProdID != ESSENTIAL)
			{
				BLOCKAPPFLAGS	stBlockApp;
				_tcscpy(stBlockApp.wcProcessPath, L"WRDWIZEXIT");
				g_BlockApplicationQueue.push(stBlockApp);
				if (m_hAppBlockThread.joinable())
				{
					m_hAppBlockThread.join();
				}
			}
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in StartServiceWorkerThreadSEH", 0, 0, true, SECONDLEVEL);
		}

		//Shutdown here the operations running.
		StopRunningOperations();

		//Release Event Handle here
		if (hActiveScanEvent != NULL)
		{
			CloseHandle(hActiveScanEvent);
			hActiveScanEvent = NULL;
		}

		if (g_hPort != NULL)
		{
			CloseHandle(g_hPort);
			g_hPort = NULL;
		}

		if (g_hCompletion != NULL)
		{
			CloseHandle(g_hCompletion);
			g_hCompletion = NULL;
		}


	main_cleanup:
		if (g_hPort != NULL)
		{
			CloseHandle(g_hPort);
		}
		if (g_hCompletion != NULL)
		{
			CloseHandle(g_hCompletion);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartServiceWorkerThreadSEH", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
  Function Name  : StopServiceByEvent
  Description    : Function which sets the service stop event.
  SR.NO			 : WRDWIZCOMMSRV_45
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
void StopServiceByEvent()
{
	if( hEvent_ServiceStop )
	{
		bStopService = TRUE;
		SetEvent( hEvent_ServiceStop ) ;
	}

	if (g_hEvent != NULL)
	{
		SetEvent(g_hEvent);
	}

	if (g_hPort != NULL)
	{
		CloseHandle(g_hPort);
		g_hPort = NULL;
	}

	if (g_hCompletion != NULL)
	{
		CloseHandle(g_hCompletion);
		g_hCompletion = NULL;
	}
}

/***********************************************************************************************
  Function Name  : WriteRegistryEntry
  Description    : Function which writes registry entry.
  SR.NO			 : WRDWIZCOMMSRV_45
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
bool WriteRegistryEntry(REGISTRY_TYPE dwType, LPTSTR szKey, LPTSTR szValueName,
						LPTSTR szValue, LPBYTE byData, DWORD dwData)
{

	bool bReturn = false;
	__try{
		switch(dwType)
		{
		case SZ_STRING:
			if(szKey[0] && szValue[0] && szValueName[0])
			{
				g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, szKey, szValueName, szValue);
			}
			break;
		case SZ_MULSTRING:
			break;
		case SZ_EXPSTRING:
			break;
		case SZ_DWORD:
			if(szKey[0] && szValueName[0])
			{
				g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, szKey, szValueName, dwData);
			}
			break;
		case SZ_BINARY:
			if(szKey[0] && byData && dwData > 0)
			{
				g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, szKey, szValueName, byData, dwData);
			}
			break;
		default:
			AddLogEntry(L"### Invalid Argument in WardwizComsvr WriteRegistryEntry", 0, 0, true, SECONDLEVEL);
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizComsvr WriteRegistryEntry", 0, 0, true, SECONDLEVEL);
	}


	return bReturn;
}

/***********************************************************************************************
  Function Name  : WriteRegistryEntrySEH
  Description    : Structured Exception handler function which writes registry entry.
  SR.NO			 : WRDWIZCOMMSRV_45
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
bool WriteRegistryEntrySEH(LPISPY_PIPE_DATA lpSpyData)
{
	g_objISpyCriticalSection.Lock();

	WriteRegistryEntry((REGISTRY_TYPE)lpSpyData->dwValue, lpSpyData->szFirstParam,
		lpSpyData->szSecondParam, lpSpyData->szThirdParam, lpSpyData->byData, lpSpyData->dwSecondValue);

	g_objISpyCriticalSection.Unlock();

	return true;
}

/***********************************************************************************************
  Function Name  : OnDataReceiveCallBack
  Description    : Callback function to receive notification from User interface.
  SR.NO			 : WRDWIZCOMMSRV_50
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
void OnDataReceiveCallBack(LPVOID lpParam)
{
	DWORD dwReply = 0;
	__try
	{
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		if (!lpSpyData)
		{
			return;
		}

		switch (lpSpyData->iMessageInfo)
		{
		case START_SCAN:
			dwReply = 0;
			if (g_objISpyScannerBase.StartScan((SCANTYPE)lpSpyData->dwValue, lpSpyData->dwSecondValue, lpSpyData->szFirstParam))
			{
				dwReply = 1; 
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case PAUSE_SCAN:
			g_objISpyScannerBase.PauseScan((SCANTYPE)lpSpyData->dwValue);
			break;
		case RESUME_SCAN:
			g_objISpyScannerBase.ResumeScan((SCANTYPE)lpSpyData->dwValue);
			break;
		case STOP_SCAN:
			dwReply = 0;
			if (g_objISpyScannerBase.StopScan((SCANTYPE)lpSpyData->dwValue))
			{
				dwReply = 1; 
			}
			lpSpyData->dwValue = dwReply;
			//lpSpyData->iMessageInfo = SCAN_FINISHED;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case HANDLE_VIRUS_ENTRY:			
			dwReply = 0;
			//Modified by Vilas on 07 April 2015
			//If File is in use, Service will send this status to GUI 
			//If File is in use,	dwReply = 4;
			//recover success,		dwReply = 0;
			dwReply = HandleVirusEntry(lpSpyData);// ->szFirstParam, lpSpyData->szSecondParam, lpSpyData->szThirdParam, lpSpyData->dwValue, lpSpyData->dwSecondValue);
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RECOVER_FILE:
			dwReply = 0;

			//Modified by Vilas on 27 Mar 2015
			//If File is in use, Service will send this status to GUI 
			//If File is in use,	dwReply = 2;
			//recover failed,		dwReply = 1;
			//recover success,		dwReply = 0;

			dwReply = g_objISpyScannerBase.RecoverFile(lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->dwSecondValue);

			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SAVE_RECOVERDB:
			dwReply = 0;
			if (g_objISpyDBManipulation.SaveEntries(lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			else
			{
				AddLogEntry(L"### Failed in SaveEntries", 0, 0, true, SECONDLEVEL);
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case WRITE_REGISTRY:
			dwReply = 0;
			if (WriteRegistryEntrySEH(lpSpyData))
			{
				dwReply = 1;
			}
			else
			{
				AddLogEntry(L"### Failed in WriteRegistryEntrySEH key: %s", lpSpyData->szFirstParam, 0, true, SECONDLEVEL);
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_DBENTRIES:
			dwReply = 0;
			if (g_objISpyDBManipulation.ReloadEntries(lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			else
			{
				AddLogEntry(L"### Failed in ReloadEntries", 0, 0, true, SECONDLEVEL);
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ADD_EMAIL_ENTRY:
		case EDIT_EMAIL_ENTRY:
		case DELETE_EMAIL_ENTRY:
			//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
			dwReply = 0;
			if (lpSpyData->dwValue == VIRUSSCAN)
			{
				if (g_objISpyDBManipulation.ProcessEntry(lpSpyData->iMessageInfo, lpSpyData->dwValue, 0, lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->szThirdParam))
				{
					dwReply = 1;
				}
			}
			else
			{
				if (g_objISpyDBManipulation.ProcessEntry(lpSpyData->iMessageInfo, lpSpyData->dwValue, lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->szThirdParam))
				{
					dwReply = 1;
				}
			}

			if (dwReply != 1)
			{
				AddLogEntry(L"### Failed in ProcessEntry for : %s", lpSpyData->szFirstParam, 0, true, SECONDLEVEL);
			}

			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ADD_FOLDER_LOCKER_ENTRY:
		case DELETE_FOLDER_LOCKER_ENTRY:
		case DELETE_RECOVER_ENTRIES:
		case REPORT_ANTIROOTKIT_ENTRY:
		case REPORT_USBSCAN_ENTRY:
			dwReply = 0;
			if (g_objISpyDBManipulation.ProcessEntry(lpSpyData->iMessageInfo, lpSpyData->szFirstParam, lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			else
			{
				AddLogEntry(L"### Failed in ProcessEntry for : %s", lpSpyData->szFirstParam, 0, true, SECONDLEVEL);
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SAVE_EMAIL_ENTRIES:
		case SAVE_FOLDER_LOCKER_ENTRY:
			dwReply = 0;
			if (g_objISpyDBManipulation.SaveEntries(lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			else
			{
				AddLogEntry(L"### Failed in SAVE_FOLDER_LOCKER_ENTRY", 0, 0, true, SECONDLEVEL);
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			
			break;

		//Apply_folder_locker_setting flag is for device driver to get to know lock or unlock events occur on main UI.
		case APPLY_FOLDER_LOCKER_SETTING:
			dwReply = 0;
			//dwvalue set to 1 for lock event occur and 2 for unlock occur. 
			if (lpSpyData->dwValue)
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;

		case START_REGISTRY_OPTIMIZER:
			dwReply = 0;
			if (g_iSpyRegOpt.StartRegistryOptimizer(lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case STOP_REGISTRY_OPTIMIZER:
			dwReply = 0;
			if (g_iSpyRegOpt.StopRegistryOptimizer())
			{
				dwReply = 1;
			}
			else
			{
				AddLogEntry(L"### Failed in StopRegistryOptimizer", 0, 0, true, SECONDLEVEL);
			}
			lpSpyData->iMessageInfo = REGISTRY_SCAN_FINISHED;
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case DELETE_REPORTS_ENTRIES:
			dwReply = 0;
			if (g_objISpyDBManipulation.RemoveReportEntry(lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->szThirdParam))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case FILE_OPERATIONS:
			dwReply = 0;
			if (g_objISpyScannerBase.FileOperations(lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SAVE_REPORTS_ENTRIES:
			dwReply = 0;
			if (g_objISpyDBManipulation.SaveEntries(lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ADD_REGISTRATION_DATA:
			dwReply = 0;
			if (g_regDataOperation.InsertDataIntoDLL(lpSpyData->byData, lpSpyData->dwValue,
				lpSpyData->dwSecondValue, lpSpyData->szFirstParam))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ADD_REGISTRATION_DATAINFILE:
			dwReply = 0;
			if (g_regDataOperation.AddRegistrationDataInFile(lpSpyData->byData, lpSpyData->dwValue) == 0x00)
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		//Varada Ikhar, Date: 23/04/2015, Implementing RegOpt Pause-Resume.
		case PAUSE_REGISTRY_OPTIMIZER:
			dwReply = 0;
			if (g_iSpyRegOpt.PauseRegistryOptimizer())
			{
				dwReply = 1;
			}
			else
			{
				AddLogEntry(L"### Failed in WardwizComSrv::OnDataReceiveCallBack to PAUSE_REGISTRY_OPTIMIZER", 0, 0, true, SECONDLEVEL);
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RESUME_REGISTRY_OPTIMIZER:
			dwReply = 0;
			if (g_iSpyRegOpt.ResumeRegistryOptimizer())
			{
				dwReply = 1;
			}
			else
			{
				AddLogEntry(L"### Failed in WardwizComSrv::OnDataReceiveCallBack to RESUME_REGISTRY_OPTIMIZER", 0, 0, true, SECONDLEVEL);
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_SIGNATURE_DATABASE:
			dwReply = 0;
			ReLoadSignatures();
			dwReply = 1;
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;

		case DATA_ENC_DEC_OPERATIONS:
			//Added By Nitin K.
			//For new implementation of Encrypt/Decrypt
			dwReply = 0;
			if (g_objWardWizDataCrypt.LaunchDataEncDecExe(lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->dwValue, lpSpyData->dwSecondValue))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SCAN_FILE:
			dwReply = 0;
			if (ScanFile(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SAVE_LOCALDB:
			dwReply = 0;
			if (SaveLocalDBFiles())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case HANDLEACTICESCANSETTINGS:
			dwReply = 0;
			if (HandleActiveScanSettings(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case APPLYPRODUCTSETTINGS:
			dwReply = 0;
			if (HandleProductSettings(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_REGISTARTION_DAYS:
			dwReply = 0;
			if (ReloadRegistrationDays())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case START_STOP_INDEXING:
			dwReply = 0;
			if (StartStopIndexing(lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
		case CLEAR_INDEXING:
			dwReply = 0;
			if (ClearIndexing())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case BOOT_SCANNER_OPR:
			if (lpSpyData->dwValue == 0x01)
			{
				if (!AddBootScannerEntry(SESSION_MANAGER_REG, BOOT_EXEC_REG, lpSpyData->szFirstParam))
				{
					AddLogEntry(L"### Failed to Set Boot scanner entry in registry, Path:[%s]", lpSpyData->szFirstParam, 0, true, SECONDLEVEL);
				}
			}
			else if (lpSpyData->dwValue == 0x02)
			{
				if (!RemoveBootScannerEntry(SESSION_MANAGER_REG, BOOT_EXEC_REG))
				{
					AddLogEntry(L"### Failed to remove Boot scanner entry from registry, Path:[%s]", lpSpyData->szFirstParam, 0, true, SECONDLEVEL);
				}
			}
			else
			{
				AddLogEntry(L"### Invalid parameters BOOT_SCANNER_OPR", 0, 0, true, SECONDLEVEL);
			}
			dwReply = 1;
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SCHEDULED_SCAN_CHANGE:
			dwReply = 0;
			if (SetEvent(hSchScanEvent) != 0)
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case LAUNCH_APP_IN_USER_CONTEXT:
			dwReply = 0;
			if (LauchAppInUserContext(lpSpyData))
			{
				dwReply = 1;
		}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case LAUNCH_APPLICATION:
			dwReply = 0;
			if (LauchApplication(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SHOW_PC_LOCK_WND:
			dwReply = 0;
			if (SetEvent(hParCtrlEvent) != 0)
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_APPLICATION_RULES:
			dwReply = 0;
			if (ReloadApplicationRules(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ON_INET_RESTRICTION:
			dwReply = 0;
			if (SetEvent(hPCtrlINetEvent) != 0)
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ON_PC_INET_RESTRICT:
			dwReply = 0;
			if (SetEvent(hPCtrlINetUsgEvent) != 0)
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_APP_RESTRICTION:
			dwReply = 0;
			if (ReloadApplicationRestriction())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_PACKET_RULES:
			dwReply = 0;
			if (m_dwDaysLeft != 0x00)
			{
				if (g_objFWClient.ReLoadFirewallRuleList())
				{
					dwReply = 1;
				}
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_PARENTAL_CONTROL_SETTINGS:
			dwReply = 0;
			if (ReLoadParentalControlSettings(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_EMAIL_SCAN_SETTINGS:
			dwReply = 0;
			if (ReLoadEmailScanSettings(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_FIREWALL_SETTINGS:
			dwReply = 0;
			if (ReLoadFWControlSettings(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case LAUNCH_APP_USING_SERVICE:
			dwReply = 0;
			if (LaunchApplicationUsingService(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ON_QUARANTNE_RECOVER:
			dwReply = 0;
			if (OnFileQuarantineFromRecover(lpSpyData->szFirstParam))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_WEB_SEC_DB:
			dwReply = 0;
			if (ReloadWebSecDB())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_BROWSE_PROT_DB:
			dwReply = 0;
			if (ReloadBrowseProtDB())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_USER_LIST:
			dwReply = 0;
			if (OnReloadUserList())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_BLOCK_SPEC_WEB_PAGE:
			dwReply = 0;
			if (ReloadBlkSpecWebDB())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_MNG_EXCLUSION:
			dwReply = 0;
			if (ReloadMngExcDB())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_USER_ACESS_LIST:
			dwReply = 0;
			if (ReloadUserAccessList())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ON_PC_INITIALISE:
			dwReply = 0;
			if (OnParCtrlInitialise())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ON_PC_UNINITIALISE:
			dwReply = 0;
			if (OnParCtrlUnInitialise())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_BROWSER_SEC:
			dwReply = 0;
			if (OnReloadBrowserSecurity())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_BROWSER_SEC_EXC:
			dwReply = 0;
			if (OnReloadBrowserSecurityExc())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_BROWSER_SEC_SPEC:
			dwReply = 0;
			if (OnReloadBrowserSecuritySpec())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_BROWSER_SECURITY:
			dwReply = 0;
			if (OnReloadBrowserSecurityVal())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ON_RESET_RESTRICTION:
			dwReply = 0;
			if (OnParCtrlResetValue())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ON_BROWSER_SECURITY_UNINITIALIZE:
			dwReply = 0;
			if (OnParBrowserSecUninit())
			{
				dwReply = 1;
		}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case ON_CHECK_INTERNET_ACCESS:
			dwReply = 0;
			if (OnCheckInternetAccess())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case UPDATE_REGISTRATION:
			dwReply = 0;
			if (UpdateRegistrationDetails2Server())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizComSrv OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
  Function Name  : WriteNumberOfDays
  Description    : Function which gets number of days & writes Machine ID into registry.
  SR.NO			 : WRDWIZCOMMSRV_51
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
void WriteNumberOfDays()
{
	__try
	{
		typedef DWORD (*GETDAYSLEFT_SERVICE)( DWORD ) ;

		GETDAYSLEFT_SERVICE	GetDaysLeft_Service = NULL ;

		TCHAR	szTemp[512] = {0} ;

		GetModuleFileName( NULL, szTemp, 511 ) ;

		TCHAR	*pTemp = wcsrchr(szTemp, '\\' ) ;
		if( pTemp )
		{
			pTemp++ ;
			*pTemp = '\0' ;
		}

		wcscat(szTemp, L"VBREGISTRATION.DLL" ) ;

		HMODULE	hMod = LoadLibrary( szTemp ) ;

		if( hMod )
			GetDaysLeft_Service = (GETDAYSLEFT_SERVICE ) GetProcAddress( hMod, "GetDaysLeft_Service" ) ;

		if( GetDaysLeft_Service )
			GetDaysLeft_Service(GetProductID()) ;

		if( hMod )
		{
			FreeLibrary( hMod ) ;
			hMod = NULL ;
		}

		//Commented as because this is duplicate call
		//Write here the machine ID in registry for further use.
		//if(!CheckForMIDInRegistry())
		//{
		//	if(!WriteMachineID2Registry())
		//	{
		//		AddLogEntry(L"### Failed to WriteMachineID2Registry", 0, 0, true, SECONDLEVEL);
		//	}
		//}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WriteNumberOfDays", 0, 0, true, SECONDLEVEL);
	}
}
/***********************************************************************************************
  Function Name  : SrvPrimaryThread
  Description    : This is Service primary thread which is gets called from StartServiceWorkerThread
  SR.NO			 : WRDWIZCOMMSRV_52
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD SrvPrimaryThread(LPVOID lpParam)
{
	__try
	{
		return SrvPrimaryThreadSEH(lpParam);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SrvPrimaryThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
  Function Name  : SrvPrimaryThreadSEH
  Description    : This is Service primary thread which is gets called from StartServiceWorkerThread
  SR.NO			 : WRDWIZCOMMSRV_52
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD SrvPrimaryThreadSEH( LPVOID lpParam )
{
	try
	{
		g_objCommServer.Run(true, false);
		hSchScanEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hParCtrlEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hPCtrlINetEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hPCtrlINetUsgEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hSendRegInfoEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		//Get here registry setting for Active scanning.
		DWORD dwActiveScanOption = 0x01;
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwActiveScanOption", dwActiveScanOption) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwActiveScanOption in SrvPrimaryThread KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		//Mark the variable as disabled if user selected as permanately disable.
		if (dwActiveScanOption == 0x00)
		{
			g_bISActiveScanDisabled = true;
		}
		else
		{
			if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwActiveScanOption", 0x01) != 0x00)
			{
				AddLogEntry(L"### Failed to Set Registry Entry for dwActiveScanOption in SrvPrimaryThread KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);;
			}
		}

		//Get here registry setting for Active scanning.
		DWORD dwQuarantineOption = 0x00;
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwQuarantineOption", dwQuarantineOption) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwQuarantineOption in SrvPrimaryThread KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		if (!RemoveBootScannerEntry(SESSION_MANAGER_REG, BOOT_EXEC_REG))
		{
			AddLogEntry(L"### Failed to remove Boot scanner entry from registry", 0, 0, true, SECONDLEVEL);
		}

		g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwPerfLevel", g_dwPerfLevel);

		//Set here autoQuarentine option
		g_objISpyScannerBase.m_objWardWizScanner.SetAutoQuarentineOption(dwQuarantineOption);

		//Create here required Directories.
		CreateRequiredDirs();

		//CheckScanLevel();
		//Ram: This call should be performed before any operation.
		g_objISpyScannerBase.SetManipulationPtr(&g_objISpyDBManipulation);

		//read the boot time entries to merge into recover DB.
		ReadBootScannedEntries();

		//Need to have Sleep because need to start service on priority
		//WardWizALU service has to be started before communication service
		//it may fail if communication service started earlier than ALU service which loads
		//registration DLL and if ALU service tries to replace, will fail.
		//Sleep(10 * 1000);

		 // issue resolved by lalit kumwat 1-14-2015 In the WardWiz Live Update popup, 
		 //it again continuously displays popup message box 
		 WriteNumberOfDays() ;

		 Sleep(1000 * 5); //sleep needed for launch @startup.

		 Check4StartUpEntries();

		 //Get here required system information and write in INI
		 AddSystemDetailsToINI();

		//Get here the saved entries
		 CString csFilePath = GetWardWizPathFromRegistry();
		 csFilePath += L"VBACTIVESCAN.DB";
		 
		 LoadActiveScanEntryFromFile(g_onAccessQueue, csFilePath.GetBuffer());

		//If loads successfully then delete the file.
		SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(csFilePath);

		 Sleep( 600 * 1000 ) ;

		//StopServiceByEvent() ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in SrvPrimaryThreadSEH", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

/***********************************************************************************************
  Function Name  : InstallService
  Description    : Function which install the service in SCM.
  SR.NO			 : WRDWIZCOMMSRV_1
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD InstallService( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName)
{
	SC_HANDLE				hService	= NULL ;
    SC_HANDLE				hSCManager	= NULL ;
	SERVICE_DESCRIPTION		SrvDesc		= {L"Coordinates between other VIBRANIUM applications for multiple operations."} ;

	TCHAR	szSrvPath[512] = {0} ;
	DWORD	dwRet = 0x00, dwLastError=0x00 ;

	DWORD	dwSrvStartType = 0xFFFFFFFF ;

	__try
	{
		if( !GetModuleFileName( NULL, szSrvPath, 511 ) )
		{
			dwRet = 0x01 ;
			//__leave ;
			goto Cleanup;
		}

		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( !hSCManager )
		{
			AddLogEntry(L"OpenSCManager Failed, Service Name :%s", pszSrvDisplayName, 0, true, SECONDLEVEL);
			dwRet = 0x02 ;
			//__leave ;
			goto Cleanup;
		}

		hService = CreateService(	hSCManager, pszSrvName, pszSrvDisplayName, SERVICE_ALL_ACCESS,
									SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
									SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
									szSrvPath, NULL, NULL, L"", NULL, NULL) ;

		dwLastError = GetLastError() ;
		if( !hService )
		{
			hService = OpenService( hSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
			if( !hService )
			{
				dwRet = 0x03 ;
				//__leave ;
				goto Cleanup;
			}
		}

		if( dwLastError != ERROR_SERVICE_EXISTS )
		{
			ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &SrvDesc ) ;
			SetServiceFailureAction( hService ) ;
		}

		StartService( hService, 0, NULL ) ;
		Sleep( 100 ) ;

		QueryServiceStatus(hService, &dwSrvStartType ) ;

		if( (dwSrvStartType == SERVICE_DISABLED) ||
			(dwSrvStartType == SERVICE_DEMAND_START) )
		{
			if( !ChangeServiceConfig(	hService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
										SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
										NULL, NULL, NULL ) )
			{
				dwRet = 0x04 ;
				//__leave ;
				goto Cleanup;
			}

			StartService( hService, 0, NULL ) ;
			Sleep( 100 ) ;
		}

		dwSrvStartType = 0xFFFFFFFF ;
		QueryServiceStartStatus( hService, &dwSrvStartType ) ;

		if( (dwSrvStartType == SERVICE_RUNNING) ||
			(dwSrvStartType == SERVICE_START_PENDING) )
		{
			dwRet = 0x00 ;
			//__leave ;
			goto Cleanup;
		}

		if( (dwSrvStartType == SERVICE_STOPPED) || (dwSrvStartType == SERVICE_STOP_PENDING) ||
			(dwSrvStartType == SERVICE_PAUSED) || (dwSrvStartType == SERVICE_PAUSE_PENDING) ||
			(dwSrvStartType == SERVICE_CONTINUE_PENDING) )
		{
			Sleep( 100 ) ;
			StartService( hService, 0, NULL ) ;
			Sleep( 100 ) ;
		}

		dwSrvStartType = 0xFFFFFFFF ;
		QueryServiceStartStatus( hService, &dwSrvStartType ) ;
		if( (dwSrvStartType == SERVICE_RUNNING) ||
			(dwSrvStartType == SERVICE_START_PENDING) )
		{
			dwRet = 0x00 ;
			//__leave ;
			goto Cleanup;
		}

		dwRet = 0x05 ;
	}
	//__finally
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{

		/*if( hService )
		{
			CloseServiceHandle( hService ) ;
			hService = NULL ;
		}

		if( hSCManager )
		{
			CloseServiceHandle( hSCManager ) ;
			hSCManager = NULL ;
		}*/

	}

Cleanup:

	if( hService )
	{
		CloseServiceHandle( hService ) ;
		hService = NULL ;
	}

	if( hSCManager )
	{
		CloseServiceHandle( hSCManager ) ;
		hSCManager = NULL ;
	}
	return dwRet ;

}

/***********************************************************************************************
  Function Name  : UnInstallService
  Description    : Function which uninstalls the service from SCM.
  SR.NO			 : WRDWIZCOMMSRV_2
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD UnInstallService( TCHAR *pszSrvName )
{
	DWORD	dwRetValue = 0x00 ;


	SC_HANDLE	schSCManager = NULL ;
    SC_HANDLE	schService = NULL ;
	DWORD		dwServiceStartType = 0xFFFFFFFF ;

	//TCHAR		szTemp[256] = {0} ;

	__try
	{

		schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( schSCManager == NULL )
		{
			dwRetValue = 0x01 ;
			//__leave ;
			goto Cleanup;
		}

		schService = OpenService( schSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
		if( !schService )
		{
			dwRetValue = 0x02 ;
			//__leave ;
			goto Cleanup;
		}

		Sleep( 100 ) ;

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;

		//sprintf(szTemp, "ControlService::%lu", ServiceStatus.dwCurrentState ) ;
		//WriteLog( szTemp ) ;
		if( ServiceStatus.dwCurrentState==SERVICE_RUNNING )
		{
			UpdateServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0) ;
			Sleep( 1000 ) ;
		}

		if( ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING )
			Sleep( 1000 ) ;

		//WriteLog( "SERVICE_CONTROL_STOP called" ) ;

		if( !DeleteService( schService ) )
		{
			if( QueryServiceStatus( schService, &dwServiceStartType ) )
			{
				dwRetValue = 0x03 ;
				//__leave ;
				goto Cleanup;
			}

			if( dwServiceStartType == SERVICE_DISABLED ||
				(dwServiceStartType == SERVICE_DEMAND_START) )
			{
				if( !ChangeServiceConfig(	schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
											SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
											NULL, NULL, NULL ) )
				{
					dwRetValue = 0x04 ;
					//__leave ;
					goto Cleanup;
				}

				Sleep( 100 ) ;
				ControlService( schSCManager, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
				if( !DeleteService( schService ) )
				{
					dwRetValue = 0x05 ;
					//__leave ;
					goto Cleanup;
				}
			}
		}

		//WriteLog( "DeleteService() done" ) ;
	}
	//__finally
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		/*if( schService )
		{
			CloseServiceHandle(schService) ;
			schService = NULL ;
		}

		if( schSCManager )
		{
			CloseServiceHandle(schSCManager) ;
			schSCManager = NULL ;
		}*/

	}

Cleanup:

	if( schService )
	{
		CloseServiceHandle(schService) ;
		schService = NULL ;
	}

	if( schSCManager )
	{
		CloseServiceHandle(schSCManager) ;
		schSCManager = NULL ;
	}

	return dwRetValue ;
}

/***********************************************************************************************
  Function Name  : StartServiceManually
  Description    : Function which start the service which is already installed.
  SR.NO			 : WRDWIZCOMMSRV_3
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD StartServiceManually( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName )
{
	DWORD		dwRet = 0x00 ;

	SC_HANDLE	schSCManager = NULL ;
	SC_HANDLE	schService = NULL ;

	DWORD		dwSrvStartType = 0xFFFFFFFF ;
	DWORD		dwLastError = 0x00 ;

	__try
	{
		AddLogEntry(L">>> Start COMM service", 0, 0, true, SECONDLEVEL);
		schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( schSCManager == NULL )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		schService = OpenService( schSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
		dwLastError = GetLastError() ;
		if( dwLastError == ERROR_SERVICE_DOES_NOT_EXIST )
		{
			dwRet = InstallService( pszSrvName, pszSrvDisplayName ) ;
			goto Cleanup ;
		}

		if( !schService )
		{
			Sleep( 100 ) ;
			schService = OpenService(	schSCManager, pszSrvName, 
										SERVICE_START|SERVICE_QUERY_CONFIG|SERVICE_CHANGE_CONFIG ) ;
		}

		if( !schService )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		QueryServiceStatus(schService, &dwSrvStartType ) ;

		if( (dwSrvStartType == SERVICE_DISABLED) ||
			(dwSrvStartType == SERVICE_DEMAND_START) )
		{
			if( !ChangeServiceConfig(	schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
										SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
										NULL, NULL, NULL ) )
			{
				dwRet = 0x03 ;
				goto Cleanup ;
			}

			StartService( schService, 0, NULL ) ;
			Sleep( 100 ) ;
		}

		dwSrvStartType = 0xFFFFFFFF ;
		QueryServiceStartStatus( schService, &dwSrvStartType ) ;

		if( (dwSrvStartType == SERVICE_RUNNING) ||
			(dwSrvStartType == SERVICE_START_PENDING) )
		{
			dwRet = 0x00 ;
			goto Cleanup ;
		}

		if( (dwSrvStartType == SERVICE_STOPPED) || (dwSrvStartType == SERVICE_STOP_PENDING) ||
			(dwSrvStartType == SERVICE_PAUSED) || (dwSrvStartType == SERVICE_PAUSE_PENDING) ||
			(dwSrvStartType == SERVICE_CONTINUE_PENDING) )
		{
			Sleep( 100 ) ;
			StartService( schService, 0, NULL ) ;
			Sleep( 100 ) ;
		}

		dwSrvStartType = 0xFFFFFFFF ;
		QueryServiceStartStatus( schService, &dwSrvStartType ) ;
		if( (dwSrvStartType == SERVICE_RUNNING) ||
			(dwSrvStartType == SERVICE_START_PENDING) )
		{
			dwRet = 0x00 ;
			goto Cleanup ;
		}

		dwRet = 0x04 ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{

	}

Cleanup:


	if( schService )
	{
		CloseServiceHandle(schService) ;
		schService = NULL ;
	}

	if( schSCManager )
	{
		CloseServiceHandle(schSCManager) ;
		schSCManager = NULL ;
	}


	return dwRet ;
}

/***********************************************************************************************
  Function Name  : StopServiceManually
  Description    : Function which stops the service.
  SR.NO			 : WRDWIZCOMMSRV_3
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD StopServiceManually( TCHAR *pszSrvName )
{
	DWORD	dwRetValue = 0x00 ;


	SC_HANDLE	schSCManager = NULL ;
    SC_HANDLE	schService = NULL ;
	DWORD		dwServiceStartType = 0xFFFFFFFF ;

	SERVICE_STATUS	ServiceStatus = {0} ;

	__try
	{

		schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( schSCManager == NULL )
		{
			dwRetValue = 0x01 ;
			//__leave ;
			goto Cleanup;
		}

		schService = OpenService( schSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
		if( !schService )
		{
			dwRetValue = 0x02 ;
			//__leave ;
			goto Cleanup;
		}

		Sleep( 100 ) ;

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
		if( (ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) ||
			(ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) )
		{
			Sleep( 1000 ) ;
			//__leave ;
			goto Cleanup;
		}

		//WriteLog( "SERVICE_CONTROL_STOP called" ) ;

		if( QueryServiceStatus( schService, &dwServiceStartType ) )
		{
			dwRetValue = 0x03 ;
			//__leave ;
			goto Cleanup;
		}

		if( dwServiceStartType == SERVICE_DISABLED ||
			(dwServiceStartType == SERVICE_DEMAND_START) )
		{
			if( !ChangeServiceConfig(	schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
										SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
										NULL, NULL, NULL ) )
			{
				dwRetValue = 0x04 ;
				//__leave ;
				goto Cleanup;
			}

			Sleep( 100 ) ;
			if( !ControlService( schSCManager, SERVICE_CONTROL_STOP, &ServiceStatus ) )
			{
				dwRetValue = 0x05 ;
				//__leave ;
				goto Cleanup;
			}
		}

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
		if( (ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) ||
			(ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) )
		{
			dwRetValue = 0x00 ;
			//__leave ;
			goto Cleanup;
		}

		dwRetValue = 0x06 ;
	}
	//__finally
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup:

	if( schService )
	{
		CloseServiceHandle(schService) ;
		schService = NULL ;
	}

	if( schSCManager )
	{
		CloseServiceHandle(schSCManager) ;
		schSCManager = NULL ;
	}

	return dwRetValue ;
}

/***********************************************************************************************
  Function Name  : QueryServiceStatus
  Description    : Function which Queries status from Windows SCM.
  SR.NO			 : WRDWIZCOMMSRV_53
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD QueryServiceStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus)
{
	LPQUERY_SERVICE_CONFIG	lpsc = NULL ;

	DWORD					dwStatus = 0x00 ;
	DWORD					dwBytesNeeded = 0x00, dwSize = 0x00 ;

	__try
	{

		if( hService == NULL )
		{
			dwStatus = 0x01 ;
			//__leave ;
			goto Cleanup;
		}

		QueryServiceConfig( hService, NULL, 0, &dwBytesNeeded ) ;
		if( !dwBytesNeeded )
		{
			dwStatus = 0x02 ;
			//__leave ;
			goto Cleanup;
		}

		dwSize = dwBytesNeeded ;

		lpsc = (LPQUERY_SERVICE_CONFIG ) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT, dwSize ) ;
		if( !lpsc )
		{
			dwStatus = 0x03 ;
			//__leave ;
			goto Cleanup;
		}

		if( !QueryServiceConfig(hService, lpsc, dwSize, &dwBytesNeeded ) )
		{
			dwStatus = 0x04 ;
			//__leave ;
			goto Cleanup;
		}

		*lpdwServiceStatus = lpsc->dwStartType ;
	}
	//__finally
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup:

	if( lpsc )
	{
		LocalFree( lpsc ) ;
		lpsc = NULL ;
	}

	return dwStatus ;
}

/***********************************************************************************************
  Function Name  : QueryServiceStartStatus
  Description    : Function which Queries status from Windows SCM.
  SR.NO			 : WRDWIZCOMMSRV_53
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD QueryServiceStartStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus)
{
	SERVICE_STATUS_PROCESS	ssStatus = {0} ;

	DWORD					dwStatus = 0x00 ;
	DWORD					dwBytesNeeded = 0x00, dwSize = 0x00 ;

	__try
	{

		if( hService == NULL )
		{
			dwStatus = 0x01 ;
			//__leave ;
			goto Cleanup;
		}

		if( !QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssStatus,
									sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
		{
			Sleep( 100 ) ;
			if( !QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssStatus,
									sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
			{
				dwStatus = 0x02 ;
				//__leave ;
				goto Cleanup;
			}
		}

		*lpdwServiceStatus = ssStatus.dwCurrentState ;
	}
	//__finally
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup:

	return dwStatus ;
}

/***********************************************************************************************
  Function Name  : SetServiceFailureAction
  Description    : Function which sets the failure actions, the actions sets is to RESTART the service
				   with any failure.
  SR.NO			 : WRDWIZCOMMSRV_55
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD SetServiceFailureAction( SC_HANDLE hService )
{
	DWORD	dwRet = 0x00 ;

	SERVICE_FAILURE_ACTIONS srvFailActions = {0} ;
	SC_ACTION				failActions[3] ;

	__try
	{

		if( hService == NULL )
		{
			dwRet = 0x01 ;
			//__leave ;
			goto Cleanup;
		}

		failActions[0].Type = SC_ACTION_RESTART ;
		failActions[0].Delay = 0;
		failActions[1].Type = SC_ACTION_RESTART ;
		failActions[1].Delay = 0;
		failActions[2].Type = SC_ACTION_RESTART ;
		failActions[2].Delay = 0;

		srvFailActions.dwResetPeriod = 86400;
		srvFailActions.lpCommand = NULL;
		srvFailActions.lpRebootMsg = NULL;
		srvFailActions.cActions = 3;
		srvFailActions.lpsaActions = failActions;

		ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &srvFailActions ) ;

	}
	//__finally
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup:

	return dwRet ;
}

/***********************************************************************************************
  Function Name  : ReadDeleteReportsValueFromRegistry
  Description    : Function which read the registry settings given by user for reports deletion.
  SR.NO			 : WRDWIZCOMMSRV_56
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD ReadDeleteReportsValueFromRegistry()
{
	HKEY key;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
		return 0;
	DWORD dwType = REG_DWORD;
	DWORD dwValue = 0x0;
	DWORD dwValueLen = sizeof(dwValue);
	int iValue = 0;
	long lResult = RegQueryValueEx(key, L"dwDaysToDelRep", NULL ,&dwType,(LPBYTE)&dwValue, &dwValueLen);
	if(lResult == ERROR_SUCCESS)
	{
		RegCloseKey(key);
		return dwValue;
	}
	else
	{
		RegCloseKey(key);
		return 0;
	}
}

/***********************************************************************************************
  Function Name  : DeleteOldReports
  Description    : Function which Deletes old reports as per users setting.
  SR.NO			 : WRDWIZCOMMSRV_57
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
bool DeleteOldReports(DWORD dwDays, DWORD dwDBType)
{
	if(!dwDays)
	{
		return false;
	}

	//This check is for wardwiz Pro version, will do it later
	if (dwDBType == 0x01)
	{
		return false;
	}

	try
	{
		char *szDeleteSpan = "-30 days";//Default 30 days
		switch (dwDays)
		{
		case 0x1E:
			szDeleteSpan = "-30 days";
			break;
		case 0x28:
			szDeleteSpan = "-40 days";
			break;
		case 0x3C:
			szDeleteSpan = "-60 days";
			break;
		}

		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);

		//If file does not exists, then return.
		if (!PathFileExists(csWardWizReportsPath))
		{
			return false;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);

		CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
		objSqlDb.Open();

		//============================================= Autorun scanner details =============================================
		char szAutorunScanDetailsQuery[MAXQUERYLENGTH] = { 0 };
		sprintf(szAutorunScanDetailsQuery, "%s%s%s",
			"DELETE FROM Wardwiz_AutorunScanDetails\
			WHERE[db_ScanSessionID] IN\
			(\
			SELECT  ASD.[db_ScanSessionID]\
			FROM Wardwiz_AutorunScanSessionDetails ASD\
			WHERE Date(ASD.[db_ScanSessionStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szAutorunScanDetailsQuery);
		objSqlDb.ExecDML(szAutorunScanDetailsQuery);
		Sleep(10);

		char szAutorunScanSessionDetailsQuery[MAXQUERYLENGTH] = { 0 };
		sprintf(szAutorunScanSessionDetailsQuery, "%s%s%s",
			"DELETE FROM Wardwiz_AutorunScanSessionDetails WHERE [db_ScanSessionID] IN(\
			 SELECT  ASD.[db_ScanSessionID]\
			 FROM Wardwiz_AutorunScanSessionDetails ASD\
			WHERE Date(ASD.[db_ScanSessionStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szAutorunScanSessionDetailsQuery);
		objSqlDb.ExecDML(szAutorunScanSessionDetailsQuery);
		Sleep(10);
		//==============================================================================================================================

		//============================================= Registry optimizer =============================================
		char szRegistryOptimizerDetailsQuery[MAXQUERYLENGTH] = { 0 };
		sprintf(szRegistryOptimizerDetailsQuery, "%s%s%s",
			"DELETE FROM Wardwiz_RegistryOptimizerDetails\
			 WHERE[db_RegistryScanId] IN\
			 (\
			 SELECT  ROD.[db_RegistryScanId]\
			 FROM[Wardwiz_RegistryOptimizerDetails] ROD\
			 WHERE Date(ROD.[db_ScanStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szRegistryOptimizerDetailsQuery);
		objSqlDb.ExecDML(szRegistryOptimizerDetailsQuery);

		Sleep(10);
		//==============================================================================================================================

		//============================================= Scan details tables =============================================
		char szWardwiz_ScanDetails[MAXQUERYLENGTH] = { 0 };
		sprintf(szWardwiz_ScanDetails, "%s%s%s",
			"DELETE FROM[Wardwiz_ScanDetails]\
			WHERE[db_ScanSessionID] IN\
			(\
			SELECT  SSD.[db_ScanSessionID]\
			FROM Wardwiz_ScanSessionDetails SSD\
			WHERE Date(SSD.[db_ScanSessionStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szWardwiz_ScanDetails);
		objSqlDb.ExecDML(szWardwiz_ScanDetails);
		Sleep(10);

		char szScanSessionDetails[MAXQUERYLENGTH] = { 0 };
		sprintf(szScanSessionDetails, "%s%s%s",
			"DELETE FROM[Wardwiz_ScanSessionDetails]\
			 WHERE[db_ScanSessionID] IN\
			 (\
			 SELECT  SSD.[db_ScanSessionID]\
			 FROM Wardwiz_ScanSessionDetails SSD\
			 WHERE Date(SSD.[db_ScanSessionStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szScanSessionDetails);
		objSqlDb.ExecDML(szScanSessionDetails);
		Sleep(10);
		//==============================================================================================================================

		//============================================= Temp file cleaner details tables..=============================================
		char szWardwiz_TempFilesCleanerDetails[MAXQUERYLENGTH] = { 0 };
		sprintf(szWardwiz_TempFilesCleanerDetails, "%s%s%s",
			"DELETE FROM [Wardwiz_TempFilesCleanerDetails] WHERE [db_TempCleanerSessionId] IN(\
			SELECT  TSD.[db_TempCleanerSessionId]\
			FROM [Wardwiz_TempFilesCleanerSessionDetails] TSD\
			WHERE Date(TSD.[db_CleanerSessionStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szWardwiz_TempFilesCleanerDetails);
		objSqlDb.ExecDML(szWardwiz_TempFilesCleanerDetails);
		Sleep(10);

		char szWardwiz_TempFilesCleanerSessionDetails[MAXQUERYLENGTH] = { 0 };
		sprintf(szWardwiz_TempFilesCleanerSessionDetails, "%s%s%s",
			"DELETE FROM Wardwiz_TempFilesCleanerSessionDetails\
			WHERE [db_TempCleanerSessionId] IN\
			(\
			SELECT  TSD.[db_TempCleanerSessionId]\
			FROM [Wardwiz_TempFilesCleanerSessionDetails] TSD\
			WHERE Date(TSD.[db_CleanerSessionStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szWardwiz_TempFilesCleanerSessionDetails);
		objSqlDb.ExecDML(szWardwiz_TempFilesCleanerSessionDetails);
		Sleep(10);
		//==============================================================================================================================

		//============================================= USB Vaccinator tables..=============================================
		char szUSBVaccinatorDetails[MAXQUERYLENGTH] = { 0 };
		sprintf(szUSBVaccinatorDetails, "%s%s%s",
			"DELETE FROM [Wardwiz_USBVaccinatorDetails] WHERE [db_USBVaccinatorSessionId] IN(\
						SELECT  USD.[db_USBVaccinatorSessionId]\
						FROM [Wardwiz_USBVaccinatorSessionDetails] USD\
						WHERE Date(USD.[db_USBSessionStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szUSBVaccinatorDetails);
		objSqlDb.ExecDML(szUSBVaccinatorDetails);
		Sleep(10);

		char szUSBVaccinatorSessionDetails[MAXQUERYLENGTH] = { 0 };
		sprintf(szUSBVaccinatorSessionDetails, "%s%s%s",
			"DELETE FROM Wardwiz_USBVaccinatorSessionDetails\
			 WHERE [db_USBVaccinatorSessionId] IN\
			 (\
			 SELECT  USD.[db_USBVaccinatorSessionId]\
			 FROM [Wardwiz_USBVaccinatorSessionDetails] USD\
			 WHERE Date(USD.[db_USBSessionStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szUSBVaccinatorSessionDetails);
		objSqlDb.ExecDML(szUSBVaccinatorSessionDetails);
		Sleep(10);
		//==============================================================================================================================


		//============================================= Update master tables.=============================================
		char szWardwiz_UpdatesMaster[MAXQUERYLENGTH] = { 0 };
		sprintf(szWardwiz_UpdatesMaster, "%s%s%s",
			"DELETE FROM[Wardwiz_UpdatesMaster]\
			 WHERE [db_UpdateId] IN\
			 (\
			 SELECT  USD.[db_UpdateId]\
			 FROM [Wardwiz_UpdatesMaster] USD\
			 WHERE Date(USD.[db_UpdateStartTime]) < Date('now', '", szDeleteSpan, "'));");

		OutputDebugStringA(szWardwiz_UpdatesMaster);
		objSqlDb.ExecDML(szWardwiz_UpdatesMaster);
		Sleep(10);
		//==============================================================================================================================

		DWORD dwProdID = 0;
		dwProdID = GetProductID();
		if (dwProdID != BASIC && dwProdID != ESSENTIAL)
		{
			//============================================= Email Scan tables.=============================================
			char szWardwiz_EmailScanDetails[MAXQUERYLENGTH] = { 0 };
			sprintf(szWardwiz_EmailScanDetails, "%s%s%s",
				"DELETE FROM[Wardwiz_EmailScanDetails]\
							 WHERE [db_EmailScanID] IN\
							 			 (\
										 			 SELECT  ESD.[db_EmailScanID]\
													 			 FROM [Wardwiz_EmailScanDetails] ESD\
																 			 WHERE Date(ESD.[db_EmailTime]) < Date('now', '", szDeleteSpan, "'));");

			//OutputDebugStringA(szWardwiz_EmailScanDetails);
			objSqlDb.ExecDML(szWardwiz_EmailScanDetails);
			Sleep(10);
			//==============================================================================================================================

			//============================================= Firewall tables.=============================================
			char szWardwiz_FirewallDetails[MAXQUERYLENGTH] = { 0 };
			sprintf(szWardwiz_FirewallDetails, "%s%s%s",
				"DELETE FROM[Wardwiz_FirewallDetails]\
							 WHERE[db_FirewallID] IN\
							 			 (\
										 			 SELECT  FD.[db_FirewallID]\
													 			 FROM[Wardwiz_FirewallDetails] FD\
																 			 WHERE Date(FD.[db_FirewallTime]) < Date('now', '", szDeleteSpan, "'));");

			//OutputDebugStringA(szWardwiz_FirewallDetails);
			objSqlDb.ExecDML(szWardwiz_FirewallDetails);
			Sleep(10);
			//==============================================================================================================================

			//============================================= Parental Control tables.=============================================
			char szWardwiz_ParentalCtrl_Details[MAXQUERYLENGTH] = { 0 };
			sprintf(szWardwiz_ParentalCtrl_Details, "%s%s%s",
				"DELETE FROM[Wardwiz_ParentalCtrl_Details]\
							 WHERE[db_PCID] IN\
							 			 (\
										 			 SELECT  PD.[db_PCID]\
													 			 FROM[Wardwiz_ParentalCtrl_Details] PD\
																 			 WHERE Date(PD.[db_PCTime]) < Date('now', '", szDeleteSpan, "'));");

			//OutputDebugStringA(szWardwiz_ParentalCtrl_Details);
			objSqlDb.ExecDML(szWardwiz_ParentalCtrl_Details);
			Sleep(10);
			
			//============================================= Parental Control browsing security.=============================================
			char szWardwiz_BrowseSession[MAXQUERYLENGTH] = { 0 };
			sprintf(szWardwiz_BrowseSession, "%s%s%s",
				"DELETE FROM[Wardwiz_BrowseSession]\
							WHERE[BrowseSession_ID] IN\
							(\
							SELECT  BS.[BrowseSession_ID]\
							FROM[Wardwiz_Browse_Security] BS\
							WHERE Date(BS.[SessionStartDate]) < Date('now', '", szDeleteSpan, "'));");
			//OutputDebugStringA(szWardwiz_BrowseSession);
			objSqlDb.ExecDML(szWardwiz_BrowseSession);
			Sleep(10);

			char szWardwiz_PC_Browse_Security_Details[MAXQUERYLENGTH] = { 0 };
			sprintf(szWardwiz_PC_Browse_Security_Details, "%s%s%s",
				"DELETE FROM[Wardwiz_Browse_Security]\
											 WHERE[ID] IN\
											 (\
											 SELECT  BS.[ID]\
											 FROM[Wardwiz_Browse_Security] BS\
											 WHERE Date(BS.[PCDate]) < Date('now', '", szDeleteSpan, "'));");
			//OutputDebugStringA(szWardwiz_PC_Browse_Security_Details);
			objSqlDb.ExecDML(szWardwiz_PC_Browse_Security_Details);
			Sleep(10);
			
			//==============================================================================================================================
		}

		objSqlDb.Close();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in DeleteOldReports", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
  Function Name  : WatchReportsActionThread
  Description    : Thread function to Deletes reports
  SR.NO			 : WRDWIZCOMMSRV_58
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD WatchReportsActionThread(LPVOID lpParam)
{
	__try
	{
		//Sleep(1000 * 15);//Debugging purpose
		while(true)
		{
			DWORD dwValue = 0x0;
			dwValue = ReadDeleteReportsValueFromRegistry();
			if(!DeleteOldReports(dwValue, 5))
			{
				return 1;
			}

			//Issue : 0000196 VBVIRUSSCANEMAIL.DB is installed in C:\Program Files\WardWiz Antivirus.
			//Resolution: We dont need to create VBVIRUSSCANEMAIL.DB for Basic And Essential
			//Resolved By : Nitin K; Date 16 March 2015
			DWORD dwProdID = 0;
			dwProdID = GetProductID();
			if (!(dwProdID == 1 || dwProdID == 5))
			{
				if (!DeleteOldReports(dwValue, 1))
				{
					return 1;
				}
			}
			Sleep(24 * 1000 * 60 * 60);//24 hours wait
		}
	}
	//catch(...)
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WatchReportsActionThread", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

/***********************************************************************************************
  Function Name  : WriteMachineID2Registry
  Description    : Function which writes Machine ID into Registry.
  SR.NO			 : WRDWIZCOMMSRV_59
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
bool WriteMachineID2Registry()
{
	__try
	{
		TCHAR	szBIOSSerialNumber[0x38] = {0};		//56 bytes considered
		TCHAR	szCPUID[0x40] = {0};
		TCHAR	szClientID[0x80] = {0};

		TCHAR	szMACAddress[0x40] = {0};

		GetCPUID( szCPUID );
		if( !szCPUID[0] )
		{
			AddLogEntry(L"### Failed to WriteMachineID2Registry::CPUID", 0, 0, true, SECONDLEVEL);
			return false;
		}

		GetBIOSSerialNumberSEH( szBIOSSerialNumber ) ;
		if( !szBIOSSerialNumber[0] )
		{

//	ISSUE No : 163
//	Some customers were not getting BIOS Serial Number, So we added MotherBoard Serial Number
//	to make Unique Machine ID

			GetMotherBoardSerialNumberSEH( szBIOSSerialNumber );
			if( !szBIOSSerialNumber[0] )
			{
				//GetCPUIDEx( szBIOSSerialNumber );
				AddLogEntry(L"### Before MAC", 0, 0, true, SECONDLEVEL);
				//GetMACAddress(szBIOSSerialNumber);
				GetMACAddress(szMACAddress);
				AddLogEntry(L"### After MAC", 0, 0, true, SECONDLEVEL);

				//GetVGAAdapterID( szBIOSSerialNumber );
				//if( !szBIOSSerialNumber[0] )		Changed on 21 10 2014 by Vilas
				if( !szMACAddress[0] )
				{
					//GetMACAddress(szBIOSSerialNumber);
					AddLogEntry(L"### Before VGA", 0, 0, true, SECONDLEVEL);
					GetVGAAdapterID( szBIOSSerialNumber );
					if( !szBIOSSerialNumber[0] )
					{
						AddLogEntry(L"### Failed to retrieve VGA ID", 0, 0, true, SECONDLEVEL);
						return false;
					}

					AddLogEntry(L"### After VGA", 0, 0, true, SECONDLEVEL);
				}

			}
		}

		if( szBIOSSerialNumber[0] )
		{
			RemoveCharsIfExists(szBIOSSerialNumber, static_cast<int>(wcslen(szBIOSSerialNumber)), static_cast<int>(sizeof(szBIOSSerialNumber)), 0x20);
		}

		if( !szMACAddress[0] )
			GetMACAddress(szMACAddress);

		int	i = static_cast<int>(wcslen( szCPUID ));

		szClientID[0] = (TCHAR) i ;
		//wsprintf( &szClientID[1], L"%s%s", szCPUID, szBIOSSerialNumber ) ;
		swprintf(&szClientID[1], 0x7E, L"%s%s%s", szCPUID, szBIOSSerialNumber, szMACAddress ) ;
		//StringCbPrintf
		//Added to terminate MID 
		i = static_cast<int>(wcslen(szClientID));
		if( i > 0x7F )
			i = 0x7F;

		szClientID[ i ] = '\0';

		if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"MVersion", szClientID) != 0)
		{
			AddLogEntry(L"### Error in GetRegistryValueData CRegistrationSecondDlg::CheckForMachineID", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	//catch(...)
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WriteMachineID2Registry", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
  Function Name  : GetCPUID
  Description    : Function which get the CPU from hardware ID.
  SR.NO			 : WRDWIZCOMMSRV_60
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD GetCPUID( TCHAR *pszCPUID )
{
	__try
	{
		TCHAR	szData[0x10] = {0} ;
		int		b[4] = {0} ;

		wcscpy( pszCPUID, L"") ;

		for (int a = 0; a < 3; a++)
		{
			__cpuid(b, a) ;

			if( (a == 0 || a == 1) && b[0] )
			{
				wsprintf(szData, L"%X", b[0] ) ;
				//i = wcslen( szTemp ) ;
				wcscat(pszCPUID, szData) ;
			}

			if( a == 2 )
			{
				if( b[0] )
				{
					wsprintf(szData, L"%X", b[0] ) ;
					wcscat(pszCPUID, szData) ;
				}

				if( b[1] )
				{
					wsprintf(szData, L"%X", b[1] ) ;
					wcscat(pszCPUID, szData) ;
				}

				if( b[2] )
				{
					wsprintf(szData, L"%X", b[2] ) ;
					wcscat(pszCPUID, szData) ;
				}

				if( b[3] )
				{
					wsprintf(szData, L"%X", b[3] ) ;
					wcscat(pszCPUID, szData) ;
				}
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetCPUID", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

/***************************************************************************************************                    
*  Function Name  : GetMotherBoardSerialNumberSEH()
*  Description    : Gets MotherBoard number through WMI
*  Author Name    : Vilas                                                                                      *
*  Date			  :	08- Sept-2014 - 12 jul -2014
*  Modified Date  :	
****************************************************************************************************/
DWORD GetMotherBoardSerialNumberSEH(TCHAR *psMotherBoardSerialNumber )
{
	__try
	{
		return GetMotherBoardSerialNumber(psMotherBoardSerialNumber);	
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		return 0x01 ;
	}

	return 0x02 ;
}

DWORD GetMotherBoardSerialNumber(TCHAR *psMotherBoardSerialNumber )
{
	//AddLogEntry(L">>> Inside GetBIOSSerialNumber");

	DWORD				dwRet = 0x00 ;

	HRESULT				hres = S_OK ;
	HRESULT				hr = S_OK ;
	IWbemLocator		*pLoc = NULL ;
	IWbemServices		*pSvc = NULL ;
	IEnumWbemClassObject* pEnumerator = NULL ;
	IWbemClassObject	*pclsObj = NULL ;
	ULONG uReturn		= 0 ;

	VARIANT				vtProp ;
	CString				hh = L"" ;

	try
	{
		//static bool			g_bCoInitializeSecurityCalled = false;

		//AddLogEntry(L">>> in GetBIOSSerialNumber::CoInitializeEx before");
/*
		hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
		if( hres != S_OK )
		{
			AddLogEntry(L"### Exception in GetBIOSSerialNumber::CoInitializeEx");
			//ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
		}
*/
		if(!g_bCoInitializeSecurityCalled)
		{
			hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
			//if( hres != S_OK )
			//{
			//	AddLogEntry(L"### Exception in GetBIOSSerialNumber::CoInitializeEx");
			//	//ErrorDescription(hres);
			//	dwRet = 0x01 ;
			//	goto Cleanup ;
			//}

			//AddLogEntry(L">>>> in before GetBIOSSerialNumber::CoInitializeSecurity ");
			hres = CoInitializeSecurity(NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE,NULL) ;
			//if( hres != S_OK )
			//{
			//	AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::CoInitializeSecurity");
			//	//ErrorDescription(hres);
			//	dwRet = 0x01 ;
			//	goto Cleanup ;
			//}

			g_bCoInitializeSecurityCalled = true;
		}

		//AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::CoCreateInstance");
		hres = CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator, (LPVOID *) &pLoc);
		if( hres != S_OK )
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::CoCreateInstance");
			//ErrorDescription(hres);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if( !pLoc )
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::Locator failed");
			//ErrorDescription(hres);
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::ConnectServer");
		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),NULL,NULL,0,NULL,0,0, &pSvc);
		if( hres != S_OK )
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ConnectServer");
			//ErrorDescription(hres);
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		if( !pSvc )
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::WebServicce failed");
			//ErrorDescription(hres);
			dwRet = 0x05 ;
			goto Cleanup ;
		}

		AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::CoSetProxyBlanket");
		hres = CoSetProxyBlanket(pSvc,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE);
		if( hres != S_OK )
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ConnectServer::CoSetProxyBlanket");
			//ErrorDescription(hres);
			dwRet = 0x06 ;
			goto Cleanup ;
		}

		//AddLogEntry(L">>>> in before GetBIOSSerialNumber::ExecQuery");
		hres = pSvc->ExecQuery(bstr_t("WQL"),bstr_t("SELECT * FROM Win32_BaseBoard"),WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,NULL,&pEnumerator);
		if( hres != S_OK )
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ExecQuery");
			//ErrorDescription(hres);
			dwRet = 0x07 ;
			goto Cleanup ;
		}

		//AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::while( pEnumerator )");
		while( pEnumerator )
		{

			//AddLogEntry(L">>> inside GetBIOSSerialNumber::while( pEnumerator )");

			hr = pEnumerator->Next(WBEM_INFINITE, 1,&pclsObj, &uReturn);
			if(0 == uReturn)
			{
				AddLogEntry(L"### Failed in GetMotherBoardSerialNumber::pEnumerator->Next");
				break ;
			}

			if( NULL == pclsObj )
			{
				AddLogEntry(L"### Failed in GetMotherBoardSerialNumber::Web Class Object");
				break;
			}

			hr = pclsObj->Get( L"SerialNumber", 0, &vtProp, 0, 0);
			hh=vtProp.bstrVal;
			VariantClear(&vtProp);
			pclsObj->Release();

			//AddLogEntry(L">>> inside GetMotherBoardSerialNumber::before hh.GetLength()");

			hh.Trim();
			if( hh.GetLength() /* wcslen(vtProp.bstrVal) > 0x02 */ )
			{
				
				wsprintf(psMotherBoardSerialNumber, L"%s", hh.Trim() ) ;
				//wsprintf(psMotherBoardSerialNumber, L"%s", vtProp.bstrVal ) ;
				AddLogEntry(L">>> Got GetMotherBoardSerialNumber::%s", psMotherBoardSerialNumber);

				break ;
			}

			//AddLogEntry(L">>> inside GetBIOSSerialNumber::after hh.GetLength()");
		}
	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		AddLogEntry(L"### Exception in GetMotherBoardSerialNumber");
	}

Cleanup:

		//AddLogEntry(L">>> inside GetBIOSSerialNumber::before Cleanup");

		if( pSvc )
			pSvc->Release() ;

		if( pLoc )
			pLoc->Release() ;

		if( pEnumerator )
			pEnumerator->Release() ;

		CoUninitialize() ;

		//AddLogEntry(L">>> inside GetBIOSSerialNumber::after Cleanup");

	//AddLogEntry(L">>> Out GetBIOSSerialNumber");

	return dwRet;
}

/***********************************************************************************************
  Function Name  : GetBIOSSerialNumber
  Description    : Function which gets BIOS Number.
  SR.NO			 : WRDWIZCOMMSRV_61
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD GetBIOSSerialNumberSEH(TCHAR *psMotherBoardSerialNumber )
{
	__try
	{
		return GetBIOSSerialNumber(psMotherBoardSerialNumber);	
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		return 0x01 ;
	}

	return 0x02 ;
}


DWORD GetBIOSSerialNumber(TCHAR *pszBIOSSerialNumber )
{
	DWORD				dwRet = 0x00 ;

	HRESULT				hres = S_OK ;
	HRESULT				hr = S_OK ;
	IWbemLocator		*pLoc = NULL ;
	IWbemServices		*pSvc = NULL ;
	IEnumWbemClassObject* pEnumerator = NULL ;
	IWbemClassObject	*pclsObj = NULL ;
	ULONG uReturn		= 0 ;

	VARIANT				vtProp ;
	CString				hh = L"" ;

	try
	{
		//static bool			g_bCoInitializeSecurityCalled = false;
/*
		hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
		}
*/
		if(!g_bCoInitializeSecurityCalled)
		{

			hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
			/*if( hres != S_OK )
			{
				ErrorDescription(hres);
				dwRet = 0x01 ;
				goto Cleanup ;
			}*/

			hres = CoInitializeSecurity(NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE,NULL) ;
			/*if( hres != S_OK )
			{
				ErrorDescription(hres);
				dwRet = 0x01 ;
				goto Cleanup ;
			}*/

			g_bCoInitializeSecurityCalled = true;
		}

		hres = CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator, (LPVOID *) &pLoc);
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if( !pLoc )
		{
			ErrorDescription(hres);
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),NULL,NULL,0,NULL,0,0,&pSvc);
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		if( !pSvc )
		{
			ErrorDescription(hres);
			dwRet = 0x05 ;
			goto Cleanup ;
		}

		hres = CoSetProxyBlanket(pSvc,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE);
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x06 ;
			goto Cleanup ;
		}

		hres = pSvc->ExecQuery(bstr_t("WQL"),bstr_t("SELECT * FROM Win32_BIOS"),WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,NULL,&pEnumerator);
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x07 ;
			goto Cleanup ;
		}

		while( pEnumerator )
		{
			hr = pEnumerator->Next(WBEM_INFINITE, 1,&pclsObj, &uReturn);
			if(0 == uReturn)
				break ;

			if( NULL == pclsObj )
				break;

			hr = pclsObj->Get( L"SerialNumber", 0, &vtProp, 0, 0);
			hh=vtProp.bstrVal;
			VariantClear(&vtProp);
			pclsObj->Release();

			hh.Trim();
			if( hh.GetLength() )
			{
				wsprintf(pszBIOSSerialNumber, L"%s", hh.Trim() ) ;
				break ;
			}
		
		}
	}
	catch(...)
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetBIOSSerialNumber", 0, 0, true, SECONDLEVEL);
	}
Cleanup:
		if( pSvc )
			pSvc->Release() ;

		if( pLoc )
			pLoc->Release() ;

		if( pEnumerator )
			pEnumerator->Release() ;

		CoUninitialize() ;

	return dwRet;
}

/***********************************************************************************************
  Function Name  : RemoveCharsIfExists
  Description    : Function which removes blank characters from BIOS Number.
  SR.NO			 : WRDWIZCOMMSRV_62
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
DWORD RemoveCharsIfExists(TCHAR *pszBIOSSerialNumber, int iLen, int iSize, TCHAR chRemove )
{
	__try
	{
		TCHAR	szTemp[56] = {0} ;

		if( (iLen <=0) || iLen > 56 )
			return 1 ;

		int i = 0x00, j = 0x00 ;

		for(i=0; i<iLen; i++ )
		{
			if( pszBIOSSerialNumber[i] != chRemove )
				szTemp[j++] = pszBIOSSerialNumber[i] ;
		}

		szTemp[j] = '\0' ;

		ZeroMemory(pszBIOSSerialNumber, iSize ) ;
		wcscpy(pszBIOSSerialNumber, szTemp ) ;
	}
	//catch(...)
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in RemoveCharsIfExists", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return 0 ;
}
/***********************************************************************************************
  Function Name  : AddServiceEntryInSafeMode
  Description    : Function which add registry entries so that service can start in safe mode.
  SR.NO			 : WRDWIZCOMMSRV_58
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
void AddServiceEntryInSafeMode( )
{
	 if(g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\VibraniumComSrv", L"", 
	  L"Service") != 0)
	 {
	  AddLogEntry(L"### SetRegistryValueData failed in Minimal for Service VibraniumComSrv in  AddServiceEntryInSafeMode()");
	 }
	
	 if(g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\VibraniumComSrv", L"", 
	  L"Service") != 0)
	 {
	  AddLogEntry(L"### SetRegistryValueData failed in Network for Service VibraniumComSrv in  AddServiceEntryInSafeMode()");
	 }
}

bool GetVGAAdapterID( TCHAR *pszDispAdapterID )
{

	HDEVINFO        hDevInfo         = 0L;
    SP_DEVINFO_DATA spDevInfoData    = {0};
	SP_CLASSIMAGELIST_DATA _spImageData = {0};

	TCHAR	szMotherBoradRes[512] = {0};

    short	wIndex           = 0;
	bool	bVGA = false;

	_try
	{

	hDevInfo = SetupDiGetClassDevs(0L, 0L, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);
    if (hDevInfo == (void*)-1)
    {
		AddLogEntry(L"#### Failed in GetVGAAdapterID::SetupDiGetClassDevs");
        return 1;
    };

	wIndex = 0;
    spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	int iDev = 0x00;

	while( 1 )
    {

		TCHAR  szBuf[MAX_PATH] = {0};
		TCHAR  szID[LINE_LEN+1]  = {0};
		TCHAR                   szName[64]      = {0};

        if (SetupDiEnumDeviceInfo(hDevInfo,
                                  wIndex,
                                  &spDevInfoData))
        {
			
            short wImageIdx       = 0;
            short wItem           = 0;

			if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                  &spDevInfoData,
                                                  SPDRP_CLASS, //SPDRP_DEVICEDESC,
                                                  0L,
                                                  (PBYTE)szBuf,
                                                  2048,
                                                  0))
            {
                wIndex++;
                continue;
            };

            SetupDiGetClassImageIndex(&_spImageData, &spDevInfoData.ClassGuid, (int*)&wImageIdx);

                TCHAR                   szPath[MAX_PATH] = {0};
                DWORD                  dwRequireSize = 0x00;

                if (!SetupDiGetClassDescription(&spDevInfoData.ClassGuid,
                                                szBuf,
                                                MAX_PATH,
                                                &dwRequireSize))
                {
                    wIndex++;
                    continue;
                };
                

				SetupDiGetDeviceInstanceId(hDevInfo, &spDevInfoData, szID, LINE_LEN, 0);
                if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                     &spDevInfoData,
                                                     SPDRP_FRIENDLYNAME,
                                                     0L,
                                                     (PBYTE)szName,
                                                     63,
                                                     0))
                {
                    //DisplayDriverDetailInfo(hItem, nIdTree, szName, wImageIdx, wImageIdx);
                    //AddNewDeviceNode(spDevInfoData.ClassGuid, szName, szID, szPath, wIndex, wOrder);
                }
                else if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                     &spDevInfoData,
                                                     SPDRP_DEVICEDESC,
                                                     0L,
                                                     (PBYTE)szName,
                                                     63,
                                                     0))
                {
                    //DisplayDriverDetailInfo(hItem, nIdTree, szName, wImageIdx, wImageIdx);
                    //AddNewDeviceNode(spDevInfoData.ClassGuid, szName, szID, szPath, wIndex, wOrder);
//                    if (!GetFirmwareEnvironmentVariable(szName, (LPCSTR)&spDevInfoData.ClassGuid, szBuf, 127))
//                        ShowErrorMsg(_hDlg, GetLastError(), "GetFirmwareEnvironmentVariable");
                };

				if( (_wcsicmp(szBuf, L"Display adapters") == 0) && 
					(_wcsicmp(szName, L"Standard VGA Graphics Adapter") == 0) )
				{

					if( wcslen( szID ) > 0x04 )
					{
						wcscpy_s(pszDispAdapterID, 0x37, &szID[0x08] );
						bVGA = true;
					}

					break;
				}

		}

		wIndex++;
		if( wIndex > 0x80 )
			break;
	 }

	 if( !pszDispAdapterID[0] )
		 //AddLogEntry(L"### GetVGAAdapterID::failed to VGA not found");
		 wcscpy_s(pszDispAdapterID, 0x37, L"_NULL" );
	}
	//catch(...)
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetVGAAdapterID", 0, 0, true, SECONDLEVEL);
	}

	return bVGA;
}
/***************************************************************************************************                    
*  Function Name  : GetMACAddress()
*  Description    : returns MAC Address which is 6 bytes
*  Author Name    : Vilas                                                                                      *
*  Date			  :	26-Sept-2014
*  Modified Date  :	27-Sept-2014
****************************************************************************************************/
DWORD GetMACAddress(TCHAR *pMacAddress, LPTSTR lpszOldMacID)
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapInfo = NULL;

	DWORD dwBufLen = 0x00;
	DWORD dwCount = 0x00;
	DWORD dwRet = 0x00, dwDivisor=0x00, dwStatus = 0x00;

	__try
	{
		dwStatus = GetAdaptersInfo( pAdapterInfo, &dwBufLen );
		if( dwStatus != ERROR_BUFFER_OVERFLOW )
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		dwDivisor = sizeof IP_ADAPTER_INFO;

		if( sizeof time_t == 0x08)
			dwDivisor -= 8;

		dwCount = dwBufLen / dwDivisor ;
		if(!dwCount )
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		pAdapterInfo = new IP_ADAPTER_INFO[dwCount];
		if(!pAdapterInfo )
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		//TCHAR	szMacAddress[64] = {0};

		ZeroMemory(pAdapterInfo, dwBufLen );
		if( GetAdaptersInfo(pAdapterInfo, &dwBufLen) != ERROR_SUCCESS )
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		//TCHAR	szMacAddress[0x400] = {0};
		TCHAR	szDescp[0x200] = {0};
		//bool	bTypeIEEE80211 = false;

		pAdapInfo = pAdapterInfo;

		while( pAdapInfo )
		{
			//MultiByteToWideChar( CP_ACP, 0, pAdapInfo->Description, -1, szDescp, sizeof(TCHAR)*0x1FF ) ;

			//Added to get only Ethernet address
			if( (strstr(pAdapInfo->Description, "Virtual ")==NULL) &&
				(strstr(pAdapInfo->Description, "Bluetooth ")==NULL) &&
				(strstr(pAdapInfo->Description, "Wireless ")==NULL) &&
				(strstr(pAdapInfo->Description, "(PAN)")==NULL) &&
				(strstr(pAdapInfo->Description, "Wi-Fi ")==NULL) &&
				(strstr(pAdapInfo->Description, "WiFi ")==NULL) )
			{
				wsprintf(pMacAddress,	L"%02X%02X%02X%02X%02X%02X", pAdapInfo->Address[0],
										pAdapInfo->Address[1], pAdapInfo->Address[2], 
										pAdapInfo->Address[3], pAdapInfo->Address[4],
										pAdapInfo->Address[5] );

				if (lpszOldMacID == NULL)
				{
					AddLogEntry(L">>> MACID: %s", pMacAddress, 0, true, ZEROLEVEL);
					break;
				}

				int iLen = _tcslen(lpszOldMacID);
				if (iLen != 0x00 && ( iLen - 0x0C ) >= 0x0C)
				{
					if (memcmp(&pMacAddress[0], &lpszOldMacID[iLen - 0x0C], 0x0C) == 0)
					{
						AddLogEntry(L">>> MACID: %s", pMacAddress, 0, true, ZEROLEVEL);
						break;
					}
				}
			}

			pAdapInfo = pAdapInfo->Next;
		}

		if( !wcslen(pMacAddress) )
		{
			AddLogEntry(L"### Failed in GetMACAddress", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetMACAddress", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( pAdapterInfo )
		delete[] pAdapterInfo;

	pAdapterInfo = NULL;

	return dwRet;
}

/**********************************************************************************************************                     
* Function Name      : CheckForMIDInRegistry
* Description        : Check Machine ID is present in registry or not
* Author Name		 : Neha Gharge	                                                                                           
* Date Of Creation   : 10th Oct 2014
* SR_No              : 
***********************************************************************************************************/
bool CheckForMIDInRegistry()
{
	try
	{
		//CITinRegWrapper objReg;
		TCHAR szMachineIDValue[0x80] = { 0 };
		DWORD dwSize = sizeof(szMachineIDValue);

		g_objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"MVersion", szMachineIDValue, dwSize);

		//check if its first time installation
		bool bFirstTimeInstallation = false;
		if (_tcslen(szMachineIDValue) == 0x00)
		{
			bFirstTimeInstallation = true;
		}

		TCHAR szHDDSerial[MAX_PATH] = { 0 };
		CCPUInfo	objCPUInfo;
		_tcscpy_s(szHDDSerial, objCPUInfo.GetDiskSerialNo());

		bool bUsingHDDSerial = false;
		//Check here machine ID it same with registry and file, else take from DB.
		if (_tcscmp(m_ActInfo.szClientID, szMachineIDValue) != 0)
		{
			if (_tcslen(m_ActInfo.szClientID) == 0)
			{
				//This means this user istalling first time.
				bUsingHDDSerial = true;
			}
			else if (CheckForHardDiskSerialNumber(m_ActInfo.szClientID, szHDDSerial))
			{
				bUsingHDDSerial = true;
			}
			else
			{
				_tcscpy_s(szMachineIDValue, m_ActInfo.szClientID);
				bUsingHDDSerial = false;
			}
		}
		else if (CheckForHardDiskSerialNumber(m_ActInfo.szClientID, szHDDSerial))//check machine ID contains HardDisk Serial number.
		{
			bUsingHDDSerial = true;
		}
		else if (_tcslen(m_ActInfo.szClientID) == 0 && _tcslen(szMachineIDValue) == 0)
		{
			bUsingHDDSerial = true;
		}

		//Generate here machine ID
		TCHAR	szClientID[0x80] = { 0 };
		if (!GenerateMachineID(szClientID, sizeof(szClientID), bUsingHDDSerial, szHDDSerial, szMachineIDValue))
		{
			return false;
		}

		if (_tcslen(m_ActInfo.szClientID) != 0)
		{
			if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"MVersion", m_ActInfo.szClientID) != 0)
			{
				AddLogEntry(L"### Error in CheckForMIDInRegistry", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
		else
		{
			if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"MVersion", szClientID) != 0)
			{
				AddLogEntry(L"### Error in CheckForMIDInRegistry", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CheckForMIDInRegistry", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

 // issue resolve by lalit kumwat 1-14-2015 In the WardWiz Live Update popup, it again continuously displays popup message box 
/***********************************************************************************************
  Function Name  : Check4StartUpEntries
  Description    : it use to launch startup process like tray and main UI
  SR.NO			 : 
  Author Name    : lalit kumawat
  Date           : 1-14-2015
***********************************************************************************************/
void Check4StartUpEntries()
{
	HANDLE					hToken = NULL;
	CExecuteProcess			objEnumprocess;
	BOOL bRet = FALSE;

	//Ram Shelke - 09-10-2015
	//Wait here till explore does not launch.
	do
	{
		bRet = OpenProcessToken(objEnumprocess.GetExplorerProcessHandle(L"explorer.exe"), TOKEN_ALL_ACCESS, &hToken);
		if (!bRet)
		{
			Sleep(1000);
		}
	} while (!bRet);

	CString csExePath, csCommandLine;
	csExePath.Format(L"%s%s", GetWardWizPathFromRegistry(), L"VBTRAY.EXE");

	//Neha Gharge 1-4-2015 Splash Screen was not displayed at the time of restart.
	csCommandLine.Format(L"%s",L"-SHOWSPSCRN");
	//Launch wardwiz tray application
	//Issue : 0000158: In Settings if we select Start-Up scan as Quick/Full Scan after restart/Shutdown when we start the PC the start-up scan is not working.
	//Resolved by: Nitin K.
	objEnumprocess.StartProcessWithTokenExplorerWait(csExePath, csCommandLine, L"explorer.exe");
	
	csExePath.Format(L"%s%s", GetWardWizPathFromRegistry(), L"VBUI.EXE");

	CITinRegWrapper objReg;
	DWORD dwScanType = 0;
	if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwStartUpScan", dwScanType) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwStartUpScan KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);;
		return;
	}

	if(dwScanType == 0)
	{
		//if dont have setting for startup scan return from here.
		return;
	}

	switch(dwScanType)
	{
	case 1:
		csCommandLine.Format(L"-STARTUPSCAN");
		break;
	case 2:
		csCommandLine.Format(L"-STARTUPSCAN -FULLSCAN");
		break;
	case 3:
		csCommandLine.Format(L"-EPSNOUI -FULLSCAN");
		break;
	case 4:
		csCommandLine.Format(L"-EPSNOUI -QUICKSCAN");
		break;
	case 5:
		csExePath.Format(L"%s%s", GetWardWizPathFromRegistry(), L"VBUSBDETECTUI.EXE");
		csCommandLine.Format(L"-EPSNOUI -USBSCAN");
		break;
	}

	//Launch wardwiz tray application
	//Issue : 0000158: In Settings if we select Start-Up scan as Quick/Full Scan after restart/Shutdown when we start the PC the start-up scan is not working.
	//Resolved by: Nitin K.
	objEnumprocess.StartProcessWithTokenExplorerWait(csExePath, csCommandLine, L"explorer.exe");
}

/***********************************************************************************************
  Function Name  : CheckScanLevel
  Description    : To check scan level 1-> Only with wardwiz scanner
									   2-> with clam scanner and second level scaner is wardwiz scanner.
  SR.NO			 : 
  Author Name    : Neha gharge
  Date           : 1-19-2015
***********************************************************************************************/
void CheckScanLevel()
{
	DWORD dwScanLevel = 0;
	if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwScanLevel", dwScanLevel) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CheckScanLevel, KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);;
		return;
	}

	switch(dwScanLevel)
	{
	case 0x01:
		g_objISpyScannerBase.m_eScanLevel = WARDWIZSCANNER;
		break;
	case 0x02:
		g_objISpyScannerBase.m_eScanLevel = CLAMSCANNER;
		break;
	default: 
		AddLogEntry(L" ### Scan level option is wrong. Please reinstall setup of Wardwiz",0,0,true,SECONDLEVEL);
		break;
	}
}

/***********************************************************************************************
  Function Name  : DeleteFileFromINI
  Description    : Delete the file which failed at the time of clean.
  SR.NO			 : 
  Author Name    : Neha gharge
  Date           : 2-17-2015
***********************************************************************************************/
bool DeleteFileFromINI()
{
	DWORD dwCount = 0x00, dwsuccessfulDeleteCount = 0x00;
	TCHAR szValueName[MAX_PATH] = { 0 };
	TCHAR szValueData[512] = { 0 };
	TCHAR szAttmptData[512] = { 0 };
	TCHAR szApplnName[512] = { 0 };
	TCHAR szFileName[512] = { 0 };
	TCHAR szQuarantineFileName[512] = { 0 };
	TCHAR szVirusName[512] = { 0 };
	TCHAR szConcatenatePath[1024] = { 0 };
	CString csAttampt(L"");
	try
	{
		g_vFailedToDeleteEvenOnRestart.clear();
		CString csQuarantineFolderPath = GetQuarantineFolderPath();
		csQuarantineFolderPath.Append(L"\\WRDWIZDELETEFAIL.INI");

		if (!PathFileExists(csQuarantineFolderPath))
		{
			AddLogEntry(L"### File not found : %s in DeleteFileFromINI ", csQuarantineFolderPath, 0, true, FIRSTLEVEL);
			return true;
		}

		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csQuarantineFolderPath);
		dwsuccessfulDeleteCount = dwCount;
		if (dwCount)
		{
			for (int i = 1; i <= static_cast<int>(dwCount); i++)
			{
				swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
				GetPrivateProfileString(L"DeleteFiles", szValueName, L"", szValueData, 511, csQuarantineFolderPath);
				if (!szValueData[0])
				{
					AddLogEntry(L"### Function is  DeleteFileFromINI.::Invalid Entries for(%s) in (%s)", szValueName, csQuarantineFolderPath, true, 0x01);
					break;
				}
				else
				{
					DWORD dwAttempt = 0x00;

					bool bISTokenizeFailed = false;
					if (g_objISpyScannerBase.m_eScanLevel == WARDWIZSCANNER)
					{
						if (!g_objISpyScannerBase.m_objWardWizScanner.TokenizationOfParameterForrecover(szValueData, szFileName,_countof(szFileName), szQuarantineFileName, _countof(szQuarantineFileName), szVirusName, _countof(szVirusName)))
						{
							AddLogEntry(L"### Failed in TokenizationOfParameterForrecover ini data in comsrv", 0, 0, true, FIRSTLEVEL);
							bISTokenizeFailed = true;
						}
					}
					else if (g_objISpyScannerBase.m_eScanLevel == CLAMSCANNER)
					{
						if (!g_objISpyScannerBase.m_objClamScanner.TokenizationOfParameterForrecover(szValueData, szFileName, _countof(szFileName), szQuarantineFileName, _countof(szQuarantineFileName), szVirusName, _countof(szVirusName)))
						{
							AddLogEntry(L"### Failed in TokenizationOfParameterForrecover ini data in comsrv", 0, 0, true, FIRSTLEVEL);
							bISTokenizeFailed = true;
						}
					}

					if (bISTokenizeFailed)
					{
						if (wcslen(szQuarantineFileName) > 0x00)
						{
							wcscpy(szVirusName, szQuarantineFileName);
						}

						//take backup of file.
						if (!g_objISpyScannerBase.m_objWardWizScanner.BackUpBeforeQuarantineOrRepair(szFileName, szQuarantineFileName))
						{
							AddLogEntry(L"#### Failed to take backup of %s", szApplnName, 0, true, SECONDLEVEL);
							return false;
						}
					}

					if (!TokenizeIniData(szFileName, szApplnName, _countof(szApplnName), dwAttempt))
					{
						AddLogEntry(L"### Failed in tokenize ini data in comsrv", 0, 0, true, FIRSTLEVEL);
						return false;
					}

					if (PathFileExists(szApplnName))
					{
						SetFileAttributes(szApplnName, FILE_ATTRIBUTE_NORMAL);
						if (!DeleteFile(szApplnName))
						{
							if (dwAttempt >= 1)
							{ 
								dwsuccessfulDeleteCount--;
								AddLogEntry(L"### Function is  DeleteFileFromINI:: %s File failed to delete", szValueData, 0, true, 0x01);
								ZeroMemory(szValueName, sizeof(szValueName));
								swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
								WritePrivateProfileString(L"DeleteFiles", szValueName, NULL, csQuarantineFolderPath);

								ZeroMemory(szValueName, sizeof(szValueName));
								swprintf_s(szValueName, _countof(szValueName), L"%lu", dwsuccessfulDeleteCount);
								WritePrivateProfileString(L"Count", L"Count", szValueName, csQuarantineFolderPath);
							}
							else if (dwAttempt < 1)
							{
								AddLogEntry(L"### Function is  DeleteFileFromINI:: %s File failed to delete", szValueData, 0, true, 0x01);
								dwAttempt++;
								csAttampt.Format(L"%d", dwAttempt);
								ZeroMemory(szAttmptData, sizeof(szAttmptData));
								_tcscpy_s(szAttmptData, _countof(szAttmptData),szApplnName);
								_tcscat_s(szAttmptData, _countof(szAttmptData), L",");
								_tcscat_s(szAttmptData, _countof(szAttmptData), csAttampt);

								if (g_objISpyScannerBase.m_eScanLevel == WARDWIZSCANNER)
								{
									if (!g_objISpyScannerBase.m_objWardWizScanner.MakeConcatStringforrecover(szAttmptData, szQuarantineFileName, szVirusName, szConcatenatePath, _countof(szConcatenatePath)))
									{
										AddLogEntry(L"### Failed in MakeConcatStringforrecover in Comsrv", 0, 0, true, FIRSTLEVEL);
										return false;
									}
								}
								else if (g_objISpyScannerBase.m_eScanLevel == CLAMSCANNER)
								{
									if (!g_objISpyScannerBase.m_objClamScanner.MakeConcatStringforrecover(szAttmptData, szQuarantineFileName, szVirusName, szConcatenatePath, _countof(szConcatenatePath)))
									{
										AddLogEntry(L"### Failed in MakeConcatStringforrecover in Comsrv", 0, 0, true, FIRSTLEVEL);
										return false;
									}
								}
						
								ZeroMemory(szValueName, sizeof(szValueName));
								swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
								WritePrivateProfileString(L"DeleteFiles", szValueName, szConcatenatePath, csQuarantineFolderPath);

								g_vFailedToDeleteEvenOnRestart.push_back(szConcatenatePath);
							}

						}
						else
						{
							dwsuccessfulDeleteCount--;
							
							ZeroMemory(szValueName, sizeof(szValueName));
							swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
							WritePrivateProfileString(L"DeleteFiles", szValueName, NULL, csQuarantineFolderPath);
							
							ZeroMemory(szValueName, sizeof(szValueName));
							swprintf_s(szValueName, _countof(szValueName), L"%lu", dwsuccessfulDeleteCount);
							WritePrivateProfileString(L"Count", L"Count", szValueName, csQuarantineFolderPath);
							if (g_objISpyScannerBase.m_eScanLevel == WARDWIZSCANNER)
							{
								if (!g_objISpyScannerBase.m_objWardWizScanner.UpdateRecoverEntry(szApplnName, szQuarantineFileName, szVirusName, 0))
								{
									AddLogEntry(L"### Failed to update UpdateRecoverEntry in comsrv",0,0,true,FIRSTLEVEL);
									return false;
								}
							}
							else if(g_objISpyScannerBase.m_eScanLevel == CLAMSCANNER)
							{
								if (!g_objISpyScannerBase.m_objClamScanner.UpdateRecoverEntry(szApplnName, szQuarantineFileName, szVirusName, 0))
								{
									AddLogEntry(L"### Failed to update UpdateRecoverEntry in comsrv", 0, 0, true, FIRSTLEVEL);
									return false;
								}
							}
					

						}
					}
					else
					{
						dwsuccessfulDeleteCount--;

						ZeroMemory(szValueName, sizeof(szValueName));
						swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
						WritePrivateProfileString(L"DeleteFiles", szValueName, NULL, csQuarantineFolderPath);

						ZeroMemory(szValueName, sizeof(szValueName));
						swprintf_s(szValueName, _countof(szValueName), L"%lu", dwsuccessfulDeleteCount);
						WritePrivateProfileString(L"Count", L"Count", szValueName, csQuarantineFolderPath);

						if (g_objISpyScannerBase.m_eScanLevel == WARDWIZSCANNER)
						{
							if (!g_objISpyScannerBase.m_objWardWizScanner.UpdateRecoverEntry(szApplnName, szQuarantineFileName, szVirusName, 0))
							{
								AddLogEntry(L"### Failed to update UpdateRecoverEntry in comsrv", 0, 0, true, FIRSTLEVEL);
								return false;
							}
						}
						else if (g_objISpyScannerBase.m_eScanLevel == CLAMSCANNER)
						{
							if (!g_objISpyScannerBase.m_objClamScanner.UpdateRecoverEntry(szApplnName, szQuarantineFileName, szVirusName, 0))
							{
								AddLogEntry(L"### Failed to update UpdateRecoverEntry in comsrv", 0, 0, true, FIRSTLEVEL);
								return false;
							}
						}

					}
				}
			}
		}
		dwCount = 0x00;
		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csQuarantineFolderPath);
		if (!dwCount)
		{
			if (PathFileExists(csQuarantineFolderPath))
			{
				SetFileAttributes(csQuarantineFolderPath, FILE_ATTRIBUTE_NORMAL);
				if (!DeleteFile(csQuarantineFolderPath))
				{
					AddLogEntry(L"### Function is  DeleteFileFromINI::Failed to delete file: %s  ", csQuarantineFolderPath, 0, true, FIRSTLEVEL);
					return false;
				}
			}
		}
		else
		{
			WritePrivateProfileString(L"DeleteFiles", NULL, NULL, csQuarantineFolderPath);
			WritePrivateProfileString(L"Count", NULL, NULL, csQuarantineFolderPath);

			DWORD dwArrIndex = 0x00;
			//This is to give proper number on INI after deleting any file successfully.
			for (int iIndex = 1; iIndex <= static_cast<int>(g_vFailedToDeleteEvenOnRestart.size()); iIndex++)
			{
				ZeroMemory(szValueName, sizeof(szValueName));
				swprintf_s(szValueName, _countof(szValueName), L"%lu", iIndex);
				WritePrivateProfileString(L"DeleteFiles", szValueName, g_vFailedToDeleteEvenOnRestart.at(dwArrIndex), csQuarantineFolderPath);

				ZeroMemory(szValueName, sizeof(szValueName));
				swprintf_s(szValueName, _countof(szValueName), L"%lu", g_vFailedToDeleteEvenOnRestart.size());
				WritePrivateProfileString(L"Count", L"Count", szValueName, csQuarantineFolderPath);

				dwArrIndex++;

			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CComSrv::DeleteFileFromINI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
	
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Neha Gharge
*  Date           : 17 Feb 2015
*  SR_NO		  :
**********************************************************************************************************/
CString GetQuarantineFolderPath()
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
		AddLogEntry(L"### Exception in CComSrv::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/**********************************************************************************************************
*  Function Name  :	TokenizeIniData
*  Description    :	Tokenization of entries from delete file ini
*  Author Name    : Neha Gharge
*  Date           : 26 Feb 2015
*  SR_NO		  :
**********************************************************************************************************/
bool TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwsizeofApplnName,DWORD &dwAttempt)
{
	TCHAR	szToken[] = L",";
	TCHAR	*pToken = NULL;

	DWORD dwAttemptCnt = 0x00;
	__try
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
			wcscpy_s(szApplicationName, (dwsizeofApplnName-1), szValueApplicationName);
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		}

		TCHAR	szAttemptCnt[16] = { 0 };

		if (pToken)
			wcscpy_s(szAttemptCnt,_countof(szAttemptCnt) ,pToken);

		if (szAttemptCnt[0])
		{
			//convert into DWORD.
			dwAttemptCnt = static_cast<DWORD>(wcstod(szAttemptCnt, _T('\0')));
			dwAttempt = dwAttemptCnt;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CComSrv::TokenizeIniData", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/***********************************************************************************************
Function Name  : RepairVirusesThread
Description    : This is called on next reboor to repair PE file infector which are failed in last scan.
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 23 March 2015
***********************************************************************************************/
DWORD RepairVirusesThread(LPVOID lpParam)
{
	__try
	{
		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };

		//Add code here merge Recover DB's created by different instance.
		MergeRecoverEntries();

		 //Qurantine files here on service restart.
		 if (!DeleteFileFromINI())
		 {
			 AddLogEntry(L"### Failed in DeleteFilefromINI function in comsrv", 0, 0, true, SECONDLEVEL);
		 }

		//Deleting Recover entries on reboot
		DeleteRecoverEntriesFromIni();

		g_objISpyScannerBase.m_objWardWizScanner.GetQuarantineFolderPath(szRepairIniPath);
		if (!wcslen(szRepairIniPath))
		{
			AddLogEntry(L"### Failed in RepairVirusesThread::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return 0;
		}

		wcscat_s(szRepairIniPath, _countof(szRepairIniPath), L"\\WWRepair.ini");

		DWORD dwRepairCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRepairIniPath);

		if (!dwRepairCount)
			return 0;

		DWORD	i = 0x01;

		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[1024] = { 0 };

		for (; i <= dwRepairCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			ZeroMemory(szValueData, sizeof(szValueData));

			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRepairIniPath);
			if (wcslen(szValueData) == 0x00)
				continue;

			ParseIniLineAndSendToRepair(szValueData);
			Sleep(100);
		}

		WritePrivateProfileString(L"Count", NULL, NULL, szRepairIniPath);
		WritePrivateProfileString(L"Files", NULL, NULL, szRepairIniPath);

		SetFileAttributes(szRepairIniPath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(szRepairIniPath);

	}
	//catch(...)
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in RepairVirusesThread", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}


/***********************************************************************************************
Function Name  : ParseIniLineAndSendToRepair
Description    : Parses WWRepair.ini and sends for scan and repair
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 23 March 2015
Modified Date  : 26 March 2015
***********************************************************************************************/
bool ParseIniLineAndSendToRepair(LPTSTR lpszIniLine)
{
	bool	bReturn = false;
	__try
	{
		TCHAR	szToken[] = L"|";
		TCHAR	*pToken = NULL;

		pToken = wcstok(lpszIniLine, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize::Threat ID not found", 0, 0, true, FIRSTLEVEL);
			return bReturn;
		}

		TCHAR	szVirusID[256] = { 0 };
		DWORD	dwVirusID = 0x00;

		if (pToken)
		{
			wcscpy(szVirusID, pToken);
			swscanf(szVirusID, L"%lu", &dwVirusID);
		}

		if (!dwVirusID)
		{
			AddLogEntry(L"### No string to tokenize::Invalid Threat ID", 0, 0, true, FIRSTLEVEL);
			return bReturn;
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize::Invalid Threat name", 0, 0, true, FIRSTLEVEL);
			return bReturn;
		}

		TCHAR	szThreatName[512] = { 0 };
		TCHAR	szFilePath[512] = { 0 };


		if (pToken)
			wcscpy(szThreatName, pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize::Threat name not found", 0, 0, true, FIRSTLEVEL);
			return bReturn;
		}

		if (pToken)
			wcscpy(szFilePath, pToken);

	//Added for recovery of missing files
	//Vilas S 11/4/2015
	/*
		if (!PathFileExists(szFilePath))
		{
			AddLogEntry(L"### No string to tokenize::File not present", 0, 0, true, FIRSTLEVEL);
			return bReturn;
		}
    */

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize::Duplicate File name not found", 0, 0, true, FIRSTLEVEL);
			return bReturn;
		}

		TCHAR	szDuplicateFile[512] = { 0 };

		if (pToken)
			wcscpy(szDuplicateFile, pToken);

		bReturn = g_objISpyScannerBase.RescanAndRepairFile(szFilePath, szThreatName, szDuplicateFile, dwVirusID);
		if (!bReturn)
			AddLogEntry(L"### Failed to repair file in ParseIniLineAndSendToRepair : %s", szFilePath, 0, true, SECONDLEVEL);

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ParseIniLineAndSendToRepair", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}


/***********************************************************************************************
Function Name  : DeleteRecoverEntriesFromIni
Description    : Deletes entries from WWRecover.ini on next reboot
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 27 March 2015
***********************************************************************************************/
DWORD DeleteRecoverEntriesFromIni()
{
	__try
	{
		TCHAR	szRecoverIniPath[MAX_PATH] = { 0 };

		Sleep(0);


		g_objISpyScannerBase.m_objWardWizScanner.GetQuarantineFolderPath(szRecoverIniPath);
		if (!wcslen(szRecoverIniPath))
		{
			AddLogEntry(L"### Failed in RepairVirusesThread::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return 1;
		}

		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), L"\\");
		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), WRDWIZRECOVERINI);


		//added for delete on reboot WWRECOVER.INI entries
		//Added by Vilas on 10 April 2015
		DWORD dwRecoverCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRecoverIniPath);

		if (dwRecoverCount)
		{

			DWORD	i = 0x01;

			TCHAR	szValueName[256] = { 0 };
			TCHAR	szValueData[1024] = { 0 };

			for (; i <= dwRecoverCount; i++)
			{
				ZeroMemory(szValueName, sizeof(szValueName));
				ZeroMemory(szValueData, sizeof(szValueData));

				swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
				GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRecoverIniPath);
				if (wcslen(szValueData) == 0x00)
					continue;

				SetFileAttributes(szValueData, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(szValueData);
				Sleep(10);
			}
		}

		WritePrivateProfileString(L"Count", NULL, NULL, szRecoverIniPath);
		WritePrivateProfileString(L"Files", NULL, NULL, szRecoverIniPath);

		SetFileAttributes(szRecoverIniPath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(szRecoverIniPath);

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in DeleteRecoverEntriesFromIni", 0, 0, true, SECONDLEVEL);
		return 1;
	}

	return 0;
}

/***********************************************************************************************
Function Name  : LoadWardWizSignatureDatabase
Description    : Function to Load Wardwiz signature database.
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 27 March 2015
***********************************************************************************************/
DWORD LoadWardWizSignatureDatabase(LPVOID lpParam)
{
	__try
	{
		DWORD dwSigCount = 0x00;
		if (g_objISpyScannerBase.LoadSignatureDatabase(dwSigCount) != 0x00)
		{
			AddLogEntry(L"### Failed to Load LoadWardwizSignatureDatabase", 0, 0, true, SECONDLEVEL);
		}

		//Write here Count in ini file.
		TCHAR szSigCount[20] = {0};
		swprintf_s(szSigCount, L"%d", dwSigCount);

		TCHAR szIniFilePath[MAX_PATH] = { 0 };
		if (GetModulePath(szIniFilePath, MAX_PATH))
		{
			_tcscat_s(szIniFilePath, L"\\VBSETTINGS\\PRODUCTSETTINGS.INI");
			WritePrivateProfileString(L"VBSETTINGS", L"ThreatDefCount", szSigCount, szIniFilePath);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in LoadWardwizSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : AddUserAndSystemInfoToLog
Description    : Adds Computer name, logged user name and OS details to log at the top
Author Name    : Vilas Suvarnakar
SR_NO		   :
Date           : 05 May 2015
Modification   :
****************************************************************************/
void AddUserAndSystemInfoToLog()
{

	TCHAR	szModulePath[MAX_PATH] = { 0 };

	if (!GetModulePath(szModulePath, MAX_PATH))
	{
		return;
	}

	TCHAR	szTemp[512] = { 0 };

	swprintf_s(szTemp, _countof(szTemp), L"%s\\Log\\%s", szModulePath, LOG_FILE);

	if (PathFileExists(szTemp))
		return;

	WardWizSystemInfo	objSysInfo;

	objSysInfo.GetSystemInformation();

	LPCTSTR		lpSystemName = objSysInfo.GetSystemName();
	LPCTSTR		lpUserName = objSysInfo.GetUserNameOfSystem();
	LPCTSTR		lpOSDetails = objSysInfo.GetOSDetails();

	ZeroMemory(szTemp, sizeof(szTemp));
	swprintf_s(szTemp, _countof(szTemp), L"%s\n", L"--------------------------------------------------------------------------------------------------------");
	AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);

	if (lpSystemName)
	{
		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* Computer name:%s\n", lpSystemName);
		AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
	}

	if (lpUserName)
	{
		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* Logged user name:%s\n", lpUserName);
		AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
	}

	if (lpOSDetails)
	{
		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* OS details:%s\n", lpOSDetails);
		AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
	}

	ZeroMemory(szTemp, sizeof(szTemp));
	swprintf_s(szTemp, _countof(szTemp), L"%s\n\n", L"--------------------------------------------------------------------------------------------------------");
	AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
	
}

/***************************************************************************************************
*  Function Name  : AddFileInfoToINI()
*  Description    : it provides functionality to add again file path and there crc into ini file.
*  Author Name    : Lalit
*  Date			  :	16-July-2015
*  SR No		  :
****************************************************************************************************/
bool AddFileInfoToINI(CString csfilePath, DWORD dwCRC, DWORD dwAttemp)
{
	bool bRet = false;
	HANDLE hFile = NULL;
	FILE *pInteRollBackFile = NULL;
	CString szFileInfo = L"";
	CString csIniFilePath = L"";
	TCHAR szCount[255] = { 0 };
	DWORD dwCount = 0;
	TCHAR szModulePath[MAX_PATH] = { 0 };

	try
	{
		if (!GetModulePath(szModulePath, MAX_PATH))
					{
						AddLogEntry(L">>>Error in GetModulePath comsrv::AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
						return false;
					}
					csIniFilePath = szModulePath;
					csIniFilePath = csIniFilePath + L"\\FAILDINTIGRITYINFO.INI";

		if (!PathFileExists(csIniFilePath))
		{
			hFile = CreateFile(csIniFilePath, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile == NULL)
			{
				AddLogEntry(L">>> Failed during FAILDINTIGRITYINFO.INI file creation in comserv::AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
				return false;
			}

			CloseHandle(hFile);

			if (!pInteRollBackFile)
				pInteRollBackFile = _wfsopen(csIniFilePath, _T("a"), 0x40);

			if (pInteRollBackFile == NULL)
			{
				AddLogEntry(L">>> Failed during FAILDINTIGRITYINFO.INI file creation in comserv::AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
				return false;
			}

			szFileInfo.Format(_T("[%s]\n"), static_cast<LPCTSTR>(L"FILEINFO"));

			fputws((LPCTSTR)szFileInfo, pInteRollBackFile);

			szFileInfo.Format(_T("%s = 0\n"), static_cast<LPCTSTR>(L"COUNT"));

			fputws((LPCTSTR)szFileInfo, pInteRollBackFile);

			fflush(pInteRollBackFile);
			fclose(pInteRollBackFile);
			pInteRollBackFile = NULL;
		}


		if (!pInteRollBackFile)
			pInteRollBackFile = _wfsopen(csIniFilePath, _T("a"), 0x40);

		if (pInteRollBackFile == NULL)
		{
			AddLogEntry(L">>> Failed during FAILDINTIGRITYINFO.INI file open for write comserv::AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (pInteRollBackFile != NULL)
		{

			GetPrivateProfileString(L"FILEINFO", L"COUNT", L"", szCount, 255, csIniFilePath);

			swscanf(szCount, L"%lu", &dwCount);

			dwCount++;
			swprintf_s(szCount, _countof(szCount), L"%d", dwCount);

			WritePrivateProfileString(L"FILEINFO", L"COUNT", szCount, csIniFilePath);

			szFileInfo.Format(_T("%lu=%s|%lu,%lu \r\n"), dwCount, static_cast<LPCTSTR>(csfilePath), dwCRC, dwAttemp);

			fputws((LPCTSTR)szFileInfo, pInteRollBackFile);

			fflush(pInteRollBackFile);
			fclose(pInteRollBackFile);
		}

	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in GetModulePath in AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : ParseIntegrityInfoINIAndRollBackCRC()
*  Description    : it provides functionality to rollback integrity of file which required system restart.
*  Author Name    : Lalit
*  Date			  :	16-July-2015
*  SR No		  :
****************************************************************************************************/
void ParseIntegrityInfoINIAndRollBackCRC()
{
	try
	{
		TCHAR	szModulePath[MAX_PATH] = { 0 };
		TCHAR	szFaildIntegrityInfoIniPath[MAX_PATH] = { 0 };
		TCHAR	szCount[255] = { 0 };
		DWORD	dwCount = 0;
		FILE	*pInteRollBackFile = NULL;

		if (m_vCsFaildFilePath.size() > 0)
		{
			m_vCsFaildFilePath.clear();
		}

		if (m_vDwCRC.size() > 0)
		{
			m_vDwCRC.clear();
		}

		if (m_vRollbackAttemp.size() > 0)
		{
			m_vRollbackAttemp.clear();
		}

		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L">>> Error in GetModulePath in ParseIntegrityInfoINIAndRollBackCRC", 0, 0, true, SECONDLEVEL);
			return;
		}

		swprintf_s(szFaildIntegrityInfoIniPath, _countof(szFaildIntegrityInfoIniPath), L"%s\\FAILDINTIGRITYINFO.INI", szModulePath);

		if (!PathFileExists(szFaildIntegrityInfoIniPath))
		{
			AddLogEntry(L">>> Error in GetModulePath in ParseIntegrityInfoINIAndRollBackCRC", 0, 0, true, SECONDLEVEL);
			return;
		}

		GetPrivateProfileString(L"FILEINFO", L"COUNT", L"", szCount, 255, szFaildIntegrityInfoIniPath);
		swscanf(szCount, L"%lu", &dwCount);
		TCHAR szValueName[255] = { 0 };
		TCHAR szValueData[MAX_PATH] = { 0 };
		CString csFileIntigrityInfo = L"";
		CString csFilePath = L"";
		CString csCrcWithAttempCount = L"";
		CString csCRC = L"";
		DWORD dwCRC = 0;
		CString csAttemp = L"";
		DWORD dwAttemp = 0;

		for (int index = 1; index <= static_cast<int>(dwCount); index++)
		{

			swprintf_s(szValueName, _countof(szValueName), L"%lu", index);
			GetPrivateProfileString(L"FILEINFO", szValueName, L"", szValueData, 2 * MAX_PATH, szFaildIntegrityInfoIniPath);
			csFileIntigrityInfo = szValueData;
			csFilePath = csFileIntigrityInfo.Left(csFileIntigrityInfo.ReverseFind('|') + 0);
			csCrcWithAttempCount = csFileIntigrityInfo.Mid(csFileIntigrityInfo.ReverseFind('|') + 1);
			csCRC = csCrcWithAttempCount.Left(csCrcWithAttempCount.ReverseFind(',') + 0);
			csAttemp = csCrcWithAttempCount.Mid(csCrcWithAttempCount.ReverseFind(',') + 1);

			swscanf(csCRC, L"%lu", &dwCRC);

			if (csAttemp.GetLength() > 1)
			{
				dwAttemp = 2;
			}
			else
			{
				swscanf(csAttemp, L"%lu", &dwAttemp);
			}

			if ((dwAttemp == 2) || (!PathFileExists(csFilePath)))
			{
				AddLogEntry(L">>>File: [%s] More Then Two attemp not allowed  in ParseIntegrityInfoINIAndRollBackCRC ", csFileIntigrityInfo, 0, true, SECONDLEVEL);
			}
			else
			{
				DWORD dwRet = 0;
				dwRet = AddCheckSum(csFilePath, dwCRC);
				if (dwRet)
				{
					AddLogEntry(L">>>Faild to Rollback Integrity of file: [%s] ParseIntegrityInfoINIAndRollBackCRC ", csFilePath, 0, true, SECONDLEVEL);
					m_vCsFaildFilePath.push_back(csFilePath);
					m_vDwCRC.push_back(dwCRC);
					m_vRollbackAttemp.push_back(dwAttemp + 1);
				}
			}

			DWORD dwCurrentCount = dwCount;
			TCHAR szCount[MAX_PATH] = { 0 };
			TCHAR szCurrentIndex[MAX_PATH] = { 0 };

			swprintf_s(szCount, _countof(szCount), L"%d", dwCount - index);

			swprintf_s(szCurrentIndex, _countof(szCurrentIndex), L"%d", index);

			WritePrivateProfileString(L"FILEINFO", L"COUNT", szCount, szFaildIntegrityInfoIniPath);

			WritePrivateProfileString(L"FILEINFO", szCurrentIndex, NULL, szFaildIntegrityInfoIniPath);
		}


		if (m_vCsFaildFilePath.size() > 0 && m_vDwCRC.size()> 0)
		{
			for (int i = 0; i < static_cast<int>(m_vCsFaildFilePath.size()); i++)
			{
				bool bRet = false;
				bRet = AddFileInfoToINI(m_vCsFaildFilePath.at(i), m_vDwCRC.at(i), m_vRollbackAttemp.at(i));
				if (!bRet)
				{
					AddLogEntry(L">>> Error in AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
				}
			}

		}
		else
		{
			DeleteFile(szFaildIntegrityInfoIniPath);
		}

		m_vCsFaildFilePath.clear();
		m_vDwCRC.clear();
		m_vRollbackAttemp.clear();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ParseIntegrityInfoINIAndRollBackCRC", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : AddCheckSum()
*  Description    : it uses to add check sum byte to encrypted file at end of file
*  Author Name    : Vilas/Lalit
*  Date			  :	09-July-2015
*  SR No		  :
****************************************************************************************************/
DWORD AddCheckSum(LPCTSTR lpFileName, DWORD dwByteNeedtoAdd)
{
	DWORD	dwRet = 0x00;
	TCHAR szszStartTime[255] = { 0 };
	LARGE_INTEGER liDistToMove = { 0 };
	DWORD dwReadBytes = 0x00;

	try
	{

		HANDLE	hFile = CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //Open with write access

		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x01;
			return dwRet;
		}

		SetFilePointerEx(hFile, liDistToMove, NULL, FILE_END);

		WriteFile(hFile, &dwByteNeedtoAdd, sizeof(DWORD), &dwReadBytes, NULL);

		if (dwReadBytes != sizeof(DWORD))
		{
			dwRet = 0x02;
		}

		CloseHandle(hFile);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in comserv::AddCheckSum", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/***********************************************************************************************
  Function Name  : SendUserInformationThread
  Description    : Thread function to send User information to server.
  SR.NO			 : 
  Author Name    : Ramkrushna Shelke
  Date           : 09 Oct 2014
***********************************************************************************************/
DWORD SendUserInformationThread(LPVOID lpParam)
{
	while(true)
	{
		//Check here the flag for offline registration
		if(CheckInternetConnection())
		{
			//Send User Information from here.
			SendUserInformation2Server(g_updateInfo ? UPDATE_INFO : CHECK_PIRACY);
			g_updateInfo = false;
		}
		ResetEvent(hSendRegInfoEvent);
		ULONG64 dwWaitTime = 1000 * 60 * 60;
		WaitForSingleObject(hSendRegInfoEvent, dwWaitTime);
	}
	return 1;
}

/***************************************************************************************************    
*  Function Name  : CheckInternetConnection                                                     
*  Description    : Checks the internet connection is available or not
*  Author Name    : Ram krushna 
*  SR_NO		  : 
*  Date           : 25-OCt-2013
****************************************************************************************************/
bool CheckInternetConnection()
{
	bool bReturn = false;
	try
	{
		CWinHttpManager objWinHttpManager;
		TCHAR szTestUrl[MAX_PATH] = {0};
		_tcscpy_s(szTestUrl, MAX_PATH, _T("http://www.google.com"));
		if(objWinHttpManager.Initialize(szTestUrl))	
		{
			if(objWinHttpManager.CreateRequestHandle(NULL))
			{
				return true;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizSupportUIDlg::CheckInternetConnection", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
  Function Name  : WriteNumberOfDays
  Description    : Function which gets number of days & writes Machine ID into registry.
  SR.NO			 : 
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
  Modification	 : It will check no of days from server according to that it will show message 
				   that copy is not genuine. It will show message in case of serial key is wrong
					and no.of.days are zero.
***********************************************************************************************/
void SendUserInformation2Server(int iRegInfoType)
{
	__try
	{
		typedef DWORD (*SENDUSERINFO)( int ) ;

		SENDUSERINFO pSendUserInfo = NULL ;

		typedef DWORD(*GETDAYSLEFT_SERVICE)(DWORD);

		GETDAYSLEFT_SERVICE	GetDaysLeft = NULL;

		TCHAR	szTemp[512] = {0} ;

		GetModuleFileName( NULL, szTemp, 511 ) ;

		TCHAR	*pTemp = wcsrchr(szTemp, '\\' ) ;
		if( pTemp )
		{
			pTemp++ ;
			*pTemp = '\0' ;
		}

		wcscat(szTemp, L"VBREGISTRATION.DLL" ) ;

		DWORD dwRet = 0x00;
		HMODULE	hMod = LoadLibrary( szTemp ) ;

		if (hMod)
			GetDaysLeft = (GETDAYSLEFT_SERVICE)GetProcAddress(hMod, "GetDaysLeft");

		if( hMod )
			pSendUserInfo = (SENDUSERINFO) GetProcAddress( hMod, "SendUserInformation2Server" ) ;

		if (GetDaysLeft)
		{
			DWORD	dwProductID = 0x00;

			dwProductID = GetProductID();
			m_dwDaysLeft = GetDaysLeft(dwProductID);
		}

		//This is check because if in offline case , No of days are expired. It will not go for online update to server.
		if (m_dwDaysLeft > 0)
		{
			if (pSendUserInfo)
				dwRet = pSendUserInfo(iRegInfoType);

			if ((dwRet == PRODUCTEXPIRED) || (dwRet == INVALIDREGNUMBER)) //return product expired ;
			{
				//send to tray
				AddLogEntry(L"### Error in VibraniumComSrv :: SendUserInformation2Server as copy is not genuine", 0, 0, true, SECONDLEVEL);
				//SendMessage2Tray(SENDPRODUCTEXPTOTRAY);
				//Sleep(10);
				//SendMessage2UI(SENDPRODUCTEXPTOTRAY);
			}
		}

		if( hMod )
		{
			FreeLibrary( hMod ) ;
			hMod = NULL ;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WriteNumberOfDays", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
  Function Name  : Check4OfflineActivationFlag
  Description    : Function to check whether the registration is happened Online/Offline.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10th Oct 2014
***********************************************************************************************/
bool Check4OfflineActivationFlag()
{
	bool bReturn = false;

	DWORD dwData = 0x00;
	if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwRegUserType", dwData) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwRegUserType in Check4OfflineActivationFlag, KeyPath: %s", g_csRegKeyPath, 0, true, ZEROLEVEL);
		return false;
	}

	if(dwData == 0x01)
	{
		bReturn = true;
	}

	return bReturn;
}

/***************************************************************************
Function Name  : SendMessage2Tray
Description    : Send Respective Message to tray.
Author Name    : Jeena Mariam Saji
S.R. No        :
Date           : 13 Oct 2017
****************************************************************************/
DWORD SendMessage2Tray(int iRequest, bool bWait)
{
	DWORD dwReturn = 0x00;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(TRAY_SERVER, false);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in WardwizComSrv::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in WardwizComSrv::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
			dwReturn = szPipeData.dwValue;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizComSrv::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}

/***************************************************************************
Function Name  : SendMessage2Tray
Description    : Send Respective Message to tray.
Author Name    : Neha Gharge
S.R. No        :
Date           : 3 Dec 2015
****************************************************************************/
bool SendMessage2Tray(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(TRAY_SERVER, false);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in WardwizComSrv::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizComSrv::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : SendMessage2Tray
Description    : Send Respective Message to UI.
Author Name    : Neha Gharge
S.R. No        :
Date           : 3 Dec 2015
****************************************************************************/
bool SendMessage2UI(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(UI_SERVER, false);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in WardwizComSrv::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizComSrv::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : StopRunningOperations
Description    : Stop here the running operations.
Author Name    : Ramkrushna Shelke
Date           : 14/12/2015
****************************************************************************/
bool StopRunningOperations()
{
	bool bReturn = false;
	__try
	{
		//Stop here indexing file enumeration.
		g_objISpyScannerBase.m_bStopScan = true;

		if (!g_objISpyScannerBase.StopScan(g_objISpyScannerBase.m_eScanType))
		{
			AddLogEntry(L"### Failed to stop scan in StopRunningOperations", 0, 0, true, SECONDLEVEL);
		}

		if (hThreadLoadSigDatabase != NULL)
		{
			::SuspendThread(hThreadLoadSigDatabase);
			::TerminateThread(hThreadLoadSigDatabase, 0);
			hThreadLoadSigDatabase = NULL;
		}

		Sleep(50);

		if (hThread_RepairViruses != NULL)
		{
			::SuspendThread(hThread_RepairViruses);
			::TerminateThread(hThread_RepairViruses, 0);
			hThread_RepairViruses = NULL;
		}

		Sleep(50);


		if (hThread_WatchReportsAction != NULL)
		{
			::SuspendThread(hThread_WatchReportsAction);
			::TerminateThread(hThread_WatchReportsAction, 0);
			hThread_WatchReportsAction = NULL;
		}

		Sleep(50);

		if (hSendUserInformationThread != NULL)
		{
			::SuspendThread(hSendUserInformationThread);
			::TerminateThread(hSendUserInformationThread, 0);
			hSendUserInformationThread = NULL;
		}

		Sleep(50);

		if (hIndexingThread != NULL)
		{
			::SuspendThread(hIndexingThread);
			::TerminateThread(hIndexingThread, 0);
			hIndexingThread = NULL;
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StopRunningOperations", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : CheckIsOffline
Description    : Check is offline registration (registry entry).
Author Name    : Nitin K.
Date           : 14/12/2015
****************************************************************************/
void CheckIsOffline()
{
	try
	{
		DWORD dwOffLineValue = 0;
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwIsOffline", dwOffLineValue) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwIsOffline in WardwizComSrv::CheckIsOffline, KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);;
			return;
		}
		switch (dwOffLineValue)
		{
		case 0x00:
			m_dwIsOffline = dwOffLineValue;
			break;
		case 0x01:
			m_dwIsOffline = dwOffLineValue;
			break;
		default:
			m_dwIsOffline = dwOffLineValue;
			AddLogEntry(L" ### Could not find IsOffLine:WardwizComSrv:Please reinstall setup of Wardwiz", 0, 0, true, SECONDLEVEL);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to fetch registry entry VibraniumCommSrv::CheckIsOffline", 0, 0, true, SECONDLEVEL);
	}
}


/**********************************************************************************************************
*  Function Name  :	AddProtectionToLiveUpdateFolder
*  Description    :	this function is use to add protection of live update folder
*  Author Name    : Lalit kumawat
*  Date           :
*  SR_NO		  :
**********************************************************************************************************/
bool AddProtectionToLiveUpdateFolder()
{
	bool bReturn = false;
	try
	{
		TCHAR szCommonAppPath[MAX_PATH] = { 0 };
		CString csDosDriveFormatPath = L"";
		CString csActualPath = L"";
		TCHAR szCommonAppPathWithDosDriveFormat[MAX_PATH] = { 0 };
		TCHAR szCompleteLiveUpdPath[MAX_PATH] = { 0 };
		CScannerLoad scannerObj;

		// to get all user path 
		//issue live update folder in window xp not getting protected. resolve by lalit kumawat
		if (GetEnvironmentVariable(L"ALLUSERSPROFILE", szCommonAppPath, 255) == 0)
		{
			return bReturn;
		}

		if (!szCommonAppPath)
		{
			return bReturn;
		}

		csActualPath = szCommonAppPath;
		csDosDriveFormatPath = scannerObj.GetPathToDriveVolume(csActualPath);
		swprintf_s(szCommonAppPathWithDosDriveFormat, _countof(szCommonAppPathWithDosDriveFormat), L"%s\\WardWiz", csDosDriveFormatPath);
		swprintf_s(szCompleteLiveUpdPath, _countof(szCompleteLiveUpdPath), L"%s\\WardWiz", csActualPath);

		if (GetFileAttributes(szCompleteLiveUpdPath) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory(szCompleteLiveUpdPath, NULL);
		}

		if (!PathFileExists(szCompleteLiveUpdPath))
		{
			return bReturn;
		}

		if (!csActualPath.IsEmpty())
		{
			scannerObj.ProtectFolderSEH(szCommonAppPathWithDosDriveFormat);
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CComSrv::AddProtectionToLiveUpdateFolder", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : HandleVirusEntry
Description    : Function to handle Detected virus entries sent from client.
Author Name    : Ram Shelke
Date           : 1/7/2016
****************************************************************************/
DWORD HandleVirusEntry(LPISPY_PIPE_DATA lpSpyData)
{
	DWORD dwRet = 0x00;
	try
	{
		if (!lpSpyData)
		{
			return dwRet;
		}

		CString csDuplicateFileName;
		DWORD dwAction = 0x00;
		dwRet = g_objISpyScannerBase.HandleVirusEntry(lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->szThirdParam, lpSpyData->dwValue, lpSpyData->dwSecondValue, csDuplicateFileName, dwAction);
		lpSpyData->dwValue = dwRet;
		_tcscpy_s(lpSpyData->szFirstParam, sizeof(lpSpyData->szFirstParam), csDuplicateFileName);
		//_tcscpy_s(lpSpyData->szSecondParam, sizeof(lpSpyData->szSecondParam), csQurStatus);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in HandleVirusEntry, Threat Path: %s", lpSpyData->szFirstParam, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************
Function Name  : CreateRequiredDirs
Description    : Create Required directories on service start.
Author Name    : Ram Shelke
Date           : 1/7/2016
****************************************************************************/
bool CreateRequiredDirs()
{
	bool bRet = false;
	try
	{
		CString csProdtDirPath = GetWardWizPathFromRegistry();
		CString csReportsPath = csProdtDirPath + L"REPORTS";
		if (!PathFileExists(csReportsPath))
		{
			if (!CreateDirectory(csReportsPath, NULL))
			{
				AddLogEntry(L"### Failed to create Dir in CreateRequiredDirs: %s", csReportsPath, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CreateRequiredDirs", 0, 0, true, SECONDLEVEL);
	}
	return bRet;
}

/**********************************************************************************************************
*  Function Name  :	GetSortedFileNames
*  Description    :	Function to get file ID's in sorted order.
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
bool GetSortedFileNames(vector<int> &vec)
{
	try
	{
		CString csQurFolderPath = GetQuarantineFolderPath();
		GetFileDigits(csQurFolderPath, vec);
		sort(vec.begin(), vec.end());

		if (vec.size() > 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetSortedFileNames", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetMaximumDigitFromFiles
*  Description    :	Function to get Maximum number from files
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
void GetFileDigits(LPCTSTR pstr, vector<int> &vec)
{
	try
	{
		CFileFind finder;
		bool bIsFolder = false;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.DB");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bIsFolder = true;
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				continue;
			}

			CString csFileName = finder.GetFileName();
			CString csDigit = csFileName.Left(csFileName.ReverseFind(L'.'));
			csDigit = csDigit.Right(csDigit.GetLength() - (csDigit.ReverseFind(L'_') + 1));
			if (csDigit.Trim().GetLength() != 0)
			{
				DWORD dwDigit = _wtoi(csDigit);
				if (dwDigit != 0)
				{
					vec.push_back(dwDigit);
				}
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in EnumTotalFolder", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	LoadRemainingEntries
*  Description    :	Function to get remaining entries of recover.
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
void MergeRecoverEntries()
{
	try
	{
		vector <int > vecNumList;

		//get the file ids and sort
		GetSortedFileNames(vecNumList);

		if (vecNumList.size() == 0)
		{
			return;
		}

		CString csFilePath;
		csFilePath.Format(L"%s\\VBRECOVERENTRIES.DB", GetQuarantineFolderPath());
		LoadDataContentFromFile(csFilePath, g_objRecoverdb);

		//Load the file contents in memory and populate in list control
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{
			csFilePath.Format(L"%s\\VBRECOVERENTRIES_%d.DB", GetQuarantineFolderPath(), *it);
			if (PathFileExists(csFilePath))
			{
				CDataManager		objCurrentRdb;
				LoadDataContentFromFile(csFilePath, objCurrentRdb);
				MergeIntoRecoverDB(objCurrentRdb);
			}
		}

		//Save the DB file here
		SaveDBFile();

		//Remove the files which are already loaded and merged.
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{
			CString csFilePath;
			csFilePath.Format(L"%s\\VBRECOVERENTRIES_%d.DB", GetQuarantineFolderPath(), *it);
			if (PathFileExists(csFilePath))
			{
				SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
				if (DeleteFile(csFilePath) == FALSE)
				{
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LoadRemainingEntries", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	SaveDBFile
*  Description    :	According to type of module Save DB .
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0003
**********************************************************************************************************/
bool SaveDBFile()
{
	try
	{
		CString csFilePath = GetQuarantineFolderPath();
		csFilePath += _T("\\VBRECOVERENTRIES.DB");

		int iFileVersion = 1;
		CFile wFile(csFilePath, CFile::modeCreate | CFile::modeWrite);

		CArchive arStore(&wFile, CArchive::store);

		g_objRecoverdb.SetFileVersion(iFileVersion);
		g_objRecoverdb.Serialize(arStore);

		arStore.Close();
		wFile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SaveDBFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	LoadDataContentFromFile
*  Description    :	Loads the input file content into serialization object.
*  Author Name    : Ramkrushna Shelke
*  Date           : 15 Jan 2016
**********************************************************************************************************/
bool LoadDataContentFromFile(CString csPathName, CDataManager &objdb)
{
	try
	{
		objdb.RemoveAll();

		if (!PathFileExists(csPathName))
			return false;

		CFile rFile(csPathName, CFile::modeRead);

		// Create a loading archive
		CArchive arLoad(&rFile, CArchive::load);
		objdb.Serialize(arLoad);

		// Close the loading archive
		arLoad.Close();
		rFile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LoadDataContentFromFile, File Path: %s", csPathName, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	MergeIntoRecoverDB
*  Description    :	Function which merges the entries in Recover DB
*  Author Name    : Ramkrushna Shelke
*  Date           : 15 Jan 2016
**********************************************************************************************************/
void MergeIntoRecoverDB(CDataManager &objdb)
{
	try
	{
		//int nCurrentItem;

		// get a reference to the contacts list
		const ContactList& contacts = objdb.GetContacts();

		// iterate over all contacts add add them to the list

		POSITION pos = contacts.GetHeadPosition();
		while (pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
			g_objRecoverdb.AddContact(contact);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MergeIntoRecoverDB", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : ScanFile
Description    : Function to scan single file using single loaded Database from service and return as per result of scan
Author Name    : Ram Shelke
SR_NO		   :
Date           : 25 Mar 2016
****************************************************************************/
DWORD ScanFile(LPISPY_PIPE_DATA lpSpyData)
{
	DWORD dwReturn = 0x00;
	g_objcsScanFile.Lock();
	__try
	{
		if (!lpSpyData)
		{
			g_objcsScanFile.Unlock();
			return 0;
		}

		TCHAR szVirusName[MAX_PATH] = { 0 };
		DWORD dwSpyID = 0;
		DWORD dwAction = 0;
		DWORD dwSignatureFailedToLoad = 0;
		dwReturn = g_objISpyScannerBase.ScanFile(lpSpyData->szFirstParam, szVirusName, dwSpyID, g_objISpyScannerBase.m_eScanType, dwAction, dwSignatureFailedToLoad, false);
		
		if (dwReturn > 0)
		{
			_tcscpy_s(lpSpyData->szSecondParam, szVirusName);
			lpSpyData->dwSecondValue = dwAction;
			(*(DWORD *)&lpSpyData->byData[0]) = dwSpyID;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ScanFile, FilePath: %s", lpSpyData->szFirstParam, 0, true, SECONDLEVEL);
	}

	g_objcsScanFile.Unlock();
	return dwReturn;
}

/***************************************************************************
Function Name  : ScanFile
Description    : Function to save maintained local DB cache from Memory to file.
Author Name    : Ram Shelke
SR_NO		   :
Date           : 25 Mar 2016
****************************************************************************/
bool SaveLocalDBFiles()
{
	bool bReturn = false;
	try
	{
		CWWizIndexer objIndexer;
		if (objIndexer.SaveLocalDatabase())
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SaveLocalDBFiles", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : IndexingThread
Description    : Thread to perform indexing of files.
SR.NO		   :
Author Name    : Ramkrushna Shelke
Date           : 15 Apr 2016
***********************************************************************************************/
DWORD IndexingThread(LPVOID lpParam)
{
	__try
	{
		//Wait for this operation then start
		DWORD dwStartWaitTime = 5* 60 * 1000; //5 minutes
		Sleep(dwStartWaitTime);

		TCHAR	szDrives[256] = { 0 };
		GetLogicalDriveStrings(255, szDrives);

		TCHAR	*pDrive = szDrives;
		while (wcslen(pDrive) > 2)
		{
			if ((GetDriveType(pDrive) & DRIVE_FIXED) == DRIVE_FIXED)
			{
				//Initialize to ZERO
				g_objISpyScannerBase.m_dwFilesIndexed = 0x00;

				AddLogEntry(L">>> Indexing started for drive: %s", pDrive, 0, true, SECONDLEVEL);
				g_objISpyScannerBase.m_bStopScan = false;
				g_objISpyScannerBase.EnumFolder(pDrive);
				AddLogEntry(L">>> Indexing Finished for drive: %s", pDrive, 0, true, SECONDLEVEL);
				
				TCHAR szLog[MAX_PATH] = { 0 };
				swprintf_s(szLog, L">>> Drive Name: %s, Total files Indexed: %d", pDrive, g_objISpyScannerBase.m_dwFilesIndexed);
				AddLogEntry(szLog, 0, 0, true, SECONDLEVEL);

				//Save local DB indexing on hard drive
				SaveLocalDBFiles();
			}
			pDrive += 0x04;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in IndexingThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
	Function Name  : StartIndexing
	Description    : This function will start indexing as per user setting.
	SR.NO		   : 
	Author Name    : Ramkrushna Shelke
	Date           : 06 May 2016
***********************************************************************************************/
void StartIndexing()
{
	//As per user settings for background indexing will start.
	DWORD dwBackgroundCaching = 0;
	if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwBackgroundCaching", dwBackgroundCaching) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwBackgroundCaching in StartIndexing, KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);;
		return;
	}

	if (dwBackgroundCaching == 0x01)
	{
		if (hIndexingThread == NULL)
		{
			//Start here indexing thread.
			hIndexingThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)IndexingThread,
				NULL, 0, 0);
			SetThreadPriority(hIndexingThread, THREAD_PRIORITY_LOWEST);
			Sleep(100);
		}
	}
}

/***********************************************************************************************
Function Name  : LoadRequiredLibrary
Description    : This function will load all required libraries
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 29 September 2017
***********************************************************************************************/
bool LoadRequiredLibrary()
{
	bool bReturn = false;
	try
	{
		DWORD	dwRet = 0x00;
		CString	csDLLPath = L"";
		csDLLPath.Format(L"%s\\SQLITE3.DLL", GetWardWizPathFromRegistry());
		if (!PathFileExists(csDLLPath))
		{
			return false;
		}

		if (!m_hSQLiteDLL)
		{
			m_hSQLiteDLL = LoadLibrary(csDLLPath);
			if (!m_hSQLiteDLL)
			{
				AddLogEntry(L"### Failed to LoadLibrary [%s]", csDLLPath, 0, true, SECONDLEVEL);
				return false;
			}
		}

		m_pSQliteOpen = (SQLITE3_OPEN)GetProcAddress(m_hSQLiteDLL, "sqlite3_open");
		m_pSQLitePrepare = (SQLITE3_PREPARE)GetProcAddress(m_hSQLiteDLL, "sqlite3_prepare");
		m_pSQLiteColumnCount = (SQLITE3_COLUMN_COUNT)GetProcAddress(m_hSQLiteDLL, "sqlite3_column_count");
		m_pSQLiteStep = (SQLITE3_STEP)GetProcAddress(m_hSQLiteDLL, "sqlite3_step");
		m_pSQLiteColumnText = (SQLITE3_COLUMN_TEXT)GetProcAddress(m_hSQLiteDLL, "sqlite3_column_text");
		m_pSQLiteClose = (SQLITE3_CLOSE)GetProcAddress(m_hSQLiteDLL, "sqlite3_close");

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : StartScheduledScanThread
Description    : This function will start scheduled scan thread
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 29 September 2017
***********************************************************************************************/
bool StartScheduledScanThread()
{
	try
	{
		if (hSchedScanThread == NULL)
		{
			hSchedScanThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SchedScanThread, NULL, 0, 0);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : SchedScanThread
Description    : This function will start scheduled scan
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 4 October 2017
***********************************************************************************************/
DWORD SchedScanThread(LPVOID lpParam)
{
	__try
	{
		DWORD dwScheduledScanOption = 0x00;
		DWORD dwWaitTime = 2 * 1000 * 60;//2 minutes
		while (1)
		{
			WaitForSingleObject(hSchScanEvent, dwWaitTime);

			if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwScheduledScan", dwScheduledScanOption) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwScheduledScan in SchedScanThreead KeyPath: %s", g_csRegKeyPath, 0, true, ZEROLEVEL);;
			}
			if (dwScheduledScanOption)
			{
				GetRecordsSEH();
			}
			ResetEvent(hSchScanEvent);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SchedScanThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : GetRecordsSEH
Description    : This function will fetch records from database and compare them
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 4 October 2017
***********************************************************************************************/
void GetRecordsSEH()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBSETTINGS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		//For Settings in Scheduled Scan

		dbSQlite.Open();
		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from WWIZSCHEDSCANTIME;");

		Sleep(20);

		m_vSchedScanList.clear(); 
		m_vSchedScanStng.clear();
		char szData[512] = { 0 };
		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);
			SCHEDSCANSTNG szSchedScanStng;

			if (qResult.GetFieldIsNull(0))
			{
				continue;
			}

			char sziStngID[10] = { 0 };
			strcpy_s(sziStngID, qResult.GetFieldValue(0));
			if (strlen(sziStngID) == 0)
			{
				continue;
			}
			szSchedScanStng.iStngID = atoi(sziStngID);

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}

			char sziSchedType[10] = { 0 };
			strcpy_s(sziSchedType, qResult.GetFieldValue(1));
			szSchedScanStng.iSchedType = sziSchedType[0];

			if (qResult.GetFieldIsNull(2))
			{
				continue;
			}

			char sziSchedLowBat[10] = { 0 };
			strcpy_s(sziSchedLowBat, qResult.GetFieldValue(2));
			if (strlen(sziSchedLowBat) == 0)
			{
				continue;
			}
			szSchedScanStng.iSchedLowBat = atoi(sziSchedLowBat);

			if (qResult.GetFieldIsNull(3))
			{
				continue;
			}

			char sziSchedShutDown[10] = { 0 };
			strcpy_s(sziSchedShutDown, qResult.GetFieldValue(3));
			if (strlen(sziSchedShutDown) == 0)
			{
				continue;
			}
			szSchedScanStng.iSchedShutDown = atoi(sziSchedShutDown);

			if (qResult.GetFieldIsNull(4))
			{
				continue;
			}

			char sziSchedWakeUp[10] = { 0 };
			strcpy_s(sziSchedWakeUp, qResult.GetFieldValue(4));
			if (strlen(sziSchedWakeUp) == 0)
			{
				continue;
			}
			szSchedScanStng.iSchedWakeUp = atoi(sziSchedWakeUp);

			if (qResult.GetFieldIsNull(5))
			{
				continue;
			}

			char sziSchedScanSkip[10] = { 0 };
			strcpy_s(sziSchedScanSkip, qResult.GetFieldValue(5));
			if (strlen(sziSchedScanSkip) == 0)
			{
				continue;
			}
			szSchedScanStng.iSchedScanSkip = atoi(sziSchedScanSkip);

			if (qResult.GetFieldIsNull(6))
			{
				continue;
			}

			char szSchedScanTime[10] = { 0 };
			strcpy_s(szSchedScanTime, qResult.GetFieldValue(6));
			if (strlen(szSchedScanTime) == 0)
			{
				continue;
			}
			szSchedScanStng.SchedScanTime = atoi(szSchedScanTime);

			if (qResult.GetFieldIsNull(7))
			{
				continue;
			}

			char szISScanSun[10] = { 0 };
			strcpy_s(szISScanSun, qResult.GetFieldValue(7));
			szSchedScanStng.byScanSun = szISScanSun[0];

			if (qResult.GetFieldIsNull(8))
			{
				continue;
			}

			char szISScanMon[10] = { 0 };
			strcpy_s(szISScanMon, qResult.GetFieldValue(8));
			szSchedScanStng.byScanMon = szISScanMon[0];

			if (qResult.GetFieldIsNull(9))
			{
				continue;
			}

			char szISScanTue[10] = { 0 };
			strcpy_s(szISScanTue, qResult.GetFieldValue(9));
			szSchedScanStng.byScanTue = szISScanTue[0];

			if (qResult.GetFieldIsNull(10))
			{
				continue;
			}

			char szISScanWed[10] = { 0 };
			strcpy_s(szISScanWed, qResult.GetFieldValue(10));
			szSchedScanStng.byScanWed = szISScanWed[0];

			if (qResult.GetFieldIsNull(11))
			{
				continue;
			}

			char szISScanThur[10] = { 0 };
			strcpy_s(szISScanThur, qResult.GetFieldValue(11));
			szSchedScanStng.byScanThur = szISScanThur[0];

			if (qResult.GetFieldIsNull(12))
			{
				continue;
			}

			char szISScanFri[10] = { 0 };
			strcpy_s(szISScanFri, qResult.GetFieldValue(12));
			szSchedScanStng.byScanFri = szISScanFri[0];

			if (qResult.GetFieldIsNull(13))
			{
				continue;
			}

			char szISScanSat[10] = { 0 };
			strcpy_s(szISScanSat, qResult.GetFieldValue(13));
			szSchedScanStng.byScanSat = szISScanSat[0];

			if (qResult.GetFieldIsNull(14))
			{
				continue;
			}

			char sziScanType[10] = { 0 };
			strcpy_s(sziScanType, qResult.GetFieldValue(14));
			szSchedScanStng.iScanType = sziScanType[0];

			if (qResult.GetFieldIsNull(15))
			{
				continue;
			}

			char szSchedScanDate[15] = { 0 };
			strcpy_s(szSchedScanDate, qResult.GetFieldValue(15));

			//Extract day, month, year from string.
			int iDay = 0;
			int iMonth = 0;
			int iYear = 0;

			char seps[] = "-";
			char *token = NULL;
			char* context = NULL;

			int iIndex = 0;
			token = strtok_s(szSchedScanDate, seps, &context);
			while (token != NULL)
			{
				if (strlen(token) > 0)
				{
					int iTokenValue = atoi(token);
					switch (iIndex)
					{
					case 0:
						iYear = iTokenValue;
						break;
					case 1:
						iMonth = iTokenValue;
						break;
					case 2:
						iDay = iTokenValue;
						break;
					default:
						break;
					}
				}
				token = strtok_s(NULL, seps, &context);
				iIndex++;
			}
			
			szSchedScanStng.SchedScanDate.wYear = iYear;
			szSchedScanStng.SchedScanDate.wMonth = iMonth;
			szSchedScanStng.SchedScanDate.wDay = iDay;

			m_vSchedScanStng.push_back(szSchedScanStng);
		}
		dbSQlite.Close();
		
		//Comparison operation starts here:

		CString csCurrentSec = CTime::GetCurrentTime().Format("%S");
		int iCurrentSec = _ttoi(csCurrentSec);
		CString csCurrentMin = CTime::GetCurrentTime().Format("%M");
		int iCurrentMin = _ttoi(csCurrentMin);
		CString csCurrentHour = CTime::GetCurrentTime().Format("%H");
		int iCurrentHour = _ttoi(csCurrentHour);
		int lpSchCurrTime = iCurrentSec + (iCurrentMin * 60) + (iCurrentHour * 3600);
		int iSchedScanTypeFin;
		int iScanTypeFin;
		int iScanID;
		DWORD dwReply = 0;
		bool bScanMissed = false;

		SCHEDSCANSTNGMAP::iterator vSchedScanList;
		for (vSchedScanList = m_vSchedScanStng.begin(); vSchedScanList != m_vSchedScanStng.end(); vSchedScanList++)
		{
			iScanID = vSchedScanList->iStngID;
			CString csScanCmdParam;
			int iShutDown = vSchedScanList->iSchedShutDown;
			int iLowBattery = vSchedScanList->iSchedLowBat;
			if (iLowBattery == 1)
			{
				SYSTEM_POWER_STATUS status;
				if(GetSystemPowerStatus(&status) != 0)
				{
					int iBatteryLife = status.BatteryLifePercent;
					if (iBatteryLife <= 15)
					{
						continue;
					}
				}
			}
			int iScanMissed = vSchedScanList->iSchedScanSkip;
			if (iScanMissed == 0)
			{
				bScanMissed = false;
			}
			else
			{
				bScanMissed = true;
			}

			int lpSchTime = vSchedScanList->SchedScanTime;
			if (CheckForCompletedScan(lpSchTime))
			{
				continue;
			}
			BYTE lpHash = vSchedScanList->iSchedType;
			BYTE lpHashValFin = vSchedScanList->iScanType;
			if (lpHashValFin == SCANQUICK)
			{
				iScanTypeFin = 0;
			}
			else if (lpHashValFin == SCANFULL)
			{
				iScanTypeFin = 1;
			}
			else if (lpHashValFin == SCANSPEC)
			{
				iScanTypeFin = 2;
				GetFileFoldList(iScanID, csScanCmdParam);
			}
			else if (lpHashValFin == TMPFILECLNR)
			{
				iScanTypeFin = 4;
			}
			else if (lpHashValFin == REGOPT)
			{
				iScanTypeFin = 5;
				GetRegOptionList(iScanID, csScanCmdParam);
			}
			DWORD dwValue = (DWORD)iScanTypeFin;
			double diff = difftime(lpSchTime, lpSchCurrTime);
			if (diff <= 120 && diff > 0)
			{
				if (lpHash == SCANDAILY)
				{
					iSchedScanTypeFin = 1;
					StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
				}
				else if (lpHash == SCANWEEKLY)
				{
					iSchedScanTypeFin = 2;
					int iDayWeek = CTime::GetCurrentTime().GetDayOfWeek();
					if (iDayWeek == ISSUNDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanSun;
						if (lpHashVal == SCANSUNDAY)
						{
							StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
						}
						else
						{
							break;
						}
					}
					else if (iDayWeek == ISMONDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanMon;
						if (lpHashVal == SCANMONDAY)
						{
							StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
						}
						else
						{
							break;
						}
					}
					else if (iDayWeek == ISTUESDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanTue;
						if (lpHashVal == SCANTUESDAY)
						{
							StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
						}
						else
						{
							break;
						}
					}
					else if (iDayWeek == ISWEDNESDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanWed;
						if (lpHashVal == SCANWEDNESDAY)
						{
							StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
						}
						else
						{
							break;
						}
					}
					else if (iDayWeek == ISTHURSDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanThur;
						if (lpHashVal == SCANTHURSDAY)
						{
							StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
						}
						else
						{
							break;
						}
					}
					else if (iDayWeek == ISFRIDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanFri;
						if (lpHashVal == SCANFRIDAY)
						{
							StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
						}
						else
						{
							break;
						}
					}
					else if (iDayWeek == ISSATURDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanSat;
						if (lpHashVal == SCANSATURDAY)
						{
							StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
						}
						else
						{
							break;
						}
					}
					else
					{
						DWORD dwYear = vSchedScanList->SchedScanDate.wYear;
						DWORD dwMonth = vSchedScanList->SchedScanDate.wMonth;
						DWORD dwDay = vSchedScanList->SchedScanDate.wDay;

						int intYear = (int)dwYear;
						int intMonth = (int)dwMonth;
						int intDay = (int)dwDay;

						CTime cTime = CTime::GetCurrentTime();
						CTime Time_DBEntry(intYear, intMonth, intDay, 0, 0, 0);
						CTimeSpan Time_Diff = cTime - Time_DBEntry;
						int iSpan = static_cast<int>(Time_Diff.GetDays());

						if (iSpan == 7)
						{
							StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
						}
					}
				}
				else if (lpHash == SCANMONTHLY)
				{
					iSchedScanTypeFin = 3;
					
					DWORD dwYear = vSchedScanList->SchedScanDate.wYear;
					DWORD dwMonth = vSchedScanList->SchedScanDate.wMonth;
					DWORD dwDay = vSchedScanList->SchedScanDate.wDay;

					int intYear = (int)dwYear;
					int intMonth = (int)dwMonth;
					int intDay = (int)dwDay;

					CTime cTime = CTime::GetCurrentTime();
					CTime Time_DBEntry(intYear, intMonth, intDay, 0, 0, 0);
					CTimeSpan Time_Diff = cTime - Time_DBEntry;
					int iSpan = static_cast<int>(Time_Diff.GetDays());

					int iDiffDays = CheckForMonth(intMonth, intYear);
					if (iSpan == iDiffDays || iSpan == 0)
					{
						StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
					}
				}
			}
			else if (diff < 0)
			{
				if (lpHash == SCANDAILY)
				{
					iSchedScanTypeFin = 1;
				}
				else if (lpHash == SCANWEEKLY)
				{
					iSchedScanTypeFin = 2;
					int iDayWeek = CTime::GetCurrentTime().GetDayOfWeek();
					if (iDayWeek == ISSUNDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanSun;
						if (lpHashVal != SCANSUNDAY)
						{
							continue;
						}
					}
					else if (iDayWeek == ISMONDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanMon;
						if (lpHashVal != SCANMONDAY)
						{
							continue;
						}
					}
					else if (iDayWeek == ISTUESDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanTue;
						if (lpHashVal != SCANTUESDAY)
						{
							continue;
						}
					}
					else if (iDayWeek == ISWEDNESDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanWed;
						if (lpHashVal != SCANWEDNESDAY)
						{
							continue;
						}
					}
					else if (iDayWeek == ISTHURSDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanThur;
						if (lpHashVal != SCANTHURSDAY)
						{
							continue;
						}
					}
					else if (iDayWeek == ISFRIDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanFri;
						if (lpHashVal != SCANFRIDAY)
						{
							continue;
						}
					}
					else if (iDayWeek == ISSATURDAY)
					{
						BYTE lpHashVal = vSchedScanList->byScanSat;
						if (lpHashVal != SCANSATURDAY)
						{
							continue;
						}
					}
				}
				else if (lpHash == SCANMONTHLY)
				{
					iSchedScanTypeFin = 3;
					DWORD dwYear = vSchedScanList->SchedScanDate.wYear;
					DWORD dwMonth = vSchedScanList->SchedScanDate.wMonth;
					DWORD dwDay = vSchedScanList->SchedScanDate.wDay;

					int intYear = (int)dwYear;
					int intMonth = (int)dwMonth;
					int intDay = (int)dwDay;

					CTime cTime = CTime::GetCurrentTime();
					int iCurDate = cTime.GetDay();
					CTime Time_DBEntry(intYear, intMonth, intDay, 0, 0, 0);
					CTimeSpan Time_Diff = cTime - Time_DBEntry;
					int iSpan = static_cast<int>(Time_Diff.GetDays());

					int iDiffDays = CheckForMonth(intMonth, intYear);
					if (iSpan != iDiffDays && iSpan != 0)
					{
						continue;
					}
					if (iSpan == 0 && intDay != iCurDate)
					{
						continue;
					}
				}
				if (!CheckForCompletedScan(lpSchTime) && (!CheckForMissedDuplicate(lpSchTime)) && bScanMissed)
				{
					DWORD dwRet = SendMessage2Tray(SEND_SCHED_SCAN_MISSED, true);
					
					if (dwRet == YES)	//User selected YES, means to start scan.
					{
						StartSchedScanProcess(iScanID, dwValue, iSchedScanTypeFin, lpSchTime, iScanTypeFin, csScanCmdParam, iShutDown);
					}
					else
					{
						dbSQlite.Open();
						CString csInsertQuery = _T("INSERT INTO WWIZSCHEDSCANMISS VALUES (null,");
						csInsertQuery.Format(_T("INSERT INTO WWIZSCHEDSCANMISS VALUES (null,%d,%d,%d,%d,Date('now'),0,0,0);"), iScanID, iSchedScanTypeFin, lpSchTime, iScanTypeFin);
						CT2A asciiSession(csInsertQuery, CP_UTF8);
						INT64 iScanSessionId = InsertSQLiteDataForStng(asciiSession.m_psz);
						dbSQlite.Close();
					}
				}
			}
		}
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in GetRecordsSEH, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : GetFileFoldList
Description    : To get File Folder list for scanning
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 27 October 2017
***********************************************************************************************/
void GetFileFoldList(int iScanID, CString &csScanCmdParam)
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBSETTINGS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		//For Files and Folders in db

		dbSQlite.Open();

		char szquery[MAX_PATH] = { 0 };
		sprintf(szquery, "Select * from WWIZSCHEDSCAN where SCHEDSCAN_ID = %I64d;", iScanID);
		CWardwizSQLiteTable qSchedResult = dbSQlite.GetTable(szquery);

		Sleep(20);

		//Clear here for list of files
		m_vSchedScanList.clear();
		for (int iRow = 0; iRow < qSchedResult.GetNumRows(); iRow++)
		{
			qSchedResult.SetRow(iRow);
			SCHEDSCANLIST szSchedScanList;

			if (qSchedResult.GetFieldIsNull(1))
			{
				continue;
			}

			//to read unicode string from db
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qSchedResult.GetFieldValue(1), -1, NULL, 0);
			wchar_t *wstrFileFolderPath = new wchar_t[wchars_num];
			if (wstrFileFolderPath == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in GetFileFoldList", 0, 0, true, SECONDLEVEL);
				continue;
			}
			MultiByteToWideChar(CP_UTF8, 0, qSchedResult.GetFieldValue(1), -1, wstrFileFolderPath, wchars_num);

			wcscpy_s(szSchedScanList.szData, wstrFileFolderPath);

			delete[] wstrFileFolderPath;

			_wcslwr(szSchedScanList.szData);

			m_vSchedScanList.push_back(szSchedScanList);
		}

		dbSQlite.Close();

		SCHEDSCANLISTMAP::iterator vSchedScanFileFold;

		CString csCommandline;
		for (vSchedScanFileFold = m_vSchedScanList.begin(); vSchedScanFileFold != m_vSchedScanList.end(); vSchedScanFileFold++)
		{
			WCHAR *charTempData = vSchedScanFileFold->szData;
			CString csFilePath(charTempData);

			csCommandline += csFilePath;
			csCommandline += L"|";
		}
		csScanCmdParam = csCommandline;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetFileFoldList", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : GetRegOptionList
Description    : To get selected registry option list for scanning
Author Name    : Amol Jaware
Date           : 22 March 2019
***********************************************************************************************/
void GetRegOptionList(int iScanID, CString &csScanCmdParam)
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBSETTINGS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		char szquery[MAX_PATH] = { 0 };
		sprintf(szquery, "Select * from WWIZREGOPTIMIZER where SCHEDSCAN_ID = %I64d;", iScanID);
		CWardwizSQLiteTable qSchedResult = dbSQlite.GetTable(szquery);

		Sleep(20);

		if (m_csArrRegOptions.GetCount() > 0)
			m_csArrRegOptions.RemoveAll();

		for (int iRow = 0; iRow < qSchedResult.GetNumRows(); iRow++)
		{
			qSchedResult.SetRow(iRow);
			
			if (qSchedResult.GetFieldIsNull(2))
				continue;
			char chComEntry[1] = { 0 };
			strcpy(chComEntry, qSchedResult.GetFieldValue(2));
			m_csArrRegOptions.Add(chComEntry);
			
			if (qSchedResult.GetFieldIsNull(3))
				continue;
			char chUninstEntry[1] = { 0 };
			strcpy(chUninstEntry, qSchedResult.GetFieldValue(3));
			m_csArrRegOptions.Add(chUninstEntry);

			if (qSchedResult.GetFieldIsNull(4))
				continue;
			char chFontEntry[1] = { 0 };
			strcpy(chFontEntry, qSchedResult.GetFieldValue(4));
			m_csArrRegOptions.Add(chFontEntry);

			if (qSchedResult.GetFieldIsNull(5))
				continue;
			char csShareDll[1] = { 0 };
			strcpy(csShareDll, qSchedResult.GetFieldValue(5));
			m_csArrRegOptions.Add(csShareDll);

			if (qSchedResult.GetFieldIsNull(6))
				continue;
			char chAppPath[1] = { 0 };
			strcpy(chAppPath, qSchedResult.GetFieldValue(6));
			m_csArrRegOptions.Add(chAppPath);

			if (qSchedResult.GetFieldIsNull(7))
				continue;
			char chHelpFileInfo[1] = { 0 };
			strcpy(chHelpFileInfo, qSchedResult.GetFieldValue(7));
			m_csArrRegOptions.Add(chHelpFileInfo);

			if (qSchedResult.GetFieldIsNull(8))
				continue;
			char chWindStartItem[1] = { 0 };
			strcpy(chWindStartItem, qSchedResult.GetFieldValue(8));
			m_csArrRegOptions.Add(chWindStartItem);

			if (qSchedResult.GetFieldIsNull(9))
				continue;
			char chWindSrv[1] = { 0 };
			strcpy(chWindSrv, qSchedResult.GetFieldValue(9));
			m_csArrRegOptions.Add(chWindSrv);

			if (qSchedResult.GetFieldIsNull(10))
				continue;
			char chInvalidExt[1] = { 0 };
			strcpy(chInvalidExt, qSchedResult.GetFieldValue(10));
			m_csArrRegOptions.Add(chInvalidExt);

			if (qSchedResult.GetFieldIsNull(11))
				continue;
			char chRootkit[1] = { 0 };
			strcpy(chRootkit, qSchedResult.GetFieldValue(11));
			m_csArrRegOptions.Add(chRootkit);

			if (qSchedResult.GetFieldIsNull(12))
				continue;
			char chRougeApp[1] = { 0 };
			strcpy(chRougeApp, qSchedResult.GetFieldValue(12));
			m_csArrRegOptions.Add(chRougeApp);

			if (qSchedResult.GetFieldIsNull(13))
				continue;
			char chWorms[1] = { 0 };
			strcpy(chWorms, qSchedResult.GetFieldValue(13));
			m_csArrRegOptions.Add(chWorms);

			if (qSchedResult.GetFieldIsNull(14))
				continue;
			char chSpyThreat[1] = { 0 };
			strcpy(chSpyThreat, qSchedResult.GetFieldValue(14));
			m_csArrRegOptions.Add(chSpyThreat);

			if (qSchedResult.GetFieldIsNull(15))
				continue;
			char chAdwThreat[1] = {0}; 
			strcpy(chAdwThreat, qSchedResult.GetFieldValue(15));
			m_csArrRegOptions.Add(chAdwThreat);

			if (qSchedResult.GetFieldIsNull(16))
				continue;
			char chKeyLog[1] = { 0 };
			strcpy(chKeyLog, qSchedResult.GetFieldValue(16));
			m_csArrRegOptions.Add(chKeyLog);

			if (qSchedResult.GetFieldIsNull(17))
				continue;
			char chBho[1] = { 0 };
			strcpy(chBho, qSchedResult.GetFieldValue(17));
			m_csArrRegOptions.Add(chBho);

			if (qSchedResult.GetFieldIsNull(18))
				continue;
			char chExplEntry[1] = { 4 };
			strcpy(chExplEntry, qSchedResult.GetFieldValue(18));
			m_csArrRegOptions.Add(chExplEntry);

			if (qSchedResult.GetFieldIsNull(19))
				continue;
			char chInetExpEntry[1] = { 0 };
			strcpy(chInetExpEntry, qSchedResult.GetFieldValue(19));
			m_csArrRegOptions.Add(chInetExpEntry);

		}

		dbSQlite.Close();

		CString csCommandline;
		for (int iCount = 0; iCount < m_csArrRegOptions.GetCount(); iCount++)
		{
			csCommandline += m_csArrRegOptions.GetAt(iCount);
			csCommandline += L"|";
		}
		csScanCmdParam = csCommandline;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetRegOptionList", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckForMonth
Description    : This function will check for month
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 05 October 2017
***********************************************************************************************/
int CheckForMonth(int intMonth, int intYear)
{
	int iDiffDays = 0;
	try
	{
		if (intMonth == 4 || intMonth == 6 || intMonth == 9 || intMonth == 11)
		{
			iDiffDays = 30;
		}
		else if (intMonth == 2)
		{
			bool isLeapYear = (intYear % 4 == 0 && intYear % 100 != 0) || (intYear % 400 == 0);
			if (isLeapYear)
			{
				iDiffDays = 29;
			}
			else
			{
				iDiffDays = 28;
			}
		}
		else
		{
			iDiffDays = 31;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CheckForMonth", 0, 0, true, SECONDLEVEL);
	}
	return iDiffDays;
}

/***********************************************************************************************
Function Name  : StartSchedScanProcess
Description    : This function will start scheduled scan
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 04 October 2017
***********************************************************************************************/
void StartSchedScanProcess(int iScanID, DWORD dwScanType, int iSchedScanTypeFin, int lpSchTime, int iScanTypeFin, CString csScanCmdParam, int iShutDown)
{
	try
	{
		StartScheduledScanner(dwScanType, csScanCmdParam, iShutDown);
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBSETTINGS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		dbSQlite.Open();
		CString csInsertQuery = _T("INSERT INTO WWIZSCHEDSCANFINISH VALUES (null,");
		csInsertQuery.Format(_T("INSERT INTO WWIZSCHEDSCANFINISH VALUES (null,%d,%d,%d,Date('now'),0,0,0);"), iSchedScanTypeFin, lpSchTime, iScanTypeFin);
		CT2A asciiSession(csInsertQuery, CP_UTF8);
		INT64 iScanSessionId = InsertSQLiteDataForStng(asciiSession.m_psz);
		dbSQlite.Close();

		if (iSchedScanTypeFin == 3)
		{
			dbSQlite.Open();
			CString csInsertQuery = _T("UPDATE WWIZSCHEDSCANTIME VALUES (null,");
			csInsertQuery.Format(_T("UPDATE WWIZSCHEDSCANTIME SET SCHEDSCANDATE = Date('now')WHERE ID = %I64d;"), iScanID);
			CT2A asciiSession(csInsertQuery, CP_UTF8);
			INT64 iScanSessionId = InsertSQLiteDataForStng(asciiSession.m_psz);
			dbSQlite.Close();
		}
		//Sleep(10 * 1000 * 60);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartSchedScanProcess", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckForCompletedScan
Description    : This function will check for already completed scans
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 04 October 2017
***********************************************************************************************/
bool CheckForCompletedScan(int iSchedtime)
{
	bool bIsScanFinish = false;
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBSETTINGS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return true;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		//For Settings in Scheduled Scan

		dbSQlite.Open();
		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from WWIZSCHEDSCANFINISH;");

		m_vSchedScanFinish.clear();

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);
			SCHEDSCANFINISH szFinishScanList;

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}
			char sziSchType[10] = { 0 };

			strcpy_s(sziSchType, qResult.GetFieldValue(1));
			szFinishScanList.iSchType = atoi(sziSchType);

			if (qResult.GetFieldIsNull(2))
			{
				continue;
			}
			char sziSchScanTime[10] = { 0 };

			strcpy_s(sziSchScanTime, qResult.GetFieldValue(2));
			szFinishScanList.SchScanTime = atoi(sziSchScanTime);

			if (qResult.GetFieldIsNull(3))
			{
				continue;
			}
			char sziScanType[10] = { 0 };

			strcpy_s(sziScanType, qResult.GetFieldValue(3));
			szFinishScanList.iScanType = atoi(sziScanType);

			if (qResult.GetFieldIsNull(4))
			{
				continue;
			}

			char szSchedScanDate[15] = { 0 };
			strcpy_s(szSchedScanDate, qResult.GetFieldValue(4));

			//Extract day, month, year from string.
			int iDay = 0;
			int iMonth = 0;
			int iYear = 0;

			char seps[] = "-";
			char *token = NULL;
			char* context = NULL;

			int iIndex = 0;
			token = strtok_s(szSchedScanDate, seps, &context);
			while (token != NULL)
			{
				if (strlen(token) > 0)
				{
					int iTokenValue = atoi(token);
					switch (iIndex)
					{
					case 0:
						iYear = iTokenValue;
						break;
					case 1:
						iMonth = iTokenValue;
						break;
					case 2:
						iDay = iTokenValue;
						break;
					default:
						break;
					}
				}
				token = strtok_s(NULL, seps, &context);
				iIndex++;
			}

			szFinishScanList.SchedDate.wYear = iYear;
			szFinishScanList.SchedDate.wMonth = iMonth;
			szFinishScanList.SchedDate.wDay = iDay;

			m_vSchedScanFinish.push_back(szFinishScanList);
		}

		dbSQlite.Close();
	
		dbSQlite.Open();

		SCHEDSCANFINMAP::iterator vSchScanFinList;
		for (vSchScanFinList = m_vSchedScanFinish.begin(); vSchScanFinList != m_vSchedScanFinish.end(); vSchScanFinList++)
		{
			int lpSchFinTime = vSchScanFinList->SchScanTime;

			DWORD dwYear = vSchScanFinList->SchedDate.wYear;
			DWORD dwMonth = vSchScanFinList->SchedDate.wMonth;
			DWORD dwDay = vSchScanFinList->SchedDate.wDay;

			int intYear = (int)dwYear;
			int intMonth = (int)dwMonth;
			int intDay = (int)dwDay;

			CTime cTime = CTime::GetCurrentTime();
			CTime Time_DBEntry(intYear, intMonth, intDay, 0, 0, 0);
			CTimeSpan Time_Diff = cTime - Time_DBEntry;
			int iSpan = static_cast<int>(Time_Diff.GetDays());
			if ((iSchedtime == lpSchFinTime) && (iSpan == 0))
			{
				bIsScanFinish = true;
			}
		}
		dbSQlite.Close();
		if (bIsScanFinish)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CheckForCompletedScan", 0, 0, true, SECONDLEVEL);
		return true;
	}
	return false;
}

/***********************************************************************************************
Function Name  : IsAnyTaskRunnning
Description    : This function will check if any tasks are running
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 27 November 2017
***********************************************************************************************/
bool IsAnyTaskRunnning()
{
	bool bReturn = false;
	DWORD dwRet = SendData2UI(IS_UI_ANY_TASK_RUNNING, 0x00, 0x00, true);
	if (dwRet == YES)
	{
		bReturn = true;
	}
	else
	{
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : CheckForMissedDuplicate
Description    : This function will check for missed scans
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 10 October 2017
***********************************************************************************************/
bool CheckForMissedDuplicate(int iSchedtime)
{
	bool bIsScanMiss = false;
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBSETTINGS.DB", csWardWizModulePath);
		if (!PathFileExists(csWardWizReportsPath))
		{
			return true;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();
		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from WWIZSCHEDSCANMISS;");

		m_vSchedScanFinish.clear();

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);
			SCHEDSCANMISS szMissedScanList; 

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}
			char sziSchScanMissID[10] = { 0 };

			strcpy_s(sziSchScanMissID, qResult.GetFieldValue(1));
			szMissedScanList.iSchMissID = atoi(sziSchScanMissID);

			if (qResult.GetFieldIsNull(2))
			{
				continue;
			}
			char sziSchTypeMiss[20] = { 0 };

			strcpy_s(sziSchTypeMiss, qResult.GetFieldValue(2));
			szMissedScanList.iSchTypeMiss = atoi(sziSchTypeMiss);

			if (qResult.GetFieldIsNull(3))
			{
				continue;
			}
			char sziScanTimeMiss[10] = { 0 };

			strcpy_s(sziScanTimeMiss, qResult.GetFieldValue(3));
			szMissedScanList.SchScanTimeMiss = atoi(sziScanTimeMiss);

			if (qResult.GetFieldIsNull(4))
			{
				continue;
			}
			char sziScanTypeMiss[10] = { 0 };

			strcpy_s(sziScanTypeMiss, qResult.GetFieldValue(4));
			szMissedScanList.iScanTypeMiss = atoi(sziScanTypeMiss);

			if (qResult.GetFieldIsNull(5))
			{
				continue;
			}

			char szSchedDateMiss[15] = { 0 };
			strcpy_s(szSchedDateMiss, qResult.GetFieldValue(5));

			//Extract day, month, year from string.
			int iDay = 0;
			int iMonth = 0;
			int iYear = 0;

			char seps[] = "-";
			char *token = NULL;
			char* context = NULL;

			int iIndex = 0;
			token = strtok_s(szSchedDateMiss, seps, &context);
			while (token != NULL)
			{
				if (strlen(token) > 0)
				{
					int iTokenValue = atoi(token);
					switch (iIndex)
					{
					case 0:
						iYear = iTokenValue;
						break;
					case 1:
						iMonth = iTokenValue;
						break;
					case 2:
						iDay = iTokenValue;
						break;
					default:
						break;
					}
				}
				token = strtok_s(NULL, seps, &context);
				iIndex++;
			}

			szMissedScanList.SchedDateMiss.wYear = iYear;
			szMissedScanList.SchedDateMiss.wMonth = iMonth;
			szMissedScanList.SchedDateMiss.wDay = iDay;

			m_vSchedScanMiss.push_back(szMissedScanList);
		}
		dbSQlite.Close();

		SCHEDSCANMISSMAP::iterator vSchScanMissList;
		for (vSchScanMissList = m_vSchedScanMiss.begin(); vSchScanMissList != m_vSchedScanMiss.end(); vSchScanMissList++)
		{
			int lpSchFinTime = vSchScanMissList->SchScanTimeMiss;

			DWORD dwYear = vSchScanMissList->SchedDateMiss.wYear;
			DWORD dwMonth = vSchScanMissList->SchedDateMiss.wMonth;
			DWORD dwDay = vSchScanMissList->SchedDateMiss.wDay;

			int intYear = (int)dwYear;
			int intMonth = (int)dwMonth;
			int intDay = (int)dwDay;

			CTime cTime = CTime::GetCurrentTime();
			CTime Time_DBEntry(intYear, intMonth, intDay, 0, 0, 0);
			CTimeSpan Time_Diff = cTime - Time_DBEntry;
			int iSpan = static_cast<int>(Time_Diff.GetDays());
			if ((iSchedtime == lpSchFinTime) && (iSpan == 0))
			{
				bIsScanMiss = true;
			}
		}
		
		if (bIsScanMiss)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CheckForMissedDuplicate", 0, 0, true, SECONDLEVEL);
		return true;
	}
	return false;
}

/***********************************************************************************************
Function Name  : WatchDogThread
Description    : Thread function to watch on application that should run.
SR.NO			 :
Author Name    : Ramkrushna Shelke
Date           : 8 Jul 2016
***********************************************************************************************/
DWORD WatchDogThread(LPVOID lpParam)
{
	try
	{
		//Start this process bit slower.
		Sleep(60 * 1000);//Wait for 1 min then start the watchdog thread.

		while (true)
		{
			//If stop request come, then do not launch any application.
			if (bStopService)
			{
				break;
			}

			CEnumProcess objEnumProcess;

			CStringArray objcsaWardWizProcesses;
			objcsaWardWizProcesses.Add(L"VBTRAY.EXE");
			
			for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
			{
				//If stop request come, then do not launch any application.
				if (bStopService)
				{
					break;
				}

				CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);
				DWORD dwSessionId = WTSGetActiveConsoleSessionId();

				if (!objEnumProcess.IsProcessRunning(dwSessionId, csProcessName, false, false, false))
				{
					TCHAR szModulePath[MAX_PATH] = { 0 };
					TCHAR szFullPath[MAX_PATH] = { 0 };

					GetModuleFileName(NULL, szModulePath, MAX_PATH);
					TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
					szTemp[0] = '\0';

					_tcscpy_s(szFullPath, szModulePath);
					_tcscat_s(szFullPath, L"\\VBTRAY.EXE"); 
					CExecuteProcess			objExecprocess;
					objExecprocess.StartProcessWithTokenExplorer(szFullPath, dwSessionId, L"-NOSPSCRN", L"explorer.exe");
				}
			}
			Sleep(10 * 1000); //Wait time 10 sec's
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WatchDogThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : GetFileExtention
Description    : Function to get extension of a file
SR.NO		   :
Author Name    : Gayatri A.
Date           : 13 Jul 2016
***********************************************************************************************/
string GetFileExtention(const string& strInput) {

	size_t szLength = strInput.rfind('.', strInput.length());
	if (szLength != string::npos) {
		return(strInput.substr(szLength + 1, strInput.length() - szLength));
	}
	return("");
}

/***********************************************************************************************
Function Name  : ScanBuffer
Description    : Function to Scan the supplied buffer for an instance of FoulString.
SR.NO		   :
Author Name    : Gayatri A.
Date           : 13 Jul 2016
***********************************************************************************************/
BOOL ScanBuffer(__in_bcount(BufferSize) PUCHAR Buffer,__in ULONG BufferSize)
{
	PUCHAR p;
	ULONG searchStringLength = sizeof(FoulString) - sizeof(UCHAR);

	for (p = Buffer;p <= (Buffer + BufferSize - searchStringLength);
		p++) {

		if (RtlEqualMemory(p, FoulString, searchStringLength)) {
			return TRUE;
		}
	}

	return FALSE;
}

///***********************************************************************************************
//Function Name  : PopulateScanQueue
//Description    : Thread Function which has a pointer to the port handle we use to send/receive messages,
//				  and a completion port handle that was already associated with the comm. port by the caller
//SR.NO		   :
//Author Name    : Gayatri A.
//Date           : 13 Jul 2016
//***********************************************************************************************/
DWORD PopulateScanQueue(__in PSCANNER_THREAD_CONTEXT Context)
{
	__try
	{
		return PopulateScanQueueSEH(Context);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in PopulateScanQueue", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

///***********************************************************************************************
//Function Name  : PopulateScanQueueSEH
//Description    : Thread Function which has a pointer to the port handle we use to send/receive messages,
//				  and a completion port handle that was already associated with the comm. port by the caller
//SR.NO		   :
//Author Name    : Gayatri A.
//Date           : 13 Jul 2016
//***********************************************************************************************/
DWORD PopulateScanQueueSEH(__in PSCANNER_THREAD_CONTEXT Context)
{
	PWLSRV_NOTIFICATION pNotification;
	SCANNER_REPLY_MESSAGE pScanReplyMessage;
	PSCANNER_MESSAGE pScanMessage = NULL;
	LPOVERLAPPED pOvlp;
	BOOL bResult;
	DWORD dwOutSize;
	HRESULT hrRetVal;
	ULONG_PTR lpCompletionKey;

#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant

	while (!bStopService) {
#pragma warning(pop)

		//  Poll for messages from the filter component to scan.
		bResult = GetQueuedCompletionStatus(Context->Completion, &dwOutSize, &lpCompletionKey, &pOvlp, INFINITE);

		//  Obtain the message: note that the message we sent down via FltGetMessage() may NOT be
		//  the one dequeued off the completion queue: this is solely because there are multiple
		//  threads per single port handle. Any of the FilterGetMessage() issued messages can be
		//  completed in random order - and we will just dequeue a random one.
		pScanMessage = CONTAINING_RECORD(pOvlp, SCANNER_MESSAGE, Ovlp);

		if (!bResult) {
			//  An error occured.
			hrRetVal = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		bResult = FALSE;

		try
		{
			if (pScanMessage != NULL)
			{
				pNotification = &pScanMessage->Notification;

				//Check here if Active Scan is disable, the no need to process the call's
				if (!g_bISActiveScanDisabled)
				{
					//CString csPrint;
					//csPrint.Format(L"\n [%ld] [%ld] [%s]", pNotification->ulCallType, HandleToLong(pNotification->FObjectHash), pNotification->usAccessedFileName);
					//OutputDebugString(csPrint);

					if (pNotification->ulCallType == 0x01)
					{
						//Invoke scanning for each file that is stored in the map..
						TCHAR szVirusName[MAX_PATH] = { 0 };
						DWORD dwSpyID = 0;
						DWORD dwAction = 0;
						DWORD dwSignatureFailedToLoad = 0;
						LPCWSTR lpFileToScan = pNotification->usAccessedFileName;

						CString csFileToScan = lpFileToScan;
						CString csExtension = csFileToScan.Mid(csFileToScan.ReverseFind('.') + 1);
						csExtension.MakeLower();
						if (csExtension == L"exe")
						{
							if (!ISExcludedPathAndExt(pNotification->usAccessedFileName))
							{
								if (g_objISpyScannerBase.ScanFile((LPTSTR)lpFileToScan, szVirusName, dwSpyID, ACTIVESCAN, dwAction, dwSignatureFailedToLoad, false) == 0x01)
								{
									//If product is unregistered then it should not block execution.
									if (m_dwDaysLeft != 0x00)
									{
										AddLogEntry(L">>> Execution blocked [file] : %s", lpFileToScan, 0, true, SECONDLEVEL);
										//OutputDebugString(L"\n>>> Execution blocked for");
										OutputDebugString(lpFileToScan);
										bResult = TRUE;
									}
								}
							}
							
						}
					}
				}

				LPCWSTR lpFileToScan = pNotification->usAccessedFileName;
				if (lpFileToScan)
				{
					CString csFileToScan = lpFileToScan;
					CString csExtension = csFileToScan.Mid(csFileToScan.ReverseFind('.') + 1);
					csExtension.MakeLower();
					if (csExtension == L"exe")
					{
							DWORD dwProdID = 0;
							dwProdID = GetProductID();
							if (!(dwProdID == ESSENTIAL || dwProdID == BASIC))
							{
								if (m_dwDaysLeft != 0x00)
								{
									if (g_objEmailClient.CheckIsEXEBlockedbyPC(pNotification->usAccessedFileName))
									{
										AddIntoAppBlockQueue(pNotification->ulCallerProcessId, pNotification->usAccessedFileName);
										bResult = TRUE;
									}
								}
							}
						}
					}

				}
			}
		catch (...)
		{
			AddLogEntry(L"### Exception in PopulateScanQueueSEH", 0, 0, true, SECONDLEVEL);
		}

		pScanReplyMessage.ReplyHeader.Status = 0;
		pScanReplyMessage.ReplyHeader.MessageId = pScanMessage->MessageHeader.MessageId;

		pScanReplyMessage.Reply.SafeToOpen = !bResult;

		hrRetVal = FilterReplyMessage(Context->Port,
			(PFILTER_REPLY_HEADER)&pScanReplyMessage,
			sizeof(FILTER_REPLY_HEADER)+sizeof(SCANNER_REPLY));

		//if (SUCCEEDED(hrRetVal)) {
		//	OutputDebugString(L">>> Scanner: Replied message");
		//}
		//else {
		//	//OutputDebugString(L">>> Scanner: Error replying message.");
		//}

		//Check here if Active Scan is disable, the no need to process the items
		if (!g_bISActiveScanDisabled)
		{
			CExecuteProcess objExecuteProcess;
			CString csUserName = objExecuteProcess.GetUserNamefromProcessID(pNotification->ulCallerProcessId);

			//check as we have fixed size to store user name.
			if (csUserName.GetLength() < 100)
			{
				AddIntoProcessingQueue(pNotification->usAccessedFileName, csUserName.GetBuffer());
		}
		}

		memset(&pScanMessage->Ovlp, 0, sizeof(OVERLAPPED));

		hrRetVal = FilterGetMessage(Context->Port,
			&pScanMessage->MessageHeader,
			FIELD_OFFSET(SCANNER_MESSAGE, Ovlp),
			&pScanMessage->Ovlp);

		if (hrRetVal != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {

			break;
		}
	}

	if (!SUCCEEDED(hrRetVal)) {
		if (hrRetVal == HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)) {
			// Scanner port disconncted.
			AddLogEntry(L"Scanner: Port is disconnected, probably due to scanner filter unloading.", 0, 0, true, ZEROLEVEL);
		}
		else {
			AddLogEntry(L"Scanner: Unknown error occured. Error = 0x%X", 0, 0, true, SECONDLEVEL);
		}
	}

	if (pScanMessage != NULL)
	{
		free(pScanMessage);
	}
	return hrRetVal;
}

///***********************************************************************************************
//Function Name  : ProcessScanQueueThread
//Description    : Thread function to Invoke scanning for all the files stored in queue.
//SR.NO		   :
//Author Name    : Gayatri A.
//Date           : 13 Jul 2016
//***********************************************************************************************/
void ProcessScanQueueThread(CConCurrentQueue<ACTIVESCANFLAGS>& pObjeOnAccess)
{
	__try
	{
		ProcessScanQueueThreadSEH(pObjeOnAccess);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ProcessScanQueueThread", 0, 0, true, SECONDLEVEL);
	}
}

///***********************************************************************************************
//Function Name  : ProcessAppBlockThread
//Description    : Thread function to Invoke Application block the files stored in queue.
//SR.NO			 :
//Author Name    : Ramkrushna Shelke
//Date           : 17 August 2018
//***********************************************************************************************/
void ProcessAppBlockThread(CConCurrentQueue<BLOCKAPPFLAGS>& pObjeAppBlock)
{
	__try
	{
		ProcessAppBlockThreadSEH(pObjeAppBlock);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ProcessAppBlockThread", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertSQLiteData
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 InsertSQLiteData(const char* szQuery)
{
	try
	{
		CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(100);

		INT64 iLastRowId = objSqlDb.GetLastRowId();
		objSqlDb.Close();

		return iLastRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizComSrv- InsertSQLiteData. Query is : ", (TCHAR*)szQuery, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***********************************************************************************************
Function Name  : InsertSQLiteDataForStng
Description    : To insert data into SQLite table
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 04 October 2017
***********************************************************************************************/
INT64 InsertSQLiteDataForStng(const char* szQuery)
{
	try
	{
		char* g_strDatabasePath = ".\\VBSETTINGS.DB";
		
		CWardWizSQLiteDatabase objSqlDb(g_strDatabasePath);
		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(100);

		INT64 iLastRowId = objSqlDb.GetLastRowId();
		objSqlDb.Close();

		return iLastRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizComSrv- InsertSQLiteDataForStng. Query is : ", (TCHAR*)szQuery, 0, true, SECONDLEVEL);
		return 0;
	}
}

///***********************************************************************************************
//Function Name  : ProcessAppBlockThreadSEH
//Description    : Thread function to Invoke application block path stored in queue.
//SR.NO			 :
//Author Name    : Ram Shelke
//Date           : 17 August 2018
//***********************************************************************************************/
void ProcessAppBlockThreadSEH(CConCurrentQueue<BLOCKAPPFLAGS>& pObjAppBLock)
{
	try
	{
		while (true)
		{
			auto itemFromQueue = pObjAppBLock.pop();


			//Invoke scanning for each file that is stored in the map..
			TCHAR szVirusName[MAX_PATH] = { 0 };
			DWORD dwSpyID = 0;
			DWORD dwAction = 0;

			DWORD dwSignatureFailedToLoad = 0;
			LPCWSTR lpFileToBlock = itemFromQueue.wcProcessPath;
			if (lpFileToBlock == NULL)
			{
				continue;
			}

			if (_tcscmp(itemFromQueue.wcProcessPath, L"WRDWIZEXIT") == 0)
			{
				break;
			}

			//OutputDebugString(lpFileToBlock);

			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = SHOW_APP_BLOCK_POPUP;
			_tcscpy_s(szPipeData.szFirstParam, (LPTSTR)lpFileToBlock);

			CExecuteProcess objExecuteProcess;
			CString csUserName = objExecuteProcess.GetUserNamefromProcessID(itemFromQueue.ulCallerProcessId);

			if (csUserName.IsEmpty())
			{
				continue;
			}

			_tcscpy_s(szPipeData.szThirdParam, csUserName);
			szPipeData.dwValue = dwAction;
			
			CISpyCommunicator objCom(TRAY_SERVER + csUserName.MakeUpper(), true, 0x02, 0x12C);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csMessage;
				csMessage.Format(L"%d", szPipeData.iMessageInfo);
				AddLogEntry(L"### SendData failed in WardwizComSrv::ProcessAppBlockThreadSEH, MessageID:%s", csMessage, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ProcessAppBlockThreadSEH", 0, 0, true, SECONDLEVEL);
	}
}

///***********************************************************************************************
//Function Name  : ProcessScanQueueThreadSEH
//Description    : Thread function to Invoke scanning for all the files stored in queue.
//SR.NO		   :
//Author Name    : Gayatri A.
//Date           : 13 Jul 2016
//***********************************************************************************************/
void ProcessScanQueueThreadSEH(CConCurrentQueue<ACTIVESCANFLAGS>& pObjeOnAccess)
{
	try
	{
		std::wstring strQueElement = L"";
		CString csFilePath = GetWardWizPathFromRegistry();
		csFilePath += L"VBACTIVESCAN.DB";
		DWORD dwProdID = 0;
		dwProdID = GetProductID();

		while (true)
		{
			auto itemFromQueue = pObjeOnAccess.pop();

			LPCWSTR lpFileToScan = itemFromQueue.wcProcessPath;
			if (lpFileToScan == NULL)
			{
				continue;
			}

			if (_tcscmp(itemFromQueue.wcProcessPath, L"WRDWIZEXIT") == 0)
			{
				break;
			}

			//Invoke scanning for each file that is stored in the map..
			TCHAR szVirusName[MAX_PATH] = { 0 };
			DWORD dwSpyID = 0;
			DWORD dwAction = 0;

			DWORD dwSignatureFailedToLoad = 0;

			CString csFileToScan = lpFileToScan;
			CString csExtension = csFileToScan.Mid(csFileToScan.ReverseFind('.') + 1);
			csExtension.MakeLower();
			//Allowed all the files to get scanned
			//if (csExtension == L"exe")
			{
				//Check here a valid path
				if (!PathFileExists(lpFileToScan))
				{
					continue;
				}

				//avoid network path
				if (PathIsNetworkPath(lpFileToScan))
				{
					continue;
				}

				//Check here if Active Scan is disable, the no need to process the call's add into DB.
				if (bStopService || g_bISActiveScanDisabled)
				{
					AddEntryIntoFile((LPTSTR)lpFileToScan, csFilePath.GetBuffer());
					continue;
				}

				if (ISExcludedPathAndExt((LPTSTR)lpFileToScan))
				{
					AddLogEntry(L">>> Excluded Path [%s] ", lpFileToScan, 0, true, ZEROLEVEL);
					continue;
				}

				CString csScanFilePath(lpFileToScan);
				//check here some extension excluded from database
				CString csFileExt = csScanFilePath.Right((csScanFilePath.GetLength() - csScanFilePath.ReverseFind('.') - 1));
				csFileExt.Trim();
				if (g_dwPerfLevel == 0x02 && !ISIncludedFileExt(csFileExt.GetBuffer()))
				{
					continue;
				}

				/*disabled this as this design wont work.
				if (!CheckScanPermission(csScanFilePath.GetBuffer(), HandleToLong(itemFromQueue.hFileObjectHash)) && itemFromQueue.dwScanTryCount < 10)
				{
					OutputDebugStringA("NOPERMISSION");
					itemFromQueue.dwScanTryCount++;
					g_onAccessQueue.push(itemFromQueue);
				}
				*/

				if (g_objISpyScannerBase.ScanFile((LPTSTR)lpFileToScan, szVirusName, dwSpyID, ACTIVESCAN, dwAction, dwSignatureFailedToLoad, false) == TRUE)
				{
					AddLogEntry(L"!! ScanFile returned true for file %s ....", lpFileToScan, 0, true, ZEROLEVEL);
					// Add entries into Database
					CString csStatus = L"";
					switch (dwAction)
					{
					case FILESKIPPED:
						csStatus = "IDS_FILE_SKIPPED";
						m_TotalThreatsCleaned = 0;
						break;
					case FILEQURENTINED:
						csStatus = "IDS_CONSTANT_THREAT_QUARANTINED";
						m_TotalThreatsCleaned = 1;
						break;
					case FILEREPAIRED:
						csStatus = "IDS_CONSTANT_THREAT_REPAIRED";
						m_TotalThreatsCleaned = 1;
						break;
					case LOWDISKSPACE:
						csStatus = "IDS_CONSTANT_THREAT_LOWDISC_SPACE";
						m_TotalThreatsCleaned = 1;
						break;
					}

					if (dwProdID == ELITE)
					{
						//Send here to EPS client
						SendData2EPSClient((LPTSTR)lpFileToScan, szVirusName, dwAction);
					}

					// Get entries from registry so that, those can be included in query..
					DWORD dwQuarantineOpt = 0x00;
					DWORD dwHeuristicOpt = 0x00;
					bool  bHeuristicOpt = false;

					GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);

					if (dwHeuristicOpt == 1)
						bHeuristicOpt = true;


					CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");

					csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),1,1,%d,%d,%d);"), ACTIVESCAN, dwQuarantineOpt, bHeuristicOpt, m_TotalThreatsCleaned);

					CT2A asciiSession(csInsertQuery, CP_UTF8);
					INT64 iScanSessionId = InsertSQLiteData(asciiSession.m_psz);

					CString csVirusName(szVirusName);
					csVirusName.Replace(L"'", L"''");

					CString csFileToScan(lpFileToScan);
					csFileToScan.Replace(L"'", L"''");

					csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%I64d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),'%s','%s','%s',NULL );"), iScanSessionId, csVirusName, csFileToScan, csStatus);

					CT2A ascii(csInsertQuery, CP_UTF8);

					InsertSQLiteData(ascii.m_psz);

					ISPY_PIPE_DATA szPipeData = { 0 };
					szPipeData.iMessageInfo = SHOWONACCESSPOPUP;
					_tcscpy_s(szPipeData.szFirstParam, szVirusName);
					_tcscpy_s(szPipeData.szSecondParam, lpFileToScan);
					_tcscpy_s(szPipeData.szThirdParam, itemFromQueue.szUserName);
					szPipeData.dwValue = dwAction;

					CISpyCommunicator objCom(TRAY_SERVER + CString(itemFromQueue.szUserName), true, 0x03, 0x12C);
					if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
					{
						CString csMessage;
						csMessage.Format(L"%d", szPipeData.iMessageInfo);
						AddLogEntry(L"### SendData failed in ProcessScanQueueThreadSEH, MessageID:%s", csMessage, 0, true, SECONDLEVEL);
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ProcessScanQueueThreadSEH", 0, 0, true, SECONDLEVEL);
	}
}

///***********************************************************************************************
//Function Name  : ProcessScanQueueThread
//Description    : Thread function to Invoke scanning for all the files stored in queue.
//SR.NO		   :
//Author Name    : Ramkrushna Shelke
//Date           : 13 Jul 2016
//***********************************************************************************************/
bool HandleActiveScanSettings(LPISPY_PIPE_DATA lpSpyData)
{
	__try
	{
		if (!lpSpyData)
			return false;

		if (lpSpyData->dwValue == DISABLEACTSCAN)
		{
			if (hActiveScanEvent != NULL)
			{
				SetEvent(hActiveScanEvent);
				Sleep(1000);
			}

			if (hActScanSetThread != NULL)
			{
				::SuspendThread(hActScanSetThread);
				::TerminateThread(hActScanSetThread, 0);
				hActScanSetThread = NULL;
			}

			if (hActScanSetThread == NULL)
			{
				memset(&g_szSpyData, 0, sizeof(g_szSpyData));
				memcpy(&g_szSpyData, lpSpyData, sizeof(g_szSpyData));

				hActScanSetThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleActiveScanSettingsThread,
					(LPVOID)&g_szSpyData, 0, 0);

				if (hActScanSetThread == NULL)
				{
					AddLogEntry(L"### HandleActiveScanSettingsThread Thread Creation failed in HandleActiveScanSettings ", 0, 0, true, SECONDLEVEL);
				}
			}
		}
		else if (lpSpyData->dwValue == ENABLEACTSCAN)
		{
			g_bISActiveScanDisabled = false;

			if (hActiveScanEvent != NULL)
			{
				SetEvent(hActiveScanEvent);
				Sleep(1000);
			}
		}
	}
		__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in HandleActiveScanSettings", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

///***********************************************************************************************
//Function Name  : HandleActiveScanSettingsThread
//Description    : Thread function to Enable Disable Active Scanning
//SR.NO		   :
//Author Name    : Ramkrushna Shelke
//Date           : 13 Jul 2016
//***********************************************************************************************/
DWORD HandleActiveScanSettingsThread(LPVOID lpParam)
{
	__try
	{
		LPISPY_PIPE_DATA pPipeData = (LPISPY_PIPE_DATA)lpParam;
		if (!pPipeData)
			return 0;


		//Disable Active Scan here
		if (pPipeData->dwValue == DISABLEACTSCAN)
		{
			g_bISActiveScanDisabled = true;
		}

		if (hActiveScanEvent != NULL)
		{
			ResetEvent(hActiveScanEvent);
		}

		//Get here the wait time,
		DWORD dwActiveScanWaitTime = 0x00;
		TCHAR szCaseID[MAX_PATH] = { 0 };
		switch (pPipeData->dwSecondValue)
		{
			case DISABLE_FOR_15_MINS:
				dwActiveScanWaitTime = 15 * 60 * 1000;// 15 Mins = 15*60*1000 = 900000 Miliseconds
				//OutputDebugString(L">>> Active Scan Disabled for 15 Mins");
				AddLogEntry(L">>> Active Scan Disabled for 15 Mins", 0, 0, true, SECONDLEVEL);
				break;
			case DISABLE_FOR_1_HOUR:
				dwActiveScanWaitTime = 60 * 60 * 1000;// 1 hour = 60*60*1000 = 3600000 Miliseconds
				AddLogEntry(L">>> Active Scan Disabled for 1 Hour", 0, 0, true, SECONDLEVEL);
				//OutputDebugString(L">>> Active Scan Disabled for 1 Hour");
				break;
			case DISABLE_UNTIL_RESTART:
				dwActiveScanWaitTime = INFINITE;
				AddLogEntry(L">>> Active Scan Disabled until system restart", 0, 0, true, SECONDLEVEL);
				//OutputDebugString(L">>> Active Scan Disabled until system restart");
				break;
			case DISABLE_PERMANANT:
				dwActiveScanWaitTime = INFINITE;
				AddLogEntry(L">>> Active Scan Disabled Permanently", 0, 0, true, SECONDLEVEL);
				//OutputDebugString(L">>> Active Scan Disabled Permanently");
				break;
			default:
				swprintf_s(szCaseID, L"%d", pPipeData->dwSecondValue);
				AddLogEntry(L"### Invalid case in HandleActiveScanSettingsThread, case : %s", szCaseID, 0, true, SECONDLEVEL);
				break;
		}


		//Wait here as per settings
		DWORD dwRet = WaitForSingleObject(hActiveScanEvent, dwActiveScanWaitTime);
		switch (dwRet)
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			break;
			// The thread got ownership of an abandoned mutex
			// The database is in an indeterminate state
		case WAIT_ABANDONED:
			break;
		case WAIT_TIMEOUT:
			//Enable here active protection
			if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwActiveScanOption", 0x01) != 0x00)
			{
				AddLogEntry(L"### SetRegistryValueData failed for dwActiveScanOption", 0, 0, true, SECONDLEVEL);
			}
			g_bISActiveScanDisabled = false;
			SendMessage2UI(SEND_ACTIVE_PROTECTION_STATUS);
			break;
		}

		AddLogEntry(L">>> Active Scan Enabled", 0, 0, true, SECONDLEVEL);

		if (!SendData2Tray(HANDLEACTICESCANSETTINGS, ENABLEACTSCAN, 0, 0, 0, false))
		{
			AddLogEntry(L"### Failed SendData2Tray in HandleActiveScanSettingsThread", 0, 0, true, SECONDLEVEL);
		}

		//Release Event Handle here
		if (hActiveScanEvent != NULL)
		{
			ResetEvent(hActiveScanEvent);
		}

		//ReIntialize ActiveScan Thread handle.
		if (hActScanSetThread != NULL)
		{
			hActScanSetThread = NULL;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in HandleActiveScanSettingsThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


///***********************************************************************************************
//Function Name  : HandleProductSettings
//Description    : Function to handle product settings to service
//SR.NO		     : 
//Author Name    : Ramkrushna Shelke
//Date           : 05 Aug 2016
//***********************************************************************************************/
bool HandleProductSettings(LPISPY_PIPE_DATA lpSpyData)
{
	__try
	{
		if (!lpSpyData)
			return false;

		switch (lpSpyData->dwValue)
		{
		case SHOWTRAYNOTIFICATION:
			break;
		case AUTOQUARENTINEOPTION:
			//Set here autoQuarentine option
			g_objISpyScannerBase.m_objWardWizScanner.SetAutoQuarentineOption(lpSpyData->dwSecondValue);
			break;
		case ENABLEDISABLESOUND:
			break;
		case EXCLUDEFILESSCANUPDATED:
			g_objISpyScannerBase.m_objWardWizScanner.ReloadExcludeDB();
			break;
		case RELOADSETTINGHEUSCAN:
			g_objISpyScannerBase.m_objWardWizScanner.m_objISpyScanner.ReloadSettingsForScanDLL();
			break;
		case PREPARECACHEINBACKGROUND:
			break;
		case SCANBYNAMEFILE:
			g_objISpyScannerBase.m_objWardWizScanner.ReloadScanByName();
			break;
		case RELOAD_EXCLUDE_DB:
			ReloadExcludeDB();
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in HandleProductSettings", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

///***********************************************************************************************
//Function Name  : ReLoadSignatures
//Description    : Function to reload signature database.
//SR.NO		     : 
//Author Name    : Ramkrushna Shelke
//Date           : 05 Aug 2016
//***********************************************************************************************/
bool ReLoadSignatures()
{
	bool bReturn = false;
	try
	{
		DWORD dwSigCount = 0x00;
		if (g_objISpyScannerBase.ReLoadSignatureDatabase(dwSigCount) != 0x00)
		{
			AddLogEntry(L"### Failed in ReLoadSignatures", 0, 0, true, SECONDLEVEL);
		}

		DWORD dwCachingMethod = 0x01;
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwCachingMethod", dwCachingMethod) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwCachingMethod in ReLoadSignatures KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		if (dwCachingMethod == 0x00)
		{
			if (!ClearIndexing())
			{
				AddLogEntry(L"### Indexing clear failed in ReLoadSignatures", 0, 0, true, SECONDLEVEL);;
			}
		}

		//Write here Count in ini file.
		TCHAR szSigCount[20] = { 0 };
		swprintf_s(szSigCount, L"%d", dwSigCount);

		TCHAR szIniFilePath[MAX_PATH] = { 0 };
		if (GetModulePath(szIniFilePath, sizeof(szIniFilePath)))
		{
			_tcscat_s(szIniFilePath, L"\\VBSETTINGS\\PRODUCTSETTINGS.INI");
			WritePrivateProfileString(L"VBSETTINGS", L"ThreatDefCount", szSigCount, szIniFilePath);
		}
		
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReLoadSignatures", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadRegistrationDays
Description    : Function to reload Registration days.
Author Name    : Ramkrushna Shelke
Date           : 19 Aug 2016
***********************************************************************************************/
bool ReloadRegistrationDays()
{
	bool bReturn = false;
	__try
	{
		m_dwDaysLeft = g_objISpyScannerBase.m_objWardWizScanner.GetDaysLeft();

		if (!g_bISFwPCEmailInitialized && m_dwDaysLeft != 0x00)
		{
			StartFirewallThread();
			StartEmailScanThread();
			g_bISFwPCEmailInitialized = true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ReloadRegistrationDays", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : AddEntryIntoFile
Description    : Function to add entry in to file
Author Name    : Ramkrushna Shelke
Date           : 19 Aug 2016
***********************************************************************************************/
bool AddEntryIntoFile(LPTSTR lpszEntry, LPTSTR lpszFilePath)
{
	bool bReturn = false;
	__try
	{
		if (!lpszEntry)
			return bReturn;

		if (!lpszFilePath)
			return bReturn;

		if (_tcslen(lpszEntry) == 0)
			return bReturn;

		if (!PathFileExists(lpszEntry))
		{
			return bReturn;
		}

		//Lock resource here
		g_objOnAccessFileAdd.Lock();

		FILE *fpActiveScanData = NULL;
		fpActiveScanData = _wfsopen(lpszFilePath, _T("a"), 0x40);

		if (fpActiveScanData == NULL)
		{
			//Unlock resource here
			g_objOnAccessFileAdd.Unlock();
			return bReturn;
		}

		if (fpActiveScanData != NULL)
		{
			fputws(lpszEntry, fpActiveScanData);
			fputws(L"\n", fpActiveScanData);
		}
		
		fflush(fpActiveScanData);
		fclose(fpActiveScanData);

		//Unlock resource here
		g_objOnAccessFileAdd.Unlock();

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in AddEntryIntoFile, FilePath: %s", lpszFilePath, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : AddEntryIntoFile
Description    : Function to add entry in to file
Author Name    : Ramkrushna Shelke
Date           : 19 Aug 2016
***********************************************************************************************/
bool LoadActiveScanEntryFromFile(CConCurrentQueue<ACTIVESCANFLAGS> &onAccessQueue, LPTSTR lpszFilePath)
{
	bool bReturn = false;
	__try
	{
		bReturn = LoadActiveScanEntryFromFileSEH(g_onAccessQueue, lpszFilePath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in LoadActiveScanEntryFromFile, FilePath: [%s]", lpszFilePath, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : AddEntryIntoFile
Description    : Function to add entry in to file
Author Name    : Ramkrushna Shelke
Date           : 19 Aug 2016
***********************************************************************************************/
bool LoadActiveScanEntryFromFileSEH(CConCurrentQueue<ACTIVESCANFLAGS> &onAccessQueue, LPTSTR lpszFilePath)
{
	bool bReturn = false;
	try
	{
		if (!lpszFilePath)
			return bReturn;

		if (!PathFileExists(lpszFilePath))
			return bReturn;

		std::ifstream in(lpszFilePath);

		if (!in) {
			return bReturn;
		}

		CString csEntry;
		std::string str;
		while (std::getline(in, str)) {

			if (str.length() == 0)
				continue;

			csEntry = str.c_str();
			csEntry = csEntry.Left(csEntry.GetLength() - 1);
			if (csEntry.GetLength() != 0 && PathFileExists(csEntry))
			{
				ACTIVESCANFLAGS	szActiveScanopt;
				memset(&szActiveScanopt, 0x00, sizeof(ACTIVESCANFLAGS));

				_tcscpy(szActiveScanopt.wcProcessPath, csEntry);
				g_onAccessQueue.push(szActiveScanopt);
			}
		}

		in.close();
		
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LoadActiveScanEntryFromFileSEH, FilePath: %s", lpszFilePath, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ISExcludedPath
Description    : Function to Check is the path is excluded.
Author Name    : Ramkrushna Shelke
Date           : 17 Sep 2016
***********************************************************************************************/
bool ISExcludedPathAndExt(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	try
	{
		//Check here is file is excluded?
		bool bIsSubFolderExcluded = false;
		CString csFilePath = lpszFilePath;
		CString csFileExt = csFilePath.Right((csFilePath.GetLength() - (csFilePath.ReverseFind(L'.'))));
		csFileExt.Trim('.');

		if (g_objISpyScannerBase.m_objWardWizScanner.ISExcludedFileExt((LPTSTR)csFileExt.GetString()))
		{
			AddLogEntry(L">>> Excluded File Extension [%s] ", csFileExt, 0, true, ZEROLEVEL);
			bReturn = true;
		}
		else if (g_objISpyScannerBase.m_objWardWizScanner.ISExcludedPath(lpszFilePath, bIsSubFolderExcluded))
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ISExcludedPath, FilePath: %s", lpszFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ClearIndexing
Description    : Function to Check is the path is excluded.
Author Name    : Ramkrushna Shelke
Date           : 17 Sep 2016
***********************************************************************************************/
bool ClearIndexing()
{
	bool bReturn = false;
	try
	{
		DWORD dwCachingMethod = 0x01;
		CITinRegWrapper				g_objReg;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwCachingMethod", dwCachingMethod) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwCachingMethod in ClearIndexing KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		if (dwCachingMethod != 0x00)//if its momentary then only allow to clean cache
		{
			return false;
		}

		CWWizIndexer objIndexer;
		if (objIndexer.ClearIndedexes())
		{
			bReturn = true;
		}

		//clear here web filter cache
		if (g_objWebProtClient.ClearURLCache())
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ClearIndexing", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}


/***********************************************************************************************
Function Name  : StopIndexing
Description    : Function to stop indexing
Author Name    : Ramkrushna Shelke
Date           : 24 Aug 2016
***********************************************************************************************/
bool StartStopIndexing(DWORD dwOption)
{
	bool bReturn = false;
	__try
	{
		if (dwOption == 0x00)
		{
			if (hIndexingThread != NULL)
			{
				SuspendThread(hIndexingThread);
				TerminateThread(hIndexingThread, 0x00);
				hIndexingThread = NULL;
			}
		}
		else
		{
			StartIndexing();
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartStopIndexing", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ApplyAutorunScanSettings
Description    : Function to read registry settings and send settings to scanner driver.
Author Name    : Ramkrushna Shelke
Date           : 09 Jan 2017
***********************************************************************************************/
bool ApplyAutorunScanSettings()
{
	bool bReturn = false;
	try
	{
		bool bAutorunBlocked = true;
		bool bUSBExecBlocked = false;
		bool bUSBWriteBlocked = false;

		CScannerLoad objScanDrv;

		//Get here registry setting for autorun block
		DWORD dwBlockAutorun = 0x01;
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwBlockAutorun", dwBlockAutorun) != 0x00)
		{
			dwBlockAutorun = 0x01;
			AddLogEntry(L"### Failed to get Registry Entry for dwBlockAutorun in ApplyAutorunScanSettings KeyPath: %s", g_csRegKeyPath, 0, true, FIRSTLEVEL);;
		}

		if (dwBlockAutorun == 0x00)
		{
			bAutorunBlocked = false;
		}

		//Get here registry setting for USB Exec Block
		DWORD dwUSBExecBlockAutorun = 0x00;
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwExecuteBlock", dwUSBExecBlockAutorun) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwExecuteBlock in ApplyAutorunScanSettings KeyPath: %s", g_csRegKeyPath, 0, true, FIRSTLEVEL);;
		}

		//check with USB Exec Block setting
		if (dwUSBExecBlockAutorun == 0x01)
		{
			bUSBExecBlocked = true;
		}

		//Get here registry setting for USB write block
		DWORD dwWriteBlock = 0x00;
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwWriteBlock", dwWriteBlock) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwWriteBlock in ApplyAutorunScanSettings KeyPath: %s", g_csRegKeyPath, 0, true, FIRSTLEVEL);;
		}

		//check with USB Exec Block setting
		if (dwWriteBlock == 0x01)
		{
			bUSBWriteBlocked = true;
		}
		 
		if (objScanDrv.SetAutorunSettings(bAutorunBlocked, bUSBExecBlocked, bUSBWriteBlocked))
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ApplyAutorunScanSettings", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : AddIntoProcessingQueue
Description    : Function to add into processing queue.
Author Name    : Ramkrushna Shelke
Date           : 17 Jan 2017
***********************************************************************************************/
bool AddIntoProcessingQueue(PWCHAR usAccessedFileName, LPTSTR lpszUserName)
{
	try
	{
		if (!usAccessedFileName || !lpszUserName)
			return false;

		if (PathFileExists(usAccessedFileName))
		{
			ACTIVESCANFLAGS	szActiveScanopt;
			memset(&szActiveScanopt, 0x00, sizeof(ACTIVESCANFLAGS));

			_tcscpy(szActiveScanopt.szUserName, lpszUserName);
			_tcscpy(szActiveScanopt.wcProcessPath, usAccessedFileName);
			g_onAccessQueue.push(szActiveScanopt);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in AddIntoProcessingQueue", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name	: GenerateMachineID
Description		: Function which Generate machine ID
SR.NO			: 
Author Name		: Ramkrushna Shelke
Date			: 30 Mar 2017
***********************************************************************************************/
bool GenerateMachineID(LPTSTR szClientID, DWORD dwSize, bool bUsingHDDSerial, LPTSTR lpszHDDSerial, LPTSTR lpszOldMacID)
{
	__try
	{
		TCHAR	szBIOSSerialNumber[0x38] = { 0 };
		TCHAR	szCPUID[0x40] = { 0 };
		TCHAR	szMACAddress[0x40] = { 0 };

		GetCPUID(szCPUID);
		if (!szCPUID[0])
		{
			AddLogEntry(L"### Failed to GenerateMachineID::CPUID", 0, 0, true, SECONDLEVEL);
			return false;
		}

		GetBIOSSerialNumberSEH(szBIOSSerialNumber);
		if (!szBIOSSerialNumber[0])
		{
			//	ISSUE No : 163
			//	Some customers were not getting BIOS Serial Number, So we added MotherBoard Serial Number
			//	to make Unique Machine ID
			GetMotherBoardSerialNumberSEH(szBIOSSerialNumber);
			if (!szBIOSSerialNumber[0])
			{
				if (bUsingHDDSerial)
				{
					if (lpszHDDSerial)
					_tcscpy_s(szMACAddress, lpszHDDSerial);
				}
				else
				{
					AddLogEntry(L"### Before MAC", 0, 0, true, ZEROLEVEL);
					GetMACAddress(szMACAddress, lpszOldMacID);
					AddLogEntry(L"### After MAC", 0, 0, true, ZEROLEVEL);

					if (!szMACAddress[0])
					{
						AddLogEntry(L"### Before VGA", 0, 0, true, ZEROLEVEL);
						GetVGAAdapterID(szBIOSSerialNumber);
						if (!szBIOSSerialNumber[0])
						{
							AddLogEntry(L"### Failed to retrieve VGA ID", 0, 0, true, SECONDLEVEL);
							return false;
						}
						AddLogEntry(L"### After VGA", 0, 0, true, ZEROLEVEL);
					}
				}
			}
		}

		if (szBIOSSerialNumber[0])
		{
			RemoveCharsIfExists(szBIOSSerialNumber, static_cast<int>(wcslen(szBIOSSerialNumber)), static_cast<int>(sizeof(szBIOSSerialNumber)), 0x20);
		}

		if (!szMACAddress[0])
		{
			if (bUsingHDDSerial)
			{
				if (lpszHDDSerial)
				_tcscpy_s(szMACAddress, lpszHDDSerial);
			}
			else
			{
				GetMACAddress(szMACAddress, lpszOldMacID);
			}
		}

		int	i = static_cast<int>(wcslen(szCPUID));

		szClientID[0] = (TCHAR)i;
		swprintf(&szClientID[1], 0x7E, L"%s%s%s", szCPUID, szBIOSSerialNumber, szMACAddress);
		
		//Added to terminate MID 
		i = static_cast<int>(wcslen(szClientID));
		if (i > 0x7F)
			i = 0x7F;

		szClientID[i] = '\0';

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GenerateMachineID", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : GetRegisteredUserInfo
*  Description    : Get registered User info from registry or from file.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
bool GetRegisteredUserInformation()
{
	DWORD	dwRet = 0x00;
	DWORD	dwSize = 0x00;
	DWORD	dwRegUserSize = 0x00;
	//Get Registration data from Files which are present on hard disk.
	dwRet = GetRegistrationDatafromFile();

	if (!dwRet)
	{
		return true;
	}

	dwRet = GetRegistrationDataFromRegistry();

	if (!dwRet)
	{
		return true;
	}

	return false;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDataFromRegistry
*  Description    : Get Registration data of user from registry.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD GetRegistrationDataFromRegistry()
{
	DWORD	dwRet = 0x00;
	try
	{
		DWORD	dwRegType = 0x00, dwRetSize = 0x00;

		HKEY	h_iSpyAV = NULL;
		HKEY	h_iSpyAV_User = NULL;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows"),
			0, KEY_READ, &h_iSpyAV) != ERROR_SUCCESS)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		dwRetSize = sizeof(m_ActInfo);
		if (RegQueryValueEx(h_iSpyAV, TEXT("VibraniumUserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo,
			&dwRetSize) != ERROR_SUCCESS)
		{
			dwRet = GetLastError();
			dwRet = 0x03;
			goto Cleanup;
		}

		if (DecryptRegistryData((LPBYTE)&m_ActInfo, sizeof(m_ActInfo)))
		{
			dwRet = 0x04;
			goto Cleanup;
		}

	Cleanup:

		if (h_iSpyAV_User)
			RegCloseKey(h_iSpyAV_User);
		if (h_iSpyAV)
			RegCloseKey(h_iSpyAV);

		h_iSpyAV_User = h_iSpyAV = NULL;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::GetRegistrationDataFromRegistry", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : DecryptRegistryData
*  Description    : Decrypt data of buffer for registration data.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD DecryptRegistryData(LPBYTE lpBuffer, DWORD dwSize)
{
	try
	{
		if (IsBadWritePtr(lpBuffer, dwSize))
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			if (lpBuffer[iIndex] != 0)
			{
				if ((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
				{
					lpBuffer[iIndex] = lpBuffer[iIndex];
				}
				else
				{
					lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
					jIndex++;
				}
				if (jIndex == WRDWIZ_KEYSIZE)
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
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::DecryptRegistryData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDatafromFile
*  Description    : Get user registration data from file.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD GetRegistrationDatafromFile()
{
	try
	{
		DWORD	dwRet = 0x01;
		CString	strUserRegFile = GetModuleFileStringPath();
		strUserRegFile = strUserRegFile + L"\\VBUSERREG.DB";
		dwRet = GetRegistrationDatafromFile(strUserRegFile);
		if (!dwRet)
			return dwRet;

		TCHAR	szAllUserPath[512] = { 0 };
		TCHAR	szSource[512] = { 0 };
		TCHAR	szSource1[512] = { 0 };
		TCHAR	szDestin[512] = { 0 };
		TCHAR	szDestin1[512] = { 0 };

		OSVERSIONINFO 	OSVer = { 0 };
		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 511);
		OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&OSVer);
		if (OSVer.dwMajorVersion > 5)
			wsprintf(szDestin, L"%s\\Wardwiz Antivirus", szAllUserPath);
		else
			wsprintf(szDestin, L"%s\\Application Data\\Wardwiz Antivirus", szAllUserPath);

		wcscpy(szDestin1, szDestin);
		wcscat(szDestin1, L"\\VBUSERREG.DB");
		dwRet = 0x01;
		dwRet = GetRegistrationDatafromFile(szDestin1);
		if (!dwRet)
			return dwRet;

		TCHAR	szDrives[256] = { 0 };
		GetLogicalDriveStrings(255, szDrives);

		TCHAR	*pDrive = szDrives;

		while (wcslen(pDrive) > 2)
		{
			dwRet = 0x01;
			memset(szDestin1, 0x00, 512 * sizeof(TCHAR));
			wsprintf(szDestin1, L"%sVBUSERREG.DB", pDrive);
			if ((GetDriveType(pDrive) & DRIVE_FIXED) == DRIVE_FIXED)
			{
				dwRet = GetRegistrationDatafromFile(szDestin1);
				if (!dwRet)
					return dwRet;
			}
			pDrive += 4;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::GetRegistrationDatafromFile", 0, 0, true, SECONDLEVEL);
	}
	return 0x01;
}

/***************************************************************************************************
*  Function Name  : GetModuleFileStringPath
*  Description    : Get the path where module is exist
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
CString GetModuleFileStringPath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDatafromFile
*  Description    : Get User registration data from file.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD GetRegistrationDatafromFile(CString strUserRegFile)
{
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	DWORD	dwRet = 0x00, dwBytesRead = 0x00;
	DWORD	dwSize = sizeof(m_ActInfo);
	hFile = CreateFile(strUserRegFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwRet = 0x01;
		goto Cleanup;
	}

	ZeroMemory(&m_ActInfo, sizeof(m_ActInfo));
	ReadFile(hFile, &m_ActInfo, dwSize, &dwBytesRead, NULL);
	if (dwSize != dwBytesRead)
	{
		dwRet = 0x02;
		goto Cleanup;
	}

	if (DecryptRegistryData((LPBYTE)&m_ActInfo, dwSize))
	{
		dwRet = 0x04;
		goto Cleanup;
	}
Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return dwRet;
}

bool CheckForHardDiskSerialNumber(LPTSTR szClientID, LPTSTR szHDDSerial)
{
	bool bReturn = false;
	__try
	{
		if (!szClientID)
			return bReturn;

		if (!szHDDSerial)
			return bReturn;

		int iLen = _tcslen(szHDDSerial);
		if (memcmp(&szClientID[_tcslen(szClientID) - iLen], szHDDSerial, iLen * sizeof(TCHAR)) == 0)
		{
			bReturn = true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CheckForHardDiskSerialNumber", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : StartScheduledScanner
Description    : To start scan at scheduled time
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 04 October 2017
***********************************************************************************************/
bool StartScheduledScanner(DWORD dwScanType, CString csScanCmdParam, int iShutDown)
{
	bool bReturn = false;
	try
	{
		CString csCommandLine;
		CExecuteProcess			objEnumprocess;
		CString csExePath;
		csExePath.Format(L"%s%s", GetWardWizPathFromRegistry(), L"VBUI.EXE");

		switch (dwScanType)
		{
		case 0:
			csCommandLine.Format(L"-SCHEDSCAN -QUICKSCAN");
			if (iShutDown == 0)
			{
				csCommandLine += L" -SDNO";
			}
			else
			{
				csCommandLine += L" -SDYES";
			}
			break;
		case 1:
			csCommandLine.Format(L"-SCHEDSCAN -FULLSCAN");
			if (iShutDown == 0)
			{
				csCommandLine += L" -SDNO";
			}
			else
			{
				csCommandLine += L" -SDYES";
			}
			break;
		case 2:
			csExePath.Format(L"%s%s", GetWardWizPathFromRegistry(), L"VBUSBDETECTUI.EXE");
			csCommandLine.Format(L"-SCHEDSCAN -CUSTOMSCAN ");
			if (iShutDown == 0)
			{
				csCommandLine += L"-SDNO ";
			}
			else
			{
				csCommandLine += L"-SDYES ";
			}
			csCommandLine += csScanCmdParam;
			break;
		case 4:
			CleanTemporaryFiles();
			break;
		case 5:
			csCommandLine.Format(L"-SCHEDSCAN -REGOPT -SDNO ");
			csCommandLine += csScanCmdParam;
			break;
		default:
			return bReturn;
		}
		objEnumprocess.StartProcessWithTokenExplorerWait(csExePath, csCommandLine, L"explorer.exe");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartScheduledScanner", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : AddBootScannerEntry
Description    : Function to add Boot scanner entry
SR.NO		   :
Author Name    : Ram Shelke
Date           : 10 October 2017
***********************************************************************************************/
bool AddBootScannerEntry(LPTSTR pKeyName, LPTSTR pValueName, LPTSTR pNewValue)
{
	bool bReturn = false;
	LONG lResult = 0;
	HKEY hKey = NULL;
	LPTSTR lpValues = NULL;
	LPTSTR lpValue = NULL;
	LPTSTR lpNewValues = NULL;
	LPTSTR lpNewValue = NULL;
	DWORD cbValues = 0;
	DWORD cbNewValues = 0;
	DWORD cbNewValue = 0;

	__try
	{

		// OPEN THE REGISTRY KEY
		lResult = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			pKeyName,
			0,
			KEY_ALL_ACCESS | KEY_WOW64_64KEY,
			&hKey
			);

		TCHAR szError[MAX_PATH] = { 0 };
		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			return false;
		}

		// READ THE REG_MULTI_SZ VALUES
		//
		// Get size of the buffer for the values
		lResult = RegQueryValueEx(
			hKey,
			pValueName,
			NULL,
			NULL,
			NULL,
			&cbValues
			);

		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// Allocate the buffer
		lpValues = (LPTSTR)malloc(cbValues);
		memset(lpValues, 0, cbValues);

		if (NULL == lpValues)
		{
			swprintf(szError, _T("ERROR 0x%x\n"), GetLastError());
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// Get the values
		lResult = RegQueryValueEx(
			hKey,
			pValueName,
			NULL,
			NULL,
			(LPBYTE)lpValues,
			&cbValues
			);

		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// SHOW THE VALUES
		//
		lpValue = lpValues;

		bool bValueExists = false;
		for (; '\0' != *lpValue; lpValue += _tcslen(lpValue) + 1)
		{
			if (0 == _tcscmp(lpValue, pNewValue))
			{
				bValueExists = TRUE;
			}
		}

		//if value exists then no need to update.
		if (bValueExists)
		{
			bReturn = false;
			goto CLEANUP;
		}

		// INSERT A NEW VALUE AFTER A SPECIFIC VALUE IN THE LIST OF VALUES
		//
		// Allocate a new buffer for the old values plus the new one
		cbNewValue = ((DWORD)_tcslen(pNewValue) + 1) * sizeof(TCHAR);
		cbNewValues = cbValues + cbNewValue;
		lpNewValues = (LPTSTR)malloc(cbNewValues);
		memset(lpNewValues, 0, cbNewValues);

		if (NULL == lpNewValues)
		{
			swprintf(szError, _T("ERROR 0x%x\n"), GetLastError());
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// Find the value after which we will insert the new one
		lpValue = lpValues;
		lpNewValue = lpNewValues;

		for (; '\0' != *lpValue; lpValue += _tcslen(lpValue) + 1)
		{
			// Copy the current value to the target buffer
			memcpy(lpNewValue, lpValue, (_tcslen(lpValue) + 1) * sizeof(TCHAR));
			lpNewValue += _tcslen(lpValue) + 1;
		}

		//We didn't find the value we wanted. Insert the new value at the end
		memcpy(lpNewValue, pNewValue, (_tcslen(pNewValue) + 1) * sizeof(TCHAR));
		lpNewValue += _tcslen(pNewValue) + 1;

		*lpNewValue = *lpValue;

		//WRITE THE NEW VALUES BACK TO THE KEY
		lResult = RegSetValueEx(
			hKey,
			pValueName,
			NULL,
			REG_MULTI_SZ,
			(LPBYTE)lpNewValues,
			cbNewValues
			);

		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in AddBootScannerEntry, KeyName:[%s], ValueName: [%s]", pKeyName, pValueName, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:
	if (NULL != lpValues) { free(lpValues); lpValues = NULL; }
	if (NULL != lpNewValues) { free(lpNewValues); lpNewValues = NULL; }
	if (NULL != hKey) { RegCloseKey(hKey); hKey = NULL; }
	return bReturn;
}


/***********************************************************************************************
Function Name  : AddBootScannerEntry
Description    : Function to add Boot scanner entry
SR.NO		   :
Author Name    : Ram Shelke
Date           : 10 October 2017
***********************************************************************************************/
bool RemoveBootScannerEntry(LPTSTR pKeyName, LPTSTR pValueName)
{
	bool bReturn = true;
	LONG lResult = 0;
	HKEY hKey = NULL;
	LPTSTR lpValues = NULL;
	LPTSTR lpValue = NULL;
	LPTSTR lpNewValues = NULL;
	LPTSTR lpNewValue = NULL;
	DWORD cbValues = 0;
	DWORD cbNewValues = 0;
	DWORD cbNewValue = 0;

	try
	{
		if (!pKeyName || !pValueName)
			return false;

		CString csExePath = GetWardWizPathFromRegistry();
		csExePath += WRDWIZBOOTSCN;

		int iLength = GetShortPathName(csExePath, NULL, 0);
		if (iLength == 0x00)
		{
			AddLogEntry(L"### Failed GetShortPathName in RemoveBootScannerEntry", 0, 0, true, SECONDLEVEL);
			return false;
		}

		TCHAR szShortPath[MAX_PATH] = { 0 };
		if (GetShortPathName(csExePath, szShortPath, iLength) == 0x00)
		{
			AddLogEntry(L"### Failed GetShortPathName in RemoveBootScannerEntry", 0, 0, true, SECONDLEVEL);
			return false;
		}

		// OPEN THE REGISTRY KEY
		lResult = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			pKeyName,
			0,
			KEY_ALL_ACCESS | KEY_WOW64_64KEY,
			&hKey
			);

		TCHAR szError[MAX_PATH] = { 0 };
		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in RemoveBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			return false;
		}

		// READ THE REG_MULTI_SZ VALUES
		//
		// Get size of the buffer for the values
		lResult = RegQueryValueEx(
			hKey,
			pValueName,
			NULL,
			NULL,
			NULL,
			&cbValues
			);

		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in RemoveBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// Allocate the buffer
		lpValues = (LPTSTR)malloc(cbValues);
		memset(lpValues, 0, cbValues);

		if (NULL == lpValues)
		{
			swprintf(szError, _T("ERROR 0x%x\n"), GetLastError());
			AddLogEntry(L"### RegOpenKeyEx in RemoveBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// Get the values
		lResult = RegQueryValueEx(
			hKey,
			pValueName,
			NULL,
			NULL,
			(LPBYTE)lpValues,
			&cbValues
			);

		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in RemoveBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// SHOW THE VALUES
		//
		lpValue = lpValues;

		bool bValueExists = false;
		for (; '\0' != *lpValue; lpValue += _tcslen(lpValue) + 1)
		{
			if (0 == _tcscmp(lpValue, szShortPath))
			{
				bValueExists = TRUE;
			}
		}

		//if value exists then no need to update.
		if (!bValueExists)
		{
			bReturn = false;
			goto CLEANUP;
		}

		// DELETE A VALUE 
		// Allocate a new buffer for the old values plus the new one
		cbNewValue = ((DWORD)_tcslen(szShortPath) + 1) * sizeof(TCHAR);
		cbNewValues = cbValues - cbNewValue;
		lpNewValues = (LPTSTR)malloc(cbNewValues);
		memset(lpNewValues, 0, cbNewValues);

		if (NULL == lpNewValues)
		{
			swprintf(szError, _T("ERROR 0x%x\n"), GetLastError());
			AddLogEntry(L"### RegOpenKeyEx in RemoveBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// Find the value after which we will insert the new one
		lpValue = lpValues;
		lpNewValue = lpNewValues;

		for (; '\0' != *lpValue; lpValue += _tcslen(lpValue) + 1)
		{
			if (0 == _tcscmp(lpValue, szShortPath))
			{
				continue;
			}

			// Copy the current value to the target buffer
			memcpy(lpNewValue, lpValue, (_tcslen(lpValue) + 1) * sizeof(TCHAR));
			lpNewValue += _tcslen(lpValue) + 1;
		}

		*lpNewValue = *lpValue;

		//WRITE THE NEW VALUES BACK TO THE KEY
		lResult = RegSetValueEx(
			hKey,
			pValueName,
			NULL,
			REG_MULTI_SZ,
			(LPBYTE)lpNewValues,
			cbNewValues
			);

		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in RemoveBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RemoveBootScannerEntry, KeyName:[%s], ValueName: [%s]", pKeyName, pValueName, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:
	if (NULL != lpValues) { free(lpValues); lpValues = NULL; }
	if (NULL != lpNewValues) { free(lpNewValues); lpNewValues = NULL; }
	if (NULL != hKey) { RegCloseKey(hKey); hKey = NULL; }
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: ReadBootScannedEntries
*  Description			: Function which reads the boot time entries which merge into recover DB.
*  Function Arguments	:
*  Author Name			: Ram Shelke
*  Date					: 24 OCT 2017
**********************************************************************************************************/
bool ReadBootScannedEntries()
{
	bool bReturn = false;
	__try
	{
		bReturn = g_objISpyScannerBase.m_objWardWizScanner.ReadBootRecoverEntries();
		INT64 iSessionID = 0x00;
		if (g_objISpyScannerBase.m_objWardWizScanner.AddScanSessionDetails(iSessionID))
		{
			if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"ScanType", 0x06) != 0x00)
			{
				AddLogEntry(L"### Failed SetRegistryValueData in ReadBootScannedEntries", 0, 0, true, SECONDLEVEL);
			}
			bReturn = g_objISpyScannerBase.m_objWardWizScanner.AddScanDetails(iSessionID);
		}

		//reset here boot time scanner entries.
		ResetBootTimeScanner();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ReadBootScannedEntries", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/****************************************************************************************************
*	Function Name : SendData2UI
*	Description	  : Function to send Active scan state to UI.
*	Author Name   :	Jeena
*	Date		  :	06/Dec/2017
*****************************************************************************************************/
bool SendData2UI(int iMessage, DWORD dwValue, DWORD dwSeondValue, bool bWait)
{
	DWORD dwAction = 0;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = iMessage;
		szPipeData.dwValue = dwValue;
		CISpyCommunicator objCom(UI_SERVER, false);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			CString csMessage;
			csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
			AddLogEntry(L"### Failed to set data in SendData2Tray, %s", csMessage, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csMessage;
				csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
				AddLogEntry(L"### Failed to set data in SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
				return false;
			}

			if (szPipeData.dwValue == 0x01)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in SendData2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/****************************************************************************************************
*	Function Name : SendData2Tray
*	Description	  : Function to send Active scan state to tray.
*	Author Name   :	Amol Jaware
*	Date		  :	30/Oct/2017
*****************************************************************************************************/
bool SendData2Tray(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam, LPTSTR lpszSecondParam, bool bWait)
{
	DWORD dwAction = 0;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = iMessage;
		szPipeData.dwValue = dwValue;
		if (lpszFirstParam != NULL)
		{
			_tcscpy(szPipeData.szFirstParam, lpszFirstParam);
		}

		if (lpszSecondParam != NULL)
		{
			_tcscpy(szPipeData.szSecondParam, lpszSecondParam);
		}

		if (!m_objComTray.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			CString csMessage;
			csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
			AddLogEntry(L"### Failed to set data in SendData2Tray, %s", csMessage, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objComTray.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csMessage;
				csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
				AddLogEntry(L"### Failed to set data in SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/****************************************************************************************************
*	Function Name : SendData2Tray
*	Description	  : Function to send Active scan state to tray.
*	Author Name   :	Amol Jaware
*	Date		  :	30/Oct/2017
*****************************************************************************************************/
bool SendAppBlock2Tray(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam, LPTSTR lpszSecondParam)
{
	DWORD dwAction = 0;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = iMessage;
		szPipeData.dwValue = dwValue;
		if (lpszFirstParam != NULL)
		{
			_tcscpy(szPipeData.szFirstParam, lpszFirstParam);
		}

		if (lpszSecondParam != NULL)
		{
			_tcscpy(szPipeData.szSecondParam, lpszSecondParam);
		}

		CISpyCommunicator objCom(TRAY_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			CString csMessage;
			csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
			AddLogEntry(L"### Failed to set data in SendData2Tray, %s", csMessage, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name		: ResetBootTimeScanner
*  Description			: Function which reset boot time scanner entries
*  Function Arguments	: 
*  Author Name			: Ram Shelke
*  Date					: 24 OCT 2017
**********************************************************************************************************/
bool ResetBootTimeScanner()
{
	bool bReturn = false;
	try
	{
		WCHAR szScnSessionDBPath[MAX_PATH] = { 0 };
		wcscpy(szScnSessionDBPath, GetWardWizPathFromRegistry());
		wcscat(szScnSessionDBPath, WRDWIZBTSCNSESSIONDB);

		if (PathFileExists(szScnSessionDBPath))
		{
			SetFileAttributes(szScnSessionDBPath, FILE_ATTRIBUTE_NORMAL);
			if (!DeleteFile(szScnSessionDBPath))
			{
				AddLogEntry(L"### Failed to DeleteFile, File: [%s]", szScnSessionDBPath, 0, true, SECONDLEVEL);
			}
		}

		WCHAR szIniFilePath[MAX_PATH] = { 0 };
		wcscpy(szScnSessionDBPath, GetWardWizPathFromRegistry());
		wcscat(szScnSessionDBPath, L"VBSETTINGS");
		wcscat(szScnSessionDBPath, L"\\ProductSettings.ini");
		WritePrivateProfileString(L"VBSETTINGS", L"BootTimeScan ", L"0", szScnSessionDBPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ResetBootTimeScanner", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : StartFirewallWorkerThread
Description    : Initialize the firewall library
Author Name    : Ramkrushna Shelke
Date           : 25 Jul 2018
***********************************************************************************************/
DWORD StartFirewallWorkerThread(LPVOID lpParam)
{
	__try
	{
		return StartFirewallWorkerThreadSEH(lpParam);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in StartFirewallWorkerThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : StartFirewallWorkerThreadSEH
Description    : Initialize the firewall library 
Author Name    : Ramkrushna Shelke
Date           : 25 Jul 2018
***********************************************************************************************/
DWORD StartFirewallWorkerThreadSEH(LPVOID lpParam)
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			//to create firewall report table
			g_objFWClient.CreateTable4Firewall();
			
			//get here main firewall setting
			DWORD dwFirewallEnableState = 0x00;
			if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwFirewallEnableState", dwFirewallEnableState) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwFirewallEnableState in StartFirewallWorkerThreadSEH KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);
			}

			//if firewall is disabled, return from here.
			if (dwFirewallEnableState == 0x00)
			{
				return 0;
			}

			//Initialize firewall here
			g_objFWClient.Initialize();

			DWORD dwStealthMode = 0x00;
			if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwStealthMode", dwStealthMode) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwStealthMode in StartFirewallWorkerThreadSEH KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);
			}

			//check here stealth mode is enabled
			if (dwStealthMode == 0x01)
			{
				g_objFWClient.ReLoadFWControlSettings4StealthMode(dwStealthMode);
			}

			DWORD dwPortScanMode = 0x00;
			if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwPortScanProt", dwPortScanMode) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwPortScanProt in StartFirewallWorkerThreadSEH KeyPath: %s", g_csRegKeyPath, 0, true, SECONDLEVEL);
			}

			//check here port scan mode is enabled
			if (dwPortScanMode == 0x01)
			{
				g_objFWClient.ReLoadFWControlSettings4PortScanProt(dwPortScanMode);
			}
		}
		hFirewallThread = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in StartFirewallWorkerThreadSEH", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : StartEMailScanWorkerThread
Description    : Start the StartEMailScanWorkerThreadSEH thread.
Author Name    : Amol J.
Date           : 06 Feb 2018
***********************************************************************************************/
DWORD StartEMailScanWorkerThread(LPVOID lpParam)
{
	__try
	{
		return StartEMailScanWorkerThreadSEH(lpParam);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in StartEMailScanWorkerThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}



/***********************************************************************************************
Function Name  : StartEMailScanWorkerThreadSEH
Description    : Initialize the library and Filter all TCP connections.
Author Name    : Amol J.
Date           : 06 Feb 2018
***********************************************************************************************/
DWORD StartEMailScanWorkerThreadSEH(LPVOID lpParam)
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.Initialize();
			g_objWebProtClient.Initialize();
			g_objBrowserSec.Initialize();
		}
		hEmailScanThread = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in StartEMailScanWorkerThreadSEH", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : StartEmailScanThread
Description    : Creating the thread StartEMailScanWorkerThread.
Author Name    : Amol J.
Date           : 06 Feb 2018
***********************************************************************************************/
void StartEmailScanThread()
{
	__try
	{
		if (m_dwDaysLeft == 0x00)
			return;

		if (hEmailScanThread == NULL)
		{
			hEmailScanThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartEMailScanWorkerThread, NULL, 0, 0);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartEmailScanThread", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : StartEmailScanThread
Description    : Creating the thread StartEMailScanWorkerThread.
Author Name    : Amol J.
Date           : 06 Feb 2018
***********************************************************************************************/
void StartFirewallThread()
{
	__try
	{
		if (m_dwDaysLeft == 0x00)
			return;

		if (hFirewallThread == NULL)
		{
			hFirewallThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartFirewallWorkerThread, NULL, 0, 0);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartFirewallThread", 0, 0, true, SECONDLEVEL);
	}
}


/**********************************************************************************************************
*  Function Name  :	SendData2EPSClient
*  Description    :	Function which sends detected entries data to wardwiz EPS client
*  Author Name    : Ram Shelke
*  Date           : 7th March 2018
*  SR_NO		  : strVirusName, strScanFileName, csAction
**********************************************************************************************************/
bool SendData2EPSClient(LPCTSTR szFilePath, LPCTSTR szVirusName, DWORD dwActionTaken)
{
	try
	{

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SHOW_VIRUSENTRY;
		szPipeData.dwSecondValue = dwActionTaken;
		wcscpy_s(szPipeData.szFirstParam, _T("0")); //send here task ID
		wcscpy_s(szPipeData.szSecondParam, szVirusName);
		wcscpy_s(szPipeData.szThirdParam, szFilePath);

		CISpyCommunicator objCom(EPS_CLIENT_AGENT, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SendData2EPSClient", 0, 0, true, SECONDLEVEL);
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::SendData2EPSClient, File: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  LauchAppInUserContext
*  Description    :  Function which lauch application in user context.
*  Author Name    :  Amol J.
*  Date           :  10 Mar 2018
****************************************************************************************************/
bool LauchAppInUserContext(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (!lpSpyData)
			return false;

		if (_tcslen(lpSpyData->szFirstParam) == 0x00 || _tcslen(lpSpyData->szSecondParam) == 0x00)
			return false;

		CWardWizOSversion		objOSVersionWrap;
		DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
		switch (OSType)
		{
		case WINOS_XP:
		case WINOS_XP64:
			ShellExecute(NULL, NULL, lpSpyData->szFirstParam, lpSpyData->szSecondParam, NULL, SWP_HIDEWINDOW);
			break;
		default:
			ShellExecute(NULL, L"runas", lpSpyData->szFirstParam, lpSpyData->szSecondParam, NULL, SWP_HIDEWINDOW);
			break;
		}

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LauchAppInUserContext", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  LauchAppInUserContext
*  Description    :  Function which lauch application in user context.
*  Author Name    :  Amol J.
*  Date           :  10 Mar 2018
****************************************************************************************************/
bool LauchApplication(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (!lpSpyData)
			return false;

		if (_tcslen(lpSpyData->szFirstParam) == 0x00 || _tcslen(lpSpyData->szSecondParam) == 0x00)
			return false;

		CExecuteProcess			objExecprocess;
		if (objExecprocess.StartProcessWithTokenExplorerWait(lpSpyData->szFirstParam, lpSpyData->szSecondParam, L"explorer.exe"))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LauchAppInUserContext", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  StartParControlThread
*  Description    :  Function to start Parental Control Thread
*  Author Name    :  Jeena Mariam Saji
*  Date           :  18 June 2018
****************************************************************************************************/
void StartParentalControlCheck()
{
	__try
	{
		if (m_hParCtrlThread == NULL)
		{
			m_hParCtrlThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartParCtrlWrkerThread, NULL, 0, 0);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartParentalControlCheck", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  StartPCInternetCheck
*  Description    :  Function to start Parental Control Internet Usage check
*  Author Name    :  Jeena Mariam Saji
*  Date           :  19 July 2018
****************************************************************************************************/
void StartPCInternetCheck()
{
	__try
	{
		if (m_hParCtrlInternet == NULL)
		{
			m_hParCtrlInternet = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartPCInternetTimeCheckThread, NULL, 0, 0);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartPCInternetCheck", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  StartINetAccessTimeCheck
*  Description    :  Function to start Parental Control Internet Access
*  Author Name    :  Nitin Shelar
*  Date           :  20 July 2018
****************************************************************************************************/
void StartINetAccessTimeCheck()
{
	__try
	{
		if (m_hParCtrlINetCheckThread == NULL)
		{
			m_hParCtrlINetCheckThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartINetAccessTimeCheckThread, NULL, 0, 0);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartINetAccessTimeCheck", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  StartPCInternetTimeCheckThread
*  Description    :  Function to start Parental Control Internet check
*  Author Name    :  Jeena Mariam Saji
*  Date           :  07 Sept 2018
****************************************************************************************************/
DWORD StartPCInternetTimeCheckThread(LPVOID lpParam)
{
	__try
	{
		DWORD dwParCtrlValue = 0x00;
		while (1)
		{
			if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParCtrlValue) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in StartPCInternetTimeCheckThread KeyPath: %s", g_csRegKeyPath, 0, true, ZEROLEVEL);;
			}

			if (dwParCtrlValue)
			{
				StartParCtrlInternetCheck();
			}

			ResetEvent(hPCtrlINetEvent);
			DWORD dwWaitTime = 2 * 1000 * 60;
			WaitForSingleObject(hPCtrlINetEvent, dwWaitTime);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in StartPCInternetTimeCheckThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  StartINetAccessTimeCheckThread
*  Description    :  Function to start Parental Control Internet Access
*  Author Name    :  Nitin Shelar
*  Date           :  20 July 2018
****************************************************************************************************/
DWORD StartINetAccessTimeCheckThread(LPVOID lpParam)
{
	__try
	{
		DWORD dwParCtrlValue = 0x00;
		while (1)
		{
			if (m_dwDaysLeft != 0x00)
			{
				if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParCtrlValue) != 0x00)
				{
					AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in StartINetAccessTimeCheckThread KeyPath: %s", g_csRegKeyPath, 0, true, ZEROLEVEL);;
				}
				if (dwParCtrlValue)
				{
					StartParCtrlINetUsageCheck();
				}
				else
				{
					g_objEmailClient.EnableDisableInternetUsage(false);
				}
			}
			ResetEvent(hPCtrlINetUsgEvent);
			DWORD dwWaitTime = 2 * 1000 * 60;
			WaitForSingleObject(hPCtrlINetUsgEvent, dwWaitTime);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in StartINetAccessTimeCheckThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  StartParCtrlWrkerThread
*  Description    :  Function to start Parental Control Thread
*  Author Name    :  Jeena Mariam Saji
*  Date           :  18 June 2018
****************************************************************************************************/
DWORD StartParCtrlWrkerThread(LPVOID lpParam)
{
	__try
	{
		DWORD dwParCtrlValue= 0x00;
		while (1)
		{
			if (m_dwDaysLeft != 0x00)
			{
				if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParCtrlValue) != 0x00)
				{
					AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in StartParCtrlWrkerThread KeyPath: %s", g_csRegKeyPath, 0, true, ZEROLEVEL);;
				}

				if (dwParCtrlValue)
				{
					StartParentalCtrlCheck();
				}
				else
				{
					g_objEmailClient.EnableDisableInternetUsage(false);
				}
			}
			ResetEvent(hParCtrlEvent);

			DWORD dwWaitTime = 2 * 1000 * 60;
			WaitForSingleObject(hParCtrlEvent, dwWaitTime);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in StartParCtrlWrkerThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  OnReloadUserList
*  Description    :  Function to reload user list
*  Author Name    :  Jeena Mariam Saji
*  Date           :  11 JULY 2019
****************************************************************************************************/
bool OnReloadUserList()
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.ReloadUserList();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnReloadUserList", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  OnParCtrlResetValue
*  Description    :  Function to reset user value
*  Author Name    :  Jeena Mariam Saji
*  Date           :  01 AUGUST 2019
****************************************************************************************************/
bool OnParCtrlResetValue()
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.ReloadUserResetValue();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnParCtrlResetValue", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  OnParBrowserSecUninit
*  Description    :  Function to uninitialize Browser security
*  Author Name    :  Jeena Mariam Saji
*  Date           :  22 Oct 2019
****************************************************************************************************/
bool OnParBrowserSecUninit()
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objBrowserSec.UnInitialize();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnParBrowserSecUninit", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/***********************************************************************************************
Function Name  : ReloadBrowseProtDB
Description    : Function to reload Browsing security settings DB.
Author Name    : Ramkrushna Selk
Date           : 05 Oct 2018
***********************************************************************************************/
bool ReloadBrowseProtDB()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objWebProtClient.ReloadBrowseProtDB();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadExcludeDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadExcludeDB
Description    : Function to reload Web protection DB files.
Author Name    : Ramkrushna Selk
Date           : 05 Oct 2018
***********************************************************************************************/
bool ReloadWebSecDB()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objWebProtClient.ReloadWebSecDB();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadExcludeDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadUserAccessList
Description    : Function to reload permissions for users.
Author Name    : Jeena Mariam Saji
Date           : 16 July 2019
***********************************************************************************************/
bool ReloadUserAccessList()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objEmailClient.ReloadUserPermission();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadUserAccessList", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadBrowseProtDB
Description    : Function to reload Browsing security settings DB.
Author Name    : Ramkrushna Selk
Date           : 05 Oct 2018
***********************************************************************************************/
bool ReloadMngExcDB()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objWebProtClient.ReloadMngExcDB();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadExcludeDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : OnParCtrlUnInitialise
Description    : Function to uninitialise Par Ctrl values on toggler change
Author Name    : Jeena Mariam Saji
Date           : 22 July 2019
***********************************************************************************************/
bool OnParCtrlUnInitialise()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objWebProtClient.UnInitialize();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnParCtrlUnInitialise", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : OnReloadBrowserSecurity
Description    : Function to reload Browser Security Values
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
bool OnReloadBrowserSecurity()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objBrowserSec.ReloadBrowserSecVal();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnReloadBrowserSecurity", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : OnReloadBrowserSecurityExc
Description    : Function to reload Browser Security Exclusion List
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
bool OnReloadBrowserSecurityExc()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objBrowserSec.ReloadBrowserSecExcVal();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnReloadBrowserSecurityExc", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : OnReloadBrowserSecuritySpec
Description    : Function to reload Browser Security Specific URL List
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
bool OnReloadBrowserSecuritySpec()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objBrowserSec.ReloadBrowserSecSpecVal();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnReloadBrowserSecuritySpec", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : OnReloadBrowserSecurityVal
Description    : Function to reload Browser Security Values
Author Name    : Jeena Mariam Saji
Date           : 05 Sept 2019
***********************************************************************************************/
bool OnReloadBrowserSecurityVal()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objBrowserSec.Initialize();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnReloadBrowserSecurityVal", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : OnParCtrlInitialise
Description    : Function to initialise Par Ctrl values on toggler change
Author Name    : Jeena Mariam Saji
Date           : 22 July 2019
***********************************************************************************************/
bool OnParCtrlInitialise()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objWebProtClient.Initialize();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnParCtrlInitialise", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadBrowseProtDB
Description    : Function to reload Browsing security settings DB.
Author Name    : Ramkrushna Selk
Date           : 05 Oct 2018
***********************************************************************************************/
bool ReloadBlkSpecWebDB()
{
	bool bReturn = false;
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			bReturn = g_objWebProtClient.ReloadBlkSpecWebDB();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadExcludeDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  ReloadApplicationRules
*  Description    :  Function to reload application rules
*  Author Name    :  Ram Shelke
*  Date           :  18 JUN 2018
****************************************************************************************************/
bool ReloadApplicationRules(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.ReloadApplicationRules4FW();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadApplicationRules", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReloadApplicationRestriction
*  Description    :  Function to reload application restriction
*  Author Name    :  Jeena Mariam Saji
*  Date           :  20 JULY 2018
****************************************************************************************************/
bool ReloadApplicationRestriction()
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.ReLoadBlockedAppList();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadApplicationRules", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  UninitializeNFApi
*  Description    :  Function to uninitialize nf api's memory.
*  Author Name    :  Amol Jaware
*  Date           :  01 Aug 2018
****************************************************************************************************/
bool UninitializeNFApi()
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.UnInitialize();
			g_objWebProtClient.UnInitialize();
			g_objBrowserSec.UnInitialize();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UninitializeNFApi", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReloadFirewallRules
*  Description    :  Function to reload firewall rules.
*  Author Name    :  Ram Shelke
*  Date           :  01 Aug 2018
****************************************************************************************************/
bool ReloadFirewallRules()
{
	try
	{
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadFirewallRules", 0, 0, true, SECONDLEVEL);
	}
	return 0;

}

/***************************************************************************************************
*  Function Name  :  ReLoadFWControlSettings
*  Description    :  Function to reload firewall rules.
*  Author Name    :  Amol Jaware
*  Date           :  06 Aug 2018
****************************************************************************************************/
bool ReLoadFWControlSettings(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (!lpSpyData)
			return false;

		if (m_dwDaysLeft != 0x00)
		{
			switch (lpSpyData->dwValue)
			{
			case RELOAD_FIREWALL_ON_OF:
				g_objFWClient.ReLoadFWControlSettingsTab(lpSpyData->dwSecondValue);
				break;
			case RELOAD_FIREWALL_PORT_SCN_REG:
				g_objFWClient.ReLoadFWControlSettings4PortScanProt(lpSpyData->dwSecondValue, lpSpyData->szFirstParam);
				break;
			case RELOAD_FIREWALL_STEALTH_MODE_REG:
				g_objFWClient.ReLoadFWControlSettings4StealthMode(lpSpyData->dwSecondValue, lpSpyData->szFirstParam);
				break;
			case RELOAD_FIREWALL_DEF_APP_BEHAVIOR:
				g_objFWClient.SetDefaultAppBehaviour(lpSpyData->dwSecondValue);
				break;
			default:
				AddLogEntry(L"### Unhandeled case ReLoadFWControlSettings", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReLoadFWControlSettings", 0, 0, true, SECONDLEVEL);
	}
	return 0;

}

/***************************************************************************************************
*  Function Name  :  ReLoadParentalControlSettings
*  Description    :  Function to reload parental control main toggler.
*  Author Name    :  Amol Jaware.
*  Date           :  06 Aug 2018
****************************************************************************************************/
bool ReLoadParentalControlSettings(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (!lpSpyData)
			return false;

		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.ReLoadParentalControlSettingsTab(lpSpyData->dwValue);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReLoadParentalControlSettings", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReLoadEmailScanSettings
*  Description    :  Function to reload email scan main toggler.
*  Author Name    :  Amol Jaware
*  Date           :  06 Aug 2018
****************************************************************************************************/
bool ReLoadEmailScanSettings(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (!lpSpyData)
			return false;
		
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.ReLoadEmailScanSettingsTab(lpSpyData->dwValue);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReLoadEmailScanSettings", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  StartParentalCtrlCheck
*  Description    :  Function to start Parental Control Check
*  Author Name    :  Jeena Mariam Saji
*  Date           :  10 Aug 2018
****************************************************************************************************/
bool StartParentalCtrlCheck()
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.StartParentalControlCheck();
			g_objEmailClient.ReLoadBlockedAppList();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartParentalCtrlCheck", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  StartParCtrlInternetCheck
*  Description    :  Function to start Parental Control Internet Check
*  Author Name    :  Jeena Mariam Saji
*  Date           :  07 Sept 2018
****************************************************************************************************/
bool StartParCtrlInternetCheck()
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.StartPCInternetUsageCheck();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartParCtrlInternetCheck", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  StartParCtrlCompUsageCheck
*  Description    :  Function to start Parental Control Comp Usage Check
*  Author Name    :  Jeena Mariam Saji
*  Date           :  10 Aug 2018
****************************************************************************************************/
bool StartParCtrlINetUsageCheck()
{
	try
	{
		if (m_dwDaysLeft != 0x00)
		{
			g_objEmailClient.StartPCINetUsageCheck();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartParCtrlINetUsageCheck", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : AddIntoAppBlockQueue
Description    : Function to add into application block queue.
Author Name    : Ramkrushna Shelke
Date           : 17 August 2018
***********************************************************************************************/
bool AddIntoAppBlockQueue(ULONG ulCallerProcID, PWCHAR usAccessedFileName)
{
	try
	{
		if (!usAccessedFileName)
			return false;

		BLOCKAPPFLAGS	stBlockApp;
		stBlockApp.ulCallerProcessId = ulCallerProcID;
		_tcscpy(stBlockApp.wcProcessPath, usAccessedFileName);
		g_BlockApplicationQueue.push(stBlockApp);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in AddIntoAppBlockQueue", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : ReLoadApplicationOnSwitch
Description    : Function to load application
Author Name    : Jeena Mariam Saji
Date           : 31 August 2018
***********************************************************************************************/
bool LaunchApplicationUsingService(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (!lpSpyData)
			return false;
		ReLoadApplicationOnSwitchUser(lpSpyData->szFirstParam,lpSpyData->dwSecondValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReLoadApplicationOnSwitch", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : ReLoadApplicationOnSwitchUser
Description    : Function to load application
Author Name    : Jeena Mariam Saji
Date           : 31 August 2018
***********************************************************************************************/
void ReLoadApplicationOnSwitchUser(LPTSTR szParam, DWORD dwSessionID)
{
	try
	{
		if (!szParam)
			return;

		CExecuteProcess			objExecprocess;
		objExecprocess.StartProcessWithTokenExplorer(szParam, dwSessionID, L"-NOSPSCRN", L"explorer.exe");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReLoadApplicationOnSwitchUser to execute Process: %s", szParam, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : OnFileQuarantineFromRecover
Description    : Function to make files quarantine on dragging and dropping
Author Name    : Akshay Patil
Date           : 05 Oct 2018
***********************************************************************************************/
bool OnFileQuarantineFromRecover(LPTSTR szRecoverVal)
{
	bool bReturn = false;
	try
	{
		CString csQuarRecoverVal = szRecoverVal;
		if (g_objISpyScannerBase.m_objWardWizScanner.OnRecoverQuarantineEntries(szRecoverVal))
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnFileQuarantineFromRecover", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadExcludeDB
Description    : Function to reload exclude DB files.
Author Name    : Ramkrushna Selk
Date           : 05 Oct 2018
***********************************************************************************************/
bool ReloadExcludeDB()
{
	bool bReturn = false;
	try
	{
		bReturn = g_objISpyScannerBase.m_objWardWizScanner.ReloadExcludeDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadExcludeDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : CleanTemporaryFiles
Description    : Function to clean temporary files.
Author Name    : Amol Jaware
Date           : 12th March 2019
***********************************************************************************************/
void CleanTemporaryFiles()
{
	try
	{
		m_hThreadTempFileClnr = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_StartTempFileCleaner,
			NULL, 0, 0);
		
		}
	catch (...)
	{
		AddLogEntry(L"### Exception in CleanTemporaryFiles", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : Thread_StartTempFileCleaner
Description    : Start temp file cleaner thread.
Author Name    : Amol Jaware
Date           : 12th March 2019
***********************************************************************************************/
DWORD Thread_StartTempFileCleaner(LPVOID lpParam)
{
	try
	{
		StartTempFileScanning();

		RemoveTempFiles();

		SendMessage2Tray(SEND_SCHED_TEMP_FILE_FINISHED, true);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in Thread_StartTempFileCleaner", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : StartTempFileScanning
Description    : Function to start temp file scanning.
Author Name    : Amol Jaware
Date           : 12th March 2019
***********************************************************************************************/
void StartTempFileScanning()
{
	try
	{
		TCHAR	szDrives[256] = { 0 };

		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBALLREPORTS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
	}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();
		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from Wardwiz_TempFilesCleanerDetails;");

		Sleep(20);

		char szData[512] = { 0 };
		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);

			if (qResult.GetFieldIsNull(0))
	{
				continue;
	}

			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(6), -1, NULL, 0);
			wchar_t *wstrDbData = new wchar_t[wchars_num];
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(6), -1, wstrDbData, wchars_num);

			_tcscpy(m_szSpecificPath, wstrDbData);

}
		dbSQlite.Close();

		if (m_szSpecificPath[0])
	{
			_wcsupr_s(m_szSpecificPath, 511);
			EnumFolder(m_szSpecificPath);
		}

		//Sleep(1000);

		ZeroMemory(m_szSpecificPath, sizeof(m_szSpecificPath));
		GetWindowsDirectory(m_szSpecificPath, 511);
		if (m_szSpecificPath[0])
		{
			wcscat_s(m_szSpecificPath, 511, L"\\Temp");
			_wcsupr_s(m_szSpecificPath, 511);
			EnumFolder(m_szSpecificPath);
		}

		Sleep(100);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartTempFileScanning", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : EnumFolder
*  Description    : Enumerates the folder and files.
*  Author Name    :	Amol Jaware
*  Date			  : 12th March 2019
****************************************************************************************************/
void EnumFolder(LPCTSTR lpFolPath)
{
	try
	{
		Sleep(10);
		CFileFind finder;
		CString strWildcard(lpFolPath);

		if (strWildcard.IsEmpty())
			return;

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");

		BOOL bWorking = finder.FindFile(strWildcard);
		
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[1024] = { 0 };

			wsprintf(szPath, L"%s", finder.GetFilePath());
			_wcsupr_s(szPath, 1023);

			if (finder.IsDirectory())
			{
				EnumFolder(szPath);

				CheckFileForViruses(szPath);
			}
			else
		{
				CheckFileForViruses(szPath);
			}
		}

		finder.Close();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in EnumFolder", lpFolPath);
	}
}

/***************************************************************************************************
*  Function Name  : CheckFileForViruses
*  Description    : To check files for viruses.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CheckFileForViruses(LPCTSTR lpFileName)
{
	DWORD	dwRet = 0x00;
	HANDLE	hFileHandle = INVALID_HANDLE_VALUE;

	try
	{
		TCHAR	szTemp[1024] = { 0 };
		TCHAR	szFileName[256] = { 0 };

		ZeroMemory(szTemp, sizeof(szTemp));
		wcscpy_s(szTemp, 1023, lpFileName);

		TCHAR	*pFileName = wcsrchr(szTemp, '\\');

		if (pFileName)
		{
			wcscpy_s(szFileName, 255, &pFileName[1]);

			*pFileName = '\0';
		}

		AddEntryinUITable(szFileName, szTemp);
		
	}
	catch (...)
	{
		dwRet = 0xFF;
	}
	return;
}

/***************************************************************************************************
*  Function Name  : AddEntryinUITable
*  Description    : Add temp file entry in table.
*  Author Name    :	Amol Jaware
*  Date			  : 12th March 2019
****************************************************************************************************/
void AddEntryinUITable(LPCTSTR lpFileName, LPCTSTR lpFilePath)
{
	try
	{
		CString csFileDetails;
		csFileDetails += lpFilePath;
		csFileDetails += L"\\";
		csFileDetails += lpFileName;
		TCHAR m_szUSBFilePath[MAX_PATH];
		memset(m_szUSBFilePath, 0, sizeof(m_szUSBFilePath));

		m_vCsTempFileList.push_back(csFileDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in AddEntryinUITable", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CleanTemporaryFiles
Description    : Function to clean temporary files.
Author Name    : Amol Jaware
Date           : 12th March 2019
***********************************************************************************************/
void RemoveTempFiles()
{
	try
	{
		int			iDetectedFiles = 0;
		DWORD		m_dwTotalFilesCleaned = 0x00;
		CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);

		if (m_vCsTempFileList.size() > 0 )
		{
			for (int i = 0; i < static_cast<int>(m_vCsTempFileList.size()); i++)
		{
				bool bRet = false;
				DWORD dwRet = DeleteFileForcefully(m_vCsTempFileList.at(i), true);

				if (dwRet == 0x00 || dwRet == 0x01 || dwRet == 0x03)
					m_dwTotalFilesCleaned++;
			}
		}

		CString csInsertQuery;
		csInsertQuery = _T("INSERT INTO Wardwiz_TempFilesCleanerSessionDetails VALUES (null,");
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_TempFilesCleanerSessionDetails VALUES (null,Datetime('now','localtime'),Datetime('now','localtime'),Date('now'),Date('now'),%d,%d);"), m_vCsTempFileList.size(), m_dwTotalFilesCleaned);
		CT2A ascii(csInsertQuery, CP_UTF8);

		if (!PathFileExistsA(g_strDatabaseFilePath))
		{
			DWORD dwProdID = 0;
			dwProdID = GetProductID();
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(dwProdID);
			objSqlDb.Close();
		}

		InsertSQLiteData(ascii.m_psz);

		if (SendData2Tray(RELOAD_WIDGETS_UI, RELOAD_TEMPORARY_FILES))
		{
			AddLogEntry(L"### Failed to SendData2Tray in Thread_RepairVirusFiles", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RemoveTempFiles", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : DeleteFileForcefully
*  Description    : To delete temp file forcefully.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
DWORD DeleteFileForcefully(LPCTSTR lpFilePath, bool bDeleteReboot)
{

	DWORD	dwRet = 0x00;

	try
	{

		if (!PathFileExists(lpFilePath))
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		SetFileAttributes(lpFilePath, FILE_ATTRIBUTE_NORMAL);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(10);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(20);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(30);
		if (!_wremove(lpFilePath))
			goto Cleanup;

		Sleep(40);
		if (!_wremove(lpFilePath))
			goto Cleanup;

		if (PathIsDirectory(lpFilePath))
			{
			if (RemoveDirectory(lpFilePath))
				goto Cleanup;

			TCHAR	szFrom[1024] = { 0 };

			SHFILEOPSTRUCT stSHFileOpStruct = { 0 };


			wcscpy_s(szFrom, 1022, lpFilePath);

			szFrom[wcslen(szFrom)] = '\0';
			szFrom[wcslen(szFrom) + 1] = '\0';

			stSHFileOpStruct.wFunc = FO_DELETE;

			stSHFileOpStruct.pFrom = szFrom;

			stSHFileOpStruct.fFlags = FOF_NOERRORUI | FOF_MULTIDESTFILES | FOF_NOCONFIRMATION | FOF_SILENT | FOF_MULTIDESTFILES;

			stSHFileOpStruct.fAnyOperationsAborted = FALSE;
			int nFileDeleteOprnRet = SHFileOperation(&stSHFileOpStruct);

			if (!nFileDeleteOprnRet)
				goto Cleanup;
		}

		MoveFileEx(lpFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

		dwRet = 0x02;

	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CScanDlg::EnumFolder", lpFolPath);
		dwRet = 0x03;
	}

Cleanup:

	return dwRet;

}

/***************************************************************************************************
*  Function Name  : SendData2Tray
*  Description    : Function which send message data to Tray application.
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date           : 9th Nov,2017
****************************************************************************************************/
bool SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;

		CISpyCommunicator objCom(TRAY_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardwizAutoScnDlg::SendData2Tray", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CWardwizAutoScnDlg::SendData2Tray", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : AddSystemDetailsToINI
*  Description    : Function to get System Info and add in INI
*  Author Name    : Akshay Patil
*  Date           : 15 May 2019
****************************************************************************************************/
void AddSystemDetailsToINI()
{
	try
	{
		CString csIniFilePath;

		SYSDETAILS objSysdetails;
		objSysdetails = GetSystemDetails();

		if (_tcslen(objSysdetails.szOsName) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"OSName", objSysdetails.szOsName, csIniFilePath);
		}

		if (_tcslen(objSysdetails.szRAM) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"RamSize", objSysdetails.szRAM, csIniFilePath);
		}

		if (_tcslen(objSysdetails.szHardDiskSize) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"HardDiskSize", objSysdetails.szHardDiskSize, csIniFilePath);
		}

		if (_tcslen(objSysdetails.szProcessor) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"Processor", objSysdetails.szProcessor, csIniFilePath);
		}

		if (_tcslen(objSysdetails.szCompName) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"ComputerName", objSysdetails.szCompName, csIniFilePath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in AddSystemDetailsToINI", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetSystemDetails
*  Description    : Function to get System Info
*  Author Name    : Akshay Patil
*  Date           : 15 May 2019
****************************************************************************************************/
SYSDETAILS GetSystemDetails()
{
	SYSDETAILS objSysdetails;
	try
	{
		WardWizSystemInfo	objSysInfo;
		objSysInfo.GetSystemInformation();

		CString csProcessorType = L"";
		if (objSysInfo.GetOSType())
			csProcessorType.Append(L"64bit");
		else
			csProcessorType.Append(L"32bit");

		//RAM Information 
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof (statex);
		GlobalMemoryStatusEx(&statex);

		CString csRamDetail = L"";
		csRamDetail.AppendFormat(L"%*I64dGB", 7, (statex.ullTotalPhys / (1024 * 1024 * 1024)) + 1);

		//HD Size
		DISK_GEOMETRY pdg = { 0 };
		BOOL bResult = FALSE;
		ULONGLONG DiskSize = 0;
		CString csHDSize = L"";
		bResult = GetDriveGeometry(&pdg);
		if (bResult)
		{
			DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder * (ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
			csHDSize.AppendFormat(L"%.2fGB", (double)DiskSize / (1024 * 1024 * 1024));
		}
		else
		{
			AddLogEntry(L"### GetDriveGeometry failed.", 0, 0, true, SECONDLEVEL);
		}

		wcscpy_s(objSysdetails.szCompName, objSysInfo.GetSystemName());
		wcscpy_s(objSysdetails.szOsName, objSysInfo.GetOSDetails());
		wcscpy_s(objSysdetails.szRAM, csRamDetail.Trim().GetBuffer());
		wcscpy_s(objSysdetails.szHardDiskSize, csHDSize.GetBuffer());
		wcscpy_s(objSysdetails.szProcessor, csProcessorType.GetBuffer());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetSystemDetails", 0, 0, true, SECONDLEVEL);
	}

	return objSysdetails;
}

/***************************************************************************************************
*  Function Name  : GetDriveGeometry
*  Description    : Function to get disk details(TracksPerCylinder, SectorsPerTrack etc)
*  Author Name    : Akshay Patil
*  Date           : 15 May 2019
****************************************************************************************************/
BOOL GetDriveGeometry(DISK_GEOMETRY *pdg)
{
	BOOL bResult = FALSE;
	try
	{
		HANDLE hDevice = INVALID_HANDLE_VALUE;
		DWORD junk = 0;
		LARGE_INTEGER lFileSize;

		hDevice = CreateFileW(WSZDRIVE, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (hDevice == INVALID_HANDLE_VALUE)
		{
			return (FALSE);
		}
		GetFileSizeEx(hDevice, &lFileSize);

		bResult = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, pdg, sizeof(*pdg), &junk, (LPOVERLAPPED)NULL);

		CloseHandle(hDevice);

		return (bResult);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetDriveGeometry", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ISExcludedFileExt
*  Description    : Function which checks input file path in exlude list and returns
@true, if found
@else false.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 26 Sep 2018
****************************************************************************************************/
bool ISIncludedFileExt(LPTSTR lpszFileExt)
{
	bool bReturn = false;
	try
	{
		if (!lpszFileExt)
		{
			return bReturn;
		}

		if (_tcslen(lpszFileExt) >= 0x32)
		{
			return bReturn;
		}

		//Load here Exclude file extension list if not loaded.
		if (!g_bISFileExtLoaded)
		{
			if (!GetFileExtRecords())
			{
				return bReturn;
			}
			g_bISFileExtLoaded = true;
		}

		char szFileExt[0x32] = { 0 };
		wcstombs(szFileExt, lpszFileExt, sizeof(szFileExt)+1);
		_strlwr(szFileExt);

		//if there are no entries means scan all the files.
		if (g_vecExts.size() == 0x00)
		{
			return true;
		}

		std::vector<std::string>::iterator vExCludeIter;
		for (vExCludeIter = g_vecExts.begin(); vExCludeIter != g_vecExts.end(); vExCludeIter++)
		{
			if (szFileExt != NULL)
			{
				if (strcmp(szFileExt, (*vExCludeIter).c_str()) == 0)
				{
					bReturn = true;
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ISExcludedFileExt, Ext: %s", lpszFileExt, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : GetFileExtRecords
*  Description    : Function which get file extension records to be excluded
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 31 July 2019
****************************************************************************************************/
bool GetFileExtRecords()
{
	bool bReturn = false;
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWebFilterIni = L"";
		csWebFilterIni.Format(L"%s\\VBSETTINGS\\EXEXT.INI", csWardWizModulePath);
		if (!PathFileExists(csWebFilterIni))
		{
			return false;
		}

		int iCount = GetPrivateProfileInt(L"INEXT", L"Count", 0, csWebFilterIni);

		char szValueName[MAX_PATH] = { 0 };
		char szValueData[512] = { 0 };
		for (int i = 1; i <= static_cast<int>(iCount); i++)
		{
			sprintf(szValueName, "%lu", i);
			GetPrivateProfileStringA("INEXT", szValueName, "", szValueData, 511, CStringA(csWebFilterIni));
			if (szValueData[0])
			{
				//OutputDebugStringA(szValueData);
				_strlwr(szValueData);
				g_vecExts.push_back(szValueData);
			}
		}
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetFileExtRecords", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : OnCheckInternetAccess
*  Description    : Function which checks internet access
@true, if blocked
@else false.
*  Author Name    : Jeena Mariam Saji
*  Date           : 10 Dec 2019
****************************************************************************************************/
bool OnCheckInternetAccess()
{
	bool bReturn = false;
	try
	{
		if (g_objEmailClient.CheckInternetAccess())
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnCheckInternetAccess", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : UpdateRegistrationDetails
*  Description    : Function to update registration details
*  Author Name    : Ramkrushna Shelke
*  Date           : 14th Feb 2023
****************************************************************************************************/
bool UpdateRegistrationDetails2Server()
{
	bool bReturn = false;
	try
	{
		g_updateInfo = true;
		if (SetEvent(hSendRegInfoEvent) != 0)
		{
			bReturn = true;
		}
			
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UpdateRegistrationDetails", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}