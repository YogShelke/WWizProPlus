// SupportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "SupportDlg.h"
#include "afxdialogex.h"


// CSupportDlg dialog

IMPLEMENT_DYNAMIC(CSupportDlg, CDialog)

CSupportDlg::CSupportDlg(CWnd* pParent /*=NULL*/)
: CJpegDialog(CSupportDlg::IDD, pParent)
{

}

CSupportDlg::~CSupportDlg()
{
}

void CSupportDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CHAT_SUPPORT, m_btnChatSupport);
	DDX_Control(pDX, IDC_BUTTON_EMAIL_SUPPORT, m_btnEmailSupport);
	DDX_Control(pDX, IDC_BUTTON_SUBMIT_TICKET, m_btnSubmitTicket);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_STATIC_HEADER_DESCRIPTION, m_stHeaderDesc);
	DDX_Control(pDX, IDC_STATIC_EMAIL_SUPPORT_DESC, m_stEmailSupportDesc);
	DDX_Control(pDX, IDC_STATIC_CHAT_DESC, m_stChatSupportDesc);
	DDX_Control(pDX, IDC_STATIC_SUBMIT_TICKET_DESC, m_stSubmitTicketDesc);
	DDX_Control(pDX, IDC_STATIC_FOOTER, m_stFooterText);
	DDX_Control(pDX, IDC_STATIC_EMAIL_SUPPORT_HEADER, m_stEmailHeader);
	DDX_Control(pDX, IDC_STATIC_CHAT_SUPPORT_HEADER, m_stChatHeader);
	DDX_Control(pDX, IDC_STATIC_SUBMIT_TICKET_HEADER, m_stSubmitTicketHeader);
	DDX_Control(pDX, IDC_STATIC_SUPPORT_CONTACT_NO, m_stSupportContactNo);
	DDX_Control(pDX, IDC_STATIC_SUPPORT_NO, m_stSupportNo);
}


BEGIN_MESSAGE_MAP(CSupportDlg, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CSupportDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CHAT_SUPPORT, &CSupportDlg::OnBnClickedButtonChatSupport)
	ON_BN_CLICKED(IDC_BUTTON_EMAIL_SUPPORT, &CSupportDlg::OnBnClickedButtonEmailSupport)
	ON_BN_CLICKED(IDC_BUTTON_SUBMIT_TICKET, &CSupportDlg::OnBnClickedButtonSubmitTicket)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CSupportDlg message handlers


