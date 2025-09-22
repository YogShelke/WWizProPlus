// USBPopupMsgDlg.cpp : implementation file
/****************************************************
*  Program Name: USBPopupMsgDlg.cpp                                                                                                    
*  Description: Gives popup msg on insertion of any removable device 
*  Author Name: Prajakta                                                                                                      
*  Date Of Creation: 22 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/


#include "stdafx.h"
#include "USBDetectUI.h"
#include "USBDetectUIDlg.h"
#include "USBPopupMsgDlg.h"


// CUSBPopupMsgDlg dialog

IMPLEMENT_DYNAMIC(CUSBPopupMsgDlg, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CUSBPopupMsgDlg                                                     
*  Description    :	C'tor
*  Author Name    : Prajakta                                                                                        
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0010
**********************************************************************************************************/
CUSBPopupMsgDlg::CUSBPopupMsgDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CUSBPopupMsgDlg::IDD, pParent)
{

}


/**********************************************************************************************************                     
*  Function Name  :	~CUSBPopupMsgDlg                                                     
*  Description    :	Destructor
*  Author Name    : Prajakta                                                                                        
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0011
**********************************************************************************************************/
CUSBPopupMsgDlg::~CUSBPopupMsgDlg()
{
}

HWINDOW   CUSBPopupMsgDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CUSBPopupMsgDlg::get_resource_instance() { return theApp.m_hInstance; }
/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Prajakta
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0012
**********************************************************************************************************/
void CUSBPopupMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MSG, m_stMsg);
	DDX_Control(pDX, IDC_BTN_YES, m_btnYES);
	DDX_Control(pDX, IDC_BTN_NO, m_btnNO);
	DDX_Control(pDX, IDC_STATIC_YESTXT, m_stYESTxt);
	DDX_Control(pDX, IDC_STATIC_NOTXT, m_stNOTxt);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_STATIC_USB_DETECT_TEXT, m_stUSBDetectText);
}

/**********************************************************************************************************                     
*  Function Name  :	MESSAGE_MAP                                                     
*  Description    :	Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Prajakta
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0013
**********************************************************************************************************/
BEGIN_MESSAGE_MAP(CUSBPopupMsgDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_YES, &CUSBPopupMsgDlg::OnBnClickedBtnYes)
	ON_BN_CLICKED(IDC_BTN_NO, &CUSBPopupMsgDlg::OnBnClickedBtnNo)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CUSBPopupMsgDlg::OnBnClickedBtnClose)
END_MESSAGE_MAP()


// CUSBPopupMsgDlg message handlers

