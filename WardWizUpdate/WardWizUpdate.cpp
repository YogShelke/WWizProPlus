/**********************************************************************************************************
Program Name          : WardWizUpdate.cpp
Description           : This class contains the functionality for updating product from local folder as well as live update.
It has 2 options	  : 1) Update from internet.
						2) Update from Local Folder.
Author Name			  : Amol Jaware
Date Of Creation      : 15th Jan 2019
Version No			  : 4.1.0.1
***********************************************************************************************************/

#include "stdafx.h"
#include "WardWizUpdate.h"
#include "WardWizUpdateDlg.h"
#include "WrdWizSystemInfo.h"
#include "CSecure64.h"
#include "CScannerLoad.h"
#include "DriverConstants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWardWizUpdateApp

BEGIN_MESSAGE_MAP(CWardWizUpdateApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


typedef DWORD(*GETFILEHASH)	(TCHAR *pFilePath, TCHAR *pFileHash);
GETFILEHASH		GetFileHash;


/***************************************************************************************************
*  Function Name  : CWardWizUpdateApp
*  Description    : CTOR CWardWizUpdateApp
*  Author Name    : Amol J.
*  SR_NO
*  Date           : 12-Feb-2019
****************************************************************************************************/
CWardWizUpdateApp::CWardWizUpdateApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	m_bCmdLineUpdate = false;
	m_bEPSLiveUpdateNoUI = false;
	m_bRequestFromUI = false;
}

/***************************************************************************************************
*  Function Name  : CWardWizUpdateApp
*  Description    : DTOR CWardWizUpdateApp
*  Author Name    : Amol J.
*  SR_NO
*  Date           : 13-Feb-2019
****************************************************************************************************/
CWardWizUpdateApp::~CWardWizUpdateApp()
{
	m_bCmdLineUpdate = false;
	m_bEPSLiveUpdateNoUI = false;
}

// The one and only CWardWizUpdateApp object

CWardWizUpdateApp theApp;


// CWardWizUpdateApp initialization

BOOL CWardWizUpdateApp::InitInstance()
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

	CWardWizUpdateDlg dlg;
	CString csCommandLine = GetCommandLine();

	csCommandLine.MakeUpper();
	if (csCommandLine.Find('-') != -1)
	{
		csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
		csCommandLine.Trim();

		if (csCommandLine.GetLength() > 0)
		{
			if (csCommandLine.CompareNoCase(TEXT("LIVEUPDATE")) == 0)
			{
				m_bCmdLineUpdate = true;
			}
			else if (csCommandLine.CompareNoCase(TEXT("EPSNOUI")) >= 0)
			{
				csCommandLine.Replace(TEXT("EPSNOUI"), L"");
				csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
				csCommandLine.Trim();

				if (csCommandLine.CompareNoCase(TEXT("LIVEUPDATE")) == 0)
					m_bEPSLiveUpdateNoUI = true;
			}
		}
	}

	AfxEnableControlContainer();

	m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();

	//Get the registry path
	m_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	GetAppPath();

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
		HWND hWindow = ::FindWindow(NULL, L"VibraniumUPDATE");
		if (!hWindow)
		{
			AddLogEntry(L"Wardwiz Update handle is not available", 0, 0, true, FIRSTLEVEL);
		}
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			AddLogEntry(L"Wardwiz Update is already running.", 0, 0, true, SECONDLEVEL);
			return FALSE;
		}
		return FALSE;
	}

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_ZERO);//WLSRV_ID_ZERO to register service for process protection

	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_ZERO);//WLSRV_ID_ZERO to register service for process protection

	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

	CheckScanLevel();

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

	return FALSE;
}

