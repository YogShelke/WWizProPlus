/************************************************************************************************************
*  Program Name		: CpuUsage.h
*  Description		: This class used to calculate total CPU usage.
*  Author Name		: Ram Shelke
*  Date Of Creation : 08 Sep 2017
************************************************************************************************************/
#pragma once

class CCpuUsage
{
public:
	CCpuUsage();
	virtual ~CCpuUsage();
	float GetCPULoad();
private:
	//functions to calculate and retrieve CPU Load information
	//functions to calculate and retrieve CPU Load information
	float CalculateCPULoad();
	unsigned long long FileTimeToInt64(const FILETIME & ft);
	float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks);
};

