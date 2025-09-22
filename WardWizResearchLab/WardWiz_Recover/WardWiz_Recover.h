
// WardWiz_Recover.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWardWiz_RecoverApp:
// See WardWiz_Recover.cpp for the implementation of this class
//

class CWardWiz_RecoverApp : public CWinApp
{
public:
	CWardWiz_RecoverApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWardWiz_RecoverApp theApp;