/***************************************************************************************************
*  Function Name  : SingleInstanceCheck
*  Description    : Check whether wardwiz Temporary Cleaner instance is exist or not
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 16 Jan 2019
****************************************************************************************************/
bool CWardWizUpdateApp::SingleInstanceCheck()
{
	try
	{
		m_hMutex = CreateMutex(NULL, TRUE, L"Global\\WARDWIZUPDATE");
		DWORD dwError = GetLastError();

		if (dwError == ERROR_ALREADY_EXISTS)
		{
			AddLogEntry(L"Second", 0, 0, false, SECONDLEVEL);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateApp::SingleInstanceCheck", 0, 0, true, SECONDLEVEL);
	}

	return false;
}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 11 May,2015
****************************************************************************************************/
CString CWardWizUpdateApp::GetModuleFilePath()
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
		AddLogEntry(L"### Exception in CVibraniumUpdateApp::GetModuleFilePath", 0, 0, true, SECONDLEVEL);
	}

	return(CString(szModulePath));
}

/***************************************************************************************************
*  Function Name  : GetAppPath
*  Description    : Get the path where module is exist
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 11 May,2015
****************************************************************************************************/
bool CWardWizUpdateApp::GetAppPath()
{
	try
	{
		TCHAR szValueAppVersion[MAX_PATH] = { 0 };
		DWORD dwSizeAppVersion = sizeof(szValueAppVersion);

		if (m_objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"AppFolder", szValueAppVersion, dwSizeAppVersion) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CWardwizGUIApp::CheckScanLevel", 0, 0, true, SECONDLEVEL);
			return false;
		}
		m_AppPath = szValueAppVersion;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumzUIApp::GetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : CheckScanLevel
Description    : To check scan level 1-> Only with wardwiz scanner
2-> with clam scanner and second level scaner is wardwiz scanner.
SR.NO			 :
Author Name    : Neha gharge
Date           : 1-19-2015
***********************************************************************************************/
void CWardWizUpdateApp::CheckScanLevel()
{
	try
	{
		DWORD dwScanLevel = 0;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwScanLevel", dwScanLevel) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CVibraniumUpdateApp::CheckScanLevel", 0, 0, true, SECONDLEVEL);;
			return;
		}

		switch (dwScanLevel)
		{
			case 0x01:
				m_eScanLevel = WARDWIZSCANNER;
				break;
			case 0x02:
				m_eScanLevel = CLAMSCANNER;
				break;
			default:
				AddLogEntry(L" ### Scan Level option is wrong. Please reinstall setup of Wardwiz", 0, 0, true, SECONDLEVEL);
				break;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::CheckScanLevel", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : LoadDLLs()
*  Description    : loading hash and extract dll
*  Author Name    : Vilas
*  SR_NO		  : WRDWIZALUSRV_0031
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateApp::LoadDLLs()
{

	TCHAR	szTemp[512] = { 0 };
	try
	{
		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"%sVBHASH.dll", m_szAVPath);

		if (!m_hHashDLL)
			m_hHashDLL = LoadLibrary(szTemp);

		if (!m_hHashDLL)
			AddLogEntry(L"### LoadDLLs::LoadLibrary failed for(%s)", szTemp, 0, true, SECONDLEVEL);

		if (!GetFileHash)
			GetFileHash = (GETFILEHASH)GetProcAddress(m_hHashDLL, "GetFileHash");

		if (!GetFileHash)
			AddLogEntry(L"### LoadDLLs::GetFileHash failed for(%s)", szTemp, 0, true, SECONDLEVEL);

		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"%sVBEXTRACT.dll", m_szAVPath);

		if (!m_hExtractDLL)
			m_hExtractDLL = LoadLibrary(szTemp);

		if (!m_hExtractDLL)
			AddLogEntry(L"### LoadDLLs::LoadLibrary failed for(%s)", szTemp, 0, true, SECONDLEVEL);

		if (!UnzipSingleFile)
			UnzipSingleFile = (UNZIPSINGLEFILE)GetProcAddress(m_hExtractDLL, "UnzipSingleFile");

		if (!UnzipSingleFile)
			AddLogEntry(L"### LoadDLLs::UnzipSingleFile failed for(%s)", szTemp, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateApp LoadDLLs::Exception", 0, 0, true, SECONDLEVEL);
	}

	return false;
}