// WWizHash.h : main header file for the WWizHash DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWWizHashApp
// See WWizHash.cpp for the implementation of this class
//

class CWWizHashApp : public CWinApp
{
public:
	CWWizHashApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
