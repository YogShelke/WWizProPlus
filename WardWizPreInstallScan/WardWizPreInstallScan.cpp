/*********************************************************************
*  Program Name	   : WardWizPreInstallScan.cpp
*  Description	   : Defines the class behaviors for the application
*  Author Name	   : Nitin Shelar
*  Date Of Creation: 21 Nov 2018
**********************************************************************/

#include "stdafx.h"
#include "WardWizPreInstallScan.h"
#include "WardWizPreInstallScanDlg.h"
#include "CScannerLoad.h"
#include "CSecure64.h"
#include "DriverConstants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWardWizPreInstallScanApp

BEGIN_MESSAGE_MAP(CWardWizPreInstallScanApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

/*********************************************************************
*  Function Name   : CWardWizPreInstallScanApp
*  Description	   : constructor
*  Author Name	   : Nitin Shelar
*  Date Of Creation: 21 Nov 2018
**********************************************************************/
CWardWizPreInstallScanApp::CWardWizPreInstallScanApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	m_dwProductID = 1;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWardWizPreInstallScanApp object

CWardWizPreInstallScanApp theApp;


// CWardWizPreInstallScanApp initialization
/***********************************************************************************************
*  Function Name   : InitInstance
*  Description     : Windows allows several copies of the same program to run at the same time.
*  Author Name	   : Nitin Shelar                                                                                                *
*  Date Of Creation: 21 Nov 2018
*************************************************************************************************/
BOOL CWardWizPreInstallScanApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	LoadResourceDLL();
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();
	m_dwProductID = GetSelectedProductID();
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

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_FIFTEEN);//WLSRV_ID_FIFTEEN to register service for process protection

	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_FIFTEEN);//WLSRV_ID_FIFTEEN to register service for process protection

	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

	CWardWizPreInstallScanDlg dlg;
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
void CWardWizPreInstallScanApp::LoadResourceDLL()
{
	CString	csWardWizModulePath = GetModuleFilePath();
	try
	{
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
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumPreInstallScanApp::GetModuleFilePath", 0, 0, true, SECONDLEVEL);
	}


}
/***************************************************************************
Function Name   : GetModuleFilePath
Description     : Function which get the current Application path.
Author Name	    : Nitin Shelar
Date Of Creation: 21 Nov 2018
****************************************************************************/
CString CWardWizPreInstallScanApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	try
	{
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumPreInstallScanApp::GetModuleFilePath", 0, 0, true, SECONDLEVEL);
	}
	return(CString(szModulePath));
}

/***************************************************************************
Function Name  : GetSelectedProductEdition
Description    : Function returns DWORD value
				1 - ESSENTIAL
				2 - PRO
				3 - ELITE
				4 - BASIC
Author Name    : NITIN SHELAR
Date           : 27 DEC 2018
****************************************************************************/
DWORD CWardWizPreInstallScanApp::GetSelectedProductID()
{
	CString csIniFilePath = GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
	try
	{
		if (!PathFileExists(csIniFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetSelectedProductID", csIniFilePath, 0, true, SECONDLEVEL);
			return 0xFFFF;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumPreInstallScanApp::GetSelectedProductID", 0, 0, true, SECONDLEVEL);
	}
	return GetPrivateProfileInt(L"VBSETTINGS", L"ProductID", 1, csIniFilePath);
}