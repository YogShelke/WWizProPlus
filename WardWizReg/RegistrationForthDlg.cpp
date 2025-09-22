// RegistrationForthDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWizRegistration.h"
#include "RegistrationForthDlg.h"


// CRegistrationForthDlg dialog

IMPLEMENT_DYNAMIC(CRegistrationForthDlg, CDialog)

CRegistrationForthDlg::CRegistrationForthDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CRegistrationForthDlg::IDD, pParent)
{

}

CRegistrationForthDlg::~CRegistrationForthDlg()
{
}

void CRegistrationForthDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FORTH_DLG_HEADER_MSG, m_stForthDlgHeaderMsg);
	DDX_Control(pDX, IDC_STATIC_INSTALL_CODE, m_stInstallationCode);
	DDX_Control(pDX, IDC_EDIT_INSTALLATION_CODE, m_edtInstallationCode);
	DDX_Control(pDX, IDC_STATIC_REQUEST_FOR_ACTIVATION_CODE, m_stReqActivationCodeText);
	DDX_Control(pDX, IDC_STATIC_ACTIVATION_CODE, m_stActivationCode);
	DDX_Control(pDX, IDC_BUTTON_FORTH_BACK, m_btnForthDlgBack);
	DDX_Control(pDX, IDC_BUTTON_FORTH_NEXT, m_btnForthDlgNext);
	DDX_Control(pDX, IDC_BUTTON_FORTH_CANCEL, m_btnForthDlgCancel);
	DDX_Control(pDX, IDC_EDIT_ACTIVATION_CODE, m_edtActivationCode);
	DDX_Control(pDX, IDC_STATIC_FIRST_HYPHEN, m_stFirstHyphen);
	DDX_Control(pDX, IDC_EDIT_ACTIVATION_CODE_TWO, m_edtActivationCodeSec);
	DDX_Control(pDX, IDC_STATIC_SEC_HYPHEN, m_stSecHyphen);
	DDX_Control(pDX, IDC_EDIT_ACTIVATION_CODE_THIRD, m_edtActivationCodeThird);
	DDX_Control(pDX, IDC_STATIC_THIRD_HYPHEN, m_stThirdHyphen);
	DDX_Control(pDX, IDC_EDIT_ACTIVATION_CODE_FORTH, m_edtActivationCodeForth);
	DDX_Control(pDX, IDC_STATIC_FORTH_HYPHEN, m_stForthHyphen);
	DDX_Control(pDX, IDC_EDIT_ACTIVATION_CODE_FIFTH, m_edtActivationCodeFifth);
	DDX_Control(pDX, IDC_STATIC_FIFTH_HYPHEN, m_stFifthHyphen);
	DDX_Control(pDX, IDC_EDIT_ACTIVATION_CODE_SIXTH, m_edtActivationCodeSixth);
	DDX_Control(pDX, IDC_STATIC_STEP_1, m_stStep1);
}


BEGIN_MESSAGE_MAP(CRegistrationForthDlg, CJpegDialog)
//		ON_WM_CTLCOLOR()
		ON_WM_NCHITTEST()
		ON_WM_SETCURSOR()
		ON_WM_PAINT()
		ON_BN_CLICKED(IDC_BUTTON_FORTH_BACK, &CRegistrationForthDlg::OnBnClickedButtonForthBack)
		ON_BN_CLICKED(IDC_BUTTON_FORTH_CANCEL, &CRegistrationForthDlg::OnBnClickedButtonForthCancel)
		ON_BN_CLICKED(IDC_BUTTON_FORTH_NEXT, &CRegistrationForthDlg::OnBnClickedButtonForthNext)
//		ON_WM_DESTROY()
ON_EN_CHANGE(IDC_EDIT_ACTIVATION_CODE, &CRegistrationForthDlg::OnEnChangeEditActivationCode)
ON_EN_CHANGE(IDC_EDIT_ACTIVATION_CODE_TWO, &CRegistrationForthDlg::OnEnChangeEditActivationCodeTwo)
ON_EN_CHANGE(IDC_EDIT_ACTIVATION_CODE_THIRD, &CRegistrationForthDlg::OnEnChangeEditActivationCodeThird)
ON_EN_CHANGE(IDC_EDIT_ACTIVATION_CODE_FORTH, &CRegistrationForthDlg::OnEnChangeEditActivationCodeForth)
ON_EN_CHANGE(IDC_EDIT_ACTIVATION_CODE_FIFTH, &CRegistrationForthDlg::OnEnChangeEditActivationCodeFifth)
ON_EN_CHANGE(IDC_EDIT_ACTIVATION_CODE_SIXTH, &CRegistrationForthDlg::OnEnChangeEditActivationCodeSixth)
END_MESSAGE_MAP()


