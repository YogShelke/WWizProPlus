
// WrdWizDownloaderDlg.cpp : implementation file
/***********************************************************************************************
*  Program Name: WrdWizDownloaderDlg.cpp
*  Description: It is a downloader which is used to download the setup files from server.
*  Author Name: Neha Gharge
*  Date Of Creation: 25-Aug-2015
*  Version No: 1.10.0.0
************************************************************************************************/
#include "stdafx.h"
#include "WrdWizDownloader.h"
#include "WrdWizDownloaderDlg.h"
//#include "afxdialogex.h"
#include "WrdWizCustomMsgBox.h"
#include "EnumProcess.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_RETRY_COUNT 15

// CAboutDlg dialog used for App About
/***************************************************************************************************
*  Function Name  : CAboutDlg
*  Description    :
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

/***************************************************************************************************
*  Function Name  : CAboutDlg
*  Description    :
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    :
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : MESSAGE_MAP for about box
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

HWINDOW   CWrdWizDownloaderDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWrdWizDownloaderDlg::get_resource_instance() { return theApp.m_hInstance; }
// CWardWizUIDlg message handlers

// CWrdWizDownloaderDlg dialog

#define BASICNCG64		 L"http://www.wardwiz.de/WardWizAVSetups/WardWizBasicSetupNCGx64.exe"
#define BASICNCI64		 L"http://www.vibranium.co.in/vibraniumAVSetups/beta/WardWizBasicSetupNCIx64.exe"
#define BASICNCG86		 L"http://www.wardwiz.de/WardWizAVSetups/WardWizBasicSetupNCGx86.exe"
#define BASICNCI86		 L"http://www.vibranium.co.in/vibraniumAVSetups/beta/WardWizBasicSetupNCIx86.exe"

#define ESSENTIALNCG64	 L"http://www.wardwiz.de/WardWizAVSetups/beta/WardWizEssentialSetupNCGx64.exe"
#define ESSENTIALNCI64	 L"http://www.vibranium.co.in/vibraniumAVSetups/beta/WardWizEssentialSetupNCIx64.exe"
#define ESSENTIALNCG86	 L"http://www.wardwiz.de/WardWizAVSetups/beta/WardWizEssentialSetupNCGx86.exe"
#define ESSENTIALNCI86	 L"http://www.vibranium.co.in/vibraniumAVSetups/beta/WardWizEssentialSetupNCIx86.exe"

#define PRONCG64		 L"http://www.wardwiz.de/WardWizAVSetups/WardWizProSetupNCGx64.exe"
#define PRONCI64		 L"http://www.vibranium.co.in/vibraniumAVSetups/WardWizProSetupNCIx64.exe"
#define PRONCG86		 L"http://www.wardwiz.de/WardWizAVSetups/WardWizProSetupNCGx86.exe"
#define PRONCI86		 L"http://www.vibranium.co.in/vibraniumAVSetups/WardWizProSetupNCIx86.exe"

#define ESSPLUSNCG64	 L"http://www.wardwiz.de/WardWizAVSetups/beta/WardWizEssPlusSetupNCGx64.exe"
#define ESSPLUSNCI64	 L"http://www.vibranium.co.in/vibraniumAVSetups/beta/WardWizEssPlusSetupNCIx64.exe"
#define ESSPLUSNCG86	 L"http://www.wardwiz.de/WardWizAVSetups/beta/WardWizEssPlusSetupNCGx86.exe"
#define ESSPLUSNCI86	 L"http://www.vibranium.co.in/vibraniumAVSetups/beta/WardWizEssPlusSetupNCIx86.exe"

#define BASICNCG64PATCH	 L"http://www.wardwiz.de/WardWizAVSetups/WardWizBasicSetupNCGx64.exe"
#define BASICNCI64PATCH	 L"http://www.wardwiz.in/Offline%20Patches/WardWizBasicPatchIx64.exe"
#define BASICNCG86PATCH	 L"http://www.wardwiz.de/WardWizAVSetups/WardWizBasicSetupNCGx86.exe"
#define BASICNCI86PATCH	 L"http://www.wardwiz.in/Offline%20Patches/WardWizBasicPatchIx86.exe"

#define ESSNCG64PATCH	 L"http://wardwiz.de/Offline%20Patches/WardWizEssentialPatchGx64.exe"
#define ESSNCI64PATCH	 L"http://www.wardwiz.in/Offline%20Patches/WardWizEssentialPatchIx64.exe"
#define ESSNCG86PATCH	 L"http://wardwiz.de/Offline%20Patches/WardWizEssentialPatchGx86.exe"
#define ESSNCI86PATCH	 L"http://www.wardwiz.in/Offline%20Patches/WardWizEssentialPatchIx86.exe"

#define ESSPLNCG64PATCH	 L"http://www.wardwiz.de/WardWizAVSetups/beta/WardWizEssPlusSetupNCGx64.exe"
#define ESSPLNCI64PATCH	 L"http://www.wardwiz.in/Offline%20Patches/WardWizEssPlusPatchIx64.exe"
#define ESSPLNCG86PATCH	 L"http://www.wardwiz.de/WardWizAVSetups/beta/WardWizEssPlusSetupNCGx86.exe"
#define ESSPLNCI86PATCH	 L"http://www.wardwiz.in/Offline%20Patches/WardWizEssPlusPatchIx86.exe"

#define BASICNCG64SETUPNAME		 L"WardWizBasicSetupNCGx64.exe"
#define BASICNCI64SETUPNAME		 L"WardWizBasicSetupNCIx64.exe"
#define BASICNCG86SETUPNAME		 L"WardWizBasicSetupNCGx86.exe"
#define BASICNCI86SETUPNAME		 L"WardWizBasicSetupNCIx86.exe"

#define ESSENTIALNCG64SETUPNAME	 L"WardWizEssentialSetupNCGx64.exe"
#define ESSENTIALNCI64SETUPNAME	 L"WardWizEssentialSetupNCIx64.exe"
#define ESSENTIALNCG86SETUPNAME	 L"WardWizEssentialSetupNCGx86.exe"
#define ESSENTIALNCI86SETUPNAME	 L"WardWizEssentialSetupNCIx86.exe"

#define PRONCG64SETUPNAME		 L"WardWizProSetupNCGx64.exe"
#define PRONCI64SETUPNAME		 L"WardWizProSetupNCIx64.exe"
#define PRONCG86SETUPNAME		 L"WardWizProSetupNCGx86.exe"
#define PRONCI86SETUPNAME		 L"WardWizProSetupNCIx86.exe"

#define ESSPLUSNCG64SETUPNAME	 L"WardWizEssPlusSetupNCGx64.exe"
#define ESSPLUSNCI64SETUPNAME	 L"WardWizEssPlusSetupNCIx64.exe"
#define ESSPLUSNCG86SETUPNAME	 L"WardWizEssPlusSetupNCGx86.exe"
#define ESSPLUSNCI86SETUPNAME	 L"WardWizEssPlusSetupNCIx86.exe"

#define BASICNCG64PATCHNAME		 L"WardWizBasicSetupNCGx64.exe"
#define BASICNCI64PATCHNAME		 L"WardWizBasicPatchIx64.exe"
#define BASICNCG86PATCHNAME		 L"WardWizBasicSetupNCGx86.exe"
#define BASICNCI86PATCHNAME		 L"WardWizBasicPatchIx86.exe"

#define ESSENTIALNCG64PATCHNAME	 L"WardWizEssentialPatchGx64.exe"
#define ESSENTIALNCI64PATCHNAME	 L"WardWizEssentialPatchIx64.exe"
#define ESSENTIALNCG86PATCHNAME	 L"WardWizEssentialPatchGx86.exe"
#define ESSENTIALNCI86PATCHNAME	 L"WardWizEssentialPatchIx86.exe"

#define ESSPLUSNCG64PATCHNAME	 L"WardWizEssPlusSetupNCGx64.exe"
#define ESSPLUSNCI64PATCHNAME	 L"WardWizEssPlusPatchIx64.exe"
#define ESSPLUSNCG86PATCHNAME	 L"WardWizEssPlusSetupNCGx86.exe"
#define ESSPLUSNCI86PATCHNAME	 L"WardWizEssPlusPatchIx86.exe"

DWORD WINAPI StartWardWizSetupDownloadProcessThread(LPVOID lpParam);

/***************************************************************************************************
*  Function Name  : CWrdWizDownloaderDlg
*  Description    : C'tor
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
CWrdWizDownloaderDlg::CWrdWizDownloaderDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWrdWizDownloaderDlg::IDD, pParent)
	, m_hStartWardWizSetupDwnldProc(NULL)
	, m_bIsWow64(false)
	, m_bIsDownloadingInProgress(false)
	, m_bIsProcessPaused(false)
	, m_bIsCloseCalled(false)
	//, m_bMinimize(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_RUN, m_btnRun);
	DDX_Control(pDX, IDC_BUTTON_DOWNLOAD_BROWSE, m_btndwnldpathBrowse);
	DDX_Control(pDX, IDC_EDIT_BROWSE_PATH, m_edtBrowsedSetupPath);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_BUTTON_RESUME, m_btnResume);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_PROGRESS_DOWNLOAD_PROGRESS, m_prgDownloadSetupStatus);
	DDX_Control(pDX, IDC_CHECK_LAUNCH, m_chkLaunchDownloadedExe);
	DDX_Control(pDX, IDC_CHECK_OPENFOLDER, m_chkOpenFolderOption);
	//DDX_Control(pDX, IDC_RADIO_32BIT_SETUP, m_radio32bitSetup);
	//DDX_Control(pDX, IDC_RADIO_64BIT_SETUP, m_radio64bitSetup);
	DDX_Control(pDX, IDC_STATIC_TEXT_TO_SHOW_PATH, m_stSelectDwnldText);
	DDX_Control(pDX, IDC_STATIC_FIRST_BOX, m_grpFirstBox);
	DDX_Control(pDX, IDC_STATIC_DWNLD_INFO, m_stDwnldInformation);
	DDX_Control(pDX, IDC_STATIC_SECOND_GROUPBOX, m_grpSecondBox);
	DDX_Control(pDX, IDC_STATIC_PROD_NAME, m_stProductName);
	DDX_Control(pDX, IDC_STATIC_ACTUAL_PROD_NAME, m_stActualProdName);
	DDX_Control(pDX, IDC_STATIC_DWNLD_PATH, m_stDwnldPath);
	DDX_Control(pDX, IDC_STATIC_ACTUAL_DOWNLD_PATH, m_stActualDownloadPath);
	DDX_Control(pDX, IDC_STATIC_TRANSFER_RATE, m_stTransferRate);
	DDX_Control(pDX, IDC_STATIC_ACTUAL_TRANSFER_RATE, m_stActualTransferRate);
	DDX_Control(pDX, IDC_STATIC_REMAINING_TIME, m_stRemainingTime);
	DDX_Control(pDX, IDC_STATIC_ACTUAL_REMAINING_TIME, m_stActualRemainingTime);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_stStatus);
	DDX_Control(pDX, IDC_STATIC_ACTUAL_STATUS, m_stActualStatus);
	DDX_Control(pDX, IDC_STATIC_THIRD_GRPBOX, m_grpThirdBox);
	DDX_Control(pDX, IDC_CHECK_DWNLD_OPTION, m_chkDownloadOption);
	DDX_Control(pDX, IDC_STATIC_FOOTER_MSG, m_stFooterText);
	DDX_Control(pDX, IDC_STATIC_BROSWER, m_stBrowser);
	DDX_Control(pDX, IDC_STATIC_DOWNLOADER_LOGO, m_stDownloaderLogo);
	DDX_Control(pDX, IDC_STATIC_DOWNLOADER_NAME, m_stdownloadHeaderName);
	DDX_Control(pDX, IDC_BUTTON_WINDOW_CLOSE, m_btnSystemClose);
	DDX_Control(pDX, IDC_BUTTON_WINDOW_MINIMIZE, m_btnSystemMinimize);
	DDX_Control(pDX, IDC_STATIC_LAUNCH_TEXT, m_stLaunchText);
	DDX_Control(pDX, IDC_STATIC_OPENFOLDER_TEXT, m_stOpenFolderText);
	DDX_Control(pDX, IDC_STATIC_OFFINE_AD, m_stOfflineAd);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWrdWizDownloaderDlg, CJpegDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CWrdWizDownloaderDlg::OnBnClickedButtonRun)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_BROWSE, &CWrdWizDownloaderDlg::OnBnClickedButtonDownloadBrowse)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CWrdWizDownloaderDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_RESUME, &CWrdWizDownloaderDlg::OnBnClickedButtonResume)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CWrdWizDownloaderDlg::OnBnClickedButtonClose)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_LAUNCH, &CWrdWizDownloaderDlg::OnBnClickedCheckLaunch)
	ON_BN_CLICKED(IDC_CHECK_OPENFOLDER, &CWrdWizDownloaderDlg::OnBnClickedCheckOpenfolder)
	//ON_BN_CLICKED(IDC_RADIO_32BIT_SETUP, &CWrdWizDownloaderDlg::OnBnClickedRadio32bitSetup)
	//ON_BN_CLICKED(IDC_RADIO_64BIT_SETUP, &CWrdWizDownloaderDlg::OnBnClickedRadio64bitSetup)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_DWNLD_OPTION, &CWrdWizDownloaderDlg::OnBnClickedCheckDwnldOption)
	//ON_NOTIFY(SimpleBrowser::BeforeNavigate2,IDC_STATIC_BROSWER, OnBeforeNavigate2)

	//ON_STN_CLICKED(IDC_STATIC_BROSWER, &CWrdWizDownloaderDlg::OnStnClickedStaticBroswer)
	ON_BN_CLICKED(IDC_BUTTON_WINDOW_CLOSE, &CWrdWizDownloaderDlg::OnBnClickedButtonWindowClose)
	ON_BN_CLICKED(IDC_BUTTON_WINDOW_MINIMIZE, &CWrdWizDownloaderDlg::OnBnClickedButtonWindowMinimize)
END_MESSAGE_MAP()



// CWrdWizDownloaderDlg message handlers
/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft
Foundation Class Library dialog boxes
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
BOOL CWrdWizDownloaderDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	//ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	//ASSERT(IDM_ABOUTBOX < 0xF000);

	//CMenu* pSysMenu = GetSystemMenu(FALSE);
	//if (pSysMenu != NULL)
	//{
	//	BOOL bNameValid;
	//	CString strAboutMenu;
	//	bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
	//	ASSERT(bNameValid);
	//	if (!strAboutMenu.IsEmpty())
	//	{
	//		pSysMenu->AppendMenu(MF_SEPARATOR);
	//		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	//	}
	//}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);

	//Issue no 1323 : minimize on task bar icon
	ModifyStyle(0, WS_SYSMENU | WS_MINIMIZEBOX);

	this->SetWindowTextW(L"WardWiz Download Manager");

#ifdef ENGLISH
	m_bmpOfflineAdPic = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_BITMAP_OFFLINE_AD));
#elif GERMAN
	m_bmpOfflineAdPic = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_BITMAP_OFFLINE_AD_GERMAN));
#endif
	m_bchkLaunchExe = true;

#ifdef WRDWIZBASIC
	//m_stActualProdName.SetWindowTextW(L"  :  WardWiz Basic");
	m_csActualProdName=L"WardWiz Basic";
#elif WRDWIZBASICPATCH
	m_csActualProdName = L"WardWiz Basic Patch";
#elif WRDWIZESSNL
	//m_stActualProdName.SetWindowTextW(L"  :  WardWiz Essential");
	m_csActualProdName=L"WardWiz Essential";
#elif WRDWIZESSNLPATCH
	m_csActualProdName = L"WardWiz Essential Patch";
#elif WRDWIZESSPLUS
	m_csActualProdName=L"WardWiz Essential Plus";
#elif WRDWIZESSPLUSPATCH
	m_csActualProdName =L"WardWiz Essential Plus Patch";
#else
	//m_stActualProdName.SetWindowTextW(L"  :  WardWiz Pro");
	m_csActualProdName=L"WardWiz Pro";
#endif



	CString csStr;
	csStr.Format(L"%d %s (%d KB %s %d KB)", 0, L"%", static_cast<DWORD>(0), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(0));

	TCHAR szLastSetupValue[16] = { 0 };
	TCHAR szLastPathValue[512] = { 0 };
	GetEnvironmentVariable(L"TEMP", m_szTempFolderPath, 511);

#ifdef WRDWIZBASIC
#ifdef RELEASENCG
	m_csLangType = L"GERMAN";
	_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGB.INI");
#elif RELEASENCI
	m_csLangType = L"ENGLISH";
	_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIB.INI");
#endif

#elif WRDWIZBASICPATCH
	#ifdef RELEASENCG
		m_csLangType = L"GERMAN";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGBPatch.INI");
	#elif RELEASENCI
		m_csLangType = L"ENGLISH";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIBPatch.INI");
	#endif

#elif WRDWIZESSNL
#ifdef RELEASENCG
	m_csLangType = L"GERMAN";
	_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEss.INI");
#elif RELEASENCI
	m_csLangType = L"ENGLISH";
	_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEss.INI");
#endif

#elif WRDWIZESSNLPATCH
	#ifdef RELEASENCG
		m_csLangType = L"GERMAN";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPatch.INI");
	#elif RELEASENCI
		m_csLangType = L"ENGLISH";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPatch.INI");
	#endif

#elif WRDWIZESSPLUS
#ifdef RELEASENCG
	m_csLangType = L"GERMAN";
	_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlus.INI");
#elif RELEASENCI
	m_csLangType = L"ENGLISH";
	_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlus.INI");
#endif
#elif WRDWIZESSPLUSPATCH
	#ifdef RELEASENCG
		m_csLangType = L"GERMAN";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlusPatch.INI");
	#elif RELEASENCI
		m_csLangType = L"ENGLISH";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlusPatch.INI");
	#endif
#else
#ifdef RELEASENCG
	_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGP.INI");
#elif RELEASENCI
	_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIP.INI");
#endif
#endif

	GetPrivateProfileString(L"Last Setup", L"1", L"", szLastSetupValue, 16, m_szIniFilePath);
	GetPrivateProfileString(L"Last Path", L"1", L"", szLastPathValue, 512, m_szIniFilePath);


	IsWow64();

	if (m_bIsWow64)
	{
		m_chkDownloadOption.SetWindowTextW(theApp.GetString(IDS_STRING_32_DOWNLOAD_SETUP));
	}
	else
	{
		m_chkDownloadOption.SetWindowTextW(theApp.GetString(IDS_STRING_64_DOWNLOAD_SETUP));
	}

	if (_tcscmp(szLastSetupValue, L"32") == 0)
	{
		if (m_bIsWow64)
		{
			m_chkDownloadOption.SetCheck(BST_CHECKED);
			m_bchkDownloadoption = true;
		}
		else
		{
			m_chkDownloadOption.SetCheck(BST_UNCHECKED);
			m_bchkDownloadoption = false;
		}

	}
	else if (_tcscmp(szLastSetupValue, L"64") == 0)
	{
		if (m_bIsWow64)
		{
			m_chkDownloadOption.SetCheck(BST_UNCHECKED);
			m_bchkDownloadoption = false;
		}
		else
		{
			m_chkDownloadOption.SetCheck(BST_CHECKED);
			m_bchkDownloadoption = true;
		}
	}
	//m_chkDownloadOption.ShowWindow(SW_HIDE);
	//int iSlashPos = 0;
	CString csActualDownloadPath(L"");

	if (_tcscmp(szLastPathValue,L"") != 0)
	{
		m_edtBrowsedSetupPath.SetWindowTextW(szLastPathValue);
		csActualDownloadPath.Format(L"  :  %s", szLastPathValue);
		//iSlashPos = csActualDownloadPath.ReverseFind(L'\\');
		//if (iSlashPos == 2)
		//{
		//	csActualDownloadPath.Truncate(iSlashPos);
		//}
		AddLogEntry(L">>> Downloaded Path from INI  : %s", szLastPathValue, 0, true, SECONDLEVEL);
		m_stActualDownloadPath.SetWindowTextW(csActualDownloadPath);
		m_stActualDownloadPath.RedrawWindow();
	}
	else
	{
		TCHAR szDesktop[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPath(NULL,
			CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE,
			NULL,
			SHGFP_TYPE_CURRENT,
			szDesktop)))
		{
			m_edtBrowsedSetupPath.SetWindowTextW(szDesktop);
			csActualDownloadPath.Format(L"  :  %s", szDesktop);
			m_stActualDownloadPath.SetWindowTextW(csActualDownloadPath);
			m_stActualDownloadPath.RedrawWindow();
		}
	}

	if (!GetModulePath(m_szModulePath, MAX_PATH))
	{
		MessageBox(theApp.GetString(IDS_STRING_ERR_GETMODULENAME), L"Vibranium", MB_ICONERROR);
		return FALSE;
	}


	//m_dwcount = 0;
	// 1319 Internet coneection failure msg on restart machine
	if (WaitForInternetConnection())
	{
		//m_simplebrowser.CreateFromControl(this, IDC_STATIC_BROSWER);
		//m_simplebrowser.PutSilent(true);
#ifdef ENGLISH
#ifdef WRDWIZBASIC
		OnURL_Navigate(L"http://www.wardwiz.com/advertise/basicin.html");
#elif WRDWIZESSNL
		OnURL_Navigate(L"http://www.wardwiz.com/advertise/essentialin.html");
#else
		OnURL_Navigate(L"http://www.wardwiz.com/advertise/proin.html");
#endif
#elif GERMAN
#ifdef WRDWIZBASIC
		OnURL_Navigate(L"http://www.wardwiz.com/advertise/basicde.html");
#elif WRDWIZESSNL
		OnURL_Navigate(L"http://www.wardwiz.com/advertise/essentialde.html");
#else
		OnURL_Navigate(L"http://www.wardwiz.com/advertise/prode.html");
#endif
#endif
	}
	else
	{
		//m_stBrowser.ShowWindow(SW_HIDE);
		//m_stOfflineAd.ShowWindow(SW_SHOW);
	}
	HideAllElements();

	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)AfxGetResourceHandle(), L"res:IDR_HTM_DOWNLOAD_MANAGER.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_DOWNLOAD_MANAGER.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************************************
*  Function Name  : On_LoadDownloader
*  Description    : Function to Download files
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2016
****************************************************************************************************/
json::value CWrdWizDownloaderDlg::On_LoadDownloader(SCITER_VALUE svFunGetProdLang, SCITER_VALUE svFunUpdateDownloadStatusCB, SCITER_VALUE svFunGetDownloadPathCB, SCITER_VALUE svFunShowNotificationMsgCB, SCITER_VALUE svFunCallOnCloseUICB)
{
	try
	{
		m_svFunGetProdLang = svFunGetProdLang;
		m_svFunUpdateDownloadStatusCB = svFunUpdateDownloadStatusCB;
		m_svFunGetDownloadPathCB = svFunGetDownloadPathCB;
		m_svFunShowNotificationMsgCB = svFunShowNotificationMsgCB;
		m_svFunCallOnCloseUICB = svFunCallOnCloseUICB;


		//TCHAR szLastSetupValue[16] = { 0 };
		m_svFunGetProdLang.call((SCITER_STRING)m_csLangType);

		CString csBrowsePath(L"");
		m_edtBrowsedSetupPath.GetWindowTextW(csBrowsePath);

		TCHAR szLastPathValue[512] = { 0 };
		if (GetStartupEntry())
		{
			if (szLastPathValue[0])
			{
				//ShowControlOnClickOfStart(true);
				OnBnClickedButtonRun();
			}
			else
			{
				//ShowControlOnClickOfStart(false);
				OnBnClickedButtonRun();
			}

		}
		else
		{
			//ShowControlOnClickOfStart(false);
			OnBnClickedButtonRun();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::On_LoadDownloader", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : HideAllElements
*  Description    : Function to Hide Old Controllers
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2016
****************************************************************************************************/
void CWrdWizDownloaderDlg::HideAllElements()
{

	try
	{

		m_btnRun.ShowWindow(SW_HIDE);
		m_btndwnldpathBrowse.ShowWindow(SW_HIDE);
		m_edtBrowsedSetupPath.ShowWindow(SW_HIDE);
		m_btnPause.ShowWindow(SW_HIDE);
		m_btnResume.ShowWindow(SW_HIDE);
		m_btnClose.ShowWindow(SW_HIDE);
		m_prgDownloadSetupStatus.ShowWindow(SW_HIDE);
		m_chkLaunchDownloadedExe.ShowWindow(SW_HIDE);
		m_chkOpenFolderOption.ShowWindow(SW_HIDE);
		m_stSelectDwnldText.ShowWindow(SW_HIDE);
		m_grpFirstBox.ShowWindow(SW_HIDE);
		m_stDwnldInformation.ShowWindow(SW_HIDE);
		m_grpSecondBox.ShowWindow(SW_HIDE);
		m_stProductName.ShowWindow(SW_HIDE);
		m_stActualProdName.ShowWindow(SW_HIDE);
		m_stDwnldPath.ShowWindow(SW_HIDE);
		m_stActualDownloadPath.ShowWindow(SW_HIDE);
		m_stTransferRate.ShowWindow(SW_HIDE);
		m_stActualTransferRate.ShowWindow(SW_HIDE);
		m_stRemainingTime.ShowWindow(SW_HIDE);
		m_stActualRemainingTime.ShowWindow(SW_HIDE);
		m_stStatus.ShowWindow(SW_HIDE);
		m_stActualStatus.ShowWindow(SW_HIDE);
		m_grpThirdBox.ShowWindow(SW_HIDE);
		m_chkDownloadOption.ShowWindow(SW_HIDE);
		m_stFooterText.ShowWindow(SW_HIDE);
		m_stBrowser.ShowWindow(SW_HIDE);
		m_stDownloaderLogo.ShowWindow(SW_HIDE);
		m_stdownloadHeaderName.ShowWindow(SW_HIDE);
		m_btnSystemClose.ShowWindow(SW_HIDE);
		m_btnSystemMinimize.ShowWindow(SW_HIDE);
		m_stLaunchText.ShowWindow(SW_HIDE);
		m_stOpenFolderText.ShowWindow(SW_HIDE);
		m_stOfflineAd.ShowWindow(SW_HIDE);
		//m_simplebrowser.ShowWindow(SW_HIDE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::HideAllElements", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnSysCommand
*  Description    : The framework calls this member function when the user selects
a command from the Control menu, or when the user selects the
Maximize or the Minimize button.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	try
	{
		if (nID == SC_MINIMIZE)
		{
			//Issue no 1323 : minimize on task bar icon
			OnBnClickedButtonWindowMinimize();
			//return;
		}

		else if ((nID & 0xFFF0) == IDM_ABOUTBOX)
		{
			CAboutDlg dlgAbout;
			dlgAbout.DoModal();
		}

		else
		{
			CJpegDialog::OnSysCommand(nID, lParam);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnSysCommand", 0, 0, true, SECONDLEVEL);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    :	The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnPaint()
{
	try
	{
		if (IsIconic())
		{
			CPaintDC dc(this); // device context for painting

			SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

			// Center icon in client rectangle
			int cxIcon = GetSystemMetrics(SM_CXICON);
			int cyIcon = GetSystemMetrics(SM_CYICON);
			CRect rect;
			GetClientRect(&rect);
			int x = (rect.Width() - cxIcon + 1) / 2;
			int y = (rect.Height() - cyIcon + 1) / 2;

			// Draw the icon
			dc.DrawIcon(x, y, m_hIcon);
		}
		else
		{
			m_prgDownloadSetupStatus.RedrawWindow();

			CPaintDC dc(this); // device context for painting
			CRect rc;
			CWnd * pW = this->GetDlgItem(IDC_STATIC_FIRST_BOX);
			pW->GetClientRect(&rc);
			pW->ClientToScreen(&rc);
			this->ScreenToClient(&rc);
			dc.FillSolidRect(&rc, RGB(255, 255, 255));
			pW = this->GetDlgItem(IDC_STATIC_SECOND_GROUPBOX);
			pW->GetClientRect(&rc);
			pW->ClientToScreen(&rc);
			this->ScreenToClient(&rc);
			dc.FillSolidRect(&rc, RGB(255, 255, 255));
			CJpegDialog::OnPaint();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnPaint", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnQueryDragIcon
*  Description    : The framework calls this member function by a minimized
(iconic) window that does not have an icon defined for its class
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
HCURSOR CWrdWizDownloaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/***************************************************************************************************
*  Function Name  : OnBnClickedButtonRun
*  Description    : Start for downloading the setups
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedButtonRun()
{
	// site name http: //wardwiz.in/Offline%20Patches/WardWizEssentialPatchx64.exe
	try
	{

		//m_bIsWow64 = false;
		//BeginWaitCursor();

		DWORD dwThreadId = 0x00;
		m_csFileTargetPath = L"";
		CString cRemaining;
		cRemaining.Format(_T("  :  %02ld Mins  %02ld Secs"), 0, 0);
		//m_stActualRemainingTime.SetWindowTextW(cRemaining);
		//m_stActualTransferRate.SetWindowTextW(L"  :  0.00 kbps");
		CString csStr;
		csStr.Format(L"%d %s (%d KB %s %d KB)", 0, L"%", static_cast<DWORD>(0), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(0));
		//m_prgDownloadSetupStatus.SetWindowTextW(csStr);
		//m_prgDownloadSetupStatus.SetPos(0);
		//m_prgDownloadSetupStatus.RedrawWindow();

		OnBnClickedCheckLaunch();

		m_hStartWardWizSetupDwnldProc = CreateThread(NULL, 0, StartWardWizSetupDownloadProcessThread, (LPVOID) this, 0, &dwThreadId);
		Sleep(500);

		if (m_hStartWardWizSetupDwnldProc == NULL)
			AddLogEntry(L"### Failed in CWrdWizDownloaderDlg::To create StartWardWizSetupDownloadProcessThread", 0, 0, true, SECONDLEVEL);

		SetTimer(TIMER_SETPERCENTAGE, 1000, NULL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnBnClickedButtonRun", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : CheckInternetConnection()
*  Description    : It checks internet connection .
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date			  :	20th Aug,2015
****************************************************************************************************/
bool CWrdWizDownloaderDlg::CheckInternetConnection()
{
	bool bReturn = false;
	try
	{

		TCHAR szTestUrl[MAX_PATH] = { 0 };
		_tcscpy_s(szTestUrl, MAX_PATH, _T("http://www.vibranium.co.in"));
		if (m_objWinHttpManager.Initialize(szTestUrl))
		{
			if (m_objWinHttpManager.CreateRequestHandle(NULL))
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardWizALUSrv::CheckInternetConnection", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : WaitForInternetConnection()
*  Description    : It waits internet connection for 5 min .
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           :	20th Aug,2015
****************************************************************************************************/
bool CWrdWizDownloaderDlg::WaitForInternetConnection()
{
	try
	{
		bool bReturn = false;
		int iRetryCount = 0;
		while (true)
		{
			if (!CheckInternetConnection())
			{
				if (iRetryCount > 3)
				{
					bReturn = false;
					break;
				}
				iRetryCount++;
				Sleep(1 * 1000);//wait here for 1 seconds
			}
			else
			{
				bReturn = true;
				break;
			}
		}

		return bReturn;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::WaitForInternetConnection", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


/***************************************************************************************************
*  Function Name  : StartDownloadingWardWizSetup()
*  Description    : Start downloading setup file .
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date			  :	20th Aug,2015.
****************************************************************************************************/
bool CWrdWizDownloaderDlg::StartDownloadingWardWizSetup(LPCTSTR szUrlPath)
{
	try
	{
		if (!szUrlPath)
			return false;

		CString csTempFileName(L"");
		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);
		m_csFileName = csFileName;
		csTempFileName.Format(L"%s.%s",csFileName,L"PART_1");
		TCHAR szInfo[MAX_PATH] = { 0 };
		TCHAR szTagetPath[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		DWORD dwTotalFileSize = 0, dwLastSetupTotalSize = 0;
		TCHAR szLastSetupFileSize[MAX_PATH] = { 0 };


		if (m_objWinHttpManager.Initialize(szUrlPath))
		{
			if (!m_objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
			{
				return false;
			}
			dwTotalFileSize = _wtol(szInfo);

			m_bRequiredSpaceNotavailable = false;
			if (!IsDriveHaveRequiredSpace(m_csDrivePath, 1, dwTotalFileSize))
			{
				m_bRequiredSpaceNotavailable = true;
				return false;
			}


#ifdef WRDWIZBASIC
#ifdef RELEASENCG
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGB.INI");
#elif RELEASENCI
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIB.INI");
#endif
#elif WRDWIZBASICPATCH
	#ifdef RELEASENCG
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGBPatch.INI");
	#elif RELEASENCI
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIBPatch.INI");
	#endif
#elif WRDWIZESSNL
#ifdef RELEASENCG
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEss.INI");
#elif RELEASENCI
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEss.INI");
#endif
#elif WRDWIZESSNLPATCH
	#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPatch.INI");
	#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPatch.INI");
	#endif
#elif WRDWIZESSPLUS
#ifdef RELEASENCG
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlus.INI");
#elif RELEASENCI
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlus.INI");
#endif
#elif WRDWIZESSPLUSPATCH
	#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlusPatch.INI");
	#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlusPatch.INI");
	#endif
#else
#ifdef RELEASENCG
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGP.INI");
#elif RELEASENCI
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIP.INI");
#endif
#endif


			GetPrivateProfileString(L"Last Setup Size", L"1", L"", szLastSetupFileSize, (MAX_PATH - 1), m_szIniFilePath);

			if (szLastSetupFileSize[0])
			{
				dwLastSetupTotalSize = _wtol(szLastSetupFileSize);
			}

			m_dwTotalFileSize = dwTotalFileSize;

			CString csBrowseFolderName(L"");
			m_edtBrowsedSetupPath.GetWindowTextW(csBrowseFolderName);
			if (PathFileExists(csBrowseFolderName))
			{
				AddLogEntry(L">>> WardWiz Setup downloaded path %s",csBrowseFolderName, 0, true, SECONDLEVEL);
				_tcscpy_s(m_szBrowseFolderName, _countof(m_szBrowseFolderName), csBrowseFolderName);
			}
			else
			{
				AddLogEntry(L"### %s Folder is not exist", csBrowseFolderName, 0, true, FIRSTLEVEL);
				return false;
			}

			_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, csTempFileName);

			m_csTempTargetFilePath.Format(L"%s", szTagetPath);

			//m_csFileTargetPath.Format(L"%s\\%s", m_szBrowseFolderName, csFileName);


			//If file is already present check filesize and download from last point
			DWORD dwStartBytes = 0;

			if (dwLastSetupTotalSize == m_dwTotalFileSize)
			{
				if (PathFileExists(szTagetPath))
				{
					//HANDLE hFile = CreateFile(szTagetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
					m_hFile = CreateFile(szTagetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
					if (m_hFile != INVALID_HANDLE_VALUE)
					{
						dwStartBytes = GetFileSize(m_hFile, 0);
						if (dwStartBytes != dwTotalFileSize)
						{
							m_objWinHttpManager.SetDownloadCompletedBytes(dwStartBytes);
						}
					}
					CloseHandle(m_hFile);
					m_hFile = NULL;
				}
			}

			WritePrivateProfileString(L"Last Setup Size", L"1", szInfo, m_szIniFilePath);

			//if physics file size is greater than server file size then download from BEGIN
			if (dwStartBytes > dwTotalFileSize)
			{
				m_objWinHttpManager.SetDownloadCompletedBytes(0);
				dwStartBytes = 0;
			}

			//We have already downloaded the file no need to download again
			if (dwStartBytes == dwTotalFileSize)
			{
				m_objWinHttpManager.SetDownloadCompletedBytes(dwTotalFileSize);
				return true;
			}

			//Start download for file
			if (m_objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize))
			{
				//Once download complete set the download completed bytes.
				m_objWinHttpManager.SetDownloadCompletedBytes(dwTotalFileSize - dwStartBytes);
			}
			else
			{
				AddLogEntry(L"### Failed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::StartDownloadingWardWizSetup", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonDownloadBrowse
*  Description    : Broswe the folder where we want to stored.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedButtonDownloadBrowse()
{
	try
	{
		CString csBroswsePathForDownloadSetup(L"");
		TCHAR *pszPath = new TCHAR[MAX_PATH];
		SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));

		CString csMessage = theApp.GetString(IDS_STRING_SELECT_DOWNLOAD_SETUP_FOLDER);
		BROWSEINFO        bi = { m_hWnd, NULL, pszPath, csMessage, BIF_RETURNONLYFSDIRS, NULL, 0L, 0 };
		LPITEMIDLIST      pIdl = NULL;

		LPITEMIDLIST  pRoot = NULL;
		//SHGetFolderLocation(m_hWnd, CSIDL_DRIVES, 0, 0, &pRoot);
		bi.pidlRoot = pRoot;
		pIdl = SHBrowseForFolder(&bi);
		if (NULL != pIdl)
		{
			SHGetPathFromIDList(pIdl, pszPath);
			size_t iLen = wcslen(pszPath);
			int iSlashPos = 0;
			if (iLen > 0)
			{
				if (pszPath != NULL)
				{
					csBroswsePathForDownloadSetup = pszPath;
					iSlashPos = csBroswsePathForDownloadSetup.Find(L'\\');
					if (iSlashPos != -1)
					{
						csBroswsePathForDownloadSetup.TrimRight(L'\\');
					}
					m_edtBrowsedSetupPath.SetWindowTextW(csBroswsePathForDownloadSetup);
				}
			}
		}
		delete[] pszPath;
		pszPath = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnBnClickedButtonDownloadBrowse", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : StartWardWizSetupDownloadProcessThread()
*  Description    : Download the wardwiz setup thread
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date			  :	20th Aug,2015.
* Issue no 1320   : Setup FileName is not coming properly.
****************************************************************************************************/
DWORD WINAPI StartWardWizSetupDownloadProcessThread(LPVOID lpParam)
{
	try
	{
		CWrdWizDownloaderDlg *pThis = (CWrdWizDownloaderDlg*)lpParam;
		if (!pThis)
			return 0;

		CString csDrivePath(L"");
		CString csMsgBox(L"");
		CString csBrowsePath(L""), csFiletoCheckAccess(L"");
		pThis->m_edtBrowsedSetupPath.GetWindowTextW(csBrowsePath);
		//pThis->ShowControlOnClickOfStart(true);
		bool bIsBrowsePathCanceled = false;
		csDrivePath = csBrowsePath;
		int iPos = csDrivePath.Find(L"\\");
		if (iPos != -1)
		{

			csDrivePath.Truncate(iPos);
			pThis->m_csDrivePath = csDrivePath;
		}
		else
		{
			//Issue no 1232 : When only drive is given
			pThis->m_csDrivePath = csDrivePath;
		}

		if ((!PathFileExists(csBrowsePath)) || PathIsNetworkPath(csBrowsePath))
		{
			pThis->KillTimer(TIMER_SETPERCENTAGE);
			//	pThis->KillTimer(TIMER_FORADVERTISEMENT);
			if (pThis->m_csLangType == L"ENGLISH")
			{
				csMsgBox.Format(L"%s\n\n%s", theApp.GetString(IDS_STRING_SELECT_VALID_PATH), theApp.GetString(IDS_STRING_CD_DRIVE_NOT_ALLOW));
			}
			else
			{
				csMsgBox.Format(L"%s\n\n%s", theApp.GetString(IDS_STRING_SELECT_VALID_PATH_G), theApp.GetString(IDS_STRING_CD_DRIVE_NOT_ALLOW_G));
			}
			//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
			pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);


			pThis->m_bIsDownloadingInProgress = false;
			//pThis->ShowControlOnClickOfStart(false);
			pThis->m_objWinHttpManager.m_bIsConnected = false;
			return 0x01;
		}

		csFiletoCheckAccess.Format(L"%s\\%s", csBrowsePath,L"Temp.txt");
		HANDLE hFile = CreateFile(csFiletoCheckAccess, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwerr = GetLastError();
			if (dwerr == ERROR_ACCESS_DENIED)
			{
				//pThis->ShowControlOnClickOfStart(false);
				pThis->KillTimer(TIMER_SETPERCENTAGE);
				//pThis->KillTimer(TIMER_FORADVERTISEMENT);
				if (pThis->m_csLangType == L"ENGLISH")
				{
					csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_SELECTED_FOLDER_NOT_ACCESS), theApp.GetString(IDS_STRING_BROWSE_ANOTHER_FOLDER));
				}
				else
				{
					csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_SELECTED_FOLDER_NOT_ACCESS_G), theApp.GetString(IDS_STRING_BROWSE_ANOTHER_FOLDER_G));
				}
				//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
				pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
				pThis->m_bIsDownloadingInProgress = false;
				//pThis->m_btndwnldpathBrowse.EnableWindow(TRUE);
				pThis->m_objWinHttpManager.m_bIsConnected = false;
				return 0x02;
			}
		}
		CloseHandle(hFile);
		hFile = NULL;

		if (PathFileExists(csFiletoCheckAccess))
		{
			if (!DeleteFile(csFiletoCheckAccess))
			{
				AddLogEntry(L"### Failed to delete temp access check file", 0, 0, true, ZEROLEVEL);
			}
		}

		pThis->m_bIsDownloadingInProgress = true;
		pThis->m_objWinHttpManager.m_bIsConnected = false;
		pThis->m_objWinHttpManager.m_dwCompletedDownloadBytes = 0;
		pThis->m_tsStartTime = CTime::GetCurrentTime();
		pThis->m_stActualDownloadPath.SetWindowTextW(L"  :  " + csBrowsePath);
		pThis->m_stActualDownloadPath.RedrawWindow();

#ifdef WRDWIZBASIC
		//if ((pThis->m_bIsWow64 == true && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
#ifdef RELEASENCG
			theApp.m_csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC Germany 64 url
#elif RELEASENCI
			theApp.m_csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC India 64 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
#ifdef RELEASENCG
			theApp.m_csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC 32 url
#elif RELEASENCI
			theApp.m_csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC 32 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == true))
		//{
		//	#ifdef RELEASENCG
		//				theApp.m_csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC 32 url
		//	#elif RELEASENCI
		//				theApp.m_csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC 32 url
		//	#else
		//				theApp.m_csFilePath.Format(L"%s", BASIC86SETUPNAME);//Basic with clam 32 url
		//	#endif
		//}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == true))
		//{
		//	#ifdef RELEASENCG
		//				theApp.m_csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC 64 url
		//	#elif RELEASENCI
		//				theApp.m_csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC 64 url
		//	#else
		//				theApp.m_csFilePath.Format(L"%s", BASIC64SETUPNAME);//Basic with clam  64 url
		//	#endif
		//}

#elif WRDWIZBASICPATCH
		//if ((pThis->m_bIsWow64 == true && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
			#ifdef RELEASENCG
						theApp.m_csFilePath.Format(L"%s", BASICNCG64PATCHNAME);//Basic NC Germany 64 url
			#elif RELEASENCI
						theApp.m_csFilePath.Format(L"%s", BASICNCI64PATCHNAME);//Basic NC India 64 url
			#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
			#ifdef RELEASENCG
						theApp.m_csFilePath.Format(L"%s", BASICNCG86PATCHNAME);//Basic NC 32 url
			#elif RELEASENCI
						theApp.m_csFilePath.Format(L"%s", BASICNCI86PATCHNAME);//Basic NC 32 url
			#endif
		}
#elif WRDWIZESSNL
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
#ifdef RELEASENCG
			theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
#elif RELEASENCI
			theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
#ifdef RELEASENCG
			theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
#elif RELEASENCI
			theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
#endif
		}
#elif WRDWIZESSNLPATCH
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
			#ifdef RELEASENCG
						theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG64PATCHNAME);//Ess NC 64 url
			#elif RELEASENCI
						theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI64PATCHNAME);//Ess NC 64 url
			#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
			#ifdef RELEASENCG
						theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG86PATCHNAME);//Ess NC 32 url
			#elif RELEASENCI
						theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI86PATCHNAME);//Ess NC 32 url
			#endif
		}
#elif WRDWIZESSPLUS
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
#ifdef RELEASENCG
			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG64SETUPNAME);//EssPlus NC 64 url
#elif RELEASENCI
			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI64SETUPNAME);//EssPlus NC 64 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
#ifdef RELEASENCG
			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG86SETUPNAME);//EssPlus NC 32 url
#elif RELEASENCI
			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI86SETUPNAME);//EssPlus NC 32 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == true))
		//{
		//	#ifdef RELEASENCG
		//				theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
		//	#elif RELEASENCI
		//				theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
		//	#else
		//				theApp.m_csFilePath.Format(L"%s", ESSENTIAL86SETUPNAME);//Ess with clam 32 url
		//	#endif
		//}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == true))
		//{
		//	#ifdef RELEASENCG
		//				theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
		//	#elif RELEASENCI
		//				theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
		//	#else
		//				theApp.m_csFilePath.Format(L"%s", ESSENTIAL64SETUPNAME);//Ess with clam  64 url
		//	#endif
		//}
#elif WRDWIZESSPLUSPATCH
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
			#ifdef RELEASENCG
						theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG64PATCHNAME);//EssPlus NC 64 url
			#elif RELEASENCI
						theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI64PATCHNAME);//EssPlus NC 64 url
			#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
			#ifdef RELEASENCG
						theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG86PATCHNAME);//EssPlus NC 32 url
			#elif RELEASENCI
						theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI86PATCHNAME);//EssPlus NC 32 url
			#endif
		}
