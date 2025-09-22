/*******************************************************************************************
*  Program Name: WardWizRestartClient.cpp
*  Description:  It is single popup dialog which popup for Restart Client messages.
*  Author Name    : Nitin Shelar
*  Date           : 19th July 2018
*  Version No:                                                                                                                              *                                                                                                                *
*********************************************************************************************/

#include "stdafx.h"
#include "afxdialogex.h"
#include "WardWizParCtrlPopUp.h"
//#include "WardWizDatabaseInterface.h"

//WardWizRestartClient dialog
//DWORD WINAPI HideBackWindow(LPVOID lpvThreadParam);
IMPLEMENT_DYNAMIC(CWardWizParCtrlPopUp, CDialog)

/***************************************************************************
Function Name  : CWardWizRestartClient
Description    : Constructor
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
CWardWizParCtrlPopUp::CWardWizParCtrlPopUp(CWnd* pParent /*=NULL*/)
: CDialog(CWardWizParCtrlPopUp::IDD, pParent)
{
}

/***************************************************************************
Function Name  : ~CWardWizRestartClient
Description    : Destructor
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
CWardWizParCtrlPopUp::~CWardWizParCtrlPopUp()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
void CWardWizParCtrlPopUp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***************************************************************************
Function Name  : MESSAGE_MAP
Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizParCtrlPopUp, CDialog)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

HWINDOW   CWardWizParCtrlPopUp::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizParCtrlPopUp::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************
Function Name  : OnInitDialog
Description    : Windows calls the OnInitDialog function through the standard global 
				 dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
BOOL CWardWizParCtrlPopUp::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	//SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	//code for painting the background Ends here
	//to set round window

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document IDR_HTM_TRAY_LIVE_UPDATE
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_PARCTRL_POPUP.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_PARCTRL_POPUP.htm");
	this->SetWindowText(L"AKTRAYPWD");
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

	::MoveWindow(this->get_hwnd(), ixRect/2, iyRect/2, pIntMaxWidth, pIntHeight, true);
	try
	{
		APPBARDATA abd;
		abd.cbSize = sizeof(abd);

		SHAppBarMessage(ABM_GETTASKBARPOS, &abd);

		switch (abd.uEdge)
		{
			case ABE_TOP:	    
					SetWindowPos(NULL, ixRect, i, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;
		}
		CString csMsgType;
		csMsgType.Format(L"%d", m_iTrayMsgType);
		m_svGetMsgType.call((SCITER_STRING)csMsgType);
		SetParentWindow();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizRestartClient::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : OnBnClickedOK
Description    : Function to close PopUp
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
void CWardWizParCtrlPopUp::OnBnClickedOK()
{
	try
	{
		OnOK();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizRestartClient::OnBnClickedOK", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : WindowProc
Description    : Windows Function
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
LRESULT CWardWizParCtrlPopUp::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    b_Handled = FALSE;
	__try
	{
		HWND hWnd = this->GetSafeHwnd();
		if (hWnd)
		{
			lResult = SciterProcND(hWnd, message, wParam, lParam, &b_Handled);

			if (b_Handled)      // if it was handled by the Sciter
				return lResult; // then no further processing is required.
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizRestartClient::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
int CWardWizParCtrlPopUp::GetTaskBarHeight()
{
	try
	{
		RECT rect;
		HWND taskBar;
		taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
		if (taskBar && ::GetWindowRect(taskBar, &rect))
		{
			return rect.bottom - rect.top;
		}
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizRestartClient::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
int CWardWizParCtrlPopUp::GetTaskBarWidth()
{
	try
	{
		RECT rect;
		HWND taskBar;
		taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
		if (taskBar && ::GetWindowRect(taskBar, &rect))
		{
			return rect.right - rect.left;
		}
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizRestartClient::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : WindowProc
Description    : Function for PreTranslation of Message
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
BOOL CWardWizParCtrlPopUp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_F4 || pMsg->wParam == VK_MENU)
	{
		return TRUE;
	}
	if (pMsg->message == WM_CHAR || pMsg->message == WM_UNICHAR)
	{
		WindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
	}
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : OnNcHitTest
Description    : The framework calls this member function for the CWnd object that contains
the cursor (or the CWnd object that used the SetCapture member function to
capture the mouse input) every time the mouse is moved.
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
LRESULT CWardWizParCtrlPopUp::OnNcHitTest(CPoint point)
{
	return __super::OnNcHitTest(point);
}

/***************************************************************************
Function Name  : WindowProc
Description    : Function to close PopUp after clicking OK
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
json::value CWardWizParCtrlPopUp::On_ClickOK()
{
	try
	{
		OnBnClickedOK();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizRestartClient::On_ClickOK", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : On_ClickClose
Description    : Function to Used for close the Window.
Author Name    : Nitin Shelar
Date           : 19th July 2018
SR_NO		   :
****************************************************************************/
json::value CWardWizParCtrlPopUp::On_ClickClose()
{
	try
	{
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"#### Exception in CWardwizRestartClient::On_ClickClose", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  OnCheckMsgType
*  Description    :  Function to set Message Type
*  Author Name    :	 Nitin Shelar
*  Date           :  19th July 2018
****************************************************************************************************/
json::value CWardWizParCtrlPopUp::OnCheckMsgType(SCITER_VALUE svGetMsgType)
{
	try
	{
		m_svGetMsgType = svGetMsgType;
	}
	catch (...)
	{
		AddLogEntry(L"#### Exception in CWardwizRestartClient::OnCheckMsgType", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : SetParentWindow
Description    : Function to set wardwiz UI as parent for restart popup
Author Name    : Nitin Shelar
Date           : 19 July 2018
****************************************************************************/
void CWardWizParCtrlPopUp::SetParentWindow()
{
	try
	{
		HWND hWndParent = ::FindWindow(NULL, L"VIBRANIUMUI");
		if (hWndParent)
			::ShowWindow(hWndParent, SW_MINIMIZE);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizRestartClient::SetParentWindow", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CWardWizParCtrlPopUp::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizParCtrlPopUp::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : To Get App Path and Set it in Script
Author Name    : Akshay Patil
Date           : 25th Jan 2019
/***************************************************************************************************/
json::value CWardWizParCtrlPopUp::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizParCtrlPopUp::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Akshay Patil
Date           : 25th Jan 2019
/***************************************************************************************************/
json::value CWardWizParCtrlPopUp::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizParCtrlPopUp::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Akshay Patil
*  Date           : 25th Jan 2019
****************************************************************************************************/
json::value CWardWizParCtrlPopUp::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizParCtrlPopUp::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}