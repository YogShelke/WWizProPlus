
// WWizAdwDBUtilityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WWizAdwDBUtility.h"
#include "WWizAdwDBUtilityDlg.h"
#include "afxdialogex.h"
#include "WWizCRC64.h"

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


// CWWizAdwDBUtilityDlg dialog



CWWizAdwDBUtilityDlg::CWWizAdwDBUtilityDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWWizAdwDBUtilityDlg::IDD, pParent)
	, m_dwTotalCount(0x00)
	, m_dwNewlyAddedCount(0x00)
	, m_dwTotalLocCount(0x00)
	, m_dwNewlyAddedLocCount(0x00)
	, m_dwBufSize(0x00)
	, m_lpbyBuffer(NULL)
	, m_dwBufOffset(0x00)
	, m_dwCharLen(0x00)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pbyEncDecSig = NULL;
	m_pbyEncDecSig = (unsigned char *)calloc(MAX_SIG_SIZE, sizeof(unsigned char));
	m_pbyEncDecSig[0x00] = 'W';
	m_pbyEncDecSig[0x01] = 'R';
	m_pbyEncDecSig[0x02] = 'D';
	m_pbyEncDecSig[0x03] = 'W';
	m_pbyEncDecSig[0x04] = 'I';
	m_pbyEncDecSig[0x05] = 'Z';
	m_pbyEncDecSig[0x06] = 'D';
	m_pbyEncDecSig[0x07] = 'B';
}

CWWizAdwDBUtilityDlg::~CWWizAdwDBUtilityDlg()
{
	if (m_lpbyBuffer != NULL)
	{
		delete[]m_lpbyBuffer;
		m_lpbyBuffer = NULL;
	}

	if (m_pbyEncDecSig != NULL)
	{
		free(m_pbyEncDecSig);
		m_pbyEncDecSig = NULL;
	}

}


void CWWizAdwDBUtilityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DBSIGNATURE, m_editDBSignature);
	DDX_Control(pDX, IDC_EDIT_VERSIONNO, m_editVersionNo);
	DDX_Control(pDX, IDC_EDIT_LOCATION_FILEPATH, m_editLocFilePath);
	DDX_Control(pDX, IDC_EDIT_DATA_FILEPATH, m_editDataFile);
	DDX_Control(pDX, IDC_BUTTON_LFILE_BROWSE, m_btnBrowseLocFile);
	DDX_Control(pDX, IDC_BUTTON_DFILE_BROWSE, m_btnBrowseDataFile);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_TOTALSIGCOUNT, m_stTotalCount);
	DDX_Control(pDX, IDC_STATIC_NEW_ENTRIESCOUNT, m_stNewEntries);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_editStatus);
	DDX_Control(pDX, IDC_STATIC_TOTALLOCCOUNT, m_stTotalLocCount);
	DDX_Control(pDX, IDC_STATIC_NEW_LOCENTRIESCOUNT, m_stTotalNewLocEntries);
	DDX_Control(pDX, IDC_EDIT_EXISTINGDB_FILEPATH, m_editExistingDBPath);
	DDX_Control(pDX, IDC_STATIC_SERVCOUNT, m_stServicesL);
	DDX_Control(pDX, s, m_stServicesD);
	DDX_Control(pDX, IDC_STATIC_FOLDERSC, m_stFoldersL);
	DDX_Control(pDX, IDC_STATIC_FOLDERSD, m_stFoldersD);
	DDX_Control(pDX, IDC_STATIC_FILESC, m_stFilesL);
	DDX_Control(pDX, IDC_STATIC_FILESD, m_stFilesD);
	DDX_Control(pDX, IDC_STATIC_SHORTCUTL, m_stShortCutL);
	DDX_Control(pDX, IDC_STATIC_SHORTCUTD, m_stShortcutD);
	DDX_Control(pDX, IDC_STATIC_TAKSL, m_stTasksL);
	DDX_Control(pDX, IDC_STATIC_TASKSD, m_stTasksD);
	DDX_Control(pDX, IDC_STATIC_REGISTRYL, m_stRegistryL);
	DDX_Control(pDX, IDC_STATIC_REGISTRYD, m_stRegistryD);
	DDX_Control(pDX, IDC_STATIC_BROWSERSL, m_BrowsersL);
	DDX_Control(pDX, IDC_STATIC_BROWSERSD, m_stBrowsersD);
	DDX_Control(pDX, IDC_STATIC_BROWSERSREGL, m_stBrowsersRegL);
	DDX_Control(pDX, IDC_STATIC_BROWSERSREGD, m_stBrowsersRegD);
}

