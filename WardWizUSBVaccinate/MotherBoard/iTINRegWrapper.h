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
	DWORD SetRegistryDWORDData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD dwValue );
	//bool RegistryRedirect(bool bEnable);

	//DWORD DeleteRegistryKey( HKEY hRootKey, TCHAR *pKeyName ) ;
	//BOOL RegDelnodeRecurse( HKEY hKeyRoot, LPTSTR lpSubKey ) ;
//protected:
};

//HOW TO USE THIS CLASS
/*

	TCHAR	pSubKeyName[] = L"SOFTWARE\\ISpy Antivirus" ;
	BYTE	bArr[] = {0x10, 0x20, 0x30 } ;
	BYTE	bOutArr[0x03] = { 0 } ;

	CITinRegWrapper iSpyReg ;

	iSpyReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, pSubKeyName, L"DWORD", 0x10 ) ;
	iSpyReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, pSubKeyName, L"STRING", L"10" ) ;
	iSpyReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, pSubKeyName, L"BINARY", bArr, sizeof(bArr) ) ;

	DWORD	dwSize = sizeof(bOutArr) ;

	iSpyReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, pSubKeyName, L"BINARY", bOutArr, dwSize ) ;

	TCHAR	szSubKey[] = L"SOFTWARE\\ISpy Antivirus\\Vilas" ;

	iSpyReg.DeleteRegistryKey( HKEY_LOCAL_MACHINE, szSubKey ) ;


*/

#endif
