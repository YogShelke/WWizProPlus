
/*********************************************************************
*  Program Name		: WardWizMemScanDlg.cpp
*  Description		: CWardWizMemScanDlg Implementation
*  Author Name		: Nitin Shelar
*  Date Of Creation	: 21 Nov 2018
**********************************************************************/
#include "stdafx.h"
#include <Psapi.h>
#include "WardWizMemScan.h"
#include "WardWizMemScanDlg.h"
#include "afxdialogex.h"
#include "WrdWizSystemInfo.h"
#include "Enumprocess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		SETFILECOUNT_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 2)
#define		SETFILEPATH_EVENT_CODE			(FIRST_APPLICATION_EVENT_CODE + 1)


DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam);
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam);
DWORD WINAPI QuarantineThread(LPVOID lpParam);

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

/***************************************************************************************************
*  Function Name  : CAboutDlg
*  Description    : C'tor
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name
*  Date           : 21/11/2018
****************************************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CWardWizMemScanDlg * CWardWizMemScanDlg::m_pThis = NULL;

/***************************************************************************
Function Name  : CWardWizMemScanDlg
Description    : Constructor
Author Name    : Nitin Shelar
Date           : 21/11/2018
****************************************************************************/
CWardWizMemScanDlg::CWardWizMemScanDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWardWizMemScanDlg::IDD, pParent), behavior_factory("WardWizMemScanDlg")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pbyEncDecKey = NULL;
	m_pbyEncDecKey = (unsigned char *)calloc(WRDWIZ_KEY_SIZE, sizeof(unsigned char));
	m_hThread_ScanCount = NULL;
	m_hQuarantineThread = NULL;
	m_hWardWizAVThread = NULL;
	m_bFullScan = false;
}

