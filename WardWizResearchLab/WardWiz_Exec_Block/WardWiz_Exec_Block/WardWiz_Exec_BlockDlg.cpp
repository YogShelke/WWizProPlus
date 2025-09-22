
// WardWiz_Exec_BlockDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWiz_Exec_Block.h"
#include "WardWiz_Exec_BlockDlg.h"
#include "afxdialogex.h"

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

// CWardWiz_Exec_BlockDlg dialog

CWardWiz_Exec_BlockDlg::CWardWiz_Exec_BlockDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWardWiz_Exec_BlockDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWardWiz_Exec_BlockDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edtCtrl);
	DDX_Control(pDX, IDC_LIST1, m_lstCtrl);
}

BEGIN_MESSAGE_MAP(CWardWiz_Exec_BlockDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT1, &CWardWiz_Exec_BlockDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDOK, &CWardWiz_Exec_BlockDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, &CWardWiz_Exec_BlockDlg::OnBnClickedOk2)
END_MESSAGE_MAP()


// CWardWiz_Exec_BlockDlg message handlers

BOOL CWardWiz_Exec_BlockDlg::OnInitDialog()
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

	m_lstCtrl.SetExtendedStyle(LVS_EX_CHECKBOXES);
	m_lstCtrl.InsertColumn(
		0,              // Rank/order of item 
		L"Application Name",          // Caption for this header 
		LVCFMT_CENTER,    // Relative position of items under header 
		387);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWardWiz_Exec_BlockDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWardWiz_Exec_BlockDlg::OnPaint()
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
HCURSOR CWardWiz_Exec_BlockDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWardWiz_Exec_BlockDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CWardWiz_Exec_BlockDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString str = L"";
	HKEY hKey;
	DWORD dwValue = 1;

	m_edtCtrl.GetWindowTextW(str);
	if (str == "")
	{
		MessageBox(L"Please enter a valid name.");
	}
	else
	{
		m_lstCtrl.InsertItem(0, str);
	}

	m_edtCtrl.SetWindowText(_T(""));
	CString csFullPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\" + str;

	LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, csFullPath, 0, KEY_ALL_ACCESS, &hKey);
	if (openRes != ERROR_SUCCESS)
	{
		openRes = RegCreateKeyEx(HKEY_LOCAL_MACHINE, csFullPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
		if (openRes == ERROR_SUCCESS)
		{
			RegSetValueEx(hKey, L"Debug", 0, REG_DWORD, reinterpret_cast<BYTE *>(&dwValue), sizeof(dwValue));
			RegCloseKey(hKey);
			MessageBox(L"Created key successfully!");
			if (!hKey)
			{
				MessageBox(L"Error in creating key.");
			}
		}
	}
}

void CWardWiz_Exec_BlockDlg::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
	int nItem = 0;
	int count = 0;
	for (nItem = m_lstCtrl.GetItemCount(); nItem >= 0; nItem--)
	{
		BOOL bChecked = m_lstCtrl.GetCheck(nItem);
		if (bChecked == 1)
		{
			CString strText = m_lstCtrl.GetItemText(nItem, 0);
			m_lstCtrl.DeleteItem(nItem);
			CString csFullPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\" + strText;
			RegDeleteKey(HKEY_LOCAL_MACHINE, csFullPath);
			count++;
		}
	}
	if (count == 0)
	{
		MessageBox(L"Please select an entry to delete");
	}
	else
	{
		MessageBox(L"Entry deleted successfully!");
	}
}
