/**********************************************************************************************************      		Program Name          : ISpyVirusPopUpDlg.cpp
	Description           : This class is derived from CDialog which used to show when any virus got detected
							on machine it will prompt to user.
	Author Name			  : Neha Gharge
	Date Of Creation      : 07th Aug 2014
	Version No            : 1.0.0.8
	Special Logic Used    : 
	Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyVirusPopUpDlg.h"
#include "OutlookAddinApp.h"

/*	ISSUE NO - 623 NAME - NITIN K. TIME - 9th June 2014 */
LPCTSTR g_OptionsForComboBox[2]={L"Repair",L"Delete"};

IMPLEMENT_DYNAMIC(CISpyVirusPopUpDlg, CDialog)

/***************************************************************************
  Function Name  : CISpyVirusPopUpDlg
  Description    : Cont'r
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0081
  Date           : 07th Aug 2014
****************************************************************************/
CISpyVirusPopUpDlg::CISpyVirusPopUpDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyVirusPopUpDlg::IDD, pParent)
	, m_dwAction(0)
{

}

/***************************************************************************
  Function Name  : ~CISpyVirusPopUpDlg
  Description    : Cont'r
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0082
  Date           : 07th Aug 2014
****************************************************************************/
CISpyVirusPopUpDlg::~CISpyVirusPopUpDlg()
{
}

/***************************************************************************
  Function Name  : DoDataExchange
  Description    : Called by the framework to exchange and validate dialog data.
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0083
  Date           : 07th Aug 2014
****************************************************************************/
void CISpyVirusPopUpDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_THREATNAME, m_stThreatName);
	DDX_Control(pDX, IDC_STATIC_SCANOBJECT, m_stScannedObject);
	DDX_Control(pDX, IDC_STATIC_SENDERADDR, m_stSendersAddr);
	DDX_Control(pDX, IDC_STATIC_ACTION, m_stActionReq);
	DDX_Control(pDX, IDC_STATIC_THREAT, m_stThreatFound);
	DDX_Text(pDX, IDC_STATIC_THREAT, m_csThreatName);
	DDX_Control(pDX, IDC_STATIC_OBJECT, m_stObject);
	DDX_Text(pDX, IDC_STATIC_OBJECT, m_csAttachmentName);
	DDX_Control(pDX, IDC_STATIC_SENDER_ADDRESS, m_stAddress);
	DDX_Text(pDX, IDC_STATIC_SENDER_ADDRESS, m_csSendersAddress);
	DDX_Control(pDX, IDC_COMBO_ACTION, m_comboActionList);
	DDX_Control(pDX, IDC_STATIC_VIRUSSCAN_HPIC, m_stVirusScanHPic);
	DDX_Control(pDX, IDC_BUTTON_MINIMIZE, m_btnMinimize);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CISpyVirusPopUpDlg, CJpegDialog)
	ON_BN_CLICKED(IDCANCEL, &CISpyVirusPopUpDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CISpyVirusPopUpDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_MINIMIZE, &CISpyVirusPopUpDlg::OnBnClickedButtonMinimize)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CISpyVirusPopUpDlg::OnBnClickedButtonClose)
END_MESSAGE_MAP()

