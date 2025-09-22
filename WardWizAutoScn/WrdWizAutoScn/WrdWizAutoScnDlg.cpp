/**********************************************************************************************************
Program Name          : WrdWizAutoScnDlg.cpp
Description           : This class contains the functionality for auto run scanner which scan for .lnk virus files and repairs them.
It has 2 options      : a) Find Virus Infected file(s)  b) Reapir file(s).
Author Name           : 
Date Of Creation      : 13nd June 2016
Version No            : 2.0.0.17
***********************************************************************************************************/

// WrdWizAutoScnDlg.cpp : implementation file
//


#include "stdafx.h"
#include "WrdWizAutoScn.h"
#include "WrdWizAutoScnDlg.h"
#include "afxdialogex.h"

#include <Shlwapi.h>
#include <Imagehlp.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include "CSecure64.h"
#include "DriverConstants.h"
#include "CScannerLoad.h"

char* g_strDatabaseFilePath = ".\\VBALLREPORTS.DB";

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif

#define SETFILEPATH_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 9)

// CWrdWizAutoScnDlg dialog

CStringList m_strListProcesses;

DWORD WINAPI Thread_StartScan(LPVOID lpParam);
DWORD WINAPI Thread_RepairVirusFiles(LPVOID lpParam);

//Added on 02 / 09 /2015 for Progress bar
DWORD WINAPI Thread_GetFilesCount(LPVOID lpParam);

CWrdWizAutoScnDlg::CWrdWizAutoScnDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWrdWizAutoScnDlg::IDD, pParent),
	m_bPause_Resume(true)
	, m_dwDetectedFiles(0x00)
	, m_objSqlDb(g_strDatabaseFilePath)
	, m_hFileHandle(INVALID_HANDLE_VALUE)
	, m_bClose(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strListProcesses.RemoveAll();
}

/***************************************************************************************************
*  Class Name     :  CWrdWizAutoScnDlg
*  Description    :  This is Destructure to free resorces(memory).
*  Author Name    :	 Amol J. 	
*  Date           :
****************************************************************************************************/
CWrdWizAutoScnDlg::~CWrdWizAutoScnDlg()
{
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Data Exchange function.
*  Author Name    : 
*  Date           : 13 June 2016
****************************************************************************************************/

void CWrdWizAutoScnDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_GENERATE_CACHE, m_generateCache);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_Close);
	DDX_Control(pDX, IDC_STATIC_CURRENT_FILE, m_CurrentFile);
	DDX_Control(pDX, IDC_LIST_FILES, m_FilesList);
	DDX_Control(pDX, IDC_CHECK_SELECT_ALL, m_SelectAll);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, m_Browse);
	DDX_Control(pDX, IDC_EDIT_SPECIFIC_PATH, m_SpecificPath);
	DDX_Control(pDX, IDC_RADIO_WHOLE_SYSTEM, m_Radio_AllDrives);
	DDX_Control(pDX, IDC_RADIO_SPECIFIC_PATH, m_Radio_SpecificPath);
	DDX_Control(pDX, IDC_STATIC_START_TIME, m_StartTime);
	DDX_Control(pDX, IDC_STATIC_END_TIME, m_EndTime);
	DDX_Control(pDX, IDC_STATIC_SELECT_SCAN_PATH, m_stSelectScanPath);
	DDX_Control(pDX, IDC_BUTTON_AUTORUN_MINIMIZE, m_btnAutorunminimize);
	DDX_Control(pDX, IDC_BUTTON_AUTORUN_CLOSE, m_btnAutoRunClose);
	DDX_Control(pDX, IDC_STATIC_SELECTALL, m_stSelectAll);
	DDX_Control(pDX, IDC_STATIC_AUTORUN_FOOTER, m_stAutorunFooter);
	DDX_Control(pDX, IDC_STATIC_ALL_DRIVE_TEXT, m_stAllDriveText);
	DDX_Control(pDX, IDC_STATIC_SELECT_PATH_TEXT, m_stSelectDrivetext);
	DDX_Control(pDX, IDC_STATIC_AUTO_HEADER, m_stAutoHeader);
	DDX_Control(pDX, IDC_PROGRESS1, m_prgScanProgress);
	DDX_Control(pDX, IDC_STATIC_THREAT_FOUND_COUNT, m_stThreatCount);
}

BEGIN_MESSAGE_MAP(CWrdWizAutoScnDlg, CJpegDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWrdWizAutoScnDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWrdWizAutoScnDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_CACHE, &CWrdWizAutoScnDlg::OnBnClickedButtonGenerateCache)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CWrdWizAutoScnDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CWrdWizAutoScnDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_RADIO_WHOLE_SYSTEM, &CWrdWizAutoScnDlg::OnBnClickedRadioWholeSystem)
	ON_BN_CLICKED(IDC_RADIO_SPECIFIC_PATH, &CWrdWizAutoScnDlg::OnBnClickedRadioSpecificPath)
	ON_BN_CLICKED(IDC_CHECK_SELECT_ALL, &CWrdWizAutoScnDlg::OnBnClickedCheckSelectAll)
	ON_EN_CHANGE(IDC_EDIT_SPECIFIC_PATH, &CWrdWizAutoScnDlg::OnEnChangeEditSpecificPath)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_AUTORUN_MINIMIZE, &CWrdWizAutoScnDlg::OnBnClickedButtonAutorunMinimize)
	ON_BN_CLICKED(IDC_BUTTON_AUTORUN_CLOSE, &CWrdWizAutoScnDlg::OnBnClickedButtonAutorunClose)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


HWINDOW   CWrdWizAutoScnDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWrdWizAutoScnDlg::get_resource_instance() { return theApp.m_hInstance; }
// CWrdWizAutoScnDlg message handlers

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Function which initializes dialog.
*  Author Name    :
*  Date           : 13 June 2016
****************************************************************************************************/

BOOL CWrdWizAutoScnDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//AddUserAndSystemInfoToLog();
	m_iRepairFileCount = 0;
	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_FOUR);  // to register service for process protection

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_FOUR);//WLSRV_ID_ZERO to register service for process protection

	ModifyStyle(0, WS_SYSMENU | WS_MINIMIZEBOX);


	HideAllElements();

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_AUTORUN_SCAN.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_AUTORUN_SCAN.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	CenterWindow();

	CString	csWardWizModulePath = GetWardWizPathFromRegistry();
	CString	csWardWizReportsPath = L"";
	csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);

	if (!PathFileExists(csWardWizReportsPath))
	{
		m_objSqlDb.Open();
		m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
		m_objSqlDb.Close();
	}
	this->SetWindowText(L"Vibranium AUTORUN Manager");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 CWrdWizAutoScnDlg::InsertDataToTable(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable in VibraniumAutoScnDlg- AutoScanner entered", 0, 0, true, ZEROLEVEL);
	try
	{
		m_objSqlDb.Open();

		int iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(50);
		INT64 iLastRowIndex = m_objSqlDb.GetLastRowId();

		m_objSqlDb.Close();

		return iLastRowIndex;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}

	return 0;
}


