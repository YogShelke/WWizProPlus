// ProductInformation.cpp : implementation file
//

#include "stdafx.h"
#include "ProductInformation.h"


// CProductInformation dialog

IMPLEMENT_DYNAMIC(CProductInformation, CDialog)

CProductInformation::CProductInformation(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CProductInformation::IDD, pParent)
	,m_pBoldFont(NULL)
{

}

CProductInformation::~CProductInformation()
{
	if(m_pBoldFont != NULL)
	{
		delete m_pBoldFont;
		m_pBoldFont = NULL;
	}
}

void CProductInformation::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PRODUCT_INFO, m_stProductInfo);
	DDX_Control(pDX, IDC_STATIC_REGISTERTO, m_RegisterTo);
	DDX_Control(pDX, IDC_STATIC_EMAILID, m_stEmailID);
	DDX_Control(pDX, IDC_STATIC_NOOFDAYSLEFT_PROD, m_stNoofDaysLeft);
	DDX_Control(pDX, IDC_STATIC_EDITION, m_stProductEdition);
	DDX_Control(pDX, IDC_STATIC_WHOLENAME, m_stWholeName);
	DDX_Text(pDX, IDC_STATIC_WHOLENAME, m_csWholeName);
	DDX_Control(pDX, IDC_STATIC_REGISTEREDEMAILID, m_stRegisteredEmailID);
	DDX_Text(pDX, IDC_STATIC_REGISTEREDEMAILID, m_csEmailID);
	DDX_Control(pDX, IDC_STATIC_NOOFDAYS, m_stNoofDays);
	DDX_Text(pDX, IDC_STATIC_NOOFDAYS, m_csNoofDays);
	DDX_Control(pDX, IDC_STATIC_EDITION_INSTALLED, m_stInstalledEdition);
	DDX_Text(pDX, IDC_STATIC_EDITION_INSTALLED, m_csInstalledEdition);
	DDX_Control(pDX, IDC_BUTTON_PROD_CLOSE, m_btnProdClose);
	DDX_Control(pDX, IDC_STATIC_VERSION, m_stVersion);
	DDX_Control(pDX, IDC_STATIC_VERSIONNO, m_stVersionNo);
	DDX_Text(pDX, IDC_STATIC_VERSIONNO, m_csVersionNo);
	DDX_Control(pDX, IDC_BUTTON_OK, m_btnOk);
	DDX_Control(pDX, IDC_STATIC_PRODUCT_KEY, m_stProductKey);
	DDX_Control(pDX, IDC_STATIC_PRODUCT_KEY_NO, m_stProductKeyNumber);
	DDX_Text(pDX, IDC_STATIC_PRODUCT_KEY_NO, m_csProductKeyNumber);
	DDX_Control(pDX, IDC_STATIC_DATA_ENC_TEXT, m_stDataEncText);
	DDX_Control(pDX, IDC_STATIC_DATAENC_VERSION, m_stDataEncVersion);
	DDX_Text(pDX, IDC_STATIC_DATAENC_VERSION, m_csDataEncVersion);
}


BEGIN_MESSAGE_MAP(CProductInformation, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_PROD_CLOSE, &CProductInformation::OnBnClickedButtonProdClose)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CProductInformation::OnBnClickedButtonOk)
END_MESSAGE_MAP()


// CProductInformation message handlers

