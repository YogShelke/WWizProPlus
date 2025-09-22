// SettingsScanDlg.cpp : implementation file
/****************************************************
*  Program Name: SettingsScanDlg.cpp                                                                                                    
*  Description: Display dialog to select scan type 
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 29 April 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "SettingsScanDlg.h"


// CSettingsScanDlg dialog

IMPLEMENT_DYNAMIC(CSettingsScanDlg, CDialog)
/***************************************************************************************************                    
*  Function Name  : SettingsScanDlg                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
CSettingsScanDlg::CSettingsScanDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CSettingsScanDlg::IDD, pParent)
{

}

/***************************************************************************************************                    
*  Function Name  : ~SettingsScanDlg                                                     
*  Description    : D'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
CSettingsScanDlg::~CSettingsScanDlg()
{
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
void CSettingsScanDlg::DoDataExchange(CDataExchange* pDX)
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
BEGIN_MESSAGE_MAP(CSettingsScanDlg, CJpegDialog)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft Foundation Class Library dialog boxes
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
// CSettingsScanDlg message handlers
BOOL CSettingsScanDlg::OnInitDialog()
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
	
	m_list.Create(this,CRect(rect1.left +9,6,481,260),ID_SETTING_SCAN);
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
LRESULT CSettingsScanDlg::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	return HTCLIENT;
}

/***********************************************************************************************
*  Function Name  : RefreshString
*  Description    : this function is  called for Language support
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 29 April 2014
***********************************************************************************************/
BOOL CSettingsScanDlg :: RefreshString()
{
	//NOTE: In this Function we are also Creating the instances for the HTMLListCtrl in m_list.InsertItem() call

	
	CString m_settingTabOptions;
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_SCAN_USB"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_SCAN_STARTUP"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_ENABLE_SOUND"));
	m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	
	
	return TRUE;
}
/***********************************************************************************************
*  Function Name  : setDefaultSettings
*  Description    : this function is  called for Default Setting options
*  SR_NO		  :
*  Author Name    : Nitin Kolapkar
*  Date           : 29 April 2014
***********************************************************************************************/

void CSettingsScanDlg :: setDefaultSettings()
{
	m_list.SendCheckStateChangedNotification(0);
	/*	ISSUE NO - 450	In setting tab, in General tab and Scan tab, default button is not working for last two options.
	NAME - NITIN K. TIME - 26st May 2014 */
	m_list.SendCheckStateChangedNotification(1);
	m_list.SendCheckStateChangedNotification(2);
}

/***************************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 29 April 2014
****************************************************************************************************/
BOOL CSettingsScanDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}