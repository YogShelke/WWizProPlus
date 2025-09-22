// USBDetectUI.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "USBDetectUI.h"
#include "USBDetectUIDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUSBDetectUIApp
//Neha Gharge 23 th May 2013, On the F1 button it shold show wardwiz help file
BEGIN_MESSAGE_MAP(CUSBDetectUIApp, CWinApp)
	ON_COMMAND(ID_HELP, &CUSBDetectUIApp::OnBnClickedButtonHelp)
END_MESSAGE_MAP()

HANDLE g_hWorkingThread = NULL;

//Ram: No need of crash report library
//CrashThreadInfo g_CrashThreadInfo;

/***************************************************************************
  Function Name  : CUSBDetectUIApp
  Description    : Constructor
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 :
****************************************************************************/
CUSBDetectUIApp::CUSBDetectUIApp() 
	//m_objWardWizCrashHandler(L"WardWiz Scan UI", CUSBDetectUIApp::CrashCallback)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_dwSelectedLangID = 0;
	m_bAllowDemoEdition = false;
	m_hResDLL = NULL;
	m_hRegistrationDLL = NULL;
	m_eScanLevel = CLAMSCANNER;
	m_dwProdID = 0x00;
	m_objCompleteEvent.ResetEvent();
}

/***************************************************************************
  Function Name  : CUSBDetectUIApp
  Description    : D'tor
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 :
****************************************************************************/
CUSBDetectUIApp::~CUSBDetectUIApp()
{
	if(m_hResDLL != NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
	if(m_hRegistrationDLL != NULL)
	{
		FreeLibrary(m_hRegistrationDLL);
		m_hRegistrationDLL = NULL;
	}
}
// The one and only CUSBDetectUIApp object

CUSBDetectUIApp theApp;


/***************************************************************************
  Function Name  : InitInstance
  Description    : This function is called to make doModal() call of CUSBDetectUIDlg object
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0001
  Modification Date : Neha Gharge (Right click scan)18-3-2015
****************************************************************************/
BOOL CUSBDetectUIApp::InitInstance()
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
	OleInitialize(NULL);
	CWinApp::InitInstance();


	//Get here registry path
	m_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

	sciter::sync::gui_thread_ctx _; // instance of gui_thread_ctx
	// it should be created as a variable inside WinMain 
	// gui_thread_ctx is critical for GUI_CODE_START/END to work

	AfxEnableControlContainer();

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

	m_dwSelectedLangID = m_objwardwizLangManager.GetSelectedLanguage();
	m_dwProdID = m_objwardwizLangManager.GetSelectedProductID();
	m_csAppPath = GetWardWizPathFromRegistry();

	CreateFonts();
	LoadResourceDLL();
	CheckiSPYAVRegistered();
	CheckScanLevel();
	LoadReqdLibrary();

	if(!ReadDemoEditionEntry())
	{
		m_bAllowDemoEdition = false;
	}
	else
	{
		m_bAllowDemoEdition = true;
	}

	bool bUSBScan = false, bFolderScan = false, bQuickFullScan = false;
	bool bEPSScan = false;
	int iStartPos = 0 , iSpacePos;
	CString csScanOption = L"";
	CString csCommandLine =  GetCommandLine();
	if(csCommandLine.Find('-') != -1)
	{
		csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
		csCommandLine.Trim(); 
		if (csCommandLine.CompareNoCase(TEXT("EPSNOUI")) >= 0)
		{
			csCommandLine.Replace(TEXT("EPSNOUI"), L"");
			csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
			csCommandLine.MakeLower();
			csCommandLine.Trim();
			if (csCommandLine.CompareNoCase(TEXT("QUICKSCAN")) >= 0 || csCommandLine.CompareNoCase(TEXT("FULLSCAN")) >= 0)
			{
				bQuickFullScan = true;
			}
		}
		if (!bQuickFullScan)
		{
			iSpacePos = csCommandLine.Find(_T(' '),iStartPos);
			csScanOption = csCommandLine;
			csCommandLine.Truncate(iSpacePos);
			csCommandLine.MakeLower();
			csCommandLine.Trim();
			if(csCommandLine.GetLength() > 0)
			{
				if (_tcscmp(csCommandLine, L"usbscan") == 0)
				{
					bUSBScan = true;
				}
				if (_tcscmp(csCommandLine, L"customscan") == 0)
				{
					bFolderScan = true;
				}
				if (csCommandLine.CompareNoCase(TEXT("customscan")) >= 0)
				{
					bEPSScan = true;
				}
			}
		}
	}
	else
	{
		ShellExecute(NULL, L"open", L"VBUI.EXE", NULL, NULL, SW_SHOW);
	}
	
	if(!bUSBScan && !bFolderScan && !bEPSScan && !bQuickFullScan)
	{
		return FALSE;		
	}

	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

	CUSBDetectUIDlg dlg;
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

	//********************ISSUE NO : 286 Neha Gharge 23/5/14 ***************************************/
	HWND hWindow = ::FindWindow(NULL,L"VBUSBDETECTUI.exe");
	if(!hWindow)
	{
		AddLogEntry(L"### VBUSBDETECTUI.exe handle is not available", 0, 0, true, SECONDLEVEL);
		return FALSE;
	}
	::ShowWindow(hWindow,SW_RESTORE);
	::BringWindowToTop(hWindow);
	::SetForegroundWindow(hWindow);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

/***************************************************************************
  Function Name  : GetModuleFilePath
  Description    : Function returns module path
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0002
****************************************************************************/
CString CUSBDetectUIApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***************************************************************************
  Function Name  : LoadResourceDLL
  Description    : Function loads the ResourceDLL
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0003
****************************************************************************/
void CUSBDetectUIApp::LoadResourceDLL()
{
	CString	csWardWizModulePath = GetModuleFilePath() ;

	CString	csWardWizResourceDLL = L"" ;
	csWardWizResourceDLL.Format( L"%s\\VBRESOURCE.DLL", csWardWizModulePath ) ;
	if( !PathFileExists( csWardWizResourceDLL ) )
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCEDLL_NOTFOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if( !m_hResDLL )
	{
		m_hResDLL = LoadLibrary( csWardWizResourceDLL ) ;
		if(!m_hResDLL)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCE_MODULE_LOAD_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}

}

/***************************************************************************
  Function Name  : CheckiSPYAVRegistered
  Description    : Function checks is the product registered or not
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0004
****************************************************************************/
DWORD CUSBDetectUIApp::CheckiSPYAVRegistered( )
{
	DWORD	dwRet = 0x00 ;

	CString	striSPYAVPath = GetModuleFilePath() ;

	CString	strRegistrationDLL = L"" ;
 
	strRegistrationDLL.Format( L"%s\\VBREGISTRATION.DLL", striSPYAVPath ) ;
	if( !PathFileExists( strRegistrationDLL ) )
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if( !m_hRegistrationDLL )
	{
		m_hRegistrationDLL = LoadLibrary( strRegistrationDLL ) ;
		if(!m_hRegistrationDLL)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_LOAD_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}

	m_GetDaysLeft	= (GETDAYSLEFT ) GetProcAddress( m_hRegistrationDLL, "GetDaysLeft" ) ;
	m_PerformRegistration	= (PERFORMREGISTRATION ) GetProcAddress( m_hRegistrationDLL, "PerformRegistration" ) ;

	if( !m_GetDaysLeft )
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_GET_DAYS_LEFT_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if( !m_PerformRegistration )
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_PERFORM_REGISTRATION_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}
	m_dwDaysLeft = m_GetDaysLeft( m_dwProdID ) ;

	m_lpLoadDoregistrationProc = (EXPIRYMSGBOX)GetProcAddress(m_hRegistrationDLL, "ShowEvalutionExpiredMsg");

	if(!m_lpLoadDoregistrationProc)
	{
		FreeLibrary( m_hRegistrationDLL ) ;
		m_lpLoadDoregistrationProc = NULL;
		m_hRegistrationDLL = NULL;
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_FAILED_LOADSIGNATURES"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		return dwRet;
	}
	return dwRet ;
}

