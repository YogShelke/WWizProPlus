
/*********************************************************************
*  Program Name		: WrdWizRescueDisk.cpp
*  Description		: CWrdWizRescueDiskApp Implementation.
*  Author Name		: NITIN SHELAR
*  Date Of Creation	: 12/02/2019
**********************************************************************/

#include "stdafx.h"
#include "WrdWizRescueDisk.h"
#include "WrdWizRescueDiskDlg.h"
#include "CScannerLoad.h"
#include "DriverConstants.h"
#include "CSecure64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWrdWizRescueDiskApp

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : NITIN SHELAR
*  Date           : 11/02/2019
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWrdWizRescueDiskApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

/*********************************************************************
*  Function Name   : CWrdWizRescueDiskApp
*  Description	   : constructor
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 11/02/2019
**********************************************************************/
CWrdWizRescueDiskApp::CWrdWizRescueDiskApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	m_hResDLL = NULL;
	m_dwProdID = 1;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/***************************************************************************
Function Name  : ~CWrdWizRescueDiskApp
Description    : Destructor
Author Name    : NITIN SHELAR
Date           : 11/02/2019
****************************************************************************/
CWrdWizRescueDiskApp::~CWrdWizRescueDiskApp()
{
	if (m_hResDLL != NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
}

// The one and only CWrdWizRescueDiskApp object
CWrdWizRescueDiskApp theApp;

/***********************************************************************************************
*  Function Name   : InitInstance
*  Description     : Windows allows several copies of the same program to run at the same time.
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 11/02/2019
*************************************************************************************************/
BOOL CWrdWizRescueDiskApp::InitInstance()
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
	m_dwProdID = theApp.m_objwardwizLangManager.GetSelectedProductID();
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
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	
	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_SIXTEEN);
	
	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_SIXTEEN);//WLSRV_ID_FIFTEEN to register service for process protection

	if (SingleInstanceCheck())
	{
		HWND hWindow = ::FindWindow(NULL, L"VibraniumRESCUDISK");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			AddLogEntry(L"Wardwiz rescue disk is already running.", 0, 0, true, ZEROLEVEL);
			return FALSE;
		}
		return FALSE;
	}

	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);
	CWrdWizRescueDiskDlg dlg;
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
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
CString CWrdWizRescueDiskApp::GetModuleFilePath()
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
		AddLogEntry(L"### Exception in CVibraniumRescueDiskApp::GetModuleFilePath()", 0, 0, true, ZEROLEVEL);
	}
	return(CString(szModulePath));
}

/***************************************************************************************************
*  Function Name   : LoadResourceDLL
*  Description     : Loading resource dll
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 11/02/2019
****************************************************************************************************/
void CWrdWizRescueDiskApp::LoadResourceDLL()
{
	try
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
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskApp::LoadResourceDLL()", 0, 0, true, ZEROLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SingleInstanceCheck()
*  Description    : Check whether wardwiz Rescue Disk instance is exist or not
*  Author Name    : NITIN SHELAR
*  SR_NO
*  Date           : 26/02/2019
****************************************************************************************************/
bool CWrdWizRescueDiskApp::SingleInstanceCheck()
{
	try
	{
		m_hMutex = CreateMutex(NULL, TRUE, L"Global\\VibraniumRESCUDISK ");
		DWORD dwError = GetLastError();

		if (dwError == ERROR_ALREADY_EXISTS)
		{
			AddLogEntry(L"Second instace of VibraniumRESCUDISK ", 0, 0, false, ZEROLEVEL);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskApp::SingleInstanceCheck()", 0, 0, true, ZEROLEVEL);
	}
	return false;
}