#else
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
#ifdef RELEASENCG
			theApp.m_csFilePath.Format(L"%s", PRONCG64SETUPNAME);//pro NC 64 url
#elif RELEASENCI
			theApp.m_csFilePath.Format(L"%s", PRONCI64SETUPNAME);//Pro NC 64 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
#ifdef RELEASENCG
			theApp.m_csFilePath.Format(L"%s", PRONCG86SETUPNAME);//pro NC 32 url
#elif RELEASENCI
			theApp.m_csFilePath.Format(L"%s", PRONCI86SETUPNAME);//Pro NC 32 url
#endif
		}
#endif
		CString csFilePath = L"";
		csFilePath.Format(L"%s\\%s", csBrowsePath, theApp.m_csFilePath);
		pThis->m_stActualDownloadPath.SetWindowTextW(L"  :  " + csFilePath);
		pThis->m_stActualDownloadPath.RedrawWindow();
		pThis->EndWaitCursor();
		pThis->m_svFunGetDownloadPathCB.call((SCITER_STRING)csFilePath, (SCITER_STRING)pThis->m_csActualProdName);

		//Issue No. 0001618: If machine gets restarts while downloading is in progress in win 10 after restarting the machine download is not getting started.
		pThis->SetStartupEntry(pThis->m_szModulePath);

		pThis->m_stActualStatus.SetWindowTextW(theApp.GetString(IDS_STRING_CHECK_INTERNET_CONNECTION));
		DWORD dwRetry = 0;


		if (!pThis->CheckInternetConnection())
		{
			/*if (!pThis->WaitForInternetConnection())
			{
			pThis->m_stActualStatus.SetWindowTextW(L"  :  Internet connection not available");
			AddLogEntry(L">>> Retry to check internet connection for google.com ", 0, 0, true, FIRSTLEVEL);
			}*/
			pThis->m_bIsCloseCalled = false;
			CString csMsg = L"";
			if (pThis->m_csLangType == L"ENGLISH")
			{
				csMsg.Format(L"%s \n%s", theApp.GetString(IDS_STRING_UNABLE_TO_DETECT_CONNECT), theApp.GetString(IDS_STRING_REQ_TO_CHECK_INETERNET));
			}
			else
			{
				csMsg.Format(L"%s \n%s", theApp.GetString(IDS_STRING_UNABLE_TO_DETECT_CONNECT_G), theApp.GetString(IDS_STRING_REQ_TO_CHECK_INETERNET_G));
			}
			//	pThis->ShowControlOnClickOfStart(true);
			pThis->KillTimer(TIMER_SETPERCENTAGE);
			//		pThis->KillTimer(TIMER_FORADVERTISEMENT);
			//pThis->MessageBox(csMsg, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
			pThis->m_bIsDownloadingInProgress = false;
			pThis->m_bIsProcessPaused = true;
			pThis->m_svFunShowNotificationMsgCB.call(5, (SCITER_STRING)csMsg);
			return 0x01;
		}
		else
		{
			AddLogEntry(L">>> internet connection available", 0, 0, true, FIRSTLEVEL);
			//break;
		}

		pThis->m_stActualStatus.SetWindowTextW(theApp.GetString(IDS_STRING_DOWNLOAD_IN_PROGRESS));
		pThis->m_objWinHttpManager.m_bIsConnected = true;

		pThis->m_objWinHttpManager.StartCurrentDownload();



