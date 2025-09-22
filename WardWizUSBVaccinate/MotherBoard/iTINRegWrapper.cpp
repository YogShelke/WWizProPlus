#include "stdafx.h"
#include "iTinRegWrapper.h"
#include <shlwapi.h>
#include <tchar.h>

CITinRegWrapper::CITinRegWrapper()
{
}
CITinRegWrapper::~CITinRegWrapper()
{
}

DWORD CITinRegWrapper::SetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD dwData )
{
	DWORD	dwRet = 0x00 ;
	HKEY	hSubKey = NULL ;

	//__try
	try
	{

		RegCreateKeyEx(hRootKey, pKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL ) ;
		if( !hSubKey )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		dwRet = RegSetValueEx(hSubKey, pValueName, 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD) ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		//AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

DWORD CITinRegWrapper::SetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, TCHAR *pszData)
{
	DWORD	dwRet = 0x00, dwSize = 0x00 ;
	HKEY	hSubKey = NULL ;

	try
	{

		RegCreateKeyEx(hRootKey, pKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL ) ;
		if( !hSubKey )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		dwSize = static_cast<DWORD>(wcslen(pszData)*sizeof(TCHAR));
		dwRet = RegSetValueEx(hSubKey, pValueName, 0, REG_SZ, (LPBYTE)pszData, dwSize ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

	}
	//__except(CWardwizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		//AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

DWORD CITinRegWrapper::SetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, LPBYTE pbData, DWORD dwSize )
{
	DWORD	dwRet = 0x00 ;
	HKEY	hSubKey = NULL ;

	try
	{

		RegCreateKeyEx(hRootKey, pKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL ) ;
		if( !hSubKey )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		dwRet = RegSetValueEx(hSubKey, pValueName, 0, REG_BINARY, (LPBYTE)pbData, dwSize ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

	}
//	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		//AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}


DWORD CITinRegWrapper::SetRegistryDWORDData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD dwValue )
{
	DWORD	dwRet = 0x00 ;
	HKEY	hSubKey = NULL ;

	try
	{

		RegOpenKeyEx(hRootKey, pKeyName, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &hSubKey ) ;
		if( !hSubKey )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		DWORD	dwSize = sizeof(DWORD);

		dwRet = RegSetValueEx(hSubKey, pValueName, 0, REG_DWORD, (LPBYTE)&dwValue, dwSize ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

		if( dwRet != ERROR_SUCCESS )
			dwRet = 0x02;

	}
//	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		//AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

DWORD CITinRegWrapper::GetRegistryDWORDData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD& dwValue)
{
	HKEY  hKey = NULL;
	DWORD dwDWORDValue = 0x00;
	DWORD dwvalueSize = sizeof(DWORD);
	DWORD dwType = REG_DWORD;

	dwValue = 0x00;

	if (RegOpenKeyEx(hRootKey, pKeyName, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
	{
		return 0x01;
	}

	if(RegQueryValueEx(hKey, pValueName, NULL, &dwType,(LPBYTE)&dwDWORDValue, &dwvalueSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return 0x02;
	}

	RegCloseKey(hKey);

	dwValue = dwDWORDValue;
	return 0x00;
}

DWORD CITinRegWrapper::GetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, PVOID pszData, DWORD &dwSize )
{
	DWORD	dwRet = 0x00, dwRegType = 0x00, dwDataSize = 0x00 ;
	HKEY	hSubKey = NULL ;

	try
	{

		if( IsBadWritePtr(pszData, dwSize) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		RegOpenKeyEx(hRootKey, pKeyName, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey ) ;
		if( !hSubKey )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		dwDataSize = dwSize ;
		dwRet = RegQueryValueEx(hSubKey, pValueName, 0, &dwRegType, (LPBYTE)pszData, &dwDataSize ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

		if( !dwRet )
		dwSize = dwDataSize ;

	}
//	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		//AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

DWORD CITinRegWrapper::GetRegistryValueData32( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, PVOID pszData, DWORD &dwSize )
{
	DWORD	dwRet = 0x00, dwRegType = 0x00, dwDataSize = 0x00 ;
	HKEY	hSubKey = NULL ;

	try
	{

		if( IsBadWritePtr(pszData, dwSize) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		RegOpenKeyEx(hRootKey, pKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) ;
		if( !hSubKey )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		dwDataSize = dwSize ;
		dwRet = RegQueryValueEx(hSubKey, pValueName, 0, &dwRegType, (LPBYTE)pszData, &dwDataSize ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

		if( !dwRet )
		dwSize = dwDataSize ;

	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		//AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}