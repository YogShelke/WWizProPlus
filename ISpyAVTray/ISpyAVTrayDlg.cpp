/*******************************************************************************************
*  Program Name: ISpyAVTrayDlg.cpp                                                                                                  
*  Description:  Main system tray dialog 
*  Author Name:  1)Ramkrushna
*				
*  Date Of Creation:15-Nov-2013                                                                                               
*  Version No:    1.0.0.2                                                                                                        
*
*  Special Logic Used:                                                                                                                            *
*
*  Modification Log:                                                                                               
*  1. Modified xyz function in main        Date modified         CSR NO    
*
*          Neha Gharge                                                                                                                            *
*          Nitin Kolapkar.                                                                                                                            *
*********************************************************************************************/ 
#include "stdafx.h"
#include "ISpyAVTray.h"
#include "ISpyAVTrayDlg.h"
#include "PipeConstants.h"
#include "TrayPopUpMultiDlg.h"
#include "PopUpDialog.h"
#include "PopUpWithReminder.h"
#include "ISpyCriticalSection.h"
#include "WardWizSplashWindow.h"
#include "EnumProcess.h"
#include "WardWizTrayReleaseNotes.h"
#include "WWizDetectQuarentineDlg.h"
#include "WWizTraySucessDlg.h"
#include "PopUpRenewProduct.h"
#include "WardWizWidgetDlg.h"
#include "EmailNotifyTrayDlg.h"
#include "WardWizSchedScanMissDlg.h"
#include "WardWizFirewallBlockPopUp.h"
#include "WardWizParCtrlPopUp.h"
#include "WWizUSBScnPopup.h"
#include "Windows.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <boost/algorithm/string/split.hpp>
#include "Wtsapi32.h"
#include "WardWizPortScanPopUp.h"
#include "WWizWebFilter.h"
#include "WardWizSchTempScanDlg.h"
#include "CommonFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	WM_ICON_NOTIFY	WM_APP+10
#define	SHOWPOPUPMESSAGE	1000

DWORD WINAPI StartUsbWatchThread(LPVOID lpvThreadParam);
DWORD WINAPI StartShowingEntry(LPVOID arg);
DWORD WINAPI StartTimerOfReminder(LPVOID lpvThreadParam);
DWORD WINAPI ShowQuarentineDlgThread(LPVOID lpvThreadParam);
DWORD WINAPI ShowProductRenewOption(LPVOID lpvThreadParam);
DWORD WINAPI ShowQuarentineDlgThread4EmailScan(LPVOID lpvThreadParam);
DWORD WINAPI ShowWidgetWindowDoModalThread(LPVOID lpvThreadParamShowWnd);
DWORD WINAPI CheckParentalControlThread(LPVOID lpvParCtrlThreadParam);
void ProcessInstallQueueThread(CConCurrentQueue<std::wstring>& pObjeOnAccess);
void ProcessInstallQueueThreadSEH(CConCurrentQueue<std::wstring>& pObjeOnAccess);

CISpyCriticalSection	g_objISpyCriticalSection;
CTrayPopUpMultiDlg		g_objTrayPopUpMultiDlg;
CPopUpDialog			g_objPopUpDlg;
CPopUpWithReminder      g_objPopUpWithReminder;
CWWizDetectQuarentineDlg	g_CWWizDetectQuarentineDlg;
CEmailNotifyTrayDlg		g_CEmailNotifyTrayDlg;
// Need to public CSciterBase, public sciter::host<CISpyAVTrayDlg>

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

/***************************************************************************************************                    
*  Function Name  : CAboutDlg                                                     
*  Description    : C'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0053
*  Date           : 18 Sep 2013
****************************************************************************************************/
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0054
*  Date           : 18 Sep 2013
****************************************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0055
*  Date           : 18 Sep 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

CISpyAVTrayDlg * CISpyAVTrayDlg::m_pThis = NULL;
CISpyCommunicatorServer  g_objCommServer(TRAY_SERVER, &CISpyAVTrayDlg::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));

/***************************************************************************************************                    
*  Function Name  : CISpyAVTrayDlg                                                     
*  Description    : C'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0056
*  Date           : 18 Sep 2013
****************************************************************************************************/
CISpyAVTrayDlg::CISpyAVTrayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CISpyAVTrayDlg::IDD, pParent)
	, m_objParentalCntrlServer(TRAY_SERVER, &CISpyAVTrayDlg::OnDataReceiveCallBackParCtrl, sizeof(ISPY_PIPE_DATA))
	, m_hLibrary(NULL)
	, m_pszThreatName(NULL)
	, m_pszAttachmentName(NULL)
	, m_pszSenderAddr(NULL)
	, m_dwAction(0)
	, m_bFlag(true)
	, m_dwTimeReminder(0)
	, m_hThreadRemider(NULL)
	, m_objReleaseNotesDlg(NULL)
	, m_hThreadDetectThreat(NULL)
	, m_pWardWizWidgetDlg(NULL)
	, m_hWidgetsUIThread(NULL)
, m_bIsIntProcessON(false)
, m_hThreadDetectEmailScanThreat(NULL)
, m_IsRestartLaterActive(false)
, m_objCom(L"\\\\.\\pipe\\{AA7D87ADE4A34ac0A23D78E2D83F6058}")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pThis = this;
}