#ifdef WRDWIZBASIC
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
#ifdef RELEASENCG
			theApp.m_csurl.Format(L"%s", BASICNCG64);//Basic NC Germany 64 url
#elif RELEASENCI
			theApp.m_csurl.Format(L"%s", BASICNCI64);//Basic NC India 64 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
#ifdef RELEASENCG
			theApp.m_csurl.Format(L"%s", BASICNCG86);//Basic NC 32 url
#elif RELEASENCI
			theApp.m_csurl.Format(L"%s", BASICNCI86);//Basic NC 32 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == true))
		//{
		//	#ifdef RELEASENCG
		//	theApp.m_csurl.Format(L"%s", BASICNCG86);//Basic NC 32 url
		//	#elif RELEASENCI
		//	theApp.m_csurl.Format(L"%s", BASICNCI86);//Basic NC 32 url
		//	#else
		//	theApp.m_csurl.Format(L"%s", BASIC86);//Basic with clam 32 url
		//	#endif

		//	pThis->m_chkLaunchDownloadedExe.EnableWindow(FALSE);
		//	pThis->m_bchkLaunchExe = false;
		//}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == true))
		//{
		//	#ifdef RELEASENCG
		//	theApp.m_csurl.Format(L"%s", BASICNCG64);//Basic NC 64 url
		//	#elif RELEASENCI
		//	theApp.m_csurl.Format(L"%s", BASICNCI64);//Basic NC 64 url
		//	#else
		//	theApp.m_csurl.Format(L"%s", BASIC64);//Basic with clam  64 url
		//	#endif

		//	pThis->m_chkLaunchDownloadedExe.EnableWindow(FALSE);
		//	pThis->m_bchkLaunchExe = false;
		//}
