
// WrdWizDownloader.cpp : Defines the class behaviors for the application.
/***********************************************************************************************
*  Program Name: WrdWizDownloader.cpp
*  Description: It is a downloader which is used to download the setup files from server.
*  Author Name: Neha Gharge
*  Date Of Creation: 25-Aug-2015
*  Version No: 1.10.0.0
************************************************************************************************/
#include "stdafx.h"
#include "WrdWizDownloader.h"
#include "WrdWizDownloaderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWrdWizDownloaderApp

BEGIN_MESSAGE_MAP(CWrdWizDownloaderApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWrdWizDownloaderApp construction

CWrdWizDownloaderApp::CWrdWizDownloaderApp()
{
	// support Restart Manager
	//m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWrdWizDownloaderApp object

CWrdWizDownloaderApp theApp;


// CWrdWizDownloaderApp initialization

BOOL CWrdWizDownloaderApp::InitInstance()
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
	OleInitialize(NULL); // for shell interaction: drag-n-drop, etc. 
	CWinApp::InitInstance();
	sciter::sync::gui_thread_ctx _; // instance of gui_thread_ctx
	// it should be created as a variable inside WinMain 
	// gui_thread_ctx is critical for GUI_CODE_START/END to work

	m_csurl = L"";
	AfxEnableControlContainer();
	
	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	//CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	if (SingleInstanceCheck())
	{
		#ifdef WRDWIZBASIC
			#ifdef RELEASENCG
				MessageBox(NULL, theApp.GetString(IDS_STRING_SINGLE_INSTANCE_G), L"Vibranium", MB_OK | MB_ICONINFORMATION);
			#elif RELEASENCI
				MessageBox(NULL, theApp.GetString(IDS_STRING_SINGLE_INSTANCE), L"Vibranium", MB_OK | MB_ICONINFORMATION);
			#endif

		#elif WRDWIZESSNL
			#ifdef RELEASENCG
				MessageBox(NULL, theApp.GetString(IDS_STRING_SINGLE_INSTANCE_G), L"Vibranium", MB_OK | MB_ICONINFORMATION);
			#elif RELEASENCI
				MessageBox(NULL, theApp.GetString(IDS_STRING_SINGLE_INSTANCE), L"Vibranium", MB_OK | MB_ICONINFORMATION);
			#endif
		#endif
		HWND hWindow = ::FindWindow(NULL, L"WardWiz Download Manager");
		if (!hWindow)
		{
			AddLogEntry(L"Wardwiz downloader handle is not available", 0, 0, true, SECONDLEVEL);
		}
		//Added By Nitin Kolapkar
		//If 1 instance is already running and we click on Right click-> Encrypt || Decrypt then stop previous operation and launch new Operation
		::ShowWindow(hWindow, SW_RESTORE);
		::BringWindowToTop(hWindow);
		::SetForegroundWindow(hWindow);
		return FALSE;
	}


	CWrdWizDownloaderDlg dlg;
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
	//if (pShellManager != NULL)
	//{
	//	delete pShellManager;
	//}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


/***************************************************************************************************
*  Function Name  : SingleInstanceCheck
*  Description    : Check whether wardwiz instance is exist or not
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
bool CWrdWizDownloaderApp::SingleInstanceCheck()
{
	m_hMutexHandle = CreateMutex(NULL, TRUE, L"{4ADC774C-9993-4E76-B699-9891C045CB25}");
	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

CString CWrdWizDownloaderApp::GetString(UINT uiResourceID)
{
	CString csReturnstring(L"");
	try
	{
		
		#ifdef ENGLISH
			csReturnstring = GetStringFromStringTable(uiResourceID);
		#elif GERMAN
			uiResourceID+=249;
			csReturnstring = GetStringFromStringTable(uiResourceID);
		#endif
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDownloaderApp::GetString", 0, 0, true, SECONDLEVEL);
	}
	return csReturnstring;
}

CString CWrdWizDownloaderApp::GetStringFromStringTable(UINT uiResourceID)
{
	CString csRetStr(L"");
	try
	{
		csRetStr.LoadStringW(uiResourceID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumDownloaderApp::GetStringFromStringTable", 0, 0, true, SECONDLEVEL);
	}
	return csRetStr;
}