BEGIN_MESSAGE_MAP(CWWizAdwDBUtilityDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CWWizAdwDBUtilityDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CWWizAdwDBUtilityDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_LFILE_BROWSE, &CWWizAdwDBUtilityDlg::OnBnClickedButtonLfileBrowse)
	ON_BN_CLICKED(IDC_BUTTON_DFILE_BROWSE, &CWWizAdwDBUtilityDlg::OnBnClickedButtonDfileBrowse)
	ON_BN_CLICKED(IDC_RADIO_LOCATIONS, &CWWizAdwDBUtilityDlg::OnBnClickedRadioLocations)
	ON_BN_CLICKED(IDC_RADIO_DATA, &CWWizAdwDBUtilityDlg::OnBnClickedRadioData)
	ON_BN_CLICKED(IDC_BUTTON_EXDBFILE_BROWSE, &CWWizAdwDBUtilityDlg::OnBnClickedButtonExdbfileBrowse)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_DB, &CWWizAdwDBUtilityDlg::OnBnClickedButtonLoadDb)
END_MESSAGE_MAP()


// CWWizAdwDBUtilityDlg message handlers

BOOL CWWizAdwDBUtilityDlg::OnInitDialog()
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

	m_editVersionNo.SetWindowTextW(L"1.1.0.0");
	m_editDBSignature.SetWindowTextW(L"WRDWIZDB05");
	CButton* pButton = (CButton*)GetDlgItem(IDC_RADIO_SERVICES);
	pButton->SetCheck(TRUE);

	pButton = (CButton*)GetDlgItem(IDC_RADIO_DATA);
	pButton->SetCheck(TRUE);
	OnBnClickedRadioData();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWWizAdwDBUtilityDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWWizAdwDBUtilityDlg::OnPaint()
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
HCURSOR CWWizAdwDBUtilityDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWWizAdwDBUtilityDlg::OnBnClickedButtonCancel()
{
	OnCancel();
}

