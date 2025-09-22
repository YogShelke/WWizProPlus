
// PackageInstallerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PackageInstaller.h"
#include "PackageInstallerDlg.h"
#include "afxdialogex.h"
#include "Connection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
END_MESSAGE_MAP()


// CPackageInstallerDlg dialog



CPackageInstallerDlg::CPackageInstallerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPackageInstallerDlg::IDD, pParent)
	, m_csUserName(_T(""))
	, m_csPassword(_T(""))
	, m_csRemoteIP(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPackageInstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_REMOTE_IP, m_editRemoteIP);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_csUserName);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_csPassword);
	DDX_Text(pDX, IDC_EDIT_REMOTE_IP, m_csRemoteIP);
}

BEGIN_MESSAGE_MAP(CPackageInstallerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INSTALL, &CPackageInstallerDlg::OnBnClickedButtonInstall)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CPackageInstallerDlg::OnBnClickedButtonCancel)
END_MESSAGE_MAP()


// CPackageInstallerDlg message handlers

BOOL CPackageInstallerDlg::OnInitDialog()
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

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPackageInstallerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPackageInstallerDlg::OnPaint()
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
HCURSOR CPackageInstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPackageInstallerDlg::OnBnClickedButtonInstall()
{
	UpdateData(TRUE);

	try
	{
		CString csMachineID;
		CString csMachineIPAddress = m_csRemoteIP;
		CString csMachineName = GetMachineNameByIP(m_csRemoteIP);

		CString sMsg;
		sMsg.Format(L"[%s] Checking connection!", csMachineName);
		OutputDebugString(sMsg);

		CConnection oConnection(csMachineIPAddress, csMachineName, L"", AfxGetMainWnd()->m_hWnd, false);
		oConnection.m_csUserName = m_csUserName;
		oConnection.m_csPassword = m_csPassword;
		if (!oConnection.EstablishAdminConnection())
		{
			MessageBox(L"Connection failed", L"WardWiz", MB_ICONINFORMATION);
			return;
		}

		if (!oConnection.InstallClient(csMachineID))
		{
			MessageBox(L"InstallClient failed", L"WardWiz", MB_ICONINFORMATION);
			return;
		}

		sMsg.Format(L"[%s] Client Installed successfully!", csMachineName);
		MessageBox(sMsg, L"WardWiz", MB_ICONINFORMATION);

		OutputDebugString(sMsg);
	}
	catch (...)
	{
	}
	return;
}


void CPackageInstallerDlg::OnBnClickedButtonCancel()
{
	OnCancel();
}


CString CPackageInstallerDlg::GetMachineNameByIP(CString csIPAddress)
{
	CString csMachineName(csIPAddress);
	unsigned int addr;
	addr = inet_addr((CStringA)csIPAddress);
	HOSTENT *lpHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	if (lpHost)
		csMachineName = lpHost->h_name;

	return csMachineName;
}