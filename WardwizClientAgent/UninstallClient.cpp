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
		if (strKeepUserDefineSettingsID.length() == 0 || strIskeepQurantinedFileID.length() == 0)
			return false;

		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFullPath[MAX_PATH] = { 0 };

		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		_tcscpy_s(szFullPath, szModulePath);
		_tcscat_s(szFullPath, L"\\WRDWIZUNINST.EXE");

		CExecuteProcess			objExecprocess;
		if (!objExecprocess.StartProcessWithTokenExplorerWait(szFullPath, L"-EPSNOUI -UNINSTALL", L"explorer.exe"))
		{
			return false;
		}
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUninstallClient::UnInstallClientSetup", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
