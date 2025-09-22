// CWrdWizBrowserSecurity.cpp : implementation file
/*********************************************************************
*  Program Name		: CWrdWizBrowserSecurity.cpp
*  Description		: For Bowser Security
*  Author Name		: Jeena Mariam Saji
*  Date Of Creation	: 20th August 2019
*  Version No		: 2.0.0.1
**********************************************************************/
#include "stdafx.h"
#include "WrdWizBrowserSecurity.h"

CWrdWizBrowserSecurity::CWrdWizBrowserSecurity() : behavior_factory("WardWizBrowserSecurity")
{
}


CWrdWizBrowserSecurity::~CWrdWizBrowserSecurity()
{
}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : To get DB Path
*  Author Name    : Jeena Mariam Saji
*  Date           : 20 August 2019
****************************************************************************************************/
json::value CWrdWizBrowserSecurity::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBBROWSEC.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : GetReportDBPath
*  Description    : To get DB Path
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Sept 2019
****************************************************************************************************/
json::value CWrdWizBrowserSecurity::GetReportDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBALLREPORTS.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::GetReportDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : OnSetRegVal
*  Description    : To set registry value
*  Author Name    : Jeena Mariam Saji
*  Date           : 20 August 2019
****************************************************************************************************/
json::value CWrdWizBrowserSecurity::OnSetRegVal(SCITER_VALUE svbToggleState)
{
	try
	{
		bool bFlagValue = svbToggleState.get(false);
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		CString cBoolVal = bFlagValue ? _T("1") : _T("0");
		CString csInsertQuery;
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBrowserSecurityState", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwBrowserSecurityState in CVibraniumBrowserSecurity::OnSetRegVal", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::OnSetRegVal", 0, 0, true, SECONDLEVEL);
	}	
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetSetRegistryVal
*  Description    : To get and set Registry value
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Sept 2019
****************************************************************************************************/
json::value CWrdWizBrowserSecurity::On_GetSetRegistryVal()
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		DWORD dwBrowserSecurity = 0x00;
		CITinRegWrapper	 m_objReg;
		bool bFound = true;
		bool bFlagValue = true;
		bool bRegValue = false;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwBrowserSecurityState", dwBrowserSecurity) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwBrowserSecurity in CVibraniumBrowserSecurity::On_GetSetRegistryVal KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			bFound = false;
			bFlagValue = false;
		}
		if (!bFound)
		{
			bFlagValue = true;
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBrowserSecurityState", bRegValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwBrowserSecurityState in CVibraniumBrowserSecurity::On_GetSetRegistryVal", 0, 0, true, SECONDLEVEL);
				bFlagValue = false;
			}
		}
		if (bFlagValue)
		{
			CString csRegVal;
			csRegVal.Format(L"%d", dwBrowserSecurity);
			return json::value((SCITER_STRING)csRegVal);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::On_GetSetRegistryVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/***************************************************************************************************
*  Function Name  : WriteRegistryEntryOfSettingsTab
*  Description    : this function is  called for Writing the Registry Key Value for Menu Items
*  Author Name    : Jeena Mariam Saji
*  Date           : 20 August 2019
****************************************************************************************************/
BOOL CWrdWizBrowserSecurity::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue)
{
	try
	{
		if (!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue, true))
		{
			AddLogEntry(L"### Error in Setting Registry CVibraniumBrowserSecurity::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
		}
		Sleep(20);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : To set registry value using service
*  Author Name    : Jeena Mariam Saji
*  Date           : 20 August 2019
****************************************************************************************************/
bool CWrdWizBrowserSecurity::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
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
			AddLogEntry(L"### Failed to set data in CVibraniumBrowserSecurity::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CVibraniumBrowserSecurity::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : OnReloadBrowserSecurity
*  Description    : To Reload Browser Security Values
*  Author Name    : Jeena Mariam Saji
*  Date           : 05 Sept 2019
****************************************************************************************************/
json::value CWrdWizBrowserSecurity::OnReloadBrowserSecurity(SCITER_VALUE svbToggleState)
{
	try
	{
		bool bFlagValue = svbToggleState.get(false);
		if (bFlagValue)
		{
			if (!SendData2Service(RELOAD_BROWSER_SECURITY, true))
			{
				AddLogEntry(L"### Failed to send data in OnReloadBrowserSecRules for RELOAD_BROWSER_SECURITY", 0, 0, true, SECONDLEVEL);
			}
		}
		else
		{
			if (!SendData2Service(ON_BROWSER_SECURITY_UNINITIALIZE, true))
			{
				AddLogEntry(L"### Failed to send data in OnReloadBrowserSecRules for ON_BROWSER_SECURITY_UNINITIALIZE", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::OnReloadBrowserSecurity", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : OnReloadBrowserSecRules
*  Description    : To Reload Browser Security Rules
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Sept 2019
****************************************************************************************************/
json::value CWrdWizBrowserSecurity::OnReloadBrowserSecRules()
{
	try
	{
		if (!SendData2Service(RELOAD_BROWSER_SEC, true))
		{
			AddLogEntry(L"### Failed to send data in OnReloadBrowserSecRules for RELOAD_BROWSER_SEC", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::OnReloadBrowserSecRules", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : OnReloadBrowserSecRules
*  Description    : To Reload Browser Security Exclusion list
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Sept 2019
****************************************************************************************************/
json::value CWrdWizBrowserSecurity::ReloadDB4MngBrowserSecExc()
{
	try
	{
		if (!SendData2Service(RELOAD_BROWSER_SEC_EXC, true))
		{
			AddLogEntry(L"### Failed to send data in ReloadDB4MngBrowserSecExc for RELOAD_BROWSER_SEC_EXC", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::ReloadDB4MngBrowserSecExc", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : ReloadDB4MngBrowserSecSpec
*  Description    : To Reload Browser Security specific list
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Sept 2019
****************************************************************************************************/
json::value CWrdWizBrowserSecurity::ReloadDB4MngBrowserSecSpec()
{
	try
	{
		if (!SendData2Service(RELOAD_BROWSER_SEC_SPEC, true))
		{
			AddLogEntry(L"### Failed to send data in ReloadDB4MngBrowserSecSpec for RELOAD_BROWSER_SEC_SPEC", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::ReloadDB4MngBrowserSecSpec", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : SendData2Service
*  Description    : To send data to service
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Sept 2019
****************************************************************************************************/
bool CWrdWizBrowserSecurity::SendData2Service(DWORD dwMsg, bool bWait)
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
		AddLogEntry(L"### Exception in CVibraniumBrowserSecurity::SendData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}