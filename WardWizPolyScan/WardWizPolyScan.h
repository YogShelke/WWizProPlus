// WardWizPolyScan.h : main header file for the WardWizPolyScan DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "SAVAPIScanner.h"		

// CWardWizPolyScanApp
// See WardWizPolyScan.cpp for the implementation of this class
//

class CWardWizPolyScanApp : public CWinApp
{
public:
	CWardWizPolyScanApp();

// Overrides
public:
	virtual BOOL InitInstance();
	bool SAVAPIInitialize();
	bool SAVAPIUninitialize();
	bool SAVAPIScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID);
	bool SAVAPIRepairFile(LPCTSTR pszFilePath, DWORD dwSpyID);
public:
	CSAVAPIScanner		m_objSAVAPIScanner;
	DECLARE_MESSAGE_MAP()
};
