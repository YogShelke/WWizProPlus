/**********************************************************************************************************
Program Name          : WardWizInstallerDlg.cpp
Description           : CWardWizInstallerDlg Implementation.
Author Name           : Tejas Shinde
Date Of Creation      : 4/25/2019
Version No            :
Special Logic Used    :
Modification Log      :
1. Name    : Description
***********************************************************************************************************/
// WardWizInstallerDlg.cpp : implementation file
//

#include "stdafx.h"
#include <Psapi.h>
#include "WardWizInstaller.h"
#include "WardWizInstallerDlg.h"
#include "afxdialogex.h"
#include "WardWizDatabaseInterface.h"
#include "WrdWizSystemInfo.h"
#include "WardWizCRC32.h"
#include "CPUInfo.h"
#include "ITinRegDataOperations.h"
#include "EnumProcess.h"
#include <winioctl.h>
#include "CScannerLoad.h"
#include "CSecure64.h"
#include "DriverConstants.h"
#include "CommonFunctions.h"
#include "iSpySrvMgmt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		SETFILEPATH_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 1)
#define		SETFILECOUNT_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 2)
#define		SETFILEPATH_PER_4INSTALLER		(FIRST_APPLICATION_EVENT_CODE + 9)
#define		MAX_RETRY_COUNT					15
#define		TIMER_DELETE_STATUS				100

#define		ERROR_SUCCESS_MSG				L"0"
#define		ERROR_FAILURE_MSG				L"1"

#define		IDR_REGDATA						2000
#define		WSZDRIVE						L"\\\\.\\C:"

GETREGISTRATIONDATA		GetRegistrationData = NULL;
AVACTIVATIONINFO		m_RegDlg_ActInfo = { 0 };
AVACTIVATIONINFO		g_ActInfo = { 0 };

GETINSTALLATIONCODE		g_GetInstallationCode = NULL;
VALIDATERESPONSE		g_ValidateResponse = NULL;

CITinRegDataOperations		g_regDataOperation;

DWORD WINAPI installThread(LPVOID lpvThreadParam);
DWORD WINAPI  AVUninstallThread(LPVOID lpvThreadParam);
DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam);
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam);
DWORD WINAPI QuarantineThread(LPVOID lpParam);
DWORD WINAPI SendRequestThread(LPVOID lpThreadParam);

bool g_bCoInitializeSecurityCalled = false;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CWardWizInstallerDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


HWINDOW   CWardWizInstallerDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizInstallerDlg::get_resource_instance() { return theApp.m_hInstance; }

CWardWizInstallerDlg *g_pThis = NULL;

// CWardWizInstallerDlg dialog
/***********************************************************************************************
*  Function Name  : CWardWizInstallerDlg
*  Description    : Constructor
*  Author Name    : Tejas Shinde
*  SR_NO		  :
*  Date           :  8 March 2019
*************************************************************************************************/
CWardWizInstallerDlg::CWardWizInstallerDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CWardWizInstallerDlg::IDD, pParent)
, m_hThreadinstall(NULL)
, m_hThreadAvUnInstallThread(NULL)
, m_bIsProxySet(false)
, m_bIsOffline(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_pThis = m_pThis = this;
	m_hStartWardWizSetupDwnldProc = NULL;
	EnumProcessModulesWWizEx = NULL;
	m_hThread_ScanCount = NULL;
	m_hQuarantineThread = NULL;
	m_hWardWizAVThread = NULL;
	m_hPsApiDLL = NULL;
	m_iPercentage = 0;
	m_csInstallationFilePath = L"";
	m_hProcess = NULL;
	m_hTmpProcess = NULL;
	m_dwTmpProcID = -1;
	m_hFile = INVALID_HANDLE_VALUE;
	memset(m_szModulePath, 0, sizeof(m_szModulePath));
}

/***************************************************************************
* Function Name  :~CWardWizInstallerDlg
* Description    : Dest'r
* Author Name    : Tejas Shinde
* Date           : 8 March 2019
****************************************************************************/
CWardWizInstallerDlg::~CWardWizInstallerDlg()
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

	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	if (m_hThreadinstall != NULL)
	{
		SuspendThread(m_hThreadinstall);
		TerminateThread(m_hThreadinstall, 0);
		m_hThreadinstall = NULL;
	}

	UnLoadSignatureDatabase();
}

void CWardWizInstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWardWizInstallerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDCANCEL, &CWardWizInstallerDlg::OnBnClickedCancel)
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


#define BASICNCG64		 L"http://updates.wardwiz.in/wwsetup/de/WardWizBasicSetupNCGx64.exe"
#define BASICNCI64		 L"http://updates.wardwiz.in/wwsetup/in/WardWizBasicSetupNCIx64.exe"
#define BASICNCG86		 L"http://updates.wardwiz.in/wwsetup/de/WardWizBasicSetupNCGx86.exe"
#define BASICNCI86		 L"http://updates.wardwiz.in/wwsetup/in/WardWizBasicSetupNCIx86.exe"

#define ESSENTIALNCG64	 L"http://updates.wardwiz.in/wwsetup/de/WardWizEssentialSetupNCGx64.exe"
#define ESSENTIALNCI64	 L"http://updates.wardwiz.in/wwsetup/in/WardWizEssentialSetupNCIx64.exe"
#define ESSENTIALNCG86	 L"http://updates.wardwiz.in/wwsetup/de/WardWizEssentialSetupNCGx86.exe"
#define ESSENTIALNCI86	 L"http://updates.wardwiz.in/wwsetup/in/WardWizEssentialSetupNCIx86.exe"

#define PRONCG64		 L"http://www.wardwiz.de/WardWizAVSetups/WardWizProSetupNCGx64.exe"
#define PRONCI64		 L"http://www.vibranium.co.in/vibraniumAVSetups/WardWizProSetupNCIx64.exe"
#define PRONCG86		 L"http://www.wardwiz.de/WardWizAVSetups/WardWizProSetupNCGx86.exe"
#define PRONCI86		 L"http://www.vibranium.co.in/vibraniumAVSetups/WardWizProSetupNCIx86.exe"

#define ESSPLUSNCG64	 L"http://updates.wardwiz.in/wwsetup/de/WardWizEssPlusSetupNCGx64.exe"
#define ESSPLUSNCI64	 L"http://updates.wardwiz.in/wwsetup/in/WardWizEssPlusSetupNCIx64.exe"
#define ESSPLUSNCG86	 L"http://updates.wardwiz.in/wwsetup/de/WardWizEssPlusSetupNCGx86.exe"
#define ESSPLUSNCI86	 L"http://updates.wardwiz.in/wwsetup/in/WardWizEssPlusSetupNCIx86.exe"

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

CWardWizInstallerDlg * CWardWizInstallerDlg::m_pThis = NULL;
CISpyCommunicatorServer  g_objCommServer(WWIZ_INSTALLER_SERVER, &CWardWizInstallerDlg::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));

// CWardWizInstallerDlg message handlers

/***************************************************************************
* Function Name  : OnInitDialog
* Description    : Load html and initialize.
* Author Name    : Tejas Shinde
* Date           : 8 March 2019
****************************************************************************/
BOOL CWardWizInstallerDlg::OnInitDialog()
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
	IsWow64();
	g_objCommServer.Run();

	TCHAR szLastSetupValue[16] = { 0 };
	TCHAR szLastPathValue[512] = { 0 };
	GetEnvironmentVariable(L"TEMP", m_szTempFolderPath, 511);

	m_csLangType = L"ENGLISH";

	switch (theApp.m_dwProdID)
	{
	case 1:
		//m_csLangType = L"ENGLISH";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEss.INI");
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEss.INI");
		break;
	case 4:
		//m_csLangType = L"ENGLISH";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIB.INI");
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGB.INI");
		break;
	case 5:
		//m_csLangType = L"ENGLISH";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlus.INI");
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlus.INI");
		break;
	case 2://for provantage
		//m_csLangType = L"ENGLISH";
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIPro.INI");
		_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGPro.INI");
		break;
	case 3://for elite
		break;
	}
	
	if (!GetModulePath(m_szModulePath, MAX_PATH))
	{
		AddLogEntry(L">>> Error in Installer GetModulePath  : %s", m_szModulePath, 0, true, ZEROLEVEL);
		return FALSE;
	}

	GetPrivateProfileString(L"Last Setup", L"1", L"", szLastSetupValue, 16, m_szIniFilePath);
	GetPrivateProfileString(L"Last Path", L"1", L"", szLastPathValue, 512, m_szIniFilePath);

	if (_tcscmp(szLastSetupValue, L"32") == 0)
	{
		if (m_bIsWow64)
		{
			m_bchkDownloadoption = true;
		}
		else
		{
			m_bchkDownloadoption = false;
		}

	}
	else if (_tcscmp(szLastSetupValue, L"64") == 0)
	{
		if (m_bIsWow64)
		{
			m_bchkDownloadoption = false;
		}
		else
		{
			m_bchkDownloadoption = true;
		}
	}

	CString csActualDownloadPath(L"");

	if (_tcscmp(szLastPathValue, L"") != 0)
	{
		csActualDownloadPath.Format(L"  :  %s", szLastPathValue);
		AddLogEntry(L">>> Downloaded Path from INI  : %s", szLastPathValue, 0, true, ZEROLEVEL);
	}

	/*m_csActualProdName = L"WardWiz Basic";*/
	//theApp.m_csurl.Format(L"%s", BASICNCG64);//Basic NC Germany 64 url
	//theApp.m_csurl.Format(L"%s", BASICNCI64);//Basic NC India 64 url

	//theApp.m_csurl.Format(L"%s", ESSENTIALNCG64);//Essential NC Germany 64 url
	//theApp.m_csurl.Format(L"%s", ESSENTIALNCI64);//Essential NC India 64 url

	/*#ifdef WRDWIZBASIC
	m_csActualProdName = L"WardWiz Basic";
	#endif*/

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);

	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)AfxGetResourceHandle(), L"res:IDR_HTM_WRDWIZ_INSTALLER.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_WRDWIZ_INSTALLER.htm");
	this->SetWindowText(L"WARDWIZINSTALLER");

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;

	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;

	ModifyStyle(0, WS_SYSMENU | WS_MINIMIZEBOX);

	::MoveWindow(this->get_hwnd(), ixRect / 2, iyRect / 2, pIntMaxWidth, pIntHeight, true);

	CStringArray objcsaWardWizInstallerProcesses;
	objcsaWardWizInstallerProcesses.Add(L"installerEx64");
	objcsaWardWizInstallerProcesses.Add(L"installerEx86");
	objcsaWardWizInstallerProcesses.Add(L"installerGx64");
	objcsaWardWizInstallerProcesses.Add(L"installerGx86");

	for (int iIndex = 0; iIndex < objcsaWardWizInstallerProcesses.GetCount(); iIndex++)
	{
		CString csProcessName = objcsaWardWizInstallerProcesses.GetAt(iIndex);
		LPTSTR lpStr = csProcessName.GetBuffer(csProcessName.GetLength());
		HWND hWindow = ::FindWindow(NULL, lpStr);
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_HIDE);
			::BringWindowToTop(hWindow);
			return FALSE;
		}
	}

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
		AddLogEntry(L"### Excpetion in CWardwizInstallerDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************
* Function Name  : OnSysCommand
* Description    : Creating doModal here.
* Author Name    : Tejas Shinde
* Date           : 8 March 2019
****************************************************************************/
void CWardWizInstallerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

/***************************************************************************
* Function Name  : OnPaint
* Description    : Paint window.
* Author Name    : Tejas Shinde
* Date           : 8 March 2019
****************************************************************************/
void CWardWizInstallerDlg::OnPaint()
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
HCURSOR CWardWizInstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : To handle the Sciter UI related requests
*  Author Name    : Tejas Shinde
*  Date			  : 16 Jan 2019
****************************************************************************************************/
LRESULT CWardWizInstallerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	_try
	{
		if (message == WM_TIMER)
		{
			if (LOWORD(wParam) == TIMER_SCAN_STATUS)
			{
				OnTimerScan();
			}
			else if (LOWORD(wParam) == TIMER_SETPERCENTAGE)
			{
				UIOnTimer();
			}
			else if (LOWORD(wParam) == TIMER_SETFLEPATH_PER)
			{
				OnTimerSetFilePath();
			}
			}
		else
		{
			lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
		}
		if (bHandled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  : PreTranslateMessage
*  Description    : Ignore Enter/escape button click events
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
/***************************************************************************************************/
BOOL CWardWizInstallerDlg::PreTranslateMessage(MSG* pMsg)
{
	try
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_SPACE))
		{
			return TRUE;
		}
		if (pMsg->message == WM_CHAR || pMsg->message == WM_UNICHAR)
		{
			WindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::PreTranslateMessage()", 0, 0, true, ZEROLEVEL);
	}
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************
*  Function Name  : On_Close
*  Description    : To Close the UI Window
*  Author Name    : Tejas Shinde
*  Date           : 29 May 2019
****************************************************************************/
json::value CWardWizInstallerDlg::On_Close()
{
	try
	{
		if (m_hStartWardWizSetupDwnldProc)
		{
			m_pDownloadController->SetThreadPoolStatus(true);
			m_pDownloadController->CancelDownload();
			TerminateThread(m_hStartWardWizSetupDwnldProc, 0);
			CloseHandle(m_hStartWardWizSetupDwnldProc);
		}
		if (m_hTmpProcess)
		{
			TerminateProcess(m_hTmpProcess, 0);
			CloseHandle(m_hTmpProcess);
		}
		if (m_hProcess)
		{
			TerminateProcess(m_hProcess, 0);
			CloseHandle(m_hProcess);
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

		//OnBnClickedCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizInstallerDlg::On_Close", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***********************************************************************************************
Function Name  : ForceTerminate
Description    : This function is workaround as process not goes from task manager.
SR.NO		   :
Author Name    :
Date           : 26/12/2018
***********************************************************************************************/
void CWardWizInstallerDlg::ForceTerminate()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::ForceTerminate", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnLoadInstaller
*  Description    : Function to Download files
*  Author Name    : Tejas Shinde
*  Date           : 16 April 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::OnLoadInstaller(SCITER_VALUE svUpdateDownloadStatus, SCITER_VALUE svNotificationCall, SCITER_VALUE svProductId, SCITER_VALUE svAlreadyDownloadStatus, SCITER_VALUE svSetupDownloadSuccess)
{
	try
	{
		CString GetResponse = L"";
		m_svUpdateDownloadStatus = svUpdateDownloadStatus;
		m_svNotificationCall = svNotificationCall;
		m_pDownloadController = NULL;
		m_dwProductId = static_cast<DWORD>(svProductId.d);
		m_svAlreadyDownloadStatus = svAlreadyDownloadStatus;
		m_svSetupDownloadSuccess = svSetupDownloadSuccess;

		TCHAR szLastPathValue[512] = { 0 };
		if (GetStartupEntry())
		{
			if (szLastPathValue[0])
			{
				GetResponse = CheckDownloadedSetup();
				if (_tcscmp(GetResponse, L"success") == 0)
				{
					m_svAlreadyDownloadStatus.call(true);
				}
				else if (_tcscmp(GetResponse, L"failed") == 0)
				{
					OnBnClickedButtonRun();
				}
			}
			else
			{
				GetResponse = CheckDownloadedSetup();
				if (_tcscmp(GetResponse, L"success") == 0)
				{
					m_svAlreadyDownloadStatus.call(true);
				}
				else if (_tcscmp(GetResponse, L"failed") == 0)
				{
					OnBnClickedButtonRun();
				}
			}
		}
		else
		{
			GetResponse = CheckDownloadedSetup();
			if (_tcscmp(GetResponse, L"success") == 0)
			{
				m_svAlreadyDownloadStatus.call(true);
			}
			else if (_tcscmp(GetResponse, L"failed") == 0)
			{
				OnBnClickedButtonRun();
			}

		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnLoadInstaller", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonRun
*  Description    : Start for downloading the setups
*  Author Name    : Tejas Shinde
*  Date           : 16 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::OnBnClickedButtonRun()
{
	try
	{
		DWORD dwThreadId = 0x00;
		m_csFileTargetPath = L"";
		g_iPreviousPerc = 0x00;
		g_lTotalDownloadedBytes = 0x00;
		g_hFile = NULL;

		if (m_pDownloadController != NULL)
		{
			delete m_pDownloadController;
			m_pDownloadController = NULL;
		}
		m_pDownloadController = new CDownloadController();
		m_pDownloadController->ResetInitData();
		m_pDownloadController->SetGUIInterface(&objGUI);

		/*CString cRemaining;
		cRemaining.Format(_T("  :  %02ld Mins  %02ld Secs"), 0, 0);
		CString csStr;
		csStr.Format(L"%d %s (%d KB %s %d KB)", 0, L"%", static_cast<DWORD>(0), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(0));*/

		m_hStartWardWizSetupDwnldProc = CreateThread(NULL, 0, StartWardWizSetupDownloadProcessThread, (LPVOID) this, 0, &dwThreadId);
		Sleep(500);

		if (m_hStartWardWizSetupDwnldProc == NULL)
			AddLogEntry(L"### Failed in CWardwizInstallerDlg::To create StartVibraniumSetupDownloadProcessThread", 0, 0, true, SECONDLEVEL);
		else
		{
			SetTimer(TIMER_SETPERCENTAGE, 1000, NULL);
			//set flag to inform that downloading thread is created
			m_svNotificationCall.call(6,(SCITER_STRING)L"");
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnBnClickedButtonRun", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : StartWardWizSetupDownloadProcessThread()
*  Description    : Download the wardwiz setup thread
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
****************************************************************************************************/
DWORD WINAPI StartWardWizSetupDownloadProcessThread(LPVOID lpParam)
{
	try
	{
		CWardWizInstallerDlg *pThis = (CWardWizInstallerDlg*)lpParam;
		if (!pThis)
			return 0;

		CString csDrivePath(L"");
		CString csMsgBox(L"");
		CString csBrowsePath(L""), csFiletoCheckAccess(L"");

		csBrowsePath = pThis->m_szTempFolderPath;

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

		csFiletoCheckAccess.Format(L"%s\\%s", csBrowsePath, L"Temp.txt");
		HANDLE hFile = CreateFile(csFiletoCheckAccess, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwerr = GetLastError();
			if (dwerr == ERROR_ACCESS_DENIED)
			{
				pThis->KillTimer(TIMER_SETPERCENTAGE);
				//pThis->KillTimer(TIMER_FORADVERTISEMENT);
				if (theApp.m_dwLangID == 0)
				{
					csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_SELECTED_FOLDER_NOT_ACCESS), theApp.GetString(IDS_STRING_BROWSE_ANOTHER_FOLDER));
				}
				else if (theApp.m_dwLangID == 2)
				{
					csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_SELECTED_FOLDER_NOT_ACCESS_G), theApp.GetString(IDS_STRING_BROWSE_ANOTHER_FOLDER_G));
				}
				pThis->m_svNotificationCall.call(1, (SCITER_STRING)csMsgBox);
				pThis->m_bIsDownloadingInProgress = false;
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

		if (pThis->m_bIsWow64 == true)
		{
			switch (pThis->m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG64SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI64SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", PRONCG64SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", PRONCI64SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3:
				break;
			}
		}
		else
		{
			switch (pThis->m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG86SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI86SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", PRONCG86SETUPNAME);//provantage NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", PRONCI86SETUPNAME);//provantage NC 32 url
				}
				break;
			case 3://for elite
				break;
			}
		}


		/*#ifdef WRDWIZBASIC
		theApp.m_csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC Germany 64 url
		theApp.m_csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC India 64 url
		//theApp.m_csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC 32 url
		//theApp.m_csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC 32 url
		#endif*/
		//theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Essential NC Germany 64 url

		//theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Essential NC India 64 url

		//#ifdef WRDWIZBASIC
		/*if (theApp.m_dwProdID == 4)
		{
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
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
		}*/
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
		//
		//#elif WRDWIZBASICPATCH
		//		//if ((pThis->m_bIsWow64 == true && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", BASICNCG64PATCHNAME);//Basic NC Germany 64 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", BASICNCI64PATCHNAME);//Basic NC India 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", BASICNCG86PATCHNAME);//Basic NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", BASICNCI86PATCHNAME);//Basic NC 32 url
		//#endif
		//		}
		//#elif WRDWIZESSNL
		/*else if (theApp.m_dwProdID == 1)
		{
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
		}*/
		//#elif WRDWIZESSNLPATCH
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG64PATCHNAME);//Ess NC 64 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI64PATCHNAME);//Ess NC 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG86PATCHNAME);//Ess NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI86PATCHNAME);//Ess NC 32 url
		//#endif
		//		}
		//#elif WRDWIZESSPLUS
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG64SETUPNAME);//EssPlus NC 64 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI64SETUPNAME);//EssPlus NC 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG86SETUPNAME);//EssPlus NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI86SETUPNAME);//EssPlus NC 32 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == true))
		//		//{
		//		//	#ifdef RELEASENCG
		//		//				theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
		//		//	#elif RELEASENCI
		//		//				theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
		//		//	#else
		//		//				theApp.m_csFilePath.Format(L"%s", ESSENTIAL86SETUPNAME);//Ess with clam 32 url
		//		//	#endif
		//		//}
		//		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == true))
		//		//{
		//		//	#ifdef RELEASENCG
		//		//				theApp.m_csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
		//		//	#elif RELEASENCI
		//		//				theApp.m_csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
		//		//	#else
		//		//				theApp.m_csFilePath.Format(L"%s", ESSENTIAL64SETUPNAME);//Ess with clam  64 url
		//		//	#endif
		//		//}
		//#elif WRDWIZESSPLUSPATCH
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG64PATCHNAME);//EssPlus NC 64 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI64PATCHNAME);//EssPlus NC 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCG86PATCHNAME);//EssPlus NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", ESSPLUSNCI86PATCHNAME);//EssPlus NC 32 url
		//#endif
		//		}
		//#else
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", PRONCG64SETUPNAME);//pro NC 64 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", PRONCI64SETUPNAME);//Pro NC 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csFilePath.Format(L"%s", PRONCG86SETUPNAME);//pro NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csFilePath.Format(L"%s", PRONCI86SETUPNAME);//Pro NC 32 url
		//#endif
		//		}
		//#endif

		pThis->m_csTmpProcess.Format(L"%s",theApp.m_csFilePath);
		pThis->m_csTmpProcess.Replace(L".exe", L".tmp");

		CString csFilePath = L"";
		csFilePath.Format(L"%s\\%s", csBrowsePath, theApp.m_csFilePath);
		//pThis->m_stActualDownloadPath.SetWindowTextW(L"  :  " + csFilePath);
		//pThis->m_stActualDownloadPath.RedrawWindow();
		pThis->EndWaitCursor();

		//Issue No. 0001618: If machine gets restarts while downloading is in progress in win 10 after restarting the machine download is not getting started.
		pThis->SetStartupEntry(pThis->m_szModulePath);

		//pThis->m_stActualStatus.SetWindowTextW(theApp.GetString(IDS_STRING_CHECK_INTERNET_CONNECTION));
		DWORD dwRetry = 0;


		if (!pThis->CheckInternetConnection())
		{

			pThis->m_bIsCloseCalled = false;
			CString csMsg = L"";
			if (theApp.m_dwLangID == 0)
			{
				csMsg.Format(L"%s \n%s", theApp.GetString(IDS_STRING_UNABLE_TO_DETECT_CONNECT), theApp.GetString(IDS_STRING_REQ_TO_CHECK_INETERNET));
			}
			else if (theApp.m_dwLangID == 2)
			{
				csMsg.Format(L"%s \n%s", theApp.GetString(IDS_STRING_UNABLE_TO_DETECT_CONNECT_G), theApp.GetString(IDS_STRING_REQ_TO_CHECK_INETERNET_G));
			}
			pThis->KillTimer(TIMER_SETPERCENTAGE);
			pThis->m_bIsDownloadingInProgress = false;
			pThis->m_bIsProcessPaused = true;
			pThis->m_svNotificationCall.call(5, (SCITER_STRING)csMsg);
			return 0x01;
		}
		else
		{
			AddLogEntry(L">>> internet connection available", 0, 0, true, FIRSTLEVEL);
		}

		pThis->m_objWinHttpManager.m_bIsConnected = true;

		pThis->m_objWinHttpManager.StartCurrentDownload();

		if (pThis->m_bIsWow64 == true)
		{
			switch (pThis->m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", ESSENTIALNCG64);//Ess NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", ESSENTIALNCI64);//Ess NC 64 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", BASICNCG64);//Basic NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", BASICNCI64);//Basic NC 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", ESSPLUSNCG64);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", ESSPLUSNCI64);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", PRONCG64);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", PRONCI64);//provantage NC 64 url
				}
				break;
			case 3://for elite
				break;
			}
		}
		else
		{
			switch (pThis->m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", ESSENTIALNCG86);//Ess NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", ESSENTIALNCI86);//Ess NC 32 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", BASICNCG86);//Basic NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", BASICNCI86);//Basic NC 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", ESSPLUSNCG86);//Ess plus NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", ESSPLUSNCI86);//Ess plus NC 32 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					theApp.m_csurl.Format(L"%s", PRONCG86);//provantage NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					theApp.m_csurl.Format(L"%s", PRONCI86);//provantage NC 32 url
				}
				break;
			case 3://for elite
				break;
			}
		}

		/*#ifdef WRDWIZBASIC
		theApp.m_csurl.Format(L"%s", BASICNCG64);//Basic NC Germany 64 url
		theApp.m_csurl.Format(L"%s", BASICNCI64);//Basic NC India 64 url
		//theApp.m_csurl.Format(L"%s", BASICNCG86);//Basic NC 32 url
		//theApp.m_csurl.Format(L"%s", BASICNCI86);//Basic NC 32 url
		#endif*/
		//#ifdef WRDWIZBASIC
		//else if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == true))
		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == true))
		/*if (theApp.m_dwProdID == 4)
		{
		if (pThis->m_bIsWow64 == true)
		{
		#ifdef RELEASENCG
		theApp.m_csurl.Format(L"%s", BASICNCG64);//Basic NC 64 url
		#elif RELEASENCI
		theApp.m_csurl.Format(L"%s", BASICNCI64);//Basic NC 64 url
		#else
		theApp.m_csurl.Format(L"%s", BASIC64);//Basic with clam  64 url
		#endif
		//pThis->m_chkLaunchDownloadedExe.EnableWindow(FALSE);
		//pThis->m_bchkLaunchExe = false;
		}
		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == true))
		else
		{
		#ifdef RELEASENCG
		theApp.m_csurl.Format(L"%s", BASICNCG86);//Basic NC 32 url
		#elif RELEASENCI
		theApp.m_csurl.Format(L"%s", BASICNCI86);//Basic NC 32 url
		#else
		theApp.m_csurl.Format(L"%s", BASIC86);//Basic with clam 32 url
		#endif
		//pThis->m_chkLaunchDownloadedExe.EnableWindow(FALSE);
		//pThis->m_bchkLaunchExe = false;
		}
		}*/
		//#elif WRDWIZBASICPATCH
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", BASICNCG64PATCH);//Basic NC Germany 64 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", BASICNCI64PATCH);//Basic NC India 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", BASICNCG86PATCH);//Basic NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", BASICNCI86PATCH);//Basic NC 32 url
		//#endif
		//		}
		//#elif WRDWIZESSNL
		/*else if (theApp.m_dwProdID == 1)
		{
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
		}*/
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
		//#elif WRDWIZESSNLPATCH
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", ESSNCG64PATCH);//Ess NC 64 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", ESSNCI64PATCH);//Ess NC 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false))//&& (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", ESSNCG86PATCH);//Ess NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", ESSNCI86PATCH);//Ess NC 32 url
		//#endif
		//		}
		//
		//#elif WRDWIZESSPLUS
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", ESSPLUSNCG64);//EssPlus NC 64 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", ESSPLUSNCI64);//EssPlus NC 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false))//&& (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", ESSPLUSNCG86);//EssPlus NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", ESSPLUSNCI86);//EssPlus NC 32 url
		//#endif
		//		}
		//#elif WRDWIZESSPLUSPATCH
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", ESSPLNCG64PATCH);//EssPlus NC 64 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", ESSPLNCI64PATCH);//EssPlus NC 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false))//&& (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", ESSPLNCG86PATCH);//EssPlus NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", ESSPLNCI86PATCH);//EssPlus NC 32 url
		//#endif
		//		}
		//#else
		//		//if ((pThis->m_bIsWow64 == true) && (pThis->m_bchkDownloadoption == false))
		//		if (pThis->m_bIsWow64 == true)
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", PRONCG64);//pro NC 64 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", PRONCI64);//Pro NC 64 url
		//#endif
		//		}
		//		//else if ((pThis->m_bIsWow64 == false) && (pThis->m_bchkDownloadoption == false))
		//		else
		//		{
		//#ifdef RELEASENCG
		//			theApp.m_csurl.Format(L"%s", PRONCG86);//pro NC 32 url
		//#elif RELEASENCI
		//			theApp.m_csurl.Format(L"%s", PRONCI86);//Pro NC 32 url
		//#endif
		//		}
		//#endif
		CString strStartTime = pThis->m_tsStartTime.Format("%H:%M:%S");

		AddLogEntry(L">>> Downloaded started at %s", strStartTime, 0, true, SECONDLEVEL);
		AddLogEntry(L">>> Wardwiz Setup Server path %s", theApp.m_csurl, 0, true, SECONDLEVEL);

		CString csNewBrowsePath;
		bool bFailed = false;


		CString csTempFileName(L"");
		CString csFileName(theApp.m_csurl);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);
		pThis->m_csFileName = csFileName;

		//Issue No. 0001485 resolved: Issue with Download Manager. If already setup exist on selected path, user can download the same setup again, It is not giving any Information popup.
		pThis->CheckAlreadySetupExit();
		if (_tcscmp(pThis->m_GetOnlineSetupResponse, L"success") == 0)
		{
			pThis->m_svAlreadyDownloadStatus.call(true);
		}
		else if (_tcscmp(pThis->m_GetOnlineSetupResponse, L"failed") == 0)
		{
			pThis->m_bIsCloseCalled = true;
			if (theApp.m_dwLangID == 0)
			{
				csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_SETUP_ALREADY_EXIST), theApp.GetString(IDS_STRING_WANT_TO_OVERWRITE));
			}
			else if (theApp.m_dwLangID == 2)
			{
				csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_SETUP_ALREADY_EXIST_G), theApp.GetString(IDS_STRING_WANT_TO_OVERWRITE_G));
			}
			pThis->m_svVarOnClickOverWriteCB = pThis->m_svNotificationCall.call(2, (SCITER_STRING)csMsgBox);
			//if (pThis->MessageBox(csMsgBox, L"Vibranium", MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
			pThis->m_bIsCloseCalled = false;
			if (pThis->m_svVarOnClickOverWriteCB == 1)
			{
				pThis->m_svAlreadyDownloadStatus.call((SCITER_VALUE)true);
				return 0X00;
			}
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
			pThis->m_svNotificationCall.call(1, (SCITER_STRING)csMsgBox);
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
				bIsDownloadSuccess = false;
			}
		}

		if (bIsDownloadSuccess)
		{
			int iLowRange = 0, iHighRange = 0;
			//pThis->m_prgDownloadSetupStatus.GetRange(iLowRange, iHighRange);

			CString csStr(L"");
			if (pThis->m_dwTotalFileSize >= 1000000)
			{
				if (pThis->m_iCurrentDownloadedByte >= 1000000)
				{
					csStr.Format(L"%d %s (%d MB %s %d MB)", iHighRange, L"%", static_cast<DWORD>(pThis->m_dwTotalFileSize / (1000 * 1024)), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(pThis->m_dwTotalFileSize / (1000 * 1024)));
				}
				else
				{
					csStr.Format(L"%d %s (%d KB %s %d MB)", iHighRange, L"%", static_cast<DWORD>(pThis->m_dwTotalFileSize / (1024)), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(pThis->m_dwTotalFileSize / (1000 * 1024)));
				}
			}
			else
			{
				csStr.Format(L"%d %s (%d KB %s %d KB)", iHighRange, L"%", static_cast<DWORD>(pThis->m_dwTotalFileSize / 1024), theApp.GetString(IDS_STRING_DOWNLOAD_OF), static_cast<DWORD>(pThis->m_dwTotalFileSize / 1024));
			}

			//pThis->m_prgDownloadSetupStatus.SetWindowTextW(csStr);
			//pThis->m_prgDownloadSetupStatus.SetPos(100);
			//pThis->m_prgDownloadSetupStatus.RedrawWindow();
			// issue :in case of re- download progress bar once showing 100 percent then showing actual status.
			// resolved by lalit kumawat 9-8-2015

			pThis->m_iCurrentDownloadedByte = pThis->m_dwTotalFileSize = 0;

			AddLogEntry(L">>> EXE file downloaded successfully", 0, 0, true, ZEROLEVEL);

			Sleep(1000);

			if (pThis->m_hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(pThis->m_hFile);
				pThis->m_hFile = INVALID_HANDLE_VALUE;
			}

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
				long lResult = RegDeleteValueW(hKey, L"WardWizInstaller");

				if (lResult != ERROR_SUCCESS)
				{
					AddLogEntry(L"### Filed to delete registry file", 0, 0, true, FIRSTLEVEL);
				}
			}

			/*if (pThis->m_bchkOpenFolder == true)
			{
				CString csOpenFolderPath(L"");
				csOpenFolderPath.Format(L"%s", pThis->m_csFileTargetPath);
				int Pos = csOpenFolderPath.ReverseFind('\\');
				csOpenFolderPath.Truncate(Pos);
				//OutputDebugString(L">>>>>>>>> open folder");
				ShellExecute(NULL, L"open", csOpenFolderPath, NULL, NULL, SW_SHOW);
			}*/


			pThis->m_objWinHttpManager.m_bIsConnected = false;
			//ini file into program data\wardwiz folder.

			// issue :812, Issue with the popup appearing after cancelling to store the setup in other location if USB drive not available.
			// resolved by lalit kumawat 9-8-2015
                    
			        if (!bFailed)
					{
						pThis->m_bIsCloseCalled = true;
						if (theApp.m_dwLangID == 0)
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_SETUP_DOWNLOADED_SUCESSFULLY));
						}
						else if (theApp.m_dwLangID == 2)
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_SETUP_DOWNLOADED_SUCESSFULLY_G));
						}
						pThis->m_svSetupDownloadSuccess.call((SCITER_VALUE)pThis->m_bIsCloseCalled);
						pThis->m_bIsSetupDownloadSuccess = true;
						//pThis->m_svNotificationCall.call(4, (SCITER_STRING)csMsgBox);
						// Issue no: 0001490, After completion of download, setup is not launching automatically.
					}
					else
					{
						if (theApp.m_dwLangID == 0)
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_SETUP_DOWNLOAD_FAILED));
						}
						else if (theApp.m_dwLangID == 2)
						{
							csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_SETUP_DOWNLOAD_FAILED_G));
						}
						pThis->m_svNotificationCall.call(1, (SCITER_STRING)csMsgBox);
					}
		}
		else
		{
			pThis->m_iCurrentDownloadedByte = pThis->m_dwTotalFileSize = 0;
			pThis->KillTimer(TIMER_SETPERCENTAGE);
			//pThis->KillTimer(TIMER_FORADVERTISEMENT);
			//pThis->m_stActualTransferRate.SetWindowTextW(L"  :  0.00 kbps");
			if (pThis->m_bRequiredSpaceNotavailable == true)
			{
				//pThis->m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_STATUS_NO_ENOUGH_SPACE));
			}
			else
			{
				//pThis->m_stActualStatus.SetWindowTextW(L"  :  " + theApp.GetString(IDS_STRING_STATUS_FAIL_TRY_AGAIN));
			}

			Sleep(1000);

			pThis->m_bIsDownloadingInProgress = false;
			pThis->RestoreWndFromTray(pThis->m_hWnd);
			pThis->m_objWinHttpManager.StopCurrentDownload();
			pThis->m_objWinHttpManager.SetDownloadCompletedBytes(0);
			pThis->m_objWinHttpManager.CloseFileHandles();
			pThis->m_objWinHttpManager.m_bIsConnected = false;


			if (pThis->m_hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(pThis->m_hFile);
				pThis->m_hFile = INVALID_HANDLE_VALUE;
			}
			pThis->m_bchkLaunchExe = true;
			//CWrdWizCustomMsgBox objCustomMsgBox(pThis);
			CString csMsg;

			//if (objCustomMsgBox.m_hWnd == NULL)
			//{
			if (pThis->m_bRequiredSpaceNotavailable == true)
			{
				pThis->m_bIsCloseCalled = true;
				//objCustomMsgBox.m_csFailedMsgText.Format(L"%s\n\t\t\t\t%s\n%s", theApp.GetString(IDS_STRING_MSG_NO_ENOUGH_SPACE), theApp.GetString(IDS_STRING_OR), theApp.GetString(IDS_STRING_DWONLOAD_MANUAL));
				if (theApp.m_dwLangID == 0)
				{
					csMsg.Format(L"%s", theApp.GetString(IDS_STRING_MSG_NO_ENOUGH_SPACE));
				}
				else if (theApp.m_dwLangID == 2)
				{
					csMsg.Format(L"%s", theApp.GetString(IDS_STRING_MSG_NO_ENOUGH_SPACE_G));
				}
				pThis->m_svNotificationCall.call(1, (SCITER_STRING)csMsg);
			}
			else
			{
				pThis->m_bIsCloseCalled = true;
				//objCustomMsgBox.m_csFailedMsgText.Format(L"%s\n\t\t\t%s\n%s\n", theApp.GetString(IDS_STRING_STATUS_FAIL_TRY_AGAIN), theApp.GetString(IDS_STRING_OR), theApp.GetString(IDS_STRING_DWONLOAD_MANUAL));
				//csMsg.Format(L"%s\n\t\t\t\t%s\n%s\n\n<a href=\"%s\">%s</a>", theApp.GetString(IDS_STRING_STATUS_FAIL_TRY_AGAIN), theApp.GetString(IDS_STRING_OR), theApp.GetString(IDS_STRING_DWONLOAD_MANUAL), theApp.m_csurl, theApp.m_csurl);
				if (theApp.m_dwLangID == 0)
				{
					csMsg.Format(L"%s \n%s", theApp.GetString(IDS_STRING_UNABLE_TO_DETECT_CONNECT), theApp.GetString(IDS_STRING_REQ_TO_CHECK_INETERNET));
				}
				else if (theApp.m_dwLangID == 2)
				{
					csMsg.Format(L"%s \n%s", theApp.GetString(IDS_STRING_UNABLE_TO_DETECT_CONNECT_G), theApp.GetString(IDS_STRING_REQ_TO_CHECK_INETERNET_G));
				}
				pThis->KillTimer(TIMER_SETPERCENTAGE);
				pThis->m_bIsDownloadingInProgress = false;
				pThis->m_bIsProcessPaused = true;
				pThis->m_svNotificationCall.call(5, (SCITER_STRING)csMsg);
			}

			AddLogEntry(L"### Failed to download Exe file %s", theApp.m_csurl, 0, true, SECONDLEVEL);
			return 0x01;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::StartVibraniumSetupDownloadProcessThread", 0, 0, true, SECONDLEVEL);
	}
	return 0x00;
}

