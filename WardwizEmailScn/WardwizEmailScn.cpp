/**********************************************************************************************************
*		Program Name          : CWardwizEmailScnApp.cpp
*		Description           : App class which has functionality for Firewall as well as Email Scan.
*		Author Name			  : Ram Shelke
*		Date Of Creation      : 29th APR 2019
*		Version No            : 3.5.0.1
*		Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "WardwizEmailScn.h"
#include "PFMailFilter.h"
#include "WardwizOSVersion.h"
#include "WWizParentalCntrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT _declspec(dllimport)
#define DLLEXPORT _declspec(dllexport)

MailFilter					g_mf;
EventHandler				g_EventHandle;
CWWizParentalCntrl			g_objParentalCtrl;

// CWardwizEmailScnApp
BEGIN_MESSAGE_MAP(CWardwizEmailScnApp, CWinApp)
END_MESSAGE_MAP()

/***********************************************************************************************
*  Function Name  : CWardwizEmailScnApp
*  Description    : Constructor
*  Author Name    : Ram Shelke
*  Date           : 29-MAR-2019
*************************************************************************************************/
CWardwizEmailScnApp::CWardwizEmailScnApp() :
m_bInitialized(false),
m_iEnabledFilters(FILTER_NONE)
{
	m_bIsEmailScanEnabled = false;
	m_bIsWebFilterEnabled = false;
	m_bIsBrowserSecEnabled = false;
	m_bPControlState = false;
	LoadUserPermission();
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	//m_bEnableDisableEmail = false;
}

// The one and only CWardwizEmailScnApp object
CWardwizEmailScnApp theApp;

/***********************************************************************************************
*  Function Name  : InitInstance
*  Description    : Function which gets called which initializing DLL
*  Author Name    : Ram Shelke
*  Date           : 29-MAR-2019
*************************************************************************************************/
BOOL CWardwizEmailScnApp::InitInstance()
{
	CWinApp::InitInstance();

	g_mf.SetParentalControlPtr(&g_objParentalCtrl);

	return TRUE;
}

/***************************************************************************************************
*  Function Name  : ExitInstance()
*  Description    : Release resources used by this DLL
*  Author Name    : Ram
*  Date			  :	17-Mar-2015
****************************************************************************************************/
int CWardwizEmailScnApp::ExitInstance()
{
	theApp.WrdWizEmailUnInitialize();
	theApp.WrdWizUnInitializeWebFilter();
	return 0;
}

/***************************************************************************************************
*  Function Name  : EnableFilter
*  Description    : Function to Enable Filter
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
void CWardwizEmailScnApp::EnableFilter(FILTERFLAGS filter)
{
	//OutputDebugString(L"CWardwizEmailScnApp::EnableFilter");

	std::lock_guard<std::mutex> lock(m_MutexLock);
	if (!m_bInitialized)
	{
		nf_adjustProcessPriviledges();

		TCHAR		szModulePath[MAX_PATH] = { 0 };
		CString		csModulePath = L"";

		if (!GetModulePath(szModulePath, MAX_PATH))
			return;

		csModulePath = szModulePath;

		//for all OS it gets installed.
		csModulePath += L"\\VBFLT";
		if (!pf_init(&g_mf, csModulePath))
		{
			AddLogEntry(L"### Failed to initialize protocol filter", 0, 0, true, SECONDLEVEL);
			return;
		}

		pf_setRootSSLCertSubject("Vibranium");

		auto status = nfapi::nf_init(NFDRIVER_NAME, &g_EventHandle);
		if (NF_STATUS_SUCCESS != status)
		{
			status = nfapi::nf_init(NFDRIVER_NAME, &g_EventHandle);
			if (NF_STATUS_SUCCESS != status)
			{
				AddLogEntryEx(SECONDLEVEL, L"### Cant get it to start Firewall, LastError: [%d]", GetLastError());
				return;
			}
		}
		OutputDebugString(L"NF_STATUS_SUCCESS\n");
		m_bInitialized = status == NF_STATUS_SUCCESS;
	}

	m_iEnabledFilters |= filter;
	if (m_iEnabledFilters && m_bInitialized)
	{
		nfapi::NF_RULE rule = { 0 };

		// Bypass system connections
		memset(&rule, 0, sizeof(rule));
		rule.filteringFlag = nfapi::NF_ALLOW;
		rule.processId = 4;
		nf_addRule(&rule, FALSE);

		// Block QUIC
		memset(&rule, 0, sizeof(rule));
		rule.protocol = IPPROTO_UDP;
		rule.remotePort = ntohs(80);
		rule.filteringFlag = NF_BLOCK;
		nf_addRule(&rule, FALSE);

		memset(&rule, 0, sizeof(rule));
		rule.protocol = IPPROTO_UDP;
		rule.remotePort = ntohs(443);
		rule.filteringFlag = NF_BLOCK;
		nf_addRule(&rule, FALSE);

		// Filter all other cases
		memset(&rule, 0, sizeof(rule));
		rule.filteringFlag = nfapi::NF_FILTER | nfapi::NF_INDICATE_CONNECT_REQUESTS;
		rule.direction = nfapi::NF_D_OUT | nfapi::NF_D_IN;
		auto status = nf_addRule(&rule, FALSE);

		status = nfapi::nf_filterIcmp(true);
		if (NF_STATUS_SUCCESS != status)
		{
			AddLogEntry(L"### failed nf_filterIcmp", 0, 0, true, SECONDLEVEL);
			return;
		}
	}
}

/***************************************************************************************************
*  Function Name  : DisableFilter
*  Description    : Function to Disable firewall filter.
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	16 APR 2019
****************************************************************************************************/
void CWardwizEmailScnApp::DisableFilter(FILTERFLAGS filter)
{
	try
	{
		std::lock_guard<std::mutex> lock(m_MutexLock);

		m_iEnabledFilters &= ~filter;
		if (!m_iEnabledFilters && m_bInitialized)
		{
			nfapi::nf_filterIcmp(false);
			nfapi::nf_deleteRules();
			nf_free();
			m_bInitialized = false;
		}
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in CWardwizEmailScnApp::DisableFilter");
	}
}

