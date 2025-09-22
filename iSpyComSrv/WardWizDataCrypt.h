#pragma once
/**********************************************************************************************************
Program Name			: WardWizDataCrypt.cpp
Description				: Wrapper class to launch Crypt.exe
Author Name				: Nitin Kolapkar
Date Of Creation		: 5th Jun 2015
Version No				: 1.11.0.0
Special Logic Used		:
Modification Log		:
***********************************************************************************************************/

class CWardWizDataCrypt
{
public:
	CWardWizDataCrypt();
	~CWardWizDataCrypt();

	bool LaunchDataEncDecExe(TCHAR *pszFileName, TCHAR *pszPass, DWORD dwKeepOriginal, DWORD dwDataCryptOpr);
};

