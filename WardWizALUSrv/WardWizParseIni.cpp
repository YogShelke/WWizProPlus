#include "stdafx.h"
#include "WardWizParseIni.h"

typedef DWORD (*GETFILEHASH)	(TCHAR *pFilePath, TCHAR *pFileHash);
extern GETFILEHASH		GetFileHash ;


extern BOOL	g_bIsWow64;
extern bool	g_bVistaOnward;
extern TCHAR	g_szAVPath[512];
extern HMODULE	g_hHashDLL;


/***************************************************************************************************                    
*  Function Name  : WardWizParseIniFile()
*  Description    : Constructor
*  Author Name    : Vilas,  
*  SR_NO		  : WRDWIZALUSRV_0001
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/

WardWizParseIniFile::WardWizParseIniFile()
{
	m_dwLastError = 0x00;
	m_dwProductID = 0x00;
	m_eScanLevel = CLAMSCANNER;
}


/***************************************************************************************************                    
*  Function Name  : ~WardWizParseIniFile()
*  Description    : Destructor
*  Author Name    : Vilas,   
*  SR_NO		  : WRDWIZALUSRV_0002
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
WardWizParseIniFile::~WardWizParseIniFile()
{
}

/***************************************************************************************************                    
*  Function Name  : ParseDownloadedPatch_Ini()
*  Description    : Parse downloaded ini
*  Author Name    : Vilas,
*  SR_NO		  : WRDWIZALUSRV_0003
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool WardWizParseIniFile::ParseDownloadedPatch_Ini(LPTSTR lpszIniPath, DWORD dwProductID, bool bProductUpdate, bool &bRetIsAnyProductChanges)
{
	bool	bRet = true;

	TCHAR	szCount[256] = {0};
	TCHAR	szSecName[32] = {0};
	TCHAR	szServerLoc[32] = {0};

	try
	{
		vALUFileInfo.clear();
		if(!GetScanLevelRegistry())
		{
			AddLogEntry(L"### Scan Level entry failed.", lpszIniPath, 0, true, 0x00);
		}

		GetProductID(m_dwProductID);

		if( !PathFileExists(lpszIniPath) )
		{
			m_dwLastError = 0x01;
			AddLogEntry(L"### ParseDownloadedPatch_Ini::File not found(%s)", lpszIniPath, 0, true, 0x00);
			goto Cleanup;
		}

		if( g_bIsWow64 )
		{
			GetPrivateProfileString(L"Count_64", L"Count", L"", szCount, 255, lpszIniPath);
			wcscpy(szSecName, L"Files_64");
			wcscpy(szServerLoc, L"64");
		}
		else
		{
			GetPrivateProfileString(L"Count_32", L"Count", L"", szCount, 255, lpszIniPath);
			wcscpy(szSecName, L"Files_32");
			wcscpy(szServerLoc, L"32");
		}

		DWORD	dwCount = 0x00, i=0x00;
		if (_tcslen(szCount) != 0x00 && bProductUpdate)
		{
			swscanf(szCount, L"%lu", &dwCount);
			if (!dwCount)
			{
				m_dwLastError = 0x03;
				AddLogEntry(L"### ParseDownloadedPatch_Ini::Invalid file entry count(%s)", lpszIniPath, 0, true, 0x00);
				goto Cleanup;
			}

			if (ParseSectionAndCheckLatestFiles(dwCount, szSecName, lpszIniPath, szServerLoc))
			{
				m_dwLastError = 0x04;
				goto Cleanup;
			}
		}

		//Common folder
		dwCount = 0x00;
		ZeroMemory(szSecName, sizeof(szSecName) );
		ZeroMemory(szCount, sizeof(szCount) );

		GetPrivateProfileString(L"Common", L"Count", L"", szCount, 255, lpszIniPath);
		wcscpy(szSecName, L"Common");
	
		if (_tcslen(szCount) != 0x00 && bProductUpdate)
		{
			swscanf(szCount, L"%lu", &dwCount);
			if (dwCount > 0)
			{
				if (ParseSectionAndCheckLatestFiles(dwCount, szSecName, lpszIniPath, szSecName))
				{
					m_dwLastError = 0x07;
					goto Cleanup;
				}
			}
		}

		//Common DB 
		dwCount = 0x00;
		ZeroMemory(szSecName, sizeof(szSecName));
		ZeroMemory(szCount, sizeof(szCount));

		GetPrivateProfileString(L"CommonDB", L"Count", L"", szCount, 255, lpszIniPath);
		wcscpy(szSecName, L"CommonDB");

		if (_tcslen(szCount) != 0x00)
		{
			swscanf(szCount, L"%lu", &dwCount);
			if (dwCount > 0)
			{
				if (ParseSectionAndCheckLatestFiles(dwCount, szSecName, lpszIniPath, szSecName))
				{
					m_dwLastError = 0x07;
					goto Cleanup;
				}
			}
		}

		bRet = false;
		m_dwLastError = 0x00;
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in ParseDownloadedPatch_Ini::(%s)", lpszIniPath, 0, true, 0x00);
	}


Cleanup:

	if( bRet )
		vALUFileInfo.clear();

	return bRet ;
}

/***************************************************************************************************
*  Function Name  : ParseDownloadedPatch_Ini4UpdtMgr()
*  Description    : Parse downloaded ini
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date			  : 11/April/2018
****************************************************************************************************/
bool WardWizParseIniFile::ParseDownloadedPatch_Ini4UpdtMgr(LPTSTR lpszIniPath, DWORD dwProductID, bool bProductUpdate, bool &bRetIsAnyProductChanges)
{
	bool	bRet = true;

	TCHAR	szCount[256] = { 0 };
	TCHAR	szSecName[32] = { 0 };
	TCHAR	szServerLoc[32] = { 0 };

	try
	{
		vALUZipFileInfo.clear();

		if (!GetScanLevelRegistry())
		{
			AddLogEntry(L"### Scan Level entry failed.", lpszIniPath, 0, true, 0x00);
		}

		GetProductID(m_dwProductID);

		if (!PathFileExists(lpszIniPath))
		{
			m_dwLastError = 0x01;
			AddLogEntry(L"### ParseDownloadedPatch_Ini4UpdtMgr::File not found(%s)", lpszIniPath, 0, true, 0x00);
			goto Cleanup;
		}

		//32 folder
		GetPrivateProfileString(L"Count_32", L"Count", L"", szCount, 255, lpszIniPath);
		wcscpy(szSecName, L"Files_32");
		wcscpy(szServerLoc, L"32");

		DWORD	dwCount = 0x00, i = 0x00;
		if (_tcslen(szCount) != 0x00 && bProductUpdate)
		{
			swscanf(szCount, L"%lu", &dwCount);
			if (!dwCount)
			{
				m_dwLastError = 0x03;
				AddLogEntry(L"### ParseDownloadedPatch_Ini4UpdtMgr::Invalid file entry count(%s)", lpszIniPath, 0, true, 0x00);
				goto Cleanup;
			}

			if (ParseSectionAndCheckLatestFiles4UpdtMgr(dwCount, szSecName, lpszIniPath, szServerLoc))
			{
				m_dwLastError = 0x04;
				goto Cleanup;
			}
		}

		//64 folder
		dwCount = 0x00;
		ZeroMemory(szSecName, sizeof(szSecName));
		ZeroMemory(szCount, sizeof(szCount));

		GetPrivateProfileString(L"Count_64", L"Count", L"", szCount, 255, lpszIniPath);
		wcscpy(szSecName, L"Files_64");
		wcscpy(szServerLoc, L"64");

		if (_tcslen(szCount) != 0x00 && bProductUpdate)
		{
			swscanf(szCount, L"%lu", &dwCount);
			if (!dwCount)
			{
				m_dwLastError = 0x03;
				AddLogEntry(L"### ParseDownloadedPatch_Ini4UpdtMgr::Invalid file entry count(%s)", lpszIniPath, 0, true, 0x00);
				goto Cleanup;
			}

			if (ParseSectionAndCheckLatestFiles4UpdtMgr(dwCount, szSecName, lpszIniPath, szServerLoc))
			{
				m_dwLastError = 0x04;
				goto Cleanup;
			}
		}

		//Common folder
		dwCount = 0x00;
		ZeroMemory(szSecName, sizeof(szSecName));
		ZeroMemory(szCount, sizeof(szCount));

		GetPrivateProfileString(L"Common", L"Count", L"", szCount, 255, lpszIniPath);
		wcscpy(szSecName, L"Common");

		if (_tcslen(szCount) != 0x00 && bProductUpdate)
		{
			swscanf(szCount, L"%lu", &dwCount);
			if (dwCount > 0)
			{
				if (ParseSectionAndCheckLatestFiles4UpdtMgr(dwCount, szSecName, lpszIniPath, szSecName))
				{
					m_dwLastError = 0x07;
					goto Cleanup;
				}
			}
		}

		//Common DB 
		dwCount = 0x00;
		ZeroMemory(szSecName, sizeof(szSecName));
		ZeroMemory(szCount, sizeof(szCount));

		GetPrivateProfileString(L"CommonDB", L"Count", L"", szCount, 255, lpszIniPath);
		wcscpy(szSecName, L"CommonDB");

		if (_tcslen(szCount) != 0x00)
		{
			swscanf(szCount, L"%lu", &dwCount);
			if (dwCount > 0)
			{
				if (ParseSectionAndCheckLatestFiles4UpdtMgr(dwCount, szSecName, lpszIniPath, szSecName))
				{
					m_dwLastError = 0x07;
					goto Cleanup;
				}
			}
		}

		bRet = false;
		m_dwLastError = 0x00;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ParseDownloadedPatch_Ini4UpdtMgr::(%s)", lpszIniPath, 0, true, 0x00);
	}


Cleanup:

	if (bRet)
		vALUZipFileInfo.clear();

	return bRet;
}

/***************************************************************************************************                    
*  Function Name  : ParseSectionAndCheckLatestFiles()
*  Description    : Parse Section and check latest files ot not
*  Author Name    : Vilas,      
*  SR_NO		  : WRDWIZALUSRV_0004
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool WardWizParseIniFile::ParseSectionAndCheckLatestFiles( DWORD dwCount, LPTSTR lpszSecName, LPTSTR lpszIniPath, LPTSTR lpszServerLocation )
{
	bool	bInvalidIni = false ;
	DWORD	i = 0x01;

	TCHAR	szValueName[256] = {0};
	TCHAR	szValueData[512] = {0};


	for(; i<=dwCount; i++)
	{
		swprintf_s(szValueName, _countof(szValueName), L"%lu", i );
		GetPrivateProfileString(lpszSecName, szValueName, L"", szValueData, 511, lpszIniPath);
		if( !szValueData[0] )
		{
			AddLogEntry(L"### VibraniumParseIniFile::Invalid Entries for(%s) in (%s)", lpszSecName, lpszIniPath, true, 0x00);
			bInvalidIni = true ;
			break;
		}

		if( ParseIniLine(szValueData, lpszServerLocation) )
		{
			AddLogEntry(L"### VibraniumParseIniFile::Parsing failed for(%s) in (%s)", szValueData, lpszIniPath, true, 0x00);
			bInvalidIni = true ;
			break;
		}

	}

	return bInvalidIni;
}

/***************************************************************************************************
*  Function Name  : ParseIniLine()
*  Description    : Parse INi Line
*  Author Name    : Amol Jaware
*  SR_NO		  : 
*  Date			  :	11/April/2018
****************************************************************************************************/
bool WardWizParseIniFile::ParseIniLine4UpdtMgr(LPTSTR lpszIniLine, LPTSTR lpszServerLocation)
{

	TCHAR	szToken[] = L",";
	TCHAR	*pToken = NULL;

	pToken = wcstok(lpszIniLine, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return true;
	}

	TCHAR	szExeName[256] = { 0 };

	if (pToken)
		wcscpy(szExeName, pToken);

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return true;
	}

	TCHAR	szZipSize[256] = { 0 };
	DWORD	dwZipSize = 0x00;

	if (pToken)
	{
		wcscpy(szZipSize, pToken);
		swscanf(szZipSize, L"%lu", &dwZipSize);
	}

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return true;
	}

	TCHAR	szFullSize[256] = { 0 };
	DWORD	dwFullSize = 0x00;

	if (pToken)
	{
		wcscpy(szFullSize, pToken);
		swscanf(szFullSize, L"%lu", &dwFullSize);
	}

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return true;
	}

	TCHAR	szFileHash[256] = { 0 };

	if (pToken)
		wcscpy(szFileHash, pToken);

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return true;
	}

	TCHAR	szZipFileHash[256] = { 0 };

	if (pToken)
		wcscpy(szZipFileHash, pToken);

	switch (m_dwProductID)
	{
	case 1:
	case 4:if (_wcsicmp(szExeName, L"VBMALICIOUSSITES.DB") == 0)
		return false;
		   else if (_wcsicmp(szExeName, L"VBSIGNATUREEMAIL.DB") == 0)
			   return false;
		break;
	case 2:
		break;
	case 3:
		break;
	}

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return true;
	}

	TCHAR	szFileShortPath[256] = { 0 };

	wcscpy(szFileShortPath, pToken);

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
	}

	TCHAR	szFileDwnldDecisionString[32] = { 0 };

	if (pToken)
		wcscpy(szFileDwnldDecisionString, pToken);

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
	}

	TCHAR	szFileDwnldDoubleCheckString[16] = { 0 };

	if (pToken)
		wcscpy(szFileDwnldDoubleCheckString, pToken);

	if ((_tcscmp(szFileDwnldDecisionString, L"NC") == 0) && (m_eScanLevel == WARDWIZSCANNER))
	{
		AddLogEntry(L">>> ParseEntry is not for setup of scan level first", 0, 0, true, FIRSTLEVEL);
		return false;
	}

	if ((_tcscmp(szFileDwnldDoubleCheckString, L"Y") == 0) && (m_eScanLevel == WARDWIZSCANNER))
	{
		AddLogEntry(L">>> Skip the file forcefully from getting downloaded.", 0, 0, true, FIRSTLEVEL);
		return false;
	}


	ALUZIPFILEINFO	aluZipInfo = { 0 };

	aluZipInfo.dwZipSize = dwZipSize;
	aluZipInfo.dwFullSize = dwFullSize;
	wcscpy(aluZipInfo.szFileName, szExeName);
	wcscpy(aluZipInfo.szFileHash, szFileHash);
	wcscpy(aluZipInfo.szZipFileHash, szZipFileHash);
	wcscpy(aluZipInfo.szFileShortPath, szFileShortPath);
	wcscpy(aluZipInfo.szServerLoc, lpszServerLocation);

	if (!IsCheckFileIsPresentLatestZipFile(&aluZipInfo))
		vALUZipFileInfo.push_back(aluZipInfo);

	return false;
}

