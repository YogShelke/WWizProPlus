#ifndef _ISPYSRVMGMT_H_
#define _ISPYSRVMGMT_H_
#pragma once
#include <winsvc.h>  

class CISpySrvMgmt
{
public:
	CISpySrvMgmt() ;
	~CISpySrvMgmt() ;

	DWORD InstallService( TCHAR *pszSrvName, TCHAR *pszSrvDisplayName, TCHAR *pszSrvPath ) ;
	DWORD UnInstallService( TCHAR *pszSrvName ) ;
	DWORD StartServiceManually( TCHAR *pszSrvName ) ;
	DWORD StopServiceManually( TCHAR *pszSrvName ) ;

	DWORD SetServiceFailureAction( SC_HANDLE hService ) ;

	DWORD QueryServiceStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus) ;
	DWORD QueryServiceStartStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus) ;
	DWORD CreateUserService(TCHAR *pszSrvName, TCHAR *pszSrvDisplayName, TCHAR *pszSrvPath);
	DWORD InstallService(PWSTR pszSrvName,
		PWSTR pszSrvDisplayName,
		PWSTR pszSrvPath,
		DWORD dwStartType,
		PWSTR pszDependencies,
		PWSTR pszAccount,
		PWSTR pszPassword);
//protected:

	//TCHAR		m_szSrvName[64] ;
	//SC_HANDLE	m_hService ;

};


#endif
