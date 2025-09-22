// CWardWizFirewall.cpp : implementation file
/*********************************************************************
*  Program Name		: CWardWizFirewall.cpp
*  Description		: This class contains functionality for firewall settings
*  Author Name		: Kunal Waghmare
*  Date Of Creation	: 30 April 2018
**********************************************************************/
#include "stdafx.h"
#include "WardWizFireWall.h"

/***********************************************************************************************
*  Function Name  : CWardWizFirewall
*  Description    : Constructor
*  SR.NO		   :
*  Author Name    : Kunal Waghmare
*  Date           :	30 April 2018
***********************************************************************************************/
CWardWizFirewall::CWardWizFirewall() : 
behavior_factory("WardWizFirewall")
, m_objComService(SERVICE_SERVER, true, 3)
, m_hSQLiteDLL(NULL)
, m_bISDBLoaded(false)
{
	LoadRequiredLibrary();
}

/***********************************************************************************************
*  Function Name  : CWardWizFirewall
*  Description    : Destructor
*  SR_NO		  :
*  Author Name    : Kunal Waghmare
*  Date           :	30 April 2018
***********************************************************************************************/
CWardWizFirewall::~CWardWizFirewall()
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
*  SR_NO		  :
*  Author Name    : Kunal Waghmare
*  Date           :	30 April 2018
****************************************************************************************************/
json::value CWardWizFirewall::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBFIREWALL.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary
*  Description    : Function which load required DLL files
*  Author Name    : Kunal Waghmare
*  SR_NO		  :
*  Date           : 30 April 2018
****************************************************************************************************/
bool CWardWizFirewall::LoadRequiredLibrary()
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
		AddLogEntry(L"### Exception in CVibraniumFirewall::LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

json::value CWardWizFirewall::OnClickApplyNow()
{
	try
	{
		SendData2ComService(RELOAD_APPLICATION_RULES);
		return json::value(1);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::OnClickApplyNow", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
*  Function Name  : SendData2Service()
*  Description    : Send message to communicationService
*  Author Name    : Kunal Waghmare
*  Date           : 4th Aug 2018
***********************************************************************************************/
json::value CWardWizFirewall::OnClickEdit()
{
	try
	{
		SendData2ComService(RELOAD_PACKET_RULES);
		return json::value(1);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::OnClickEdit", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
*  Function Name  : SendData2Service()
*  Description    : Send message to communicationService
*  Author Name    : Ramkrushna Shelke
*  SR_No		  :
*  Date           : 29/09/2016
***********************************************************************************************/
bool CWardWizFirewall::SendData2ComService(int iMessageInfo, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;

		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CWardwizUIApp::SendData2ComService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CWardwizUIApp::SendData2ComService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizGUIApp::SendData2ComService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : SendData2ComService()
*  Description    : Send message to communicationService
*  Author Name    : Ramkrushna Shelke
*  SR_No		  :
*  Date           : 29/09/2016
***********************************************************************************************/
bool CWardWizFirewall::SendData2ComService(int iMessageInfo, DWORD dwValue, DWORD dwSecondValue, LPTSTR szFirstParam, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = dwValue;
		szPipeData.dwSecondValue = dwSecondValue;
		_tcscpy(szPipeData.szFirstParam, szFirstParam);


		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CVibraniumFirewall::SendData2ComService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CVibraniumFirewall::SendData2ComService", 0, 0, true, SECONDLEVEL);
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

/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : this function is  called for setting the Registry Keys using the Services
*  Author Name    : Nitin Shelar
*  Date           : 20 June 2018
****************************************************************************************************/
bool CWardWizFirewall::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
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
			AddLogEntry(L"### Failed to set data in CVibraniumFirewall::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CVibraniumFirewall::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WriteRegistryEntryOfSettingsTab
*  Description    : this function is  called for Writing the Registry Key Value for Menu Items
*  Author Name    : Nitin Shelar.
*  Date           : 20 June 2018
****************************************************************************************************/
BOOL CWardWizFirewall::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue)
{
	try
	{
		if (!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue, true))
		{
			AddLogEntry(L"### Error in Setting Registry CVibraniumFirewall::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
		}
		Sleep(20);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : OnSetRegVal
*  Description    : This function is used to set the registry value on toggler button
*  Author Name    : Nitin Shelar
*  Date           :	20 Jun 2018
*  Updated By	  : Kunal Waghmare
*  Updated Date	  : 26 Jun 2018	
****************************************************************************************************/
json::value CWardWizFirewall::OnSetRegVal(SCITER_VALUE svbToggleState)
{
	try
	{
		bool bFlagValue = svbToggleState.get(false);
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		CString cBoolVal = bFlagValue ? _T("1") : _T("0");
		CString csInsertQuery;
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwFirewallEnableState", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwFirewallEnableState in CVibraniumFirewall::OnSetRegVal", 0, 0, true, SECONDLEVEL);
		}
		else
		{	
			CString	csUserName = theApp.GetCurrentUserName();
			csInsertQuery = _T("INSERT INTO Wardwiz_FirewallDetails(db_FirewallDate, db_FirewallTime, db_FirewallEnabled, db_Username) VALUES (Date('now'),Datetime('now','localtime'),");
			csInsertQuery.Append(cBoolVal);
			csInsertQuery.Append(L",'");
			csInsertQuery.Append(csUserName);
			csInsertQuery.Append(_T("');"));
			CT2A ascii(csInsertQuery, CP_UTF8);
			INT64 iScanId = InsertDataToTable(ascii.m_psz);
		}
		DWORD dwSecondValue = bFlagValue ? 0x01 : 0x00;
		//TODO: GUIID
		if (!SendData2ComService(RELOAD_FIREWALL_SETTINGS, RELOAD_FIREWALL_ON_OF, dwSecondValue, L"{20405C0F-3A0A-4C66-9986-E3F1C7B5A197}"))
		{
			AddLogEntry(L"### Failed to SendData2ComService RELOAD_FIREWALL_ON_OF CVibraniumFirewall::OnSetRegVal", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::OnSetRegVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Kunal Waghmare
*  Date           : 26 Jun 2018
*  SR_NO		  :
**********************************************************************************************************/
INT64 CWardWizFirewall::InsertDataToTable(const char* szQuery)
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
		AddLogEntry(L"### Exception in CVibraniumFirewall::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	GetSQLiteDBFilePath
*  Description    :	Helper function to get Current working directory path
*  Author Name    : Kunal Waghmare
*  Date           : 26 Jun 2018
**********************************************************************************************************/
CString CWardWizFirewall::GetSQLiteDBFilePath()
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
		AddLogEntry(L"### Exception in value CVibraniumFirewall::GetSQLiteDBFilePath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************************************
*  Function Name  : GetRegVal
*  Description    : This function is used to get the registry value on toggler button
*  Author Name    : Nitin Shelar
*  Date           :	20 Jun 2019
****************************************************************************************************/
json::value CWardWizFirewall::GetRegVal()
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		DWORD dwFirewall = 0x00;
		CITinRegWrapper	 m_objReg;
		bool bFound = true;
		bool bFlagValue = true;
		bool bRegValue = false;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwFirewallEnableState", dwFirewall) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwFirewallEnableState in CVibraniumFirewall::GetRegVal KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			bFound = false;
			bFlagValue = false;
		}
		if (!bFound)
		{
			bFlagValue = true;
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwFirewallEnableState", bRegValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwFirewallEnableState in CVibraniumFirewall::GetRegVal", 0, 0, true, SECONDLEVEL);
				bFlagValue = false;
			}
		}
		if (bFlagValue)
		{
			CString csRegVal;
			csRegVal.Format(L"%d", dwFirewall);
			return json::value((SCITER_STRING)csRegVal);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::GetRegVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	CheckForNetworkPath
*  Description    :	To check weather file is network file or not.
*  Author Name    : Kunal Waghmare
*  SR_NO		  :
*  Date           : 25 Jun 2018
**********************************************************************************************************/
json::value CWardWizFirewall::CheckForNetworkPath(SCITER_VALUE svFilePath)
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
		AddLogEntry(L"### Exception in CVibraniumFirewall::CheckForNetworkPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetTargetFilePath
*  Description    :	To get target path of '.lnk' file.
*  Author Name    : Kunal Waghmare
*  Date           : 7 July 2018
**********************************************************************************************************/
json::value CWardWizFirewall::GetTargetFilePath(SCITER_VALUE svFilePath)
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
		AddLogEntry(L"### Exception in CVibraniumFirewall::GetTargetFilePath", 0, 0, true, SECONDLEVEL);
	}
	return json::value(SCITER_STRING(L""));
}

/***************************************************************************************************
*  Function Name  : OnSetRegVal
*  Description    : This function is used to set the registry value on toggler button
*  Author Name    : Amol Jaware
*  Date           :	04 Aug 2018
****************************************************************************************************/
json::value CWardWizFirewall::OnSetRegVal4PortScn(SCITER_VALUE svbToggleState)
{
	try
	{
		bool bFlagValue = svbToggleState.get(false);
		LPCTSTR lpPortScnProt = (LPCTSTR)theApp.m_csRegKeyPath;
		if (!WriteRegistryEntryOfSettingsTab(lpPortScnProt, L"dwPortScanProt", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwPortScanProt in CVibraniumFirewall::OnSetRegVal4PortScn", 0, 0, true, SECONDLEVEL);
		}

		DWORD dwSecondValue = bFlagValue ? 0x01 : 0x00;

		//TODO: GUIID
		if (!SendData2ComService(RELOAD_FIREWALL_SETTINGS, RELOAD_FIREWALL_PORT_SCN_REG, dwSecondValue, L"{20405C0F-3A0A-4C66-9986-E3F1C7B5A197}"))
		{
			AddLogEntry(L"### Failed to SendData2ComService RELOAD_FIREWALL_PORT_SCN_REG CVibraniumFirewall::OnSetRegVal4PortScn", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::OnSetRegVal4PortScn", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetRegVal
*  Description    : This function is used to get the registry value on toggler button
*  Author Name    : Amol Jaware
*  Date           :	04 Aug 2018
****************************************************************************************************/
json::value CWardWizFirewall::GetRegVal4PortScn()
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		DWORD dwPortScnProt = 0x00;
		CITinRegWrapper	 m_objReg;
		bool bFound = true;
		bool bFlagValue = true;
		bool bRegValue = false;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwPortScanProt", dwPortScnProt) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwPortScanProt in CVibraniumFirewall::GetRegVal4PortScn KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			bFound = false;
			bFlagValue = false;
		}
		if (!bFound)
		{
			bFlagValue = true;
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwPortScanProt", bRegValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwPortScanProt in CVibraniumFirewall::GetRegVal4PortScn", 0, 0, true, SECONDLEVEL);
				bFlagValue = false;
			}
		}
		if (bFlagValue)
		{
			CString csRegVal;
			csRegVal.Format(L"%d", dwPortScnProt);
			return json::value((SCITER_STRING)csRegVal);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::GetRegVal4PortScn", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnSetRegVal
*  Description    : This function is used to set the registry value on toggler button
*  Author Name    : Amol Jaware
*  Date           :	04 Aug 2018
****************************************************************************************************/
json::value CWardWizFirewall::OnSetRegVal4StealthMode(SCITER_VALUE svbToggleState)
{
	try
	{
		bool bFlagValue = svbToggleState.get(false);
		LPCTSTR lpStealthMode = (LPCTSTR)theApp.m_csRegKeyPath;
		if (!WriteRegistryEntryOfSettingsTab(lpStealthMode, L"dwStealthMode", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwStealthMode in CVibraniumFirewall::OnSetRegVal4StealthMode", 0, 0, true, SECONDLEVEL);
		}
		DWORD dwSecondValue = bFlagValue ? 0x01 : 0x00;

		//TODO: GUIID
		if (!SendData2ComService(RELOAD_FIREWALL_SETTINGS, RELOAD_FIREWALL_STEALTH_MODE_REG, dwSecondValue, L"{20405C0F-3A0A-4C66-9986-E3F1C7B5A197}"))
		{
			AddLogEntry(L"### Failed to SendData2ComService RELOAD_FIREWALL_STEALTH_MODE_REG CGenXFirewall::OnSetRegVal4StealthMode", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::OnSetRegVal4StealthMode", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetRegVal
*  Description    : This function is used to get the registry value on toggler button
*  Author Name    : Amol Jaware
*  Date           :	04 Aug 2018
****************************************************************************************************/
json::value CWardWizFirewall::GetRegVal4StealthMode()
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		DWORD dwStealthMode = 0x00;
		CITinRegWrapper	 m_objReg;
		bool bFound = true;
		bool bFlagValue = true;
		bool bRegValue = false;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwStealthMode", dwStealthMode) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwStealthMode in CVibraniumFirewall::GetRegVal4StealthMode KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			bFound = false;
			bFlagValue = false;
		}
		if (!bFound)
		{
			bFlagValue = true;
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStealthMode", bRegValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwStealthMode in CVibraniumFirewall::GetRegVal4StealthMode", 0, 0, true, SECONDLEVEL);
				bFlagValue = false;
			}
		}
		if (bFlagValue)
		{
			CString csRegVal;
			csRegVal.Format(L"%d", dwStealthMode);
			return json::value((SCITER_STRING)csRegVal);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::GetRegVal4StealthMode", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnSetRegVal4DefAppBehavior
*  Description    : Set the registry value on dropdown button of default app behavior
*  Author Name    : Kunal Waghmare
*  Date           :	23 Aug 2018
****************************************************************************************************/
json::value CWardWizFirewall::OnSetRegVal4DefAppBehavior(SCITER_VALUE sviDropDownVal)
{
	try
	{
		DWORD dwDefAppBehavior = sviDropDownVal.get(0);
		LPCTSTR lpDefAppBehavior = (LPCTSTR)theApp.m_csRegKeyPath;
		if (!WriteRegistryEntryOfSettingsTab(lpDefAppBehavior, L"dwDefAppBehavior", dwDefAppBehavior))
		{
			AddLogEntry(L"### Error in Setting Registry dwDefAppBehavior in CVibraniumFirewall::OnSetRegVal4DefAppBehavior", 0, 0, true, SECONDLEVEL);
		}

		if (!SendData2ComService(RELOAD_FIREWALL_SETTINGS, RELOAD_FIREWALL_DEF_APP_BEHAVIOR, dwDefAppBehavior, L"{20405C0F-3A0A-4C66-9986-E3F1C7B5A197}"))
		{
			AddLogEntry(L"### Failed to SendData2ComService RELOAD_FIREWALL_DEF_APP_BEHAVIOR CVibraniumFirewall::OnSetRegVal4DefAppBehavior", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::OnSetRegVal4DefAppBehavior", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetRegVal4DefAppBehavior
*  Description    : Get the registry value on dropdown button of default app behavior
*  Author Name    : Kunal Waghmare
*  Date           :	23 Aug 2018
****************************************************************************************************/
json::value CWardWizFirewall::GetRegVal4DefAppBehavior()
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		DWORD dwDefAppBehavior = 0x00;
		CITinRegWrapper	 m_objReg;
		bool bFound = true;
		CString csRegVal;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwDefAppBehavior", dwDefAppBehavior) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwDefAppBehavior in CVibraniumFirewall::GetRegVal4DefAppBehavior KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			bFound = false;
			dwDefAppBehavior = 0x00;
		}
		if (!bFound)
		{
			dwDefAppBehavior = 0x00;
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDefAppBehavior", dwDefAppBehavior))
			{
				AddLogEntry(L"### Error in Setting Registry dwDefAppBehavior in CVibraniumFirewall::GetRegVal4DefAppBehavior", 0, 0, true, SECONDLEVEL);
				dwDefAppBehavior = 0x00;
			}
		}
		csRegVal.Format(L"%d", dwDefAppBehavior);
		return json::value((SCITER_STRING)csRegVal);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::GetRegVal4DefAppBehavior", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnClickSetPage
*  Description    : This function is used to set the setting page name.
*  Author Name    : Amol Jaware
*  Date           :	18 Dec 2018
****************************************************************************************************/
json::value CWardWizFirewall::OnClickSetPage()
{
	try
	{
		theApp.m_csPageName = L"#FIREWALL";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::OnClickSetPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnClickSetOtherPage
*  Description    : This function is used to set the none page name.
*  Author Name    : Amol Jaware
*  Date           :	18 Dec 2018
****************************************************************************************************/
json::value CWardWizFirewall::OnClickSetOtherPage()
{
	try
	{
		theApp.m_csPageName = L"#NONE";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewall::OnClickSetOtherPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}