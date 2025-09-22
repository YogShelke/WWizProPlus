/********************************************************************************************************** 
   Program Name          : ITinEnumDriversAPI.cpp
   Description           : This class enumerates the files using WINAPI & and Using Kernel32.dll function
						   compares both the results for getting hidden rootkits.
   Author Name           : Vilas Suvarnakar                                                                                
   Date Of Creation      : 7/24/2014
   Version No            : 1.0.0
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description
***********************************************************************************************************/

#include "stdafx.h" 
#include "ITinEnumDriversAPI.h"
#include "ITinDriverInfo.h"

/***************************************************************************
  Function Name  : EnumerateFolderByAPIs
  Description    : Constructor
  Author Name    : Vilas Suvarnakar
  S.R. No        : 
  Date           : 7/24/2014
****************************************************************************/
CITinEnumDriversAPI::CITinEnumDriversAPI()
{
	ZeroMemory(szAllUserDir, sizeof(szAllUserDir) ) ;
	ZeroMemory(szComnProgFilesDir, sizeof(szAllUserDir) ) ;
	ZeroMemory(szProgFilesDir, sizeof(szAllUserDir) ) ;
	ZeroMemory(szWinDir, sizeof(szAllUserDir) ) ;
	ZeroMemory(szTempDir, sizeof(szAllUserDir) ) ;
	ZeroMemory(szPublicDir, sizeof(szAllUserDir) ) ;
	ZeroMemory(szUserProfileDir, sizeof(szAllUserDir) ) ;
	ZeroMemory(szSystem32Dir, sizeof(szAllUserDir) ) ;
	ZeroMemory(szProgramDirX86, sizeof(szProgramDirX86) ) ;
	
}

/***************************************************************************
  Function Name  : EnumerateFolderByAPIs
  Description    : Destructor
  Author Name    : Vilas Suvarnakar
  S.R. No        : 
  Date           : 7/24/2014
****************************************************************************/
CITinEnumDriversAPI::~CITinEnumDriversAPI()
{
	m_hDLL = NULL ;
}

/***************************************************************************
  Function Name  : GetInitialValues
  Description    : Reterive all the Initial Values.
  Author Name    : Vilas Suvarnakar
  S.R. No        : WRDWIZRKSCNDLL_058
  Date           : 7/24/2014
****************************************************************************/
void CITinEnumDriversAPI::GetInitialValues()
{
	GetEnvironmentVariable( TEXT("ALLUSERSPROFILE"), szAllUserDir, 255 ) ;
	GetEnvironmentVariable( TEXT("CommonProgramFiles"), szComnProgFilesDir, 255 ) ;
	GetEnvironmentVariable( TEXT("ProgramFiles"), szProgFilesDir, 255 ) ;
	GetEnvironmentVariable( TEXT("SystemRoot"), szWinDir, 255 ) ;
	GetEnvironmentVariable( TEXT("TEMP"), szTempDir, 255 ) ;
	GetEnvironmentVariable( TEXT("PUBLIC"), szPublicDir, 255 ) ;
	GetEnvironmentVariable( TEXT("USERPROFILE"), szUserProfileDir, 255 ) ;

	if( m_bIsWow64 )
		GetEnvironmentVariable(TEXT("PROGRAMFILES(X86)"), szProgramDirX86, 255 ) ;
	if( szProgramDirX86[0] )
	{
		_wcsupr_s( szProgramDirX86, sizeof(szProgramDirX86)) ;
	}

	if( szAllUserDir[0] )
	{
		_wcsupr_s( szAllUserDir , sizeof(szAllUserDir)) ;
	}
	if( szComnProgFilesDir[0] )
		_wcsupr_s( szComnProgFilesDir, sizeof(szComnProgFilesDir)) ;
	if( szProgFilesDir[0] )
		_wcsupr_s( szProgFilesDir, sizeof(szProgFilesDir)) ;
	if( szWinDir[0] )
		_wcsupr_s( szWinDir, sizeof(szWinDir)) ;
	if( szTempDir[0] )
		_wcsupr_s( szTempDir, sizeof(szTempDir)) ;
	if( szPublicDir[0] )
		_wcsupr_s( szPublicDir, sizeof(szPublicDir)) ;
	if( szUserProfileDir[0] )
		_wcsupr_s( szUserProfileDir, sizeof(szUserProfileDir)) ;
	if( szWinDir[0] )
		wsprintf(szSystem32Dir, L"%s\\SYSTEM32", szWinDir ) ;
}

