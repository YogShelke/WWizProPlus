// ISpyGUI.cpp : Defines the class behaviors for the application.
/***************************************************************
*  Program Name: ISpyGUI.cpp                                                                                                    
*  Description: App class of WardWiz Project.
*  Author Name: Ramkrushna , Nitin , Neha , prasanna  ,prajakta.                                                                                                    
*  Date Of Creation: 18th Sep ,2013
*  Version No: 1.0.0.2
*****************************************************************/

#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "WrdwizStartupTipsDlg.h"
//#include "CrashThread.h"
#include "CSecure64.h"
#include "WrdWizSystemInfo.h"
#include "DriverConstants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//const unsigned int WRDWIZ_KEY = 0x5757495A;			//WWIZ

extern AVACTIVATIONINFO	m_RegDlg_ActInfo ;
AVACTIVATIONINFO	g_ActInfo = {0} ;

#define		IDR_RESDATA_DES			7000
#define		IDR_REGDATA				2000
#define		ITIN_DATA_ENC_KEY		0x49546176

//HANDLE g_hWorkingThread = NULL;
//CrashThreadInfo g_CrashThreadInfo;

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CISpyGUIApp, CWinApp)
	/*	ISSUE NO - 653 NAME - NITIN K. TIME - 12th June 2014 */
	ON_COMMAND(ID_HELP, &CISpyGUIApp::OnBnClickedButtonUpdate)
END_MESSAGE_MAP()


/***************************************************************************************************                    
*  Function Name  : CISpyGUIApp                                                     
*  Description    : C'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
CISpyGUIApp::CISpyGUIApp():
		m_dwDaysLeft(0xFFFF)
	//,	m_objWardWizCrashHandler(L"WardWizAVUI", CISpyGUIApp::CrashCallback)
{
	m_GetDaysLeft			= NULL ;
	m_PerformRegistration	= NULL ;
	m_hRegistrationDLL		= NULL ;
	m_lpLoadEmail			= NULL;
	m_hResDLL				= NULL;
	m_dwSelectedLangID		= 0;
	m_dwProductID			= 1;
	/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2013 */
	m_bEnableSound			= false;
	m_bStartUpScan          = false;
	m_bRunFullScan 			= false;
	m_bRunQuickScan 		= false;
	m_bAllowStartUpTip 		= false;
	m_bAllowStartUpScan 	= false;
	m_bCheckScan 			= false;
	m_bAllowDemoEdition		= false;
	m_bRunLiveUpdate		= false;
	m_bShowProdExpMsgBox	= false;
	m_eScanLevel			= CLAMSCANNER;
	m_bOnCloseFromMainUI	= false; //Neha Gharge 2/4/2015 Crash occur after OnCancel function
//m_bRegistrationInProcessForboth = false;
	memset(&m_szRegKey, 0, sizeof(m_szRegKey));
	m_pbyEncDecKey = NULL;
	m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE,sizeof(unsigned char)); 
	m_hMutexHandle			= NULL;
	m_bDataCryptOpr			= false;
	m_iDataOpr				= 0;
	m_dwCryptFileCount		= 0;
	m_bIsEnDecFrmShellcmd = false;
	m_bIsFileInegrityInprogrs = false;
	m_bCrKeepOrg = true;
	m_bDialogsOpenInDataEnc = false;
	m_hCryptOprEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hUpdateOprEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bRegistrationInProcess = false;
	m_bIsPopUpDisplayed = false;
	UnzipFile = NULL;
	m_eSetupLocId = WARDWIZINDIA;
}
CISpyGUIApp theApp;

/***************************************************************************************************                    
*  Function Name  : ~CISpyGUIApp                                                     
*  Description    : D'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
CISpyGUIApp::~CISpyGUIApp(void)
{
	if(m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}
	if(m_hResDLL != NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
	if (m_hZip != NULL)
	{
		FreeLibrary(m_hZip);
		m_hZip = NULL;
	}
	if(m_hRegistrationDLL != NULL)
	{
		FreeLibrary(m_hRegistrationDLL);
		m_hRegistrationDLL = NULL;
	}
	// add by lalit 5-4-2015 memory allocated by new deleted
	if (m_SettingsReport != NULL)
	{
		delete m_SettingsReport;
		m_SettingsReport = NULL;

	}

	if (m_pSettingsScanTypeDlg != NULL)
	{
		delete m_pSettingsScanTypeDlg;
		m_pSettingsScanTypeDlg = NULL;

	}

	if (m_SettingsPassChange != NULL)
	{
		delete m_SettingsPassChange;
		m_SettingsPassChange = NULL;

	}

	if (m_hCryptOprEvent)
	{
		::CloseHandle(m_hCryptOprEvent);
		m_hCryptOprEvent = NULL;
	}
	
	if (m_hUpdateOprEvent)
	{
		::CloseHandle(m_hUpdateOprEvent);
		m_hUpdateOprEvent = NULL;
	}
}

/***************************************************************************************************                    
*  Function Name  : InitInstance                                                     
*  Description    : Windows allows several copies of the same program to run at the same time.
*  Author Name    : Ramkrushna Shelke ,Neha ,Nitin,Prasanna.
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
BOOL CISpyGUIApp::InitInstance()
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

	//Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	//Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	CISpyGUIDlg dlg;
	
	if (CheckMutexOfDriverInstallation())
	{
		MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_DRIVER_INSTALLATION_PROCESS_INPROGRESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	AddUserAndSystemInfoToLog();

	m_dwSelectedLangID = m_objwardwizLangManager.GetSelectedLanguage();
	m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
	
	CheckiSPYAVRegistered( ) ;
	LoadResourceDLL();
	CheckScanLevel();
	CheckSetupLocId();
	CreateFonts();
	LoadExtractDll();
	//Issue 786 In case wardwiz addin get unloaded. On opening of UI we are loading if its unloaded
	if (m_dwProductID == 0x02)
	{
		if (CheckAndSetLoadBehaviourOfAddin())
		{
			AddLogEntry(L"### LoadBehaviour entry either not found or not set properly", 0, 0, true, ZEROLEVEL);
		}
	}

	if(!ReadRegistryEntryofStartUp())
	{
		AddLogEntry(L"### Error in ReadRegistryEntryofStartUp", 0, 0, true, SECONDLEVEL);
	}
	
	/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
	m_bEnableSound = ReadSoundSettingFromRegistry();

	m_bStartUpScan = false;
	m_bRunFullScan = false;
	m_bRunQuickScan = false;
	m_bCheckScan = false;
	m_bSettingFlag = false;
	m_dwOSType = 0;

	CString csCommandLine =  GetCommandLine();
	if(csCommandLine.Find('-') != -1)
	{
		csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
		csCommandLine.Trim();
		if(csCommandLine.GetLength() > 0)
		{
			WaitForSingleObject(dlg.m_hWnd,1000*20);	
			if( csCommandLine.CompareNoCase(TEXT("STARTUPSCAN")) >= 0 )
			{
				m_bCheckScan = true;
				if(m_bAllowStartUpScan)
				{
					m_bStartUpScan = true;
					csCommandLine.Replace(TEXT("STARTUPSCAN"), L"");
					csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
					csCommandLine.MakeLower();
					csCommandLine.Trim();
					if( csCommandLine.Compare(TEXT("fullscan")) == 0 )
					{
						m_bRunFullScan = true;
					}
					else
					{
						m_bRunQuickScan = true;
					}
				}
			}/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */
			else if(csCommandLine.CompareNoCase(TEXT("LIVEUPDATE")) >= 0 )
			{
				m_bRunLiveUpdate = true;
			}
			else if (csCommandLine.CompareNoCase(TEXT("ENC")) >= 0)
			{
				/*CISpyPasswordPopUpDlg	m_objPasswordObj;
				m_objPasswordObj.m_bEncryptionOption = true;
				INT_PTR Ret = m_objPasswordObj.DoModal();
				if (Ret == IDCANCEL)
				{
					return FALSE;
				}*/
				m_bDataCryptOpr = true;
				m_iDataOpr = ENCRYPTION;
				csCommandLine.Delete(0, csCommandLine.Find(' ') + 1);
				m_csDataCryptFilePath = csCommandLine;
				m_bIsEnDecFrmShellcmd = true;
			}

			else if (csCommandLine.CompareNoCase(TEXT("DEC")) >= 0)
			{
				/*CISpyPasswordPopUpDlg	m_objPasswordObj;
				m_objPasswordObj.m_bEncryptionOption = false;
				INT_PTR Ret = m_objPasswordObj.DoModal();
				if (Ret == IDCANCEL)
				{
					return FALSE;
				}*/
				m_bDataCryptOpr = true;
				m_iDataOpr = DECRYPTION;
				csCommandLine.Delete(0, csCommandLine.Find(' ') + 1);
				m_csDataCryptFilePath = csCommandLine;
				m_bIsEnDecFrmShellcmd = true;
			}
		}
	}
	
	if(!m_bAllowStartUpScan && m_bCheckScan)
	{
		return FALSE;
	}
	
	if(SingleInstanceCheck())
	{
		//Issue :product name and product folder name is same so on double click of tray creating problem - Neha Gharge.
		/******************ISSUE NO : 30 Neha G 23/5/2014 **********************************/
		HWND hWindow = ::FindWindow(NULL,L"WRDWIZAVUI");
		if(!hWindow)
		{
			AddLogEntry(L"GenX handle is not available", 0, 0, true, SECONDLEVEL);
		}
		//Added By Nitin Kolapkar
		//If 1 instance is already running and we click on Right click-> Encrypt || Decrypt then stop previous operation and launch new Operation
		if (m_bDataCryptOpr == true)
		{
			StartDataCryptOpr(m_iDataOpr, m_csDataCryptFilePath);
		}
		::ShowWindow(hWindow,SW_RESTORE);
		::BringWindowToTop(hWindow);
		::SetForegroundWindow(hWindow);
		return FALSE;		
	}

	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_ZERO);//WLSRV_ID_ZERO to register service for process protection

	//CreateFonts();
