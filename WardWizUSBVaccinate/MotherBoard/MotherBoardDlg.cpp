/**********************************************************************************************************
Program Name          : MotherBoardDlg.cpp
Description           : This class contains the functionality for USB Vaccination.
						It has 2 options : a) USB Vaccination  b) Un-hide File/Folder
Author Name			  : Yogeshwar Rasal
Date Of Creation      : 27th May 2016
Version No            : 2.0.0.17
***********************************************************************************************************/
#pragma once

#include "stdafx.h"
#include "MotherBoard.h"
#include "MotherBoardDlg.h"
#include "iTINRegWrapper.h"

char* g_strDatabaseFilePath = ".\\VBALLREPORTS.DB";

#include <stdio.h>
#include <intrin.h>
#include <cfgmgr32.h>
#include <newdev.h>
#include <setupapi.h>
#include <shlobj.h>

#include <Wbemidl.h>
#include <comutil.h>
#include <Lm.h>   
#include <iphlpapi.h>
//#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <Shlwapi.h>

#include <math.h>
#include <Sddl.h>
#include "CSecure64.h"
#include "DriverConstants.h"
#include "CScannerLoad.h"

#pragma comment(lib, "Wbemuuid.lib" )
#pragma comment(lib, "comsuppw.lib" )
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FILECOUNT_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 4)
#define		SECONDLEVEL			0x02

DWORD WINAPI Thread_StartScanForUnhide(LPVOID lpParam);

/*

typedef struct tagRawSMBIOSData
{
    BYTE        Used20CallingMethod;
    BYTE        SMBIOSMajorVersion;
    BYTE        SMBIOSMinorVersion;
    BYTE        DmiRevision;
    DWORD       Length;
    BYTE        SMBIOSTableData[];
} RawSMBIOSData, *PRawSMBIOSData;

*/
//typedef struct _UNICODE_STRING {   
//  USHORT  Length;//长度   
//  USHORT  MaximumLength;//最大长度   
//  PWSTR  Buffer;//缓存指针，访问物理内存时，此处指向UNICODE字符串"\device\physicalmemory"   
//} UNICODE_STRING,*PUNICODE_STRING;   
   
   
typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;//长度 18h   
    HANDLE RootDirectory;//  00000000   
    PUNICODE_STRING ObjectName;//指向对象名的指针   
    ULONG Attributes;//对象属性00000040h   
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR，0   
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE，0   
} OBJECT_ATTRIBUTES;   
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;
   
typedef DWORD  (__stdcall *ZWOS)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES);   
typedef DWORD  (__stdcall *ZWMV)(HANDLE,HANDLE,PVOID,ULONG,ULONG,PLARGE_INTEGER,PSIZE_T,DWORD,ULONG,ULONG);   
typedef DWORD  (__stdcall *ZWUMV)(HANDLE,PVOID); 


// CAboutDlg dialog used for App About

/***************************************************************************************************
*  Class Name     :  CAboutDlg
*  Description    :  This Class is used to implement dialog.
*  Author Name    :
*  Date           :
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMotherBoardDlg dialog

/***************************************************************************************************
*  Function Name  :  CMotherBoardDlg
*  Description    :  This Constructor is used to create dialog.
*  Author Name    :
*  Date           :
****************************************************************************************************/
CMotherBoardDlg::CMotherBoardDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CMotherBoardDlg::IDD, pParent),
	m_iUSBVaccine(0), m_iEnableTaskManager(0), m_iRegistryEditor(0),
	m_ikEnableRunWindow(0), m_iFolderOption(0), m_iUnhideFolderOption(0), m_iDefaultIESettings(0), m_bClickedNo(true)
	, m_objSqlDb(g_strDatabaseFilePath)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/***************************************************************************************************
*  Function Name  :  DoDataExchange
*  Description    :  This method is Called by the framework to exchange and validate dialog data.
*  Author Name    :
*  Date           :
****************************************************************************************************/
void CMotherBoardDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_USB_VACCINE, m_chkUSBVaccine);
//	DDX_Check(pDX, IDC_CHECK_USB_VACCINE, m_iUSBVaccine);
	DDX_Control(pDX, IDC_BUTTON_BROWSEFOLDER, m_btnBrowse);
	DDX_Control(pDX, IDC_EDIT_HIDE_FOLDERPATH, m_editPath);
	DDX_Text(pDX, IDC_EDIT_HIDE_FOLDERPATH, m_csEditPath);
	DDX_Control(pDX, IDC_CHECK_ENABLE_TASK_MANAGER, m_chkEnableTaskManager);
	DDX_Check(pDX, IDC_CHECK_ENABLE_TASK_MANAGER, m_iEnableTaskManager);
	DDX_Control(pDX, IDC_CHECK_ENABLE_REGISTRYEDITOR, m_chkRegistryEditor);
	DDX_Check(pDX, IDC_CHECK_ENABLE_REGISTRYEDITOR, m_iRegistryEditor);
	DDX_Control(pDX, IDC_CHECK_ENABLE_RUNOPTION, m_chkEnableRunWindow);
	DDX_Check(pDX, IDC_CHECK_ENABLE_RUNOPTION, m_ikEnableRunWindow);
	DDX_Control(pDX, IDC_CHECK_ENABLE_FOLDEROPTION, m_chkFolderOption);
	DDX_Check(pDX, IDC_CHECK_ENABLE_FOLDEROPTION, m_iFolderOption);
	DDX_Control(pDX, IDC_CHECK_UNHIDEFILESANDFOLDERS, m_chkUnhideFolderOption);
//	DDX_Check(pDX, IDC_CHECK_UNHIDEFILESANDFOLDERS, m_iUnhideFolderOption);
	DDX_Control(pDX, IDC_CHECK_DEFAULT_IEXPLORESETTINGS, m_chkDefaultIESettings);
	DDX_Check(pDX, IDC_CHECK_DEFAULT_IEXPLORESETTINGS, m_iDefaultIESettings);
	DDX_Control(pDX, IDC_BUTTON_OK, m_OkButton);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_STATIC_CURRENTFILE, m_CurrentFile);
	DDX_Control(pDX, IDC_STATIC_USB_VACCINE_HEADER, m_stUSBVaccineHeader);
	DDX_Control(pDX, IDC_STATIC_USB_VACCINE_TITLE, m_stUSBVaccineTitle);
	DDX_Control(pDX, IDC_STATIC_UNHIDE_FILE_FOLDER, m_stUnhideFileFolder);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_STATIC_SELECT_FILE_FOLDER_TITLE, m_stSelectFileFolderHeader);
	DDX_Control(pDX, IDC_STATIC_ALL_RIGHTS, m_stAllRightsText);
	DDX_Control(pDX, IDC_BUTTON_MINIMIZE, m_btnMinimize);
}

BEGIN_MESSAGE_MAP(CMotherBoardDlg, CJpegDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	//ON_BN_CLICKED(IDOK, &CMotherBoardDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEFOLDER, &CMotherBoardDlg::OnBnClickedButtonBrowsefolder)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CMotherBoardDlg::OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CMotherBoardDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_CHECK_UNHIDEFILESANDFOLDERS, &CMotherBoardDlg::OnBnClickedCheckUnhidefilesandfolders)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_IEXPLORESETTINGS, &CMotherBoardDlg::OnBnClickedCheckDefaultIexploresettings)
	ON_BN_CLICKED(IDC_CHECK_USB_VACCINE, &CMotherBoardDlg::OnBnClickedCheckUsbVaccine)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CMotherBoardDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_MINIMIZE, &CMotherBoardDlg::OnBnClickedButtonMinimize)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

HWINDOW   CMotherBoardDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CMotherBoardDlg::get_resource_instance() { return theApp.m_hInstance; }

// CMotherBoardDlg message handlers

/***************************************************************************************************
*  Function Name  :  OnInitDialog
*  Description    :  This method is called in response to the WM_INITDIALOG message.
*  Author Name    :
*  Date           :
****************************************************************************************************/
BOOL CMotherBoardDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();


	//AddUserAndSystemInfoToLog();
	
	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_SIX);  // to register service for process protection

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_SIX);//WLSRV_ID_SIX to register service for process protection

	//New implementation for USB vaccinator UI
	//Added By Nitin K Date: 11th May 2015
	ModifyStyle(0, WS_SYSMENU | WS_MINIMIZEBOX);

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);

	CString csNoofdays;

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	/*if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}*/
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//New implementation for USB vaccinator UI
	//Added By Nitin K Date: 11th May 2015
	m_btnClose.SetSkin(theApp.m_hResDLL, IDB_BITMAP_CLOSE, IDB_BITMAP_CLOSE, IDB_BITMAP_CLOSEOVER, IDB_BITMAP_CLOSE, 0, 0, 0, 0, 0);
	m_btnClose.SetWindowPos(&wndTop, m_rect.left + 485, 0, 26, 17, SWP_NOREDRAW);
	
	m_btnMinimize.SetSkin(theApp.m_hResDLL, IDB_BITMAP_MINMIZE, IDB_BITMAP_MINMIZE, IDB_BITMAP_MINMIZEMOVER, 0, 0, 0, 0, 0, 0);
	//m_btnMinimize.SetWindowPos(&wndTop, m_rect.left + 664, 0, 26,17, SWP_NOREDRAW);
	m_btnMinimize.SetWindowPos(&wndTop, m_rect.left + 459, 0, 26, 17, SWP_NOREDRAW);

	m_stUSBVaccineHeader.SetTextColor(RGB(24, 24, 24));
	m_stUSBVaccineHeader.SetBkColor(RGB(240, 240, 240));
	m_stUSBVaccineHeader.SetFont(&theApp.m_fontWWTextMediumSize);
	m_stUSBVaccineHeader.SetWindowPos(&wndTop, m_rect.left + 10, 60, 510, 25, SWP_NOREDRAW);
