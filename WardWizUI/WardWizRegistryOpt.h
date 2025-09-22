/****************************************************
*  Program Name: CWardWizRegistryOpt.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28th March 2016
*  Version No: 2.0.0.1
****************************************************/
#pragma once

#include "RegOptStruct.h"
#include "iSpyMemMapClient.h"
#include "WardWizDatabaseInterface.h"
// CWardWizRegistryOpt

class CWardWizRegistryOpt :		sciter::event_handler,
								sciter::behavior_factory
{

	HELEMENT self;
public:
	CWardWizRegistryOpt();
	virtual ~CWardWizRegistryOpt();

	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}

	BEGIN_FUNCTION_MAP
		FUNCTION_1("OnStartRegistryOpt", On_StartRegistryOpt) // On_OnStartRegistryOpt()	
		FUNCTION_0("OnStopRegistryOptOpr", On_StopRegistryOpt) // On_OnStopRegistryOpt()		
		FUNCTION_0("OnPauseRegistryOptOpr", On_PauseRegistryOpt) // On_OnStopRegistryOpt()		
		FUNCTION_0("OnResumeRegistryOptOpr", On_ResumeRegistryOpt) // On_OnStopRegistryOpt()		
		FUNCTION_0("funClickShowDetails", On_funClickShowDetails)
		FUNCTION_0("OnSetTimer", OnSetTimer)
		FUNCTION_0("CallContinueRegOptScan", On_CallContinueRegOptScan)
		FUNCTION_0("OnStartRegistryOpt4SchedScan", On_StartRegistryOpt4SchedScan)
	END_FUNCTION_MAP

	//Registry Optimizer related functions
	json::value On_StartRegistryOpt(SCITER_VALUE svArrRegOptSelectedEntries);
	json::value On_StopRegistryOpt();
	json::value On_PauseRegistryOpt();
	json::value On_ResumeRegistryOpt();
	json::value On_funClickShowDetails();
	json::value On_StartRegistryOpt4SchedScan();
	json::value OnSetTimer();
	json::value On_CallContinueRegOptScan();
public:
	void StartRegistryOptScan(LPREGOPTSCANOPTIONS lpRegOpt);
	void GetDWORDFromScanOptions(DWORD &dwRegScanOpt, LPREGOPTSCANOPTIONS lpRegOpt);
	void callUISetRegistryStatusfunction(CString csPercent, CString csTotalEntries);
	void ScanningStopped();

	bool StopRegistryOptScan();
	bool PauseRegistryOptimizer();
	bool ResumeRegistryOptimizer();

	void EnterRegistryOptimizerDetails(DWORD dwTotalFileCount, DWORD dwRepairedFileCount);
	void GetRegistryOptionList(CString csCommandLine);
	void OnCallSetPercentage(CString csPercentage, CString csTotalEntries);
	void OnCallSetFinishStatus();
	iSpyServerMemMap_Client m_objiTinServerMemMap_Client;
	HANDLE			m_hRegOptThread;
	DWORD			m_dwPercentage;
	DWORD			m_dwTotalEntries;
	SCITER_VALUE	m_svFunSetRegistryScanStatusCB;
	CString			m_csCommandLine;
	bool			m_bIsMultiRegOptFinish;
	sciter::dom::element	ela;
};


