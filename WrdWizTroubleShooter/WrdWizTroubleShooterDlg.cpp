
// WrdWizTroubleShooterDlg.cpp : implementation file
/**********************************************************************************************************
Program Name          : WrdWizTroubleShooterDlg.cpp
Description           : To show status of installation of driver process
Author Name           : Neha Gharge
Date Of Creation      : 10/31/2015
Version No            : 1.12.0.0
Special Logic Used    :
Modification Log      :
1. Name    : Description
***********************************************************************************************************/

#include "stdafx.h"
#include "WrdWizTroubleShooter.h"
#include "WrdWizTroubleShooterDlg.h"
#include "afxdialogex.h"
#include "CSecure64.h"
#include "DriverConstants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWrdWizTroubleShooterDlg dialog
#define	WM_ICON_NOTIFY	WM_APP+190
#define SENDMSGINSTALLATIONDRIVER (WM_USER + 150)
#define SENDMSGREGISTEREDDRIVER (WM_USER + 250)
#define SENDMSGSTARTDRIVER (WM_USER + 350)
#define SENDMSGINSTALLCOMPLETED (WM_USER + 450)
#define SENDMSGCLOSETLB (WM_USER + 450)
CWrdWizTroubleShooterDlg * CWrdWizTroubleShooterDlg::m_pThis = NULL;
CISpyCommunicatorServer  g_objCommServer(WWIZ_TROUBLESHOOT_SERVER, CWrdWizTroubleShooterDlg::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));

/***************************************************************************
Function Name  : CWrdWizTroubleShooterDlg
Description    : Constructor
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
CWrdWizTroubleShooterDlg::CWrdWizTroubleShooterDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWrdWizTroubleShooterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pThis = this;
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
void CWrdWizTroubleShooterDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_stStatus);
	//DDX_Text(pDX, IDC_STATIC_STATUS, m_csStatus);
}

/***************************************************************************
Function Name  : BEGIN_MESSAGE_MAP
Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
BEGIN_MESSAGE_MAP(CWrdWizTroubleShooterDlg, CJpegDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(SENDMSGINSTALLATIONDRIVER, OnInstallationDriver)
	ON_MESSAGE(SENDMSGSTARTDRIVER, OnStartDriver)
	ON_MESSAGE(SENDMSGINSTALLCOMPLETED, OnInstallationCompleted)
	ON_MESSAGE(SENDMSGCLOSETLB, OnCloseTLb)
END_MESSAGE_MAP()


// CWrdWizTroubleShooterDlg message handlers
/***************************************************************************
Function Name  : OnInitDialog
Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
				 Foundation Class Library dialog boxes
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
BOOL CWrdWizTroubleShooterDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER);

	this->SetWindowTextW(L"WRDWIZTLB");

	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_TWELVE);  // to register service for process protection

	//run the pipe here
	g_objCommServer.Run();
	//HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));  // Icon to use

	//if (!m_TrayIcon.Create(
	//	NULL,                            // Let icon deal with its own messages
	//	WM_ICON_NOTIFY,                  // Icon notify message to use
	//	_T("WardWiz tlb"),  // tooltip
	//	hIcon,
	//	0,                  // ID of tray icon
	//	FALSE,
	//	0, // balloon tip
	//	_T("Welcome"),               // balloon title
	//	NIIF_INFO,                    // balloon icon
	//	20))                            // balloon timeout
	//{
	//	return -1;
	//}

	//m_TrayIcon.SetMenuDefaultItem(0, TRUE);
	//m_TrayIcon.ShowIcon();

	//m_TrayIcon.SetTooltipText(L"WardWiz TLB");//(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"));

	if (!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_TROUBLESHOOT_BG), _T("JPG")))
	{
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
	Draw();

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	int ixRect = cxIcon - 372;
	int iyRect = cyIcon - 70;

	CRect rect;
	this->GetClientRect(rect);

	//CRgn		 rgn;
	//rgn.CreateRoundRectRgn(rect.left, rect.top, rect.right , rect.bottom , 0, 0);
	//this->SetWindowRgn(rgn, TRUE);

	try
	{
		APPBARDATA abd;
		abd.cbSize = sizeof(abd);

		SHAppBarMessage(ABM_GETTASKBARPOS, &abd);

		switch (abd.uEdge)
		{
		case ABE_TOP:
			SetWindowPos(NULL, ixRect, i, rect.Width(), rect.Height(), SWP_NOREDRAW);
			break;

		case ABE_BOTTOM:
			SetWindowPos(NULL, ixRect, iyRect + 20, rect.Width() - 3, rect.Height() - 3, SWP_NOREDRAW);
			break;

		case ABE_LEFT:
			SetWindowPos(NULL, j, iyRect, rect.Width(), rect.Height(), SWP_NOREDRAW);
			break;

		case ABE_RIGHT:
			SetWindowPos(NULL, ixRect, iyRect, rect.Width(), rect.Height(), SWP_NOREDRAW);
			break;
		}

		
	}
	catch (...)
	{
	}

	LOGFONT lfInstallerTitle;
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 25;
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 10;
	m_BoldText.CreateFontIndirect(&lfInstallerTitle);
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));	 //	   with	face name "Verdana".

	m_stStatus.SetWindowPos(&wndTop, rect.left + 100, 30, 250, 30, SWP_NOREDRAW);
	m_stStatus.SetTextColor(RGB(75, 75, 76));
	m_stStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_TLB_INSTALLING_DRIVER_STATUS"));
	m_stStatus.SetFont(&m_BoldText);
	m_stStatus.SetBkColor(RGB(218, 219, 221));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************
Function Name  : OnPaint
Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft
Foundation Class Library dialog boxes
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/

void CWrdWizTroubleShooterDlg::OnPaint()
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

/***************************************************************************
Function Name  : OnPaint
Description    : The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
HCURSOR CWrdWizTroubleShooterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   : 
****************************************************************************/
int CWrdWizTroubleShooterDlg::GetTaskBarHeight()
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
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumTroubleShooterDlg::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   :
****************************************************************************/
int CWrdWizTroubleShooterDlg::GetTaskBarWidth()
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
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumTroubleShooterDlg::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : OnCtlColor
Description    : The framework calls this member function when a child control is about to be drawn.
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   :
****************************************************************************/
HBRUSH CWrdWizTroubleShooterDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr;
	hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

