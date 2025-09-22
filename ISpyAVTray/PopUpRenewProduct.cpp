// PopUpWithReminder.cpp : implementation file
/*******************************************************************************************
*  Program Name: PopUpRenewProduct.cpp
*  Description:  PopUp for Renewal of Product
*  Author Name:  Jeena Mariam Saji
*  Date Of Creation: 12 April 2017
*  Version No:   3.1.0.0
*********************************************************************************************/
#include "stdafx.h"
#include "PopUpRenewProduct.h"
#include "afxdialogex.h"
#include "ISpyAVTray.h"

// CPopUpRenewProduct dialog


#define		IDR_REGDATA				2000

IMPLEMENT_DYNAMIC(CPopUpRenewProduct, CDialog)
/***************************************************************************************************
*  Function Name  : CPopUpRenewProduct
*  Description    : C'tor
*  Author Name    : Jeena Mariam Saji
*  Date           : 12 April 2017
****************************************************************************************************/
CPopUpRenewProduct::CPopUpRenewProduct(CWnd* pParent /*=NULL*/)
: CDialog(CPopUpRenewProduct::IDD, pParent)
{
	memset(&m_szRegKey, 0, sizeof(m_szRegKey));
}

/***************************************************************************************************
*  Function Name  : ~CPopUpRenewProduct
*  Description    : C'tor
*  Author Name    : Jeena Mariam Saji
*  Date           : 12 April 2017
****************************************************************************************************/
CPopUpRenewProduct::~CPopUpRenewProduct()
{
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Exachaning data
*  Author Name    : Jeena Mariam Saji
*  Date           : 12 April 2017
****************************************************************************************************/
void CPopUpRenewProduct::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPopUpRenewProduct, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

HWINDOW   CPopUpRenewProduct::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CPopUpRenewProduct::get_resource_instance() { return theApp.m_hInstance; }

// CPopUpRenewProduct message handlers
/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Initialization of dialog
*  Author Name    : Jeena Mariam Saji
*  Date           : 12 April 2017
****************************************************************************************************/
BOOL CPopUpRenewProduct::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	BringWindowToTop();
	//	SetForegroundWindow();
	SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_PRODUCT_EXPIRED.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_PRODUCT_EXPIRED.htm");
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);
	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************
Function Name  : OnCtlColor
Description    : The framework calls this member function when a child control is about to be drawn.
Author Name    : Jeena Mariam Saji
Date           : 12 April 2017
****************************************************************************/
HBRUSH CPopUpRenewProduct::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if (
		ctrlID == IDC_STATIC_HEADER_TEXT_LIVE_REM_UPDATE ||
		ctrlID == IDC_STATIC_REM_UPDATE_TEXT ||
		ctrlID == IDC_STATIC_REM_CMB
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
Author Name    : Jeena Mariam Saji
Date           : 12 April 2017
****************************************************************************/
LRESULT CPopUpRenewProduct::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	return HTCLIENT;
}

/***************************************************************************************************
*  Function Name  :	WindowProc
*  Description    :	This callback Procedure is used to Handle All Window Actions
*  Author Name    : Jeena Mariam Saji
*  Date           : 12 April 2017
****************************************************************************************************/
LRESULT CPopUpRenewProduct::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
		AddLogEntry(L"### Excpetion in CPopUpRenewProduct::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  :	OnRebootLater
*  Description    :	Sciter Function to Handle "Restart Later " Button.
*  Author Name    :	Jeena Mariam Saji
*  Date           : 12 April 2017
****************************************************************************************************/
json::value CPopUpRenewProduct::OnRenewProduct()
{
	try
	{
		HWND hWindow = ::FindWindow(NULL, L"VIBRANIUMUI");
		if (hWindow)
		{
			::SendMessage(hWindow, WM_MESSAGESHOWREG, 0, 0);

			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
		}
		else// If UI is not open then it will make shellexecute() call
		{
			CString csUIFilePath;
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (!GetModulePath(szModulePath, MAX_PATH))
			{
				return false;
			}
			csUIFilePath = szModulePath;
			csUIFilePath += L"\\VBUI.EXE";
			ShellExecute(NULL, L"open", csUIFilePath, L"-SHOWREG", NULL, SW_SHOW);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpRenewProduct::OnRebootLater", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : OnLoadProductDetails
*  Description    : Function to get details of product whether it is expired
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 April 2017
****************************************************************************************************/
json::value CPopUpRenewProduct::OnLoadProductDetails(SCITER_VALUE svProductDetails)
{
	try
	{
		m_svProductDetails = svProductDetails;
		m_szEmailId_GUI = theApp.GetRegisteredEmailID();
		m_dwNoofDays = theApp.m_dwDaysLeft;

		if (m_szEmailId_GUI == L"" && _tcslen(theApp.m_szRegKey) == 0)
		{
			m_iDisabled = 5;		//Unregistered
		}
		else if (m_szEmailId_GUI && _tcslen(theApp.m_szRegKey) == 0)
		{
			m_iDisabled = 1;
			if (m_dwNoofDays == 0)		//Trial Version
			{
				m_iDisabled = 4;		//Expired
			}
		}
		else if (theApp.m_szRegKey && m_szEmailId_GUI)
		{
			m_iDisabled = 2;		//Registered User
			if (m_dwNoofDays == 0)
			{
				m_iDisabled = 3;	//Expired
			}
		}
		m_svProductDetails.call(m_iDisabled);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpRenewProduct::OnLoadProductDetails", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 April 2017
****************************************************************************************************/
json::value CPopUpRenewProduct::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpRenewProduct::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : Function to get Language ID
Author Name    : Jeena Mariam Saji
Date		   : 13 April 2017
/***************************************************************************************************/
json::value CPopUpRenewProduct::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_dwSelectedLangID;

		CString csLangID;
		csLangID.Format(L"LangID: %d", theApp.m_dwSelectedLangID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpRenewProduct::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 April 2017
****************************************************************************************************/
json::value CPopUpRenewProduct::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpRenewProduct::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CPopUpRenewProduct::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPopUpRenewProduct::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}