#elif WRDWIZBASICPATCH
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
			#ifdef RELEASENCG
						theApp.m_csurl.Format(L"%s", BASICNCG64PATCH);//Basic NC Germany 64 url
			#elif RELEASENCI
						theApp.m_csurl.Format(L"%s", BASICNCI64PATCH);//Basic NC India 64 url
			#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
			#ifdef RELEASENCG
						theApp.m_csurl.Format(L"%s", BASICNCG86PATCH);//Basic NC 32 url
			#elif RELEASENCI
						theApp.m_csurl.Format(L"%s", BASICNCI86PATCH);//Basic NC 32 url
			#endif
		}
#elif WRDWIZESSNL
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
#ifdef RELEASENCG
			theApp.m_csurl.Format(L"%s", ESSENTIALNCG64);//Ess NC 64 url
#elif RELEASENCI
			theApp.m_csurl.Format(L"%s", ESSENTIALNCI64);//Ess NC 64 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == false))//&& (pThis->m_bchkDownloadoption == false))
		else
		{
#ifdef RELEASENCG
			theApp.m_csurl.Format(L"%s", ESSENTIALNCG86);//Ess NC 32 url
#elif RELEASENCI
			theApp.m_csurl.Format(L"%s", ESSENTIALNCI86);//Ess NC 32 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == true))
		//{
		//	#ifdef RELEASENCG
		//	theApp.m_csurl.Format(L"%s", ESSENTIALNCG86);//Ess NC 32 url
		//	#elif RELEASENCI
		//	theApp.m_csurl.Format(L"%s", ESSENTIALNCI86);//Ess NC 32 url
		//	#else
		//	theApp.m_csurl.Format(L"%s", ESSENTIAL86);//Ess with clam 32 url
		//	pThis->MessageBox(theApp.m_csurl, L"MICRO", MB_OK);
		//	#endif

		//	pThis->m_chkLaunchDownloadedExe.EnableWindow(FALSE);
		//	pThis->m_bchkLaunchExe = false;
		//}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == true))
		//{
		//	#ifdef RELEASENCG
		//	theApp.m_csurl.Format(L"%s", ESSENTIALNCG64);//Ess NC 64 url
		//	#elif RELEASENCI
		//	theApp.m_csurl.Format(L"%s", ESSENTIALNCI64);//Ess NC 64 url
		//	#else
		//	theApp.m_csurl.Format(L"%s", ESSENTIAL64);//Ess with clam  64 url
		//	#endif

		//	pThis->m_chkLaunchDownloadedExe.EnableWindow(FALSE);
		//	pThis->m_bchkLaunchExe = false;
		//}
#elif WRDWIZESSNLPATCH
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
			#ifdef RELEASENCG
						theApp.m_csurl.Format(L"%s", ESSNCG64PATCH);//Ess NC 64 url
			#elif RELEASENCI
						theApp.m_csurl.Format(L"%s", ESSNCI64PATCH);//Ess NC 64 url
			#endif
		}
		//else if ((pThis->m_bIsWow64 == false))//&& (pThis->m_bchkDownloadoption == false))
		else
		{
			#ifdef RELEASENCG
						theApp.m_csurl.Format(L"%s", ESSNCG86PATCH);//Ess NC 32 url
			#elif RELEASENCI
						theApp.m_csurl.Format(L"%s", ESSNCI86PATCH);//Ess NC 32 url
			#endif
		}

#elif WRDWIZESSPLUS
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
#ifdef RELEASENCG
			theApp.m_csurl.Format(L"%s", ESSPLUSNCG64);//EssPlus NC 64 url
#elif RELEASENCI
			theApp.m_csurl.Format(L"%s", ESSPLUSNCI64);//EssPlus NC 64 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == false))//&& (pThis->m_bchkDownloadoption == false))
		else
		{
#ifdef RELEASENCG
			theApp.m_csurl.Format(L"%s", ESSPLUSNCG86);//EssPlus NC 32 url
#elif RELEASENCI
			theApp.m_csurl.Format(L"%s", ESSPLUSNCI86);//EssPlus NC 32 url
#endif
		}
#elif WRDWIZESSPLUSPATCH
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
			#ifdef RELEASENCG
				theApp.m_csurl.Format(L"%s", ESSPLNCG64PATCH);//EssPlus NC 64 url
			#elif RELEASENCI
				theApp.m_csurl.Format(L"%s", ESSPLNCI64PATCH);//EssPlus NC 64 url
			#endif
		}
		//else if ((pThis->m_bIsWow64 == false))//&& (pThis->m_bchkDownloadoption == false))
		else
		{
			#ifdef RELEASENCG
				theApp.m_csurl.Format(L"%s", ESSPLNCG86PATCH);//EssPlus NC 32 url
			#elif RELEASENCI
				theApp.m_csurl.Format(L"%s", ESSPLNCI86PATCH);//EssPlus NC 32 url
			#endif
		}
