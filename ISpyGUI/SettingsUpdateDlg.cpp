// SettingsUpdateDlg.cpp : implementation file
/****************************************************
*  Program Name: SettingUpdateDlg.cpp                                                                                                    
*  Description: Display Update setting
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 19 May 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "SettingsUpdateDlg.h"


// CSettingsUpdateDlg dialog

IMPLEMENT_DYNAMIC(CSettingsUpdateDlg, CDialog)
/***********************************************************************************************
*  Function Name  : CSettingsUpdateDlg
*  Description    : C'tor
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 19 May 2014
***********************************************************************************************/
CSettingsUpdateDlg::CSettingsUpdateDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CSettingsUpdateDlg::IDD, pParent)
{

}

/***********************************************************************************************
*  Function Name  : ~CSettingsUpdateDlg
*  Description    : D'tor
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 19 May 2014
***********************************************************************************************/
CSettingsUpdateDlg::~CSettingsUpdateDlg()
{
}

/***********************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 19 May 2014
***********************************************************************************************/
void CSettingsUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***********************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 19 May 2014
***********************************************************************************************/
BEGIN_MESSAGE_MAP(CSettingsUpdateDlg, CJpegDialog)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

/***********************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 19 May 2014
***********************************************************************************************/
// CSettingsUpdateDlg message handlers
BOOL CSettingsUpdateDlg::OnInitDialog()
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
	
	m_list.Create(this,CRect(rect1.left +9,6,481,260),ID_SETTING_UPDATE);
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

/***********************************************************************************************
*  Function Name  : OnNcHitTest
*  Description    : The framework calls this member function for the CWnd object that contains the cursor every time the mouse is moved.
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 19 May 2014
***********************************************************************************************/
LRESULT CSettingsUpdateDlg::OnNcHitTest(CPoint point)
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
BOOL CSettingsUpdateDlg :: RefreshString()
{
	//NOTE: In this Function we are also Creating the instances for the HTMLListCtrl in m_list.InsertItem() call

	try
	{
		CString m_settingTabOptions;
		m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_UPDATE_SMART_DEF"));
		m_list.InsertItem(m_settingTabOptions, 0, HTML_TEXT);
		/* its commented due to resloving coflict between UI  and system Tray  and  "Auto Product Update " menu from setting  tab is also remove  */

		//m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_UPDATE_AUTO_UPDATE"));
		//m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);

		//Varada Ikhar, Date: 18/04/2015
		// Issue:1.Go to WardWiz Settings. 2.Open Update in Settings. 3.2nd setting "Apply Update on StartUp" setting should be removed.
		//Commented the below 2 statement.
		//m_settingTabOptions.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_HTMLLISTCNTRL_UPDATE_APPLY_UPDATE"));
		//m_list.InsertItem(m_settingTabOptions,0,HTML_TEXT);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSettingsUpdateDlg::RefreshString", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : setDefaultSettings
*  Description    : this function is  called for Default Setting options
*  Author Name    : Nitin Kolapkar
*  SR_NO			 :
*  Date           : 19 May 2014
***********************************************************************************************/

void CSettingsUpdateDlg :: setDefaultSettings()
{
	try
	{
		if (theApp.m_dwProductID == ESSENTIAL || theApp.m_dwProductID == PRO || theApp.m_dwProductID == ELITE || theApp.m_dwProductID == BASIC)
		{
			m_list.SendCheckStateChangedNotification(0);
			/* its commented due to resloving conflict between UI  and system Tray  and  "Auto Product Update " menu from setting  tab is also remove  */

			//	m_list.SendCheckStateChangedNotification(1);

			//Varada Ikhar, Date: 18/04/2015
			// Issue:1.Go to WardWiz Settings. 2.Open Update in Settings. 3.2nd setting "Apply Update on StartUp" setting should be removed.
			//Commented the below statement.
			//m_list.SendCheckStateChangedNotification(1);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSettingsUpdateDlg :: setDefaultSettings", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : PreTranslateMessage
*  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Nitin Kolapkar
*  SR_NO			 :
*  Date           : 19 May 2014
***********************************************************************************************/
BOOL CSettingsUpdateDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}