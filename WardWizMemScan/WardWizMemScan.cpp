 
/*********************************************************************
*  Program Name	   : WardWizMemScan.cpp
*  Description	   : Defines the class behaviors for the application.
*  Author Name	   : Nitin Shelar
*  Date Of Creation: 21 Nov 2018
**********************************************************************/
#include "stdafx.h"
#include "WardWizMemScan.h"
#include "WardWizMemScanDlg.h"
#include "CSecure64.h"
#include "CScannerLoad.h"
#include "DriverConstants.h"
#include "WrdWizSystemInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWardWizMemScanApp

BEGIN_MESSAGE_MAP(CWardWizMemScanApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

/*********************************************************************
*  Function Name   : CWardWizMemScanApp
*  Description	   : constructor
*  Author Name	   : Nitin Shelar
*  Date Of Creation: 21 Nov 2018
**********************************************************************/
CWardWizMemScanApp::CWardWizMemScanApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	m_hResDLL = NULL;
	m_dwProductID = 1;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}
/***************************************************************************
Function Name  : ~CWardWizMemScanApp
Description    : Destructor
Author Name    : Nitin Shelar
Date           : 21 Nov 2018
****************************************************************************/
CWardWizMemScanApp::~CWardWizMemScanApp()
{
	if (m_hResDLL != NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
	if (m_hRegistrationDLL != NULL)
	{
		FreeLibrary(m_hRegistrationDLL);
		m_hRegistrationDLL = NULL;
	}
	if (m_hMutexHandle != NULL)
	{
		CloseHandle(m_hMutexHandle);
		m_hMutexHandle = NULL;
	}
}
// The one and only CWardWizMemScanApp object

CWardWizMemScanApp theApp;

/***********************************************************************************************
*  Function Name   : InitInstance
*  Description     : Windows allows several copies of the same program to run at the same time.
*  Author Name	   : Nitin Shelar                                                                                                *
*  Date Of Creation: 21 Nov 2018
*************************************************************************************************/
BOOL CWardWizMemScanApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	LoadResourceDLL(); 
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();

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

	CWardWizMemScanDlg dlg;
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
/***************************************************************************************************
*  Function Name   : LoadResourceDLL
*  Description     : Loading resource dll
*  Author Name	   : Nitin Shelar
*  Date Of Creation: 21 Nov 2018
****************************************************************************************************/
void CWardWizMemScanApp::LoadResourceDLL()
{
	CString	csWardWizModulePath = GetModuleFilePath();

	CString	csWardWizResourceDLL = L"";
	csWardWizResourceDLL.Format(L"%s\\VBRESOURCE.DLL", csWardWizModulePath);
	if (!PathFileExists(csWardWizResourceDLL))
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCE_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if (!m_hResDLL)
	{
		m_hResDLL = LoadLibrary(csWardWizResourceDLL);
		if (!m_hResDLL)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCE_MODULE_LOAD_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}

}
/***************************************************************************
Function Name   : GetModuleFilePath
Description     : Function which get the current Application path.
Author Name	    : Nitin Shelar
Date Of Creation: 21 Nov 2018
****************************************************************************/
CString CWardWizMemScanApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}
