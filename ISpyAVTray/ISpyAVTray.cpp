// ISpyAVTray.cpp : Defines the class behaviors for the application.
/***************************************************************
*  Program Name: ISpyAVTray.cpp                                                                                                    
*  Description: App class of WardWiz tray Project.
*  Author Name: Ramkrushna , Nitin , Neha , prasanna  ,prajakta.                                                                                                    
*  Date Of Creation: 18th Sep ,2013
*  Version No: 1.0.0.2
*****************************************************************/

#include "stdafx.h"
#include "ISpyAVTray.h"
#include "ISpyAVTrayDlg.h"
#include "WardWizSplashWindow.h"
#include "WardwizOSVersion.h"
#include "CSecure64.h"
#include "WrdWizSystemInfo.h"
#include "DriverConstants.h"
#include "CScannerLoad.h"
#include "Enumprocess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		IDR_REGDATA				2000

// CISpyAVTrayApp
/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0003
*  Date           : 18 Sep 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CISpyAVTrayApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

AVACTIVATIONINFO	g_ActInfo = { 0 };

// CISpyAVTrayApp construction
/***************************************************************************************************                    
*  Function Name  : CISpyAVTrayApp                                                     
*  Description    : C'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZTRAY_0001
*  Date           : 18 Sep 2013
****************************************************************************************************/
CISpyAVTrayApp::CISpyAVTrayApp():
	m_iPos(0)
{
	m_hResDLL = NULL;
	m_hRegistrationDLL = NULL;
	m_bIsShowRegTrue = false;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


/***************************************************************************************************                    
*  Function Name  : ~CISpyAVTrayApp                                                     
*  Description    : D'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO          : WRDWIZTRAY_0002
*  Date           : 18 Sep 2013
****************************************************************************************************/
CISpyAVTrayApp ::~CISpyAVTrayApp()
{
	if(m_hResDLL)
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


// The one and only CISpyAVTrayApp object

CISpyAVTrayApp theApp;


// CISpyAVTrayApp initialization
/***********************************************************************************************                    
*  Function Name  : InitInstance                                                     
*  Description    : Windows allows several copies of the same program to run at the same time.
*  Author Name    : Ramkrushna Shelke                                                                                                *
*  Date           : 7-May-2014
*  Modified		  :	Rajil
*  SR_NO          : WRDWIZTRAY_0004
*************************************************************************************************/
BOOL CISpyAVTrayApp::InitInstance()
{
	//CreateFonts();
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
	OleInitialize(NULL);
	AfxEnableControlContainer();

	//take here the registry path
	m_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
	CWardWizOSversion	objGetOSVersion;
	m_dwOsVersion = objGetOSVersion.DetectClientOSVersion();

	sciter::sync::gui_thread_ctx _; // instance of gui_thread_ctx

	//Varada Ikhar, Date: 6th May-2015
	//New Implementation : To show Release information after successful product update.
	//For calling rich edit control
	/////////////////////////////////////////////////////////////////////////
	::AfxInitRichEdit2();
	/////////////////////////////////////////////////////////////////////////
	//if (CheckMutexOfDriverInstallation())
	//{
	//	MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_DRIVER_INSTALLATION_PROCESS_INPROGRESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
	//	return FALSE;
	//}

	CExecuteProcess objExecProc;
	CString csUserName = objExecProc.GetUserNamefromProcessID(GetCurrentProcessId());
	if (SingleInstanceCheck(csUserName))
	{
		return FALSE;
	}
	
	//Issue resolved: 0000260
	// Add "About..." menu item to system menu.
	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_ONE);  // to register service for process protection

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_ONE);//WLSRV_ID_ONE to register service for process protection

	//Adding Computer name, user name and OS information at the top of tray log file
	//Added by Vilas on 5 May 2015
	AddUserAndSystemInfoToLog();

	m_dwSelectedLangID = m_objwardwizLangManager.GetSelectedLanguage();
	m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();

	CreateFonts();
	GetAppPath();
	LoadResourceDLL();
	CheckiSPYAVRegistered( );

	CString csCommandLine =  GetCommandLine();
	//MessageBox(NULL,csCommandLine,0,0);
	m_iPos = csCommandLine.ReverseFind(L'-');
	if(m_iPos >= 0) //Neha Gharge 4/1/2015 Splash screen was not displayed at the time of restart.
	{
		CString csTemp = csCommandLine.Right(csCommandLine.GetLength() - (m_iPos + 1));
		if(csTemp.Compare(L" "))
		{
			csTemp.Replace(TEXT(""),L"");
			csTemp.Trim();
		}
	//Issue -Tray Crash. Rajil 28/05/2014.,
		if((csTemp.CompareNoCase(TEXT("SHOWSPSCRN")) == 0))
		{
			CWardWizSplashWindow objWardWizSplashWindow;
			objWardWizSplashWindow.DoModal();	
			m_bIsShowRegTrue = true;
		}
	}
	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_SYSINFO);

	CISpyAVTrayDlg dlg;
	m_pMainWnd = &dlg;	
	INT_PTR nResponse  = dlg.DoModal();
	
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

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

