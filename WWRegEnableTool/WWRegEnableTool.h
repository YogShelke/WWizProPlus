/*********************************************************************
*  Program Name		: WWRegEnableToolApp.h
*  Description		: CWWRegEnableToolApp Implementation.
*  Author Name		: NITIN SHELAR
*  Date Of Creation	: 20/11/2019
**********************************************************************/
// WWRegEnableTool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWWRegEnableToolApp:
// See WWRegEnableTool.cpp for the implementation of this class

/***************************************************************************************************
*  Class Name	  : CWWRegEnableToolApp
*  Description    : CWWRegEnableToolApp class implementation.
*  Author Name    :	NITIN SHELAR
*  Date			  : 20/11/2019
****************************************************************************************************/
class CWWRegEnableToolApp : public CWinApp
{
public:
	CWWRegEnableToolApp();
	~CWWRegEnableToolApp();
// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

public:
	CWardwizLangManager			m_objwardwizLangManager;
	CEvent						m_objCompleteEvent;

	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;

	DWORD						m_dwProdID;
	HMODULE						m_hResDLL;
	HANDLE						m_hMutex;

	void LoadResourceDLL();
	CString GetModuleFilePath();
	bool SingleInstanceCheck();
};

extern CWWRegEnableToolApp theApp;