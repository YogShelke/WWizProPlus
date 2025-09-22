// ISpyAntiRootkit.cpp : implementation file
//
/*******************************************************************************
*  Program Name: ISpyAntiRootkit.cpp                                                                                                    
*  Description: Scan Antirootkit                                                                                                          
*  Author Name: Prajakta & Neha                                                                                                     
*  Date Of Creation: 21 Jan 2014
*  Version No: 1.0.0.2
*******************************************************************************/

#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "ISpyAntiRootkit.h"


#define TIMER_ROOTKIT_SCAN_STATUS			100
#define TIMER_PAUSE_HANDLER					101 //Varada Ikhar, Date:27/01/2015, Adding 'Total Scan Time' in the log.
#define DETECTEDSTATUS			L"Detected"
#define DELETEDSTATUS			L"Quarantined"
#define FAILEDSTATUS 			L"Failed"
#define FILEREPAIRED 			L"Repaired"
bool					g_bIsRootkitScanning;
bool                    g_bIsRootkitCleaning;
#define BAIL_OUT(code) { g_bIsRootkitScanning = false; return code; }

CISpyGUIDlg *g_TabCtrlRtktWindHandle = NULL;
DWORD WINAPI DeleteThread(LPVOID lpParam);
DWORD WINAPI GetPercentage(LPVOID lpvThreadParam);
DWORD WINAPI VirusFoundEntriesInAntirootThread(LPVOID lpvThreadParam);
// CISpyAntiRootkit dialog

