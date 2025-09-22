// TrayPopUpMultiDlg.cpp
//
#include "stdafx.h"
#include "TrayPopUpMultiDlg.h"

// CTrayPopUpMultiDlg dialog

IMPLEMENT_DYNAMIC(CTrayPopUpMultiDlg, CDialog)

/***************************************************************************
Function Name  : CTrayPopUpMultiDlg
Description    : Constructor
Author Name    : Nitin K.
Date           : 24th July 2014
SR_NO          : WRDWIZTRAY_0025
****************************************************************************/
CTrayPopUpMultiDlg::CTrayPopUpMultiDlg(CWnd* pParent /*=NULL*/)
: CJpegDialog(CTrayPopUpMultiDlg::IDD, pParent)
{
	m_iCurrentCount = 0;
}

/***************************************************************************
Function Name  : ~CTrayPopUpMultiDlg
Description    : Destructor
Author Name    : Nitin K.
Date           : 24th July 2014
SR_NO          : WRDWIZTRAY_0026
****************************************************************************/
CTrayPopUpMultiDlg::~CTrayPopUpMultiDlg()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Nitin K.
Date           : 24th July 2014
SR_NO          : WRDWIZTRAY_0027
****************************************************************************/
void CTrayPopUpMultiDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_THREAT_NAME, m_stThreatName);
	DDX_Control(pDX, IDC_STATIC_ATTACHMENT_NAME, m_stAttachmentName);
	DDX_Control(pDX, IDC_STATIC_SENDER_ADDRESS, m_stSenderAddress);
	DDX_Control(pDX, IDC_STATIC_ACTION, m_stAction);
	DDX_Control(pDX, IDC_STATIC_THREAT_TEXT, m_stThreatNameText);
	DDX_Control(pDX, IDC_STATIC_ATTACHMENT_TEXT, m_stAttachmentText);
	DDX_Control(pDX, IDC_STATIC_SENDER_TEXT, m_stSenderText);
	DDX_Control(pDX, IDC_STATIC_ACTION_TEXT, m_stActionText);
	DDX_Control(pDX, IDC_BUTTON_PREV, m_btnPrevious);
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_STATIC_WW_LOGO, m_bmpLogo);
	DDX_Control(pDX, IDC_STATIC_CUR_NO, m_stCurrentNo);
	DDX_Control(pDX, IDC_STATIC_SEPERATOR, m_stSeparator);
	DDX_Control(pDX, IDC_STATIC_TOTAL_COUNT, m_stTotalCount);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
}

/***************************************************************************
Function Name  : BEGIN_MESSAGE_MAP
Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
Author Name    : Nitin K.
Date           : 24th July 2014
SR_NO          : WRDWIZTRAY_0028
****************************************************************************/
BEGIN_MESSAGE_MAP(CTrayPopUpMultiDlg, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CTrayPopUpMultiDlg::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_PREV, &CTrayPopUpMultiDlg::OnBnClickedButtonPrev)
//	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CTrayPopUpMultiDlg::OnBnClickedButtonClose)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

HWINDOW   CTrayPopUpMultiDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CTrayPopUpMultiDlg::get_resource_instance() { return theApp.m_hInstance; }

// CTrayPopUpMultiDlg message handlers