void CWWizAdwDBUtilityDlg::OnBnClickedButtonLfileBrowse()
{
	try
	{
		static TCHAR szEncFilter[] = L"All Files(*.*)|*.*|";
		CFileDialog FileDlg(TRUE, L"All Files(*.*)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
		if (FileDlg.DoModal() == IDOK)
		{
			CString csFilePath = FileDlg.GetPathName();
			m_editLocFilePath.SetWindowText(csFilePath);
		}
	}
	catch (...)
	{
	}
}

void CWWizAdwDBUtilityDlg::OnBnClickedButtonDfileBrowse()
{
	try
	{
		static TCHAR szEncFilter[] = L"All Files(*.*)|*.*|";
		CFileDialog FileDlg(TRUE, L"All Files(*.*)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
		if (FileDlg.DoModal() == IDOK)
		{
			CString csFilePath = FileDlg.GetPathName();
			m_editDataFile.SetWindowText(csFilePath);
		}
	}
	catch (...)
	{
	}
}

void CWWizAdwDBUtilityDlg::OnBnClickedRadioLocations()
{
	m_editDataFile.EnableWindow(FALSE);
	m_editDataFile.SetWindowText(L"");
	m_btnBrowseDataFile.EnableWindow(FALSE);

	m_editLocFilePath.EnableWindow(TRUE);
	m_btnBrowseLocFile.EnableWindow(TRUE);
}

void CWWizAdwDBUtilityDlg::OnBnClickedRadioData()
{
	m_editDataFile.EnableWindow(TRUE);
	m_btnBrowseDataFile.EnableWindow(TRUE);

	m_editLocFilePath.SetWindowText(L"");
	m_editLocFilePath.EnableWindow(FALSE);
	m_btnBrowseLocFile.EnableWindow(FALSE);
}

void CWWizAdwDBUtilityDlg::OnBnClickedButtonStart()
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_RADIO_DATA);
	int iSelection = pButton->GetCheck();

	CString csFilePath;
	iSelection == 0x01 ? m_editDataFile.GetWindowText(csFilePath) : m_editLocFilePath.GetWindowText(csFilePath);


	if (!PathFileExists(csFilePath))
	{
		MessageBox(L"Please provide valid file path.", L"WardWiz", MB_ICONEXCLAMATION);
		return;
	}

	FUNCTIONS enumFunct = GetCurrentFunSelection();
	if (enumFunct == NOSELECTION)
	{
		MessageBox(L"Please select entry type.", L"WardWiz", MB_ICONEXCLAMATION);
		return;
	}

	pButton = (CButton*)GetDlgItem(IDC_RADIO_LOCATIONS);
	int iSelLocations = pButton->GetCheck();

	size_t   i;
	char szFilePath[MAX_PATH] = { 0 };
	wcstombs_s(&i, szFilePath, (size_t)MAX_PATH, csFilePath, (size_t)MAX_PATH);

	m_dwNewlyAddedCount = 0x00;
	m_dwNewlyAddedLocCount = 0x00;

	std::ifstream file(szFilePath);
	std::string str;
	while (std::getline(file, str))
	{
		m_dwCharLen += strlen(str.c_str());
		switch (enumFunct)
		{
		case SERVICES:
			iSelLocations == 0x01 ? m_vecServicesLocations.push_back(str) : m_vecServicesData.push_back(str);
			iSelLocations == 0x01 ? m_dwNewlyAddedLocCount++ : m_dwNewlyAddedCount++;
			break;
		case FOLDERS:
			iSelLocations == 0x01 ? m_vecFoldersLocations.push_back(str) : m_vecFoldersData.push_back(str);
			iSelLocations == 0x01 ? m_dwNewlyAddedLocCount++ : m_dwNewlyAddedCount++;
			break;
		case FILES:
			iSelLocations == 0x01 ? m_vecFilesLocations.push_back(str) : m_vecFilesData.push_back(str);
			iSelLocations == 0x01 ? m_dwNewlyAddedLocCount++ : m_dwNewlyAddedCount++;
			break;
		case SHORTCUTS:
			iSelLocations == 0x01 ? m_vecShortcutsLocations.push_back(str) : m_vecShortcutsData.push_back(str);
			iSelLocations == 0x01 ? m_dwNewlyAddedLocCount++ : m_dwNewlyAddedCount++;
			break;
		case TASKS:
			iSelLocations == 0x01 ? m_vecsTasksLocations.push_back(str) : m_vecTasksData.push_back(str);
			iSelLocations == 0x01 ? m_dwNewlyAddedLocCount++ : m_dwNewlyAddedCount++;
			break;
		case REGISTRY:
			iSelLocations == 0x01 ? m_vecRegLocations.push_back(str) : m_vecRegData.push_back(str);
			iSelLocations == 0x01 ? m_dwNewlyAddedLocCount++ : m_dwNewlyAddedCount++;
			break;
		case BROWSERSFILES:
			iSelLocations == 0x01 ? m_vecBrowserLocations.push_back(str) : m_vecBrowserData.push_back(str);
			iSelLocations == 0x01 ? m_dwNewlyAddedLocCount++ : m_dwNewlyAddedCount++;
			break;
		case BROWSERSREGISTRY:
			iSelLocations == 0x01 ? m_vecBrowserRegLocations.push_back(str) : m_vecBrowserRegData.push_back(str);
			iSelLocations == 0x01 ? m_dwNewlyAddedLocCount++ : m_dwNewlyAddedCount++;
			break;
		default:
			break;
		}
	}
	file.close();

	remove_duplicates(m_vecServicesLocations);
	remove_duplicates(m_vecServicesData);

	m_stAdwCleaner.stServices.dwLocationCount = m_vecServicesLocations.size();
	m_stAdwCleaner.stServices.dwServicesCount = m_vecServicesData.size();

	remove_duplicates(m_vecFoldersLocations);
	remove_duplicates(m_vecFoldersData);

	m_stAdwCleaner.stFolders.dwLocationCount = m_vecFoldersLocations.size();
	m_stAdwCleaner.stFolders.dwFoldersCount = m_vecFoldersData.size();

	remove_duplicates(m_vecFilesLocations);
	remove_duplicates(m_vecFilesData);

	m_stAdwCleaner.stFiles.dwLocationCount = m_vecFilesLocations.size();
	m_stAdwCleaner.stFiles.dwFilesCount = m_vecFilesData.size();
	
	remove_duplicates(m_vecShortcutsLocations);
	remove_duplicates(m_vecShortcutsData);

	m_stAdwCleaner.stShortcuts.dwLocationCount = m_vecShortcutsLocations.size();
	m_stAdwCleaner.stShortcuts.dwShortcutsCount = m_vecShortcutsData.size();

	remove_duplicates(m_vecsTasksLocations);
	remove_duplicates(m_vecTasksData);

	m_stAdwCleaner.stSheduledTasks.dwLocationCount = m_vecsTasksLocations.size();
	m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount = m_vecTasksData.size();

	remove_duplicates(m_vecRegLocations);
	remove_duplicates(m_vecRegData);

	m_stAdwCleaner.stRegistry.dwLocationCount = m_vecRegLocations.size();
	m_stAdwCleaner.stRegistry.dwRegistryCount = m_vecRegData.size();

	remove_duplicates(m_vecBrowserLocations);
	remove_duplicates(m_vecBrowserData);

	m_stAdwCleaner.stBrowsers.dwLocationCount = m_vecBrowserLocations.size();
	m_stAdwCleaner.stBrowsers.dwBrowsersCount = m_vecBrowserData.size();
	
	remove_duplicates(m_vecBrowserRegLocations);
	remove_duplicates(m_vecBrowserRegData);

	m_stAdwCleaner.stBrowsersReg.dwLocationCount = m_vecBrowserRegLocations.size();
	m_stAdwCleaner.stBrowsersReg.dwBrowsersCount = m_vecBrowserRegData.size();

	//Calcuate total here
	m_dwTotalCount = m_vecServicesData.size() + m_vecFoldersData.size() + m_vecFilesData.size() + m_vecShortcutsData.size()
		+ m_vecTasksData.size() + m_vecRegData.size() + m_vecBrowserData.size() + m_vecBrowserRegData.size();

	m_dwTotalLocCount = m_vecServicesLocations.size() + m_vecFoldersLocations.size() + m_vecFilesLocations.size() + m_vecShortcutsLocations.size()
		+ m_vecsTasksLocations.size() + m_vecRegLocations.size() + m_vecBrowserLocations.size() + m_vecBrowserRegLocations.size();

	//Create Memory here
	if (m_lpbyBuffer != NULL)
	{
		delete []m_lpbyBuffer;
		m_lpbyBuffer = NULL;
	}

	m_dwBufSize = m_dwCharLen;
	m_dwBufSize += m_dwTotalLocCount;
	m_dwBufSize += m_dwTotalCount;

	m_lpbyBuffer = (LPBYTE)malloc(m_dwBufSize);
	if (!m_lpbyBuffer)//Buffer allocation failed.
	{
		return;
	}

	m_dwBufOffset = 0x00;
	memset(m_lpbyBuffer, 0, m_dwBufSize);

	MakeBuffer(m_vecServicesLocations);
	MakeBuffer(m_vecServicesData);

	MakeBuffer(m_vecFoldersLocations);
	MakeBuffer(m_vecFoldersData);
	
	MakeBuffer(m_vecFilesLocations);
	MakeBuffer(m_vecFilesData);

	MakeBuffer(m_vecShortcutsLocations);
	MakeBuffer(m_vecShortcutsData);

	MakeBuffer(m_vecsTasksLocations);
	MakeBuffer(m_vecTasksData);

	MakeBuffer(m_vecRegLocations);
	MakeBuffer(m_vecRegData);

	MakeBuffer(m_vecBrowserLocations);
	MakeBuffer(m_vecBrowserData);

	MakeBuffer(m_vecBrowserRegLocations);
	MakeBuffer(m_vecBrowserRegData);
	
	//Check buffer CRC here
	ULONG64	ulBufCRC = 0x00;
	CWWizCRC64	objWWizCRC64;
	objWWizCRC64.CalcCRC64(ulBufCRC, m_lpbyBuffer, m_dwBufSize);

	DecryptData(m_lpbyBuffer, m_dwBufSize);

	CString csVersionNo;
	m_editVersionNo.GetWindowText(csVersionNo);
	
	size_t   tVersionSize;
	DWORD dwVersionLength = _tcslen(csVersionNo);
	char * lpVersion = (char*)malloc(dwVersionLength + 1);
	if (!lpVersion)
	{
		MessageBox(L"Failed to Allocation memory", L"WardWiz", MB_ICONEXCLAMATION);
		return;
	}

	memset(lpVersion, 0, dwVersionLength + 1);
	wcstombs_s(&tVersionSize, lpVersion, dwVersionLength + 1, csVersionNo.GetBuffer(), dwVersionLength + 1);

	CString csDBSignature;
	m_editDBSignature.GetWindowText(csDBSignature);
	size_t   tSize;
	char szDBSig[0x0B] = { 0 };
	wcstombs_s(&tSize, szDBSig, sizeof(szDBSig), csDBSignature.GetBuffer(), sizeof(szDBSig) + 1);

	csFilePath += L".NEW";
	if (!SaveDataIntoFile(csFilePath.GetBuffer(), m_stAdwCleaner, szDBSig, lpVersion, dwVersionLength, ulBufCRC))
	{
		MessageBox(L"Failed to Create DB file", L"WardWiz", MB_ICONEXCLAMATION);
		return;
	}
	
	DoReportingStuff();

	MessageBox(L"DB Created", L"WardWiz", MB_ICONINFORMATION);
}

