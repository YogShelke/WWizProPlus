// SettingsDlg.cpp : implementation file
/****************************************************
*  Program Name: SettingsDlg.cpp                                                                                                    
*  Description: Parent Setting UI window.
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 27 May 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "SettingsDlg.h"
//#include "SettingsGeneral.h"


// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialog)

/***************************************************************************************************                    
*  Function Name  : CSettingsDlg                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
*  Modification   : Neha Gharge 15th OCt,2015 Folder locker setting changes
****************************************************************************************************/
CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CSettingsDlg::IDD, pParent)
	,m_stScan(NULL)
	,m_btnGeneralSettings(NULL)
	,m_btnEmailSettings(NULL)
	,m_btnScanSettings(NULL)
	,m_btnUpdateSettings(NULL)
	,m_pSettingsGeneralDlg(NULL)
	,m_pSettingsEmailDlg(NULL)
	,m_pSettingsScanDlg(NULL)
	,m_pSettingsUpdateDlg(NULL)
{

}

/***************************************************************************************************                    
*  Function Name  : ~CSettingsDlg                                                     
*  Description    : D'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
*  Modification   : Neha Gharge 15th OCt,2015 Folder locker setting changes
****************************************************************************************************/
CSettingsDlg::~CSettingsDlg()
{
	if(m_stScan != NULL)
	{
		delete m_stScan;
		m_stScan = NULL;
	}
	if(m_stScan != NULL)
	{
		delete m_stScan;
		m_stScan = NULL;
	}

	if(m_btnGeneralSettings != NULL)
	{
		delete m_btnGeneralSettings;
		m_btnGeneralSettings = NULL;
	}

	if(m_btnEmailSettings != NULL)
	{
		delete m_btnEmailSettings;
		m_btnEmailSettings = NULL;
	}

	if(m_btnScanSettings != NULL)
	{
		delete m_btnScanSettings;
		m_btnScanSettings = NULL;
	}

	if(m_btnUpdateSettings != NULL)
	{
		delete m_btnUpdateSettings;
		m_btnUpdateSettings = NULL;
	}

	if(m_pSettingsGeneralDlg != NULL)
	{
		delete m_pSettingsGeneralDlg;
		m_pSettingsGeneralDlg = NULL;
	}

	if(m_pSettingsEmailDlg != NULL)
	{
		delete m_pSettingsEmailDlg;
		m_pSettingsEmailDlg = NULL;
	}

	if(m_pSettingsScanDlg != NULL)
	{
		delete m_pSettingsScanDlg;
		m_pSettingsScanDlg = NULL;
	}

	if(m_pSettingsUpdateDlg != NULL)
	{
		delete m_pSettingsUpdateDlg;
		m_pSettingsUpdateDlg = NULL;
	}
	
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CLOSE_SETTINGDLG, m_Close_Button);
	DDX_Control(pDX, IDC_STATIC_SETTINGS_HEADER, m_stSettingsHeader);
	DDX_Control(pDX, IDC_Settings_LOGO, m_stSettingLogo);

	DDX_Control(pDX, IDC_DLGPOS_SETTINGS, m_dlgPos);
	DDX_Control(pDX, IDC_BUTTON_DEFAULT, m_btnSettingsDefault);
	DDX_Control(pDX, IDC_BUTTON_SETTINGS_OK, m_btnSettingsOk);
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CSettingsDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()

	// issue: resolved by lalit 10/17/2014, to add mouse over hand symbol on setting tab dialog menu 
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON1, &CSettingsDlg::OnBnClickedBackButton)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS_OK, &CSettingsDlg::OnBnClickedButtonSettingsOk)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT, &CSettingsDlg::OnBnClickedButtonDefault)
END_MESSAGE_MAP()


// CSettingsDlg message handlers
/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BOOL CSettingsDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);
//	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SETTINGBG), _T("JPG")))
	{
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
	}

