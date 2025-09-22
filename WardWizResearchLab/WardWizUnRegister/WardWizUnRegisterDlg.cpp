
// WardWizUnRegisterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWizUnRegister.h"
#include "WardWizUnRegisterDlg.h"
#include "iTINRegWrapper.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WRDWIZEVALREG	L"VBUSERREG.DB"

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


// CWardWizUnRegisterDlg dialog



CWardWizUnRegisterDlg::CWardWizUnRegisterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWardWizUnRegisterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWardWizUnRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_UNREGISTER, m_btnUnRegister);
}

BEGIN_MESSAGE_MAP(CWardWizUnRegisterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ABOUT_WARDWIZ, &CWardWizUnRegisterDlg::OnBnClickedButtonAboutWardwiz)
	ON_BN_CLICKED(IDC_BUTTON_UNREGISTER, &CWardWizUnRegisterDlg::OnBnClickedButtonUnregister)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CWardWizUnRegisterDlg::OnBnClickedButtonCancel)
END_MESSAGE_MAP()


// CWardWizUnRegisterDlg message handlers

BOOL CWardWizUnRegisterDlg::OnInitDialog()
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

void CWardWizUnRegisterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWardWizUnRegisterDlg::OnPaint()
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
HCURSOR CWardWizUnRegisterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWardWizUnRegisterDlg::OnBnClickedButtonAboutWardwiz()
{
	CAboutDlg objAbout;
	objAbout.DoModal();
}

void CWardWizUnRegisterDlg::OnBnClickedButtonUnregister()
{
	bool bReturn = true;

	if (!RemoveRegistry())
	{
		bReturn = false;
	}
	
	if(!RemoveFilesFromHardDrive())
	{
		bReturn = false;
	}

	if (!RemoveFromEvalRegDLL())
	{
		bReturn = false;
	}

	if (bReturn)
	{
		MessageBox(L"WardWiz Unregistered successfully", L"WardWiz", MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(L"WardWiz Unregistration failed, please contact wardwiz support", L"WardWiz", MB_ICONEXCLAMATION);
	}
}

void CWardWizUnRegisterDlg::OnBnClickedButtonCancel()
{
	OnCancel();
}

bool CWardWizUnRegisterDlg::RemoveRegistry()
{
	bool bReturn = false;
	
	CITinRegWrapper g_objReg;
	if (g_objReg.DelRegistryValueName(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows", L"VibraniumUserInfo")  > 0x02)
	{
		AddLogEntry(L"REMOVAL FAILED REGISTRY: [%s]", L"SOFTWARE\\Microsoft\\Windows\\VibraniumUserInfo", 0, true, ZEROLEVEL);
		return bReturn;
	}

	bReturn = true;

	return bReturn;
}

bool CWardWizUnRegisterDlg::RemoveFilesFromHardDrive()
{
	bool bReturn = true;

	TCHAR	szDestin[512] = { 0 };
	TCHAR	szAllUserPath[512] = { 0 };
	GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 511);

	OSVERSIONINFO 	OSVer = { 0 };
	OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&OSVer);

	if (OSVer.dwMajorVersion > 5)
	{
		wsprintf(szDestin, L"%s\\VIBRANIUM\\%s", szAllUserPath, WRDWIZEVALREG);
	}
	else
	{
		wsprintf(szDestin, L"%s\\Application Data\\VIBRANIUM\\%s", szAllUserPath, WRDWIZEVALREG);
	}

	if (PathFileExists(szDestin))
	{
		SetFileAttributes(szDestin, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(szDestin))
		{
			AddLogEntry(L"REMOVAL FAILED FILE: [%s]", szDestin, 0, true, ZEROLEVEL);
			bReturn = false;
		}
	}

	if (OSVer.dwMajorVersion > 5)
	{
		wsprintf(szDestin, L"%s\\Wardwiz Antivirus\\%s", szAllUserPath, WRDWIZEVALREG);
	}
	else
	{
		wsprintf(szDestin, L"%s\\Application Data\\VIBRANIUM\\%s", szAllUserPath, WRDWIZEVALREG);
	}

	if (PathFileExists(szDestin))
	{
		SetFileAttributes(szDestin, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(szDestin))
		{
			AddLogEntry(L"REMOVAL FAILED FILE: [%s]", szDestin, 0, true, ZEROLEVEL);
			bReturn = false;
		}
	}

	wsprintf(szDestin, L"%s%s", L"C:\\Program Files\\VIBRANIUM\\", WRDWIZEVALREG);
	if (PathFileExists(szDestin))
	{
		SetFileAttributes(szDestin, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(szDestin))
		{
			AddLogEntry(L"REMOVAL FAILED FILE: [%s]", szDestin, 0, true, ZEROLEVEL);
			bReturn = false;
		}
	}

	TCHAR	szPath[512] = { 0 };
	TCHAR	szDrives[256] = { 0 };
	GetLogicalDriveStrings(255, szDrives);

	TCHAR	*pDrive = szDrives;

	while (wcslen(pDrive) > 2)
	{
		memset(szPath, 0x00, 512 * sizeof(TCHAR));
		wsprintf(szPath, L"%sVBUSERREG.DB", pDrive);

		if (PathFileExists(szPath))
		{
			SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);
			if(!DeleteFile(szPath))
			{
				AddLogEntry(L"REMOVAL FAILED FILE: [%s]", szDestin, 0, true, ZEROLEVEL);
			}
		}
		pDrive += 4;
	}
	return bReturn;
}

bool CWardWizUnRegisterDlg::RemoveFromEvalRegDLL()
{
	try
	{
		TCHAR	pDllPath[512] = { 0 };
		TCHAR	*pName = NULL;

		AVACTIVATIONINFO	ActInfo = {0};

		wsprintf(pDllPath, TEXT("%sVBEVALREG.DLL"), L"C:\\Program Files\\VIBRANIUM\\");
		if (!PathFileExists(pDllPath))
			return false;

		HANDLE	hUpdateRes = NULL;

		hUpdateRes = BeginUpdateResource(pDllPath, FALSE);
		if (!hUpdateRes)
		{
			return false;
		}

		if (!UpdateResource(hUpdateRes, MAKEINTRESOURCE(0x00), L"REGDATA",
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			&ActInfo, sizeof(AVACTIVATIONINFO)))
		{
			return false;
		}

		if (!UpdateResource(hUpdateRes, MAKEINTRESOURCE(0x7D0), L"REGDATA",
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			&ActInfo, sizeof(AVACTIVATIONINFO)))
		{
			return false;
		}

		if (!EndUpdateResource(hUpdateRes, FALSE))
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardWizValCheck RemoveFromEvalRegDLL", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

