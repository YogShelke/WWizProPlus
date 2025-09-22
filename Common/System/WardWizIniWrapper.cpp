/**********************************************************************************************************	
	Program Name			: WardWizIniWrapper.cpp
	Description				: Wrapper class to do ini file Read/Write operations
	Author Name				: Ramkrushna Shelke 
	Date Of Creation		: 06 May 2016
	Version No				: 1.14.0.30
	Special Logic Used		: 
	Modification Log		:
		Ram					: Created Wrapper class
***********************************************************************************************************/
#include "StdAfx.h"
#include "WardWizIniWrapper.h"

/***************************************************************************
Function Name  : CWardWizIniWrapper
Description    : C'tor
Author Name    : Ram Shelke
Date           : 06 May 2016
****************************************************************************/
CWardWizIniWrapper::CWardWizIniWrapper(LPTSTR lpszFilePath)
{
	_tcscpy_s(m_szFilePath, lpszFilePath);
}

/***************************************************************************
Function Name  : CWardWizIniWrapper
Description    : Dest'r
Author Name    : Ram Shelke
Date           : 06 May 2016
****************************************************************************/
CWardWizIniWrapper::~CWardWizIniWrapper()
{
}

/***************************************************************************
Function Name  : GetStringValue
Description    : Function to get string value 
Author Name    : Ram Shelke
Date           : 06 May 2016
****************************************************************************/
bool CWardWizIniWrapper::GetStringValue(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, 
						__out_ecount_part_opt(nSize, return +1) LPWSTR lpReturnedString, DWORD nSize )
{
	__try
	{
		if (!lpAppName || !lpKeyName || !lpDefault || !lpReturnedString)
			return false;

		return GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, m_szFilePath) ? true : false;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizIniWrapper::GetStringValue, SetionName: %s, KeyName : %s", lpAppName, lpKeyName, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/***************************************************************************
Function Name  : GetIntegerValue
Description    : Function to get integer value
Author Name    : Ram Shelke
Date           : 06 May 2016
****************************************************************************/
int CWardWizIniWrapper::GetIntegerValue(LPCWSTR lpAppName, LPCWSTR lpKeyName, INT nDefault)
{
	__try
	{
		return static_cast<int>(GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, m_szFilePath));
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizIniWrapper::GetIntegerValue, SetionName: %s, KeyName : %s", lpAppName, lpKeyName, true, SECONDLEVEL);
	}
	return -1;
}
