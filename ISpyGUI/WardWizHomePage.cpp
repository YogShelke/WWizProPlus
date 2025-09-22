/**********************************************************************************************************                     
	  Program Name          : WardWizHomePage.cpp
	  Description           : Main Home Page Inner Dialog for WardWiz
	  Author Name			: Rajil Yadav	                                                                       	 
	  Date Of Creation      : 30th April 2014
	  Version No            : 0.0.0.1
***********************************************************************************************************/
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "ISpyGUIDlg.h"
#include "WardWizHomePage.h"

using namespace std;
IMPLEMENT_DYNAMIC(CWardWizHomePage, CDialog)

/**********************************************************************************************************                     
* Function Name         : CWardWizHomePage
* Description           : C'tor
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 30th April 2014
* SR_No                 : 
***********************************************************************************************************/
CWardWizHomePage::CWardWizHomePage(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWardWizHomePage::IDD, pParent)
	,m_DaysText(NULL)
	,m_NoDaysText(NULL)
	//,m_bRegistrationInProcess(false)
	,m_bNonGenuineCopy(false)
	, m_bIsPopUpDisplayed(false)
{

}

/**********************************************************************************************************                     
* Function Name         : ~CWardWizHomePage
* Description           : D'tor
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 30th April 2014
* SR_No                 : 
***********************************************************************************************************/
CWardWizHomePage::~CWardWizHomePage()
{

	if(m_DaysText != NULL)
	{
		delete m_DaysText;
		m_DaysText = NULL;
	}
	if(m_NoDaysText != NULL)
	{
		delete m_NoDaysText;
		m_NoDaysText = NULL;
	}
}

/**********************************************************************************************************                     
* Function Name         : DoDataExchange
* Description           : Called by the framework to exchange and validate dialog data.
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 30th April 2014
* SR_No                 : 
***********************************************************************************************************/
void CWardWizHomePage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_NOT_PROTECTED, m_btnNonProtect);
	DDX_Control(pDX, IDC_STATIC_HEADER_IMAGE, m_stUiHeaderPic);
	DDX_Control(pDX, IDC_STATIC_CALENDAR, m_stCalendar);
	//DDX_Control(pDX, IDC_BUTTON1, m_btnRegisterNow);
	DDX_Control(pDX, IDC_STATIC_SCAN_THREAT, m_stScanPic);
	//DDX_Control(pDX, IDC_STATIC_DUMMY_TEXT, m_stDummyText);
	DDX_Control(pDX, IDC_STATIC_THEAT_DETAILS, m_stThreatDetails);
	DDX_Control(pDX, IDC_STATIC_LAST_SCAN_TYPE, m_stLastScanType);
	DDX_Control(pDX, IDC_STATIC_LAST_SCAN_DATE, m_stLastScanDate);
	DDX_Control(pDX, IDC_STATIC_DATABASE_VERSION, m_stDatabaseVersion);
	DDX_Control(pDX, IDC_STATIC_LAST_UPDATE_TIME, m_stLastUpdateTimeAndDate);
	DDX_Control(pDX, IDC_STATIC_LASTUPDATE_TIME, m_stLastUpdateTime);
	DDX_Control(pDX, IDC_STATIC_LAST_UPDATE_DATE, m_stLastUpdateDate);
	DDX_Control(pDX, IDC_STATIC_VERSION_NO, m_stVersionNo);
	DDX_Control(pDX, IDC_STATIC_SCAN_DATE, m_stlastScanDate);
	DDX_Control(pDX, IDC_STATIC_SCAN_TYPE, m_stScanType);
	DDX_Control(pDX, IDC_STATIC_TRAIL_VERSION, m_stTrailVersion);
	DDX_Control(pDX, IDC_BUTTON_PROTECT_BUTTON, m_btnProtectButton);
	DDX_Control(pDX, IDC_STATIC_DAYLEFTTEXT, m_stDaysLefttex);
	DDX_Control(pDX, IDC_STATIC_NO_OF_DAYS, m_stNoOFDaysText);
	DDX_Control(pDX, IDC_STATIC_UI_HEADER, m_stUIHeader);
	DDX_Control(pDX, IDC_STATIC_NOT_PROTECTED, m_stNonProtectMsg);
	DDX_Control(pDX, IDC_STATIC_NON_PROTECT_IMG, m_stNonProtectPic);
	DDX_Control(pDX, IDC_BUTTON_REGISTER_BUTTON, m_btnRegisterButton);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_stTextVar);
	DDX_Control(pDX, IDC_BUTTON_FIX_IT_NOW, m_btnFixItNow);
}

/**********************************************************************************************************                     
* Function Name         : MESSAGE_MAP
* Description           : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 30th April 2014
* SR_No                 : 
***********************************************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizHomePage, CJpegDialog)
	ON_WM_PAINT()
//	ON_BN_CLICKED(IDC_BUTTON_PROTECT_BUTTON, &CWardWizHomePage::OnBnClickedButtonProtectButton)
//	ON_BN_CLICKED(IDC_BUTTON_NOT_PROTECTED, &CWardWizHomePage::OnBnClickedButtonNotProtected)
//	ON_BN_CLICKED(IDC_BUTTON1, &CWardWizHomePage::OnBnClickedButtonRegisterNow)
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_REGISTER_BUTTON, &CWardWizHomePage::OnBnClickedButtonRegisterButton)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_FIX_IT_NOW, &CWardWizHomePage::OnBnClickedButtonFixItNow)
END_MESSAGE_MAP()




// CWardWizHomePage message handlers
/**********************************************************************************************************                     
* Function Name         : OnInitDialog
* Description           : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 30th April 2014
* SR_No                 : 
***********************************************************************************************************/
BOOL CWardWizHomePage::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
/*
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
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ModifyStyle(0, WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX );

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	CString csNoofdays;
	if(!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_HOME_PAGE), _T("JPG")))
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_FAILMSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}

	// TODO:  Add extra initialization here
	Draw();
	
	this->GetClientRect(m_rect);
	CRgn		 rgn;
	rgn.CreateRectRgn(m_rect.left, m_rect.top, m_rect.right - 3, m_rect.bottom - 3/*, 41, 41*/);
	this->SetWindowRgn(rgn, TRUE);
	
	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));
	
	
	
	m_NoDaysText = new CFont;
	/*	ISSUE NO - Number of days left not coming proper  NAME - NITIN K. TIME - 3rd July 2014 */
	m_NoDaysText->CreatePointFont(420,L"Cambria",0);
	m_stNoOFDaysText.SetFont(&theApp.m_FontWWDaysLeftFont);
//	m_stNoOFDaysText.SetTextAlign(0);
	m_stNoOFDaysText.SetBkColor(RGB(208,209,210));
