#pragma once
//Contains Definitions for the Functions to be supported by the Adapter class.

#include <afxcoll.h>
#include "PipeConstants.h"

class IWardwizScanAdapter
{
public:
	// Forbid copying
	IWardwizScanAdapter(IWardwizScanAdapter const &) = delete;
	IWardwizScanAdapter & operator=(IWardwizScanAdapter const &) = delete;

	//virtual destructor.
	virtual ~IWardwizScanAdapter()=default;

	virtual BOOL StartScan(CStringArray &csaAllScanPaths) = 0;

	virtual BOOL PauseScan() = 0;
	
	virtual BOOL ResumeScan() = 0;

	virtual BOOL StopScan() = 0;
	
	virtual BOOL BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath) = 0;
	
	virtual BOOL QuarantineFiles() = 0;
	
	virtual BOOL QuarantineSelectedfile() = 0;
	
	virtual BOOL SearchForVirusAndQuarantine() = 0;
	
	virtual BOOL RecoverOperations(int dwMessageinfo, CString csRecoverFileEntry, DWORD dwType, bool bWait = false, bool bReconnect = false) = 0;
	
	virtual BOOL RepairFile(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect) = 0;
	
	virtual BOOL RepairFile(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait = false, bool bReconnect = false) = 0;

protected:
	// allow construction for child classes only
	IWardwizScanAdapter()=default;

};


