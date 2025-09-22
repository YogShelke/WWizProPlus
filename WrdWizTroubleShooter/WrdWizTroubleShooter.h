
// WrdWizTroubleShooter.h : main header file for the PROJECT_NAME application
//

#pragma once


#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizLangManager.h"

// CWrdWizTroubleShooterApp:
// See WrdWizTroubleShooter.cpp for the implementation of this class
//

class CWrdWizTroubleShooterApp : public CWinApp
{
public:
	CWrdWizTroubleShooterApp();

// Overrides
public:
	virtual BOOL InitInstance();
	CWardwizLangManager	m_objwardwizLangManager;
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWrdWizTroubleShooterApp theApp;