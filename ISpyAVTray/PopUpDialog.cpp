// PopUpDialog.cpp : implementation file
//
/*******************************************************************************************
*  Program Name: PopUpDialog.cpp                                                                                                  
*  Description:  It is single popup dialog which popup for multiple messages.
*  Author Name:  1)Nitin Kolpkar
*
*  Date Of Creation: 24th July 2014                                                                                              
*  Version No: 1.0.0.4                                                                                                                                 *                                                                                                                *
*********************************************************************************************/ 

#include "stdafx.h"
#include "PopUpDialog.h"


// CPopUpDialog dialog

IMPLEMENT_DYNAMIC(CPopUpDialog, CDialog)

/***************************************************************************
  Function Name  : CPopUpDialog
  Description    : Constructor
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0042
****************************************************************************/
CPopUpDialog::CPopUpDialog(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CPopUpDialog::IDD, pParent)
{

}

/***************************************************************************
  Function Name  : ~CPopUpDialog
  Description    : Destructor
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0043
****************************************************************************/
CPopUpDialog::~CPopUpDialog()
{
}

/***************************************************************************
  Function Name  : DoDataExchange
  Description    : Called by the framework to exchange and validate dialog data.
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0044
****************************************************************************/
void CPopUpDialog::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_UPDATE_TEXT, m_stAutoUpdateText);
	DDX_Control(pDX, IDC_BUTTON_DOWNLOAD_NOW, m_btnDownloadNow);
	DDX_Control(pDX, IDC_BUTTON_REMIND_LATER, m_btnRemindLater);
	DDX_Control(pDX, IDC_STATIC_DOWNLOAD_BTMP, m_bmpDownload);
	DDX_Control(pDX, IDC_STATIC_HEADER_TEXT_LIVE_UPDATE, m_stWWUpdate);
	DDX_Control(pDX, IDC_BUTTON_CLOSING, m_btnClosing);
}

/***************************************************************************
  Function Name  : MESSAGE_MAP
  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0045
****************************************************************************/
BEGIN_MESSAGE_MAP(CPopUpDialog, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_NOW, &CPopUpDialog::OnBnClickedButtonDownloadNow)
	ON_BN_CLICKED(IDC_BUTTON_REMIND_LATER, &CPopUpDialog::OnBnClickedButtonRemindLater)
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_CLOSING, &CPopUpDialog::OnClosingGetCalled)
END_MESSAGE_MAP()

HWINDOW   CPopUpDialog::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CPopUpDialog::get_resource_instance() { return theApp.m_hInstance; }