/***************************************************************************
Function Name  : ~CWardWizPreInstallScanDlg
Description    : Dest'r
Author Name    : NITIN SHELAR
Date           : 17/01/2019
SR_NO		   :
****************************************************************************/
CWardWizMemScanDlg::~CWardWizMemScanDlg()
{
	FunCleanup();
	UnLoadSignatureDatabase();

	if (m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name
*  Date           : 21/11/2018
****************************************************************************************************/
void CWardWizMemScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizMemScanDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CWardWizMemScanDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

HWINDOW   CWardWizMemScanDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizMemScanDlg::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Initializes the dialog window
*  Author Name    : Nitin Shelar
*  Date			  : 21/11/2018
****************************************************************************************************/
BOOL CWardWizMemScanDlg::OnInitDialog()
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

	SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_RESCUE.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_RESCUE.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;

	//m_root_el = root();
	sciter::dom::element root = sciter::dom::element::root_element(this->get_hwnd());
	SciterGetElementIntrinsicWidths(root, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(root, pIntMinWidth, &pIntHeight);

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;

	::MoveWindow(this->get_hwnd(), ixRect / 2, iyRect / 2, pIntMaxWidth, pIntHeight, true);
	
	try
	{
		APPBARDATA abd;
		abd.cbSize = sizeof(abd);

		SHAppBarMessage(ABM_GETTASKBARPOS, &abd);

		switch (abd.uEdge)
		{
		case ABE_TOP:
			//SetWindowPos(NULL, ixRect, i, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizPreInstallScanDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	// TODO: Add extra initialization here
	
	DWORD dwData = 1;
	m_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium\\", L"dwHeuScan", dwData);
	m_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium\\", L"dwCachingMethod", dwData);
	
	this->SetWindowText(L"VibraniumMEMSCAN");
	// TODO: Add extra initialization here	
	DWORD dwSigCount = 0x00;
	if (LoadSignatureDatabase(dwSigCount) != 0x00)
	{
		AddLogEntry(L"### Failed to Load SignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************************************
*  Function Name  :  OnSysCommand
*  Description    :  it calls function,when the user selects a command from the Control menu
*  Author Name    :
*  Date           :
****************************************************************************************************/
void CWardWizMemScanDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    :
*  Date           : 21/11/2018
****************************************************************************************************/
void CWardWizMemScanDlg::OnPaint()
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

/***************************************************************************************************
*  Function Name  : OnQueryDragIcon
*  Description    : The framework calls this member function by a minimized
(iconic) window that does not have an icon defined for its class
*  Author Name    : Nitin Shelar
*  Date           : 21/11/2018
****************************************************************************************************/
HCURSOR CWardWizMemScanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Get the product id
*  Author Name    : Nitin Shelar
*  Date           : 29th Oct 2018
****************************************************************************************************/
json::value CWardWizMemScanDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		CString csIniFilePath = GetModuleFileStringPath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";
		if (!PathFileExists(csIniFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetSelectedProductID", csIniFilePath, 0, true, SECONDLEVEL);
			return 0xFFFF;
		}
		
		iProdValue = GetPrivateProfileInt(L"VBSETTINGS", L"ProductID", 1, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}

	return iProdValue;
}

/***************************************************************************************************
*  Function Name  : GetModuleFileStringPath
*  Description    : Get the path where module is exist
*  Author Name    : NITIN SHELAR
*  Date           : 14/01/2019
****************************************************************************************************/
CString CWardWizMemScanDlg::GetModuleFileStringPath()
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::GetModuleFileStringPath", 0, 0, true, SECONDLEVEL);
	}
	return(CString(szModulePath));
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Nitin Shelar
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CWardWizMemScanDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpDialog::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Nitin Shelar
*  Date           : 29th Oct 2018
****************************************************************************************************/
json::value CWardWizMemScanDlg::On_GetLanguageID()
{
	int iLangValue = 0;

	try
	{
		CString csIniFilePath = GetModuleFileStringPath() + L"\\VBSETTINGS" + L"\\ProductSettings.ini";

		if (!PathFileExists(csIniFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetSelectedLanguage", csIniFilePath, 0, true, SECONDLEVEL);
			return 0xFFFF;
		}

		iLangValue = GetPrivateProfileInt(L"VBSETTINGS", L"LanguageID", 0, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************
Function Name  : OnBnClickedCancel
Description    : Cancel button click handler
Author Name    : Nitin Shelar
Date           : 21/11/2018
****************************************************************************/
void CWardWizMemScanDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : To handle the Sciter UI related requests
*  Author Name    : Nitin Shelar
*  Date           : 29th Oct 2018
****************************************************************************************************/
LRESULT CWardWizMemScanDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	__try
	{
		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
		
		if (bHandled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Shelar
Date           : 29th Oct 2018
/***************************************************************************************************/
json::value CWardWizMemScanDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_Close
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Shelar
Date           : 29th Oct 2018
/***************************************************************************************************/
json::value CWardWizMemScanDlg::On_Close()
{
	try
	{
		CDialogEx::OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_Close", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_Minimize
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Shelar
Date           : 29th Oct 2018
/***************************************************************************************************/
json::value CWardWizMemScanDlg::On_Minimize()
{
	try
	{
		this->ShowWindow(SW_MINIMIZE);
		return json::value();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_Minimize", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	EnumFolder
*  Description    :	Enumerate each folders of system and calculate total files count.
*  Author Name    :	NITIN SHELAR
*  Date           : 31 Oct 2018
**********************************************************************************************************/
void CWardWizMemScanDlg::EnumFolder(LPCTSTR pstr)
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

				if (m_bManualStop)
					break;

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
		AddLogEntry(L"### Exception in CWardwizScan::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTotalScanFilesCount
Description    : Get total files count in case of fullscan and custom scan
Author Name    : NITIN SHELAR
Date           : 31 Oct 2018
****************************************************************************/
DWORD WINAPI GetTotalScanFilesCount(LPVOID lpParam)
{
	try
	{
		CWardWizMemScanDlg *pThis = (CWardWizMemScanDlg*)lpParam;
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
		AddLogEntry(L"### Exception in CWardwizScan::GetTotalScanFilesCount", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : EnumerateThread
Description    : Thread to enumerarte process in case of quick scan and
				 files and folders in case of custom and full scan.
Author Name    : NITIN SHELAR
Date           : 31 Oct 2018
****************************************************************************/
DWORD WINAPI EnumerateThread(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizMemScanDlg *pThis = (CWardWizMemScanDlg*)lpvThreadParam;
		if (!pThis)
			return 0;
		int	iIndex = 0x00;
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
			pThis->ScanFinished();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::EnumerateThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	EnumFolderForScanning
*  Description    :	enumerate files of system and sent it to scan.
*  Author Name    :	NITIN SHELAR
*  Date           : 31 Oct 2018
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizMemScanDlg::EnumFolderForScanning(LPCTSTR pstr)
{
	try
	{
		if (!pstr)
			return;

		if (m_bManualStop)
		{
			return;
		}

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

					if (m_bManualStop)
					{
						break;
					}

					if (PathFileExists(csFilePath))
					{
						m_FileScanned++;
						ScanForSingleFile(csFilePath);
					}
				}
			}
			Sleep(10);
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::EnumFolderForScanning", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : CheckFileOrFolderOnRootPath
Description    : Function which check IS file/Folder present on root path
Author Name    : NITIN SHELAR
Date           : 31 Oct 2018
/***************************************************************************************************/
bool CWardWizMemScanDlg::CheckFileOrFolderOnRootPath(CString csFilePath)
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CheckFileOrFolderOnRootPath, Path: %s", csFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	StartScanning
*  Description    :	To start Full scan ,custom scan and quick scan accoeding m_scantype variable.
*  Author Name    : NITIN SHELAR
*  Date           : 31 Oct 2018
**********************************************************************************************************/
void CWardWizMemScanDlg::StartScanning()
{
	try
	{
		m_bIsPathExist = false;
		m_bManualStop = false;
		m_iThreatsFoundCount = 0;
		m_ScanCount = false;
		m_FileScanned = 0;
		m_csPreviousPath = L"";
		m_eScanType = m_eCurrentSelectedScanType;
		
		if (!GetScanningPaths(m_csaAllScanPaths))
		{
			return;
		}

		m_iTotalFileCount = 0;
		m_dwTotalFileCount = 0;
		m_iMemScanTotalFileCount = 0;
		m_dwVirusFoundCount = 0;
		m_dwVirusCleanedCount = 0;

		if (m_hThread_ScanCount != NULL)
			m_hThread_ScanCount = NULL;

		m_hThread_ScanCount = ::CreateThread(NULL, 0, GetTotalScanFilesCount, (LPVOID) this, 0, NULL);
		Sleep(500);

		DWORD m_dwThreadId = 0;
		
		if (m_hWardWizAVThread != NULL)
			m_hWardWizAVThread = NULL;

		m_hWardWizAVThread = ::CreateThread(NULL, 0, EnumerateThread, (LPVOID) this, 0, &m_dwThreadId);
		Sleep(500);
		CString csScanStarted = L"";
		switch (m_eScanType)
		{
		case FULLSCAN:
			csScanStarted = L">>> Full scanning started...";
			break;
		case CUSTOMSCAN:
			csScanStarted = L">>> Custom scanning started...";
			break;
		default:
			csScanStarted = L">>> Scanning started...";
			break;
		}

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
		AddLogEntry(L"### Exception in CScanDlg::StartScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetScanningPaths
*  Description    :	Get scan path according to scanning types.
*  Author Name    : NITIN SHELAR
*  Date           : 31 Oct 2018
**********************************************************************************************************/
bool CWardWizMemScanDlg::GetScanningPaths(CStringArray &csaReturn)
{
	try
	{

		switch (m_eScanType)
		{
		case FULLSCAN:
			csaReturn.RemoveAll();
			if (!GetAllDrivesList(csaReturn))
			{
				return false;
			}
			break;
		case CUSTOMSCAN:
		{
			m_bCustomscan = true;
			bool bIsArray = false;
			m_svArrCustomScanSelectedEntries.isolate();
			bIsArray = m_svArrCustomScanSelectedEntries.is_array();
			if (!bIsArray)
			{
				return false;
			}
			csaReturn.RemoveAll();
			for (unsigned iCurrentValue = 0, count = m_svArrCustomScanSelectedEntries.length(); iCurrentValue < count; iCurrentValue++)
			{
				const SCITER_VALUE EachEntry = m_svArrCustomScanSelectedEntries[iCurrentValue];
				const std::wstring chFilePah = EachEntry[L"File_Path"].get(L"");
				bool bValue = EachEntry[L"selected"].get(false);
				if (bValue)
				{
					csaReturn.Add(chFilePah.c_str());
				}
			}
		}
		break;
		case USBSCAN:
		case USBDETECT:
			break;
		}
		if (csaReturn.GetCount() > 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::GetScanningPaths", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetAllDrivesList
*  Description    :	Makes list of drives present on a system.
*  Author Name    : NITIN SHELAR
*  Date           : 31 Oct 2018
**********************************************************************************************************/
bool CWardWizMemScanDlg::GetAllDrivesList(CStringArray &csaReturn)
{
	csaReturn.RemoveAll();
	bool bReturn = false;
	CString csDrive;
	int iCount = 0;

	for (char chDrive = 'A'; chDrive <= 'Z'; chDrive++)
	{
		csDrive.Format(L"%c:", chDrive);
		if (PathFileExists(csDrive))
		{
			csaReturn.Add(csDrive);
			bReturn = true;
		}
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name	 :	Check4DBFiles
*  Description		 :	Checks for signature db are exist or not
*  Author Name		 :  NITIN SHELAR
*  Date				 :  31 Oct 2018
*  Modification Date :  6 Jan 2015 Neha Gharge
*  MOdification		 :  Clam And WardWiz Scanner Handle by preprocessor
**********************************************************************************************************/
bool CWardWizMemScanDlg::Check4DBFiles()
{
	DWORD dwDBVersionLength = 0;
	TCHAR szModulePath[MAX_PATH] = { 0 };
	//CWrdwizEncDecManager objWrdwizEncDecMgr;

	if (!GetModulePath(szModulePath, MAX_PATH))
	{
		return false;
	}
	CString csDBFilesFolderPath = szModulePath;
	CString csWRDDBFilesFolderPath = szModulePath;
	csDBFilesFolderPath += L"\\DB";
	csWRDDBFilesFolderPath += L"\\VBDB";
	if (!PathFileExists(csDBFilesFolderPath) && !PathFileExists(csWRDDBFilesFolderPath))
	{
		return false;
	}
	else if (!Check4ValidDBFiles(csWRDDBFilesFolderPath))
	{
		return false;
	}
	else
	{
		CStringArray csaDBFiles;
		if (theApp.m_eScanLevel == WARDWIZSCANNER)
		{
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\VIBRANIUMAV1.DB");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\VIBRANIUMAVR.DB");
		}
		else
		{
			csaDBFiles.Add(csDBFilesFolderPath + L"\\MAIN.CVD");
			csaDBFiles.Add(csDBFilesFolderPath + L"\\DAILY.CLD");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\VIBRANIUMAV1.DB");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\VIBRANIUMAVR.DB");
		}

		for (int iIndex = 0; iIndex < csaDBFiles.GetCount(); iIndex++)
		{
			if (!PathFileExists(csaDBFiles.GetAt(iIndex)))
			{
				return false;
			}
		}
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	Check4ValidDBFiles
*  Description    :	This function will check for valid signature and valid Version length in DB files
if any mismatch found, will return false otherwise true.
*  Author Name    : Nitin Shelar
*  Date           : 31 Oct 2018
**********************************************************************************************************/
bool CWardWizMemScanDlg::Check4ValidDBFiles(CString csDBFolderPath)
{
	try
	{
		CString csFilePath;
		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAV1);
		DWORD dwDBVersionLength = 0;
		DWORD dwDBMajorVersion = 0;


		//DB Version lenfth should be in between 7 and 19
		//Eg: 1.0.0.0 to 9999.9999.9999.9999
		if (!(dwDBVersionLength >= 7 && dwDBVersionLength <= 19))
		{
			AddLogEntry(L"### Invalid DB Version length, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAVR);


		//DB Version lenfth should be in between 7 and 19
		//Eg: 1.0.0.0 to 9999.9999.9999.9999
		if (!(dwDBVersionLength >= 7 && dwDBVersionLength <= 19))
		{
			AddLogEntry(L"### Invalid DB Version length, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::Check4ValidDBFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : On_StartQuickScan
*  Description    : Accepts the request from UI and starts the Quick scan
*  Author Name    : Nitin Shelar
*  Date           : 31 Oct 2018
****************************************************************************************************/
json::value CWardWizMemScanDlg::On_StartFullScan(SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB, SCITER_VALUE svFunNotificationMessageCB)
{
	try
	{
		theApp.m_bIsScanning = true;
		m_bFullScan = true;
		m_eCurrentSelectedScanType = FULLSCAN;
		m_svAddVirusFoundEntryCB = svFunAddVirusFoundEntryCB;
		m_svSetScanFinishedStatusCB = svFunSetScanFinishedStatusCB;
		m_svFunNotificationMessageCB = svFunNotificationMessageCB;
		StartScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_StartFullScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_StartQuickScan
*  Description    : Accepts the request from UI and starts the Quick scan
*  Author Name    : Nitin Shelar
*  Date			  : 31 Octo 2018
****************************************************************************************************/
json::value CWardWizMemScanDlg::On_StartCustomScan(SCITER_VALUE svArrCustomScanSelectedEntries, SCITER_VALUE svStatusFunctionCB, SCITER_VALUE svFunAddVirusFoundEntryCB, SCITER_VALUE svFunSetScanFinishedStatusCB, SCITER_VALUE svFunNotificationMessageCB)
{
	try
	{
		theApp.m_bIsScanning = true;
		m_bCustomscan = true;
		m_eCurrentSelectedScanType = CUSTOMSCAN;
		m_svArrCustomScanSelectedEntries = svArrCustomScanSelectedEntries;
		m_svAddVirusFoundEntryCB = svFunAddVirusFoundEntryCB;
		m_svSetScanFinishedStatusCB = svFunSetScanFinishedStatusCB;
		m_svFunNotificationMessageCB = svFunNotificationMessageCB;
		//AddDeleteEditCustomScanEntryToINI(m_svArrCustomScanSelectedEntries);
		StartScanning();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_StartCustomScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
Function Name  : on_timer
Description    : On timer
Author Name    : Nitin Shelar
Date           : 31 Oct 2018
/***************************************************************************************************/
bool CWardWizMemScanDlg::on_timer(HELEMENT he)
{
	__try
	{
		return  on_timerSEH(he);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::on_timer", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : on_timer
Description    : On timer
Author Name    : Nitin Shelar
Date           : 31 Oct 2018
/***************************************************************************************************/
bool CWardWizMemScanDlg::on_timer(HELEMENT he, UINT_PTR extTimerId)
{
	try
	{
		/*CallUISetStatusfunction(m_csCurrentFilePath);
		CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
		CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
		CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
		m_csPreviousPath = m_csCurrentFilePath;
		AddLogEntry(L"### Called CVibraniumMemScanDlg::on_timer()", 0, 0, true, SECONDLEVEL);*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::on_timer(HELEMENT he, UINT_PTR extTimerId)", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : on_timerSEH
Description    : On timer
Author Name    : Nitin Shelar
Date           : 31 Oct 2018
/***************************************************************************************************/
bool CWardWizMemScanDlg::on_timerSEH(HELEMENT he)
{
	CallUISetStatusfunction(m_csCurrentFilePath.GetBuffer());
	CString csTotalFileCount; csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
	CString csCurrentFileCount; csCurrentFileCount.Format(L"%d", m_FileScanned);
	CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
	//m_csPreviousPath = m_csCurrentFilePath;
	return true;
}

/***************************************************************************************************
*  Function Name  : callUISetStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : NITIN SHELAR
*  Date			  : 31 Oct 2018
****************************************************************************************************/
void CWardWizMemScanDlg::CallUISetStatusfunction(LPTSTR lpszPath)
{
	__try
	{
		CallUISetStatusfunctionSEH(lpszPath);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CallUISetStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallUISetStatusfunctionSEH
*  Description    : Calls call_function to invoke ant UI function
*  Note           : No need to add exception handling because
*  Author Name    : NITIN SHELAR
*  Date			  : 31 Oct 2018
****************************************************************************************************/
void CWardWizMemScanDlg::CallUISetStatusfunctionSEH(LPTSTR lpszPath)
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

	GetShortPathName(csFolderPath, m_szActiveScanFilePath, 40);
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

/***************************************************************************************************
*  Function Name  : CallUISetFileCountfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Shelar
*  Date           : 31 Oct 2018
****************************************************************************************************/
void CWardWizMemScanDlg::CallUISetFileCountfunction(CString csTotalFileCount, CString csCurrentFileCount)
{
	try
	{
		sciter::value map;
		map.set_item("one", sciter::string(csCurrentFileCount));
		map.set_item("two", sciter::string(csTotalFileCount));
		//Send here event
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETFILECOUNT_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::callUIfunction", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	ScanForSingleFile
*  Description    :	Scan each single file .
*  Author Name    : Nitin Shelar
*  Date           : 31 Oct 2018
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizMemScanDlg::ScanForSingleFile(CString csFilePath)
{
	if (csFilePath.GetLength() == 0)
		return;
	try
	{
		bool bSetStatus = false;
		bool bVirusFound = false;
		bool bRescan = true;
		DWORD dwISpyID = 0x00;

		CString csVirusName(L"");
		CString csVirusPath(L"");
		DWORD dwISpywareID = 0;
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
					DWORD dwISpyID = 0;
					TCHAR szVirusName[MAX_PATH] = { 0 };
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
			csStatus.Format(L"%s", GetString(L"IDS_CONSTANT_THREAT_DETECTED"));
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
		AddLogEntry(L"### Exception in void CWardwizScan::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_PasueCustomScan
*  Description    : Accepts the request from UI and Pasue Custom scan
*  Author Name    : Nitin Shelar
*  Date			  : 1 Nov 2018
****************************************************************************************************/
json::value CWardWizMemScanDlg::On_PauseCustomScan(SCITER_VALUE svFunPauseResumeFunCB)
{
	try
	{
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS);
		}

		if (m_bFullScan)
		{
			m_svSetPauseStatusCB = svFunPauseResumeFunCB;
			if (PauseScan())
				m_bFullScan = false;
		}
		else if (m_bCustomscan)
		{
			m_svSetPauseStatusCB = svFunPauseResumeFunCB;
			if (PauseScan())
				m_bCustomscan = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_PasueResumeCustomScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***********************************************************************************************
*  Function Name  : PauseScan
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    : Nitin Shelar
*  Date			  : 1 Nov 2018
*************************************************************************************************/
bool CWardWizMemScanDlg::PauseScan()
{
	try
	{
		if (m_hThread_ScanCount != NULL)
		{
			::SuspendThread(m_hThread_ScanCount);
		}

		if (m_hWardWizAVThread != NULL)
		{
			::SuspendThread(m_hWardWizAVThread);
			CallUISetPauseStatusfunction(GetString(L"IDS_STATUS_SCAN_PAUSE"));
			AddLogEntry(L">>> Scanning Paused::m_hVibraniumAVThread", 0, 0, true, FIRSTLEVEL);
		}
		else
		{
			AddLogEntry(L"### Failed to pause scan::m_hVibraniumAVThread", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::PauseScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : callUISetPauseStatusfunction
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Shelar
*  Date			  : 1 Nov 2018
****************************************************************************************************/
void CWardWizMemScanDlg::CallUISetPauseStatusfunction(CString csData)
{
	try
	{
		m_svSetPauseStatusCB.call(SCITER_STRING(csData));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CallUISetPauseStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_PasueCustomScan
*  Description    : Accepts the request from UI and Pasue Custom scan
*  Author Name    : Nitin shelar
*  Date			  : 1 nov 2018
****************************************************************************************************/
json::value CWardWizMemScanDlg::On_ResumeCustomScan(SCITER_VALUE svFunPauseResumeFunCB)
{
	try
	{
		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::SetTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS, 1000, NULL);
		}

		if ( !m_bFullScan)
		{
			m_svSetPauseStatusCB = svFunPauseResumeFunCB;
			if (!ResumeScan())
			{
				AddLogEntry(L"### Exception in CVibraniumMemScanDlg::ResumeScan", 0, 0, true, SECONDLEVEL);
				m_bFullScan = false;
			}
			else
			{
				m_bFullScan = true;
			}
		}
		else if (!m_bCustomscan)
		{
			m_svSetPauseStatusCB = svFunPauseResumeFunCB;
			if (!ResumeScan())
			{
				AddLogEntry(L"### Exception in CVibraniumMemScanDlg::ResumeScan", 0, 0, true, SECONDLEVEL);
				m_bCustomscan = false;
			}
			else
			{
				m_bCustomscan = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_PasueResumeCustomScan", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***********************************************************************************************
*  Function Name  : ResumeScan
*  Description    : Resume scanning if user click on stop/close button and click to 'No' for stop confirmation box.
*  Author Name    : Nitin shelar
*  Date			  : 1 nov 2018
*************************************************************************************************/
bool CWardWizMemScanDlg::ResumeScan()
{
	try
	{
		if (m_hThread_ScanCount != NULL)
		{
			::ResumeThread(m_hThread_ScanCount);
		}
		else
			return false;

		if (m_hWardWizAVThread != NULL)
		{
			::ResumeThread(m_hWardWizAVThread);
			CallUISetPauseStatusfunction(GetString(L"IDS_STATUS_SCAN_RESUME"));
		}
		else
		{
			AddLogEntry(L"### Failed to pause scan as Send PAUSE_SCAN request failed.", 0, 0, true, SECONDLEVEL);
			return false;
		}
		sciter::dom::element(self).start_timer(500);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::ResumeScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : ScanFinished
*  Description    : display status Scan Finished
*  Author Name    : Nitin shelar
*  Date			  : 1 nov 2018
*************************************************************************************************/
bool CWardWizMemScanDlg::ScanFinished()
{
	CString csCompleteScanning;
	CString csFileScanCount;
	CString csMsgNoFileExist(L"");
	CString csCurrentFileCount;
	CString csTotalFileCount;
	OnTimerScan();

	csCurrentFileCount.Format(L"%d", m_FileScanned);
	csTotalFileCount.Format(L"%d", m_dwTotalFileCount);
	CallUISetFileCountfunction(csTotalFileCount, csCurrentFileCount);
	Sleep(500);
	CallUISetScanFinishedStatus(GetString(L"IDS_STATUS_INFECTEDFILES"));
	m_tsScanEndTime = CTime::GetCurrentTime();
	CString csTime = m_tsScanEndTime.Format(_T("%H:%M:%S"));
	AddLogEntry(_T(">>> End Scan Time: %s"), csTime, 0, true, FIRSTLEVEL);
	
	CString csElapsedTime;
	csElapsedTime.Format(L"%s%s", GetString(L"IDS_STATIC_ELAPSEDTIME"), csTime);
	AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	CString cstypeofscan = L"";
	switch (m_eScanType)
	{
	case FULLSCAN:	cstypeofscan = L"Full";
		break;

	case CUSTOMSCAN:cstypeofscan = L"Custom";
		break;

	default:		cstypeofscan = L"";
		break;
	}

	if (!m_bManualStop)
	{
		csCompleteScanning.Format(L">>> %s %s.", cstypeofscan, GetString(L"IDS_STATUS_INFECTEDFILES"));
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		csFileScanCount.Format(L"%s%d", GetString(L"IDS_STATIC_FILESCANNED"), m_FileScanned);
		csCompleteScanning.Format(L">>> %s = %d, %s = %d", GetString(L"IDS_STATIC_USB_FILESCANNED"), m_FileScanned, GetString(L"IDS_STATIC_USB_THREAD_FOUND"), m_dwVirusFoundCount);
	}
	else
	{
		csCompleteScanning.Format(L">>> %s %s.", cstypeofscan, GetString(L"IDS_STATUS_SCAN_ABORTED"));
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		csFileScanCount.Format(L"%s%d", GetString(L"IDS_STATIC_FILESCANNED"), m_FileScanned);
		csCompleteScanning.Format(L">>> %s = %d, %s = %d", GetString(L"IDS_STATIC_USB_FILESCANNED"), m_FileScanned, GetString(L"IDS_STATIC_USB_THREAD_FOUND"), m_dwVirusFoundCount);
	}
	AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"--------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	csCompleteScanning.Format(L"%s%d", GetString(L"IDS_STATIC_TREATFOUND"), m_dwVirusFoundCount);

	if (m_bManualStop == false)
	{
		HWND hWindow = ::FindWindow(NULL, L"VibraniumMEMSCAN");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
		}
	}
	m_bFullScan = false;
	m_bCustomscan = false;
	theApp.m_bIsScanning = false;
	return true;
}

/***************************************************************************************************
*  Function Name  : callUISetScanFinishedStatus
*  Description    : Calls call_function to invoke ant UI function
*  Author Name    : Nitin Shelar
*  Date			  : 02 Nov 2018
****************************************************************************************************/
void CWardWizMemScanDlg::CallUISetScanFinishedStatus(CString csData)
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CallUISetScanFinishedStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : OnTimerScan
Description    : On timer
Author Name    : Nitin Shelar
SR_NO		   :
Date           : 02 Nov 2018
/***************************************************************************************************/
void CWardWizMemScanDlg::OnTimerScan()
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::OnTimerScan", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	LoadSignatureDatabase
*  Description    :	Function to load Signature database.
*  Author Name    : Nitin Shelar
*  Date           : 26/11/2018
**********************************************************************************************************/
DWORD CWardWizMemScanDlg::LoadSignatureDatabase(DWORD &dwSigCount)
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.LoadSignatureDatabase(dwSigCount);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	UnLoadSignatureDatabase
*  Description    :	Function to Unload Signature database.
*  Author Name    : Nitin Shelar
*  Date           : 26/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizMemScanDlg::UnLoadSignatureDatabase()
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = m_objISpyScanner.UnLoadSignatureDatabase();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x01;
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::UnLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : CallUISetVirusFoundEntryfunction
*  Description    : used to insert threat found entries
*  Author Name    : Nitin Shelar
*  Date			  : 27/11/2018
****************************************************************************************************/
void CWardWizMemScanDlg::CallUISetVirusFoundEntryfunction(CString csVirusName, CString csFilePath, CString csActionTaken, CString SpyID)
{
	try
	{
		m_svAddVirusFoundEntryCB.call(SCITER_STRING(csVirusName), SCITER_STRING(csFilePath), SCITER_STRING(csActionTaken), SCITER_STRING(SpyID));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CallUISetVirusFoundEntryfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : onModalLoop
Description    : for reseting the Lightbox event msgbox
Author Name    : Nitin Shelar
SR_NO		   :
Date           : 27/11/2018
/***************************************************************************************************/
json::value CWardWizMemScanDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;

			theApp.m_objCompleteEvent.SetEvent();
			Sleep(200);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	HandleVirusEntry
*  Description    :	When any entry comes for cleaning, wardwiz scanner take a backup store into quarantine
folder and keep a record into DB file
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizMemScanDlg::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThirdParam, DWORD dwISpyID, CString &csBackupFilePath, DWORD &dwAction)
{
	DWORD dwRet = 0;
	CString csStatus;
	m_csQuarentineEntries.Lock();
	try
	{
		TCHAR szAction[MAX_PATH] = { 0 };
		if ((!szThreatPath) || (!szThreatName) || (!szThirdParam))
		{
			AddLogEntry(L"### CVibraniumMemScanDlg::HandleVirusEntry file name not available", 0, 0, true, SECONDLEVEL);
			dwRet = SANITYCHECKFAILED;
			m_csQuarentineEntries.Unlock();
			return dwRet;
		}

		if (!PathFileExists(szThreatPath))
		{
			AddLogEntry(L"### CVibraniumMemScanDlg::HandleVirusEntry No file available %s", szThreatPath, 0, true, SECONDLEVEL);

			dwRet = FILENOTEXISTS;
			m_csQuarentineEntries.Unlock();
			return dwRet;
		}
		//For files having read-only attribute
		::SetFileAttributes(szThreatPath, FILE_ATTRIBUTE_NORMAL);

		CString csQuaratineFolderPath = m_csSortedDrive;
		if (!PathFileExists(csQuaratineFolderPath))
		{
			if (!CreateDirectory(csQuaratineFolderPath, NULL))
			{
				AddLogEntry(L"### CVibraniumMemScanDlg::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
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
			}
			else
			{
				dwAction = FILEQURENTINED;
				_tcscpy(szAction, GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
			}
		}
		else
		{
			AddLogEntry(L"### Unhandled case to clean, VirusName: [%s], FilePath: [%s]", szThreatName, szThreatPath, true, SECONDLEVEL);
		}
		bool bDoRecover = true;
		//csStatus.Format(L"%s", GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
		//CallUISetVirusFoundEntryfunction(szThreatName, szThreatPath, csStatus, m_csISpyID);
	
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
	}
	catch (...)
	{
		dwRet = 0x10;
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::HandleVirusEntry", 0, 0, true, SECONDLEVEL);
	}

	m_csQuarentineEntries.Unlock();
	return dwRet;
}

/***************************************************************************
Function Name  : CheckForDiscSpaceAvail
Description    : to check whether there is enough space to take a backup
percentage.
Author Name    : Nitin Shelar
SR_NO		   :
Date           : 29/11/2018
****************************************************************************/
DWORD CWardWizMemScanDlg::CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath)
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
			AddLogEntry(L"### CWardwizScanner::Error in opening existing file %s for finding a size of path file", csThreatPath, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CheckForDiscSpaceAvail for file %s", csThreatPath, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
CString CWardWizMemScanDlg::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************
Function Name  : IsDriveHaveRequiredSpace
Description    : to check whether there is enough space to take a backup
percentage.
Author Name    : Nitin Shelar
Date           : 29/11/2018
****************************************************************************/
bool CWardWizMemScanDlg::IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize)
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::IsDriveHaveRequiredSpace", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	BackUpBeforeQuarantineOrRepair
*  Description    :	Taking a backup before taking any action on detected files.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizMemScanDlg::BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath, LPTSTR lpszBackupFilePath)
{
	try
	{
		if (!lpszBackupFilePath)
			return false;
		DWORD dwStatus = 0;
		CString csEncryptFilePath, csQuarantineFolderpath = L"";
		TCHAR szQuarantineFileName[MAX_PATH] = { 0 };
		UINT RetUnique = 0;

		csQuarantineFolderpath = m_csSortedDrive;

		if (!PathFileExists(csOriginalThreatPath))
		{
			AddLogEntry(L"### CVibraniumMemScanDlg::BackUpBeforeQuarantineOrRepair Original file not available %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
			return false;
		}

		if (!PathFileExists(csQuarantineFolderpath))
		{
			if (!CreateDirectory(csQuarantineFolderpath, NULL))
			{
				AddLogEntry(L"### CVibraniumMemScanDlg::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::BackUpBeforeQuarantineOrRepair, FilePath: %s", csOriginalThreatPath, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name		: CheckIFAlreadyBackupTaken
*  Description			: Function to check whether backup already taken (or) not.
*  Function Arguments	: szFileHash
*  Author Name			: Nitin Shelar
*  Date					: 29/11/2018
**********************************************************************************************************/
bool CWardWizMemScanDlg::CheckIFAlreadyBackupTaken(LPCTSTR szFileHash, LPTSTR szBackupPath)
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CheckIFAlreadyBackupTaken, HASH: %s, BackupPath:%s ", szFileHash, szBackupPath, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name		: CheckEntryPresent
*  Description			: Function to check entry present in list
*  Function Arguments	: szFileHash
*  Author Name			: Nitin Shelar
*  Date					: 29/11/2018
**********************************************************************************************************/
bool CWardWizMemScanDlg::CheckEntryPresent(LPCTSTR szFileHash, LPTSTR szBackupPath)
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::CheckIFAlreadyBackupTaken, HASH: %s, BackupPath:%s ", szFileHash, szBackupPath, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	Encrypt_File
*  Description    :	Encrypt file and keep into quarantine folder as temp file.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizMemScanDlg::Encrypt_File(TCHAR *szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus)
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
			AddLogEntry(L"### CVibraniumMemScanDlg::Error in opening existing file %s", szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		if (!dwFileSize)
		{
			AddLogEntry(L"### CVibraniumMemScanDlg::Error in GetFileSize of file %s", szFilePath, 0, true, SECONDLEVEL);
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
			AddLogEntry(L"### CVibraniumMemScanDlg::Error in allocating memory", 0, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}

		memset(lpFileData, 0x00, dwFileSize);
		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
		ReadFile(hFile, lpFileData, dwFileSize, &dwBytesRead, NULL);
		if (dwFileSize != dwBytesRead)
		{
			AddLogEntry(L"### CVibraniumMemScanDlg::Error in ReadFile of file %s", szFilePath, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!CreateRandomKeyFromFile(hFile, dwFileSize))
		{
			AddLogEntry(L"### CVibraniumMemScanDlg : Error in CreateRandomKeyFromFile", 0, 0, true, SECONDLEVEL);
			CloseHandle(hFile);
			dwRet = 0x08;
			goto Cleanup;
		}
		CloseHandle(hFile);

		if (DecryptData((LPBYTE)lpFileData, dwBytesRead))
		{
			AddLogEntry(L"### CVibraniumMemScanDlg::Error in DecryptData", 0, 0, true, SECONDLEVEL);
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
			AddLogEntry(L"### CVibraniumMemScanDlg::Error in creating file %s", lpszTargetFilePath, 0, true, SECONDLEVEL);
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

//Prajakta N.
/**********************************************************************************************************
*  Function Name  :	CreateRandomKeyFromFile
*  Description    :	Create a random key to insert into encrypted file.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizMemScanDlg::CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize)
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
	return true;
}

/**********************************************************************************************************
*  Function Name  :	DecryptData
*  Description    :	Encrypt/Decrypt data.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
DWORD CWardWizMemScanDlg::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::DecryptData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name		: GetFileHash(
*  Description			: Function to get file hash
*  Function Arguments	: pFilePath (In), pFileHash(out)
*  Author Name			: Nitin Shelar
*  Date					: 29/11/2018
**********************************************************************************************************/
bool CWardWizMemScanDlg::GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash)
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
			AddLogEntry(L"### Failed in CVibraniumMemScanDlg::GetFileHash (%s)", L"VBHASH.DLL");
			return true;
		}

		typedef DWORD(*GETFILEHASH)	(TCHAR *pFilePath, TCHAR *pFileHash);
		GETFILEHASH fpGetFileHash = (GETFILEHASH)GetProcAddress(hHashDLL, "GetFileHash");
		if (!fpGetFileHash)
		{
			AddLogEntry(L"### Failed in CVibraniumMemScanDlg::GetFileHash Address(%s)", L"GetFileHash");
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::GetFileHash, FilePath: %s", pFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineEntry
*  Description    :	if ISPYID =0 , wardwiz scanner delete that file
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizMemScanDlg::QuarantineEntry(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath)
{
	AddLogEntry(L">>> Quarantine Entry [%s]: %s", csVirusName, csQurFilePaths, true, SECONDLEVEL);

	bool bReturn = false;
	try
	{
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
			MoveFileEx(csQurFilePaths, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			bReturn = false; //quarantine is failed but we keep the record in the ini.
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::QuarantineFile", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	LoadRecoversDBFile
*  Description    :	Load all entries of recover files.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizMemScanDlg::LoadRecoversDBFile()
{
	try
	{
		m_objISpyDBManipulation.LoadEntries(0x00, m_csSortedDrive);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::LoadRecoversDBFile", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name		: InsertRecoverEntry
*  Description			: Insert entry into recover DB
*  Function Arguments	: szThreatPath, csDuplicateName, szThreatName.
dwShowStatus = 0; Repair/Delete Sucess
dwShowStatus = 1; Delete failed
dwShowStatus = 2; Repair failed
*  Author Name			: Nitin Shelar
*  Date					: 29/11/2018
*  SR_NO				:
**********************************************************************************************************/
bool CWardWizMemScanDlg::InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus)
{
	CIspyList newEntry(szThreatPath, csDuplicateName, szThreatName, L"", L"", L"", dwShowStatus);

	m_objISpyDBManipulation.InsertEntry(newEntry, RECOVER);
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SaveInRecoverDB
*  Description    :	Save all entry into recover files.
*  Author Name    : Nitin Shelar
*  Date           : 29/11/2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizMemScanDlg::SaveInRecoverDB()
{
	OutputDebugString(L">>> In SaveInReportsDB");

	bool bReturn = false;
	try
	{
		bReturn = m_objISpyDBManipulation.SaveEntries(0x00, m_csSortedDrive);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::LoadRecoversDBFile", 0, 0, true, SECONDLEVEL);
	}
	return  bReturn;
}

/***********************************************************************************************
Function Name  : On_ClickQuickScanCleanBtn
Description    : Cleans detected virus entries
SR.NO		   :
Author Name    : Nitin Shelar
Date           : 29/11/2018
***********************************************************************************************/
json::value CWardWizMemScanDlg::On_ClickScanCleanBtn(SCITER_VALUE svArrCleanEntries, SCITER_VALUE svQarantineFlag)
{
	try
	{
		GetSortedDriveList();
		m_svVirusCount = svQarantineFlag;
		OnClickCleanButton(svArrCleanEntries);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_ClickQuickScanCleanBtn", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	OnClickCleanButton
*  Description    :	Clean and Quarantine infected files
*  Author Name    : Nitin Shelar
*  SR_NO		  :
*  Date           : 29/11/2018
**********************************************************************************************************/
bool CWardWizMemScanDlg::OnClickCleanButton(SCITER_VALUE svArrayCleanEntries)
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
		m_svArrayCleanEntries = svArrayCleanEntries;
		m_hQuarantineThread = ::CreateThread(NULL, 0, QuarantineThread, (LPVOID) this, 0, NULL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::OnClickCleanButton", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	QuarantineThread
*  Description    :	If user clicks on clean button.Quarantine thread gets called.
*  Author Name    : Nitin Shelar
*  SR_NO		  :
*  Date           : 29/11/2018
**********************************************************************************************************/
DWORD WINAPI QuarantineThread(LPVOID lpParam)
{
	try
	{
		CWardWizMemScanDlg *pThis = (CWardWizMemScanDlg *)lpParam;
		pThis->QuaratineEntries();
		return 1;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	QuaratineEntries
*  Description    :	Repaires or quarantines selected files one by one.
*  Author Name    : Nitin Shelar
*  SR_NO		  :
*  Date           : 29/11/2018
**********************************************************************************************************/
void CWardWizMemScanDlg::QuaratineEntries()
{
	try
	{
		DWORD dwAction = 0;
		dwAction = FILEQURENTINED;
		CString csBackupPath;
		//Send file to quarentine
		DWORD dwVirusCount = 0x00;
		dwVirusCount = m_svArrayCleanEntries.length();
		for (DWORD dwCurrentVirusEntry = 0; dwCurrentVirusEntry < dwVirusCount; dwCurrentVirusEntry++)
		{
			const SCITER_VALUE svEachEntry = m_svArrayCleanEntries[dwCurrentVirusEntry];
			const std::wstring chThreatName = svEachEntry[L"ThreatName"].get(L"");
			const std::wstring chFilePath = svEachEntry[L"FilePath"].get(L"");
			bool bSelected = svEachEntry[L"selected"].get(true);
			if (bSelected)
			{
				DWORD dwCleanResult = HandleVirusEntry(chFilePath.c_str(), chThreatName.c_str(), L"Quick Scan", m_wISpywareID, csBackupPath, dwAction);
				if (dwCleanResult != 0x00)
				{
					AddLogEntry(L"### Failed to HandleVirusEntry, FilePath: %s, Threat Name: %s", chFilePath.c_str(), chThreatName.c_str(), true, SECONDLEVEL);
					m_csScanFile.Unlock();
				}
			}
		}
		m_svVirusCount.call();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetDriveSize
*  Description    :	To select drive which contain max size. 
*  Author Name    : NITIN SHELAR
*  Date           : 03/01/2019
**********************************************************************************************************/
bool CWardWizMemScanDlg::GetDriveSize(CString  csDriveName)
{
	try
	{
		BOOL fResult;
		DWORD64 dwTotalNumberOfBytes;
		ULARGE_INTEGER ulFreeBytesAvailable, ulTotalNumBytes, ulTotalNumFreeBytes;

		fResult = GetDiskFreeSpaceEx(csDriveName, &ulFreeBytesAvailable, &ulTotalNumBytes, &ulTotalNumFreeBytes);
		
		if (fResult)
		{
			dwTotalNumberOfBytes = ulTotalNumBytes.QuadPart;
			dwTotalNumberOfBytes = dwTotalNumberOfBytes / (1024 * 1024);
			
			if (m_dw64MaxSize == 0x00)
			{
				m_dw64MaxSize = dwTotalNumberOfBytes;
				m_csSortedDrive = csDriveName;
			}

			if (m_dw64MaxSize < dwTotalNumberOfBytes)
			{
				m_dw64MaxSize = dwTotalNumberOfBytes;
				m_csSortedDrive = csDriveName;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetSortedDriveList
*  Description    :	Get drive path which size is maximum as compare to other drive size.
*  Author Name    : NITIN SHELAR
*  Date           : 03/01/2019
**********************************************************************************************************/
bool CWardWizMemScanDlg::GetSortedDriveList()
{
	CString csDrive;
	try
	{
		CStringArray csaReturn;
		int iIntex = 0;
		TCHAR szStr[255];

		if (!GetAllDrivesList(csaReturn))
		{
			return false;
		}

		for (iIntex = 0; iIntex < csaReturn.GetSize(); iIntex++)
		{
			if (iIntex == 0)
				m_dw64MaxSize = 0x00;
			wsprintf(szStr, L"  %s", csaReturn);
			csDrive = csaReturn.GetAt(iIntex);
			if (!GetDriveSize(csDrive))
			{
				return false;
			}
		}

		if (PathFileExists(m_csSortedDrive))
		{
			m_csSortedDrive += L"\\WardWiz";
			
			if (!PathFileExists(m_csSortedDrive))
				CreateDirectory(m_csSortedDrive, NULL);

			m_csSortedDrive += L"\\QUARANTINE";

			if (!PathFileExists(m_csSortedDrive))
			{
				if (!CreateDirectory(m_csSortedDrive, NULL))
				{
					AddLogEntry(L"### CVibraniumMemScanDlg::Create Qurantine directory failed", 0, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::QuarantineThread", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	BrowseForFolder
*  Description    :	To browse folder for custom scan.
*  Author Name    : NITIN SHELAR
*  Date           : 03/01/2019
**********************************************************************************************************/
CString CWardWizMemScanDlg::BrowseForFolder()
{
	CString				csFolderPath;
	BROWSEINFO   		bi;
	LPITEMIDLIST 		pidl;
	TCHAR  				szPathName[MAX_PATH];
	
	try
	{
		ZeroMemory(&bi, sizeof(bi));
		bi.lpszTitle = TEXT("Please select a folder for storing received files :");
		bi.ulFlags = BIF_RETURNONLYFSDIRS;
		pidl = SHBrowseForFolder(&bi);

		if (NULL != pidl)
		{
			BOOL bRet = SHGetPathFromIDList(pidl, szPathName);
			if (FALSE == bRet)
			{
				return L"";
			}
			csFolderPath = szPathName;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::BrowseForFolder", 0, 0, true, SECONDLEVEL);
	}
	return csFolderPath;
}

/***********************************************************************************************
Function Name  : on_GetFolderPath
Description    : Get folder path for scanning from UI.
Author Name    : NITIN SHELAR
Date           : 04/01/2019
***********************************************************************************************/
json::value CWardWizMemScanDlg::On_GetFolderPath()
{
	CString csFolderPAth;
	try
	{
		csFolderPAth = BrowseForFolder();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::on_GetFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return (SCITER_VALUE)csFolderPAth.GetBuffer();
}

/***************************************************************************************************
Function Name  : PreTranslateMessage
Description    : Ignore Enter/escape button click events
Author Name    : NITIN SHELAR
Date           : 17/01/2019
/***************************************************************************************************/
BOOL CWardWizMemScanDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE || pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_TAB || pMsg->wParam == VK_F4))
	{
		return TRUE;
	}
	if (pMsg->message == WM_CHAR || pMsg->message == WM_UNICHAR)
	{
		WindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
	}
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : GetString
Description    : Function which returns the string from selected language ini
file.
Author Name    : NITIN SHELAR
Date           : 17/01/2019
****************************************************************************/
CString CWardWizMemScanDlg::GetString(CString csStringID)
{
	TCHAR szValue[2000] = { 0 };
	DWORD dwLangID = 0x00;
	CString csFilePath;
	try
	{
		dwLangID = GetSelectedLanguage();
		csFilePath = GetModuleFileStringPath() + L"\\VBSETTINGS";
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::GetString", 0, 0, true, SECONDLEVEL);
	}
	return szValue;
}

/***************************************************************************
Function Name  : GetSelectedLanguage
Description    : Function returns DWORD value
0 - ENGLISH
1 - HINDI
2 - GERMAN
3 - CHINESE
4 - SPANISH
5 - FRENCH
Author Name    : NITIN SHELAR
Date           : 17/01/2019
****************************************************************************/
DWORD CWardWizMemScanDlg::GetSelectedLanguage()
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
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::GetSelectedLanguage", 0, 0, true, SECONDLEVEL);
	}
	return GetPrivateProfileInt(L"VBSETTINGS", L"LanguageID", 0, csIniFilePath);
}

/***************************************************************************
Function Name  : FunCleanup
Description    : Function for clean allocated resoureces.
Author Name    : NITIN SHELAR
Date           : 25/03/2019
****************************************************************************/
void CWardWizMemScanDlg::FunCleanup()
{
	try
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

		m_dwTotalFileCount = 0;
		m_FileScanned = 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::FunCleanup", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_CloseThreads()
*  Description    : Accepts the request from UI and force to close all threads.
*  Author Name    : Nitin Shelar
*  Date			  : 1 Nov 2018
****************************************************************************************************/
json::value CWardWizMemScanDlg::On_CloseThreads()
{
	try
	{
		m_bManualStop = true;

		if (!m_bFullScan || !m_bCustomscan)
		{
			if (!ResumeScan())
			{
				m_bCustomscan = false;
				m_bFullScan = false;
			}
		}

		DWORD dwWaitTime = 10 * 1000;

		if (m_hThread_ScanCount != NULL)
		{
			if (WaitForSingleObject(m_hThread_ScanCount, dwWaitTime) == WAIT_TIMEOUT)
				TerminateThread(m_hThread_ScanCount,0);
		}
		
		if (m_hThread_ScanCount != NULL)
		{
			if (WaitForSingleObject(m_hWardWizAVThread, dwWaitTime) == WAIT_TIMEOUT)
				TerminateThread(m_hWardWizAVThread, 0);
		}

		CWnd *pwnd = theApp.m_pMainWnd;
		if (pwnd != NULL)
		{
			::KillTimer(pwnd->m_hWnd, TIMER_SCAN_STATUS);
		}

		FunCleanup();
		ScanFinished();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_CloseThreads()", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}
