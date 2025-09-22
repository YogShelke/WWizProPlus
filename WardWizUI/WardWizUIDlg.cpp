
/*********************************************************************
*  Program Name: CWardWizUIDlg.cpp
*  Description: CWardWizUIDlg Implementation
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28 March 2016
*  Version No: 2.0.0.1
**********************************************************************/
#include "stdafx.h"
#include "WardWizUI.h"
#include "WardWizUIDlg.h"
#include "afxdialogex.h"
#include "WardWizUpdates.h"
#include "WardWizOSversion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SETCRYPTDETAIL_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 8)
#define SETTOOLTIP_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 9)
#define SHOWREG_PAGE_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 10)
#define SCHED_SCAN_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 11)

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

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

CWardWizUIDlg * CWardWizUIDlg::m_pThis = NULL;
CISpyCommunicatorServer  g_objCommServer(UI_SERVER, CWardWizUIDlg::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));

CWardWizUIDlg::CWardWizUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWardWizUIDlg::IDD, pParent)
	, m_objWardWizScan()
	, m_objWardWizRegistryOpt()
	, m_objWardWizDataCrypt()
	, m_objWardWizRecover()
	, m_objWardWizAntirootkit()
	, m_objWardWizUpdates()
	, m_bLiveUpdateMsg(false)
	, m_bIsActiveProtectionON(false)
	, m_bIsRegInProgress(false)
	, m_hThreadLaunchOprtn(NULL)
	, m_objComService(SERVICE_SERVER, true, 3)
	, m_objComTray(TRAY_SERVER, true, 3)
{
	m_pThis = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWardWizUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWardWizUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(HANDLEUIREQUEST, OnUserMessagesHandleUIRequest)
	ON_WM_CLOSE()
	ON_WM_HELPINFO()
	ON_MESSAGE(WM_MESSAGESHOWREG, ShowRegMessageHandler)
	ON_MESSAGE(WM_MESSAGEMAXIMWND, OnMaximiseWindow)
END_MESSAGE_MAP()

HWINDOW   CWardWizUIDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizUIDlg::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Initializes the dialog window
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
BOOL CWardWizUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
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

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	g_objCommServer.Run();

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_INDEX.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_INDEX.htm");	
	this->SetWindowText(L"VibraniumUI");

	//Lauch System Tray here
	StartWardWizTray();

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	if (theApp.m_bRunLiveUpdate)
	{
		ShowProductUpdate();
	}
	//Issue No.1754: Start up scan(Quick, Full scan) is not working.
	if (theApp.m_bStartUpScan)
	{
		OnStartUpScan();
	}
	if (theApp.m_bScanPage)
	{
		m_pThis->ShowScanPage();
	}
	if (theApp.m_bReportPage)
	{
		m_pThis->ShowReports();
	}
	if (theApp.m_bSchedScan)
	{
		OnStartUpScan();
	}
	if (theApp.m_bEPSNoUIScan)
	{
		::MoveWindow(this->get_hwnd(), 0, 0, 0, 0, true);
		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
		OnStartUpScan();
	}
	if (theApp.m_bDataCryptOpr)
	{
		bool bIsRequestFromExplorer = true;
		ShowDataCryptOpr(bIsRequestFromExplorer);
	}
	if (theApp.m_bOpenRegDlg)
	{
		if (theApp.m_dwDaysLeft <= 0)
		{
			Sleep(1000 * 3);
			ShowRegistrationDialog();
		}
	}
	if (theApp.m_bAllowStartUpTip && !theApp.m_bStartUpScan && !theApp.m_bRunLiveUpdate && !theApp.m_bIsEnDecFrmShellcmd && !theApp.m_bScanPage && !theApp.m_bReportPage)
	{
		ShowToolTipDialog();
	}

	CString	csWardWizModulePath = GetWardWizPathFromRegistry();
	CString	csWardWizReportsPath = L"";
	csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
	CT2A dbPath(csWardWizReportsPath, CP_UTF8);

	if (!PathFileExists(csWardWizReportsPath))
	{
		 // Add entries into Database..
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
		objSqlDb.Open();
		objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
		objSqlDb.Close();
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Get the product id
*  Author Name    : Amol Jaware
*  Date			  : 05 Aug 2016
****************************************************************************************************/
json::value CWardWizUIDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}

	return iProdValue;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Amol Jaware
*  Date			  : 5 Aug 2016
****************************************************************************************************/
json::value CWardWizUIDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

void CWardWizUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWardWizUIDlg::OnPaint()
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
HCURSOR CWardWizUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : To handle the Sciter UI related requests
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
LRESULT CWardWizUIDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	__try
	{
		if (message == WM_TIMER)
		{
			if (LOWORD(wParam) == TIMER_SCAN_STATUS)
			{
				m_pThis->m_objWardWizScan.OnTimerScan();
			}
			else if (LOWORD(wParam) == TIMER_SCAN_STATUS_FULL)
			{
				m_pThis->m_objCWardWizFullScan.OnTimerScan();
			}
			else if (LOWORD(wParam) == TIMER_SCAN_STATUS_QUICK)
			{
				m_pThis->m_objCWardWizQuickScan.OnTimerScan();
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
		AddLogEntry(L"### Exception in CWardwizUIDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  : On_Close
*  Description    : to close the Application
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
json::value CWardWizUIDlg::On_Close()
{
	{	//This is workaroung but not a fix.
		//Forcefully close as because the current UI architecture is:
		//All sub dialog objects are created in this class,
		//while destroying these objects getting delayed, which takes minutes to close UI.
		CWardWizOSversion	objOSVersion;
		if (objOSVersion.DetectClientOSVersion() == WINOS_XP || objOSVersion.DetectClientOSVersion() == WINOS_VISTA)
		{
			exit(0);
		}
		else
		{
			OnCancel();
		}
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_Minimize
*  Description    : Minimize the Application
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
json::value CWardWizUIDlg::On_Minimize()
{
	this->ShowWindow(SW_MINIMIZE);
	return json::value();
}

/***************************************************************************************************
*  Function Name  : OnDataReceiveCallBack
*  Description    : It receive flag send from named pipe
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
void CWardWizUIDlg::OnDataReceiveCallBack(LPVOID lpParam)
{
	__try
	{
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		if (!lpSpyData)
			return;

		DWORD dwReply = 0;
		switch (lpSpyData->iMessageInfo)
		{
		case SCAN_STARTED:

			break;

		case SCAN_FINISHED:
			m_pThis->ScanFinished();
			break;

		case SHOW_VIRUSENTRY:
			m_pThis->m_objWardWizScan.VirusFoundEntries(lpSpyData);
			break;
		case SET_TOTALFILECOUNT:
			m_pThis->SetTotalFileCount(lpSpyData);
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case REGISTRY_SCAN_FINISHED:
			m_pThis->m_objWardWizRegistryOpt.ScanningStopped();
			break;

		case DATA_ENC_DEC_SHOW_STATUS:
		{
			theApp.m_csDataCryptFilePath = lpSpyData->szFirstParam;
			bool dwIsSaveAS = lpSpyData->dwValue == SAVE_AS ? true : false;

			m_pThis->m_objWardWizDataCrypt.UpdateDataCryptOpr(lpSpyData);

			if (dwIsSaveAS)
			{
				g_objCommServer.SendResponse(lpSpyData);
			}
		}
		break;

		case ANTIROOT_STARTED:
			m_pThis->m_objWardWizAntirootkit.RootKitScanningStarted();
			break;
		case ANTIROOT_FINISHED:
			m_pThis->m_objWardWizAntirootkit.RootKitScanningFinished();
			break;
		case CHECKLIVEUPDATETRAY:
			m_pThis->ShowProductUpdate();
			break;
		case SEND_ACTIVE_PROTECTION_STATUS:
			m_pThis->ReloadHomePage();
			break;
		case DATA_ENC_DEC_OPERATIONS:
			theApp.m_csDataCryptFilePath = lpSpyData->szFirstParam;
			theApp.m_bDataCryptOpr = true;
			theApp.m_iDataOpr = lpSpyData->dwValue;
			{
				bool bIsRequestFromExplorer = false;
				m_pThis->ShowDataCryptOpr(bIsRequestFromExplorer);
			}
			break;

		case SHOW_SPECUI_PAGE:
			if (lpSpyData->dwValue == SCANPAGE)
			{
				m_pThis->ShowScanPage();
			}
			else if (lpSpyData->dwValue == REPORT)
			{
				m_pThis->ShowReports();
			}
			break;
		case START_SCHEDULED_SCAN:
			if (lpSpyData->dwValue == 2)
				m_pThis->SendRegistryOptSchedScanData(lpParam);
			else
			m_pThis->StartSheduledScan(lpSpyData->dwValue, lpSpyData->dwSecondValue);
			break;
		case IS_UI_ANY_TASK_RUNNING:
			dwReply = 0;
			if (m_pThis->CheckForTasksRunningUI())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SEND_PC_TRAY_CLOSE:
			dwReply = 0;
			if (m_pThis->ClosePasswordWndForPC())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SendRegistryOptSchedScanData
*  Description    : It sends data from pipe.
*  Author Name    : Amol Jaware
*  Date			  : 27 March 2018
****************************************************************************************************/
void CWardWizUIDlg::SendRegistryOptSchedScanData(LPVOID lpParam)
{
	try
	{

		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		if (!lpSpyData)
			return;

		m_pThis->StartSheduledScan4RegOpt(lpSpyData->dwValue, lpSpyData->dwSecondValue, lpSpyData->szFirstParam);

	}
	catch (...)
	{
	}
}
/***************************************************************************
Function Name  : OnUserMessagesHandleUIRequest
Description    : This function get call when some other module send messge
WM_COMMAND	   : HANDLEUIREQUEST
Author Name    : Nitin Kolapkar
Date           : 28 March 2016
****************************************************************************/
LRESULT CWardWizUIDlg::OnUserMessagesHandleUIRequest(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);
		AddLogEntry(L"### OnUserMessagesHandleUIRequest called" , 0, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::OnUserMessagesHandleUIRequest", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : ScanFinished
Description    : send UI scan finished message
Author Name    : Nitin Kolapkar
Date           : 28 March 2016
****************************************************************************/
void CWardWizUIDlg::ScanFinished()
{
	try
	{
		m_objWardWizScan.ScanFinished();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ScanFinished", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : SetTotalFileCount
Description    : set total file count on UI
Author Name    : Nitin Kolapkar
Date           : 28 March 2016
****************************************************************************/
void CWardWizUIDlg::SetTotalFileCount(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		if (theApp.m_bIsScanning)
		{
			DWORD dwTotalFileCount = lpSpyData->dwValue;
			CString csTotalFileCount = L"";
			CString csScannedFileCount = L"";
			csTotalFileCount.Format(L"%d", dwTotalFileCount);
			m_objWardWizScan.m_dwTotalFileCount = dwTotalFileCount;
			m_objWardWizScan.m_FileScanned = 0;
			csScannedFileCount.Format(L"%d", m_objWardWizScan.m_FileScanned);
			m_objWardWizScan.CallUISetFileCountfunction(csTotalFileCount, csScannedFileCount);
		}

		else if (theApp.m_bIsAntirootkitScanning)
		{
			DWORD dwTotalFileCnt = 0x00, dwTotalDriveCnt = 0x00, dwTotalProcCnt = 0x00, dwDetectedFileCnt = 0x00, dwDetectedDriveCnt = 0x00, dwDetectedProcCnt = 0x00;
			dwTotalFileCnt = (*(DWORD *)&lpSpyData->byData[0]);
			dwTotalDriveCnt = (*(DWORD *)&lpSpyData->byData[8]);
			dwTotalProcCnt = (*(DWORD *)&lpSpyData->byData[16]);
			dwDetectedFileCnt = (*(DWORD *)&lpSpyData->byData[24]);
			dwDetectedDriveCnt = (*(DWORD *)&lpSpyData->byData[32]);
			dwDetectedProcCnt = (*(DWORD *)&lpSpyData->byData[40]);
			m_objWardWizAntirootkit.CallRootkitUIFileCountfunction(dwTotalFileCnt, dwTotalDriveCnt, dwTotalProcCnt, dwDetectedFileCnt, dwDetectedDriveCnt, dwDetectedProcCnt);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::SetTotalFileCount", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_CallMessageBoxOnUI
*  Description    : This is 1st call from UI to get the Callback pointer for Showing messagebox on UI
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
json::value CWardWizUIDlg::On_CallMessageBoxOnUI(SCITER_VALUE progressCb)
{
	try
	{
		m_pMessageboxFunctionCB = progressCb;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_CallMessageBoxOnUI", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : CallUIMessageBox
*  Description    : Calls the view.msgbox in UI
*  Additional	  : "information" for displaying error ICON,
					"alert" for displaying alert ICON,
					"error" for displaying error ICON,
					"warning" for displaying warning ICON,
					"question" for displaying question ICON,
*  Author Name    : Nitin Kolapkar
*  Date			  : 28 March 2016
****************************************************************************************************/
void CWardWizUIDlg::CallUIMessageBox(CString csMessage, CString csMessageHeader, CString csMessageboxType)
{
	try
	{
		m_pMessageboxFunctionCB.call((SCITER_STRING)csMessage, (SCITER_STRING)csMessageHeader, (SCITER_STRING)csMessageboxType);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::CallUIMessageBox", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : ShowDataCryptOpr
Description    : Launch the Crypt.exe as commandline with required parameters
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 16th April 2016
***********************************************************************************************/
void CWardWizUIDlg::ShowDataCryptOpr(bool bIsRequestFromExplorer)
{
	try
	{
		CString csCommandLinePath = L"";
		csCommandLinePath = RemoveUnWantedPipeSymblFromPath(theApp.m_csDataCryptFilePath);
			sciter::value map;
			map.set_item("one", sciter::string(csCommandLinePath));
			map.set_item("two", SCITER_VALUE(theApp.m_iDataOpr));
			map.set_item("three", bIsRequestFromExplorer);
			m_root_el = root();
			sciter::dom::element ela = m_root_el;
			BEHAVIOR_EVENT_PARAMS beParams;
			beParams.cmd = SETCRYPTDETAIL_EVENT_CODE;
			beParams.he = beParams.heTarget = ela;
			beParams.data = map;
			ela.fire_event(beParams, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ShowDataCryptOpr", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : RemoveUnWantedPipeSymblFromPath
Description    : it provides functionality to remove unwanted pipe sysmbol from end of path
SR.NO		   :
Author Name    : Lalit kumawat
Date           : 29th June 2015
***********************************************************************************************/
CString  CWardWizUIDlg::RemoveUnWantedPipeSymblFromPath(CString csSelectedArgumentPath)
{
	CString csPath = L"";
	try
	{
		csPath = csSelectedArgumentPath.Mid(csSelectedArgumentPath.ReverseFind('|') + 1);
		TCHAR  szTmpPath[6 * MAX_PATH] = { 0 };
		swprintf_s(szTmpPath, _countof(szTmpPath), L"%s", csSelectedArgumentPath);
		if (csSelectedArgumentPath.ReverseFind('|') == -1 && csPath != L"")
		{
			return csSelectedArgumentPath;
		}
		while (csPath == L"")
		{
			CString csPath = csSelectedArgumentPath.Mid(csSelectedArgumentPath.ReverseFind('|') + 1);
			swprintf_s(szTmpPath, _countof(szTmpPath), L"%s", szTmpPath);
			if (csSelectedArgumentPath.ReverseFind('|') == -1 || csPath != L"")
			{
				break;
			}
			TCHAR	*pTemp = wcsrchr(szTmpPath, '|');
			if (!pTemp)
			{
				AddLogEntry(L"### Failed in removing unwanted pipe character from path CVibraniumCryptDlg::RemoveUnWantedPipeSymblFromPath");
			}
			*pTemp = '\0';
			csSelectedArgumentPath = szTmpPath;
			csPath = csSelectedArgumentPath.Mid(csSelectedArgumentPath.ReverseFind('|') + 1);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::RemoveUnWantedPipeSymblFromPath", 0, 0, true, SECONDLEVEL);
	}
	return csSelectedArgumentPath;
}


/***********************************************************************************************
Function Name  : On_ClickApplyDefaultSettings
Description    : Apply default settings
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
json::value CWardWizUIDlg::On_ClickRegisterNowBtn()
{
	try
	{
		m_bIsRegInProgress = true;
		theApp.DoRegistration();
		theApp.GetDaysLeft();
		GetDays();
		m_bIsRegInProgress = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUIDlg::On_ClickRegisterNowBtn", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : GetRegisteredUserInfo
Description    : Get registered user info and show it on UI (Accounts)
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 20th May 2016
***********************************************************************************************/
json::value CWardWizUIDlg::GetRegisteredUserInfo()
{
	TCHAR	szKey[0x32] = { 0 };
	CString csThreatDefCount;
	try
	{
		CString csEmailID = L"NA", csUserFirstName = L"NA", csUserLastName = L"NA", csNoofDays;
		CString csFullName = L"NA", csDataEncVersion, csProdVersion, csInstalledEdition, csProductKey;
		CString csDatabaseVersion, csDelearCode = L"0", csReferenceID = L"0", csScanEngineVer = L"0";
		DWORD dwDaysLeft = 0;
		ReadProductVersion4mRegistry();
		csProdVersion.Format(L"%s", m_csRegProductVer);
		csScanEngineVer.Format(L"%s", m_csRegScanEngineVer);
		csDatabaseVersion.Format(L"%s", m_csRegDataBaseVer);
		theApp.GetRegisteredUserInfo();
		dwDaysLeft = theApp.GetDaysLeft();
		if (!dwDaysLeft)
		{
			csNoofDays = L"0";
		}
		else
		{
			csNoofDays.Format(L"%d", dwDaysLeft);
		}
		if (GetInformationFromINI())
		{
			csThreatDefCount.Format(L"%s", m_csValueData);

			if (csThreatDefCount == "")
			{
				csThreatDefCount = L"8127413";//Default Value as current DB Count.
			}
		}
		m_bAllowDemoEdition = false;
		if (m_bAllowDemoEdition)
		{
			csInstalledEdition = L"Vibranium Demo";
		}
		else
		{
			switch (theApp.m_dwProductID)
			{
			case ESSENTIAL:
				csInstalledEdition = L"Vibranium";
				break;
			case PRO:
				csInstalledEdition = L"Vibranium";
				break;
			case ELITE:
				csInstalledEdition = L"Vibranium";
				break;
			case BASIC:
				csInstalledEdition = L"Vibranium";
				break;
			case ESSENTIALPLUS:
				csInstalledEdition = L"Vibranium";
				break;

			default:
				csInstalledEdition = L"NA";
				AddLogEntry(L"### Invalid product edition", 0, 0, true, SECONDLEVEL);
				break;
			}
		}

		if (theApp.m_ActInfo.dwProductNo != theApp.m_dwProductID || !theApp.CheckForMachineID(theApp.m_ActInfo))
		{
			csEmailID = L"NA";
			csUserFirstName = L"NA";
			csUserLastName = L"NA";
			csNoofDays = L"0";
			csFullName = L"NA";
			csProductKey = L"NA";
			csDelearCode = L"0";
			csReferenceID = L"0";
		}
		else
		{
			csEmailID.Format(L"%s", theApp.m_ActInfo.szEmailID);
			csUserFirstName.Format(L"%s", theApp.m_ActInfo.szUserFirstName);
			csUserLastName.Format(L" %s", theApp.m_ActInfo.szUserLastName);
			csUserFirstName.AppendFormat(L"%s", csUserLastName);
			csFullName = csUserFirstName;
			memcpy(&szKey, theApp.m_ActInfo.szKey, sizeof(theApp.m_ActInfo.szKey));
			szKey[wcslen(szKey) + 1] = '\0';
			if (csFullName == L" ")
			{
				csFullName = L"NA";
			}
			if (csEmailID == L"")
			{
				csEmailID = L"NA";
			}
			csProductKey.Format(L"%s", szKey);
			if (csProductKey == L"")
			{
				if (dwDaysLeft)
				{
					csProductKey.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_TRAIL_VERSION"));
				}
				else
				{
					csProductKey.Format(L"NA");
				}
			}
			else
			{
				csProductKey.Format(L"%s", szKey);
			}

			if (_tcslen(m_csDelearCod) != 0)
			{
				csDelearCode.Format(L"%s", m_csDelearCod);
			}
			if (_tcslen(m_csReferenceID) != 0)
			{
				csReferenceID.Format(L"%s", m_csReferenceID);
			}
		}

		if (theApp.m_dwProductID == BASIC)
		{
			csDataEncVersion = L"0";
		}
		else
		{
			csDataEncVersion.Format(L"%s", m_csDataEncVer);
		}

		const sciter::value svArrayMembers[12] = { sciter::string(csFullName), sciter::string(csEmailID),
			sciter::string(csNoofDays), sciter::string(csInstalledEdition), sciter::string(csProdVersion), sciter::string(csProductKey),
			sciter::string(csDataEncVersion), sciter::string(csThreatDefCount), sciter::string(csDatabaseVersion), sciter::string(csDelearCode), sciter::string(csReferenceID), sciter::string(csScanEngineVer) };
		sciter::value svArrUserDetails = sciter::value(svArrayMembers, 12);
		return  svArrUserDetails;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUIDlg::GetRegisteredUserInfo", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : ReadProductVersion4mRegistry
Description    : Retrieves Product Version from Registry
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 27th May 2016
***********************************************************************************************/
void CWardWizUIDlg::ReadProductVersion4mRegistry()
{
	try
	{
		HKEY hKey = NULL;
		DWORD dwvalueSType = 0;
		DWORD dwvalueSize = sizeof(DWORD);
		DWORD ChkvalueForDemoEdition = 0;
		TCHAR szAppVersion[1024];
		TCHAR szScanEngineVersion[1024];
		DWORD dwAppVersionLen = 1024;
		DWORD dwszScanEngVerLen = 1024;
		TCHAR szDataEncVersion[1024];
		DWORD dwDataEncVersion = 1024;
		DWORD dwType = REG_SZ;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
		{
			return;
		}
		long ReadReg = RegQueryValueEx(hKey, L"AppVersion", NULL, &dwType, (LPBYTE)&szAppVersion, &dwAppVersionLen);
		if (ReadReg == ERROR_SUCCESS)
		{
			m_csRegProductVer = (LPCTSTR)szAppVersion;
		}
		
		ReadReg = RegQueryValueEx(hKey, L"ScanEngineVersion", NULL, &dwType, (LPBYTE)&szScanEngineVersion, &dwszScanEngVerLen);
		if (ReadReg == ERROR_SUCCESS)
		{
			m_csRegScanEngineVer = (LPCTSTR)szScanEngineVersion;
		}

		ReadReg = RegQueryValueEx(hKey, L"DataEncVersion", NULL, &dwType, (LPBYTE)&szDataEncVersion, &dwDataEncVersion);
		if (ReadReg == ERROR_SUCCESS)
		{
			m_csDataEncVer = (LPCTSTR)szDataEncVersion;
		}
		dwType = REG_DWORD;
		ReadReg = RegQueryValueEx(hKey, L"dwWardWizDemo", NULL, &dwType, (LPBYTE)&dwvalueSType, &dwvalueSize);
		ChkvalueForDemoEdition = (DWORD)dwvalueSType;
		if (ChkvalueForDemoEdition == 0)
		{
			m_bAllowDemoEdition = false;
		}
		else
		{
			m_bAllowDemoEdition = true;
		}
		TCHAR szvalueVersion[1024];
		DWORD dwvaluelengthVersion = 1024;
		DWORD dwtypeVersion = REG_SZ;
		ReadReg = RegQueryValueEx(hKey, L"DataBaseVersion", NULL, &dwtypeVersion, (LPBYTE)&szvalueVersion, &dwvaluelengthVersion);
		if (ReadReg == ERROR_SUCCESS)
		{
			m_csRegDataBaseVer = (LPCTSTR)szvalueVersion;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUIDlg::ReadProductVersion4mRegistry", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
* Function Name    : ShowHomepageControls
* Description      : Show and hide main controls
* Author Name	   : Neha Gharge
* Date Of Creation : 6th may 2014
* SR_NO            :
***********************************************************************************************************/
void CWardWizUIDlg::ShowHomepageControls(bool bEnable)
{
	m_bNonGenuineCopy = false;
	DWORD dwDaysLeft = 0x00;
	RegistryEntryOnUI();
	if (theApp.m_GetDaysLeft)
	{
		theApp.m_dwDaysLeft = theApp.m_GetDaysLeft(theApp.m_dwProductID);
	}

	dwDaysLeft = theApp.GetDaysLeft();
	CString csDaysleft = L"";
	csDaysleft.Format(L"%d", dwDaysLeft);
	m_dwNoofDays = theApp.m_dwDaysLeft;

	if (IsAnyChangeInDate())
	{
		if (!theApp.SendData2ComService(RELOAD_REGISTARTION_DAYS))
		{
			AddLogEntry(L"### Failed to SendData2Service in CWardwizUIDlg::ShowHomepageControls", 0, 0, true, SECONDLEVEL);
		}
	}

	SetNotProtectedMsg();

	if (!SendData2Tray(RELOAD_WIDGETS_UI, RELOAD_UPDATES))
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ShowHomepageControls", 0, 0, true, SECONDLEVEL);
	}
	m_svDataToDisplayCB.call((SCITER_STRING)csDaysleft, (SCITER_STRING)m_csUpdateDate, (SCITER_STRING)m_csScanType, m_iProtectionStatusMsgType);//(SCITER_STRING)m_csProtectionStatusMsg);
}

/**********************************************************************************************************
* Function Name         : RegistryEntryOnUI
* Description           : Details of AV from Registry.
* Author Name			: Rajil Yadav
* Date Of Creation      : 7th May 2014
* SR_No                 :
***********************************************************************************************************/
void CWardWizUIDlg::RegistryEntryOnUI()
{
	try
	{
		CITinRegWrapper objReg;
		DWORD dwData = 0x00;
		CString csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();
		
		HKEY key;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Vibranium"), &key) != ERROR_SUCCESS)
		{
			return;
		}

		DWORD dwValueOFOffline = sizeof(DWORD) + 1;
		DWORD dwOfflineTypeType = REG_DWORD;
		DWORD dwRegUserType = 0x00, dwNGC = 0x00;
		long ReadReg = RegQueryValueEx(key, L"dwRegUserType", NULL, &dwOfflineTypeType, (LPBYTE)&m_dwvalueSType, &dwValueOFOffline);
		if (ReadReg == ERROR_SUCCESS)
		{
			dwRegUserType = (DWORD)m_dwvalueSType;
		}
		ReadReg = RegQueryValueEx(key, L"dwNGC", NULL, &dwOfflineTypeType, (LPBYTE)&m_dwvalueSType, &dwValueOFOffline);
		if (ReadReg == ERROR_SUCCESS)
		{
			dwNGC = (DWORD)m_dwvalueSType;
			if (theApp.m_dwDaysLeft == 0)
			{
				theApp.GetDaysLeft();
			}

			if (dwRegUserType == 1 && theApp.m_dwDaysLeft == 0 && dwNGC == 1)
			{
				m_bNonGenuineCopy = true;
			}
		}

		TCHAR szvalue[1024];
		DWORD dwvalue_length = 1024;
		DWORD dwtype = REG_SZ;
		TCHAR szDay[3] = { 0 };
		TCHAR szMonth[3] = { 0 };
		TCHAR szYear[5] = { 0 };
		TCHAR szCurrentDay[3] = { 0 };
		TCHAR szCurrentMonth[3] = { 0 };
		TCHAR szCurrentYear[3] = { 0 };
		TCHAR szCurrentBuffer[9] = { 0 };
		ReadReg = RegQueryValueEx(key, L"LastLiveupdatedt", NULL, &dwtype, (LPBYTE)&szvalue, &dwvalue_length);

		if (ReadReg == ERROR_SUCCESS)
		{
			m_lpstrDate = (LPCTSTR)szvalue;

			m_csUpdateDate.Format(L"%s", m_lpstrDate);

			CString csCommandLine = (CString)m_lpstrDate;
			
			int iPos = 0;
			int szTemp[3] = { 0 };
			for (int i = 0; i< 3; i++)
			{
				CString csTemp = csCommandLine.Tokenize(_T("/"), iPos);
				szTemp[i] = _wtoi(csTemp);
			}
			int iYear = szTemp[2];
			int iDay = szTemp[1];
			int iMonth = szTemp[0];

			CTime Time_Curr = CTime::GetCurrentTime();
			int iMonth1 = Time_Curr.GetMonth();
			int iDate1 = Time_Curr.GetDay();
			int iYear1 = Time_Curr.GetYear();

			if (!isValidDate(iDay, iMonth, iYear) || !isValidDate(iDate1, iMonth1, iYear1))
			{
				AddLogEntry(L"### Invalid LastLiveupdatedt/GetCurrentTime in CVibraniumHomePage::RegistryEntryOnUI().", 0, 0, true, SECONDLEVEL);

			}
			else
			{
				CTime Time_RegistryDate(iYear, iMonth, iDay, 0, 0, 0);
				CTime Time_CurDate(iYear1, iMonth1, iDate1, 0, 0, 0);
				CTimeSpan Time_Diff = Time_CurDate - Time_RegistryDate;
				int Span = static_cast<int>(Time_Diff.GetDays());
				if (Span > 7)
				{
					m_bLiveUpdateMsg = true;
					m_bVirusMsgDetect = 0;
				}
			}
		}

		TCHAR szvalueDT[1024];
		DWORD dwvalue_lengthDT = 1024;
		DWORD dwtypeDT = REG_SZ;
		ReadReg = RegQueryValueEx(key, L"LastScandt", NULL, &dwtypeDT, (LPBYTE)&szvalueDT, &dwvalue_lengthDT);
		if (ReadReg == ERROR_SUCCESS)
		{
			LPCTSTR szLastScanDate = (LPCTSTR)szvalueDT;
		}
		TCHAR szvalueVersion[1024];
		DWORD dwvaluelengthVersion = 1024;
		DWORD dwtypeVersion = REG_SZ;
		ReadReg = RegQueryValueEx(key, L"DataBaseVersion", NULL, &dwtypeVersion, (LPBYTE)&szvalueVersion, &dwvaluelengthVersion);
		if (ReadReg == ERROR_SUCCESS)
		{
			m_csRegDataBaseVer = (LPCTSTR)szvalueVersion;
		}
		DWORD dwvaluelengthSType = sizeof(DWORD) + 1;
		DWORD dwtypeSType = REG_DWORD;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csProdRegKey.GetBuffer(), L"dwActiveScanOption", dwData) == 0x00)
		{
			DWORD dwActiveScanOption = (DWORD)dwData;
			if (dwActiveScanOption == 1)
				m_bIsActiveProtectionON = true;
			else
				m_bIsActiveProtectionON = false;
		}
		dwvaluelengthSType = sizeof(DWORD) + 1;
		dwtypeSType = REG_DWORD;
		ReadReg = RegQueryValueEx(key, L"ScanType", NULL, &dwtypeSType, (LPBYTE)&m_dwvalueSType, &dwvaluelengthSType);
		if (ReadReg == ERROR_SUCCESS)
		{
			DWORD Scantype = (DWORD)m_dwvalueSType;
			switch (Scantype)
			{
			case 0: m_csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FULL_SCAN");
					m_iProtectedMsgType = 1;
				break;
			case 1: m_csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOM_SCAN");
					m_iProtectedMsgType = 2;
				break;
			case 2: m_csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_QUICK_SCAN");
					m_iProtectedMsgType = 3;
				break;
			case 3:
			case 4: m_csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_SCAN");
					m_iProtectedMsgType = 4;
				break;
			case 5: m_csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_HEADER");
					m_iProtectedMsgType = 4;
				break;
			case 6: m_csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BOOT_TIME_SCAN");
				m_iProtectedMsgType = 5;
				break;
			default: m_csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_TYPE");
					 m_iProtectedMsgType = 0;
				break;
			}
		}
		RegCloseKey(key);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::RegistryEntryOnUI()", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
* Function Name      : IsValidDate
* Description        : If user changed the date of 'LastLiveUpdatdt' from registry to invalid date, then error pop-up gets displayed.
: Function checks whether the LastLiveUpdatedt is valid or not & returns true if date is valid, else returns false.
* Author Name		 : Varada IKhar
* Date Of Creation   : 11th April 2015
* SR_No              :
***********************************************************************************************************/
bool CWardWizUIDlg::isValidDate(int iDay, int iMonth, int iYear)
{
	try
	{
		if (iYear < 1970 || iYear > 3000 || iMonth < 1 || iMonth > 12 || iDay < 1)
		{
			return false;
		}
		if (iMonth == 2)
		{
			int Leapyear;
			Leapyear = iYear % 400 == 0 || (iYear % 4 == 0 && iYear % 100 != 0);
			if (Leapyear)
			{
				if (iDay > 29)
				{
					return false;
				}
			}
			else
			{
				if (iDay > 28)
				{
					return false;
				}
			}
		}
		else if (iMonth == 1 || iMonth == 3 || iMonth == 5 || iMonth == 7 || iMonth == 8 || iMonth == 10 || iMonth == 12)
		{
			if (iDay > 31)
			{
				return false;
			}
		}
		else if (iMonth == 4 || iMonth == 6 || iMonth == 9 || iMonth == 11)
		{
			if (iDay > 30)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumHomePage::isValidDate", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  ShowProductUpdate
*  Description    :  Function is called for Product Updation
*  Author Name    :  Jeena Mariam Saji
*  Date           :  16 May 2016
****************************************************************************************************/
void CWardWizUIDlg::ShowProductUpdate()
{
	try
	{
		if (m_bIsRegInProgress)
			return;
		const SCITER_VALUE result = m_root_el.call_function("CallShowProductUpdate"); //StartProductUpdateFromCmdLine
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ShowProductUpdate", 0, 0, true, SECONDLEVEL);
	}
} 

/**********************************************************************************************************
* Function Name         : GetDays
* Description           : To send number of days after successfull registration
* Author Name			: Adil Sheikh
* Date Of Creation      : 25th June 2016
* SR_No                 :
***********************************************************************************************************/
void CWardWizUIDlg::GetDays()
{
	DWORD dwDaysLeft;
	try
	{
		dwDaysLeft = theApp.GetDaysLeft();
		CString csDaysleft = L"";
		csDaysleft.Format(L"%d", dwDaysLeft);
		const SCITER_VALUE result = m_root_el.call_function("CallGetDays",(SCITER_STRING)csDaysleft); //GetDaysLeftToUIOnStart
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::GetDays", 0, 0, true, SECONDLEVEL);
	}
}
/**********************************************************************************************************
* Function Name         : HomepageControls
* Description           : To send all UI details to HTML 
* Author Name			: Adil Sheikh
* Date Of Creation      : 25th June 2016
* SR_No                 :
***********************************************************************************************************/
json::value CWardWizUIDlg::HomepageControls(SCITER_VALUE svDataToDisplayCB)
{
	try
	{
		m_svDataToDisplayCB = svDataToDisplayCB;
		ShowHomepageControls(m_bData);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::HomepageControls", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
* Function Name         : SetNotProtectedMsg
* Description           : On Not protected message to be disaled
* Author Name			: Varada Ikhar
* Date Of Creation      : 6th April 2015
* SR_No                 :
***********************************************************************************************************/
void CWardWizUIDlg::SetNotProtectedMsg()
{
	m_iProtectionStatusMsgType = 0;
	try
	{
		m_szEmailId_GUI = theApp.GetRegisteredEmailID();
		if (m_bNonGenuineCopy)
		{
			m_csProtectionStatusMsg = theApp.m_objwardwizLangManager.GetString(L"IDS_NOT_GENUINE");
			m_iProtectionStatusMsgType = 1;
			m_bNonGenuineCopy = false;
		}

		if ((m_dwNoofDays <= 30) && (m_dwNoofDays >= 0) && (_tcslen(theApp.m_szRegKey) != 0))
		{
			m_iProtectionStatusMsgType = 7;
		}
		//Check Active protection is ON or not
		if (!m_bIsActiveProtectionON)
		{
			m_csProtectionStatusMsg = m_csProtectionStatusMsg = theApp.m_objwardwizLangManager.GetString(L"IDS_ACTIVE_PROTECTION_DISABLED");
			m_iProtectionStatusMsgType = 8;
			return;
		}
		if (m_dwNoofDays == 0)
		{

			if (m_szEmailId_GUI && _tcslen(m_szEmailId_GUI) == 0)
			{
				m_csProtectionStatusMsg = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_UNREGISTERED");
				m_iProtectionStatusMsgType = 2;
			}
			else
			{
				m_iProtectionStatusMsgType = 6;
				if (theApp.m_szRegKey && _tcslen(theApp.m_szRegKey) == 0)
				{
					m_csProtectionStatusMsg = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_TRIAL_EXPIRED");
					m_iProtectionStatusMsgType = 3;
				}
				else
				{
					m_csProtectionStatusMsg = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_EXPIRED");
					m_iProtectionStatusMsgType = 4;
				}
			}
			return;
		}
		else if (m_dwNoofDays > 0)
		{
			if (theApp.m_szRegKey && _tcslen(theApp.m_szRegKey) == 0)
			{
				m_csProtectionStatusMsg = theApp.m_objwardwizLangManager.GetString(L"IDS_TRAIL_VERSION");
 				m_iProtectionStatusMsgType = 9;
				return;
			}
		}
		
		if (m_bLiveUpdateMsg)
		{
			m_csProtectionStatusMsg = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_OLD_DATABASE");
			m_iProtectionStatusMsgType = 5;
			m_bLiveUpdateMsg = 0;
			return;
		}
		else
		{
			m_iProtectionStatusMsgType = 0;
			return;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumHomePage::SetNotProtectedMsg()", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/**********************************************************************************************************
* Function Name         : HomeControls
* Description           : To do hide show of buttons on UI
* Author Name			: Adil Sheikh
* Date Of Creation      : 30th June 2016
* SR_No                 :
***********************************************************************************************************/
json::value CWardWizUIDlg::HomeControls(SCITER_VALUE svDisabledCB)
{ 
	try
	{
		m_iDisabled = 0;
		m_svDisabledCB = svDisabledCB;
		if (m_szEmailId_GUI == L"" && _tcslen(theApp.m_szRegKey) == 0)
		{
			m_iDisabled = 5;		//Unregistered
		}

		else if (m_szEmailId_GUI && _tcslen(theApp.m_szRegKey) == 0)
		{
			m_iDisabled = 1;
			if (m_dwNoofDays == 0)		//Trial Version
			{
				m_iDisabled = 4;		//Expired
			}
		}
		else if (theApp.m_szRegKey && m_szEmailId_GUI)
		{
			m_iDisabled = 2;		//Registered User
			if (m_dwNoofDays == 0)
			{
				m_iDisabled = 3;	//Expired
			}
		}
		m_svDisabledCB.call(m_iDisabled);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::HomeControls", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : StartiSpyAVTray
Description    : This function will launch the Tray.
Author Name    : Ramkrushna Shelke
SR_NO			 :
Date           : 18th Nov 2013
****************************************************************************/
void CWardWizUIDlg::StartWardWizTray()
{
	AddLogEntry(L">>> WardwizGUIDlg : Start Wardwiz tray", 0, 0, true, FIRSTLEVEL);
	TCHAR szModulePath[MAX_PATH] = { 0 };
	TCHAR szFullPath[MAX_PATH] = { 0 };

	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	_tcscpy_s(szFullPath, szModulePath);
	_tcscat_s(szFullPath, L"\\VBTRAY.EXE");

	//ISsue no 742 Neha Gharge 17/6/2014 
	::ShellExecute(NULL, L"Open", szFullPath, L"-NOSPSCRN", NULL, SWP_SHOWWINDOW);
}

/***************************************************************************
Function Name  : GetInformationFromINI
Description    : This function is used to read Threat Definition Count from PRODUCTSETTINGS.ini file.
Author Name    : Adil Sheikh
SR_NO		   :
Date           : 8th july 2016
****************************************************************************/
bool CWardWizUIDlg::GetInformationFromINI()
{
	try
	{
		TCHAR  szActualINIPath[255] = { 0 };
		if (theApp.m_dwProductID)
		{
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%sVBSETTINGS\\%s",theApp.m_AppPath, L"PRODUCTSETTINGS.ini");
		}

		if (!szActualINIPath)
		{
			AddLogEntry(L"### File not found : %s in GetInformationFromINI ", szActualINIPath, 0, true, SECONDLEVEL);
			return false;
		}
		else
		{
			GetPrivateProfileString(L"VBSETTINGS", L"ThreatDefCount", L"", m_csValueData, 511, szActualINIPath);

			GetPrivateProfileString(L"VBSETTINGS", L"DealerCode", L"", m_csDelearCod, 511, szActualINIPath);
		
			GetPrivateProfileString(L"VBSETTINGS", L"ReferenceID", L"", m_csReferenceID, 511, szActualINIPath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in CWardwizUIDlg::GetInformationFromINI ",0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : OnClose
Description    : The framework calls this member function as a signal 
				   that the CWnd or an application is to terminate.
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 27th July 2016
***********************************************************************************************/
void CWardWizUIDlg::OnClose()
{
	try
	{
		const SCITER_VALUE result = m_root_el.call_function("CloseUI");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::OnClose", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  ShowRegistrationDialog
*  Description    :  Function is called for Registration
*  Modified By    :  Nitin K.
*  Date           :  29 July 2016
****************************************************************************************************/
void CWardWizUIDlg::ShowRegistrationDialog()
{
	try
	{
		sciter::value map;
		map.set_item("one", sciter::string());
		m_root_el = root();
		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS beParams;

		beParams.cmd = SHOWREG_PAGE_EVENT_CODE;
		beParams.he = beParams.heTarget = ela;
		beParams.data = map;
		ela.fire_event(beParams, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ShowRegistrationDialog", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  LaunchUtilities
*  Description    :  Function is called for launching utilities
*  Modified By    :  Nitin K.
*  Date           :  05 August 2016
****************************************************************************************************/
json::value CWardWizUIDlg::LaunchUtilities(SCITER_VALUE svUtilityType)
{
	try
	{
		CString m_csAppPath = L"";
		m_csAppPath = theApp.m_AppPath;
		if (svUtilityType == 0)
		{
			m_csAppPath += L"VBAUTORUNSCN.EXE";
			ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
		}
		if (svUtilityType == 1)
		{
			m_csAppPath += L"VBTEMPCLR.EXE";
			ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
		}
		if (svUtilityType == 2)
		{
			m_csAppPath += L"VBUSBVAC.EXE";
			ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
		}
		if (svUtilityType == 3)
		{
			m_csAppPath += L"VBRESCUEDISK.EXE";
			ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
		}
		if (svUtilityType == 4)
		{
			m_csAppPath += L"VBREGTOOL.EXE";
			ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
		}
		/*if (svUtilityType == 3)
		{
			m_csAppPath += L"WRDWIZPROCESSEXPLORER.EXE";
			ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
		}*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::LaunchUtilities", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  LaunchWardWizUpdate
*  Description    :  Function is called for launching updates
*  Modified By    :  Amol Jaware
*  Date           :  15 Jan 2019
****************************************************************************************************/
json::value CWardWizUIDlg::LaunchWardWizUpdate(SCITER_VALUE svUpdateType)
{
	try
	{
		CString m_csAppPath = L"";
		m_csAppPath = theApp.m_AppPath;
		if (svUpdateType == 0)
		{
			m_csAppPath += L"VBUPDATE.EXE";
			ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
		}
		else if(svUpdateType == 1)
		{
			m_csAppPath += L"VBUPDATE.EXE";
			ShellExecute(NULL, L"open", m_csAppPath, L"-LIVEUPDATE", NULL, SW_SHOW);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::LaunchWardWizUpdate", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ShowCheckboxValue
*  Description    : Call registry entery function to make enable/disable start up tip.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 09 Oug 2016
****************************************************************************************************/
json::value CWardWizUIDlg::On_ShowCheckboxValue(SCITER_VALUE svCheckBoxValue)
{
	try
	{
		WriteRegistryEntryofStartUpTips(svCheckBoxValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_ShowCheckboxValue", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : WriteRegistryEntryofStartUpTips
*  Description    : Write registry entry of enable/disable start up tip.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 09 Oug 2016
****************************************************************************************************/
void CWardWizUIDlg::WriteRegistryEntryofStartUpTips(SCITER_VALUE svChangeValue)
{
	AddLogEntry(L">>> CWardwizUIDlg : WriteRegistryEntryofStartUpTips", 0, 0, true, FIRSTLEVEL);
	bool bValue = svChangeValue.get(false);  
	DWORD dwChangeValue = 0;
	if(bValue)
		dwChangeValue = 1;
	else
		dwChangeValue = 0;

	if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath, L"dwShowStartupTips", REG_DWORD, dwChangeValue))
	{
		AddLogEntry(L"### Error in Setting Registry VibraniumStartupTipsDlg::EnableStartupTipsSettings", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : Set registry key using service through pipe.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 09 Oug 2016
****************************************************************************************************/
bool CWardWizUIDlg::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName);
	szPipeData.dwSecondValue = dwData;

	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CWardwizUIDlg : SendVibraniumStartupTipsOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizUIDlg : SendVibraniumStartupTipsOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  OnStartUpScan
*  Description    :  Function is called for Startup Scan
*  Author Name    :  Jeena Mariam Saji
*  Date           :  11 Aug 2016
****************************************************************************************************/
void CWardWizUIDlg::OnStartUpScan()
{
	try
	{
		CString csScanType = L"";
		bool bIsScanShutDown;
		if (theApp.m_bSchedScanShutDown)
		{
			bIsScanShutDown = true;
		}
		else
		{
			bIsScanShutDown = false;
		}
		if (theApp.m_bRunFullScan)
		{
			csScanType = L"FullScan";
			const SCITER_VALUE result = m_root_el.call_function("CallOnStartUpScan", (SCITER_STRING)csScanType, bIsScanShutDown);
		}
		else if (theApp.m_bRunQuickScan)
		{
			csScanType = L"QuickScan";
			const SCITER_VALUE result = m_root_el.call_function("CallOnStartUpScan", (SCITER_STRING)csScanType, bIsScanShutDown);
		}
		else if (theApp.m_bRunRegOpt)
		{
			CString csCommandLine = GetCommandLine();
			csCommandLine.MakeUpper();

			if (csCommandLine.Find(L"-SCHEDSCAN -REGOPT -SDNO") == 0)
			{
				csCommandLine.Replace(TEXT("-SCHEDSCAN -REGOPT -SDNO"), L"");
				csCommandLine.Trim();
				m_objWardWizRegistryOpt.GetRegistryOptionList(csCommandLine);
			}

			csScanType = L"regopt";
			const SCITER_VALUE result = m_root_el.call_function("CallOnStartUpScan", (SCITER_STRING)csScanType, false);
			
		}
		if (theApp.m_bEPSFullScanNoUI == true)
		{
			csScanType = L"FullScan";
			const SCITER_VALUE result = m_root_el.call_function("CallOnStartUpScanNoUI", (SCITER_STRING)csScanType, bIsScanShutDown);
		}
		else if (theApp.m_bEPSQuickScanNoUI == true)
		{
			csScanType = L"QuickScan";
			const SCITER_VALUE result = m_root_el.call_function("CallOnStartUpScanNoUI", (SCITER_STRING)csScanType, bIsScanShutDown);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::OnStartUpScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : GetSupportNo
Description    : Get support No from registry.
Author Name    : Amol Jaware.
SR_NO		   :
Date           : 16th Oug 2016
/***************************************************************************************************/
json::value CWardWizUIDlg::GetSupportNo()
{
	CITinRegWrapper	objReg;
	TCHAR	szPath[512] = { 0 };
	DWORD	dwSize = 511;
	try
	{
		objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"SupportNo", szPath, dwSize);
		if (!szPath[0])
		{
			AddLogEntry(L"### Failed to get support no entry", 0, 0, true, ZEROLEVEL);
			return L"";
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::GetSupportNo", 0, 0, true, SECONDLEVEL);
	}
	return szPath;

}

/***************************************************************************************************
Function Name  : OnHelpInfo
Description    : Launch Help File
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 28th Aug 2016
/***************************************************************************************************/
BOOL CWardWizUIDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	try
	{
		const SCITER_VALUE result = m_root_el.call_function("OpenHelpFile"); //StartupToolTipDialog
		return TRUE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::OnHelpInfo", 0, 0, true, SECONDLEVEL);
	}
	return __super::OnHelpInfo(pHelpInfo);
}

/***************************************************************************************************
Function Name  : ShowToolTipDialog
Description    : Function to show ToolTip
Author Name    : Jeena Mariam Saji
SR_NO		   :
Date           : 13 Sept 2016
/***************************************************************************************************/
void CWardWizUIDlg::ShowToolTipDialog()
{
	try
	{
		sciter::value map;
		map.set_item("one", sciter::string());
		m_root_el = root();
		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS beParams;

		beParams.cmd = SETTOOLTIP_EVENT_CODE;
		beParams.he = beParams.heTarget = ela;
		beParams.data = map;
		ela.fire_event(beParams, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ShowToolTipDialog", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
Function Name  : onModalLoop
Description    : for reseting the Lightbox event msgbox
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 7th Oct 2016
/***************************************************************************************************/
json::value CWardWizUIDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
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
		AddLogEntry(L"### Exception in CWardwizUIDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : PreTranslateMessage
Description    : Ignore Enter/escape button click events
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 7th Oct 2016
/***************************************************************************************************/
BOOL CWardWizUIDlg::PreTranslateMessage(MSG* pMsg)
{
	if (wcscmp(theApp.m_csPageName, L"#EMAILSCAN") == 0x00)
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_TAB))
		{
			return TRUE;
		}
	}
	else if (wcscmp(theApp.m_csPageName, L"#SETTINGS") == 0x00)
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE || pMsg->wParam == VK_DOWN))
		{
			return TRUE;
		}
	}
	else if (wcscmp(theApp.m_csPageName, L"#PARENTAL_CONTROL") == 0X00)
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_DOWN))
		{
			return TRUE;
		}
	}
	else if (wcscmp(theApp.m_csPageName, L"#FIREWALL") == 0X00)
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_SHIFT))
		{
			return TRUE;
		}
	}
	else if (wcscmp(theApp.m_csPageName, L"#RECOVER_FILES") == 0X00)
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT))
		{
			return TRUE;
		}
	}
	else
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE || pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_TAB))
		{
			return TRUE;
		}
	}
	if (pMsg->message == WM_CHAR || pMsg->message == WM_UNICHAR)
	{
		WindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
	}
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CWardWizUIDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : ReloadHomePage
Description    : Reload Home page
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 2nd Nov 2016
/***************************************************************************************************/
void CWardWizUIDlg::ReloadHomePage()
{
	try
	{
		const SCITER_VALUE result = m_root_el.call_function("ReloadHomePage");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ReloadHomePage", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  On_EnableDriverProtection
*  Description    :  Function is called for Enabling Driver Protection
*  Modified By    :  Nitin K.
*  Date           :  3rd November 2016
****************************************************************************************************/
json::value CWardWizUIDlg::On_EnableDriverProtection()
{
	try
	{
		DWORD dwEnableProtection = 1;
		if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath, L"dwActiveScanOption", REG_DWORD, dwEnableProtection))
		{
			AddLogEntry(L"### Error in Setting Registry CWardwizUIDlg::On_EnableDriverProtection", 0, 0, true, SECONDLEVEL);
			return 0;
		}
		if (!SendData2Service(HANDLEACTICESCANSETTINGS, ENABLEACTSCAN, 0, 0, 0, false))
		{
			if (!SendData2Service(HANDLEACTICESCANSETTINGS, ENABLEACTSCAN, 0, 0, 0, false))
			{
				AddLogEntry(L"### Failed SendData2Service in CWardwizUIDlg::On_EnableDriverProtection", 0, 0, true, SECONDLEVEL);
			}
		}

		if (!SendData2Tray(HANDLEACTICESCANSETTINGS, ENABLEACTSCAN, 0, 0, 0, false))
		{
			AddLogEntry(L"### Failed SendData2Tray in CWardwizUIDlg::On_EnableDriverProtection", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_EnableDriverProtection", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : SendData2Service
Description    : Sends action request to service
Author Name    : Nitin Kolapkar
Date           : 3rd November 2016
SR_NO		   :
***********************************************************************************************/
bool CWardWizUIDlg::SendData2Service(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam, LPTSTR lpszSecondParam, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = iMessage;
		szPipeData.dwValue = dwValue;
		if (lpszFirstParam != NULL)
		{
			wcscpy_s(szPipeData.szFirstParam, lpszFirstParam);
		}
		if (lpszSecondParam != NULL)
		{
			wcscpy_s(szPipeData.szSecondParam, lpszSecondParam);
		}
		szPipeData.dwSecondValue = dwSeondValue;
		if (!m_objComService.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			CString csMessage;
			csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
			AddLogEntry(L"### Failed to set data in CWardwizUIDlg::SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
			return false;
		}
		if (bWait)
		{
			if (!m_objComService.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csMessage;
				csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
				AddLogEntry(L"### Failed to set data in CWardwizUIDlg::SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in CWardwizUIDlg::SendData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : IsAnyChangeInDate
Description    : Function which checks is any changes happening in system date by user
Author Name    : Ramkrushna Shelke
Date           : 7th Feb 2017
SR_NO		   :
***********************************************************************************************/
bool CWardWizUIDlg::IsAnyChangeInDate()
{
	__try
	{
		static SYSTEMTIME	lastTime = { 0 };
		SYSTEMTIME	stCurrTime = { 0 };
		GetSystemTime(&stCurrTime);

		if (lastTime.wDay != stCurrTime.wDay || lastTime.wMonth != stCurrTime.wMonth || lastTime.wYear != stCurrTime.wYear)
		{
			lastTime = stCurrTime;
			return true;
		}
		lastTime = stCurrTime;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::IsAnyChangeInDate", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : ShowRegMessageHandler
Description    : Function to handle messages related to show registration window.
Author Name    : Jeena Mariam Saji
Date           : 26th APR 2017
SR_NO		   :
***********************************************************************************************/
LRESULT CWardWizUIDlg::ShowRegMessageHandler(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		if (theApp.m_dwDaysLeft <= 0)
		{
			ShowRegistrationDialog();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ShowRegMessageHandler", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : OnMaximiseWindow
Description    : Function to handle messages related to maximise window.
Author Name    : Jeena Mariam Saji
Date           : 27th Jan 2018
***********************************************************************************************/
LRESULT CWardWizUIDlg::OnMaximiseWindow(WPARAM wParam, LPARAM lParam)
{
	try
	{
		ModifyStyleEx(WS_EX_TOOLWINDOW, WS_EX_APPWINDOW);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::OnMaximiseWindow", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/****************************************************************************************************
*	Function Name : ShowScanPage
*	Description	  : Function to open UI Scan page.
*	Author Name   :	Amol Jaware
*	Date		  :	26/Sep/2017
*****************************************************************************************************/
void CWardWizUIDlg::ShowScanPage()
{
	try
	{
		if (m_bIsRegInProgress)
			return;
		const SCITER_VALUE result = m_root_el.call_function("CallShowScanPage");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ShowScanPage", 0, 0, true, SECONDLEVEL);
	}
}

/****************************************************************************************************
*	Function Name : ShowReports
*	Description	  : Function to open UI Report page.
*	Author Name   :	Amol Jaware
*	Date		  :	26/Sep/2017
*****************************************************************************************************/
void CWardWizUIDlg::ShowReports()
{
	try
	{
		if (m_bIsRegInProgress)
			return;
		const SCITER_VALUE result = m_root_el.call_function("CallShowReports");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ShowReports", 0, 0, true, SECONDLEVEL);
	}
}

/****************************************************************************************************
*	Function Name : SendData2Tray
*	Description	  : Function to send Active scan state to tray.
*	Author Name   :	Amol Jaware
*	Date		  :	26/Oct/2017
*****************************************************************************************************/
bool CWardWizUIDlg::SendData2Tray(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam, LPTSTR lpszSecondParam, bool bWait)
{
	DWORD dwAction = 0;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = iMessage;
		szPipeData.dwValue = dwValue;
		if (!m_objComTray.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			CString csMessage;
			csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
			AddLogEntry(L"### Failed to set data in CWardwizUIDlg::SendData2Tray, %s", csMessage, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objComTray.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csMessage;
				csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
				AddLogEntry(L"### Failed to set data in CWardwizUIDlg::SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in CWardwizUIDlg::SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : StartSheduledScan
Description    : Function to start Scheduled scan
Author Name    : Jeena Mariam Saji
SR_NO		   :
Date           : 10th Nov 2017
****************************************************************************/
bool CWardWizUIDlg::StartSheduledScan(DWORD dwScanType, DWORD dwShutDownVal)
{
	bool bReturn = false;
	try
	{
		sciter::value map;
		CString csCommandLineVal;
		csCommandLineVal.Format(L"%d", dwScanType); 
		bool bIsScanShutDown;
		if (dwShutDownVal == 1)
			bIsScanShutDown = true;
		else
			bIsScanShutDown = false;

		map.set_item("one", sciter::string(csCommandLineVal));
		map.set_item("two", sciter::value(bIsScanShutDown));
		m_root_el = root();
		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS beParams;
		beParams.cmd = SCHED_SCAN_EVENT_CODE;
		beParams.he = beParams.heTarget = ela;
		beParams.data = map;
		ela.fire_event(beParams, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::StartSheduledScan", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : StartSheduledScan4RegOpt
Description    : Function to start registry option Scheduled scan
Author Name    : Amol Jaware
SR_NO		   :
Date           : 27th March 2019
****************************************************************************/
bool CWardWizUIDlg::StartSheduledScan4RegOpt(DWORD dwScanType, DWORD dwShutDownVal, CString csRegOptList)
{
	bool bReturn = false;
	try
	{
		m_objWardWizRegistryOpt.GetRegistryOptionList(csRegOptList); //send registry options to CWardWizRegistryOpt class

		sciter::value map;
		CString csCommandLineVal;
		csCommandLineVal.Format(L"%d", dwScanType);
		bool bIsScanShutDown;
		if (dwShutDownVal == 1)
			bIsScanShutDown = true;
		else
			bIsScanShutDown = false;

		map.set_item("one", sciter::string(csCommandLineVal));
		map.set_item("two", sciter::value(bIsScanShutDown));
		m_root_el = root();
		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS beParams;
		beParams.cmd = SCHED_SCAN_EVENT_CODE;
		beParams.he = beParams.heTarget = ela;
		beParams.data = map;
		ela.fire_event(beParams, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::StartSheduledScan4RegOpt", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***********************************************************************************************
Function Name  : CallGetUIStatus
Description    : This function will check if any task is running 
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 27 November 2017
***********************************************************************************************/
json::value CWardWizUIDlg::CallGetUIStatus(SCITER_VALUE svCallGetUIStatus)
{
	try
	{
		m_svCallGetUIStatus = svCallGetUIStatus;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::CallGetUIStatus", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : CallGetUIStatus
Description    : This function will check if any task is running
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 27 November 2017
***********************************************************************************************/
bool CWardWizUIDlg::CheckForTasksRunningUI()
{
	bool bReturn = false;
	try
	{
		SCITER_VALUE result = m_svCallGetUIStatus.call();
		if (result == false)
		{
			bReturn = false;
		}
		else
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::CheckForTasksRunningUI", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SendData2Tray
*  Description    : Function which send message data to Tray application.
*  Author Name    : Amol Jaware
*  Date           : 14th Nov,2017
****************************************************************************************************/
bool CWardWizUIDlg::SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwValue;

		CISpyCommunicator objCom(TRAY_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardwizUIDlg::SendData2Tray", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CWardwizUIDlg::SendData2Tray", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : GetDBPath
Description    : This function will get Database Path
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 01 February 2018
***********************************************************************************************/
json::value CWardWizUIDlg::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBFEATURESLOCK.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::GetDBPathforSched", 0, 0, true, SECONDLEVEL);
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
json::value CWardWizUIDlg::GetDecryptPasssword(SCITER_VALUE svEncryptPasssword)
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
		AddLogEntry(L"### Exception in CWardwizUIDlg::GetDecryptPasssword", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szPassHash);
}

/***********************************************************************************************
Function Name  : GetDataBasePath
Description    : Get Database path
Author Name    : Jeena Mariam Saji
Date           : 08th June 2018
***********************************************************************************************/
json::value CWardWizUIDlg::GetDataBasePath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBPARCONTROL.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : ClosePasswordWndForPC
Description    : Close Password Popup For PC
Author Name    : Jeena Mariam Saji
Date           : 04th August 2018
***********************************************************************************************/
bool CWardWizUIDlg::ClosePasswordWndForPC()
{
	try
	{
		HWND hWindow = ::FindWindow(NULL, L"AKTRAYPWD");
		if (hWindow)
		{
			::PostMessage(hWindow, WM_CLOSE, 0, 0);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ClosePasswordWndForPC", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Jeena Mariam Saji
*  Date			  : 04 Dec 2018
****************************************************************************************************/
json::value CWardWizUIDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 1, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ReadReportsDBPath
*  Description    : Get the path for reports
*  Author Name    : Jeena Mariam Saji
*  Date			  : 30 Dec 2018
****************************************************************************************************/
json::value CWardWizUIDlg::ReadReportsDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBALLREPORTS.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : ReadValuesForHomePage
*  Description    : Get value for home page
*  Author Name    : Jeena Mariam Saji
*  Date			  : 30 Dec 2018
****************************************************************************************************/
json::value CWardWizUIDlg::ReadValuesForHomePage()
{
	try
	{
		return  m_bIsActiveProtectionON;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_CallHideUIDialog
*  Description    : To hide UI Dialog
*  Author Name    : Jeena Mariam Saji
*  Date			  : 30 Jan 2019
****************************************************************************************************/
json::value CWardWizUIDlg::On_CallHideUIDialog()
{
	try
	{
		SetWindowLongA(this->get_hwnd(), WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
		this->ShowWindow(SW_HIDE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_CallHideUIDialog", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/**********************************************************************************************
*  Function Name  : On_GetRegVal
*  Description    : To Read Registry value of Toggle buttons on Home
*  Author Name    : Swapnil Bhave
*  Date			  : 09 May 2019
***********************************************************************************************/
json::value CWardWizUIDlg::On_GetRegVal(SCITER_VALUE svValueName)
{
	SCITER_VALUE svToggleFlag;
	try
	{
		DWORD dwData = 0x00;
		CITinRegWrapper objReg;
		sciter::string sarg1 = svValueName.get(L"");
		CString ValueName = sarg1.c_str();
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), ValueName.GetBuffer(), dwData) != 0x00)
		{
			AddLogEntry(L"### GetRegistryDWORDData failed in CWardwizUIDlg::On_GetRegVal()", 0, 0, true, SECONDLEVEL);
		}
		else
		{
			if (dwData == 1)
			{
				svToggleFlag = 1;
			}
			else
			{
				svToggleFlag = 0;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_GetRegVal()", 0, 0, true, SECONDLEVEL);
	}
	return svToggleFlag;
}
/**********************************************************************************************
*  Function Name  : On_SetRegVal
*  Description    : To Set Values in Registry according to toggle button on Home
*  Author Name    : Swapnil Bhave
*  Date			  : 09 May 2019
*  Updated by	  : Akshay Patil
*  Date			  : 13 Aug 2019
***********************************************************************************************/
json::value CWardWizUIDlg::On_SetRegVal(SCITER_VALUE svValueName)
{
	sciter::string sarg1 = svValueName.get(L"");
	CString ValueName = sarg1.c_str();
	DWORD dwValueSet = 1;
	try
	{
		if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath, ValueName.GetBuffer(), REG_DWORD, dwValueSet, true))
		{
			AddLogEntry(L"### SetRegistrykeyUsingService failed in CWardwizUIDlg::On_SetRegVal()", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_SetRegVal()", 0, 0, true, SECONDLEVEL);
	}

	return json::value(0);
}
/**********************************************************************************************
*  Function Name  : On_ToggleWidget
*  Description    : This Function Turn On or Off Widget from Home Page Toggle Control
*  Author Name    : Swapnil Bhave
*  Date			  : 22 May 2019
***********************************************************************************************/
json::value CWardWizUIDlg::On_ToggleWidget(SCITER_VALUE bToggleState)
{
	DWORD dwValueSet = (DWORD)bToggleState.get(1);
	try
	{
		if (dwValueSet == 0x00)
		{
			if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath, L"dwWidgetsUIState", REG_DWORD, dwValueSet, true))
			{
				AddLogEntry(L"### SetRegistrykeyUsingService failed in CWardwizUIDlg::On_ToggleWidget(0)", 0, 0, true, SECONDLEVEL);
			}
			if (!SendData2Tray(RELOAD_WIDGETS_UI, WIDGET_HIDE_UI))
			{
				AddLogEntry(L"### Exception in CWardwizSettings::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
			}
		}
		else if (dwValueSet == 0x01)
		{
			if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath, L"dwWidgetsUIState", REG_DWORD, dwValueSet, true))
			{
				AddLogEntry(L"### SetRegistrykeyUsingService failed in CWardwizUIDlg::On_ToggleWidget(1)", 0, 0, true, SECONDLEVEL);
			}
			if (!SendData2Tray(RELOAD_WIDGETS_UI, WIDGET_SHOW_UI))
			{
				AddLogEntry(L"### Exception in CWardwizSettings::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
			}
		}
		else
		{
			AddLogEntry(L"### CWardwizUIDlg::On_ToggleWidget()", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************
*  Function Name  : ReadCurrentValues
*  Description    : Reads all registry entries
*  Author Name    : Akshay Patil
*  Date			  : 13 Aug 2019
***********************************************************************************************/
json::value CWardWizUIDlg::ReadCurrentValues(SCITER_VALUE svFunCurrentValueCB)
{
	try
	{
		ReadCurrentRegValues(svFunCurrentValueCB);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::ReadCurrentValue", 0, 0, true, SECONDLEVEL);
	}

	return json::value(0);
}

/**********************************************************************************************
*  Function Name  : ReadCurrentRegValues
*  Description    : Read current home page toggler values from Registry
*  Author Name    : Akshay Patil
*  Date			  : 13 Aug 2019
***********************************************************************************************/
void CWardWizUIDlg::ReadCurrentRegValues(SCITER_VALUE svFunCurrentValueCB)
{
	try
	{
		CString strKey;
		CITinRegWrapper objReg;

		for (DWORD dwValue = 1; dwValue <= 4; dwValue++)
		{
			switch (dwValue)
			{
				case 1:
					strKey = L"dwFirewallEnableState";
					break;
				case 2:
					strKey = L"dwEmailScanState";
					break;
				case 3:
					strKey = L"dwParentalCntrlFlg";
					break;
				case 4:
					strKey = L"dwBrowserSecurityState";
					break;
				default:
					break;
			}

			DWORD dwReturnDWORDValue = 0x00;
			if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), strKey.GetBuffer(), dwReturnDWORDValue) != 0x00)
			{
				AddLogEntry(L"### GetRegistryDWORDData failed in CWardwizUIDlg::ReadCurrentRegValues", 0, 0, true, SECONDLEVEL);
			}

			CString csKey = L"";
			csKey.Format(L"%d", dwValue);
			CString csValue = L"";
			csValue.Format(L"%d", dwReturnDWORDValue);
			svFunCurrentValueCB.call((SCITER_STRING)csKey, (SCITER_STRING)csValue);
		}		
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizUIDlg::ReadCurrentRegValues", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :	On_ClickRemoteAssistance
*  Description    :	Invoking Remote Assistance.
*  Author Name    : Kunal Waghmare
*  Date           : 30 Aug 2019
****************************************************************************************************/
json::value CWardWizUIDlg::On_ClickRemoteAssistance()
{
	try
	{
		CString csUIFilePath;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (CheckIsAppRunning(L"WardWiz Remote Assistance"))
		{
			return 0;
		}
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L"### Excpetion for GetModulePath() in CWardwizUIDlg::On_ClickRemoteAssistance", 0, 0, true, SECONDLEVEL);
			return 0;
		}
		csUIFilePath = szModulePath;
		csUIFilePath += L"\\VBRMTAST.EXE";

		ShellExecute(NULL, L"open", csUIFilePath, NULL, NULL, SW_SHOW);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizUIDlg::On_ClickRemoteAssistance", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	CheckAppIsRunning
*  Description    :	Check Application is running or not
*  Author Name    : Kunal Waghmare
*  Date           : 30 Aug 2019
****************************************************************************************************/
bool CWardWizUIDlg::CheckIsAppRunning(LPTSTR lpStr)
{
	try{
		if (lpStr == NULL)
			return false;
		HWND hWindow = ::FindWindow(NULL, lpStr);
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			return true;
		}
	}
	catch (...)
	{ 
		AddLogEntry(L"### Excpetion in CWardwizUIDlg::CheckIsAppRunning", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************
*  Function Name  : FetchReportValue
*  Description    : To read value from DB for Reports
*  Author Name    : Jeena Mariam Saji
*  Date			  : 14 Oct 2019
***********************************************************************************************/
json::value CWardWizUIDlg::FetchReportValue(SCITER_VALUE svFunUpdateReportsValue)
{
	try
	{
		TCHAR	szPath[512] = { 0 };
		CString csTotalScanned;
		CString csThreatsFound;
		CString csThreatsCleaned;
		CString csURLsBlocked;
		DWORD	dwSize = sizeof(szPath);
		CITinRegWrapper	objReg;
		CWardWizSQLiteDatabase dbSQlite;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"AppFolder", szPath, dwSize);
		CString csWardWizModulePath = szPath;
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBALLREPORTS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("SELECT SUM(db_TotalFilesScanned) FROM wardwiz_ScanSessionDetails");
		char szTotalItemsScanned[10] = { 0 };
		if (qResult.GetFieldIsNull(0))
		{
			csTotalScanned = L"0";
		}
		else
		{
			strcpy(szTotalItemsScanned, qResult.GetFieldValue(0));
			csTotalScanned = (CString)szTotalItemsScanned;
		}
		CWardwizSQLiteTable qResult1 = dbSQlite.GetTable("SELECT SUM(db_TotalThreatsFound) FROM wardwiz_ScanSessionDetails");
		char szTotalThreatsFound[10] = { 0 };
		if (qResult1.GetFieldIsNull(0))
		{
			csThreatsFound = L"0";
		}
		else
		{
			strcpy(szTotalThreatsFound, qResult1.GetFieldValue(0));
			csThreatsFound = (CString)szTotalThreatsFound;
		}

		CWardwizSQLiteTable qResult2 = dbSQlite.GetTable("SELECT SUM(db_TotalThreatsCleaned) FROM wardwiz_ScanSessionDetails");
		char szTotalThreatsCleaned[10] = { 0 };
		if (qResult2.GetFieldIsNull(0))
		{
			csThreatsCleaned = L"0";
		}
		else
		{
			strcpy(szTotalThreatsCleaned, qResult2.GetFieldValue(0));
			csThreatsCleaned = (CString)szTotalThreatsCleaned;
		}

		CWardwizSQLiteTable qResult3 = dbSQlite.GetTable("SELECT count(*) FROM Wardwiz_Browser_Sec where WebCategory = 122 OR WebCategory = 125");
		char szTotalURLBlocked[10] = { 0 };
		if (qResult3.GetFieldIsNull(0))
		{
			csURLsBlocked = L"0";
		}
		else
		{
			strcpy(szTotalURLBlocked, qResult3.GetFieldValue(0));
			csURLsBlocked = (CString)szTotalURLBlocked;
		}
		dbSQlite.Close();

		svFunUpdateReportsValue.call((SCITER_STRING)csTotalScanned, (SCITER_STRING)csThreatsFound, (SCITER_STRING)csThreatsCleaned, (SCITER_STRING)csURLsBlocked);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::FetchReportValue", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : On_IsCheckOfline
Description    : Function to check ofline registration.
Author Name    : Nitin Shelar
Date           : 03/10/2019
***********************************************************************************************/
json::value CWardWizUIDlg::On_IsCheckOfline()
{
	bool RetVal = false;
	try
	{
		CITinRegWrapper objReg;
		DWORD dwOffLineValue = 0;
		CString csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();

		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csProdRegKey.GetBuffer(), L"dwIsOffline", dwOffLineValue) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwIsOffline in CWardwizUIDlg::On_IsCheckOfline", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (dwOffLineValue == 0)
		{
			RetVal = true;
		}
		else
		{
			RetVal = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_IsCheckOfline()", 0, 0, true, SECONDLEVEL);
	}
	return RetVal;
}

/***********************************************************************************************
Function Name  : FunCheckInternetAccessBlock
Description    : Function to check internet access block
Author Name    : Jeena Mariam Saji
Date           : 10 Dec 2019
***********************************************************************************************/
json::value CWardWizUIDlg::FunCheckInternetAccessBlock()
{
	bool RetVal = false;
	try
	{
		if (theApp.m_dwProductID == BASIC || theApp.m_dwProductID == ESSENTIAL)
		{
			return false;
		}

		CITinRegWrapper objReg;
		DWORD dwParentalControl = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParentalControl) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CWardwizUIDlg::FunCheckInternetAccessBlock", 0, 0, true, SECONDLEVEL);
		}

		if (dwParentalControl == 1)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = ON_CHECK_INTERNET_ACCESS;

			CISpyCommunicator objCom(SERVICE_SERVER);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send Data in CWardwizUIDlg::SendData", 0, 0, true, SECONDLEVEL);
			}

			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read Data in CWardwizUIDlg::ReadData", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizUIDlg::FunCheckInternetAccessBlock()", 0, 0, true, SECONDLEVEL);
	}
	return RetVal;
}