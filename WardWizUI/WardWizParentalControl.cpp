// CWardWizParentalControl.cpp : implementation file
/*********************************************************************
/****************************************************
*  Program Name: CWardWizParentalControl.h
*  Author Name: Jeena Mariam Saji
*  Date Of Creation: 30th April 2018
**********************************************************************/
#include "stdafx.h"
#include "WardWizParentalControl.h"
#include "ISpyCommunicator.h"
#include "iTinRegWrapper.h"

#ifndef UNICODE
#define UNICODE
#endif

#pragma comment(lib, "netapi32.lib")
#define INFO_BUFFER_SIZE 32767

#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <lm.h>

/***********************************************************************************************
Function Name  : CWardWizParentalControl
Description    : Constructor
Author Name    : Jeena Mariam Saji
Date           : 30th April 2018
***********************************************************************************************/
CWardWizParentalControl::CWardWizParentalControl() : behavior_factory("WardWizParControl")
, m_objComService(SERVICE_SERVER, true, 3)
, m_hSQLiteDLL(NULL)
, m_bISDBLoaded(false)
{
	LoadRequiredLibrary();
}

/***********************************************************************************************
Function Name  : CWardWizParentalControl
Description    : Destructor
Author Name    : Jeena Mariam Saji
Date           : 30th April 2018
***********************************************************************************************/
CWardWizParentalControl::~CWardWizParentalControl()
{
	if (m_hSQLiteDLL != NULL)
	{
		FreeLibrary(m_hSQLiteDLL);
		m_hSQLiteDLL = NULL;
	}
}

/***********************************************************************************************
Function Name  : SetRectTrackChangeList
Description    : SetRectTrackChangeList
Author Name    : Jeena Mariam Saji
Date           : 30th April 2018
***********************************************************************************************/
json::value CWardWizParentalControl::SetRectTrackChange()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		if (!SendData2Service(SHOW_PC_LOCK_WND, true))
		{
			AddLogEntry(L"### Failed to ChangeParCtrlSetting in PARCTRLCHANGE for SHOW_PC_LOCK_WND", 0, 0, true, SECONDLEVEL);
		}
		CallCheckRectTrackChange();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::SetRectTrackChangeList", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : SetRectTrackChangeList
