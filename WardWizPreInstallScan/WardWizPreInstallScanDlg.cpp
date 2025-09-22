/*********************************************************************
*  Program Name		: WardWizPreInstallScanDlg.cpp
*  Description		: CWardWizPreInstallScanDlg Implementation
*  Author Name		: Nitin Shelar
*  Date Of Creation	: 21 Nov 2018
**********************************************************************/
#include "stdafx.h"
#include <Psapi.h>
#include "WardWizPreInstallScan.h"
#include "WardWizPreInstallScanDlg.h"
#include "afxdialogex.h"
#include "WrdWizSystemInfo.h"
#include "Enumprocess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		SETFILEPATH_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 1)
#define		SETFILECOUNT_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 2)
#define		SESSION_MANAGER_REG				L"SYSTEM\\CurrentControlSet\\Control\\Session Manager"
#define		BOOT_EXEC_REG					L"BootExecute"

DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam);
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam);
DWORD WINAPI QuarantineThread(LPVOID lpParam);

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_WARDWIZPREINSTALLSCAN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

/***************************************************************************************************
*  Function Name  : CAboutDlg
*  Description    : C'tor
*  Author Name    : Nitn Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name	  : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

/***************************************************************************
Function Name  : CWardWizPreInstallScanDlg
Description    : Constructor
Author Name    : Nitin Shelar
Date           : 21/11/2018
SR_NO		   :
****************************************************************************/
CWardWizPreInstallScanDlg::CWardWizPreInstallScanDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWardWizPreInstallScanDlg::IDD, pParent), behavior_factory("WardWizPreInsScanDlg")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	EnumProcessModulesWWizEx = NULL;
	m_pbyEncDecKey = NULL;
	m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE, sizeof(unsigned char));
	m_hThread_ScanCount = NULL;
	m_hQuarantineThread = NULL;
	m_hWardWizAVThread = NULL;
	m_hPsApiDLL = NULL;
}