/***************************************************************************************************                    
*  Function Name  : GetModuleFilePath                                                     
*  Description    : Get the path where module is exist
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0005
*  Date           : 18 Sep 2013
****************************************************************************************************/
CString CISpyAVTrayApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***************************************************************************************************                    
*  Function Name  : SingleInstanceCheck                                                     
*  Description    : Check whether wardwiz tray instance is exist or not
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0006
*  Date           : 18 Sep 2013
****************************************************************************************************/
bool CISpyAVTrayApp::SingleInstanceCheck(CString csUserName)
{
	HANDLE hHandle = CreateMutex(NULL, TRUE, L"{A60BD4A3-B10A-4514-8024-75E2CB78B91F}" + csUserName.MakeUpper());
	DWORD dwError = GetLastError();
	if(dwError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : CheckiSPYAVRegistered                                                     
*  Description    : Loading registrations dll and proc address of function written in registration dll
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0007
*  Date           : 18 Sep 2013
****************************************************************************************************/
DWORD CISpyAVTrayApp::CheckiSPYAVRegistered( )
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
	m_dwDaysLeft = m_GetDaysLeft( m_dwProductID ) ;

	m_lpLoadDoregistrationProc = (EXPIRYMSGBOX)GetProcAddress(m_hRegistrationDLL, "ShowEvalutionExpiredMsg");

	if(!m_lpLoadDoregistrationProc)
	{
		FreeLibrary( m_hRegistrationDLL ) ;
		m_lpLoadDoregistrationProc = NULL;
		m_hRegistrationDLL = NULL;
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_FAILED_LOADSIGNATURES"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		return dwRet;
	}

	m_lpLoadProductInformation = (DOREGISTRATION)GetProcAddress(m_hRegistrationDLL, "ShowProductInformation");

	if(!m_lpLoadProductInformation)
	{
		FreeLibrary( m_hRegistrationDLL ) ;
		m_lpLoadProductInformation = NULL;
		m_hRegistrationDLL = NULL;
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_FAILED_LOADSIGNATURES"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		return dwRet;
	}

	CString	strRegisteredDataDLL = L"" ;

	strRegisteredDataDLL.Format( L"%s\\VBREGISTERDATA.DLL", striSPYAVPath ) ;
	if( !PathFileExists( strRegisteredDataDLL ) )
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTERDATA_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	}

	if( !m_hRegisteredDataDLL )
	{
		m_hRegisteredDataDLL = LoadLibrary( strRegisteredDataDLL ) ;
	}

	m_lpLoadEmail = (GETREGISTEREFOLDERDDATA)GetProcAddress(m_hRegisteredDataDLL, "GetRegisteredData");
	if(!m_lpLoadEmail)
	{
		FreeLibrary( m_hRegistrationDLL ) ;
		m_lpLoadProductInformation = NULL;
		m_hRegistrationDLL = NULL;
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_GET_REGISTER_DATA_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		return dwRet;
	}
	return dwRet ;
}

