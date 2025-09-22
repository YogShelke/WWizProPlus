// SettingsGeneral.cpp : implementation file
/****************************************************
*  Program Name: SettingsGeneral.cpp                                                                                                    
*  Description: Display general setting dialog 
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 29 April 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "resource.h"
#include "SettingsGeneralDlg.h"



// CSettingsGeneralDlg dialog

IMPLEMENT_DYNAMIC(CSettingsGeneralDlg, CDialog)
/***************************************************************************************************                    
*  Function Name  : CSettingsGeneralDlg                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
CSettingsGeneralDlg::CSettingsGeneralDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CSettingsGeneralDlg::IDD, pParent)
{

}

/***************************************************************************************************                    
*  Function Name  : ~CSettingsGeneralDlg                                                     
*  Description    : D'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
CSettingsGeneralDlg::~CSettingsGeneralDlg()
{
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
void CSettingsGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
}


/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CSettingsGeneralDlg, CJpegDialog)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


// CSettingsGeneralDlg message handlers
/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box
					procedure common to all Microsoft  Foundation Class Library dialog boxes
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
BOOL CSettingsGeneralDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();


	//	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SETTINGS_INNER_BG), _T("JPG")))
	{
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
	}

	Draw();

	CRect rect1;
	this->GetClientRect(rect1);
	CRgn		 rgn;
	rgn.CreateRoundRectRgn(rect1.left-1, rect1.top, rect1.right-2, rect1.bottom-11,0,0);
	this->SetWindowRgn(rgn, TRUE);
	//SetWindowPos(NULL, 1, 88, rect1.Width()-5, rect1.Height() - 5, SWP_NOREDRAW);

	m_ImageList.Create(16,16,ILC_COLOR24|ILC_MASK,4,4);
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ARROW));
	
	m_list.Create(this,CRect(rect1.left +9,6,481,260),ID_SETTING_GENERAL);

	m_list.SetImageList(&m_ImageList);
	m_list.SetExtendedStyle(HTMLLIST_STYLE_GRIDLINES|HTMLLIST_STYLE_CHECKBOX|HTMLLIST_STYLE_IMAGES);
	
	m_bCheckBoxes = TRUE;
	m_bGridLines = TRUE;
	m_bShowImages = TRUE;

	if(!RefreshString())
	{
		AddLogEntry(L">>> CSettingsGeneralDlg::OnInitDialog :: Failed RefreshString()", 0, 0, true, FIRSTLEVEL);
	}
	
	return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : OnNcHitTest                                                     
*  Description    : The framework calls this member function for the CWnd object that contains the cursor every time the mouse is moved.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO		  :
*  Date           : 29 April 2014
****************************************************************************************************/
LRESULT CSettingsGeneralDlg::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	return HTCLIENT;
}
/***************************************************************************************************                    
*  Function Name  : RefreshString                                                     
*  Description    : this function is  called for Language support
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
BOOL CSettingsGeneralDlg :: RefreshString()
{
	CString m_settingTabOptions;
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_GENERAL_TRAY_NOTIFICATION"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_GENERAL_TRAY_DELETE_REPORTS"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_SHOW_TOOLTIP"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_GENERAL_CHANGE_PASSWORD"));
	if(theApp.m_dwProductID == ESSENTIAL)
	{
		m_settingTabOptions = L"";
		m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_GENERAL_CHANGE_PASSWORD_DATAENCRYPTION"));
	}

	// password change option not required in any of product edition
	// resolved by 14-9-2015
	// we are not allowing password change option for pro setup for data encryption.
	// resolved by lalit kumawaat 7-21-2015
	//if (theApp.m_dwProductID != BASIC && theApp.m_dwProductID != PRO)
	//{
	//	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);	
	//}
	return TRUE;
}	

/***************************************************************************************************                    
*  Function Name  : setDefaultSettings                                                     
*  Description    : Set default setting
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
void CSettingsGeneralDlg :: setDefaultSettings()
{
	m_list.SendCheckStateChangedNotification(0);
	m_list.SendCheckStateChangedNotification(1);
	/*	ISSUE NO - 450	In setting tab, in General tab and Scan tab, default button is not working for last two options.
	NAME - NITIN K. TIME - 26st May 2014 */
	m_list.SendCheckStateChangedNotification(2);
	m_list.SendCheckStateChangedNotification(3);

}

/***************************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
BOOL CSettingsGeneralDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}