// WardWizSplashWindow.cpp : implementation file
//

/*******************************************************************************************
*  Program Name: WardWizSplashWindow.cpp                                                                                                  
*  Description:  Displaying the Splash Screen.
*  Author Name:  1)Rajil
*				 
*
*  Date Of Creation: 7-May 2014                                                                                              
*  Version No:    1.0.0.2                                                                                                        *
*                                                                                                                                      *
*  Special Logic Used:                                                                                                                            *
*                                                                                                                                      *
*  Modification Log:                                                                                               
*  1. Modified xyz function in main        Date modified         CSR NO    *
*  Modification By:                                                                                                                                    *
*                                                                                                                    *
*********************************************************************************************/ 
#include "stdafx.h"
#include "WardWizSplashWindow.h"
#include "ISpyAVTray.h"
#include "WardWizResource.h"
#include "WardwizLangManager.h"
#include "ISpyAVTrayDlg.h"

#define	SHOWPOPUPMESSAGE	5000

// CWardWizSplashWindow dialog

IMPLEMENT_DYNAMIC(CWardWizSplashWindow, CDialog)

/***********************************************************************************************                    
*  Function Name  : CWardWizSplashWindow                                                     
*  Description    : C'tor
*  Author Name    : Rajil                                                                                        *
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0012
*************************************************************************************************/
CWardWizSplashWindow::CWardWizSplashWindow(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWardWizSplashWindow::IDD, pParent)
	,m_stWardWizHyperLink(theApp.m_hResDLL)
	,m_stMarketHyperLink(theApp.m_hResDLL)
	,m_stSubText(theApp.m_hResDLL)
{
	
}

/***********************************************************************************************                    
*  Function Name  : ~CWardWizSplashWindow                                                     
*  Description    : D'tor
*  Author Name    : Rajil                                                                                        *
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0013
*************************************************************************************************/
CWardWizSplashWindow::~CWardWizSplashWindow()
{
	
}

/***********************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Attaching variable to their ID's
*  Author Name    : Rajil                                                                                        *
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0014
*************************************************************************************************/
void CWardWizSplashWindow::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_COPYRIGHT, m_stCopyRightText);
	DDX_Control(pDX, IDC_STATIC_SUB_TEXT, m_stSubText);
	DDX_Control(pDX, IDC_STATIC_SPLASH_IMG, m_stSplasImg);
	DDX_Control(pDX, IDC_STATIC_HYPRLINK_MARKET, m_stMarketHyperLink);
	DDX_Control(pDX, IDC_STATIC_HYPERLINK_WARDWIZ, m_stWardWizHyperLink);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_stSplashHeader);
}

/***********************************************************************************************                    
*  Function Name  : BEGIN_MESSAGE_MAP                                                     
*  Description    : Message Map
*  Author Name    : Rajil                                                                                        *
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0015
*************************************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizSplashWindow, CJpegDialog)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_NCHITTEST()
	ON_STN_CLICKED(IDC_STATIC_SUB_TEXT, &CWardWizSplashWindow::OnStnClickedStaticSubText)
	ON_STN_CLICKED(IDC_STATIC_HYPRLINK_MARKET, &CWardWizSplashWindow::OnStnClickedStaticHyprlinkMarket)
	ON_STN_CLICKED(IDC_STATIC_HYPERLINK_WARDWIZ, &CWardWizSplashWindow::OnStnClickedStaticHyperlinkWardwiz)
	ON_MESSAGE(_HYPERLINK_EVENT,OnChildFire)
END_MESSAGE_MAP()


// CWardWizSplashWindow message handlers
/***********************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Initializing the variables
*  Author Name    : Rajil                                                                                        *
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0016
*************************************************************************************************/
BOOL CWardWizSplashWindow::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);

	if (theApp.m_dwSelectedLangID != 2)
	{
		if (!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SPLASH), _T("JPG")))
		{
			AddLogEntry(L"### Failed to load IDR_JPG_TRAY for English In CWardwizCommonTrayDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
		}
	}
	else if (theApp.m_dwSelectedLangID == 2)
	{
		if (!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SPLASH_GERMAN), _T("JPG")))
		{
			AddLogEntry(L"### Failed to load IDR_JPG_TRAY for German In CWardwizCommonTrayDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
		}
	}
	Draw();

	this->GetClientRect(m_rect);
	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	m_stSubText.ShowWindow(SW_HIDE);
	m_stSplasImg.ShowWindow(SW_HIDE);
	m_stWardWizHyperLink.ShowWindow(SW_HIDE);
	m_stMarketHyperLink.ShowWindow(SW_HIDE);
	
	SetTimer(SHOWPOPUPMESSAGE, 5000 , NULL); 
	return TRUE;  // return TRUE unless you set the focus to a control
}

