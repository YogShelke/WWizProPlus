/**********************************************************************************************************
Program Name          : WrdWizAutoScnDlg.cpp
Description           : This class contains the functionality for Temporary File Cleaner: Save disk space by 
                        -removing temporary file(s).
It has 2 options	  : a) Find temporary file(s)  b) Remove temporary file(s).
Author Name			  : Amol Jaware
Date Of Creation      : 02nd June 2016
Version No            : 2.0.0.17
***********************************************************************************************************/

// WrdWizAutoScnDlg.cpp : implementation file

#include "stdafx.h"
#include "WrdWizAutoScn.h"
#include "WrdWizAutoScnDlg.h"
#include "afxdialogex.h"
#include "ISpyCommunicator.h"

char* g_strDatabaseFilePath = ".\\VBALLREPORTS.DB";
bool bIsTempDetailsInDB = false;

#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif

#include <Psapi.h>
#include <TlHelp32.h>

#include "CSecure64.h"
#include "DriverConstants.h"
#include "CScannerLoad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SETFILEPATH_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 6)
#define SETFILEPATH_UPDATE_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 7)
// CWrdWizAutoScnDlg dialog

CStringList m_strListProcesses;

DWORD WINAPI Thread_StartScan(LPVOID lpParam);
DWORD WINAPI Thread_RepairVirusFiles(LPVOID lpParam);

/***************************************************************************************************
*  Class Name     :  CWrdWizAutoScnDlg
*  Description    :  This Class constructor is used to initilization.
*  Author Name    :
*  Date           :
****************************************************************************************************/
CWrdWizAutoScnDlg::CWrdWizAutoScnDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWrdWizAutoScnDlg::IDD, pParent)
	, m_objSqlDb(g_strDatabaseFilePath)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strListProcesses.RemoveAll();
	m_vFileCacheInfo.clear();
	m_bScanStarted = false;
	m_pdwCrc32Table = NULL;
	m_bIsTempFileFoundFinished = false;
	m_ExcludeFols.RemoveAll();
	m_dwTotDetectedFiles = 0x00;
	m_dwTotalFilesCleaned = 0x00;
	m_dwActioned = 0x00;
}

/***************************************************************************************************
*  Class Name     :  CWrdWizAutoScnDlg
*  Description    :  This is Destructure to free resorces(memory).
*  Author Name    :
*  Date           :
****************************************************************************************************/
CWrdWizAutoScnDlg::~CWrdWizAutoScnDlg()
{
	FreeResource();
}

/***************************************************************************************************
*  Function Name     :	  DoDataExchange
*  Description		 :	  This function is Called by the framework to exchange and validate dialog data.	
*  Author Name		 :	
*  Date				 :	
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
	DDX_Control(pDX, IDC_STATIC_DUPLICATE_FILES_COUNT, m_DuplicateFilesCountText);
	DDX_Control(pDX, IDC_STATIC_SELECT_ALL, m_stSelctAll);
	DDX_Control(pDX, IDC_BUTTON_MINIMIZE, m_btnMinimize);
	DDX_Control(pDX, IDC_BUTTON_CLOSEUI, m_btnCloseUI);
	DDX_Control(pDX, IDC_STATIC_TEMP_FOOTER, m_stTempFooter);
	DDX_Control(pDX, IDC_STATIC_TEMP_CLEANER_HEADER, m_stTempCleanerHeader);
}

BEGIN_MESSAGE_MAP(CWrdWizAutoScnDlg, CJpegDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWrdWizAutoScnDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWrdWizAutoScnDlg::OnBnClickedCancel)
	//ON_BN_CLICKED(IDC_BUTTON_GENERATE_CACHE, &CWrdWizAutoScnDlg::OnBnClickedButtonGenerateCache)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CWrdWizAutoScnDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CWrdWizAutoScnDlg::OnBnClickedButtonBrowse)
	//ON_BN_CLICKED(IDC_RADIO_WHOLE_SYSTEM, &CWrdWizAutoScnDlg::OnBnClickedRadioWholeSystem)
	ON_BN_CLICKED(IDC_RADIO_SPECIFIC_PATH, &CWrdWizAutoScnDlg::OnBnClickedRadioSpecificPath)
	ON_BN_CLICKED(IDC_CHECK_SELECT_ALL, &CWrdWizAutoScnDlg::OnBnClickedCheckSelectAll)
	ON_BN_CLICKED(IDC_BUTTON_MINIMIZE, &CWrdWizAutoScnDlg::OnBnClickedButtonMinimize)
	//ON_BN_CLICKED(IDC_BUTTON_CLOSEUI, &CWrdWizAutoScnDlg::OnBnClickedButtonCloseui)
	ON_WM_SETCURSOR()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

HWINDOW   CWrdWizAutoScnDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWrdWizAutoScnDlg::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************************************
*  Function Name     :	  OnInitDialog
*  Description		 :	  This function is Called by the framework to exchange and validate dialog data.
*  Author Name		 :
*  Date				 :
****************************************************************************************************/
BOOL CWrdWizAutoScnDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	//AddUserAndSystemInfoToLog();
	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_FIVE);  // to register service for process protection

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_FIVE);//WLSRV_ID_FIVE to register service for process protection

	InitializeAllVariablesToZero();
	LoadRequiredModules();
	//Init();

	HideAllElements();

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TEMP_FILE_CLEANER.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TEMP_FILE_CLEANER.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	//initial window point center
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
	if (theApp.m_bTmpFilePage)
	{
		this->ShowTempFileCleaner();
	}

	this->SetWindowText(L"Vibranium TEMP CLEANER");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************************************
