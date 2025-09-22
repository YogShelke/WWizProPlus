/**********************************************************************************************************
*		Program Name          : SAVAPIScanner.h
*		Description           : This file contains implementation of SAVAPI
*		Author Name			  : Ramkrushna Shelke
*		Date Of Creation      : 29 JUN 2018
*		Version No            : 3.1.0.30
***********************************************************************************************************/
#pragma once
#include "savapi.h"
#include "CSavapi.h"

class CSAVAPIScanner
{
public:
	CSAVAPIScanner();
	virtual ~CSAVAPIScanner();
	bool SAVAPI_initialize();
	bool SAVAPI_ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID);
	bool SAVAPI_uninitialize();
	bool SAVAPI_RepairFile(LPCTSTR szFilePath, DWORD dwSpyID);
private:
	CSavapi scanner_;
	DWORD	m_dwPolyScannerFlag;
};