//	m_stUSBVaccineHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_EMAIL_SUPPORT"));
	m_stUSBVaccineHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_DESC"));

	
	m_chkUSBVaccine.SetWindowPos(&wndTop, m_rect.left + 50, 110, 14, 14, SWP_NOREDRAW);

	m_stUSBVaccineTitle.SetTextColor(RGB(24, 24, 24));
	m_stUSBVaccineTitle.SetBkColor(RGB(255, 255, 255));
	m_stUSBVaccineTitle.SetFont(&theApp.m_fontWWTextNormal);
	m_stUSBVaccineTitle.SetWindowPos(&wndTop, m_rect.left + 70, 110, 100, 20, SWP_NOREDRAW);
	//	m_stUSBVaccineTitle.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_EMAIL_SUPPORT"));
	m_stUSBVaccineTitle.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_TITLE"));

	m_stSelectFileFolderHeader.SetTextColor(RGB(24, 24, 24));
	m_stSelectFileFolderHeader.SetBkColor(RGB(255, 255, 255));
	m_stSelectFileFolderHeader.SetFont(&theApp.m_fontWWTextNormal);
	m_stSelectFileFolderHeader.SetWindowPos(&wndTop, m_rect.left + 50, 155, 300, 20, SWP_NOREDRAW);
	//	m_stSelectFileFolderHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_EMAIL_SUPPORT"));
	m_stSelectFileFolderHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_FILE_FOLDER"));

	m_chkUnhideFolderOption.SetWindowPos(&wndTop, m_rect.left + 50, 175, 14, 14, SWP_NOREDRAW);

	m_stUnhideFileFolder.SetTextColor(RGB(24, 24, 24));
	m_stUnhideFileFolder.SetBkColor(RGB(255, 255, 255));
	m_stUnhideFileFolder.SetFont(&theApp.m_fontWWTextNormal);
	m_stUnhideFileFolder.SetWindowPos(&wndTop, m_rect.left + 70, 175, 200, 20, SWP_NOREDRAW);
	//	m_stUnhideFileFolder.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_EMAIL_SUPPORT"));
	m_stUnhideFileFolder.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_UNHIDE_FILE_FOLDER"));

	m_editPath.SetWindowPos(&wndTop, m_rect.left + 50, 200, 350, 21, SWP_NOREDRAW);

	m_btnBrowse.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_DISABLE_WHITE_BG, 0, 0, 0, 0, 0);
	m_btnBrowse.SetWindowPos(&wndTop, m_rect.left + 410, 200, 57, 21, SWP_NOREDRAW);
	m_btnBrowse.SetTextColorA(RGB(0,0,0),1,1);
//	m_btnBrowse.SetFont(&theApp.m_fontWWTextNormal);
	m_btnBrowse.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_BROWSE"));


	m_CurrentFile.SetTextColor(RGB(24, 24, 24));
	m_CurrentFile.SetBkColor(RGB(255,255,255));
	//m_CurrentFile.SetFont(&theApp.m_fontWWTextSubTitle);
	m_CurrentFile.SetWindowPos(&wndTop, m_rect.left + 50, 225, 425, 42, SWP_NOREDRAW);
	//	m_CurrentFile.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_EMAIL_SUPPORT"));
	//m_CurrentFile.SetWindowTextW(L"Unhide files/folders:");

	m_OkButton.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_DISABLE_WHITE_BG, 0, 0, 0, 0, 0);
	m_OkButton.SetWindowPos(&wndTop, m_rect.left + 361, 280, 57, 21, SWP_NOREDRAW);
	m_OkButton.SetTextColorA(RGB(0, 0, 0), 1, 1);
	m_OkButton.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_OK"));

	m_CancelButton.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_DISABLE_WHITE_BG, 0, 0, 0, 0, 0);
	m_CancelButton.SetWindowPos(&wndTop, m_rect.left + 428, 280, 57, 21, SWP_NOREDRAW);
	m_CancelButton.SetTextColorA(RGB(0, 0, 0), 1, 1);
	m_CancelButton.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CANCEL"));
	

	LOGFONT lfInstallerTitle;
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 15;
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 6;
	m_BoldText.CreateFontIndirect(&lfInstallerTitle);
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));	 //	   with	face name "Verdana".

	m_stAllRightsText.SetWindowPos(&wndTop, m_rect.left + 20, 315, 320, 24, SWP_NOREDRAW);
	m_stAllRightsText.SetTextColor(RGB(238, 238, 238));
	m_stAllRightsText.SetBkColor(RGB(88, 88, 90));
	m_stAllRightsText.SetFont(&m_BoldText);
	CString csFooter;
	csFooter = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOOTER_MSG");
	csFooter += _T(" \u00AE ");
	csFooter += theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOOTER_MSG_YEAR");
	m_stAllRightsText.SetWindowTextW(csFooter);

	InitializeVariablesToZero();

	OnBnClickedCheckUnhidefilesandfolders();

	//Unhide the files and folder from this dir
	//UnhideFilesFolders( lpDrive );
	HideAllElements();

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_USB_VACCIN.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_USB_VACCIN.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;  
	INT pIntHeight = 0; 
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);
	
	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	CString	csWardWizModulePath = GetWardWizPathFromRegistry();
	CString	csWardWizReportsPath = L"";
	csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);

	if (!PathFileExists(csWardWizReportsPath))
	{
		m_objSqlDb.Open();
		m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
		m_objSqlDb.Close();
	}
	this->SetWindowText(L"Vibranium USB VACCINATOR");
//Kolhapur tools ends
//////////////
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************************************
*  Function Name  : HideAllElements
*  Description    : Function to hide all elements applied when not scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 4 May 2016
****************************************************************************************************/
void CMotherBoardDlg::HideAllElements()
{
	m_stUSBVaccineHeader.ShowWindow(SW_HIDE);
	m_chkUSBVaccine.ShowWindow(SW_HIDE);
	m_stUSBVaccineTitle.ShowWindow(SW_HIDE);
	m_stSelectFileFolderHeader.ShowWindow(SW_HIDE);
	m_chkUnhideFolderOption.ShowWindow(SW_HIDE);
	m_CurrentFile.ShowWindow(SW_HIDE);
	m_stUnhideFileFolder.ShowWindow(SW_HIDE);
	m_editPath.ShowWindow(SW_HIDE);
	m_btnBrowse.ShowWindow(SW_HIDE);
	m_CurrentFile.ShowWindow(SW_HIDE);
	m_OkButton.ShowWindow(SW_HIDE);
	m_CancelButton.ShowWindow(SW_HIDE);
	m_stAllRightsText.ShowWindow(SW_HIDE);

	m_btnMinimize.ShowWindow(SW_HIDE);
	m_btnClose.ShowWindow(SW_HIDE);
}


/***************************************************************************************************
*  Function Name  :  OnSysCommand
*  Description    :  it calls function,when the user selects a command from the Control menu
*  Author Name    :
*  Date           :
****************************************************************************************************/
void CMotherBoardDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.


/***************************************************************************************************
*  Function Name  :  OnPaint
*  Description    :  Used to Start GUI Designing
*  Author Name    :
*  Date           :
****************************************************************************************************/
void CMotherBoardDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMotherBoardDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*
void CMotherBoardDlg::OnBnClickedOk()
{
	//Kolhapur tools
	
	//USBVaccine();
	//EnableTaskMgr( );
	//EnableRegedit( );
	//EnableRunWindow();
	
	SetDefaultExplorerSettings();
	
	MessageBox(L"Sucess", L"Vibranium", MB_ICONINFORMATION);
}
*/

//void CMotherBoardDlg::AddLogEntry(const TCHAR *szMessage,bool isDateTime )

/*
void CMotherBoardDlg::AddLogEntry(const TCHAR *szMessage, const TCHAR *szMessage1 , const TCHAR *szMessage2 , bool isDateTime , DWORD dwLogLevel)
{
	try
	{
		static TCHAR csScanLogFullPath[512] = {0};
		FILE *pRtktOutFile = NULL;

		if(csScanLogFullPath[0] == 0)
		{
			TCHAR	szCompName[256] = {0};
			DWORD	dwCompSize = 255;

			GetComputerName(szCompName, &dwCompSize );
			GetModuleFileName(NULL, csScanLogFullPath, 511 );

			TCHAR *pFileName = wcsrchr( csScanLogFullPath, '\\' );
			if( pFileName )
			{
				pFileName++;
				*pFileName = '\0';
			}

			wcscat_s(csScanLogFullPath, 511, L"Log\\VIBOUSBVAC.LOG");
			//wcscat_s(csScanLogFullPath, 511, szCompName);
			//wcscat_s(csScanLogFullPath, 511, L"_ID.Log");
		}

		if(!pRtktOutFile)
			pRtktOutFile = _wfsopen(csScanLogFullPath, _T("a"), 0x40);

		if(pRtktOutFile != NULL)
		{
			if(isDateTime)
			{
				TCHAR tbuffer[9]= {0};
				TCHAR dbuffer[9] = {0};

				_wstrtime_s(tbuffer, 9);
				_wstrdate_s(dbuffer, 9);

				TCHAR szOutMessage[1024] = {0};

				
				wsprintf( szOutMessage, L"[%s %s]%s\r\n", dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
				fputws((LPCTSTR)szOutMessage, pRtktOutFile);

			}
			else
			{
				fputws((LPCTSTR)szMessage, pRtktOutFile);
			}

			fflush(pRtktOutFile);
			fclose(pRtktOutFile);
		}
	}
	catch(...)
	{
	}
}
*/


/*
DWORD CMotherBoardDlg::MakeAutorunDirectory( LPCTSTR pszDirPath, bool bHideAttribute )
{
	DWORD dwRet = 0x00;

	try
	{
	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		//AddLogEntry(L"### Exception in DLLEXPORT DWORD DecryptBuffer", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}

*/