/////////////////////////////////////////////////////////////////////////
	::AfxInitRichEdit2();
/////////////////////////////////////////////////////////////////////////
	//Show Tips of the day dialog here

	if(!m_bStartUpScan)
	{
		// resolved by lalit kumawat 6-20-2015
		//issue :- if i encrypt file using right click, it should not show tips of the day dlg.
		if ((m_bAllowStartUpTip) && (!m_bRunLiveUpdate) && (!m_bIsEnDecFrmShellcmd))
		{
			CWrdwizStartupTipsDlg objWardWizShowTip;
			INT_PTR nResponseTipsDlg = objWardWizShowTip.DoModal();
			if (nResponseTipsDlg == IDCANCEL)
			{
				return FALSE;
			}
		}
	}

	m_bIsEnDecFrmShellcmd = false;
	//m_objWardWizCrashHandler.Initialize();

	/* Create another thread */
	//g_CrashThreadInfo.m_bStop = false;
	//g_CrashThreadInfo.m_hWakeUpEvent = CreateEvent(NULL, FALSE, FALSE, _T("WakeUpEvent"));
	//ATLASSERT(g_CrashThreadInfo.m_hWakeUpEvent!=NULL);

	//DWORD dwThreadId = 0;
	//g_hWorkingThread = ::CreateThread(NULL, 0, CrashThread, (LPVOID)&g_CrashThreadInfo, 0, &dwThreadId);
	//ATLASSERT(g_hWorkingThread!=NULL);

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

	// Close another thread
    //g_CrashThreadInfo.m_bStop = true;
    //SetEvent(g_CrashThreadInfo.m_hWakeUpEvent);
    //// Wait until thread terminates
    //WaitForSingleObject(g_hWorkingThread, INFINITE);

    //::CoUninitialize();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


/**********************************************************************************************************                     
*  Function Name  :	CreateFonts                                                     
*  Description    :	Creates fonts for the project depending upon the Language selected
*  SR.NO          : 
*  Author Name    : Prasanna                                                                                          
*  Date           : 30 Apr 2014
**********************************************************************************************************/

void CISpyGUIApp::CreateFonts()
{
	DWORD OSType = m_objOSVersionWrap.DetectClientOSVersion();
	m_dwOSType = OSType;
	DWORD LanguageType = m_dwSelectedLangID;
	CreateFontsFor(OSType,LanguageType);
/*
	LOGFONT lfInstallerTitle;  
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
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
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
	}
	*/


	
}

/***************************************************************************************************                    
*  Function Name  : ExitInstance                                                     
*  Description    :	Called by the framework from within the Run member function to exit this instance of the application.
*  Author Name    : Ramkrushna Shelke 
*  SR_NO		  :
*  Date           : 18 sep 2013
****************************************************************************************************/
int CISpyGUIApp::ExitInstance()
{
	if( m_hRegistrationDLL != NULL)
	{
		FreeLibrary( m_hRegistrationDLL ) ;
		m_hRegistrationDLL = NULL ;
	}

	if( m_hRegisteredDataDLL != NULL)
	{
		FreeLibrary( m_hRegisteredDataDLL ) ;
		m_hRegisteredDataDLL = NULL ;
	}
	if (m_hMutexHandle != NULL)
	{
		CloseHandle(m_hMutexHandle);
		m_hMutexHandle = NULL;
	}
	return CWinApp::ExitInstance();
}

