// PopUpWithReminder.cpp : implementation file
/*******************************************************************************************
*  Program Name: PopUpWithReminder.cpp                                                                                                  
*  Description:  reminder popup
*  Author Name:  1)Lalit			
*  Date Of Creation:15 July 2014                                                                                               
*  Version No:    1.0.0.5                                                                                                                                                                               
*  Special Logic Used:                                          
*  Modification Log:                                                                                               
*  1. Modified xyz function in main        Date modified         CSR NO                                                                                                                            
*********************************************************************************************/ 
#include "stdafx.h"
#include "PopUpWithReminder.h"
#include "afxdialogex.h"
#include "ISpyAVTray.h"

// CPopUpWithReminder dialog

IMPLEMENT_DYNAMIC(CPopUpWithReminder, CDialog)
/***************************************************************************************************                    
*  Function Name  : CPopUpWithReminder                                                     
*  Description    : C'tor
*  Author Name    : Lalit
*  SR_NO		  : WRDWIZTRAY_0147
*  Date           : 18 Sep 2013
****************************************************************************************************/
CPopUpWithReminder::CPopUpWithReminder(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CPopUpWithReminder::IDD, pParent)
	, m_dwRemindTime(0)
{


}

/***************************************************************************************************                    
*  Function Name  : ~CPopUpWithReminder                                                     
*  Description    : C'tor
*  Author Name    : Lalit
*  SR_NO		  : WRDWIZTRAY_0148
*  Date           : 18 Sep 2013
****************************************************************************************************/
CPopUpWithReminder::~CPopUpWithReminder()
{
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Exachaning data
*  Author Name    : Lalit
*  SR_NO		  : WRDWIZTRAY_0149
*  Date           : 18 Sep 2013
****************************************************************************************************/
void CPopUpWithReminder::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_BUTTON_REMIND_LATER, m_btnReminder);
	//DDX_Control(pDX, IDC_BUTTON_DOWNLOAD_NOW, m_btnUpdateNow);
	DDX_Control(pDX, IDC_STATIC_HEADER_TEXT_LIVE_REM_UPDATE, m_stLiveUpdateHeader);
	DDX_Control(pDX, IDC_STATIC_REM_UPDATE_TEXT, m_stAutoRemUpdateText);
	DDX_Control(pDX, IDC_STATIC_REM_DOWNLOAD_BTMP, m_bmpRemDownload);

	DDX_Control(pDX, IDC_STATIC_REM_CMB, m_stCmbLable);
	DDX_Control(pDX, IDC_COMBO1, m_CmbForTimeReminder);
	DDX_Control(pDX, IDC_BUTTON_REM_RESTART_NOW, m_btnRestartNow);
	DDX_Control(pDX, IDC_BUTTON_REM_REMIND_LATER, m_btnRestartLater);
	DDX_Control(pDX, IDC_BUTTON1, m_closeBtn);
}

BEGIN_MESSAGE_MAP(CPopUpWithReminder, CJpegDialog)
	//ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_NOW, &CPopUpWithReminder::OnBnClickedButtonDownloadNow)
	//ON_BN_CLICKED(IDC_BUTTON_REMIND_LATER, &CPopUpWithReminder::OnBnClickedButtonRemindLater)
	ON_BN_CLICKED(IDC_BUTTON_REM_RESTART_NOW, &CPopUpWithReminder::OnBnClickedReminderRestartNow)
	//ON_BN_CLICKED(IDC_BUTTON_REM_REMIND_LATER, &CPopUpWithReminder::OnBnClickedReminderRemindLater)

	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	
	ON_BN_CLICKED(IDC_BUTTON1, &CPopUpWithReminder::OnClosingButtonGetClicked)
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

HWINDOW   CPopUpWithReminder::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CPopUpWithReminder::get_resource_instance() { return theApp.m_hInstance; }

// CPopUpWithReminder message handlers
/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Initialization of dialog
*  Author Name    : Lalit
*  SR_NO		  : WRDWIZTRAY_0150
*  Date           : 18 Sep 2013
****************************************************************************************************/
BOOL CPopUpWithReminder::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW ); 
 //	BringWindowToTop();
