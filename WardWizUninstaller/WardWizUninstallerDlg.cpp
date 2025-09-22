/**********************************************************************************************************
Program Name          : WardWizUninstallerDlg.cpp
	Description           :
	Author Name			  : Ramkrushna Shelke
	Date Of Creation      : 6th Feb 2015
	Version No            : 1.9.0.0
	Special Logic Used    :
	Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizUninstaller.h"
#include "WardWizUninstallerDlg.h"
#include "afxdialogex.h"
#include "iSpySrvMgmt.h"
#include "WardWizDatabaseInterface.h"
#include <Wtsapi32.h>
#include "iTinRegWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
protected:
	DECLARE_MESSAGE_MAP()
};

/***************************************************************************
Function Name  : CAboutDlg
Description    : C'tor
Author Name    : Ram Shelke
SR_NO		   :
Date           : 20 May 2016
****************************************************************************/
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Ram Shelke
SR_NO		   :
Date           : 20 May 2016
****************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

#define TIMER_DELETE_STATUS			100

DWORD WINAPI DeleteFileThread(LPVOID lpvThreadParam);
DWORD WINAPI UninstallThread(LPVOID lpvThreadParam);

/***************************************************************************
Function Name  : CWardWizUninstallerDlg
Description    : C'tor
Author Name    : Nitin K
SR_NO			 :
Date           : 06th Feb 2015
****************************************************************************/
CWardWizUninstallerDlg::CWardWizUninstallerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWardWizUninstallerDlg::IDD, pParent)
	, m_iTotalFileCount(0)
	, m_vFilePathLists(0)
	, m_vFolderPathLists(0)
	, m_bMoveFileExFlag(FALSE)
	, m_bQuarantine(false)
	, m_bPassDbMngr(false)
	, m_bUserSettings(false)
	, m_iTotalDeletedFileCount(0)
	, m_bUninstallInProgress(FALSE)
	, m_bUninstallCmd(false)
	, m_lpEmailNFUnlink(NULL)
	, m_hModuleEmailDLL(NULL)
	, m_bIsWow64(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_csArrRegistryEntriesDel.RemoveAll();
	m_csArrRegistryEntriesDel.Add(L"AppFolder");
	m_csArrRegistryEntriesDel.Add(L"AppVersion");
	m_csArrRegistryEntriesDel.Add(L"DataBaseVersion");
	m_csArrRegistryEntriesDel.Add(L"dwScanLevel");
	m_csArrRegistryEntriesDel.Add(L"dwProductID");
	LoadRequiredLibrary();
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    :
SR_NO			 :
Date           : 06th Feb 2015
****************************************************************************/
void CWardWizUninstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    :
*  SR_NO
*  Date           : 06th Feb 2015
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizUninstallerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

HWINDOW   CWardWizUninstallerDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizUninstallerDlg::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box
					procedure common to all Microsoft Foundation Class Library dialog boxes
*  Author Name    :
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
BOOL CWardWizUninstallerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.
	
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	bShowLightBox = false;
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

	bool bUninstallCmd = false;
	CString csScanOption = L"";
	CString csCmdLineText;
	CString csCommandLine = GetCommandLine();
	if (csCommandLine.Find('-') != -1)
	{
		csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
		csCommandLine.Trim();
		if (csCommandLine.CompareNoCase(TEXT("EPSNOUI")) >= 0)
		{
			csCommandLine.Replace(TEXT("EPSNOUI"), L"");
			csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
			csCommandLine.MakeLower();
			csCommandLine.Trim();
		}
		if (csCommandLine.GetLength() > 0)
		{
			if (_tcscmp(csCommandLine, L"uninstall") >= 0)
			{
				m_bUninstallCmd = bUninstallCmd = true;
				csCommandLine.Replace(TEXT("uninstall"), L"");
				csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
				csCommandLine.Trim();
			}
			if (_tcscmp(csCommandLine, L"0") >= 0)
			{
				m_bIsUserDefinedSettings = false;
				csCommandLine.Replace(TEXT("0"), L"");
			}
			else if (_tcscmp(csCommandLine, L"1") >= 0)
			{
				m_bIsUserDefinedSettings = true;
				csCommandLine.Replace(TEXT("1"), L"");
			}
			csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
			csCommandLine.Trim();
			if (_tcscmp(csCommandLine, L"0") == 0)
			{
				m_bIsKeepQuarFiles = false;
			}
			else if (_tcscmp(csCommandLine, L"1") == 0)
			{
				m_bIsKeepQuarFiles = true;
			}
		}
	}

		//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_UNINSTALL.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_UNINSTALL.htm");
	m_root_el = root();
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_RootElt = root();
	SciterGetElementIntrinsicWidths(m_RootElt, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_RootElt, pIntMinWidth, &pIntHeight);
	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	WTSRegisterSessionNotification(this->get_hwnd(), NOTIFY_FOR_ALL_SESSIONS);
	//Issue resovled: 0001790
	this->SetWindowText(L"VibraniumUNINST");
	CreateUninstallerEntryinRegistry();
	
	m_csWardWizPath = GetWardWizPathFromRegistry();

	//WardWiz Installation path should not be blank else it will remove all files from the system.
	if (m_csWardWizPath.Trim().GetLength() == 0x00)
	{
		if (bUninstallCmd)
		{
			exit(0);
			return FALSE;
		}

		MessageBox(L"Failed to get Vibraniumpath from registry.", L"Vibranium", MB_ICONERROR);
		exit(0);
		return FALSE;
	}

	
	if (bUninstallCmd)
	{
		::MoveWindow(this->get_hwnd(), 0, 0, 0, 0, true);
		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
		UninstallforEPS();
	}

	bBackSpace = false;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************
Function Name  : UninstallforEPS
Description    : Function to uninstall WardWiz through Commandline
Author Name    : Jeena Mariam Saji
SR_NO		   :
Date           : 20 Feb 2018
****************************************************************************/
void CWardWizUninstallerDlg::UninstallforEPS()
{
	const SCITER_VALUE result = m_root_el.call_function("CallShowUninstall", m_bIsUserDefinedSettings, m_bIsKeepQuarFiles);
}

void CWardWizUninstallerDlg::CreateUninstallerEntryinRegistry()
{
	LONG		lResult = 0x00;
	HKEY hKey = NULL;
	CString csSubkey = NULL;
	CString strKey = NULL;
	DWORD dwType = REG_DWORD;
	DWORD returnDWORDValue;
	DWORD dwSize = sizeof(returnDWORDValue);
	DWORD dwData = 512;

	csSubkey = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Persisted";
	strKey.Format(L"%s\\VibraniumUNINST.exe", theApp.GetModuleFilePath());

	lResult = ::RegOpenKeyEx(HKEY_CURRENT_USER, csSubkey, 0, KEY_ALL_ACCESS, &hKey);
	if (lResult == ERROR_SUCCESS)
	{
		lResult = ::RegQueryValueEx(hKey, strKey, NULL, &dwType, (LPBYTE)&returnDWORDValue, &dwSize);
		if (lResult != ERROR_SUCCESS)
		{
			lResult = RegSetValueEx(hKey, strKey, 0, REG_DWORD, (LPBYTE)&dwData, dwSize);
			if (lResult != ERROR_SUCCESS)
			{
				AddLogEntry(L"### error in creating value %s\\%s", csSubkey, strKey, true, SECONDLEVEL);
			}
		}
	}
	else
	{
		csSubkey = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Store";
		lResult = ::RegOpenKeyEx(HKEY_CURRENT_USER, csSubkey, 0, KEY_ALL_ACCESS, &hKey);
		if (lResult == ERROR_SUCCESS)
		{
			lResult = ::RegQueryValueEx(hKey, strKey, NULL, &dwType, (LPBYTE)&returnDWORDValue, &dwSize);
			if (lResult != ERROR_SUCCESS)
			{
				lResult = RegSetValueEx(hKey, strKey, 0, REG_DWORD, (LPBYTE)&dwData, dwSize);
				if (lResult != ERROR_SUCCESS)
				{
					AddLogEntry(L"### error in creating value %s\\%s", csSubkey,strKey, true, SECONDLEVEL);
				}
			}
		}
	}

}
/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : The framework calls this member function when the user selects a command from the 
					Control menu, or when the user selects the Maximize or the Minimize button.
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWardWizUninstallerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    : Paints the UI
					If you add a minimize button to your dialog, you will need the code below
					to draw the icon.  For MFC applications using the document/view model,
					this is automatically done for you by the framework.
*  Author Name    :
*  SR_NO
*  Date           : 6th Feb 2015
****************************************************************************************************/
void CWardWizUninstallerDlg::OnPaint()
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

/***************************************************************************
	Function Name  : OnQueryDragIcon
	Description    : The framework calls this member function by a minimized
					(iconic) window that does not have an icon defined for its class
					The system calls this function to obtain the cursor to display while the user drags
					the minimized window.
	Author Name    :
	SR_NO			 :
	Date           : 06 FEB 2015
****************************************************************************/
HCURSOR CWardWizUninstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************
Function Name  : WindowProc
Description    : The framework calls this member function by a minimized
Author Name    : Ram Shelke
SR_NO		   :
Date           : 06 FEB 2015
****************************************************************************/
LRESULT CWardWizUninstallerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0L;
	BOOL    bHandled = FALSE;
	HWND hWindow = NULL;
	switch (LOWORD(wParam))
	{
	case WTS_REMOTE_CONNECT:
	case WTS_REMOTE_DISCONNECT:
	case WTS_SESSION_LOGON:
	case WTS_SESSION_LOGOFF:
	case WTS_SESSION_LOCK:
	case WTS_SESSION_UNLOCK:
	case WTS_SESSION_REMOTE_CONTROL:
		hWindow = ::FindWindow(NULL, L"VibraniumUNINST");
		if (hWindow)
		{
			::InvalidateRect(hWindow, NULL, TRUE);
			bHandled = TRUE;
		}
		break;
	}

	
	if (!bHandled || !bBackSpace)      // if it was handled by the Sciter
	{
		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
	}

	bBackSpace = false;

	if (bHandled)      // if it was handled by the Sciter
		return lResult; // then no further processing is required.

	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************
Function Name  : On_ClickRestartNow
Description    : button event handler to restart machine
Author Name    : Ram Shelke
SR_NO		   : 
Date           : 18 MAY 2016
****************************************************************************/
json::value CWardWizUninstallerDlg::On_ClickRestartNow()
{
	CEnumProcess enumproc;
	enumproc.RebootSystem(0);
	return 0;
}


/***************************************************************************
Function Name  : On_ButtonClickMinimize
Description    : button event handler to minimize window
Author Name    : Ram Shelke
SR_NO		   : 
Date           : 18 MAY 2016
****************************************************************************/
json::value CWardWizUninstallerDlg::On_ButtonClickMinimize()
{
	ShowWindow(SW_MINIMIZE);
	return 0;
}

/***************************************************************************
Function Name  : On_ClickStartUninstall
Description    : Event handler for button click of next
Author Name    : Ram Shelke
SR_NO		   :
Date           : 06 FEB 2015
****************************************************************************/
json::value CWardWizUninstallerDlg::On_ClickStartUninstall(SCITER_VALUE svArrSelectedOptions, SCITER_VALUE svFunSetPercentageCB, SCITER_VALUE svFuntionFinished, SCITER_VALUE svEnableDisableButtons, SCITER_VALUE svShowWrdWizRunningCB)
{
	svArrSelectedOptions.isolate();
	if (!svArrSelectedOptions.is_array())
	{
		return 0;
	}
	m_svFunSetPercentageCB = svFunSetPercentageCB;
	m_svFuntionFinished = svFuntionFinished;
	m_svEnableDisableButtons = svEnableDisableButtons;
	m_svShowWrdWizRunningCB = svShowWrdWizRunningCB;
	const SCITER_VALUE EachEntry = svArrSelectedOptions[0];
	m_bUserSettings = EachEntry[L"UserSettings"].get(false);
	m_bQuarantine = EachEntry[L"QuarantineEntries"].get(false);

	//WardWiz Installation path should not be blank else it will remove all files from the system.
	if (m_csWardWizPath.Trim().GetLength() == 0x00)
	{
		AddLogEntry(L"### Failed to get Vibraniumpath from registry.", 0, 0, true, SECONDLEVEL);
		exit(0);
		return 0;
	}

	m_hThreadDeleteFile = ::CreateThread(NULL, 0, DeleteFileThread, (LPVOID) this, 0, NULL);
	return 0;
}

/***********************************************************************************************
*  Function Name  : InitializeVariables
*  Description    : Function to initialize variables
*  Author Name    : Ram Shelke 
*  SR_NO		  : 
*  Date           : 17 MAY 2016
*************************************************************************************************/
void CWardWizUninstallerDlg::InitializeVariables()
{
	m_dwTotalDeletedFileCount = 0;
	m_dwTotalFileCount = 0;
}

/***********************************************************************************************
*  Function Name  : DeleteFileThread
*  Description    : After clicking Next button. DeleteFileThread is created it stops the driver 
					protection and deletes the registry, setup files
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 06 FEB 2015
*************************************************************************************************/
DWORD WINAPI DeleteFileThread(LPVOID lpvThreadParam)
{
	CWardWizUninstallerDlg *pThis = (CWardWizUninstallerDlg*)lpvThreadParam;
	if (!pThis)
	{
		return 0x00;
	}

	pThis->InitializeVariables();
	pThis->EnumFolderToCountFiles(pThis->m_csWardWizPath);
	pThis->StartUninstallProgram();
	
	return 0x00;
}

/***********************************************************************************************
*  Function Name  : EnumFolderToCountFiles
*  Description    : Enumrate all the files in folder to check whether folder is empty or not.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 03 Feb 2015
*************************************************************************************************/
void CWardWizUninstallerDlg::EnumFolderToCountFiles(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString csFilePath = finder.GetFilePath();
				SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
				CString csFileName = finder.GetFileName();
				if (csFileName == "QUARANTINE")
				{
					if (m_bQuarantine)
					{
						continue;
					}
					else
					{
						m_vFolderPathLists.push_back(csFilePath);
						m_bMoveFileExFlag = FALSE;
						EnumFolderToCountFiles(csFilePath);//, theApp.m_bQuarantine, theApp.m_bPassDbMngr);
					}
				}
				else if (csFileName == "VBSETTINGS")
				{
					SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
					MoveFileEx(csFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					m_bMoveFileExFlag = TRUE;
					EnumFolderToCountFiles(csFilePath);
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					CString szMessage = NULL;
					szMessage.Format(L"### CWWizUninstSecondDlg::Folder to be deleted on Restart : %s", csFilePath);
					AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
					m_bMoveFileExFlag = FALSE;
					continue;
				}
				else
				{
					m_vFolderPathLists.push_back(csFilePath);
					m_bMoveFileExFlag = FALSE;
					EnumFolderToCountFiles(csFilePath);//, theApp.m_bQuarantine, theApp.m_bPassDbMngr);
				}
			}
			else
			{
				CString csFileName = finder.GetFileName();
				CString csFilePath = finder.GetFilePath();
				SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
				if (csFileName == "VBEVALREG.DLL")
				{
					if (m_bPassDbMngr)
					{
						continue;
					}
				}
				if (csFileName == "VBRESOURCE.DLL")
				{
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					CString szMessage = NULL;
					szMessage.Format(L"### CWWizUninstSecondDlg::Folder to be deleted on Restart : %s", csFilePath);
					AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
					continue;
				}
				if (csFileName == "VBSHELLEXT.DLL")
				{
					if (PathFileExists(csFilePath))
					{
						UnregisterComDll(csFilePath);
					}
				}
				if (m_bMoveFileExFlag)
				{
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					CString szMessage = NULL;
					szMessage.Format(L"### CWWizUninstSecondDlg::Folder to be deleted on Restart : %s", csFilePath);
					AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
					continue;
				}
				m_vFilePathLists.push_back(csFilePath);
				m_iTotalFileCount++;
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::EnumFolderToCountFiles", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : UnregisterComDll
Description    : Unregister COM Dll
Author Name    : Neha Gharge
S.R. No        :
Date           : 3/28/2015
****************************************************************************/
void CWardWizUninstallerDlg::UnregisterComDll(CString csAppPath)
{
	try
	{
		CWardWizOSversion		objOSVersionWrap;
		CString csExePath, csCommandLine;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		csExePath.Format(L"%s\\%s", systemDirPath, L"regsvr32.exe");

		//On xp runas parameter never work It will not unregister the VBSHELLEXT.DLL
		//So NUll parameter send.
		DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
		//Neha Gharge Message box showing of register successful on reinstallation.
		switch (OSType)
		{
		case WINOS_XP:
		case WINOS_XP64:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, NULL, csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		default:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		}

		if (PathFileExists(csAppPath))
		{
			if (!DeleteFile(csAppPath))
			{
				AddLogEntry(L"### DeleteFile Failed for file: %s .", csAppPath, 0, true, SECONDLEVEL);
				if (!MoveFileEx(csAppPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
				{
					AddLogEntry(L"### MoveFileEx Failed for file: %s.", csAppPath, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UnRegisterComDLL", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : StartUninstallProgram
*  Description    : Starts the uninstallation process by creating UninstallThread
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWardWizUninstallerDlg::StartUninstallProgram()
{
	m_bUninstallInProgress = TRUE;
	m_hThreadUninstall = ::CreateThread(NULL, 0, UninstallThread, (LPVOID) this, 0, NULL);
	Sleep(500);
	SetTimer(TIMER_DELETE_STATUS, 10, NULL);
}

/***********************************************************************************************
*  Function Name  : UninstallThread
*  Description    : This thread stops the Driver protection and deletes the files
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 06 Feb 2015
*************************************************************************************************/
DWORD WINAPI UninstallThread(LPVOID lpvThreadParam)
{
	CWardWizUninstallerDlg *pThis = (CWardWizUninstallerDlg*)lpvThreadParam;
	if (!pThis)
	{
		return 0x00;
	}

	pThis->EnableDisableControls(false);
	pThis->RestoreReqRegistry();

	// Lalits function calls
	pThis->StopProtectionDrivers();

	if (!pThis->CloseAllAplication(true))
	{
		pThis->CloseApp();
	}
	else
	{
		//Nitin's function calls
		pThis->m_iTotalDeletedFileCount = 0;
		pThis->m_bMoveFileExFlag = FALSE;
		if (theApp.m_dwProductID != ESSENTIAL && theApp.m_dwProductID != BASIC)
		{
			pThis->GetNFDllUnlink();
			pThis->UninstallFWDrivers();
			pThis->GetFWDriverPath();
		}
		Sleep(500);
		pThis->StartUninstllSqlServerAndDisableIISServices();
		pThis->DeleteFiles(pThis->m_vFilePathLists);
		pThis->DeleteFolder(pThis->m_vFolderPathLists);
		pThis->DeleteRegistryKeys();
		pThis->DeleteAllAppShortcut();
		pThis->DeleteFinished();
		pThis->RemoveDriverRegistryKeyAndPauseDrivers();
		pThis->UninstallationMessage();
		if (pThis->m_bUninstallCmd)
		{
			exit(0);
			return 0x00;
		}
		pThis->ShowFinishedPage();
		pThis->RestoreWindow();
	}
	
	pThis->EnableDisableControls(true);

	return 0x00;
}

/***************************************************************************
Function Name  : OnTimer
Description    : The framework calls this member function after each interval 
				 specified in the SetTimer member function used to install a timer.
Author Name    : Ram Shelke
SR_NO		   :
Date           : 06 FEB 2015
****************************************************************************/
void CWardWizUninstallerDlg::OnTimer(UINT_PTR nIDEvent)
{
	TCHAR	szTemp[256] = { 0 };
	
	if (nIDEvent == TIMER_DELETE_STATUS)
	{
		int		iPercentage = int(((float)(m_iTotalDeletedFileCount) / m_iTotalFileCount) * 100);
		CString csPercent;
		csPercent.Format(L"%d", iPercentage);
		m_svFunSetPercentageCB.call(SCITER_STRING(csPercent));
	}
	__super::OnTimer(nIDEvent);
}

/***************************************************************************
Function Name  : OnTimer
Description    : Function which get called when uninstallation get finished.
Author Name    : Ram Shelke
SR_NO		   :
Date           : 06 FEB 2015
****************************************************************************/
void CWardWizUninstallerDlg::ShowFinishedPage()
{
	m_svFuntionFinished.call();
}

/***************************************************************************
Function Name  : RestoreWindow
Description    : Function to restore window and bring to Desktop.
Author Name    : Ram Shelke
SR_NO		   :
Date           : 16 AUG 2016
****************************************************************************/
void CWardWizUninstallerDlg::RestoreWindow()
{
	ShowWindow(SW_RESTORE);
	BringWindowToTop();
	m_bUninstallInProgress = FALSE;
}

/***************************************************************************************************
*  Function Name  : StopProtectionDrivers
*  Description    : stops the driver protection
*  Author Name    : Lalit K.
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWardWizUninstallerDlg::StopProtectionDrivers()
{
	// TODO: Add your control notification handler code here
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int OsType = 0;
	bool returnStatus = false;

	try
	{

		OsType = objGetOSVersion.DetectClientOSVersion();
		if (OsType == 5)
		{
			cSecure64Obj.RegisterProcessId(WLSRV_ID_NINE); // self protection
			scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection
			//StopAndUninstallProcessProtection(OsType);
			//StopAndUninstallScanner();
		}
		else
			if (OsType == 10)
			{
				//StopAndUninstallScanner();
				scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection

			}
			else
			{
				cSecure64Obj.RegisterProcessId(WLSRV_ID_NINE); // self protection
				scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection
				//StopAndUninstallProcessProtection(OsType);
				//StopAndUninstallScanner();
			}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RemoveDriverService", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : CloseAllAplication
Description    : Function which closes Runing Application.
Author Name    :
S.R. No        : WRDWIZSETUPDLL_0003
Date           :
****************************************************************************/
bool CWardWizUninstallerDlg::CloseAllAplication(bool bIsReInstall)
{
	AddLogEntry(L">>> CWardwizUninstSecondDlg : CloseAllAplication", 0, 0, true, ZEROLEVEL);
	m_csAppPath = theApp.GetModuleFilePath();
	bool bISFullPath = false;
	if (_tcslen(m_csAppPath) > 0)
	{
		bISFullPath = true;
	}

	CString csModulePath = m_csAppPath;

	if (bIsReInstall)
	{
		if (!GetCloseAction4OutlookIfRunning(true))
		{
			return false;
		}
	}

	//CString csAppPath = csModulePath + L"WRDWIZAVUI.EXE";
	CEnumProcess objEnumProcess;
	if (objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", false, false, false))
	{
		//int iRet = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		SCITER_VALUE svReturn = 0; //= m_svShowWrdWizRunningCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		if (theApp.m_bRetval == true)
		{
			svReturn = 1;
		}
		/*if (iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", true, false, false);

		}
		else
		{
			return false;
		}*/
		if (svReturn == 1)
		{
			objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", true, false, false);
		}
		else
		{
			return false;
		}
	}
	if (objEnumProcess.IsProcessRunning(L"VBUI.EXE", false, false, false))
	{
		//int iRet = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		SCITER_VALUE svReturn = 0;//= m_svShowWrdWizRunningCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		if (theApp.m_bRetval == true)
		{
			svReturn = 1;
		}
		/*if (iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"VBUI.EXE", true, false, false);

		}
		else
		{
			return false;
		}*/
		if (svReturn == 1)
		{
			objEnumProcess.IsProcessRunning(L"VBUI.EXE", true, false, false);

		}
		else
		{
			return false;
		}
	}
	//Issue Resolved : When we insert usb and uninstall the setup still usb scan is in process.
	//Resolved By : Nitin K. Date: 11th March 2015
	if (objEnumProcess.IsProcessRunning(L"VBUSBDETECTUI.EXE", false, false, false))
	{
		//int iRet = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		SCITER_VALUE svReturn = 0; // = m_svShowWrdWizRunningCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		if (theApp.m_bRetval == true)
		{
			svReturn = 1;
		}
		/*if (iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"VBUSBDETECTUI.EXE", true, false, false);
		}
		else
		{
			return false;
		}*/
		if (svReturn == 1)
		{
			objEnumProcess.IsProcessRunning(L"VBUSBDETECTUI.EXE", true, false, false);
		}
		else
		{
			return false;
		}
	}
	CStringArray objcsaWardWizProcesses;
	objcsaWardWizProcesses.Add(L"VBSCANNER.EXE");
	objcsaWardWizProcesses.Add(L"VBTRAY.EXE");

	for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
	{
		CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);
		if (objEnumProcess.IsProcessRunning(csProcessName, true, false, false))
		{
			AddLogEntry(L">>> %s was running, Terminated", csProcessName, 0, true, SECONDLEVEL);
		}

	}

	//close and remove service here
	CISpySrvMgmt		iSpySrvMgmtObj;
	if (iSpySrvMgmtObj.StopServiceManually(WARDWIZSERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Service WardWizComsrv", 0, 0, true, SECONDLEVEL);
	}


	if (iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv", 0, 0, true, SECONDLEVEL);

	}

	//close and remove service here
	if (iSpySrvMgmtObj.StopServiceManually(WARDWIZUPDATESERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Update Service WardWizComsrv", 0, 0, true, SECONDLEVEL);
	}

	if (iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Update Service WardWizComsrv", 0, 0, true, SECONDLEVEL);

	}

	if (theApp.m_dwProductID == ELITE)
	{
		if (iSpySrvMgmtObj.UnInstallService(WARDWIZEPSCLIENTSERVNAME) != 0x00)
		{
			AddLogEntry(L"### Unable to UnInstall WardWizEPSCLIENTSERVNAME Service", 0, 0, true, SECONDLEVEL);

		}
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : CloseApp
*  Description    : for closing application
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWardWizUninstallerDlg::CloseApp()
{
	OnCancel();
}

/***************************************************************************************************
*  Function Name  : DeleteFiles
*  Description    : Deletes all the application files one by one
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
bool CWardWizUninstallerDlg::DeleteFiles(std::vector<CString>	&vFilePathLists)
{
	try
	{
		AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteFiles", 0, 0, true, ZEROLEVEL);
		CString csFileName = NULL;
		int size = static_cast<int>(vFilePathLists.size());
		while (!vFilePathLists.empty())
		{
			csFileName = vFilePathLists.back();
			if (PathFileExistsW(csFileName))
			{
				SetFileAttributes(csFileName, FILE_ATTRIBUTE_NORMAL);
				if (DeleteFile(csFileName))
				{

					m_iTotalDeletedFileCount++;

					//Issue resolved: 0001864, Check if there are more files no need to wait.
					if (size < 0xC8)
					{
						Sleep(500);
					}
				}
				else
				{
					DWORD err = GetLastError();
					m_iTotalDeletedFileCount++;
					m_bRestartReq = true;
					if (MoveFileEx(csFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
					{
						CString szMessage = NULL;
						szMessage.Format(L"### CVibraniumUninstSecondDlg::File to be deleted on Restart : %s", csFileName);
						AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
						DWORD err1 = GetLastError();
					}
				}
			}
			vFilePathLists.pop_back();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CuninstallDlg::DeleteFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DeleteFolder
*  Description    : Deletes all the application folder one by one
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
bool CWardWizUninstallerDlg::DeleteFolder(std::vector<CString>	&vFolderPathLists)
{
	try
	{
		AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteFolder", 0, 0, true, ZEROLEVEL);
		CString csFolderName = NULL;
		int size = static_cast<int>(vFolderPathLists.size());
		while (!vFolderPathLists.empty())
		{
			csFolderName = vFolderPathLists.back();
			if (PathFileExistsW(csFolderName))
			{
				if (RemoveDirectory(csFolderName) == 0x00)
				{
					MoveFileEx(csFolderName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
			else
			{
				MoveFileEx(csFolderName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				CString szMessage = NULL;
				szMessage.Format(L"### CWWizUninstSecondDlg::Folder to be deleted on Restart : %s", csFolderName);
				AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
			}
			vFolderPathLists.pop_back();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CuninstallDlg::DeleteFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : DeleteRegistryKeys
*  Description    : Enumrate all the files in folder to check whether folder is empty or not.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 03 Feb 2015
*************************************************************************************************/
bool CWardWizUninstallerDlg::DeleteRegistryKeys()
{
	LONG		lResult = 0x00;
	HKEY hKey = NULL;
	DWORD dwRegCount = 0x00;
	CString szSubkey = NULL;
	CITinRegWrapper g_objReg;
	HKEY	hRootKey = NULL;
	hRootKey = HKEY_LOCAL_MACHINE;
	TCHAR	szSubKey[512] = { 0 };
	TCHAR	szValueName[512] = { 0 };
	_tcscpy(szSubKey, L"SOFTWARE\\Microsoft\\Windows");
	_tcscpy(szValueName, L"VibraniumUserInfo");

	AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteRegistryKeys", 0, 0, true, ZEROLEVEL);
	if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csProdKeyName, &hKey) != ERROR_SUCCESS)
	{
		AddLogEntry(L"### Unable to open registry key", 0, 0, true, SECONDLEVEL);
		return FALSE;
	}

	dwRegCount = static_cast<DWORD>(m_csArrRegistryEntriesDel.GetCount());
	for (dwRegCount = 0; dwRegCount < static_cast<DWORD>(m_csArrRegistryEntriesDel.GetCount()); dwRegCount++)
	{
		szSubkey = L"";
		szSubkey = m_csArrRegistryEntriesDel.GetAt(dwRegCount);

		lResult = RegDeleteValue(hKey, szSubkey);
		if (lResult != ERROR_SUCCESS)
		{
			CString szMessage = NULL;
			szMessage.Format(L">>> CWWizUninstSecondDlg : DeleteRegistryKeys:: Unable to delete Registry key %s", szSubkey);
			AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
			return FALSE;

		}
	}

	if (!m_bUserSettings)
	{
		if (theApp.m_dwProductID == 2)
		{
			CString csKeyName = theApp.m_csProdKeyName + L"\\EmailScanSetting";
			lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, csKeyName);
			if (lResult == ERROR_SUCCESS)
			{
				AddLogEntry(L">>> successfully deleted key %s", csKeyName, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### failed to delete key %s", csKeyName, 0, true, SECONDLEVEL);
			}
		}

		lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, theApp.m_csProdKeyName);
		if (lResult == ERROR_SUCCESS)
		{
			AddLogEntry(L">>> successfully deleted key %s", theApp.m_csProdKeyName, 0, true, FIRSTLEVEL);
		}
		else
		{
			AddLogEntry(L"### failed to delete key %s", theApp.m_csProdKeyName, 0, true, SECONDLEVEL);
		}
	}


	lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F2935F4-F652-4E3E-BD71-0F1B4C91D5B1}_is1"));
	if (lResult == ERROR_SUCCESS)
	{
		AddLogEntry(L">>> successfully deleted key SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F2935F4-F652-4E3E-BD71-0F1B4C91D5B1}_is1", 0, 0, true, FIRSTLEVEL);
	}
	else
	{
		AddLogEntry(L"### failed to delet key SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F2935F4-F652-4E3E-BD71-0F1B4C91D5B1}_is1 ", 0, 0, true, SECONDLEVEL);
	}

	if (g_objReg.DelRegistryValueName(hRootKey, szSubKey, szValueName))
		AddLogEntry(L">>> No Registry value for SOFTWARE\\Microsoft\\Windows", szSubKey, szValueName, true, FIRSTLEVEL);
	else
		AddLogEntry(L">>> Deleted Registry value for SOFTWARE\\Microsoft\\Windows", szSubKey, szValueName, true, FIRSTLEVEL);

	return TRUE;
}

/***************************************************************************************************
*  Function Name  : DeleteAllAppShortcut
*  Description    : Deletes all shortcuts of application
*  Author Name    : Lalit K
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWardWizUninstallerDlg::DeleteAllAppShortcut()
{
	try
	{
		AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteAllAppShortcut", 0, 0, true, ZEROLEVEL);
		TCHAR CommonAppPath[MAX_PATH];
		HRESULT result = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, CommonAppPath);
		CString obj = CommonAppPath;
		swprintf_s(CommonAppPath, _countof(CommonAppPath), L"%s\\%s", CommonAppPath, L"Vibranium");
		RemoveFilesUsingSHFileOperation(CommonAppPath);

		if (theApp.m_dwProductID == ELITE)
		{
			TCHAR CommonAppPathWardWiz[MAX_PATH];
			HRESULT hResult = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, CommonAppPathWardWiz);
			CString csObj = CommonAppPathWardWiz;
			swprintf_s(CommonAppPathWardWiz, _countof(CommonAppPathWardWiz), L"%s\\%s", CommonAppPathWardWiz, L"Vibranium Client");
			RemoveFilesUsingSHFileOperation(CommonAppPathWardWiz);

			TCHAR DeskTopIconPath[MAX_PATH];
			HRESULT result1 = SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, DeskTopIconPath);
			swprintf_s(DeskTopIconPath, _countof(DeskTopIconPath), L"%s\\%s", DeskTopIconPath, L"Vibranium Client.lnk");
			DeleteFile(DeskTopIconPath);

			TCHAR CommDeskTopIconPath[MAX_PATH];
			HRESULT result2 = SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, CommDeskTopIconPath);
			swprintf_s(CommDeskTopIconPath, _countof(CommDeskTopIconPath), L"%s\\%s", CommDeskTopIconPath, L"Vibranium Client.lnk");
			DeleteFile(CommDeskTopIconPath);

			TCHAR CommDeskTopWardWizEliteIconPath[MAX_PATH];
			HRESULT WardWizEliteresult2 = SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, CommDeskTopWardWizEliteIconPath);
			swprintf_s(CommDeskTopWardWizEliteIconPath, _countof(CommDeskTopWardWizEliteIconPath), L"%s\\%s", CommDeskTopWardWizEliteIconPath, L"Vibranium Server.url");
			DeleteFile(CommDeskTopWardWizEliteIconPath);

			TCHAR CommonAppPathWardWizElite[MAX_PATH];
			HRESULT WardWizElitehResult = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, CommonAppPathWardWizElite);
			CString csWardWizEliteObj = CommonAppPathWardWizElite;
			swprintf_s(CommonAppPathWardWizElite, _countof(CommonAppPathWardWizElite), L"%s\\%s", CommonAppPathWardWizElite, L"Vibranium Server.url");
			RemoveFilesUsingSHFileOperation(CommonAppPathWardWizElite);

			TCHAR DeskTopWardWizEliteIconPath[MAX_PATH];
			HRESULT WardWizEliteresult1 = SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, DeskTopWardWizEliteIconPath);
			swprintf_s(DeskTopWardWizEliteIconPath, _countof(DeskTopWardWizEliteIconPath), L"%s\\%s", DeskTopWardWizEliteIconPath, L"Vibranium Server.url");
			DeleteFile(DeskTopWardWizEliteIconPath);

			TCHAR QuickLaunchIconPath[512] = { 0 };
			HRESULT result3 = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, QuickLaunchIconPath);
			swprintf_s(QuickLaunchIconPath, _countof(QuickLaunchIconPath), L"%s\\%s", QuickLaunchIconPath, L"Microsoft\\Internet Explorer\\Quick Launch\\Vibranium Client.lnk");
			DeleteFile(QuickLaunchIconPath);

			TCHAR IconCacheDB[512] = { 0 };
			HRESULT result6 = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, IconCacheDB);
			swprintf_s(IconCacheDB, _countof(IconCacheDB), L"%s\\%s", IconCacheDB, L"IconCache.db");
			DeleteFile(IconCacheDB);

			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
		}

		TCHAR CommonAppPathWardWiz[MAX_PATH];
		HRESULT hResult = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, CommonAppPathWardWiz);
		CString csObj = CommonAppPathWardWiz;
		swprintf_s(CommonAppPathWardWiz, _countof(CommonAppPathWardWiz), L"%s\\%s", CommonAppPathWardWiz, L"Vibranium");
		RemoveFilesUsingSHFileOperation(CommonAppPathWardWiz);

		TCHAR DeskTopIconPath[MAX_PATH];
		HRESULT result1 = SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, DeskTopIconPath);
		swprintf_s(DeskTopIconPath, _countof(DeskTopIconPath), L"%s\\%s", DeskTopIconPath, L"Vibranium.lnk");
		DeleteFile(DeskTopIconPath);
		//CString obj2= DeskTopIconPath;
		//AfxMessageBox(obj2);

		TCHAR CommDeskTopIconPath[MAX_PATH];
		HRESULT result2 = SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, CommDeskTopIconPath);
		swprintf_s(CommDeskTopIconPath, _countof(CommDeskTopIconPath), L"%s\\%s", CommDeskTopIconPath, L"Vibranium.lnk");
		DeleteFile(CommDeskTopIconPath);

		TCHAR QuickLaunchIconPath[512] = { 0 };
		HRESULT result3 = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, QuickLaunchIconPath);
		swprintf_s(QuickLaunchIconPath, _countof(QuickLaunchIconPath), L"%s\\%s", QuickLaunchIconPath, L"Microsoft\\Internet Explorer\\Quick Launch\\Vibranium.lnk");
		DeleteFile(QuickLaunchIconPath);

		TCHAR IconCacheDB[512] = { 0 };
		HRESULT result6 = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, IconCacheDB);
		swprintf_s(IconCacheDB, _countof(IconCacheDB), L"%s\\%s", IconCacheDB, L"IconCache.db");
		DeleteFile(IconCacheDB);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}
	catch (...)
	{
	}
}

/***************************************************************************
Function Name  : RemoveFilesUsingSHFileOperation
Description    : Function which removes file using SHFileOperation
This function can be used to copy, move, rename, or
delete a file system object.
Author Name    : Ramkrushna Shelke
SR.NO			 : EMAILSCANPLUGIN_0037
Date           : 07th Aug 2014
****************************************************************************/
DWORD CWardWizUninstallerDlg::RemoveFilesUsingSHFileOperation(TCHAR *pFolder)
{
	HRESULT			hr = 0x00;
	try
	{
		SHFILEOPSTRUCT	sfo = { 0 };

		TCHAR	szTemp[512] = { 0 };

		wsprintf(szTemp, L"%s\\*\0", pFolder);

		sfo.hwnd = NULL;
		sfo.wFunc = FO_DELETE;
		sfo.pFrom = szTemp;
		sfo.pTo = NULL;
		sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

		hr = SHFileOperation(&sfo);
		if (!hr)
		{
			_wrmdir((wchar_t *)pFolder);
			RemoveDirectory(pFolder);
		}
	}
	catch (...)
	{

	}
	return hr;
}

/***************************************************************************************************
*  Function Name  : DeleteFinished
*  Description    : called to kill the timer of status bar
*  Author Name    :	Nitin K
*  SR_NO          :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWardWizUninstallerDlg::DeleteFinished()
{
	AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteFinished", 0, 0, true, ZEROLEVEL);
	KillTimer(TIMER_DELETE_STATUS);
}

/***************************************************************************************************
*  Function Name  : UninstallationMessage
*  Description    : it shows the third dialog of uninstallation finished
*  Author Name    :	Nitin K
*  SR_NO          :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWardWizUninstallerDlg::UninstallationMessage()
{
	if (!m_bPassDbMngr || !m_bQuarantine)
	{
		MoveFileEx(m_csWardWizPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	}
}

/***************************************************************************
Function Name  : RemoveDriverRegistryKeyAndPauseDrivers
Description    : removes registry keys for drivers
Author Name    : lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
void CWardWizUninstallerDlg::RemoveDriverRegistryKeyAndPauseDrivers()
{
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int OsType = 0;
	bool returnStatus = false;
		
	try
	{
		OsType = objGetOSVersion.DetectClientOSVersion();
		if (OsType == 5)
		{
			scannerObj.PauseDriverProtection(0); // self protection
			cSecure64Obj.PauseDriverProtection(0); // self protection

			Sleep(10 * 1000);

			CString csDelKey = L"SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);

			CString csDelSecure64Key = L"SYSTEM\\CurrentControlSet\\Services\\WrdWizSecure64";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelSecure64Key);

		}
		else if (OsType == 10)
		{
			scannerObj.PauseDriverProtection(0); // self protection

			Sleep(10 * 1000);

			CString csDelKey = L"SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);
		}
		else
		{

			scannerObj.PauseDriverProtection(0); // self protection
			cSecure64Obj.PauseDriverProtection(0); // self protection							\\scanner

			Sleep(10 * 1000);

			CString csDelKey = L"SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);

			CString csDelSecure64Key = L"SYSTEM\\CurrentControlSet\\Services\\WrdWizSecure64";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelSecure64Key);
		}

		TCHAR	szDelKey[MAX_PATH] = { 0 };
		CString csDelKey = L"SYSTEM\\CurrentControlSet\\services\\WardwizALUSrv";
		RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);

		TCHAR	szDelComSrvKey[MAX_PATH] = { 0 };
		CString csDelComSrvKey = L"SYSTEM\\CurrentControlSet\\services\\VibraniumComSrv";
		RegDelnode(HKEY_LOCAL_MACHINE, csDelComSrvKey);
		
		CString csDelWRDWIZFLT = L"SYSTEM\\CurrentControlSet\\Services\\VBFLT";
		RegDelnode(HKEY_LOCAL_MACHINE, csDelWRDWIZFLT);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RemoveDriverService", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************
Function Name  : GetCloseAction4OutlookIfRunning
Description    : Closing outloog If Running.
Author Name    : Ramkrushna Shelke
S.R. No        : WRDWIZSETUPDLL_0004
Date           : 7/25/2014
****************************************************************************/
bool CWardWizUninstallerDlg::GetCloseAction4OutlookIfRunning(bool bIsReInstall)
{
	m_csProductName = L"BASIC";
	//No need to show the prompt for outlook close if the product is BASIC/ESSENTIAL
	if (m_csProductName == L"BASIC" || m_csProductName == L"ESSENTIAL")
	{
		return true;
	}

	while (true)
	{
		CEnumProcess objEnumProcess;
		if (objEnumProcess.IsProcessRunning(L"OUTLOOK.EXE", false, false, false))
		{
			CString csMessage;
			if (bIsReInstall)
				csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_OUTLOOK_CLOSE_REINSTALL_MSG");
			else
				csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_OUTLOOK_CLOSE_UNINSTALL_MSG");

			int iRet = MessageBox(csMessage, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_RETRYCANCEL);
			if (iRet == IDCANCEL)
			{
				return false;
			}
		}
		else
		{
			break;
		}
	}
	return true;
}

//*************************************************************
//  RegDelnode()
//  Purpose:    Deletes a registry key and all its subkeys / values.
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//*************************************************************
BOOL CWardWizUninstallerDlg::RegDelnode(HKEY hKeyRoot, CString csSubKey)
{
	try
	{
		return RegDelnodeRecurse(hKeyRoot, csSubKey);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CVibraniumUninstallerDlg::RegDelnode, SubKey[%s]", csSubKey, 0, true, SECONDLEVEL);
	}
	return FALSE;
}

//*************************************************************
//  RegDelnodeRecurse()
//  Purpose:    Deletes a registry key and all its subkeys / values.
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//*************************************************************
BOOL CWardWizUninstallerDlg::RegDelnodeRecurse(HKEY hKeyRoot, CString csSubKey)
{
	try
	{
		LPTSTR lpEnd = NULL;
		LONG lResult;
		DWORD dwSize;
		TCHAR szName[MAX_PATH];
		HKEY hKey;
		FILETIME ftWrite;

		// First, see if we can delete the key without having
		// to recurse.
		lResult = RegDeleteKey(hKeyRoot, csSubKey);
		if (lResult == ERROR_SUCCESS)
		{ 
			if (hKeyRoot != NULL)
				RegCloseKey(hKeyRoot);
			return TRUE;
		}

		lResult = RegOpenKeyEx(hKeyRoot, csSubKey, 0, KEY_READ, &hKey);
		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_FILE_NOT_FOUND) 
			{
				if (hKeyRoot != NULL)
					RegCloseKey(hKeyRoot);
				return TRUE;
			}
			else 
			{
				if (hKeyRoot != NULL)
					RegCloseKey(hKeyRoot);
				return FALSE;
			}
		}

		// Check for an ending slash and add one if it is missing.
		if (csSubKey.GetAt(csSubKey.GetLength()) != L'\\')
		{
			csSubKey += L'\\';
		}

		// Enumerate the keys
		dwSize = MAX_PATH;
		lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);
		if (lResult == ERROR_SUCCESS)
		{
			do {
				csSubKey += CString(szName);
				if (!RegDelnodeRecurse(hKeyRoot, csSubKey)) {
					break;
				}
				dwSize = MAX_PATH;
				lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);
			} while (lResult == ERROR_SUCCESS);
		}
		csSubKey = csSubKey.Left(csSubKey.ReverseFind('\\'));
		RegCloseKey(hKey);
		// Try again to delete the key.
		lResult = RegDeleteKey(hKeyRoot, csSubKey);
		if (lResult == ERROR_SUCCESS)
		{
			if (hKeyRoot != NULL)
				RegCloseKey(hKeyRoot);
			return TRUE;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CVibraniumUninstallerDlg::RegDelnodeRecurse, SubKey[%s]", csSubKey, 0, true, SECONDLEVEL);
	}
	return FALSE;
}

//*************************************************************
//  Function Name	:	EnableDisableControls
//  Purpose			:	Function to enable disable button control
//  Parameters		:	bool bEnable
//  Author			:	Ram Shelke
//  Return			:	void
//*************************************************************
void CWardWizUninstallerDlg::EnableDisableControls(bool bEnable)
{
	SCITER_VALUE sValue = bEnable ? L"1" : L"0";
	m_svEnableDisableButtons.call(SCITER_VALUE(CANCEL), sValue);
	m_svEnableDisableButtons.call(SCITER_VALUE(CLOSE), sValue);
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 Aug 2016
****************************************************************************************************/
json::value CWardWizUninstallerDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

json::value CWardWizUninstallerDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			//theApp.g_lbRetval = sciter::value();
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;
			theApp.m_objCompleteEvent.SetEvent();
			//::WaitForSingleObject(theApp.g_lbCompleteEvent, INFINITE);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

void CWardWizUninstallerDlg::CallNotificationMessage(SCITER_STRING strMessageString, int msgType)
{
	try
	{
		m_svShowWrdWizRunningCB.call((SCITER_STRING)strMessageString,msgType);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::CheckForRepairFileINIEntries()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CWardWizUninstallerDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : PreTranslateMessage
Description    : Ignore Enter/escape button click events
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 12th Oct 2016
/***************************************************************************************************/
BOOL CWardWizUninstallerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->wParam == VK_BACK)
	{
		bBackSpace = true;
	}
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE))
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
Function Name  : On_ButtonClickMinimize
Description    : button event handler to minimize window
Author Name    : Ram Shelke
SR_NO		   :
Date           : 18 MAY 2016
****************************************************************************/
json::value CWardWizUninstallerDlg::OnCloseCalled()
{
	try
	{
		PostQuitMessage(WM_CLOSE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::OnCloseCalled", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Amol Jaware
*  Date			  : 17 Nov 2016
****************************************************************************************************/
json::value CWardWizUninstallerDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

void CWardWizUninstallerDlg::OnClose()
{
	if (m_bUninstallInProgress)
	{
		if (bShowLightBox == true)
		{
			return;
		}
		bShowLightBox = true;
		CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UNINSTALLATION_IN_PROGRESS"), 1);
		if (theApp.m_bRetval == true)
		{
			bShowLightBox = false;
			return;
		}
	}
	else
	{
		OnCloseCalled();
	}
	//__super::OnClose();
}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : This function is used to send DB file path to UI
*  Author Name    : Amol Jaware
*  Date           :	21 March 2017
****************************************************************************************************/
json::value CWardWizUninstallerDlg::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s\\%s", theApp.m_AppPath, L"VBFOLDERLOCKER.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : GetDBPathforEPS
Description    : This function will get Database Path
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 20 February 2018
***********************************************************************************************/
json::value CWardWizUninstallerDlg::GetDBPathforEPS()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"\\VBFEATURESLOCK.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::GetDBPathforSched", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : GetDecryptPasssword
Description    : This function will get Password
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 21 March 2018
***********************************************************************************************/
json::value CWardWizUninstallerDlg::GetDecryptPasssword(SCITER_VALUE svEncryptPasssword)
{
	TCHAR szPassHash[MAX_PATH] = { 0 };
	try
	{
		SCITER_STRING  strEncryptPswd = svEncryptPasssword.get(L"");
		CString csEncryptPwd = strEncryptPswd.c_str();
		DWORD dwSize = csEncryptPwd.GetLength() + 1;

		DWORD dwStatus = theApp.CalculateMD5(csEncryptPwd.GetBuffer(), dwSize, szPassHash);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::GetDecryptPasssword", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szPassHash);
}

/***********************************************************************************************
Function Name  : LoadRequiredLibrary
Description    : Function Loads required Library
Author Name    : Amol Jaware
Date           : 02 Aug 2018
***********************************************************************************************/
bool CWardWizUninstallerDlg::LoadRequiredLibrary()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL)
		{
			return true;
		}

		if (!m_hModuleEmailDLL)
		{
			m_hModuleEmailDLL = LoadLibrary(L"VBEMAILSCN.DLL");
			if (!m_hModuleEmailDLL)
			{
				AddLogEntry(L"### Error in Loading VBEMAILSCN.DLL", 0, 0, true, SECONDLEVEL);
				return false;
			}

			m_lpEmailNFUnlink = (WRDWIZEMAILNFDLLUNLINK)GetProcAddress(m_hModuleEmailDLL, "WrdWizEmailNFDllUnlink");
			if (!m_lpEmailNFUnlink)
			{
				AddLogEntry(L"### CVibraniumUninstallerDlg:LoadRequiredLibrary Error in GetProcAddress::VibraniumEmailNFDll", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpEmailNFUnlink = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : GetNFDllUnlink
Description    : Function which unlink nf dll's.
Author Name    : Amol Jaware
Date           : 02 Aug 2018
***********************************************************************************************/
bool CWardWizUninstallerDlg::GetNFDllUnlink()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredLibrary();

		if (m_lpEmailNFUnlink)
		{
			bReturn = m_lpEmailNFUnlink();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::Initialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : UninstallFWDrivers
Description    : Function uninstall (ignis) firewall drivers.
Author Name    : Amol Jaware
Date           : 03 Aug 2018
***********************************************************************************************/
void CWardWizUninstallerDlg::UninstallFWDrivers()
{
	try
	{
		//WardWiz UnInstallation path should not be blank else it will remove all files from the system.
		if (m_csWardWizPath.Trim().GetLength() == 0x00)
		{
			AddLogEntry(L"### Failed to get Vibraniumpath from registry CVibraniumUninstallerDlg::UninstallFWDrivers", 0, 0, true, SECONDLEVEL);
			return;
		}
		CString csWrdWziPath = m_csWardWizPath;
		//CString csCommandLine;
		//csCommandLine = L"STOP.CMD";
		csWrdWziPath += "DRIVERS\\IGNIS_STOP.CMD";
		if ((int)ShellExecute(NULL, NULL, csWrdWziPath, NULL, NULL, SWP_HIDEWINDOW) < 32)
			AddLogEntry(L"### Failed ShellExecute to uninstall firewall drivers CVibraniumUninstallerDlg::UninstallFWDrivers", 0, 0, true, SECONDLEVEL);
		else
			AddLogEntry(L"### Successed ShellExecute to uninstall firewall drivers CVibraniumUninstallerDlg::UninstallFWDrivers", 0, 0, true, SECONDLEVEL);

		Sleep(200);

		csWrdWziPath += "DRIVERS\\STOPVibraniumFLT.BAT";
		if ((int)ShellExecute(NULL, NULL, csWrdWziPath, NULL, NULL, SWP_HIDEWINDOW) < 32)
			AddLogEntry(L"### Failed ShellExecute to uninstall VibraniumFLT drivers CVibraniumUninstallerDlg::UninstallFWDrivers", 0, 0, true, SECONDLEVEL);
		else
			AddLogEntry(L"### Successed ShellExecute to uninstall VibraniumFLT drivers CVibraniumUninstallerDlg::UninstallFWDrivers", 0, 0, true, SECONDLEVEL);

		CString csDelKey = L"Software\\ignis";
		RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::UninstallFWDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : GetFWDriverPath
Description    : This function used to get path of drivers folder to delete ignis drivers for firewall.
Author Name    : Amol Jaware
Date           : 04 Aug 2018
***********************************************************************************************/
void CWardWizUninstallerDlg::GetFWDriverPath()
{
	CString csWrdWizFLTDriverPath;
	TCHAR systemDirPath[MAX_PATH] = _T("");
	try
	{
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));
		csWrdWizFLTDriverPath.Format(L"%s\\drivers\\%s", systemDirPath, L"ignis.sys");
		if (PathFileExists(csWrdWizFLTDriverPath))
			DeleteFWDrivers(csWrdWizFLTDriverPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::GetFWDriverPath", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : DeleteFWDrivers
Description    : This function will delete all firewall (ignis) drivers from drivers folder.
Author Name    : Amol Jaware
Date           : 04 Aug 2018
***********************************************************************************************/
void CWardWizUninstallerDlg::DeleteFWDrivers(CString csWrdWizFLTDriverPath)
{
	try
	{
		SetFileAttributes(csWrdWizFLTDriverPath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(csWrdWizFLTDriverPath))
		{
			AddLogEntry(L"### Failed to delete file CVibraniumUninstallerDlg::DeleteFWDrivers %s", csWrdWizFLTDriverPath, 0, true, FIRSTLEVEL);
			if (PathFileExists(csWrdWizFLTDriverPath))
			{
				MoveFileEx(csWrdWizFLTDriverPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::DeleteFWDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : RestoreReqRegistry
Description    : This function will restore required registry of Usb.
Author Name    : Akshay Patil
Date           : 26 Oct 2018
***********************************************************************************************/
void CWardWizUninstallerDlg::RestoreReqRegistry()
{
	try
	{
		DWORD dwStartType = 0x00;
		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\USBSTOR", L"Start", dwStartType) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for Start in CVibraniumUninstallerDlg::RestoreReqRegistry", 0, 0, true, SECONDLEVEL);;
		}

		if (dwStartType == 0x04)
		{	
			if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\USBSTOR", L"Start", 0x03) != 0x00)
			{
				AddLogEntry(L"### SetRegistryValueData failed for HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\USBSTOR ", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::RestoreReqRegistry", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : StartUninstllSqlServerAndDisableIISService()
*  Description    : This Function uninstall the Sql Server 2012.
*  Author Name    : Tejas Shinde
*  Date			  :	04 Dec 2018
****************************************************************************************************/
void CWardWizUninstallerDlg::StartUninstllSqlServerAndDisableIISServices()
{
	if (theApp.m_dwProductID == ELITE)
	{
		IsWow64();

		if (m_bIsWow64 == true)
		{
			CITinRegWrapper objReg;
			CString g_csRegKeyPath = L"SOFTWARE\\Vibranium";
			TCHAR szValue[MAX_PATH] = { 0 };
			DWORD dwSize = sizeof(szValue);
			objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize);
			CString csAppPathValue = L"";
			csAppPathValue.Format(L"%s", szValue);
			CString csExePath, csCommandLine, csUnistallSqlServerExePath, csUninstallPowerShellExePath, csDisableIISServicePath;
			csExePath.Format(L"%s%s", csAppPathValue, L"SQLEXPR_x86_ENU.exe");
			csUnistallSqlServerExePath.Format(L"%s%s", csAppPathValue, L"x64_CONFIGUNINSTALLSQLSERVERCMDFILE.ini");
			CString csPath = L"\"";
			csUnistallSqlServerExePath += L"\"";
			csPath += csUnistallSqlServerExePath;
			csCommandLine.Format(L"%s%s", L"/Q /IACCEPTSQLSERVERLICENSETERMS=1 /ConfigurationFile=", csPath);
			LPTSTR csFinalSqlServerExePath = csExePath.GetBuffer(csExePath.GetLength());
			LPTSTR csFinalSqlCommandLine = csCommandLine.GetBuffer(csCommandLine.GetLength());
			LaunchProcessThrCommandLine(csFinalSqlServerExePath, csFinalSqlCommandLine);

			TCHAR scFOLDERID_ProgramFilesX86_on64[MAX_PATH];
			CString  csSqlServerFinalPath, csSqlServerInstancePath, csSqlServerInstanceFolderPath, csWWIZSQLDBLOGPath;
			SHGetSpecialFolderPath(0, scFOLDERID_ProgramFilesX86_on64, CSIDL_PROGRAM_FILESX86, FALSE);
			csSqlServerFinalPath.Format(L"%s%s", scFOLDERID_ProgramFilesX86_on64, L"\\Microsoft SQL Server");
			csSqlServerInstancePath.Format(L"%s%s", scFOLDERID_ProgramFilesX86_on64, L"\\Microsoft SQL Server\\MSSQL11.WARDWIZ");
			csSqlServerInstanceFolderPath.Format(L"%s%s", scFOLDERID_ProgramFilesX86_on64, L"\\Microsoft SQL Server\\MSSQL11.WARDWIZ\\MSSQL\\DATA");
			csWWIZSQLDBLOGPath.Format(L"%s%s", scFOLDERID_ProgramFilesX86_on64, L"\\Microsoft SQL Server\\WWIZINSTSQLDBLOG.TXT");

			LPTSTR csFinalSqlServerWWIZExePath = csSqlServerFinalPath.GetBuffer(csSqlServerFinalPath.GetLength());
			LPTSTR csFinalSqlServerInstanceDBFolderPath = csSqlServerInstanceFolderPath.GetBuffer(csSqlServerInstanceFolderPath.GetLength());
			LPTSTR csFinalWWIZINSTSQLDBLOGPath = csWWIZSQLDBLOGPath.GetBuffer(csWWIZSQLDBLOGPath.GetLength());
			GetBackupSqlServerWWIZInstanceDB(csFinalSqlServerWWIZExePath, csFinalSqlServerInstanceDBFolderPath, csFinalWWIZINSTSQLDBLOGPath);

			if (csSqlServerInstancePath.Trim().GetLength() != 0x00)
			{
				if (PathFileExists(csSqlServerInstancePath))
				{
					DeleteSqlServerWWIZInstanceDirectory(csSqlServerInstancePath);
				}
			}

			TCHAR szSndPSCmdValue[MAX_PATH] = { 0 };
			DWORD dwNewSize = sizeof(szSndPSCmdValue);
			g_csRegKeyPath = L"SOFTWARE\\Microsoft\\PowerShell\\1\\PowerShellEngine";
			objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"ApplicationBase", szSndPSCmdValue, dwNewSize);
			CString csPowerShellPathValue = L"";
			csPowerShellPathValue.Format(L"%s", szSndPSCmdValue);
			csUninstallPowerShellExePath.Format(L"%s%s", csPowerShellPathValue, L"\\powershell.exe");
			csDisableIISServicePath.Format(L"%s%s", csAppPathValue, L"DISABLE_IIS.ps1");
			CString csDISABLE_IISServicePath = L"\"";
			csDisableIISServicePath += L"\"";
			csDISABLE_IISServicePath += csDisableIISServicePath;
			csCommandLine.Format(L"%s%s", L"-ExecutionPolicy RemoteSigned -windowstyle hidden -file ", csDISABLE_IISServicePath);
			LPTSTR csFinalPowerShellExePath = csUninstallPowerShellExePath.GetBuffer(csUninstallPowerShellExePath.GetLength());
			LPTSTR csFinalDisableIIServiceCommandLine = csCommandLine.GetBuffer(csCommandLine.GetLength());
			LaunchProcessThrCommandLine(csFinalPowerShellExePath, csFinalDisableIIServiceCommandLine);
		}
		else
		{
			CITinRegWrapper objReg;
			CString g_csRegKeyPath = L"SOFTWARE\\Vibranium";
			TCHAR szValue[MAX_PATH] = { 0 };
			DWORD dwSize = sizeof(szValue);
			objReg.GetRegistryValueData32(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize);
			CString csAppPathValue = L"";
			csAppPathValue.Format(L"%s", szValue);
			CString csExePath, csCommandLine, csUnistallSqlServerExePath, csUninstallPowerShellExePath, csDisableIISServicePath;
			csExePath.Format(L"%s%s", csAppPathValue, L"SQLEXPR_x86_ENU.exe");
			csUnistallSqlServerExePath.Format(L"%s%s", csAppPathValue, L"x86_CONFIGUNINSTALLSQLSERVERCMDFILE.ini");
			CString csPath = L"\"";
			csUnistallSqlServerExePath += L"\"";
			csPath += csUnistallSqlServerExePath;
			csCommandLine.Format(L"%s%s", L"/Q /IACCEPTSQLSERVERLICENSETERMS=1 /ConfigurationFile=", csPath);
			LPTSTR csFinalSqlServerExePath = csExePath.GetBuffer(csExePath.GetLength());
			LPTSTR csFinalSqlCommandLine = csCommandLine.GetBuffer(csCommandLine.GetLength());
			LaunchProcessThrCommandLine(csFinalSqlServerExePath, csFinalSqlCommandLine);

			TCHAR scFOLDERID_ProgramFilesX86_on32[MAX_PATH];
			CString  csSqlServerFinalPath, csSqlServerInstancePath, csSqlServerInstanceFolderPath, csWWIZSQLDBLOGPath;
			SHGetSpecialFolderPath(0, scFOLDERID_ProgramFilesX86_on32, CSIDL_PROGRAM_FILES, FALSE);
			csSqlServerFinalPath.Format(L"%s%s", scFOLDERID_ProgramFilesX86_on32, L"\\Microsoft SQL Server");
			csSqlServerInstancePath.Format(L"%s%s", scFOLDERID_ProgramFilesX86_on32, L"\\Microsoft SQL Server\\MSSQL11.WARDWIZ");
			csSqlServerInstanceFolderPath.Format(L"%s%s", scFOLDERID_ProgramFilesX86_on32, L"\\Microsoft SQL Server\\MSSQL11.WARDWIZ\\MSSQL\\DATA");
			csWWIZSQLDBLOGPath.Format(L"%s%s", scFOLDERID_ProgramFilesX86_on32, L"\\Microsoft SQL Server\\WWIZINSTSQLDBLOG.TXT");

			LPTSTR csFinalSqlServerWWIZExePath = csSqlServerFinalPath.GetBuffer(csSqlServerFinalPath.GetLength());
			LPTSTR csFinalSqlServerInstanceDBFolderPath = csSqlServerInstanceFolderPath.GetBuffer(csSqlServerInstanceFolderPath.GetLength());
			LPTSTR csFinalWWIZINSTSQLDBLOGPath = csWWIZSQLDBLOGPath.GetBuffer(csWWIZSQLDBLOGPath.GetLength());
			GetBackupSqlServerWWIZInstanceDB(csFinalSqlServerWWIZExePath, csFinalSqlServerInstanceDBFolderPath, csFinalWWIZINSTSQLDBLOGPath);
			if (csSqlServerInstancePath.Trim().GetLength() != 0x00)
			{
				if (PathFileExists(csSqlServerInstancePath))
				{
					DeleteSqlServerWWIZInstanceDirectory(csSqlServerInstancePath);
				}
			}

			TCHAR szSndPSCmdValue[MAX_PATH] = { 0 };
			DWORD dwNewSize = sizeof(szSndPSCmdValue);
			g_csRegKeyPath = L"SOFTWARE\\Microsoft\\PowerShell\\1\\PowerShellEngine";
			objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"ApplicationBase", szSndPSCmdValue, dwNewSize);
			CString csPowerShellPathValue = L"";
			csPowerShellPathValue.Format(L"%s", szSndPSCmdValue);
			csUninstallPowerShellExePath.Format(L"%s%s", csPowerShellPathValue, L"\\powershell.exe");
			csDisableIISServicePath.Format(L"%s%s", csAppPathValue, L"DISABLE_IIS.ps1");
			CString csDISABLE_IISServicePath = L"\"";
			csDisableIISServicePath += L"\"";
			csDISABLE_IISServicePath += csDisableIISServicePath;
			csCommandLine.Format(L"%s%s", L"-ExecutionPolicy RemoteSigned -windowstyle hidden -file ", csDISABLE_IISServicePath);
			LPTSTR csFinalPowerShellExePath = csUninstallPowerShellExePath.GetBuffer(csUninstallPowerShellExePath.GetLength());
			LPTSTR csFinalDisableIIServiceCommandLine = csCommandLine.GetBuffer(csCommandLine.GetLength());
			LaunchProcessThrCommandLine(csFinalPowerShellExePath, csFinalDisableIIServiceCommandLine);
		}
	}
}

/***************************************************************************************************
*  Function Name  : IsWow64()
*  Description    : It will check client machine is 64 bit or 32bit.
*  Author Name    : Tejas Shinde
*  Date			  :	04 Dec 2018
****************************************************************************************************/
void CWardWizUninstallerDlg::IsWow64()
{
	TCHAR				szOSVer[16] = { 0 };
	SYSTEM_INFO			sysInfo = { 0 };
	__try
	{

		SYSTEM_INFO			sysInfo = { 0 };
		GetNativeSystemInfo(&sysInfo);
		if ((sysInfo.wProcessorArchitecture&PROCESSOR_ARCHITECTURE_AMD64) == PROCESSOR_ARCHITECTURE_AMD64)
		{
			m_bIsWow64 = true;
		}
		else
		{
			m_bIsWow64 = false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::IsWow64", 0, 0, true, SECONDLEVEL);
	}
}
/***********************************************************************************************
Function Name  : LaunchProcessThrCommandLine
Description    : This function will get CommandLine As Paramaters and Execute the Command
Author Name    : Tejas Shinde
Date           : 04 Dec 2018
***********************************************************************************************/
BOOL CWardWizUninstallerDlg::LaunchProcessThrCommandLine(LPTSTR pszAppPath, LPTSTR pszCmdLine)
{

	if (!pszAppPath)
		return FALSE;

	if (!pszCmdLine)
		return FALSE;

	STARTUPINFO			si = { 0 };
	PROCESS_INFORMATION	pi = { 0 };

	try
	{

		si.cb = sizeof(STARTUPINFO);

		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		TCHAR commandLine[2 * MAX_PATH + 16] = { 0 };
		swprintf_s(commandLine, _countof(commandLine), L"\"%s\" %s ", pszAppPath, pszCmdLine);

		if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		{
			AddLogEntry(L"### Failed CVibraniumUninstallerDlg::LaunchProcessThrCommandLine : [%s]", commandLine);
			return TRUE;
		}

		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;


	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUninstallerDlg::LaunchProcessThrCommandLine");
		return TRUE;
	}

	return FALSE;
}
/******************************************************************************************************************************
*  Function Name  : CreateDirectoryWWIZINSTANCEBACKUP
*  Description    : It will check WWIZInstance Backup directory. If not present, it will create WWIZInstance Backup directory.
*  Author Name    : Tejas Shinde
*  Date			  :	04 Dec 2018
*******************************************************************************************************************************/
bool CWardWizUninstallerDlg::CreateDirectoryWwizInstanceBackup(LPTSTR lpszPath)
{
	__try
	{
		CreateDirectory(lpszPath, NULL);
		if (PathFileExists(lpszPath))
			return false;

		_wmkdir(lpszPath);
		if (PathFileExists(lpszPath))
			return false;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### CVibraniumUninstallerDlg::CreateDirectoryVibraniumInstanceBackup::Exception", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***********************************************************************************************
*  Function Name  : DeleteSqlServerWWIZDirectory
*  Description    : This Function Get Sql Server DB File From Mysql11.Wardwiz Instance Directory
*  Author Name    : Tejas Shinde
*  Date           : 04 Dec 2018
*************************************************************************************************/
bool CWardWizUninstallerDlg::GetBackupSqlServerWWIZInstanceDB(CString csSourceSqlDir, LPTSTR csDestSqlServerDir, LPTSTR csSqlWwizInstaDbLog)
{
	try
	{
		TCHAR	m_szBackupSqlDBDirName[128];
		TCHAR csDestPathTemp[512] = { 0 };
		TCHAR		csWWizSqlServerDestPath[512] = { 0 };
		SYSTEMTIME	ST = { 0 };
		TCHAR	szTemp[512] = { 0 };
		TCHAR	szFileName[128] = { 0 };
		TCHAR	szFilePath[512] = { 0 };
		bool	bCopied = false;
		CFileFind finder;
		GetLocalTime(&ST);

		if (!csSourceSqlDir)
			return FALSE;

		if (!csDestSqlServerDir)
			return FALSE;

		swprintf_s(m_szBackupSqlDBDirName, _countof(m_szBackupSqlDBDirName), L"_%d_%d_%d_%d_%d", ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute);
		CString GetTempSqlServerPath;
		CString csPath = L"";
		csPath += csSourceSqlDir;
		GetTempSqlServerPath.Format(L"%s%s%s%s", csPath, L"\\WardwizBackup", m_szBackupSqlDBDirName, L".MDF");
		LPTSTR csFinalSqlServerExePath = GetTempSqlServerPath.GetBuffer(GetTempSqlServerPath.GetLength());

		CString strWildcard(csFinalSqlServerExePath);
		strWildcard += _T("\\*.*");

		//CString csFilePath = csFinalSqlServerExePath + L"\\WWIZINSTSQLDBLOG.txt";

		if (!PathFileExists(csFinalSqlServerExePath))
		{
			if (CreateDirectoryWwizInstanceBackup(csFinalSqlServerExePath))
			{
				AddLogEntry(L"### Failed to create & Check dest folder path :: %s", csFinalSqlServerExePath, 0, true, SECONDLEVEL);
			}
		}

		TCHAR csFilePath[MAX_PATH];
		TCHAR csExistingFilePath[MAX_PATH];
		swprintf_s(csFilePath, _countof(csFilePath), L"%s\\WWIZINSTSQLDBLOG.txt", csFinalSqlServerExePath);
		//existing file name
		swprintf_s(csExistingFilePath, _countof(csExistingFilePath), L"%s\\WWIZINSTSQLDBLOG.txt", csSourceSqlDir);
		//HANDLE hSFile = CreateFile(csFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		HANDLE hSFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (CopyFile(csExistingFilePath, csFilePath, FALSE))
		{
			bCopied = true;
		}
		else
		{
			AddLogEntry(L">>> Failed to copy file:: %s", csFinalSqlServerExePath, 0, true, ZEROLEVEL);
			bCopied = false;
		}
		TCHAR csSqlWWIZEPSWEBMDFFilePath[MAX_PATH];
		TCHAR csExistingSqlWWIZEPSWEBFilePath[MAX_PATH];
		swprintf_s(csSqlWWIZEPSWEBMDFFilePath, _countof(csSqlWWIZEPSWEBMDFFilePath), L"%s\\wardwizepsweb.mdf", csFinalSqlServerExePath);
		swprintf_s(csExistingSqlWWIZEPSWEBFilePath, _countof(csExistingSqlWWIZEPSWEBFilePath), L"%s\\wardwizepsweb.mdf", csDestSqlServerDir);
		HANDLE hSWWIZEPSWEBMDFFile = CreateFile(csSqlWWIZEPSWEBMDFFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (CopyFile(csExistingSqlWWIZEPSWEBFilePath, csSqlWWIZEPSWEBMDFFilePath, FALSE))
		{
			bCopied = true;
		}
		else
		{
			AddLogEntry(L">>> Failed to copy file:: %s", csFinalSqlServerExePath, 0, true, ZEROLEVEL);
			bCopied = false;
		}

		TCHAR csSqlWWIZEPSWEB_LOGLDF_FilePath[MAX_PATH];
		TCHAR csExistingSqlWWIZEPSWEB_LOGLDF_FilePath[MAX_PATH];
		swprintf_s(csSqlWWIZEPSWEB_LOGLDF_FilePath, _countof(csSqlWWIZEPSWEB_LOGLDF_FilePath), L"%s\\wardwizepsweb_log.ldf", csFinalSqlServerExePath);
		//existing file name wardwizepsweb_log.ldf
		swprintf_s(csExistingSqlWWIZEPSWEB_LOGLDF_FilePath, _countof(csExistingSqlWWIZEPSWEB_LOGLDF_FilePath), L"%s\\wardwizepsweb_log.ldf", csDestSqlServerDir);
		HANDLE hSWWIZEPSWEBLOGLDFFile = CreateFile(csSqlWWIZEPSWEB_LOGLDF_FilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (CopyFile(csExistingSqlWWIZEPSWEB_LOGLDF_FilePath, csSqlWWIZEPSWEB_LOGLDF_FilePath, FALSE))
		{
			bCopied = true;
		}
		else
		{
			AddLogEntry(L">>> Failed to copy file:: %s", csFinalSqlServerExePath, 0, true, ZEROLEVEL);
			bCopied = false;
		}

		DeleteFile(csSqlWwizInstaDbLog);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::GetBackupSqlServerVibraniumInstanceDB", 0, 0, true, SECONDLEVEL);
	}
	return false;
}
/******************************************************************************************************************
*  Function Name  : IsDots
*  Description    : Function to  if the found file has the name "." or "..",then indicating actually a directory.
*  Author Name    : Tejas Shinde
*  Date           : 04 Dec 2018
*******************************************************************************************************************/
BOOL CWardWizUninstallerDlg::IsDots(const TCHAR* str)
{
	try
	{
		if (_tcscmp(str, (const wchar_t*) ".") && _tcscmp(str, (const wchar_t*) "..")) return FALSE;
		return TRUE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUninstallerDlg::IsDots");
		return TRUE;
	}

}

/***********************************************************************************************
*  Function Name  : DeleteSqlServerWWIZDirectory
*  Description    : Function to delete the Sql Server Wardwiz Instance Folder completely
*  Author Name    : Tejas Shinde
*  Date           : 04 Dec 2018
*************************************************************************************************/
bool CWardWizUninstallerDlg::DeleteSqlServerWWIZInstanceDirectory(CString csInputFileName)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(csInputFileName);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString str = finder.GetFilePath();
				DeleteSqlServerWWIZInstanceDirectory(str);
			}
			else
			{
				CString strFilePath = finder.GetFilePath();
				SetFileAttributes(strFilePath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(strFilePath);
			}
		}
		finder.Close();
		RemoveDirectory(csInputFileName);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::DeleteSqlServerVibraniumDirectory", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Kunal Waghmare
*  Date			  : 26 Dec 2018
****************************************************************************************************/
json::value CWardWizUninstallerDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : FunCheckInternetAccessBlock
*  Description    : To check internet access block
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 Dec 2019
****************************************************************************************************/
json::value CWardWizUninstallerDlg::FunCheckInternetAccessBlock()
{
	bool RetVal = false;
	try
	{
		DWORD dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
		if (dwProductID == BASIC || dwProductID == ESSENTIAL)
		{
			return false;
		}

		CString csRegKeyVal;
		csRegKeyVal = CWWizSettingsWrapper::GetProductRegistryKey();
		CITinRegWrapper objReg;
		DWORD dwParentalControl = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyVal.GetBuffer(), L"dwParentalCntrlFlg", dwParentalControl) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CVibraniumUninstallerDlg::FunCheckInternetAccessBlock", 0, 0, true, SECONDLEVEL);
		}

		if (dwParentalControl == 1)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = ON_CHECK_INTERNET_ACCESS;

			CISpyCommunicator objCom(SERVICE_SERVER);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send Data in CVibraniumUninstallerDlg::SendData", 0, 0, true, SECONDLEVEL);
			}

			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read Data in CVibraniumUninstallerDlg::ReadData", 0, 0, true, SECONDLEVEL);
			}

			DWORD dwVal = szPipeData.dwValue;
			if (dwVal == 0x01)
			{
				RetVal = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUninstallerDlg::FunCheckInternetAccessBlock()", 0, 0, true, SECONDLEVEL);
	}
	return RetVal;
}