/***************************************************************************
  Function Name  : CreateFonts
  Description    : To generate fonts according to OS type
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0005
****************************************************************************/
void CUSBDetectUIApp::CreateFonts()
{
	DWORD OSType = m_objOSVersionWrap.DetectClientOSVersion();
	DWORD LanguageType = m_dwSelectedLangID;
	CreateFontsFor(OSType,LanguageType);
}

/***************************************************************************
  Function Name  : CreateFontsFor
  Description    : Function used to generate fonts according to OS type
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0006
****************************************************************************/
void CUSBDetectUIApp::CreateFontsFor(DWORD OSType,DWORD LanguageType)
{
	switch(OSType)
	{
	case WINOS_95:
		break;
	case WINOS_98:
		break;
	case WINOS_2000:
		break;
	case WINOS_NT:
		switch(LanguageType)
		{
		case HINDI:

			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));


			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:

			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));
			break; 
		}
		break;
	case WINOS_VISTA:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));
			break;
		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:

			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));

			break; 
		}
		break;
	case WINOS_XP:
	case WINOS_XP64:   //Name:Varada Ikhar, Date:13/01/2015, Version: 1.8.3.12, Issue No:1, Description:In Windows XP for Basic ,after clicking ok on tooltip gives dialog "new os or old os".
		switch(LanguageType)
		{
		case HINDI:

			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));

			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:

			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));

			break; 
		}
		break;
	case WINOS_WIN7:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));
			m_fontWWTextStatus.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextSubTitle.CreatePointFont(160,_T("Verdana"),0);
			break;
		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWLogoHeader.CreatePointFont(150,_T("Microsoft Sans serif Regular"));
			m_fontWWTextStatus.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			break; 
		}
		break;
	case WINOS_WIN8:
		switch(LanguageType)
		{
		case HINDI:

			m_fontWWTextNormal.CreatePointFont(115,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));

			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:

			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));

			break; 
		}
		break;
	case WINOS_WIN8_1:
		switch(LanguageType)
		{
		case HINDI:

			m_fontWWTextNormal.CreatePointFont(115,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));

			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:

			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));


			break; 
		}
		break;
	case WINOSUNKNOWN_OR_NEWEST:
		switch(LanguageType)
		{
		case HINDI:

			m_fontWWTextNormal.CreatePointFont(115,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));

			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:

			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Microsoft Sans serif Regular"));

			break;
		}
		break;
	}

}

