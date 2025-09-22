
// WrdWizProcessExplorer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWrdWizProcessExplorerApp:
// See WrdWizProcessExplorer.cpp for the implementation of this class
//

class CWrdWizProcessExplorerApp : public CWinApp
{
public:
	CWrdWizProcessExplorerApp();
	~CWrdWizProcessExplorerApp(void);

// Overrides
public:
	virtual BOOL InitInstance();

	HMODULE					m_hResDLL;
	HANDLE					m_hMutex;
	CWardwizLangManager		m_objwardwizLangManager;
	DWORD					m_dwSelectedLangID;
	DWORD					m_dwProdID;

	// Implementation

	DECLARE_MESSAGE_MAP()

public:
	bool SingleInstanceCheck();
	void LoadResourceDLL();
	CString GetModuleFilePath();

public:
	CEvent						m_objCompleteEvent;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;
};

extern CWrdWizProcessExplorerApp theApp;