/***************************************************************************
Function Name  : WindowProc
Description    : 
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   :
****************************************************************************/
LRESULT CWrdWizTroubleShooterDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return CJpegDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************
Function Name  : OnInstallationDriver
Description    : call from ON_MESSAGE to change status on UI
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   :
****************************************************************************/
LRESULT CWrdWizTroubleShooterDlg::OnInstallationDriver(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);
		m_stStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_TLB_INSTALLING_DRIVER_STATUS"));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumTroubleShooterDlg::OnInstallationDriver", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : OnStartDriver
Description    : call from ON_MESSAGE to change status on UI
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   :
****************************************************************************/
LRESULT CWrdWizTroubleShooterDlg::OnStartDriver(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);
		m_stStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_TLB_START_DRIVER_STATUS"));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumTroubleShooterDlg::OnStartDriver", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : OnInstallationCompleted
Description    : call from ON_MESSAGE to change status on UI
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   :
****************************************************************************/
LRESULT CWrdWizTroubleShooterDlg::OnInstallationCompleted(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);
		m_stStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_TLB_INSTALLATION_COMPLETED_STATUS"));
		Sleep(3000);
		CloseUI();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumTroubleShooterDlg::OnInstallationCompleted", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : OnCloseTLb
Description    : call from ON_MESSAGE to close exe.
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   :
****************************************************************************/
LRESULT CWrdWizTroubleShooterDlg::OnCloseTLb(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);
		CloseUI();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumTroubleShooterDlg::OnCloseTLb", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : CloseUI
Description    : Close User interface
Author Name    : Neha Gharge
Date           : 31/10/2015
SR_NO		   :
****************************************************************************/
void CWrdWizTroubleShooterDlg::CloseUI()
{
	try
	{
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumTroubleShooterDlg::CloseUI", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	PreTranslateMessage
*  Description    : To translate window messages before they are dispatched
to the TranslateMessage and DispatchMessage Windows functions
*  SR.N0		  :
*  Author Name    :	Neha Gharge
*  Date           : 31/10/2015
**********************************************************************************************************/
BOOL CWrdWizTroubleShooterDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : OnDataReceiveCallBack
Description    : It receive flag send from named pipe
Author Name    : Neha Gharge
SR_NO		   :
Date           : 5 Nov,2015
****************************************************************************/
void CWrdWizTroubleShooterDlg::OnDataReceiveCallBack(LPVOID lpParam)
{
	__try
	{
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;

		if (!lpSpyData)
			return;

		switch (lpSpyData->iMessageInfo)
		{
		case SHOWSTATUSOFINSTALLATIONDRIVER:
			m_pThis->ShowDriverStatus(lpSpyData->dwValue);
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumTroubleShooterDlg::OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

void CWrdWizTroubleShooterDlg::ShowDriverStatus(DWORD dwMsg)
{
	if (dwMsg == SENDMSGINSTALLATIONDRIVER)
	{
		m_pThis->m_stStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_TLB_INSTALLING_DRIVER_STATUS"));
	}
	else if (dwMsg == SENDMSGSTARTDRIVER)
	{
		m_pThis->m_stStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_TLB_START_DRIVER_STATUS"));
	}
	else
	{
		m_pThis->m_stStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_TLB_INSTALLATION_COMPLETED_STATUS"));
		Sleep(3000);
		m_pThis->CloseUI();
	}
}