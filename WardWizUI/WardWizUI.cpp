/**********************************************************************************************************
Program Name          : WardWizUI.cpp
Description           : App class For WardWiz UI
Author Name			  : Nitin Kolapkar
Date Of Creation      : 27th May 2016
Version No            : 2.0.0.1
Modification Log      :
***********************************************************************************************************/
// WardWizUI.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WardWizUI.h"
#include "WardWizUIDlg.h"
#include "CSecure64.h"
#include "CScannerLoad.h"
#include "DriverConstants.h"
#include "WrdWizSystemInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

AVACTIVATIONINFO	g_ActInfo = { 0 };

#define		IDR_REGDATA				2000

// CWardWizUIApp

BEGIN_MESSAGE_MAP(CWardWizUIApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWardWizUIApp construction
/***********************************************************************************************
*  Function Name  : CWardWizUIApp
*  Description    : Constructor
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           :  27-May-2016
*************************************************************************************************/
CWardWizUIApp::CWardWizUIApp():
	m_dwDaysLeft(0xFFFF)
	, m_hMutexHandle(NULL)
{
		m_hResDLL				= NULL;
		m_hRegistrationDLL = NULL;
		m_eScanLevel = CLAMSCANNER;
		m_bQuickScan = false;
		m_bIsScanning = false;
		m_bIsAntirootkitScanning = false;
		m_UnzipFile = NULL;
		m_dwProductID = 1;
		m_bOpenRegDlg = false;
		m_bAllowStartUpTip = false;
		m_bIsEnDecFrmShellcmd = false;
		m_bScanPage = false;
		m_bReportPage = false;
		m_bEPSFullScanNoUI = false;
		m_bEPSQuickScanNoUI = false;
		m_bEPSNoUIScan = false;
		m_csTaskID = L"0";
		m_csPageName = L"NA";
		m_objCompleteEvent.ResetEvent();
}

/***********************************************************************************************
*  Function Name  : ~CWardWizUIApp
*  Description    : Destructor
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           :  27-May-2016
*************************************************************************************************/
CWardWizUIApp::~CWardWizUIApp()
{
	if (m_hResDLL != NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
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
CWardWizUIApp theApp;
/***************************************************************************************************
*  Function Name  : InitInstance
*  Description    : CWardWizUIApp initialization
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
BOOL CWardWizUIApp::InitInstance()
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
	
	OleInitialize(NULL); // for system drag-n-drop

	CWinApp::InitInstance();

	//Get the registry path
	m_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

	sciter::sync::gui_thread_ctx _; // instance of gui_thread_ctx
	// it should be created as a variable inside WinMain 
	// gui_thread_ctx is critical for GUI_CODE_START/END to work
	CWardWizUIDlg dlg;

	LoadReqdLibrary();


	m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
	ReadRegistryEntryofStartUp();
	CString csCommandLine = GetCommandLine();
	CString csRegOptCmdLine = csCommandLine;

	if (csCommandLine.Find('-') != -1)
	{
		csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
		csCommandLine.Trim();
		if (csCommandLine.GetLength() > 0)
		{
			WaitForSingleObject(dlg.m_hWnd, 1000 * 20); //Waiting for 20 seconds
			if (csCommandLine.CompareNoCase(TEXT("STARTUPSCAN")) >= 0)
			{
				m_bCheckScan = true;
				if (m_bAllowStartUpScan)
				{
					m_bStartUpScan = true;
					csCommandLine.Replace(TEXT("STARTUPSCAN"), L"");
					csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
					csCommandLine.MakeLower();
					csCommandLine.Trim();
					if (csCommandLine.Compare(TEXT("fullscan")) == 0)
					{
						m_bRunFullScan = true;
						m_dwScanType = 0;
					}
					else
					{
						m_bRunQuickScan = true;
						m_dwScanType = 1;
					}
				}
				}
			else if (csCommandLine.CompareNoCase(TEXT("SHOWREPORTS")) == 0)
			{
				m_bReportPage = true;
			}
			else if (csCommandLine.CompareNoCase(TEXT("SHOWSCANPAGE")) == 0)
			{
				m_bScanPage = true;
			}
			else if (csCommandLine.CompareNoCase(TEXT("SCHEDSCAN")) >= 0)
			{
				CString csCmdLineVal;
				m_bCheckScan = true;
				m_bSchedScan = true;
				csCmdLineVal = csCommandLine;
				csCommandLine.Replace(TEXT("SCHEDSCAN"), L"");
				csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
				csCommandLine.MakeLower();
				csCommandLine.Trim();
				csCommandLine.Delete(csCommandLine.Find(' '), csCommandLine.GetLength());
				if (csCommandLine.Compare(TEXT("fullscan")) == 0)
				{
					csCmdLineVal.Replace(TEXT("SCHEDSCAN -FULLSCAN"), L"");
					csCmdLineVal.Delete(0, csCmdLineVal.Find('-') + 1);
					if (csCmdLineVal.Compare(TEXT("SDYES")) == 0)
					{
						m_bSchedScanShutDown = true;
					}
					else if (csCmdLineVal.Compare(TEXT("SDNO")) == 0)
					{
						m_bSchedScanShutDown = false;
					}
					m_bRunFullScan = true;
					m_dwScanType = 0;
				}
				else if (csCommandLine.Compare(TEXT("quickscan")) == 0)
				{
					csCmdLineVal.Replace(TEXT("SCHEDSCAN -QUICKSCAN"), L"");
					csCmdLineVal.Delete(0, csCmdLineVal.Find('-') + 1);
					if (csCmdLineVal.Compare(TEXT("SDYES")) == 0)
					{
						m_bSchedScanShutDown = true;
					}
					else if (csCmdLineVal.Compare(TEXT("SDNO")) == 0)
					{
						m_bSchedScanShutDown = false;
					}
					m_bRunQuickScan = true;
					m_dwScanType = 1;
				}
				else if (csCommandLine.Compare(TEXT("regopt")) == 0)
				{

					csRegOptCmdLine.MakeUpper();

					if (csRegOptCmdLine.Find(L"-SCHEDSCAN -REGOPT -SDNO") == 0)
					{
						csRegOptCmdLine.Replace(TEXT("-SCHEDSCAN -REGOPT -SDNO"), L"");
						csRegOptCmdLine.Trim();
					}

					m_bRunRegOpt = true;
					m_dwScanType = 2;
				}
			}
			else if (csCommandLine.CompareNoCase(TEXT("SHOWREG")) >= 0)
			{
				m_bOpenRegDlg = true;
			}
			else if (csCommandLine.CompareNoCase(TEXT("LIVEUPDATE")) >= 0)
			{
				m_bRunLiveUpdate = true;
			}
			else if (csCommandLine.CompareNoCase(TEXT("EPSNOUI")) >= 0)
			{
				m_bEPSNoUIScan = true;
				csCommandLine.Replace(TEXT("EPSNOUI"), L"");
				csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
				csCommandLine.MakeLower();
				csCommandLine.Trim();
				if (csCommandLine.Compare(TEXT("fullscan")) >= 0)
				{
					m_bEPSFullScanNoUI = true;
					csCommandLine.Replace(TEXT("fullscan"), L"");
					csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
				}
				else if (csCommandLine.Compare(TEXT("quickscan")) >= 0)
				{
					m_bEPSQuickScanNoUI = true;
					csCommandLine.Replace(TEXT("quickscan"), L"");
					csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
				}
				m_csTaskID = csCommandLine;
			}
			else if (csCommandLine.CompareNoCase(TEXT("ENC")) >= 0) //Keep this at last
			{
				m_bDataCryptOpr = true;
				m_iDataOpr = ENCRYPTION;
				csCommandLine.Delete(0, csCommandLine.Find(' ') + 1);
				m_csDataCryptFilePath = csCommandLine;
				m_bIsEnDecFrmShellcmd = true;
			}

			else if (csCommandLine.CompareNoCase(TEXT("DEC")) >= 0) //Keep this at last
			{
				m_bDataCryptOpr = true;
				m_iDataOpr = DECRYPTION;
				csCommandLine.Delete(0, csCommandLine.Find(' ') + 1);
				m_csDataCryptFilePath = csCommandLine;
				m_bIsEnDecFrmShellcmd = true;
			}
		}
	}
	GetAppPath();
	LoadResourceDLL();
	CheckScanLevel();
	
	CheckisWardWizRegistered();

	AddUserAndSystemInfoToLog();
		
	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

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
		//Issue :product name and product folder name is same so on double click of tray creating problem - Neha Gharge.
		/******************ISSUE NO : 30 Neha G 23/5/2014 **********************************/
		HWND hWindow = ::FindWindow(NULL, L"VibraniumUI");
		if (!hWindow)
		{
			AddLogEntry(L"Wardwiz handle is not available", 0, 0, true, SECONDLEVEL);
		}
		//Added By Nitin Kolapkar
		//If 1 instance is already running and we click on Right click-> Encrypt || Decrypt then stop previous operation and launch new Operation
		if (m_bDataCryptOpr == true)
		{
			StartDataCryptOpr(m_iDataOpr, m_csDataCryptFilePath);
		}

		if (m_bOpenRegDlg)
		{
			SendMessage(hWindow, WM_MESSAGESHOWREG, 0, 0);
		}

		if (m_bSchedScan)
		{ 
			if (m_bRunRegOpt)
				StartScheduledScan4RegOpt(csRegOptCmdLine);
			else			
				StartScheduledScan();
		}

		::ShowWindow(hWindow, WS_EX_APPWINDOW);
		::ShowWindow(hWindow, SW_RESTORE);
		SendMessage(hWindow, WM_MESSAGEMAXIMWND, 0, 0);

		::BringWindowToTop(hWindow);
		::SetForegroundWindow(hWindow);
		return FALSE;
	}

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_ZERO);//WLSRV_ID_ZERO to register service for process protection

	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_ZERO);//WLSRV_ID_ZERO to register service for process protection

	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

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
*  Function Name  : LoadReqdLibrary
*  Description    : Function to load the required library, which gets called when application loads the DLL.
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date			  :	21 March 2018
****************************************************************************************************/
void CWardWizUIApp::LoadReqdLibrary()
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
		AddLogEntry(L"### Exception in CWardwizUIApp::LoadReqdLibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CalculateMD5
*  Description    : Function to calculate MD5, a DLL function .
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date			  :	21 March 2018
****************************************************************************************************/
DWORD CWardWizUIApp::CalculateMD5(TCHAR *pString, int iStringlen, TCHAR *pFileHash)
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

/***************************************************************************************************
*  Function Name  : LoadResourceDLL
*  Description    : Load resource dll.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 27 May 2014
****************************************************************************************************/
void CWardWizUIApp::LoadResourceDLL()
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
CString CWardWizUIApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}


