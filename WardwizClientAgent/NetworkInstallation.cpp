#include "NetworkInstallation.h"


CNetworkInstallation::CNetworkInstallation():
	m_bStop(false)
{
}


CNetworkInstallation::~CNetworkInstallation()
{
}

bool CNetworkInstallation::InstallClientSetup(LPTSTR lpszIP, LPTSTR lpszUserName, LPTSTR lpszPassword)
{
	try
	{
		if (!lpszIP)
			return false;

		if (!lpszUserName)
			return false;

		if (!lpszPassword)
			return false;

		CString csMachineID;
		CString csMachineIPAddress = lpszIP;
		CString csMachineName = GetMachineNameByIP(csMachineIPAddress);

		CString sMsg;
		sMsg.Format(L"[%s] Checking connection!", csMachineName);
		OutputDebugString(sMsg);

		CConnection oConnection(csMachineIPAddress, csMachineName, L"", NULL, false);
		oConnection.m_csUserName = lpszUserName;
		oConnection.m_csPassword = lpszPassword;
		if (!oConnection.EstablishAdminConnection())
		{
			return false;
		}

		if (!oConnection.InstallClient(csMachineID))
		{
			return false;
		}

		sMsg.Format(L"[%s] Client Installed successfully!", csMachineName);

		OutputDebugString(sMsg);
	}
	catch (...)
	{
	}
	return true;
}

CString CNetworkInstallation::GetMachineNameByIP(CString csIPAddress)
{
	CString csMachineName(csIPAddress);
	unsigned int addr;
	addr = inet_addr((CStringA)csIPAddress);
	HOSTENT *lpHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	if (lpHost)
		csMachineName = lpHost->h_name;

	return csMachineName;
}


void CNetworkInstallation::StopRunningTasks()
{
	m_bStop = true;
}


void CNetworkInstallation::ReInitializeVariables()
{
	m_bStop = false;
}