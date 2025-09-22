
// WrdWizAutoScn.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WrdWizAutoScn.h"
#include "WrdWizAutoScnDlg.h"

#include "WrdWizSystemInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWrdWizAutoScnApp
//Neha Gharge 23 th May 2013, On the F1 button it shold show wardwiz help file
BEGIN_MESSAGE_MAP(CWrdWizAutoScnApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWrdWizAutoScnApp::OnBnClickedButtonHelp)
END_MESSAGE_MAP()


// CWrdWizAutoScnApp construction

CWrdWizAutoScnApp::CWrdWizAutoScnApp()
{
	// support Restart Manager
	//m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	m_hResDLL = NULL;
	m_dwProdID = 0x00;
	m_dwSelectedLangID = 0;
	m_objCompleteEvent.ResetEvent();
	m_bTmpFilePage = false;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CWrdWizAutoScnApp object

CWrdWizAutoScnApp theApp;

/***************************************************************************************************
*  Function Name  : ~CISpyGUIApp
*  Description    : D'tor
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           :15 May 2015
****************************************************************************************************/
CWrdWizAutoScnApp::~CWrdWizAutoScnApp(void)
{

	if (m_hResDLL != NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
	m_bTmpFilePage = false;
}




// CWrdWizAutoScnApp initialization

BOOL CWrdWizAutoScnApp::InitInstance()
{
	CWinApp::InitInstance();
	OleInitialize(NULL); // for system drag-n-drop
	sciter::sync::gui_thread_ctx _; // instance of gui_thread_ctx
	// it should be created as a variable inside WinMain 
	// gui_thread_ctx is critical for GUI_CODE_START/END to work

	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	
	CWrdWizAutoScnDlg dlg;
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CString csCommandLine = GetCommandLine();
	if (csCommandLine.Find('-') != -1)
	{
		csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
		csCommandLine.Trim();
		if (csCommandLine.GetLength() > 0)
		{
			WaitForSingleObject(dlg.m_hWnd, 1000 * 20); //Waiting for 20 seconds
			if (csCommandLine.CompareNoCase(TEXT("TEMPFILECLEANER")) == 0)
			{
				m_bTmpFilePage = true;
			}
		}
	}

	//if (CheckMutexOfDriverInstallation())
	//{
	//	MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_DRIVER_INSTALLATION_PROCESS_INPROGRESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
	//	return FALSE;
	//}

	m_dwSelectedLangID = m_objwardwizLangManager.GetSelectedLanguage();
	m_dwProdID = m_objwardwizLangManager.GetSelectedProductID();


	//Added here due to wardwiz help added by Neha
	//Modified by Vilas on 26 May 2015
	AddUserAndSystemInfoToLog();

	LoadResourceDLL();

	CreateFonts();

	//Check for single instance 
	//Neha Gharge 7 July,2015
	if (SingleInstanceCheck())
	{
		HWND hWindow = ::FindWindow(NULL, L"Vibranium TEMP CLEANER");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			AddLogEntry(L"Wardwiz Temporary removal is already running.", 0, 0, true, SECONDLEVEL);
			return FALSE;
		}
		return FALSE;
	}

	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);

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
*  Function Name  : LoadResourceDLL
*  Description    : Load resource dll.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 may 2015
****************************************************************************************************/
void CWrdWizAutoScnApp::LoadResourceDLL()
{
	CString	csWardWizModulePath = GetModuleFilePath();

	CString	csWardWizResourceDLL = L"";
	csWardWizResourceDLL.Format(L"%s\\VBRESOURCE.DLL", csWardWizModulePath);
	if (!PathFileExists(csWardWizResourceDLL))
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCE_MODULE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if (!m_hResDLL)
	{
		m_hResDLL = LoadLibrary(csWardWizResourceDLL);
		if (!m_hResDLL)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCE_MODULE_LOAD_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}

}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 11 May,2015
****************************************************************************************************/
CString CWrdWizAutoScnApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}


/**********************************************************************************************************
*  Function Name  :	CreateFonts
*  Description    :	Creates fonts for the project depending upon the Language selected
*  SR.NO          :
*  Author Name    : Neha Gharge
*  Date           : 11 May 2015
**********************************************************************************************************/

void CWrdWizAutoScnApp::CreateFonts()
{
	DWORD OSType = m_objOSVersionWrap.DetectClientOSVersion();
	m_dwOSType = OSType;
	DWORD LanguageType = m_dwSelectedLangID;
	CreateFontsFor(OSType, LanguageType);
}

