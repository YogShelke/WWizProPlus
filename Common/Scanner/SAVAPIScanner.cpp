/**********************************************************************************************************
*		Program Name          : SAVAPIScanner.cpp
*		Description           : This file contains implementation of SAVAPI
*		Author Name			  : Ramkrushna Shelke
*		Date Of Creation      : 29 JUN 2018
*		Version No            : 3.1.0.30
***********************************************************************************************************/
#include "stdafx.h"
#include "SAVAPIScanner.h"

/***********************************************************************************************
Function Name  : CSAVAPIScanner
Description    : Const'
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
CSAVAPIScanner::CSAVAPIScanner():
m_dwPolyScannerFlag(0x01)
{
	DWORD dwPolyScanner = 0x01;
	CITinRegWrapper	 objReg;
	objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwPolyScanner", dwPolyScanner);
	//if polyscanner is disabled then return.
	if (dwPolyScanner == 0x00)
	{
		m_dwPolyScannerFlag = 0x00;
	}
}

/***********************************************************************************************
Function Name  : ~CSAVAPIScanner
Description    : Dest'
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
CSAVAPIScanner::~CSAVAPIScanner()
{
}

/***********************************************************************************************
Function Name  : SAVAPI_initialize
Description    : Function which intializes SAVAPI
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIScanner::SAVAPI_initialize()
{
	bool bReturn = false;

	if (!m_dwPolyScannerFlag)
		return bReturn;
	
	SAVAPI_STATUS ret_code = SAVAPI_S_OK;
	ret_code = scanner_.StartSAVAPI();

	if (ret_code != SAVAPI_S_OK && ret_code != SAVAPI_E_APC_ALREADY_INITIALIZED)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Failed to StartSAVAPI, ErrorCode: %d", ret_code);
		return false;
	}

	ret_code = scanner_.SetOption(SAVAPI_OPTION_PRODUCT, L"11804");
	if (ret_code != SAVAPI_S_OK)
	{
		ret_code = scanner_.SetOption(SAVAPI_OPTION_PRODUCT, L"11842");
		if (ret_code != SAVAPI_S_OK)
		{
			AddLogEntryEx(SECONDLEVEL, _T("SAVAPI licensing failed with code: %d"), ret_code);
			return bReturn;
		}
	}

	ret_code = scanner_.SetOption(SAVAPI_OPTION_ARCHIVE_SCAN, _T("0"));
	if (ret_code != SAVAPI_S_OK)
	{
		AddLogEntryEx(SECONDLEVEL, _T("SAVAPI_initialize Setting option to scan archives failed with code: %d"), ret_code);
		return bReturn;
	}

	/* Temporary OFF till AVIRA SOLVE apc_random_id issue
	ret_code = scanner_.SetOption(SAVAPI_OPTION_FPC, _T("1"));
	if (ret_code != SAVAPI_S_OK)
	{
		AddLogEntryEx(SECONDLEVEL, _T("SAVAPI_initialize Setting option to scan with FPC failed with code: %d"), ret_code);
		return bReturn;
	}
	*/

	bReturn = true;

	return bReturn;
}

/***********************************************************************************************
Function Name  : SAVAPI_ScanFile
Description    : Function which scan files using SAVAPI
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIScanner::SAVAPI_ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID)
{
	bool bReturn = false;
	try
	{
		if (!m_dwPolyScannerFlag)
			return bReturn;

		SAVAPI_STATUS ret_code;
		ret_code = scanner_.SetOption(SAVAPI_OPTION_REPAIR, _T("0"));
		if (ret_code == SAVAPI_E_NOT_INITIALIZED)
		{
			SAVAPI_initialize();
		}

		ret_code = scanner_.SetOption(SAVAPI_OPTION_REPAIR, _T("0"));
		if (ret_code != SAVAPI_S_OK)
		{
			AddLogEntryEx(ZEROLEVEL, _T("SAVAPI_ScanFile Setting option to scan with FPC failed with code: %d"), ret_code);
			return bReturn;
		}

		ret_code = scanner_.ScanFile(szFilePath);

		// Insert scan result
		if (!scanner_.scan_ErrorReport.IsEmpty())
		{
			AddLogEntryEx(SECONDLEVEL, _T("### Error [%s] in scanning File:[%s]"), scanner_.scan_ErrorReport, szFilePath);
		}

		if (scanner_.m_bFound)
		{
			_tcscpy(szVirusName, scanner_.m_csThreatName);
			dwISpyID = 0x00;
			if (scanner_.m_bISRepairable)
			{
				dwISpyID = 0xFFFFF;
			}
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, _T("### Exception in CSAVAPIScanner::SAVAPI_ScanFile FilePath:[%s]"), szFilePath);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : SAVAPI_RepairFile
Description    : Function Repairs file using SAVAPI
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIScanner::SAVAPI_RepairFile(LPCTSTR szFilePath, DWORD dwSpyID)
{
	bool bReturn = false;
	try
	{
		if (!m_dwPolyScannerFlag)
			return bReturn;

		if (dwSpyID != 0xFFFFF)
			return bReturn;

		SAVAPI_STATUS ret_code;
		ret_code = scanner_.SetOption(SAVAPI_OPTION_REPAIR, _T("1"));
		if (ret_code == SAVAPI_E_NOT_INITIALIZED)
		{
			SAVAPI_initialize();
		}

		if (ret_code != SAVAPI_S_OK)
		{
			AddLogEntryEx(SECONDLEVEL, _T("### Setting option to repair failed with code: %d"), ret_code);
			return bReturn;
		}

		ret_code = scanner_.SetOption(SAVAPI_OPTION_NOTIFY_REPAIR, _T("0"));
		if (ret_code != SAVAPI_S_OK)
		{
			AddLogEntryEx(SECONDLEVEL, _T("### Setting option to notify repair failed with code: %d"), ret_code);
			return bReturn;
		}

		ret_code = scanner_.ScanFile(szFilePath);
		if (ret_code == SAVAPI_S_OK)
		{
			// Insert scan result
			if (scanner_.scan_ErrorReport.IsEmpty())
			{
				return true;
			}
		}

		// Insert scan result
		if (!scanner_.scan_ErrorReport.IsEmpty())
		{
			AddLogEntryEx(SECONDLEVEL, _T("### Error [%s] in scanning File:[%s]"), scanner_.scan_ErrorReport, szFilePath);
		}
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, _T("### Exception in CSAVAPIScanner::SAVAPI_ScanFile FilePath:[%s]"), szFilePath);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : SAVAPI_uninitialize
Description    : Function which uninitialize SAVAPI
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CSAVAPIScanner::SAVAPI_uninitialize()
{
	bool bReturn = false;
	__try
	{
		if (!m_dwPolyScannerFlag)
			return bReturn;

		scanner_.StopSAVAPI();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntryEx(SECONDLEVEL, _T("### Exception in CSAVAPIScanner::SAVAPI_uninitialize"));
	}
	return bReturn;
}





