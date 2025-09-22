/**********************************************************************************************************
*		Program Name          : WWizFirewallFilter.h
*		Description           : App class which has functionality for Firewall as well as Email Scan.
*		Author Name			  : Ram Shelke
*		Date Of Creation      : 29th APR 2019
*		Version No            : 3.5.0.1
*		Modification Log      :
***********************************************************************************************************/
#pragma once

#include "stdafx.h"
#include "nfdriver.h"
#include "ScannerContants.h"
#include <vector>
#include <map>
#include "nfapi.h"

using namespace nfapi;

class CWWizFirewallFilter
{
public:
	CWWizFirewallFilter();
	virtual ~CWWizFirewallFilter();
	bool LoadBlockedApplicationList();
	bool ReLoadBlockedApplicationList();
	bool ISApplicationBlocked(std::wstring strProcessName);
	void OnOpenDevice();
	void OnCloseDevice();
	void OnClearEvent();
	void OnSetEvent();
	bool StartPortScanProtection();
	bool StopPortScanProtection();
	bool InsertIntoFWReports();
	bool SendParCtrlMessage2Tray(int iRequest, bool bWait = false);
	HANDLE			m_hCommDevice;
	HANDLE			m_hCommEvent;
	HANDLE			m_hThread;
	bool			m_bStopPortScan;
	bool			m_bPortScanDetected;
private:
	std::vector<std::string>	m_vBlockedAppList;
};