/***************************************************************************
  Function Name  : GetServicePath
  Description    : Function which get the full path as a input from shortest path like
				   Program Files, TEMP folder path, Windows Directory
  Author Name    : Vilas Suvarnakar
  S.R. No        : WRDWIZRKSCNDLL_059
  Date           : 7/24/2014
****************************************************************************/
bool CITinEnumDriversAPI::GetServicePath( TCHAR *pShortPath, TCHAR *pNormalPath )
{
	TCHAR	szPath[1024] = {0} ;
	TCHAR	*pTemp = NULL ;


	if( memcmp(pShortPath, L"\\??\\", 4) == 0 )
		wcscpy(szPath, &pShortPath[4] ) ;
	else
		wcscpy(szPath, pShortPath ) ;

	_wcsupr(szPath);

	pTemp = wcsstr( szPath, L"SYSTEM32\\" ) ;
	if( pTemp )
	{
		pTemp += wcslen(L"SYSTEM32\\") ;
		wsprintf(pNormalPath, L"%s\\%s", szSystem32Dir, pTemp ) ;

		return true ;
	}

	pTemp = wcsstr( szPath, szProgFilesDir ) ;
	if( pTemp )
	{
		pTemp += wcslen(szProgFilesDir)*sizeof(TCHAR) ;
		wsprintf(pNormalPath, L"%s%s", szProgFilesDir, pTemp ) ;

		return true ;
	}

	pTemp = wcsstr( szPath, szComnProgFilesDir ) ;
	if( pTemp )
	{
		pTemp += wcslen(szComnProgFilesDir)*sizeof(TCHAR) ;
		wsprintf(pNormalPath, L"%s%s", szComnProgFilesDir, pTemp ) ;

		return true ;
	}


	pTemp = wcsstr( szPath, szSystem32Dir ) ;
	if( pTemp )
	{
		pTemp += wcslen(szSystem32Dir)*sizeof(TCHAR) ;
		wsprintf(pNormalPath, L"%s%s", szSystem32Dir, pTemp ) ;

		return true ;
	}

	pTemp = wcsstr( szPath, szTempDir ) ;
	if( pTemp )
	{
		pTemp += wcslen(szTempDir)*sizeof(TCHAR) ;
		wsprintf(pNormalPath, L"%s%s", szTempDir, pTemp ) ;

		return true ;
	}

	pTemp = wcsstr( szPath, szAllUserDir ) ;
	if( pTemp )
	{
		pTemp += wcslen(szAllUserDir)*sizeof(TCHAR) ;
		wsprintf(pNormalPath, L"%s%s", szAllUserDir, pTemp ) ;

		return true ;
	}

	pTemp = wcsstr( szPath, szUserProfileDir ) ;
	if( pTemp )
	{
		pTemp += wcslen(szUserProfileDir)*sizeof(TCHAR) ;
		wsprintf(pNormalPath, L"%s%s", szUserProfileDir, pTemp ) ;

		return true ;
	}

	pTemp = wcsstr( szPath, szWinDir ) ;
	if( pTemp )
	{
		pTemp += wcslen(szWinDir)*sizeof(TCHAR) ;
		wsprintf(pNormalPath, L"%s%s", szWinDir, pTemp ) ;

		return true ;
	}

	return false ;
}

