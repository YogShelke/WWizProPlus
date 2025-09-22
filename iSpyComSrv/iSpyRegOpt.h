/**********************************************************************************************************            	  Program Name          : WardWiz Registry optimization
	  Description           : This class used for Registry Optimization.
	  Author Name			: Ramkrushna Shelke                                                                        	  Date Of Creation      : 17 Jan 2014
	  Version No            : 1.0.0.4
	  Special Logic Used    : 
	  Modification Log      : 
***********************************************************************************************************/
#ifndef _ISPYREGOPT_H_
#define _ISPYREGOPT_H_
#pragma once
#include "iSpySrvMgmt_RegOpt.h"
#include "iSpyMemMapServer.h"

class CISpyRegOpt
{
public:
	CISpyRegOpt();
	~CISpyRegOpt();
	bool StartRegistryOptimizer(DWORD	dwScanOptions);
	bool StopRegistryOptimizer();
	bool PauseRegistryOptimizer();
	bool ResumeRegistryOptimizer();
public:
	HANDLE						m_hThread_StartRegScan;
	bool						m_bIsRegOptInProgress;
};

#endif