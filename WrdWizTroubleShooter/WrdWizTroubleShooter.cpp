
// WrdWizTroubleShooter.cpp : Defines the class behaviors for the application.
//
/***************************************************************
*  Program Name: WrdWizTroubleShooter.cpp
*  Description: App class of WardWiz troubleshooter exe.
*  Author Name: Neha Gharge
*  Date Of Creation: 31st Oct,2015
*  Version No: 1.12.0.0
*****************************************************************/

#include "stdafx.h"
#include "WrdWizTroubleShooter.h"
#include "WrdWizTroubleShooterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 31/10/2015
****************************************************************************************************/

BEGIN_MESSAGE_MAP(CWrdWizTroubleShooterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWrdWizTroubleShooterApp construction
/***************************************************************************************************
*  Function Name  : CWrdWizTroubleShooterApp
*  Description    : C'tor
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 31/10/2015
****************************************************************************************************/

CWrdWizTroubleShooterApp::CWrdWizTroubleShooterApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWrdWizTroubleShooterApp object

CWrdWizTroubleShooterApp theApp;


// CWrdWizTroubleShooterApp initialization
/***************************************************************************************************
*  Function Name  : InitInstance
*  Description    :  Windows allows several copies of the same program to run at the same time.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 31/10/2015
****************************************************************************************************/
BOOL CWrdWizTroubleShooterApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CWrdWizTroubleShooterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

