#include "NetworkInstallation.h"

CNetworkInstallation::CNetworkInstallation():
	m_bStop(false)
{
}

CNetworkInstallation::~CNetworkInstallation()
{
}

DWORD CNetworkInstallation::InstallClientSetupUsingTray(LPTSTR lpszTaskID, LPTSTR lpszIP, LPTSTR lpszUserName, LPTSTR lpszPassword)
{
	DWORD dwRet = 0x00;
	try
	{
		if (!lpszIP)
			return SANITY_CHECK_FAILURE;

		if (!lpszUserName)
			return SANITY_CHECK_FAILURE;

		if (!lpszPassword)
			return SANITY_CHECK_FAILURE;

		CString csFirstParam;
		csFirstParam.Format(L"%s %s", lpszTaskID, lpszIP);

		if (SendMessage2Tray(EPS_INSTALL_SETUP, csFirstParam, lpszUserName, lpszPassword, true) != 0x01)
		{
			dwRet = 0x01;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNetworkInstallation::InstallClientSetupUsingTray, IP: [%s]", lpszIP, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

DWORD CNetworkInstallation::InstallClientSetup(LPTSTR lpszTaskID, LPTSTR lpszIP, LPTSTR lpszUserName, LPTSTR lpszPassword)
{
	DWORD dwRet = 0x00;
	try
	{
		if (!lpszIP)
			return SANITY_CHECK_FAILURE;

		if (!lpszUserName)
			return SANITY_CHECK_FAILURE;

		if (!lpszPassword)
			return SANITY_CHECK_FAILURE;

		CString csMachineID;
		CString csMachineIPAddress = lpszIP;
		CString csMachineName = GetMachineNameByIP(csMachineIPAddress);

		CString sMsg;
		sMsg.Format(L"[%s] Checking connection!", csMachineName);
		OutputDebugString(sMsg);

		CConnection oConnection(csMachineIPAddress, csMachineName, L"", NULL, false);
		oConnection.m_csUserName = lpszUserName;
		oConnection.m_csPassword = lpszPassword;
		DWORD dwRet = oConnection.EstablishAdminConnection(lpszTaskID);
		if (dwRet != 0x00)
		{
			return dwRet;
		}

		dwRet = oConnection.InstallClient(lpszTaskID, csMachineID);
		if (dwRet != 0x00)
		{
			return dwRet;
		}

		dwRet = 0x00;
		sMsg.Format(L"[%s] Client Installed successfully!", csMachineName);
		OutputDebugString(sMsg);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNetworkInstallation::InstallClientSetup, IP: [%s]", lpszIP, 0, true, SECONDLEVEL);
	}
	return dwRet;
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

/***************************************************************************
Function Name  : SendMessage2Tray
Description    : Send Respective Message to tray.
Author Name    : Jeena Mariam Saji
S.R. No        :
Date           : 12 Mar 2018
****************************************************************************/
DWORD CNetworkInstallation::SendMessage2Tray(int iRequest, CString csFirstParam, CString csSecondParam, CString csThirdParam, bool bWait)
{
	DWORD dwReturn = 0x00;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;
		_tcscpy(szPipeData.szFirstParam, csFirstParam);
		_tcscpy(szPipeData.szSecondParam, csSecondParam);
		_tcscpy(szPipeData.szThirdParam, csThirdParam);

		CISpyCommunicator objCom(TRAY_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CNetworkInstallation::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return dwReturn;
			}
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CNetworkInstallation::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return dwReturn;
			}
			dwReturn = szPipeData.dwValue;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNetworkInstallation::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}