/***************************************************************************
  Function Name  : ShowEvaluationExpiredMsg
  Description    : Function used to check is product expired or not
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0007
****************************************************************************/
bool CUSBDetectUIApp::ShowEvaluationExpiredMsg()
{
	if(!m_lpLoadDoregistrationProc)
	{
		return false;
	}
	// Issue no 1211..
	//Function is now returning dword value. The action will be execute according to that
	if (m_lpLoadDoregistrationProc(false) == IDOK)
	{
		if (!m_PerformRegistration)
		{
			return false;
		}
		m_PerformRegistration();
	}
	m_dwDaysLeft = m_GetDaysLeft( m_dwProdID ) ;
	return false;
}

/***************************************************************************
  Function Name  : GetDaysLeft
  Description    : Function used to get the number of days left to product get expired
  Author Name    : Neha 
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0008
****************************************************************************/
DWORD CUSBDetectUIApp::GetDaysLeft()
{
	if(m_GetDaysLeft != NULL)
	{
		m_dwDaysLeft = m_GetDaysLeft( m_dwProdID );
		return m_dwDaysLeft;
	}
	return 0;
}

bool CUSBDetectUIApp::ReadDemoEditionEntry()
{
	CITinRegWrapper objReg;
	DWORD dwDemoEditionEntry = 0;
	if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwWardWizDemo", dwDemoEditionEntry) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwVibraniumDemo in CUSBDetectUIApp::ReadDemoEditionEntry", 0, 0, true, SECONDLEVEL);;
		return false;
	}

	if(dwDemoEditionEntry == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/***************************************************************************
  Function Name  : CrashCallback
  Description    : Function used when USB UI is crashed
  Author Name    : RamKrushna Shelke.
  Date           : 15 July 2014
  SR_NO			 : WRDWIZUSBUI_0009
****************************************************************************/
//void CUSBDetectUIApp::CrashCallback(LPVOID lpvState)
//{
//    UNREFERENCED_PARAMETER(lpvState);
//	AddLogEntry(L"### WardWiz USB UI application crashed");
//}

/***********************************************************************************************
  Function Name  : CheckScanLevel
  Description    : To check scan level 1-> Only with wardwiz scanner
									   2-> with clam scanner and second level scaner is wardwiz scanner.
  SR.NO			 : 
  Author Name    : Neha gharge
  Date           : 1-19-2015
***********************************************************************************************/
void CUSBDetectUIApp::CheckScanLevel()
{
	DWORD dwScanLevel = 0;
	if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwScanLevel", dwScanLevel) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CUSBDetectUIApp::CheckScanLevel", 0, 0, true, SECONDLEVEL);;
		return;
	}

	switch(dwScanLevel)
	{
	case 0x01:
		m_eScanLevel = WARDWIZSCANNER;
		break;
	case 0x02:
		m_eScanLevel = CLAMSCANNER;
		break;
	default: 
		AddLogEntry(L" ### Scan Level option is wrong. Please reinstall setup of GenX",0,0,true,SECONDLEVEL);
		break;
	}
}

