// SpywareFoundDlg.cpp : implementation file
/****************************************************
*  Program Name: SpywareFoundDlg.cpp                                                                                                    
*  Description: Displays total threats detected after complete scanning
*  Author Name: Prajakta                                                                                                      
*  Date Of Creation: 21 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/


#include "stdafx.h"
#include "ISpyGUI.h"
#include "SpywareFoundDlg.h"


// CSpywareFoundDlg dialog

IMPLEMENT_DYNAMIC(CSpywareFoundDlg, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CSpywareFoundDlg                                                     
*  Description    :	C'tor
*  Author Name    : Prajakta   
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
CSpywareFoundDlg::CSpywareFoundDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CSpywareFoundDlg::IDD, pParent)
	, m_iSpywareCount(0)
{

}

/**********************************************************************************************************                     
*  Function Name  :	~CSpywareFoundDlg                                                     
*  Description    :	Destructor
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
CSpywareFoundDlg::~CSpywareFoundDlg()
{
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
void CSpywareFoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_WARNING, m_stWarningMessage);
	DDX_Control(pDX, IDC_STATIC_THREATSFOUND, m_stThreatsFound);
	DDX_Control(pDX, IDC_STATIC_INFOTEXT, m_stMessage);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BTN_OK, m_btnOK);
}

/**********************************************************************************************************                     
*  Function Name  :	MESSAGE_MAP                                                     
*  Description    :	Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
BEGIN_MESSAGE_MAP(CSpywareFoundDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CSpywareFoundDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_OK, &CSpywareFoundDlg::OnBnClickedBtnOk)
END_MESSAGE_MAP()

/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
BOOL CSpywareFoundDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	//SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_WARNING_MSG_BG), _T("JPG")))
	{
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
	::SetForegroundWindow(this->m_hWnd);
	
	Draw();
	CRect rect1;
	this->GetClientRect(rect1);
	//SetWindowPos(&wndTop, rect1.top + 430,rect1.top + 250, rect1.Width()-3, rect1.Height() - 3, SWP_NOREDRAW);

	// ISSUE NO : 428 Neha gharge 26/5/2014*******************************************/
	CRgn		 rgn;
	rgn.CreateRoundRectRgn(rect1.left, rect1.top, rect1.right- 3, rect1.bottom- 3,0,0);
	this->SetWindowRgn(rgn, TRUE);
	//changes neha 04 july
	m_stWarningMessage.SetWindowPos(&wndTop,rect1.left + 150,50,100,17,SWP_NOREDRAW);
	m_stWarningMessage.SetFont(&theApp.m_fontWWTextNormal);
	m_stWarningMessage.SetTextColor(WHITE);

	m_stThreatsFound.SetWindowPos(&wndTop,rect1.left + 80,70,280,17,SWP_NOREDRAW);
	m_stThreatsFound.SetFont(&m_fontTitle);
	m_stThreatsFound.SetTextColor(WHITE);
	m_stThreatsFound.SetFont(&theApp.m_fontWWTextNormal );

	
	m_stMessage.SetWindowPos(&wndTop,rect1.left + 30,115,350,30,SWP_NOREDRAW);
//	m_stMessage.SetWindowText(L"Caution! Since recover functionality is not yet present, \n         you will be unable to recover deleted files.");
	m_stMessage.SetFont(&theApp.m_fontWWTextNormal );

	m_btnOK.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnOK.SetWindowPos(&wndTop,rect1.left + 155,162,57,21,SWP_NOREDRAW);
	m_btnOK.SetTextColorA(BLACK,1,1);
	m_btnOK.SetFont(&theApp.m_fontWWTextNormal );

	/*	ISSUE NO - 518 NAME - NITIN K. TIME - 30th May 2014 */
	m_btnClose.SetSkin(theApp.m_hResDLL, IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnClose.SetWindowPos(&wndTop,rect1.left + 346,0,26,17,SWP_NOREDRAW);


	if(!RefreshString())
	{
		AddLogEntry(L">>> CSpywareFoundDlg::OnInitDialog :: Failed RefreshString()", 0, 0, true, FIRSTLEVEL);
	}
	

	return TRUE;
}

/**********************************************************************************************************                     
*  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that contains the cursor every time the mouse is moved.
*  Author Name    : Prajakta  
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
LRESULT CSpywareFoundDlg::OnNcHitTest(CPoint point)
{
	if(!IsZoomed())
	{
		return HTCAPTION;
	}
	return CJpegDialog::OnNcHitTest(point);
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Prajakta  
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
HBRUSH CSpywareFoundDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	int	ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if((ctrlID == IDC_STATIC_THREATSFOUND)		||
		ctrlID == IDC_STATIC_WARNING			||
		ctrlID == IDC_STATIC_INFOTEXT			)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}
/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnClose                                                     
*  Description    :	Closes the spyware found dialog
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
void CSpywareFoundDlg::OnBnClickedBtnClose()
{
	OnCancel();
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnOk                                                     
*  Description    :	Closes the spyware found dialog
*  Author Name    : Prajakta  
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
void CSpywareFoundDlg::OnBnClickedBtnOk()
{
	OnOK();
}

/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage                                                     
*  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 21 Nov 2013
**********************************************************************************************************/
BOOL CSpywareFoundDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***********************************************************************************************
* Function Name  : RefreshString
* Description    : this function is  called for Language support
* Author Name    : Nitin Kolapkar
* SR_NO		     :
*  Date          : 29 April 2014
***********************************************************************************************/
BOOL CSpywareFoundDlg :: RefreshString()
{
	DWORD m_dwSelectedLangID;
	m_stWarningMessage.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_SPYFOUNDDLG_WARNING_TEXT"));
	
	m_dwSelectedLangID = theApp.m_objwardwizLangManager.GetSelectedLanguage();

	CString csThreatFondText;
	csThreatFondText.Format(L"%d %s", m_iSpywareCount,theApp.m_objwardwizLangManager.GetString(L"IDS_SPYFOUNDDLG_THREATS_FOUND_TEXT"));
	m_stThreatsFound.SetWindowText(csThreatFondText);

	m_stMessage.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_SPYFOUNDDLG_MESSAGE_TEXT"));

	/*	ISSUE NO - 356 NAME - NITIN K. TIME - 24st May 2014 */
	m_btnOK.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_OK"));
	m_btnOK.SetFont(&theApp.m_fontWWTextNormal );
	
	return TRUE;
}