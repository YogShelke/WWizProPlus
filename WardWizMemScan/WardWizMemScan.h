
// WardWizMemScan.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "WardwizLangManager.h"
#include "EnumProcess.h"
#include "WardWizCrashHandler.h"
#include "iTINRegWrapper.h"
#include "ScannerContants.h"
#include "AVRegInfo.h"

// CWardWizMemScanApp:
// See WardWizMemScan.cpp for the implementation of this class
//

class CWardWizMemScanApp : public CWinApp
{
public:
	CWardWizMemScanApp();
	~CWardWizMemScanApp();

// Overrides
public:
	virtual BOOL InitInstance();
	HMODULE					m_hResDLL;
	HANDLE					m_hMutex;
	CWardwizLangManager		m_objwardwizLangManager;
	DWORD					m_dwProductID;
	HMODULE					m_hRegistrationDLL;
	HANDLE					m_hMutexHandle;
	CString					m_csRegKeyPath;
	SCANLEVEL				m_eScanLevel;
	bool					m_bIsScanning;
public:
	void LoadResourceDLL();
	CString GetModuleFilePath();
// Implementation

	CEvent					m_objCompleteEvent;

	SCITER_VALUE			m_iRetval;
	SCITER_VALUE			m_bRetval;

	DECLARE_MESSAGE_MAP()
};

extern CWardWizMemScanApp theApp;