/***************************************************************************************************                    
*  Function Name  : ParseIniLine()
*  Description    : Parse INi Line
*  Author Name    : Vilas,  
*  SR_NO		  : WRDWIZALUSRV_0005
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
*  Modified Date  : Neha Gharge 2/2/2015 (Tokenize for NC,YC)
****************************************************************************************************/
bool WardWizParseIniFile::ParseIniLine(LPTSTR lpszIniLine, LPTSTR lpszServerLocation )
{

	TCHAR	szToken[] = L",";
	TCHAR	*pToken = NULL;

	pToken = wcstok(lpszIniLine, szToken);
	if( !pToken )
	{
		AddLogEntry(L"### No string to tokenize from ini",0,0,true,FIRSTLEVEL);
		return true;
	}

	TCHAR	szExeName[256] = {0};
	
	if( pToken )
		wcscpy(szExeName, pToken );

	pToken = wcstok(NULL, szToken);
	if( !pToken )
	{
		AddLogEntry(L"### No string to tokenize from ini",0,0,true,FIRSTLEVEL);
		return true;
	}

	TCHAR	szZipSize[256] = {0};
	DWORD	dwZipSize = 0x00;

	if( pToken )
	{
		wcscpy(szZipSize, pToken );
		swscanf(szZipSize, L"%lu", &dwZipSize );
	}

	pToken = wcstok(NULL, szToken);
	if( !pToken )
	{
		AddLogEntry(L"### No string to tokenize from ini",0,0,true,FIRSTLEVEL);
		return true;
	}

	TCHAR	szFullSize[256] = {0};
	DWORD	dwFullSize = 0x00;

	if( pToken )
	{
		wcscpy(szFullSize, pToken );
		swscanf(szFullSize, L"%lu", &dwFullSize );
	}

	pToken = wcstok(NULL, szToken);
	if( !pToken )
	{
		AddLogEntry(L"### No string to tokenize from ini",0,0,true,FIRSTLEVEL);
		return true;
	}

	TCHAR	szFileHash[256] = {0};

	if( pToken )
		wcscpy(szFileHash, pToken );

			pToken = wcstok(NULL, szToken);
	if( !pToken )
	{
		AddLogEntry(L"### No string to tokenize from ini",0,0,true,FIRSTLEVEL);
		return true;
	}

	switch(m_dwProductID)
	{
		case 1	:
		case 4	:if(_wcsicmp(szExeName,L"VBMALICIOUSSITES.DB")==0)
				   return false;
			   else if(_wcsicmp(szExeName,L"VBSIGNATUREEMAIL.DB")==0)
				   return false;
			   break;
		case 2: 
				break;
		case 3:
				break;
	}
	
	//Issue : CHM files no needs to be checked here
	//Resolved by Nitin K. Date: 11th Jan 2016
	/*if( wcsstr(szExeName, L".CHM" )|| wcsstr(szExeName, L".chm" ))
	{
		switch(m_dwProductID)
		{
			case 1: if(_wcsicmp(szExeName,L"WRDWIZAVESSENTIAL.CHM")!= 0)
							return false;
					break;
			case 2: if(_wcsicmp(szExeName,L"WRDWIZAVPRO.CHM")!=0)
						  return false;
					break;
			case 3: if(_wcsicmp(szExeName,L"WRDWIZAVELITE.CHM")!=0)
							return false;
					break;
			case 4: if(_wcsicmp(szExeName,L"WRDWIZAVBASIC.CHM")!=0)
						return false;
					break;
		}
	}*/

	TCHAR	szFileShortPath[256] = {0};

	wcscpy(szFileShortPath, pToken );

	pToken = wcstok(NULL, szToken);
	if( !pToken )
	{
		AddLogEntry(L"### No string to tokenize from ini",0,0,true,FIRSTLEVEL);
	}

	TCHAR	szFileDwnldDecisionString[32] = {0};

	if( pToken )
		wcscpy(szFileDwnldDecisionString, pToken );

	pToken = wcstok(NULL, szToken);
	if( !pToken )
	{
		AddLogEntry(L"### No string to tokenize from ini",0,0,true,FIRSTLEVEL);
	}

	TCHAR	szFileDwnldDoubleCheckString[16] = {0};

	if( pToken )
		wcscpy(szFileDwnldDoubleCheckString, pToken );

	if((_tcscmp(szFileDwnldDecisionString,L"NC") == 0) && (m_eScanLevel == WARDWIZSCANNER))
	{
		AddLogEntry(L">>> ParseEntry is not for setup of scan level first",0,0,true,FIRSTLEVEL);
		return false;
	}

	if((_tcscmp(szFileDwnldDoubleCheckString,L"Y") == 0) && (m_eScanLevel == WARDWIZSCANNER))
	{
		AddLogEntry(L">>> Skip the file forcefully from getting downloaded.",0,0,true,FIRSTLEVEL);
		return false;
	}

	ALUFILEINFO	aluInfo = {0} ;

	aluInfo.dwZipSize = dwZipSize;
	aluInfo.dwFullSize = dwFullSize;
	wcscpy(aluInfo.szFileName, szExeName );
	wcscpy(aluInfo.szFileHash, szFileHash );
	wcscpy(aluInfo.szFileShortPath, szFileShortPath );
	wcscpy(aluInfo.szServerLoc, lpszServerLocation );

	if( !CheckFilePresentIsLatest(&aluInfo) )
		vALUFileInfo.push_back( aluInfo );

	return false;
}

