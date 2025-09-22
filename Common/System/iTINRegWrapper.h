#pragma once

#ifndef _ITIN_REGISTRYWRAPPER_H_
#define _ITIN_REGISTRYWRAPPER_H_

#include <Windows.h>
#include <stdio.h>

class CITinRegWrapper
{
public:
	CITinRegWrapper() ;
	~CITinRegWrapper() ;


	DWORD SetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD dwData ) ;
	DWORD SetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, TCHAR *pszData) ;
	DWORD SetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, LPBYTE pbData, DWORD dwSize ) ;
	DWORD GetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, PVOID pszData, DWORD &dwSize ) ;
	DWORD GetRegistryValueData32( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, PVOID pszData, DWORD &dwSize ) ;
	DWORD GetRegistryDWORDData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD& dwValue);
	DWORD DelRegistryValueName( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName);
	//bool RegistryRedirect(bool bEnable);

	//DWORD DeleteRegistryKey( HKEY hRootKey, TCHAR *pKeyName ) ;
	BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey);
	//protected:
};

#endif