/***************************************************************************
  Function Name  : OnInitDialog
  Description    : Called in response to the WM_INITDIALOG message.
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0084
  Date           : 07th Aug 2014
****************************************************************************/
BOOL CISpyVirusPopUpDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	if(!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_VIRUSSCAN_BG), _T("JPG")))
	{
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}

	Draw();
	CRect rect;
	this->GetClientRect(rect);
	SetWindowPos(&wndTop, rect.top + 430,rect.top + 250, rect.Width()-3, rect.Height() - 3, SWP_NOREDRAW);

	m_bmpVirusScanPic.LoadBitmapW(IDB_BITMAP_VIRUSSCAN_HPIC);
	m_stVirusScanHPic.SetBitmap(m_bmpVirusScanPic);
	m_stVirusScanHPic.SetWindowPos(&wndTop,rect.left + 0,50,379,42,SWP_NOREDRAW);

	m_stThreatName.SetWindowPos(&wndTop,rect.left + 23,94,90,20,SWP_NOREDRAW);
	m_stThreatName.SetBkColor(RGB(255,255,255));
	
	m_stThreatFound.SetWindowPos(&wndTop,rect.left + 120,94,133,20,SWP_NOREDRAW);
	m_stThreatFound.SetBkColor(RGB(255,255,255));
	

	m_stScannedObject.SetWindowPos(&wndTop,rect.left + 23,114,90,20,SWP_NOREDRAW);
	m_stScannedObject.SetBkColor(RGB(255,255,255));
	m_stObject.SetWindowPos(&wndTop,rect.left + 120,114,133,20,SWP_NOREDRAW);
	m_stObject.SetBkColor(RGB(255,255,255));

	m_stSendersAddr.SetWindowPos(&wndTop,rect.left + 23,134,90,20,SWP_NOREDRAW);
	m_stSendersAddr.SetBkColor(RGB(255,255,255));

	m_stAddress.SetWindowPos(&wndTop,rect.left + 120,134,153,20,SWP_NOREDRAW);
	m_stAddress.SetBkColor(RGB(255,255,255));
	//m_stAddress.SetWindowText(m_csSendersAddress);

	m_stActionReq.SetWindowPos(&wndTop,rect.left + 23,154,90,20,SWP_NOREDRAW);
	m_stActionReq.SetBkColor(RGB(255,255,255));
	m_comboActionList.SetWindowPos(&wndTop,rect.left + 120,154,133,20,SWP_NOREDRAW);
	m_comboActionList.AddString(g_OptionsForComboBox[0]);
	m_comboActionList.AddString(g_OptionsForComboBox[1]);
	m_comboActionList.SetCurSel(0);
	

	m_btnMinimize.SetSkin(IDB_BITMAP_MINIMIZE,IDB_BITMAP_MINIMIZE,IDB_BITMAP_HMINIMIZE,0,0,0,0,0,0);
	m_btnMinimize.SetWindowPos(&wndTop,rect.left + 324,0,26,17,SWP_NOREDRAW);

	m_btnClose.SetSkin(IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_HCLOSE,0,0,0,0,0,0);
	m_btnClose.SetWindowPos(&wndTop,rect.left + 350,0,26,17,SWP_NOREDRAW);
	
	m_btnOk.SetSkin(IDB_BITMAP_48x16_Btn,IDB_BITMAP_48x16_Btn,IDB_BITMAP_48x16_HBtn,0,0,0,0,0,0);
	m_btnOk.SetWindowPos(&wndTop,rect.left + 265,184,48,16,SWP_NOREDRAW);
	m_btnOk.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FLDRLCKPSSWRD_OK"));
	m_btnOk.SetTextColor(RGB(255,255,255));

	m_btnCancel.SetSkin(IDB_BITMAP_48x16_Btn,IDB_BITMAP_48x16_Btn,IDB_BITMAP_48x16_HBtn,0,0,0,0,0,0);
	m_btnCancel.SetWindowPos(&wndTop,rect.left + 323,184,48,16,SWP_NOREDRAW);
	m_btnCancel.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FLDRLCKPSSWRD_CANCEL"));
	m_btnCancel.SetTextColor(RGB(255,255,255));


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************
  Function Name  : OnBnClickedCancel
  Description    : Cancel button click handler
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0085
  Date           : 07th Aug 2014
****************************************************************************/
void CISpyVirusPopUpDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

/***************************************************************************
  Function Name  : OnBnClickedOk
  Description    : Ok button click handler
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0086
  Date           : 07th Aug 2014
****************************************************************************/
void CISpyVirusPopUpDlg::OnBnClickedOk()
{
	if(theApp.m_dwDaysLeft == 0)
	{
		theApp.ShowEvaluationExpiredMsg();
		return;
	}

	m_dwAction = m_comboActionList.GetCurSel();

	OnOK();
}

/***************************************************************************
  Function Name  : OnBnClickedButtonMinimize
  Description    : Minimize button click handler
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0087
  Date           : 07th Aug 2014
****************************************************************************/
void CISpyVirusPopUpDlg::OnBnClickedButtonMinimize()
{
	// TODO: Add your control notification handler code here
	this->ShowWindow(SW_MINIMIZE);
}

/***************************************************************************
  Function Name  : OnBnClickedButtonClose
  Description    : Close button click handler
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0088
  Date           : 07th Aug 2014
****************************************************************************/
void CISpyVirusPopUpDlg::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

/***************************************************************************
  Function Name  : PreTranslateMessage
  Description    : Override this function to filter window messages before they are dispatched to the Windows							functions TranslateMessage and DispatchMessage The default implementation performs accelerator-						key translation, so you must call the CWinApp::PreTranslateMessage member function in your							overridden version.
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0089
  Date           : 07th Aug 2014
****************************************************************************/
BOOL CISpyVirusPopUpDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
	
}
