#pragma once
#include "CommonFunctions.h"
#include "Connections.h"

class CNetworkInstallation
{
public:
	CNetworkInstallation();
	virtual ~CNetworkInstallation();
	DWORD InstallClientSetup(LPTSTR lpszTaskID, LPTSTR lpszIP, LPTSTR lpszUserName, LPTSTR lpszPassword);
	CString GetMachineNameByIP(CString csIPAddress);
	void ReInitializeVariables();
	void StopRunningTasks();
	DWORD SendMessage2Tray(int iRequest, CString csFirstParam, CString csSecondParam, CString csThirdParam, bool bWait = false);
	DWORD InstallClientSetupUsingTray(LPTSTR lpszTaskID, LPTSTR lpszIP, LPTSTR lpszUserName, LPTSTR lpszPassword);
private:
	bool	m_bStop;
};