/***************************************************************************************************
*  Function Name  : CreateFontsFor
*  Description    : Creates fonts according to OS type and launguage type.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 11 May 2015
****************************************************************************************************/
void CWrdWizAutoScnApp::CreateFontsFor(DWORD OSType, DWORD LanguageType)
{
	switch (OSType)
	{
	case WINOS_95:
		break;
	case WINOS_98:
		break;
	case WINOS_2000:
		break;
	case WINOS_NT:
		switch (LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(250, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(220, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(120, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(140, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
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
		}
		break;
	case WINOS_VISTA:
		switch (LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(250, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(220, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(120, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(140, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(150, _T("Verdana"), 0);
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
		}
		break;
	case WINOS_XP:
	case WINOS_XP64:   //Name:Varada Ikhar, Date:13/01/2015, Version: 1.8.3.12, Issue No:1, Description:In Windows XP for Basic ,after clicking ok on tooltip gives dialog "new os or old os".
		switch (LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(250, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(220, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(120, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(140, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(150, _T("Verdana"), 0);
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
		}
		break;
	case WINOS_WIN7:
		switch (LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(190, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(180, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(110, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(120, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(140, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(160, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(130, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(75, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(90, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			m_fontWWTextMediumSize.CreatePointFont(110, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(130, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(80, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWDaysLeftFont.CreatePointFont(380, _T("Cambria"), 0);
			break;
		}
		break;
	case WINOS_WIN8:
		switch (LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(175, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(165, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(145, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(105, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(115, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(140, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(140, _T("Verdana"), 0);
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
		}
		break;
	case WINOS_WIN8_1:
		switch (LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(175, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(165, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(145, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(105, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(115, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(140, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(140, _T("Verdana"), 0);
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
		}
		break;
	case WINOSUNKNOWN_OR_NEWEST:
		switch (LanguageType)
		{
		case HINDI:
			m_fontWWTextTitle.CreatePointFont(200, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(190, _T("Verdana"), 0);
			m_fontWWTextSubTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSubTitleDescription.CreatePointFont(105, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextNormal.CreatePointFont(115, _T("Microsoft Sans serif Regular"), 0);
			m_fontWWTextMediumSize.CreatePointFont(140, _T("Verdana"), 0);
			m_FontWWStartUpFontTitle.CreatePointFont(250, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpFontSubTitle.CreatePointFont(150, _T("Microsoft Sans serif Regular"), 0);
			m_FontWWStartUpTips.CreatePointFont(100, _T("Microsoft Sans serif Regular"), 0);
			//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
			m_fontWWLogoHeader.CreatePointFont(150, _T("Eras Demi ITC"));
			break;

		case ENGLISH:
		case SPANISH:
		case GERMAN:
		case FRENCH:
		case CHINESE:
			m_fontWWTextTitle.CreatePointFont(150, _T("Verdana"), 0);
			m_fontWWTextSmallTitle.CreatePointFont(140, _T("Verdana"), 0);
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
		}
		break;
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonHelp
*  Description    : It shows help files on the basis of edition.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 05 May 2015
****************************************************************************************************/
void CWrdWizAutoScnApp::OnBnClickedButtonHelp()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFullPath[MAX_PATH] = { 0 };

		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		_tcscpy_s(szFullPath, _countof(szFullPath), szModulePath);
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
					_tcscpy_s(szFullPath, L"\0");
					_tcscpy_s(szFullPath, szModulePath);
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
					_tcscpy_s(szFullPath, L"\0");
					_tcscpy_s(szFullPath, szModulePath);
					_tcscat_s(szFullPath, L"\\WrdWizAVPro.chm");
				}
				break;
			default: _tcscat_s(szFullPath, L"\\WrdWizAVPro.chm");
				break;
			}
			break;

		case ELITE:_tcscat_s(szFullPath, L"\\WrdWizAVElite.chm");
			break;

		case BASIC:
			switch (m_dwSelectedLangID)
			{
			case ENGLISH:  _tcscat_s(szFullPath, L"\\WrdWizAVBasic.chm");
				break;
			case GERMAN:  _tcscat_s(szFullPath, L"\\WrdWizAVBasicGerman.chm");
				if (!PathFileExists(szFullPath))
				{
					_tcscpy_s(szFullPath, L"\0");
					_tcscpy_s(szFullPath, szModulePath);
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
Function Name  : AddUserAndSystemInfoToLog
Description    : Adds Computer name, logged user name and OS details to log at the top
Author Name    : Vilas Suvarnakar
SR_NO		   :
Date           : 11 May 2015
Modification   :
****************************************************************************/
void CWrdWizAutoScnApp::AddUserAndSystemInfoToLogSEH()
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


void CWrdWizAutoScnApp::AddUserAndSystemInfoToLog()
{
	try
	{
		AddUserAndSystemInfoToLogSEH();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAutoScnApp::AddUserAndSystemInfoToLog", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SingleInstanceCheck
*  Description    : Check whether wardwiz Temporary Cleaner instance is exist or not
*  Author Name    : Vilas Suvarnakar
*  SR_NO
*  Date           : 12 May 2015
****************************************************************************************************/
bool CWrdWizAutoScnApp::SingleInstanceCheck()
{

	m_hMutex = CreateMutex(NULL, TRUE, L"Global\\VibraniumTEMPCLN");
	DWORD dwError = GetLastError();

	if (dwError == ERROR_ALREADY_EXISTS)
	{
		AddLogEntry(L"Second", 0, 0, false, SECONDLEVEL);
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
bool CWrdWizAutoScnApp::CheckMutexOfDriverInstallation()
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