BOOL CSupportDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);

	CString csNoofdays;
	if (!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SUPPORT_BG), _T("JPG")))
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ANTIROOTKIT_FAILMSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}

	// TODO:  Add extra initialization here
	Draw();

	this->GetClientRect(m_rect);
	CRgn		 rgn;
	rgn.CreateRectRgn(m_rect.left, m_rect.top, m_rect.right - 3, m_rect.bottom - 3/*, 41, 41*/);
	this->SetWindowRgn(rgn, TRUE);

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL, MAKEINTRESOURCE(IDC_CURSOR_HAND));

	m_btnClose.SetSkin(theApp.m_hResDLL, IDB_BITMAP_CLOSE, IDB_BITMAP_CLOSE, IDB_BITMAP_CLOSEOVER, IDB_BITMAP_CLOSE, 0, 0, 0, 0, 0);
	m_btnClose.SetWindowPos(&wndTop, m_rect.left + 570, 0, 26, 17, SWP_NOREDRAW);

	//m_stHeaderDesc
	m_stHeaderDesc.SetTextColor(RGB(24, 24, 24));
	m_stHeaderDesc.SetBkColor(RGB(238, 238, 238));
	m_stHeaderDesc.SetFont(&theApp.m_fontWWTextTitle);
	m_stHeaderDesc.SetWindowPos(&wndTop, m_rect.left + 40, 90, 350, 35, SWP_NOREDRAW);
	m_stHeaderDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_HEADER"));

	//Issue: 0000686: change mobile number on support dialog.
	//Resolved by : Nitin K Date: 9th July 2015
	m_stSupportNo.SetWindowPos(&wndTop, m_rect.left + 440, 95, 150, 25, SWP_SHOWWINDOW);
	//m_stSupportNo.SetTextColor(RGB(24, 24, 24));
	m_stSupportNo.SetBkColor(RGB(238, 238, 238));
	m_stSupportNo.SetFont(&theApp.m_fontWWTextNormal);
	GetSupportNo();

	//Contact No added on support dialog
	//added by Nitin K. Date 13th May 2015
	m_bmpSupportContactPic = LoadBitmap(theApp.m_hResDLL, MAKEINTRESOURCE(IDB_BITMAP_SUPPORT_CONTACT_NO));
	m_stSupportContactNo.SetBitmap(m_bmpSupportContactPic);
	m_stSupportContactNo.SetWindowPos(&wndTop, m_rect.left + 400, 83, 183, 42, SWP_NOREDRAW);
	

	m_btnEmailSupport.SetSkin(theApp.m_hResDLL, IDB_BITMAP_SUPPORT_EMAIL, IDB_BITMAP_SUPPORT_EMAIL, IDB_BITMAP_SUPPORT_EMAIL_HOVER, IDB_BITMAP_SUPPORT_EMAIL, 0, 0, 0, 0, 0);
	m_btnEmailSupport.SetWindowPos(&wndTop, m_rect.left + 00, 125, 148, 64, SWP_NOREDRAW);

	m_btnChatSupport.SetSkin(theApp.m_hResDLL, IDB_BITMAP_SUPPORT_LIVE_CHAT, IDB_BITMAP_SUPPORT_LIVE_CHAT, IDB_BITMAP_SUPPORT_LIVE_CHAT_HOVER, IDB_BITMAP_SUPPORT_LIVE_CHAT, 0, 0, 0, 0, 0);
	m_btnChatSupport.SetWindowPos(&wndTop, m_rect.left + 00, 215, 148, 67, SWP_NOREDRAW);

	m_btnSubmitTicket.SetSkin(theApp.m_hResDLL, IDB_BITMAP_SUBMIT_TICKET, IDB_BITMAP_SUBMIT_TICKET, IDB_BITMAP_SUBMIT_TICKET_HOVER, IDB_BITMAP_SUBMIT_TICKET, 0, 0, 0, 0, 0);
	m_btnSubmitTicket.SetWindowPos(&wndTop, m_rect.left + 00, 310, 148, 67, SWP_NOREDRAW);


	m_stEmailHeader.SetTextColor(RGB(24, 24, 24));
	m_stEmailHeader.SetBkColor(RGB(238, 238, 238));
	m_stEmailHeader.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stEmailHeader.SetWindowPos(&wndTop, m_rect.left + 170, 125, 300, 25, SWP_NOREDRAW);
	m_stEmailHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_EMAIL_SUPPORT"));

	m_stEmailSupportDesc.SetTextColor(RGB(24, 24, 24));
	m_stEmailSupportDesc.SetBkColor(RGB(238, 238, 238));
	//m_stEmailSupportDesc.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stEmailSupportDesc.SetWindowPos(&wndTop, m_rect.left + 170, 155, 400, 40, SWP_NOREDRAW);
	m_stEmailSupportDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_EMAIL_SUPPORT_DESC"));

	m_stChatHeader.SetTextColor(RGB(24, 24, 24));
	m_stChatHeader.SetBkColor(RGB(238, 238, 238));
	m_stChatHeader.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stChatHeader.SetWindowPos(&wndTop, m_rect.left + 170, 215, 300, 25, SWP_NOREDRAW);
	m_stChatHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_CHAT_SUPPORT"));

	m_stChatSupportDesc.SetTextColor(RGB(24, 24, 24));
	m_stChatSupportDesc.SetBkColor(RGB(238, 238, 238));
	//m_stChatSupportDesc.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stChatSupportDesc.SetWindowPos(&wndTop, m_rect.left + 170, 245, 400, 40, SWP_NOREDRAW);
	m_stChatSupportDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_CHAT_SUPPORT_DESC"));

	m_stSubmitTicketHeader.SetTextColor(RGB(24, 24, 24));
	m_stSubmitTicketHeader.SetBkColor(RGB(238, 238, 238));
	m_stSubmitTicketHeader.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stSubmitTicketHeader.SetWindowPos(&wndTop, m_rect.left + 170, 315, 300, 25, SWP_NOREDRAW);
	m_stSubmitTicketHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_SUBMIT_TICKET_SUPPORT"));

	m_stSubmitTicketDesc.SetTextColor(RGB(24, 24, 24));
	m_stSubmitTicketDesc.SetBkColor(RGB(238, 238, 238));
	//m_stSubmitTicketDesc.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stSubmitTicketDesc.SetWindowPos(&wndTop, m_rect.left + 170, 345, 400, 40, SWP_NOREDRAW);
	m_stSubmitTicketDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUPPORT_SUBMIT_TICKET_SUPPORT_DESC"));


	LOGFONT lfInstallerTitle;
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 15;
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 6;
	m_BoldText.CreateFontIndirect(&lfInstallerTitle);
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));	 //	   with	face name "Verdana".

	m_stFooterText.SetWindowPos(&wndTop, m_rect.left + 20, 387, 320, 24, SWP_NOREDRAW);
	m_stFooterText.SetTextColor(RGB(238, 238, 238));
	m_stFooterText.SetBkColor(RGB(88, 88, 90));
	m_stFooterText.SetFont(&m_BoldText);
	CString csFooter;
	csFooter = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOOTER_MSG");
	csFooter += _T(" \u00AE ");
	csFooter += theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOOTER_MSG_YEAR");
	m_stFooterText.SetWindowTextW(csFooter);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************