// CRegistrationForthDlg message handlers

BOOL CRegistrationForthDlg::OnInitDialog()
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

	m_stForthDlgHeaderMsg.SetWindowPos(&wndTop,rect.left + 52,120,500,34,SWP_NOREDRAW | SWP_NOZORDER);
	m_stForthDlgHeaderMsg.SetBkColor(RGB(230,232,238));
	m_stForthDlgHeaderMsg.SetTextColor(RGB(24,24,24));
	m_stForthDlgHeaderMsg.ShowWindow(SW_HIDE);

	m_stInstallationCode.SetWindowPos(&wndTop,rect.left + 52,105,140,34,SWP_NOREDRAW | SWP_NOZORDER);
	m_stInstallationCode.SetBkColor(RGB(251,252,254));
	m_stInstallationCode.SetTextColor(RGB(24,24,24));

	m_edtInstallationCode.SetWindowPos(&wndTop,rect.left + 202,100,355,35,SWP_NOREDRAW | SWP_NOZORDER);
	m_edtInstallationCode.SetBkColor(RGB(251, 252, 254));
	GetDlgItem(IDC_EDIT_INSTALLATION_CODE)->ModifyStyle(0, WS_DISABLED);
	
	m_stStep1.SetWindowPos(&wndTop, rect.left + 52, 160, 620, 30, SWP_NOREDRAW | SWP_NOZORDER);
	m_stStep1.SetBkColor(RGB(251, 252, 254));
	m_stStep1.SetTextColor(RGB(24, 24, 24));

	m_stReqActivationCodeText.SetWindowPos(&wndTop,rect.left + 52,210,450,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_stReqActivationCodeText.SetBkColor(RGB(251,252,254));
	m_stReqActivationCodeText.SetTextColor(RGB(24,24,24));

	m_stActivationCode.SetWindowPos(&wndTop,rect.left + 52,270,130,20,SWP_NOREDRAW | SWP_NOZORDER);
	m_stActivationCode.SetBkColor(RGB(251,252,254));
	m_stActivationCode.SetTextColor(RGB(24,24,24));

	//issue no 1132 display not proper. Neha Gharge
	m_edtActivationCode.SetWindowPos(&wndTop,rect.left + 192,265,40,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_edtActivationCode.SetLimitText(2);
	m_edtActivationCode.SetFont(&theApp.m_fontWWTextSubTitle);
	 

	m_stFirstHyphen.SetWindowPos(&wndTop,rect.left + 239,262,15,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_stFirstHyphen.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stFirstHyphen.SetBkColor(RGB(251,252,254));
	m_stFirstHyphen.SetTextColor(RGB(24,24,24));
	
	m_edtActivationCodeSec.SetWindowPos(&wndTop,rect.left + 255,265,70,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_edtActivationCodeSec.SetLimitText(4);
	m_edtActivationCodeSec.SetFont(&theApp.m_fontWWTextSubTitle);

	m_stSecHyphen.SetWindowPos(&wndTop,rect.left + 332,262,15,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_stSecHyphen.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stSecHyphen.SetBkColor(RGB(251,252,254));
	m_stSecHyphen.SetTextColor(RGB(24,24,24));

	m_edtActivationCodeThird.SetWindowPos(&wndTop,rect.left + 350,265,70,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_edtActivationCodeThird.SetLimitText(4);
	m_edtActivationCodeThird.SetFont(&theApp.m_fontWWTextSubTitle);

	m_stThirdHyphen.SetWindowPos(&wndTop,rect.left + 427,262,15,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_stThirdHyphen.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stThirdHyphen.SetBkColor(RGB(251,252,254));
	m_stThirdHyphen.SetTextColor(RGB(24,24,24));

	m_edtActivationCodeForth.SetWindowPos(&wndTop,rect.left + 445,265,70,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_edtActivationCodeForth.SetLimitText(4);
	m_edtActivationCodeForth.SetFont(&theApp.m_fontWWTextSubTitle);

	m_stForthHyphen.SetWindowPos(&wndTop,rect.left + 522,262,15,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_stForthHyphen.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stForthHyphen.SetBkColor(RGB(251,252,254));
	m_stForthHyphen.SetTextColor(RGB(24,24,24));

	m_edtActivationCodeFifth.SetWindowPos(&wndTop,rect.left + 540,265,70,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_edtActivationCodeFifth.SetLimitText(4);
	m_edtActivationCodeFifth.SetFont(&theApp.m_fontWWTextSubTitle);

	m_stFifthHyphen.SetWindowPos(&wndTop,rect.left + 617,262,15,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_stFifthHyphen.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stFifthHyphen.SetBkColor(RGB(251,252,254));
	m_stFifthHyphen.SetTextColor(RGB(24,24,24));

	m_edtActivationCodeSixth.SetWindowPos(&wndTop,rect.left + 635,265,70,30,SWP_NOREDRAW | SWP_NOZORDER);
	m_edtActivationCodeSixth.SetLimitText(4);
	m_edtActivationCodeSixth.SetFont(&theApp.m_fontWWTextSubTitle);

	m_btnForthDlgBack.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnForthDlgBack.SetWindowPos(&wndTop, rect.left+ 52, 340,57,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnForthDlgBack.SetTextColorA((0,0,0),1,1);

	m_btnForthDlgNext.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnForthDlgNext.SetWindowPos(&wndTop, rect.left + 555, 340, 57,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnForthDlgNext.SetTextColorA((0,0,0),1,1);

	m_btnForthDlgCancel.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnForthDlgCancel.SetWindowPos(&wndTop, rect.left + 630, 340, 57,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnForthDlgCancel.SetTextColorA((0,0,0),1,1);

	RefreshString();
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CRegistrationForthDlg :: RefreshString()
{
/*	m_stForthDlgHeaderMsg.SetWindowTextW(L"PLease send below installation code vai SMS and follow instruction");
	m_stForthDlgHeaderMsg.SetFont(&theApp.m_fontWWTextTitle);*/	

	m_stInstallationCode.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_INSTALATION_CODE"));
	m_stInstallationCode.SetFont(&theApp.m_fontWWTextSubTitle);

	m_edtInstallationCode.SetFont(&theApp.m_fontInnerDialogTitle);
	m_edtInstallationCode.RedrawWindow();

	m_stStep1.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_STEP1_OFFLINE_TEXT"));
	m_stStep1.SetFont(&theApp.m_fontWWTextSubTitle);

	m_stReqActivationCodeText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REQUEST_OFFLINE_TEXT"));
	m_stReqActivationCodeText.SetFont(&theApp.m_fontWWTextSubTitle);

	m_stActivationCode.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ACTIVATION_CODE"));
	m_stActivationCode.SetFont(&theApp.m_fontWWTextSubTitle);

	m_edtActivationCode.SetFont(&theApp.m_fontWWTextSubTitle);

	m_edtActivationCode.SetFocus();
	m_edtActivationCode.RedrawWindow();

	m_edtActivationCodeSec.RedrawWindow();

	m_edtActivationCodeThird.RedrawWindow();

	m_edtActivationCodeForth.RedrawWindow();

	m_edtActivationCodeFifth.RedrawWindow();

	m_edtActivationCodeSixth.RedrawWindow();

	m_btnForthDlgNext.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_BUTTON_NEXT"));
	m_btnForthDlgNext.SetFont(&theApp.m_fontWWTextNormal);

	m_btnForthDlgBack.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_BUTTON_BACK"));
	m_btnForthDlgBack.SetFont(&theApp.m_fontWWTextNormal);

	m_btnForthDlgCancel.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_BUTTON_CANCEL"));
	m_btnForthDlgCancel.SetFont(&theApp.m_fontWWTextNormal);

	m_stFirstHyphen.SetWindowTextW(L"_");
	m_stSecHyphen.SetWindowTextW(L"_");
	m_stThirdHyphen.SetWindowTextW(L"_");
	m_stForthHyphen.SetWindowTextW(L"_");
	m_stFifthHyphen.SetWindowTextW(L"_");
	Invalidate();
	return TRUE;
}

LRESULT CRegistrationForthDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

bool CRegistrationForthDlg::ShowHideForthDlg(bool bEnable)
{
	m_stForthDlgHeaderMsg.ShowWindow(SW_HIDE);
	m_stInstallationCode.ShowWindow(bEnable);
	m_edtInstallationCode.SetWindowTextW(theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_szInstallCode);
	m_edtInstallationCode.ShowWindow(bEnable);
	m_stReqActivationCodeText.ShowWindow(bEnable);
	m_stActivationCode.ShowWindow(bEnable);
	m_edtActivationCode.ShowWindow(bEnable);
	m_edtActivationCodeSec.ShowWindow(bEnable);
	m_edtActivationCodeThird.ShowWindow(bEnable);
	m_edtActivationCodeForth.ShowWindow(bEnable);
	m_edtActivationCodeFifth.ShowWindow(bEnable);
	m_edtActivationCodeSixth.ShowWindow(bEnable);
	m_stFirstHyphen.ShowWindow(bEnable);
	m_stSecHyphen.ShowWindow(bEnable);
	m_stThirdHyphen.ShowWindow(bEnable);
	m_stForthHyphen.ShowWindow(bEnable);
	m_stFifthHyphen.ShowWindow(bEnable);
	m_btnForthDlgBack.ShowWindow(bEnable);
	m_btnForthDlgCancel.ShowWindow(bEnable);
	m_btnForthDlgNext.ShowWindow(bEnable);
	m_edtActivationCode.SetWindowTextW(L"");
	m_edtActivationCodeSec.SetWindowTextW(L"");
	m_edtActivationCodeThird.SetWindowTextW(L"");
	m_edtActivationCodeForth.SetWindowTextW(L"");
	m_edtActivationCodeFifth.SetWindowTextW(L"");
	m_edtActivationCodeSixth.SetWindowTextW(L"");
	Invalidate();
	return true;
}
void CRegistrationForthDlg::OnBnClickedButtonForthBack()
{
	ShowHideForthDlg(false);
	CRegistrationSecondDlg *pObjRegWindow = reinterpret_cast<CRegistrationSecondDlg*>(this->GetParent());
	this->ShowWindow(SW_HIDE);
	theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_SEL_OPTION"));
	pObjRegWindow->ShutdownThread();
	theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_bisThreadCompleted = false;
	//pObjRegWindow->ResetVariable();
	pObjRegWindow->ShowHideSecondDlg(true);
}

void CRegistrationForthDlg::OnBnClickedButtonForthCancel()
{
	this->ShowWindow(SW_HIDE);
	theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.OnBnClickedButtonCancel();
}

void CRegistrationForthDlg::OnBnClickedButtonForthNext()
{
	_tcscpy(m_szActivationCode,L"");
	_tcscpy(m_szActivationCodeSec,L"");
	_tcscpy(m_szActivationCodeThird,L"");
	_tcscpy(m_szActivationCodeForth,L"");
	_tcscpy(m_szActivationCodeFifth,L"");
	_tcscpy(m_szActivationCodeSixth,L"");
	m_edtActivationCode.GetWindowText(m_szActivationCode,4);
	m_edtActivationCodeSec.GetWindowText(m_szActivationCodeSec,8);
	m_edtActivationCodeThird.GetWindowText(m_szActivationCodeThird,8);
	m_edtActivationCodeForth.GetWindowText(m_szActivationCodeForth,8);
	m_edtActivationCodeFifth.GetWindowText(m_szActivationCodeFifth,8);
	m_edtActivationCodeSixth.GetWindowText(m_szActivationCodeSixth,8);
	int TotalActivationLenght = static_cast<int>(((_tcslen(m_szActivationCode)) + (_tcslen(m_szActivationCodeSec)) + (_tcslen(m_szActivationCodeThird)) + (_tcslen(m_szActivationCodeForth)) + (_tcslen(m_szActivationCodeFifth)) + (_tcslen(m_szActivationCodeSixth))));
	if(TotalActivationLenght< 22)
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_ENTER_CORRECT_ACTIVATION_CODE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	_tcscat(m_szActivationCode,m_szActivationCodeSec);
	_tcscat(m_szActivationCode,m_szActivationCodeThird);
	_tcscat(m_szActivationCode,m_szActivationCodeForth);
	_tcscat(m_szActivationCode,m_szActivationCodeFifth);
	_tcscat(m_szActivationCode,m_szActivationCodeSixth);
	this->ShowWindow(SW_HIDE);
	SetEvent(theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_hOfflineNextEvent);
	theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_PROCESSING"));
	theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_objRegistrationThirdDlg.RefreshString();
	theApp.m_objRegistrationDlg.m_objRegistrationSecondDlg.m_objRegistrationThirdDlg.ShowWindow(SW_SHOW);
}


BOOL CRegistrationForthDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(!pWnd)
		return FALSE;
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();

	if( iCtrlID == IDC_BUTTON_FORTH_NEXT	||
		iCtrlID == IDC_BUTTON_FORTH_CANCEL	||
		iCtrlID == IDC_BUTTON_FORTH_BACK
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

BOOL CRegistrationForthDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);

}

void CRegistrationForthDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::OnPaint();
}

//HBRUSH CRegistrationForthDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
//{
//	 HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
//
//	 switch (nCtlColor)
//	 {
//		 case CTLCOLOR_EDIT:              // Set color to green on black and return the background brush.
//              pDC->SetBkColor(RGB(251,252,254));
//              return (HBRUSH)(m_pEditBkBrush->GetSafeHandle());
//	    default:
//              return CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
//	 }
//}

//void CRegistrationForthDlg::OnDestroy()
//{
//	CJpegDialog::OnDestroy();
//
//	
//   // Free the space allocated for the background brush
//   delete m_pEditBkBrush;
//}

void CRegistrationForthDlg::OnEnChangeEditActivationCode()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CJpegDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	//Issue no 1078 : Block should be get selected when we changed any thing in previous block
	//Neha Gharge
	if(m_edtActivationCode.GetWindowTextLength() == 2)
	{
		m_edtActivationCodeSec.SetFocus();
		m_edtActivationCodeSec.SetSel(0, 4, FALSE);
	}
}

void CRegistrationForthDlg::OnEnChangeEditActivationCodeTwo()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CJpegDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	//Issue no 1078 : Block should be get selected when we changed any thing in previous block
	//Neha Gharge
	if(m_edtActivationCodeSec.GetWindowTextLength() == 4)
	{
		m_edtActivationCodeThird.SetFocus();
		m_edtActivationCodeThird.SetSel(0, 4, FALSE);
	}
}

void CRegistrationForthDlg::OnEnChangeEditActivationCodeThird()
{
	//Issue no 1078 : Block should be get selected when we changed any thing in previous block
	//Neha Gharge
	if(m_edtActivationCodeThird.GetWindowTextLength() == 4)
	{
		m_edtActivationCodeForth.SetFocus();
		m_edtActivationCodeForth.SetSel(0, 4, FALSE);
	}
}

void CRegistrationForthDlg::OnEnChangeEditActivationCodeForth()
{
	//Issue no 1078 : Block should be get selected when we changed any thing in previous block
	//Neha Gharge
	if(m_edtActivationCodeForth.GetWindowTextLength() == 4)
	{
		m_edtActivationCodeFifth.SetFocus();
		m_edtActivationCodeFifth.SetSel(0, 4, FALSE);
	}
}

void CRegistrationForthDlg::OnEnChangeEditActivationCodeFifth()
{
	//Issue no 1078 : Block should be get selected when we changed any thing in previous block
	//Neha Gharge
	if(m_edtActivationCodeFifth.GetWindowTextLength() == 4)
	{
		m_edtActivationCodeSixth.SetFocus();
		m_edtActivationCodeSixth.SetSel(0, 4, FALSE);
	}
}

void CRegistrationForthDlg::OnEnChangeEditActivationCodeSixth()
{
	if(m_edtActivationCodeSixth.GetWindowTextLength() == 4)
	{
		m_btnForthDlgNext.SetFocus();
	}
}