//	SetForegroundWindow();
	SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_RESTART_POPUP.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_RESTART_POPUP.htm");
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);
	//::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;
	try
	{
		APPBARDATA abd;
		abd.cbSize = sizeof(abd);

		SHAppBarMessage (ABM_GETTASKBARPOS, &abd);
		
		switch (abd.uEdge)
		{
		case ABE_TOP:
			SetWindowPos(NULL, ixRect, i, pIntMinWidth, pIntHeight,  SWP_NOREDRAW);
			break;

		case ABE_BOTTOM:
			SetWindowPos(NULL, ixRect, iyRect + 100, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;

		case ABE_LEFT:
			SetWindowPos(NULL, j, iyRect, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;

		case ABE_RIGHT:
			SetWindowPos(NULL, ixRect, iyRect, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;
        
		} 
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CPopUpWithReminder::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************
  Function Name  : GetTaskBarHeight
  Description    : To get the Task Bar Height
  Author Name    : lalit
  Date           : 18th July 2014
  SRNO			 : WRDWIZTRAY_0151
****************************************************************************/
int CPopUpWithReminder::GetTaskBarHeight() 
{ 
	RECT rect; 
	HWND taskBar ;
	taskBar = ::FindWindow(L"Shell_TrayWnd", NULL); 
	if(taskBar && ::GetWindowRect(taskBar, &rect)) 
	{ 
		return rect.bottom - rect.top; 
	} 
	return 0;
}


/***************************************************************************
  Function Name  : GetTaskBarWidth
  Description    : To get the Task Bar Width
  Author Name    : lalit
  Date           : 18th July 2014
  SRNO			 : WRDWIZTRAY_0152
****************************************************************************/
int CPopUpWithReminder::GetTaskBarWidth() 
{ 
	RECT rect; 
	HWND taskBar ;
	taskBar = ::FindWindow(L"Shell_TrayWnd", NULL); 
	if(taskBar && ::GetWindowRect(taskBar, &rect)) 
	{ 
		return rect.right - rect.left; 
	} 
	return 0;
}


/***************************************************************************
  Function Name  : OnBnClickedButtonDownloadNow
  Description    : On Restart Now Button Click it will restart computer.
  Author Name    : Lalit 
  Date           : 18th July 2014
****************************************************************************/
//void CPopUpWithReminder::OnBnClickedButtonDownloadNow()
//{
//	OnCancel();	
//}

//void CPopUpWithReminder::OnBnClickedButtonRemindLater()
//{
//	int iSelectedReminderType = m_CmbForTimeReminder.GetCurSel();
//	if(iSelectedReminderType == 1)
//	{
//		((CISpyAVTrayDlg*)AfxGetMainWnd())->m_dwTimeReminder = 1;
//	}
//	else if(iSelectedReminderType == 2)
//	{
//		((CISpyAVTrayDlg*)AfxGetMainWnd())->m_dwTimeReminder = 2;
//	}
//	else if(iSelectedReminderType == 3)
//	{
//		((CISpyAVTrayDlg*)AfxGetMainWnd())->m_dwTimeReminder = 3;
//	}
//	OnOK();
//}

/***************************************************************************
  Function Name  : OnNcHitTest
  Description    : The framework calls this member function for the CWnd object that contains 
					the cursor (or the CWnd object that used the SetCapture member function to 
					capture the mouse input) every time the mouse is moved.
  Author Name    : lalit
  Date           : 
  SR_No			 : 
****************************************************************************/
//LRESULT CPopUpWithReminder::OnNcHitTest(CPoint point)
//{
//	return HTCLIENT;
//}

/***************************************************************************
  Function Name  : OnCtlColor
  Description    : The framework calls this member function when a child control is about to be drawn.
  Author Name    : lalit
  Date           : 18th July 2014
  SR_No			 :  WRDWIZTRAY_0153
****************************************************************************/


HBRUSH CPopUpWithReminder::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if(	
			ctrlID ==	IDC_STATIC_HEADER_TEXT_LIVE_REM_UPDATE	||
			ctrlID ==	IDC_STATIC_REM_UPDATE_TEXT ||
			ctrlID ==    IDC_STATIC_REM_CMB
	  )
  
	{
			pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}	
	return hbr;
}

/***************************************************************************
  Function Name  : OnNcHitTest
  Description    : OnNcHitTest 
  Author Name    : lalit
  Date           : 18th July 2014 
  SR_No			 :  WRDWIZTRAY_0154
****************************************************************************/
LRESULT CPopUpWithReminder::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	return HTCLIENT;
}

/***************************************************************************
  Function Name  : OnBnClickedReminderRestartNow
  Description    : Restart Machine
  Author Name    : lalit
  Date           : 18th July 2014
  SR_No			 : WRDWIZTRAY_0155
****************************************************************************/
void CPopUpWithReminder::OnBnClickedReminderRestartNow()
{
	OnOK();
}

/***************************************************************************
  Function Name  : OnBnClickedReminderRemindLater
  Description    : Reminder come after sometime
  Author Name    : lalit
  Date           : 18th July 2014
  SR_No			 : WRDWIZTRAY_0156
****************************************************************************/
void CPopUpWithReminder::OnBnClickedReminderRemindLater(SCITER_VALUE Reminder_Time)
{
	try
	{
		//int iSelectedReminderType = m_CmbForTimeReminder.GetCurSel();
		//CString SelectedReminderType = Reminder_Time.c_str();
		int iReminder_Time = static_cast<int>(Reminder_Time.d);
		//WardWiz Tray Crashes when user clicks on reminder later button
		//This crash is happening some time.

		//Varada Ikhar, Date: 12th May-2015
		//Issue : In Tray, after product update, if for reboot confirmation pop-up, clicked on 'Remind later'. Pop-up don't apper again for remainder.
		if (iReminder_Time == 10)
		{
			m_dwRemindTime = 1;
		}
		else if (iReminder_Time == 1)
		{
			m_dwRemindTime = 2;
		}
		else if (iReminder_Time == 3)
		{
			m_dwRemindTime = 3;
		}

		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpWithReminder::OnBnClickedReminderRemindLater", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : OnClosingButtonGetClicked
  Description    : on closing tray
  Author Name    : lalit
  Date           : 18th July 2014
  SR_No			 : WRDWIZTRAY_0157
****************************************************************************/
void CPopUpWithReminder::OnClosingButtonGetClicked()
{
	//OnBnClickedReminderRemindLater();
}

/***************************************************************************************************
*  Function Name  :	WindowProc
*  Description    :	This callback Procedure is used to Handle All Window Actions
*  Author Name    :	Yogeshwar Rasal
*  Date           : 14 June 2016
****************************************************************************************************/
LRESULT CPopUpWithReminder::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

	LRESULT lResult;
	BOOL    b_Handled = FALSE;
	__try
	{
		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &b_Handled);

		if (b_Handled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CPopUpWithReminder::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CJpegDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  :	OnRebootNow
*  Description    :	Sciter Function to Handle "Restart Now " Button.
*  Author Name    :	Yogeshwar Rasal
*  Date           : 20 June 2016
****************************************************************************************************/
json::value CPopUpWithReminder::OnRebootNow()
{
	try
	{
		OnBnClickedReminderRestartNow();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpWithReminder::OnRebootNow", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  :	OnRebootLater
*  Description    :	Sciter Function to Handle "Restart Later " Button.
*  Author Name    :	Yogeshwar Rasal
*  Date           : 20 June 2016
****************************************************************************************************/
json::value CPopUpWithReminder::OnRebootLater(SCITER_VALUE svReminder_Time)
{
	try
	{ 
		//sciter::string sv_Time = Reminder_Time.get(L"");
		//std::wstring Remind_Time = svReminder_Time.get(L"");	// Remind_Time = sv_Time.c_str;
		OnBnClickedReminderRemindLater(svReminder_Time);
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpWithReminder::OnRebootLater", 0, 0, true, SECONDLEVEL);
	}
	 return json::value(0);
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : Function to get Language ID
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CPopUpWithReminder::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();

		CString csLangID;
		csLangID.Format(L"LangID: %d", theApp.m_dwSelectedLangID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpWithReminder::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 Aug 2016
****************************************************************************************************/
json::value CPopUpWithReminder::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpWithReminder::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CPopUpWithReminder::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpWithReminder::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CPopUpWithReminder::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpWithReminder::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}