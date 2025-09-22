// WWHeuScn.h : main header file for the WWHeuScn DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWWHeuScnApp
// See WWHeuScn.cpp for the implementation of this class
//

class CWWHeuScnApp : public CWinApp
{
public:
	CWWHeuScnApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
