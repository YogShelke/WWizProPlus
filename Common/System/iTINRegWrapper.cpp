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

	__try
	{
		BOOL bWow64Process;
		DWORD dwSamDesired = KEY_ALL_ACCESS;
		if (IsWow64Process(GetCurrentProcess(), &bWow64Process))
		{
			dwSamDesired |= KEY_WOW64_64KEY;
		}

		RegCreateKeyEx(hRootKey, pKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, dwSamDesired, NULL, &hSubKey, NULL);
		if( !hSubKey )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		dwRet = RegSetValueEx(hSubKey, pValueName, 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD) ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

DWORD CITinRegWrapper::SetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, TCHAR *pszData)
{
	DWORD	dwRet = 0x00, dwSize = 0x00 ;
	HKEY	hSubKey = NULL ;

	__try
	{
		BOOL bWow64Process;
		DWORD dwSamDesired = KEY_ALL_ACCESS;
		if (IsWow64Process(GetCurrentProcess(), &bWow64Process))
		{
			dwSamDesired |= KEY_WOW64_64KEY;
		}

		RegCreateKeyEx(hRootKey, pKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, dwSamDesired, NULL, &hSubKey, NULL);
		if( !hSubKey )
		{
			RegOpenKeyEx(hRootKey, pKeyName, 0, dwSamDesired, &hSubKey);
			if (!hSubKey)
			{
				dwRet = 0x01;
				goto Cleanup;
			}
		}

		//Ram: Removed compiler warnings
		dwSize = static_cast<DWORD>(wcslen(pszData)*sizeof(TCHAR));
		dwRet = RegSetValueEx(hSubKey, pValueName, 0, REG_SZ, (LPBYTE)pszData, dwSize ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

DWORD CITinRegWrapper::SetRegistryValueData( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, LPBYTE pbData, DWORD dwSize )
{
	DWORD	dwRet = 0x00 ;
	HKEY	hSubKey = NULL ;

	__try
	{
		BOOL bWow64Process;
		DWORD dwSamDesired = KEY_ALL_ACCESS;
		if (IsWow64Process(GetCurrentProcess(), &bWow64Process))
		{
			dwSamDesired |= KEY_WOW64_64KEY;
		}

		RegCreateKeyEx(hRootKey, pKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, dwSamDesired, NULL, &hSubKey, NULL);
		if( !hSubKey )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		dwRet = RegSetValueEx(hSubKey, pValueName, 0, REG_BINARY, (LPBYTE)pbData, dwSize ) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
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

	//Ram: Commented as because, if default value is passed as 0x01, here it was getting reset to 0x00.
	//dwValue = 0x00;

	BOOL bWow64Process;
	DWORD dwSamDesired = KEY_QUERY_VALUE;
	if (IsWow64Process(GetCurrentProcess(), &bWow64Process))
	{
		dwSamDesired |= KEY_WOW64_64KEY;
	}

	if (RegOpenKeyEx(hRootKey, pKeyName, 0, dwSamDesired, &hKey) != ERROR_SUCCESS)
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

	__try
	{

		if( IsBadWritePtr(pszData, dwSize) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		BOOL bWow64Process;
		DWORD dwSamDesired = KEY_QUERY_VALUE;
		if (IsWow64Process(GetCurrentProcess(), &bWow64Process))
		{
			dwSamDesired |= KEY_WOW64_64KEY; //KEY_WOW64_64KEY KEY_WOW64_32KEY
		}

		RegOpenKeyEx(hRootKey, pKeyName, 0, dwSamDesired, &hSubKey);
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
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

DWORD CITinRegWrapper::GetRegistryValueData32( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, PVOID pszData, DWORD &dwSize )
{
	DWORD	dwRet = 0x00, dwRegType = 0x00, dwDataSize = 0x00 ;
	HKEY	hSubKey = NULL ;

	__try
	{

		if( IsBadWritePtr(pszData, dwSize) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		BOOL bWow64Process;
		DWORD dwSamDesired = KEY_ALL_ACCESS;
		if (IsWow64Process(GetCurrentProcess(), &bWow64Process))
		{
			dwSamDesired |= KEY_WOW64_64KEY;
		}

		RegOpenKeyEx(hRootKey, pKeyName, 0, dwSamDesired, &hSubKey);
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
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizRegWrapper::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

/***********************************************************************************************
  Function Name  : DelRegistryValueName
  Description    : Deletes Registry value name.
  Author Name    : Vilas Suvarnakar
  SR.NO			 : 
  Date           : 17 Jan 2015
***********************************************************************************************/
DWORD CITinRegWrapper::DelRegistryValueName( HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName)
{
	DWORD	dwRet	= 0x00 ;
	HKEY	hSubKey = NULL ;

	__try
	{
		BOOL bWow64Process;
		DWORD dwSamDesired = KEY_SET_VALUE;
		if (IsWow64Process(GetCurrentProcess(), &bWow64Process))
		{
			dwSamDesired |= KEY_WOW64_64KEY;
		}

		RegOpenKeyEx(hRootKey, pKeyName, 0, dwSamDesired, &hSubKey);
		if( !hSubKey )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		dwRet = RegDeleteValueW( hSubKey, pValueName );
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizRegWrapper::DelRegistryValueName", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hSubKey )
		RegCloseKey( hSubKey );

	hSubKey = NULL;

	return dwRet ;
}

/**********************************************************************************************************
*  Function Name  :	RegDelnodeRecurse
*  Description    :
*  Author Name    : Vilas & Prajakta
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL CITinRegWrapper::RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
	LPTSTR		lpEnd;
	LONG		lResult;
	DWORD		dwSize;
	TCHAR		szName[MAX_PATH] = { 0 };
	HKEY		hKey = NULL;
	FILETIME	ftWrite;

	// First, see if we can delete the key without having
	// to recurse.
	__try
	{

		lResult = RegDeleteKey(hKeyRoot, lpSubKey);
		if (lResult == ERROR_SUCCESS)
			return TRUE;

		lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
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
		lpEnd = lpSubKey + lstrlen(lpSubKey);

		if (*(lpEnd - 1) != TEXT('\\'))
		{
			*lpEnd = TEXT('\\');
			lpEnd++;
			*lpEnd = TEXT('\0');
		}

		// Enumerate the keys

		dwSize = MAX_PATH;
		lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
			NULL, NULL, &ftWrite);
		if (lResult == ERROR_SUCCESS)
		{
			do
			{
				lstrcpy(lpEnd, szName);
				if (!RegDelnodeRecurse(hKeyRoot, lpSubKey))
				{
					break;
				}

				dwSize = MAX_PATH;

				lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
					NULL, NULL, &ftWrite);

			} while (lResult == ERROR_SUCCESS);
		}

		lpEnd--;
		*lpEnd = TEXT('\0');

		RegCloseKey(hKey);
		hKey = NULL;

		// Try again to delete the key.

		lResult = RegDeleteKey(hKeyRoot, lpSubKey);

		if (lResult == ERROR_SUCCESS)
			return TRUE;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::RegDelnodeRecurse", 0, 0, true, SECONDLEVEL);
	}

	return FALSE;
}