/***************************************************************************************************
*  Function Name  : On_OnBnClickedAutoRunScan
*  Description    : Function to start scanning automatically
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date			  : 3 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_OnBnClickedAutoRunScan(SCITER_VALUE svbAutoRunScan, SCITER_VALUE svNewPath, SCITER_VALUE svAddAutoRunScanFoundEntriesCB, SCITER_VALUE svEmptyFolderNotificationCB)
{
	TCHAR	szSpecificPath[512] = { 0 };
	TCHAR	szTemp[512] = { 0 };
	//For issue:0000823
	//Added by Vilas on 17 - 12 - 2015
	m_svbAutoRunScan = svbAutoRunScan;
	m_svNewPath = svNewPath;
	m_svAddAutoRunScanFoundEntriesCB = svAddAutoRunScanFoundEntriesCB;
	m_svEmptyFolderNotificationCB = svEmptyFolderNotificationCB;
	bool b_AutoRunScan = svbAutoRunScan.get(false);
	m_bIsTempFileFoundFinished = false;
	if (TRUE == b_AutoRunScan)
	{
		m_bAllDrives = true;
	}
	else 
	{
		m_bAllDrives = false;
		const std::wstring  chFilePath = svNewPath.get(L"");
	    wcscpy_s(m_szSpecificPath, 511, chFilePath.c_str());
	}

	m_bFileCountObtained = false;
	m_dwTotalFiles = 0x00;
	m_bScanStarted = true;
	m_bScanStart = true;
	m_bClose = false;
	m_dwDetectedFiles = 0x00;
	m_dwRepairedFiles = 0x00;
	m_dwRebootFileDeletion = 0x00;

	//Total Files Count for Progress bar
	//Added on 02 / 09 / 2015
	m_hThreadFilesCount = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_GetFilesCount,
		(LPVOID)this, 0, 0);
	Sleep(500);

	// Add entries into Database..

	CString csInsertQuery = _T("INSERT INTO Wardwiz_AutorunScanSessionDetails VALUES (null,");

	csInsertQuery.Format(_T("INSERT INTO Wardwiz_AutorunScanSessionDetails VALUES (null,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),%d,%d,%d );"), m_dwDetectedFiles, m_dwTotalFiles, m_dwRepairedFiles);

	CT2A ascii(csInsertQuery, CP_UTF8);

	m_iScanSessionId = InsertDataToTable(ascii.m_psz);

	m_hThreadStartScan = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_StartScan,
		(LPVOID)this, 0, 0);
	Sleep(500);
	if (!m_hThreadStartScan)
	{
		m_bScanStarted = false;
		m_bScanStart = false;
		AddLogEntry(L"thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


/***************************************************************************************************
*  Function Name  : On_OnBnClickedAutoRunPause_Resume
*  Description    : Function to pause or resume scanning
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date			  : 4 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_OnBnClickedAutoRunPause_Resume()
{
	m_objSqlDb.Close();

	TCHAR	szSpecificPath[512] = { 0 };

	if (m_bPause_Resume == true)
	{
		m_bPause_Resume = false;

		if (m_bScanStarted)
		{
			if (m_hThreadFilesCount != NULL)
			{
				::SuspendThread(m_hThreadFilesCount);
			}

			if (m_hThreadStartScan != NULL)
			{
				::SuspendThread(m_hThreadStartScan);
			}
		}

		if (m_bRepairStart)
		{
			::SuspendThread(m_hThreadRepair);
		}

		return 0;
	}
	else
	//if (_wcsnicmp(szSpecificPath, theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"), 0x06) == 0x00)
	{
		m_bPause_Resume = true;

		if (m_bScanStarted)
		{
			if (m_hThreadFilesCount != NULL)
			{
				::ResumeThread(m_hThreadFilesCount);
			}
			
			if (m_hThreadStartScan != NULL)
			{
				::ResumeThread(m_hThreadStartScan);
			}
		}

		if (m_bRepairStart)
		{
			::ResumeThread(m_hThreadRepair);
		}

		return 0;
	}
}


/***************************************************************************************************
*  Function Name  : On_OnBnClickedAutoRunRepair
*  Description    : Function to repair the damaged file
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date			  : 5 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_OnBnClickedAutoRunRepair(SCITER_VALUE svrecords, SCITER_VALUE setUpdateVirusFoundentriesCB, SCITER_VALUE svUpdateRepairedFilesStatusCB)
{
	m_bIsTempFileFoundFinished = false;  //when click on repair button it should be false
	TCHAR	szSpecificPath[512] = { 0 };
	try
	{
		bool bIsArray = false;
		svrecords.isolate();
		bIsArray = svrecords.is_array();

		if (!bIsArray)
		{
			return false;
		}
		m_bRepairStart = true;

		m_setUpdateVirusFoundentriesCB = setUpdateVirusFoundentriesCB;
		m_svrecords = svrecords;
		m_svUpdateRepairedFilesStatusCB = svUpdateRepairedFilesStatusCB;
		
		//m_Close.EnableWindow(FALSE);
		m_hThreadRepair = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_RepairVirusFiles,
			(LPVOID)this, 0, 0);
		Sleep(1000);
	}
	catch (...)
	{ 
		if (!m_hThreadRepair)
		{
			m_bRepairStart = false;
			AddLogEntry(L"thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
			//::MessageBox(this->m_hWnd, L"Thread creation failed, please check memory usage.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			//m_generateCache.EnableWindow();
			//m_Close.EnableWindow();
		}
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Nitin K
*  Date			  : 1 July 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProdID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}

	return iProdValue;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Amol Jaware
*  Date			  : 5 Aug 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_GetLanguageID()
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

/***************************************************************************************************
*  Function Name  : HideAllElements
*  Description    : Function to hide all elements applied when not scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 4 May 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::HideAllElements()
{
	m_stAutoHeader.ShowWindow(SW_HIDE);
	m_stSelectScanPath.ShowWindow(SW_HIDE);
	m_Radio_AllDrives.ShowWindow(SW_HIDE);
	m_stAllDriveText.ShowWindow(SW_HIDE);
	m_Radio_SpecificPath.ShowWindow(SW_HIDE);
	m_stSelectDrivetext.ShowWindow(SW_HIDE);
	m_SpecificPath.ShowWindow(SW_HIDE);
	m_Browse.ShowWindow(SW_HIDE);
	m_btnAutorunminimize.ShowWindow(SW_HIDE);
	m_btnAutoRunClose.ShowWindow(SW_HIDE);
	m_FilesList.ShowWindow(SW_HIDE);
	m_CurrentFile.ShowWindow(SW_HIDE);
	m_StartTime.ShowWindow(SW_HIDE);
	m_EndTime.ShowWindow(SW_HIDE);
	m_stThreatCount.ShowWindow(SW_HIDE);
	m_prgScanProgress.ShowWindow(SW_HIDE);
	m_Close.ShowWindow(SW_HIDE);
	m_generateCache.ShowWindow(SW_HIDE);
	m_stAutorunFooter.ShowWindow(SW_HIDE);
	m_SelectAll.ShowWindow(SW_HIDE);
	m_stSelectAll.ShowWindow(SW_HIDE);

}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    : Function for setting window style and behaviour.
*  Author Name    : 
*  SR_NO		  :
*  Date			  : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnPaint()
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
		CJpegDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
/***************************************************************************************************
*  Function Name  : OnQueryDragIcon
*  Description    : function to obtain the cursor to display while the user drags
                    the minimized window.
*  Author Name    :
*  SR_NO		  :
*  Date			  : 14 June 2016
****************************************************************************************************/
HCURSOR CWrdWizAutoScnDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/***************************************************************************************************
*  Function Name  : OnBnClickedOk
*  Description    : function to perform ok operation while message box appears on the screen. 
*  Author Name    :
*  SR_NO		  :
*  Date			  : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnOK();
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCancel
*  Description    : function to perform cancel operation while message box appears on the screen.
*  Author Name    :
*  SR_NO		  :
*  Date			  : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedCancel()
{
	//FreeUsedResources();
	OnCancel();
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonGenerateCache
*  Description    : function to generate cache before scanning for viruses begins.
*  Author Name    :
*  SR_NO		  :
*  Date			  : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg :: OnBnClickedButtonGenerateCache()
{
	TCHAR	szSpecificPath[512] = { 0 };

	//For issue:0000823
	//Added by Vilas on 17 - 12 - 2015
	//m_bScanStart = false;
	m_bRepairStart = false;
	m_generateCache.EnableWindow(FALSE);

	//m_dwTotalFiles = 0x00;
	//m_bFileCountObtained = false;

	ZeroMemory(szSpecificPath, sizeof(szSpecificPath));
	m_generateCache.GetWindowTextW(szSpecificPath, 510);
	if (_wcsicmp(szSpecificPath, theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN")) == 0x00)
	{
		//Added on 02 / 09 /2015 for Progress bar
		m_prgScanProgress.SetRange(0, 100);
		m_prgScanProgress.SetPos(0);

		if (m_Radio_SpecificPath.GetCheck() == BST_CHECKED)
		{
			ZeroMemory(szSpecificPath, sizeof(szSpecificPath));
			m_SpecificPath.GetWindowTextW(szSpecificPath, 511);
			if (!PathFileExists(szSpecificPath))
			{
				//AfxMessageBox(L"Please enter valid file or folder");
				::MessageBox(this->m_hWnd, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_SELECT_VALID_FILE_FOLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				m_generateCache.EnableWindow();
				return;
			}

			if ((GetDriveType(szSpecificPath) == DRIVE_CDROM) ||
				(GetDriveType(szSpecificPath) == DRIVE_REMOTE))
			{
				::MessageBox(this->m_hWnd, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_SELECT_LOCAL_DRIVE_PATH"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				m_SpecificPath.SetWindowText(L"");
				m_SpecificPath.SetFocus();
				m_generateCache.EnableWindow();
				return;
			}

			m_bAllDrives = false;
		}

	}

	ZeroMemory(szSpecificPath, sizeof(szSpecificPath));
	m_generateCache.GetWindowTextW(szSpecificPath, 510);

	//For issue:0000823
	//Added by Vilas on 17 - 12 - 2015
	if (_wcsnicmp(szSpecificPath, theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"), 0x05) == 0x00)
	{
		m_objSqlDb.Close();

		if (m_bScanStarted)
		{
			if (m_hThreadFilesCount)
			{
				::SuspendThread(m_hThreadFilesCount);
			}

			if (m_hThreadStartScan)
			{
				::SuspendThread(m_hThreadStartScan);
			}
		}

		if (m_bRepairStart)
		{
			::SuspendThread(m_hThreadRepair);
		}

		m_generateCache.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"));
		m_generateCache.EnableWindow();

		return;
	}

	if (_wcsnicmp(szSpecificPath, theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"), 0x06) == 0x00)
	{
		if (m_bScanStarted)
		{
			if (m_hThreadFilesCount != NULL)
			{
				::ResumeThread(m_hThreadFilesCount);
			}

			if (m_hThreadStartScan != NULL)
			{
				::ResumeThread(m_hThreadStartScan);
			}
		}

		if (m_bRepairStart)
		{
			::ResumeThread(m_hThreadRepair);
		}

		m_generateCache.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
		m_generateCache.EnableWindow();

		return;
	}

	if (_wcsnicmp(szSpecificPath, theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"), 0x04) == 0x00)
	{
		//For issue:0000823
		//Added by Vilas on 17 - 12 - 2015
		m_dwTotalFiles = 0x00;
		m_bFileCountObtained = false;

		m_bScanStarted = true;
		m_bScanStart = true;
		m_bClose = false;
		m_Close.EnableWindow(FALSE);

		m_Browse.EnableWindow(FALSE);
		m_prgScanProgress.SetRange(0, 100);
		m_prgScanProgress.SetPos(0);

		//Total Files Count for Progress bar
		//Added on 02 / 09 / 2015
		m_hThreadFilesCount = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_GetFilesCount,
			(LPVOID)this, 0, 0);
		Sleep(1000);

		m_hThreadStartScan = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_StartScan,
			(LPVOID)this, 0, 0);
		Sleep(1000);
		if (!m_hThreadStartScan)
		{
			m_bScanStarted = false;
			m_bScanStart = false;
			AddLogEntry(L"thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
			//::MessageBox(this->m_hWnd, L"Thread creation failed, please check memory usage.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_generateCache.EnableWindow();
			m_Close.EnableWindow();
		}
	}

	if (_wcsnicmp(szSpecificPath, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_REPAIR"), 0x04) == 0x00)
	{
		m_bRepairStart = true;
		m_Close.EnableWindow(FALSE);
		m_hThreadRepair = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_RepairVirusFiles,
			(LPVOID)this, 0, 0);
		Sleep(1000);
		if (!m_hThreadRepair)
		{
			m_bRepairStart = false;
			AddLogEntry(L"thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
			//::MessageBox(this->m_hWnd, L"Thread creation failed, please check memory usage.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_generateCache.EnableWindow();
			m_Close.EnableWindow();
		}
	}

}


/***************************************************************************************************
*  Function Name  : OnBnClickedButtonClose
*  Description    : function to perform close operation on window.
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonClose()
{
	TCHAR	szTemp[512] = { 0 };
	SCITER_VALUE svRet = 0;

	m_objSqlDb.Close();

	if (m_bIsRequestFromTaskBar)
	{
		if (m_bScanStarted)
		{
			if (m_bScanStart)
			{
				if (m_hThreadStartScan != NULL)
				{
					if (m_hThreadFilesCount != NULL)
					{
						::SuspendThread(m_hThreadFilesCount);
					}

					if (m_hThreadStartScan != NULL)
					{
						::SuspendThread(m_hThreadStartScan);
					}

					CallNotificationMessage(2, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_INPROGRESS"));
					if (theApp.m_bRetval == true)
					{
						m_bClose = true;
						svRet = 1;
					}
					else
					{
						svRet = 0;
					}
				}
			}
			if (m_bRepairStart)
			{
				if (m_hThreadRepair != NULL)
				{
					::SuspendThread(m_hThreadRepair);
					//svRet = m_svEmptyFolderNotificationCB.call(3, "Repair is in progress. Do you want to close?");
					CallNotificationMessage(3, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_REPAIR_INPROGRESS"));
					if (theApp.m_bRetval == true)
					{
						m_bClose = true;
						svRet = 1;
					}
					else
					{
						svRet = 0;
					}
				}
			}
			if (svRet == 0)
			{
				if (m_bScanStart)
				{
					if (m_hThreadFilesCount != NULL)
					{
						::ResumeThread(m_hThreadFilesCount);
					}

					if (m_hThreadStartScan != NULL)
					{
						::ResumeThread(m_hThreadStartScan);
					}
				}

				if (m_bRepairStart)
				{
					if (m_hThreadRepair != NULL)
					{
						::ResumeThread(m_hThreadRepair);
					}
				}
				m_bIsRequestFromTaskBar = false;
				return;
			}
			else
			{
				if (m_hThreadFilesCount != NULL)
				{
					::TerminateThread(m_hThreadFilesCount, 0x00);
					m_hThreadFilesCount = NULL;
				}

				if (m_hThreadStartScan != NULL)
				{
					::ResumeThread(m_hThreadStartScan);
					DWORD dwWaitTimeout = 3 * 1000; //3 seconds
					::WaitForSingleObject(m_hThreadStartScan, dwWaitTimeout);
					m_hThreadStartScan = NULL;
				}
				if (m_hThreadRepair != NULL)
				{
					::TerminateThread(m_hThreadRepair, 0x00);
					m_hThreadRepair = NULL;
				}

				UpdateSessionRecord();
				OnCancel();
			}
		}
		else
		{
			if (m_ListIndex)
			{
				if (m_bScanStart)
				{
					if (m_hThreadFilesCount != NULL)
					{
						::TerminateThread(m_hThreadFilesCount, 0x00);
						CloseHandle(m_hThreadFilesCount);
						m_hThreadFilesCount = NULL;
					}

					if (m_hThreadStartScan != NULL)
					{
						::ResumeThread(m_hThreadStartScan);
						DWORD dwWaitTimeout = 3 * 1000; //3 seconds
						::WaitForSingleObject(m_hThreadStartScan, dwWaitTimeout);
						m_hThreadStartScan = NULL;
					}

					m_bScanStarted = m_bScanStart = false;
				}

				UpdateSessionRecord();
				return;
			}
		}
	}
	//Added on 19 - 12 - 2015 by Vilas
	//Close button should not close main GUI directly
	if (m_bIsTempFileFoundFinished == true || m_bISRepairFile == true)
	{
		//svRet = m_svEmptyFolderNotificationCB.call(4, L"Threat(s) are detected. Do you want to close?");
		CallNotificationMessage(4, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_THREATS_DETECTED"));
		if (theApp.m_bRetval == true)
		{
			svRet = 1;
		}
		else
		{
			svRet = 0;
		}
		if (svRet == 1)
		{
			UpdateSessionRecord();
			OnCancel();
			return;
		}
		else
			return;
	}

	if (m_bScanStarted)
	{
		m_bScanStarted = m_bScanStart = false;
	}
	OnCancel();
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonBrowse
*  Description    : function for browse operation while select path radio button is checked
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonBrowse()
{
	TCHAR	pszPath[512] = { 0 };

	BROWSEINFO        bi = { m_hWnd, NULL, pszPath, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_SELECT_FILE_FOR_SCAN"), BIF_RETURNONLYFSDIRS | BIF_BROWSEINCLUDEFILES, NULL, 0L, 0 };
	LPITEMIDLIST      pIdl = NULL;

	LPITEMIDLIST  pRoot = NULL;
	bi.pidlRoot = pRoot;

	pIdl = SHBrowseForFolder(&bi);
	if (NULL != pIdl)
	{
		SHGetPathFromIDList(pIdl, pszPath);

		if ((GetDriveType(pszPath) == DRIVE_CDROM) ||
			(GetDriveType(pszPath) == DRIVE_REMOTE))
		{
			::MessageBox(this->m_hWnd, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_SELECT_LOCAL_DRIVE_PATH"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_SpecificPath.SetWindowText(L"");
			m_SpecificPath.SetFocus();
			return;
		}


		if (pszPath[0])
		{
			wcscpy_s(m_szSpecificPath, 511, pszPath);
			m_SpecificPath.SetWindowText(pszPath);
			m_SpecificPath.SetFocus();
		}
	}

	m_generateCache.SetWindowTextW(L"Scan");
}


void CWrdWizAutoScnDlg::InitializeAllVariablesToZero()
{
	ZeroMemory(m_ApplRunningPath, sizeof(m_ApplRunningPath));
	ZeroMemory(m_szLogPath, sizeof(m_szLogPath));
	ZeroMemory(m_szLogComparePath, sizeof(m_szLogComparePath));
	ZeroMemory(m_szSpecificPath, sizeof(m_szSpecificPath));
	ZeroMemory(m_SysDir, sizeof(m_SysDir));

	
	ZeroMemory(m_szComputerName, sizeof(m_szComputerName));

	m_hHashDLL = NULL;
	m_hThreadStartScan = NULL;
	m_hThreadRepair = NULL;
	m_GenerateStarted = false;
	m_bAllDrives = true;

	m_dwScannedFiles = 0x00;
	m_dwDetectedFiles = 0x00;
	m_dwRepairedFiles = 0x00;
	m_dwFailed = 0x00;
	m_dwRebootFileDeletion = 0x00;
	m_dwScanStartTickCount = 0x00;

	m_FilesList.DeleteAllItems();
	m_bScanStart = false;
	m_bRepairStart = false;
	m_bRemovDriveRemoved = false;
	//m_strListProcesses.RemoveAll();

	//Added on 02 / 09 /2015 for Threat(s) count
	m_dwTotalFiles = 0x00;
	m_hThreadFilesCount = NULL;

	m_stThreatCount.ShowWindow(FALSE);
	CString strText = L"";
	strText.Format(L"%s 0", theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_THREAT_COUNT"));
	m_stThreatCount.SetWindowTextW(strText);

	//Added on 20 / 10 /2015 for Correct status of Progress bar
	m_bFileCountObtained = false;
}

/***************************************************************************************************
*  Function Name  : LoadRequiredModules
*  Description    : function to load required modules while startup of application
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
bool CWrdWizAutoScnDlg::LoadRequiredModules()
{

	DWORD	dwSize = 0x7F;

	GetComputerName(m_szComputerName, &dwSize);
	if (m_szComputerName[0])
		_wcslwr_s(m_szComputerName, 0x7F);

	GetSystemDirectory(m_SysDir, 255);
	if (m_SysDir[0])
		_wcsupr_s(m_SysDir, 255 );

	GetModuleFileName(NULL, m_ApplRunningPath, sizeof(m_ApplRunningPath) - 2);

	TCHAR	*pFileName = wcsrchr(m_ApplRunningPath, '\\');

	if (!pFileName)
		return false;

	*pFileName = '\0';

	swprintf_s(m_szLogPath, 511, L"%s\\log\\VIBOAUTORUNSCN.LOG", m_ApplRunningPath);

	return true;
}

/***************************************************************************************************
*  Function Name  : GenerateProcessKillList
*  Description    : function to generate process kill list during close function
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::GenerateProcessKillList()
{
	TCHAR	szPath[512] = { 0 };

	swprintf_s(szPath, 511, L"%s\\WSCRIPT.EXE", m_SysDir);
	m_strListProcesses.AddTail(szPath);
}

/***************************************************************************************************
*  Function Name  : KillSpecifiedRunningProcesses
*  Description    : function to kill specific process from the list
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
DWORD CWrdWizAutoScnDlg::KillSpecifiedRunningProcesses()
{
	DWORD	dwPID[0x100] = { 0 };
	DWORD	dwProcesses = 0x00, dwTempPerc = 0x00, i = 0x00;
	int		dwPercentage = 0x00;

	TCHAR	szProcPath[1024] = { 0 };
	TCHAR	szProcName[256] = { 0 };
	TCHAR	*pProcName = NULL;

	try
	{

		m_CurrentFile.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_SCANNING"));

		EnumProcesses(dwPID, 0x400, &dwProcesses);
		dwProcesses = dwProcesses / sizeof(DWORD);

		for (i = 0x00; i<dwProcesses; i++)
		{

			HMODULE		hMods[1024] = { 0 };
			HANDLE		hProcess = NULL;

			DWORD		dwModules = 0x00;

			memset(szProcPath, 0x00, 1024);
			memset(szProcName, 0x00, 256);

			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SUSPEND_RESUME | PROCESS_TERMINATE,
				FALSE, dwPID[i]);
			if (!hProcess)
				continue;

			if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules))
			{
				DWORD error = GetLastError();
				CloseHandle(hProcess);
				continue;
			}

			GetModuleFileNameEx(hProcess, hMods[0], szProcPath, 1023);
			if (!szProcPath[0])
			{
				CloseHandle(hProcess);
				continue;
			}

			_wcsupr_s(szProcPath);

			if (m_strListProcesses.Find(szProcPath))
			{
				bool bRunningFound = false;

				SuspendProcessThreadsForcefully(dwPID[i], bRunningFound);
				if (TerminateProcess(hProcess, 0x00))
				{
					//Process Terminated successfully
				}
				else
				{
					////Process Termination failed
				}
			}

			CloseHandle(hProcess);
			Sleep(10);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::KillSpecifiedRunningProcesses", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : SuspendProcessThreadsForcefully
*  Description    : function to suspend thread forcefully while close is performed. 
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
DWORD CWrdWizAutoScnDlg::SuspendProcessThreadsForcefully(DWORD dwPID, bool &bRunningFound)
{

	DWORD					dwRet = 0x00;
	HANDLE					hThreadSnap = NULL;

	THREADENTRY32			tie = { 0 };

	try
	{

		hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPID);
		if (hThreadSnap == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		tie.dwSize = sizeof(THREADENTRY32);
		if (!Thread32First(hThreadSnap, &tie))
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		do
		{
			if (tie.th32OwnerProcessID == dwPID)
			{
				HANDLE hThread = ::OpenThread(THREAD_QUERY_INFORMATION, FALSE, tie.th32ThreadID);
				if (hThread != INVALID_HANDLE_VALUE)
				{
					::SuspendThread(hThread);
					::CloseHandle(hThread);
					hThread = INVALID_HANDLE_VALUE;
				}
			}

		} while (Thread32Next(hThreadSnap, &tie));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::SuspendProcessThreadsForcefully", 0, 0, true, SECONDLEVEL);
	}

Cleanup:
	if (hThreadSnap != INVALID_HANDLE_VALUE)
		CloseHandle(hThreadSnap);

	hThreadSnap = INVALID_HANDLE_VALUE;

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : StartScanning
*  Description    : function for start scanning for viruses and if detected the catch them into table for repair.
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::StartScanning()
{
	try
	{
		TCHAR	szDrives[256] = { 0 };

		m_dwScannedFiles = m_dwDetectedFiles = m_dwRepairedFiles = m_dwRebootFileDeletion = 0x00;
		//m_ListIndex = 0x00;
		m_dwScanStartTickCount = 0x00;
		//m_FilesList.DeleteAllItems();

		m_bRemovDriveRemoved = false;

		ZeroMemory(&m_StartScanTime, sizeof(m_StartScanTime));
		GetSystemTime(&m_StartScanTime);

		m_dwScanStartTickCount = GetTickCount();
		
		//Added threat(s) count in GUI by Vilas on 03 / 09 / 2015
		//m_stThreatCount.ShowWindow(TRUE);
		CString strText = L"";
		strText.Format(L"%s 0", theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_THREAT_COUNT"));
		//m_stThreatCount.SetWindowTextW(strText);
		AddLogEntry(L"Scanning Started...", 0, 0, true, SECONDLEVEL);

		if (m_bAllDrives)
		{

			GetLogicalDriveStrings(255, szDrives);

			TCHAR	*pDrive = szDrives;

			while (wcslen(pDrive) > 2)
			{

				if ((GetDriveType(pDrive) & DRIVE_FIXED) == DRIVE_FIXED)
					EnumFolder(pDrive);

				pDrive += 0x04;
			}

		}
		else
		{
			//m_SpecificPath.GetWindowTextW(m_szSpecificPath, 510);
			Sleep(500);
			if (/*(m_Radio_SpecificPath.GetCheck() == BST_CHECKED) &&*/ PathIsDirectory(m_szSpecificPath))
				EnumFolder(m_szSpecificPath);
			else
				CheckFileForViruses(m_szSpecificPath);
		}

		ZeroMemory(szDrives, sizeof(szDrives));
		swprintf_s(szDrives, 255, L"Scanning finished. Scanned files::%lu, Detected files::%lu\n", m_dwScannedFiles, m_ListIndex);
		AddLogEntry(szDrives, 0, 0, true, SECONDLEVEL);

		CString csScanStartTime = 0;
		
		//CString csInsertQuery = _T("INSERT INTO Wardwiz_AutorunScanDetails VALUES (null,");
		//csInsertQuery.Format(_T("INSERT INTO Wardwiz_AutorunScanDetails VALUES (null,Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),Date('now'),%d,%d,%d );"), m_ListIndex, m_dwScannedFiles, m_dwRepairedFiles);

		//CT2A ascii(csInsertQuery, CP_UTF8);

		//CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);

		//objSqlDb.Open();
		//objSqlDb.CreateWardwizSQLiteTables();
		//objSqlDb.Close();

		//m_iAutoScanId = InsertDataToTable(ascii.m_psz);
		//bIsTempDetailsInDB = true;

		CString csInsertQuery = 0;
		csInsertQuery.Format(_T("UPDATE Wardwiz_AutorunScanSessionDetails SET db_ScanSessionEndDate = Date('now'),db_ScanSessionEndTime = Datetime('now', 'localtime'),db_ThreatsFoundCount = %d,db_FilesScannedCount = %d,db_FilesCleanedCount = %d WHERE db_ScanSessionID = %I64d;"), m_dwDetectedFiles, m_dwScannedFiles, m_dwRepairedFiles, m_iScanSessionId);

		CT2A ascii(csInsertQuery, CP_UTF8);

		if (!PathFileExistsA(g_strDatabaseFilePath))
		{
			m_objSqlDb.Open();
			m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
			m_objSqlDb.Close();
		}

		InsertDataToTable(ascii.m_psz);

		m_svFuncScanFinishedCB.call();  //function for beyound 100% issue

		if (m_ListIndex)
		{
			//m_SelectAll.EnableWindow();
			m_SelectAll.SetCheck(BST_CHECKED);
		}
		else
		{
		//	//m_generateCache.EnableWindow();

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::StartScanning", 0, 0, true, SECONDLEVEL);
	}
}
/***************************************************************************************************
*  Function Name  : DeleteFileForcefully
*  Description    : function to delete file forcefully during delete operation
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
DWORD CWrdWizAutoScnDlg::DeleteFileForcefully(LPCTSTR lpFilePath)
{
	DWORD	dwRet = 0x00;

	try
	{
		SetFileAttributes(lpFilePath, FILE_ATTRIBUTE_NORMAL);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(10);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(20);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(30);
		if (!_wremove(lpFilePath))
			goto Cleanup;

		Sleep(40);
		if (!_wremove(lpFilePath))
			goto Cleanup;

		dwRet = 0x01;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::DeleteFileForcefully", 0, 0, true, SECONDLEVEL);
		dwRet = 0x02;
	}

Cleanup:

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : EnumFolder
*  Description    : function to check directory for viruses
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::EnumFolder(LPCTSTR lpFolPath)
{
	try
	{

		if ( (m_bRemovDriveRemoved) && (!m_bAllDrives))
		{
			return;
		}

		if (m_bClose)
			return;

		CFileFind finder;

		CString strWildcard(lpFolPath);

	/*	DWORD	dwAttributes = GetFileAttributes(lpFolPath);


		if (INVALID_FILE_ATTRIBUTES != dwAttributes)
			SetFileAttributes(lpFolPath, (dwAttributes & (~FILE_ATTRIBUTE_HIDDEN)));

		SetFileAttributes(lpFolPath, FILE_ATTRIBUTE_NORMAL);*/

		if (strWildcard.IsEmpty())
			return;

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");

		if (!PathFileExists(lpFolPath))
		{
			m_bRemovDriveRemoved = true;
			return;
		}

		
		BOOL bWorking = finder.FindFile(strWildcard);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[1024] = { 0 };

			wsprintf(szPath, L"%s", finder.GetFilePath());

			if (finder.IsDirectory())
				EnumFolder(szPath);
			else
				CheckFileForViruses(szPath);
		}

		finder.Close();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckFileForViruses
*  Description    : function to check file for viruses
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::CheckFileForViruses(LPCTSTR lpFileName)
{
	__try 
	{
		CheckFileForVirusesSEH(lpFileName);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckFileForViruses", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckFileForVirusesSEH
*  Description    : function to check file for viruses
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::CheckFileForVirusesSEH(LPCTSTR lpFileName)
{
	DWORD	dwRet = 0x00;
	TCHAR	szTemp[256] = { 0 };
	bool	bVirusFound = false;
	try
	{

		//m_CurrentFile.SetWindowTextW(lpFileName);

		m_hFileHandle = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		DWORD	dwFileSize = GetFileSize(m_hFileHandle, NULL);

		if ((dwFileSize == 0xFFFFFFFF) || (!dwFileSize) ||
			(dwFileSize > 0x5000) ||				//Thumb.db size		= 8,432 byes
			(dwFileSize < 0xED)						//autorun.inf size	= 237 byes
			)
		{
			dwRet = 0x02;
			goto Cleanup;
		}


		//Modified date : 28 / 02 / 2015
		//Analyzed and Need to do :: if( (dwFileSize > 0xFFF ) && (dwFileSize < 0x2000) )

		//if( (dwFileSize > 0x300 ) && (dwFileSize < 0x2000) )
		if ((dwFileSize > 0xFFF) && (dwFileSize < 0x2000))
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		BYTE	szData[0x60] = { 0 };
		DWORD	dwReadBytes = 0x00;

		ReadFile(m_hFileHandle, szData, 0x60, &dwReadBytes, NULL);
		if (0x60 != dwReadBytes)
		{
			dwRet = 0x04;
			goto Cleanup;
		}


		TCHAR	szVirusName[0x100] = { 0 };

		if (dwFileSize < 0x500)
		{
			CheckForAutorunInfFile(szData, dwReadBytes, bVirusFound);
			if (bVirusFound)
			{
				m_bIsTempFileFoundFinished = true;

				AddEntryinListView(L"WW.WORM.AUTORUN.INF", lpFileName);

				goto Cleanup;
			}
		}

		//Modified by Vilas on 04 / 12 / 2015
		//due to malware found of size 0x1000( 2088 )
		if ((dwFileSize > 0xFF) && (dwFileSize < 0x1001))
		{
			CheckForAutorunLNKFile(lpFileName, szData, dwReadBytes, m_hFileHandle, dwFileSize, bVirusFound, szVirusName);
			if (bVirusFound)
			{
				m_bIsTempFileFoundFinished = true;

				if (wcslen(szVirusName))
					AddEntryinListView(szVirusName, lpFileName);
				else
					AddEntryinListView(L"WW.TRO.AUTORUNNER.B", lpFileName);

				goto Cleanup;
			}
		}

		if ((dwFileSize > 0x1FFF) && (dwFileSize < 0x3001))
		{
			CheckForThumbsDBFile(szData, dwReadBytes, m_hFileHandle, dwFileSize, bVirusFound);
			if (bVirusFound)
			{
				m_bIsTempFileFoundFinished = true;

				AddEntryinListView(L"WW.WORM.AUTORUN.T", lpFileName);
				goto Cleanup;
			}
		}

		if ((dwFileSize > 0x1FFF) && (dwFileSize < 0x3001))
		{
			CheckForCantix(szData, dwReadBytes, m_hFileHandle, dwFileSize, bVirusFound);
			if (bVirusFound)
			{
				m_bIsTempFileFoundFinished = true;

				AddEntryinListView(L"WW.WORM.VBS.CANTIX.A", lpFileName);
				goto Cleanup;
			}
		}

		if ((dwFileSize > 0x3BFF) && (dwFileSize < 0x5001))
		{
			CheckforDesktopVBS(szData, dwReadBytes, m_hFileHandle, dwFileSize, bVirusFound);
			if (bVirusFound)
			{
				m_bIsTempFileFoundFinished = true;
				AddEntryinListView(L"WW.WORM.VBS.Dunihi.T", lpFileName);
				goto Cleanup;
			}
		}
		dwRet = 0x05;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckFileForViruses", 0, 0, true, SECONDLEVEL);
		dwRet = 0x06;
	}

Cleanup:
	try
	{
		m_dwScannedFiles++;

		CString csScannedFileCount = L"";
		csScannedFileCount.Format(L"%d", m_dwScannedFiles);
		CString csTotalFileCount = L"";
		if (m_bFileCountObtained)
			csTotalFileCount.Format(L"%d", m_dwTotalFiles);
		else
			csTotalFileCount.Format(L"%d", 0);

		if (bVirusFound)
		{
			m_dwDetectedFiles++;
		}

		//m_svSetScanFileStatuscb.call((SCITER_STRING)csScannedFileCount, (SCITER_STRING)csTotalFileCount,lpFileName);
		TCHAR chFileName[MAX_PATH] = { 0 };
		GetShortPathName(lpFileName, chFileName, 60);
		sciter::value map;
		map.set_item("one", (SCITER_STRING)csScannedFileCount);
		map.set_item("two", (SCITER_STRING)csTotalFileCount);
		map.set_item("three", chFileName);

		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETFILEPATH_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);

		if (m_hFileHandle != INVALID_HANDLE_VALUE)
			CloseHandle(m_hFileHandle);

		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckFileForViruses", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckForAutorunInfFile
*  Description    : function to check file for autorun.inf type viruses
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
bool CWrdWizAutoScnDlg::CheckForAutorunInfFile(LPBYTE	lpbBuffer, DWORD dwSize, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;

		if (IsBadReadPtr(lpbBuffer, dwSize))
			goto Cleanup;

		//[autorun
		BYTE	bAutorun[0x08] = { 0x5B, 0x61, 0x75, 0x74, 0x6F, 0x72, 0x75, 0x6E };

		if (memcmp(lpbBuffer, bAutorun, sizeof(bAutorun)) == 0x00)
		{
			//open=WScript.exe //e:VBScript 
			BYTE bOpenWScript[0x1D] = { 0x6F, 0x70, 0x65, 0x6E, 0x3D, 0x57, 0x53, 0x63,
				0x72, 0x69, 0x70, 0x74, 0x2E, 0x65, 0x78, 0x65,
				0x20, 0x2F, 0x2F, 0x65, 0x3A, 0x56, 0x42, 0x53,
				0x63, 0x72, 0x69, 0x70, 0x74	/*, 0x20, 0x74, 0x68,
												0x75, 0x6D, 0x62, 0x2E, 0x64, 0x62, 0x20, 0x61,
												0x75, 0x74, 0x6F	*/
			};

			if (memcmp(&lpbBuffer[0x0B], bOpenWScript, sizeof(bOpenWScript)) == 0x00)
				bFound = bVirusFound = true;

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckForAutorunInfFile", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bVirusFound;
}

//Modified for Gamerue 
//by Vilas on 13 / 07 / 2015
/***************************************************************************************************
*  Function Name  : CheckForAutorunLNKFile
*  Description    : function to check file for autorun.lnk type viruses
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
bool CWrdWizAutoScnDlg::CheckForAutorunLNKFile(LPCTSTR lpFileName, LPBYTE lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound, LPTSTR lpszVirusName)
{
	bool bWScriptFound = false;
	bool bWThumbsFound = false;

	try
	{
		bFound = false;

		if (IsBadReadPtr(lpbBuffer, dwSize))
			goto Cleanup;

		//4C 00 00 00
		if (*((DWORD *)&lpbBuffer[0x00]) != 0x4C)
			goto Cleanup;

		//
		//if ((*((DWORD *)&lpbBuffer[0x50]) != 0x4FE0501F) || (*((DWORD *)&lpbBuffer[0x54]) != 0x3AEA20D0) ||

		if ((((*((DWORD *)&lpbBuffer[0x50])) & 0x4FE0FF1F) != ((*((DWORD *)&lpbBuffer[0x50])) & 0x4FE0FF1F)))
			goto Cleanup;
		
         //commented by pradip on 14-12-2017
		/*if ( (*((DWORD *)&lpbBuffer[0x54]) != 0x3AEA20D0) ||
			(*((DWORD *)&lpbBuffer[0x58]) != 0xD8A21069) || 
			(*((DWORD *)&lpbBuffer[0x5C]) != 0x302B0008)
			)
			goto Cleanup;
         */
		BYTE	bFileData[0x400] = { 0 };
		DWORD	dwBytesRead = 0x00;

		SetFilePointer(hFile, 0x100, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x400, &dwBytesRead, NULL);

		if (dwBytesRead < 0x49)
			goto Cleanup;

		unsigned char bWScriptExe[0x38] = { 0x77, 0x00, 0x73, 0x00, 0x63, 0x00, 0x72, 0x00,
			0x69, 0x00, 0x70, 0x00, 0x74, 0x00, 0x2E, 0x00,
			0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x00, 0x00,
			0x1A, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x2F, 0x00,
			0x2F, 0x00, 0x65, 0x00, 0x3A, 0x00, 0x56, 0x00,
			0x42, 0x00, 0x53, 0x00, 0x63, 0x00, 0x72, 0x00,
			0x69, 0x00, 0x70, 0x00, 0x74, 0x00, 0x20, 0x00	/*,
															0x74, 0x00, 0x68, 0x00, 0x75, 0x00, 0x6D, 0x00,
															0x62, 0x00, 0x2E, 0x00, 0x64, 0x00, 0x62, 0x00, 0x20
															*/
		};


		//Added for Gamerue 
		//by Vilas on 13 / 07 / 2015
		TCHAR	szDrive[0x08] = { 0 };
		TCHAR	szVolumeName[0x100] = { 0 };


		//by Vilas on 07 / 09 / 2015
		TCHAR	szTemp[0x100] = { 0 };

		const wchar_t *pFileName = wcsrchr(lpFileName, '\\');

		szDrive[0] = lpFileName[0];
		szDrive[1] = lpFileName[1];
		szDrive[2] = lpFileName[2];

		//Modified by Vilas on 08 / 07 / 2016
		//to support Win10 detection, issue no:0001550
		if ((wcsstr(pFileName, L"GB)."))
			)
		{
			bFound = true;
			wcscpy_s(lpszVirusName, 0xFE, L"WW.TRO.AUTORUNNER.AA");
			goto Cleanup;
		}

		GetVolumeInformationW(szDrive, szVolumeName, 0xFF, NULL, NULL, NULL, NULL, NULL);
		if (wcslen(szVolumeName))
		{
			_wcsupr_s(szVolumeName);

			if (wcsstr(pFileName, szVolumeName))
			{
				bFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.TRO.AUTORUNNER.AA");
				goto Cleanup;
			}

			//by Vilas on 07 / 09 / 2015
			swprintf_s(szTemp, 255, L" (%c) ", szDrive[0]);

			//if (wcsstr(pFileName, L"GB)."))
			//Modified by Vilas on 07 / 09 / 2015
			if ((wcsstr(pFileName, L"GB).")) ||
				(wcsstr(pFileName, szTemp))
				)
			{
				bFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.TRO.AUTORUNNER.AA");
				goto Cleanup;
			}
		}

		//by Vilas on 15 / 09 / 2015
		//for the detection of Worm.Win32.Gamarue.lnk
		DWORD	dwIter = 0x08;

		if (( wcslen(pFileName) > 0x09 ) &&
			(GetDriveType(szDrive) == DRIVE_REMOVABLE)
			)
		{
			for (; dwIter < 0x101;)
			{
				if (m_bClose)
					goto Cleanup;

				ZeroMemory(szTemp, sizeof(szTemp));
				swprintf_s(szTemp, 255, L"(%luGB).", dwIter);
				if (wcsstr(pFileName, szTemp))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.TRO.AUTORUNNER.AA");
					goto Cleanup;
				}

				dwIter *= 0x02;
			}
		}

		dwIter = 0x00;
		dwBytesRead -= 0x49;
		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bWScriptExe, 0x15) == 0x00)
					bWScriptFound = true;
			}

			if (!bWThumbsFound)
			{
				if (_memicmp(&bFileData[dwIter + 0x24], &bWScriptExe[0x24], 0x14) == 0x00)
					bWThumbsFound = true;
			}

			if (bWScriptFound && bWThumbsFound)
			{
				bFound = true;
				break;
			}
		}

		if (bFound)
			goto Cleanup;
		//Added by Vilas on 31 March 2015
		//Samples given by Tapasvi ( his harddisk )

		//c.m.d...e.x.e.
		//63 00 6d 00 64 00 2e 00 65 00 78 00 65 00

		// /.c s.t.a.r.t.
		//63 00 20 00 73 00 74 00 61 00 72 00  74 00

		//v.b.s.&
		//76 00 62 00 73 00 26 00
		//		or

		//V.B.S.&
		//56 00 42 00 53 00 26 00
		//Found lnk sapmles on 14 April 2015 so used _memicmp instead of memcmp

		BYTE	bCmdExe[] = { 0x63, 0x00, 0x6d, 0x00, 0x64, 0x00, 0x2e, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00 };
		BYTE	bCStart[] = { 0x63, 0x00, 0x20, 0x00, 0x73, 0x00, 0x74, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74, 0x00 };
		BYTE	bVbsExt[] = { 0x76, 0x00, 0x62, 0x00, 0x73, 0x00, 0x26, 0x00 };

		BYTE	bVBSExt[] = { 0x56, 0x00, 0x42, 0x00, 0x53, 0x00, 0x26, 0x00 };

		DWORD	dwCStartLocation = 0x00;


		dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bCmdExe, sizeof(bCmdExe)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bCmdExe);
					continue;
				}
			}

			if ((bWScriptFound) && (!bWThumbsFound) )
			{
				if (_memicmp(&bFileData[dwIter], bCStart, sizeof(bCStart)) == 0x00)
				{
					bWThumbsFound = true;
					dwIter += sizeof(bCStart);
					dwCStartLocation = dwIter;
					continue;
				}
			}

			//if (bWScriptFound && bWThumbsFound)
			if (dwCStartLocation)
			{
				//(memcmp(&bFileData[dwIter], bVBSExt, sizeof(bVBSExt)) == 0x00))
				if ((_memicmp(&bFileData[dwIter], bVbsExt, sizeof(bVbsExt)) == 0x00) &&
					((dwIter - dwCStartLocation)<0x20) 
					)
				{
					bFound = true;
					break;
				}
			}

		}


		if (bFound)
			goto Cleanup;


		//Added by Vilas on 27 April 
		//Samples given by Customer Nilesh


		BYTE	bc[] = { 0x2F, 0x00, 0x63, 0x00, 0x20, 0x00 };		//Checking /c(1 space)

		BYTE	bclsstart[29] = {	0x73, 0x00, 0x26, 0x00, 0x63, 0x00, 0x6C, 0x00, 0x73, 0x00, 0x26, 0x00, 0x63, 0x00, 0x6C, 
									0x00, 0x73, 0x00, 0x26, 0x00, 0x73, 0x00, 0x74, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74
								};


		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bCmdExe, sizeof(bCmdExe)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bCmdExe);
					continue;
				}
			}

			if ((bWScriptFound) && (!bWThumbsFound))
			{
				if (_memicmp(&bFileData[dwIter], bc, sizeof(bc)) == 0x00)
				{
					bWThumbsFound = true;
					dwIter += sizeof(bCStart);
					dwCStartLocation = dwIter;
					continue;
				}
			}

			if (bWScriptFound && bWThumbsFound)
			{
				if ((_memicmp(&bFileData[dwIter], bclsstart, sizeof(bclsstart)) == 0x00) &&
					((dwIter - dwCStartLocation)<0x70)
					)
				{
					bFound = true;
					break;
				}
			}

		}


		if (bFound)
			goto Cleanup;

		//Added by Vilas on 06 April 2015
		//Samples given by Tapasvi ( Anupam Pandey )
		
		//R.E.C.Y.C.L.E.R.\.S.-.
		//52 00 45 00 43 00 59 00 43 00 4C 00 45 00 52 00 5C 00 53 00 2D 00
		
		//c.p.l.
		//63 00 70 00 6C 00


		BYTE	bRecycler[] = { 0x52, 0x00, 0x45, 0x00, 0x43, 0x00, 0x59, 0x00, 0x43, 0x00, 0x4C, 0x00, 0x45, 0x00, 0x52, 0x00, 0x5C, 0x00, 0x53, 0x00, 0x2D, 0x00 };
		BYTE	bCpl[] = { 0x63, 0x00, 0x70, 0x00, 0x6C, 0x00 };

		
		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRecycler, sizeof(bRecycler)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRecycler);
					dwCStartLocation = dwIter;
					continue;
				}
			}

			if ((bWScriptFound) &&
				((dwIter - dwCStartLocation)<0x80)
				)
			{
				if (_memicmp(&bFileData[dwIter], bCpl, sizeof(bCpl)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}
	
		if (bFound)
			goto Cleanup;


		//Added by Vilas on 15 April 2015
		//Samples given by Anupam Pandey

		//:\Windows\System32\rundll32.exe
		BYTE bRundll32Path[32] = {		0x3A, 0x5C, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x5C, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6D, 
										0x33, 0x32, 0x5C, 0x72, 0x75, 0x6E, 0x64, 0x6C, 0x6C, 0x33, 0x32, 0x2E, 0x65, 0x78, 0x65 };

		//(1 spaces)rundll32.exe (2 spaces)
		BYTE bRundll32ExeName[22] = {	0x20, 0x00, 0x72, 0x00, 0x75, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x6C, 0x00, 0x6C, 0x00,
										0x33, 0x00, 0x32, 0x00, 0x20, 0x00, 0x20, 0x00 };

		//shell32.dll
		BYTE bShell32DLLName[22] = {	0x73, 0x00, 0x68, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x33, 0x00, 0x32, 0x00, 0x2E, 
										0x00, 0x64, 0x00, 0x6C, 0x00, 0x6C, 0x00 };


		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRundll32Path, sizeof(bRundll32Path)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRundll32Path);
					dwCStartLocation = dwIter;
					
				}

				continue;
			}


			if ((bWScriptFound) && (!bWThumbsFound))
			{
				if (_memicmp(&bFileData[dwIter], bRundll32ExeName, sizeof(bRundll32ExeName)) == 0x00)
				{
					bWThumbsFound = true;
					dwIter += sizeof(bRundll32ExeName);
					dwCStartLocation = dwIter;
					
				}

				continue;
			}

			if ((bWScriptFound) && (bWThumbsFound) && ((dwIter - dwCStartLocation)<0x10)
				)
			{
				if (_memicmp(&bFileData[dwIter], bShell32DLLName, sizeof(bShell32DLLName)) == 0x00)
				{
					bFound = true;
					break;
				}
			}

		}

		if (bFound)
			goto Cleanup;


		//Added by Vilas on 04 Dec 2015
		//Samples given by Tapi ( Customer is Krishna Ballare)

		//\ & attrib +s +h %cd
		BYTE bAttribSH[41] = { 0x5C, 0x00, 0x20, 0x00, 0x26, 0x00, 0x20, 0x00, 0x61, 0x00, 0x74, 0x00, 0x74, 0x00, 0x72, 0x00,
			0x69, 0x00, 0x62, 0x00, 0x20, 0x00, 0x2B, 0x00, 0x73, 0x00, 0x20, 0x00, 0x2B, 0x00, 0x68, 0x00,
			0x20, 0x00, 0x25, 0x00, 0x63, 0x00, 0x64, 0x00, 0x25};

		//exe & start %temp%
		BYTE bStartTemp[37] = { 0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x20, 0x00, 0x26, 0x00, 0x20, 0x00, 0x73, 0x00, 0x74, 0x00,
			0x61, 0x00, 0x72, 0x00, 0x74, 0x00, 0x20, 0x00, 0x25, 0x00, 0x74, 0x00, 0x65, 0x00, 0x6D, 0x00,
			0x70, 0x00, 0x25, 0x00, 0x5C};

		bWScriptFound = false;
		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bAttribSH, sizeof(bAttribSH)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bAttribSH);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bStartTemp, sizeof(bStartTemp)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 7-11-2017
		///.c. .".{.b.5.d.c.c.7.f.7.-.e.4.3.f.
		//2F 00 63 00 20 00 22 00 7B 00 62 00 35 00 64 00 63 00 63 00 37 00 66 00 37 00 2D 00 65 00 34 00 33 00 66 00
		BYTE bFirstCheck[] = { 0x2F, 0x00, 0x63, 0x00, 0x20, 0x00, 0x22, 0x00, 0x7B, 0x00, 0x62,
			0x00, 0x35, 0x00, 0x64, 0x00, 0x63, 0x00, 0x63, 0x00, 0x37, 0x00,
			0x66, 0x00, 0x37, 0x00, 0x2D, 0x00, 0x65, 0x00, 0x34, 0x00, 0x33,
			0x00, 0x66, 0x00 };

		//2.b.7.4.c.2...e.x.e
		//32 00 62 00 37 00 34 00 63 00 32 00 2E 00 65 00 78 00 65
		BYTE bSecondCheck[] = { 0x32, 0x00, 0x62, 0x00, 0x37, 0x00, 0x34, 0x00, 0x63, 0x00,
			0x32, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };


		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bFirstCheck, sizeof(bFirstCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bFirstCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bSecondCheck, sizeof(bSecondCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added By pradip on 9-11-2017
		//Customer Abdul Momin from Gujarat
		//w"..r.u.n.d.l.l.3.2...e.x.e
		//77 22 F7 00 72 00 75 00 6E 00 64 00 6C 00 6C 00 33 00 32 00 2E 00 65 00 78 00 65
		BYTE bRundll32Check[] = { 0x77, 0x22, 0xF7, 0x00, 0x72, 0x00, 0x75,
			0x00, 0x6E, 0x00, 0x64, 0x00, 0x6C, 0x00,
			0x6C, 0x00, 0x33, 0x00, 0x32, 0x00, 0x2E,
			0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		//Modified By Pradip on 10-11-2017
		//rJ]. .rundll32.exe..J.......rJ].rJ].
		// 72 4A 5D A7 20 00 72 75 6E 64 6C 6C 33 32 2E 65 78 65 00 00 4A 00 09 00 04 00 EF BE 72 4A 5D A7 72 4A 5D

		BYTE bRJCheck[] = { 0x72, 0x4A, 0x5D, 0xA7, 0x20, 0x00, 0x72, 0x75, 0x6E,
			0x64, 0x6C, 0x6C, 0x33, 0x32, 0x2E, 0x65, 0x78, 0x65,
			0x00, 0x00, 0x4A, 0x00, 0x09, 0x00, 0x04, 0x00, 0xEF,
			0xBE, 0x72, 0x4A, 0x5D, 0xA7, 0x72, 0x4A, 0x5D };


		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup; 

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRJCheck, sizeof(bRJCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRJCheck);		//Modified By pradip on 25-11-2017
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRundll32Check, sizeof(bRundll32Check)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 25-11-2017

		//R:\951360278\
		// 52 3A 5C 39 35 31 33 36 30 32 37 38 5C
       BYTE bRCheck[] = { 0x52, 0x3A, 0x5C, 0x39, 0x35, 0x31, 0x33, 0x36, 0x30, 0x32, 0x37, 0x38, 0x5C };

		//1SPS..XF.L8C....&.m
		//31 53 50 53 E2 8A 58 46 BC 4C 38 43 BB FC 13 93 26 98 6D CE
		BYTE bSPSCheck[] = { 0x31, 0x53, 0x50, 0x53, 0xE2, 0x8A, 0x58, 0x46, 0xBC, 0x4C, 0x38, 0x43, 0xBB, 0xFC, 0x13, 0x93, 0x26, 0x98, 0x6D, 0xCE };



		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRCheck, sizeof(bRCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bSPSCheck, sizeof(bSPSCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 14-12-2017
		//Customer Sachin Bingude from pune
		//R.o.a.m.i.n.g.....f.2
		//52 00 6F 00 61 00 6D 00 69 00 6E 00 67 00 00 00 16 00 66 00 32
		BYTE bRoamingCheck[] = { 0x52, 0x00, 0x6F, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x69,
			0x00, 0x6E, 0x00, 0x67, 0x00, 0x00, 0x00, 0x16, 0x00, 0x66, 0x00, 0x32 };

		//pc-pc...........&.Q.A:"M....
		//70 63 2D 70 63 00 00 00 00 00 00 00 00 00 00 00 26 B1 51 D0 41 3A 22 4D AD CE 80 B2
		BYTE bPCCheck[] = { 0x70, 0x63, 0x2D, 0x70, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00,
			                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0xB1, 0x51, 0xD0,
			                0x41, 0x3A, 0x22, 0x4D, 0xAD, 0xCE, 0x80, 0xB2 };



		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRoamingCheck, sizeof(bRoamingCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRoamingCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bPCCheck, sizeof(bPCCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//.lnk files created by malware "Gamarue" family has less than 1800 bytes
		if (dwFileSize > 0x708)
			goto Cleanup;

		//Added by Vilas on 20 May 2015
		//Samples given by Santosh Gadkar
		//.lnk files created by malware "Gamarue" family and "NetWorm" detected as K7AntiVirus
		BYTE	bwNNDQ[24]		= {	0x0B, 0x00, 0x00, 0xA0, 0x77, 0x4E, 0xC1, 0x1A, 0xE7, 0x02, 0x5D, 0x4E, 0xB7, 0x44, 0x2E, 0xB1,
									0xAE, 0x51, 0x98, 0xB7, 0xD5, 0x00, 0x00, 0x00 };

		BYTE	bSID[12]		= { 0x53, 0x00, 0x2D, 0x00, 0x31, 0x00, 0x2D, 0x00, 0x35, 0x00, 0x2D, 0x00 };

		BYTE	bCheckData[19]	= { 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0xA0, 0x58, 0x00, 0x00, 0x00, 0x00,
									0x00, 0x00, 0x00 };

		DWORD	dwTemp = 0x00;
		bool	bThirdFound = false;
		bool	bFourthFound = false;

		dwBytesRead = 0x00;
		ZeroMemory(bFileData, sizeof(bFileData));
		ReadFile(hFile, bFileData, 0x400, &dwBytesRead, NULL);

		if (dwBytesRead < 0x20)
			goto Cleanup;

		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (!dwCStartLocation)
			{
				if (_memicmp(&bFileData[dwIter], bwNNDQ, sizeof(bwNNDQ)) == 0x00)
				{
					dwIter += sizeof(bwNNDQ);
					dwCStartLocation = dwIter;
				}

				continue;
			}

			//if ((dwCStartLocation) && (!bFourthFound) )
			if (dwCStartLocation)
			{
				if (!dwTemp)
				{
					if (_memicmp(&bFileData[dwIter], bSID, sizeof(bSID)) == 0x00)
					{

						dwIter += sizeof(bSID);
						bThirdFound = true;
						continue;
					}

				}

				if (bThirdFound)
				{
					if ((bFileData[dwIter] == 0x2D) && (bFileData[dwIter+1] == 0x00))
						bFourthFound = true;
				}

				if (bFourthFound)
				{
					if (_memicmp(&bFileData[dwIter], bCheckData, sizeof(bCheckData)) == 0x00)
					{
						break;
					}
				}

			}

		/*	bFound = true;
			dwIter += sizeof(bCheckData);
			for (; dwTemp < dwCStartLocation; dwTemp++)
			{
				if (m_szComputerName[dwTemp] != bFileData[dwIter++])
				{
					bFound = false;
					break;
				}
			}
			
			break;
		*/
		}

		if ( (bFourthFound) &&
			((*((DWORD *)&bFileData[dwBytesRead - 0x04]) == 0x00) && ((bFileData[dwBytesRead - 0x05]&0xE0)==0xE0))
			)
			bFound = true;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckForAutorunLNKFile", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bFound;
}

/***************************************************************************************************
*  Function Name  : CheckForThumbsDBFile
*  Description    : function to check file for thumbs.db viruses
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
bool CWrdWizAutoScnDlg::CheckForThumbsDBFile(LPBYTE	lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;
		if (m_bClose)
			goto Cleanup;

		if (IsBadReadPtr(lpbBuffer, dwSize))
			goto Cleanup;

		BYTE	bMuslimah[0x08] = { 0x6D, 0x75, 0x73, 0x6C, 0x69, 0x6D, 0x61, 0x68 };

		if (memcmp(&lpbBuffer[0x05], bMuslimah, sizeof(bMuslimah)) != 0x00)
			goto Cleanup;

		BYTE	bYuyun[8] = { 0x6D, 0x65, 0x3A, 0x59, 0x75, 0x79, 0x75, 0x6E };

		if (memcmp(&lpbBuffer[0x3E], bYuyun, sizeof(bYuyun)) != 0x00)
			goto Cleanup;

		BYTE	bFileData[0x40] = { 0 };
		DWORD	dwBytesRead = 0x00;

		SetFilePointer(hFile, 0x46A, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x35, &dwBytesRead, NULL);

		BYTE bRunWScriptExe[0x35] = { 0x52, 0x75, 0x6E, 0x20, 0x22, 0x57, 0x53, 0x63,
			0x72, 0x69, 0x70, 0x74, 0x2E, 0x65, 0x78, 0x65,
			0x20, 0x2F, 0x2F, 0x65, 0x3A, 0x56, 0x42, 0x53,
			0x63, 0x72, 0x69, 0x70, 0x74, 0x20, 0x22, 0x2B,
			0x74, 0x6D, 0x70, 0x74, 0x2B, 0x22, 0x20, 0x22,
			0x22, 0x22, 0x2B, 0x51, 0x2B, 0x22, 0x22, 0x22,
			0x22, 0x0A, 0x0A, 0x00, 0x27
		};


		if (memcmp(bFileData, bRunWScriptExe, sizeof(bRunWScriptExe)) == 0x00)
			bFound = bVirusFound = true;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckForThumbsDBFile", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bVirusFound;
}

/***************************************************************************************************
*  Function Name  : CheckForCantix
*  Description    : function to check file for catix viruses
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
bool CWrdWizAutoScnDlg::CheckForCantix(LPBYTE lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;

		if (IsBadReadPtr(lpbBuffer, dwSize))
			goto Cleanup;

		//microsot
		//BYTE	microsot[0x08] = { 0x6D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66 };
		if ((*((DWORD *)&lpbBuffer[0x0C]) != 0x7263696D) || (*((DWORD *)&lpbBuffer[0x10]) != 0x666F736F))
			goto Cleanup;


		//redir.dll
		//BYTE	microsot[0x08] = { 0x72, 0x65, 0x64, 0x69, 0x72, 0x2E, 0x64, 0x6C, 0x6C };
		if ((*((DWORD *)&lpbBuffer[0x20]) != 0x69646572) || (*((DWORD *)&lpbBuffer[0x24]) != 0x6C642E72))
			goto Cleanup;

		//Windows.Serviks.
		BYTE	bWindowsServiks[0x10] = { 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x6B, 0x73, 0x0D };

		BYTE	bFileData[0x200] = { 0 };
		DWORD	dwBytesRead = 0x00, dwIter = 0x00;

		SetFilePointer(hFile, 0x90, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bWindowsServiks))
			goto Cleanup;

		dwBytesRead -= sizeof(bWindowsServiks);
		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (memcmp(&bFileData[dwIter], bWindowsServiks, sizeof(bWindowsServiks)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		if (!bVirusFound)
			goto Cleanup;


		//Run "WScript.exe //e:VBScript "
		BYTE bRunWScript[0x1F] = { 0x52, 0x75, 0x6E, 0x20, 0x22, 0x57, 0x53, 0x63,
			0x72, 0x69, 0x70, 0x74, 0x2E, 0x65, 0x78, 0x65,
			0x20, 0x2F, 0x2F, 0x65, 0x3A, 0x56, 0x42, 0x53,
			0x63, 0x72, 0x69, 0x70, 0x74, 0x20, 0x22
		};

		bVirusFound = false;
		dwBytesRead = 0x00;
		ZeroMemory(bFileData, sizeof(bFileData));

		SetFilePointer(hFile, 0xF50, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bRunWScript))
			goto Cleanup;

		dwBytesRead -= sizeof(bRunWScript);
		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (memcmp(&bFileData[dwIter], bRunWScript, sizeof(bRunWScript)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		bFound = bVirusFound;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckForCantix", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bVirusFound;
}

/***************************************************************************************************
*  Function Name  : CheckforDesktopVBS
*  Description    : function to check file for desktop vbf viruses
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
bool CWrdWizAutoScnDlg::CheckforDesktopVBS(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;
		if (IsBadReadPtr(lpbBuffer, dwBufSize))
			goto Cleanup;

		unsigned char bStartData[0x10] = {
				0x6F, 0x6E, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0x20, 0x72, 0x65, 0x73, 0x75, 0x6D, 0x65, 0x20
					};

		if (memcmp(lpbBuffer, bStartData, sizeof(bStartData)) != 0x00)
		{
			goto Cleanup;
		}

		//Encrypted data in function call which used to	ExecuteGlobal
		unsigned char bParameterData[0x2C] = {
				0x2B, 0x49, 0x44, 0x41, 0x67, 0x64, 0x47, 0x68, 0x6C, 0x62, 0x67, 0x30, 0x4B, 0x61, 0x57, 0x59,
				0x67, 0x49, 0x47, 0x52, 0x79, 0x61, 0x58, 0x5A, 0x6C, 0x4C, 0x6D, 0x52, 0x79, 0x61, 0x58, 0x5A,
				0x6C, 0x64, 0x48, 0x6C, 0x77, 0x5A, 0x53, 0x41, 0x67, 0x50, 0x53, 0x41
			};


		BYTE	bFileData[0x200] = { 0 };
		DWORD	dwBytesRead = 0x00, dwIter = 0x00;

		//Checking First time Encrypted data in function call which used to	ExecuteGlobal 
		SetFilePointer(hFile, 0x800, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);
		if (dwBytesRead < sizeof(bParameterData) )
		{
			goto Cleanup;
		}

		dwBytesRead -= sizeof(bParameterData);
		dwIter = 0x00;

		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (memcmp(&bFileData[dwIter], bParameterData, sizeof(bParameterData)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		if (!bVirusFound)
		{
			goto Cleanup;
		}


		//Checking Second time Encrypted data in function call which used to	ExecuteGlobal
		bVirusFound = false;
		ZeroMemory(bFileData, sizeof(bFileData));
		SetFilePointer(hFile, 0xE00, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);
		if (dwBytesRead < sizeof(bParameterData))
		{
			goto Cleanup;
		}

		dwBytesRead -= sizeof(bParameterData);
		dwIter = 0x00;

		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (memcmp(&bFileData[dwIter], bParameterData, sizeof(bParameterData)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		if (!bVirusFound)
		{
			goto Cleanup;
		}

		//Checking Third time Encrypted data in function call which used to	ExecuteGlobal
		bVirusFound = false;
		ZeroMemory(bFileData, sizeof(bFileData));
		SetFilePointer(hFile, 0x1900, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);
		if (dwBytesRead < sizeof(bParameterData))
		{
			goto Cleanup;
		}

		dwBytesRead -= sizeof(bParameterData);
		dwIter = 0x00;

		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (m_bClose)
				goto Cleanup;

			if (memcmp(&bFileData[dwIter], bParameterData, sizeof(bParameterData)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		bFound = bVirusFound;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckforDesktopVBS", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bVirusFound;
}

/***************************************************************************************************
*  Function Name  : AddEntryinListView
*  Description    : function to add detected viruse files to table list view. 
*  Author Name    : Adil Sheikh
*  Date           : 3 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::AddEntryinListView(LPCTSTR lpVirusName, LPCTSTR lpFileName, CString lpAction)
{
	//m_svSetScanFinishedStatuscb.call(lpVirusName);
	if (m_bClose)
		return;

	m_svAddAutoRunScanFoundEntriesCB.call((SCITER_STRING)lpVirusName, (SCITER_STRING)lpFileName, (SCITER_STRING)lpAction);
	

	TCHAR	szTemp[256] = { 0 };

	swprintf_s(szTemp, 255, L"%s", lpVirusName); 

	AddLogEntry(szTemp, lpFileName, lpAction, true, SECONDLEVEL);

	CString csVirusName(szTemp);
	csVirusName.Replace(L"'", L"''");

	CString csFileToScan(lpFileName);
	csFileToScan.Replace(L"'", L"''");

	CString csInsertQuery = _T("INSERT INTO Wardwiz_AutorunScanDetails VALUES (null,");
	csInsertQuery.Format(_T("INSERT INTO Wardwiz_AutorunScanDetails VALUES (null, %I64d ,Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),Date('now'),'%s','%s','IDS_CONSTANT_THREAT_DETECTED' );"), m_iScanSessionId, csVirusName, csFileToScan);

	CT2A ascii(csInsertQuery, CP_UTF8);

	if (!PathFileExistsA(g_strDatabaseFilePath))
	{
		m_objSqlDb.Open();
		m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
		m_objSqlDb.Close();
	}

	m_iAutoScanId = InsertDataToTable(ascii.m_psz);
	//Added threat(s) count in GUI by Vilas on 03 / 09 / 2015
	swprintf_s(szTemp, 255, L"%s %lu", theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_THREAT_COUNT"), m_ListIndex);
	m_stThreatCount.SetWindowTextW(szTemp);
}

/***************************************************************************************************
*  Function Name  : UnhideDirectory
*  Description    : function to make all directory unhide while scanning
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
bool CWrdWizAutoScnDlg::UnhideDirectory(LPCTSTR lpFileName)
{
	bool	bRet = false;

	try
	{
		TCHAR	szFilePath[512] = { 0 };
		TCHAR	*pExt = NULL;

		wcscpy_s(szFilePath, 511, lpFileName);
		pExt = wcsrchr(szFilePath, '.');
		if (pExt)
			*pExt = '\0';

		if (!PathFileExists(szFilePath))
			goto Cleanup;

		DWORD	dwAttrib = GetFileAttributes(szFilePath);

		if ((dwAttrib != 0xFFFFFFFF) &&
			((dwAttrib & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
			)
			SetFileAttributes(szFilePath, (dwAttrib & ~FILE_ATTRIBUTE_HIDDEN));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::UnhideDirectory", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bRet;
}

/***************************************************************************************************
*  Function Name  : DeleteFileForcefully
*  Description    : function to delete filepath and reboot files forcefully
*  Author Name    : 
*  Date           : 14 June 2016
****************************************************************************************************/
DWORD CWrdWizAutoScnDlg::DeleteFileForcefully(LPCTSTR lpFilePath, bool bDeleteReboot)
{

	DWORD	dwRet = 0x00;

	try
	{

		if (!PathFileExists(lpFilePath))
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		SetFileAttributes(lpFilePath, FILE_ATTRIBUTE_NORMAL);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(10);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(20);
		if (DeleteFile(lpFilePath))
			goto Cleanup;

		Sleep(30);
		if (!_wremove(lpFilePath))
			goto Cleanup;

		Sleep(40);
		if (!_wremove(lpFilePath))
			goto Cleanup;

		MoveFileEx(lpFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		m_dwRebootFileDeletion++;

		dwRet = 0x02;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::DeleteFileForcefully", 0, 0, true, SECONDLEVEL);
		//AddLogEntry(L"### Exception in CScanDlg::EnumFolder", lpFolPath);
		dwRet = 0x03;
	}

Cleanup:

	return dwRet;

}

/***************************************************************************************************
*  Function Name  : OnBnClickedRadioWholeSystem
*  Description    : function to make all drive selected for scanning
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedRadioWholeSystem()
{
	m_Radio_SpecificPath.SetCheck(BST_UNCHECKED);
	m_Radio_AllDrives.SetCheck(BST_CHECKED);

	//m_SpecificPath.EnableWindow(FALSE);
	//m_Browse.EnableWindow(FALSE);

	m_bAllDrives = true;

	//m_generateCache.SetWindowTextW(L"Scan");
}

/***************************************************************************************************
*  Function Name  : OnBnClickedRadioSpecificPath
*  Description    : function to make specific path selected for scanning
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedRadioSpecificPath()
{
	m_Radio_SpecificPath.SetCheck(BST_CHECKED);
	m_Radio_AllDrives.SetCheck( BST_UNCHECKED );

	m_SpecificPath.EnableWindow();
	m_Browse.EnableWindow();

	m_SpecificPath.SetFocus();

	m_bAllDrives = false;

	m_generateCache.SetWindowTextW(L"Scan");
}

/***************************************************************************************************
*  Function Name  : EnumFolderForFileCount
*  Description    : function to get file count during scan
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::EnumFolderForFileCount(LPCTSTR lpFolPath)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(lpFolPath);
			strWildcard += _T("\\*.*");
		BOOL bWorking = finder.FindFile(strWildcard);
		if (bWorking)
		{
		while (bWorking)
		{
			if (m_bClose)
				return;

			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[1024] = { 0 };

			wsprintf(szPath, L"%s", finder.GetFilePath());

			if (finder.IsDirectory())
				EnumFolderForFileCount(szPath);
			else
			{
				//Added on 20 / 10 /2015 for Correct status of Progress bar
				m_dwTotalFiles++;
			}
			}
		}
		else
		{
			m_dwTotalFiles++;
		}
		finder.Close();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::EnumFolderForFileCount", 0, 0, true, SECONDLEVEL);
		//AddLogEntry(L"### Exception in CScanDlg::EnumFolder", lpFolPath);
	}
}

//Added on 02 / 09 /2015 for Progress bar
/***************************************************************************************************
*  Function Name  : Thread_GetFilesCount
*  Description    : function to total file count during scan
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
DWORD WINAPI Thread_GetFilesCount(LPVOID lpParam)
{
	DWORD		dwRet = 0x00;
	try
	{
		CWrdWizAutoScnDlg	*pCacheGenDlg = (CWrdWizAutoScnDlg *)lpParam;

		if (pCacheGenDlg->m_bAllDrives)
		{
			TCHAR	szDrives[256] = { 0 };

			GetLogicalDriveStrings(255, szDrives);

			TCHAR	*pDrive = szDrives;

			while (wcslen(pDrive) > 2)
			{

				if ((GetDriveType(pDrive) & DRIVE_FIXED) == DRIVE_FIXED)
					pCacheGenDlg->EnumFolderForFileCount(pDrive);

				pDrive += 0x04;
			}

		}
		else
		{
			pCacheGenDlg->EnumFolderForFileCount(pCacheGenDlg->m_szSpecificPath);
		}
		if (pCacheGenDlg->m_dwTotalFiles == 0)
		{
			//pCacheGenDlg->m_svEmptyFolderNotificationCB.call(1, "No file(s) found in selected folder.");
			pCacheGenDlg->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_VALID_FILE_FOLDER"));
			return 0;
		}

		//Added on 20 / 10 /2015 for Correct status of Progress bar
		pCacheGenDlg->m_bFileCountObtained = true;
		pCacheGenDlg->m_dwTotalFiles;
		//pCacheGenDlg->m_svSetScanFileStatuscb.call((LPVOID) lpParam);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WINAPI Thread_GetFilesCount", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : Thread_StartScan
*  Description    : function to run start scan thread
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
DWORD WINAPI Thread_StartScan(LPVOID lpParam)
{

	DWORD		dwRet = 0x00;

	SYSTEMTIME	sysTime = { 0 };
	TCHAR		szTemp[256] = { 0 };

	CWrdWizAutoScnDlg	*pCacheGenDlg = (CWrdWizAutoScnDlg *)lpParam;

	GetLocalTime(&sysTime);

	try
	{
		
		if (!pCacheGenDlg)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		
		pCacheGenDlg->KillSpecifiedRunningProcesses();

		pCacheGenDlg->StartScanning();

		//Added on 02 / 09 /2015 for Progress bar
		//int		iPercentage = 100;
	

		//wsprintf(szTemp, TEXT("%d%%"), iPercentage);

		//pCacheGenDlg->SetElapsedTimetoGUI();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WINAPI Thread_StartScan", 0, 0, true, SECONDLEVEL);
		
	}

Cleanup:

	if (pCacheGenDlg)
	{

		if (pCacheGenDlg->m_ListIndex > 0x00)
		{
			

		}
		else
		{
			//For issue:0000823
			//Added by Vilas on 17 - 12 - 2015
			//pCacheGenDlg->m_generateCache.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
			//pCacheGenDlg->m_SelectAll.SetCheck(BST_CHECKED);

			//pCacheGenDlg->m_Close.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_CLOSE"));

			//pCacheGenDlg->m_generateCache.EnableWindow();
			//pCacheGenDlg->m_SelectAll.EnableWindow(FALSE);

			//pCacheGenDlg->m_Radio_AllDrives.EnableWindow();
			//pCacheGenDlg->m_Radio_SpecificPath.EnableWindow();

			//if (pCacheGenDlg->m_Radio_AllDrives.GetCheck())
			//{
				//pCacheGenDlg->m_Radio_SpecificPath.SetCheck(BST_UNCHECKED);
				//pCacheGenDlg->m_SpecificPath.EnableWindow(FALSE);
				//pCacheGenDlg->m_Browse.EnableWindow(FALSE);
			//}
			//else
			//{
				//pCacheGenDlg->m_Radio_AllDrives.SetCheck(BST_UNCHECKED);
				//pCacheGenDlg->m_SpecificPath.EnableWindow();
				//pCacheGenDlg->m_Browse.EnableWindow();
			//}
		}

		//Commented out because we added threat(s) count in GUI 
		//Modified by Vilas on 03 / 09 / 2015

		//ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, 255, L"Viruses found : %lu", pCacheGenDlg->m_ListIndex);
		pCacheGenDlg->m_CurrentFile.SetWindowTextW(szTemp);
	}

	pCacheGenDlg->m_bScanStarted = false;
	pCacheGenDlg->m_bScanStart = false;
	//Issue no 1112, To restore UI after message
	pCacheGenDlg->ShowWindow(SW_RESTORE);
	::SetForegroundWindow(pCacheGenDlg->GetSafeHwnd());
	
	return dwRet;
}

//add Funtion comment
/***************************************************************************************************
*  Function Name  : Thread_RepairVirusFiles
*  Description    : function to run repair virus files thread
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
DWORD WINAPI Thread_RepairVirusFiles(LPVOID lpParam)
{
	DWORD		dwRet = 0x00;
	DWORD		dwDetectedFiles = 0x00;
	DWORD		dwActioned = 0x00;
	DWORD		dwTicked = 0x00;
	bool		bOperation = true;

	int	dwIndex = 0x00;
	bool	bChecked = false;

	SYSTEMTIME	sysTime = { 0 };
	TCHAR		szTemp[256] = { 0 };

	TCHAR		szHiddenPath[0x40] = { 0 };

	CWrdWizAutoScnDlg	*pCacheGenDlg = (CWrdWizAutoScnDlg *)lpParam;

	GetSystemTime(&sysTime);

	try
	{
		if (!pCacheGenDlg)
		{
			dwRet = 0x01;
			goto Cleanup;
		}
		
		dwDetectedFiles = pCacheGenDlg->m_svrecords.length();

		if (!dwDetectedFiles)
		{
			pCacheGenDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_NO_FILE_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			goto Cleanup;
		}

		pCacheGenDlg->m_bScanStarted = true;
		AddLogEntry(L"Repair started...", 0, 0, true, SECONDLEVEL);

		while (true)
		{
			dwDetectedFiles = pCacheGenDlg->m_svrecords.length();
			if (!dwDetectedFiles)
				break;
			bChecked = false;
			pCacheGenDlg->m_dwRepairedFiles = 0;
			for (; dwIndex < static_cast<int>(dwDetectedFiles); dwIndex++)
			{
				const SCITER_VALUE svEachEntry = pCacheGenDlg->m_svrecords[dwIndex];
				const std::wstring chThreatName = svEachEntry[L"ThreatName"].get(L"");
				const std::wstring chFilePath = svEachEntry[L"FilePath"].get(L"");
				const std::wstring chActionTaken = svEachEntry[L"ActionTaken"].get(L"");
				bool bValue = svEachEntry[L"selected"].get(false);

				if (!bValue)
					continue;

				bChecked = true;
				bOperation = false;
				dwTicked++;

				TCHAR	szFilePath[512] = { 0 };
				TCHAR	szVirusName[256] = { 0 };
				TCHAR	szAction[256] = { 0 };

				wcscpy_s(szFilePath, wcslen(chFilePath.c_str()) + 1, chFilePath.c_str());
				wcscpy_s(szVirusName, wcslen(chThreatName.c_str()) + 1, chThreatName.c_str());
				wcscpy_s(szAction, wcslen(chActionTaken.c_str()) + 1, chActionTaken.c_str());

				if (_wcsicmp(szAction, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED")) != 0x00)
				{
					dwActioned;
					continue;
				}

				if (_wcsicmp(szVirusName, L"WW.TRO.AUTORUNNER.B") == 0x00)
					pCacheGenDlg->UnhideDirectory(szFilePath);

				//Added for Gamerue 
				//by Vilas on 13 / 07 / 2015
				if (_wcsicmp(szVirusName, L"WW.TRO.AUTORUNNER.AA") == 0x00)
				{
					szHiddenPath[0x00] = szFilePath[0x00];
					szHiddenPath[0x01] = szFilePath[0x01];
					szHiddenPath[0x02] = szFilePath[0x02];

					//wcscpy_s(szRootPath, 0x3E, szHiddenPath);

					szHiddenPath[0x03] = 0xA0;
					szHiddenPath[0x04] = 0x00;

					pCacheGenDlg->UnhideDirectory(szHiddenPath);
				}

				DWORD dwRet = pCacheGenDlg->DeleteFileForcefully(szFilePath, true);

				switch (dwRet)
				{
				case 0x00:
					pCacheGenDlg->m_iRepairFileCount++;
					pCacheGenDlg->m_dwRepairedFiles++;
					AddLogEntry(szVirusName, L"\t\tRepaired successfully\t\t", szFilePath, true, SECONDLEVEL);
					pCacheGenDlg->m_setUpdateVirusFoundentriesCB.call(dwIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
					break;

				case 0x01:
					pCacheGenDlg->m_iRepairFileCount++;
					pCacheGenDlg->m_dwFailed++;
					AddLogEntry(szVirusName, L"\t\tRepaired failed(01)\t\t", szFilePath, true, SECONDLEVEL);
					pCacheGenDlg->m_setUpdateVirusFoundentriesCB.call((SCITER_VALUE)dwIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND"));
					break;

				case 0x02:
					pCacheGenDlg->m_iRepairFileCount++;
					AddLogEntry(szVirusName, L"\t\tRepaired on Reboot\t\t", szFilePath, true, SECONDLEVEL);
					pCacheGenDlg->m_setUpdateVirusFoundentriesCB.call((SCITER_VALUE)dwIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_RESTART_REQUIRED"));
					break;

				case 0x03:
					pCacheGenDlg->m_iRepairFileCount++;
					pCacheGenDlg->m_dwFailed++;
					AddLogEntry(szVirusName, L"\t\tRepaired failed(03)\t\t", szFilePath, true, SECONDLEVEL);
					pCacheGenDlg->m_setUpdateVirusFoundentriesCB.call((SCITER_VALUE)dwIndex, L"Repaired(03)");
					break;
				}
				if (pCacheGenDlg->m_iRepairFileCount == dwDetectedFiles)
				{
					pCacheGenDlg->m_bISRepairFile = false;
				}
				else
				{
					pCacheGenDlg->m_bISRepairFile = true;
				}
			}

			if (!bChecked)
				break;
			else
			{
				if ((DWORD)dwIndex >= dwDetectedFiles)
				{
					break;
				}
				else
				{
					dwIndex = 0x00;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WINAPI Thread_RepairVirusFiles", 0, 0, true, SECONDLEVEL);
		//pHeuScnDlg->AddLogEntry(L"### Exception in Thread_GenerateUpdateFiles()");
	}

Cleanup:

	if (pCacheGenDlg)
	{
		pCacheGenDlg->m_bRepairStart = false;
		pCacheGenDlg->m_bScanStarted = false;

		CString csInsertQuery = 0;
		csInsertQuery.Format(_T("UPDATE Wardwiz_AutorunScanSessionDetails SET db_ScanSessionEndDate = Date('now'),db_ScanSessionEndTime = Datetime('now', 'localtime'),db_ThreatsFoundCount = %d,db_FilesScannedCount = %d,db_FilesCleanedCount = %d WHERE db_ScanSessionID = %I64d;"), pCacheGenDlg->m_dwDetectedFiles, pCacheGenDlg->m_dwScannedFiles, pCacheGenDlg->m_dwRepairedFiles, pCacheGenDlg->m_iScanSessionId);

		CT2A ascii(csInsertQuery, CP_UTF8);

		if (!PathFileExistsA(g_strDatabaseFilePath))
		{
			pCacheGenDlg->m_objSqlDb.Open();
			pCacheGenDlg->m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
			pCacheGenDlg->m_objSqlDb.Close();
		}

		pCacheGenDlg->InsertDataToTable(ascii.m_psz);

		//pCacheGenDlg->m_FilesList.EnableWindow();
		pCacheGenDlg->ShowWindow(SW_RESTORE);
		//Issue no 1112, To restore UI after message
		::SetForegroundWindow(pCacheGenDlg->GetSafeHwnd());	
		if (bOperation)
		{
			AddLogEntry(L"No entries selected for repair.\n", 0, 0, true, SECONDLEVEL);
			//pCacheGenDlg->m_setUpdateVirusFoundentriesCB.call((SCITER_VALUE)dwIndex, L"Repaired(03)");
			pCacheGenDlg->m_svUpdateRepairedFilesStatusCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_NO_ENTRY_SELECTED"),(SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"));

		}
		else
		{
			//pCacheGenDlg->SetForegroundWindowAndBringToTop(pCacheGenDlg->m_MainWindow);
			pCacheGenDlg->ShowWindow(SW_RESTORE);
			//if ((dwTicked == 0x01) && (dwActioned == 0x01))
			if ((dwTicked == dwActioned) && (dwActioned > 0x00))
			{
				AddLogEntry(L"Please select detected file(s) to repair.\n", 0, 0, true, SECONDLEVEL);
				//Callback required
				pCacheGenDlg->m_svUpdateRepairedFilesStatusCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_NO_DETECTED_ENTRY_SELECTED"),(SCITER_STRING) theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"));
			}
			else
			{
				ZeroMemory(szTemp, sizeof(szTemp));
				swprintf_s(szTemp, 255, L"Repair finished. Repaired files::      %lu, Reboot Repair files::   %lu, Total detected files::%lu\n\n",
					//callback required
					pCacheGenDlg->m_dwRepairedFiles, pCacheGenDlg->m_dwRebootFileDeletion, dwDetectedFiles);

				AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);

				ZeroMemory(szTemp, sizeof(szTemp));
				if (pCacheGenDlg->m_dwRebootFileDeletion)
					swprintf_s(szTemp, 255, L"%s %lu\n%s %lu\n%s %lu\n\n %s",
					theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_REPAIR_FILES"), pCacheGenDlg->m_dwRepairedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_REBOOT_REPAIR_FILES"), pCacheGenDlg->m_dwRebootFileDeletion, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_TOTAL_DETECTED_FILE"), dwDetectedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_REBOOT_TO_COMPLETE"));
				else
					swprintf_s(szTemp, 255, L"%s %lu\n%s %lu\n%s",
					theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_REPAIR_FILES"), pCacheGenDlg->m_dwRepairedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_TOTAL_DETECTED_FILE"), dwDetectedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_COMPLETE_MSG"));

				//AfxMessageBox(szTemp);
				pCacheGenDlg->m_svUpdateRepairedFilesStatusCB.call((SCITER_STRING) szTemp,(SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"));
			}
		}

		//pCacheGenDlg->m_Radio_AllDrives.EnableWindow();
		//pCacheGenDlg->m_Radio_SpecificPath.EnableWindow();
		if (pCacheGenDlg->m_Radio_AllDrives.GetCheck())
		{
			//pCacheGenDlg->m_Radio_SpecificPath.SetCheck(BST_UNCHECKED);
			//pCacheGenDlg->m_SpecificPath.EnableWindow(FALSE);
			//pCacheGenDlg->m_Browse.EnableWindow(FALSE);
		}
		else
		{
			//pCacheGenDlg->m_Radio_AllDrives.SetCheck(BST_UNCHECKED);
			//pCacheGenDlg->m_Radio_SpecificPath.EnableWindow();
			//pCacheGenDlg->m_SpecificPath.EnableWindow();
			//pCacheGenDlg->m_Browse.EnableWindow();
	    }

		//pCacheGenDlg->m_generateCache.EnableWindow();
		if ((pCacheGenDlg->m_dwRepairedFiles + pCacheGenDlg->m_dwRebootFileDeletion + pCacheGenDlg->m_dwFailed + dwActioned) == dwDetectedFiles)
		{
			//pCacheGenDlg->m_generateCache.SetWindowTextW(L"Scan");

			//pCacheGenDlg->m_SelectAll.SetCheck(BST_UNCHECKED);
			//pCacheGenDlg->m_SelectAll.EnableWindow(FALSE);
		}
		else
		{
			//pCacheGenDlg->m_SelectAll.SetCheck(BST_UNCHECKED);
			//pCacheGenDlg->m_SelectAll.EnableWindow();
		}

		//pCacheGenDlg->m_Radio_SpecificPath.EnableWindow();
		//pCacheGenDlg->m_Radio_AllDrives.EnableWindow();

		//pCacheGenDlg->m_CurrentFile.SetWindowTextW(L"");
		//pCacheGenDlg->m_StartTime.SetWindowTextW(L"");
		//pCacheGenDlg->m_EndTime.SetWindowTextW(L"");

		//Added threat(s) count in GUI by Vilas on 03 / 09 / 2015
		//pCacheGenDlg->m_stThreatCount.SetWindowTextW(L"");
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCheckSelectAll
*  Description    : function to perform select all operation on all detected files
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedCheckSelectAll()
{
	if (m_SelectAll.GetCheck() == BST_CHECKED)
	{
		SetAllItemsInListViewasSelected();
	}
	else
	{
		SetAllItemsInListViewasSelected(false);
	}
}

/***************************************************************************************************
*  Function Name  : SetAllItemsInListViewasSelected
*  Description    : function to set all item in list view selected by default.
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::SetAllItemsInListViewasSelected(bool bSelectAll)
{
	DWORD dwTotDetectedFiles = m_FilesList.GetItemCount();

	if (!dwTotDetectedFiles)
		goto Cleanup;

	for (DWORD i = 0x00; i < dwTotDetectedFiles; i++)
	{
		if (bSelectAll)
			m_FilesList.SetCheck(i);
		else
			m_FilesList.SetCheck(i, FALSE);
	}

Cleanup:

	return;
}

/***************************************************************************************************
*  Function Name  : OnSetCursor
*  Description    : function to set window cusor position
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
BOOL CWrdWizAutoScnDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!pWnd)
		return FALSE;

	// hand cursor on information button neha gharge 22/5/2014.
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if (
		iCtrlID == IDC_RADIO_WHOLE_SYSTEM ||
		iCtrlID == IDC_RADIO_SPECIFIC_PATH ||
		iCtrlID == IDC_CHECK_SELECT_ALL ||
		iCtrlID == IDC_BUTTON_AUTORUN_MINIMIZE ||
		iCtrlID == IDC_BUTTON_AUTORUN_CLOSE ||
		iCtrlID == IDC_BUTTON_BROWSE ||
		iCtrlID == IDC_BUTTON_GENERATE_CACHE ||
		iCtrlID == IDC_BUTTON_CLOSE)
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if (csClassName == _T("Button") && m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}
	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonAutorunMinimize
*  Description    : function to minimize window operation
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonAutorunMinimize()
{
	this->ShowWindow(SW_MINIMIZE);
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonAutorunClose
*  Description    : function to perform close operation while close button is clicked
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonAutorunClose()
{
	//Added on 19 - 12 - 2015 by Vilas
	//Appropriate message is shown on main GUI
	ZeroMemory(szMessage, sizeof(szMessage));
	//wcscpy_s(szMessage, 256, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_INPROGRESS"));
	wcscpy_s(szMessage, 256, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_INPROGRESS_STOP"));

	OnBnClickedButtonClose();
}

/***************************************************************************************************
*  Function Name  : OnEnChangeEditSpecificPath
*  Description    : function for enabling edit specific path
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnEnChangeEditSpecificPath()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	m_generateCache.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
}

/***************************************************************************************************
*  Function Name  : OnClose
*  Description    : function to perform close operation while close button is clicked
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	//m_bIsTempFileFoundFinished = false;
	m_bIsRequestFromTaskBar = true;
	OnBnClickedButtonClose();
	//CJpegDialog::OnClose();
}

//Added by Vilas on 02 / 09 / 2015
/***************************************************************************************************
*  Function Name  : SetElapsedTimetoGUI
*  Description    : function to set elapsed time GUI
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::SetElapsedTimetoGUI()
{
	TCHAR	szTemp[256] = { 0 };

	SYSTEMTIME		CurrTime = { 0 };

	GetSystemTime(&CurrTime);

	CTime	objStartTime(m_StartScanTime);
	CTime	objCurrTime(CurrTime);

	CTimeSpan	objTimeDiff = objCurrTime - objStartTime;

	CString		strElapsedTime = L"";
	CString		strElapsedTimeFrame = objTimeDiff.Format(L"%H:%M:%S");
	strElapsedTime.Format(L"%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ELAPSEDTIME"), strElapsedTimeFrame);

	swprintf(szTemp, 254, L"%s", strElapsedTime.GetBuffer(strElapsedTime.GetLength()));
	m_EndTime.SetWindowTextW(szTemp);
	strElapsedTime.ReleaseBuffer();
}

/***************************************************************************************************
*  Function Name  : SetForegroundWindowAndBringToTop
*  Description    : function to set foreground window and bring it to the top
*  Author Name    :
*  Date           : 14 June 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::SetForegroundWindowAndBringToTop(HWND hWnd)
{
	if (!::IsWindow(hWnd))
		return;

	::SendMessage(hWnd, WM_ACTIVATE, 0, 0);
	//::ShowWindow(hWnd, SW_SHOW );

	DWORD	lockTimeOut = 0;
	HWND	hCurrWnd = ::GetForegroundWindow();
	DWORD	dwThisTID = ::GetCurrentThreadId();
	DWORD	dwCurrTID = ::GetWindowThreadProcessId(hCurrWnd, 0);


	if (dwThisTID != dwCurrTID)
	{
		::AttachThreadInput(dwThisTID, dwCurrTID, TRUE);

		::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lockTimeOut, 0);
		::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);

		::AllowSetForegroundWindow(ASFW_ANY);
	}

	::SetForegroundWindow(hWnd);

	if (dwThisTID != dwCurrTID)
	{
		::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)lockTimeOut, SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
		::AttachThreadInput(dwThisTID, dwCurrTID, FALSE);
	}
}


/**********************************************************************************************************
*  Function Name		: WindowProc
*  Description			: Window procedure added
*  Author Name			: Sanjay Khapre
*  Date					: 10 May 2016
*  SR_NO				:
**********************************************************************************************************/
LRESULT CWrdWizAutoScnDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	
	lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
	if (bHandled)      // if it was handled by the Sciter
		return lResult; // then no further processing is required.
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  : On_Close
*  Description    : Function to close dialog box
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date			  : 9 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_Close()
{
	m_bIsTempFileFoundFinished = false; //to avoid repeated msg
	OnClose();
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_Stop
*  Description    : Function to stop scanning
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date			  : 9 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_Stop()
{
	try
	{
		m_objSqlDb.Close();

		if (m_bPause_Resume == true)
		{
			m_bPause_Resume = false;

			if (m_bScanStarted)
			{
				if (m_hThreadFilesCount != NULL)
				{
					::SuspendThread(m_hThreadFilesCount);
				}

				if (m_hThreadStartScan != NULL)
				{
					::SuspendThread(m_hThreadStartScan);
				}
			}

			if (m_bRepairStart)
			{
				::SuspendThread(m_hThreadRepair);
			}
		}
		//if (!bIsTempDetailsInDB)
		//{
		//	CString csInsertQuery = _T("INSERT INTO Wardwiz_AutorunScanDetails VALUES (null,");
		//	csInsertQuery.Format(_T("INSERT INTO Wardwiz_AutorunScanDetails VALUES (null,Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),Date('now'),%d,%d,%d );"), m_dwDetectedFiles, m_dwScannedFiles, m_dwRepairedFiles);

		//	CT2A ascii(csInsertQuery, CP_UTF8);

		//	CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);

		//	objSqlDb.Open();
		//	objSqlDb.CreateWardwizSQLiteTables();
		//	objSqlDb.Close();

		//	m_iAutoScanId = InsertDataToTable(ascii.m_psz);
		//	bIsTempDetailsInDB = true;
		//}
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::On_Stop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedYesButton
*  Description    : Function to stop scanning while popup appears in sciter on stop button click
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date			  : 9 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::OnBnClickedYesButton()
{
	try
	{
		if (m_bPause_Resume == false)
		{
			if (m_bScanStart)
			{
				if (m_hThreadFilesCount != NULL)
				{
					::TerminateThread(m_hThreadFilesCount, 0x00);
					CloseHandle(m_hThreadFilesCount);
					m_hThreadFilesCount = NULL;
				}

				if (m_hThreadStartScan != NULL)
				{
					DWORD dwWaitTimeout = 3 * 1000; //3 seconds
					::WaitForSingleObject(m_hThreadStartScan, dwWaitTimeout);
					CloseHandle(m_hThreadStartScan);
					m_hThreadStartScan = NULL;
				}

				m_bScanStarted = m_bScanStart = false;
				m_bPause_Resume = true;

				UpdateSessionRecord();
			}
			return (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ABRTUSER");
		}
		//Added on 19 - 12 - 2015 by Vilas
		//Close button should not close main GUI directly
		if (m_bScanStarted)
		{
			//UpdateSessionRecord();
			m_bScanStarted = m_bScanStart = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::OnBnClickedYesButton", 0, 0, true, SECONDLEVEL);
	}
	return (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ABRTUSER");
}

/***************************************************************************************************
*  Function Name  : OnBnClickedNoButton
*  Description    : Dialog box function while stop button is pressed
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date			  : 9 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::OnBnClickedNoButton()
{
	try
	{
		if (m_bPause_Resume == false)
		{
			m_bPause_Resume = true;

			if (m_bScanStarted)
			{
				if (m_hThreadFilesCount != NULL)
				{
					::ResumeThread(m_hThreadFilesCount);
				}

				if (m_hThreadStartScan != NULL)
				{
					::ResumeThread(m_hThreadStartScan);
				}
			}

			if (m_bRepairStart)
			{
				::ResumeThread(m_hThreadRepair);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::OnBnClickedNoButton", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  On_SelectFileFolder
*  Description    :  Used to select File/Folder to Unhide.
*  Author Name    :  Adil Sheikh
*  Date           :  5th Aug 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_SelectFileFolder()
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
*  Function Name  :  OnBnClickedButtonBrowsefolder
*  Description    :  Used to Handle Action for Browse Button
*  Author Name    :
*  Date           :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonBrowsefolder()
{
	try
	{
		TCHAR *pszPath = new TCHAR[MAX_PATH];
		CString csTemp = L"";
		SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));
		//CString csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_USB_VACCINE_SELECT_FILE_FOLDER");
		BROWSEINFO        bi = { GetSafeHwnd(), NULL, pszPath, 0, BIF_BROWSEINCLUDEFILES, NULL, 0L, 0 };		//BIF_RETURNONLYFSDIRS
		LPITEMIDLIST      pIdl = NULL;
		m_csEditPath = L"";
		LPITEMIDLIST  pRoot = NULL;
		bi.pidlRoot = pRoot;
		pIdl = SHBrowseForFolder(&bi);
		if (NULL != pIdl)
		{
			SHGetPathFromIDList(pIdl, pszPath);
			size_t iLen = wcslen(pszPath);
			//if (iLen > 0 && GetDriveType(pszPath) != DRIVE_CDROM && GetDriveType(pszPath) == DRIVE_REMOVABLE)
			if ((iLen > 0 && GetDriveType(pszPath) != DRIVE_CDROM) && *pszPath != 'A') // A is Floppy disk drive.
			{
				csTemp = pszPath;
				if (csTemp.Right(1) == L"\\")
				{
					csTemp = csTemp.Left(static_cast<int>(iLen)-1);
				}
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
		delete[] pszPath;
		pszPath = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::OnBnClickedButtonBrowsefolder", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : On_Minimize
*  Description    : Minimize window while minimize window is pressed on UI.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date			  : 27 Aug 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_Minimize()
{
	try
	{
		OnBnClickedButtonAutorunMinimize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::On_Minimize", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_funcScanFinished
*  Description    : Function indicates scanning completed.
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date			  : 20 Sep 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_funcScanFinished(SCITER_VALUE svFuncScanFinishedCB)
{
	try
	{
		CString csInsertQuery = 0;
		csInsertQuery.Format(_T("UPDATE Wardwiz_AutorunScanSessionDetails SET db_ScanSessionEndDate = Date('now'),db_ScanSessionEndTime = Datetime('now', 'localtime'),db_ThreatsFoundCount = %d,db_FilesScannedCount = %d,db_FilesCleanedCount = %d WHERE db_ScanSessionID = %I64d;"), m_dwDetectedFiles, m_dwScannedFiles, m_dwRepairedFiles, m_iScanSessionId);

		CT2A ascii(csInsertQuery, CP_UTF8);

		if (!PathFileExistsA(g_strDatabaseFilePath))
		{
			m_objSqlDb.Open();
			m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
			m_objSqlDb.Close();
		}

		InsertDataToTable(ascii.m_psz);

		m_svFuncScanFinishedCB = svFuncScanFinishedCB;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::On_funcScanFinished", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

json::value CWrdWizAutoScnDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
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
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

void CWrdWizAutoScnDlg::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
{
	try
	{
		m_svEmptyFolderNotificationCB.call(iMsgType, (SCITER_STRING)strMessageString);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : UpdateSessionRecord
*  Description    : Function to updates seesion record.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  : 4 OCT 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::UpdateSessionRecord()
{
	try
	{	// Add entries into Database..
		CString csInsertQuery = 0;
		csInsertQuery.Format(_T("UPDATE Wardwiz_AutorunScanSessionDetails SET db_ScanSessionEndDate = Date('now'),db_ScanSessionEndTime = Datetime('now', 'localtime'),db_ThreatsFoundCount = %d,db_FilesScannedCount = %d,db_FilesCleanedCount = %d WHERE db_ScanSessionID = %I64d;"), m_dwDetectedFiles, m_dwScannedFiles, m_dwRepairedFiles, m_iScanSessionId);

		CT2A ascii(csInsertQuery, CP_UTF8);

		if (!PathFileExistsA(g_strDatabaseFilePath))
		{
			m_objSqlDb.Open();
			m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
			m_objSqlDb.Close();
		}

		m_csReports.Lock();
		InsertDataToTable(ascii.m_psz);
		m_csReports.Unlock();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::UpdateSessionRecord", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CWrdWizAutoScnDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


json::value CWrdWizAutoScnDlg::CloseWhileNoDetectedEntry()
{
	//m_bIsRequestFromTaskBar = true;
	OnCancel();
	return 0;
}

/***************************************************************************************************
Function Name  : CloseFileHandle
Description    : Closing the CreateFile handle.
Author Name    : Amol J.
SR_NO		   :
Date           : 18th Aug 2017
/***************************************************************************************************/
void CWrdWizAutoScnDlg::CloseFileHandle()
{
	__try
	{
		if (m_hFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CloseFileHandle", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Jeena Mariam Saji
*  Date			  : 04 Dec 2018
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : FunCheckInternetAccessBlock
*  Description    : To check internet access block
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 Dec 2019
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::FunCheckInternetAccessBlock()
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
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CWardwizAutoScnDlg::FunCheckInternetAccessBlock", 0, 0, true, SECONDLEVEL);
		}

		if (dwParentalControl == 1)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = ON_CHECK_INTERNET_ACCESS;

			CISpyCommunicator objCom(SERVICE_SERVER);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send Data in CWardwizAutoScnDlg::SendData", 0, 0, true, SECONDLEVEL);
			}

			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read Data in CWardwizAutoScnDlg::ReadData", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::FunCheckInternetAccessBlock()", 0, 0, true, SECONDLEVEL);
	}
	return RetVal;
}