#include "stdafx.h"
#include "WardwizLaunch.h"
#include "WardwizLaunchDlg.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
	
#define WARDWIZBASICSETUP32		"WardWizBasicSetupx86.exe"
#define WARDWIZBASICSETUP64		"WardWizBasicSetupx64.exe"
#define WARDWIZESSNTSETUP32		"WardWizEssentialSetupx86.exe"
#define WARDWIZESSNTSETUP64		"WardWizEssentialSetupx64.exe"
#define WARDWIZPROSETUP32		"WardWizProSetupx86.exe"
#define WARDWIZPROSETUP64		"WardWizProSetupx64.exe"
#define WARDWIZESSPLUSNTSETUP32		"WardWizEssPlusSetupx86.exe"
#define WARDWIZESSPLUSNTSETUP64		"WardWizEssPlusSetupx64.exe"

DWORD WINAPI LaunchingThread(LPVOID lpvThreadParam);
DWORD WINAPI HideLaunchedGIFThread(LPVOID lpvThreadParam);


CWardwizLaunchDlg::CWardwizLaunchDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWardwizLaunchDlg::IDD, pParent)
	,m_hThread_Launch(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWardwizLaunchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_GIF_LAUNCH, m_stlaunchGif);
	DDX_Control(pDX, IDC_STATIC_EXTRACT_TEXT, m_stExtractSetupText);
	DDX_Control(pDX, IDC_STATIC_WAIT_TEXT, m_stWaitText);
}

BEGIN_MESSAGE_MAP(CWardwizLaunchDlg, CJpegDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CWardwizLaunchDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ModifyStyleEx( WS_EX_APPWINDOW, WS_EX_TOOLWINDOW ); 

	if(!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_LAUNCH_IMAGE), _T("JPG")))
	{
		::MessageBox(NULL,L"Failed to load",L"Vibranium", MB_ICONERROR);
	}

	Draw();
	//SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER);

	this->GetClientRect(m_rect);
	CRgn		 rgn;
	//Nitin K. 18th July 2014
	rgn.CreateRectRgn(m_rect.top, m_rect.left ,m_rect.Width()-2,m_rect.Height()-2);
	this->SetWindowRgn(rgn, TRUE);

	m_stlaunchGif.Load(MAKEINTRESOURCE(IDR_GIF_LAUNCH),_T("GIF"));
	if(!m_stlaunchGif.IsPlaying())
	{
		m_stlaunchGif.Draw();
	}
	//Nitin K. 18th July 2014
	CFont *fontExtractText = new CFont;
	fontExtractText->CreatePointFont(100,L"Verdana",0);

	m_stExtractSetupText.SetWindowPos(&wndTop, 0,196,500,40, SWP_NOREDRAW);
	m_stExtractSetupText.SetWindowTextW(L"Extracting setup...");
	m_stExtractSetupText.SetFont(fontExtractText);
	m_stExtractSetupText.ModifyStyle(SS_LEFT,SS_CENTER);

	m_stlaunchGif.SetWindowPos(&wndTop, m_rect.left + 142,226,500,50, SWP_NOREDRAW);
	m_stlaunchGif.ShowWindow(SW_SHOW);
	
	
//	m_stWaitText.SetWindowPos(&wndTop, (m_rect.Width()/2)- 120,260,400,40, SWP_NOREDRAW);
	m_stWaitText.SetWindowPos(&wndTop,0,260,500,40, SWP_NOREDRAW);
	//m_stWaitText.SetTextAlign(1);
	m_stWaitText.SetWindowTextW(L"Please wait for a moment");
	m_stWaitText.SetFont(fontExtractText);
	m_stWaitText.ModifyStyle(SS_LEFT,SS_CENTER);

	LaunchUI();

	//IsWow64();

	//OnCancel();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWardwizLaunchDlg::LaunchUI()
{
	m_hThread_Launch = NULL;
	DWORD m_dwThreadId = 0;
	m_hThread_Launch = ::CreateThread(NULL, 0, LaunchingThread, (LPVOID) this, 0, &m_dwThreadId);
	Sleep(1000);

	m_hThread_Hide = NULL;
	DWORD m_dwHideThreadId = 0;
	m_hThread_Hide = ::CreateThread(NULL, 0, HideLaunchedGIFThread, (LPVOID) this, 0, &m_dwHideThreadId);
	Sleep(500);
	
}
void CWardwizLaunchDlg::OnPaint()
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