#else
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		if (pThis->m_bIsWow64 == true)
		{
#ifdef RELEASENCG
			theApp.m_csurl.Format(L"%s", PRONCG64);//pro NC 64 url
#elif RELEASENCI
			theApp.m_csurl.Format(L"%s", PRONCI64);//Pro NC 64 url
#endif
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		else
		{
#ifdef RELEASENCG
			theApp.m_csurl.Format(L"%s", PRONCG86);//pro NC 32 url
#elif RELEASENCI
			theApp.m_csurl.Format(L"%s", PRONCI86);//Pro NC 32 url
#endif
		}
#endif
		CString strStartTime = pThis->m_tsStartTime.Format("%H:%M:%S");

		AddLogEntry(L">>> Downloaded started at %s", strStartTime,0,true,SECONDLEVEL);
		AddLogEntry(L">>> Wardwiz Setup Server path %s", theApp.m_csurl, 0, true, SECONDLEVEL);

		CString csNewBrowsePath;
		bool bFailed = false;


		CString csTempFileName(L"");
		CString csFileName(theApp.m_csurl);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);
		pThis->m_csFileName = csFileName;

		pThis->m_csFileTargetPath.Format(L"%s\\%s", csBrowsePath, csFileName);
		//Issue No. 0001485 resolved: Issue with Download Manager. If already setup exist on selected path, user can download the same setup again, It is not giving any Information popup.
		if (PathFileExists(pThis->m_csFileTargetPath))
		{
			pThis->m_bIsCloseCalled = true;
			if (pThis->m_csLangType == L"ENGLISH")
			{
				csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_SETUP_ALREADY_EXIST), theApp.GetString(IDS_STRING_WANT_TO_OVERWRITE));
			}
			else
			{
				csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_SETUP_ALREADY_EXIST_G), theApp.GetString(IDS_STRING_WANT_TO_OVERWRITE_G));
			}
			sciter::value v_svVarOnClickOverWriteCB=pThis->m_svFunShowNotificationMsgCB.call(2, (SCITER_STRING)csMsgBox);
			//if (pThis->MessageBox(csMsgBox, L"Vibranium", MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
			if (v_svVarOnClickOverWriteCB == 1)
			{
				AddLogEntry(L"### Failed to save file to %s path. Setup file already exists.", pThis->m_csFileTargetPath, 0, true, FIRSTLEVEL);
				pThis->KillTimer(TIMER_SETPERCENTAGE);
				pThis->m_bIsDownloadingInProgress = false;
				pThis->m_objWinHttpManager.m_bIsConnected = false;
				//return 0x01;
				pThis->OnBnClickedButtonWindowClose();
			}
			pThis->m_bIsCloseCalled = false;
		}
		//Issue No. 0001573: Setup file already in use pop-up is getting appeared after the downloading gets completed and when we click "OK" on pop-up downloader should get closed.
		CEnumProcess objEnumProcess;
		if (objEnumProcess.IsProcessRunning(theApp.m_csFilePath, false, false, false))
		{
			AddLogEntry(L"### Failed to move file to %s path", pThis->m_csFileTargetPath, 0, true, FIRSTLEVEL);
			if (pThis->m_csLangType == L"ENGLISH")
			{
				csMsgBox.Format(L"%s %s", pThis->m_csFileTargetPath, theApp.GetString(IDS_STRING_FILE_ALREADY_IN_USE));
			}
			else
			{
				csMsgBox.Format(L"%s %s", pThis->m_csFileTargetPath, theApp.GetString(IDS_STRING_FILE_ALREADY_IN_USE_G));
			}
			pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
			pThis->m_bIsDownloadingInProgress = false;
			pThis->OnBnClickedButtonClose();
		}


		//Issue Resolved: 0002663, Retry here if download fails.
		bool bIsDownloadSuccess = false;
		DWORD dwRetryCount = 0x00;
		while (dwRetryCount < MAX_RETRY_COUNT)
		{
			if (pThis->StartDownloadingWardWizSetup(theApp.m_csurl))
			{
				bIsDownloadSuccess = true;
				break;
			}
			else
			{
				AddLogEntry(L"### Failed to Download Setup: %s, Retrying...", theApp.m_csurl, 0, true, ZEROLEVEL);
				dwRetryCount++;
			}
		}

		if (bIsDownloadSuccess)
		{
			int iLowRange = 0, iHighRange = 0;
			pThis->m_prgDownloadSetupStatus.GetRange(iLowRange, iHighRange);

			CString csStr(L"");
			if (pThis->m_dwTotalFileSize >= 1000000)
			{
				if (pThis->m_iCurrentDownloadedByte >= 1000000)
				{
					csStr.Format(L"%d %s (%d MB %s %d MB)", iHighRange, L"%", static_cast<DWORD>(pThis->m_dwTotalFileSize / (1000 * 1024)), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(pThis->m_dwTotalFileSize / (1000 * 1024)));
				}
				else
				{
					csStr.Format(L"%d %s (%d KB %s %d MB)", iHighRange, L"%", static_cast<DWORD>(pThis->m_dwTotalFileSize / (1024)), theApp.GetString(IDS_STRING_DOWNLOAD_OF),static_cast<DWORD>(pThis->m_dwTotalFileSize / (1000 * 1024)));
				}
			}
			else
			{
				csStr.Format(L"%d %s (%d KB %s %d KB)", iHighRange, L"%", static_cast<DWORD>(pThis->m_dwTotalFileSize / 1024), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(pThis->m_dwTotalFileSize / 1024));
			}

			pThis->m_prgDownloadSetupStatus.SetWindowTextW(csStr);
			pThis->m_prgDownloadSetupStatus.SetPos(100);
			pThis->m_prgDownloadSetupStatus.RedrawWindow();

			if (!CopyFile(pThis->m_csTempTargetFilePath, pThis->m_csFileTargetPath, FALSE))
			{
				CString csTemp;
				DWORD dwErr = GetLastError();
				csTemp.Format(L"%d", dwErr);
				AddLogEntry(L"### Failed to move file to %s path with last error %s", pThis->m_csFileTargetPath, csTemp, true, FIRSTLEVEL);

				if (!PathFileExists(pThis->m_szBrowseFolderName) || (dwErr == ERROR_ACCESS_DENIED))
				{
					CString csMsgBox, csNewBrowsePath;
					if ((dwErr == ERROR_ACCESS_DENIED))
					{
						if (pThis->m_csLangType == L"ENGLISH")
						{
							csMsgBox.Format(L"%s %s %s \n%s", theApp.GetString(IDS_STRING_UNABLE_COPY_FILE), pThis->m_szBrowseFolderName, theApp.GetString(IDS_STRING_PATH_DENIED), theApp.GetString(IDS_STRING_ALTERNATE_PATH));
						}
						else
						{
							csMsgBox.Format(L"%s %s %s \n%s", theApp.GetString(IDS_STRING_UNABLE_COPY_FILE_G), pThis->m_szBrowseFolderName, theApp.GetString(IDS_STRING_PATH_DENIED_G), theApp.GetString(IDS_STRING_ALTERNATE_PATH_G));
						}
					}
					else
					{
						if (pThis->m_csLangType == L"ENGLISH")
						{
							csMsgBox.Format(L"%s %s\n%s", pThis->m_szBrowseFolderName, theApp.GetString(IDS_STRING_PATH_NOT_AVAIL), theApp.GetString(IDS_STRING_ALTERNATE_PATH));
						}
						else
						{
							csMsgBox.Format(L"%s %s\n%s", pThis->m_szBrowseFolderName, theApp.GetString(IDS_STRING_PATH_NOT_AVAIL_G), theApp.GetString(IDS_STRING_ALTERNATE_PATH_G));
						}
					}
					pThis->KillTimer(TIMER_SETPERCENTAGE);
					//pThis->KillTimer(TIMER_FORADVERTISEMENT);
					pThis->m_stActualTransferRate.SetWindowTextW(L"  :  0.00 kbps");

					//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
					pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);

					pThis->OnBnClickedButtonDownloadBrowse();
					pThis->m_edtBrowsedSetupPath.GetWindowTextW(csNewBrowsePath);
					if (!PathFileExists(csNewBrowsePath))
					{
						if (pThis->m_csLangType == L"ENGLISH")
						{
							csMsgBox.Format(L"%s %s \n%s", csNewBrowsePath, theApp.GetString(IDS_STRING_PATH_NOT_AVAIL), theApp.GetString(IDS_STRING_FAIL_TO_STORE));
						}
						else
						{
							csMsgBox.Format(L"%s %s \n%s", csNewBrowsePath, theApp.GetString(IDS_STRING_PATH_NOT_AVAIL_G), theApp.GetString(IDS_STRING_FAIL_TO_STORE_G));
						}
						//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
						pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
						bIsBrowsePathCanceled = true;
					}
					else
					{
						pThis->m_csFileTargetPath.Format(L"%s\\%s", csNewBrowsePath, pThis->m_csFileName);
						if (PathFileExists(csNewBrowsePath))
						{
							if (!CopyFile(pThis->m_csTempTargetFilePath, pThis->m_csFileTargetPath, FALSE))
							{
								DWORD dwErr = GetLastError();
								if ((dwErr == ERROR_ACCESS_DENIED))
								{
									if (pThis->m_csLangType == L"ENGLISH")
									{
										csMsgBox.Format(L"%s %s %s \n%s", theApp.GetString(IDS_STRING_UNABLE_COPY_FILE), csNewBrowsePath, theApp.GetString(IDS_STRING_PATH_DENIED), theApp.GetString(IDS_STRING_TRY_TO_DOWNLOAD_AGAIN));
									}
									else
									{
										csMsgBox.Format(L"%s %s %s \n%s", theApp.GetString(IDS_STRING_UNABLE_COPY_FILE_G), csNewBrowsePath, theApp.GetString(IDS_STRING_PATH_DENIED_G), theApp.GetString(IDS_STRING_TRY_TO_DOWNLOAD_AGAIN_G));
									}
									//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
									pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
								}
								bIsBrowsePathCanceled = true;
								AddLogEntry(L"### Failed to move file to %s path", csNewBrowsePath, 0, true, FIRSTLEVEL);
							}
						}
					}
				}
				else
				{

					if (!CopyFile(pThis->m_csTempTargetFilePath, pThis->m_csFileTargetPath, FALSE))
					{
						DWORD dwErr = GetLastError();
						if (dwErr == 0x20)
						{
							AddLogEntry(L"### Failed to move file to %s path", pThis->m_csFileTargetPath, 0, true, FIRSTLEVEL);
							if (pThis->m_csLangType == L"ENGLISH")
							{
								csMsgBox.Format(L"%s %s", pThis->m_csFileTargetPath, theApp.GetString(IDS_STRING_FILE_ALREADY_IN_USE));
							}
							else
							{
								csMsgBox.Format(L"%s %s", pThis->m_csFileTargetPath, theApp.GetString(IDS_STRING_FILE_ALREADY_IN_USE_G));
							}
							//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
							pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
							//Issue No:0001573 Setup file already in use pop-up is getting appeared after the downloading gets completed and when we click "OK" on pop-up downloader should get closed.
							bIsBrowsePathCanceled = true;
							pThis->m_bIsDownloadingInProgress = false;
							pThis->OnBnClickedButtonClose();

						}
						else
						{
							AddLogEntry(L"### Failed to move file to %s path", pThis->m_csFileTargetPath, 0, true, FIRSTLEVEL);
							if (pThis->m_csLangType == L"ENGLISH")
							{
								csMsgBox.Format(L"%s %s", theApp.GetString(IDS_STRING_FAIL_TO_STORE), theApp.GetString(IDS_STRING_TRY_TO_DOWNLOAD_AGAIN));
							}
							else
							{
								csMsgBox.Format(L"%s %s", theApp.GetString(IDS_STRING_FAIL_TO_STORE_G), theApp.GetString(IDS_STRING_TRY_TO_DOWNLOAD_AGAIN_G));
							}
							//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
							pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
							bIsBrowsePathCanceled = true;
						}
					}
					else
					{
						if (PathFileExists(pThis->m_csTempTargetFilePath))
						{
							if (!DeleteFile(pThis->m_csTempTargetFilePath))
							{
								AddLogEntry(L"### Failed to delete file", 0, 0, true, FIRSTLEVEL);
							}
						}
					}
				}
			}

			if (PathFileExists(pThis->m_csTempTargetFilePath))
			{
				if (!DeleteFile(pThis->m_csTempTargetFilePath))
				{
					AddLogEntry(L"### Failed to delete file", 0, 0, true, FIRSTLEVEL);
				}
			}

			// issue :in case of re- download progress bar once showing 100 percent then showing actual status.
			// resolved by lalit kumawat 9-8-2015

			pThis->m_iCurrentDownloadedByte = pThis->m_dwTotalFileSize = 0;

			AddLogEntry(L">>> EXE file downloaded successfully", 0, 0, true, ZEROLEVEL);
			pThis->m_bIsDownloadingInProgress = false;
			pThis->RestoreWndFromTray(pThis->m_hWnd);
			pThis->m_objWinHttpManager.StopCurrentDownload();
			pThis->m_objWinHttpManager.SetDownloadCompletedBytes(0);
			pThis->m_objWinHttpManager.CloseFileHandles();
			pThis->KillTimer(TIMER_SETPERCENTAGE);
			//pThis->KillTimer(TIMER_FORADVERTISEMENT);
			pThis->m_objWinHttpManager.m_bIsConnected = false;

			if (PathFileExists(pThis->m_szIniFilePath))
			{
				if (DeleteFile(pThis->m_szIniFilePath))
				{
					AddLogEntry(L"### Filed to delete %s file", pThis->m_szIniFilePath, 0, true, FIRSTLEVEL);
				}
			}

			if (pThis->m_hFile != NULL)
			{
				CloseHandle(pThis->m_hFile);
				pThis->m_hFile = NULL;
			}
			if (pThis->GetStartupEntry())
			{
				HKEY hKey;
				LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);

				if (openRes != ERROR_SUCCESS)
				{
					openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
					if (openRes != ERROR_SUCCESS)
					{
						AddLogEntry(L"### Error in opening RUN key", 0, 0, true, SECONDLEVEL);
					}
				}
				long lResult = RegDeleteValueW(hKey, L"WWizDownloader");

				if (lResult != ERROR_SUCCESS)
				{
					AddLogEntry(L"### Filed to delete registry file", 0, 0, true, FIRSTLEVEL);
				}
			}

			if (pThis->m_bchkOpenFolder == true)
			{
				CString csOpenFolderPath(L"");
				csOpenFolderPath.Format(L"%s", pThis->m_csFileTargetPath);
				int Pos = csOpenFolderPath.ReverseFind('\\');
				csOpenFolderPath.Truncate(Pos);
				ShellExecute(NULL, L"open", csOpenFolderPath, NULL, NULL, SW_SHOW);
			}


			pThis->m_objWinHttpManager.m_bIsConnected = false;
			//ini file into program data\wardwiz folder.

			// issue :812, Issue with the popup appearing after cancelling to store the setup in other location if USB drive not available.
			// resolved by lalit kumawat 9-8-2015

			if (!bIsBrowsePathCanceled)
			{
				if (pThis->m_bchkLaunchExe == true)
				{
					if (bFailed)
					{
						pThis->m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_FAILED_DOWNLOAD_SETUP_FILE));
						//	pThis->MessageBox(theApp.GetString(IDS_STRING_FAILED_DOWNLOAD_SETUP_FILE), L"Vibranium", MB_OK | MB_ICONEXCLAMATION);

						if (pThis->m_csLangType == L"ENGLISH")
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_FAILED_DOWNLOAD_SETUP_FILE));
						}
						else
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_FAILED_DOWNLOAD_SETUP_FILE_G));
						}
						//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
						pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
					}
					else
					{
						pThis->MessageBox(pThis->m_csFileTargetPath, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
						pThis->m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_LAUNCH_IN_PROGRESS));
						ShellExecute(NULL, L"open", pThis->m_csFileTargetPath, NULL, NULL, SW_SHOW);

						pThis->m_stActualStatus.SetWindowTextW(L"  : " + theApp.GetString(IDS_STRING_SETUP_LAUNCH));
						pThis->CancelUI();
					}
				}
				else
				{
					if (!bFailed)
					{
						pThis->m_bIsCloseCalled = true;
						pThis->m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_SETUP_DOWNLOADED_SUCESSFULLY));
						//pThis->MessageBox(theApp.GetString(IDS_STRING_SETUP_DOWNLOADED_SUCESSFULLY), L"Vibranium", MB_OK | MB_ICONINFORMATION);
						if (pThis->m_csLangType == L"ENGLISH")
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_SETUP_DOWNLOADED_SUCESSFULLY));
						}
						else
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_SETUP_DOWNLOADED_SUCESSFULLY_G));
						}
						///pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
						pThis->m_svFunShowNotificationMsgCB.call(4, (SCITER_STRING)csMsgBox);
						// Issue no: 0001490, After completion of download, setup is not launching automatically.
						ShellExecute(NULL, L"open", pThis->m_csFileTargetPath, NULL, NULL, SW_SHOW);
						pThis->CancelUI();
					}
					else
					{
						pThis->m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_SETUP_DOWNLOAD_FAILED));
						//pThis->MessageBox(theApp.GetString(IDS_STRING_SETUP_DOWNLOAD_FAILED), L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
						if (pThis->m_csLangType == L"ENGLISH")
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_SETUP_DOWNLOAD_FAILED));
						}
						else
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_SETUP_DOWNLOAD_FAILED_G));
						}
						//pThis->MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
						pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
					}
				}

			}
			bIsBrowsePathCanceled = false;
			pThis->m_bchkLaunchExe = true;

		}
		else
		{
			pThis->m_iCurrentDownloadedByte = pThis->m_dwTotalFileSize = 0;
			pThis->KillTimer(TIMER_SETPERCENTAGE);
			//pThis->KillTimer(TIMER_FORADVERTISEMENT);
			pThis->m_stActualTransferRate.SetWindowTextW(L"  :  0.00 kbps");
			if (pThis->m_bRequiredSpaceNotavailable == true)
			{
				pThis->m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_STATUS_NO_ENOUGH_SPACE));
			}
			else
			{
				pThis->m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_STATUS_FAIL_TRY_AGAIN));
			}
			pThis->m_bIsDownloadingInProgress = false;
			pThis->RestoreWndFromTray(pThis->m_hWnd);
			pThis->m_objWinHttpManager.StopCurrentDownload();
			pThis->m_objWinHttpManager.SetDownloadCompletedBytes(0);
			pThis->m_objWinHttpManager.CloseFileHandles();
			pThis->m_objWinHttpManager.m_bIsConnected = false;


			if (pThis->m_hFile != NULL)
			{
				CloseHandle(pThis->m_hFile);
				pThis->m_hFile = NULL;
			}
			pThis->m_bchkLaunchExe = true;
			CWrdWizCustomMsgBox objCustomMsgBox(pThis);
			CString csMsg;

			if (objCustomMsgBox.m_hWnd == NULL)
			{
				if (pThis->m_bRequiredSpaceNotavailable == true)
				{
					pThis->m_bIsCloseCalled = true;
					//objCustomMsgBox.m_csFailedMsgText.Format(L"%s\n\t\t\t\t%s\n%s", theApp.GetString(IDS_STRING_MSG_NO_ENOUGH_SPACE), theApp.GetString(IDS_STRING_OR), theApp.GetString(IDS_STRING_DWONLOAD_MANUAL));
					if (pThis->m_csLangType == L"ENGLISH")
					{
						csMsg.Format(L"%s", theApp.GetString(IDS_STRING_MSG_NO_ENOUGH_SPACE));
					}
					else
					{
						csMsg.Format(L"%s", theApp.GetString(IDS_STRING_MSG_NO_ENOUGH_SPACE_G));
					}
					pThis->m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsg);
				}
				else
				{
					pThis->m_bIsCloseCalled = true;
					//objCustomMsgBox.m_csFailedMsgText.Format(L"%s\n\t\t\t%s\n%s\n", theApp.GetString(IDS_STRING_STATUS_FAIL_TRY_AGAIN), theApp.GetString(IDS_STRING_OR), theApp.GetString(IDS_STRING_DWONLOAD_MANUAL));
					//csMsg.Format(L"%s\n\t\t\t\t%s\n%s\n\n<a href=\"%s\">%s</a>", theApp.GetString(IDS_STRING_STATUS_FAIL_TRY_AGAIN), theApp.GetString(IDS_STRING_OR), theApp.GetString(IDS_STRING_DWONLOAD_MANUAL), theApp.m_csurl, theApp.m_csurl);
					if (pThis->m_csLangType == L"ENGLISH")
					{
						csMsg.Format(L"%s", theApp.GetString(IDS_STRING_STATUS_FAIL_TRY_AGAIN));
					}
					else
					{
						csMsg.Format(L"%s", theApp.GetString(IDS_STRING_STATUS_FAIL_TRY_AGAIN_G));
					}
					pThis->m_svFunShowNotificationMsgCB.call(3, (SCITER_STRING)csMsg, (SCITER_STRING)theApp.m_csurl);
				}
				//objCustomMsgBox.m_dwMsgErrorNo = 0x01;
				//objCustomMsgBox.DoModal();
			}


			// functionality of manually download add if download by download manager failed.
			// added by lalit kumawat 9-3-2015
			//	pThis->MessageBox(L"Failed to download the WardWiz setup", L"Vibranium", MB_ICONEXCLAMATION | MB_OK);
			AddLogEntry(L"### Failed to download Exe file %s", theApp.m_csurl, 0, true, SECONDLEVEL);

			pThis->CancelUI();
			return 0x01;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::StartWardWizSetupDownloadProcessThread", 0, 0, true, SECONDLEVEL);
	}
	return 0x00;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonPause
*  Description    : It is paused the process.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedButtonPause()
{
	try
	{
		if (m_objWinHttpManager.m_bIsConnected == true)
		{
			m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_DOWNLOAD_PAUSED));
		}

		//KillTimer(TIMER_FORADVERTISEMENT);
		m_bIsProcessPaused = false;
		PauseDownload();
	}

	catch (...)
	{
		AddLogEntry(L"### CWrdWizDownloaderDlg::OnBnClickedButtonPause::Exception", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonResume
*  Description    : It resumes the process.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedButtonResume()
{
	try
	{
		CString csMsgBox;
		if (!m_hStartWardWizSetupDwnldProc)
		{
			AddLogEntry(L"### Wardwiz download setup thread is not running", 0, 0, true, ZEROLEVEL);
			return;
		}

		SetTimer(TIMER_SETPERCENTAGE, 1000, NULL);

		if (m_objWinHttpManager.m_bIsConnected == true)
		{
			m_stActualStatus.SetWindowTextW(theApp.GetString(IDS_STRING_DOWNLOAD_IN_PROGRESS));


			if (!PathFileExists(m_csTempTargetFilePath))
			{
				AddLogEntry(L"### %s path is not exist", m_csTempTargetFilePath, 0, true, FIRSTLEVEL);
				if (m_csLangType == L"ENGLISH")
				{
					csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_NO_TARGETPATH_RESTART_DOWNLOAD));
				}
				else
				{
					csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_NO_TARGETPATH_RESTART_DOWNLOAD_G));
				}
				sciter::value v_svVarNoTargetPathCB = m_svFunShowNotificationMsgCB.call(2, (SCITER_STRING)csMsgBox);

				if (v_svVarNoTargetPathCB == 0)
				{
					if (m_hStartWardWizSetupDwnldProc)
					{
						SuspendThread(m_hStartWardWizSetupDwnldProc);

						if (TerminateThread(m_hStartWardWizSetupDwnldProc, 0x00))
						{
							AddLogEntry(L">>> Download wardwiz setup thread stopped successfully.", 0, 0, true, ZEROLEVEL);
							m_hStartWardWizSetupDwnldProc = NULL;
						}
					}
					m_objWinHttpManager.StopCurrentDownload();
					m_objWinHttpManager.SetDownloadCompletedBytes(0);
					m_objWinHttpManager.CloseFileHandles();
					if (m_hFile != NULL)
					{
						CloseHandle(m_hFile);
						m_hFile = NULL;
					}

					KillTimer(TIMER_SETPERCENTAGE);
					//KillTimer(TIMER_FORADVERTISEMENT);
					OnBnClickedButtonRun();
					return;
				}
				else
				{
					if (m_hStartWardWizSetupDwnldProc)
					{
						SuspendThread(m_hStartWardWizSetupDwnldProc);
						if (TerminateThread(m_hStartWardWizSetupDwnldProc, 0x00))
						{
							AddLogEntry(L">>> Download wardwiz setup thread stopped successfully.", 0, 0, true, ZEROLEVEL);
							m_hStartWardWizSetupDwnldProc = NULL;
						}
					}
					m_objWinHttpManager.StopCurrentDownload();
					m_objWinHttpManager.SetDownloadCompletedBytes(0);
					m_objWinHttpManager.CloseFileHandles();
					if (m_hFile != NULL)
					{
						CloseHandle(m_hFile);
						m_hFile = NULL;
					}

					KillTimer(TIMER_SETPERCENTAGE);
					//KillTimer(TIMER_FORADVERTISEMENT);
				}
			}
		}
		m_bIsProcessPaused = false;
		ResumeDownload();
	}
	catch (...)
	{
		AddLogEntry(L"### CWrdWizDownloaderDlg::OnBnClickedButtonResume::Exception", 0, 0, true, SECONDLEVEL);
	}

	return;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonClose
*  Description    : It will restore the setup file path, size of last setup file and last target path
into INI. Also in "Run" registry.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedButtonClose()
{
	try
	{
		if (m_bIsDownloadingInProgress)
		{
			m_bIsProcessPaused = true;
			PauseDownload();
			CString csMsgBox;
			if (m_csLangType == L"ENGLISH")
			{
				csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_MSG_DOWNLOAD_IN_PROGRESS), theApp.GetString(IDS_STRING_WANT_TO_DOWNLOAD));
			}
			else
			{
				csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_MSG_DOWNLOAD_IN_PROGRESS_G), theApp.GetString(IDS_STRING_WANT_TO_DOWNLOAD_G));
			}
			//m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);

			if (m_hStartWardWizSetupDwnldProc)
			{
				SuspendThread(m_hStartWardWizSetupDwnldProc);

				if (TerminateThread(m_hStartWardWizSetupDwnldProc, 0x00))
				{
					AddLogEntry(L">>> Download wardwiz setup thread stopped successfully.", 0, 0, true, ZEROLEVEL);
					m_hStartWardWizSetupDwnldProc = NULL;
				}
			}
			m_objWinHttpManager.StopCurrentDownload();
			m_objWinHttpManager.SetDownloadCompletedBytes(0);
			m_objWinHttpManager.CloseFileHandles();

			if (m_hFile != NULL)
			{
				CloseHandle(m_hFile);
				m_hFile = NULL;
			}

			if ((DWORD)m_iCurrentDownloadedByte < m_dwTotalFileSize)
			{

#ifdef WRDWIZBASIC
#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGB.INI");
#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIB.INI");
#endif
#elif WRDWIZBASICPATCH
#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGBPATCH.INI");
#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIBPATCH.INI");
#endif
#elif WRDWIZESSNL
#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEss.INI");
#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEss.INI");
#endif
#elif WRDWIZESSNLPATCH
#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPatch.INI");
#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPatch.INI");
#endif
#elif  WRDWIZESSPLUS
#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlus.INI");
#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlus.INI");
#endif
#elif  WRDWIZESSPLUSPATCH
#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlusPatch.INI");
#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlusPatch.INI");
#endif
#else
#ifdef RELEASENCG
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGP.INI");
#elif RELEASENCI
				_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIP.INI");
#endif
#endif


				if ((m_bIsWow64 == true) && (m_bchkDownloadoption == false))
				{
					WritePrivateProfileString(L"Last Setup", L"1", L"64", m_szIniFilePath);
				}
				if ((m_bIsWow64 == true) && (m_bchkDownloadoption == true))
				{
					WritePrivateProfileString(L"Last Setup", L"1", L"32", m_szIniFilePath);
				}
				if ((m_bIsWow64 == false) && (m_bchkDownloadoption == true))
				{
					WritePrivateProfileString(L"Last Setup", L"1", L"64", m_szIniFilePath);
				}
				if ((m_bIsWow64 == false) && (m_bchkDownloadoption == false))
				{
					WritePrivateProfileString(L"Last Setup", L"1", L"32", m_szIniFilePath);
				}

				CString csBrowseFolderName;
				m_edtBrowsedSetupPath.GetWindowTextW(csBrowseFolderName);
				WritePrivateProfileString(L"Last Path ", L"1", csBrowseFolderName, m_szIniFilePath);
			}

			KillTimer(TIMER_SETPERCENTAGE);
			//KillTimer(TIMER_FORADVERTISEMENT);
			//MessageBox(theApp.GetString(IDS_STRING_DOWNLOAD_ABORTED), L"Vibranium", MB_OK|MB_ICONEXCLAMATION);
			if (m_csLangType == L"ENGLISH")
			{
				csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_DOWNLOAD_ABORTED));
			}
			else
			{
				csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_DOWNLOAD_ABORTED_G));
			}
			//MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
			m_bIsCloseCalled = true;
			m_bIsDownloadingInProgress = true;
			m_svFunShowNotificationMsgCB.call(1, (SCITER_STRING)csMsgBox);
			m_bIsCloseCalled = false;
			m_bIsProcessPaused = true;
			OnCancel();
		}
		else
		{
			m_objWinHttpManager.StopCurrentDownload();
			m_objWinHttpManager.SetDownloadCompletedBytes(0);
			m_objWinHttpManager.CloseFileHandles();
			OnCancel();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CWrdWizDownloaderDlg::OnBnClickedButtonClose::Exception", 0, 0, true, SECONDLEVEL);
	}

	return;

}