//	m_stNoOFDaysText.SetGradientColor(RGB(238,239,239));
//	m_stNoOFDaysText.SetVerticalGradient(1);
	m_stNoOFDaysText.SetTextColor(BLACK);
	m_stNoOFDaysText.SetWindowTextW(L"");	
	/*	ISSUE NO - alignment of Calender Digits was not proper NAME - NITIN K. TIME - 30th June 2014 */
	m_stNoOFDaysText.SetWindowPos(&wndTop, m_rect.left + 465,182,75,64, SWP_SHOWWINDOW);


	CFont *fontdaysleft = new CFont;
	fontdaysleft->CreatePointFont(140,L"Franklin Gothic Heavy",0);
	//m_stDaysLefttex
	m_stDaysLefttex.SetWindowPos(&wndTop, m_rect.left + 445,150,104,32, SWP_NOREDRAW);
	m_stDaysLefttex.SetFont(fontdaysleft);
	m_stDaysLefttex.SetTextAlign(1);
	m_stDaysLefttex.SetColor(RGB(243,75,39));
	m_stDaysLefttex.SetGradientColor(RGB(198,37,45));
	m_stDaysLefttex.SetVerticalGradient(1);
	m_stDaysLefttex.SetTextColor(WHITE);

	//Issue - Header must be bold . Rajil Yadav. 22/05/2014
	m_DaysText = new CFont;
	m_DaysText->CreatePointFont(120,L"Cambria",0);
//	m_stUIHeader.SetWindowTextW(L"Your computer is protected");
	m_stUIHeader.SetFont(&theApp.m_FontWWStartUpFontSubTitle);
	m_stUIHeader.SetTextColor(RGB(25,153,78));
	m_stUIHeader.SetBkColor(RGB(235, 231, 232));
	m_stUIHeader.SetWindowPos(&wndTop, m_rect.left +110,45,400,30, SWP_NOREDRAW);
	m_stUIHeader.ShowWindow(SW_HIDE);

	//Varada Ikhar, Date: 07-04-2015
//	m_stNonProtectMsg.SetWindowTextW(L"Your computer is Not protected");
	m_stNonProtectMsg.SetFont(&theApp.m_FontWWStartUpFontSubTitle);
	m_stNonProtectMsg.SetTextColor(RGB(255,15,78));
	m_stNonProtectMsg.SetBkColor(RGB(235, 231, 232));
	m_stNonProtectMsg.SetWindowPos(&wndTop, m_rect.left +110,42,390,30, SWP_NOREDRAW);
	//m_stNonProtectMsg.SetBkColor(RGB(0, 0, 0));
	//m_stNonProtectMsg.ShowWindow(SW_HIDE);

	m_stLastUpdateTimeAndDate.SetWindowPos(&wndTop, m_rect.left + 60,150,120,20, SWP_NOREDRAW);
	m_stLastUpdateTimeAndDate.SetFont(&theApp.m_fontWWTextNormal);
	m_stLastUpdateTimeAndDate.SetBkColor(RGB(255,255,255));
	
	//Varada Ikhar, Date: 24/03/2015
	//Not protected message needs to be changed.
	m_btnNonProtect.SetSkin(theApp.m_hResDLL, IDB_BITMAP_NON_PROTECT_BUTTON_NORMAL, IDB_BITMAP_NON_PROTECT_BUTTON_NORMAL, IDB_BITMAP_NON_PROTECT_BUTTON_NORMAL, IDB_BITMAP_NON_PROTECT_BUTTON_NORMAL, 0, 0, 0, 0, 0);
	m_btnNonProtect.SetWindowPos(&wndTop, m_rect.left + 47,38,46,28, SWP_NOREDRAW);
	//m_btnNonProtect.ShowWindow(SW_HIDE);

	//Varada Ikhar, Date: 07-04-2015
	//Not protected message needs to be changed.
	m_btnFixItNow.SetSkin(theApp.m_hResDLL, IDB_BITMAP_FIXDNW_PROTECTED_NORMAL, IDB_BITMAP_FIXDNW_PROTECTED_NORMAL, IDB_BITMAP_FIXDNW_PROTECTED_HOVER, IDB_BITMAP_FIXDNW_PROTECTED_NORMAL, IDB_BITMAP_FIXDNW_PROTECTED_HOVER, 0, 0, 0, 0);
	m_btnFixItNow.SetWindowPos(&wndTop, m_rect.left + 500, 40, 79, 25, SWP_NOREDRAW);
	m_btnFixItNow.SetTextColor(RGB(255, 255, 255));

	m_stDatabaseVersion.SetWindowPos(&wndTop, m_rect.left+60,180, 120,20, SWP_NOREDRAW);
	m_stDatabaseVersion.SetFont(&theApp.m_fontWWTextNormal);
	m_stDatabaseVersion.SetBkColor(RGB(255,255,255));
//	m_stDatabaseVersion.SetWindowTextW(L"Database Version:");


	m_stLastScanDate.SetWindowPos(&wndTop, m_rect.left+60,210,130,20, SWP_NOREDRAW);
	m_stLastScanDate.SetFont(&theApp.m_fontWWTextNormal);
	m_stLastScanDate.SetBkColor(RGB(255,255,255));
//	m_stLastScanDate.SetWindowTextW(L"Last Scan Date:");
	
	m_stLastScanType.SetWindowPos(&wndTop, m_rect.left+60,240,120,20, SWP_NOREDRAW);
	m_stLastScanType.SetFont(&theApp.m_fontWWTextNormal);
	m_stLastScanType.SetBkColor(RGB(255,255,255));
//	m_stLastScanType.SetWindowTextW(L"Last Scan Type:");

	
	m_stLastUpdateTime.SetWindowPos(&wndTop, m_rect.left + 243, 150,50,20, SWP_NOREDRAW);
//	m_stLastUpdateTime.SetFont(&m_BoldText);
	m_stLastUpdateTime.SetBkColor(RGB(255,255,255));
	m_stLastUpdateTime.SetWindowTextW(L"00:00:00");
	m_stLastUpdateTime.ShowWindow(FALSE);

	// Issue -  date and time not proper. Rajil Yadav 10/12/2014

	m_stLastUpdateDate.SetWindowPos(&wndTop, m_rect.left + 195,150,150,15, SWP_NOREDRAW);
//	m_stLastUpdateDate.SetFont(&m_BoldText);
	m_stLastUpdateDate.SetBkColor(RGB(255,255,255));
	m_stLastUpdateDate.SetWindowTextW(L"00/00/0000 00:00:00 ");
	
	m_stVersionNo.SetWindowPos(&wndTop,m_rect.left+195,180,100,20,SWP_NOREDRAW);