/***********************************************************************************************                    
*  Function Name  : OnTimer                                                     
*  Description    : Setting the timer Event
*  Author Name    : Rajil                                                                                        *
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0017
*************************************************************************************************/
void CWardWizSplashWindow::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == SHOWPOPUPMESSAGE)
	{
		KillTimer(SHOWPOPUPMESSAGE);
		EndDialog(IDOK);
	}
	CJpegDialog::OnTimer(nIDEvent);
}

/***********************************************************************************************                    
*  Function Name  : OnCtlColor                                                     
*  Description    : Transparent the obj.
*  Author Name    : Rajil
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0018
*************************************************************************************************/
HBRUSH CWardWizSplashWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if (ctrlID == IDC_STATIC_COPYRIGHT || ctrlID == IDC_STATIC_SUB_TEXT || ctrlID == IDC_STATIC_HYPRLINK_MARKET || ctrlID == IDC_STATIC_HYPERLINK_WARDWIZ || ctrlID == IDC_STATIC_HEADER)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

/***********************************************************************************************                    
*  Function Name  : OnSetCursor                                                     
*  Description    : The framework calls this member function if mouse input is not captured and 
				    the mouse causes cursor movementwithin the CWnd object.
*  Author Name    : Rajil
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0019
*************************************************************************************************/
BOOL CWardWizSplashWindow::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( iCtrlID == IDC_STATIC_SUB_TEXT || 
		iCtrlID == IDC_STATIC_HYPRLINK_MARKET || 
		iCtrlID == IDC_STATIC_HYPERLINK_WARDWIZ)
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

/***********************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : To translate window messages before they are dispatched to the TranslateMessage 
				    and DispatchMessage Windows functions.
*  Author Name    : Rajil
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0020
*************************************************************************************************/
BOOL CWardWizSplashWindow::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***********************************************************************************************                    
*  Function Name  : OnNcHitTest                                                     
*  Description    : The framework calls this member function for the CWnd object that contains the cursor 
					(or the CWnd object that used the SetCapture member function to capture the mouse input)every time the mouse is moved.
*  Author Name    : Rajil
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0021
*************************************************************************************************/
LRESULT CWardWizSplashWindow::OnNcHitTest(CPoint point)
{
	return HTNOWHERE;
}

/***********************************************************************************************                    
*  Function Name  : OnStnClickedStaticSubText                                                     
*  Description    : on click of static text.
*  Author Name    : Rajil
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0022
*************************************************************************************************/
void CWardWizSplashWindow::OnStnClickedStaticSubText()
{
	/*ShellExecute(NULL,L"open",L"http://wardwiz.com/index.php?route=information/information&information_id=18",NULL,NULL,SW_SHOW);*/
	
}

/***********************************************************************************************                    
*  Function Name  : OnStnClickedStaticHyprlinkMarket                                                     
*  Description    : on click of hyperlink text
*  Author Name    : Rajil
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0023
*************************************************************************************************/
void CWardWizSplashWindow::OnStnClickedStaticHyprlinkMarket()
{
	/*ShellExecute(NULL,L"open",L"https://www.facebook.com/pages/Itinav/495433037229494?ref=hl",NULL,NULL,SW_SHOW);*/

}

/***********************************************************************************************                    
*  Function Name  : OnStnClickedStaticHyperlinkWardwiz                                                     
*  Description    : on click of another hyperlink text
*  Author Name    : Rajil
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0024
*************************************************************************************************/
void CWardWizSplashWindow::OnStnClickedStaticHyperlinkWardwiz()
{
	
}

/***********************************************************************************************                    
*  Function Name  : OnChildFire                                                     
*  Description    : When user click on forgot password hyperlink static.WM_COMMAND calls OnChildFire Function.
*  Author Name    : Rajil
*  Date           : 7-May-2014
*  SR NO		  : SR.NO WRDWIZTRAY_0025
*************************************************************************************************/
LRESULT CWardWizSplashWindow::OnChildFire(WPARAM wparam,LPARAM lparam)
{
	if(wparam == IDC_STATIC_HYPERLINK_WARDWIZ)
	{
		ShellExecute(NULL,L"open",L"https://vibranium.co.in/",NULL,NULL,SW_SHOW);
		//http://wardwiz.com/index.php
	}
	if(wparam == IDC_STATIC_HYPRLINK_MARKET)
	{
		CString str1("open"),str2("mailto:info@vibranium.co.in ? subject= Buy Vibranium Antivirus ");
		//marketing@wardwiz.com
		ShellExecute(NULL, str1, str2, NULL , NULL, SW_SHOW);
	}
	if(wparam == IDC_STATIC_SUB_TEXT)
	{
		CString str1("open"),str2("mailto:support@vibranium.co.in ? subject= Technical Support ");
		//support@wardwiz.com
		ShellExecute(NULL, str1, str2, NULL , NULL, SW_SHOW);
	}

	return 0;
}