/***************************************************************************************************
*  Function Name  : OnTimer
*  Description    : The framework calls this member function after each interval specified in the SetTimer member function used to install a timer.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	try
	{
		CString csPercetage(L"");
		if (nIDEvent == TIMER_SETPERCENTAGE)
		{
			int iCurrentDownloadBytes = m_objWinHttpManager.GetDownloadCompletedBytes();

			int icompletedDownloadBytes = m_objWinHttpManager.m_dwCompletedDownloadBytes;

			m_iCurrentDownloadedByte = 0;

			m_iCurrentDownloadedByte = iCurrentDownloadBytes + icompletedDownloadBytes;

			m_dwPercentage = GetPercentage(m_iCurrentDownloadedByte, m_dwTotalFileSize);

			// issue :in case of re- download progress bar once showing 100 percent then showing actual status.
			// resolved by lalit kumawat 9-8-2015

			if (m_iCurrentDownloadedByte <= 0)
				return;

			if (m_dwTotalFileSize <= 0)
				return;

			if (m_dwPercentage > 100)
			{
				return;
			}

			CString csStr;
			if (m_dwTotalFileSize >= 1000000)
			{
				if (m_iCurrentDownloadedByte >= 1000000)
				{
					csStr.Format(L"%d %s (%d MB %s %d MB)", m_dwPercentage, L"%", static_cast<DWORD>(m_iCurrentDownloadedByte / (1000 * 1024)), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(m_dwTotalFileSize / (1000 * 1024)));
				}
				else
				{
					csStr.Format(L"%d %s (%d KB %s %d MB)", m_dwPercentage, L"%", static_cast<DWORD>(m_iCurrentDownloadedByte / (1024)), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(m_dwTotalFileSize / (1000 * 1024)));
				}
			}
			else
			{
				csStr.Format(L"%d %s (%d KB %s %d KB)", m_dwPercentage, L"%", static_cast<DWORD>(m_iCurrentDownloadedByte / 1024), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(m_dwTotalFileSize / 1024));
			}

			m_prgDownloadSetupStatus.SetWindowTextW(csStr);
			m_prgDownloadSetupStatus.RedrawWindow();
			m_prgDownloadSetupStatus.SetPos(m_dwPercentage);

			//Calculate the transfer rate
			CTimeSpan tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsStartTime);
			CString Ctime = tsScanElapsedTime.Format(_T("Elapsed Time:%H:%M:%S"));
			long lSeconds = tsScanElapsedTime.GetSeconds();
			long lMinutes = tsScanElapsedTime.GetMinutes();
			long lHours = tsScanElapsedTime.GetHours();

			long lMinTOSec = lMinutes * 60;
			long lHrsToSec = lHours * 3600;
			long TotalSec = lHrsToSec + lMinTOSec + lSeconds;

			if (TotalSec == 0)
			{
				TotalSec = 1;
			}

			long lSpeed = m_iCurrentDownloadedByte / TotalSec;
			double lSpeed_Kbps = (((double)lSpeed) * 0.0078125);

			CString cSpeed;
			if (lSpeed_Kbps > 1000.0)
			{
				cSpeed.Format(_T("    %0.2f Mbps"), (lSpeed_Kbps/1000));
			}
			else
			{
				cSpeed.Format(_T("    %0.2f Kbps"), lSpeed_Kbps);
			}
			m_stActualTransferRate.SetWindowText(cSpeed);
			m_stActualTransferRate.RedrawWindow();

			if (lSpeed == 0)
			{
				lSpeed = 1;
			}

			/*Issue No:17, 48 Issue Desc: 17.Remaining Time in updates has minutes in 3 digits.
			48. In updates, the remaining time is coming wrong
			Resolved by :	Divya S..*/

			long lSecToHrs, lSecToMin, lSecToSec;
			long lRemainingTime = (m_dwTotalFileSize - m_iCurrentDownloadedByte) / lSpeed;
			if (m_iCurrentDownloadedByte != 0)
			{
				lSecToHrs = lRemainingTime / 3600;
				lSecToMin = lRemainingTime / 60;
				lSecToSec = (lRemainingTime - (lSecToMin * 60));
			}
			else
			{
				lSecToHrs = 0;
				lSecToMin = 0;
				lSecToSec = 0;
			}
			CString cRemaining;
			if (lSecToHrs > 0)
			{
				cRemaining.Format(_T("    %02ld Hrs  %02ld Mins  %02ld Secs"), lSecToHrs, lSecToMin, lSecToSec);
			}
			else
			{
				cRemaining.Format(_T("    %02ld Mins  %02ld Secs"), lSecToMin, lSecToSec);
			}
			m_stActualRemainingTime.SetWindowText(cRemaining);
			m_stActualRemainingTime.RedrawWindow();
			if (m_csLangType == L"ENGLISH")
			{
				csPercetage.Format(L"WardWiz (%d%s) %s", m_dwPercentage, L"%", theApp.GetString(IDS_STRING_TRAYTXT_DOWNLOADED));
			}
			else
			{
				csPercetage.Format(L"WardWiz (%d%s) %s", m_dwPercentage, L"%", theApp.GetString(IDS_STRING_TRAYTXT_DOWNLOADED_G));
			}
			ShowNotifyIcon(m_hWnd, 0x02, csPercetage);
			CString csPercent = L"";

			//Issue no 0001482 : While downloading setup when progress bar is in 99% popup appearing as 'setup file downloaded successfully'
			if (m_dwPercentage >= 99)
			{
				m_dwPercentage = 100;
			}
			csPercent.Format(L"%d", m_dwPercentage);


			m_svFunUpdateDownloadStatusCB.call((SCITER_STRING)cRemaining, (SCITER_STRING)cSpeed, (SCITER_STRING)csPercent);
		}

		CJpegDialog::OnTimer(nIDEvent);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnTimer", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : GetPercentage()
