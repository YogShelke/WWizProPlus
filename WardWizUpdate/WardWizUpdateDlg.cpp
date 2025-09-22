/*********************************************************************************************************************************
*  Program Name	: WardWizUpdateDlg.cpp
*  Description	: WardWizUpdateDlg Implementation
*  Author Name	: Amol Jaware
*  Date Of Creation: 15 Jan 2019
*  Version No	: 4.1.0.1
**********************************************************************************************************************************/
// WardWizUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWizUpdate.h"
#include "WardWizUpdateDlg.h"
#include "afxdialogex.h"
#include "WardWizDatabaseInterface.h"
#include "WrdwizEncDecManager.h"
#include "ISpyCommServer.h"
#include "WardWizUpdateManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD	WINAPI StartALUpdateProcessThread(LPVOID lpParam);
DWORD	UpdateManagerTaskThread(LPVOID lpParam);
DWORD	WINAPI UpdateFromLocalFolderThread(LPVOID lpvThreadParam);
INT64	EnterUpdateDetails(int iFileCount, int iFileSize);
CString GetSQLiteDBFileSource();

CString			g_csPreviousListControlStatus = L"";
CString			g_csWardWizModulePath;
CString			g_csTaskID = L"0";

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

// CWardWizUpdateDlg dialog

CWardWizUpdateDlg	*g_objWardWizUpdateDlg;

/***************************************************************************************************
*  Function Name  : CWardWizUpdateDlg
*  Description    : CTOR CWardWizUpdateDlg
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
CWardWizUpdateDlg::CWardWizUpdateDlg(CWnd* pParent)
	: CDialogEx(CWardWizUpdateDlg::IDD, pParent)
	//, m_objIPCALUpdateClient(ALUPDATE)
	, m_dwCurrentLocalFileSize(0x00)
	, m_iTotalNoofFiles(0x00)
	, m_objWardWizUpdateManager(&CWardWizUpdateDlg::UpdateOpProcess)
	, m_bIsManualStop(false)
	, m_bIsStopFrmTaskbar(false)
	, m_hThread_StartALUpdateProcess(NULL)
	, m_hThreadStartUpdateManagerTask(NULL)
	, m_csDispTotSizeExt(_T("KB"))
	, m_csCurrentDownloadedbytes(_T("[KB]"))
	, m_bRetval(false)
	, m_iRetval(0)
	, m_csTotalFileSize(L"")
	, m_csDownloadPercentage(L"")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_objWardWizUpdateDlg = this;
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
void CWardWizUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWardWizUpdateDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

HWINDOW   CWardWizUpdateDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizUpdateDlg::get_resource_instance() { return theApp.m_hInstance; }

// CWardWizUpdateDlg message handlers

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Loading Update html file.
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
BOOL CWardWizUpdateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (theApp.m_bEPSLiveUpdateNoUI == true)
	{
		StartEPSLiveUpdateNOUI();
	}
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
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)AfxGetResourceHandle(), L"res:IDR_HTM_UPDATE.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_UPDATE.htm");
	this->SetWindowText(L"VibraniumUPDATE");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;

	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if (theApp.m_bCmdLineUpdate == true)
	{
		this->ShowLiveUpdate();
	}

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************************************
*  Function Name  : OnSysCommand
*  Description    : Handels Control-menu request for the predefined actions specified in the preceding table.
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
void CWardWizUpdateDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    : Handels ON_WM_PAINT to paint the dialog(window).
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
void CWardWizUpdateDlg::OnPaint()
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
HCURSOR CWardWizUpdateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : To handle the Sciter UI related requests
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
LRESULT CWardWizUpdateDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	__try
	{
		if (message == WM_TIMER)
		{
			if (LOWORD(wParam) == TIMER_SCAN_STATUS)
			{
				//m_pThis->m_objWardWizScan.OnTimerScan();
			}
		}
		else
		{
			lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
		}
		if (bHandled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : Get the App Path and Set it in Script
Author Name    : Aml Jaware
SR_NO		   :
Date           : 20th Jan 2019
/***************************************************************************************************/
json::value CWardWizUpdateDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : Get the language ID and Set it in Script
Author Name    : Amol Jaware
SR_NO		   :
Date           : 20th Jan 2019
/***************************************************************************************************/
json::value CWardWizUpdateDlg::On_GetLanguageID()
{
	int iLangValue = 0;

	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}

	return iLangValue;
}

/***************************************************************************************************
Function Name  : On_GetProductID
Description    : Get the product ID.
Author Name    : Amol Jaware
SR_NO		   :
Date           : 20th Jan 2019
/***************************************************************************************************/
json::value CWardWizUpdateDlg::On_GetProductID()
{
	int iProdValue = 0;

	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}

	return iProdValue;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : Get the UI theme ID.
