// WardWizFirewallBlockPopUp.cpp : implementation file
//
/*******************************************************************************************
*  Program Name: WardWizFirewallBlockPopUp.cpp
*  Description:  It is single popup dialog which popup for multiple messages.
*  Author Name:  1)Kunal Waghmare
*
*  Date Of Creation: 10th July 2018
*  Version No:                                                                                                                              *                                                                                                                *
*********************************************************************************************/

#include "stdafx.h"
#include "afxdialogex.h"
#include "WardWizFirewallBlockPopUp.h"

// CWardWizFirewallBlockPopUp dialog
#define SET_EVENT_CODE_BLK (FIRST_APPLICATION_EVENT_CODE + 15)

IMPLEMENT_DYNAMIC(CWardWizFirewallBlockPopUp, CDialog)


/***************************************************************************
Function Name  : CWardWizFirewallBlockPopUp
Description    : Constructor
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
CWardWizFirewallBlockPopUp::CWardWizFirewallBlockPopUp(CWnd* pParent)
: CDialog(CWardWizFirewallBlockPopUp::IDD, pParent)
, m_bProductUpdate(false), behavior_factory("WardWizFirewallBlockPopUp")
{
}

/***************************************************************************
Function Name  : ~CWardWizFirewallBlockPopUp
Description    : Destructor
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
CWardWizFirewallBlockPopUp::~CWardWizFirewallBlockPopUp()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
void CWardWizFirewallBlockPopUp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***************************************************************************
Function Name  : MESSAGE_MAP
Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizFirewallBlockPopUp, CDialog)
	ON_WM_NCHITTEST()
	ON_MESSAGE(WM_MESSAGESHOWAPPBLK, ShowAppBlockMessageHandler)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

HWINDOW   CWardWizFirewallBlockPopUp::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizFirewallBlockPopUp::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************
Function Name  : OnInitDialog
Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
BOOL CWardWizFirewallBlockPopUp::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	//SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_BLOCK_APPLICATION_POPUP.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_BLOCK_APPLICATION_POPUP.htm");
	this->SetWindowText(L"AKTRAYAPPBLOCK");
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	//m_root_el = root();
	m_root_el = sciter::dom::element::root_element(this->get_hwnd());
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);
	
	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;
	//this->ShowWindow(SW_HIDE);

	::MoveWindow(this->get_hwnd(), 0, 0, 0, 0, true);
		
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
		CString csMsgType;
		csMsgType.Format(L"%d", m_iTrayMsgType);

		m_svGetMsgType.call((SCITER_STRING)csMsgType, (SCITER_STRING)csFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizFirewallBlockPopUp::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : WindowProc
Description    : Windows Function
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
LRESULT CWardWizFirewallBlockPopUp::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
		AddLogEntry(L"### Excpetion in CWardwizFirewallBlockPopUp::WindowProc", 0, 0, true, SECONDLEVEL);
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
int CWardWizFirewallBlockPopUp::GetTaskBarHeight()
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
		AddLogEntry(L"### Excpetion in CWardwizFirewallBlockPopUp::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
int CWardWizFirewallBlockPopUp::GetTaskBarWidth()
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
		AddLogEntry(L"### Excpetion in CWardwizFirewallBlockPopUp::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : Function for PreTranslation of Message
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
BOOL CWardWizFirewallBlockPopUp::PreTranslateMessage(MSG* pMsg)
{
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : OnNcHitTest
Description    : The framework calls this member function for the CWnd object that contains
the cursor (or the CWnd object that used the SetCapture member function to
capture the mouse input) every time the mouse is moved.
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
LRESULT CWardWizFirewallBlockPopUp::OnNcHitTest(CPoint point)
{
	return __super::OnNcHitTest(point);
}


/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Kunal Waghmare
*  Date           : 10th July 2018
****************************************************************************************************/
json::value CWardWizFirewallBlockPopUp::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFirewallBlockPopUp::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

/***************************************************************************************************
Function Name  : GetLanguageID
Description    : function to get language id.
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
/***************************************************************************************************/
json::value CWardWizFirewallBlockPopUp::On_GetLanguageID()
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
		AddLogEntry(L"### Exception in CWardwizFirewallBlockPopUp::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
/***************************************************************************************************/
json::value CWardWizFirewallBlockPopUp::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFirewallBlockPopUps::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  OnCheckMsgType
*  Description    :  Function to set Message Type
*  Author Name    :  Kunal Waghmare
*  Date           :  11 July 2018
****************************************************************************************************/
json::value CWardWizFirewallBlockPopUp::OnCheckMsgType(SCITER_VALUE svGetMsgType)
{
	try
	{
		m_svGetMsgType = svGetMsgType;
	}
	catch (...)
	{
		AddLogEntry(L"#### Exception in CWardwizFirewallBlockPopUp::OnCheckMsgType", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  SingleInstanceCheck
*  Description    :  Function to check for single instance
*  Author Name    :  Jeena Mariam Saji
*  Date           :  17 August 2018
****************************************************************************************************/
bool CWardWizFirewallBlockPopUp::SingleInstanceCheck()
{
	/*m_hMutexHandle = CreateMutex(NULL, TRUE, L"{0794EE9E-7D0C-44F4-90CE-4EE422A7F931}");
	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}*/
	return false;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CWardWizFirewallBlockPopUp::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFirewallBlockPopUp::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : ShowAppBlockMessageHandler
Description    : Function to handle messages related to show Web block window.
Author Name    : Kunal waghmare
Date           : 15/07/2019
***********************************************************************************************/
LRESULT CWardWizFirewallBlockPopUp::ShowAppBlockMessageHandler(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		PSTAPPBLK pAppBlk = (PSTAPPBLK)lParam;
		if (pAppBlk == NULL)
			return 0;

		CString csMsgType;
		csMsgType.Format(L"%d", pAppBlk->iTrayMsgType);
		sciter::value map;
		map.set_item("one", (SCITER_STRING)csMsgType);
		map.set_item("two", (SCITER_STRING)pAppBlk->szFilePath);

		//Send here event
		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SET_EVENT_CODE_BLK;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizFirewallBlockPopUp::ShowAppBlockMessageHandler", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : OnClose
Description    : callback function which calls before window close
Author Name    : Kunal waghmare
Date           : 15/07/2019
***********************************************************************************************/
void CWardWizFirewallBlockPopUp::OnClose()
{
	__super::OnClose();
}

