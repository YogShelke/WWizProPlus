#include "stdafx.h"
#include "WWizAdwareCleaner.h"
#include "WWizAdwareCleanerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD WINAPI StartScanningThread(LPVOID lpvThreadParam);
DWORD WINAPI LoadDataBase(LPVOID lpvThreadParam);

#define USER_PROGRESS_TIMER	WM_USER + 100

#define SETFILEPATH_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 1)

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

HWINDOW   CWWizAdwareCleanerDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWWizAdwareCleanerDlg::get_resource_instance() { return theApp.m_hInstance; }

// CWWizAdwareCleanerDlg dialog



CWWizAdwareCleanerDlg::CWWizAdwareCleanerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWWizAdwareCleanerDlg::IDD, pParent)
	, m_bScanning(false)
	, m_dwScannedCount(0x00)
	, m_dwDetectedCount(0x00)
	, m_dwRemovedCount(0x00)
	, m_dwLocationCount(0x00)
	, m_dwDataCount(0x00)
	, m_dwTotalIterations(0x00)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWWizAdwareCleanerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_stStatus);
	DDX_Control(pDX, IDC_PROGRESS_STATE, m_prgState);
	DDX_Control(pDX, IDC_STATIC_THREATS_FOUND, m_stThreatsFound);
	DDX_Control(pDX, IDC_BUTTON_SCAN_PAUSE, m_btnScan);
	DDX_Control(pDX, IDC_BUTTON_CLEAN, m_btnClean);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnCancel);
	DDX_Control(pDX, IDC_LIST_FOUND_ENTRIES, m_lstFoundEntries);
	DDX_Control(pDX, IDC_BUTTON_PAUSE_RESUME, m_btnPauseResume);
	DDX_Control(pDX, IDC_STATIC_SCANNED_COUNT, m_stScannedCount);
	DDX_Control(pDX, IDC_STATIC_THREATS_CLEANED, m_stThreatsCleaned);
	DDX_Control(pDX, IDC_CHECK_SELECTALL, m_chkSelectAll);
}

BEGIN_MESSAGE_MAP(CWWizAdwareCleanerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CWWizAdwareCleanerDlg message handlers

BOOL CWWizAdwareCleanerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	/*ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
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
	}*/

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	//CRect oRcRect;
	//m_lstFoundEntries.GetWindowRect(&oRcRect);
	//m_lstFoundEntries.InsertColumn(0, L"Threat Name", LVCFMT_LEFT, 100);
	//m_lstFoundEntries.InsertColumn(1, L"Path", LVCFMT_LEFT,  390);
	//m_lstFoundEntries.InsertColumn(2, L"Action", LVCFMT_LEFT, 100);
	//m_lstFoundEntries.InsertColumn(3, L"", LVCFMT_LEFT, 0);

	//m_prgState.SetRange(0, 100);

	//ListView_SetExtendedListViewStyle(m_lstFoundEntries.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);

	HideAllElements();
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)AfxGetResourceHandle(), L"res:IDR_HTM_ADWARE_CLEANER_SA.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_ADWARE_CLEANER_SA.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWWizAdwareCleanerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWWizAdwareCleanerDlg::OnPaint()
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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWWizAdwareCleanerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWWizAdwareCleanerDlg::OnBnClickedButtonScan()
{
	m_hThreadScan = ::CreateThread(NULL, 0, StartScanningThread, (LPVOID) this, 0, NULL);
}

