/*******************************************************************************************
*  Program Name:		WardWizSchTempScanDlg.cpp
*  Description:			To show temp file cleaner tray popup which will come by schedule scan.
*  Author Name:			Amol Jaware
*  Date Of Creation:	12th March 2019
*  Version No:                                                                                                                              *                                                                                                                *
*********************************************************************************************/
// WardWizSchTempScanDlg.cpp : implementation file

#include "stdafx.h"
#include "WardWizSchTempScanDlg.h"
#include "afxdialogex.h"


// CWardWizSchTempScanDlg dialog

IMPLEMENT_DYNAMIC(CWardWizSchTempScanDlg, CDialog)

/***************************************************************************
Function Name  : CWardWizSchTempScanDlg
Description    : Constructor
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
CWardWizSchTempScanDlg::CWardWizSchTempScanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWardWizSchTempScanDlg::IDD, pParent)
{

}

/***************************************************************************
Function Name  : CWardWizSchTempScanDlg
Description    : Destructor
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
CWardWizSchTempScanDlg::~CWardWizSchTempScanDlg()
{
}

void CWardWizSchTempScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWardWizSchTempScanDlg, CDialog)
END_MESSAGE_MAP()


HWINDOW   CWardWizSchTempScanDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizSchTempScanDlg::get_resource_instance() { return theApp.m_hInstance; }

// CWardWizSchTempScanDlg message handlers

/***************************************************************************
Function Name  : OnInitDialog
Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
BOOL CWardWizSchTempScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_TEMP_FILE_CLNR.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_TEMP_FILE_CLNR.htm");
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
		AddLogEntry(L"### Excpetion in CWardwizSchTempScanDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : OnBnClickedOK
Description    : Function to close PopUp
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
void CWardWizSchTempScanDlg::OnBnClickedOK()
{
	try
	{
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSchTempScanDlg::OnBnClickedOK", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : WindowProc
Description    : Windows Function
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
LRESULT CWardWizSchTempScanDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
		AddLogEntry(L"### Excpetion in WindowProc", 0, 0, true, SECONDLEVEL);
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
int CWardWizSchTempScanDlg::GetTaskBarHeight()
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
		AddLogEntry(L"### Excpetion in CWardwizSchTempScanDlg::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
int CWardWizSchTempScanDlg::GetTaskBarWidth()
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
		AddLogEntry(L"### Excpetion in CWardwizSchTempScanDlg::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : Function for PreTranslation of Message
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
BOOL CWardWizSchTempScanDlg::PreTranslateMessage(MSG* pMsg)
{
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : OnNcHitTest
Description    : The framework calls this member function for the CWnd object that contains
the cursor (or the CWnd object that used the SetCapture member function to
capture the mouse input) every time the mouse is moved.
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
LRESULT CWardWizSchTempScanDlg::OnNcHitTest(CPoint point)
{
	return __super::OnNcHitTest(point);
}

/***************************************************************************
Function Name  : On_ClickOK
Description    : Function to close PopUp after clicking OK
Author Name    : Amol Jaware
Date           : 12th March 2019
SR_NO		   :
****************************************************************************/
json::value CWardWizSchTempScanDlg::On_ClickOK()
{
	try
	{
		OnBnClickedOK();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSchTempScanDlg::On_ClickOK", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : To Get the Language ID and Set it in Script
Author Name    : Amol Jaware
SR_NO		   :
Date           : 12th March 2019
/***************************************************************************************************/
json::value CWardWizSchTempScanDlg::On_GetLanguageID()
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
		AddLogEntry(L"### Exception in CWardwizSchTempScanDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Amol Jaware
*  Date			  : 12th March 2019
****************************************************************************************************/
json::value CWardWizSchTempScanDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSchTempScanDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Amol Jaware
SR_NO		   :
Date           : 12th March 2019
/***************************************************************************************************/
json::value CWardWizSchTempScanDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSchTempScanDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_ClickStartScan
Description    : To start Scan
Author Name    : Amol Jaware
SR_NO		   :
Date           : 12th March 2019
/***************************************************************************************************/
json::value CWardWizSchTempScanDlg::On_ClickStartScan()
{
	try
	{
		OnOK();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSchTempScanDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Amol Jaware
Date           : 12th March 2019
/***************************************************************************************************/
json::value CWardWizSchTempScanDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSchTempScanDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}