//	m_stVersionNo.SetFont(&m_BoldText);
	m_stVersionNo.SetBkColor(RGB(255,255,255));
	m_stVersionNo.SetWindowTextW(L"0");
	
	m_stlastScanDate.SetWindowPos(&wndTop, m_rect.left+195,210,200,20, SWP_NOREDRAW);
//	m_stlastScanDate.SetFont(&m_BoldText);
	m_stlastScanDate.SetBkColor(RGB(255,255,255));
	m_stlastScanDate.SetWindowTextW(L"00/00/0000 00:00:00");

	m_stScanType.SetWindowPos(&wndTop, m_rect.left+195,240,100,20, SWP_NOREDRAW);
//	m_stScanType.SetFont(&m_BoldText);
	m_stScanType.SetBkColor(RGB(255,255,255));
//	m_stScanType.SetWindowTextW(L"None");
	m_stScanType.SetFont(&theApp.m_fontWWTextNormal);

	//Varada Ikhar, Date: 24/03/2015
	//Not protected message needs to be changed.
	m_btnProtectButton.SetSkin(theApp.m_hResDLL, IDB_BITMAP_PROTECT_BUTTON, IDB_BITMAP_PROTECT_BUTTON, IDB_BITMAP_PROTECT_BUTTON, IDB_BITMAP_PROTECT_BUTTON, 0, 0, 0, 0, 0);
	m_btnProtectButton.SetWindowPos(&wndTop, m_rect.left +47,38,41,21, SWP_NOREDRAW);
	m_btnProtectButton.ShowWindow(SW_HIDE);

	m_bmpProtectedNew = LoadBitmap(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_PROTECTED_HEADER_PIC));
	m_stUiHeaderPic.SetBitmap(m_bmpProtectedNew);
	m_stUiHeaderPic.SetWindowPos(&wndTop, m_rect.left+40,30,55,57, SWP_NOREDRAW);
	m_stUiHeaderPic.ShowWindow(SW_HIDE);

	m_bmpNonProtectedNew = LoadBitmap(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_NON_PROTECT_HEADER_IMG));
	m_stNonProtectPic.SetBitmap(m_bmpNonProtectedNew);
	m_stNonProtectPic.SetWindowPos(&wndTop, m_rect.left+40,30,55,57, SWP_NOREDRAW);

	m_bmpDaysLeftPic = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_CALENDAR));
	m_stCalendar.SetBitmap(m_bmpDaysLeftPic);
	m_stCalendar.SetWindowPos(&wndTop,m_rect.left+433,130,126,121,SWP_NOREDRAW);

	m_bmpScanImg = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_THREAT_SCAN_IMG));
	m_stScanPic.SetBitmap(m_bmpScanImg);
	m_stScanPic.SetWindowPos(&wndTop,m_rect.left+20,335,60,58, SWP_NOREDRAW);

	//m_stTrailVersion.SetWindowPos(&wndTop, m_rect.left + 450,110,120,20, SWP_NOREDRAW);
	//m_stTrailVersion.SetFont(m_DaysText);
	//m_stTrailVersion.SetTextColor(RGB(88,88,90));
	//m_stTrailVersion.SetBkColor(RGB(255,255,255));
	m_stTrailVersion.ShowWindow(FALSE);
	//m_stTrailVersion.SetWindowTextW(L"Trail Version");

//	m_btnRegisterNow.ShowWindow(SW_HIDE);

	
	m_btnRegisterButton.SetSkin(theApp.m_hResDLL,IDB_BITMAP_REGISTER_NOW,IDB_BITMAP_REGISTER_NOW,IDB_BITMAP_REGISTER_NOW_HOVER,0,0,0,0,0,0);
	
	m_btnRegisterButton.SetWindowPos(&wndTop, m_rect.left + 433,265,126,25, SWP_NOREDRAW);
	//m_btnRegisterButton.SetTextColorA(RGB(1,1,1), 1,1);
	m_btnRegisterButton.SetTextColor(RGB(255,255,255));
	m_btnRegisterButton.SetFont(m_DaysText);


	CFont boldText;
	boldText.CreateFontW(10,8,0,0,FW_BOLD,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"FF_MODERN");

	m_stThreatDetails.ShowWindow(SW_SHOW);
