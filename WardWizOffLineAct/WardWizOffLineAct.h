// WardWizOffLineAct.h : main header file for the WardWizOffLineAct DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWardWizOffLineActApp
// See WardWizOffLineAct.cpp for the implementation of this class
//

class CWardWizOffLineActApp : public CWinApp
{
public:
	CWardWizOffLineActApp();

	BYTE	m_sz2MIDBytes[0x04];
	BYTE	m_bKey[0x08];

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
