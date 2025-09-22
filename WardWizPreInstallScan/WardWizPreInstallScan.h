
// WardWizPreInstallScan.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWardWizPreInstallScanApp:
// See WardWizPreInstallScan.cpp for the implementation of this class
//

class CWardWizPreInstallScanApp : public CWinApp
{
public:
	CWardWizPreInstallScanApp();

// Overrides
public:
	virtual BOOL InitInstance();
	CWardwizLangManager		m_objwardwizLangManager;

	void LoadResourceDLL();
	CString GetModuleFilePath();
	DWORD GetSelectedProductID();

	HMODULE					m_hResDLL;

	DWORD					m_dwProductID;

	CEvent						m_objCompleteEvent;

	bool					m_bIsScanning;
	bool					m_bQuickScan;

	SCITER_VALUE			m_iRetval;
	SCITER_VALUE			m_bRetval;
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWardWizPreInstallScanApp theApp;