/**********************************************************************************************************
Program Name          : WardWizInstaller.cpp
Description           : App class For WardWiz Installer
Author Name			  : Tejas Shinde
Date Of Creation      : 4/25/2019
Version No            : 
Modification Log      :
***********************************************************************************************************/
// WardWizInstaller.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WardWizInstaller.h"
#include "WardWizInstallerDlg.h"
#include "CScannerLoad.h"
#include "CSecure64.h"
#include "CScannerLoad.h"
#include "DriverConstants.h"

#include "WrdWizSystemInfo.h"

#pragma warning(disable : 4995)  

#pragma comment(lib, "Advapi32.lib") 
#pragma comment(lib, "ole32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWardWizInstallerApp

BEGIN_MESSAGE_MAP(CWardWizInstallerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWardWizInstallerApp construction
/***********************************************************************************************
*  Function Name  : CWardWizInstallerApp
*  Description    : Constructor
*  Author Name    : Tejas Shinde
*  SR_NO		  :
*  Date           :  8 March 2019
*************************************************************************************************/
CWardWizInstallerApp::CWardWizInstallerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	m_dwLangID = 0;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/***************************************************************************
*   Function Name   : ~CWardWizInstallerApp
*   Description     : Dest'r
*   Author Name     : Tejas Shinde
*   Date            : 8 March 2019
****************************************************************************/
CWardWizInstallerApp::~CWardWizInstallerApp()
{
	// Place all significant initialization in InitInstance
}

// The one and only CWardWizInstallerApp object

CWardWizInstallerApp theApp;


// CWardWizInstallerApp initialization
/***************************************************************************
*   Function Name   : InitInstance
*   Description     : CWardWizInstallerApp Initialization
*   Author Name     : Tejas Shinde
*   Date            : 8 March 2019
****************************************************************************/
BOOL CWardWizInstallerApp::InitInstance()
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
	g_csRegKeyPath = L"SOFTWARE\\Vibranium";

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
	
	GetProdIdNVersion();
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	
	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);
	
	CScannerLoad objscanner;
	objscanner.RegisterProcessId(WLSRV_ID_SEVENTEEN);
	objscanner.RegisterProcessId(WLSRV_ID_EIGHTEEN);
	objscanner.RegisterProcessId(WLSRV_ID_NINETEEN);
	objscanner.RegisterProcessId(WLSRV_ID_TWENTY);
	objscanner.RegisterProcessId(WLSRV_ID_TWENTYONE);
	objscanner.RegisterProcessId(WLSRV_ID_TWENTYTWO);

	CSecure64 objCsecure;
	objCsecure.RegisterProcessId(WLSRV_ID_SEVENTEEN);
	objCsecure.RegisterProcessId(WLSRV_ID_EIGHTEEN);
	objCsecure.RegisterProcessId(WLSRV_ID_NINETEEN);
	objCsecure.RegisterProcessId(WLSRV_ID_TWENTY);
	objCsecure.RegisterProcessId(WLSRV_ID_TWENTYONE);
	objCsecure.RegisterProcessId(WLSRV_ID_TWENTYTWO);

	CWardWizInstallerDlg dlg;
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

/***************************************************************************
* Function Name   : GetModuleFilePath
* Description     : Function which get the current Application path.
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
****************************************************************************/
CString CWardWizInstallerApp::GetModuleFilePath()
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
		AddLogEntry(L"### Exception in CWardwizInstallerApp::GetModuleFilePath", 0, 0, true, SECONDLEVEL);
	}
	
	return(CString(szModulePath));
}

/***************************************************************************
*Function Name    : GetSelectedProductEdition
*Description      : Function returns DWORD value
1 - ESSENTIAL   2 - PRO
3 - ELITE        4 - BASIC
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
****************************************************************************/
void CWardWizInstallerApp::GetProdIdNVersion()
{
	try
	{
		TCHAR szProdVer[MAX_PATH] = { 0 };
		CString csIniFilePath = GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";

		if (PathFileExists(csIniFilePath))
		{
			m_dwProdID = GetPrivateProfileInt(L"VBSETTINGS", L"ProductID", 1, csIniFilePath);

			GetPrivateProfileString(L"VBSETTINGS", L"ProductVer", L"", szProdVer, MAX_PATH, csIniFilePath);
			m_csProdVersion = szProdVer;
		}
		else
		{
			AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetProdIdNVersion", csIniFilePath, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerApp::GetProdIdNVersion", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
* Function Name   : GetString
* Description     : Function which takes msg in id format.
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
****************************************************************************/
CString CWardWizInstallerApp::GetString(UINT uiResourceID)
{
	CString csReturnstring(L"");
	try
	{
		csReturnstring = GetStringFromStringTable(uiResourceID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerApp::GetString", 0, 0, true, SECONDLEVEL);
	}

	return csReturnstring;
}

/***************************************************************************
* Function Name   : GetStringFromStringTable
* Description     : Function which loads string id.
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
****************************************************************************/
CString CWardWizInstallerApp::GetStringFromStringTable(UINT uiResourceID)
{
	CString csRetStr(L"");
	try
	{
		csRetStr.LoadStringW(uiResourceID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerApp::GetStringFromStringTable", 0, 0, true, SECONDLEVEL);
	}

	return csRetStr;
}