
// WardWiz_RecoverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWiz_Recover.h"
#include "WardWiz_RecoverDlg.h"
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


// CWardWiz_RecoverDlg dialog


CWardWiz_RecoverDlg::CWardWiz_RecoverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWardWiz_RecoverDlg::IDD, pParent)
	, m_szFilePath(_T(""))
	, m_bIsRecoverStart(false)
	, m_RecoverStatus(_T(""))
	//, m_RecFileCount(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWardWiz_RecoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_lstFoundEntries);
	DDX_Control(pDX, IDC_EDIT1, m_editFilePath);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_editStatus);
	DDX_Control(pDX, IDC_STATIC_FILE_CNT, m_RecoverFileCount);
}

BEGIN_MESSAGE_MAP(CWardWiz_RecoverDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CWardWiz_RecoverDlg::OnBnClickRecover)
	ON_BN_CLICKED(IDC_BUTTON2, &CWardWiz_RecoverDlg::OnBnClickBrowse)
	ON_BN_CLICKED(IDC_BUTTON3, &CWardWiz_RecoverDlg::OnBnClickCancel)
END_MESSAGE_MAP()


// CWardWiz_RecoverDlg message handlers

BOOL CWardWiz_RecoverDlg::OnInitDialog()
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
	CRect oRcRect;
	/*m_lstFoundEntries.GetWindowRect(&oRcRect);
	m_lstFoundEntries.SetExtendedStyle(LVS_EX_CHECKBOXES);
	m_lstFoundEntries.InsertColumn(1, L"Threat Name", LVCFMT_CENTER, 349);
	m_lstFoundEntries.InsertColumn(2, L"Action", LVCFMT_CENTER, 127);*/

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWardWiz_RecoverDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWardWiz_RecoverDlg::OnPaint()
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
HCURSOR CWardWiz_RecoverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//Recover Button
void CWardWiz_RecoverDlg::OnBnClickRecover()
{
	bool bDeleteBackupFile = false;
	DWORD dwRet, dwStatus = 0x00;
	TCHAR csSelectedPath[260];
	CString csFileName;
	CString csFolderPath;
	CString csSaveFileName;
	
	CString csDefaultExtension;
	//csDefaultExtension = "exe";

	//save the file
	if (m_bIsRecoverStart == true)
	{
		CFileDialog dlg(FALSE, csDefaultExtension, NULL, OFN_OVERWRITEPROMPT, L"All files|*.*||");
		if (dlg.DoModal() != IDOK)
		{
			//the user didn't click OK
			return;
		}
		csSaveFileName = dlg.GetPathName();
	}
	else
	{
		CString csSelectFilePath(L"");
		m_editFilePath.GetWindowTextW(csSelectFilePath);

		if (csSelectFilePath.GetLength() == 0)
			m_editStatus.SetWindowText(L"Pleas select file for recover.");
		else if (!PathFileExists(csSelectFilePath))
			m_editStatus.SetWindowText(L"Pleas select valid file for recover.");
		return;
	}
	wcscpy_s(csSelectedPath, 260, m_szFilePath);
	wcscpy_s(m_szOriginalThreatPath, 260, csSaveFileName);

	dwRet = Decrypt_File(csSelectedPath, m_szOriginalThreatPath, dwStatus, bDeleteBackupFile);
}