/***************************************************************************
Function Name  : ~CWardWizPreInstallScanDlg
Description    : Dest'r
Author Name    : Nitin Shelar
Date           : 21/11/2018
SR_NO		   :
****************************************************************************/
CWardWizPreInstallScanDlg::~CWardWizPreInstallScanDlg()
{
	if (m_hThread_ScanCount != NULL)
	{
		SuspendThread(m_hThread_ScanCount);
		TerminateThread(m_hThread_ScanCount, 0);
		m_hThread_ScanCount = NULL;
	}

	if (m_hWardWizAVThread != NULL)
	{
		SuspendThread(m_hWardWizAVThread);
		TerminateThread(m_hWardWizAVThread, 0);
		m_hWardWizAVThread = NULL;
	}

	if (m_hQuarantineThread != NULL)
	{
		SuspendThread(m_hQuarantineThread);
		TerminateThread(m_hQuarantineThread, 0);
		m_hQuarantineThread = NULL;
	}

	UnLoadSignatureDatabase();

	if (m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}

	if (m_hPsApiDLL != NULL)
	{
		FreeLibrary(m_hPsApiDLL);
		m_hPsApiDLL = NULL;
	}
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
void CWardWizPreInstallScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizPreInstallScanDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CWardWizPreInstallScanDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

HWINDOW   CWardWizPreInstallScanDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizPreInstallScanDlg::get_resource_instance() { return theApp.m_hInstance; }


/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Initializes the dialog window
*  Author Name    : Nitin Shelar
*  Date			  : 21/11/2018
****************************************************************************************************/
BOOL CWardWizPreInstallScanDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_PREINSTALATION_SCAN.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_PREINSTALATION_SCAN.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	int ixRect = pIntMaxWidth;
	int iyRect = pIntHeight;

	sciter::dom::element root = sciter::dom::element::root_element(this->get_hwnd());
	SciterGetElementIntrinsicWidths(root, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(root, pIntMinWidth, &pIntHeight);
	
	// TODO: Add extra initialization here

	this->SetWindowText(L"VibraniumPreInsScan");
	// TODO: Add extra initialization here
	::MoveWindow(this->get_hwnd(), ixRect / 2, iyRect / 2, pIntMaxWidth, pIntHeight, true);
	try
	{
		APPBARDATA abd;
		abd.cbSize = sizeof(abd);

		SHAppBarMessage(ABM_GETTASKBARPOS, &abd);

		switch (abd.uEdge)
		{
		case ABE_TOP:
			SetWindowPos(NULL, ixRect, i, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizPreInstallScanDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	return TRUE; // return TRUE  unless you set the focus to a control
}

/***************************************************************************************************
*  Function Name  :  OnSysCommand
*  Description    :  it calls function,when the user selects a command from the Control menu
*  Author Name    :  Nitin Shelar
*  Date           :  21/11/2018
****************************************************************************************************/
void CWardWizPreInstallScanDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
void CWardWizPreInstallScanDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
/***************************************************************************************************
*  Function Name  : OnQueryDragIcon
*  Description    : The framework calls this member function by a minimized
					(iconic) window that does not have an icon defined for its class
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
HCURSOR CWardWizPreInstallScanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************
Function Name  : OnBnClickedCancel
Description    : Cancel button click handler
Author Name    : Nitin Shelar
Date           : 21/11/2018
****************************************************************************/
void CWardWizPreInstallScanDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
int CWardWizPreInstallScanDlg::GetTaskBarHeight()
{
	try
	{
		RECT rect;
		HWND taskBar;
		taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
		if (taskBar && ::GetWindowRect(taskBar, &rect))
		{
			return rect.bottom - rect.top;
		}
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizPreInstallScanDlg::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
int CWardWizPreInstallScanDlg::GetTaskBarWidth()
{
	try
	{
		RECT rect;
		HWND taskBar;
		taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
		if (taskBar && ::GetWindowRect(taskBar, &rect))
		{
			return rect.right - rect.left;
		}
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizPreInstallScanDlg::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : To handle the Sciter UI related requests
*  Author Name    : Nitin Shelar
*  Date           : 29th Oct 2018
****************************************************************************************************/
LRESULT CWardWizPreInstallScanDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	__try
	{
		switch (message)
		{
			case WM_CLOSE:
				return 0;
		}
		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);

		if (bHandled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Get the product id
*  Author Name    : Nitin Shelar
*  Date           : 29th Oct 2018
****************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}

	return iProdValue;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Nitin Shelar
*  Date           : 29th Oct 2018
****************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';
		
		CString csIniFilePath = szModulePath;
		csIniFilePath += L"\\VBSETTINGS\\PRODUCTSETTINGS.INI";
		iLangValue = GetPrivateProfileInt(L"VBSETTINGS", L"LanguageID", 0, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
Function Name  : On_Close
Description    : On_Close for close Dialog box.
Author Name    : Nitin Shelar
Date           : 29th Oct 2018
/***************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_Close()
{
	try
	{
		m_bManualStop = true;

		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS);
		}

		Sleep(1000);

		if (m_hThread_ScanCount != NULL)
		{
			SuspendThread(m_hThread_ScanCount);
			TerminateThread(m_hThread_ScanCount, 0);
			m_hThread_ScanCount = NULL;
		}
		
		if (m_hWardWizAVThread != NULL)
		{
			SuspendThread(m_hWardWizAVThread);
			TerminateThread(m_hWardWizAVThread, 0);
			m_hWardWizAVThread = NULL;
		}

		Sleep(5);

		CWardWizOSversion	objOSVersion;
		if (objOSVersion.DetectClientOSVersion() == WINOS_XP || objOSVersion.DetectClientOSVersion() == WINOS_VISTA)
		{
			exit(0);
		}
		else
		{
			ForceTerminate();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_Close", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Shelar
Date           : 29th Oct 2018
/***************************************************************************************************/
json::value CWardWizPreInstallScanDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_StartQuickScan
*  Description    : Accepts the request from UI and starts the Quick scan
*  Author Name    : Nitin Shelar
*  Date           : 15th Nov 2018
****************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_StartQuickScan(SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB)
{
	try
	{
		theApp.m_bIsScanning = true;
		theApp.m_bQuickScan = true;
		m_bIsManualStop = false;
		m_bStop = false;
		m_bIsManualStopScan = false;
		m_eCurrentSelectedScanType = QUICKSCAN;
		m_svAddVirusFoundEntryCB = svFunAddVirusFoundEntryCB;
		m_svSetScanFinishedStatusCB = svFunSetScanFinishedStatusCB;
		StartScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_StartQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_SysFileScan
*  Description    : Accepts the request from UI and starts the system scan
*  Author Name    : Nitin Shelar
*  Date           : 22th Nov 2018
****************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_SysFileScan(SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB)
{
	try
	{
		theApp.m_bIsScanning = true;
		m_bIsManualStop = false;
		m_bStop = false;
		m_bIsManualStopScan = false;
		m_eCurrentSelectedScanType = SYSFILESCAN;
		m_svAddVirusFoundEntryCB = svFunAddVirusFoundEntryCB;
		m_svSetScanFinishedStatusCB = svFunSetScanFinishedStatusCB;
		StartScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_StartQuickScan", 0, 0, true, SECONDLEVEL);
	}
return json::value();
}

/**********************************************************************************************************
*  Function Name  :	StartScanning
*  Description    :	To start Full scan ,custom scan and quick scan accoeding m_scantype variable.
*  Author Name    : Nitin Shelar
*  Date           : 15th Nov 2018
**********************************************************************************************************/
void CWardWizPreInstallScanDlg::StartScanning()
{
	try
	{
		m_bRedFlag = false;
		m_bIsPathExist = false;
		UINT uRetVal = 0;
		m_bManualStop = false;
		theApp.m_bQuickScan = false;
		m_iThreatsFoundCount = 0;
		m_ScanCount = false;
		m_FileScanned = 0;
		m_csPreviousPath = L"";
		m_eScanType = m_eCurrentSelectedScanType;
		m_bScnAborted = false;

		DWORD dwSigCount = 0x00;
		if (LoadSignatureDatabase(dwSigCount) != 0x00)
		{
			AddLogEntry(L"### Failed to Load SignatureDatabase", 0, 0, true, SECONDLEVEL);
		}
		m_bScanStartedStatusOnGUI = true;

		theApp.m_bQuickScan = true;

		/*if (!GetScanningPaths(m_csaAllScanPaths))
		{
			return;
		}*/

		m_iTotalFileCount = 0;
		m_dwTotalFileCount = 0;
		m_iMemScanTotalFileCount = 0;
		m_dwVirusFoundCount = 0;
		m_dwVirusCleanedCount = 0;

		if (theApp.m_bQuickScan == true)
		{
			m_eScanType = QUICKSCAN;
			GetModuleCount();
		}

		m_hThread_ScanCount = ::CreateThread(NULL, 0, GetTotalScanFilesCount, (LPVOID) this, 0, NULL);
		Sleep(500);

		DWORD m_dwThreadId = 0;
		m_hWardWizAVThread = ::CreateThread(NULL, 0, EnumerateThread, (LPVOID) this, 0, &m_dwThreadId);
		Sleep(500);
		CString csScanStarted = L"";

		csScanStarted = L">>>Scanning started...";

		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS, 1000, NULL);  // call OnTimer function
		}
		sciter::dom::element ela = self;
		AddLogEntry(csScanStarted, 0, 0, true, SECONDLEVEL);
		m_tsScanStartTime = CTime::GetCurrentTime();
		m_tsScanPauseResumeElapsedTime -= m_tsScanPauseResumeElapsedTime;
		sciter::dom::element(self).start_timer(500);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::StartScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetModuleCount
*  Description    :	Give total modules of proesses in the case of quick scan
*  Author Name    : Nitin Shelar
*  Date           : 15th Nov 2018
**********************************************************************************************************/
void CWardWizPreInstallScanDlg::GetModuleCount()
{
	try
	{
		DWORD	dwPID[0x100] = { 0 };
		DWORD	dwBytesRet = 0x00, dwProcIndex = 0x00;
		m_csaModuleList.RemoveAll();
		EnumProcesses(dwPID, 0x400, &dwBytesRet);
		dwBytesRet = dwBytesRet / sizeof(DWORD);
		m_iTotalFileCount = 0;
		for (dwProcIndex = 0; dwProcIndex < dwBytesRet; dwProcIndex++)
		{
			if (m_bManualStop)
			{
				break;
			}

			HMODULE		hMods[1024] = { 0 };
			HANDLE		hProcess = NULL;
			DWORD		dwModules = 0x00;
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, dwPID[dwProcIndex]);

			if (!hProcess)
				continue;

			if (EnumProcessModulesWWizEx != NULL)
			{
				if (!EnumProcessModulesWWizEx(hProcess, hMods, sizeof(hMods), &dwModules, LIST_MODULES_ALL))
				{
					DWORD error = GetLastError();
					CloseHandle(hProcess);
					continue;
				}
			}
			else
			{
				if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules))
				{
					DWORD error = GetLastError();
					CloseHandle(hProcess);
					continue;
				}
			}

			for (DWORD iModIndex = 0; iModIndex < (dwModules / sizeof(HMODULE)); iModIndex++)
			{
				if (m_bManualStop)
				{
					break;
				}

				TCHAR szModulePath[MAX_PATH * 2] = { 0 };
				GetModuleFileNameEx(hProcess, hMods[iModIndex], szModulePath, MAX_PATH * 2);
				if (!IsDuplicateModule(szModulePath, sizeof(szModulePath) / sizeof(TCHAR)))
				{
					m_iTotalFileCount++;
					_tcsupr_s(szModulePath, sizeof(szModulePath) / sizeof(TCHAR));
					m_csaModuleList.AddHead(szModulePath);
					m_csaAllScanPaths.Add(szModulePath);
				}
			}
			CloseHandle(hProcess);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::GetModuleCount", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : IsDuplicateModule
Description    : Function to find duplicates modules to avoid multiple scanning.
Author Name    : Nitin Shelar
Date           : 15th Nov 2018
****************************************************************************/
bool CWardWizPreInstallScanDlg::IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize)
{
	bool bReturn = false;
	try
	{
		_tcsupr_s(szModulePath, dwSize);
		if (m_csaModuleList.Find(szModulePath, 0))
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CClamScanner::IsDuplicateModule", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : EnumerateThread
Description    : Thread to enumerarte process in case of quick scan and
				 files and folders in case of custom and full scan.
Author Name    : Nitin Shelar
Date           : 15th Nov 2018
****************************************************************************/
DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizPreInstallScanDlg *pThis = (CWardWizPreInstallScanDlg*)lpvThreadParam;
		if (!pThis)
			return 0;
		int	iIndex = 0x00;
		if (pThis->m_eScanType == QUICKSCAN)
		{
			pThis->EnumerateProcesses();
		}

		
		iIndex = (int)pThis->m_csaAllScanPaths.GetCount();
		if (!iIndex)
			return 2;
		for (int i = 0; i < iIndex; i++)
		{
			CString csPath = pThis->m_csaAllScanPaths.GetAt(i);
			if (!PathFileExists(csPath))
			{
				continue;
			}
			pThis->m_bIsPathExist = true;
			pThis->EnumFolderForScanning(csPath);
		}


		if (!pThis->m_bManualStop)
		{
			ITIN_MEMMAP_DATA iTinMemMap = { 0 };
			iTinMemMap.iMessageInfo = DISABLE_CONTROLS;
			iTinMemMap.dwSecondValue = pThis->m_iThreatsFoundCount;
		}
		pThis->ScanFinished();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::EnumerateThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	EnumFolderForScanning
*  Description    :	enumerate files of system and sent it to scan.
*  Author Name    :	NITIN SHELAR
*  Date           : 15 Nov 2018
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizPreInstallScanDlg::EnumFolderForScanning(LPCTSTR pstr)
{
	try
	{
		if (!pstr)
			return;

		CFileFind finder;
		DWORD	dwAttributes = 0;
		CString strWildcard(pstr);
		CString csFilePath(pstr);

		//Check here is file/folder from removable device
		if (GetDriveType((LPCWSTR)strWildcard.Left(2)) == DRIVE_REMOVABLE)
		{
			//Is file/folder is hidden?
			if (FILE_ATTRIBUTE_HIDDEN == (GetFileAttributes(strWildcard) & FILE_ATTRIBUTE_HIDDEN))
			{
				//Check is file/folder on root path?
				if (CheckFileOrFolderOnRootPath(strWildcard))
				{
					SetFileAttributes(strWildcard, FILE_ATTRIBUTE_NORMAL);
				}
			}
		}

		BOOL bRet = PathIsDirectory(strWildcard);
		if (bRet == FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strWildcard[strWildcard.GetLength() - 1] != '\\')
				strWildcard += _T("\\*.*");
			else
				strWildcard += _T("*.*");
		}
		else
		{
			if (!PathFileExists(pstr))
			{
				return;
			}

			if (m_bManualStop)
			{
				return;
			}

			m_csCurrentFilePath = pstr;
			m_FileScanned++;
			ScanForSingleFile(pstr);
			return;
		}

		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;
			if (finder.IsDirectory())
			{
				CString str = finder.GetFilePath();
				EnumFolderForScanning(str);
			}
			else
			{
				CString csFilePath = finder.GetFilePath();
				if (csFilePath.Trim().GetLength() > 0)
				{
					//Check here is file/folder from removable device
					if (GetDriveType((LPCWSTR)csFilePath.Left(2)) == DRIVE_REMOVABLE)
					{
						//Is file/folder is hidden?
						if (FILE_ATTRIBUTE_HIDDEN == (GetFileAttributes(csFilePath) & FILE_ATTRIBUTE_HIDDEN))
						{
							//Check is file/folder on root path?
							if (CheckFileOrFolderOnRootPath(csFilePath))
							{
								SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
							}
						}
					}

					if (PathFileExists(csFilePath))
					{
						if (m_bManualStop)
						{
							break;
						}
						m_FileScanned++;
						ScanForSingleFile(csFilePath);
					}
				}
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::EnumFolderForScanning", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : CheckFileOrFolderOnRootPath
Description    : Function which check IS file/Folder present on root path
Author Name    : NITIN SHELAR
Date           : 15 Nov 2018
/***************************************************************************************************/
bool CWardWizPreInstallScanDlg::CheckFileOrFolderOnRootPath(CString csFilePath)
{
	try
	{
		int iPos = csFilePath.ReverseFind(L'\\');
		if (iPos == csFilePath.GetLength() - 1)
		{
			iPos = csFilePath.Left(csFilePath.GetLength() - 1).ReverseFind(L'\\');
		}

		if (iPos == 0x02)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CheckFileOrFolderOnRootPath, Path: %s", csFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : GetTotalScanFilesCount
Description    : Get total files count in case of fullscan and custom scan
Author Name    : Nitin Shelar
Date           : 15th Nov 2018
****************************************************************************/
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam)
{
	try
	{
		CWardWizPreInstallScanDlg *pThis = (CWardWizPreInstallScanDlg*)lpParam;
		if (!pThis)
			return 1;
		int	iIndex = 0x00;
		iIndex = (int)pThis->m_csaAllScanPaths.GetCount();
		if (!iIndex)
			return 2;

		for (int i = 0; i < iIndex; i++)
		{
			pThis->EnumFolder(pThis->m_csaAllScanPaths.GetAt(i));
		}

		if (pThis->m_iTotalFileCount)
		{
			pThis->m_ScanCount = true;
		}
		pThis->m_dwTotalFileCount = pThis->m_iTotalFileCount;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::GetTotalScanFilesCount", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	EnumFolder
*  Description    :	Enumerate each folders of system and calculate total files count.
*  Author Name    : Nitin Shelar
*  Date           : 15th Nov 2018
**********************************************************************************************************/
void CWardWizPreInstallScanDlg::EnumFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");
		BOOL bWorking = finder.FindFile(strWildcard);
		if (bWorking)
		{
			while (bWorking)
			{
				bWorking = finder.FindNextFile();
				if (finder.IsDots())
					continue;

				// if it's a directory, recursively search it 
				if (finder.IsDirectory())
				{
					CString str = finder.GetFilePath();
					EnumFolder(str);
				}
				else
				{
					m_iTotalFileCount++;
				}
			}
		}
		else
		{
			m_iTotalFileCount++;
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetScanningPaths
*  Description    :	Get scan path according to scanning types.
*  Author Name    : Nitin Shelar
*  Date           : 15th Nov 2018
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::GetScanningPaths(CStringArray &csaReturn)
{
	try
	{
		CString cScanPath;
		TCHAR	szProgFilesDir86[MAX_PATH] = { 0 };
		TCHAR	szProgFilesDir[MAX_PATH] = { 0 };
		TCHAR	szWindowsDir[MAX_PATH] = { 0 };
		WardWizSystemInfo	objSysInfo;

		switch (m_eScanType)
		{
		case QUICKSCAN:
			theApp.m_bQuickScan = true;
			csaReturn.RemoveAll();
			if (objSysInfo.GetOSType())
			{
				GetEnvironmentVariable(TEXT("PROGRAMFILES(X86)"), szProgFilesDir86, 255);
				csaReturn.Add(szProgFilesDir86);
			}
			else
			{
				GetEnvironmentVariable(TEXT("ProgramFiles"), szProgFilesDir, 255);
				csaReturn.Add(szProgFilesDir);
			}

			break;
		case SYSFILESCAN:
				csaReturn.RemoveAll();
				if (objSysInfo.GetOSType())
				{
					SHGetSpecialFolderPath(0, szProgFilesDir, CSIDL_PROGRAM_FILES, FALSE);
					csaReturn.Add(szProgFilesDir);
				}
				else
				{
					SHGetSpecialFolderPath(0, szProgFilesDir86, CSIDL_PROGRAM_FILESX86, FALSE);
					csaReturn.Add(szProgFilesDir86);
				}
				SHGetSpecialFolderPath(0, szWindowsDir, CSIDL_WINDOWS, FALSE);
				csaReturn.Add(szWindowsDir);
			break;
		}
		if (csaReturn.GetCount() > 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::GetScanningPaths", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************            
*  Function Name  :	EnumerateProcesses
*  Description    :	Enumaerate processes in case of quick scan.
					Changes (Ram) : Time complexity decresed as we enumerating processes and modules
					to calculate file count, There is no need to enumerate it again.
					kept in CStringList while file count calculation, same list is used again.
*  Author Name    : Nitin Shelar
*  Date           : 15/11/2018
**********************************************************************************************************/
void CWardWizPreInstallScanDlg::EnumerateProcesses()
{
	try
	{
		CString csProcessPath(L"");
		CString csToken(L"");
		CString csTokenSytemRoot(L"");
		TCHAR szSystemDirectory[MAX_PATH] = { 0 };
		bool bSystemDirectory = false;
		bool bReplaceWindowPath = false;

		POSITION pos = m_csaModuleList.GetHeadPosition();
		while (pos != NULL)
		{
			if (m_bManualStop)
			{
				break;
			}
			
			csProcessPath = m_csaModuleList.GetNext(pos);
			int iPos = 0;
			int SlashPos = 0;
			SlashPos = csProcessPath.Find(_T("\\"), iPos);
			if (SlashPos == 0)
			{
				csToken = csProcessPath.Right(csProcessPath.GetLength() - (SlashPos + 1));
				bSystemDirectory = true;
			}
			GetWindowsDirectory(szSystemDirectory, MAX_PATH);
			if (bSystemDirectory == true)
			{
				SlashPos = 0;
				iPos = 0;
				SlashPos = csToken.Find(_T("\\"), iPos);
				csTokenSytemRoot = csToken;
				csTokenSytemRoot.Truncate(SlashPos);
				if (csTokenSytemRoot == L"SystemRoot")
				{
					bReplaceWindowPath = true;
				}
				else if (csTokenSytemRoot == L"??")
				{
					csToken.Replace(L"??\\", L"");
					csProcessPath = csToken;
				}
				bSystemDirectory = false;
			}
			if (bReplaceWindowPath == true)
			{
				csToken.Replace(csTokenSytemRoot, szSystemDirectory);
				csProcessPath = csToken;
				bReplaceWindowPath = false;
			}
			if (PathFileExists(csProcessPath))
			{
				if (m_bManualStop)
				{
					break;
				}
				m_FileScanned++;
				ScanForSingleFile(csProcessPath);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::EnumerateProcesses", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ScanForSingleFile
*  Description    :	Scan each single file .
*  Author Name    : Nitin Shelar
*  Date           : 15/11/2018
**********************************************************************************************************/
void CWardWizPreInstallScanDlg::ScanForSingleFile(CString csFilePath)
{
	bool bFound = false;
	bool bRescan = true;
	DWORD dwISpyID = 0;
	DWORD dwReturn = 0x00;
	int arrIndex = 0;
	TCHAR szVirusName[MAX_PATH] = { 0 };
	if (csFilePath.GetLength() == 0)
		return;
	try
	{
		bool bSetStatus = false;
		bool bVirusFound = false;

		CString csVirusName(L"");
		CString csVirusPath(L"");
		
		DWORD dwAction = 0x00;
		CString csCurrentFile(L"");
		CString csStatus(L"");
		m_csCurrentFilePath = csFilePath;
		if (csFilePath.Trim().GetLength() > 0)
		{
			if (m_csPreviousPath != csFilePath)
			{
				if (PathFileExists(csFilePath))
				{
					m_bIsPathExist = true;
					DWORD dwSignatureFailedToLoad = 0;
					DWORD dwActionTaken = 0x00;
					DWORD dwRet = 0;

					dwRet = m_objISpyScanner.ScanFile(csFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, bRescan);
					if (dwRet != 0x00)
					{
						if (dwISpyID >= 0)
						{
							csVirusName = szVirusName;
							m_wISpywareID = dwISpyID;
							csStatus.Format(L"%s",GetString(L"IDS_CONSTANT_THREAT_DETECTED"));
							bVirusFound = true;
						}
					}
				}
			}
		}

		//virus found 
		if (bVirusFound)
		{
			bSetStatus = true;
			m_csISpyID.Format(L"%d", dwISpyID);
			CallUISetVirusFoundEntryfunction(csVirusName, csFilePath, csStatus, m_csISpyID);
			m_dwVirusFoundCount++;
		}
		else
		{
			bSetStatus = true;
			csStatus = csFilePath;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in void CWardwizPreInstallScanDlg::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ScanFinished
*  Description    :	display status Scan Finished
*  Author Name    : Nitin Shelar
*  Date           : 15/11/2018
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::ScanFinished()
{
	CString csCompleteScanning;
	CString csFileScanCount;
	CString csMsgNoFileExist(L"");
	CString csCurrentFileCount;
	CString cstypeofscan = L"";

	OnTimerScan();
	csCurrentFileCount.Format(L"%d", m_FileScanned);
	CallUISetFileCountfunction(L"", csCurrentFileCount);

	OutputDebugString(L">>> m_hThreadStatusEntry stopped");
	Sleep(500);

	if (!m_bIsPathExist)
	{
		csMsgNoFileExist.Format(L"%s", GetString(L"IDS_CUSTOM_SCAN_SELECT_INVALID_SELECTION"));
		CallNotificationMessage(1, (SCITER_STRING)csMsgNoFileExist);
	}
	CallUISetScanFinishedStatus(GetString(L"IDS_STATUS_INFECTEDFILES"));
	m_tsScanEndTime = CTime::GetCurrentTime();
	CString csTime = m_tsScanEndTime.Format(_T("%H:%M:%S"));
	AddLogEntry(_T(">>> End Scan Time: %s"), csTime, 0, true, FIRSTLEVEL);
	CString csElapsedTime;
	csElapsedTime.Format(L"%s%s", GetString(L"IDS_STATIC_ELAPSEDTIME"), csTime);
	AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	
	if (!m_bIsManualStop)
	{
		csCompleteScanning.Format(L">>> %s %s.", cstypeofscan, GetString(L"IDS_STATUS_INFECTEDFILES"));
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		csFileScanCount.Format(L"%s%d", GetString(L"IDS_STATIC_FILESCANNED"), m_FileScanned);
		csCompleteScanning.Format(L">>> %s = %d, %s = %d", GetString(L"IDS_STATIC_USB_FILESCANNED"), m_FileScanned, GetString(L"IDS_STATIC_USB_THREAD_FOUND"), m_dwVirusFoundCount);
	}
	else
	{
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		AddLogEntry(L"--------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
		csCompleteScanning.Format(L">>> %s %s.", cstypeofscan, GetString(L"IDS_STATUS_SCAN_ABORTED"));
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		csFileScanCount.Format(L"%s%d", GetString(L"IDS_STATIC_FILESCANNED"), m_FileScanned);
		csCompleteScanning.Format(L">>> %s = %d, %s = %d", GetString(L"IDS_STATIC_USB_FILESCANNED"), m_FileScanned, GetString(L"IDS_STATIC_USB_THREAD_FOUND"), m_dwVirusFoundCount);
	}
	AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"--------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	csCompleteScanning.Format(L"%s%d", GetString(L"IDS_STATIC_TREATFOUND"), m_dwVirusFoundCount);

	if (m_bIsManualStopScan == false)
	{
		HWND hWindow = ::FindWindow(NULL, L"VibraniumPreInsScan");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
		}
	}

	DWORD dwTotalRebootCount = 0x00;
	if (m_iThreatsFoundCount == 0)
	{
		if (dwTotalRebootCount)
		{
			CString csMsgToRebootSystem(L"");
			csMsgToRebootSystem.Format(L"%s %d %s\n\n%s", GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART1"), dwTotalRebootCount, GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART2"), GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));
			CallNotificationMessage(2, (SCITER_STRING)csMsgToRebootSystem);
		}
	}
	theApp.m_bQuickScan = false;
	theApp.m_bIsScanning = false;
	return true;
}

/***************************************************************************************************
Function Name  : OnTimerScan
Description    : On timer event for file count.
Author Name    : Nitin Shelar
Date           : 15/11/2018
/***************************************************************************************************/
void CWardWizPreInstallScanDlg::OnTimerScan()
{
	try
	{
		CallUISetStatusfunction(m_csCurrentFilePath.GetBuffer());
		CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
		CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
		CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::OnTimerScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : CallNotificationMessage
Description    : Calls Light box on UI
Author Name    : Nitin Shelar
Date           : 15/11/2018
/***************************************************************************************************/
void CWardWizPreInstallScanDlg::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
{
	try
	{
		m_svFunNotificationMessageCB.call(iMsgType, (SCITER_STRING)strMessageString);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		Sleep(300);
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : NITIN SHELAR
*  Date			  : 10th Nov 2016
****************************************************************************************************/
void CWardWizPreInstallScanDlg::CallUISetStatusfunction(LPTSTR lpszPath)
{
	__try
	{
		CallUISetStatusfunctionSEH(lpszPath);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::CallUISetStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallUISetStatusfunctionSEH
*  Description    : Calls call_function to invoke ant UI function
Note: No need to add exception handling because
*  Author Name    : NITIN SHELAR
*  Date			  : 31 Oct 2018
****************************************************************************************************/
void CWardWizPreInstallScanDlg::CallUISetStatusfunctionSEH(LPTSTR lpszPath)
{
	if (!lpszPath)
	{
		return;
	}
	CString csPath(lpszPath);
	TCHAR					m_szActiveScanFilePath[1024];
	memset(m_szActiveScanFilePath, 0, sizeof(m_szActiveScanFilePath));
	int iCount = csPath.ReverseFind('\\');
	CString csFileName = csPath.Right(csPath.GetLength() - (iCount + 1));
	CString csFolderPath;
	csFolderPath = csPath.Left(iCount);

	GetShortPathName(csFolderPath, m_szActiveScanFilePath, 60);
	CString csTempFileName = csFileName;
	iCount = csTempFileName.ReverseFind('.');
	CString csFileExt = csTempFileName.Right(csTempFileName.GetLength() - (iCount));
	csTempFileName = csTempFileName.Left(iCount);
	if (csTempFileName.GetLength() > 10)
	{
		csTempFileName = csTempFileName.Left(10);
		csFileName.Format(L"%s~%s", csTempFileName, csFileExt);
	}

	if (_tcslen(m_szActiveScanFilePath) == 0 || csFileName.GetLength() == 0)
		return;

	CString csFinalFilePath;
	csFinalFilePath.Format(L"%s\\%s", m_szActiveScanFilePath, csFileName);

	sciter::dom::element ela = self;
	BEHAVIOR_EVENT_PARAMS params;
	params.cmd = SETFILEPATH_EVENT_CODE;
	params.he = params.heTarget = ela;
	params.data = SCITER_STRING(csFinalFilePath.Trim());
	ela.fire_event(params, true);
}

/***************************************************************************************************
*  Function Name  : callUISetScanFinishedStatus
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Shelar
*  Date			  : 02 Nov 2018
****************************************************************************************************/
void CWardWizPreInstallScanDlg::CallUISetScanFinishedStatus(CString csData)
{
	try
	{
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS);
		}

		m_svSetScanFinishedStatusCB.call(SCITER_STRING(csData));
		sciter::dom::element(self).stop_timer();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::CallUISetScanFinishedStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallUISetFileCountfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Shelar
*  Date           : 31 Oct 2018
****************************************************************************************************/
void CWardWizPreInstallScanDlg::CallUISetFileCountfunction(CString csTotalFileCount, CString csCurrentFileCount)
{
	try
	{
		sciter::value map;
		map.set_item("one", sciter::string(csCurrentFileCount));
		map.set_item("two", sciter::string(csTotalFileCount));
		//Send here event
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETFILECOUNT_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::callUIfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : on_timer
Description    : On timer
Author Name    : Nitin Shelar
Date           : 31 Oct 2018
/***************************************************************************************************/
bool CWardWizPreInstallScanDlg::on_timer(HELEMENT he, UINT_PTR extTimerId)
{
	try
	{
		/*CallUISetStatusfunction(m_csCurrentFilePath);
		CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
		CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
		CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
		m_csPreviousPath = m_csCurrentFilePath;
		AddLogEntry(L"### Called CWardwizScan::on_timer()", 0, 0, true, SECONDLEVEL);*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::on_timer(HELEMENT he, UINT_PTR extTimerId)", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : on_timer
Description    : On_timer cals on_timerSEH function 
Author Name    : Nitin Shelar
Date           : 31 Oct 2018
/***************************************************************************************************/
bool CWardWizPreInstallScanDlg::on_timer(HELEMENT he)
{
	__try
	{
		return  on_timerSEH(he);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::on_timer", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : on_timerSEH
Description    : on_timerSEH for count total file and display file path on dialog box.
Author Name    : Nitin Shelar
Date           : 31 Oct 2018
/***************************************************************************************************/
bool CWardWizPreInstallScanDlg::on_timerSEH(HELEMENT he)
{
	CallUISetStatusfunction(m_csCurrentFilePath.GetBuffer());
	CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
	CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
	CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
	return true;
}

/***************************************************************************
Function Name  : LoadPSAPILibrary
Description    : Load PSAPI.DLL.
For Issue	   : In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
Author Name    : Nitin Shelar
Date           : 31 Oct 2018
****************************************************************************/
void CWardWizPreInstallScanDlg::LoadPSAPILibrary()
{
	__try
	{
		TCHAR	szSystem32[256] = { 0 };
		TCHAR	szTemp[256] = { 0 };
		GetSystemDirectory(szSystem32, 255);

		ZeroMemory(szTemp, sizeof(szTemp));
		wsprintf(szTemp, L"%s\\PSAPI.DLL", szSystem32);
		if (!m_hPsApiDLL)
		{
			m_hPsApiDLL = LoadLibrary(szTemp);
		}

		if (!EnumProcessModulesWWizEx)
		{
			EnumProcessModulesWWizEx = (ENUMPROCESSMODULESEX)GetProcAddress(m_hPsApiDLL, "EnumProcessModulesEx");
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScan::LoadPSAPILibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_PasueQuickScan
*  Description    : Accepts the request from UI and Pause Quick scan
*  Author Name    : Nitin Shelar
*  Date           : 23 Oct 2018
****************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_PauseQuickScan(SCITER_VALUE svFunPauseResumeFunCB)
{
	try
	{
		m_svSetPauseStatusCB = svFunPauseResumeFunCB;
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS);
		}
		if (theApp.m_bQuickScan)
		{
			if (m_hThread_ScanCount != NULL)
			{
				::SuspendThread(m_hThread_ScanCount);
				AddLogEntry(L">>> m_hThread_ScanCount Scanning paused inside CWardwizPreInstallScanDlg::On_PauseQuickScan", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### m_hThread_ScanCount failed to pause scanning as SuspendThread request failed.", 0, 0, true, SECONDLEVEL);
			}
			if (m_hWardWizAVThread != NULL)
			{
				::SuspendThread(m_hWardWizAVThread);
				m_svSetPauseStatusCB.call(SCITER_STRING(GetString(L"IDS_STATUS_SCAN_PAUSE")));
				AddLogEntry(L">>> m_hVibraniumAVThread Scanning paused inside CWardwizPreInstallScanDlg::On_PauseQuickScan", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### m_hVibraniumAVThread failed to pause scan as SuspendThread request failed.", 0, 0, true, SECONDLEVEL);
			}
			theApp.m_bQuickScan = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_PasueQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_ResumeQuickScan
*  Description    : Accepts the request from UI and Resumes quick scan
*  Author Name    : Nitin Shelar
*  Date           : 23 Oct 2018
****************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_ResumeQuickScan(SCITER_VALUE svFunPauseResumeFunCB)
{
	try
	{
		m_svSetPauseStatusCB = svFunPauseResumeFunCB;
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS, 1000, NULL);
		}
		if (!theApp.m_bQuickScan)
		{
			if (m_hThread_ScanCount != NULL)
			{
				::ResumeThread(m_hThread_ScanCount);
				AddLogEntry(L">>> m_hThread_ScanCount thread resume for scanning inside CWardwizPreInstallScanDlg::On_ResumeQuickScan", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### m_hThread_ScanCount failed to resume thread for scanning inside CWardwizPreInstallScanDlg::On_ResumeQuickScan", 0, 0, true, SECONDLEVEL);
			}
			if (m_hWardWizAVThread != NULL)
			{
				::ResumeThread(m_hWardWizAVThread);
				m_svSetPauseStatusCB.call(SCITER_STRING(GetString(L"IDS_STATUS_SCAN_RESUME")));
				AddLogEntry(L">>> m_hVibraniumAVThread thread resume for scanning inside CWardwizPreInstallScanDlg::On_ResumeQuickScan.", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### m_hVibraniumAVThread thread resume for scanning inside CWardwizPreInstallScanDlg::On_ResumeQuickScan", 0, 0, true, SECONDLEVEL);
				return false;
			}
			sciter::dom::element(self).start_timer(500);
			theApp.m_bQuickScan = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_ResumeQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/**********************************************************************************************************
*  Function Name  :	LoadSignatureDatabase
*  Description    :	Function to load Signature database.
*  Author Name    : Nitin Shelar
*  Date           : 26/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizPreInstallScanDlg::LoadSignatureDatabase(DWORD &dwSigCount)
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.LoadSignatureDatabase(dwSigCount);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	UnLoadSignatureDatabase
*  Description    :	Function to Unload Signature database.
*  Author Name    : Nitin Shelar
*  Date           : 26/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizPreInstallScanDlg::UnLoadSignatureDatabase()
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.UnLoadSignatureDatabase();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::UnLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : CallUISetVirusFoundEntryfunction
*  Description    : used to insert threat found entries
*  Author Name    : Nitin Shelar
*  Date			  : 27/11/2018
****************************************************************************************************/
void CWardWizPreInstallScanDlg::CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID)
{
	try
	{
		m_svAddVirusFoundEntryCB.call(SCITER_STRING(csVirusName), SCITER_STRING(csFilePath), SCITER_STRING(csActionTaken), SCITER_STRING(SpyID));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::CallUISetVirusFoundEntryfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : onModalLoop
Description    : for reseting the Lightbox event msgbox
Author Name    : Nitin Shelar
SR_NO		   :
Date           : 27/11/2018
/***************************************************************************************************/
json::value CWardWizPreInstallScanDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;

			theApp.m_objCompleteEvent.SetEvent();
			Sleep(200);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	HandleVirusEntry
*  Description    :	When any entry comes for cleaning, wardwiz scanner take a backup store into quarantine
					folder and keep a record into DB file
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizPreInstallScanDlg::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThirdParam, DWORD dwISpyID, CString &csBackupFilePath, DWORD &dwAction)
{
	DWORD dwRet = 0;
	CString csStatus;
	m_csQuarentineEntries.Lock();
	try
	{
		TCHAR szAction[MAX_PATH] = { 0 };
		if ((!szThreatPath) || (!szThreatName) || (!szThirdParam))
		{
			AddLogEntry(L"### CWardwizPreInstallScanDlg::HandleVirusEntry file name not available", 0, 0, true, SECONDLEVEL);
			dwRet = SANITYCHECKFAILED;
			m_csQuarentineEntries.Unlock();
			return dwRet;
		}

		if (!PathFileExists(szThreatPath))
		{
			AddLogEntry(L"### CWardwizPreInstallScanDlg::HandleVirusEntry No file available %s", szThreatPath, 0, true, SECONDLEVEL);

			dwRet = FILENOTEXISTS;
			m_csQuarentineEntries.Unlock();
			return dwRet;
		}
		//For files having read-only attribute
		::SetFileAttributes(szThreatPath, FILE_ATTRIBUTE_NORMAL);

		CString csQuaratineFolderPath = GetProgramFilePath();
		if (!PathFileExists(csQuaratineFolderPath))
		{
			if (!CreateDirectory(csQuaratineFolderPath, NULL))
			{
				AddLogEntry(L"### CWardwizPreInstallScanDlg::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
				dwRet = CREATEDIRECTORYFAILED;
			}
		}

		bool bISbackupSuccess = false;

		TCHAR szBackupFilePath[MAX_PATH * 2] = { 0 };

		//DWORD value flag to show entry in Recover window (or) not.
		DWORD dwShowEntryStatus = 0;
		m_dwAutoQuarOption = QURENTINE;

		if (dwISpyID > 0)
		{
			//Terminate the process if its running
			CEnumProcess objEnumProcess;
			objEnumProcess.IsProcessRunning(szThreatPath, true);

			DWORD dwRepairID = 0;
			DWORD dwFailedToLoadSignature = 0;
			m_bRescan = true;
			TCHAR szVirusName[MAX_PATH] = { 0 };
			_tcscpy(szVirusName, szThreatName);

			if (CheckForDiscSpaceAvail(csQuaratineFolderPath, szThreatPath) == 0x01)
			{
				dwRet = LOWDISKSPACE;
				m_csQuarentineEntries.Unlock();
				return dwRet;
			}

			if (!BackUpBeforeQuarantineOrRepair(szThreatPath, szBackupFilePath))
			{
				AddLogEntry(L"#### Failed to take backup of %s", szThreatPath, 0, true, SECONDLEVEL);
				bISbackupSuccess = false;
				dwRet = 0x07;
			}

			bISbackupSuccess = true;

			if (!m_objISpyScanner.RepairFile(szThreatPath, dwISpyID))
			{
				dwShowEntryStatus = 1;
				dwAction = FILEREBOOTREPAIR;
				dwRet = 0x04;
			}
			else
			{
				dwAction = FILEREPAIRED;
				_tcscpy(szAction, GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
			}
		}
		else if (dwISpyID == 0)
		{
			//While taking a back up of any file. we have to check disk where we take a back up 
			//is having space or not.
			if (CheckForDiscSpaceAvail(csQuaratineFolderPath, szThreatPath) == 0x01)
			{
				dwRet = LOWDISKSPACE;
				m_csQuarentineEntries.Unlock();
				return dwRet;
			}

			if (!BackUpBeforeQuarantineOrRepair(szThreatPath, szBackupFilePath))
			{
				AddLogEntry(L"#### Failed to take backup of %s", szThreatPath, 0, true, SECONDLEVEL);
				bISbackupSuccess = false;
				dwRet = BACKUPFILEFAILED;
			}

			bISbackupSuccess = true;
			//quarantine file
			if (!QuarantineEntry(szThreatPath, szThreatName, szBackupFilePath))
			{
				AddLogEntry(L"### Failed to Quarantine file: %s", szThreatPath, 0, true, SECONDLEVEL);
				dwShowEntryStatus = 1;
				dwRet = DELETEFILEFAILED;
				dwAction = FILEREBOOTQUARENTINE;

				_tcscpy(szAction, GetString(L"IDS_AUTO_RUN_SCAN_RESTART_REQUIRED"));
				CallUISetVirusFoundEntryfunction(szThreatName, szThreatPath, szAction, m_csISpyID);
			}
			else
			{
				dwAction = FILEQURENTINED;
				_tcscpy(szAction, GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
				CallUISetVirusFoundEntryfunction(szThreatName, szThreatPath, szAction, m_csISpyID);
			}
		}
		else
		{
			AddLogEntry(L"### Unhandled case to clean, VirusName: [%s], FilePath: [%s]", szThreatName, szThreatPath, true, SECONDLEVEL);
		}

		bool bDoRecover = true;

		if (bISbackupSuccess)
		{
			csBackupFilePath = szBackupFilePath;
			if (bDoRecover)
			{
				LoadRecoversDBFile();

				if (!InsertRecoverEntry(szThreatPath, szBackupFilePath, szThreatName, dwShowEntryStatus))
				{
					AddLogEntry(L"### Error in InsertRecoverEntry, Path: %s | BackupPath: %s", szThreatPath, szBackupFilePath, true, SECONDLEVEL);
					dwRet = INSERTINTORECOVERFAILED;
				}

				if (!SaveInRecoverDB())
				{
					AddLogEntry(L"### Error in SaveInRecoverDB, Path: %s | BackupPath: %s", szThreatPath, szBackupFilePath, true, SECONDLEVEL);
					dwRet = SAVERECOVERDBFAILED;
				}
			}
		}
		m_bRescan = false;
	}
	catch (...)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::HandleVirusEntry", 0, 0, true, SECONDLEVEL);
	}

	m_csQuarentineEntries.Unlock();
	return dwRet;
}

/***************************************************************************
Function Name  : CheckForDiscSpaceAvail
Description    : to check whether there is enough space to take a backup
				percentage.
Author Name    : Nitin Shelar
SR_NO		   :
Date           : 29/11/2018
****************************************************************************/
DWORD CWardWizPreInstallScanDlg::CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath)
{
	DWORD dwRet = 0x00;
	CString csDrivePath;
	try
	{
		csDrivePath = csQuaratineFolderPath;
		int iPos = csDrivePath.Find(L"\\");
		if (iPos != -1)
		{
			csDrivePath.Truncate(iPos);
		}
		HANDLE hFile = CreateFile(csThreatPath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### CWardwizScanner::Error in opening existing file %s for finding a size of path file", csThreatPath, 0, true, SECONDLEVEL);
		}

		DWORD dwFileSize = GetFileSize(hFile, NULL);

		CloseHandle(hFile);

		if (!IsDriveHaveRequiredSpace(csDrivePath, 1, dwFileSize))
		{
			AddLogEntry(L"### Low space to store the back up of file %s", csThreatPath, 0, true, SECONDLEVEL);
			dwRet = 0x01;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::CheckForDiscSpaceAvail for file %s", csThreatPath, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
CString CWardWizPreInstallScanDlg::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************
Function Name  : IsDriveHaveRequiredSpace
Description    : to check whether there is enough space to take a backup
percentage.
Author Name    : Nitin Shelar
Date           : 29/11/2018
****************************************************************************/
bool CWardWizPreInstallScanDlg::IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize)
{
	bool bReturn = false;
	bool isbSpaceAvailable = false;
	try
	{
		DWORD64 TotalNumberOfFreeBytes;

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
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::IsDriveHaveRequiredSpace", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	BackUpBeforeQuarantineOrRepair
*  Description    :	Taking a backup before taking any action on detected files.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath, LPTSTR lpszBackupFilePath)
{
	try
	{
		if (!lpszBackupFilePath)
			return false;
		DWORD dwStatus = 0;
		CString csEncryptFilePath, csQuarantineFolderpath = L"";
		TCHAR szQuarantineFileName[MAX_PATH] = { 0 };
		UINT RetUnique = 0;

		csQuarantineFolderpath = GetProgramFilePath();

		if (!PathFileExists(csOriginalThreatPath))
		{
			AddLogEntry(L"### CWardwizPreInstallScanDlg::BackUpBeforeQuarantineOrRepair Original file not available %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
			return false;
		}

		if (!PathFileExists(csQuarantineFolderpath))
		{
			if (!CreateDirectory(csQuarantineFolderpath, NULL))
			{
				AddLogEntry(L"### CWardwizPreInstallScanDlg::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		//Get here file hash
		TCHAR szFileHash[128] = { 0 };
		if (!GetFileHash(csOriginalThreatPath.GetBuffer(), szFileHash))
		{
			return false;
		}

		//check here if backup has is taken already.
		if (CheckIFAlreadyBackupTaken(szFileHash, lpszBackupFilePath))
		{
			return  true;
		}

		if (Encrypt_File(csOriginalThreatPath.GetBuffer(), csQuarantineFolderpath.GetBuffer(), lpszBackupFilePath, szFileHash, dwStatus) != 0x00)
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::BackUpBeforeQuarantineOrRepair, FilePath: %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name		: CheckIFAlreadyBackupTaken
*  Description			: Function to check whether backup already taken (or) not.
*  Function Arguments	: szFileHash
*  Author Name			: Nitin Shelar
*  Date					: 29/11/2018
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::CheckIFAlreadyBackupTaken(LPCTSTR szFileHash, LPTSTR szBackupPath)
{
	bool bReturn = false;
	__try
	{
		//Sanity check
		if (!szFileHash || !szBackupPath)
			return bReturn;
		if (CheckEntryPresent(szFileHash, szBackupPath))
		{
			bReturn = true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::CheckIFAlreadyBackupTaken, HASH: %s, BackupPath:%s ", szFileHash, szBackupPath, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: CheckEntryPresent
*  Description			: Function to check entry present in list
*  Function Arguments	: szFileHash
*  Author Name			: Nitin Shelar
*  Date					: 29/11/2018
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::CheckEntryPresent(LPCTSTR szFileHash, LPTSTR szBackupPath)
{
	bool bReturn = false;
	try
	{
		//Sanity check
		if (!szFileHash)
			return bReturn;

		const ContactList& contacts = m_objISpyDBManipulation.m_RecoverDBEntries.GetContacts();
		POSITION pos = contacts.GetHeadPosition();
		while (pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
			CString csSecondEntry = contact.GetSecondEntry();
			if (csSecondEntry.Find(szFileHash) > 0)
			{
				_tcscpy(szBackupPath, csSecondEntry);
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::CheckIFAlreadyBackupTaken, HASH: %s, BackupPath:%s ", szFileHash, szBackupPath, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	Encrypt_File
*  Description    :	Encrypt file and keep into quarantine folder as temp file.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizPreInstallScanDlg::Encrypt_File(TCHAR *szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus)
{

	DWORD	dwRet = 0x00;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00;
	TCHAR	szExt[16] = { 0 };
	DWORD	dwLen = 0x00;
	LPBYTE	lpFileData = NULL;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE;

	__try
	{
		//Sanity check
		if (!szFilePath || !szQurFolderPath || !lpszFileHash || !lpszTargetFilePath)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//Check is valid paths
		if (!PathFileExists(szFilePath) || !PathFileExists(szQurFolderPath))
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		hFile = CreateFile(szFilePath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in opening existing file %s", szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		if (!dwFileSize)
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in GetFileSize of file %s", szFilePath, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x03;
			goto Cleanup;
		}
		if (lpFileData)
		{
			free(lpFileData);
			lpFileData = NULL;
		}

		lpFileData = (LPBYTE)malloc(dwFileSize);
		if (!lpFileData)
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in allocating memory", 0, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}

		memset(lpFileData, 0x00, dwFileSize);
		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
		ReadFile(hFile, lpFileData, dwFileSize, &dwBytesRead, NULL);
		if (dwFileSize != dwBytesRead)
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in ReadFile of file %s", szFilePath, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!CreateRandomKeyFromFile(hFile, dwFileSize))
		{
			AddLogEntry(L"### CWardwizSCANNERBase : Error in CreateRandomKeyFromFile", 0, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x08;
			goto Cleanup;
		}
		CloseHandle(hFile);

		if (DecryptData((LPBYTE)lpFileData, dwBytesRead))
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in DecryptData", 0, 0, true, SECONDLEVEL);
			dwRet = 0x05;
			goto Cleanup;
		}

		::SetFileAttributes(szFilePath, FILE_ATTRIBUTE_NORMAL);

		TCHAR szTargetFilePath[MAX_FILE_PATH_LENGTH] = { 0 };
		_stprintf(szTargetFilePath, L"%s\\%s.tmp", szQurFolderPath, lpszFileHash);

		//copy here into output parameter
		_tcscpy(lpszTargetFilePath, szTargetFilePath);

		hFileEnc = CreateFile(lpszTargetFilePath, GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileEnc == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### CWardwizSCANNERBase::Error in creating file %s", lpszTargetFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x06;
			goto Cleanup;
		}

		dwBytesRead = 0x00;
		SetFilePointer(hFileEnc, 0x00, NULL, FILE_BEGIN);
		WriteFile(hFileEnc, WRDWIZ_SIG, WRDWIZ_SIG_SIZE, &dwBytesRead, NULL); // Write sig "WARDWIZ"
		if (dwBytesRead != WRDWIZ_SIG_SIZE)
			dwRet = 0x9;

		SetFilePointer(hFileEnc, (0x00 + WRDWIZ_SIG_SIZE), NULL, FILE_BEGIN);
		WriteFile(hFileEnc, m_pbyEncDecKey, WRDWIZ_KEY_SIZE, &dwBytesRead, NULL); // Write Encryption key
		if (dwBytesRead != WRDWIZ_KEY_SIZE)
			dwRet = 0x9;

		SetFilePointer(hFileEnc, (0x0 + WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE), NULL, FILE_BEGIN);
		WriteFile(hFileEnc, lpFileData, dwFileSize, &dwBytesRead, NULL); // Write encrypted data in file
		if (dwFileSize != dwBytesRead)
			dwRet = 0x07;

		CloseHandle(hFileEnc);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup:

	if (lpFileData)
		free(lpFileData);
	lpFileData = NULL;

	if (m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}
	return dwRet;
}

//Prajakta N.
/**********************************************************************************************************
*  Function Name  :	CreateRandomKeyFromFile
*  Description    :	Create a random key to insert into encrypted file.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize)
{
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	bool			bReturn = false;
	int				iTmp = 0x00;
	int				iIndex = 0x00, jIndex = 0x00;
	int				iRandValue = 0x00, iReadPos = 0x00;
	unsigned char	szChar = 0x0;

	iTmp = dwFileSize / WRDWIZ_KEY_SIZE;

	if (m_pbyEncDecKey == NULL)
	{
		m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE, sizeof(unsigned char));
	}
	for (iIndex = 0x00, jIndex = 0x00; iIndex < iTmp; iIndex++, jIndex++)
	{
		if (jIndex >= WRDWIZ_KEY_SIZE)
		{
			break;
		}

		iRandValue = rand();
		iRandValue = iRandValue % WRDWIZ_KEY_SIZE;

		iReadPos = (iIndex *  WRDWIZ_KEY_SIZE) + iRandValue;

		DWORD dwBytesRead = 0x0;
		SetFilePointer(hFile, iReadPos, NULL, FILE_BEGIN);
		ReadFile(hFile, &szChar, sizeof(BYTE), &dwBytesRead, NULL);

		if (szChar == 0x00)
		{
			szChar = iRandValue;
		}
		m_pbyEncDecKey[jIndex] = szChar;

		if (iIndex == (iTmp - 0x01) && jIndex < WRDWIZ_KEY_SIZE)
		{
			iIndex = 0x00;
		}
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	DecryptData
*  Description    :	Encrypt/Decrypt data.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizPreInstallScanDlg::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
{
	__try
	{
		if (IsBadWritePtr(lpBuffer, dwSize))
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00 || m_pbyEncDecKey == NULL)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			//if(lpBuffer[iIndex] != 0)
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
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::DecryptData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name		: GetFileHash(
*  Description			: Function to get file hash
*  Function Arguments	: pFilePath (In), pFileHash(out)
*  Author Name			: Nitin Shelar
*  Date					: 29/11/2018
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash)
{
	bool bReturn = false;
	try
	{

		if (!pFilePath || !pFileHash)
			return bReturn;

		HMODULE hHashDLL = LoadLibrary(L"VBHASH.DLL");
		if (!hHashDLL)
		{
			DWORD dwRet = GetLastError();
			AddLogEntry(L"### Failed in CWardwizPreInstallScanDlg::GetFileHash (%s)", L"VBHASH.DLL");
			return true;
		}

		typedef DWORD(*GETFILEHASH)	(TCHAR *pFilePath, TCHAR *pFileHash);
		GETFILEHASH fpGetFileHash = (GETFILEHASH)GetProcAddress(hHashDLL, "GetFileHash");
		if (!fpGetFileHash)
		{
			AddLogEntry(L"### Failed in CWardwizPreInstallScanDlg::GetFileHash Address(%s)", L"GetFileHash");
			return true;
		}

		DWORD dwRet = fpGetFileHash(pFilePath, pFileHash);
		if (dwRet == 0x00)
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::GetFileHash, FilePath: %s", pFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineEntry
*  Description    :	if ISPYID =0 , wardwiz scanner delete that file
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::QuarantineEntry(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath)
{
	AddLogEntry(L">>> Quarantine Entry [%s]: %s", csVirusName, csQurFilePaths, true, SECONDLEVEL);

	bool bReturn = false;
	try
	{
		m_bFileFailedToDelete = false;
		SetFileAttributes(csQurFilePaths, FILE_ATTRIBUTE_NORMAL); 
		bReturn = ::DeleteFile(csQurFilePaths) ? true : false;
		if (!bReturn)
		{
			CEnumProcess objEnumProcess;
			if (objEnumProcess.IsProcessRunning(csQurFilePaths, true))
			{
				AddLogEntry(L">>> Killing running Process: %s", csQurFilePaths, 0, true, ZEROLEVEL);
				::Sleep(1000);
				SetFileAttributes(csQurFilePaths, FILE_ATTRIBUTE_NORMAL);
				bReturn = ::DeleteFile(csQurFilePaths) ? true : false;
			}
		}
		if (!bReturn)
		{
			m_bFileFailedToDelete = true;
			//WriteFileForBootScan(csQurFilePaths, csVirusName); //for future enhancement
			bReturn = false; //quarantine is failed but we keep the record in the ini.
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::QuarantineFile", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	LoadRecoversDBFile
*  Description    :	Load all entries of recover files.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizPreInstallScanDlg::LoadRecoversDBFile()
{
	CString csDirPath;
	try
	{
		csDirPath = CString(GetProgramFilePath());
		m_objISpyDBManipulation.LoadEntries(0x00, csDirPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::LoadRecoversDBFile", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetProgramFilePath()
*  Description    :	Get program file path as per OS
*  Author Name    : Nitin Shelar
*  Date           : 17/12/2018
*  SR_NO		  :
**********************************************************************************************************/
CString CWardWizPreInstallScanDlg::GetProgramFilePath()
{
	WardWizSystemInfo objSysInfo;
	TCHAR szProgFilesDir[MAX_PATH];
	CString csProgFilePath = L"";
	try
	{
		if (objSysInfo.GetOSType())
		{
			SHGetSpecialFolderPath(0, szProgFilesDir, CSIDL_PROGRAM_FILES, FALSE);
			csProgFilePath = szProgFilesDir;
		}
		else
		{
			SHGetSpecialFolderPath(0, szProgFilesDir, CSIDL_PROGRAM_FILESX86, FALSE);
			csProgFilePath = szProgFilesDir;
		}

		csProgFilePath += L"\\WardWiz";
		if (!PathFileExists(csProgFilePath))
		{
			if (!CreateDirectory(csProgFilePath, NULL))
			{
				AddLogEntry(L"### CWardwizPreInstallScanDlg::Create WardWiz directory failed", 0, 0, true, SECONDLEVEL);
			}
		}
		csProgFilePath += L"\\Quarantine";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::LoadRecoversDBFile", 0, 0, true, SECONDLEVEL);
	}
	return csProgFilePath;
}
/**********************************************************************************************************
*  Function Name		: InsertRecoverEntry
*  Description			: Insert entry into recover DB
*  Function Arguments	: szThreatPath, csDuplicateName, szThreatName.
						  dwShowStatus = 0; Repair/Delete Sucess
						  dwShowStatus = 1; Delete failed
						  dwShowStatus = 2; Repair failed
*  Author Name			: Nitin Shelar
*  Date					: 29/11/2018
*  SR_NO				:
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus)
{
	CIspyList newEntry(szThreatPath, csDuplicateName, szThreatName, L"", L"", L"", dwShowStatus);

	m_objISpyDBManipulation.InsertEntry(newEntry, RECOVER);
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SaveInRecoverDB
*  Description    :	Save all entry into recover files.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::SaveInRecoverDB()
{
	OutputDebugString(L">>> In SaveInReportsDB");

	bool bReturn = false;
	try
	{
		CString csProgramPath = GetProgramFilePath();
		bReturn = m_objISpyDBManipulation.SaveEntries(0x00, csProgramPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::SaveInRecoverDB", 0, 0, true, SECONDLEVEL);
	}
	return  bReturn;
}

/***********************************************************************************************
Function Name  : On_ClickQuickScanCleanBtn
Description    : Cleans detected virus entries
SR.NO		   :
Author Name    : Nitin Shelar
Date           : 29/11/2018
***********************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_ClickScanCleanBtn(SCITER_VALUE svArrCleanEntries, SCITER_VALUE svQarantineFlag)
{
	try
	{
		m_svVirusCount = svQarantineFlag;
		OnClickCleanButton(svArrCleanEntries);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_ClickQuickScanCleanBtn", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	ShutDownScanning
*  Description    :	Shut down scanning with terminating all thread safely.
*  Author Name    : Nitin Shelar
*  SR_NO		  :	
*  Date           : 29/11/2018
**********************************************************************************************************/
bool CWardWizPreInstallScanDlg::OnClickCleanButton(SCITER_VALUE svArrayCleanEntries)
{
	try
	{
		m_svArrayCleanEntries = svArrayCleanEntries;
		bool bIsArray = false;
		svArrayCleanEntries.isolate();
		bIsArray = svArrayCleanEntries.is_array();
		if (!bIsArray)
		{
			return false;
		}
		m_hQuarantineThread = ::CreateThread(NULL, 0, QuarantineThread, (LPVOID) this, 0, NULL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::OnClickCleanButton", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineThread
*  Description    :	If user clicks on clean button.Quarantine thread gets called.
*  Author Name    : Nitin Shelar
*  SR_NO		  :	
*  Date           : 29/11/2018
**********************************************************************************************************/
DWORD WINAPI QuarantineThread(LPVOID lpParam)
{
	try
	{
		CWardWizPreInstallScanDlg *pThis = (CWardWizPreInstallScanDlg *)lpParam;
		DWORD dwAction = 0;
		dwAction = FILEQURENTINED;
		CString csBackupPath;
		//Send file to quarentine
		DWORD dwVirusCount = 0x00;
		dwVirusCount = pThis->m_svArrayCleanEntries.length();
		SCITER_VALUE svFlag = 0;

		for (DWORD dwCurrentVirusEntry = 0; dwCurrentVirusEntry < dwVirusCount; dwCurrentVirusEntry++)
		{
			const SCITER_VALUE svEachEntry = pThis->m_svArrayCleanEntries[dwCurrentVirusEntry];
			const std::wstring chThreatName = svEachEntry[L"ThreatName"].get(L"");
			const std::wstring chFilePath = svEachEntry[L"FilePath"].get(L"");
			DWORD dwCleanResult = pThis->HandleVirusEntry(chFilePath.c_str(), chThreatName.c_str(), L"Quick Scan", pThis->m_wISpywareID, csBackupPath, dwAction);
			if (dwCleanResult != 0x00)
			{
				AddLogEntry(L"### Failed to HandleVirusEntry, FilePath: %s, Threat Name: %s", chFilePath.c_str(), chThreatName.c_str(), true, SECONDLEVEL);
				pThis->m_csScanFile.Unlock();
				MoveFileEx(chFilePath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				//pThis->WriteFileForBootScan(chFilePath.c_str(), chThreatName.c_str());
				svFlag = 1;
			}
		}
		if (svFlag == 1)
		{
			pThis->m_svVirusCount.call(svFlag);
		}
		else
		{
			pThis->m_svVirusCount.call(svFlag);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/**********************************************************************************************************
*  Function Name  :	WriteFileForBootScan
*  Description    :	Write file for Entry of threat which can not be quarantined.
*  Author Name    : Nitin Shelar
*  SR_NO		  :
*  Date           : 19/12/2018
**********************************************************************************************************/
void CWardWizPreInstallScanDlg::WriteFileForBootScan(CString csQurFilePaths, CString csVirusName)
{
	try
	{
		HANDLE	hFile = NULL;
		TCHAR	szThreatFound[MAX_PATH];
		CFile	myFile;
		CString csVirusEntry;
		BYTE	buffer[4096];

		CString csDirPath = GetProgramFilePath();
		csDirPath += L"\\QUARANTINE.DB";
		wsprintf(szThreatFound, L"%s|%s\r\n", csVirusName, csQurFilePaths);
		csVirusEntry = szThreatFound;
		UINT uSize = (UINT)csVirusEntry.GetLength();

		if (!PathFileExists(csDirPath))
		{
			hFile = CreateFile(csDirPath,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
			CloseHandle(hFile);
		}

		if (myFile.Open(csDirPath, CFile::modeReadWrite))
		{
			UINT nBytesRead = myFile.Read(buffer, sizeof(buffer));
			myFile.Write(CT2A(csVirusEntry), uSize);
			myFile.Close();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_RestartForBootScan
*  Description    : Machine gets restart for boot scan.
*  Author Name    : Nitin Shelar
*  Date           : 22/12/2018
****************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_RestartForBootScan(SCITER_VALUE svBootFlag)
{
	DWORD dwBootFlag = 0x00;
	CString csQuarantinePath;
	try
	{

		//CString csGetExePath = L"VibraniumBOOTSCN.EXE";
		//AddBootScannerEntry(SESSION_MANAGER_REG, BOOT_EXEC_REG, csGetExePath.GetBuffer());
		
		csQuarantinePath = GetProgramFilePath();
		csQuarantinePath += L"\\QUARANTINE.DB";
		DeleteFile(csQuarantinePath);
		
		if (svBootFlag == 1)
		{
			CEnumProcess objenumproc;
			objenumproc.RebootSystem(dwBootFlag);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_RestartForBootScan", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***********************************************************************************************
Function Name  : AddBootScannerEntry
Description    : Function to add Boot scanner entry
SR.NO		   :
Author Name    : Nitin Shelar.
Date           : 22/12/2018
***********************************************************************************************/
bool CWardWizPreInstallScanDlg::AddBootScannerEntry(LPTSTR pKeyName, LPTSTR pValueName, LPTSTR pNewValue)
{
	bool bReturn = false;
	LONG lResult = 0;
	HKEY hKey = NULL;
	LPTSTR lpValues = NULL;
	LPTSTR lpValue = NULL;
	LPTSTR lpNewValues = NULL;
	LPTSTR lpNewValue = NULL;										
	DWORD cbValues = 0;
	DWORD cbNewValues = 0;
	DWORD cbNewValue = 0;

	__try
	{
		// OPEN THE REGISTRY KEY
		lResult = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			pKeyName,
			0,
			KEY_ALL_ACCESS | KEY_WOW64_64KEY,
			&hKey
			);

		TCHAR szError[MAX_PATH] = { 0 };
		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			return false;
		}

		// READ THE REG_MULTI_SZ VALUES
		// Get size of the buffer for the values
		lResult = RegQueryValueEx(
			hKey,
			pValueName,
			NULL,
			NULL,
			NULL,
			&cbValues
			);

		if (ERROR_SUCCESS != lResult)
		{
			swprintf(szError, _T("ERROR :[0x%x]"), lResult);
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		// Allocate the buffer
		lpValues = (LPTSTR)malloc(cbValues);
		memset(lpValues, 0, cbValues);

		if (NULL == lpValues)
		{
			swprintf(szError, _T("ERROR 0x%x\n"), GetLastError());
			AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

// Get the values
lResult = RegQueryValueEx(
	hKey,
	pValueName,
	NULL,
	NULL,
	(LPBYTE)lpValues,
	&cbValues
	);

if (ERROR_SUCCESS != lResult)
{
	swprintf(szError, _T("ERROR :[0x%x]"), lResult);
	AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
	bReturn = false;
	goto CLEANUP;
}

// SHOW THE VALUES
lpValue = lpValues;

bool bValueExists = false;
for (; '\0' != *lpValue; lpValue += _tcslen(lpValue) + 1)
{
	if (0 == _tcscmp(lpValue, pNewValue))
	{
		bValueExists = TRUE;
	}
}

//if value exists then no need to update.
if (bValueExists)
{
	bReturn = false;
	goto CLEANUP;
}

// INSERT A NEW VALUE AFTER A SPECIFIC VALUE IN THE LIST OF VALUES
// Allocate a new buffer for the old values plus the new one
cbNewValue = ((DWORD)_tcslen(pNewValue) + 1) * sizeof(TCHAR);
cbNewValues = cbValues + cbNewValue;
lpNewValues = (LPTSTR)malloc(cbNewValues);
memset(lpNewValues, 0, cbNewValues);

if (NULL == lpNewValues)
{
	swprintf(szError, _T("ERROR 0x%x\n"), GetLastError());
	AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
	bReturn = false;
	goto CLEANUP;
}

// Find the value after which we will insert the new one
lpValue = lpValues;
lpNewValue = lpNewValues;

for (; '\0' != *lpValue; lpValue += _tcslen(lpValue) + 1)
{
	// Copy the current value to the target buffer
	memcpy(lpNewValue, lpValue, (_tcslen(lpValue) + 1) * sizeof(TCHAR));
	lpNewValue += _tcslen(lpValue) + 1;
}

//We didn't find the value we wanted. Insert the new value at the end
memcpy(lpNewValue, pNewValue, (_tcslen(pNewValue) + 1) * sizeof(TCHAR));
lpNewValue += _tcslen(pNewValue) + 1;

*lpNewValue = *lpValue;

//WRITE THE NEW VALUES BACK TO THE KEY
lResult = RegSetValueEx(
	hKey,
	pValueName,
	NULL,
	REG_MULTI_SZ,
	(LPBYTE)lpNewValues,
	cbNewValues
	);

if (ERROR_SUCCESS != lResult)
{
	swprintf(szError, _T("ERROR :[0x%x]"), lResult);
	AddLogEntry(L"### RegOpenKeyEx in AddBootScannerEntry %s", szError, 0, true, SECONDLEVEL);
	bReturn = false;
	goto CLEANUP;
}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in AddBootScannerEntry, KeyName:[%s], ValueName: [%s]", pKeyName, pValueName, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:
	if (NULL != lpValues) { free(lpValues); lpValues = NULL; }
	if (NULL != lpNewValues) { free(lpNewValues); lpNewValues = NULL; }
	if (NULL != hKey) { RegCloseKey(hKey); hKey = NULL; }
	return bReturn;
}

/***********************************************************************************************
Function Name  : ForceTerminate
Description    : This function is workaround as process not goes from task manager.
SR.NO		   :
Author Name    : 
Date           : 26/12/2018
***********************************************************************************************/
void CWardWizPreInstallScanDlg::ForceTerminate()
{
	_try
	{
		HANDLE hHandle;
		DWORD dwExitCode = 0;
		hHandle = ::OpenProcess(PROCESS_ALL_ACCESS, 0, GetCurrentProcessId());
		::GetExitCodeProcess(hHandle, &dwExitCode);
		::TerminateProcess(hHandle, dwExitCode);
		CloseHandle(hHandle);
	}
		__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::ForceTerminate", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetString
Description    : Function which returns the string from selected language ini
file.
Author Name    : NITIN SHELAR
Date           : 27/12/2018
****************************************************************************/
CString CWardWizPreInstallScanDlg::GetString(CString csStringID)
{
	TCHAR szValue[2000] = { 0 };
	try
	{
		DWORD dwLangID = GetSelectedLanguage();

		CString csFilePath = GetModuleFileStringPath() + L"\\VBSETTINGS";
		switch (dwLangID)
		{
		case ENGLISH:
			csFilePath += L"\\ENGLISH.INI";
			break;
		case HINDI:
			csFilePath += L"\\HINDI.INI";
			break;
		case GERMAN:
			csFilePath += L"\\GERMAN.INI";
			break;
		case CHINESE:
			csFilePath += L"\\CHINESE.INI";
			break;
		case SPANISH:
			csFilePath += L"\\SPANISH.INI";
			break;
		case FRENCH:
			csFilePath += L"\\FRENCH.INI";
			break;
		}

		if (!PathFileExists(csFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetString", csFilePath, 0, true, SECONDLEVEL);
			return EMPTY_STRING;
		}
		
		GetPrivateProfileString(L"VBSETTINGS", csStringID, L"", szValue, 2000, csFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::GetString", 0, 0, true, SECONDLEVEL);
	}
	return szValue;
}

/***************************************************************************
Function Name  : GetSelectedLanguage
Description    : Function returns DWORD value
				0 - ENGLISH
				1 - HINDI
				2 - GERMAN
				3 - CHINESE
				4 - SPANISH
				5 - FRENCH
Author Name    : NITIN SHELAR
Date           : 27/12/2018
****************************************************************************/
DWORD CWardWizPreInstallScanDlg::GetSelectedLanguage()
{
	CString csIniFilePath;
	try
	{
		csIniFilePath = GetModuleFileStringPath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
		if (!PathFileExists(csIniFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetSelectedLanguage", csIniFilePath, 0, true, SECONDLEVEL);
			return 0xFFFF;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::GetSelectedLanguage", 0, 0, true, SECONDLEVEL);
	}

	return GetPrivateProfileInt(L"VBSETTINGS", L"LanguageID", 0, csIniFilePath);
}

/***************************************************************************************************
*  Function Name  : GetModuleFileStringPath
*  Description    : Get the path where module is exist
*  Author Name    : NITIN SHELAR
*  Date           : 27/12/2018
****************************************************************************************************/
CString CWardWizPreInstallScanDlg::GetModuleFileStringPath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	try
	{
		GetModuleFileName(NULL, szModulePath, MAX_PATH);

		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::GetModuleFileStringPath", 0, 0, true, SECONDLEVEL);
	}
	return(CString(szModulePath));
}

/***************************************************************************************************
Function Name  : PreTranslateMessage
Description    : Ignore Enter/escape button click events
Author Name    : Amol Jaware
Date           : 10th Jan 2019
/***************************************************************************************************/
BOOL CWardWizPreInstallScanDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE || pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_TAB))
	{
			return TRUE;
	}
	if (pMsg->message == WM_CHAR || pMsg->message == WM_UNICHAR)
	{
		WindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
	}
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Akshay Patil
*  Date			  : 04 Jan 2019
****************************************************************************************************/
json::value CWardWizPreInstallScanDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPreInstallScanDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}