/***************************************************************************************************                    
*  Function Name  : CheckFilePresentIsLatest()
*  Description    : Check file present is latest or not
*  Author Name    : Vilas,   
*  SR_NO		  : WRDWIZALUSRV_0006
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool WardWizParseIniFile::CheckFilePresentIsLatest( PALUFILEINFO pALUFileInfo)
{

	TCHAR	szFilePath[512] = {0};

	GetFilePath(szFilePath, pALUFileInfo->szFileShortPath, pALUFileInfo->szFileName) ;
	
	if( !PathFileExists( szFilePath ) )
		return false;

	DWORD	dwFileSize = 0x00;
	TCHAR	szFileHash[128] = {0};

	GetFileSizeAndHash(szFilePath, dwFileSize, szFileHash );

	if( pALUFileInfo->dwFullSize != dwFileSize )
		return false;

	if( wcscmp(pALUFileInfo->szFileHash, szFileHash) != 0x00 )
		return false;

	return true;
}
/***************************************************************************************************
*  Function Name  : ParseSectionAndCheckLatestFiles4UpdtMgr()
*  Description    : Parse Section and check latest files ot not
*  Author Name    : Amol Jaware.
*  SR_NO		  : 
*  Date			  :	11-April-2018
****************************************************************************************************/
bool WardWizParseIniFile::ParseSectionAndCheckLatestFiles4UpdtMgr(DWORD dwCount, LPTSTR lpszSecName, LPTSTR lpszIniPath, LPTSTR lpszServerLocation)
{
	bool	bInvalidIni = false;
	DWORD	i = 0x01;

	TCHAR	szValueName[256] = { 0 };
	TCHAR	szValueData[512] = { 0 };

	for (; i <= dwCount; i++)
	{
		swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
		GetPrivateProfileString(lpszSecName, szValueName, L"", szValueData, 511, lpszIniPath);
		if (!szValueData[0])
		{
			AddLogEntry(L"### ParseSectionAndCheckLatestFiles4UpdtMgr::Invalid Entries for(%s) in (%s)", lpszSecName, lpszIniPath, true, 0x00);
			bInvalidIni = true;
			break;
		}

		if (ParseIniLine4UpdtMgr(szValueData, lpszServerLocation))
		{
			AddLogEntry(L"### ParseSectionAndCheckLatestFiles4UpdtMgr::Parsing failed for(%s) in (%s)", szValueData, lpszIniPath, true, 0x00);
			bInvalidIni = true;
			break;
		}

	}

	return bInvalidIni;
}