/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  Author Name    : Prajakta
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0014
**********************************************************************************************************/
BOOL CUSBPopupMsgDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	this->SetWindowText(L"VBUSB");
	m_stUSBDetectText.ShowWindow(SW_HIDE);
	m_stMsg.ShowWindow(SW_HIDE);
	m_btnNO.ShowWindow(SW_HIDE);
	m_stYESTxt.ShowWindow(SW_HIDE);
	m_btnYES.ShowWindow(SW_HIDE);
	m_stNOTxt.ShowWindow(SW_HIDE);
	m_btnClose.ShowWindow(SW_HIDE);
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_USB_SCAN_FIRST_POP_UP.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_USB_SCAN_FIRST_POP_UP.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();//Here its called i guess, -  yes. And you don need m_root_el, youcan  call root() for that. ok
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	CenterWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnYes                                                     
*  Description    :	Send message to tray and close UI.
*  Author Name    : Prajakta
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0015
**********************************************************************************************************/
void CUSBPopupMsgDlg::OnBnClickedBtnYes()
{
	if(!SendmessgaeToTray(RESET_USBSCAN_VARIABLE))
	{
		AddLogEntry(L"### Error in CUSBPopupMsgDlg::SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
	}
	Sleep(20);
	OnOK();
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnNo                                                     
*  Description    :	Send message to tray and close UI.
*  Author Name    : Prajakta
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0016
**********************************************************************************************************/
void CUSBPopupMsgDlg::OnBnClickedBtnNo()
{
	if(!SendmessgaeToTray(RESET_USBSCAN_VARIABLE))
	{
		AddLogEntry(L"### Error in CUSBPopupMsgDlg::SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
	}
	Sleep(20);
	OnCancel();
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Prajakta                                                                                         
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0017
**********************************************************************************************************/
HBRUSH CUSBPopupMsgDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_MSG      ||
		ctrlID == IDC_STATIC_YESTXT   ||
		ctrlID == IDC_STATIC_NOTXT 
		)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} return hbr;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnClose                                                     
*  Description    :	Send message to tray and close UI.
*  Author Name    : Prajakta                                                                                         
*  Date           : 22 Nov 2013
*  SR_NO		  : WRDWIZUSBUI_0018
**********************************************************************************************************/
void CUSBPopupMsgDlg::OnBnClickedBtnClose()
{
	if(!SendmessgaeToTray(RESET_USBSCAN_VARIABLE))
	{
		AddLogEntry(L"### Error in CUSBPopupMsgDlg::SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
	}
	Sleep(20);
	OnCancel();
}

/***********************************************************************************************
  Function Name  : PreTranslateMessage
  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
  Author Name    : Nitin Kolapkar
  Date           : 29 April 2014
  SR_NO			 : WRDWIZUSBUI_0019
***********************************************************************************************/
BOOL CUSBPopupMsgDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***********************************************************************************************
  Function Name  : RefreshString
  Description    : this function is  called for setting the different Fonts
  Author Name    : Nitin Kolapkar
  Date           : 29 April 2014
  SR_NO			 : WRDWIZUSBUI_0020
***********************************************************************************************/
BOOL CUSBPopupMsgDlg :: RefreshString()
{
	m_stUSBDetectText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USBPOPUP_USBDETECT_TEXT"));
	m_stUSBDetectText.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USBPOPUP_MSG"));
	m_stMsg.SetFont(&theApp.m_fontWWTextNormal);
	m_btnYES.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USBPOPUP_BUTTON_YES"));
	m_btnYES.SetFont(&theApp.m_fontWWTextNormal);
	m_btnNO.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USBPOPUP_BUTTON_NO"));
	m_btnNO.SetFont(&theApp.m_fontWWTextNormal);
	m_stYESTxt.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USBPOPUP_YES_TEXT"));
	m_stYESTxt.SetFont(&theApp.m_fontWWTextNormal);
	m_stNOTxt.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USBPOPUP_NO_TEXT"));
	m_stNOTxt.SetFont(&theApp.m_fontWWTextNormal);
	return true;
}

/***********************************************************************************************
  Function Name  : SendMessageToTray()
  Description    : Send message to tray.exe.to update data in usbdetect.dll.
  Author Name    : Neha gharge
  Date           : 26 April 2014
  SR_NO			 : WRDWIZUSBUI_0021
***********************************************************************************************/
bool CUSBPopupMsgDlg::SendmessgaeToTray(int iMessage,bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	szPipeData.iMessageInfo = iMessage;
	CISpyCommunicator objCom(TRAY_SERVER);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CExtractAttchForScan::SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CExtractAttchForScan::SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}


LRESULT CUSBPopupMsgDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;

	lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
	if (bHandled)      // if it was handled by the Sciter
		return lResult; // then no further processing is required.

	return __super::WindowProc(message, wParam, lParam);
}

json::value CUSBPopupMsgDlg::On_CloseUI()
{
	try
	{
		OnBnClickedBtnClose();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBPopupMsgDlg::On_CloseUI", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

json::value CUSBPopupMsgDlg::On_ButtonClickYes()
{
	try
	{
		OnBnClickedBtnYes();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBPopupMsgDlg::On_ButtonClickYes", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 Aug 2016
****************************************************************************************************/
json::value CUSBPopupMsgDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProdID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBPopupMsgDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

json::value CUSBPopupMsgDlg::On_GetAppPath()
{
	return (SCITER_STRING)theApp.GetModuleFilePath();
}
/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Amol Jaware
*  Date			  : 5 Aug 2016
****************************************************************************************************/
json::value CUSBPopupMsgDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Kunal Waghmare
*  Date			  : 26 Dec 2018
****************************************************************************************************/
json::value CUSBPopupMsgDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBPopupMsgDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}