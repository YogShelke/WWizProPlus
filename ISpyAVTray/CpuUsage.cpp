/************************************************************************************************************
*  Program Name		: CpuUsage.cpp
*  Description		: This class used to calculate total CPU usage.
*  Author Name		: Ram Shelke
*  Date Of Creation : 08 Sep 2017
************************************************************************************************************/
#include "stdafx.h"
#include <windows.h>
#include "CpuUsage.h"

/***************************************************************************************************
*  Function Name  :	CCpuUsage
*  Description    : Const'r
*  Author Name    : Ram Shelke
*  Date           :	27 Nov 2017
***************************************************************************************************/
CCpuUsage::CCpuUsage(void)
{
	//Code block initialization for the memory referenced in the Kernel
	MEMORYSTATUSEX memStat;
	memStat.dwLength = sizeof (memStat);
	GlobalMemoryStatusEx(&memStat);

	//loads the SYSTEMTIME
	SYSTEMTIME sysTime;
	//Retrieves data so that we have a way to Get it to output when using the pointers
	GetSystemTime(&sysTime);
}

/***************************************************************************************************
*  Function Name  :	~CCpuUsage
*  Description    : Dest'r
*  Author Name    : Ram Shelke
*  Date           :	27 Nov 2017
****************************************************************************************************/
CCpuUsage::~CCpuUsage()
{
}

/***************************************************************************************************
*  Function Name  :	CalculateCPULoad
*  Description    : Function which calculates CPU usage.
*  Author Name    : Ram Shelke
*  Date           :	27 Nov 2017
****************************************************************************************************/
float CCpuUsage::CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
	static unsigned long long _previousTotalTicks = 0;
	static unsigned long long _previousIdleTicks = 0;

	unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
	unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;


	float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

	_previousTotalTicks = totalTicks;
	_previousIdleTicks = idleTicks;
	return ret;
}

/***************************************************************************************************
*  Function Name  :	FileTimeToInt64
*  Description    : Function which converts file time to int64
*  Author Name    : Ram Shelke
*  Date           :	27 Nov 2017
****************************************************************************************************/
unsigned long long CCpuUsage::FileTimeToInt64(const FILETIME & ft)
{
	return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}

/***************************************************************************************************
*  Function Name  :	GetCPULoad
*  Description    :
*  Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
*  You'll need to call this at regular intervals, since it measures the load between
*  the previous call and the current one.  Returns -1.0 on error.
*  Author Name    : Ram Shelke
*  Date           :	27 Nov 2017
****************************************************************************************************/
float CCpuUsage::GetCPULoad()
{
	FILETIME idleTime, kernelTime, userTime;
	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
}