// issue: resolved by lalit 10/17/2014, to add mouse over hand symbol on setting tab dialog menu 
	m_hButtonCursor = AfxGetApp()->LoadStandardCursor(IDC_HAND);

	Draw();

	CRect rect1;
	this->GetClientRect(rect1);
	CRgn		 rgn;
	rgn.CreateRoundRectRgn(rect1.left, rect1.top, rect1.right-2, rect1.bottom-2,0,0);
	this->SetWindowRgn(rgn, TRUE);
	//SetWindowPos(NULL, 1, 88, rect1.Width()-5, rect1.Height() - 5, SWP_NOREDRAW);

	InitTabDialog();

	//Varada Ikhar, Date:16-03-2015, Issue:In Settings->the title "WardWiz Settings" should be in center.
	//commented below 2 statement.
	//m_settingsHeaderFont.CreateFontW(22,8,0,0,0//FW_BOLD
	//	,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"Eras Demi ITC");
	
	//m_stSettingsHeader.SetFont(&m_settingsHeaderFont);
	m_stSettingsHeader.SetFont(&theApp.m_fontWWLogoHeader);
	m_stSettingsHeader.SetWindowPos(&wndTop,rect1.left + 45,0,450,50, SWP_NOREDRAW);

	m_settingsHeaderFont.DeleteObject();
	m_Close_Button.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE, IDB_BITMAP_CLOSE, IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_Close_Button.SetWindowPos(&wndTop, rect1.left + 608, 0, 26,17, SWP_NOREDRAW);


	m_bmpSettingsWardWizLogo = LoadBitmap(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_SETTINGS_LOGO));
	m_stSettingLogo.SetBitmap(m_bmpSettingsWardWizLogo);
	m_stSettingLogo.SetWindowPos(&wndTop,rect1.left +0,0,40,27, SWP_NOREDRAW);

	/*	ISSUE NO - 242 ::In settings tab: default & ok btn not aligned properly.
	 NAME - NITIN K. TIME - 22st May 2014*/
	m_btnSettingsDefault.SetSkin(theApp.m_hResDLL,IDB_BITMAP_USB_NORMAL,IDB_BITMAP_USB_NORMAL, IDB_BITMAP_USB_HOVER,IDB_BITMAP_USB_NORMAL,0,0,0,0,0);
	m_btnSettingsDefault.SetWindowPos(&wndTop, rect1.left + 505, 293, 57,21, SWP_NOREDRAW);
	m_btnSettingsDefault.SetTextColorA(BLACK,1,1);
	/*	ISSUE NO - 242 ::In settings tab: default & ok btn not aligned properly.
	 NAME - NITIN K. TIME - 22st May 2014*/
	m_btnSettingsOk.SetSkin(theApp.m_hResDLL,IDB_BITMAP_USB_NORMAL,IDB_BITMAP_USB_NORMAL, IDB_BITMAP_USB_HOVER,IDB_BITMAP_USB_NORMAL,0,0,0,0,0);
	m_btnSettingsOk.SetWindowPos(&wndTop, rect1.left + 572, 293, 57,21, SWP_NOREDRAW);
	m_btnSettingsOk.SetTextColorA(BLACK,1,1);

	m_ImageList.Create(16,16,ILC_COLOR24|ILC_MASK,4,4);
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ARROW));

	if(!RefreshString())
	{
		AddLogEntry(L">>> CSettingsDlg::OnInitDialog :: Failed RefreshString()", 0, 0, true, FIRSTLEVEL);
	}
	
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