*  Description    : Calculating percentage
*  Author Name    : neha gharge
*  SR_NO		  :
*  Date			  :	7 Sept ,2015
****************************************************************************************************/
DWORD CWrdWizDownloaderDlg::GetPercentage(int iDownloaded, int iTotalSize)
{
	__try
	{
		return static_cast<DWORD>(((static_cast<double>((iDownloaded)) / iTotalSize)) * 100);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardWizALUSrv::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : IsWow64()
*  Description    : It will check client machine is 64 bit or 32bit.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date			  :	20th Aug,2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::IsWow64()
{
	TCHAR				szOSVer[16] = { 0 };
	SYSTEM_INFO			sysInfo = { 0 };
	__try
	{

		SYSTEM_INFO			sysInfo = { 0 };
		GetNativeSystemInfo(&sysInfo);

		if ((sysInfo.wProcessorArchitecture&PROCESSOR_ARCHITECTURE_AMD64) == PROCESSOR_ARCHITECTURE_AMD64)
			m_bIsWow64 = true;
		else
			m_bIsWow64 = false;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::IsWow64", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnClose
*  Description    : On system close.
*  Author Name    : Jeena Mariam Saji
*  SR_NO
*  Date           : 13 June 2016
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnClose()
{
	try
	{
		m_bIsProcessPaused = true;
		//Issue No.0001554 UI color is getting changed on clicking on ALT+F4 or close window from task bar.
		if (m_bIsCloseCalled == false && m_bIsProcessPaused == true)
		{
			m_bIsCloseCalled = true;
			m_bIsProcessPaused = false;
			m_svFunCallOnCloseUICB.call();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnClose", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCheckLaunch
*  Description    : Launch the setup file check box
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedCheckLaunch()
{
	__try
	{
		if (m_chkLaunchDownloadedExe.GetCheck() == BST_CHECKED)
		{
			m_bchkLaunchExe = true;
		}
		else
		{
			m_bchkLaunchExe = false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnBnClickedCheckLaunch", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCheckOpenfolder
*  Description    : Checkbox for opening folder where setup file is downloaded.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedCheckOpenfolder()
{
	__try
	{
		if (m_chkOpenFolderOption.GetCheck() == BST_CHECKED)
		{
			m_bchkOpenFolder = true;
		}
		else
		{
			m_bchkOpenFolder = false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnBnClickedCheckOpenfolder", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnCtlColor
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
HBRUSH CWrdWizDownloaderDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	try
	{
		HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		int	ctrlID;
		ctrlID = pWnd->GetDlgCtrlID();
		if (ctrlID == IDC_CHECK_LAUNCH ||
			ctrlID == IDC_CHECK_OPENFOLDER ||
			ctrlID == IDC_STATIC_TEXT_TO_SHOW_PATH ||
			ctrlID == IDC_STATIC_DWNLD_INFO ||
			ctrlID == IDC_STATIC_PROD_NAME ||
			ctrlID == IDC_STATIC_ACTUAL_PROD_NAME ||
			ctrlID == IDC_STATIC_DWNLD_PATH ||
			//ctrlID == IDC_STATIC_ACTUAL_DOWNLD_PATH ||
			ctrlID == IDC_STATIC_TRANSFER_RATE ||
			//ctrlID == IDC_STATIC_ACTUAL_TRANSFER_RATE ||
			ctrlID == IDC_STATIC_REMAINING_TIME ||
			//ctrlID == IDC_STATIC_ACTUAL_REMAINING_TIME ||
			ctrlID == IDC_STATIC_STATUS ||
			ctrlID == IDC_STATIC_ACTUAL_STATUS ||
			ctrlID == IDC_CHECK_DWNLD_OPTION ||
			ctrlID == IDC_BUTTON_CLOSE ||
			ctrlID == IDC_BUTTON_RUN ||
			ctrlID == IDC_BUTTON_PAUSE ||
			ctrlID == IDC_BUTTON_RESUME ||
			ctrlID == IDC_BUTTON_DOWNLOAD_BROWSE ||
			ctrlID == IDC_STATIC_BROSWER	||
			ctrlID == IDC_STATIC_FOOTER_MSG ||
			ctrlID == IDC_STATIC_DOWNLOADER_NAME ||
			ctrlID == IDC_STATIC_LAUNCH_TEXT ||
			ctrlID == IDC_STATIC_OPENFOLDER_TEXT
			)
		{
			pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
		}	return hbr;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnCtlColor", 0, 0, true, SECONDLEVEL);
	}
	return hbr;
}

/**********************************************************************************************************
*  Function Name  :	SetStartupEntry
*  Description    :	Set a downloader entry into registry.
*  Author Name    : neha Gharge
*  SR_NO		  :
*  Date           : 24th Aug,2015
**********************************************************************************************************/
void CWrdWizDownloaderDlg::SetStartupEntry(TCHAR* szDownloaderPath)
{
	__try
	{
		HKEY hKey;
		//Xp dont have HKCR run entry so when it is not in HKCR it will create in HKLM
		//LPCTSTR sk = TEXT("SOFTWARE\\Vibranium");
		if (szDownloaderPath == NULL)
			return;

		LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);

		if (openRes != ERROR_SUCCESS)
		{
			openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
			if (openRes != ERROR_SUCCESS)
			{
				AddLogEntry(L"### Error in Open run key.", 0, 0, true, SECONDLEVEL);
				RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
				if (!hKey)
				{
					AddLogEntry(L"### Error in creating run key.", 0, 0, true, SECONDLEVEL);
				}
			}
		}

		LPCTSTR SetPath = TEXT("WWizDownloader");
		DWORD dwDataLen = 512;
		TCHAR szDownloadpath[512] = { 0 };
		_tcscpy_s(szDownloadpath, _countof(szDownloadpath), szDownloaderPath);

#ifdef WRDWIZBASIC
#ifdef RELEASENCG
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCBG.EXE");
#elif RELEASENCI
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCBI.exe");
#endif
#elif WRDWIZBASICPATCH
#ifdef RELEASENCG
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WWPATCHDMNCBG.EXE");
#elif RELEASENCI
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WWPATCHDMNCBI.exe");
#endif
#elif WRDWIZESSNL
#ifdef RELEASENCG
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCEG.exe");
#elif RELEASENCI
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCEI.exe");
#endif
#elif WRDWIZESSNLPATCH
#ifdef RELEASENCG
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WWPATCHDMNCEG.exe");
#elif RELEASENCI
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WWPATCHDMNCEI.exe");
#endif
#elif WRDWIZESSPLUS
#ifdef RELEASENCG
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCEPG.exe");
#elif RELEASENCI
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCEPI.exe");
#endif
#elif WRDWIZESSPLUSPATCH
#ifdef RELEASENCG
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WWPATCHDMNCEPG.exe");
#elif RELEASENCI
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WWPATCHDMNCEPI.exe");
#endif
#else	
#ifdef RELEASENCG
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCPG.exe");
#elif RELEASENCI
		_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCPI.exe");
#endif
#endif

		//Issue No. 2432 resolved	
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		//LONG setRes = RegSetValueEx(hKey, SetPath, 0, REG_SZ, (LPBYTE)szDownloadpath, dwDataLen);
		LONG setRes = RegSetValueEx(hKey, SetPath, 0, REG_SZ, (LPBYTE)szModulePath, dwDataLen);
		if (setRes != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to set entry into run.", 0, 0, true, SECONDLEVEL);
		}
		else
		{
			AddLogEntry(L"### Success to set entry into run.", 0, 0, true, SECONDLEVEL);
		}

		RegCloseKey(hKey);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::SetStartupEntry", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetStartupEntry
*  Description    : Get a downloader entry into registry.
*  Author Name    : neha Gharge
*  SR_NO		  :
*  Date           : 24th Aug,2015
**********************************************************************************************************/
bool CWrdWizDownloaderDlg::GetStartupEntry()
{
	__try
	{
		HKEY hKey;
		//Xp dont have HKCR run entry so when it is not in HKCR it will create in HKLM
		LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);

		if (openRes != ERROR_SUCCESS)
		{
			openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
			if (openRes != ERROR_SUCCESS)
			{
				AddLogEntry(L"### Error in opening run key.", 0, 0, true, FIRSTLEVEL);
			}
		}

		LPCTSTR SetPath = TEXT("WWizDownloader");
		DWORD dwDataLen = 512;
		TCHAR szDownloadpath[512] = { 0 };
		DWORD dwType = REG_SZ;

		LONG getRes = RegQueryValueEx(hKey, SetPath, 0, &dwType, (LPBYTE)szDownloadpath, &dwDataLen);
		if (getRes != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to Get entry into run.", 0, 0, true, FIRSTLEVEL);
		}

		if (szDownloadpath[0])
		{
			return true;
		}
		else
		{
			return false;
		}

		RegCloseKey(hKey);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::GetStartupEntry", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCheckDwnldOption
*  Description    : this checkbox is to check if machine is 64 bit and user wants to download 32bit or vice versa
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedCheckDwnldOption()
{
	__try
	{
		if (m_chkDownloadOption.GetCheck() == BST_CHECKED)
		{
			m_bchkDownloadoption = true;
		}
		else
		{
			m_bchkDownloadoption = false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnBnClickedCheckDwnldOption", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : MinimizeWndToTray
*  Description    : Minimize Window into tray.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
VOID CWrdWizDownloaderDlg::MinimizeWndToTray()
{
	__try
	{
		if (GetDoAnimateMinimize())
		{
			RECT rcFrom, rcTo;

			// Get the rect of the window. It is safe to use the rect of the whole
			// window - DrawAnimatedRects will only draw the caption
			GetWindowRect(&rcFrom);
			GetTrayWndRect(&rcTo);

			// Get the system to draw our animation for us
			DrawAnimatedRects(IDANI_CAPTION, &rcFrom, &rcTo);
		}

		// Add the tray icon. If we add it before the call to DrawAnimatedRects,
		// the taskbar gets erased, but doesn't get redrawn until DAR finishes.
		// This looks untidy, so call the functions in this order

		// Hide the window
		//this->ShowWindow(SW_HIDE);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::MinimizeWndToTray", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetDoAnimateMinimize
*  Description    : Check to see if the animation has been disabled
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
BOOL CWrdWizDownloaderDlg::GetDoAnimateMinimize(VOID)
{
	ANIMATIONINFO ai;
	__try
	{
		ai.cbSize = sizeof(ai);
		SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::GetDoAnimateMinimize", 0, 0, true, SECONDLEVEL);
	}
	return ai.iMinAnimate ? TRUE : FALSE;
}

// Returns the rect of where we think the syCWrdWizDownloaderDlg::MinimizeWndToTraystem tray is. This will work for
// all current versions of the shell. If explorer isn't running, we try our
// best to work with a 3rd party shell. If we still can't find anything, we
// return a rect in the lower right hand corner of the screen
/***************************************************************************************************
*  Function Name  : GetTrayWndRect
*  Description    : It gives tray window rectangle co-ordinates.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
VOID CWrdWizDownloaderDlg::GetTrayWndRect(LPRECT lpTrayRect)
{
	// First, we'll use a quick hack method. We know that the taskbar is a window
	// of class Shell_TrayWnd, and the status tray is a child of this of class
	// TrayNotifyWnd. This provides us a window rect to minimize to. Note, however,
	// that this is not guaranteed to work on future versions of the shell. If we
	// use this method, make sure we have a backup!
	__try
	{
		HWND hShellTrayWnd = ::FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);
		if (hShellTrayWnd)
		{
			HWND hTrayNotifyWnd = ::FindWindowEx(hShellTrayWnd, NULL, TEXT("TrayNotifyWnd"), NULL);
			if (hTrayNotifyWnd)
			{
				::GetWindowRect(hTrayNotifyWnd, lpTrayRect);
				return;
			}
		}

		// OK, we failed to get the rect from the quick hack. Either explorer isn't
		// running or it's a new version of the shell with the window class names
		// changed (how dare Microsoft change these undocumented class names!) So, we
		// try to find out what side of the screen the taskbar is connected to. We
		// know that the system tray is either on the right or the bottom of the
		// taskbar, so we can make a good guess at where to minimize to
		APPBARDATA appBarData;
		appBarData.cbSize = sizeof(appBarData);
		if (SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData))
		{
			// We know the edge the taskbar is connected to, so guess the rect of the
			// system tray. Use various fudge factor to make it look good
			switch (appBarData.uEdge)
			{
			case ABE_LEFT:
			case ABE_RIGHT:
				// We want to minimize to the bottom of the taskbar
				lpTrayRect->top = appBarData.rc.bottom - 100;
				lpTrayRect->bottom = appBarData.rc.bottom - 16;
				lpTrayRect->left = appBarData.rc.left;
				lpTrayRect->right = appBarData.rc.right;
				break;

			case ABE_TOP:
			case ABE_BOTTOM:
				// We want to minimize to the right of the taskbar
				lpTrayRect->top = appBarData.rc.top;
				lpTrayRect->bottom = appBarData.rc.bottom;
				lpTrayRect->left = appBarData.rc.right - 100;
				lpTrayRect->right = appBarData.rc.right - 16;
				break;
			}

			return;
		}

		// Blimey, we really aren't in luck. It's possible that a third party shell
		// is running instead of explorer. This shell might provide support for the
		// system tray, by providing a Shell_TrayWnd window (which receives the
		// messages for the icons) So, look for a Shell_TrayWnd window and work out
		// the rect from that. Remember that explorer's taskbar is the Shell_TrayWnd,
		// and stretches either the width or the height of the screen. We can't rely
		// on the 3rd party shell's Shell_TrayWnd doing the same, in fact, we can't
		// rely on it being any size. The best we can do is just blindly use the
		// window rect, perhaps limiting the width and height to, say 150 square.
		// Note that if the 3rd party shell supports the same configuraion as
		// explorer (the icons hosted in NotifyTrayWnd, which is a child window of
		// Shell_TrayWnd), we would already have caught it above
		hShellTrayWnd = ::FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);
		if (hShellTrayWnd)
		{
			::GetWindowRect(hShellTrayWnd, lpTrayRect);
			if (lpTrayRect->right - lpTrayRect->left > DEFAULT_RECT_WIDTH)
				lpTrayRect->left = lpTrayRect->right - DEFAULT_RECT_WIDTH;
			if (lpTrayRect->bottom - lpTrayRect->top > DEFAULT_RECT_HEIGHT)
				lpTrayRect->top = lpTrayRect->bottom - DEFAULT_RECT_HEIGHT;

			return;
		}

		// OK. Haven't found a thing. Provide a default rect based on the current work
		// area
		SystemParametersInfo(SPI_GETWORKAREA, 0, lpTrayRect, 0);
		lpTrayRect->left = lpTrayRect->right - DEFAULT_RECT_WIDTH;
		lpTrayRect->top = lpTrayRect->bottom - DEFAULT_RECT_HEIGHT;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::GetTrayWndRect", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ShowNotifyIcon
*  Description    : Notifies the notification area, icon to tray
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
VOID CWrdWizDownloaderDlg::ShowNotifyIcon(HWND hWnd, DWORD dwAdd, CString csMessage)
{
	try
	{
		NOTIFYICONDATA nid;
		ZeroMemory(&nid, sizeof(nid));
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_TRAYMESSAGE;
		nid.hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
		lstrcpy(nid.szTip, csMessage);

		if (dwAdd == 0x00)
			Shell_NotifyIcon(NIM_ADD, &nid);
		else if (dwAdd == 0x01)
			Shell_NotifyIcon(NIM_DELETE, &nid);
		else if (dwAdd == 0x02)
			Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::ShowNotifyIcon", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : RestoreWndFromTray
*  Description    : It restores window from tray
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
VOID CWrdWizDownloaderDlg::RestoreWndFromTray(HWND hWnd)
{
	__try
	{
		if (GetDoAnimateMinimize())
		{
			// Get the rect of the tray and the window. Note that the window rect
			// is still valid even though the window is hidden
			RECT rcFrom, rcTo;
			GetTrayWndRect(&rcFrom);
			::GetWindowRect(hWnd, &rcTo);

			// Get the system to draw our animation for us
			DrawAnimatedRects(IDANI_CAPTION, &rcFrom, &rcTo);
		}

		// Show the window, and make sure we're the foreground window
		::ShowWindow(hWnd, SW_SHOW);
		::SetActiveWindow(hWnd);
		::SetForegroundWindow(hWnd);

		// Remove the tray icon. As described above, remove the icon after the
		// call to DrawAnimatedRects, or the taskbar will not refresh itself
		// properly until DAR finished
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::RestoreWndFromTray", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : An application-defined function that processes messages sent to a window
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 7 sep 2015
****************************************************************************************************/
LRESULT CWrdWizDownloaderDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		LRESULT lResult;
		BOOL    bHandled = FALSE;

		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
		if (bHandled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.

		if (LOWORD(lParam) == WM_LBUTTONUP)
		{
			RestoreWndFromTray(m_hWnd);
			ShowNotifyIcon(m_hWnd, 0x01, L"WardWiz Downloader");
			return 0;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CJpegDialog::WindowProc(message, wParam, lParam);
}

/***********************************************************************************************
Function Name  : SaveAsDoublicateFile
Description    : it provides functionality to save As option if same name file already exists.
SR.NO		   :
Author Name    : Neha Gharge
Date           : 10-sept- 2015
***********************************************************************************************/
CString  CWrdWizDownloaderDlg::SaveAsDoublicateFile(CString csfilePath)
{

	DWORD dwFDialogOutput = 0;
	CString csNewFilePath = L"";
	CString csExtension = L"";
	CString csUserAddExtension = L"";
	CString csFileName = L"";
	try
	{

		csFileName = csfilePath.Mid(csfilePath.ReverseFind('\\') + 1);
		int iPos = csFileName.ReverseFind('.');

		if (iPos != -1)
		{
			csExtension = csfilePath.Mid(csfilePath.ReverseFind('.') + 1);
			csExtension = L"*." + csExtension;

			//00001228 Neha Gharge
			//Save As popup dialog comes as separate window.
			CFileDialog objFileDlg(FALSE, csExtension, csExtension,
				OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, csExtension,this->GetParentOwner());

			if (objFileDlg.DoModal() == IDOK)
			{
				csNewFilePath = objFileDlg.GetPathName();
			}
			else
			{
				csNewFilePath = L"";
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::SaveAsDoublicateFile", 0, 0, true, SECONDLEVEL);
	}
	return csNewFilePath;
}


void CWrdWizDownloaderDlg::OnURL_Navigate(LPCTSTR URL)
{
	try
	{	//if (UpdateData(TRUE))
		//{
		//m_simplebrowser.Navigate(URL);
		//}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnURL_Navigate", 0, 0, true, SECONDLEVEL);
	}
}

void CWrdWizDownloaderDlg::OnBeforeNavigate2(NMHDR *pNMHDR, LRESULT *pResult)
{
	try
	{
		SimpleBrowser::Notification
			*notification = (SimpleBrowser::Notification *)pNMHDR;

		CString string;

		string.Format(_T("OnBeforeNavigate2: \"%s\", \"%s\", [0x%08X,%d bytes], \"%s\"\r\n"),
			notification->URL,
			notification->frame,
			notification->post_data, notification->post_data_size,
			notification->headers);

		m_csNavigateUrl = notification->URL;

		*pResult = 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnURL_Navigate", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonWindowClose
*  Description    : Function to Close Window
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2016
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedButtonWindowClose()
{
	try
	{
		OnBnClickedButtonClose();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnBnClickedButtonWindowClose", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonWindowMinimize
*  Description    : Function to Minimize Window
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2016
****************************************************************************************************/
void CWrdWizDownloaderDlg::OnBnClickedButtonWindowMinimize()
{

	try
	{
		MinimizeWndToTray();
		if (m_csLangType == L"ENGLISH")
		{
			ShowNotifyIcon(m_hWnd, 0x00, L"WardWiz Downloader");
		}
		else
		{
			ShowNotifyIcon(m_hWnd, 0x00, L"WardWiz-Downloader");
		}
		//m_bMinimize = true;

		// Return TRUE to tell DefDlgProc that we handled the message, and set
		// DWL_MSGRESULT to 0 to say that we handled WM_SYSCOMMAND
		SetWindowLong(m_hWnd, 0, 0);
		this->ShowWindow(SW_HIDE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::OnBnClickedButtonWindowMinimize", 0, 0, true, SECONDLEVEL);
	}
}

void CWrdWizDownloaderDlg::CancelUI()
{
	try
	{
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::CancelUI", 0, 0, true, SECONDLEVEL);
	}
}

bool CWrdWizDownloaderDlg::PauseDownload()
{
	try
	{
		KillTimer(TIMER_SETPERCENTAGE);
		if (!m_hStartWardWizSetupDwnldProc)
		{
			AddLogEntry(L"### Wardwiz download setup thread is not running", 0, 0, true, ZEROLEVEL);
			return true;
		}
		if (SuspendThread(m_hStartWardWizSetupDwnldProc) != 0xFFFFFFFF)
		{
			AddLogEntry(L">>> Wardwiz download setup thread suspended successfully.", 0, 0, true, SECONDLEVEL);
			return true;
		}
		AddLogEntry(L"### Wardwiz download setup thread suspended failed.", 0, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::PauseDownload", 0, 0, true, SECONDLEVEL);
	}

	return false;
}

bool CWrdWizDownloaderDlg::ResumeDownload()
{
	try
	{
		if (!m_hStartWardWizSetupDwnldProc)
		{
			AddLogEntry(L"### Wardwiz download setup thread is not running", 0, 0, true, ZEROLEVEL);
			return true;
		}

		if (ResumeThread(m_hStartWardWizSetupDwnldProc) != 0xFFFFFFFF)
		{
			AddLogEntry(L">>> Wardwiz download setup thread resumed successfully.", 0, 0, true, ZEROLEVEL);
			return true;
		}
		AddLogEntry(L"### Wardwiz download setup thread resumed failed.", 0, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::ResumeDownload", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : GetEmptyDrivePath
Description    : it provides the other empty drive if current drive full.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
bool CWrdWizDownloaderDlg::IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize)
{
	bool bReturn = false;
	bool isbSpaceAvailable = false;
	try
	{
		DWORD64 TotalNumberOfFreeBytes;
		//csDrive.Format(L"%c:", szDrive);

		if (PathFileExists(csDrive))
		{
			if (!GetDiskFreeSpaceEx((LPCWSTR)csDrive, &m_uliFreeBytesAvailable, &m_uliTotalNumberOfBytes, &m_uliTotalNumberOfFreeBytes))
			{
				isbSpaceAvailable = false;
				bReturn = false;
				AddLogEntry(L"### Failed in  GetDiskFreeSpaceEx", 0, 0, true, SECONDLEVEL);
			}

			TotalNumberOfFreeBytes = m_uliTotalNumberOfFreeBytes.QuadPart;
			TCHAR szFilePath[255] = { 0 };
			DWORD64 dwfileSize = 0;
			dwSetupFileSize = (dwSetupFileSize * iSpaceRatio) / (1024 * 1024);
			TotalNumberOfFreeBytes = TotalNumberOfFreeBytes / (1024 * 1024);
			if (dwSetupFileSize < TotalNumberOfFreeBytes)
			{
				bReturn = true;
			}
			else
			{
				bReturn = false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::IsDriveHaveRequiredSpace", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    :
percentage.
Author Name    : Neha Gharge
SR_NO		   :
Date           : 20th Feb 2016
//Issue no 1321 :While downloading is in progress if we click on "Esc" key download manager UI getting closed and it is not giving any popup message.
****************************************************************************/
BOOL CWrdWizDownloaderDlg::PreTranslateMessage(MSG* pMsg)
{
	try
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		{
			return TRUE;
		}
		return CJpegDialog::PreTranslateMessage(pMsg);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::PreTranslateMessage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_BtnClickPause
*  Description    : Function to Pause Download
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2016
****************************************************************************************************/
json::value CWrdWizDownloaderDlg::On_BtnClickPause()
{
	try
	{
		OnBnClickedButtonPause();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::On_BtnClickPause", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_BtnClickResume
*  Description    : Function to Resume Download
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2016
****************************************************************************************************/
json::value CWrdWizDownloaderDlg::On_BtnClickResume()
{
	try
	{
		m_bIsCloseCalled = false;
		OnBnClickedButtonResume();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::On_BtnClickResume", 0, 0, true, SECONDLEVEL);
	}
	return 0;

}

/***************************************************************************************************
*  Function Name  : On_BtnClickMinimize
*  Description    : Function to Minimize Wardwiz Downloader
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2016
****************************************************************************************************/
json::value CWrdWizDownloaderDlg::On_BtnClickMinimize()
{
	try
	{
		OnBnClickedButtonWindowMinimize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::On_BtnClickMinimize", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_BtnClickClose
*  Description    : Function to Close Wardwiz Downloader
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2016
****************************************************************************************************/
json::value CWrdWizDownloaderDlg::On_BtnClickClose()
{
	try
	{
		OnBnClickedButtonClose();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWrdWizDownloaderDlg::On_BtnClickClose", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : Get_String
*  Description    : Function to send string from String Table to front end
*  Author Name    : Jeena Mariam Saji
*  Date			  : 08 Nov 2016
****************************************************************************************************/
json::value CWrdWizDownloaderDlg::Get_String(SCITER_VALUE svStringValue)
{
	UINT temp = (UINT)svStringValue.d;
	CString csMessage = theApp.GetString(temp);
	return (SCITER_STRING)csMessage;
}