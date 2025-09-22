#ifndef _ISPYSRVMGMT_REGOPT_H_
#define _ISPYSRVMGMT_REGOPT_H_

#include <Windows.h>
#include <stdio.h>
#include "RegOptStruct.h"
#include "iSpyMemMapServer.h"
#include "PipeConstants.h"

class iSpySrvMgmt_RegOpt
{
public:
	iSpySrvMgmt_RegOpt() ;
	~iSpySrvMgmt_RegOpt() ;

	REGOPTSCANOPTIONS			RegOpt_ScanOpt ;
	iSpyServerMemMap_Server		m_RegOpt_MemMap_Server_Obj;

	bool InitializeVariables() ;
	void IsWow64() ;

	DWORD ScanAndRepairRegistryEntries(LPREGOPTSCANOPTIONS pScanOpt ) ;

	//DWORD CheckInvalidActiveXEntries() ;
	//DWORD CheckInvalidUninstallEntries() ;
	//DWORD CheckInvalidFontEntries() ;
	DWORD CheckInvalidSharedLibraries( ) ;
	DWORD CheckInvalidApplicationPaths( LPCTSTR lpszSubKey) ;
	DWORD CheckInvalidHelpFile( ) ;
	DWORD CheckInvalidStartupEntries( ) ;
	DWORD CheckInvalidServices( ) ;
	DWORD CheckInvalidExtensions( ) ;
	DWORD CheckRootKitEntries( ) ;
	DWORD CheckRogueApplications( ) ;
	DWORD CheckWormEntries( ) ;
	DWORD CheckInvalidSpywares() ;
	DWORD CheckInvalidAdwares() ;
	DWORD CheckKeyLogger() ;
	DWORD CheckInvalidBHO() ;
	DWORD CheckInvalidExplorer() ;
	DWORD CheckInvalidIExplorer() ;
	DWORD CheckShellExecuteHooks(TCHAR *pSubKey, DWORD &dwEntries, HKEY hPredKey = HKEY_LOCAL_MACHINE ) ;
	DWORD EnumRegValueDataForPathExist(TCHAR *pSubKey, TCHAR *, DWORD &dwEntries, HKEY hPredKey = HKEY_LOCAL_MACHINE ) ;
	DWORD EnumRegValueNameForPathExist(TCHAR *pSubKey, TCHAR *pEntryName, DWORD &dwEntries, HKEY hPredKey = HKEY_LOCAL_MACHINE );
	DWORD EnumRegValueNameDeleteAll(TCHAR *pSubKey, TCHAR *EntryName, DWORD &dwEntries, HKEY hPredKey ) ;
	DWORD EnumRegValueNameForDeletion(TCHAR *pSubKey, TCHAR *pTypeInfo, DWORD &dwEntries, HKEY hPredKey ) ;
	DWORD CheckForPathExists(TCHAR *pPath ) ;
	DWORD CheckServiceValidPath( HKEY hSubKey, TCHAR *pPath ) ;
	DWORD CheckValidExplorerExtension( HKEY hSubKey, TCHAR *pSubKey ) ;
	DWORD SetRegSZValue(TCHAR *pSubKey, TCHAR *pValueName, TCHAR *pValueData, bool, HKEY hPredKey= HKEY_LOCAL_MACHINE );
	BOOL DeleteInvalidKey( HKEY hKeyRoot, LPTSTR lpSubKey ) ;
	BOOL RegDelnodeRecurse( HKEY hKeyRoot, LPTSTR lpSubKey ) ;
	void AddToLog(TCHAR *pText, bool bTime = true ) ;
	void CalculatePercentage(DWORD dwPercentage);
	DWORD GetRepairedEntriesTotalCount() ;

	//Handling for Current user key
	//Added on 08 April 2015 by Vilas
	DWORD CheckForCurrentUserEntries(LPTSTR lpszSubKey, DWORD &dwEntries);
	DWORD EnumRegValueNameForPathExist(HKEY hSubKey, TCHAR *pSubKey, TCHAR *pEntryName, DWORD &dwEntries, HKEY hPredKey = HKEY_LOCAL_MACHINE);

	//Added on 09 April 2015 by Vilas
	//Handle 32 bit uninstall entries
	DWORD CheckInvalidActiveXEntries(HKEY hPredefKey, LPCTSTR lpszSubKey);
	DWORD CheckInvalidUninstallEntries(LPCTSTR lpszSubKey);
	DWORD CheckForInvalidClassID(LPCTSTR lpszSubKey, DWORD &dwEntries);

	//Added on 11 April 2015 by Vilas
	//Handle 32 bit uninstall entries
	DWORD CheckInvalidFontEntries(LPCTSTR lpszSubKey);
	DWORD SetRegDWORDValue(TCHAR *pSubKey, TCHAR *pValueName, DWORD dwValueData, bool, HKEY hPredKey = HKEY_LOCAL_MACHINE);


	// Adding User, computer name and OS details
	//Added by Vilas on 05 May 2015
	void AddUserAndSystemInfoToLog();
	void AddUserAndSystemInfoToLogSEH();

protected:

	DWORD	dwActiveXEntries ;
	DWORD	dwUnInstallEntries ;
	DWORD	dwFontEntries ;
	DWORD	dwSharedDLLs ;
	DWORD	dwAppPathEntries ;
	DWORD	dwHelpFilesEntries ;
	DWORD	dwStartupRepairedEntries ;
	DWORD	dwServicesEntries ;
	DWORD	dwExtensionEntries ;
	DWORD	dwRootKitEntries ;
	DWORD	dwRogueEntries ;
	DWORD	dwWormEntries ;
	DWORD	dwSpywareEntries ;
	DWORD	dwAdwareEntries ;
	DWORD	dwKeyLoggerEntries ;
	DWORD	dwBHOEntries ;
	DWORD	dwExplorerEntries ;
	DWORD	dwIEEntries ;

	int		dwPercentage ;


	TCHAR	szWindowsDir[256] ;
	TCHAR	szSystemDir[256] ;
	TCHAR	szProgramDir[256] ;
	TCHAR	szProgramDirX86[256] ;
	TCHAR	szApplPath[512] ;
	TCHAR	szAppDataPath[512] ;
	TCHAR	szCommProgram[512] ;
	TCHAR	szProgramData[256] ;
	TCHAR	szUserProfile[256] ;
	TCHAR	szTempLocal[256] ;
	TCHAR	szPublic[256] ;
	TCHAR	szAppData[256] ;

	bool	bVistaOnward ;
	BOOL	m_bIsWow64 ;
	PVOID	OldValue ;

	//Handling for 64 bit abd 32 bit
	//Added on 08 April 2015 by Vilas
	bool	m_bIsx64;

	TCHAR	m_szCommProgramX86[256];

	CWardwizLangManager m_objWardwizLangManager;
};


#endif
