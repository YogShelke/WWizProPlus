
// WrdWizDownloader.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWrdWizDownloaderApp:
// See WrdWizDownloader.cpp for the implementation of this class
//

class CWrdWizDownloaderApp : public CWinApp
{
public:
	CWrdWizDownloaderApp();

// Overrides
public:
	virtual BOOL InitInstance();
	HANDLE	m_hMutexHandle;
	bool SingleInstanceCheck();

	CString m_csurl;
	CString m_csFilePath;
	CString GetString(UINT uiResourceID);
	CString GetStringFromStringTable(UINT uiResourceID);
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWrdWizDownloaderApp theApp;