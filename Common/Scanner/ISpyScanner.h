/**********************************************************************************************************                     
	  Program Name          : ISpyScanner.h
	  Description           : Header file Ispy scanner implementation
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 20 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna           Created Wrapper class for iSpy AV functionality implementation
***********************************************************************************************************/
#pragma once
#include "stdafx.h"
#include "ScannerContants.h"
#include "ISpyCriticalSection.h"
#include "BDClient.h"

class CISpyScanner
{
public:
	CISpyScanner(void);
	virtual ~CISpyScanner(void);
	bool LoadRequiredModules();
	bool ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID,DWORD &dwSignatureFailedToLoad, bool bRescan = false);
	bool RepairFile(LPCTSTR szThreatPath, DWORD dwISpyID);
	DWORD LoadSignatureDatabase(DWORD &dwSigCount);
	DWORD UnLoadSignatureDatabase();
	bool ReloadSettingsForScanDLL();
	bool Replace2OriginalFile(LPCTSTR szThreatPath, LPCTSTR szNewThreatPath);
public:
	HMODULE						m_hModuleISpyScanDLL;
	HMODULE						m_hModuleISpyRepairDLL;
	LOADSIGNATURE				m_lpLoadSigProc;
	UNLOADSIGNATURES			m_lpUnLoadSigProc;
	RELOADSETTINGS				m_lpReadSettingsProc;
	SCANFILE					m_lpScanFileProc;
	REPAIRFILE					m_lpRepairFileProc;
	bool						m_bSignatureLoaded;
	bool						m_bRescan;
	CISpyCriticalSection		m_objcsScanLoad;
	CISpyCriticalSection		m_objcsScanFile;
	CBDClient					m_objBDclient;
};

