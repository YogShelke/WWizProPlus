// CWardWizEmailScan.cpp : implementation file
/*********************************************************************
*  Program Name		: CWardWizEmailScan.cpp
*  Description		: This class contains functionality for the scanning of emails with or without attachments, 
					  and settings about the actions to be performed
*  Author Name		: Amol Jaware
*  Date Of Creation	: 21 March 2017
*  Version No		: 3.1.0.0
**********************************************************************/
#include "stdafx.h"
#include "WardWizEmailScan.h"
#include "WardWizDatabaseInterface.h"

/***********************************************************************************************
Function Name  : CWardWizEmailScan
Description    : Constructor
Author Name    : Amol Jaware
Date           : 21 March 2017
***********************************************************************************************/
CWardWizEmailScan::CWardWizEmailScan() : behavior_factory("WardWizEmailScan")
, m_objComService(SERVICE_SERVER, true, 3)
, m_hSQLiteDLL(NULL)
, m_bISDBLoaded(false)
{
	LoadRequiredLibrary();
}

CWardWizEmailScan::~CWardWizEmailScan()
{
	if (m_hSQLiteDLL != NULL)
	{
		FreeLibrary(m_hSQLiteDLL);
		m_hSQLiteDLL = NULL;
}
}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : This function is used to send DB file path to UI
*  Author Name    : Amol Jaware
*  Date           :	21 March 2017
****************************************************************************************************/
json::value CWardWizEmailScan::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VibraniumEMAILSCAN.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : On_WriteSubjectInINI
*  Description    : This function is used to write email subject in ProductSettings.ini file.
*  Author Name    : Amol Jaware
*  Date           :	16 feb 2018
****************************************************************************************************/
json::value CWardWizEmailScan::On_WriteSubjectInINI(SCITER_VALUE svSubject)
{
	try
	{
		CString csSubject(L"");
		sciter::string strSubject;
		strSubject = svSubject.to_string();
		csSubject = strSubject.c_str();
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"EmailSubject", csSubject, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::On_WriteSubjectInINI", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : On_ClickChangeSettings
Description    : Write Current settings values from Registry
Author Name    : Amol Jaware.
Date           : 03rd March 2018
***********************************************************************************************/
json::value CWardWizEmailScan::On_ClickChangeSettings(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue)
{
	try
	{
		On_ClickSettingsButton(svIndexValue, svBoolSettingsTrayNotify, svintSettingsRegValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::On_ClickChangeSettings", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : On_ClickSettingsButton
Description    : Write Current settings values from Registry
Author Name    : Amol Jaware.
Date           : 03rd March 2018
***********************************************************************************************/
void CWardWizEmailScan::On_ClickSettingsButton(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue)
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		int iButtonClickID = static_cast<int>(svIndexValue.d);
		bool bFlagValue = svBoolSettingsTrayNotify.get(false);
		int iRegistryValue = static_cast<int>(svintSettingsRegValue.d);
		switch (iButtonClickID)
		{
			case AUTO_QUARNTINE:
				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailQuarantineOpt", iRegistryValue))
				{
					AddLogEntry(L"### Error in Setting Registry dwEmailQuarantineOpt in CVibraniumEmailScan::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
				}

				if (!SendSettingsToService(APPLYPRODUCTSETTINGS, AUTOQUARENTINEOPTION, iRegistryValue))
				{
					AddLogEntry(L"### Failed to SendSettingsToService in CVibraniumEmailScan::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
				}
				break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : WriteRegistryEntryOfSettingsTab
*  Description    : this function is  called for Writing the Registry Key Value for Menu Items
*  Author Name    : Amol Jaware.
*  Date           : 03rd March 2018
****************************************************************************************************/
BOOL CWardWizEmailScan::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue)
{
	try
	{
		if (!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue, true))
		{
			AddLogEntry(L"### Error in Setting Registry CVibraniumEmailScan::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
		}
		Sleep(20);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : this function is  called for setting the Registry Keys using the Services
*  Author Name    : Amol Jaware
*  Date           : 03rd March 2018
****************************************************************************************************/
bool CWardWizEmailScan::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
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

		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CVibraniumEmailScan::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CVibraniumEmailScan::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SendSettingsToService
*  Description    : Function to send settings to service.
*  Author Name    : Amol Jaware.
*  Date           :	03rd March 2018
****************************************************************************************************/
bool CWardWizEmailScan::SendSettingsToService(int iMessageInfo, DWORD dwValue, DWORD dwSecondValue)
{
	__try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = dwValue;
		szPipeData.dwSecondValue = dwSecondValue;
		if (!m_objComService.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in SendSettingsToService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		TCHAR szMessage[MAX_PATH] = { 0 };
		swprintf_s(szMessage, L"MessageID: %d, Value: %d, SecondValue: %d", iMessageInfo, dwValue, dwSecondValue);
		AddLogEntry(L"### Exception in CVibraniumEmailScan::SendSettingsToService, %s", szMessage, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary
*  Description    : Function which load required DLL files
*  Author Name    : Tejas Tanaji Shinde
*  Date           :	27 April 2018
****************************************************************************************************/
bool CWardWizEmailScan::LoadRequiredLibrary()
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
		AddLogEntry(L"### Exception in CVibraniumEmailScan::LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : GetSubjectFromINI
*  Description    : This function is used to read the subject from ProductSettings.ini file.
*  Author Name    : Amol Jaware
*  Date           :	30 May 2018
****************************************************************************************************/
json::value CWardWizEmailScan::GetSubjectFromINI()
{
	TCHAR szSubjectValueData[512] = { 0 };
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		GetPrivateProfileString(L"VBSETTINGS", L"EmailSubject", L"", szSubjectValueData, 511, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::GetSubjectFromINI", 0, 0, true, SECONDLEVEL);
	}
	return szSubjectValueData;
}

/***************************************************************************************************
*  Function Name  : OnClickSetPage
*  Description    : This function is used to set the emailscan page name.
*  Author Name    : Amol Jaware
*  Date           :	31 May 2018
****************************************************************************************************/
json::value CWardWizEmailScan::OnClickSetPage()
{
	try
	{
		theApp.m_csPageName = L"#EMAILSCAN";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::OnClickSetPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnClickSetOtherPage
*  Description    : This function is used to set the none page name.
*  Author Name    : Amol Jaware
*  Date           :	31 May 2018
****************************************************************************************************/
json::value CWardWizEmailScan::OnClickSetOtherPage()
{
	try
	{
		theApp.m_csPageName = L"#NONE";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::OnClickSetOtherPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnSetRegVal
*  Description    : This function is used to set the registry value on toggler button
*  Author Name    : Nitin Shelar
*  Date           :	19 Jun 2018
****************************************************************************************************/
json::value CWardWizEmailScan::OnSetRegVal(SCITER_VALUE svbToggleState)
{
	try
	{
		bool bFlagValue = svbToggleState.get(false);
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScanState", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwParentalCntrlFlg in CVibraniumEmailScan::OnSetRegVal", 0, 0, true, SECONDLEVEL);
		}

		DWORD dwSecondValue = bFlagValue ? 0x01 : 0x00;
		if (!SendData2ComService(RELOAD_EMAIL_SCAN_SETTINGS, dwSecondValue))
		{
			AddLogEntry(L"### Failed to SendData2ComService RELOAD_EMAIL_SCAN_SETTINGS CVibraniumEmailScan::OnSetRegVal", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::OnSetRegVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetRegVal
*  Description    : This function is used to get the registry value on toggler button
*  Author Name    : Nitin Shelar
*  Date           :	19 Jun 2019
****************************************************************************************************/
json::value CWardWizEmailScan::GetRegVal()
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		DWORD dwEmailScan = 0x00;
		CITinRegWrapper	 m_objReg;
		bool bFound = true;
		bool bFlagValue = true;
		bool bRegValue = false;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwEmailScanState", dwEmailScan) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalControl in CVibraniumEmailScan::GetRegVal KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			bFound = false;
			bFlagValue = false;
		}
		if (!bFound)
		{
			bFlagValue = true;
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScanState", bRegValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwParentalCntrlFlg in CVibraniumEmailScan::GetRegVal", 0, 0, true, SECONDLEVEL);
				bFlagValue = false;
			}
		}
		if (bFlagValue)
		{
			CString csRegVal;
			csRegVal.Format(L"%d", dwEmailScan);
			return json::value((SCITER_STRING)csRegVal);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumEmailScan::GetRegVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : SendData2ComService()
*  Description    : Send message to communicationService
*  Author Name    : Amol Jaware
*  Date           : 07 Aug 2018
***********************************************************************************************/
bool CWardWizEmailScan::SendData2ComService(int iMessageInfo, DWORD dwValue, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = dwValue;
		
		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CVibraniumEmailScan::SendData2ComService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CVibraniumEmailScan::SendData2ComService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::SendData2ComService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}