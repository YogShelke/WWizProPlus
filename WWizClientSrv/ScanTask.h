#pragma once
#ifdef __windows__
#undef __windows__
#endif
#include "CommonFunctions.h"
#include "Connections.h"

class CScanTask
{
public:
	CScanTask();
	virtual ~CScanTask();
	void ReInitializeVariables();
	void StopRunningTasks();
	bool StartScan(int iScanType, CString csTaskID, CStringArray &csPath, CString csScanPath);
	DWORD SendMessage2Service(int iRequest, CString csAppPath, CString csCommand, bool bWait = false);
private:
	bool	m_bStop;
};