Function Name  : OnBnClickedButtonClose
Description    : Closes the dialog
Author Name    : Nitin K
SR_NO		   :
Date           : 10th May 2015
****************************************************************************/
void CSupportDlg::OnBnClickedButtonClose()
{
	try
	{
		OnCancel();
	}

	catch (...)
	{
		AddLogEntry(L"### Failed to close Support Window", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetDefaultHTTPBrowser
Description    : Function Hits the Chat support URL with Default browser
Date           : 10th May 2015
****************************************************************************/
void CSupportDlg::OnBnClickedButtonChatSupport()
{
	try
	{
		TCHAR	szPath[512] = { 0 };

		if (GetDefaultHTTPBrowser(szPath))
		{
			GetEnvironmentVariable(L"ProgramFiles", szPath, 511);
			wcscat_s(szPath, 511, L"\\Internet Explorer\\iexplore.exe");
		}

		ShellExecute(NULL, L"open", szPath, L"http://www.wardwiz.in/Wardwiz-chat-support", NULL, SW_SHOW);
	}

	catch (...)
	{
		AddLogEntry(L"### Failed to Hit chat Support Window", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : OnBnClickedButtonEmailSupport
Description    : Function Hits the Email support URL with Default browser
Author Name    : Nitin K
SR_NO		   :
Date           : 10th May 2015
****************************************************************************/
void CSupportDlg::OnBnClickedButtonEmailSupport()
{
	try
	{
		TCHAR	szPath[512] = { 0 };

		if (GetDefaultHTTPBrowser(szPath))
		{
			GetEnvironmentVariable(L"ProgramFiles", szPath, 511);
			wcscat_s(szPath, 511, L"\\Internet Explorer\\iexplore.exe");
		}

		ShellExecute(NULL, L"open", szPath, L"http://www.wardwiz.in/email-support", NULL, SW_SHOW);
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to Hit email Support Window", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : OnBnClickedButtonSubmitTicket
Description    : Function Hits the Submit ticket URL with Default browser
Author Name    : Nitin K
SR_NO		   :
Date           : 10th May 2015
****************************************************************************/
void CSupportDlg::OnBnClickedButtonSubmitTicket()
{
	try
	{
		TCHAR	szPath[512] = { 0 };

		if (GetDefaultHTTPBrowser(szPath))
		{
			GetEnvironmentVariable(L"ProgramFiles", szPath, 511);
			wcscat_s(szPath, 511, L"\\Internet Explorer\\iexplore.exe");
		}

		ShellExecute(NULL, L"open", szPath, L"http://www.wardwiz.com/support/", NULL, SW_SHOW);
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to Hit submit ticket Support Window", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetDefaultHTTPBrowser
Description    : Function which gets default browser present on client machine
Author Name    : 
SR_NO		   :
Date           : 10th May 2015
****************************************************************************/
bool CSupportDlg::GetDefaultHTTPBrowser(LPTSTR lpszBrowserPath)
{
	try
	{

		CITinRegWrapper	objReg;

		TCHAR	szPath[512] = { 0 };
		DWORD	dwSize = 511;

		objReg.GetRegistryValueData(HKEY_CURRENT_USER, L"Software\\Classes\\http\\shell\\open\\command", L"", szPath, dwSize);
		if (!szPath[0])
			return true;

		_wcsupr_s(szPath);

		TCHAR	*pTemp = StrStr(szPath, L".EXE");

		if (!pTemp)
			return true;

		pTemp += wcslen(L".EXE") + 0x01;

		*pTemp = '\0';

		if (szPath[wcslen(szPath) - 1] == '"')
			szPath[wcslen(szPath) - 1] = '\0';

		wcscpy_s(lpszBrowserPath, 511, &szPath[1]);

		if (!PathFileExists(lpszBrowserPath))
			return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to Domodal Support Window", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : disable default operations on Escape/Enter button 
Author Name    : Nitin K.
SR_NO		   :
Date           : 10th May 2015
****************************************************************************/
BOOL CSupportDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}


BOOL CSupportDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!pWnd)
		return FALSE;

	// hand cursor on information button neha gharge 22/5/2014.
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if (
		iCtrlID == IDC_BUTTON_CHAT_SUPPORT  ||
		iCtrlID == IDC_BUTTON_EMAIL_SUPPORT ||
		iCtrlID == IDC_BUTTON_SUBMIT_TICKET ||
		iCtrlID == IDC_BUTTON_CLOSE	

		)
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if (csClassName == _T("Button") && m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}

	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}


/***************************************************************************
Function Name  : GetSupportNo
Description    : Get support No from registry.
Author Name    : neha gharge.
SR_NO		   :
Date           : 14th Sept 2015
****************************************************************************/
bool CSupportDlg::GetSupportNo()
{
	CITinRegWrapper	objReg;

	TCHAR	szPath[512] = { 0 };
	DWORD	dwSize = 511;

	objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WardWiz Antivirus", L"SupportNo", szPath, dwSize);
	if (!szPath[0])
	{
		AddLogEntry(L"### Failed to get support no entry", 0, 0, true, ZEROLEVEL);
		m_stSupportNo.SetWindowTextW(L"");
		m_stSupportContactNo.ShowWindow(SW_HIDE);
		return false;
	}

	m_stSupportNo.SetWindowTextW(szPath);
	return true;

}