HCURSOR CWardwizLaunchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWardwizLaunchDlg::IsWow64()
{

	m_bIsWow64 = false;

	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL) ;

	LPFN_ISWOW64PROCESS		IsWow64Process = NULL ;

	OSVERSIONINFO	OSVI = {0} ;

	OSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
	GetVersionEx(&OSVI) ;

	IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle( TEXT("kernel32")),
															"IsWow64Process") ;
	if(!IsWow64Process)
	{
		AfxMessageBox(L"Failed to get module handle IsWow64Process from kernel32");
		return;
	}
	IsWow64Process(GetCurrentProcess(), &m_bIsWow64) ;
	//LaunchOSSetup();
}

void CWardwizLaunchDlg::LaunchOSSetup()
{

	static CString csFilePath;
	TCHAR szTemp[512] = { 0 };
	TCHAR szModulePath[MAX_PATH] = {0};
	if(!GetModulePath(szModulePath, MAX_PATH))
	{
		MessageBox(L"Error in GetModulePath", L"Vibranium", MB_ICONERROR|MB_OK);
		return;
	}
	csFilePath = szModulePath;
	csFilePath += _T("\\");

#ifdef WRDWIZBASIC
	if(m_bIsWow64)
	{
		csFilePath += WARDWIZBASICSETUP64;
	}
	else 
	{
		csFilePath += WARDWIZBASICSETUP32;
	}
#elif WRDWIZESSNL
	if(m_bIsWow64)
	{
		csFilePath += WARDWIZESSNTSETUP64;
	}
	else 
	{
		csFilePath += WARDWIZESSNTSETUP32;
	}
#elif WRDWIZPRO
	if(m_bIsWow64)
	{
		csFilePath += WARDWIZPROSETUP64;
	}
	else 
	{
		csFilePath += WARDWIZPROSETUP32;
	}
#elif WRDWIZESSPLUS
	if(m_bIsWow64)
	{
		csFilePath += WARDWIZESSPLUSNTSETUP64;
	}
	else
	{
		csFilePath += WARDWIZESSPLUSNTSETUP32;
	}
#endif

	if(!PathFileExists(csFilePath))
	{
		CString csFilePath;
		csFilePath.Format(L"%s Setup file unavailable\nPlease contact to Vibranium Support team", csFilePath);
		MessageBox(csFilePath, L"Vibranium", MB_ICONEXCLAMATION);
		return;
	}

	swprintf_s(szTemp, _countof(szTemp), L"\" /c %s\"", csFilePath);

	AddLogEntry(L">>> %s Path is execute", szTemp, 0, true, SECONDLEVEL);
	LaunchProcessThrCommandLine(szTemp);

	//ShellExecute(NULL, L"open", csFilePath , NULL, NULL, SW_SHOW);

}

bool CWardwizLaunchDlg::GetModulePath(TCHAR *szModulePath, DWORD dwSize)
{
	if(0 == GetModuleFileName(NULL, szModulePath, dwSize))
	{
		return false;
	}

	if(_tcsrchr(szModulePath, _T('\\')))
	{
		*_tcsrchr(szModulePath, _T('\\'))= 0;
	}
	return true;
}

DWORD WINAPI LaunchingThread(LPVOID lpvThreadParam)
{
	CWardwizLaunchDlg *pThis = (CWardwizLaunchDlg*)lpvThreadParam;
	if(!pThis)
		return 0;
	//Nitin K. 18th July 2014
	int iFindWCount = 0;

	pThis->IsWow64();

	pThis->LaunchOSSetup();

	pThis->ShutDown();
	return 1;
}

