/************************************************************************************************************************
Program Name          : USBDetectUIDlg.cpp 
Description           : If user clicks yes on confimation UI . it will start enumerate USb contains files and start scan,
						if user clicks on Clean button, detected file get repaired.
Modified By			  : Amol Jaware
Date Of Modification  : 04th July 2016
Version No            : 2.1.0.54
*************************************************************************************************************************/
#include "stdafx.h"
#include "USBDetectUI.h"
#include "USBDetectUIDlg.h"
#include "USBPopupMsgDlg.h"
#include "iTinEmailContants.h"
#include "ISpyCommServer.h"
#include "WrdwizEncDecManager.h"
#include <mmsystem.h>
#include "afxwin.h"
#include "CSecure64.h"
#include "DriverConstants.h"
#include "CScannerLoad.h"
#include "WrdWizSystemInfo.h"

#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

char*	g_strDatabaseFilePath = ".\\VBALLREPORTS.DB";

#define TIMER_SCAN_STATUS			200
#define TIMER_PAUSE_HANDLER			201

#define SETFILEPATH_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 3)
#define SETPERCENTAGE_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 4)

// CAboutDlg dialog used for App About

DWORD WINAPI USBScanThread(LPVOID lpvThreadParam);
DWORD WINAPI QuarantineUSBThread(LPVOID lpParam);
DWORD WINAPI GetScanUSBFilesCount(LPVOID lpParam );
UINT PlayThreatsFoundThread(LPVOID lpThis);
UINT PlayScanFinishedThread(LPVOID lpThis);
CString GetSQLiteDBFilePath();

class CAboutDlg : public CJpegDialog
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
public:
	afx_msg void OnBnClickedButtonAboutClosebtn();
	void ReadDatabaseVersionFromRegistry();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOk();
};

/**********************************************************************************************************                     
*  Function Name  :	CAboutDlg                                                     
*  Description    :	C'tor
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0022
**********************************************************************************************************/
CAboutDlg::CAboutDlg() : CJpegDialog(CAboutDlg::IDD)
{
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0023
**********************************************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
}

/**********************************************************************************************************                     
*  Function Name  :	MESSAGE_MAP                                                     
*  Description    :	Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0024
**********************************************************************************************************/
BEGIN_MESSAGE_MAP(CAboutDlg, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT_CLOSEBTN, &CAboutDlg::OnBnClickedButtonAboutClosebtn)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CAboutDlg::OnBnClickedButtonOk)
END_MESSAGE_MAP()

CISpyCommunicatorServer  g_objCommServer(USB_SERVER, CUSBDetectUIDlg::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));
CISpyCommunicator		 g_objCom(TRAY_SERVER, false);

/**********************************************************************************************************                     
*  Function Name  :	CUSBDetectUIDlg                                                     
*  Description    :	C'tor
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0025
//Issue no: 1151 Short name issue. if long file path it comes C:\...\xyz.txt format Neha Gharge Scantype is kept
for lenght.
**********************************************************************************************************/
CUSBDetectUIDlg::CUSBDetectUIDlg(CWnd* pParent /*=NULL*/) : CJpegDialog(CUSBDetectUIDlg::IDD, pParent) //, behavior_factory("WardWizUSB")
	, m_hScanDllModule(NULL)
	, m_lpLoadSigProc(NULL)
	, m_lpUnLoadSigProc(NULL)
	, m_lpScanFileProc(NULL)
	,m_bIsExpanded(false)
	,m_bSignatureLoaded(false)
	,m_hUSBScanThread(NULL)
	,m_iTotalFileCount(0)
	,m_dwFileScanned(0)
	,m_iThreatsFoundCount(0)
	,m_dwTotalThreatsCleaned(0)
	,m_bISScanning(false)
	,m_bQuarantineFinished(false)
	,m_bScanCount(false)
	,m_bPlaySound(false)
	,m_bRescan(false)
	,m_bClose(false)
	,m_bScanningAborted(false)
	,m_bIsCleaning(false)
	,dwSignatureFailedToLoad(0)
	, m_bDeviceDetached(false)
	,m_iDriveCompareResult(-1)
	, m_bMsgAbortPopup(false)
	, m_bCloseMsgAbortPopup(false)
	,m_dwScanOption(0)
	, m_bNoActionafterScanComplete(false)
	, m_bScanningCompleted(false)
	, m_hQuarantineUSBThread(NULL)
	, m_bOnWMClose(false)
	, m_objDBManager(true)
	, m_objRecoverDB(true)
	, m_objCom(SERVICE_SERVER, true)
	, m_objScanCom(SERVICE_SERVER, true, 2)
	, m_bIsManualStop(false)
	, m_bIsPathExist(false)
	, m_bIsCloseFrmTaskBar(false)
	, m_bEPSCustomScan(false)
	, m_dwIsAborted(0)
	, bQuickScan(false)
	, bFullScan(false)
	, m_csTaskID(L"0")
	, m_bIsShutDownScan(false)
	, m_bUSBAutoScan(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/**********************************************************************************************************                     
*  Function Name  :	~CUSBDetectUIDlg                                                     
*  Description    :	D'tor
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  :WRDWIZUSBUI_0026
**********************************************************************************************************/
CUSBDetectUIDlg::~CUSBDetectUIDlg()
{
	if(m_lpUnLoadSigProc != NULL)
	{
		m_lpUnLoadSigProc();
	}

	if(m_hScanDllModule!=NULL)
	{
		FreeLibrary(m_hScanDllModule);
		m_hScanDllModule = NULL;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0027
**********************************************************************************************************/
void CUSBDetectUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_HIDE_SHOW_DETAILS_BUTTON, m_btnShowHideButton);
	DDX_Control(pDX, IDC_STATIC_SCANNING_TEXT, m_stScanningText);
	DDX_Control(pDX, IDC_STATIC_STATUS_TEXT, m_stStatusText);
	DDX_Control(pDX, IDC_STATIC_ACTUAL_SCANNING_FILEFOLDER, m_stActualScnFileFolderName);
	DDX_Control(pDX, IDC_STATIC_ACTUAL_STATUS, m_stActualStatus);
	DDX_Control(pDX, IDC_STATIC_FILESCANNED, m_stFilescanned);
	DDX_Control(pDX, IDC_STATIC_STARTTIME, m_stStartTime);
	DDX_Control(pDX, IDC_STATIC_THREADFOUND, m_stThreadFound);
	DDX_Control(pDX, IDC_STATIC_ELAPSED_TIME, m_stElapsedTime);
	DDX_Control(pDX, IDC_BUTTON_PAUSE_RESUME, m_btnPauseResume);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BUTTON_CLEAN, m_btnClean);
	DDX_Control(pDX, IDC_LIST_VIRUSLIST_CONTROL, m_lstVirusTrack);
	DDX_Control(pDX, IDC_STATIC_HIDE_DETAILS, m_stHideDetails);
	DDX_Control(pDX, IDC_STATIC_SHOWDETAILS, m_stShowDetails);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnUsbClose);
	DDX_Control(pDX, IDC_BUTTON_MINIMIZE, m_btnUsbMinimize);
	DDX_Control(pDX, IDC_STATIC_HIDE_DETAILS_FORDIALOG, m_stHideDetailsText);
	DDX_Control(pDX, IDC_STATIC_SHOWDETAILS_ONDIALOG, m_stShowDetailstext);
	DDX_Control(pDX, IDC_STATIC_HEDAER_TEXT, m_stHeaderText);
	DDX_Control(pDX, IDC_STATIC_LOGO_PIC, m_stHeaderLogo);
	DDX_Control(pDX, IDC_STATIC_USB_GIF, m_stUsbGifloader);
	DDX_Control(pDX, IDC_PROGRESS_USBSTATUS, m_prgUsbProgressbar);
	DDX_Control(pDX, IDC_STATIC_PERCENTAGE_VALUE, m_stUsbPercentage);
	DDX_Control(pDX, IDC_STATIC_PAUSE_RESUME_STATUS, m_stPauseAndResumeStatus);
	DDX_Control(pDX, IDC_STATIC_CLEANINGTEXT, m_stCleaningText);
	DDX_Control(pDX, IDC_EDIT_ACTUAL_STATUS, m_edtActualStatus);
	DDX_Control(pDX, IDC_CHECK_SELECT_ALL, m_chkSelectAll);
	DDX_Control(pDX, IDC_STATIC_SELECT_ALL, m_stSelectAll);
}

/**********************************************************************************************************                     
*  Function Name  :	MESSAGE_MAP                                                     
*  Description    :	Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0028
**********************************************************************************************************/
BEGIN_MESSAGE_MAP(CUSBDetectUIDlg, CJpegDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CUSBDetectUIDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_MINIMIZE, &CUSBDetectUIDlg::OnBnClickedButtonMinimize)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_VIRUSLIST_CONTROL, &CUSBDetectUIDlg::OnNMCustomdrawListViruslistControl)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_MESSAGE(DRIVEREMOVALSCANSTATUS, OnDriveRemovalStatus)
	ON_BN_CLICKED(IDC_CHECK_SELECT_ALL, &CUSBDetectUIDlg::OnBnClickedCheckSelectAll)
END_MESSAGE_MAP()


HWINDOW   CUSBDetectUIDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CUSBDetectUIDlg::get_resource_instance() { return theApp.m_hInstance; }

