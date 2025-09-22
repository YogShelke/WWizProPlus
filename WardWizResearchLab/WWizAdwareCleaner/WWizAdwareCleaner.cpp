
// WWizAdwareCleaner.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WWizAdwareCleaner.h"
#include "WWizAdwareCleanerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWWizAdwareCleanerApp

BEGIN_MESSAGE_MAP(CWWizAdwareCleanerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWWizAdwareCleanerApp construction

CWWizAdwareCleanerApp::CWWizAdwareCleanerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWWizAdwareCleanerApp object

CWWizAdwareCleanerApp theApp;


// CWWizAdwareCleanerApp initialization

BOOL CWWizAdwareCleanerApp::InitInstance()
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

	if (SingleInstanceCheck())
	{
		HWND hWindow = ::FindWindow(NULL, L"WardWiz Adware Cleaner");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			return FALSE;
		}
	}
	if (!CheckForReqdFiles())
	{
		return false;
	}
	CWWizAdwareCleanerDlg dlg;
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
*  Function Name  : SingleInstanceCheck
*  Description    : It checks whether any other instance is already running
*  Author Name    : Jeena Mariam Saji
*  Date			  : 27 June 2017
****************************************************************************************************/
bool CWWizAdwareCleanerApp::SingleInstanceCheck()
{
	m_hMutexHandle = CreateMutex(NULL, TRUE, L"{9DB30BF8-4201-41E0-B887-69E1C08A9999}");
	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : CheckForReqdFiles
*  Description    : To check whether required files are present
*  Author Name    : Jeena Mariam Saji
*  Date			  : 14 July 2017
****************************************************************************************************/
bool CWWizAdwareCleanerApp::CheckForReqdFiles()
{
	try
	{
		CString csDLLPath;
		csDLLPath.Format(L"%s\\SQLITE3.DLL", GetModuleFilePath());

		if (!PathFileExists(csDLLPath))
		{
			MessageBox(NULL, L"Couldn't find SQLITE3.DLL. Please reinstall and try again.", L"WardWiz Adware Cleaner", MB_OK);
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerApp::CheckForReqdFiles()", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : To get Module File Path
*  Author Name    : Jeena Mariam Saji
*  Date			  : 29 June 2017
****************************************************************************************************/
CString CWWizAdwareCleanerApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***************************************************************************************************
*  Function Name  : OnCopyFolder
*  Description    : Function to copy folder and its contents from source to destination
*  Author Name    : Jeena Mariam Saji
*  Date			  : 7 July 2017
****************************************************************************************************/
void CWWizAdwareCleanerApp::OnCopyFolder(TCHAR* szSourceString, TCHAR* szDestString)
{
	try
	{
		if (!szSourceString)
			return;

		if (!szDestString)
			return;

		WIN32_FIND_DATA info = { 0 };
		HANDLE hwnd;

		TCHAR szTempFileString[1000] = { 0 };

		TCHAR szTempFileSource[1000] = { 0 };
		TCHAR szTempFileDest[1000] = { 0 };

		_tcscpy_s(szTempFileString, szSourceString);
		_tcscat_s(szTempFileString, _T("\\*.*"));

		CreateDirectory(szDestString, NULL);
		hwnd = FindFirstFile(szTempFileString, &info);
		do
		{
			if (!_tcscmp(info.cFileName, _T("."))) continue;
			if (!_tcscmp(info.cFileName, _T(".."))) continue;

			_tcscpy_s(szTempFileSource, szSourceString);
			_tcscat_s(szTempFileSource, _T("\\"));
			_tcscat_s(szTempFileSource, info.cFileName);

			_tcscpy_s(szTempFileDest, szDestString);
			_tcscat_s(szTempFileDest, _T("\\"));
			_tcscat_s(szTempFileDest, info.cFileName);
			if (info.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				_tcscpy_s(szTempFileDest, szDestString);
				_tcscat_s(szTempFileDest, _T("\\"));
				_tcscat_s(szTempFileDest, info.cFileName);
				OnCopyFolder(szTempFileSource, szTempFileDest);
			}
			else
			{
				_tcscpy_s(szTempFileString, szDestString);
				_tcscat_s(szTempFileString, _T("\\"));
				_tcscat_s(szTempFileString, info.cFileName);
				if (!CopyFile(szTempFileSource, szTempFileDest, false))
				{
					AddLogEntry(L"### Failed to copy folders and file in CWWizAdwareCleanerApp::OnCopyFolder", 0, 0, true, SECONDLEVEL);
				}
			}
		} while (FindNextFile(hwnd, &info));

		FindClose(hwnd);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerApp::OnCopyFolder", 0, 0, true, SECONDLEVEL);
	}
}
