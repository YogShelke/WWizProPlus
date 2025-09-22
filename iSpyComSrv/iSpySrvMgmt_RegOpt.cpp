#include "stdafx.h"
#include "iSpySrvMgmt_RegOpt.h"
#include "WrdWizSystemInfo.h"
#include <shlwapi.h>
#include <tchar.h>


iSpySrvMgmt_RegOpt::iSpySrvMgmt_RegOpt():
	m_RegOpt_MemMap_Server_Obj(REGISTRYOPTIMIZER)
{
	m_RegOpt_MemMap_Server_Obj.CreateServerMemoryMappedFile() ;
	InitializeVariables();
}

iSpySrvMgmt_RegOpt::~iSpySrvMgmt_RegOpt()
{
}

bool iSpySrvMgmt_RegOpt::InitializeVariables()
{
	__try
	{

		m_bIsx64 = false;

		m_bIsWow64 = FALSE ;
		bVistaOnward = false ;

		dwPercentage = 0x00 ;

		dwActiveXEntries = dwUnInstallEntries = dwFontEntries =
		dwSharedDLLs = dwAppPathEntries = dwHelpFilesEntries =
		dwStartupRepairedEntries = dwServicesEntries =	
		dwExtensionEntries = dwRootKitEntries =	dwRogueEntries =
		dwWormEntries =	dwSpywareEntries = dwAdwareEntries =
		dwKeyLoggerEntries = dwBHOEntries = dwExplorerEntries =	
		dwIEEntries = 0x00 ;

		memset(&RegOpt_ScanOpt, 0x00, sizeof(RegOpt_ScanOpt) ) ;

		OSVERSIONINFO	OSVI = {0} ;

		OSVI.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
		GetVersionEx( &OSVI ) ;

		if( OSVI.dwMajorVersion > 5 )
			bVistaOnward = true ;

		IsWow64() ;

		GetWindowsDirectory( szWindowsDir, 255 ) ;
		GetSystemDirectory(szSystemDir, 255 ) ;

		if( m_bIsWow64 )
		{
			GetEnvironmentVariable(TEXT("PROGRAMFILES(X86)"), szProgramDirX86, 255 ) ;
			_wcsupr_s( szProgramDirX86, wcslen(szProgramDirX86)*sizeof(TCHAR) ) ;

			ExpandEnvironmentStrings(L"%ProgramW6432%", szProgramDir, 255 ) ;
		}
		else
			GetEnvironmentVariable(TEXT("PROGRAMFILES"), szProgramDir, 255 ) ;

		GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), szAppDataPath, 511 ) ;
		GetEnvironmentVariable(TEXT("COMMONPROGRAMFILES"), szCommProgram, 511 ) ;
		GetEnvironmentVariable(TEXT("PROGRAMDATA"), szProgramData, 255 ) ;
		GetEnvironmentVariable(TEXT("USERPROFILE"), szUserProfile, 255 ) ;
		GetEnvironmentVariable(TEXT("TEMP"), szTempLocal, 255 ) ;
		GetEnvironmentVariable(TEXT("PUBLIC"), szPublic, 255 ) ;
		GetEnvironmentVariable(TEXT("APPDATA"), szAppData, 255 ) ;

		if( !bVistaOnward )
			wcscat(szAppDataPath, TEXT("\\Application Data") ) ;

		GetModuleFileName(NULL, szApplPath, 511 ) ;
		PathRemoveFileSpec( szApplPath ) ;
		SetCurrentDirectory( szApplPath ) ;

		if( szWindowsDir[0])
			_wcsupr_s( szWindowsDir, wcslen(szWindowsDir)*sizeof(TCHAR) ) ;
		if( szSystemDir[0])
			_wcsupr_s( szSystemDir, wcslen(szSystemDir)*sizeof(TCHAR) ) ;
		if( szProgramDir[0])
			_wcsupr_s( szProgramDir, wcslen(szProgramDir)*sizeof(TCHAR) ) ;
		if( szApplPath[0])
			_wcsupr_s( szApplPath, wcslen(szApplPath)*sizeof(TCHAR) ) ;
		if( szAppDataPath[0])
			_wcsupr_s( szAppDataPath, wcslen(szAppDataPath)*sizeof(TCHAR) ) ;
		if( szCommProgram[0])
			_wcsupr_s( szCommProgram, wcslen(szCommProgram)*sizeof(TCHAR) ) ;
		if( szProgramData[0])
			_wcsupr_s( szProgramData, wcslen(szProgramData)*sizeof(TCHAR) ) ;
		if( szUserProfile[0])
			_wcsupr_s( szUserProfile, wcslen(szUserProfile)*sizeof(TCHAR) ) ;
		if( szTempLocal[0])
			_wcsupr_s( szTempLocal, wcslen(szTempLocal)*sizeof(TCHAR) ) ;
		if( szPublic[0] )
			_wcsupr_s( szPublic, wcslen(szPublic)*sizeof(TCHAR) ) ;
		if( szAppData[0] )
			_wcsupr_s( szAppData, wcslen(szAppData)*sizeof(TCHAR) ) ;

		SYSTEM_INFO	sysInfo = { 0 };

		GetSystemInfo(&sysInfo);

		if( (sysInfo.wProcessorArchitecture&PROCESSOR_ARCHITECTURE_AMD64) == PROCESSOR_ARCHITECTURE_AMD64 )
			m_bIsx64 = true;

		ZeroMemory(m_szCommProgramX86, sizeof(m_szCommProgramX86));
		if (m_bIsx64)
		{
			GetEnvironmentVariable(TEXT("CommonProgramFiles(x86)"), m_szCommProgramX86, 511);
			if (m_szCommProgramX86[0])
				_wcsupr_s(m_szCommProgramX86, wcslen(m_szCommProgramX86)*sizeof(TCHAR) );
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::InitializeVariables", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

void iSpySrvMgmt_RegOpt::IsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL) ;

	LPFN_ISWOW64PROCESS		IsWow64Process = NULL ;

	IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(	GetModuleHandle( TEXT("kernel32")),
															"IsWow64Process") ;
	if( !IsWow64Process )
		return ;

	IsWow64Process( GetCurrentProcess(), &m_bIsWow64 ) ;

}

DWORD iSpySrvMgmt_RegOpt::ScanAndRepairRegistryEntries(LPREGOPTSCANOPTIONS pScanOpt )
{

	dwPercentage = 0x00 ;

	dwActiveXEntries = dwUnInstallEntries = dwFontEntries =
	dwSharedDLLs = dwAppPathEntries = dwHelpFilesEntries =
	dwStartupRepairedEntries = dwServicesEntries =	
	dwExtensionEntries = dwRootKitEntries =	dwRogueEntries =
	dwWormEntries =	dwSpywareEntries = dwAdwareEntries =
	dwKeyLoggerEntries = dwBHOEntries = dwExplorerEntries =	
	dwIEEntries = 0x00 ;

	DWORD	dwTotalEntries = 0x12 ;
	DWORD	dwCurrentEntry = 0x01 ;

	//TCHAR	szPercentage[64] = {0} ;
	//ITIN_MEMMAP_DATA iTinMemMap = {0};

	SYSTEMTIME	ST = {0} ;
	TCHAR		szTemp[256] = {0};

	AddUserAndSystemInfoToLogSEH();

	GetLocalTime( &ST ) ;
	
	//In log file (Registry optimizer) "Scanning Registry invalied entries" word 'invalid' spelled incorrect.
	//Niranjan Deshak - 04/03/2015.
	wsprintf(szTemp, TEXT("%s::%02d-%02d-%04d"), m_objWardwizLangManager.GetString("IDS_SCN_INVALID_REG_STRT"),ST.wDay, ST.wMonth, ST.wYear);
	AddToLog(szTemp);

	if( (pScanOpt->bActiveX)&&(!m_bIsWow64) )
	{
		CheckInvalidActiveXEntries( HKEY_CLASSES_ROOT, TEXT("TypeLib") );

		if (m_bIsx64)
			CheckInvalidActiveXEntries(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wow6432Node\\Classes\\TypeLib"));

		pScanOpt->dwStats[0] = dwActiveXEntries ;
	}
	Sleep( 1000 ) ;
	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	//iTinMemMap.dwFirstValue = dwPercentage;
	//iTinMemMap.dwSecondValue = GetRepairedEntriesTotalCount();

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( &iTinMemMap, sizeof(iTinMemMap) ) ;

	
	dwUnInstallEntries = 0x00;
	if( (pScanOpt->bUninstall) &&(!m_bIsWow64))
	{
		CheckInvalidUninstallEntries(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall") );

		if (m_bIsx64)
			CheckInvalidUninstallEntries(TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));

		pScanOpt->dwStats[1] = dwUnInstallEntries ;
	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;

	dwFontEntries = 0x00;
	if( (pScanOpt->bFont) &&(!m_bIsWow64))
	{
		CheckInvalidFontEntries(TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts") );
		if (m_bIsx64)
			CheckInvalidFontEntries(TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"));

		pScanOpt->dwStats[2] = dwFontEntries ;
	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bSharedLibraries) &&(!m_bIsWow64) )
	{
		CheckInvalidSharedLibraries() ;
		pScanOpt->dwStats[3] = dwSharedDLLs ; 
	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	dwAppPathEntries = 0x00;
	if( (pScanOpt->bApplicationPaths) &&(!m_bIsWow64))
	{

		CheckInvalidApplicationPaths(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths"));

		if (m_bIsx64)
			CheckInvalidApplicationPaths(TEXT("SOFTWARE\\WOW6432NODE\\Microsoft\\Windows\\CurrentVersion\\App Paths"));

		pScanOpt->dwStats[4] = dwAppPathEntries ;
	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bHelpFiles) &&(!m_bIsWow64) )
	{
		CheckInvalidHelpFile( ) ;
		pScanOpt->dwStats[5] = dwHelpFilesEntries ; 
	}
	Sleep( 1000 ) ;
	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bStartup) &&(!m_bIsWow64) )
	{
		CheckInvalidStartupEntries( ) ;
		pScanOpt->dwStats[6] = dwStartupRepairedEntries ; 
	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bServices)&&(!m_bIsWow64) )
	{
		CheckInvalidServices( ) ;
		pScanOpt->dwStats[7] = dwServicesEntries ; 
	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bExtensions) &&(!m_bIsWow64) )
	{
		CheckInvalidExtensions( ) ;
		pScanOpt->dwStats[8] = dwExtensionEntries ; 
	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bRootKit) &&(!m_bIsWow64))
	{
		CheckRootKitEntries( ) ;
		pScanOpt->dwStats[9] = dwRootKitEntries ;
	}
	Sleep( 1000 ) ;
	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;

	if( (pScanOpt->bRogueApplications)&&(!m_bIsWow64) )
	{
		CheckRogueApplications( ) ;
		pScanOpt->dwStats[10] = dwRogueEntries ;
	}
	Sleep( 1000 ) ;
	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bWorm) &&(!m_bIsWow64))
	{
		CheckWormEntries( ) ;
		pScanOpt->dwStats[11] = dwWormEntries ;
	}

	Sleep( 1000 ) ;
	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bSpywares) &&(!m_bIsWow64))
	{
		CheckInvalidSpywares( ) ;
		pScanOpt->dwStats[12] = dwSpywareEntries ;

	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bAdwares) &&(!m_bIsWow64) )
	{
		CheckInvalidAdwares( ) ;
		pScanOpt->dwStats[13] = dwAdwareEntries ; 

	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bKeyLogger) &&(!m_bIsWow64) )
	{
		CheckKeyLogger() ;
		pScanOpt->dwStats[14] = dwKeyLoggerEntries ; 

	}

	Sleep( 1000 ) ;
	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bBHO) &&(!m_bIsWow64))
	{
		CheckInvalidBHO( ) ;
		pScanOpt->dwStats[15] = dwBHOEntries ;
	}

	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bExplorer) &&(!m_bIsWow64) )
	{
		CheckInvalidExplorer( ) ;
		pScanOpt->dwStats[16] = dwExplorerEntries ;
	}
	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;


	if( (pScanOpt->bIExplorer) &&(!m_bIsWow64) )
	{
		CheckInvalidIExplorer( ) ;
		pScanOpt->dwStats[17] = dwIEEntries ; 
	}

	Sleep( 1000 ) ;

	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
	dwCurrentEntry++ ;

	CalculatePercentage(dwPercentage);
	//wsprintf(szPercentage, L"%d#%lu", dwPercentage, GetRepairedEntriesTotalCount() ) ;
	//m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( szPercentage, wcslen(szPercentage)*sizeof(TCHAR) ) ;

	//Sleep( 3000 ) ;

	//Sleep time reduced. No need to have 3 secs
	Sleep(500);

	ZeroMemory( szTemp, sizeof(szTemp) );
	wsprintf(szTemp, TEXT("%s"), m_objWardwizLangManager.GetString("IDS_SCN_INVALID_REG_COMPLETE"));
	AddToLog( szTemp );

	ZeroMemory( szTemp, sizeof(szTemp) );
	wsprintf(szTemp, TEXT("%s::%lu\n"), m_objWardwizLangManager.GetString("IDS_TOTAL_REPAIRED_ENTRIES"),GetRepairedEntriesTotalCount() );
	AddToLog( szTemp );

	return 0 ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidActiveXEntries                                                     