//	m_stThreatDetails.SetWindowTextW(L"Threat details");
	m_stThreatDetails.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stThreatDetails.SetWindowPos(&wndTop, m_rect.left + 80,340,300,27, SWP_NOREDRAW);
	m_stThreatDetails.SetBkColor(RGB(88,88,90));
	m_stThreatDetails.SetTextColor(WHITE);

	//ISSUE NO:- 162 RY Date :- 21/5/2014
	
	m_stTextVar.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stTextVar.SetWindowPos(&wndTop, m_rect.left + 80,368,500,20, SWP_NOREDRAW);
	m_stTextVar.SetBackColor(RGB(86,86,86));
	m_stTextVar.SetAutoSize(false);
	m_stTextVar.SetFont(&theApp.m_fontWWTextNormal);
	m_stTextVar.SetTextColor(WHITE);
	DWORD temp = ReadMarqueeEntryFromRegistry();
	m_stTextVar.ShowWindow(temp);

	bLiveUpdateMsg=0;
	bVirusMsgDetect=0; 
	m_bNonGenuineCopy = false;
	RefreshStrings();
	StartiSpyAVTray();
	GetStringDetails();
	ShowHomepageControls(true);
	Invalidate(); //Name:Varada Ikhar, Date:08/01/2015, Version:1.8.3.11, Issue No:4, Description: After reinstalling showing zero days left even if the trail product is registered. 
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**********************************************************************************************************                     
* Function Name         : OnPaint
* Description           : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
void CWardWizHomePage::OnPaint()
{
		CPaintDC dc(this);
	//		int m_dwNoofDay = 300;
	
		if (IsIconic())
		{
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

		m_dwNoofDays = theApp.m_dwDaysLeft;
		if(m_dwNoofDays <= 30){
		dc.Rectangle(24,109,405,307);
		dc.Rectangle(415,307,578,109);
		}
		else
		{
			
			dc.Rectangle(24,109,578,307);
		}
	}
	else
	{

		if(m_dwNoofDays <= 30)
		{
		dc.Rectangle(24,109,405,307);
		dc.Rectangle(415,307,578,109);
		}
		else
		{
			dc.Rectangle(24,109,578,307);
		}

	///KillTimer(0);
	//SetTimer(0,50,NULL);
		//CJpegDialog::Draw();
		CJpegDialog::OnPaint();
	}

}
//Varada Ikhar, Date : 24/03/2015
// Non-protect message needs to be changed
//Commented the following function.
/**********************************************************************************************************                     
* Function Name         : OnBnClickedButtonProtectButton
* Description           : on Click on protect button
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
//void CWardWizHomePage::OnBnClickedButtonProtectButton()
//{
//	MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PROTECT_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION);
//	// TODO: Add your control notification handler code here
//}

/**********************************************************************************************************                     
* Function Name         : StartiSpyAVTray
* Description           : Lauching tray
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
void CWardWizHomePage::StartiSpyAVTray()
{
	AddLogEntry(L">>> GenXTRAY : Start GenX tray", 0, 0, true, FIRSTLEVEL);
	TCHAR szModulePath[MAX_PATH] = {0};
	TCHAR szFullPath[MAX_PATH] = {0};

	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	_tcscpy_s(szFullPath, szModulePath);
	_tcscat_s(szFullPath, L"\\WRDWIZTRAY.EXE");

	::ShellExecute(NULL, L"Open", szFullPath, L"", NULL, 0);
}

/**********************************************************************************************************                     
* Function Name         : SetNotProtectedMsg
* Description           : On Not protected message to be disaled
* Author Name			: Varada Ikhar	                                                                                           
* Date Of Creation      : 6th April 2015
* SR_No                 : 
***********************************************************************************************************/
void CWardWizHomePage::SetNotProtectedMsg()
{

	try
	{
		szEmailId_GUI = theApp.GetRegisteredEmailID();
		if (m_bNonGenuineCopy)
		{
			m_btnProtectButton.ShowWindow(SW_HIDE);
			m_stUiHeaderPic.ShowWindow(FALSE);
			m_stUIHeader.ShowWindow(FALSE);
			m_stNonProtectPic.ShowWindow(SW_SHOW);
			m_stNonProtectMsg.ShowWindow(TRUE);
			m_btnNonProtect.ShowWindow(TRUE);
			m_btnNonProtect.Invalidate();
			m_stNonProtectMsg.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_NOT_GENUINE"));
			m_btnRegisterButton.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTER_NOW"));
			m_btnFixItNow.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FIX_NOW_REGISTER"));
			m_bNonGenuineCopy = false;
			return;
		}

		if ((m_dwNoofDays <= 30) && (m_dwNoofDays >= 0) && (_tcslen(theApp.m_szRegKey) != 0))
		{
			m_btnRegisterButton.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_REGISTER_NOW_TO_RENEW_NOW"));
		}

		if (m_dwNoofDays == 0)
		{
					
			if(szEmailId_GUI && _tcslen(szEmailId_GUI) == 0)
			{
				//MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_UNREGISTERED") ,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION| MB_OK);
				m_stNonProtectMsg.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_UNREGISTERED"));
			}
			else
			{
				//MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_EXPIRED "),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
				//m_btnFixItNow.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FIX_NOW_RENEW"));
				if (theApp.m_szRegKey && _tcslen(theApp.m_szRegKey) == 0)
				{
					m_stNonProtectMsg.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_TRIAL_EXPIRED"));
				}
				else
				{
					m_btnRegisterButton.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_REGISTER_NOW_TO_RENEW_NOW"));
					m_btnFixItNow.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FIX_NOW_RENEW"));
					m_stNonProtectMsg.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_EXPIRED"));
				}
			}
			return;
		}

		if (bLiveUpdateMsg)
		{
			//MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_OLD_DATABASE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			//m_btnFixItNow.ShowWindow(SW_SHOW);
			m_stNonProtectMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_OLD_DATABASE"));
			bLiveUpdateMsg = 0;
			return;

		}
		//if(bVirusMsgDetect)
		//{
		//	//MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_NEW_PROTECT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
		//	m_stNonProtectMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_NEW_PROTECT"));
		//	return;
		//}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXHomePage::SetNotProtectedMsg()", 0, 0, true, SECONDLEVEL);
	}
}

/*
HCURSOR CWardWizHomePage::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

*/

