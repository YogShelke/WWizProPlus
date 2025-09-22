/****************************************************
*  Program Name: ISpyRepairDLL.h                                                                                                   
*  Author Name: Prajakta                                                                                                      
*  Date Of Creation: 7 Dec 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/

#pragma once
#include "ISpyRepair.h"
#include "WrdwizEncDecManager.h"
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


//CLASS & FUNCTIONS DECLARATION
class CISpyRepairDLLApp : public CWinApp
{
public:
	CISpyRepairDLLApp();
	CISpyRepair					m_objISpyRepair;
	CWrdwizEncDecManager        m_objWrdwizEncDecManager;
	CString						m_csTmpRepairDBFileName;
	bool						m_bDBLoaded;
// Overrides
public:
	virtual BOOL InitInstance();
	bool RepairFile(LPCTSTR szFilePath, DWORD dwSpyID);
	bool LoadISpyRepairDB();
	bool ParseExpression(CString csLine);
	bool FetchRepairExpr(DWORD dwSpyID);
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};
