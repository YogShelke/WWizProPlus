// WardWizFirewall.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "WardWizFirewall.h"
#include "WardWizDatabaseInterface.h"
#include "ISpyCommunicator.h"
#include "iTinRegWrapper.h"
#include <netlistmgr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT _declspec(dllimport)
#define DLLEXPORT _declspec(dllexport)

#define FIREWALL_XML_FILE	_T("FIREWALL.XML");

#pragma comment(lib, "IPHLPAPI.lib")

BOOLEAN			gUnattendedMode				= FALSE;
BOOLEAN			gDefaultActionInAllow		= TRUE;
BOOLEAN			gDefaultActionOutAllow		= TRUE;

QWORD			gLogLevelForUnattendedMode	= 0;
QWORD			gDefaultInFlags				= 0;
QWORD			gDefaultOutFlags			= 0;

// List of the network adapters
IGNIS_ADAPTER*			gAdapters = NULL;
DWORD					gNumberOfAdapters = 0;
CRITICAL_SECTION		gAdapterListClientLock;
CRITICAL_SECTION		gXmlLock;

TiXmlElement *			gXmlRoot = NULL;

BYTE gOemClientId[16] =
{ 0xC0, 0xEF, 0xC5, 0x24,
0x78, 0x1A, 0x33, 0xAF,
0x22, 0x7F, 0x7C, 0xC4,
0xCF, 0x46, 0x2E, 0x1B
};

BEGIN_MESSAGE_MAP(CWardWizFirewallApp, CWinApp)
END_MESSAGE_MAP()

// CWardWizFirewallApp construction
CWardWizFirewallApp::CWardWizFirewallApp()
{
}

// The one and only CWardWizFirewallApp object
CWardWizFirewallApp theApp;

// CWardWizFirewallApp initialization
BOOL CWardWizFirewallApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