/***************************************************************************************************
*  Function Name  : OnStartAdwareScan
*  Description    : Function to start Adware Scan
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnStartAdwareScan(SCITER_VALUE svAddAdwareEntryCB, SCITER_VALUE svSetScanFinishedStatus, SCITER_VALUE svSetScanPercentage)
{
	m_svAddAdwareEntryCB = svAddAdwareEntryCB;
	m_svSetScanFinishedStatus = svSetScanFinishedStatus;
	m_svSetScanPercentage = svSetScanPercentage;
	OnBnClickedButtonScan();
	return json::value();
}

void CWWizAdwareCleanerDlg::LoadDB()
{
	//m_prgState.ModifyStyle(0, PBS_MARQUEE);
	//m_prgState.SetMarquee(TRUE, 0);

	//SetStatus(L"Loading database...");
	//m_btnPauseResume.EnableWindow(FALSE);
	//m_btnScan.EnableWindow(FALSE);
	//m_btnCancel.EnableWindow(FALSE);

	m_svFunGetLoadStatus.call(0);
	m_objAdwScan.LoadDB();
	Sleep(500);
	m_svFunGetLoadStatus.call(1);
	
	//SetStatus(L"Ready...Please click on Scan button to start scanning process.");
	//m_btnPauseResume.EnableWindow(TRUE);
	//m_btnScan.EnableWindow(TRUE);
	//m_btnCancel.EnableWindow(TRUE);
	//m_prgState.SetMarquee(FALSE, 1000);
	//m_prgState.ModifyStyle(PBS_MARQUEE, 0);
}

DWORD WINAPI LoadDataBase(LPVOID lpvThreadParam)
{
	CWWizAdwareCleanerDlg *pThis = (CWWizAdwareCleanerDlg *)lpvThreadParam;
	if (!pThis)
		return 0;

	pThis->LoadDB();

	return 1;
}

DWORD WINAPI StartScanningThread(LPVOID lpvThreadParam)
{
	CWWizAdwareCleanerDlg *pThis = (CWWizAdwareCleanerDlg *)lpvThreadParam;
	if(!pThis)
		return 0;

	pThis->InitializeScan();
	pThis->StartScanning();

	return 1;
}

void CWWizAdwareCleanerDlg::InitializeScan()
{
	m_bScanning = false;
	m_dwScannedCount = 0x00;
	m_dwDetectedCount = 0x00;
	m_dwRemovedCount = 0x00;
	m_dwDataCount = 0x00;
	m_dwLocationCount = 0x00;
	m_dwTotalIterations = 0x00;
	//m_btnCancel.SetWindowText(L"Stop");
	//m_btnScan.ShowWindow(FALSE);
	//m_btnClean.ShowWindow(FALSE);
	//m_btnPauseResume.ShowWindow(TRUE);
}

void CWWizAdwareCleanerDlg::OnBnClickedButtonStop()
{
	if (m_hThreadScan != NULL)
	{
		::SuspendThread(m_hThreadScan);
		::TerminateThread(m_hThreadScan, 0x00);
	}
	OnCancel();
}

/***************************************************************************************************
*  Function Name  : OnBtnStopCleaning
*  Description    : Function to stop cleaning
*  Author Name    : Jeena Mariam Saji
*  Date			  : 15 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnBtnStopCleaning()
{
	if (m_hThreadScan != NULL)
	{
		::SuspendThread(m_hThreadScan);
		::TerminateThread(m_hThreadScan, 0x00);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : OnStartAdwareScan
*  Description    : Function to pause Adware Scan
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnBtnClickPause()
{
	OnBnClickedButtonPause();
	return json::value();
}

void CWWizAdwareCleanerDlg::OnBnClickedButtonPause()
{
	if (!m_bScanning)
		return;

	PauseScan();
}

bool CWWizAdwareCleanerDlg::PauseScan()
{
	if (m_hThreadScan != NULL)
	{
		::SuspendThread(m_hThreadScan);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : OnStartAdwareScan
*  Description    : Function to resume Adware Scan
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnBtnClickResume()
{
	ResumeScan();
	return json::value();
}

void CWWizAdwareCleanerDlg::OnBnClickedButtonResume()
{
	ResumeScan();
}

bool CWWizAdwareCleanerDlg::ResumeScan()
{
	if (m_hThreadScan != NULL)
	{
		::ResumeThread(m_hThreadScan);
	}
	return true;
}
void CWWizAdwareCleanerDlg::StartScanning()
{
	m_bScanning = true;
	//SetStatus(L"Scanning in progress...");
	SetTimer(USER_PROGRESS_TIMER, 1000, NULL);

	m_dwLocationCount = m_objAdwScan.GetTotalLocationCount();
	m_dwDataCount = m_objAdwScan.GetTotalDataCount();

	m_dwTotalIterations = GetTotalNumberOfIterations();

	for (int iType = SERVICESLOC; iType <= BROWSERSREGISTRYDATA; iType+= 2)
	{
		VECSTRING & vecLocations = m_objAdwScan.GetVectorDetails((ENUMTYPE)iType);
		if (vecLocations.size() == 0)
			continue;

		VECSTRING & vecData = m_objAdwScan.GetVectorDetails((ENUMTYPE)(iType + 1));
		if (vecLocations.size() == 0)
			continue;

		if (vecData.size() == 0)
			continue;

		for (VECSTRING::iterator it = vecData.begin(); it != vecData.end(); ++it)
		{
			string strData = *it;
			for (VECSTRING::iterator it = vecLocations.begin(); it != vecLocations.end(); ++it)
			{
				string strLocation = *it;
				CString cs2Display;
				bool bResult = false;

				TCHAR sztStrLocation[MAX_PATH] = { 0 };
				TCHAR sztStrData[MAX_PATH] = { 0 };
				// Convert char* string to a wchar_t* string.  
				size_t convertedChars = 0;
				mbstowcs_s(&convertedChars, sztStrLocation, strlen(strLocation.c_str()) + 1, strLocation.c_str(), _TRUNCATE);
				mbstowcs_s(&convertedChars, sztStrData, strlen(strData.c_str()) + 1, strData.c_str(), _TRUNCATE);

				CString csLocation = sztStrLocation;
				CString csData = sztStrData;

				switch ((ENUMTYPE)(iType) + 1)
				{
				case SERVICESDATA:
					bResult = Check4Registry(csLocation, csData, cs2Display);
					break;
				case FOLDERSDATA:
					bResult = Check4FileFolders(csLocation, csData, cs2Display);
					break;
				case FILESDATA:
					bResult = Check4FileFolders(csLocation, csData, cs2Display);
					break;
				case SHORTCUTSDATA:
					bResult = Check4FileFolders(csLocation, csData, cs2Display);
					break;
				case TASKSDATA:
					bResult = Check4FileFolders(csLocation, csData, cs2Display);
					break;
				case REGISTRYDATA:
					bResult = Check4Registry(csLocation, csData, cs2Display);
					break;
				case BROWSERSFILESDATA:
					bResult = Check4FileFolders(csLocation, csData, cs2Display);
					break;
				case BROWSERSREGISTRYDATA:
					bResult = Check4Registry(csLocation, csData, cs2Display);
					break;
				default:
					break;
				}

				if (bResult)
				{
					m_dwDetectedCount++;
					InsertData((ENUMTYPE)(iType + 1), csData, cs2Display);
					AddLogEntry(L"### DETECTED: Adware Name: [%s], Path: [%s]", csData, cs2Display, true, SECONDLEVEL);
				}
			}
			m_dwScannedCount++;
			//DisplayResult();
		}
		//vecData.clear();
		//vecLocations.clear();
	}

	ScanFinished();
	KillTimer(USER_PROGRESS_TIMER);
}

void CWWizAdwareCleanerDlg::InsertData(ENUMTYPE eType, CString csThreatName, CString csPath)
{
	CString csType;
	switch (eType)
	{
	case SERVICESDATA:
		csType = L"Services";
		break;
	case FOLDERSDATA:
		csType = L"Folder";
		break;
	case FILESDATA:
		csType = L"File";
		break;
	case SHORTCUTSDATA:
		csType = L"Shortcut";
		break;
	case TASKSDATA:
		csType = L"Task";
		break;
	case REGISTRYDATA:
		csType = L"Registry";
		break;
	case BROWSERSFILESDATA:
		csType = L"BrowserEntry";
		break;
	case BROWSERSREGISTRYDATA:
		csType = L"BrowserRegEntry";
		break;
	default:
		break;
	}

	OnCallAdwareEntryFunction(csThreatName, csPath, L"Detected", csType);
}

void CWWizAdwareCleanerDlg::OnCallAdwareEntryFunction(CString csThreatName, CString csPath, CString csStatus, CString csType)
{
	m_svAddAdwareEntryCB.call(SCITER_STRING(csThreatName), SCITER_STRING(csPath), SCITER_STRING(csStatus), SCITER_STRING(csType));
}

bool CWWizAdwareCleanerDlg::Check4FileFolders(CString csLocation, CString csData, CString &cs2Display)
{
	bool bReturn = false;
	CString csFullPath;
	csFullPath.Format(L"%s\\%s", csLocation, csData);
	if (PathFileExists(csFullPath))
	{
		cs2Display = csFullPath;
		bReturn = true;
	}
	return bReturn;
}

bool CWWizAdwareCleanerDlg::Check4Registry(CString csLocation, CString csData, CString &cs2Display)
{
	bool bReturn = false;

	CString csRegHive = csLocation.Left(csLocation.Find(L'\\'));
	CString csRegPath = csLocation.Right(csLocation.GetLength()  - (csLocation.Find(L'\\') + 1));

	HKEY hKey = HKEY_LOCAL_MACHINE;
	if (csRegHive == L"HKCR")
	{
		hKey = HKEY_CLASSES_ROOT;
	}
	else if (csRegHive == L"HKCU")
	{
		hKey = HKEY_CURRENT_USER;
	}
	else if (csRegHive == L"HKLM")
	{
		hKey = HKEY_LOCAL_MACHINE;
	}
	else if (csRegHive == L"HKU")
	{
		hKey = HKEY_USERS;
	}
	else if (csRegHive == L"HKCC")
	{
		hKey = HKEY_CURRENT_CONFIG;
	}
	else
	{
		return bReturn;
	}

	csRegPath += L"\\";
	csRegPath += csData;

	HKEY	hSubKey = NULL;
	RegOpenKeyEx(hKey, csRegPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey);
	if (!hSubKey)
	{
		return bReturn;
	}

	cs2Display = csLocation + L"\\" + csData;
	bReturn = true;
	return bReturn;
}

void CWWizAdwareCleanerDlg::DisplayResult()
{

	CString csScannedCount;
	csScannedCount.Format(L"Scanned: %d", m_dwScannedCount);
	CString csDetectedCount;
	csDetectedCount.Format(L"Found: %d", m_dwDetectedCount);
	CString csCleanedCount;
	csCleanedCount.Format(L"Cleaned: %d", m_dwRemovedCount);

	sciter::value map;
	map.set_item("one", (SCITER_STRING)csScannedCount);
	map.set_item("two", (SCITER_STRING)csDetectedCount);
	map.set_item("three", (SCITER_STRING)csCleanedCount);

	sciter::dom::element ela = m_root_el;
	BEHAVIOR_EVENT_PARAMS params;
	params.cmd = SETFILEPATH_EVENT_CODE;
	params.he = params.heTarget = ela;
	params.data = map;
	ela.fire_event(params, true);
}

void CWWizAdwareCleanerDlg::ScanFinished()
{
	m_svSetScanFinishedStatus.call();
	m_bScanning = false;

	//m_prgState.SetPos(100);

	//SetStatus(L"Scanning completed...");
	//m_btnScan.ShowWindow(TRUE);
	//m_btnPauseResume.ShowWindow(FALSE);
	//m_btnCancel.SetWindowText(L"Cancel");

	if (m_dwDetectedCount > 0)
	{
		//SetStatus(L"Scanning completed, Please verify entries and press clean button.");
		//m_btnClean.ShowWindow(TRUE);
		//m_btnScan.ShowWindow(FALSE);
	}
	Invalidate();
}

void CWWizAdwareCleanerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == USER_PROGRESS_TIMER)
	{
		float iPercentage = ((float)m_dwScannedCount / (float)m_dwTotalIterations) * 100;
		CString csString;
		csString.Format(L"%d", static_cast<DWORD>(iPercentage));
		if (iPercentage <= 100)
		{
			//m_prgState.SetPos((int)iPercentage);
			m_svSetScanPercentage.call();
		}
	}
	CDialog::OnTimer(nIDEvent);
}

/***************************************************************************************************
*  Function Name  : OnCloseUI
*  Description    : Function to close UI
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnCloseUI()
{
	OnClose();
	return json::value();
}

/***************************************************************************************************
*  Function Name  : OnBtnClickClean
*  Description    : Function to clean adware entries
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnBtnClickClean(SCITER_VALUE svArrayRecords, SCITER_VALUE svOnUpdateUIStatus)
{
	bool bArray = false;
	svArrayRecords.isolate();
	bArray = svArrayRecords.is_array();
	m_svArrayRecords = svArrayRecords;
	m_svOnUpdateUIStatus = svOnUpdateUIStatus;
	if (!bArray)
	{
		return false;
	}
	OnBnClickedButtonClean();
	return json::value();
}

void CWWizAdwareCleanerDlg::OnClose()
{
	OnBnClickedButtonStop();
}

DWORD CWWizAdwareCleanerDlg::GetTotalNumberOfIterations()
{
	return m_dwDataCount;
}

void CWWizAdwareCleanerDlg::OnBnClickedButtonClean()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		DWORD dwTotDetectedFiles = 0x00;
		dwTotDetectedFiles = m_svArrayRecords.length();
		int iDetectedFiles = 0;
		CString csTotalCleanedCount;
		CString csFullPathQuarantine = L"";
		bool bRestartRequired = false;
		CString csInsertQuery;

		CString csQuarantinePath = theApp.GetModuleFilePath() + L"\\Quarantine";
		if (!PathFileExists(csQuarantinePath))
		{
			CreateDirectory(csQuarantinePath, NULL);
		}

		CString	csWardWizModulePath = theApp.GetModuleFilePath();
		CString	csWardWizAdwPath = L"";
		csWardWizAdwPath.Format(L"%s\\WWIZADWCLEANER.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizAdwPath))
		{
			CT2A dbPath(csWardWizAdwPath, CP_UTF8);
			dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

			dbSQlite.Open();

			if (!dbSQlite.TableExists("WWIZADWCLEANER"))
			{
				dbSQlite.ExecDML("CREATE TABLE [WWIZADWCLEANER] (\
				[db_ScanID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
				[db_VirusName] NVARCHAR(256)  NULL,\
				[db_OrigPath] NVARCHAR(512)  NULL,\
				[db_QuarPath] NVARCHAR(256)  NULL,\
				[db_IsFileFold] BOOLEAN NULL,\
				[db_IsRestartReqd] BOOLEAN NULL\
				)");
			}
			dbSQlite.Close();
		}
		for (DWORD dwIndex = 0x00; dwIndex < dwTotDetectedFiles; dwIndex++)
		{
			//if (m_lstFoundEntries.GetCheck(dwIndex))
			//{
			const SCITER_VALUE svEachEntry = m_svArrayRecords[dwIndex];
			const std::wstring chThreatName = svEachEntry[L"ThreatName"].get(L"");
			const std::wstring chFilePath = svEachEntry[L"FilePath"].get(L"");
			const std::wstring chActionTaken = svEachEntry[L"ActionTaken"].get(L"");
			const std::wstring chThreatType = svEachEntry[L"ThreatType"].get(L"");

			bool bValue = svEachEntry[L"selected"].get(false);
			if (!bValue)
				continue;
			CString csName			= chThreatName.c_str();
			CString csPath			= chFilePath.c_str();
			CString csActionTaken   = chActionTaken.c_str();
			CString csType			= chThreatType.c_str();
			int iIsFileFold = 0;
			int iIsRestartReqd = 0;

			if (csActionTaken != "Cleaned")
			{
				if (csType == L"Services")
				{
					if (UnInstallService(csName))
					{
						m_dwRemovedCount++;
					}
					m_lstFoundEntries.SetItemText(dwIndex, 2, L"Removed");
				}
				else if (csType == L"Folder")
				{
					iIsFileFold = 0;
					csFullPathQuarantine = csQuarantinePath + L"\\" + csName;

					TCHAR szSourceString[1000];
					lstrcpyn(szSourceString, csPath, 1000);

					TCHAR szDestString[1000];
					lstrcpyn(szDestString, csFullPathQuarantine, 1000);

					theApp.OnCopyFolder(szSourceString, szDestString);

					EmptyDirectory(csPath.GetBuffer(), bRestartRequired, true);
					if (!RemoveDirectory(csPath))
					{
						if (!MoveFileEx(csPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
						{
							AddLogEntry(L"### MoveFileEx failed for file: [%s]", csPath, 0, true, SECONDLEVEL);
						}
						else
						{
							bRestartRequired = true;
							iIsRestartReqd = 1;
						}
					}
					m_dwRemovedCount++;
				}
				else if (csType == L"File")
				{
					iIsFileFold = 1;
					csFullPathQuarantine = csQuarantinePath + L"\\" + csName;
					CopyFile(csPath, csFullPathQuarantine, false);
					if (RemoveFile(csPath, bRestartRequired))
					{
						m_dwRemovedCount++;
					}
				}
				else if (csType == L"Shortcut")
				{
					if (RemoveFile(csPath, bRestartRequired))
					{
						m_dwRemovedCount++;
					}
				}
				else if (csType == L"Task")
				{
					if (RemoveFile(csPath, bRestartRequired))
					{
						m_dwRemovedCount++;
					}
				}
				else if (csType == L"Registry")
				{
					if (RemoveRegistry(csPath))
					{
						m_dwRemovedCount++;
					}
				}
				else if (csType == L"BrowserEntry")
				{
					if (RemoveFile(csPath, bRestartRequired))
					{
						m_dwRemovedCount++;
					}
				}
				else if (csType == L"BrowserRegEntry")
				{
					if (RemoveRegistry(csPath))
					{
						m_dwRemovedCount++;
					}
				}
				else
				{
					//Invalid
					continue;
				}
			}
			//}
			csInsertQuery = _T("INSERT INTO WWIZADWCLEANER VALUES (null,");
			csInsertQuery.Format(_T("INSERT INTO WWIZADWCLEANER VALUES (null,'%s','%s','%s', '%d', '%d');"), csName, csPath, csFullPathQuarantine, iIsFileFold, iIsRestartReqd);
			CT2A ascii(csInsertQuery, CP_UTF8);
			INT64 iScanId = InsertDataToTable(ascii.m_psz);
		}

		csTotalCleanedCount.Format(L"Cleaned: %d", m_dwRemovedCount);
		m_svOnUpdateUIStatus.call((SCITER_STRING)csTotalCleanedCount, bRestartRequired);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::OnBnClickedButtonClean()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBtnCallTimer
*  Description    : Function to update percentage and details on UI
*  Author Name    : Jeena Mariam Saji
*  Date			  : 19 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnBtnCallTimer()
{
	DisplayResult();
	float iPercentage = ((float)m_dwScannedCount / (float)m_dwTotalIterations) * 100;
	CString csString;
	csString.Format(L"%d", static_cast<DWORD>(iPercentage));
	return (SCITER_STRING)csString;
}

/***************************************************************************************************
*  Function Name  : OnBtnClickReboot
*  Description    : Function to reboot machine
*  Author Name    : Jeena Mariam Saji
*  Date			  : 15 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnBtnClickReboot()
{
	CEnumProcess objEP;
	objEP.RebootSystem(0);
	return json::value();
}
/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : An application-defined function that processes messages sent to a window
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
LRESULT CWWizAdwareCleanerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		LRESULT lResult;
		BOOL    bHandled;

		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
		if (bHandled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.

		if (LOWORD(lParam) == WM_LBUTTONUP)
		{
			RestoreWndFromTray(m_hWnd);
			ShowNotifyIcon(m_hWnd, 0x01, L"WardWiz Adware Cleaner");
			return 0;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  : RestoreWndFromTray
*  Description    : It restores window from tray
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
VOID CWWizAdwareCleanerDlg::RestoreWndFromTray(HWND hWnd)
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
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::RestoreWndFromTray", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ShowNotifyIcon
*  Description    : Notifies the notification area, icon to tray
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
VOID CWWizAdwareCleanerDlg::ShowNotifyIcon(HWND hWnd, DWORD dwAdd, CString csMessage)
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
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::ShowNotifyIcon", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetDoAnimateMinimize
*  Description    : Check to see if the animation has been disabled
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
BOOL CWWizAdwareCleanerDlg::GetDoAnimateMinimize(VOID)
{
	ANIMATIONINFO ai;
	__try
	{
		ai.cbSize = sizeof(ai);
		SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::GetDoAnimateMinimize", 0, 0, true, SECONDLEVEL);
	}
	return ai.iMinAnimate ? TRUE : FALSE;
}

/***************************************************************************************************
*  Function Name  : GetTrayWndRect
*  Description    : It gives tray window rectangle co-ordinates.
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
VOID CWWizAdwareCleanerDlg::GetTrayWndRect(LPRECT lpTrayRect)
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
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::GetTrayWndRect", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnLoadDB
*  Description    : Function to load adware.db
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnLoadDB(SCITER_VALUE svFunGetLoadStatus, SCITER_VALUE svFunCheckInternet, SCITER_VALUE svFunCheckforUninstall)
{
	m_svFunGetLoadStatus = svFunGetLoadStatus;
	m_svFunCheckInternet = svFunCheckInternet;
	m_svFunCheckforUninstall = svFunCheckforUninstall;

	bool bReturn = false;
	bReturn = CheckInternetConnection();
	if (bReturn)
	{
		m_svFunCheckInternet.call(1);
	}
	else
	{
		m_svFunCheckInternet.call(0);
	}

	bool bCheck = false;
	bCheck = CheckforUninstall();
	if (bCheck)
	{
		m_svFunCheckforUninstall.call(1);	//wardwiz already installed
	}
	else
	{
		m_svFunCheckforUninstall.call(0);
	}

	OnLoadDatabase();
	return json::value();
}

void CWWizAdwareCleanerDlg::OnLoadDatabase()
{
	::CreateThread(NULL, 0, LoadDataBase, (LPVOID) this, 0, NULL);
}

/***************************************************************************************************
*  Function Name  : CheckInternetConnection
*  Description    : Function to check internet connection
*  Author Name    : Jeena Mariam Saji
*  Date			  : 20 June 2017
****************************************************************************************************/
bool CWWizAdwareCleanerDlg::CheckInternetConnection()
{
	bool bReturn = false;

	CWinHttpManager objWinHttpManager;
	TCHAR szTestUrl[MAX_PATH] = { 0 };
	_tcscpy_s(szTestUrl, MAX_PATH, _T("http://www.wardwiz.com"));
	if (objWinHttpManager.Initialize(szTestUrl))
	{
		if (objWinHttpManager.CreateRequestHandle(NULL))
		{
			return true;
		}
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CheckforUninstall
*  Description    : Function to check if WardWiz is already installed before uninstallation
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 June 2017
****************************************************************************************************/
bool CWWizAdwareCleanerDlg::CheckforUninstall()
{
	try
	{
		CString csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizAdwPath = theApp.GetModuleFilePath() + L"\\";
		bool bReturn = false;

		if (_tcscmp(csWardWizModulePath, csWardWizAdwPath) == 0)
		{
			bReturn = true;
		}
		else
		{
			bReturn = false;
		}
		return bReturn;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WWizAdwareRecoverDlg::CheckforUninstall", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : HideAllElements
*  Description    : Function to hide MFC controllers
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 June 2017
****************************************************************************************************/
void CWWizAdwareCleanerDlg::HideAllElements()
{
	m_stStatus.ShowWindow(SW_HIDE);
	m_stStatus.ShowWindow(SW_HIDE);
	m_prgState.ShowWindow(SW_HIDE);
	m_stThreatsFound.ShowWindow(SW_HIDE);
	m_btnScan.ShowWindow(SW_HIDE);
	m_btnClean.ShowWindow(SW_HIDE);
	m_btnCancel.ShowWindow(SW_HIDE);
	m_lstFoundEntries.ShowWindow(SW_HIDE);
	m_btnPauseResume.ShowWindow(SW_HIDE);
	m_stScannedCount.ShowWindow(SW_HIDE);
	m_stThreatsCleaned.ShowWindow(SW_HIDE);
}

bool CWWizAdwareCleanerDlg::UnInstallService(CString csSvcName)
{
	bool bReturn = false;

	CISpySrvMgmt	objServ;
	if (objServ.UnInstallService(csSvcName.GetBuffer()) != 0)
	{
		return bReturn;
	}

	bReturn = true;

	return bReturn;
}

void CWWizAdwareCleanerDlg::EmptyDirectory(TCHAR* folderPath, bool &bRestartRequired, bool bTakeBackup)
{
	TCHAR szfileFound[MAX_PATH] = { 0 };
	WIN32_FIND_DATA info;
	HANDLE hp;
	swprintf(szfileFound, L"%s\\*.*", folderPath);
	hp = FindFirstFile(szfileFound, &info);
	do
	{
		if (!((_tcscmp(info.cFileName, L".") == 0) ||
			(_tcscmp(info.cFileName, L"..") == 0)))
		{
			if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ==
				FILE_ATTRIBUTE_DIRECTORY)
			{
				wstring subFolder = folderPath;
				subFolder.append(L"\\");
				subFolder.append(info.cFileName);
				EmptyDirectory((TCHAR*)subFolder.c_str(), bRestartRequired, bTakeBackup);
				RemoveDirectory(subFolder.c_str());
			}
			else
			{
				swprintf(szfileFound, L"%s\\%s", folderPath, info.cFileName);
				SetFileAttributes(szfileFound, FILE_ATTRIBUTE_NORMAL);
				if (!DeleteFile(szfileFound))
				{
					if (!MoveFileEx(szfileFound, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
					{
						AddLogEntry(L"### MoveFileEx failed for file: [%s]", szfileFound, 0, true, SECONDLEVEL);
					}
					bRestartRequired = true;
				}
			}
		}

	} while (FindNextFile(hp, &info));
	FindClose(hp);
}

bool CWWizAdwareCleanerDlg::RemoveFile(CString csPath, bool &bRestartRequired)
{
	bool bReturn = false;
	SetFileAttributes(csPath, FILE_ATTRIBUTE_NORMAL);
	if (!DeleteFile(csPath))
	{
		if (!MoveFileEx(csPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
		{
			AddLogEntry(L"### MoveFileEx failed for file: [%s]", csPath, 0, true, SECONDLEVEL);
			return bReturn;
		}
		bRestartRequired = true;
	}

	bReturn = true;

	return bReturn;
}

bool CWWizAdwareCleanerDlg::RemoveRegistry(CString csLocation)
{
	CString csRegHive = csLocation.Left(csLocation.Find(L'\\'));
	CString csRegPath = csLocation.Right(csLocation.GetLength() - (csLocation.Find(L'\\') + 1));

	HKEY hKey = HKEY_LOCAL_MACHINE;
	if (csRegHive == L"HKCR")
	{
		hKey = HKEY_CLASSES_ROOT;
	}
	else if (csRegHive == L"HKCU")
	{
		hKey = HKEY_CURRENT_USER;
	}
	else if (csRegHive == L"HKLM")
	{
		hKey = HKEY_LOCAL_MACHINE;
	}
	else if (csRegHive == L"HKU")
	{
		hKey = HKEY_USERS;
	}
	else if (csRegHive == L"HKCC")
	{
		hKey = HKEY_CURRENT_CONFIG;
	}
	else
	{
		return false;
	}
	CITinRegWrapper objReg;
	return objReg.RegDelnodeRecurse(hKey, csRegPath.GetBuffer()) == TRUE ? true : false;
}

/***************************************************************************************************
*  Function Name  : OnBtnClickMinimise
*  Description    : Function to minimise UI
*  Author Name    : Jeena Mariam Saji
*  Date			  : 14 June 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnBtnClickMinimise()
{
	this->ShowWindow(SW_MINIMIZE);
	return json::value();
}

/***************************************************************************************************
*  Function Name  : InsertDataToTable
*  Description    : Function to insert entries into database
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 July 2017
****************************************************************************************************/
INT64 CWWizAdwareCleanerDlg::InsertDataToTable(const char* szQuery)
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = theApp.GetModuleFilePath();
		CString	csWardWizAdwPath = L"";
		csWardWizAdwPath.Format(L"%s\\WWIZADWCLEANER.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizAdwPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		int iRows = dbSQlite.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = dbSQlite.GetLastRowId();
		dbSQlite.Close();
		return iLastRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::InsertDataToTable()", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : GetWardWizPathFromRegistry
*  Description    : Function to retrieve path from registry
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 July 2017
****************************************************************************************************/
CString CWWizAdwareCleanerDlg::GetWardWizPathFromRegistry()
{
	try
	{
		CITinRegWrapper objReg;
		TCHAR szValue[MAX_PATH] = { 0 };
		DWORD dwSize = sizeof(szValue);
		CString csAppFolderPath;

		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WardWiz", L"AppFolder", szValue, dwSize) == 0)
		{
			csAppFolderPath = szValue;
			return csAppFolderPath;
		}
		return L"";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::GetWardWizPathFromRegistry()", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : OnBtnClickUninstall
*  Description    : Function to uninstall WardWiz Adware Cleaner
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 July 2017
****************************************************************************************************/
json::value CWWizAdwareCleanerDlg::OnBtnClickUninstall()
{
	try
	{
		bool bRestartRequired = false;
		CString csPath = theApp.GetModuleFilePath() + L"\\unins000.exe";
		ShellExecute(NULL, L"Open", csPath, L"/verysilent", NULL, 0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareCleanerDlg::OnBtnClickUninstall()", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}