/**********************************************************************************************************                     
* Function Name         : RegistryEntryOnUI
* Description           : Details of AV from Registry.
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
void CWardWizHomePage::RegistryEntryOnUI()
{
	try
	{
		HKEY key;
		if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\WardWiz Antivirus"),&key)!= ERROR_SUCCESS)
		{
			return;
		}

		DWORD dwValueOFOffline = sizeof(DWORD) + 1;
		DWORD dwOfflineTypeType = REG_DWORD;
		DWORD dwRegUserType = 0x00 ,dwNGC = 0x00;
		long ReadReg = RegQueryValueEx(key, L"dwRegUserType", NULL, &dwOfflineTypeType, (LPBYTE)&m_szvalueSType, &dwValueOFOffline);
		if (ReadReg == ERROR_SUCCESS)
		{
			dwRegUserType = (DWORD)m_szvalueSType;
		}
		ReadReg = RegQueryValueEx(key, L"dwNGC", NULL, &dwOfflineTypeType, (LPBYTE)&m_szvalueSType, &dwValueOFOffline);
		if (ReadReg == ERROR_SUCCESS)
		{
			dwNGC = (DWORD)m_szvalueSType;
	

			//Ram, Issue No: 0001216, Resolved
			if (theApp.m_dwDaysLeft == 0)
			{
				theApp.GetDaysLeft();
			}

			if (dwRegUserType == 1 && theApp.m_dwDaysLeft == 0 && dwNGC == 1)
			{
				m_btnProtectButton.ShowWindow(SW_HIDE);
				m_stUiHeaderPic.ShowWindow(FALSE);
				m_stUIHeader.ShowWindow(FALSE);
				m_stNonProtectPic.ShowWindow(SW_SHOW);
				m_stNonProtectMsg.ShowWindow(TRUE);
				m_btnNonProtect.ShowWindow(TRUE);
				m_btnNonProtect.Invalidate();
				m_bNonGenuineCopy = true;

			}
		}

		TCHAR szvalue[1024];
		DWORD dwvalue_length = 1024;
		DWORD dwtype=REG_SZ;
		TCHAR szDay[3] = {0};
		TCHAR szMonth[3] = {0};
		TCHAR szYear[5] = {0};
		TCHAR szCurrentDay[3] = {0};
		TCHAR szCurrentMonth[3] = {0};
		TCHAR szCurrentYear[3] = {0};
		TCHAR szCurrentBuffer[9] = {0};
		ReadReg=RegQueryValueEx(key, L"LastLiveupdatedt", NULL ,&dwtype,(LPBYTE)&szvalue, &dwvalue_length);
	
		//Neha Gharge 20-1-2015 
		//If the difference is 7 days further from date of last update then only unprotected message will disaplay
		//else protected message will be display.
		if(ReadReg == ERROR_SUCCESS)
		{
			m_lpstrDate=(LPCTSTR)szvalue;
			
			CString csCommandLine =  (CString)m_lpstrDate;
			int iPos = 0;
			int szTemp[3] = {0};
			for( int i=0; i< 3 ;i++)
			{
				CString csTemp = csCommandLine.Tokenize(_T("/"),iPos);
				szTemp[i] = _wtoi(csTemp);
			}
			int iYear = szTemp[2];
			int iDay = szTemp[1];
			int iMonth = szTemp[0];
	
			//Varada Ikhar, Date: 4/11/2015
			//Issue: If user changed the date of 'LastLiveUpdatdt' from registry to invalid date, then error pop-up gets displayed.
			//This case needs to be handled.
			//CTime Time_RegistryDate(iYear,iMonth,iDay,0,0,0);		
			
			//SYSTEMTIME  CurrTime = {0} ;
			CTime Time_Curr = CTime::GetCurrentTime() ;
			//CTime Time_Curr( CurrTime ) ;
			int iMonth1 = Time_Curr.GetMonth();
			int iDate1 = Time_Curr.GetDay();
			int iYear1 = Time_Curr.GetYear();

			//Varada Ikhar, Date: 4/11/2015
			//Issue: If user changed the date of 'LastLiveUpdatdt' from registry to invalid date, then error pop-up gets displayed.
			//This case needs to be handled.
			if (!isValidDate(iDay, iMonth, iYear) || !isValidDate(iDate1, iMonth1, iYear1))
			{
				AddLogEntry(L"### Invalid LastLiveupdatedt/GetCurrentTime in CGenXHomePage::RegistryEntryOnUI().", 0, 0, true, SECONDLEVEL);

				m_btnNonProtect.ShowWindow(SW_HIDE);
				m_stNonProtectMsg.ShowWindow(FALSE);
				m_stNonProtectPic.ShowWindow(SW_HIDE);
				m_stUIHeader.ShowWindow(SW_SHOW);
				m_btnProtectButton.ShowWindow(FALSE);
				m_btnFixItNow.ShowWindow(SW_HIDE);
				m_btnFixItNow.Invalidate();
				m_stUiHeaderPic.ShowWindow(TRUE);
				m_btnProtectButton.Invalidate();	
			}
			else
			{

				CTime Time_RegistryDate(iYear, iMonth, iDay, 0, 0, 0);

				CTime Time_CurDate(iYear1, iMonth1, iDate1, 0, 0, 0);

				CTimeSpan Time_Diff = Time_CurDate - Time_RegistryDate;

				int Span = static_cast<int>(Time_Diff.GetDays());

				if (Span > 7)
				{
					bLiveUpdateMsg = 1;
					bVirusMsgDetect = 0;
					m_btnProtectButton.ShowWindow(SW_HIDE);
					m_stUiHeaderPic.ShowWindow(FALSE);
					m_stUIHeader.ShowWindow(FALSE);
					m_stNonProtectPic.ShowWindow(SW_SHOW);				
					m_stNonProtectMsg.ShowWindow(TRUE);
					m_btnNonProtect.ShowWindow(TRUE);
					//Varada Ikhar, Date : 24/03/2015
					// Non-protect message needs to be changed.
					m_btnFixItNow.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"));
					m_btnFixItNow.ShowWindow(SW_SHOW);
					m_btnFixItNow.Invalidate();
	
					m_btnNonProtect.Invalidate();

				}
				else
				{
					//	m_stUiHeaderPic.ShowWindow(TRUE);
					m_btnNonProtect.ShowWindow(SW_HIDE);
					m_stNonProtectMsg.ShowWindow(FALSE);
					m_stNonProtectPic.ShowWindow(SW_HIDE);
					//m_stNonProtectMsg.ShowWindow(FALSE);
					m_stUIHeader.ShowWindow(SW_SHOW);
					//Varada Ikhar, Date : 24/03/2015
					// Non-protect message needs to be changed.
					//m_btnProtectButton.ShowWindow(TRUE);
					m_btnProtectButton.ShowWindow(FALSE);
					m_btnFixItNow.ShowWindow(SW_HIDE);
					m_btnFixItNow.Invalidate();
	
					m_stUiHeaderPic.ShowWindow(TRUE);
					m_btnProtectButton.Invalidate();

				}
			}
			
			m_stLastUpdateDate.SetWindowTextW(m_lpstrDate);
		}
		
		//TCHAR szvalueTM[1024];
		DWORD dwvalue_lengthTM = 1024;
		DWORD dwtypeTM=REG_SZ;
		ReadReg=RegQueryValueEx(key, L"LastLiveupdatetm", NULL ,&dwtypeTM,(LPBYTE)&m_szvalueTM, &dwvalue_lengthTM);
	
		if(ReadReg == ERROR_SUCCESS)
		{
			
			m_stLastUpdateTime.SetWindowTextW((LPCTSTR)m_szvalueTM);
		}
		
		
		TCHAR szvalueDT[1024];
		DWORD dwvalue_lengthDT = 1024;
		DWORD dwtypeDT=REG_SZ;
		
		ReadReg=RegQueryValueEx(key, L"LastScandt", NULL ,&dwtypeDT,(LPBYTE)&szvalueDT, &dwvalue_lengthDT);
		if(ReadReg == ERROR_SUCCESS)
		{
			LPCTSTR szLastScanDate=(LPCTSTR)szvalueDT;
			m_stlastScanDate.SetWindowTextW(szLastScanDate);
		}
		
	
		TCHAR szvalueVersion[1024];
		DWORD dwvaluelengthVersion = 1024;
		DWORD dwtypeVersion=REG_SZ;
		ReadReg=RegQueryValueEx(key, L"DataBaseVersion", NULL ,&dwtypeVersion,(LPBYTE)&szvalueVersion, &dwvaluelengthVersion);
		if(ReadReg == ERROR_SUCCESS)
		{
			m_stVersionNo.SetWindowTextW((LPCTSTR)szvalueVersion);
		}
		
	
		
		DWORD dwvaluelengthSType = sizeof(DWORD)+1;
		DWORD dwtypeSType=REG_DWORD;
		ReadReg=RegQueryValueEx(key, L"ScanType", NULL ,&dwtypeSType,(LPBYTE)&m_szvalueSType,&dwvaluelengthSType);
		if(ReadReg == ERROR_SUCCESS)
		{
				DWORD Scantype =(DWORD)m_szvalueSType;
				switch(Scantype)
				{
					case 0: m_stScanType.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FULL_SCAN"));
							break;
					case 1: m_stScanType.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CUSTOM_SCAN"));
							break;
					case 2: m_stScanType.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_QUICK_SCAN"));
							break;
					//Issue: On home UI, antirootkit scan and USB scan is not shown in last scan type.
					//Name: Niranjan Deshak - 3/2/2015.
					case 3: 
					case 4: m_stScanType.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_USB_SCAN "));
							break;
					case 5: m_stScanType.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_HEADER "));
							break;
					
				}
		}
		//Varada Ikhar, Date: 07-04-2015
		//DWORD szvalueVirusFound;
		//DWORD dwvaluelengthVirusFound = sizeof(szvalueVirusFound)+1;
		//DWORD dwtypeVirusFound=REG_DWORD;
		//ReadReg=RegQueryValueEx(key, L"VirusFound", NULL ,&dwtypeVirusFound,(LPBYTE)&szvalueVirusFound, &dwvaluelengthVirusFound);
		//if(ReadReg == ERROR_SUCCESS)
		//{
		//	DWORD VirusCount=(DWORD)szvalueVirusFound;
		//	if(VirusCount>0)
		//	{
		//		//ISSUE NO:- 52 RY Date :- 21/5/2014	
		//		bVirusMsgDetect=1;
		//		m_btnProtectButton.ShowWindow(FALSE);
		//		//GetDlgItem(IDC_STATIC_MAINUI_HEADER_PIC)->ShowWindow(FALSE);	
		//		//Varada Ikhar, Date: 07-04-2015
		//		m_stUIHeader.ShowWindow(TRUE);
		//		m_stUiHeaderPic.ShowWindow(TRUE);
		//		//m_btnNonProtect.ShowWindow(TRUE);
		//		//m_stNonProtectMsg.ShowWindow(TRUE);
		//		m_stNonProtectPic.ShowWindow(TRUE);
		//		m_btnNonProtect.Invalidate();
		//	}
		//	else if(VirusCount == 0 && bLiveUpdateMsg == 0)
		//	{
		//		
		//		m_btnNonProtect.ShowWindow(FALSE);
		//		m_stNonProtectMsg.ShowWindow(FALSE);
		//		m_stNonProtectPic.ShowWindow(FALSE);
		//		m_stUIHeader.ShowWindow(TRUE);
		//		m_stUiHeaderPic.ShowWindow(TRUE);
		//		//Varada Ikhar, Date : 24/03/2015
		//		// Non-protect message needs to be changed.
		//		//m_btnProtectButton.ShowWindow(TRUE);
		//		m_btnProtectButton.ShowWindow(FALSE);
		//		m_btnFixItNow.ShowWindow(SW_HIDE);
		//		m_btnProtectButton.Invalidate();
		//		//m_stUiHeaderPic.ShowWindow(TRUE);
		//		
		//	}
		//}
		RegCloseKey(key);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXHomePage::RegistryEntryOnUI()", 0, 0, true, SECONDLEVEL);
	}

}

/**********************************************************************************************************                     
* Function Name         : RefreshStrings
* Description           : this function is  called for setting the Text UI with different Language Support
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
void CWardWizHomePage::RefreshStrings()
{
	//m_btnQuickScan->SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_QUICKSCAN"));
	
	m_stThreatDetails.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_THREAT_DETALIS"));

	//Varada Ikhar, Date: 06-04-2015,
	//Not protected message needs to be changed.
	//m_stNonProtectMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_NOT_PROTECTED_MSG"));
	//m_btnFixItNow.SetWindowTextW(L"Fix It Now");

	m_btnRegisterButton.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTER_NOW"));
	//m_stTrailVersion.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_TRAIL_VERSION"));
	m_stScanType.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_TYPE"));
	m_stLastScanDate.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_DATE"));	
	m_stVersionNo.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_DATABASE_VERSION"));
	//m_stLastUpdateDate.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DATE"));
	m_stLastUpdateTime.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_TIME"));
	m_stLastScanType.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_LAST_SCAN_TYPE"));
	m_stLastScanDate.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_DATE_TEXT"));
	m_stDatabaseVersion.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_DATABASE_VER"));
	m_stUIHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PROTECT_MSG"));
	m_stDaysLefttex.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DAYS_LEFT"));
	m_stLastUpdateTimeAndDate.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_LAST_UPDATE_DATE"));
	
}

/**********************************************************************************************************                     
* Function Name         : OnNcHitTest
* Description           : The framework calls this member function for the CWnd object that contains the cursor (or the CWnd object that used the SetCapture member function to capture the mouse input) every time the mouse is moved.
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
LRESULT CWardWizHomePage::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/**********************************************************************************************************                     
* Function Name         : OnSetCursor
* Description           : The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within the CWnd object.
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
BOOL CWardWizHomePage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	/*if( iCtrlID == IDC_BUTTON_NOT_PROTECTED || iCtrlID == IDC_BUTTON_REGISTER_BUTTON || iCtrlID ==IDC_STATIC_TEXT ||
		iCtrlID == IDC_STATIC_TRAIL_VERSION)*/
	if (iCtrlID == IDC_BUTTON_REGISTER_BUTTON || iCtrlID == IDC_STATIC_TRAIL_VERSION || iCtrlID == IDC_STATIC_TRAIL_VERSION)
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if(csClassName == _T("Button") && m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}
	// TODO: Add your message handler code here and/or call default

	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}