/***************************************************************************************************                    
*  Function Name  : GetDaysLeft                                                     
*  Description    : Get Number of days left
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0008
*  Date           : 15 Jun 2014
****************************************************************************************************/
DWORD CISpyAVTrayApp::GetDaysLeft()
{
	if(m_GetDaysLeft != NULL)
	{
		m_dwDaysLeft = m_GetDaysLeft( m_dwProductID );
		return m_dwDaysLeft;
	}
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	CreateFonts                                                     
*  Description    :	Creates fonts for the project depending upon the Language selected
*  SR_NO		  : WRDWIZTRAY_0009      : 
*  Author Name    : Prasanna                                                                                          
*  Date           : 30 Apr 2014
**********************************************************************************************************/
void CISpyAVTrayApp::CreateFonts()
{

	DWORD OSType = m_objOSVersionWrap.DetectClientOSVersion();
	DWORD LanguageType = m_dwSelectedLangID;
	CreateFontsFor(OSType,LanguageType);

	/*LOGFONT lfInstallerTitle;  
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 15;
	lfInstallerTitle.lfHeight = 30;
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));	 //	   with	face name "Verdana".
	m_fontInnerDialogTitle.CreateFontIndirect(&lfInstallerTitle);	

	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 15;
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 6;
	m_fontText.CreateFontIndirect(&lfInstallerTitle);	

	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 20;
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 8;
	m_FontNonProtectMsg.CreateFontIndirect(&lfInstallerTitle);

	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 15;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 6;
	m_fontTextNormal.CreateFontIndirect(&lfInstallerTitle);	

	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 14;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 5;
	lfInstallerTitle.lfUnderline = true;
	m_fontHyperlink.CreateFontIndirect(&lfInstallerTitle);	
	
	//Edited by Prasanna-----------------------------------------------------------------------------------------
	switch(m_dwSelectedLangID)
	{
		case ENGLISH:
				//For Registry Optimizer and Data Encryption
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = 24;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = 10;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
				m_fontWWTextTitle.CreateFontIndirect(&lfInstallerTitle);

				//For all other titles
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = 22;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = 9;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
				m_fontWWTextSmallTitle.CreateFontIndirect(&lfInstallerTitle);

				//For the Sub titles
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = 18;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = 7;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
				m_fontWWTextSubTitle.CreateFontIndirect(&lfInstallerTitle);
				
				//For normal Text in the GUI
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = 15;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = 5;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));
				m_fontWWTextNormal.CreateFontIndirect(&lfInstallerTitle);	

				//For the Description given below the titles
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = 12;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = 4;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));
				m_fontWWTextSubTitleDescription.CreateFontIndirect(&lfInstallerTitle);	
				break;
		case HINDI:
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = -22;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = -18;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
				m_fontWWTextTitle.CreateFontIndirect(&lfInstallerTitle);

				//For all other titles
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = -18;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = -18;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
				m_fontWWTextSmallTitle.CreateFontIndirect(&lfInstallerTitle);

				//For the Sub titles
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = -22;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = -13;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
				m_fontWWTextSubTitle.CreateFontIndirect(&lfInstallerTitle);
				
				//For normal Text in the GUI
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = -17;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = -10;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));
				m_fontWWTextNormal.CreateFontIndirect(&lfInstallerTitle);	

				//For the Description given below the titles
				memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
				lfInstallerTitle.lfHeight = -15;
				lfInstallerTitle.lfWeight = FW_NORMAL;
				lfInstallerTitle.lfWidth = -8;
				wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));
				m_fontWWTextSubTitleDescription.CreateFontIndirect(&lfInstallerTitle);	
				break;
//Edited by Prasanna-----------------------------------------------------------------------------------------
	}*/
}

/***************************************************************************************************                    
*  Function Name  : LoadResourceDLL                                                     
*  Description    : Loading resource dll
*  Author Name    : Rajil
*  SR_NO		  : WRDWIZTRAY_0010
*  Date           : 7th May 2014
****************************************************************************************************/
void CISpyAVTrayApp::LoadResourceDLL()
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

/***************************************************************************************************                    
*  Function Name  : CreateFontsFor                                                     
*  Description    : Creates fonts according to OS type and launguage type..
*  Author Name    : Prassana
*  SR_NO		  : WRDWIZTRAY_0011
*  Date           : 27 May 2014
****************************************************************************************************/
void CISpyAVTrayApp::CreateFontsFor(DWORD OSType,DWORD LanguageType)
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
			m_fontWWTextTitle.CreatePointFont(250,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(220,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(170,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break; 
		}
		break;
	case WINOS_VISTA:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(250,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(220,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(110,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break; 
		}
		break;
	case WINOS_XP:
	case WINOS_XP64:   //Name:Varada Ikhar, Date:13/01/2015, Version: 1.8.3.12, Issue No:1, Description:In Windows XP for Basic ,after clicking ok on tooltip gives dialog "new os or old os".
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(250,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(220,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(110,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break; 
		}
		break;
	case WINOS_WIN7:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(190,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(180,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(140,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(110,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break; 
		}
		break;
	case WINOS_WIN8:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(175,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(165,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(125,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(105,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(115,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(140,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(110,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break; 
		}
		break;
	case WINOS_WIN8_1:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(175,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(165,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(145,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(105,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(115,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(140,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(110,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break; 
		}
		break;
	case WINOSUNKNOWN_OR_NEWEST:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(200,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(190,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(105,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(115,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(140,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(110,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);

			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date: 6th May-2015
			//New Implementation : To show Release information after successful product update.
			m_FontWWRelNoteTitle.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			break;
		}
		break;
	}
}



/***************************************************************************
Function Name  : AddUserAndSystemInfoToLog
Description    : Adds Computer name, logged user name and OS details to log at the top
Author Name    : Vilas Suvarnakar
SR_NO		   :
Date           : 04 May 2015
Modification   :
****************************************************************************/
void CISpyAVTrayApp::AddUserAndSystemInfoToLog()
{

	try
	{
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

		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"%s\n\n", L"--------------------------------------------------------------------------------------------------------");
		AddLogEntry(szTemp, 0, 0, false, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayApp::AddUserAndSystemInfoToLog", 0, 0, true, SECONDLEVEL);
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
bool CISpyAVTrayApp::CheckMutexOfDriverInstallation()
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

/***************************************************************************************************
*  Function Name  :  GetAppPath
*  Description    :  Get App Path
*  Author Name    :  Jeena Mariam Saji
*  Date           :  15 Sept 2016
****************************************************************************************************/
bool CISpyAVTrayApp::GetAppPath()
{
	try
	{
		TCHAR szValueAppVersion[MAX_PATH] = { 0 };
		DWORD dwSizeAppVersion = sizeof(szValueAppVersion);

		if (m_objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"AppFolder", szValueAppVersion, dwSizeAppVersion) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CWardwizTrayApp::CheckScanLevel", 0, 0, true, SECONDLEVEL);
			return false;
		}
		m_AppPath = szValueAppVersion;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayApp::GetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : GetRegisteredEmailID
*  Description    : Get registered email ID
*  Author Name    : Jeena Mariam Saji
*  SR_NO
*  Date           : 13th April 2017
****************************************************************************************************/
LPTSTR CISpyAVTrayApp::GetRegisteredEmailID()
{
	return GetRegisteredUserInformation();
}

/***************************************************************************************************
*  Function Name  : GetRegisteredUserInfo
*  Description    : Get registered User info from registry or from file.
*  Author Name    : Jeena Mariam Saji
*  SR_NO
*  Date           : 13th April 2017
****************************************************************************************************/
LPTSTR CISpyAVTrayApp::GetRegisteredUserInformation()
{
	try
	{

		DWORD	dwRet = 0x00;
		DWORD	dwSize = 0x00;
		DWORD	dwRegUserSize = 0x00;
		dwRet = GetRegistrationDataFromRegistry();
		if (!dwRet)
		{
			if (!CheckForMachineID(m_ActInfo))
			{
				memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
				return _T("");
			}
			goto Cleanup;
		}

		if (dwRet)
		{
			dwRet = GetRegistrationDatafromFile();
		}

		if (!dwRet)
		{
			if (!CheckForMachineID(m_ActInfo))
			{
				memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
				return _T("");
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
				return _T("");
			}
			dwRet = 0x00;
			goto Cleanup;
		}

		if (m_ActInfo.dwProductNo != theApp.m_dwProductID)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			dwRet = 0x02;
			goto Cleanup;
		}

	Cleanup:
		if (!dwRet)
		{
			memcpy(&g_ActInfo, &m_ActInfo, sizeof(m_ActInfo));
		}

		wcscpy(m_szRegKey, m_ActInfo.szKey);
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in CWardwizTrayApp::GetRegisteredUserInformation ", 0, 0, true, SECONDLEVEL);
	}
	return m_ActInfo.szEmailID;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDataFromRegistry
*  Description    : Get Registration data of user from registry.
*  Author Name    : Jeena Mariam Saji
*  SR_NO
*  Date           : 13th April 2017
****************************************************************************************************/
DWORD CISpyAVTrayApp::GetRegistrationDataFromRegistry()
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

		if (m_ActInfo.dwProductNo != theApp.m_dwProductID)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			dwRet = 0x02;
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
		AddLogEntry(L"### Exception in CWardwizTrayApp::GetRegistrationDataFromRegistry", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : DecryptRegistryData
*  Description    : Decrypt data of buffer for registration data.
*  Author Name    : Jeena Mariam Saji
*  SR_NO
*  Date           : 13th April 2017
****************************************************************************************************/
DWORD CISpyAVTrayApp::DecryptRegistryData(LPBYTE lpBuffer, DWORD dwSize)
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
		AddLogEntry(L"### Exception in CWardwizTrayApp::DecryptRegistryData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDatafromFile
*  Description    : Get user registration data from file.
*  Author Name    : Jeena Mariam Saji
*  SR_NO
*  Date           : 13th April 2017
****************************************************************************************************/
DWORD CISpyAVTrayApp::GetRegistrationDatafromFile()
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
		AddLogEntry(L"### Exception in CWardwizTrayApp::GetRegistrationDatafromFile", 0, 0, true, SECONDLEVEL);
	}
	return 0x01;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDatafromFile
*  Description    : Get User registration data from file.
*  Author Name    : Jeena Mariam Saji
*  SR_NO
*  Date           : 13th April 2017
****************************************************************************************************/
DWORD CISpyAVTrayApp::GetRegistrationDatafromFile(CString strUserRegFile)
{
	DWORD	dwRet = 0x00, dwBytesRead = 0x00;
	try
	{
		HANDLE	hFile = INVALID_HANDLE_VALUE;
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

		if (m_ActInfo.dwProductNo != theApp.m_dwProductID)
		{
			dwRet = 0x05;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			dwRet = 0x06;
			goto Cleanup;
		}

	Cleanup:
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in CWardwizTrayApp::GetRegistrationDatafromFile ", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}
/***************************************************************************************************
*  Function Name  : CheckForMachineID
*  Description    : function which compare with Machine ID present in DB files & machine ID in registry.
*  Author Name    : Ram
*  SR_NO		  :
*  Date           : 18 Sep 2013
****************************************************************************************************/
bool CISpyAVTrayApp::CheckForMachineID(const AVACTIVATIONINFO	&actInfo)
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
		AddLogEntry(L"### Exception in CWardwizTrayApp::CheckForMachineID");
	}
	return true;
}
