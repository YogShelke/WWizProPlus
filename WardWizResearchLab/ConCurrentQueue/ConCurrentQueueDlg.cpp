
// ConCurrentQueueDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConCurrentQueue.h"
#include "ConCurrentQueueDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CConCurrentQueueDlg::CConCurrentQueueDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CConCurrentQueueDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CConCurrentQueueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CNCLIST, m_lstCnClist);
	DDX_Control(pDX, IDC_EDIT1, m_editNumbers);
	DDX_Control(pDX, IDC_BUTTON1, m_btnAdd);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_btnSave);
}

BEGIN_MESSAGE_MAP(CConCurrentQueueDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CConCurrentQueueDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CConCurrentQueueDlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CConCurrentQueueDlg message handlers

BOOL CConCurrentQueueDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_onAccessQueue.Load(L"D:\\a.txt");

	StartThreads();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CConCurrentQueueDlg::OnPaint()
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
HCURSOR CConCurrentQueueDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CConCurrentQueueDlg::OnBnClickedButton1()
{
	CString csCount;
	m_editNumbers.GetWindowText(csCount);

	DWORD dwCount = _wtoi(csCount);

	for (DWORD dwIndex = 0x00; dwIndex < dwCount; dwIndex++)
	{
		CString csCount;
		csCount.Format(L"Value: %d", dwIndex);
		m_onAccessQueue.push(csCount.GetBuffer());
	}
}

void ProcessScanQueueThread(CConCurrentQueue<std::wstring>& pObjeOnAccess)
{
	return;

	while (true)
	{
		auto item = pObjeOnAccess.pop();
		OutputDebugString(L"Out");
		OutputDebugString(item.c_str());
	}
}

void CConCurrentQueueDlg::StartThreads()
{
	
	DWORD dwThreadCount = SCANNER_DEFAULT_THREAD_COUNT;
	//  Create specified number of threads.
	for (DWORD iThreadNum = 0; iThreadNum < dwThreadCount; iThreadNum++) {

		m_Threads[iThreadNum] = std::thread(ProcessScanQueueThread, std::ref(m_onAccessQueue));
		SetThreadPriority(m_Threads[iThreadNum].native_handle(), THREAD_PRIORITY_HIGHEST);
		//ProcessingThread.join();
	}
}

void CConCurrentQueueDlg::OnBnClickedButtonSave()
{
	m_onAccessQueue.Save(L"D:\\a.txt");
}