*  Description    :	Checks for invalid ActiveX Entries
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
//DWORD iSpySrvMgmt_RegOpt::CheckInvalidActiveXEntries()
DWORD iSpySrvMgmt_RegOpt::CheckInvalidActiveXEntries(HKEY hPredefKey, LPCTSTR lpszSubKey)
{
	DWORD	dwRet = 0x00 ;

	HKEY	hKey = NULL, hSubKey = NULL ;
	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;

	TCHAR	szSubKeyName[512] = {0} ;
	TCHAR	szTemp[512] = {0} ;

	HKEY	hKey_ActivexPath = NULL ;
	DWORD	dwSubSubKeys = 0x00 ;
	TCHAR	szSubSubKeyName[512] = {0} ;
	TCHAR	szPath[512] = {0} ;
	TCHAR	szTempData[1024] = {0} ;

	__try
	{
	/*	dwActiveXEntries = 0x00 ;

		if( RegOpenKeyEx(	HKEY_CLASSES_ROOT, TEXT("TypeLib"),
							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}
	*/

		//HKEY_CLASSES_ROOT, TEXT("TypeLib")
		if (RegOpenKeyEx(hPredefKey, lpszSubKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//Issue		: Registry optimizer some entries not getting clean
		//Resolved	: Vilas
		//Date		: 05 / Feb / 2015
		bool	bRegInvalidFound = false;

		while( true )
		{

			bRegInvalidFound = false;
			i = dwSubKeys = 0x00;

			if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
									NULL, NULL, NULL ) != ERROR_SUCCESS )
			{
				dwRet = 0x02 ;
				goto Cleanup ;
			}

			for(; i<dwSubKeys; i++ )
			{

				dwType = dwRet = 0x00 ;
				dwSize = 511 ;
				memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;

				RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
				if( !szSubKeyName[0] )
					continue ;

				if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
					continue ;

				dwSubSubKeys = 0x00 ;
				RegQueryInfoKey(hSubKey, NULL, NULL, 0, &dwSubSubKeys, NULL, NULL, NULL, NULL,
									NULL, NULL, NULL ) ;
				/*if(  dwSubSubKeys != 0x01 )
				{
					RegCloseKey( hSubKey ) ;
					hSubKey = NULL ;
					continue ;
				}*/

				bool	bPathNotFound = false;

				for (DWORD dwIndex = 0x00; dwIndex < dwSubSubKeys; dwIndex++)
				{

					dwSize = 511;
					memset(szSubSubKeyName, 0x00, 512 * sizeof(TCHAR));
					RegEnumKeyEx(hSubKey, dwIndex, szSubSubKeyName, &dwSize, 0, NULL, NULL, NULL);
					if (!szSubSubKeyName[0])
					{
						//RegCloseKey( hSubKey ) ;
						//hSubKey = NULL ;
						continue ;
					}
					else
					{
						HKEY	hSubSubKey = NULL;

						RegOpenKeyEx(hSubKey, szSubSubKeyName, 0, KEY_READ, &hSubSubKey);

						for (DWORD dwIter = 0x00; dwIter < 0x03; dwIter++)
						{
							TCHAR	szSubSubSubKeyName[512] = { 0 };

							dwSize = 511;

							RegEnumKeyEx(hSubSubKey, dwIter, szSubSubSubKeyName, &dwSize, 0, NULL, NULL, NULL);
							if (!szSubSubSubKeyName[0])
								continue;

							memset(szTemp, 0x00, 512 * sizeof(TCHAR));
							wsprintf(szTemp, TEXT("%s\\%s\\%s\\%s\\Win32"), lpszSubKey, szSubKeyName, szSubSubKeyName, szSubSubSubKeyName);
							if (RegOpenKeyEx(hPredefKey, szTemp, 0, KEY_READ, &hKey_ActivexPath) != ERROR_SUCCESS)
							{
								memset(szTemp, 0x00, 512 * sizeof(TCHAR));
								wsprintf(szTemp, TEXT("%s\\%s\\%s\\%s\\Win64"), lpszSubKey, szSubKeyName, szSubSubKeyName, szSubSubSubKeyName);
								if (RegOpenKeyEx(hPredefKey, szTemp, 0, KEY_READ, &hKey_ActivexPath) != ERROR_SUCCESS)
								{
									//RegCloseKey(hSubKey);
									//hSubKey = NULL;
									continue;
								}
								else
								{
									bPathNotFound = true;
									break;
								}
							}
							else
							{
								bPathNotFound = true;
								break;
							}
						}

						if (hSubSubKey)
						{
							RegCloseKey(hSubSubKey);
							hSubSubKey = NULL;
						}

						if (bPathNotFound)
							break;
					}
				}

				if (bPathNotFound )
				{
					dwSize = 511 ;
					memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
					RegQueryValueEx(hKey_ActivexPath, TEXT(""), 0, &dwType, (LPBYTE)szPath, &dwSize);
					
					//Modified on 09 April to check all path
					//if( (szPath[0]) && (szPath[1] == ':') )
					if (szPath[0])
						dwRet = CheckServiceValidPath(hKey_ActivexPath, szPath );	//dwRet = CheckForPathExists( szPath ) ;
				}
				else
				{
					dwRet = 0x01;
				}

				if (hKey_ActivexPath)
					RegCloseKey( hKey_ActivexPath ) ;
				hKey_ActivexPath = NULL ;

				RegCloseKey( hSubKey ) ;
				hSubKey = NULL ;

				if( dwRet )
				{
					if( DeleteInvalidKey( hKey, szSubKeyName ) )
					//if( true )
					{
						dwActiveXEntries++;
						bRegInvalidFound = true;
						memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
						if (HKEY_LOCAL_MACHINE == hPredefKey)
							wsprintf(szTempData, TEXT("[COM/ActiveX]\t\tHKEY_LOCAL_MACHINE\\%s[%s]"), szTemp, szPath ) ;
						else
							wsprintf(szTempData, TEXT("[COM/ActiveX]\t\tHKEY_CLASSES_ROOT\\%s[%s]"), szTemp, szPath);
						AddToLog( szTempData ) ;
						//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
						//AddToListView("Windows Services", szTemp, "Repaired" ) ;
					}
				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
				break;
		}

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidActiveXEntries", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hSubKey )
		RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	return dwRet ;
}


/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidUninstallEntries                                                     
*  Description    :	Checks for invalid Uninstall Entries
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
*  Modified Date  : 09 April 2015
**********************************************************************************************************/
//DWORD iSpySrvMgmt_RegOpt::CheckInvalidUninstallEntries()
DWORD iSpySrvMgmt_RegOpt::CheckInvalidUninstallEntries(LPCTSTR lpszSubKey)
{
	DWORD	dwRet = 0x00 ;

	HKEY	hKey = NULL, hSubKey = NULL ;
	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;

	TCHAR	szSubKeyName[512] = {0} ;
	TCHAR	szTemp[512] = {0} ;
	TCHAR	szPath[512] = {0} ;
	TCHAR	szTempData[1024] = {0} ;

	__try
	{
		//dwUnInstallEntries = 0x00 ;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszSubKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		//Issue		: Registry optimizer some entries not getting clean
		//Resolved	: Vilas
		//Date		: 05 / Feb / 2015
		bool	bRegInvalidFound = false;

		while( true )
		{

			bRegInvalidFound = false;
			i = dwSubKeys = 0x00;

			if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
									NULL, NULL, NULL ) != ERROR_SUCCESS )
			{
				dwRet = 0x02 ;
				goto Cleanup ;
			}

			for(; i<dwSubKeys; i++ )
			{
				dwType = dwRet = 0x00 ;
				dwSize = 511 ;
				memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;

				RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
				if( !szSubKeyName[0] )
					continue ;

				if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
					continue ;

				dwSize = 511 ;
				memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
				RegQueryValueEx(hSubKey, TEXT("DisplayName"), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
				dwRet = static_cast<DWORD>(wcslen( szPath )) ;

				RegCloseKey( hSubKey ) ;
				hSubKey = NULL ;

				if( !dwRet )
				{
					if( DeleteInvalidKey( hKey, szSubKeyName ) )
					//if( true )
					{
						dwUnInstallEntries++ ;
						bRegInvalidFound = true;
						memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
						wsprintf(szTempData, TEXT("[Uninstall_Entries]\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s"), szSubKeyName ) ;
						AddToLog( szTempData ) ;

						//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
						//AddToListView("Windows Services", szTemp, "Repaired" ) ;
					}
				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
					break;
		}

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidUninstallEntries", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hKey )
		RegCloseKey( hKey ) ;
	hKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidFontEntries                                                     
*  Description    :	Checks for invalid Font Entries
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
*  Modified Date  : 09 Feb 2015
**********************************************************************************************************/
//DWORD iSpySrvMgmt_RegOpt::CheckInvalidFontEntries()
DWORD iSpySrvMgmt_RegOpt::CheckInvalidFontEntries(LPCTSTR lpszSubKey)
{
	DWORD	dwRet = 0x00 ;

	HKEY	hKey = NULL, hSubKey = NULL ;
	DWORD	dwValues = 0x00, dwSize = 0x00, dwType, i = 0x00 ;

	TCHAR	szSubKeyName[512] = {0} ;
	TCHAR	szTemp[512] = {0} ;
	TCHAR	szPath[512] = {0} ;
	TCHAR	szTempData[1024] = {0} ;
	TCHAR	szValueName[512] = {0} ;
	TCHAR	szValueData[512] = {0} ;

	DWORD	dwValueName, dwValueData ;

	__try
	{
		//dwFontEntries = 0x00 ;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszSubKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		bool	bRegInvalidFound = false;

		while( true )
		{
			bRegInvalidFound = false;
			dwValues = 0x00;

			RegQueryInfoKey(hKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
							NULL, NULL, NULL ) ;
			if( dwValues == 0x00 )
			{
				dwRet = 0x02 ;
				goto Cleanup ;
			}

			for(i=0; i<dwValues; i++ )
			{
				memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
				memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;

				dwValueName = dwValueData = 511 ;

				RegEnumValue(	hKey, i, szValueName, &dwValueName, 0, &dwType, 
								(LPBYTE)szValueData, &dwValueData) ;
				if( !szValueName[0] )
					continue ;

				if( PathFileExists(szValueData ) )
					continue ;

				if( wcsrchr(szValueData, '\\' ) )
					continue ;

				memset(szTemp, 0x00, 512*sizeof(TCHAR) ) ;
				swprintf(szTemp, TEXT("%s\\Fonts\\%s"), szWindowsDir, szValueData ) ;

				dwRet = CheckForPathExists( szTemp ) ;
				if( dwRet )
				{

					if( RegDeleteValue( hKey, szValueName ) == ERROR_SUCCESS )
					{
						//Delete value name
						memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
						//swprintf(szTempData, TEXT("[Font_Entries]\t\tHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\%s[%s]"), szValueName, szTemp ) ;
						swprintf(szTempData, TEXT("[Font_Entries]\t\tHKLM\\%s\\%s[%s]"), lpszSubKey, szValueName, szTemp);
						AddToLog( szTempData ) ;

						//RegDeleteValue( hKey, szValueName ) ;
						dwFontEntries++ ;
						bRegInvalidFound = true;
					}
				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
					break;
		}

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidFontEntries", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hKey )
		RegCloseKey( hKey ) ;

	hKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidSharedLibraries                                                     
*  Description    :	Checks for invalid Shared Libraries
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidSharedLibraries( )
{

	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;

	__try
	{
		dwSharedDLLs = 0x00 ;
		EnumRegValueNameForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs"),
										TEXT("SharedDLLs_Entries"), dwSharedDLLs) ;

		//Added for 32 bit SharedDLLs
		//Added on 08 April 2015
		//HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\SharedDLLs
		EnumRegValueNameForPathExist(TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs"),
			TEXT("SharedDLLs_Entries"), dwSharedDLLs);

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidSharedLibraries", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}


/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidApplicationPaths                                                     
*  Description    :	Checks for invalid application paths
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidApplicationPaths(LPCTSTR lpszSubKey)
{
	
//dwAppPathEntries = 0x00 ;
//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
		
	DWORD	dwRet = 0x00 ;

	HKEY	hKey = NULL, hSubKey = NULL ;
	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;

	TCHAR	szSubKeyName[512] = {0} ;
	TCHAR	szTemp[512] = {0} ;
	TCHAR	szPath[512] = {0} ;
	TCHAR	szTempData[1024] = {0} ;

	//bShowMsg = true ;

	__try
	{
		//dwAppPathEntries = 0x00 ;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszSubKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		//Issue		: Registry optimizer some entries not getting clean
		//Resolved	: Vilas
		//Date		: 05 / Feb / 2015
		bool	bRegInvalidFound = false;

		while( true )
		{
			bRegInvalidFound = false;
			i = dwSubKeys = 0x00;
			if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
									NULL, NULL, NULL ) != ERROR_SUCCESS )
			{
				dwRet = 0x02 ;
				goto Cleanup ;
			}

			for(; i<dwSubKeys; i++ )
			{
				dwType = dwRet = 0x00 ;
				dwSize = 511 ;
				memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;
				memset(szTemp, 0x00, 512*sizeof(TCHAR) ) ;

				RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
				if( !szSubKeyName[0] )
					continue ;

				if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
					continue ;

				dwSize = 511 ;
				memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
				RegQueryValueEx(hSubKey, TEXT(""), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
				if( szPath[0] )
					dwRet = CheckServiceValidPath( hSubKey, szPath ) ;
				else
					dwRet = 0x01 ;

				RegCloseKey( hSubKey ) ;
				hSubKey = NULL ;

				if( dwRet )
				{

					if( DeleteInvalidKey( hKey, szSubKeyName ) )
					//if( true )
					{
						dwAppPathEntries++ ;
						bRegInvalidFound = true;
						memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
						wsprintf(szTempData, TEXT("[AppPath_Entries]\t\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s[%s]"), szSubKeyName, szPath ) ;
						AddToLog( szTempData ) ;

						//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
						//AddToListView("Windows Services", szTemp, "Repaired" ) ;
					}
				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
					break;
		}

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidApplicationPaths", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hKey )
		RegCloseKey( hKey ) ;
	hKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidHelpFile                                                     
*  Description    :	Checks for invalid help file
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidHelpFile( )
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;
	__try
	{
		dwHelpFilesEntries = 0x00 ;
		//m_ScanningText.SetWindowTextW( TEXT("Scanning : Invalid Help File entries") ) ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidHelpFile", 0, 0, true, SECONDLEVEL);
	}
	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidStartupEntries                                                     
*  Description    :	Checks for invalid startup entries
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidStartupEntries( )
{

	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;

	__try
	{

		dwStartupRepairedEntries = 0x00 ;

		EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), TEXT("Startup_Entries"), dwStartupRepairedEntries ) ;
		EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"), TEXT("Startup_Entries"), dwStartupRepairedEntries ) ;
		EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx"), TEXT("Startup_Entries"), dwStartupRepairedEntries ) ;

		EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), TEXT("Startup_Entries"), dwStartupRepairedEntries, HKEY_CURRENT_USER ) ;
		//EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), TEXT("Startup Entries"), dwStartupRepairedEntries, HKEY_CURRENT_USER ) ;
		//EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), TEXT("Startup Entries"), dwStartupRepairedEntries, HKEY_CURRENT_USER ) ;

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidStartupEntries", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidServices                                                     
*  Description    :	Checks for invalid windows services
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidServices( )
{
	DWORD	dwRet = 0x00 ;

	HKEY	hKey = NULL, hSubKey = NULL ;
	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;

	TCHAR	szSubKeyName[512] = {0} ;
	TCHAR	szTemp[512] = {0} ;
	TCHAR	szTempData[1024] = {0};


	__try
	{

		dwServicesEntries = 0x00 ;

		if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\services"),
							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		//Issue		: Registry optimizer some entries not getting clean
		//Resolved	: Vilas
		//Date		: 05 / Feb / 2015
		bool bRegInvalidFound = false;
		while( true )
		{
			bRegInvalidFound = false;
			i = dwSubKeys = 0x00;
			if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
									NULL, NULL, NULL ) != ERROR_SUCCESS )
			{
				dwRet = 0x02 ;
				goto Cleanup ;
			}

			for(; i<dwSubKeys; i++ )
			{
				dwType = dwRet = 0x00 ;
				dwSize = 511 ;
				memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;

				RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
				if( !szSubKeyName[0] )
					continue ;

				if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
					continue ;


				dwSize = 511 ;
				memset(szTemp, 0x00, 512*sizeof(TCHAR) ) ;
				RegQueryValueEx(hSubKey, TEXT("ImagePath"), 0, &dwType, (LPBYTE)szTemp, &dwSize) ;
				if( szTemp[0] )
					dwRet = CheckServiceValidPath( hSubKey, szTemp ) ;

				RegCloseKey( hSubKey ) ;
				hSubKey = NULL ;

				//dwRet = 0x01 ;
				if( dwRet )
				{
					if( DeleteInvalidKey( hKey, szSubKeyName ) )
					{
						bRegInvalidFound = true;
						dwServicesEntries++ ;
						memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
						wsprintf(szTempData, TEXT("[WinNT_Services]\t\tHKLM\\SYSTEM\\CurrentControlSet\\services\\%s[%s]"), szSubKeyName, szTemp ) ;
						AddToLog( szTempData ) ;

						//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
						//AddToListView("Windows Services", szTemp, "Repaired" ) ;
					}
				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
					break;
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidServices", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	RegCloseKey( hKey ) ;
	hKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidExtensions                                                     
*  Description    :	Checks for invalid extensions
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidExtensions( )
{
	DWORD	dwRet = 0x00 ;

	HKEY	hKey = NULL, hSubKey = NULL ;
	DWORD	dwSubKeys = 0x00, dwSize = 0x00, i = 0x00 ;

	TCHAR	szSubKeyName[512] = {0} ;
	TCHAR	szTemp[1024] = {0} ;

	__try
	{

		dwExtensionEntries = 0x00 ;

		if( RegOpenKeyEx(HKEY_CURRENT_USER, 
						TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts"), 
						0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		//Issue		: Registry optimizer some entries not getting clean
		//Resolved	: Vilas
		//Date		: 05 / Feb / 2015
		bool	bRegInvalidFound = false;

		while( true )
		{
			bRegInvalidFound = false;
			i = dwSubKeys = 0x00;
			if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
									NULL, NULL, NULL ) != ERROR_SUCCESS )
			{
				dwRet = 0x02 ;
				goto Cleanup ;
			}

			for(; i<dwSubKeys; i++ )
			{
				dwRet = 0x00 ;
				dwSize = 511 ;
				memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;

				RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
				if( wcsnlen(szSubKeyName, 511) < 0x02 )
					continue ;

				if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
					continue ;

				//sprintf(szTemp, "%s\\OpenWithList", szSubKeyName ) ;
				dwRet = CheckValidExplorerExtension( hSubKey, szSubKeyName ) ;

				RegCloseKey( hSubKey ) ;
				hSubKey = NULL ;

				if( !dwRet )
				{
					if( DeleteInvalidKey( hKey, szSubKeyName ) )
					//if( true )
					{
						bRegInvalidFound = true;
						dwExtensionEntries++ ;
						memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
						wsprintf(szTemp, TEXT("[Explorer_extensions]\tHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s"), szSubKeyName ) ;
						AddToLog( szTemp ) ;

						//sprintf(szTemp, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s", szSubKeyName ) ;
						//AddToListView("Explorer extensions", szTemp, "Repaired" ) ;
					}
				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
					break;
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidExtensions", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hSubKey )
		RegCloseKey( hSubKey ) ;

	if( hKey )
		RegCloseKey( hKey ) ;

	hSubKey = NULL ;
	hKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckRootKitEntries                                                     
*  Description    :	Checks invalid rootkit entries
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckRootKitEntries( )
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;

	__try
	{
		dwRootKitEntries = 0x00 ;
		//m_ScanningText.SetWindowTextW( TEXT("Scanning : RootKit entries") ) ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckRootKitEntries", 0, 0, true, SECONDLEVEL);
	}

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckRogueApplications                                                     
*  Description    :	Checks invalid rogue applications
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckRogueApplications( )
{
	DWORD	dwRet = 0x00 ;

	__try
	{
		dwRogueEntries = 0x00 ;
		
		//Checking invalid class IDs HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID
		//Checking invalid class IDs HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Classes\CLSID

		CheckForInvalidClassID(L"SOFTWARE\\Classes\\CLSID", dwRogueEntries);
		
		if (m_bIsx64)
			CheckForInvalidClassID(L"SOFTWARE\\Wow6432Node\\Classes\\CLSID", dwRogueEntries);

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckRogueApplications", 0, 0, true, SECONDLEVEL);
	}

	return dwRet ;
}

/**********************************************************************************************************
*  Function Name  :	CheckForInvalidClassID
*  Description    :	Checks invalid class ID
*  Author Name    : Vilas
*  Date           : 09 April 2015
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckForInvalidClassID(LPCTSTR lpszSubKey, DWORD &dwEntries)
{
	DWORD	dwRet = 0x00;
	HKEY	hCLSIDKey = NULL;
	
	__try
	{
		
		DWORD	dwSubKeys = 0x00;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszSubKey, 0, KEY_ALL_ACCESS, &hCLSIDKey) != ERROR_SUCCESS)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		while (true)
		{
			bool	bRegKeyDelete = false;

			if (RegQueryInfoKey(hCLSIDKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL) != ERROR_SUCCESS)
			{
				dwRet = 0x02;
				goto Cleanup;
			}

			if (!dwSubKeys)
			{
				dwRet = 0x03;
				goto Cleanup;
			}

			DWORD	i = 0x00;

			for (; i < dwSubKeys; i++)
			{
				DWORD	dwSize = 511;

				TCHAR	szSubKeyName[512] = { 0 };

				RegEnumKeyEx(hCLSIDKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL);
				if (szSubKeyName[0] != 0x007B)			//Checking {
					continue;

				HKEY	hSubKey = NULL;

				if (RegOpenKeyEx(hCLSIDKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey) != ERROR_SUCCESS)
					continue;

				DWORD	dwSubSubKeys = 0x00;
				bool	bInvalidCLSID = false;

				RegQueryInfoKey(hSubKey, NULL, NULL, 0, &dwSubSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				if (!dwSubSubKeys)
				{
					dwSize = 0x00;
					RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwSize, NULL, NULL, NULL, NULL);
					if (!dwSize)
						bInvalidCLSID = true;
				}
				else
				{
					DWORD	dwEmptyKey = 0x00, dwNullValueData = 0x00;

					for (DWORD dwIndex = 0x00; dwIndex < dwSubSubKeys; dwIndex++)
					{
						TCHAR	szSubSubKey[512] = { 0 };
						HKEY	hSubSubKey = NULL;

						dwSize = 511;
						RegEnumKeyEx(hSubKey, dwIndex, szSubSubKey, &dwSize, 0, NULL, NULL, NULL);
						if (!szSubSubKey[0])
							continue;

						RegOpenKeyEx(hSubKey, szSubSubKey, 0, KEY_READ, &hSubSubKey);
						if (hSubSubKey)
						{
							DWORD	dwType = 0x00;
							TCHAR	szDefaultValueData[512] = { 0 };

							dwNullValueData = dwSize = 0x00;

							RegQueryInfoKey(hSubSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwSize, NULL, NULL, NULL, NULL);

							for (DWORD dwValueIndex = 0x00; dwValueIndex < dwSize; dwValueIndex++)
							{
								TCHAR	szValueName[512] = { 0 };
								TCHAR	szValueData[512] = { 0 };

								DWORD	dwValueName = 511, dwValueData = 511;

								dwType = 0x00;
								RegEnumValue(hSubSubKey, dwValueIndex, szValueName, &dwValueName, 0, &dwType, (LPBYTE)szValueData, &dwValueData);
								if ((!szValueName[0]) && (!szValueData[0]))
									dwNullValueData++;

								/*if (dwSize == 0x01)
								{
									dwSize = 511;
									RegQueryValueEx(hSubSubKey, TEXT(""), 0, &dwType, (LPBYTE)szDefaultValueData, &dwSize);
									if (!szDefaultValueData[0])
										dwDefaultValueData++;

								}*/
							}

							RegCloseKey(hSubSubKey);
							hSubSubKey = NULL;

							if ((dwSize == dwNullValueData) && (dwSize>0x00))
								dwEmptyKey++;
						}
					}

					if (dwEmptyKey == dwSubSubKeys)
						bInvalidCLSID = true;
				}

				if (bInvalidCLSID)
				{
					if (DeleteInvalidKey(hCLSIDKey, szSubKeyName))
					{
						dwEntries++;
						bRegKeyDelete = true;

						TCHAR	szTempData[512] = { 0 };

						wsprintf(szTempData, TEXT("[Invalid_CLSID]\t\tHKLM\\%s[%s]"), lpszSubKey, szSubKeyName);
						AddToLog(szTempData);
					}
				}

				RegCloseKey(hSubKey);
				hSubKey = NULL;

				if (bRegKeyDelete)
					break;
			}

			if (!bRegKeyDelete)
				break;
		}

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckForInvalidClassID", 0, 0, true, SECONDLEVEL);
	}


Cleanup:

	if (hCLSIDKey)
		RegCloseKey(hCLSIDKey);
	hCLSIDKey = NULL;

	return dwRet;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckWormEntries                                                     
*  Description    :	Checks invalid worm entries
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckWormEntries( )
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;

	__try
	{
		dwWormEntries = 0x00 ;
		//m_ScanningText.SetWindowTextW( TEXT("Scanning : Worm entries") ) ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckWormEntries", 0, 0, true, SECONDLEVEL);
	}

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidSpywares                                                     
*  Description    :	Checks invalid spyware threats
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidSpywares()
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTempData[512] = { 0 };

	try
	{

	/*	HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunServicesOnce
		HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunServices
		HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\RunServicesOnce
		HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\RunServices
		HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer\Run
		HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\Userinit
		HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell
		HKEY_LOCAL_MACHINE \ Software \ Microsoft \ Windows \ CurrentVersion \ Explorer \ ShellExecuteHooks
		HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellServiceObjects

		HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellExecuteHooks
HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Notify
HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\AppInit
HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Start Page
HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Browser Helper Objects
HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ShellServiceObjectDelayLoad
HKLM\SYSTEM\CurrentControlSet\Services\WinSock2
HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\UserInit
HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\BootExecute


		[HKEY_CLASSES_ROOT\exefile\shell\open\command] ="\"%1\" %*"
[HKEY_CLASSES_ROOT\comfile\shell\open\command] ="\"%1\" %*"
[HKEY_CLASSES_ROOT\batfile\shell\open\command] ="\"%1\" %*"
[HKEY_CLASSES_ROOT\htafile\Shell\Open\Command] ="\"%1\" %*"
[HKEY_CLASSES_ROOT\piffile\shell\open\command] ="\"%1\" %*"
[HKEY_LOCAL_MACHINE\Software\CLASSES\batfile\shell\open\command] ="\"%1\" %*"
[HKEY_LOCAL_MACHINE\Software\CLASSES\comfile\shell\open\command] ="\"%1\" %*"
[HKEY_LOCAL_MACHINE\Software\CLASSES\exefile\shell\open\command] ="\"%1\" %*"
[HKEY_LOCAL_MACHINE\Software\CLASSES\htafile\Shell\Open\Command] ="\"%1\" %*"
[HKEY_LOCAL_MACHINE\Software\CLASSES\piffile\shell\open\command] ="\"%1\" %*"
	*/

		dwSpywareEntries = 0x00 ;
		
		TCHAR	szData[512] = { 0 };

		if (!SetRegSZValue(TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
			TEXT("Shell"), TEXT("explorer.exe"), true))
		{
			dwSpywareEntries++;

			wsprintf(szTempData, TEXT("[Spy_Entries]\t\tHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\%s[%s]"), TEXT("Shell"), TEXT("explorer.exe"));
			AddToLog(szTempData);
		}


		//Added by Vilas on 16 April 2015
		//Checking for malware running after user initialization done when user logged in
		if ( wcslen(szSystemDir)>0x10 )
		{
			wsprintf(szData, TEXT("%s\\userinit.exe,"), szSystemDir);
			if (!SetRegSZValue(TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
				TEXT("Userinit"), szData, true))
			{
				dwSpywareEntries++;

				wsprintf(szTempData, TEXT("[Spy_Entries]\t\tHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\%s[%s]"), TEXT("Userinit"), szData);
				AddToLog(szTempData);
			}
		}


		//Added by Vilas on 11 April 2015
		//Checking for Disble Registry tool
		DWORD	dwRequiredValue = 0x00;
		if (!SetRegDWORDValue(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableRegistryTools", dwRequiredValue, true ) )
		{
			//DisableRegistryTools set to zero
			dwSpywareEntries++;

			wsprintf(szTempData, TEXT("[Spy_Entries]\t\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\%s[%lu]"), L"DisableRegistryTools", dwRequiredValue);
			AddToLog(szTempData);
		}

		CITinRegWrapper objReg;

		//Checking for Disble Task Manager tool
		dwRequiredValue = 0x00;
		if (!objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableTaskMgr", dwRequiredValue))
		{
			//DisableRegistryTools set to zero
			dwSpywareEntries++;

			wsprintf(szTempData, TEXT("[Spy_Entries]\t\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\%s[%lu]"), L"DisableTaskMgr", dwRequiredValue);
			AddToLog(szTempData);
		}

		dwRequiredValue = 0x00;
		if (!objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", L"DisableRegistryTools", dwRequiredValue))
		{
			//DisableRegistryTools set to zero
			dwSpywareEntries++;

			wsprintf(szTempData, TEXT("[Spy_Entries]\t\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\%s[%lu]"), L"DisableTaskMgr", dwRequiredValue);
			AddToLog(szTempData);
		}


		CheckShellExecuteHooks( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks"), dwSpywareEntries) ;

		//Checking for current user MuiCache
		//Added on 08 April 2015 by Vilas
		//HKEY_CURRENT_USER\Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache
		TCHAR	szSubkey[] = TEXT("Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache");

		CheckForCurrentUserEntries(szSubkey, dwSpywareEntries);

		goto Cleanup ;
	}
	catch (...)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidSpywares", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

/**********************************************************************************************************
*  Function Name  :	CheckForCurrentUserEntries
*  Description    :	Handling for current user
*  Author Name    : Vilas
*  Date           : 08 April 2015
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckForCurrentUserEntries(LPTSTR lpszSubKey, DWORD &dwEntries)
{
	DWORD	dwRet = 0x00;

	HKEY	hCurrentUserKey = NULL;
	HKEY	hSubKey = NULL;

	__try
	{
	
		RegConnectRegistry(NULL, HKEY_USERS, &hCurrentUserKey);

/*		RegOpenCurrentUser(KEY_ALL_ACCESS, &hCurrentUserKey);
		if (!hCurrentUserKey)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//HKEY_CURRENT_USER\Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache
		DWORD	dwSubKeys = 0x00, i = 0x00, dwAccess=0x00;

		TCHAR	szToken[] = L"\\";
		TCHAR	*pToken = NULL;

		pToken = wcstok(lpszSubKey, szToken);
		while (pToken)
		{
			if (wcsicmp(pToken, L"Software") == 0x00)
				dwAccess = KEY_READ;
			else
				dwAccess = KEY_ALL_ACCESS;

			if (RegOpenKeyEx(hCurrentUserKey, pToken, 0, dwAccess, &hSubKey) != ERROR_SUCCESS)
			{
				DWORD	dw = GetLastError();
				dwRet = 0x02;
				break;
			}

			RegCloseKey(hCurrentUserKey);
			hCurrentUserKey = hSubKey;

			pToken = wcstok(NULL, szToken);
		}

		if (hSubKey)
			EnumRegValueNameForPathExist(hSubKey, lpszSubKey, L"MuiCache_Entries", dwEntries, HKEY_CURRENT_USER);
*/

		DWORD	dwSubKeys = 0x00, i = 0x00;

		if (RegQueryInfoKey(hCurrentUserKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL) != ERROR_SUCCESS)
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		//HKEY	hOrgCurrUserKey = hCurrentUserKey;

		for (; i < dwSubKeys; i++)
		{
			DWORD	dwType = 0x00;// , dwAccess;
			DWORD	dwSize = 511;

			TCHAR	szCurrSubKeyName[512] = { 0 };
			TCHAR	szSubKeyName[512] = { 0 };

			RegEnumKeyEx(hCurrentUserKey, i, szCurrSubKeyName, &dwSize, 0, NULL, NULL, NULL);
			if ((wcslen(szCurrSubKeyName) <0x09) || (wcsstr(szCurrSubKeyName, L"_Classes")))
				continue;

			if (wmemcmp(szCurrSubKeyName, L"S-", wcslen(L"S-") ) != 0x00)
				continue;

			wsprintf(szSubKeyName, TEXT("%s\\%s"), szCurrSubKeyName, lpszSubKey);

			RegOpenKeyEx(hCurrentUserKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey);

	/*		HKEY	hSubKeyTemp = NULL;

			TCHAR	szCurrSubKeyNameTemp[512] = { 0 };

			wsprintf(szCurrSubKeyNameTemp, TEXT("%s\\%s"), szCurrSubKeyName, lpszSubKey);

			RegOpenKeyEx(hOrgCurrUserKey, szCurrSubKeyNameTemp, 0, KEY_ALL_ACCESS, &hSubKeyTemp);
			
			if (RegOpenKeyEx(hCurrentUserKey, szCurrSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey) != ERROR_SUCCESS)
			{
				dwRet = 0x01;
				goto Cleanup;
			}

			TCHAR	szToken[] = L"\\";
			TCHAR	*pToken = NULL;

			RegCloseKey(hCurrentUserKey);
			hCurrentUserKey = NULL;

			hCurrentUserKey = hSubKey;

			wcscpy_s(szSubKeyName, 511, lpszSubKey);

			pToken = wcstok(szSubKeyName, szToken);
			while (pToken)
			{
				
				if (RegOpenKeyEx(hCurrentUserKey, pToken, 0, KEY_ALL_ACCESS, &hSubKey) != ERROR_SUCCESS)
				{
					DWORD	dw = GetLastError();
					dwRet = 0x02;
					break;
				}

				RegCloseKey(hCurrentUserKey);
				hCurrentUserKey = hSubKey;

				pToken = wcstok(NULL, szToken);
			}
		*/
			if (hSubKey)
			{
				EnumRegValueNameForPathExist(hSubKey, lpszSubKey, L"MuiCache_Entries", dwEntries, HKEY_CURRENT_USER);

				RegCloseKey(hSubKey);
				hSubKey = NULL;
			}
		}

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckForCurrentUserEntries", 0, 0, true, SECONDLEVEL);
	}

Cleanup:


	if (hSubKey)
	{
		RegCloseKey(hSubKey);
		hSubKey = NULL;
	}

	if (hCurrentUserKey)
	{
		RegCloseKey(hCurrentUserKey);
		hCurrentUserKey = NULL;
	}

	return dwRet;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidAdwares                                                     
*  Description    :	Checks invalid adware threats
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidAdwares()
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;

	__try
	{

	/*
	[HKEY_LOCAL_MACHINE\Software\Microsoft\Active Setup\Installed Components\KeyName] 
	StubPath=C:\PathToFile\Filename.exe 

	//need to take care of this n study first
	HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Internet Explorer\ActiveX Compatibility
	HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\ShellServiceObjectDelayLoad
	*/
		dwAdwareEntries = 0x00 ;

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidAdwares", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckKeyLogger                                                     
*  Description    :	Checks invalid keyloggers
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckKeyLogger()
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;

	__try
	{

		dwKeyLoggerEntries = 0x00 ;
		//m_ScanningText.SetWindowTextW( TEXT("Scanning : Keyloggers entries") ) ;

		//goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckKeyLogger", 0, 0, true, SECONDLEVEL);
	}

//Cleanup :

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidBHO                                                     
*  Description    :	Checks invalid browser helper object
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidBHO()
{
	DWORD	dwRet = 0x00 ;

	HKEY	hKey = NULL, hSubKey = NULL ;
	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;

	TCHAR	szSubKeyName[512] = {0} ;
	TCHAR	szBHOKey[512] = {0} ;
	TCHAR	szPath[512] = {0} ;
	TCHAR	szTempData[1024] = {0} ;

	__try
	{
		dwBHOEntries = 0x00 ;

		if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects"),
							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		//Issue		: Registry optimizer some entries not getting clean
		//Resolved	: Vilas
		//Date		: 05 / Feb / 2015
		bool	bRegInvalidFound = false;

		while( true )
		{
			bRegInvalidFound = false;
			i = dwSubKeys = 0x00;

			if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
									NULL, NULL, NULL ) != ERROR_SUCCESS )
			{
				dwRet = 0x02 ;
				goto Cleanup ;
			}

			for(; i<dwSubKeys; i++ )
			{
				dwType = dwRet = 0x00 ;
				dwSize = 511 ;
				memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;


				RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
				if( !szSubKeyName[0] )
					continue ;

				memset(szBHOKey, 0x00, 512*sizeof(TCHAR) ) ;
				wsprintf(szBHOKey, TEXT("CLSID\\%s\\InProcServer32"), szSubKeyName ) ;

				if( RegOpenKeyEx(HKEY_CLASSES_ROOT, szBHOKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
					continue ;

				dwSize = 511 ;
				memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
				RegQueryValueEx(hSubKey, TEXT(""), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
				if( szPath[0] )
					dwRet = CheckForPathExists( szPath ) ;

				RegCloseKey( hSubKey ) ;
				hSubKey = NULL ;

				//dwRet = 0x01 ;
				if( dwRet )
				{
					if( DeleteInvalidKey( hKey, szSubKeyName ) )
					//if( true )
					{
						dwBHOEntries++ ;
						bRegInvalidFound = true;
						memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
						wsprintf(szTempData, TEXT("[BHO_Entries]\t\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\%s[%s]"), szSubKeyName, szPath ) ;
						AddToLog( szTempData ) ;

						//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
						//AddToListView("Windows Services", szTemp, "Repaired" ) ;
					}

				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
				break;
		}

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidBHO", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hKey )
		RegCloseKey( hKey ) ;
	hKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidExplorer                                                     
*  Description    :	Checks invalid explorer entries
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidExplorer()
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;

	__try
	{

	/*
	Windows Explorer
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\RunMRU
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\ComDlg32\OpenSavePidlMRU
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\ComDlg32\LastVisitedPidlMRU
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\RecentDocs
HKEY_CURRENT_USER\Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\UserAssist
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\SharedTaskScheduler
	*/

		dwExplorerEntries = 0x00 ;

		EnumRegValueNameDeleteAll(	TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"),
									TEXT("Explorer_Entries"), dwExplorerEntries, 
									HKEY_CURRENT_USER ) ;

		EnumRegValueNameDeleteAll(	TEXT("Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache"),
									TEXT("Explorer_Entries"), dwExplorerEntries, 
									HKEY_CURRENT_USER ) ;

		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\TypedPaths"),
									TEXT("Explorer_Entries"), dwExplorerEntries,
									HKEY_CURRENT_USER ) ;



		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs"),
									TEXT("Explorer_Entries"), dwExplorerEntries,
									HKEY_CURRENT_USER ) ;

	/*	pHistClrDlg->EnumSubKeyForDeletion(
						"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs",
						true ) ;
	*/

		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist"),
									TEXT("Explorer_Entries"), dwExplorerEntries,
									HKEY_CURRENT_USER ) ;

		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSavePidlMRU"),
									TEXT("Explorer_Entries"), dwExplorerEntries,
									HKEY_CURRENT_USER ) ;

		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\LastVisitedPidlMRU"),
									TEXT("Explorer_Entries"), dwExplorerEntries,
									HKEY_CURRENT_USER ) ;

		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\Recent File List"),
									TEXT("Explorer_Entries"), dwExplorerEntries,
									HKEY_CURRENT_USER ) ;

		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit"),
									TEXT("Explorer_Entries"), dwExplorerEntries,
									HKEY_CURRENT_USER ) ;

		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Wordpad\\Recent File List"),
									TEXT("Explorer_Entries"), dwExplorerEntries,
									HKEY_CURRENT_USER ) ;


		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidExplorer", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckInvalidIExplorer                                                     
*  Description    :	Checks invalid internet explorer entries
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckInvalidIExplorer()
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[128] = {0} ;

	__try
	{
		dwIEEntries = 0x00 ;

		EnumRegValueNameDeleteAll(TEXT("Software\\Microsoft\\Internet Explorer\\TypedUrls"), 
							TEXT("IExplorer_Entries"), dwIEEntries, HKEY_CURRENT_USER ) ;

		EnumRegValueNameDeleteAll(TEXT("Software\\Microsoft\\Internet Explorer\\IntelliForms\\Storage1"), 
							TEXT("IExplorer_Entries"), dwIEEntries, HKEY_CURRENT_USER ) ;

		EnumRegValueNameDeleteAll(TEXT("Software\\Microsoft\\Internet Explorer\\IntelliForms\\Storage2"), 
							TEXT("IExplorer_Entries"), dwIEEntries, HKEY_CURRENT_USER ) ;

		EnumRegValueNameDeleteAll(TEXT("Software\\Microsoft\\MediaPlayer\\Player\\RecentFileList"), 
							TEXT("IExplorer_Entries"), dwIEEntries, HKEY_CURRENT_USER ) ;

		goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidIExplorer", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckShellExecuteHooks                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckShellExecuteHooks(TCHAR *pSubKey, DWORD &dwEntries, HKEY hPredKey )
{
	DWORD	dwRet = 0x00 ;
	HKEY	hKey = NULL, hSubKey = NULL ;

	DWORD	i, dwValues = 0x00 ;
	DWORD	dwValueName, dwValueData, dwType, dwSize ;
	TCHAR	szValueName[512] = {0}, szPath[512] ;
	TCHAR	szValueData[512] = {0} ;
	TCHAR	szTemp[1024] = {0} ;

	__try
	{
		if( RegOpenKeyEx( hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		RegQueryInfoKey(hKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
						NULL, NULL, NULL ) ;
		if( dwValues == 0x00 )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		for(i=0; i<dwValues; i++ )
		{
			memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
			memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;

			dwValueName = dwValueData = 511 ;
			dwType = 0x00 ;
			RegEnumValue(	hKey, i, szValueName, &dwValueName, 0, &dwType, 
							(LPBYTE)szValueData, &dwValueData) ;
			if( !szValueName[0] )
				continue ;

			memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
			wsprintf(szTemp, TEXT("CLSID\\%s\\InProcServer32"), szValueName ) ;
			if( RegOpenKeyEx(HKEY_CLASSES_ROOT, szTemp, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
				continue ;

			dwType = 0x00 ;
			dwSize = 511 ;
			memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
			RegQueryValueEx(hSubKey, TEXT(""), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
			/*
				ISSUE NO - 4 In Windows XP,registry optimizer same count of "Total entries repaired" is repeated multiple times.
				NAME - Niranjan Deshak. - 12th Jan 2015
				Description : Handle ie., hSubKey is commented but, It is required later after RegDeleteValue required further to delete entry.

			*/
			/*
				RegCloseKey( hSubKey ) ;
				hSubKey = NULL ;
			*/

			if( wcslen( szPath ) )
			{
				dwRet = CheckForPathExists( szPath ) ;
				if( dwRet )
				{
					//Delete value name
					if( RegDeleteValue( hSubKey, NULL ) == ERROR_SUCCESS )
					{
						memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
						if( hPredKey == HKEY_CURRENT_USER )
							swprintf(szTemp, TEXT("[Spywares_Entries]\tHKCU\\%s[%s]"), pSubKey, szPath ) ;
						else
							swprintf(szTemp, TEXT("[Spywares_Entries]\tHKLM\\%s[%s]"), pSubKey, szPath ) ;
						AddToLog( szTemp ) ;
						//AddToListView(pEntryName, szTemp, TEXT("Repaired") ) ;
					
						//LONG lRet = 0; 
						//lRet = RegDeleteValue( hSubKey, szValueName ) ;
						//lRet = RegDeleteValue( hSubKey, NULL ) ;
						//if(lRet == ERROR_SUCCESS)
						dwEntries++ ;
					}
				}
			}

			RegCloseKey( hSubKey ) ;
			hSubKey = NULL ;
		}

		//goto Cleanup ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckShellExecuteHooks", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hKey )
		RegCloseKey( hKey ) ;
	hKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	EnumRegValueDataForPathExist                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::EnumRegValueDataForPathExist(TCHAR *pSubKey, TCHAR *pEntryName, 
								   DWORD &dwEntries, HKEY hPredKey )
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwValues = 0x00, dwType = 0x00, i ;
	HKEY	hSubKey = NULL ;

	TCHAR	szValueName[512], szValueData[512], szTemp[2048] ;
	DWORD	dwValueName, dwValueData ;

	__try
	{

		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		bool	bRegInvalidFound = false;

		while( true )
		{
			bRegInvalidFound = false;
			dwValues = 0x00;
			RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
							NULL, NULL, NULL ) ;
			if( dwValues == 0x00 )
			{
				dwRet = 0x02 ;
				//goto Cleanup ;
				break;
			}

			for(i=0; i<dwValues; i++ )
			{
				memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
				memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;

				dwValueName = dwValueData = 511 ;

				RegEnumValue(	hSubKey, i, szValueName, &dwValueName, 0, &dwType, 
								(LPBYTE)szValueData, &dwValueData) ;
				if( !szValueName[0] )
					continue ;

				dwRet = CheckForPathExists( szValueData ) ;
				if( dwRet )
				{

					if( RegDeleteValue( hSubKey, szValueName ) == ERROR_SUCCESS )
					{
						//Delete value name
						memset(szTemp, 0x00, sizeof(szTemp) ) ;
						if( hPredKey == HKEY_CURRENT_USER )
							swprintf(szTemp, TEXT("[%s]\tHKCU\\%s[%s]"), pEntryName, pSubKey, szValueName ) ;
						else
							swprintf(szTemp, TEXT("[%s]\tHKLM\\%s[%s]"), pEntryName, pSubKey, szValueName ) ;

						AddToLog( szTemp ) ;
						//RegDeleteValue( hSubKey, szValueName ) ;
						dwEntries++ ;
						bRegInvalidFound = true;
					}
				}

				if( bRegInvalidFound )
					break;

			}

			if( !bRegInvalidFound )
					break;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::EnumRegValueDataForPathExist", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( hSubKey )
		RegCloseKey( hSubKey ) ;

	hSubKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	EnumRegValueNameForPathExist                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::EnumRegValueNameForPathExist(TCHAR *pSubKey, TCHAR *pEntryName, 
								   DWORD &dwEntries, HKEY hPredKey )
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwValues = 0x00, dwType = 0x00, i ;
	HKEY	hSubKey = NULL ;

	TCHAR	szValueName[512], szValueData[512], szTemp[1024] ;
	DWORD	dwValueName, dwValueData ;

	__try
	{

		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		bool	bRegInvalidFound = false;

		while( true )
		{
			bRegInvalidFound = false;
			dwValues = 0x00;
			RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
							NULL, NULL, NULL ) ;
			if( dwValues == 0x00 )
			{
				dwRet = 0x02 ;
				break;
			}

			for(i=0; i<dwValues; i++ )
			{
				memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
				memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;

				dwValueName = dwValueData = 511 ;

				RegEnumValue(	hSubKey, i, szValueName, &dwValueName, 0, &dwType, 
								(LPBYTE)szValueData, &dwValueData) ;
				if( !szValueName[0] )
					continue ;

				dwRet = CheckForPathExists( szValueName ) ;
				if( dwRet )
				{
					//Delete value name
				/*	if( hPredKey == HKEY_CURRENT_USER )
						sprintf(szTemp, "HKCU\\%s", pSubKey ) ;
					else
						sprintf(szTemp, "HKLM\\%s", pSubKey ) ;
					AddToListView("Shared DLLs", szTemp, "Repaired" ) ;
				*/
					if( RegDeleteValue( hSubKey, szValueName ) == ERROR_SUCCESS )
					{
						bRegInvalidFound = true;
						dwEntries++;

						memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
						if( hPredKey == HKEY_CURRENT_USER )
							wsprintf(szTemp, TEXT("[%s]\tHKCU\\%s[%s]"), pEntryName, pSubKey, szValueName ) ;
						else
							wsprintf(szTemp, TEXT("[%s]\tHKLM\\%s[%s]"), pEntryName, pSubKey, szValueName ) ;
						AddToLog( szTemp ) ;
					}
				}
				
				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
					break;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::EnumRegValueNameForPathExist1", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( hSubKey )
		RegCloseKey( hSubKey ) ;

	hSubKey = NULL ;

	return dwRet ;
}



/**********************************************************************************************************
*  Function Name  :	EnumRegValueNameForPathExist
*  Description    :
*  Author Name    : Vilas
*  Date           : 08 April 2015
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::EnumRegValueNameForPathExist(HKEY hSubKey, TCHAR *pSubKey, TCHAR *pEntryName, DWORD &dwEntries, HKEY hPredKey)
{

	DWORD	dwRet = 0x00;
	DWORD	dwValues = 0x00, dwType = 0x00, i;
	

	TCHAR	szValueName[512], szValueData[512], szTemp[1024];
	DWORD	dwValueName, dwValueData;

	__try
	{

		if (!hSubKey)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		bool	bRegInvalidFound = false;

		while (true)
		{
			bRegInvalidFound = false;
			dwValues = 0x00;
			RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
				NULL, NULL, NULL);
			if (dwValues == 0x00)
			{
				dwRet = 0x02;
				break;
			}

			for (i = 0; i<dwValues; i++)
			{
				memset(szValueName, 0x00, 512 * sizeof(TCHAR));
				memset(szValueData, 0x00, 512 * sizeof(TCHAR));

				dwValueName = dwValueData = 511;

				RegEnumValue(hSubKey, i, szValueName, &dwValueName, 0, &dwType,
					(LPBYTE)szValueData, &dwValueData);
				if (!szValueName[0])
					continue;

				TCHAR	*pTemp = wcschr(szValueName, '\\');
				if (!pTemp)
					continue;

				dwRet = CheckForPathExists(szValueName);
				if (dwRet)
				{
					//Delete value name
					/*	if( hPredKey == HKEY_CURRENT_USER )
					sprintf(szTemp, "HKCU\\%s", pSubKey ) ;
					else
					sprintf(szTemp, "HKLM\\%s", pSubKey ) ;
					AddToListView("Shared DLLs", szTemp, "Repaired" ) ;
					*/
					if (RegDeleteValue(hSubKey, szValueName) == ERROR_SUCCESS)
					{
						bRegInvalidFound = true;
						dwEntries++;

						memset(szTemp, 0x00, 1024 * sizeof(TCHAR));
						if (hPredKey == HKEY_CURRENT_USER)
							wsprintf(szTemp, TEXT("[%s]\tHKCU\\%s[%s]"), pEntryName, pSubKey, szValueName);
						else
							wsprintf(szTemp, TEXT("[%s]\tHKLM\\%s[%s]"), pEntryName, pSubKey, szValueName);
						AddToLog(szTemp);
					}
				}

				if (bRegInvalidFound)
					break;
			}

			if (!bRegInvalidFound)
				break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::EnumRegValueNameForPathExist2", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet;
}


/**********************************************************************************************************                     
*  Function Name  :	EnumRegValueNameDeleteAll                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::EnumRegValueNameDeleteAll(TCHAR *pSubKey, TCHAR *EntryName, 
								DWORD &dwEntries, HKEY hPredKey )
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwValues = 0x00, dwType = 0x00, i ;
	HKEY	hSubKey = NULL ;

	TCHAR	szValueName[512], szValueData[512], szTemp[1024] ;
	DWORD	dwValueName, dwValueData ;

	__try
	{

		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		bool	bRegInvalidFound = false;

		while( true )
		{

			bRegInvalidFound = false;
			dwValues = 0x00;
			RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
							NULL, NULL, NULL ) ;
			if( dwValues == 0x00 )
			{
				dwRet = 0x02 ;
				//goto Cleanup ;
				break;
			}

			for(i=0; i<dwValues; i++ )
			{
				memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
				memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;

				dwValueName = dwValueData = 511 ;

				RegEnumValue(	hSubKey, i, szValueName, &dwValueName, 0, &dwType, 
								(LPBYTE)szValueData, &dwValueData) ;
				if( !szValueName[0] )
					continue ;

				if( RegDeleteValue( hSubKey, szValueName ) == ERROR_SUCCESS )
				{
					//Delete value name
					memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
					if( hPredKey == HKEY_CURRENT_USER )
						wsprintf(szTemp, TEXT("[%s]\tHKCU\\%s\\%s"), EntryName, pSubKey, szValueName ) ;
					else
						wsprintf(szTemp, TEXT("[%s]\tHKLM\\%s\\%s"), EntryName, pSubKey, szValueName ) ;
					AddToLog( szTemp ) ;

					//RegDeleteValue( hSubKey, szValueName ) ;
					dwEntries++ ;

					bRegInvalidFound = true;
				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
					break;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::EnumRegValueNameDeleteAll", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( hSubKey )
		RegCloseKey( hSubKey ) ;

	hSubKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	EnumRegValueNameForDeletion                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::EnumRegValueNameForDeletion(TCHAR *pSubKey, TCHAR *pTypeInfo, DWORD &dwEntries, 
								  HKEY hPredKey )
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwValues = 0x00, dwType = 0x00, i ;
	HKEY	hSubKey = NULL ;

	TCHAR	szValueName[512], szValueData[512], szTemp[1024] ;
	DWORD	dwValueName, dwValueData ;

	__try
	{

		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		bool	bRegInvalidFound = false;

		while( true )
		{
			bRegInvalidFound = false;
			dwValues = 0x00;
			RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
							NULL, NULL, NULL ) ;
			if( dwValues == 0x00 )
			{
				dwRet = 0x02 ;
				//goto Cleanup ;
				break;
			}

			for(i=0; i<dwValues; i++ )
			{
				memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
				memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;

				dwValueName = dwValueData = 511 ;

				RegEnumValue(	hSubKey, i, szValueName, &dwValueName, 0, &dwType, 
								(LPBYTE)szValueData, &dwValueData) ;
				if( !szValueName[0] )
					continue ;

				//dwRet = 0x01 ;
				//if( dwRet )
				if( RegDeleteValue( hSubKey, szValueName ) == ERROR_SUCCESS )
				{
					bRegInvalidFound = true;
					memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
					if( hPredKey == HKEY_CURRENT_USER )
						wsprintf(szTemp, TEXT("[%s]\tHKCU\\%s\\%s"), pTypeInfo, pSubKey, szValueName ) ;
					else
						wsprintf(szTemp, TEXT("[%s]\\HKLM\\%s\\%s"), pTypeInfo, pSubKey, szValueName ) ;
					AddToLog( szTemp ) ;

					//RegDeleteValue( hSubKey, szValueName ) ;
					dwEntries++ ;
				}

				if( bRegInvalidFound )
					break;
			}

			if( !bRegInvalidFound )
					break;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::EnumRegValueNameForDeletion", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( hSubKey )
		RegCloseKey( hSubKey ) ;

	hSubKey = NULL ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckForPathExists                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckForPathExists(TCHAR *pPath )
{

	__try
	{
		TCHAR	szPath[512] = {0} ;
		TCHAR	*pTemp = NULL ;


		if( pPath[0] == '"' )
			wcscpy(szPath, &pPath[1] ) ;
		else
			wcscpy(szPath, pPath ) ;
		_wcsupr( szPath ) ;

		pTemp = wcsstr(szPath, TEXT(".EXE") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT(".EXE") ) ;
			*pTemp = '\0' ;
		}

		pTemp = wcsstr(szPath, TEXT(".SYS") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT(".SYS") ) ;
			*pTemp = '\0' ;
		}

		pTemp = wcsstr(szPath, TEXT(".DLL") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT(".DLL") ) ;
			*pTemp = '\0' ;
		}

		pTemp = wcsstr(szPath, TEXT(".OCX") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT(".OCX") ) ;
			*pTemp = '\0' ;
		}

		if( PathFileExists( szPath ) )
			return 0 ;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckForPathExists", 0, 0, true, SECONDLEVEL);
		return 0;
	}

	return 1 ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckServiceValidPath                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckServiceValidPath( HKEY hSubKey, TCHAR *pPath )
{
	DWORD	dwRet = 0x01;

	__try
	{
		TCHAR	szPath[512] = {0} ;
		TCHAR	szTemp[512] = {0} ;
		TCHAR	*pTemp = NULL ;

		if( pPath[0] == '"' )
			wcscpy(szTemp, &pPath[1] ) ;
		else
			wcscpy(szTemp, pPath ) ;
		_wcsupr( szTemp ) ;
		wcscpy(szPath, szTemp ) ;

		if( !wcschr(szTemp, '.' ) )
			return 0 ;

		pTemp = wcsstr( szTemp, TEXT("SYSTEM32\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("SYSTEM32\\") ) ;
			wsprintf(szPath, TEXT("%s\\%s"), szSystemDir, pTemp ) ;
		}

		pTemp = wcsstr( szTemp, TEXT("%PROGRAMFILES%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%PROGRAMFILES%\\") ) ;
			wsprintf(szPath, TEXT("%s\\%s"), szProgramDir, pTemp ) ;
		}
		if( m_bIsWow64 )
		{
			pTemp = wcsstr( szTemp, TEXT("%PROGRAMFILES(X86)%\\") ) ;
			if( pTemp )
			{
				pTemp += wcslen( TEXT("%PROGRAMFILES(X86)%\\") ) ;
				wsprintf(szPath, TEXT("%s\\%s"), szProgramDirX86, pTemp ) ;
			}
		}

		pTemp = wcsstr( szTemp, TEXT("%COMMONPROGRAMFILES%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%COMMONPROGRAMFILES%\\") ) ;
			wsprintf(szPath, TEXT("%s\\%s"), szCommProgram, pTemp ) ;

			if (PathFileExists(szPath))
			{
				dwRet = 0x00;
				return dwRet;
			}

			//If OS is 64 bit, we need to check path in 32 also.
			//Modified by Vilas on 10 April 2015
			if (m_bIsx64)
			{
				wsprintf(szPath, TEXT("%s\\%s"), m_szCommProgramX86, pTemp);
				if (PathFileExists(szPath))
				{
					dwRet = 0x00;
					return dwRet;
				}
			}
		}

		pTemp = wcsstr( szTemp, TEXT("%SYSTEMROOT%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%SYSTEMROOT%\\") );
			wsprintf(szPath, TEXT("%s\\%s"), szWindowsDir, pTemp ) ;
		}

		pTemp = wcsstr( szTemp, TEXT("%ALLUSERSPROFILE%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%ALLUSERSPROFILE%\\") );
			wsprintf(szPath, TEXT("%s\\%s"), szProgramData, pTemp ) ;
		}

		pTemp = wcsstr( szTemp, TEXT("%PROGRAMDATA%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%PROGRAMDATA%\\") );
			wsprintf(szPath, TEXT("%s\\%s"), szProgramData, pTemp ) ;
		}

		pTemp = wcsstr( szTemp, TEXT("%USERPROFILE%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%USERPROFILE%\\") );
			wsprintf(szPath, TEXT("%s\\%s"), szUserProfile, pTemp ) ;
		}

		pTemp = wcsstr( szTemp, TEXT("%TEMP%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%TEMP%\\") );
			wsprintf(szPath, TEXT("%s\\%s"), szTempLocal, pTemp ) ;
		}

		pTemp = wcsstr( szTemp, TEXT("%PUBLIC%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%PUBLIC%\\") );
			wsprintf(szPath, TEXT("%s\\%s"), szPublic, pTemp ) ;
		}

		pTemp = wcsstr( szTemp, TEXT("%APPDATA%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%APPDATA%\\") );
			wsprintf(szPath, TEXT("%s\\%s"), szAppData, pTemp ) ;
		}

		pTemp = wcsstr( szTemp, TEXT("%WINDIR%\\") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT("%WINDIR%\\") );
			wsprintf(szPath, TEXT("%s\\%s"), szWindowsDir, pTemp ) ;
		}

		if( _memicmp(pPath, TEXT("\\??\\"), wcslen(TEXT("\\??\\")) ) == 0 )
			wcscpy(szPath, &szTemp[wcslen(TEXT("\\??\\"))] ) ;


		pTemp = wcsstr(szPath, TEXT(".EXE") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT(".EXE") ) ;
			*pTemp = '\0' ;
		}

		pTemp = wcsstr(szPath, TEXT(".SYS") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT(".SYS") ) ;
			*pTemp = '\0' ;
		}

		pTemp = wcsstr(szPath, TEXT(".DLL") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT(".DLL") ) ;
			*pTemp = '\0' ;
		}

		pTemp = wcsstr(szPath, TEXT(".OCX") ) ;
		if( pTemp )
		{
			pTemp += wcslen( TEXT(".OCX") ) ;
			*pTemp = '\0' ;
		}

	/*	if( m_bIsWow64 )
			Wow64DisableWow64FsRedirection( &OldValue ) ;
	*/
		if( PathFileExists( szPath ) )
			dwRet = 0x00 ;
	/*
		if( m_bIsWow64 )
			Wow64RevertWow64FsRedirection( OldValue ) ;
	*/

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckServiceValidPath", 0, 0, true, SECONDLEVEL);
		return 0;
	}

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckValidExplorerExtension                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::CheckValidExplorerExtension( HKEY hSubKey, TCHAR *pSubKey )
{

	__try
	{

		HKEY	hSubSubKey = NULL, hOpenWithListKey = NULL ;
		TCHAR	szSubKeyName[512] = {0} ;
		TCHAR	szOpenWithListValue[512] = {0} ;

		DWORD	dwSubKeys = 0x00, dwSize = 511 ;

		RegQueryInfoKey( hSubKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
									NULL, NULL, NULL ) ;
		if( dwSubKeys != 0x01 )
			return 1 ;


		RegEnumKeyEx(hSubKey, 0, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
		if( _wcsicmp(szSubKeyName, TEXT("OpenWithList")) != 0 )
			return 2 ;


		if( RegOpenKeyEx(hSubKey, TEXT("OpenWithList"), 0, KEY_ALL_ACCESS, &hSubSubKey ) != ERROR_SUCCESS )
			return 3 ;

		DWORD	dwValues = 0x00, dwType = 0x00 ;

		RegQueryInfoKey(	hSubSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
							NULL, NULL, NULL ) ;
		if( dwValues != 0x00 )
		{
			RegCloseKey( hSubSubKey ) ;
			return 4 ;
		}

		dwSize = 511 ;
		RegQueryValueEx(hOpenWithListKey, TEXT(""), 0, &dwType, (LPBYTE)szOpenWithListValue, &dwSize) ;
		RegCloseKey( hSubSubKey ) ;

		if( wcslen(szOpenWithListValue) == 0 )
			return 0 ;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckValidExplorerExtension", 0, 0, true, SECONDLEVEL);
		return 6;
	}

	return 5 ;

}

/**********************************************************************************************************                     
*  Function Name  :	SetRegSZValue                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::SetRegSZValue(TCHAR *pSubKey, TCHAR *pValueName, TCHAR *pValueData, bool bCheck, HKEY hPredKey )
{
	DWORD	dwRet = 0x00 ;
	HKEY	hSubKey = NULL;

	__try
	{
		
		TCHAR	szValueData[512] = {0} ;
		DWORD	dwSize = 512, dwType = 0x00 ;

		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		RegQueryValueEx(hSubKey, pValueName, 0, &dwType, (LPBYTE)szValueData, &dwSize) ;
		dwSize = static_cast<DWORD>(wcslen(pValueData)*sizeof(TCHAR) + 2 );
		if( bCheck )
		{
			if( _wcsicmp(szValueData, pValueData) != 0 )
				dwRet = RegSetValueEx(hSubKey, pValueName, 0, dwType, (LPBYTE)pValueData, dwSize ) ;
			else
				dwRet = 0x02 ;
		}
		else
			dwRet = RegSetValueEx(hSubKey, pValueName, 0, dwType, (LPBYTE)pValueData, dwSize ) ;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x03;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::SetRegSZValue", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( hSubKey )
		RegCloseKey( hSubKey ) ;

	hSubKey = NULL ;

	return dwRet ;
}



/**********************************************************************************************************
*  Function Name  :	SetRegDWORDValue
*  Description    :
*  Author Name    : Vilas & Prajakta
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD iSpySrvMgmt_RegOpt::SetRegDWORDValue(TCHAR *pSubKey, TCHAR *pValueName, DWORD dwSetValueData, bool bCheck, HKEY hPredKey)
{
	DWORD	dwRet = 0x00;
	HKEY	hSubKey = NULL;

	__try
	{

		DWORD	dwValueData = 0x00;
		DWORD	dwSize = 0x00, dwType = 0x00;

		if (RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey) != ERROR_SUCCESS)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		dwSize = sizeof(DWORD);
		RegQueryValueEx(hSubKey, pValueName, 0, &dwType, (LPBYTE)dwValueData, &dwSize);
		
		dwSize = sizeof(DWORD);
		if (bCheck)
		{
			if (dwSetValueData != dwValueData )
				dwRet = RegSetValueEx(hSubKey, pValueName, 0, dwType, (LPBYTE)&dwSetValueData, dwSize);
			else
				dwRet = 0x02;
		}
		else
			dwRet = RegSetValueEx(hSubKey, pValueName, 0, dwType, (LPBYTE)&dwSetValueData, dwSize);

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x03;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::SetRegDWORDValue", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if (hSubKey)
		RegCloseKey(hSubKey);

	hSubKey = NULL;

	return dwRet;
}

/**********************************************************************************************************                     
*  Function Name  :	DeleteInvalidKey                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL iSpySrvMgmt_RegOpt::DeleteInvalidKey( HKEY hKeyRoot, LPTSTR lpSubKey )
{
	//Issue		: Registry optimizer some entries not getting clean
	//Resolved	: Vilas
	//Date		: 05 / Feb / 2015

	__try
	{

		TCHAR	szDelKey[2048] = {0} ;

		lstrcpy(szDelKey, lpSubKey) ;
		if( RegDelnodeRecurse(hKeyRoot, szDelKey) )
			return TRUE;

		Sleep( 10 );
		ZeroMemory(szDelKey, sizeof(szDelKey) );
		lstrcpy(szDelKey, lpSubKey) ;
		if( RegDelnodeRecurse(hKeyRoot, szDelKey) )
			return TRUE;

		Sleep( 100 );
		ZeroMemory(szDelKey, sizeof(szDelKey) );
		lstrcpy(szDelKey, lpSubKey) ;
		if( RegDelnodeRecurse(hKeyRoot, szDelKey) )
			return TRUE;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::DeleteInvalidKey", 0, 0, true, SECONDLEVEL);
	}

	return FALSE;
}

/**********************************************************************************************************                     
*  Function Name  :	RegDelnodeRecurse                                                     
*  Description    :	
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL iSpySrvMgmt_RegOpt::RegDelnodeRecurse( HKEY hKeyRoot, LPTSTR lpSubKey )
{
    LPTSTR		lpEnd ;
    LONG		lResult ;
    DWORD		dwSize ;
    TCHAR		szName[MAX_PATH] = {0} ;
    HKEY		hKey = NULL ;
    FILETIME	ftWrite ;

    // First, see if we can delete the key without having
    // to recurse.

	__try
	{

		lResult = RegDeleteKey(hKeyRoot, lpSubKey);

		if (lResult == ERROR_SUCCESS) 
			return TRUE;

		lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

		if (lResult != ERROR_SUCCESS) 
		{
			if (lResult == ERROR_FILE_NOT_FOUND)
			{
				//printf("Key not found.\n");
				return TRUE;
			} 
			else
			{
				//printf("Error opening key.\n");
				return FALSE;
			}
		}

		// Check for an ending slash and add one if it is missing.

		lpEnd = lpSubKey + lstrlen(lpSubKey) ;

		if (*(lpEnd - 1) != TEXT('\\')) 
		{
			*lpEnd =  TEXT('\\') ;
			lpEnd++;
			*lpEnd =  TEXT('\0') ;
		}

		// Enumerate the keys

		dwSize = MAX_PATH ;
		lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
							   NULL, NULL, &ftWrite) ;

		if (lResult == ERROR_SUCCESS) 
		{
			do
			{

				lstrcpy (lpEnd, szName) ;
				if( !RegDelnodeRecurse(hKeyRoot, lpSubKey) )
				{
					break;
				}

				dwSize = MAX_PATH ;

				lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
									   NULL, NULL, &ftWrite);

			} while (lResult == ERROR_SUCCESS);
		}

		lpEnd--;
		*lpEnd = TEXT('\0');

		RegCloseKey( hKey ) ;
		hKey = NULL ;

		// Try again to delete the key.

		lResult = RegDeleteKey(hKeyRoot, lpSubKey) ;

		if( lResult == ERROR_SUCCESS )
			return TRUE ;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::RegDelnodeRecurse", 0, 0, true, SECONDLEVEL);
	}

    return FALSE ;
}

/**********************************************************************************************************                     
*  Function Name  :	GetRepairedEntriesTotalCount                                                     
*  Description    :	Returns total number of registry entries repaired
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 27 Jan 2014
**********************************************************************************************************/

DWORD iSpySrvMgmt_RegOpt::GetRepairedEntriesTotalCount()
{
	return (dwActiveXEntries + dwUnInstallEntries + dwFontEntries +
			dwSharedDLLs + dwAppPathEntries + dwHelpFilesEntries +
			dwStartupRepairedEntries + dwServicesEntries +
			dwExtensionEntries + dwRootKitEntries +	dwRogueEntries +
			dwWormEntries +	dwSpywareEntries + dwAdwareEntries +
			dwKeyLoggerEntries + dwBHOEntries + dwExplorerEntries +	
			dwIEEntries ) ;

}


/**********************************************************************************************************                     
*  Function Name  :	AddToLog                                                     
*  Description    :	Adds registry optimization information in log(REGISTRYOPTIMIZER.log)
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void iSpySrvMgmt_RegOpt::AddToLog(TCHAR *pText, bool bTime )
{
	TCHAR	szLogPath[512] = {0} ;
	TCHAR	szTemp[128] = {0} ;

	FILE	*Fp = NULL ;

	SYSTEMTIME	ST = {0} ;

	__try
	{

		GetLocalTime( &ST ) ;
	/*	wsprintf(szLogPath, TEXT("%s\\Logs\\RegOpt_%02d%02d%04d.log"), 
				szAppDataPath, ST.wDay, ST.wMonth, ST.wYear ) ;
	*/
		TCHAR szModulePath[MAX_PATH] = {0};
		GetModuleFileName(NULL, szModulePath, MAX_PATH);

		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		wsprintf(szLogPath, TEXT("%s\\Log\\REGISTRYOPTIMIZER.LOG"), szModulePath) ;

		Fp = _wfopen( szLogPath, TEXT("at+") ) ;
		if( !Fp )
			goto Cleanup ;

		fseek(Fp, 0L, SEEK_END);

		if (bTime)
		{
			if (ST.wHour > 12)
				wsprintf(szTemp, TEXT("[%02d:%02d:%02d PM]"), (ST.wHour - 12), ST.wMinute, ST.wSecond);
			else
				wsprintf(szTemp, TEXT("[%02d:%02d:%02d AM]"), ST.wHour, ST.wMinute, ST.wSecond);

			//fseek(Fp, 0L, SEEK_END);
			fwprintf(Fp, TEXT("%s\t%s\n"), szTemp, pText);
		}
		else
		{
			fwprintf(Fp, TEXT("%s\n"), pText);
		}

		fclose(Fp) ;
		Fp = NULL ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup:

	if( Fp )
		fclose( Fp ) ;

	Fp = NULL ;

}

void iSpySrvMgmt_RegOpt::CalculatePercentage(DWORD dwPercentage)
{
	ITIN_MEMMAP_DATA iTinMemMap = {0};
	iTinMemMap.dwFirstValue = dwPercentage;
	iTinMemMap.dwSecondValue = GetRepairedEntriesTotalCount();
	m_RegOpt_MemMap_Server_Obj.UpdateServerMemoryMappedFile( &iTinMemMap, sizeof(iTinMemMap) ) ;
}



/***************************************************************************
Function Name  : AddUserAndSystemInfoToLog
Description    : Adds Computer name, logged user name and OS details to log at the top
Author Name    : Vilas Suvarnakar
SR_NO		   :
Date           : 04 May 2015
Modification   :
****************************************************************************/
void iSpySrvMgmt_RegOpt::AddUserAndSystemInfoToLog()
{

	TCHAR	szModulePath[MAX_PATH] = { 0 };

	if (!GetModulePath(szModulePath, MAX_PATH))
	{
		return;
	}

	TCHAR	szLogPath[512] = { 0 };
	TCHAR	szTemp[512] = { 0 };

	swprintf_s(szLogPath, _countof(szLogPath), L"%s\\Log\\%s", szModulePath, L"REGISTRYOPTIMIZER.LOG");

	if (PathFileExists(szLogPath))
		return;

	WardWizSystemInfo	objSysInfo;

	objSysInfo.GetSystemInformation();

	LPCTSTR		lpSystemName = objSysInfo.GetSystemName();
	LPCTSTR		lpUserName = objSysInfo.GetUserNameOfSystem();
	LPCTSTR		lpOSDetails = objSysInfo.GetOSDetails();

	

	ZeroMemory(szTemp, sizeof(szTemp));
	swprintf_s(szTemp, _countof(szTemp), L"%s\n", L"--------------------------------------------------------------------------------------------------------");
	//AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
	AddLogEntryWithPath(szLogPath, szTemp, 0, 0, false, SECONDLEVEL);
	if (lpSystemName)
	{
		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* %s:%s\n", m_objWardwizLangManager.GetString("IDS_COMPUTER_NAME"),lpSystemName);
		//AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		AddLogEntryWithPath(szLogPath, szTemp, 0, 0, false, SECONDLEVEL);
	}

	if (lpUserName)
	{
		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* %s:%s\n", m_objWardwizLangManager.GetString("IDS_LOGGED_USER_NAME"), lpUserName);
		//AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		AddLogEntryWithPath(szLogPath, szTemp, 0, 0, false, SECONDLEVEL);
	}

	if (lpOSDetails)
	{
		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* %s:%s\n", m_objWardwizLangManager.GetString("IDS_OS_DETAILS"), lpOSDetails);
		//AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		AddLogEntryWithPath(szLogPath, szTemp, 0, 0, false, SECONDLEVEL);
	}

	ZeroMemory(szTemp, sizeof(szTemp));
	swprintf_s(szTemp, _countof(szTemp), L"%s\n\n", L"--------------------------------------------------------------------------------------------------------");
	//AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
	AddLogEntryWithPath(szLogPath, szTemp, 0, 0, false, SECONDLEVEL);
}


void iSpySrvMgmt_RegOpt::AddUserAndSystemInfoToLogSEH()
{
	__try
	{
		AddUserAndSystemInfoToLog();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddToLog(L"### Exception in VibraniumMgmt_RegOpt::AddUserAndSystemInfoToLogSEH");
		AddLogEntry(L"### Exception in VibraniumMgmt_RegOpt::AddUserAndSystemInfoToLogSEH", 0, 0, true, SECONDLEVEL);
	}
}