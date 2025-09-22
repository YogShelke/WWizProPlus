/**********************************************************************************************************            	  
	  Program Name          : ISpyCommServer.cpp
	  Description           : Communicator class using named pipes.
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 20 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper Server class for communication using named pipes.		20 Jan 2014
							: Need to call SetFileAttributes Before every deltefile call.
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizALUSrv.h"
#include "WardWizParseIni.h"
#include "ISpyCommServer.h"
#include "iSpyMemMapServer.h"
#include "WardWizDumpCreater.h"
#include "iTinRegWrapper.h"
#include "WrdWizSystemInfo.h"
#include "ExecuteProcess.h"
#include "CSecure64.h"
#include "DriverConstants.h"
#include "CScannerLoad.h"
#include "WardWizDatabaseInterface.h"

#include <Userenv.h>
#include "WinHttpClient.h"

#define MAX_UPDATE_RETRYCOUNT	10
#define DRIVERS_FOLDER_NAME	 L"DRIVERS"

CISpyCriticalSection	g_objcriticalSection ;
std::vector<ALUFILEINFO > vALUFileInfo;
std::vector<ALUZIPFILEINFO> vALUZipFileInfo;
std::vector<CString	>vReplacedFiles;

void OnDataReceiveCallBack(LPVOID lpParam);

const TCHAR szUpdateFolderName[MAX_PATH] = L"wardwizcloud";
const TCHAR szwardwizEPSPublish[MAX_PATH] = L"WardWizDev\\WardWizEPS\\WardWizUpdates";
const TCHAR szwardwizEPSProgData[MAX_PATH] = L"WardWizEPS";
CISpyCommunicatorServer		g_objCommALUpdateServer(AUTOUPDATESRV_SERVER, &OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));
iSpyServerMemMap_Server g_objALupdateServ(ALUPDATE);

DWORD			g_dwDaysLeft = 0x00;
CString			g_csProdRegKey = "";
CString			g_csServerName = "www.vibranium.co.in";
CString			g_csTaskID = L"0";
CITinRegWrapper g_objReg;
int				g_iPreviousPerc = 0x00;
DWORD			g_dwProductID = 0x00;


/***************************************************************************************************                    
*  Function Name  : _tmain()
*  Description    : Entry point
*  Author Name    : Vilas , Neha  
*  SR_NO		  : WRDWIZALUSRV_0012
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
int _tmain(int argc, _TCHAR* argv[])
{

	DWORD	nRetCode = 0x00 ;
	TCHAR	szValue[0x80] = { 0 };
	DWORD	dwSize = sizeof(szValue);
	TCHAR	szPath[512] = { 0 };
	CITinRegWrapper	objReg;
	CString csCommandLine = GetCommandLine();
	m_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
	m_csRegKeyActiveScan = CWWizSettingsWrapper::GetProductRegistryKey();
	
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

		if (_wcsicmp(argv[1], L"GETLASTSCANTIME") == 0)
		{
			objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, m_csRegKeyPath.GetBuffer(), L"LastScandt", szPath, dwSize);
			_tprintf(L"%s", szPath);
			return 0;
		}

		if (_wcsicmp(argv[1], L"ENABLERTP") == 0)
		{
			try
			{
				if (argc < 3)
				{
					printf("Invalid argument.");
					return 0;
				}
				objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, m_csRegKeyActiveScan.GetBuffer(), L"dwActiveScanOption", szPath, dwSize);

				if (_wcsicmp(argv[2], L"1") == 0)
				{
					dwEnableProtection = 1;
					if (!SetRegistrykeyUsingService(m_csRegKeyActiveScan, L"dwActiveScanOption", REG_DWORD, dwEnableProtection, true))
					{
						AddLogEntry(L"### Error in Setting Registry VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
					}

					if (!SendData2Service(HANDLEACTICESCANSETTINGS, ENABLEACTSCAN, 0, 0, 0, true))
					{
						AddLogEntry(L"### Failed to SendData to Service VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
					}
				}
				else if (_wcsicmp(argv[2], L"0") == 0)
				{
					dwEnableProtection = 0;
					if (!SetRegistrykeyUsingService(m_csRegKeyActiveScan, L"dwActiveScanOption", REG_DWORD, dwEnableProtection, true))
					{
						AddLogEntry(L"### Error in Setting Registry VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
					}

					if (!SendData2Service(HANDLEACTICESCANSETTINGS, DISABLEACTSCAN, DISABLE_PERMANANT, 0, 0, true))
					{
						AddLogEntry(L"### Failed to SendData to Service VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
					}
				}
				if (!SendData2UI(SEND_ACTIVE_PROTECTION_STATUS, false))
				{
					AddLogEntry(L"### Failed to SendData to UI VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
				}
			}
			catch (...)
			{
				AddLogEntry(L"### Failed to SendData to UI VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
			}
			return 0;
		}
		if (_wcsicmp(argv[1], L"GETLASTTHREATDETAILS") == 0)
		{
			try
			{
				CWardWizSQLiteDatabase dbSQlite;
				objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, m_csRegKeyPath.GetBuffer(), L"AppFolder", szPath, dwSize);
				CString csWardWizModulePath = szPath;
				CString	csWardWizReportsPath = L"";
				csWardWizReportsPath.Format(L"%sVBALLREPORTS.DB",csWardWizModulePath);

				if (!PathFileExists(csWardWizReportsPath))
				{
					return 0;
				}

				CT2A dbPath(csWardWizReportsPath, CP_UTF8);
				dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

				dbSQlite.Open();

				CWardwizSQLiteTable qResult = dbSQlite.GetTable("select * from Wardwiz_ScanDetails where db_ScanID=(select max (db_ScanID) from Wardwiz_ScanDetails);");

				STRUCTSCANDETAILS szScanDetails;

				if (qResult.GetNumRows() == 0)
					printf("No_Threats \t: true");
				else
				{
					printf("No_Threats \t: false");	//no_threat boolean value
					strcpy_s(szScanDetails.szData, qResult.GetFieldValue(7)); //threat path
					printf("\nPath \t\t: %s", szScanDetails.szData);
					strcpy_s(szScanDetails.szData, qResult.GetFieldValue(6)); //threat name
					printf("\nThreat_Name \t: %s", szScanDetails.szData);
					strcpy_s(szScanDetails.szData, qResult.GetFieldValue(5)); //end time
					printf("\nScan_Time \t: %s", szScanDetails.szData);
				}
				dbSQlite.Close();
			}
			catch (...)
			{
				AddLogEntry(L"### Failed to passing argument GETLASTTHREATDETAILS to VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
			}
			return 0;
		}

		if (_wcsicmp(argv[1], L"GETLASTDBUPDATEDATE") == 0)
		{
			try
			{
				WinHttpClient client(L"http://www.wardwiz.com/wardbuploader/lastupdate.txt");

				if (!client.SendHttpRequest())
				{
					_tprintf(L"Command failed: Could not find enough information.");
					return 0;
				}
				else
				{
					wstring httpResponseContent = client.GetResponseContent();
					_tprintf(L"%s", httpResponseContent.c_str());
					return 0;
				}
			}
			catch (...)
			{
				AddLogEntry(L"### Failed to passing argument GETLASTDBUPDATEDATE to VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
			}
		}
		if (_wcsicmp(argv[1], L"ENCRSTATE") == 0)
		{
			try
			{
				TCHAR			szActualINIPath[255] = { 0 };
				TCHAR			szLastEncrFile[255] = { 0 };
				CString			csIniFilePath;
				CString			csLastEncrFile;

				csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
				GetPrivateProfileString(L"VBSETTINGS", L"LastEncrypted", L"", szLastEncrFile, 511, csIniFilePath);
				csLastEncrFile = (LPCTSTR)szLastEncrFile;
				if (csLastEncrFile == L"")
				{
					_tprintf(L"No result found...");
				}
				else
				{
					_tprintf(L"%s\n", szLastEncrFile);
					_tprintf(L"Algorithm : AES-256");
				}
			}
			catch (...)
			{
				AddLogEntry(L"### Failed to pass argument ENCRSTATE to VibraniumALUSrv::_tmain", 0, 0, true, SECONDLEVEL);
			}
			return 0;
		}
		return nRetCode ;
	}

	StartServiceCtrlDispatcher( SrvTable ) ;

	return nRetCode ;

}

/***************************************************************************************************                    
*  Function Name  : ServiceMain()
*  Description    : Service entry point function
*  Author Name    : Vilas , Neha   
*  SR_NO		  : WRDWIZALUSRV_0013
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
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

/***************************************************************************************************                    
*  Function Name  : ServiceCtrlHandler()
*  Description    : Service control handler.
*  Author Name    : Vilas , Neha  
*  SR_NO		  : WRDWIZALUSRV_0014
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
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

/***************************************************************************************************                    
*  Function Name  : UpdateServiceStatus()
*  Description    : Update service status
*  Author Name    : Vilas , Neha  
*  SR_NO		  : WRDWIZALUSRV_0015
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
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

/***************************************************************************************************                    
*  Function Name  : StartServiceWorkerThread()
*  Description    : Start service worker thread
*  Author Name    : Vilas , Neha 
*  SR_NO		  : WRDWIZALUSRV_0016
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void StartServiceWorkerThread(DWORD dwArgC, LPTSTR *pArgV)
{
	__try
	{
		hEvent_ServiceStop = CreateEvent(NULL, TRUE, FALSE, NULL) ;
		if( !hEvent_ServiceStop )
		{
			//AddToLo
			return ;
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


		AddServiceEntryInSafeMode();

		hThread_SrvPrimary = CreateThread(	NULL, 0, (LPTHREAD_START_ROUTINE) SrvPrimaryThread, 
			NULL, 0, 0 ) ;
		Sleep( 500 ) ;

		if(hThread_SrvPrimary == NULL)
		{
			StopServiceByEvent();
		}


		WaitForSingleObject( hEvent_ServiceStop, INFINITE ) ;
		::InterlockedIncrement(&m_lAppStopping);

		if( hEvent_ServiceStop )
		{
			CloseHandle(hEvent_ServiceStop) ;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartServiceWorkerThread", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : StopServiceByEvent()
*  Description    : Stop service
*  Author Name    : Vilas , Neha  
*  SR_NO		  : WRDWIZALUSRV_0017
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void StopServiceByEvent()
{
	if( hEvent_ServiceStop )
	{
		SetEvent( hEvent_ServiceStop ) ;
		::InterlockedIncrement(&m_lAppStopping);
	}

	if (m_pDownloadController != NULL)
	{
		delete m_pDownloadController;
		m_pDownloadController = NULL;
	}

	if (m_pDownloadController4UpdtMgr != NULL)
	{
		delete m_pDownloadController4UpdtMgr;
		m_pDownloadController4UpdtMgr = NULL;
	}
}

/***************************************************************************************************                    
*  Function Name  : InstallService()
*  Description    : Install service
*  Author Name    : Vilas , Neha            
*  SR_NO		  : WRDWIZALUSRV_0018
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD InstallService( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName)
{
	SC_HANDLE				hService	= NULL ;
    SC_HANDLE				hSCManager	= NULL ;
	SERVICE_DESCRIPTION		SrvDesc		= {L"Coordinates between other WARDWIZ applications for multiple operations."} ;

	TCHAR	szSrvPath[512] = {0} ;
	DWORD	dwRet = 0x00, dwLastError=0x00 ;

	DWORD	dwSrvStartType = 0xFFFFFFFF ;

	__try
	{
		if( !GetModuleFileName( NULL, szSrvPath, 511 ) )
		{
			dwRet = 0x01 ;
			__leave ;
		}

		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( !hSCManager )
		{
			AddLogEntry(L"OpenSCManager Failed, Service Name :%s", pszSrvDisplayName, 0, true, SECONDLEVEL);
			dwRet = 0x02 ;
			__leave ;
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
				__leave ;
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
				__leave ;
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
			__leave ;
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
			__leave ;
		}

		dwRet = 0x05 ;
	}
	__finally
	{

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

	}
	return dwRet ;

}

/***************************************************************************************************                    
*  Function Name  : UnInstallService()
*  Description    : UninstallService
*  Author Name    : Vilas , Neha 
*  SR_NO		  : WRDWIZALUSRV_0019
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
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
			__leave ;
		}

		schService = OpenService( schSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
		if( !schService )
		{
			dwRetValue = 0x02 ;
			__leave ;
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
				__leave ;
			}

			if( dwServiceStartType == SERVICE_DISABLED ||
				(dwServiceStartType == SERVICE_DEMAND_START) )
			{
				if( !ChangeServiceConfig(	schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
											SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
											NULL, NULL, NULL ) )
				{
					dwRetValue = 0x04 ;
					__leave ;
				}

				Sleep( 100 ) ;
				ControlService( schSCManager, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
				if( !DeleteService( schService ) )
				{
					dwRetValue = 0x05 ;
					__leave ;
				}
			}
		}

		//WriteLog( "DeleteService() done" ) ;
	}
	__finally
	{
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

	}

	return dwRetValue ;
}

/***************************************************************************************************                    
*  Function Name  : StartServiceManually()
*  Description    : Start Service Manually
*  Author Name    : Vilas , Neha   
*  SR_NO		  : WRDWIZALUSRV_0020
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD StartServiceManually( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName )
{
	DWORD		dwRet = 0x00 ;

	SC_HANDLE	schSCManager = NULL ;
	SC_HANDLE	schService = NULL ;

	DWORD		dwSrvStartType = 0xFFFFFFFF ;
	DWORD		dwLastError = 0x00 ;

	__try
	{
		AddLogEntry(L">>> Start ALU service", 0, 0, true, FIRSTLEVEL);
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

/***************************************************************************************************                    
*  Function Name  : StopServiceManually()
*  Description    : StopService Manually
*  Author Name    : Vilas , Neha    
*  SR_NO		  : WRDWIZALUSRV_0021
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
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
			__leave ;
		}

		schService = OpenService( schSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
		if( !schService )
		{
			dwRetValue = 0x02 ;
			__leave ;
		}

		Sleep( 100 ) ;

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
		if( (ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) ||
			(ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) )
		{
			Sleep( 1000 ) ;
			__leave ;
		}

		//WriteLog( "SERVICE_CONTROL_STOP called" ) ;

		if( QueryServiceStatus( schService, &dwServiceStartType ) )
		{
			dwRetValue = 0x03 ;
			__leave ;
		}

		if( dwServiceStartType == SERVICE_DISABLED ||
			(dwServiceStartType == SERVICE_DEMAND_START) )
		{
			if( !ChangeServiceConfig(	schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
										SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
										NULL, NULL, NULL ) )
			{
				dwRetValue = 0x04 ;
				__leave ;
			}

			Sleep( 100 ) ;
			if( !ControlService( schSCManager, SERVICE_CONTROL_STOP, &ServiceStatus ) )
			{
				dwRetValue = 0x05 ;
				__leave ;
			}
		}

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
		if( (ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) ||
			(ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) )
		{
			dwRetValue = 0x00 ;
			__leave ;
		}

		dwRetValue = 0x06 ;
	}
	__finally
	{
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

	}

	return dwRetValue ;
}

/***************************************************************************************************                    
*  Function Name  : QueryServiceStatus()
*  Description    : Query Service Status
*  Author Name    : Vilas , Neha   
*  SR_NO		  : WRDWIZALUSRV_0022
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
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
			__leave ;
		}

		QueryServiceConfig( hService, NULL, 0, &dwBytesNeeded ) ;
		if( !dwBytesNeeded )
		{
			dwStatus = 0x02 ;
			__leave ;
		}

		dwSize = dwBytesNeeded ;

		lpsc = (LPQUERY_SERVICE_CONFIG ) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT, dwSize ) ;
		if( !lpsc )
		{
			dwStatus = 0x03 ;
			__leave ;
		}

		if( !QueryServiceConfig(hService, lpsc, dwSize, &dwBytesNeeded ) )
		{
			dwStatus = 0x04 ;
			__leave ;
		}

		*lpdwServiceStatus = lpsc->dwStartType ;
	}
	__finally
	{
		if( lpsc )
		{
			LocalFree( lpsc ) ;
			lpsc = NULL ;
		}
	}

	return dwStatus ;
}

/***************************************************************************************************                    
*  Function Name  : QueryServiceStartStatus()
*  Description    : Query Service Start Status
*  Author Name    : Vilas , Neha  
*  SR_NO		  : WRDWIZALUSRV_0023
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
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
			__leave ;
		}

		if( !QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssStatus,
									sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
		{
			Sleep( 100 ) ;
			if( !QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE) &ssStatus,
									sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
			{
				dwStatus = 0x02 ;
				__leave ;
			}
		}

		*lpdwServiceStatus = ssStatus.dwCurrentState ;
	}
	__finally
	{
	}

	return dwStatus ;
}

/***************************************************************************************************                    
*  Function Name  : SetServiceFailureAction()
*  Description    : Set Service Failure Action
*  Author Name    : Vilas , Neha 
*  SR_NO		  : WRDWIZALUSRV_0024
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
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
			__leave ;
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
	__finally
	{
	}

	return dwRet ;
}

/***************************************************************************************************                    
*  Function Name    : GetProductID
*  Output parameter : dwProductID = return Client product ID
*  Description      : This treat will show the status and percentage from ALU service to UI
*  Author Name      : Vilas                                                         
*  SR_NO		  : WRDWIZALUSRV_0025
*  Date             : 4- Jul -2014 (Auto Live Update)
****************************************************************************************************/
bool GetProductID( DWORD &dwProductID)
{
	CString csIniFilePath = GetAVPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";

	if( !PathFileExists(csIniFilePath) )
		return GetDWORDValueFromRegistry(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwProductID", dwProductID);

	g_dwProductID = dwProductID = GetPrivateProfileInt(L"VBSETTINGS", L"ProductID", 0, csIniFilePath);

	if( !dwProductID)
		return true;

	return false;
}

/***************************************************************************************************                    
*  Function Name  : GetDWORDValueFromRegistry
*  Description    : It will give the dword value from registry.
*  Author Name    : Vilas                                           
*  SR_NO		  : WRDWIZALUSRV_0026
*  Date			  :	4- Jul -2014 (Auto Live Update)
****************************************************************************************************/
bool GetDWORDValueFromRegistry(HKEY hMain, LPTSTR lpszSubKey, LPTSTR lpszValuneName, DWORD &dwProductID)
{
	HKEY	hSubKey = NULL ;
	__try
	{
		if (RegOpenKeyEx(hMain, lpszSubKey, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
			return true;

		DWORD	dwSize = sizeof(DWORD) ;
		DWORD	dwType = 0x00 ;

		RegQueryValueEx(hSubKey, lpszValuneName, 0, &dwType, (LPBYTE)&dwProductID, &dwSize) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

		if( !dwProductID )
			return true;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetDWORDValueFromRegistry", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : SrvPrimaryThread
*  Description    : Run named pipe and create shared memory
*  Author Name    : Vilas , Neha                     
*  SR_NO		  : WRDWIZALUSRV_0027
*  Date			  :	4- Jul -2014 (Auto Live Update)
*  Modified Date  :	11-Jul -2014 (Neha Gharge)
****************************************************************************************************/
DWORD SrvPrimaryThread( LPVOID lpParam )
{
	bool	bSuccess = true;

	try
	{
		//Varada Ikhar, Date: 8th May-2015
		//New Implementation : To Show 'Release Information' after successful product update.
		DWORD   dwSuccess = 0x00;
		DWORD dwRelNoteShow = 0x00;

		CString WrdWizShellExtDll = L"";
		g_hUpdateFromUIEvent = NULL;
		g_hUpdateFromAgentEvent = NULL;
		g_bRequestFromUI = false;

		g_bUpdateFailed = false;
		g_bExtractionFailed = false;
		g_bRestartFlag = true;

		g_bUpToDate = false;

		m_lRetryCount = 0x00;
		g_lTotalDownloadedBytes = 0x00;
		m_lAppStopping = 0x00;
		m_pDownloadController = NULL;
		m_pDownloadController4UpdtMgr = NULL;

		//Varada Ikhar, Date:29/01/2015, Version:1.8.3.5,
		//Issue : Even if auto-live update is off, it shows "Product Updated Successfully" pop-up.
		g_bUpdateSuccess = false;
		g_bIsRelayClient = true;

		g_csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();
		
		DWORD	dwProductID = 0x00;
		GetProductID(dwProductID);
		if (dwProductID == ELITE)
		{
			//Read here registry entry for is relay client.
			DWORD dwISRelayClient = 0x00;
			if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwISRelayClient", dwISRelayClient) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwISRelayClient in VibraniumALUSrv, KeyPath: %s", g_csProdRegKey, 0, true, FIRSTLEVEL);
			}

			if (dwISRelayClient != 0x01)
			{
				g_bIsRelayClient = false;
			}
		}

		CWardWizOSversion		objOSVersionWrap;
		g_OSType = objOSVersionWrap.DetectClientOSVersion();

		g_hUpdateFromUIEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		g_hUpdateFromAgentEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		//Shifted this call @start of this function, as because sometimes commservice not getting this event while opening.
		HANDLE hUpdateOprCmplted = CreateEvent(NULL, TRUE, FALSE, WWUPDATECMPLTEVENT);
		ResetEvent(hUpdateOprCmplted);

		if (hUpdateOprCmplted != NULL)
		{
			AddLogEntry(L">>> Creat event of VibraniumUPDATECMPLTEVENT", 0, 0, true, FIRSTLEVEL);
		}
		else
		{
			AddLogEntry(L"### Failed to creat event of VibraniumUPDATECMPLTEVENT", 0, 0, true, FIRSTLEVEL);
		}

		g_objCommALUpdateServer.Run();

		g_objALupdateServ.CreateServerMemoryMappedFile();

		ZeroMemory(g_szAllUserPath, sizeof(g_szAllUserPath) );
		ZeroMemory(g_szAVPath, sizeof(g_szAVPath) );

		GetEnvironmentVariable(L"ALLUSERSPROFILE", g_szAllUserPath, 255 );

		GetAVPathFromRegistry(g_szAVPath);
		if( !PathFileExists(g_szAVPath) )
		{
			GetModuleFileName(NULL, g_szAVPath, 511);
			TCHAR	*pTemp = wcsrchr(g_szAVPath, '\\' );
			if( pTemp)
			{
				pTemp++;
				*pTemp = '\0';
			}
		}

		g_csAppFolderName = g_szAVPath;
		g_csAppFolderName = g_csAppFolderName.Left(g_csAppFolderName.GetLength() - 1);
		g_csAppFolderName = g_csAppFolderName.Right(g_csAppFolderName.GetLength() - ( g_csAppFolderName.ReverseFind(L'\\') + 1));

		PerformPostUpdateRegOperations();
		if (!SetAllALtFilesAttributeToNorml())
		{
			AddLogEntry(L"### Failed to SetAllALtFilesAttributeToNorml",0,0,true,FIRSTLEVEL);
		}

		WrdWizShellExtDll.Format(L"%s%s",g_szAVPath,L"VBSHELLEXT_OLD.DLL");
		if(!PathFileExists(WrdWizShellExtDll))
		{
			AddLogEntry(L"### VBSHELLEXT_OLD.DLL %s path file not exist.", WrdWizShellExtDll, 0, true, FIRSTLEVEL);
		}
		else
		{
			//Delete old file on restart of service Neha Gharge 20-3-2015.
			SetFileAttributes(WrdWizShellExtDll, FILE_ATTRIBUTE_NORMAL);
			if (!DeleteFile(WrdWizShellExtDll))
			{
				if (!MoveFileEx(WrdWizShellExtDll, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
				{
					AddLogEntry(L"### VBSHELLEXT_OLD.DLL %s file is not deleted.", WrdWizShellExtDll, 0, true, FIRSTLEVEL);
				}
			}
			else
			{
				AddLogEntry(L">>> VBSHELLEXT_OLD.DLL %s file is deleted successfully.", WrdWizShellExtDll, 0, true, FIRSTLEVEL);
			}
		}

		GetProductID( dwProductID );
		TCHAR	szIniFilePath[512] = { 0 };

		if(!dwProductID)
		{
			AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
			return true;
		}

		switch( dwProductID )
		{
			case 1:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 2:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 3:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 4:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 5:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", g_szAllUserPath, g_csAppFolderName);
				break;
		}
		if (PathFileExists(szIniFilePath))
		{
			SetFileAttributes(szIniFilePath, FILE_ATTRIBUTE_NORMAL);
			if (!DeleteFile(szIniFilePath))
			{
				AddLogEntry(L"### Failed to delete previous ini file %s", szIniFilePath, 0, true, FIRSTLEVEL);
				if (!MoveFileEx(szIniFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
				{
					AddLogEntry(L"### Failed to MoveFileEx previous ini file %s", szIniFilePath, 0, true, FIRSTLEVEL);
				}
			}
			else
			{
				AddLogEntry(L">>> Successful to delete previous ini file %s", szIniFilePath, 0, true, FIRSTLEVEL);
			}
		}
		else
		{
			AddLogEntry(L"### Path of previous ini file %s is not exist", szIniFilePath, 0, true, FIRSTLEVEL);
		}
		AddServiceNameIntoVector();

		//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
		//Resolved By :  Nitin K.
		g_bIsALUDeleted = FALSE;

		if (!HandleDriverReplaceCaseOnWin10())
		{
			AddLogEntry(L"### Failed to HandleDriverReplaceCaseOnWin10", 0, 0, true, FIRSTLEVEL);
		}

		DeleteAllRenamedFiles();

		DeleteAllALtFilesFromProgramData();

		//1.9.4.0 version the file which are download first time not getting renamed. In 1.9.0.0 we create a file then copy
		//But in 1.9.4.0 its just download file and replaced but failed to rename.
		if (!RenameFileFailedIn1940Version())
		{
			AddLogEntry(L"### Failed to RenameFileFailedIn1940Version", 0, 0, true, FIRSTLEVEL);
		}

		ReadValueFromDLLregisterSection();
		
		//Update Process completes here, give signal to COMM service proceed.
		SetEvent(hUpdateOprCmplted);

		if (!hThread_WaitToStartTray)
		{
			hThread_WaitToStartTray = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WaitToStartTrayThread,
				NULL, 0, 0);
			Sleep(500);

			if (hThread_WaitToStartTray == NULL)
				AddLogEntry(L"### Failed in SrvPrimaryThread::To create WaitToStartTrayThread", 0, 0, true, SECONDLEVEL);

		}

		g_bRestartFlag = false;

		if( !hThread_AutoUpdateAfter3Hrs )
		{
			hThread_AutoUpdateAfter3Hrs = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) StartALUpdateAfter3Hrs, 
														NULL, 0, 0 ) ;
			Sleep( 500 ) ;

			if(hThread_AutoUpdateAfter3Hrs == NULL)
				AddLogEntry(L"### Failed in SrvPrimaryThread::To create ALUpdateProcessThread", 0, 0, true, SECONDLEVEL);
		}

		//checked here registry IS_UPDATE_MANAGER
		CITinRegWrapper objReg;
		DWORD dwUpdateManager = 0;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwUpdtManager", dwUpdateManager) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwUpdtManager", 0, 0, true, ZEROLEVEL);;
			return 0;
		}


	}
	catch(...)
	{
		AddLogEntry(L"### Exception in SrvPrimaryThread", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

/***************************************************************************************************                    
*  Function Name  : StartALUpdateProcessThread()
*  Description    : This threat will download ini.,Fills struct with modified binaries, 
					Download that binaries and copy into its respective location
*  Author Name    : Vilas , Neha   
*  SR_NO		  : WRDWIZALUSRV_0032
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD StartALUpdateProcessThread( LPVOID lpParam )
{
	g_dwIsUptoDateCompleted = ALUPDATEFAILED_INTERNETCONNECTION;
	DWORD dwStatus = 0x00;
	DWORD	dwALUpdateStatus = ALUPDATEDSUCCESSFULLY;

	bool bSuccess = true;
	try
	{
		//Unlock the critical section object as this is start call.
		if (g_OSType != WINOS_XP && g_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Unlock();
		}

		//Issue resolved :0000178 Issue with Tray Notification.
		if (g_bEnableAutoliveUpdate == false && g_bRequestFromUI == true)
		{
			//Send to Tray MEssage Update Now PopUp
			//Issue Resolved: We dont need to wait after send Message to Tray, so setting bWait to false
			//Resolved By : Nitin K. 22 April 2015
			if (!SendMessage2UI(CLOSE_UPDATENOW_POPUP, 2, 0, 0, false))//2 is for Tray server 
			{
				AddLogEntry(L"### Failed to get response from tray server", 0, 0, true, SECONDLEVEL);
			}

			//it is blocking call
			//ClearMemoryMapObjects();
			//ResetEvent(g_hUpdateFromUIEvent);
			//return 0;
		}


		/* Issue NO - 159
		Failed to update product. Please check internet connection” 
		even if internet connection is there
		Neha G.
		10/sept/2014
		*/

		for (int iRetryCount = 0x00; iRetryCount < 0x02; iRetryCount++)
		{
			bSuccess = CheckInternetConnection();
			if (bSuccess)
			{
				break;
			}
		}

		if(!bSuccess)//if internet problem persists, it returns false only for this case
		{

			/*	if(!SendUpdateInfoToGUI(SETUPDATESTATUS, L"", L"", 4, 0))
			{
				AddLogEntry(L"### SendUpdateInfoToGUI First Param: 1", 0, 0, true, SECONDLEVEL);
			}
		*/

			dwALUpdateStatus = ALUPDATEFAILED_INTERNETCONNECTION;
			AddLogEntry(L"### Failed in StartALUpdateProcessThread::No internet connection", 0, 0, true, SECONDLEVEL);

			goto Cleanup;
		}


		TCHAR	szUpdateServerCount[10] = { 0 };
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\UpdateServers.ini";
		GetPrivateProfileString(L"UPDATESERVERS", L"Count", L"0", szUpdateServerCount, 511, csIniFilePath);
		CString csUpdateServerCount = (LPCTSTR)szUpdateServerCount;
		m_iServerCount = _ttoi(csUpdateServerCount);

		SendUpdateInfoToGUI(SETMESSAGE, L"Checking for updates", L"", 0, 0);
		DWORD dwRetryCount = 0;
		if ( m_iServerCount)
		{
			for (int i = 0; i < m_iServerCount; i++)
			{
				dwRetryCount = 0;
				CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\UpdateServers.ini";
				CString csCurrVal;
				TCHAR	szUpdateServerName[255] = { 0 };
				csCurrVal.Format(L"%d", i + 1);
				GetPrivateProfileString(L"UPDATESERVERS", csCurrVal, L"0", szUpdateServerName, 511, csIniFilePath);
				g_csServerName = szUpdateServerName;
				
				if (bSuccess)
				{
					break;
				}
			}
		}
		
		if( !bSuccess )
		{
			dwALUpdateStatus = ALUPDATEFAILED_DOWNLOADINIPARSE;
			AddLogEntry(L"### Failed in StartALUpdateProcessThread::DownLoadIniFileAndParse", 0, 0, true, SECONDLEVEL);
			//if(!g_bRequestFromUI && g_bEnableAutoliveUpdate == true)
			//{
				g_bUpdateFailed = true;
			//}
			goto Cleanup;
		}

		AddLogEntry(L">>> succeed in StartALUpdateProcessThread::DownLoadIniFileAndParse", 0, 0, true, ZEROLEVEL);

		if( !vALUFileInfo.size() )
		{
			AddLogEntry(L">>> StartALUpdateProcessThread::Product is upto date!!!", 0, 0, true, ZEROLEVEL);

			dwALUpdateStatus = ALUPDATED_UPTODATE;
			g_dwIsUptoDateCompleted = dwALUpdateStatus;
			//update date and time only after successful updates.
			if(!UpdateTimeDate())
			{
				AddLogEntry(L"### Failed to write update date into registry",0,0,true,SECONDLEVEL);
			}

			//Pass this message to GUI
			//SendUpdateInfoToGUI(SETUPDATESTATUS,L"",L"",3,0);

			//dwALUpdateStatus = ALUPDATED_UPTODATE;

			if(!g_bRequestFromUI && g_bEnableAutoliveUpdate == true)
			{
				// send message to tray ui that it is up - to - date
				g_bUpToDate = true;
			}
			goto Cleanup;
		}

		if(g_bEnableAutoliveUpdate == false && g_bRequestFromUI == false)
		{
			//Send to Tray MEssage Update Now PopUp
			if(!SendMessage2UI(UPDATENOW_POPUP , 2 , 0, 0, true))//2 is for Tray server 
			{
				AddLogEntry(L"### Failed to get response from tray server", 0, 0, true, SECONDLEVEL);
			}

			//it is blocking call
			ClearMemoryMapObjects();
			ResetEvent(g_hUpdateFromUIEvent);
			ResetEvent(g_hUpdateFromAgentEvent);
			return 0;
		}

		AddLogEntry(L">>> susceed in StartALUpdateProcessThread::Product is not up to date!!!", 0, 0, true, ZEROLEVEL);
		
		if( dwStatus == 0x01 )
		{
			if( g_bRequestFromUI )
			{
			/*	if(!SendUpdateInfoToGUI(SETUPDATESTATUS, L"", L"", 1, 0))
				{
					AddLogEntry(L"### Failed in SendUpdateInfoToGUI First Param: 1", 0, 0, true, SECONDLEVEL);
				}
			*/
				dwALUpdateStatus = ALUPDATEDSUCCESSFULLY;
				AddLogEntry(L"### Failed in StartALUpdateProcessThread::Up-to-date", 0, 0, true, SECONDLEVEL);
			}
			goto Cleanup;
		}
			
		AddLogEntry(L">>> suscced in StartALUpdateProcessThread::DownLoadLatestFilesFromServer", 0, 0, true, ZEROLEVEL);

		SendUpdateInfoToGUI(SETMESSAGE, L"Updating files", L"", 0, 0);

		AddLogEntry(L">>> suscced in StartALUpdateProcessThread::ExtractAllDownLoadedFiles", 0, 0, true, ZEROLEVEL);
		//Issue : 0000449 Issue with size mentioned in downloaded column. : Once download was successful it was showing Downloaded 100%(0 kb)
		//Resolved by Nitin K.

		DWORD dwPercent = g_dwPercentage;
		if (g_iPreviousPerc >= (int)(g_dwPercentage + 1))
		{
			dwPercent = g_iPreviousPerc;
		}

		if (dwPercent >= 100)
		{
			dwPercent = 98;
		}

		if (!SendUpdateInfoToGUI(SETDOWNLOADPERCENTAGE, L"", L"", g_dwTotalFileSize, (dwPercent + 1)))
		{
			AddLogEntry(L"### Failed in SendUpdateInfoToGUI SETDOWNLOADPERCENTAGE", 0, 0, true, SECONDLEVEL);
		}
		
		bSuccess = ReplaceDownloadedFiles();

		if (bSuccess)
		{
			dwALUpdateStatus = ALUPDATEFAILED_UPDATINGFILE;
			AddLogEntry(L"### Failed in StartALUpdateProcessThread::ReplaceDownloadedFiles", 0, 0, true, SECONDLEVEL);
			if (!g_bRequestFromUI)
			{
				g_bUpdateFailed = true;
			}
			goto Cleanup;
		}

		//update date and time only after successful updates.
		if(!UpdateTimeDate())
		{
			AddLogEntry(L"### Failed to write update date into registry",0,0,true,SECONDLEVEL);
		}

		AddLogEntry(L">>> Live-Updated successfully", 0, 0, true, SECONDLEVEL);

		//Varada Ikhar, Date:29/01/2015, Version:1.8.3.5,
		//Issue : Even if auto-live update is off, it shows "Product Updated Successfully" pop-up.
		g_bUpdateSuccess = true;

		//Issue : 0000449 Issue with size mentioned in downloaded column. : Once download was successful it was showing Downloaded 100%(0 kb)
		//Resolved by Nitin K.
		SendUpdateInfoToGUI(SETDOWNLOADPERCENTAGE, L"", L"", g_dwTotalFileSize, 100);
		
		//Commented out to send update status to GUI at last
		//Commented by Vilas on 04 May 2015
		//SendUpdateInfoToGUI(SETUPDATESTATUS, L"", L"", 1, 0);

		g_dwIsUptoDateCompleted  = dwALUpdateStatus = ALUPDATEDSUCCESSFULLY;

		PerformPostUpdateRegOperations();

		ReadValueFromDLLregisterSection();

		//Do not wait till database load when update is finished.
		if (SendData2CommService(RELOAD_SIGNATURE_DATABASE, false) != 0x00)
		{
			AddLogEntry(L"### Failed to send reload Database message to service", 0, 0, true, SECONDLEVEL);
		}

		if (!SendData2CommService(CLEAR_INDEXING, false))
		{
			AddLogEntry(L"### Failed to SendData2Service for CLEAR_INDEXING", 0, 0, true, SECONDLEVEL);
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in StartALUpdateProcessThread", 0, 0, true, SECONDLEVEL);
	}

Cleanup:
	g_dwIsUptoDateCompleted = dwALUpdateStatus;

	////Added by Vilas on 04 May 2015
	if (g_bRequestFromUI)
		SendUpdateInfoToGUI(SETUPDATESTATUS, L"", L"", dwALUpdateStatus, 0);

	if( hThread_StartALUpdateProcess )
		StopALUpdateProcessThread( );

	//remove proxy setting data
	g_objWinHttpManager.m_dwProxySett = 0X00;
	if (g_objWinHttpManager.m_csServer.GetLength() > 0)
	{
		g_objWinHttpManager.m_csServer.Empty();
	}

	//If update is failed and we send message to use to retry, 
	//but some of the handles still open so we need to focefull exit the process, 
	//so that next retry update can go in success.
	if (g_bExtractionFailed)
	{
		exit(0);
	}
	return 0;
}


/***************************************************************************************************                    
*  Function Name  : DeleteFileForcefully()
*  Description    : Delete files forcefully from temp folder after extracting 
*  Author Name    : Vilas  
*  SR_NO		  : WRDWIZALUSRV_0036
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool DeleteFileForcefully(LPTSTR lpszFilePath )
{
	try
	{
		if( !PathFileExists(lpszFilePath) )
			return false;

		SetFileAttributes(lpszFilePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile( lpszFilePath );

		if( !PathFileExists(lpszFilePath) )
			return false;

		TCHAR	szRenamePath[512] = {0} ;

		//swprintf_s(szRenamePath, _countof(szRenamePath), L"%s.%lu", g_szAllUserPath, GetTickCount() );

		//Modified for deleting existing file
		//Added by Vilas on 29 April 2015
		swprintf_s(szRenamePath, _countof(szRenamePath), L"%s.%lu", lpszFilePath, GetTickCount());
		
		SetFileAttributes(szRenamePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile( szRenamePath );
		_wrename(lpszFilePath, szRenamePath );
	}
	catch( ... )
	{
		AddLogEntry(L"### VibraniumAluSrv::DeleteFileForcefully::Exception",0,0,true,SECONDLEVEL);
	}

	return false;
}

/***************************************************************************************************
*  Function Name  : ReplaceDownloadedFiles()
*  Description    : Downloaded files get extracted get copy into production folder. &
If all files cpoied successfully, old files get deleted and new files get
replaced. If some files get failed to copy, all files get rolled back
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0037
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool ReplaceDownloadedFiles()
{
	CString	szFilePath;

	DWORD	dwCount = 0x00, i = 0x00;

	TCHAR	szSourceFilePath[512] = { 0 };
	TCHAR	szDestFilePath[512] = { 0 };
	TCHAR	szTemp[512] = { 0 };
	TCHAR	szReplaceDestination[512] = { 0 };

	bool	bCopied = true;
	int iStartPos = 0;
	TCHAR szAVPath[512] = { 0 };
	try
	{
		AddLogEntry(L">>> In ReplaceDownloadedFiles function", 0, 0, true, ZEROLEVEL);
		dwCount = static_cast<DWORD>(vALUFileInfo.size());

		for (; i<dwCount; i++)
		{
			ZeroMemory(szSourceFilePath, sizeof(szSourceFilePath));
			ZeroMemory(szDestFilePath, sizeof(szDestFilePath));
			SendUpdateInfoToGUI(SETMESSAGE, L"Updating files", L"", (i + 1), dwCount);
			swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s\\%s\\%s_%s", g_szAllUserPath, g_csAppFolderName, vALUFileInfo[i].szFileName, L"WWIZALT");
			AddLogEntry(L">>> SourcePath :: %s", szSourceFilePath, 0, true, ZEROLEVEL);

			//This code to replace the file in there respective folder.
			CString csReplaceDestination = L"";
			csReplaceDestination = vALUFileInfo[i].szFileShortPath;
			_tcsnccpy(szAVPath, g_szAVPath, (_tcslen(g_szAVPath) - 1));
			csReplaceDestination.Replace(L"AppFolder", szAVPath);
			iStartPos = 0;
			swprintf_s(szDestFilePath, _countof(szDestFilePath), L"%s\\%s", csReplaceDestination, vALUFileInfo[i].szFileName);
			AddLogEntry(L">>> Destination Path :: %s", szDestFilePath, 0, true, ZEROLEVEL);


			//GetFilePath and Create sub Directory inside the AppFolder, if required
			if (CheckDestFolderAndCreate(vALUFileInfo[i].szFileShortPath))
			{
				AddLogEntry(L"### Failed to create & Check dest folder path :: %s", vALUFileInfo[i].szFileShortPath, 0, true, SECONDLEVEL);
				break;
			}

			if (!PathFileExists(szSourceFilePath))
			{
				AddLogEntry(L"### Source path file is not existing :: %s", szSourceFilePath, 0, true, SECONDLEVEL);
				break;
			}

			bCopied = true;
			//Retry count three time to replace files.
			for (BYTE b = 0x00; b < MAX_UPDATE_RETRYCOUNT; b++)
			{

				if (PathFileExists(szDestFilePath))
				{
					if (CheckForServiceName(i))
					{
						AddLogEntry(L">>> Destination path file is existing :: %s", szDestFilePath, 0, true, ZEROLEVEL);
						swprintf_s(szTemp, _countof(szTemp), L"%s\\%s_%s", csReplaceDestination, vALUFileInfo[i].szFileName, L"WWIZALTSRV");

						SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
						if (DeleteFile(szTemp) == FALSE)
						{
							AddLogEntry(L">>> DeleteFile Failed :: %s", szTemp, 0, true, ZEROLEVEL);
						}
						if (_wrename(szDestFilePath, szTemp) != 0)
						{
							AddLogEntry(L">>> _wrename Failed :: %s", szTemp, 0, true, ZEROLEVEL);
						}
						swprintf_s(szTemp, _countof(szTemp), L"%s\\%s", csReplaceDestination, vALUFileInfo[i].szFileName);
					}
					else
					{
						AddLogEntry(L">>> Destination path file is existing :: %s", szDestFilePath, 0, true, ZEROLEVEL);
						swprintf_s(szTemp, _countof(szTemp), L"%s\\%s_%s", csReplaceDestination, vALUFileInfo[i].szFileName, L"WWIZALT");
						SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
						if (DeleteFile(szTemp) == FALSE)
						{
							AddLogEntry(L">>> DeleteFile Failed :: %s", szTemp, 0, true, ZEROLEVEL);
						}
					}
				}
				else
				{
					AddLogEntry(L">>> Destination path file is not existing :: %s", szDestFilePath, 0, true, ZEROLEVEL);
					swprintf_s(szTemp, _countof(szTemp), L"%s\\%s_%s", csReplaceDestination, vALUFileInfo[i].szFileName, L"WWIZALT");
					SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
					if (DeleteFile(szTemp) == FALSE)
					{
						AddLogEntry(L">>> DeleteFile Failed :: %s", szTemp, 0, true, ZEROLEVEL);
					}
				}

				if (CopyFile(szSourceFilePath, szTemp, FALSE))
				{
					bCopied = false;
					g_dwReplacedCount++;
					int iReplacecount = g_dwReplacedCount;
					int iTotalFileCount = dwCount;
					DWORD dwPercentage = static_cast<DWORD>(((static_cast<double>((iReplacecount)) / iTotalFileCount)) * 19);
					if ((DWORD)g_iPreviousPerc < g_dwPercentage + dwPercentage)
					{
						DWORD dwPercent = g_dwPercentage;
						if (g_iPreviousPerc >= (int)(g_dwPercentage + dwPercentage + 1))
						{
							dwPercent = g_iPreviousPerc;
						}
						if ((dwPercent + dwPercentage + 1) <= 99)
						{
							SendUpdateInfoToGUI(SETDOWNLOADPERCENTAGE, L"", L"", g_dwTotalFileSize, (dwPercent + dwPercentage + 1));
						}
					}
					AddEntryToALUDelIni(szTemp, vALUFileInfo[i].szFileName);
					_tcscpy(szTemp, L"");
					break;
				}
				Sleep(10);

			}

			if (bCopied)
			{
				AddLogEntry(L"### Failed to copy %s file to %s file", szSourceFilePath, szTemp, true, SECONDLEVEL);
				//RollBack all Files
				break;
			}

		}

		if (g_dwReplacedCount != dwCount)
		{
			//Roll back all renamed files
			AddLogEntry(L">>> In rollback function", 0, 0, true, ZEROLEVEL);
			ReplaceAllOriginalFiles();
			return true;
		}
		else
		{
			//Delete All renamed files
			AddLogEntry(L">>> In replace and delete downloaded file function", 0, 0, true, ZEROLEVEL);
			DeleteAllRenamedFiles();
			DeleteAllDownloadedFiles();
			return false;
		}
	}
	catch (...)
	{
		bCopied = false;
		AddLogEntry(L"### VibraniumALUSrv::ReplaceDownloadedFiles::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bCopied;
}

/***************************************************************************************************                    
*  Function Name  : DeleteAllRenamedFiles()
*  Description    : If all files cpoied successfully, old files get deleted and new files get 
					replaced. All renamed file get replaced with original name
*  Author Name    : Vilas , Neha 
*  SR_NO		  : WRDWIZALUSRV_0038
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool DeleteAllRenamedFilesSEH()
{
	DWORD i=0x00, dwCount=0x00;

	TCHAR	szIniFilePath[512] = {0};
	TCHAR	szValueData[512] = {0};
	TCHAR	szValueName[256] = {0};
	TCHAR	szFailedReplacedFile[512] = {0};
	TCHAR   szTemp[512] ={0};
	
	std::vector<CString	>vFailedtoReplacedFiles;

	swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", g_szAllUserPath, g_csAppFolderName);
		

	if( !PathFileExists(szIniFilePath) )
	{
		AddLogEntry(L"### File not found : %s in DeleteAllRenamedFiles ", szIniFilePath, 0, true, ZEROLEVEL);
		//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
		//Resolved By :  Nitin K.
		g_bIsALUDeleted = TRUE ;
	}

	dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath );

	CString csErrorDeleteAllRenamedFiles;
	csErrorDeleteAllRenamedFiles.Format(L"%d",dwCount);
	AddLogEntry(L"### In Function DeleteAllRenamedFiles (%s)", csErrorDeleteAllRenamedFiles , 0, true, 0x00);

	if(g_bRestartFlag)
	{
		ZeroMemory(szValueName, sizeof(szValueName) );
		WritePrivateProfileString(L"DB", NULL, NULL, szIniFilePath );
		g_bRestartFlag = false;
	}

	if( dwCount )
	{
		for (int i = 1; i <= static_cast<int>(dwCount); i++)
		{
			swprintf_s(szValueName, _countof(szValueName), L"%lu", i );
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, 511, szIniFilePath);
			if( !szValueData[0] )
			{
				//AddLogEntry(L"### Function is  DeleteAllRenamedFiles....WardWizParseIniFile::Invalid Entries for(%s) in (%s)", szValueName, szIniFilePath, true, 0x00);
				break;
			}
			else
			{
				vReplacedFiles.push_back(szValueData);
			}
		}
	}
	dwCount =0x00;
	vFailedtoReplacedFiles.clear();

	CString csNewFileNamePath,csOldFileNamePath,csServiceName;
	int iPos = 0;
	for (int i = 0; i < static_cast<int>(vReplacedFiles.size()); i++)
	{
		csNewFileNamePath = vReplacedFiles.at(i);
		_tcscpy(szTemp,csNewFileNamePath);
		if( CheckForService(szTemp, csServiceName ) )
		{
			CString csFilePath = szTemp;
			csFilePath = csFilePath.Left(csFilePath.ReverseFind(L'\\') + 1);
			
			swprintf_s(szTemp, _countof(szTemp), L"%s%s_%s", csFilePath, csServiceName, L"WWIZALTSRV");
			csOldFileNamePath.Format(L"%s",szTemp);
			if(PathFileExists(csOldFileNamePath))
			{
				SetFileAttributes(csOldFileNamePath, FILE_ATTRIBUTE_NORMAL);
				if(!DeleteFile(csOldFileNamePath))
				{
					AddLogEntry(L"### Failed to delete %s", csOldFileNamePath, 0, true, SECONDLEVEL);
					vFailedtoReplacedFiles.push_back(szTemp);
					++dwCount;
					continue;
				}
				else
				{
					continue;
				}
			}
			{
				swprintf_s(szTemp, _countof(szTemp), L"%s%s_%s", csFilePath, csServiceName, L"WWIZALTDEL");
				csOldFileNamePath.Format(L"%s", szTemp);
				if (PathFileExists(csOldFileNamePath))
				{
					SetFileAttributes(csOldFileNamePath, FILE_ATTRIBUTE_NORMAL);
					if (!DeleteFile(csOldFileNamePath))
					{
						AddLogEntry(L"### Failed to delete %s", csOldFileNamePath, 0, true, SECONDLEVEL);
						vFailedtoReplacedFiles.push_back(szTemp);
						++dwCount;
						continue;
					}
					else
					{
						continue;
					}
				}
			}
		}
		else
		{
			iPos = 0;
			csOldFileNamePath = csNewFileNamePath.Left(csNewFileNamePath.ReverseFind(L'_'));
		}

		iPos = 0;
		if(!PathFileExists(csNewFileNamePath))
		{
			AddLogEntry(L">>> new file %s exist", csNewFileNamePath, 0, true, ZEROLEVEL);
			break;
		}

		if(PathFileExists(csOldFileNamePath))
		{
			AddLogEntry(L">>> old file %s exist", csOldFileNamePath, 0, true, ZEROLEVEL);
			SetFileAttributes(csOldFileNamePath, FILE_ATTRIBUTE_NORMAL);
			if(DeleteFile(csOldFileNamePath))
			{
				_wrename(csNewFileNamePath ,csOldFileNamePath);
				continue;
			}
			Sleep(10);

			SetFileAttributes(csOldFileNamePath, FILE_ATTRIBUTE_NORMAL);
			if(DeleteFile(csOldFileNamePath))
			{
				_wrename(csNewFileNamePath ,csOldFileNamePath);
				continue;
			}
			vFailedtoReplacedFiles.push_back(csNewFileNamePath);
			++dwCount;
		}
		else
		{
			//In the case when any file is not exist in old setup and we are adding in new update
			//at that time is never get replaced.
			//1.9.0.0 setup we always create the file but in 1.9.4.0 onward we not create file even not get replaced.
			AddLogEntry(L">>> old file %s file is not available.", csOldFileNamePath, 0, true, ZEROLEVEL);
			_wrename(csNewFileNamePath, csOldFileNamePath);
			continue;
		}
	}

	//Neha Gharge 18-2-2015 if only DB file get downloaded.and dwcount == 0. file should not get delete when only DB entry is thr.
	DWORD	dwDBCount = 0x00;
	dwDBCount = GetPrivateProfileInt(L"DB", L"Count", 0, szIniFilePath);
	
	if( dwCount )
	{
		WritePrivateProfileString(L"Files", NULL, NULL, szIniFilePath );
		WritePrivateProfileString(L"Count", NULL, NULL, szIniFilePath );

		for (int index = 0; index < static_cast<int>(vFailedtoReplacedFiles.size()); index++)
		{
			swprintf_s(szFailedReplacedFile, _countof(szFailedReplacedFile), L"%s", vFailedtoReplacedFiles.at(index));
			AddEntryToALUDelIni(szFailedReplacedFile);
			_tcscpy(szFailedReplacedFile,L"");
		}
	}
	else
	{
		if(!dwDBCount)
		{
			if(PathFileExists(szIniFilePath))
			{
				SetFileAttributes(szIniFilePath, FILE_ATTRIBUTE_NORMAL);
				if(!DeleteFile(szIniFilePath))
				{
					AddLogEntry(L"### Failed to delete file: %s  ",szIniFilePath, 0,true, SECONDLEVEL);
				}
			}
		}
		else
		{
			//Neha Gharge 18-2-2015 if only DB file get downloaded.and dwcount == 0. file should not get delete when only DB entry is thr.
			//But files and count section should be delete as no entry is thr for replacement.
			WritePrivateProfileString(L"Files", NULL, NULL, szIniFilePath );
			WritePrivateProfileString(L"Count", NULL, NULL, szIniFilePath );
		}
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : DeleteAllRenamedFiles()
*  Description    : Call DeleteAllRenamedFilesSEH
*  Author Name    : Vilas
**  SR_NO		  : WRDWIZALUSRV_0039
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool DeleteAllRenamedFiles()
{
	bool bReturn = false;

	__try
	{
		bReturn = DeleteAllRenamedFilesSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### VibraniumAluSrv::DeleteAllRenamedFiles::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : GetTotalUpdateFilesCount()
*  Description    : Get Total Update Files Count        
*  Author Name    : Vilas       
**  SR_NO		  : WRDWIZALUSRV_0039
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
DWORD GetTotalUpdateFilesCount(bool bSendGUI )
{

	DWORD dwALUFilesCount = 0x00;

	try
	{
	 
		dwALUFilesCount = static_cast<DWORD>(vALUFileInfo.size());

		if( bSendGUI )
		{
			//Send total Files Count to GUI
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### VibraniumALUSrv::GetTotalUpdateFilesCount::Exception",0,0,true,SECONDLEVEL);
	}

	return dwALUFilesCount;
}

/***************************************************************************************************                    
*  Function Name  : StopALUpdateProcessThread()
*  Description    : Stops ALUpdate Process Thread         
*  Author Name    : Vilas 
**  SR_NO		  : WRDWIZALUSRV_0040
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool  StopALUpdateProcessThread( )
{
	try
	{
		g_objWinHttpManager.StopCurrentDownload();
		g_objWinHttpManager.SetDownloadCompletedBytes(0);
		g_objWinHttpManager.CloseFileHandles();
	
		if (m_pDownloadController != NULL)
		{
			m_pDownloadController->SetThreadPoolStatus(false);
			m_pDownloadController->CancelDownload();

			//Wait for some time
			Sleep(1000);

			m_pDownloadController->StopController();
		}

		if (m_pDownloadController != NULL)
		{
			delete m_pDownloadController;
			m_pDownloadController = NULL;
		}
		
		TCHAR	szIniFilePath[512] = { 0 };
		if (g_hFile != NULL)
		{
			CloseHandle(g_hFile);
			g_hFile = NULL;
		}
		if (g_hIniFile != NULL)
		{
			CloseHandle(g_hIniFile);
			g_hIniFile = NULL;
		}

		if (g_hTargetFile != NULL)
		{
			CloseHandle(g_hTargetFile);
			g_hTargetFile = NULL;
		}

		CloseTrasferDataToUIThread();
		//issue solved:4469. VibroPatchTS.ini & WWizPatchB.ini would not be deleted if ALUDel.ini file exist.so product version will reflect after restarting machine.
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", g_szAllUserPath, g_csAppFolderName);
		

		if (!g_bRequestFromUI)
		{
			if (g_bUpdateFailed)
			{
				Sleep(100);
				SendMessage2UI(UPDATE_FINISHED, 2, 2);
				g_bUpdateFailed = false;
			}
			else if (g_bUpToDate)
			{
				Sleep(100);
				SendMessage2UI(UPDATE_FINISHED, 2, 3);
				g_bUpToDate = false;

			}
			else if (g_bUpdateSuccess) //condition is checked.
			{
				Sleep(100);

				SendMessage2UI(UPDATE_FINISHED, 2, 1, g_bIsAnyProductChanges ? 1 : 0);

				if (!WriteIntoRegistry())
				{
					AddLogEntry(L"### Failed to write app version no and database", 0, 0, true, SECONDLEVEL);
				}
			}
			g_bRequestFromUI = false;
		}

		ClearMemoryMapObjects();
		ResetEvent(g_hUpdateFromUIEvent);
		ResetEvent(g_hUpdateFromAgentEvent);
		Sleep(10);
	}
	catch (...)
	{
		AddLogEntry(L"### VibraniumALUSrv::StopALUpdateProcessThread::Exception", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : StopALUpdateProcessThread4UpdtMgr()
*  Description    : Stops ALUpdate Process Thread
*  Author Name    : Amol Jaware
*  SR_NO		  : 
*  Date           :	17-April-2018
****************************************************************************************************/
bool  StopALUpdateProcessThread4UpdtMgr()
{
	try
	{
		g_objWinHttpManagerUpDM.StopCurrentDownload();
		g_objWinHttpManagerUpDM.SetDownloadCompletedBytes(0);
		g_objWinHttpManagerUpDM.CloseFileHandles();

		if (m_pDownloadController4UpdtMgr != NULL)
		{
			m_pDownloadController4UpdtMgr->SetThreadPoolStatus(false);
			m_pDownloadController4UpdtMgr->CancelDownload();

			//Wait for some time
			Sleep(1000);

			m_pDownloadController4UpdtMgr->StopController();
		}

		if (m_pDownloadController4UpdtMgr != NULL)
		{
			delete m_pDownloadController4UpdtMgr;
			m_pDownloadController4UpdtMgr = NULL;
		}

		TCHAR	szIniFilePath[512] = { 0 };
		if (g_hFile != NULL)
		{
			CloseHandle(g_hFile);
			g_hFile = NULL;
		}
		if (g_hIniFile != NULL)
		{
			CloseHandle(g_hIniFile);
			g_hIniFile = NULL;
		}

		if (g_hTargetFile != NULL)
		{
			CloseHandle(g_hTargetFile);
			g_hTargetFile = NULL;
		}

		ResetEvent(g_hUpdateFromAgentEvent);
		Sleep(10);
	}
	catch (...)
	{
		AddLogEntry(L"### VibraniumALUSrv::StopALUpdateProcessThread4UpdtMgr::Exception", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : CheckDestFolderAndCreate()
*  Description    : It will check all prodution folder. If not present, it will create that folder.
*  Author Name    : Vilas         
*  SR_NO		  : WRDWIZALUSRV_0043
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CheckDestFolderAndCreate( LPTSTR lpszShortPath )
{

	bool bCreate = false;

	TCHAR	*pTemp = NULL;

	try
	{
	
		pTemp = wcschr(lpszShortPath, '\\');

		if( !pTemp )
			return bCreate;

		TCHAR	szTemp[256] = {0};

		pTemp++;
		wcscpy_s(szTemp, 255, pTemp );

		TCHAR	szTokens[128] = {0};
		TCHAR	szPath[512] = {0};

		TCHAR	szToken[] = L"\\";
		TCHAR	*pToken = NULL;
		TCHAR	*pTokenNext = NULL;

		pToken = wcstok_s(szTemp, szToken, &pTokenNext);

		do
		{

			if( !pToken )
				break;

			if( !wcslen(pToken) )
				break;

			wcscat_s(szTokens, 128, pToken);
			swprintf_s(szPath, 511, L"%s%s", g_szAVPath, szTokens );

			if( !PathFileExists(szPath) )
			{
				bCreate = CreateDirectoryFocefully( szPath );
				if( bCreate )
					break;
			}

			wcscat_s(szTokens, 128, L"\\");

			pToken = wcstok_s(NULL, szToken, &pTokenNext);

		}while( 1 );
	}
	catch( ... )
	{
		AddLogEntry(L"### VibraniumALUSrv::CheckDestFolderAndCreate::Exception",0,0,true,SECONDLEVEL);
	}

	return bCreate ;
}

/***************************************************************************************************                    
*  Function Name  : CreateDirectoryFocefully()
*  Description    : It will check directory. If not present, it will create that directory.
*  Author Name    : Vilas  
*  SR_NO		  : WRDWIZALUSRV_0044
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CreateDirectoryFocefully( LPTSTR lpszPath )
{

	try
	{

		CreateDirectory( lpszPath, NULL );
		if( PathFileExists( lpszPath ) )
			return false;

		_wmkdir( lpszPath );
		if( PathFileExists( lpszPath ) )
			return false;
	}
	catch( ... )
	{
		AddLogEntry(L"### VibraniumALUSrv::CreateDirectoryFocefully::Exception",0,0,true,SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : CreateDirectoryFocefully4EPSHierarchy()
*  Description    : It will check directory. If not present, it will create that directory.
*  Author Name    : Amol Jaware
*  SR_NO		  : 
*  Date			  :	11/April/2018
****************************************************************************************************/
bool CreateDirectoryFocefully4EPSHierarchy(LPTSTR lpszPath)
{

	try
	{

		CreateDirectory(lpszPath, NULL);
		if (PathFileExists(lpszPath))
			return false;

		_wmkdir(lpszPath);
		if (PathFileExists(lpszPath))
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### VibraniumALUSrv::CreateDirectoryFocefully4EPSHierarchy::Exception", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************                    
*  Function Name  : ReplaceAllOriginalFiles()
*  Description    : If all files not copied properly, it will roll back all files to its old 
					files.
*  Author Name    : Vilas, neha  
**  SR_NO		  : WRDWIZALUSRV_0045
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool ReplaceAllOriginalFiles( )
{
	DWORD	dwCount = 0x00, i=0x00 ;

	TCHAR	szSourceFilePath[512] = {0};
	TCHAR	szDestFilePath[512] = {0};
	TCHAR	szTemp[512] = {0};
	TCHAR   szValueName[512] = {0};
	TCHAR   szValueData[512] = {0};
	TCHAR   szIniFilePath[512] = {0};
	TCHAR   szSrvOriginalFilePath[512]= {0};
	CString csServiceName;

	TCHAR	*pTemp = NULL;

	bool	bCopied = true;

	try
	{
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", g_szAllUserPath, g_csAppFolderName);
		

		if( !PathFileExists(szIniFilePath) )
		{
			//AddLogEntry(L"### ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
			// divya
			/* Issue Number 159 
			Failed to update product. Please check internet connection” 
			even if internet connection is there
			*/
			AddLogEntry(L"### Function is ReplaceAllOriginalFiles..... ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, ZEROLEVEL);
			//return false;
		}
		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath );

		if( dwCount )
		{
			for (int i = 1; i <= static_cast<int>(dwCount); i++)
			{
				swprintf_s(szValueName, _countof(szValueName), L"%lu", i );
				GetPrivateProfileString(L"Files", szValueName, L"", szValueData, 511, szIniFilePath);
				if( !szValueData[0] )
				{
					AddLogEntry(L"### VibraniumParseIniFile::Invalid Entries for(%s) in (%s)", szValueName, szIniFilePath, true, ZEROLEVEL);
					break;
				}
				else
				{
					vReplacedFiles.push_back(szValueData);
				}
			}
		}
		dwCount =0x00;


		dwCount = static_cast<DWORD>(vReplacedFiles.size());
		if( dwCount == 0x00 )
			return false ;

		bCopied = true;
		for(i=0; i<dwCount; i++ )
		{
			ZeroMemory( szSourceFilePath, sizeof(szSourceFilePath) );
			swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s", vReplacedFiles.at(i) );

			if( CheckForService(szSourceFilePath, csServiceName ) )
			{
				
				swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s%s", g_szAVPath,csServiceName);
				swprintf_s(szTemp, _countof(szTemp), L"%s%s_%s", g_szAVPath, csServiceName, L"WWIZALTSRV");
				if(PathFileExists(szSourceFilePath))
				{
					SetFileAttributes(szSourceFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szSourceFilePath);
				}
				_wrename(szTemp,szSourceFilePath);
				continue;
			}

			else 
			{
				if(PathFileExists(szSourceFilePath))
				{
					AddLogEntry(L">>> delete replaced file %s from destination",szSourceFilePath,0,true,ZEROLEVEL);
					SetFileAttributes(szSourceFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szSourceFilePath);
					bCopied = false;
					continue;
				}
			}

		}

		if( bCopied )
			AddLogEntry(L"### ReplaceAllOriginalFiles::Copy to original file(%s) failed", szSourceFilePath);
	}
	catch( ... )
	{
		AddLogEntry(L"### VibraniumALUSrv::CreateDirectoryFocefully::Exception",0,0,true,SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : AddEntryToALUDelIni()
*  Description    : Add all new files to ALUdel.ini files.
*  Author Name    : Vilas, neha   
*  SR_NO		  : WRDWIZALUSRV_0046
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool AddEntryToALUDelIni( LPTSTR lpszFilePath, LPTSTR lpszFileName )
{
	TCHAR	szValueName[512] = { 0 };
	TCHAR	szTempKey[512] = { 0 };
	TCHAR	szIniFilePath[512] = { 0 };
	TCHAR   szDuplicateString[512] = { 0 };
	TCHAR   szNullString[512] = { 0 };
	TCHAR   szCurrentFileName[512] = { 0 };
	DWORD   result = 0x01;
	DWORD	dwCount = 0x00;
	DWORD   dwDBCount = 0x00;
	bool	mFlag = false;

	try
	{
		if (lpszFileName != NULL)
		{
			_tcscpy(szCurrentFileName, lpszFileName);
		}
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", g_szAllUserPath, g_csAppFolderName);
		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);

		//Considering DB count Neha Gharge 18-2-2015 
		dwDBCount = GetPrivateProfileInt(L"DB", L"Count", 0x00, szIniFilePath);

		if (dwCount == 0)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", ++dwCount);
			WritePrivateProfileString(L"Files", szValueName, lpszFilePath, szIniFilePath);
		}
		else
		{
			for (; result <= dwCount; result++)
			{
				ZeroMemory(szTempKey, sizeof(szTempKey));
				swprintf_s(szTempKey, _countof(szTempKey), L"%lu", result);

				{
					GetPrivateProfileString(L"Files", szTempKey, L"", szDuplicateString, 511, szIniFilePath);
				}
				CString csFilePath = (CString)lpszFilePath;
				CString csDuplicateString = (CString)szDuplicateString;
				if (csFilePath.Compare(csDuplicateString) == 0)
				{
					AddLogEntry(L"### File Entry already present in ALUDel.ini", 0, 0, true, ZEROLEVEL);
					mFlag = false;
					break;
				}
				else
				{
					mFlag = true;
				}
			}
			if (mFlag)
			{
				{
					ZeroMemory(szValueName, sizeof(szValueName));
					swprintf_s(szValueName, _countof(szValueName), L"%lu", ++dwCount);
					WritePrivateProfileString(L"Files", szValueName, lpszFilePath, szIniFilePath);
					mFlag = false;
				}
			}
		}

		if (dwCount)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwCount);
			WritePrivateProfileString(L"Count", L"Count", szValueName, szIniFilePath);
		}

		//Neha Gharge 18-2-2015 if only DB file get downloaded.and dwcount == 0.
		if (dwDBCount)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwDBCount);
			WritePrivateProfileString(L"DB", L"Count", szValueName, szIniFilePath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### VibraniumALUSrv::AddEntryToALUDelIni::Exception for file(%s)", lpszFilePath, 0, true, SECONDLEVEL);
	}

	return false;
}

/***************************************************************************************************                    
*  Function Name  : DeleteAllDownloadedFiles()
*  Description    : Delete all downloaded files frm program data.
*  Author Name    : Vilas, neha    
*  SR_NO		  : WRDWIZALUSRV_0047
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool DeleteAllDownloadedFiles( )
{
	AddLogEntry(L">>> In DeleteAllDownloadedFiles", 0, 0, true, ZEROLEVEL);

	DWORD	dwCount = 0x00, i=0x00;

	TCHAR	szZipFilePath[512] = {0};
	TCHAR	szZipUnzipPath[512] = {0};

	try
	{
		dwCount = static_cast<DWORD>(vALUFileInfo.size());
		if( !dwCount )
			return false;

		for(; i<dwCount; i++ )
		{
			ZeroMemory(szZipFilePath, sizeof(szZipFilePath) );
			ZeroMemory(szZipUnzipPath, sizeof(szZipUnzipPath) );

			swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s\\%s\\%s.zip", g_szAllUserPath, g_csAppFolderName, vALUFileInfo[i].szFileName);
			swprintf_s(szZipUnzipPath, _countof(szZipUnzipPath), L"%s\\%s\\%s_%s", g_szAllUserPath, g_csAppFolderName, vALUFileInfo[i].szFileName, L"WWIZALT");

			AddLogEntry(L">>> Delete File: %s", szZipFilePath, 0, true, FIRSTLEVEL);
			SetFileAttributes(szZipFilePath, FILE_ATTRIBUTE_NORMAL);
			if (DeleteFile(szZipFilePath) == FALSE)
			{
				AddLogEntry(L">>> OnRestart Delete File: %s", szZipFilePath, 0, true, FIRSTLEVEL);
			}

			AddLogEntry(L">>> Delete File: %s", szZipUnzipPath, 0, true, FIRSTLEVEL);
			SetFileAttributes(szZipUnzipPath, FILE_ATTRIBUTE_NORMAL);
			if (DeleteFile(szZipUnzipPath) == FALSE)
			{
				AddLogEntry(L">>> OnRestart Delete File: %s", szZipUnzipPath, 0, true, FIRSTLEVEL);
			}

			if( PathFileExists(szZipFilePath) )
			{
				Sleep( 1 );
				SetFileAttributes(szZipFilePath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile( szZipFilePath );
			}

			if( PathFileExists(szZipUnzipPath) )
			{
				Sleep( 1 );
				SetFileAttributes(szZipUnzipPath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile( szZipUnzipPath );
			}

			if( PathFileExists(szZipFilePath) )
				MoveFileEx(szZipFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );

			if( PathFileExists(szZipUnzipPath) )
				MoveFileEx(szZipUnzipPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### VibraniumALUSrv::DeleteAllDownloadedFiles::Exception for delete operation",0,0,true,SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : GetTotalExistingBytes()
*  Description    : Get existing byte. \\This function is not used in whole program
*  Author Name    : Vilas, neha  
*  SR_NO		  : WRDWIZALUSRV_0049
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD GetTotalExistingBytes()
{
	memset(g_ExistingBytes, 0, sizeof(g_ExistingBytes));

	DWORD dwFirstFileSize = 0;
	DWORD dwSecondFileSize = 0;

	CString csFilePath = GetModuleFilePath() + L"\\DB\\DAILY.CLD";
	//HANDLE hSFile = CreateFile(csFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	HANDLE hSFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	dwFirstFileSize = 0;
	if(hSFile != INVALID_HANDLE_VALUE)
	{
		dwFirstFileSize = GetFileSize(hSFile,NULL);

	}
	CloseHandle(hSFile);

	csFilePath = GetModuleFilePath() + L"\\DB\\MAIN.CVD";
	//HANDLE hTFile = CreateFile(csFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	HANDLE hTFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	dwSecondFileSize = 0;
	if(hTFile != INVALID_HANDLE_VALUE)
	{
		dwSecondFileSize=GetFileSize(hTFile,NULL);
	}
	CloseHandle(hTFile);

	g_ExistingBytes[0] = dwFirstFileSize;
	g_ExistingBytes[1] = dwSecondFileSize;
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : GetTotalFilesSize()
*  Description    : Get total downloaded files size byte from server. 
*  Author Name    : Vilas, neha    
*  SR_NO		  : WRDWIZALUSRV_0050
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD GetTotalFilesSize()
{
	DWORD dwTotalFilesSize = 0;
	try
	{
		if(g_vUrlLists.size() > 0)
		{
			for(unsigned int iItemCount = 0; iItemCount < g_vUrlLists.size(); iItemCount++)
			{
				CString csItem = g_vUrlLists[iItemCount];
				TCHAR szInfo[MAX_PATH] = {0};
				TCHAR szTagetPath[MAX_PATH] = {0};
				DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
				DWORD dwFileSize = 0;
				if(g_objWinHttpManager.Initialize(csItem))
				{
					ZeroMemory(szInfo, sizeof(szInfo) ) ;
					g_objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo,dwBufLen);
					dwFileSize = 0x00 ;
					swscanf( szInfo, L"%lu", &dwFileSize ) ;
					dwTotalFilesSize += dwFileSize;
				}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::GetTotalFilesSize", 0, 0, true, SECONDLEVEL);
		dwTotalFilesSize = 0;
	}
	return dwTotalFilesSize;
}

/***************************************************************************************************                    
*  Function Name  : CheckInternetConnection()
*  Description    : It checks internet connection . 
*  Author Name    : Vilas, neha   
*  SR_NO		  : WRDWIZALUSRV_0053
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CheckInternetConnection()
{
	bool bReturn = false;
	try
	{
		CWWizHttpManager objWinHttpManager;
		TCHAR szTestUrl[MAX_PATH] = { 0 };
		_tcscpy_s(szTestUrl, MAX_PATH, _T("http://www.google.com"));
		if (objWinHttpManager.Initialize(szTestUrl))
		{
			if (objWinHttpManager.CreateRequestHandle(NULL))
			{
				return true;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::CheckInternetConnection", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : WaitForInternetConnection()
*  Description    : It waits internet connection for 10 min . 
*  Author Name    : Vilas, neha  
*  SR_NO		  : WRDWIZALUSRV_0054
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool WaitForInternetConnection()
{
	bool bReturn = false;
	int iRetryCount = 0;
	while(true)
	{
		if(!CheckInternetConnection())
		{
			if(iRetryCount > MAX_RETRY_COUNT)
			{
				bReturn = false;
				break;				
			}
			iRetryCount++;
			Sleep(10 * 1000);//wait here for 10 seconds
		}
		else
		{
			bReturn = true;
			break;
		}
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : GetApplicationDataFolder()
*  Description    : Gives path of application folder. 
*  Author Name    : Vilas, neha  
*  SR_NO		  : WRDWIZALUSRV_0055
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool GetApplicationDataFolder(TCHAR *szAppPath)
{
	try
	{
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szAppPath)))
		{
			return true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::GetApplicationDataFolder", 0, 0, true, SECONDLEVEL);;
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : OnDataReceiveCallBack()
*  Description    : Recieves pipe data from UI. 
*  Author Name    : Vilas, neha  
*  SR_NO		  : WRDWIZALUSRV_0056
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void OnDataReceiveCallBack(LPVOID lpParam)
{
}

/***************************************************************************************************                    
*  Function Name  : StartUpdate()
*  Description    : Initializing all variables and create a thread which start downloding and replacing file. 
*  Author Name    : Vilas, neha   
*  SR_NO		  : WRDWIZALUSRV_0057
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void StartUpdate()
{
	__try
	{

		if( !hThread_StartALUpdateProcess )
		{
			hThread_StartALUpdateProcess = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) StartALUpdateProcessThread, 
														NULL, 0, 0 ) ;
			Sleep( 500 ) ;

			if(hThread_StartALUpdateProcess == NULL)
				AddLogEntry(L"### Failed in SrvPrimaryThread::To create ALUpdateProcessThread", 0, 0, true, SECONDLEVEL);
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizComSrv StartUpdate", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : SendUpdateInfoToGUI()
*  Description    : Send all update info to GUI through shared memory
*  Author Name    : Vilas, neha  
*  SR_NO		  : WRDWIZALUSRV_0058
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool SendUpdateInfoToGUI(int iMessageInfo , CString csFirstParam , CString csSecondParam , DWORD dwFirstParam , DWORD dwSecondParm)
{

	if(g_bRequestFromUI)
	{
		AddLogEntry(L">>> Inside VibraniumALUSrv::SendUpdateInfoToGUI started %s",csFirstParam,0,true,ZEROLEVEL);
		if (g_OSType != WINOS_XP && g_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Lock();
		}

		//Send Messagees using pipe.
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		_tcscpy(szPipeData.szFirstParam, csFirstParam);
		szPipeData.dwValue = dwFirstParam;
		szPipeData.dwSecondValue = dwSecondParm;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(50);
			if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csLog;
				csLog.Format(L"iMessageInfo: %d, FirstParam :%s, SeondParam: %s, dwFirstParam: %d, dwSecondParm:%d", iMessageInfo, csFirstParam,
					csSecondParam, dwFirstParam, dwSecondParm);
				AddLogEntry(L"### Failed to send data inSendUpdateInfoToGUI %s", csLog, 0, true, SECONDLEVEL);
				if (g_OSType != WINOS_XP && g_OSType != WINOS_XP64)
				{
					g_objcriticalSection.Unlock();
				}
				return false;
			}
		}
		
		//Sleep(50);
		AddLogEntry(L">>> Inside VibraniumALUSrv::SendUpdateInfoToGUI Finished %s",csFirstParam,0,true,ZEROLEVEL);
		if (g_OSType != WINOS_XP && g_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Unlock();
		}
	}
	AddLogEntry(L">>> Inside VibraniumALUSrv::SendUpdateInfoToGUI end %s",csFirstParam,0,true,ZEROLEVEL);
	return true;
}

/***************************************************************************************************
*  Function Name  : GetTotalFileSizeThread
*  Description    : Thread function to get total file size.
*  Author Name    : Ram Shelke
*  Date           :	14-12-2017
****************************************************************************************************/
DWORD GetTotalFileSizeThread(LPVOID lpParam)
{
	try
	{
		//Get the total downloaded file size here
		g_dwTotalFileSize = GetTotalFilesSize();
		SendUpdateInfoToGUI(SETTOTALFILESIZE, EMPTY_STRING, EMPTY_STRING, g_dwTotalFileSize, 0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetTotalFileSizeThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : StartDownloadingINI()
*  Description    : Downloading Only ini according to client product ID
*  Author Name    : Vilas, neha 
*  SR_NO		  : WRDWIZALUSRV_0061
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool StartDownloadingINI(LPCTSTR szUrlPath)
{
	try
	{
		if(!szUrlPath)
			return false;

		//Sleep( 15 * 1000 );

		TCHAR szInfo[MAX_PATH] = {0};
		TCHAR szTagetPath[MAX_PATH] = {0};
		TCHAR szTargetFolder[MAX_PATH] = {0};
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		DWORD dwTotalFileSize = 0;

		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		if(g_objWinHttpManager.Initialize(szUrlPath))
		{
			if(!g_objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo,dwBufLen))
			{
				return false;
			}
			dwTotalFileSize = _wtol(szInfo);

			_stprintf_s(szTargetFolder, MAX_PATH, L"%s\\%s", g_szAppDataFolder, g_csAppFolderName);

			if(!PathFileExists(szTargetFolder))
			{
				CreateDirectoryFocefully(szTargetFolder);
			}

			_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s\\%s", g_szAppDataFolder, g_csAppFolderName, csFileName);

			//If file is already present check filesize and download from last point
			DWORD dwStartBytes = 0;
			if(PathFileExists(szTagetPath))
			{
				
				g_hIniFile = CreateFile(szTagetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if(g_hIniFile != INVALID_HANDLE_VALUE)
				{
					dwStartBytes = GetFileSize(g_hIniFile, 0);
					if(dwStartBytes != dwTotalFileSize)
					{
						g_objWinHttpManager.SetDownloadCompletedBytes(dwStartBytes);
					}
				}
				CloseHandle(g_hIniFile);
				g_hIniFile = NULL;
			}
			
			//Start download for ini file
			if(g_objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize))
			{
				//Once download complete set the download completed bytes.
				g_objWinHttpManager.SetDownloadCompletedBytes(dwTotalFileSize - dwStartBytes);
			}
			else
			{
				AddLogEntry(L"### Failed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::StartDownloadingINI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : StartDownloadingINIForZipFiles()
*  Description    : Downloading Only ini according to client product ID
*  Author Name    : Amol Jaware.
*  SR_NO		  : 
*  Date           :	12-April-2018
****************************************************************************************************/
bool StartDownloadingINIForZipFiles(LPCTSTR szUrlPath)
{
	try
	{
		if (!szUrlPath)
			return false;

		TCHAR szInfo[MAX_PATH] = { 0 };
		TCHAR szTargetPath[MAX_PATH] = { 0 };
		TCHAR szTargetFolder[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		DWORD dwTotalFileSize = 0;
		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);
		CWinHttpManager		objWinHttpManagerUpDM;
		Sleep(2000);

		if (objWinHttpManagerUpDM.Initialize(szUrlPath))
		{
			if (!objWinHttpManagerUpDM.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
			{
				return false;
			}
			dwTotalFileSize = _wtol(szInfo);

			wcscpy(szTargetFolder, m_cswardwizEPSPublishPath);
			if (!PathFileExists(szTargetFolder))
			{
				CreateDirectoryFocefully(szTargetFolder);
			}

			_stprintf_s(szTargetPath, MAX_PATH, L"%s\\%s", m_cswardwizEPSPublishPath, csFileName);

			//If file is already present check filesize and download from last point
			DWORD dwStartBytes = 0;
			if (PathFileExists(szTargetPath))
			{

				g_hIniFile = CreateFile(szTargetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (g_hIniFile != INVALID_HANDLE_VALUE)
				{
					dwStartBytes = GetFileSize(g_hIniFile, 0);
					if (dwStartBytes != dwTotalFileSize)
					{
						objWinHttpManagerUpDM.SetDownloadCompletedBytes(dwStartBytes);
					}
				}
				CloseHandle(g_hIniFile);
				g_hIniFile = NULL;
			}

			//Start download for ini file
			if (objWinHttpManagerUpDM.Download(szTargetPath, dwStartBytes, dwTotalFileSize))
			{
				//Once download complete set the download completed bytes.
				objWinHttpManagerUpDM.SetDownloadCompletedBytes(dwTotalFileSize - dwStartBytes);
			}
			else
			{
				AddLogEntry(L"### Failed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::StartDownloadingINIForZipFiles", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : SendMessage2UI()
*  Description    : Send message to Ui through Pipe
*  Author Name    : Vilas, neha 
*  SR_NO		  : WRDWIZALUSRV_0062
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool SendMessage2UI(int iRequest, DWORD dwUITypetoSendMessge, DWORD dwMsgType, DWORD dwSecondOption, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;

		CString csLog;
		csLog.Format(L"Request: %d\tUITypetoSendMessge:%d\tdwMsgType:%d", iRequest, dwUITypetoSendMessge, dwMsgType);

		if(dwUITypetoSendMessge == 1)//1 is for Main UI
		{
			CISpyCommunicator objCom(UI_SERVER, false);
			if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in VibraniumALUSrv::SendMessage2UI %s", csLog, 0, true, SECONDLEVEL);
				return false;
			}
		}
		else if(dwUITypetoSendMessge == 2)//2 Tray Server
		{
			szPipeData.dwValue = dwMsgType;
			szPipeData.dwSecondValue = dwSecondOption;
			CISpyCommunicator objCom(TRAY_SERVER, bWait);
			if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in VibraniumALUSrv::SendMessage2UI %s", csLog, 0, true, SECONDLEVEL);
				return false;
			}
		
			if(bWait)
			{
				if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
				{
					AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendRegisteredData2Service %s", csLog, 0, true, SECONDLEVEL);
					return false;
				}

				if(szPipeData.dwValue != 1)
				{
					return false;
				}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::SendMessage2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : GetPercentage()
*  Description    : Calculating percentage
*  Author Name    : Vilas, neha 
*  SR_NO		  : WRDWIZALUSRV_0063
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD GetPercentage(int iDownloaded, int iTotalSize)
{
	__try
	{
		return static_cast<DWORD>(((static_cast<double>((iDownloaded))/ iTotalSize))*80);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{		
		AddLogEntry(L"### Exception in VibraniumALUSrv::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : ClearMemoryMapObjects()
*  Description    : Clear memory object
*  Author Name    : Vilas, neha 
*  SR_NO		  : WRDWIZALUSRV_0064
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void ClearMemoryMapObjects()
{
	ITIN_MEMMAP_DATA iTinMemMap = {0};
	g_objALupdateServ.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
}

/***************************************************************************************************                    
*  Function Name  : CloseTrasferDataToUIThread()
*  Description    : Close Trasfer Data To UI thread
*  Author Name    : Vilas, neha    
*  SR_NO		  : WRDWIZALUSRV_0065
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void CloseTrasferDataToUIThread()
{
	if (g_OSType != WINOS_XP && g_OSType != WINOS_XP64)
	{
		g_objcriticalSection.Unlock();
	}
}

/***************************************************************************************************                    
*  Function Name  : CheckAnyModuleIsInProcess()
*  Description    : check module which are in process
*  Author Name    : Vilas, neha 
*  SR_NO		  : WRDWIZALUSRV_0067
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CheckAnyModuleIsInProcess()
{
	CEnumProcess objEnumProcess;
	if(objEnumProcess.IsProcessRunning(L"VBSCANNER.EXE", true, false, false))
	{
		AddLogEntry(L">>> %s was running", L"VBSCANNER.EXE", 0, true, ZEROLEVEL);
		return true;
	}

	if(objEnumProcess.IsProcessRunning(L"VBUSBDETECTUI.EXE", true, false, false))
	{
		AddLogEntry(L">>> %s was running", L"VBUSBDETECTUI.EXE", 0, true, ZEROLEVEL);
		return true;
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : IsZipFileMisMatched()
*  Description    : Matching size of zip file downloaded and Actual Size in ini.
*  Author Name    : Vilas, neha 
*  SR_NO		  : WRDWIZALUSRV_0068
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool IsZipFileMisMatched(LPCTSTR szFilePath, int iItemCount)
{
	TCHAR szTagetPath[512] = {0};
	CString csFileName(szFilePath);
	int iFound = csFileName.ReverseFind(L'/');
	csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);
	_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s\\%s", g_szAppDataFolder, g_csAppFolderName, csFileName);

	DWORD dwFileSize = 0;

	g_hTargetFile = CreateFile(szTagetPath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(g_hTargetFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	dwFileSize = 0;
	if(g_hTargetFile != INVALID_HANDLE_VALUE)
	{
		dwFileSize = GetFileSize(g_hTargetFile,NULL);
		
	}
	CloseHandle(g_hTargetFile);
	g_hTargetFile = NULL;

	if(dwFileSize != vALUFileInfo[iItemCount].dwZipSize)
	{
		return true;

	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : CheckForServiceName
*  Description    : If service is replcing, It will check name of service  
*  Author Name    : Vilas , Neha  
*  SR_NO		  : WRDWIZALUSRV_0069
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CheckForServiceName(int iIndex)
{
	for (int j = 0; j < static_cast<int>(g_vServiceNameVector.size()); j++)
	{
		if(_tcscmp((vALUFileInfo[iIndex].szFileName),g_vServiceNameVector.at(j))== 0)
		{
			return true;
		}
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : CheckForService()
*  Description    : It will return name of service 
*  Author Name    : Vilas , Neha  
*  SR_NO		  : WRDWIZALUSRV_0070
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CheckForService(TCHAR *szFilePathFromVector , CString &MatchedServiceName)
{
	for (int j = 0; j < static_cast<int>(g_vServiceNameVector.size()); j++)
	{
		if( wcsstr(szFilePathFromVector, g_vServiceNameVector.at(j) ) )
		{
			MatchedServiceName = g_csServiceNameArray[j];
			return true;
		}
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : AddServiceNameIntoVector()
*  Description    : Add all services into vector.
*  Author Name    : Vilas , Neha 
**  SR_NO		  : WRDWIZALUSRV_0071
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool AddServiceNameIntoVector()
{
	__try
	{
		g_vServiceNameVector.clear();

		for (int j = 0; g_csServiceNameArray[j] != L"\0"; j++)
		{
			g_vServiceNameVector.push_back(g_csServiceNameArray[j]);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::AddServiceNameIntoVector", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : StartALUpdateAfter3Hrs()
*  Description    : Start Updation after 3 hrs.
*  Author Name    : Vilas , Neha  
**  SR_NO		  : WRDWIZALUSRV_0072
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD StartALUpdateAfter3Hrs( LPVOID lpParam )
{
	__try
	{
		// issue:- product version getting update after setup install event product does not update completly
		// resolved by neha,lalit 9-18-2015
		DWORD dwWaitTime = 10 * 60 * 1000;//10 minutes

		WaitForSingleObject(g_hUpdateFromUIEvent, dwWaitTime);
		
		while(true)
		{
			if (ReadAutoliveUpdateEnableCheck())
			{
				//StartUpdate();
				LaunchUpdateExe();
			}
			
			ResetEvent(g_hUpdateFromUIEvent);
			
			//DWORD dwWaitTime = 3 * 60 * 1000;//3 min
			DWORD dwWaitTime = 3*60*60*1000;//3 hrs

			WaitForSingleObject(g_hUpdateFromUIEvent , dwWaitTime); 
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{		
		AddLogEntry(L"### Exception in WardwizComSrv SendUpdateInfoToGUI", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

/***************************************************************************************************
*  Function Name  : LaunchUpdateExe
*  Description    : Launch Update exe.
*  Author Name    : Vilas , Neha
**  SR_NO		  : WRDWIZALUSRV_0072
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void LaunchUpdateExe()
{
	try
	{

		CExecuteProcess			objEnumprocess;
		CString csExePath, csCommandLine;

		csCommandLine.Format(L"%s", L"-EPSNOUI -LIVEUPDATE");
		csExePath.Format(L"%s%s", GetWardWizPathFromRegistry(), L"VBUPDATE.EXE");
		objEnumprocess.StartProcessWithTokenExplorerWait(csExePath, csCommandLine, L"explorer.exe");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizComSrv LaunchUpdateExe", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : ReadAutoliveUpdateEnableCheck()
*  Description    : Read AutoliveUpdate Check
*  Author Name    : Vilas , Neha  
**  SR_NO		  : WRDWIZALUSRV_0073
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool ReadAutoliveUpdateEnableCheck()
{
	CITinRegWrapper objReg;
	
	DWORD dwAutoliveUpdate = 0;
	if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwAutoDefUpdate", dwAutoliveUpdate) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwAutoDefUpdate", 0, 0, true, ZEROLEVEL);;
		return false;
	}

	if(dwAutoliveUpdate == 0)
	{
		g_bEnableAutoliveUpdate =  false;
	}
	else
	{
		g_bEnableAutoliveUpdate =  true;
	}
	return g_bEnableAutoliveUpdate;
}

/***************************************************************************************************
*  Function Name  : ReadProductUpdateEnableCheck()
*  Description    : Read AutoliveUpdate Check
*  Author Name    : Ramkrushna Shelke
**  SR_NO		  : 
*  Date			  :	10 - Oct - 2016
****************************************************************************************************/
bool ReadProductUpdateEnableCheck()
{
	bool bReturn = true;
	try
	{
		CITinRegWrapper objReg;

		DWORD dwProductUpdate = 0;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwAutoProductUpdate", dwProductUpdate) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwAutoDefUpdate in ReadProductUpdateEnableCheck", 0, 0, true, SECONDLEVEL);;
			return bReturn;
		}

		if (dwProductUpdate == 0x00)
		{
			bReturn = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReadProductUpdateEnableCheck", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : WriteIntoRegistry()
*  Description    : Write into registry
*  Author Name    : Vilas , Neha 
*  SR_NO		  : WRDWIZALUSRV_0074
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
*  Modified Date  : 7-Feb-2015 Neha Gharge Update version no issue resolved.
****************************************************************************************************/
bool WriteIntoRegistry()
{
	TCHAR	szIniFilePath[512] = {0};
	TCHAR  szValueData[512] ={0};
	DWORD	dwProductID = 0x00;
	DWORD dwSuccess = 0x00;
	CITinRegWrapper objReg;
	//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
	//Resolved By :  Nitin K.
	TCHAR	szALUDelIniFilePath[512] = { 0 };
	
	GetProductID( dwProductID );
	if(!dwProductID)
	{
		AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
		return true;
	}

	switch( dwProductID )
	{
		case 1:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 2:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 3:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 4:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 5:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", g_szAllUserPath, g_csAppFolderName);
			break;
	}

	
	if( !PathFileExists(szIniFilePath) )
	{
		//AddLogEntry(L"### ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
		//divya
		/* Issue Number 159 
		Failed to update product. Please check internet connection” 
		even if internet connection is there
		*/
		AddLogEntry(L"### Functin is WriteIntoRegistry.... ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, ZEROLEVEL);
		//return false;
	}
	//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
	//Resolved By :  Nitin K.
	
	swprintf_s(szALUDelIniFilePath, _countof(szALUDelIniFilePath), L"%s\\%s\\%s", g_szAllUserPath, g_csAppFolderName, WRDWIZALUDELINI);

	//issue:- support number not getting reflected in product,now support number added to ini file
	//  resolve by lalit,neha 9-18-2015
	if (PathFileExists(szIniFilePath))
	{
		DWORD dwOfflineReg = 0;

		dwOfflineReg = CheckOfflineReg();


		if (dwOfflineReg == 0x00)
		{
			GetPrivateProfileString(L"SupportNumber", L"SupportNumberNC", L"", szValueData, 511, szIniFilePath);
		}
		else
		{
			GetPrivateProfileString(L"SupportNumber", L"SupportNumber", L"1800 203 4600", szValueData, 511, szIniFilePath);
		}
		//Issue: For now we dont need to show contact no for german/NC setup.
		/*if (szValueData[0])
		{*/
		dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"SupportNo", szValueData);
		if (dwSuccess)
		{
			AddLogEntry(L"### Failed to Set support number in registry", 0, 0, true, SECONDLEVEL);
		}
		//}
		
	}

	if ((!PathFileExists(szALUDelIniFilePath)) && g_bIsALUDeleted == FALSE)
	{

		if (PathFileExists(szIniFilePath))
		{
			GetPrivateProfileString(L"ProductVersion", L"ProductVer", L"", szValueData, 511, szIniFilePath);
			if (!szValueData[0])
			{
				AddLogEntry(L"### WriteIntoRegistry::Invalid Entries for(%s) in (%s)", szValueData, szIniFilePath, true, ZEROLEVEL);
			}
			else
			{
				if (ReadProductUpdateEnableCheck())
				{
					dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"AppVersion", szValueData);
					if (dwSuccess)
					{
						AddLogEntry(L"### Failed to Setregistryvaluedata %s", CWWizSettingsWrapper::GetProductRegistryKey(), 0, true, SECONDLEVEL);
					}
				}

				TCHAR szScanEngineVer[0x20] = { 0 };
				GetPrivateProfileString(L"ScanEngineVersion", L"ScanEngineVer", L"", szScanEngineVer, 0x20, szIniFilePath);
				if (szScanEngineVer[0])
				{
					dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"ScanEngineVersion", szScanEngineVer);
					if (dwSuccess)
					{
						AddLogEntry(L"### Failed to Setregistryvaluedata for %s : ScanEngineVersion", CWWizSettingsWrapper::GetProductRegistryKey(), 0, true, SECONDLEVEL);
					}
				}
			}

			GetPrivateProfileString(L"DatabaseVersion", L"DatabaseVer", L"", szValueData, 511, szIniFilePath);
			if (!szValueData[0])
			{
				AddLogEntry(L"### WriteIntoRegistry::Invalid Entries for(%s) in (%s)", szValueData, szIniFilePath, true, ZEROLEVEL);
			}
			else
			{
				dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"DataBaseVersion", szValueData);
				if (dwSuccess)
				{
					AddLogEntry(L"### Failed to Setregistryvaluedata", 0, 0, true, SECONDLEVEL);
				}
			}

			//Neha Gharge 8 August,2015 Need to add DataEncrtption version no.
			if (dwProductID != 4)//if product ID is basic. No need to add dataencryption version.
			{
				GetPrivateProfileString(L"DataEncVersion", L"DataEncVer", L"", szValueData, 511, szIniFilePath);
				if (!szValueData[0])
				{
					AddLogEntry(L"### WriteIntoRegistry::Invalid Entries for(%s) in (%s)", szValueData, szIniFilePath, true, ZEROLEVEL);
					return false;
				}
				else
				{
					dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"DataEncVersion", szValueData);
					if (dwSuccess)
					{
						AddLogEntry(L"### Failed to Setregistryvaluedata", 0, 0, true, SECONDLEVEL);
						return false;
					}
				}
			}
		}
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : IsLatestFile()
*  Description    : Is file latest
*  Author Name    : Vilas , Neha  
**  SR_NO		  : WRDWIZALUSRV_0075
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool IsLatestFile( const CString& csFilePath)
{
	int iFound = csFilePath.ReverseFind(L'/');

	if( iFound == -1 )
		return false;

	CString csFileName = csFilePath.Right(csFilePath.GetLength() - iFound - 1);

	if( csFileName.GetLength() < 4 )
		return false;

	csFileName = csFileName.Mid(0, csFileName.GetLength()- 4 );

	bool bFound = true;
	DWORD i=0;

	for(; i<vALUFileInfo.size(); i++ )
	{
		if( csFileName.CompareNoCase( vALUFileInfo[i].szFileName) == 0 )
		{
			bFound = false;
			break;
		}
	}

	if( bFound )
		return false;

	TCHAR	szFullFilePath[512] = {0};

	GetFilePathFromshortPath(szFullFilePath, vALUFileInfo[i].szFileShortPath, vALUFileInfo[i].szFileName );

	TCHAR csFileALTPath[512] = {0};
	TCHAR	szFileHash[512] = {0};
	DWORD	dwFileSize = 0x00;

	swprintf_s(csFileALTPath, 511, L"%s_WWIZALTSRV", szFullFilePath );

	if( PathFileExists(csFileALTPath) )
	{
		GetFileSizeAndHash(csFileALTPath, dwFileSize, szFileHash );
		if( dwFileSize == vALUFileInfo[i].dwFullSize )
		{
			if(_wcsicmp(szFileHash, vALUFileInfo[i].szFileHash ) == 0 )
			{
				vALUFileInfo.erase( vALUFileInfo.begin()+ i );
				return true;
			}
		}
	}

	ZeroMemory(csFileALTPath, sizeof(csFileALTPath) );
	swprintf_s(csFileALTPath, 511, L"%s_WWIZALT", szFullFilePath );

	if( PathFileExists(csFileALTPath) )
	{
		dwFileSize = 0x00;
		ZeroMemory(szFileHash, sizeof(szFileHash) );

		GetFileSizeAndHash(csFileALTPath, dwFileSize, szFileHash );
		if( dwFileSize == vALUFileInfo[i].dwFullSize )
		{
			if(_wcsicmp(szFileHash, vALUFileInfo[i].szFileHash ) == 0 )
			{
				vALUFileInfo.erase( vALUFileInfo.begin()+ i );
				return true;
			}
		}
	}

	return false;
}

/***************************************************************************************************                    
*  Function Name  : GetFileSizeAndHash()
*  Description    : Get FileSize And Hash
*  Author Name    : Vilas , Neha 
**  SR_NO		  : WRDWIZALUSRV_0076
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool GetFileSizeAndHash(TCHAR *pFilePath, DWORD &dwFileSize, TCHAR *pFileHash)
{

	try
	{

		HANDLE hFile = CreateFile( pFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### GetFileSizeAndHash : Error in opening file %s",pFilePath,0,true,SECONDLEVEL);
			return true;
		}

		dwFileSize = GetFileSize(hFile, NULL );
		CloseHandle( hFile );
		hFile = INVALID_HANDLE_VALUE;

		if( pFileHash )
		{
				if( !GetFileHash(pFilePath, pFileHash) )
				return false;
			else
			{
				AddLogEntry(L"### VibraniumALUSrv Exception in CVibraniumALUpdDlg::IsWow64",0,0,true,SECONDLEVEL);
				return true;
			}
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### VibraniumALUSrv Exception in CVibraniumALUSrv::IsWow64",0,0,true,SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************                    
*  Function Name  : GetFilePathFromshortPath()
*  Description    : Get FilePath From shortPath
*  Author Name    : Vilas , Neha
**  SR_NO		  : WRDWIZALUSRV_0077
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool GetFilePathFromshortPath(LPTSTR lpszFilePath, LPTSTR lpszShortPath, LPTSTR lpszFileName )
{

	TCHAR	*pTemp = wcschr(lpszShortPath, '\\');

	if( pTemp )
	{
		pTemp++;
		swprintf_s(lpszFilePath, 511, L"%s%s\\%s", g_szAVPath, pTemp, lpszFileName );
	}
	else
		swprintf_s(lpszFilePath, 511, L"%s%s", g_szAVPath, lpszFileName );

	return false;
}

/***************************************************************************************************                    
*  Function Name  : GetNumberOfDaysLeft()
*  Description    : get Number of days left
*  Author Name    : Vilas , Neha 
**  SR_NO		  : WRDWIZALUSRV_0078
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void GetNumberOfDaysLeft()
{
	__try
	{
		typedef DWORD (*GETDAYSLEFT_SERVICE)( DWORD ) ;

		GETDAYSLEFT_SERVICE	GetDaysLeft = NULL ;

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
			GetDaysLeft = (GETDAYSLEFT_SERVICE ) GetProcAddress( hMod, "GetDaysLeft" ) ;

		if( GetDaysLeft )
		{
			DWORD	dwProductID = 0x00;

			GetProductID( dwProductID );	
			g_dwDaysLeft = GetDaysLeft( dwProductID ) ;
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

/***************************************************************************************************                    
*  Function Name  : UpdateTimeDate()
*  Description    : Update date and time
*  Author Name    : Vilas , Neha 
**  SR_NO		  : WRDWIZALUSRV_0079
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool UpdateTimeDate()
{
	DWORD dwSuccess = 0x00;
	CString  csDate,csTime;
	TCHAR szOutMessage[30] = {0};
	TCHAR tbuffer[16]= {0};
	TCHAR dbuffer[16] = {0};
	/*Rajil Yadav Issue no. 598, 6/11/2014*/
	//SYSTEMTIME  CurrTime = {0} ;
	//GetLocalTime( &CurrTime ) ;
	//CTime Time_Curr( CurrTime ) ;
	//Neha Gharge 20-1-2015 Need a local time not a UTC time to compare update older than 7 days.
	CTime Time_Curr = CTime::GetCurrentTime() ;
	int iMonth = Time_Curr.GetMonth();
	int iDate = Time_Curr.GetDay();
	int iYear = Time_Curr.GetYear();

	_wstrtime_s(tbuffer, 15);
	csTime.Format(L"%s\0\r\n",tbuffer);
	csDate.Format(L"%d/%d/%d",iMonth,iDate,iYear);
	_stprintf(szOutMessage, _T("%s %s\0"), csDate, tbuffer);

	if (g_dwIsUptoDateCompleted != ALUPDATED_UPTODATE) //1 means ALUPDATED_UPTODATE
	{
		CITinRegWrapper objReg;
		dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"LastLiveupdatedt", szOutMessage);
		if (dwSuccess)
		{
			AddLogEntry(L"###Failed to Setregistryvaluedata for LastLiveupdatedt", 0, 0, true, SECONDLEVEL);
			//return false;
		}
		dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"LastLiveupdatetm", tbuffer);
		if (dwSuccess)
		{
			AddLogEntry(L"###Failed to Setregistryvaluedata for LastLiveupdatetm", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
return true;
}

/***************************************************************************************************                    
*  Function Name  : AddServiceEntryInSafeMode()
*  Description    : Add service entry in safe mode
*  Author Name    : Vilas , Neha 
**  SR_NO		  : WRDWIZALUSRV_0080
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void AddServiceEntryInSafeMode( )
{
	CITinRegWrapper	objReg;

	if(objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\WardwizALUSrv", L"", 
		L"Service") != 0)
	{
		AddLogEntry(L"### SetRegistryValueData failed in Minimal for ServiceVibraniumALUSrv in  AddServiceEntryInSafeMode()",0,0,true,SECONDLEVEL);
	}

	if(objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\WardwizALUSrv", L"", 
		L"Service") != 0)
	{
		AddLogEntry(L"### SetRegistryValueData failed in Network for ServiceVibraniumALUSrv in  AddServiceEntryInSafeMode()",0,0,true,SECONDLEVEL);
	}
}

/***********************************************************************************************
  Function Name  : GetDomainName
  Description    : Function to get wardwiz domain name.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10 Oct 2014
***********************************************************************************************/
bool GetDomainName(LPTSTR pszDomainName, DWORD dwSize)
{
	if (!g_bIsRelayClient)
	{
		CString csServerName = CWWizSettings::WWizGetServerIPAddress();
		_tcscpy_s(pszDomainName, dwSize, csServerName);
	}
	else
	{
		if(g_csServerName.IsEmpty())
		{ 
			_tcscpy_s(pszDomainName, dwSize, L"www.vibranium.co.in");
		}
		else
		{
			_tcscpy_s(pszDomainName, dwSize, g_csServerName);
		}
	}
	return true;
}

/***********************************************************************************************
  Function Name  : PerformPostUpdateRegOperations
  Description    : Performs registry operation after successful update.
  Author Name    : Vilas Suvarnakar
  SR.NO			 : 
  Date           : 17 Jan 2015
  Modified Date	 : 7/2/2015 Neha Gharge all Ini name changes.
***********************************************************************************************/
void PerformPostUpdateRegOperations( )
{
	TCHAR	szIniFilePath[512] = {0};

	__try
	{

		DWORD	dwProductID = 0x00;

		GetProductID( dwProductID );
		if( (!dwProductID))
		{
			AddLogEntry(L"### Product ID mismatched In PerformPostUpdateRegOperations", 0, 0, true, SECONDLEVEL);
			return;
		}

		switch( dwProductID )
		{
			case 1:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", g_csAppFolderName, g_szAllUserPath);
				break;
			case 2:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", g_csAppFolderName, g_szAllUserPath);
				break;
			case 3:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", g_csAppFolderName, g_szAllUserPath);
				break;
			case 4:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 5:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", g_szAllUserPath, g_csAppFolderName);
				break;
		}

		//When we update from 1.0.0.8 to 1.10.0.0 version where run entry in registry is present. On restart, it find for new file
		// VibroPatchTS.ini which are not present on restart as well so it will search entry from VBPatchTS.ini file.
		if (!PathFileExists(szIniFilePath))
		{
			switch (dwProductID)
			{
			case 1:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VBPatchTS.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 2:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchP.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 3:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchT.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 4:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchB.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			}
		}

		DWORD	dwNumberRegOpt = GetPrivateProfileInt(L"RegOpt", L"Count", 0, szIniFilePath);

		if( !dwNumberRegOpt )
		{
			AddLogEntry(L">>>> No post update Registry Operations", 0, 0, true, ZEROLEVEL);
			goto Cleanup;
		}

		DWORD dwIter = 0x01;
		for(; dwIter <=dwNumberRegOpt; dwIter++ )
		{
			TCHAR	szValueName[128] = {0};
			TCHAR	szValueData[2048] = {0};

			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwIter );
			GetPrivateProfileString(L"RegOpt", szValueName, L"", szValueData, sizeof(szValueData)-0x02, szIniFilePath);

			if( !wcslen(szValueData) )
				continue;

			ParseRegLineForPostUpdate( szValueData );
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in PerformPostUpdateRegOperations", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return;
}

/***********************************************************************************************
  Function Name  : ParseRegLineForPostUpdate
  Description    : Parses line and performs registry operation after successful update.
  Author Name    : Vilas Suvarnakar
  SR.NO			 : 
  Date           : 17 Jan 2015
  modification   : 30 March 2015 Neha Gharge
				 : Modification of any registry key.
***********************************************************************************************/
DWORD ParseRegLineForPostUpdate( LPTSTR lpszIniLine )
{
	DWORD	dwRet = 0x00;
	__try
	{
		TCHAR	szToken[] = L",";
		TCHAR	*pToken = NULL;

		pToken = wcstok(lpszIniLine, szToken);
		if( !pToken )
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		TCHAR	szRootKey[32] = {0};
		TCHAR	szSubKey[512] = {0};
		TCHAR	szValueName[512] = {0};
		TCHAR	szRegOpt[32] = {0};
		TCHAR	szModifiactionValue[512] = { 0 };
		TCHAR	szModifiactionPath[512] = { 0 };

		wcscpy(szRootKey, pToken );


		pToken = wcstok(NULL, szToken);
		if( !pToken )
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		wcscpy(szSubKey, pToken );

		pToken = wcstok(NULL, szToken);
		if( !pToken )
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		wcscpy(szValueName, pToken );

		pToken = wcstok(NULL, szToken);
		if( !pToken )
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		wcscpy(szRegOpt, pToken );
		_wcsupr( szRegOpt );

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			dwRet = 0x05;
		}

		if (pToken)
			wcscpy(szModifiactionValue, pToken);



		DWORD	dwRegOpt = 0x00;

		if( wcscmp( szRegOpt, L"DEL" ) == 0x00 )
			dwRegOpt = 0x01;

		if( wcscmp( szRegOpt, L"ADD" ) == 0x00 )
			dwRegOpt = 0x02;

		if( wcscmp( szRegOpt, L"MOD" ) == 0x00 )
			dwRegOpt = 0x03;

		if( !dwRegOpt )
		{
			dwRet = 0x05;
			goto Cleanup;
		}

		HKEY	hRootKey = NULL ;

		if( wcscmp(szRootKey, L"HKLM" ) == 0x00 )
			hRootKey = HKEY_LOCAL_MACHINE;
		else
			hRootKey = HKEY_CURRENT_USER;

		switch( dwRegOpt )
		{
			case 0x01:
				if( g_objReg.DelRegistryValueName(hRootKey, szSubKey, szValueName ) )
					AddLogEntry(L">>> No Registry value for post update operation", szSubKey, szValueName, true, FIRSTLEVEL);
				else
					AddLogEntry(L">>> Deleted Registry value for post update operation", szSubKey, szValueName, true, FIRSTLEVEL);
				break;

			case 0x03:
				RenameModifiedregitrypath(szModifiactionValue);
				if (g_objReg.SetRegistryValueData(hRootKey, szSubKey, szValueName, g_szModificationAppPath))
				{
					AddLogEntry(L"### Failed to modify Registry value for post update operation %s, %s", szSubKey, szValueName, true, SECONDLEVEL);
				}
				else
				{
					AddLogEntry(L">>> Modify Registry value for post update operation %s, %s", szSubKey, szValueName, true, ZEROLEVEL);
				}

				break;
			default:
				AddLogEntry(L">>> No operation for post update Registry", 0, 0, true, FIRSTLEVEL);
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ParseRegLineForPostUpdate", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet;
}

/***************************************************************************
Function Name  : RegisterComDLL
Description    : Register Com DLL
Author Name    : Neha Gharge
Date           : 17th March 2015
****************************************************************************/
bool RegisterComDLL()
{
	try
	{	
		CString csExePath, csCommandLine;
		CWardWizOSversion		objOSVersionWrap;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		csExePath.Format(L"%s\\%s", systemDirPath, L"regsvr32.exe");

		for (int j = 0; j < static_cast<int>(g_vCOMDLLNameVector.size()); j++)
		{
			csCommandLine.Format(L"-s \"%s%s\"", GetWardWizPathFromRegistry(),g_vCOMDLLNameVector.at(j));
			//On xp runas parameter never work It will not unregister the VBshellext.dll
			//So NUll parameter send.
			//DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
			//Neha Gharge Message box showing of register successful on reinstallation.
			switch (g_OSType)
			{
			case WINOS_XP:
			case WINOS_XP64:
				ShellExecute(NULL, NULL, csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
				break;
			default:
				ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
				break;
			}
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RegisterComDLL", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReadValueFromDLLregisterSectionSEH()
*  Description    : Read value register dll section.
*  Author Name    : Neha
**  SR_NO		  :
*  Modified Date  :	17th March 2015
****************************************************************************************************/
void ReadValueFromDLLregisterSection()
{
	__try
	{
		ReadValueFromDLLregisterSectionSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ReadValueFromDLLregisterSection", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : ReadValueFromDLLregisterSectionSEH()
*  Description    : Read value register dll section.
*  Author Name    : Neha
**  SR_NO		  : 
*  Modified Date  :	17th March 2015
****************************************************************************************************/
void ReadValueFromDLLregisterSectionSEH()
{
	TCHAR	szIniFilePath[512] = { 0 };

	try
	{

		DWORD	dwProductID = 0x00;
		CString csDllName(L"");

		g_vServiceNameVector.clear();

		GetProductID(dwProductID);
		if (!dwProductID)
		{
			AddLogEntry(L"### Product ID mismatched for com dll registration section.", 0, 0, true, SECONDLEVEL);
			return;
		}

		switch (dwProductID)
		{
		case 1:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 2:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 3:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 4:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 5:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		}

		//When we update from 1.0.0.8 to 1.10.0.0 version where shellextension.dll is present. On restart, it find for new file
		// WWizPatchB.ini which are not present on restart as well so it will search entry from VBPatchTS.ini file.
		if (!PathFileExists(szIniFilePath))
		{
			switch (dwProductID)
			{
			case 1:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VBPatchTS.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 2:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchP.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 3:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchT.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 4:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchB.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			case 5:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", g_szAllUserPath, g_csAppFolderName);
				break;
			}
		}

		DWORD	dwNumberRegisterDLL = GetPrivateProfileInt(L"DllRegister", L"Count", 0, szIniFilePath);
		if (!dwNumberRegisterDLL)
		{
			AddLogEntry(L"### No entry of dll to register", 0, 0, true, ZEROLEVEL);
			return;
		}

		for (DWORD iIndex = 0x01; iIndex <= dwNumberRegisterDLL; iIndex++)
		{
			TCHAR	szValueName[128] = { 0 };
			TCHAR	szValueData[512] = { 0 };

			swprintf_s(szValueName, _countof(szValueName), L"%lu", iIndex);
			GetPrivateProfileString(L"DllRegister", szValueName, L"", szValueData, 511, szIniFilePath);
			if (szValueData[0])
			{
				csDllName = szValueData;
				AddCOMDLLNameIntoVector(csDllName);
			}
		}
		RegisterComDLL();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUsrv::ReadValueFromDLLregisterSection", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : AddCOMDLLNameIntoVector()
*  Description    : Add all COM Dll into vector.
*  Author Name    : Neha
**  SR_NO		  :
*  Modified Date  :	17th March 2015
****************************************************************************************************/
bool AddCOMDLLNameIntoVector(CString csCOMDLLName)
{
	g_vCOMDLLNameVector.push_back(csCOMDLLName);
	return true;
}

/***************************************************************************************************
*  Function Name  : RenameModifiedregitrypath()
*  Description    : To read modified reg path.
*  Author Name    : Neha
**  SR_NO		  :
*  Modified Date  :	17th March 2015
****************************************************************************************************/
bool RenameModifiedregitrypath(TCHAR *szModifiactionValue)
{
	try
	{
		CString	csModificationAppPath;
		TCHAR szAVPath[512] = { 0 };
		csModificationAppPath = szModifiactionValue;
		_tcsnccpy(szAVPath, g_szAVPath, (_tcslen(g_szAVPath) - 1));
		csModificationAppPath.Replace(L"AppFolder", szAVPath);
		wcscpy(g_szModificationAppPath, csModificationAppPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUsrv::RenameModifiedregitrypath", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : GetFileSize
Description    : Function to get file size in reference agrgument.
				 if function success return true else false.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 08 Apr 2015
***********************************************************************************************/
bool GetFileSize(LPTSTR pFilePath, DWORD &dwFileSize)
{
	bool bReturn = false;
	__try
	{
		HANDLE hFile = CreateFile(pFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### In GetFileSize Error in opening file %s", pFilePath, 0, true, SECONDLEVEL);
			return bReturn;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;

		bReturn = true;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in GetFileSize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : UnzipUsingZipArchive
Description    : Second level extraction.if function success return zero else non-zero.
Author Name    : Vilas Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
DWORD UnzipUsingZipArchiveSEH(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath)
{
	DWORD	dwReturn = 0x00;

	CZipArchive zip;

	try
	{
	
		if (PathFileExists(lpszUnizipFilePath))
			DeleteFileForcefully(lpszUnizipFilePath);

		if (!zip.Open(lpszZipFilePath, CZipArchive::zipOpenReadOnly))
		{
			dwReturn = 0x01;
			goto Cleanup;
		}

		TCHAR	szPath[256] = { 0 };
		TCHAR	szFile[256] = { 0 };

		wcscpy_s(szPath, 255, lpszUnizipFilePath);

		TCHAR	*pFileName = wcsrchr(szPath, '\\');

		if (!pFileName)
		{
			dwReturn = 0x02;
			goto Cleanup;
		}

		TCHAR	*pTemp = pFileName;

		pFileName++;
		wcscpy_s(szFile, 255, pFileName);

		*pTemp = '\0';

		if (!zip.ExtractFile(0, szPath, true, szFile))
		{
			dwReturn = 0x03;
			goto Cleanup;
		}
	}
	catch (...)
	{
		dwReturn = 0x04;
		AddLogEntry(L"### Exception in UnzipUsingZipArchiveSEH", 0, 0, true, SECONDLEVEL);

		//Need to Delete Downloaded file, as may be it is corrupted, as throwing exception.
		zip.CloseFile(lpszZipFilePath);
		if (!DeleteFileForcefully(lpszZipFilePath))
		{
			AddLogEntry(L"### UnzipUsingZipArchiveSEH:: Failed to delete file forcefully: %s ", lpszZipFilePath, 0, true, SECONDLEVEL);
		}
		return dwReturn;
	}
Cleanup:
	zip.CloseFile(lpszZipFilePath);
	return dwReturn;
}

/***********************************************************************************************
Function Name  : UnzipUsingZipArchive
Description    : Second level extraction.if function success return zero else non-zero.
Author Name    : Vilas Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
DWORD UnzipUsingZipArchive(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath)
{
	DWORD dwReturn = 0x00;

	__try
	{
		dwReturn = UnzipUsingZipArchiveSEH(lpszZipFilePath, lpszUnizipFilePath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x05;
		AddLogEntry(L"### Exception in UnzipUsingZipArchive", 0, 0, true, SECONDLEVEL);
	}

	return dwReturn;
}

/***********************************************************************************************
Function Name  : SendData2CommService
Description    : Send data to communication service.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 08 Apr 2015
***********************************************************************************************/
DWORD SendData2CommService(int iMesssageInfo, bool bWait)
{
	DWORD dwRet = 0x00;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMesssageInfo;
		CISpyCommunicator objCom(SERVICE_SERVER, false);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in SendData2CommService", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
			goto FAILED;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in SendData2CommService", 0, 0, true, SECONDLEVEL);
				dwRet = 0x02;
				goto FAILED;
			}

			if (szPipeData.dwValue != 1)
			{
				dwRet = 0x03;
				goto FAILED;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SendData2CommService", 0, 0, true, SECONDLEVEL);
		dwRet = 0x04;
	}
FAILED:
	return dwRet;
}

/***********************************************************************************************
Function Name  : RunUtililtyInSilentMode
Description    : Run utility.exe setup with verysilent mode and add utilities in startup menu
Author Name    : Neha Gharge
SR.NO		   :
Date           : 30 Apr 2015
***********************************************************************************************/
bool RunUtililtyInSilentMode()
{
	__try
	{
		return RunUtililtyInSilentModeSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### VibraniumAluSrv::RunUtililtyInSilentMode::Exception", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : RunUtililtyInSilentModeSEH
Description    : Run utility.exe setup with verysilent mode and add utilities in startup menu
Author Name    : Neha Gharge
SR.NO		   :
Date           : 30 Apr 2015
***********************************************************************************************/
bool RunUtililtyInSilentModeSEH()
{
	try
	{

		CString csExePath, csCommandLine, csExePathExist;
		CWardWizOSversion		objOSVersionWrap;

		csExePathExist.Format(L"%s%s", g_szAVPath, L"VButility.exe");
		AddLogEntry(L">>> VibraniumUtility file is %s", csExePathExist, 0, true, FIRSTLEVEL);
		if (!PathFileExists(csExePathExist))
		{
			AddLogEntry(L"### VibraniumUtility file is not available in the appfolder", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		csExePath.Format(L"\"%s%s\"", g_szAVPath, L"VButility.exe");

		// issue- service interactive popup displaying on window7 32 bit and vista32 machine.
		// resolved by lalit kumawat 8-14-2015

		//csCommandLine.Format(L"%s", L"\/verysilent \/SUPPRESSMSGBOXES ");

		//Issue: 1083 If we update setup installed in Deutsch language after updating when we uninstall it,uninstaller is getting opened in English language.
		//Resolved by Nitin K. Date: 4th Dec 2015
		DWORD dwLangID = m_objwardwizLangManager.GetSelectedLanguage();
		CString csLanguage = L"";
		switch (dwLangID)
		{
		case ENGLISH:
			csLanguage += L"LANG=english";
			break;
		case HINDI:
			csLanguage += L"LANG=hindi";
			break;
		case GERMAN:
			csLanguage += L"LANG=german";
			break;
		case CHINESE:
			csLanguage += L"LANG=chinese";
			break;
		case SPANISH:
			csLanguage += L"LANG=spanish";
			break;
		case FRENCH:
			csLanguage += L"LANG=french";
			break;
		default:
			csLanguage += L"LANG=english";
			break;
		}
		csCommandLine.Format(L"%s%s", L"/verysilent /SUPPRESSMSGBOXES /", csLanguage);
		//On xp runas parameter never work It will not unregister the VBshellext.dll
		//So NUll parameter send.
		//DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
		//Neha Gharge Message box showing of register successful on reinstallation.
		switch (g_OSType)
		{
		case WINOS_XP:
		case WINOS_XP64:
			ShellExecute(NULL, NULL, csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		default:
			ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		}
		AddLogEntry(L">>> VibraniumUtility shell execute is done Exe Path: %s, CommandLine: %s", csExePathExist, csCommandLine, true, SECONDLEVEL);
		//UTILITY.exe getfilehash get failed.Because both shell execution and gethashfile run at the time.
		Sleep(13 * 1000);
		return true;

	}
	catch (...)
	{
		AddLogEntry(L"Exception in VibraniumAluSrv::RunUtililtyInSilentMode", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
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
*  Function Name  : SendMsgToShowReleaseNotesMsg()
*  Description    : It will download ReleaseNotes.ini file in the AppData Folder.
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date			  :	7th May-2015
****************************************************************************************************/
void SendMsgToShowReleaseNotesMsg()
{
	try
	{
		TCHAR	szALUDelIniFilePath[512] = { 0 };
		TCHAR	szIniFilePath[512] = { 0 };
		TCHAR   szValueData[512] = { 0 };
		DWORD	dwProductID = 0x00;
		DWORD	dwRelNotesShow = 0x00;
		CITinRegWrapper objReg;

		GetProductID(dwProductID);
		if (!dwProductID)
		{
			AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
			return;
		}

		switch (dwProductID)
		{
		case 1:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 2:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 3:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 4:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		case 5:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", g_szAllUserPath, g_csAppFolderName);
			break;
		}

		if (!PathFileExists(szIniFilePath))
		{
			AddLogEntry(L"### Functin is ShowReleaseNotesMsg.... ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
			return;
		}

		swprintf_s(szALUDelIniFilePath, _countof(szALUDelIniFilePath), L"%s\\%s\\%s", g_szAllUserPath, g_csAppFolderName, L"ALUDel.ini");

		if (PathFileExists(szALUDelIniFilePath) && g_bIsALUDeleted == FALSE)
		{
			//Restart is required to apply updates
			if (ShowReleaseNotesMsg(szIniFilePath))
			{
				dwRelNotesShow = 0x01;
				if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwRelNotesShow", dwRelNotesShow) != 0)
				{
					AddLogEntry(L"### Failed to Set Registry value of dwRelNotesShow to 1 in VibraniumALUSrv::ShowReleaseNotesMsg", 0, 0, true, SECONDLEVEL);
				}
			}
			else
			{
				dwRelNotesShow = 0x00;
				if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwRelNotesShow", dwRelNotesShow) != 0)
				{
					AddLogEntry(L"### Failed to Set Registry value of dwRelNotesShow to 1 in VibraniumALUSrv::ShowReleaseNotesMsg", 0, 0, true, SECONDLEVEL);
				}
			}

		}
		else
		{
			//Restart is not required after successful product update.
			if (ShowReleaseNotesMsg(szIniFilePath))
			{
				dwRelNotesShow = 0x01;
				if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwRelNotesShow", dwRelNotesShow) == 0)
				{
					if (g_bRequestFromUI)
					{
						SendMessage2UI(SHOW_RELEASE_NOTES, 2, 0, 0, true);
					}
					else
					{
						SendMessage2UI(SHOW_RELEASE_NOTES, 2, 1, 0, true);
					}
				}
				else
				{
					AddLogEntry(L"### Failed to Set Registry value of dwRelNotesShow to 1 in VibraniumALUSrv::ShowReleaseNotesMsg", 0, 0, true, SECONDLEVEL);
				}
			}
			else
			{
				dwRelNotesShow = 0x00;
				if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwRelNotesShow", dwRelNotesShow) != 0)
				{
					AddLogEntry(L"### Failed to Set Registry value of dwRelNotesShow to 1 in VibraniumALUSrv::ShowReleaseNotesMsg", 0, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::SendMsgToShowReleaseNotesMsg", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ShowReleaseNotesMsg()
*  Description    : It will download ReleaseNotes.ini file in the AppData Folder.
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date			  :	7th May-2015
****************************************************************************************************/
bool ShowReleaseNotesMsg(TCHAR	szIniFilePath[512])
{
	try
	{
		TCHAR   szValueData[512] = { 0 };
		TCHAR	szValueAppVersion[MAX_PATH] = { 0 };
		TCHAR	szValueDBVersion[MAX_PATH] = { 0 };
		DWORD	dwSizeAppVersion = sizeof(szValueAppVersion);
		DWORD	dwSizeDBVersion = sizeof(szValueDBVersion);
		DWORD   dwCount = 0x00, dwCountFeature = 0x00, dwCountBugsFixed = 0x00;
		int		iResAppVer = 0, iResDBVer = 0;
		CITinRegWrapper objReg;
		GetPrivateProfileString(L"ProductVersion", L"ProductVer", L"", szValueData, 511, szIniFilePath);
		if (!szValueData[0])
		{
			AddLogEntry(L"### Failed to get ProductVersion from (%s) in VibraniumALUSrv::ShowReleaseNotesMsg", 0, szIniFilePath, true, FIRSTLEVEL);
			return false;
		}

		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"AppVersion", szValueAppVersion, dwSizeAppVersion) == 0x00)
		{
			CString csCurrentVersion(szValueData);
			CString csValueAppVersion(szValueAppVersion);

			int iCurrentVersion[4] = { 0 }, iAppVersion[4] = { 0 };
			if ((ParseVersionString(iAppVersion, csValueAppVersion)) && (ParseVersionString(iCurrentVersion, csCurrentVersion)))
			{
				iResAppVer = CompareVersions(iAppVersion, iCurrentVersion);
			}
		}
		else
		{
			AddLogEntry(L"### Failed to get Registry Entry for AppVersion in VibraniumALUSrv::ShowReleaseNotesMsg", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		GetPrivateProfileString(L"DatabaseVersion", L"DatabaseVer", L"", szValueData, 511, szIniFilePath);
		if (!szValueData[0])
		{
			AddLogEntry(L"### Failed to get DatabaseVersion from (%s) in VibraniumALUSrv::ShowReleaseNotesMsg", 0, szIniFilePath, true, SECONDLEVEL);
			return false;
		}

		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"DataBaseVersion", szValueDBVersion, dwSizeDBVersion) == 0)
		{
			CString csCurrentDBVersion(szValueData);
			CString csValueDBVersion(szValueDBVersion);

			int iCurrentDBVersion[4] = { 0 }, iDBVersion[4] = { 0 };
			if ((ParseVersionString(iDBVersion, csValueDBVersion)) && (ParseVersionString(iCurrentDBVersion, csCurrentDBVersion)))
			{
				iResDBVer = CompareVersions(iDBVersion, iCurrentDBVersion);
			}
		}
		else
		{
			AddLogEntry(L"### Failed to get Registry Entry for DataBaseVersion in VibraniumALUSrv::ShowReleaseNotesMsg", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (iResAppVer == 0 && iResDBVer == 0)
		{
			return false;
		}

		dwCountFeature = GetPrivateProfileInt(L"AddedFeature", L"Count", 0, szIniFilePath);
		dwCountBugsFixed = GetPrivateProfileInt(L"EnhancedFunctionality", L"Count", 0, szIniFilePath);
		dwCount = dwCountFeature + dwCountBugsFixed;

		if (dwCount == 0x00)
		{
			AddLogEntry(L">>> No Release Information to display.", 0, 0, true, ZEROLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::ShowReleaseNotesMsg.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**************************************************************************************************** 
* Function Name	: ParseVersionString
* Description	: To parse CString version strings and store it in Array of integers.
* Author Name	: Varada Ikhar
* SR_NO			:
* Date			: 7th May 2015
* *************************************************************************************************** */
bool ParseVersionString(int iDigits[4], CString & csVersion)
{
	try
	{
		int iTokenPos = 0;
		csVersion.Insert(0, _T("."));
		CString csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
		int iVersion = _ttoi(csToken);
		int iSubVersion = 0;
		int iCount = 0;

		iDigits[iCount] = iVersion;
		iCount++;
		while ((!csToken.IsEmpty()) && (iCount <= 3))
		{
			csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
			iSubVersion = _ttoi(csToken);
			iDigits[iCount] = iSubVersion;
			iCount++;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::ParseVersionString", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CompareVersions
*  Description    : To compare two products versions.
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 7th May 2015
****************************************************************************************************/
int CompareVersions(int iVersion1[4], int iVersion2[4])
{
	try
	{
		for (int iIndex = 0; iIndex < 4; ++iIndex)
		{
			if (iVersion1[iIndex] < iVersion2[iIndex])
				return -1;
			if (iVersion1[iIndex] > iVersion2[iIndex])
				return 1;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::CompareVersions.", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : DeleteAllALtFilesFromProgramDataSEH
*  Description    : To delete _wwzalt file from programData which is read only.
*  Author Name    : Vilas & lalit kumawat
*  SR_NO		  :
*  Date           : 5-15-2015
****************************************************************************************************/
bool DeleteAllALtFilesFromProgramDataSEH(LPCTSTR lpFolPath)
{
	
	CFileFind finder;

	CString strWildcard(lpFolPath);

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

		TCHAR	szPath[512] = { 0 };

		wsprintf(szPath, L"%s", finder.GetFilePath());
		if (finder.IsDirectory() )
			continue;

		if (finder.IsReadOnly())
			DeleteFileForcefully(szPath);
	}

	finder.Close();

	return false;
}

/***************************************************************************************************
*  Function Name  : DeleteAllALtFilesFromProgramData
*  Description    : WrapperFunction to delete _wwzalt file from programData which is read only.
*  Author Name    : Vilas & lalit kumawat
*  SR_NO		  :
*  Date           : 5-15-2015
****************************************************************************************************/
bool DeleteAllALtFilesFromProgramData()
{
	bool bReturn = false;

	__try
	{
		TCHAR	szAllUserProgPath[512] = { 0 };

		swprintf_s(szAllUserProgPath, _countof(szAllUserProgPath), L"%s\\%s", g_szAllUserPath, g_csAppFolderName);

		bReturn = DeleteAllALtFilesFromProgramDataSEH(szAllUserProgPath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### VibraniumAluSrv::DeleteAllALtFilesFromProgramData::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : WaitToStartTrayThread()
*  Description    : wait to start tray
*  Author Name    : Neha
**  SR_NO		  : 
*  Date			  :	
****************************************************************************************************/
DWORD WaitToStartTrayThread(LPVOID lpParam)
{
	DWORD dwReturn = 0x00;

	__try
	{
		return WaitToStartTrayThreadSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### VibraniumAluSrv::WaitToStartTrayThread::Exception", 0, 0, true, SECONDLEVEL);
	}

	return 0x01;
}

DWORD WaitToStartTrayThreadSEH()
{
	try
	{
		CString csExePath, csCommandLine;
		csExePath.Format(L"%s", L"VBTRAY.EXE");

		DWORD					dwSessionId = 0x00;

		//Neha Gharge 1-4-2015 Splash Screen was not displayed at the time of restart.
		//csCommandLine.Format(L"%s", L"-SHOWSPSCRN");
		//Launch wardwiz tray application
		CExecuteProcess objEnumprocess;
		//Issue : 0000158: In Settings if we select Start-Up scan as Quick/Full Scan after restart/Shutdown when we start the PC the start-up scan is not working.
		//Resolved by: Nitin K.
		//objEnumprocess.StartProcessWithTokenExplorerWait(csExePath, csCommandLine, L"explorer.exe");

		dwSessionId = WTSGetActiveConsoleSessionId();

		HANDLE					hToken = NULL;

		BOOL bRet = FALSE;
		do
		{

			bRet = OpenProcessToken(objEnumprocess.GetExplorerProcessHandle(csExePath), TOKEN_ALL_ACCESS, &hToken);
			if (!bRet)
			{
				//CString csError;
				//csError.Format(L"%d", GetLastError());
				//AddLogEntry(L"### Failed to OpenProcessToken %s", csError, 0, true, SECONDLEVEL);;
				//return FALSE;

				Sleep(1000);
			}
		} while (!bRet);

		// lalit 3-5-2015 ,aluService getting start earlier to protection services.
		Sleep(500);
		CSecure64  objCSecure;
		objCSecure.RegisterProcessId(WLSRV_ID_SEVEN);  // Issue resolved: 0001288, duplicate ID's was present

		CScannerLoad	objCScanner;
		objCScanner.RegisterProcessId(WLSRV_ID_SEVEN);//WLSRV_ID_SEVEN to register service for process protection

		//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
		//Resolved By :  Nitin K.
		if (!WriteIntoRegistry())
		{
			AddLogEntry(L"### Failed to write app version no and database", 0, 0, true, SECONDLEVEL);
		}

		//As We dont have product updates so no need to run utiility.exe
		//In next version we will provide utility.exe too.
		return 0;

		////Till the tray lauch properly
		//Sleep(7 * 1000);

		//Varada Ikhar, Date: 8th May-2015
		//New Implementation : To Show 'Release Information' after successful product update.
		DWORD dwRelNoteShow = 0x00;
		bool bInstallUtility = false;
		bool bShowReleaseNotes = false;

		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwRelNotesShow", dwRelNoteShow) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwRelNotesShow in VibraniumALUSrv, KeyPath: %s", g_csProdRegKey, 0, true, FIRSTLEVEL);

			dwRelNoteShow = 0x01;
			if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwRelNotesShow", dwRelNoteShow) != 0)
			{
				AddLogEntry(L"### Failed to Set Registry value of dwRelNotesShow to 1 in VibraniumALUSrv::ShowReleaseNotesMsg, KeyPath: %s", g_csProdRegKey, 0, true, SECONDLEVEL);
			}

			bShowReleaseNotes = true;
			bInstallUtility = true;
		}
		else
		{
			if (dwRelNoteShow == 0x01)
			{
				bShowReleaseNotes = true;
				bInstallUtility = true;
			}
		}


		AddLogEntry(L">>> Before if (bInstallUtility)", 0, 0, true, FIRSTLEVEL);
		
		// issue- service interactive popup displaying on window7 32 bit and vista32 machine.
		// resolved by lalit kumawat 8-24-2015
		//Note: this solution just minimize the occurrence of this issue.
		if (bInstallUtility)
		{
			AddLogEntry(L">>> Before if (!LaunchTLB())", 0, 0, true, FIRSTLEVEL);
			
			//if (!LaunchTLB())
			//{
			//	AddLogEntry(L"### Failed to LaunchTLB", 0, 0, true, FIRSTLEVEL);
			//}

			AddLogEntry(L">>> Before RunUtililtyInSilentMode", 0, 0, true, FIRSTLEVEL);

			if (!RunUtililtyInSilentMode())
			{
				AddLogEntry(L"### Failed to RunUtililtyInSilentMode", 0, 0, true, FIRSTLEVEL);
			}

			AddLogEntry(L">>> After RunUtililtyInSilentMode", 0, 0, true, FIRSTLEVEL);
		}

		AddLogEntry(L">>> Before if (bShowReleaseNotes)", 0, 0, true, FIRSTLEVEL);

		//Ram:
		//Release Notes to display after successful update.
		/*if (bShowReleaseNotes)
		{
			SendMessage2UI(SHOW_RELEASE_NOTES, 2, 0, 0, true);
		}*/
	
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::WaitToStartTrayThread.", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SetAllALtFilesAttributeToNorml()
*  Description    : Set all files attributes to normal. Flag renamefile = false means setattributes to normal
					and renamefile = true , rename file.
*  Author Name    : Lalit
**  SR_NO		  :
*  Date			  :
****************************************************************************************************/
bool SetAllFileAttributeToNormal(LPCTSTR lpFolPath, LPCTSTR lpQuarntinePath, bool bRenameFile)
{
	try
	{
		CFileFind finder;

		CString strWildcard(lpFolPath);
		TCHAR  szAppPath[MAX_PATH] = { 0 };
		TCHAR  szQuarntinePath[MAX_PATH] = { 0 };
		CString csQuarntinePath(lpQuarntinePath);

		if (!lpQuarntinePath || !lpFolPath)
		{
			return false;
		}

		swprintf_s(szQuarntinePath, _countof(szQuarntinePath), L"%s", csQuarntinePath);
		//	csQuarntinePath = szQuarntinePath;

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");

		SetFileAttributes(lpFolPath, FILE_ATTRIBUTE_NORMAL);

		BOOL bWorking = finder.FindFile(strWildcard);


		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[512] = { 0 };

			wsprintf(szPath, L"%s", finder.GetFilePath());

			if (finder.IsDirectory())
			{
				SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);
				if (_wcsicmp(szPath, szQuarntinePath) == 0x00)
				{
					continue;
				}
				else
				{
					if (!bRenameFile)
					{
						SetAllFileAttributeToNormal(szPath, lpQuarntinePath, false);
					}
					else
					{
						SetAllFileAttributeToNormal(szPath, lpQuarntinePath, true);
					}
				}
			}
			if (!bRenameFile)
			{
				SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);
			}
			else
			{
				if (!CheckForRenameRequired(szPath))
				{
					continue;
				}

			}

		}

		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"Exception in VibraniumAluSrv::SetAllFileAttributeToNormal", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : SetAllALtFilesAttributeToNorml()
*  Description    : Set all files attributes to normal
*  Author Name    : Lalit
**  SR_NO		  :
*  Date			  :
****************************************************************************************************/
bool SetAllALtFilesAttributeToNorml()
{
	bool bReturn = false;

	__try
	{
		TCHAR	szProgPath[512] = { 0 };
		TCHAR	szQuarntinePath[512] = { 0 };

		swprintf_s(szProgPath, _countof(szProgPath), L"%s", g_szAVPath);

		swprintf_s(szQuarntinePath, _countof(szQuarntinePath), L"%s%s", g_szAVPath, L"QUARANTINE");

		bReturn = SetAllFileAttributeToNormal(szProgPath, szQuarntinePath , false);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### VibraniumAluSrv::SetAllFileAttributeToNormal::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : RenameFileFailedIn1940Version()
*  Description    : Rename file which get failed particularly in 1.9.4.0 version.
*  Author Name    : Neha
**  SR_NO		  :
*  Date			  :
****************************************************************************************************/
//1.9.4.0 version the file which are download first time not getting renamed. In 1.9.0.0 we create a file then copy
//But in 1.9.4.0 its just download file and replaced but failed to rename.
bool RenameFileFailedIn1940Version()
{
	bool bReturn = false;
	__try
	{
		TCHAR	szProgPath[512] = { 0 };
		TCHAR	szQuarntinePath[512] = { 0 };
		
		swprintf_s(szProgPath, _countof(szProgPath), L"%s", g_szAVPath);

		swprintf_s(szQuarntinePath, _countof(szQuarntinePath), L"%s%s", g_szAVPath, L"QUARANTINE");

		bReturn = SetAllFileAttributeToNormal(szProgPath, szQuarntinePath, true);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"Exception in VibraniumAluSrv::RenameFileFailedIn1940Version", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CheckForRenameRequired()
*  Description    : Check for file rename required
*  Author Name    : Neha
**  SR_NO		  :
*  Date			  :
****************************************************************************************************/
bool CheckForRenameRequired(CString csRenameFile)
{
	try
	{
		int iPos = 0;
		//csNewFilePath = with _WWIZALT and csOldFilePath = original name
		CString csNewFilePath, csOldFilePath;
		
		if (csRenameFile.Right(8) == L"_WWIZALT")
		{
			csNewFilePath.Format(L"%s", csRenameFile);
			csOldFilePath = csRenameFile.Tokenize(L"_", iPos);
			if (!PathFileExists(csOldFilePath))
			{
				if (PathFileExists(csNewFilePath))
				{
					_wrename(csNewFilePath, csOldFilePath);
					return true;
				}
			}
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in VibraniumAluSrv::CheckForRenameRequired", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : CheckScanLevel
Description    : To check scan level 1-> Only with wardwiz scanner
2-> with clam scanner and second level scaner is wardwiz scanner.
SR.NO			 :
Author Name    : Neha gharge
Date           : 1-19-2015
***********************************************************************************************/
DWORD CheckScanLevel()
{
	DWORD dwScanLevel = 0x00;
	try
	{
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwScanLevel", dwScanLevel) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CheckScanLevel", 0, 0, true, FIRSTLEVEL);;
			return 0x00;
		}

		switch (dwScanLevel)
		{
		case 0x01:
			return 0x01;
			break;
		case 0x02:
			return 0x02;
			break;
		default:
			return 0x00;
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in VibraniumAluSrv::CheckScanLevel", 0, 0, true, SECONDLEVEL);

	}
	return dwScanLevel;
}

/***********************************************************************************************
Function Name  : CheckOfflineReg
Description    : To check scan level
				 0-> Off
				 1-> On
SR.NO			 :
Author Name    : Neha gharge
Date           : 12-18-2015
***********************************************************************************************/
DWORD CheckOfflineReg()
{
	DWORD dwOfflineReg = 0x00;
	DWORD dwScanlevel = 0x00;
	try
	{
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwIsOffline", dwOfflineReg) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwIsOffline in CheckOfflineReg", 0, 0, true, FIRSTLEVEL);
			
			dwScanlevel = CheckScanLevel();
			if (dwScanlevel == 0x02)
			{
				dwOfflineReg = 0x01;
			}
			else
			{
				dwOfflineReg = 0x00;
			}

			if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwIsOffline", dwOfflineReg) != 0x00)
			{
				AddLogEntry(L"### Failed to Set Registry Entry for dwIsOffline in CheckOfflineReg", 0, 0, true, SECONDLEVEL);
				return 0x00;
			}
		}

		switch (dwOfflineReg)
		{
		case 0x01:
			dwOfflineReg = 0x01;
			break;
		default:
			dwOfflineReg = 0x00;
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in VibraniumAluSrv::CheckOfflineReg", 0, 0, true, SECONDLEVEL);
	}
	return dwOfflineReg;
}

/***********************************************************************************************
Function Name  : UnzipUsingShell32API
Description    : Function to Unzip file using Shell 32 COM API.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
DWORD UnzipUsingShell32API(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath)
{
	DWORD dwReturn = 0x00;

	__try
	{
		dwReturn = UnzipUsingShell32APISEH(lpszZipFilePath, lpszUnizipFilePath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x05;
		AddLogEntry(L"### Exception in UnzipUsingShell32API", 0, 0, true, SECONDLEVEL);
	}

	return dwReturn;
}

/***********************************************************************************************
Function Name  : UnzipUsingShell32APISEH
Description    : Structured Exception handling function which Extracts file using shellwin32API
Author Name    : Ram Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
DWORD UnzipUsingShell32APISEH(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath)
{
	DWORD dwReturn = 0x00;
	try
	{
		//Sanity check
		if (lpszZipFilePath == NULL || lpszUnizipFilePath == NULL)
		{
			dwReturn = 0x01;
			return dwReturn;
		}

		//To get name of zip file.
		TCHAR szZipFilePath[512] = { 0 };
		_tcscpy_s(szZipFilePath, 512, lpszZipFilePath);

		TCHAR szZipFileName[MAX_PATH] = { 0 };
		TCHAR *pZipTemp = wcsrchr(szZipFilePath, '\\');
		if (!pZipTemp)
		{
			dwReturn = 0x02;
			return dwReturn;
		}

		pZipTemp++;
		_tcscpy_s(szZipFileName, MAX_PATH, pZipTemp);

		TCHAR szUnZipFilePath[512] = { 0 };
		_tcscpy_s(szUnZipFilePath, 512, lpszUnizipFilePath);

		TCHAR szFilePath[2048] = { 0 };
		TCHAR szFileName[MAX_PATH] = { 0 };

		//Get here the path of Taget directory
		TCHAR *pUnZipTemp = wcsrchr(szUnZipFilePath, '\\');
		if (!pUnZipTemp)
		{
			dwReturn = 0x03;
			return dwReturn;
		}

		pUnZipTemp++;
		_tcscpy_s(szFileName, MAX_PATH, pUnZipTemp);
		
		pUnZipTemp--;
		*pUnZipTemp = '\0';
		_tcscpy_s(szFilePath, 2048, szUnZipFilePath);
		
		//Check here that target file exists
		if ((_waccess(szFilePath, 0)) == -1)
		{
			dwReturn = 0x04;
			return dwReturn;
		}

		CString csActualFileName = szZipFileName;
		csActualFileName = csActualFileName.Left(csActualFileName.GetLength() - 4);//Get here the actual file name.

		TCHAR szSourcePath[512] = { 0 };
		TCHAR szTargetPath[512] = { 0 };
		swprintf(szSourcePath, L"%s\\%s", szFilePath, csActualFileName);
		swprintf(szTargetPath, L"%s\\%s", szFilePath, szFileName);

		//Delete here if target path exists.
		if (PathFileExists(szTargetPath))
		{
			SetFileAttributes(szTargetPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szTargetPath);
		}

		//Unzip the file using Shellwin32 API
		HRESULT hr = ::CoInitialize(NULL);
		hr = UnzipToFolder(lpszZipFilePath, szFilePath);
		::CoUninitialize();

		if (FAILED(hr))
		{
			CString csError;
			csError.Format(L"Error: hr=0x%08X", hr);
			AddLogEntry(L"### Failed Function UnzipToFolder, %s", csError, 0, true, SECONDLEVEL);
			dwReturn = 0x05;
			return dwReturn;
		}

		//Rename the extracted file.
		SetFileAttributes(szSourcePath, FILE_ATTRIBUTE_NORMAL);
		if (_wrename(szSourcePath, szTargetPath) != 0)
		{
			AddLogEntry(L"### Failed to _wrename File, Source:[%s], Dest[%s] ", szSourcePath, szTargetPath, true, FIRSTLEVEL);
			dwReturn = 0x06;
			return dwReturn;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UnzipUsingShell32APISEH, ZipFilePath: %s, UnZipFilePath:%s", lpszZipFilePath, lpszUnizipFilePath, true, SECONDLEVEL);
		dwReturn = 0x07;
	}
	return dwReturn;
}

/***********************************************************************************************
Function Name  : UnzipToFolder
Description    : Unzip a zip file to the specified folder.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
HRESULT UnzipToFolder(PCWSTR pszZipFile, PCWSTR pszDestFolder)
{
	try
	{
		HRESULT hr;

		CComPtr<IShellDispatch> spISD;
		hr = spISD.CoCreateInstance(CLSID_Shell);
		if (FAILED(hr))
			return hr;

		CComVariant vtZipFile(pszZipFile);
		CComPtr<Folder> spZipFile;
		hr = spISD->NameSpace(vtZipFile, &spZipFile);
		if (FAILED(hr))
			return hr;

		CComVariant vtDestFolder(pszDestFolder);
		CComPtr<Folder> spDestination;
		hr = spISD->NameSpace(vtDestFolder, &spDestination);
		if (FAILED(hr))
			return hr;
		if (!spDestination)
			return E_POINTER;

		CComPtr<FolderItems> spFilesInside;
		hr = spZipFile->Items(&spFilesInside);
		if (FAILED(hr))
			return hr;

		CComPtr<IDispatch> spDispItem;
		hr = spFilesInside.QueryInterface(&spDispItem);
		if (FAILED(hr))
			return hr;

		CComVariant vtItem(spDispItem);
		CComVariant vtOptions(FOF_NO_UI);
		hr = spDestination->CopyHere(vtItem, vtOptions);
		if (FAILED(hr))
			return hr;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UnzipToFolder, SourceFileName: %s, DestPath: %s", pszZipFile, pszDestFolder, true, SECONDLEVEL);
		return S_FALSE;
	}
	return S_OK;
}

/***************************************************************************
Function Name  : LaunchTLBSEH()
Description    : launch troubleshooter EXE
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
bool LaunchTLBSEH()
{
	try
	{
		CString csTLBFilePath;
		csTLBFilePath = GetWardWizPathFromRegistry();
		csTLBFilePath += L"VBTLB.EXE";
		CExecuteProcess			objEnumprocess;
		objEnumprocess.StartProcessWithTokenExplorerWait(csTLBFilePath, L"", L"explorer.exe");
		//ShellExecute(NULL, L"open", csTLBFilePath, NULL, NULL, SW_SHOW);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumAluSrv::LaunchTLB", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
Function Name  : LaunchTLB()
Description    : launch troubleshooter EXE
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
bool LaunchTLB()
{
	__try
	{
		return LaunchTLBSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in VibraniumAluSrv::LaunchTLB", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : HandleDriverReplaceCaseOnWin10
*  Description    : Function which handles a special case on Win10, the design change by windows 10 is
					Driver files will not be replaced when they are loaded.
					This is only case to handle from version 2.2 to 2.3
*  Author Name    : Ramkrushna Shelke
*  Date			  :	3-Mar-2017
****************************************************************************************************/
bool HandleDriverReplaceCaseOnWin10()
{
	bool bReturn = false;
	try
	{
		for (int j = 0; g_csaDriverServices[j].GetLength() != 0; j++)
		{
			CString csDriverSvcName = g_csaDriverServices[j];
			
			TCHAR szFilePath[MAX_PATH] = { 0 };
			swprintf_s(szFilePath, _countof(szFilePath), L"%s%s\\%s_%s", g_szAVPath, DRIVERS_FOLDER_NAME, csDriverSvcName, L"WWIZALT");
			if (PathFileExists(szFilePath))
			{
				CString csNewFileName;
				csNewFileName.Format(L"%s%s\\%s", g_szAVPath, DRIVERS_FOLDER_NAME, csDriverSvcName);
				if (PathFileExists(csNewFileName))
				{
					TCHAR szFile2Del[MAX_PATH] = { 0 };
					swprintf_s(szFile2Del, _countof(szFilePath), L"%s%s\\%s_%s", g_szAVPath, DRIVERS_FOLDER_NAME, csDriverSvcName, L"WWIZALTDEL");
					if (PathFileExists(szFile2Del))
					{
						SetFileAttributes(szFile2Del, FILE_ATTRIBUTE_NORMAL);
						DeleteFile(szFile2Del);
					}

					if (_wrename(csNewFileName, szFile2Del) != 0)
					{
						AddLogEntry(L"### [OPR: RENAME] failed in HandleDriverReplaceCaseOnWin10: OLDNAME: [%s], NEWNAME:[%s]", csNewFileName, szFile2Del, true, SECONDLEVEL);
						return bReturn;
					}

					if (_wrename(szFilePath, csNewFileName) != 0)
					{
						AddLogEntry(L"### [OPR: RENAME] failed in HandleDriverReplaceCaseOnWin10: OLDNAME: [%s], NEWNAME:[%s]", szFilePath, csNewFileName, true, SECONDLEVEL);
						return bReturn;
					}

					//Add here entry to remove file on restart.
					AddEntryToALUDelIni(szFile2Del, csDriverSvcName.GetBuffer());

					bReturn = true;
				}
				else
				{
					TCHAR szFile2Ren[MAX_PATH] = { 0 };
					swprintf_s(szFile2Ren, _countof(szFilePath), L"%s%s\\%s_%s", g_szAVPath, DRIVERS_FOLDER_NAME, csDriverSvcName, L"WWIZALTSRV");
					if (PathFileExists(szFile2Ren))
					{
						if (_wrename(szFile2Ren, csNewFileName) != 0)
						{
							AddLogEntry(L"### [OPR: RENAME] failed in HandleDriverReplaceCaseOnWin10: OLDNAME: [%s], NEWNAME:[%s]", szFile2Ren, csNewFileName, true, SECONDLEVEL);
							return bReturn;
						}
					}
				}
			}
			else
			{
				CString csNewFileName;
				csNewFileName.Format(L"%s%s\\%s", g_szAVPath, DRIVERS_FOLDER_NAME, csDriverSvcName);
				if (PathFileExists(csNewFileName))
				{
					TCHAR szFile2Ren[MAX_PATH] = { 0 };
					swprintf_s(szFile2Ren, _countof(szFilePath), L"%s%s\\%s_%s", g_szAVPath, DRIVERS_FOLDER_NAME, csDriverSvcName, L"WWIZALTDEL");
					if (PathFileExists(szFile2Ren))
					{
						if (!DeleteFile(szFile2Ren))
						{
							//Add here entry to remove file on restart.
							AddEntryToALUDelIni(szFile2Ren, csDriverSvcName.GetBuffer());
						}
						bReturn = true;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in HandleDriverReplaceCaseOnWin10", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : This function is  called for Set the Registry Key Value for Menu Items using Service.
*  Author Name    : Amol J.
*  Date			  :	30-June-2017
****************************************************************************************************/
bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
	
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;

		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in VibraniumAluSrv : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in VibraniumAluSrv : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in VibraniumALUSrv : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);

	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SendData2Service
*  Description    :	Enable/Disable active scan feature for.
*  Author Name    : Amol J.
*  Date			  :	30-June-2017
****************************************************************************************************/
bool SendData2Service(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam, LPTSTR lpszSecondParam, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = iMessage;
		szPipeData.dwValue = dwValue;

		if (lpszFirstParam != NULL)
		{
			wcscpy_s(szPipeData.szFirstParam, lpszFirstParam);
		}

		if (lpszSecondParam != NULL)
		{
			wcscpy_s(szPipeData.szSecondParam, lpszSecondParam);
		}

		szPipeData.dwSecondValue = dwSeondValue;

		CISpyCommunicator objCom(SERVICE_SERVER, bWait, 3);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			CString csMessage;
			csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
			AddLogEntry(L"### Failed to set data in VibraniumALUSrv::SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csMessage;
				csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
				AddLogEntry(L"### Failed to set data in VibraniumALUSrv::SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in VibraniumALUSrv : SendData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SendData2UI
*  Description    : Send Data to UI.
*  Author Name    : Amol J.
*  Date			  :	30-June-2017
****************************************************************************************************/
bool SendData2UI(int iMessageInfo, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = iMessageInfo;

		bool bUIExists = true;
		CISpyCommunicator objComUI(UI_SERVER);
		if (!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			bUIExists = false;
			AddLogEntry(L"### Failed to send Data in VibraniumALUSrv::SendData2UI", 0, 0, true, SECONDLEVEL);
		}

		if (bWait && bUIExists)
		{
			if (!objComUI.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in VibraniumALUSrv::SendData2UI", 0, 0, true, SECONDLEVEL);
			}
			if (szPipeData.dwValue == 0)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in VibraniumALUSrv : SendData2UI", 0, 0, true, SECONDLEVEL);

	}
	return true;
}