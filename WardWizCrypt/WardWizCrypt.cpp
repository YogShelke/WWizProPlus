/**********************************************************************************************************
	Program Name			: WardWizCrypt.cpp
	Description				: Application classs which is derived from CWinApp
	Author Name				: Ramkrushna Shelke
	Date Of Creation		: 5th Jun 2015
	Version No				: 1.11.0.0
	Special Logic Used		:
	Modification Log		:
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizCrypt.h"
#include "WardWizCryptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWardWizCryptApp

BEGIN_MESSAGE_MAP(CWardWizCryptApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWardWizCryptApp construction

//Added a function to check registration and single instance 
//Neha Gharge 1st July,2015
CWardWizCryptApp::CWardWizCryptApp()
{
	// support Restart Manager
	//m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	m_bIsNoGui			= false;
	m_GetDaysLeft = NULL;
	m_hRegistrationDLL = NULL;
	m_dwProductID = 1;
	m_dwDaysLeft = 0xFFFF;
	m_hMutexHandle = NULL;
	m_csModulePath = L"";
	m_csPassword = L"";
	m_csDefaultDataEncVersion = L"1.0";               // issue data encryption version getting mismatch, default data encryption verion in case of data decryption utility
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWardWizCryptApp object

CWardWizCryptApp theApp;

/***************************************************************************************************
*  Function Name  : ~CWardWizCryptApp
*  Description    : D'tor
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 1 July 2015
****************************************************************************************************/
CWardWizCryptApp::~CWardWizCryptApp(void)
{
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


// CWardWizCryptApp initialization

BOOL CWardWizCryptApp::InitInstance()
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

	//if (CheckMutexOfDriverInstallation())
	//{
	//	MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_DRIVER_INSTALLATION_PROCESS_INPROGRESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
	//	return FALSE;
	//}

	CWardWizCryptDlg dlg;

	//Added a function to check registration and single instance 
	//Neha Gharge 1st July,2015
	m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();

	CWardWizOSversion		objOSVersionWrap;
	m_OSType = objOSVersionWrap.DetectClientOSVersion();
	CheckiSPYAVRegistered();

	//MessageBox(NULL, L"", L"", MB_OK);

	//Added By Nitin Kolapkar
	//Parsing the command line for request from Main UI and starting the CryptOpr byDefault
	CString csCommandLine = GetCommandLine();
	CString csEntry;
	int iPos = 0, iCmdParmIndex = 0;
	DWORD dwSeventhEntry = 0;
	CString csaTokenEntry[7];
	CString csFirstEntry, csSecondEntryNoGui, csThirdEntryCryptType, csForthEntryPassword,
		csFifthEntryDeleteStatus, csSixthEntryFilePath, csSeventhEntry;
	CString resToken;

	csEntry = csCommandLine;
	resToken = csEntry.Tokenize(_T("-"), iPos);
	while (resToken != _T(""))
	{
		//Issue: 0000623  If file path contains - then to exclude string from tokenization process
		//Resolved By :  Nitin K Date : 1st July 2015
		if (iCmdParmIndex < 4)
		{
			csaTokenEntry[iCmdParmIndex++] = resToken;
			resToken = csEntry.Tokenize(_T("-"), iPos);
		}
		else
		{
			//resToken = csEntry.Tokenize(_T("-"), iPos);
			csaTokenEntry[iCmdParmIndex++] = resToken;
			resToken = csEntry.Tokenize(_T(""), iPos);
			csaTokenEntry[iCmdParmIndex++] = resToken;
			break;
		}
	}
	// WardWizCrypt.exe launch by double click and we are avoiding double click event.
	// resolved by lalit kumawat 6-29-2015
	if (iCmdParmIndex <= 1)
	{
		exit(0);       
	}
	csFirstEntry = csaTokenEntry[0].Trim();
	csSecondEntryNoGui = csaTokenEntry[1].Trim();
	csThirdEntryCryptType = csaTokenEntry[2].Trim();
	csForthEntryPassword = csaTokenEntry[3].Trim();
	csFifthEntryDeleteStatus = csaTokenEntry[4].Trim();
	csSixthEntryFilePath = csaTokenEntry[5].Trim();
	csSeventhEntry = csaTokenEntry[6].Trim();
	//1. Issue with Browse & Stop buttons on Data encryption UI. 2. Strctured handling for insufficient parameters
	//Resolved By : Nitin K Date: 9th July 2015
	if (!(csSecondEntryNoGui.CompareNoCase(TEXT("NOGUI")) == 0))
	{
		m_bIsNoGui = false;
	}
	else
	{
		m_bIsNoGui = true;

		if (csThirdEntryCryptType.CompareNoCase(TEXT("ENC")) == 0)
		{
			dlg.m_bIsEncrypt = true;
		}
		else if (csThirdEntryCryptType.CompareNoCase(TEXT("DEC")) == 0)
		{
			dlg.m_bIsEncrypt = false;
		}
		else
		{
			dlg.CryptOprFailed();
			AddLogEntry(L"### error in CWardWizCryptDlg::InitInstance::SingleInstanceCheck::ENC-DEC missing", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (csForthEntryPassword.CompareNoCase(TEXT("")) >= 0)
		{
			dlg.m_csPassword = csForthEntryPassword;
			//dlg.m_editPassword.SetWindowTextW(csForthEntryPassword);
		}
		else
		{
			dlg.CryptOprFailed();
			AddLogEntry(L"### error in CWardWizCryptDlg::InitInstance::SingleInstanceCheck:: Pass missing", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (csFifthEntryDeleteStatus.CompareNoCase(TEXT("DELETEORIGINAL")) == 0)
		{
			dlg.m_dwKeepOriginal = DELETEORIGINAL;
		}
		else if (csFifthEntryDeleteStatus.CompareNoCase(TEXT("KEEPORIGINAL")) == 0)
		{
			dlg.m_dwKeepOriginal = KEEPORIGINAL;
		}
		else
		{
			dlg.CryptOprFailed();
			AddLogEntry(L"### error in CWardWizCryptDlg::InitInstance::SingleInstanceCheck::FileDelete param missing", 0, 0, true, SECONDLEVEL);
			return false;
		}
		iPos = 0; iCmdParmIndex = 0;
		//Issue resolved: Tokenizer updated for Command line
		//Resolved by Nitin K. Date: 20th June 2015
		csEntry = csSixthEntryFilePath;
		resToken = csEntry.Tokenize(_T("|"), iPos);
		while (resToken != _T(""))
		{
			/*if (PathFileExists(resToken))
			{*/
				m_csaTokenEntryFilePath[iCmdParmIndex++] = resToken;
				resToken = csEntry.Tokenize(_T("|"), iPos);
			/*}*/
		}
		int i = m_csaTokenEntryFilePath->GetLength();
		if (i == 0)
		{
			dlg.CryptOprFailed();
			AddLogEntry(L"### error in CWardWizCryptDlg::InitInstance::Filepath insufficient", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}

	//Added a function to check registration and single instance 
	//Neha Gharge 1st July,2015
	if (theApp.m_dwDaysLeft == 0)
	{
		if (!m_bIsNoGui)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_EMAIL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		}
		return FALSE;
	}

	if (SingleInstanceCheck())
	{
		dlg.CryptOprFailed();
		AddLogEntry(L"### error in CWardWizCryptDlg::InitInstance::SingleInstanceCheck", 0, 0, true, SECONDLEVEL);
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

/***************************************************************************
Function Name  : AddUserAndSystemInfoToLog
Description    : Adds Computer name, logged user name and OS details to log at the top
Author Name    : Vilas Suvarnakar
SR_NO		   :
Date           : 04 May 2015
Modification   :
****************************************************************************/
//void CWardWizCryptApp::AddUserAndSystemInfoToLog()
//{
//
//	try
//	{
//		TCHAR	szModulePath[MAX_PATH] = { 0 };
//
//		if (!GetModulePath(szModulePath, MAX_PATH))
//		{
//			return;
//		}
//
//		TCHAR	szTemp[512] = { 0 };
//
//		swprintf_s(szTemp, _countof(szTemp), L"%s\\Log\\%s", szModulePath, LOG_FILE);
//
//		if (PathFileExists(szTemp))
//			return;
//
//		WardWizSystemInfo	objSysInfo;
//
//		objSysInfo.GetSystemInformation();
//
//		LPCTSTR		lpSystemName = objSysInfo.GetSystemName();
//		LPCTSTR		lpUserName = objSysInfo.GetUserNameOfSystem();
//		LPCTSTR		lpOSDetails = objSysInfo.GetOSDetails();
//
//		if (lpSystemName)
//		{
//			ZeroMemory(szTemp, sizeof(szTemp));
//			swprintf_s(szTemp, _countof(szTemp), L"Computer name:%s\n", lpSystemName);
//			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
//		}
//
//		if (lpUserName)
//		{
//			ZeroMemory(szTemp, sizeof(szTemp));
//			swprintf_s(szTemp, _countof(szTemp), L"Logged user name:%s\n", lpUserName);
//			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
//		}
//
//		if (lpOSDetails)
//		{
//			ZeroMemory(szTemp, sizeof(szTemp));
//			swprintf_s(szTemp, _countof(szTemp), L"OS details:%s\n\n", lpOSDetails);
//			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CISpyGUIApp::AddUserAndSystemInfoToLog", 0, 0, true, SECONDLEVEL);
//	}
//}



/***************************************************************************************************
*  Function Name  : CheckiSPYAVRegistered
*  Description    : Loading registrations dll and proc address of function written in registration dll
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 1 July 2015
****************************************************************************************************/
DWORD CWardWizCryptApp::CheckiSPYAVRegistered()
{
	DWORD	dwRet = 0x00;

	CString	striSPYAVPath = GetModuleFilePath();

	CString	strRegistrationDLL = L"";

	strRegistrationDLL.Format(L"%s\\VBREGISTRATION.DLL", striSPYAVPath);
	if (!PathFileExists(strRegistrationDLL))
	{
		MessageBox(NULL, L"VBREGISTRATION.DLL module not found.\nPlease reinstall Vibranium to fix this problem.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if (!m_hRegistrationDLL)
	{
		m_hRegistrationDLL = LoadLibrary(strRegistrationDLL);
		if (!m_hRegistrationDLL)
		{
			MessageBox(NULL, L"Failed to load VBREGISTRATION.DLL module.\nPlease reinstall Vibranium to fix this problem.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}

	m_GetDaysLeft = (GETDAYSLEFT)GetProcAddress(m_hRegistrationDLL, "GetDaysLeft");

	if (!m_GetDaysLeft)
	{
		FreeLibrary(m_hRegistrationDLL);
		m_hRegistrationDLL = NULL;
		MessageBox(NULL, L"GetDaysLeft not found in VBREGISTRATION.DLL module.\nPlease reinstall Vibranium to fix this problem.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	m_dwDaysLeft = m_GetDaysLeft(m_dwProductID);

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 1 July 2015
****************************************************************************************************/
CString CWardWizCryptApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';
	m_csModulePath = szModulePath;
	return(CString(szModulePath));
}

/***************************************************************************************************
*  Function Name  : SingleInstanceCheck
*  Description    : Check whether wardwiz instance is exist or not
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 1 July 2015
****************************************************************************************************/
bool CWardWizCryptApp::SingleInstanceCheck()
{

	//HANDLE hHandle = CreateMutex(NULL, TRUE, L"{3E4EBB94-1B9C-4d7c-99BB-ED5576FC3FFA}");
	//Issue: 0000057: UI doesn't get launch Immediately.
	//Resolved By : Nitin K 17th March 2015
	m_hMutexHandle = CreateMutex(NULL, TRUE, L"{13520CD5-0250-4778-9E66-7518AF054650}");
	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

/***************************************************************************
Function Name  : CheckMutexOfDriverInstallation
Description    : Check Mutex Of Driver Installation
Author Name    : Neha Gharge
SR_NO		   :
Date           : 30th OCt 2015
Modification   :
****************************************************************************/
bool CWardWizCryptApp::CheckMutexOfDriverInstallation()
{
	try
	{
		m_hMutexHandleDriverInstallation = CreateMutex(NULL, TRUE, L"{E7E0EC4A-DE70-409C-8405-33F19CB0B74F}");
		DWORD dwError = GetLastError();
		if (dwError == ERROR_ALREADY_EXISTS)
		{
			return true;
		}
		else
		{
			if (m_hMutexHandleDriverInstallation != NULL)
			{
				if (!(ReleaseMutex(m_hMutexHandleDriverInstallation)))
				{
					DWORD dwErr = GetLastError();
					AddLogEntry(L"### Failed to release mutex of driver installation.", 0, 0, true, SECONDLEVEL);
					CloseHandle(m_hMutexHandleDriverInstallation);
				}
				else
				{
					CloseHandle(m_hMutexHandleDriverInstallation);
					AddLogEntry(L">>> Succeed to release mutex of driver installation.", 0, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CISpyGUIApp::CheckMutexOfDriverInstallation", 0, 0, true, SECONDLEVEL);
	}
	return false;
}