/***************************************************************************************************                    
*  Function Name  : GetModuleFilePath                                                     
*  Description    : Get the path where module is exist
*  Author Name    : Ramkrushna Shelke 
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
CString CISpyGUIApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***************************************************************************************************                    
*  Function Name  : SingleInstanceCheck                                                     
*  Description    : Check whether wardwiz instance is exist or not
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 sep 2013
****************************************************************************************************/
bool CISpyGUIApp::SingleInstanceCheck()
{
	
	//HANDLE hHandle = CreateMutex(NULL, TRUE, L"{3E4EBB94-1B9C-4d7c-99BB-ED5576FC3FFA}");
	//Issue: 0000057: UI doesn't get launch Immediately.
	//Resolved By : Nitin K 17th March 2015
	m_hMutexHandle = CreateMutex(NULL, TRUE, L"{3E4EBB94-1B9C-4d7c-99BB-ED5576FC3FFA}");
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
*  SR_NO
*  Date           : 18 sep 2013
****************************************************************************************************/
DWORD CISpyGUIApp::CheckiSPYAVRegistered( )
{
	DWORD	dwRet = 0x00 ;

	CString	striSPYAVPath = GetModuleFilePath() ;

	CString	strRegistrationDLL = L"" ;
 
	strRegistrationDLL.Format( L"%s\\WRDWIZREGISTRATION.DLL", striSPYAVPath ) ;
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
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}

	m_GetDaysLeft	= (GETDAYSLEFT ) GetProcAddress( m_hRegistrationDLL, "GetDaysLeft" ) ;
	m_PerformRegistration	= (PERFORMREGISTRATION ) GetProcAddress( m_hRegistrationDLL, "PerformRegistration" ) ;
	m_CloseRegistrationWindow	= (PERFORMREGISTRATION ) GetProcAddress( m_hRegistrationDLL, "CloseRegistrationWindow" ) ;

	if( !m_GetDaysLeft )
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_GET_DAYS_LEFT_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if( !m_PerformRegistration || !m_CloseRegistrationWindow)
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_MODULE_PERFORM_REGISTRATION_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}
	m_dwDaysLeft = m_GetDaysLeft( m_dwProductID ) ;
	//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
	//Neha Gharge 4 jan,2016
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
	// resolve by lalit kumawat 3-17-015
	//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
	//(same issue applies to change password & scan computer at start up settings
	m_lpCloseProductInformationDlg = (DOREGISTRATION)GetProcAddress(m_hRegistrationDLL, "CloseProductInformationDlg");

	CString	strRegisteredDataDLL = L"" ;

	strRegisteredDataDLL.Format( L"%s\\WRDWIZREGISTERDATA.DLL", striSPYAVPath ) ;
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
*  Description    : Get days left of registration.
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
DWORD CISpyGUIApp::GetDaysLeft()
{
	if(m_GetDaysLeft != NULL)
	{
		m_dwDaysLeft = m_GetDaysLeft( m_dwProductID );
		return m_dwDaysLeft;
	}
	return m_dwDaysLeft;
}

