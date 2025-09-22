/**********************************************************************************************************
*		Program Name          : SAVAPIClient.cpp
*		Description           : This file contains implementation of SAVAPI
*		Author Name			  : Ramkrushna Shelke
*		Date Of Creation      : 29 JUN 2018
*		Version No            : 3.1.0.30
***********************************************************************************************************/
#include "stdafx.h"
#include "SAVAPIClient.h"


/***********************************************************************************************
Function Name  : CSAVAPIClient
Description    : Const'
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
CSAVAPIClient::CSAVAPIClient():
m_hModulePolyScanDLL(NULL)
, m_lpScanFileProc(NULL)
, m_lpRepairFileProc(NULL)
, m_lpSapiInitialize(NULL)
, m_lpSapiUnInitialize(NULL)
{
	CWardwizLangManager objwardwizLangManager;
	m_dwProductID = objwardwizLangManager.GetSelectedProductID();
}

/***********************************************************************************************
Function Name  : ~CSAVAPIClient
Description    : Dest'
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
CSAVAPIClient::~CSAVAPIClient()
{
	//Unload SAVAPI here
	if (m_lpSapiUnInitialize != NULL)
	{
		m_lpSapiUnInitialize();
	}

	if (m_hModulePolyScanDLL != NULL)
	{
		FreeLibrary(m_hModulePolyScanDLL);
		m_hModulePolyScanDLL = NULL;
	}
}

/***********************************************************************************************
Function Name  : SAVAPI_initialize
Description    : Function which intializes SAVAPI
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIClient::SAVAPI_initialize()
{
	bool bReturn = false;
	__try
	{
		if (m_hModulePolyScanDLL == NULL)
			LoadRequiredModules();

		if (m_lpSapiInitialize)
		{
			bReturn = m_lpSapiInitialize();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSAVAPIClient::SAVAPI_initialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : SAVAPI_ScanFile
Description    : Function which scan files using SAVAPI
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIClient::SAVAPI_ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID)
{
	bool bReturn = false;
	try
	{
		if (!szFilePath)
			return bReturn;

		if (m_lpScanFileProc)
		{
			TCHAR szSAVAPIVirusName[MAX_PATH] = { 0 };
			bReturn = m_lpScanFileProc(szFilePath, szSAVAPIVirusName, dwISpyID, false);
			CString csVirusName(szSAVAPIVirusName);
			csVirusName.Replace(L"/", L".");
			_tcscpy(szVirusName, csVirusName);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSAVAPIClient::SAVAPI_ScanFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : SAVAPI_uninitialize
Description    : Function which uninitialize SAVAPI
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIClient::SAVAPI_uninitialize()
{
	bool bReturn = false;
	__try
	{
		if (m_lpSapiUnInitialize)
		{
			bReturn = m_lpSapiUnInitialize();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSAVAPIClient::SAVAPI_uninitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : SAVAPI_RepairFile
Description    : Function Repairs file using SAVAPI
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIClient::SAVAPI_RepairFile(LPCTSTR szFilePath, DWORD dwSpyID)
{
	bool bReturn = false;
	__try
	{
		if (!szFilePath)
			return bReturn;

		if (m_lpRepairFileProc)
		{
			bReturn = m_lpRepairFileProc(szFilePath, dwSpyID);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSAVAPIClient::SAVAPI_RepairFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadRequiredModules
Description    : Function Loads required modules
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIClient::LoadRequiredModules()
{
	bool bReturn = false;
	__try
	{
		if (m_hModulePolyScanDLL)
		{
			return true;
		}

		if (!m_hModulePolyScanDLL)
		{
			m_hModulePolyScanDLL = LoadLibrary(L"VBPOLYSCN.DLL");
			if (!m_hModulePolyScanDLL)
			{
				AddLogEntry(L"### Error in Loading VBPOLYSCN.DLL", 0, 0, true, SECONDLEVEL);
				return false;
			}

			m_lpSapiInitialize = (SAVAPIINITIALIZE)GetProcAddress(m_hModulePolyScanDLL, "SAVAPIInitialize");
			if (!m_lpSapiInitialize)
			{
				AddLogEntry(L"### CSAVAPIClient: Error in GetProcAddress::SAVAPIInitialize", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModulePolyScanDLL);
				m_lpSapiInitialize = NULL;
				m_hModulePolyScanDLL = NULL;
				return false;
			}

			m_lpSapiUnInitialize = (SAVAPIUNINITIALIZE)GetProcAddress(m_hModulePolyScanDLL, "SAVAPIUninitialize");
			if (!m_lpSapiUnInitialize)
			{
				AddLogEntry(L"### CSAVAPIClient: Error in GetProcAddress::SAVAPIUninitialize", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModulePolyScanDLL);
				m_hModulePolyScanDLL = NULL;
				m_lpSapiUnInitialize = NULL;
				return false;
			}

			m_lpScanFileProc = (SCANFILE)GetProcAddress(m_hModulePolyScanDLL, "SAVAPIScanFile");
			if (!m_lpScanFileProc)
			{
				AddLogEntry(L"### CSAVAPIClient: Error in GetProcAddress::SAVAPIScanFile", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModulePolyScanDLL);
				m_hModulePolyScanDLL = NULL;
				m_lpScanFileProc = NULL;
				return false;
			}

			m_lpRepairFileProc = (REPAIRFILE)GetProcAddress(m_hModulePolyScanDLL, "SAVAPIRepairFile");
			if (!m_lpRepairFileProc)
			{
				AddLogEntry(L"### CSAVAPIClient: Error in GetProcAddress::SAVAPIRepairFile", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModulePolyScanDLL);
				m_hModulePolyScanDLL = NULL;
				m_lpRepairFileProc = NULL;
				return false;
			}
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSAVAPIClient::SAVAPI_RepairFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ReadHeuristicScanStatus
*  Description    : Function to get Registry for Heuristic Scan.
*  Author Name    : Ram Shelke
*  Date			  :	15 Sep 2016
****************************************************************************************************/
bool CSAVAPIClient::ReadHeuristicScanStatus()
{
	try
	{
		//Get here registry setting for Heuristic Scan.
		DWORD dwIsHeuScan = 0x01;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwHeuScan", dwIsHeuScan) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwHeuScan in ReadHeuristicScanStatus", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (dwIsHeuScan == 0x00)
		{
			m_bIsHeuScan = false;
		}
		else if (dwIsHeuScan == 0x01)
		{
			m_bIsHeuScan = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed in CBDClient::ReadHeuristicScanStatus", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}