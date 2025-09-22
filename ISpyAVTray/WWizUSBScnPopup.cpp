/*******************************************************************************************
*  Program Name   : CWWizUSBScnPopup.cpp
*  Description    : This is the popup Dialog that is displayed on automastic scanning of USB Drive
*  Author Name    : Jeena Mariam Saji
*  Date           : 02th July 2019
*  Version No:                                                                                                                              *                                                                                                                *
*********************************************************************************************/

#include "stdafx.h"
#include "afxdialogex.h"
#include "WWizUSBScnPopup.h"

//CWWizUSBScnPopup dialog
IMPLEMENT_DYNAMIC(CWWizUSBScnPopup, CDialog)

/***************************************************************************
Function Name  : CWWizUSBScnPopup
Description    : Constructor
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
CWWizUSBScnPopup::CWWizUSBScnPopup(CWnd* pParent /*=NULL*/)
: CDialog(CWWizUSBScnPopup::IDD, pParent)
{
}

/***************************************************************************
Function Name  : ~CWWizUSBScnPopup
Description    : Destructor
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
CWWizUSBScnPopup::~CWWizUSBScnPopup()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
void CWWizUSBScnPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***************************************************************************
Function Name  : MESSAGE_MAP
Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
BEGIN_MESSAGE_MAP(CWWizUSBScnPopup, CDialog)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

HWINDOW   CWWizUSBScnPopup::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWWizUSBScnPopup::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************
Function Name  : OnInitDialog
Description    : Windows calls the OnInitDialog function through the standard global
dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
BOOL CWWizUSBScnPopup::OnInitDialog()
{
	CDialog::OnInitDialog();

	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_USB_POPUP.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_USB_POPUP.htm");
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
		this->SetWindowText(L"VBUSBPOPUP");
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizUSBScnPopup::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : WindowProc
Description    : Windows Function
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
LRESULT CWWizUSBScnPopup::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
		AddLogEntry(L"### Excpetion in CWardwizUSBScnPopup::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
int CWWizUSBScnPopup::GetTaskBarHeight()
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
		AddLogEntry(L"### Excpetion in CWardwizUSBScnPopup::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
int CWWizUSBScnPopup::GetTaskBarWidth()
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
		AddLogEntry(L"### Excpetion in CWardwizUSBScnPopup::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : WindowProc
Description    : Function for PreTranslation of Message
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
BOOL CWWizUSBScnPopup::PreTranslateMessage(MSG* pMsg)
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
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
SR_NO		   :
****************************************************************************/
LRESULT CWWizUSBScnPopup::OnNcHitTest(CPoint point)
{
	return __super::OnNcHitTest(point);
}

/***************************************************************************************************
*  Function Name  :  OnCheckMsgType
*  Description    :  Function to set Message Type
*  Author Name    :  Jeena Mariam Saji
*  Date           :  02th July 2019
****************************************************************************************************/
json::value CWWizUSBScnPopup::OnCheckMsgType()
{
	try
	{
		CString csMsgType;
		csMsgType.Format(L"%d", m_iTrayMsgType);
		return (SCITER_STRING)csMsgType;
	}
	catch (...)
	{
		AddLogEntry(L"#### Exception in CWardwizUSBScnPopup::OnCheckMsgType", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  OnGetDriveName
*  Description    :  Function to get Drive Name
*  Author Name    :	 Jeena Mariam Saji
*  Date           :  28th June 2019
****************************************************************************************************/
json::value CWWizUSBScnPopup::OnGetDriveName()
{
	try
	{
		csDriveName = L" " + csDriveName;
		return (SCITER_STRING)csDriveName;
	}
	catch (...)
	{
		AddLogEntry(L"#### Exception in CWardwizUSBScnPopup::OnCheckMsgType", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 02th July 2019
/***************************************************************************************************/
json::value CWWizUSBScnPopup::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUSBScnPopup::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : To Get App Path and Set it in Script
Author Name    :  Jeena Mariam Saji
Date           :  02th July 2019
/***************************************************************************************************/
json::value CWWizUSBScnPopup::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUSBScnPopup::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    :  Jeena Mariam Saji
Date           :  02th July 2019
/***************************************************************************************************/
json::value CWWizUSBScnPopup::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUSBScnPopup::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    :  Jeena Mariam Saji
*  Date           :  02th July 2019
****************************************************************************************************/
json::value CWWizUSBScnPopup::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUSBScnPopup::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}