BOOL CProductInformation::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//Issue Resolved : 0000109: Information Message box should have OK button
	//Resolved By: Nitin K 17th March 2015

	//Modified Date 10 July,2010 Neha Gharge 
	//Added a data encryption version no.
	if (!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_PASSWORD_DATA_FOLDER), _T("JPG")))
	{
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
	Draw();

	CRect rect;
	this->GetClientRect(rect);

	m_pBoldFont=NULL;
	m_pBoldFont = new(CFont);
	m_pBoldFont->CreateFont(15,6,0,0,FW_BOLD,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"verdana");
//	SetWindowPos(&wndTop, rect.top + 430,rect.top + 260, rect.Width()-3, rect.Height()-3, SWP_NOREDRAW);
	CRgn rgn;
	rgn.CreateRectRgn(rect.top, rect.left ,rect.Width()-3,rect.Height()-3);
	this->SetWindowRgn(rgn, TRUE);

	m_stProductInfo.SetWindowPos(&wndTop, rect.left + 23,52,250,20, SWP_NOREDRAW);
	m_stProductInfo.SetFont(m_pBoldFont);
	m_stProductInfo.SetBkColor(RGB(255,255,255));

	m_RegisterTo.SetWindowPos(&wndTop, rect.left + 23,77,100,15, SWP_NOREDRAW);
	m_RegisterTo.SetBkColor(RGB(255,255,255));

	m_stWholeName.SetWindowPos(&wndTop, rect.left + 129,77,200,15, SWP_NOREDRAW);
	m_stWholeName.SetBkColor(RGB(255,255,255));
	//m_stWholeName.SetWindowTextW(L"veikatesh ramkrushna iyyer subramann");

	m_stEmailID.SetWindowPos(&wndTop, rect.left + 23,100,100,15, SWP_NOREDRAW);
	m_stEmailID.SetBkColor(RGB(255,255,255));

	m_stRegisteredEmailID.SetWindowPos(&wndTop, rect.left + 129,100,200,15, SWP_NOREDRAW);
	m_stRegisteredEmailID.SetBkColor(RGB(255,255,255));
	//m_stRegisteredEmailID.SetWindowTextW(L"Veikateshiyer@gmail.com");

	m_stNoofDaysLeft.SetWindowPos(&wndTop, rect.left + 23,123,100,15, SWP_NOREDRAW);
	m_stNoofDaysLeft.SetBkColor(RGB(255,255,255));

	m_stNoofDays.SetWindowPos(&wndTop, rect.left + 129,123,200,15, SWP_NOREDRAW);
	m_stNoofDays.SetBkColor(RGB(255,255,255));
	//m_stNoofDays.SetWindowTextW(L"30");

	m_stProductEdition.SetWindowPos(&wndTop, rect.left + 23,146,100,15, SWP_NOREDRAW);
	m_stProductEdition.SetBkColor(RGB(255,255,255));

	m_stInstalledEdition.SetWindowPos(&wndTop, rect.left + 129,146,200,15, SWP_NOREDRAW);
	m_stInstalledEdition.SetBkColor(RGB(255,255,255));
	//m_stInstalledEdition.SetWindowTextW(L"Silver");

	m_stVersion.SetWindowPos(&wndTop, rect.left + 23,169,100,15, SWP_NOREDRAW);
	m_stVersion.SetBkColor(RGB(255,255,255));

	m_stVersionNo.SetWindowPos(&wndTop, rect.left + 129,169,190,15, SWP_NOREDRAW);
	m_stVersionNo.SetBkColor(RGB(255,255,255));
	//m_stVersionNo.SetWindowTextW(L"1.0.0.3");

	//Issue Resolved : 0000109: Information Message box should have OK button
	//Resolved By: Nitin K 17th March 2015

	m_stProductKey.SetWindowPos(&wndTop, rect.left + 23, 192, 100, 15, SWP_NOREDRAW);
	m_stProductKey.SetBkColor(RGB(255, 255, 255));

	m_stProductKeyNumber.SetWindowPos(&wndTop, rect.left + 129, 192, 200, 15, SWP_NOREDRAW);
	m_stProductKeyNumber.SetBkColor(RGB(255, 255, 255));

	m_stDataEncText.SetWindowPos(&wndTop, rect.left + 23, 212, 116, 15, SWP_NOREDRAW);
	m_stDataEncText.SetBkColor(RGB(255, 255, 255));

	m_stDataEncVersion.SetWindowPos(&wndTop, rect.left + 139, 212, 200, 15, SWP_NOREDRAW);
	m_stDataEncVersion.SetBkColor(RGB(255, 255, 255));

	m_btnOk.SetWindowPos(&wndTop, rect.left + 150, 238, 57, 21, SWP_NOREDRAW);
	m_btnOk.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_DISABLE_WHITE_BG, 0, 0, 0, 0, 0);
	m_btnOk.SetTextColorA(0, 1, 1);

	m_btnProdClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnProdClose.SetWindowPos(&wndTop,rect.left + 345,0,26,17,SWP_NOREDRAW);
	
	if(!RefreshString())
	{
		AddLogEntry(L">>> CProductInformation::OnInitDialog :: Failed RefreshString()", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CProductInformation::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	//Invoke setup->click on Product Information icon->click esc it is not getting closed (it is not mapped to esc key)
	//Niranjan Deshak- 28/02/2014
	/*if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}*/
	return CJpegDialog::PreTranslateMessage(pMsg);
}

void CProductInformation::OnBnClickedButtonProdClose()
{
	// TODO: Add your control notification handler code here
	// resolve by lalit kumawat 3-17-015
	//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
	//(same issue applies to change password & scan computer at start up settings
	OnCancel();
}


/***********************************************************************************************
  Function Name  : RefreshString
  Description    : this function is  called for setting the Text UI with different Language Support
  Author Name    : Nitin Kolapkar
  Date           : 5th May 2014
***********************************************************************************************/
BOOL CProductInformation :: RefreshString()
{
	/*****************ISSUE NO -102,129 Neha Gharge 22/5/14 ************************************************/
	m_stProductInfo.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODINFO_PRODINFO_TEXT"));

	m_RegisterTo.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODINFO_REGTO_TEXT"));

	m_stEmailID.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODINFO_EMAILID_TEXT"));

	m_stNoofDaysLeft.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODINFO_NOOFDAYS_TEXT"));

	m_stProductEdition.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODINFO_PRODEDITION_TEXT"));

	m_stVersion.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODINFO_PRODVERSION_TEXT"));

	//Issue Resolved : 0000109: Information Message box should have OK button
	//Resolved By: Nitin K 17th March 2015
	m_stProductKey.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODINFO_PRODKEY_TEXT"));

	//Modified Date 10 July,2010 Neha Gharge 
	//Added a data encryption version no.
	m_stDataEncText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_DATAENC_TEXT"));
	//issue:- in Basic setup it showing DataEncryption version information even we don't have DataEncryption features in Basic Edition.
	//Resolved by lalit kumawat 8-24-2015
	if (theApp.m_dwProductID == BASIC)
	{
		m_stDataEncVersion.ShowWindow(false);
		m_stDataEncText.ShowWindow(false);
	}
	
	return true;	
}


//Issue Resolved : 0000109: Information Message box should have OK button
//Resolved By: Nitin K 17th March 2015
void CProductInformation::OnBnClickedButtonOk()
{
	OnCancel();
	// TODO: Add your control notification handler code here
}