/***************************************************************************************************
*  Function Name  : WrdWizFwInitialize
*  Description    : Function to Initilize library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool WrdWizFwInitialize()
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.WrdWizFwInitialize() ? false : true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in VibraniumFwInitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ClearAllRules
*  Description    : Function to clear all rules.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	23 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool ClearAllRules()
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.ClearAllRules();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ClearAllRules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SetDefaultAppBehaviour
*  Description    : Function to set default application behaviour
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	23 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool SetDefaultAppBehaviour(DWORD dwValue)
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.SetDefaultAppBehaviour(dwValue);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SetDefaultAppBehaviour", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : SetDefaultAppBehaviour
*  Description    : Function to set default application behaviour
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizFirewallApp::SetDefaultAppBehaviour(DWORD dwValue)
{
	__try
	{
		PTSTATUS status = STATUS_SUCCESS;
		IGNIS_SETTINGS newSettings;
		IGNIS_SETTINGS currentSettings;
		DWORD noOfSettingsToUpdate;

		RtlZeroMemory(&newSettings, sizeof(IGNIS_SETTINGS));
		RtlZeroMemory(&currentSettings, sizeof(IGNIS_SETTINGS));
		noOfSettingsToUpdate = 0;

		status = PubIgnisGetCurrentSettings(&currentSettings);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"### ERROR : Unable to retrieve current settings in SetDefaultAppBehaviour, status: 0x%X", status);
			return false;
		}
		// Copy the current settings in the new settings
		RtlCopyMemory(&newSettings, &currentSettings, sizeof(IGNIS_SETTINGS));

		newSettings.DefaultFirewallAction = (TRAFFIC_ACTION)dwValue;

		status = PubIgnisUpdateSettings(&newSettings);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"### ERROR: PubIgnisUpdateSettings failed in SetDefaultAppBehaviour, status 0x%08x", status);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::SetDefaultAppBehaviour", 0, 0, true, SECONDLEVEL);
	}
	return 0x01;
}

/***************************************************************************************************
*  Function Name  : CreateFwRules
*  Description    : Function to create firewall rules
*  Author Name    : Ram
*  SR_NO		  : 
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT DWORD CreateFwRules(LPSTR lpProcessPath, RULE_ACTION dwAction, TRAFFIC_DIRECTION dwDirection, bool bCheckMD5, BYTE dwCheckProtocol,
	BYTE Protocol, BYTE bCheckLocalIP, LPTSTR lpLocalIP, BYTE bCheckRemoteIP, LPTSTR csRemoteIP, BYTE bCheckLocalPort,
	LPTSTR lpLocalPortRange, BYTE bCheckRemotePort, LPTSTR lpRemotePortRange)
{
	DWORD dwRuleID = -1;
	__try
	{
		dwRuleID = theApp.CreateRule(lpProcessPath, dwAction, dwDirection, bCheckMD5, dwCheckProtocol,
			Protocol, bCheckLocalIP, lpLocalIP, bCheckRemoteIP, csRemoteIP, bCheckLocalPort,
			lpLocalPortRange, bCheckRemotePort, lpRemotePortRange);

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CreateFwRules", 0, 0, true, SECONDLEVEL);
	}
	return dwRuleID;
}

/***************************************************************************************************
*  Function Name  : WrdWizFwUnInitialize
*  Description    : Function to Initilize library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool WrdWizFwUnInitialize()
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.WrdWizFwUnInitialize() ? false : true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in VibraniumFwUnInitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : UnInitializeIgnis
*  Description    : Function to un Initilize library
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
DWORD CWardWizFirewallApp::WrdWizFwUnInitialize()
{
	__try
	{
		PTSTATUS status = STATUS_UNSUCCESSFUL;
		if (NULL != gAdapters)
		{
			status = CleanupAdaptersInfoClient(gAdapters, gNumberOfAdapters);
			if (!PT_SUCCESS(status))
			{
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: CleanupAdaptersInfoClient failed with status: 0x%X", status);
				return 0x01;
			}
		}

		gAdapters = NULL;

		// uninit ignis
		status = PubUninitializeIgnis();
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: PubUninitializeIgnis failed with status 0x%x ", status);
			return 0x02;
		}

		// unload the ignis api
		FreeLibraryFunctions();

		//// save the data in the xml 
		//if (m_xmlDoc.SaveFile() == FALSE)
		//	AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: Could not save xml; Error description: %s.", m_xmlDoc.ErrorDesc());

		// delete the critical sections
		DeleteCriticalSection(&gAdapterListClientLock);
		DeleteCriticalSection(&gXmlLock);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::UnInitializeIgnis", 0, 0, true, SECONDLEVEL);
	}
	return 0x00;
}

/***************************************************************************************************
*  Function Name  : InitializeIgnis
*  Description    : Function to Initilize library
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
DWORD CWardWizFirewallApp::WrdWizFwInitialize()
{
	__try
	{
		CreateFirewallDB();
		return WrdWizFwInitializeSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::InitializeIgnis", 0, 0, true, SECONDLEVEL);
	}
	return 0x01;
}

/***************************************************************************************************
*  Function Name  : InitializeIgnisSEH
*  Description    : Function to Initilize library using structured exception handling
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
DWORD CWardWizFirewallApp::WrdWizFwInitializeSEH()
{
	try
	{
		PTSTATUS status = STATUS_UNSUCCESSFUL;
		IGNIS_INIT ignisInit;
		DWORD result;

		TCHAR module_path[_MAX_PATH] = { 0 };
		// Get current path  
		if (0 == GetModuleFileName(NULL, module_path, _MAX_PATH))
		{
			return 0x01;
		}

		if (!PathRemoveFileSpec(module_path))
		{
			return 0x01;
		}

		CString csPath(module_path);
		csPath += _T("\\");
		csPath += FIREWALL_XML_FILE;

		CStringA filepath = CStringA(csPath);

		//// Load the xml file into an TiXmlDocument object
		//if (m_xmlDoc.LoadFile(filepath) == FALSE)
		//{
		//	AddLogEntryEx(SECONDLEVEL, L"### [ERROR] Corrupt xml file; Error description: %s", m_xmlDoc.ErrorDesc());
		//	return 0x02;
		//}

		//// Retrieve the root node of the xml
		//gXmlRoot = m_xmlDoc.RootElement();
		//if (gXmlRoot == NULL)
		//{
		//	AddLogEntryEx(SECONDLEVEL, L"### [ERROR] Could not retrieve the root of the xml");
		//	return 0x03;
		//}

		// Load ignis.dll and the Ignis API functions
		result = ResolveLibraryFunctions();
		if (ERROR_SUCCESS != result)
		{
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR] ResolveLibraryFunctions failed with result: %d", result);
			return ERROR_DLL_INIT_FAILED;
		}

		// Get the firewall settings from xml and populate the IGNIS_INIT object
		RtlSecureZeroMemory(&ignisInit, sizeof(IGNIS_INIT));
		ignisInit.LibraryVersionUsed = LIBRARY_INTERFACE_VERSION;

		CString csRegKeyPath;
		csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		//Get here registry setting for Active scanning.
		DWORD dwFirewallEnableState = 0x00;
		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwFirewallEnableState", dwFirewallEnableState) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwFirewallEnableState in CVibraniumFirewallApp::VibraniumFwInitializeSEH KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}
		ignisInit.StartSettings.Enabled = dwFirewallEnableState ? true : false;

		// 0x00	-	Automatic
		// 0x01	-	Allow
		// 0x02	-	Block
		DWORD dwDefAppBehavior = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwDefAppBehavior", dwDefAppBehavior) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwDefAppBehavior in CVibraniumFirewallApp::VibraniumFwInitializeSEH KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		/*if (dwDefAppBehavior == 0x00)
		{
			dwDefAppBehavior = 0x01;//automatic to allow;
		}*/

		ignisInit.StartSettings.DefaultFirewallAction = (TRAFFIC_ACTION)dwDefAppBehavior;

		// BlockPortScans
		DWORD dwPortScanProt = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwPortScanProt", dwPortScanProt) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwPortScanProt in CVibraniumFirewallApp::VibraniumFwInitializeSEH KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		ignisInit.StartSettings.BlockPortScans = dwPortScanProt ? true : false;
		
		memcpy(ignisInit.OemClientId, gOemClientId, sizeof(gOemClientId));

		status = PubInitializeIgnis(&ignisInit);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR] InitializeIgnis failed, status = %x", status);
			return 0x05;
		}

		// Initialize the critical sections
		InitializeCriticalSectionAndSpinCount(&gAdapterListClientLock, 0x1000);
		InitializeCriticalSectionAndSpinCount(&gXmlLock, 0x1000);

		//// Add all the rules present in the xml
		//EnterCriticalSection(&gXmlLock);
		//status = AddRulesFromXml();
		//LeaveCriticalSection(&gXmlLock);
		//if (!PT_SUCCESS(status))
		//{
		//	AddLogEntryEx(SECONDLEVEL, L"### [ERROR] AddRulesFromXml failed, status = %x", status);
		//	return 0x06;
		//}

		// Enable all the callbacks
		status = SetupCallbacks();
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR] SetupCallbacks failed, status = %x", status);
			return 0x07;
		}

		//SetDefaultAppBehaviour(dwDefAppBehavior);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::InitializeIgnisSEH", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SaveFileXML
