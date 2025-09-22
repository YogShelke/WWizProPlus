/**********************************************************************************************************                     
	  Program Name          :     ScanDlg.cpp                                                                                                      
	  Description           :     This dialog is using for following Scanning functionality
								  1) Quick Scan.
								  2) Full Scan.
								  3) Custom Scan.
								  4) Removable devices scan.

	  Author Name:                Ramkrushna Shelke                                                                                           
	  Date Of Creation      :     11th Nov 2013
	  Version No            :     0.0.0.1
	  Special Logic Used    :     Multithreading functionality to launch the scanner, pause scanning &
								  Terminate Scanning. 
	  Modification Log      :               
	  1. Ramkrushna               function in main        11-11-2013              CSR NO                                          
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyGUI.h"
#include "ScanDlg.h"
#include "ISpyGUIDlg.h"
#include "EnumProcess.h"
#include "psapi.h"
#include "ITinRegWrapper.h"
#include "WrdwizEncDecManager.h"
#include "SelectDialog.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define TIMER_SCAN_STATUS			100
#define TIMER_PAUSE_HANDLER			101
#define TIMER_FILESSCANNED_STATUS	102

#define BUFSIZE 2024
#define MAX_OUTPUT 8192
//#define DETECTEDSTATUS			L"Detected"
//#define DELETEDSTATUS			L"Quarantined"
//#define FILEREPAIRED 			L"Repaired"
//#define FAILEDSTATUS			L"Failed"
//#define	szpIpsy					"ISPY"
//#define NOFILEFOUND				L"No file found"

BOOL					isScanning;
#define BAIL_OUT(code) { isScanning = FALSE; return code; }

//HANDLE hChildStdinWrDup = INVALID_HANDLE_VALUE, hChildStdoutRdDup = INVALID_HANDLE_VALUE;
//HANDLE					m_LogMutex;
//HANDLE					m_hEvtStop;
//PROCESS_INFORMATION		pi;
//DWORD					exitcode = 0;
CString					g_csPreviousFile = L"";
CString					g_csPreviousFilePath = L"";
CString					g_csPreviousStatus = L"";
CString					g_csPreviousVirusFoundPath = L"";

//Thread initialization here
//DWORD WINAPI PipeToClamAV(LPVOID lpvThreadParam);
//DWORD WINAPI CalculateTotalFilesInFolder(LPVOID lpvThreadParam);
DWORD WINAPI StatusEntryThread(LPVOID lpvThreadParam);
DWORD WINAPI VirusFoundEntriesThread(LPVOID lpvThreadParam);
DWORD WINAPI QuarantineThread(LPVOID lpParam);
UINT PlayScanFinishedThread(LPVOID lpThis);
UINT PlayThreatsFoundThread(LPVOID lpThis);

//DWORD WINAPI GetScanFilesCount(LPVOID lpParam ) ;
//HANDLE m_hThread_ScanCount = NULL ;

bool CScanDlg::m_instanceFlag = false;
CScanDlg* CScanDlg::m_pobjScanDlg = NULL;
CISpyGUIDlg *g_TabCtrlWindHandle = NULL;
IMPLEMENT_DYNAMIC(CScanDlg, CDialog)

/***************************************************************************
  Function Name  : CScanDlg
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 26th Nov 2013
****************************************************************************/
CScanDlg::CScanDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CScanDlg::IDD, pParent)
	,m_bRedFlag(false)
	,m_bScanStarted(false)
	,m_bScanningStopped(false)
	,m_bQuarantineFinished(false)
	,m_dwTotalScanPathCount(1)
	,m_virusFound(0)
	,m_bScnAborted(false)
	,m_bPlaySound(true)
	,m_objIPCClient(FILESYSTEMSCAN)
	,m_objIPCClientVirusFound(VIRUSFOUNDENTRY)
	,m_hThreadVirEntries(NULL)
	,m_objCom(SERVICE_SERVER, true)
	,m_bClose(false)
	,m_bHome(false)
	,m_bQuickScan(false)
	,m_bCustomscan(false)
	,m_bFullScan(false)
	,m_bIsCleaning(false)
	,m_hQuarantineThread(NULL)
	,m_bScanStartedStatusOnGUI(false)
	,m_dwFailedDeleteFileCount(0)
	/*
		ISSUE NO - 44 Two dialogs appears after full scan 
		NAME - Niranjan Deshak. - 12th Jan 2015
	*/
	,m_bFlagScanFinished(true)
	, m_bSignatureFailed(false) //Varada Ikhar	
	, m_stDragDropFiles(&m_lstDrivePicker)
	, m_bOnWMClose(false)
	, m_bIsMemScan(false)
	, m_bIsPopUpDisplayed(false)
	, m_edtStatus(DT_LEFT | DT_NOPREFIX | DT_VCENTER,FALSE,ESCANDIALOG)
{
	m_eScanType = QUICKSCAN;
	m_eCurrentSelectedScanType = QUICKSCAN;
	m_hScanStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	if (theApp.m_eScanLevel == CLAMSCANNER)
	{
		m_bIsMemScan = true;
	}
}

/***************************************************************************
  Function Name  : CScanDlg
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 26th Nov 2013
****************************************************************************/
CScanDlg::~CScanDlg()
{
	if(m_hScanStopEvent)
	{
		::CloseHandle(m_hScanStopEvent);
		m_hScanStopEvent = NULL;
	}
	m_instanceFlag = false;
}

/***************************************************************************
  Function Name  : GetScanDlgInstance
  Description    : CWnd* pParent
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 03 May 2013
****************************************************************************/
CScanDlg* CScanDlg::GetScanDlgInstance(CWnd* pParent)
{
    if(! m_instanceFlag)
    {
        m_pobjScanDlg = new CScanDlg(pParent);
        m_instanceFlag = true;
        return m_pobjScanDlg;
    }
    else
    {
        return m_pobjScanDlg;
    }
}

/***************************************************************************
  Function Name  : ResetInstance
  Description    : Reset and instance for CScanDlg from outside.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 03 May 2013
****************************************************************************/
void CScanDlg::ResetInstance()
{
	delete m_pobjScanDlg; // REM : it works even if the pointer is NULL (does nothing then)
	m_pobjScanDlg = NULL; // so ResetInstance will still work.
}

/***************************************************************************
  Function Name  : DoDataExchange
  Description    : Called by the framework to exchange and validate dialog data.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 26th Nov 2013
****************************************************************************/
void CScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FULLSCANTITLE, m_stTitle);
	DDX_Control(pDX, IDC_STATIC_FILES_SCANNED, m_csFilesScanned);
	DDX_Control(pDX, IDC_STATIC_ELAPSED_TIME, m_stElapsedTime);
	DDX_Control(pDX, IDC_STATIC_THREATS_FOUND, m_stThreatsFound);
	DDX_Control(pDX, IDC_LIST_VIRUSLIST, m_lstVirusesList);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_edtStatus);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPauseResume);
	//	DDX_Control(pDX, IDC_BACK_BUTTON, m_btnBack);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BUTTON_SPYCLEAN, m_btnClean);
	//DDX_Control(pDX, IDC_BTN_QUICKSCAN, m_btnQuickScn);
	//DDX_Control(pDX, IDC_BTN_FULLSCAN, m_btnFullScn);
	//DDX_Control(pDX, IDC_BTN_CUSTOMSCAN, m_btnCustomScn);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_EdtInputPath);
	DDX_Control(pDX, IDC_BUTTON_BROWSEPATH, m_btnBrowse);
	DDX_Control(pDX, IDC_BUTTON_CUSTOMSCAN, m_btninnerCutomScan);
	DDX_Control(pDX, IDC_ST_HEADERALL, m_stHeaderAll);
	DDX_Control(pDX, IDC_BUTTON_START_SCAN, m_btnScan);
	DDX_Control(pDX, IDC_SCAN_PROGRESS, m_prgScanProgress);
	//DDX_Control(pDX, IDC_STATIC_SCANPERCENTAGE, m_ScanPercentage);
	DDX_Control(pDX, IDC_LIST_DRIVE_CONTROL, m_lstDrivePicker);
	DDX_Control(pDX, IDC_HEADER_REMOVABLEDEVEICE, m_stHeaderRemovable);
	DDX_Control(pDX, IDC_CHECK_SELECTALL, m_chkSelectAll);
	DDX_Control(pDX, IDC_CHECK_DISABLESOUND, m_chkDisableSound);
	DDX_Control(pDX, IDC_STATIC_SELECTALL, m_stSelectAll);
	DDX_Control(pDX, IDC_STATIC_DISABLESOUND, m_stDisableSound);
	DDX_Control(pDX, IDC_STATIC_QUICKSCAN_HEADER_NAME, m_stQuickHeaderName);
	DDX_Control(pDX, IDC_STATIC_QUICKSCAN_HEADER_DES, m_stQuickHeaderDescription);
	DDX_Control(pDX, IDC_STATIC_FULLSCAN_HEADERNAME, m_stFullScanHeaderName);
	DDX_Control(pDX, IDC_STATIC_FULLSCAN_HEADER_DES, m_stFullScanHeaderDes);
	DDX_Control(pDX, IDC_STATIC_CUSTOMSCAN_HEADER_NAME, m_stCustomScnHeaderName);
	DDX_Control(pDX, IDC_STATIC_CUSTOMSCAN_HEADER_DES, m_stCustomHeaderDes);
	DDX_Control(pDX, IDC_STATIC_DRAG_DROP_FILES, m_stDragDropFiles);
	DDX_Control(pDX, IDC_BUTTON_CUSTOM_ADD, m_btnCutomAdd);
	DDX_Control(pDX, IDC_BUTTON_CUSTOM_EDIT, m_btnCustomEdit);
	DDX_Control(pDX, IDC_BUTTON_CUSTOM_DELETE, m_btnCustomDelete);
	DDX_Control(pDX, IDC_CHECK_CUSTOM_SELECT_ALL, m_cbCustomSelectAll);
	DDX_Control(pDX, IDC_STATIC_CUSTOM_DRAG_DROP, m_stCustomDragDropText);
}


/**********************************************************************************************************                     
* Function Name  : MESSAGE_MAP
* Description    : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
*  Author Name    :	Ramkrusha shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
BEGIN_MESSAGE_MAP(CScanDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	//ON_BN_CLICKED(IDC_BACK_BUTTON, &CScanDlg::OnBnClickedBackButton)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CScanDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CScanDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_SPYCLEAN, &CScanDlg::OnBnClickedButtonClean)
	//ON_BN_CLICKED(IDC_BTN_QUICKSCAN, &CScanDlg::OnBnClickedBtnQuickscan)
	//ON_BN_CLICKED(IDC_BTN_FULLSCAN, &CScanDlg::OnBnClickedBtnFullscan)
	//ON_BN_CLICKED(IDC_BTN_CUSTOMSCAN, &CScanDlg::OnBnClickedBtnCustomscan)
	ON_BN_CLICKED(IDC_BUTTON_START_SCAN, &CScanDlg::OnBnClickedButtonStartScan)
	ON_BN_CLICKED(IDC_BUTTON_CUSTOMSCAN, &CScanDlg::OnBnClickedButtonCustomscan)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEPATH, &CScanDlg::OnBnClickedButtonBrowsepath)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_VIRUSLIST, &CScanDlg::OnNMCustomdrawListViruslist)
	ON_BN_CLICKED(IDC_CHECK_SELECTALL, &CScanDlg::OnBnClickedCheckSelectall)
	ON_BN_CLICKED(IDC_CHECK_DISABLESOUND, &CScanDlg::OnBnClickedCheckDisablesound)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_SELECT_ALL, &CScanDlg::OnBnClickedCheckCustomSelectAll)
	ON_BN_CLICKED(IDC_BUTTON_CUSTOM_ADD, &CScanDlg::OnBnClickedButtonCustomAdd)
	ON_BN_CLICKED(IDC_BUTTON_CUSTOM_EDIT, &CScanDlg::OnBnClickedButtonCustomEdit)
	ON_BN_CLICKED(IDC_BUTTON_CUSTOM_DELETE, &CScanDlg::OnBnClickedButtonCustomDelete)
END_MESSAGE_MAP()

// CScanDlg message handlers

/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.NO		  : 
*  Author Name    : Ramkrushna Shelke
*  Date           : 26 Nov 2013
**********************************************************************************************************/
BOOL CScanDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_INNER_DIALOG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	m_objIPCClient.OpenServerMemoryMappedFile();
	m_objIPCClientVirusFound.OpenServerMemoryMappedFile();

	memset(m_szOriginalFilePath, 0x00, sizeof(TCHAR)*512 ) ;
	memset(m_szAppData, 0x00, sizeof(TCHAR)*512 ) ;
	GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), m_szAppData, 511 ) ;

	//Rajil Yadav 09/06/2014. Multiple name occurs on Same WIndow Header.
	CRect rect1;
	this->GetClientRect(rect1);

	m_stQuickHeaderName.SetTextColor(RGB(24,24,24));
	m_stQuickHeaderName.SetBkColor(RGB(230,232,238));
	m_stQuickHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stQuickHeaderName.SetWindowPos(&wndTop,rect1.left + 20,07,400,31,SWP_NOREDRAW);
	m_stQuickHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_QUICKSCAN_HEADER"));

	m_stQuickHeaderDescription.SetTextColor(RGB(24,24,24));
	m_stQuickHeaderDescription.SetBkColor(RGB(230,232,238));
	m_stQuickHeaderDescription.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stQuickHeaderDescription.SetWindowPos(&wndTop,rect1.left + 20,35,400,15,SWP_NOREDRAW);
	m_stQuickHeaderDescription.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_QUICKSCAN_DES"));

	//m_lstVirusesList.InsertColumn(0, L"", LVCFMT_LEFT, 20);

	m_lstVirusesList.InsertColumn(0,theApp.m_objwardwizLangManager.GetString(L"IDS_LSTCTRL_TREATNAME"), LVCFMT_LEFT, 100);
	m_lstVirusesList.InsertColumn(1,theApp.m_objwardwizLangManager.GetString(L"IDS_LSTCTRL_FILEPATH"), LVCFMT_LEFT, 380);
	m_lstVirusesList.InsertColumn(2,theApp.m_objwardwizLangManager.GetString(L"IDS_LSTCTRL_ACTION"), LVCFMT_LEFT, 102);
	m_lstVirusesList.InsertColumn(3, L"WardWizID", LVCFMT_LEFT,0);
	m_lstVirusesList.SetTextColor(RGB(100,100,100));

	CHeaderCtrl* pHeaderCtrl = m_lstVirusesList.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);

	ListView_SetExtendedListViewStyle (m_lstVirusesList.m_hWnd, LVS_EX_CHECKBOXES |  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	//Issue: Check box Enabled for Custom Scan List box Resolved By: Nitin K Date:24th April 2015
	ListView_SetExtendedListViewStyle (m_lstDrivePicker.m_hWnd, LVS_EX_CHECKBOXES |  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);

	int nFlags = 0;
	nFlags |= DDS_DLIL_HARDDRIVES;
	nFlags |= DDS_DLIL_CDROMS;
	nFlags |= DDS_DLIL_NETDRIVES;
	nFlags |= DDS_DLIL_REMOVABLES;
	nFlags |= DDS_DLIL_RAMDRIVES;

	//m_lstDrivePicker.
	//InitList (16, nFlags );
	//m_lstDrivePicker.ShowWindow(SW_HIDE);

	//Varada Ikhar, Date:26/02/2015, Issue:The UI look for space between header and Listview should be standard.
	m_lstVirusesList.SetWindowPos(&wndTop, rect1.left + 6, 68, 586, 207, SWP_NOREDRAW);

	/*****************ISSUE NO -228,374 Neha Gharge 22/5/14 ************************************************/
	m_chkSelectAll.SetWindowPos(&wndTop, rect1.left + 8, 220 + 65, 13 , 13, SWP_NOREDRAW);
	m_chkSelectAll.ShowWindow(SW_SHOW);
	m_chkSelectAll.SetCheck(true);
	m_stSelectAll.SetWindowPos(&wndTop,rect1.left + 27,220 + 64,120,17,SWP_NOREDRAW);
	m_stSelectAll.SetBkColor(RGB(70, 70, 70));
	//m_stSelectAll.SetWindowTextW(L"Select All");
	m_stSelectAll.SetTextColor(RGB(255,255,255));
	m_stSelectAll.SetFont(&theApp.m_fontWWTextNormal);
	m_stSelectAll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SELECTALL"));



	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));


	m_chkDisableSound.SetWindowPos(&wndTop,rect1.left + 507,220 + 61, 12, 12, SWP_HIDEWINDOW);
	m_chkDisableSound.SetCheck(TRUE);
	m_stDisableSound.SetWindowPos(&wndTop,rect1.left + 523,220 + 61,90,17,SWP_HIDEWINDOW);
	//m_stDisableSound.SetWindowTextW(L"Disable Sound");
	m_stDisableSound.SetBkColor(RGB(70, 70, 70));
	m_stDisableSound.SetTextColor(RGB(255,255,255));
	m_stDisableSound.SetFont(&theApp.m_fontWWTextNormal);

	m_stElapsedTime.SetBkColor(RGB(70, 70, 70));
	m_stElapsedTime.SetTextColor(RGB(255, 255, 255));
	//m_stElapsedTime.SetWindowTextW(L"Elapsed Time: 00:00:00");
	m_stElapsedTime.SetWindowPos(&wndTop, rect1.left + 6, 320, 180,17, SWP_NOREDRAW);
	m_stElapsedTime.SetFont(&theApp.m_fontWWTextNormal);


	m_csFilesScanned.SetBkColor(RGB(70, 70, 70));
	m_csFilesScanned.SetTextColor(RGB(255, 255, 255));
	//m_csFilesScanned.SetWindowTextW(L"Files Scanned:   0");
	m_csFilesScanned.SetWindowPos(&wndTop, rect1.left + 195, 320, 180, 17, SWP_NOREDRAW);
	m_csFilesScanned.SetFont(&theApp.m_fontWWTextNormal);

	m_stThreatsFound.SetBkColor(RGB(70, 70, 70));
	m_stThreatsFound.SetTextColor(RGB(255, 255, 255));
	//m_stThreatsFound.SetWindowTextW(L"Threats Found:  0");
	m_stThreatsFound.SetWindowPos(&wndTop, rect1.left+375, 320, 180, 17, SWP_NOREDRAW);
	m_stThreatsFound.SetFont(&theApp.m_fontWWTextNormal);

	//Issue no: 1151 Short name issue. if long file path it comes C:\...\xyz.txt format Neha Gharge
	m_edtStatus.SetBkColor(RGB(70, 70, 70));
	m_edtStatus.SetWindowPos(&wndTop, rect1.left + 5, 373, 570 , 22, SWP_NOREDRAW);
	m_edtStatus.SetTextColor(RGB(255,255,255));
	m_edtStatus.setFont(&theApp.m_fontWWTextNormal);
	m_edtStatus.SetPath(TRUE);
	m_edtStatus.setTextFormat(DT_LEFT | DT_VCENTER);
	GetDlgItem(IDC_EDIT_STATUS)->ModifyStyle(0, WS_DISABLED);

	//ISsue Not reported neha Gharge 23/5/2014***********************************/
	m_prgScanProgress.SetWindowPos(&wndTop, rect1.left + 6, 345, 455 , 21, SWP_SHOWWINDOW);
	m_prgScanProgress.SetFont(&theApp.m_fontWWTextNormal);

	//m_ScanPercentage.SetWindowPos(&wndTop, rect1.left + 27 + 523, 374, 50, 20, SWP_NOREDRAW);
	//m_ScanPercentage.SetFont(&theApp.m_fontText);
	//m_ScanPercentage.SetBkColor(RGB(255,255,255));
	////m_ScanPercentage.SetWindowText(L"0%");
	//m_ScanPercentage.ShowWindow(SW_HIDE);

	m_prgScanProgress.AlignText(DT_CENTER);
	m_prgScanProgress.SetBarColor(RGB(171,238,0));
	//m_prgScanProgress.SetBarColor(RGB(243,239,238));
	m_prgScanProgress.SetBkColor(RGB(243,239,238));
	m_prgScanProgress.SetShowPercent(true);
	/*Rajil Yadav Issue no. 555, 6/4/2014*/
	m_btnScan.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnScan.SetWindowPos(&wndTop,rect1.left + 468,345,57,21,SWP_NOREDRAW);
	m_btnScan.SetTextColorA(BLACK,1,1);
	m_btnScan.SetFont(&theApp.m_fontWWTextNormal);
	m_btnScan.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));

	m_btnPauseResume.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnPauseResume.SetWindowPos(&wndTop,rect1.left + 468,345,57,21,SWP_NOREDRAW);
	m_btnPauseResume.SetTextColorA(BLACK,1,1);
	m_btnPauseResume.SetFont(&theApp.m_fontWWTextNormal);
	m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));

	m_btnStop.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnStop.SetWindowPos(&wndTop, rect1.left + 535,345, 57, 21, SWP_NOREDRAW);
	m_btnStop.SetTextColorA(BLACK,1,1);
	m_btnStop.SetFont(&theApp.m_fontWWTextNormal);
	m_btnStop.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"));

	//issue clean button has border
	m_btnClean.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnClean.SetWindowPos(&wndTop, rect1.left + 535 , 345, 57, 21, SWP_HIDEWINDOW);
	m_btnClean.SetTextColorA(BLACK,1,1);
	m_btnClean.SetFont(&theApp.m_fontWWTextNormal);
	m_btnClean.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CLEAN"));

	m_EdtInputPath.SetWindowPos(&wndTop, rect1.left + 6, 68 , 455 , 21, SWP_HIDEWINDOW);
	m_btnBrowse.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnBrowse.SetWindowPos(&wndTop,rect1.left + 468, 68, 57, 21, SWP_HIDEWINDOW);
	m_btnBrowse.SetTextColorA(BLACK,1,1);
	m_btnBrowse.SetFont(&theApp.m_fontWWTextNormal);
	//Issue: Custom scan UI implementation
	//Added by: Nitin K.
	m_btninnerCutomScan.SetSkin(theApp.m_hResDLL, IDB_BITMAP_CUSTOM_SCAN, IDB_BITMAP_CUSTOM_SCAN, IDB_BITMAP_CUSTOM_SCAN, IDB_BITMAP_CUSTOM_SCAN, 0, 0, 0, 0, 0);
	//Updated By Nitin K. 19th March 2015
	m_btninnerCutomScan.SetWindowPos(&wndTop, rect1.left + 420, 370, 172, 21, SWP_HIDEWINDOW);
	m_btninnerCutomScan.SetTextColorA(BLACK,1,1);
	m_btninnerCutomScan.SetFont(&theApp.m_fontWWTextNormal);

	m_eScanType = QUICKSCAN;
	m_eCurrentSelectedScanType = QUICKSCAN;
	m_csaAllScanPaths.Add(L"QUICKSCAN");

	m_csaAllScanPaths.RemoveAll();

	ReadUISettingFromRegistry();

	m_prgScanProgress.SetRange(0, 100);
	m_prgScanProgress.SetPos(0);

	//commented due to crash happening
	//SetReportDate();

	m_stFullScanHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FULLSCAN_HEADER"));
	m_stFullScanHeaderName.SetTextColor((24,24,24));
	m_stFullScanHeaderName.SetBkColor(RGB(230,232,238));
	m_stFullScanHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stFullScanHeaderName.SetWindowPos(&wndTop,rect1.left + 20,07,400,31,SWP_NOREDRAW);
	m_stFullScanHeaderName.ShowWindow(SW_HIDE);

	m_stFullScanHeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FULLSCN_DES"));
	m_stFullScanHeaderDes.SetTextColor(RGB(24,24,24));
	m_stFullScanHeaderDes.SetBkColor(RGB(230,232,238));
	m_stFullScanHeaderDes.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stFullScanHeaderDes.SetWindowPos(&wndTop,rect1.left + 20,35,400,18,SWP_NOREDRAW);
	m_stFullScanHeaderDes.ShowWindow(SW_HIDE);



	m_stCustomScnHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOMSCAN_HEADER"));
	m_stCustomScnHeaderName.SetTextColor((24,24,24));
	m_stCustomScnHeaderName.SetBkColor(RGB(230,232,238));
	m_stCustomScnHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stCustomScnHeaderName.SetWindowPos(&wndTop,rect1.left + 20,07,400,31,SWP_NOREDRAW);
	m_stCustomScnHeaderName.ShowWindow(SW_HIDE);


	m_stCustomHeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOMSCN_DES"));
	m_stCustomHeaderDes.SetTextColor((24,24,24));
	m_stCustomHeaderDes.SetBkColor(RGB(230,232,238));
	m_stCustomHeaderDes.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stCustomHeaderDes.SetWindowPos(&wndTop,rect1.left + 20,35,400,18,SWP_NOREDRAW);
	m_stCustomHeaderDes.ShowWindow(SW_HIDE);

	if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	{
		m_stQuickHeaderName.SetWindowPos(&wndTop,rect1.left + 20,14,400,31,SWP_NOREDRAW);
		m_stQuickHeaderDescription.SetWindowPos(&wndTop,rect1.left + 20,38,400,15,SWP_NOREDRAW);
		m_stFullScanHeaderName.SetWindowPos(&wndTop,rect1.left + 20,14,400,31,SWP_NOREDRAW);
		m_stFullScanHeaderDes.SetWindowPos(&wndTop,rect1.left + 20,38,400,18,SWP_NOREDRAW);
		m_stCustomScnHeaderName.SetWindowPos(&wndTop,rect1.left + 20,14,400,31,SWP_NOREDRAW);
		m_stCustomHeaderDes.SetWindowPos(&wndTop,rect1.left + 20,38,400,18,SWP_NOREDRAW);
	}

	m_bmpQuickScan = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stHeaderAll.SetWindowPos(&wndTop,rect1.left + 6,10,582,45,SWP_NOREDRAW);
	m_stHeaderAll.SetBitmap(m_bmpQuickScan);
	ResetControlsValues();
	//Design implementation of Custom scan 
	//Added by: Nitin K. 28th March 2015
	m_bmpDragAndDrop = LoadBitmapW(theApp.m_hResDLL, MAKEINTRESOURCE(IDB_BITMAP_CUSTOM_DRAG_DROP));
	m_stDragDropFiles.SetWindowPos(&wndTop, rect1.left + 420, 70, 173, 250, SWP_NOREDRAW);
	m_stDragDropFiles.SetBitmap(m_bmpDragAndDrop);

	m_stCustomDragDropText.SetTextColor(RGB(255,255,255));
	//m_stCustomDragDropText.SetBkColor(RGB(241,241,242));
	m_stCustomDragDropText.SetBkColor(RGB(70,70,70));
	m_stCustomDragDropText.SetFont(&theApp.m_fontWWTextNormal);
	m_stCustomDragDropText.SetWindowPos(&wndTop, rect1.left + 430, 330, 150, 30, SWP_SHOWWINDOW);

