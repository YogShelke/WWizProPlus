// SettingsEmailDlg.cpp : implementation file
/****************************************************
*  Program Name: SettingsEmailDlg.cpp                                                                                                    
*  Description: setting for enable/disable an Email scan. 
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 29 April 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "SettingsEmailDlg.h"
#include "JpegDialog.h"
#include "ISpyGUI.h"


// CSettingsEmailDlg dialog

IMPLEMENT_DYNAMIC(CSettingsEmailDlg, CDialog)

/***************************************************************************************************                    
*  Function Name  : CSettingsEmailDlg                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
CSettingsEmailDlg::CSettingsEmailDlg(CWnd* pParent /*=NULL*/)
	:CJpegDialog(CSettingsEmailDlg::IDD, pParent)
{

}

/***************************************************************************************************                    
*  Function Name  : ~CSettingsEmailDlg                                                     
*  Description    : D'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
CSettingsEmailDlg::~CSettingsEmailDlg()
{
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
void CSettingsEmailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CSettingsEmailDlg, CJpegDialog)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


// CSettingsEmailDlg message handlers
/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft Foundation Class Library dialog boxes
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
BOOL CSettingsEmailDlg::OnInitDialog()
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
	
	m_list.Create(this,CRect(rect1.left +9,6,481,260),ID_SETTING_EMAIL);
//	m_list.Create(this,CRect(rect1.left +5,35,33,35),123);
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
*  Description    : The framework calls this member function for the CWnd object that contains the cursor (or the CWnd object that used the SetCapture member function to capture the mouse input) every time the mouse is moved.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
LRESULT CSettingsEmailDlg::OnNcHitTest(CPoint point)
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
BOOL CSettingsEmailDlg :: RefreshString()
{
	//NOTE: In this Function we are also Creating the instances for the HTMLListCtrl in m_list.InsertItem() call

	
	CString m_settingTabOptions;
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_MAIL_EMAIL_SCAN"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
/*	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_MAIL_VIRUS_SCAN"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_MAIL_SPAM_FILTER"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_MAIL_CONTENT_FILTER"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_MAIL_SIGNATURE"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
*/	
	return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : setDefaultSettings                                                     
*  Description    : Set default setting.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/

void CSettingsEmailDlg :: setDefaultSettings()
{
	DWORD dwProductID = theApp.m_dwProductID;
	if(dwProductID == PRO || dwProductID == ELITE)
	{
		m_list.SendCheckStateChangedNotification(0);
	}
}

/***************************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
BOOL CSettingsEmailDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}