*  Description    : Function to which saves the added rules into xml
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizFirewallApp::SaveFileXML()
{
	try
	{
		// Add all the rules present in the xml
		EnterCriticalSection(&gXmlLock);
		// save the data in the xml 
		if (m_xmlDoc.SaveFile() == FALSE)
		{
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: Could not save xml; Error description: %s.", m_xmlDoc.ErrorDesc());
			return false;
		}
		LeaveCriticalSection(&gXmlLock);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::SaveFileXML", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : CreateRule
*  Description    : Function to create rule
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
DWORD CWardWizFirewallApp::CreateRule(LPSTR lpProcessPath, RULE_ACTION dwAction, TRAFFIC_DIRECTION dwDirection, bool bCheckMD5, BYTE dwCheckProtocol,
	BYTE Protocol, BYTE bCheckLocalIP, LPTSTR lpLocalIP, BYTE bCheckRemoteIP, LPTSTR csRemoteIP, BYTE bCheckLocalPort,
	LPTSTR lpLocalPortRange, BYTE bCheckRemotePort, LPTSTR lpRemotePortRange)
{
	PTSTATUS status = STATUS_SUCCESS;
	IGNIS_RULE rule = { 0 };
	ZeroMemory(&rule, sizeof(IGNIS_RULE));

	try
	{
		if (lpProcessPath != NULL)
		{
			PPROCESS_INFO processInfo = (PPROCESS_INFO)malloc(sizeof(PROCESS_INFO) + (strlen(lpProcessPath) + 1) * sizeof(WCHAR));
			if (NULL == processInfo)
			{
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR] allocating memory for process %s", CString(lpProcessPath));
				status = STATUS_INSUFFICIENT_RESOURCES;
				return false;
			}

			memset((void*)processInfo, 0, sizeof(PROCESS_INFO) + (strlen(lpProcessPath) + 1) * sizeof(WCHAR));

			size_t i;
			for (i = 0; i < strlen(lpProcessPath); i++)
			{
				processInfo->ProcessPath.Buffer[i] = (WCHAR)lpProcessPath[i];
			}

			processInfo->ProcessPath.Buffer[i] = L'\0';
			processInfo->ProcessPath.Length = (DWORD)((wcslen(processInfo->ProcessPath.Buffer) + 1) * sizeof(WCHAR));

			rule.Process = processInfo;
			rule.Flags |= RULE_CHECK_PROCESS;

			if (bCheckMD5)
			{
				DWORD md5Size = SIZEOF_MD5;
				// we need to actually calculate the MD5 value
				status = PubIgnisComputeMD5(&processInfo->ProcessPath,
					processInfo->Md5Value,
					&md5Size);
				if (!PT_SUCCESS(status))
				{
					AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: PubIgnisComputeMD5 failed with status: 0x%X", status);
					goto CLEANUP;
				}

				rule.Flags |= RULE_CHECK_MD5;
				rule.Process->Md5Valid = TRUE;
			}
		}

		rule.Category = RULE_CATEGORY_USER;
		rule.Action = dwAction;
		rule.Direction = dwDirection;
		
		if (dwCheckProtocol == 0x31)
		{
			rule.Flags |= RULE_CHECK_PROTOCOL;
			rule.Protocol = (BYTE)Protocol;
		}
		
		rule.IpVersion = ipVersion4;
		
		if (bCheckLocalIP == 0x31)
		{
			if (lpLocalIP != NULL)
			{
				CString csLocalIP(lpLocalIP);
				DWORD b3, b2, b1, b0;
				if (swscanf_s(csLocalIP, L"%d.%d.%d.%d", &b3, &b2, &b1, &b0) == 4)
				{
					rule.LocalNetwork.Ipv4.Ipv4ValueBytes[3] = (BYTE)b3;
					rule.LocalNetwork.Ipv4.Ipv4ValueBytes[2] = (BYTE)b2;
					rule.LocalNetwork.Ipv4.Ipv4ValueBytes[1] = (BYTE)b1;
					rule.LocalNetwork.Ipv4.Ipv4ValueBytes[0] = (BYTE)b0;
					rule.LocalNetwork.Ipv4.Mask = 0xffffffff;
					rule.Flags |= RULE_CHECK_LOCAL_NETWORK;
				}
			}
		}

		if (bCheckLocalPort != 0x30 && bCheckLocalPort <= 0x32)
		{
			if (lpLocalPortRange != NULL)
			{
				CString csLocalPortRange(lpLocalPortRange);
				if ((!csLocalPortRange.IsEmpty()))
				{
					rule.Flags |= RULE_CHECK_REM_PORT;
					rule.LocalPorts.Min = rule.LocalPorts.Max = _wtoi(csLocalPortRange);
				}
			}
		}

		if (bCheckRemoteIP == 0x31)
		{
			DWORD b3, b2, b1, b0;
			if (swscanf_s(csRemoteIP, L"%d.%d.%d.%d", &b3, &b2, &b1, &b0) == 4)
			{
				rule.RemoteNetworkCount = 1;
				rule.RemoteNetworks = (PIGNIS_IP)malloc(sizeof(IGNIS_IP));
				if (NULL == rule.RemoteNetworks)
				{
					AddLogEntryEx(SECONDLEVEL, L"### [ERROR] allocating memory for remote network %s", CString(lpProcessPath));
					goto CLEANUP;
				}
				rule.RemoteNetworks[0].Ipv4.Ipv4ValueBytes[3] = (BYTE)b3;
				rule.RemoteNetworks[0].Ipv4.Ipv4ValueBytes[2] = (BYTE)b2;
				rule.RemoteNetworks[0].Ipv4.Ipv4ValueBytes[1] = (BYTE)b1;
				rule.RemoteNetworks[0].Ipv4.Ipv4ValueBytes[0] = (BYTE)b0;
				rule.RemoteNetworks[0].Ipv4.Mask = 0xffffffff;
				rule.Flags |= RULE_CHECK_REM_NETWORK;
			}
		}

		if (bCheckRemotePort != 0x30 && bCheckRemotePort <= 0x32)
		{
			if (lpRemotePortRange != NULL)
			{
				CString csRemotePortRange(lpRemotePortRange);
				if ((!csRemotePortRange.IsEmpty()))
				{
					rule.Flags |= RULE_CHECK_REM_PORT;
					rule.RemotePortRanges = (PPORT_RANGE)malloc(sizeof(PORT_RANGE));
					if (NULL == rule.RemotePortRanges)
					{
						AddLogEntryEx(SECONDLEVEL, L"### ERROR: can't allocate memory for remote port in CVibraniumFirewallApp::CreateRule");
						status = STATUS_INSUFFICIENT_RESOURCES;
						goto CLEANUP;
					}
					rule.RemotePortRangesCount = 1;
					rule.RemotePortRanges[0].Min = _wtoi(csRemotePortRange);
					rule.RemotePortRanges[0].Max = _wtoi(csRemotePortRange);
				}
			}
		}

		// Add the rule in ignis
		status = PubIgnisAddRule(&rule);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: IgnisAddRule failed with status 0x%x", status);
			goto CLEANUP;
		}

		/*
		EnterCriticalSection(&gXmlLock);
		if (InsertRuleToXml(&rule))
		{
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: Inserting the new rule in xml failed");
		}
		LeaveCriticalSection(&gXmlLock);
		*/

		status = FreeClientRule(&rule);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: FreeClientRule failed with status 0x%x", status);
			goto CLEANUP;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::CreateRule", 0, 0, true, SECONDLEVEL);
	}
CLEANUP:
	PTSTATUS statusSup;
	if (!PT_SUCCESS(status))
	{
		statusSup = FreeClientRule(&rule);
		if (!PT_SUCCESS(statusSup))
		{
			printf("FreeClientRule failed with status: 0x%X\n", statusSup);
		}
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CreateFirewallDB
*  Description    : Function to create firewall database.
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizFirewallApp::CreateFirewallDB()
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csAppRulesDB = L"";
		csAppRulesDB.Format(L"%sVBFIREWALL.DB", csWardWizModulePath);

		CT2A dbPath(csAppRulesDB, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);

		objSqlDb.Open();

		CStringA csInsertQuery;
		csInsertQuery.Format("CREATE TABLE IF NOT EXISTS WWIZ_FIREWALL_PACKET_RULE_CONFIG(ID INTEGER PRIMARY KEY, LOCAL_IP TINYINT NOT NULL, LOCAL_IP_DATA VARCHAR(1000), LOCAL_PORT TINYINT NOT NULL, LOCAL_PORT_DATA VARCHAR(1000), REMOTE_IP TINYINT NOT NULL, REMOTE_IP_DATA VARCHAR(1000), REMOTE_PORT TINYINT NOT NULL, REMOTE_PORT_DATA VARCHAR(1000), PROGRAMPATH VARCHAR(1000) NOT NULL, PROGRAMNAME VARCHAR(500)NOT NULL, PROTOCOL TINYINT NOT NULL, ACCESS TINYINT NOT NULL, NETWORKTYPE VARCHAR(50), DIRECTION INTEGER NOT NULL, SETRULES BOOL NOT NULL, RESERVED1 NVARCHAR(250), RESERVED2 NVARCHAR(250), RESERVED3 NVARCHAR(250));");

		objSqlDb.ExecDML(csInsertQuery);

		csInsertQuery.Format("CREATE TABLE IF NOT EXISTS WWIZ_FIREWALL_NETWORK_SETTINGS_CONFIG(ID INTEGER PRIMARY KEY, CONNECTION_NAME NVARCHAR(500), NW_STATUS BOOL, NETWORK_PROFILE TINYINT, NETWORKCATEGORY TINYINT, IPV4IFINDEX INTEGER, IPV6IFINDEX INTEGER, INTERFACETYPE TINYINT, GUID VARCHAR(100), LOCALIPV4 VARCHAR(20), LOCALIPV6 VARCHAR(40), DNSSERVERIPV4 VARCHAR(100), DNSSERVERIPV6 VARCHAR(200), GATEWAYSIPV4 VARCHAR(20), GATEWAYSIPV6 VARCHAR(40), GW_PHY_ADDR VARCHAR(40), DHCPIPV4 VARCHAR(20), DHCPIPV6 VARCHAR(40), DHCP_PHY_ADDR VARCHAR(40),ACTIVEPROFILE TINYINT, RESERVED1 NVARCHAR(250), RESERVED2 NVARCHAR(250), RESERVED3 NVARCHAR(250));");

		objSqlDb.ExecDML(csInsertQuery);

		objSqlDb.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::CreateFirewallDB", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : EnableDisFWPortScnProt
*  Description    : Enable disable toggler button for Port scan protection of firewall. 
*					0x00 - disable, 0x01- Enable
*  Author Name    : Amol Jaware 
*  Date			  :	06 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT void EnableDisFWPortScnProt(DWORD dwRegValue, LPTSTR szGUID)
{
	__try
	{
		bool bEnable = dwRegValue ? true : false;
		theApp.EnableDisablePortScan(gAdapters, gNumberOfAdapters, bEnable);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in EnableDisFWPortScnProt", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : EnableDisFWStealthMode
*  Description    : Enable disable toggler button for Stealth mode of firewall.
*  Author Name    : Amol Jaware
*  Date			  :	06 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT void EnableDisFWStealthMode(DWORD dwRegValue, LPTSTR szGUID)
{
	__try
	{
		bool bEnable = dwRegValue ? true : false;
		theApp.EnableDisableStealthMode(gAdapters, gNumberOfAdapters, bEnable);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in EnableDisFWStealthMode", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : EnableDisFirewallSettingsTab
*  Description    : Enable disable toggler button for firewall settings tab.
*					0x01 - ON
*					0x00 - OFF
*  Author Name    : Amol Jaware
*  Date			  :	07 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT void EnableDisFirewallSettingsTab(DWORD dwRegValue, LPTSTR szGUID)
{
	__try
	{
		if (dwRegValue == 0x01)
		{
			if (theApp.WrdWizFwInitialize() != 0x00)
			{
				AddLogEntry(L"### Failed to VibraniumFwInitialize in EnableDisFirewallSettingsTab", 0, 0, true, SECONDLEVEL);
			}
		}
		else
		{
			if (theApp.WrdWizFwUnInitialize() != 0x00)
			{
				AddLogEntry(L"### Failed to VibraniumFwUnInitialize in EnableDisFirewallSettingsTab", 0, 0, true, SECONDLEVEL);
			}
		}

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in EnableDisFirewallSettingsTab", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : DeleteRule
*  Description    : Function to delete rule using ID
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizFirewallApp::DeleteRule(LONGLONG id)
{
	bool bReturn = false;
	__try
	{
		// Delete the rule from ignis
		PTSTATUS status = STATUS_SUCCESS;
		status = PubIgnisDeleteRule(id);   // Flags not used
		if (PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"Deleted rule with id %I64d", id);

			/*
			// Delete the rule from the xml
			EnterCriticalSection(&gXmlLock);
			if (DeleteRuleFromXml(id))
				printf("Error deleting rule with id %llu from xml\n", id);
			LeaveCriticalSection(&gXmlLock);
			*/

			bReturn = true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::DeleteRule", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ClearAllRules
*  Description    : Function which clears all rules
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizFirewallApp::ClearAllRules()
{
	bool bReturn = false;
	__try
	{
		PTSTATUS status = STATUS_SUCCESS;
		// Delete all rules from ignis
		status = PubIgnisDeleteAllRules();
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"### IgnisDeleteAllRules failed with status 0x%x", status);
		}
		else
		{
			AddLogEntryEx(ZEROLEVEL, L"[SUCCESS]: Deleted all rules");
			
			/*
			// Delete all rules from xml
			EnterCriticalSection(&gXmlLock);
			if (DeleteAllRulesFromXml())
			{
				AddLogEntryEx(SECONDLEVEL, L"Deleting all the rules from xml failed");
			}
			LeaveCriticalSection(&gXmlLock);
			*/
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::ClearAllRules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : GetProfile
*  Description    : Function to get profile using GUIID
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
LONGLONG CWardWizFirewallApp::GetProfile(CHAR* guidString)
{
	QWORD profile = 0x00;
	try
	{
		if (!guidString)
			return profile;

		GUID guid;
		PTSTATUS status = STATUS_SUCCESS;

		status = ConvertStringToGUID(guidString, &guid);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"Invalid GUID specified, GUID: [%s]", CString(guidString));
			return profile;
		}

		status = PubIgnisGetProfileForAdapter(&guid, &profile);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"ERROR: IgnisGetProfileForAdapter failed for GUID %s with status: 0x%X", guidString, status);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::GetProfile", 0, 0, true, SECONDLEVEL);
	}
	return profile;
}

/***************************************************************************************************
*  Function Name  : SetProfile
*  Description    : Function which sets profile using GUIID
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizFirewallApp::SetProfile(CHAR* guidString, QWORD profile)
{
	try
	{
		if (!guidString)
			return false;
		
		GUID guid;
		PTSTATUS status = STATUS_SUCCESS;
		status = ConvertStringToGUID(guidString, &guid);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"Invalid GUID specified, GUID: [%s]", CString(guidString));
			return false;
		}

		EnterCriticalSection(&gAdapterListClientLock);
		status = PubIgnisSetProfileForAdapter(&guid, profile);
		if (PT_SUCCESS(status))
		{
			// we need to manually update our adapter structure because it would be absurd
			// for the callback to happen because the adapter changed their profiles
			DWORD i;
			for (i = 0; i < gNumberOfAdapters; ++i)
			{
				if (0 == memcmp(&guid, &(gAdapters[i].AdapterID), sizeof(GUID)))
				{
					gAdapters[i].ActiveProfile = profile;
					break;
				}
			}
		}
		else
		{
			AddLogEntryEx(SECONDLEVEL, L"ERROR: IgnisSetProfileForAdapter failed for GUID %s with profile 0x%I64X, status: 0x%X", guidString, profile, status);
		}
		LeaveCriticalSection(&gAdapterListClientLock);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::SetProfile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : EnableDisablePortScan
*  Description    : Function which enables/disable port scan for specified network adaptor
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizFirewallApp::EnableDisablePortScan(CHAR* guidString, bool bEnable)
{
	OutputDebugString(L"CWardWizFirewallApp::EnableDisablePortScan");
	try
	{
		if (!guidString)
			return false;
		
		GUID guid;
		PTSTATUS status = STATUS_SUCCESS;

		status = ConvertStringToGUID(guidString, &guid);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"Invalid GUID specified, GUID: [%s]", CString(guidString));
			return false;
		}

		status = PubIgnisSetPortscanForAdapter(&guid, bEnable);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"ERROR: IgnisSetPortscanForAdapter failed for GUID [%s] with status : 0x%X", guidString, status);
			return false;
		}

		if (bEnable)
		{
			OutputDebugString(L"Port Scan is Enabled");
			AddLogEntryEx(ZEROLEVEL, L"Port Scan is Enabled for GUID: [%s]", CString(guidString));
		}
		else
		{
			OutputDebugString(L"Port Scan is Disabled");
			AddLogEntryEx(ZEROLEVEL, L"Port Scan is Disabled for GUID: [%s]", CString(guidString));
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::EnableDisablePortScan", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ClearPortScan
*  Description    : Function in which All locked hosts will be removed.
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizFirewallApp::ClearPortScan()
{
	__try
	{
		PTSTATUS status = STATUS_SUCCESS;
		status = PubIgnisSetOpt(OPT_CLEAR_PORTSCAN_LOCKED_HOSTS, 0, 1);   // Flags not used
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"ERROR: `clearportscan` failed, status 0x%08x", status);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::ClearPortScan", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ISStealthModeEnabled
*  Description    : Function which check that steath mode is enabled or not.
*  Author Name    : Ram Shelke
*  Date			  :	10 Aug 2018
****************************************************************************************************/
bool CWardWizFirewallApp::ISStealthModeEnabled(CHAR *guidString)
{
	try
	{
		GUID guid;
		PTSTATUS status = STATUS_SUCCESS;
		BOOLEAN value;
		status = ConvertStringToGUID(guidString, &guid);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"Invalid GUID specified, GUID: [%s]", CString(guidString));
			return false;
		}

		status = PubIgnisGetStealthForAdapter(&guid, &value);
		if (PT_SUCCESS(status))
		{
			if (value)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			AddLogEntryEx(SECONDLEVEL, L"ERROR: IgnisSetPortscanForAdapter failed for GUID %s with status: 0x%X", guidString, status);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::ISStealthModeEnabled", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SetStealthMode
*  Description    : Function to set stealth mode for network adaptor
*  Author Name    : Ram Shelke
*  Date			  :	10 Aug 2018
****************************************************************************************************/
bool CWardWizFirewallApp::SetStealthMode(CHAR *guidString, UINT value)
{
	try
	{
		GUID guid;
		PTSTATUS status = STATUS_SUCCESS;
		status = ConvertStringToGUID(guidString, &guid);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"Invalid GUID specified, GUID: [%s]", CString(guidString));
			return false;
		}

		status = PubIgnisSetStealthForAdapter(&guid, (BOOLEAN)value);
		if (PT_SUCCESS(status))
		{
			if (value)
			{
				AddLogEntryEx(ZEROLEVEL, L"Stealth mode is enabled");
			}
			else
			{
				AddLogEntryEx(ZEROLEVEL, L"Stealth mode is disabled");
			}
			return true;
		}
		else
		{
			AddLogEntryEx(SECONDLEVEL, L"ERROR: IgnisSetPortscanForAdapter failed for GUID %s with status: 0x%X", guidString, status);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::SetStealthMode", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : EnableDisablePortScan
*  Description    : Function to Enable disable port scan for all adaptors
*  Author Name    : Ram Shelke
*  Date			  :	10 Aug 2018
****************************************************************************************************/
void CWardWizFirewallApp::EnableDisablePortScan(
	__in        IGNIS_ADAPTER*      Adapters,
	__in        DWORD               NoOfAdapters,
	__in        bool				bEnable
	)
{
	__try
	{
		DWORD i = 0x00;
		CHAR guidString[MAX_PATH];

		if (NULL == Adapters)
		{
			return;
		}

		// Print every network adapter on the console
		for (i = 0; i < NoOfAdapters; ++i)
		{
			ConvertGUIDToString(&(Adapters[i].AdapterID), guidString);
			EnableDisablePortScan(guidString, bEnable);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::EnableDisablePortScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : EnableDisableStealthMode
*  Description    : Function to Enable disable stealth mode for all adaptors
*  Author Name    : Ram Shelke
*  Date			  :	10 Aug 2018
****************************************************************************************************/
void CWardWizFirewallApp::EnableDisableStealthMode(
	__in        IGNIS_ADAPTER*      Adapters,
	__in        DWORD               NoOfAdapters,
	__in        bool				bEnable
	)
{
	__try
	{
		DWORD i = 0x00;
		CHAR guidString[MAX_PATH];

		if (NULL == Adapters)
		{
			return;
		}

		// Print every network adapter on the console
		for (i = 0; i < NoOfAdapters; ++i)
		{
			ConvertGUIDToString(&(Adapters[i].AdapterID), guidString);
			UINT uiStealthMode = bEnable ? 0x01 : 0x00;
			SetStealthMode(guidString, uiStealthMode);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::EnableDisableStealthMode", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : VisuallyDisplayNetwork
*  Description    : To insert Adapter data in database
*  Author Name    : Kunal Waghmare
*  SR_NO		  :
*  Date			  :	11 Aug 2018
****************************************************************************************************/
void CWardWizFirewallApp::VisuallyDisplayNetwork(
__in        IGNIS_ADAPTER*      Adapters,
__in        DWORD               NoOfAdapters
)
{
	try
	{
		DWORD i, j;
		CHAR guidString[MAX_PATH];
		CStringA csLocalIP4;
		CStringA csLocalIP6;
		CStringA csDNSIP4;
		CStringA csDNSIP6;
		CStringA csGWIP4;
		CStringA csGWIP6;
		CStringA csGWPAddr;
		CStringA csDHCPIP4;
		CStringA csDHCPIP6;
		CStringA csDHCP_PAddr;
		int iEthernet = 0;
		int iWifi = 0;
		CString csConn_Name;
		if (NULL == Adapters)
		{
			return;
		}
		CWardWizSQLiteDatabase objSqlDb("VBFIREWALL.DB");

		//get here registry value
		CString csRegKeyPath;
		csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		CITinRegWrapper objReg;
		DWORD dwStealthMode = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwStealthMode", dwStealthMode) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwStealthMode in CVibraniumFirewallApp::VisuallyDisplayNetwork KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}
		
		//check here stealth mode settings and apply
		EnableDisableStealthMode(Adapters, NoOfAdapters, dwStealthMode ? true : false);

		// Print every network adapter on the console
		for (i = 0; i < NoOfAdapters; ++i)
		{

			csLocalIP4 = "";
			csLocalIP6 = "";
			csDNSIP4 = "";
			csDNSIP6 = "";
			csGWIP4 = "";
			csGWIP6 = "";
			csGWPAddr = "";
			csDHCPIP4 = "";
			csDHCPIP6 = "";
			csDHCP_PAddr = "";
			csConn_Name = "";

			ConvertGUIDToString(&(Adapters[i].AdapterID), guidString);
			
			if (Adapters[i].Medium > 71)
			{
				continue;
			}
			else if (Adapters[i].Medium == 6)
			{
				iEthernet++;
				csConn_Name.AppendFormat(L"Ethernet%d", iEthernet);
			}
			else if (Adapters[i].Medium == 71)
			{
				iWifi++;
				csConn_Name.AppendFormat(L"Wifi Network%d", iWifi);
			}

			for (j = 0; j < Adapters[i].NoOfLocalIpAddresses; ++j)
			{
				if (Adapters[i].LocalIpAddresses == NULL)
				{
					break;
				}

				if (ipVersion4 == Adapters[i].LocalIpAddresses[j].Version)
				{
					CHAR   ip[128] = { 0 };
					GetIPV4(&(Adapters[i].LocalIpAddresses[j]), &ip);
					csLocalIP4 = ip;
				}

				if (ipVersion6 == Adapters[i].LocalIpAddresses[j].Version)
				{
					CHAR   ip[128] = { 0 };
					GetIPV6(&(Adapters[i].LocalIpAddresses[j]), &ip);
					csLocalIP6 = ip;
				}
			}

			for (j = 0; j < Adapters[i].NoOfDnsServers; ++j)
			{
				if (Adapters[i].DnsServers == NULL)
				{
					break;
				}
				if (ipVersion4 == Adapters[i].DnsServers[j].Version)
				{
					if (csDNSIP4.GetLength() > 0)
					{
						csDNSIP4.Append(",");
					}
					
					CHAR   ip[128] = { 0 };
					GetIPV4(&(Adapters[i].DnsServers[j]), &ip);
					csDNSIP4.Append(CStringA(ip));
				}
				if (ipVersion6 == Adapters[i].DnsServers[j].Version)
				{
					if (csDNSIP6.GetLength() > 0)
					{
						csDNSIP6.Append(",");
					}
					CHAR   ip[128] = { 0 };
					GetIPV6(&(Adapters[i].DnsServers[j]), &ip);
					csDNSIP6.Append(CStringA(ip));
				}
			}

			for (j = 0; j < Adapters[i].NoOfGateways; ++j)
			{
				if (Adapters[i].Gateways == NULL)
				{
					break;
				}
				if (ipVersion4 == Adapters[i].Gateways[j].Version)
				{
					CHAR   ip[128] = { 0 };
					GetIPV4(&(Adapters[i].Gateways[j]), &ip);
					csGWIP4.Append(CStringA(ip));
				}
				if (ipVersion6 == Adapters[i].Gateways[j].Version)
				{
					CHAR   ip[128] = { 0 };
					GetIPV6(&(Adapters[i].Gateways[j]), &ip);
					csGWIP6.Append(CStringA(ip));
				}
				if (Adapters[i].Gateways[j].PhysAddrLen)
				{
					CHAR   ip[128] = { 0 };
					GetIPV6(&(Adapters[i].Gateways[j]), &ip);
					csGWPAddr.Append(CStringA(ip));
				}
			}

			for (j = 0; j < Adapters[i].NoOfDhcpServers; ++j)
			{
				if (Adapters[i].DhcpServers == NULL)
				{
					break;
				}
				if (ipVersion4 == Adapters[i].DhcpServers[j].Version)
				{
					if (csDHCPIP4.GetLength() > 0)
					{
						csDHCPIP4.Append(",");
					}
					CHAR   ip[128] = { 0 };
					GetIPV4(&(Adapters[i].DhcpServers[j]), &ip);
					csDHCPIP4.Append(CStringA(ip));
				}
				if (ipVersion6 == Adapters[i].DhcpServers[j].Version)
				{
					if (csDHCPIP6.GetLength() > 0)
					{
						csDHCPIP6.Append(",");
					}
					CHAR   ip[128] = { 0 };
					GetIPV6(&(Adapters[i].DhcpServers[j]), &ip);
					csDHCPIP6.Append(CStringA(ip));
				}
				if (Adapters[i].DhcpServers[j].PhysAddrLen)
				{
					CHAR   ip[128] = { 0 };
					GetIPV6(&(Adapters[i].DhcpServers[j]), &ip);
					csDHCP_PAddr.Append(CStringA(ip));
				}
			}

			CStringA csSelectQuery = "SELECT ID FROM WWIZ_FIREWALL_NETWORK_SETTINGS_CONFIG WHERE GUID = ";
			csSelectQuery.AppendFormat("'%s';", guidString);

			objSqlDb.Open();
			CWardwizSQLiteTable qResult = objSqlDb.GetTable(csSelectQuery);
			objSqlDb.Close();

			CString csInsertQuery;
			DWORD dwRows = qResult.GetNumRows();
			if (dwRows != 0)
			{
				//update data
				csInsertQuery = _T("UPDATE WWIZ_FIREWALL_NETWORK_SETTINGS_CONFIG SET LOCALIPV4 = ");
				csInsertQuery.AppendFormat(L"'%s', NETWORKCATEGORY = %d, LOCALIPV6 = '%s', IPV4IFINDEX = %d, IPV6IFINDEX = %d, INTERFACETYPE = %d, GUID = '%s', DNSSERVERIPV4 = '%s', \
											DNSSERVERIPV6 = '%s', GATEWAYSIPV4 = '%s', GATEWAYSIPV6 = '%s', GW_PHY_ADDR = '%s', DHCPIPV4 = '%s',\
											DHCPIPV6 = '%s', DHCP_PHY_ADDR = '%s', ACTIVEPROFILE = %d \
											WHERE GUID = '%s'", CString(csLocalIP4), 1, CString(csLocalIP6), Adapters[i].Ipv4IfIndex, Adapters[i].Ipv6IfIndex, 
											Adapters[i].Medium, CString(guidString), CString(csDNSIP4), CString(csDNSIP6), CString(csGWIP4), CString(csGWIP6), CString(csGWPAddr), 
											CString(csDHCPIP4), CString(csDHCPIP6), CString(csDHCP_PAddr), Adapters[i].ActiveProfile, CString(guidString));
			}
			else
			{
				//insert data
				csInsertQuery = _T("INSERT INTO WWIZ_FIREWALL_NETWORK_SETTINGS_CONFIG(CONNECTION_NAME, NW_STATUS , NETWORK_PROFILE , NETWORKCATEGORY, IPV4IFINDEX, \
								   									   	IPV6IFINDEX, INTERFACETYPE, GUID, LOCALIPV4, LOCALIPV6, DNSSERVERIPV4, DNSSERVERIPV6, GATEWAYSIPV4, GATEWAYSIPV6, GW_PHY_ADDR, DHCPIPV4, DHCPIPV6, DHCP_PHY_ADDR,ACTIVEPROFILE,\
																												RESERVED1, RESERVED2, RESERVED3) VALUES (");

				csInsertQuery.AppendFormat(_T("'%s', %d, %d, %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, '%s', '%s', '%s');"),
					csConn_Name, 0, 1, 1, Adapters[i].Ipv4IfIndex, Adapters[i].Ipv6IfIndex, Adapters[i].Medium, CString(guidString), CString(csLocalIP4), CString(csLocalIP6),
					CString(csDNSIP4), CString(csDNSIP6), CString(csGWIP4), CString(csGWIP6), CString(csGWPAddr), CString(csDHCPIP4), CString(csDHCPIP6), CString(csDHCP_PAddr),
					Adapters[i].ActiveProfile, "", "", "");
			}

			objSqlDb.Open();
			CT2A ascii(csInsertQuery, CP_UTF8);
			objSqlDb.ExecDML(ascii.m_psz);
			objSqlDb.Close();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::VisuallyDisplayNetwork", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SendParCtrlMessage2Tray
*  Description    : To send message to tray
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
bool CWardWizFirewallApp::SendParCtrlMessage2Tray(int iRequest, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(TRAY_SERVER, true, 0x03);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumFirewallApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CVibraniumFirewallApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

///***************************************************************************************************
//*  Function Name  : SendParCtrlMessage2Tray
//*  Description    : To send message to tray
//*  Author Name    : Jeena Mariam Saji
//*  Date			  :	10 Aug 2018
//****************************************************************************************************/
//bool CWardWizFirewallApp::GetNetworkCategory(GUID IgnisGUID, NLM_NETWORK_CATEGORY &iNetworkCategory)
//{
//	bool bReturn = false;
//	try
//	{
//		HRESULT hr;
//		CComPtr <INetworkListManager> m_pNLM;
//		m_pNLM.CoCreateInstance(CLSID_NetworkListManager);
//		CComPtr<IEnumNetworks> pEnumNetworks;
//		if (SUCCEEDED(m_pNLM->GetNetworks(NLM_ENUM_NETWORK_ALL, &pEnumNetworks)))
//		{
//
//			DWORD dwReturn = 0;
//			while (true)
//			{
//				CComPtr<INetwork> pNetwork;
//				hr = pEnumNetworks->Next(1, &pNetwork, &dwReturn);
//				if (hr == S_OK && dwReturn > 0)
//				{
//					if (!pNetwork) continue;
//
//					GUID nwGUID;
//					pNetwork->GetNetworkId(&nwGUID);
//
//					CHAR guidString[MAX_PATH];
//					ConvertGUIDToString(&nwGUID, guidString);
//					
//					CHAR guidIgnis[MAX_PATH];
//					ConvertGUIDToString(&IgnisGUID, guidIgnis);
//
//					CStringA csCmp;
//					csCmp.Format("[%s] [%s]", guidString, guidIgnis);
//					OutputDebugStringA(csCmp);
//
//					if (memcmp(&IgnisGUID, &nwGUID, sizeof(GUID)) == 0x00)
//					{
//						NLM_NETWORK_CATEGORY enCat;
//						pNetwork->GetCategory(&enCat);
//						iNetworkCategory = enCat;
//						bReturn = true;
//						break;
//					}
//				}
//				else
//				{
//					break;
//				}
//
//			}
//
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CVibraniumFirewallApp::GetNetworkCategory", 0, 0, true, SECONDLEVEL);
//		bReturn = false;
//	}
//	return bReturn;
//}