/***************************************************************************************************
*  Function Name  : CheckInternetConnection()
*  Description    : It checks internet connection .
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::CheckInternetConnection()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckInternetConnection", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : WaitForInternetConnection()
*  Description    : It waits internet connection for 5 min .
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::WaitForInternetConnection()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::WaitForInternetConnection", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : OnTimer
*  Description    : The framework calls this member function after each interval specified in the SetTimer member function used to install a timer.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::OnTimer(UINT_PTR nIDEvent)
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
				cSpeed.Format(_T("    %0.2f Mbps"), (lSpeed_Kbps / 1000));
			}
			else
			{
				cSpeed.Format(_T("    %0.2f Kbps"), lSpeed_Kbps);
			}

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

			if (theApp.m_dwLangID == 0)
			{
				csPercetage.Format(L"WardWiz (%d%s) %s", m_dwPercentage, L"%", theApp.GetString(IDS_STRING_TRAYTXT_DOWNLOADED));
			}
			else if (theApp.m_dwLangID == 2)
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


			m_svUpdateDownloadStatus.call((SCITER_STRING)cRemaining, (SCITER_STRING)cSpeed, (SCITER_STRING)csPercent);
		}

		CDialogEx::OnTimer(nIDEvent);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnTimer", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : OnTimerSetFilePath
*  Description    : Set percentage and file path of while installing wardwiz main setup.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::OnTimerSetFilePath()
{
	try
	{
		sciter::value map;
		map.set_item("one", (SCITER_STRING)m_pThis->m_csInstallationFilePath);
		map.set_item("two", m_pThis->m_iPercentage);

		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETFILEPATH_PER_4INSTALLER;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnTimerSetFilePath", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : UIOnTimer
*  Description    : Set status for wardwiz setup downloading.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::UIOnTimer()
{
	try
	{
		CString csPercetage(L"");

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
			cSpeed.Format(_T("    %0.2f Mbps"), (lSpeed_Kbps / 1000));
		}
		else
		{
			cSpeed.Format(_T("    %0.2f Kbps"), lSpeed_Kbps);
		}

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

		if (theApp.m_dwLangID == 0)
		{
			csPercetage.Format(L"WardWiz (%d%s) %s", m_dwPercentage, L"%", theApp.GetString(IDS_STRING_TRAYTXT_DOWNLOADED));
		}
		else if (theApp.m_dwLangID == 2)
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


		m_svUpdateDownloadStatus.call((SCITER_STRING)cRemaining, (SCITER_STRING)cSpeed, (SCITER_STRING)csPercent);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::UIOnTimer", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetPercentage()
*  Description    : Calculating percentage
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetPercentage(int iDownloaded, int iTotalSize)
{
	__try
	{
		return static_cast<DWORD>(((static_cast<double>((iDownloaded)) / iTotalSize)) * 100);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ShowNotifyIcon
*  Description    : Notifies the notification area, icon to tray
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
****************************************************************************************************/
VOID CWardWizInstallerDlg::ShowNotifyIcon(HWND hWnd, DWORD dwAdd, CString csMessage)
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
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::ShowNotifyIcon", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : StartDownloadingWardWizSetup()
*  Description    : Start downloading setup file .
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::StartDownloadingWardWizSetup(LPCTSTR szUrlPath)
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
		//csTempFileName.Format(L"%s", csFileName);
		//csTempFileName.Format(L"%s.%s", csFileName, L"PART_1");
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

			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 0)
					_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEss.INI");
				else if (theApp.m_dwLangID == 2)
					_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEss.INI");
				break;
			case 4:
				if (theApp.m_dwLangID == 0)
					_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIB.INI");
				else if (theApp.m_dwLangID == 2)
					_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGB.INI");
				break;
			case 5:
				if (theApp.m_dwLangID == 0)
					_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlus.INI");
				else if (theApp.m_dwLangID == 2)
					_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlus.INI");
				break;
			case 2://for provantage
				if (theApp.m_dwLangID == 0)
					_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIPro.INI");
				else if (theApp.m_dwLangID == 2)
					_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGPro.INI");
				break;
			case 3://for elite
				break;
			}

			//#ifdef WRDWIZBASIC
			/*if (theApp.m_dwProdID == 4)
			{
			if (theApp.m_dwLangID == 0)
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizIMNCIB.INI");
			else if (theApp.m_dwLangID == 2)
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizIMNCGB.INI");
			}*/
			//#endif
			//#elif WRDWIZBASICPATCH
			//#ifdef RELEASENCG
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGBPatch.INI");
			//#elif RELEASENCI
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIBPatch.INI");
			//#endif
			//#elif WRDWIZESSNL
			/*#ifdef RELEASENCG
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEss.INI");
			#elif RELEASENCI
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEss.INI");
			#endif*/
			/*else if (theApp.m_dwProdID == 4)
			{
			if (theApp.m_dwLangID == 0)
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEss.INI");
			else if (theApp.m_dwLangID == 2)
			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEss.INI");
			}*/
			//#elif WRDWIZESSNLPATCH
			//#ifdef RELEASENCG
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPatch.INI");
			//#elif RELEASENCI
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPatch.INI");
			//#endif
			//#elif WRDWIZESSPLUS
			//#ifdef RELEASENCG
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlus.INI");
			//#elif RELEASENCI
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlus.INI");
			//#endif
			//#elif WRDWIZESSPLUSPATCH
			//#ifdef RELEASENCG
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlusPatch.INI");
			//#elif RELEASENCI
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlusPatch.INI");
			//#endif
			//#else
			//#ifdef RELEASENCG
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGP.INI");
			//#elif RELEASENCI
			//			_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIP.INI");
			//#endif
			//#endif


			GetPrivateProfileString(L"Last Setup Size", L"1", L"", szLastSetupFileSize, (MAX_PATH - 1), m_szIniFilePath);

			if (szLastSetupFileSize[0])
			{
				dwLastSetupTotalSize = _wtol(szLastSetupFileSize);
			}

			m_dwTotalFileSize = dwTotalFileSize;

			CString csBrowseFolderName;
			csBrowseFolderName.Format(L"%s", m_szTempFolderPath);
			if (PathFileExists(csBrowseFolderName))
			{
				AddLogEntry(L">>> Wardwiz Setup downloaded path %s", csBrowseFolderName, 0, true, SECONDLEVEL);
				_tcscpy_s(m_szBrowseFolderName, _countof(m_szBrowseFolderName), csBrowseFolderName);
			}
			else
			{
				AddLogEntry(L"### %s Folder is not exist", csBrowseFolderName, 0, true, FIRSTLEVEL);
				return false;
			}

			_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, csTempFileName);

			m_csTempTargetFilePath.Format(L"%s", szTagetPath);

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
					m_hFile = INVALID_HANDLE_VALUE;
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
			/*if (m_objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize))
			{
			//Once download complete set the download completed bytes.
			m_objWinHttpManager.SetDownloadCompletedBytes(dwTotalFileSize - dwStartBytes);
			}
			else
			{
			AddLogEntry(L"### Failed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
			return false;
			}*/
			if (StartDownloadFile(szUrlPath, szTagetPath))
			{
				AddLogEntry(L"### StartDownloadFile Successed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
			}
			else
			{
				AddLogEntry(L"### StartDownloadFile Failed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
				return false;
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::StartDownloadingGenXSetup", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : StartDownloadFile()
*  Description    : Start downloading files on by one .
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0052
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizInstallerDlg::StartDownloadFile(LPCTSTR szUrlPath, TCHAR szTargPath[MAX_PATH])
{
	try
	{
		if (!szUrlPath)
			return false;

		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		STRUCT_DOWNLOAD_INFO sDownloadInfo = { 0 };
		_tcscpy_s(sDownloadInfo.szMainUrl, URL_SIZE, szUrlPath);
		_tcscpy_s(sDownloadInfo.szSectionName, csFileName);
		_tcscpy_s(sDownloadInfo.szExeName, csFileName);

		DWORD dwThreadSize = DEFAULT_DOWNLOAD_THREAD;
		/*if (g_bRequestFromUI)
		{
		dwThreadSize = DEFAULT_DOWNLOAD_THREAD + 0x02;
		}*/

		sDownloadInfo.dwDownloadThreadCount = dwThreadSize;
		_stprintf_s(sDownloadInfo.szLocalTempDownloadPath, MAX_PATH, L"%s", szTargPath);
		_stprintf_s(sDownloadInfo.szLocalPath, MAX_PATH, L"%s", szTargPath);

		TCHAR szTagetPath[MAX_PATH] = { 0 };
		_stprintf_s(szTagetPath, MAX_PATH, L"%s", szTargPath);

		//_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s\\%s", g_szAppDataFolder, g_csAppFolderName, csFileName);

		DWORD dwTotalFileSize = 0x00;
		TCHAR szInfo[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		CWWizHttpManager objWinHttpManager;
		if (!objWinHttpManager.Initialize(szUrlPath))
		{
			AddLogEntry(L"### Initialize Failed in StartDownloadFile, URLPath: [%s]", szUrlPath, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
		{
			AddLogEntry(L"### GetHeaderInfo Failed in StartDownloadFile, URLPath: [%s]", szUrlPath, 0, true, SECONDLEVEL);
			return false;
		}
		dwTotalFileSize = _wtol(szInfo);

		DWORD dwStartBytes = 0;
		if (PathFileExists(szTagetPath))
		{
			g_hFile = CreateFile(szTagetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (g_hFile != INVALID_HANDLE_VALUE)
			{
				dwStartBytes = GetFileSize(g_hFile, 0);
				CloseHandle(g_hFile);
				g_hFile = NULL;
			}

			if (dwStartBytes == dwTotalFileSize)
			{
				if (!CheckForCorruption(szTagetPath))
				{
					g_lTotalDownloadedBytes += dwTotalFileSize;
					objGUI.SetDownloadedBytes(dwTotalFileSize, dwTotalFileSize, 0);
					return true;
				}
			}
			SetFileAttributes(szTagetPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szTagetPath);

		}

		if (!m_pDownloadController)
			return false;

		bool bDownloadFailed = false;
		DWORD dwRetryCount = 0x00;
		do
		{
			bDownloadFailed = false;
			m_pDownloadController->ResetInitData();
			if (!m_pDownloadController->StartController(&sDownloadInfo))
			{
				AddLogEntry(L"### Retry to download: FilePath: [%s\\%s]", sDownloadInfo.szLocalTempDownloadPath, sDownloadInfo.szExeName, true, FIRSTLEVEL);
				bDownloadFailed = true;
				dwRetryCount++;
			}

			if (bDownloadFailed)
			{
				//if physics file size is greater than server file size then download from BEGIN
				if (PathFileExists(szTagetPath))
				{
					//Delete the file and download again.
					SetFileAttributes(szTagetPath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szTagetPath);
					g_objWinHttpManager.SetDownloadCompletedBytes(0);
					dwStartBytes = 0;
				}				

				if(!bDownloadFailed)
				{
					//Start download for file
					objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize);
					if (PathFileExists(szTagetPath))
					{
						bDownloadFailed = false;
						break;
					}
				}
				else
				{
					break;
				}
			}
		} while (bDownloadFailed && dwRetryCount < MAX_RETRY_COUNT);

		if (bDownloadFailed)
		{
			//if physics file size is greater than server file size then download from BEGIN
			if (PathFileExists(szTagetPath))
			{
				//Delete the file and download again.
				SetFileAttributes(szTagetPath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(szTagetPath);
				g_objWinHttpManager.SetDownloadCompletedBytes(0);
				dwStartBytes = 0;
			}

			if (!bDownloadFailed)
			{
				//Start download for file
				objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize);
				if (!PathFileExists(szTagetPath))
				{
					AddLogEntry(L"### Failed to download file [%s]", szUrlPath, 0, true, SECONDLEVEL);
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		AddLogEntry(L">>> File Downloaded: FilePath: [%s\\%s]", sDownloadInfo.szLocalTempDownloadPath, sDownloadInfo.szExeName, true, ZEROLEVEL);
		g_lTotalDownloadedBytes += dwTotalFileSize;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::StartDownloadFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : RestoreWndFromTray
*  Description    : It restores window from tray
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
VOID CWardWizInstallerDlg::RestoreWndFromTray(HWND hWnd)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::RestoreWndFromTray", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetTrayWndRect
*  Description    : It gives tray window rectangle co-ordinates.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
VOID CWardWizInstallerDlg::GetTrayWndRect(LPRECT lpTrayRect)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetTrayWndRect", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetDoAnimateMinimize
*  Description    : Check to see if the animation has been disabled
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
BOOL CWardWizInstallerDlg::GetDoAnimateMinimize(VOID)
{
	ANIMATIONINFO ai;
	__try
	{
		ai.cbSize = sizeof(ai);
		SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetDoAnimateMinimize", 0, 0, true, SECONDLEVEL);
	}

	return ai.iMinAnimate ? TRUE : FALSE;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonClose
*  Description    : It will restore the setup file path, size of last setup file and last target path
into INI. Also in "Run" registry.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::OnBnClickedButtonClose()
{
	try
	{
		if (m_bIsDownloadingInProgress)
		{
			m_bIsProcessPaused = true;
			CString csMsgBox;
			if (theApp.m_dwLangID == 0)
			{
				csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_MSG_DOWNLOAD_IN_PROGRESS), theApp.GetString(IDS_STRING_WANT_TO_DOWNLOAD));
			}
			else if (theApp.m_dwLangID == 2)
			{
				csMsgBox.Format(L"%s \n%s", theApp.GetString(IDS_STRING_MSG_DOWNLOAD_IN_PROGRESS_G), theApp.GetString(IDS_STRING_WANT_TO_DOWNLOAD_G));
			}

			if (m_hStartWardWizSetupDwnldProc)
			{
				SuspendThread(m_hStartWardWizSetupDwnldProc);

				if (TerminateThread(m_hStartWardWizSetupDwnldProc, 0x00))
				{
					AddLogEntry(L">>> Download Wardwiz setup thread stopped successfully.", 0, 0, true, ZEROLEVEL);
					CloseHandle(m_hStartWardWizSetupDwnldProc);
					m_hStartWardWizSetupDwnldProc = NULL;
				}
			}
			m_objWinHttpManager.StopCurrentDownload();
			m_objWinHttpManager.SetDownloadCompletedBytes(0);
			m_objWinHttpManager.CloseFileHandles();

			if (m_hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
			}

			if ((DWORD)m_iCurrentDownloadedByte < m_dwTotalFileSize)
			{

				switch (m_dwProductId)
				{
				case 1:
					if (theApp.m_dwLangID == 2)
						_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEss.INI");
					else if (theApp.m_dwLangID == 0)
						_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEss.INI");
					break;
				case 4:
					if (theApp.m_dwLangID == 2)
						_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGB.INI");
					else if (theApp.m_dwLangID == 0)
						_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIB.INI");
					break;
				case 5:
					if (theApp.m_dwLangID == 2)
						_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGEssPlus.INI");
					else if (theApp.m_dwLangID == 0)
						_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIEssPlus.INI");
					break;
				case 2:
					if (theApp.m_dwLangID == 0)
						_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCIPro.INI");
					else if (theApp.m_dwLangID == 2)
						_stprintf_s(m_szIniFilePath, MAX_PATH, L"%s\\%s", m_szTempFolderPath, L"WWizDMNCGPro.INI");
					break;
				case 3://for elite
					break;
				}
				/*
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
				*/

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

				WritePrivateProfileString(L"Last Path ", L"1", m_szTempFolderPath, m_szIniFilePath);
			}

			KillTimer(TIMER_SETPERCENTAGE);
			//KillTimer(TIMER_FORADVERTISEMENT);
			//MessageBox(theApp.GetString(IDS_STRING_DOWNLOAD_ABORTED), L"Vibranium", MB_OK|MB_ICONEXCLAMATION);
			if (theApp.m_dwLangID == 0)
			{
				csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_DOWNLOAD_ABORTED));
			}
			else if (theApp.m_dwLangID == 2)
			{
				csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_DOWNLOAD_ABORTED_G));
			}
			//MessageBox(csMsgBox, L"Vibranium", MB_OK | MB_ICONEXCLAMATION);
			m_bIsCloseCalled = true;
			m_bIsDownloadingInProgress = true;
			m_svNotificationCall.call(1, (SCITER_STRING)csMsgBox);
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
		AddLogEntry(L"### CWardwizInstallerDlg::OnBnClickedButtonClose::Exception", 0, 0, true, SECONDLEVEL);
	}

	return;
}

/***************************************************************************************************
*  Function Name  : LaunchWwizNExitInstaller
*  Description    : Function to Close Installer UI and Wardwiz UI Page Open
*  Author Name    : Tejas Shinde
*  Date           : 23 May 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::LaunchWwizNExitInstaller()
{
	try
	{
		CancelUI();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::LaunchVibraniumNExitInstaller", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : CancelUI
*  Description    : Function to Close Window and launch UI.exe
*  Author Name    : Tejas Shinde
*  Date           : 13 May 2019
****************************************************************************************************/
void CWardWizInstallerDlg::CancelUI()
{
	try
	{
		CString csFinalUninstallFirstParameter, FinalUninstallSecondParameter;
		csFinalUninstallFirstParameter = GetWardWizPathFromRegistry() + L"VBUI.EXE";
		FinalUninstallSecondParameter = L"-SHOWREG";
		ShellExecute(NULL, L"open", csFinalUninstallFirstParameter, FinalUninstallSecondParameter, NULL, SW_SHOW);
		HWND hWindow = ::FindWindow(NULL, L"VBUI");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
		}

		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CancelUI", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CloseUI
*  Description    : Function to Close Window  if setup download gets failed.
*  Author Name    : Tejas Shinde
*  Date           : 13 May 2019
****************************************************************************************************/
void CWardWizInstallerDlg::CloseUI()
{
	try
	{
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CloseUI", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonWindowClose
*  Description    : Function to Close Window.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::OnBnClickedButtonWindowClose()
{
	try
	{
		OnBnClickedButtonClose();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnBnClickedButtonWindowClose", 0, 0, true, SECONDLEVEL);
	}
}

/*****************************************************************************************
*  Function Name  : GetEmptyDrivePath
*  Description    : it provides the other empty drive if current drive full percentage.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
******************************************************************************************/
bool CWardWizInstallerDlg::IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::IsDriveHaveRequiredSpace", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : On_BtnClickResume
*  Description    : Function to Resume Download
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_BtnClickResume()
{
	try
	{
		m_bIsCloseCalled = false;
		OnBnClickedButtonResume();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_BtnClickResume", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonResume
*  Description    : It resumes the process.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::OnBnClickedButtonResume()
{
	try
	{
		CString csMsgBox;
		if (!m_hStartWardWizSetupDwnldProc)
		{
			AddLogEntry(L"### Wardwiz download setup thread is not running", 0, 0, true, ZEROLEVEL);
			return;
		}

		SetTimer(TIMER_SETPERCENTAGE, 10, NULL);

		if (m_objWinHttpManager.m_bIsConnected == true)
		{

			if (!PathFileExists(m_csTempTargetFilePath))
			{
				AddLogEntry(L"### %s path is not exist", m_csTempTargetFilePath, 0, true, FIRSTLEVEL);
				if (theApp.m_dwLangID == 0)
				{
					csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_NO_TARGETPATH_RESTART_DOWNLOAD));
				}
				else if (theApp.m_dwLangID == 2)
				{
					csMsgBox.Format(L"%s", theApp.GetString(IDS_STRING_NO_TARGETPATH_RESTART_DOWNLOAD_G));
				}
				sciter::value v_svVarNoTargetPathCB = m_svNotificationCall.call(2, (SCITER_STRING)csMsgBox);

				if (v_svVarNoTargetPathCB == 0)
				{
					if (m_hStartWardWizSetupDwnldProc)
					{
						SuspendThread(m_hStartWardWizSetupDwnldProc);

						if (TerminateThread(m_hStartWardWizSetupDwnldProc, 0x00))
						{
							AddLogEntry(L">>> Download Wardwiz setup thread stopped successfully.", 0, 0, true, ZEROLEVEL);
							CloseHandle(m_hStartWardWizSetupDwnldProc);
							m_hStartWardWizSetupDwnldProc = NULL;
						}
					}
					m_objWinHttpManager.StopCurrentDownload();
					m_objWinHttpManager.SetDownloadCompletedBytes(0);
					m_objWinHttpManager.CloseFileHandles();
					if (m_hFile != INVALID_HANDLE_VALUE)
					{
						CloseHandle(m_hFile);
						m_hFile = INVALID_HANDLE_VALUE;
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
							CloseHandle(m_hStartWardWizSetupDwnldProc);
							m_hStartWardWizSetupDwnldProc = NULL;
						}
					}
					m_objWinHttpManager.StopCurrentDownload();
					m_objWinHttpManager.SetDownloadCompletedBytes(0);
					m_objWinHttpManager.CloseFileHandles();
					if (m_hFile != INVALID_HANDLE_VALUE)
					{
						CloseHandle(m_hFile);
						m_hFile = INVALID_HANDLE_VALUE;
					}

					KillTimer(TIMER_SETPERCENTAGE);
				}
			}
		}
		m_bIsProcessPaused = false;
	}
	catch (...)
	{
		AddLogEntry(L"### CWardwizInstallerDlg::OnBnClickedButtonResume::Exception", 0, 0, true, SECONDLEVEL);
	}

	return;
}

/**********************************************************************************************************
*  Function Name  :	SetStartupEntry
*  Description    :	Set a downloader entry into registry.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::SetStartupEntry(TCHAR* szDownloaderPath)
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

		LPCTSTR SetPath = TEXT("WardWizInstaller");
		DWORD dwDataLen = 512;
		TCHAR szDownloadpath[512] = { 0 };
		_tcscpy_s(szDownloadpath, _countof(szDownloadpath), szDownloaderPath);

		switch(m_dwProductId)
		{
		case 1:
			if (theApp.m_dwLangID == 2)
				_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCEG.exe");
			else if (theApp.m_dwLangID == 0)
				_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCEI.exe");
			break;
		case 4:
			if (theApp.m_dwLangID == 2)
				_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZINSTALLERNCGB.EXE");
			else if (theApp.m_dwLangID == 0)
				_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZINSTALLERNCIB.EXE");
			break;
		case 5:
			if (theApp.m_dwLangID == 2)
				_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCEPG.exe");
			else if (theApp.m_dwLangID == 0)
				_tcscat_s(szDownloadpath, _countof(szDownloadpath), L"\\WRDWIZDMNCEPI.exe");
			break;
		case 2://for provantage
			break;
		case 3://for elite
			break;
		}

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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SetStartupEntry", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetStartupEntry
*  Description    : Get a downloader entry into registry.
*  Author Name    : Tejas Shinde
*  Date           : 22 April 20195
**********************************************************************************************************/
bool CWardWizInstallerDlg::GetStartupEntry()
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

		LPCTSTR SetPath = TEXT("WardWizInstaller");
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetStartupEntry", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
*  Function Name  : GetTaskBarHeight
*  Description    : To get the Task Bar Height
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
****************************************************************************/
int CWardWizInstallerDlg::GetTaskBarHeight()
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
		AddLogEntry(L"### Excpetion in CWardwizInstallerDlg::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
*  Function Name  : GetTaskBarWidth
*  Description    : To get the Task Bar Width
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
****************************************************************************/
int CWardWizInstallerDlg::GetTaskBarWidth()
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
		AddLogEntry(L"### Excpetion in CWardwizInstallerDlg::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : OnBnClickedCancel()
Description    : function to close dialog box
Author Name    : Tejas Shinde
Date           : 8 March 2019
****************************************************************************/
void CWardWizInstallerDlg::OnBnClickedCancel()
{
	HWND hWindow = ::FindWindow(NULL, L"WARDWIZINSTALLER");
	if (hWindow)
	{
		::ShowWindow(hWindow, SW_RESTORE);
		::BringWindowToTop(hWindow);
    }
	const SCITER_VALUE result = m_root_el.call_function("InstallingClose");
}

/***************************************************************************************************
*  Function Name  : onModalLoop
*  Description    : for reseting the Lightbox event msgbox
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;
			theApp.m_objCompleteEvent.SetEvent();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::onModalLoop()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_Minimize
*  Description    : Minimize window while pressed on UI.
*  Author Name    : Tejas Shinde
*  Date           : 13 March 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_Minimize()
{
	try
	{
		::ShowWindow(this->get_hwnd(), SW_MINIMIZE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_Minimize()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetModuleFileStringPath
*  Description    : Get the path where module is exist
*  Author Name    : Tejas Shinde
*  Date           : 25 March 2019
****************************************************************************************************/
CString CWardWizInstallerDlg::GetModuleFileStringPath()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetModuleFileStringPath", 0, 0, true, SECONDLEVEL);
	}
	return(CString(szModulePath));
}


/*************************************************************************************
*  Function Name  : CheckOtherAVProduct
*  Description    : This function is used to Install Wardwiz Setup
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
*************************************************************************************/
json::value CWardWizInstallerDlg::CheckOtherAVProduct(SCITER_VALUE svCallFinishXMLStatus, SCITER_VALUE cvSendUninstallAvDetails2UI, SCITER_VALUE svPreInstallScnStatus)
{
	try
	{
		m_svCallFinishXMLStatus = svCallFinishXMLStatus;
		m_cvSendUninstallAvDetails2UI = cvSendUninstallAvDetails2UI;
		m_svPreInstallScnStatus = svPreInstallScnStatus;
		StartCheckOtherAvProgram();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckOtherAVProduct", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/*******************************************************************************************************
*  Function Name  : StartCheckOtherAvProgram
*  Description    : Starts the Checking Other Antivirus Information process by creating UninstallThread
*  Author Name    : Tejas Shinde
*  Date           : 8 April 2019
*********************************************************************************************************/
void CWardWizInstallerDlg::StartCheckOtherAvProgram()
{
	m_binstallInProgress = TRUE;

	if (m_hThreadAvUnInstallThread)
	{
		m_hThreadAvUnInstallThread = NULL;
	}

	if (!m_hThreadAvUnInstallThread)
	{
		m_hThreadAvUnInstallThread = ::CreateThread(NULL, 0, AVUninstallThread, (LPVOID) this, 0, NULL);
		Sleep(500);
	}

	SetTimer(TIMER_DELETE_STATUS, 10, NULL);
}

/***************************************************************************************************
*  Function Name  : AVUninstallThread
*  Description    : This Function Alredy Downloaded WWIZ Setup Performs Installing Setup.
*  Author Name    : Tejas Shinde
*  Date           : 8 April 2019
****************************************************************************************************/
DWORD WINAPI AVUninstallThread(LPVOID lpParam)
{
	try
	{
		CWardWizInstallerDlg *pThis = (CWardWizInstallerDlg*)lpParam;
		if (!pThis)
			return 0;

		pThis->CheckOtherAVProductWardwizSetup();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::installThread", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/*************************************************************************************
*  Function Name  : StartInstallWardwizSetup
*  Description    : This function is used to Install Wardwiz Setup
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
*************************************************************************************/
json::value CWardWizInstallerDlg::StartInstallWardwizSetup(SCITER_VALUE svInstallFinishFlag, SCITER_VALUE svbRegFlag, SCITER_VALUE svCloseMsg, SCITER_VALUE svChkVerMsg, SCITER_VALUE svChkInstallFinishStatus)
{
	try
	{
		m_bIsRegFlag = svbRegFlag.get(false);
		m_svInstallFinishFlagCB = svInstallFinishFlag;
		m_svCloseMsg = svCloseMsg;
		m_svChkVerMsg = svChkVerMsg;
		m_svSetupInstalledSuccess = svChkInstallFinishStatus;
		SetTimer(TIMER_SETFLEPATH_PER, 50, NULL);
		StartInstallProgram();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::StartInstallGenXSetup", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : StartUninstallProgram
*  Description    : Starts the uninstallation process by creating UninstallThread
*  Author Name    : Tejas Shinde
*  Date           : 8 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::StartInstallProgram()
{
	m_binstallInProgress = TRUE;
	if (!m_hThreadinstall)
	{
		m_hThreadinstall = NULL;
	}

	m_hThreadinstall = ::CreateThread(NULL, 0, installThread, (LPVOID) this, 0, NULL);

	Sleep(500);

	SetTimer(TIMER_DELETE_STATUS, 10, NULL);
}

/***************************************************************************************************
*  Function Name  : installThread
*  Description    : This Function Alredy Downloaded WWIZ Setup Performs Installing Setup.
*  Author Name    : Tejas Shinde
*  Date           : 8 April 2019
****************************************************************************************************/
DWORD WINAPI installThread(LPVOID lpParam)
{
	try
	{
		CWardWizInstallerDlg *pThis = (CWardWizInstallerDlg*)lpParam;
		if (!pThis)
			return 0;
		
		pThis->InstallWardwizSetup();
		
		if (!pThis->TerminateInstallThreads())
		{
			AddLogEntry(L"### Failed to Terminate installation threads");
		}

		if (!pThis->RegisterForProtection())
		{
			AddLogEntry(L"### Failed to Register for Protection");
		}

		if (!pThis->PerformPostRegOperation())
		{
			AddLogEntry(L"### Failed to perform post registration operation");
		}

		Sleep(100);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::installThread", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : IsWow64()
*  Description    : It will check client machine is 64 bit or 32bit.
*  Author Name    : Tejas Shinde
*  Date           : 22 March 2019
****************************************************************************************************/
void CWardWizInstallerDlg::IsWow64()
{
	TCHAR				szOSVer[16] = { 0 };
	SYSTEM_INFO			sysInfo = { 0 };
	__try
	{

		SYSTEM_INFO			sysInfo = { 0 };
		GetNativeSystemInfo(&sysInfo);
		if ((sysInfo.wProcessorArchitecture&PROCESSOR_ARCHITECTURE_AMD64) == PROCESSOR_ARCHITECTURE_AMD64)
		{
			m_bIsWow64 = true;
		}
		else
		{
			m_bIsWow64 = false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::IsWow64", 0, 0, true, SECONDLEVEL);
	}
}

/*************************************************************************************
*  Function Name  : CheckOtherAVProductWardwizSetup
*  Description    : This function is used to Installing AvCleaner Setup
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
*************************************************************************************/
bool CWardWizInstallerDlg::CheckOtherAVProductWardwizSetup()
{
	CString Langname, csWWizSetupFinalPath, csWWizSetupCommndline;
	try
	{
		if (m_bIsWow64 == true)
		{
			csWWizSetupFinalPath.Format(L"%s%s", theApp.GetModuleFilePath(), L"\\incompcheckx64.exe");
			csWWizSetupCommndline.Format(L"%s", L"/verysilent");
			LPTSTR csFinalWWIZAVSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
			LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
			InstallWWizSetupThrCommandLine(csFinalWWIZAVSetupExePath, csFinalWWIZInstallCommandLine);
		}
		else
		{
			csWWizSetupFinalPath.Format(L"%s%s", theApp.GetModuleFilePath(), L"\\incompcheckx86.exe");
			csWWizSetupCommndline.Format(L"%s", L"/verysilent");
			LPTSTR csFinalWWIZAVSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
			LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
			InstallWWizSetupThrCommandLine(csFinalWWIZAVSetupExePath, csFinalWWIZInstallCommandLine);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckOtherAVProductVibraniumSetup", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/*************************************************************************************
*  Function Name  : InstallWardwizSetup
*  Description    : This function is used to Install Wardwiz Setup
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
*************************************************************************************/
bool CWardWizInstallerDlg::InstallWardwizSetup()
{
	CString Langname, csWWizSetupFinalPath, csWWizSetupCommndline, csFilePath ,csLanguageName;
	TCHAR WWIZSetupExeDesktopPath[MAX_PATH];
	TCHAR AlreadyWWIZSetupExeExistPath[MAX_PATH];
	try
	{
		IsWow64();

		if (m_bIsWow64 == true)
		{
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSPLUSNCG64SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSPLUSNCI64SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", PRONCG64SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", PRONCI64SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3:
				break;
			}
		}
		else
		{
			m_csSetupBit = L"32";
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSPLUSNCG86SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSPLUSNCI86SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", PRONCG86SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", PRONCI86SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3://for elite
				break;
			}
		}

		swprintf_s(WWIZSetupExeDesktopPath, L"%s\\%s", m_szTempFolderPath, csFilePath);
		swprintf_s(AlreadyWWIZSetupExeExistPath, L"%s\\%s", m_szWWizInstallerLocationPath, csFilePath);
		
		if (m_bIsSetupDownloadSuccess==true)
		{
			if (PathFileExists(WWIZSetupExeDesktopPath))
			{
				if (m_bIsWow64 == true)
				{
					csWWizSetupFinalPath.Format(L"%s", WWIZSetupExeDesktopPath);
					csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
					LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
					LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
					InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
				}
				else
				{
					csWWizSetupFinalPath.Format(L"%s", WWIZSetupExeDesktopPath);
					csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
					LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
					LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
					InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
				}
			}
		}
		if (m_svGetResponseFinalInstall==0)
		{
			if ((PathFileExists(AlreadyWWIZSetupExeExistPath)))
			{
				if (m_bIsWow64 == true)
				{
					csWWizSetupFinalPath.Format(L"%s", AlreadyWWIZSetupExeExistPath);
					csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
					LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
					LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
					InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
				}
				else
				{
					csWWizSetupFinalPath.Format(L"%s", AlreadyWWIZSetupExeExistPath);
					csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
					LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
					LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
					InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
				}
			}
			else if ((PathFileExists(m_WWizOfflineSetupFinalPath)))
			{
				if (m_bIsWow64 == true)
				{
					csWWizSetupFinalPath.Format(L"%s", m_WWizOfflineSetupFinalPath);
					csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
					LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
					LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
					InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
				}
				else
				{
					csWWizSetupFinalPath.Format(L"%s", m_WWizOfflineSetupFinalPath);
					csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
					LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
					LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
					InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
				}
		   }
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::InstallVibraniumSetup", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***********************************************************************************************
*  Function Name  : InstallWWizSetupThrCommandLine
*  Description    : This function will get CommandLine As Paramaters and Execute the Command
*  Author Name    : Tejas Shinde
*  Date           : 22 March 2019
***********************************************************************************************/
BOOL CWardWizInstallerDlg::InstallWWizSetupThrCommandLine(LPTSTR pszWWIZSetupPath, LPTSTR pszCmdLine)
{
	if (!pszWWIZSetupPath)
		return FALSE;
	
	if (!pszCmdLine)
		return FALSE;

	STARTUPINFO			si = { 0 };
	PROCESS_INFORMATION	pi = { 0 };

	try
	{

		si.cb = sizeof(STARTUPINFO);

		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		TCHAR commandLine[2 * MAX_PATH + 16] = { 0 };
		swprintf_s(commandLine, _countof(commandLine), L"\"%s\" %s ", pszWWIZSetupPath, pszCmdLine);
		if (m_hProcess)
		{
			m_hProcess = NULL;
		}
		if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		{
			AddLogEntry(L"### Failed CWardwizInstallerDlg::InstallWardwizSetupThrCommandLine : [%s]", commandLine);
			return TRUE;
		}
		m_hProcess = pi.hProcess;
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizInstallerDlg::InstallWardwizSetupThrCommandLine");
		return TRUE;
	}

	return FALSE;
}

/***************************************************************************************************
*  Function Name  : OnDataReceiveCallBack
*  Description    : Receive flag set from pipe
*  Author Name    :  Tejas Shinde
*  Date           :  16 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::OnDataReceiveCallBack(LPVOID lpParam)
{
	DWORD dwReply = 0;
	try
	{

		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		int iRetDays = 0;
		dwReply = 0;

		switch (lpSpyData->iMessageInfo)
		{

		case SEND_OTHER_AV_DETAILS:

			if (lpSpyData->dwValue == FINISH_EXTRACT_FILES)
			{
				m_pThis->SendExtractFinishedFlag(lpSpyData->szFirstParam);
				dwReply = 1;
			}
			else if (lpSpyData->dwValue == UNINSTALL_AV_DETAILS)
			{
				if (m_pThis->SendWWizUninstallAvDetails2UI(lpSpyData->szFirstParam))
				{
					dwReply = 1;
				}
			}
			else if (lpSpyData->dwValue == FINISH_UNINSTALL_AV_DETAILS)
			{
				m_pThis->SendWWizXMLFinishFlag(lpSpyData->dwSecondValue);
			}
			else if (lpSpyData->dwValue == INSTALLER_LOCATION_PATH)
			{
				m_pThis->SendInstallerLocationPath(lpSpyData->szFirstParam);
				dwReply = 1;
			}

			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;

		case SEND_INSTALLER_STATUS_DETAILS:

			if (lpSpyData->dwValue == FILE_PATH_DETAILS)
			{
				m_pThis->m_csInstallationFilePath = lpSpyData->szFirstParam;
				m_pThis->m_iPercentage = int(lpSpyData->dwSecondValue);
				dwReply = 1;
			}
			else if (lpSpyData->dwValue == CHKVER_MSG)
			{
				if (m_pThis->CheckVersionAppMsg(lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->szThirdParam))
				{
					dwReply = 1;
				}
			}
			else if (lpSpyData->dwValue == REINSTALL_MSG)
			{
				if (m_pThis->ReinstallCloseAppMsg(lpSpyData->szFirstParam))
				{
					dwReply = 1;
				}
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;	

		case SEND_INSTALLER_STATUS_SUCCESS:
			
			if (lpSpyData->dwValue == FINISH_INSTALLATIONS)
			{
				m_pThis->SendFinishedInstallationsFlag(lpSpyData->dwSecondValue);
			}

			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  SendExtractFinishedFlag
*  Description    :  Send flag once finished Extracting Setup Files
*  Author Name    : Tejas Shinde
*  Date           : 25 June 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::SendExtractFinishedFlag(LPTSTR m_csSetupExtractTempFolderPath)
{
	try
	{
		int iTemp = 1;
		m_svGetProdIdNVersion.call();

		if (!m_csSetupExtractTempFolderPath)
		{
			return 0;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SendExtractFinishedFlag", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  :  SendWWizInstallationPath2UI
*  Description    :  Send file path to UI.
*  Author Name    :  Tejas Shinde.
*  Date           :  16 April 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::SendWWizInstallationPath2UI(LPTSTR szFilePath, int iPercentage)
{
	try
	{

		if (!szFilePath)
		{
			return 0;
		}

		sciter::value map;
		map.set_item("one", (SCITER_STRING)szFilePath);
		map.set_item("two", iPercentage);

		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETFILEPATH_PER_4INSTALLER;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SendVibraniumInstallationPath2UI", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  SendWWizUninstallAvDetails2UI
*  Description    :  Send file path to UI.
*  Author Name    :  Tejas Shinde.
*  Date           :  16 April 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::SendWWizUninstallAvDetails2UI(LPTSTR szUninstallAvDetails)
{
	try
	{
		if (!szUninstallAvDetails)
		{
			return 0;
		}

		int iPos = 0;
		CString csUninstallAvInfo = szUninstallAvDetails;
		CString csAvDisplayName, csAvDisplayIcon, csAvFirstParamUninstallString, csAvSecondParamUninstallString = L"";
		csAvDisplayIcon = csUninstallAvInfo.Tokenize(_T("|"), iPos);
		csAvDisplayIcon.Trim();
		csUninstallAvInfo.Delete(0, iPos + 1);
		iPos = 0;
		csAvDisplayName = csUninstallAvInfo.Tokenize(_T("|"), iPos);
		csAvDisplayName.Trim();

		csAvFirstParamUninstallString = csUninstallAvInfo.Tokenize(_T("|"), iPos);
		csAvFirstParamUninstallString.Trim();

		csAvSecondParamUninstallString = csUninstallAvInfo.Tokenize(_T("|"), iPos);
		csAvSecondParamUninstallString.Trim();

		m_cvSendUninstallAvDetails2UI.call(sciter::string(csAvDisplayIcon), sciter::string(csAvDisplayName), sciter::string(csAvFirstParamUninstallString), sciter::string(csAvSecondParamUninstallString));

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SendVibraniumUninstallAvDetails2UI", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  SendWWizXMLFinishFlag
*  Description    :  Send flag once finished reading AVXML
*  Author Name    :  Tejas Shinde.
*  Date           :  16 April 2019
****************************************************************************************************/
void CWardWizInstallerDlg::SendWWizXMLFinishFlag(DWORD dwFinishFlag)
{
	try
	{
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::KillTimer(pwnd->m_hWnd, TIMER_SETPERCENTAGE);
		}

		if (dwFinishFlag == 0)
			m_svCallFinishXMLStatus.call(true);

		sciter::dom::element(self).stop_timer();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SendVibraniumXMLFinishFlag", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  On_LaunchAvToUninstall
*  Description    :  Get Antivirus Uninstallstring from UI and Execute Uninstall.exe.
*  Author Name    :  Tejas Shinde.
*  Date           :  30 April 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_LaunchAvToUninstall(SCITER_VALUE svUninstallStringFirstParam, SCITER_VALUE svUninstallStringSecondParam)
{
	int iPos = 0;
	try
	{
		CString csFinalUninstallFirstParameter, csFinalUninstallSecondParameter ,csGetUninstallStringFirstParam, csGetUninstallStringSecondParam(L"");
		sciter::string m_svUninstallStringFirstParam, m_svUninstallStringSecondParam;

		m_svUninstallStringFirstParam = svUninstallStringFirstParam.to_string();
		m_svUninstallStringSecondParam = svUninstallStringSecondParam.to_string();

		csGetUninstallStringFirstParam = m_svUninstallStringFirstParam.c_str();
		csGetUninstallStringSecondParam = m_svUninstallStringSecondParam.c_str();

		if (!csGetUninstallStringFirstParam && !csGetUninstallStringSecondParam)
		{
			return 0;
		}

		csFinalUninstallFirstParameter = csGetUninstallStringFirstParam;
		csFinalUninstallSecondParameter = csGetUninstallStringSecondParam;

		int SlashPos = csFinalUninstallFirstParameter.Find(_T("\\"), iPos);
		int iTemp = csFinalUninstallFirstParameter.ReverseFind('\\');
		CString csTemp = csFinalUninstallFirstParameter;

		csTemp.Delete(0, ++iTemp);

		SHELLEXECUTEINFO sei = { sizeof(sei) };
		ZeroMemory(&sei, sizeof(sei));
		sei.lpFile = csFinalUninstallFirstParameter;
		sei.lpParameters = csFinalUninstallSecondParameter;
		sei.lpDirectory = NULL;
		sei.nShow = SW_HIDE;
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShellExecuteEx(&sei);
		DWORD dwProcessID = GetProcessId(sei.hProcess);
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);

		if (hProcess != NULL)
			v_hProcess.push_back(hProcess);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_LaunchAvToUninstall", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : On_StartQuickScan
*  Description    : Accepts the request from UI and starts the Quick scan
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_StartQuickScan(SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB)
{
	try
	{
		v_hProcess.clear();
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_StartQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/**********************************************************************************************************
*  Function Name  :	StartScanning
*  Description    :	To start Full scan ,custom scan and quick scan accoeding m_scantype variable.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::StartScanning()
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

		if (GetMachineIDOnMemory())
			AddLogEntry(L"#### Machine ID Generated Successfully!", 0, 0, true, SECONDLEVEL);
		else
			AddLogEntry(L"#### Failed to Generate Machine ID!", 0, 0, true, SECONDLEVEL);


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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::StartScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	LoadSignatureDatabase
*  Description    :	Function to load Signature database.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
DWORD CWardWizInstallerDlg::LoadSignatureDatabase(DWORD &dwSigCount)
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.LoadSignatureDatabase(dwSigCount);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	GetModuleCount
*  Description    :	Give total modules of proesses in the case of quick scan
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::GetModuleCount()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetModuleCount", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************
*  Function Name  : IsDuplicateModule
*  Description    : Function to find duplicates modules to avoid multiple scanning.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
****************************************************************************************/
bool CWardWizInstallerDlg::IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::IsDuplicateModule", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/*********************************************************************************
*  Function Name  : GetTotalScanFilesCount
*  Description    : Get total files count in case of fullscan and custom scan
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************/
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam)
{
	try
	{
		CWardWizInstallerDlg *pThis = (CWardWizInstallerDlg*)lpParam;
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetTotalScanFilesCount", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	EnumFolder
*  Description    :	Enumerate each folders of system and calculate total files count.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::EnumFolder(LPCTSTR pstr)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/******************************************************************************************************************************
*  Function Name  : EnumerateThread
*  Description    : Thread to enumerarte process in case of quick scan and  files and folders in case of custom and full scan.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
********************************************************************************************************************************/
DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizInstallerDlg *pThis = (CWardWizInstallerDlg*)lpvThreadParam;
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::EnumerateThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	EnumerateProcesses
*  Description    :	Enumaerate processes in case of quick scan.
Changes (Ram) : Time complexity decresed as we enumerating processes and modules
to calculate file count, There is no need to enumerate it again.
kept in CStringList while file count calculation, same list is used again.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::EnumerateProcesses()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::EnumerateProcesses", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	EnumFolderForScanning
*  Description    :	enumerate files of system and sent it to scan.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::EnumFolderForScanning(LPCTSTR pstr)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::EnumFolderForScanning", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckFileOrFolderOnRootPath
*  Description    : Function which check IS file/Folder present on root path
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
/***************************************************************************************************/
bool CWardWizInstallerDlg::CheckFileOrFolderOnRootPath(CString csFilePath)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckFileOrFolderOnRootPath, Path: %s", csFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	ScanForSingleFile
*  Description    :	Scan each single file .
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::ScanForSingleFile(CString csFilePath)
{
	bool bFound = false;
	bool bRescan = true;
	DWORD dwISpyID = 0;
	DWORD dwReturn = 0x00;
	//int arrIndex = 0;
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
							csStatus.Format(L"%s", GetString(L"IDS_CONSTANT_THREAT_DETECTED"));
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
		AddLogEntry(L"### Exception in void CWardwizInstallerDlg::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallUISetVirusFoundEntryfunction
*  Description    : used to insert threat found entries
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
****************************************************************************************************/
void CWardWizInstallerDlg::CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID)
{
	try
	{
		m_svAddVirusFoundEntryCB.call(SCITER_STRING(csVirusName), SCITER_STRING(csFilePath), SCITER_STRING(csActionTaken), SCITER_STRING(SpyID));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CallUISetVirusFoundEntryfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************
*  Function Name  : GetString
*  Description    : Function which returns the string from selected language ini file.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
*****************************************************************************************/
CString CWardWizInstallerDlg::GetString(CString csStringID)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetString", 0, 0, true, SECONDLEVEL);
	}

	return szValue;
}

/***************************************************************************
* Function Name  : GetSelectedLanguage
* Description    : Function returns DWORD value
0 - ENGLISH  1 - HINDI  2 - GERMAN 3 - CHINESE 4 - SPANISH 5 - FRENCH
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
****************************************************************************/
DWORD CWardWizInstallerDlg::GetSelectedLanguage()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetSelectedLanguage", 0, 0, true, SECONDLEVEL);
	}

	return GetPrivateProfileInt(L"VBSETTINGS", L"LanguageID", 0, csIniFilePath);
}

/**********************************************************************************************************
*  Function Name  :	ScanFinished
*  Description    :	display status Scan Finished
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::ScanFinished()
{
	try
	{
		CString csCompleteScanning;
		CString csFileScanCount;
		CString csMsgNoFileExist(L"");
		CString csCurrentFileCount;
		CString cstypeofscan = L"";

		OnTimerScan();
		csCurrentFileCount.Format(L"%d", m_FileScanned);
		CallUISetFileCountfunction(L"", csCurrentFileCount);

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
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::ScanFinished", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : CallNotificationMessage
*  Description    : Calls Light box on UI
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
/***************************************************************************************************/
void CWardWizInstallerDlg::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnTimerScan
*  Description    : On timer event for file count.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
/***************************************************************************************************/
void CWardWizInstallerDlg::OnTimerScan()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnTimerScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
****************************************************************************************************/
void CWardWizInstallerDlg::CallUISetStatusfunction(LPTSTR lpszPath)
{
	__try
	{
		CallUISetStatusfunctionSEH(lpszPath);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CallUISetStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallUISetFileCountfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
****************************************************************************************************/
void CWardWizInstallerDlg::CallUISetFileCountfunction(CString csTotalFileCount, CString csCurrentFileCount)
{
	try
	{

		CString csTotalFile = csTotalFileCount;
		CString csCurrentFile = csCurrentFileCount;
		m_svPreInstallScnStatus.call((SCITER_STRING)csTotalFile, (SCITER_STRING)csCurrentFile);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::callUIfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallUISetStatusfunctionSEH
*  Description    : Calls call_function to invoke ant UI function
Note: No need to add exception handling because
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
****************************************************************************************************/
void CWardWizInstallerDlg::CallUISetStatusfunctionSEH(LPTSTR lpszPath)
{
	try
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
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CallUISetStatusfunctionSEH", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : callUISetScanFinishedStatus
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
****************************************************************************************************/
void CWardWizInstallerDlg::CallUISetScanFinishedStatus(CString csData)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CallUISetScanFinishedStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : on_timer
*  Description    : On_timer cals on_timerSEH function
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
/***************************************************************************************************/
bool CWardWizInstallerDlg::on_timer(HELEMENT he)
{
	__try
	{
		return  on_timerSEH(he);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::on_timer", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : on_timerSEH
*  Description    : on_timerSEH for count total file and display file path on dialog box.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
/***************************************************************************************************/
bool CWardWizInstallerDlg::on_timerSEH(HELEMENT he)
{
	CallUISetStatusfunction(m_csCurrentFilePath.GetBuffer());
	CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
	CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
	CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);

	return true;
}

/************************************************************************************************************
*  Function Name  : LoadPSAPILibrary
*  Description    : Load PSAPI.DLL.
For Issue	  : In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
*************************************************************************************************************/
void CWardWizInstallerDlg::LoadPSAPILibrary()
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::LoadPSAPILibrary", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	UnLoadSignatureDatabase
*  Description    :	Function to Unload Signature database.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
**********************************************************************************************************/
DWORD CWardWizInstallerDlg::UnLoadSignatureDatabase()
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.UnLoadSignatureDatabase();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::UnLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***********************************************************************************************
*  Function Name  : On_ClickQuickScanCleanBtn
*  Description    : Cleans detected virus entries
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
***********************************************************************************************/
json::value CWardWizInstallerDlg::On_ClickScanCleanBtn(SCITER_VALUE svArrCleanEntries, SCITER_VALUE svQarantineFlag)
{
	try
	{
		m_svVirusCount = svQarantineFlag;
		OnClickCleanButton(svArrCleanEntries);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_ClickQuickScanCleanBtn", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***********************************************************************************************
*  Function Name  : On_GetRestartPC
*  Description    : When User Click Restart Button then User Pc Restarted.
*  Author Name    : Tejas Shinde
*  Date           : 3rd May 2019
***********************************************************************************************/
json::value CWardWizInstallerDlg::On_GetRestartPC()
{
	try
	{
		CEnumProcess enumproc;
		enumproc.RebootSystem(0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_ClickQuickScanCleanBtn", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_Close
*  Description    : On_Close for close Dialog box.
*  Author Name    : Tejas Shinde
*  Date           : 8 March 2019
/***************************************************************************************************/
json::value CWardWizInstallerDlg::On_ExitSetup()
{
	try
	{
		CDialogEx::OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_ExitSetup", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	ShutDownScanning
*  Description    :	Shut down scanning with terminating all thread safely.
*  Author Name    : Tejas Shinde
*  Date           : 6th May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::OnClickCleanButton(SCITER_VALUE svArrayCleanEntries)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnClickCleanButton", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineThread
*  Description    :	If user clicks on clean button.Quarantine thread gets called.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
DWORD WINAPI QuarantineThread(LPVOID lpParam)
{
	try
	{
		CWardWizInstallerDlg *pThis = (CWardWizInstallerDlg *)lpParam;
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	HandleVirusEntry
*  Description    :	When any entry comes for cleaning, wardwiz scanner take a backup store into quarantine
folder and keep a record into DB file
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
DWORD CWardWizInstallerDlg::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThirdParam, DWORD dwISpyID, CString &csBackupFilePath, DWORD &dwAction)
{
	DWORD dwRet = 0;
	CString csStatus;
	m_csQuarentineEntries.Lock();
	try
	{
		TCHAR szAction[MAX_PATH] = { 0 };
		if ((!szThreatPath) || (!szThreatName) || (!szThirdParam))
		{
			AddLogEntry(L"### CWardwizInstallerDlg::HandleVirusEntry file name not available", 0, 0, true, SECONDLEVEL);
			dwRet = SANITYCHECKFAILED;
			m_csQuarentineEntries.Unlock();
			return dwRet;
		}

		if (!PathFileExists(szThreatPath))
		{
			AddLogEntry(L"### CWardwizInstallerDlg::HandleVirusEntry No file available %s", szThreatPath, 0, true, SECONDLEVEL);

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
				AddLogEntry(L"### CWardwizInstallerDlg::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::HandleVirusEntry", 0, 0, true, SECONDLEVEL);
	}

	m_csQuarentineEntries.Unlock();
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	GetProgramFilePath()
*  Description    :	Get program file path as per OS
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
CString CWardWizInstallerDlg::GetProgramFilePath()
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
				AddLogEntry(L"### CWardwizInstallerDlg::Create Wardwiz directory failed", 0, 0, true, SECONDLEVEL);
			}
		}
		csProgFilePath += L"\\Quarantine";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetProgramFilePath", 0, 0, true, SECONDLEVEL);
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
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus)
{
	try
	{
		CIspyList newEntry(szThreatPath, csDuplicateName, szThreatName, L"", L"", L"", dwShowStatus);
		m_objISpyDBManipulation.InsertEntry(newEntry, RECOVER);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::InsertRecoverEntry", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/*************************************************************************************************
*  Function Name  : CheckForDiscSpaceAvail
*  Description    : to check whether there is enough space to take a backup percentage.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**************************************************************************************************/
DWORD CWardWizInstallerDlg::CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath)
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
			AddLogEntry(L"### CWardwizInstallerDlg::Error in opening existing file %s for finding a size of path file", csThreatPath, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckForDiscSpaceAvail for file %s", csThreatPath, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	BackUpBeforeQuarantineOrRepair
*  Description    :	Taking a backup before taking any action on detected files.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath, LPTSTR lpszBackupFilePath)
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
			AddLogEntry(L"### CWardwizInstallerDlg::BackUpBeforeQuarantineOrRepair Original file not available %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
			return false;
		}

		if (!PathFileExists(csQuarantineFolderpath))
		{
			if (!CreateDirectory(csQuarantineFolderpath, NULL))
			{
				AddLogEntry(L"### CWardwizInstallerDlg::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::BackUpBeforeQuarantineOrRepair, FilePath: %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name		: GetFileHash(
*  Description			: Function to get file hash
*  Function Arguments	: pFilePath (In), pFileHash(out)
*  Author Name			:  Tejas Shinde.
*  Date				    :  13 May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash)
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
			AddLogEntry(L"### Failed in CWardwizInstallerDlg::GetFileHash (%s)", L"VBHASH.DLL");
			return true;
		}

		typedef DWORD(*GETFILEHASH)	(TCHAR *pFilePath, TCHAR *pFileHash);
		GETFILEHASH fpGetFileHash = (GETFILEHASH)GetProcAddress(hHashDLL, "GetFileHash");
		if (!fpGetFileHash)
		{
			AddLogEntry(L"### Failed in CWardwizInstallerDlg::GetFileHash Address(%s)", L"GetFileHash");
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetFileHash, FilePath: %s", pFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}

	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineEntry
*  Description    :	if ISPYID =0 , wardwiz scanner delete that file
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::QuarantineEntry(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath)
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
			WriteFileForBootScan(csQurFilePaths, csVirusName);
			bReturn = false; //quarantine is failed but we keep the record in the ini.
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::QuarantineFile", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	WriteFileForBootScan
*  Description    :	Write file for Entry of threat which can not be quarantined.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::WriteFileForBootScan(CString csQurFilePaths, CString csVirusName)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	LoadRecoversDBFile
*  Description    :	Load all entries of recover files.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
void CWardWizInstallerDlg::LoadRecoversDBFile()
{
	CString csDirPath;
	try
	{
		csDirPath = CString(GetProgramFilePath());
		m_objISpyDBManipulation.LoadEntries(0x00, csDirPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::LoadRecoversDBFile", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_RestartForBootScan
*  Description    : Machine gets restart for boot scan.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_RestartForBootScan(SCITER_VALUE svBootFlag)
{
	DWORD dwBootFlag = 0x00;
	CString csQuarantinePath;
	try
	{
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_RestartForBootScan", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	SaveInRecoverDB
*  Description    :	Save all entry into recover files.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::SaveInRecoverDB()
{

	bool bReturn = false;
	try
	{
		CString csProgramPath = GetProgramFilePath();
		bReturn = m_objISpyDBManipulation.SaveEntries(0x00, csProgramPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SaveInRecoverDB", 0, 0, true, SECONDLEVEL);
	}
	return  bReturn;
}

/**********************************************************************************************************
*  Function Name		: CheckIFAlreadyBackupTaken
*  Description			: Function to check whether backup already taken (or) not.
*  Function Arguments	: szFileHash
*  Author Name			:  Tejas Shinde.
*  Date					:  13 May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::CheckIFAlreadyBackupTaken(LPCTSTR szFileHash, LPTSTR szBackupPath)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckIFAlreadyBackupTaken, HASH: %s, BackupPath:%s ", szFileHash, szBackupPath, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: CheckEntryPresent
*  Description			: Function to check entry present in list
*  Function Arguments	: szFileHash
*  Author Name			:  Tejas Shinde.
*  Date					:  13 May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::CheckEntryPresent(LPCTSTR szFileHash, LPTSTR szBackupPath)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckIFAlreadyBackupTaken, HASH: %s, BackupPath:%s ", szFileHash, szBackupPath, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	Encrypt_File
*  Description    :	Encrypt file and keep into quarantine folder as temp file.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
DWORD CWardWizInstallerDlg::Encrypt_File(TCHAR *szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus)
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
			AddLogEntry(L"### CWardwizInstallerDlg::Error in opening existing file %s", szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		if (!dwFileSize)
		{
			AddLogEntry(L"### CWardwizInstallerDlg::Error in GetFileSize of file %s", szFilePath, 0, true, SECONDLEVEL);
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
			AddLogEntry(L"### CWardwizInstallerDlg::Error in allocating memory", 0, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}

		memset(lpFileData, 0x00, dwFileSize);
		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
		ReadFile(hFile, lpFileData, dwFileSize, &dwBytesRead, NULL);
		if (dwFileSize != dwBytesRead)
		{
			AddLogEntry(L"### CWardwizInstallerDlg::Error in ReadFile of file %s", szFilePath, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!CreateRandomKeyFromFile(hFile, dwFileSize))
		{
			AddLogEntry(L"### CWardwizInstallerDlg : Error in CreateRandomKeyFromFile", 0, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x08;
			goto Cleanup;
		}
		CloseHandle(hFile);

		if (DecryptData((LPBYTE)lpFileData, dwBytesRead))
		{
			AddLogEntry(L"### CWardwizInstallerDlg::Error in DecryptData", 0, 0, true, SECONDLEVEL);
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
			AddLogEntry(L"### CWardwizInstallerDlg::Error in creating file %s", lpszTargetFilePath, 0, true, SECONDLEVEL);
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

/**********************************************************************************************************
*  Function Name  :	CreateRandomKeyFromFile
*  Description    :	Create a random key to insert into encrypted file.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize)
{
	try
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
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CreateRandomKeyFromFile", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/**********************************************************************************************************
*  Function Name  :	DecryptData
*  Description    :	Encrypt/Decrypt data.
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
**********************************************************************************************************/
DWORD CWardWizInstallerDlg::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::DecryptData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : Get_String
*  Description    : Function to send string from String Table to front end
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::Get_String(SCITER_VALUE svStringValue)
{
	CString csMessage;
	try
	{
		UINT temp = (UINT)svStringValue.d;
		csMessage = theApp.GetString(temp);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::Get_String", 0, 0, true, SECONDLEVEL);
	}

	return (SCITER_STRING)csMessage;
}

/***************************************************************************************************
*  Function Name  : GetRegLocPath
*  Description    : This function is used to send Register Locations DB file path to UI
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
****************************************************************************************************/
CString CWardWizInstallerDlg::GetRegLocDBPath(LPTSTR szRegUserLocDetails)
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		try
		{
			if (!szRegUserLocDetails)
			{
				return L"";
			}

			//TCHAR *szTemp = _tcsrchr(szRegUserLocDetails, L'\\');
			//szTemp[0] = '\0';
			m_cvRegLocDB.call((sciter::string)szRegUserLocDetails);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetModuleFileStringPath", 0, 0, true, SECONDLEVEL);
		}
		//return(CString(szRegUserLocDetails));

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetModuleFileStringPath");
	}
	return L"";
}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : This function is used to send DB file path to UI
*  Author Name    :  Tejas Shinde.
*  Date           :  13 May 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		CString csModulePath = theApp.GetModuleFilePath();
		swprintf_s(szActualIPath, L"%s\\%s", csModulePath, L"RLOC.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : On_ClickTrialProduct
*  Description    : Trial activation of product
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_ClickTrialProduct(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetRegStatusCB, SCITER_VALUE svFunSetNoInternetMsg, SCITER_VALUE svResponse)
{
	try
	{
		svArrUserDetails.isolate();
		bool bIsArray = false;
		bIsArray = svArrUserDetails.is_array();

		if (!bIsArray)
		{
			return false;
		}
		m_bActiveProduct = false;
		m_bTryProduct = true;
		m_bOnlineActivation = false;
		m_bOfflineActivation = false;
		m_svFunSetRegStatusCB = svFunSetRegStatusCB;
		m_svFunSetNoInternetMsg = svFunSetNoInternetMsg;
		m_csResponseData = svResponse.get(L"").c_str();
		PerformRegistration(svArrUserDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_ClickTrialProduct");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ClickActivateProduct
*  Description    : activation of product using product activation key
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_ClickActivateProduct(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetRegStatusCB, SCITER_VALUE svFunSetNoInternetMsg, SCITER_VALUE svResponse)
{
	try
	{
		svArrUserDetails.isolate();
		bool bIsArray = false;
		bIsArray = svArrUserDetails.is_array();

		if (!bIsArray)
		{
			return false;
		}
		m_bOnlineActivation = true;
		m_bOfflineActivation = false;
		m_bTryProduct = false;
		m_bActiveProduct = true;
		m_svFunSetRegStatusCB = svFunSetRegStatusCB;
		m_svFunSetNoInternetMsg = svFunSetNoInternetMsg;
		m_csResponseData = svResponse.get(L"").c_str();
		PerformRegistration(svArrUserDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_ClickActivateProduct");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetOfflineActivationKey
*  Description    : Creates offline key for activaion and sends it to UI
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CWardWizInstallerDlg::GetOfflineActivationKey(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetOfflineActKeyCB, SCITER_VALUE svFunSetNoInternetMsg)
{
	try
	{
		svArrUserDetails.isolate();
		bool bIsArray = false;
		bIsArray = svArrUserDetails.is_array();

		if (!bIsArray)
		{
			return false;
		}
		m_bOnlineActivation = false;
		m_bOfflineActivation = true;
		m_bTryProduct = false;
		m_bActiveProduct = true;
		m_svFunSetOfflineActKeyCB = svFunSetOfflineActKeyCB;
		m_svFunSetNoInternetMsg = svFunSetNoInternetMsg;
		PerformRegistration(svArrUserDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetOfflineActivationKey");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ClickOfflineRegistration
*  Description    : Registeres product using offline activation
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_ClickOfflineRegistration(SCITER_VALUE svOfflineActKey, SCITER_VALUE svFunSetRegStatusCB)
{
	try
	{
		m_svFunSetRegStatusCB = svFunSetRegStatusCB;

		const wstring chOfflineActKey = svOfflineActKey.get(L"");
		memset(&m_szOfflineActivationCode, 0x00, sizeof(m_szOfflineActivationCode));
		memcpy(m_szOfflineActivationCode, chOfflineActKey.c_str(), 0x16 * sizeof(TCHAR));
		SetEvent(m_hOfflineNextEvent);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_ClickOfflineRegistration");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : PerformRegistration
*  Description    : Performs registration for different activation modes
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
void CWardWizInstallerDlg::PerformRegistration(SCITER_VALUE svArrUserDetails)
{
	try
	{
		CString csChkWindowText;
		const SCITER_VALUE EachEntry = svArrUserDetails[0];

		const std::wstring Salutation = EachEntry[L"Salutation"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szTitle, Salutation.c_str());

		const std::wstring FirstName = EachEntry[L"FirstName"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szUserFirstName, FirstName.c_str());

		const std::wstring LastName = EachEntry[L"LastName"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szUserLastName, LastName.c_str());

		const std::wstring PhoneNumber = EachEntry[L"PhoneNumber"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szMobileNo, PhoneNumber.c_str());

		const std::wstring EmailID = EachEntry[L"EmailID"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szEmailID, EmailID.c_str());

		const std::wstring DealerCode = EachEntry[L"DealerCode"].get(L"");
		wcscpy(m_szDealerCode, DealerCode.c_str());

		const std::wstring ReferenceID = EachEntry[L"ReferenceID"].get(L"");
		wcscpy(m_szReferenceID, ReferenceID.c_str());

		const std::wstring Country = EachEntry[L"Country"].get(L"");
		wcscpy(m_szCountry, Country.c_str());

		const std::wstring State = EachEntry[L"State"].get(L"");
		wcscpy(m_szState, State.c_str());

		const std::wstring City = EachEntry[L"City"].get(L"");
		wcscpy(m_szCity, City.c_str());

		const std::wstring PinCode = EachEntry[L"PinCode"].get(L"");
		wcscpy(m_szPinCode, PinCode.c_str());

		const std::wstring EngineerName = EachEntry[L"EngineerName"].get(L"");
		wcscpy(m_szEngineerName, EngineerName.c_str());

		const std::wstring EngineerMobNo = EachEntry[L"EngineerMobNo"].get(L"");
		wcscpy(m_szEngineerMobNo, EngineerMobNo.c_str());

		_tcscpy_s(m_RegDlg_ActInfo.szClientID, 0x80, m_szMachineId);

		if (m_bActiveProduct == true)
		{
			m_hOfflineNextEvent = NULL;
			m_hOfflineNextEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			const std::wstring ProdID = EachEntry[L"ProductID"].get(L"");
			memcpy(&m_RegDlg_ActInfo.szKey, ProdID.c_str(), sizeof(m_RegDlg_ActInfo.szKey));
		}

		if (m_bTryProduct == true)
		{
			memcpy(&m_RegDlg_ActInfo.szKey, L"", sizeof(m_RegDlg_ActInfo.szKey));
		}

		TCHAR	szInstallCode[32]	= { 0 };

		if (m_bOfflineActivation == true && m_bActiveProduct == true)
		{
			if (!m_szMachineId[0])
			{
				DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
				return;
			}
			if (!LoadWrdWizOfflineRegDll())
			{
				DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
				AddLogEntry(L"### Failed to locate VBOFFLINEREG.DLL", 0, 0, true, SECONDLEVEL);
				return;
			}
			if (g_GetInstallationCode)
				g_GetInstallationCode(m_RegDlg_ActInfo.szKey, m_szMachineId, szInstallCode);

			if (!szInstallCode[0])
			{
				DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
				AddLogEntry(L"### Generation of installation code is failed", 0, 0, true, SECONDLEVEL);
				return;
			}

			if ((_tcslen(szInstallCode) < 16) && (_tcslen(szInstallCode) > 28))
			{
				DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
				AddLogEntry(L"### Generation of invalid installation code", 0, 0, true, SECONDLEVEL);
				return;
			}

			if (m_dwProductId == 1)
			{
				_tcscpy(m_szInstallCode, L"1");
			}
			else if (m_dwProductId == 2)
			{
				_tcscpy(m_szInstallCode, L"2");
			}
			else if (m_dwProductId == 3)
			{
				_tcscpy(m_szInstallCode, L"3");
			}
			else if (m_dwProductId == 4)
			{
				_tcscpy(m_szInstallCode, L"4");
			}
			else if (m_dwProductId == 5)
			{
				_tcscpy(m_szInstallCode, L"5");
			}
			_tcscat(m_szInstallCode, szInstallCode);
			m_svFunSetOfflineActKeyCB.call(m_szInstallCode);

		}
		else if ((m_bOnlineActivation == true && m_bActiveProduct == true) || m_bTryProduct == true)
		{
			wcscpy_s(m_RegDlg_ActInfo.szRegionCode, 64, L"");
			Invalidate();
		}

		DWORD	dwSendRequstThreadID = 0x00;

		dwDaysRemain = 0x00;
		m_hSendRequestThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendRequestThread, this, 0, &dwSendRequstThreadID);
		Sleep(1000);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::PerformRegistration");
	}
}

/**********************************************************************************************************
* Function Name      : LoadWrdWizOfflineRegDll()
* Description        : Loading WrdWizOfflineReg Dll from installation folder . and getprocaddr of some function.
* Author Name		 : Neha Gharge
* Date Of Creation   : 10th Oct 2014
* SR_No              :
***********************************************************************************************************/
bool CWardWizInstallerDlg::LoadWrdWizOfflineRegDll()
{
	try
	{
		CString csDllPath = L"";
		csDllPath.Format(L"%s\\VBOFFLINEREG.DLL", theApp.GetModuleFilePath());

		if (!PathFileExists(csDllPath))
		{
			return false;
		}

		if (!m_hOfflineDLL)
		{
			m_hOfflineDLL = LoadLibrary(csDllPath);
			if (!m_hOfflineDLL)
			{
				AddLogEntry(L"Locating error in VBOFFLINEREG.DLL", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		g_GetInstallationCode = (GETINSTALLATIONCODE)GetProcAddress(m_hOfflineDLL, "GetInstallationCode");
		if (!g_GetInstallationCode)
			return false;

		g_ValidateResponse = (VALIDATERESPONSE)GetProcAddress(m_hOfflineDLL, "ValidateResponse");
		if (!g_ValidateResponse)
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::LoadVibraniumOfflineRegDll");
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SendRequestThread
*  Description    : thread function to perform registration
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
DWORD WINAPI SendRequestThread(LPVOID lpThreadParam)
{
	CWardWizInstallerDlg	*pRegDlg = (CWardWizInstallerDlg *)lpThreadParam;
	CString		csIniFilePath;
	if (!pRegDlg)
		return 1;

	DWORD	dwRet = 0x00;

	if (pRegDlg->GetRegisteredUserInfo(pRegDlg->m_dwProductId) || pRegDlg->m_bActiveProduct)
	{
		SYSTEMTIME		CurrTime = { 0 };

		WORD		wDaysLeft = 0x00;

		memset(&pRegDlg->m_ActInfo, 0x00, sizeof(pRegDlg->m_ActInfo));
		memcpy(&pRegDlg->m_ActInfo, &m_RegDlg_ActInfo, sizeof(m_RegDlg_ActInfo));
		GetSystemTime(&CurrTime);
		pRegDlg->m_ActInfo.RegTime = CurrTime;

		DWORD				dwDaysLeft = 0x00;
		AVACTIVATIONINFO	ActInfo = { 0 };
		memcpy(&ActInfo, &pRegDlg->m_ActInfo, sizeof(ActInfo));

		//Write here Product Number
		pRegDlg->m_ActInfo.dwProductNo = pRegDlg->m_dwProductId;
		ActInfo.dwProductNo = pRegDlg->m_dwProductId;

		CString csProductID;
		csProductID.Format(L">>> Product ID: %d", pRegDlg->m_dwProductId);
		AddLogEntry(csProductID);

		//Send data to server & Get Server response
		if (pRegDlg->m_bOnlineActivation == true || pRegDlg->m_bTryProduct == true)
		{
			//Send data to server & Get Server response
			DWORD dwRes = pRegDlg->GetRegistrationDateFromServer(&CurrTime, &dwDaysLeft);
			if (dwRes > 0)
			{
				if (dwRes == PRODUCTEXPIRED)
				{
					pRegDlg->m_ActInfo.dwTotalDays = dwDaysLeft;
					pRegDlg->AddProdRegInfoToLocal(ActInfo, sizeof(ActInfo));
				}

				pRegDlg->DisplayFailureMessage((REGFAILUREMSGS)dwRes);
				goto FAILED;
			}

			////Ram, Resolved Issue No: 0001075
			pRegDlg->m_ActInfo.RegTimeServer = CurrTime;
			ActInfo.RegTimeServer = CurrTime;
			ActInfo.dwTotalDays = dwDaysLeft;

			//Local time gives your local time rather than UTC.
			GetSystemTime(&CurrTime);
			pRegDlg->m_ActInfo.RegTime = CurrTime;
			ActInfo.RegTime = CurrTime;
			pRegDlg->m_ActInfo.dwTotalDays = dwDaysLeft;
		}

		if (pRegDlg->m_bOfflineActivation == true)
		{
			TCHAR szTempActivationCode[32] = { 0 };
			//code for offline registration.
			WaitForSingleObject(pRegDlg->m_hOfflineNextEvent, INFINITE);

			if (g_ValidateResponse)
			{
				DWORD dwValidResponse = g_ValidateResponse(pRegDlg->m_szOfflineActivationCode, (BYTE)pRegDlg->m_dwProductId, CurrTime, wDaysLeft);
				if (dwValidResponse)
				{
					pRegDlg->DisplayFailureMessageForOfflineRegistration((REGFAILUREMSGS)dwValidResponse);
					goto FAILED;
				}
			}

			dwDaysLeft = static_cast<DWORD>(wDaysLeft);

			ActInfo.RegTime = pRegDlg->m_ActInfo.RegTime = CurrTime;
			ActInfo.dwTotalDays = pRegDlg->m_ActInfo.dwTotalDays = dwDaysLeft;
		}

		ResetEvent(pRegDlg->m_hOfflineNextEvent);

		memcpy(&g_ActInfo, &ActInfo, sizeof(ActInfo));

		if (!g_regDataOperation.InsertDataIntoDLL((LPBYTE)&ActInfo, sizeof(ActInfo), IDR_REGDATA, TEXT("REGDATA")))
		{
			AddLogEntry(L"### Failed to InsertDataIntoDLL in CWardwizInstallerDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}

		Sleep(10);

		if (pRegDlg->AddRegistrationDataInRegistry())
		{
			AddLogEntry(L"### AddRegistrationDataInRegistry in CWardwizInstallerDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}

		Sleep(10);

		if (!g_regDataOperation.AddRegistrationDataInFile((LPBYTE)&ActInfo, sizeof(ActInfo)))
		{
			AddLogEntry(L"### AddRegistrationDataInFile failed in CWardwizInstallerDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}

		Sleep(30);

		pRegDlg->SpreadRegistrationFilesInSystem();
	}
	else
	{

		memcpy(&g_ActInfo, &pRegDlg->m_ActInfo, sizeof(g_ActInfo));
		SYSTEMTIME		CurrTime = { 0 };
		GetSystemTime(&CurrTime);
		CTime	Time_Reg(pRegDlg->m_ActInfo.RegTime);
		CTime	Time_Curr(CurrTime);

		DWORD	dwDays = 0x00;

		if (Time_Curr > Time_Reg)
		{
			CTimeSpan	Time_Diff = Time_Curr - Time_Reg;
			if (Time_Diff.GetDays() < pRegDlg->m_ActInfo.dwTotalDays)
				dwDays = pRegDlg->m_ActInfo.dwTotalDays - static_cast<DWORD>(Time_Diff.GetDays());
		}

		if (!dwDays)
		{
			CString csMessage;
			//theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_STATUS"));
			csMessage.Format(L"\t%s \n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL1"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL2"));
			pRegDlg->DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, csMessage);
			goto FAILED;
		}
	}

	pRegDlg->SpreadRegistrationFilesInSystem();

	//check if not offline mode
	if (!pRegDlg->m_bOfflineActivation)
	{
		if (_tcslen(pRegDlg->m_szDealerCode) != 0)
		{
			csIniFilePath = theApp.GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"DealerCode", pRegDlg->m_szDealerCode, csIniFilePath);
		}

		if (_tcslen(pRegDlg->m_szReferenceID) != 0)
		{
			csIniFilePath = theApp.GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"ReferenceID", pRegDlg->m_szReferenceID, csIniFilePath);
		}

		if (_tcslen(pRegDlg->m_szEngineerName) != 0)
		{
			csIniFilePath = theApp.GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"EngineerName", pRegDlg->m_szEngineerName, csIniFilePath);
		}

		if (_tcslen(pRegDlg->m_szEngineerMobNo) != 0)
		{
			csIniFilePath = theApp.GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"EngineerMobNo", pRegDlg->m_szEngineerMobNo, csIniFilePath);
		}
	}

	if (_tcslen(pRegDlg->m_szCountry) != 0)
	{
		csIniFilePath = theApp.GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"Country", pRegDlg->m_szCountry, csIniFilePath);
	}

	if (_tcslen(pRegDlg->m_szState) != 0)
	{
		csIniFilePath = theApp.GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"State", pRegDlg->m_szState, csIniFilePath);
	}

	if (_tcslen(pRegDlg->m_szCity) != 0)
	{
		csIniFilePath = theApp.GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"City", pRegDlg->m_szCity, csIniFilePath);
	}

	if (_tcslen(pRegDlg->m_szPinCode) != 0)
	{
		csIniFilePath = theApp.GetModuleFilePath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"PinCode", pRegDlg->m_szPinCode, csIniFilePath);
	}

	if (pRegDlg->m_bIsProxySet || pRegDlg->m_bOfflineActivation)
	{
		pRegDlg->DisplayFailureMsgOnUI(ERROR_SUCCESS_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_STATIC_SUCCESS"));
	}

	return dwRet;
FAILED:
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetRegisteredUserInfo
*  Description    : Get registered user information
*  Author Name    : Akshay Patil
*  Date			  : 20 Jun 2019
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetRegisteredUserInfo(DWORD dwProdID)
{

	DWORD	dwRet = 0x00;
	DWORD	dwSize = 0x00;
	DWORD	dwRegUserSize = 0x00;
	try
	{
		dwDaysRemain = 0x00;

		m_RegisterationDLL = NULL;

		CString	strEvalRegDLL("");
		CString	strRegisterDLL("");

		CString	strAppPath = theApp.GetModuleFilePath();
		dwRet = GetRegistrationDataFromRegistry();
		if (!dwRet)
		{
			if (!CheckForMachineID(m_ActInfo))
			{
				memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
				dwRet = 0x06;
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
				dwRet = 0x06;
			}
			goto Cleanup;
		}

		strEvalRegDLL.Format(TEXT("%s\\VBEVALREG.DLL"), strAppPath);
		if (!PathFileExists(strEvalRegDLL))
		{
			dwRet = 0x01;
			AddLogEntry(L"### VBEVALREG.DLL not found in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		strRegisterDLL.Format(TEXT("%s\\VBREGISTERDATA.DLL"), strAppPath);
		if (!PathFileExists(strRegisterDLL))
		{
			dwRet = 0x02;
			AddLogEntry(L"### VBREGISTERDATA.DLL not found in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		m_RegisterationDLL = LoadLibrary(strRegisterDLL);
		GetRegistrationData = (GETREGISTRATIONDATA)GetProcAddress(m_RegisterationDLL, "GetRegisteredData");

		if (!GetRegistrationData)
		{
			dwRet = 0x04;
			AddLogEntry(L"### VBREGISTERDATA.DLL version is incorrect in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		dwSize = sizeof(m_ActInfo);
		dwRegUserSize = 0x00;

		if (GetRegistrationData((LPBYTE)&m_ActInfo, dwRegUserSize, IDR_REGDATA, L"REGDATA") == 0)
		{
			if (!CheckForMachineID(m_ActInfo))
			{
				memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
				dwRet = 0x06;
				goto Cleanup;
			}
			dwRet = 0x00;
			goto Cleanup;
		}

		if (dwSize != dwRegUserSize)
			dwRet = 0x05;

		//Match here the machine ID with Stored machineID
		if (m_dwProductId != m_ActInfo.dwProductNo)
		{
			AddLogEntry(L"### Product ID not matched returning", 0, 0, true, ZEROLEVEL);
			dwRet = 0x05;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			AddLogEntry(L"### Machine ID not matched returning", 0, 0, true, SECONDLEVEL);
			dwRet = 0x06;
			goto Cleanup;
		}

	Cleanup:

		if (!dwRet)
		{
			memcpy(&g_ActInfo, &m_ActInfo, sizeof(m_ActInfo));
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetRegisteredUserInfo");
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDataFromRegistry
*  Description    : function to get registration detaios from registry
*  Author Name    : Akshay Patil
*  Date			  : 20 Jun 2019
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetRegistrationDataFromRegistry()
{
	DWORD	dwRet = 0x00;
	try
	{

		DWORD	dwRegType = 0x00, dwRetSize = 0x00;

		HKEY	h_iSpyAV = NULL;
		HKEY	h_iSpyAV_User = NULL;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h_iSpyAV) != ERROR_SUCCESS)
		{
			dwRet = 0x01;
			goto Cleanup;
		}
		dwRetSize = sizeof(m_ActInfo);
		if (RegQueryValueEx(h_iSpyAV, TEXT("VibraniumUserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo,
			&dwRetSize) != ERROR_SUCCESS)
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		if (DecryptDataReg((LPBYTE)&m_ActInfo, sizeof(m_ActInfo)))
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		if (m_dwProductId != m_ActInfo.dwProductNo)
		{
			AddLogEntry(L"### Product ID not matched returning", 0, 0, true, ZEROLEVEL);
			dwRet = 0x05;
			return dwRet;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			AddLogEntry(L"### Machine ID not matched returning", 0, 0, true, SECONDLEVEL);
			dwRet = 0x06;
			return dwRet;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetRegistrationDataFromRegistry");
	}
Cleanup:
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDatafromFile
*  Description    : function to get Registration details from file
*  Author Name    : Akshay Patil
*  Date			  : 20 Jun 2019
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetRegistrationDatafromFile()
{
	DWORD	dwRet = 0x01;
	try
	{
		CString	strUserRegFile = GetWardWizPathFromRegistry();
		strUserRegFile = strUserRegFile + L"VBUSERREG.DB";
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetRegistrationDatafromFile");
	}
	return 0x01;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDatafromFile
*  Description    : function to read registartion data from db
*  Author Name    : Akshay Patil
*  Date			  : 20 Jun 2019
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetRegistrationDatafromFile(CString strUserRegFile)
{
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	DWORD	dwRet = 0x00, dwBytesRead = 0x00;
	try
	{
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

		if (DecryptDataReg((LPBYTE)&m_ActInfo, sizeof(m_ActInfo)))
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		if (m_dwProductId != m_ActInfo.dwProductNo)
		{
			AddLogEntry(L"### Product ID not matched returning", 0, 0, true, ZEROLEVEL);
			dwRet = 0x05;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			AddLogEntry(L"### Machine ID not matched returning", 0, 0, true, SECONDLEVEL);
			dwRet = 0x06;
			goto Cleanup;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetRegistrationDatafromFile");
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
*  Function Name  : CheckForMachineID
*  Description    : function which compare with Machine ID present in DB files & machine ID in registry.
*  Author Name    : Ram
*  SR_NO		  :
*  Date           : 18 Sep 2013
****************************************************************************************************/
bool CWardWizInstallerDlg::CheckForMachineID(const AVACTIVATIONINFO	&actInfo)
{
	try
	{
		//Need to compare the string fron 2nd character, as we are storing first character with string length.
		//also need to compare with machine ID added with more charaters in the registry, example like
		//we firstly installed 1.8 setup in that MAC address not present, then we added MAC Address in further
		//releases then Machine ID string will get increased in registry but not in DB files.
		//In this case we need to compare with DB file machine ID with new generated Machine ID with same legnth 
		//of Machine id which is present in DB files.
		if (memcmp(&actInfo.szClientID[1], &m_szMachineId[1], wcslen(actInfo.szClientID) * sizeof(TCHAR)) != 0)
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckForMachineID");
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : GetRegistrationDateFromServer
*  Description    : function to extract date, time and days from repsonse
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetRegistrationDateFromServer(LPSYSTEMTIME lpServerTime, LPDWORD lpDaysRemaining)
{
	DWORD	dwRet = 0x01;
	try
	{
		if (m_bIsProxySet)
		{
			TCHAR	szUserInfo[512] = { 0 };
			TCHAR	szKey[0x32] = { 0 };

			if (!m_bActiveProduct)
			{
				wsprintf(szUserInfo, L"http://www.wardwiz.com/new_reg/get/trial.php?FName=%s&LName=%s&Email=%s&ContactNo=%s&ProdID=%d&MachineID=%s&OSName=%s&RamSize=%s&HDSize=%s&Processor=%s&CompName=%s&UserName=%s", m_ActInfo.szUserFirstName,
					m_ActInfo.szUserLastName, m_ActInfo.szEmailID, m_ActInfo.szMobileNo, m_ActInfo.dwProductNo, m_ActInfo.szClientID, m_objSysdetails.szOsName, m_objSysdetails.szRAM, m_objSysdetails.szHardDiskSize, m_objSysdetails.szProcessor, m_objSysdetails.szCompName, m_objSysdetails.szLoggedInUser);
			}
			else
			{
				memcpy(&szKey, m_ActInfo.szKey, sizeof(m_ActInfo.szKey));
				szKey[wcslen(szKey) + 1] = '\0';

				//If m_bIsOffline = true, then url is changed for german setup
				if (!m_bIsOffline)
				{					
					wsprintf(szUserInfo, L"http://www.wardwiz.com/new_reg/get/registration.php?FName=%s&LName=%s&Email=%s&RegCode=%s&ContactNo=%s&DealerCodeID=%s&ReferenceID=%s&ProdID=%d&MachineID=%s&Country=%s&State=%s&City=%s&PinCode=%s&EngineerName=%s&EngineerMobNo=%s&OSName=%s&RamSize=%s&HDSize=%s&Processor=%s&CompName=%s&UserName=%s", m_ActInfo.szUserFirstName,
						m_ActInfo.szUserLastName, m_ActInfo.szEmailID, szKey, m_ActInfo.szMobileNo, m_szDealerCode, m_szReferenceID, m_ActInfo.dwProductNo, m_ActInfo.szClientID, m_szCountry, m_szState, m_szCity, m_szPinCode, m_szEngineerName, m_szEngineerMobNo, m_objSysdetails.szOsName, m_objSysdetails.szRAM, m_objSysdetails.szHardDiskSize, m_objSysdetails.szProcessor, m_objSysdetails.szCompName, m_objSysdetails.szLoggedInUser);					
				}
				else
				{
					wsprintf(szUserInfo, L"http://www.wardwiz.com/new_reg/get/de/registration.php?FName=%s&LName=%s&Email=%s&RegCode=%s&ContactNo=%s&DealerCodeID=%s&ReferenceID=%s&ProdID=%d&MachineID=%s&Country=%s&State=%s&City=%s&PinCode=%s&EngineerName=%s&EngineerMobNo=%s&OSName=%s&RamSize=%s&HDSize=%s&Processor=%s&CompName=%s&UserName=%s&Lang=%d", m_ActInfo.szUserFirstName,
						m_ActInfo.szUserLastName, m_ActInfo.szEmailID, szKey, m_ActInfo.szMobileNo, m_szDealerCode, m_szReferenceID, m_ActInfo.dwProductNo, m_ActInfo.szClientID, m_szCountry, m_szState, m_szCity, m_szPinCode, m_szEngineerName, m_szEngineerMobNo, m_objSysdetails.szOsName, m_objSysdetails.szRAM, m_objSysdetails.szHardDiskSize, m_objSysdetails.szProcessor, m_objSysdetails.szCompName, m_objSysdetails.szLoggedInUser, theApp.m_dwLangID);					
				}
			}

			AddLogEntry(L">>>> Sending HTTP request", 0, 0, true, FIRSTLEVEL);
			AddLogEntry(szUserInfo);

			WinHttpClient client(szUserInfo);
			client.SetProxy(m_szServer);
			client.SetProxyUsername(m_szUsername);
			client.SetProxyPassword(m_szPassword);
			
			//while registration after entering all the required details
			//click Next->popup appears if internet connection is  not there as 'no internet connection
			//click 'OK' on that dialog box->there it should show Retry button
			//so that we not need enter all the details again
			//Neha Gharge 10th Aug ,2015
			if (!client.SendHttpRequest())
			{
				while (true)
				{
					SCITER_VALUE sv_GetRetryCancelVal;
					m_svFunSetNoInternetMsg.call();
					::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
					theApp.m_objCompleteEvent.ResetEvent();
					if (theApp.m_bRetval == true)
					{
						sv_GetRetryCancelVal = 0;
					}
					else
					{
						sv_GetRetryCancelVal = 1;
					}
					if (sv_GetRetryCancelVal == 0)
					{
						if (client.SendHttpRequest())
						{
							break;
						}
					}
					else
					{
						DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_STATIC_FAIL"));
						return dwRet;
					}
				}
			}

			// The response content.
			wstring httpResponseContent = client.GetResponseContent();

			AddLogEntry(L">>>> Getting response", 0, 0, true, FIRSTLEVEL);
			AddLogEntry(httpResponseContent.c_str());

			if (m_csResponseData.GetLength() > 0)
				m_csResponseData.Empty();

			m_csResponseData = httpResponseContent.c_str();
		}

		if ((m_csResponseData.GetLength() > 22) && (m_csResponseData.GetLength() < 512))
		{
			dwRet = ExtractDate(m_csResponseData.GetBuffer(), lpServerTime, lpDaysRemaining);
		}
		else
			dwRet = NULLRESPONSE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetRegistrationDateFromServer");
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : ExtractDate
*  Description    : Extracts date and time form response
*  Author Name    : Akshay Patil
*  Date			  : 17 Jun 2019
****************************************************************************************************/
DWORD CWardWizInstallerDlg::ExtractDate(TCHAR *pTime, LPSYSTEMTIME lpServerTime, LPDWORD lpdwServerDays)
{
	__try
	{
		SYSTEMTIME	ServerTime = { 0 };
		TCHAR	szTemp[8] = { 0 };
		TCHAR	*chSep = L"#";
		TCHAR	pszTime[64] = { 0 };

		//check here desired lenth else return with failed.
		if (wcslen(pTime) > 0x1E)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x01;
		}

		TCHAR	*pDateTime = wcstok(pTime, chSep);
		if (!pDateTime)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x01;
		}

		TCHAR	*pDays = wcstok(NULL, chSep);
		TCHAR	*pResponseCode = wcstok(NULL, chSep);

		if (!pDays)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x02;
		}

		if (!pResponseCode && wcslen(pResponseCode) != 3)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x03;
		}

		DWORD	dwDays = 0x00;
		DWORD	dwResponseCode = 0x00;

		if (wcslen(pDateTime) > 0x3F)
		{
			AddLogEntry(L"### Invalid Response code from server Time ", 0, 0, true, SECONDLEVEL);
			return 0x14;
		}

		wcscpy(pszTime, pDateTime);

		//Checking for if no of days is -ve.
		if (pDays[0] == '-')
		{
			AddLogEntry(L"### No of days is in -ve returning from CRegistrationSecondDlg::ExtractDate ", 0, 0, true, SECONDLEVEL);
			return 0x02;
		}

		swscanf(pResponseCode, L"%d", &dwResponseCode);

		if (dwResponseCode == 0x01)
		{
			AddLogEntry(L"### Registration key already been used", 0, 0, true, SECONDLEVEL);
			return MACHINEIDMISMATCH;
		}

		if (dwResponseCode == 0x02)
		{
			AddLogEntry(L"### Email ID for Registration is invalid", 0, 0, true, SECONDLEVEL);
			return INVALIDEMAILID;
		}

		if (dwResponseCode == 0x03)
		{
			AddLogEntry(L"### Country code invalid", 0, 0, true, SECONDLEVEL);
			return COUNTRYCODEINVALID;
		}

		if (dwResponseCode == 0x04)
		{
			AddLogEntry(L"### Invalid Registration Number", 0, 0, true, SECONDLEVEL);
			return INVALIDREGNUMBER;
		}

		if (dwResponseCode == 0x05)
		{
			AddLogEntry(L"### Invalid Wardwiz Product key", 0, 0, true, SECONDLEVEL);
			return INVALIDPRODVERSION;
		}

		//006 - Invalid Agent, Means the request is not came from Wardwiz client.
		//007 - Database connectivity fails.
		if (dwResponseCode == 0x08)
		{
			AddLogEntry(L"### Failed to update the user information on server, need to resend", 0, 0, true, SECONDLEVEL);
			return USERINFOUPDATEFAILD;
		}

		swscanf(pDays, L"%d", &dwDays);
		if (!dwDays)
		{
			AddLogEntry(L"### Product Expired, Number of days left %s", pDays, 0, true, SECONDLEVEL);
			return PRODUCTEXPIRED;
		}
		else
		{
			*lpdwServerDays = dwDays;
		}

		memcpy(szTemp, pszTime, 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wDay);
		if (!ServerTime.wDay)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x12;
		}

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[3], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wMonth);
		if (!ServerTime.wMonth)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x13;
		}

		DWORD	dwYear = 0x00;

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[6], 4 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &dwYear);
		if (!dwYear)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x14;
		}

		ServerTime.wYear = (WORD)dwYear;

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[11], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wHour);


		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[14], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wMinute);


		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[17], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wSecond);

		*lpServerTime = ServerTime;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::ExtractDate");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SpreadRegistrationFilesInSystem
*  Description    : Drops Registered user .db file in system
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
DWORD CWardWizInstallerDlg::SpreadRegistrationFilesInSystem()
{
	try
	{
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
		{
			wsprintf(szDestin, L"%s\\Wardwiz Antivirus", szAllUserPath);
		}
		else
		{
			wsprintf(szDestin, L"%s\\Application Data\\Wardwiz Antivirus", szAllUserPath);
		}

		TCHAR szModulePath[MAX_PATH] = { 0 };
		_tcscpy(szModulePath, theApp.GetModuleFilePath());

		wcscpy(szDestin1, szDestin);

		wsprintf(szSource1, L"%s\\VBUSERREG.DB", szModulePath);
		wcscat(szDestin1, L"\\VBUSERREG.DB");

		CopyFileToDestination(szSource1, szDestin1);

		memset(szDestin1, 0x00, 512 * sizeof(TCHAR));
		wsprintf(szDestin1, L"%c:\\VBUSERREG.DB", szAllUserPath[0]);

		CopyFileToDestination(szSource1, szDestin1);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SpreadRegistrationFilesInSystem");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : CopyFileToDestination
*  Description    : Copy file to destination
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
bool CWardWizInstallerDlg::CopyFileToDestination(TCHAR *pszSource, TCHAR *pszDest)
{
	try
	{
		if (CheckDestinationPathExists(pszDest))
		{
			SetFileAttributes(pszDest, FILE_ATTRIBUTE_NORMAL);
			if (!DeleteFile(pszDest))
			{
				if (PathFileExists(pszDest))
				{
					MoveFileEx(pszDest, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
				else
					AddLogEntry(L"### Error in DeleteFile CWardwizInstallerDlg::CopyFileToDestination %s", pszDest, 0, true, SECONDLEVEL);
			}
		}

		if (!CopyFile(pszSource, pszDest, FALSE))
		{
			AddLogEntry(L"### Error in CopyFile CWardwizInstallerDlg::FileOperations, Source: %s, Dest: %s", pszSource, pszDest, true, SECONDLEVEL);
			return false;
		}

		if (PathFileExists(pszDest))
		{
			if (SetFileAttributes(pszDest, FILE_ATTRIBUTE_HIDDEN) == 0)
			{
				AddLogEntry(L"### Error in SetFileAttributes CWardwizInstallerDlg::FileOperations, Source: %s", pszSource, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CopyFileToDestination", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : CopyFileToDestination
*  Description    : Copy file to destination
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
bool CWardWizInstallerDlg::CheckDestinationPathExists(LPCTSTR DestinationPath)
{
	try
	{
		CString csDestPath(DestinationPath);
		int iPos = csDestPath.ReverseFind(L'\\');
		CString csDestFolderPath = csDestPath.Left(iPos);
		if (!PathFileExists(csDestFolderPath))
		{
			if (!CreateDirectory(csDestFolderPath, NULL))
			{
				AddLogEntry(L"### Error in CWardwizInstallerDlg::CheckDestinationPathExists,DestinationPath: %s", csDestFolderPath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CopyFileToDestination", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : FunGetSetRegisterData
*  Description    : This function is used to get data for product registration page
*  Author Name    : NITIN SHELAR
*  Date           :	17 August 2018
****************************************************************************************************/
json::value CWardWizInstallerDlg::FunGetMachineId()
{
	TCHAR szMachinId[MAX_PATH] = { 0 };
	try
	{
		_tcscpy_s(szMachinId, MAX_PATH, m_szMachineId);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::FunGetMachineId", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szMachinId);
}

/***************************************************************************************************
*  Function Name  : On_GetInternetConnection
*  Description    : This function is used to check internet connection
*  Author Name    : Akshay Patil
*  Date           :	11 April 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_GetInternetConnection()
{
	try
	{
		WinHttpClient client(L"http://www.google.com", NULL, 0x01);
		client.SetTimeouts(HTTP_RESOLVE_TIMEOUT, HTTP_CONNECT_TIMEOUT, HTTP_SEND_TIMEOUT, HTTP_RECEIVE_TIMEOUT);
		return (client.SendHttpRequest());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_GetInternetConnection", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : GetHostFileForWWRedirectionFlag
*  Description    : Checks hosts file for local redirection. if found, returns true else false.
*  Author Name    : Akshay Patil
*  Date           :	11 April 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::GetHostFileForWWRedirectionFlag()
{
	try
	{
		if (ReadHostFileForWWRedirection())
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetHostFileForVibraniumRedirectionFlag", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : DisplayFailureMsgOnUI
Description    : Callback to display failure msg
Author Name    : Nitin K
Date           : 3rd Dec 2015
***********************************************************************************************/
void CWardWizInstallerDlg::DisplayFailureMsgOnUI(CString csMessageType, CString strMessage)
{
	try
	{
		m_svFunSetRegStatusCB.call((SCITER_STRING)csMessageType, (SCITER_STRING)strMessage);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::DisplayFailureMsgOnUI");
	}
}

/***********************************************************************************************
Function Name  : DisplayFailureMessageForOfflineRegistration
Description    : Displays detailed error message in case offline registration is failed
Author Name    : Nitin K
SR.NO		   :
Date           : 3rd Dec 2015
***********************************************************************************************/
void CWardWizInstallerDlg::DisplayFailureMessageForOfflineRegistration(REGFAILUREMSGS dwRes)
{
	CString csMessage = L"";
	try
	{
		switch (dwRes)
		{
		case 0x01: //failed due to memory corruption
		case 0x02: //failed due to invalid length
		case 0x03: //failed due to decryption buffer failed
		case 0x05: //failed due to CRC failed
		case 0x06: //failed due to CheckSum failed
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_ACTIVATION1");
			break;
		case 0x04: //Activation code is invalid
			csMessage.Format(L"%s \n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_ACTIVATION1"),
				theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_ACTIVATION2"));
			break;
		case 07://Expired or Invalid Machine date
			csMessage.Format(L"%s \n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_PROD_EXP1"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL2"));
			break;
		default:
			csMessage.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_ACTIVATION1"));
			break;
		}
		DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, csMessage);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::DisplayFailureMessageForOfflineRegistration");
	}

}

/***********************************************************************************************
Function Name  : DisplayFailureMessage
Description    : Displays detailed error message in case online registration is failed
Author Name    : Nitin K
Date           : 3rd Dec 2015
***********************************************************************************************/
void CWardWizInstallerDlg::DisplayFailureMessage(REGFAILUREMSGS dwRes)
{
	CString csMessage;
	try
	{
		UINT iType = MB_ICONEXCLAMATION | MB_OK;
		switch (dwRes)
		{
		case 0x01:
		case 0x02:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE");
			break;
		case 0x03:
			iType = MB_ICONEXCLAMATION | MB_OK;
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_RES_CODE_INVALID");
			break;
		case 0x04:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE");
			break;
		case MACHINEIDMISMATCH:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_USED_REGISTRATION_KEY");
			break;
		case INVALIDEMAILID:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_EMAIL_ID");
			break;
		case COUNTRYCODEINVALID:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_VALID_SUPPORT");
			break;
		case INVALIDREGNUMBER:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_REGNO");
			break;
		case INVALIDPRODVERSION:
		{
								   /*	ISSUE NO - 753 NAME - NITIN K. TIME - 17th June 2014 */
								   switch (m_dwProductId)
								   {
								   case ESSENTIAL:
									   csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
										   theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Essential");
									   break;
								   case PRO:
									   csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
										   theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Pro");
									   break;
								   case ELITE:
									   csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
										   theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Elite");
									   break;
								   case BASIC:
									   csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
										   theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Basic");
									   break;
								   case ESSENTIALPLUS:
									   csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
										   theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Essential Plus");
									   break;

								   default:
									   csMessage.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INVALID_PROD_KEY"));
									   break;
								   }
		}
			break;
		case USERINFOUPDATEFAILD:
			csMessage.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE"), theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_CONTACT_SUPPORT"));
			//User Information update failed on server side
			break;
		case PRODUCTEXPIRED:
			csMessage.Format(L"%s \n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_PROD_EXP1"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL2"));
			break;
		case NULLRESPONSE:
			csMessage.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE"), theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_CONTACT_SUPPORT"));
			break;
		default:
			break;
		}
		DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, csMessage);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::DisplayFailureMessage");
	}
}

/***************************************************************************************************
*  Function Name  : AddProdRegInfoToLocal
*  Description    : Adding product registration information locally.
*  Author Name    : Amol Jaware
*  Date			  : 22 June 2017
****************************************************************************************************/
void CWardWizInstallerDlg::AddProdRegInfoToLocal(AVACTIVATIONINFO ActInfo, DWORD dwResSize)
{
	try
	{
		if (!g_regDataOperation.InsertDataIntoDLL((LPBYTE)&ActInfo, sizeof(ActInfo), IDR_REGDATA, TEXT("REGDATA")))
		{
			AddLogEntry(L"### Failed to InsertDataIntoDLL in CWardwizInstallerDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}

		Sleep(10);

		if (AddRegistrationDataInRegistry())
		{
			AddLogEntry(L"### AddRegistrationDataInRegistry in CWardwizInstallerDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}

		Sleep(10);

		if (g_regDataOperation.AddRegistrationDataInFile((LPBYTE)&ActInfo, sizeof(ActInfo)))
		{
			AddLogEntry(L"### AddRegistrationDataInFile failed in CWardwizInstallerDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::AddProdRegInfoToLocal", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : AddRegistrationDataInRegistry
*  Description    : Adding product registration on registration.
*  Author Name    : Akshay Patil
*  Date			  : 22 June 2017
****************************************************************************************************/
DWORD CWardWizInstallerDlg::AddRegistrationDataInRegistry()
{
	DWORD	dwRet = 0x00;
	HKEY	h_iSpyAV = NULL;
	try
	{
		AVACTIVATIONINFO	ActInfo = { 0 };

		memcpy(&ActInfo, &m_ActInfo, sizeof(ActInfo));

		if (DecryptDataReg((LPBYTE)&ActInfo, sizeof(ActInfo)))
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WOW64_64KEY | KEY_ALL_ACCESS, NULL, &h_iSpyAV, NULL) != ERROR_SUCCESS)
		{
			dwRet = 0x05;
			goto Cleanup;
		}

		if (RegSetValueEx(h_iSpyAV, TEXT("VibraniumUserInfo"), 0, REG_BINARY, (LPBYTE)&ActInfo, sizeof(ActInfo)) != ERROR_SUCCESS)
		{
			dwRet = 0x06;
			goto Cleanup;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::AddRegistrationDataInRegistry");
	}

Cleanup:

	if (h_iSpyAV)
		RegCloseKey(h_iSpyAV);

	h_iSpyAV = NULL;

	return dwRet;
}

/***********************************************************************************************
Function Name  : GetMachineIDOnMemory
Description    : Function which writes Machine ID into Registry.
SR.NO			 : WRDWIZCOMMSRV_59
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
bool CWardWizInstallerDlg::GetMachineIDOnMemory()
{
	try
	{
		TCHAR	szBIOSSerialNumber[0x38] = { 0 };		//56 bytes considered
		TCHAR	szCPUID[0x40] = { 0 };
		TCHAR	szClientID[0x80] = { 0 };
		TCHAR	szMACAddress[0x40] = { 0 };

		TCHAR szHDDSerial[MAX_PATH] = { 0 };
		CCPUInfo	objCPUInfo;
		_tcscpy_s(szHDDSerial, objCPUInfo.GetDiskSerialNo());

		GetCPUID(szCPUID);
		if (!szCPUID[0])
		{
			AddLogEntry(L"### Failed to GetMachineIDOnMemory::CPUID", 0, 0, true, SECONDLEVEL);
			return false;
		}

		GetBIOSSerialNumberSEH(szBIOSSerialNumber);
		if (!szBIOSSerialNumber[0])
		{

			//	ISSUE No : 163
			//	Some customers were not getting BIOS Serial Number, So we added MotherBoard Serial Number
			//	to make Unique Machine ID

			GetMotherBoardSerialNumberSEH(szBIOSSerialNumber);
			if (!szBIOSSerialNumber[0])
			{
				if (szHDDSerial[0])
				{
					_tcscpy_s(szMACAddress, szHDDSerial);
				}
				else
				{
					AddLogEntry(L"### Before MAC", 0, 0, true, SECONDLEVEL);
					GetMACAddress(szMACAddress);
					AddLogEntry(L"### After MAC", 0, 0, true, SECONDLEVEL);

					if (!szMACAddress[0])
					{
						AddLogEntry(L"### Before VGA", 0, 0, true, SECONDLEVEL);
						GetVGAAdapterID(szBIOSSerialNumber);
						if (!szBIOSSerialNumber[0])
						{
							AddLogEntry(L"### Failed to retrieve VGA ID", 0, 0, true, SECONDLEVEL);
							return false;
						}

						AddLogEntry(L"### After VGA", 0, 0, true, SECONDLEVEL);
					}
				}
			}
		}

		if (szBIOSSerialNumber[0])
		{
			RemoveCharsIfExists(szBIOSSerialNumber, static_cast<int>(wcslen(szBIOSSerialNumber)), static_cast<int>(sizeof(szBIOSSerialNumber)), 0x20);
		}

		if (!szMACAddress[0])
		{
			if (szHDDSerial[0])
			{
				_tcscpy_s(szMACAddress, szHDDSerial);
			}
			else
			{
				GetMACAddress(szMACAddress);
			}
		}

		int	i = static_cast<int>(wcslen(szCPUID));

		szClientID[0] = (TCHAR)i;
		swprintf(&szClientID[1], 0x7E, L"%s%s%s", szCPUID, szBIOSSerialNumber, szMACAddress);

		i = static_cast<int>(wcslen(szClientID)); \
		if (i > 0x7F)
			i = 0x7F;

		szClientID[i] = '\0';

		_tcscpy(m_szMachineId, szClientID);
	}
	//__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	catch (...)
	{
		AddLogEntry(L"### Exception in GetMachineIDOnMemory", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***********************************************************************************************
Function Name  : GetMachineIDOnMemory
Description    : Function which removes characters if exists.
Author Name    :
Date           :
***********************************************************************************************/
DWORD CWardWizInstallerDlg::RemoveCharsIfExists(TCHAR *pszBIOSSerialNumber, int iLen, int iSize, TCHAR chRemove)
{
	TCHAR	szTemp[56] = { 0 };

	if ((iLen <= 0) || iLen > 56)
		return 1;

	int i = 0x00, j = 0x00;

	for (i = 0; i<iLen; i++)
	{
		if (pszBIOSSerialNumber[i] != chRemove)
			szTemp[j++] = pszBIOSSerialNumber[i];
	}

	szTemp[j] = '\0';

	ZeroMemory(pszBIOSSerialNumber, iSize);
	wcscpy(pszBIOSSerialNumber, szTemp);

	return 0;
}

/***********************************************************************************************
Function Name  : GetCPUID
Description    : Function which get the CPU from hardware ID.
SR.NO			 : WRDWIZCOMMSRV_60
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
DWORD CWardWizInstallerDlg::GetCPUID(TCHAR *pszCPUID)
{
	__try
	{
		TCHAR	szData[0x10] = { 0 };
		int		b[4] = { 0 };

		wcscpy(pszCPUID, L"");

		for (int a = 0; a < 3; a++)
		{
			__cpuid(b, a);

			if ((a == 0 || a == 1) && b[0])
			{
				wsprintf(szData, L"%X", b[0]);
				//i = wcslen( szTemp ) ;
				wcscat(pszCPUID, szData);
			}

			if (a == 2)
			{
				if (b[0])
				{
					wsprintf(szData, L"%X", b[0]);
					wcscat(pszCPUID, szData);
				}

				if (b[1])
				{
					wsprintf(szData, L"%X", b[1]);
					wcscat(pszCPUID, szData);
				}

				if (b[2])
				{
					wsprintf(szData, L"%X", b[2]);
					wcscat(pszCPUID, szData);
				}

				if (b[3])
				{
					wsprintf(szData, L"%X", b[3]);
					wcscat(pszCPUID, szData);
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetCPUID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : GetBIOSSerialNumberSEH
Description    : Function which gets BIOS Number.
SR.NO			 : WRDWIZCOMMSRV_61
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
DWORD CWardWizInstallerDlg::GetBIOSSerialNumberSEH(TCHAR *psMotherBoardSerialNumber)
{
	__try
	{
		return GetBIOSSerialNumber(psMotherBoardSerialNumber);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return 0x01;
	}

	return 0x02;
}

/***********************************************************************************************
Function Name  : GetBIOSSerialNumber
Description    : Function which gets BIOS Number.
SR.NO			 : WRDWIZCOMMSRV_61
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
DWORD CWardWizInstallerDlg::GetBIOSSerialNumber(TCHAR *pszBIOSSerialNumber)
{
	DWORD				dwRet = 0x00;

	HRESULT				hres = S_OK;
	HRESULT				hr = S_OK;
	IWbemLocator		*pLoc = NULL;
	IWbemServices		*pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject	*pclsObj = NULL;
	ULONG uReturn = 0;

	VARIANT				vtProp;
	CString				hh = L"";

	try
	{
		//static bool			g_bCoInitializeSecurityCalled = false;
		/*
		hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
		if( hres != S_OK )
		{
		ErrorDescription(hres);
		dwRet = 0x01 ;
		goto Cleanup ;
		}
		*/
		if (!g_bCoInitializeSecurityCalled)
		{

			hres = CoInitializeEx(0, COINIT_MULTITHREADED);
			/*if( hres != S_OK )
			{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
			}*/

			hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
			/*if( hres != S_OK )
			{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
			}*/

			g_bCoInitializeSecurityCalled = true;
		}

		hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
		if (hres != S_OK)
		{
			ErrorDescription(hres);
			dwRet = 0x02;
			goto Cleanup;
		}

		if (!pLoc)
		{
			ErrorDescription(hres);
			dwRet = 0x03;
			goto Cleanup;
		}

		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
		if (hres != S_OK)
		{
			ErrorDescription(hres);
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!pSvc)
		{
			ErrorDescription(hres);
			dwRet = 0x05;
			goto Cleanup;
		}

		hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
		if (hres != S_OK)
		{
			ErrorDescription(hres);
			dwRet = 0x06;
			goto Cleanup;
		}

		hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_BIOS"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
		if (hres != S_OK)
		{
			ErrorDescription(hres);
			dwRet = 0x07;
			goto Cleanup;
		}

		while (pEnumerator)
		{
			hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
			if (0 == uReturn)
				break;

			if (NULL == pclsObj)
				break;

			hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
			hh = vtProp.bstrVal;
			VariantClear(&vtProp);
			pclsObj->Release();

			hh.Trim();
			if (hh.GetLength())
			{
				wsprintf(pszBIOSSerialNumber, L"%s", hh.Trim());
				break;
			}

		}
	}
	catch (...)
		//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetBIOSSerialNumber", 0, 0, true, SECONDLEVEL);
	}
Cleanup:
	if (pSvc)
		pSvc->Release();

	if (pLoc)
		pLoc->Release();

	if (pEnumerator)
		pEnumerator->Release();

	CoUninitialize();

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetMotherBoardSerialNumberSEH()
*  Description    : Gets MotherBoard number through WMI
*  Author Name    : Vilas                                                                                      *
*  Date			  :	08- Sept-2014 - 12 jul -2014
*  Modified Date  :
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetMotherBoardSerialNumberSEH(TCHAR *psMotherBoardSerialNumber)
{
	__try
	{
		return GetMotherBoardSerialNumber(psMotherBoardSerialNumber);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return 0x01;
	}

	return 0x02;
}

/***************************************************************************************************
*  Function Name  : GetMotherBoardSerialNumber()
*  Description    : Gets MotherBoard number through WMI
*  Author Name    : Vilas
*  Date			  :	08- Sept-2014 - 12 jul -2014
*  Modified Date  :
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetMotherBoardSerialNumber(TCHAR *psMotherBoardSerialNumber)
{
	//AddLogEntry(L">>> Inside GetBIOSSerialNumber");

	DWORD				dwRet = 0x00;

	HRESULT				hres = S_OK;
	HRESULT				hr = S_OK;
	IWbemLocator		*pLoc = NULL;
	IWbemServices		*pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject	*pclsObj = NULL;
	ULONG uReturn = 0;

	VARIANT				vtProp;
	CString				hh = L"";

	try
	{
		//static bool			g_bCoInitializeSecurityCalled = false;

		//AddLogEntry(L">>> in GetBIOSSerialNumber::CoInitializeEx before");
		/*
		hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
		if( hres != S_OK )
		{
		AddLogEntry(L"### Exception in GetBIOSSerialNumber::CoInitializeEx");
		//ErrorDescription(hres);
		dwRet = 0x01 ;
		goto Cleanup ;
		}
		*/
		if (!g_bCoInitializeSecurityCalled)
		{
			hres = CoInitializeEx(0, COINIT_MULTITHREADED);
			//if( hres != S_OK )
			//{
			//	AddLogEntry(L"### Exception in GetBIOSSerialNumber::CoInitializeEx");
			//	//ErrorDescription(hres);
			//	dwRet = 0x01 ;
			//	goto Cleanup ;
			//}

			//AddLogEntry(L">>>> in before GetBIOSSerialNumber::CoInitializeSecurity ");
			hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
			//if( hres != S_OK )
			//{
			//	AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::CoInitializeSecurity");
			//	//ErrorDescription(hres);
			//	dwRet = 0x01 ;
			//	goto Cleanup ;
			//}

			g_bCoInitializeSecurityCalled = true;
		}

		//AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::CoCreateInstance");
		hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
		if (hres != S_OK)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::CoCreateInstance");
			//ErrorDescription(hres);
			dwRet = 0x02;
			goto Cleanup;
		}

		if (!pLoc)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::Locator failed");
			//ErrorDescription(hres);
			dwRet = 0x03;
			goto Cleanup;
		}

		AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::ConnectServer");
		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
		if (hres != S_OK)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ConnectServer");
			//ErrorDescription(hres);
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!pSvc)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::WebServicce failed");
			//ErrorDescription(hres);
			dwRet = 0x05;
			goto Cleanup;
		}

		AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::CoSetProxyBlanket");
		hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
		if (hres != S_OK)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ConnectServer::CoSetProxyBlanket");
			//ErrorDescription(hres);
			dwRet = 0x06;
			goto Cleanup;
		}

		//AddLogEntry(L">>>> in before GetBIOSSerialNumber::ExecQuery");
		hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_BaseBoard"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
		if (hres != S_OK)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ExecQuery");
			//ErrorDescription(hres);
			dwRet = 0x07;
			goto Cleanup;
		}

		//AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::while( pEnumerator )");
		while (pEnumerator)
		{

			//AddLogEntry(L">>> inside GetBIOSSerialNumber::while( pEnumerator )");

			hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
			if (0 == uReturn)
			{
				AddLogEntry(L"### Failed in GetMotherBoardSerialNumber::pEnumerator->Next");
				break;
			}

			if (NULL == pclsObj)
			{
				AddLogEntry(L"### Failed in GetMotherBoardSerialNumber::Web Class Object");
				break;
			}

			hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
			hh = vtProp.bstrVal;
			VariantClear(&vtProp);
			pclsObj->Release();

			//AddLogEntry(L">>> inside GetMotherBoardSerialNumber::before hh.GetLength()");

			hh.Trim();
			if (hh.GetLength() /* wcslen(vtProp.bstrVal) > 0x02 */)
			{

				wsprintf(psMotherBoardSerialNumber, L"%s", hh.Trim());
				//wsprintf(psMotherBoardSerialNumber, L"%s", vtProp.bstrVal ) ;
				AddLogEntry(L">>> Got GetMotherBoardSerialNumber::%s", psMotherBoardSerialNumber);

				break;
			}

			//AddLogEntry(L">>> inside GetBIOSSerialNumber::after hh.GetLength()");
		}
	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetMotherBoardSerialNumber");
	}

Cleanup:

	//AddLogEntry(L">>> inside GetBIOSSerialNumber::before Cleanup");

	if (pSvc)
		pSvc->Release();

	if (pLoc)
		pLoc->Release();

	if (pEnumerator)
		pEnumerator->Release();

	CoUninitialize();

	//AddLogEntry(L">>> inside GetBIOSSerialNumber::after Cleanup");

	//AddLogEntry(L">>> Out GetBIOSSerialNumber");

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetMACAddress()
*  Description    : returns MAC Address which is 6 bytes
*  Author Name    : Vilas                                                                                      *
*  Date			  :	26-Sept-2014
*  Modified Date  :	27-Sept-2014
****************************************************************************************************/
DWORD CWardWizInstallerDlg::GetMACAddress(TCHAR *pMacAddress, LPTSTR lpszOldMacID)
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapInfo = NULL;

	DWORD dwBufLen = 0x00;
	DWORD dwCount = 0x00;
	DWORD dwRet = 0x00, dwDivisor = 0x00, dwStatus = 0x00;

	__try
	{
		dwStatus = GetAdaptersInfo(pAdapterInfo, &dwBufLen);
		if (dwStatus != ERROR_BUFFER_OVERFLOW)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		dwDivisor = sizeof IP_ADAPTER_INFO;

		if (sizeof time_t == 0x08)
			dwDivisor -= 8;

		dwCount = dwBufLen / dwDivisor;
		if (!dwCount)
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		pAdapterInfo = new IP_ADAPTER_INFO[dwCount];
		if (!pAdapterInfo)
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		//TCHAR	szMacAddress[64] = {0};

		ZeroMemory(pAdapterInfo, dwBufLen);
		if (GetAdaptersInfo(pAdapterInfo, &dwBufLen) != ERROR_SUCCESS)
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		//TCHAR	szMacAddress[0x400] = {0};
		TCHAR	szDescp[0x200] = { 0 };
		//bool	bTypeIEEE80211 = false;

		pAdapInfo = pAdapterInfo;

		while (pAdapInfo)
		{
			//MultiByteToWideChar( CP_ACP, 0, pAdapInfo->Description, -1, szDescp, sizeof(TCHAR)*0x1FF ) ;

			//Added to get only Ethernet address
			if ((strstr(pAdapInfo->Description, "Virtual ") == NULL) &&
				(strstr(pAdapInfo->Description, "Bluetooth ") == NULL) &&
				(strstr(pAdapInfo->Description, "Wireless ") == NULL) &&
				(strstr(pAdapInfo->Description, "(PAN)") == NULL) &&
				(strstr(pAdapInfo->Description, "Wi-Fi ") == NULL) &&
				(strstr(pAdapInfo->Description, "WiFi ") == NULL))
			{
				wsprintf(pMacAddress, L"%02X%02X%02X%02X%02X%02X", pAdapInfo->Address[0],
					pAdapInfo->Address[1], pAdapInfo->Address[2],
					pAdapInfo->Address[3], pAdapInfo->Address[4],
					pAdapInfo->Address[5]);

				if (lpszOldMacID == NULL)
				{
					AddLogEntry(L">>> MACID: %s", pMacAddress, 0, true, ZEROLEVEL);
					break;
				}

				int iLen = _tcslen(lpszOldMacID);
				if (iLen != 0x00 && (iLen - 0x0C) >= 0x0C)
				{
					if (memcmp(&pMacAddress[0], &lpszOldMacID[iLen - 0x0C], 0x0C) == 0)
					{
						AddLogEntry(L">>> MACID: %s", pMacAddress, 0, true, ZEROLEVEL);
						break;
					}
				}
			}

			pAdapInfo = pAdapInfo->Next;
		}

		if (!wcslen(pMacAddress))
		{
			AddLogEntry(L"### Failed in GetMACAddress", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetMACAddress", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if (pAdapterInfo)
		delete[] pAdapterInfo;

	pAdapterInfo = NULL;

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetVGAAdapterID()
*  Description    : returns VGAAdapterID
*  Author Name    :
*  Date			  :
****************************************************************************************************/
bool CWardWizInstallerDlg::GetVGAAdapterID(TCHAR *pszDispAdapterID)
{

	HDEVINFO        hDevInfo = 0L;
	SP_DEVINFO_DATA spDevInfoData = { 0 };
	SP_CLASSIMAGELIST_DATA _spImageData = { 0 };

	TCHAR	szMotherBoradRes[512] = { 0 };

	short	wIndex = 0;
	bool	bVGA = false;

	_try
	{

		hDevInfo = SetupDiGetClassDevs(0L, 0L, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);
		if (hDevInfo == (void*)-1)
		{
			AddLogEntry(L"#### Failed in GetVGAAdapterID::SetupDiGetClassDevs");
			return 1;
		};

		wIndex = 0;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		int iDev = 0x00;

		while (1)
		{

			TCHAR  szBuf[MAX_PATH] = { 0 };
			TCHAR  szID[LINE_LEN + 1] = { 0 };
			TCHAR                   szName[64] = { 0 };

			if (SetupDiEnumDeviceInfo(hDevInfo,
				wIndex,
				&spDevInfoData))
			{

				short wImageIdx = 0;
				short wItem = 0;

				if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
					&spDevInfoData,
					SPDRP_CLASS, //SPDRP_DEVICEDESC,
					0L,
					(PBYTE)szBuf,
					2048,
					0))
				{
					wIndex++;
					continue;
				};

				SetupDiGetClassImageIndex(&_spImageData, &spDevInfoData.ClassGuid, (int*)&wImageIdx);

				TCHAR                   szPath[MAX_PATH] = { 0 };
				DWORD                  dwRequireSize = 0x00;

				if (!SetupDiGetClassDescription(&spDevInfoData.ClassGuid,
					szBuf,
					MAX_PATH,
					&dwRequireSize))
				{
					wIndex++;
					continue;
				};


				SetupDiGetDeviceInstanceId(hDevInfo, &spDevInfoData, szID, LINE_LEN, 0);
				if (SetupDiGetDeviceRegistryProperty(hDevInfo,
					&spDevInfoData,
					SPDRP_FRIENDLYNAME,
					0L,
					(PBYTE)szName,
					63,
					0))
				{
					//DisplayDriverDetailInfo(hItem, nIdTree, szName, wImageIdx, wImageIdx);
					//AddNewDeviceNode(spDevInfoData.ClassGuid, szName, szID, szPath, wIndex, wOrder);
				}
				else if (SetupDiGetDeviceRegistryProperty(hDevInfo,
					&spDevInfoData,
					SPDRP_DEVICEDESC,
					0L,
					(PBYTE)szName,
					63,
					0))
				{
					//DisplayDriverDetailInfo(hItem, nIdTree, szName, wImageIdx, wImageIdx);
					//AddNewDeviceNode(spDevInfoData.ClassGuid, szName, szID, szPath, wIndex, wOrder);
					//                    if (!GetFirmwareEnvironmentVariable(szName, (LPCSTR)&spDevInfoData.ClassGuid, szBuf, 127))
					//                        ShowErrorMsg(_hDlg, GetLastError(), "GetFirmwareEnvironmentVariable");
				};

				if ((_wcsicmp(szBuf, L"Display adapters") == 0) &&
					(_wcsicmp(szName, L"Standard VGA Graphics Adapter") == 0))
				{

					if (wcslen(szID) > 0x04)
					{
						wcscpy_s(pszDispAdapterID, 0x37, &szID[0x08]);
						bVGA = true;
					}

					break;
				}

			}

			wIndex++;
			if (wIndex > 0x80)
				break;
		}

		if (!pszDispAdapterID[0])
			//AddLogEntry(L"### GetVGAAdapterID::failed to VGA not found");
			wcscpy_s(pszDispAdapterID, 0x37, L"_NULL");
	}
	//catch(...)
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetVGAAdapterID", 0, 0, true, SECONDLEVEL);
	}

	return bVGA;
}

/***************************************************************************************************
*  Function Name  : ReadHostFileForWWRedirection()
*  Description    : Checks hosts file for local redirection. if found, returns true else false.
*  Author Name    : Vilas
*  Date			  :	29-Sept-2014
*  Modified Date  :	30-Sept-2014
****************************************************************************************************/
bool CWardWizInstallerDlg::ReadHostFileForWWRedirection()
{
	bool bRedirection = false;
	try
	{
		FILE *pFileStream = NULL;

		long lFileSize = 0x00, lReadSize = 0x00;

		TCHAR szHostPath[512] = { 0 };
		TCHAR szFileLine[1024] = { 0 };

		try
		{
			GetSystemDirectory(szHostPath, 511);
			wcscat_s(szHostPath, L"\\Drivers\\etc\\hosts");

			pFileStream = _wfsopen(szHostPath, _T("r"), _SH_DENYNO);
			if (!pFileStream)
				goto Cleanup;

			fseek(pFileStream, 0L, SEEK_END);
			lFileSize = ftell(pFileStream);

			if (!lFileSize)
				goto Cleanup;

			fseek(pFileStream, 0L, SEEK_SET);

			while (true)
			{
				if (fgetws(szFileLine, 1023, pFileStream) == NULL)
					break;

				if (!wcslen(szFileLine))
					break;

				if ((StrStrI(szFileLine, L"wardwiz.")) || (StrStrI(szFileLine, L"162.144.82.101")))
				{
					bRedirection = true;
					break;
				}

				if (feof(pFileStream))
					break;

				lReadSize += static_cast<long>(wcslen(szFileLine));
				if (lReadSize >= lFileSize)
					break;

				ZeroMemory(szFileLine, sizeof(szFileLine));
			}
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in ReadHostFileForWardwizRedirection", 0, 0, true, SECONDLEVEL);
		}

	Cleanup:

		if (pFileStream)
			fclose(pFileStream);

		pFileStream = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::ReadHostFileForGenXRedirection");
	}
	return bRedirection;
}

/***************************************************************************************************
*  Function Name  : GetProdIdNVersion()
*  Description    : Function to get product version & id
*  Author Name    : Tejas Shinde
*  Date			  :	14-06-2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::GetProdIdNVersion(SCITER_VALUE svGetProdIdNProdVersion, SCITER_VALUE svGetProdIdNVersion)
{
	try
	{
		m_svGetProdIdNProdVersion = svGetProdIdNProdVersion;
		m_svGetProdIdNVersion = svGetProdIdNVersion;
		m_svGetProdIdNProdVersion.call((SCITER_VALUE)theApp.m_dwProdID, (SCITER_STRING)theApp.m_csProdVersion);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetProdIdNVersion", 0, 0, true, ZEROLEVEL);
	}

	return 0;
}


/***************************************************************************************************
*  Function Name  : SetDownloadedBytes
*  Description    : callback function to set downloaded bytes.
*  Author Name    : Ram Shelke
*  Date			  :	13-Dec-2017
****************************************************************************************************/
void CGUIInterface::SetDownloadedBytes(DWORD dwTotalFileLength, DWORD dwDownloadedBytes, double dTransferRate)
{
	try
	{

		long DownloadBytes = g_pThis->g_lTotalDownloadedBytes + dwDownloadedBytes;

		if (g_pThis->m_dwTotalFileSize == 0x00)
		{
			g_pThis->m_dwPercentage = 0x01;
		}
		else
		{
			g_pThis->m_dwPercentage = g_pThis->GetPercentage(DownloadBytes, g_pThis->m_dwTotalFileSize);
		}

		if (g_pThis->m_dwPercentage >= 100)
		{
			g_pThis->m_dwPercentage = 99;
			return;
		}

		DWORD dwPercent = g_pThis->m_dwPercentage;
		if (dwPercent == 0x00)
		{
			return;
		}

		if (g_pThis->g_iPreviousPerc >= (int)dwPercent)
		{
			dwPercent = g_pThis->g_iPreviousPerc;
		}

		if ((DWORD)DownloadBytes > g_pThis->m_dwTotalFileSize)
		{
			DownloadBytes = g_pThis->m_dwTotalFileSize;
		}

		CString csPercent, cRemaining, cSpeed;

		csPercent.Format(L"%d", dwPercent);
		g_pThis->m_svUpdateDownloadStatus.call((SCITER_STRING)cRemaining, (SCITER_STRING)cSpeed, (SCITER_STRING)csPercent);
		g_pThis->g_iPreviousPerc = dwPercent;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SetDownloadedBytes", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SetPercentDownload
*  Description    : callback function to set downloaded percentage.
*  Author Name    : Ram Shelke
*  Date			  :	13-Dec-2017
****************************************************************************************************/
void CGUIInterface::SetPercentDownload(int nPercent)
{

}

/***************************************************************************************************
*  Function Name  : SetSelectedLanguage
*  Description    : Set language ID in .ini file
*  Author Name    : Kunal Waghmare
*  Date           : 26 June 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::SetSelectedLanguage(SCITER_VALUE svrLangName)
{
	try
	{
		DWORD dwLangID = svrLangName.get(0);
		theApp.m_dwLangID = dwLangID;
		CString csLangID = L"0";
		csLangID.Format(L"%d", dwLangID);
		CString csIniFilePath = GetModuleFileStringPath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"LanguageID", csLangID, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SetSelectedLanguage", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : PauseDownload
*  Description    : Pause setup downloading
*  Author Name    : Kunal Waghmare
*  Date           : 26 June 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::PauseDownload()
{
	try
	{
		m_bIsProcessPaused = false;
		KillTimer(TIMER_SETPERCENTAGE);
		if (!m_hStartWardWizSetupDwnldProc)
		{
			AddLogEntry(L"### Wardwiz download setup thread is not running", 0, 0, true, ZEROLEVEL);
			return true;
		}
		if (SuspendThread(m_hStartWardWizSetupDwnldProc) != 0xFFFFFFFF)
		{
			m_pDownloadController->SetThreadPoolStatus(true);
			AddLogEntry(L">>> Wardwiz download setup thread suspended successfully.", 0, 0, true, SECONDLEVEL);
			return true;
		}
		AddLogEntry(L"### Wardwiz download setup thread suspended failed.", 0, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::PauseDownload", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : ResumeDownload
*  Description    : resume setup downloading
*  Author Name    : Kunal Waghmare
*  Date           : 26 June 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::ResumeDownload()
{
	try
	{
		if (!m_hStartWardWizSetupDwnldProc)
		{
			AddLogEntry(L"### Wardwiz download setup thread is not running", 0, 0, true, ZEROLEVEL);
			return true;
		}
		SetTimer(TIMER_SETPERCENTAGE, 10, NULL);
		if (ResumeThread(m_hStartWardWizSetupDwnldProc) != 0xFFFFFFFF)
		{
			m_pDownloadController->SetThreadPoolStatus(false);
			AddLogEntry(L">>> Wardwiz download setup thread resumed successfully.", 0, 0, true, ZEROLEVEL);
			return true;
		}
		AddLogEntry(L"### Wardwiz download setup thread resumed failed.", 0, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::ResumeDownload", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : On_PauseInstallation
*  Description    : Pause installation
*  Author Name    : Kunal Waghmare
*  Date           : 26 June 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_PauseInstallation()
{
	try
	{
		m_bIsProcessPaused = false;
		KillTimer(TIMER_SETFLEPATH_PER);
		if (!m_hThreadinstall)
		{
			AddLogEntry(L"### Wardwiz installation thread is not running", 0, 0, true, ZEROLEVEL);
			return true;
		}

		if (m_dwTmpProcID == -1)
			m_hTmpProcess = GetTmpProcessHandle(m_csTmpProcess);

		typedef LONG(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
		NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandleA("ntdll"), "NtSuspendProcess");
		if (m_hTmpProcess)
		{
			if (pfnNtSuspendProcess)
			{
				pfnNtSuspendProcess(m_hTmpProcess);
				AddLogEntry(L">>> Wardwiz tmp setup installation process suspended successfully.", 0, 0, true, SECONDLEVEL);
			}
		}
		if (m_hProcess)
		{
			if (pfnNtSuspendProcess)
			{
				pfnNtSuspendProcess(m_hProcess);
				AddLogEntry(L">>> Wardwiz installation process suspended successfully.", 0, 0, true, SECONDLEVEL);
			}
		}
		if (SuspendThread(m_hThreadinstall) != 0xFFFFFFFF)
		{
			AddLogEntry(L">>> Wardwiz installation thread suspended successfully.", 0, 0, true, SECONDLEVEL);
			return true;
		}

		AddLogEntry(L"### Wardwiz installation thread suspended failed.", 0, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_PauseInstallation", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : On_ResumeInstallation
*  Description    : Resumes installation
*  Author Name    : Kunal Waghmare
*  Date           : 26 June 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_ResumeInstallation()
{
	try
	{
		if (!m_hThreadinstall)
		{
			AddLogEntry(L"### Wardwiz installation thread is not running", 0, 0, true, ZEROLEVEL);
			return true;
		}
		SetTimer(TIMER_SETFLEPATH_PER, 50, NULL);

		typedef LONG(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);
		NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(GetModuleHandleA("ntdll"), "NtResumeProcess");
		if (m_hProcess)
		{
			if (pfnNtResumeProcess)
			{
				pfnNtResumeProcess(m_hProcess);
				AddLogEntry(L">>> Wardwiz installation thread resumed successfully.", 0, 0, true, ZEROLEVEL);
			}
		}
		if (m_hTmpProcess)
		{
			if (pfnNtResumeProcess)
			{
				pfnNtResumeProcess(m_hTmpProcess);
				AddLogEntry(L">>> Wardwiz tmp setup installation process thread resumed successfully.", 0, 0, true, SECONDLEVEL);
			}
		}
		if (ResumeThread(m_hThreadinstall) != 0xFFFFFFFF)
		{
			AddLogEntry(L">>> Wardwiz installation thread resumed successfully.", 0, 0, true, ZEROLEVEL);
			return true;
		}
		AddLogEntry(L"### Wardwiz installation thread resumed failed.", 0, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_ResumeInstallation", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : On_PasueQuickScan
*  Description    : Pause Quick scan (preinstall)
*  Author Name    : Kunal Waghmare
*  Date           : 26 June 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_PauseQuickScan()
{
	try
	{
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
				AddLogEntry(L">>> m_hThread_ScanCount Scanning paused inside CWardwizInstallerDlg::On_PauseQuickScan", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### m_hThread_ScanCount failed to pause scanning as SuspendThread request failed.", 0, 0, true, SECONDLEVEL);
			}
			if (m_hWardWizAVThread != NULL)
			{
				::SuspendThread(m_hWardWizAVThread);
				AddLogEntry(L">>> m_hWardwizAVThread Scanning paused inside CWardwizInstallerDlg::On_PauseQuickScan", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### m_hWardwizAVThread failed to pause scan as SuspendThread request failed.", 0, 0, true, SECONDLEVEL);
			}
			theApp.m_bQuickScan = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_PasueQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_ResumeQuickScan
*  Description    : Resumes quick scan (preinstall)
*  Author Name    : Kunal Waghmare
*  Date           : 26 June 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_ResumeQuickScan()
{
	try
	{
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
				AddLogEntry(L">>> m_hThread_ScanCount thread resume for scanning inside CWardwizInstallerDlg::On_ResumeQuickScan", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### m_hThread_ScanCount failed to resume thread for scanning inside CWardwizInstallerDlg::On_ResumeQuickScan", 0, 0, true, SECONDLEVEL);
			}
			if (m_hWardWizAVThread != NULL)
			{
				::ResumeThread(m_hWardWizAVThread);
				AddLogEntry(L">>> m_hWardwizAVThread thread resume for scanning inside CWardwizInstallerDlg::On_ResumeQuickScan.", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### m_hWardwizAVThread thread resume for scanning inside CWardwizInstallerDlg::On_ResumeQuickScan", 0, 0, true, SECONDLEVEL);
				return false;
			}
			//sciter::dom::element(self).start_timer(500);
			theApp.m_bQuickScan = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_ResumeQuickScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : GetTmpProcessHandle
*  Description    : return tmp process handle of installation
*  Author Name    : Kunal Waghmare
*  Date           : 26 June 2019
****************************************************************************************************/
HANDLE CWardWizInstallerDlg::GetTmpProcessHandle(CString csTmpProcessName)
{
	try
	{
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
		{
			if (wcsicmp(entry.szExeFile, m_csTmpProcess.GetBuffer()) == 0)
			{
				m_dwTmpProcID = entry.th32ProcessID;
			}
		}
		CloseHandle(snapshot);
		return OpenProcess(PROCESS_ALL_ACCESS, TRUE, m_dwTmpProcID);
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetTmpProcessHandle", 0, 0, true, SECONDLEVEL);
	}
	return NULL;
}

/***************************************************************************************************
*  Function Name  : CheckForCorruption
*  Description    : Function which checks for corrupted file.
*  Author Name    : Ram Shelke
*  Date			  :	13-Dec-2017
****************************************************************************************************/
bool CWardWizInstallerDlg::CheckForCorruption(LPTSTR szZipFilePath)
{
	try
	{
		if (!szZipFilePath)
			return false;

		if (!PathFileExists(szZipFilePath))
			return false;

		CString csFileName(szZipFilePath);
		int iFound = csFileName.ReverseFind(L'\\');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		/*TCHAR	szZipUnzipPath[512] = { 0 };
		swprintf_s(szZipUnzipPath, _countof(szZipUnzipPath), L"%s\\%s\\%s_%s", m_szAllUserPath, m_csAppFolderName, csFileName, L"WTEST");

		DWORD dwRet = UnzipUsingZipArchive(szZipFilePath, szZipUnzipPath);

		SetFileAttributes(szZipUnzipPath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(szZipUnzipPath);

		if (dwRet != 0x00)
		{
		return true;
		}*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateManager::CheckForCorruption, FilePath: [%s]", szZipFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : GetSystemInformation
*  Description    : Function to get System Info from INI
*  Author Name    : Akshay Patil
*  Date           : 19 Jun 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::GetSystemInformation(SCITER_VALUE svSetSystemInfoCB)
{
	try
	{
		m_svSetSystemInfoCB = svSetSystemInfoCB;
		SystemInfoDetails();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetSystemInformation");
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : SystemInfoDetails
*  Description    : Function to get System Info from INI
*  Author Name    : Akshay Patil
*  Date           : 19 Jun 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::SystemInfoDetails()
{
	try
	{
		if (!GetSystemDetails())
		{
			AddLogEntry(L"### SystemInfoDetails::Failed to get system details");
			return false;
		}

		const sciter::value data[6] = { sciter::string(m_objSysdetails.szOsName), sciter::string(m_objSysdetails.szRAM), sciter::string(m_objSysdetails.szHardDiskSize),
			sciter::string(m_objSysdetails.szProcessor), sciter::string(m_objSysdetails.szCompName), sciter::string(m_objSysdetails.szLoggedInUser) };

		sciter::value svArrSystemDetails = sciter::value(data, 6);
		m_svSetSystemInfoCB.call(svArrSystemDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SystemInfoDetails", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : GetSystemDetails
*  Description    : Function to get System Info
*  Author Name    : Akshay Patil
*  Date           : 15 May 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::GetSystemDetails()
{
	try
	{
		WardWizSystemInfo	objSysInfo;
		objSysInfo.GetSystemInformation();

		CString csProcessorType = L"";

		if (objSysInfo.GetOSType())
			csProcessorType.Append(L"64bit");
		else
			csProcessorType.Append(L"32bit");

		//RAM Information 
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof (statex);
		GlobalMemoryStatusEx(&statex);

		CString csRamDetail = L"";
		csRamDetail.AppendFormat(L"%*I64dGB", 7, (statex.ullTotalPhys / (1024 * 1024 * 1024)) + 1);

		//HD Size
		DISK_GEOMETRY pdg = { 0 };
		BOOL bResult = FALSE;
		ULONGLONG DiskSize = 0;
		CString csHDSize = L"";
		bResult = GetDriveGeometry(&pdg);
		if (bResult)
		{
			DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder * (ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
			csHDSize.AppendFormat(L"%.2fGB", (double)DiskSize / (1024 * 1024 * 1024));
		}
		else
		{
			AddLogEntry(L"### CWardwizInstallerDlg::GetDriveGeometry failed.", 0, 0, true, SECONDLEVEL);
		}

		memset(&m_objSysdetails, 0x00, sizeof(m_objSysdetails));
		wcscpy_s(m_objSysdetails.szCompName, objSysInfo.GetSystemName());
		wcscpy_s(m_objSysdetails.szOsName, objSysInfo.GetOSDetails());
		wcscpy_s(m_objSysdetails.szRAM, csRamDetail.Trim().GetBuffer());
		wcscpy_s(m_objSysdetails.szHardDiskSize, csHDSize.GetBuffer());
		wcscpy_s(m_objSysdetails.szProcessor, csProcessorType.GetBuffer());
		wcscpy_s(m_objSysdetails.szLoggedInUser, objSysInfo.GetUserNameOfSystem());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetSystemDetails", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : GetDriveGeometry
*  Description    : Function to get disk details(TracksPerCylinder, SectorsPerTrack etc)
*  Author Name    : Akshay Patil
*  Date           : 15 May 2019
****************************************************************************************************/
BOOL CWardWizInstallerDlg::GetDriveGeometry(DISK_GEOMETRY *pdg)
{
	BOOL bResult = FALSE;
	try
	{
		HANDLE hDevice = INVALID_HANDLE_VALUE;
		DWORD junk = 0;
		LARGE_INTEGER lFileSize;

		hDevice = CreateFileW(WSZDRIVE, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (hDevice == INVALID_HANDLE_VALUE)
		{
			return (FALSE);
		}
		GetFileSizeEx(hDevice, &lFileSize);

		bResult = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, pdg, sizeof(*pdg), &junk, (LPOVERLAPPED)NULL);

		CloseHandle(hDevice);

		return (bResult);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::GetDriveGeometry", 0, 0, true, SECONDLEVEL);
	}

	return (bResult);
}

/***************************************************************************************************
*  Function Name  : DecryptDataReg
*  Description    : Decrypt/Encrypt data for registration.
*  Author Name    : Akshay Patil
*  Date           : 26 Jun 2019
****************************************************************************************************/
DWORD CWardWizInstallerDlg::DecryptDataReg(LPBYTE lpBuffer, DWORD dwSize)
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
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::DecryptDataReg");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : PerformPostRegOperation
*  Description    : Function to perform post reg changes for regitry & INI.
*  Author Name    : Akshay Patil
*  Date           : 26 Jun 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::PerformPostRegOperation()
{
	try
	{
		if (m_bIsRegFlag == true)
		{
			CopyUserLocDetails();

			if (AddRegDataToRegistry())
			{
				AddLogEntry(L"### AddRegDataToRegistry::Failed to add registration details to registry");
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::PerformPostRegOperation");
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : AddRegDataToRegistry
*  Description    : Function to add registration type to registry.
*  Author Name    : Akshay Patil
*  Date           : 27 Jun 2019
****************************************************************************************************/
DWORD CWardWizInstallerDlg::AddRegDataToRegistry()
{
	HKEY key = NULL;
	DWORD dwRegVal = 0x00;
	DWORD dwRet = 0x00;

	try
	{
		if (m_bOnlineActivation)
			dwRegVal = 0x00;
		else if (m_bOfflineActivation)
			dwRegVal = 0x01;

		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Vibranium"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WOW64_64KEY | KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS)
		{
			dwRet = 0x01;
			return dwRet;
		}

		if (RegSetValueEx(key, TEXT("dwRegUserType"), 0, REG_DWORD, (LPBYTE)&dwRegVal, sizeof(DWORD)) != ERROR_SUCCESS)
		{
			RegCloseKey(key);
			key = NULL;
			dwRet = 0x02;
			return dwRet;
		}
		RegCloseKey(key);
		key = NULL;
	}
	catch (...)
	{
		dwRet = 0x03;
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::AddRegDataToRegistry");
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : CopyUserLocDetails
*  Description    : Function to copy user location details from temp INI to WardWiz folder INI.
*  Author Name    : Akshay Patil
*  Date           : 27 Jun 2019
****************************************************************************************************/
void CWardWizInstallerDlg::CopyUserLocDetails()
{
	try
	{
		CString csIniFilePath;

		if (_tcslen(m_szDealerCode) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"DealerCode", m_szDealerCode, csIniFilePath);
		}

		if (_tcslen(m_szReferenceID) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"ReferenceID", m_szReferenceID, csIniFilePath);
		}

		if (_tcslen(m_szEngineerName) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"EngineerName", m_szEngineerName, csIniFilePath);
		}

		if (_tcslen(m_szEngineerMobNo) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"EngineerMobNo", m_szEngineerMobNo, csIniFilePath);
		}

		if (_tcslen(m_szCountry) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"Country", m_szCountry, csIniFilePath);
		}

		if (_tcslen(m_szState) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"State", m_szState, csIniFilePath);
		}

		if (_tcslen(m_szCity) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"City", m_szCity, csIniFilePath);
		}

		if (_tcslen(m_szPinCode) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"PinCode", m_szPinCode, csIniFilePath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CopyUserLocDetails");
	}
}

/***************************************************************************************************
*  Function Name  : RegisterForProtection
*  Description    : Function to register installer for protection.
*  Author Name    : Akshay Patil
*  Date           : 5 July 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::RegisterForProtection()
{
	try
	{
		CScannerLoad	objCScanner;
		objCScanner.RegisterProcessId(WLSRV_ID_SEVENTEEN);//WLSRV_ID_SEVENTEEN to register service for process protection

		CSecure64  objCSecure;
		objCSecure.RegisterProcessId(WLSRV_ID_SEVENTEEN);//WLSRV_ID_SEVENTEEN to register service for process protection
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::RegisterForProtection");
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : TerminateInstallThreads
*  Description    : Function to terminate installation threads safely.
*  Author Name    : Akshay Patil
*  Date           : 5 July 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::TerminateInstallThreads()
{
	try
	{
		KillTimer(TIMER_SETFLEPATH_PER);

		if (m_hTmpProcess)
		{
			TerminateProcess(m_hTmpProcess, 0);
			CloseHandle(m_hTmpProcess);
		}
		if (m_hProcess)
		{
			TerminateProcess(m_hProcess, 0);
			CloseHandle(m_hProcess);
		}

		m_hTmpProcess = NULL;
		m_hProcess = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::TerminateInstallThreads");
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : InstallFinishStatus
*  Description    : Function to call install finish status.
*  Author Name    : Akshay Patil
*  Date           : 5 July 2019
****************************************************************************************************/
void CWardWizInstallerDlg::InstallFinishStatus()
{
	try
	{
		m_svInstallFinishFlagCB.call();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::InstallFinishStatus");
	}
}

/***************************************************************************************************
*  Function Name  : CheckDownloadedSetup
*  Description    : Checking if Downloaded Setup Exist for not.
*  Author Name    : Tejas Shinde
*  Date           : 9 July 2019
****************************************************************************************************/
CString  CWardWizInstallerDlg::CheckDownloadedSetup()
{
	TCHAR WWIZSetupExeDesktopPath[MAX_PATH] = { 0 };
	TCHAR csWWizOnlineSetupFinalPath[MAX_PATH] = { 0 };
	TCHAR szFileHash[512] = { 0 };
	TCHAR szSetupUrlInfo[512] = { 0 };
	CString csFilePath, csGetInstallerModuleFilePath;
	CString cProdID = L"0";
	CString csResponse = L"";
	bool bSuccess = true;
	CStringArray objcsaWardWizInstallerProcesses, objcsaWardWizInstallerWithSetupProcesses;

	try
	{
		if (m_bIsWow64 == true)
		{
			m_csSetupBit = L"64";
			switch(m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", ESSPLUSNCG64SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", ESSPLUSNCI64SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", PRONCG64SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", PRONCI64SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3:
				break;
			}
		}
		else
		{
			m_csSetupBit = L"32";
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", ESSPLUSNCG86SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", ESSPLUSNCI86SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2: 
				if (theApp.m_dwLangID == 2)
				{
				   m_csSetupLang = L"de";
				   csFilePath.Format(L"%s", PRONCG86SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", PRONCI86SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3://for elite
				break;
			}
		}
		bSuccess=CheckOfflineInternetConnection();
		if (bSuccess==false)
		{
			csGetInstallerModuleFilePath = m_szWWizInstallerLocationPath;
			if (PathFileExists(csGetInstallerModuleFilePath))
			{
				swprintf_s(m_WWizOfflineSetupFinalPath, L"%s\\%s", csGetInstallerModuleFilePath, csFilePath);
				if (PathFileExists(m_WWizOfflineSetupFinalPath))
					{
						objcsaWardWizInstallerProcesses.Add(L"installerEx86.exe");
						objcsaWardWizInstallerProcesses.Add(L"installerEx64.exe");
						objcsaWardWizInstallerProcesses.Add(L"installerGx86.exe");
						objcsaWardWizInstallerProcesses.Add(L"installerGx64.exe");
						for (int iIndex = 0; iIndex < objcsaWardWizInstallerProcesses.GetCount(); iIndex++)
						{
							CString csinstallerProcessName = objcsaWardWizInstallerProcesses.GetAt(iIndex);
							swprintf_s(m_csWWizOfflineInstallerSetupPath, L"%s\\%s", csGetInstallerModuleFilePath, csinstallerProcessName);
							if (PathFileExists(m_csWWizOfflineInstallerSetupPath))
							{
								break;
							}
						}
					}
				   if (PathFileExists(m_csWWizOfflineInstallerSetupPath) && PathFileExists(m_WWizOfflineSetupFinalPath))
					{
					   AddLogEntry(L"### In CWardwizInstallerDlg::CheckOfflineInstallerSetup for Installer Exe Path (%s) and SetupExe (%s)Exists.", m_csWWizOfflineInstallerSetupPath, m_WWizOfflineSetupFinalPath, true, 0x00);
						return L"success";
					}
					else
					{
						AddLogEntry(L"### In CWardwizInstallerDlg::CheckOfflineInstallerSetup for Installer Exe Path (%s) and SetupExe (%s) Not Exists.", m_csWWizOfflineInstallerSetupPath, m_csWWizSetupFinalPath, true, 0x00);
						return L"failed";
					}
			}

		}
		else if (bSuccess==true)
		{
				AddLogEntry(L">>> internet connection available", 0, 0, true, FIRSTLEVEL);
				swprintf_s(csWWizOnlineSetupFinalPath, L"%s\\%s", m_szTempFolderPath, csFilePath);
				if (PathFileExists(csWWizOnlineSetupFinalPath))
				{
					if (!GetFileHash(csWWizOnlineSetupFinalPath, szFileHash))
					{
						return false;
					}
				}
				cProdID.Format(L"%d", m_dwProductId);
				wsprintf(szSetupUrlInfo, L"http://www.wardwiz.com/version_check.php?md5=%s&prod_id=%s&bit=%s&lang=%s", szFileHash, cProdID, m_csSetupBit, m_csSetupLang);
				WinHttpClient client(szSetupUrlInfo);
				if (!client.SendHttpRequest())
				{
					AddLogEntry(L"### SendHttpRequest %s failed in  CWardwizInstallerDlg::CheckDownloadedSetup", szSetupUrlInfo, 0, true, SECONDLEVEL);
					return L"failed";
				}
				else
				{
					wstring httpResponseContent = client.GetResponseContent();
					csResponse = httpResponseContent.c_str();
					csResponse.Trim();
					return csResponse;
                }
		}
    }
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckDownloadedSetup");
	}
	return csResponse;
}

/***************************************************************************************************
*  Function Name  : CheckAnyUninstallationPorcessIsRunning
*  Description    : Check any AV uninstallation process is running or not
*  Author Name    : Kunal Waghmare
*  Date           : 23 Sept 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::CheckAnyUninstallationPorcessIsRunning()
{
	try{

		DWORD dwExitCode = 0;
		BOOL bRetVal = FALSE;
		int iCntr = 0;
		if (v_hProcess.size() > 0)
		{
			for (vector<HANDLE>::iterator vItr = v_hProcess.begin(); vItr != v_hProcess.end(); ++vItr, iCntr++)
			{
				if (*vItr != NULL)
				{
					bRetVal = GetExitCodeProcess(*vItr, &dwExitCode);
					if (bRetVal == TRUE && dwExitCode == STILL_ACTIVE)
					{
						return json::value(1);
					}
					else if (dwExitCode == 41220)
					{
						//after opening and closing uninstall ui of avast and malwarebyte AV, dwExitCode obtained in
						//GetExitCodeProcess() is 41220
						v_hProcess.erase(v_hProcess.begin() + iCntr);
						iCntr--;
					}
				}
				else
				{
					v_hProcess.erase(v_hProcess.begin() + iCntr);
					iCntr--;
				}
				dwExitCode = 0;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckAnyUninstallationPorcessIsRunning");
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : On_SetProxyServer
*  Description    : Set Proxy details for registration request
*  Author Name    : Akshay Patil
*  Date           : 24 Oct 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::On_SetProxyServer(SCITER_VALUE svArrProxyDetails)
{
	try
	{
		TCHAR szServer[MAX_PATH] = {0};
		bool bIsArray = false;
		m_bIsProxySet = true;

		svArrProxyDetails.isolate();		
		bIsArray = svArrProxyDetails.is_array();

		if (!bIsArray)
		{
			return false;
		}
		
		const SCITER_VALUE EachEntry = svArrProxyDetails[0];

		const std::wstring Server = EachEntry[L"Server"].get(L"");
		wcscpy(szServer, Server.c_str());

		const std::wstring Port = EachEntry[L"Port"].get(L"");
		wsprintf(m_szServer, L"%s:%s", szServer, Port.c_str());

		const std::wstring Username = EachEntry[L"Username"].get(L"");
		wcscpy(m_szUsername, Username.c_str());

		const std::wstring Password = EachEntry[L"Password"].get(L"");
		wcscpy(m_szPassword, Password.c_str());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_SetProxyServer");
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : On_IsCheckOfline
Description    : Function to check ofline registration.
Author Name    : Akshay Patil
Date           : 04 Nov 2019
***********************************************************************************************/
json::value CWardWizInstallerDlg::On_CheckIsOffline()
{
	try
	{
		CITinRegWrapper objReg;
		DWORD dwOffLineValue = 1;
		CString csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();

		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csProdRegKey.GetBuffer(), L"dwIsOffline", dwOffLineValue) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwIsOffline in CWardwizInstallerDlg::On_CheckIsOffline", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (dwOffLineValue == 0)
		{
			m_bIsOffline = true;
		}
		else
		{
			m_bIsOffline = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::On_CheckIsOffline()", 0, 0, true, SECONDLEVEL);
	}
	return m_bIsOffline;
}
/***************************************************************************************************
*  Function Name  : SendInstallerLocationPath
*  Description    : Send Installer Setup Exe Path
*  Author Name    : Tejas Shinde
*  Date           : 12 Nov 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::SendInstallerLocationPath(LPTSTR m_csInstallerLocationPath)
{
	try
	{
		if (!m_csInstallerLocationPath)
		{
			return false;
		}
		m_szWWizInstallerLocationPath = m_csInstallerLocationPath;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SendInstallerLocationPath", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}
/*********************************************************************************************************
*  Function Name  : CheckOfflineInternetConnection
*  Description    : This function is used to check internet connection for Offline Installing Setup
*  Author Name    : Tejas Shinde
*  Date           : 12 Nov 2019
**********************************************************************************************************/
bool CWardWizInstallerDlg::CheckOfflineInternetConnection()
{
	try
	{
		WinHttpClient client(L"http://www.google.com");
		return (client.SendHttpRequest());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckOfflineInternetConnection", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : ReinstallCloseAppMsg
*  Description    : Closeing Already Running Wardwiz Application With Message.
*  Author Name    : Tejas Shinde
*  Date           : 9 Oct 2019
****************************************************************************************************/
bool CWardWizInstallerDlg::ReinstallCloseAppMsg(CString csMessageContent)
{
	try
	{
		bool bISFullPath = false;

		if (!csMessageContent)
		{
			return false;
		}
		m_csAppPath = GetWardWizPathFromRegistry();
		if (_tcslen(m_csAppPath) > 0)
		{
			bISFullPath = true;
		}
		m_svCloseMsg.call((SCITER_STRING)csMessageContent);

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::ReinstallCloseAppMsg", 0, 0, true, SECONDLEVEL);
		return false;
	}
}
/***************************************************************************************************
*  Function Name  : OnCloseRunningWizApp
*  Description    : Closing Already Running Wardwiz Application with using yes/no  Button Result.
*  Author Name    : Tejas Shinde
*  Date           : 9 Oct 2019
****************************************************************************************************/
json::value CWardWizInstallerDlg::OnCloseWWizRunningApp(SCITER_VALUE svGetResult)
{
	try
	{
		DWORD dwLangID = svGetResult.get(0);
		bool bISFullPath = false;

		if (_tcslen(m_csAppPath) > 0)
		{
			bISFullPath = true;
		}

		CString csModulePath = m_csAppPath;

		//CString csAppPath = csModulePath + L"WRDWIZAVUI.EXE";
		CEnumProcess objEnumProcess;

		////While Installation or Uninstalltion if crypt exe is running it should close that exe first
		////Neha Gharge 1st July 2015

		CStringArray objcsaWardWizProcesses;
		objcsaWardWizProcesses.Add(L"VBTRAY.EXE");
		objcsaWardWizProcesses.Add(L"WRDWIZAVUI.EXE");
		objcsaWardWizProcesses.Add(L"VBUI.EXE");
		objcsaWardWizProcesses.Add(L"VBAUTORUNSCN.EXE");
		objcsaWardWizProcesses.Add(L"VBTEMPCLR.EXE");
		objcsaWardWizProcesses.Add(L"VBUSBVAC.EXE");
		objcsaWardWizProcesses.Add(L"VBUSBDETECTUI.EXE");
		objcsaWardWizProcesses.Add(L"VBCRYPT.EXE");
		objcsaWardWizProcesses.Add(L"VBSCANNER.EXE");
		objcsaWardWizProcesses.Add(L"VBUI.EXE");
		//objcsaWardWizProcesses.Add(L"VBCOMMSRV.EXE");//Issue resolved: 0001145

		bool bIsAnyProcTerminated = false;
		bool bIsYesAllReply = false;
		DWORD dwRetMessage = 1;//Bu default YesAll option selected.

		for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
		{
			CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);

			if (objEnumProcess.IsProcessRunning(csProcessName, false, false, false))
			{
				if (!bIsYesAllReply)
				{
					CString csLangID = L"0";
					csLangID.Format(L"%d", dwLangID);

					switch (dwLangID)
					{
					case 1:bIsYesAllReply = true;//YES
						   break;
					}
				}
				if (objEnumProcess.IsProcessRunning(csProcessName, true, false, false))
				{
					bIsAnyProcTerminated = true;
					AddLogEntry(L">>> %s was running, Terminated", csProcessName, 0, true, SECONDLEVEL);
				}
				else
				{
					bIsAnyProcTerminated = true;
					AddLogEntry(L"### %s was running, Failed to terminated", csProcessName, 0, true, SECONDLEVEL);
				}

			}
		}
		//Neha Gharge Unregister com dll 20/4/2015
		CString csAppPath(L"");
		csAppPath = m_csAppPath + L"VBSHELLEXT.DLL";
		if (PathFileExists(csAppPath))
		{
			UnregisterComDll(csAppPath);
		}

		csAppPath = csModulePath + L"VBSHELLEXT_OLD.DLL";
		if (PathFileExists(csAppPath))
		{
			UnregisterComDll(csAppPath);
		}

		//ISSUE NO - 8 After re- installation showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
		//NAME - Niranjan Deshak. - 29th Jan 2015.	
		if (bIsAnyProcTerminated)
		{
			CCommonFunctions objCCommonFunctions;
			objCCommonFunctions.RefreshTaskbarNotificationArea();
		}

		//Dwret is add into addlogentry So that we enable to know the problem while installation
		//Neha Gharge
		//close and remove service here
		CISpySrvMgmt		iSpySrvMgmtObj;
		DWORD dwRet = 0x00;
		CString csFailureCase(L"");

		dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZSERVICENAME);
		csFailureCase.Format(L"%d", dwRet);
		if (dwRet != 0x00)
		{
			AddLogEntry(L"### Unable to Stop Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
		}
		//Issue: 1145: 	Issue with Reinstallation, while reinstallation Retry popup appears after clicking on "Retry" button installation get complete.
		//Resolved By: Nitin Kolapkar
		//Need to put sleep because In inno setup we are not able to find whether Comm service is stopped or not so putting 5 seconds sleep (Approx)
		Sleep(5 * 1000);

		dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME);
		csFailureCase.Format(L"%d", dwRet);
		if (dwRet != 0x00)
		{
			AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
		}
		//Issue: 1145: 	Issue with Reinstallation, while reinstallation Retry popup appears after clicking on "Retry" button installation get complete.
		//Resolved By: Nitin Kolapkar
		//Need to put sleep because In inno setup we are not able to find whether Comm service is stopped or not so putting 5 seconds sleep (Approx)
		Sleep(5 * 1000);

		//close and remove service here
		dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZUPDATESERVICENAME);
		csFailureCase.Format(L"%d", dwRet);
		if (dwRet != 0x00)
		{
			AddLogEntry(L"### Unable to Stop Update Service WardwizALUsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
		}

		dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME);
		csFailureCase.Format(L"%d", dwRet);
		if (dwRet != 0x00)
		{
			AddLogEntry(L"### Unable to UnInstall Update Service WardwizALUsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnCloseRunningWizApp", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************
Function Name  : UnregisterComDll
Description    : Unregister COM Dll
Author Name    : Neha Gharge
S.R. No        :
Date           : 3/28/2015
****************************************************************************/
void CWardWizInstallerDlg::UnregisterComDll(CString csAppPath)
{
	try
	{
		CWardWizOSversion		objOSVersionWrap;
		CString csExePath, csCommandLine;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		csExePath.Format(L"%s\\%s", systemDirPath, L"regsvr32.exe");

		//On xp runas parameter never work It will not unregister the VBSHELLEXT.DLL
		//So NUll parameter send.
		DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
		//Neha Gharge Message box showing of register successful on reinstallation.
		switch (OSType)
		{
		case WINOS_XP:
		case WINOS_XP64:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, NULL, csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		default:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		}

		if (PathFileExists(csAppPath))
		{
			if (!DeleteFile(csAppPath))
			{
				AddLogEntry(L"### DeleteFile Failed for file: %s .", csAppPath, 0, true, SECONDLEVEL);
				if (!MoveFileEx(csAppPath, NULL, MOVEFILE_COPY_ALLOWED))
				{
					AddLogEntry(L"### MoveFileEx Failed for file: %s.", csAppPath, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::UnRegisterComDLL", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckAlreadySetupExit
*  Description    : Checking if Downloaded Setup Exist for not.
*  Author Name    : Tejas Shinde
*  Date           : 17 Dec 2019
****************************************************************************************************/
CString  CWardWizInstallerDlg::CheckAlreadySetupExit()
{
	TCHAR WWIZSetupExeDesktopPath[MAX_PATH] = { 0 };
	TCHAR csWWizOfflineSetupFinalPath[MAX_PATH] = { 0 };
	TCHAR csWWizOnlineSetupFinalPath[MAX_PATH] = { 0 };
	TCHAR szFileHash[512] = { 0 };
	TCHAR szSetupUrlInfo[512] = { 0 };
	CString csFilePath, csGetInstallerModuleFilePath, csMsgBox;
	CString cProdID = L"0";
	CString csResponse = L"";
	bool bSuccess = true;
	CStringArray objcsaWardWizInstallerProcesses, objcsaWardWizInstallerWithSetupProcesses;

	try
	{
		if (m_bIsWow64 == true)
		{
			m_csSetupBit = L"64";
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", ESSPLUSNCG64SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", ESSPLUSNCI64SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", PRONCG64SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", PRONCI64SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3:
				break;
			}
		}
		else
		{
			m_csSetupBit = L"32";
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", ESSPLUSNCG86SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", ESSPLUSNCI86SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					m_csSetupLang = L"de";
					csFilePath.Format(L"%s", PRONCG86SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					m_csSetupLang = L"eng";
					csFilePath.Format(L"%s", PRONCI86SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3://for elite
				break;
			}
		}
		csGetInstallerModuleFilePath = m_szWWizInstallerLocationPath;
		if (PathFileExists(csGetInstallerModuleFilePath))
		{
			swprintf_s(m_csWWizAlreadySetupFinalPath, L"%s\\%s", csGetInstallerModuleFilePath, csFilePath);
			if (PathFileExists(m_csWWizAlreadySetupFinalPath))
			{
				objcsaWardWizInstallerProcesses.Add(L"installerEx86.exe");
				objcsaWardWizInstallerProcesses.Add(L"installerEx64.exe");
				objcsaWardWizInstallerProcesses.Add(L"installerGx86.exe");
				objcsaWardWizInstallerProcesses.Add(L"installerGx64.exe");
				for (int iIndex = 0; iIndex < objcsaWardWizInstallerProcesses.GetCount(); iIndex++)
				{
					CString csinstallerProcessName = objcsaWardWizInstallerProcesses.GetAt(iIndex);
					swprintf_s(m_csWWizAlreadyInstallerSetupPath, L"%s\\%s", csGetInstallerModuleFilePath, csinstallerProcessName);
					if (PathFileExists(m_csWWizAlreadyInstallerSetupPath))
					{
						break;
					}
				}
			}
			if (PathFileExists(m_csWWizAlreadyInstallerSetupPath) && PathFileExists(m_csWWizAlreadySetupFinalPath))
			{
				if (!GetFileHash(m_csWWizAlreadySetupFinalPath, szFileHash))
				{
					return false;
				}
				cProdID.Format(L"%d", m_dwProductId);
				wsprintf(szSetupUrlInfo, L"http://www.wardwiz.com/version_check.php?md5=%s&prod_id=%s&bit=%s&lang=%s", szFileHash, cProdID, m_csSetupBit, m_csSetupLang);
				WinHttpClient client(szSetupUrlInfo);
				if (!client.SendHttpRequest())
				{
					AddLogEntry(L"### SendHttpRequest %s failed in  CWardwizInstallerDlg::CheckDownloadedSetup", szSetupUrlInfo, 0, true, SECONDLEVEL);
					return L"failed";
				}
				else
				{
					wstring httpResponseContent = client.GetResponseContent();
					m_GetOnlineSetupResponse = httpResponseContent.c_str();
					m_GetOnlineSetupResponse.Trim();
					return m_GetOnlineSetupResponse;
				}
			}
			else
			{
				AddLogEntry(L"### In CWardwizInstallerDlg::CheckAlreadySetupExit for Installer Exe Path (%s) and SetupExe (%s) Not Exists.", m_csWWizOfflineInstallerSetupPath, m_csWWizSetupFinalPath, true, 0x00);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckAlreadySetupExit");
	}
	return L"";
}
/***********************************************************************************************************
*  Function Name  : OnGetResponseFinalInstall
*  Description    : This function is used to Get Response to Which Location Setup taken and Install Setup.
*  Author Name    : Tejas Shinde
*  Date           : 17  Dec 2019
************************************************************************************************************/
json::value CWardWizInstallerDlg::OnGetResponseFinalInstall(SCITER_VALUE svGetResponseFinalInstall)
{
	try
	{
		m_svGetResponseFinalInstall = static_cast<DWORD>(svGetResponseFinalInstall.d);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OnGetResponseFinalInstall", 0, 0, true, SECONDLEVEL);
	}

	return true;
}
/***********************************************************************************************
*  Function Name  : GetInstallerLocationWardwizSetup
*  Description    : This function is used to Install Where Installer and Setup Exe Wardwiz Setup
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
*************************************************************************************************/
bool CWardWizInstallerDlg::GetInstallerLocationWardwizSetup()
{
	CString Langname, csWWizSetupFinalPath, csWWizSetupCommndline, csFilePath, csLanguageName;
	TCHAR AlreadyWWIZSetupExeExistPath[MAX_PATH];
	CString				csFailureCase(L"");
	try
	{
		IsWow64();

		if (m_bIsWow64 == true)
		{
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSPLUSNCG64SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSPLUSNCI64SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", PRONCG64SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", PRONCI64SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3:
				break;
			}
		}
		else
		{
			m_csSetupBit = L"32";
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSPLUSNCG86SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSPLUSNCI86SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", PRONCG86SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", PRONCI86SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3://for elite
				break;
			}
		}
		swprintf_s(AlreadyWWIZSetupExeExistPath, L"%s\\%s", m_szWWizInstallerLocationPath, csFilePath);
		if (PathFileExists(m_csWWizAlreadyInstallerSetupPath) && PathFileExists(AlreadyWWIZSetupExeExistPath))
		{
			if (m_bIsWow64 == true)
			{
				csWWizSetupFinalPath.Format(L"%s", AlreadyWWIZSetupExeExistPath);
				csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
				LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
				LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
				InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
			}
			else
			{
				csWWizSetupFinalPath.Format(L"%s", AlreadyWWIZSetupExeExistPath);
				csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
				LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
				LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
				InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::InstallGenXSetup", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/*************************************************************************************
*  Function Name  : OfflineInstallWardwizSetup
*  Description    : This function is used to Offline Install Wardwiz Setup
*  Author Name    : Tejas Shinde
*  Date           : 18 March 2019
*************************************************************************************/
bool CWardWizInstallerDlg::OfflineInstallWardwizSetup()
{
	CString Langname, csWWizSetupFinalPath, csWWizSetupCommndline, csFilePath, csLanguageName;
	TCHAR WWIZSetupExeDesktopPath[MAX_PATH];
	try
	{
		IsWow64();

		if (m_bIsWow64 == true)
		{
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSENTIALNCG64SETUPNAME);//Ess NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSENTIALNCI64SETUPNAME);//Ess NC 64 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", BASICNCG64SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", BASICNCI64SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSPLUSNCG64SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSPLUSNCI64SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", PRONCG64SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", PRONCI64SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3:
				break;
			}
		}
		else
		{
			m_csSetupBit = L"32";
			switch (m_dwProductId)
			{
			case 1:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSENTIALNCG86SETUPNAME);//Ess NC 32 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSENTIALNCI86SETUPNAME);//Ess NC 32 url
				}
				break;
			case 4:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", BASICNCG86SETUPNAME);//Basic NC Germany 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", BASICNCI86SETUPNAME);//Basic NC India 64 url
				}
				break;
			case 5:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", ESSPLUSNCG86SETUPNAME);//Ess plus NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", ESSPLUSNCI86SETUPNAME);//Ess plus NC 64 url
				}
				break;
			case 2:
				if (theApp.m_dwLangID == 2)
				{
					csLanguageName += L"german";
					csFilePath.Format(L"%s", PRONCG86SETUPNAME);//provantage NC 64 url
				}
				else if (theApp.m_dwLangID == 0)
				{
					csLanguageName += L"english";
					csFilePath.Format(L"%s", PRONCI86SETUPNAME);//provantage NC 64 url
				}
				break;
			case 3://for elite
				break;
			}
		}

		if (PathFileExists(m_csWWizOfflineInstallerSetupPath))
		{
			swprintf_s(WWIZSetupExeDesktopPath, L"%s\\%s", m_szWWizInstallerLocationPath, csFilePath);
			if (PathFileExists(WWIZSetupExeDesktopPath))
			{
				if (m_bIsWow64 == true)
				{
					csWWizSetupFinalPath.Format(L"%s", WWIZSetupExeDesktopPath);
					csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
					LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
					LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
					InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
				}
				else
				{
					csWWizSetupFinalPath.Format(L"%s", WWIZSetupExeDesktopPath);
					csWWizSetupCommndline.Format(L"%s%s", L"/verysilent /LANG=", csLanguageName);
					LPTSTR csFinalWWIZSetupExePath = csWWizSetupFinalPath.GetBuffer(csWWizSetupFinalPath.GetLength());
					LPTSTR csFinalWWIZInstallCommandLine = csWWizSetupCommndline.GetBuffer(csWWizSetupCommndline.GetLength());
					InstallWWizSetupThrCommandLine(csFinalWWIZSetupExePath, csFinalWWIZInstallCommandLine);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::OfflineInstallVibraniumSetup", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : ParseVersionString
*  Description    : To parse CString version strings and store it in Array of integers.
*  Author Name    : Niranjan Deshak
*  SR_NO
*  Date           : 01 jan 2015
****************************************************************************************************/
bool CWardWizInstallerDlg::ParseVersionString(int iDigits[4], CString& csVersion)
{
	try
	{
		int iTokenPos = 0;
		csVersion.Insert(0, _T("."));
		CString csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
		int iVersion = _ttoi(csToken);
		int iSubVersion = 0;
		int iCount = 0;

		iDigits[iCount] = iVersion;
		iCount++;
		while ((!csToken.IsEmpty()) && (iCount <= 3))
		{
			csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
			iSubVersion = _ttoi(csToken);
			iDigits[iCount] = iSubVersion;
			iCount++;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::ParseVersionString", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CompareVersions
*  Description    : To compare two products versions.
*  Author Name    : Niranjan Deshak
*  SR_NO		  :
*  Date           : 01 jan 2015
****************************************************************************************************/
int CWardWizInstallerDlg::CompareVersions(int iVersion1[4], int iVersion2[4])
{
	try
	{
		for (int iIndex = 0; iIndex < 4; ++iIndex)
		{
			if (iVersion1[iIndex] < iVersion2[iIndex])
				return -1;
			if (iVersion1[iIndex] > iVersion2[iIndex])
				return 1;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CompareVersions", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************************************************
*  Function Name  : CheckVersionAppMsg
*  Description    : Closeing Already Running Wardwiz Application With Message.
*  Author Name    : Tejas Shinde
*  Date           : 9 Oct 2019
****************************************************************************************************************************************/
bool CWardWizInstallerDlg::CheckVersionAppMsg(CString csFirstMessageContent, CString csSecondMessageContent, CString csAppPathContent)
{
	try
	{
		bool bIsEliteVersion; 
		CString csRegKeyPath(L""), csFirstMsg(L""), csSecondMsg(L""), csFinalFirstMsg, csFinalSecondMsg, csAppPath, csCurrentVersion;
		int iPos = 0;
		bool bISFullPath = false;
		if ((!csFirstMessageContent) && (!csSecondMessageContent) && (!csAppPathContent))
		{
			return false;
		}
		csFirstMsg.Format(L"%s", csFirstMessageContent);
		csSecondMsg.Format(L"%s", csSecondMessageContent);
		csAppPath = csAppPathContent.Tokenize(_T("//"), iPos);
		csAppPath.Trim();
		csAppPathContent.Delete(0, iPos + 1);
		iPos = 0;
		csCurrentVersion = csAppPathContent.Tokenize(_T("//"), iPos);
		csCurrentVersion.Trim();

		m_csAppPath = GetWardWizPathFromRegistry();
		if (_tcslen(m_csAppPath) > 0)
		{
			bISFullPath = true;
		}
		CITinRegWrapper objReg;
		TCHAR szValue[MAX_PATH] = { 0 };
		TCHAR szCurrentVersion[MAX_PATH] = { 0 };
		DWORD dwSize = sizeof(szValue);

		TCHAR szValueAppVersion[MAX_PATH] = { 0 };
		DWORD dwSizeAppVersion = sizeof(szValueAppVersion);
#ifndef _WIN32
		if (objReg.GetRegistryValueData32(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize) == 0)
		{
			MessageBox(theApp.GetMainWnd()->m_hWnd, L"32 bit version of Vibranium is installed on your machine\n Please uninstall 32 bit setup before installing Vibranium 64 bit setup.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
			return false;
		}
#endif
		CString csProduct = L"";
		TCHAR szProductName[MAX_PATH] = { 0 };
		DWORD dwProdID = 0;
		csRegKeyPath = L"SOFTWARE\\Vibranium";
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"dwProductID", dwProdID) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwProductID from %s", csRegKeyPath, 0, true, SECONDLEVEL);;
			CString strOldAppEntry = L"";
			strOldAppEntry.Format(L"%s Antivirus", theApp.g_csRegKeyPath.GetBuffer());
			if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, strOldAppEntry.GetBuffer(), L"dwProductID", dwProdID) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwProductID from %s", strOldAppEntry, 0, true, SECONDLEVEL);;
			}
		}
		switch (dwProdID)
		{
		case ESSENTIAL:
			_tcscpy_s(szProductName, _countof(szProductName), L"ESSENTIAL");
			csProduct = L"WardWiz Essential";
			break;
		case ESSENTIALPLUS:
			_tcscpy_s(szProductName, _countof(szProductName), L"ESSENTIALPLUS");
			csProduct = L"WardWiz Essential Plus";
			break;
		case PRO:
			_tcscpy_s(szProductName, _countof(szProductName), L"PRO");
			csProduct = L"WardWiz Pro";
			break;
		case ELITE:
			_tcscpy_s(szProductName, _countof(szProductName), L"ELITE");
			csProduct = L"WardWiz Elite";
			bIsEliteVersion = true;
			break;
		case BASIC:
			_tcscpy_s(szProductName, _countof(szProductName), L"BASIC");
			csProduct = L"WardWiz Basic";
			break;
		}

		if (csProduct.GetLength() != 0)
		{
			if (_tcscmp(szProductName, csAppPath) != 0)
			{
				csFinalFirstMsg.Format(csFirstMsg, csProduct);
				m_svChkVerMsg.call((SCITER_STRING)csFinalFirstMsg);
			}
		}

		DWORD dwAppKey = objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppVersion", szValueAppVersion, dwSizeAppVersion);
		if (dwAppKey == 0)
		{
			CString csValueAppVersion(szValueAppVersion);

			int iCurrentVersion[4] = { 0 }, iAppVersion[4] = { 0 };
			if ((ParseVersionString(iAppVersion, csValueAppVersion)) && (ParseVersionString(iCurrentVersion, csCurrentVersion)))
			{
				int iRes = CompareVersions(iAppVersion, iCurrentVersion);
				if (iRes == 1)
				{
					csFinalSecondMsg.Format(L"%s", csSecondMsg);
					m_svChkVerMsg.call((SCITER_STRING)csFinalSecondMsg);
				}
				else if (iRes == 0 || iRes == -1)
				{
					if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize) == 0)
					{
						if (bIsEliteVersion)
						{
							return true;
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::CheckVersionAppMsg", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  SendFinishedInstallationsFlag
*  Description    :  Send flag once finished Installations Setup 
*  Author Name    :  Tejas Shinde
*  Date           :  27 Jan 2020
****************************************************************************************************/
bool CWardWizInstallerDlg::SendFinishedInstallationsFlag(DWORD dwInstallFinishFlag)
{
	try
	{
		if (dwInstallFinishFlag == 0)
		{
			m_svSetupInstalledSuccess.call(true);
		}
		else
		{
			InstallFinishStatus();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizInstallerDlg::SendFinishedInstallationsFlag", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}
