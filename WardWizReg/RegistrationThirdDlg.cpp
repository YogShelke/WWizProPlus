// RegistrationThirdDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RegistrationThirdDlg.h"
//#include "RegistrationSecondDlg.h"
//#include "RegistrationDlg.h"
#include "WardWizRegistration.h"
//CRegistrationDlg g_objRegistration;
//CRegistrationSecondDlg g_objRegistrationSecondDlg;

IMPLEMENT_DYNAMIC(CRegistrationThirdDlg, CDialog)

CRegistrationThirdDlg::CRegistrationThirdDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CRegistrationThirdDlg::IDD, pParent)
	,m_pBoldFont(NULL)
{

}

CRegistrationThirdDlg::~CRegistrationThirdDlg()
{
	if(m_pBoldFont != NULL)
	{
		delete m_pBoldFont;
		m_pBoldFont = NULL;
	}
}

void CRegistrationThirdDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PRELOAD, m_stPreloaderImage);
	DDX_Control(pDX, IDC_STATIC_CONNECTTOSERVER_TEXT, m_stConnectToServer);
	DDX_Control(pDX, IDC_STATIC_CORRECT_PIC, m_stCorrectMsgPic);
	DDX_Control(pDX, IDC_STATIC_FAILED_PIC, m_stFailedMsgPic);
	DDX_Control(pDX, IDC_STATIC_CORRECT_MSG, m_stSuccessMsg);
	DDX_Control(pDX, IDC_STATIC_FAILED_MSG, m_stFailedText);

	DDX_Control(pDX, IDC_BUTTON_THIRD_FINISH, m_btnThirdFinish);
	DDX_Control(pDX, IDC_BUTTON_THIRD_CANCEL, m_btnThirdCancel);
}