DWORD WINAPI HideLaunchedGIFThread(LPVOID lpvThreadParam)
{
	CWardwizLaunchDlg *pThis = (CWardwizLaunchDlg*)lpvThreadParam;
	if (!pThis)
		return 0;

	HWND hWindow = ::FindWindow(NULL,L"Select Setup Language");// English OS
	if (!hWindow)
	{
		hWindow = ::FindWindow(NULL, L"Setup-Sprache auswählen");
	}

	if (hWindow)
	{
		pThis->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER);
		pThis->ShowWindow(SW_HIDE);
		return 1;
	}

	while(!hWindow)
	{
		hWindow = ::FindWindow(NULL,L"Select Setup Language");//German OS
		if (!hWindow)
		{
			hWindow = ::FindWindow(NULL, L"Setup - Vibranium");
			if (!hWindow)
			{
				hWindow = ::FindWindow(NULL, L"Setup-Sprache auswählen");
			}
			
		}
		if (hWindow)
		{
			pThis->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER);
			pThis->ShowWindow(SW_HIDE);
		}
		Sleep(500);
	}
	return 1;
}

void CWardwizLaunchDlg::ShutDown()
{
	if(m_stlaunchGif.IsPlaying())
	{
		m_stlaunchGif.Stop();
	}
	m_stlaunchGif.ShowWindow(SW_HIDE);

	if (m_hThread_Hide != NULL)
	{
		SuspendThread(m_hThread_Hide);
		TerminateThread(m_hThread_Hide, 0x00);
	}
	Sleep(100);
	OnCancel();
}

LRESULT CWardwizLaunchDlg::OnNcHitTest(CPoint point)
{
	//Nitin K. 18th July 2014
	return HTCLIENT; // HTCLIENT : because we wanted dialog to come front on click and it should not be movable also
}

/***************************************************************************
  Function Name  : OnCtlColor
  Description    : The framework calls this member function when a child control is about to be drawn.
  Author Name    : Microsoft
  Date           : 18th July 2014
****************************************************************************/

HBRUSH CWardwizLaunchDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID ==  IDC_STATIC_EXTRACT_TEXT  || ctrlID == IDC_STATIC_WAIT_TEXT  )

	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} 
	return hbr;
}


BOOL CWardwizLaunchDlg::LaunchProcessThrCommandLine(LPTSTR pszCmdLine)
{

	STARTUPINFO			si = { 0 };
	PROCESS_INFORMATION	pi = { 0 };

	try
	{

		si.cb = sizeof(STARTUPINFO);

		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW;
		bool bIsBatchFileRunning = true;
		if (bIsBatchFileRunning == true)
		{
			bIsBatchFileRunning = false;
			TCHAR systemDirPath[MAX_PATH] = _T("");
			GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

			TCHAR commandLine[2 * MAX_PATH + 16] = _T("");

			swprintf_s(commandLine, _countof(commandLine), L"\"%s\\cmd.exe\" %s ", systemDirPath, pszCmdLine);


			if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
			{
				AddLogEntry(L"### Failed for  CVibraniumLaunchDlg::LaunchProcessThrCommandLine::%s", pszCmdLine);
				return TRUE;
			}
		}
		else
		{

			if (!CreateProcess(NULL, pszCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
			{
				AddLogEntry(L"### Failed for  CVibraniumLaunchDlg::LaunchProcessThrCommandLine::%s", pszCmdLine);
				return TRUE;
			}
		}


		//if( !CreateProcess(NULL, pszCmdLine, NULL, NULL, TRUE,	CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) )
		//	{
		//		AddLogEntry(L"### Failed for CVibraniumALUpdDlg::RebuildSolution::%s", pszCmdLine);
		//		return TRUE;
		//	}

		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumLaunchDlg::LaunchProcessThrCommandLine");
		return TRUE;
	}

	return FALSE;
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : To translate window messages before they are dispatched
to the TranslateMessage and DispatchMessage Windows functions
Author Name    : Nitin K.
Date           : 1st March 2016
****************************************************************************/
BOOL CWardwizLaunchDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

