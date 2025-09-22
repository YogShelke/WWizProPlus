// WardWizALUpd.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWardWizALUpdApp:
// See WardWizALUpd.cpp for the implementation of this class
//

class CWardWizALUpdApp : public CWinApp
{
public:
	CWardWizALUpdApp();
   bool m_AppIsRunningFromCmd ;
   int m_AppCmdBuildType;			//0 for build, 1 for rebuild
   int m_ichkBuildType;				//0 for BuildType unchecked & 1 for BuildType checked
// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWardWizALUpdApp theApp;