/***************************************************************************
*  Function Name  : InitTabDialog
*  Description    : Initialized tab control
*  Author Name    : Nitin Kolpkar
*  SR_NO		  :
*  Date           : 24th April 2014
****************************************************************************/
BOOL CSettingsDlg::InitTabDialog()
{
	//create the TabDialog
	m_pTabDialog = new CTabDialog(IDD_TABDLG, this);
	if(m_pTabDialog->Create(IDD_TABDLG, this) == FALSE)
	{
		return FALSE;
	}
	

	if(!AddPagesToTabDialog())
	{
		return FALSE;
	}

	m_pTabDialog->SetDialogType(1);
	

	CRect objMainRect;
	GetWindowRect(&objMainRect);
	ScreenToClient(&objMainRect);

	//get TabDialog's position from the static control
	CRect oRect;
	m_dlgPos.GetWindowRect(&oRect);
	ScreenToClient(&oRect);
	m_dlgPos.MoveWindow(oRect.left, oRect.top, oRect.Width(), oRect.Height());
//	m_dlgPos.ShowWindow(SW_HIDE);

	m_pTabDialog->m_iDlgCx = objMainRect.Width()-2; //- oRect.left - 47  - 37/*- 1*/;
	m_pTabDialog->m_iDlgCy = objMainRect.Height()-5 ;// - BUTTON_HEIGHT + 10 + 37 + 40 + 6;

	//set the TabDialog's positon
//	m_pTabDialog->SetWindowPos(this, objMainRect.left - 2 , objMainRect.top + 97,objMainRect.Width() +2, objMainRect.Height(),SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	m_pTabDialog->SetWindowPos(this, objMainRect.left - 2 , objMainRect.top + 28,objMainRect.Width() +100, objMainRect.Height(),SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	m_pTabDialog->MoveWindow(objMainRect.left - 2 , objMainRect.top + 28, objMainRect.Width()+2, objMainRect.Height());
	m_pTabDialog->InitPagesShow();
	return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : OnCtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
HBRUSH CSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int	ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if(ctrlID == IDC_STATIC_SETTINGS			||
		ctrlID == IDC_STATIC_SETTINGS_HEADER)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}
/***************************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : To translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BOOL CSettingsDlg::PreTranslateMessage(MSG* pMsg)
{
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedBackButton                                                     
*  Description    : this function is called closing the Setting Tab Dialog
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 19 May 2014
****************************************************************************************************/
void CSettingsDlg::OnBnClickedBackButton()
{
	OnCancel();
}

/***************************************************************************************************                    
*  Function Name  : RefreshString                                                     
*  Description    : this function is called for refreshing string for various Language Support
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 19 May 2014
*  Modification   : Neha Gharge 15th OCt,2015 Folder locker setting changes
****************************************************************************************************/
BOOL CSettingsDlg :: RefreshString()
{
	//NOTE: In this Function we are also Creating the instances for the HTMLListCtrl in m_list.InsertItem() call

	m_stSettingsHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SETTINGS_HEADER"));
	m_stSettingsHeader.SetFont(&theApp.m_fontWWLogoHeader);

	/*	ISSUE NO - 449	In setting tab, the changes are getting applied even if we close without pressing ok button.
	NAME - NITIN K. TIME - 26st May 2014 */

	m_btnSettingsOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CLOSE"));
/*	CString m_settingTabOptions;
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_EMAIL_SCAN"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_USB_DETECT"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_SHOW_TOOLTIP"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_ENABLE_SOUND"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_SYSTEM_STARTUP_SCAN"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
*/	

	CString csStaticText , csButtonText;
	if(theApp.m_dwSelectedLangID == 0)
	{
		csStaticText = L"  ";
		csButtonText = L"         ";
	}
	if(theApp.m_dwSelectedLangID == 1)
	{
		csStaticText = L"  ";
		csButtonText = L"     ";
	}
	if(theApp.m_dwSelectedLangID == 2)
	{
		csStaticText = L"     ";
		csButtonText = L"         ";
	}
	if(theApp.m_dwSelectedLangID == 3)
	{
		csStaticText = L"    ";
		csButtonText = L"         ";
	}
	if(theApp.m_dwSelectedLangID == 4)
	{
		csStaticText = L"    ";
		csButtonText = L"         ";
	}
	if(theApp.m_dwSelectedLangID == 5)
	{
		csStaticText = L"    ";
		csButtonText = L"         ";
	}
	
	if(m_btnGeneralSettings != NULL)
	{
		m_btnGeneralSettings->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_GENERAL"));
	}
	if(m_btnScanSettings != NULL)
	{
		m_btnScanSettings->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
	}
	if(m_btnEmailSettings != NULL)
	{
			m_btnEmailSettings->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_EMAIL"));
	}
	if(m_btnUpdateSettings != NULL)
	{
		m_btnUpdateSettings->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_UPDATE"));
	}

//	m_btnVirusScan->SetTextAlignment(TA_NOUPDATECP);
	return true;
}


