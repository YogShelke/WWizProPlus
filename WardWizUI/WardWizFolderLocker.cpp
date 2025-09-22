// CWardWizFolderLocker.cpp : implementation file
/*********************************************************************
*  Program Name		: CWardWizFolderLocker.cpp
*  Description		: This class contains functionality for the locking 
					of files and folders with a password to restrict the
					accessibility, modification and deletion
*  Author Name		: Jeena Mariam Saji
*  Date Of Creation	: 15 March 2017
*  Version No       : 3.1.0.0
**********************************************************************/
#include "stdafx.h"
#include "WardWizFolderLocker.h"
#include "WardWizDatabaseInterface.h"

/***********************************************************************************************
Function Name  : CWardWizFolderLocker
Description    : Constructor
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 16 March 2017
***********************************************************************************************/
CWardWizFolderLocker::CWardWizFolderLocker() : 
behavior_factory("WardWizFolderLocker")
, m_objComService(SERVICE_SERVER, true, 3)
, m_hSQLiteDLL(NULL)
, m_bISDBLoaded(false)
{
	LoadRequiredLibrary();
}

/***********************************************************************************************
Function Name  : CWardWizFolderLocker
Description    : Destructor
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 16 March 2017
***********************************************************************************************/
CWardWizFolderLocker::~CWardWizFolderLocker()
{
	if (m_hSQLiteDLL != NULL)
	{
		FreeLibrary(m_hSQLiteDLL);
		m_hSQLiteDLL = NULL;
	}
}

/***********************************************************************************************
Function Name  : GetDBPath
Description    : Destructor
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 16 March 2017
***********************************************************************************************/
json::value CWardWizFolderLocker::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBFOLDERLOCKER.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFolderLocker::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : CheckForNetworkPath
Description    : Destructor
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 16 March 2017
***********************************************************************************************/
json::value CWardWizFolderLocker::CheckForNetworkPath(SCITER_VALUE svFilePath)
{
	try
	{
		const std::wstring  chFilePath = svFilePath.get(L"");
		if (PathIsNetworkPath((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFolderLocker::CheckForNetworkPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary
*  Description    : Function which load required DLL files
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
bool CWardWizFolderLocker::LoadRequiredLibrary()
{
	bool bReturn = false;

	try
	{
		DWORD	dwRet = 0x00;

		CString	csDLLPath = L"";
		csDLLPath.Format(L"%s\\SQLITE3.DLL", theApp.GetModuleFilePath());
		if (!PathFileExists(csDLLPath))
		{
			return false;
		}

		if (!m_hSQLiteDLL)
		{
			m_hSQLiteDLL = LoadLibrary(csDLLPath);
			if (!m_hSQLiteDLL)
			{
				AddLogEntry(L"### Failed to LoadLibrary [%s]", csDLLPath, 0, true, SECONDLEVEL);
				return false;
			}
		}

		m_pSQliteOpen = (SQLITE3_OPEN)GetProcAddress(m_hSQLiteDLL, "sqlite3_open");
		m_pSQLitePrepare = (SQLITE3_PREPARE)GetProcAddress(m_hSQLiteDLL, "sqlite3_prepare");
		m_pSQLiteColumnCount = (SQLITE3_COLUMN_COUNT)GetProcAddress(m_hSQLiteDLL, "sqlite3_column_count");
		m_pSQLiteStep = (SQLITE3_STEP)GetProcAddress(m_hSQLiteDLL, "sqlite3_step");
		m_pSQLiteColumnText = (SQLITE3_COLUMN_TEXT)GetProcAddress(m_hSQLiteDLL, "sqlite3_column_text");
		m_pSQLiteClose = (SQLITE3_CLOSE)GetProcAddress(m_hSQLiteDLL, "sqlite3_close");

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFolderLocker::LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileOrFolderPath
*  Description    :	To check weather path is filepath or folderpath.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 23 Sept. 2016
**********************************************************************************************************/
json::value CWardWizFolderLocker::CheckFileOrFolderPath(SCITER_VALUE svFileFolderPath)
{
	try
	{
		const std::wstring  chFilePath = svFileFolderPath.get(L"");
		if (PathIsDirectory((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::CheckFileOrFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	DecryptData
*  Description    :	To decrypt the password.
*  Author Name    : Amol Jaware
*  Date           : 4 April. 2017
**********************************************************************************************************/
/*DWORD CWardWizFolderLocker::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
{
	try
	{
		if (IsBadWritePtr(lpBuffer, dwSize))
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			if (lpBuffer[iIndex] != 0)
			{
				if ((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
				{
					lpBuffer[iIndex] = lpBuffer[iIndex];
				}
				else
				{
					lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
					jIndex++;
				}
				if (jIndex == WRDWIZ_KEYSIZE)
				{
					jIndex = 0x00;
				}
				if (iIndex >= dwSize)
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFolderLocker::DecryptData");
	}
	return 0;
}
json::value CWardWizFolderLocker :: OnClickfunEncryptPassword(SCITER_VALUE svEncrptPassword)
{
	try
	{
		sciter::string strPass= svEncrptPassword.to_string();
		
		char byPass[0x32] = { 0 };
		wcstombs(byPass, strPass.c_str(), sizeof(byPass) + 1);

		if (DecryptData((LPBYTE)&byPass, sizeof(byPass)) != 0x00)
		{
			AddLogEntry(L"### DecryptData Failed in CVibraniumFolderLocker::OnClickfunEncryptPassword");
			return 0;
		}

		//TCHAR szDecryptPass[0x64] = { 0 };
		//MultiByteToWideChar(CP_ACP, 0, byPass, -1, szDecryptPass, sizeof(szDecryptPass));
		return SCITER_VALUE(byPass);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFolderLocker::OnClickfunEncryptPassword");
	}
	return 0;
}
*/