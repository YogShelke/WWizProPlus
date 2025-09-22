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

bool CScanTask::StartScan(int iScanType, CString csTaskID, CStringArray &csPath)
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
		_tcscat_s(szFullPath, L"\\WRDWIZUI.EXE");

		CString csCommandline;
		switch (iScanType)
		{
		case 1:
			csCommandline.Format(L"-EPSNOUI -QUICKSCAN -%s", csTaskID);
			break;
		case 2:
			csCommandline.Format(L"-EPSNOUI -FULLSCAN -%s", csTaskID);
			break;
		case 3:
			csCommandline.Format(L"-EPSNOUI -I -CUSTOMSCAN %s", csTaskID);
			break;
		default:
			break;
		}

		CExecuteProcess			objExecprocess;
		objExecprocess.StartProcessWithTokenExplorerWait(szFullPath, csCommandline, L"explorer.exe");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanTask::StartScan", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

