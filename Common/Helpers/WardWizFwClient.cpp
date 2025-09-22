#include "stdafx.h"
#include "WardWizFwClient.h"
#include "WardWizDatabaseInterface.h"

/***********************************************************************************************
Function Name  : CWardWizFwClient
Description    : Const'r
Author Name    : Ramkrushna Shelke
Date           : 25 Jul 2018
***********************************************************************************************/
CWardWizFwClient::CWardWizFwClient():
m_hModuleFwDLL(NULL)
, m_lpFwInitialize(NULL)
, m_lpFwUnInitialize(NULL)
, m_lpCreateRules(NULL)
, m_lpEnableDisFWPortScnProt(NULL)
, m_lpEnableDisFWStealthMode(NULL)
, m_lpEnableDisFirewallSettinsTab(NULL)
, m_lpClearAllRules(NULL)
, m_lpSetDefaultAppBehaviour(NULL)
, m_csDatabaseFilePath(".\\VBALLREPORTS.DB")
{
}

/***********************************************************************************************
Function Name  : ~CWardWizFwClient
Description    : Dest'r
Author Name    : Ramkrushna Shelke
Date           : 25 Jul 2018
***********************************************************************************************/
CWardWizFwClient::~CWardWizFwClient()
{
	//Unload here
	if (m_lpFwUnInitialize != NULL)
	{
		m_lpFwUnInitialize();
	}

	if (m_hModuleFwDLL != NULL)
	{
		FreeLibrary(m_hModuleFwDLL);
		m_hModuleFwDLL = NULL;
	}
}

/***********************************************************************************************
Function Name  : Initialize
Description    : Function which initialize required library and variables.
Author Name    : Ramkrushna Shelke
Date           : 25 Jul 2018
***********************************************************************************************/
bool CWardWizFwClient::Initialize()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleFwDLL == NULL)
			LoadRequiredModules();

		if (m_lpFwInitialize)
		{
			bReturn = m_lpFwInitialize();
		}

		CreateTable4Firewall();
		ReLoadFirewallRuleList();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::Initialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : Initialize