/***************************************************************************************************                    
*  Function Name  : ShowEvaluationExpiredMsg                                                     
*  Description    : Show popup of expiry product message.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 17 Jan 2014
****************************************************************************************************/
bool CISpyGUIApp::ShowEvaluationExpiredMsg(bool bShowAtStartUp)
{
	//issue no : 816 neha gharge 30/6/2014
	if(!m_lpLoadDoregistrationProc)
	{
		return false;
	}
	//m_bRegistrationInProcessForboth = true;
	//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
	//Neha Gharge 4 jan,2016
	theApp.m_bIsPopUpDisplayed = true;
	if (m_lpLoadDoregistrationProc(bShowAtStartUp) == IDOK)
	{
		if (!m_PerformRegistration)
		{
			theApp.m_bIsPopUpDisplayed = false;
			return false;
		}
		m_bRegistrationInProcess = true;
		m_PerformRegistration();
	}
	theApp.m_bIsPopUpDisplayed = false;
	m_bRegistrationInProcess = false;
	m_dwDaysLeft = m_GetDaysLeft( m_dwProductID );
	//m_bRegistrationInProcessForboth = false;
	if(!SendEmailPluginChange2Service(RELOAD_REGISTARTION_DAYS))
	{
		AddLogEntry(L"### Error in CSystemTray::SendEmailPluginChange2Service", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : ShowProductInformation                                                     
*  Description    : Shows product information on Main UI
*  Author Name    : Neha Gharge 
*  SR_NO
*  Date           : 17 Jan 2014
****************************************************************************************************/
bool CISpyGUIApp::ShowProductInformation()
{
	if(!m_lpLoadProductInformation)
	{
		return false;
	}
	if(!m_lpLoadProductInformation())
	{
		return false;
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
LPTSTR CISpyGUIApp::GetRegisteredEmailID()
{
	return GetRegisteredUserInfo();
}


/***************************************************************************************************                    
*  Function Name  : GetRegisteredUserInfo                                                     
*  Description    : Get registered User info from registry or from file.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
LPTSTR CISpyGUIApp::GetRegisteredUserInfo( )
{
	
	DWORD	dwRet = 0x00 ;
	DWORD	dwSize = 0x00 ;
	DWORD	dwRegUserSize = 0x00 ;
	dwRet = GetRegistrationDataFromRegistry( ) ;
	if( !dwRet )
	{
		goto Cleanup;
	}


	dwSize = sizeof(m_ActInfo) ;
	dwRegUserSize = 0x00 ;

	memset(&m_ActInfo,0x00,sizeof(m_ActInfo));

	if( m_lpLoadEmail((LPBYTE)&m_ActInfo, dwRegUserSize , IDR_REGDATA, L"REGDATA" ) == 0)
	{
		dwRet = 0x00 ;
		goto Cleanup;
	}

Cleanup:

	//if( dwRet )
	//	dwRet = GetRegistrationDataFromRegistry( ) ;

	if( dwRet )
	{
		dwRet = GetRegistrationDatafromFile( ) ;
	}

	if( dwRet )
	{
		return L"";
	}

	if( !dwRet )
		memcpy(&g_ActInfo, &m_ActInfo, sizeof(m_ActInfo) ) ;
	//AfxMessageBox(m_ActInfo.szEmailID,0,0);
	/*	ISSUE NO - 818 NAME - NITIN K. TIME - 26th June 2014 */
	if(g_ActInfo.dwProductNo != theApp.m_dwProductID)
	{
		return L"";
	}

	//Varada Ikhar, Date:09/04/2015
	//Not-protected message needs to be changed.
	//(If product is expired, then with the help of activation key we check whether it is activated or trial product.)
	wcscpy_s(m_szRegKey, m_ActInfo.szKey);

	return m_ActInfo.szEmailID ;
}

/***************************************************************************************************                    
*  Function Name  : GetRegistrationDataFromRegistry                                                     
*  Description    : Get Registration data of user from registry.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CISpyGUIApp::GetRegistrationDataFromRegistry( )
{
	DWORD	dwRet = 0x00 ;
	DWORD	dwRegType = 0x00, dwRetSize = 0x00 ;

	HKEY	h_iSpyAV = NULL ;
	HKEY	h_iSpyAV_User = NULL ;

	//if( RegCreateKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus"), 
	if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows"), 
						0, KEY_READ, &h_iSpyAV ) != ERROR_SUCCESS )
	{
		dwRet = 0x01 ;
		goto Cleanup ;
	}
	dwRetSize = sizeof(m_ActInfo) ;
	/*if( RegQueryValueEx(h_iSpyAV, TEXT("UserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo, 
						&dwRetSize) != ERROR_SUCCESS )
	{
		dwRet = GetLastError() ;
		dwRet = 0x03 ;
		goto Cleanup ;
	}*/

	if( RegQueryValueEx(h_iSpyAV, TEXT("WardWizUserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo, 
						&dwRetSize) != ERROR_SUCCESS )
	{
		dwRet = GetLastError() ;
		dwRet = 0x03 ;
		goto Cleanup ;
	}

	if( DecryptRegistryData( (LPBYTE )&m_ActInfo, sizeof(m_ActInfo)) )
	{
		dwRet = 0x04 ;
		goto Cleanup ;
	}

	if( _tcscmp(m_ActInfo.szTitle,  L"Mr.") != 0	&&
	_tcscmp(m_ActInfo.szTitle,  L"Mrs.") != 0	&&
	_tcscmp(m_ActInfo.szTitle,  L"Ms.") != 0	)
	{
		dwRet = 0x5;
		goto Cleanup;
	}


Cleanup:

	if( h_iSpyAV_User )
		RegCloseKey( h_iSpyAV_User ) ;
	if( h_iSpyAV )
		RegCloseKey( h_iSpyAV ) ;

	h_iSpyAV_User = h_iSpyAV = NULL ;

	return dwRet ;
}

//Prajakta Nanaware
/***************************************************************************************************                    
*  Function Name  : DecryptData                                                     
*  Description    : Decrypt data of buffer.
*  Author Name    : Prajakta Nanaware 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CISpyGUIApp::DecryptData( LPBYTE lpBuffer, DWORD dwSize )
{
	if( IsBadWritePtr( lpBuffer, dwSize ) )
		return 1 ;

	DWORD	iIndex = 0 ;
	DWORD jIndex = 0;

	if (lpBuffer == NULL || dwSize == 0x00)
	{
		return 1;
	}

	for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
	{
		lpBuffer[iIndex] ^= m_pbyEncDecKey[jIndex++];
		if (jIndex == WRDWIZ_KEY_SIZE)
		{
			jIndex = 0x00;
		}
		if (iIndex >= dwSize)
		{
			break;
		}
	}
	
	/*DWORD	i = 0 ;
	DWORD dwEncKey = 0x5757495A;

	if (lpBuffer == NULL || dwSize == 0x00)
	{
		return 1;
	}

	for (i = 0x00;  i < dwSize; i+= 4)
	{
		if(*((DWORD *)&lpBuffer[i]) != 0x0)
		{
			dwEncKey += LOWORD(dwEncKey);
			*((DWORD *)&lpBuffer[i]) = *((DWORD *)&lpBuffer[i]) ^ dwEncKey ;
		}
	}*/
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : CreateRandomKeyFromFile                                                     
*  Description    : creates key of size 0x10 (16 in decimal)
*  Author Name    : Prajakta Nanaware 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
// issue resolve by lalit kumawat 4-11-2015
//Text file containing only three lines or less data is not getting encrypted.
//	Steps To Reproduce	In data encryption->encrypt text file with few data->after encryption ? 
//open the encrypted file data is visible.(Data is not getting encrypted in case of txt file)

bool CISpyGUIApp::CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize)
{
	bool			bReturn = false;
	DWORD			iTmp = 0x00;
	DWORD			iIndex = 0x00, jIndex = 0x00;
	DWORD			iRandValue = 0x00, iReadPos = 0x00;

	unsigned char	szChar = 0x0;

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return bReturn;
	}

	iTmp = dwFileSize / WRDWIZ_KEY_SIZE;


	//Issue		: File size less than 16 bytes are not encrypting and some bytes are not encrypting.
	//Resolved	: Vilas
	//Date		: 14 - 01 - 2015
	
	//if( iTmp == 0x00 )
	if (iTmp < WRDWIZ_KEY_SIZE)
		iTmp = WRDWIZ_KEY_SIZE;

	if(m_pbyEncDecKey == NULL)
	{
		m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE,sizeof(unsigned char)); 
		if(m_pbyEncDecKey == NULL)
		{
			AddLogEntry(L"### Allocation failed in CGenXGUIApp::CreateRandomKeyFromFile", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}
	}
	for(iIndex = 0x00, jIndex = 0x00; iIndex < iTmp; iIndex++, jIndex++)
	{
		if (jIndex >= WRDWIZ_KEY_SIZE)
		{
			break;
		}

		szChar = 0x00;
		iRandValue = GetTickCount();//rand();
		iRandValue = iRandValue % WRDWIZ_KEY_SIZE;

		iReadPos = (iIndex *  WRDWIZ_KEY_SIZE) +  iRandValue;

		DWORD dwBytesRead = 0x0;
		SetFilePointer( hFile, iReadPos, NULL, FILE_BEGIN ) ;
		ReadFile( hFile, &szChar, sizeof(BYTE), &dwBytesRead, NULL ) ;
			
		if (szChar == 0x00)
		{
			szChar = static_cast<unsigned char>(iRandValue);
		}
		m_pbyEncDecKey[jIndex] = szChar;
		if (iIndex == (iTmp - 0x01) && jIndex < WRDWIZ_KEY_SIZE)
		{
			iIndex = 0x00;
		}
		
		Sleep(100);
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : ReadKeyFromEncryptedFile                                                     
*  Description    : Read key from any encrypted file
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
bool CISpyGUIApp::ReadKeyFromEncryptedFile(HANDLE hFile)
{
	bool	bReturn = false;
	int		iReadPos = 0x0;
	DWORD   dwBytesRead = 0x0;
	
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return bReturn;
	}
	if(!IsFileAlreadyEncrypted(hFile))
	{
		return bReturn;
	}
	//read encryption key
	if(m_pbyEncDecKey == NULL)
	{
		m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE,sizeof(unsigned char)); 
	}
	memset(m_pbyEncDecKey, 0x00, WRDWIZ_KEY_SIZE * sizeof(unsigned char));
	SetFilePointer(hFile, (0x0 + WRDWIZ_SIG_SIZE), NULL, FILE_BEGIN);
	ReadFile( hFile, &m_pbyEncDecKey[0x0], WRDWIZ_KEY_SIZE * sizeof(unsigned char), &dwBytesRead, NULL ) ;
	if (dwBytesRead != WRDWIZ_KEY_SIZE)
	{
		return bReturn;
	}
	bReturn = true;
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : IsFileAlreadyEncrypted                                                     
*  Description    : Checks given file is already encrypted or not.
*  Author Name    : Prajkta Nanaware 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
bool CISpyGUIApp::IsFileAlreadyEncrypted(HANDLE hFile)
{
	bool	bReturn = false;
	int		iReadPos = 0x0;
	DWORD   dwBytesRead = 0x0;
	unsigned char	bySigBuff[WRDWIZ_SIG_SIZE] = {0x00};

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return bReturn;
	}
	
	//check if file is already encrypted by checking existence of sig "WARDWIZ"
	SetFilePointer(hFile, iReadPos, NULL, FILE_BEGIN);
	ReadFile( hFile, &bySigBuff[0x0], WRDWIZ_SIG_SIZE * sizeof(unsigned char), &dwBytesRead, NULL ) ;
	if(dwBytesRead != WRDWIZ_SIG_SIZE)
	{
		return bReturn;
	}
	if(memcmp(&bySigBuff, WRDWIZ_SIG, WRDWIZ_SIG_SIZE) == 0)
	{
		bReturn = true;
	}
	else
	{
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : DoRegistration                                                     
*  Description    : Registartion process get started.
*  Author Name    : Ramkrushna shelke 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CISpyGUIApp::DoRegistration()
{
	__try
	{
		//issue no : 816 neha gharge 30/6/2014
		if(!m_PerformRegistration)
		{
			return;
		}
//		m_bRegistrationInProcessForboth = true;
		m_PerformRegistration( ) ;
		m_dwDaysLeft = m_GetDaysLeft( m_dwProductID );
//		m_bRegistrationInProcessForboth = false;
		if(!SendEmailPluginChange2Service(RELOAD_REGISTARTION_DAYS))
		{
			AddLogEntry(L"### Error in CSystemTray::SendEmailPluginChange2Service", 0, 0, true, SECONDLEVEL);
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CGenXGUIApp::DoRegistration", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : CloseRegistrationWindow                                                     
*  Description    : Close registration winodow when user clicks on close button.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
bool CISpyGUIApp::CloseRegistrationWindow()
{
	__try
	{
		if(!m_CloseRegistrationWindow)
		{
			return false;
		}
		return m_CloseRegistrationWindow();
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CGenXGUIApp::DoRegistration", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : LoadResourceDLL                                                     
*  Description    : Load resource dll.
*  Author Name    : Ramkrushna Shelke 
*  SR_NO		  :
*  Date           : 27 May 2014
****************************************************************************************************/
void CISpyGUIApp::LoadResourceDLL()
{
	CString	csWardWizModulePath = GetModuleFilePath() ;

	CString	csWardWizResourceDLL = L"" ;
	csWardWizResourceDLL.Format( L"%s\\WRDWIZRESOURCE.DLL", csWardWizModulePath ) ;
	if( !PathFileExists( csWardWizResourceDLL ) )
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCE_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if( !m_hResDLL )
	{
		m_hResDLL = LoadLibrary( csWardWizResourceDLL ) ;
		if(!m_hResDLL)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCE_MODULE_LOAD_FAILED") , theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}

}

/***************************************************************************************************                    
*  Function Name  : CreateFontsFor                                                     
*  Description    : Creates fonts according to OS type and launguage type.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CISpyGUIApp::CreateFontsFor(DWORD OSType,DWORD LanguageType)
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
			m_fontWWTextMediumSize.CreatePointFont(140,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;
	

		case SPANISH:
		case GERMAN:
			m_fontWWTextTitle.CreatePointFont(170, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(130, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(90, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(110, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWDaysLeftFont.CreatePointFont(380, _T("Cambria"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(170,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(110,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(200,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			m_FontWWDaysLeftFont.CreatePointFont(380,_T("Cambria"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break; 
		}
		break;
	case WINOS_VISTA:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(250,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(220,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(140,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		
		case SPANISH:
		case GERMAN:
			m_fontWWTextTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(130, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(110, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(200, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWDaysLeftFont.CreatePointFont(380, _T("Cambria"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(110,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			m_FontWWDaysLeftFont.CreatePointFont(380,_T("Cambria"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
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
			m_fontWWTextSubTitleDescription.CreatePointFont(110,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(140,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		
		case SPANISH:
		case GERMAN:
			m_fontWWTextTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(130, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(110, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(200, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWDaysLeftFont.CreatePointFont(380, _T("Cambria"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;
		case ENGLISH:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(110,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			m_FontWWDaysLeftFont.CreatePointFont(380,_T("Cambria"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break; 
		}
		break;
	case WINOS_WIN7:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(190,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(180,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(120,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(140,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC")); 
			break;

		
		case SPANISH:
		case GERMAN:
			m_fontWWTextTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(130, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			m_fontWWTextMediumSize.CreatePointFont(110, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(200, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWDaysLeftFont.CreatePointFont(380, _T("Cambria"), 0);
			break;

		case ENGLISH:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWLogoHeader.CreatePointFont(150,_T("Eras Demi ITC"));
			m_fontWWTextMediumSize.CreatePointFont(110,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			m_FontWWDaysLeftFont.CreatePointFont(380,_T("Cambria"),0);
			break; 
		}
		break;
	case WINOS_WIN8:
		switch(LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(175,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(165,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(145,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(105,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(115,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(140,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case SPANISH:
		case GERMAN:
			m_fontWWTextTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(140, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(130, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(110, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(200, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWDaysLeftFont.CreatePointFont(380, _T("Cambria"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;
		case ENGLISH:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(140,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(110,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			m_FontWWDaysLeftFont.CreatePointFont(380,_T("Cambria"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
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
			m_fontWWTextMediumSize.CreatePointFont(140,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case SPANISH:
		case GERMAN:
			m_fontWWTextTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(140, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(130, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(110, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(200, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWDaysLeftFont.CreatePointFont(380, _T("Cambria"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;
		case ENGLISH:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(140,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(110,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			m_FontWWDaysLeftFont.CreatePointFont(380,_T("Cambria"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
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
			m_fontWWTextMediumSize.CreatePointFont(140,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(100,_T("Microsoft Sans serif Regular"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150,_T("Verdana"),0);
			m_fontWWTextSmallTitle.CreatePointFont(140,_T("Verdana"),0);
			m_fontWWTextSubTitle.CreatePointFont(130,_T("Verdana"),0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextNormal.CreatePointFont(90,_T("Microsoft Sans serif Regular"),0);
			m_fontWWTextMediumSize.CreatePointFont(110,_T("Verdana"),0);
			m_FontWWStartUpFontTitle.CreatePointFont(250,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130,_T("Microsoft Sans serif Regular"),0);
			m_FontWWStartUpTips.CreatePointFont(80,_T("Microsoft Sans serif Regular"),0);
			m_FontWWDaysLeftFont.CreatePointFont(380,_T("Cambria"),0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;
		}
		break;
	}
}

/***************************************************************************************************                    
*  Function Name  : ReadRegistryEntryofStartUp                                                     
*  Description    : Read registry of start up settings.
*  Author Name    : Neha gharge ,prassana 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
bool CISpyGUIApp::ReadRegistryEntryofStartUp()
{
	HKEY hKey;
	LONG ReadReg;
	DWORD dwvalueSType  = 0;
	DWORD dwvalueSize = sizeof(DWORD);
	DWORD dwChkvalueForTip = 0 ,dwChkvalueForScan = 0,dwChkvalueForDemoEdition = 0;
	DWORD dwType=REG_DWORD;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus"),&hKey)!= ERROR_SUCCESS)
	{
		return false;
	}
	ReadReg=RegQueryValueEx(hKey,L"dwShowStartupTips",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
	dwChkvalueForTip=(DWORD)dwvalueSType;
	if(dwChkvalueForTip==0)
	{
		m_bAllowStartUpTip = false;
	}
	else
	{
		m_bAllowStartUpTip = true;
	}

	ReadReg=RegQueryValueEx(hKey,L"dwStartUpScan",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
	dwChkvalueForScan=(DWORD)dwvalueSType;
	if(dwChkvalueForScan==0)
	{
		m_bAllowStartUpScan = false;
	}
	else
	{
		m_bAllowStartUpScan = true;
	}

	ReadReg=RegQueryValueEx(hKey,L"dwWardWizDemo",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
	dwChkvalueForDemoEdition=(DWORD)dwvalueSType;
	if(dwChkvalueForDemoEdition==0)
	{
		m_bAllowDemoEdition = false;
	}
	else
	{
		m_bAllowDemoEdition = true;
	}
	if (CheckForShowProdExpMsgBox())
	{
		m_bShowProdExpMsgBox = true;
	}
	else
	{
		m_bShowProdExpMsgBox = false;
	}
	RegCloseKey(hKey);
	return true;
}



/***********************************************************************************************
*  Function Name  : SendEmailPluginChange2Service()
*  Description    : Send message to emailplugin 
*  Author Name    : Neha gharge
*  SR_No		  :
*  Date           : 31/5/2014
***********************************************************************************************/
bool CISpyGUIApp::SendEmailPluginChange2Service(int iMessageInfo, DWORD dwType,bool bEmailPluginWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = iMessageInfo;
	
	CISpyCommunicator objCom(EMAILPLUGIN_SERVER);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send Data in CGenXGUIApp::SendEmailPluginChange2Service");
		return false;
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : GetRegistrationDatafromFile                                                     
*  Description    : Get user registration data from file.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CISpyGUIApp::GetRegistrationDatafromFile( )
{

	DWORD	dwRet = 0x01 ;

	CString	strUserRegFile = GetModuleFilePath() ;
	strUserRegFile = strUserRegFile + L"\\WRDWIZUSERREG.DB" ;
	dwRet = GetRegistrationDatafromFile( strUserRegFile ) ;
	if( !dwRet )
		return dwRet ;

	TCHAR	szAllUserPath[512] = {0} ;

	TCHAR	szSource[512] = {0} ;
	TCHAR	szSource1[512] = {0} ;
	TCHAR	szDestin[512] = {0} ;
	TCHAR	szDestin1[512] = {0} ;

	OSVERSIONINFO 	OSVer = {0} ;

	GetEnvironmentVariable( L"ALLUSERSPROFILE", szAllUserPath, 511 ) ;
	OSVer.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
	GetVersionEx( &OSVer ) ;
	if( OSVer.dwMajorVersion > 5 )
		wsprintf( szDestin, L"%s\\Wardwiz Antivirus", szAllUserPath ) ;
	else
		wsprintf( szDestin, L"%s\\Application Data\\Wardwiz Antivirus", szAllUserPath ) ;


	wcscpy_s( szDestin1, szDestin ) ;
	wcscat_s( szDestin1, L"\\WRDWIZUSERREG.DB") ;

	dwRet = 0x01 ;
	dwRet = GetRegistrationDatafromFile( szDestin1 ) ;
	if( !dwRet )
		return dwRet ;


	TCHAR	szDrives[256] = {0} ;
	GetLogicalDriveStrings( 255, szDrives ) ;

	TCHAR	*pDrive = szDrives ;

	while( wcslen(pDrive) > 2 )
	{
		dwRet = 0x01 ;
	
		memset(szDestin1, 0x00, 512*sizeof(TCHAR) ) ;
		wsprintf( szDestin1, L"%sWRDWIZUSERREG.DB",	pDrive ) ;

		if( ( GetDriveType(pDrive) & DRIVE_FIXED ) == DRIVE_FIXED )
		{
			dwRet = GetRegistrationDatafromFile( szDestin1 ) ;
			if( !dwRet )
				return dwRet ;
		}
		pDrive += 4 ;
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
DWORD CISpyGUIApp::GetRegistrationDatafromFile( CString strUserRegFile )
{
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	DWORD	dwRet = 0x00, dwBytesRead=0x00 ;

	DWORD	dwSize = sizeof(m_ActInfo) ;

	hFile = CreateFile(	strUserRegFile, GENERIC_READ, 0, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( hFile == INVALID_HANDLE_VALUE )
	{
		dwRet = 0x01 ;
		goto Cleanup ;
	}

	ZeroMemory(&m_ActInfo, sizeof(m_ActInfo) ) ;
	ReadFile( hFile, &m_ActInfo, dwSize, &dwBytesRead, NULL ) ;
	if( dwSize != dwBytesRead )
	{
		dwRet = 0x02 ;
		goto Cleanup ;
	}

	if( DecryptRegistryData( (LPBYTE )&m_ActInfo, dwSize ) )
	{
		dwRet = 0x04 ;
		goto Cleanup ;
	}
	if( _tcscmp(m_ActInfo.szTitle,  L"Mr.") != 0	&&
	_tcscmp(m_ActInfo.szTitle,  L"Mrs.") != 0	&&
	_tcscmp(m_ActInfo.szTitle,  L"Ms.") != 0	)
	{
		dwRet = 0x5;
		goto Cleanup;
	}

Cleanup:

	if( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile ) ;
	hFile = INVALID_HANDLE_VALUE ;


	return dwRet ;
}
/*	ISSUE NO - 653 NAME - NITIN K. TIME - 12th June 2014 */
/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonUpdate                                                     
*  Description    : It shows help files on the basis of edition.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CISpyGUIApp::OnBnClickedButtonUpdate()
{
	TCHAR szModulePath[MAX_PATH] = {0};
	TCHAR szFullPath[MAX_PATH] = {0};

	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	_tcscpy_s(szFullPath, szModulePath);
	switch(m_dwProductID)
	{
		  //Issue: CHM(Help) Files should be available in german language as well
		  //Resolved By: Nitin K Date: 11th Jan 2016
		case ESSENTIAL :
			  switch (m_dwSelectedLangID)
			  {
				  case ENGLISH:  _tcscat_s(szFullPath, L"\\WrdWizAVEssential.chm");
					  break;
				  case GERMAN:  
					  _tcscat_s(szFullPath, L"\\WrdWizAVEssentialGerman.chm");
					  if(!PathFileExists(szFullPath))
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
		  case PRO : 
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
		  case ELITE:_tcscat_s(szFullPath, L"\\WrdWizAVElite.chm");
			 break;
		  case BASIC :
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
		  default : AddLogEntry(L"Invalid option, help file not available", 0, 0, true, SECONDLEVEL);
			 break;
	}

	::ShellExecute(NULL, L"Open", szFullPath, L"", NULL,SW_SHOWNORMAL);
}

/***************************************************************************************************                    
*  Function Name  : ReadSoundSettingFromRegistry                                                     
*  Description    : Read enable/disable sound entry of scan finished or threat found key.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
bool CISpyGUIApp::ReadSoundSettingFromRegistry()
{
	HKEY key;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Wardwiz Antivirus"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
	{
		//AddLogEntry(L"Unable to open registry key");
		return false;
	}

	DWORD dwPlaySound;
	DWORD dwPlaySoundSize = sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	long ReadReg = RegQueryValueEx(key, L"dwEnableSound", NULL ,&dwType,(LPBYTE)&dwPlaySound, &dwPlaySoundSize);
	if(ReadReg == ERROR_SUCCESS)
	{
		dwPlaySound = (DWORD)dwPlaySound;
		if(dwPlaySound == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : DecryptRegistryData                                                     
*  Description    : Decrypt data of buffer for registration data.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CISpyGUIApp::DecryptRegistryData( LPBYTE lpBuffer, DWORD dwSize )
{
	if( IsBadWritePtr( lpBuffer, dwSize ) )
		return 1 ;

	DWORD	iIndex = 0 ;
	DWORD jIndex = 0;

	if (lpBuffer == NULL || dwSize == 0x00)
	{
		return 1;
	}

	for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
	{
		if(lpBuffer[iIndex] != 0)
		{
			if((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
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
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : GetTotalExistingBytes                                                     
*  Description    : get total bytes of existing virus database file.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
*  Modified Date  : 6/2/2015 Neha Gharge FP file added
****************************************************************************************************/
DWORD CISpyGUIApp::GetTotalExistingBytes()
{

	memset(m_ExistingBytes, 0, sizeof(m_ExistingBytes));

	DWORD dwFirstFileSize = 0;
	DWORD dwSecondFileSize = 0;
	DWORD dwThirdFileSize = 0;
	DWORD dwForthFileSize = 0;
	DWORD dwFifthFileSize = 0;

	CString csFilePath = GetModuleFilePath() + L"\\DB\\DAILY.CLD";
	
	HANDLE hFFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	dwFirstFileSize = 0;
	if(hFFile != INVALID_HANDLE_VALUE)
	{
		dwFirstFileSize = GetFileSize(hFFile,NULL);
		
	}
	CloseHandle(hFFile);

	csFilePath = theApp.GetModuleFilePath() + L"\\DB\\MAIN.CVD";
	//HANDLE hTFile = CreateFile(csFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	HANDLE hSFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	dwSecondFileSize = 0;
	if(hSFile != INVALID_HANDLE_VALUE)
	{
		dwSecondFileSize=GetFileSize(hSFile,NULL);
	}
	CloseHandle(hSFile);

	csFilePath = theApp.GetModuleFilePath() + L"\\WRDWIZDATABASE\\WRDWIZAV1.DB";
	HANDLE hTFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	dwThirdFileSize = 0;
	if(hTFile != INVALID_HANDLE_VALUE)
	{
		dwThirdFileSize=GetFileSize(hTFile,NULL);
	}
	CloseHandle(hTFile);

	csFilePath = theApp.GetModuleFilePath() + L"\\WRDWIZDATABASE\\WRDWIZAVR.DB";
	HANDLE hFourFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	dwForthFileSize = 0;
	if(hFourFile != INVALID_HANDLE_VALUE)
	{
		dwForthFileSize=GetFileSize(hFourFile,NULL);
	}
	CloseHandle(hFourFile);

	csFilePath = theApp.GetModuleFilePath() + L"\\DB\\WRDWIZWHLST.FP";
	HANDLE hFiFthFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	dwFifthFileSize = 0;
	if(hFiFthFile != INVALID_HANDLE_VALUE)
	{
		dwFifthFileSize=GetFileSize(hFiFthFile,NULL);
	}
	CloseHandle(hFiFthFile);

	m_ExistingBytes[0] = dwFirstFileSize;
	m_ExistingBytes[1] = dwSecondFileSize;
	m_ExistingBytes[2] = dwThirdFileSize;
	m_ExistingBytes[3] = dwForthFileSize;
	m_ExistingBytes[4] = dwFifthFileSize;
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : CrashCallback                                                     
*  Description    : Crash dump analysis
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 27 July 2014
****************************************************************************************************/
void CISpyGUIApp::CrashCallback(LPVOID lpvState)
{
    UNREFERENCED_PARAMETER(lpvState);
	AddLogEntry(L"### GenX application crashed");
}

/***********************************************************************************************
  Function Name  : CheckScanLevel
  Description    : To check scan level 1-> Only with wardwiz scanner
									   2-> with clam scanner and second level scaner is wardwiz scanner.
  SR.NO			 : 
  Author Name    : Neha gharge
  Date           : 1-19-2015
***********************************************************************************************/
void CISpyGUIApp::CheckScanLevel()
{
	DWORD dwScanLevel = 0;
	if(m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wardwiz Antivirus", L"dwScanLevel", dwScanLevel) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CNextGenExGUIApp::CheckScanLevel", 0, 0, true, SECONDLEVEL);;
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
		AddLogEntry(L" ### Scan Level option is wrong. Please reinstall setup of WardWiz",0,0,true,SECONDLEVEL);
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
bool CISpyGUIApp::ShowRestartMsgOnProductUpdate()
{
	try
	{
		TCHAR	szAllUserPath[255] = { 0 };
		TCHAR  szActualPath[255] = { 0 };

		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 255);
		swprintf_s(szActualPath, _countof(szActualPath), L"%s\\%s", szAllUserPath, L"WardWiz Antivirus\\AluDel.ini");
		int fileExist = 0;
		fileExist = PathFileExists(szActualPath);
		if (fileExist == 1)
		{
			HWND hwnd = ::GetActiveWindow();
			CString csRestartRequired;
			int IdReturn = 0;
			if ((theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG1") == L"" ) && (theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG2") == L"") && (theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_ONSCAN_CLICK_MSG3") == L""))
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
		AddLogEntry(L"### Exception in CNextGenExGUIApp::ShowRestartMsgOnProductUpdate", 0, 0, true, SECONDLEVEL);
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
void CISpyGUIApp::AddUserAndSystemInfoToLog()
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
		AddLogEntry(L"### Exception in CNextGenExGUIApp::AddUserAndSystemInfoToLog", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : StartDataCryptOpr
Description    : Start the Crypt operation (If 1 instance is already running and we click on Right click-> Encrypt || Decrypt then stop previous operation and launch new Operation)
Author Name    : Nitin K
SR_NO		   :
Date           : 15th June 2015
Modification   :
****************************************************************************/
void CISpyGUIApp::StartDataCryptOpr(int iDataOpr, CString csDataCryptFilePath)
{
	SendDataCryptOpr2Gui(DATA_ENC_DEC_OPERATIONS, iDataOpr, csDataCryptFilePath);
}

/***************************************************************************
Function Name  : SendDataCryptOpr2Gui
Description    : Send the Crypt operation request to existing UI
Author Name    : Nitin K
SR_NO		   :
Date           : 15th June 2015
Modification   :
****************************************************************************/
bool CISpyGUIApp::SendDataCryptOpr2Gui(int iMessageInfo, DWORD dwDataOpr, CString csDataCryptFilePath)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	szPipeData.iMessageInfo = iMessageInfo;
	szPipeData.dwValue = dwDataOpr;
	wcscpy_s(szPipeData.szFirstParam, csDataCryptFilePath);
	CISpyCommunicator objCom(UI_SERVER);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send Data in CGenXGUIApp::SendEmailPluginChange2Service");
		return false;
	}
	return true;
}


/***************************************************************************
Function Name  : CheckMutexOfDriverInstallation
Description    : Check Mutex Of Driver Installation
Author Name    : Neha Gharge
SR_NO		   :
Date           : 30th OCt 2015
Modification   :
****************************************************************************/
bool CISpyGUIApp::CheckMutexOfDriverInstallation()
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
		AddLogEntry(L"### Exception in CNextGenExGUIApp::CheckMutexOfDriverInstallation", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************
Function Name  : CheckAndSetLoadBehaviourOfAddin
Description    : Check load behaviour of Email Addin and if not loaded then set load behaviour 
Author Name    : Neha Gharge
SR_NO		   :
Date           : 09th Feb 2015
Modification   :
//Issue 786 In case wardwiz addin get unloaded. On opening of UI we are loading if its unloaded
****************************************************************************/
bool CISpyGUIApp::CheckAndSetLoadBehaviourOfAddin()
{
	__try
	{
		DWORD dwLoadAddin = 0x00;
		if (m_objReg.GetRegistryDWORDData(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Outlook\\Addins\\OutlookAddin.Addin", L"LoadBehavior", dwLoadAddin) != 0)
		{
			AddLogEntry(L"### Failed to Get registry value in CNextGenExGUIApp::CheckAndSetLoadBehaviourOfAddin", 0, 0, true, ZEROLEVEL);
			return false;
		}

		if (dwLoadAddin != 0x03)
		{
			if (m_objReg.SetRegistryValueData(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Outlook\\Addins\\OutlookAddin.Addin", L"LoadBehavior", 0x03) != 0)
			{
				AddLogEntry(L"### Failed to Get registry value in CNextGenExGUIApp::CheckAndSetLoadBehaviourOfAddin", 0, 0, true, ZEROLEVEL);
				return false;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIApp::CheckAndSetLoadBehaviourOfAddin", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : LoadExtractDll
Description    : Load Extract dll for update through local updates
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 16th Feb 2016
Modification   :
****************************************************************************/
bool CISpyGUIApp::LoadExtractDll()
{
	try
	{
		CString	csWardWizModulePath = GetModuleFilePath();
		CString	csWardWizExtractDLL = L"";
		csWardWizExtractDLL.Format(L"%s\\WRDWIZEXTRACT.DLL", csWardWizModulePath);
		if (!PathFileExists(csWardWizExtractDLL))
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_LOAD_FAILED_EXTRACT_DLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}

		m_hZip = LoadLibrary(csWardWizExtractDLL);
		if (!m_hZip)
		{
			AddLogEntry(L"### CExtractAttchForScan::Failed in loading WRDWIZEXTRACT.DLL", 0, 0, true, SECONDLEVEL);
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_LOAD_FAILED_EXTRACT_DLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_hZip = NULL;
			return false;
		}

		UnzipFile = (UNZIPFILE)GetProcAddress(m_hZip, "UnRarForUpdates");
		if (!UnzipFile)
		{
			AddLogEntry(L"### CExtractAttchForScan::Failed in GetProcAddress:UnzipFile", 0, 0, true, SECONDLEVEL);
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_FUN_UNZIPFILE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_hZip = NULL;
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIApp::LoadExtractDll", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : ShowProdExpiredUnRegisteredMsgBox()
Description    : Show product expired/Unregistered messagebox On start of UI
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 22nd Feb 2016
Modification   :
****************************************************************************/
void CISpyGUIApp::ShowProdExpiredUnRegisteredMsgBox()
{
	try
	{
		if (m_bShowProdExpMsgBox)
		{
			if (theApp.m_dwDaysLeft == 0)
			{
				theApp.GetDaysLeft();
			}
			if (theApp.m_dwDaysLeft == 0)
			{
				HWND hWindow = ::FindWindow(NULL, L"WRDWIZAVUI");
				if (hWindow)
				{
					::PostMessage(hWindow, LAUNCHPRODEXPMSGBOX, 0, 0);

				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIApp::ShowProdExpiredUnRegisteredMsgBox", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : CheckForShowProdExpMsgBox()
Description    : Check for Prod Exp msg box display count in ProductSettings.INI file
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 24th Feb 2016
Modification   :
****************************************************************************/
bool CISpyGUIApp::CheckForShowProdExpMsgBox()
{
	bool bRet = false;
	try
	{
		CString csValue = L"";
		CString csIniFilePath = theApp.GetModuleFilePath() + L"\\WRDSETTINGS" + L"\\ProductSettings.ini";

		if (!PathFileExists(csIniFilePath))
		{
			MessageBox(NULL, L"ProductSettings.ini file not found.\nPlease reinstall WardWiz to fix this problem.", L"WardWiz", MB_ICONEXCLAMATION);
			exit(0);
		}

		DWORD dwShowProdExpMsgBoxValue = GetPrivateProfileInt(L"WRDSETTINGS", L"ShowProdExpMsgBox", 0, csIniFilePath);
		dwShowProdExpMsgBoxValue++;
		if (dwShowProdExpMsgBoxValue == 1)
		{
			csValue = L"";
			csValue.Format(L"%d", dwShowProdExpMsgBoxValue); 
			bRet = true;
		}
		else if (dwShowProdExpMsgBoxValue >= 3)
		{
			csValue = L"";
			csValue.Format(L"%d", 0);
			bRet = false;
		}
		else
		{
			csValue = L"";
			csValue.Format(L"%d", dwShowProdExpMsgBoxValue);
			bRet = false;
		}
		WritePrivateProfileString(L"WRDSETTINGS", L"ShowProdExpMsgBox", csValue, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIApp::CheckForShowProdExpMsgBox", 0, 0, true, SECONDLEVEL);
	}
	return bRet;
}


/***********************************************************************************************
Function Name  : CheckSetupLevel
Description    : To check scan level 0-> WardWiz India Setup (Means setup is Clam, NoClam both)
2-> WardWiz Germany Setup (Means setup is Only NoClam setup)
SR.NO			 :
Author Name    : Nitin Kolapkar
Date           : 7th March 2015
***********************************************************************************************/
void CISpyGUIApp::CheckSetupLocId()
{
	DWORD dwSetupLocId = 0;
	try
	{
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wardwiz Antivirus", L"dwLocID", dwSetupLocId) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CNextGenExGUIApp::CheckSetupLocId", 0, 0, true, SECONDLEVEL);;
			return;
		}

		switch (dwSetupLocId)
		{
		case 0x00:
			m_eSetupLocId = WARDWIZINDIA;
			break;
		case 0x01:
			m_eSetupLocId = WARDWIZGERMANY;
			break;
		default:
			AddLogEntry(L" ### Setup Level option is wrong. Please reinstall setup of WardWiz", 0, 0, true, SECONDLEVEL);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIApp::CheckSetupLocId", 0, 0, true, SECONDLEVEL);
	}
}