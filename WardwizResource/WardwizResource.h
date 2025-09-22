// WardwizResource.h : main header file for the WardwizResource DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWardwizResourceApp
// See WardwizResource.cpp for the implementation of this class
//

class CWardwizResourceApp : public CWinApp
{
public:
	CWardwizResourceApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