/***************************************************************************
Function Name  : OnInitDialog
Description    : This method is called in response to the WM_INITDIALOG message.
Author Name    : Nitin K.
Date           : 18th July 2014
SR_NO          : WRDWIZTRAY_0029
****************************************************************************/
BOOL CTrayPopUpMultiDlg::OnInitDialog()
{

	CJpegDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	m_iCurrentCount = static_cast<int>(m_ArrThreatName.GetCount());

	//Required Members
	int cxIcon = 0;
	int cyIcon = 0;
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	int i = 0;
	int j = 0;
	int ixRect = 0;
	int iyRect = 0;
	LPCBYTE pb = 0;
	UINT cb = 0;

	//Draw();
	
		//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
		this->setup_callback(); // attach sciter::host callbacks
		sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
		
		// load intial document
		sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_POPUP_THREAT_DETECT.htm", pb, cb);
		(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_POPUP_THREAT_DETECT.htm");

		m_root = root();
		SciterGetElementIntrinsicWidths(m_root, &pIntMinWidth, &pIntMaxWidth);
		SciterGetElementIntrinsicHeight(m_root, pIntMinWidth, &pIntHeight);
		::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

		cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
		cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);
		// Dialog is visible properly
		//24/6/2014 Neha/Nitin
		i = GetTaskBarHeight();
		j = GetTaskBarWidth();
		ixRect = cxIcon - pIntMaxWidth;
		iyRect = cyIcon - pIntHeight;
		this->ShowWindow(SW_HIDE);
		HideAllElements();

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
			SetWindowPos(NULL,ixRect,iyRect+200, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;

		case ABE_LEFT:
			SetWindowPos(NULL,j,iyRect, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;

		case ABE_RIGHT:
			SetWindowPos(NULL, ixRect, iyRect, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CTrayPopUpMultiDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	
	return TRUE;
}

	/***************************************************************************
	Function Name  : HideAllElements
	Description    : To Hide old UI Elements.
	Author Name    : Yogeshwar Rasal
	Date           : 9th July 2016
	****************************************************************************/
	void CTrayPopUpMultiDlg::HideAllElements()
	 {
		m_stAction.ShowWindow(SW_HIDE);
		m_stCurrentNo.ShowWindow(SW_HIDE);
		m_stSeparator.ShowWindow(SW_HIDE);
		m_stTotalCount.ShowWindow(SW_HIDE);
		m_btnPrevious.ShowWindow(SW_HIDE);
		m_btnNext.ShowWindow(SW_HIDE);
	 }
	
	/***************************************************************************
	Function Name  : GetTaskBarHeight
	Description    : To get the Task Bar Height
	Author Name    : Nitin K.
	Date           : 18th July 2014
	SR_NO          : WRDWIZTRAY_0030
	****************************************************************************/
	int CTrayPopUpMultiDlg::GetTaskBarHeight()
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
	Author Name    : Nitin K.
	Date           : 18th July 2014
	SR_NO          : WRDWIZTRAY_0031
	****************************************************************************/
	int CTrayPopUpMultiDlg::GetTaskBarWidth()
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
	Function Name  : OnBnClickedButtonNext
	Description    : To Display next entry
	Author Name    : Nitin K.
	Date           : 18th July 2014
	SR_NO          : WRDWIZTRAY_0032
	****************************************************************************/
	void CTrayPopUpMultiDlg::OnBnClickedButtonNext()
	{
		int nindex = m_iCurrentCount;
		if ((nindex <= m_ArrThreatName.GetCount()) && (nindex <= m_ArrAttachmentName.GetCount()) && (nindex <= m_ArrSenderAddress.GetCount()) && (nindex <= m_ArrActionTaken.GetCount()))
		 {
			if (m_iCurrentCount == 1)
			{
			 m_svThreatinfoCB.call((SCITER_STRING)m_ArrThreatName[nindex], (SCITER_STRING)m_ArrAttachmentName[nindex], (SCITER_STRING)m_ArrSenderAddress[nindex], (SCITER_STRING)m_ArrActionTaken[nindex]);
			}
			else
			{
				nindex = nindex + 1;
				m_svThreatinfoCB.call((SCITER_STRING)m_ArrThreatName[nindex], (SCITER_STRING)m_ArrAttachmentName[nindex], (SCITER_STRING)m_ArrSenderAddress[nindex], (SCITER_STRING)m_ArrActionTaken[nindex]);
			}
		 }
	}

	/***************************************************************************
	Function Name  : OnBnClickedButtonPrev
	Description    : To Display previous entry
	Author Name    : Nitin K.
	Date           : 18th July 2014
	SR_NO          : WRDWIZTRAY_0033
	****************************************************************************/
	void CTrayPopUpMultiDlg::OnBnClickedButtonPrev()
	{
		int nindex = m_iCurrentCount;
		if ((nindex <= m_ArrThreatName.GetCount()) && (nindex <= m_ArrAttachmentName.GetCount()) && (nindex <= m_ArrSenderAddress.GetCount()) && (nindex <= m_ArrActionTaken.GetCount()))
		{
			if (m_iCurrentCount == 1)
			{
				m_svThreatinfoCB.call((SCITER_STRING)m_ArrThreatName[nindex], (SCITER_STRING)m_ArrAttachmentName[nindex], (SCITER_STRING)m_ArrSenderAddress[nindex], (SCITER_STRING)m_ArrActionTaken[nindex]);
			}
			else
			{
				nindex = nindex - 1;
				m_svThreatinfoCB.call((SCITER_STRING)m_ArrThreatName[nindex], (SCITER_STRING)m_ArrAttachmentName[nindex], (SCITER_STRING)m_ArrSenderAddress[nindex], (SCITER_STRING)m_ArrActionTaken[nindex]);
			}
		}
	}

	/***************************************************************************
	Function Name  : OnBnClickedButtonClose
	Description    : This will called when user manually clicks close button
	Author Name    : 
	Date           : 
	****************************************************************************/
	void CTrayPopUpMultiDlg::OnBnClickedButtonClose()
	{
		m_iCurrentCount = 0;
		m_ArrThreatName.RemoveAll();
		m_ArrAttachmentName.RemoveAll();
		m_ArrSenderAddress.RemoveAll();
		m_ArrActionTaken.RemoveAll();
		OnCancel();
	}
	
	/***************************************************************************
	Function Name  : OnCtlColor
	Description    : The framework calls this member function when a child control is about to be drawn.
	Author Name    : Microsoft
	Date           : 18th July 2014
	SR_NO          : WRDWIZTRAY_0039
	****************************************************************************/

	HBRUSH CTrayPopUpMultiDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
	{
		HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		int ctrlID;
		ctrlID = pWnd->GetDlgCtrlID();
		if(
			ctrlID ==	IDC_STATIC_THREAT_NAME			||
			ctrlID ==	IDC_STATIC_ATTACHMENT_NAME		||
			ctrlID ==	IDC_STATIC_SENDER_ADDRESS		||
			ctrlID ==	IDC_STATIC_ACTION				||
			ctrlID ==	IDC_STATIC_THREAT_TEXT			||
			ctrlID ==	IDC_STATIC_ATTACHMENT_TEXT		||
			ctrlID ==	IDC_STATIC_SENDER_TEXT			||
			ctrlID ==	IDC_STATIC_ACTION_TEXT			||
			ctrlID ==	IDC_STATIC_CUR_NO				||
			ctrlID ==	IDC_STATIC_SEPERATOR			||
			ctrlID ==	IDC_STATIC_TOTAL_COUNT			||
			ctrlID ==	IDC_STATIC_WW_LOGO
			)

		{
			pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
		}
		return hbr;
	}

	/***************************************************************************
	Function Name  : OnNcHitTest
	Description    : This will called when user manually clicks close button
	Author Name    : Nitin K.
	Date           : 18th July 2014
	SR_NO          : WRDWIZTRAY_0041
	****************************************************************************/
	LRESULT CTrayPopUpMultiDlg::OnNcHitTest(CPoint point)
	{
		return HTCLIENT;
	}

	/***************************************************************************
	Function Name  : PreTranslateMessage
	Description    : To translate window messages before they are dispatched to
					 TranslateMessage and DispatchMessage Windows functions
	Author Name    : Nitin K.
	Date           : 18th July 2014
	SR_NO          : WRDWIZTRAY_0076
	****************************************************************************/
	BOOL CTrayPopUpMultiDlg::PreTranslateMessage(MSG* pMsg)
	{

		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;

		return CJpegDialog::PreTranslateMessage(pMsg);
	}

	/***************************************************************************
	Function Name  : UpdateUI
	Description    : Update the tray UI with new mail details.
	Author Name    : Nitin K./Neha Gharge
	Date           : 24th June 2015
	SR_NO          :
	****************************************************************************/
	void CTrayPopUpMultiDlg::UpdateUI()
	{
		CString szCurrentCount;
		szCurrentCount.Format(L"%d / %d", m_iCurrentCount, m_iFinalCount);
//		m_stCurrentNo.SetWindowTextW(szCurrentCount);  
//		m_stCurrentNo.RedrawWindow();
		Invalidate();
	}

	/***************************************************************************
	Function Name  : OnMouseMove
	Description    : Pop Up must remain constant in case of Mouse move on Dialog
	Author Name    : Nitin Kolapkar
	Date           : 26th August 2015
	SR_NO          :
	****************************************************************************/
	void CTrayPopUpMultiDlg::OnMouseMove(UINT nFlags, CPoint point)
	{
		CJpegDialog::OnMouseMove(nFlags, point);
	}

	/***************************************************************************************************
	*  Function Name  :	WindowProc
	*  Description    :	This callback Procedure is used to Handle All Window Actions
	*  Author Name    :	Yogeshwar Rasal
	*  Date           :	06 July 2016
	****************************************************************************************************/
	LRESULT CTrayPopUpMultiDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
			(L"### Excpetion in CTrayPopUpMultiDlg::WindowProc", 0, 0, true, SECONDLEVEL);
		}
		return CJpegDialog::WindowProc(message, wParam, lParam);
	}
	
	/***************************************************************************************************
	*  Function Name  :	GetThreat_Info
	*  Description    :	This Sciter Function is used to set values of Threats.
	*  Author Name    :	Yogeshwar Rasal
	*  Date           :	06 July 2016
	****************************************************************************************************/
	json::value CTrayPopUpMultiDlg::GetThreatInfo(SCITER_VALUE svThreatInfo)
	{
		try
		{
			m_svThreatinfoCB = svThreatInfo;
			int iIndex = m_iCurrentCount;
			if ((iIndex <= m_ArrThreatName.GetCount()) && (iIndex <= m_ArrAttachmentName.GetCount()) && (iIndex <= m_ArrSenderAddress.GetCount()) && (iIndex <= m_ArrActionTaken.GetCount()))
			{
				m_svThreatinfoCB.call((SCITER_STRING)m_ArrThreatName[iIndex], (SCITER_STRING)m_ArrAttachmentName[iIndex], (SCITER_STRING)m_ArrSenderAddress[iIndex], (SCITER_STRING)m_ArrActionTaken[iIndex]);
			}
		}
		catch (...)
		{
			AddLogEntry(L"### Excpetion in CTrayPopUpMultiDlg::GetThreatInfo", 0, 0, true, SECONDLEVEL);
		}
	  return json::value(0);
	}

	/***************************************************************************************************
	*  Function Name  :	onClickBTNNext
	*  Description    :	This Sciter Function is used to shift at next Page.
	*  Author Name    :	Yogeshwar Rasal
	*  Date           :	06 July 2016
	****************************************************************************************************/
	json::value CTrayPopUpMultiDlg::onClickBTNNext()
	{
		try
		{
			OnBnClickedButtonNext();
		}
		catch (...)
		{
			AddLogEntry(L"### Excpetion in CTrayPopUpMultiDlg::onClickBTNNext", 0, 0, true, SECONDLEVEL);
		}
     return json::value(0);
	}

	/***************************************************************************************************
	*  Function Name  :	onClickBTNPrev
	*  Description    :	This Sciter Function is used to shift at next Page.
	*  Author Name    :	Yogeshwar Rasal
	*  Date           :	06 July 2016
	****************************************************************************************************/
	json::value CTrayPopUpMultiDlg::onClickBTNPrev()
	{
		try
		{
			OnBnClickedButtonPrev();
		}
		catch (...)
		{
			AddLogEntry(L"### Excpetion in CTrayPopUpMultiDlg::onClickBTNPrev", 0, 0, true, SECONDLEVEL);
		}
	 return json::value(0);
	}

	/***************************************************************************************************
	*  Function Name  :	onClickBTNClose
	*  Description    :	This Sciter Function is used to close Mail Popup.
	*  Author Name    :	Yogeshwar Rasal
	*  Date           :	06 July 2016
	****************************************************************************************************/
	json::value CTrayPopUpMultiDlg::onClickBTNClose()
	{
		try
		{
			OnBnClickedButtonClose();
		}
		catch (...)
		{
			AddLogEntry(L"### Excpetion in CTrayPopUpMultiDlg::onClickBTNClose", 0, 0, true, SECONDLEVEL);
		}
	 return json::value(0);
	}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
	json::value CTrayPopUpMultiDlg::On_GetLanguageID()
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
		AddLogEntry(L"### Exception in CTrayPopUpMultiDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 Aug 2016
****************************************************************************************************/
	json::value CTrayPopUpMultiDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CTrayPopUpMultiDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
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
	json::value CTrayPopUpMultiDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CTrayPopUpMultiDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CTrayPopUpMultiDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CTrayPopUpMultiDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}