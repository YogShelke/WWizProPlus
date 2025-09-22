#pragma once
#include "CommonFunctions.h"
#include "Connections.h"

class CNetworkInstallation
{
public:
	CNetworkInstallation();
	virtual ~CNetworkInstallation();
	bool InstallClientSetup(LPTSTR lpszIP, LPTSTR lpszUserName, LPTSTR lpszPassword);
	CString GetMachineNameByIP(CString csIPAddress);
	void ReInitializeVariables();
	void StopRunningTasks();
private:
	bool	m_bStop;
};