/***************************************************************************
  Function Name  : EnumerateDriverServicesThrouthAPI
  Description    : Enumerate the Driver Services through out the API.
  Author Name    : Vilas Suvarnakar
  S.R. No        : WRDWIZRKSCNDLL_060
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinEnumDriversAPI::EnumerateDriverServicesThrouthAPI( ) 
{
	SC_HANDLE	hSvcCtlMgr = NULL ;
	SC_HANDLE	hService = NULL;

	DWORD		dwRet = 0x00, dw_Error = 0x00, i ;
	DWORD		dwSize = 0x00 ;
	DWORD		dwBytesNeeded = 0x00 ;
	DWORD		dwServiceCount = 0x00 ;

	ENUM_SERVICE_STATUS		struct_ServiceStatus = {0} ;
	ENUM_SERVICE_STATUS		*lpServiceStatus = NULL ;
	LPQUERY_SERVICE_CONFIG	lpQSCBuf = NULL ;

	//TCHAR					szNormalPath[256] = {0} ;
	//TCHAR					szServicesCount[256] = {0} ;
	//ITINDRIVERINFO			sDriverInfo = {0} ;
	__try
	{
		AddLogEntry(L">>> Inside EnumerateDriverServicesThrouthAPI", 0, 0, true, ZEROLEVEL);
		hSvcCtlMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT) ;
		if( !hSvcCtlMgr )
		{
			AddLogEntry(L"### retuning while OpenSCManager 0x01", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		EnumServicesStatus(	hSvcCtlMgr, SERVICE_DRIVER, SERVICE_STATE_ALL, &struct_ServiceStatus, dwSize,
							&dwBytesNeeded, &dwServiceCount, NULL) ;
		dw_Error = GetLastError() ;

		if( dw_Error == ERROR_MORE_DATA )
		{
			dwSize = dwBytesNeeded + sizeof(ENUM_SERVICE_STATUS) ;
			lpServiceStatus = new ENUM_SERVICE_STATUS [dwSize] ;
			if( lpServiceStatus )
			{
				ZeroMemory(lpServiceStatus, dwSize ) ;
				EnumServicesStatus(	hSvcCtlMgr, SERVICE_DRIVER, SERVICE_STATE_ALL, lpServiceStatus, dwSize,
									&dwBytesNeeded, &dwServiceCount, NULL) ;
				AddLogEntry(L"### retuning while EnumServicesStatus", 0, 0, true, ZEROLEVEL);
			}
		}

		
		AddLogEntry(L"### retuning while EnumServicesStatus", 0, 0, true, ZEROLEVEL);

		if( !dwServiceCount )
		{
			AddLogEntry(L"### retuning while EnumServicesStatus 0x02", 0, 0, true, ZEROLEVEL);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		lpQSCBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, 4096);
		if( !lpQSCBuf )
		{
			AddLogEntry(L"### retuning while EnumServicesStatus 0x03", 0, 0, true, SECONDLEVEL);
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		//swprintf(szServicesCount, L">>> Services Count: %d", dwServiceCount);
		//AddLogEntry(szServicesCount, 0, 0, true, ZEROLEVEL);
	
		for(i=0; i < dwServiceCount; i++ )
		{
			//swprintf(szServicesCount, L">>> Count: %d", i);
			//AddLogEntry(szServicesCount, 0, 0, true, ZEROLEVEL);

			hService = NULL ;
			hService = OpenService(hSvcCtlMgr, lpServiceStatus[i].lpServiceName, SERVICE_QUERY_CONFIG) ;
			if( !hService )
			{
				continue ;
			}

			ZeroMemory(lpQSCBuf, 4096 ) ;
			if( !QueryServiceConfig(hService, lpQSCBuf, 4096, &dwBytesNeeded) )
			{
				AddLogEntry(L"### failed in QueryServiceConfig", 0, 0, true, ZEROLEVEL);
				CloseServiceHandle( hService ) ;
				continue ; 
			}

			if( lpQSCBuf->lpBinaryPathName[0] )
			{
				//We need this enumeration when actual rotkit scanning is on
				//ZeroMemory(&sDriverInfo, sizeof(sDriverInfo) ) ;
				//wcscpy(sDriverInfo.szDriverDisplayName, lpQSCBuf->lpDisplayName ) ;
				//wcscpy(sDriverInfo.szDriverPath, lpQSCBuf->lpBinaryPathName ) ;
				//vDriverInfo.push_back( sDriverInfo ) ;
			}
			AddLogEntry(L">>> Next Service", 0, 0, true, ZEROLEVEL);
		}
		AddLogEntry(L">>> Outside for loop", 0, 0, true, ZEROLEVEL);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumEnumDriversAPI::EnumerateDriverServicesThrouthAPI", 0, 0, true, SECONDLEVEL);
	}

	AddLogEntry(L">>> At cleanup", 0, 0, true, ZEROLEVEL);
Cleanup:
	if( lpQSCBuf )
	{
		LocalFree( lpQSCBuf ) ;
		lpQSCBuf = NULL ;
	}

	AddLogEntry(L">>> At cleanup1", 0, 0, true, ZEROLEVEL);
	if( lpServiceStatus != NULL)
	{
		delete []lpServiceStatus ;
		lpServiceStatus = NULL ;
	}

	AddLogEntry(L">>> At cleanup2", 0, 0, true, ZEROLEVEL);

	if( hSvcCtlMgr != NULL)
	{
		CloseServiceHandle( hSvcCtlMgr ) ;
		hSvcCtlMgr = NULL ;
	}

	AddLogEntry(L">>> At cleanup3", 0, 0, true, ZEROLEVEL);
	return dwRet ;
}