#include "ScanTask.h"
#include "ExecuteProcess.h"

CScanTask::CScanTask() :
	m_bStop(false)
{
}

CScanTask::~CScanTask()
{
}

void CScanTask::ReInitializeVariables()
{
	m_bStop = false;
}

void CScanTask::StopRunningTasks()
{
	m_bStop = true;
}

bool CScanTask::StartScan(int iScanType, CString csTaskID, CStringArray &csPath, CString csScanPath)
{
	bool bReturn = false;
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFullPath[MAX_PATH] = { 0 };

		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		_tcscpy_s(szFullPath, szModulePath);
		_tcscat_s(szFullPath, L"\\WRDWIZUSBDETECTUI.EXE");

		CString csCommandline;
		switch (iScanType)
		{
		case 1:
			csCommandline.Format(L"-EPSNOUI -QUICKSCAN -TID:%s", csTaskID);
			break;
		case 2:
			csCommandline.Format(L"-EPSNOUI -FULLSCAN -TID:%s", csTaskID);
			break;
		case 3:
			csCommandline.Format(L"-EPSNOUI -CUSTOMSCAN -TID:%s %s", csTaskID, csScanPath);
			break;
		default:
			break;
		}

		if (SendMessage2Service(LAUNCH_APPLICATION, szFullPath, csCommandline, true) == 0x01)
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanTask::StartScan", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : SendMessage2Tray
Description    : Send Respective Message to tray.
Author Name    : Jeena Mariam Saji
S.R. No        :
Date           : 12 Mar 2018
****************************************************************************/
DWORD CScanTask::SendMessage2Service(int iRequest, CString csAppPath, CString csCommand, bool bWait)
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
				AddLogEntry(L"### Failed to SendData in CScanTask::SendMessage2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CScanTask::SendMessage2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
			dwReturn = szPipeData.dwValue;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanTask::SendMessage2Service", 0, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}
