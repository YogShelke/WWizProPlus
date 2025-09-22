/**********************************************************************************************************
Program Name          : WardWiz Indexing DLL
Description           : This DLL application is to make file indexing DB.
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 05th Apr 2016
Version No            : 1.14.0.12
Special Logic Used    : This DLL has export function to calculate CRC from buffer and find in local Indexing
database, if present no need to scan file.
Modification Log      :
***********************************************************************************************************/
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "stdafx.h"
#include "WWizBalBst.h"

typedef std::map<TCHAR, CWWizBalBst<UINT64>	*> BALBSTMAP;

class CWWizIndexDLLApp : public CWinApp
{
public:
	CWWizIndexDLLApp();
	~CWWizIndexDLLApp();
// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	bool CheckWLStatus(PBYTE pbData, size_t len, UINT64 &uiCRCValue);
	bool InsertIntoWhiteDB(TCHAR pDrive, UINT64 &uiCRCValue);
	bool LoadLocalDatabase();
	bool SaveLocalDatabase();
	bool ClearDatabase();
public:
	CWWizBalBst<UINT64>	*m_pBalBst[50];
	BALBSTMAP			m_mapBalbst;
	DECLARE_MESSAGE_MAP()
};