BEGIN_MESSAGE_MAP(CRegistrationThirdDlg, CJpegDialog)
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_THIRD_FINISH, &CRegistrationThirdDlg::OnBnClickedButtonThirdFinish)
	ON_BN_CLICKED(IDC_BUTTON_THIRD_CANCEL, &CRegistrationThirdDlg::OnBnClickedButtonThirdCancel)
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CRegistrationThirdDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SECOND_REGISTRATIONUI), _T("JPG")))
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	}

	Draw();
	
	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect;
	this->GetClientRect(rect);

	SetWindowPos(NULL, 0, 0, rect.Width()-5, rect.Height() - 30, SWP_NOREDRAW);
	//Added reenter button if it failes in any case.
	//Neha Gharge 10 Aug,2015
	// TODO:  Add extra initialization here
	if (m_stPreloaderImage.Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_GIF_LOADING),_T("GIF")))
		m_stPreloaderImage.SetWindowPos(&wndTop, rect.left + 350, 150, 105,91, SWP_NOREDRAW | SWP_NOZORDER);
		if(!m_stPreloaderImage.IsPlaying())
		{
			m_stPreloaderImage.Draw();
		}
		m_stPreloaderImage.ShowWindow(SW_SHOW);
	//m_stPreloaderImage.Stop();

	m_pBoldFont=NULL;
	m_pBoldFont = new(CFont);
	m_pBoldFont->CreateFont(25,10,0,0,0,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"MyriadPro-Regular");


	m_stConnectToServer.SetWindowPos(&wndTop, rect.left + 285,250,390,30, SWP_NOREDRAW | SWP_NOZORDER);
	m_stConnectToServer.SetBkColor(RGB(251,252,254));
	m_stConnectToServer.SetFont(m_pBoldFont);
	m_stConnectToServer.ShowWindow(SW_SHOW);
	
	//m_bmpPicCorrect.LoadBitmapW(IDB_BITMAP_CHECKMARK);
	//m_stCorrectMsgPic.SetBitmap(m_bmpPicCorrect);
	m_stCorrectMsgPic.SetWindowPos(&wndTop, rect.left + 340, 150, 105,91, SWP_NOREDRAW | SWP_NOZORDER);
	m_bmpPicCorrect = LoadBitmapW(theApp.m_hResDLL, MAKEINTRESOURCE(IDB_BITMAP_REG_CORRECT));
	m_stCorrectMsgPic.SetBitmap(m_bmpPicCorrect);
	m_stCorrectMsgPic.ShowWindow(SW_HIDE);

	/*	ISSUE NO - 345 NAME - NITIN K. TIME - 24st May 2014 */
	m_stSuccessMsg.SetWindowPos(&wndTop, rect.left + 290, 250,390,30, SWP_NOREDRAW | SWP_NOZORDER);
	m_stSuccessMsg.SetBkColor(RGB(251,252,254));
	m_stSuccessMsg.SetFont(m_pBoldFont);
	m_stSuccessMsg.ShowWindow(SW_HIDE);

	//m_bmpPicFailed.LoadBitmapW(IDB_BITMAP_CROSSMARK);
	//m_stFailedMsgPic.SetBitmap(m_bmpPicFailed);
	m_stFailedMsgPic.SetWindowPos(&wndTop, rect.left + 340, 150, 105,91, SWP_NOREDRAW | SWP_NOZORDER);
	m_bmpPicFailed = LoadBitmapW(theApp.m_hResDLL, MAKEINTRESOURCE(IDB_BITMAP_REG_FAILED));
	m_stFailedMsgPic.SetBitmap(m_bmpPicFailed);
	m_stFailedMsgPic.ShowWindow(SW_HIDE);

	/*	ISSUE NO - 345 NAME - NITIN K. TIME - 24st May 2014 */
	m_stFailedText.SetWindowPos(&wndTop, rect.left + 310, 250,390,30, SWP_NOREDRAW | SWP_NOZORDER);
	m_stFailedText.SetBkColor(RGB(251,252,254));
	m_stFailedText.SetFont(m_pBoldFont);
	m_stFailedText.ShowWindow(SW_HIDE);

	m_btnThirdFinish.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG  ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnThirdFinish.SetWindowPos(&wndTop, rect.left + 535, 340, 57,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnThirdFinish.SetTextColorA((0,0,0),1,1);
	m_btnThirdFinish.EnableWindow(false);

	m_btnThirdCancel.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG  ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnThirdCancel.SetWindowPos(&wndTop, rect.left + 610, 340, 57,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnThirdCancel.SetTextColorA((0,0,0),1,1);
	m_btnThirdCancel.ShowWindow(SW_HIDE);

	RefreshString();

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CRegistrationThirdDlg::ShowMsg(bool bMsgType)
{
	//Added reenter button if it failes in any case.
	//Neha Gharge 10 Aug,2015
	CRect rect;
	this->GetClientRect(rect);
	if(bMsgType)
	{
		if(m_stPreloaderImage.IsPlaying())
		{
			m_stPreloaderImage.Stop();
		}
		m_stPreloaderImage.ShowWindow(SW_HIDE);
		m_stConnectToServer.ShowWindow(SW_HIDE);
		//issue no 802 Neha gharge
		m_stFailedMsgPic.ShowWindow(SW_HIDE);
		m_stFailedText.ShowWindow(SW_HIDE);
		m_stCorrectMsgPic.ShowWindow(SW_SHOW);
		m_stCorrectMsgPic.RedrawWindow();
		m_stSuccessMsg.ShowWindow(SW_SHOW);
		m_stSuccessMsg.RedrawWindow();
		m_btnThirdCancel.EnableWindow(true);
		m_btnThirdCancel.ShowWindow(SW_HIDE);
		m_btnThirdFinish.EnableWindow(true);
		m_btnThirdFinish.SetWindowPos(&wndTop, rect.left + 660, 340, 57,21, SWP_SHOWWINDOW);
		m_btnThirdFinish.ShowWindow(SW_SHOW);
		theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_bisThreadCompleted = false;
		
	}
	else
	{
		if(m_stPreloaderImage.IsPlaying())
		{
			m_stPreloaderImage.Stop();
		}
		m_stPreloaderImage.ShowWindow(SW_HIDE);
		m_stConnectToServer.ShowWindow(SW_HIDE);
		m_stCorrectMsgPic.ShowWindow(SW_HIDE);
		m_stSuccessMsg.ShowWindow(SW_HIDE);
		//issue no 802 Neha gharge
		m_stFailedMsgPic.ShowWindow(SW_SHOW);
		m_stFailedMsgPic.RedrawWindow();
		m_stFailedText.ShowWindow(SW_SHOW);
		m_stFailedText.RedrawWindow();
		m_btnThirdCancel.EnableWindow(true);
		m_btnThirdCancel.ShowWindow(SW_SHOW);
		m_btnThirdCancel.SetWindowPos(&wndTop, rect.left + 595, 340, 57, 21, SWP_SHOWWINDOW);
		m_btnThirdFinish.EnableWindow(true);
		m_btnThirdFinish.SetWindowPos(&wndTop, rect.left + 660, 340,57,21, SWP_SHOWWINDOW);
		m_btnThirdFinish.ShowWindow(SW_SHOW);
		theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_bisThreadCompleted = false;
	}
	if(theApp.m_hRegMutexHandle)	
	{
		CloseHandle(theApp.m_hRegMutexHandle);
		theApp.m_hRegMutexHandle = NULL;
	}
}
void CRegistrationThirdDlg::OnBnClickedButtonThirdFinish()
{
	theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.ShutdownThread();
	::SendMessage(AfxGetMainWnd()->m_hWnd, WM_CLOSE, 0, 0);	
}

void CRegistrationThirdDlg::OnBnClickedButtonThirdCancel()
{
	//Added reenter button if it failes in any case.
	//Neha Gharge 10 Aug,2015
	theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.ShutdownThread();
	CRegistrationSecondDlg *pObjRegWindow = reinterpret_cast<CRegistrationSecondDlg*>(this->GetParent());
	m_stConnectToServer.ShowWindow(SW_SHOW);
	m_stCorrectMsgPic.ShowWindow(SW_HIDE);
	m_stSuccessMsg.ShowWindow(SW_HIDE);
	m_stFailedMsgPic.ShowWindow(SW_HIDE);
	m_stFailedText.ShowWindow(SW_HIDE);
	m_stPreloaderImage.ShowWindow(SW_SHOW);
	this->ShowWindow(SW_HIDE);
	pObjRegWindow->OnBnClickedButtonBack();

}

void CRegistrationThirdDlg::EnableFinish(bool bEnable)
{
	m_btnThirdFinish.EnableWindow(bEnable);
	m_btnThirdFinish.ShowWindow(SW_SHOW);
	
	if(bEnable)
	{
		m_btnThirdCancel.EnableWindow(true);
	}
}

LRESULT CRegistrationThirdDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

BOOL CRegistrationThirdDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(!pWnd)
		return FALSE;
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();

	if( 
		iCtrlID == IDC_BUTTON_THIRD_FINISH	||
		iCtrlID == IDC_BUTTON_THIRD_CANCEL
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

BOOL CRegistrationThirdDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);

}

/***********************************************************************************************
  Function Name  : RefreshString
  Description    : this function is  called for setting the Text UI with different Language Support
  Author Name    : Nitin Kolapkar
  Date           : 29 April 2014
***********************************************************************************************/
BOOL CRegistrationThirdDlg :: RefreshString()
{
	if (theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_bOnlineActivation == true || theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_bTryProduct == true)
	{
		m_stConnectToServer.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CONNECTING_SERVER"));

	}
	else if (theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_bOfflineActivation == true)
	{
		m_stConnectToServer.SetWindowTextW((L"      ") + theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_OFFLINE_ACTIVATION"));
	}

	m_btnThirdFinish.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_BUTTON_FINISH"));
	m_btnThirdFinish.SetFont(&theApp.m_fontWWTextNormal);
	
	m_btnThirdCancel.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_BUTTON_CANCEL"));
	//m_btnThirdCancel.SetWindowText(L"Re-Enter");
	m_btnThirdCancel.SetFont(&theApp.m_fontWWTextNormal);

	m_stSuccessMsg.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_STATIC_SUCCESS"));
	m_stSuccessMsg.SetFont(&theApp.m_fontWWTextTitle);

	m_stFailedText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_STATIC_FAIL"));
	m_stFailedText.SetFont(&theApp.m_fontWWTextTitle);

	//m_stConnectToServer.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_CONNECTING"));
	return true;
}

void CRegistrationThirdDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::OnPaint();
}
