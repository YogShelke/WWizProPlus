// ISpyPasswordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWizCrypt.h"
#include "ISpyPasswordDlg.h"
#include "afxdialogex.h"


// CISpyPasswordDlg dialog

IMPLEMENT_DYNAMIC(CISpyPasswordDlg, CDialogEx)

CISpyPasswordDlg::CISpyPasswordDlg(CWnd* pParent /*=NULL*/)
: CJpegDialog(CISpyPasswordDlg::IDD, pParent)
{

}

CISpyPasswordDlg::~CISpyPasswordDlg()
{
}

void CISpyPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	//DDX_Control(pDX, IDOK, m_btnOK);
	//DDX_Control(pDX, IDCANCEL, m_btnCancle);
	DDX_Control(pDX, IDC_BUTTN_OK, m_btnOK);
	DDX_Control(pDX, IDC_BUTTON_CANCLE, m_btnCancle);
	DDX_Control(pDX, IDC_STATIC_GROUPBOX, m_stGroupBox);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_STATIC_DES, m_stDes);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_stTitle);
}


BEGIN_MESSAGE_MAP(CISpyPasswordDlg, CJpegDialog)
	ON_WM_CTLCOLOR()

	ON_BN_CLICKED(IDC_BUTTON_CANCLE, &CISpyPasswordDlg::OnBnClickedButtonCancle)
	ON_BN_CLICKED(IDC_BUTTN_OK, &CISpyPasswordDlg::OnBnClickedButtnOk)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CISpyPasswordDlg::OnClose)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CISpyPasswordDlg message handlers


HBRUSH CISpyPasswordDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int	ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();

	if (ctrlID == IDC_BUTTN_OK ||
		ctrlID == IDC_BUTTON_CANCLE ||
		ctrlID == IDC_BUTTON_CLOSE ||
		ctrlID == IDC_STATIC_DES ||
		ctrlID == IDC_STATIC_TITLE 
		
		)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}	return hbr;

}


BOOL CISpyPasswordDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	if (!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_PASSWORD_BG), _T("JPG")))
	{
		::MessageBox(NULL, L"Failed", L"WardWiz", MB_ICONERROR);
	}

	Draw();

	CRect rect;
	this->GetClientRect(rect);

	CRgn		 rgn;
	rgn.CreateRoundRectRgn(rect.left, rect.top, rect.right - 2, rect.bottom - 2, 0, 0);
	this->SetWindowRgn(rgn, TRUE);

	m_btnClose.SetWindowPos(&wndTop, rect.left + 290, 0, 26, 17, SWP_NOREDRAW);
	m_btnClose.SetSkin(AfxGetResourceHandle(), IDB_BITMAP_CLOSE_BUTTON, IDB_BITMAP_CLOSE_BUTTON, IDB_BITMAP_CLOSE_BUTTON_HOVER, IDB_BITMAP_CLOSE_BUTTON, IDB_BITMAP_CLOSE_BUTTON_HOVER, 0, 0, 0, 0);
//
	m_stTitle.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASS_DLG_DEC_PASS"));
	m_stTitle.SetWindowPos(&wndTop, rect.left + 62, 12, 150, 25, SWP_NOREDRAW);
	CFont fontWWTextSmallTitle;
	fontWWTextSmallTitle.CreatePointFont(180, _T("Verdana"), 0);
	m_stTitle.SetFont(&fontWWTextSmallTitle);
	m_stTitle.SetTextColor(RGB(255, 255, 255));


	m_btnCancle.SetWindowPos(&wndTop, rect.left + 217, rect.top + 132, 58, 21, SWP_NOREDRAW);
	m_btnCancle.SetSkin(AfxGetResourceHandle(), IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, 0, 0, 0, 0, 0);
	m_btnCancle.SetTextColorA(BLACK, 1, 1);

	m_btnOK.SetWindowPos(&wndTop, rect.left + 145, rect.top+132, 58, 21, SWP_NOREDRAW);
	m_btnOK.SetSkin(AfxGetResourceHandle(), IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, 0, 0, 0, 0, 0);
	m_btnOK.SetTextColorA(BLACK, 1, 1);

	m_stGroupBox.SetWindowPos(&wndTop, rect.left + 25, rect.top + 50, 275, 115, SWP_NOREDRAW);

	m_editPassword.SetWindowPos(&wndTop, rect.left + 45, rect.top + 90, 230, 25, SWP_NOREDRAW);

	m_stDes.SetWindowPos(&wndTop, rect.left + 45, rect.top + 70, 220, 25, SWP_NOREDRAW);
	m_stDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASS_DLG_ENTER_PASS"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CISpyPasswordDlg::OnBnClickedButtonCancle()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


void CISpyPasswordDlg::OnBnClickedButtnOk()
{
	// TODO: Add your control notification handler code here
	m_editPassword.GetWindowText(theApp.m_csPassword);
	if (theApp.m_csPassword !=L"")
		OnOK();
	else
	{
		m_editPassword.SetFocus();
	}
}


void CISpyPasswordDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	OnCancel();
}


BOOL CISpyPasswordDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}
