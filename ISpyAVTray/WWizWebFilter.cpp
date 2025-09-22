/*******************************************************************************************
*  Program Name   :	WWizWebFilter.cpp
*  Description    :	It is Website blocked show Notification popup
*  Author Name    : Nitin Shelar
*  Date           : 11/07/2019
*  Version No:                                                                                                                              *                                                                                                                *
*********************************************************************************************/
#include "stdafx.h"
#include "WWizWebFilter.h"

#define SET_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 9)
IMPLEMENT_DYNAMIC(CWWizWebFilterDlg, CDialog)

/***************************************************************************
Function Name  : CWWizWebFilter
Description    : Constructor
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
CWWizWebFilterDlg::CWWizWebFilterDlg(CWnd* pParent /*=NULL*/)
: CDialog(CWWizWebFilterDlg::IDD, pParent), behavior_factory("WrdWizWebBlock"),
m_bTrayforBrowserSec(false)
{
}

/***************************************************************************
Function Name  : ~CWWizWebFilter
Description    : Destructor
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
CWWizWebFilterDlg::~CWWizWebFilterDlg()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
void CWWizWebFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/***************************************************************************
Function Name  : MESSAGE_MAP
Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
BEGIN_MESSAGE_MAP(CWWizWebFilterDlg, CDialog)
	ON_WM_NCHITTEST()
	ON_MESSAGE(WM_MESSAGESHOWCONTENT, ShowWebBlockMessageHandler)
END_MESSAGE_MAP()

HWINDOW   CWWizWebFilterDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWWizWebFilterDlg::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************
Function Name  : OnInitDialog
Description    : Windows calls the OnInitDialog function through the standard global
				 dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
BOOL CWWizWebFilterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback();
	sciter::attach_dom_event_handler(this->get_hwnd(), this); 
	
	// load intial document IDR_HTM_TRAY_LIVE_UPDATE
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_BLK_SPECIFIC_WEBSITE.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_BLK_SPECIFIC_WEBSITE.htm");
	this->SetWindowText(L"AKTRAYWBBLOCKDLG");
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
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWebFilterDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : OnBnClickedOK
Description    : Function to close PopUp
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
void CWWizWebFilterDlg::OnBnClickedOK()
{
	try
	{
		OnOK();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWebFilterDlg::OnBnClickedOK", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : WindowProc
Description    : Windows Function
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
LRESULT CWWizWebFilterDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
		AddLogEntry(L"### Excpetion in CWardwizWebFilterDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : Function for PreTranslation of Message
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
BOOL CWWizWebFilterDlg::PreTranslateMessage(MSG* pMsg)
{
	try
	{
		if (pMsg->wParam == VK_F4 || pMsg->wParam == VK_MENU)
		{
			return TRUE;
		}
		if (pMsg->message == WM_CHAR || pMsg->message == WM_UNICHAR)
		{
			WindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWebFilterDlg::PreTranslateMessage", 0, 0, true, SECONDLEVEL);
	}
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : OnNcHitTest
Description    : The framework calls this member function for the CWnd object that contains
				 the cursor (or the CWnd object that used the SetCapture member function to
				 capture the mouse input) every time the mouse is moved.
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
LRESULT CWWizWebFilterDlg::OnNcHitTest(CPoint point)
{
	return __super::OnNcHitTest(point);
}

/***************************************************************************
Function Name  : On_ClickOK
Description    : Function to close PopUp after clicking OK
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
json::value CWWizWebFilterDlg::On_ClickOK()
{
	try
	{
		OnBnClickedOK();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWebFilterDlg::On_ClickOK", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : On_ClickClose
Description    : Function to Used for close the Window.
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
json::value CWWizWebFilterDlg::On_ClickClose()
{
	try
	{
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"#### Exception in CWardwizWebFilterDlg::On_ClickClose", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
int CWWizWebFilterDlg::GetTaskBarHeight()
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
		AddLogEntry(L"### Excpetion in CWardwizWebFilterDlg::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
int CWWizWebFilterDlg::GetTaskBarWidth()
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
		AddLogEntry(L"### Excpetion in CWardwizWebFilterDlg::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : To Get the Language ID and Set it in Script
Author Name    : Nitin Shelar
Date           : 11/07/2019
/***************************************************************************************************/
json::value CWWizWebFilterDlg::On_GetLanguageID()
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
		AddLogEntry(L"### Exception in CWardwizWebFilterDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Nitin Shelar
*  Date           : 11/07/2019
****************************************************************************************************/
json::value CWWizWebFilterDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWebFilterDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Shelar
Date           : 11/07/2019
/***************************************************************************************************/
json::value CWWizWebFilterDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWebFilterDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetShowBlockSite
Description    : for Get the url and category and set it in pop-up
Author Name    : Nitin Shelar
Date           : 12/07/2019
/***************************************************************************************************/
json::value CWWizWebFilterDlg::On_GetShowBlockSite(SCITER_VALUE svOnCallGetMsgType)
{
	try
	{
		m_callMsgType = svOnCallGetMsgType;
		m_callMsgType.call((SCITER_STRING)m_csUrl, (SCITER_STRING)m_csCategory);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWebFilterDlg::On_GetShowBlockSite", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : ShowWebBlockMessageHandler
Description    : Function to handle messages related to show Web block window.
Author Name    : Nitin Shelar
Date           : 15/07/2019
***********************************************************************************************/
LRESULT CWWizWebFilterDlg::ShowWebBlockMessageHandler(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		PSTWEBCAT pWebCategories = (PSTWEBCAT)lParam;
		if (pWebCategories == NULL)
			return 0;

		sciter::value map;
		map.set_item("one", (SCITER_STRING)pWebCategories->szURL);
		map.set_item("two", (SCITER_STRING)pWebCategories->szCategory);

		//Send here event
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SET_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWebFilterDlg::ShowWebBlockMessageHandler", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/***********************************************************************************************
Function Name  : OnCheckMsgType
Description    : To Check type of message for Browser Secutiry popup
Author Name    : Swapnil Bhave
Date           : 10/09/2019
***********************************************************************************************/
json::value CWWizWebFilterDlg::OnCheckMsgType()
{
	try
	{
		int iTrayBrowserSecVal = 1;
		if (m_bTrayforBrowserSec)
		{
			return iTrayBrowserSecVal;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWebFilterDlg::OnCheckMsgType", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/***********************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Swapnil Bhave
Date           : 10/09/2019
***********************************************************************************************/
json::value CWWizWebFilterDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWebFilterDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}