//Browse button
void CWardWiz_RecoverDlg::OnBnClickBrowse()
{
	try
	{

		m_editStatus.SetWindowText(L"");  //clear the status
		m_RecoverFileCount.SetWindowText(L""); //clear recover file count
		
		static TCHAR szEncFilter[] = L"All Files(*.*)|*.*|";
		CFileDialog FileDlg(TRUE, L"All Files(*.*)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
		if (FileDlg.DoModal() == IDOK)
		{
			CString csFilePath = FileDlg.GetPathName();
			m_szFilePath = FileDlg.GetPathName();
			m_editFilePath.SetWindowText(csFilePath);
			m_bIsRecoverStart = true;
			m_editStatus.SetWindowText(L"");
		}
		else
		{
			m_bIsRecoverStart = false;
			m_editFilePath.SetWindowText(L"");
		}
	}
	catch (...)
	{	}

}

DWORD CWardWiz_RecoverDlg::Decrypt_File(TCHAR* szRecoverFilePath, TCHAR* szOriginalThreatPath, DWORD &dwStatus, bool bDeleteBackupFile)
{
	DWORD	dwRet = 0x00;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00;
	TCHAR	szTemp[1024] = { 0 };
	TCHAR	szExt[16] = { 0 };
	DWORD	dwLen = 0x00;
	LPBYTE	lpFileData = NULL;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE;
	DWORD FlagToCreateFile;
	bool bAccessflag = false;
	__try
	{
		if (!PathFileExists(szRecoverFilePath))
		{
			//AfxMessageBox( TEXT("Please select file for operation") ) ;
			dwRet = 0x01;
			goto Cleanup;
		}

		::SetFileAttributes(szRecoverFilePath, FILE_ATTRIBUTE_NORMAL);
		hFile = CreateFile(szRecoverFilePath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		if (!dwFileSize)
		{
			CloseHandle(hFile);
			dwRet = 0x03;
			goto Cleanup;
		}

		//	lpFileData = (LPBYTE ) malloc( dwFileSize + 1 ) ;
		lpFileData = (LPBYTE)malloc(dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE));
		if (!lpFileData)
		{
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}

		memset(lpFileData, 0x00, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)));
		SetFilePointer(hFile, (0x00 + (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), NULL, FILE_BEGIN);
		ReadFile(hFile, lpFileData, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), &dwBytesRead, NULL);
		if ((dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) != dwBytesRead)
		{
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}
		//read key from file
		if (!ReadKeyFromEncryptedFile(hFile))
		{
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}
		CloseHandle(hFile);

		if (DecryptData((LPBYTE)lpFileData, dwBytesRead))
		{
			dwRet = 0x05;
			goto Cleanup;
		}

		wcscpy_s(szTemp,1024, szOriginalThreatPath);
		//szTemp[ dwLen-0x05 ] = '\0' ;
		if (PathFileExists(szTemp))
		{
			SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
			if (!(DeleteFile(szTemp)))
			{
				//AddLogEntry(L"### Delete existing %s File ", szTemp, 0, true, SECONDLEVEL);
				FlagToCreateFile = OPEN_EXISTING;
				bAccessflag = true;
			}
			if (bAccessflag == false)
			{
				FlagToCreateFile = OPEN_ALWAYS;
			}
		}
		else
		{
			if (PathIsNetworkPath(szTemp))
			{
				SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
				if (!(DeleteFile(szTemp)))
				{
					int err = GetLastError();
					//AddLogEntry(L"### Delete existing %s File ", szTemp, 0, true, SECONDLEVEL);
					FlagToCreateFile = OPEN_EXISTING;
					bAccessflag = true;
				}

			}
			if (bAccessflag == false)
			{
				FlagToCreateFile = OPEN_ALWAYS;
			}
		}

		::SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
		hFileEnc = CreateFile(szTemp, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			FlagToCreateFile, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileEnc == INVALID_HANDLE_VALUE)
		{
			int err = GetLastError();
			dwRet = 0x06;
			goto Cleanup;
		}

		dwBytesRead = 0x00;
		WriteFile(hFileEnc, lpFileData, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), &dwBytesRead, NULL);
		if ((dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) != dwBytesRead)
			dwRet = 0x07;
		CloseHandle(hFileEnc);

		if (bDeleteBackupFile)
		{
			SetFileAttributes(szRecoverFilePath, FILE_ATTRIBUTE_NORMAL);
			if (!(DeleteFile(szRecoverFilePath)))
			{
				//AddLogEntry(L"### Quarantine %s File not deleted after decrypt data.", szRecoverFilePath, 0, true, SECONDLEVEL);
			}
		}
	}
	__except ( EXCEPTION_EXECUTE_HANDLER)
	{
		//AddLogEntry(L"### Exception in CWardWizScanner::Decrypt_File, File Path: %s, Original Path: %s", szRecoverFilePath, szOriginalThreatPath, true, SECONDLEVEL);
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
	m_RecoverFileCount.SetWindowText(L"1");
	m_editStatus.SetWindowText(L"File recover completed successfully.");
	
	return dwRet;
}

DWORD CWardWiz_RecoverDlg::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
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
	__except ( EXCEPTION_EXECUTE_HANDLER)
	{
		//AddLogEntry(L"### Exception in CWardWizScanner::DecryptData", 0, 0, true, SECONDLEVEL);
		MessageBox(L"Exception in CWardWiz_RecoverDlg::DecryptData",L"",0);
	}
	return 0;
}

bool CWardWiz_RecoverDlg::ReadKeyFromEncryptedFile(HANDLE hFile)
{
	bool	bReturn = false;
	int		iReadPos = 0x0;
	DWORD   dwBytesRead = 0x0;

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return bReturn;
	}

	if (!IsFileAlreadyEncrypted(hFile))
	{
		return bReturn;
	}
	//read encryption key
	if (m_pbyEncDecKey == NULL)
	{
		m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE, sizeof(unsigned char));
	}
	memset(m_pbyEncDecKey, 0x00, WRDWIZ_KEY_SIZE * sizeof(unsigned char));
	SetFilePointer(hFile, (0x0 + WRDWIZ_SIG_SIZE), NULL, FILE_BEGIN);
	ReadFile(hFile, &m_pbyEncDecKey[0x0], WRDWIZ_KEY_SIZE * sizeof(unsigned char), &dwBytesRead, NULL);
	if (dwBytesRead != WRDWIZ_KEY_SIZE)
	{
		return bReturn;
	}
	bReturn = true;
	return bReturn;
}

bool CWardWiz_RecoverDlg::IsFileAlreadyEncrypted(HANDLE hFile)
{
	bool	bReturn = false;
	int		iReadPos = 0x0;
	DWORD   dwBytesRead = 0x0;
	unsigned char	bySigBuff[WRDWIZ_SIG_SIZE] = { 0x00 };

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return bReturn;
	}

	//check if file is already encrypted by checking existence of sig "WARDWIZ"
	SetFilePointer(hFile, iReadPos, NULL, FILE_BEGIN);
	ReadFile(hFile, &bySigBuff[0x0], WRDWIZ_SIG_SIZE * sizeof(unsigned char), &dwBytesRead, NULL);
	if (dwBytesRead != WRDWIZ_SIG_SIZE)
	{
		return bReturn;
	}
	if (memcmp(&bySigBuff, WRDWIZ_SIG, WRDWIZ_SIG_SIZE) == 0)
	{
		bReturn = true;
	}
	else
	{
		bReturn = false;
	}
	return bReturn;
}

void CWardWiz_RecoverDlg::OnBnClickCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}