/***************************************************************************************************
*  Function Name  : ShowRestartMsgOnProductUpdate
*  Description	  : If user updates the product but don't restart, and try to perform any operation, then message to 'Restart Computer' should pop-up.
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 17 March 2015
****************************************************************************************************/
bool CUSBDetectUIApp::ShowRestartMsgOnProductUpdate()
{
	try
	{
		TCHAR  szAllUserPath[255] = { 0 };
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
			if (theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG1") == L"" && theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG2") == L"" && theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG3") == L"")
			{
				csRestartRequired.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG2"), theApp.m_objwardwizLangManager.GetString(L"IDS_GUI_RESTART_OR_CONTINUE"));
				IdReturn = MessageBox(hwnd, csRestartRequired, theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG1"), MB_ICONQUESTION | MB_YESNO);
			}
			else
			{
				csRestartRequired.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG2"), theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG3"));
				IdReturn = MessageBox(hwnd, csRestartRequired, theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG1"), MB_ICONQUESTION | MB_YESNO);
			}
			if(IdReturn == IDYES)
			{
				CEnumProcess enumproc;
				enumproc.RebootSystem(0);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIApp in ShowRestartMsgOnProductUpdate", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonHelp
*  Description    : It shows help files on the basis of edition.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 05 May 2015
****************************************************************************************************/
void CUSBDetectUIApp::OnBnClickedButtonHelp()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFullPath[MAX_PATH] = { 0 };

		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		_tcscpy(szFullPath, szModulePath);
		switch (m_dwProdID)
		{
			// Issue: CHM(Help) Files should be available in german language as well
			//Resolved By: Nitin K Date: 11th Jan 2016
		case ESSENTIAL:
			switch (m_dwSelectedLangID)
			{
			case ENGLISH:  _tcscat_s(szFullPath, L"\\WrdWizAVEssential.chm");
				break;
			case GERMAN:
				_tcscat_s(szFullPath, L"\\WrdWizAVEssentialGerman.chm");
				if (!PathFileExists(szFullPath))
				{
					_tcscpy(szFullPath, L"\0");
					_tcscpy(szFullPath, szModulePath);
					_tcscat_s(szFullPath, L"\\WrdWizAVEssential.chm");
				}
				break;
			default: _tcscat_s(szFullPath, L"\\WrdWizAVEssential.chm");
				break;
			}
			break;
		case PRO:
			switch (m_dwSelectedLangID)
			{
			case ENGLISH:  _tcscat_s(szFullPath, L"\\WrdWizAVPro.chm");
				break;
			case GERMAN:  _tcscat_s(szFullPath, L"\\WrdWizAVProGerman.chm");
				if (!PathFileExists(szFullPath))
				{
					_tcscpy(szFullPath, L"\0");
					_tcscpy(szFullPath, szModulePath);
					_tcscat_s(szFullPath, L"\\WrdWizAVPro.chm");
				}
				break;
			default: _tcscat_s(szFullPath, L"\\WrdWizAVPro.chm");
				break;
			}
			break;
		case ELITE:_tcscat(szFullPath, L"\\WrdWizAVElite.chm");
			break;
		case BASIC:
			switch (m_dwSelectedLangID)
			{
			case ENGLISH:  _tcscat_s(szFullPath, L"\\WrdWizAVBasic.chm");
				break;
			case GERMAN:  _tcscat_s(szFullPath, L"\\WrdWizAVBasicGerman.chm");
				if (!PathFileExists(szFullPath))
				{
					_tcscpy(szFullPath, L"\0");
					_tcscpy(szFullPath, szModulePath);
					_tcscat_s(szFullPath, L"\\WrdWizAVBasic.chm");
				}
				break;
			default: _tcscat_s(szFullPath, L"\\WrdWizAVBasic.chm");
				break;
			}
			break;
		default: AddLogEntry(L"Invalid option, help file not available", 0, 0, true, SECONDLEVEL);
			break;
		}

		::ShellExecute(NULL, L"Open", szFullPath, L"", NULL, SW_SHOWNORMAL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIApp::OnBnClickedButtonHelp()", 0, 0, true, SECONDLEVEL);
	}
	
}


/***************************************************************************
Function Name  : CheckMutexOfDriverInstallation
Description    : Check Mutex Of Driver Installation
Author Name    : Neha Gharge
SR_NO		   :
Date           : 30th OCt 2015
Modification   :
****************************************************************************/
bool CUSBDetectUIApp::CheckMutexOfDriverInstallation()
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
		AddLogEntry(L"### Exception in CWardwizGUIApp::CheckMutexOfDriverInstallation", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
*  Function Name  : SendData2Service()
*  Description    : Send message to communicationService
*  Author Name    : Ramkrushna Shelke
*  SR_No		  :
*  Date           : 29/09/2016
***********************************************************************************************/
bool CUSBDetectUIApp::SendData2ComService(int iMessageInfo, DWORD dwType, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;

		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CUSBDetectUIApp::SendData2ComService");
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIApp::SendData2ComService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : LoadReqdLibrary
*  Description    : Function to load the required library, which gets called when application loads the DLL.
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date			  :	21 March 2018
****************************************************************************************************/
void CUSBDetectUIApp::LoadReqdLibrary()
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
		AddLogEntry(L"### Exception in CUSBDetectUIApp::LoadReqdLibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CalculateMD5
*  Description    : Function to calculate MD5, a DLL function .
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date			  :	21 March 2018
****************************************************************************************************/
DWORD CUSBDetectUIApp::CalculateMD5(TCHAR *pString, int iStringlen, TCHAR *pFileHash)
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
