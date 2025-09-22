/*******************************************************************************
*  Program Name:  MarQDlg.cpp
*  Description: Show Marquee with Message for EPS SQLSERVER Installation 
*  Author Name: Tejas Shinde
*  Date Of Creation: 31 January 2019
********************************************************************************/

#include "stdafx.h"
#include "MarQDlg.h"
#include "afxdialogex.h"


// CMarQDlg dialog

IMPLEMENT_DYNAMIC(CMarQDlg, CDialog)

/***************************************************************************************************
*  Function Name  : CMarQDlg
*  Description    : C'tor
*  Author Name    : Tejas Shinde
*  Date           : 31 January 2019
****************************************************************************************************/
CMarQDlg::CMarQDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMarQDlg::IDD, pParent)
{

}

/***************************************************************************
*  Function Name  : ~CMarQDlg
*  Description    : Destructor
*  Author Name    : Tejas Shinde
*  Date           : 31 January 2019
****************************************************************************/
CMarQDlg::~CMarQDlg()
{
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name	  : Tejas Shinde
*  Date           : 31 January 2019
****************************************************************************************************/
void CMarQDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_SHOWMARQUEE, m_prgUsbProgressbar);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : hande the Custom Draw Dilaog Marquee with Message Show 
*  Author Name	  : Tejas Shinde
*  Date           : 31 January 2019
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CMarQDlg, CDialog)
	ON_STN_CLICKED(IDC_STATIC_MESSAGE, &CMarQDlg::OnStnClickedStaticMessage)
	ON_NOTIFY(NM_CUSTOMDRAW, IDD_DIALOG_SHOW_MARQUEE, &CMarQDlg::OnNMCustomdrawDialogShowMarquee)
END_MESSAGE_MAP()

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : handle the Custom Static Message Show in Dialog
*  Author Name	  : Tejas Shinde
*  Date           : 31 January 2019
****************************************************************************************************/
void CMarQDlg::OnStnClickedStaticMessage()
{
	
}

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Initializes the dialog window
*  Author Name	  : Tejas Shinde
*  Date           : 31 January 2019
****************************************************************************************************/
BOOL CMarQDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	m_prgUsbProgressbar.SetBarColor(RGB(51, 204, 255));
	m_prgUsbProgressbar.SetMarquee(TRUE, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************************************
* Function Name  : OnNMCustomdrawDialogShowMarquee
* Description    : This Function Used to Draw Custom Dialog With Show Marquee 
* Author Name	 : Tejas Shinde
* Date           : 31 January 2019
/***************************************************************************************************/
void CMarQDlg::OnNMCustomdrawDialogShowMarquee(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	*pResult = 0;
}

/***************************************************************************************************
* Function Name  : PreTranslateMessage
* Description    : Ignore Enter/escape/Alt+F4/shift/ctrl button click events
* Author Name	 : Tejas Shinde
* Date           : 31 January 2019
/***************************************************************************************************/
BOOL CMarQDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_F4 || pMsg->wParam == VK_MENU)
	{
		return TRUE;
	}
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE || pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_TAB))
	{
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}