/**********************************************************************************************************                     
* Function Name         : OnBnClickedButtonRegisterButton
* Description           : Registration Page is open
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
void CWardWizHomePage::OnBnClickedButtonRegisterButton()
{
	__try
	{
		//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
		//Neha Gharge 4 jan,2016
		m_bIsPopUpDisplayed = true;
		theApp.m_bRegistrationInProcess = true;
		theApp.DoRegistration();
		theApp.m_bRegistrationInProcess = false;
		m_bIsPopUpDisplayed = false;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CGenXGUIDlg::OnBnClickedButtonRegisternow", 0, 0, true, SECONDLEVEL);
	}
	theApp.GetDaysLeft();

	// ISsue no 691 neha gharge 13/6/2014
	CISpyGUIDlg * g_ObjIspyGUIHwnd = NULL;
	g_ObjIspyGUIHwnd = (CISpyGUIDlg *) (CISpyGUIDlg*)AfxGetMainWnd();
	if(g_ObjIspyGUIHwnd != NULL)
	{
		g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
	}
	//ShowHomepageControls(true);
	// TODO: Add your control notification handler code here
}

/**********************************************************************************************************                     
* Function Name         : PreTranslateMessage
* Description           : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
BOOL CWardWizHomePage::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
	
}

/**********************************************************************************************************                     
* Function Name         : GetStringDetails
* Description           : Displaying Latest Threat
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
bool CWardWizHomePage::GetStringDetails()
{
	//HANDLE hFile  = INVALID_HANDLE_VALUE;
	//CString	szExt ;
	//DWORD	dwLen = 0x00 ;
	DWORD  dwCountSecNewsEng = 0x00, dwCountSecNewsGerman = 0x00;
	TCHAR  szValueData[512] = { 0 };
	TCHAR  szValueName[512] = { 0 };
	//Issue: Security news should be available in german language as well
	//Resolved By: Nitin K Date: 11th Jan 2016
	CString csFilePathSecNews = theApp.GetModuleFilePath() + L"\\WRDWIZSECURITYNEWS.TXT";
	//CString szValue;

	if (!PathFileExists(csFilePathSecNews))
	{
		MessageBox(csFilePathSecNews + theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_MESSAGE_FILE_ERROR"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	CString csSpywareFullString;
	int i = 0;
	TCHAR szLine[MAX_PATH] = {0};
	//std::wfstream file;
	//file.open(csFilePathThreatDetails, std::ios::in);
	//while (!file.eof() && i < 5)
	//{
	//	file.getline(szLine, MAX_PATH * 2);
	//	if(szLine && *szLine && _tcslen(szLine) > 0)
	//	{//Issue - Add Unicode  RAjil 28/05/2014.,
	//		csSpywareFullString  += szLine;
	//		csSpywareFullString += _T(" \u2605 ");
	//		i++;
	//	}
	//}

	//file.close();
	dwCountSecNewsEng = GetPrivateProfileInt(L"SecNewsEnglish", L"Count", 0, csFilePathSecNews);
	dwCountSecNewsGerman = GetPrivateProfileInt(L"SecNewsGerman", L"Count", 0, csFilePathSecNews);

	if (dwCountSecNewsEng == 0x00 && dwCountSecNewsGerman == 0x00)
	{
		AddLogEntry(L">>> No Security news to display.", 0, 0, true, ZEROLEVEL);
		return false;
	}
	else
	{
		//CString csAppendstring(L"");
		//CString csReleaseInfo = theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_RELEASE_NOTE_NEW_FEATURES"); //L""; //

		switch (theApp.m_dwSelectedLangID)
		{
		case GERMAN:
			for (i = 1; i <= static_cast<int>(dwCountSecNewsGerman); i++)
			{

				swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
				GetPrivateProfileString(L"SecNewsGerman", szValueName, L"", szValueData, 511, csFilePathSecNews);
				if (!szValueData[0])
				{
					AddLogEntry(L"### Invalid Entries for(%s) in (%s) CGenXHomePage::GetStringDetails.", szValueName, csFilePathSecNews, true, 0x00);
				}
				else
				{
					csSpywareFullString.Format(L"%s %s", csSpywareFullString, szValueData);
					csSpywareFullString += _T(" \u2605 ");
				}
			}
			break;
		default:
		case ENGLISH:
			for (i = 1; i <= static_cast<int>(dwCountSecNewsGerman); i++)
			{

				swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
				GetPrivateProfileString(L"SecNewsEnglish", szValueName, L"", szValueData, 511, csFilePathSecNews);
				if (!szValueData[0])
				{
					AddLogEntry(L"### Invalid Entries for(%s) in (%s) CGenXHomePage::GetStringDetails.", szValueName, csFilePathSecNews, true, 0x00);
				}
				else
				{
					csSpywareFullString.Format(L"%s %s", csSpywareFullString, szValueData);
					csSpywareFullString += _T(" \u2605 ");
				}
			}
			break;
		}
	}
	//file.close();
	int iPos = csSpywareFullString.ReverseFind(',');
	 m_csTemp = csSpywareFullString.Left(csSpywareFullString.GetLength()- 2); 
//	m_csTemp += "...";
	
	//Issue No-  Speed of Scrolling Text is maintained Rajil Y. 22/05/2014
	LPCSTR str = (LPCSTR)(LPCTSTR)m_csTemp;
	m_stTextVar.AddString(reinterpret_cast<LPCTSTR>(str),1,RGB(88,88,90));
//	m_stTextVar.AddString(UpdateStringAppearance());
	m_stTextVar.SetScrollSpeed(400);
	//m_stTextVar.
	m_stTextVar.SetTextColor(WHITE);
	//UpdateStringAppearance();
	

	
	//m_stDummyText.SetMarquee(true, 1000);
	return true;
}

/**********************************************************************************************************                     
* Function Name         : OnCtlColor
* Description           : The framework calls this member function when a child control is about to be drawn.
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
HBRUSH CWardWizHomePage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID ==  IDC_STATIC_THEAT_DETAILS  || ctrlID == IDC_STATIC_TEXT  )

	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} return hbr;
}

/**********************************************************************************************************                     
* Function Name         : UpdateStringAppearance
* Description           : Update String appearance with color change.
* Author Name			: Rajil Yadav	                                                                                           
* Date Of Creation      : 7th May 2014
* SR_No                 : 
***********************************************************************************************************/
CColorString CWardWizHomePage::UpdateStringAppearance(void)
{
 //  UpdateData(TRUE);
   DWORD dwAddStyle = RGB(255,255,255);
   
   CColorString strAddAppearance(m_csTemp, dwAddStyle);

   m_stTextVar.SetString(0, strAddAppearance);

   return (strAddAppearance);
}