IMPLEMENT_DYNAMIC(CISpyAntiRootkit, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CISpyAntiRootkit                                                     
*  Description    :	C'tor
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
CISpyAntiRootkit::CISpyAntiRootkit(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyAntiRootkit::IDD, pParent)
	, m_FontText(NULL)
	, m_bScanningFinished(false)
	, m_dwScannedCnt(0)
	, m_dwScannedFileFolder(0)
	, m_dwScannedCntRegistry(0)
	, m_dwThreatCnt(0)
	, m_dwThreatCntRegistry(0)
	, m_dwThreatCntFilefolder(0)
	, m_bProcessTabSelected(false)
	, m_bRegistryTabSelected(false)
	, m_bFilesFoldersTabSelected(false)
	, m_bRootkitDeleted(false)
	, m_bScanAborted(false)
	, m_bScanningStopped(false)
	, m_bAntirootClose(false)
	, m_bAntirootkitHome(false)
	, m_hGetPercentage(NULL)
	, m_bRedFlag(false)
	, m_csPreviousStatus(L"")
	,m_dwType(0)
	,m_dwTotalType(0)
	,m_icount(0)
	,m_objIPCRootkitClient(ROOTKIT)
	,m_dwGetCheckValues(0)
	,m_bCheckProcess(false)
	,m_bCheckRegistry(false)
	,m_bCheckFileFolder(false)
	,m_bAntirootScanningInProgress(false)
	,m_bAntirootkitCompleted(false)
	, m_bAntirootkitScan(false)
	, m_bOnWMClose(false)
	, m_bIsPopUpDisplayed(false)
{

}

/**********************************************************************************************************                     
*  Function Name  :	~CISpyAntiRootkit                                                     
*  Description    :	D'tor
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
CISpyAntiRootkit::~CISpyAntiRootkit()
{
	if(m_FontText != NULL)
	{
		delete m_FontText;
		m_FontText = NULL;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_BTNBACK, m_btnBack);
	DDX_Control(pDX, IDC_PIC_ANTIROOTKIT_HEADER, m_stHAntiRootkit);
	DDX_Control(pDX, IDC_BTN_SCAN, m_btnScan);
// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_CHK_PROCESS, m_chkProcess);
	DDX_Control(pDX, IDC_CHK_REGISTRY, m_chkRegistry);
	DDX_Control(pDX, IDC_CHK_FILESFOLDERS, m_chkFilesFolders);
	DDX_Control(pDX, IDC_STATIC_PROCESS, m_stProcess);
	DDX_Control(pDX, IDC_STATIC_REGISTRY, m_stRegistry);
	DDX_Control(pDX, IDC_STATIC_FILESFOLDERS, m_stFilesFolders);
	DDX_Control(pDX, IDC_STATIC_SELECTIONTEXT, m_stSelectionText);
	DDX_Control(pDX, IDC_PROGRESS_ANTIROOTKIT, m_prgAntiRootkit);
	DDX_Control(pDX, IDC_STATIC_PERCENTAGE, m_stPercentage);
	DDX_Control(pDX, IDC_EDIT_ANTIROOTKIT, m_edtAntiRootkitStatus);
	DDX_Control(pDX, IDC_TAB_PROCESS, m_tabAntiRootkit);
	DDX_Control(pDX, IDC_LIST_ANTIROOTKIT, m_lstAntiRookit);
	DDX_Control(pDX, IDC_BTNANTIROOTKIT_DELETE, m_btnAntiRootkitDelete);
	DDX_Control(pDX, IDC_CHECK_ANTIROOTKITSELECTALL, m_chkAntiRootkitSelectAll);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKITSELECTALL, m_stAntiRootkitSelectAll);
	DDX_Control(pDX, IDC_LIST_REGISTRY, m_lstRegistry);
	DDX_Control(pDX, IDC_LIST_FILESFOLDERS, m_lstFilesFolders);
	DDX_Control(pDX, IDC_STATIC_SCANNEDENTRIES, m_stScannedEntries);
	DDX_Text(pDX, IDC_STATIC_SCANNEDENTRIES, m_csScannedEntries);
	DDX_Control(pDX, IDC_STATIC_THREATSFOUND, m_stThreatsFound);
	DDX_Text(pDX, IDC_STATIC_THREATSFOUND, m_csThreatsfound);
	DDX_Control(pDX, IDC_STATIC_SCANNEDREGISTRY, m_stScannedRegistry);
	DDX_Control(pDX, IDC_STATIC_THREATSREGISTRY, m_stThreatsRegistry);
	DDX_Control(pDX, IDC_STATIC_SCANNEDFILES, m_stScannedFilesFolders);
	DDX_Control(pDX, IDC_STATIC_THREATSFILES, m_stThreatsFilesFolders);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKIT_HEADER, m_stAntirootkitHeader);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKIT_SUBHEADER, m_stAntirootkitSubHeader);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKIT_STATUS, m_csScannedText);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKIT_SUBSTATUS, m_csScannedSubText);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKIT_SCAN_REG, m_stRegisteryText);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKIT_DETECTED_REG, m_stRegistrySubText);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKIT_SCAN_FILE, m_stFileText);
	DDX_Control(pDX, IDC_STATIC_ANTIROOTKIT_DETECTED_FILES, m_stFilesSubText);
	DDX_Control(pDX, IDC_CHECK_FOR_WINDOWREGISTRY, m_chkWindowRegistry);
	DDX_Control(pDX, IDC_CHECK_FILEFOLDER, m_chkFileFolder);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_btnBackButton);
}

/**********************************************************************************************************                     
*  Function Name  :	MESSAGE_MAP                                                     
*  Description    :	Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
BEGIN_MESSAGE_MAP(CISpyAntiRootkit, CJpegDialog)
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	//ON_BN_CLICKED(IDC_BTNBACK, &CISpyAntiRootkit::OnBnClickedBtnback)
	ON_BN_CLICKED(IDC_BTN_SCAN, &CISpyAntiRootkit::OnBnClickedBtnScan)
	// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.
	ON_BN_CLICKED(IDC_BTN_STOP, &CISpyAntiRootkit::OnBnClickedBtnStop)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PROCESS, &CISpyAntiRootkit::OnTcnSelchangeTabProcess)
	ON_BN_CLICKED(IDC_BTNANTIROOTKIT_DELETE, &CISpyAntiRootkit::OnBnClickedBtnantirootkitDelete)
	ON_BN_CLICKED(IDC_CHECK_ANTIROOTKITSELECTALL, &CISpyAntiRootkit::OnBnClickedCheckAntirootkitselectall)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_ANTIROOTKIT, &CISpyAntiRootkit::OnNMCustomdrawListAntirootkit)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_REGISTRY, &CISpyAntiRootkit::OnNMCustomdrawListRegistry)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_FILESFOLDERS, &CISpyAntiRootkit::OnNMCustomdrawListFilesfolders)
	ON_BN_CLICKED(IDC_CHECK_FOR_WINDOWREGISTRY, &CISpyAntiRootkit::OnBnClickedCheckForWindowregistry)
	ON_BN_CLICKED(IDC_CHECK_FILEFOLDER, &CISpyAntiRootkit::OnBnClickedCheckFilefolder)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CISpyAntiRootkit::OnBnClickedButtonBack)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CISpyAntiRootkit message handlers
/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                    
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure
					common to all Microsoft Foundation Class Library dialog boxes
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
BOOL CISpyAntiRootkit::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_REGOPT_BG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_FAILMSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	CRect rect1;
	this->GetClientRect(rect1);
	SetWindowPos(NULL, 1, 88, rect1.Width()-5, rect1.Height() - 5, SWP_NOREDRAW);
	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	//Set MainDlg Ptr
	g_TabCtrlRtktWindHandle =(CISpyGUIDlg*)AfxGetMainWnd();	

	/*****************ISSUE NO -299,300 Neha Gharge 22/5/14 ************************************************/

	//m_stAntirootkitHeader.SetBkColor(RGB(230,232,238));
    m_stAntirootkitHeader.SetTextColor(RGB(24,24,24));
   	m_stAntirootkitHeader.SetWindowPos(&wndTop,rect1.left +20,07,400,31,SWP_NOREDRAW);
	m_stAntirootkitHeader.SetFont(&theApp.m_fontWWTextSmallTitle);

   // m_stAntirootkitSubHeader.SetBkColor(RGB(230,232,238));
	m_stAntirootkitSubHeader.SetTextColor(RGB(24,24,24));
	m_stAntirootkitSubHeader.SetWindowPos(&wndTop,rect1.left +20,35,400,18,SWP_NOREDRAW);
    m_stAntirootkitSubHeader.SetFont(&theApp.m_fontWWTextSubTitleDescription);

	if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	{
		m_stAntirootkitHeader.SetWindowPos(&wndTop,rect1.left +20,14,400,31,SWP_NOREDRAW);
		m_stAntirootkitSubHeader.SetWindowPos(&wndTop,rect1.left +20,38,400,18,SWP_NOREDRAW);
	}


	m_bmpAntiRootkit = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stHAntiRootkit.SetWindowPos(&wndTop,rect1.left +6,10,582,45,SWP_NOREDRAW);
	m_stHAntiRootkit.SetBitmap(m_bmpAntiRootkit);

	m_FontText = new CFont;
	m_FontText->CreatePointFont(100, _T("Verdana"));
	
	m_stSelectionText.SetWindowPos(&wndTop, rect1.left +50, 105, 450,23, SWP_NOREDRAW);
	//m_stSelectionText.SetWindowTextW(L"Select rootkit scan option");
	m_stSelectionText.SetTextColor(RGB(0, 0, 0));
	m_stSelectionText.SetFont(&theApp.m_fontWWTextSubTitle);

	m_chkProcess.SetWindowPos(&wndTop, rect1.left +50, 230, 15, 15, SWP_NOREDRAW);
	m_chkProcess.SetCheck(true);
	//m_stProcess.SetWindowText(L"Hidden items in running processes");
	m_stProcess.SetTextColor(RGB(0, 0, 0));
	m_stProcess.SetWindowPos(&wndTop, rect1.left +75, 228, 370, 23, SWP_NOREDRAW);
	//m_stProcess.SetFont(&theApp.m_fontWWTextNormal);
	m_stProcess.SetFont(&theApp.m_fontWWTextMediumSize);

	m_chkRegistry.SetWindowPos(&wndTop, rect1.left +50, 190, 15, 15, SWP_NOREDRAW);
	m_chkRegistry.SetCheck(true);
	//m_stRegistry.SetWindowText(L"Hidden items in windows registry");
	m_stRegistry.SetTextColor(RGB(0, 0, 0));
	m_stRegistry.SetWindowPos(&wndTop, rect1.left +75, 188, 370, 23, SWP_NOREDRAW);
	//m_stRegistry.SetFont(&theApp.m_fontWWTextNormal);
	m_stRegistry.SetFont(&theApp.m_fontWWTextMediumSize);

	m_chkFilesFolders.SetWindowPos(&wndTop, rect1.left +50, 150, 15, 15, SWP_NOREDRAW);
	m_chkFilesFolders.SetCheck(true);
	/*m_stFilesFolders.SetWindowText(L"Hidden items in files and folders");*/
	m_stFilesFolders.SetTextColor(RGB(0, 0, 0));
	m_stFilesFolders.SetWindowPos(&wndTop, rect1.left +75, 148, 370, 23, SWP_NOREDRAW);
	//m_stFilesFolders.SetFont(&theApp.m_fontWWTextNormal);
	m_stFilesFolders.SetFont(&theApp.m_fontWWTextMediumSize);

	/*****************ISSUE NO -192 Neha Gharge 22/5/14 ************************************************/
	m_btnScan.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
/*In Antirootkit scan, Space between progress bar and scan button should be less.
	Niranjan Deshak - 27/02/2015*/
	m_btnScan.SetWindowPos(&wndTop, rect1.left + 468,325,57,21, SWP_NOREDRAW);
	/*m_btnScan.SetWindowTextW(L"Scan");*/
	m_btnScan.SetFont(&theApp.m_fontWWTextNormal);
	m_btnScan.SetTextColorA(BLACK,1,1);
	m_btnScan.ShowWindow(true);
	// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.
	m_btnStop.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnStop.SetWindowPos(&wndTop, rect1.left +533,325,57,21, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	/*m_btnPauseResume.SetWindowTextW(L"Pause");*/
	m_btnStop.SetTextColorA(BLACK,1,1);
	m_btnStop.ShowWindow(true);
	m_btnStop.EnableWindow(false);
	m_btnStop.SetFont(&theApp.m_fontWWTextNormal);

	m_edtAntiRootkitStatus.SetBkColor(RGB(70,70,70));
	m_edtAntiRootkitStatus.SetTextColor(WHITE);
	m_edtAntiRootkitStatus.SetWindowPos(&wndTop, rect1.left +6, 360, 510 , 20, SWP_NOREDRAW);
	m_edtAntiRootkitStatus.SetFont(&theApp.m_fontWWTextNormal);

	/*In Antirootkit scan, Space between progress bar and scan button should be less.
	Niranjan Deshak - 27/02/2015*/
	m_prgAntiRootkit.SetWindowPos(&wndTop, rect1.left + 6, 325, 455, 21, SWP_SHOWWINDOW);
	m_prgAntiRootkit.SetFont(&theApp.m_fontText);

	m_prgAntiRootkit.AlignText(DT_CENTER);
	m_prgAntiRootkit.SetBarColor(RGB(171,238,0));
	//m_prgAntiRootkit.SetBkColor(RGB(243,239,238));
	m_prgAntiRootkit.SetShowPercent(true);

	m_stPercentage.ShowWindow(SW_HIDE);
	//m_btnBack.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	//m_btnBack.SetWindowPos(&wndTop,+21,354,31,32,SWP_NOREDRAW);
	//m_btnBack.ShowWindow(SW_HIDE);
	SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANSTATUS"));

	m_tabAntiRootkit.SetWindowPos(&wndTop, rect1.left +9,76,582,23,SWP_NOREDRAW);
	m_tabAntiRootkit.ShowWindow(false);
	m_tabAntiRootkit.EnableWindow(false);
	m_tabAntiRootkit.SetFont(&theApp.m_fontWWTextNormal);
	m_tabAntiRootkit.SetCurSel(0);

	TC_ITEM TabCtrlItem;
	TabCtrlItem.mask = TCIF_IMAGE | TCIF_TEXT;
	//TabCtrlItem.pszText = L"Processes";
	CString csItem = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_PROCESSES");
	LPWSTR lpwWinReg = (LPWSTR)csItem.GetBuffer();
	TabCtrlItem.pszText = lpwWinReg;
	m_tabAntiRootkit.InsertItem(0,&TabCtrlItem);
	m_tabAntiRootkit.SetItemSize(191);
	//m_tabAntiRootkit.SetFont(m_FontText);
	//TC_ITEM TabCtrlItem;
	//LPDRAWITEMSTRUCT lpdis;
	//TabCtrlItem.mask = TCIF_TEXT;
    //CString str= theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_PROCESSES");;
	//TabCtrlItem.pszText = CT2W(str);
	////TabCtrlItem.pszText= _tcsc
	//m_tabAntiRootkit.InsertItem(0,&TabCtrlItem);
	//m_tabAntiRootkit.SetItemSize(195);
	//m_tabAntiRootkit.SetFont(m_FontText);

	csItem = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_WINREG");
	lpwWinReg = (LPWSTR)csItem.GetBuffer();
	TabCtrlItem.pszText = lpwWinReg;
	m_tabAntiRootkit.InsertItem(1,&TabCtrlItem);
	m_tabAntiRootkit.SetItemSize(192);

	csItem = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_FILESFOLDERS");
	lpwWinReg = (LPWSTR)csItem.GetBuffer();
	TabCtrlItem.pszText = lpwWinReg;
	m_tabAntiRootkit.InsertItem(2,&TabCtrlItem);
	m_tabAntiRootkit.SetItemSize(190);
   /* str = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_WINREG");
	TabCtrlItem.pszText = CT2W(str);
	m_tabAntiRootkit.InsertItem(1,&TabCtrlItem);
	m_tabAntiRootkit.SetItemSize(195);
	str = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_FILESFOLDERS");
	TabCtrlItem.pszText = CT2W(str);
	m_tabAntiRootkit.InsertItem(2,&TabCtrlItem);
	m_tabAntiRootkit.SetItemSize(195);*/

	m_lstAntiRookit.InsertColumn(0, L"PID", LVCFMT_LEFT,70);
	m_lstAntiRookit.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_PROCNAME"), LVCFMT_LEFT, 120);
	m_lstAntiRookit.InsertColumn(2, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_PROCPATH"), LVCFMT_LEFT, 250);
	m_lstAntiRookit.InsertColumn(3, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ACTION"), LVCFMT_LEFT, 138);
	m_lstAntiRookit.SetWindowPos(&wndTop, rect1.left +9,100,582,197,SWP_NOREDRAW);
	ListView_SetExtendedListViewStyle (m_lstAntiRookit.m_hWnd, LVS_EX_CHECKBOXES |  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	m_lstAntiRookit.ShowWindow(false);
	CHeaderCtrl* pHeaderCtrl = m_lstAntiRookit.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);

	m_lstRegistry.InsertColumn(0, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SERVNAME"), LVCFMT_LEFT, 190);
	m_lstRegistry.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SERVPATH"), LVCFMT_LEFT,250);
	//m_lstRegistry.InsertColumn(2, L"Registry Data", LVCFMT_LEFT, 194);
	m_lstRegistry.InsertColumn(2, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ACTION"), LVCFMT_LEFT, 138);
	m_lstRegistry.SetWindowPos(&wndTop, rect1.left +9,100,582,197,SWP_NOREDRAW);
	ListView_SetExtendedListViewStyle (m_lstRegistry.m_hWnd, LVS_EX_CHECKBOXES |  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	m_lstRegistry.ShowWindow(false);

	pHeaderCtrl = m_lstRegistry.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);

	m_lstFilesFolders.InsertColumn(0,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_FILEPATH"), LVCFMT_LEFT,440);
	//m_lstFilesFolders.InsertColumn(1, L"File/Folder Name", LVCFMT_LEFT,290);
	m_lstFilesFolders.InsertColumn(1,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ACTION"), LVCFMT_LEFT,138);
	m_lstFilesFolders.SetWindowPos(&wndTop, rect1.left +9,100,582,197,SWP_NOREDRAW);
	ListView_SetExtendedListViewStyle (m_lstFilesFolders.m_hWnd, LVS_EX_CHECKBOXES |  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	m_lstFilesFolders.ShowWindow(false);
	
	pHeaderCtrl = m_lstFilesFolders.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);

	/*****************ISSUE NO -79,374 Neha Gharge 22/5/14 ************************************************/

	m_chkAntiRootkitSelectAll.SetWindowPos(&wndTop, rect1.left +8, 307, 13 , 13, SWP_NOREDRAW);
	m_chkAntiRootkitSelectAll.ShowWindow(SW_HIDE);
	m_chkAntiRootkitSelectAll.SetCheck(true);
	m_stAntiRootkitSelectAll.SetWindowPos(&wndTop,rect1.left +27, 307, 120, 17, SWP_NOREDRAW);
	m_stAntiRootkitSelectAll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SELECTALL"));
	m_stAntiRootkitSelectAll.SetTextColor(RGB(255,255,255));

	
	m_chkFileFolder.SetWindowPos(&wndTop, rect1.left +8, 307, 13 , 13, SWP_NOREDRAW);
	m_chkFileFolder.ShowWindow(SW_HIDE);
	m_chkFileFolder.SetCheck(true);

	m_chkWindowRegistry.SetWindowPos(&wndTop, rect1.left +8, 307, 13 , 13, SWP_NOREDRAW);
	m_chkWindowRegistry.ShowWindow(SW_HIDE);
	m_chkWindowRegistry.SetCheck(true);
	//m_stAntiRootkitSelectAll.SetBkColor(RGB(88,88,90));
	m_stAntiRootkitSelectAll.ShowWindow(false);
	m_stAntiRootkitSelectAll.SetFont(&theApp.m_fontWWTextNormal);

	m_csScannedText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_PROC"));
    //m_csScannedText.SetBkColor(RGB(88,88,90));
	m_csScannedText.SetTextColor(RGB(255,255,255));
	m_csScannedText.SetWindowPos(&wndTop, rect1.left +8, 330, 150, 30, SWP_NOREDRAW);
	m_csScannedText.ShowWindow(false);
	m_csScannedText.SetFont(&theApp.m_fontWWTextNormal);
	
	//m_stScannedEntries.SetBkColor(RGB(88,88,90));
	m_stScannedEntries.SetTextColor(RGB(255,255,255));
	m_stScannedEntries.SetWindowPos(&wndTop, rect1.left +165, 330, 140, 17, SWP_NOREDRAW);
	m_stScannedEntries.ShowWindow(false);
	m_csScannedEntries.Format(L": %d",m_dwScannedCnt);
	m_stScannedEntries.SetWindowTextW(m_csScannedEntries);
	m_stScannedEntries.SetFont(&theApp.m_fontWWTextNormal);

	m_csScannedSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_PROC"));
    //m_csScannedSubText.SetBkColor(RGB(88,88,90));
	m_csScannedSubText.SetTextColor(RGB(255,255,255));
	m_csScannedSubText.SetWindowPos(&wndTop, rect1.left +8, 355, 150, 30, SWP_NOREDRAW);
	m_csScannedSubText.ShowWindow(false);
	m_csScannedSubText.SetFont(&theApp.m_fontWWTextNormal);

	
	//m_stThreatsFound.SetBkColor(RGB(88,88,90));
	m_stThreatsFound.SetTextColor(RGB(255,255,255));
	m_stThreatsFound.SetWindowPos(&wndTop, rect1.left +165, 355, 140, 25, SWP_NOREDRAW);
	m_stThreatsFound.ShowWindow(false);
	m_csThreatsfound.Format(L": %d",m_dwThreatCnt);
	m_stThreatsFound.SetWindowTextW(m_csThreatsfound);
	m_stThreatsFound.SetFont(&theApp.m_fontWWTextNormal);

	m_stRegisteryText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_REG"));
    //m_stRegisteryText.SetBkColor(RGB(88,88,90));
	m_stRegisteryText.SetTextColor(RGB(255,255,255));
	m_stRegisteryText.SetWindowPos(&wndTop, rect1.left +8, 330, 190, 30, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	m_stRegisteryText.ShowWindow(false);
	m_stRegisteryText.SetFont(&theApp.m_fontWWTextNormal);

	//m_stScannedRegistry.SetBkColor(RGB(88,88,90));
	m_stScannedRegistry.SetTextColor(RGB(255,255,255));
	m_stScannedRegistry.SetWindowPos(&wndTop, rect1.left +185, 330, 140, 25, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	m_stScannedRegistry.ShowWindow(false);
	m_stScannedRegistry.SetFont(&theApp.m_fontWWTextNormal);

	m_stRegistrySubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_REG"));
    //m_stRegistrySubText.SetBkColor(RGB(88,88,90));
	m_stRegistrySubText.SetTextColor(RGB(255,255,255));
	m_stRegistrySubText.SetWindowPos(&wndTop, rect1.left +8, 355, 190, 30, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	m_stRegistrySubText.ShowWindow(false);
	m_stRegistrySubText.SetFont(&theApp.m_fontWWTextNormal);

	//m_stThreatsRegistry.SetBkColor(RGB(88,88,90));
	m_stThreatsRegistry.SetTextColor(RGB(255,255,255));
	m_stThreatsRegistry.SetWindowPos(&wndTop, rect1.left +185, 355, 140, 25, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	m_stThreatsRegistry.ShowWindow(false);
	m_stThreatsRegistry.SetFont(&theApp.m_fontWWTextNormal);

	m_stFileText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_FILE"));
   // m_stFileText.SetBkColor(RGB(88,88,90));
	m_stFileText.SetTextColor(RGB(255,255,255));
	m_stFileText.SetWindowPos(&wndTop, rect1.left +8, 330, 190, 25, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	m_stFileText.ShowWindow(false);
	m_stFileText.SetFont(&theApp.m_fontWWTextNormal);

	//m_stScannedFilesFolders.SetBkColor(RGB(88,88,90));
	m_stScannedFilesFolders.SetTextColor(RGB(255,255,255));
	m_stScannedFilesFolders.SetWindowPos(&wndTop, rect1.left +168, 330, 140, 25, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	m_stScannedFilesFolders.SetFont(&theApp.m_fontWWTextNormal);
	m_stScannedFilesFolders.ShowWindow(false);

	m_stFilesSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_FILE"));
    //m_stFilesSubText.SetBkColor(RGB(88,88,90));
	m_stFilesSubText.SetTextColor(RGB(255,255,255));
	m_stFilesSubText.SetWindowPos(&wndTop, rect1.left +8, 355, 190, 30, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	m_stFilesSubText.SetFont(&theApp.m_fontWWTextNormal);
	m_stFilesSubText.ShowWindow(false);

	//m_stThreatsFilesFolders.SetBkColor(RGB(88,88,90));
	m_stThreatsFilesFolders.SetTextColor(RGB(255,255,255));
	m_stThreatsFilesFolders.SetWindowPos(&wndTop, rect1.left +168, 355, 140, 17, SWP_NOREDRAW);//Not merged in latest code.Neha Gharge
	m_stThreatsFilesFolders.ShowWindow(false);
	m_stThreatsFilesFolders.SetFont(&theApp.m_fontWWTextNormal);

	/*****************ISSUE NO -192 Neha Gharge 22/5/14 ************************************************/
	m_btnAntiRootkitDelete.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnAntiRootkitDelete.SetWindowPos(&wndTop, rect1.left +533,350,57,21, SWP_NOREDRAW);
	m_btnAntiRootkitDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CLEAN"));
	m_btnAntiRootkitDelete.SetTextColorA(BLACK,1,1);
	m_btnAntiRootkitDelete.ShowWindow(false);
	m_btnAntiRootkitDelete.SetFont(&theApp.m_fontWWTextNormal);

	//ISSUE NO -337 rajil Y 23/5/14 
	m_btnBackButton.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnBackButton.SetWindowPos(&wndTop, rect1.left +466,350,57,21, SWP_NOREDRAW);
	m_btnBackButton.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_BACK"));
	m_btnBackButton.SetFont(&theApp.m_fontWWTextNormal);
	m_btnBackButton.SetTextColorA(BLACK,1,1);

	//Issue : Click any scan->the message below the progress bar should not shift from one side to other
	//if we right click & select 'right to left' option.
	//Resolved By : Nitin K  Date : 10th March 2015
	GetDlgItem(IDC_EDIT_ANTIROOTKIT)->ModifyStyle(0, WS_DISABLED);
	RefreshStrings();
	return TRUE; 
}

/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage                                                     
*  Description    :	Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
BOOL CISpyAntiRootkit::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within the CWnd object.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
BOOL CISpyAntiRootkit::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BTN_BACK ||
		iCtrlID == IDC_BTN_SCAN ||
		// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.
		iCtrlID == IDC_BTN_STOP ||
		iCtrlID == IDC_BTNANTIROOTKIT_DELETE ||
		iCtrlID == IDC_CHK_PROCESS ||
		iCtrlID == IDC_CHK_REGISTRY ||
		iCtrlID == IDC_CHK_FILESFOLDERS ||
		iCtrlID == IDC_BUTTON_BACK
		)
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
*  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that contains the cursor (or the CWnd object that used the SetCapture member function to capture the mouse input) every time the mouse is moved.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
LRESULT CISpyAntiRootkit::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
HBRUSH CISpyAntiRootkit::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_PROCESS      ||
		ctrlID == IDC_STATIC_REGISTRY   ||
		ctrlID == IDC_STATIC_FILESFOLDERS ||
		ctrlID == IDC_STATIC_SELECTIONTEXT ||
		ctrlID == IDC_STATIC_SCANNEDENTRIES ||
		ctrlID == IDC_STATIC_THREATSFOUND  ||
		ctrlID == IDC_STATIC_ANTIROOTKIT_HEADER||
		ctrlID == IDC_STATIC_ANTIROOTKIT_SUBHEADER||
		ctrlID == IDC_STATIC_ANTIROOTKITSELECTALL ||
		ctrlID == IDC_STATIC_SCANNEDREGISTRY ||
		ctrlID == IDC_STATIC_THREATSREGISTRY ||
		ctrlID == IDC_STATIC_SCANNEDFILES ||
		ctrlID == IDC_STATIC_THREATSFILES ||
		ctrlID == IDC_STATIC_ANTIROOTKIT_STATUS||
		ctrlID == IDC_STATIC_ANTIROOTKIT_SUBSTATUS||
		ctrlID == IDC_STATIC_ANTIROOTKIT_SCAN_REG||
		ctrlID == IDC_STATIC_ANTIROOTKIT_DETECTED_REG||
		ctrlID == IDC_STATIC_ANTIROOTKIT_SCAN_FILE||
		ctrlID == IDC_STATIC_ANTIROOTKIT_DETECTED_FILES ||
		ctrlID == IDC_BUTTON_BACK

		)

	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} 
	if(	ctrlID  == IDC_EDIT_ANTIROOTKIT)
	{
		if(m_bRedFlag)
		{
			pDC->SetTextColor(RGB(255, 0, 0));
		}
		//pDC->SetBkMode(TRANSPARENT);
	}
	else if ( ctrlID  == IDC_EDIT_ANTIROOTKIT )
	{
		//CBrush m_brHollow;
		//m_brHollow.CreateStockObject(HOLLOW_BRUSH);
		//pDC->SetBkColor(RGB(255,255,255));
		//hbr = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}	
	return hbr;
}



//void CISpyAntiRootkit::OnBnClickedBtnback()
//{
//	m_lstFilesFolders.DeleteAllItems();
//	m_lstRegistry.DeleteAllItems();
//	m_lstAntiRookit.DeleteAllItems();
//	ShowHideControls();
//	HideChildControls(false);
//	this->ShowWindow(SW_HIDE);
//	//CISpyGUIDlg *pObjMainUI = reinterpret_cast<CISpyGUIDlg*>( this->GetParent() );
//	//if(pObjMainUI)
//	//{
//		//pObjMainUI->ShowHideMainPageControls(true);
//	//}
//}

/**********************************************************************************************************                     
*  Function Name  :	ShowHideControls                                                     
*  Description    :	Show and hide controls
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::ShowHideControls(bool bEnable)
{
	m_chkProcess.SetCheck(bEnable);
	m_chkRegistry.SetCheck(bEnable);
	m_chkFilesFolders.SetCheck(bEnable);
	//m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
	//m_btnScan.EnableWindow(true);
// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.

	m_btnStop.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"));
	m_btnStop.EnableWindow(false);
	m_prgAntiRootkit.SetPos(0);
	//m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
	//m_btnBack.EnableWindow(true);
	SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANSTATUS"));
}

/**********************************************************************************************************                     
*  Function Name  :	HideControls4ScannedList                                                     
*  Description    :	Hide controls on the basic of bEnable flag.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::HideControls4ScannedList(bool bEnable)
{
	m_stSelectionText.ShowWindow(bEnable);
	m_chkProcess.ShowWindow(bEnable);
	m_stProcess.ShowWindow(bEnable);
	m_chkRegistry.ShowWindow(bEnable);
	m_stRegistry.ShowWindow(bEnable);
	m_chkFilesFolders.ShowWindow(bEnable);
	m_stFilesFolders.ShowWindow(bEnable);
	m_prgAntiRootkit.ShowWindow(bEnable);
	m_edtAntiRootkitStatus.ShowWindow(bEnable);
	m_btnScan.ShowWindow(bEnable);
// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.

	m_btnStop.ShowWindow(bEnable);
	Invalidate();
	//m_prgAntiRootkit.SetPos(0);
	
}
/**********************************************************************************************************                     
*  Function Name  :	HideChildControls                                                     
*  Description    :	Hide child winodows.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::HideChildControls(bool bEnable)
{
	m_btnBackButton.ShowWindow(bEnable);
	m_csScannedSubText.ShowWindow(bEnable);
	m_csScannedText.ShowWindow(bEnable);
	m_lstAntiRookit.DeleteAllItems();
	m_lstRegistry.DeleteAllItems();
	m_lstFilesFolders.DeleteAllItems();
	m_bProcessTabSelected = false;
	m_bFilesFoldersTabSelected = false;
	m_bRegistryTabSelected = false;
	m_tabAntiRootkit.ShowWindow(bEnable);
	m_lstAntiRookit.ShowWindow(bEnable);
	m_lstRegistry.ShowWindow(bEnable);
	m_lstFilesFolders.ShowWindow(bEnable);
	/*****************ISSUE NO -79 Neha Gharge 22/5/14 ************************************************/
	m_chkAntiRootkitSelectAll.ShowWindow(bEnable);
	m_chkWindowRegistry.ShowWindow(bEnable);
	m_chkFileFolder.ShowWindow(bEnable);
	m_stAntiRootkitSelectAll.ShowWindow(bEnable);
	m_btnAntiRootkitDelete.ShowWindow(bEnable);
	m_stThreatsFound.ShowWindow(bEnable);
	m_stScannedEntries.ShowWindow(bEnable);
	m_stScannedRegistry.ShowWindow(bEnable);
	m_stThreatsRegistry.ShowWindow(bEnable);
	m_stScannedFilesFolders.ShowWindow(bEnable);
	m_stThreatsFilesFolders.ShowWindow(bEnable);
	m_stFileText.ShowWindow(bEnable);
	m_stFilesSubText.ShowWindow(bEnable);
	m_stRegisteryText.ShowWindow(bEnable);
	m_stRegistrySubText.ShowWindow(bEnable);
	Invalidate();
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnScan                                                     
*  Description    :	if clicks on scan button,it will start scanning and change button to stop. If stop, 
					it will stop scanning.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnBnClickedBtnScan()
{
	try
	{


		//Varada Ikhar, Date:17-03-2015, 
		//Issue:User updated the product and does not restarted, and started a scan, then it should show message as 
		//"Product updated, restart is required to take a effect."
		if (!theApp.ShowRestartMsgOnProductUpdate())
		{
			return;
		}
		//Varada Ikhar, Date:02-03-2015, Issue: Pause-Resume
		m_btnScan.EnableWindow(false);

		//if(!m_chkProcess.GetCheck() || !m_chkRegistry.GetCheck() || !m_chkFilesFolders.GetCheck())
		//{
		//	MessageBox(L"Please select rootkit scan option", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		//}
	    m_bAntirootkitScan=true;// Amit Dutta
		CString csBtnText;
		m_btnScan.GetWindowText(csBtnText);
		// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.
		if(csBtnText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"))
		{
			if(SendRequestCommon(RESUME_SCAN))
			{
				SetTimer(TIMER_ROOTKIT_SCAN_STATUS, 1000, NULL);

				//Varada Ikhar, Date:27/01/2015, Adding 'Total Scan Time' in the log.
				m_tsScanPauseResumeElapsedTime += ((CTime::GetCurrentTime() - m_tsScanPauseResumeTime));

				SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANRESUME"));

				Sleep(50); //Varada Ikhar, Date:19/02/2015, Issue: In Win-Vista32 bit, if pause-resume pressed continuously, progress bar show wrong value.
				AddLogEntry(L">>> Scanning Resumed..", 0, 0, true, ZEROLEVEL); //Varada
				m_btnScan.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
				m_btnStop.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"));
				m_btnStop.EnableWindow(true);
				//m_btnBack.EnableWindow(false);
				m_btnScan.EnableWindow(true);
				
			}

		}
		
		else if(csBtnText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"))
		{
			
			if(SendRequestCommon(PAUSE_SCAN))
			{
				//Varada Ikhar, Date:27/01/2015, Adding 'Total Scan Time' in the log.
				OnTimer(TIMER_PAUSE_HANDLER);

				//Kill the timer here

				KillTimer(TIMER_ROOTKIT_SCAN_STATUS);

				Sleep(50); //Varada Ikhar, Date:19/02/2015, Issue: In Win-Vista32 bit, if pause-resume pressed continuously, progress bar show wrong value.

				SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANPAUSE"));
				m_btnScan.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"));
				m_btnStop.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"));
				m_btnStop.EnableWindow(true);
				AddLogEntry(L">>> Scanning Paused..", 0, 0, true, ZEROLEVEL);
				m_btnScan.EnableWindow(true);
			}

		}
		
		else if(csBtnText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"))
		{
		
			if(!isAnyEntrySeletected())
			{

				//Ram: Issue Resolved: 0001193
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ROOTKITSCAN_OPTION"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
				m_bIsPopUpDisplayed = false;

				//Varada Ikhar, Date:10-03-2015, Issue: If user unselect all the 3 options & click scan, after message pop-up scan button is still disable.
				m_btnScan.EnableWindow(true);
				return;
			}
			m_bAntirootScanningInProgress = true;
			m_bScanningFinished = true;
			g_bIsRootkitScanning = true;
			m_bAntirootClose = false;
			m_bAntirootkitHome = false;
			m_bRedFlag = false;
			m_dwPercentage = 0;
			m_dwScannedFileFolder = 0;
			m_dwScannedCntRegistry = 0;
			m_dwScannedCnt = 0;
			m_dwThreatCntFilefolder = 0;
			m_dwThreatCnt = 0;
			m_dwThreatCntRegistry =0; 	

			if(g_TabCtrlRtktWindHandle != NULL)
			{
				// lalit  4-27-2015 , due to mutuall operation implementaion no more need to hide other tab control when any one is in-progress 
				//g_TabCtrlRtktWindHandle->m_pTabDialog->DisableAllExceptSelected();
			}
			SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANSTART"));
			if(!StartAntirootScanUsingService())
			{
				AddLogEntry(L"### Failed to send data to service StartAntirootScanUsingService in OnBnClickedBtnScan", 0, 0, true, SECONDLEVEL);
				return;
			}
			Sleep(500);

			//DWORD dwret = m_objIPCRootkitClient.OpenServerMemoryMappedFile();
			//if(dwret)
			//{
			//	MessageBox(L"!!!Error in OpenServerMemoryMappedFile",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			//	return;
			//}

			m_hGetPercentage = ::CreateThread(NULL, 0, GetPercentage, (LPVOID) this, 0, NULL);
			/*
				ISSUE NO - 2 In antirootkit scan ,while scanning "hidden items in windows registry" 
				and hidden items in running processess" the progress bar shows 0.
				NAME - Niranjan Deshak. - 10th Jan 2015
			*/
			//Sleep( 1000 ) ;

			//Varada Ikhar, Date:27/01/2015, Adding 'Total Scan Time' in the log.
			m_tsScanStartTime   = CTime::GetCurrentTime();
			m_tsScanPauseResumeElapsedTime -= m_tsScanPauseResumeElapsedTime;

			SetTimer(TIMER_ROOTKIT_SCAN_STATUS, 1000, NULL);
			//SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANSTART"));
			
			//Varada Ikhar, Date:24/01/2015, Adding log entry for Rootkit scanning started.
			CString csRKScanningStarted;
			csRKScanningStarted.Format(L">>> %s...",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANSTART"));
			AddLogEntry(csRKScanningStarted, 0, 0, true, SECONDLEVEL);

			m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
			m_btnScan.EnableWindow(true);
			//m_btnStop.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
			m_btnStop.EnableWindow(true);
			//m_btnBack.EnableWindow(false);//}
//			m_btnBackButton.ShowWindow(true);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CiGenXAntiRootkit::OnBnClickedBtnScan", 0, 0, true, SECONDLEVEL);
	}
		
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnPauseresume                                                     
*  Description    :	If clicks on pause button it will paused scanning and resume button will resume scanning.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/

/**********************************************************************************************************                     
*  Function Name  :	SetScanStatus                                                     
*  Description    :	Set scan Status on edit box
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::SetScanStatus(CString csStatus)
{
	m_edtAntiRootkitStatus.SetWindowText(csStatus);
	m_edtAntiRootkitStatus.Invalidate();
}

/**********************************************************************************************************                     
*  Function Name  :	ShowAntiRootkitScannedList                                                     
*  Description    :	Show rootscan list after completing scanning process. 
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::ShowAntiRootkitScannedList()
{

	m_bScanAborted = true;
	//m_bScanningStopped = true;
	g_bIsRootkitScanning = false;
	
	HideControls4ScannedList(false);

	m_stHAntiRootkit.ShowWindow(SW_SHOW);
	m_stThreatsFound.ShowWindow(SW_SHOW);
	m_lstAntiRookit.ShowWindow(SW_SHOW);
	m_lstAntiRookit.EnableWindow(true);
	m_chkAntiRootkitSelectAll.ShowWindow(SW_SHOW);
	/*****************ISSUE NO -79 Neha Gharge 22/5/14 ************************************************/
	m_chkWindowRegistry.ShowWindow(SW_HIDE);
	m_chkFileFolder.ShowWindow(SW_HIDE);
	m_chkAntiRootkitSelectAll.SetCheck(true);
	m_stAntiRootkitSelectAll.ShowWindow(SW_SHOW);
	m_btnAntiRootkitDelete.ShowWindow(SW_SHOW);
	m_stScannedEntries.ShowWindow(SW_SHOW);
	m_tabAntiRootkit.ShowWindow(SW_SHOW);
	m_tabAntiRootkit.EnableWindow(true);
	m_btnBackButton.ShowWindow(SW_SHOW);

	if(m_icount == 3)
	{
		if(m_bCheckFileFolder && m_bCheckProcess && m_bCheckRegistry)
		{
			m_tabAntiRootkit.SetCurSel(0);
			m_bProcessTabSelected = true;
		}
	}
	if(m_icount == 2)
	{
		if(m_bCheckProcess && m_bCheckRegistry)
		{
			m_tabAntiRootkit.SetCurSel(0);
			m_bProcessTabSelected = true;
		}
		else if(m_bCheckRegistry && m_bCheckFileFolder)
		{
			m_tabAntiRootkit.SetCurSel(1);
			m_bRegistryTabSelected = true;
		}
		else if(m_bCheckProcess && m_bCheckFileFolder)
		{
			m_tabAntiRootkit.SetCurSel(0);
			m_bProcessTabSelected = true;
		}
	}

	if(m_icount == 1)
	{
		if(m_bCheckProcess)
		{
			m_tabAntiRootkit.SetCurSel(0);
			m_bProcessTabSelected = true;
		}
		else if(m_bCheckRegistry)
		{
			m_tabAntiRootkit.SetCurSel(1);
			m_bRegistryTabSelected = true;
		}
		else if(m_bCheckFileFolder)
		{
			m_tabAntiRootkit.SetCurSel(2);
			m_bFilesFoldersTabSelected = true;
		}
	}
	
	ShowListcontrolOnBasisofSelection();
	//m_btnBack.EnableWindow(SW_SHOW);
	Invalidate();
	
}

/**********************************************************************************************************                     
*  Function Name  :	OnTcnSelchangeTabProcess                                                     
*  Description    :	Shows controls after selecting any one tab 
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnTcnSelchangeTabProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	int i = m_tabAntiRootkit.GetCurSel();
	if(i > 3)
	{
		return;
	}
	if(i == 1)
	{
		m_bRegistryTabSelected = true;
		m_lstRegistry.ShowWindow(true);
		m_stFileText.ShowWindow(false);
		m_stFilesSubText.ShowWindow(false);
		m_csScannedText.ShowWindow(false);
		m_csScannedSubText.ShowWindow(false);
		m_lstAntiRookit.ShowWindow(false);
		m_lstFilesFolders.ShowWindow(false);
		m_stScannedEntries.ShowWindow(false);
		m_stThreatsFound.ShowWindow(false);
		m_stScannedFilesFolders.ShowWindow(false);
		m_stThreatsFilesFolders.ShowWindow(false);
		m_bFilesFoldersTabSelected = false;
		m_bProcessTabSelected = false;
		m_stRegisteryText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_REG"));
		m_stRegisteryText.ShowWindow(true);
		m_csScannedEntries.Format(L": %d",m_dwScannedCntRegistry);
		m_stScannedRegistry.SetWindowTextW(m_csScannedEntries);
		m_stScannedRegistry.ShowWindow(true);
		m_stRegistrySubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_REG"));
		m_stRegistrySubText.ShowWindow(true);
		m_csThreatsfound.Format(L": %d",m_dwThreatCntRegistry);
		m_stThreatsRegistry.SetWindowTextW(m_csThreatsfound);
		/*****************ISSUE NO -79 Neha Gharge 22/5/14 ************************************************/
		m_stThreatsRegistry.ShowWindow(true);
		m_chkWindowRegistry.SetCheck(true);
		m_chkWindowRegistry.ShowWindow(true);
		m_chkFileFolder.ShowWindow(false);
		m_chkAntiRootkitSelectAll.ShowWindow(false);
		
	}
	else if(i == 2)
	{
		m_bFilesFoldersTabSelected = true;
		m_lstFilesFolders.ShowWindow(true);
		m_stRegisteryText.ShowWindow(false);
		m_stRegistrySubText.ShowWindow(false);
		m_csScannedText.ShowWindow(false);
		m_csScannedSubText.ShowWindow(false);
		m_lstAntiRookit.ShowWindow(false);
		m_lstRegistry.ShowWindow(false);
		m_stScannedEntries.ShowWindow(false);
		m_stThreatsFound.ShowWindow(false);
		m_stScannedRegistry.ShowWindow(false);
		m_stThreatsRegistry.ShowWindow(false);
		m_bProcessTabSelected = false;
		m_bRegistryTabSelected = false;
		m_stFileText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_FILE"));
		m_stFileText.ShowWindow(true);
		m_csScannedEntries.Format(L": %d",m_dwScannedFileFolder);
		m_stScannedFilesFolders.SetWindowTextW(m_csScannedEntries);
		m_stScannedFilesFolders.ShowWindow(true);
		m_stFilesSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_FILE"));
		m_stFilesSubText.ShowWindow(true);
		m_csThreatsfound.Format(L": %d",m_dwThreatCntFilefolder);
		m_stThreatsFilesFolders.SetWindowTextW(m_csThreatsfound);
		/*****************ISSUE NO -79 Neha Gharge 22/5/14 ************************************************/
		m_stThreatsFilesFolders.ShowWindow(true);
		m_chkWindowRegistry.ShowWindow(false);
		m_chkFileFolder.ShowWindow(true);
		m_chkFileFolder.SetCheck(true);
		m_chkAntiRootkitSelectAll.ShowWindow(false);
	}
	else if(i == 0)
	{
		m_bProcessTabSelected = true;
		m_lstAntiRookit.ShowWindow(true);
		m_stRegisteryText.ShowWindow(false);
		m_stRegistrySubText.ShowWindow(false);
		m_stFileText.ShowWindow(false);
		m_stFilesSubText.ShowWindow(false);
		m_lstRegistry.ShowWindow(false);
		m_lstFilesFolders.ShowWindow(false);
		m_stScannedRegistry.ShowWindow(false);
		m_stThreatsRegistry.ShowWindow(false);
		m_stScannedFilesFolders.ShowWindow(false);
		m_stThreatsFilesFolders.ShowWindow(false);
		m_bFilesFoldersTabSelected = false;
		m_bRegistryTabSelected = false;
		m_csScannedText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_PROC"));
		m_csScannedText.ShowWindow(true);
		m_csScannedEntries.Format(L": %d",m_dwScannedCnt);
		m_stScannedEntries.SetWindowTextW(m_csScannedEntries);
		m_stScannedEntries.ShowWindow(true);
		m_csScannedSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_PROC"));
		m_csScannedSubText.ShowWindow(true);
		m_csThreatsfound.Format(L": %d",m_dwThreatCnt);
		m_stThreatsFound.SetWindowTextW(m_csThreatsfound);
		/*****************ISSUE NO -79 Neha Gharge 22/5/14 ************************************************/
		m_stThreatsFound.ShowWindow(true);
		m_chkWindowRegistry.ShowWindow(false);
		m_chkFileFolder.ShowWindow(false);
		m_chkAntiRootkitSelectAll.SetCheck(true);
		m_chkAntiRootkitSelectAll.ShowWindow(true);
	}
	m_bAntirootkitCompleted = false;
	m_bAntirootScanningInProgress = false;

}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnantirootkitDelete                                                     
*  Description    :	After clicking clean button.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnBnClickedBtnantirootkitDelete() // for clean button
{
	m_tabAntiRootkit.EnableWindow(false);
	bool bEntrySelected = false;
	int iCnt = 0;
	CString  csAction;
	g_bIsRootkitCleaning = true;
	
	
	if(m_bProcessTabSelected)
	{
		iCnt = m_lstAntiRookit.GetItemCount();
		if(iCnt == 0)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_NOENTRIES"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			g_bIsRootkitCleaning = false;//Amit Dutta
			goto CleanUp;
		}
		for(int i = 0; i < m_lstAntiRookit.GetItemCount(); i++)
		{
			csAction = m_lstAntiRookit.GetItemText(i, 3);
			if(m_lstAntiRookit.GetCheck(i))
			{
				if(csAction == DETECTEDSTATUS)
				{
					bEntrySelected = true;
					break;
				}
				else if(csAction == DELETEDSTATUS)
				{
					m_lstAntiRookit.SetCheck(i, FALSE);
				}
			}
		}
	}
	else if(m_bRegistryTabSelected)
	{
		iCnt = m_lstRegistry.GetItemCount();
		if(iCnt == 0)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_NOENTRIES"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			g_bIsRootkitCleaning = false;
			goto CleanUp;
		}
		for(int i = 0; i < m_lstRegistry.GetItemCount(); i++)
		{
			csAction = m_lstRegistry.GetItemText(i, 2);
			if(m_lstRegistry.GetCheck(i))
			{
				if(csAction == DETECTEDSTATUS)
				{
					bEntrySelected = true;
					break;
				}
				else if(csAction == DELETEDSTATUS)
				{
					m_lstRegistry.SetCheck(i, FALSE);
				}
			}
		}
	}
	else if(m_bFilesFoldersTabSelected)
	{
		iCnt = m_lstFilesFolders.GetItemCount();
		if(iCnt == 0)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_NOENTRIES"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;

			g_bIsRootkitCleaning = false;
			goto CleanUp;
		}
		for(int i = 0; i < m_lstFilesFolders.GetItemCount(); i++)
		{
			csAction = m_lstFilesFolders.GetItemText(i, 1);
			if(m_lstFilesFolders.GetCheck(i))
			{
				if(csAction == DETECTEDSTATUS)
				{
					bEntrySelected = true;
					break;
				}
				else if(csAction == DELETEDSTATUS)
				{
					m_lstFilesFolders.SetCheck(i, FALSE);
				}
			}
		}
	}
	if(!bEntrySelected)
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_CLEANENTRIES"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION|MB_OK);
		m_bIsPopUpDisplayed = false;
		m_tabAntiRootkit.EnableWindow(true);
		
		return;
	}

	//Ram, Issue No: 0001216, Resolved
	if (theApp.m_dwDaysLeft == 0)
	{
		theApp.GetDaysLeft();
	}

	//issue no 721 neha gharge.
	//Check here for Evaluation
	if(theApp.m_dwDaysLeft == 0)
	{
		if(!theApp.ShowEvaluationExpiredMsg())
		{
			theApp.GetDaysLeft();
			return;
		}
	}
	//AfxBeginThread(DeleteThread, this, THREAD_PRIORITY_NORMAL, NULL, NULL, NULL);
	m_hCleaningThread = ::CreateThread(NULL, 0, DeleteThread, (LPVOID) this, 0, NULL);
	Sleep(500);

CleanUp:
	m_tabAntiRootkit.EnableWindow(true);
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckAntirootkitselectall                                                     
*  Description    :	After clicking select all button of processes.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnBnClickedCheckAntirootkitselectall()
{
	int iCheck = m_chkAntiRootkitSelectAll.GetCheck();
	if(m_bProcessTabSelected)
	{
		for(int i = 0; i < m_lstAntiRookit.GetItemCount(); i++)
		{
			m_lstAntiRookit.SetCheck(i, iCheck);
		}
	}

}

/**********************************************************************************************************                     
*  Function Name  :	OnTimer                                                     
*  Description    :	Displays total repaired entries & scanning percentage 
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnTimer(UINT_PTR nIDEvent)
{
	TCHAR	szTemp[256] = {0} ;

	if(nIDEvent == TIMER_ROOTKIT_SCAN_STATUS)
	{
		//On WM_CLOSE call,UI will not get repaint after resuming scan as TIMER_SCAN_STATUS gets killed on pause.
		//Neha Gharge 13/5/2015
		if (m_bOnWMClose == false)
		{
			//Varada Ikhar, Date:27/01/2015, Adding 'Total Scan Time' in the log.
			m_tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsScanStartTime) - m_tsScanPauseResumeElapsedTime;
			CString csTime = m_tsScanElapsedTime.Format(_T("%H:%M:%S"));

			static int iPercentage = 0;
			//GetPercentage(dwPercentage);

			//Varada Ikhar, Date:19/02/2015, Issue: In Win-Vista32 bit, if pause-resume pressed continuously, progress bar show wrong value.
			//CString cspercentage;
			//cspercentage.Format(L"Percentage = %d",m_dwPercentage);
			//AddLogEntry(cspercentage,0,0,true,ZEROLEVEL);
			m_prgAntiRootkit.SetPos(m_dwPercentage);
		}

		//Neha Gharge & Varada Ikhar, Date:10-03-2015
		//Issue: Not Reported. If any user clicks on stop button just before getting the scan complete message but percentage value is 100%.
		if (m_dwPercentage == 100)
		{
			m_btnStop.EnableWindow(false);
		}

		if(m_dwPercentage > 100)
		{
			m_dwPercentage = 0;
			KillTimer(TIMER_ROOTKIT_SCAN_STATUS);
			//ShowAntiRootkitScannedList();
		}
	}
	else if(nIDEvent == TIMER_PAUSE_HANDLER)
	{
		m_tsScanPauseResumeTime = CTime::GetCurrentTime();
		KillTimer (TIMER_ROOTKIT_SCAN_STATUS);
	}
	CJpegDialog::OnTimer(nIDEvent);
}

/**********************************************************************************************************                     
*  Function Name  :	DeleteThread                                                     
*  Description    :	Thread to clean detected entries
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
DWORD WINAPI DeleteThread(LPVOID lpParam)
{
	CISpyAntiRootkit *pThis = (CISpyAntiRootkit *) lpParam;
	pThis->m_btnAntiRootkitDelete.EnableWindow(false);
	pThis->m_btnBackButton.EnableWindow(false);
	//pThis->m_btnBack.EnableWindow(false);
	pThis->DeleteEntries();
	return 1;
}

/**********************************************************************************************************                     
*  Function Name  :	DeleteEntries                                                     
*  Description    :	Send entry to service to get clean
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::DeleteEntries()
{
	try
	{
		bool bCheck = false;
		DWORD dwCleanCount = 0;
		CString csAction;
		m_bRedFlag = true;

		TCHAR	Datebuff[9] = {0} ;
		TCHAR	csDate[9] = {0} ;
		BOOL bBackUp=0;

		_wstrdate_s(Datebuff,9);
		_tcscpy_s(csDate,Datebuff);


		if(!SendReportOperations2Service(RELOAD_DBENTRIES, L"", REPORTS, true))
		{
			AddLogEntry(L"### Error in CGenXAntiRootkit::SendRecoverOperations2Service RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
		}

		if(m_bProcessTabSelected)
		{
			CString csProcessPath, csProcessID;
			for(int i = 0; i < m_lstAntiRookit.GetItemCount(); i++)
			{
				bCheck = m_lstAntiRookit.GetCheck(i)?true:false;
				csAction = m_lstAntiRookit.GetItemText(i, 3);
				if(bCheck && csAction == DETECTEDSTATUS)
				{
					csProcessPath = m_lstAntiRookit.GetItemText(i, 2); //process name
					csProcessID = m_lstAntiRookit.GetItemText(i, 0); //process ID
					//0x01 dword for process	
					if(!SendFile4RepairUsingService(HANDLE_VIRUS_ENTRY, csProcessPath, csProcessID, L"" ,0x01, ROOTKITSCAN, true))
					{
						m_lstAntiRookit.SetItemText(i, 3, FAILEDSTATUS);
						AddLogEntry(L"### Error in CGenXAntiRootkit::SendFile4RepairUsingService HANDLE_VIRUS_ENTRY", 0, 0, true, SECONDLEVEL);
						continue;
					}
					//ISSue no : 307 Neha Gharge 28/5/2014**************************/
					AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"),L"NA",csProcessPath,DELETEDSTATUS);

					m_lstAntiRookit.SetCheck(i,false);
					m_lstAntiRookit.SetItemText(i, 3, DELETEDSTATUS);
					dwCleanCount++;
				}
			}
			m_tabAntiRootkit.EnableWindow(true);			
		}
		if(m_bRegistryTabSelected)//no code change for registry now.
		{
			CString csServiceName, csServicePath;
			for(int i = 0; i < m_lstRegistry.GetItemCount(); i++)
			{
				bCheck = m_lstRegistry.GetCheck(i)?true:false;
				csAction = m_lstRegistry.GetItemText(i, 2);
				if(bCheck && csAction == DETECTEDSTATUS)
				{
					csServiceName = m_lstRegistry.GetItemText(i, 0); //registry key 
					csServicePath = m_lstRegistry.GetItemText(i, 1); //registry value
					//csRegData = m_lstRegistry.GetItemText(i, 2); // registry data
					//0x02 for Registry	
					if(!SendFile4RepairUsingService(HANDLE_VIRUS_ENTRY, csServiceName, csServicePath,L"",0x02, ROOTKITSCAN, true))
					{
						m_lstRegistry.SetItemText(i, 2, FAILEDSTATUS);
						AddLogEntry(L"### Error in CGenXAntiRootkit::SendFile4RepairUsingService HANDLE_VIRUS_ENTRY", 0, 0, true, SECONDLEVEL);
						continue;
					}
					//ISSue No 307 Neha Gharge 28/5/2014 **************************************/
					AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), csServiceName, csServicePath, DELETEDSTATUS);
					m_lstRegistry.SetCheck(i,false);
					m_lstRegistry.SetItemText(i, 2, DELETEDSTATUS);
					dwCleanCount++;
				}
			}
			m_tabAntiRootkit.EnableWindow(true);
		}
		if(m_bFilesFoldersTabSelected)
		{
			CString csFileFolderPath,csFileFolderName;
			for(int i = 0; i < m_lstFilesFolders.GetItemCount(); i++)
			{
				bCheck = m_lstFilesFolders.GetCheck(i)?true:false;
				csAction = m_lstFilesFolders.GetItemText(i, 1);
				if(bCheck && csAction == DETECTEDSTATUS)
				{
					csFileFolderPath = m_lstFilesFolders.GetItemText(i, 0); //file/folder path 
					//csFileFolderName = m_lstFilesFolders.GetItemText(i, 1);//file/folder name
					//0x03 for device name or file/folder name
					if(!SendFile4RepairUsingService(HANDLE_VIRUS_ENTRY, csFileFolderPath, L"", L"", 0x03, ROOTKITSCAN, true))
					{
						m_lstFilesFolders.SetItemText(i, 1, FAILEDSTATUS);
						AddLogEntry(L"### Error in CGenXAntiRootkit::SendFile4RepairUsingService HANDLE_VIRUS_ENTRY", 0, 0, true, SECONDLEVEL);
						continue;
					}
					//ISSue No 307 Neha Gharge 28/5/2014 **************************************/
					AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), L"NA", csFileFolderPath, DELETEDSTATUS);
					m_lstFilesFolders.SetCheck(i,false);
					m_lstFilesFolders.SetItemText(i, 1, DELETEDSTATUS);
					dwCleanCount++;
				}
			}
			m_tabAntiRootkit.EnableWindow(true);
		}
		m_bRootkitDeleted = true;

		Sleep(5);
		
		if(!SendFile4RepairUsingService(SAVE_REPORTS_ENTRIES, L"", L"",L"", 5, ROOTKITSCAN))
		{
			AddLogEntry(L"### Error in CGenXAntiRootkit::SendFile4RepairUsingService SAVE_REPORTS_ENTRIES", 0, 0, true, SECONDLEVEL);
		}
		m_bRedFlag = false;
		g_bIsRootkitCleaning = false;
		CString csTotalClean;
		csTotalClean.Format(L"%s %d", theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_CLEANING_SUCCESSFULL"), dwCleanCount);

		m_bIsPopUpDisplayed = true;
		MessageBox(csTotalClean, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
		m_bIsPopUpDisplayed = false;

		if(!IsAllEntriesCleaned())
		{
			m_btnAntiRootkitDelete.EnableWindow(true);
			//m_btnBack.EnableWindow(true);
		}
		else
		{
			if(!SetRegistrykeyUsingService(L"SOFTWARE\\Wardwiz Antivirus", 
				L"VirusFound", REG_DWORD, 0x00, false))
			{
				AddLogEntry(L"### Error in SetRegistrykeyUsingService VirusFound in CNextGenExAntiRootkit::DeleteEntries", 0, 0, true, SECONDLEVEL);
			}
			goto CleanUp;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXAntiRootkit::DeleteEntries", 0, 0, true, SECONDLEVEL);
	}
CleanUp:
	m_tabAntiRootkit.EnableWindow(true);
	m_btnAntiRootkitDelete.EnableWindow(true);
	//m_btnBack.EnableWindow(true);
		
}

/**********************************************************************************************************                     
*  Function Name  :	SendReportOperations2Service                                                     
*  Description    :	Send rootkit scan report to service, so that it get stored to DB file
*  Author Name    : Neha Ghareg 
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::SendReportOperations2Service(int dwMessageinfo, CString csReportFileEntry ,DWORD dwType, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = dwMessageinfo;
	_tcscpy_s(szPipeData.szFirstParam , csReportFileEntry);
	szPipeData.dwValue = dwType;
	CISpyCommunicator objCom(SERVICE_SERVER, true);

	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXAntiRootkit::SendReportOperations2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to Read data in CGenXAntiRootkit::SendReportOperations2Service", 0, 0, true, SECONDLEVEL);
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
*  Function Name  :	SendFile4RepairUsingService                                                     
*  Description    :	Send file to repair using services
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::SendFile4RepairUsingService(int iMessage, CString csEntryOne, CString csEntryTwo, CString csEntryThree, DWORD dwISpyID , DWORD dwScanType, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = iMessage;
	szPipeData.dwValue =  dwISpyID;
	szPipeData.dwSecondValue = dwScanType;
	wcscpy_s(szPipeData.szFirstParam, csEntryOne);
	wcscpy_s(szPipeData.szSecondParam , csEntryTwo);
	wcscpy_s(szPipeData.szThirdParam , csEntryThree);

	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXAntiRootkit::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CGenXAntiRootkit::SendFile4RepairUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SetRegistrykeyUsingService                                                     
*  Description    :	Set any registry key using service
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait)
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
		AddLogEntry(L"### Failed to send data in CGenXAntiRootkit : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CGenXAntiRootkit : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	IsAllEntriesCleaned                                                     
*  Description    :	Checks all entry clean or not
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::IsAllEntriesCleaned()
{
	bool bReturn = true;

	if(m_bProcessTabSelected)
	{
		for(int i = 0; i < m_lstAntiRookit.GetItemCount(); i++)
		{
			CString csAction = m_lstAntiRookit.GetItemText(i, 3);
			if(csAction == DETECTEDSTATUS	||
				csAction == FAILEDSTATUS	)
			{
				bReturn = false;
				break;
			}
		}
	}
	if(m_bRegistryTabSelected)
	{
		for(int i = 0; i < m_lstRegistry.GetItemCount(); i++)
		{
			CString csAction = m_lstRegistry.GetItemText(i, 2);
			if(csAction == DETECTEDSTATUS	||
				csAction == FAILEDSTATUS	)
			{
				bReturn = false;
				break;
			}
		}
	}
	if(m_bFilesFoldersTabSelected)
	{
		for(int i = 0; i < m_lstFilesFolders.GetItemCount(); i++)
		{
			CString csAction = m_lstFilesFolders.GetItemText(i, 1);
			if(csAction == DETECTEDSTATUS	||
				csAction == FAILEDSTATUS	)
			{
				bReturn = false;
				break;
			}
		}
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	ShutDownRootkitScanning                                                     
*  Description    :	After manual stopping rootkit scan this function get called.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::ShutDownRootkitScanning()
{
	if (!g_bIsRootkitScanning) 
	{
		return true;
	}
	try
	{
		//m_btnBackButton.ShowWindow(SW_SHOW);
		m_bScanningStopped = false;

		//Varada Ikhar, Date: 23/04/2015
		//Issue : 000033 : 1.In all scans, start scanning 2. click close button in UI then popup displays 'Do you want to stop rootkit scan' 3. Dont take any action
		//4. Rootkit scanning completed message appears 5. Then click yes on stop confirmation dialog box 6. UI is getting closed by leaving succesful dialog box on the desktop.
		CString csScanBtnText = L""; 
		m_btnScan.GetWindowText(csScanBtnText);
		if (csScanBtnText != theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"))
		{
			if (SendRequestCommon(PAUSE_SCAN))
			{
				//On WM_CLOSE call, UI will not get repaint after resuming scan as TIMER_SCAN_STATUS gets killed on pause.
				//Neha Gharge 13/5/2015
				//OnTimer(TIMER_PAUSE_HANDLER);
				//KillTimer(TIMER_ROOTKIT_SCAN_STATUS);
				Sleep(50);
				m_bOnWMClose = true;
				m_tsScanPauseResumeTime = CTime::GetCurrentTime();
				SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANPAUSE"));
				AddLogEntry(L">>> Rootkit Scanning Paused..", 0, 0, true, ZEROLEVEL);
			}
			else
			{
				AddLogEntry(L"### Failed to Pause Rootkit scanning.", 0, 0, true, SECONDLEVEL);
			}
		}
		//Issue : 0000480 Issue with message mentioned in Antirootkit scan.
		//Resolved By: Nitin K. 
		CString csTemp;
		CString csStr = theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCAN_IN_PROGESS_FIRST");
		csTemp.Format(L"%s %s\0\r\n", L"Rootkit", csStr);
		csTemp.Append(L"\n");
		csTemp.Append(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCAN_IN_PROGESS_SECOND"));
		
		m_bIsPopUpDisplayed = true;
		int iReturn = MessageBox(csTemp,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		m_bIsPopUpDisplayed = false;

		//Issue : 1.In Antirootkit scan (32 & 64 bit) if we click on stop button,
		//and do not take any action after completion "Scanning Aborted" pop-up appears.
		//Resolved By: Nitin K Date:10-Feb-2015
		if( iReturn == IDYES )
		{
			//Varada Ikhar, Date: 28/04/2015
			// Issue : 0000250 :1. Start Antirootkit scan. 2. Exit tray from the task bar. 3. Rootkit scanning aborted message box appears.
			//4. But that entry is not reflecting in reports.
			AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_SCAN_ABORTED"));
			if (!SendReportOperations2Service(SAVE_REPORTS_ENTRIES, L"", REPORTS, true))
			{
				AddLogEntry(L"### Error in CGenXAntiRootkit::SendRecoverOperations2Service RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
			}

			if (g_bIsRootkitScanning)
			{
				m_bScanningStopped = true;
			}
		}
		if(iReturn == IDNO)
		{
			//m_btnBackButton.ShowWindow(false);
			//Issue : 3.In Antirootkit scan if we click on stop button and press "No" after scanning completion,
			//"Back" button gets disappear.
			//Resolved By: Nitin K Date:10-Feb-2015
			//if(g_bIsRootkitScanning == false)
			//{
			//	m_btnBackButton.ShowWindow(true);
			//}
			m_bAntirootClose = false;// Amit Dutta 
			m_bScanningStopped = false;

			//Varada Ikhar, Date: 23/04/2015
			//Issue : 000033 : 1.In all scans, start scanning 2. click close button in UI then popup displays 'Do you want to stop rootkit scan' 3. Dont take any action
			//4. Rootkit scanning completed message appears 5. Then click yes on stop confirmation dialog box 6. UI is getting closed by leaving succesful dialog box on the desktop.
			CString csScanBtnText = L"";
			m_btnScan.GetWindowText(csScanBtnText);
			if (csScanBtnText != theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"))
			{
				if (SendRequestCommon(RESUME_SCAN))
				{
					//On WM_CLOSE call,UI will not get repaint after resuming scan as TIMER_SCAN_STATUS gets killed on pause.
					//Neha Gharge 13/5/2015
					if (m_bOnWMClose == true)
					{
						m_bOnWMClose = false;
					}
					SetTimer(TIMER_ROOTKIT_SCAN_STATUS, 1000, NULL);
					m_tsScanPauseResumeElapsedTime += ((CTime::GetCurrentTime() - m_tsScanPauseResumeTime));
					SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANRESUME"));
					Sleep(50);
					AddLogEntry(L">>> Rootkit Scanning Resumed..", 0, 0, true, ZEROLEVEL);
				}
				else
				{
					AddLogEntry(L"### Failed to Resume Rootkit scanning.", 0, 0, true, SECONDLEVEL);
				}
			}
			return false;
		}

		else if(iReturn == IDYES && m_bAntirootClose && g_bIsRootkitScanning == false)
		{
			//Neha Gharge 3/3/2015 
			//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
			if(m_bAntirootClose == true)
			{
				theApp.m_bOnCloseFromMainUI = true;
			}

			m_btnBackButton.ShowWindow(true);
			m_bAntirootScanningInProgress = false;
			m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));

			return true;
		}

		else if(iReturn == IDYES && ( m_bAntirootClose || m_bScanningStopped))//|| m_bAntirootkitHome
		{
			//Neha Gharge 3/3/2015 
			//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
			if (m_bAntirootClose == true)
			{
				theApp.m_bOnCloseFromMainUI = true;
			}

			ISPY_PIPE_DATA szPipeData = {0};
			szPipeData.iMessageInfo = STOP_SCAN;
			szPipeData.dwValue =  static_cast<DWORD>(ANTIROOTKITSCAN);
			CISpyCommunicator objCom(SERVICE_SERVER, true);
			// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.
			m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
			m_btnScan.EnableWindow(false);
			m_btnStop.EnableWindow(false);
			//Issue : In Antirootkit scan if we click on stop button and press "yes" after scanning completion,
			//scan and stop button remains disabled.
			//Resolved By: Nitin K Date:10-Feb-2015
			if(g_bIsRootkitScanning == false)
			{
				m_btnScan.EnableWindow(true);
			}
			if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CGenXAntiRootkit::ShutDownAntiRootkitScanning", 0, 0, true, SECONDLEVEL);
				return false;
			}
			if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CGenXAntiRootkit::ShutDownAntiRootkitScanning", 0, 0, true, SECONDLEVEL);
				return false;
			}
			
			if(m_hGetPercentage != NULL)
			{
				ResumeThread(m_hGetPercentage);
				TerminateThread(m_hGetPercentage, 0);
				m_hGetPercentage = NULL;
			}
			
			//Varada Ikhar, Date:27/01/2015, Adding 'Total Scan Time' in the log.
			CTimeSpan tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsScanStartTime) - m_tsScanPauseResumeElapsedTime;

			//Ticket No : 43 and 37 Now according to design after clicking close button from UI, No one functionality
			// will give message box of aborted 
			//Even though 100% is shown on progress bar but click on close button. then also no completion message will popup

			//if(m_bAntirootClose)//m_bAntirootkitHome ||
			//{
				//MessageBox( TEXT("Rootkit scanning is aborted"), TEXT("Wardwiz"), MB_ICONEXCLAMATION);

				//Issue : In Antirootkit scan->click 'scan'->click 'stop'->dont take any action->'scanning completed' dialog box appears
				//->now click on 'yes' button in stop confirmation pop up->message below the progress bar changing from 'scanning completed' 
				//to 'scanning aborted' by user.
				// Neha Gharge & Varada Ikhar, Date:10-03-2015
				//SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ABRTUSER"));

				//MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ROOTKITABORT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
			//}
			/*	ISSUE NO - 496 NAME - NITIN K. TIME - 30th May 2014 */       
			//Amit Dutta
			//if(m_bScanningStopped == true)
			//{
			//  RootKitScanningFinished();
			//}
			m_bAntirootScanningInProgress = false;
			KillTimer(TIMER_ROOTKIT_SCAN_STATUS);
			//g_bIsRootkitScanning = false;       // after stop scanning re-set control not getting
			// resolved by lalit kumawat,5-14-2015
			//issue-if antirootkit scanning is running and close from tray, in such case scan button gettig enable before ui close
			//m_btnScan.EnableWindow(true);
		}

	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXAntiRootkit::ShutDownRootkitScanning", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	StartAntirootScanUsingService                                                     
*  Description    : Start rootkit scan using service
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::StartAntirootScanUsingService()
{
	try
	{
		DWORD dwScanOption = 0;
		GetDWORDFromRootKitScanOptions(dwScanOption);
		m_dwRootKitOption = dwScanOption;

		ISPY_PIPE_DATA szPipeData = {0};
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = START_SCAN;
		szPipeData.dwValue =  static_cast<DWORD>(ANTIROOTKITSCAN);
		szPipeData.dwSecondValue = m_dwRootKitOption;
		
		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CGenXAntiRootkit::StartAntirootScanUsingServic", 0, 0, true, SECONDLEVEL);
			return false;
		}

		//isScanning = true;				
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartAntirootScanUsingServic", 0, 0, true, SECONDLEVEL);
	}
	return true;
	
}

/**********************************************************************************************************                     
*  Function Name  :	GetDWORDFromRootKitScanOptions                                                     
*  Description    : Get dword value from selected rootkit scan options
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CISpyAntiRootkit::GetDWORDFromRootKitScanOptions( DWORD &dwRegScanOpt)
{
	DWORD	dwValue = 0x00 ;
	DWORD	dwTemp  = 0x00 ;

	for(DWORD i=0x00; i<0x03; i++ )
	{
		switch( i )
		{
		case 0:
			if( m_chkProcess.GetCheck() )
			{
				dwValue = dwTemp = 0x01 ;
			}
			break ;
		case 1:
			if( m_chkRegistry.GetCheck() )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;

		case 2:
			if( m_chkFilesFolders.GetCheck() )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		}
	}
	dwRegScanOpt = dwValue;
}

/**********************************************************************************************************                     
*  Function Name  :	isAnyEntrySeletected                                                     
*  Description    :	Checks atleast one entry is selected or not.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::isAnyEntrySeletected()
{
	 m_icount = 0;
	 m_bCheckFileFolder = false;
	 m_bCheckProcess = false;
	 m_bCheckRegistry = false;

	if( m_chkProcess.GetCheck())
	{
		m_dwGetCheckValues = 0x01;
		m_bCheckProcess = true;
		m_icount++;
	}
	if( m_chkRegistry.GetCheck())
	{
		m_dwGetCheckValues = 0x02;
		m_bCheckRegistry = true;
		m_icount++;
	}
	if( m_chkFilesFolders.GetCheck())
	{
		m_dwGetCheckValues = 0x03;
		m_bCheckFileFolder = true;
		m_icount++;
	}

	if(m_icount==0)
	{
		return false;
	}
	else
	{
		return true;
	}

}

/**********************************************************************************************************                     
*  Function Name  :	SendRequestCommon                                                     
*  Description    :	Send request to comm service
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::SendRequestCommon(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;
		szPipeData.dwValue =  static_cast<DWORD>(ANTIROOTKITSCAN);

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

/**********************************************************************************************************                     
*  Function Name  :	RootKitScanningStarted                                                     
*  Description    :	Open memory after server memory gets created
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 28 May ,2014
**********************************************************************************************************/
bool CISpyAntiRootkit::RootKitScanningStarted()
{
	DWORD dwret = m_objIPCRootkitClient.OpenServerMemoryMappedFile();
   // after scanning stop ui does not showing scanning abort mssage and other option disable
	
	m_bScanningStopped = false;
	m_chkFilesFolders.EnableWindow(FALSE);    //VARADA IKHAR
	m_chkRegistry.EnableWindow(FALSE);        //VARADA IKHAR
	m_chkProcess.EnableWindow(FALSE);         //VARADA IKHAR
	if(dwret)
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ERROR_IN_OPEN_MEMORY_MAPPED_FILE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		m_bIsPopUpDisplayed = false;
		return false;
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	RootKitScanningFinished                                                    
*  Description    :	According to manual stop and complete scan gives message box and reset controls.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::RootKitScanningFinished()
{
	try
	{
		OutputDebugStringW(L"WrdWizAntirootscan UI::Scanning finished");
		if (!g_bIsRootkitScanning)
		{
			return false;
		}
		g_bIsRootkitScanning = false;
		CString csBtnText, stopBtnText;  //ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
		m_btnScan.GetWindowText(csBtnText);   //ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
		m_btnStop.GetWindowText(stopBtnText);	//ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
		if ((csBtnText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE")) || (csBtnText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME")))	//ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
		{
			m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));	//ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
			m_btnScan.EnableWindow(FALSE);	//ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
		}
		if (stopBtnText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"))	//ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
		{
			m_btnStop.EnableWindow(FALSE);	//ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
		}

		::WaitForSingleObject(m_hGetPercentage, 500);
		if (m_hGetPercentage != NULL)
		{
			ResumeThread(m_hGetPercentage);
			TerminateThread(m_hGetPercentage, 0);
			m_hGetPercentage = NULL;
			//Varada Ikhar, Date:04-03-2015, Issue: In C Drive->WardWiz Antivirus folder->Log->WRDWIZAVUI.LOG->spelled wrong(succesufully ).
			AddLogEntry(L"Terminate get percentage thread successfully", 0, 0, true, SECONDLEVEL);
		}
		//Issue:3. In WardWiz Essential,in antirootkit scan if I click scan and click stop button and don't take action after completion of scan if I click "No" "Back" button is shown on UI below "Scan" button.
		//Resolved By: Nitin K Date: 25th-Feb-2015
		//g_bIsRootkitScanning = false;	

		//Varada Ikhar, Date:27/01/2015, Adding 'Total Scan Time' in the log.
		CString csTime = m_tsScanElapsedTime.Format(_T("%H:%M:%S"));
		CString csTotalScanTime;
		csTotalScanTime.Format(L">>> Total Elapsed Time = %s", csTime);

		//KillTimer(TIMER_ROOTKIT_SCAN_STATUS);
		//Neha Gharge 3/3/2015 
		//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
		if (m_bScanningStopped == false)
			//if(m_bAntirootClose == false && m_bScanningStopped == false)
		{
			m_prgAntiRootkit.SetPos(m_dwPercentage); //Varada
			if (m_dwThreatCntFilefolder == 0 && m_dwThreatCnt == 0 && m_dwThreatCntRegistry == 0)
			{
				//ISSue No 307 Neha Gharge 28/5/2014 **************************************/
				AddEntriesInReportsDB(L"Rootkit Scan", L"NA", L"NA", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND"));
			}

			//Varada Ikhar, Date:24/01/2015, Adding log entry for Rootkit scanning finished, Files Scanned, Threats Found and Elapsed Time..
			CString csRKScanningComplete;
			AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANCOMPLETE"));
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> Total Scanned:		Processes = [%d]\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwScannedCnt, m_dwScannedCntRegistry, m_dwScannedFileFolder);
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> Total Threats Found:	Processes = [%d]\t\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwThreatCnt, m_dwThreatCntRegistry, m_dwThreatCntFilefolder);
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			AddLogEntry(csTotalScanTime, 0, 0, true, SECONDLEVEL);
			AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
			OutputDebugStringW(L"WrdWizAntirootscan UI::Before Message box");

			//add by lalit 5-7-2015,it use to highlight Task bar icon when scannin completed message box appeared. 
			::SetForegroundWindow(m_hWnd);
			
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCANCOMPLETE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
			m_bIsPopUpDisplayed = false;

			if (!SendReportOperations2Service(SAVE_REPORTS_ENTRIES, L"", REPORTS, true))
			{
				AddLogEntry(L"### Error in CGenXAntiRootkit::SendRecoverOperations2Service RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
			}
			KillTimer(TIMER_ROOTKIT_SCAN_STATUS); //varada
			ShowAntiRootkitScannedList();
			m_bAntirootScanningInProgress = false;
			m_bAntirootkitCompleted = true;
			m_bScanningStopped = false;
			m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
			m_btnScan.EnableWindow(true);
			if (g_TabCtrlRtktWindHandle != NULL)
			{
				g_TabCtrlRtktWindHandle->m_btnCustomScan->EnableWindow(true);
				g_TabCtrlRtktWindHandle->m_btnQuickScan->EnableWindow(true);
				g_TabCtrlRtktWindHandle->m_btnFullScan->EnableWindow(true);
			}

			//Varada Ikhar, Date:02-03-2015, Issue: Check boxes should be enabled after message gets displayed.
			m_chkFilesFolders.EnableWindow(TRUE);
			m_chkRegistry.EnableWindow(TRUE);
			m_chkProcess.EnableWindow(TRUE);

			return true;
		}
		else if (m_bScanningStopped == true)
		{
			//MessageBox( TEXT("Rootkit scanning is aborted"), TEXT("Wardwiz"), MB_ICONEXCLAMATION);
			//ISSue No 307 Neha Gharge 28/5/2014 **************************************/
			/*AddEntriesInReportsDB(L"Rootkit Scan", L"NA", L"NA", L"Scanning Aborted");

			if (!SendReportOperations2Service(SAVE_REPORTS_ENTRIES, L"", REPORTS, true))
			{
				AddLogEntry(L"### Error in CWrdWizAntiRootkit::SendRecoverOperations2Service RELOAD_DBENTRIES REPORTS", 0, 0, true, SECONDLEVEL);
			}*/

			//Varada Ikhar, Date:24/01/2015, Adding log entry for Rootkit scanning aborted, Files Scanned, Threats Found and Elapsed Time.
			CString csRKScanningComplete;
			AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ROOTKITABORT"));
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> Total Scanned:		Processes = [%d]	\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwScannedCnt, m_dwScannedCntRegistry, m_dwScannedFileFolder);
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> Total Threats Found:	Processes = [%d]\t\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwThreatCnt, m_dwThreatCntRegistry, m_dwThreatCntFilefolder);
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			AddLogEntry(csTotalScanTime, 0, 0, true, SECONDLEVEL);
			AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);

			//Issue : In Antirootkit scan->click 'scan'->click 'stop'->dont take any action->'scanning completed' dialog box appears
			//->now click on 'yes' button in stop confirmation pop up->message below the progress bar changing from 'scanning completed' 
			//to 'scanning aborted' by user.
			// Neha Gharge & Varada Ikhar, Date:10-03-2015
			SetScanStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ABRTUSER"));

			m_bIsPopUpDisplayed = true;
			//Varada Ikhar, Date: 21/04/2015
			//Issue : 000051 : Issue in symbol represented for abort message:-In antirootkit scan the symbol which comes in “Scanning aborted” message box should be changed.
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ROOTKITABORT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
			m_bIsPopUpDisplayed = false;


			m_btnBackButton.ShowWindow(true);
			if (m_bScanningStopped == true)
			{
				ShowAntiRootkitScannedList();
				m_bAntirootScanningInProgress = false;
				m_bAntirootkitCompleted = false;
			}
			m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
			m_btnScan.EnableWindow(true);
			if (g_TabCtrlRtktWindHandle != NULL)
			{
				g_TabCtrlRtktWindHandle->m_btnCustomScan->EnableWindow(true);
				g_TabCtrlRtktWindHandle->m_btnQuickScan->EnableWindow(true);
				g_TabCtrlRtktWindHandle->m_btnFullScan->EnableWindow(true);
			}
			m_bScanningStopped = false;

			//Varada Ikhar, Date:02-03-2015, Issue: Check boxes should be enabled after message gets displayed.
			m_chkFilesFolders.EnableWindow(TRUE);
			m_chkRegistry.EnableWindow(TRUE);
			m_chkProcess.EnableWindow(TRUE);
		}
		//Issue:3. In WardWiz Essential,in antirootkit scan if I click scan and click stop button and don't take action after completion of scan if I click "No" "Back" button is shown on UI below "Scan" button.
		//Resolved By: Nitin K Date: 25th-Feb-2015
		//g_bIsRootkitScanning = false;
		m_btnScan.EnableWindow(true);  //ISSUE NO: 12, VARADA IKHAR DATE: 17/12/2014
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExAntiRootkit::RootKitScanningFinished", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	GetPercentage                                                     
*  Description    :	Get all entries through shared memory which has to display on the UI
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
DWORD WINAPI GetPercentage(LPVOID lpvThreadParam)
{
	try
	{
		CISpyAntiRootkit *pThis = (CISpyAntiRootkit*)lpvThreadParam;
		if(!pThis)
			return 0;

		wchar_t Buffer[100];
		CString csType,csPID;

		while(true)
		{ 

			
			ITIN_MEMMAP_DATA iTinMemMap = {0};
			pThis->m_objIPCRootkitClient.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));
		
			switch(iTinMemMap.iMessageInfo)
			{

				case SETSCANSTATUS:
				{
					CString csCurrentStatus =L"";
					csCurrentStatus = iTinMemMap.szFirstParam;
					OutputDebugString(csCurrentStatus);
					if(csCurrentStatus.GetLength() > 0)
					{
						OutputDebugString(csCurrentStatus);
						pThis->m_csPreviousStatus.SetString(csCurrentStatus);
						pThis->m_edtAntiRootkitStatus.SetWindowTextW(pThis->m_csPreviousStatus);
					}
					csCurrentStatus.ReleaseBuffer();
				}
				
				break;
			
				case SETPERCENTAGE:
				{
					pThis->m_dwPercentage  = iTinMemMap.dwFirstValue;
					//_itow(iTinMemMap.dwFirstValue,Buffer,10);
					//AddLogEntry(L"Client side value of percentage %s",Buffer);
					break;
				}
				case SHOWVIRUSFOUND:
				{ 
					CString csCurrentPath(L"");
					csCurrentPath.Format(L"%s", iTinMemMap.szFirstParam);
					if(csCurrentPath.GetLength() > 0)
					{
						OutputDebugString(csCurrentPath);
						pThis->m_dwForInsertItem = iTinMemMap.dwSecondValue;
						csPID.Format(L"%d",iTinMemMap.dwFirstValue);
						csType.Format(L"%d",iTinMemMap.dwSecondValue);
						pThis->InsertItem(iTinMemMap.szFirstParam,csPID,iTinMemMap.szSecondParam,DETECTEDSTATUS);
						//AddLogEntry(L"Client Process path : %s",iTinMemMap.szFirstParam);
						//AddLogEntry(L"Client Process name: %s",iTinMemMap.szSecondParam);
						//AddLogEntry(L"csPID :%s",csPID);
						//AddLogEntry(L"Client Type :%s",csType);
					}
					csCurrentPath.ReleaseBuffer();
					break; 
				}
				case SHOWDETECTEDENTRIES:
					{
						//pThis->m_dwDetectedEntries = iTinMemMap.dwFirstValue;
						pThis->m_dwType = iTinMemMap.dwSecondValue;
						pThis->AddDetectedCount(iTinMemMap.dwFirstValue);
						_itow_s(iTinMemMap.dwFirstValue,Buffer,10);
						//AddLogEntry(L"Client side value of detected entries %s",Buffer);
						break;
					}
				case SHOWTOTALCOUNT:
					{
						pThis->m_dwTotalType = iTinMemMap.dwSecondValue;
						pThis->AddTotalCount(iTinMemMap.dwFirstValue);
						_itow_s(iTinMemMap.dwFirstValue,Buffer,10);
						//AddLogEntry(L"Client side value of total entries %s",Buffer);
						break;
					}
			}
			
			/*
				ISSUE NO - 2 In antirootkit scan ,while scanning "hidden items in windows registry" 
				and hidden items in running processess" the progress bar shows 0.
				NAME - Niranjan Deshak. - 10th Jan 2015
			*/
			//Sleep(10);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXAntiRootkit::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	InsertItem                                                     
*  Description    :	Insert item into list control
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CISpyAntiRootkit::InsertItem(CString csFirstParam, CString csSecondParam,CString csThirdParam, CString csForthParam)
{
	LVITEM lvItem;
	int nItem=0;
	int imgNbr = 0;

	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;

	
	CString cstmp;
	if(m_dwForInsertItem == 0x01)//Process
	{
		m_lstAntiRookit.InsertItem(&lvItem);
		m_lstAntiRookit.SetItemText(nItem, 0, csSecondParam);
		m_lstAntiRookit.SetItemText(nItem, 1, csFirstParam);
		m_lstAntiRookit.SetItemText(nItem, 2, csThirdParam);
		m_lstAntiRookit.SetItemText(nItem, 3, csForthParam);
		m_lstAntiRookit.SetCheck(nItem, TRUE);
		//ISSue No 307 Neha Gharge 28/5/2014 **************************************/
		AddEntriesInReportsDB(L"RootKit Scan",csFirstParam,csThirdParam,csForthParam);
	}
	else if(m_dwForInsertItem == 0x02)//Registry
	{
		//*********************ISSUE 306***********************************************************// 
		m_lstRegistry.InsertItem(&lvItem);
		m_lstRegistry.SetItemText(nItem, 0, csThirdParam );
		m_lstRegistry.SetItemText(nItem, 1, csFirstParam );
		m_lstRegistry.SetItemText(nItem, 2, csForthParam);
	//	m_lstRegistry.SetItemText(nItem, 3, csForthParam);
		m_lstRegistry.SetCheck(nItem, TRUE);
		//ISSue No 307 Neha Gharge 28/5/2014 **************************************/
		AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), csThirdParam, csFirstParam, csForthParam);
	}

	else if(m_dwForInsertItem == 0x03)//fileorfolders or driverpath
	{
		m_lstFilesFolders.InsertItem(&lvItem);
		m_lstFilesFolders.SetItemText(nItem, 0, csFirstParam);
		//m_lstFilesFolders.SetItemText(nItem, 1, csThirdParam);
		m_lstFilesFolders.SetItemText(nItem, 1, csForthParam);
		m_lstFilesFolders.SetCheck(nItem, TRUE);
		//ISSue No 307 Neha Gharge 28/5/2014 **************************************/
		AddEntriesInReportsDB(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ENTRY"), L"NA", csFirstParam, csForthParam);
		
	}

}

/**********************************************************************************************************                     
*  Function Name  :	AddDetectedCount                                                     
*  Description    :	Add detected count of files/registry/process.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CISpyAntiRootkit::AddDetectedCount(DWORD dwDetectedEntries)
{
	if(m_dwType == 0x01)
	{
		
		m_dwThreatCnt = dwDetectedEntries;
		m_csScannedSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_PROC"));
		m_csThreatsfound.Format(L": %d",m_dwThreatCnt);
		m_stThreatsFound.SetWindowTextW(m_csThreatsfound);
		
	}
	if(m_dwType == 0x02)
	{
		
		m_dwThreatCntRegistry = dwDetectedEntries;
		m_stRegistrySubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_REG"));
        m_csThreatsfound.Format(L": %d",m_dwThreatCntRegistry);
		m_stThreatsFound.SetWindowTextW(m_csThreatsfound);
		
	}
	if(m_dwType == 0x03)
	{
		m_dwThreatCntFilefolder = dwDetectedEntries;
		m_stFilesSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_FILE"));
		m_csThreatsfound.Format(L": %d",m_dwThreatCntFilefolder);
		m_stThreatsFilesFolders.SetWindowTextW(m_csThreatsfound);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	AddTotalCount                                                     
*  Description    :	Add total count of files/registry/process count
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CISpyAntiRootkit::AddTotalCount(DWORD dwTotalEntries)
{
	if(m_dwTotalType == 0x01)
	{
		m_dwScannedCnt = dwTotalEntries;
		//m_csScannedText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_PROC"));
		m_csScannedEntries.Format(L": %d",m_dwScannedCnt);
		m_stScannedEntries.SetWindowTextW(m_csScannedEntries);
		
	}
	if(m_dwTotalType == 0x02)
	{
		m_dwScannedCntRegistry= dwTotalEntries;
		//m_stRegisteryText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_REG"));
		m_csScannedEntries.Format(L": %d",m_dwScannedCntRegistry);
		m_stScannedRegistry.SetWindowTextW(m_csScannedEntries);
	}
	if(m_dwTotalType == 0x03)
	{
		m_dwScannedFileFolder = dwTotalEntries;
		//m_stFileText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_FILE"));
		m_csScannedEntries.Format(L": %d",m_dwScannedFileFolder);
		m_stScannedFilesFolders.SetWindowTextW(m_csScannedEntries);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnNMCustomdrawListFilesfolders                                                     
*  Description    : Customization of processes list control
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnNMCustomdrawListAntirootkit(NMHDR *pNMHDR, LRESULT *pResult)
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
			if(pLVCD->iSubItem == 3)
			{
				CString csStatus = m_lstAntiRookit.GetItemText(static_cast<int>(pLVCD->nmcd.dwItemSpec), 3) ;
				if(DETECTEDSTATUS == csStatus || FAILEDSTATUS == csStatus)
				{
					pLVCD->clrText = RGB(255, 0, 0);
				}
				else if(DELETEDSTATUS == csStatus || FILEREPAIRED == csStatus)
				{
					pLVCD->clrText = RGB(171,238,0);
				}
			}
		}
		break;
	}
	m_bAntirootkitCompleted = false;
	m_bAntirootScanningInProgress = false;
}

/**********************************************************************************************************                     
*  Function Name  :	OnNMCustomdrawListFilesfolders                                                     
*  Description    : Customization of registry list control
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnNMCustomdrawListRegistry(NMHDR *pNMHDR, LRESULT *pResult)
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
				CString csStatus = m_lstRegistry.GetItemText(static_cast<int>(pLVCD->nmcd.dwItemSpec), 2);
				if(DETECTEDSTATUS == csStatus || FAILEDSTATUS == csStatus)
				{
					pLVCD->clrText = RGB(255, 0, 0);
				}
				else if(DELETEDSTATUS == csStatus || FILEREPAIRED == csStatus)
				{
					pLVCD->clrText = RGB(171,238,0);
				}
			}
		}
		break;
	}
	m_bAntirootkitCompleted = false;
	m_bAntirootScanningInProgress = false;
}

/**********************************************************************************************************                     
*  Function Name  :	OnNMCustomdrawListFilesfolders                                                     
*  Description    : Customization of files folders list control
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnNMCustomdrawListFilesfolders(NMHDR *pNMHDR, LRESULT *pResult)
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
			if(pLVCD->iSubItem == 1)
			{
				CString csStatus = m_lstFilesFolders.GetItemText(static_cast<int>(pLVCD->nmcd.dwItemSpec), 1);
				if(DETECTEDSTATUS == csStatus || FAILEDSTATUS == csStatus)
				{
					pLVCD->clrText = RGB(255, 0, 0);
				}
				else if(DELETEDSTATUS == csStatus || FILEREPAIRED == csStatus)
				{
					pLVCD->clrText = RGB(171,238,0);
				}
			}
		}
		break;
	}
	m_bAntirootkitCompleted = false;
	m_bAntirootScanningInProgress = false;
}

/**********************************************************************************************************                     
*  Function Name  :	ShutDownRootkitCleaning                                                     
*  Description    :	Shutdown rootkit scan while cleaning
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::ShutDownRootkitCleaning()
{
	if(!g_bIsRootkitCleaning)
	{
		return true;
	}
	try
	{
		m_bIsPopUpDisplayed = true;
		int iReturn = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ROOTKITCLEAN"), 
			theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		m_bIsPopUpDisplayed = false;

		if(iReturn == IDNO)
		{
			return false;
		}

		else if(iReturn == IDYES && m_bAntirootClose && g_bIsRootkitCleaning == false)
		{
			return true;
		}

		else if(iReturn == IDYES && m_bAntirootClose)
		{

			if(m_hCleaningThread != NULL)
			{
				ResumeThread(m_hCleaningThread);
				TerminateThread(m_hCleaningThread, 0);
				m_hCleaningThread = NULL;
			}

			if(m_bAntirootClose)
			{
				m_bIsPopUpDisplayed = true;
				//MessageBox( TEXT("Rootkit cleaning is aborted"), TEXT("Wardwiz"), MB_ICONEXCLAMATION);
			    MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ROOTKIT_CLEANABORT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				m_bIsPopUpDisplayed = false;

				g_bIsRootkitCleaning = false;
			}

		}

	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXAntiRootkit::ShutDownRootkitCleaning", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ShowListcontrolOnBasisofSelection                                                     
*  Description    : Shows list control on the basis of tab selection
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::ShowListcontrolOnBasisofSelection()
{
	if(m_bRegistryTabSelected)
	{
		m_bRegistryTabSelected = true;
		m_lstRegistry.ShowWindow(true);
		m_lstAntiRookit.ShowWindow(false);
		m_lstFilesFolders.ShowWindow(false);
		m_stScannedEntries.ShowWindow(false);
		m_stThreatsFound.ShowWindow(false);
		m_stScannedFilesFolders.ShowWindow(false);
		m_stThreatsFilesFolders.ShowWindow(false);
		m_bFilesFoldersTabSelected = false;
		m_bProcessTabSelected = false;
		m_stRegisteryText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_REG"));
		m_stRegisteryText.ShowWindow(true);
		m_csScannedEntries.Format(L": %d",m_dwScannedCntRegistry);
		m_stScannedRegistry.SetWindowTextW(m_csScannedEntries);
		m_stScannedRegistry.ShowWindow(true);
		m_stRegistrySubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_REG"));
		m_stRegistrySubText.ShowWindow(true);
		m_csThreatsfound.Format(L": %d",m_dwThreatCntRegistry);
		m_stThreatsRegistry.SetWindowTextW(m_csThreatsfound);
		/*****************ISSUE NO -79 Neha Gharge 22/5/14 ************************************************/
		m_stThreatsRegistry.ShowWindow(true);
		m_chkWindowRegistry.ShowWindow(true);
		m_chkFileFolder.ShowWindow(false);
		m_chkAntiRootkitSelectAll.ShowWindow(false);
		
	}
	else if(m_bFilesFoldersTabSelected)
	{
		
		m_bFilesFoldersTabSelected = true;
		m_lstFilesFolders.ShowWindow(true);
		m_lstAntiRookit.ShowWindow(false);
		m_lstRegistry.ShowWindow(false);
		m_stScannedEntries.ShowWindow(false);
		m_stThreatsFound.ShowWindow(false);
		m_stScannedRegistry.ShowWindow(false);
		m_stThreatsRegistry.ShowWindow(false);
		m_bProcessTabSelected = false;
		m_bRegistryTabSelected = false;
		m_stFileText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_FILE"));
		m_stFileText.ShowWindow(true);
		m_csScannedEntries.Format(L": %d",m_dwScannedFileFolder);
		m_stScannedFilesFolders.SetWindowTextW(m_csScannedEntries);
		m_stScannedFilesFolders.ShowWindow(true);
		m_stFilesSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_FILE"));
		m_stFilesSubText.ShowWindow(true);
		m_csThreatsfound.Format(L": %d",m_dwThreatCntFilefolder);
		m_stThreatsFilesFolders.SetWindowTextW(m_csThreatsfound);
		/*****************ISSUE NO -79 Neha Gharge 22/5/14 ************************************************/
		m_stThreatsFilesFolders.ShowWindow(true);
		m_chkWindowRegistry.ShowWindow(false);
		m_chkFileFolder.ShowWindow(true);
		m_chkAntiRootkitSelectAll.ShowWindow(false);
	}
	else if(m_bProcessTabSelected)
	{
		m_bProcessTabSelected = true;
		m_lstAntiRookit.ShowWindow(true);
		m_lstRegistry.ShowWindow(false);
		m_lstFilesFolders.ShowWindow(false);
		m_stScannedRegistry.ShowWindow(false);
		m_stThreatsRegistry.ShowWindow(false);
		m_stScannedFilesFolders.ShowWindow(false);
		m_stThreatsFilesFolders.ShowWindow(false);
		m_bFilesFoldersTabSelected = false;
		m_bRegistryTabSelected = false;
		m_csScannedText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_PROC"));
		m_csScannedText.ShowWindow(true);
		m_csScannedEntries.Format(L": %d",m_dwScannedCnt);
		m_stScannedEntries.SetWindowTextW(m_csScannedEntries);
		m_stScannedEntries.ShowWindow(true);
		m_csScannedSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_PROC"));
		m_csScannedSubText.ShowWindow(true);
		m_csThreatsfound.Format(L": %d",m_dwThreatCnt);
		m_stThreatsFound.SetWindowTextW(m_csThreatsfound);
		/*****************ISSUE NO -79 Neha Gharge 22/5/14 ************************************************/
		m_stThreatsFound.ShowWindow(true);
		m_chkWindowRegistry.ShowWindow(false);
		m_chkFileFolder.ShowWindow(false);
		m_chkAntiRootkitSelectAll.ShowWindow(true);
	}
	Invalidate();
}

