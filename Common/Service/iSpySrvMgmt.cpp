#include "stdafx.h"
#include "iSpySrvMgmt.h"
#include "WardWizDumpCreater.h"
CISpySrvMgmt::CISpySrvMgmt()
{
}
CISpySrvMgmt::~CISpySrvMgmt()
{
}


DWORD CISpySrvMgmt::InstallService( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName, TCHAR *pszSrvPath)
{
	SC_HANDLE				hService	= NULL ;
    SC_HANDLE				hSCManager	= NULL ;
	SERVICE_DESCRIPTION		SrvDesc		= {L"Vibranium Service module"} ;

	TCHAR	szSrvPath[512] = {0} ;
	DWORD	dwRet = 0x00, dwLastError=0x00 ;

	DWORD	dwSrvStartType = 0xFFFFFFFF ;

	__try
	{
		if( !PathFileExists( pszSrvPath ) )
		{
			dwRet = 0x01 ;
			__leave ;
		}

		if( (!pszSrvName[0]) || (!pszSrvDisplayName[0]) )
		{
			dwRet = 0x01 ;
			__leave ;
		}

		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( !hSCManager )
		{
			//AddToLog( "OpenSCManager Failed" ) ;
			dwRet = 0x02 ;
			__leave ;
		}

		hService = CreateService(	hSCManager, pszSrvName, pszSrvDisplayName, SERVICE_ALL_ACCESS,
									SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
									SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
									pszSrvPath, NULL, NULL, L"", NULL, NULL) ;

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

DWORD CISpySrvMgmt::UnInstallService( TCHAR *pszSrvName )
{
	DWORD	dwRetValue = 0x00 ;


	SC_HANDLE	schSCManager = NULL ;
    SC_HANDLE	schService = NULL ;
	DWORD		dwServiceStartType = 0xFFFFFFFF ;

	SERVICE_STATUS		ServiceStatus = {0} ;

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
			dwRetValue = GetLastError();
			__leave ;
		}

		Sleep( 200 ) ;

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
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

DWORD CISpySrvMgmt::StartServiceManually( TCHAR *pszSrvName )
{
	DWORD		dwRet = 0x00 ;

	SC_HANDLE	schSCManager = NULL ;
	SC_HANDLE	schService = NULL ;

	DWORD		dwSrvStartType = 0xFFFFFFFF ;
	DWORD		dwLastError = 0x00 ;

	__try
	{
		schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( schSCManager == NULL )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		schService = OpenService( schSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
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

DWORD CISpySrvMgmt::StopServiceManually( TCHAR *pszSrvName )
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
			
			dwRetValue = GetLastError();
			__leave ;
		}

		Sleep( 200 ) ;

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
		if( (ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) ||
			(ServiceStatus.dwCurrentState==SERVICE_STOPPED) )
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

DWORD CISpySrvMgmt::SetServiceFailureAction( SC_HANDLE hService )
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
		failActions[0].Delay = 1000;
		failActions[1].Type = SC_ACTION_RESTART ;
		failActions[1].Delay = 1000;
		failActions[2].Type = SC_ACTION_RESTART ;
		failActions[2].Delay = 1000;

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

DWORD CISpySrvMgmt::QueryServiceStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus)
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


DWORD CISpySrvMgmt::QueryServiceStartStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus)
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

DWORD CISpySrvMgmt::CreateUserService(TCHAR *pszSrvName, TCHAR *pszSrvDisplayName, TCHAR *pszSrvPath)
{
	SC_HANDLE				hService = NULL;
	SC_HANDLE				hSCManager = NULL;
	SERVICE_DESCRIPTION		SrvDesc = { L"Vibranium Service module" };

	TCHAR	szSrvPath[512] = { 0 };
	DWORD	dwRet = 0x00, dwLastError = 0x00;

	DWORD	dwSrvStartType = 0xFFFFFFFF;

	__try
	{
		if (!PathFileExists(pszSrvPath))
		{
			dwRet = 0x01;
			__leave;
		}

		if ((!pszSrvName[0]) || (!pszSrvDisplayName[0]))
		{
			dwRet = 0x01;
			__leave;
		}

		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!hSCManager)
		{
			//AddToLog( "OpenSCManager Failed" ) ;
			dwRet = 0x02;
			__leave;
		}

		hService = CreateService(hSCManager, pszSrvName, pszSrvDisplayName, SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
			SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
			pszSrvPath, NULL, NULL, L"", NULL, NULL);

		dwLastError = GetLastError();
		if (!hService)
		{
			hService = OpenService(hSCManager, pszSrvName, SERVICE_ALL_ACCESS);
			if (!hService)
			{
				dwRet = 0x03;
				__leave;
			}
			else
			{

				SERVICE_STATUS	ServiceStatus = { 0 };

				QueryServiceStatus(hService, &dwSrvStartType);

				if ((dwSrvStartType == SERVICE_DISABLED) ||
					(dwSrvStartType == SERVICE_DEMAND_START))
				{
					if (!ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
						SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
						NULL, NULL, NULL))
					{
						dwRet = 0x04;
						__leave;
					}
					Sleep(100);
				}

				dwSrvStartType = 0xFFFFFFFF;
				QueryServiceStartStatus(hService, &dwSrvStartType);

				if ((dwSrvStartType == SERVICE_RUNNING) ||
					(dwSrvStartType == SERVICE_START_PENDING))
				{
					ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);
					Sleep(100);
				}

				dwSrvStartType = 0xFFFFFFFF;
				QueryServiceStartStatus(hService, &dwSrvStartType);
				if ((dwSrvStartType == SERVICE_STOPPED) || (dwSrvStartType == SERVICE_STOP_PENDING) ||
					(dwSrvStartType == SERVICE_PAUSED) || (dwSrvStartType == SERVICE_PAUSE_PENDING) ||
					(dwSrvStartType == SERVICE_CONTINUE_PENDING))
				{
					dwRet = 0x00;
					Sleep(100);
				}
				else
					dwRet = 0x05;
			}

		}
		else
		{
			dwRet = 0x00;
			__leave;
		}

	}
	__finally
	{

		if (hService)
		{
			dwLastError = GetLastError();
			if (dwLastError != ERROR_SERVICE_EXISTS)
			{
				ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &SrvDesc);
				SetServiceFailureAction(hService);
			}

			CloseServiceHandle(hService);
			hService = NULL;
		}

		if (hSCManager)
		{
			CloseServiceHandle(hSCManager);
			hSCManager = NULL;
		}

	}

	return dwRet;
}