FUNCTIONS CWWizAdwDBUtilityDlg::GetCurrentFunSelection()
{
	if (((CButton*)GetDlgItem(IDC_RADIO_SERVICES))->GetCheck())
	{
		return SERVICES;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_FOLDERS))->GetCheck())
	{
		return FOLDERS;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_FILES))->GetCheck())
	{
		return FILES;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_SHORTCUT))->GetCheck())
	{
		return SHORTCUTS;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_TASKS))->GetCheck())
	{
		return TASKS;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_REGISTRY))->GetCheck())
	{
		return REGISTRY;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_BROWSER))->GetCheck())
	{
		return BROWSERSFILES;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_BROWSER_REGISTRY))->GetCheck())
	{
		return BROWSERSREGISTRY;
	}
	else
	{
		return NOSELECTION;
	}
}

bool CWWizAdwDBUtilityDlg::SaveDataIntoFile(LPTSTR szFilePath, STRUCTADWCLEANER &szAdwClean, LPSTR szSignature, LPSTR szVersion, DWORD dwVersionLength, ULONG64 ulFileCRC)
{
	bool bReturn = false;

	HANDLE	hOutputFileHandle = NULL;
	LPBYTE bySigBuffer = NULL;
	try
	{
		if (PathFileExists(szFilePath))
		{
			SetFileAttributes(szFilePath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szFilePath);
		}

		hOutputFileHandle = CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutputFileHandle == INVALID_HANDLE_VALUE)
		{
			return bReturn;
		}

		DWORD dwSigOffset = 0x00;

		DWORD dwBufferOffset = dwSigOffset;
		SetFilePointer(hOutputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		DWORD dwSigLength = 0x0A + 2 + dwVersionLength + 2;
		bySigBuffer = new BYTE[dwSigLength];
		if (!bySigBuffer)
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Initialize memory
		memset(bySigBuffer, 0, dwSigLength);

		memcpy(&bySigBuffer[dwSigOffset], szSignature, 0x0A);
		dwSigOffset += 0x0A;

		memcpy(&bySigBuffer[dwSigOffset], "|", 0x01);
		dwSigOffset += 0x01;

		memcpy(&bySigBuffer[dwSigOffset], szVersion, dwVersionLength);
		dwSigOffset += dwVersionLength;

		memcpy(&bySigBuffer[dwSigOffset], "|", 0x01);
		dwSigOffset += 0x01;

		//Write here wardwizSignature
		DWORD dwBytesWritten = 0;
		if (!WriteFile(hOutputFileHandle, bySigBuffer, dwSigOffset, &dwBytesWritten, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != dwSigOffset)
		{
			bReturn = false;
			goto CLEANUP;
		}

		dwBufferOffset = dwSigOffset;
		SetFilePointer(hOutputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);
		
		
		//Write here count structure
		dwBytesWritten = 0;
		if (!WriteFile(hOutputFileHandle, &szAdwClean, sizeof(szAdwClean), &dwBytesWritten, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != sizeof(szAdwClean))
		{
			bReturn = false;
			goto CLEANUP;
		}

		//write CRC
		dwBufferOffset += dwBytesWritten;
		SetFilePointer(hOutputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		dwBytesWritten = 0;
		if (!WriteFile(hOutputFileHandle, &ulFileCRC, sizeof(ulFileCRC), &dwBytesWritten, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != sizeof(ulFileCRC))
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Read here remaining file
		dwBufferOffset += dwBytesWritten;
		SetFilePointer(hOutputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		dwBytesWritten = 0;
		if (!WriteFile(hOutputFileHandle, m_lpbyBuffer, m_dwBufSize, &dwBytesWritten, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != m_dwBufSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwDBUtilityDlg::SaveDataIntoFile, %s", szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:

	if (bySigBuffer != NULL)
	{
		delete[]bySigBuffer;
		bySigBuffer = NULL;
	}

	//Need to close file handle after collecting buffer
	if (hOutputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOutputFileHandle);
		hOutputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

bool CWWizAdwDBUtilityDlg::MakeBuffer(std::vector<string> &vecData)
{
	bool bReturn = false;
	try
	{
		if (vecData.size() == 0)
		{
			return bReturn;
		}

		if (m_dwBufOffset > m_dwBufSize)
		{
			return bReturn;
		}

		for (std::vector<string>::iterator it = vecData.begin(); it != vecData.end(); ++it)
		{
			string strLine = *it;
			int iStrLen = strlen(strLine.c_str());

			if ((iStrLen + m_dwBufOffset) > m_dwBufSize)
				break;

			memcpy(&m_lpbyBuffer[m_dwBufOffset], strLine.c_str(), iStrLen);
			m_dwBufOffset += iStrLen;

			char *byNewLine = "*";
			memcpy(&m_lpbyBuffer[m_dwBufOffset], byNewLine, 0x01);
			m_dwBufOffset += 0x01;
		}
	
		bReturn = true;
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in CWWizAdwDBUtilityDlg::MakeBuffer", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : DecryptData
*  Description    : Decrypt data of buffer for registration data.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWWizAdwDBUtilityDlg::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
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
		AddLogEntry(L"### Exception in CWardWizUIApp::DecryptData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

void CWWizAdwDBUtilityDlg::OnBnClickedButtonExdbfileBrowse()
{
	try
	{
		static TCHAR szEncFilter[] = L"All Files(*.*)|*.*|";
		CFileDialog FileDlg(TRUE, L"All Files(*.*)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
		if (FileDlg.DoModal() == IDOK)
		{
			CString csFilePath = FileDlg.GetPathName();
			m_editExistingDBPath.SetWindowText(csFilePath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwDBUtilityDlg::OnBnClickedButtonExdbfileBrowse", 0, 0, true, SECONDLEVEL);
	}
}

void CWWizAdwDBUtilityDlg::OnBnClickedButtonLoadDb()
{
	CString csExistingDBFile;
	m_editExistingDBPath.GetWindowText(csExistingDBFile);

	if (!PathFileExists(csExistingDBFile))
	{
		MessageBox(L"Please provide valid file path.", L"WardWiz", MB_ICONEXCLAMATION);
		return;
	}

	DWORD dwDBMajorVersion = 0x00;
	DWORD dwDBVersionLength = 0x00;
	if (!IsFileAlreadyEncrypted(csExistingDBFile, dwDBVersionLength, dwDBMajorVersion))
	{
		return;
	}

	ClearVectorData();

	DWORD dwLoadedCount = 0x00;
	if (!LoadContaintFromFile(csExistingDBFile.GetBuffer(), dwDBVersionLength, dwLoadedCount))
	{
		AddLogEntry(L"### Exception in CWrdwizEncDecManager::ParseDBVersion", 0, 0, true, SECONDLEVEL);
		return;
	}

	DoReportingStuff();
}


/***************************************************************************************************
*  Function Name  : LoadContaintFromFile
*  Description    : Load Signature Database with new implementation, this applies from 1.1.0.0 DB version
and above.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	10 May 2016
****************************************************************************************************/
bool CWWizAdwDBUtilityDlg::LoadContaintFromFile(LPTSTR szFilePath, DWORD dwVersionLength, DWORD &dwSigCount)
{
	bool bReturn = false;

	HANDLE	hInputFileHandle = NULL;
	BYTE	*bFileBuffer = NULL;
	try
	{
		hInputFileHandle = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFileHandle == INVALID_HANDLE_VALUE)
		{
			return bReturn;
		}

		/* Get file size here */
		DWORD dwFileSize = GetFileSize(hInputFileHandle, 0);

		/* If file size if 0 return */
		if (dwFileSize == 0x00)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DWORD dwBufferOffset = MAX_SIG_SIZE + 2 + dwVersionLength + 2;
		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		DWORD dwBytesRead = 0;
		if (!ReadFile(hInputFileHandle, &m_stAdwCleaner, sizeof(m_stAdwCleaner), &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != sizeof(m_stAdwCleaner))
		{
			bReturn = false;
			goto CLEANUP;
		}

		dwBufferOffset += dwBytesRead;
		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		//Read CRC here
		dwBytesRead = 0;
		ULONG64 ulFileCRC = 0x00;
		if (!ReadFile(hInputFileHandle, &ulFileCRC, sizeof(ulFileCRC), &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != sizeof(ulFileCRC))
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Read here remaining file
		dwBufferOffset += dwBytesRead;
		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		DWORD dwRemSize = dwFileSize - dwBufferOffset;
		bFileBuffer = (BYTE*)malloc(dwRemSize * sizeof(BYTE));

		if (bFileBuffer == NULL)
		{
			bReturn = false;
			goto CLEANUP;
		}

		dwBytesRead = 0;
		if (!ReadFile(hInputFileHandle, bFileBuffer, dwRemSize, &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != dwRemSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DecryptData(bFileBuffer, dwRemSize);

		//Check buffer CRC here
		ULONG64	ulBufCRC = 0x00;
		CWWizCRC64	objWWizCRC64;
		objWWizCRC64.CalcCRC64(ulBufCRC, bFileBuffer, dwRemSize);

		//verify here, if not matched means there is change in DB file.
		if (ulBufCRC != ulFileCRC)
		{
			CString csCRC;
			csCRC.Format(L"%04x", ulBufCRC);
			AddLogEntry(L"### CRC value not matched, Calculated CRC: [%s] , File: %s", szFilePath, csCRC, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}
		
		dwSigCount = 0x00;

		int iLineOffset = 0;
		DWORD dwLinesCount = 0x00;
		DWORD iBufOffset = 0x00;
		
		char szLine[MAX_PATH] = { 0 };
		while (iBufOffset < dwRemSize)
		{
			if (bFileBuffer[iBufOffset] != '*')
			{
				if (iLineOffset < MAX_PATH)
				{
					szLine[iLineOffset++] = bFileBuffer[iBufOffset];
				}
			}
			else
			{
				dwLinesCount++;
				szLine[iLineOffset] = '\0';
				m_dwCharLen += strlen(szLine);
				
				if (dwLinesCount <= m_stAdwCleaner.stServices.dwLocationCount)
				{
					m_vecServicesLocations.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount))
				{
					m_vecServicesData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount))
				{
					m_vecFoldersLocations.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount))
				{
					m_vecFoldersData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount))
				{
					m_vecFilesLocations.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount))
				{
					m_vecFilesData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount))
				{
					m_vecShortcutsLocations.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount))
				{
					m_vecShortcutsData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount))
				{
					m_vecsTasksLocations.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount))
				{
					m_vecTasksData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount))
				{
					m_vecRegLocations.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stRegistry.dwRegistryCount))
				{
					m_vecRegData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stBrowsers.dwLocationCount))
				{
					m_vecBrowserLocations.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stBrowsers.dwLocationCount + m_stAdwCleaner.stBrowsers.dwBrowsersCount))
				{
					m_vecBrowserData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stBrowsers.dwLocationCount + m_stAdwCleaner.stBrowsers.dwBrowsersCount + m_stAdwCleaner.stBrowsersReg.dwLocationCount))
				{
					m_vecBrowserRegLocations.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stBrowsers.dwLocationCount + m_stAdwCleaner.stBrowsers.dwBrowsersCount + m_stAdwCleaner.stBrowsersReg.dwBrowsersCount))
				{
					m_vecBrowserRegData.push_back(szLine);
				}
				else
				{
					AddLogEntry(L"### UnHandled string, %s", A2BSTR(szLine), 0, true, SECONDLEVEL);
				}

				iLineOffset = 0;
				memset(&szLine, 0, sizeof(szLine));
			}
			iBufOffset++;
		}
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::LoadContaintFromFile, %s", szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:
	//Cleanup here memory
	if (bFileBuffer != NULL)
	{
		delete[]bFileBuffer;
		bFileBuffer = NULL;
	}

	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

bool CWWizAdwDBUtilityDlg::IsFileAlreadyEncrypted(CString csFileName, DWORD &dwDBVersionLength, DWORD &dwDBMajorVersion)
{
	bool bReturn = false;
	char	bySigBuff[MAX_VERSIONCHKBUFF] = { 0x00 };

	if (csFileName.GetLength() == NULL || m_pbyEncDecSig == NULL)
	{
		return bReturn; //error
	}

	FILE *pFile = NULL;
	m_ulFileSize = 0x0;
	pFile = _wfsopen(csFileName, _T("r"), 0x20);
	if (!pFile)
	{
		return bReturn;

	}
	fseek(pFile, 0x00, SEEK_END);
	m_ulFileSize = ftell(pFile);
	if (m_ulFileSize < MAX_SIG_SIZE)
	{
		fclose(pFile);
		pFile = NULL;
		return bReturn;
	}

	fseek(pFile, 0x0, SEEK_SET);
	//
	//TCHAR szVersion[20] = {0};
	//GetVersionDetails(pFile);

	bool bValidSig = false;
	fread_s(&bySigBuff[0x00], MAX_SIG_SIZE, MAX_SIG_SIZE, 0x01, pFile);

	if (memcmp(&bySigBuff, &m_pbyEncDecSig[0x00], MAX_SIG_SIZE) == 0x00)
	{
		bValidSig = true;
	}

	fseek(pFile, 0x0, SEEK_SET);

	//Get here the version number by tokenizing.
	memset(&bySigBuff, 0, MAX_VERSIONCHKBUFF);
	fread_s(&bySigBuff[0x00], MAX_VERSIONCHKBUFF, MAX_VERSIONCHKBUFF, 0x01, pFile);

	//Tokenize buffer
	char seps[] = "|";
	char *token = NULL;
	char* context = NULL;
	token = strtok_s(bySigBuff, seps, &context);

	DWORD dwCount = 0;

	bool bValidSigLength = false;
	bool bValidVersion = false;
	while (token != NULL)
	{
		if (strlen(token) > 0)
		{
			if (dwCount == 0)
			{
				if (strlen(token) == 0x0A)
				{
					bValidSigLength = true;
				}
			}

			if (dwCount >= 1)
			{
				break;
			}
			dwCount++;
		}
		token = strtok_s(NULL, seps, &context);
	}

	if (token != NULL)
	{
		DWORD dwlength = (DWORD)strlen(token);
		if (dwlength > 0)
		{
			/* Parse here DB vesion and get major vers*/
			if (ParseDBVersion(token, dwDBMajorVersion))
			{
				bValidVersion = true;
				dwDBVersionLength = dwlength;
			}
		}
	}

	if (bValidSig && bValidVersion && bValidSigLength)
	{
		bReturn = true;
	}
	fclose(pFile);
	pFile = NULL;
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	ParseDBVersion
*  Description    :	This function will parse the DB version
*  Author Name    :	Prajakta
*  SR.NO		  : WRDWIZCOMMON_0103
*  Date           : 15 May 2014
**********************************************************************************************************/
bool CWWizAdwDBUtilityDlg::ParseDBVersion(LPSTR lpszVersion, DWORD &dwMajorVersion)
{
	__try
	{
		if (!lpszVersion)
			return false;

		const char sToken[2] = ".";
		char *token;
		int iCount = 0;

		/* get the first token */
		token = strtok(lpszVersion, sToken);

		/* walk through other tokens */
		while (token != NULL && (iCount <= 3))
		{
			/* take major version from here */
			if (iCount == 1)
			{
				if (strlen(token) > 0)
				{
					dwMajorVersion = static_cast<DWORD>(atoi(token));
					break;
				}
			}
			iCount++;
			token = strtok(NULL, sToken);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdwizEncDecManager::ParseDBVersion", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/*=====================================================================================================*/
/*												REPORTING   		 								  */
/*=====================================================================================================*/
void CWWizAdwDBUtilityDlg::DoReportingStuff()
{
	//Calcuate total here
	m_dwTotalCount = m_vecServicesData.size() + m_vecFoldersData.size() + m_vecFilesData.size() + m_vecShortcutsData.size()
		+ m_vecTasksData.size() + m_vecRegData.size() + m_vecBrowserData.size() + m_vecBrowserRegData.size();

	m_dwTotalLocCount = m_vecServicesLocations.size() + m_vecFoldersLocations.size() + m_vecFilesLocations.size() + m_vecShortcutsLocations.size()
		+ m_vecsTasksLocations.size() + m_vecRegLocations.size() + m_vecBrowserLocations.size() + m_vecBrowserRegLocations.size();

	CString csTotalCount;
	csTotalCount.Format(L"Total Data count: %d", m_dwTotalCount);
	m_stTotalCount.SetWindowText(csTotalCount);

	CString csNewlyAddedCount;
	csNewlyAddedCount.Format(L"New entries Data added:  %d", m_dwNewlyAddedCount);
	m_stNewEntries.SetWindowText(csNewlyAddedCount);

	CString csTotalLocCount;
	csTotalLocCount.Format(L"Total Location count: %d", m_dwTotalLocCount);
	m_stTotalLocCount.SetWindowText(csTotalLocCount);

	CString csNewlyAddedLocCount;
	csNewlyAddedLocCount.Format(L"New Location entries added:  %d", m_dwNewlyAddedLocCount);
	m_stTotalNewLocEntries.SetWindowText(csNewlyAddedLocCount);

	CString csCommon;
	csCommon.Format(L"L:  %d", m_vecServicesLocations.size());
	m_stServicesL.SetWindowText(csCommon);

	csCommon.Format(L"D:  %d", m_vecServicesData.size());
	m_stServicesD.SetWindowText(csCommon);

	csCommon.Format(L"L:  %d", m_vecFoldersLocations.size());
	m_stFoldersL.SetWindowText(csCommon);

	csCommon.Format(L"D:  %d", m_vecFoldersData.size());
	m_stFoldersD.SetWindowText(csCommon);

	csCommon.Format(L"L:  %d", m_vecFilesLocations.size());
	m_stFilesL.SetWindowText(csCommon);

	csCommon.Format(L"D:  %d", m_vecFilesData.size());
	m_stFilesD.SetWindowText(csCommon);

	csCommon.Format(L"L:  %d", m_vecShortcutsLocations.size());
	m_stShortCutL.SetWindowText(csCommon);

	csCommon.Format(L"D:  %d", m_vecShortcutsData.size());
	m_stShortcutD.SetWindowText(csCommon);

	csCommon.Format(L"L:  %d", m_vecsTasksLocations.size());
	m_stTasksL.SetWindowText(csCommon);

	csCommon.Format(L"D:  %d", m_vecTasksData.size());
	m_stTasksD.SetWindowText(csCommon);

	csCommon.Format(L"L:  %d", m_vecRegLocations.size());
	m_stRegistryL.SetWindowText(csCommon);

	csCommon.Format(L"D:  %d", m_vecRegData.size());
	m_stRegistryD.SetWindowText(csCommon);

	csCommon.Format(L"L:  %d", m_vecBrowserLocations.size());
	m_BrowsersL.SetWindowText(csCommon);

	csCommon.Format(L"D:  %d", m_vecBrowserData.size());
	m_stBrowsersD.SetWindowText(csCommon);

	csCommon.Format(L"L:  %d", m_vecBrowserRegLocations.size());
	m_stBrowsersRegL.SetWindowText(csCommon);

	csCommon.Format(L"D:  %d", m_vecBrowserRegData.size());
	m_stBrowsersRegD.SetWindowText(csCommon);
}

void CWWizAdwDBUtilityDlg::ClearVectorData()
{
	m_dwTotalCount = 0x00;
	m_dwNewlyAddedCount = 0x00;
	m_dwTotalLocCount = 0x00;
	m_dwNewlyAddedLocCount = 0x00;

	m_vecServicesLocations.clear();
	m_vecFoldersLocations.clear();
	m_vecFilesLocations.clear();
	m_vecShortcutsLocations.clear();
	m_vecsTasksLocations.clear();
	m_vecRegLocations.clear();
	m_vecBrowserLocations.clear();
	m_vecBrowserRegLocations.clear();

	m_vecServicesData.clear();
	m_vecFoldersData.clear();
	m_vecFilesData.clear();
	m_vecShortcutsData.clear();
	m_vecTasksData.clear();
	m_vecRegData.clear();
	m_vecBrowserData.clear();
	m_vecBrowserRegData.clear();
}