// CUSBDetectUIDlg message handlers
/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft Foundation Class Library dialog boxes
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0029
**********************************************************************************************************/
BOOL CUSBDetectUIDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	// Add "About..." menu item to system menu.
	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_THREE);  // to register service for process protection

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_THREE);//WLSRV_ID_THREE to register service for process protection

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_ABOUT_WARDWIZ"));
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	ModifyStyle(0, WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//Run the pipe server here
	g_objCommServer.Run();
	StartWardWizAVTray();
	ReadUISettingFromRegistry();
	m_hScanDllModule = NULL;
	if (!LoadRequiredDllForUSBScan())
	{
		AddLogEntry(L"### Error in Loading VBSCANDLL.DLL", 0, 0, true, SECONDLEVEL);
		CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ERROR_LOAD_WRDWIZSCANDLL"));
		return FALSE;
	}
	bool bUSBScan = false, bFolderScan = false, bQuickFullScan = false, bEPSScan = false;
	int iStartPos = 0, iSpacePos = 0;
	CString csScanOption = L"";
	CString csCmdLineText;
	CString csTaskName = L"";
	DWORD dwScanType = 0;
	CString csEPSNOUIToken = L"-EPSNOUI";
	CString csScanTypeToken = L"";
	CString csTIDToken = L"-TID:";
	CString csCommandTaskID = L"";
	CString csCommandLine = GetCommandLine();
	//csCommandLine.MakeUpper();
	CString csCmdForAddLogEntry = L"";
	csCmdForAddLogEntry = csCommandLine;

	if (csCommandLine.GetLength() > 0)
	{
		if (csCommandLine.CompareNoCase(TEXT("VBUSBDETECTUI.EXE")) >= 0)
		{
			csCommandLine.Replace(TEXT("VBUSBDETECTUI.EXE"), L"");
			m_bEPSCustomScan = true;
			csCommandLine.Trim();
		}
		if (csCommandLine.Find(TEXT("-SCHEDSCAN")) != -1)
		{
			csCommandLine.Replace(TEXT("-SCHEDSCAN"), L"");
			csCommandLine.Trim();
			if (csCommandLine.Find(TEXT("-SDNO")) != -1)
			{
				m_bIsShutDownScan = false;
				csCommandLine.Replace(TEXT("-SDNO"), L"");
			}
			if(csCommandLine.Find(TEXT("-SDYES")) != -1)
			{
				m_bIsShutDownScan = true;
				csCommandLine.Replace(TEXT("-SDYES"), L"");
			}
			csCommandLine.Trim();
		}
		if ((dwScanType = getScanType(csCommandLine)) == -1)
		{
			AddLogEntry(L"### Invalid scan type: [%s]", csCmdForAddLogEntry, 0, true, SECONDLEVEL);
			PostQuitMessage(WM_QUIT);
			return false;
		}
		switch (dwScanType)
		{
			case FULLSCAN: 
				csScanTypeToken = L"-FULLSCAN";
				m_eScanType = FULLSCAN;
				m_dwScanOption = 0x03;
				bQuickFullScan = true;
				bFullScan = true;
				break;
			case QUICKSCAN:
				csScanTypeToken = L"-QUICKSCAN";
				m_eScanType = QUICKSCAN;
				m_dwScanOption = 0x04;
				bQuickFullScan = true;
				bQuickScan = true;
				break;
			case CUSTOMSCAN:
				csScanTypeToken = L"-CUSTOMSCAN";
				this->SetWindowText(L"VBCUSTOMSCAN");
				m_eScanType = CUSTOMSCAN;
				m_dwScanOption = 0x02;
				bFolderScan = true;
				break;
			case USBSCAN:
				csScanTypeToken = L"USBSCAN";
				m_eScanType = USBSCAN;
				m_dwScanOption = 0x01;
				bUSBScan = true;
				break;
			default:
				break;
		}
		if (isEPSNOUIInCommand(csCommandLine))
		{
			bEPSScan = true;
			csCommandLine.Replace(csEPSNOUIToken, L"");
			csCommandLine.Trim();
			m_bEPSCustomScan = true;
		}
		else
		{
			bEPSScan = false;
		}
		if (bEPSScan == false && dwScanType != 1 && bUSBScan == false)
		{
			//return false;  quick or full scan can't be without -EPSNOUI cmd
			AddLogEntry(L"### Invalid command: In Quick Scan or Full Scan -EPSNOUI should be there [%s]", csCmdForAddLogEntry, 0, true, SECONDLEVEL);
			PostQuitMessage(WM_QUIT);
			return false;
		}
		if (isTaskIDInCommand(csCommandLine))
		{
			csTaskName = csCommandLine; 
			int iEnd = csTaskName.Find(csTIDToken);
			iEnd++;
			if (iEnd > 1 && m_eScanType == 1)
			{

				CString csTaskID = L"";
				int iFirstSpace = 0;
				csTaskID = csTaskName;
				csTaskID.Replace(csScanTypeToken, L"");
				csTaskID.Replace(csTIDToken, L"");
				csTaskID.Trim();
				iFirstSpace = csTaskID.Find(TEXT(" "));
				csTaskID.Truncate(iFirstSpace);
				csCommandTaskID = csTaskID;
			}
			else if (iEnd > 1)
			{
				csCommandTaskID = csTaskName.Tokenize(csTIDToken, iEnd);
			}
			else
			{
				csTaskName.Replace(csTIDToken, L"");
				csTaskName.Trim();
				int iSpace= csTaskName.Find(' ');
				csTaskName.Truncate(iSpace);
				csCommandTaskID = csTaskName;
			}
			csCommandLine.Replace(csTIDToken, L"");
			csCommandLine.Replace(csCommandTaskID, L"");
			csCommandLine.Trim();
		}
	}
	else
	{
		AddLogEntry(L"### Invalid command: [%s]", csCmdForAddLogEntry, 0, true, SECONDLEVEL);
		PostQuitMessage(WM_QUIT);
		return false;
	}

	CString csCommandLineForUSBRemove = L"";
	CString csCommandVal = L"";
	if (csCommandLine.Find('-') != -1)
	{
		csCommandLine.Delete(0, csCommandLine.Find('-')+1);
		iSpacePos = 0;

		if (!bQuickFullScan)
		{
			csCommandLine.Trim();
			iSpacePos = csCommandLine.Find(_T(' '), iStartPos);
			if (csCommandLine.Find('-') != -1)
			{
				int iDashPosition = 0, iSpacePosition = 0;
				iDashPosition = csCommandLine.Find('-', 0);
				iSpacePosition = csCommandLine.Find(_T(' '), iDashPosition);
				csCommandVal = csCommandLine;
				csCommandVal.Delete(0, iDashPosition+1);
				int iPosition = csCommandVal.Find(_T(' '), 0);
				csCommandVal.Delete(iPosition, csCommandVal.GetLength());
				if(CheckForWardWizCommand(csCommandVal))
				{
					csCommandLine.Delete(iDashPosition, (iSpacePosition - iDashPosition));
				}
			}
			csScanOption = csCommandLine;
			csCommandLineForUSBRemove = csScanOption;
			csCommandLine.Truncate(iSpacePos);
		}
		csCommandLine.Trim();
		csTaskName.Trim();
	}
	int iPos = 0;
	int icurPos = 0;
	if (csCommandTaskID.GetLength())
		m_csTaskID = csCommandTaskID;
	iPos = csCommandLineForUSBRemove.Find(_T(" "));
	if (!bQuickFullScan)
	{
		if (iPos <= 0)
		{
			return FALSE;
		}
	}
	m_csUSBDriveName = L"";
	m_csUSBDriveName = csCommandLineForUSBRemove.Right(csCommandLineForUSBRemove.GetLength() - iPos);
	m_csUSBDriveName = m_csUSBDriveName.Tokenize(L";", icurPos);
	m_csUSBDriveName.Trim();

	DWORD dwUsbScan = 0;
	CITinRegWrapper objReg;
	if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwUsbScan", dwUsbScan) != 0x00)
	{
		AddLogEntry(L"### Failed GetRegistryDWORDData dwUsbScan in CUSBDetectUIDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);;
	}

	if (bUSBScan)		
	{
		if(dwUsbScan == 0x02)
		{
			m_bUSBAutoScan = true;
		}
		else
		{
			CUSBPopupMsgDlg objUSBPopupMsgDlg;
			::BringWindowToTop(objUSBPopupMsgDlg.m_hWnd);
			int iRet = static_cast<int>(objUSBPopupMsgDlg.DoModal());
			if (iRet == IDCANCEL)
			{
				OnCancel();
				return FALSE;
			}
		}
	}

	if (!bUSBScan && !bFolderScan && !bQuickScan && !bFullScan)
	{
		return FALSE;
	}
	if (!theApp.ShowRestartMsgOnProductUpdate())
	{
		return 0;
	}

	//Check here DB files are present or not
	if (!Check4DBFiles())
	{
		OnCancel();
		return FALSE;
	}
	int Pos = csScanOption.Find(_T(" "));
	if (!bQuickFullScan)
	{
		m_csEnumFolderORDriveName = L"";
		m_csEnumFolderORDriveName = csScanOption.Right(csScanOption.GetLength() - Pos);
		m_csEnumFolderORDriveName.Trim();
		TokenizePath(m_csEnumFolderORDriveName);
	}
	CString	csWardWizModulePath = GetWardWizPathFromRegistry();
	CString	csWardWizReportsPath = L"";
	csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
	if (!PathFileExists(csWardWizReportsPath))
	{
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		m_objSqlDb.SetDataBaseFilePath(dbPath.m_psz);
		m_objSqlDb.Open();
		m_objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProdID);
		m_objSqlDb.Close();
	}

	//issue no 415 neha Gharge.
	for (int iIndex = 0; iIndex < m_csaEnumfolders.GetCount(); iIndex++)
	{
		if (!PathFileExists(m_csaEnumfolders.GetAt(iIndex)))
		{
			DWORD dwLastError = GetLastError();
			CString csMsgPathDoesntExist;
			if (dwLastError == ERROR_SHARING_VIOLATION)
			{
				csMsgPathDoesntExist.Format(L"%s %s", m_csaEnumfolders.GetAt(iIndex), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_PATH_ACCESS_DENIED"));
			}
			else
			{
				//issue no 878 Neha Gharge
				csMsgPathDoesntExist.Format(L"%s %s", m_csaEnumfolders.GetAt(iIndex), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_NOT_AVAILABLE_SCAN"));
			}
			// This call is commented for issue no. 2786.
			//CallNotificationMessage(1, (SCITER_STRING)csMsgPathDoesntExist);
			if (theApp.m_bRetval == true)
			{
				OnCancel();
				return FALSE;
			}
		}
	}

	if (!theApp.SendData2ComService(RELOAD_REGISTARTION_DAYS))
	{
		AddLogEntry(L"### Failed to SendData2ComService RELOAD_REGISTARTION_DAYS in CUSBDetectUIDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	m_tsScanStartTime = CTime::GetCurrentTime();
	Sleep(1000);

	//***************Issue No :414 neha gharge 26/5/2014 *******************************************/
	memset(m_szUSBdetectedPath, 0, sizeof(m_szUSBdetectedPath));
	GetShortPathName(m_csEnumFolderORDriveName, m_szUSBdetectedPath, 60);
	CString csTime = m_tsScanStartTime.Format(_T("%H:%M:%S"));
	AddLogEntry(_T("Start Scan Time: %s"), csTime);
	m_csStartUpTime.Format(L"%s: %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_START_TIME"), csTime);
	m_bOnWMClose = false;
	SetTimer(TIMER_SCAN_STATUS, 500, NULL);  // call OnTimer function
	m_hUSBScanThread = NULL;
	DWORD m_dwThreadId = 0;
	HideAllElements();
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document

	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_USB_SCAN.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_USB_SCAN.htm");
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();//Here its called i guess, -  yes. And you don need m_root_el, youcan  call root() for that. ok
	if(bEPSScan)
	{
		const SCITER_VALUE result = m_root_el.call_function("CallOnScanEPS");
		::MoveWindow(this->get_hwnd(), 0, 0, 0, 0, true);
		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	}
	else if (m_bUSBAutoScan)
	{
		CString csDriveNameVal = m_csUSBDriveName;
		csDriveNameVal.Replace(L"|", L"\\");
		::MoveWindow(this->get_hwnd(), 0, 0, 0, 0, true);
		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
		if (!SendData2Tray(SEND_USB_AUTOSCAN, USB_AUTO_SCAN_START, csDriveNameVal.GetBuffer()))
		{
			AddLogEntry(L"### Exception in CUSBDetectUIDlg::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
		}
	}
	else
	{
		SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
		SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);
		m_root_el.call_function("getScanType", m_dwScanOption);
		::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	}
	//initial window point center
	CenterWindow();
	this->SetWindowText(L"VBUSB");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/**********************************************************************************************************
*  Function Name  :	isEPSNOUIInCommand
*  Description    :	Helper function to get EPSNOUI command
*  Author Name    : Amol J.
*  Date           : 15 Mar 2018
**********************************************************************************************************/
bool CUSBDetectUIDlg::isEPSNOUIInCommand(CString csCommandLine)
{
	try
	{
		return (csCommandLine.Find(TEXT("-EPSNOUI")) != -1);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::isEPSNOUIInCommand", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	getScanType
*  Description    :	Helper function to get scan type.
*  Author Name    : Amol J.
*  Date           : 15 Mar 2018
**********************************************************************************************************/
DWORD CUSBDetectUIDlg::getScanType(CString csCommandLine)
{
	try
	{
		if (csCommandLine.Find(TEXT("-FULLSCAN")) != -1)
			return FULLSCAN;
		else if (csCommandLine.Find(TEXT("-QUICKSCAN")) != -1)
			return QUICKSCAN;
		else if (csCommandLine.Find(TEXT("-CUSTOMSCAN")) != -1)
			return CUSTOMSCAN;
		else if (csCommandLine.Find(TEXT("-USBSCAN")) != -1)
			return USBSCAN; //for schedule scan
		else
			return -1; //invalid scan type
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::getScanType", 0, 0, true, SECONDLEVEL);
	}
	return -1;
}

bool CUSBDetectUIDlg::isTaskIDInCommand(CString csCommandLine)
{
	try
	{
		return (csCommandLine.Find(TEXT("-TID:")) != -1); //return Task ID
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::isTaskIDInCommand", 0, 0, true, SECONDLEVEL);
	}
	return false;
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
		AddLogEntry(L"### Exception in value CVibraniumUpdates::GetSQLiteDBFilePath", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 CUSBDetectUIDlg::InsertDataToTable(const char* szQuery)
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
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : OnStartUSBScan
*  Description    : Function to Find threats
*  Author Name    : Amol Jaware
*  Date			  : 4 July 2016
****************************************************************************************************/
json::value CUSBDetectUIDlg::OnStartUSBScan(SCITER_VALUE svAddVirusFoundEnteryCB, SCITER_VALUE svScanFinishedStatusCB, SCITER_VALUE svFilePathStatusCB, SCITER_VALUE svUSBdetectedDriveCB, SCITER_VALUE svPercentageCB)
{
	try
	{
		m_svAddVirusFoundEnteryCB = svAddVirusFoundEnteryCB;
		m_svScanFinishedStatusCB = svScanFinishedStatusCB;
		m_svFilePathStatusCB = svFilePathStatusCB;
		m_svUSBdetectedDriveCB = svUSBdetectedDriveCB;
		m_svPercentageCB = svPercentageCB;
		m_hThreadUSbScanCount = ::CreateThread(NULL, 0, GetScanUSBFilesCount, (LPVOID) this, 0, NULL);
		m_hUSBScanThread = ::CreateThread(NULL, 0, USBScanThread, (LPVOID) this, 0, NULL);

		DWORD dwQuarantineOpt;
		DWORD dwHeuristicOpt;
		bool  bHeuristicOpt = false;
		GetWardwizRegistryDetails(dwQuarantineOpt, dwHeuristicOpt);
		if (dwHeuristicOpt == 1)
			bHeuristicOpt = true;

		SCANTYPE eScanType;
		if (m_dwScanOption == 0x01)
		{
			eScanType = USBSCAN;
		}
		else if (m_dwScanOption == 0x02)
		{
			eScanType = CUSTOMSCAN;
		}
		else if (m_dwScanOption == 0x03)
		{
			eScanType = FULLSCAN;
		}
		else if (m_dwScanOption == 0x04)
		{
			eScanType = QUICKSCAN;
		}
		// Add entries into Database..
		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		m_objSqlDb.SetDataBaseFilePath(dbPath.m_psz);
		CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),%d,%d,%d,%d,%d );"), eScanType, m_dwFileScanned, m_iThreatsFoundCount, dwQuarantineOpt, bHeuristicOpt, m_dwTotalThreatsCleaned);
		CT2A ascii(csInsertQuery, CP_UTF8);
		m_objSqlDb.Close();
		m_iScanSessionId = InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnStartUSBScan", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return (m_bDeviceDetached);
}

/***************************************************************************************************
*  Function Name  : On_CloseButton
*  Description    : Function to close UI.
*  Author Name    : Amol Jaware
*  Date			  : 4 July 2016
****************************************************************************************************/
json::value CUSBDetectUIDlg::On_CloseButton(SCITER_VALUE svbIsManualStop)
{
	try
	{
		m_bIsManualStop = svbIsManualStop.get(false);
		OnBnClickedButtonClose();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_OnCloseButton", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	OnSysCommand                                                     
*  Description    :	The framework calls this member function when the user selects 
				    a command from the Control menu, or when the user selects the 
				    Maximize or the Minimize button.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0030
**********************************************************************************************************/
void CUSBDetectUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

/**********************************************************************************************************                     
*  Function Name  :	OnPaint                                                     
*  Description    :	The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0031
**********************************************************************************************************/
void CUSBDetectUIDlg::OnPaint()
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
		CJpegDialog::OnPaint();
	}
}
/**********************************************************************************************************                     
*  Function Name  :	OnQueryDragIcon                                                     
*  Description    :	The framework calls this member function by a minimized
				   (iconic) window that does not have an icon defined for its class
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0032
**********************************************************************************************************/
HCURSOR CUSBDetectUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0033
**********************************************************************************************************/
HBRUSH CUSBDetectUIDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	 HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_SCANNING_TEXT  ||
		ctrlID == IDC_STATIC_STATUS_TEXT	||
		ctrlID == IDC_STATIC_HIDE_DETAILS_FORDIALOG ||
		ctrlID == IDC_STATIC_SHOWDETAILS_ONDIALOG ||
		ctrlID == IDC_STATIC_HEDAER_TEXT ||
		ctrlID ==IDC_STATIC_CLEANINGTEXT
     )
	 {
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonClose                                                     
*  Description    :	Handler when Close button is clicked.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0035
**********************************************************************************************************/
void CUSBDetectUIDlg::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	try
	{
		m_bClose = true;
		m_bScanningAborted = true;
		m_objSqlDb.Close();
		KillTimer(TIMER_SCAN_STATUS);
		if (ShutdownScanning(m_bClose))
		{
			if (!m_bIsCleaning)
			{
				if (m_iThreatsFoundCount == 0)
				{
					if (m_bScanningCompleted == false)
					{
						AddEntriesInReportsDB(L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED"));
						if (!m_objDBManager.SaveEntries(REPORTS))
						{
							AddLogEntry(L"### Failed to CUSBDetectUIDlg::OnBnClickedButtonClose in REPORTS", 0, 0, true, SECONDLEVEL);
						}
					}
				}
				else if (m_iThreatsFoundCount > 0 && m_bScanningAborted == false)
				{
					if (m_bScanningCompleted == false)
					{
						AddEntriesInReportsDB(L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED"));
						if (!m_objDBManager.SaveEntries(REPORTS))
						{
							AddLogEntry(L"### Failed to CUSBDetectUIDlg::OnBnClickedButtonClose in REPORTS", 0, 0, true, SECONDLEVEL);
						}
					}
				}

			}
			if (m_iThreatsFoundCount > 0 && m_bIsCleaning == false && m_bClose)//&& m_bScanningAborted == true)
			{
				SCITER_VALUE svArrayEntries = m_root_el.call_function("GetArrayOfDetectedEntries");
				bool bIsArray = false;
				svArrayEntries.isolate();
				bool IsbIsAnyEntryDetectedInArray = false;
				bIsArray = svArrayEntries.is_array();
				for (int iIndex = 0; iIndex < svArrayEntries.length(); iIndex++)
				{
					const SCITER_VALUE svEachEntery = svArrayEntries[iIndex]; //amol
					const std::wstring szActionTaken = svEachEntery[L"ActionTaken"].get(L"");
					CString csStatus = L"";
					csStatus.Format(L"%s", szActionTaken.c_str());
					if (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"))
					{
						IsbIsAnyEntryDetectedInArray = true;
						break;
					}
				}
				if (IsbIsAnyEntryDetectedInArray)
				{
					CString csThreatDetectMsg;
					csThreatDetectMsg.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_THREAT_DETECTED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_WANNA_CLOSE"));
					if (!m_bNoActionafterScanComplete)
					{
						m_dwIsAborted = 1; //aborted
						m_svScanFinishedStatusCB.call();
						SendScanFinishedData2EPSClient(m_dwIsAborted);
						CallNotificationMessage(3, (SCITER_STRING)csThreatDetectMsg);
						if (theApp.m_bRetval == true)
						{
							OnCancel();
						}
					}
				}
				else
				{
					if (!m_bIsManualStop)
					{
						OnCancel();
					}
					else 
					{
						m_dwIsAborted = 1; //aborted
						m_svScanFinishedStatusCB.call();
						SendScanFinishedData2EPSClient(m_dwIsAborted);
					}

					if (m_bIsCloseFrmTaskBar)
					{
						OnCancel();
					}
				}
			}
			else if (m_bIsCleaning && m_bClose)
			{
				if (m_hQuarantineUSBThread != NULL)
				{
					if (SuspendThread(m_hQuarantineUSBThread) == -1)
					{
						CString csErrorMsg = L"";
						DWORD ErrorCode = GetLastError();
						csErrorMsg.Format(L"### Failed to Suspend QuarantineThread in CUSBDetectUIDlg::OnbtnClickClose with GetLastError code %d", ErrorCode);
						AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
					}
					AddLogEntry(L">>> Paused QuarantineThread.", 0, 0, true, ZEROLEVEL);
				}
				CString csCleanProcess = L"";
				csCleanProcess.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_CLEANING_INPROCESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_WANNA_CLOSE"));
				CallNotificationMessage(2, (SCITER_STRING)csCleanProcess);
				if (theApp.m_bRetval == false)
				{
					if (m_hQuarantineUSBThread != NULL)
					{
						if (ResumeThread(m_hQuarantineUSBThread) == -1)
						{
							CString csErrorMsg = L"";
							DWORD ErrorCode = GetLastError();
							csErrorMsg.Format(L"### Failed to Resume QuarantineThread in CUSBDetectUIDlg::OnbtnClickClose with GetLastError code %d", ErrorCode);
							AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
						}
						AddLogEntry(L">>> Resumed QuarantineThread.", 0, 0, true, ZEROLEVEL);
					}
					return;
				}
				else
				{
					if (m_hQuarantineUSBThread != NULL)
					{
						if (TerminateThread(m_hQuarantineUSBThread, 0) == FALSE)
						{
							CString csErrorMsg = L"";
							DWORD ErrorCode = GetLastError();
							csErrorMsg.Format(L"### Failed to Terminate QuarantineThread in CScanDlg::CloseCleaning with GetLastError code %d", ErrorCode);
							AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
						}
						m_hQuarantineUSBThread = NULL;
						AddLogEntry(L">>> Terminated QuarantineThread.", 0, 0, true, ZEROLEVEL);
					}
					if (!m_objDBManager.SaveEntries(REPORTS))
					{
						AddLogEntry(L"### Failed to CUSBDetectUIDlg::OnBnClickedButtonClose in REPORTS", 0, 0, true, SECONDLEVEL);
					}
					OnCancel();
				}
			}
			else
			{
				OnCancel();
			}
		}
		m_bIsCleaning = false;
		m_bMsgAbortPopup = false;
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnBnClickedButtonClose", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_OnBnClickedButtonMinimize
*  Description    : Function to minimize ui when click on minimize button
*  Author Name    : Amol Jaware
*  Date			  : 4 July 2016
****************************************************************************************************/
json::value CUSBDetectUIDlg::On_OnBnClickedButtonMinimize()
{
	try
	{ 
		OnBnClickedButtonMinimize();
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_OnBnClickedButtonMinimize", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonMinimize                                                     
*  Description    :	Minimize USB UI.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0036
**********************************************************************************************************/
void CUSBDetectUIDlg::OnBnClickedButtonMinimize()
{
	this->ShowWindow(SW_MINIMIZE);
}

/**********************************************************************************************************                     
*  Function Name  :	LoadRequiredDllForUSBScan                                                     
*  Description    :	Load required dll files for USB Scan
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0037
**********************************************************************************************************/
bool CUSBDetectUIDlg::LoadRequiredDllForUSBScan()
{
	TCHAR szModulePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	CString	csWardWizScanDll = L"" ;
	csWardWizScanDll.Format( L"%s\\VBSCANDLL.DLL", szModulePath ) ;
	if( !PathFileExists( csWardWizScanDll ) )
	{
		CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_MODULE_NOT_FOUND"));
		exit(0);
	}

	if( !m_hScanDllModule )
	{
		m_hScanDllModule = LoadLibrary( csWardWizScanDll ) ;
		if(!m_hScanDllModule)
		{
			CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED_TO_LOAD_LIBRARY"));
			exit(0);
		}
	}
	
	m_lpLoadSigProc = (LOADSIGNATURE)GetProcAddress(m_hScanDllModule, "LoadSignatures");
	if(!m_lpLoadSigProc)
	{
		AddLogEntry(L"### CUSBDetectUIDlg: Error in GetProcAddress::LoadSignatures", 0, 0, true, SECONDLEVEL);
		FreeLibrary(m_hScanDllModule);
		m_lpLoadSigProc = NULL;
		m_hScanDllModule = NULL;
		return false;
	}

	m_lpUnLoadSigProc = (UNLOADSIGNATURES)GetProcAddress(m_hScanDllModule, "UnLoadSignatures");
	if(!m_lpUnLoadSigProc)
	{
		AddLogEntry(L"### CUSBDetectUIDlg: Error in GetProcAddress::UnLoadSignatures", 0, 0, true, SECONDLEVEL);
		FreeLibrary(m_hScanDllModule);
		m_hScanDllModule = NULL;
		m_lpUnLoadSigProc = NULL;
		return false;
	}

	m_lpScanFileProc = (SCANFILE)GetProcAddress(m_hScanDllModule, "ScanFile");
	if(!m_lpScanFileProc)
	{
		AddLogEntry(L"### CUSBDetectUIDlg: Error in GetProcAddress::ScanFile", 0, 0, true, SECONDLEVEL);
		FreeLibrary(m_hScanDllModule);
		m_hScanDllModule = NULL;
		m_lpScanFileProc = NULL;
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetWardwizRegistryDetails
*  Description    :	Read Scanning related options from registry
*  Author Name    : Gayatri A.
*  SR_NO		  :
*  Date           : 13 Sep 2016
**********************************************************************************************************/
bool CUSBDetectUIDlg::GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt)
{
	try
	{
		HKEY hKey;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, &hKey) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to open registry key in CWardwizScan::GetWardwizRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			return false;
		}

		DWORD dwOptionSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;

		long ReadReg = RegQueryValueEx(hKey, L"dwQuarantineOption", NULL, &dwType, (LPBYTE)&dwQuarantineOpt, &dwOptionSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to get registry key value for Quarantine Option in CWardwizScan::GetWardwizRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
		}

		ReadReg = RegQueryValueEx(hKey, L"dwHeuScan", NULL, &dwType, (LPBYTE)&dwHeuScanOpt, &dwOptionSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to get registry key value for Heuristic Scan Option in CWardwizScan::GetWardwizRegistryDetails, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::GetWardwizRegistryDetails", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	EnumFolder                                                     
*  Description    :	enumerate files of usb and sent it to scan.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0038
**********************************************************************************************************/
void CUSBDetectUIDlg::EnumFolder(LPCTSTR pstr)
{
	bool bIsFolder = false;
	try
	{
		if (!pstr)
			return;

		CString csVirusName(L"");
		DWORD dwISpywareID = 0;
		CFileFind finder;
		
		// build a string with wildcards
		CString strWildcard(pstr);		
		strWildcard += _T("\\*.*");

		CString csFilePath = pstr;
		if (GetDriveType((LPCWSTR)csFilePath) == DRIVE_CDROM)
		{
			if (m_bIsPathExist != true)
				m_bIsPathExist = false;
		}
		else
		{
			m_bIsPathExist = true;
		}


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

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bIsFolder = true;
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			if (dwSignatureFailedToLoad != 0)
			{
				bWorking = FALSE;
				AddLogEntry(L"%s", L"### Failed to Load Wardwiz Signature DataBase", L"", true, SECONDLEVEL);
				break;
			}

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString str = finder.GetFilePath();

				bool bIsSubFolderExcluded = false;
				if (m_objExcludeFilesFolders.ISExcludedPath((LPTSTR)str.GetString(), bIsSubFolderExcluded))
				{
					if (bIsSubFolderExcluded)
					{
						AddLogEntry(L">>> Excluded Path [%s] ", str, 0, true, ZEROLEVEL);
						continue;
					}
				}

				EnumFolder(str);
			}
			else
			{
				if (m_bDeviceDetached && m_iDriveCompareResult == 0)
				{
					break;
				}
				//CString csFilePath = finder.GetFilePath();
				csFilePath = finder.GetFilePath();
				if(csFilePath.Trim().GetLength() > 0)
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

					if(m_csPreviousFilePath != csFilePath)
					{
						bool bIsSubfolderExcluded = false;
						CString csFileExt;
						int iExtRevCount = csFilePath.ReverseFind('.');
						if (iExtRevCount > 0)
						{
							csFileExt = csFilePath.Right((csFilePath.GetLength() - iExtRevCount));
							csFileExt.Trim('.');

							if (m_objExcludeFilesFolders.ISExcludedFileExt((LPTSTR)csFileExt.GetString()))
							{
								AddLogEntry(L">>> Excluded File Extension [%s] ", csFileExt, 0, true, ZEROLEVEL);
								continue;
							}
						}
						if (m_objExcludeFilesFolders.ISExcludedPath((LPTSTR)csFilePath.GetString(), bIsSubfolderExcluded))
						{
							AddLogEntry(L">>> Excluded Path [%s] ", csFilePath, 0, true, ZEROLEVEL);
							continue;
						}
						if(PathFileExists(csFilePath) || PathIsNetworkPath(csFilePath))
						{		
							DWORD dwActionTaken = 0x00;
							DWORD dwISpyID = 0;
							TCHAR szVirusName[MAX_PATH] = {0};
							//Varada Ikhar, Date: 19/02/2015, Issue: Database needs to be updated. Database not valid.
							dwSignatureFailedToLoad = 0; 

							if ((CheckFileIsInRepairRebootIni(csFilePath)) ||
								(CheckFileIsInRecoverIni(csFilePath)) || (CheckFileIsInDeleteIni(csFilePath))								)
								continue;

							CString csActionId;
							m_dwFileScanned++;
							if (ScanFile(csFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, dwActionTaken, m_bRescan))
							{
								if(dwISpyID >= 0)
								{
									csVirusName = szVirusName;
									dwISpywareID = dwISpyID;
									
									CString csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
									csActionId = "IDS_FILE_SKIPPED";
									switch (dwActionTaken)
									{
									case FILESKIPPED:
										csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
										csActionId = "IDS_FILE_SKIPPED";
										break;
									case FILEQURENTINED:
										csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED");
										csActionId = "IDS_CONSTANT_THREAT_QUARANTINED";
										m_dwTotalThreatsCleaned++;
										break;
									case FILEREPAIRED:
										csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED");
										csActionId = "IDS_CONSTANT_THREAT_REPAIRED";
										m_dwTotalThreatsCleaned++;
										break;
									case LOWDISKSPACE:
										csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE");
										csActionId = "IDS_CONSTANT_THREAT_LOWDISC_SPACE";
										break;
									case FILEREBOOTQUARENTINE:
										csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE");
										m_dwTotalThreatsCleaned++;
										break;
									case FILEREBOOTREPAIR:
										csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR");
										m_dwTotalThreatsCleaned++;
										break;
									}

									if (theApp.m_dwProdID == ELITE)
									{
										if (!SendData2EPSClient(csFilePath, csVirusName.GetBuffer(), dwActionTaken))
										{
											AddLogEntry(L"### Failed to SendData to EPS Client in CUSBDetectUIDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
										}
									}

									m_iThreatsFoundCount++;
									if (HandleVirusFoundEntries(csVirusName, csFilePath, csStatus, dwISpywareID))
									{
										SCANTYPE eScanType;
										if (m_dwScanOption == 0x01)
										{
											eScanType = USBSCAN;
										}
										else
										{
											eScanType = CUSTOMSCAN;
										}
										// Add entries into Database..
										/*CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");

										csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%I64d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),'%s','%s','%s');"), m_iScanSessionId, csVirusName, csFilePath, csActionId);

										CT2A ascii(csInsertQuery, CP_UTF8);

										m_objSqlDb.Close();
										InsertDataToTable(ascii.m_psz);*/

										if (m_iThreatsFoundCount % 5 == 0)
										{
											if (m_bPlaySound)
											{
												AfxBeginThread(PlayThreatsFoundThread,(LPVOID) this);
											}
										}
										m_bIsExpanded = false;
									}
								
								}

								if (m_bUSBAutoScan)
								{
									ISPY_PIPE_DATA szPipeData = { 0 };
									szPipeData.iMessageInfo = SHOWONACCESSPOPUP;
									_tcscpy_s(szPipeData.szFirstParam, szVirusName);
									_tcscpy_s(szPipeData.szSecondParam, csFilePath);
									szPipeData.dwValue = dwActionTaken;

									HWND hWindow = ::FindWindow(NULL, L"VBUSBPOPUP");
									if (hWindow)
									{
										::PostMessage(hWindow, WM_CLOSE, 0, 0);
									}
									if (!g_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
									{
										CString csMessage;
										csMessage.Format(L"%d", szPipeData.iMessageInfo);
										AddLogEntry(L"### SendData failed in ProcessScanQueueThreadSEH, MessageID:%s", csMessage, 0, true, SECONDLEVEL);
									}
								}
							}
						}
					}
					//Ticket no 54 Neha Gharge when file name is long ext not visible m_stActualstatus to m_edtActualstatus
					m_csPreviousFilePath = csFilePath;
				}
				Sleep(10);
				//CallUISetStatusfunction(csFilePath, m_iThreatsFoundCount, m_dwFileScanned);
				CString csFileScanned;
				csFileScanned.Format(L"%s: %d",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"),m_dwFileScanned);
			}
		}
		finder.Close();

		if(!bWorking && !bIsFolder)
		{
			CString csFilePath = pstr;
			if(csFilePath.Trim().GetLength() > 0)
			{
				if(m_csPreviousFilePath != csFilePath)
				{
					if (PathFileExists(csFilePath) || PathIsNetworkPath(csFilePath))
					{
						bool bIsSubfolderExcluded = false;
						CString csFileExt;
						bool bFlag = false;
						int iExtRevCount = csFilePath.ReverseFind('.');
						if (iExtRevCount > 0)
						{
							csFileExt = csFilePath.Right((csFilePath.GetLength() - iExtRevCount));
							csFileExt.Trim('.');

							if (m_objExcludeFilesFolders.ISExcludedFileExt((LPTSTR)csFileExt.GetString()))
							{
								AddLogEntry(L">>> Excluded File Extension [%s] ", csFileExt, 0, true, ZEROLEVEL);
								bFlag = true;
							}
						}
						if (!bFlag)
						{
							if (!m_objExcludeFilesFolders.ISExcludedPath((LPTSTR)csFilePath.GetString(), bIsSubfolderExcluded))
							{
								DWORD dwActionTaken = 0x00;
								DWORD dwISpyID = 0;
								TCHAR szVirusName[MAX_PATH] = { 0 };

								if ((!CheckFileIsInRepairRebootIni(csFilePath)) &&
									(!CheckFileIsInRecoverIni(csFilePath)) && (!CheckFileIsInDeleteIni(csFilePath)))
								{
									m_dwFileScanned++;
									if (ScanFile(csFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, dwActionTaken, m_bRescan))
									{
										if (dwISpyID >= 0)
										{
											csVirusName = szVirusName;
											dwISpywareID = dwISpyID;
											CString csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
											switch (dwActionTaken)
											{
											case FILESKIPPED:
												csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
												break;
											case FILEQURENTINED:
												csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED");
												m_dwTotalThreatsCleaned++;
												break;
											case FILEREPAIRED:
												csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED");
												m_dwTotalThreatsCleaned++;
												break;
											case LOWDISKSPACE:
												csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE");
												break;
											case FILEREBOOTQUARENTINE:
												csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE");
												m_dwTotalThreatsCleaned++;
												break;
											case FILEREBOOTREPAIR:
												csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR");
												m_dwTotalThreatsCleaned++;
												break;
											}

											if (theApp.m_dwProdID == ELITE)
											{
												if (!SendData2EPSClient(csFilePath, csVirusName.GetBuffer(), dwActionTaken))
												{
													AddLogEntry(L"### Failed to SendData to EPS Client in CUSBDetectUIDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
												}
											}

											m_iThreatsFoundCount++;
											if (HandleVirusFoundEntries(csVirusName, csFilePath, csStatus, dwISpywareID))
											{
												m_bIsExpanded = false;
											}
										}
									}
								}
							}
							else
							{
								AddLogEntry(L">>> Excluded Path [%s] ", csFilePath, 0, true, ZEROLEVEL);
							}
						}
					}
				}
				m_csPreviousFilePath = csFilePath;
			}
			Sleep(10);
			//CallUISetStatusfunction(csFilePath, m_iThreatsFoundCount, m_dwFileScanned);
			CString csFileScanned;
			csFileScanned.Format(L"%s: %d",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"),m_dwFileScanned);
		}

		if(m_csPreviousFilePath.GetLength() == 0) //sending such folder path in which no file exist
			m_csPreviousFilePath = csFilePath;
	}
	catch(...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	ScanFile                                                     
*  Description    :	Exported funtion to scan file
					Scanning using service, the aim is to keep the database in common memory location.
*  Author Name    : Neha Gharge, Ram Shelke                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0039
**********************************************************************************************************/
bool CUSBDetectUIDlg::ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan)
{
	try
	{
		bool bSendFailed = false;


		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SCAN_FILE;
		wcscpy_s(szPipeData.szFirstParam, szFilePath);
		
		if (!m_objScanCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::ScanFile", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}

		if (!m_objScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CUSBDetectUIDlg::ScanFile", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}

		if (szPipeData.dwValue == 1)
		{
			dwActionTaken = szPipeData.dwSecondValue;
			_tcscpy(szVirusName, szPipeData.szSecondParam);
			dwISpyID = (*(DWORD *)&szPipeData.byData[0]);
			return true;
		}

		if (bSendFailed)
		{
			if (m_lpScanFileProc)
			{
				if (m_lpScanFileProc(szFilePath, szVirusName, dwISpyID, bRescan))
				{
					return true;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::ScanFile, File: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	HandleVirusFoundEntries                                                     
*  Description    :	handle virus found entry to insert into list control.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0040
**********************************************************************************************************/
bool CUSBDetectUIDlg::HandleVirusFoundEntries(CString strVirusName, CString strScanFileName, CString csAction, DWORD dwSpyID)
{
	try
	{
		CString csISpyID = L"";
		csISpyID.Format(L"%d", dwSpyID);
		InsertItem(strVirusName, strScanFileName, csAction, csISpyID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::HandleVirusFoundEntries, Virus: [%s] | File: [%s]", strVirusName, strScanFileName, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	USBScanThread                                                     
*  Description    :	Thread creats on initialization of dialog and send enumerated files for scanning
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0041
**********************************************************************************************************/
DWORD WINAPI USBScanThread(LPVOID lpvThreadParam)
{
	CUSBDetectUIDlg *pThis = (CUSBDetectUIDlg*)lpvThreadParam;

	if(!pThis)
		return 0;

	pThis->m_bISScanning = true;
	
	if (pThis->m_dwScanOption == 0x01)
	{
		AddLogEntry(L">>> USB scanning started...", 0, 0, true, SECONDLEVEL); //Varada Ikhar, Date:24/01/2015, Adding a log entry.
	}
	else
	{
		AddLogEntry(L">>> Custom scanning started...", 0, 0, true, SECONDLEVEL);
	}
	if (pThis->bQuickScan || pThis->bFullScan)
	{
		for(int iIndex = 0; iIndex < pThis->m_csaAllScanPaths.GetCount(); iIndex++)
		{
			memset(pThis->m_szUSBdetectedPath, 0, sizeof(pThis->m_szUSBdetectedPath));
			GetShortPathName(pThis->m_csaAllScanPaths.GetAt(iIndex), pThis->m_szUSBdetectedPath, 60);
			pThis->m_svUSBdetectedDriveCB.call((SCITER_STRING)pThis->m_szUSBdetectedPath);
			if ((PathIsNetworkPath(pThis->m_csaAllScanPaths.GetAt(iIndex))) && (pThis->m_dwScanOption == 0x02))
			{
				//pThis->KillTimer(TIMER_SCAN_STATUS);
				//pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_RIGHT_CUSTOM_MSG_NETWORK_PATH"));
				pThis->m_bISScanning = false;
				//pThis->OnBnClickedButtonClose();
				return 0;
			}
			else
			{
				pThis->EnumFolder(pThis->m_csaAllScanPaths.GetAt(iIndex));
			}
		}
	}
	else
	{
		for(int iIndex = 0; iIndex < pThis->m_csaEnumfolders.GetCount(); iIndex++)
		{
			memset(pThis->m_szUSBdetectedPath, 0, sizeof(pThis->m_szUSBdetectedPath));
			GetShortPathName(pThis->m_csaEnumfolders.GetAt(iIndex),pThis->m_szUSBdetectedPath,60);

			pThis->m_svUSBdetectedDriveCB.call((SCITER_STRING)pThis->m_szUSBdetectedPath);
			if (( PathIsNetworkPath(pThis->m_csaEnumfolders.GetAt(iIndex))) && (pThis->m_dwScanOption == 0x02))
			{
				//pThis->KillTimer(TIMER_SCAN_STATUS);
				pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_RIGHT_CUSTOM_MSG_NETWORK_PATH"));
				pThis->m_bISScanning = false;
				pThis->OnBnClickedButtonClose();
				return 0;
			}
			else
			{
				pThis->EnumFolder(pThis->m_csaEnumfolders.GetAt(iIndex));
			}
		}
	}
	Sleep(200);
	if (pThis->m_bDeviceDetached && pThis->m_iDriveCompareResult == 0 && (pThis->m_dwScanOption==0x01)) //only for usb
	{
		pThis->DeviceRemoved();
		if (!pThis->m_objDBManager.SaveEntries(REPORTS))
		{
			AddLogEntry(L"### Failed to CUSBDetectUIDlg::OnBnClickedButtonClose in REPORTS", 0, 0, true, SECONDLEVEL);
		}
		return 1;
	}
	pThis->m_bISScanning = false;
	pThis->USBScanningFinished();
	return 1;
}

/**********************************************************************************************************                     
*  Function Name  :	USBScanningFinished                                                     
*  Description    :	calls function when scan finishes.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0042
**********************************************************************************************************/
bool CUSBDetectUIDlg::USBScanningFinished()
{
	if(m_bISScanning)
		return false;

	Sleep(500); //slight delay is required to solved issue 6090 & 6091

	try
	{
		SaveLocalDatabase();

		if (m_bISScanning == false)
		{
			if (!m_bScanningAborted)
			{
				HWND hWindow = ::FindWindow(NULL, L"VBUSB");
				if (hWindow && !m_bUSBAutoScan)
				{
					::ShowWindow(hWindow, SW_RESTORE);
					::BringWindowToTop(hWindow);
					::SetForegroundWindow(hWindow);
				}
			}

			CString csScanningFinished;
			//KillTimer(TIMER_SCAN_STATUS);
			CTimeSpan		tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsScanStartTime) - m_tsScanPauseResumeElapsedTime;
			CString csTime = tsScanElapsedTime.Format(_T("%H:%M:%S"));
			CString csElapsedTime;
			csElapsedTime.Format(L"%s  %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_SCANTIME"), csTime);
			SetLastScanDateTime();
			if (dwSignatureFailedToLoad != 0)
			{
				int iLowRange = 0;
				if (m_bPlaySound)
				{
					AfxBeginThread(PlayScanFinishedThread, NULL);
				}
				AddEntriesInReportsDB(L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED"));
				if (m_dwScanOption == 0x01)
				{
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L"### Database needs to be Updated.(dwSignatureFailedToLoad !=0)");
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DATABASE_MESSAGE"));
				}
				else if (m_dwScanOption == 0x02)
				{
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L"### Database needs to be Updated.(dwSignatureFailedToLoad !=0)");
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DATABASE_MESSAGE"));
				}
				CallNotificationMessage(1, (SCITER_STRING)csScanningFinished);
				dwSignatureFailedToLoad = 0x00;
				if (!m_objDBManager.SaveEntries(REPORTS))
				{
					AddLogEntry(L"### Failed to CUSBDetectUIDlg::USBScanningFinished in REPORTS", 0, 0, true, SECONDLEVEL);
				}
				if (m_bMsgAbortPopup == false)
				{
					OnBnClickedButtonClose();
				}
			}
			else if (m_iThreatsFoundCount >= 0)
			{
				DWORD dwTotalRebootCount = 0x00;
				dwTotalRebootCount = CheckForDeleteFileINIEntries() + CheckForRepairFileINIEntries();
				if (dwTotalRebootCount)
				{
					CString csMsgToRebootSystem(L"");
					csMsgToRebootSystem.Format(L"%s %d %s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART1"), dwTotalRebootCount, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART2"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));
					CallNotificationMessage(2, (SCITER_STRING)csMsgToRebootSystem);
					if (theApp.m_bRetval == true)
					{
						//Write a code to restart computer.
						CEnumProcess enumproc;
						enumproc.RebootSystem(0);
					}
				}

				int iLowRange = 0, iHighRange = 0;
				m_dwIsAborted = 0;//finished
				//KillTimer(TIMER_SCAN_STATUS);
				m_bScanningCompleted = true;
				m_svScanFinishedStatusCB.call();
				if (theApp.m_dwProdID == ELITE)
				{
					SendScanFinishedData2EPSClient(m_dwIsAborted);
				}
				if (m_bPlaySound)
				{
					AfxBeginThread(PlayScanFinishedThread, NULL);
				}
				if (m_dwScanOption == 0x01)
				{
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_USB_COMPLETED"));
					AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L">>> %s : %d, %s : %d , %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VIRUS_FOUND"), m_iThreatsFoundCount, csElapsedTime);
					AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L"%s\n%s : %d"/*\n%s"*/, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_USB_COMPLETED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VIRUS_FOUND"), m_iThreatsFoundCount/*, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_CLEAN")*/);
				}
				else if (m_dwScanOption == 0x02)
				{
					if (m_bIsPathExist == false)
					{
						csScanningFinished.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_INVALID_SELECTION"));
					}
					else
					{
						AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
						csScanningFinished.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_USB_COMPLETED"));
						AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
						csScanningFinished.Format(L">>> %s : %d, %s : %d , %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VIRUS_FOUND"), m_iThreatsFoundCount, csElapsedTime);
						AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
						AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
						csScanningFinished.Format(L"%s\n%s : %d"/*\n%s"*/, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_CUSTOMSCAN_COMPLETED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VIRUS_FOUND"), m_iThreatsFoundCount/*, theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_CLEAN")*/);
					}
				}
				KillTimer(TIMER_SCAN_STATUS);
				AddEntriesInReportsDB(L"NA", L"NA", L"NA");
				if (!m_objDBManager.SaveEntries(REPORTS))
				{
					AddLogEntry(L"### Failed to CUSBDetectUIDlg::USBScanningFinished in REPORTS", 0, 0, true, SECONDLEVEL);
				}
				if (m_bEPSCustomScan)
				{
					OnCancel();
				}
				if (m_bUSBAutoScan)
				{
					CString csDriveNameVal = m_csUSBDriveName;
					csDriveNameVal.Replace(L"|", L"\\");
					OnCancel();
					if (!m_iThreatsFoundCount)
					{
						if (!SendData2Tray(SEND_USB_AUTOSCAN, USB_AUTO_SCAN_END, csDriveNameVal.GetBuffer()))
						{
							AddLogEntry(L"### Exception in CUSBDetectUIDlg::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
						}
					}
				}
				if (m_bIsShutDownScan == true)
				{
					CEnumProcess enumproc;
					enumproc.RebootSystem(1);
				}
				if (!dwTotalRebootCount)
					CallNotificationMessage(5, (SCITER_STRING)csScanningFinished);
			}
			else
			{
				int iLowRange = 0, iHighRange = 0;
				//KillTimer(TIMER_SCAN_STATUS);
				m_bScanningCompleted = true;
				if (m_bPlaySound)
				{
					AfxBeginThread(PlayScanFinishedThread, NULL);
				}
				AddEntriesInReportsDB(L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND"));
				if (m_dwScanOption == 0x01)
				{
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_USB_COMPLETED"));
					AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L">>> %s = %d, %s, %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"), csElapsedTime);
					AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL); //Varada Ikhar
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScanningFinished.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_USB_COMPLETED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"));
				}
				else if (m_dwScanOption == 0x02)
				{
					if (m_bIsPathExist == false)
					{
						csScanningFinished.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_INVALID_SELECTION"));
					}
					else
					{
						AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
						csScanningFinished.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_CUSTOMSCAN_COMPLETED"));
						AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
						csScanningFinished.Format(L">>> %s = %d, %s, %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"), csElapsedTime);
						AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL); //Varada Ikhar
						AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
						csScanningFinished.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_CUSTOMSCAN_COMPLETED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"));
					}
				}
				if (m_bDeviceDetached && m_iDriveCompareResult == 0 && m_dwScanOption == 0x01)
				{
					CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_REMOVED"));
				}
				else
				{
					m_dwIsAborted = 0;
					KillTimer(TIMER_SCAN_STATUS);
					m_svScanFinishedStatusCB.call();
					SendScanFinishedData2EPSClient(m_dwIsAborted);
					if (m_bIsShutDownScan == true)
					{
						CEnumProcess enumproc;
						enumproc.RebootSystem(1);
					}
					CallNotificationMessage(5, (SCITER_STRING)csScanningFinished);
				}
				if (m_bEPSCustomScan)
				{
					OnCancel();
				}
				if (m_bMsgAbortPopup == false)
				{
					OnBnClickedButtonClose();
				}
			}
			if (!m_iThreatsFoundCount)
			{
				OnBnClickedButtonClose();
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::USBScanningFinished", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	InsertItem                                                     
*  Description    :	Insert virus name ,virus file name ,ID and action into list control
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0043
**********************************************************************************************************/
void CUSBDetectUIDlg::InsertItem(CString strVirusName, CString strScanFileName, CString csAction, CString csISpyID)
{
	try
	{
		m_svAddVirusFoundEnteryCB.call((SCITER_STRING)strVirusName, (SCITER_STRING)strScanFileName, (SCITER_STRING)csAction, (SCITER_STRING)csISpyID);

		if (strScanFileName.GetLength() == 0 ||
			strVirusName.GetLength() == 0 ||
			csAction.GetLength() == 0 ||
			csISpyID.GetLength() == 0)
		{
			return;
		}
		
		CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");

		strVirusName.Replace(L"'", L"''");
		strScanFileName.Replace(L"'", L"''");

		csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%I64d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'),'%s','%s','%s',NULL);"), m_iScanSessionId, strVirusName, strScanFileName, csAction);

		CT2A ascii(csInsertQuery, CP_UTF8);
		InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::InsertItem", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonPauseResume                                                     
*  Description    :	Pause and resume scanning , according to written on button.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0044
**********************************************************************************************************/
void CUSBDetectUIDlg::OnBnClickedButtonPauseResume()
{
	try
	{
		CString csButtonText;
		m_objCom.Close();
		if (csButtonText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"))
		{
			if (m_hThreadUSbScanCount != NULL)
			{
				ResumeThread(m_hThreadUSbScanCount);
			}
			if (m_hUSBScanThread != NULL)
			{
				ResumeThread(m_hUSBScanThread);
			}
			//SetTimer(TIMER_SCAN_STATUS, 100, NULL);
			m_tsScanPauseResumeElapsedTime += ((CTime::GetCurrentTime() - m_tsScanPauseResumeTime));
			AddLogEntry(L"Scanning Resumed..", 0, 0, true, FIRSTLEVEL);
		}
		else if (csButtonText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"))
		{
			if (m_hThreadUSbScanCount != NULL)
			{
				SuspendThread(m_hThreadUSbScanCount);
			}
			if (m_hUSBScanThread != NULL)
			{
				SuspendThread(m_hUSBScanThread);
			}
			//OnTimer(TIMER_PAUSE_HANDLER);
			AddLogEntry(L"Scanning Paused..", 0, 0, true, FIRSTLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnBnClickedButtonPause", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_OnCleanButton
*  Description    : Function to clean threats.
*  Modified By    : Amol Jaware
*  Date			  : 4 July 2016
****************************************************************************************************/
json::value CUSBDetectUIDlg::On_OnCleanButton(SCITER_VALUE svArrRecords, SCITER_VALUE svUpdateThreatFoundEnteryCB, SCITER_VALUE svCleanFinishedCB)
{
	bool bIsArray = false;
	svArrRecords.isolate();
	bIsArray = svArrRecords.is_array();
	try
	{
		if (!bIsArray)
		{
			return false;
		}

		bool iSelected = false;
		m_svArrRecords = svArrRecords;
		m_svUpdateThreatFoundEnteryCB = svUpdateThreatFoundEnteryCB;
		m_svCleanFinishedCB = svCleanFinishedCB;
		for (int iIndex = 0; iIndex < m_svArrRecords.length(); iIndex++)
		{
			const SCITER_VALUE svEachEntery = m_svArrRecords[iIndex]; 
			const std::wstring szThreatName = svEachEntery[L"ThreatName"].get(L"");
			const std::wstring szFileName = svEachEntery[L"FilePath"].get(L"");
			const std::wstring szActionTaken = svEachEntery[L"ActionTaken"].get(L"");
			bool bValue = svEachEntery[L"selected"].get(false);
			CString csStatus = L"";
			csStatus.Format(L"%s", szActionTaken.c_str());
			if(bValue)
			{
				if (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED") || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED") || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE"))//csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND") 
				{
					iSelected = true;
					break;
				}
			}
		}

		if (!iSelected)
		{
			CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NOTSELECTEDFORENTRIES_CLEAN"));
			return 0;
		}

		//Check here for Evaluation
		if (!theApp.m_bAllowDemoEdition)
		{
			if (theApp.m_dwDaysLeft == 0)
			{
				theApp.GetDaysLeft();
			}

			if (theApp.m_dwDaysLeft == 0)
			{
				if (!theApp.ShowEvaluationExpiredMsg())
				{
					theApp.GetDaysLeft();
					return 0;
				}
			}
		}
		m_hQuarantineUSBThread = NULL;
		m_hQuarantineUSBThread = ::CreateThread(NULL, 0, QuarantineUSBThread, (LPVOID) this, 0, NULL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnBnClickedButtonClean.", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	QuarantineUSBThread                                                     
*  Description    : If user clicks on clean button.Quarantine thread gets called.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0046
**********************************************************************************************************/
DWORD WINAPI QuarantineUSBThread(LPVOID lpParam)
{
	try
	{
		CUSBDetectUIDlg *pThis = (CUSBDetectUIDlg *)lpParam;
		if (pThis == NULL)
			return 0;		
		pThis->QuaratineUSBEntries();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception inCUSBDetectUIDlg::QuarantineUSBThread", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/**********************************************************************************************************                     
*  Function Name  :	QuaratineUSBEntries                                                     
*  Description    :	Repaires or quarantines selected files one by one.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0047
**********************************************************************************************************/
void CUSBDetectUIDlg::QuaratineUSBEntries()
{
	AddLogEntry(L">>> USB quarantine started...", 0, 0, true, FIRSTLEVEL);
	m_bIsCleaning = true;
	DWORD		dwCleanCount = 0;
	CString		csThreatName, csThreatPath, csStatus, csISpyID;
	CString		csPathName = L"";

	DWORD		dwRebootRepair = 0x00;
	DWORD		dwQuarantine = 0x00;
	//CString csStatus = L"";
	DWORD dwVirusCount = 0x00;
	dwVirusCount = m_svArrRecords.length();

	try
	{
		for (int iIndex = 0; iIndex < m_csaEnumfolders.GetCount(); iIndex++)
		{
			if (!PathFileExists(m_csaEnumfolders.GetAt(iIndex)))
			{
				if (!PathIsNetworkPath(m_csaEnumfolders.GetAt(iIndex)))
				{
					if (m_dwScanOption == 0x01)
					{
						csPathName.Format(L"%s %s", m_csaEnumfolders.GetAt(iIndex), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_NOT_AVAILABLE"));
						CallNotificationMessage(1, (SCITER_STRING)csPathName);
						if (theApp.m_bRetval == false)
						{
							m_bIsCleaning = false;
							return;
						}
					}
					else if (m_dwScanOption == 0x02)
					{
						csPathName.Format(L"%s %s", m_csaEnumfolders.GetAt(iIndex), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_NOT_AVAILABLE"));
						CallNotificationMessage(1, (SCITER_STRING)csPathName);
						m_bIsCleaning = false;
						continue;
					}
				}
			}
		}
		if (!m_objRecoverDB.ClearDBEntries(RECOVER))
		{
			AddLogEntry(L"### Failed to ClearDBEntries in CUSBDetectUIDlg::QuaratineUSBEntries RECOVER", 0, 0, true, SECONDLEVEL);
		}

		if (!m_objDBManager.ClearDBEntries(REPORTS))
		{
			AddLogEntry(L"### Failed to ClearDBEntries in CUSBDetectUIDlg::QuaratineUSBEntries REPORTS", 0, 0, true, SECONDLEVEL);
		}

		if (m_svArrRecords.length() > 0)
		{
			//Issue neha Gharge 16/6/2014
			for (int iIndex = 0; iIndex < m_svArrRecords.length(); iIndex++)
			{
				const SCITER_VALUE svEachEntery = m_svArrRecords[iIndex];
				const std::wstring szThreatName = svEachEntery[L"ThreatName"].get(L"");
				const std::wstring szFileName = svEachEntery[L"FilePath"].get(L"");
				const std::wstring szActionTaken = svEachEntery[L"ActionTaken"].get(L"");
				const std::wstring szSpyID = svEachEntery[L"WardWizID"].get(L"");
				csStatus.Format(L"%s", szActionTaken.c_str());

				bool bCheck = svEachEntery[L"selected"].get(false);

				if (bCheck && (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED") || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE"))) //csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND") 
				{
					csThreatName.Format(L"%s", szThreatName.c_str());
					csThreatPath.Format(L"%s", szFileName.c_str());
					csISpyID.Format(L"%s", szSpyID);
					if (PathIsNetworkPath(csThreatPath))
					{
						m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR"));
						AddEntriesInReportsDB(csThreatName, csThreatPath, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR"));
						AddLogEntry(L"### Error in QuaratineUSBEntries: File %s is network path exist", csThreatPath, 0, true, SECONDLEVEL);
						continue;
					}

					if (!PathFileExists(csThreatPath))
					{
						m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND"));
						AddLogEntry(L"### Error in QuaratineUSBEntries: File %s does not exist", csThreatPath, 0, true, SECONDLEVEL);
						continue;
					}

					bool bDefaultRet = false;
					DWORD dwISpyID = 0;
					dwISpyID = _wtol((LPCTSTR)csISpyID);
					if (dwISpyID >= 0)
					{
						CString csEntryState;
						GetShortFilePath(csThreatPath);
						csEntryState.Format(L"%s:%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_FILE"), m_szCleaningShortPath);

						ISPY_PIPE_DATA szPipeData = { 0 };
						szPipeData.iMessageInfo = HANDLE_VIRUS_ENTRY;
						szPipeData.dwValue = dwISpyID;
						wcscpy_s(szPipeData.szFirstParam, csThreatPath);
						wcscpy_s(szPipeData.szSecondParam, csThreatName);

						if (m_dwScanOption == 0x01)
						{
							wcscpy_s(szPipeData.szThirdParam, L"USB Scan");
						}
						else
						{
							wcscpy_s(szPipeData.szThirdParam, L"RightClick Scan");
						}
						bool bSendReapir = SendFile4RepairUsingService(&szPipeData, true, true);

						DWORD dwShowEntryStatus = szPipeData.dwValue == 0x00 ? 0x00 : 0x01;

						switch (szPipeData.dwValue)
						{
						case 0x00:
							if (dwISpyID > 0)
							{
								m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
							}
							else
							{
								m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
							}

							break;

						case 0x04:
							m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR"));
							dwRebootRepair++;
							AddLogEntry(L"### Repair on Reboot File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
							break;

						case 0x05:
							m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE"));
							dwQuarantine++;
							AddLogEntry(L"### quarantine File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
							break;
						case 0x08:
							m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_ALREADY_REPAIRED"));
							AddLogEntry(L"### Already Repaired File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
							break;
						case 0x09:
							m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE"));
							AddLogEntry(L"### Low disc take a backup of file File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
							break;
						default:
							CString csFailedValue;
							csFailedValue.Format(L"%d", szPipeData.dwValue);
							AddLogEntry(L"### Repair failed file::%s with Error ::%s", csThreatPath, csFailedValue, true, SECONDLEVEL);
							if (dwISpyID > 0)
							{
								m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
							}
							else
							{
								m_svUpdateThreatFoundEnteryCB.call(iIndex, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
							}
							bDefaultRet = true;
							AddLogEntry(L"### Repair failed File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						}//end of switch case

						if (szPipeData.dwValue != 0x08)
						{
							if (!InsertRecoverEntry(csThreatPath, szPipeData.szFirstParam, csThreatName, dwShowEntryStatus))
							{
								AddLogEntry(L"### Error in InsertRecoverEntry", 0, 0, true, SECONDLEVEL);
							}

							CString csStatus;
							csStatus.Format(L"%s", szPipeData.szSecondParam);

							if (m_dwScanOption == 0x01)
							{
								AddEntriesInReportsDB(csThreatName, csThreatPath, csStatus);
							}
							else
							{
								AddEntriesInReportsDB(csThreatName, csThreatPath, csStatus);
							}
						}
						if (bDefaultRet == true)
						{
							szPipeData.dwValue = 0x00;
						}
						if (szPipeData.dwValue == 0x00)
						{
							dwCleanCount++;
						}
					}
				}
			}
		}
		AddLogEntry(L">>> Quarantine USB entries is completed.", 0, 0, true, ZEROLEVEL);
		m_bQuarantineFinished = true;
		m_bIsCleaning = false;
		m_svCleanFinishedCB.call();

		if (!m_objRecoverDB.SaveEntries(RECOVER))
		{
			AddLogEntry(L"### Failed to CUSBDetectUIDlg::USBScanningFinished in REPORTS", 0, 0, true, SECONDLEVEL);
		}

		if (!m_objDBManager.SaveEntries(REPORTS))
		{
			AddLogEntry(L"### Failed to CUSBDetectUIDlg::USBScanningFinished in REPORTS", 0, 0, true, SECONDLEVEL);
		}

		CString csTotalClean;
		csTotalClean.Format(L"%s\n\n%s: %d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_COMPLETED"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_CLEAN_COUNT"), dwCleanCount);
		CallNotificationMessage(1, (SCITER_STRING)csTotalClean);
		CString csMsgToRebootSystem(L"");
		if (ISAllItemsCleaned())
		{
			csMsgToRebootSystem.Format(L"%s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_MSG_ONCLEAN_PART1"), theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_MSG_ONCLEAN_PART2"));
			if (dwRebootRepair || dwQuarantine)
			{
				CallNotificationMessage(2, (SCITER_STRING)csMsgToRebootSystem);
				if (theApp.m_bRetval == true)
				{
					//Write a code to restart computer.
					CEnumProcess enumproc;
					enumproc.RebootSystem(0);
				}

			}
			else
			{

				DWORD dwTotalRebootCount = 0x00;

				dwTotalRebootCount = CheckForDeleteFileINIEntries() + CheckForRepairFileINIEntries();
				if (dwTotalRebootCount)
				{
					csMsgToRebootSystem.Format(L"%s %d %s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART1"), dwTotalRebootCount, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART2"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));
					CallNotificationMessage(2, (SCITER_STRING)csMsgToRebootSystem);
					if (theApp.m_bRetval == true)
					{
						//Write a code to restart computer.
						CEnumProcess enumproc;
						enumproc.RebootSystem(0);
					}

				}
			}

		}
		else
		{
			m_iThreatsFoundCount = 0;
			csMsgToRebootSystem.Format(L"%s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_MSG_ONCLEAN_PART1"), theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_MSG_ONCLEAN_PART2"));
			if (dwRebootRepair || dwQuarantine)
			{
				CallNotificationMessage(2, (SCITER_STRING)csMsgToRebootSystem);
				if (theApp.m_bRetval == true)
				{
					//Write a code to restart computer.
					CEnumProcess enumproc;
					enumproc.RebootSystem(0);
				}
			}
			else
			{

				DWORD dwTotalRebootCount = 0x00;

				dwTotalRebootCount = CheckForDeleteFileINIEntries() + CheckForRepairFileINIEntries();
				if (dwTotalRebootCount)
				{
					csMsgToRebootSystem.Format(L"%s %d %s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART1"), dwTotalRebootCount, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART2"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));
					CallNotificationMessage(2, (SCITER_STRING)csMsgToRebootSystem);
					if (theApp.m_bRetval == true)
					{
						//Write a code to restart computer.
						CEnumProcess enumproc;
						enumproc.RebootSystem(0);
					}

				}
			}
			OnCancel();
		}

	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::QuaratineEntries", 0, 0, true, SECONDLEVEL);
	}
	AddLogEntry(L">>> Quarantine Finished..", 0, 0, true, SECONDLEVEL);
}

/**********************************************************************************************************                     
*  Function Name  :	ShutdownScanning                                                     
*  Description    :	Shut down scanning with terminating all thread safely.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0049
**********************************************************************************************************/
bool CUSBDetectUIDlg::ShutdownScanning(bool bClose)
{
	try
	{
		if (!m_bISScanning)
		{
			return true;
		}

		m_objCom.Close();
		CString csButtonText = L"";
		if (m_hThreadUSbScanCount != NULL)
		{
			SuspendThread(m_hThreadUSbScanCount);
		}

		if (m_hUSBScanThread != NULL)
		{
			if (SuspendThread(m_hUSBScanThread) == -1)
			{
				AddLogEntry(L"### Failed to suspend/pause usb scan thread.", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
		m_bOnWMClose = true;
		m_tsScanPauseResumeTime = CTime::GetCurrentTime();
		AddLogEntry(L"Scanning Paused..", 0, 0, true, FIRSTLEVEL);
		m_bMsgAbortPopup = true;
		SCITER_VALUE svReturn = 0;

		if (m_dwScanOption == 0x01)
		{
			CallNotificationMessage(2, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_DO_YOU_WANT_STOP_SCAN"));
			if (theApp.m_bRetval == true)
			{
				svReturn = 1;
			}
			else
			{
				svReturn = 0;
			}
		}
		else
		{
			CallNotificationMessage(2, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_DO_YOU_WANT_STOP_CUSTOMSCAN"));
			if (theApp.m_bRetval == true)
			{
				svReturn = 1;
			}
			else
			{
				svReturn = 0;
			}
		}
		if (svReturn == 0 && m_bISScanning == true)
		{
			if (m_hThreadUSbScanCount != NULL)
			{
				ResumeThread(m_hThreadUSbScanCount);
			}

			if (m_hUSBScanThread != NULL)
			{
				if (ResumeThread(m_hUSBScanThread) == -1)
				{
					AddLogEntry(L"### Failed to resume usb scan thread.", 0, 0, true, SECONDLEVEL);
				}
			}
			if (m_bOnWMClose)
			{
				m_bOnWMClose = false;
			}
			//SetTimer(TIMER_SCAN_STATUS, 100, NULL);
			m_tsScanPauseResumeElapsedTime += ((CTime::GetCurrentTime() - m_tsScanPauseResumeTime));
			AddLogEntry(L"Scanning Resumed..", 0, 0, true, FIRSTLEVEL);
			m_bMsgAbortPopup = false;
			return false;
		}
		else if (((svReturn == 1) || (svReturn == 0)) && (m_bISScanning == false))
		{
			if (!m_bClose)
			{
				if (m_bMsgAbortPopup == false && m_bCloseMsgAbortPopup == false)
				{
					OnCancel();
				}
			}
			else
			{
				if (m_iThreatsFoundCount > 0)
				{
					m_bNoActionafterScanComplete = true;
					return false;
				}
			}
			return true;
		}

		if (m_hThreadUSbScanCount != NULL)
		{
			SuspendThread(m_hThreadUSbScanCount);
			TerminateThread(m_hThreadUSbScanCount, 0);
			m_hThreadUSbScanCount = NULL;
		}

		if (m_hUSBScanThread != NULL)
		{
			SuspendThread(m_hUSBScanThread);
			TerminateThread(m_hUSBScanThread, 0);
			m_hUSBScanThread = NULL;
		}
		//KillTimer(TIMER_SCAN_STATUS);
		SetLastScanDateTime();
		CTimeSpan		tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsScanStartTime) - m_tsScanPauseResumeElapsedTime;
		CString csTime = tsScanElapsedTime.Format(_T("%H:%M:%S"));
		CString csElapsedTime;
		csElapsedTime.Format(L"%s  %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ELAPSEDTIME"), csTime);
		CString csScaanningAborted;
		if (m_bScanningAborted == true && m_bISScanning == true)
		{
			if (m_iThreatsFoundCount > 0)
			{
				AddEntriesInReportsDB(L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED"));
				if (!m_objDBManager.SaveEntries(REPORTS))
				{
					AddLogEntry(L"### Failed to CUSBDetectUIDlg::ShutdownScanning in REPORTS", 0, 0, true, SECONDLEVEL);
				}

				if (m_dwScanOption == 0x01)
				{
					csScaanningAborted.Format(L"%s\n%s : %d", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_SCANNING_ABORTED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VIRUS_FOUND"), m_iThreatsFoundCount);
				}
				else if (m_dwScanOption == 0x02)
				{
					csScaanningAborted.Format(L"%s\n%s : %d", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_CUSTOM_SCANNING_ABORTED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VIRUS_FOUND"), m_iThreatsFoundCount);
				}
				CallUISetStatusfunction(L"", m_iThreatsFoundCount, m_dwFileScanned);
				m_svFunNotificationMessageCB.call(1, (SCITER_STRING)csScaanningAborted);
				if (m_dwScanOption == 0x01)
				{
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScaanningAborted.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_SCANNING_ABORTED"));
					AddLogEntry(csScaanningAborted, 0, 0, true, SECONDLEVEL);
					csScaanningAborted.Format(L">>> %s = %d, %s = %d, %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VIRUS_FOUND"), m_iThreatsFoundCount, csElapsedTime);
					AddLogEntry(csScaanningAborted, 0, 0, true, SECONDLEVEL);
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
				}
				else if (m_dwScanOption == 0x02)
				{
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScaanningAborted.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_CUSTOM_SCANNING_ABORTED"));
					AddLogEntry(csScaanningAborted, 0, 0, true, SECONDLEVEL);
					csScaanningAborted.Format(L">>> %s = %d, %s = %d, %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VIRUS_FOUND"), m_iThreatsFoundCount, csElapsedTime);
					AddLogEntry(csScaanningAborted, 0, 0, true, SECONDLEVEL);
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
				}
			}
			else
			{
				AddEntriesInReportsDB(L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED"));
				if (!m_objDBManager.SaveEntries(REPORTS))
				{
					AddLogEntry(L"### Failed to CUSBDetectUIDlg::ShutdownScanning in REPORTS", 0, 0, true, SECONDLEVEL);
				}

				if (m_dwScanOption == 0x01)
				{
					csScaanningAborted.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_SCANNING_ABORTED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"));
				}
				else if (m_dwScanOption == 0x02)
				{
					csScaanningAborted.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_CUSTOM_SCANNING_ABORTED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"));
				}
				CallNotificationMessage(1, (SCITER_STRING)csScaanningAborted);
				OnCancel();

				if (m_dwScanOption == 0x01)
				{
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScaanningAborted.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_SCANNING_ABORTED"));
					AddLogEntry(csScaanningAborted, 0, 0, true, SECONDLEVEL);
					csScaanningAborted.Format(L">>> %s = %d, %s, %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"), csElapsedTime);
					AddLogEntry(csScaanningAborted, 0, 0, true, SECONDLEVEL);
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
				}
				else if (m_dwScanOption == 0x02)
				{
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
					csScaanningAborted.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_CUSTOM_SCANNING_ABORTED"));
					AddLogEntry(csScaanningAborted, 0, 0, true, SECONDLEVEL);
					csScaanningAborted.Format(L">>> %s = %d, %s, %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"), csElapsedTime);
					AddLogEntry(csScaanningAborted, 0, 0, true, SECONDLEVEL);
					AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
				}
			}
		}
		m_bISScanning = false;
		if (m_bPlaySound)
		{
			AfxBeginThread(PlayScanFinishedThread, NULL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnBnClickedButtonStop", 0, 0, true, SECONDLEVEL);
	}
	return true;

}

/**********************************************************************************************************                     
*  Function Name  :	OnNMCustomdrawListViruslistControl                                                     
*  Description    : Customize list control at the time of inserting any entry into list controls.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0050
**********************************************************************************************************/
void CUSBDetectUIDlg::OnNMCustomdrawListViruslistControl(NMHDR *pNMHDR, LRESULT *pResult)
{
	try
	{
		NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

		// Take the default processing unless we set this to something else below.
		*pResult = 0;

		switch (pLVCD->nmcd.dwDrawStage)
		{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			break;
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnNMCustomdrawListViruslistControl", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	SendFile4RepairUsingService                                                     
*  Description    :	Send request to clean file to service
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0051
**********************************************************************************************************/
bool CUSBDetectUIDlg::SendFile4RepairUsingService(int iMessage, CString csThreatPath,CString csThreatName, DWORD dwISpyID ,bool bWait, bool bReconnect)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = iMessage;
		szPipeData.dwValue = dwISpyID;
		wcscpy_s(szPipeData.szFirstParam, csThreatPath);
		wcscpy_s(szPipeData.szSecondParam, csThreatName);

		if (m_dwScanOption == 0x01)
		{
			wcscpy_s(szPipeData.szThirdParam, L"USB Scan");
		}
		else
		{
			wcscpy_s(szPipeData.szThirdParam, L"Custom Scan");
		}
		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CUSBDetectUIDlg::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::SendFile4RepairUsingService, ThreatPath:[%s]", csThreatPath, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ISAllItemsCleaned                                                     
*  Description    :	Checks whether all selected entries are cleaned or not.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0052
**********************************************************************************************************/
bool CUSBDetectUIDlg::ISAllItemsCleaned()
{
	bool bReturn = true;
	DWORD dwFileNotFoundCount = 0;
	DWORD dwDetectedCount = 0;
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	OnTimer                                                     
*  Description    :	The framework calls this member function after each interval specified in the SetTimer 
					member function used to install a timer.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0053
**********************************************************************************************************/
void CUSBDetectUIDlg::OnTimer(UINT_PTR nIDEvent)
{	
	TCHAR	szTemp[256] = {0} ;
	try
	{
		if (nIDEvent == TIMER_SCAN_STATUS)
		{
			if (m_bOnWMClose == false)
			{
				CTimeSpan tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsScanStartTime) - m_tsScanPauseResumeElapsedTime;
				CString csTime = tsScanElapsedTime.Format(_T("%H:%M:%S"));
				CString csElapsedTime;
				csElapsedTime.Format(L"%s  %s", m_csStaticElapsedTime, csTime);
			}
			if (m_iTotalFileCount)
			{
				/*int iPercentage = int(((float)(m_dwFileScanned) / m_iTotalFileCount) * 100);
				m_svPercentageCB.call(iPercentage);
				wsprintf(szTemp, TEXT("%d%%"), iPercentage);*/
			}
		}
		else if (nIDEvent == TIMER_PAUSE_HANDLER)
		{
			m_tsScanPauseResumeTime = CTime::GetCurrentTime();
			//KillTimer(TIMER_SCAN_STATUS);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnTimer", 0, 0, true, SECONDLEVEL);
	}
	CJpegDialog::OnTimer(nIDEvent);
}

/**********************************************************************************************************                     
*  Function Name  :	SendRecoverOperations2Service                                                     
*  Description    :	Send a request to stored data into recover db.So that user can recover file.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0054
**********************************************************************************************************/
bool CUSBDetectUIDlg::SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry ,DWORD dwType, bool bWait, bool bReconnect)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = dwMessageinfo;
		_tcscpy(szPipeData.szFirstParam, csRecoverFileEntry);
		szPipeData.dwValue = dwType;
		CISpyCommunicator objCom(SERVICE_SERVER, bReconnect);

		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CScanDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CScanDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::SendRecoverOperations2Service, File:[%s]", csRecoverFileEntry, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/**********************************************************************************************************                     
*  Function Name  :	GetScanUSBFilesCount                                                     
*  Description    :	Get total files count in USB after enumerating whole USB.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0055
**********************************************************************************************************/
DWORD WINAPI GetScanUSBFilesCount(LPVOID lpParam )
{
	CUSBDetectUIDlg *pThis = (CUSBDetectUIDlg*)lpParam;
	if (!pThis)
		return 1;

	__try
	{
		pThis->m_iTotalFileCount = 0x00;
		if (pThis->bQuickScan || pThis->bFullScan)
		{
			if (!pThis->GetScanningPaths(pThis->m_csaAllScanPaths))
			{
				return false;
			}
			int	iIndex = 0x00;
			iIndex = (int)pThis->m_csaAllScanPaths.GetCount();
			if (!iIndex)
				return 2;
			for (int i = 0; i < iIndex; i++)
			{
				pThis->EnumTotalFolder(pThis->m_csaAllScanPaths.GetAt(i));
			}
		}
		else
		{
			for (int iIndex = 0; iIndex < pThis->m_csaEnumfolders.GetCount(); iIndex++)
			{
				pThis->EnumTotalFolder(pThis->m_csaEnumfolders.GetAt(iIndex));
			}
			if (pThis->m_iTotalFileCount)
			{
				pThis->m_bScanCount = true;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetScanUSBFilesCount", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	EnumTotalFolder                                                     
*  Description    :	Enumerate total files and folder
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0056
**********************************************************************************************************/
void CUSBDetectUIDlg::EnumTotalFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		bool bIsFolder = false;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bIsFolder = true;
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString str = finder.GetFilePath();
				EnumTotalFolder(str);
			}
			else
			{
				m_iTotalFileCount++;
			}
		}
		finder.Close();

		if(!bWorking && !bIsFolder)
		{
			m_iTotalFileCount++;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::EnumTotalFolder", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	PlayThreatsFoundThread                                                     
*  Description    :	Thread will play a sound when virus get found.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0057
**********************************************************************************************************/
UINT PlayThreatsFoundThread(LPVOID lpThis)
{
	CUSBDetectUIDlg *pThis = (CUSBDetectUIDlg*)lpThis;
	__try
	{
		if (!pThis)
		{
			return 0;
		}
		pThis->m_crSectionPlaySound.Lock();
		if (pThis->m_bISScanning == false)
		{
			return true;
		}
		PlaySound(_T("ThreatsFound.wav"), NULL, SND_FILENAME | SND_LOOP | SND_SYNC);
		pThis->m_crSectionPlaySound.Unlock();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in PlayThreatsFoundThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	ReadUISettingFromRegistry                                                     
*  Description    :	Read Enable/disable sound entry from registry
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0058
**********************************************************************************************************/
bool CUSBDetectUIDlg::ReadUISettingFromRegistry()
{
	try
	{
		HKEY key;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, &key) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Failed to open registry key in CUSBDetectUIDlg::ReadUISettingFromRegistry, Key Path %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
			return false;
		}

		DWORD dwPlaySound;
		DWORD dwPlaySoundSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		long ReadReg = RegQueryValueEx(key, L"dwEnableSound", NULL, &dwType, (LPBYTE)&dwPlaySound, &dwPlaySoundSize);
		if (ReadReg == ERROR_SUCCESS)
		{
			dwPlaySound = (DWORD)dwPlaySound;
			if (dwPlaySound == 0)
			{
				m_bPlaySound = false;
			}
			else
			{
				m_bPlaySound = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::ReadUISettingFromRegistry", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	PlayScanFinishedThread                                                     
*  Description    :	Thread will play a sound when scan gets finished.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0059
**********************************************************************************************************/
UINT PlayScanFinishedThread(LPVOID lpThis)
{
	__try
	{
		PlaySound(_T("ScanFinished.wav"), NULL, SND_FILENAME | SND_LOOP | SND_SYNC);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in PlayScanFinishedThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	OnClose                                                     
*  Description    :	The framework calls this member function as a signal that the CWnd or an application is to terminate.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0060
**********************************************************************************************************/
void CUSBDetectUIDlg::OnClose()
{
	__try
	{
		m_bIsCloseFrmTaskBar = true; // To avoid issue : 5544
		OnBnClickedButtonClose();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnClose", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	CloseUI                                                     
*  Description    :	Close USB UI
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0062
**********************************************************************************************************/
void CUSBDetectUIDlg::CloseUI()
{
	__try
	{
		OnBnClickedButtonClose();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::CloseUI", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name	 : Check4DBFiles                                                     
*  Description		 : Check whether all virus signature and repair routine DB is available or not.
*  Author Name		 : Neha Gharge                                                                                        
*  Date				 : 22 Jun 2014
*  SR_NO			 : WRDWIZUSBUI_0063
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
**********************************************************************************************************/
bool CUSBDetectUIDlg::Check4DBFiles()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
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
			CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_LATEST_SIGNATURE_NOT_PRESENT"));
			return false;
		}
		else if (!Check4ValidDBFiles(csWRDDBFilesFolderPath))
		{
			CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DATABASE_MESSAGE"));
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
					CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_LATEST_SIGNATURE_NOT_PRESENT"));
					return false;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::Check4DBFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnDataReceiveCallBack                                                     
*  Description    :	To receive notification as soon as registration completed.
*  Author Name    : Prajkta Nanaware                                                                                       
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0064
**********************************************************************************************************/
void CUSBDetectUIDlg::OnDataReceiveCallBack(LPVOID lpParam)
{
	__try
	{
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		switch(lpSpyData->iMessageInfo)
		{
		case RELOAD_REGISTARTION_DAYS:
			theApp.GetDaysLeft();
			break;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	GetShortFilePath                                                     
*  Description    :	Retrieves the short path form of the specified path.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jun 2014
*  SR_NO		  : WRDWIZUSBUI_0065
**********************************************************************************************************/
void CUSBDetectUIDlg::GetShortFilePath(CString csFilePath)
{
	try
	{
		if (csFilePath == L"")
		{
			return;
		}
		memset(m_szCleaningShortPath, 0, sizeof(m_szCleaningShortPath));
		GetShortPathName(csFilePath, m_szCleaningShortPath, 60);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::GetShortFilePath, FilePath:[%s]", csFilePath, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : SetLastScanDateTime( )
  Description    : Sets the last Scan Type and Scan Date & time for USB Scan or USB Detect.
  Author Name    : Niranjan Deshak.
  S.R. No        : 
  Date           : 4/2/2015
****************************************************************************/
void CUSBDetectUIDlg::SetLastScanDateTime( )
{
	SYSTEMTIME  CurrTime = { 0 };

	TCHAR	szLastDtTime[256] = { 0 }, szTime[16] = { 0 };

	try
	{
		GetLocalTime(&CurrTime);
		_wstrtime_s(szTime, 15);

		swprintf_s(szLastDtTime, _countof(szLastDtTime), L"%d/%d/%d %s", CurrTime.wMonth, CurrTime.wDay, CurrTime.wYear, szTime);

		CITinRegWrapper	objReg;
		DWORD	dwRet = 0x00;

		DWORD	dwScanType;
		if (m_dwScanOption == 0x01)
		{
			dwScanType = USBSCAN;
		}
		else if (m_dwScanOption == 0x02)
		{
			dwScanType = CUSTOMSCAN;
		}
		if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath, L"ScanType", REG_DWORD, dwScanType, true))
		{
			AddLogEntry(L"### Failed in Setting Registry CUSBDetectUIDlg::SetLastScanDateTime", 0, 0, true, SECONDLEVEL);
		}
		Sleep(5);

		if (!SendRegistryData2Service(SZ_STRING, theApp.m_csRegKeyPath.GetBuffer(),
			_T("LastScandt"), szLastDtTime, true))
		{
			AddLogEntry(L"### Failed to SET LastScandt CUSBDetectUIDlg::SetLastScanDateTime", 0, 0, true, SECONDLEVEL);
		}
		Sleep(5);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumRootKitScan::UpdateLastScanDateTime", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	SetRegistrykeyUsingService                                                     
*  Description    :	Set any dword value into given key into registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 4 Feb 2015
**********************************************************************************************************/
bool CUSBDetectUIDlg::SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = dwType;

		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg : SendVibraniumStartupTipsOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg : SendVibraniumStartupTipsOperation2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SetRegistrykeyUsingService, Key:[%s], Value:[%s]", SubKey, lpValueName, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************                    
*  Function Name  : SendRegistryData2Service                                                     
*  Description    : Send request to service to set registry through pipe.
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 4-Feb-2015
*************************************************************************************************/
bool CUSBDetectUIDlg::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait)
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
		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizLiveUpSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumkLiveUpSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SendRegistryData2Service, Key:[%s], Value:[%s]", szKey, szValue, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : CheckForDeleteFileINIEntries
*  Description    : check whether delete file ini present or not
*  Author Name    : Neha gharge
*  SR_NO		  :
*  Date           : 18 Feb 2015
*************************************************************************************************/
DWORD CUSBDetectUIDlg::CheckForDeleteFileINIEntries()
{
	bool bReturn = false;
	DWORD dwCount = 0x00;
	try
	{
		CString csQuarantineFolderPath = GetQuarantineFolderPath();
		csQuarantineFolderPath.Append(L"\\WRDWIZDELETEFAIL.INI");
		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csQuarantineFolderPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::CheckForDeleteFileINIEntries()", 0, 0, true, SECONDLEVEL);
	}
	return dwCount;
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get quarantine folder(i.e backup folder) path.
*  Author Name    :	Neha Gharge
*  SR_NO		  :
*  Date           : 18 Feb 2015
**********************************************************************************************************/
CString CUSBDetectUIDlg::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = {0};
		if(!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************
Function Name  : OnDriveRemovalStatus
Description    :
Author Name    : Nitin K
SR_NO			 :
Date           : 11th March 2015
****************************************************************************/
LRESULT CUSBDetectUIDlg::OnDriveRemovalStatus(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	try
	{
		m_chDriveleter = (char)wParam;
		m_bDeviceDetached = true;
		m_csDriveLetter = L"";
		m_csDriveLetter.Format(L"%s:", (CString)m_chDriveleter);
		m_iDriveCompareResult = m_csUSBDriveName.Compare(m_csDriveLetter);
		if (m_iDriveCompareResult == 0)
		{
			if (m_bISScanning == false && m_bIsCleaning == false)
				exit(0);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnDriveRemovalStatus", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : DeviceRemoved
Description    : Function is called when USB Device is removed inbetween of scan 
Author Name    : Nitin K
SR_NO			 :
Date           : 11th March 2015
****************************************************************************/
void CUSBDetectUIDlg::DeviceRemoved()
{
	try
	{
		CString csScanningFinished;
		KillTimer(TIMER_SCAN_STATUS);

		CTimeSpan		tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsScanStartTime) - m_tsScanPauseResumeElapsedTime;
		CString csTime = tsScanElapsedTime.Format(_T("%H:%M:%S"));
		CString csElapsedTime;
		csElapsedTime.Format(L"%s  %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_SCANTIME"), csTime);

		SetLastScanDateTime();
		if (m_bPlaySound)
		{
			AfxBeginThread(PlayScanFinishedThread, NULL);
		}
		AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
		csScanningFinished.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_REMOVED"));
		AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL);
		csScanningFinished.Format(L">>> %s = %d, %s, %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_dwFileScanned, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_THREATS_FOUND"), csElapsedTime);
		AddLogEntry(csScanningFinished, 0, 0, true, SECONDLEVEL); //Varada Ikhar
		AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
		CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_USB_REMOVED"));
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::DeviceRemoved", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	TokenizePath                                                     
*  Description    :	Tokenize differnt path into array
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 6 Sept. 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CUSBDetectUIDlg::TokenizePath(CString csEnumFolderORDriveName)
{
	try
	{
		CString resToken;
		int curPos = 0;

		resToken = csEnumFolderORDriveName.Tokenize(L"|", curPos);

		while (resToken != _T(""))
		{
			m_csaEnumfolders.Add(resToken);
			resToken = csEnumFolderORDriveName.Tokenize(L"|", curPos);
		};
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::TokenizePath, Path:[%s]", csEnumFolderORDriveName, 0, true, SECONDLEVEL);
	}
}

//Get ID from string
CString GetLanguageIDFromAction(CString csMessage)
{
	CString csLanguageID = EMPTY_STRING;
	try
	{

		if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED")) == 0)
		{
			csLanguageID = "IDS_USB_SCANNING_ABORTED";
		}
		else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND")) == 0)
		{
			csLanguageID = "IDS_USB_SCAN_NO_THREAT_FOUND";
		}
		else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_DETECTED";
		}
		else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_QUARANTINED";
		}
		else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_REPAIRED";
		}
		else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_NO_FILE_FOUND";
		}
		else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
		}
		else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
		}
		else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_FAILED")) == 0)
		{
			csLanguageID = "IDS_CONSTANT_THREAT_FAILED";
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetLanguageIDFromAction, Message:[%s]", csMessage, 0, true, SECONDLEVEL);
	}
	return csLanguageID;
}

/**********************************************************************************************************                     
*  Function Name  :	AddEntriesInReportsDB                                                     
*  Description    :	Add entries in report DB.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 9th Sept 2014
*  SR_NO		  : 
**********************************************************************************************************/
void CUSBDetectUIDlg::AddEntriesInReportsDB(CString csThreatName, CString csFilePath, CString csAction)
{
	try
	{
		CString csScanType = L"NA";
		CTime ctDateTime = CTime::GetCurrentTime();
		CString csUSBReportEntry = L"";
		CString csTime = ctDateTime.Format(_T("%H:%M:%S"));

		SYSTEMTIME  CurrTime = { 0 };
		GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
		CTime Time_Curr(CurrTime);
		int iMonth = Time_Curr.GetMonth();
		int iDate = Time_Curr.GetDay();
		int iYear = Time_Curr.GetYear();

		CString csDate = L"";
		csDate.Format(L"%d/%d/%d", iMonth, iDate, iYear);
		SCANTYPE eScanType;
		if (m_dwScanOption == 0x01)
		{
			csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_SCAN");
			eScanType = USBSCAN;
		}
		else
		{
			csScanType = theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CUSTOMSCAN");
			eScanType = CUSTOMSCAN;
		}
		CString csDateTime = L"NA";
		csDateTime.Format(_T("%s %s"), csDate, csTime);
		csUSBReportEntry.Format(L"%s#%s#%s#%s#%s#", csDateTime, csScanType, csThreatName, csFilePath, csAction);

		// Get the action ID for Action
		CString csActionID = GetLanguageIDFromAction(csAction);
		// Add entries into Database..
		CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");
		csInsertQuery.Format(_T("UPDATE Wardwiz_ScanSessionDetails SET db_ScanSessionEndDate = Date('now'),db_ScanSessionEndTime = Datetime('now', 'localtime'),db_TotalFilesScanned = %d,db_TotalThreatsFound = %d, db_TotalThreatsCleaned = %d WHERE db_ScanSessionID = %I64d;"), m_dwFileScanned, m_iThreatsFoundCount, m_dwTotalThreatsCleaned, m_iScanSessionId);
		CT2A ascii(csInsertQuery, CP_UTF8);
		m_objSqlDb.Close();
		InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::AddEntriesInReportsDB, Threat Name:[%s], File Path:[%s]", csThreatName, csFilePath, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	SendReportOperations2Service                                                     
*  Description    :	Send rootkit scan report to service, so that it get stored to DB file
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 9 Sept ,2014
**********************************************************************************************************/
bool CUSBDetectUIDlg::SendReportOperations2Service(int dwMessageinfo, CString csReportFileEntry ,DWORD dwType, bool bWait)
{
	try
	{
		CString csMessageinfo(L"");
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = dwMessageinfo;
		_tcscpy(szPipeData.szFirstParam, csReportFileEntry);
		szPipeData.dwValue = dwType;

		csMessageinfo.Format(L"%d", dwMessageinfo);
		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SendReportOperations2Service of messageinfo %s", csMessageinfo, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CUSBDetectUIDlg::of messageinfo %s", csMessageinfo, 0, true, SECONDLEVEL);
				return false;
			}
			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::SendReportOperations2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

BOOL CUSBDetectUIDlg::PreTranslateMessage(MSG* pMsg)
{
	try
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		{
			return TRUE;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::PreTranslateMessage", 0, 0, true, SECONDLEVEL);
	}
	return FALSE;
}

void CAboutDlg::OnBnClickedButtonAboutClosebtn()
{
	OnCancel();
}


/***************************************************************************
Function Name  : OnInitDialog
Description    : Initialize the about dialog box's controls.
Author Name    : Neha Gharge
SR_NO			 :
Date           : 27th March 2014
****************************************************************************/
BOOL CAboutDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	CRect rect1;
	this->GetClientRect(rect1);
	CRgn rgn;
	rgn.CreateRectRgn(rect1.top, rect1.left, rect1.Width() - 3, rect1.Height() - 3);
	this->SetWindowRgn(rgn, TRUE);
	ReadDatabaseVersionFromRegistry();
	return TRUE;  // return TRUE unless you set the focus to a control
}


/**************************************************************************************
Function Name  : ReadDatabaseVersionFromRegistry
Description    : Read Email Scan Entry from registry show controls on main Ui accordingly
Author Name    : Nitin K.
SR_NO			 :
Date           : 24th April 2014
****************************************************************************************/
void CAboutDlg::ReadDatabaseVersionFromRegistry()
{
	HKEY key;
	long ReadReg;
	TCHAR szvalueVersion[1024];
	DWORD dwvaluelengthVersion = 1024;
	DWORD dwtypeVersion = REG_SZ;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
	{
		return;
	}
	ReadReg = RegQueryValueEx(key, L"AppVersion", NULL, &dwtypeVersion, (LPBYTE)&szvalueVersion, &dwvaluelengthVersion);
	if (ReadReg == ERROR_SUCCESS)
	{
		CString stMessage, stMessageDBVersion;
		CString stVersion;
		stVersion = (LPCTSTR)szvalueVersion;
		stMessage.Format(L"%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ABOUTGUI_VERSION"), stVersion);
		ReadReg = RegQueryValueEx(key, L"DataBaseVersion", NULL, &dwtypeVersion, (LPBYTE)&szvalueVersion, &dwvaluelengthVersion);
		if (ReadReg == ERROR_SUCCESS)
		{
			stVersion = (LPCTSTR)szvalueVersion;
			stMessageDBVersion.Format(L"\n\n%s %s ", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ABOUTGUI_DBVERSION "), stVersion);
			stMessage.Append(stMessageDBVersion);
		}
	}
}

void CAboutDlg::OnBnClickedButtonOk()
{
	OnOK();
}

/**********************************************************************************************************
*  Function Name  :	Check4ValidDBFiles
*  Description    :	This function will check for valid signature and valid Version length in DB files
					if any mismatch found, will return false otherwise true.
*  Author Name    :	Varada Ikhar
*  SR_NO		  :
*  Date           : 20 Mar 2015
**********************************************************************************************************/
bool CUSBDetectUIDlg::Check4ValidDBFiles(CString csDBFolderPath)
{
	try
	{
		CString csFilePath;
		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAV1);

		DWORD dwDBVersionLength = 0;
		DWORD dwMajorVersion = 0;
		CWrdwizEncDecManager objWrdwizEncDecMgr;
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(csFilePath, dwDBVersionLength, dwMajorVersion))
		{
			AddLogEntry(L"### Invalid DB found (or) may corrupted, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		//DB Version lenfth should be in between 7 and 19
		//Eg: 1.0.0.0 to 9999.9999.9999.9999
		if (!(dwDBVersionLength >= 7 && dwDBVersionLength <=  19) )
		{
			AddLogEntry(L"### Invalid DB Version length, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAVR);
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(csFilePath, dwDBVersionLength, dwMajorVersion))
		{
			AddLogEntry(L"### Invalid DB found (or) may corrupted, File Name %s", csFilePath, 0, true, SECONDLEVEL);
			return false;
		}

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

/**********************************************************************************************************
*  Function Name  :	SendRecoverOperations2Service
*  Description    :	Sends data to service to repair a file and service reply the status to GUI
*  SR.NO		  :
*  Author Name    : Vilas Suvarnakar
*  Date           : 07 April 2015
**********************************************************************************************************/
bool CUSBDetectUIDlg::SendFile4RepairUsingService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect)
{
	try
	{
		if (!pszPipeData)
			return false;

		if (!m_objCom.SendData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send RESUME_SCAN data in CVibraniumRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read RESUME_SCAN data in CVibraniumRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/***************************************************************************************************
*  Function Name  : HideAllElements
*  Description    : Function to hide all elements applied when not scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 May 2016
****************************************************************************************************/
void CUSBDetectUIDlg::HideAllElements()
{
	try
	{
		m_btnShowHideButton.ShowWindow(SW_HIDE);
		m_stScanningText.ShowWindow(SW_HIDE);
		m_stStatusText.ShowWindow(SW_HIDE);
		m_stActualScnFileFolderName.ShowWindow(SW_HIDE);
		m_stActualStatus.ShowWindow(SW_HIDE);
		m_stFilescanned.ShowWindow(SW_HIDE);
		m_stHeaderLogo.ShowWindow(SW_HIDE);
		m_btnStop.ShowWindow(SW_HIDE);
		m_btnClean.ShowWindow(SW_HIDE);
		m_stHeaderLogo.SetBitmap(NULL);
		m_stStartTime.ShowWindow(SW_HIDE);
		m_stThreadFound.ShowWindow(SW_HIDE);
		m_stElapsedTime.ShowWindow(SW_HIDE);
		m_prgUsbProgressbar.ShowWindow(SW_HIDE);
		m_stUsbGifloader.ShowWindow(SW_HIDE);
		m_btnPauseResume.ShowWindow(SW_HIDE);
		m_lstVirusTrack.ShowWindow(SW_HIDE);
		m_stHideDetails.ShowWindow(SW_HIDE);
		m_stShowDetails.ShowWindow(SW_HIDE);
		m_stHeaderText.ShowWindow(SW_HIDE);
		m_stHideDetailsText.ShowWindow(SW_HIDE);
		m_stShowDetailstext.ShowWindow(SW_HIDE);
		m_stUsbPercentage.ShowWindow(SW_HIDE);
		m_stPauseAndResumeStatus.ShowWindow(SW_HIDE);
		m_stCleaningText.ShowWindow(SW_HIDE);
		m_chkSelectAll.ShowWindow(SW_HIDE);
		m_stSelectAll.ShowWindow(SW_HIDE);
		m_edtActualStatus.ShowWindow(SW_HIDE);
		m_btnShowHideButton.ShowWindow(SW_HIDE);
		m_stScanningText.ShowWindow(SW_HIDE);
		m_stStatusText.ShowWindow(SW_HIDE);
		m_stActualScnFileFolderName.ShowWindow(SW_HIDE);
		m_stActualStatus.ShowWindow(SW_HIDE);
		m_stFilescanned.ShowWindow(SW_HIDE);
		m_stStartTime.ShowWindow(SW_HIDE);
		m_stThreadFound.ShowWindow(SW_HIDE);
		m_stElapsedTime.ShowWindow(SW_HIDE);
		m_btnPauseResume.ShowWindow(SW_HIDE);
		m_btnStop.ShowWindow(SW_HIDE);
		m_btnClean.ShowWindow(SW_HIDE);
		m_lstVirusTrack.ShowWindow(SW_HIDE);
		m_stHideDetails.ShowWindow(SW_HIDE);
		m_stShowDetails.ShowWindow(SW_HIDE);
		m_btnUsbClose.ShowWindow(SW_HIDE);
		m_btnUsbMinimize.ShowWindow(SW_HIDE);
		m_stHideDetailsText.ShowWindow(SW_HIDE);
		m_stShowDetailstext.ShowWindow(SW_HIDE);
		m_stHeaderText.ShowWindow(SW_HIDE);
		m_stHeaderLogo.ShowWindow(SW_HIDE);
		m_stUsbGifloader.ShowWindow(SW_HIDE);
		m_prgUsbProgressbar.ShowWindow(SW_HIDE);
		m_stUsbPercentage.ShowWindow(SW_HIDE);
		m_stPauseAndResumeStatus.ShowWindow(SW_HIDE);
		m_stCleaningText.ShowWindow(SW_HIDE);
		m_edtActualStatus.ShowWindow(SW_HIDE);
		m_chkSelectAll.ShowWindow(SW_HIDE);
		m_stSelectAll.ShowWindow(SW_HIDE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::HideAllElements", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : CheckForRepairFileINIEntries
*  Description    : check whether repair file ini present or not
*  Author Name    : Vilas s
*  SR_NO		  :
*  Date           : 10 April 2015
*************************************************************************************************/
DWORD CUSBDetectUIDlg::CheckForRepairFileINIEntries()
{
	bool bReturn = false;
	DWORD dwCount = 0x00;
	try
	{
		CString csRepairIniPath = GetQuarantineFolderPath();
		csRepairIniPath += L"\\";
		csRepairIniPath += WRDWIZREPAIRINI;

		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csRepairIniPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::CheckForRepairFileINIEntries()", 0, 0, true, SECONDLEVEL);
	}

	return dwCount;
}

/***************************************************************************
Function Name  : StartWardWizAVTray
Description    : This function will launch the Tray.
Author Name    : Neha Gharge
SR_NO		   :
Date           : 24th April 2015
****************************************************************************/
void CUSBDetectUIDlg::StartWardWizAVTray()
{
	try
	{
		AddLogEntry(L">>> CUSBDetectUIDlg : Start Vibranium tray", 0, 0, true, FIRSTLEVEL);
		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFullPath[MAX_PATH] = { 0 };

		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		if (szTemp == NULL)
		{
			return;
		}
		szTemp[0] = '\0';

		_tcscpy(szFullPath, szModulePath);
		_tcscat(szFullPath, L"\\VBTRAY.EXE");

		::ShellExecute(NULL, L"Open", szFullPath, L"-NOSPSCRN", NULL, SWP_SHOWWINDOW);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::StartVibranium AV Tray",0,0,true,SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : OnBnClickedCheckSelectAll
Description    : After clicking select all button
Author Name    : Neha Gharge
SR_NO		   :
Date           : 13th May 2015
//ticket no 389 select all check box required
****************************************************************************/
void CUSBDetectUIDlg::OnBnClickedCheckSelectAll()
{
	try
	{
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnBnClickedCheckSelectAll()", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name		: InsertRecoverEntry
*  Description			: Insert entry into recover DB
*  Function Arguments	: szThreatPath, csDuplicateName, szThreatName.
dwShowStatus = 0;	Repair/Delete Sucess
dwShowStatus = 1; Delete failed
dwShowStatus = 2; Repair failed
*  Author Name			: Neha Gharge
*  Date					: 4 Dec 2014
*  SR_NO				:
**********************************************************************************************************/
bool CUSBDetectUIDlg::InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus)
{
	try
	{
		CIspyList newEntry(szThreatPath, csDuplicateName, szThreatName, L"", L"", L"", dwShowStatus);
		m_objRecoverDB.InsertEntry(newEntry, RECOVER);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::InsertRecoverEntry, ThreatPath: %s", szThreatPath, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name		: SaveLocalDatabase
*  Description			: Function to save local white database into hard disk
*  Author Name			: Ram Shelke
*  Date					: 12 Apr 2016
*  SR_NO				:
**********************************************************************************************************/
bool CUSBDetectUIDlg::SaveLocalDatabase()
{
	__try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SAVE_LOCALDB;

		if (!m_objScanCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
		}

		if (!m_objScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CUSBDetectUIDlg::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
		}

		if (szPipeData.dwValue == 1)
		{
			return true;
		}
	}
		__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name		: WindowProc
*  Description			: Window procedure added
*  Author Name			: Sanjay Khapre
*  Date					: 10 May 2016
*  SR_NO				:
**********************************************************************************************************/
LRESULT CUSBDetectUIDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	__try
	{
		if (message == WM_TIMER)
		{
			if (LOWORD(wParam) == TIMER_SCAN_STATUS)
			{
				StatusTimer(m_csPreviousFilePath.GetBuffer()); //issue solved : 5934
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
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  : On_OnResumeScan
*  Description    : Function to resume thread.
*  Author Name    : Amol Jaware
*  Date			  : 4 July 2016
****************************************************************************************************/
json::value  CUSBDetectUIDlg::On_ResumeScan()
{
	try
	{
		if (m_hThreadUSbScanCount != NULL)
		{
			ResumeThread(m_hThreadUSbScanCount);
		}

		if (m_hUSBScanThread != NULL)
		{
			ResumeThread(m_hUSBScanThread);
		}
		SetTimer(TIMER_SCAN_STATUS, 500, NULL); 
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_OnResumeScan()", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_OnPauseScan
*  Description    : Function to pause thread.
*  Author Name    : Amol Jaware
*  Date			  : 4 July 2016
****************************************************************************************************/
json::value CUSBDetectUIDlg::On_OnPauseScan()
{
	try
	{
		m_objSqlDb.Close();

		if (m_hThreadUSbScanCount != NULL)
		{
			SuspendThread(m_hThreadUSbScanCount);
		}
		
		if (m_hUSBScanThread != NULL)
		{
			SuspendThread(m_hUSBScanThread);
		}
		KillTimer(TIMER_SCAN_STATUS);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_OnPauseScan()", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_SendNotificationMessageCB
*  Description    : Function to show notifications.
*  Author Name    : Amol Jaware
*  Date			  : 11 July 2016
****************************************************************************************************/
json::value CUSBDetectUIDlg::On_SendNotificationMessageCB(SCITER_VALUE svFunNotificationMessageCB)
{
	try
	{
		m_svFunNotificationMessageCB = svFunNotificationMessageCB;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_SendNotificationMessageCB()", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : CallUISetStatusfunction
*  Description    : Function to set Scan Details.
*  Author Name    : Nitin Kolapkar
*  Date			  : 
****************************************************************************************************/
void CUSBDetectUIDlg::CallUISetStatusfunction(CString csFilePath, int iThreatsFoundCount, int iFileScanned)
{
	try
	{
		if (m_bDeviceDetached)
		{
			return;
		}
		TCHAR					m_szUSBFilePath[MAX_PATH];
		memset(m_szUSBFilePath, 0, sizeof(m_szUSBFilePath));
		GetShortPathName(csFilePath, m_szUSBFilePath, 60);

		sciter::value map;
		map.set_item("one", sciter::string(m_szUSBFilePath));
		map.set_item("two", iThreatsFoundCount);
		map.set_item("three", iFileScanned);

		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETFILEPATH_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::callUIfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 Aug 2016
****************************************************************************************************/
json::value CUSBDetectUIDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProdID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInRepairRebootIni
*  Description    :	Check whether file available in repair ini
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Oct 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CUSBDetectUIDlg::CheckFileIsInRepairRebootIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRepairIniPath))
		{
			AddLogEntry(L"### Failed in CUSBDetectUIDlg::CheckFileIsInRepairRebootIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		wcscat_s(szRepairIniPath, _countof(szRepairIniPath), L"\\WWRepair.ini");
		DWORD dwRepairCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRepairIniPath);
		if (!dwRepairCount)
			return bReturn;

		DWORD	i = 0x01;

		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[2048] = { 0 };
		TCHAR	szFilePath[512] = { 0 };

		swprintf_s(szFilePath, _countof(szFilePath), L"|%s|", csFilePath);
		_wcsupr(szFilePath);

		for (; i <= dwRepairCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRepairIniPath);
			_wcsupr(szValueData);
			if (wcsstr(szValueData, szFilePath) != NULL)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::CheckFileIsInRepairRebootIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInRecoverIni
*  Description    :	Parses WWRecover.ini and not sends to scan if file is found.
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Oct 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CUSBDetectUIDlg::CheckFileIsInRecoverIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR	szRecoverIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRecoverIniPath))
		{
			AddLogEntry(L"### Failed in CheckFileIsInRecoverIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), L"\\");
		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), WRDWIZRECOVERINI);

		DWORD dwRecoverCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRecoverIniPath);

		if (!dwRecoverCount)
			return bReturn;

		DWORD	i = 0x01;
		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[512] = { 0 };

		for (; i <= dwRecoverCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			ZeroMemory(szValueData, sizeof(szValueData));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRecoverIniPath);
			if (csFilePath.CompareNoCase(szValueData) == 0x00)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::CheckFileIsInRecoverIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInDeleteIni
*  Description    :	Check whether file available in delete.ini
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Oct 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CUSBDetectUIDlg::CheckFileIsInDeleteIni(CString csQurFilePaths)
{
	bool	bReturn = false;
	TCHAR szValueName[260] = { 0 };
	DWORD dwCount = 0x00;
	TCHAR szDuplicateString[512] = { 0 };
	TCHAR szTempKey[512] = { 0 };
	TCHAR szApplnName[512] = { 0 };
	TCHAR szConcatnateDeleteString[1024] = { 0 };
	TCHAR szFileName[512] = { 0 };
	TCHAR szQuarantineFileName[512] = { 0 };
	TCHAR szVirusName[512] = { 0 };

	try
	{
		CString csDeleteFailedINIPath(L"");
		CString csQuarantineFolderPath = GetQuarantineFolderPath();

		csDeleteFailedINIPath.Format(L"%s\\WRDWIZDELETEFAIL.INI", csQuarantineFolderPath);
		DWORD dwDeleteCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csDeleteFailedINIPath);
		if (!dwDeleteCount)
			return bReturn;

		for (int i = 0x01; i <= static_cast<int>(dwDeleteCount); i++)
		{
			ZeroMemory(szTempKey, sizeof(szTempKey));
			swprintf_s(szTempKey, _countof(szTempKey), L"%lu", i);
			GetPrivateProfileString(L"DeleteFiles", szTempKey, L"", szDuplicateString, 511, csDeleteFailedINIPath);
			ZeroMemory(szApplnName, sizeof(szApplnName));
			if (!TokenizationOfParameterForrecover(szDuplicateString, szFileName, _countof(szFileName), szQuarantineFileName, _countof(szQuarantineFileName), szVirusName, _countof(szVirusName)))
			{
				AddLogEntry(L"### CheckFileIsInDeleteIni::TokenizationOfParameterForrecover is not tokenize properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}
			if (!TokenizeIniData(szFileName, szApplnName, _countof(szApplnName)))
			{
				AddLogEntry(L"### CUSBDetectUIDlg::CheckFileIsInDeleteIni::TokenizeIniData is not tokenize properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}
			CString csDuplicateString = (CString)szDuplicateString;
			CString csFileName = (CString)szApplnName;
			if (csFileName.CompareNoCase(csQurFilePaths) == 0)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::CheckFileIsInDeleteIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Oct 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CUSBDetectUIDlg::GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath)
{
	bool	bReturn = true;
	try
	{
		TCHAR	szModulePath[MAX_PATH] = { 0 };

		GetModulePath(szModulePath, MAX_PATH);
		if (!wcslen(szModulePath))
			return bReturn;

		wcscpy_s(lpszQuarantineFolPath, MAX_PATH - 1, szModulePath);
		wcscat_s(lpszQuarantineFolPath, MAX_PATH - 1, L"\\Quarantine");
		bReturn = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	TokenizationOfParameterForrecover
*  Description    :	Tokenize input path to get file name, quarantine name and virus name
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Oct 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CUSBDetectUIDlg::TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName)
{
	TCHAR	szToken[] = L"|";
	TCHAR	*pToken = NULL;
	try
	{
		if (lpWholePath == NULL || szFileName == NULL || szQuarantinepath == NULL || szVirusName == NULL)
			return false;

		pToken = wcstok(lpWholePath, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (pToken)
		{
			wcscpy_s(szFileName, (dwsizeofFileName - 1), pToken);
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szQuarantinepath, (dwsizeofquarantinefileName - 1), pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szVirusName, (dwsizeofVirusName - 1), pToken);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::TokenizationOfParameterForrecover", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	TokenizeIniData
*  Description    :	Tokenization of entries from delete file ini
*  Author Name    : Jeena Mariam Saji
*  Date           : 04 Oct 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CUSBDetectUIDlg::TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName)
{
	TCHAR	szToken[] = L",";
	TCHAR	*pToken = NULL;
	try
	{
		if (lpszValuedata == NULL || szApplicationName == NULL)
			return false;

		pToken = wcstok(lpszValuedata, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		TCHAR	szValueApplicationName[512] = { 0 };
		if (pToken)
		{
			wcscpy_s(szValueApplicationName, _countof(szValueApplicationName), pToken);
			wcscpy_s(szApplicationName, (dwSizeofApplicationName - 1), szValueApplicationName);
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		}

		TCHAR	szAttemptCnt[16] = { 0 };

		if (pToken)
			wcscpy_s(szAttemptCnt, _countof(szAttemptCnt), pToken);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::TokenizeIniData", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

json::value CUSBDetectUIDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;
			theApp.m_objCompleteEvent.SetEvent();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

void CUSBDetectUIDlg::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
{
	try
	{
		m_svFunNotificationMessageCB.call(iMsgType, (SCITER_STRING)strMessageString);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CUSBDetectUIDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Amol Jaware
*  Date			  : 5 Aug 2016
****************************************************************************************************/
json::value CUSBDetectUIDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
Function Name  : On_PercentageTimer
Description    : 
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 17th Feb 2017
/***************************************************************************************************/
json::value CUSBDetectUIDlg::On_PercentageTimer()
{
	try
	{
		m_iPercentage = 0; 
		m_iPercentage = int(((float)(m_dwFileScanned) / m_iTotalFileCount) * 100);
		return m_iPercentage;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_PercentageTimer", 0, 0, true, SECONDLEVEL);
	}
	return m_iPercentage;
}

/***************************************************************************************************
Function Name  : On_FileStatusTimer
Description    :
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 17th Feb 2017
/***************************************************************************************************/
json::value CUSBDetectUIDlg::On_FileStatusTimer(SCITER_VALUE svArrFileStatusDetails)
{
	try
	{
		m_svArrFileStatusDetails = svArrFileStatusDetails;
		//StatusTimer(m_csPreviousFilePath.GetBuffer()); //issue solved : 5934
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in On_FileStatusTimer", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : StatusTimer
Description    :
Author Name    : Ram Shelke
SR_NO		   :
Date           : 17th Feb 2017
/***************************************************************************************************/
void CUSBDetectUIDlg::StatusTimer(LPTSTR lpFilePath)
{
	__try
	{
		if (!lpFilePath)
			return;

		StatusTimerSEH(lpFilePath);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in StatusTimer, FilePath: %s", lpFilePath, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : StatusTimerSEH
Description    : Function with structured exception handled.
Author Name    : Ram Shelke
SR_NO		   :
Date           : 17th Feb 2017
/***************************************************************************************************/
void CUSBDetectUIDlg::StatusTimerSEH(LPTSTR lpFilePath)
{
	CString csFinalFilePath;
	CString csPath;

	if (!lpFilePath)
		return;

	csPath = lpFilePath;;
	TCHAR	szActiveScanFilePath[1024] = { 0 };
	int iCount = csPath.ReverseFind('\\');
	CString csFileName = csPath.Right(csPath.GetLength() - (iCount + 1));
	CString csFolderPath;
	csFolderPath = csPath.Left(iCount);
	if (GetShortPathName(csFolderPath, szActiveScanFilePath, 60) == 0x00)
		return;

	CString csTempFileName = csFileName;
	iCount = csTempFileName.ReverseFind('.');
	CString csFileExt = csTempFileName.Right(csTempFileName.GetLength() - (iCount));
	csTempFileName = csTempFileName.Left(iCount);
	if (csTempFileName.GetLength() > 10)
	{
		csTempFileName = csTempFileName.Left(7);
		csFileName.Format(L"%s~%s", csTempFileName, csFileExt);
	}

	if (iCount <= 0)
	{
		CString cstmpFileName = csFileName.Left(3);
		csFinalFilePath.Format(L"%s\\%s~", szActiveScanFilePath, cstmpFileName);
	}
	else
	{
		int iFoldPathLength = _tcslen(szActiveScanFilePath);

		if (szActiveScanFilePath[--iFoldPathLength] == '\\')
		{
			csFinalFilePath.Format(L"%s%s", szActiveScanFilePath, csFileName);
		}
		else
		{
			csFinalFilePath.Format(L"%s\\%s", szActiveScanFilePath, csFileName);
		}
	}
	m_svArrFileStatusDetails.call(sciter::string(csFinalFilePath), sciter::value(m_iThreatsFoundCount), sciter::value((int)m_dwFileScanned));
}

/***************************************************************************************************
Function Name  : CheckFileOrFolderOnRootPath
Description    : Function which check IS file/Folder present on root path
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 25th Apr 2017
/*************************s**************************************************************************/
bool CUSBDetectUIDlg::CheckFileOrFolderOnRootPath(CString csFilePath)
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
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::CheckFileOrFolderOnRootPath, Path: %s", csFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetScanningPaths
*  Description    :	Get scan path according to scanning types.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 23 Feb 2018
**********************************************************************************************************/
bool CUSBDetectUIDlg::GetScanningPaths(CStringArray &csaReturn)
{
	try
	{
		CString cScanPath;
		TCHAR	szProgFilesDir86[MAX_PATH] = { 0 };
		TCHAR	szProgFilesDir[MAX_PATH] = { 0 };
		WardWizSystemInfo	objSysInfo;

		switch (m_eScanType)
		{
		case QUICKSCAN:
			m_bQuickScan = true;
			csaReturn.RemoveAll();
			//csaReturn.Add(L"QUICKSCAN");
			if (objSysInfo.GetOSType())
			{
				GetEnvironmentVariable(TEXT("PROGRAMFILES(X86)"), szProgFilesDir86, 255);
				csaReturn.Add(szProgFilesDir86);
			}
			else
			{
				GetEnvironmentVariable(TEXT("ProgramFiles"), szProgFilesDir, 255);
				csaReturn.Add(szProgFilesDir);
			}
			break;
		case FULLSCAN:
			m_bFullScan = true;
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
				const std::wstring chFilePah = EachEntry[L"FilePath"].get(L"");
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
			//return OnGetSelection();
			break;
		}
		if (csaReturn.GetCount() > 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::GetScanningPaths", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetAllDrivesList
*  Description    :	Makes list of drives present on a system.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 23 Feb 2018
**********************************************************************************************************/
bool CUSBDetectUIDlg::GetAllDrivesList(CStringArray &csaReturn)
{
	bool bReturn = false;
	try
	{
		csaReturn.RemoveAll();
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
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::GetAllDrivesList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}


/**********************************************************************************************************
*  Function Name  :	SendData2EPSClient
*  Description    :	Function which sends detected entries data to wardwiz EPS client
*  Author Name    : Ram Shelke
*  Date           : 7th March 2018
*  SR_NO		  : strVirusName, strScanFileName, csAction
**********************************************************************************************************/
bool CUSBDetectUIDlg::SendData2EPSClient(LPCTSTR szFilePath, LPCTSTR szVirusName, DWORD dwActionTaken)
{
	try
	{
		
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SHOW_VIRUSENTRY;
		szPipeData.dwSecondValue = dwActionTaken;
		wcscpy_s(szPipeData.szFirstParam, m_csTaskID); //send here task ID
		wcscpy_s(szPipeData.szSecondParam, szVirusName);
		wcscpy_s(szPipeData.szThirdParam, szFilePath);

		CISpyCommunicator objCom(EPS_CLIENT_AGENT, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SendData2EPSClient", 0, 0, true, SECONDLEVEL);
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::SendData2EPSClient, File: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	SendData2EPSClient
*  Description    :	Function which sends detected entries data to wardwiz EPS client
*  Author Name    : Ram Shelke
*  Date           : 7th March 2018
*  SR_NO		  : strVirusName, strScanFileName, csAction
**********************************************************************************************************/
bool CUSBDetectUIDlg::SendScanFinishedData2EPSClient(DWORD dwIsAborted)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SCAN_FINISHED;
		szPipeData.dwSecondValue = dwIsAborted;
		wcscpy_s(szPipeData.szFirstParam, m_csTaskID); //send here task ID

		CISpyCommunicator objCom(EPS_CLIENT_AGENT, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SendScanFinishedData2EPSClient", 0, 0, true, SECONDLEVEL);
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::SendScanFinishedData2EPSClient", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : GetDBPathforEPS
Description    : This function will get Database Path
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 20 February 2018
***********************************************************************************************/
json::value CUSBDetectUIDlg::GetDBPathforEPS()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", GetWardWizPathFromRegistry(), L"\\VBFEATURESLOCK.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::GetDBPathforEPS", 0, 0, true, SECONDLEVEL);
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
json::value CUSBDetectUIDlg::GetDecryptPasssword(SCITER_VALUE svEncryptPasssword)
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
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::GetDecryptPasssword", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szPassHash);
}

/***********************************************************************************************
Function Name  : CheckForWardWizCommand
Description    : This function will check id commandline has any WardWiz Commands in it
Author Name    : Jeena Mariam Saji
Date           : 30 October 2018
***********************************************************************************************/
bool CUSBDetectUIDlg::CheckForWardWizCommand(CString csCommandVal)
{
	bool bReturn = false;
	try
	{
		CStringArray csCommandValue;
		csCommandValue.Add(L"EPSNOUI");
		csCommandValue.Add(L"TID");
		csCommandValue.Add(L"VBUSBDETECTUI.EXE");
		csCommandValue.Add(L"SCHEDSCAN");
		csCommandValue.Add(L"SDYES");
		csCommandValue.Add(L"FULLSCAN");
		csCommandValue.Add(L"QUICKSCAN");
		csCommandValue.Add(L"CUSTOMSCAN");
		csCommandValue.Add(L"USBSCAN");
		csCommandValue.Add(L"SDNO");
		for (int iIndex = 0; iIndex < csCommandValue.GetCount(); iIndex++)
		{
			CString csCurrentVal;
			csCurrentVal = csCommandValue.GetAt(iIndex);
			if (csCommandVal.Find(csCurrentVal) != -1)
			{
				bReturn = true;
				return bReturn;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::CheckForVibraniumCommand", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Akshay Patil
*  Date			  : 09 Jan 2018
****************************************************************************************************/
json::value CUSBDetectUIDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SendData2Tray
*  Description    : To send data to tray
*  Author Name    : Jeena Mariam Saji
*  Date			  : 01 July 2019
****************************************************************************************************/
bool CUSBDetectUIDlg::SendData2Tray(DWORD dwMessage, DWORD dwValue, LPTSTR lpszFirstParam, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwValue;
		if (lpszFirstParam != NULL)
		{
			wcscpy_s(szPipeData.szFirstParam, lpszFirstParam);
		}
		CISpyCommunicator objCom(TRAY_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CUSBDetectUIDlg::SendData2Tray", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CUSBDetectUIDlg::SendData2Tray", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : FunCheckInternetAccessBlock
Description    : Function to check internet access block
Author Name    : Jeena Mariam Saji
Date           : 10 Dec 2019
***********************************************************************************************/
json::value CUSBDetectUIDlg::FunCheckInternetAccessBlock()
{
	bool RetVal = false;
	try
	{
		if (theApp.m_dwProdID == BASIC || theApp.m_dwProdID == ESSENTIAL)
		{
			return false;
		}

		CITinRegWrapper objReg;
		DWORD dwParentalControl = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwParentalCntrlFlg", dwParentalControl) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CUSBDetectUIDlg::FunCheckInternetAccessBlock", 0, 0, true, SECONDLEVEL);
		}

		if (dwParentalControl == 1)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = ON_CHECK_INTERNET_ACCESS;

			CISpyCommunicator objCom(SERVICE_SERVER);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send Data in CUSBDetectUIDlg::SendData", 0, 0, true, SECONDLEVEL);
			}

			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read Data in CUSBDetectUIDlg::ReadData", 0, 0, true, SECONDLEVEL);
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