/***************************************************************************************************                    
*  Function Name  : AddPagesToTabDialog                                                     
*  Description    : this function is called for adding the pages into the Tab Dlg
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BOOL CSettingsDlg::AddPagesToTabDialog()
{
	m_dlgPos.ShowWindow(SW_HIDE);
		try
	{
		
		switch(theApp.m_dwProductID)
		{
			case ESSENTIAL:
				ShowEssentialSetting();
				break;
			case PRO:
				ShowProSetting();
				break;
			case ELITE:
				ShowEliteSetting();
				break;
			case BASIC:
				ShowBasicSetting();
				break;
		}
		return TRUE;
	}
	catch(...)
	{
		return false;
	}

}

/***************************************************************************************************                    
*  Function Name  : ShowEssentialSetting                                                     
*  Description    : this function is called for adding the pages into the Tab Dlg for Essential Version
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BOOL CSettingsDlg::ShowEssentialSetting()
{
		RECT oRcBtnRect;

		////create General Option Button
		m_btnGeneralSettings = new CxSkinButton();

		CPoint objBtnPoint = BTN_LOCATION;
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y +9 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnGeneralSettings->Create(L"General", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_GENERAL_OPTION);

		m_btnGeneralSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE,IDB_BITMAP_SETTING_OPTION_HOVER , 0, 0, 0, 0);
		m_btnGeneralSettings->SetTextAlignment(TA_LEFT);
		m_btnGeneralSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnGeneralSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnGeneralSettings->SetFont(&theApp.m_fontWWTextNormal);
		
		//create General Setting dialog
		m_pSettingsGeneralDlg = new CSettingsGeneralDlg(m_pTabDialog);
		if(m_pSettingsGeneralDlg->Create(IDD_DIALOG_SETTINGS_GENERAL, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add General Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsGeneralDlg, m_btnGeneralSettings);

//---------------------------------------------------------------------------------

		////create Scan Option Button
		m_btnScanSettings = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27  ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnScanSettings ->Create(L"Scan", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_SCAN_OPTION);

		m_btnScanSettings ->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_SETTING_OPTION_HOVER, 0, 0, 0, 0);
		m_btnScanSettings ->SetTextAlignment(TA_LEFT);
		m_btnScanSettings ->SetTextColorA(RGB(255,255,255),0,1);
		m_btnScanSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnScanSettings ->SetFont(&theApp.m_fontWWTextNormal);
		
		//create Scan Option dialog
		m_pSettingsScanDlg = new CSettingsScanDlg(m_pTabDialog);
		if(m_pSettingsScanDlg->Create(IDD_DIALOG_SETTINGS_SCAN, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Scan Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsScanDlg, m_btnScanSettings);

//-------------------------------------------------------------------------------------------

		////create Update Option Button
		m_btnUpdateSettings = new CxSkinButton();


		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27 + 27;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnUpdateSettings->Create(L"Update", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UPDATE_OPTION);

		m_btnUpdateSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE,IDB_BITMAP_SETTING_OPTION_HOVER, 0, 0, 0, 0);
		m_btnUpdateSettings->SetTextAlignment(TA_LEFT);
		m_btnUpdateSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnUpdateSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnUpdateSettings->SetFont(&theApp.m_fontWWTextNormal);


		//create Update Option dialog
		m_pSettingsUpdateDlg= new CSettingsUpdateDlg(m_pTabDialog);
		if(m_pSettingsUpdateDlg->Create(IDD_DIALOG_SETTINGS_UPDATE, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Update Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsUpdateDlg, m_btnUpdateSettings);


		return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : ShowProSetting                                                     
*  Description    : this function is called for adding the pages into the Tab Dlg for PRO Version
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
*  Modification   : Neha Gharge 15th OCt,2015 Folder locker setting changes
****************************************************************************************************/
BOOL CSettingsDlg::ShowProSetting()
{
		RECT oRcBtnRect;
		////create General Option Button
		m_btnGeneralSettings = new CxSkinButton();

		CPoint objBtnPoint = BTN_LOCATION;
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y +9 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnGeneralSettings->Create(L"General", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_GENERAL_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnGeneralSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE,0 , 0, 0, 0, 0);
		m_btnGeneralSettings->SetTextAlignment(TA_LEFT);
		m_btnGeneralSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnGeneralSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnGeneralSettings->SetFont(&theApp.m_fontWWTextNormal);
		
		//create General Setting dialog
		m_pSettingsGeneralDlg = new CSettingsGeneralDlg(m_pTabDialog);
		if(m_pSettingsGeneralDlg->Create(IDD_DIALOG_SETTINGS_GENERAL, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add General Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsGeneralDlg, m_btnGeneralSettings);

		//-----------------------------------------------------------------------------------------------------------------------------------

		////create Scan Option Button
		m_btnScanSettings = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27  ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnScanSettings ->Create(L"Scan", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_SCAN_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnScanSettings ->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnScanSettings ->SetTextAlignment(TA_LEFT);
		m_btnScanSettings ->SetTextColorA(RGB(255,255,255),0,1);
		m_btnScanSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnScanSettings ->SetFont(&theApp.m_fontWWTextNormal);
		
		//create Scan Option dialog
		m_pSettingsScanDlg = new CSettingsScanDlg(m_pTabDialog);
		if(m_pSettingsScanDlg->Create(IDD_DIALOG_SETTINGS_SCAN, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Scan Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsScanDlg, m_btnScanSettings);

//-------------------------------------------------------------------------------------------

		////create Email Option Button
		m_btnEmailSettings = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27 + 27 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnEmailSettings->Create(L"Email", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_EMAIL_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnEmailSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnEmailSettings->SetTextAlignment(TA_LEFT);
		m_btnEmailSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnEmailSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnEmailSettings->SetFont(&theApp.m_fontWWTextNormal);
		
		//create Email Option dialog
		m_pSettingsEmailDlg = new CSettingsEmailDlg(m_pTabDialog);	
		if(m_pSettingsEmailDlg->Create(IDD_DIALOG_SETTINGS_EMAIL, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Email Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsEmailDlg, m_btnEmailSettings);

//------------------------------------------------------------------------------------------------
		
		
		////create Update Option Button
		m_btnUpdateSettings = new CxSkinButton();


		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27 + 27 + 27 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnUpdateSettings->Create(L"Update", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UPDATE_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnUpdateSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnUpdateSettings->SetTextAlignment(TA_LEFT);
		m_btnUpdateSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnUpdateSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnUpdateSettings->SetFont(&theApp.m_fontWWTextNormal);
	
		
		//create Update Option dialog
		m_pSettingsUpdateDlg= new CSettingsUpdateDlg(m_pTabDialog);
		if(m_pSettingsUpdateDlg->Create(IDD_DIALOG_SETTINGS_UPDATE, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Update Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsUpdateDlg, m_btnUpdateSettings);
		return TRUE;

}

/***************************************************************************************************                    
*  Function Name  : ShowEliteSetting                                                     
*  Description    : this function is called for adding the pages into the Tab Dlg for ELITE Version
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 31 May 2014
****************************************************************************************************/
BOOL CSettingsDlg::ShowEliteSetting()
{
		RECT oRcBtnRect;
		////create General Option Button
		m_btnGeneralSettings = new CxSkinButton();

		CPoint objBtnPoint = BTN_LOCATION;
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y +9 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnGeneralSettings->Create(L"General", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_GENERAL_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnGeneralSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE,0 , 0, 0, 0, 0);
		m_btnGeneralSettings->SetTextAlignment(TA_LEFT);
		m_btnGeneralSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnGeneralSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnGeneralSettings->SetFont(&theApp.m_fontWWTextNormal);
		
		//create General Setting dialog
		m_pSettingsGeneralDlg = new CSettingsGeneralDlg(m_pTabDialog);
		if(m_pSettingsGeneralDlg->Create(IDD_DIALOG_SETTINGS_GENERAL, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add General Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsGeneralDlg, m_btnGeneralSettings);

//---------------------------------------------------------------------------------

		////create Scan Option Button
		m_btnScanSettings = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27  ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnScanSettings ->Create(L"Scan", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_SCAN_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnScanSettings ->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnScanSettings ->SetTextAlignment(TA_LEFT);
		m_btnScanSettings ->SetTextColorA(RGB(255,255,255),0,1);
		m_btnScanSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnScanSettings ->SetFont(&theApp.m_fontWWTextNormal);
		
		//create Scan Option dialog
		m_pSettingsScanDlg = new CSettingsScanDlg(m_pTabDialog);
		if(m_pSettingsScanDlg->Create(IDD_DIALOG_SETTINGS_SCAN, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Scan Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsScanDlg, m_btnScanSettings);

//-------------------------------------------------------------------------------------------

		////create Email Option Button
		m_btnEmailSettings = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27 + 27 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnEmailSettings->Create(L"Email", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_EMAIL_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnEmailSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnEmailSettings->SetTextAlignment(TA_LEFT);
		m_btnEmailSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnEmailSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnEmailSettings->SetFont(&theApp.m_fontWWTextNormal);
		
		//create Email Option dialog
		m_pSettingsEmailDlg = new CSettingsEmailDlg(m_pTabDialog);	
		if(m_pSettingsEmailDlg->Create(IDD_DIALOG_SETTINGS_EMAIL, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Email Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsEmailDlg, m_btnEmailSettings);

//------------------------------------------------------------------------------------------------

		////create Update Option Button
		m_btnUpdateSettings = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27 + 27 + 27 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnUpdateSettings->Create(L"Update", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UPDATE_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnUpdateSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnUpdateSettings->SetTextAlignment(TA_LEFT);
		m_btnUpdateSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnUpdateSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnUpdateSettings->SetFont(&theApp.m_fontWWTextNormal);
	
		
		//create Update Option dialog
		m_pSettingsUpdateDlg= new CSettingsUpdateDlg(m_pTabDialog);
		if(m_pSettingsUpdateDlg->Create(IDD_DIALOG_SETTINGS_UPDATE, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Update Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsUpdateDlg, m_btnUpdateSettings);

		return TRUE;
}


/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonSettingsOk                                                     
*  Description    : Close setting popup dialog
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsDlg::OnBnClickedButtonSettingsOk()
{
	OnCancel();
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonDefault                                                     
*  Description    : Set all default setting 
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
*  Modification   : Neha Gharge 15th OCt,2015 Folder locker setting changes
****************************************************************************************************/
void CSettingsDlg::OnBnClickedButtonDefault()
{
	m_pSettingsGeneralDlg->setDefaultSettings();
	m_pSettingsScanDlg->setDefaultSettings();
	m_pSettingsEmailDlg->setDefaultSettings();
	m_pSettingsUpdateDlg->setDefaultSettings();

	MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_MSG_OK"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONINFORMATION);
	UpdateData(TRUE);
}

/***************************************************************************************************                    
*  Function Name  : CloseSettingWindow                                                     
*  Description    : Close setting popup UI
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsDlg::CloseSettingWindow()
{
	OnCancel();
}

/***************************************************************************************************                    
*  Function Name  : ShowBasicSetting                                                     
*  Description    : this function is called for adding the pages into the Tab Dlg for Basic Version
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BOOL CSettingsDlg::ShowBasicSetting()
{
		RECT oRcBtnRect;

		////create General Option Button
		m_btnGeneralSettings = new CxSkinButton();

		CPoint objBtnPoint = BTN_LOCATION;
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y +9 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnGeneralSettings->Create(L"General", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_GENERAL_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnGeneralSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE,0 , 0, 0, 0, 0);
		m_btnGeneralSettings->SetTextAlignment(TA_LEFT);
		m_btnGeneralSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnGeneralSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnGeneralSettings->SetFont(&theApp.m_fontWWTextNormal);
		
		//create General Setting dialog
		m_pSettingsGeneralDlg = new CSettingsGeneralDlg(m_pTabDialog);
		if(m_pSettingsGeneralDlg->Create(IDD_DIALOG_SETTINGS_GENERAL, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add General Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsGeneralDlg, m_btnGeneralSettings);

//---------------------------------------------------------------------------------

		////create Scan Option Button
		m_btnScanSettings = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27  ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnScanSettings ->Create(L"Scan", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_SCAN_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnScanSettings ->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnScanSettings ->SetTextAlignment(TA_LEFT);
		m_btnScanSettings ->SetTextColorA(RGB(255,255,255),0,1);
		m_btnScanSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnScanSettings ->SetFont(&theApp.m_fontWWTextNormal);
		
		//create Scan Option dialog
		m_pSettingsScanDlg = new CSettingsScanDlg(m_pTabDialog);
		if(m_pSettingsScanDlg->Create(IDD_DIALOG_SETTINGS_SCAN, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Scan Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsScanDlg, m_btnScanSettings);

//-------------------------------------------------------------------------------------------

		////create Update Option Button
		m_btnUpdateSettings = new CxSkinButton();


		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 9 + 27 + 27;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnUpdateSettings->Create(L"Update", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UPDATE_OPTION);
//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnUpdateSettings->SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION, IDB_BITMAP_SETTING_OPTION_HOVER,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnUpdateSettings->SetTextAlignment(TA_LEFT);
		m_btnUpdateSettings->SetTextColorA(RGB(255,255,255),0,1);
		m_btnUpdateSettings->SetFocusTextColor(RGB(4,4,4));
		m_btnUpdateSettings->SetFont(&theApp.m_fontWWTextNormal);


		//create Update Option dialog
		m_pSettingsUpdateDlg= new CSettingsUpdateDlg(m_pTabDialog);
		if(m_pSettingsUpdateDlg->Create(IDD_DIALOG_SETTINGS_UPDATE, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Update Option in to lookup table
		m_pTabDialog->AddPage(m_pSettingsUpdateDlg, m_btnUpdateSettings);


		return TRUE;
}

// issue: resolved by lalit 10/17.2014, to add mouse over hand symbol on setting tab dialog menu 
/***************************************************************************
  Function Name  : OnSetCursor
  Description    : The framework calls this member function if mouse input 
				   is not captured and the mouse causes cursor movement within the CWnd object.
  Author Name    : Lalit
  SR_NO			 :
  Date           : 10/17/2014
  *  Modification   : Neha Gharge 15th OCt,2015 Folder locker setting changes
  ****************************************************************************/
BOOL CSettingsDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(!pWnd)
		return FALSE;
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();

	if (iCtrlID == ID_BUTTON_GENERAL_OPTION || iCtrlID == ID_BUTTON_SCAN_OPTION || iCtrlID == ID_BUTTON_UPDATE_OPTION || iCtrlID == ID_BUTTON_EMAIL_OPTION || iCtrlID == IDC_BUTTON_CLOSE_SETTINGDLG)
	{
		::SetCursor(m_hButtonCursor);
		return TRUE;
	}
	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}