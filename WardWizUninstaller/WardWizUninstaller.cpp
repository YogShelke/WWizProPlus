/**********************************************************************************************************
Program Name          : WardWizUninstaller.cpp
Description           :
Author Name			: Ramkrushna Shelke
Date Of Creation      : 6th Feb 2015
Version No            : 1.9.0.0
Special Logic Used    :
Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizUninstaller.h"
#include "WardWizUninstallerDlg.h"
#include "WardWizDatabaseInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CWardWizUninstallerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

/***************************************************************************************************
*  Function Name  : CWardWizUninstallerApp
*  Description    : Cons't
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 20 May 2016
****************************************************************************************************/
CWardWizUninstallerApp::CWardWizUninstallerApp()
{
	m_hResDLL = NULL;
	m_dwProductID = 0x00;
	m_objCompleteEvent.ResetEvent();
}

/***************************************************************************************************
*  Function Name  : ~CWardWizUninstallerApp
*  Description    : Des't
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date           : 20 May 2016
****************************************************************************************************/
CWardWizUninstallerApp::~CWardWizUninstallerApp()
{
	if (m_hResDLL != NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
}

CWardWizUninstallerApp theApp;

/***************************************************************************************************
*  Function Name  : InitInstance
*  Description    :  The Windows operating system allows you to run more than one copy, or "instance," 
					 of the same application. WinMain calls InitInstance every time a new instance of 
					 the application starts.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 20 May 2016
****************************************************************************************************/
BOOL CWardWizUninstallerApp::InitInstance()
{
	CWinApp::InitInstance();

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

	//Get the registry path
	m_csProdKeyName = CWWizSettingsWrapper::GetProductRegistryKey();

	LoadResourceDLL();
	m_dwSelectedLangID = m_objwardwizLangManager.GetSelectedLanguage();
	m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
	m_AppPath = GetModuleFilePath();
	if (SingleInstanceCheck())
	{
		HWND hWindow = ::FindWindow(NULL, L"VibraniumUNINST");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			AddLogEntry(L"Wardwiz Uninstaller is already running.", 0, 0, true, SECONDLEVEL);
			return FALSE;
		}
	}
	LoadReqdLibrary();

	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

	CWardWizUninstallerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
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
*  Function Name  : LoadResourceDLL
*  Description    : Load resource dll.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 27 May 2014
****************************************************************************************************/
void CWardWizUninstallerApp::LoadResourceDLL()
{
	CString	csWardWizModulePath = GetModuleFilePath();

	CString	csWardWizResourceDLL = L"";
	csWardWizResourceDLL.Format(L"%s\\VBRESOURCE.DLL", csWardWizModulePath);
	if (!PathFileExists(csWardWizResourceDLL))
	{
		exit(0);
	}

	if (!m_hResDLL)
	{
		m_hResDLL = LoadLibrary(csWardWizResourceDLL);
		if (!m_hResDLL)
		{
			exit(0);
		}
	}

}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
CString CWardWizUninstallerApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***************************************************************************************************
*  Function Name  : SingleInstanceCheck
*  Description    : Check whether WardWiz Uninstaller instance is already running
*  Author Name    : Jeena Mariam Saji
*  SR_NO
*  Date           : 23 Sept 2016
****************************************************************************************************/
bool CWardWizUninstallerApp::SingleInstanceCheck()
{
	try
	{
		m_hMutex = CreateMutex(NULL, TRUE, L"{B953A064-E773-4271-9C37-6CC24F2F4DF9}");
		DWORD dwError = GetLastError();
		if (dwError == ERROR_ALREADY_EXISTS)
		{
			return true;
		}
		return false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerApp::SingleInstanceCheck()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : LoadReqdLibrary
*  Description    : Function to load the required library, which gets called when application loads the DLL.
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date			  :	21 March 2018
****************************************************************************************************/
void CWardWizUninstallerApp::LoadReqdLibrary()
{
	__try
	{
		m_hInstLibrary = LoadLibrary(L"VBHASH.DLL");
		if (!m_hInstLibrary)
		{
			AddLogEntry(L"### Failed to load library : VBHASH.DLL", 0, 0, true, SECONDLEVEL);
			return;
		}

		m_pGetbyteStrHash = (GetByteStringHashFunc)GetProcAddress(m_hInstLibrary, "GetStringHash");
		if (!m_pGetbyteStrHash)
		{
			AddLogEntry(L"### Failed to load function : GetByteStringHash", 0, 0, true, SECONDLEVEL);
			return;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerApp::LoadReqdLibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CalculateMD5
*  Description    : Function to calculate MD5, a DLL function .
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date			  :	21 March 2018
****************************************************************************************************/
DWORD CWardWizUninstallerApp::CalculateMD5(TCHAR *pString, int iStringlen, TCHAR *pFileHash)
{
	DWORD	dwReadBytes = 0x00;
	DWORD dwResult = 0x00;
	__try
	{
		if (m_pGetbyteStrHash != NULL)
		{
			dwResult = m_pGetbyteStrHash(pString, iStringlen, pFileHash);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return dwResult;
	}
	return dwResult;
}