/**********************************************************************************************************                     
* Function Name    : ShowHomepageControls
* Description      : Show and hide main controls
* Author Name	   : Neha Gharge	                                                                                           
* Date Of Creation : 6th may 2014
* SR_NO            : 
***********************************************************************************************************/
void CWardWizHomePage::ShowHomepageControls(bool bEnable)
{
	m_bNonGenuineCopy = false;
	m_stNonProtectPic.ShowWindow(bEnable);
	m_stNoOFDaysText.ShowWindow(bEnable);
	m_stDaysLefttex.ShowWindow(bEnable);
	m_stUIHeader.ShowWindow(bEnable);
	m_stNonProtectMsg.ShowWindow(bEnable);
	m_btnNonProtect.ShowWindow(bEnable);
	m_stDatabaseVersion.ShowWindow(bEnable);
	m_stLastScanDate.ShowWindow(bEnable);
	m_stLastScanType.ShowWindow(bEnable);
	//m_stLastUpdateTime.ShowWindow(bEnable);
	m_stLastUpdateTimeAndDate.ShowWindow(bEnable);
	m_stLastUpdateDate.ShowWindow(bEnable);
	m_stVersionNo.ShowWindow(bEnable);
	m_stlastScanDate.ShowWindow(bEnable);
	m_stScanType.ShowWindow(bEnable);

	//Varada Ikhar, Date : 24/03/2015
	// Non-protect message needs to be changed.
	//m_btnProtectButton.ShowWindow(bEnable);
	m_btnProtectButton.ShowWindow(FALSE);
	
	m_stUiHeaderPic.ShowWindow(bEnable);
	m_stNonProtectPic.ShowWindow(bEnable);
	m_stCalendar.ShowWindow(bEnable);
	m_stScanPic.ShowWindow(bEnable);
//	m_stTrailVersion.ShowWindow(bEnable);
	m_btnRegisterButton.ShowWindow(bEnable);
	m_stThreatDetails.ShowWindow(bEnable);
	//m_stDummyText.ShowWindow(bEnable);

	RegistryEntryOnUI();
	//if(bLiveUpdateMsg == 1)
	//{
	//	bLiveUpdateMsg = 0;
	//}
	//
	
	
	//Varada Ikhar, Date: 07-04-2015
	//Not-protected message needs to be changed.
	if (theApp.m_GetDaysLeft)
	{
		theApp.m_dwDaysLeft = theApp.m_GetDaysLeft(theApp.m_dwProductID);
	}
	m_dwNoofDays = theApp.m_dwDaysLeft;
	if(m_dwNoofDays < 10)
	{
		m_csNoofdays.Format(L"0%d",m_dwNoofDays);
	}
	else
	{
		m_csNoofdays.Format(L"%d",m_dwNoofDays);
	}
	m_stNoOFDaysText.SetWindowTextW(m_csNoofdays);
	m_stNoOFDaysText.ShowWindow(SW_SHOW);
	m_dwNoofDays = theApp.m_dwDaysLeft;
	if(m_dwNoofDays > 30)
	{
		m_btnRegisterButton.ShowWindow(SW_HIDE);
		m_stCalendar.ShowWindow(SW_HIDE);
		//m_stDayLeftOutPic.ShowWindow(SW_HIDE);
		m_stDaysLefttex.ShowWindow(SW_HIDE);
		m_stNoOFDaysText.ShowWindow(SW_HIDE);
//		m_stTrailVersion.ShowWindow(SW_HIDE);
	}
	else if(m_dwNoofDays == 0)
	{
		m_stUIHeader.ShowWindow(FALSE);
		m_stUiHeaderPic.ShowWindow(FALSE);
		m_btnProtectButton.ShowWindow(FALSE);
		m_btnNonProtect.ShowWindow(TRUE);
		m_stNonProtectMsg.ShowWindow(TRUE);
		m_stNonProtectPic.ShowWindow(TRUE);

		//Varada Ikhar, Date: 07-04-2015
		//Not-protected message needs to be changed.
		m_btnFixItNow.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FIX_NOW_REGISTER"));
		m_btnFixItNow.ShowWindow(SW_SHOW);
		m_btnFixItNow.Invalidate();

//		m_stTrailVersion.ShowWindow(SW_HIDE);
		m_btnNonProtect.Invalidate();
	}

	//Varada Ikhar, Date: 07-04-2015
	//Not-protected message needs to be changed.
	SetNotProtectedMsg();
	Invalidate();
}

