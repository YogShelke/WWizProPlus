#include "StdAfx.h"
#include "Registry.h"

CRegistry::CRegistry(void)
{
}

CRegistry::~CRegistry(void)
{
}

CString CRegistry::GetStringValue( HKEY hKeyArg, LPCTSTR keyNameArg, LPCTSTR valNameArg )
{	
	HKEY	hSubKey = NULL ;
	TCHAR	szModulePath[MAX_PATH] = {0};

	if( RegOpenKey(hKeyArg, keyNameArg, &hSubKey ) != ERROR_SUCCESS )
		return L"";

	DWORD	dwSize = 511 ;
	DWORD	dwType = 0x00 ;

	RegQueryValueEx(hSubKey, valNameArg, 0, &dwType, (LPBYTE)szModulePath, &dwSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if(_tcslen(szModulePath) > 0)
	{
		return CString(szModulePath) ;
	}
	return L"";
}

bool CRegistry::SetStringValue(HKEY hKey, LPCTSTR keyNameArg, LPCTSTR valNameArg, LPCTSTR szData)
{
	LONG openRes = RegOpenKey(HKEY_LOCAL_MACHINE, keyNameArg, &hKey);
	if ( openRes != ERROR_SUCCESS)
	{
		RegCreateKeyEx(HKEY_LOCAL_MACHINE,keyNameArg,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,(LPDWORD)REG_CREATED_NEW_KEY);
	}
	LONG setRes = RegSetValueEx (hKey ,valNameArg, 0, REG_SZ, (LPBYTE)szData, static_cast<DWORD>(_tcslen(valNameArg)) + 1);
	if(setRes == ERROR_MORE_DATA)
	{
		return false;
		AddLogEntry(L"Error writing to Registry.");
	}
	return true;
}
