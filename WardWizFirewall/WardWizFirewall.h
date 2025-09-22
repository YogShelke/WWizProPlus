// WardWizFirewall.h : main header file for the WardWizFirewall DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#pragma warning (disable:4005)

#include "resource.h"		// main symbols
#include <sal.h>

extern "C"
{
	#include "ptstatus.h"
}

#include <ntstatus.h>
typedef long NTSTATUS;
#define WIN32_NO_STATUS

#include <winsock2.h>
#include "ignis_dll.h"
#include "xml_helper.h"
#include "callbacks.h"
#include "ip_helper.h"
#include "adapter_helper.h"
#include "imports.h"
#include "rule_helper.h"
#include "options_public.h"
#include "xml_helper.h"

extern IGNIS_ADAPTER*       gAdapters;
extern DWORD                gNumberOfAdapters;
extern CRITICAL_SECTION     gAdapterListClientLock;


// CWardWizFirewallApp
// See WardWizFirewall.cpp for the implementation of this class
//

class CWardWizFirewallApp : public CWinApp
{
public:
	CWardWizFirewallApp();

// Overrides
public:
	virtual BOOL InitInstance();
	DWORD WrdWizFwInitialize();
	DWORD WrdWizFwUnInitialize();
	DWORD WrdWizFwInitializeSEH();
	bool SaveFileXML();
	DWORD CreateRule(LPSTR lpProcessPath, RULE_ACTION dwAction, TRAFFIC_DIRECTION dwDirection, bool bCheckMD5, BYTE dwCheckProtocol,
		BYTE Protocol, BYTE bCheckLocalIP, LPTSTR csLocalIP, BYTE bCheckRemoteIP, LPTSTR csRemoteIP, BYTE bCheckLocalPort,
		LPTSTR csLocalPortRange, BYTE bCheckRemotePort, LPTSTR csRemotePortRange);
	bool CreateFirewallDB();
	bool DeleteRule(LONGLONG id);
	bool ClearAllRules();
	LONGLONG GetProfile(CHAR* guidString);
	bool SetProfile(CHAR* guidString, QWORD profile);
	bool EnableDisablePortScan(CHAR* guidString, bool bEnable);
	bool ClearPortScan();
	bool ISStealthModeEnabled(CHAR *guidString);
	bool SetStealthMode(CHAR *guidString, UINT value);
	bool SetDefaultAppBehaviour(DWORD dwValue);
	void EnableDisablePortScan(
		__in        IGNIS_ADAPTER*      Adapters,
		__in        DWORD               NoOfAdapters,
		__in        bool				bEnable
		);
	void EnableDisableStealthMode(
		__in        IGNIS_ADAPTER*      Adapters,
		__in        DWORD               NoOfAdapters,
		__in        bool				bEnable
		);
	void CWardWizFirewallApp::VisuallyDisplayNetwork(
		__in        IGNIS_ADAPTER*      Adapters,
		__in        DWORD               NoOfAdapters
		);

	bool SendParCtrlMessage2Tray(int iRequest, bool bWait = false);
	//bool GetNetworkCategory(GUID IgnisGUID, NLM_NETWORK_CATEGORY &iNetworkCategory);
private:
	TiXmlDocument	m_xmlDoc; 
	DECLARE_MESSAGE_MAP()
};

extern CWardWizFirewallApp theApp;