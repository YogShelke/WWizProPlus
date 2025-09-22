#pragma once
#include "stdafx.h"

class CWardWizFwClient
{
public:
	CWardWizFwClient();
	virtual ~CWardWizFwClient();
	bool Initialize();
	bool UnInitialize();
	bool LoadRequiredModules();
	DWORD CreateRule(LPSTR lpProcessPath, RULE_ACTION dwAction, TRAFFIC_DIRECTION dwDirection, bool bCheckMD5, BYTE dwCheckProtocol,
		BYTE Protocol, BYTE bCheckLocalIP, LPTSTR lpLocalIP, BYTE bCheckRemoteIP, LPTSTR csRemoteIP, BYTE bCheckLocalPort,
		LPTSTR lpLocalPortRange, BYTE bCheckRemotePort, LPTSTR lpRemotePortRange);
	bool LoadFirewallRuleList();
	bool ReLoadFirewallRuleList();
	void ReLoadFWControlSettings4PortScanProt(DWORD dwRegValue, LPTSTR szFirstParam = EMPTY_STRING);
	void ReLoadFWControlSettings4StealthMode(DWORD dwRegValue, LPTSTR szFirstParam = EMPTY_STRING);
	void ReLoadFWControlSettingsTab(DWORD dwRegValue);
	void CreateTable4Firewall();
	bool ClearAllRules();
	bool SetDefaultAppBehaviour(DWORD dwValue);
	bool LoadFirewallAllApplicationRules();
private:
	HMODULE								m_hModuleFwDLL;
	WRDWIZFWINITIALIZE					m_lpFwInitialize;
	WRDWIZFWUNINITIALIZE				m_lpFwUnInitialize;
	CREATEFWRULES						m_lpCreateRules;
	ENABLEDISFWPORTSCNPROT				m_lpEnableDisFWPortScnProt;
	ENABLEDISFWSTEALTHMODE				m_lpEnableDisFWStealthMode;
	ENABLEDISFIREWALLSETTINGSTAB		m_lpEnableDisFirewallSettinsTab;
	CLEARALLRULES						m_lpClearAllRules;
	SETDEFAULTAPPBEHAVIOUR				m_lpSetDefaultAppBehaviour;
	CStringA							m_csDatabaseFilePath;
};

