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
#include "WardWizPortScanPopUp.h"

// CWardWizPortScanPopUp dialog

IMPLEMENT_DYNAMIC(CWardWizPortScanPopUp, CDialog)


/***************************************************************************
Function Name  : CWardWizPortScanPopUp
Description    : Constructor
Author Name    : Kunal Waghmare
Date           : 10th July 2018
Updated Date   : 13 Aug 2018
****************************************************************************/
CWardWizPortScanPopUp::CWardWizPortScanPopUp(CWnd* pParent)
: CDialog(CWardWizPortScanPopUp::IDD, pParent)
	, m_bProductUpdate(false)
{
	m_hMutexHandle = NULL;	
}

/***************************************************************************
Function Name  : ~CWardWizPortScanPopUp
Description    : Destructor
Author Name    : Kunal Waghmare
Date           : 10th July 2018
Updated Date   : 13 Aug 2018
****************************************************************************/
CWardWizPortScanPopUp::~CWardWizPortScanPopUp()
{
	if (m_hMutexHandle != NULL)
	{
		ReleaseMutex(m_hMutexHandle);
		CloseHandle(m_hMutexHandle);
		m_hMutexHandle = NULL;
	}
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
void CWardWizPortScanPopUp::DoDataExchange(CDataExchange* pDX)
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
BEGIN_MESSAGE_MAP(CWardWizPortScanPopUp, CDialog)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

HWINDOW   CWardWizPortScanPopUp::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizPortScanPopUp::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************
Function Name  : OnInitDialog
Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
BOOL CWardWizPortScanPopUp::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	
	if (SingleInstanceCheck())
	{
		EndDialog(0);
		return FALSE;
	}

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_PORT_SCAN.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_PORT_SCAN.htm");
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
	this->ShowWindow(SW_HIDE);
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
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizPortScanPopUp::OnInitDialog", 0, 0, true, SECONDLEVEL);
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
LRESULT CWardWizPortScanPopUp::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
		AddLogEntry(L"### Excpetion in CWardwizPortScanPopUp::WindowProc", 0, 0, true, SECONDLEVEL);
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
int CWardWizPortScanPopUp::GetTaskBarHeight()
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
		AddLogEntry(L"### Excpetion in CWardwizPortScanPopUp::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
int CWardWizPortScanPopUp::GetTaskBarWidth()
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
		AddLogEntry(L"### Excpetion in CWardwizPortScanPopUp::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : Function for PreTranslation of Message
Author Name    : Kunal Waghmare
Date           : 10th July 2018
SR_NO		   :
****************************************************************************/
BOOL CWardWizPortScanPopUp::PreTranslateMessage(MSG* pMsg)
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
LRESULT CWardWizPortScanPopUp::OnNcHitTest(CPoint point)
{
	return __super::OnNcHitTest(point);
}


/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Kunal Waghmare
*  Date           : 10th July 2018
****************************************************************************************************/
json::value CWardWizPortScanPopUp::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPortScanPopUp::On_GetProductID", 0, 0, true, SECONDLEVEL);
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
json::value CWardWizPortScanPopUp::On_GetLanguageID()
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
		AddLogEntry(L"### Exception in CWardwizPortScanPopUp::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
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
json::value CWardWizPortScanPopUp::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPortScanPopUps::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SingleInstanceCheck
*  Description    : Check whether port scan instance is exist or not
*  Author Name    : Kunal Waghmare
*  Date           : 13 Aug 2018
****************************************************************************************************/
bool CWardWizPortScanPopUp::SingleInstanceCheck()
{
	m_hMutexHandle = CreateMutex(NULL, TRUE, L"{7ACBFD8D-04AA-4CF5-A202-2472FBE8EEDE}");
	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CWardWizPortScanPopUp::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPortScanPopUp::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}