/***************************************************************************************************
*  Function Name  :  RenameIfFileExists
*  Description    :  Used to Rename Autorun Files
*  Author Name    :
*  Date           :
****************************************************************************************************/
DWORD CMotherBoardDlg::MakeAutorunDirectory( LPCTSTR pszDirPath, bool bHideAttribute )
{
	DWORD dwRet = 0x00;

	try
	{
		if( !PathFileExists( pszDirPath ) )
		{
			if( !CreateDirectory(pszDirPath, NULL ) )
			{
				Sleep( 10 );
				if( !CreateDirectory(pszDirPath, NULL ) )
				{
					Sleep( 10 );
					if( !SHCreateDirectory(NULL, pszDirPath ) )
					{
						Sleep( 10 );
						if( !_wmkdir( pszDirPath ) )
						{
							Sleep( 100 );
							_wmkdir( pszDirPath );

							goto Cleanup;
						}
					}
				}
			}
		}
	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		return 0x03;
		AddLogEntry(L"### Exception in MakeAutorunDirectory", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( !PathFileExists( pszDirPath ) )
		return 0x01;

	if( bHideAttribute )
	{
		if( SetFileAttributes( pszDirPath, ( (GetFileAttributes(pszDirPath)) | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) )
			return 0x00;
	}
	else
		return 0x00;

	return 0x02;
}

/***************************************************************************************************
*  Function Name  :  RenameIfFileExists
*  Description    :  Used to Rename Autorun Files
*  Author Name    :
*  Date           :
****************************************************************************************************/
DWORD CMotherBoardDlg::RenameIfFileExists( LPCTSTR pszSourcePath )
{
	DWORD dwRet = 0x00;

	TCHAR	szAutorunPath[0x200] = {0};

	try
	{
		swprintf(szAutorunPath, 0x1FF, L"%s_%lu", pszSourcePath, GetTickCount() );
		if( !_wrename(pszSourcePath, szAutorunPath ) )
			goto Cleanup;

		Sleep( 100 );
		ZeroMemory(szAutorunPath, sizeof(szAutorunPath) );
		swprintf(szAutorunPath, 0x1FF, L"%s_%lu", pszSourcePath, GetTickCount() );
		if( !_wrename(pszSourcePath, szAutorunPath ) )
			goto Cleanup;

		Sleep( 100 );
		ZeroMemory(szAutorunPath, sizeof(szAutorunPath) );
		swprintf(szAutorunPath, 0x1FF, L"%s_%lu", pszSourcePath, GetTickCount() );
		if( !_wrename(pszSourcePath, szAutorunPath ) )
			goto Cleanup;

		dwRet = 0x01;
	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		AddLogEntry(L"### Exception in RenameIfFileExists", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 CMotherBoardDlg::InsertDataToTable(const char* szQuery)
{
	try
	{
		AddLogEntry(L"InsertDataToTable in VibraniumAutoScnDlg- AutoScanner entered", 0, 0, true, ZEROLEVEL);

		m_objSqlDb.Open();

		int iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRwId = m_objSqlDb.GetLastRowId();

		m_objSqlDb.Close();

		return iLastRwId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in InsertDataToTable in MotherBoardDlg.", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Amol Jaware
*  Date			  : 5 Aug 2016
****************************************************************************************************/
json::value CMotherBoardDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/**********************************************************************************************************
*  Function Name  :	EnterVaccinationDetails
*  Description    :	Responsible to create the query appropriate to insert data for USB Vaccination.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
void CMotherBoardDlg::EnterVaccinationDetails(LPCTSTR pszRemDrive, INT64 iSessionId)
{
	CString csInsertQuery = _T("INSERT INTO Wardwiz_USBVaccinatorDetails VALUES (null,");
	try
	{
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_USBVaccinatorDetails VALUES (null,%I64d,date('now'),date('now'),datetime('now','localtime'), datetime('now','localtime'),'%s' );"), iSessionId, pszRemDrive);

		CT2A ascii(csInsertQuery, CP_UTF8);

		InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in EnterVaccinationDetails. The query used is: ", csInsertQuery, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  VaccinateUSBDrive
*  Description    :  Used to Begins USB Vaccination
*  Author Name    :
*  Date           :
****************************************************************************************************/
DWORD CMotherBoardDlg::VaccinateUSBDrive(LPCTSTR pszRemDrive, LPTSTR lpszRetMessage)
{
	DWORD dwRet = 0x00;

	TCHAR	szAutorunPath[0x200] = {0};

	try
	{
		bool	bLocalLess16GB = false;
		swprintf(szAutorunPath, 0x1FF, L"%sAutorun.inf", pszRemDrive);
		if( PathIsDirectory(szAutorunPath) )
		{
			MakeAutorunDirectory(szAutorunPath, true );
			AddLogEntry(L"### VaccinateUSBDrive::USB already vaccinated", 0, 0, true, SECONDLEVEL);

			if (DRIVE_FIXED == GetDriveType(pszRemDrive))
				bLocalLess16GB = true;
			if (bLocalLess16GB)
			{
				swprintf_s(lpszRetMessage, 255, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_ALREADY_VACCINATED_LESS16GB"), pszRemDrive);
			}
			else
			{
				swprintf_s(lpszRetMessage, 255, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_ALREADY_VACCINATED"), pszRemDrive);
			}
			EnterVaccinationDetails(pszRemDrive,m_iUSBSessionId);
			goto Cleanup;
		}

		if( PathFileExists(szAutorunPath) )
		{
			dwRet = RenameIfFileExists(szAutorunPath);
			if( dwRet )
			{
				AddLogEntry(L"### VaccinateUSBDrive::Failed to rename, it's in use", 0, 0, true, SECONDLEVEL);
				goto Cleanup;
			}
		}

		dwRet = MakeAutorunDirectory(szAutorunPath, true);
		EnterVaccinationDetails(pszRemDrive, m_iUSBSessionId);
		
	}
	catch( ... )
	{
		dwRet = 0xFF;
		AddLogEntry(L"### Exception in VaccinateUSBDrive", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}


/***************************************************************************************************
*  Function Name  :  USBVaccine
*  Description    :  Used to implement core Functionality for USB Vaccination
*  Author Name    :
*  Date           :
****************************************************************************************************/
DWORD CMotherBoardDlg::USBVaccine()
{

	DWORD dwRet = 0x00;

	try
	{
		//Enumerate USB Drive
		//Check Autorun.inf present
		//Create Folder Autorun.inf and make as a hidden folder

		bool	bUSBNotFound = true;
		bool	bLocalLess16GB = false;
		bool	bFirstDriveSuccess = false;

		TCHAR	szDrives[512] = {0};
		TCHAR	szRemDrive[0x04] = {0};
		LPCTSTR	lpDrive = NULL;

		TCHAR	szTemp[512] = { 0 };
		TCHAR	szMessge[1024] = { 0 };
		TCHAR	szRetMessage[256] = { 0 };

		TCHAR	szSucceedMsg[512] = { 0 };
		TCHAR	szFailedMsg[512] = { 0 };
		TCHAR	szAlreadyVacMsg[512] = { 0 };

		TCHAR	szUSBDriveSuccess[16] = { 0 };
		TCHAR	szUSBDriveFailed[16] = { 0 };
		TCHAR	szUSBDriveAlreadyVac[16] = { 0 };

		GetLogicalDriveStrings( 511, szDrives );
		if( !szDrives[0] )
		{
			dwRet = 0x01;
			AddLogEntry(L"### USBVaccine::GetLogicalDriveStrings failed.", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		lpDrive = szDrives;
		while( wcslen(lpDrive) )
		{

			//If USB is removed and still explorer shows drive
			//Issue observed in Abhijeet laptop
			//Added by Vilas on 26 May 2015
			if (!PathFileExists(lpDrive))
			{
				lpDrive += 0x04;
				continue;
			}

			//Added for Pendrive showing as Local drive instead of removal like ScanDisk PD
			//Added by Vilas on 19 - 12 - 2015 for issue no:0000960
			//Supporting upto 64GB(0x1000000001) drive
			//Disavantage is that drive whose size is less than 16GB will also vaccinate.
			//8GB(1F6000000)
			//16GB(400000000)
			ULARGE_INTEGER	lDriveSize;

			lDriveSize.HighPart = 0x00;
			lDriveSize.LowPart = 0x00;

			BOOL bSucceed = GetDiskFreeSpaceEx(lpDrive, NULL, &lDriveSize, NULL);

			if (_memicmp(lpDrive, L"A:\\", 0x06) != 0x00)
			{
				if ((DRIVE_REMOVABLE == GetDriveType(lpDrive)) || ( (lDriveSize.QuadPart < 0x400000001) && (bSucceed) )
					)
				{
					bLocalLess16GB = bUSBNotFound = false;

					if (DRIVE_FIXED == GetDriveType(lpDrive))
						bLocalLess16GB = true;

					ZeroMemory(szRetMessage, sizeof(szRetMessage));
					dwRet = VaccinateUSBDrive( lpDrive, szRetMessage );
					if (!dwRet)
					{
						if (szRetMessage[0])
						{

							//Added by Vilas on 22 Dec 2015
							//Formatted message for multiple drive ( USB/Drive already vaccinated for drive : F:\, G:\)

							if (bLocalLess16GB)
								swprintf_s(szUSBDriveAlreadyVac, 15, theApp.m_objwardwizLangManager.GetString(L"IDS_USBANDDRIVE"));

							if (!szAlreadyVacMsg[0])
								wcscpy_s(szAlreadyVacMsg, 511, szRetMessage);
							else
							{
								//wcscat_s(szMessge, 1023, szRetMessage);
								swprintf_s(szTemp, 511, L", %s", lpDrive);
								wcscat_s(szAlreadyVacMsg, 511, szTemp);
							}

							AddLogEntry(szRetMessage, 0, 0, true, SECONDLEVEL);
						}
						else
						{

							if (bLocalLess16GB)
								swprintf_s(szUSBDriveSuccess, 15, theApp.m_objwardwizLangManager.GetString(L"IDS_USBANDDRIVE"));

							swprintf_s(szTemp, 511, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SUCCESS"), lpDrive);

							AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);

							//wcscat_s(szTemp, 511, L"\n");
							if (!szSucceedMsg[0])
							{ 
								bFirstDriveSuccess = true;
								wcscpy_s(szSucceedMsg, 511, szTemp);
								
								CString csInsertQuery = _T("INSERT INTO Wardwiz_USBVaccinatorSessionDetails VALUES (null,");

								csInsertQuery.Format(_T("INSERT INTO Wardwiz_USBVaccinatorSessionDetails VALUES (null,Datetime('now','localtime'),Datetime('now','localtime'),Date('now'),Date('now'),%d,%d,'%s','%s');"), m_dwScannedFiles, m_dwUnhideFiles, csTypeSelected, lpDrive);

								CT2A ascii(csInsertQuery, CP_UTF8);

								m_iUSBSessionId = InsertDataToTable(ascii.m_psz);
							}
							else
							{
								swprintf_s(szTemp, 511, L", %s", lpDrive);
								wcscat_s(szSucceedMsg, 511, szTemp);

								CString csInsertQuery = _T("INSERT INTO Wardwiz_USBVaccinatorSessionDetails VALUES (null,");

								csInsertQuery.Format(_T("INSERT INTO Wardwiz_USBVaccinatorSessionDetails VALUES (null,Datetime('now','localtime'),Datetime('now','localtime'),Date('now'),Date('now'),%d,%d,'%s','%s');"), m_dwScannedFiles, m_dwUnhideFiles, csTypeSelected, lpDrive);

								CT2A ascii(csInsertQuery, CP_UTF8);

								m_iUSBSessionId = InsertDataToTable(ascii.m_psz);

							}
						}
					}
					//dwRet = dwRet;
					else
					{

						if (bLocalLess16GB)
							swprintf_s(szUSBDriveFailed, 15, theApp.m_objwardwizLangManager.GetString(L"IDS_USBANDDRIVE"));

						swprintf_s(szTemp, 511, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_FAILED"), lpDrive);
						
						AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);

						//wcscat_s(szTemp, 511, L"\n");
						if (!szFailedMsg[0])
							wcscpy_s(szFailedMsg, 511, szTemp);
						else
						{
							swprintf_s(szTemp, 511, L", %s", lpDrive);
							wcscat_s(szFailedMsg, 511, szTemp);
						}

						//MessageBox(szTemp, L"Vibranium", MB_ICONEXCLAMATION);
					}
						//dwRet = dwRet;

					//UnhideFilesFolders( lpDrive );
				}
			}

			//swprintf_s(szTemp, 511, L"Drive letter found :: %s", lpDrive);
			//AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);
			lpDrive += 0x04;

		}
		if (bFirstDriveSuccess)
			wcscat_s(szSucceedMsg, 511, L".");

		if (bUSBNotFound)
		{
			dwRet = 0x02;
			AddLogEntry(L"### USB drive not found.", 0, 0, true, SECONDLEVEL);
			//::MessageBox(this->m_hWnd, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_DRIVE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_svVaccineStatusFnCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_DRIVE_NOT_FOUND"));
			//CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_DRIVE_NOT_FOUND"));
		}
		else
		{
		/*	if ( dwRet)
				::MessageBox(this->m_hWnd, szMessge, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			else
				::MessageBox(this->m_hWnd, szMessge, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
		*/

			//Added by Vilas on 21 - 12 - 2015 for the message
			//USB Vaccinated successfully for drive : F:\
			//USB Vaccinated successfully for drive : G:\
			//Converted into USB Vaccinated successfully for drive : F:\, G:\

			if (wcslen(szSucceedMsg) && wcslen(szFailedMsg) )
			{

				if ((wcslen(szUSBDriveSuccess)) && (wcslen(szUSBDriveFailed)))
					swprintf_s(szMessge, 1023, L"%s %s\n%s %s", szUSBDriveSuccess, &szSucceedMsg[0x04], szUSBDriveFailed, &szFailedMsg[0x04]);

				if ((wcslen(szUSBDriveSuccess)) && (wcslen(szUSBDriveFailed) == 0x00))
					swprintf_s(szMessge, 1023, L"%s %s\n%s", szUSBDriveSuccess, &szSucceedMsg[0x04], szFailedMsg);

				if ((wcslen(szUSBDriveFailed)) && (wcslen(szUSBDriveSuccess) == 0x00))
					swprintf_s(szMessge, 1023, L"%s\n%s %s", szSucceedMsg, szUSBDriveFailed, &szFailedMsg[0x04]);

				if ((wcslen(szUSBDriveSuccess) == 0x00) && (wcslen(szUSBDriveFailed) == 0x00))
					swprintf_s(szMessge, 1023, L"%s\n%s", szSucceedMsg, szFailedMsg);
			}

			if ((wcslen(szSucceedMsg) ) && (wcslen(szFailedMsg) == 0x00))
			{
				//wcscpy_s(szMessge, 1023, szSucceedMsg);
				if ( wcslen(szUSBDriveSuccess) )
					swprintf_s(szMessge, 1023, L"%s %s", szUSBDriveSuccess, &szSucceedMsg[0x04]);
				else
					wcscpy_s(szMessge, 1023, szSucceedMsg);

			}

			if ((wcslen(szSucceedMsg) ==0x00) && (wcslen(szFailedMsg)) )
			{
				if (wcslen(szUSBDriveFailed))
					swprintf_s(szMessge, 1023, L"%s %s", szUSBDriveFailed, &szFailedMsg[0x04]);
				else
					wcscpy_s(szMessge, 1023, szFailedMsg);
			}

			//Added by Vilas on 22 Dec 2015
			if ((wcslen(szAlreadyVacMsg)) && (wcslen(szMessge)) )
			{
				if (wcslen(szUSBDriveAlreadyVac))
				{
					wcscat_s(szMessge, 1023, L"\n");
					wcscat_s(szMessge, 1023, szUSBDriveAlreadyVac);
					wcscat_s(szMessge, 1023, &szAlreadyVacMsg[0x03]);

				}
				else
				{
					wcscat_s(szMessge, 1023, L"\n");
					wcscat_s(szMessge, 1023, szAlreadyVacMsg);
				}
			}
			
			if ((wcslen(szAlreadyVacMsg)) && (wcslen(szMessge)==0x00))
			{
				if (wcslen(szUSBDriveAlreadyVac))
					swprintf_s(szMessge, 1023, L"%s %s", szUSBDriveAlreadyVac, &szAlreadyVacMsg[0x04]);
				else
					wcscpy_s(szMessge, 1023, szAlreadyVacMsg);
			}

			//MessageBox type changed as per success or failure
			//Added by Vilas on 23 Dec 2015
			if ((wcslen(szSucceedMsg) == 0x00) && (wcslen(szAlreadyVacMsg) == 0x00))
			{
				//::MessageBox(this->m_hWnd, szMessge, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				m_svVaccineStatusFnCB.call(szMessge);
				// Add entries into Database..
				CString csInsertQuery = _T("UPDATE Wardwiz_USBVaccinatorSessionDetails ");

				csInsertQuery.Format(_T("UPDATE Wardwiz_USBVaccinatorSessionDetails SET db_USBSessionEndTime =Datetime('now','localtime'), db_USBSessionEndDate = Date('now') ,db_FilesScanCount = %d, db_FilesUnHideCount= %d, db_USBSessionPathSelected = %d WHERE db_USBVaccinatorSessionId = %I64d ;"), m_dwScannedFiles, m_dwUnhideFiles, lpDrive, m_iUSBSessionId);

				CT2A ascii(csInsertQuery, CP_UTF8);

				//CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
				//objSqlDb.Open();
				//objSqlDb.CreateWardwizSQLiteTables();
				//objSqlDb.Close();

				m_iUSBSessionId = InsertDataToTable(ascii.m_psz);
				//CallNotificationMessage((SCITER_STRING)szMessge);
			}
			else
			{
			//	::MessageBox(this->m_hWnd, szMessge, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
				m_svVaccineStatusFnCB.call(szMessge);
				//CallNotificationMessage((SCITER_STRING)szMessge);
			}
		}

	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		dwRet = 0xFF;
		AddLogEntry(L"### Exception in USBVaccine", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}


/***************************************************************************************************
*  Function Name  :  EnableTaskMgr
*  Description    :  Used to Enable Task Manager
*  Author Name    :
*  Date           :
****************************************************************************************************/
bool CMotherBoardDlg::EnableTaskMgr( )
{
	CITinRegWrapper objReg;
	if(objReg.SetRegistryDWORDData(	HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
									L"DisableTaskMgr", 0x00) != 0x00)
	{
		AddLogEntry(L"### Enable TaskMgr failed.", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  EnableRegedit
*  Description    :  Used to Enable RegEdit
*  Author Name    :
*  Date           :
****************************************************************************************************/
bool CMotherBoardDlg::EnableRegedit( )
{
	CITinRegWrapper objReg;
	if(objReg.SetRegistryDWORDData(	HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
									L"DisableRegistryTools", 0x00) != 0x00)
	{
		AddLogEntry(L"### Enable Regedit failed.", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/***************************************************************************************************
*  Function Name  :  GetCurrentLoggedUserSID
*  Description    :  Used to Get Current Login User
*  Author Name    :
*  Date           :
****************************************************************************************************/
DWORD CMotherBoardDlg::GetCurrentLoggedUserSID( LPTSTR lpUserSID )
{
	DWORD dwRet = 0x00;

	HANDLE	hToken = NULL;
	LPBYTE	lpBuffer = NULL;

	DWORD dwBufferSize = 0;

	LPTSTR lpStringSID = NULL;

	try
	{

		OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken );
		if( !hToken )
		{
			AddLogEntry(L"### GetCurrentLoggedUserSID:: OpenProcessToken failed.", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
			goto Cleanup;
		}


		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBufferSize );
		if( !dwBufferSize )
		{
			AddLogEntry(L"### GetCurrentLoggedUserSID:: GetTokenInformation failed.", 0, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup;
		}

		lpBuffer = new BYTE[ dwBufferSize ];
		if( !lpBuffer )
		{
			AddLogEntry(L"### GetCurrentLoggedUserSID:: new operator failed.", 0, 0, true, SECONDLEVEL);
			dwRet = 0x03;
			goto Cleanup;
		}

		ZeroMemory( lpBuffer, dwBufferSize );

		PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>( &lpBuffer[0] );
		if( !pTokenUser )
		{
			AddLogEntry(L"### GetCurrentLoggedUserSID:: ZeroMemory failed.", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}

		GetTokenInformation( hToken, TokenUser, pTokenUser, dwBufferSize, &dwBufferSize) ;

		ConvertSidToStringSid(pTokenUser->User.Sid, &lpStringSID);
		DWORD	dwSIDLen = (DWORD )wcslen(lpStringSID);

		if( dwSIDLen )
		{
			wcscpy_s(lpUserSID, 255, lpStringSID );
			lpUserSID[dwSIDLen] = '\0';
		}
		else
		{
			dwRet = 0x05;
			AddLogEntry(L"### GetCurrentLoggedUserSID:: ConvertSidToStringSid failed.", 0, 0, true, SECONDLEVEL);
		}

	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		AddLogEntry(L"### Exception in GetCurrentLoggedUserSID", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( hToken )
		CloseHandle( hToken );

	hToken = NULL;

	if( lpStringSID )
		LocalFree((HLOCAL)lpStringSID);

	lpStringSID = NULL;

	if( lpBuffer )
		delete [] lpBuffer;

	lpBuffer = NULL;

	return dwRet;
}


/***************************************************************************************************
*  Function Name  :  EnableRunWindow
*  Description    :  Used to Enable Running Window
*  Author Name    :
*  Date           :
****************************************************************************************************/
DWORD CMotherBoardDlg::EnableRunWindow( )
{
	DWORD dwRet = 0x00;

	TCHAR	szSID[0x100] = {0};
	TCHAR	szSubKey[0x200] = {0};

	try
	{

		dwRet = GetCurrentLoggedUserSID( szSID );
		if( dwRet )
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		swprintf(szSubKey, 0x1FF, L"%s\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", szSID );

		CITinRegWrapper objReg;
	
		if(objReg.SetRegistryDWORDData(	HKEY_USERS, szSubKey,
										L"Start_ShowRun", 0x01) )
		{
			dwRet = 0x02;
			goto Cleanup;
			AddLogEntry(L"### EnableRunWindow:: Run Window enabled failed due to Registry access problem.", 0, 0, true, SECONDLEVEL);
		}
		else
		AddLogEntry(L"### EnableRunWindow:: Run Window enabled successfully, please restart to take effect.", 0, 0, true, SECONDLEVEL);
	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		AddLogEntry(L"### Exception in EnableRunWindow", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	EnterHideUnhideDetails
*  Description    :	Insert Hide/Unhide related details into database.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
void CMotherBoardDlg::EnterHideUnhideDetails(LPCTSTR pszDirPath, INT64 iSessionId)
{
	try
	{
		CString csInsertQuery = _T("INSERT INTO Wardwiz_USBVaccinatorDetails VALUES (null,");
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_USBVaccinatorDetails VALUES (null,%I64d,date('now'),date('now'),datetime('now','localtime'), datetime('now','localtime'),'%s' );"), iSessionId, pszDirPath);

		CT2A ascii(csInsertQuery, CP_UTF8);

		//CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
		//objSqlDb.Open();
		//objSqlDb.CreateWardwizSQLiteTables();
		//objSqlDb.Close();

		InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in EnterHideUnhideDetails", 0, 0, true, SECONDLEVEL);

	}
}

/***************************************************************************************************
*  Function Name  :  UnhideFilesFolders
*  Description    :  Used to Unhide Files and Folders
*  Author Name    :
*  Date           :
****************************************************************************************************/
DWORD CMotherBoardDlg::UnhideFilesFolders( LPCTSTR pszDirPath, bool bDeleteLNKFile )
{
	DWORD		dwRet = 0x00;

	TCHAR		szSubDir[0x400] = {0};
	TCHAR		szFilePath[0x400] = {0};
	TCHAR		szTemp[0x400] = { 0 };

	WIN32_FIND_DATA		ffd = {0};

	HANDLE hFind = INVALID_HANDLE_VALUE;

	bool	bEndSlash = false;

	bool	bUnhide = false;

	try
	{

		if( !PathFileExists( pszDirPath ) )
		{
			AddLogEntry(L"### UnhideFilesFolders::Given path not found.", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
			goto Cleanup;
		}

		wcscpy_s(szFilePath, 1023, pszDirPath);
		_wcsupr_s(szFilePath, 1023);

		if (CheckInExcludeList(szFilePath))
		{
			goto Cleanup;
		}

		DWORD	dwFileAttributes = 0x00;
		if (PathIsDirectory(pszDirPath))
		{
			if( pszDirPath[wcslen(pszDirPath) -1 ] == '\\' )
			{
				bEndSlash = true;
				swprintf(szSubDir, 0x3FF, L"%s*.*", pszDirPath );
			}
			else
				swprintf(szSubDir, 0x3FF, L"%s\\*.*", pszDirPath );
		}
		else
		{
			m_dwScannedFiles++;
			m_CurrentFile.SetWindowTextW(pszDirPath);
			dwFileAttributes = GetFileAttributes(pszDirPath);
			if (FILE_ATTRIBUTE_HIDDEN == (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				SetFileAttributes(pszDirPath, (dwFileAttributes & (~FILE_ATTRIBUTE_HIDDEN)));
				bUnhide = true;
			}

			if (FILE_ATTRIBUTE_SYSTEM == (dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
			{
				SetFileAttributes(pszDirPath, (dwFileAttributes & (~FILE_ATTRIBUTE_SYSTEM)));
				bUnhide = true;
			}

			if (bUnhide)
			{
				m_dwUnhideFiles++;
				swprintf_s(szTemp, 0x3FF, L"Unhide file ::%s", szFilePath);
				AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);
				EnterHideUnhideDetails(szFilePath, m_iUSBSessionId); //single unhide file entry 
			}

			goto Cleanup;
		}

		//BOOL bWorking = finder.FindFile(szSubDir);
		hFind = FindFirstFile(szSubDir, &ffd);

		if( INVALID_HANDLE_VALUE == hFind )
		{
			//AddLogEntry(L"### UnhideFilesFolders::FindFile failed.", 0, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup;
		}

		do
		{
			bUnhide = false;

			if( (_wcsicmp(ffd.cFileName, L".") == 0 ) || (_wcsicmp(ffd.cFileName, L"..") == 0 ) )
				continue;

			if( bEndSlash )
				swprintf(szFilePath, 0x3FF, L"%s%s", pszDirPath, ffd.cFileName );
			else
				swprintf(szFilePath, 0x3FF, L"%s\\%s", pszDirPath, ffd.cFileName );

			m_dwScannedFiles++;
			m_CurrentFile.SetWindowTextW(szFilePath);

			if (PathFileExists(szFilePath))
			{
				TCHAR szShortName[MAX_PATH] = { 0 };
				DWORD dwbuffer = 60;
				BEHAVIOR_EVENT_PARAMS params;
				sciter::value map;
				GetShortPathName(szFilePath, szShortName, dwbuffer);
				m_csFileName.Format(L"%s", szShortName);
				map.set_item("one", sciter::string(m_csFileName));
				params.cmd = FILECOUNT_EVENT_CODE;
				params.he = params.heTarget = m_root_el;
				params.data = map;
				m_root_el.fire_event(params, true);
			}
			//Added secure function by Vilas on 01 / 10 / 2015
			_wcsupr_s(szFilePath, 1023);

			if (CheckInExcludeList(szFilePath))
			{
				continue;
			}
			
			if (FILE_ATTRIBUTE_HIDDEN == (ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				SetFileAttributes(szFilePath, (ffd.dwFileAttributes & (~FILE_ATTRIBUTE_HIDDEN)));
				bUnhide = true;
			}

			if (FILE_ATTRIBUTE_SYSTEM == (GetFileAttributes(szFilePath) & FILE_ATTRIBUTE_SYSTEM))
			{
				SetFileAttributes(szFilePath, (GetFileAttributes(szFilePath) & (~FILE_ATTRIBUTE_SYSTEM)));
				bUnhide = true;
			}

			if (bUnhide)
			{
				m_dwUnhideFiles++;
				swprintf_s(szTemp, 0x3FF, L"Unhide file ::%s", szFilePath);

				//Add entry to the database..
				EnterHideUnhideDetails(szFilePath,m_iUSBSessionId);

				AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);
			}

			
			// if it's a directory, recursively search it 
			if (FILE_ATTRIBUTE_DIRECTORY == (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				UnhideFilesFolders(szFilePath);

			ZeroMemory( szFilePath, sizeof(szFilePath) );

		} while( FindNextFile(hFind, &ffd) != 0 );

		FindClose( hFind );
		hFind = INVALID_HANDLE_VALUE;
		
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in UnhideFilesFolders", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}

bool CMotherBoardDlg::SetDefaultExplorerSettings()
{
	CITinRegWrapper objReg;

	if(objReg.SetRegistryDWORDData(	HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", L"NoFolderOptions" , 0x00) )
	{
		return false;
	}

	if(objReg.SetRegistryDWORDData(	HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", L"NoFolderOptions" , 0x00) )
	{
		return false;
	}

	return true;
}


/***************************************************************************************************
*  Function Name  :  OnBnClickedButtonBrowsefolder
*  Description    :  Used to Handle Action for Browse Button
*  Author Name    :
*  Date           :
****************************************************************************************************/
void CMotherBoardDlg::OnBnClickedButtonBrowsefolder()
{
TCHAR *pszPath = new TCHAR[MAX_PATH];
	SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));

	CString csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_FILE_FOLDER");
	BROWSEINFO        bi = {m_hWnd, NULL, pszPath, csMessage, BIF_BROWSEINCLUDEFILES, NULL, 0L, 0};		//BIF_RETURNONLYFSDIRS
	LPITEMIDLIST      pIdl = NULL;
	m_csEditPath = L"";
	LPITEMIDLIST  pRoot = NULL;
	bi.pidlRoot = pRoot;
	pIdl = SHBrowseForFolder(&bi);
	if(NULL != pIdl)
	{
		SHGetPathFromIDList(pIdl, pszPath);
		size_t iLen = wcslen(pszPath);
		//if (iLen > 0 && GetDriveType(pszPath) != DRIVE_CDROM && GetDriveType(pszPath) == DRIVE_REMOVABLE)
		if ((iLen > 0 && GetDriveType(pszPath) != DRIVE_CDROM) && *pszPath != 'A') // A is Floppy disk drive.
		{
			CString csTemp;
			csTemp = pszPath;
			if(csTemp.Right(1) == L"\\")
			{
				csTemp = csTemp.Left(static_cast<int>(iLen) -1);
			}
			m_editPath.SetWindowText(csTemp);
			m_csEditPath = csTemp;
		}
		else 
		{
			m_csEditPath = L"INVALID";
		}
	}
	else 
	{
		m_csEditPath = L"CANCEL";
	}
	delete [] pszPath;
	pszPath = NULL;
}

/***************************************************************************************************
*  Function Name  :  OnBnClickedButtonOk
*  Description    :  Used to Handle Events for OK Button
*  Author Name    :
*  Date           :
****************************************************************************************************/
void CMotherBoardDlg::OnBnClickedButtonOk()
{
	//UpdateData(TRUE);

	m_OkButton.EnableWindow(FALSE);

	bool	bProcessSuccess = false;
	bool	bRebootRequired = false;

	TCHAR	szTemp[512] = { 0 };

	if ((!m_iUSBVaccine) && (!m_iUnhideFolderOption))
	{
		//::MessageBox(this->m_hWnd, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		//m_OkButton.EnableWindow();
		m_svVaccineStatusFnCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT"));
		//CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT"));
		return;
	}

	m_bProcessStarted = true;

	m_chkUSBVaccine.EnableWindow(FALSE);
	m_chkUnhideFolderOption.EnableWindow(FALSE);
	m_editPath.EnableWindow(FALSE);
	m_btnBrowse.EnableWindow(FALSE);

	DWORD	dwRetUSBStatus = 0x00;

	if(m_iUSBVaccine == 0x01)
	{
		dwRetUSBStatus = USBVaccine();
		
		//if (dwRetUSBStatus == 0x00)
		//issue: Need not to close UI dialog when unhide folder option selected
		//Added on 20 / July / 2015
		if( (dwRetUSBStatus == 0x00) && (!m_iUnhideFolderOption) )
		{
			bProcessSuccess = true;
			//issue: Need to close UI dialog when USB is vaccinated
			m_bProcessStarted = false;
			//OnBnClickedButtonCancel();

			//return;
			
			//wcscpy_s(szTemp, 511, L"USB vaccination ");
		}
	}

	if(m_iEnableTaskManager == 0x01)
	{
		if(EnableTaskMgr( ))
		{
			bProcessSuccess = true;
			bRebootRequired = true;
		}
	}

	if(m_iRegistryEditor == 0x01)
	{
		if(EnableRegedit( ))
		{
			bProcessSuccess = true;
			bRebootRequired = true;
		}
	}

	if(m_ikEnableRunWindow == 0x01)
	{
		if(EnableRunWindow() == 0x00)
		{
			bProcessSuccess = true;
			bRebootRequired = true;
		}
	}

	if(m_iFolderOption == 0x01)
	{
		bProcessSuccess = SetDefaultExplorerSettings();
	}

	m_dwScannedFiles = m_dwUnhideFiles = 0x00;

	if(m_iUnhideFolderOption == 0x01)
	{
		if(m_csEditPath.GetLength() == 0)
		{
			//::MessageBox(this->m_hWnd, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_FILE_FOLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_svVaccineStatusFnCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_FILE_FOLDER"));
			//CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_FILE_FOLDER"));
			m_bProcessStarted = false;
			goto Cleanup;
		}

		if(!PathFileExists(m_csEditPath))
		{
		//	::MessageBox(this->m_hWnd, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_VALID_FILE_FOLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_svVaccineStatusFnCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_VALID_FILE_FOLDER"));
			//CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_VALID_FILE_FOLDER"));
			m_bProcessStarted = false;
			goto Cleanup;
		}

		CString csInsertQuery = _T("INSERT INTO Wardwiz_USBVaccinatorSessionDetails VALUES (null,");

		csTypeSelected.Replace(L"'", L"''");
		m_csEditPath.Replace(L"'", L"''");

		csInsertQuery.Format(_T("INSERT INTO Wardwiz_USBVaccinatorSessionDetails VALUES (null,Datetime('now','localtime'),Datetime('now','localtime'),Date('now'),Date('now'),%d,%d,'%s','%s');"), m_dwScannedFiles, m_dwUnhideFiles, csTypeSelected, m_csEditPath);

		CT2A ascii(csInsertQuery, CP_UTF8);

		m_iUSBSessionId = InsertDataToTable(ascii.m_psz);


		m_hThreadStartScanForUnhide = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_StartScanForUnhide,
			(LPVOID)this, 0, 0);
		Sleep(1000);
		if (!m_hThreadStartScanForUnhide)
		{
			AddLogEntry(L"thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
			//::MessageBox(this->m_hWnd, L"Thread creation failed, please check memory usage.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			bProcessSuccess = false;
			m_bProcessStarted = false;

			m_OkButton.EnableWindow();
		}

		
		/*if(UnhideFilesFolders(m_csEditPath) != 0x00)
		{
			AddLogEntry(L"### Failed in UnhideFilesFolders");
		}
		else
		{
			if (m_dwUnhideFiles)
			{
				swprintf_s(szTemp, 511, L"Total unhide file(s) :%lu", m_dwUnhideFiles);
				AddLogEntry(szTemp);
				wcscat_s(szTemp, 511, L"\n");
			}

			if (!m_dwScannedFiles)
			{
				wcscpy_s(szTemp, 511, L"No file(s)/folder(s) found in selected folder.");
				AddLogEntry(szTemp);
				wcscat_s(szTemp, 511, L"\n");
			}
			bProcessSuccess = true;
		}
		*/
	}

	if(m_iDefaultIESettings == 0x01)
	{
		if(!SetGoogleAsDefaultBrowserInernetExplorer())
		{
			AddLogEntry(L"### Failed SetGoogleAsDefaultBrowserInernetExplorer");
		}
		else
		{
			bProcessSuccess = true;
		}
	}


/*	if (!m_iUnhideFolderOption)
	{
		if(bProcessSuccess)
		{
			if (szTemp[0])
				wcscat_s(szTemp, 511, L"Process completed successfully.\n");
			else
				wcscpy_s(szTemp, 511, L"Process completed successfully.\n");

			if (bRebootRequired)
				wcscat_s(szTemp, 511, L"Please restart your computer to take effect.\n");

			AddLogEntry(L"Process completed successfully.\n");
		
			MessageBox(szTemp, L"Vibranium", MB_ICONINFORMATION);
		}
		else
		{
			//Avoiding due to failed msg already shown
			if (dwRetUSBStatus != 0x02 )
				MessageBox(L"Process Failed.Please see log for more information", L"Vibranium", MB_ICONEXCLAMATION);
		}
	}
*/
Cleanup:

	if ((!m_iUnhideFolderOption) || (!m_bProcessStarted) )
	{

		m_bProcessStarted = false;

		m_editPath.SetWindowTextW(L"");

		m_OkButton.EnableWindow();
		//m_CancelButton.EnableWindow();
		
		//m_editPath.EnableWindow();
		//m_btnBrowse.EnableWindow();

		m_chkUSBVaccine.EnableWindow();
		m_chkUnhideFolderOption.EnableWindow();
		if (m_chkUnhideFolderOption.GetCheck() == BST_CHECKED)
		{
			m_editPath.EnableWindow();
			m_btnBrowse.EnableWindow();
		}
		else
		{
			m_editPath.EnableWindow(FALSE);
			m_btnBrowse.EnableWindow(FALSE);
		}

		::ShowWindow(this->m_hWnd, SW_RESTORE);
	}
}

void CMotherBoardDlg::OnBnClickedButtonCancel()
{

/*	if (m_bProcessStarted)
	{
		if (::MessageBox(this->m_hWnd, L"Do you want to stop process ?", L"Vibranium", MB_YESNO | MB_ICONQUESTION) == IDNO)
			return;
	}

	OnCancel();
*/
	OnBnClickedButtonClose();
}

/***************************************************************************************************
*  Function Name  :  OnBnClickedCheckUnhidefilesandfolders
*  Description    :  Used to Handle Events for Unhiding Files
*  Author Name    :
*  Date           :
****************************************************************************************************/
void CMotherBoardDlg::OnBnClickedCheckUnhidefilesandfolders()
{
	UpdateData(TRUE);

	m_editPath.EnableWindow(m_iUnhideFolderOption);
	m_btnBrowse.EnableWindow(m_iUnhideFolderOption);
}

/***************************************************************************************************
*  Function Name  :  SetGoogleAsDefaultBrowserInernetExplorer
*  Description    :  Used to set Google as default Explorer
*  Author Name    :
*  Date           :
****************************************************************************************************/
bool CMotherBoardDlg::SetGoogleAsDefaultBrowserInernetExplorer()
{
	CITinRegWrapper objReg;

	try
	{

		if(objReg.SetRegistryValueData(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Internet Explorer\\Main", L"Start Page", L"http://www.google.com"))
		{
			return false;
		}

		if(objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer\\Main", L"Start Page", L"http://www.google.com"))
		{
			return false;
		}


		TCHAR	szSID[0x100] = {0};
		TCHAR	szSubKey[0x200] = {0};

		GetCurrentLoggedUserSID( szSID );
		if( !szSID[0] )
		{
			AddLogEntry(L"### Failed SetGoogleAsDefaultBrowserInernetExplorer:: SID generation");
			return false;
		}


		swprintf(szSubKey, 0x1FF, L"%s\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CabinetState", szSID );
		if(objReg.SetRegistryDWORDData(	HKEY_USERS, szSubKey,
										L"FullPathAddress", 0x01) )
		{
			AddLogEntry(L"### EnableRunWindow:: Address Bar enabled failed due to Registry access problem.", 0, 0, true, SECONDLEVEL);
			return false;
		}

		ZeroMemory(szSubKey, sizeof(szSubKey) );
		swprintf(szSubKey, 0x1FF, L"%s\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", szSID );
		if(objReg.SetRegistryDWORDData(	HKEY_USERS, szSubKey,
										L"HideFileExt", 0x00) )
		{
			AddLogEntry(L"### EnableRunWindow:: Hide file extension enabled failed due to Registry access problem.", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### EnableRunWindow:: Exception in SetGoogleAsDefaultBrowserInernetExplorer.", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

void CMotherBoardDlg::OnBnClickedCheckDefaultIexploresettings()
{
	// TODO: Add your control notification handler code here
}


void CMotherBoardDlg::OnBnClickedCheckUsbVaccine()
{
	// TODO: Add your control notification handler code here
}


/***************************************************************************************************
*  Function Name  :  Thread_StartScanForUnhide
*  Description    :  Used to start (Handle) Thread for Scanning Files
*  Author Name    :
*  Date           :
****************************************************************************************************/
DWORD WINAPI Thread_StartScanForUnhide(LPVOID lpParam)
{
	DWORD		dwRet = 0x00;

	try
	{
		CMotherBoardDlg	*pCacheGenDlg = (CMotherBoardDlg *)lpParam;

		pCacheGenDlg->UnhideFilesFolders(pCacheGenDlg->m_csEditPath);

		//Unhide selected file/folder
		pCacheGenDlg->SelectedFilesFolders(pCacheGenDlg->m_csEditPath);
		
		pCacheGenDlg->m_CurrentFile.SetWindowTextW(L"");

		TCHAR	szTemp[512] = { 0 };

		//if (pCacheGenDlg->m_dwUnhideFiles)
		//{
		if (!pCacheGenDlg->m_dwUnhideFiles)
		{
			swprintf_s(szTemp, 511, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_NO_FILE_FOLDER"), pCacheGenDlg->m_dwUnhideFiles);
			//pCacheGenDlg->AddLogEntry(szTemp);
			AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);
			//Add entry to the database..
			pCacheGenDlg->EnterHideUnhideDetails(L"IDS_USB_VACCINE_NO_FILE_FOLDER", pCacheGenDlg->m_iUSBSessionId);
	
			wcscat_s(szTemp, 511, L"\n");
		}
		else
		{
			swprintf_s(szTemp, 511, L"%s : %lu", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_TOTAL_FILES"), pCacheGenDlg->m_dwUnhideFiles);
			//pCacheGenDlg->AddLogEntry(szTemp);
			AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);
			wcscat_s(szTemp, 511, L"\n");
		}
		//}

		if (!pCacheGenDlg->m_dwScannedFiles)
		{
			wcscpy_s(szTemp, 511, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_NO_FILE_FOLDER_FOUND"));
			//pCacheGenDlg->AddLogEntry(szTemp);
			AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);
			wcscat_s(szTemp, 511, L"\n");
		}

		pCacheGenDlg->m_bProcessStarted = false;
		// Add entries into Database..
		CString csInsertQuery = _T("UPDATE Wardwiz_USBVaccinatorSessionDetails ");

		csInsertQuery.Format(_T("UPDATE Wardwiz_USBVaccinatorSessionDetails SET db_USBSessionEndTime =Datetime('now','localtime'), db_USBSessionEndDate = Date('now') ,db_FilesScanCount = %d, db_FilesUnHideCount= %d WHERE db_USBVaccinatorSessionId = %I64d ;"), pCacheGenDlg->m_dwScannedFiles, pCacheGenDlg->m_dwUnhideFiles, pCacheGenDlg->m_iUSBSessionId);

		CT2A ascii(csInsertQuery, CP_UTF8);

		//CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
		//objSqlDb.Open();
		//objSqlDb.CreateWardwizSQLiteTables();
		//objSqlDb.Close();

		pCacheGenDlg->InsertDataToTable(ascii.m_psz);

		//pCacheGenDlg->AddLogEntry(L"Process completed successfully.\n");
		AddLogEntry(L"Process completed successfully.\n\n", 0, 0, true, SECONDLEVEL);

		pCacheGenDlg->ShowWindow(SW_RESTORE);
		//::MessageBox(pCacheGenDlg->m_hWnd, szTemp, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
		
	    pCacheGenDlg->m_svVaccineStatusFnCB.call(szTemp);
		//pCacheGenDlg->CallNotificationMessage((SCITER_STRING)szTemp);

		pCacheGenDlg->m_editPath.SetWindowTextW(L"");

		pCacheGenDlg->m_OkButton.EnableWindow();
		//m_CancelButton.EnableWindow();
		pCacheGenDlg->m_chkUSBVaccine.EnableWindow();
		pCacheGenDlg->m_chkUnhideFolderOption.EnableWindow();

		if (pCacheGenDlg->m_chkUnhideFolderOption.GetCheck() == BST_CHECKED)
		{
			pCacheGenDlg->m_editPath.EnableWindow();
			pCacheGenDlg->m_btnBrowse.EnableWindow();
		}
		else
		{
			pCacheGenDlg->m_editPath.EnableWindow(FALSE);
			pCacheGenDlg->m_btnBrowse.EnableWindow(FALSE);
		}
	}
	catch (...)
	{
		//pHeuScnDlg->AddLogEntry(L"### Exception in Thread_GenerateUpdateFiles()");
	}

	return dwRet;

}

/***************************************************************************************************
*  Function Name  :  PreTranslateMessage
*  Description    :  Used to translate messages on Event 
*  Author Name    :
*  Date           :
****************************************************************************************************/
BOOL CMotherBoardDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************************************
*  Function Name  :  OnBnClickedButtonClose
*  Description    :  Used to implement various Terminating Scenarios 
*  Author Name    :	 Yogeshwar Rasal
*  Date           :  2 June 2016
*  Modification	  :  PostQuitMessage() added at OnCancel()
****************************************************************************************************/
void CMotherBoardDlg::OnBnClickedButtonClose()
{
	try
	{
		m_objSqlDb.Close();
		
		if (m_bProcessStarted)
		{
			if ((m_iUnhideFolderOption == 0x01) && (m_hThreadStartScanForUnhide))
			{
				::SuspendThread(m_hThreadStartScanForUnhide);
				m_bProcessStarted = false;
			}
		}
		
		if (m_hThreadStartScanForUnhide != NULL)
		{
			::TerminateThread(m_hThreadStartScanForUnhide, 0);
			m_hThreadStartScanForUnhide = NULL;
			m_bProcessStarted = false;
		}
		// Add entries into Database..
		CString csInsertQuery = _T("UPDATE Wardwiz_USBVaccinatorSessionDetails ");

		csInsertQuery.Format(_T("UPDATE Wardwiz_USBVaccinatorSessionDetails SET db_USBSessionEndTime =Datetime('now','localtime'), db_USBSessionEndDate = Date('now') ,db_FilesScanCount = %d, db_FilesUnHideCount= %d WHERE db_USBVaccinatorSessionId = %I64d ;"), m_dwScannedFiles, m_dwUnhideFiles, m_iUSBSessionId);

		CT2A ascii(csInsertQuery, CP_UTF8);

		//CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
		//objSqlDb.Open();
		//objSqlDb.CreateWardwizSQLiteTables();
		//objSqlDb.Close();

		InsertDataToTable(ascii.m_psz);
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::OnBnClickedButtonClose", 0, 0, true, SECONDLEVEL);
	}
	PostQuitMessage(WM_CLOSE);
}

/***************************************************************************************************
*  Function Name  :  OnBnClickedButtonMinimize
*  Description    :  Used to implement Minimize Button Click 
*  Author Name    :	 Yogeshwar Rasal
*  Date           :	 2 June 2016
****************************************************************************************************/
void CMotherBoardDlg::OnBnClickedButtonMinimize()
{
	this->ShowWindow(SW_MINIMIZE);
}

/***************************************************************************************************
*  Function Name  :  OnClose
*  Description    :  Used to implement various Terminating Scenarios (Taskbar Close)
*  Author Name    :  Yogeshwar Rasal
*  Date			  :  23th Sept. 2016
****************************************************************************************************/
void CMotherBoardDlg::OnClose()
{
	try
	{ 
    	SCITER_VALUE svIsDetectedFile = m_root_el.call_function("IsWindowClose");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::OnClose", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************
Function Name  : InitializeVariablesToZero
Description    : Initializing variables to it's initial value
Author Name    : Vilas Suvarnakar
SR_NO		   :
Date           : 14 May 2015
Modification   :
****************************************************************************/
void CMotherBoardDlg::InitializeVariablesToZero()
{
	//Sleep(1000 * 20);
	m_bProcessStarted = false;
	m_cslExcludeFiles.RemoveAll();

	m_cslExcludeFiles.AddTail(L"DESKTOP.INI");
	m_cslExcludeFiles.AddTail(L"VBUSERREG.DB");
}

/***************************************************************************
Function Name  : CheckInExcludeList
Description    : Checking file in exclude list. If found, return true
Author Name    : Vilas Suvarnakar
SR_NO		   :
Date           : 14 May 2015
Modification   :
****************************************************************************/
bool CMotherBoardDlg::CheckInExcludeList( LPCTSTR lpFile)
{
	bool	bFound = false;

	LPCTSTR lpFileName = wcsrchr(lpFile, '\\');

	if (!lpFileName)
		return bFound;

	lpFileName++;

	if (m_cslExcludeFiles.Find(lpFileName))
	{
		bFound = true;
	}

	return bFound;
}

/***************************************************************************************************
*  Function Name  :  WindowProc
*  Description    :  Used to implement WindowProc
*  Author Name    :  Yogeshwar Rasal
*  Date           :	 2 June 2016
****************************************************************************************************/
LRESULT CMotherBoardDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	
	lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
	if (bHandled)      // if it was handled by the Sciter
		return lResult; // then no further processing is required.

	return CJpegDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  :  On_StartVaccineProcess
*  Description    :  Starts USB Vaccination and File Unhiding Process 
*  Author Name    :	 Yogeshwar Rasal
*  Date           :  31st May 2016
****************************************************************************************************/
json::value CMotherBoardDlg::On_StartVaccineProcess(SCITER_VALUE svBoolUSBVaccin, SCITER_VALUE svFilePath, SCITER_VALUE svNotificationMsgCB)
{
	try
	{
		m_svVaccineStatusFnCB = svNotificationMsgCB;
		bool b_USBVac = svBoolUSBVaccin.get(false);
		csTypeSelected = L"";

		if (TRUE == b_USBVac)
		{
			m_iUSBVaccine = 0x01;
			m_iUnhideFolderOption = 0x00;
			csTypeSelected = "USBVACCINE";
		}
		else if (FALSE == b_USBVac)
		{
			m_iUnhideFolderOption = 0x01;
			m_iUSBVaccine = 0x00;
			m_csEditPath = svFilePath.get(L"").c_str();
			csTypeSelected = "HIDEUNHIDEFOLDERS";
		}

		// Add entries into Database..
		//CString csInsertQuery = _T("INSERT INTO Wardwiz_USBVaccinatorSessionDetails VALUES (null,");

		//csInsertQuery.Format(_T("INSERT INTO Wardwiz_USBVaccinatorSessionDetails VALUES (null,Datetime('now','localtime'),Datetime('now','localtime'),Date('now'),Date('now'),%d,%d,'%s','%s');"), m_dwScannedFiles, m_dwUnhideFiles, csTypeSelected,m_csEditPath);

		//CT2A ascii(csInsertQuery, CP_UTF8);

		////CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
		////objSqlDb.Open();
		////objSqlDb.CreateWardwizSQLiteTables();
		////objSqlDb.Close();

		//m_iUSBSessionId = InsertDataToTable(ascii.m_psz);
		OnBnClickedButtonOk();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::On_StartVaccineProcess, File Path: %s", svFilePath.get(L"").c_str(), 0, true, SECONDLEVEL);
	}
  return json::value(m_bProcessStarted);
}

/***************************************************************************************************
*  Function Name  :  On_Close
*  Description    :  Used to Close a Dialog.
*  Author Name    :	 Yogeshwar Rasal
*  Date           :  31st May 2016
****************************************************************************************************/
json::value CMotherBoardDlg::On_Close()
{
		try
		{
			OnBnClickedButtonClose();
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CMotherBoardDlg::On_Close", 0, 0, true, SECONDLEVEL);
		}
     return 0;
}

 /***************************************************************************************************
 *  Function Name  :  On_Minimize
 *  Description    :  Used to Minimize a Dialog. 
 *  Author Name    :  Yogeshwar Rasal
 *  Date           :  31st May 2016
 ****************************************************************************************************/
json::value CMotherBoardDlg::On_Minimize()
 {
	 try
	 {
		 OnBnClickedButtonMinimize();
	 }
	 catch (...)
	 {
		 AddLogEntry(L"### Exception in CMotherBoardDlg::On_Minimize",0, 0, true, SECONDLEVEL);
	 }
	 return json::value(0);
 }

/***************************************************************************************************
*  Function Name  :  On_SelectFileFolder
*  Description    :  Used to select File/Folder to Unhide.
*  Author Name    :  Yogeshwar Rasal
*  Date           :  31st May 2016
****************************************************************************************************/
 json::value CMotherBoardDlg::On_SelectFileFolder()
 {
	 try
	 {
		 OnBnClickedButtonBrowsefolder();
	 }
	 catch (...)
	 {
		 AddLogEntry(L"### Exception in CMotherBoardDlg::On_SelectFileFolder", 0, 0, true, SECONDLEVEL);
	 }
	return json::value((SCITER_STRING)m_csEditPath);
 }

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Nitin K
*  Date			  : 1 July 2016
****************************************************************************************************/
json::value CMotherBoardDlg::On_GetProductID()
{
	 int iProdValue = 0;
	 try
	 {
		 iProdValue = theApp.m_dwProdID;
	 }
	 catch (...)
	 {
		 AddLogEntry(L"### Exception in CMotherBoardDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	 }
	 return json::value(iProdValue);
}

/***************************************************************************************************
*  Function Name  : OnBnClickedNo
*  Description    : Function  on No button clicked in sciter.
*  Author Name    : Yogeshwar Rasal
*  Date			  : 24th Aug. 2016
****************************************************************************************************/
json::value CMotherBoardDlg::OnBnClickedNo()
{
	try
	{
		if (m_bClickedNo)
		{
			if (m_hThreadStartScanForUnhide != NULL)
			{
				::ResumeThread(m_hThreadStartScanForUnhide);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::OnBnClickedNo", 0, 0, true, SECONDLEVEL);
	}
   return json::value(m_bProcessStarted);
}

/***************************************************************************************************
*  Function Name  : OnPauseThread
*  Description    : Function on close button to suspend thread.
*  Author Name    : Yogeshwar Rasal
*  Date			  : 24th Aug. 2016
****************************************************************************************************/
json::value CMotherBoardDlg::OnPauseThread()
{
	try
	{
		m_objSqlDb.Close();

		if (m_hThreadStartScanForUnhide != NULL)
		{
			::SuspendThread(m_hThreadStartScanForUnhide);
		}
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::OnPauseThread", 0, 0, true, SECONDLEVEL);
	}
   return 0;
}


json::value CMotherBoardDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			//theApp.g_lbRetval = sciter::value();
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;
			theApp.m_objCompleteEvent.SetEvent();
			//::WaitForSingleObject(theApp.g_lbCompleteEvent, INFINITE);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
void CMotherBoardDlg::CallNotificationMessage(SCITER_STRING strMessageString)
{
	try
	{
		m_svVaccineStatusFnCB.call((SCITER_STRING)strMessageString);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CMotherBoardDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : SelectedFilesFolders
Description    : Used to Unhide selected Folder
Author Name    : 
SR_NO		   :
Date           : 23rd Dec 2016
/***************************************************************************************************/

DWORD CMotherBoardDlg::SelectedFilesFolders(LPCTSTR pszDirPath)
{
	DWORD	dwFileAttributes = 0x00;
	try
	{
		if (PathFileExists(pszDirPath))
		{
			dwFileAttributes = GetFileAttributes(pszDirPath);
			if (_tcslen(pszDirPath) == 2) //the length of drive with : (colon) // to avoid issue:3770
			{
				return 0;
			}
			else
			if (FILE_ATTRIBUTE_HIDDEN == (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				SetFileAttributes(pszDirPath, FILE_ATTRIBUTE_NORMAL);
				m_dwScannedFiles++;
				m_dwUnhideFiles++;

				//Add entry to the database..
				EnterHideUnhideDetails(pszDirPath, m_iUSBSessionId);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SelectedFilesFolders Filepath: %s",pszDirPath, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Jeena Mariam Saji
*  Date			  : 04 Dec 2018
****************************************************************************************************/
json::value CMotherBoardDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : FunCheckInternetAccessBlock
*  Description    : To check internet access block
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 Dec 2019
****************************************************************************************************/
json::value CMotherBoardDlg::FunCheckInternetAccessBlock()
{
	bool RetVal = false;
	try
	{
		DWORD dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
		if (dwProductID == BASIC || dwProductID == ESSENTIAL)
		{
			return false;
		}

		CString csRegKeyVal;
		csRegKeyVal = CWWizSettingsWrapper::GetProductRegistryKey();
		CITinRegWrapper objReg;
		DWORD dwParentalControl = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyVal.GetBuffer(), L"dwParentalCntrlFlg", dwParentalControl) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CMotherBoardDlg::FunCheckInternetAccessBlock", 0, 0, true, SECONDLEVEL);
		}

		if (dwParentalControl == 1)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = ON_CHECK_INTERNET_ACCESS;

			CISpyCommunicator objCom(SERVICE_SERVER);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send Data in CMotherBoardDlg::SendData", 0, 0, true, SECONDLEVEL);
			}

			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read Data in CMotherBoardDlg::ReadData", 0, 0, true, SECONDLEVEL);
			}

			DWORD dwVal = szPipeData.dwValue;
			if (dwVal == 0x01)
			{
				RetVal = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMotherBoardDlg::FunCheckInternetAccessBlock()", 0, 0, true, SECONDLEVEL);
	}
	return RetVal;
}