//	m_stDragDropFiles.SetBitmap(m_bmpQuickScan);
	m_lstDrivePicker.SetWindowPos(&wndTop, rect1.left + 5, 70, 405, 290, SWP_SHOWWINDOW);
	
	m_cbCustomSelectAll.SetWindowPos(&wndTop, rect1.left + 5, 375, 13, 13, SWP_SHOWWINDOW);
	//Issue: Check box Enabled for Custom Scan List box Resolved By: Nitin K Date:24th April 2015
	m_cbCustomSelectAll.SetCheck(true);
	//m_cbCustomSelectAll.SetBkColor(RGB(70, 70, 70));
	//m_cbCustomSelectAll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SELECTALL"));
	
	m_btnCutomAdd.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN57x21, IDB_BITMAP_BTN57x21, IDB_BITMAP_57x21_H_over, IDB_BITMAP_57x21_DISABLE, 0, 0, 0, 0, 0);
	m_btnCutomAdd.SetWindowPos(&wndTop, rect1.left + 232, 370, 57, 21, SWP_NOREDRAW);
	m_btnCutomAdd.SetTextColorA(BLACK, 1, 1);
	m_btnCutomAdd.SetFont(&theApp.m_fontWWTextNormal);
	m_btnCutomAdd.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));

	m_btnCustomEdit.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN57x21, IDB_BITMAP_BTN57x21, IDB_BITMAP_57x21_H_over, IDB_BITMAP_57x21_DISABLE, 0, 0, 0, 0, 0);
	m_btnCustomEdit.SetWindowPos(&wndTop, rect1.left + 292, 370, 57, 21, SWP_NOREDRAW);
	m_btnCustomEdit.SetTextColorA(BLACK, 1, 1);
	m_btnCustomEdit.SetFont(&theApp.m_fontWWTextNormal);
	m_btnCustomEdit.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_EDIT"));
	
	m_btnCustomDelete.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN57x21, IDB_BITMAP_BTN57x21, IDB_BITMAP_57x21_H_over, IDB_BITMAP_57x21_DISABLE, 0, 0, 0, 0, 0);
	m_btnCustomDelete.SetWindowPos(&wndTop, rect1.left + 352, 370, 57, 21, SWP_NOREDRAW);
	m_btnCustomDelete.SetTextColorA(BLACK, 1, 1);
	m_btnCustomDelete.SetFont(&theApp.m_fontWWTextNormal);
	m_btnCustomDelete.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DELETE"));

	m_EdtInputPath.ShowWindow(SW_HIDE);
	m_lstDrivePicker.ShowWindow(SW_HIDE);
	m_stDragDropFiles.EnableWindow(FALSE);
	m_stDragDropFiles.ShowWindow(SW_HIDE);
	m_stCustomDragDropText.ShowWindow(SW_HIDE);
	m_btnCutomAdd.ShowWindow(SW_HIDE);
	m_btnCustomDelete.ShowWindow(SW_HIDE);
	m_btnCustomEdit.ShowWindow(SW_HIDE);
	m_cbCustomSelectAll.ShowWindow(SW_HIDE);

	LoadCutomScanINI();
	showListControlForCustomScan();
	RefreshStrings();
	//Issue : Click any scan->the message below the progress bar should not shift from one side to other
	//if we right click & select 'right to left' option.
	//Resolved By : Nitin K  Date : 10th March 2015
	GetDlgItem(IDC_EDIT_STATUS)->ModifyStyle(0, WS_DISABLED);
	GetDlgItem(IDC_EDIT_INPUT)->ModifyStyle(0, WS_DISABLED);
	return TRUE;  // return TRUE unless you set the focus to a control
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
HBRUSH CScanDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	//*****************************ISSUE NO : 374 Neha gharge 24/5/2014 **********************************/
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int	ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if(	ctrlID  == IDC_EDIT_STATUS)
	{
		if(m_bRedFlag)
		{
			pDC->SetTextColor(RGB(255, 0, 0));
		}
		//pDC->SetBkMode(TRANSPARENT);
	}
	else if ( ctrlID  == IDC_EDIT_STATUS )
	{
		//CBrush m_brHollow;
		//m_brHollow.CreateStockObject(HOLLOW_BRUSH);
		//pDC->SetBkColor(RGB(255,255,255));
		//hbr = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}	
	else if ( ctrlID == IDC_STATIC_QUICKSCAN_HEADER_NAME ||
		 ctrlID == IDC_STATIC_QUICKSCAN_DESC ||
		 ctrlID == IDC_STATIC_QUICKSCAN_HEADER_DES ||
		 ctrlID == IDC_STATIC_FULLSCAN_HEADERNAME ||
		 ctrlID == IDC_STATIC_FULLSCAN_HEADER_DES ||
		 ctrlID == IDC_STATIC_CUSTOMSCAN_HEADER_NAME ||
		 ctrlID == IDC_STATIC_CUSTOMSCAN_HEADER_DES
     )
  	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} 
	return hbr;
}

//void CScanDlg::OnBnClickedBackButton()
//{
//	if(m_virusFound > 0 && !m_bQuarantineFinished)
//	{
//		int iRet = MessageBox(L"Viruses are detected, do you want to go back?", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONEXCLAMATION);
//		if(iRet == IDNO)
//		{
//			return;
//		}
//	}
//	
//	ResetControlsValues();
//
//	this->ShowWindow(SW_HIDE);
//	CISpyGUIDlg *pObjMainUI = reinterpret_cast<CISpyGUIDlg*>( this->GetParent() );
//	pObjMainUI->ShowHideMainPageControls(true);
//}