//
DWORD CISpySrvMgmt::InstallService(PWSTR pszSrvName,
	PWSTR pszSrvDisplayName,
	PWSTR pszSrvPath,
	DWORD dwStartType,
	PWSTR pszDependencies,
	PWSTR pszAccount,
	PWSTR pszPassword)
{
	SC_HANDLE				hService = NULL;
	SC_HANDLE				hSCManager = NULL;
	SERVICE_DESCRIPTION		SrvDesc = { L"Vibranium Service module" };

	TCHAR	szSrvPath[512] = { 0 };
	DWORD	dwRet = 0x00, dwLastError = 0x00;

	DWORD	dwSrvStartType = 0xFFFFFFFF;

	__try
	{
		if (!PathFileExists(pszSrvPath))
		{
			dwRet = 0x01;
			__leave;
		}

		if ((!pszSrvName[0]) || (!pszSrvDisplayName[0]))
		{
			dwRet = 0x01;
			__leave;
		}

		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
			SC_MANAGER_CREATE_SERVICE);
		if (!hSCManager)
		{
			//AddToLog( "OpenSCManager Failed" ) ;
			dwRet = 0x02;
			__leave;
		}

		// Install the service into SCM by calling CreateService
		hService = CreateService(
			hSCManager,                   // SCManager database
			pszSrvName,                 // Name of service
			pszSrvDisplayName,                 // Name to display
			SERVICE_QUERY_STATUS,           // Desired access
			SERVICE_WIN32_OWN_PROCESS,      // Service type
			dwStartType,                    // Service start type
			SERVICE_ERROR_NORMAL,           // Error control type
			pszSrvPath,                         // Service's binary
			NULL,                           // No load ordering group
			NULL,                           // No tag identifier
			pszDependencies,                // Dependencies
			pszAccount,                     // Service running account
			pszPassword                     // Password of the account
			);

		dwLastError = GetLastError();
		if (!hService)
		{
			hService = OpenService(hSCManager, pszSrvName, SERVICE_ALL_ACCESS);
			if (!hService)
			{
				dwRet = 0x03;
				__leave;
			}
			else
			{
				SERVICE_STATUS	ServiceStatus = { 0 };

				QueryServiceStatus(hService, &dwSrvStartType);

				if ((dwSrvStartType == SERVICE_DISABLED) ||
					(dwSrvStartType == SERVICE_DEMAND_START))
				{
					if (!ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
						SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
						NULL, NULL, NULL))
					{
						dwRet = 0x04;
						__leave;
					}
					Sleep(100);
				}

				dwSrvStartType = 0xFFFFFFFF;
				QueryServiceStartStatus(hService, &dwSrvStartType);

				if ((dwSrvStartType == SERVICE_RUNNING) ||
					(dwSrvStartType == SERVICE_START_PENDING))
				{
					ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);
					Sleep(100);
				}

				dwSrvStartType = 0xFFFFFFFF;
				QueryServiceStartStatus(hService, &dwSrvStartType);
				if ((dwSrvStartType == SERVICE_STOPPED) || (dwSrvStartType == SERVICE_STOP_PENDING) ||
					(dwSrvStartType == SERVICE_PAUSED) || (dwSrvStartType == SERVICE_PAUSE_PENDING) ||
					(dwSrvStartType == SERVICE_CONTINUE_PENDING))
				{
					dwRet = 0x00;
					Sleep(100);
				}
				else
					dwRet = 0x05;
			}

		}
		else
		{
			dwRet = 0x00;
			__leave;
		}

	}
	__finally
	{

		if (hService)
		{
			dwLastError = GetLastError();
			if (dwLastError != ERROR_SERVICE_EXISTS)
			{
				ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &SrvDesc);
				SetServiceFailureAction(hService);
			}

			CloseServiceHandle(hService);
			hService = NULL;
		}

		if (hSCManager)
		{
			CloseServiceHandle(hSCManager);
			hSCManager = NULL;
		}
	}
	return dwRet;
}