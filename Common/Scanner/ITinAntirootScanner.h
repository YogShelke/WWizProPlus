#pragma once
#include "stdafx.h"
#include "ISpyScanner.h"

class CITinAntirootKitScanner
{
public:
	CITinAntirootKitScanner(void);
	virtual ~CITinAntirootKitScanner(void);

public:
	HMODULE				m_hAntirootScanDLL ;
	typedef bool(*STARTANTIROOTSCAN)(DWORD) ;
	typedef bool(*STOPANTIROOTSCAN)() ;
	typedef bool(*PAUSERESUME)();
	typedef DWORD(*GETDWORDVALUES)();
	typedef DWORD(*REPAIRPROCESS)(DWORD , TCHAR*);
	typedef DWORD(*REPAIRDRIVER)(TCHAR*, TCHAR*);
	typedef void (*SETSCANFUNCTIONPTR)(CISpyScanner *pobjISpyScanner);

STARTANTIROOTSCAN	 m_StartAntirootscan;
STOPANTIROOTSCAN	 m_StopAntirootScan;
GETDWORDVALUES		 m_GetPercentage,m_GetdetectedEntriesofDrivers,m_GetdetectedEntriesofHiddenProcesses,m_GetdtectedEntriesofHiddenFileorFolder;
PAUSERESUME			 m_PauseAntirootKit,m_ResumeAntirootKit;
REPAIRPROCESS        m_RepairProcess;
REPAIRDRIVER		 m_RepairDriver;
SETSCANFUNCTIONPTR	 m_pSetFunctionPtr;

public:
	bool UnloadLibrary();
	bool LoadLibraryOfAntirootkit();
	bool StartAntirootScan(SCANTYPE eScanType,DWORD dwAntirootScanOpt, CISpyScanner * pobjScanner);
	bool PauseAntirootScan();
	bool ResumeAntirootScan();
	bool StopAntirootScan();
	//bool HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, DWORD dwISpyID);

	//Signature Changed by Vilas on 07 April 2015 to handle failing cases
	DWORD HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, DWORD dwISpyID);
};