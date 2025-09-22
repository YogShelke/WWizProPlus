#include "stdafx.h"
#include "UninstallClient.h"
#include "ExecuteProcess.h"

using namespace std;

CUninstallClient::CUninstallClient():
m_bStop(false)
{
}


CUninstallClient::~CUninstallClient()
{
}


void CUninstallClient::StopRunningTasks()
{
	m_bStop = true;
}


void CUninstallClient::ReInitializeVariables()
{
	m_bStop = false;
}

bool CUninstallClient::UnInstallClientSetup(std::string strKeepUserDefineSettingsID, std::string strIskeepQurantinedFileID)
{
	bool bReturn = false;
	try
	{
		CString csCmdLine = L"-EPSNOUI -UNINSTALL";
		if (strKeepUserDefineSettingsID.length() == 0 || strIskeepQurantinedFileID.length() == 0)
			return false;

		if (strcmp(strKeepUserDefineSettingsID.c_str(), "1") == 0)
			csCmdLine += L" 1";
		else
			csCmdLine += L" 0";

		if (strcmp(strIskeepQurantinedFileID.c_str(), "1") == 0)
			csCmdLine += L" 1";
		else
			csCmdLine += L" 0";

		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFullPath[MAX_PATH] = { 0 };

		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		_tcscpy_s(szFullPath, szModulePath);
		_tcscat_s(szFullPath, L"\\WARDWIZUNINSTCON.EXE");

		if (SendMessage2Service(LAUNCH_APP_IN_USER_CONTEXT, szFullPath, csCmdLine, true) == 0x01)
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUninstallClient::UnInstallClientSetup", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : SendMessage2Tray
Description    : Send Respective Message to tray.
Author Name    : Jeena Mariam Saji
S.R. No        :
Date           : 10 Mar 2018
****************************************************************************/
DWORD CUninstallClient::SendMessage2Service(int iRequest, CString csAppPath, CString csCommand, bool bWait)
{
	DWORD dwReturn = 0x00;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;
		_tcscpy(szPipeData.szFirstParam, csAppPath);
		_tcscpy(szPipeData.szSecondParam, csCommand);

		CISpyCommunicator objCom(SERVICE_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CUninstallClient::SendMessage2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CUninstallClient::SendMessage2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
			dwReturn = szPipeData.dwValue;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUninstallClient::SendMessage2Service", 0, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}