*  Function Name  : HideAllElements
*  Description    : Function to hide all elements applied when not scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 May 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::HideAllElements()
{
	m_Radio_AllDrives.ShowWindow(SW_HIDE);
	m_Radio_SpecificPath.ShowWindow(SW_HIDE);
	m_SpecificPath.ShowWindow(SW_HIDE);
	m_Browse.ShowWindow(SW_HIDE);
	//m_btnAutorunminimize.ShowWindow(SW_HIDE);
	//m_btnAutoRunClose.ShowWindow(SW_HIDE);
	m_btnCloseUI.ShowWindow(SW_HIDE);
	m_btnMinimize.ShowWindow(SW_HIDE);
	m_FilesList.ShowWindow(SW_HIDE);
	m_CurrentFile.ShowWindow(SW_HIDE);
	m_StartTime.ShowWindow(SW_HIDE);
	m_EndTime.ShowWindow(SW_HIDE);
	m_stTempCleanerHeader.ShowWindow(SW_HIDE);
	m_DuplicateFilesCountText.ShowWindow(SW_HIDE);
	m_Radio_AllDrives.ShowWindow(SW_HIDE);
	m_Close.ShowWindow(SW_HIDE);
	m_generateCache.ShowWindow(SW_HIDE);
	m_stTempFooter.ShowWindow(SW_HIDE);
	m_SelectAll.ShowWindow(SW_HIDE);
	m_stSelctAll.ShowWindow(SW_HIDE);

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
	AddLogEntry(L"InsertDataToTable in VibraniumAutoScnDlg- TempFileCleaner entered", 0, 0, true, ZEROLEVEL);
	try
	{
		m_objSqlDb.Open();

		int iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = m_objSqlDb.GetLastRowId();
		m_objSqlDb.Close();

		return iLastRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumAutoScnDlg::InsertDataToTable()", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	AddTempFileSessionEntryToDB
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 CWrdWizAutoScnDlg::AddTempFileSessionEntryToDB(INT64 iScanID, int iDetectedFiles, int iRepairedFiles)
{
	CString csInsertQuery;
	try
	{
		csInsertQuery = _T("INSERT INTO Wardwiz_TempFilesCleanerSessionDetails VALUES (null,");
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_TempFilesCleanerSessionDetails VALUES (null,Datetime('now','localtime'),Datetime('now','localtime'),Date('now'),Date('now'),%d,%d);"), iDetectedFiles, iRepairedFiles);

		CT2A ascii(csInsertQuery, CP_UTF8);
		INT64 iScanId = InsertDataToTable(ascii.m_psz);
		return iScanId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumAutoScnDlg::AddTempFileSessionEntryToDB Query : %s", csInsertQuery, 0, true, SECONDLEVEL);
		return 0;
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_OnBnClickedTempCleanFind
*  Description    : Function to Find temporary files
*  Author Name    : Amol Jaware
*  Date			  : 2 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_OnBnClickedTempCleanFind(SCITER_VALUE svFunScanFinishedCB, SCITER_VALUE OnBnClickedButtonCloseCB)
{
	m_svFunScanFinishedCB = svFunScanFinishedCB;
	m_svOnBnClickedButtonCloseCB = OnBnClickedButtonCloseCB;	
	m_iScanSessionId = AddTempFileSessionEntryToDB(0, m_dwTotDetectedFiles, m_dwTotalFilesCleaned);
	//m_bIsTempFileRepairFinished = false;
	m_hThreadStartScan = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_StartScan,
		(LPVOID)this, 0, 0);
	//Sleep(1000);
	if (!m_hThreadStartScan)
	{
		//m_generateCache.EnableWindow();
		AddLogEntry(L"Find temporary file(s) thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
		//::MessageBox(this->m_hWnd, L"Find temporary file(s) thread creation failed, please check memory usage.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_OnBnClickedTempCleanRemove
*  Description    : Function to remove temporary files
*  Author Name    : Amol Jaware
*  Date			  : 2 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_OnBnClickedTempFileClean(SCITER_VALUE svArrRecords, SCITER_VALUE svRemovedStatusCB)
{
	//m_generateCache.EnableWindow(FALSE);
	m_bIsTempFileFoundFinished = false; //when click on remove button it should be false
	bool bIsArray=false;
	svArrRecords.isolate();
	bIsArray = svArrRecords.is_array();
	try
	{
		if (!bIsArray)
		{
			return false;
		}
		//passing arguments value to member variables.
		m_svArrRecords = svArrRecords;
		m_svRemovedStatusCB = svRemovedStatusCB;
		//create thread to make changes status of found temp file.
		m_hThreadRepair = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_RepairVirusFiles, (LPVOID)this, 0, 0);
		//Sleep(1000);
		if (!m_hThreadRepair)
		{
			AddLogEntry(L"Remove temporary file(s) thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
			//exit(0);
			//m_generateCache.EnableWindow();
			m_Close.EnableWindow();
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::OnClickCleanButton", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
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
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    : The framework calls this function when Windows or an application makes a request 
                  : to repaint a portion of an application's window.
*  Author Name    : 
*  Date			  : 
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
*  Description    : call to obtain the cursor to display while the user drags the minimized window.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
HCURSOR CWrdWizAutoScnDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************************************
*  Function Name  : OnBnClickedOk
*  Description    : 
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedOk()
{
//	// TODO: Add your control notification handler code here
//	//CDialogEx::OnOK();
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCancel
*  Description    : This function is used to free used resources.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedCancel()
{
	//FreeUsedResources();
	OnCancel();
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonStartScan
*  Description    : This function is used to start thread to scan.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonStartScan()
{
	TCHAR	szSpecificPath[512] = { 0 };

	m_bScanStart = false;
	m_bRepairStart = false;

	m_bScanThreadFinished = false;
	m_generateCache.GetWindowTextW(szSpecificPath, 510);

	if (_wcsicmp(szSpecificPath, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_FIND_TEMP_FILE")) == 0x00)
	{
		
		m_generateCache.EnableWindow(FALSE);
		m_hThreadStartScan = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_StartScan,
			(LPVOID)this, 0, 0);
		//Sleep(1000);
		if (!m_hThreadStartScan)
		{
			m_generateCache.EnableWindow();
			AddLogEntry(L"Find temporary file(s) thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
			//::MessageBox(this->m_hWnd, L"Find temporary file(s) thread creation failed, please check memory usage.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}
	if (_wcsicmp(szSpecificPath, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_REMOVE_TEMP_FILE")) == 0x00)
	{
		m_generateCache.EnableWindow(FALSE);
		m_Close.EnableWindow(FALSE);
		m_hThreadRepair = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_RepairVirusFiles,
			(LPVOID)this, 0, 0);
		//Sleep(1000);
		if (!m_hThreadRepair)
		{
			AddLogEntry(L"Remove temporary file(s) thread creation failed, please check memory usage.\n", 0, 0, true, SECONDLEVEL);
			//::MessageBox(this->m_hWnd, L"Remove temporary file(s) thread creation failed, please check memory usage.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_generateCache.EnableWindow();
			m_Close.EnableWindow();
		}
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonClose
*  Description    : To close UI
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonClose()
{
	try
	{
		m_objSqlDb.Close();

		//FreeUsedResources();
		SCITER_VALUE svRet;
		if (m_bScanStarted)
		{
			if (m_bScanStart)
			{
				::SuspendThread(m_hThreadStartScan);
			}
			if (m_bRepairStart)
			{
				::SuspendThread(m_hThreadRepair);
			}
		}
		if (svRet == 0)
		{
			if (m_bScanStart)
			{
				::ResumeThread(m_hThreadStartScan);
			}
			if (m_bRepairStart)
			{
				::ResumeThread(m_hThreadRepair);
			}
			return;
		}
		if (m_bScanStarted)
		{
			if (m_bScanStart)
			{
				::TerminateThread(m_hThreadStartScan, 0);
			}
			if (m_bRepairStart)
			{
				::TerminateThread(m_hThreadRepair, 0);
			}
		}
		UpdateSessionRecord();
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::OnBnClickedButtonClose", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : On_PauseTempFileOperation
*  Description    : To pause once click on close button while temp file finder/removal is running.
*  Author Name    : Amol Jaware
*  Date			  : 6 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_PauseTempFileOperation()
{
	try
	{
		m_objSqlDb.Close();
		if (m_bScanStarted)
		{
			if (m_bScanStart)
			{
				::SuspendThread(m_hThreadStartScan);
			}
			if (m_bRepairStart)
			{
				::SuspendThread(m_hThreadRepair);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumAutoScnDlg::On_PauseTempFileOperation()", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ResumeTempFileOperation
*  Description    : To make resume once click on NO button (do you want to close NO).
*  Author Name    : Amol Jaware
*  Date			  : 6 June 2016
****************************************************************************************************/
json::value CWrdWizAutoScnDlg::On_ResumeTempFileOperation()
{
	try
	{
		if (m_bScanStarted)
		{
			if (m_bScanStart)
			{
				::ResumeThread(m_hThreadStartScan);
			}

			if (m_bRepairStart)
			{
				::ResumeThread(m_hThreadRepair);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumAutoScnDlg::On_ResumeTempFileOperation()", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonBrowse
*  Description    : To include files.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonBrowse()
{
	TCHAR	pszPath[512] = { 0 }; 

		BROWSEINFO        bi = { m_hWnd, NULL, pszPath, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_TO_SELECT_FOLDER_SCAN"), BIF_RETURNONLYFSDIRS, NULL, 0L, 0 };	// | BIF_BROWSEINCLUDEFILES
	LPITEMIDLIST      pIdl = NULL;

	LPITEMIDLIST  pRoot = NULL;
	bi.pidlRoot = pRoot;

	pIdl = SHBrowseForFolder(&bi);
	if (NULL != pIdl)
	{
		SHGetPathFromIDList(pIdl, pszPath);

		if (pszPath[0])
		{
			wcscpy_s(m_szSpecificPath, 511, pszPath);
			m_SpecificPath.SetWindowText(pszPath);
		}
	}
}

/***************************************************************************************************
*  Function Name  : InitializeAllVariablesToZero
*  Description    : To initialize all the declared variables.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::InitializeAllVariablesToZero()
{
	ZeroMemory(m_ApplRunningPath, sizeof(m_ApplRunningPath));
	ZeroMemory(m_szLogPath, sizeof(m_szLogPath));
	ZeroMemory(m_szLogComparePath, sizeof(m_szLogComparePath));
	ZeroMemory(m_szSpecificPath, sizeof(m_szSpecificPath));
	ZeroMemory(m_SysDir, sizeof(m_SysDir));
	ZeroMemory(m_WinDir, sizeof(m_WinDir));
	ZeroMemory(m_ProgPath, sizeof(m_ProgPath));
	ZeroMemory(m_Progx86Path, sizeof(m_Progx86Path));
	ZeroMemory(m_szProgressMsg, sizeof(m_szProgressMsg));
	

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
	m_dwTotDetectedFiles = 0x00;
	m_dwTotalFilesCleaned = 0x00;

	m_FilesList.DeleteAllItems();
	m_vFileCacheInfo.clear();
	m_bScanStarted = false;

	m_bX64 = false;

	m_pdwCrc32Table = NULL;
	FreeResource();

	m_ExcludeFols.RemoveAll();

	m_bScanStart = false;
	m_bRepairStart = false;
	//m_strListProcesses.RemoveAll();
}

/***************************************************************************************************
*  Function Name  : LoadRequiredModules
*  Description    : To load required modules.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
bool CWrdWizAutoScnDlg::LoadRequiredModules()
{
	GetSystemDirectory(m_SysDir, 255);
	try
	{
		if (m_SysDir[0])
			_wcsupr_s(m_SysDir, 255);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::LoadRequiredModules()", 0, 0, true, SECONDLEVEL);
	}
	SYSTEM_INFO	sysInfo = { 0 };

	GetSystemInfo(&sysInfo);

	if ((sysInfo.wProcessorArchitecture&PROCESSOR_ARCHITECTURE_AMD64) == PROCESSOR_ARCHITECTURE_AMD64)
		m_bX64 = true;

	GetWindowsDirectory(m_WinDir, 255);
	GetEnvironmentVariable(L"PROGRAMFILES", m_ProgPath, 255);

	if (m_bX64)
		GetEnvironmentVariable(L"PROGRAMFILES(X86)", m_Progx86Path, 255);

	if (m_WinDir)
	{
		_wcsupr_s(m_WinDir, 255);
		m_ExcludeFols.AddTail(m_WinDir);
	}

	if (m_ProgPath)
	{
		_wcsupr_s(m_ProgPath, 255);
		m_ExcludeFols.AddTail(m_ProgPath);
	}

	if (m_Progx86Path)
	{
		_wcsupr_s(m_Progx86Path, 255);
		m_ExcludeFols.AddTail(m_Progx86Path);
	}

	GetModuleFileName(NULL, m_ApplRunningPath, sizeof(m_ApplRunningPath) - 2);

	TCHAR	*pFileName = wcsrchr(m_ApplRunningPath, '\\');

	if (!pFileName)
		return false;

	*pFileName = '\0';

	TCHAR	szCompName[64] = { 0 };

	DWORD	dwSize = 63;

	GetComputerName(szCompName, &dwSize);

	swprintf(m_szLogPath, 511, L"%s\\LOG\\VIBOTEMPCLN.LOG", m_ApplRunningPath);

	return true;
}

/***************************************************************************************************
*  Function Name  : GenerateProcessKillList
*  Description    : Function which adds process in the list, such as WSCRIPT.EXE
*  Author Name    : Vilas suvarnakar
*  Date			  : 
****************************************************************************************************/
void CWrdWizAutoScnDlg::GenerateProcessKillList()
{
	TCHAR	szPath[512] = { 0 };

	swprintf_s(szPath, 511, L"%s\\WSCRIPT.EXE", m_SysDir);

	/*CString	strPath("");

	strPath.Format(L"%s\\WSCRIPT.EXE", m_SysDir);

	POSITION pos = m_strListProcesses.GetTailPosition();

	m_strListProcesses.InsertAfter(pos, strPath);
	*/
	m_strListProcesses.AddTail(szPath);
}

/***************************************************************************************************
*  Function Name  : KillSpecifiedRunningProcesses
*  Description    : function which terminates process which are detected as virus process.
*  Author Name    :
*  Date			  :
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

			_wcsupr_s(szProcPath, 1023);

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
		//AddLogEntry(L"### Exception in CVibraniumRKScnApp::EnumerateRunningProcessThread", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : SuspendProcessThreadsForcefully
*  Description    : To suspend specified process forcefully which dected as temp file.
*  Author Name    : 
*  Date			  :
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
	}

Cleanup:
	if (hThreadSnap != INVALID_HANDLE_VALUE)
		CloseHandle(hThreadSnap);

	hThreadSnap = INVALID_HANDLE_VALUE;

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : StartScanning
*  Description    : To start scan temp files.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::StartScanning()
{
	try
	{

		TCHAR	szDrives[256] = { 0 };
		m_dwTotalFilesCleaned = 0x00;
		m_dwScannedFiles = m_dwDetectedFiles = m_dwRepairedFiles = m_dwRebootFileDeletion = 0x00;
		//m_ListIndex = 0x00;
		m_dwScanStartTickCount = 0x00;
		m_FilesList.DeleteAllItems();
		m_vFileCacheInfo.clear();
		m_DuplicateFilesCountText.SetWindowTextW(L"");

		m_bScanStarted = true;

		ZeroMemory(&m_StartScanTime, sizeof(m_StartScanTime));
		GetSystemTime(&m_StartScanTime);

		m_dwScanStartTickCount = GetTickCount();
		AddLogEntry(L"Temporary file(s) finder Started...", 0, 0, true, SECONDLEVEL);

		ZeroMemory(m_szSpecificPath, sizeof(m_szSpecificPath));
		GetEnvironmentVariable(L"TEMP", m_szSpecificPath, 511);
		if (m_szSpecificPath[0])
		{
			_wcsupr_s(m_szSpecificPath, 511);
			EnumFolder(m_szSpecificPath);
		}

		//Sleep(1000);

		ZeroMemory(m_szSpecificPath, sizeof(m_szSpecificPath));
		GetWindowsDirectory(m_szSpecificPath, 511);
		if (m_szSpecificPath[0])
		{
			wcscat_s(m_szSpecificPath, 511, L"\\Temp");
			_wcsupr_s(m_szSpecificPath, 511);
			EnumFolder(m_szSpecificPath);
		}

		//m_svFunAddVirusFoundEntryCB.call(L"",L"",L"",m_szSpecificPath);
		//Added to show correct elapsed time
		//Added by Vilas on 19 / 08 / 2015
		Sleep(100);

		ZeroMemory(szDrives, sizeof(szDrives));
		swprintf_s(szDrives, 255, L"Temporary file(s) finder process finished. Temporary file(s)::%lu\n", m_ListIndex);
		AddLogEntry(szDrives, 0, 0, true, SECONDLEVEL);

		CString csStartTime;
		csStartTime.Format(_T("%02d-%02d-%02d %02d:%02d:%02d"), m_StartScanTime.wDay,m_StartScanTime.wMonth,m_StartScanTime.wYear, m_StartScanTime.wHour, m_StartScanTime.wMinute, m_StartScanTime.wSecond);

		m_bScanStarted = false;
		m_bScanStart = false;

		if (m_ListIndex)
		{ 
			m_SelectAll.EnableWindow();
			m_SelectAll.SetCheck(BST_CHECKED);
		}
	}
	catch (...)
	{
		//pHeuScnDlg->AddLogEntry(L"### Exception in Thread_GenerateUpdateFiles()");
	}
}

/***************************************************************************************************
*  Function Name  : DeleteFileForcefully
*  Description    : To delete detected temp files.
*  Author Name    :
*  Date			  :
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
		//AddLogEntry(L"### Exception in CScanDlg::EnumFolder", lpFolPath);
		dwRet = 0x02;
	}
Cleanup:

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : EnumFolder
*  Description    : Enumerates the folder and files.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::EnumFolder(LPCTSTR lpFolPath)
{
	try
	{

		Sleep(10);

		CFileFind finder;

		CString strWildcard(lpFolPath);

//		DWORD	dwAttributes = GetFileAttributes(lpFolPath);

//		if (INVALID_FILE_ATTRIBUTES != dwAttributes)
//			SetFileAttributes(lpFolPath, (dwAttributes & (~FILE_ATTRIBUTE_HIDDEN)));

		if (strWildcard.IsEmpty())
			return;

//		if (CheckPathInExcludeFolder(lpFolPath))
//			return;

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");


		BOOL bWorking = finder.FindFile(strWildcard);

		while (bWorking)
		{
			//Added to show correct elapsed time
			//Added by Vilas on 19 / 08 / 2015
			//SetElapsedTimetoGUI();

			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[1024] = { 0 };

			wsprintf(szPath, L"%s", finder.GetFilePath());

			_wcsupr_s(szPath, 1023);

			if (finder.IsDirectory())
			{
				EnumFolder(szPath);

				//Code added to remove folder
				//by Vilas on 19 / 11 / 2015
				CheckFileForViruses(szPath);
			}
			else
			{
				CheckFileForViruses(szPath);
			}
		}

		finder.Close();

	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CScanDlg::EnumFolder", lpFolPath);
	}
}

/***************************************************************************************************
*  Function Name  : CheckFileForViruses
*  Description    : To check files for viruses.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::CheckFileForViruses(LPCTSTR lpFileName)
{
	DWORD	dwRet = 0x00;
	HANDLE	hFileHandle = INVALID_HANDLE_VALUE;

	try
	{
		m_dwScannedFiles++;

		//m_CurrentFile.SetWindowTextW(lpFileName);

		TCHAR	szTemp[1024] = { 0 };

		//Added by Vilas on 19 / 08 / 2015
		//Issue reported by Harald during second scan means elapsed time jumps directly because no enumeration of files
		//SetElapsedTimetoGUI();

		TCHAR	szFileName[256] = { 0 };

		ZeroMemory(szTemp, sizeof(szTemp));
		wcscpy_s(szTemp, 1023, lpFileName);

		TCHAR	*pFileName = wcsrchr(szTemp, '\\');

		if (pFileName)
		{
			wcscpy_s(szFileName, 255, &pFileName[1]);

			*pFileName = '\0';
		}

		AddEntryinUITable(szFileName, szTemp);

		//m_dwTotalFilesCleaned = m_dwRebootFileDeletion + m_dwRepairedFiles + m_dwActioned;
		//m_iScanSessionId = AddTempFileSessionEntryToDB(0, dwTotDetectedFiles, m_dwTotalFilesCleaned);
		//m_iScanId = AddTempFileEntryToDB(m_iScanSessionId, szFilePath, szFileName, csActionId);

		AddLogEntry(lpFileName, 0, 0, true, SECONDLEVEL);

		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf(szTemp, 254, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_TEMP_FILE") , m_ListIndex);
		m_DuplicateFilesCountText.SetWindowTextW(szTemp);



	}
	catch (...)
	{
		dwRet = 0xFF;
	}
	return;
}

/***************************************************************************************************
*  Function Name  : SearchInFileCache
*  Description    : Function which search fileHash in listed file cache list.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
DWORD CWrdWizAutoScnDlg::SearchInFileCache(DWORD dwFileSize, DWORD dwFileChecksum)
{
	DWORD	dwRet = 0x00;
	bool	bFound = false;

	try
	{
		DWORD	dwSize = static_cast<DWORD>(m_vFileCacheInfo.size());

		if (!dwSize)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		if ((!dwFileSize) && (!dwFileChecksum))
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		DWORD	dwIter = 0x00;

		for (; dwIter<dwSize; dwIter++)
		{
			if ((m_vFileCacheInfo[dwIter].dwFileSize == dwFileSize) &&
				(m_vFileCacheInfo[dwIter].dwFileChecksum == dwFileChecksum))
			{
				bFound = true;
				break;
			}
		}

		if (bFound )
			dwRet = 0x00;
		else
			dwRet = 0x03;
	}
	catch (...)
	{
		dwRet = 0x04;
	}

Cleanup:

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : AddFileInfoInFileCache
*  Description    : Function which add file information like if hash value in list.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
DWORD CWrdWizAutoScnDlg::AddFileInfoInFileCache(DWORD dwFileSize, DWORD dwFileChecksum)
{
	DWORD	dwRet = 0x00;
	try
	{
		if ((!dwFileSize) && (!dwFileChecksum))
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		FILECACHEINFO	sFileGoodCacheInfo = { 0 };

		sFileGoodCacheInfo.dwFileSize = dwFileSize;
		sFileGoodCacheInfo.dwFileChecksum = dwFileChecksum;

		m_vFileCacheInfo.push_back(sFileGoodCacheInfo);
	}
	catch (...)
	{
	}

Cleanup:

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : Init
*  Description    : Function which initializes CRC table value.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::Init()
{
	// This is the official polynomial used by CRC32 in PKZip.
	// Often times the polynomial shown reversed as 0x04C11DB7.
	DWORD	dwPolynomial = 0xEDB88320;
	int		i, j;

	try
	{

		FreeResource();
		m_pdwCrc32Table = new DWORD[256];

		DWORD	dwCrc = 0x00;

		for (i = 0; i < 256; i++)
		{
			dwCrc = i;
			for (j = 8; j > 0; j--)
			{
				if (dwCrc & 1)
					dwCrc = (dwCrc >> 1) ^ dwPolynomial;
				else
					dwCrc >>= 1;
			}
		
			m_pdwCrc32Table[i] = dwCrc;
		}
	}
	catch (...)
	{
		// An unknown exception happened, or the table isn't initialized
	}
}

/***************************************************************************************************
*  Function Name  : FileCRC32Win32
*  Description    : Function which calculated CRC value 32 bit.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
DWORD CWrdWizAutoScnDlg::FileCRC32Win32(HANDLE hFile, DWORD &dwCrc32)
{
	
	DWORD	dwErrorCode = 0x00;
	
	dwCrc32 = 0xFFFFFFFF;

	try
	{
		// Is the table initialized?
		if (m_pdwCrc32Table == NULL)
		{
			dwErrorCode = 0x01;
			goto Cleanup;
		}

		
		BYTE	buffer[0x4000] = { 0 };
		DWORD	dwBytesRead = 0x00, dwLoop = 0x00;

		BOOL	 bSuccess = ReadFile(hFile, buffer, sizeof(buffer), &dwBytesRead, NULL);
		while (bSuccess && dwBytesRead)
		{
			for (dwLoop = 0; dwLoop < dwBytesRead; dwLoop++)
				CalculateCRC32(buffer[dwLoop], dwCrc32);

			bSuccess = ReadFile(hFile, buffer, sizeof(buffer), &dwBytesRead, NULL);
		}

	}
	catch (...)
	{
		// An unknown exception happened, or the table isn't initialized
		dwErrorCode = 0x02;
	}

	dwCrc32 = ~dwCrc32;

Cleanup:

	return dwErrorCode;
}

/***************************************************************************************************
*  Function Name  : CalculateCRC32
*  Description    : Function which calculated CRC value 32 bit.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::CalculateCRC32(const BYTE byte, DWORD &dwCrc32)
{
	dwCrc32 = ((dwCrc32) >> 8) ^ m_pdwCrc32Table[(byte) ^ ((dwCrc32)& 0x000000FF)];
}

/***************************************************************************************************
*  Function Name  : CheckForAutorunInfFile
*  Description    : To check autorun in file.
*  Author Name    :
*  Date			  :
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
	}

Cleanup:

	return bVirusFound;
}

/***************************************************************************************************
*  Function Name  : CheckForAutorunLNKFile
*  Description    : To check autorun LNK file in file.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
bool CWrdWizAutoScnDlg::CheckForAutorunLNKFile(LPBYTE	lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound)
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

		if ( (*((DWORD *)&lpbBuffer[0x54]) != 0x3AEA20D0) ||
			(*((DWORD *)&lpbBuffer[0x58]) != 0xD8A21069) || 
			(*((DWORD *)&lpbBuffer[0x5C]) != 0x302B0008)
			)
			goto Cleanup;

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

		DWORD dwIter = 0x00;
		dwBytesRead -= 0x49;
		for (; dwIter<dwBytesRead; dwIter++)
		{

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

	}
	catch (...)
	{
	}

Cleanup:

	return bFound;
}

/***************************************************************************************************
*  Function Name  : CheckForThumbsDBFile
*  Description    : To chekc for thumbsDB files.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
bool CWrdWizAutoScnDlg::CheckForThumbsDBFile(LPBYTE	lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;

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
	}

Cleanup:

	return bVirusFound;
}

/***************************************************************************************************
*  Function Name  : CheckForCantix
*  Description    : To check for Cantix file.
*  Author Name    :
*  Date			  :
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
	}

Cleanup:

	return bVirusFound;
}

/***************************************************************************************************
*  Function Name  : CheckforDesktopVBS
*  Description    : To check for DesktopVBS files.
*  Author Name    :
*  Date			  :
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
	}

Cleanup:

	return bVirusFound;
}

/***************************************************************************************************
*  Function Name  : AddEntryinUITable
*  Description    : Add temp file entry in table.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::AddEntryinUITable(LPCTSTR lpFileName, LPCTSTR lpFilePath, CString lpAction)
{
	m_dwDetectedFiles++;
	CString csFileDetails;
	csFileDetails += lpFilePath;
	csFileDetails += L"\\";
	csFileDetails += lpFileName;
	TCHAR m_szUSBFilePath[MAX_PATH];
	memset(m_szUSBFilePath, 0, sizeof(m_szUSBFilePath));
	//GetShortPathName(lpFilePath, (LPWSTR)lpFileName, 60);

	//GetShortPathName(csFilePath, NULL, 0);
	sciter::value map;
	map.set_item("one", lpFileName);
	map.set_item("two", lpFilePath);
	map.set_item("three", (SCITER_STRING)lpAction);

	sciter::dom::element ela = m_root_el;
	//const  SCITER_VALUE arrValues[3] = { (SCITER_STRING)csFilePath, iThreatsFoundCount, iFileScanned };
	BEHAVIOR_EVENT_PARAMS params;
	params.cmd = SETFILEPATH_EVENT_CODE;
	params.he = params.heTarget = ela;
	params.data = map;
	ela.fire_event(params, true);
	
}

/***************************************************************************************************
*  Function Name  : UnhideDirectory
*  Description    : To find temp file in unhide directory.
*  Author Name    :
*  Date			  :
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
	}

Cleanup:

	return bRet;
}

/***************************************************************************************************
*  Function Name  : DeleteFileForcefully
*  Description    : To delete temp file forcefully.
*  Author Name    :
*  Date			  :
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


		//Code added to remove folder
		//by Vilas on 19 / 11 / 2015
		if (PathIsDirectory(lpFilePath))
		{
			if (RemoveDirectory(lpFilePath))
				goto Cleanup;

			TCHAR	szFrom[1024] = { 0 };

			SHFILEOPSTRUCT stSHFileOpStruct = { 0 };


			wcscpy_s(szFrom, 1022, lpFilePath);

			szFrom[wcslen(szFrom)] = '\0';
			szFrom[wcslen(szFrom)+1] = '\0';

			stSHFileOpStruct.wFunc = FO_DELETE;
			
			stSHFileOpStruct.pFrom = szFrom;
			
			stSHFileOpStruct.fFlags = FOF_NOERRORUI | FOF_MULTIDESTFILES | FOF_NOCONFIRMATION | FOF_SILENT | FOF_MULTIDESTFILES;
			
			stSHFileOpStruct.fAnyOperationsAborted = FALSE;
			int nFileDeleteOprnRet = SHFileOperation(&stSHFileOpStruct);

			if (!nFileDeleteOprnRet)
				goto Cleanup;
		}

		MoveFileEx(lpFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		m_dwRebootFileDeletion++;

		dwRet = 0x02;

	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CScanDlg::EnumFolder", lpFolPath);
		dwRet = 0x03;
	}

Cleanup:

	return dwRet;

}

/***************************************************************************************************
*  Function Name  : OnBnClickedRadioSpecificPath
*  Description    : 
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedRadioSpecificPath()
{
	m_Radio_SpecificPath.SetCheck(BST_CHECKED);
	//m_Radio_AllDrives.SetCheck( BST_UNCHECKED );

	m_SpecificPath.EnableWindow();
	m_Browse.EnableWindow();

	m_SpecificPath.SetFocus();

	m_bAllDrives = false;
}


/***************************************************************************************************
*  Function Name  : Thread_StartScan
*  Description    : WholeSystem
*  Author Name    :
*  Date			  :
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

		pCacheGenDlg->m_bScanStart = true;

		ZeroMemory(pCacheGenDlg->m_szProgressMsg, sizeof(pCacheGenDlg->m_szProgressMsg));
		wcscpy_s(pCacheGenDlg->m_szProgressMsg, 255, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_FIND_INPROGRESS"));

		swprintf_s(szTemp, 255, L"%s %.2d/%.2d/%.2d %.2d:%.2d:%.2d", theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_START_TIME"), sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		
		//pCacheGenDlg->KillSpecifiedRunningProcesses();
		
		pCacheGenDlg->StartScanning();
	}
	catch (...)
	{
		//pHeuScnDlg->AddLogEntry(L"### Exception in Thread_GenerateUpdateFiles()");
	}

Cleanup:

	if (pCacheGenDlg)
	{

		pCacheGenDlg->m_generateCache.EnableWindow();
		if (pCacheGenDlg->m_ListIndex > 0x00)
		{
			//pCacheGenDlg->m_generateCache.EnableWindow();
			pCacheGenDlg->m_generateCache.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_REMOVE_TEMP_FILE"));
			pCacheGenDlg->m_SelectAll.SetCheck(BST_CHECKED);
		}

		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, 255, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_FINAL_STATUS"), pCacheGenDlg->m_dwScannedFiles, pCacheGenDlg->m_ListIndex);

		pCacheGenDlg->m_CurrentFile.SetWindowTextW(szTemp);
	}

	CString csInsertQuery = 0;
	csInsertQuery.Format(_T("UPDATE Wardwiz_TempFilesCleanerSessionDetails SET db_CleanerSessionEndDate = Date('now'),db_CleanerSessionEndTime = Datetime('now', 'localtime'), db_FilesScanCount = %d,db_FilesCleanCount = %d WHERE db_TempCleanerSessionID = %I64d;"), pCacheGenDlg->m_dwDetectedFiles, pCacheGenDlg->m_dwTotalFilesCleaned, pCacheGenDlg->m_iScanSessionId);

	CT2A ascii(csInsertQuery, CP_UTF8);

	/*if (!PathFileExistsA(g_strDatabaseFilePath))
	{
		pCacheGenDlg->m_objSqlDb.Open();
		pCacheGenDlg->m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
		pCacheGenDlg->m_objSqlDb.Close();
	}*/
	pCacheGenDlg->InsertDataToTable(ascii.m_psz);
	//pCacheGenDlg->SetForegroundWindowAndBringToTop(pCacheGenDlg->m_MainWindow);
	pCacheGenDlg->ShowWindow(SW_RESTORE);
	//::SendMessage(pCacheGenDlg->m_MainWindow, SW_RESTORE, 0, 0);
	pCacheGenDlg->m_bIsTempFileFoundFinished = true;
	pCacheGenDlg->m_svFunScanFinishedCB.call();
	//pCacheGenDlg->m_bScanThreadFinished = true;
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	AddTempFileEntryToDB
*  Description    :	Generates appropriate query and invokes function, to add temp files data to
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Sep 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 CWrdWizAutoScnDlg::AddTempFileEntryToDB(INT64 iScanID, CString csFilePath, CString csVirusName, CString csAction)
{
	CString csInsertQuery;
	try
	{
		csInsertQuery = _T("INSERT INTO Wardwiz_TempFilesCleanerDetails VALUES (null,");

		csVirusName.Replace(L"'", L"''");
		csFilePath.Replace(L"'", L"''");

		csInsertQuery.Format(_T("INSERT INTO Wardwiz_TempFilesCleanerDetails VALUES (null,%I64d,Datetime('now','localtime'),Datetime('now','localtime'),Date('now'),Date('now'),'%s','%s','%s');"), iScanID, csVirusName, csFilePath, csAction);

		//CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
		//objSqlDb.Open();
		//objSqlDb.CreateWardwizSQLiteTables();
		//objSqlDb.Close();

		CT2A ascii(csInsertQuery, CP_UTF8);

		INT64 iScanId = InsertDataToTable(ascii.m_psz);

		return iScanId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::AddTempFileEntryToDB", csInsertQuery, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : Thread_RepairVirusFiles
*  Description    : To repair virus infected files.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
DWORD WINAPI Thread_RepairVirusFiles(LPVOID lpParam)
{

	CWrdWizAutoScnDlg	*pCacheGenDlg = (CWrdWizAutoScnDlg *)lpParam;

	DWORD dwTotDetectedFiles = 0x00;
	DWORD		dwTicked = 0x00;
	bool		bOperation = true;
	DWORD		dwRet = 0x00;
	int			iIndex = 0;
	DWORD		dwDetectedFiles = 0x00;
	SYSTEMTIME	sysTime = { 0 };
	TCHAR		szTemp[1024] = { 0 };

	TCHAR	szFilePath[1024] = { 0 };
	TCHAR	szFileName[512] = { 0 };
	TCHAR	szAction[512] = { 0 };
	CString csActionId;
	pCacheGenDlg->m_dwActioned = 0x00;
	GetSystemTime(&sysTime);

	try
	{

		if (!pCacheGenDlg)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		pCacheGenDlg->m_bRepairStart = true;

//		ZeroMemory(pCacheGenDlg->m_szProgressMsg, sizeof(pCacheGenDlg->m_szProgressMsg));
//		wcscpy_s(pCacheGenDlg->m_szProgressMsg, 255, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_INPROGRESS"));

		pCacheGenDlg->m_dwRebootFileDeletion = pCacheGenDlg->m_dwFailed = pCacheGenDlg->m_dwRepairedFiles = 0x00;

		//dwTotDetectedFiles = pCacheGenDlg->m_FilesList.GetItemCount();
		dwTotDetectedFiles = pCacheGenDlg->m_svArrRecords.length();
		
		::SetForegroundWindow(pCacheGenDlg->m_hWnd);
		if (!dwTotDetectedFiles)
		{
			pCacheGenDlg->m_svRemovedStatusCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_NO_FILE_FOUND"));
			csActionId = "IDS_TEMP_FILE_CLEAN_NO_FILE_FOUND";
			goto Cleanup;
		}

		AddLogEntry(L"Remove temporary file(s) started...",0,0,true,FIRSTLEVEL);

		pCacheGenDlg->m_bScanStarted = true;
		
		bool	bChecked = false;
		int iDetectedFiles = 0;

		while (true)
		{
			dwDetectedFiles = pCacheGenDlg->m_svArrRecords.length();
			
			if (!dwDetectedFiles)
				break;
			dwDetectedFiles--;
			bChecked = false;
			for (iDetectedFiles = dwDetectedFiles; iDetectedFiles >= 0; iDetectedFiles--)
			{
				const SCITER_VALUE svEachEntry = pCacheGenDlg->m_svArrRecords[iDetectedFiles];
				const std::wstring chFileName = svEachEntry[L"FileName"].get(L"");
				const std::wstring chFilePath = svEachEntry[L"FilePath"].get(L"");
				const std::wstring chActionTaken = svEachEntry[L"ActionTaken"].get(L"");
				bool bValue = svEachEntry[L"selected"].get(false);

				if(!bValue)
					continue;

				bChecked = true;
				bOperation = false;
				dwTicked++;

				
				
				ZeroMemory(szFilePath, sizeof(szFilePath));
				ZeroMemory(szFileName, sizeof(szFileName));
				ZeroMemory(szAction, sizeof(szAction));

				const size_t lFilePathlen = wcslen(chFilePath.c_str());
				const size_t lFileNamelen = wcslen(chFileName.c_str());
				const size_t lActionTakenlen = wcslen(chActionTaken.c_str());

				wcscpy_s(szFilePath, lFilePathlen + 1, chFilePath.c_str());
				wcscpy_s(szFileName, lFileNamelen + 1 , chFileName.c_str());
				wcscpy_s(szAction, lActionTakenlen + 1, chActionTaken.c_str());

				if (_wcsicmp(szAction, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED")) != 0x00)
				{
					pCacheGenDlg->m_dwActioned++;
					csActionId = "IDS_CONSTANT_THREAT_DETECTED";
					continue;
				}

				wcscat_s(szFilePath, 1023, L"\\");
				wcscat_s(szFilePath, 1023, szFileName);

				ZeroMemory(szTemp, sizeof(szTemp));
				swprintf_s(szTemp, 1023, L"%s ...%s", theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVING"), szFilePath);
				csActionId = "IDS_TEMP_FILE_CLEAN_REMOVING";
				DWORD dwRet = pCacheGenDlg->DeleteFileForcefully(szFilePath, true);

				switch (dwRet)
				{
				case 0x00:
					pCacheGenDlg->m_dwRepairedFiles++;
					AddLogEntry(L"Removed successfully.\tFile::", szFilePath, 0, true, SECONDLEVEL);
					pCacheGenDlg->funcCleanTempFileFoundEntry(iDetectedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVED"), 1);
					csActionId = "IDS_TEMP_FILE_CLEAN_REMOVED";
					continue;
					
				case 0x01:
					pCacheGenDlg->m_dwFailed++;
					AddLogEntry(L"Remove failed(01).\tFile::", szFilePath, 0, true, SECONDLEVEL);
					pCacheGenDlg->funcCleanTempFileFoundEntry(iDetectedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND"), 2);
					csActionId = "IDS_CONSTANT_THREAT_NO_FILE_FOUND";
					continue;

				case 0x02:
					AddLogEntry(L"Removed on Reboot.\tFile::", szFilePath, 0, true, SECONDLEVEL);
					csActionId = "IDS_AUTO_RUN_SCAN_RESTART_REQUIRED";
					pCacheGenDlg->funcCleanTempFileFoundEntry(iDetectedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_RESTART_REQUIRED"), 2);
					continue;

				case 0x03:
					pCacheGenDlg->m_dwFailed++;
					AddLogEntry(L"Remove failed(03).\tFile::", szFilePath, 0, true, SECONDLEVEL);
					pCacheGenDlg->funcCleanTempFileFoundEntry(iDetectedFiles, theApp.m_objwardwizLangManager.GetString(L"Removed(03)"), 2);
					csActionId = "IDS_TEMP_FILE_CLEAN_REMOVED";
					continue;
				}
			}

			//if (pCacheGenDlg->m_dwRepairedFiles>0)
			pCacheGenDlg->m_dwTotDetectedFiles = dwTotDetectedFiles;
			pCacheGenDlg->m_dwTotalFilesCleaned = pCacheGenDlg->m_dwRebootFileDeletion + pCacheGenDlg->m_dwRepairedFiles + pCacheGenDlg->m_dwActioned;

			CString csInsertQuery = 0;
			csInsertQuery.Format(_T("UPDATE Wardwiz_TempFilesCleanerSessionDetails SET db_CleanerSessionEndDate = Date('now'),db_CleanerSessionEndTime = Datetime('now', 'localtime'), db_FilesScanCount = %d,db_FilesCleanCount = %d WHERE db_TempCleanerSessionID = %I64d;"), pCacheGenDlg->m_dwDetectedFiles, pCacheGenDlg->m_dwTotalFilesCleaned, pCacheGenDlg->m_iScanSessionId);

			CT2A ascii(csInsertQuery, CP_UTF8);

			if (!PathFileExistsA(g_strDatabaseFilePath))
			{
			pCacheGenDlg->m_objSqlDb.Open();
			pCacheGenDlg->m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
			pCacheGenDlg->m_objSqlDb.Close();
			}
			pCacheGenDlg->InsertDataToTable(ascii.m_psz);
			//pCacheGenDlg->m_iScanSessionId = pCacheGenDlg->AddTempFileSessionEntryToDB(0, dwTotDetectedFiles, pCacheGenDlg->m_dwTotalFilesCleaned);
			//pCacheGenDlg->m_iScanId = pCacheGenDlg->AddTempFileEntryToDB(pCacheGenDlg->m_iScanSessionId, szFilePath, szFileName, csActionId);

			if (!bChecked)
				break;
			else
			{
				if (iDetectedFiles < 0)
					break;
				else
					iDetectedFiles = dwDetectedFiles;
				/*if ((DWORD)iIndex >= dwDetectedFiles)
					break;
				else
				{
					iIndex = 0;
				}*/
			}
		}
	}
	catch (...)
	{
		//pHeuScnDlg->AddLogEntry(L"### Exception in Thread_GenerateUpdateFiles()");
	}

Cleanup:

	if (pCacheGenDlg)
	{
		pCacheGenDlg->m_bRepairStart = false;
		pCacheGenDlg->m_bScanStarted = false;
		//pCacheGenDlg->m_FilesList.EnableWindow();
		::SetForegroundWindow(pCacheGenDlg->GetSafeHwnd());
		//pCacheGenDlg->SetForegroundWindowAndBringToTop(pCacheGenDlg->m_MainWindow);
		pCacheGenDlg->ShowWindow(SW_RESTORE);
		if (bOperation)
		{
			//call call back function
			//pCacheGenDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_NO_FILE_SELECTED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			pCacheGenDlg->m_svRemovedStatusCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_NO_FILE_SELECTED"));
			AddLogEntry(L"No entries selected for repair.\n", 0, 0, true, SECONDLEVEL);
		}
		else
		{
			if ((dwTicked == pCacheGenDlg->m_dwActioned) && (pCacheGenDlg->m_dwActioned > 0x00))
			{
				AddLogEntry(L"Please select detected file(s) to repair.\n", 0, 0, true, SECONDLEVEL);
				//pCacheGenDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_SELECT_REPAIR"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				pCacheGenDlg->m_svRemovedStatusCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_SELECT_REPAIR"));
			}
			else
			{
				ZeroMemory(szTemp, sizeof(szTemp));
				//swIDS_TEMP_FILE_CLEAN_REMOVED_FILESprintf_s(szTemp, 1023, L"%s  %s , %lu  %s d %lu  %s , %lu ",
					//theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVAL_FINISHED"),theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVED_FILES"), pCacheGenDlg->m_dwRepairedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REBOOT_REMOVE_FILES"), pCacheGenDlg->m_dwRebootFileDeletion, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_TOTAL_FILES"), dwTotDetectedFiles);

				swprintf_s(szTemp, 1023, L"%s  %lu  %s  %lu,  %s",
					theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVED_FILES"), pCacheGenDlg->m_dwRepairedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REBOOT_REMOVE_FILES"), pCacheGenDlg->m_dwRebootFileDeletion, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVAL_FINISHED"));
				AddLogEntry(szTemp, 0, 0, true, SECONDLEVEL);

				ZeroMemory(szTemp, sizeof(szTemp));
				if (pCacheGenDlg->m_dwRebootFileDeletion)
				{
					//swprintf_s(szTemp, 1023, L"%s  %lu , %s  %lu , %s  %lu , %s",
						//theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVED_FILES"), pCacheGenDlg->m_dwRepairedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REBOOT_REMOVE_FILES"), pCacheGenDlg->m_dwRebootFileDeletion, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_TOTAL_FILES"), dwTotDetectedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REBOOT_TO_FINISH"));

					swprintf_s(szTemp, 1023, L"%s  %lu  %s  %lu,  %s",
						theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVED_FILES"), pCacheGenDlg->m_dwRepairedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REBOOT_REMOVE_FILES"), pCacheGenDlg->m_dwRebootFileDeletion, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REBOOT_TO_FINISH"));
				}
				else
				{
					//swprintf_s(szTemp, 1023, L"%s  %lu , %s  %lu , %s",
						//theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVED_FILES"), pCacheGenDlg->m_dwRepairedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_TOTAL_FILES"), dwTotDetectedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVAL_FINISHED"));

					swprintf_s(szTemp, 1023, L"%s  %lu,  %s ",
						theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVED_FILES"), pCacheGenDlg->m_dwRepairedFiles, theApp.m_objwardwizLangManager.GetString(L"IDS_TEMP_FILE_CLEAN_REMOVAL_FINISHED"));
				}

				//pCacheGenDlg->MessageBox(szTemp, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
				pCacheGenDlg->m_svRemovedStatusCB.call(szTemp);

			}
		}

		if (!pCacheGenDlg->SendData2Tray(RELOAD_WIDGETS_UI, RELOAD_TEMPORARY_FILES))
		{
			AddLogEntry(L"### Failed to SendData2Tray in Thread_RepairVirusFiles", 0, 0, true, SECONDLEVEL);
		}

		if ((pCacheGenDlg->m_dwRepairedFiles + pCacheGenDlg->m_dwRebootFileDeletion + pCacheGenDlg->m_dwFailed + pCacheGenDlg->m_dwActioned)
			== dwTotDetectedFiles)
		{
			/*pCacheGenDlg->m_generateCache.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_FIND_TEMP_FILE"));
			pCacheGenDlg->m_SelectAll.EnableWindow(FALSE);
			pCacheGenDlg->m_SelectAll.SetCheck(BST_UNCHECKED);*/
		}
		else
		{
		//pCacheGenDlg->m_SelectAll.EnableWindow();
		//pCacheGenDlg->m_SelectAll.SetCheck(BST_UNCHECKED);
		}
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCheckSelectAll
*  Description    : To select all files through the selectall button.
*  Author Name    :
*  Date			  :
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
*  Description    : To select files in the list view.
*  Author Name    :
*  Date			  :
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
*  Function Name  : FreeResource
*  Description    : To make free resources.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::FreeResource()
{
	try
	{
		delete [] m_pdwCrc32Table;
		m_pdwCrc32Table = NULL;
	}
	catch (...)
	{

	}
}

/***************************************************************************************************
*  Function Name  : CheckPathInExcludeFolder
*  Description    : To check path in exclude folder.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
bool CWrdWizAutoScnDlg::CheckPathInExcludeFolder(LPCTSTR lpFolPath)
{
	bool	bFound = false;

	DWORD	dwCount = static_cast<DWORD>(m_ExcludeFols.GetCount());

	POSITION	pos = m_ExcludeFols.GetHeadPosition();

	if (!pos)
		return bFound;

	while(pos)
	{
		CString csFolPath = m_ExcludeFols.GetAt(pos);

		if (_memicmp(lpFolPath, csFolPath, csFolPath.GetLength()) == 0x00 )
		{
			bFound = true;
			break;
		}

		m_ExcludeFols.GetNext(pos);
	}
	return bFound;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonMinimize
*  Description    : To make the minimize UI.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnBnClickedButtonMinimize()
{
	this->ShowWindow(SW_MINIMIZE);
}

/***************************************************************************************************
*  Function Name  : On_OnCloseButton
*  Description    : To close the UI.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
json::value  CWrdWizAutoScnDlg::On_OnCloseButton()
{
	m_bIsTempFileFoundFinished = false; //to avoid redundancy of detected msg. do u want to close?
	OnBnClickedButtonClose();
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnSetCursor
*  Description    : To set the cursor on information button.
*  Author Name    :
*  Date			  :
****************************************************************************************************/
BOOL CWrdWizAutoScnDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!pWnd)
		return FALSE;

	// hand cursor on information button neha gharge 22/5/2014.
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if (
		iCtrlID == IDC_BUTTON_GENERATE_CACHE ||
		iCtrlID == IDC_BUTTON_CLOSE ||
		iCtrlID == IDC_BUTTON_MINIMIZE ||
		iCtrlID == IDC_CHECK_SELECT_ALL ||
		iCtrlID == IDC_BUTTON_CLOSEUI )
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
*  Function Name  : SetForegroundWindowAndBringToTop
*  Description    : Function which takes the window as topmost window.
*  Author Name    :
*  Date			  :
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

/***************************************************************************************************
*  Function Name  : OnClose
*  Description    : To close the window.
*  Author Name    : 
*  Date			  :
****************************************************************************************************/
void CWrdWizAutoScnDlg::OnClose()
{
	SCITER_VALUE svIsDetectedFile = m_root_el.call_function("bISDetectedEntery");//taskbar close from frontend

	// TODO: Add your message handler code here and/or call default
	//OnBnClickedButtonClose();
	//CJpegDialog::OnClose();
}

//Added by Vilas on 19 / 08 / 2015
//Issue reported by Harald during second scan means elapsed time jumps directly because no enumeration of files
/***************************************************************************************************
*  Function Name  : SetElapsedTimetoGUI
*  Description    : Total time to be taken to find temp files.
*  Author Name    :
*  Date			  :
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
		OnBnClickedButtonMinimize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::On_Minimize", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : funcCleanTempFileFoundEntry
*  Description    : Removed Temp File Updatation.
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date			  : 09 Sep 2016
****************************************************************************************************/
void CWrdWizAutoScnDlg::funcCleanTempFileFoundEntry(int iDetectedFilesCount, CString csDetectedFileMsg, int iRemovedTempFileStatus)
{
	try
	{ 
		sciter::value map;
		map.set_item("one", iDetectedFilesCount);
		map.set_item("two", (SCITER_STRING)csDetectedFileMsg);
		map.set_item("three", iRemovedTempFileStatus);

		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETFILEPATH_UPDATE_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::funcCleanTempFileFoundEntry", 0, 0, true, SECONDLEVEL);
	}
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
*  Function Name  : UpdateSessionRecord
*  Description    : Function to update session record
*  Author Name    : Jeena Mariam Saji
*  Date			  : 8 Feb 2017
****************************************************************************************************/
void CWrdWizAutoScnDlg::UpdateSessionRecord()
{
	try
	{
		CString csInsertQuery = 0;
		if (m_dwTotalFilesCleaned == 0)
		{
			m_dwTotalFilesCleaned = m_dwRebootFileDeletion + m_dwRepairedFiles + m_dwActioned;
		}
		csInsertQuery.Format(_T("UPDATE Wardwiz_TempFilesCleanerSessionDetails SET db_CleanerSessionEndDate = Date('now'),db_CleanerSessionEndTime = Datetime('now', 'localtime'), db_FilesScanCount = %d,db_FilesCleanCount = %d WHERE db_TempCleanerSessionID = %I64d;"), m_dwDetectedFiles, m_dwTotalFilesCleaned, m_iScanSessionId);
		CT2A ascii(csInsertQuery, CP_UTF8);

		if (!PathFileExistsA(g_strDatabaseFilePath))
		{
			m_objSqlDb.Open();
			m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
			m_objSqlDb.Close();
		}
		InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::UpdateSessionRecord", 0, 0, true, SECONDLEVEL);
	}
}

/****************************************************************************************************
*	Function Name : ShowTempFileCleaner
*	Description	  : Function to open UI Temp File page.
*	Author Name   :	Amol Jaware
*	Date		  :	11/Oct/2017
*****************************************************************************************************/
void CWrdWizAutoScnDlg::ShowTempFileCleaner()
{
	try
	{
		const SCITER_VALUE result = m_root_el.call_function("CallTempFileCleaner");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::ShowTempFileCleaner", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SendData2Tray
*  Description    : Function which send message data to Tray application.
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date           : 9th Nov,2017
****************************************************************************************************/
bool CWrdWizAutoScnDlg::SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;

		CISpyCommunicator objCom(TRAY_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardwizAutoScnDlg::SendData2Tray", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CWardwizAutoScnDlg::SendData2Tray", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Jeena Mariam Saji
*  Date			  : 05 Dec 2018
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