/**********************************************************************************************************                     
* Function Name     : ReadMarqueeEntryFromRegistry
* Description       : SRead Data From Registry.
* Author Name		: Neha Gharge	                                                                                           
* Date Of Creation  : 24th may 2014
* SR_NO				:
***********************************************************************************************************/
DWORD CWardWizHomePage::ReadMarqueeEntryFromRegistry()
{
	DWORD Setting = 0;
	HKEY key;
	DWORD szvalue;
	DWORD dwvaluelength = sizeof(DWORD);
	DWORD dwtype = REG_DWORD;

	if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus"),&key)!= ERROR_SUCCESS)
	{
		AddLogEntry(L"### Unable to open registry key in CGenXHomePage::ReadMarqueeEntryFromRegistry", 0, 0, true, SECONDLEVEL);
		return Setting;
	}
	
	long ReadReg=RegQueryValueEx(key, L"dwShowSecNews", NULL ,&dwtype,(LPBYTE)&szvalue, &dwvaluelength);
	if(ReadReg == ERROR_SUCCESS)
	{
		Setting = (DWORD)szvalue;
	}	
	RegCloseKey(key);

	return Setting;
}

/**********************************************************************************************************                     
* Function Name      : CheckRegistrationProcessStatus
* Description        : Check status of registration process status.
* Author Name		 : Neha Gharge	                                                                                           
* Date Of Creation   : 24th may 2014
* SR_No              : 
***********************************************************************************************************/
bool CWardWizHomePage::CheckRegistrationProcessStatus()
{
	bool bRet = false;
	//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
	//Neha Gharge 4 jan,2016
	if (theApp.m_bRegistrationInProcess)
	{
		if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_IN_PROCESS"),
			theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			//m_bRegistrationInProcess = true;
			return false;
		}
		//m_bRegistrationInProcess = false;
		
		bRet = theApp.CloseRegistrationWindow();
		if (bRet)
		{
			m_bIsPopUpDisplayed = false;
			theApp.m_bIsPopUpDisplayed = false;
			return bRet;
		}
		
	}
	return true;
}

/**********************************************************************************************************
* Function Name      : OnBnClickedButtonFixItNow
* Description        : it use to update the product
* Author Name		 : lalit kumawat
* Date Of Creation   :4-7-2015 
* SR_No              : 
***********************************************************************************************************/
void CWardWizHomePage::OnBnClickedButtonFixItNow()
{
	try
	{
		CString csButtonText = L"";
		m_btnFixItNow.GetWindowText(csButtonText);

		if ((csButtonText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FIX_NOW_REGISTER")) || (csButtonText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FIX_NOW_RENEW")))
		{
		
			OnBnClickedButtonRegisterButton();
			
		}
		else if(csButtonText == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"))
		{
			if (theApp.m_dwDaysLeft > 0)
			{
				CISpyGUIDlg * ObjIspyGUIHwnd = NULL;
				ObjIspyGUIHwnd = (CISpyGUIDlg*)AfxGetMainWnd();
				if (ObjIspyGUIHwnd != NULL)
				{
					ObjIspyGUIHwnd->OnLiveUpdateFixDNowCall();

				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnBnClickedButtonFixItNow", 0, 0, true, SECONDLEVEL);
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
bool CWardWizHomePage::isValidDate(int iDay, int iMonth, int iYear)
{
	try
	{
		if (iYear < 1970 || iYear > 3000 || iMonth < 1  || iMonth > 12 || iDay < 1)
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
		AddLogEntry(L"### Exception in CGenXHomePage::isValidDate", 0, 0, true, SECONDLEVEL);
		return false;
	}
	//If non of the above condition satisfy, then return true.
	return true;
}