/***************************************************************************************************
*  Function Name  : IsCheckFileIsPresentLatestZipFile()
*  Description    : Check Zip file present is latest or not
*  Author Name    : Amol Jaware.
*  SR_NO		  : 
*  Date			  :	05 April 2018
****************************************************************************************************/
bool WardWizParseIniFile::IsCheckFileIsPresentLatestZipFile(PALUZIPFILEINFO pALUZipFileInfo)
{

	TCHAR	szFilePath[512] = { 0 };

	GetFilePath(szFilePath, pALUZipFileInfo->szFileShortPath, pALUZipFileInfo->szFileName);

	if (!PathFileExists(szFilePath))
		return false;

	DWORD	dwFileSize = 0x00;
	TCHAR	szZipFileHash[128] = { 0 };

	GetFileSizeAndHash(szFilePath, dwFileSize, szZipFileHash);

	if (pALUZipFileInfo->dwFullSize != dwFileSize)
		return false;

	if (wcscmp(pALUZipFileInfo->szZipFileHash, szZipFileHash) != 0x00)
		return false;

	return true;
}

/***************************************************************************************************                    
*  Function Name  : GetFilePath()
*  Description    : Get File Path
*  Author Name    : Vilas,   
*  SR_NO		  : WRDWIZALUSRV_0007
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool WardWizParseIniFile::GetFilePath(LPTSTR lpszFilePath, LPTSTR lpszShortPath, LPTSTR lpszFileName )
{

	TCHAR	*pTemp = wcschr(lpszShortPath, '\\');

	if( pTemp )
	{
		pTemp++;
		swprintf_s(lpszFilePath, 511, L"%s%s\\%s", g_szAVPath, pTemp, lpszFileName );
	}
	else
		swprintf_s(lpszFilePath, 511, L"%s%s", g_szAVPath, lpszFileName );

	return false;
}

/***************************************************************************************************                    
*  Function Name  : GetFileSizeAndHash()
*  Description    : Get File size and hash
*  Author Name    : Vilas,   
*  SR_NO		  : WRDWIZALUSRV_0008
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool WardWizParseIniFile::GetFileSizeAndHash(TCHAR *pFilePath, DWORD &dwFileSize, TCHAR *pFileHash)
{
	try
	{
		HANDLE hFile = CreateFile( pFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### GetFileSizeAndHash : Error in opening file %s",pFilePath,0,true,SECONDLEVEL);
			return true;
		}

		dwFileSize = GetFileSize(hFile, NULL );
		CloseHandle( hFile );
		hFile = INVALID_HANDLE_VALUE;

		if( pFileHash )
		{
				if( !GetFileHash(pFilePath, pFileHash) )
				return false;
			else
			{
				AddLogEntry(L"### Exception in CVibraniumParseIni::IsWow64",0,0,true,SECONDLEVEL);
				return true;
			}
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumParseIni::IsWow64",0,0,true,SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************                    
*  Function Name  : GetLastErrorForParseDownloadedPatch_Ini()
*  Description    : Get last error
*  Author Name    : Vilas,        
*  SR_NO		  : WRDWIZALUSRV_0009
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD WardWizParseIniFile::GetLastErrorForParseDownloadedPatch_Ini()
{
	return m_dwLastError;
}

/***************************************************************************************************                    
*  Function Name    : GetProductID
*  Output parameter : dwProductID = return Client product ID
*  Description      : This treat will show the status and percentage from ALU service to UI
*  Author Name      : Vilas  
*  SR_NO		    : WRDWIZALUSRV_0010
*  Date             : 4- Jul -2014 (Auto Live Update)
****************************************************************************************************/