/***************************************************************************************************
*  Function Name  : IsFilterEnabled
*  Description    : Function to check filter Enabled or not.
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	16 APR 2019
****************************************************************************************************/
bool CWardwizEmailScnApp::IsFilterEnabled(FILTERFLAGS filter) const
{
	return (m_iEnabledFilters & filter) == filter;
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
		bReturn = theApp.LoadDefaultRules();
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
bool CWardwizEmailScnApp::SetDefaultAppBehaviour(DWORD dwValue)
{
	__try
	{
		g_EventHandle.SetDefaultBehaviour(dwValue);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::SetDefaultAppBehaviour", 0, 0, true, SECONDLEVEL);
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
DWORD CWardwizEmailScnApp::WrdWizFwUnInitialize()
{
	__try
	{
		if (IsFilterEnabled(FILTER_APPLICATION))
		{
			//clear firewall rules
			ClearAllRules();
			LoadDefaultRules();

			DisableFilter(FILTER_APPLICATION);

			if (!g_EventHandle.m_objFirewallFilter.StopPortScanProtection())
			{
				AddLogEntryEx(SECONDLEVEL, L"### Failed to Stop port scan protection, LastError: [%d]", GetLastError());
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::VibraniumFwUnInitialize", 0, 0, true, SECONDLEVEL);
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
DWORD CWardwizEmailScnApp::WrdWizFwInitialize()
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
DWORD CWardwizEmailScnApp::WrdWizFwInitializeSEH()
{
	OutputDebugString(L"CWardwizEmailScnApp::WrdWizFwInitializeSEH");
	try
	{
		CString csRegKeyPath;
		csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		//Get here registry setting for Active scanning.
		DWORD dwFirewallEnableState = 0x00;
		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwFirewallEnableState", dwFirewallEnableState) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwFirewallEnableState in CWardwizEmailScnApp::VibraniumFwInitializeSEH KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}
		
		FILTERFLAGS eFilterFlag = dwFirewallEnableState ? FILTER_APPLICATION : FILTER_NONE;
		if (dwFirewallEnableState == 0x01)
		{
			EnableFilter(eFilterFlag);
		}
		else
		{
			DisableFilter(eFilterFlag);
		}

		// 0x00	-	Automatic
		// 0x01	-	Allow
		// 0x02	-	Block
		DWORD dwDefAppBehavior = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwDefAppBehavior", dwDefAppBehavior) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwDefAppBehavior in CWardwizEmailScnApp::VibraniumFwInitializeSEH KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}
		g_EventHandle.SetDefaultBehaviour(dwDefAppBehavior);

		// BlockPortScans
		DWORD dwPortScanProt = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwPortScanProt", dwPortScanProt) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwPortScanProt in CWardwizEmailScnApp::VibraniumFwInitializeSEH KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		bool bEnable = dwPortScanProt ? true : false;
		EnableDisablePortScan(bEnable);

		if (bEnable)
		{
			if (!g_EventHandle.m_objFirewallFilter.StartPortScanProtection())
			{
				AddLogEntryEx(SECONDLEVEL, L"### Failed to start port scan protection, LastError: [%d]", GetLastError());
			}
		}

		DWORD dwStealthMode = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwStealthMode", dwStealthMode) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwPortScanProt in CWardwizEmailScnApp::VibraniumFwInitializeSEH KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		bEnable = dwStealthMode ? true : false;
		EnableDisableStealthMode(bEnable);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::InitializeIgnisSEH", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : CreateRule
*  Description    : Function to create rule
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
DWORD CWardwizEmailScnApp::CreateRule(LPSTR lpProcessPath, RULE_ACTION dwAction, TRAFFIC_DIRECTION dwDirection, bool bCheckMD5, BYTE dwCheckProtocol,
	BYTE Protocol, BYTE bCheckLocalIP, LPTSTR lpLocalIP, BYTE bCheckRemoteIP, LPTSTR csRemoteIP, BYTE bCheckLocalPort,
	LPTSTR lpLocalPortRange, BYTE bCheckRemotePort, LPTSTR lpRemotePortRange)
{
	try
	{
		NF_RULE_EX szConInfo = { 0 };
		memset(&szConInfo, 0, sizeof(NF_RULE_EX));

		szConInfo.direction = dwDirection;
		szConInfo.ip_family = AF_INET;

		if (dwCheckProtocol == 0x31)
			szConInfo.protocol = (BYTE)Protocol;

		if (bCheckLocalIP == 0x31)
		{
			if (lpLocalIP != NULL)
			{
				CStringA csLocalIP(lpLocalIP);
				*((unsigned long*)szConInfo.localIpAddress) = inet_addr(csLocalIP);
			}
		}

		if (bCheckLocalPort != 0x30 && bCheckLocalPort <= 0x32)
		{
			if (lpLocalPortRange != NULL)
			{
				CString csLocalPortRange(lpLocalPortRange);
				if ((!csLocalPortRange.IsEmpty()))
				{
					szConInfo.localPort = ntohs(_wtoi(csLocalPortRange));
				}
			}
		}

		if (bCheckRemoteIP == 0x31)
		{
			CStringA csRemoteIP(csRemoteIP);
			*((unsigned long*)szConInfo.remoteIpAddress) = inet_addr(csRemoteIP);
		}

		if (bCheckRemotePort != 0x30 && bCheckRemotePort <= 0x32)
		{
			if (lpRemotePortRange != NULL)
			{
				CString csRemotePortRange(lpRemotePortRange);
				if ((!csRemotePortRange.IsEmpty()))
				{
					szConInfo.remotePort = ntohs(_wtoi(csRemotePortRange));
				}
			}
		}

		szConInfo.filteringFlag = NF_ALLOW;
		if (dwAction == ACTION_DENY)
		{
			szConInfo.filteringFlag = NF_BLOCK;
		}

		if (lpProcessPath)
		{
			CString csProcessPath(lpProcessPath);
			if (csProcessPath.GetLength() > 0)
			{
				CString csProcessName = csProcessPath.Right(csProcessPath.GetLength() - csProcessPath.ReverseFind(L'\\') - 1);
				csProcessName.MakeLower();
				memcpy(&szConInfo.processName, csProcessName.GetBuffer(), sizeof(szConInfo.processName));
			}
		}

		NF_STATUS nfStatus = nfapi::nf_addRuleEx(&szConInfo, TRUE);
		if (NF_STATUS_SUCCESS != nfStatus)
		{
			AddLogEntry(L"### Failed to add Rule nf_addRuleEx", 0, 0, true, SECONDLEVEL);
			return 0x02;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::CreateRule", 0, 0, true, SECONDLEVEL);
	}
	return 0x00;
}

/***************************************************************************************************
*  Function Name  : CreateFirewallDB
*  Description    : Function to create firewall database.
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardwizEmailScnApp::CreateFirewallDB()
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
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::CreateFirewallDB", 0, 0, true, SECONDLEVEL);
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
		theApp.EnableDisablePortScan(bEnable);
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
		theApp.EnableDisableStealthMode(bEnable);
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
*  Function Name  : EnableDisableStealthMode
*  Description    : Function to Enable Disable Stealth Mode
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardwizEmailScnApp::EnableDisableStealthMode(bool bEnable)
{
	bool bReturn = false;
	__try
	{
		NF_STATUS status = nf_filterIcmp(bEnable);
		if (status != NF_STATUS_SUCCESS)
		{
			AddLogEntry(L"### Failed to EnableDisable StealthMode", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::EnableDisableStealthMode", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : DeleteRule
*  Description    : Function to delete rule using ID
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardwizEmailScnApp::DeleteRule(LONGLONG id)
{
	bool bReturn = false;
	__try
	{
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::DeleteRule", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ClearAllRules
*  Description    : Function which clears all rules
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardwizEmailScnApp::ClearAllRules()
{
	bool bReturn = true;
	__try
	{
		NF_STATUS nfStatus = nfapi::nf_deleteRules();
		if (NF_STATUS_SUCCESS != nfStatus)
		{
			AddLogEntry(L"### Failed to delete Rules in nf_deleteRules", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::ClearAllRules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : LoadDefaultRules
*  Description    : Function which add default rules to driver which are required to work.
*  Author Name    : Ram Shelke
*  Date			  :	26 Dec 2019
****************************************************************************************************/
bool CWardwizEmailScnApp::LoadDefaultRules()
{
	bool bReturn = true;
	__try
	{
		nfapi::NF_RULE rule = { 0 };

		// Filter all TCP connections
		memset(&rule, 0, sizeof(rule));
		rule.direction = NF_D_OUT | NF_D_IN;
		rule.protocol = IPPROTO_TCP;
		rule.filteringFlag = NF_FILTER;
		nfapi::nf_addRule(&rule, TRUE);

		// Block QUIC
		memset(&rule, 0, sizeof(rule));
		rule.protocol = IPPROTO_UDP;
		rule.remotePort = ntohs(80);
		rule.filteringFlag = NF_BLOCK;
		nf_addRule(&rule, TRUE);

		memset(&rule, 0, sizeof(rule));
		rule.protocol = IPPROTO_UDP;
		rule.remotePort = ntohs(443);
		rule.filteringFlag = NF_BLOCK;
		auto status = nf_addRule(&rule, TRUE);

		if (NF_STATUS_SUCCESS != status)
		{
			AddLogEntry(L"### failed nf_addRuleEx in CWardwizEmailScnApp::LoadDefaultRules", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::LoadDefaultRules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : EnableDisablePortScan
*  Description    : Function which enables/disable port scan for specified network adaptor
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardwizEmailScnApp::EnableDisablePortScan(bool bEnable)
{
	try
	{
		NF_STATUS status = nfapi::nf_filterPortScan(bEnable);
		if (status != NF_STATUS_SUCCESS)
		{
			AddLogEntry(L"### Failed to EnableDisable PortScan", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::EnableDisablePortScan", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SendParCtrlMessage2Tray
*  Description    : To send message to tray
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
bool CWardwizEmailScnApp::SendParCtrlMessage2Tray(int iRequest, bool bWait)
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
				AddLogEntry(L"### Failed to send data in CWardwizEmailScnApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CWardwizEmailScnApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : WrdWizEmailInitialize
*  Description    : Exported function to initialize Email Scan.
*  Author Name    : Ram Shelke
*  Date           : 29-MAR-2019
*************************************************************************************************/
extern "C" DLLEXPORT bool WrdWizEmailInitialize()
{
	bool bReturn = false;
	try
	{
		bReturn = theApp.WrdWizEmailInitialize();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in WardwizEmailInitialize", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : WrdWizFwUnInitialize
*  Description    : Function to Uninitilize library
*  Author Name    : Amol Jaware
*  Date			  :	01 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WrdWizEmailUnInitialize()
{
	__try
	{
		// Free the libraries
		theApp.WrdWizEmailUnInitialize();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizEmailUnInitialize", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WrdWizWebProtInitialize
*  Description    : Exported function to initialize Website protection
*  Author Name    : Amol Jaware
*  Date			  :	01 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WrdWizWebProtInitialize()
{
	bool bReturn = false;
	try
	{
		bReturn = theApp.WrdWizInitializeWebFilter();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in WardwizEmailInitialize", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : WardWizBrowserSecInitialize
*  Description    : Exported function to initialize Browser Security
*  Author Name    : Jeena Mariam Saji
*  Date			  :	04 Sept 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WardWizBrowserSecInitialize()
{
	bool bReturn = false;
	try
	{
		bReturn = theApp.WrdWizInitializeBrowserSec();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in WardwizBrowserSecInitialize", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : WrdWizFwUnInitialize
*  Description    : Function to Uninitilize library
*  Author Name    : Amol Jaware
*  Date			  :	01 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WrdWizWebProtUnInitialize()
{
	__try
	{
		// Free the libraries
		theApp.WrdWizUnInitializeWebFilter();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizEmailUnInitialize", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WardWizBrowserSecUnnitialize
*  Description    : Exported function to uninitialize Browser Security
*  Author Name    : Jeena Mariam Saji
*  Date			  :	04 Sept 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WardWizBrowserSecUnnitialize()
{
	__try
	{
		// Free the libraries
		theApp.WardWizUnnitializeBrowserSec();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizBrowserSecUnnitialize", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WardWizBrowserSecReload
*  Description    : Exported function to reload Browser Security values
*  Author Name    : Jeena Mariam Saji
*  Date			  :	04 Sept 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WardWizBrowserSecReload()
{
	__try
	{
		g_mf.ReloadBrowserSecDB();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizBrowserSecReload", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WardWizBrowserSecExcReload
*  Description    : Exported function to reload Browser Security exclusion values
*  Author Name    : Jeena Mariam Saji
*  Date			  :	04 Sept 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WardWizBrowserSecExcReload()
{
	__try
	{
		g_mf.ReloadBrowserSecExclusionDB();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizBrowserSecExcReload", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WardWizBrowserSecSpecReload
*  Description    : Exported function to reload Browser Security specific values
*  Author Name    : Jeena Mariam Saji
*  Date			  :	04 Sept 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WardWizBrowserSecSpecReload()
{
	__try
	{
		g_mf.ReloadBrowserSecSpecificDB();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizBrowserSecSpecReload", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReloadApplicationRules
*  Description    : Function which reloads application rules
*  Author Name    : Ram Shelke
*  Date			  :	13 Jul 2019
****************************************************************************************************/
extern "C" DLLEXPORT bool ReloadApplicationRules()
{
	try
	{
		g_EventHandle.m_objFirewallFilter.ReLoadBlockedApplicationList();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in ReloadApplicationRules", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : WrdWizEmailNFDll
*  Description    : Function to Uninstall nf dll.
*  Author Name    : Amol Jaware
*  Date			  :	02 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool WrdWizEmailNFDllUnlink()
{
	__try
	{
		theApp.UnregisterNFDrivers();
		theApp.GetNFAPIDriverPath();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizEmailNFDll", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : UnregisterNFDrivers
Description    : removes NFAPI drivers
Author Name    : Amol Jaware
Date           : 20/07/2018
****************************************************************************/
void CWardwizEmailScnApp::UnregisterNFDrivers()
{
	CWardWizOSversion objGetOSVersion;
	const char* csDriverName = "";
	int OsType = 0;
	try
	{
		OsType = objGetOSVersion.DetectClientOSVersion();
		//issue solved 6882
		/*if (OsType == 5 || OsType == 6 || OsType == 10) //XP,Vista & XP64
		{
		csDriverName = "VBFLT";
		}*/
		CITinRegWrapper	 m_objReg;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		TCHAR szValueAppVersion[MAX_PATH] = { 0 };
		DWORD dwSizeAppVersion = sizeof(szValueAppVersion);
		m_objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"AppVersion", szValueAppVersion, dwSizeAppVersion);

		CString csValueAppVersion(szValueAppVersion);
		int iAppVersion[4] = { 0 };

		if (csValueAppVersion.GetLength() > 0)
			ParseVersionString(iAppVersion, csValueAppVersion);
		else
		{
			//for all OS
			csDriverName = "VBFLT";
			if (strlen(csDriverName))
				nfapi::nf_unRegisterDriver(csDriverName);
			return;
		}
		if (iAppVersion[0] <= 3 && iAppVersion[1] <= 1)
		{
			if (OsType == 7) //win7
			{
				csDriverName = "VBFLT07";
			}
			if (OsType == 8) //win8
			{
				csDriverName = "VBFLT10";
			}
			if (OsType == 11) //win10
			{
				csDriverName = "VBFLT10";
			}
			if (strlen(csDriverName))
				nfapi::nf_unRegisterDriver(csDriverName);

			if (OsType != 7)//avoid installation faild error while uninstalling the setup.
			{
				csDriverName = "VBFLT07";
				nfapi::nf_unRegisterDriver(csDriverName);
			}
		}
		else
		{
			//for all OS
			csDriverName = "VBFLT";
			if (strlen(csDriverName))
				nfapi::nf_unRegisterDriver(csDriverName);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::UnregisterNFDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : GetNFAPIDriverPath
Description    : This function used to get path of drivers folder to delete nfapi drivers.
Author Name    : Amol Jaware
Date           : 23 July 2018
***********************************************************************************************/
void CWardwizEmailScnApp::GetNFAPIDriverPath()
{
	CString csWrdWizFLTDriverPath;
	TCHAR systemDirPath[MAX_PATH] = _T("");
	try
	{
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));
		csWrdWizFLTDriverPath.Format(L"%s\\drivers\\%s", systemDirPath, L"VBFLT.SYS");//VBFLT.SYS For all OS
		if (PathFileExists(csWrdWizFLTDriverPath))
		DeleteNFDrivers(csWrdWizFLTDriverPath);
		csWrdWizFLTDriverPath = L"";

		csWrdWizFLTDriverPath.Format(L"%s\\drivers\\%s", systemDirPath, L"VBFLT07.SYS");// VBFLT07.SYS
		if (PathFileExists(csWrdWizFLTDriverPath))
			DeleteNFDrivers(csWrdWizFLTDriverPath);
		csWrdWizFLTDriverPath = L"";

		csWrdWizFLTDriverPath.Format(L"%s\\drivers\\%s", systemDirPath, L"VBFLT08.SYS");//VBFLT08.SYS 
		if (PathFileExists(csWrdWizFLTDriverPath))
			DeleteNFDrivers(csWrdWizFLTDriverPath);
		csWrdWizFLTDriverPath = L"";

		csWrdWizFLTDriverPath.Format(L"%s\\drivers\\%s", systemDirPath, L"VBFLT10.SYS");//VBFLT10.SYS 
		if (PathFileExists(csWrdWizFLTDriverPath))
			DeleteNFDrivers(csWrdWizFLTDriverPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::GetNFAPIDriverPath", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : DeleteNFDrivers
Description    : This function will delete all nfapi drivers from drivers folder.
Author Name    : Amol Jaware
Date           : 23 July 2018
***********************************************************************************************/
void CWardwizEmailScnApp::DeleteNFDrivers(CString csWrdWizFLTDriverPath)
{
	try
	{
		SetFileAttributes(csWrdWizFLTDriverPath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(csWrdWizFLTDriverPath))
		{
			AddLogEntry(L"### Failed to delete file CWardwizEmailScnApp::DeleteNFDrivers %s", csWrdWizFLTDriverPath, 0, true, FIRSTLEVEL);
			if (PathFileExists(csWrdWizFLTDriverPath))
			{
				MoveFileEx(csWrdWizFLTDriverPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::DeleteNFDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ReLoadEmailScanSettings
*  Description    : 
*  Author Name    : 
*  Date			  :	07 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool ReLoadEmailScanSettings(DWORD dwRegValue)
{
	bool bReturn = false;
	bool bEnableDisableEmailState = false;
	__try
	{
			if (dwRegValue == 1)
			{
				bEnableDisableEmailState = true;
			}
			else if (dwRegValue == 0)
			{
				bEnableDisableEmailState = false;
			}

			theApp.EnableDisableEmailScan(bEnableDisableEmailState);
		
		if (bReturn)
		{
			AddLogEntry(L"### Failed to connect driver while initialiazing drivers in ReLoadEmailScanSettings", 0, 0, true, SECONDLEVEL);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ReLoadEmailScanSettings", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ReLoadParentalControlSettings
*  Description    :
*  Author Name    :
*  Date			  :	07 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool ReLoadParentalControlSettings(DWORD dwRegValue)
{
	bool bReturn = false;
	__try
	{
		//if (dwRegValue) TODO: write required code here 
		if (dwRegValue == 0x00)
		{
			theApp.m_bBlockInternetUsage = false;
			theApp.m_bRestrictInternetUsage = false;
			theApp.UninitializWWPCWebFileter();
		}
		else
		{
			theApp.WrdWizInitializeWebFilter();
		}
		if (bReturn)
			AddLogEntry(L"### Failed to connect driver while initialiazing drivers in ReLoadParentalControlSettings", 0, 0, true, SECONDLEVEL);

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ReLoadParentalControlSettings", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : StartParControlCheck
*  Description    : To start parental control check
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool StartParControlCheck()
{
	bool bReturn = true;
	__try
	{
		g_objParentalCtrl.StartParentalControl();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartParControlCheck", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : StartInternetCheck
*  Description    : To start parental control Internet check
*  Author Name    : Jeena Mariam Saji
*  Date			  :	7 Sept 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool StartInternetCheck()
{
	bool bReturn = true;
	__try
	{
		g_objParentalCtrl.StartInternetCheck();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartInternetCheck", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}
/***************************************************************************************************
*  Function Name  : StartINetUsageCheck
*  Description    : To start parental control internet usage check
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool StartINetUsageCheck()
{
	bool bReturn = true;
	__try
	{
		g_objParentalCtrl.StartParCtrlINetAccess();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StartINetUsageCheck", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : WrdWizEmailInitialize
*  Description    : Function to initialize NFApi drivers for email scan with respective OS.
*  Author Name    : Amol Jaware
*  Date			  :	07 Aug 2018
****************************************************************************************************/
bool CWardwizEmailScnApp::WrdWizEmailInitialize()
{
	try
	{
		TCHAR		szModulePath[MAX_PATH] = { 0 };
		CString		csModulePath = L"";

		CITinRegWrapper	 m_objReg;
		DWORD dwEmailScan = 0x00;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwEmailScanState", dwEmailScan) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwEmailScanState in CWardwizEmailScnApp::WardwizEmailInitialize KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);
		}

		if (dwEmailScan == 0)
		{
			m_bIsEmailScanEnabled = false;
			return false;
		}
		else if (dwEmailScan == 1)
			m_bIsEmailScanEnabled = true;

		if (!GetModulePath(szModulePath, MAX_PATH))
			return false;

		csModulePath = szModulePath;

		EnableFilter(FILTER_APPLICATION);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::WardwizEmailInitialize", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WrdWizEmailInitialize
*  Description    : Function to initialize NFApi drivers for email scan with respective OS.
*  Author Name    : Amol Jaware
*  Date			  :	07 Aug 2018
****************************************************************************************************/
bool CWardwizEmailScnApp::WrdWizInitializeWebFilter()
{
	try
	{
		TCHAR		szModulePath[MAX_PATH] = { 0 };
		CString		csModulePath = L"";

		CITinRegWrapper	 m_objReg;
		DWORD dwParentalCntrlFlg = 0x00;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParentalCntrlFlg) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwWebFilterState in CWardwizEmailScnApp::WardwizInitializeWebFilter KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);
		}

		if (dwParentalCntrlFlg == 0)
		{
			m_bIsWebFilterEnabled = false;
				return false;
		}
		else if (dwParentalCntrlFlg == 1)
			m_bIsWebFilterEnabled = true;

		if (!GetModulePath(szModulePath, MAX_PATH))
			return false;

		csModulePath = szModulePath;

		EnableFilter(FILTER_WEB);

		g_mf.ReInitializeMailFilter();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::WardwizInitializeWebFilter", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WrdWizInitializeBrowserSec
*  Description    : Function to initialize browser security values 
*  Author Name    : Jeena Mariam Saji
*  Date			  :	04 Sept 2019
****************************************************************************************************/
bool CWardwizEmailScnApp::WrdWizInitializeBrowserSec()
{
	try
	{
		TCHAR		szModulePath[MAX_PATH] = { 0 };
		CString		csModulePath = L"";

		CITinRegWrapper	 m_objReg;
		DWORD dwBrowserSecFlg = 0x00;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwBrowserSecurityState", dwBrowserSecFlg) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwBrowserSecurityState in CWardwizEmailScnApp::WardwizInitializeBrowserSec KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);
		}

		if (dwBrowserSecFlg == 0)
		{
			m_bIsBrowserSecEnabled = false;
				return false;
		}
		else if (dwBrowserSecFlg == 1)
			m_bIsBrowserSecEnabled = true;

		if (!GetModulePath(szModulePath, MAX_PATH))
			return false;

		csModulePath = szModulePath;

		EnableFilter(FILTER_WEB);

		g_mf.ReloadBrowserSecDB();
		g_mf.ReloadBrowserSecExclusionDB();
		g_mf.ReloadBrowserSecSpecificDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::WardwizInitializeBrowserSec", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : UninitializWWPCWebFileter
*  Description    : Function to Uninitialize WrdWiz PC Web Filter variables.
*  Author Name    : Amol Jaware
*  Date			  :	18 July 2019
****************************************************************************************************/
void CWardwizEmailScnApp::UninitializWWPCWebFileter()
{
	try
	{
		g_mf.UninitializeWWPCWebFilter();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::UninitializWardwizPCWebFileter", 0, 0, true, SECONDLEVEL);
	}
	
}

/***************************************************************************************************
*  Function Name  : WrdWizEmailUnInitialize
*  Description    : Function to Un initialize NFApi drivers for Email scan.
*  Author Name    : Ram Shelke
*  Date			  :	26 Jun 2019
****************************************************************************************************/
bool CWardwizEmailScnApp::WrdWizEmailUnInitialize()
{
	try
	{
		DisableFilter(FILTER_APPLICATION);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::WardwizEmailUnInitialize", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/***************************************************************************************************
*  Function Name  : WrdWizUnInitializeWebFilter
*  Description    : Function to Un initialize NFApi drivers for Web Protection.
*  Author Name    : Ram Shelke
*  Date			  :	26 Jun 2019
****************************************************************************************************/
bool CWardwizEmailScnApp::WrdWizUnInitializeWebFilter()
{
	try
	{
		if (CheckFilterFlagsDisabled())
			DisableFilter(FILTER_WEB);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::WardwizUnInitializeWebFilter", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WardWizUnnitializeBrowserSec
*  Description    : Function to Un initialize NFApi drivers for Browser Security
*  Author Name    : Jeena Mariam Saji
*  Date			  :	04 Sept 2019
****************************************************************************************************/
bool CWardwizEmailScnApp::WardWizUnnitializeBrowserSec()
{
	try
	{
		if (CheckFilterFlagsDisabled())
			DisableFilter(FILTER_WEB);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::VibraniumUnnitializeBrowserSec", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CheckFilterFlagsDisabled
*  Description    : Function to check if flags are disabled for the filter
*  Author Name    : Jeena Mariam Saji
*  Date			  :	12 Oct 2019
****************************************************************************************************/
bool CWardwizEmailScnApp::CheckFilterFlagsDisabled()
{
	bool bReturn = false;
	try
	{
		DWORD dwBrowserSecFlg = 0x00;
		DWORD dwParentalCntrlFlg = 0x00;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		CITinRegWrapper	 m_objReg;

		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwBrowserSecurityState", dwBrowserSecFlg) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwBrowserSecurityState in CWardwizEmailScnApp::CheckFilterFlagsDisabled KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);
		}

		if (dwBrowserSecFlg == 0)
			m_bIsBrowserSecEnabled = false;
		else if (dwBrowserSecFlg == 1)
			m_bIsBrowserSecEnabled = true;


		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParentalCntrlFlg) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwWebFilterState in CWardwizEmailScnApp::CheckFilterFlagsDisabled KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);
		}

		if (dwParentalCntrlFlg == 0)
			m_bIsWebFilterEnabled = false;
		else if (dwParentalCntrlFlg == 1)
			m_bIsWebFilterEnabled = true;

		if (!m_bIsBrowserSecEnabled && !m_bIsWebFilterEnabled)
			bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::CheckFilterFlagsDisabled", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : EnableDisableInternetUsage
*  Description    : To enable disable internet usage flag
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool EnableDisableInternetUsage(bool bEnable)
{
	bool bReturn = false;
	try
	{
		theApp.m_bBlockInternetUsage = bEnable;
		theApp.m_bRestrictInternetUsage = bEnable;
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in EnableDisableInternetUsage", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CheckEXEBlockedbyPC
*  Description    : To check if exe is blocked
*  Author Name    : Jeena Mariam Saji
*  Date			  :	13 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool CheckEXEBlockedbyPC(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	try
	{
		bReturn = g_objParentalCtrl.CheckEXEBlocked(lpszFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CheckEXEBlockedbyPC", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ReloadUserListforParCtrl
*  Description    : To reload user list for Parental Control
*  Author Name    : Jeena Mariam Saji
*  Date			  :	11 July 2019
****************************************************************************************************/
extern "C" DLLEXPORT bool ReloadUserListforParCtrl()
{
	try
	{
		g_objParentalCtrl.LoadUserListforPCtrl();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadUserListforParCtrl", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ReloadUserPermission
*  Description    : To reload user permissions
*  Author Name    : Jeena Mariam Saji
*  Date			  :	16 July 2019
****************************************************************************************************/
extern "C" DLLEXPORT bool ReloadUserPermission()
{
	try
	{
		g_objParentalCtrl.LoadUserListforPCtrl();
		g_objParentalCtrl.LoadUserPermission();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadUserPermission", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ReloadApplicationList
*  Description    : Toreload blocked list
*  Author Name    : Jeena Mariam Saji
*  Date			  :	13 Aug 2018
****************************************************************************************************/

extern "C" DLLEXPORT bool ReloadApplicationList()
{
	try
	{
		g_objParentalCtrl.ReLoadBlockedApplication();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in ReloadApplicationList", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ReloadUserResetValue
*  Description    : Toreload user reset value
*  Author Name    : Jeena Mariam Saji
*  Date			  :	01 Aug 2019
****************************************************************************************************/
extern "C" DLLEXPORT bool ReloadUserResetValue()
{
	try
	{
		theApp.m_mapParCtrlUserList.clear();
		theApp.LoadUserPermission();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in ReloadUserResetValue", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : EnableDisableEmailScan
*  Description    : Get state of Email scan enable disable state.
*  Author Name    : Amol Jaware
*  Date			  :	30 Aug 2018
****************************************************************************************************/
void CWardwizEmailScnApp::EnableDisableEmailScan(bool bEmailScanState)
{
	try
	{
		if (bEmailScanState)
		{
			m_bIsEmailScanEnabled = true;
		}
		else
		{
			m_bIsEmailScanEnabled = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizEmailScnApp::EnableDisableEmailScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : EnableDisableParentalControl
*  Description    : Get state of Parental Control enable disable state.
*  Author Name    : Amol Jaware
*  Date			  :	30 Aug 2018
****************************************************************************************************/
void CWardwizEmailScnApp::EnableDisableParentalControl(bool bPControlState)
{
	try
	{
		if (bPControlState)
		{
			m_bPControlState = true;
		}
		else
		{
			m_bPControlState = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizEmailScnApp::EnableDisableEmailScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ParseVersionString
*  Description    : To parse CString version strings and store it in Array of integers.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 12 Sep 2018
****************************************************************************************************/
bool CWardwizEmailScnApp::ParseVersionString(int iDigits[4], CString& csVersion)
{
	try
	{
		int iTokenPos = 0;
		csVersion.Insert(0, _T("."));
		CString csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
		int iVersion = _ttoi(csToken);
		int iSubVersion = 0;
		int iCount = 0;

		iDigits[iCount] = iVersion;
		iCount++;
		while ((!csToken.IsEmpty()) && (iCount <= 3))
		{
			csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
			iSubVersion = _ttoi(csToken);
			iDigits[iCount] = iSubVersion;
			iCount++;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScnApp::ParseVersionString", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DisableFilter
*  Description    : Function to Disable firewall filter.
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	16 APR 2019
****************************************************************************************************/
void CWardwizEmailScnApp::DisablePFFilter(FILTERFLAGS filter)
{
	try
	{
		std::lock_guard<std::mutex> lock(m_MutexLock);

		m_iEnabledFilters &= ~filter;
		if (!m_iEnabledFilters && m_bInitialized)
		{
			nfapi::nf_deleteRules();
		}
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in CWardwizEmailScnApp::DisableFilter");
	}
}

/***************************************************************************************************
*  Function Name  : EnableFilter
*  Description    : Function to Enable Filter
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
void CWardwizEmailScnApp::EnablePFFilter(FILTERFLAGS filter)
{
	//OutputDebugString(L"CWardwizEmailScnApp::EnableFilter");

	std::lock_guard<std::mutex> lock(m_MutexLock);

	if (!m_bInitialized)
	{
		nf_adjustProcessPriviledges();

		TCHAR		szModulePath[MAX_PATH] = { 0 };
		CString		csModulePath = L"";

		if (!GetModulePath(szModulePath, MAX_PATH))
			return;

		csModulePath = szModulePath;

		//for all OS it gets installed.
		csModulePath += L"\\VBFLT";
		if (!pf_init(&g_mf, csModulePath))
		{
			AddLogEntry(L"### Failed to initialize protocol filter", 0, 0, true, SECONDLEVEL);
			return;
		}

		pf_setRootSSLCertSubject("Vibranium");

		auto status = nfapi::nf_init(NFDRIVER_NAME, pf_getNFEventHandler());
		if (NF_STATUS_SUCCESS != status)
		{
			status = nfapi::nf_init(NFDRIVER_NAME, pf_getNFEventHandler());
			if (NF_STATUS_SUCCESS != status)
			{
				AddLogEntryEx(SECONDLEVEL, L"### Cant get it to start Firewall, LastError: [%d]", GetLastError());
				return;
			}
		}
		OutputDebugString(L"NF_STATUS_SUCCESS\n");
		m_bInitialized = status == NF_STATUS_SUCCESS;
	}

	m_iEnabledFilters |= filter;
	if (m_iEnabledFilters && m_bInitialized)
	{
		nfapi::NF_RULE rule = {0};

		// Filter all TCP connections
		memset(&rule, 0, sizeof(rule));
		rule.direction = NF_D_OUT | NF_D_IN;
		rule.protocol = IPPROTO_TCP;
		rule.filteringFlag = NF_FILTER;
		nfapi::nf_addRule(&rule, TRUE);

		// Block QUIC
		memset(&rule, 0, sizeof(rule));
		rule.protocol = IPPROTO_UDP;
		rule.remotePort = ntohs(80);
		rule.filteringFlag = NF_BLOCK;
		nf_addRule(&rule, TRUE);

		memset(&rule, 0, sizeof(rule));
		rule.protocol = IPPROTO_UDP;
		rule.remotePort = ntohs(443);
		rule.filteringFlag = NF_BLOCK;
		auto status = nf_addRule(&rule, TRUE);

		if (NF_STATUS_SUCCESS != status)
		{
			AddLogEntry(L"### failed nf_addRuleEx in CWardwizEmailScnApp::EnablePFFilter", 0, 0, true, SECONDLEVEL);
			return;
		}
	}
}

/***************************************************************************************************
*  Function Name  : ReloadWebSecDB
*  Description    : Function to reload web protection related database
*  Author Name    : Ram Shelke
*  Date			  :	12 Jul 2019
****************************************************************************************************/
extern "C" DLLEXPORT bool ReloadWebSecDB()
{
	try
	{
		g_mf.ReloadWebSecDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in ReloadWebSecDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ReloadWebSecDB
*  Description    : Function to reload web protection settings database
*  Author Name    : Ram Shelke
*  Date			  :	12 Jul 2019
****************************************************************************************************/
extern "C" DLLEXPORT bool ReloadBrowseProtDB()
{
	try
	{
		g_mf.ReloadBrowseProtDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in ReloadBrowseProtDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : ReloadBlkSpecWebDB
Description    : Function to reload Specific website which need to be blocked
Author Name    : Ramkrushna Shelke
Date           : 05 Oct 2018
***********************************************************************************************/
extern "C" DLLEXPORT bool ReloadBlkSpecWebDB()
{
	bool bReturn = false;
	try
	{
		bReturn = g_mf.ReloadBlkSpecWebDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadExcludeDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadMngExcDB
Description    : Function to reload Exclusion list
Author Name    : Ramkrushna Shelke
Date           : 05 Oct 2018
***********************************************************************************************/
extern "C" DLLEXPORT bool ReloadMngExcDB()
{
	bool bReturn = false;
	try
	{
		bReturn = g_mf.ReloadMngExcDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadExcludeDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ClearURLCache
*  Description    : This function will clean prepared cache
*  Author Name    : Amol Jaware
*  Date			  :	30 Aug 2018
****************************************************************************************************/
extern "C" DLLEXPORT bool ClearURLCache()
{
	bool bReturn = false;
	try
	{
		bReturn = g_mf.ClearURLCache();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in ClearURLCache", 0, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadUserPermission
Description    : To load user permission
Author Name    : Jeena Mariam Saji
Date           : 01 August 2019
***********************************************************************************************/
void CWardwizEmailScnApp::LoadUserPermission()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPARCONTROL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		m_mapParCtrlUserList.clear();

		dbSQlite.Open();
		CWardwizSQLiteTable qResultUser = dbSQlite.GetTable("Select USERNAME from WWiz_PC_UserList;");
		Sleep(20);

		for (int iRow = 0; iRow < qResultUser.GetNumRows(); iRow++)
		{
			qResultUser.SetRow(iRow);
			if (qResultUser.GetFieldIsNull(0))
			{
				continue;
			}

			char szUserNameVal[50] = { 0 };
			bool bUserVal = false;
			strcpy_s(szUserNameVal, qResultUser.GetFieldValue(0));
			m_mapParCtrlUserList.insert(make_pair(szUserNameVal, bUserVal));
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::LoadUserPermission ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckUserResetValue
Description    : To check user reset value
Author Name    : Jeena Mariam Saji
Date           : 01 August 2019
***********************************************************************************************/
bool CWardwizEmailScnApp::CheckUserResetValue(std::string strUserName)
{
	bool bReturn = false;
	try
	{
		MAPUSERLISTRESET::iterator it = m_mapParCtrlUserList.find(strUserName);
		if (it != m_mapParCtrlUserList.end())
		{
			bReturn = it->second;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::CheckUserResetValue ", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : FunSetParCtrlUserResetVal
Description    : To set user reset value
Author Name    : Jeena Mariam Saji
Date           : 01 August 2019
***********************************************************************************************/
void CWardwizEmailScnApp::FunSetParCtrlUserResetVal(std::string strUserName)
{
	try
	{
		MAPUSERLISTRESET::iterator it = m_mapParCtrlUserList.find(strUserName);
		if (it != m_mapParCtrlUserList.end())
		{
			it->second = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumParentalCntrl::FunSetParCtrlUserResetVal ", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CheckInternetAccessVal
Description    : To check internet access value
Author Name    : Jeena Mariam Saji
Date           : 10 December 2019
***********************************************************************************************/
extern "C" DLLEXPORT bool CheckInternetAccessVal()
{
	bool bReturn = false;
	try
	{
		if (g_objParentalCtrl.CheckInternetAccessBlock())
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CheckInternetAccessVal", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}