// CPopUpDialog message handlers
/***************************************************************************
  Function Name  : OnInitDialog
  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0046
****************************************************************************/
BOOL CPopUpDialog::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW); 
//	BringWindowToTop();
//	SetForegroundWindow();
	SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	
	HideAllElements();

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_LIVE_UPDATE.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_LIVE_UPDATE.htm");
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;
	
	//CRect rect;
	//this->GetClientRect(rect);
	
	try
	{
		APPBARDATA abd;
		abd.cbSize = sizeof(abd);
		SHAppBarMessage (ABM_GETTASKBARPOS, &abd);

		switch (abd.uEdge)
		{
		case ABE_TOP:
			SetWindowPos(NULL, ixRect, i, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
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
		AddLogEntry(L"### Excpetion in CPopUpDialog::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}	
	return TRUE;  	
}

/***************************************************************************
Function Name  : HideAllElements
Description    : Function to hide old controls
Author Name    : Jeena Mariam Saji
Date           : 16th June 2016
SR_NO		   : 
****************************************************************************/
void CPopUpDialog::HideAllElements()
{
	m_stAutoUpdateText.ShowWindow(SW_HIDE);
	m_btnDownloadNow.ShowWindow(SW_HIDE);
	m_btnRemindLater.ShowWindow(SW_HIDE);
	m_bmpDownload.ShowWindow(SW_HIDE);
	m_stWWUpdate.ShowWindow(SW_HIDE);
	m_btnClosing.ShowWindow(SW_HIDE);	
}

/***************************************************************************
  Function Name  : GetTaskBarHeight
  Description    : To get the Task Bar Height
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0047
****************************************************************************/
int CPopUpDialog::GetTaskBarHeight() 
{
	try
	{
		RECT rect;
		HWND taskBar;
		taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
		if(taskBar && ::GetWindowRect(taskBar, &rect))
		{
			return rect.bottom - rect.top;
		}
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CPopUpDialog::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : GetTaskBarWidth
  Description    : To get the Task Bar Width
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0048
****************************************************************************/
int CPopUpDialog::GetTaskBarWidth() 
{ 
	try
	{
		RECT rect;
		HWND taskBar;
		taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
		if(taskBar && ::GetWindowRect(taskBar, &rect))
		{
			return rect.right - rect.left;
		}
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CPopUpDialog::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : OnBnClickedButtonDownloadNow
  Description    : On Update Now Button Click it will open the UI and start downloading
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0049
****************************************************************************/
void CPopUpDialog::OnBnClickedButtonDownloadNow()
{
	try
	{
		HWND hWindow = ::FindWindow(NULL, L"VibraniumUPDATE");
		if (hWindow)
		{
			::ShowWindow(hWindow,SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			/*::PostMessage(hWindow,CHECKLIVEUPDATE,0,0);*/
			SendData2UI(CHECKLIVEUPDATETRAY, false);
		}
		else// If UI is not open then it will make shellexecute() call
		{
			CString csUIFilePath;
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (!GetModulePath(szModulePath, MAX_PATH))
			{

			}
			csUIFilePath = szModulePath;
			csUIFilePath += L"\\VBUPDATE.EXE";

			ShellExecute(NULL, L"open", csUIFilePath, L"-LIVEUPDATE", NULL, SW_SHOW);
		}		
	}
	catch (...)
	{
		AddLogEntry(L"### Error in CPopUpDialog::OnBnClickedButtonDownloadNow", 0, 0, true, SECONDLEVEL);
	}
	OnCancel();
}

/***************************************************************************
  Function Name  : OnBnClickedButtonRemindLater
  Description    : reminder action call
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0050
****************************************************************************/
void CPopUpDialog::OnBnClickedButtonRemindLater()
{
	try
	{
		OnCancel();
	}

	catch (...)
	{
		AddLogEntry(L"### Excpetion in CPopUpDialog::OnBnClickedButtonRemindLater", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : OnNcHitTest
  Description    : The framework calls this member function for the CWnd object that contains 
				   the cursor (or the CWnd object that used the SetCapture member function to 
				   capture the mouse input) every time the mouse is moved.
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0051
****************************************************************************/
LRESULT CPopUpDialog::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/***************************************************************************
  Function Name  : OnCtlColor
  Description    : The framework calls this member function when a child control is about to be drawn.
  Author Name    : Nitin K
  Date           : 24th July 2014
  SR_NO			 : RDWIZTRAY_0052
****************************************************************************/
HBRUSH CPopUpDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if(	
			ctrlID ==	IDC_STATIC_UPDATE_TEXT			||
			ctrlID ==	IDC_STATIC_HEADER_TEXT_LIVE_UPDATE	
	  )
  
	{
			pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}	
	return hbr;
}

/***************************************************************************
Function Name  : OnClosingGetCalled
Description    : Function to close
Author Name    : Jeena Mariam Saji
Date           : 16th June 2016
SR_NO		   :
****************************************************************************/
void CPopUpDialog::OnClosingGetCalled()
{
	try
	{
		OnCancel();
	}

	catch (...)
	{
		AddLogEntry(L"### Excpetion in CPopUpDialog::OnClosingGetCalled", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : Function for PreTranslation of Message
Author Name    : Jeena Mariam Saji
Date           : 16th June 2016
SR_NO		   :
****************************************************************************/
BOOL CPopUpDialog::PreTranslateMessage(MSG* pMsg)
{

	if( pMsg->wParam == VK_ESCAPE )
		return TRUE;

	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***********************************************************************************************
Function Name  : SendData2UI
Description    : Send Data to UI
Author Name    : Nitin Kolapkar
Date           : 25th June 2014
SR_NO		   : WRDWIZTRAY_0146
***********************************************************************************************/
bool CPopUpDialog::SendData2UI(int iMessageInfo, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = iMessageInfo;

		CISpyCommunicator objComUI(UI_SERVER, true);
		if (!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CSystemTray::SendData2UI", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Error in CPopUpDialog::SendData2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : WindowProc
Description    : Windows Function
Author Name    : Jeena Mariam Saji
Date           : 16th June 2016
SR_NO		   :
****************************************************************************/
LRESULT CPopUpDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
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
		AddLogEntry(L"### Excpetion in CPopUpDialog::WindowProc", 0, 0, true, SECONDLEVEL);
	}

	return CJpegDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************
Function Name  : On_ClickUpdate
Description    : Function to Download Setup 
Author Name    : Jeena Mariam Saji
Date           : 16th June 2016
SR_NO		   :
****************************************************************************/
json::value CPopUpDialog::On_ClickUpdate()
{
	try
	{
		OnBnClickedButtonDownloadNow();
	}

	catch (...)
	{
		AddLogEntry(L"### Excpetion in CPopUpDialog::On_ClickUpdate", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : On_ClickRemindLater
Description    : Function to Remind about the Updates Later
Author Name    : Jeena Mariam Saji
Date           : 16th June 2016
SR_NO		   :
****************************************************************************/
json::value CPopUpDialog::On_ClickRemindLater()
{
	try
	{
		OnBnClickedButtonRemindLater();
	}

	catch (...)
	{
		AddLogEntry(L"### Excpetion in CPopUpDialog::On_ClickRemindLater", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : Function to get Language ID
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CPopUpDialog::On_GetLanguageID()
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
		AddLogEntry(L"### Exception in CPopUpDialog::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 Aug 2016
****************************************************************************************************/
json::value CPopUpDialog::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpDialog::On_GetProductID", 0, 0, true, SECONDLEVEL);
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
json::value CPopUpDialog::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpDialog::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CPopUpDialog::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpDialog::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