bool WardWizParseIniFile::GetProductID( DWORD &dwProductID)
{
	CString csIniFilePath = GetAVPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";

	if( !PathFileExists(csIniFilePath) )
		return GetDWORDValueFromRegistry(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwProductID", dwProductID);

	dwProductID = GetPrivateProfileInt(L"VBSETTINGS", L"ProductID", 0, csIniFilePath);

	if( !dwProductID)
		return true;

	return false;
}


/***************************************************************************************************                    
*  Function Name  : GetDWORDValueFromRegistry
*  Description    : It will give the dword value from registry.
*  Author Name    : Vilas   
*  SR_NO		  : WRDWIZALUSRV_0011
*  Date			  :	4- Jul -2014 (Auto Live Update)
****************************************************************************************************/
bool WardWizParseIniFile::GetDWORDValueFromRegistry(HKEY hMain, LPTSTR lpszSubKey, LPTSTR lpszValuneName, DWORD &dwProductID)
{
	HKEY	hSubKey = NULL ;

	if (RegOpenKeyEx(hMain, lpszSubKey, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
		return true;

	DWORD	dwSize = sizeof(DWORD) ;
	DWORD	dwType = 0x00 ;

	RegQueryValueEx(hSubKey, lpszValuneName, 0, &dwType, (LPBYTE)&dwProductID, &dwSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if( !dwProductID )
		return true;

	return false;
}

/***************************************************************************************************                    
*  Function Name  : GetScanLevelRegistry
*  Description    : Get Value of Scan Level value from registry.
*  Author Name    : Neha Gharge  
*  SR_NO		  : 
*  Date			  :	22- Jan -2015 (Auto Live Update)
****************************************************************************************************/
bool WardWizParseIniFile::GetScanLevelRegistry()
{
	DWORD dwScanLevel = 0x00;
	bool bReturn = false;
	if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwScanLevel", dwScanLevel) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel", 0, 0, true, SECONDLEVEL);;
		return false;
	}
	switch(dwScanLevel)
	{
	case 0x01:
		bReturn = true;
		m_eScanLevel = WARDWIZSCANNER;
		break;
	case 0x02:
		bReturn = true;
		m_eScanLevel = CLAMSCANNER;
		break;
	default: 
		bReturn = false;
		AddLogEntry(L" ### Scan Level option is wrong. Please reinstall setup of GenX",0,0,true,SECONDLEVEL);
		break;
	}
	return bReturn;
}