/***************************************************************************************************
*  Function Name  : ShowRestartMsgOnProductUpdate
*  Description	  : If user updates the product but don't restart, and try to perform any operation, then message to 'Restart Computer' should pop-up.
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 17 March 2015
****************************************************************************************************/
bool CWardWizUIApp::ShowRestartMsgOnProductUpdate()
{
	try
	{
		TCHAR	szAllUserPath[255] = { 0 };
		TCHAR  szActualPath[255] = { 0 };

		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 255);
		swprintf_s(szActualPath, _countof(szActualPath), L"%s\\%s", szAllUserPath, L"Vibranium\\AluDel.ini");
		int fileExist = 0;
		fileExist = PathFileExists(szActualPath);
		if (fileExist == 1)
		{
			HWND hwnd = ::GetActiveWindow();
			CString csRestartRequired;
			int IdReturn = 0;
			if ((theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG1") == L"") && (theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG2") == L"") && (theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG3") == L""))
			{
				csRestartRequired.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG2"), theApp.m_objwardwizLangManager.GetString(L"IDS_GUI_RESTART_OR_CONTINUE"));
				IdReturn = MessageBox(hwnd, csRestartRequired, theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG1"), MB_ICONQUESTION | MB_YESNO);
			}
			else
			{
				csRestartRequired.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG2"), theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG3"));
				IdReturn = MessageBox(hwnd, csRestartRequired, theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG1"), MB_ICONQUESTION | MB_YESNO);
			}
			if (IdReturn == IDYES)
			{
				CEnumProcess enumproc;
				enumproc.RebootSystem(0);
				return false;
			}
		}
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CISpyGUIApp::ShowRestartMsgOnProductUpdate", 0, 0, true, SECONDLEVEL);
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
void CWardWizUIApp::CheckScanLevel()
{
	DWORD dwScanLevel = 0;
	if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwScanLevel", dwScanLevel) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CWardwizGUIApp::CheckScanLevel", 0, 0, true, SECONDLEVEL);;
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

bool CWardWizUIApp::GetAppPath()
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
		AddLogEntry(L"### Exception in CWardwizUIApp::GetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

void CWardWizUIApp::MessageBoxUI(CString csMessage, CString csMessageHeader, CString csMessageType)
{
	if (!m_pMainWnd)
		return;

	CWardWizUIDlg *pMainWnd = NULL;
	pMainWnd = (CWardWizUIDlg*)m_pMainWnd;

	if (!pMainWnd)
	{
		AddLogEntry(L"### Failed to get MainWnd Handle in CWardwizUIApp::CallUIFunction", 0, 0, true, SECONDLEVEL);
		return;
	}

	pMainWnd->CallUIMessageBox(csMessage, csMessageHeader, csMessageType);
}

/***************************************************************************************************
*  Function Name  : CheckiSPYAVRegistered
*  Description    : Loading registrations dll and proc address of function written in registration dll
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 sep 2013
****************************************************************************************************/
DWORD CWardWizUIApp::CheckisWardWizRegistered()
{
	DWORD	dwRet = 0x00;
	try
	{
		CString	striSPYAVPath = GetModuleFilePath();

		CString	strRegistrationDLL = L"";

		strRegistrationDLL.Format(L"%s\\VBREGISTRATION.DLL", striSPYAVPath);
		if (!PathFileExists(strRegistrationDLL))
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}

		if (!m_hRegistrationDLL)
		{
			m_hRegistrationDLL = LoadLibrary(strRegistrationDLL);
			if (!m_hRegistrationDLL)
			{
				MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				exit(0);
			}
		}

		m_GetDaysLeft = (GETDAYSLEFT)GetProcAddress(m_hRegistrationDLL, "GetDaysLeft");
		m_PerformRegistration = (PERFORMREGISTRATION)GetProcAddress(m_hRegistrationDLL, "PerformRegistration");
		m_CloseRegistrationWindow = (PERFORMREGISTRATION)GetProcAddress(m_hRegistrationDLL, "CloseRegistrationWindow");

		if (!m_GetDaysLeft)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_GET_DAYS_LEFT_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}

		if (!m_PerformRegistration || !m_CloseRegistrationWindow)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_PERFORM_REGISTRATION_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
		m_dwDaysLeft = m_GetDaysLeft(m_dwProductID);
		if (theApp.m_bOpenRegDlg)
		{
			Sleep(7000);
			if (m_dwDaysLeft <= 0)
			{
				m_dwDaysLeft = m_GetDaysLeft(m_dwProductID);
			}
		}
		//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
		//Neha Gharge 4 jan,2016
		m_lpLoadDoregistrationProc = (EXPIRYMSGBOX)GetProcAddress(m_hRegistrationDLL, "ShowEvalutionExpiredMsg");

		if (!m_lpLoadDoregistrationProc)
		{
			FreeLibrary(m_hRegistrationDLL);
			m_lpLoadDoregistrationProc = NULL;
			m_hRegistrationDLL = NULL;
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_FAILED_LOADSIGNATURES"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			return dwRet;
		}

		m_lpLoadProductInformation = (DOREGISTRATION)GetProcAddress(m_hRegistrationDLL, "ShowProductInformation");

		if (!m_lpLoadProductInformation)
		{
			FreeLibrary(m_hRegistrationDLL);
			m_lpLoadProductInformation = NULL;
			m_hRegistrationDLL = NULL;
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_FAILED_LOADSIGNATURES"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			return dwRet;
		}
		m_lpFnLoadProductInformation = (GETREGISTRATIONINFO)GetProcAddress(m_hRegistrationDLL, "GetRegisteredUserInfo");
		if (!m_lpFnLoadProductInformation)
		{
			FreeLibrary(m_hRegistrationDLL);
			m_lpFnLoadProductInformation = NULL;
			m_hRegistrationDLL = NULL;
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_FAILED_LOADSIGNATURES"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			return dwRet;
		}
		// resolve by lalit kumawat 3-17-015
		//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
		//(same issue applies to change password & scan computer at start up settings
		m_lpCloseProductInformationDlg = (DOREGISTRATION)GetProcAddress(m_hRegistrationDLL, "CloseProductInformationDlg");

		CString	strRegisteredDataDLL = L"";

		strRegisteredDataDLL.Format(L"%s\\VBREGISTERDATA.DLL", striSPYAVPath);
		if (!PathFileExists(strRegisteredDataDLL))
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTERDATA_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		}

		if (!m_hRegisteredDataDLL)
		{
			m_hRegisteredDataDLL = LoadLibrary(strRegisteredDataDLL);
		}

		m_lpLoadEmail = (GETREGISTEREFOLDERDDATA)GetProcAddress(m_hRegisteredDataDLL, "GetRegisteredData");
		if (!m_lpLoadEmail)
		{
			FreeLibrary(m_hRegistrationDLL);
			m_lpLoadProductInformation = NULL;
			m_lpFnLoadProductInformation = NULL;
			m_hRegistrationDLL = NULL;
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_GET_REGISTER_DATA_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			return dwRet;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizGUIApp::CheckisVibraniumRegistered", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : DoRegistration
*  Description    : Registartion process get started.
*  Author Name    : Ramkrushna shelke
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CWardWizUIApp::DoRegistration()
{
	__try
	{
		//issue no : 816 neha gharge 30/6/2014
		if (!m_PerformRegistration)
		{
			return;
		}
		m_PerformRegistration();
		m_dwDaysLeft = m_GetDaysLeft(m_dwProductID);
		if (!SendEmailPluginChange2Service(RELOAD_REGISTARTION_DAYS))
		{
			AddLogEntry(L"### Error in CSystemTray::SendEmailPluginChange2Service", 0, 0, true, SECONDLEVEL);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizGUIApp::DoRegistration", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : SendEmailPluginChange2Service()
*  Description    : Send message to emailplugin
*  Author Name    : Neha gharge
*  SR_No		  :
*  Date           : 31/5/2014
***********************************************************************************************/
bool CWardWizUIApp::SendEmailPluginChange2Service(int iMessageInfo, DWORD dwType, bool bEmailPluginWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;

		CISpyCommunicator objCom(EMAILPLUGIN_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CWardwizGUIApp::SendEmailPluginChange2Service");
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizGUIApp::SendEmailPluginChange2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : SendData2Service()
*  Description    : Send message to communicationService
*  Author Name    : Ramkrushna Shelke
*  SR_No		  :
*  Date           : 29/09/2016
***********************************************************************************************/
bool CWardWizUIApp::SendData2ComService(int iMessageInfo, DWORD dwType, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;

		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CWardwizUIApp::SendData2ComService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizGUIApp::SendEmailPluginChange2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : GetDaysLeft
*  Description    : Get days left of registration.
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
DWORD CWardWizUIApp::GetDaysLeft()
{
	try
	{
		if (m_GetDaysLeft != NULL)
		{
			m_dwDaysLeft = m_GetDaysLeft(m_dwProductID);
			return m_dwDaysLeft;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizGUIApp::GetDaysLeft", 0, 0, true, SECONDLEVEL);
	}
	return m_dwDaysLeft;
}

/***************************************************************************************************
*  Function Name  : ShowProductInformation
*  Description    : Shows product information on Main UI
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 17 Jan 2014
****************************************************************************************************/
bool CWardWizUIApp::GetRegisteredUserInfo()
{
	try
	{
		if (!m_lpFnLoadProductInformation)
		{
			return false;
		}

		AVACTIVATIONINFO	szActInfo;
		if (!m_lpFnLoadProductInformation(szActInfo))
		{
			return false;
		}

		if (!CheckForMachineID(szActInfo))
		{
			memset(&szActInfo, 0x00, sizeof(szActInfo));
			return false;
		}

		memcpy(&m_ActInfo, &szActInfo, sizeof(szActInfo));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizGUIApp::GetRegisteredUserInfo", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : GetRegisteredEmailID
*  Description    : Get registered email ID
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
LPTSTR CWardWizUIApp::GetRegisteredEmailID()
{
	return GetRegisteredUserInformation();
}

/***************************************************************************************************
*  Function Name  : GetRegisteredUserInfo
*  Description    : Get registered User info from registry or from file.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
LPTSTR CWardWizUIApp::GetRegisteredUserInformation()
{

	DWORD	dwRet = 0x00;
	DWORD	dwSize = 0x00;
	DWORD	dwRegUserSize = 0x00;
	dwRet = GetRegistrationDataFromRegistry();
	
	if (!dwRet)
	{
		goto Cleanup;
	}

	//Get Registration data from Files which are present on hard disk.
	if (dwRet)
	{
		dwRet = GetRegistrationDatafromFile();
	}

	if (!dwRet)
	{
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			return L"";
		}
		goto Cleanup;
	}

	dwSize = sizeof(m_ActInfo);
	dwRegUserSize = 0x00;

	memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));

	if (m_lpLoadEmail((LPBYTE)&m_ActInfo, dwRegUserSize, IDR_REGDATA, L"REGDATA") == 0)
	{
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			return L"";
		}
		dwRet = 0x00;
		goto Cleanup;
	}

	if (m_ActInfo.dwProductNo != theApp.m_dwProductID)
	{
		return L"";
	}

	//Match here the machine ID with Stored machineID
	//if someone provides the DB files from other computer then it works so 
	//necessary to check for machine ID.
	if (!CheckForMachineID(m_ActInfo))
	{
		memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
		return L"";
	}

Cleanup:
	if (!dwRet)
	{
		memcpy(&g_ActInfo, &m_ActInfo, sizeof(m_ActInfo));
	}

	wcscpy(m_szRegKey, m_ActInfo.szKey);

	return m_ActInfo.szEmailID;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDataFromRegistry
*  Description    : Get Registration data of user from registry.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWardWizUIApp::GetRegistrationDataFromRegistry()
{
	DWORD	dwRet = 0x00;
	try
	{
		DWORD	dwRegType = 0x00, dwRetSize = 0x00;

		HKEY	h_iSpyAV = NULL;
		HKEY	h_iSpyAV_User = NULL;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows"),
			0, KEY_READ, &h_iSpyAV) != ERROR_SUCCESS)
		{
			dwRet = 0x01;
			goto Cleanup;
		}
		dwRetSize = sizeof(m_ActInfo);
		if (RegQueryValueEx(h_iSpyAV, TEXT("VibraniumUserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo,
			&dwRetSize) != ERROR_SUCCESS)
		{
			dwRet = GetLastError();
			dwRet = 0x03;
			goto Cleanup;
		}

		if (DecryptRegistryData((LPBYTE)&m_ActInfo, sizeof(m_ActInfo)))
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		//if (_tcscmp(m_ActInfo.szTitle, L"Mr.") != 0 &&
		//	_tcscmp(m_ActInfo.szTitle, L"Mrs.") != 0 &&
		//	_tcscmp(m_ActInfo.szTitle, L"Ms.") != 0)
		//{
		//	dwRet = 0x5;
		//	goto Cleanup;
		//}

		if (m_ActInfo.dwProductNo != theApp.m_dwProductID)
		{
			dwRet = 0x6;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			dwRet = 0x6;
			goto Cleanup;
		}
	Cleanup:

		if (h_iSpyAV_User)
			RegCloseKey(h_iSpyAV_User);
		if (h_iSpyAV)
			RegCloseKey(h_iSpyAV);

		h_iSpyAV_User = h_iSpyAV = NULL;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::GetRegistrationDataFromRegistry", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : DecryptRegistryData
*  Description    : Decrypt data of buffer for registration data.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWardWizUIApp::DecryptRegistryData(LPBYTE lpBuffer, DWORD dwSize)
{
	try
	{
		if (IsBadWritePtr(lpBuffer, dwSize))
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			if (lpBuffer[iIndex] != 0)
			{
				if ((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
				{
					lpBuffer[iIndex] = lpBuffer[iIndex];
				}
				else
				{
					lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
					jIndex++;
				}
				if (jIndex == WRDWIZ_KEYSIZE)
				{
					jIndex = 0x00;
				}
				if (iIndex >= dwSize)
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::DecryptRegistryData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : GetRegistrationDatafromFile                                                     
*  Description    : Get user registration data from file.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWardWizUIApp::GetRegistrationDatafromFile( )
{
	try
	{
		DWORD	dwRet = 0x01;
		CString	strUserRegFile = GetModuleFilePath();
		strUserRegFile = strUserRegFile + L"\\VBUSERREG.DB";
		dwRet = GetRegistrationDatafromFile(strUserRegFile);
		if (!dwRet)
			return dwRet;

		TCHAR	szAllUserPath[512] = { 0 };
		TCHAR	szSource[512] = { 0 };
		TCHAR	szSource1[512] = { 0 };
		TCHAR	szDestin[512] = { 0 };
		TCHAR	szDestin1[512] = { 0 };

		OSVERSIONINFO 	OSVer = { 0 };
		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 511);
		OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&OSVer);
		if (OSVer.dwMajorVersion > 5)
			wsprintf(szDestin, L"%s\\Wardwiz Antivirus", szAllUserPath);
		else
			wsprintf(szDestin, L"%s\\Application Data\\Wardwiz Antivirus", szAllUserPath);

		wcscpy(szDestin1, szDestin);
		wcscat(szDestin1, L"\\VBUSERREG.DB");
		dwRet = 0x01;
		dwRet = GetRegistrationDatafromFile(szDestin1);
		if (!dwRet)
			return dwRet;

		TCHAR	szDrives[256] = { 0 };
		GetLogicalDriveStrings(255, szDrives);

		TCHAR	*pDrive = szDrives;

		while (wcslen(pDrive) > 2)
		{
			dwRet = 0x01;
			memset(szDestin1, 0x00, 512 * sizeof(TCHAR));
			wsprintf(szDestin1, L"%sVBUSERREG.DB", pDrive);
			if ((GetDriveType(pDrive) & DRIVE_FIXED) == DRIVE_FIXED)
			{
				dwRet = GetRegistrationDatafromFile(szDestin1);
				if (!dwRet)
					return dwRet;
			}
			pDrive += 4;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::GetRegistrationDatafromFile", 0, 0, true, SECONDLEVEL);
	}
	return 0x01 ;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDatafromFile
*  Description    : Get User registration data from file.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWardWizUIApp::GetRegistrationDatafromFile(CString strUserRegFile)
{
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	DWORD	dwRet = 0x00, dwBytesRead = 0x00;
	DWORD	dwSize = sizeof(m_ActInfo);
	hFile = CreateFile(strUserRegFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwRet = 0x01;
		goto Cleanup;
	}

	ZeroMemory(&m_ActInfo, sizeof(m_ActInfo));
	ReadFile(hFile, &m_ActInfo, dwSize, &dwBytesRead, NULL);
	if (dwSize != dwBytesRead)
	{
		dwRet = 0x02;
		goto Cleanup;
	}

	if (DecryptRegistryData((LPBYTE)&m_ActInfo, dwSize))
	{
		dwRet = 0x04;
		goto Cleanup;
	}

	//if (_tcscmp(m_ActInfo.szTitle, L"Mr.") != 0 &&
	//	_tcscmp(m_ActInfo.szTitle, L"Mrs.") != 0 &&
	//	_tcscmp(m_ActInfo.szTitle, L"Ms.") != 0)
	//{
	//	dwRet = 0x5;
	//	goto Cleanup;
	//}

	if (m_ActInfo.dwProductNo != theApp.m_dwProductID)
	{
		dwRet = 0x6;
		goto Cleanup;
	}

	//Match here the machine ID with Stored machineID
	//if someone provides the DB files from other computer then it works so 
	//necessary to check for machine ID.
	if (!CheckForMachineID(m_ActInfo))
	{
		memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
		dwRet = 0x6;
		goto Cleanup;
	}
Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : SingleInstanceCheck
*  Description    : Check whether wardwiz instance is exist or not
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 sep 2013
****************************************************************************************************/
bool CWardWizUIApp::SingleInstanceCheck()
{
	//Issue: 0000057: UI doesn't get launch Immediately.
	//Resolved By : Nitin K 17th March 2015
	m_hMutexHandle = CreateMutex(NULL, TRUE, L"{3E4EBB94-1B9C-4d7c-99BB-ED5576FC3FFA}");
	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

/***************************************************************************
Function Name  : StartDataCryptOpr
Description    : Start the Crypt operation (If 1 instance is already running and we click on Right click-> Encrypt || Decrypt then stop previous operation and launch new Operation)
Author Name    : Nitin K
SR_NO		   :
Date           : 15th June 2015
Modification   :
****************************************************************************/
void CWardWizUIApp::StartDataCryptOpr(int iDataOpr, CString csDataCryptFilePath)
{
	try
	{
		SendDataCryptOpr2Gui(DATA_ENC_DEC_OPERATIONS, iDataOpr, csDataCryptFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::StartDataCryptOpr", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : StartScheduledScan
Description    : To start scheduled scan
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 10 November 2017
***********************************************************************************************/
void CWardWizUIApp::StartScheduledScan()
{
	try
	{
		DWORD dwScanType = m_dwScanType;
		SendData2StartScheduledScan(START_SCHEDULED_SCAN, dwScanType, m_bSchedScanShutDown);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::StartScheduledScan", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : StartScheduledScan4RegOpt
Description    : To start scheduled scan for registry optimizer
SR.NO		   :
Author Name    : Amol Jaware
Date           : 27 March 2019
***********************************************************************************************/
void CWardWizUIApp::StartScheduledScan4RegOpt(CString csRegOptList)
{
	try
	{
		DWORD dwScanType = m_dwScanType;
		SendData2StartScheduledScan4RegOpt(START_SCHEDULED_SCAN, dwScanType, m_bSchedScanShutDown, csRegOptList);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::StartScheduledScan4RegOpt", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : SendDataCryptOpr2Gui
Description    : Send the Crypt operation request to existing UI
Author Name    : Nitin K
SR_NO		   :
Date           : 15th June 2015
Modification   :
****************************************************************************/
bool CWardWizUIApp::SendDataCryptOpr2Gui(int iMessageInfo, DWORD dwDataOpr, CString csDataCryptFilePath)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = dwDataOpr;
		wcscpy_s(szPipeData.szFirstParam, csDataCryptFilePath);
		CISpyCommunicator objCom(UI_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CWardwizUIApp::SendDataCryptOpr2Gui");
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::SendDataCryptOpr2Gui", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReadRegistryEntryofStartUp
*  Description    : Read registry of start up settings.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 09 Aug 2016
****************************************************************************************************/
bool CWardWizUIApp::ReadRegistryEntryofStartUp()
{
	HKEY hKey;
	LONG ReadReg;
	DWORD dwvalueSType = 0;
	DWORD dwvalueSize = sizeof(DWORD);
	DWORD dwChkvalueForTip = 0, dwChkvalueForScan = 0, dwChkvalueForDemoEdition = 0;
	DWORD dwType = REG_DWORD;
	try
	{ 
		if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Vibranium"), &hKey) != ERROR_SUCCESS)
		{
			return false;
		}
		ReadReg = RegQueryValueEx(hKey, L"dwShowStartupTips", NULL, &dwType, (LPBYTE)&dwvalueSType, &dwvalueSize);
		dwChkvalueForTip = (DWORD)dwvalueSType;
		if (dwChkvalueForTip == 0)
		{
			m_bAllowStartUpTip = false;
		}
		else
		{
			m_bAllowStartUpTip = true;
		}

		ReadReg = RegQueryValueEx(hKey, L"dwStartUpScan", NULL, &dwType, (LPBYTE)&dwvalueSType, &dwvalueSize);
		dwChkvalueForScan = (DWORD)dwvalueSType;
		if (dwChkvalueForScan == 0)
		{
			m_bAllowStartUpScan = false;
		}
		else
		{
			m_bAllowStartUpScan = true;
		}

		ReadReg = RegQueryValueEx(hKey, L"dwWardWizDemo", NULL, &dwType, (LPBYTE)&dwvalueSType, &dwvalueSize);
		dwChkvalueForDemoEdition = (DWORD)dwvalueSType;
		if (dwChkvalueForDemoEdition == 0)
		{
			m_bAllowDemoEdition = false;
		}
		else
		{
			m_bAllowDemoEdition = true;
		}
		RegCloseKey(hKey);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizUIApp::ReadRegistryEntryofStartUp", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : AddUserAndSystemInfoToLog
Description    : Adds Computer name, logged user name and OS details to log at the top
Author Name    : Vilas Suvarnakar
SR_NO		   :
Date           : 04 May 2015
Modification   :
****************************************************************************/
void CWardWizUIApp::AddUserAndSystemInfoToLog()
{
	try
	{
		DWORD dwType = REG_SZ;
		HKEY hKey = NULL; 
		TCHAR szvalue[1024];
		DWORD dwvalue_length = 1024;
		TCHAR	szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return;
		}

		TCHAR	szTemp[512] = { 0 };
		swprintf_s(szTemp, _countof(szTemp), L"%s\\Log\\%s", szModulePath, LOG_FILE);
		if (PathFileExists(szTemp))
			return;

		WardWizSystemInfo	objSysInfo;
		objSysInfo.GetSystemInformation();
		LPCTSTR		lpSystemName = objSysInfo.GetSystemName();
		LPCTSTR		lpUserName = objSysInfo.GetUserNameOfSystem();
		LPCTSTR		lpOSDetails = objSysInfo.GetOSDetails();
		LPCTSTR		lpEmailID = GetRegisteredEmailID();
		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"%s\n", L"--------------------------------------------------------------------------------------------------------");
		AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);

		if (lpSystemName)
		{
			ZeroMemory(szTemp, sizeof(szTemp));
			swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* Computer name:%s\n", lpSystemName);
			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		}

		if (lpUserName)
		{
			ZeroMemory(szTemp, sizeof(szTemp));
			swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* Logged user name:%s\n", lpUserName);
			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		}

		if (lpOSDetails)
		{
			ZeroMemory(szTemp, sizeof(szTemp));
			swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* OS details:%s\n", lpOSDetails);
			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		}

		if (lpEmailID)
		{
			ZeroMemory(szTemp, sizeof(szTemp));
			swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* Email ID:%s\n", lpEmailID);
			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		}
		else
		{
			swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* Email ID:NA\n");
			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		}

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
		{
			return;
		}
		dwvalue_length = 1024;
		long ReadReg = RegQueryValueEx(hKey, L"AppVersion", NULL, &dwType, (LPBYTE)&szvalue, &dwvalue_length);
		if (ReadReg == ERROR_SUCCESS)
		{
			swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* Product Version:%s\n", szvalue);
			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		}
		dwvalue_length = 1024;
		ReadReg = RegQueryValueEx(hKey, L"DataBaseVersion", NULL, &dwType, (LPBYTE)&szvalue, &dwvalue_length);
		if (ReadReg == ERROR_SUCCESS)
		{
			swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* DataBase Version:%s\n", szvalue);
			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		}
		dwvalue_length = 1024;
		ReadReg = RegQueryValueEx(hKey, L"LastLiveupdatedt", NULL, &dwType, (LPBYTE)&szvalue, &dwvalue_length);
		if (ReadReg == ERROR_SUCCESS)
		{
			swprintf_s(szTemp, _countof(szTemp), L"\t\t\t* Last Update Date:%s\n", szvalue);
			AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
		}
		ZeroMemory(szTemp, sizeof(szTemp)); 
		swprintf_s(szTemp, _countof(szTemp), L"%s\n\n", L"--------------------------------------------------------------------------------------------------------");
		AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::AddUserAndSystemInfoToLog", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckForMachineID
*  Description    : function which compare with Machine ID present in DB files & machine ID in registry.
*  Author Name    : Ram
*  SR_NO		  :
*  Date           : 18 Sep 2013
****************************************************************************************************/
bool CWardWizUIApp::CheckForMachineID(const AVACTIVATIONINFO	&actInfo)
{
	try
	{
		CITinRegWrapper objReg;
		TCHAR szValue[0x80] = { 0 };
		DWORD dwSize = sizeof(szValue);
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"MVersion", szValue, dwSize) != 0)
		{
			AddLogEntry(L"### Error in GetRegistryValueData CRegistrationSecondDlg::CheckForMachineID", 0, 0, true, SECONDLEVEL);
			return false;
		}

		//Need to compare the string fron 2nd character, as we are storing first character with string length.
		//also need to compare with machine ID added with more charaters in the registry, example like
		//we firstly installed 1.8 setup in that MAC address not present, then we added MAC Address in further
		//releases then Machine ID string will get increased in registry but not in DB files.
		//In this case we need to compare with DB file machine ID with new generated Machine ID with same legnth 
		//of Machine id which is present in DB files.
		if (memcmp(&actInfo.szClientID[1], &szValue[1], wcslen(actInfo.szClientID) * sizeof(TCHAR)) != 0)
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::CheckForMachineID");
	}
	return true;
}

/***************************************************************************
Function Name  : SendData2StartScheduledScan
Description    : Function which sends data to another instance of UI application to start scheduled scan.
Author Name    : Jeena Mariam Saji
SR_NO		   :
Date           : 10th Nov 2017
****************************************************************************/
bool CWardWizUIApp::SendData2StartScheduledScan(int iMessageInfo, DWORD dwScanType, bool IsShutDownSchedScan)
{
	try
	{ 
		DWORD dwShutDownValue = 0;
		if (IsShutDownSchedScan)
			dwShutDownValue = 1;
		else
			dwShutDownValue = 0;
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = dwScanType;
		szPipeData.dwSecondValue = dwShutDownValue;
		CISpyCommunicator objCom(UI_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardwizUIApp::SendData2StartScheduledScan");
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::SendData2StartScheduledScan", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : SendData2StartScheduledScan4RegOpt
Description    : Function which sends data to another instance of UI application to start reigstry optimizer scheduled scan.
Author Name    : Amol Jaware
SR_NO		   :
Date           : 27 March 2019
****************************************************************************/
bool CWardWizUIApp::SendData2StartScheduledScan4RegOpt(int iMessageInfo, DWORD dwScanType, bool IsShutDownSchedScan, CString csRegOptList)
{
	try
	{
		DWORD dwShutDownValue = 0;
		if (IsShutDownSchedScan)
			dwShutDownValue = 1;
		else
			dwShutDownValue = 0;
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = dwScanType;
		szPipeData.dwSecondValue = dwShutDownValue;
		_tcscpy(szPipeData.szFirstParam, csRegOptList);
		CISpyCommunicator objCom(UI_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardwizUIApp::SendData2StartScheduledScan4RegOpt");
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::SendData2StartScheduledScan4RegOpt", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************
Function Name  : GetCurrentUserName
Description    : Function returns current username.
Author Name    : Kunal Waghmare
Date           : 26 Jun 2018
****************************************************************************/
LPCTSTR CWardWizUIApp::GetCurrentUserName()
{
	WardWizSystemInfo	objSysInfo;
	try
	{
		objSysInfo.GetSystemInformation();
		return objSysInfo.GetUserNameOfSystem();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::GetCurrentUserName", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}