Description    : Function which initialize required library and variables.
Author Name    : Ramkrushna Shelke
Date           : 25 Jul 2018
***********************************************************************************************/
bool CWardWizFwClient::UnInitialize()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleFwDLL == NULL)
			LoadRequiredModules();

		if (m_lpFwUnInitialize)
		{
			bReturn = m_lpFwUnInitialize();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::UnInitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadRequiredModules
Description    : Function Loads required modules
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CWardWizFwClient::LoadRequiredModules()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleFwDLL)
		{
			return true;
		}

		if (!m_hModuleFwDLL)
		{
			m_hModuleFwDLL = LoadLibrary(L"VBFW.DLL");
			if (!m_hModuleFwDLL)
			{
				AddLogEntryEx(SECONDLEVEL, L"### Error in Loading VBEMAILSCN.DLL, LastError:[%d]", GetLastError());
				return false;
			}

			m_lpFwInitialize = (WRDWIZFWINITIALIZE)GetProcAddress(m_hModuleFwDLL, "WrdWizFwInitialize");
			if (!m_lpFwInitialize)
			{
				AddLogEntry(L"### CWardwizFwClient:LoadRequiredModules Error in GetProcAddress::VibraniumFwInitialize", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleFwDLL);
				m_lpFwInitialize = NULL;
				m_hModuleFwDLL = NULL;
				return false;
			}

			m_lpFwUnInitialize = (WRDWIZFWUNINITIALIZE)GetProcAddress(m_hModuleFwDLL, "WrdWizFwUnInitialize");
			if (!m_lpFwUnInitialize)
			{
				AddLogEntry(L"### CWardwizFwClient:LoadRequiredModules Error in GetProcAddress::VibraniumFwUnInitialize", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleFwDLL);
				m_lpFwUnInitialize = NULL;
				m_hModuleFwDLL = NULL;
				return false;
			}

			m_lpCreateRules = (CREATEFWRULES)GetProcAddress(m_hModuleFwDLL, "CreateFwRules");
			if (!m_lpCreateRules)
			{
				AddLogEntry(L"### VibraniumFwClient:LoadRequiredModules Error in GetProcAddress::CreateFwRules", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleFwDLL);
				m_lpCreateRules = NULL;
				m_hModuleFwDLL = NULL;
				return false;
			}

			m_lpEnableDisFirewallSettinsTab = (ENABLEDISFIREWALLSETTINGSTAB)GetProcAddress(m_hModuleFwDLL, "EnableDisFirewallSettingsTab");
			if (!m_lpEnableDisFirewallSettinsTab)
			{
				AddLogEntry(L"### CWardwizFwClient:LoadRequiredModules Error in GetProcAddress::CreateFwRules", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleFwDLL);
				m_lpEnableDisFirewallSettinsTab = NULL;
				m_hModuleFwDLL = NULL;
				return false;
			}
			m_lpEnableDisFWPortScnProt = (ENABLEDISFWPORTSCNPROT)GetProcAddress(m_hModuleFwDLL, "EnableDisFWPortScnProt");
			if (!m_lpEnableDisFWPortScnProt)
			{
				AddLogEntry(L"### CWardwizFwClient:LoadRequiredModules Error in GetProcAddress::CreateFwRules", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleFwDLL);
				m_lpEnableDisFWPortScnProt = NULL;
				m_hModuleFwDLL = NULL;
				return false;
			}

			m_lpEnableDisFWStealthMode = (ENABLEDISFWSTEALTHMODE)GetProcAddress(m_hModuleFwDLL, "EnableDisFWStealthMode");
			if (!m_lpEnableDisFWStealthMode)
			{
				AddLogEntry(L"### CWardwizFwClient:LoadRequiredModules Error in GetProcAddress::CreateFwRules", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleFwDLL);
				m_lpEnableDisFWStealthMode = NULL;
				m_hModuleFwDLL = NULL;
				return false;
			}

			m_lpClearAllRules = (CLEARALLRULES)GetProcAddress(m_hModuleFwDLL, "ClearAllRules");
			if (!m_lpClearAllRules)
			{
				AddLogEntry(L"### CWardwizFwClient:LoadRequiredModules Error in GetProcAddress::CreateFwRules", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleFwDLL);
				m_lpClearAllRules = NULL;
				m_hModuleFwDLL = NULL;
				return false;
			}

			m_lpSetDefaultAppBehaviour = (SETDEFAULTAPPBEHAVIOUR)GetProcAddress(m_hModuleFwDLL, "SetDefaultAppBehaviour");
			if (!m_lpSetDefaultAppBehaviour)
			{
				AddLogEntry(L"### CWardwizFwClient:LoadRequiredModules Error in GetProcAddress::CreateFwRules", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleFwDLL);
				m_lpSetDefaultAppBehaviour = NULL;
				m_hModuleFwDLL = NULL;
				return false;
			}
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::LoadRequiredModules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CreateRule
*  Description    : Function to create rule
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
DWORD CWardWizFwClient::CreateRule(LPSTR lpProcessPath, RULE_ACTION dwAction, TRAFFIC_DIRECTION dwDirection, bool bCheckMD5, BYTE dwCheckProtocol,
	BYTE Protocol, BYTE bCheckLocalIP, LPTSTR lpLocalIP, BYTE bCheckRemoteIP, LPTSTR csRemoteIP, BYTE bCheckLocalPort,
	LPTSTR lpLocalPortRange, BYTE bCheckRemotePort, LPTSTR lpRemotePortRange)
{
	DWORD dwRet = -1;
	__try
	{
		if (m_hModuleFwDLL == NULL)
			LoadRequiredModules();

		if (m_lpCreateRules)
		{
			dwRet = m_lpCreateRules(lpProcessPath, dwAction, dwDirection, bCheckMD5, dwCheckProtocol,
				Protocol, bCheckLocalIP, lpLocalIP, bCheckRemoteIP, csRemoteIP, bCheckLocalPort,
				lpLocalPortRange, bCheckRemotePort, lpRemotePortRange);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::CreateRule", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : LoadFirewallRuleList
*  Description    : Function to load firewall rule
*  Author Name    : Kunal Waghmare
*  Date			  :	4 Aug 2018
****************************************************************************************************/
bool CWardWizFwClient::LoadFirewallRuleList()
{
	try
	{
		TCHAR	szPath[512] = { 0 };
		DWORD	dwSize = sizeof(szPath);
		CITinRegWrapper	objReg;
		CWardWizSQLiteDatabase dbSQlite;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"AppFolder", szPath, dwSize);
		CString csWardWizModulePath = szPath;
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBFIREWALL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("SELECT PROGRAMPATH, ACCESS, DIRECTION, PROTOCOL, LOCAL_IP, LOCAL_IP_DATA, REMOTE_IP, REMOTE_IP_DATA, LOCAL_PORT, LOCAL_PORT_DATA, REMOTE_PORT, REMOTE_PORT_DATA FROM WWIZ_FIREWALL_PACKET_RULE_CONFIG WHERE SETRULES = '1' AND NOT PROGRAMPATH = 'IDS_ALL_APP';");
		DWORD dwRows = qResult.GetNumRows();
		if (dwRows != 0)
		{
			LPSTR lpszPath = NULL;
			RULE_ACTION dwAction;
			TRAFFIC_DIRECTION dwDirection;
			BYTE byteProtocol;
			BYTE byteLocalIP;
			BYTE byteRemoteIP;
			BYTE byteLocalPort;
			BYTE byteRemotePort;
			int iCount = 1;
			LPSTR lpData = NULL;
			int len = 0;
			for (DWORD dwIndex = 0x00; dwIndex < dwRows; dwIndex++)
			{
				CString csLocalIP;
				CString csRemoteIP;
				CString csLocalPort;
				CString csRemotePort;
				qResult.SetRow(dwIndex);
				if (qResult.GetFieldIsNull(0))
				{
					continue;
				}
				
				for (iCount = 0; iCount < 12; iCount++)
				{					
					lpData = NULL;		
					lpData = (LPSTR)qResult.GetFieldValue(iCount);
					switch (iCount)
					{
					case 0:lpszPath = lpData;
						break;
					case 1:dwAction = (RULE_ACTION)atoi(lpData);
						break;
					case 2:dwDirection = (TRAFFIC_DIRECTION)atoi(lpData);
						break;
					case 3: if(strlen(lpData) == 1)
							{
								byteProtocol = (BYTE)lpData[0];
							}
						break;
					case 4:if (strlen(lpData) == 1)
							{
							   byteLocalIP = (BYTE)lpData[0];
							}
						break;
					case 5: csLocalIP = lpData;
						break;
					case 6:if (strlen(lpData) == 1)
							{
							   byteRemoteIP = (BYTE)lpData[0];
							}
						break;
					case 7: csRemoteIP = lpData;
						break;
					case 8:if (strlen(lpData) == 1)
							{
							   byteLocalPort = (BYTE)lpData[0];
							}
						break;
					case 9: csLocalPort = lpData;
						break;
					case 10:if (strlen(lpData) == 1)
							{
								byteRemotePort = (BYTE)lpData[0];
							}
						break;
					case 11:csRemotePort = lpData;
						break;
					}
				}

				//check here to protocol needs to check
				BYTE bycheckProtocol = 0x30;
				if (byteProtocol != 0x30)
				{
					bycheckProtocol = 0x31;
				}

				CreateRule(lpszPath, dwAction, dwDirection, false, byteProtocol, byteProtocol, byteLocalIP, (LPTSTR)csLocalIP.GetString(), byteRemoteIP, (LPTSTR)csRemoteIP.GetString(), byteLocalPort, (LPTSTR)csLocalPort.GetString(), byteRemotePort, (LPTSTR)csRemotePort.GetString());
			}
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::LoadFirewallRuleList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReLoadFirewallRuleList
*  Description    : Function to reload firewall rule
*  Author Name    : Kunal Waghmare
*  Date			  :	4 Aug 2018
****************************************************************************************************/
bool CWardWizFwClient::ReLoadFirewallRuleList()
{
	try
	{
		ClearAllRules();
		LoadFirewallAllApplicationRules();
		LoadFirewallRuleList();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::ReLoadFirewallRuleList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReLoadFWControlSettings4PortScanProt
*  Description    : Function to reload firewall rule
*  Author Name    : Amol Jaware
*  Date			  :	06 Aug 2018
****************************************************************************************************/
void CWardWizFwClient::ReLoadFWControlSettings4PortScanProt(DWORD dwRegValue, LPTSTR szFirstParam)
{
	try
	{
		if (m_hModuleFwDLL == NULL)
			LoadRequiredModules();

		if (m_lpEnableDisFWPortScnProt != NULL)
		{
			m_lpEnableDisFWPortScnProt(dwRegValue, szFirstParam);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::ReLoadFWControlSettings4PortScanProt", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ReLoadFirewallRuleList
*  Description    : Function to reload firewall rule
*  Author Name    : Amol Jaware
*  Date			  :	06 Aug 2018
****************************************************************************************************/
void CWardWizFwClient::ReLoadFWControlSettings4StealthMode(DWORD dwRegValue, LPTSTR szFirstParam)
{
	try
	{
		if (m_hModuleFwDLL == NULL)
			LoadRequiredModules();

		if (m_lpEnableDisFWStealthMode)
			m_lpEnableDisFWStealthMode(dwRegValue, szFirstParam);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::ReLoadFWControlSettings4StealthMode", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ReLoadFWControlSettingsTab
*  Description    : Function to reload firewall settings tab.
*  Author Name    : Amol Jaware
*  Date			  :	07 Aug 2018
****************************************************************************************************/
void CWardWizFwClient::ReLoadFWControlSettingsTab(DWORD dwRegValue)
{
	try
	{
		if (m_hModuleFwDLL == NULL)
			LoadRequiredModules();

		if (m_lpEnableDisFirewallSettinsTab)
			m_lpEnableDisFirewallSettinsTab(dwRegValue);

		if (dwRegValue == 0x01)//means ON
		{
			ReLoadFirewallRuleList();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::ReLoadFWControlSettingsTab", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateTable4Firewall
*  Description    : Function to create table for firewall.
*  Author Name    : Amol Jaware
*  Date			  :	12 Aug 2018
****************************************************************************************************/
void CWardWizFwClient::CreateTable4Firewall()
{
	try
	{
		CWardWizSQLiteDatabase ObjWardWizSQLiteDatabase(m_csDatabaseFilePath.GetBuffer());
		ObjWardWizSQLiteDatabase.Open();
		
		if (!ObjWardWizSQLiteDatabase.TableExists("Wardwiz_FirewallDetails"))
		{
			ObjWardWizSQLiteDatabase.ExecDML("CREATE TABLE [Wardwiz_FirewallDetails] (\
													  [db_FirewallID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
													  [db_FirewallDate] DATE  NULL,\
													  [db_FirewallTime]  TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
													  [db_FirewallEnabled]  BOOLEAN NULL,\
													  [db_FW_LocalIP]  NVARCHAR(20) NULL,\
													  [db_Username] NVARCHAR(512)  NULL,\
													  [db_Reserved1] NVARCHAR(512)  NULL,\
													  [db_Reserved2] NVARCHAR(128)  NULL,\
													  [db_Reserved3] NVARCHAR(128)  NULL\
													 )");
		}

		ObjWardWizSQLiteDatabase.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::CreateTable4Firewall", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : ClearAllRules
*  Description    : Function to clear all rules.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	23 Aug 2018
****************************************************************************************************/
bool CWardWizFwClient::ClearAllRules()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleFwDLL == NULL)
			LoadRequiredModules();

		if (m_lpClearAllRules)
		{
			bReturn = m_lpClearAllRules();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::ClearAllRules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SetDefaultAppBehaviour
*  Description    : Function to set default application behaviour in firewall.
*  Author Name    : Ram Shelke
*  Date			  :	23 Aug 2018
****************************************************************************************************/
bool CWardWizFwClient::SetDefaultAppBehaviour(DWORD dwValue)
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleFwDLL == NULL)
			LoadRequiredModules();

		if (m_lpSetDefaultAppBehaviour)
		{
			bReturn = m_lpSetDefaultAppBehaviour(dwValue);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SetDefaultAppBehaviour", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadFirewallAllApplicationRules
*  Description    : Function to load firewall rule for all application.
*  Author Name    : Ramkrushna Shelke
*  Date			  :	4 Aug 2018
****************************************************************************************************/
bool CWardWizFwClient::LoadFirewallAllApplicationRules()
{
	try
	{
		TCHAR	szPath[512] = { 0 };
		DWORD	dwSize = sizeof(szPath);
		CITinRegWrapper	objReg;
		CWardWizSQLiteDatabase dbSQlite;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"AppFolder", szPath, dwSize);
		CString csWardWizModulePath = szPath;
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBFIREWALL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("SELECT PROGRAMPATH, ACCESS, DIRECTION, PROTOCOL, LOCAL_IP, LOCAL_IP_DATA, REMOTE_IP, REMOTE_IP_DATA, LOCAL_PORT, LOCAL_PORT_DATA, REMOTE_PORT, REMOTE_PORT_DATA FROM WWIZ_FIREWALL_PACKET_RULE_CONFIG WHERE SETRULES = '1' AND PROGRAMPATH = 'IDS_ALL_APP';");
		DWORD dwRows = qResult.GetNumRows();
		if (dwRows != 0)
		{
			LPSTR lpszPath = NULL;
			RULE_ACTION dwAction;
			TRAFFIC_DIRECTION dwDirection;
			BYTE byteProtocol;
			BYTE byteLocalIP;
			BYTE byteRemoteIP;
			BYTE byteLocalPort;
			BYTE byteRemotePort;
			int iCount = 1;
			LPSTR lpData = NULL;
			int len = 0;
			for (DWORD dwIndex = 0x00; dwIndex < dwRows; dwIndex++)
			{
				CString csLocalIP;
				CString csRemoteIP;
				CString csLocalPort;
				CString csRemotePort;
				qResult.SetRow(dwIndex);
				if (qResult.GetFieldIsNull(0))
				{
					continue;
				}

				for (iCount = 0; iCount < 12; iCount++)
				{	
					lpData = NULL;
					lpData = (LPSTR)qResult.GetFieldValue(iCount);
					switch (iCount)
					{
					case 1:dwAction = (RULE_ACTION)atoi(lpData);
						break;
					case 2:dwDirection = (TRAFFIC_DIRECTION)atoi(lpData);
						break;
					case 3: if (strlen(lpData) == 1)
					{
						byteProtocol = (BYTE)lpData[0];
					}
							break;
					case 4:if (strlen(lpData) == 1)
					{
						byteLocalIP = (BYTE)lpData[0];
					}
						   break;
					case 5:csLocalIP = lpData;
						break;
					case 6:if (strlen(lpData) == 1)
					{
						byteRemoteIP = (BYTE)lpData[0];
					}
						   break;
					case 7:csRemoteIP = lpData;
						break;
					case 8:if (strlen(lpData) == 1)
					{
						byteLocalPort = (BYTE)lpData[0];
					}
						   break;
					case 9:csLocalPort = lpData;
						break;
					case 10:if (strlen(lpData) == 1)
					{
						byteRemotePort = (BYTE)lpData[0];
					}
							break;
					case 11:csRemotePort = lpData;
						break;
					}
				}

				//check here to protocol needs to check
				BYTE bycheckProtocol = 0x30;
				if (byteProtocol != 0x30)
				{
					bycheckProtocol = 0x31;
				}
			
				CreateRule(lpszPath, dwAction, dwDirection, false, byteProtocol, byteProtocol, byteLocalIP, (LPTSTR)csLocalIP.GetString(), byteRemoteIP, (LPTSTR)csRemoteIP.GetString(), byteLocalPort, (LPTSTR)csLocalPort.GetString(), byteRemotePort, (LPTSTR)csRemotePort.GetString());
			}
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFwClient::LoadFirewallRuleList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}