/**********************************************************************************************************                     
*  Function Name  :	ISDuplicateEntry                                                     
*  Description    :	It checks duplicate entry before inserting into list control
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::ISDuplicateEntry(CString strScanFileName)
{
	bool bReturn = false;
	for(int i = 0; i < m_lstVirusesList.GetItemCount(); i++)
	{
		if(strScanFileName == m_lstVirusesList.GetItemText(i, 1))
		{
			bReturn = true;
			break;
		}
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	InsertItem                                                     
*  Description    :	Insert virus name ,virus file name ,ID and action into list control
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::InsertItem(CString strVirusName, CString strScanFileName, CString csAction, CString csISpyID)
{
	try
	{
		if(strScanFileName.GetLength()	== 0		|| 
			strVirusName.GetLength()	== 0		|| 
			csAction.GetLength()		== 0		|| 
			csISpyID.GetLength()		== 0)
		{
			return;
		}

		if(ISDuplicateEntry(strScanFileName))
		{
			return;
		}

		//m_FileScanned++;
		//m_virusFound++;

		if(m_virusFound % 5 == 0)
		{
			/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
			if(theApp.m_bEnableSound)
			{
				AfxBeginThread(PlayThreatsFoundThread, NULL);
			}
		}

		LVITEM lvItem;
		int nItem;
		int imgNbr = 0;
		lvItem.mask = LVIF_IMAGE;
		lvItem.iItem = 0;
		lvItem.iSubItem = 0;
		lvItem.iImage = 0;
		nItem = m_lstVirusesList.InsertItem(&lvItem);

		CString cstmp;
		m_lstVirusesList.SetItemText(nItem, 0, strVirusName);
		m_lstVirusesList.SetItemText(nItem, 1, strScanFileName);
		m_lstVirusesList.SetItemText(nItem, 2, csAction);
		m_lstVirusesList.SetItemText(nItem, 3, csISpyID);
		//m_lstVirusesList.SetCheck(nItem, TRUE);
		int iCheck = m_chkSelectAll.GetCheck();
		if (iCheck == 1)
		{
			m_lstVirusesList.SetCheck(nItem, BST_CHECKED);
		}
		else
		{
			m_lstVirusesList.SetCheck(nItem, BST_UNCHECKED);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::InsertItem", 0, 0, true, SECONDLEVEL);
	}
//#if 1
//	TCHAR Datebuff[9];
//	TCHAR csDate[9];
//	_wstrdate_s(Datebuff,9);
//	_tcscpy(csDate,Datebuff);
//	AddEntriesInReportsDB(csDate,CTime::GetCurrentTime(),m_eScanType,strVirusName,strScanFileName,csAction);
//#endif
}

/**********************************************************************************************************                     
*  Function Name  :	StartUSBScan                                                     
*  Description    :	To start USB scan
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::StartUSBScan()
{	
	SetControls4USBDetect();
	m_btnScan.EnableWindow(FALSE);
	StartScanning();
}	

/**********************************************************************************************************                     
*  Function Name  :	StartScanning                                                     
*  Description    :	To start Full scan ,custom scan and quick scan accoeding m_scantype variable.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::StartScanning()
{
	try
	{
		m_bRedFlag = false;

		UINT uRetVal = 0;
		m_bQuickScan = false;
		m_bFullScan = false;
		m_bCustomscan = false;
		// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
		//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"  
		m_eScanType = m_eCurrentSelectedScanType;
		m_bScnAborted = false;
		//Varada Ikhar, Date:17-03-2015, 
		//Issue:User updated the product and does not restarted, and started a scan, then it should show message as 
		//"Product updated, restart is required to take a effect."
		if (!theApp.ShowRestartMsgOnProductUpdate())
		{
			return;
		}

		//Issue : Synchrnization in Scan-Pause-Resume Button
		//Resolved By : Nitin K. Date : 09th March 2015
		BeginWaitCursor();
		//Issue				: Scanning status shows "Scanning Started ..." after completion.
		//Date				: 29 - Nov - 2014
		//Issue Resolved	: Vilas
		m_bScanStartedStatusOnGUI = true;

		if(!GetScanningPaths(m_csaAllScanPaths))
		{
			return;
		}
		//Check for DB files at installation path
		if(!Check4DBFiles())
		{
			//Ticket No.526, No action will take place if check DB get failed. 
			// m_bQuickscan,m_bFullscan and m_bCustomScan not getting reset.
			//Neha Gharge 2 June,2015.
			AddLogEntry(L"### Function Check4DBFiles failed", 0, 0, true, SECONDLEVEL);
			ReInitializeVariables();
			m_bQuickScan = false;
			m_bFullScan = false;
			m_bCustomscan = false;
			return;
		}
		m_iTotalFileCount = 0;
		m_iMemScanTotalFileCount = 0;
		if(!StartScanUsingService(m_csaAllScanPaths))
		{
			AddLogEntry(L"### Failed to send data to service", 0, 0, true, SECONDLEVEL);
			m_bQuickScan = false;
			m_bFullScan = false;
			m_bCustomscan = false;
			return;
		}

		// ISSUE NO : 422 Neha Gharge ***************************************************/
		g_TabCtrlWindHandle =(CISpyGUIDlg*)AfxGetMainWnd();	
		if(g_TabCtrlWindHandle !=NULL)
		{
			// lalit  4-27-2015 , due to mutuall operation implementaion no more need to hide other tab control when any one is in-progress 
			//g_TabCtrlWindHandle->m_pTabDialog->DisableAllExceptSelected(); // Commented by lalit 4-23-2015
		}
		m_hThreadVirEntries = ::CreateThread(NULL, 0, VirusFoundEntriesThread, (LPVOID) this, 0, NULL);
		Sleep( 500 ) ; //Name: Varada Ikhar, Date:22/01/2015 Version:1.8.3.3
		//Issue Description: In full scan, scan count gets displayed starting from count 30 onwards.

		m_hThreadStatusEntry = ::CreateThread(NULL, 0, StatusEntryThread, (LPVOID) this, 0, NULL);
		Sleep( 500 ) ;//Name: Varada Ikhar, Date:22/01/2015 Version:1.8.3.3
		//Issue Description: In full scan, scan count gets displayed starting from count 30 onwards.
		m_chkSelectAll.EnableWindow(FALSE);
		ReInitializeBeforeStartScan();
		EnableAllWindows(FALSE);

		SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCANNER_LAUNCH"));

		//Varada Ikhar, Date: 18/04/2015 
		//Issue : 0000128 : In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
		CString csScanStarted = L"";
		switch (m_eScanType)
		{
		case QUICKSCAN:	
			csScanStarted = L">>> Quick scanning started...";
			break;
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
		AddLogEntry(csScanStarted, 0, 0, true, SECONDLEVEL);

		////Add scan start time log entry
		m_tsScanStartTime   = CTime::GetCurrentTime();
		m_tsScanPauseResumeElapsedTime -= m_tsScanPauseResumeElapsedTime;

		SetTimer(TIMER_SCAN_STATUS, 1000, NULL);
		SetTimer(TIMER_FILESSCANNED_STATUS, 800, NULL);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartScanning", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	ReInitializeBeforeStartScan                                                     
*  Description    :	Some variables and controls need to initialize before start any type of scan.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::ReInitializeBeforeStartScan()
{
	try
	{
		
		ResetControlsValues();

		g_csPreviousFilePath = L"";
		g_csPreviousFile = L"";
		m_csPreviousFile = L"";
		g_csPreviousStatus = L"";
		g_csPreviousVirusFoundPath = L"";

	//	m_btnBack.EnableWindow(FALSE);
		m_btnPauseResume.EnableWindow(TRUE);
		m_btnStop.EnableWindow(TRUE);

		m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
		m_btnPauseResume.ShowWindow(TRUE);
		//Issue : Synchrnization in Scan-Pause-Resume Button
		//Resolved By : Nitin K. Date : 09th March 2015
		EndWaitCursor();
		m_btnClean.ShowWindow(FALSE);
		m_btnStop.ShowWindow(TRUE);
		m_btnScan.ShowWindow(FALSE);

		//In custom scan->Browse any folder->start scan->immediately pause when Elapsed time is 0->
		//then press the stop button->Elapsed Time shows 3 or 5 sec. 
		//Niranjan Deshak - 12/03/2015.
		CTimeSpan span = m_tsScanElapsedTime.GetTimeSpan();
		m_tsScanElapsedTime.operator-=(span);
	
		switch( m_eScanType )
		{
		case QUICKSCAN:
			//m_btnFullScn.EnableWindow(FALSE);
			//m_btnCustomScn.EnableWindow(FALSE);
			break;
		case FULLSCAN:
			//m_btnQuickScn.EnableWindow(FALSE);
			//m_btnCustomScn.EnableWindow(FALSE);
			break;
		case CUSTOMSCAN:
			m_btninnerCutomScan.ShowWindow(TRUE);
			//m_btnQuickScn.EnableWindow(FALSE);
			//m_btnFullScn.EnableWindow(FALSE);
			m_btnScan.ShowWindow(FALSE);
			//m_btnBrowse.ShowWindow(TRUE);
			break;
		case USBSCAN:
		case USBDETECT:	
		//	m_btnCustomScn.ShowWindow(FALSE);
			m_btninnerCutomScan.ShowWindow(FALSE);
			//m_btnQuickScn.ShowWindow(FALSE);
		//	m_btnFullScn.ShowWindow(FALSE);
			m_btnScan.EnableWindow(TRUE);
			m_btnBrowse.ShowWindow(FALSE);
			m_lstDrivePicker.EnableWindow(FALSE);
			m_btnPauseResume.ShowWindow(FALSE);
			m_btnStop.ShowWindow(FALSE);

			break;
		}
		//Invalidate();
		TextSetWindow(L"##");
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in void CScanDlg::ReInitialize", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	SetScanStatus                                                     
*  Description    :	Write a process status into edit control on UI
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::SetScanStatus(CString csStatus)
{
	m_edtStatus.SetWindowText(csStatus);
	m_edtStatus.SetFont(&theApp.m_fontWWTextNormal);
	//m_edtStatus.Invalidate();
	m_edtStatus.UpdateWindow();
}

/**********************************************************************************************************                     
*  Function Name  :	StatusEntryThread                                                     
*  Description    :	A thread which read data from shared memory and shows on Scan dialog UI
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
DWORD WINAPI StatusEntryThread(LPVOID lpvThreadParam)
{
	AddLogEntry(L">>> In StatusEntryThread", 0, 0, true, FIRSTLEVEL);
	CScanDlg *pThis = (CScanDlg*)lpvThreadParam;
	if(!pThis)
		return 0;

	while(true)
	{ 
		CString csCurrentStatus(L"");

		ITIN_MEMMAP_DATA iTinMemMap = {0};
		pThis->m_objIPCClient.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));
	
		switch(iTinMemMap.iMessageInfo)
		{
		case SETSCANSTATUS:
			{
				csCurrentStatus = iTinMemMap.szFirstParam;
				pThis->m_virusFound = iTinMemMap.dwSecondValue;
				
				//Name: Varada Ikhar, Date:22/01/2015 Version:1.8.3.3
				//Issue Description: In full scan, scan count gets displayed starting from count 30 onwards.
 				//This check is for files count going beyond number of files present.
				if (pThis->m_bIsMemScan)
				{
					if (pThis->m_FileScanned <= pThis->m_iMemScanTotalFileCount || pThis->m_iMemScanTotalFileCount == 0)
					{
						pThis->m_FileScanned = iTinMemMap.dwFirstValue;
					}
				}
				else if (pThis->m_FileScanned <= pThis->m_iTotalFileCount || pThis->m_iTotalFileCount == 0)
				{
					pThis->m_FileScanned = iTinMemMap.dwFirstValue; 
				}

				if(csCurrentStatus.GetLength() > 0 && (g_csPreviousStatus != csCurrentStatus))
				{
					OutputDebugString(csCurrentStatus);
					
					pThis->TextSetWindow(csCurrentStatus);
					g_csPreviousStatus.SetString(csCurrentStatus);
				}
			}
			break;
		//Varada Ikhar, Date: 30/04/2015
		//Issue : In custom scan, if folder with very few number of files is given, then scanning is not getting paused on click of pause/stop/close button.
		case DISABLE_CONTROLS:
			//Issue: Virus found count doesnt show on UI in case of fewer no of files
			//Resolved By: Nitin K Date : 15th May 2015
			pThis->m_virusFound = iTinMemMap.dwSecondValue;
			pThis->m_btnPauseResume.EnableWindow(false);
			pThis->m_btnStop.EnableWindow(false);
			g_TabCtrlWindHandle->m_btnClose.EnableWindow(false);
			break;
		}
		csCurrentStatus.ReleaseBuffer();
		Sleep(5);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	VirusFoundEntriesThread                                                     
*  Description    :	A thread which read data of virus found entries from shared memory and shows on Scan dialog UI
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
DWORD WINAPI VirusFoundEntriesThread(LPVOID lpvThreadParam)
{
	AddLogEntry(L">>> In VirusFoundEntriesThread", 0, 0, true, FIRSTLEVEL);
	CScanDlg *pThis = (CScanDlg*)lpvThreadParam;
	if(!pThis)
		return 0;
	
	while(true)
	{ 
		CString csCurrentPath(L"");

		ITIN_MEMMAP_DATA iTinMemMap = {0};
		pThis->m_objIPCClientVirusFound.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));
		
		switch(iTinMemMap.iMessageInfo)
		{
		case SHOWVIRUSFOUND:
			{ 
				csCurrentPath.Format(L"%s", iTinMemMap.szSecondParam);
				if(csCurrentPath.GetLength() > 0 && (g_csPreviousVirusFoundPath != csCurrentPath))
				{
					OutputDebugString(csCurrentPath);
					CString csSpyID;
					csSpyID.Format(L"%d", iTinMemMap.dwFirstValue);
					pThis->InsertItem(iTinMemMap.szFirstParam, csCurrentPath, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"), csSpyID);
				}
				g_csPreviousVirusFoundPath.SetString(csCurrentPath);
			}
			break; 
		}
		csCurrentPath.ReleaseBuffer();
		Sleep(5);
	}
}


/**********************************************************************************************************                     
*  Function Name  :	EnableAllWindows                                                     
*  Description    :	Enable required controls.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::EnableAllWindows(BOOL bEnable)
{
	//m_btnBack.EnableWindow(bEnable);
	m_btnPauseResume.EnableWindow(!bEnable);
	m_btnStop.EnableWindow(!bEnable);
	//Invalidate(TRUE);
}

/**********************************************************************************************************                     
*  Function Name  :	TextSetWindow                                                     
*  Description    :	Set status text.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::TextSetWindow(CString csStatus)
{
	if(csStatus.GetLength() == 0)
		return;

	try
	{
		CString csCurrentFile(L"");
		CString csFilePath(L"");
		CString csVirusName(L"");
		CString csVirusPath(L"");
		CString csISpyID = L"0";

		bool bSetStatus = false;
		bool bVirusFound = false;

		csCurrentFile = csStatus;
		if(csCurrentFile.GetLength() > 0)
		{
			m_bScanStarted = true;
			if(g_csPreviousFile != csCurrentFile)
			{
				//m_FileScanned++;
				csFilePath = csCurrentFile;
			}
			g_csPreviousFile = csCurrentFile;
			bSetStatus = true;
		}

		if(bSetStatus)
		{
			CString csFilePath;
			//Neha Gharge Short foem for all files 26/2/2015
			if (theApp.m_eScanLevel != WARDWIZSCANNER)
			{
				csFilePath = GetFileNameOnly(csStatus);
			}
			else
			{
				csFilePath = csStatus;
			}
			 
			if (PathFileExists(csFilePath))
			{
				//Issue no: 1151 Short name issue. if long file path it comes C:\...\xyz.txt format Neha Gharge
				//GetShortFilePath(csFilePath);
				SetScanStatus(csFilePath);
			}
			else
			{
				SetScanStatus(csStatus);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in void CScanDlg::ReInitialize", 0, 0, true, SECONDLEVEL);
	}
}


/**********************************************************************************************************                     
*  Function Name  :	GetVirusNameAndPath                                                     
*  Description    :	Retrives virus name , Virus file path
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::GetVirusNameAndPath(CString csStatus, CString &csVirusName, CString &csVirusPath)
{
	bool bReturn  = false;
	try
	{
		if(csStatus.GetLength() == 0)
			return bReturn;

		int iPivot = csStatus.ReverseFind(':');

		csVirusName = csStatus.Right((csStatus.GetLength() - iPivot) - 2);
		csVirusName = csVirusName.Left(csVirusName.GetLength() - 6);
		csVirusPath = csStatus.Left(iPivot);
		bReturn = true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetVirusNameAndPath", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	GetFileNameOnly                                                     
*  Description    :	Get file name on basis of [OK] text. 
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
CString CScanDlg::GetFileNameOnly(CString csInputPath)
{
	CString csReturn = L"";
	try
	{
		if(csInputPath.GetLength() == 0)
			return csReturn;

		if(!IsFullString(csInputPath))
		{
			return csReturn;
		}

		int iTempPos, iFirstPos;
		iFirstPos = iTempPos = (int)std::wstring((LPCTSTR)csInputPath).rfind(L"]");
		if(iTempPos > 0)
		{
			if( (csInputPath[ --iTempPos] == '[')  ||
				(csInputPath[ --iTempPos] == '[')  ||
				(csInputPath[ --iTempPos] == '[')  ||
				(csInputPath[ --iTempPos] == '[')  ||
				(csInputPath[ --iTempPos] == '[') )
			{
				iFirstPos = iTempPos;
				CString csFilePath = csInputPath.Left(iFirstPos - 2);
				if(PathFileExists(csFilePath))
				{
					csReturn = csFilePath;
				}
			}
		}
		else 
		{
			CString csTemp = csInputPath.Right(6);
			csTemp.Trim();
			if(csTemp == L"OK")
			{
				csReturn = csInputPath.Left(csInputPath.GetLength() - 8);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetFileNameOnly, Input Path: %s", csInputPath, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	ShowSpyFoundDlg                                                     
*  Description    :	Show spyware found dialog after detection of virus. 
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::ShowSpyFoundDlg(bool bCloseUI)
{
	//if(m_bScanningStopped)
	//{
	//	return true;
	//}

	bisLastScan =1;
	//LastScandatetime();

	int iLowRange = 0, iHighRange = 0;
	m_prgScanProgress.GetRange(iLowRange, iHighRange);
	//VARADA IKHAR*****ISSUE NO. 2, DATE: 24/12/2014*****************************************
	//m_prgScanProgress.SetPos(iHighRange);
	//m_ScanPercentage.SetWindowText(L"100%");
	iHighRange = iHighRange - iLowRange;
	//m_prgScanProgress.SetRange(iLowRange,iHighRange);
	//Issue No.5 In Wardwiz Basic/Essential, if full scan is aborted, the progress bar shows value more than 100%.
	//Gaurav Chopdar Date:8/1/2015
	//SetRange  supports for 16 bit, SetRange32 support 32 bit.

	if (iHighRange)
	{
		m_prgScanProgress.SetRange32(iLowRange, iHighRange);
	}

	//VARADA IKHAR**********************************************
	//Varada Ikhar, Date: 14/02/2015, Issue: Database needs to be updated.Database not valid.
	if(m_bSignatureFailed == true)
	{
		m_prgScanProgress.SetPos(iLowRange);
	}
	else if(!m_bScnAborted)
	{
		m_prgScanProgress.SetPos(iHighRange);
	}
	if(m_virusFound == 0 && m_eScanType == USBSCAN)
	{	
		//if this is usb scanning close the ui here once scanning is finished
		CISpyGUIDlg *pObjMainUI = reinterpret_cast<CISpyGUIDlg*>(this->GetParent());
		if (pObjMainUI!= NULL)
			pObjMainUI->CloseUISafely();
		return false;
	}

	if(m_virusFound == 0)
	{
		//TCHAR Datebuff[9];
		//TCHAR csDate[9];
		//_wstrdate_s(Datebuff,9);
		//_tcscpy(csDate,Datebuff);
		//AddEntriesInReportsDB(csDate, CTime::GetCurrentTime(), m_eScanType, L"NA", L"NA",L"No Threats Found");

		/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
		if(theApp.m_bEnableSound)
		{
			AfxBeginThread(PlayScanFinishedThread, NULL);
		}

		CString cstypeofscan = L"";
		switch(m_eScanType)
		{
			case QUICKSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_QUICK");
								break;
			case FULLSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_FULL");
								break;
			case CUSTOMSCAN:	cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM");
								break;
			default:			cstypeofscan = L"";
								break;
		}

		if(!bCloseUI)
		{
			CString csMessage;
//VARADA IKHAR*****ISSUE NO. 7, DATE: 23/12/2014***************************************************************************************
			//Varada Ikhar, Date: 14/02/2015, Issue: Database needs to be updated.Database not valid.
			if(m_bSignatureFailed == true)
				csMessage.Format(L"%s",theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DATABASE_MESSAGE"));
			else if(m_bScnAborted)
				csMessage.Format(L"%s %s",cstypeofscan,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_ABORTED"));
			else if(!m_bScnAborted)
				csMessage.Format(L"%s %s",cstypeofscan,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANNING_COMPLETED"));

//VARADA IKHAR**************************************************************************************************************************
			//add by lalit 5-7-2015,it use to highlight Task bar icon when scannin completed message box appeared. 
			::SetForegroundWindow(m_hWnd);

			m_bIsPopUpDisplayed = true;
			MessageBox(csMessage, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION |MB_TOPMOST );
			m_bIsPopUpDisplayed = false;

			ResetControlsValues();
			m_eScanType = m_eCurrentSelectedScanType;
		}
		//Varada Ikhar, Date: 14/02/2015, Issue: Database needs to be updated.Database not valid.
		m_bSignatureFailed = false;
		return true;
	}

	try
	{
		//m_btnBack.EnableWindow(FALSE);
		m_btnPauseResume.EnableWindow(FALSE);
		m_btnStop.ShowWindow(FALSE);
m_btnClean.ShowWindow(TRUE);
m_btnClean.EnableWindow(TRUE);


/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
if(theApp.m_bEnableSound)
{
	AfxBeginThread(PlayThreatsFoundThread, NULL);
}
//resolve by lalit 5-5-05
// issue -if any threat detect in scan and i click on another scan in-
//such case threat are detected Popup Appear which is not required when we change option from tab dlg.
if (g_TabCtrlWindHandle == NULL)
{
	g_TabCtrlWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();
}

if (g_TabCtrlWindHandle != NULL)
{
	if (!g_TabCtrlWindHandle->m_bIsTabMenuClicked)
	{
		CSpywareFoundDlg	dlg(this);
		dlg.m_iSpywareCount = m_virusFound;
		
		m_bIsPopUpDisplayed = true;
		dlg.DoModal();
		m_bIsPopUpDisplayed = false;
	}
}
else
{
	AddLogEntry(L"### Error in getting a handle of g_TabCtrlWindHandle in ShowSpyFoundDlg function.", 0, 0, true, FIRSTLEVEL);
}
g_TabCtrlWindHandle->m_bIsTabMenuClicked = false;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::ShowSpyFoundDlg", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	IsFullString
*  Description    :	Check for full string is completed or not by checking [OK] text.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::IsFullString(CString csInputPath)
{
	try
	{
		if(( (csInputPath.Find(L"[") != -1) && (csInputPath.Find(L"]") != -1)) ||
			(csInputPath.Find(L"OK") > 0))
		{
			return true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in void CScanDlg::IsFullString", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/**********************************************************************************************************
*  Function Name  :	QuarantineThread
*  Description    :	If user clicks on clean button.Quarantine thread gets called.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
DWORD WINAPI QuarantineThread(LPVOID lpParam)
{
	CScanDlg *pThis = (CScanDlg *) lpParam;
	pThis->m_btnClean.EnableWindow(FALSE);
	pThis->QuaratineEntries();
	//ISSUE NOT REPRORTED Rajil Yadav 11.06.2014
	pThis->m_btnPauseResume.ShowWindow(TRUE);
	pThis->m_btnPauseResume.EnableWindow(FALSE);
	return 1;
}

/**********************************************************************************************************
*  Function Name  :	QuaratineEntries
*  Description    :	Repaires or quarantines selected files one by one.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::QuaratineEntries()
{
	AddLogEntry(L"------------------------------------------------", 0, 0, true, SECONDLEVEL);
	AddLogEntry(L">>> Quarantine started..", 0, 0, true, SECONDLEVEL);
	m_bIsCleaning = true;
	SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_QUARANTINE_STARTED"));

	EnableDisableControlForClean(false);

	try
	{
		if (m_eCurrentSelectedScanType == QUICKSCAN)
		{
			m_bQuickScan = true;
		}
		else if (m_eCurrentSelectedScanType == FULLSCAN)
		{
			m_bFullScan = true;
		}
		else if (m_eCurrentSelectedScanType == CUSTOMSCAN)
		{
			m_bCustomscan = true;
		}
		m_bRedFlag = true;

		BOOL bCheck = FALSE;
		DWORD dwVirusCount = 0x00;
		DWORD dwCleanCount = 0;
		DWORD dwRebootRepair = 0;
		DWORD dwQuarantine = 0;
		CString csThreatName, csThreatPath, csStatus, csISpyID;

		TCHAR	Datebuff[9] = {0} ;
		TCHAR	csDate[9] = {0} ;
		BOOL bBackUp=0;

		_wstrdate_s(Datebuff,9);
		_tcscpy_s(csDate,Datebuff);

		if(!SendRecoverOperations2Service(RELOAD_DBENTRIES , L"", RECOVER, true, true))
		{
			AddLogEntry(L"### Error in CScanDlg::SendRecoverOperations2Service RELOAD_DBENTRIES RECOVER", 0, 0, true, SECONDLEVEL);
		}
		
		if(!SendRecoverOperations2Service(RELOAD_DBENTRIES, L"", REPORTS, true, true))
		{
			AddLogEntry(L"### Error in CScanDlg::SendFile4RepairUsingService RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
		}

		dwVirusCount = m_lstVirusesList.GetItemCount();

		//for(int i = 0; i < m_lstVirusesList.GetItemCount(); i++)
		for(DWORD i = 0; i < dwVirusCount; i++)
		{
			//issue no : 733 neha gharge 16/6/2014
			bCheck = m_lstVirusesList.GetCheck(i);
			csStatus = m_lstVirusesList.GetItemText(i, 2);
			//Issue 1317 : When no file found is status , then no need to consider for second cleaning 
			if (bCheck && (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED") || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE")))// csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND") 
			{
				csThreatName = m_lstVirusesList.GetItemText(i, 0);
				csThreatPath = m_lstVirusesList.GetItemText(i, 1);
				csISpyID     = m_lstVirusesList.GetItemText(i, 3);
			
				DWORD dwISpyID = 0;
				dwISpyID  = _wtol((LPCTSTR)csISpyID);
				if(dwISpyID >= 0)
				{
					CString csEntryState;
					//Issue no: 1151 Short name issue. if long file path it comes C:\...\xyz.txt format Neha Gharge
					//Neha Gharge Short form for all files 26/2/2015
					//GetShortFilePath(csThreatPath);
					csEntryState.Format(L"%s : %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_FILE"), csThreatPath);
					SetScanStatus(csEntryState);
					
					//csEntryState.Format(L"%s : %s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_FILE"),csThreatPath);
					//SetScanStatus(csEntryState);

					/*Added by Prajakta: To handle if file does not exist*/
					if(!PathFileExists(csThreatPath))
					{
						m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND"));
						AddLogEntry(L"### Error in CScanDlg::QuaratineEntries: File %s does not exist",csThreatPath, 0, true, SECONDLEVEL);
						continue;
						
					}
					//Sleep(5);

					//Modified by Vilas on 07 April 2015
					//Added to handle failure cases ( in use now )

					//bool CScanDlg::SendFile4RepairUsingService(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait, bool bReconnect)

					ISPY_PIPE_DATA szPipeData = { 0 };
					
					szPipeData.iMessageInfo = HANDLE_VIRUS_ENTRY;
					szPipeData.dwValue = dwISpyID;
					wcscpy_s(szPipeData.szFirstParam, csThreatPath);
					wcscpy_s(szPipeData.szSecondParam, csThreatName);
					// Third parameter need to be send which scan currently going on.
					//Issue 1190 report entry for this scans after USB scan is wrong
					if (m_eCurrentSelectedScanType == FULLSCAN)
					{
						wcscpy_s(szPipeData.szThirdParam, L"Full Scan");
					}
					if (m_eCurrentSelectedScanType == QUICKSCAN)
					{
						wcscpy_s(szPipeData.szThirdParam, L"Quick Scan");
					}
					if (m_eCurrentSelectedScanType == CUSTOMSCAN)
					{
						wcscpy_s(szPipeData.szThirdParam, L"Custom Scan");
					}

					//if(!SendFile4RepairUsingService(HANDLE_VIRUS_ENTRY, csThreatPath, csThreatName, dwISpyID, true, true))
					bool bSendReapir = SendFile4RepairUsingService(&szPipeData, true, true);

					switch (szPipeData.dwValue)
					{
						case 0x00:
							if (dwISpyID > 0)
							{
								m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
								m_lstVirusesList.SetCheck(i, FALSE);
							}
							else
							{
								m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
								m_lstVirusesList.SetCheck(i, FALSE);
							}

						break;

						case 0x04:
							//Issue: Issue with full scan in German UI. In Full scan in Action column the "Reboot Quarantine" status is not converted into German language.
							//Resolved by: Nitin K Date : 5th Jan 2016
							m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR"));
							m_lstVirusesList.SetCheck(i, FALSE);
							dwRebootRepair++;
							AddLogEntry(L"### Repair on Reboot File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;

						case 0x05:
							//Issue: Issue with full scan in German UI. In Full scan in Action column the "Reboot Quarantine" status is not converted into German language.
							//Resolved by: Nitin K Date : 5th Jan 2016
							m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE"));
							m_lstVirusesList.SetCheck(i, FALSE);
							dwQuarantine++;
							AddLogEntry(L"### quarantine File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
						break;
						case 0x08:
							m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_ALREADY_REPAIRED"));
							m_lstVirusesList.SetCheck(i, FALSE);
							AddLogEntry(L"### Already Repaired File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
							break;
						case 0x09:
							m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE"));
							m_lstVirusesList.SetCheck(i, FALSE);
							AddLogEntry(L"### Low disc take a backup of file File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
							break;
						default:
							CString csFailedValue;
							csFailedValue.Format(L"%d", szPipeData.dwValue);
							AddLogEntry(L"### Repair failed file::%s with Error ::%s", csThreatPath, csFailedValue,true,SECONDLEVEL);
							//Even though it error we show status as repair and quarantine. 
							//Count should be increase in that case
							//Just keeping a addlogentry to verify the problem.
							szPipeData.dwValue = 0x00;
							if (dwISpyID > 0)
							{
								m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"));
								m_lstVirusesList.SetCheck(i, FALSE);
							}
							else
							{
								m_lstVirusesList.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED"));
								m_lstVirusesList.SetCheck(i, FALSE);
							}
							
							AddLogEntry(L"### Repair failed File::%s, Virus Name::%s", csThreatPath, csThreatName, true, SECONDLEVEL);
					}

				/*	}
					else
					{
						if(dwISpyID > 0)
						{
							m_lstVirusesList.SetItemText(i, 2, FILEREPAIRED);
							m_lstVirusesList.SetCheck(i, FALSE);
						}
						else
						{
							m_lstVirusesList.SetItemText(i, 2, DELETEDSTATUS);
							m_lstVirusesList.SetCheck(i, FALSE);
						}
					}
					*/

					//Ram, Issue Resolved: 0001230
					if (szPipeData.dwValue == 0x00)
					{
						dwCleanCount++;
					}
				}
			}
		}
		m_bQuarantineFinished = true;
		m_bIsCleaning = false;


		//Adding for showing message as a "Your Computer is not Protected"
		if( dwVirusCount > 0x00 )
		{
			DWORD	dwbVirusFound = 0x01;

			if( dwCleanCount >= dwVirusCount )
				dwbVirusFound = 0x00;

			if(!SetRegistrykeyUsingService(L"SOFTWARE\\Wardwiz Antivirus", 
				L"VirusFound", REG_DWORD, dwbVirusFound, false))
			{
				AddLogEntry(L"### Error in Set SetRegistrykeyUsingService VirusFound in CScanDlg::QuaratineEntries", 0, 0, true, SECONDLEVEL);
			}
		}


		Sleep(5);
		//used 0 as Type for Saving RECOVER DB
		if(!SendFile4RepairUsingService(SAVE_RECOVERDB, L"", L"", 0, true, true))
		{
			AddLogEntry(L"### Error in CScanDlg::SendFile4RepairUsingService SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
		}

		Sleep(5);
		//used 0 as Type for Saving RECOVER DB
		if(!SendFile4RepairUsingService(SAVE_REPORTS_ENTRIES, L"", L"", 5, true, true))
		{
			AddLogEntry(L"### Error in CScanDlg::SendFile4RepairUsingService SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
		}

		m_bRedFlag = false;
		CString cstypeofscan = L"";
		switch(m_eScanType)
		{
			case QUICKSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_QUICK");
								break;
			case FULLSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_FULL");
								break;
			case CUSTOMSCAN:	cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM");
								break;
			default:			cstypeofscan = L"";
								break;
		}

		CString csTotalClean;
		csTotalClean.Format(L"%s %s,%s: %d",cstypeofscan,theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_COMPLETED"),theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_CLEAN_COUNT") ,dwCleanCount);
		SetScanStatus(csTotalClean);

		csTotalClean.Format(L"%s %s\n\n%s: %d",cstypeofscan,theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CLEANING_COMPLETED"),theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_CLEAN_COUNT"), dwCleanCount);
		
		m_bIsPopUpDisplayed = true;
		MessageBox(csTotalClean,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
		m_bIsPopUpDisplayed = false;

		EnableDisableControlForClean(true);

		CString csMsgToRebootSystem(L"");

		if(!ISAllItemsCleaned())
		{
			m_btnClean.EnableWindow(TRUE);

			
			csMsgToRebootSystem.Format(L"%s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_MSG_ONCLEAN_PART1"), theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_MSG_ONCLEAN_PART2"));

			//Check for failed delete ini if entry found give a message to restart
			//added for Repair reboot on 07 April 2015 by Vilas
			if (dwRebootRepair || dwQuarantine)
			{
				m_bIsPopUpDisplayed = true;
				if (MessageBox(csMsgToRebootSystem, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
				{
					m_bIsPopUpDisplayed = false;
					//Write a code to restart computer.
					CEnumProcess enumproc;
					enumproc.RebootSystem(0);
				}
				m_bIsPopUpDisplayed = false;
			}
			else
			{

				DWORD dwTotalRebootCount = 0x00;

				dwTotalRebootCount = CheckForDeleteFileINIEntries() + CheckForRepairFileINIEntries();
				if (dwTotalRebootCount)
				{
					csMsgToRebootSystem.Format(L"%s %d %s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART1"), dwTotalRebootCount, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART2"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));

					m_bIsPopUpDisplayed = true;
					if (MessageBox(csMsgToRebootSystem, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
					{
						m_bIsPopUpDisplayed = false;
						//Write a code to restart computer.
						CEnumProcess enumproc;
						enumproc.RebootSystem(0);
					}
					m_bIsPopUpDisplayed = false;
				}
			}
		}
		else
		{
			if(!SetRegistrykeyUsingService(L"SOFTWARE\\Wardwiz Antivirus",
				L"VirusFound", REG_DWORD, 0x00, false))
			{
				AddLogEntry(L"### Error in Set SetRegistrykeyUsingService VirusFound in CScanDlg::QuaratineEntries", 0, 0, true, SECONDLEVEL);
			}

			//Added for reset the list control
			//m_virusFound = 0;
			//Issue: Virus found count doesnt show on UI in case of fewer no of files
			//Resolved By: Nitin K Date : 15th May 2015
			m_virusFound = 0;
			/**************************ISSUE No: 259 Neha Gharge 22/5/14 *********************************/
			ResetControlsValues();
			//Issue No.15 After virus found If I clean selected entries, It cleans it, But If I select the entries from remaining entries I am not able to clean it, Clean button is disabled
			//Gaurav Chopdar Date:5/1/2015
			//m_virusFound = 0;
			m_btnClean.EnableWindow(false);

			csMsgToRebootSystem.Format(L"%s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_MSG_ONCLEAN_PART1"), theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_MSG_ONCLEAN_PART2"));

			//Check for failed delete ini if entry found give a message to restart
			//added for Repair reboot on 07 April 2015 by Vilas
			if (dwRebootRepair || dwQuarantine)
			{
				m_bIsPopUpDisplayed = true;
				if (MessageBox(csMsgToRebootSystem, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
				{
					m_bIsPopUpDisplayed = false;
					//Write a code to restart computer.
					CEnumProcess enumproc;
					enumproc.RebootSystem(0);
				}
				m_bIsPopUpDisplayed = false;
			}
			else
			{

				DWORD dwTotalRebootCount = 0x00;

				dwTotalRebootCount = CheckForDeleteFileINIEntries() + CheckForRepairFileINIEntries();
				if (dwTotalRebootCount)
				{
					csMsgToRebootSystem.Format(L"%s %d %s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART1"), dwTotalRebootCount, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART2"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));
					
					m_bIsPopUpDisplayed = true;
					if (MessageBox(csMsgToRebootSystem, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
					{
						m_bIsPopUpDisplayed = false;

						//Write a code to restart computer.
						CEnumProcess enumproc;
						enumproc.RebootSystem(0);
					}
					m_bIsPopUpDisplayed = false;
				}
			}
		}
		//Issue no 1133 Neha Gharge
			if(m_bQuickScan)
			{
				m_bQuickScan = false;
				if(g_TabCtrlWindHandle != NULL)
				{
					if(g_TabCtrlWindHandle->m_btnCustomScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnCustomScan->EnableWindow(true);
					}
					if(g_TabCtrlWindHandle->m_btnFullScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnFullScan->EnableWindow(true);
					}
					if(g_TabCtrlWindHandle->m_btnAntirootkitScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnAntirootkitScan->EnableWindow(true);
					}
				}

			}
			if(m_bFullScan)
			{
				m_bFullScan = false;
				if(g_TabCtrlWindHandle != NULL)
				{
					if(g_TabCtrlWindHandle->m_btnQuickScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnQuickScan->EnableWindow(true);
					}
					if(g_TabCtrlWindHandle->m_btnCustomScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnCustomScan->EnableWindow(true);
					}
					if(g_TabCtrlWindHandle->m_btnAntirootkitScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnAntirootkitScan->EnableWindow(true);
					}
				}
			}

			if(m_bCustomscan)
			{
				m_bCustomscan = false;
				if(g_TabCtrlWindHandle != NULL)
				{
					if(g_TabCtrlWindHandle->m_btnQuickScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnQuickScan->EnableWindow(true);
					}
					if(g_TabCtrlWindHandle->m_btnFullScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnFullScan->EnableWindow(true);
					}
					if(g_TabCtrlWindHandle->m_btnAntirootkitScan != NULL)
					{
						g_TabCtrlWindHandle->m_btnAntirootkitScan->EnableWindow(true);
					}
				}
			}
			//if(g_TabCtrlWindHandle != NULL)
			//{
			//	g_TabCtrlWindHandle->m_pTabDialog->EnableAllBtn();
			//}
		
		//ISSUE NOT REPRORTED Rajil Yadav 11.06.2014
		//Issue No.15 Gaurav chopdar Date :5/1/2015 Commented following lines for Issue No.15 
		//m_virusFound = 0;
		//m_btnClean.EnableWindow(false);
	
		//Resolved Issue No. 30 While Cleaning Threats, Other scan options should be disabled.
		//-Niranjan Deshak. 25/12/2014
		if(g_TabCtrlWindHandle !=NULL)
		{
			//g_TabCtrlWindHandle->m_pTabDialog->EnableAllExceptSelected();
		}
		if(m_hQuarantineThread!= NULL)
		{
			//Varada Ikhar, Date: 28/04/2015
			//Issue : While quarantine entries is in progress, if user clicks on close, quarantine process should be paused.
			if (TerminateThread(m_hQuarantineThread, 0) == FALSE)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to Terminate QuarantineThread in CScanDlg::CloseCleaning with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Terminated QuarantineEntries thread successfully.", 0, 0, true, FIRSTLEVEL);
			m_hQuarantineThread = NULL;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::QuaratineEntries", 0, 0, true, SECONDLEVEL);
	}
	
	AddLogEntry(L">>> Quarantine Finished..", 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"------------------------------------------------", 0, 0, true, SECONDLEVEL);
}

/**********************************************************************************************************                     
*  Function Name  :	GetQuarantineFolderPath                                                     
*  Description    :	Get quarantine folder(i.e backup folder) path.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
CString CScanDlg::GetQuarantineFolderPath()
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

/**********************************************************************************************************                     
*  Function Name  :	QuarantineFile                                                     
*  Description    :	Clean file
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::QuarantineFile(CString csFilePath)
{
	bool bReturn = false;
	try
	{
		SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
		bReturn = ::DeleteFile(csFilePath) != 0;
		if(!bReturn)
		{
			CEnumProcess objEnumProcess;
			if(objEnumProcess.IsProcessRunning(csFilePath, true))
			{
				AddLogEntry(L">>> Killing running Process: %s", csFilePath, 0, true, FIRSTLEVEL);			
				::Sleep(1000);
				SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
				bReturn = ::DeleteFile(csFilePath) != 0;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::QuarantineFile", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	GetFileName                                                     
*  Description    :	Get file name from given path of file.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::GetFileName(CString csFilePath, CString &csFileName)
{
	try
	{
		if(csFilePath.GetLength() == 0)
		{
			return false;
		}
		int iFind = csFilePath.ReverseFind(L'\\');
		if(iFind != -1)
		{
			csFileName = csFilePath.Mid(iFind + 1, csFilePath.GetLength());
			return true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetFileName", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	ShutDownScanning                                                     
*  Description    :	Shut down scanning with terminating all thread safely.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::ShutDownScanning(bool bCloseUI)
{
	if (!isScanning) { return true; }

	try
	{
		//ISSue No 390  Neha gharge 23/5/2014*********************************************/
		CString cstypeofscan = L"";
		switch(m_eScanType)
		{
			case QUICKSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_QUICK");
								break;
			case FULLSCAN:		cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_FULL");
								break;
			case CUSTOMSCAN:	cstypeofscan = theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM");
								break;
			default:			cstypeofscan = L"";
								break;
		}
		
		//Varada Ikhar , Date: 23/04/2015
		// Issue : 000033 : 1.In all scans, start scanning 2. click close button in UI then popup displays 'Do you want to stop rootkit scan' 3. Dont take any action
		//4. Rootkit scanning completed message appears 5. Then click yes on stop confirmation dialog box 6. UI is getting closed by leaving succesful dialog box on the desktop.
		if (!(m_virusFound > 0 && m_bClose == true))
		{
			if (!PauseScan())
			{
				AddLogEntry(L"### Failed to pause scan.", 0, 0, true, SECONDLEVEL);
			}
		}

		CString csTemp;
		CString csStr = theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCAN_IN_PROGESS_FIRST");
		csTemp.Format(L"%s %s\0\r\n",cstypeofscan,csStr);
		csTemp.Append(L"\n");
		csTemp.Append(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCAN_IN_PROGESS_SECOND"));
		
		m_bIsPopUpDisplayed = true;
		int iReturn = MessageBox(csTemp, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		m_bIsPopUpDisplayed = false;

		if(iReturn == IDNO)
		{
			//Varada Ikhar , Date: 23/04/2015
			// Issue : 000033 : 1.In all scans, start scanning 2. click close button in UI then popup displays 'Do you want to stop rootkit scan' 3. Dont take any action
			//4. Rootkit scanning completed message appears 5. Then click yes on stop confirmation dialog box 6. UI is getting closed by leaving succesful dialog box on the desktop.
			if (!ResumeScan())
			{
				AddLogEntry(L"### Failed to resume scan.", 0, 0, true, SECONDLEVEL);
			}
			return false;
		}
		//Varada Ikhar, Date:18-04-2015,
		//Issue : 0000131 & 0000129 : 1. Start scanning in Quick scan 2. Go to dataencryption & click on 'Browse' 3. Quick scan completed dialog box appears. 
		//4. Keep it aside & cancel browsed explorer. 5. Go to quick scan again & click on 'scan' as it is still enabled. Then click 'OK' on that dialog box. 
		//6. After completion of 100% no pop up appears & when clicked on stop it is showing 'Quick scanning is in progress. Do you want to stop'. click yes no change occurs in progress bar & the status below. 
		//7. In reports it is showing as 'No threats found' 
		else
		{
			m_btnPauseResume.EnableWindow(false);
			m_btnPauseResume.ShowWindow(true);
			m_btnScan.EnableWindow(false);
			m_btnScan.ShowWindow(false);
		}
		//Neha Gharge 3/3/2015 
		//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
		if (m_bClose == true && iReturn == IDYES)
		{
			theApp.m_bOnCloseFromMainUI = true;
		}
		
		if (isScanning == TRUE)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = STOP_SCAN;

			CISpyCommunicator objCom(SERVICE_SERVER, true);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CScanDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
				return false;
			}
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CScanDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		if(m_hThreadVirEntries != NULL)
		{
			ResumeThread(m_hThreadVirEntries);
			TerminateThread(m_hThreadVirEntries, 0);
			m_hThreadVirEntries = NULL;
		}

		if(m_hThreadStatusEntry != NULL)
		{
			ResumeThread(m_hThreadStatusEntry);
			TerminateThread(m_hThreadStatusEntry, 0);
			m_hThreadStatusEntry = NULL;
		}

		OutputDebugString(L">>> m_hThreadStatusEntry stopped");

		m_bScnAborted = true;
		//Varada Ikhar, Date:18-04-2015,
		//Issue : 0000131 & 0000129 : 1. Start scanning in Quick scan 2. Go to dataencryption & click on 'Browse' 3. Quick scan completed dialog box appears. 
		//4. Keep it aside & cancel browsed explorer. 5. Go to quick scan again & click on 'scan' as it is still enabled. Then click 'OK' on that dialog box. 
		//6. After completion of 100% no pop up appears & when clicked on stop it is showing 'Quick scanning is in progress. Do you want to stop'. click yes no change occurs in progress bar & the status below. 
		//7. In reports it is showing as 'No threats found' 
		if (m_bFlagScanFinished)
		{
			ReInitializeVariables();
		}
		
		//Set here the progress bar with no text
		if (theApp.m_eScanLevel == CLAMSCANNER)
		{
			m_prgScanProgress.SetWindowText(L"");
		}

	   // resolved by lalit 1-8-2015 .issue  after scanner stop ui not getting scanning aborted message and all scanning option disable.
		// below line commented due to above issue
		//m_bScanningStopped = true;
		
		isScanning = FALSE;

		EnableAllWindows(TRUE);

		//CTimeSpan tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsScanStartTime) - m_tsScanPauseResumeElapsedTime;
		//CTimeSpan tsScanElapsedTime = m_tsScanEndTime - m_tsScanStartTime;
		CString csTime =  m_tsScanElapsedTime.Format(_T("%H:%M:%S"));

		AddLogEntry(L"-------------------------------------------------------------");
		CString csCompleteScanning;

		//Issue		: Scanning Aborted message Added
		//Resolved	: Vilas
		//Date		: 28 - Nov - 2014
		/* Issue No: 13 In quick/full/custom scan incomplete message after scanning aborted.
		Resolved by : Nitin K Date: 19-Dec-2014
		*/
		//csCompleteScanning.Format(L"%s, %s : %d, %s:%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_ABORTED"),theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_FOUND"),m_virusFound,theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_SCANTIME"), csTime);
		//SetScanStatus(csCompleteScanning);
		//AddLogEntry(csCompleteScanning);
		AddLogEntry(L"-------------------------------------------------------------");

		KillTimer(TIMER_SCAN_STATUS);
		KillTimer(TIMER_FILESSCANNED_STATUS);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonStop                                                     
*  Description    :	Handler after clicking stop button.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnBnClickedButtonStop()
{
	// lalit 4-28-2015 , if we close on stop button and same time exit from tray then two popup comes , now single popup will come
	g_TabCtrlWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();
	if (g_TabCtrlWindHandle != NULL)
	{
		g_TabCtrlWindHandle->m_bIsCloseHandleCalled = true;
		//m_bClose = true;
		if (!ShutDownScanning())
		{
			g_TabCtrlWindHandle->m_bisUIcloseRquestFromTray = false;
		}
		g_TabCtrlWindHandle->m_bIsCloseHandleCalled = false;
		if (g_TabCtrlWindHandle->m_bisUIcloseRquestFromTray)
		{
			g_TabCtrlWindHandle->HandleCloseButton();
		}
			
	}
	
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonPause                                                     
*  Description    :	Pause scanning.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnBnClickedButtonPause()
{
	try
	{
		//Varada Ikhar, Date: 13/05/2015
		//Issue : Double click scan button,scan will start and elapsed time will be 0. 4.Progress bar will be showing 0 % and once pop - up come it will show 100 %.
		if (isScanning == false)
			return;

		CString csButtonText;
		m_btnPauseResume.GetWindowText(csButtonText);

		if(csButtonText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"))
		{
			//if (!isScanning) return ;

			//if (pi.hProcess != INVALID_HANDLE_VALUE)
			//	ResumeThread(pi.hThread);

			if(SendRequestCommon(RESUME_SCAN))
			{
				SetTimer(TIMER_SCAN_STATUS, 1000, NULL);
				SetTimer(TIMER_FILESSCANNED_STATUS, 800, NULL);
				
				//SetTimer(TIMER_FILEPATH_STATUS, 0, NULL);
				m_tsScanPauseResumeElapsedTime += ((CTime::GetCurrentTime() - m_tsScanPauseResumeTime));
				SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_RESUME"));
				m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
				AddLogEntry(L">>> Scanning Resumed..", 0, 0, true, FIRSTLEVEL);
			}
		}
		else if(csButtonText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"))
		{
			//if (!isScanning) 
			//{
			//	return ;
			//}
			//if (pi.hProcess != INVALID_HANDLE_VALUE)
			//	SuspendThread(pi.hThread);

			if(SendRequestCommon(PAUSE_SCAN))
			{
				//Kill the timer here
				OnTimer(TIMER_PAUSE_HANDLER);

				m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"));
				SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_PAUSE"));
				AddLogEntry(L">>> Scanning Paused..", 0, 0, true, FIRSTLEVEL);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::OnBnClickedButtonPause", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnTimer                                                     
*  Description    :	The framework calls this member function after each interval specified in the SetTimer 
					member function used to install a timer.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnTimer(UINT_PTR nIDEvent)
{
	try
	{
		TCHAR	szTemp[256] = { 0 };

		if (nIDEvent == TIMER_SCAN_STATUS)
		{
			//On WM_CLOSE call,UI will not get repaint after resuming scan as TIMER_SCAN_STATUS gets killed on pause.
			//Neha Gharge 13/5/2015
			if (m_bOnWMClose == false)
			{
				m_tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsScanStartTime) - m_tsScanPauseResumeElapsedTime;
				//CString csTime =  tsScanElapsedTime.Format(_T("Elapsed Time:%H:%M:%S"));
				CString csTime = m_tsScanElapsedTime.Format(_T("%H:%M:%S"));
				CString csElapsedTime;
				csElapsedTime.Format(L"%s%s", m_csStaticElapsedTime , csTime);
				m_stElapsedTime.SetWindowText(csElapsedTime);
			}

			//Ram: Memory scan implementation
			//Check here if memory scan is running show percentage of memory scan
			if (m_bIsMemScan)
			{
				if (m_ScanCount && m_iMemScanTotalFileCount)
				{
					int		iPercentage = int(((float)(m_FileScanned) / m_iMemScanTotalFileCount) * 100);
					if (theApp.m_eScanLevel == CLAMSCANNER)
					{
						m_prgScanProgress.SetWindowText(L"(" + theApp.m_objwardwizLangManager.GetString(L"IDS_SCANNING_MEMORY") + L") ");
					}
					m_prgScanProgress.SetPos(m_FileScanned);
					m_prgScanProgress.RedrawWindow();
					wsprintf(szTemp, TEXT("%d%%"), iPercentage);
				}
				
			}
			else
			{
				if (m_ScanCount && m_iTotalFileCount)
				{
					int		iPercentage = int(((float)(m_FileScanned) / m_iTotalFileCount) * 100);
					m_prgScanProgress.SetPos(m_FileScanned);
					m_prgScanProgress.RedrawWindow();
					wsprintf(szTemp, TEXT("%d%%"), iPercentage);
				}
			}

		}
		else if (nIDEvent == TIMER_FILESSCANNED_STATUS)
		{
			CString csFileScanned, csVirusFound;
			//csFileScanned.Format(L"Files Scanned:   %d", m_FileScanned);
			//csVirusFound.Format(L"Threats Found:  %d", m_virusFound)
			csFileScanned.Format(L"%s%d", m_csStaticFilesScanned, m_FileScanned);
			csVirusFound.Format(L"%s%d", m_csStaticThreatsFound, m_virusFound);

			m_csFilesScanned.SetWindowText(csFileScanned);
			m_stThreatsFound.SetWindowText(csVirusFound);
		}
		else if (nIDEvent == TIMER_PAUSE_HANDLER)
		{
			m_tsScanPauseResumeTime = CTime::GetCurrentTime();
			KillTimer(TIMER_SCAN_STATUS);
		}
		CJpegDialog::OnTimer(nIDEvent);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::OnTimer", 0, 0, true, SECONDLEVEL);
	}
}


/**********************************************************************************************************                     
*  Function Name  :	ReInitializeVariables                                                     
*  Description    :	Reinitializtion of variables after scanning 
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::ReInitializeVariables()
{
	//set flag scan not in start status
	m_bScanStarted = false;
	m_bScanningStopped = false;
	//Neha Gharge 13/5/2015
	m_bOnWMClose = false;
	m_csaAllScanPaths.RemoveAll();
	//m_btnBack.EnableWindow(TRUE);
	//m_btnFullScn.EnableWindow(TRUE);
//	m_btnCustomScn.EnableWindow(TRUE);
//	m_btnQuickScn.EnableWindow(TRUE);
	

	if(m_eScanType == CUSTOMSCAN)
	{
		m_EdtInputPath.SetWindowText(L"");
		//m_btnBrowse.EnableWindow(TRUE);
		//m_btninnerCutomScan.EnableWindow(TRUE);
		//m_btninnerCutomScan.ShowWindow(SW_SHOW);
		//Design implementation of Custom scan 
		//Added by: Nitin K. 28th March 2015
		CRect rect1;
		this->GetClientRect(rect1);
		m_btnStop.EnableWindow(FALSE);
		//Issue:0000223 pause button is visible in case of custom scan completed &  Select All text not showing after custom scan. Resolved By: Nitin K Date:24th April 2015
		//Varada Ikhar, Date: 02/05/2015
		//Issue: If UI is closed on click of 'close' button, the pause-resume button does not gets dispalyed.
		if (m_virusFound == 0 && m_bClose == false)
		{
			m_stSelectAll.SetWindowPos(&wndTop, rect1.left + 27, 375, 120, 17, SWP_SHOWWINDOW);
		//Issue: Select All text not showing on Wix XP
		//Resolved By : Nitin K. Date: 20th May 2015
		m_stSelectAll.RedrawWindow();
		m_btnPauseResume.ShowWindow(SW_HIDE);
		}
	//	m_stSelectAll.SetWindowPos(&wndTop, rect1.left + 27, 220 + 64, 120, 17, SWP_NOREDRAW);
		//m_btnStop.ShowWindow(TRUE);
		
	}
	else if(m_eScanType == USBDETECT || m_eScanType == USBSCAN)
	{
		m_lstDrivePicker.EnableWindow(TRUE);
		m_btnScan.EnableWindow(TRUE);
		m_btnScan.ShowWindow(TRUE);
	}
	else
	{
		//Ticket no .0000229 and 0000249 When quick scan/Full scan is in progress and we close UI directly pause button disappears.
		//Just interchanges the statement 1st showwindow and then  Enabled.
		//after threats detected scan button gets enabled again.
		//Neha Gharge 27th April,2015
		// resolved by lalit 5-5-2015, if full scan is running and clicked on custom scan and abort previous,now double-2 scan button appear. 
		if (m_virusFound == 0 && (!m_bClose) && m_eCurrentSelectedScanType!= CUSTOMSCAN)
		{
			m_btnScan.ShowWindow(TRUE);
			m_btnScan.EnableWindow(TRUE);
		}
	}
	//Invalidate();
}

/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the 
					mouse causes cursor movement within the CWnd object.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
BOOL CScanDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BUTTON_PAUSE			||
		iCtrlID == IDC_BACK_BUTTON			||
		iCtrlID == IDC_BUTTON_STOP			||
		iCtrlID == IDC_BUTTON_START_SCAN	||
		//iCtrlID == IDC_BTN_QUICKSCAN		||
		/*iCtrlID == IDC_BTN_FULLSCAN			||
		iCtrlID == IDC_BTN_CUSTOMSCAN		||*/
		iCtrlID == IDC_BUTTON_SPYCLEAN		||
		iCtrlID == IDC_BUTTON_BROWSEPATH	||
		iCtrlID == IDC_BUTTON_CUSTOMSCAN	||
		iCtrlID == IDC_BUTTON_CUSTOM_ADD	||
		iCtrlID == IDC_BUTTON_CUSTOM_DELETE	||
		iCtrlID == IDC_BUTTON_CUSTOM_EDIT	||
		iCtrlID == IDC_BUTTON_CLEAN)
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if(csClassName == _T("Button") && m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}
	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonClean                                                     
*  Description    :	Handler to clean selected entries from check box.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnBnClickedButtonClean()
{
	//issue no : 733 neha gharge 16/6/2014
	bool iSelected = false;
	for(int i = 0; i < m_lstVirusesList.GetItemCount(); i++)
	{
		CString  csStatus = m_lstVirusesList.GetItemText(i, 2);
		if(m_lstVirusesList.GetCheck(i))
		{
			//Issue 1317 : When no file found is status , then no need to consider for second cleaning 
			if (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED") || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_FAILED") || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE"))//csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND") || 
			{
				iSelected = true;
				break;
			}
			else if (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED") || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED"))
			{
				m_lstVirusesList.SetCheck(i, FALSE);
			}
		}
	}

	if(!iSelected)
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NOTSELECTEDFORENTRIES_CLEAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION|MB_OK);
		m_bIsPopUpDisplayed = false;
		return;
	}

	//Check here for Evaluation
	if(!theApp.m_bAllowDemoEdition)
	{
		//Extra check has been added to get number of days left.
		//Ram, Issue No: 0001216, Resolved
		if (theApp.m_dwDaysLeft == 0)
		{
			theApp.GetDaysLeft();
		}

		if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				return;
			}
		}
	 }
		//Resolved Issue No. 30 While Cleaning Threats, Other scan options should be disabled.
		//-Niranjan Deshak. 25/12/2014. 
		if(g_TabCtrlWindHandle !=NULL)
		{
			// lalit  4-27-2015 , due to mutuall operation implementaion no more need to hide other tab control when any one is in-progress 
			//g_TabCtrlWindHandle->m_pTabDialog->DisableAllExceptSelected();
		}
	
		m_hQuarantineThread = ::CreateThread(NULL, 0, QuarantineThread, (LPVOID) this, 0, NULL);
	
}


/**********************************************************************************************************                     
*  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that contains the cursor
					(or the CWnd object that used the SetCapture member function to capture the mouse input)
					every time the mouse is moved.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
LRESULT CScanDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnQuickscan                                                     
*  Description    :	Shows control of quick scan when user click on quick scan from tab control
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::OnBnClickedBtnQuickscan()
{
	//Rajil Issue :-651 09/06/2014.
	// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
	//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
	if (m_eCurrentSelectedScanType == QUICKSCAN)
	{
		return true;
	}
	if (isScanning) return false;

	/*	ISSUE NO - 617 NAME - NITIN K. TIME - 10th June 2014 */
	ReadUISettingFromRegistry();


	if((m_eScanType == FULLSCAN || m_eScanType == CUSTOMSCAN) && (m_virusFound > 0) ) 
	{
		/*	ISSUE NO - 844 NAME - NITIN K. TIME - 27th June 2014 */
		if(m_eScanType == FULLSCAN)
		{
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SCAN_MESSAGE_MOVE_QUICK_FULL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_YESNO | MB_ICONQUESTION) == IDNO)
			{
				m_bIsPopUpDisplayed = false;
				return false;
			}
			m_bIsPopUpDisplayed = false;
		}
		if(m_eScanType == CUSTOMSCAN)
		{
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SCAN_MESSAGE_MOVE_QUICK_CUSTOM"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_YESNO | MB_ICONQUESTION) == IDNO)
			{	
				m_bIsPopUpDisplayed = false;
				return false;
			}
			//Issue No : 0001189 : Right click on any encrypted file when scanning is in progress then no popup is appearing.
			m_bIsPopUpDisplayed = false;
		}
	}

	//m_eScanType = QUICKSCAN;	
	m_eCurrentSelectedScanType = QUICKSCAN;

	CRect rect1;
	this->GetClientRect(rect1);
	m_stHeaderAll.SetBitmap(m_bmpQuickScan);
	m_virusFound = 0;
	
	//Varada Ikhar, Date:26/02/2015, Issue:The UI look for space between header and Listview should be standard.
	m_lstVirusesList.SetWindowPos(&wndTop, rect1.left + 6, 68, 586, 207, SWP_SHOWWINDOW);
	m_lstVirusesList.DeleteAllItems();
	m_chkSelectAll.SetCheck(TRUE);

	ResetControlsValues();
	RefreshStrings();
	m_stFullScanHeaderName.ShowWindow(SW_HIDE);
	m_stFullScanHeaderDes.ShowWindow(SW_HIDE);

	m_stCustomHeaderDes.ShowWindow(SW_HIDE);
	m_stCustomScnHeaderName.ShowWindow(SW_HIDE);

	m_stQuickHeaderDescription.ShowWindow(SW_SHOW);
	m_stQuickHeaderName.ShowWindow(SW_SHOW);

	m_EdtInputPath.ShowWindow(FALSE);
	m_btnBrowse.ShowWindow(FALSE);
	m_btninnerCutomScan.ShowWindow(FALSE);
	m_btnScan.ShowWindow(TRUE);
	m_btnScan.EnableWindow(TRUE);
	m_btnPauseResume.ShowWindow(FALSE);
	m_lstDrivePicker.ShowWindow(FALSE);
	m_stHeaderRemovable.ShowWindow(FALSE);
//Issue No. 642 Rajil Yadav 10_6_2014
	m_btnScan.ShowWindow(TRUE);
	m_btnScan.EnableWindow(TRUE);
	m_btnStop.ShowWindow(TRUE);
	m_btnStop.EnableWindow(FALSE);
	m_btnClean.ShowWindow(FALSE);
	
	m_csaAllScanPaths.RemoveAll();
	SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_QUICK_SCANNING_STATUS"));
	//Added by : Nitin K 18th March
	m_stDragDropFiles.EnableWindow(FALSE);
	m_stDragDropFiles.ShowWindow(SW_HIDE);
	m_stCustomDragDropText.ShowWindow(SW_HIDE);
	m_btnCutomAdd.ShowWindow(SW_HIDE);
	m_btnCustomDelete.ShowWindow(SW_HIDE);
	m_btnCustomEdit.ShowWindow(SW_HIDE);
	m_cbCustomSelectAll.ShowWindow(SW_HIDE);
	m_stSelectAll.SetWindowPos(&wndTop, rect1.left + 27, 220 + 64, 120, 17, SWP_NOREDRAW);
	return true;
	//SetScanStatus(L"Quick Scan: Please click scan button to start scanning");
}

// Rajil Yadav Issue No. 503. 6.6.2014
/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnFullscan                                                     
*  Description    :	Shows control of full scan when user click on full scan from tab control
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::OnBnClickedBtnFullscan()
{
	//Rajil Issue :-651 09/06/2014.
	// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
	//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
	if (m_eCurrentSelectedScanType == FULLSCAN)
	{
		return true;
	}
	if (isScanning) return false;

	/*	ISSUE NO - 617 NAME - NITIN K. TIME - 10th June 2014 */
	ReadUISettingFromRegistry();


	if((m_eScanType == CUSTOMSCAN || m_eScanType == QUICKSCAN) && (m_virusFound > 0) ) 
	{
		/*	ISSUE NO - 844 NAME - NITIN K. TIME - 27th June 2014 */
		if(m_eScanType == CUSTOMSCAN)
		{
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SCAN_MESSAGE_MOVE_FULL_CUSTOM"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_YESNO | MB_ICONQUESTION) == IDNO)
			{
				m_bIsPopUpDisplayed = false;
				return false;
			}
			//Issue No : 0001189 : Right click on any encrypted file when scanning is in progress then no popup is appearing.
			m_bIsPopUpDisplayed = false;
		}
		if(m_eScanType == QUICKSCAN)
		{
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SCAN_MESSAGE_MOVE_FULL_QUICK"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_YESNO | MB_ICONQUESTION) == IDNO)
			{
				m_bIsPopUpDisplayed = false;
				return false;
			}
			//Issue No : 0001189 : Right click on any encrypted file when scanning is in progress then no popup is appearing.
			m_bIsPopUpDisplayed = false;
		}
	}
	
	// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
	//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
	//m_eScanType = FULLSCAN;
	m_eCurrentSelectedScanType = FULLSCAN;
	CRect rect1;
	this->GetClientRect(rect1);
	m_virusFound = 0;
	//m_stHeaderAll.SetBitmap(m_bmpQuickScan);
	
	//Varada Ikhar, Date:26/02/2015, Issue:The UI look for space between header and Listview should be standard.
	m_lstVirusesList.SetWindowPos(&wndTop, rect1.left + 6, 68, 586, 207, SWP_SHOWWINDOW);
	m_lstVirusesList.DeleteAllItems();
	m_chkSelectAll.SetCheck(TRUE);

	m_stFullScanHeaderName.ShowWindow(SW_SHOW);
	m_stFullScanHeaderDes.ShowWindow(SW_SHOW);

	m_stQuickHeaderDescription.ShowWindow(SW_HIDE);
	m_stQuickHeaderName.ShowWindow(SW_HIDE);

	m_stCustomHeaderDes.ShowWindow(SW_HIDE);
	m_stCustomScnHeaderName.ShowWindow(SW_HIDE);
	
	ResetControlsValues();
	RefreshStrings();


	m_EdtInputPath.ShowWindow(FALSE);
	m_btnBrowse.ShowWindow(FALSE);
	m_btninnerCutomScan.ShowWindow(FALSE);
//Issue No. 642 Rajil Yadav 10_6_2014
	m_btnScan.ShowWindow(TRUE);
	m_btnScan.EnableWindow(TRUE);
	m_btnStop.ShowWindow(TRUE);
	m_btnStop.EnableWindow(FALSE);
	m_btnClean.ShowWindow(FALSE);

	m_btnPauseResume.ShowWindow(FALSE);
	m_lstDrivePicker.ShowWindow(FALSE);
	m_stHeaderRemovable.ShowWindow(FALSE);
	SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FULL_STATUS"));

	//Added by : Nitin K 18th March
	m_stDragDropFiles.EnableWindow(FALSE);
	m_stDragDropFiles.ShowWindow(SW_HIDE);
	m_stCustomDragDropText.ShowWindow(SW_HIDE);
	m_btnCutomAdd.ShowWindow(SW_HIDE);
	m_btnCustomDelete.ShowWindow(SW_HIDE);
	m_btnCustomEdit.ShowWindow(SW_HIDE);
	m_cbCustomSelectAll.ShowWindow(SW_HIDE);
	m_stSelectAll.SetWindowPos(&wndTop, rect1.left + 27, 220 + 64, 120, 17, SWP_NOREDRAW);
	return true;
}
// Rajil Yadav Issue No. 503. 6.6.2014
/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnCustomscan                                                     
*  Description    :	Shows control of custom scan when user click on custom scan from tab control
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::OnBnClickedBtnCustomscan()
{
	//Rajil Issue :-651 09/06/2014.
	// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
	//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
	if (m_eCurrentSelectedScanType == CUSTOMSCAN)
	{
		return true;
	}

	/*	ISSUE NO - 617 NAME - NITIN K. TIME - 10th June 2014 */
	ReadUISettingFromRegistry();

	if (isScanning)
		return false;
	
	if((m_eScanType == FULLSCAN || m_eScanType == QUICKSCAN) && (m_virusFound > 0) ) 
	{
		/*	ISSUE NO - 844 NAME - NITIN K. TIME - 27th June 2014 */
		if(m_eScanType == QUICKSCAN)
		{
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SCAN_MESSAGE_MOVE_CUSTOM_QUICK"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_YESNO | MB_ICONQUESTION) == IDNO)
			{
				m_bIsPopUpDisplayed = false;
				return false;
			}
			//Issue No : 0001189 : Right click on any encrypted file when scanning is in progress then no popup is appearing.
			m_bIsPopUpDisplayed = false;
		}
		if(m_eScanType == FULLSCAN)
		{
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SCAN_MESSAGE_MOVE_CUSTOM_FULL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_YESNO | MB_ICONQUESTION) == IDNO)
			{	
				m_bIsPopUpDisplayed = false;
				return false;
			}
			//Issue No : 0001189 : Right click on any encrypted file when scanning is in progress then no popup is appearing.
			m_bIsPopUpDisplayed = false;
		}
	}
	
	// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
	//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
	//m_eScanType = CUSTOMSCAN;
	m_eCurrentSelectedScanType = CUSTOMSCAN;
	CRect rect1;
	this->GetClientRect(rect1);
	m_stHeaderAll.SetBitmap(m_bmpQuickScan);
	//Design implementation of Custom scan 
	//Added by: Nitin K. 28th March 2015
	//m_lstVirusesList.SetWindowPos(&wndTop, rect1.left + 6, 98,586 ,177, SWP_SHOWWINDOW);
	m_lstVirusesList.SetWindowPos(&wndTop, rect1.left + 6, 68, 586, 207, SWP_NOREDRAW);
	m_lstVirusesList.DeleteAllItems();
	m_virusFound = 0;
	m_chkSelectAll.SetCheck(TRUE);
	//Nitin K 18th March
	
	m_stSelectAll.SetWindowPos(&wndTop, rect1.left + 27, 375, 120, 17, SWP_NOREDRAW);


	m_stDragDropFiles.EnableWindow(TRUE);
	m_stDragDropFiles.ShowWindow(SW_SHOW);
	m_stCustomDragDropText.ShowWindow(SW_SHOW);

	m_stFullScanHeaderName.ShowWindow(SW_HIDE);
	m_stFullScanHeaderDes.ShowWindow(SW_HIDE);

	m_stQuickHeaderDescription.ShowWindow(SW_HIDE);
	m_stQuickHeaderName.ShowWindow(SW_HIDE);

	m_stCustomHeaderDes.ShowWindow(SW_SHOW);
	m_stCustomScnHeaderName.ShowWindow(SW_SHOW);
	
	ResetControlsValues();
	RefreshStrings();
	//Issue No. 642 Rajil Yadav 10_6_2014
	m_btnScan.ShowWindow(FALSE);
	//m_EdtInputPath.ShowWindow(TRUE);
	m_EdtInputPath.SetWindowText(L"");
	//m_btnBrowse.ShowWindow(TRUE);
	m_btninnerCutomScan.ShowWindow(TRUE);
	//m_btnPauseResume.ShowWindow(TRUE);
	//m_btnStop.ShowWindow(TRUE);
	m_btnStop.EnableWindow(FALSE);
	m_btnClean.ShowWindow(FALSE);
	m_lstDrivePicker.ShowWindow(TRUE);
	//m_lstDrivePicker.DeleteAllItems();
	
	//m_lstDrivePicker.DeleteAllItems();
	m_btnCutomAdd.ShowWindow(TRUE);
	m_btnCustomDelete.ShowWindow(TRUE);
	m_btnCustomEdit.ShowWindow(TRUE);
	m_cbCustomSelectAll.ShowWindow(TRUE);
	if(!m_bScanStarted)
	{
		m_btnBrowse.EnableWindow(TRUE);
		m_btninnerCutomScan.EnableWindow(TRUE);
	}
	SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOM_SCAN_STATUS"));
	return true;
}
// Rajil Yadav Issue No. 503. 6.6.2014
/**********************************************************************************************************                     
*  Function Name  :	GetAllDrivesList                                                     
*  Description    :	Makes list of drives present on a system.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::GetAllDrivesList(CStringArray &csaReturn)
{
	csaReturn.RemoveAll();
	bool bReturn = false;
	CString csDrive;
	int iCount = 0;
	
	for(char chDrive = 'A'; chDrive <= 'Z'; chDrive++)
	{
		csDrive.Format(L"%c:", chDrive);
		if(PathFileExists(csDrive))
		{
			csaReturn.Add(csDrive);
			bReturn = true;
		}
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonStartScan                                                     
*  Description    :	Handler when clicks on quick scan and full scan start button.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnBnClickedButtonStartScan()
{
	try
	{
		//In Quick scan/Full scan->click 'scan'->click 'stop'->click 'Yes' in stop confirmation dialogbox->
		//still the pause button is enabled->if clicked on 'pause'->Quick scan completed dialogbox appearing & but scan still continues.
		//Niranjan Deshak - 14/03/2015.
		// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
		//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
		m_bScnAborted = false;
		if (!m_bScnAborted)
		{
			StartScanning();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Error in Setting Registry CScanDlg::OnBnClickedButtonStartScan", 0, 0, true, SECONDLEVEL);
	}

}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonCustomscan                                                     
*  Description    :	Handler when clicks on custom scan start button.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnBnClickedButtonCustomscan()
{	
	//m_btnScan.EnableWindow(FALSE);
	//issue no : 730 neha gharge 16/6/2014
	//issue no 34 Neha Gharge 19-12-2014
	int iNetworkFilePath = 0;
	m_iTotalNoofFiles = 0;
	theApp.isScanning = true;
	//Issue: Virus found count doesnt show on UI in case of fewer no of files
	//Resolved By: Nitin K Date : 15th May 2015
	g_csPreviousVirusFoundPath = L"";
	// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
	//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
	m_eScanType = m_eCurrentSelectedScanType;
	m_bScnAborted = false;

	if (m_eScanType == CUSTOMSCAN)
	{
		if (m_lstDrivePicker.GetItemCount() == 0)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_SELECT_FILE_FOLDER_FOR_CUSTOM_SCAN"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
		}
		CString csScanPath;
		m_iTotalNoofFiles = 0;
		//Issue: Check box Enabled for Custom Scan List box Resolved By: Nitin K Date:24th April 2015
		int iIsSelected = 0;
		for (int i = 0; i < m_lstDrivePicker.GetItemCount(); i++)
		{
			if (m_lstDrivePicker.GetCheck(i) == BST_CHECKED)
			{
				iIsSelected++;
				csScanPath = m_lstDrivePicker.GetItemText(i, 0);
				if (PathFileExists(csScanPath))
				{
					if (PathIsDirectory(csScanPath))
					{
						//It is directory
						EnumFolderToCheckEmptyFolder(csScanPath);
						if (m_iTotalNoofFiles > 0)
						{
							m_csaAllScanPaths.Add(csScanPath);
							continue;
						}
					}
					else
					{
						//It is File
						if (PathIsNetworkPath(csScanPath))
						{
							iNetworkFilePath++;
							continue;
						}

						else
						{
							m_csaAllScanPaths.Add(csScanPath);
							m_iTotalNoofFiles++;
						}
					}
				}
			}
			//EnumFolderToCheckEmptyFolder(csScanPath);
			//m_csaAllScanPaths.Add(csScanPath);
		}
		//Issue: Check box Enabled for Custom Scan List box Resolved By: Nitin K Date:24th April 2015
		if (iIsSelected == 0)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_VALID_SELECT_FOR_SCAN"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
		}
		if (m_iTotalNoofFiles == 0)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_SELECTED_EMPTY_FOLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_bIsPopUpDisplayed = false;
			
			//m_lstDrivePicker.DeleteAllItems();
			return;
		}
		AddDeleteEditCustomScanEntryToINI();
		StartScanning();
		ShowControlsAfterCustomScanStarted(TRUE);
		//Design implementation of Custom scan 
		//Added by: Nitin K. 28th March 2015
		CRect rect1;
		this->GetClientRect(rect1);
		m_stSelectAll.SetWindowPos(&wndTop, rect1.left + 27, 220 + 64, 120, 17, SWP_NOREDRAW);
	}
	//{
	//	CString cScanPath;
	//	/*m_EdtInputPath.GetWindowText(cScanPath);
	//	if(PathIsNetworkPath(cScanPath))
	//	{
	//		MessageBox(L"Selected path is network path.\nPlease select local folders or files.",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
	//		return;
	//	}*/
	//	m_csaAllScanPaths.RemoveAll();
	//	m_iTotalNoofFiles = 0;
	//	for (int i = 0; i < m_lstDrivePicker.GetItemCount(); i++)
	//	{
	//		cScanPath = m_lstDrivePicker.GetItemText(i,0);
	//		EnumFolderToCheckEmptyFolder(cScanPath);
	//		m_csaAllScanPaths.Add(cScanPath);
	//	}

	//	/*if(cScanPath.GetLength() != 0)
	//	{
	//		size_t iLen = wcslen(cScanPath);
	//		if(iLen > 0)
	//		{
	//			CString csTemp;
	//			csTemp = cScanPath;
	//			if(csTemp.Right(1) == L"\\")
	//			{
	//				csTemp = csTemp.Left(iLen -1);
	//			}
	//			m_EdtInputPath.SetWindowTextW(csTemp);
	//			m_csaAllScanPaths.RemoveAll();
	//			m_csaAllScanPaths.Add(csTemp);
	//		}
	//	}*/
	//	//if(m_csaAllScanPaths.GetCount() == 0)
	//	//{
	//	//	//MessageBox(L"Please browse folder to scan", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
	//	//	MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CUSTOMSCAN_BROWSE_FOLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK); //VARADA IKHAR*****PUNCTUATION ISSUE*****DATE:29/12/2014
	//	//	return;
	//	//}
	//	//CString csBrowsePath = m_csaAllScanPaths.GetAt(0);
	//	//if(PathIsNetworkPath(csBrowsePath))
	//	{
	//		//same way like other file in the system.
	//		m_btninnerCutomScan.EnableWindow(false);
	//		m_btnBrowse.EnableWindow(FALSE);
	//		StartScanning();
	//		ShowControlsAfterCustomScanStarted(TRUE);
	//	}
	//	else 
	//	{
	//		//Issue			: UI not getting refreshed if in Custom scan given folder is Empty
	//		//Resolved by	: Nitin K
	//		//Date			: 03 Jan - 2015
	//		//m_iTotalNoofFiles = 0;
	//		//EnumFolderToCheckEmptyFolder(cScanPath);
	//		if(!PathFileExists(csBrowsePath))
	//		{
	//			CString csMsgForNofileforScan = L"";
	//			//csMsgForNofileforScan.Format(L"%s %s",m_csaAllScanPaths.GetAt(0),L"path does not exist to scan");
	//			csMsgForNofileforScan.Format(L"%s %s",m_csaAllScanPaths.GetAt(0),theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CUSTOMSCAN_PATH_NOT_FOUND")); //VARADA IKHAR*****PUNCTUATION ISSUE*****DATE:29/12/2014
	//			MessageBox(csMsgForNofileforScan,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
	//			m_EdtInputPath.SetWindowTextW(L"");
	//			return;
	//		}
	//		//Issue			: UI not getting refreshed if in Custom scan given folder is Empty
	//		//Resolved by	: Nitin K
	//		//Date			: 03 Jan - 2015
	//		else if(!m_iTotalNoofFiles)
	//		{
	//			CString csMsgForNofileforScan = L"";
	//			csMsgForNofileforScan.Format(L"%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CUSTOMSCAN_FOLDER_IS_EMPTY")); 
	//			MessageBox(csMsgForNofileforScan,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
	//			m_EdtInputPath.SetWindowTextW(L"");
	//			ResetControlsValues();
	//			return;
	//		}
	//		else
	//		{
	//			m_btninnerCutomScan.EnableWindow(false);
	//			m_btnBrowse.EnableWindow(FALSE);
	//			StartScanning();
	//			ShowControlsAfterCustomScanStarted(TRUE);
	//		}
	//	}

	//}

}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonBrowsepath                                                     
*  Description    :	Browse folder path for custom scanning.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnBnClickedButtonBrowsepath()
{

	CSelectDialog ofd(TRUE, _T("*.*"), NULL,
		OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_NONETWORKBUTTON,
		_T("All files and folders(*.*)|*.*||"));

	m_bIsPopUpDisplayed = true;
	if (ofd.DoModal() != IDOK)
	{
		m_bIsPopUpDisplayed = false;
		return;
	}
	m_bIsPopUpDisplayed = false;

	for (int i = 0; i < ofd.m_SelectedItemList.GetCount(); i++)
	{
		LVFINDINFO lvInfo;
		lvInfo.flags = LVFI_STRING;
		CString csFileName = NULL;
		CString csScanPath = NULL;
		int iCListEntriesLength = 0;
		csFileName = ofd.m_SelectedItemList[i];
		lvInfo.psz = csFileName;
		if (m_lstDrivePicker.FindItem(&lvInfo, -1) != -1)
			continue;
		if (PathIsNetworkPath(csFileName))
			continue;
		
		for (int jIndex = 0; jIndex < m_lstDrivePicker.GetItemCount(); jIndex++)
		{
			csScanPath = m_lstDrivePicker.GetItemText(jIndex, 0);
			iCListEntriesLength += csScanPath.GetLength();
		}

		if ((1000 - iCListEntriesLength) > csFileName.GetLength())
		{
			m_lstDrivePicker.InsertItem(0, ofd.m_SelectedItemList[i]);
		}


	}
	//TCHAR *pszPath = new TCHAR[MAX_PATH];
	//SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));

	//CString csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_TO_SELECT_FOLDER_SCAN");
	//BROWSEINFO        bi = {m_hWnd, NULL, pszPath, csMessage, BIF_RETURNONLYFSDIRS, NULL, 0L, 0};
	//
	//LPITEMIDLIST      pIdl = NULL;

	//LPITEMIDLIST  pRoot = NULL;
	////SHGetFolderLocation(m_hWnd, CSIDL_DRIVES, 0, 0, &pRoot);
	//bi.pidlRoot = pRoot;
	//pIdl = SHBrowseForFolder(&bi);
	//if(NULL != pIdl)
	//{
	//	SHGetPathFromIDList(pIdl, pszPath);
	//	size_t iLen = wcslen(pszPath);
	//	if(iLen > 0)
	//	{
	//		CString csTemp;
	//		csTemp = pszPath;
	//		if(csTemp.Right(1) == L"\\")
	//		{
	//			csTemp = csTemp.Left(iLen -1);
	//		}
	//		m_EdtInputPath.SetWindowTextW(csTemp);
	//		m_csaAllScanPaths.RemoveAll();
	//		m_csaAllScanPaths.Add(csTemp);
	//	}
	//}
	//delete [] pszPath;
	//pszPath = NULL;
}

/**********************************************************************************************************                     
*  Function Name  :	SetScanPath                                                     
*  Description    :	Adds scan path in array and shows on edit controls.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::SetScanPath(CString csScanPath)
{
	m_csaAllScanPaths.RemoveAll();
	m_csaAllScanPaths.Add(csScanPath);
	m_EdtInputPath.SetWindowTextW(csScanPath);
}

/**********************************************************************************************************                     
*  Function Name  :	GetScanningPaths                                                     
*  Description    :	Get scan path according to scanning types.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::GetScanningPaths(CStringArray &csaReturn)
{
	CString cScanPath;
	/*if(csaReturn.GetCount() > 0)
	{
		csaReturn.RemoveAll();
	}*/

	switch(m_eScanType)
	{
	case QUICKSCAN:
		m_bQuickScan = true;
		csaReturn.RemoveAll();
		csaReturn.Add(L"QUICKSCAN");

		break;
	case FULLSCAN:
		m_bFullScan = true;
		csaReturn.RemoveAll();
		if(!GetAllDrivesList(csaReturn))
		{
			return false;
		}
		break;
	case CUSTOMSCAN:
		{
			m_bCustomscan = true;
			//Nitin K. Custom Scan
			/*CString cScanPath;
			m_EdtInputPath.GetWindowText(cScanPath);
			if(cScanPath.GetLength() == 0)
			{
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_TO_SELECT_PATHFOR_SCAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				m_btnBrowse.EnableWindow(TRUE);
				m_btninnerCutomScan.EnableWindow(TRUE);
				return false;
			}
			//Extra check is need to get exact path for scanner parameters.
			if(cScanPath[cScanPath.GetLength() - 1] == L'\\' && cScanPath[cScanPath.GetLength() - 2] != L'\\')
			{
				cScanPath = cScanPath.Mid(0, cScanPath.GetLength() - 1);
			}
			csaReturn.Add(cScanPath);
			cScanPath.ReleaseBuffer();
			for (int i = 0; i < m_lstDrivePicker.GetItemCount(); i++)
		{
			cScanPath = m_lstDrivePicker.GetItemText(i,0);
			m_csaAllScanPaths.Add(cScanPath);
		}
			*/
			/*for (int i = 0; i < m_lstDrivePicker.GetItemCount(); i++)
			{
				cScanPath = m_lstDrivePicker.GetItemText(i, 0);
				csaReturn.Add(cScanPath);
			}*/
		}
		break;
	case USBSCAN:
	case USBDETECT:
		return OnGetSelection();
		break;
	}
	if(csaReturn.GetCount() > 0)
	{
		return true;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	OnGetSelection                                                     
*  Description    :	Get name from seleced drives.(Not in used)
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::OnGetSelection() 
{
	TCHAR szSelection [27];
	//m_lstDrivePicker.GetSelectedDrives ( szSelection );

	if(_tcslen(szSelection) == 0)
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_TO_SELECT_DRIVES_SCAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		m_bIsPopUpDisplayed = false;

		return false;
	}

	m_csaAllScanPaths.RemoveAll();
	for(int i = 0; i < static_cast<int>(_tcslen(szSelection)); i++)
	{
		CString csDriveName;
		csDriveName.Format(L"%c:",szSelection[i]);
		if(PathFileExists(csDriveName))
		{
			m_csaAllScanPaths.Add(csDriveName);
		}
		else
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_FILES_FOR_SCAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_bIsPopUpDisplayed = false;
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SelectDriveParameter                                                     
*  Description    :	Select drive parameter.(Not in used)
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::SelectDriveParameter(LPCTSTR cs)
{
	CString csDriveSingleLetter;
	csDriveSingleLetter.Format(L"%c", cs[0]);
	//m_lstDrivePicker.SetSelection(csDriveSingleLetter);
}

/**********************************************************************************************************                     
*  Function Name  :	SetControls4USBDetect                                                     
*  Description    :	Show control of USB detect (not in used)
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::SetControls4USBDetect()
{
	CRect rect1;
	this->GetClientRect(rect1);
	m_stHeaderAll.SetBitmap(m_bmpUsbScan);
	m_lstVirusesList.SetWindowPos(&wndTop, rect1.left + 150, 68, 568 , 180, SWP_SHOWWINDOW);	
	m_chkSelectAll.SetCheck(TRUE);
	m_lstDrivePicker.SetWindowPos(&wndTop,rect1.left + 21, 68, 120, 286,SWP_SHOWWINDOW);

	int nFlags = 0;
	nFlags |= DDS_DLIL_HARDDRIVES;
	nFlags |= DDS_DLIL_CDROMS;
	nFlags |= DDS_DLIL_NETDRIVES;
	nFlags |= DDS_DLIL_REMOVABLES;
	nFlags |= DDS_DLIL_RAMDRIVES;

	//m_lstDrivePicker.DeleteAllItems();
	//m_lstDrivePicker.InitList (16, nFlags );
	//m_lstDrivePicker.UnCheckAll();
	
	SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USBSCAN_STATUS"));
	if(m_csaAllScanPaths.GetCount() > 0)
	{
		SelectDriveParameter(m_csaAllScanPaths.GetAt(0));
	}

	ResetControlsValues();

	m_EdtInputPath.ShowWindow(false);
	m_btnBrowse.ShowWindow(false);
	m_btninnerCutomScan.ShowWindow(true);
	m_btnPauseResume.ShowWindow(true);
	m_btnScan.ShowWindow(TRUE);
	m_btnScan.EnableWindow(TRUE);
	m_btninnerCutomScan.EnableWindow(FALSE);
	m_btnBrowse.EnableWindow(FALSE);
	m_btninnerCutomScan.ShowWindow(false);
	m_lstDrivePicker.ShowWindow(SW_SHOW);
	m_stHeaderRemovable.ShowWindow(SW_SHOW);
//	m_btnQuickScn.ShowWindow(false);
//	m_btnFullScn.ShowWindow(false);
//	m_btnCustomScn.ShowWindow(false);
}


/**********************************************************************************************************                     
*  Function Name  :	GetModuleCount                                                     
*  Description    :	Get a count of processes.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::GetModuleCount( )
{
	DWORD	dwPID[0x100] = {0} ;
	DWORD	dwBytesRet = 0x00, i=0x00 ;
	
	EnumProcesses(dwPID, 0x400, &dwBytesRet ) ;
	dwBytesRet = dwBytesRet/sizeof(DWORD) ;

	m_iTotalFileCount = 0;
	for( i=0; i<dwBytesRet; i++ )
	{
		HMODULE		hMods[1024] = {0} ;
		HANDLE		hProcess = NULL ;

		DWORD		dwModules = 0x00 ;

		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
								FALSE, dwPID[i] ) ;
		if( !hProcess )
			continue ;


		EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules) ;

		dwModules = dwModules / sizeof(HMODULE ) ;

		m_iTotalFileCount += dwModules ;
		CloseHandle( hProcess ) ;
	}

	if( m_iTotalFileCount )
	{
		m_prgScanProgress.ShowWindow( SW_SHOW ) ;
		//m_ScanPercentage.ShowWindow( SW_SHOW ) ;
		m_prgScanProgress.SetRange32( 0, m_iTotalFileCount ) ;

		m_ScanCount = true ;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	ResetControlsValues                                                     
*  Description    :	Reset all controls before and after scanning.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::ResetControlsValues()
{
	//m_virusFound = 0;	
	m_FileScanned = 0;
	m_dwTotalScanPathCount = 1;
	m_bScanStarted = false;
	// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
	//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
	//m_bScnAborted = false;
	m_bScanningStopped = false;
	m_bClose = false;
	m_bHome = false;
	m_bRedFlag = false;
	
	/*
		ISSUE NO - 44 Two dialogs appears after full scan.
		NAME - Niranjan Deshak. - 12th Jan 2015.
	*/
	m_bFlagScanFinished = true;

	CString csElapsedtime,csFileScanned,csThreatFound;
	csElapsedtime.Format(L"%s%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ELAPSEDTIME"),L"00:00:00");
	m_stElapsedTime.SetWindowText(csElapsedtime);  

	csFileScanned.Format(L"%s%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED"),L"0");
	m_csFilesScanned.SetWindowText(csFileScanned);

	csThreatFound.Format(L"%s%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TREATFOUND"),L"0");
	m_stThreatsFound.SetWindowText(csThreatFound);
	m_prgScanProgress.SetPos(0);
	//m_ScanPercentage.SetWindowText(L"0%");
	//Issue - 28/05/2014.,
	if(m_virusFound > 0)
	{
		m_btnStop.ShowWindow(FALSE);
		//Varada Ikhar, Date:18-04-2015,
		//Issue : 0000131 & 0000129 : 1. Start scanning in Quick scan 2. Go to dataencryption & click on 'Browse' 3. Quick scan completed dialog box appears. 
		//4. Keep it aside & cancel browsed explorer. 5. Go to quick scan again & click on 'scan' as it is still enabled. Then click 'OK' on that dialog box. 
		//6. After completion of 100% no pop up appears & when clicked on stop it is showing 'Quick scanning is in progress. Do you want to stop'. click yes no change occurs in progress bar & the status below. 
		//7. In reports it is showing as 'No threats found' 
		//m_btnScan.EnableWindow(true);
		m_btnPauseResume.ShowWindow(SW_SHOW);
		m_btnPauseResume.EnableWindow(false);
		m_btnStop.EnableWindow(false);
		m_btnStop.ShowWindow(SW_HIDE);
		m_btnClean.EnableWindow(true);
		m_btnClean.ShowWindow(SW_SHOW);
		Invalidate();
	}
	else
	{
		//m_bQuickScan = false;
		//m_bFullScan = false;
		//m_bCustomscan = false;
		m_lstVirusesList.DeleteAllItems();
	}

	// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
	//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
	//if(m_eScanType == QUICKSCAN)
	if (m_eCurrentSelectedScanType == QUICKSCAN)
	{
		HideControlForCustomScan(TRUE);
		SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_QUICK_SCANNING_STATUS"));
	}
	//if (m_eScanType == FULLSCAN)
	if (m_eCurrentSelectedScanType == FULLSCAN)
	{
		HideControlForCustomScan(TRUE);
		SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FULL_STATUS "));
		
	}
//	if (m_eScanType == CUSTOMSCAN)
	if (m_eCurrentSelectedScanType == CUSTOMSCAN)
	{	//Issue - 28/05/2014.,
		HideControlForCustomScan(FALSE);
		m_btnStop.EnableWindow(false);
		m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
		SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOM_SCAN_STATUS "));
		m_lstDrivePicker.ShowWindow(TRUE);
		//m_lstDrivePicker.DeleteAllItems();
		m_stDragDropFiles.ShowWindow(TRUE);
		m_stCustomDragDropText.ShowWindow(SW_SHOW);
		m_btnCutomAdd.ShowWindow(TRUE); 
		m_btnCustomEdit.ShowWindow(TRUE);
		m_btnCustomDelete.ShowWindow(TRUE);
		m_cbCustomSelectAll.ShowWindow(TRUE);
		m_btninnerCutomScan.ShowWindow(TRUE);
		m_btnClean.ShowWindow(SW_HIDE);
		//Issue Not reported: Select all text is not visible in case of custom scan is completed
		CRect rect1;
		this->GetClientRect(rect1);
		m_stSelectAll.SetWindowPos(&wndTop, rect1.left + 27, 375, 120, 17, SWP_NOREDRAW);
		m_stSelectAll.ShowWindow(SW_SHOW);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	SetReportDate                                                     
*  Description    :	Set a report date into registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::SetReportDate()
{
	HKEY hKey;
	LPCTSTR sk = TEXT("SOFTWARE\\Wardwiz Antivirus");

	LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sk, 0, KEY_ALL_ACCESS , &hKey);

	if (openRes==ERROR_SUCCESS)
	{
		//AddLogEntry(L"Success opening key.");
	}
	else
	{
		//AddLogEntry(L"Error opening key.");
		//long CreateReg = RegCreateKeyEx(HKEY_LOCAL_MACHINE,sk,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,(LPDWORD)REG_CREATED_NEW_KEY);
		//if(CreateReg==ERROR_SUCCESS)		
		//{
			//AddLogEntry(L"Success created key.");
		//}
		//else
		//{
			//AddLogEntry(L"UnSuccess created key.");
		//}
	}
	
	CString  csDate;
	SYSTEMTIME  CurrTime = {0} ;
	GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
	CTime Time_Curr( CurrTime ) ;
	int iMonth = Time_Curr.GetMonth();
	int iDate = Time_Curr.GetDay();
	int iYear = Time_Curr.GetYear();

	csDate.Format(L"%d/%d/%d",iMonth,iDate,iYear);
	TCHAR Datebuff[9];
	_wstrdate_s(Datebuff,9);

	LPCTSTR SetReportDate = TEXT("SetReportDate");
	LPCTSTR dwData = csDate;
	DWORD dwDataLen = 1024;
	LONG setRes = RegSetValueEx (hKey ,SetReportDate, 0, REG_SZ, (LPBYTE)dwData,dwDataLen);
	if (setRes == ERROR_SUCCESS)
	{
		//AddLogEntry(L"Success writing to Registry.");
	}
	else if(setRes == ERROR_MORE_DATA)
	{
		//AddLogEntry(L"Error writing to Registry.");
	}

}

/**********************************************************************************************************                     
*  Function Name  :	AddEntriesInReportsDB                                                     
*  Description    :	Adds a scanned action entry with time and date into DB files
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::AddEntriesInReportsDB(CString csDate,CTime ctDateTime, SCANTYPE eScanType, CString csThreatName, CString csFilePath, CString csAction)
{
	TCHAR csScanType[0x30];
	switch(eScanType)
	{
		case FULLSCAN:
			_tcscpy_s(csScanType, L"Full Scan");
			break;
		case CUSTOMSCAN:
			_tcscpy_s(csScanType, L"Custom Scan");
			break;
		case QUICKSCAN:
			_tcscpy_s(csScanType, L"Quick Scan");
			break;
		case USBSCAN:
		case USBDETECT:
			_tcscpy_s(csScanType, L"USB Scan");
			break;

	}

	CString csTime =  ctDateTime.Format(_T("%H:%M:%S"));

	OutputDebugString(L">>> AddinReports " + csFilePath);

	//m_objReportsDB.bWithoutEdit=0;
	//m_iContactVersion = 2;
	//m_objReportsDB.SetFileVersion(m_iContactVersion);
	//CString csDateTime = L"";
	//csDateTime.Format(_T("%s %s"),csDate,csTime);
	//CIspyList newEntry(csDateTime,csScanType,csThreatName,csFilePath,csAction);
	//m_objReportsDB.AddContact(newEntry, true);
}


//bool CScanDlg::SaveInReportsDB()
//{
//	OutputDebugString(L">>> In SaveInReportsDB");
//
//	FILE *pFile = NULL;
//	static CString csFilePath;
//	TCHAR szModulePath[MAX_PATH] = {0};
//	if(!GetModulePath(szModulePath, MAX_PATH))
//	{
//		MessageBox(L"Error in GetModulePath", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
//		return false;
//	}
//	csFilePath = szModulePath;
//	csFilePath += _T("\\WRDWIZREPORTS.DB");
//		
//	m_bSuccess=0;
//	CFile wFile(csFilePath, CFile::modeCreate | CFile::modeWrite);
// 
//	// Create a storing archive
//	CArchive arStore(&wFile,CArchive::store);
//	m_objReportsDB.Serialize(arStore);
//	
//	// Close the storing archive
//	arStore.Close();
//	wFile.Close();
// 
//	if(!m_bSuccess)
//	{
//		return true;
//	}
//	return false;
//}

/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage                                                     
*  Description    :	Translate window messages before they are dispatched to the TranslateMessage 
					and DispatchMessage Windows functions
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
BOOL CScanDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/**********************************************************************************************************                     
*  Function Name  :	OnNMCustomdrawListViruslist                                                     
*  Description    :	Customize list control at the time of inserting any entry into list controls.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnNMCustomdrawListViruslist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

	// Take the default processing unless we set this to something else below.
	*pResult = 0;

	switch(pLVCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		break;

	case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
		{
			if(pLVCD->iSubItem == 2)
			{
				CString csStatus = m_lstVirusesList.GetItemText(static_cast<int>(pLVCD->nmcd.dwItemSpec), 2) ;
				if (theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED") == csStatus || theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_FAILED") == csStatus || theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND") == csStatus || theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE") == csStatus)
				{
					pLVCD->clrText = RGB(255, 0, 0);
				}
				else if (theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED") == csStatus 
					|| theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED") == csStatus 
					|| theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE") == csStatus 
					|| theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR") == csStatus
					|| theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_ALREADY_REPAIRED") == csStatus)
				{
					pLVCD->clrText = RGB(171,238,0);
				}
			}
		}
		break;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	ISAllItemsCleaned                                                     
*  Description    :	Checks whether all selected entries are cleaned or not.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::ISAllItemsCleaned()
{
	bool bReturn = true;
	//Issue: in custom scan if No file found at time of virus clean then on UI shows status in Virus list control
	//resolved by Nitin K. Date: 13th May 2015
	DWORD dwFileNotFoundCount = 0;
	DWORD dwDetectedCount = 0;
	for (int i = 0; i < m_lstVirusesList.GetItemCount(); i++)
	{
		CString csStatus = m_lstVirusesList.GetItemText(i, 2);
		if (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED") ||
			csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_FAILED")
			)
		{
			bReturn = false;
			dwDetectedCount++;
			//break;
		}
		//Issue 1317 : When no file found is status , then no need to consider for second cleaning 
		if (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE"))//csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND") || 
		{
			dwFileNotFoundCount++;
			//break;
		}
	}
	if (dwFileNotFoundCount > 0)
	{
		if (dwDetectedCount > 0)
		{
			bReturn = false;
		}
		else
		{
			bReturn = false;
			m_virusFound = 0;
		}
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckSelectall                                                     
*  Description    :	Checks whether all entries are selected or not.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnBnClickedCheckSelectall()
{
	int iCheck = m_chkSelectAll.GetCheck();

	for(int i = 0; i < m_lstVirusesList.GetItemCount(); i++)
	{
		m_lstVirusesList.SetCheck(i, iCheck);
	}
}

//bool CScanDlg::LoadRequiredModules()
//{
//	m_hModuleISpyScanDLL = LoadLibrary(L"WRDWIZSCANDLL.DLL");
//	if(!m_hModuleISpyScanDLL)
//	{
//		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAIL_LOAD_WRDWIZSCANDLL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
//		return false;
//	}
//	
//	m_lpLoadSigProc = (LOADSIGNATURE)GetProcAddress(m_hModuleISpyScanDLL, "LoadSignatures");
//	if(!m_lpLoadSigProc)
//	{
//		FreeLibrary(m_hModuleISpyScanDLL);
//		m_lpLoadSigProc = NULL;
//		m_hModuleISpyScanDLL = NULL;
//		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAIL_LOAD_SIGNATURE_WRDWIZSCANDLL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
//		return false;
//	}
//
//	m_lpUnLoadSigProc = (UNLOADSIGNATURES)GetProcAddress(m_hModuleISpyScanDLL, "UnLoadSignatures");
//	if(!m_lpUnLoadSigProc)
//	{
//		FreeLibrary(m_hModuleISpyScanDLL);
//		m_hModuleISpyScanDLL = NULL;
//		m_lpUnLoadSigProc = NULL;
//		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAIL_LOAD_UNSIGNATURE_WRDWIZSCANDLL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
//		return false;
//	}
//
//	m_lpScanFileProc = (SCANFILE)GetProcAddress(m_hModuleISpyScanDLL, "ScanFile");
//	if(!m_lpScanFileProc)
//	{
//		FreeLibrary(m_hModuleISpyScanDLL);
//		m_hModuleISpyScanDLL = NULL;
//		m_lpScanFileProc = NULL;
//		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANFILE_FAILED_LOAD"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
//		return false;
//	}
//
//	m_hModuleISpyRepairDLL = LoadLibrary(L"WRDWIZREPAIRDLL.DLL");
//	if(!m_hModuleISpyRepairDLL)
//	{
//		AfxMessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED_LOAD_WRDWIZREPAIRDLL"));
//		return false;
//	}
//
//	m_lpRepairFileProc = (REPAIRFILE)GetProcAddress(m_hModuleISpyRepairDLL, "RepairFile");
//	if(!m_lpRepairFileProc)
//	{
//		AfxMessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED_REAIRFILE_FUNC"));
//		FreeLibrary(m_hModuleISpyRepairDLL);
//		return false;
//	}
//	return true;
//}

//bool CScanDlg::LoadSignatures(LPTSTR lpFilePath)
//{
//	if(!m_lpLoadSigProc)
//	{
//		return false;
//	}
//	return m_lpLoadSigProc(lpFilePath);
//}

/**********************************************************************************************************                     
*  Function Name  :	ReadUISettingFromRegistry                                                     
*  Description    :	Read Enable/disable sound entry from registry
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::ReadUISettingFromRegistry()
{
	HKEY key;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Wardwiz Antivirus"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
	{
		//AddLogEntry(L"Unable to open registry key");
		return false;
	}

	DWORD dwPlaySound;
	DWORD dwPlaySoundSize = sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	long ReadReg = RegQueryValueEx(key, L"dwEnableSound", NULL ,&dwType,(LPBYTE)&dwPlaySound, &dwPlaySoundSize);
	if(ReadReg == ERROR_SUCCESS)
	{
		dwPlaySound = (DWORD)dwPlaySound;
		if(dwPlaySound == 0)
		{
			/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
			theApp.m_bEnableSound = false;
			//m_chkDisableSound.SetCheck(TRUE);
		}
		else
		{
			/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
			theApp.m_bEnableSound = true;
			//m_chkDisableSound.SetCheck(FALSE);
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	PlayScanFinishedThread                                                     
*  Description    :	Thread will play a sound when scan gets finished.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
UINT PlayScanFinishedThread(LPVOID lpThis)
{
	PlaySound(_T("ScanFinished.wav"), NULL, SND_FILENAME | SND_LOOP | SND_SYNC);
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	PlayThreatsFoundThread                                                     
*  Description    :	Thread will play a sound when virus get found.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
UINT PlayThreatsFoundThread(LPVOID lpThis)
{
	PlaySound(_T("ThreatsFound.wav"), NULL, SND_FILENAME | SND_LOOP | SND_SYNC);
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckDisablesound                                                     
*  Description    :	Checks play sound checks box is ticked or not.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnBnClickedCheckDisablesound()
{
	DWORD dwPlaySound = 1;
	int iCheck = m_chkDisableSound.GetCheck();
	if(iCheck == 0)
	{
		/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
		theApp.m_bEnableSound = true;
	}
	else
	{
		dwPlaySound = 0;
		/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
		theApp.m_bEnableSound = false;		
	}

	HKEY hKey;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus"),&hKey)!= ERROR_SUCCESS)
	{
		return;
	}

	LONG setRes = RegSetValueEx (hKey ,L"dwEnableSound", 0, REG_DWORD, (LPBYTE)&dwPlaySound, sizeof(DWORD));
	if(setRes != ERROR_SUCCESS)
	{
		AddLogEntry(L"### Error in Setting Registry CScanDlg::OnBnClickedCheckDisablesound", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	MakeEntryInRegistry                                                     
*  Description    :	If virus is found and not get cleaned. Virusfound = 1 sets into registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::MakeEntryInRegistry()
{
	if(m_virusFound > 0)
	{
		HKEY hKey;
		LPCTSTR sk = TEXT("SOFTWARE\\Wardwiz Antivirus");

		LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sk, 0, KEY_ALL_ACCESS , &hKey);

		if (openRes==ERROR_SUCCESS)
		{
			//AddLogEntry(L"Success opening key.");
		}
		else
		{
			//AddLogEntry(L"Error opening key.");
			long CreateReg = RegCreateKeyEx(HKEY_LOCAL_MACHINE,sk,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,(LPDWORD)REG_CREATED_NEW_KEY);
			if(CreateReg==ERROR_SUCCESS)		
			{
				//AddLogEntry(L"Success created key.");
			}
			else
			{
				//AddLogEntry(L"UnSuccess created key.");
			}

		}

		LPCTSTR valueVirusFound = TEXT("VirusFound");
		DWORD dwdataVirusFound = m_virusFound;
		DWORD dwDataVirusLength = sizeof(dwdataVirusFound);
		LONG setRes = RegSetValueEx (hKey ,valueVirusFound, 0, REG_DWORD, (LPBYTE)&dwdataVirusFound,dwDataVirusLength);

		if (setRes == ERROR_SUCCESS)
		{
			//AddLogEntry(L"Success writing to Registry.");
		}
		else if(setRes == ERROR_MORE_DATA)
		{
			//AddLogEntry(L"Error writing to Registry.");
		}
	}
}



//DWORD CScanDlg::Encrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus)
//{
//
//	DWORD	dwRet = 0x00 ;
//	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00 ;
//	TCHAR	szExt[16] = {0} ;
//	DWORD	dwLen = 0x00 ;
//	LPBYTE	lpFileData = NULL ;
//	HANDLE	hFile = INVALID_HANDLE_VALUE ;
//	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;
//	m_dwEnKey = *( (DWORD * )&szpIpsy ) ;
//
//
//	__try
//	{
//		
//		if( !PathFileExists( m_szFilePath ) )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//
//		hFile = CreateFile(	m_szFilePath, GENERIC_READ, 0, NULL,
//								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
//		if( hFile == INVALID_HANDLE_VALUE )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		dwFileSize = GetFileSize( hFile, NULL ) ;
//		if( !dwFileSize )
//		{
//			dwRet = 0x03 ;
//			goto Cleanup ;
//		}
//		if( lpFileData )
//		{
//			free(lpFileData);
//			lpFileData=NULL;
//
//		}
//
//		lpFileData = (LPBYTE ) malloc( dwFileSize +0x08 ) ;
//		if( !lpFileData )
//		{
//			dwRet = 0x04 ;
//			goto Cleanup ;
//		}
//
//		memset(lpFileData, 0x00, dwFileSize ) ;
//		SetFilePointer( hFile, 0x00, NULL, FILE_BEGIN ) ;
//		ReadFile( hFile, lpFileData, dwFileSize, &dwBytesRead, NULL ) ;
//		if( dwFileSize != dwBytesRead )
//		{
//			dwRet = 0x04 ;
//			goto Cleanup ;
//		}
//		CloseHandle( hFile ) ;
//		if( DecryptData( (LPBYTE)lpFileData, dwBytesRead ) )
//		{
//			dwRet = 0x05 ;
//			goto Cleanup ;
//		}
//		
//		
//		hFileEnc = CreateFile(m_szFilePath, GENERIC_WRITE, 0, NULL,
//								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
//		if( hFileEnc == INVALID_HANDLE_VALUE )
//		{
//			dwRet = 0x06 ;
//			goto Cleanup ;
//		}
//
//		dwBytesRead = 0x00 ;
//		WriteFile( hFileEnc, lpFileData, dwFileSize, &dwBytesRead, NULL ) ;
//		if( dwFileSize != dwBytesRead )
//			dwRet = 0x07 ;
//		CloseHandle( hFileEnc ) ;
//
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//Cleanup :
//
//	if( lpFileData )
//		free( lpFileData ) ;
//	lpFileData = NULL ;
//
//	
//	return dwRet ;
//}

//DWORD CScanDlg::DecryptData( LPBYTE lpBuffer, DWORD dwSize )
//{
//
//	if( IsBadWritePtr( lpBuffer, dwSize ) )
//		return 1 ;
//
//	DWORD	i = 0 ;
//
//	for(; i<dwSize; )
//	{
//		if( (dwSize - i) < 0x04 )
//			break ;
//
//		*( (DWORD * )&lpBuffer[i] ) = *( (DWORD * )&lpBuffer[i] ) ^ m_dwEnKey ;
//		i += 0x04 ;
//	}
//
//	return 0 ;
//}
//
//BOOL CScanDlg::BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath)
//{
//	DWORD dwStatus=0;
//	CString csEncryptFilePath,csQuarantineFolderpath;
//	TCHAR szQuarantineFileName[MAX_PATH]={0};
//	UINT RetUnique=0;
//
//	//_tcscpy(m_szOriginalFilePath,csOriginalThreatPath);
//
//	csQuarantineFolderpath = GetQuarantineFolderPath(); 
//
//	RetUnique=GetTempFileName(csQuarantineFolderpath, L"tmp", 0, szQuarantineFileName);
//	if(!RetUnique)
//	{
//		return FALSE;
//	}
//	
//	m_csDuplicateName=szQuarantineFileName;
//			
//	if(!(CopyFile(csOriginalThreatPath,m_csDuplicateName,FALSE)))
//	{
//		return FALSE;
//	}
//	Encrypt_File(szQuarantineFileName,dwStatus);
//
////	csEncryptFilePath=m_szEncFile;
////	DeleteFile(csEncryptFilePath);
//	
//	return TRUE;
//
//}

//void CScanDlg::RecoverEntries(CString strOriginalName,CString strDuplicateName,CString strThreatName)
//{
//	m_nContactVersion = 1;
//	m_objRecoverEntriesdb.SetFileVersion(m_nContactVersion);
//	CIspyList newContact(strOriginalName,strDuplicateName,strThreatName, L"");
//	m_objRecoverEntriesdb.AddContact(newContact);
//}

//BOOL CScanDlg::StoredataContentToFile(CString csPathName)
//{
//	
//	CFile wFile(csPathName, CFile::modeCreate | CFile::modeWrite);
// 
//	// Create a storing archive
//	CArchive arStore(&wFile,CArchive::store);
//
//	m_objRecoverEntriesdb.Serialize(arStore);
//	
//	// Close the storing archive
//	arStore.Close();
//	wFile.Close();
//
//
//	return true;
//}

//void CScanDlg::LoadReportsDBFile()
//{
//	CString csFilePath;
//	TCHAR szModulePath[MAX_PATH] = {0};
//	if(!GetModulePath(szModulePath, MAX_PATH))
//	{
//		MessageBox(L"Error in GetModulePath", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
//		return;
//	}
//	csFilePath = szModulePath;
//	csFilePath.Append(L"\\WRDWIZREPORTS.DB");
//
//	if(!PathFileExists(csFilePath))
//	{
//		return;
//	}
//
//	CFile rFile(csFilePath, CFile::modeRead);
//	// Create a loading archive
//	CArchive arLoad(&rFile, CArchive::load);
//	m_objReportsDB.Serialize(arLoad);
//
//	// Close the storing archive
//	arLoad.Close();
//	rFile.Close();
//}


//void CScanDlg::LoadRecoverDBFile()
//{
//	CString csFilePath;
//	TCHAR szModulePath[MAX_PATH] = {0};
//	if(!GetModulePath(szModulePath, MAX_PATH))
//	{
//		MessageBox(L"Error in GetModulePath", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
//		return;
//	}
//	csFilePath = szModulePath;
//	csFilePath.Append(L"\\WRDWIZRECOVERENTRIES.DB");
//
//	if(!PathFileExists(csFilePath))
//	{
//		return;
//	}
//
//	CFile rFile(csFilePath, CFile::modeRead);
//	// Create a loading archive
//	CArchive arLoad(&rFile, CArchive::load);
//	m_objRecoverEntriesdb.Serialize(arLoad);
//
//	// Close the storing archive
//	arLoad.Close();
//	rFile.Close();
//}

/**********************************************************************************************************                     
*  Function Name  :	StartScanUsingService                                                     
*  Description    :	Used named pipe to give signal to service to start scanning .
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::StartScanUsingService(CStringArray &csaAllScanPaths)
{
	try
	{
		if(csaAllScanPaths.GetCount() == 0)
		{
			return false;
		}

		TCHAR szScanPath[1000] = {0};
		if(!MakeFullTokenizedScanPath(csaAllScanPaths, szScanPath))
		{
			return false;
		}

		ISPY_PIPE_DATA szPipeData = {0};
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = START_SCAN;
		szPipeData.dwValue =  static_cast<DWORD>(m_eScanType);
		wcscpy_s(szPipeData.szFirstParam, szScanPath);

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		//Varada Ikhar, Date: 13/05/2015
		//Issue : Double click scan button,scan will start and elapsed time will be 0. 4.Progress bar will be showing 0 % and once pop - up come it will show 100 %.
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		else
		{
			isScanning = true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SendRequestCommon                                                     
*  Description    :	Send a pause,stop,resume scanning request to comm service.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::SendRequestCommon(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to SERVICE_SERVER in CScanDlg::SendRequestCommon", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

//Send the all files path # sign token
/**********************************************************************************************************                     
*  Function Name  :	MakeFullTokenizedScanPath                                                     
*  Description    :	Toknization of scan path
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::MakeFullTokenizedScanPath(CStringArray &csaAllScanPaths, LPTSTR szScanPath)
{
	try
	{
		if(!szScanPath)
		{
			return false;
		}

		if(csaAllScanPaths.GetCount() == 0)
		{
			return false;
		}

		_tcscpy(szScanPath, csaAllScanPaths.GetAt(0));
		for(int iIndex = 1; iIndex < csaAllScanPaths.GetCount(); iIndex++)
		{
			_tcscat(szScanPath, L"#");
			_tcscat(szScanPath, csaAllScanPaths.GetAt(iIndex));
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::MakeFullTokenizedScanPath", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ScanningStarted                                                     
*  Description    :	Start scanning
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::ScanningStarted()
{

	//if the setup is with clam means need to run memory scan before any scan.
	if (theApp.m_eScanLevel == CLAMSCANNER)
	{
		m_bIsMemScan = true;
		m_prgScanProgress.SetWindowText(L"(" + theApp.m_objwardwizLangManager.GetString(L"IDS_SCANNING_MEMORY") + L") ");
	}

	m_bScanStarted = true;
	m_bScanningStopped = false;
	::ResetEvent(m_hScanStopEvent);
//VARADA IKHAR*****ISSUE NO 9. DATE : 23/12/2014************************************************************************* 
	//CString csElapsedTime;
	//csElapsedTime.Format(L"%s%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ELAPSEDTIME"),L"00:00:00");
	//m_stElapsedTime.SetWindowTextW(csElapsedTime);
//VARADA IKHAR****************************************************************************************************************

	if(!m_bIsMemScan)
	{
		SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCANNING_STARTED"));
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ScanningFinished                                                     
*  Description    :	calls function when scan finishes.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
*  Modified Date  : 9/2/2015 Neha Gharge No.of counts on UI and in log files are different.
**********************************************************************************************************/
bool CScanDlg::ScanningFinished()
{
	theApp.isScanning = false;
	CString csFileScanCount;
	
	/*
		ISSUE NO - 44 Two dialogs appears after full scan 
		NAME - Niranjan Deshak. - 12th Jan 2015
		Description : Added if condition for m_bFlagScanFinished
	*/
	if(m_bScanningStopped || m_bFlagScanFinished == false)
	{
		return true;
	}
	m_bFlagScanFinished = false;
	//Need to set the total file count with file scanned 
	//because so of the entries gonna missed because of clamscanner.
  // resolved by lalit 1-8-2015 .issue  after scanner stop ui not getting scanning aborted message and all scanning option disable.
 // below line commented due to above issue
	m_bScanningStopped = true;
	
	::WaitForSingleObject(m_hThreadVirEntries, 500);
	if(m_hThreadVirEntries != NULL)
	{
		ResumeThread(m_hThreadVirEntries);
		TerminateThread(m_hThreadVirEntries, 0);
		m_hThreadVirEntries = NULL;
	}

	::WaitForSingleObject(m_hThreadStatusEntry, 500);
	if(m_hThreadStatusEntry != NULL)
	{
		ResumeThread(m_hThreadStatusEntry);
		TerminateThread(m_hThreadStatusEntry, 0);
		m_hThreadStatusEntry = NULL;
	}

	OutputDebugString(L">>> m_hThreadStatusEntry stopped");
	
	//Save Local DB once scanning finished.

	SaveLocalDatabase();
	EnableAllWindows(TRUE);

	//Varada Ikhar, Date:18-04-2015,
	//Issue : 0000131 & 0000129 : 1. Start scanning in Quick scan 2. Go to dataencryption & click on 'Browse' 3. Quick scan completed dialog box appears. 
	//4. Keep it aside & cancel browsed explorer. 5. Go to quick scan again & click on 'scan' as it is still enabled. Then click 'OK' on that dialog box. 
	//6. After completion of 100% no pop up appears & when clicked on stop it is showing 'Quick scanning is in progress. Do you want to stop'. click yes no change occurs in progress bar & the status below. 
	//7. In reports it is showing as 'No threats found' 
	//ReInitializeVariables();

	m_bScanStarted = false;
	isScanning = false;

	if (theApp.m_eScanLevel == CLAMSCANNER)
	{
		m_prgScanProgress.SetWindowText(L"");
	}

	//Add scan end time log entry
	m_tsScanEndTime = CTime::GetCurrentTime();
	CString csTime =  m_tsScanEndTime.Format(_T("%H:%M:%S"));
	AddLogEntry(_T(">>> End Scan Time: %s"), csTime, 0, true, FIRSTLEVEL);

	if(m_virusFound > 0)
	{
		m_bRedFlag = true;
	}

	csTime =  m_tsScanElapsedTime.Format(_T("%H:%M:%S"));
	
	/*	ISSUE NO - 768 NAME - Neha G. TIME - 28th June 2014 */
	KillTimer(TIMER_SCAN_STATUS);
	CString csElapsedTime;
	csElapsedTime.Format(L"%s%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ELAPSEDTIME"),csTime);
	m_stElapsedTime.SetWindowText(csElapsedTime);

	//Issue		: Scanning Aborted message Added
	//Resolved	: Vilas
	//Date		: 28 - Nov - 2014
	CString csCompleteScanning;
	//Varada Ikhar, Date: 14/02/2015, Issue: Database needs to be updated.Database not valid.
	if (m_bSignatureFailed == true)
	{
		CString csFileScanned;
		m_FileScanned = 0;
		csFileScanned.Format(L"%s%d",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED"),m_FileScanned);
		m_csFilesScanned.SetWindowText(csFileScanned);
		csCompleteScanning.Format(L"%s",theApp.m_objwardwizLangManager.GetString(L"IDS_DATABASE_SIGNATURE_FAILED"));
	}
	else if(!m_bScnAborted)
	{
		/* Issue No: 13 In quick/full/custom scan incomplete message after scanning aborted.
		Resolved by : Nitin K Date: 19-Dec-2014
		*/
		csCompleteScanning.Format(L"%s, %s : %d, %s:%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_INFECTEDFILES"),theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_FOUND"),m_virusFound,theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_SCANTIME"), csTime);
	}
	else
	{
		/* Issue No: 13 In quick/full/custom scan incomplete message after scanning aborted.
		Resolved by : Nitin K Date: 19-Dec-2014
		*/
		csCompleteScanning.Format(L"%s, %s : %d, %s:%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_ABORTED"),theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_FOUND"),m_virusFound,theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_SCANTIME"), csTime);
	}
	SetScanStatus(csCompleteScanning);
	KillTimer(TIMER_FILESSCANNED_STATUS);
	
	AddLogEntry(L"---------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	//Varada Ikhar, Date: 18/04/2015,
	//Issue: 0000128 : In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
	CString cstypeofscan = L"";
	switch (m_eScanType)
	{
		case QUICKSCAN:	cstypeofscan = L"Quick";
						break;

		case FULLSCAN:	cstypeofscan = L"Full";
						break;

		case CUSTOMSCAN:cstypeofscan = L"Custom";
						break;

		default:		cstypeofscan = L"";
						break;
	}
	//Varada Ikhar, Date: 14/02/2015, Issue: Database needs to be updated.Database not valid.
	if(m_bSignatureFailed == true)
	{
		csCompleteScanning.Format(L"### Database needs to be Updated.(m_bSignatureFailed = true)");
	}
	else if(!m_bScnAborted)
	{
		m_FileScanned = m_iTotalFileCount; //Name:Varada Ikhar,Date:24/01/2015, Total files scanned should be same as totalfile count only if scanning is completed.
		//Varada Ikhar, Date: 18/04/2015,
		//Issue: 0000128 : In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
		csCompleteScanning.Format(L">>> %s %s.", cstypeofscan, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_INFECTEDFILES"));
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		csFileScanCount.Format(L"%s%d",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED"),m_FileScanned);
		m_csFilesScanned.SetWindowText(csFileScanCount);
		csCompleteScanning.Format(L">>> %s = %d, %s = %d, %s = %s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_FileScanned,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_THREAD_FOUND"), m_virusFound,theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_SCANTIME"), csTime);
	}
	else
	{
		//Varada Ikhar, Date: 18/04/2015,
		//Issue: 0000128 : In COMMSRV & AVUI log files for quick/full/custom scan,type of scan should be mentioned when particular scan is performed. eg.Quick Scan Started (Suggestion)
		csCompleteScanning.Format(L">>> %s %s.", cstypeofscan, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_ABORTED"));
		AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
		csFileScanCount.Format(L"%s%d",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED"),m_FileScanned);
		m_csFilesScanned.SetWindowText(csFileScanCount);
		csCompleteScanning.Format(L">>> %s = %d, %s = %d, %s = %s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_FILESCANNED"), m_FileScanned,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_THREAD_FOUND"), m_virusFound,theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_SCANTIME"), csTime);
	}
	
	//Varada Ikhar, Date:24/01/2015, Adding a log entry of total scan time.
	AddLogEntry(csCompleteScanning, 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"--------------------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);

	//Date			:	19 - 12 - 2014
	//Resolved by	:	Vilas
	//Issue			:	Virus or Malware detected count on Main GUI and Popup is different.
	//				Actually count setting was done in Timer. When user Aborts(Stops) scanning, Timer  
	//				sets first and then final count is updated.
	//				So Popup is giving final count and Main GUI not.
	csCompleteScanning.Format(L"%s%d",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TREATFOUND"),m_virusFound);
	m_stThreatsFound.SetWindowText(csCompleteScanning);

	//Added by Nitin Kolapkar
	//Date 17 Jan 2015
	//Issue:
	//if any virus found and does not clean then UI close and reopen it says to run full scan, 
	//even after full scan complete msg not changed to protected
	if(m_bFullScan)
	{
		if( (m_virusFound == 0) && (m_bScnAborted == false))
		{
			DWORD	dwbVirusFound = 0x00;
			if(!SetRegistrykeyUsingService(L"SOFTWARE\\Wardwiz Antivirus", 
				L"VirusFound", REG_DWORD, dwbVirusFound, false))
			{
				AddLogEntry(L"### Error in Set SetRegistrykeyUsingService VirusFound in CScanDlg::QuaratineEntries", 0, 0, true, SECONDLEVEL);
			}
		}
	}

	//KillTimer(TIMER_FILESSCANNED_STATUS);
	
	//Neha Gharge 3/3/2015 
	//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
	//if(m_bHome || m_bClose)
	
	if (m_virusFound > 0)
	{
		ShowSpyFoundDlg(true);
	}
	else
	{
		ShowSpyFoundDlg(false);
	}
	
	m_bScanningStopped = true;

	DWORD dwTotalRebootCount = 0x00;

	if (m_virusFound == 0)
	{
		dwTotalRebootCount = CheckForDeleteFileINIEntries() + CheckForRepairFileINIEntries();
		if (dwTotalRebootCount)
		{
			CString csMsgToRebootSystem(L"");

			csMsgToRebootSystem.Format(L"%s %d %s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART1"), dwTotalRebootCount, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART2"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));
			
			m_bIsPopUpDisplayed = true;
			if (MessageBox(csMsgToRebootSystem, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				m_bIsPopUpDisplayed = false;
				//Write a code to restart computer.
				CEnumProcess enumproc;
				enumproc.RebootSystem(0);
			}
			m_bIsPopUpDisplayed = false;

		}
	}
	if(m_virusFound == 0)
	{

			ResetControlsValues();

	}
	m_bQuickScan = false;
	m_bFullScan = false;
	m_bCustomscan = false;
	/**************************ISSUE No: Reported in new sheet Neha Gharge 22/5/14 *********************************/
		/*if(m_bQuickScan)
		{
			m_bQuickScan = false;
			if(g_TabCtrlWindHandle != NULL)
			{
				if(g_TabCtrlWindHandle->m_btnCustomScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnCustomScan->EnableWindow(true);
				}
				if(g_TabCtrlWindHandle->m_btnFullScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnFullScan->EnableWindow(true);
				}
				if(g_TabCtrlWindHandle->m_btnAntirootkitScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnAntirootkitScan->EnableWindow(true);
				}
			}
		}
		if(m_bFullScan)
		{
			m_bFullScan = false;
			if(g_TabCtrlWindHandle != NULL)
			{
				if(g_TabCtrlWindHandle->m_btnQuickScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnQuickScan->EnableWindow(true);
				}
				if(g_TabCtrlWindHandle->m_btnCustomScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnCustomScan->EnableWindow(true);
				}
				if(g_TabCtrlWindHandle->m_btnAntirootkitScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnAntirootkitScan->EnableWindow(true);
				}
			}
		}

		if(m_bCustomscan)
		{
			m_bCustomscan = false;
			if(g_TabCtrlWindHandle != NULL)
			{
				if(g_TabCtrlWindHandle->m_btnQuickScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnQuickScan->EnableWindow(true);
				}
				if(g_TabCtrlWindHandle->m_btnFullScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnFullScan->EnableWindow(true);
				}
				if(g_TabCtrlWindHandle->m_btnAntirootkitScan != NULL)
				{
					g_TabCtrlWindHandle->m_btnAntirootkitScan->EnableWindow(true);
				}
			}
		}*/
		//Varada Ikhar, Date:18-04-2015,
		//Issue : 0000131 & 0000129 : 1. Start scanning in Quick scan 2. Go to dataencryption & click on 'Browse' 3. Quick scan completed dialog box appears. 
		//4. Keep it aside & cancel browsed explorer. 5. Go to quick scan again & click on 'scan' as it is still enabled. Then click 'OK' on that dialog box. 
		//6. After completion of 100% no pop up appears & when clicked on stop it is showing 'Quick scanning is in progress. Do you want to stop'. click yes no change occurs in progress bar & the status below. 
		//7. In reports it is showing as 'No threats found' 
		ReInitializeVariables();
		m_chkSelectAll.EnableWindow(TRUE);
	::SetEvent(m_hScanStopEvent);

	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ShowStaus                                                     
*  Description    :	Show status (Not in used).
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::ShowStaus(LPCTSTR szStatus)
{
}

/**********************************************************************************************************                     
*  Function Name  :	ShowVirusEntry                                                     
*  Description    :	Show virus entry (Not in used).
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::ShowVirusEntry(LPCTSTR szStatus)
{
}

/**********************************************************************************************************                     
*  Function Name  :	SetTotalFileCount                                                     
*  Description    :	Set total file count.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::SetTotalFileCount(DWORD dwFileCount, DWORD dwIsMemScan)
{
	//Varada Ikhar,		Date:02/02/2015,	Version: 1.8.3.5
	//Issue: If we scan a empty folder, and click on other scan options fields in other scan becomes blank.
	if(dwFileCount == 0)
	{
		m_ScanCount = false;
		m_prgScanProgress.ShowWindow( SW_SHOW ) ;
		return;
	}

	if(dwFileCount > 0)
	{
		m_ScanCount = true ;

		CString csLog;

		if (dwIsMemScan == TRUE)
		{
			csLog.Format(L">>> MEMSCAN: Total Files to Scan: %d", dwFileCount);
			m_iMemScanTotalFileCount = dwFileCount;

			if (m_iMemScanTotalFileCount)
			{
				m_prgScanProgress.SetRange32(0, m_iMemScanTotalFileCount);
			}
		}
		else
		{
			m_iTotalFileCount = dwFileCount;
			csLog.Format(L">>> NOMEMSCAN: Total Files to Scan: %d", dwFileCount);
			if (!m_bIsMemScan)
			{
				if (m_iTotalFileCount)
				{
					m_prgScanProgress.SetRange32(0, m_iTotalFileCount);
				}
			}
		}

		AddLogEntry(L"%s", csLog, 0, true, SECONDLEVEL);

		m_prgScanProgress.ShowWindow( SW_SHOW ) ;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	SendFile4RepairUsingService                                                     
*  Description    :	Send request to clean file to service
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::SendFile4RepairUsingService(int iMessage, CString csThreatPath,CString csThreatName, DWORD dwISpyID ,bool bWait, bool bReconnect)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = iMessage;
	szPipeData.dwValue =  dwISpyID;
	wcscpy_s(szPipeData.szFirstParam, csThreatPath);
	wcscpy_s(szPipeData.szSecondParam , csThreatName);

	if(!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CScanDlg::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CScanDlg::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if(szPipeData.dwValue == 0)
		{
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SetRegistrykeyUsingService                                                     
*  Description    :	Set any dword value into given key into registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD; 
	//szPipeData.hHey = HKEY_LOCAL_MACHINE;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName );
	szPipeData.dwSecondValue = dwData;

	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SendRecoverOperations2Service                                                     
*  Description    :	Send a request to stored data into recover db.So that user can recover file.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry ,DWORD dwType, bool bWait, bool bReconnect)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = dwMessageinfo;
	_tcscpy_s(szPipeData.szFirstParam , csRecoverFileEntry);
	szPipeData.dwValue = dwType;
	//CISpyCommunicator objCom(SERVICE_SERVER, bReconnect);

	if(!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CScanDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to Read data in CScanDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if(szPipeData.dwValue != 1)
		{
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name	 :	Check4DBFiles                                                     
*  Description		 :	Checks for signature db are exist or not
*  Author Name		 : Ramkrushna Shelke
*  SR_NO		   	 :
*  Date				 : 26 Nov 2013
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
**********************************************************************************************************/
bool CScanDlg::Check4DBFiles()
{
	DWORD dwDBVersionLength = 0;
	TCHAR szModulePath[MAX_PATH] = {0};
	CWrdwizEncDecManager objWrdwizEncDecMgr;

	if(!GetModulePath(szModulePath, MAX_PATH))
	{
			return false;
	}
	CString csDBFilesFolderPath = szModulePath;
	CString csWRDDBFilesFolderPath = szModulePath;
	csDBFilesFolderPath += L"\\DB";
	csWRDDBFilesFolderPath += L"\\WRDWIZDATABASE";
	if(!PathFileExists(csDBFilesFolderPath) && !PathFileExists(csWRDDBFilesFolderPath))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_LATEST_SIGNATURE_NOT_PRESENT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
		m_bIsPopUpDisplayed = false;

		return false;
	}
	//Varada Ikhar, Date:20/03/2015
	//Issue: If DB is corrupted and USB scan is performed then the message "Database should be updated" takes more time to display.
	else if (!Check4ValidDBFiles(csWRDDBFilesFolderPath))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DATABASE_MESSAGE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		m_bIsPopUpDisplayed = false;

		return false;
	}
	else
	{
		CStringArray csaDBFiles;
 
		if( theApp.m_eScanLevel == WARDWIZSCANNER)
		{
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\WRDWIZAV1.DB");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\WRDWIZAVR.DB");
		}
		else
		{
			csaDBFiles.Add(csDBFilesFolderPath + L"\\MAIN.CVD");
			csaDBFiles.Add(csDBFilesFolderPath + L"\\DAILY.CLD");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\WRDWIZAV1.DB");
			csaDBFiles.Add(csWRDDBFilesFolderPath + L"\\WRDWIZAVR.DB");
		}
		
		for(int iIndex = 0; iIndex < csaDBFiles.GetCount(); iIndex++)
		{
			if(!PathFileExists(csaDBFiles.GetAt(iIndex)))
			{
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_LATEST_SIGNATURE_NOT_PRESENT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
				m_bIsPopUpDisplayed = false;
				return false;
			}
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	EnableDisableControlForClean                                                     
*  Description    :	Enable/ disable function for clean process
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::EnableDisableControlForClean(bool bEnable)
{
	m_prgScanProgress.EnableWindow(bEnable);
	//m_btnBrowse.EnableWindow(bEnable);
	m_btninnerCutomScan.EnableWindow(bEnable);
	m_btnScan.EnableWindow(bEnable);
	m_chkDisableSound.EnableWindow(bEnable);
	m_chkSelectAll.EnableWindow(bEnable);
}

/**********************************************************************************************************                     
*  Function Name  :	RefreshStrings                                                     
*  Description    :	this function is  called for setting the Text UI with different Language Support
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::RefreshStrings()
{
	CString csElapsedTime,csThreatFound,csFileScanned;
	m_stQuickHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_QUICKSCAN_HEADER"));
	m_stQuickHeaderDescription.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_QUICKSCAN_DES"));
	m_stSelectAll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SELECTALL"));
	m_stDisableSound.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DISABLESOUND"));
	csElapsedTime.Format(L"%s%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ELAPSEDTIME"),L"00:00:00");
	m_stElapsedTime.SetWindowTextW(csElapsedTime);
	csThreatFound.Format(L"%s%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED"),L"0");
	m_csFilesScanned.SetWindowTextW(csThreatFound);
	csFileScanned.Format(L"%s%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TREATFOUND"),L"0");
	m_stThreatsFound.SetWindowTextW(csFileScanned);
	//m_ScanPercentage.SetWindowText(L"0%");
	m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
	m_btnPauseResume.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
	m_btnStop.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"));
	m_btnClean.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CLEAN"));
	m_btnBrowse.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_BROWSE"));;
	m_btninnerCutomScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
	m_stFullScanHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FULLSCAN_HEADER"));
	m_stFullScanHeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FULLSCN_DES"));
	m_stCustomHeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOMSCN_DES"));
	m_stCustomScnHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOMSCAN_HEADER"));
	//Issue: Custom scan UI implementation
	//Added by: Nitin K.
	m_stCustomDragDropText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOM_DRAG_DROP_TEXT"));
	m_csStaticElapsedTime = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ELAPSEDTIME");
	m_csStaticFilesScanned = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED");
	m_csStaticThreatsFound = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TREATFOUND");

	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	CloseCleaning                                                     
*  Description    :	To close cleaning process
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::CloseCleaning()
{
	try
	{
		if (m_bIsCleaning == false)
		{
			return false;
		}

		//Varada Ikhar, Date: 28/04/2015
		//Issue : While quarantine entries is in progress, if user clicks on close, quarantine process should be paused.
		if (m_hQuarantineThread != NULL)
		{
			if (SuspendThread(m_hQuarantineThread) == -1)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to Suspend QuarantineThread in CScanDlg::CloseCleaning with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Quarantine Thread Paused.", 0, 0, true, FIRSTLEVEL);
		}

		CString csCleanProcess = L"";
		//Issue No:1186 While cleaning process is in progress decrypt any file from any path by using right click decryption option. Then the popup appearing as 'Cleaning is in progress. Do you want to Close?' ->message should be changed.
		csCleanProcess.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_CLEANING_INPROCESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_PROG_TWO"));
		
		m_bIsPopUpDisplayed = true;
		int iReturn = MessageBox(csCleanProcess, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		m_bIsPopUpDisplayed = false;

		if (iReturn == IDNO)
		{
			//Varada Ikhar, Date: 28/04/2015
			//Issue : While quarantine entries is in progress, if user clicks on close, quarantine process should be paused.
			if (m_hQuarantineThread != NULL)
			{
				if (ResumeThread(m_hQuarantineThread) == -1)
				{
					CString csErrorMsg = L"";
					DWORD ErrorCode = GetLastError();
					csErrorMsg.Format(L"### Failed to Resume QuarantineThread in CScanDlg::CloseCleaning with GetLastError code %d", ErrorCode);
					AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
				}
				AddLogEntry(L">>> Quarantine Thread Resumed.", 0, 0, true, FIRSTLEVEL);
			}
			return false;
		}
		else
		{
			if (m_hQuarantineThread != NULL)
			{
				//Varada Ikhar, Date: 28/04/2015
				//Issue : While quarantine entries is in progress, if user clicks on close, quarantine process should be paused.
				if (m_hQuarantineThread != NULL)
				{
					if (TerminateThread(m_hQuarantineThread, 0) == FALSE)
					{
						CString csErrorMsg = L"";
						DWORD ErrorCode = GetLastError();
						csErrorMsg.Format(L"### Failed to Terminate QuarantineThread in CScanDlg::CloseCleaning with GetLastError code %d", ErrorCode);
						AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
					}
					AddLogEntry(L">>> Quarantine Thread Terminated.", 0, 0, true, FIRSTLEVEL);
					m_hQuarantineThread = NULL;
				}
			}
			if (!SaveDBEntries())
			{
				AddLogEntry(L"### Error in SaveDBEntries", 0, 0, true, SECONDLEVEL);
			}
			//Varada Ikhar, Date: 4th May-2015
			//Issue : While cleanning is in progress, if any other tab is clicked and clicked 'Yes' to stop confirmation box,
			//Cleanning operation gets aborted. However, next time when clicked on any other tab again "Cleaning is in progress. Do you want to stop?" message gets displayed.
			m_bIsCleaning = false;
			//Issue: 1141cleaning of virus files are aborted but still few detected files are present which are to be cleaned.
			if (!ISAllItemsCleaned())
			{
				m_chkSelectAll.EnableWindow(TRUE);
				m_btnClean.EnableWindow(TRUE);
			}
			else
			{
				ResetControlsValues();
				m_btnClean.EnableWindow(FALSE);
			}

			if (m_eCurrentSelectedScanType == QUICKSCAN)
			{
				m_bQuickScan = false;
			}
			else if (m_eCurrentSelectedScanType == FULLSCAN)
			{
				m_bFullScan = false;
			}
			else if (m_eCurrentSelectedScanType == CUSTOMSCAN)
			{
				m_bCustomscan = false;
			}
			//Issue no : 1187While cleaning process is in progress decrypt any file by using right click decrypt option.
			//And stop cleaning process but it is not redirecting to Dataencryption feature as it appears if we abort scanning process.
			if (theApp.m_hCryptOprEvent)
			{
				SetEvent(theApp.m_hCryptOprEvent);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::CloseCleaning.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ThreatsFound                                                     
*  Description    :	Show a message that threats are found
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::ThreatsFound()
{
	if(m_virusFound > 0 && m_bIsCleaning == false)
	{
		CString csThreatsFound = L"";
		csThreatsFound.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_THREAT_DETECTED"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_WANNA_CLOSE"));
		
		m_bIsPopUpDisplayed = true;
		int iReturn = MessageBox(csThreatsFound,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION|MB_YESNO);
		m_bIsPopUpDisplayed = false;

		if(iReturn == IDNO)
		{
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SaveDBEntries                                                     
*  Description    :	Save recover entries in DB folder
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
bool CScanDlg::SaveDBEntries()
{
	//used 0 as Type for Saving RECOVER DB
	if(!SendFile4RepairUsingService(SAVE_RECOVERDB, L"", L"", 0, true, true))
	{
		AddLogEntry(L"### Error in CScanDlg::SaveDBEntries SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
		return false;
	}

	Sleep(5);
	//used 0 as Type for Saving RECOVER DB
	if(!SendFile4RepairUsingService(SAVE_REPORTS_ENTRIES, L"", L"", 5, true, true))
	{
		AddLogEntry(L"### Error in CScanDlg::SaveDBEntries SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;	
}

/**********************************************************************************************************                     
*  Function Name  :	OnPaint                                                     
*  Description    :	The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 26 Nov 2013
**********************************************************************************************************/
void CScanDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	//m_edtStatus.SetWindowTextW(m_csStaticFilePath); //Sanjay Khapre
	m_stElapsedTime.Invalidate();
	m_csFilesScanned.Invalidate();
	m_stThreatsFound.Invalidate();
	m_stSelectAll.Invalidate();
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}


/**********************************************************************************************************                     
*  Function Name  :	VirusFoundEntries                                                     
*  Description    :	Pipe Communication to send Virus found entries to UI
					Issue: Displaying Virus found entries on UI through Pipe Communication
					Date : 15-Jan-2015
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 15-Jan-2015 
**********************************************************************************************************/
bool CScanDlg :: VirusFoundEntries(LPISPY_PIPE_DATA lpSpyData)
{
	CString csCurrentPath(L"");

	csCurrentPath.Format(L"%s", lpSpyData->szSecondParam);
	if(csCurrentPath.GetLength() > 0 && (g_csPreviousVirusFoundPath != csCurrentPath))
	{
		OutputDebugString(csCurrentPath);
		CString csSpyID;
		csSpyID.Format(L"%d", lpSpyData->dwValue);
		this->InsertItem(lpSpyData->szFirstParam, csCurrentPath, theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED"), csSpyID);
		AddLogEntry(L">>> Virus Found GUI: %s, File Path: %s", lpSpyData->szFirstParam, csCurrentPath, true, SECONDLEVEL);
	}
	g_csPreviousVirusFoundPath.SetString(csCurrentPath);
	return true;
}
/***********************************************************************************************                    
*  Function Name  : EnumFolderToCheckEmptyFolder                                                     
*  Description    : Enumrate all the files in folder to check whether folder is empty or not.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 03 Feb 2015
*************************************************************************************************/
bool CScanDlg::EnumFolderToCheckEmptyFolder(LPCTSTR pstr)
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
				CString str = finder.GetFilePath();
				EnumFolderToCheckEmptyFolder(str);
			}
			else
			{
				m_iTotalNoofFiles++;
				return true;
			}
		}
		finder.Close();
		return false;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::EnumFolderToCheckEmptyFolder", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetShortFilePath
*  Description    :	Retrieves the short path form of the specified path.
*  Author Name    : Neha Gharge
*  Date           : 26 Feb 2015
*  SR_NO		  : 
**********************************************************************************************************/
void CScanDlg::GetShortFilePath(CString csFilePath)
{
	if (csFilePath == L"")
	{
		return;
	}
	memset(m_szShortPath, 0, sizeof(m_szShortPath));
	GetShortPathName(csFilePath, m_szShortPath, 120);

}


/***********************************************************************************************                    
*  Function Name  : CheckForDeleteFileINIEntries                                                     
*  Description    : check whether delete file ini present or not
*  Author Name    : Neha gharge
*  SR_NO		  :
*  Date           : 18 Feb 2015
*************************************************************************************************/
DWORD CScanDlg::CheckForDeleteFileINIEntries()
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
*  Function Name  :	Check4ValidDBFiles
*  Description    :	This function will check for valid signature and valid Version length in DB files
					if any mismatch found, will return false otherwise true.
*  Author Name    :	Varada Ikhar
*  SR_NO		  :
*  Date           : 20 Mar 2015
**********************************************************************************************************/
bool CScanDlg::Check4ValidDBFiles(CString csDBFolderPath)
{
	try
	{
		CString csFilePath;
		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAV1);

		DWORD dwDBVersionLength = 0;
		DWORD dwDBMajorVersion = 0;
		CWrdwizEncDecManager objWrdwizEncDecMgr;
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(csFilePath, dwDBVersionLength, dwDBMajorVersion))
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

		csFilePath.Format(L"%s\\%s", csDBFolderPath, WRDWIZAVR);

		DWORD dwMajorVersion = 0x00;
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(csFilePath, dwDBVersionLength, dwMajorVersion))
		{
			AddLogEntry(L"### Invalid DB found (or) may corrupted, File Name %s", csFilePath , 0, true, SECONDLEVEL);
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

/***********************************************************************************************
*  Function Name  : HideControlForCustomScan
*  Description    : Hides the control for Custom Scan
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::HideControlForCustomScan(bool bFlag)
{
	try
	{
		m_lstVirusesList.ShowWindow(bFlag);
		m_prgScanProgress.ShowWindow(bFlag);
		m_csFilesScanned.ShowWindow(bFlag);
		m_stElapsedTime.ShowWindow(bFlag);
		m_stThreatsFound.ShowWindow(bFlag);
		m_btnPauseResume.ShowWindow(bFlag);
		m_btnStop.ShowWindow(bFlag);
		m_chkSelectAll.ShowWindow(bFlag);
		//Issue: Custom scan UI implementation
		//Added by: Nitin K.
		//m_stSelectAll.ShowWindow(bFlag);
		m_btnScan.ShowWindow(bFlag);
		m_edtStatus.ShowWindow(bFlag);
		m_stCustomDragDropText.ShowWindow(!bFlag);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::HideControlForCustomScan", 0, 0, true, SECONDLEVEL);;
	}
}

/***********************************************************************************************
*  Function Name  : showListControlForCustomScan
*  Description    : Display the Listcontrols for custom scan
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::showListControlForCustomScan()
{
	try
	{
		//Design implementation of Custom scan 
		//Added by: Nitin K. 28th March 2015
		m_lstDrivePicker.InsertColumn(0, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOMSCAN_FILESFOLDERS_PATH"), LVCFMT_LEFT, 380);

		// Set full row selection and an image list
		ListView_SetExtendedListViewStyle(m_lstDrivePicker.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);

		// Specify what we will allow to be dropped onto the list
		CFileDropCStatic::DROPLISTMODE dropMode;
		dropMode.iMask = CFileDropCStatic::DL_ACCEPT_FILES | CFileDropCStatic::DL_ACCEPT_FOLDERS;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::showListControlForCustomScan", 0, 0, true, SECONDLEVEL);;
	}
}

/***********************************************************************************************
*  Function Name  : ShowControlsAfterCustomScanStarted
*  Description    :	Display the Visrus list and other control when custom scan starts
*  Author Name    : Nitin K.
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::ShowControlsAfterCustomScanStarted(bool bFlag)
{
	try
	{
		m_lstVirusesList.ShowWindow(bFlag);
		m_prgScanProgress.ShowWindow(bFlag);
		m_csFilesScanned.ShowWindow(bFlag);
		m_stElapsedTime.ShowWindow(bFlag);
		m_stThreatsFound.ShowWindow(bFlag);
		m_edtStatus.ShowWindow(bFlag);
		m_btnPauseResume.ShowWindow(bFlag);
		m_btnStop.ShowWindow(bFlag);
		m_chkSelectAll.ShowWindow(bFlag);
		m_stSelectAll.ShowWindow(bFlag);
		m_btnScan.ShowWindow(!bFlag);
		m_lstDrivePicker.ShowWindow(!bFlag);
		m_stDragDropFiles.ShowWindow(!bFlag);
		m_stCustomDragDropText.ShowWindow(!bFlag);
		m_btnCutomAdd.ShowWindow(!bFlag);
		m_btnCustomDelete.ShowWindow(!bFlag);
		m_btnCustomEdit.ShowWindow(!bFlag);
		m_cbCustomSelectAll.ShowWindow(!bFlag);
		m_btninnerCutomScan.ShowWindow(!bFlag);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::ShowControlsAfterCustomScanStarted", 0, 0, true, SECONDLEVEL);;
	}
}

/***********************************************************************************************
*  Function Name  : OnBnClickedCheckCustomSelectAll
*  Description    : Selects/Deselects the all list control entries for custom scan
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::OnBnClickedCheckCustomSelectAll()
{
	try
	{
		int iCheck = m_cbCustomSelectAll.GetCheck();

		for (int i = 0; i < m_lstDrivePicker.GetItemCount(); i++)
		{
			m_lstDrivePicker.SetCheck(i, iCheck);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::OnBnClickedCheckCustomSelectAll", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : OnBnClickedButtonCustomAdd
*  Description    : display the browse window for custom scan selection
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::OnBnClickedButtonCustomAdd()
{
	try
	{
		bool bNetworkPath = false;
		bool bAlreadyAdded = false;
		CSelectDialog ofd(TRUE, _T("*.*"), NULL,
			OFN_HIDEREADONLY | OFN_NODEREFERENCELINKS | OFN_FILEMUSTEXIST |OFN_NONETWORKBUTTON,
			_T("All files and folders(*.*)|*.*||"));

		m_bIsPopUpDisplayed = true;
		if (ofd.DoModal() != IDOK)
		{
			m_bIsPopUpDisplayed = false;
			return;
		}
		m_bIsPopUpDisplayed = false;

		for (int iPos = 0; iPos < ofd.m_SelectedItemList.GetCount(); iPos++)
		{
			LVFINDINFO lvInfo;
			lvInfo.flags = LVFI_STRING;
			CString csFileName = NULL;
			CString csScanPath = NULL;
			int iCListEntriesLength = 0;
			csFileName = ofd.m_SelectedItemList[iPos];
			lvInfo.psz = csFileName;
			//Design implementation of Custom scan 
			//Added by: Nitin K. 28th March 2015
			if (m_lstDrivePicker.FindItem(&lvInfo, -1) != -1)
			{
				bAlreadyAdded = true;
				continue;
			}
			if (PathIsNetworkPath(csFileName))
			{
				bNetworkPath = true;
				continue;
			}
			//Issue fix : 1129 Add maximum files/folders in to custom scan and then add any drive & click 'Scan' then WardWiz is getting hang.
			int iCount = 0;
			for (iCount; iCount < m_lstDrivePicker.GetItemCount(); iCount++)
			{
				csScanPath = m_lstDrivePicker.GetItemText(iCount, 0);
				iCListEntriesLength += csScanPath.GetLength();
				
			}
			iCListEntriesLength += iPos;
			if ((1000 - iCListEntriesLength - iCount) > csFileName.GetLength())
			{
				m_lstDrivePicker.InsertItem(0, ofd.m_SelectedItemList[iPos]);
				m_lstDrivePicker.SetCheck(0, 1);
			}
			else
			{
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MAX_LIMIT_FOR_CUSTOM_SCAN"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
				m_bIsPopUpDisplayed = false;
				break;
			}

		}
		
		/*if (bAlreadyAdded)
		{
			MessageBox(L"File/Folder is already added in List.", L"WardWiz", MB_ICONERROR | MB_OK);
		}*/
		if (bNetworkPath)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NETWORK_PATH_NOT_ALLOWED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR | MB_OK);
			m_bIsPopUpDisplayed = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::OnBnClickedButtonCustomAdd", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : OnBnClickedButtonCustomEdit
*  Description    : Called for editing any perticular entry in list control 
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::OnBnClickedButtonCustomEdit()
{
	try
	{
		int iPos = -1;
		int iChkCount = 0;
		bool bSelected = false;
		if (m_lstDrivePicker.GetItemCount() == 0)
		{
			m_bIsPopUpDisplayed = true;
			//Issue : 0000378 Issue with symbols in message box in Custom scan
			//Resolved by Nitin K Date: 14th May 2015
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NO_ENTRY_AVAILABLE_FOR_EDIT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONWARNING | MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
		}
		for (int i = 0; i < m_lstDrivePicker.GetItemCount(); i++)
		{
			if (m_lstDrivePicker.GetCheck(i))
			{
				iPos = i;
				iChkCount++;
				bSelected = true;
			}
		}
		if (!bSelected)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NO_ENTRY_SELECTED_FOR_EDIT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONWARNING | MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		if (iChkCount > 1)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NO_ENTRY_SELECT_ONE_FOR_EDIT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONWARNING | MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		CSelectDialog ofd(TRUE, _T("*.*"), NULL,OFN_HIDEREADONLY  | OFN_NONETWORKBUTTON,_T("All files and folders(*.*)|*.*||"));

		m_bIsPopUpDisplayed = true;
		if (ofd.DoModal() != IDOK)
		{
			m_bIsPopUpDisplayed = false;
			return;
		}
		m_bIsPopUpDisplayed = false;
		
		if (iPos != -1)
		{
			m_lstDrivePicker.DeleteItem(iPos);
		}

		for (int iListCount = 0; iListCount < ofd.m_SelectedItemList.GetCount(); iListCount++)
		{
			LVFINDINFO lvInfo;
			lvInfo.flags = LVFI_STRING;
			CString csFileName = NULL;
			CString csScanPath = NULL;
			int iCListEntriesLength = 0;
			csFileName = ofd.m_SelectedItemList[iListCount];
			lvInfo.psz = csFileName;
			if (m_lstDrivePicker.FindItem(&lvInfo, -1) != -1)
				continue;
			if (PathIsNetworkPath(csFileName))
				continue;

			for (int iCount = 0; iCount < m_lstDrivePicker.GetItemCount(); iCount++)
			{
				csScanPath = m_lstDrivePicker.GetItemText(iCount, 0);
				iCListEntriesLength += csScanPath.GetLength();
			}
			if ((1000 - iCListEntriesLength) > csFileName.GetLength())
			{

				m_lstDrivePicker.InsertItem(iPos, ofd.m_SelectedItemList[iListCount]);
				m_lstDrivePicker.SetCheck(iPos, 1);
			}
			m_cbCustomSelectAll.SetCheck(1);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CScanDlg::OnBnClickedButtonCustomEdit", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : OnBnClickedButtonCustomDelete
*  Description    : Called to delete the selected list control entry
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::OnBnClickedButtonCustomDelete()
{
	try
	{
		int iChkCount  = 0 ;
		int iListCount = 0 ;
		int iCount = 0 ;
		iListCount = m_lstDrivePicker.GetItemCount();
		if (iListCount == 0)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NO_ENTRY_AVAILABLE_FOR_DELETE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_bIsPopUpDisplayed = false;
			m_cbCustomSelectAll.SetCheck(0);
			return;
		}
		for (iCount = 0; iCount < m_lstDrivePicker.GetItemCount(); iCount++)
		{
			if (m_lstDrivePicker.GetCheck(iCount) == BST_CHECKED)
			{
				iChkCount++;
			}
		}
		if (iCount != 0 && iChkCount > 0)
		{
			m_bIsPopUpDisplayed = true;
			if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_CONFIRM_DELETION"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDNO)
			{
				m_bIsPopUpDisplayed = false;
				return;
		}
			m_bIsPopUpDisplayed = false;
		}
		else if (iChkCount == 0)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NO_ENTRY_SELECTED_FOR_DELETE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
		}

		for (iListCount; iListCount >= 0; iListCount--)
		{
			if (m_lstDrivePicker.GetCheck(iListCount) == BST_CHECKED)
			{
				m_lstDrivePicker.DeleteItem(iListCount);
			}
		}
		AddDeleteEditCustomScanEntryToINI();
		for (iCount = 0; iCount < m_lstDrivePicker.GetItemCount(); iCount++)
		{
			m_lstDrivePicker.SetCheck(iCount, 1);
		}

		m_cbCustomSelectAll.SetCheck(1);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CScanDlg::OnBnClickedButtonCustomEdit", 0, 0, true, SECONDLEVEL);;
	}
}

/***********************************************************************************************
*  Function Name  : ExpandShortcut
*  Description    : gives the full path for given path
*  Author Name    : Nitin K.
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
CString CScanDlg::ExpandShortcut(CString& csFilename) const
{
	CString csExpandedFile = NULL;
	try
	{
		USES_CONVERSION;		// For T2COLE() below

		// Make sure we have a path
		if (csFilename.IsEmpty())
		{
			ASSERT(FALSE);
			return csExpandedFile;
		}

		// Get a pointer to the IShellLink interface
		HRESULT hr;
		IShellLink* pIShellLink;

		hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (LPVOID*)&pIShellLink);

		if (SUCCEEDED(hr))
		{
			// Get a pointer to the persist file interface
			IPersistFile* pIPersistFile;
			hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);

			if (SUCCEEDED(hr))
			{
				// Load the shortcut and resolve the path
				// IPersistFile::Load() expects a UNICODE string
				// so we're using the T2COLE macro for the conversion
				// For more info, check out MFC Technical note TN059
				// (these macros are also supported in ATL and are
				// so much better than the ::MultiByteToWideChar() family)
				hr = pIPersistFile->Load(T2COLE(csFilename), STGM_READ);

				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = pIShellLink->GetPath(csExpandedFile.GetBuffer(MAX_PATH),
						MAX_PATH,
						&wfd,
						SLGP_UNCPRIORITY);

					csExpandedFile.ReleaseBuffer(-1);
				}
				pIPersistFile->Release();
			}
			pIShellLink->Release();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::ExpandShortcut", 0, 0, true, SECONDLEVEL);
	}
	return csExpandedFile;
}

/***********************************************************************************************
*  Function Name  : AddDeleteEditCustomScanEntryToINI
*  Description    : Add the selected entries of custom scan into INI file for next time use
*  Author Name    : Nitin K.
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::AddDeleteEditCustomScanEntryToINI()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L"### CScanDlg::AddDeleteEditCustomScanEntryToINI::GetModulePath failed", 0, 0, true, SECONDLEVEL);
			return;
		}
		CString csCustomScanINIPath = szModulePath;
		csCustomScanINIPath += L"\\WRDSETTINGS";
		TCHAR szValueName[260] = { 0 };
		CString csCustomScanEntryINIPath(L"");

		csCustomScanEntryINIPath.Format(L"%s\\WRDWIZCUSTOMSCANENTRY.INI", csCustomScanINIPath);
		DeleteFile(csCustomScanEntryINIPath);
		if (!PathFileExists(csCustomScanINIPath))
		{
			if (!CreateDirectory(csCustomScanINIPath, NULL))
			{
				AddLogEntry(L"### CScanDlg::Create WRDSETTINGS directory failed", 0, 0, true, SECONDLEVEL);
			}
		}
		m_dwFailedDeleteFileCount = 0;
		for (int iCount = 0; iCount < m_lstDrivePicker.GetItemCount(); iCount++)
		{
			CString szStr = m_lstDrivePicker.GetItemText(iCount, 0);
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", ++m_dwFailedDeleteFileCount);
			WritePrivateProfileString(L"Custom Scan Entry", szValueName, szStr, csCustomScanEntryINIPath);
		}

		ZeroMemory(szValueName, sizeof(szValueName));
		swprintf_s(szValueName, _countof(szValueName), L"%lu", m_lstDrivePicker.GetItemCount());
		WritePrivateProfileString(L"Custom Scan Entry", L"Count", szValueName, csCustomScanEntryINIPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::AddDeleteEditCustomScanEntryToINI", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : LoadCutomScanINI
*  Description    : Loads the entry for last custom scan from INI file
*  Author Name    : Nitin K.
*  SR_NO		  :
*  Date           : 18 March 2015
*************************************************************************************************/
void CScanDlg::LoadCutomScanINI()
{
	try
	{
		CString sz_Count = L"";
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L"### CScanDlg::AddDeleteEditCustomScanEntryToINI::GetModulePath failed", 0, 0, true, SECONDLEVEL);
			return;
		}

		TCHAR	m_szCustomINIPath[256];
		ZeroMemory(m_szCustomINIPath, sizeof(m_szCustomINIPath));
		swprintf_s(m_szCustomINIPath, _countof(m_szCustomINIPath), L"%s\\WRDSETTINGS\\WRDWIZCUSTOMSCANENTRY.INI", szModulePath);


		ZeroMemory(m_szCustomCount, sizeof(m_szCustomCount));

		GetPrivateProfileString(L"Custom Scan Entry", L"Count", L"", m_szCustomCount, 255, m_szCustomINIPath);
		m_iCustomCount = 0;
		m_iCustomCount = _ttoi(m_szCustomCount);
		for (int i = 1; i <= m_iCustomCount; i++)
		{
			sz_Count = L"";
			sz_Count.Format(L"%d", i);
			GetPrivateProfileString(L"Custom Scan Entry", sz_Count, L"", m_szCustomCount, 255, m_szCustomINIPath);
			if (PathFileExists(m_szCustomCount))
			{
				m_listCustomCount.Add((CString)m_szCustomCount);
			}
		}

		for (int iCount = 0; iCount < m_listCustomCount.GetCount(); iCount++)
		{
			CString str = m_listCustomCount.GetAt(iCount);
			m_lstDrivePicker.InsertItem(iCount, str);
			m_lstDrivePicker.SetCheck(iCount, TRUE);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::LoadCutomScanINI", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	SendRecoverOperations2Service
*  Description    :	Sends data to service to repair a file and service reply the status to GUI
*  SR.NO		  :
*  Author Name    : Vilas Suvarnakar
*  Date           : 07 April 2015
**********************************************************************************************************/
bool CScanDlg::SendFile4RepairUsingService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect)
{

	try
	{

		//CISpyCommunicator objCom(SERVICE_SERVER, bReconnect);

		if (!m_objCom.SendData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CGenXRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CGenXRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::SendFile4RepairUsingService1", 0, 0, true, SECONDLEVEL);
	}

	return true;
}


/***********************************************************************************************
*  Function Name  : CheckForRepairFileINIEntries
*  Description    : check whether repair file ini present or not
*  Author Name    : Vilas s
*  SR_NO		  :
*  Date           : 10 April 2015
*************************************************************************************************/
DWORD CScanDlg::CheckForRepairFileINIEntries()
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

/***********************************************************************************************
*  Function Name  : PauseScan
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 29/04/2015
*************************************************************************************************/
bool CScanDlg::PauseScan()
{
	try
	{
		CString csPauseResumeBtnText = L"";
		m_btnPauseResume.GetWindowText(csPauseResumeBtnText);
		if (csPauseResumeBtnText != theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"))
		{
			if (SendRequestCommon(PAUSE_SCAN))
			{
				//On WM_CLOSE call,UI will not get repaint after resuming scan as TIMER_SCAN_STATUS gets killed on pause.
				//Neha Gharge 13/5/2015
				//OnTimer(TIMER_PAUSE_HANDLER);
				m_bOnWMClose = true;
				m_tsScanPauseResumeTime = CTime::GetCurrentTime();
				SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_PAUSE"));
				AddLogEntry(L">>> Scanning Paused..", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### Failed to pause scan as Send PAUSE_SCAN request failed.", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::PauseScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : ResumeScan
*  Description    : Resume scanning if user click on stop/close button and click to 'No' for stop confirmation box.
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 29/04/2015
*************************************************************************************************/
bool CScanDlg::ResumeScan()
{
	try
	{
		CString csPauseResumeBtnText = L"";
		m_btnPauseResume.GetWindowText(csPauseResumeBtnText);
		if (csPauseResumeBtnText != theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME") && isScanning == TRUE)
		{
			if (SendRequestCommon(RESUME_SCAN))
			{
				//On WM_CLOSE call,UI will not get repaint after resuming scan as TIMER_SCAN_STATUS gets killed on pause.
				//Neha Gharge 13/5/2015
				if (m_bOnWMClose)
				{
					m_bOnWMClose = false;
				}
				SetTimer(TIMER_SCAN_STATUS, 1000, NULL);
				SetTimer(TIMER_FILESSCANNED_STATUS, 800, NULL);
				m_tsScanPauseResumeElapsedTime += ((CTime::GetCurrentTime() - m_tsScanPauseResumeTime));
				SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_RESUME"));
			}
			else
			{
				AddLogEntry(L"### Failed to pause scan as Send PAUSE_SCAN request failed.", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::ResumeScan.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : MemScanningFinished
*  Description    : Function which called by service using pipe communication when memory scan get finished.
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 29/04/2015
*************************************************************************************************/
bool CScanDlg::MemScanningFinished()
{
	try
	{
		KillTimer(TIMER_SCAN_STATUS);
		KillTimer(TIMER_FILESSCANNED_STATUS);

		if (m_iTotalFileCount)
		{
			m_prgScanProgress.SetRange32(0, m_iTotalFileCount);
		}

		if (theApp.m_eScanLevel == CLAMSCANNER)
		{
			m_prgScanProgress.SetWindowText(L"(" + theApp.m_objwardwizLangManager.GetString(L"IDS_SCANNING_FILES") + L") ");
		}

		CString csFileScanned;
		csFileScanned.Format(L"%s%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FILESCANNED"), L"0");
		m_csFilesScanned.SetWindowText(csFileScanned);

		m_prgScanProgress.SetPos(0);


		CString csScanStarted = L"";
		switch (m_eScanType)
		{
		case QUICKSCAN:
			csScanStarted = theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_QSCANNER_LAUNCH");
			break;
		case FULLSCAN:
			csScanStarted = theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_FSCANNER_LAUNCH");
			break;
		case CUSTOMSCAN:
			csScanStarted = theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_CSCANNER_LAUNCH");
			break;
		default:
			csScanStarted = theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCANNER_LAUNCH");
			break;
		}

		SetScanStatus(csScanStarted);

		m_bIsMemScan = false;
		m_FileScanned = 0;

		SetTimer(TIMER_SCAN_STATUS, 1000, NULL);
		SetTimer(TIMER_FILESSCANNED_STATUS, 800, NULL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::MemScanningFinished", 0, 0, true, SECONDLEVEL);
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
bool CScanDlg::SaveLocalDatabase()
{
	__try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SAVE_LOCALDB;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CUSBDetectUIDlg::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
		}

		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
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
		AddLogEntry(L"### Exception in CScanDlg::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
	}
	return false;
}