/***********************************************************************************************
*  Function Name  : RefreshString
*  Description    : this function is  called for setting the Text UI with different Language Support
*  Author Name    : Amit Dutta
*  SR_NO	         :
*  Date           : 30 April 2014
***********************************************************************************************/

void CISpyAntiRootkit::RefreshStrings()
{
 m_stAntirootkitHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_HEADER"));
 m_stAntirootkitSubHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SUBHEADER"));
 m_stSelectionText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_ROOTKITSCAN_OPTION"));
 m_stProcess.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_HIDDENPROC"));
 m_stRegistry.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_HIDDENREG"));
 m_stFilesFolders.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_HIDDENFILEFLDR"));
 m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
 //m_btnPauseResume.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"));
}

/***********************************************************************************************
*  Function Name  : OnBnClickedButtonAntirootkit
*  Description    : this function is  called for setting the UI back to normal condition
*  Author Name    : Amit Dutta
*  SR_NO			 :
*  Date           : 9th May 2014
***********************************************************************************************/

void CISpyAntiRootkit::OnBnClickedButtonAntirootkit()
{
    HideControls4ScannedList(true);
	HideChildControls(false);
	ShowHideControls(true);
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonBack                                                     
*  Description    :	on click of back button on second UI. To show first UI
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnBnClickedButtonBack()
{
	
	HideChildControls(false);
	HideControls4ScannedList(true);
	ShowHideControls(true);
	m_prgAntiRootkit.SetPos(0);
	m_btnScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));

	// TODO: Add your control notification handler code here
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckForWindowregistry                                                     
*  Description    : After clicking on select all check box of window registry
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnBnClickedCheckForWindowregistry()
{
	int iCheck = m_chkAntiRootkitSelectAll.GetCheck();

	if(m_bRegistryTabSelected)
	{
		for(int i = 0; i < m_lstRegistry.GetItemCount(); i++)
		{
			m_lstRegistry.SetCheck(i, iCheck);
		}
	}

}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckFilefolder                                                    
*  Description    : After clicking on select all check box of files and folders
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnBnClickedCheckFilefolder()
{
	// TODO: Add your control notification handler code here
	int iCheck = m_chkAntiRootkitSelectAll.GetCheck();

	if(m_bFilesFoldersTabSelected)
	{
		for(int i = 0; i < m_lstFilesFolders.GetItemCount(); i++)
		{
			m_lstFilesFolders.SetCheck(i, iCheck);
		}
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickBtnAntirootkitUI                                                    
*  Description    :	Show Antirootkit UI
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
/*****************ISSUE NO -104,197,308,86 Neha Gharge 22/5/14 ************************************************/
bool CISpyAntiRootkit::OnBnClickBtnAntirootkitUI()
{
	m_lstFilesFolders.DeleteAllItems();
	m_lstRegistry.DeleteAllItems();
	m_lstAntiRookit.DeleteAllItems();
	HideControls4ScannedList(true);
	HideChildControls(false);
	ShowHideControls(true);
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickBtnFirstAntirootkitUI                                                     
*  Description    :	Show First UI of antiroot kit after completing scanning
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::OnBnClickBtnFirstAntirootkitUI()
{
	m_lstFilesFolders.DeleteAllItems();
	m_lstRegistry.DeleteAllItems();
	m_lstAntiRookit.DeleteAllItems();
	HideControls4ScannedList(true);
	HideChildControls(false);
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickBtnSecondAntirootkitUI                                                     
*  Description    :	Show second UI of antiroot kit after completing scanning
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::OnBnClickBtnSecondAntirootkitUI()
{
	ShowAntiRootkitScannedList();
	m_bAntirootkitCompleted = false;
	m_bAntirootScanningInProgress = false;
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	RootKitUIDispalyedOnbasisOfFlags                                                     
*  Description    :	Display Antirootkit UI on the basis of flags
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 28 May 2014
**********************************************************************************************************/
bool CISpyAntiRootkit::RootKitUIDispalyedOnbasisOfFlags()
{
	if(m_bAntirootScanningInProgress == true)
	{
		OnBnClickBtnFirstAntirootkitUI();
	}
	else if(m_bAntirootkitCompleted == true)
	{
		OnBnClickBtnSecondAntirootkitUI();
	}
	else
	{
		OnBnClickBtnAntirootkitUI();
	}
	return true;
}


/***************************************************************************
*  Function Name  : AddEntriesInReportsDB
*  Description    : Add entries into reports
*  Author Name    : Neha gharge
*  SR_NO          :
*  Date           : 28th may 2014
*  Modification   : 28th may 2014
****************************************************************************/
void CISpyAntiRootkit::AddEntriesInReportsDB(CString csScanType, CString csThreatName, CString csFilePath, CString csAction)
{
	//TCHAR szDatebuff[9];
	//TCHAR szDate[9];
	//_wstrdate_s(szDatebuff,9);
	//_tcscpy(szDate,szDatebuff);

	//Issue not reported Neha Gharge 31/5/2014

	CTime ctDateTime = CTime::GetCurrentTime();
	CString csAntirootkitReportEntry = L"";					
	CString csTime =  ctDateTime.Format(_T("%H:%M:%S"));

	//OutputDebugString(L">>> AddinReports " + csFilePath);

	 SYSTEMTIME  CurrTime = {0} ;
	 GetLocalTime( &CurrTime ) ;//Ram, Issue resolved:0001218
	 CTime Time_Curr( CurrTime ) ;
	 int iMonth = Time_Curr.GetMonth();
	 int iDate = Time_Curr.GetDay();
	 int iYear = Time_Curr.GetYear();

	 CString csDate = L"";
	 csDate.Format(L"%d/%d/%d",iMonth,iDate,iYear);


	CString csDateTime = L"";
	csDateTime.Format(_T("%s %s"),csDate,csTime);
	csAntirootkitReportEntry.Format(L"%s#%s#%s#%s#%s#",csDateTime,csScanType,csThreatName,csFilePath,csAction);
	//CIspyList newEntry(csDateTime,csScanType,csThreatName,csFilePath,csAction);
	if(!SendReportOperations2Service(REPORT_ANTIROOTKIT_ENTRY,csAntirootkitReportEntry,0x05,true))
	{
		AddLogEntry(L"### Error in CGenXAntiRootkit::AddEntriesInReportsDB", 0, 0, true, SECONDLEVEL);
		return;
	}
	Sleep(5);
}

/**********************************************************************************************************                     
*  Function Name  :	OnPaint                                                     
*  Description    :	The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Jan 2014
**********************************************************************************************************/
void CISpyAntiRootkit::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnStop                                                   
*  Description    :	to stop Antirootkit scanning
*  Author Name    : lalit kumawat 
*  SR_NO		  :
*  Date           : 10/29/2014
**********************************************************************************************************/
// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.
void CISpyAntiRootkit::OnBnClickedBtnStop()
{
	// lalit 4-28-2015 , if we close on stop button and same time exit from tray then two popup comes , now single popup will come
	g_TabCtrlRtktWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();
	if (g_TabCtrlRtktWindHandle != NULL)
	{
		g_TabCtrlRtktWindHandle->m_bIsCloseHandleCalled = true;
		//m_bAntirootClose = true;
		if (!ShutDownRootkitScanning())
		{
			g_TabCtrlRtktWindHandle->m_bisUIcloseRquestFromTray = false;
		}
		else
		{
			m_bAntirootkitScan = false; //Amit Dutta 
			
			KillTimer(TIMER_ROOTKIT_SCAN_STATUS);
		}
		g_TabCtrlRtktWindHandle->m_bIsCloseHandleCalled = false;
		if (g_TabCtrlRtktWindHandle->m_bisUIcloseRquestFromTray)
		{
			g_bIsRootkitScanning = false;
			//lalit kumawat 5-5-2015 need to close tray manually if we override already "Do you want to close pop to UI Close"
			g_TabCtrlRtktWindHandle->CloseSystemTray();
			g_TabCtrlRtktWindHandle->HandleCloseButton();
		}
			

	}
	
}


/***************************************************************************
Function Name  : SetRootkitTotalNDetectedCount
Description    : Set Total Count and Detected Entries From the Hidden Files/Folders and Hidden Processes and Hidden Drivers on Tab control on UI.
Author Name    : Niranjan Deshak
S.R. No        : WRDWIZRKSCNDLL_041
Date           : 05/03/2015
****************************************************************************/
void CISpyAntiRootkit::SetRootkitTotalNDetectedCount(DWORD dwTotalFileCnt, DWORD dwTotalDriveCnt, DWORD dwTotalProcCnt, DWORD dwDetectedFileCnt, DWORD dwDetectedDriveCnt, DWORD dwDetectedProcCnt)
{
	
	//For FilenFolder Total and Detected cnt
	m_dwScannedFileFolder = dwTotalFileCnt;
	//m_stFileText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_FILE"));
	m_csScannedEntries.Format(L": %d", m_dwScannedFileFolder);
	m_stScannedFilesFolders.SetWindowTextW(m_csScannedEntries);


	m_dwThreatCntFilefolder =   dwDetectedFileCnt;
	m_stFilesSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_FILE"));
	m_csThreatsfound.Format(L": %d", m_dwThreatCntFilefolder);
	m_stThreatsFilesFolders.SetWindowTextW(m_csThreatsfound);
	

	//For Drivers Total and Detected cnt
	m_dwScannedCntRegistry = dwTotalDriveCnt;
	//m_stRegisteryText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_REG"));
	m_csScannedEntries.Format(L": %d", m_dwScannedCntRegistry);
	m_stScannedRegistry.SetWindowTextW(m_csScannedEntries);
	

	m_dwThreatCntRegistry = dwDetectedDriveCnt;
	m_stRegistrySubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_REG"));
	m_csThreatsfound.Format(L": %d", m_dwThreatCntRegistry);
	m_stThreatsFound.SetWindowTextW(m_csThreatsfound);
	

	//For Proc Total and Detected cnt
	m_dwScannedCnt = dwTotalProcCnt;
	//m_csScannedText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_SCAN_PROC"));
	m_csScannedEntries.Format(L": %d", m_dwScannedCnt);
	m_stScannedEntries.SetWindowTextW(m_csScannedEntries);
	

	m_dwThreatCnt = dwDetectedProcCnt;
	m_csScannedSubText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_DETECT_PROC"));
	m_csThreatsfound.Format(L": %d", m_dwThreatCnt);
	m_stThreatsFound.SetWindowTextW(m_csThreatsfound);
	

	

}