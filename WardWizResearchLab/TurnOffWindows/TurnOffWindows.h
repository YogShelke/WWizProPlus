// TurnOffWindows.h : main header file for the TURNOFFWINDOWS application
//

#if !defined(AFX_TURNOFFWINDOWS_H__25D2F7B8_84EA_4E45_B9CB_2B45E383FDEE__INCLUDED_)
#define AFX_TURNOFFWINDOWS_H__25D2F7B8_84EA_4E45_B9CB_2B45E383FDEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTurnOffWindowsApp:
// See TurnOffWindows.cpp for the implementation of this class
//

class CTurnOffWindowsApp : public CWinApp
{
public:
	CTurnOffWindowsApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTurnOffWindowsApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTurnOffWindowsApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TURNOFFWINDOWS_H__25D2F7B8_84EA_4E45_B9CB_2B45E383FDEE__INCLUDED_)
