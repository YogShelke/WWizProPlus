#pragma once
#include "json_writer.h"
#include "json_reader.h"
#include "json_data.h"
#include "CommonFunctions.h"
#include "Connections.h"

class CScanTask
{
public:
	CScanTask();
	virtual ~CScanTask();
	void ReInitializeVariables();
	void StopRunningTasks();
	bool StartScan(int iScanType, CString csTaskID, CStringArray &csPath);
private:
	bool	m_bStop;
};

