
// WardWiz_Exec_Block.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWardWiz_Exec_BlockApp:
// See WardWiz_Exec_Block.cpp for the implementation of this class
//

class CWardWiz_Exec_BlockApp : public CWinApp
{
public:
	CWardWiz_Exec_BlockApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWardWiz_Exec_BlockApp theApp;