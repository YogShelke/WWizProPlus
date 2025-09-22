#pragma once

#ifdef __windows__
#undef __windows__
#endif
#include <afxcoll.h>
#include <iostream>
#include <string>
#include <psapi.h>
#include "CommonFunctions.h"
#include "WardwizScanner.h"
#include "WardwizSettingsUpdater.h"

class CWardwizServiceInvoker
{
public:
	int						m_iTotalFileCount;
	int						m_iThreatsFoundCount;
	CString					m_csVirusName;
	bool					m_bThreatDetected;

	CWardwizServiceInvoker();
	~CWardwizServiceInvoker();

	// Gets a list of processes and returns process count.
	int GetRunningProcessList(CStringArray &csaProcessList);

	// Gets a list of attached devices and returns device count.
	int GetAttachedDeviceList(const std::string &refcstrRootDirectory, const std::string &refcstrExtension);
	DWORD UpdateUSBPORTsPolicy(BOOL bEnableDisable);
	DWORD UpdateCDROMsPolicy(BOOL bEnableDisable);
	bool  InvokeScanFromService(CStringArray &csaAllScanPaths, SCANTYPE eScanType);
	bool  InvokeSettingsUpdaterFromService();

private:
	void SetProcessNameAndID(DWORD processID, CString &);
	CStringArray csaProcessNameId;
};

