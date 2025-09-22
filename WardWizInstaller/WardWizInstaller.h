
// WardWizInstaller.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizOSVersion.h"
#include "WardwizLangManager.h"
#include "Enumprocess.h"
#include "WardWizCrashHandler.h"
#include "iTINRegWrapper.h"
#include "ScannerContants.h"

// CWardWizInstallerApp:
// See WardWizInstaller.cpp for the implementation of this class
//

class CWardWizInstallerApp : public CWinApp
{
public:
	CWardWizInstallerApp();
	~CWardWizInstallerApp();

	BYTE	m_sz2MIDBytes[0x04];
	BYTE	m_bKey[0x08];
// Overrides
public:
	
	HMODULE					m_hResDLL;
	CWardwizLangManager		m_objwardwizLangManager;

	CEvent						m_objCompleteEvent;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;

	virtual BOOL InitInstance();
	CString GetString(UINT uiResourceID);
	CString GetStringFromStringTable(UINT uiResourceID);
	CString GetModuleFilePath();
	void GetProdIdNVersion();
	void LoadResourceDLL();

	int							m_dwProdID;
	int							m_dwLangID;
	CString                     m_csFilePath;
	CString						m_csurl;
	CString						m_csProdVersion;
	bool						m_bIsScanning;
	bool						m_bQuickScan;
	CString                     g_csRegKeyPath;
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWardWizInstallerApp theApp;