/**********************************************************************************************************
	Program Name			: WardWizIniWrapper.h
	Description				: Wrapper class to do ini file Read/Write operations
	Author Name				: Ramkrushna Shelke
	Date Of Creation		: 06 May 2016
	Version No				: 1.14.0.30
	Special Logic Used		:
	Modification Log		:
	Ram					: Created Wrapper class
***********************************************************************************************************/
#pragma once
class CWardWizIniWrapper
{
public:
	CWardWizIniWrapper(LPTSTR lpszFilePath);
	virtual ~CWardWizIniWrapper();
	
	bool GetStringValue(
		__in_opt LPCWSTR lpAppName,
		__in_opt LPCWSTR lpKeyName,
		__in_opt LPCWSTR lpDefault,
		__out_ecount_part_opt(nSize, return +1) LPWSTR lpReturnedString,
		__in     DWORD nSize
		);

	int GetIntegerValue(
		__in     LPCWSTR lpAppName,
		__in     LPCWSTR lpKeyName,
		__in     INT nDefault
		);
public:
	TCHAR	m_szFilePath[MAX_PATH];
};

