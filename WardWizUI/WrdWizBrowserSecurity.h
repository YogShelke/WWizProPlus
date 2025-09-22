/****************************************************
*  Program Name: CWrdWizBrowserSecurity.h
*  Author Name: Jeena Mariam Saji
*  Date Of Creation: 20th Aug 2019
*  Version No: 2.0.0.1
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once

#include "WardWizUI.h"

class CWrdWizBrowserSecurity : sciter::event_handler,
	sciter::behavior_factory
{
	HELEMENT self;

public:
	CWrdWizBrowserSecurity();
	virtual ~CWrdWizBrowserSecurity();

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
		FUNCTION_0("GetDBPath", GetDBPath)
		FUNCTION_0("GetReportDBPath", GetReportDBPath)
		FUNCTION_1("OnSetRegVal", OnSetRegVal)
		FUNCTION_0("GetSetRegistryVal", On_GetSetRegistryVal)
		FUNCTION_1("OnReloadBrowserSecurity", OnReloadBrowserSecurity)
		FUNCTION_0("OnReloadBrowserSecRules", OnReloadBrowserSecRules)
		FUNCTION_0("ReloadDB4MngBrowserSecExc", ReloadDB4MngBrowserSecExc)
		FUNCTION_0("ReloadDB4MngBrowserSecSpec", ReloadDB4MngBrowserSecSpec)
	END_FUNCTION_MAP

	json::value GetDBPath();
	json::value GetReportDBPath();
	json::value OnSetRegVal(SCITER_VALUE svbToggleState);
	json::value OnReloadBrowserSecurity(SCITER_VALUE svbToggleState);
	json::value OnReloadBrowserSecRules();
	json::value ReloadDB4MngBrowserSecExc();
	json::value ReloadDB4MngBrowserSecSpec();
	json::value On_GetSetRegistryVal();

public:
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);
	BOOL WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue);
	bool SendData2Service(DWORD dwMsg, bool bWait = false);
};