Author Name    : Amol Jaware
SR_NO		   :
Date           : 20th Jan 2019
/***************************************************************************************************/
json::value CWardWizUpdateDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
Function Name  : On_Minimize
Description    : Minimize the UI.
Author Name    : Amol Jaware
SR_NO		   :
Date           : 20th Jan 2019
/***************************************************************************************************/
json::value CWardWizUpdateDlg::On_Minimize()
{
	try
	{
		OnBnClickedButtonMinimize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_Minimize", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
Function Name  : On_Close
Description    : close the UI.
Author Name    : Nitin Shelar
SR_NO		   :
Date           : 09/10/2019
/***************************************************************************************************/
json::value CWardWizUpdateDlg::On_Close()
{
	try
	{
		if (m_hThread_StartALUpdateProcess)
			TheadTerminationState(m_hThread_StartALUpdateProcess);

		CDialogEx::OnCancel();
		HWND hWindow = ::FindWindow(NULL, L"VibraniumUPDATE");
		if (hWindow)
			exit(0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_Close", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : OnBnClickedButtonMinimize
Description    : Minimize the UI.
Author Name    : Amol Jaware
SR_NO		   :
Date           : 20th Jan 2019
/***************************************************************************************************/
void CWardWizUpdateDlg::OnBnClickedButtonMinimize()
{
	this->ShowWindow(SW_MINIMIZE);
}

/***********************************************************************************************
Function Name  : On_ClickUpdatesFromLocalFolder
Description    : Local Folder Updates
SR.NO		   :
Author Name    : Nihar Deshpande
Date           : 19-05-2016
***********************************************************************************************/
json::value CWardWizUpdateDlg::On_ClickUpdatesFromLocalFolder(SCITER_VALUE svFilePath, SCITER_VALUE svNotificationMessageFromLocalCB)
{
	try
	{
		m_UnzipFile = NULL;
		m_StopUnRarOperation = NULL;
		LoadExtractDll();  //load extractdll
		m_bAborted = false;

		m_bOfflineUpdateStarted = false;
		m_svNotificationMessageFromLocalCB = svNotificationMessageFromLocalCB;
		SCITER_STRING  strFilePath = svFilePath.get(L"");
		CString csFilePath = strFilePath.c_str();
		m_updateType = UPDATEFROMLOCALFOLDER;
		m_csInputFolderPath = csFilePath;
		OnBnClickedButtonNext();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUpdateDlg::On_ClickUpdatesFromLocalFolder", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : InitializeUpdateProcThread
Description    : On click Live Update Next button thread should be initialize with NULL.
SR.NO		   :
Author Name    : Amol Jaware
Date           : 21-02-2019
***********************************************************************************************/
void CWardWizUpdateDlg::InitializeUpdateProcThread()
{
	try
	{
		if (m_hThread_StartALUpdateProcess)
			m_hThread_StartALUpdateProcess = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUpdateDlg::InitializeUpdateProcThread", 0, 0, true, SECONDLEVEL);
	}

}

/***********************************************************************************************
Function Name  : On_ClickUpdateFromInternet
Description    :On line update
SR.NO		   :
Author Name    : Nihar Deshpande
Date           : 19-05-2016
***********************************************************************************************/
json::value CWardWizUpdateDlg::On_ClickUpdateFromInternet(SCITER_VALUE pSetUpdateStatusCb, SCITER_VALUE m_pAddUpdateTableCb, SCITER_VALUE m_pUpdateUpdateTableCb, SCITER_VALUE m_pRowAddCb, SCITER_VALUE m_pUpdateCompleteCb)
{
	try
	{
		InitializeUpdateProcThread();
		m_svLiveUpdateStatusFunctionCB = pSetUpdateStatusCb;
		m_svAddUpdateTableCb = m_pAddUpdateTableCb;
		m_svUpdateUpdateTableCb = m_pUpdateUpdateTableCb;
		m_svRowAddCb = m_pRowAddCb;
		m_svUpdateCompleteCb = m_pUpdateCompleteCb;
		m_updateType = UPDATEFROMINTERNT;
		m_dwCurrentLocalFileSize = 0x00;
		m_iTotalNoofFiles = 0;
		m_iUpdateID = EnterUpdateDetails(0, 0);
		OnBnClickedButtonNext();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_ClickUpdateFromInternet", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***********************************************************************************************
*  Function Name  : OnBnClickedButtonNext
*  Description    : After clicking Next button. It shows it's child dialog
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-5-2016
*************************************************************************************************/
void CWardWizUpdateDlg::OnBnClickedButtonNext()
{
	m_bOlderdatabase = false;
	m_bdatabaseuptodate = false;
	try
	{
		if (m_updateType == NONE)
		{
			//m_svNotificationMessageFromLocalCB.call(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_LIVE_UPDATE_OPTION"));
			CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_LIVE_UPDATE_OPTION"), NONE);
			return;
		}

		switch (m_updateType)
		{
		case UPDATEFROMINTERNT:
			m_svUpdateStatusFunctionCB = m_svLiveUpdateStatusFunctionCB;
			StartDownloading();
			m_svLiveAddUpdateTableCb = m_svAddUpdateTableCb;
			m_svLiveUpdateUpdateTableCb = m_svUpdateUpdateTableCb;
			m_svLiveRowAddCb = m_svRowAddCb;
			m_svLiveUpdateCompleteCb = m_svUpdateCompleteCb;
			m_iDBUpdateID = m_iUpdateID;
			m_bIsStopFrmTaskbar = false;
			m_bIsManualStop = false;
			m_iTotalFileSize = 0;
			m_iCurrentDownloadBytes = 0;
			m_csDownloadPercentage = L"";
			m_dwPercentage = 0;
			StartALUpdateUsingALupdateService();
			break;
		case UPDATEFROMLOCALFOLDER:
			m_hUpdateFromLocalFolderThread = NULL;
			m_hUpdateFromLocalFolderThread = ::CreateThread(NULL, 0, UpdateFromLocalFolderThread, (LPVOID) this, 0, NULL);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizUpdateDlg::OnBnClickedButtonNext", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : UpdateFromLocalFolderThread
*  Description    : update from local folder for offline update
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-5-2016
*************************************************************************************************/
DWORD WINAPI UpdateFromLocalFolderThread(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizUpdateDlg *pThis = (CWardWizUpdateDlg*)lpvThreadParam;
		if (!pThis)
			return 1;
		CString csInputFileName = pThis->m_csInputFolderPath;
		pThis->m_dwMaxVersionInZip = 0x00;
		pThis->m_bOfflineUpdateStarted = true;
		if (pThis->m_csInputFolderPath.Compare(L"") == 0)
		{
			pThis->m_csInputFolderPath = L"";
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_FOLDER_PATH"), UPDATEFROMLOCALFOLDER);
			return 0;
		}

		if (!PathFileExists(csInputFileName))
		{
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_FOLDER_NO_PATH"), UPDATEFROMLOCALFOLDER);
			return 0;
		}

		DWORD dwUnzipCount = 0;
		csInputFileName = pThis->ExtractRARFile(csInputFileName, dwUnzipCount);
		if (dwUnzipCount == 0)
		{
			pThis->m_bOfflineUpdateStarted = false;
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_INVALID_MSG1"), UPDATEFROMLOCALFOLDER);
			//break;
			return 0;
		}
		std::vector<CString> csVectInputFiles;
		if (!pThis->CheckForValidUpdatedFiles(csInputFileName, csVectInputFiles))
		{
			if (pThis->m_bOlderdatabase)
			{
				pThis->m_bOlderdatabase = false;
				pThis->EnumAndDeleteTempFolder(csInputFileName);
				pThis->m_bOfflineUpdateStarted = false;
				pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_DB_FILES_ARE_OLDER"), UPDATEFROMLOCALFOLDER);
				return 0;
			}
			else if (pThis->m_bdatabaseuptodate)
			{
				pThis->m_bdatabaseuptodate = false;
				pThis->EnumAndDeleteTempFolder(csInputFileName);
				pThis->m_bOfflineUpdateStarted = false;
				pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDC_STATIC_UPDATED_DATABASE"), UPDATEFROMLOCALFOLDER);
				return 0;
			}
			else
			{
				pThis->EnumAndDeleteTempFolder(csInputFileName);
				pThis->m_bOfflineUpdateStarted = false;
				pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_INVALID_MSG1"), UPDATEFROMLOCALFOLDER);
			}
			return 0;
		}

		int dwUpdateLocalRet = pThis->UpdateFromLocalFolder(csVectInputFiles);
		pThis->EnumAndDeleteTempFolder(csInputFileName);
		if (dwUpdateLocalRet == 0x01)
		{
			if (!pThis->UpdateVersionIntoRegistry())
			{
				AddLogEntry(L"### Failed to update database version into registry", 0, 0, true, SECONDLEVEL);
			}
			pThis->m_bisDateTime = true;
			pThis->UpdateTimeDate();

			pThis->m_bOfflineUpdateStarted = false;
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_DB_UPDATE_SUCCESS"), UPDATEFROMLOCALFOLDER);
			return 0;
		}
		if (dwUpdateLocalRet == 0x03)
		{
			if (pThis->m_bOlderdatabase)
			{
				pThis->m_bOlderdatabase = false;

				pThis->m_bOfflineUpdateStarted = false;
				pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_DB_FILES_ARE_OLDER"), UPDATEFROMLOCALFOLDER);
				return 0;
			}
			else
			{
				pThis->m_bOfflineUpdateStarted = false;
				pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE"), UPDATEFROMLOCALFOLDER);
				return 0;
			}
		}
		if (dwUpdateLocalRet == 0x02)
		{
			if (!pThis->UpdateVersionIntoRegistry())
			{
				AddLogEntry(L"### Failed to update database version into registry", 0, 0, true, SECONDLEVEL);
			}
			pThis->m_bisDateTime = true;
			pThis->UpdateTimeDate();

			pThis->m_bOfflineUpdateStarted = false;
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_SUCCESS"), UPDATEFROMLOCALFOLDER);

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UpdateFromLocalFolderThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : CallNotificationMessage
*  Description    : Set the notification in UI
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date           : 22-Jan-2019
*************************************************************************************************/
void CWardWizUpdateDlg::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString, DWORD dwFlagUpdt)
{
	try
	{
		if (dwFlagUpdt == UPDATEFROMINTERNT)
		{
			m_csCurrentDownloadedbytes = L"0";
			theApp.m_objCompleteEvent.ResetEvent();
			m_svFunNotificationMessageCB.call(iMsgType, (SCITER_STRING)strMessageString);
			::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
			Sleep(700);
			theApp.m_objCompleteEvent.ResetEvent();
		}
		else if (dwFlagUpdt == UPDATEFROMLOCALFOLDER)
		{
			m_svNotificationMessageFromLocalCB.call(iMsgType, (SCITER_STRING)strMessageString);
			::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
			Sleep(700);
			theApp.m_objCompleteEvent.ResetEvent();

			if (m_hUpdateFromLocalFolderThread != NULL)
			{
				::SuspendThread(m_hUpdateFromLocalFolderThread);
				::TerminateThread(m_hUpdateFromLocalFolderThread, 0x00);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : StartALUpdateUsingALupdateService
*  Description    : Send a request to start Autp live update to ALU service through named pipe
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::StartALUpdateUsingALupdateService()
{
	try
	{
		if (!m_hThread_StartALUpdateProcess)
		{
			m_hThread_StartALUpdateProcess = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartALUpdateProcessThread,
				(LPVOID)this, 0, 0);
			Sleep(500);

			if (m_hThread_StartALUpdateProcess == NULL)
				AddLogEntry(L"### Failed in CWardwizUpdateDlg::StartALUpdateUsingALupdateService::To create StartALUpdateProcessThread", 0, 0, true, SECONDLEVEL);
		}

		//checked here registry IS_UPDATE_MANAGER
		CITinRegWrapper objReg;
		DWORD dwUpdateManager = 0;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwUpdtManager", dwUpdateManager) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwUpdtManager", 0, 0, true, ZEROLEVEL);;
			return 0;
		}

		if (dwUpdateManager)
		{
			if (theApp.m_dwProductID == ELITE)
			{
				if (!m_hThreadStartUpdateManagerTask)
				{
					m_hThreadStartUpdateManagerTask = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateManagerTaskThread,
						(LPVOID)this, 0, 0);
					Sleep(500);

					if (m_hThreadStartUpdateManagerTask == NULL)
						AddLogEntry(L"### Failed in StartALUpdateUsingALupdateService::To create UpdateManagerTaskThread", 0, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::StartALUpdateUsingALupdateService", 0, 0, true, SECONDLEVEL);
	}


	return true;
}

/***********************************************************************************************
*  Function Name  : StartALUpdateProcessThread
*  Description    : Start live update
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date           : 23-Jan-2019
*************************************************************************************************/
DWORD WINAPI StartALUpdateProcessThread(LPVOID lpParam)
{
	CWardWizUpdateDlg *pThis = (CWardWizUpdateDlg*)lpParam;
	try
	{
		pThis->m_objWardWizUpdateManager.StartUpdate();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartALUpdateProcessThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : CloseUpdtNoUISrv
*  Description    : Close update no ui entry from Task Manager
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date           : 12-Apr-2019
*************************************************************************************************/
void CWardWizUpdateDlg::CloseUpdtNoUISrv()
{
	try
	{
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::CloseUpdtNoUISrv()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : UpdateManagerTaskThread()
*  Description    : Thread function which makes client/server machine as update manager
*  Author Name    : Amol Jaware
*  Date			  :	10/April/2018
****************************************************************************************************/
DWORD UpdateManagerTaskThread(LPVOID lpParam)
{
	CWardWizUpdateDlg *pThis = (CWardWizUpdateDlg*)lpParam;

	try
	{
		if (pThis == NULL)
			return 0;

		pThis->m_objWardWizUpdateManager.StartUpdate4UpdateManger();
		if (theApp.m_bEPSLiveUpdateNoUI)
			pThis->CloseUpdtNoUISrv();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::UpdateManagerTaskThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : UpdateFromLocalFolder
*  Description    : Offline update from local folder
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 24-Jan-2019
****************************************************************************************************/
DWORD CWardWizUpdateDlg::UpdateFromLocalFolder(std::vector<CString> &csVectInputFiles)
{
	try
	{
		return CopyFromLocalFolder2InstalledFolder(csVectInputFiles);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::UpdateFromLocalFolder", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name	 : CopyFromLocalFolder2InstalledFolder
*  Description		 : Copy from the local folder to installation folder
*  Author Name		 : Nihar Deshpande
*  Date				 : 19-May-2016
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
*  Modified Date     : 6/2/2015 Neha Gharge FP file added
****************************************************************************************************/
DWORD CWardWizUpdateDlg::CopyFromLocalFolder2InstalledFolder(std::vector<CString> &csVectInputFiles)
{
	DWORD dwReturn = 0x00;
	try
	{
		int iUpdateFileCount = 0;
		int iItemCount = static_cast<int>(csVectInputFiles.size());

		for (int iIndex = 0; iIndex < iItemCount; iIndex++)
		{
			CString csSourcePath = csVectInputFiles[iIndex];
			if (PathFileExists(csSourcePath))
			{
				CString csFileName;
				int iFound = csSourcePath.ReverseFind(L'\\');
				csFileName = csSourcePath.Right(csSourcePath.GetLength() - iFound - 1);
				CString csDestination;
				//Prajakta
				if (theApp.m_eScanLevel != WARDWIZSCANNER)
				{
					if ((csFileName == L"DAILY.CLD") || (csFileName == L"MAIN.CVD") || (csFileName == L"WRDWIZWHLST.FP"))
					{
						csDestination.Format(L"%s\\DB\\%s", theApp.GetModuleFilePath(), csFileName);
					}
					else
					{
						csDestination.Format(L"%s\\VBDB\\%s", theApp.GetModuleFilePath(), csFileName);
					}
				}
				else
				{
					if ((csFileName == L"DAILY.CLD") || (csFileName == L"MAIN.CVD") || (csFileName == L"WRDWIZWHLST.FP"))
					{
						continue;
					}
					else
					{
						csDestination.Format(L"%s\\VBDB\\%s", theApp.GetModuleFilePath(), csFileName);
					}
				}
				Sleep(10);
				if (MoveFileEx(csSourcePath, csDestination, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
				{
					iUpdateFileCount++;
				}
				else
				{
					AddLogEntry(L"### Error in CWardwizUpdateDlg:FILE_OPERATIONS in SendLiveUpdateOperation2Service", 0, 0, true, SECONDLEVEL);
				}
			}
		}

		if (iUpdateFileCount <= 3)
		{
			dwReturn = 0x01;
			goto Cleanup;
		}
		else if (iUpdateFileCount >= 4)
		{
			dwReturn = 0x02;
			goto Cleanup;
		}
		else
		{
			dwReturn = 0x03;
			goto Cleanup;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::CopyDownloadedFiles2InstalledFolder", 0, 0, true, SECONDLEVEL);
		dwReturn = 0x03;
	}
Cleanup:
	return dwReturn;
}

/***********************************************************************************************
*  Function Name  : UpdateTimeDate
*  Description    : This function update date and time into registry after completion of download
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-05-2016
*************************************************************************************************/
void CWardWizUpdateDlg::UpdateTimeDate()
{
	try
	{
		CString  csDate, csTime;
		TCHAR szOutMessage[30] = { 0 };
		TCHAR tbuffer[9] = { 0 };
		TCHAR dbuffer[9] = { 0 };
		SYSTEMTIME  CurrTime = { 0 };
		GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
		CTime Time_Curr(CurrTime);
		int iMonth = Time_Curr.GetMonth();
		int iDate = Time_Curr.GetDay();
		int iYear = Time_Curr.GetYear();

		if (m_bisDateTime)
		{
			_wstrtime_s(tbuffer, 9);
			csTime.Format(L"%s\0\r\n", tbuffer);
			csDate.Format(L"%d/%d/%d", iMonth, iDate, iYear);
			_stprintf(szOutMessage, _T("%s %s\0"), csDate, tbuffer);
		}

		if (!SendRegistryData2Service(SZ_STRING, theApp.m_csRegKeyPath.GetBuffer(),
			_T("LastLiveupdatedt"), szOutMessage, true))
		{
			AddLogEntry(L"### Failed to LastLiveupdatedt in CWardwizUpdateDlg::UpdateTimeDate", 0, 0, true, SECONDLEVEL);
		}
		Sleep(10);
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUpdateDlg::UpdateTimeDate", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : SendRegistryData2Service
*  Description    : Send request to service to set registry through pipe.
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = dwType;
		wcscpy_s(szPipeData.szFirstParam, szKey);
		wcscpy_s(szPipeData.szSecondParam, szValue);
		if (szData)
		{
			wcscpy_s(szPipeData.szThirdParam, szData);
		}
		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizUpdateDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CWardwizUpdateDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUpdateDlg::SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function Name  : Send_UpdateType
Description    : Send Update Type(Product update / only Database update)
Author Name    : Jeena Mariam Saji
SR_NO		   :
Date           : 12th Dec 2016
/***************************************************************************************************/
json::value CWardWizUpdateDlg::Send_UpdateType(SCITER_VALUE m_csUpdateType)
{
	try
	{
		m_svFunDisplayDownloadType = m_csUpdateType;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::Send_UpdateType()", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  : On_SendNotificationMessageCB
*  Description    : Show notifications messages from backend.
*  Author Name    :	Amol Jaware
*  SR_NO		  :
*  Date           : 05-08-2016
/*********************************************************************************************************/
json::value CWardWizUpdateDlg::On_SendNotificationMessageCB(SCITER_VALUE svFunNotificationMessageCB)
{
	try
	{
		m_svFunNotificationMessageCB = svFunNotificationMessageCB;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_SendNotificationMessageCB()", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  : On_PauaseOfflineUpdate
*  Description    : To pause the thread while update is on.
*  Author Name    :	Amol Jaware
*  SR_NO		  :
*  Date           : 20-09-2016
/*********************************************************************************************************/
json::value CWardWizUpdateDlg::On_PauaseOfflineUpdate()
{
	try
	{
		if (m_bOfflineUpdateStarted == true)
		{
			if (m_hUpdateFromLocalFolderThread != NULL)
			{
				::SuspendThread(m_hUpdateFromLocalFolderThread);
			}

			m_bOfflineUpdateStarted = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_PauaseOfflineUpdate()", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  : On_ResumeOfflineUpdate
*  Description    : To resume the thread while update is on.
*  Author Name    :	Amol Jaware
*  SR_NO		  :
*  Date           : 20-09-2016
/*********************************************************************************************************/
json::value CWardWizUpdateDlg::On_ResumeOfflineUpdate()
{
	try
	{
		if (m_bOfflineUpdateStarted == false)
		{
			if (m_hUpdateFromLocalFolderThread != NULL)
			{
				::ResumeThread(m_hUpdateFromLocalFolderThread);
			}

			m_bOfflineUpdateStarted = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_ResumeOfflineUpdate()", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************
*  Function Name  : On_CloseOfflineUpdate
*  Description    : To terminate the thread.
*  Author Name    :	Amol Jaware
*  SR_NO		  :
*  Date           : 20-09-2016
/*********************************************************************************************************/
json::value CWardWizUpdateDlg::On_CloseOfflineUpdate()
{
	try
	{
		if (m_hUpdateFromLocalFolderThread != NULL)
		{
			m_csInputFolderPath = L""; //.def file removed from this variable
			m_bAborted = true;

			::ResumeThread(m_hUpdateFromLocalFolderThread);

			if (!m_StopUnRarOperation)
			{
				AddLogEntry(L"### StopUnrarOperation Function Address failed in CWardwizUpdateDlg::On_CloseOfflineUpdate", 0, 0, true, SECONDLEVEL);
				goto CLEANUP;
			}

			if (!m_StopUnRarOperation())
			{
				::SuspendThread(m_hUpdateFromLocalFolderThread);
				::TerminateThread(m_hUpdateFromLocalFolderThread, 0x00);
				goto CLEANUP;
			}

			DWORD dwTimeOut = 2 * 1000;//wait for 2 second
			DWORD dwWaitResult = WaitForSingleObject(m_hUpdateFromLocalFolderThread, dwTimeOut);  // no time-out interval

			switch (dwWaitResult)
			{
				// The thread got ownership of the mutex
			case WAIT_OBJECT_0:
				//Addlog
				m_hUpdateFromLocalFolderThread = NULL;
				break;
				// The database is in an indeterminate state
			case WAIT_TIMEOUT:
				//addlog
				break;
			}

			m_bOfflineUpdateStarted = false;

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_CloseOfflineUpdate()", 0, 0, true, SECONDLEVEL);
	}

CLEANUP:
	if (m_hZip != NULL) //unload extractdll library
	{
		FreeLibrary(m_hZip);
		m_hZip = NULL;
	}
	if (m_UnzipFile != NULL)
	{
		m_UnzipFile = NULL;
	}
	if (m_StopUnRarOperation != NULL)
	{
		m_StopUnRarOperation = NULL;
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_PasueResumeUpdate
*  Description    : Accepts the request from UI and pauses and resumes the updates
*  Author Name    : Nihar Deshpande
*  Date			  : 19-05-2016
****************************************************************************************************/
json::value CWardWizUpdateDlg::On_ResumeUpdates()
{
	try
	{
		if (!theApp.m_bUpdates)
		{
			ResumeLiveUpdateThread();
			theApp.m_bUpdates = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_ResumeUpdates", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : ResumeLiveUpdateThread
*  Description    : Accepts the request from UI and resumes the update thread
*  Author Name    : Nihar Deshpande
*  Date			  : 19-05-2016
****************************************************************************************************/
bool CWardWizUpdateDlg::ResumeLiveUpdateThread()
{
	try
	{
		m_objWardWizUpdateManager.SetThreadPoolStatus(false);

		if (!m_hThread_StartALUpdateProcess)
		{
			AddLogEntry(L"### CWardwizUpdateDlg m_hThread_StartALUpdateProcess Thread is not running", 0, 0, true, ZEROLEVEL);
			return false;
		}

		if (ResumeThread(m_hThread_StartALUpdateProcess) != 0xFFFFFFFF)
		{
			AddLogEntry(L">>> StartALUpdateProcessThread m_hThread_StartALUpdateProcess resumed successfully.", 0, 0, true, ZEROLEVEL);
			return false;
		}

		if (theApp.m_dwProductID == ELITE)
		{
			if (!m_hThreadStartUpdateManagerTask)
			{
				AddLogEntry(L"### CWardwizUpdateDlg m_hThreadStartUpdateManagerTask Thread is not running", 0, 0, true, ZEROLEVEL);
				return false;
			}
			if (ResumeThread(m_hThreadStartUpdateManagerTask) != 0xFFFFFFFF)
			{
				AddLogEntry(L">>> StartALUpdateProcessThread m_hThreadStartUpdateManagerTask resumed successfully.", 0, 0, true, ZEROLEVEL);
				return false;
			}
		}

		AddLogEntry(L"### Failed to resume thread in CWardwizUpdateDlg::ResumeLiveUpdateThread.", 0, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### CWardwizUpdateDlg::ResumeLiveUpdateThread::Exception", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : On_PasueResumeUpdate
*  Description    : Accepts the request from UI and pauses and resumes the updates
*  Author Name    : Nihar Deshpande
*  Date			  : 19-05-2016
****************************************************************************************************/
json::value CWardWizUpdateDlg::On_PauseUpdates()
{
	try
	{
		if (theApp.m_bUpdates)
		{
			PauseLiveUpdate();
			theApp.m_bUpdates = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::On_PauseUpdates", 0, 0, true, SECONDLEVEL);
	}

	return json::value();
}

/***************************************************************************************************
*  Function Name  : PauseLiveUpdate
*  Description    : Request from UI and pause the updates
*  Author Name    : Amol Jaware
*  Date			  : 25-Jan-2019
****************************************************************************************************/
bool CWardWizUpdateDlg::PauseLiveUpdate()
{
	try
	{

		if (!m_hThread_StartALUpdateProcess)
		{
			AddLogEntry(L"### CWardwizUpdateDlg::PauseLiveUpdate m_hThread_StartALUpdateProcess Thread is not running", 0, 0, true, ZEROLEVEL);
			return false;
		}

		if (SuspendThread(m_hThread_StartALUpdateProcess) != 0xFFFFFFFF)
		{
			AddLogEntry(L">>> CWardwizUpdateDlg::PauseLiveUpdate m_hThread_StartALUpdateProcess suspended successfully.", 0, 0, true, ZEROLEVEL);
			return false;
		}

		if (theApp.m_dwProductID == ELITE)
		{
			if (!m_hThreadStartUpdateManagerTask)
			{
				AddLogEntry(L"### CWardwizUpdateDlg::PauseLiveUpdate m_hThreadStartUpdateManagerTask Thread is not running", 0, 0, true, ZEROLEVEL);
				return false;
			}
			if (SuspendThread(m_hThreadStartUpdateManagerTask) != 0xFFFFFFFF)
			{
				AddLogEntry(L">>> CWardwizUpdateDlg::PauseLiveUpdate m_hThreadStartUpdateManagerTask suspended successfully.", 0, 0, true, ZEROLEVEL);
				return false;
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exceptin inside CWardwizUpdateDlg::PauseLiveUpdate", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : On_ClickStopUpdates
*  Description    : It shows message whether downloading should stop or not.
*  Author Name    :	Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
****************************************************************************************************/
json::value CWardWizUpdateDlg::On_ClickStopUpdates(SCITER_VALUE svIsStopFrmTaskbar, SCITER_VALUE svbIsManualStop)
{
	try
	{
		StopUpdates(svIsStopFrmTaskbar, svbIsManualStop);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUpdateDlg::On_ClickStopUpdates", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***********************************************************************************************
*  Function Name  : ExtractRARFile
*  Description    : Function to Unrar the file for Local updates
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
CString CWardWizUpdateDlg::ExtractRARFile(CString csInputFileName, DWORD &dwUnzipCount)
{
	TCHAR szUnzipPath[512] = { 0 };
	try
	{
		if (m_UnzipFile != NULL)
		{
			if (m_UnzipFile(csInputFileName.GetBuffer(), szUnzipPath, dwUnzipCount) != 0x00)
			{
				if (!m_bAborted)
				{
					AddLogEntry(L"### m_UnzipFile Failed in CWardwizUpdateDlg::ExtractRARFile", 0, 0, true, SECONDLEVEL);
				}
				return CString(L"");
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::ExtractRARFile", 0, 0, true, SECONDLEVEL);
	}

	return CString(szUnzipPath);
}

/***********************************************************************************************
*  Function Name  : UpdateVersionIntoRegistry
*  Description    : Upadte Version from registry
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::UpdateVersionIntoRegistry()
{
	CString csVersionregValue = L"";
	csVersionregValue = m_csVersionNo;
	csVersionregValue.Trim();
	try
	{
		if (SendData2CommService(RELOAD_SIGNATURE_DATABASE, false) != 0x00)
		{
			AddLogEntry(L"### Failed to send reload Database message to service CWardwizUpdateDlg::UpdateVersionIntoRegistry", 0, 0, true, SECONDLEVEL);
		}
		if (!SendRegistryData2Service(SZ_STRING, theApp.m_csRegKeyPath.GetBuffer(),
			_T("DataBaseVersion"), (LPTSTR)csVersionregValue.GetBuffer(), true))
		{
			AddLogEntry(L"### Failed to DataBaseVersion SendRegistryData2Service CWardwizUpdateDlg::UpdateVersionIntoRegistry", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizUpdateDlg::UpdateVersionIntoRegistry", 0, 0, true, SECONDLEVEL);

	}

	return true;
}

/***********************************************************************************************
Function Name  : SendData2CommService
Description    : Send data to communication service.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 08 Apr 2015
***********************************************************************************************/
DWORD CWardWizUpdateDlg::SendData2CommService(int iMesssageInfo, bool bWait)
{
	DWORD dwRet = 0x00;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMesssageInfo;
		CISpyCommunicator objCom(SERVICE_SERVER, false);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizUpdateDlg::SendData2CommService", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
			goto FAILED;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CWardwizUpdateDlg::SendData2CommService", 0, 0, true, SECONDLEVEL);
				dwRet = 0x02;
				goto FAILED;
			}

			if (szPipeData.dwValue != 1)
			{
				dwRet = 0x03;
				goto FAILED;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::SendData2CommService", 0, 0, true, SECONDLEVEL);
		dwRet = 0x04;
	}

FAILED:
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	GetSQLiteDBFilePath
*  Description    :	Helper function to get Current working directory path
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
CString GetSQLiteDBFilePath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';
		return(CString(szModulePath));
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUpdateDlg::GetSQLiteDBFilePath", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertSQLData
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 InsertSQLData(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable in VibraniumAutoScnDlg- AutoScanner entered", 0, 0, true, ZEROLEVEL);
	try
	{
		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
		objSqlDb.Open();
		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iRowId = objSqlDb.GetLastRowId();
		objSqlDb.Close();
		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizUpdateDlg::InsertSQLData", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	UpdateUpdaterDetails
*  Description    :	Helper function, invoked to update the record inserted with appropriate update related details
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 UpdateUpdaterDetails(INT64 iUpdateID, int iFileCount, int iFileSize)
{
	CString csInsertQuery = _T("UPDATE Wardwiz_UpdatesMaster VALUES (null,");
	try
	{
		csInsertQuery.Format(_T("UPDATE Wardwiz_UpdatesMaster SET db_UpdateEndTime= datetime('now','localtime'), db_FilesDownloadCount= %d, db_DownloadFileSize = %d WHERE db_UpdateId = %d"), iFileCount, iFileSize, iUpdateID);
		CT2A ascii(csInsertQuery, CP_UTF8);
		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		if (!PathFileExists(csWardWizReportsPath))
		{
			CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			objSqlDb.Close();
		}

		INT64 iRowId = InsertSQLData(ascii.m_psz);
		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizUpdateDlg::UpdateUpdaterDetails. The query is : %s", csInsertQuery, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  : StopUpdates
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
/*********************************************************************************************************/
bool CWardWizUpdateDlg::StopUpdates(SCITER_VALUE svIsStopFrmTaskbar, SCITER_VALUE svbIsManualStop)
{
	try
	{
		m_bIsStopFrmTaskbar = svIsStopFrmTaskbar.get(false);
		m_bIsManualStop = svbIsManualStop.get(false);
		StopLiveUpdates();
		UpdateUpdaterDetails(m_iUpdateID, m_iTotalNoofFiles, m_dwCurrentLocalFileSize);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::StopUpdates", 0, 0, true, SECONDLEVEL);
	}
	
	return 0;
}

/***************************************************************************************************
*  Function Name  : StopLiveUpdates
*  Description    : Stop live update
*  Author Name    :	Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
****************************************************************************************************/
bool CWardWizUpdateDlg::StopLiveUpdates()
{
	try
	{
		ShutDownDownloadLiveupdates();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::StopLiveUpdates", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ShutDownDownload
*  Description    : It shows message whether downloading should stop or not.
*  Author Name    :	Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
****************************************************************************************************/
bool CWardWizUpdateDlg::ShutDownDownloadLiveupdates()
{
	try
	{
		DWORD dwValue = 1;

		//if ((dwValue == 2 && m_objWardWizUpdateManager.m_bRequestFromUI == false) || (dwValue == 1 && m_objWardWizUpdateManager.m_bRequestFromUI == true))
		if (dwValue == 1 && theApp.m_bRequestFromUI == true)
		{
			m_objWardWizUpdateManager.m_bUpdateFailed = true;
			CloseALUpdateProcess();
		}
		m_objWardWizUpdateManager.StopLiveUpdateThread(dwValue); //this has to be executed in case click on retry button
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::ShutDownDownloadLiveupdates", 0, 0, true, SECONDLEVEL);
	}

	return false;
}

/***********************************************************************************************
*  Function Name  : SendRequestCommon
*  Description    : Send request to auto live update services.
*  Author Name    :	Nihar Deshpande
*  Date           : 19-May-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::SendRequestCommon(int iRequest, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;
		szPipeData.dwValue = 1;
		CISpyCommunicator objCom(AUTOUPDATESRV_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to AUTOUPDATESRV_SERVER in CWardwizUpdateDlg::SendRequestCommon", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData to AUTOUPDATESRV_SERVER in CWardwizUpdateDlg::SendRequestCommon", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::SendRequestCommon", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CloseALUpdateProcess()
*  Description    : Close ALUpdateProcess thread
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0066
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void CWardWizUpdateDlg::CloseALUpdateProcess()
{	
	if (theApp.m_dwProductID == ELITE)
	{

		if (!m_hThreadStartUpdateManagerTask)
		{
			AddLogEntry(L"### Update Thread is not running", 0, 0, true, ZEROLEVEL);
			return;
		}

		if (SuspendThread(m_hThreadStartUpdateManagerTask) != 0xFFFFFFFF)
		{
			AddLogEntry(L">>> Update thread suspended successfully.", 0, 0, true, ZEROLEVEL);

			//SuspendThread( hThread_StartALUpdateProcess );

			if (TerminateThread(m_hThreadStartUpdateManagerTask, 0x00))
			{
				AddLogEntry(L">>> Update thread stopped successfully.", 0, 0, true, ZEROLEVEL);
				m_hThreadStartUpdateManagerTask = NULL; //it is required when we terminate from UI.

				m_objWardWizUpdateManager.RemoveProxySetting();
				//return;
			}
			else
				AddLogEntry(L"### CWardwizUpdateDlg::ALUpdateProcess stopped failed.", 0, 0, true, SECONDLEVEL);
		}

	}

	if (!m_hThread_StartALUpdateProcess)
	{
		AddLogEntry(L"### Update Thread is not running", 0, 0, true, ZEROLEVEL);
		return;
	}

	if (SuspendThread(m_hThread_StartALUpdateProcess) != 0xFFFFFFFF)
	{
		AddLogEntry(L">>> Update thread suspended successfully.", 0, 0, true, ZEROLEVEL);

		//SuspendThread( hThread_StartALUpdateProcess );

		if (TerminateThread(m_hThread_StartALUpdateProcess, 0x00))
		{
			AddLogEntry(L">>> Update thread stopped successfully.", 0, 0, true, ZEROLEVEL);
			m_hThread_StartALUpdateProcess = NULL; //it is required when we terminate from UI.

			m_objWardWizUpdateManager.RemoveProxySetting();
			return;
		}
		AddLogEntry(L"### CWardwizUpdateDlg::ALUpdateProcess stopped failed.", 0, 0, true, SECONDLEVEL);
	}
	
	m_objWardWizUpdateManager.SetThreadPoolStatus(true);

}

/***************************************************************************************************
*  Function Name  : LoadExtractDll()
*  Description    : Load extract dll's
*  Author Name    : Vilas, neha
****************************************************************************************************/
bool CWardWizUpdateDlg::LoadExtractDll()
{
	try
	{
		CString	csWardWizModulePath = GetModuleFilePath();
		CString	csWardWizExtractDLL = L"";
		csWardWizExtractDLL.Format(L"%s\\VBEXTRACT.DLL", csWardWizModulePath);
		if (!PathFileExists(csWardWizExtractDLL))
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_LOAD_FAILED_EXTRACT_DLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}

		m_hZip = LoadLibrary(csWardWizExtractDLL);
		if (!m_hZip)
		{
			AddLogEntry(L"### Failed to load VBEXTRACT.DLL in CWardwizUpdateDlg::LoadExtractDll", 0, 0, true, SECONDLEVEL);
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_LOAD_FAILED_EXTRACT_DLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_hZip = NULL;
			return false;
		}

		m_UnzipFile = (UNZIPFILE)GetProcAddress(m_hZip, "UnRarForUpdates");
		if (!m_UnzipFile)
		{
			AddLogEntry(L"### GetProcAddress UnRarForUpdates failed in CWardwizUpdateDlg::LoadExtractDll", 0, 0, true, SECONDLEVEL);
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FUN_UNZIPFILE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_hZip = NULL;
			return false;
		}

		m_StopUnRarOperation = (STOPUNRAROPERATION)GetProcAddress(m_hZip, "StopUnrarOperation");
		if (!m_StopUnRarOperation)
		{
			AddLogEntry(L"### GetProcAddress StopUnrarOperation failed in CWardwizUpdateDlg::LoadExtractDll", 0, 0, true, SECONDLEVEL);
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FUN_UNZIPFILE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_hZip = NULL;
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::LoadExtractDll", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 11 May,2015
****************************************************************************************************/
CString CWardWizUpdateDlg::GetModuleFilePath()
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
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::GetModuleFilePath", 0, 0, true, SECONDLEVEL);
	}

	return(CString(szModulePath));
}

/***********************************************************************************************
*  Function Name  : EnumAndDeleteTempFolder
*  Description    : Function to delete the db files from temp location
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-05-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::EnumAndDeleteTempFolder(CString csInputFileName)
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
				EnumAndDeleteTempFolder(str);
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
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::EnumAndDeleteTempFolder", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
*  Function Name  : CheckForValidUpdatedFiles
*  Description    : Checks files in folder are valid or not
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::CheckForValidUpdatedFiles(CString csInputFolder, std::vector<CString> &csVectInputFiles)
{
	bool bReturn = false;
	int iMatchCount = 0;
	int iMisMatchedCount = 0;

	TCHAR szDataBaseFolder[MAX_PATH] = { 0 };
	try
	{
		if (theApp.m_eScanLevel == CLAMSCANNER)
		{
			_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"DB");
			if (!PathFileExists(szDataBaseFolder))
			{
				if (CreateDirectoryFocefully(szDataBaseFolder))
				{
					AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
				}
			}

			_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"VBDB");
			if (!PathFileExists(szDataBaseFolder))
			{
				if (CreateDirectoryFocefully(szDataBaseFolder))
				{
					AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
				}
			}
		}
		else
		{
			_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"VBDB");
			if (!PathFileExists(szDataBaseFolder))
			{
				if (CreateDirectoryFocefully(szDataBaseFolder))
				{
					AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
				}
			}
		}

		if (m_csFilesList.GetCount() > 0)
		{
			m_csFilesList.RemoveAll();
		}

		EnumFolder(csInputFolder);

		for (int iIndex = 0; iIndex < m_csFilesList.GetCount(); iIndex++)
		{
			CString csInputPathProgramData = NULL;
			CString csInputPath = csInputFolder + L"\\" + m_csFilesList[iIndex];
			if (PathFileExists(csInputPath))
			{
				DWORD	dwStatus = 0x00;
				dwStatus = ValidateDB_File((LPTSTR)csInputPath.GetBuffer(), dwStatus, csInputPathProgramData);
				if (dwStatus == 0x08)
				{
					iMisMatchedCount++;
				}
				if (dwStatus == 0x00)
				{
					csInputPath = csInputPathProgramData;
					csVectInputFiles.push_back(csInputPath);
					iMatchCount++;
				}
			}
		}

		//UpdateUpdaterDetails(m_iUpdateID,(int) m_csFilesList.GetCount(), m_dwCurrentLocalFileSize);
		if (iMatchCount >= 1)
		{
			bReturn = true;
		}
		else
		{
			bReturn = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizUpdateDlg::CheckForValidUpdatedFiles", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CreateDirectoryFocefully()
*  Description    : It will check directory. If not present, it will create that directory.
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date			  :	19-05-2016
****************************************************************************************************/
bool CWardWizUpdateDlg::CreateDirectoryFocefully(LPTSTR lpszPath)
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
		AddLogEntry(L"### Failed to create directory in CWardwizUpdateDlg::CreateDirectoryFocefully", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***********************************************************************************************
*  Function Name  : EnumFolder
*  Description    : Enumrate all the files in folder.
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
void CWardWizUpdateDlg::EnumFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		int i = 0;
		m_iTotalNoofFiles = 0;
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
				continue;
			}
			else
			{
				m_csFilesList.Add(finder.GetFileName());
				m_iTotalNoofFiles++;
			}
		}
		finder.Close();
		i = 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : ValidateDB_File
*  Description    : checking signature and valid version no and size of files
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*  Modifiled date :	6/6/2014 neha gharge
//Modified:
//Issue No 107	Doesn’t check the contents of the DB file. Even if DB contains garbage its accepted
//ITin_Developement_Release_1.0.0.3_Patches
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
*  Modified Date     : 6/2/2015 Neha Gharge FP file added
*************************************************************************************************/
DWORD CWardWizUpdateDlg::ValidateDB_File(TCHAR *m_szFilePath, DWORD &dwStatus, CString &csInputPathProgramData)
{
	DWORD	dwRet = 0x00;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00, dwDecBytesRead = 0x00;
	TCHAR	szTemp[1024] = { 0 }, szFileName[1024] = { 0 };
	TCHAR	szExt[16] = { 0 };
	DWORD	dwLen = 0x00;
	LPBYTE	lpFileData = NULL;
	LPBYTE	lpEncryptionSignature = (LPBYTE)"WRDWIZDB";
	DWORD	dwDecKeySize = 0x00;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE;
	TCHAR   szWholeSignature[0x30] = { 0 };
	DWORD   dwRetCheckversion = 0x00;
	CString csFileName, csActualWRDWIZFilePath, csActualClamFilePath;
	int j = 0;

	try
	{
		if (!m_szFilePath)
			return 0x05;
		m_CurrentFilePath = L"";
		m_CurrentFilePath.Format(L"%s", m_szFilePath);
		csFileName = m_CurrentFilePath.Mid(m_CurrentFilePath.ReverseFind('\\') + 1);
		if (theApp.m_eScanLevel == CLAMSCANNER)
		{
			if ((csFileName.CompareNoCase(L"DAILY.CLD") == 0) || csFileName.CompareNoCase(L"MAIN.CVD") == 0 || csFileName.CompareNoCase(L"WRDWIZWHLST.FP") == 0)
			{
				csActualClamFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"DB", csFileName);
				if (!PathFileExists(csActualClamFilePath))
				{
					AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
					csInputPathProgramData = m_szFilePath;
					dwRet = 0x00;
					goto Cleanup;
				}
			}
			else
			{
				csActualWRDWIZFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"VBDB", csFileName);
				if (!PathFileExists(csActualWRDWIZFilePath))
				{
					AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
					csInputPathProgramData = m_szFilePath;
					dwRet = 0x00;
					goto Cleanup;
				}
			}
		}
		else
		{
			if ((csFileName.CompareNoCase(L"DAILY.CLD") == 0) || csFileName.CompareNoCase(L"MAIN.CVD") == 0 || csFileName.CompareNoCase(L"WRDWIZWHLST.FP") == 0)
			{
				dwRet = 0x08;
				goto Cleanup;
			}
			csActualWRDWIZFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"VBDB", csFileName);
			if (!PathFileExists(csActualWRDWIZFilePath))
			{
				AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
				csInputPathProgramData = m_szFilePath;
				dwRet = 0x00;
				goto Cleanup;
			}
		}
		if (!PathFileExists(m_szFilePath))
		{
			dwRet = 0x01;
			goto Cleanup;
		}
		dwLen = static_cast<DWORD>(wcslen(m_szFilePath));
		if ((dwLen < 0x08) || (dwLen > 0x400))
		{
			dwRet = 0x02;
			goto Cleanup;
		}
		DWORD dwDBVersionLength = 0;
		DWORD dwDBMajorVersion = 0;
		CWrdwizEncDecManager objWrdwizEncDecMgr;
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(m_szFilePath, dwDBVersionLength, dwDBMajorVersion))
		{
			AddLogEntry(L"### Wardwiz Signature criteria not matched");
			dwRet = 0x08;		//Invalid files
			goto Cleanup;
		}
		dwDecKeySize = MAX_SIG_SIZE + dwDBVersionLength + MAX_TOKENSTRINGLEN;
		hFile = CreateFile(m_szFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### Updates : Error in opening existing Database file %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup;
		}
		dwFileSize = GetFileSize(hFile, NULL);
		if (!dwFileSize)
		{
			AddLogEntry(L"### Updates : Error in GetFileSize of %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x03;
			goto Cleanup;
		}
		m_dwCurrentLocalFileSize = dwFileSize;
		if (lpFileData != NULL)
		{
			free(lpFileData);
			lpFileData = NULL;
		}
		dwBytesRead = 0x00;
		unsigned char bySigBuff[0x30] = { 0x00 };
		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
		ReadFile(hFile, &bySigBuff[0x0], 0x30, &dwBytesRead, NULL);
		if (dwBytesRead != 0x30)
		{
			AddLogEntry(L"### Updates : Error in ReadFile while reading signature %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
		char szFileName[0x11] = { 0 };
		memcpy(szFileName, &bySigBuff[0], 0x0A);
		TCHAR szDBFileName[0x11] = { 0 };
		//convert multibyte string to unicode
		size_t convertedChars;
		mbstowcs_s(&convertedChars, szDBFileName, strlen(szFileName) + 1, szFileName, _TRUNCATE);
		char *szVersionNumber = new char[dwDBVersionLength + 1];
		memset(szVersionNumber, 0, dwDBVersionLength + 1);
		memcpy(szVersionNumber, &bySigBuff[0x0B], dwDBVersionLength);
		//szVersionNumber[strlen(szVersionNumber) + 1] = '\0';
		TCHAR * szDBVersionNumber = new TCHAR[dwDBVersionLength + 1];
		mbstowcs_s(&convertedChars, szDBVersionNumber, strlen(szVersionNumber) + 1, szVersionNumber, _TRUNCATE);
		if (!ValidateFileNVersion(szDBFileName, szDBVersionNumber, dwDBVersionLength))
		{
			AddLogEntry(L"### Updates : Invalidate File", 0, 0, true, SECONDLEVEL);
			dwRet = 0x08;
			goto Cleanup;
		}
		if (szVersionNumber != NULL)
		{
			delete[]szVersionNumber;
			szVersionNumber = NULL;
		}
		if (szDBVersionNumber != NULL)
		{
			delete[]szDBVersionNumber;
			szDBVersionNumber = NULL;
		}
		lpFileData = (LPBYTE)malloc(dwFileSize - dwDecKeySize);
		if (!lpFileData)
		{
			AddLogEntry(L"### Updates : Error in allocation of memory", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
		memset(lpFileData, 0x00, (dwFileSize - dwDecKeySize));
		SetFilePointer(hFile, (0x00 + dwDecKeySize), NULL, FILE_BEGIN);
		ReadFile(hFile, lpFileData, (dwFileSize - dwDecKeySize), &dwBytesRead, NULL);
		if ((dwFileSize - dwDecKeySize) != dwBytesRead)
		{
			AddLogEntry(L"### Updates : Error in ReadFile %s", m_szFilePath, 0, true, FIRSTLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
		CString csScanLogFullPath;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		memset(szModulePath, 0x00, MAX_PATH * sizeof(TCHAR));
		GetEnvironmentVariable(L"ALLUSERSPROFILE", szModulePath, MAX_PATH);
		csScanLogFullPath = szModulePath;
		csScanLogFullPath += L"\\Wardwiz Antivirus";
		CString FileName = L"";
		csInputPathProgramData = m_szFilePath;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::Database_File_Updation", 0, 0, true, SECONDLEVEL);
		return false;
	}
Cleanup:

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	if (lpFileData)
		free(lpFileData);
	lpFileData = NULL;

	return dwRet;
}

/***************************************************************************************************
*  Function Name  :   GetAppFolderPath
*  Description    :   Get App folder path.
*  Author Name    :   Nihar Deshpande
*  SR_NO		  :
*  Date           :   19-05-2016
****************************************************************************************************/
CString CWardWizUpdateDlg::GetAppFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csAppFolderPath = szModulePath;
		return csAppFolderPath;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::GetAppFolderPath()", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***********************************************************************************************
*  Function Name  : ValidateFileNVersion
*  Description    : checks for validate signature
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::ValidateFileNVersion(CString csFileName, CString csVersion, DWORD dwDBVersionLength)
{
	bool bReturn = false;
	m_csFileName = csFileName;
	try
	{
		int iRet = CheckForValidVersion(csVersion);
		if (iRet == 0x07)
		{
			m_bdatabaseuptodate = true;
			return false;
		}

		if (iRet == 0x08)
		{
			if (CheckForMaxVersionInZip(csVersion))
			{
				m_csVersionNo = csVersion;
			}
			return true;
		}

		if (iRet == 0x09)
		{
			m_bdatabaseuptodate = true;
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::ValidateFileNVersion", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***********************************************************************************************
*  Function Name  : CheckForValidVersion
*  Description    : checking  valid version no
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-05-2016
*************************************************************************************************/
DWORD CWardWizUpdateDlg::CheckForValidVersion(CString csVersionNo)
{
	TCHAR szVersionNo[16] = { 0 };
	TCHAR szVersion[5] = { 0 };
	DWORD dwVersionNoInRegistry = 0;
	DWORD dwVersionNo = 0;
	DWORD dwRet = 0x00;
	int j = 0;
	try
	{
		_tcscpy_s(szVersionNo, csVersionNo);
		for (int i = 0; i < 9; i++)
		{
			if (isdigit(szVersionNo[i]))
			{
				szVersion[j] = szVersionNo[i];
				j++;
			}
		}
		dwVersionNo = static_cast<DWORD>(wcstod(szVersion, _T('\0')));
		dwVersionNoInRegistry = ReadDBVersionFromReg();
		if (dwVersionNoInRegistry > dwVersionNo)
		{
			dwRet = 0x07;
			goto Cleanup;
		}
		if (dwVersionNoInRegistry < dwVersionNo)
		{
			dwRet = 0x08;
			goto Cleanup;
		}
		if (dwVersionNoInRegistry == dwVersionNo)
		{
			dwRet = 0x09;
			goto Cleanup;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::CheckForValidVersion", 0, 0, true, SECONDLEVEL);
	}

Cleanup:
	return dwRet;
}

/***********************************************************************************************
*  Function Name  : ReadDBVersionFromReg
*  Description    : ReadVersion from registry
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
DWORD CWardWizUpdateDlg::ReadDBVersionFromReg()
{
	DWORD dwRegVersionNo = 0x00;
	TCHAR szRegVersionNo[1024] = { 0 };
	TCHAR szRegVersion[10] = { 0 };
	DWORD dwvalue_length = 1024;
	DWORD dwtype = REG_SZ;
	HKEY key;
	int j = 0;
	try
	{
		if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, &key) != ERROR_SUCCESS)
		{
			//AddLogEntry(L"Unable to open registry key");
		}

		long ReadReg = RegQueryValueEx(key, L"DataBaseVersion", NULL, &dwtype, (LPBYTE)&szRegVersionNo, &dwvalue_length);

		if (ReadReg == ERROR_SUCCESS)
		{
			for (int i = 0; i < 9; i++)
			{
				if (isdigit(szRegVersionNo[i]))
				{
					szRegVersion[j] = szRegVersionNo[i];
					j++;
				}
			}
		}
		dwRegVersionNo = static_cast<DWORD>(wcstod(szRegVersion, _T('\0')));
		RegCloseKey(key);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizUpdateDlg::ReadDBVersionFromReg", 0, 0, true, SECONDLEVEL);

	}

	return dwRegVersionNo;
}

/***********************************************************************************************
*  Function Name  : CheckForMaxVersionInZip
*  Description    :
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::CheckForMaxVersionInZip(CString csVersionNo)
{
	try
	{
		TCHAR szVersionNo[10] = { 0 };
		DWORD dwVersionNo = 0;
		TCHAR szVersion[5] = { 0 };
		int j = 0;

		_tcscpy_s(szVersionNo, csVersionNo);

		for (int i = 0; i < 9; i++)
		{
			if (isdigit(szVersionNo[i]))
			{
				szVersion[j] = szVersionNo[i];
				j++;
			}
		}
		dwVersionNo = static_cast<DWORD>(wcstod(szVersion, _T('\0')));
		if (m_dwMaxVersionInZip < dwVersionNo)
		{
			m_dwMaxVersionInZip = dwVersionNo;
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::CheckForMaxVersionInZip", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/**********************************************************************************************************
*  Function Name  :	EnterUpdateDetails
*  Description    :	Helper function, invoked to INSERT the record appropriate update related details
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 EnterUpdateDetails(int iFileCount, int iFileSize)
{
	try
	{
		CString csInsertQuery = _T("INSERT INTO Wardwiz_UpdatesMaster VALUES (null,");
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_UpdatesMaster VALUES(null,datetime('now','localtime'), datetime('now','localtime'),date('now'),date('now'),%d,%d);"), iFileCount, iFileSize);

		CT2A ascii(csInsertQuery, CP_UTF8);

		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);

		if (!PathFileExists(csWardWizReportsPath))
		{
			CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			objSqlDb.Close();
		}

		INT64 iRowId = InsertSQLData(ascii.m_psz);
		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizUpdateDlg::EnterUpdateDetails", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***********************************************************************************************
*  Function Name  : StartDownloading
*  Description    : Function which intialize variables which starts the download.
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
bool CWardWizUpdateDlg::StartDownloading()
{
	bool bReturn = false;
	theApp.m_bUpdates = true;
	m_iFileCount = 0;
	m_iIntFilesSize = 0;
	m_iDBUpdateID = 0;
	try
	{
		ZeroMemory(m_szAllUserPath, sizeof(m_szAllUserPath));
		GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, 511);

		g_csPreviousListControlStatus = L"";
		m_bIsCheckingForUpdateEntryExistInList = true;
		m_iCount = 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::StartDownloading", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	InsertUpdaterData
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
*  SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 InsertUpdaterData(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable in CWardwizUpdateDlg- InsertUpdaterData entered", 0, 0, true, ZEROLEVEL);
	try
	{
		g_csWardWizModulePath = GetSQLiteDBFileSource();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", g_csWardWizModulePath);

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iRowId = objSqlDb.GetLastRowId();
		objSqlDb.Close();

		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value  CWardwizUpdateDlg::InsertUpdaterData", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	UpdateUpdaterDBDetails
*  Description    :	Function to update Update module details into database for specific record.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 UpdateUpdaterDBDetails(int iFileCount, int iFileSize, INT64 iUpdateID)
{
	CString csInsertQuery = _T("UPDATE Wardwiz_UpdatesMaster VALUES (null,");
	try
	{
		g_csWardWizModulePath = GetSQLiteDBFileSource();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", g_csWardWizModulePath);

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		if (!PathFileExistsA(dbPath.m_psz))
		{
			CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			objSqlDb.Close();
		}

		csInsertQuery.Format(_T("UPDATE Wardwiz_UpdatesMaster SET db_UpdateEndTime= datetime('now','localtime'), db_FilesDownloadCount= %d, db_DownloadFileSize = %d WHERE db_UpdateId = %d"), iFileCount, iFileSize, iUpdateID);

		CT2A ascii(csInsertQuery, CP_UTF8);
		INT64 iRowId = InsertUpdaterData(ascii.m_psz);
		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value  CWardwizUpdateDlg::UpdateUpdaterDBDetails. Query is : %s", csInsertQuery, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : StartDownLoadThread
*  Description    : This thread will show the status and percentage from ALU service to UI
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
*  Modified date  :	 19-May-2016 (Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateDlg::UpDateDowloadStatus(DWORD dwFlag, DWORD dwPercent, DWORD dwTotalFileSize, DWORD dwCurrentDownloadedbytes, void* param)
{
	DWORD dwFileSizeInKB = 0, dwMessageType;
	m_csTotalFileSize = L"";
	CString	csListControlStatus = L"";
	m_iRowCount = 0;
	CString l_csPercentage = L"";
	CITinRegWrapper objReg;
	DWORD dwProductUpdate = 0;
	CString csDisplaySize = _T("KB");
	int iSize = 0;
	int iTotalSize = 0;
	try
	{
		/*if (!lpSpyData)
			return false;*/

		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwAutoProductUpdate", dwProductUpdate) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwAutoDefUpdate in CWardwizUpdateDlg::UpDateDowloadStatus", 0, 0, true, SECONDLEVEL);;
		}
		if (dwProductUpdate == 1)
		{
			m_csUpdateType.Format(L"1");		//Product Update is ON in Settings
		}
		else
		{
			m_csUpdateType.Format(L"0");		//Product Update is OFF in Settings
		}

		m_svFunDisplayDownloadType.call((SCITER_STRING)m_csUpdateType);

		if (m_bIsCheckingForUpdateEntryExistInList)
		{
			InsertItem(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"), theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"));
		}
		switch (dwFlag)
		{
		case SETTOTALFILESIZE:
			m_iTotalFileSize = dwTotalFileSize;
			dwFileSizeInKB = static_cast<DWORD>(m_iTotalFileSize / 1024);
			iSize = 0;
			iSize = ConvertBytes2KB(m_iTotalFileSize);

			if (iSize > 1024)
			{
				m_csDispTotSizeExt = _T("MB");
				iSize = ConvertBytes2MB(m_iTotalFileSize);
			}
			if (iSize > 1024)
			{
				m_csDispTotSizeExt = _T("GB");
				iSize = ConvertBytes2GB(m_iTotalFileSize);
			}
			m_iTotalFileSizeCount = iSize;
			m_csTotalFileSize.Format(L"%d %s", iSize, m_csDispTotSizeExt);
			m_iIntFilesSize = (int)dwFileSizeInKB;
			m_svUpdateStatusFunctionCB.call((SCITER_STRING)m_csDownloadPercentage, (SCITER_STRING)m_csTotalFileSize, (SCITER_STRING)m_csCurrentDownloadedbytes);
			break;

		case SETDOWNLOADPERCENTAGE:
			m_iCurrentDownloadBytes = dwTotalFileSize;
			m_dwPercentage = dwCurrentDownloadedbytes;
			m_csDownloadPercentage.Format(L"%d %s", m_dwPercentage, L"%");
			iSize = 0;
			iSize = ConvertBytes2KB(m_iCurrentDownloadBytes);

			if (iSize > 1024)
			{
				csDisplaySize = _T("MB");
				iSize = ConvertBytes2MB(m_iCurrentDownloadBytes);
			}
			if (iSize > 1024)
			{
				csDisplaySize = _T("GB");
				iSize = ConvertBytes2GB(m_iCurrentDownloadBytes);
			}

			m_csCurrentDownloadedbytes.Format(L"[%d %s]", static_cast<DWORD>(iSize), csDisplaySize);

			dwFileSizeInKB = static_cast<DWORD>(m_iTotalFileSize / 1024);
			m_csTotalFileSize.Format(L"%d %s", m_iTotalFileSizeCount, m_csDispTotSizeExt);
			m_svUpdateStatusFunctionCB.call((SCITER_STRING)m_csDownloadPercentage, (SCITER_STRING)m_csTotalFileSize, (SCITER_STRING)m_csCurrentDownloadedbytes);
			break;

		case SETMESSAGE:
			if (dwPercent == DOWNLOADINGFILES)
				m_csListControlStatus.Format(L"%s", L"Downloading files");
			else if (dwPercent == UPDATINGFILES)
				m_csListControlStatus.Format(L"%s", L"Updating files");

			if (m_csListControlStatus.CompareNoCase(L"Downloading files") == 0)
			{
				csListControlStatus.Format(L"%s: %d/%d", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DOWNLOADING"), dwTotalFileSize, dwCurrentDownloadedbytes);
				m_iFileCount = (int)dwCurrentDownloadedbytes;
			}
			else if (m_csListControlStatus.CompareNoCase(L"Updating files") == 0)
			{
				m_svLiveRowAddCb.call();
				csListControlStatus.Format(L"%s: %d/%d", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_UPDATING"), dwTotalFileSize, dwCurrentDownloadedbytes);
			}
			if (csListControlStatus.GetLength() > 0 && (g_csPreviousListControlStatus != csListControlStatus))
			{
				OutputDebugString(csListControlStatus);
				InsertItem(csListControlStatus, m_csListControlStatus);
			}
			g_csPreviousListControlStatus.SetString(csListControlStatus);
			break;

		case SETUPDATESTATUS:
			dwMessageType = dwTotalFileSize;
			switch (dwMessageType)
			{
			case ALUPDATEDSUCCESSFULLY:
				m_dwstatusMsg = ALUPDATEDSUCCESSFULLY;
				m_svLiveUpdateCompleteCb.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED"));
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_INTERNETCONNECTION:
				m_dwstatusMsg = ALUPDATEFAILED_INTERNETCONNECTION;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_DOWNLOADINIPARSE:
				m_dwstatusMsg = ALUPDATEFAILED_DOWNLOADINIPARSE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATED_UPTODATE:
				m_dwstatusMsg = ALUPDATED_UPTODATE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_DOWNLOADFILE:
				m_dwstatusMsg = ALUPDATEFAILED_DOWNLOADFILE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_EXTRACTFILE:
				m_dwstatusMsg = ALUPDATEFAILED_EXTRACTFILE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_UPDATINGFILE:
				m_dwstatusMsg = ALUPDATEFAILED_UPDATINGFILE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				break;
			case ALUPDATEFAILED_LOWDISKSPACE:
				m_dwstatusMsg = ALUPDATEFAILED_LOWDISKSPACE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			}
			break;
		}
		csListControlStatus.ReleaseBuffer();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::UpDateDowloadStatus", 0, 0, true, SECONDLEVEL);
	}

	return 1;
}

/***********************************************************************************************
*  Function Name  : InsertItem
*  Description    : Insert item In tables , An new parameter added m_idivsion , to keep track of
rows
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
*************************************************************************************************/
void CWardWizUpdateDlg::InsertItem(CString csInsertItem, CString csActualStatus)
{
	try
	{
		if (csActualStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"))
		{
			if (m_bIsCheckingForUpdateEntryExistInList)
			{
				OnAddUpdateStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"));
				m_bIsCheckingForUpdateEntryExistInList = false;
			}
		}
		else if (csActualStatus == L"Already downloaded")
		{
			m_bISalreadyDownloadAvailable = true;
			OnAddUpdateStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_ALREADY_DOWNLOADED"));
		}
		else if (csActualStatus == L"Downloading files")
		{
			m_bISalreadyDownloadAvailable = false;
			OnAddUpdateStatus(csInsertItem);
		}
		else if (csActualStatus == L"Updating files")
		{
			OnAddUpdateStatus(csInsertItem);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::InsertItem", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : OnAddUpdateStatus
*  Description    : On adding the status into list control
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
*************************************************************************************************/
void CWardWizUpdateDlg::OnAddUpdateStatus(CString csStatus)
{
	try
	{
		if (m_iCount > 0)
		{
			m_svLiveUpdateUpdateTableCb.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED"), (SCITER_STRING)csStatus, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"));
			m_iRowCount++;
			m_iCount++;
		}
		else
		{
			m_svLiveAddUpdateTableCb.call((SCITER_STRING)csStatus, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"));
			m_iRowCount++;
			m_iCount++;
		}
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::OnAddUpdateStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : ConvertBytes2KB
Description    : converting bytes to KB
Author Name    : Amol J.
SR_NO		   :
Date           : 14th Dec 2017
/***************************************************************************************************/
int CWardWizUpdateDlg::ConvertBytes2KB(int iCurrentDownloadBytes)
{
	try
	{
		return (iCurrentDownloadBytes / 1024);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::ConvertBytes2KB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : ConvertBytes2MB
Description    : converting KB to MB
Author Name    : Amol J.
SR_NO		   :
Date           : 14th Dec 2017
/***************************************************************************************************/
int CWardWizUpdateDlg::ConvertBytes2MB(int iCurrentDownloadKB)
{
	try
	{
		return (iCurrentDownloadKB / 1048576);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::ConvertBytes2MB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : ConvertBytes2GB
Description    : converting MB to GB
Author Name    : Amol J.
SR_NO		   :
Date           : 14th Dec 2017
/***************************************************************************************************/
int CWardWizUpdateDlg::ConvertBytes2GB(int iCurrentDownloadMB)
{
	try
	{
		return (iCurrentDownloadMB / 1073741824);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::ConvertBytes2GB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetSQLiteDBFilePath
*  Description    :	Helper function to get Current working directory path
*  Author Name    : Gayatri A.
*  Date           : 12 Dec 2016
*  SR_NO		  :
**********************************************************************************************************/
CString GetSQLiteDBFileSource()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szModulePath, MAX_PATH);

		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		return(CString(szModulePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUpdateDlg::GetSQLiteDBFileSource", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name	 : ShowUpdateCompleteMessage
*  Description		 : It shows the message after completing a download
*  Author Name	     : Nihar Deshpande
*  SR_NO		     :
*  Date				 : 19-05-2016
*  Modification Date : 25-Jul-2014
*  Modified Date	 : Neha Gharge 5/5/2015 for new status message for failures and succesful cases.
****************************************************************************************************/
void CWardWizUpdateDlg::ShowUpdateCompleteMessage()
{
	bool bShowRedirectUI = false;
	DWORD dwCount = 0;
	TCHAR szIniFilePath[512] = { 0 };
	try
	{
 		if (!m_bIsStopFrmTaskbar)
		{
			if (m_bIsManualStop)
			{
				CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPDATE_ABORTED"), UPDATEFROMINTERNT);
			}
			else
			{
				if (m_dwstatusMsg == ALUPDATEDSUCCESSFULLY)
				{
					//bool bmessgeBoxResponce = false;

					swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", m_szAllUserPath);

					if (!PathFileExists(szIniFilePath))
					{
						AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
						dwCount = 0;
					}
					else
					{
						dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
					}

					if (dwCount <= 0)
					{
						swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", m_szAllUserPath);

						if (!PathFileExists(szIniFilePath))
						{
							AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
							dwCount = 0;
						}
						else
						{
							dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
						}
					}


					if (!m_bISalreadyDownloadAvailable || dwCount > 0)
					{
						bShowRedirectUI = true;
						if (dwCount > 0)
							CallNotificationMessage(5, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPDATED_SUCCESS"), UPDATEFROMINTERNT); //for restart
						else
							CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPDATED_SUCCESS"), UPDATEFROMINTERNT);
						/*if(theApp.m_bRetval == true) 
						{
						bmessgeBoxResponce = true;
						}*/
					}
					else
					{
						dwCount = 0;
						CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPTODATE"), UPDATEFROMINTERNT);
					}

					//if (bmessgeBoxResponce)
					//{
					/*swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", m_szAllUserPath);

					if (!PathFileExists(szIniFilePath))
					{
					AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
					dwCount = 0;
					}
					else
					{
					dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
					}
					if (dwCount <= 0)
					{
					swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", m_szAllUserPath);

					if (!PathFileExists(szIniFilePath))
					{
					AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
					dwCount = 0;
					}
					else
					{
					dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
					}
					}*/
					if (dwCount > 0)
					{
						/*CallNotificationMessage(2, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_FOR_UPDATE"));
						if (theApp.m_bRetval == true)
						{
						CEnumProcess enumproc;
						enumproc.RebootSystem(0);
						}
						else
						{
						UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
						}*/
						UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
					}
					//}
				}
				else if (m_dwstatusMsg == ALUPDATED_UPTODATE)
				{
					CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPTODATE"), UPDATEFROMINTERNT);
				}
				else if (m_dwstatusMsg == ALUPDATEFAILED_INTERNETCONNECTION)
				{
					CallNotificationMessage(3, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_MSG_1"), UPDATEFROMINTERNT);
				}
				else if (m_dwstatusMsg == ALUPDATEFAILED_DOWNLOADINIPARSE || m_dwstatusMsg == ALUPDATEFAILED_DOWNLOADFILE || m_dwstatusMsg == ALUPDATEFAILED_EXTRACTFILE || m_dwstatusMsg == ALUPDATEFAILED_UPDATINGFILE)
				{
					CString csText = L"";
					csText.Format(L"%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_MSG_1"), theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_CONTACT_SUPPORT"));
					CallNotificationMessage(3, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_MSG_1"), UPDATEFROMINTERNT);
				}
				else if (m_dwstatusMsg == ALUPDATEFAILED_LOWDISKSPACE)
				{
					CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPTODATE"), UPDATEFROMINTERNT);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::ShowUpdateCompleteMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : UpdateOpProcess
Description    : call back function pointer given to set download status on UI.
Author Name    : Amol Jaware
SR_NO		   : 
Date           : 1st Feb 2019
****************************************************************************/
void CWardWizUpdateDlg::UpdateOpProcess(DWORD dwFlag, DWORD dwPercent, DWORD dwTotalFileSize, DWORD dwCurrentDownloadedbytes, void * param)
{
	try
	{
		if (param == NULL)
		{
			return;
		}

		switch (dwFlag)
		{
		case SETTOTALFILESIZE:
		case SETUPDATESTATUS:
		case SETMESSAGE:
		case SETDOWNLOADPERCENTAGE:
			g_objWardWizUpdateDlg->UpDateDowloadStatus(dwFlag, dwPercent, dwTotalFileSize, dwCurrentDownloadedbytes, param);
			break;
		case UPDATE_FINISHED:
			g_objWardWizUpdateDlg->ShowUpdateCompleteMessage();
			break;

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::UpdateOpProcess", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : onModalLoop
Description    : Handle sciter level events.
Author Name    : Amol Jaware
SR_NO		   :
Date           : 1st Feb 2019
****************************************************************************/
json::value CWardWizUpdateDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			m_bRetval = svDialogBoolVal.get(false);
			m_iRetval = svDialogIntVal;

			theApp.m_objCompleteEvent.SetEvent();
			Sleep(200);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/****************************************************************************************************
*	Function Name : StartEPSLiveUpdateNOUI
*	Description	  : Function to open Live Update page with No UI.
*	Author Name   :	Amol Jaware
*	Date		  :	25/Feb/2019
*****************************************************************************************************/
void CWardWizUpdateDlg::StartEPSLiveUpdateNOUI()
{
	try
	{
		m_hThread_StartALUpdateProcess = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartALUpdateProcessThread, (LPVOID)this, 0, 0);
		if (m_hThread_StartALUpdateProcess)
			WaitForSingleObject(m_hThread_StartALUpdateProcess, INFINITE);

		exit(0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::StartEPSLiveUpdateNOUI", 0, 0, true, SECONDLEVEL);
	}
}

/****************************************************************************************************
*	Function Name : ShowLiveUpdate
*	Description	  : Function to open Live Update page.
*	Author Name   :	Amol Jaware
*	Date		  :	13/Feb/2019
*****************************************************************************************************/
void CWardWizUpdateDlg::ShowLiveUpdate()
{
	try
	{
		const SCITER_VALUE result = m_root_el.call_function("CallLiveUpdate");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::ShowLiveUpdate", 0, 0, true, SECONDLEVEL);
	}
}

/****************************************************************************************************
*	Function Name : TheadTerminationState
*	Description	  : To terminate thread
*	Author Name   :	Nitin Shelar
*	Date		  :	15/10/2019
*****************************************************************************************************/
void CWardWizUpdateDlg::TheadTerminationState(HANDLE hThread)
{
	try
	{
		if (hThread != NULL && hThread != INVALID_HANDLE_VALUE)
		{
			SuspendThread(hThread);
			TerminateThread(hThread, 0x00);
			hThread = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::TheadTerminationState", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : FunCheckInternetAccessBlock
*  Description    : To check internet access block
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 Dec 2019
****************************************************************************************************/
json::value CWardWizUpdateDlg::FunCheckInternetAccessBlock()
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
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CWardwizUpdateDlg::FunCheckInternetAccessBlock", 0, 0, true, SECONDLEVEL);
		}

		if (dwParentalControl == 1)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = ON_CHECK_INTERNET_ACCESS;

			CISpyCommunicator objCom(SERVICE_SERVER);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send Data in CWardwizUpdateDlg::SendData", 0, 0, true, SECONDLEVEL);
			}

			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read Data in CWardwizUpdateDlg::ReadData", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizUpdateDlg::FunCheckInternetAccessBlock()", 0, 0, true, SECONDLEVEL);
	}
	return RetVal;
}