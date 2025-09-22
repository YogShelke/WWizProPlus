// RegistrationMsgPopupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWizRegistration.h"
#include "RegistrationMsgPopupDlg.h"
#include "ISpyCommunicator.h"



// CRegistrationMsgPopupDlg dialog

IMPLEMENT_DYNAMIC(CRegistrationMsgPopupDlg, CDialog)

CRegistrationMsgPopupDlg::CRegistrationMsgPopupDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CRegistrationMsgPopupDlg::IDD, pParent)
	,m_pTextFont(NULL)
	,m_pBoldFont(NULL)
	,m_bShowAtStartUp(false)

{

}

CRegistrationMsgPopupDlg::~CRegistrationMsgPopupDlg()
{
	if(m_pTextFont != NULL)
	{
		delete m_pTextFont;
		m_pTextFont = NULL;
	}
	if(m_pBoldFont != NULL)
	{
		delete m_pBoldFont;
		m_pBoldFont = NULL;
	}
}

void CRegistrationMsgPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_REGISTERNOW_REGISTARTION, m_btnRegister);
	DDX_Control(pDX, IDC_STATIC_EXPIRED_TEXT, m_stExpiredText);
	DDX_Control(pDX, IDC_BUTTON_REGMSG_MINIMIZE, m_btnRegMinimize);
	DDX_Control(pDX, IDC_BUTTON_REGMSG_CLOSE, m_btnRegClose);
	DDX_Control(pDX, IDC_STATIC_MSGICON_PIC, m_stRegMsgIconPic);
	DDX_Control(pDX, IDC_STATIC_DESCRIPTION, m_stRegMsgDescription);
	DDX_Control(pDX, IDC_BUTTON_REGMSG_BUYNOW, m_btnRegmsgBuyNow);
	DDX_Control(pDX, IDC_CHECK_DO_NOT_SHOW_AGAIN, m_chkDoNotShowAgain);
	DDX_Control(pDX, IDC_STATIC_DO_NOT_SHOW_AGAIN, m_stDoNotShowAgain);
	DDX_Control(pDX, IDC_BUTTON_CONTINUE, m_btnContinue);
}


BEGIN_MESSAGE_MAP(CRegistrationMsgPopupDlg, CJpegDialog)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_REGMSG_MINIMIZE, &CRegistrationMsgPopupDlg::OnBnClickedButtonRegmsgMinimize)
	ON_BN_CLICKED(IDC_BUTTON_REGISTERNOW_REGISTARTION, &CRegistrationMsgPopupDlg::OnBnClickedButtonRegisternowRegistartion)
	ON_BN_CLICKED(IDC_BUTTON_REGMSG_CLOSE, &CRegistrationMsgPopupDlg::OnBnClickedButtonRegmsgClose)
	ON_BN_CLICKED(IDC_BUTTON_REGMSG_BUYNOW, &CRegistrationMsgPopupDlg::OnBnClickedButtonRegmsgBuynow)
	ON_BN_CLICKED(IDC_CHECK_DO_NOT_SHOW_AGAIN, &CRegistrationMsgPopupDlg::OnBnClickedCheckDoNotShowAgain)
	ON_BN_CLICKED(IDC_BUTTON_CONTINUE, &CRegistrationMsgPopupDlg::OnBnClickedButtonContinue)
END_MESSAGE_MAP()


// CRegistrationMsgPopupDlg message handlers

BOOL CRegistrationMsgPopupDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_PASSWORD_BG), _T("JPG")))
	{
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
	Draw();

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL, MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect;
	this->GetClientRect(rect);

	m_pTextFont=NULL;
	m_pTextFont = new(CFont);
	m_pTextFont->CreatePointFont(90, _T("Verdana"));


	m_pBoldFont=NULL;
	m_pBoldFont = new(CFont);
	m_pBoldFont->CreateFont(15,6,0,0,FW_BOLD,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"verdana");

	//SetWindowPos(&wndTop, rect.top + 430,rect.top + 260, rect.Width()-3, rect.Height()-3, SWP_NOREDRAW);
	CRgn rgn;
	rgn.CreateRectRgn(rect.top, rect.left ,rect.Width()-3,rect.Height()-3);
	this->SetWindowRgn(rgn, TRUE);
	m_bmpRegMsgIcon = LoadBitmap(theApp.m_hResDLL,  MAKEINTRESOURCE(IDB_BITMAP_MSGICON));
	m_stRegMsgIconPic.SetBitmap(m_bmpRegMsgIcon);

	
	m_btnRegMinimize.SetSkin(theApp.m_hResDLL,IDB_BITMAP_MINMIZE,IDB_BITMAP_MINMIZE,IDB_BITMAP_MINMIZEMOVER,IDB_BITMAP_MINMIZE,0,0,0,0,0);
	m_btnRegMinimize.SetWindowPos(&wndTop,rect.left + 345,0,26,17,SWP_NOREDRAW);
	m_btnRegMinimize.ShowWindow(SW_HIDE);

	m_btnRegClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnRegClose.SetWindowPos(&wndTop,rect.left + 345,0,26,17,SWP_NOREDRAW);

	m_stRegMsgIconPic.SetWindowPos(&wndTop, rect.left + 80,52,42,39, SWP_NOREDRAW);

	/*	ISSUE NO - 493 NAME - NITIN K. TIME - 29th May 2014 */
	m_stExpiredText.SetWindowPos(&wndTop, rect.left + 128,65,210,20, SWP_NOREDRAW);
	m_stExpiredText.SetBkColor(RGB(255,255,255));
	m_stExpiredText.SetTextColor(RGB(238,0,0));
	m_stExpiredText.SetFont(m_pBoldFont);

	m_stRegMsgDescription.SetWindowPos(&wndTop, rect.left + 30,100,340,40, SWP_NOREDRAW);
	m_stRegMsgDescription.SetBkColor(RGB(255,255,255));
	m_stRegMsgDescription.SetFont(m_pTextFont);

	m_btnRegmsgBuyNow.SetSkin(theApp.m_hResDLL, IDB_BITMAP_103x21_BUTTON, IDB_BITMAP_103x21_BUTTON, IDB_BITMAP3_103x21HBUTTON,IDB_BITMAP_103x21_BUTTON,0,0,0,0,0);
	m_btnRegmsgBuyNow.SetWindowPos(&wndTop, rect.left + 72,145,103,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnRegmsgBuyNow.SetTextColor(RGB(255,255,255));

	m_btnRegister.SetSkin(theApp.m_hResDLL, IDB_BITMAP_103x21_BUTTON, IDB_BITMAP_103x21_BUTTON, IDB_BITMAP3_103x21HBUTTON,IDB_BITMAP_103x21_BUTTON,0,0,0,0,0);
	m_btnRegister.SetWindowPos(&wndTop, rect.left + 210, 145, 103,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnRegister.SetTextColor(RGB(255,255,255));


	m_chkDoNotShowAgain.SetWindowPos(&wndTop, rect.left + 30, 178, 13, 13, SWP_NOREDRAW | SWP_NOZORDER);
	m_chkDoNotShowAgain.SetCheck(true);

	m_stDoNotShowAgain.SetWindowPos(&wndTop, rect.left + 48, 177, 200, 24, SWP_NOREDRAW);
	m_stDoNotShowAgain.SetBkColor(RGB(255, 255, 255));
	m_stDoNotShowAgain.SetFont(m_pTextFont);

	
	//m_stContinue.SetResourceDllHandle(theApp.m_hResDLL);

	m_btnContinue.SetWindowPos(&wndTop, rect.left + 270, 175, 70, 21, SWP_NOREDRAW);
	m_btnContinue.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BUTTON_CONTINUE, IDB_BITMAP_BUTTON_CONTINUE, IDB_BITMAP_BUTTON_CONTINUE, IDB_BITMAP_BUTTON_CONTINUE, 0, 0, 0, 0, 0);
	m_btnContinue.SetTextColorA(BLUE, 1, BLUE);
	m_btnContinue.SetFont(m_pTextFont);
	
	m_chkDoNotShowAgain.ShowWindow(SW_HIDE);
	m_stDoNotShowAgain.ShowWindow(SW_HIDE);
	if (!m_bShowAtStartUp)
	{
		m_chkDoNotShowAgain.ShowWindow(SW_HIDE);
		m_stDoNotShowAgain.ShowWindow(SW_HIDE);
		m_btnContinue.ShowWindow(SW_HIDE);
	}
	if(!RefreshString())
	{
		AddLogEntry(L">>> CRegistrationDlg::OnInitDialog :: Failed RefreshString()", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRegistrationMsgPopupDlg::OnBnClickedButtonRegmsgMinimize()
{
	// TODO: Add your control notification handler code here
	this->ShowWindow(SW_MINIMIZE);
}

void CRegistrationMsgPopupDlg::OnBnClickedButtonRegisternowRegistartion()
{
	//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
	//Neha Gharge 4 jan,2016
	OnOK();
	//theApp.PerformRegistration();
}

void CRegistrationMsgPopupDlg::OnBnClickedButtonRegmsgClose()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CRegistrationMsgPopupDlg::OnBnClickedButtonRegmsgBuynow()
{
	//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
	//Neha Gharge 4 jan,2016
	OnCancel();
	ShellExecute(NULL,L"open",L"http://www.vibranium.co.in/",NULL,NULL,SW_SHOW);
}

BOOL CRegistrationMsgPopupDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();

	if( 
		iCtrlID == IDC_BUTTON_REGMSG_MINIMIZE		  ||
		iCtrlID == IDC_BUTTON_REGMSG_CLOSE			  ||
		iCtrlID == IDC_BUTTON_REGMSG_BUYNOW			  ||
		iCtrlID == IDC_BUTTON_REGISTERNOW_REGISTARTION||
		iCtrlID == IDC_BUTTON_CONTINUE
		
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
BOOL CRegistrationMsgPopupDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
	
}




/***********************************************************************************************
  Function Name  : RefreshString
  Description    : this function is  called for setting the Text UI with different Language Support
  Author Name    : Nitin Kolapkar
  Date           : 5th May 2014
***********************************************************************************************/
BOOL CRegistrationMsgPopupDlg :: RefreshString()
{
	try
	{
		/*	ISSUE NO - Product expired MSG box not coming proper in case of Unregistered product nd Expired product NAME - NITIN K. TIME - 3rd July 2014 */
		if (theApp.m_bRegEmail)
		{
			m_stExpiredText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REGMSGPOPUP_EXPIREDMSG_TEXT"));
			m_stRegMsgDescription.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REGMSGPOPUP_EXPIRED_DESC_TEXT"));
		}
		else
		{
			m_stExpiredText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REGMSGPOPUP_UNREGISTEREDMSG_TEXT"));
			m_stRegMsgDescription.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REGMSGPOPUP_UNREGISTERED_DESC_TEXT"));
		}


		m_btnRegmsgBuyNow.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REGMSGPOPUP_BUTTON_BUY_NOW"));

		m_btnRegister.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REGMSGPOPUP_BUTTON_REGISTER_NOW"));

		m_stDoNotShowAgain.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_DO_NOT_SHOW_AGAIN"));// L"Nicht mehr anzeigen"

		m_btnContinue.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CONTINUE"));//(L"Fortsetzen");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationMsgPopupDlg::RefreshString", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : OnBnClickedCheckDoNotShowAgain
Description    : 
Author Name    : Nitin Kolapkar
Date           : 5th May 2014
***********************************************************************************************/
void CRegistrationMsgPopupDlg::OnBnClickedCheckDoNotShowAgain()
{
	//DWORD dwShowProdExpMsg = 0;
	//if (m_chkDoNotShowAgain.GetCheck() == BST_UNCHECKED)
	//{
	//	//theApp.m_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium", L"dwShowProdExpMsgBox", dwShowProdExpMsg);
	//	if (!SetRegistrykeyUsingService(L"SOFTWARE\\Vibranium", L"dwShowProdExpMsgBox", REG_DWORD, dwShowProdExpMsg, true))
	//	{
	//		AddLogEntry(L"### Error in Setting Registry CRegistrationMsgPopupDlg::OnBnClickedCheckDoNotShowAgain()", 0, 0, true, SECONDLEVEL);
	//	}
	//}
	//else
	//{
	//	dwShowProdExpMsg = 1;
	//	//theApp.m_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium", L"dwShowProdExpMsgBox", dwShowProdExpMsg);
	//	if (!SetRegistrykeyUsingService(L"SOFTWARE\\Vibranium", L"dwShowProdExpMsgBox", REG_DWORD, dwShowProdExpMsg, true))
	//	{
	//		AddLogEntry(L"### Error in Setting Registry CRegistrationMsgPopupDlg::OnBnClickedCheckDoNotShowAgain()", 0, 0, true, SECONDLEVEL);
	//	}
	//}
}


/***************************************************************************************************
*  Function Name  : OnBnClickedButtonContinue
*  Description    : called on click of continue button
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 26th Feb 2016
****************************************************************************************************/
void CRegistrationMsgPopupDlg::OnBnClickedButtonContinue()
{
	OnBnClickedButtonRegmsgClose();
}



/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : this function is  called for setting the Registry Keys using the Services
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CRegistrationMsgPopupDlg::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;
		//szPipeData.hHey = HKEY_LOCAL_MACHINE;

		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;

		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CRegistrationMsgPopupDlg::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CRegistrationMsgPopupDlg::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationMsgPopupDlg::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}