// WardWizALUpd.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WardWizALUpd.h"
#include "WardWizALUpdDlg.h"
#include "PatchMerger.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWardWizALUpdApp

BEGIN_MESSAGE_MAP(CWardWizALUpdApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWardWizALUpdApp construction

/***************************************************************************
  Function Name  : CWardWizALUpdApp
  Description    : Constructor
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0003
****************************************************************************/
CWardWizALUpdApp::CWardWizALUpdApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWardWizALUpdApp object

CWardWizALUpdApp theApp;

/***************************************************************************
  Function Name  : InitInstance
  Description    : This function is called to make doModal() call of WardWizALUpdDlg object
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0004
****************************************************************************/
BOOL CWardWizALUpdApp::InitInstance()
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
	m_AppIsRunningFromCmd = false;
	m_AppCmdBuildType = 0 ; // 0 for Build & 1 for Rebuild
	m_ichkBuildType = 0;	//0 for BuildType unchecked 
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	/*CWardWizALUpdDlg dlg;
	m_pMainWnd = &dlg;*/
	//AfxMessageBox(L"Wait...."); 
	bool bIsInvalidCmd = false; 
	CString sCommandLineArgs = m_lpCmdLine;
	try
	{
			if (sCommandLineArgs.Trim() == L"AUTO")
			{
				m_ichkBuildType = 0; //0 for BuildType unchecked
				m_AppIsRunningFromCmd = true;
			}
			else if(sCommandLineArgs.Trim() == L"b")     // build "b"
			{
				m_AppIsRunningFromCmd = true;
				m_AppCmdBuildType = 0;
			}
			else if(sCommandLineArgs.Trim() == L"rb")     // rebuild "rb"
			{
				m_AppIsRunningFromCmd = true;
				m_AppCmdBuildType = 1;
			}
			else if(sCommandLineArgs == L"")
			{
				bIsInvalidCmd = false;
			}
			else
			{
				bIsInvalidCmd = true;
				HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
				CString header = L"Invalid argument...";
				AttachConsole(ATTACH_PARENT_PROCESS);     // Use current console window
				LPDWORD charsWritten = 0;
				WriteConsole(/*GetStdHandle(STD_OUTPUT_HANDLE)*/stdHandle, header, header.GetLength(), NULL, NULL);
				CloseHandle(stdHandle); 
				FreeConsole(); 
				//DWORD res = GetLastError(); 
				CWinApp::ExitInstance(); 
			}
	}
	catch(...)
	{

	}
    if(!bIsInvalidCmd)
	{
		//CPatchMerger dlg;
		CWardWizALUpdDlg dlg;
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
	}
	 
	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