/***************************************************************************************************                    
*  Function Name  : ~CISpyAVTrayDlg                                                     
*  Description    : D'tor
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0057
*  Date           : 18 Sep 2013
****************************************************************************************************/
CISpyAVTrayDlg::~CISpyAVTrayDlg(void)
{
	if(m_hLibrary != NULL)
	{
		FreeLibrary(m_hLibrary);
		m_hLibrary = NULL;
	}
	if(m_hThreadRemider != NULL)
	{
		::SuspendThread(m_hThreadRemider);
		::TerminateThread(m_hThreadRemider,0x00);
		m_hThreadRemider = NULL;
	}
	/*if(m_hEvtStop)
	{
		CloseHandle(m_hEvtStop);
		m_hEvtStop = NULL;
	}*/
	//Varada Ikhar, Date: 8th May-2015
	//New Implementation : To Show 'Release Information' after successful product update.
	if (m_objReleaseNotesDlg)
	{
		delete m_objReleaseNotesDlg;
		m_objReleaseNotesDlg = NULL;
	}

	if (m_pWardWizWidgetDlg != NULL)
	{
		delete m_pWardWizWidgetDlg;
		m_pWardWizWidgetDlg = NULL;
	}

	StopInstallationProcessQueue();
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0058
*  Date           : 18 Sep 2013
****************************************************************************************************/
void CISpyAVTrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0059
*  Date           : 18 Sep 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CISpyAVTrayDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_WTSSESSION_CHANGE()
	ON_WM_ENDSESSION()
END_MESSAGE_MAP()


// CISpyAVTrayDlg message handlers
/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft Foundation Class Library dialog boxes
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0060
*  Date           : 18 Sep 2013
*  Updated By	  : Kunal Waghmare 
*  Date           : 3 July 2018
****************************************************************************************************/
BOOL CISpyAVTrayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	/*ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
	pSysMenu->AppendMenu(MF_SEPARATOR);
	pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}
	}*/

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	WTSRegisterSessionNotification(this->m_hWnd, NOTIFY_FOR_ALL_SESSIONS);

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ModifyStyleEx( WS_EX_APPWINDOW, WS_EX_TOOLWINDOW );

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER);

	HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));  // Icon to use
	GetProductID();
	CString csProdInfo;
	csProdInfo.Format(L"%s\n", m_csInstalledEdition);
	if (CheckProductInfo(csProdInfo) == false)
	{
		GetRegisteredUserInfo();
		csProdInfo.AppendFormat(L"%s: %s\n%s: %s\n%s: %s",
			theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_PROD_VER"), m_csRegProductVer,
			theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_DB_VER"), m_csRegDataBaseVer,
			theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_LAST_UPDATE"), m_csUpdateDate);
		//theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_THREAT_COUNT"), m_csThreatDefCount);
	}
	// Until it creates the icon inside the tray, keep on creating it
	try
	{
		bool bISCreated = false;
		DWORD dwRetry = 0;
		while (dwRetry < 3)
		{
			if (!m_TrayIcon.Create(
				NULL,                            // Let icon deal with its own messages
				WM_ICON_NOTIFY,                  // Icon notify message to use
				csProdInfo,  // tooltip
				hIcon,
				0,                  // ID of tray icon
				FALSE,
				0, // balloon tip
				_T("Welcome"),               // balloon title
				NIIF_INFO,                    // balloon icon
				20))                            // balloon timeout
			{
				AddLogEntry(L">>> m_TrayIcon.Create failed retrying..", 0, 0, true, SECONDLEVEL);
				dwRetry++;
				Sleep(1000);//Wait here for 1 sec
			}
			else
			{
				bISCreated = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L">>> m_TrayIcon.Create failed retrying..", 0, 0, true, SECONDLEVEL);
	}

	RefreshSysTrayIcon();
	AddLogEntry(L">>> In CWardwizTrayDlg OnInitDialog, before g_objCommServer.Run()", 0, 0, true, SECONDLEVEL);
	g_objCommServer.Run();

	//start parental control server here
	CString csUserVal;
	csUserVal = GetCurrentUserName();
	csUserVal.MakeLower();
	m_csCurrentUser = TRAY_SERVER + csUserVal;
	m_objParentalCntrlServer.SetPipeName(m_csCurrentUser.GetBuffer());
	m_objParentalCntrlServer.Run();

	m_TrayIcon.SetMenuDefaultItem(0, TRUE);
	if (!m_TrayIcon.ShowIcon())
	{
		AddLogEntry(L"### Failed to ShowIcon in CWardwizTrayDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
		
	DWORD m_dwThreadId = 0;
	m_hThread = NULL;
	m_hThread = ::CreateThread(NULL, 0, StartUsbWatchThread, (LPVOID) this, 0, &m_dwThreadId);

	ZeroMemory(m_szAllUserPath, sizeof(m_szAllUserPath) );
	GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, 511);

	//SetTimer(SHOWPOPUPMESSAGE, 1000 * 1, NULL);

	//m_TrayIcon.SetTooltipText(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"));
	m_ThreadMultiPopUp = NULL;
	m_hThreadShowPopUp= NULL;
	m_hThreadShowRenewOption = NULL;

	m_bFlag = true;

	CString	csWardWizModulePath = GetWardWizPathFromRegistry();
	CString	csWardWizReportsPath = L"";
	csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
	CT2A dbPath(csWardWizReportsPath, CP_UTF8);
	
	//Check if Reports DB file does not exists then create new repots DB.
	 //if (!PathFileExists(csWardWizReportsPath))
	 { 
		//Create SQLite tables if not created.
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
		objSqlDb.Open();
		objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
		objSqlDb.Close();
	 }

	DWORD m_dwThreadShowRenewOption = 0;
	m_dwNoofDays = theApp.m_dwDaysLeft;
	
	if (m_dwNoofDays == 0 && m_hThreadShowRenewOption == NULL && theApp.m_bIsShowRegTrue == true)
	{
		m_hThreadShowRenewOption = CreateThread(NULL, 0, ShowProductRenewOption, (LPVOID)this, 0, &m_dwThreadShowRenewOption);
	}
	/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */
	//CheckRegistryForUpdate();

	InsertUserNametoINI();
	//insert login event for event
	DWORD dwRegEntryForTray = 0;
	dwRegEntryForTray = ReadRegistryEntry(L"dwParCtrlActiveFlag");
	if (dwRegEntryForTray != 1)
	{
		InsertParCtrlReport(0);
	}

	DWORD dwShowThreadID = 0X00;
	m_hWidgetsUIThread = ::CreateThread(NULL, 0, ShowWidgetWindowDoModalThread, (LPVOID) this, 0, &dwShowThreadID);

	if (theApp.m_dwProductID == ESSENTIALPLUS)
	{
		DWORD dwPCtrlThreadID = 0X00;
		m_hParentalCtrlThread = ::CreateThread(NULL, 0, CheckParentalControlThread, (LPVOID) this, 0, &dwPCtrlThreadID);
	}
	theApp.m_dwShowTrayPopup = ReadRegistryEntry(L"dwShowTrayPopup");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************************************                    
*  Function Name  : OnSysCommand                                                     
*  Description    : The framework calls this member function when the user selects a command from the Control menu, or when the user selects the Maximize or the Minimize button.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0061
*  Date           : 18 Sep 2013
****************************************************************************************************/
void CISpyAVTrayDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
/***************************************************************************************************                    
*  Function Name  : OnPaint                                                     
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0062
*  Date           : 18 Sep 2013
****************************************************************************************************/
void CISpyAVTrayDlg::OnPaint()
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
/***************************************************************************************************                    
*  Function Name  : OnQueryDragIcon                                                     
*  Description    : The framework calls this member function by a minimized
				   (iconic) window that does not have an icon defined for its class
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0063
*  Date           : 18 Sep 2013
****************************************************************************************************/
HCURSOR CISpyAVTrayDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************************************
*  Function Name  : WidgetActiveScanState
*  Description    : Updating Active scan color state for widget.
*  Author Name    : Amol J.
*  Date           : 23 Oct 2017
****************************************************************************************************/
bool CISpyAVTrayDlg::WidgetActiveScanState(DWORD dwFlagState)
{
	try
	{
		if (m_pWardWizWidgetDlg != NULL)
		{
			m_pWardWizWidgetDlg->WidgetActiveScanColorState(dwFlagState);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::WidgetActiveScanState", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : ShowWidgetWindowDoModalThread
*  Description    : Thread which create object of CWardWizWidgetDlg class. 
*  Author Name    : Amol Jaware
*  Date           : 27 Sep 2017
****************************************************************************************************/
DWORD WINAPI ShowWidgetWindowDoModalThread(LPVOID lpvShowWidgetThreadParam)
{
	__try
	{ 
		CISpyAVTrayDlg *pThis = (CISpyAVTrayDlg*)lpvShowWidgetThreadParam;
		if (!pThis)
			return 0;
	
		pThis->ShowWidgetWindow();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizTrayDlg::ShowWidgetWindowDoModalThread", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : CheckParentalControlThread
*  Description    : Thread which checks for parental control restrictions
*  Author Name    : Jeena Mariam Saji
*  Date           : 10 July 2019
****************************************************************************************************/
DWORD WINAPI CheckParentalControlThread(LPVOID lpvParCtrlThreadParam)
{
	__try
	{
		CISpyAVTrayDlg *pThis = (CISpyAVTrayDlg*)lpvParCtrlThreadParam;
		if (!pThis)
			return 0;

		pThis->ReloadUserNameList();
		pThis->CheckParCtrlPermission();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizTrayDlg::CheckParentalControlThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ShowWidgetWindow
*  Description    : Thread which create object of CWardWizWidgetDlg class.
*  Author Name    : Amol Jaware
*  Date           : 27 Sep 2017
****************************************************************************************************/
void CISpyAVTrayDlg::ShowWidgetWindow()
{
	try
	{
		if (m_pWardWizWidgetDlg != NULL)
		{
			delete m_pWardWizWidgetDlg;
			m_pWardWizWidgetDlg = NULL;
		}

		if (m_pWardWizWidgetDlg == NULL)
		{
			m_pWardWizWidgetDlg = new CWardWizWidgetDlg();
			m_pWardWizWidgetDlg->DoModal();
			m_pWardWizWidgetDlg = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ShowWidgetWindow", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : ReloadUserNameList
*  Description    : Function calls to reload user list for parental control
*  Author Name    : Jeena Mariam Saji
*  Date           : 11 July 2019
****************************************************************************************************/
void CISpyAVTrayDlg::ReloadUserNameList()
{
	try
	{
		if (!SendData2Service(RELOAD_USER_LIST, 0x00))
		{
			AddLogEntry(L"###  Failed to send data in CWardwizTrayDlg::ReloadUserNameList SendData2Service::RELOAD_USER_LIST", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ReloadUserNameList", 0, 0, true, SECONDLEVEL);
	}
}
/***************************************************************************************************
*  Function Name  : CheckParCtrlPermission
*  Description    : Checks for parental control restrictions
*  Author Name    : Jeena Mariam Saji
*  Date           : 10 July 2019
****************************************************************************************************/
void CISpyAVTrayDlg::CheckParCtrlPermission()
{
	try
	{
		InsertUserNametoINI();
		if (!SendData2Service(SHOW_PC_LOCK_WND, 0x00))
		{
			AddLogEntry(L"###  Failed to send data in CWardwizTrayDlg::CheckParCtrlPermission SendData2Service::SHOW_PC_LOCK_WND", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(ON_INET_RESTRICTION, 0x00))
		{
			AddLogEntry(L"###  Failed to send data in CWardwizTrayDlg::CheckParCtrlPermission SendData2Service::ON_INET_RESTRICTION", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(ON_PC_INET_RESTRICT, 0x00))
		{
			AddLogEntry(L"###  Failed to send data in CWardwizTrayDlg::CheckParCtrlPermission SendData2Service::ON_PC_INET_RESTRICT", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ShowWidgetWindow", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : ShowPasswordWindow
*  Description    : Thread which create object of CWardWizWidgetDlg class.
*  Author Name    :
*  Date           :
****************************************************************************************************/
bool CISpyAVTrayDlg::ShowPasswordWindow()
{
	try
	{
		CWardWizRestartClient objWardWizRestartClient(this);
		objWardWizRestartClient.m_iTrayMsgType = WARNING_CLIENT_RESTART;
		int iRet = (int)objWardWizRestartClient.DoModal();
		if (iRet != IDOK)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ShowPasswordWindow", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : StartUsbWatchThread                                                     
*  Description    : Thread which calls StartUSBDetectWatch function
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0064
*  Date           : 18 Sep 2013
****************************************************************************************************/
DWORD WINAPI StartUsbWatchThread(LPVOID lpvThreadParam)
{
	CISpyAVTrayDlg *pThis = (CISpyAVTrayDlg*)lpvThreadParam;
	if(!pThis)
	{
		return 0;
	}
	pThis->StartUSBDetectWatch();
	return 1;
}

/***************************************************************************************************                    
*  Function Name  : StartUSBDetectWatch                                                     
*  Description    : Load VBUSBDETECT.dll and call getproc of DetectUSB and ResetUSBVariable
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0065
*  Date           : 18 Sep 2013
****************************************************************************************************/
bool CISpyAVTrayDlg::StartUSBDetectWatch()
{
	//ISSue No :429 Neha Gharge 28/5/2014 **************************************/
	bool bReturn = false;
	try
	{
		if(m_hLibrary == NULL)
		{
			AddLogEntry(L">>> Before loading Library",0,0,true,ZEROLEVEL);
			m_hLibrary = LoadLibrary(TEXT("VBUSBDETECT.DLL"));
			
			if(m_hLibrary==NULL)
			{
				AddLogEntry(L"### Failed to loading Library VBUSBDETECT.DLL",0,0,true,SECONDLEVEL);
				return false;
			}
			else if(m_hLibrary!=NULL)
			{
				m_StartDialog = (DetectUSBFunc) GetProcAddress(m_hLibrary, "DetectUSB");
				m_ResetUSBVariable = (RESETUSBVARIABLE)GetProcAddress(m_hLibrary, "ResetUSBVariable");
				//m_fpISRemovableDevice = (FPISREMOVABLEDEVICE)GetProcAddress(m_hLibrary, "ISRemovableDevice");
				
				if(!m_StartDialog)
				{
					AddLogEntry(L"### Failed to m_StartDialog getproc",0,0,true,SECONDLEVEL);
					FreeLibrary(m_hLibrary);
					m_hLibrary = NULL;
					return false;
				}

				if(!m_ResetUSBVariable)
				{
					AddLogEntry(L"### Failed to m_ResetUSBVariable getproc",0,0,true,SECONDLEVEL);
					FreeLibrary(m_hLibrary);
					m_hLibrary = NULL;
					return false;
				}

				//if (!m_fpISRemovableDevice)
				//{
				//	AddLogEntry(L"### Failed to m_fpISRemovableDevice", 0, 0, true, SECONDLEVEL);
				//	FreeLibrary(m_hLibrary);
				//	m_hLibrary = NULL;
				//	return false;
				//}

				AddLogEntry(L">>> After loading Library & getproc",0,0,true,ZEROLEVEL);
				m_StartDialog();
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWatchService::StartUSBDetectWatch", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : ResetUSBVariable                                                     
*  Description    : Reset USB Variables
*  Author Name    : Neha gharge
**  SR_NO		  : WRDWIZTRAY_0066
*  Date           : 28th may 2014
****************************************************************************************************/
bool CISpyAVTrayDlg::ResetUSBVariable()
{
	bool bReturn = false;

	try
	{
		if(!m_ResetUSBVariable)
		{
			return false;
		}

		m_ResetUSBVariable();
		bReturn = true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ResetUSBControls", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : OnLButtonDblClk                                                     
*  Description    : Action to be taken on double left click on tray icon 
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0067
*  Date           : 18 Sep 2013
****************************************************************************************************/
void CISpyAVTrayDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDblClk(nFlags, point);
}

/***************************************************************************************************                    
*  Function Name  : OnTimer                                                     
*  Description    : The framework calls this member function after each interval specified in the SetTimer member function used to install a timer.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0068
*  Date           : 18 Sep 2013
****************************************************************************************************/
void CISpyAVTrayDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == SHOWPOPUPMESSAGE)
	{
		KillTimer(SHOWPOPUPMESSAGE);
				
		//CISpyCommonTrayDlg objISpyCommonTrayDlg;
		//objISpyCommonTrayDlg.m_bEmailScanTreatMsg= true;
		//objISpyCommonTrayDlg.DoModal();

	}
	CDialog::OnTimer(nIDEvent);
}

/***************************************************************************************************                    
*  Function Name  : ShowEmailScanTrayPopup                                                     
*  Description    : To add entries of detected virus
*  Author Name    : Nitin K.
*  SR_NO		  : WRDWIZTRAY_0069
*  Date           : 18th July 2014
****************************************************************************************************/
void CISpyAVTrayDlg::ShowEmailScanTrayPopup(LPCTSTR szThreatName, LPCTSTR szAttachmentName, LPCTSTR szSenderAddr, DWORD dwAction)
{
	g_objISpyCriticalSection.Lock();
	
	m_pszThreatName = szThreatName;
	m_pszAttachmentName = szAttachmentName;
	m_pszSenderAddr = szSenderAddr;
	m_dwAction = dwAction;
	DWORD m_dwThreadId = 0;
	DWORD m_dwThreadIdShowPopUp = 0;

	g_objTrayPopUpMultiDlg.m_ArrThreatName.Add(m_pszThreatName);
	g_objTrayPopUpMultiDlg.m_ArrAttachmentName.Add(m_pszAttachmentName);
	g_objTrayPopUpMultiDlg.m_ArrSenderAddress.Add(m_pszSenderAddr);
	if(dwAction == 0)
	{
		g_objTrayPopUpMultiDlg.m_ArrActionTaken.Add(L"Threat Repaired");
	}
	else if (dwAction == 1)
	{
		g_objTrayPopUpMultiDlg.m_ArrActionTaken.Add(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_THREAT_DETECTED"));
	}
	else
	{
		//Issue: We are not taking any action on infected mail so no need to show Action dialog
		//Resolved By : Nitin Kolapkar Date: 29th July
		//Issue no 1162,1050,1032,1029 The images according to status is not coming properly
		//Neha Gharge 29th Dec,2015
		g_objTrayPopUpMultiDlg.m_ArrActionTaken.Add(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_THREATS_BLOCKED"));

	}
	g_objTrayPopUpMultiDlg.m_iFinalCount++;
	//If final count is greater than 1, update the Ui with new mail inputs.
	//24/6/2015 Neha/Nitin 
	if (g_objTrayPopUpMultiDlg.m_iFinalCount>1)
	{
		//Issue: if there is only one entry then it should not show fwd & backward arrow.
		//Resolved By: Nitin K Date: 26th August 2015
		g_objTrayPopUpMultiDlg.m_btnPrevious.ShowWindow(SW_SHOW);
		g_objTrayPopUpMultiDlg.m_btnPrevious.RedrawWindow();
		g_objTrayPopUpMultiDlg.m_btnNext.ShowWindow(SW_SHOW);
		g_objTrayPopUpMultiDlg.m_btnNext.RedrawWindow();
		g_objTrayPopUpMultiDlg.m_stCurrentNo.ShowWindow(SW_SHOW);
		g_objTrayPopUpMultiDlg.m_stCurrentNo.RedrawWindow();
		g_objTrayPopUpMultiDlg.UpdateUI();

	}
	Sleep(20);

	BringWindowToTop();
	SetForegroundWindow();

	if(m_hThreadShowPopUp == NULL)
	{
		m_hThreadShowPopUp = CreateThread(NULL, 0, StartShowingEntry, (LPVOID)this, 0, &m_dwThreadIdShowPopUp);
	}
	g_objISpyCriticalSection.Unlock();
}

/***************************************************************************************************                    
*  Function Name  : DisplayUpdateMessage                                                     
*  Description    : The message to be displayed from the status given from ALU services
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0070
*  Date           : 1st Aug 2014
****************************************************************************************************/
/* ISSUE NO -  NAME - lalit Reported by neha  - 3rd July 2014 */
bool CISpyAVTrayDlg::DisplayUpdateMessage(DWORD dwTypeOfUpdateMsg, DWORD dwMessageOption)
{

	CISpyCommonTrayDlg objISpyCommonTrayDlg;
	DWORD dwCount = 0;
	TCHAR szIniFilePath[512] = {0};
	TCHAR szName[512] = {0};
	try
	{
		if (dwTypeOfUpdateMsg == 2)
		{
			//Domodal for updates failed (internet problem)
			//objISpyCommonTrayDlg.m_bLiveUpdateFailedMsg = true;
			//objISpyCommonTrayDlg.DoModal();
		}
		else if (dwTypeOfUpdateMsg == 3)
		{
			//Domodal for updates are uptodate 
			//AfxMessageBox(L"Update Up-to-date");
		}
		else
		{
			/* ISSUE NO -  NAME - lalit Reported by neha   */

			//AfxMessageBox(L"Updates sucessfully");
			//objISpyCommonTrayDlg.m_bLiveUpdateSucessfulMsg = true;
			//int iRet = objISpyCommonTrayDlg.DoModal();
			//if(iRet == IDCANCEL)
			//{
				m_csAppFolderName = GetWardWizPathFromRegistry();
				m_csAppFolderName = m_csAppFolderName.Left(m_csAppFolderName.GetLength() - 1);
				m_csAppFolderName = m_csAppFolderName.Right(m_csAppFolderName.GetLength() - (m_csAppFolderName.ReverseFind(L'\\') + 1));

				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", m_szAllUserPath, m_csAppFolderName);

				TCHAR szModulePath[MAX_PATH] = { 0 };
				if (!GetModulePath(szModulePath, MAX_PATH))
				{
					AddLogEntry(L"### Exception in CWardwizTrayDlg::DisplayUpdateMessage :: GetModulePath : %s", szModulePath, 0, true, SECONDLEVEL);

					return false;
				}
				CString csUIFilePath;
				csUIFilePath = szModulePath;


			if (!PathFileExists(szIniFilePath))
			{
				AddLogEntry(L"### ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
				dwCount = 0;
			}
			else
			{
				//Neha Gharge, Date:17-03-2015
				//Issue: If only DB is updated in live update, restart now msg does not pop-up.
				dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);

				//Commented: 
				//if(dwCount == 0)
				//{
				//	/* If DB is not valid issue Neha Gharge 1/1/2014*/
				//	dwCount = GetPrivateProfileInt(L"DB", L"Count", 0x00, szIniFilePath);
				//	if (dwCount)
				//	{
				//		dwCount = 0x01;
				//	}
				//}

			}

			if (dwCount)
			{
				//Temporary pop-up disabled

				//CPopUpWithReminder     objRemPopUpDlg;
				//if (g_objPopUpWithReminder.m_hWnd != NULL)
				//	return false;
				//DWORD dwRegEntryForTray = 0;
				//dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
				//if (dwRegEntryForTray == 1)
				//{
				//	int iRestartNowRet = static_cast<int>(g_objPopUpWithReminder.DoModal());
				//	/* ISSUE NO -  NAME - lalit Reported by neha  */
				//	//objRemPopUpDlg.SetFocus();
				//	//Varada Ikhar, Date: 11th May-2015
				//	//Crash for Bring Window to top as handle not available.
				//	if (g_objPopUpWithReminder.m_hWnd != NULL)
				//		g_objPopUpWithReminder.BringWindowToTop();

				//	if (iRestartNowRet == IDOK)
				//	{
				//		CEnumProcess enumproc;
				//		enumproc.RebootSystem(0);
				//	}
				//	else if (iRestartNowRet == IDCANCEL)
				//	{
				//		//Varada Ikhar, Date: 12th May-2015
				//		//Issue : In Tray, after product update, if for reboot confirmation pop-up, clicked on 'Remind later'. Pop-up don't apper again for remainder.
				//		m_dwTimeReminder = g_objPopUpWithReminder.m_dwRemindTime;
				//		DWORD dwThreadId = 0;
				//		m_hThreadRemider = CreateThread(NULL, 0, StartTimerOfReminder, (LPVOID) this, 0, &dwThreadId);
				//	}
				//}
			}
			else
			{
				objISpyCommonTrayDlg.m_bLiveUpdateSucessfulMsg = true;
				DWORD dwRegEntryForTray = 0;
				
				dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");

				CWWizTraySucessDlg		objWWizTraySucessDlg;
				if (dwMessageOption == 0x01)
				{
					objWWizTraySucessDlg.m_bProductUpdate = true;
				}

				if (dwRegEntryForTray == 1)
				{
					objWWizTraySucessDlg.DoModal();
				}
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::DisplayUpdateMessage", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : CompareUserName
*  Description    : To compare user name
*  Author Name    : Jeena Marim Saji
*  Date           : 04 July 2019
****************************************************************************************************/
bool CISpyAVTrayDlg::CompareUserName(LPTSTR csValue)
{
	bool bReturn = false;
	try
	{
		CString csVal;
		csVal = csValue;
		csVal.MakeLower();
		CString csUserVal;

		csUserVal = GetCurrentUserName();
		csUserVal.MakeLower();
		if (_tcscmp(csUserVal, csVal) == 0)
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::CompareUserName", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : OnDataReceiveCallBackParCtrl
*  Description    : Receive flag set from pipe
*  Author Name    : Jeena Marim Saji
*  Date           : 04 July 2019
****************************************************************************************************/
void CISpyAVTrayDlg::OnDataReceiveCallBackParCtrl(LPVOID lpParam)
{
	DWORD dwReply = 0;
	__try
	{
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		int iRetDays = 0;

		switch (lpSpyData->iMessageInfo)
		{
		case SEND_PC_TRAY_LOCK_WND:
			if (m_pThis->CompareUserName(lpSpyData->szFirstParam) && m_pThis->LaunchPasswordWndForPC(lpSpyData->szFirstParam))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SEND_PC_INET_TRAY_LOCK_WND:
			if (m_pThis->CompareUserName(lpSpyData->szFirstParam) && m_pThis->LaunchPasswordWndForPCINet())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SEND_PC_TRAY_CLOSE:
			if (m_pThis->CompareUserName(lpSpyData->szFirstParam))
			{
				if (m_pThis->ClosePasswordWndForPC())
				{
					dwReply = 1;
				}
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SHOWONACCESSPOPUP:
			dwReply = 0;
			if (m_pThis->CompareUserName(lpSpyData->szThirdParam))
			{
				if (m_pThis->AddIntoQueue(lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->dwValue))
				{
					dwReply = 1;
				}
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SHOW_APP_BLOCK_POPUP:
			if (m_pThis->LaunchAppBlockDlg(lpSpyData->szFirstParam))
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
		AddLogEntry(L"### Exception in CWardwizAVTrayDlg OnDataReceiveCallBackParCtrl", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : OnDataReceiveCallBack                                                     
*  Description    : Receive flag set from pipe
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : WRDWIZTRAY_0071
*  Date           : 18 Sep 2013
****************************************************************************************************/
void CISpyAVTrayDlg::OnDataReceiveCallBack(LPVOID lpParam)
{
	// Issue No :429 Neha gharge 28/5/2014
	DWORD dwReply = 0;
	__try
	{
		OutputDebugString(L">>> In CISpyAVTrayDlg: OnDataReceiveCallBack");
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		int iRetDays = 0;
		switch(lpSpyData->iMessageInfo)
		{
		case SHOW_EMAILSCAN_TRAY_POPUP:
			//m_pThis->ShowEmailScanTrayPopup(lpSpyData->szFirstParam,lpSpyData->szSecondParam,lpSpyData->szThirdParam,lpSpyData->dwValue);
			m_pThis->AddIntoQueueEmailScanInfo(lpSpyData->szFirstParam, lpSpyData->szSecondParam, lpSpyData->szThirdParam, lpSpyData->dwValue);
			break;
		case RESET_USBSCAN_VARIABLE:
			dwReply = 0;
			if(m_pThis->ResetUSBVariable())
			{
				dwReply = 1; 
			}
			else
			{
				AddLogEntry(L"### Failed to ResetUSBVariable", 0, 0, true, SECONDLEVEL);
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		
		case RELOAD_REGISTARTION_DAYS:
			iRetDays = theApp.GetDaysLeft();
			if (iRetDays > 0)
			{
				if (m_pThis->m_pWardWizWidgetDlg != NULL)
				{
					m_pThis->m_pWardWizWidgetDlg->GetDaysRegLeft(iRetDays);
				}
			}
			break;
		case UPDATE_FINISHED :
			m_pThis->DisplayUpdateMessage(lpSpyData->dwValue, lpSpyData->dwSecondValue);
			break;
		case UPDATENOW_POPUP:
			dwReply = 0;
			if(m_pThis->DisplayUpdateNowPopup())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		//Issue Resolved :0000178 : Issue with Tray Notification.
		//Resolved By : Nitin K
		case CLOSE_UPDATENOW_POPUP:
			dwReply = 0;
			if (m_pThis->DisplayUpdateNowPopup(true))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;	
		case SCAN_FINISHED_SIGNATUREFAILED:
			/* If DB is not valid issue Neha Gharge 1/1/2014*/
			m_pThis->DisplayUpdateNowPopup();
			break;
			//Varada Ikhar, Date: 8th May-2015
			//New Implementation : To Show 'Release Information' after successful product update.
		case SHOW_RELEASE_NOTES:
			m_pThis->ShowReleaseNotes();
			break;
		case SENDPRODUCTEXPTOTRAY:
			// In case of  dwRegUserType = offline and No of days are zero after registered it online
			// tray will popup with message as copy is not genunine 4 dec 1015 Neha Gharge
			if (!m_pThis->SetNonGenunineRegistry(theApp.m_csRegKeyPath, L"dwNGC", 0, 0x01, true))
			{
				AddLogEntry(L"### Failed to set data in CWardwizTrayDlg::SetNonGenunineRegistry", 0, 0, true, FIRSTLEVEL);
			}
			m_pThis->DisplayProductExpiryMsg();
			break;
		case SEND_SCHED_SCAN_MISSED:
			dwReply = 0;
			if (m_pThis->ShowSchedScanMissed())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case HANDLEACTICESCANSETTINGS:
			dwReply = 0;
			if (m_pThis->HandleActiveScanSettings(lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_WIDGETS_UI:
			dwReply = 0; 
			if (m_pThis->ReLoadWidgetsUISettings(lpSpyData->dwValue))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
		case EPS_INSTALL_SETUP:
			dwReply = 0;
			if (m_pThis->EPSInstallClientSetup(lpSpyData))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SHOW_PASSWORD_WIND:
			if (m_pThis->LaunchPasswordWnd())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SHOW_PORT_SCAN_POPUP:
			if (m_pThis->ShowPortScanBlockPopUp())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SEND_WEBSITE_BLK_NOTIFICATION:
			m_pThis->ShowBlockWebPopUp(lpSpyData->szFirstParam, lpSpyData->szSecondParam);
			break;
		case SEND_SCHED_TEMP_FILE_FINISHED:
			dwReply = 0;
			if (m_pThis->ShowSchedTempFileScanFinished())
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SEND_USB_AUTOSCAN:
			dwReply = 0;
			if (m_pThis->ShowUSBAutoScanPopUp(lpSpyData->dwValue, lpSpyData->szFirstParam))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case SEND_BROWSER_SECURITY_DETAILS:
			dwReply = 0;
			if (m_pThis->ShowBrowserSecurityPopUp(lpSpyData->szFirstParam, lpSpyData->szSecondParam))
			{
				dwReply = 1;
			}
			lpSpyData->dwValue = dwReply;
			g_objCommServer.SendResponse(lpSpyData);
			break;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizAVTrayDlg OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ShowBlockWebPopUp()
*  Description    : To show and send data to web block pop-up
*  Author Name    : Nitin Shelar
*  Date           :	15/07/2019
****************************************************************************************************/
int CISpyAVTrayDlg::ShowBlockWebPopUp(LPTSTR lpszWebUrl, LPTSTR lpszWebCategory)
{
	try
	{
		if (lpszWebUrl == NULL || lpszWebCategory == NULL)
			return 0;

		if (_tcslen(lpszWebUrl) - 1 >= MAX_PATH)
		{
			return 0;
		}

		if (theApp.m_dwShowTrayPopup == 1)
		{
			HWND hWndWeb = ::FindWindow(NULL, L"AKTRAYWBBLOCKDLG");
			if (hWndWeb == NULL)
			{
				CWWizWebFilterDlg WebFilterObj;
				WebFilterObj.m_csUrl = CString(lpszWebUrl);
				WebFilterObj.m_csCategory = CString(lpszWebCategory);
				WebFilterObj.DoModal();
			}
			else
			{
				STWEBCAT szWebCategories = { 0 };
				_tcscpy(szWebCategories.szURL, lpszWebUrl);
				_tcscpy(szWebCategories.szCategory, lpszWebCategory);

				::ShowWindow(hWndWeb, SW_SHOW);
				::SendMessage(hWndWeb, WM_MESSAGESHOWCONTENT, 0, (LPARAM)&szWebCategories);
			}
		}
	 }
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAVTrayDlg::ShowBlockWebPopUp", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  : StartShowingEntry
*  Description    : To show the messages of virus detected
*  Author Name    : Nitin K.
*  SR_NO		  : WRDWIZTRAY_0072
*  Date           : 18th July 2014
****************************************************************************************************/
DWORD WINAPI StartShowingEntry(LPVOID arg)
{
	try
	{
		CISpyAVTrayDlg *pThis = (CISpyAVTrayDlg*)arg;
		if (!pThis)
			return 0;
		DWORD dwRegEntryForTray = 0;
		dwRegEntryForTray = pThis->ReadRegistryEntry(L"dwShowTrayPopup");
		if (dwRegEntryForTray == 1)
		{
			g_objTrayPopUpMultiDlg.DoModal();
		}

		pThis->ResetVariables();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAVTrayDlg::StartShowingEntry", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***************************************************************************************************
*  Function Name  : ShowQuarentineDlgThread
*  Description    : To do dialog operations(DoModal & Reinitialize) 
*  Author Name    : Yogeshwar Rasal
*  Date           :	 30 July 2016
****************************************************************************************************/
DWORD WINAPI ShowQuarentineDlgThread(LPVOID lpvThreadParam)
{
	try
	{
		CISpyAVTrayDlg *pThis = (CISpyAVTrayDlg*)lpvThreadParam;

		if (!pThis)
			return 0;
		pThis->LaunchDialog();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAVTrayDlg::ShowQuarentineDlgThread", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}
void CISpyAVTrayDlg::LaunchDialog()
{

	DWORD dwRegEntryForTray = 0;
	dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
	if (dwRegEntryForTray == 1)
	{
		g_CWWizDetectQuarentineDlg.m_bIsCloseClicked = false;
		g_CWWizDetectQuarentineDlg.DoModal();	
	}
	ReInitializeQurentineDlg();
}

/***************************************************************************************************                    
*  Function Name  : ShowQuarentineDlgThread4EmailScan
*  Description    : To do dialog operations(DoModal & Reinitialize)
*  Author Name    : Amol Jaware
*  Date           :	27 June 2018
****************************************************************************************************/
DWORD WINAPI ShowQuarentineDlgThread4EmailScan(LPVOID lpvThreadParam)
{
	try
	{
		CISpyAVTrayDlg *pThis = (CISpyAVTrayDlg*)lpvThreadParam;

		if (!pThis)
			return 0;
		pThis->LaunchEmailTotifyTrayDialog();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAVTrayDlg::ShowQuarentineDlgThread4EmailScan", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***************************************************************************************************
*  Function Name  : LaunchEmailTotifyTrayDialog
*  Description    : Launch Email Scan Tray dialog.
*  Author Name    : Amol Jaware
*  Date           :	27 June 2018
****************************************************************************************************/
void CISpyAVTrayDlg::LaunchEmailTotifyTrayDialog()
{
	try
	{

		DWORD dwRegEntryForTray = 0;
		dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
		if (dwRegEntryForTray == 1)
		{
			g_CEmailNotifyTrayDlg.m_bIsCloseClicked = false;
			g_CEmailNotifyTrayDlg.DoModal();
		}

		ReInitializeQurentineDlg4EmailScan();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::LaunchEmailTotifyTrayDialog", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ResetVariables                                                     
*  Description    : To RESET THE HANDLE
*  Author Name    : Nitin K.
*  SR_NO		  : WRDWIZTRAY_0073
*  Date           : 18th July 2014
****************************************************************************************************/
void CISpyAVTrayDlg::ResetVariables()
{
	if(m_hThreadShowPopUp != NULL)
	{
		m_hThreadShowPopUp= NULL;
	}
}

/***********************************************************************************************
  Function Name  : CheckRegistryForUpdate
  Description    : this function is  called for Reading values from Registry 
  Author Name    : Nitin Kolapkar
  Date           : 23rd July 2014
*  SR_NO		  : WRDWIZTRAY_0074
**********************************************************************************************/
void CISpyAVTrayDlg :: CheckRegistryForUpdate()
{
	DWORD dwRegEntryForUpdate = 0;
	dwRegEntryForUpdate =	ReadRegistryEntry(L"dwAutoProductUpdate");
	if(dwRegEntryForUpdate == 1)
	{
		//g_objPopUpDlg.DoModal();
		// g_objRemPopUpDlg.DoModal();

	}
}

/***************************************************************************************************                    
*  Function Name  : ReadRegistryEntry                                                     
*  Description    : this function is  called for Reading values from Registry
*  Author Name    : Nitin Kolapkar
*  SR_NO		  : WRDWIZTRAY_0075
*  Date           : 23rd July 2014
****************************************************************************************************/
DWORD CISpyAVTrayDlg::ReadRegistryEntry(CString strKey)
{
	DWORD dwType = REG_DWORD;
	DWORD dwRetValue = 0;

	DWORD dwSize = sizeof(dwRetValue);
	HKEY hKey;
	m_Key = theApp.m_csRegKeyPath;

	LONG lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, LPCTSTR(m_Key), 0,KEY_READ, &hKey);
	if(lResult == ERROR_SUCCESS)
		lResult = ::RegQueryValueEx(hKey, strKey, NULL,&dwType, (LPBYTE)&dwRetValue, &dwSize);

	if(lResult == ERROR_SUCCESS)
	{	
		::RegCloseKey(hKey);
		return dwRetValue;	
	}
	else 
	{
		::RegCloseKey(hKey);
		return 0;
	}

	return 0;
}

/*************************************************************************************************** 
*  Function Name  : DisplayUpdateNowPopup                                                     
*  Description    : Show Update Now PopUp
*  Author Name    : Neha Gharge
*  SR_NO		  : WRDWIZTRAY_0077
*  Date           : 14th August 2014
****************************************************************************************************/
bool CISpyAVTrayDlg::DisplayUpdateNowPopup(bool bRequestToClose)
{
	//Issue: UpdateNowPopUp Dialog is already Domodaled and If user start updates from main UI then need to close this PopupDialog
	//Issue Resolved :0000178 : Issue with Tray Notification.
	//Resolved By : Nitin K
	if (bRequestToClose == true)
	{
		if (g_objPopUpDlg.m_hWnd != NULL)
		{
			g_objPopUpDlg.OnClosingGetCalled();
		}
		return true;
	}

	// Issue :Crash on when Update Now popup already exiting and again pop up notifcation comes
	// Neha Gharge 11-5-2015

	if (g_objPopUpDlg.m_hWnd != NULL)
		return true;
	DWORD dwRegEntryForTray = 0;
	dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
	if (dwRegEntryForTray == 1)
	{
		int Ret = static_cast<int>(g_objPopUpDlg.DoModal());
		if(Ret == IDOK)
		{
			return true;
		}
		else if(Ret == IDCANCEL)
		{
			return true;
		}
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : StartTimerOfReminder                                                     
*  Description    : Wait for remainder time.
*  Author Name    : Neha Gharge
*  SR_NO		  : WRDWIZTRAY_0078
*  Date           : 14th August 2014
****************************************************************************************************/
DWORD WINAPI StartTimerOfReminder(LPVOID lpvThreadParam)
{
	try
	{
		CISpyAVTrayDlg *pThis = (CISpyAVTrayDlg*)lpvThreadParam;
		if (!pThis)
		{
			return 0;
		}
		while (true)
		{
			DWORD dwWaitTime = 3 * 60 * 60 * 1000;
			if (pThis->m_dwTimeReminder == 1)
			{
				dwWaitTime = 10 * 60 * 1000;//10 mins
			}
			else if (pThis->m_dwTimeReminder == 2)
			{
				dwWaitTime = 60 * 60 * 1000;//1 hour
			}
			WaitForSingleObject(pThis->m_hThreadRemider, dwWaitTime);//3 hour

			//Issue no. : 101
			//Added for AVTray crash
			if (pThis->m_dwTimeReminder)
			{
				//CPopUpWithReminder      objRemPopUpDlg;
				// to bring on top of opened file
				if (g_objPopUpWithReminder.m_hWnd != NULL)
					return 0;
				DWORD dwRegEntryForTray = 0;
				dwRegEntryForTray = pThis->ReadRegistryEntry(L"dwShowTrayPopup");
				if (dwRegEntryForTray == 1)
				{
					int iRemindret = static_cast<int>(g_objPopUpWithReminder.DoModal());
					/* ISSUE NO -  NAME - lalit Reported by neha  - 3rd July 2014 */
					//Varada Ikhar, Date: 11th May-2015
					//Crash for Bring Window to top as handle not available.
					if (g_objPopUpWithReminder.m_hWnd != NULL)
						g_objPopUpWithReminder.BringWindowToTop();

					if (iRemindret == IDOK)
					{
						CEnumProcess enumproc;
						enumproc.RebootSystem(0);
						break;
					}
					//Varada Ikhar, Date: 12th May-2015
					//Issue : In Tray, after product update, if for reboot confirmation pop-up, clicked on 'Remind later'. Pop-up don't apper again for remainder.
					if (iRemindret == IDCANCEL)
					{
						pThis->m_dwTimeReminder = g_objPopUpWithReminder.m_dwRemindTime;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WardwizAVTrayDlg::StartTimerOfReminder", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ShowReleaseNotes
*  Description    : If Release Note had not been shown Do-modal the Dialog.
*  Author Name    : Varada Ikhar
*  SR_NO		  : 
*  Date           : 6th May 2015
****************************************************************************************************/
bool CISpyAVTrayDlg::ShowReleaseNotes()
{
	try
	{
		if (m_objReleaseNotesDlg != NULL)
		{
			delete m_objReleaseNotesDlg;
			m_objReleaseNotesDlg = NULL;
		}

		m_objReleaseNotesDlg = new CWardWizTrayReleaseNotes();
		if (m_objReleaseNotesDlg != NULL)
		{
			DWORD dwRelNoteShow = 0x00;
			if (m_objReleaseNotesDlg->GetRegistryDWORDEntry(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwRelNotesShow", dwRelNoteShow) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwRelNotesShow in CWardwizTrayDlg::ShowReleaseNotes", 0, 0, true, SECONDLEVEL);;
				return false;
			}
			else
			{
				if (dwRelNoteShow == 0x01)
				{
					if (m_objReleaseNotesDlg->m_hWnd == NULL)
					{
						DWORD dwRegEntryForTray = 0;
						dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
						if (dwRegEntryForTray == 1)
						{
							m_objReleaseNotesDlg->DoModal();
						}
					}				
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ShowReleaseNotes", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ShowSchedScanMissed
*  Description    : Display ShowSchedScanMissed message.
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date           : 13th Oct 2017
****************************************************************************************************/
bool CISpyAVTrayDlg::ShowSchedScanMissed()
{
	try
	{
		CWardWizSchedScanMissDlg objWWizSchedScanMissDlg;
		DWORD dwRegEntryForTray = 0;
		dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
		if (dwRegEntryForTray == 1)
		{
		int iRet = (int)objWWizSchedScanMissDlg.DoModal();
		if (iRet == IDOK)
		{
			return true;
		}
	}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ShowSchedScanMissed", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : ShowSchedTempFileScanFinished
*  Description    : Display ShowSchedTempFileScanFinished message.
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date           : 13th Mar 2019
****************************************************************************************************/
bool CISpyAVTrayDlg::ShowSchedTempFileScanFinished()
{
	try
	{
		CWardWizSchTempScanDlg objWardWizSchTempScanDlg;

		int iRet = (int)objWardWizSchTempScanDlg.DoModal();
		if (iRet == IDOK)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ShowSchedTempFileScanFinished", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : DisplayProductExpiryMsg
*  Description    : Display product expiry message.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 3th Dec 2015
****************************************************************************************************/
bool CISpyAVTrayDlg::DisplayProductExpiryMsg()
{
	try
	{
		CISpyCommonTrayDlg objISpyCommonTrayDlg;
		objISpyCommonTrayDlg.m_bExpiryDateMsg = true;
		DWORD dwRegEntryForTray = 0;
		dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
		if (dwRegEntryForTray == 1)
		{
			objISpyCommonTrayDlg.DoModal();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::DisplayProductExpiryMsg", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SetNonGenunineRegistry
*  Description    : Set as copy of wardwiz is not genuine.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 7th Dec,2015
****************************************************************************************************/
bool CISpyAVTrayDlg::SetNonGenunineRegistry(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;

		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;

		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CWardwizTrayDlg::SetNonGenunineRegistry", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CWardwizTrayDlg::SetNonGenunineRegistry", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::SetNonGenunineRegistry", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : IsRemovableDevice
*  Description    : Function to check is input path this removable device.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 11th Dec 2015
****************************************************************************************************/
//bool CISpyAVTrayDlg::IsRemovableDevice(LPTSTR lpDrive)
//{
//	bool bReturn = true;
//	try
//	{
//		if (!m_fpISRemovableDevice)
//		{
//			return false;
//		}
//
//		bReturn = m_fpISRemovableDevice(lpDrive);
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizAVTrayDlg::IsRemovableDevice", 0, 0, true, SECONDLEVEL);
//	}
//	return bReturn;
//}

/***************************************************************************************************
*  Function Name  :	AddIntoQueue
*  Description    :	This Function is used to Add File info. into Vector.
*  Author Name    :	Ramkrushna Shelke
*  Date           :	 27 July 2016
****************************************************************************************************/
bool CISpyAVTrayDlg::AddIntoQueue(LPTSTR szThreatName, LPTSTR szFilePath, DWORD dwActionTaken)
{
	try
	{
		DWORD dwDetectThreadID = 0x00;

		if (m_hThreadDetectThreat == NULL)
		{
			m_hThreadDetectThreat = CreateThread(NULL, 0, ShowQuarentineDlgThread, (LPVOID)this, 0, &dwDetectThreadID);
		}
		Sleep(100);
		//Add entries in queue.

		g_CWWizDetectQuarentineDlg.AddIntoQueue(szThreatName, szFilePath, dwActionTaken);
		AddLogEntry(L"### File added %s", szFilePath, 0, true, SECONDLEVEL);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::AddIntoQueue, FilePath: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :	AddIntoQueueEmailScanInfo
*  Description    :	This Function is used to Add File info. into Vector.
*  Author Name    : Amol Jaware
*  Date           :	 27 June 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::AddIntoQueueEmailScanInfo(LPTSTR szThreatName, LPTSTR szFilePath, LPTSTR szSenderAddr, DWORD dwActionTaken)
{
	try
	{
		DWORD dwDetectThreadID = 0x00;

		if (m_hThreadDetectEmailScanThreat == NULL)
		{
			m_hThreadDetectEmailScanThreat = CreateThread(NULL, 0, ShowQuarentineDlgThread4EmailScan, (LPVOID)this, 0, &dwDetectThreadID);
		}
		//Add entries in queue.
		Sleep(2*1000);
		g_CEmailNotifyTrayDlg.AddIntoQueueEmailScanInfo(szThreatName, szFilePath, szSenderAddr, dwActionTaken);
		AddLogEntry(L"### Attachment File added %s", szFilePath, 0, true, SECONDLEVEL);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::AddIntoQueueEmailScanInfo", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :	ReInitializeQurentineDlg
*  Description    :	This Function is used to ReInitialize Thread.
*  Author Name    :	Yogeshwar Rasal
*  Date           :	27 July 2016
****************************************************************************************************/
bool CISpyAVTrayDlg::ReInitializeQurentineDlg()
{
	try
	{
		if (m_hThreadDetectThreat != NULL)
		{
			m_hThreadDetectThreat = NULL;
		}
		ClearEntries();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAVTrayDlg::ReInitializeQurentineDlg", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :	ReInitializeQurentineDlg
*  Description    :	This Function is used to ReInitialize Thread.
*  Author Name    :	Amol Jaware
*  Date           :	27 June 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::ReInitializeQurentineDlg4EmailScan()
{
	try
	{
		if (m_hThreadDetectEmailScanThreat != NULL)
		{
			m_hThreadDetectEmailScanThreat = NULL;
		}
		ClearEntries4EmailScan();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAVTrayDlg::ReInitializeQurentineDlg4EmailScan", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :	ClearEntries
*  Description    :	This Function is used to Reset Threat Entries.
*  Author Name    :	Yogeshwar Rasal
*  Date           :	 30 July 2016
****************************************************************************************************/
void CISpyAVTrayDlg::ClearEntries()
{
	try
	{
	  g_CWWizDetectQuarentineDlg.m_vecThreatList.clear();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::ClearEntries", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :	ClearEntries4EmailScan
*  Description    :	This Function is used to Reset Threat Entries.
*  Author Name    :	Amol Jaware
*  Date           :	27 June 2018
****************************************************************************************************/
void CISpyAVTrayDlg::ClearEntries4EmailScan()
{
	try
	{
		g_CEmailNotifyTrayDlg.m_vecThreatList.clear();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::ClearEntries4EmailScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  ReadProductVersion4mRegistry
*  Description    :  Read values from Registry
*  Author Name    :  Jeena Mariam Saji
*  Date           :  15 Sept 2016
****************************************************************************************************/
void CISpyAVTrayDlg::ReadProductVersion4mRegistry()
{
	try
	{
		HKEY hKey = NULL;
		DWORD dwvalueSType = 0;
		DWORD dwvalueSize = sizeof(DWORD);
		TCHAR szAppVersion[1024];
		TCHAR szvalue[1024];
		DWORD dwAppVersionLen = 1024;
		DWORD dwType = REG_SZ;
		DWORD ChkvalueForDemoEdition = 0;
		DWORD dwtype = REG_SZ;
		DWORD dwvalue_length = 1024;
		TCHAR szvalueVersion[1024];
		DWORD dwvaluelengthVersion = 1024;
		DWORD dwtypeVersion = REG_SZ;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
		{
			return;
		}
		long ReadReg = RegQueryValueEx(hKey, L"AppVersion", NULL, &dwType, (LPBYTE)&szAppVersion, &dwAppVersionLen);
		if (ReadReg == ERROR_SUCCESS)
		{
			m_csRegProductVer = (LPCTSTR)szAppVersion;
			if (m_csRegProductVer == L"")
			{
				m_csRegProductVer = L"2.1.0.1";
			}
		}
		ReadReg = RegQueryValueEx(hKey, L"DataBaseVersion", NULL, &dwtypeVersion, (LPBYTE)&szvalueVersion, &dwvaluelengthVersion);
		if (ReadReg == ERROR_SUCCESS)
		{
			m_csRegDataBaseVer = (LPCTSTR)szvalueVersion;
			if (m_csRegDataBaseVer == L"")
			{
				m_csRegDataBaseVer = L"1.1.3.21";
			}
		}
		ReadReg = RegQueryValueEx(hKey, L"LastLiveupdatedt", NULL, &dwtype, (LPBYTE)&szvalue, &dwvalue_length);
		if (ReadReg == ERROR_SUCCESS)
		{
			int iPos = 0;
			m_csUpdateDate = (CString)szvalue;
			m_csUpdateDate = m_csUpdateDate.Tokenize(_T(" "), iPos);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizTrayDlg::ReadProductVersion4mRegistry", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  GetInformationFromINI
*  Description    :  Get information from INI
*  Author Name    :  Jeena Mariam Saji
*  Date           :  15 Sept 2016
****************************************************************************************************/
bool CISpyAVTrayDlg::GetInformationFromINI()
{
	try
	{
		TCHAR  szActualINIPath[255] = { 0 };
		TCHAR  szThreatDefCount[255] = { 0 };
		if (theApp.m_dwProductID)
		{
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%sVBSETTINGS\\%s", theApp.m_AppPath, L"PRODUCTSETTINGS.ini");
		}
		if (!PathFileExists(szActualINIPath))
		{
			AddLogEntry(L"### File not found : %s in GetInformationFromINI ", szActualINIPath, 0, true, SECONDLEVEL);
			return false;
		}
		else
		{
			GetPrivateProfileString(L"VBSETTINGS", L"ThreatDefCount", L"", szThreatDefCount, 511, szActualINIPath);
			m_csThreatDefCount = (LPCTSTR)szThreatDefCount;
			if (szThreatDefCount == L"")
			{
				m_csThreatDefCount = L"8127413";//Default Value as current DB Count.
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in CWardwizTrayDlg::GetInformationFromINI ", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  GetRegisteredUserInfo
*  Description    :  Get information of User
*  Author Name    :  Jeena Mariam Saji
*  Date           :  15 Sept 2016
****************************************************************************************************/
void CISpyAVTrayDlg::GetRegisteredUserInfo()
{
	try
	{
		ReadProductVersion4mRegistry();
		GetInformationFromINI();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizTrayDlg::GetRegisteredUserInfo", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  ShowProductRenewOption
*  Description    :  Show PopUp for Renewal of Product
*  Author Name    :  Jeena Mariam Saji
*  Date           :  13th April 2017
****************************************************************************************************/
DWORD WINAPI ShowProductRenewOption(LPVOID lpvThreadParam)
{
	try
	{
		//Wait here for some time then show.
		Sleep(2* 60 * 1000); //2 minutes

		CISpyAVTrayDlg *pThis = (CISpyAVTrayDlg*)lpvThreadParam;

		if (!pThis)
			return 0;
		DWORD dwRegEntryForTray = 0;
		dwRegEntryForTray = pThis->ReadRegistryEntry(L"dwShowTrayPopup");
		if (dwRegEntryForTray == 1)
		{
			CPopUpRenewProduct  objPopUpRenewProduct;
			objPopUpRenewProduct.DoModal();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ShowProductRenewOption", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***************************************************************************************************
*  Function Name  :  HandleActiveScanSettings
*  Description    :  Handling UI to Tray active scan state call.
*  Author Name    :  Amol J.
*  Date           :  24 Oct 2017
****************************************************************************************************/
bool CISpyAVTrayDlg::HandleActiveScanSettings(DWORD dwAction)
{
	try
	{
		if (m_pWardWizWidgetDlg != NULL)
		{
			m_pWardWizWidgetDlg->WidgetActiveScanColorState(dwAction);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::HandleActiveScanSettings", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReLoadWidgetsUISettings
*  Description    :  Function which reloads modified settings
*  Author Name    :  Amol J.
*  Date           :  09 Nov 2017
****************************************************************************************************/
bool CISpyAVTrayDlg::ReLoadWidgetsUISettings(DWORD dwAction)
{
	try
	{
		switch (dwAction)
		{
			case RELOAD_TEMPORARY_FILES:
				if (m_pWardWizWidgetDlg != NULL)
				{
					m_pWardWizWidgetDlg->CallGetTotalTempFolderSize();
				}
				break;
			case WIDGET_HIDE_UI:
				if (m_pWardWizWidgetDlg != NULL)
				{
					m_pWardWizWidgetDlg->CloseUI();
					m_pWardWizWidgetDlg = NULL;
				}
				break;
			case WIDGET_SHOW_UI:
				if (m_pWardWizWidgetDlg == NULL)
				{
					LaunchWidgetsUI();
				}
				break;
			case RELOAD_UPDATES:
				if (m_pWardWizWidgetDlg != NULL)
				{
					m_pWardWizWidgetDlg->CallGetProdUpdtInfo();
					m_pWardWizWidgetDlg->GetDaysLeftOnHomeBtn();
				}
				break;
			default:
				AddLogEntry(L"### Unhandled case in CWardwizTrayDlg::ReLoadWidgetsUISettings", 0, 0, true, SECONDLEVEL);
					break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ReLoadWidgetsUISettings", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  LaunchWidgetsUI
*  Description    :  Function which Launch Widgets UI.
*  Author Name    :  Amol J.
*  Date           :  09 Nov 2017
****************************************************************************************************/
void CISpyAVTrayDlg::LaunchWidgetsUI()
{
	__try
	{
		if (m_hWidgetsUIThread != NULL)
		{
			SuspendThread(m_hWidgetsUIThread);
			TerminateThread(m_hWidgetsUIThread, 0x00);
			m_hWidgetsUIThread = NULL;
		}

		DWORD dwShowThreadID = 0x00;
		m_hWidgetsUIThread = ::CreateThread(NULL, 0, ShowWidgetWindowDoModalThread, (LPVOID) this, 0, &dwShowThreadID);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizTrayDlg::LaunchWidgetsUI", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : StartInstallationProcessQueue
Description    : Function which starts Installation process queue
SR.NO		   :
Author Name    : Ramkrushna Shelke
Date           : 13 Jul 2016
***********************************************************************************************/
bool CISpyAVTrayDlg::StartInstallationProcessQueue()
{
	bool bReturn = false;
	try
	{
		if (m_bIsIntProcessON)
			return true;

		//  Create specified number of threads.
		for (DWORD iThreadNum = 0; iThreadNum < INSTALLATION_DEFAULT_THREAD_COUNT; iThreadNum++) {
			m_hProcessingThreads[iThreadNum] = std::thread(ProcessInstallQueueThread, std::ref(m_cInstallationQueue));
			SetThreadPriority(m_hProcessingThreads[iThreadNum].native_handle(), THREAD_PRIORITY_HIGHEST);
		}

		m_bIsIntProcessON = true;
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizTrayDlg::StartInstallationProcessQueue", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***********************************************************************************************
Function Name  : StopInstallationProcessQueue
Description    : Function which stops installation process queue
SR.NO		   :
Author Name    : Ramkrushna Shelke
Date           : 13 Jul 2016
***********************************************************************************************/
bool CISpyAVTrayDlg::StopInstallationProcessQueue()
{
	try
	{
		if (!m_bIsIntProcessON)
			return false;

		//Terminate the thread here by adding exit call
		for (int i = 0; i < INSTALLATION_DEFAULT_THREAD_COUNT; ++i) {
			m_cInstallationQueue.push(L"WRDWIZEXIT");
		}

		//Wait here till thread exits.
		for (int i = 0; i < INSTALLATION_DEFAULT_THREAD_COUNT; ++i) {
			if (m_hProcessingThreads[i].joinable())
			{
				m_hProcessingThreads[i].join();
			}
		}

		m_bIsIntProcessON = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizTrayDlg::StopInstallationProcessQueue", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : ProcessScanQueueThread
Description    : Thread function to Invoke installation for all the files stored in queue.
SR.NO		   :
Author Name    : Ramkrushna Shelke
Date           : 13 Jul 2016
***********************************************************************************************/
void ProcessInstallQueueThread(CConCurrentQueue<std::wstring>& pObjeOnAccess)
{
	OutputDebugString(L">>> In ProcessInstallQueueThread");
	__try
	{
		ProcessInstallQueueThreadSEH(pObjeOnAccess);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ProcessInstallQueueThread", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : ProcessScanQueueThreadSEH
Description    : Thread function to Invoke installation for all the files stored in queue.
SR.NO		   :
Author Name    : Gayatri A.
Date           : 13 Jul 2016
***********************************************************************************************/
void ProcessInstallQueueThreadSEH(CConCurrentQueue<std::wstring>& pObjeOnAccess)
{
	try
	{
		while (true)
		{
			auto itemFromQueue = pObjeOnAccess.pop();

			if (itemFromQueue == L"WRDWIZEXIT")
			{
				break;
			}

			std::wstring split_me(itemFromQueue.c_str());
			std::vector<std::wstring> Tokens = SplitString(split_me, L' ');
			if (Tokens.size() == 0x04)
			{
				CString csTaskID(Tokens[0].c_str());
				CString csIP(Tokens[1].c_str());
				CString csUserName(Tokens[2].c_str());
				CString csPassword(Tokens[3].c_str());

				if (CISpyAVTrayDlg::m_pThis->InstallClientSetup(csTaskID.GetBuffer(), csIP.GetBuffer(), csUserName.GetBuffer(), csPassword.GetBuffer()) != 0x00)
				{
					AddLogEntry(L"### Failed to Install Client IP: [%s]", csIP, 0, true, SECONDLEVEL);
					continue;
				}

				AddTaskCompletedServerLog(csTaskID.GetBuffer(), L"3", csIP.GetBuffer(), L"Client [%s] Installed successfully", csIP);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ProcessInstallQueueThreadSEH", 0, 0, true, SECONDLEVEL);
	}
}

bool CISpyAVTrayDlg::EPSInstallClientSetup(LPISPY_PIPE_DATA lpData)
{
	//Ram: Temporary OFF
	bool bReturn = false;
	return false;

	try
	{
		if (!lpData)
			return bReturn;

		if (!StartInstallationProcessQueue())
		{
			AddLogEntry(L"### Failed to StartInstallationProcessQueue in CWardwizTrayDlg::EPSInstallClientSetup", 0, 0, true, SECONDLEVEL);
			return false;
		}

		CString csClientInfo;
		csClientInfo.Format(L"%s %s %s", lpData->szFirstParam, lpData->szSecondParam, lpData->szThirdParam);

		CString csInstallMachineInfo;
		csInstallMachineInfo.Format(L">>> TaskID, Target Machine: [%s], Username: [%s]", lpData->szFirstParam, lpData->szSecondParam);
		m_cInstallationQueue.push(csClientInfo.GetBuffer());

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::EPSInstallClientSetup", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

DWORD CISpyAVTrayDlg::InstallClientSetup(LPTSTR lpszTaskID, LPTSTR lpszIP, LPTSTR lpszUserName, LPTSTR lpszPassword)
{
	DWORD dwRet = 0x00;
	try
	{
		if (!lpszTaskID)
			return SANITY_CHECK_FAILURE;

		if (!lpszIP)
			return SANITY_CHECK_FAILURE;

		if (!lpszUserName)
			return SANITY_CHECK_FAILURE;

		if (!lpszPassword)
			return SANITY_CHECK_FAILURE;

		CString csMachineID;
		CString csMachineIPAddress = lpszIP;
		CString csMachineName = GetMachineNameByIP(csMachineIPAddress);

		CString sMsg;
		sMsg.Format(L"[%s] Checking connection!", csMachineName);
		OutputDebugString(sMsg);

		CConnection oConnection(csMachineIPAddress, csMachineName, L"", NULL, false);
		oConnection.m_csUserName = lpszUserName;
		oConnection.m_csPassword = lpszPassword;
		dwRet = oConnection.EstablishAdminConnection(lpszTaskID);
		if (dwRet != 0x00)
		{
			return dwRet;
		}

		dwRet = oConnection.InstallClient(lpszTaskID, csMachineID);
		if (dwRet != 0x00)
		{
			return dwRet;
		}

		dwRet = 0x00;
		sMsg.Format(L"[%s] Client Installed successfully!", csMachineName);
		OutputDebugString(sMsg);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::InstallClientSetup", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}


CString CISpyAVTrayDlg::GetMachineNameByIP(CString csIPAddress)
{
	CString csMachineName(csIPAddress);
	unsigned int addr;
	addr = inet_addr((CStringA)csIPAddress);
	HOSTENT *lpHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	if (lpHost)
		csMachineName = lpHost->h_name;

	return csMachineName;
}

/***************************************************************************************************
*  Function Name  :  LaunchWidgetsUI
*  Description    :  Function which Launch Widgets UI.
*  Author Name    :  Amol J.
*  Date           :  09 Nov 2017
****************************************************************************************************/
bool CISpyAVTrayDlg::LaunchPasswordWnd()
{
	__try
	{

		return ShowPasswordWindow();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizTrayDlg::LaunchPasswordWnd", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  LaunchPasswordWndForPC
*  Description    :  Function to show PopUp
*  Author Name    :  Jeena Mariam Saji
*  Date           :  18 June 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::LaunchPasswordWndForPC(LPTSTR lpFirstParam)
{	
	try
	{	
		if (!lpFirstParam)
			return false;
		CString csUserName = lpFirstParam;
		csUserName.MakeLower();
		CString csCurrentUser;
		csCurrentUser = GetCurrentUserName();
		csCurrentUser.MakeLower();
		if (csUserName != csCurrentUser)
		{
			return false;
		}
		HWND hWindow = ::FindWindow(NULL, L"AKTRAYPWD");
		if (!hWindow)
		{
			LPCTSTR SubKey = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
			CString csKeyVal;
			csKeyVal = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
			DWORD dwChangeValue = 0;
			DWORD dwLockWorkStation = 0x00;
			if (m_objReleaseNotesDlg->GetRegistryDWORDEntry(HKEY_LOCAL_MACHINE, csKeyVal.GetBuffer(), L"DisableLockWorkstation", dwLockWorkStation) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for DisableLockWorkstation in CAddLogEntryTrayDlg::LaunchPasswordWndForPC", 0, 0, true, SECONDLEVEL);;
			}
			else 
			{
				if (!SetRegistrykeyUsingService(SubKey, L"DisableLockWorkstation", REG_DWORD, dwChangeValue))
				{
					AddLogEntry(L"### Error in Setting Registry CWardwizTrayDlg::LaunchPasswordWndForPC", 0, 0, true, SECONDLEVEL);
					return false;
				}
			}

			CWardWizParCtrlPopUp objCWardWizParCtrlPopUp(this);
			Sleep(50);
			int iRet = (int)objCWardWizParCtrlPopUp.DoModal();
			if (iRet != IDOK)
			{
				return true;
			}
			else
			{
				LockWorkStation();
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::LaunchPasswordWndForPC", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  LaunchPasswordWndForPCINet
*  Description    :  Function to show PopUp
*  Author Name    :  Jeena Mariam Saji
*  Date           :  10 July 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::LaunchPasswordWndForPCINet()
{
	try
	{
		//CWardWizFirewallBlockPopUp objWardWizFirewallBlockPopUp(NULL);
		//objWardWizFirewallBlockPopUp.m_iTrayMsgType = WARNING_PC_INET_LOCK;
		//DWORD dwRegEntryForTray = 0;
		//dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
		//if (dwRegEntryForTray == 1)
		//{
		//	int iRet = (int)objWardWizFirewallBlockPopUp.DoModal();
		//}
		/*if (iRet != IDOK)
		{
			return true;
		}
		else
		{
			LockWorkStation();
		}*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::LaunchPasswordWndForPCINet", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  OnSessionChange
*  Description    :  Handler for onsessionchange
*  Author Name    :  Jeena Mariam Saji
*  Date           :  25 June 2018
*  Updated By	  : Kunal Waghmare 
*  Date           : 3 July 2018
*  Updated By	  : Nitin Shelar
*  Date           : 2 August 2018
****************************************************************************************************/
void CISpyAVTrayDlg::OnSessionChange(UINT nSessionState, UINT nId)
{
	try
	{
		DWORD dwRegEntryForTray = 0;
		DWORD dwSessionID = 0x00;
		dwRegEntryForTray = ReadRegistryEntry(L"dwParCtrlActiveFlag");
		if (WTS_SESSION_UNLOCK == nSessionState)
		{
			CString csTrayPath;
			csTrayPath.Format(GetWardWizPathFromRegistry() + L"VBTRAY.EXE");
			CString csCurrentUser;
			csCurrentUser = GetCurrentUsrName(dwSessionID);

			if (!SendData2Service(LAUNCH_APP_USING_SERVICE, csTrayPath, dwSessionID, 0x00))
			{
				AddLogEntry(L"### Failed to send data in CWardwizTrayDlg::OnSessionChange SendData2Service::RELOAD_APPLICATION_ON_SWITCH", 0, 0, true, SECONDLEVEL);
			}

			CheckParCtrlPermission();
		}
		else if (WTS_SESSION_LOCK == nSessionState)
		{
			InsertUserNametoINI();
			InsertParCtrlReport(dwRegEntryForTray);
		}
		if (WTS_CONSOLE_CONNECT == nSessionState)
		{
			CString csTrayPath;
			csTrayPath.Format(GetWardWizPathFromRegistry() + L"VBTRAY.EXE");
			CString csCurrentUser;
			csCurrentUser = GetCurrentUsrName(dwSessionID);

			if (!SendData2Service(LAUNCH_APP_USING_SERVICE, csTrayPath, dwSessionID, 0x00))
			{
				AddLogEntry(L"### Failed to send data in CWardwizTrayDlg::OnSessionChange SendData2Service::RELOAD_APPLICATION_ON_SWITCH", 0, 0, true, SECONDLEVEL);
			}
			CheckParCtrlPermission();
		}
		if (WTS_CONSOLE_DISCONNECT == nSessionState)
		{
			InsertUserNametoINI();
		}
		if (WTS_SESSION_LOGON == nSessionState)
		{ 
			CString csTrayPath;
			csTrayPath.Format(GetWardWizPathFromRegistry() + L"VBTRAY.EXE");
			CString csCurrentUser;
			csCurrentUser = GetCurrentUsrName(dwSessionID);

			if (!SendData2Service(LAUNCH_APP_USING_SERVICE, csTrayPath, dwSessionID, 0x00))
			{
				AddLogEntry(L"### Failed to send data in CWardwizTrayDlg::OnSessionChange SendData2Service::RELOAD_APPLICATION_ON_SWITCH", 0, 0, true, SECONDLEVEL);
			}
			CheckParCtrlPermission();
		}
		if (WTS_SESSION_LOGOFF == nSessionState)
		{
			InsertUserNametoINI();
		}
		if (WTS_REMOTE_DISCONNECT == nSessionState)
		{
			InsertUserNametoINI();
		}
		if (WTS_REMOTE_CONNECT == nSessionState)
		{
			InsertUserNametoINI();
		}
		if (WTS_SESSION_REMOTE_CONTROL == nSessionState)
		{
			InsertUserNametoINI();
		}
		CDialog::OnSessionChange(nSessionState, nId);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::OnSessionChange", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertParCtrlReport
*  Description    :	Insert Parent Control information in report
*  Author Name    : Kunal Waghmare
*  Date           : 3 July 2018
**********************************************************************************************************/
void CISpyAVTrayDlg::InsertParCtrlReport(DWORD dwFlag)
{
	try
	{
		//dwFlag 0:Login 1:Block 2:Log_out 3:Enabled 4:Disabled
		CString csInsertQuery, csSelectQuery;
		CString	csUserName = GetCurrentUserName().MakeLower();
		csInsertQuery = _T("INSERT INTO Wardwiz_ParentalCtrl_Details(db_PCDate, db_PCTime, db_PCActivity, db_Username) VALUES (Date('now'),Datetime('now','localtime'),");
		csInsertQuery.AppendFormat(L"%d", dwFlag);
		csInsertQuery.Append(L",'");
		csInsertQuery.Append(csUserName);
		csInsertQuery.Append(_T("');"));
		CT2A ascii(csInsertQuery, CP_UTF8);
		INT64 iScanId = InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::InsertParCtrlReport", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Kunal Waghmare
*  Date           : 3 July 2018
*  SR_NO		  :
**********************************************************************************************************/
INT64 CISpyAVTrayDlg::InsertDataToTable(const char* szQuery)
{
	try
	{
		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		m_objSqlDb.SetDataBaseFilePath(dbPath.m_psz);
		m_objSqlDb.Open();
		int iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = m_objSqlDb.GetLastRowId();
		m_objSqlDb.Close();
		return iLastRowId;
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CWardwizTrayDlg::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	GetSQLiteDBFilePath
*  Description    :	Helper function to get Current working directory path
*  Author Name    : Kunal Waghmare
*  Date           : 3 July 2018
**********************************************************************************************************/
CString CISpyAVTrayDlg::GetSQLiteDBFilePath()
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
		AddLogEntry(L"### Exception in value CWardwizTrayDlg::GetSQLiteDBFilePath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************************************
*  Function Name  :  SendData2Service
*  Description    :  Send data to service
*  Author Name    :  Jeena Mariam Saji
*  Date           :  31 August 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::SendData2Service(DWORD dwMsg, CString csParam, DWORD dwVal, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMsg;
		_tcscpy(szPipeData.szFirstParam, csParam.GetBuffer());
		szPipeData.dwSecondValue = dwVal;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SendData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SendData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::SendData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  SendData2Service
*  Description    :  Send data to service
*  Author Name    :  Jeena Mariam Saji
*  Date           :  25 June 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::SendData2Service(DWORD dwMsg, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMsg;
		
		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SendData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SendData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::SendData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  ClosePasswordWndForPC
*  Description    :  Close tray popup
*  Author Name    :  Jeena Mariam Saji
*  Date           :  26 June 2018
*  Updated By	  : Kunal Waghmare 
*  Date           : 3 July 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::ClosePasswordWndForPC()
{	
	try
	{
		HWND hWindow = ::FindWindow(NULL, L"AKTRAYPWD");
		if (hWindow)
			::PostMessage(hWindow, WM_CLOSE, 0, 0);
		
		LPCTSTR SubKey = theApp.m_csRegKeyPath;
		LPCTSTR strKey = L"dwParCtrlActiveFlag";
		DWORD dwChangeValue = 0;
		if (!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue, true))
		{
			AddLogEntry(L"### Error in Setting Registry CWardwizTrayDlg::ClosePasswordWndForPC", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ClosePasswordWndForPC", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : OnEndSession
Description    : OnEndSession
Author Name    : Kunal Waghmare
Date           : 3 July 2018
****************************************************************************/
void CISpyAVTrayDlg::OnEndSession(BOOL bEnding)
{
	try
	{
		CDialog::OnEndSession(bEnding);
		DWORD dwRegEntryForTray = 0;
		dwRegEntryForTray = ReadRegistryEntry(L"dwParCtrlActiveFlag");
		if (dwRegEntryForTray != 1)
		{
			InsertParCtrlReport(2);
		}
		AddLogEntry(L">>> OnEndSession", 0, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::OnEndSession", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : SetRegistrykeyUsingService
Description    : SetRegistrykeyUsingService.
Author Name    : Kunal Waghmare
Date           : 3 July 2018
****************************************************************************/
bool CISpyAVTrayDlg::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;
		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;
		Sleep(100);
		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CWardwizTrayDlg::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CWardwizTrayDlg::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : GetCurrentUserName
Description    : Function returns current username.
Author Name    : Kunal Waghmare
Date           : 3 July 2018
****************************************************************************/
CString CISpyAVTrayDlg::GetCurrentUserName()
{
	try
	{
		DWORD dwPID = ::GetCurrentProcessId();
		CExecuteProcess objExecProcess;
		return objExecProcess.GetUserNamefromProcessID(dwPID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIApp::GetCurrentUserName", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}
/***************************************************************************
Function Name  : InsertUserNametoINI
Description    : Function insert current username into ini file.
Author Name    : Nitin Shelar
Date           : 27 July 2018
****************************************************************************/
void CISpyAVTrayDlg::InsertUserNametoINI()
{
	try
	{
		CString csUserName;
		DWORD dwSessionID;
		csUserName = GetCurrentUsrName(dwSessionID);
		csUserName.MakeLower();
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";

		if (_tcslen(csUserName) == 0x00)
		{
			return;
		}

		if (!PathFileExists(csIniFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizTrayDlg::InsertUserNametoINI", csIniFilePath, 0, true, SECONDLEVEL);
			return;
		}
		WritePrivateProfileString(L"VBSETTINGS", L"CurrentUserName", csUserName, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::InsertUserNametoINI", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :  ShowPortScanBlockPopUp
*  Description    :  Function which shows port scan block pop-up
*  Author Name    :  Ram Shelke
*  Date           :  11 AUG 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::ShowPortScanBlockPopUp()
{
	try
	{
		CWardWizPortScanPopUp	objWardWizPortScanPopUp;
		objWardWizPortScanPopUp.DoModal();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::ShowPortScanBlockPopUp", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  LaunchAppBlockDlg
*  Description    :  Function to show PopUp
*  Author Name    :  Akshay Patil
*  Date           :  13 August 2018
****************************************************************************************************/
bool CISpyAVTrayDlg::LaunchAppBlockDlg(LPTSTR lpFirstParam)
{
	CString csDestPath;
	try
	{
		static int i_NumberOfCalls = 0;
		if (i_NumberOfCalls >= 1)
		{
			i_NumberOfCalls = 0;
			return false;
		}
		i_NumberOfCalls++;

		if (!lpFirstParam)
			return false;

		csDestPath =(CString)lpFirstParam;

		if (csDestPath.GetLength() > 40)
		{
			csDestPath.Format(L"%s...%s", csDestPath.Left(15), csDestPath.Right(15));
		}

		HWND hWndWeb = ::FindWindow(NULL, L"AKTRAYAPPBLOCK");
		if (hWndWeb == NULL)
		{
			DWORD dwRegEntryForTray = 0;
			dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
			if (dwRegEntryForTray == 1)
			{
				CWardWizFirewallBlockPopUp objWardWizFirewallBlockPopUp;
				objWardWizFirewallBlockPopUp.m_iTrayMsgType = WARNING_PAR_CTRL_APP_BLOCK;
				objWardWizFirewallBlockPopUp.csFilePath = csDestPath;
				objWardWizFirewallBlockPopUp.DoModal();
			}
		}
		else
		{
			STAPPBLK szAppBlock = { 0 };
			_tcscpy(szAppBlock.szFilePath, csDestPath);
			szAppBlock.iTrayMsgType = WARNING_PAR_CTRL_APP_BLOCK;
			::ShowWindow(hWndWeb, SW_SHOW);
			::SendMessage(hWndWeb, WM_MESSAGESHOWAPPBLK, 0, (LPARAM)&szAppBlock);
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::LaunchAppBlockDlg", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  GetCurrentUsrName
*  Description    :  Get Current User Name
*  Author Name    :  Jeena Mariam Saji
*  Date           :  7 Sept 2018
****************************************************************************************************/
CString CISpyAVTrayDlg::GetCurrentUsrName(DWORD &dwSessionID)
{
	CString csUserName;
	try
	{
		int itemIdx = 0;
		PWTS_SESSION_INFO ppSessionInfo = NULL;
		DWORD			  pCount = 0;
		WTS_SESSION_INFO  wts;

		WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE,
			0,
			1,
			&ppSessionInfo,
			&pCount);
		for (DWORD i = 0; i < pCount; i++)
		{
			wts = ppSessionInfo[i];

			DWORD					TSSessionId = wts.SessionId;
			WTS_CONNECTSTATE_CLASS	TSState = wts.State;

			switch (TSState)
			{
			case WTSActive:
				dwSessionID = TSSessionId;
				csUserName = GetTSUserName(TSSessionId);
				break;
			}
		}

		WTSFreeMemory(ppSessionInfo);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::GetCurrentUsrName", 0, 0, true, SECONDLEVEL);
		return L"";
	}
	return csUserName;
}

/***************************************************************************************************
*  Function Name  :  GetTSUserName
*  Description    :  Get Current User Name
*  Author Name    :  Jeena Mariam Saji
*  Date           :  7 Sept 2018
****************************************************************************************************/
CString CISpyAVTrayDlg::GetTSUserName(DWORD sessionID)
{
	CString currentUserName;
	try
	{
		LPTSTR  ppBuffer = NULL;
		DWORD   pBytesReturned = 0;
		currentUserName.Empty();

		if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
			sessionID,
			WTSUserName,
			&ppBuffer,
			&pBytesReturned))
		{
			currentUserName = CString(ppBuffer);
		}

		WTSFreeMemory(ppBuffer);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::GetTSUserName", 0, 0, true, SECONDLEVEL);
		return L"";
	}
	return currentUserName;
}

/****************************************************************************************************
*  Function Name	: CheckProductUptoDate
*  Description		: Checking product is upto date or not.
*  Author Name		: Kunal Waghmare
*  Date				: 3Mar 2019
****************************************************************************************************/
bool CISpyAVTrayDlg::CheckProductUptoDate()
{
	bool bReturn = false;
	try
	{
		CString csCommandLine = m_csUpdateDate;
		int iPos = 0;
		int szTemp[3] = { 0 };
		for (int i = 0; i < 3; i++)
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

		CTime Time_RegistryDate(iYear, iMonth, iDay, 0, 0, 0);
		CTime Time_CurDate(iYear1, iMonth1, iDate1, 0, 0, 0);
		CTimeSpan Time_Diff = Time_CurDate - Time_RegistryDate;
		int Span = static_cast<int>(Time_Diff.GetDays());
		if (Span > 7)
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
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::CheckProductUptoDate", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/****************************************************************************************************
*  Function Name	: CheckProductInfo
*  Description		: Check product is upto date or active scan enabled or registration required.
*  Author Name		: Kunal Waghmare
*  Date				: 3Mar 2019
****************************************************************************************************/
bool CISpyAVTrayDlg::CheckProductInfo(CString &csProdInfo)
{
	bool bReturn = false;
	HKEY hKey = NULL;
	TCHAR szvalue[1024];
	DWORD dwtype = REG_SZ;
	DWORD dwvalue_length = 1024;
	try
	{

		if (theApp.m_dwDaysLeft < 1)
		{
			csProdInfo.AppendFormat(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_REG_REQRD"));
			bReturn = true;
		}
		else if (ReadRegistryEntry(L"dwActiveScanOption") != 0x01)
		{
			csProdInfo.AppendFormat(L"%s\n", theApp.m_objwardwizLangManager.GetString(L"IDS_ACTIVE_SCN_OFF"));
			bReturn = true;
		}
		else if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
		{
			long ReadReg = RegQueryValueEx(hKey, L"LastLiveupdatedt", NULL, &dwtype, (LPBYTE)&szvalue, &dwvalue_length);
			if (ReadReg == ERROR_SUCCESS)
			{
				int iPos = 0;
				m_csUpdateDate = (CString)szvalue;
				m_csUpdateDate = m_csUpdateDate.Tokenize(_T(" "), iPos);
			}
			RegCloseKey(hKey);
			if (CheckProductUptoDate() == false)
			{
				csProdInfo.AppendFormat(L"%s\n", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_REQRD"));
				bReturn = true;
			}
		}
		return bReturn;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizTrayDlg::CheckProductInfo", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/****************************************************************************************************
*  Function Name	: GetProductID
*  Description		: Get ProductID 
*  Author Name		: Kunal Waghmare
*  Date				: 3Mar 2019
****************************************************************************************************/
bool CISpyAVTrayDlg::GetProductID()
{
	bool bReturn = false;
	try
	{
		HKEY hKey = NULL;
		DWORD dwvalueSType = 0;
		DWORD dwvalueSize = sizeof(DWORD);
		DWORD dwAppVersionLen = 1024;
		DWORD dwType = REG_SZ;
		DWORD ChkvalueForDemoEdition = 0;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
		{
			return bReturn;
		}
		dwType = REG_DWORD;
		long ReadReg = RegQueryValueEx(hKey, L"dwWardWizDemo", NULL, &dwType, (LPBYTE)&dwvalueSType, &dwvalueSize);
		ChkvalueForDemoEdition = (DWORD)dwvalueSType;
		if (ChkvalueForDemoEdition == 0)
		{
			m_bAllowDemoEdition = false;
		}
		else
		{
			m_bAllowDemoEdition = true;
		}
		RegCloseKey(hKey);
		if (m_bAllowDemoEdition)
		{
			m_csInstalledEdition = L"Vibranium";
		}
		else
		{
			switch (theApp.m_dwProductID)
			{
			case ESSENTIAL:
				m_csInstalledEdition = L"Vibranium";
				break;
			case PRO:
				m_csInstalledEdition = L"Vibranium";
				break;
			case ELITE:
				m_csInstalledEdition = L"Vibranium";
				break;
			case BASIC:
				m_csInstalledEdition = L"Vibranium";
				break;
			case ESSENTIALPLUS:
				m_csInstalledEdition = L"Vibranium";
				break;
			default:
				m_csInstalledEdition = L"NA";
				AddLogEntry(L"### Invalid product edition", 0, 0, true, SECONDLEVEL);
				break;
			}
		}
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizTrayDlg::GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/****************************************************************************************************
*  Function Name	: ShowUSBAutoScanPopUp
*  Description		: To show popup on AutoScan of USB Drive
*  Author Name		: Jeena Mariam Saji
*  Date				: 01 July 2019
****************************************************************************************************/
bool CISpyAVTrayDlg::ShowUSBAutoScanPopUp(DWORD dwAction, LPTSTR lpFirstParam)
{
	bool bReturn = false;
	try
	{
		if (!lpFirstParam)
			return false;
		CWWizUSBScnPopup objWWizUSBScnPopup(NULL);
		objWWizUSBScnPopup.m_iTrayMsgType = dwAction;
		objWWizUSBScnPopup.csDriveName = lpFirstParam;
		DWORD dwRegEntryForTray = 0;
		dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
		if (dwRegEntryForTray == 1)
		{
			HWND hWindow = ::FindWindow(NULL, L"VBUSBPOPUP");
			if (hWindow)
			{
				::PostMessage(hWindow, WM_CLOSE, 0, 0);
			}
			objWWizUSBScnPopup.DoModal();
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizTrayDlg::ShowUSBAutoScanPopUp", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
/****************************************************************************************************
*  Function Name	: ShowBrowserSecurityPopUp
*  Description		: To show popup of Browser Security
*  Author Name		: Swapnil Bhave
*  Date				: 10 Spt 2019
****************************************************************************************************/
bool CISpyAVTrayDlg::ShowBrowserSecurityPopUp(LPTSTR lpFirstParam, LPTSTR lpSecondParam)
{
	bool bReturn = false;
	CString csDestUrl;
	CString csBlkUrl(lpFirstParam);
	try
	{
		if (!lpFirstParam)
			return false;
		CWWizWebFilterDlg objWWizWebFilterDlg(NULL);
		objWWizWebFilterDlg.m_bTrayforBrowserSec = true;
		if (csBlkUrl.GetLength() >= 27)
		{
			csDestUrl.Format(L"%s%s%s", csBlkUrl.Left(12), L"...", csBlkUrl.Right(12));
		}
		else
		{
			csDestUrl = csBlkUrl;
		}
		objWWizWebFilterDlg.m_csUrl = csDestUrl;
		objWWizWebFilterDlg.m_csCategory = lpSecondParam;
		DWORD dwRegEntryForTray = 0;
		dwRegEntryForTray = ReadRegistryEntry(L"dwShowTrayPopup");
		if (dwRegEntryForTray == 1)
		{
			HWND hWindow = ::FindWindow(NULL, L"AKTRAYWBBLOCKDLG");
			if (hWindow)
			{
				::PostMessage(hWindow, WM_CLOSE, 0, 0);
			}
			objWWizWebFilterDlg.DoModal();
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizTrayDlg::ShowBrowserSecurityPopUp", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  RefreshSysTrayIcon
*  Description    :  Forcefully the Warwiz Tray to refresh it
*  Author Name    :  Tejas Shinde
*  Date           :  30 July 2019
****************************************************************************************************/
void CISpyAVTrayDlg::RefreshSysTrayIcon()
{
	try
	{
		CCommonFunctions objCCommonFunctions;
		objCCommonFunctions.RefreshTaskbarNotificationArea();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayDlg::RefreshSysTrayIcon", 0, 0, true, SECONDLEVEL);
	}
}