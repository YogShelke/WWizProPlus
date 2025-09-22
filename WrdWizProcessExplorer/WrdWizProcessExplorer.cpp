
/*********************************************************************
*  Program Name:	CWrdWizProcessExplorer.cpp
*  Description:		CWrdWizProcessExplorer Implementation
*  Author Name:		Kunal Waghmare
*  Date Of Creation: 17th October 2018
**********************************************************************/

#include "stdafx.h"
#include "WrdWizProcessExplorer.h"
#include "WrdWizProcessExplorerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWrdWizProcessExplorerApp

BEGIN_MESSAGE_MAP(CWrdWizProcessExplorerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CWrdWizProcessExplorerApp construction

CWrdWizProcessExplorerApp::CWrdWizProcessExplorerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	m_hResDLL = NULL;
	m_dwProdID = 0x00;
	m_dwSelectedLangID = 0;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CWrdWizProcessExplorerApp object

CWrdWizProcessExplorerApp theApp;

CWrdWizProcessExplorerApp::~CWrdWizProcessExplorerApp(void)
{

	if (m_hResDLL != NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
}

// CWrdWizProcessExplorerApp initialization

BOOL CWrdWizProcessExplorerApp::InitInstance()
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

	CWrdWizProcessExplorerDlg dlg;

	m_dwSelectedLangID = m_objwardwizLangManager.GetSelectedLanguage();
	m_dwProdID = m_objwardwizLangManager.GetSelectedProductID();

	LoadResourceDLL();

	//Check for single instance 
	//Neha Gharge 7 July,2015
	if (SingleInstanceCheck())
	{
		HWND hWindow = ::FindWindow(NULL, L"WRDWIZPROCESSEXPLORER");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			AddLogEntry(L"Wardwiz Process Explorer is already running.", 0, 0, true, SECONDLEVEL);
			return FALSE;
		}
		return FALSE;
	}

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

bool CWrdWizProcessExplorerApp::SingleInstanceCheck()
{
	m_hMutex = CreateMutex(NULL, TRUE, L"Global\\WRDWIZPROCESSEXPLORER");
	
	DWORD dwError = GetLastError();

	if (dwError == ERROR_ALREADY_EXISTS)
	{
		AddLogEntry(L"Second", 0, 0, false, SECONDLEVEL);
		return true;
	}

	return false;
}

void CWrdWizProcessExplorerApp::LoadResourceDLL()
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

CString CWrdWizProcessExplorerApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}