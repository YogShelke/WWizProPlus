/**********************************************************************************************************
*		Program Name          : SAVAPIClient.h
*		Description           : This file contains implementation of SAVAPI
*		Author Name			  : Ramkrushna Shelke
*		Date Of Creation      : 29 JUN 2018
*		Version No            : 3.1.0.30
***********************************************************************************************************/
#pragma once
#include "stdafx.h"
#include "ITinRegWrapper.h"

class CSAVAPIClient
{
public:
	CSAVAPIClient();
	virtual ~CSAVAPIClient();
	bool SAVAPI_initialize();
	bool SAVAPI_ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID);
	bool SAVAPI_uninitialize();
	bool SAVAPI_RepairFile(LPCTSTR szFilePath, DWORD dwSpyID);
	bool LoadRequiredModules();
	bool ReadHeuristicScanStatus();
private:
	HMODULE				m_hModulePolyScanDLL;
	SCANFILE			m_lpScanFileProc;
	REPAIRFILE			m_lpRepairFileProc;
	SAVAPIINITIALIZE	m_lpSapiInitialize;
	SAVAPIUNINITIALIZE	m_lpSapiUnInitialize;
	DWORD				m_dwProductID;
	bool				m_bIsHeuScan;
	CITinRegWrapper		m_objReg;
};