Description    : SetRectTrackChangeList
Author Name    : Jeena Mariam Saji
Date           : 13th July 2018
***********************************************************************************************/
json::value CWardWizParentalControl::SetRectTrackChangeForINet()
{
	try
	{
		if (!SendData2Service(ON_INET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to send data in SetRectTrackChangeForINet for ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(ON_RESET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to send data in SetRectTrackChangeForINet for ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
		CallCheckRectTrackChangeINet();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::SetRectTrackChangeList", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : OnSetInternetRestriction
Description    : On Set Internet Restriction
Author Name    : Jeena Mariam Saji
Date           : 07th September 2018
***********************************************************************************************/
json::value CWardWizParentalControl::OnSetInternetRestriction()
{
	try
	{
		if (!SendData2Service(ON_INET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to send data in OnSetInternetRestriction for ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(ON_RESET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to send data in SetRectTrackChangeForINet for ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::OnSetInternetRestriction", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : SetRectTrackChangeList
Description    : SetRectTrackChangeList
Author Name    : Jeena Mariam Saji
Date           : 06th July 2018
***********************************************************************************************/
json::value CWardWizParentalControl::SetRectTrackChangeList()
{
	try
	{
		ChangeParCtrlSetting();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::SetRectTrackChangeList", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : SetRectTrackChangeListINet
Description    : Function to set rect tracker value in list view for internet
Author Name    : Kunal Waghmare
Date           : 13th July 2018
***********************************************************************************************/
json::value CWardWizParentalControl::SetRectTrackChangeListINet()
{
	try
	{
		ChangeParCtrlSetting();
		if (!SendData2Service(ON_RESET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to send data in SetRectTrackChangeForINet for ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::SetRectTrackChangeListINet", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : GetSetRectTracker
Description    : Function to set rect tracker value in list view
Author Name    : Jeena Mariam Saji
Date           : 27th June 2018
***********************************************************************************************/
json::value CWardWizParentalControl::GetSetRectTracker(SCITER_VALUE svCheckRectTrackChange, SCITER_VALUE svCheckRectTrackINetChange)
{
	try
	{
		m_svCheckRectTrackChange = svCheckRectTrackChange;
		m_svCheckRectTrackINetChange = svCheckRectTrackINetChange;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::GetSetRectTracker", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : CallCheckRectTrackChange
Description    : Function to set rect tracker value in list view
Author Name    : Jeena Mariam Saji
Date           : 27th June 2018
***********************************************************************************************/
void CWardWizParentalControl::CallCheckRectTrackChange()
{
	try
	{
		m_svCheckRectTrackChange.call();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::CallCheckRectTrackChange", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CallCheckRectTrackChangeINet
Description    : Function to set rect tracker value in list view
Author Name    : Jeena Mariam Saji
Date           : 13th July 2018
***********************************************************************************************/
void CWardWizParentalControl::CallCheckRectTrackChangeINet()
{
	try
	{
		m_svCheckRectTrackINetChange.call();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::CallCheckRectTrackChangeINet", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : GetDBPath
Description    : Get Database path
Author Name    : Jeena Mariam Saji
Date           : 21 June 2018
***********************************************************************************************/
json::value CWardWizParentalControl::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBPARCONTROL.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : GetDBPathBlockList
Description    : Get Database path for Blocked List
Author Name    : Jeena Mariam Saji
Date           : 25 July 2018
***********************************************************************************************/
json::value CWardWizParentalControl::GetDBPathBlockList()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VibraniumPARBLK.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::GetDBPathBlockList", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : GetDataBasePath
Description    : Load required libraries
Author Name    : Jeena Mariam Saji
Date           : 30th April 2018
***********************************************************************************************/
bool CWardWizParentalControl::LoadRequiredLibrary()
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
		AddLogEntry(L"### Exception in CVibraniumParentalControl::LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : GetWindowsUserList
Description    : This function is used to returning windows user list.
Author Name    : Amol Jaware
Date           : 07th May 2018
***********************************************************************************************/
json::value CWardWizParentalControl::GetWindowsUserList(SCITER_VALUE svGetUserList)
{
	LPUSER_INFO_1 pBuf = NULL;
	LPUSER_INFO_1 pTmpBuf;
	DWORD dwLevel = 1;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i;
	DWORD dwTotalCount = 0;
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = NULL;
	DWORD  bufCharCount = INFO_BUFFER_SIZE;

	// Call the NetUserEnum() function, specifying level 0;
	//   enumerate global user account types only.
	try
	{
		do // begin do
		{
			nStatus = NetUserEnum((LPCWSTR)pszServerName,
				dwLevel,
				FILTER_NORMAL_ACCOUNT, // global users
				(LPBYTE*)&pBuf,
				dwPrefMaxLen,
				&dwEntriesRead,
				&dwTotalEntries,
				&dwResumeHandle);
			// If the call succeeds,
			if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
			{
				if ((pTmpBuf = pBuf) != NULL)
				{
					// Loop through the entries.
					for (i = 0; (i < dwEntriesRead); i++)
					{
						assert(pTmpBuf != NULL);
						if (pTmpBuf == NULL)
						{
							fwprintf_s(stderr, L"An access violation has occurred\n");
							break;
						}
						
						if (!(pTmpBuf->usri1_flags & UF_ACCOUNTDISABLE))
						{
							svGetUserList.call(pTmpBuf->usri1_name);
						}
						
						pTmpBuf++;
						dwTotalCount++;
					}
				}
			}
			// Otherwise, print the system error.
			else
			{
				//fwprintf_s(stderr, L"A system error has occurred: %d\n", nStatus);
			}
			// Free the allocated buffer.
			if (pBuf != NULL)
			{
				NetApiBufferFree(pBuf);
				pBuf = NULL;
			}
		}
		// Continue to call NetUserEnum while there are more entries.
		while (nStatus == ERROR_MORE_DATA); // end do
		// Check again for allocated memory.
		if (pBuf != NULL)
		NetApiBufferFree(pBuf);
		// Print the final count of users enumerated.
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::GetWindowsUserList", 0, 0, true, SECONDLEVEL);
	}
	
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetSetRegistryVal
*  Description    : To get and set Registry value
*  Author Name    : Jeena Mariam Saji
*  Date           : 18 June 2018
****************************************************************************************************/
json::value CWardWizParentalControl::On_GetSetRegistryVal()
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		DWORD dwParentalControl = 0x00;
		CITinRegWrapper	 m_objReg;
		bool bFound = true;
		bool bFlagValue = true;
		bool bRegValue = false;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParentalControl) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalControl in CVibraniumParentalControl::On_GetSetRegistryVal KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			bFound = false;
			bFlagValue = false;
		}
		if (!bFound)
		{
			bFlagValue = true;
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwParentalCntrlFlg", bRegValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwParentalCntrlFlg in CVibraniumParentalControl::On_GetSetRegistryVal", 0, 0, true, SECONDLEVEL);
				bFlagValue = false;
			}
		}
		if (bFlagValue)
		{
			CString csRegVal;
			csRegVal.Format(L"%d", dwParentalControl);
			return json::value((SCITER_STRING)csRegVal);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::On_GetSetRegistryVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SendData2Service
*  Description    : To send data to Services
*  Author Name    : Jeena Mariam Saji
*  Date           : 18 June 2018
****************************************************************************************************/
bool CWardWizParentalControl::SendData2Service(DWORD dwMsg, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMsg;
		Sleep(30);
		CISpyCommunicator objCom(L"\\\\.\\pipe\\{AA7D87ADE4A34ac0A23D78E2D83F6058}", true, 0x03);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::SendData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WriteRegistryEntryOfSettingsTab
*  Description    : WriteRegistryEntryOfSettingsTab
*  Author Name    : Jeena Mariam Saji
*  Date           : 19 June 2018
****************************************************************************************************/
BOOL CWardWizParentalControl::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue)
{
	try
	{
		if (!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue, true))
		{
			AddLogEntry(L"### Error in Setting Registry CVibraniumParentalControl::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
		}
		Sleep(20);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : SetRegistrykeyUsingService
*  Author Name    : Jeena Mariam Saji
*  Date           : 19 June 2018
****************************************************************************************************/
bool CWardWizParentalControl::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;
		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;

		CISpyCommunicator objCom(L"\\\\.\\pipe\\{AA7D87ADE4A34ac0A23D78E2D83F6058}");

		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CVibraniumParentalControl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CVibraniumParentalControl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : OnChangeRegVal
*  Description    : OnChangeRegVal
*  Author Name    : Jeena Mariam Saji
*  Date           : 19 June 2018
*  Updated By     : Kunal Waghmare
*  Date           : 3 July 2018
****************************************************************************************************/
json::value CWardWizParentalControl::OnChangeRegVal(SCITER_VALUE svToggleState)
{
	try
	{
		bool bFlagValue = svToggleState.get(false);
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwParentalCntrlFlg", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwParentalCntrlFlg in CVibraniumParentalControl::OnChangeRegVal", 0, 0, true, SECONDLEVEL);
		}
		else
		{
			DWORD dwVal = bFlagValue ? 3 : 4;
			CString csInsertQuery,csSelectQuery;
			csInsertQuery = _T("INSERT INTO Wardwiz_ParentalCtrl_Details(db_PCDate, db_PCTime, db_PCActivity, db_Username) VALUES (Date('now'),Datetime('now','localtime'),");
			csInsertQuery.AppendFormat(L"%d",dwVal);
			csInsertQuery.Append(L",'");
			csInsertQuery += CString(theApp.GetCurrentUserName()).MakeLower();
			csInsertQuery.Append(_T("');"));
			CT2A ascii(csInsertQuery, CP_UTF8);
			INT64 iScanId = InsertDataToTable(ascii.m_psz);
		}
		DWORD dwValue = bFlagValue ? 0x01 : 0x00;

		if (bFlagValue)
		{
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwParCtrlActiveFlag", 1))
			{
				AddLogEntry(L"### Error in Setting Registry dwParCtrlActiveFlag inCVibraniumParentalControl::OnChangeRegVal", 0, 0, true, SECONDLEVEL);
			}
		}
		else
		{
			HWND hWindow = FindWindow(NULL, L"AKTRAYPWD");
			PostMessage(hWindow, WM_CLOSE, 0, 0);
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwParCtrlActiveFlag", 0))
			{
				AddLogEntry(L"### Error in Setting Registry dwParCtrlActiveFlag in CVibraniumParentalControl::OnChangeRegVal", 0, 0, true, SECONDLEVEL);
			}
		}
		ChangeParCtrlSettingOnToggler(bFlagValue);
		if (!SendData2ComService(RELOAD_PARENTAL_CONTROL_SETTINGS, dwValue))
		{
			AddLogEntry(L"### Failed to SendData2ComService RELOAD_PARENTAL_CONTROL_SETTINGS CVibraniumParentalControl::OnChangeRegVal", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(ON_RESET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to send data in SetRectTrackChangeForINet for ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::OnChangeRegVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Kunal Waghmare
*  Date           : 29 Jun 2018
*  SR_NO		  :
**********************************************************************************************************/
INT64 CWardWizParentalControl::InsertDataToTable(const char* szQuery)
{
	try
	{
		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		m_objSqlDb.SetDataBaseFilePath(dbPath.m_psz);
		m_objSqlDb.Open();
		int iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = m_objSqlDb.GetLastRowId();
		m_objSqlDb.Close();
		return iLastRowId;
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CVibraniumParentalControl::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	GetSQLiteDBFilePath
*  Description    :	Helper function to get Current working directory path
*  Author Name    : Kunal Waghmare
*  Date           : 29 Jun 2018
**********************************************************************************************************/
CString CWardWizParentalControl::GetSQLiteDBFilePath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';
		return(CString(szModulePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CVibraniumParentalControl::GetSQLiteDBFilePath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************************************
*  Function Name  : ChangeParCtrlSetting
*  Description    : Send data to service
*  Author Name    : Jeena Mariam Saji
*  Date           : 21 June 2018
****************************************************************************************************/
void CWardWizParentalControl::ChangeParCtrlSetting()
{
	try
	{
		if (!SendData2Service(RELOAD_USER_ACESS_LIST, true))
		{
			AddLogEntry(L"### Failed to ChangeParCtrlSetting in PARCTRLCHANGE for SHOW_PC_LOCK_WND", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(SHOW_PC_LOCK_WND, true))
		{
			AddLogEntry(L"### Failed to ChangeParCtrlSetting in PARCTRLCHANGE for SHOW_PC_LOCK_WND", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(ON_INET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to ChangeParCtrlSetting in PARCTRLCHANGE for ON_PC_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(ON_PC_INET_RESTRICT, true))
		{
			AddLogEntry(L"### Failed to ChangeParCtrlSetting in PARCTRLCHANGE for ON_PC_INET_RESTRICT", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::ChangeParCtrlSetting", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnSetRestriction
*  Description    : To change restriction
*  Author Name    : Jeena Mariam Saji
*  Date           : 21 June 2018
****************************************************************************************************/
json::value CWardWizParentalControl::OnSetRestriction()
{
	try
	{
		if (!SendData2Service(SHOW_PC_LOCK_WND, 0x00))
		{
			AddLogEntry(L"### Failed to Send Data to Service in CVibraniumParentalControl::OnSetRestriction", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::OnSetRestriction", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : OnSetINetRestriction
*  Description    : To change restriction
*  Author Name    : Nitin Shelar
*  Date           : 20 July 2018
****************************************************************************************************/
json::value CWardWizParentalControl::OnSetINetRestriction()
{
	try
	{
		if (!SendData2Service(ON_PC_INET_RESTRICT, 0x00))
		{
			AddLogEntry(L"### Failed to Send Data to Service in CVibraniumParentalControl::OnSetINetRestriction", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(ON_RESET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to send data in SetRectTrackChangeForINet for ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::OnSetINetRestriction", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : OnChangeAppRestriction
*  Description    : To change app restriction
*  Author Name    : Jeena Mariam Saji
*  Date           : 20 July 2018
****************************************************************************************************/
json::value CWardWizParentalControl::OnChangeAppRestriction()
{
	try
	{
		if (!SendData2Service(RELOAD_APP_RESTRICTION, 0x00))
		{
			AddLogEntry(L"### Failed to Send Data to Service in CVibraniumParentalControl::OnChangeAppRestriction", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::OnChangeAppRestriction", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	CheckForNetworkPath
*  Description    :	To check weather file is network file or not.
*  Author Name    : Kunal Waghmare
*  Date           : 23 July 2018
**********************************************************************************************************/
json::value CWardWizParentalControl::CheckForNetworkPath(SCITER_VALUE svFilePath)
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
		AddLogEntry(L"### Exception in CVibraniumParentalControl::CheckForNetworkPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetTargetFilePath
*  Description    :	To get target path of '.lnk' file.
*  Author Name    : Kunal Waghmare
*  Date           : 23 July 2018
**********************************************************************************************************/
json::value CWardWizParentalControl::GetTargetFilePath(SCITER_VALUE svFilePath)
{
	try
	{
		CString csFilePath = svFilePath.get(L"").c_str();
		CString csTargetFilePath;
		HRESULT hr;
		IShellLink* pIShellLink;
		hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pIShellLink);
	
		if (SUCCEEDED(hr))
		{
			IPersistFile* pIPersistFile;
			hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);
			if (SUCCEEDED(hr))
			{
				// Load the shortcut and resolve the path
				// IPersistFile::Load() expects a UNICODE string
				// so we're using the T2COLE macro for the conversion
				// For more info, check out MFC Technical note TN059
				// (these macros are also supported in ATL and are
				// so much better than the ::MultiByteToWideChar() family)
				hr = pIPersistFile->Load(T2COLE(csFilePath), STGM_READ);
				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = pIShellLink->GetPath(csTargetFilePath.GetBuffer(MAX_PATH),
						MAX_PATH,
						&wfd,
						SLGP_RAWPATH);
					csTargetFilePath.ReleaseBuffer(-1);
				}
				pIPersistFile->Release();
			}
			pIShellLink->Release();
		}

		return json::value(SCITER_STRING(csTargetFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::GetTargetFilePath", 0, 0, true, SECONDLEVEL);
	}

	return json::value(SCITER_STRING(L""));
}

/***************************************************************************************************
*  Function Name  : OnSetUserTogglerVal
*  Description    : To change settings on chnage of toggler
*  Author Name    : Jeena Mariam Saji
*  Date           : 06 August 2018
****************************************************************************************************/
json::value CWardWizParentalControl::OnSetUserTogglerVal(SCITER_VALUE svUserValue, SCITER_VALUE svUserStatusValue)
{
	try
	{
		CString csUserVal = svUserValue.get(L"").c_str();
		bool bUserStatusVal = svUserStatusValue.get(false);
		ChangeParCtrlSetting();
		if (!bUserStatusVal)
		{
			SendParCtrlMessage2Tray(SEND_PC_TRAY_CLOSE, csUserVal);
		}
		if (!SendData2Service(ON_RESET_RESTRICTION, true))
		{
			AddLogEntry(L"### Failed to send data in SetRectTrackChangeForINet for ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::OnSetUserTogglerVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : SendData2ComService()
*  Description    : Send message to communicationService
*  Author Name    : Amol Jaware
*  Date           : 07 Aug 2018
***********************************************************************************************/
bool CWardWizParentalControl::SendData2ComService(int iMessageInfo, DWORD dwValue, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = dwValue;

		CISpyCommunicator objCom(L"\\\\.\\pipe\\{AA7D87ADE4A34ac0A23D78E2D83F6058}");
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CVibraniumParentalControl::SendData2ComService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CVibraniumParentalControl::SendData2ComService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::SendData2ComService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CheckIsWrdWizFile
*  Description    : To check path is WardWiz own file or not.
*  Author Name    : Amol Jaware
*  Date           :	27 Nov 2018
****************************************************************************************************/
json::value CWardWizParentalControl::CheckIsWrdWizFile(SCITER_VALUE svFilePathQuarantine)
{
	try
	{
		CString csQuarFilePath = svFilePathQuarantine.get(L"").c_str();
		CString csWardWizPath = GetWardWizPathFromRegistry();

		if (_tcscmp(csWardWizPath, csQuarFilePath.Left(csWardWizPath.GetLength())) == 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::CheckIsWrdWizFile", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : OnClickSetPage
*  Description    : This function is used to set the setting page name.
*  Author Name    : Amol Jaware
*  Date           :	18 Dec 2018
****************************************************************************************************/
json::value CWardWizParentalControl::OnClickSetPage()
{
	try
	{
		theApp.m_csPageName = L"#PARENTAL_CONTROL";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::OnClickSetPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnClickSetOtherPage
*  Description    : This function is used to set the none page name.
*  Author Name    : Amol Jaware
*  Date           :	18 Dec 2018
****************************************************************************************************/
json::value CWardWizParentalControl::OnClickSetOtherPage()
{
	try
	{
		theApp.m_csPageName = L"#NONE";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::OnClickSetOtherPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : CreatePCBrowseSecDB
*  Description    : Function to create firewall database.
*  Author Name    : Swapnil Bhave
*  Date			  :	02 July 2018
****************************************************************************************************/
json::value CWardWizParentalControl::CreatePCBrowseSecDB()
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csAppRulesDB = L"";
		CStringA csInsertQuery;

		csAppRulesDB.Format(L"%sVBPCWEBFILTER.DB", csWardWizModulePath);

		CT2A dbPath(csAppRulesDB, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);

		objSqlDb.Open();
	
		csInsertQuery.Format("CREATE TABLE IF NOT EXISTS WWIZ_BROWSE_PROTECTION(db_BrowseID INTEGER PRIMARY KEY NOT NULL, FK_USERNAME NVARCHAR(250) NOT NULL, db_Browse BOOL, db_Blk_Category BOOL, db_Blk_Website BOOL,db_Res1 NVCHAR(212),db_Res2 NVCHAR(212),db_Res3 NVCHAR(212));");
		objSqlDb.ExecDML(csInsertQuery);

		csInsertQuery.Format("CREATE TABLE IF NOT EXISTS WWIZ_AGE_WEB_CATEGORIES(ID INTEGER PRIMARY KEY NOT NULL, db_AgeGroupId INTEGER NOT NULL, FK_USERNAME NVARCHAR(250) NOT NULL, db_LastStatus INTEGER,db_Adult INTEGER,db_Advertisement INTEGER,db_Aggressive INTEGER,db_Bank INTEGER,db_Bitcoin INTEGER,db_Chat INTEGER,db_Child INTEGER,db_Dating INTEGER,db_Download INTEGER,db_Drug INTEGER,db_Financial INTEGER,db_FreeWebEmails INTEGER,db_Grambling INTEGER,db_Games INTEGER,db_JobSearch INTEGER,db_SexEducation INTEGER,db_Shopping INTEGER,db_SocNetworking INTEGER,db_Sports INTEGER,db_VideoStreaming INTEGER,db_AllOther INTEGER,db_Res1 NVCHAR(212),db_Res2 NVCHAR(212),db_Res3 NVCHAR(212),db_Res4 INTEGER,db_Res5 INTEGER,db_Res6 INTEGER,db_Res7 INTEGER,db_Res8 INTEGER,db_Res9 INTEGER,db_Res10 INTEGER);");
		objSqlDb.ExecDML(csInsertQuery);

		csInsertQuery.Format("CREATE TABLE IF NOT EXISTS WWIZ_MANAGE_EXCLUSION_LIST(ID INTEGER PRIMARY KEY NOT NULL, FK_USERNAME NVARCHAR(250) NOT NULL, db_ExcList NVARCHAR(256),db_Res1 NVCHAR(212),db_Res2 NVCHAR(212),db_Res3 NVCHAR(212))");
		objSqlDb.ExecDML(csInsertQuery);

		csInsertQuery.Format("CREATE TABLE IF NOT EXISTS WWIZ_SPEC_WEB_BLOCK(ID INTEGER PRIMARY KEY NOT NULL, FK_USERNAME NVARCHAR(250) NOT NULL, db_BlockWebsiteName NVARCHAR(256),db_IsSubDomain INTEGER, db_Res1 NVCHAR(212),db_Res2 NVCHAR(212),db_Res3 NVCHAR(212))");
		objSqlDb.ExecDML(csInsertQuery);

		objSqlDb.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::CreatePCBrowseSecDB", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***********************************************************************************************
Function Name  : GetDBPathBlockList
Description    : Get Database path for Blocked List
Author Name    : Swapnil Bhave
Date           : 02 July 2019
***********************************************************************************************/
json::value CWardWizParentalControl::GetPCBrowseSecDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBPCWEBFILTER.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::GetPCBrowseSecDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : SendParCtrlMessage2Tray
*  Description    : This function is used to set the none page name.
*  Author Name    : Jeena Mariam Saji
*  Date           :	24 June 2019
****************************************************************************************************/
bool CWardWizParentalControl::SendParCtrlMessage2Tray(int iRequest, CString csUserName)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;
		_tcscpy(szPipeData.szFirstParam, csUserName.GetBuffer());

		CISpyCommunicator objCom(TRAY_SERVER + csUserName, true, 0x03, 0x12C);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumParentalCntrl::SendParCtrlMessage2Tray", 0, 0, true, ZEROLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::SendParCtrlMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReloadDB4WebAgeSec
*  Description    : This function is used to reload db for age web security.
*  Author Name    : Amol Jaware
*  Date           :	10 July 2019
****************************************************************************************************/
json::value CWardWizParentalControl::ReloadDB4WebAgeSec()
{
	try
	{
		if (SendData2Service(RELOAD_WEB_SEC_DB, true))
		{
			AddLogEntry(L"### Failed to reload db in SendData2Service", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::ReloadDB4WebAgeSec", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : ReloadBrowseProtDB
*  Description    : This function is used to reload db for Browser protection.
*  Author Name    : Amol Jaware
*  Date           :	13 July 2019
****************************************************************************************************/
json::value CWardWizParentalControl::ReloadBrowseProtDB()
{
	try
	{
		if (SendData2Service(RELOAD_BROWSE_PROT_DB, true))
		{
			AddLogEntry(L"### Failed to reload VibraniumBrowseProtection db in SendData2Service", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::ReloadBrowseProtDB", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : ReloadDB4BlkSpecWeb
*  Description    : This function is used to reload db for Browser protection.
*  Author Name    : Amol Jaware
*  Date           :	13 July 2019
****************************************************************************************************/
json::value CWardWizParentalControl::ReloadDB4BlkSpecWeb()
{
	try
	{
		if (SendData2Service(RELOAD_BLOCK_SPEC_WEB_PAGE, true))
		{
			AddLogEntry(L"### Failed to reload RELOAD_BLOCK_SPEC_WEB_PAGE db in SendData2Service", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::ReloadDB4BlkSpecWeb", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : ReloadDB4MngExc
*  Description    : This function is used to reload db for Browser protection.
*  Author Name    : Amol Jaware
*  Date           :	13 July 2019
****************************************************************************************************/
json::value CWardWizParentalControl::ReloadDB4MngExc()
{
	try
	{
		if (SendData2Service(RELOAD_MNG_EXCLUSION, true))
		{
			AddLogEntry(L"### Failed to reload RELOAD_MNG_EXCLUSION db in SendData2Service", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumParentalCntrl::ReloadDB4MngExc", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : ChangeParCtrlSettingOnToggler
*  Description    : This function is used to change parental control settings on toggler on and off
*  Author Name    : Jeena Mariam Saji
*  Date           :	22 July 2019
****************************************************************************************************/
void CWardWizParentalControl::ChangeParCtrlSettingOnToggler(bool bTogglerVal)
{
	try
	{
		if (bTogglerVal)
		{
			if (!SendData2Service(ON_PC_INITIALISE, true))
			{
				AddLogEntry(L"### Failed to SendData2Service in PARCTRLCHANGE for ON_PC_INITIALISE", 0, 0, true, SECONDLEVEL);
			}
		}
		else
		{
			if (!SendData2Service(ON_PC_UNINITIALISE, true))
			{
				AddLogEntry(L"### Failed to SendData2Service in PARCTRLCHANGE for ON_PC_INITIALISE", 0, 0, true, SECONDLEVEL);
			}
		}
		ChangeParCtrlSetting();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::ChangeParCtrlSettingOnToggler", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckDomainName
Description    : Check whether domain name is 'wardwiz' or not
Author Name    : Kunal Waghmare
Date           : 25 July 2019
***********************************************************************************************/
json::value CWardWizParentalControl::CheckDomainName(SCITER_VALUE scr_Website)
{
	try
	{
		wchar_t szHostName[MAX_PATH] = L"";
		wchar_t szURLPath[MAX_PATH * 5] = L"";
		URL_COMPONENTS urlComp;
		memset(&urlComp, 0, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);
		urlComp.lpszHostName = szHostName;
		urlComp.dwHostNameLength = MAX_PATH;
		urlComp.lpszUrlPath = szURLPath;
		urlComp.dwUrlPathLength = MAX_PATH * 5;
		urlComp.dwSchemeLength = 1; // None zero

		SCITER_STRING  strWebsite = scr_Website.get(L"");
		CString csFullURL = strWebsite.c_str();
		
		if (csFullURL.Find(L"http://") == -1 && csFullURL.Find(L"http://") == -1)
			csFullURL.Insert(0, L"http://");
		if (!::WinHttpCrackUrl(csFullURL, static_cast<DWORD>(csFullURL.GetLength()), 0, &urlComp))
		{
			AddLogEntry(L"### WinHttpCrackUrl failed URLPath", 0, 0, true, SECONDLEVEL);
		}

		CString csHostName = urlComp.lpszHostName;
		csHostName.Replace(L"www.", L"");

		int iPos = 0;
		for (CString sItem = csHostName.Tokenize(L".", iPos); iPos >= 0; sItem = csHostName.Tokenize(L".", iPos))
		{
			if(sItem.Compare(L"Vibranium") == 0)
				return json::value((SCITER_STRING)L"1");
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::CheckDomainName", 0, 0, true, SECONDLEVEL);
	}
	
	return json::value((SCITER_STRING)L"0");
}

/***********************************************************************************************
Function Name  : IsDomainName
Description    : If sub domain matched return true.
Author Name    : Amol Jaware
Date           : 31 July 2019
***********************************************************************************************/
json::value CWardWizParentalControl::IsDomainName(SCITER_VALUE svHostName, SCITER_VALUE svInputURL)
{
	bool bFlag = false;
	try
	{
		CString csDomainNameDB = GetDomainNameFromURL(svHostName.get(L"").c_str());
		if (csDomainNameDB.GetLength() == 0)
		{
			return bFlag;
		}

		csDomainNameDB.Replace(L"www.", L"");
		CString csDomainNameURL = GetDomainNameFromURL(svInputURL.get(L"").c_str());

		if (csDomainNameURL.GetLength() == 0)
		{
			return bFlag;
		}

		csDomainNameURL.Replace(L"www.", L"");

		wchar_t *chpString = wcsstr(csDomainNameDB.GetBuffer(), csDomainNameURL.GetBuffer());

		if (chpString)
			bFlag = true;
		else
		{
			bFlag = false;
			wchar_t *chpStrings = wcsstr(csDomainNameURL.GetBuffer(), csDomainNameDB.GetBuffer());
			if (chpStrings)
				bFlag = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::IsDomainName", 0, 0, true, SECONDLEVEL);
	}
	return bFlag;
}

/***********************************************************************************************
Function Name  : GetDomainNameFromURL
Description    : To get Domain name from url
Author Name    : Jeena MAriam SAji
Date           : 07 Aug 2019
***********************************************************************************************/
CString CWardWizParentalControl::GetDomainNameFromURL(CString csURL)
{
	try
	{
		wchar_t szHostName[MAX_PATH] = L"";
		wchar_t szURLPath[MAX_PATH * 5] = L"";
		URL_COMPONENTS urlComp;
		memset(&urlComp, 0, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);
		urlComp.lpszHostName = szHostName;
		urlComp.dwHostNameLength = MAX_PATH;
		urlComp.lpszUrlPath = szURLPath;
		urlComp.dwUrlPathLength = MAX_PATH * 5;
		urlComp.dwSchemeLength = 1; // None zero

		CString csFullURL = csURL;
		if ((csFullURL.Find(L"http://") == -1) && (csFullURL.Find(L"https://") == -1))
		{
			csFullURL.Insert(0, L"http://");
		}
		if (!::WinHttpCrackUrl(csFullURL, static_cast<DWORD>(csFullURL.GetLength()), 0, &urlComp))
		{
			AddLogEntry(L"### WinHttpCrackUrl failed in CVibraniumParentalControl::GetDomainNameFromURL", 0, 0, true, SECONDLEVEL);
		}
		return CString(urlComp.lpszHostName);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalControl::GetDomainNameFromURL", 0, 0, true, SECONDLEVEL);
	}
	return EMPTY_STRING;
}