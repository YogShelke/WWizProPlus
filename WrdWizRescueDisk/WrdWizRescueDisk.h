/*********************************************************************
*  Program Name		: WrdWizRescueDisk.h
*  Description		: CWrdWizRescueDiskApp Implementation.
*  Author Name		: NITIN SHELAR
*  Date Of Creation	: 26 feb 2019
**********************************************************************/
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"
#include "WardwizOSVersion.h"

/***************************************************************************************************
*  Class Name	  : CWrdWizRescueDiskApp
*  Description    : CWrdWizRescueDiskApp class implementation.
*  Author Name    :	NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
class CWrdWizRescueDiskApp : public CWinApp
{
public:
	CWrdWizRescueDiskApp();
	~CWrdWizRescueDiskApp();
// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

public:
	CWardwizLangManager		m_objwardwizLangManager;

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

extern CWrdWizRescueDiskApp theApp;
