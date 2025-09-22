// WrdwizStartupTipsDlg.cpp : implementation file
/****************************************************
*  Program Name: WrdwizStartupTipsDlg.cpp                                                                                                    
*  Description: Shows start up tip screen before lauching UI.
*  Author Name: Prasanna Lotke  , Neha                                                                                                  
*  Date Of Creation: 27 April 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "WrdwizStartupTipsDlg.h"
#include <fstream>
#include <iostream>
#include "DriverConstants.h"
//#include "CSecure64.h"



// CWrdwizStartupTipsDlg dialog

IMPLEMENT_DYNAMIC(CWrdwizStartupTipsDlg, CDialog)
/***************************************************************************************************                    
*  Function Name  : CWrdwizStartupTipsDlg                                                     
*  Description    : C'tor
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
CWrdwizStartupTipsDlg::CWrdwizStartupTipsDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWrdwizStartupTipsDlg::IDD, pParent)
	,m_dwTotalStartTips(0)
	,m_iLastTooltipShown(0)
{

}

/***************************************************************************************************                    
*  Function Name  : ~CWrdwizStartupTipsDlg                                                     
*  Description    : D'tor
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
CWrdwizStartupTipsDlg::~CWrdwizStartupTipsDlg()
{
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_STARTTIPS_HEADER, m_stShowTipHeader);
	DDX_Control(pDX, IDC_STATIC_STARTTIPS_BULB, m_stShowTipBulb);
	DDX_Control(pDX, IDC_BUTTON_PREV, m_btnPrev);
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BUTTON_OK, m_btnOk);
	DDX_Control(pDX, IDC_RICHEDIT21_TIP, m_RichEdtCtrlTip);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_STATIC_ENABLE_TIPS, m_stEnableTips);
	DDX_Control(pDX, IDC_CHECK_ENABLE_STARTUP_TIPS, m_chkEnableTips);
	DDX_Control(pDX, IDC_STATIC_TIP_OF_DAY, m_stTipOfDay);
	DDX_Control(pDX, IDC_STATIC_DYK, m_stDYK);
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                    
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWrdwizStartupTipsDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_PREV, &CWrdwizStartupTipsDlg::OnBnClickedButtonPrev)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CWrdwizStartupTipsDlg::OnBnClickedButtonNext)
	ON_WM_PAINT()
	ON_EN_VSCROLL(IDC_RICHEDIT21_TIP, &CWrdwizStartupTipsDlg::OnEnVscrollRichedit21Tip)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CWrdwizStartupTipsDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CWrdwizStartupTipsDlg::OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_STARTUP_TIPS, &CWrdwizStartupTipsDlg::OnBnClickedCheckEnableStartupTips)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft  Foundation Class Library dialog boxes
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
BOOL CWrdwizStartupTipsDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	//CSecure64  objCSecure;
	//objCSecure.RegisterProcessId(WLSRV_ID_ELEVEN);//WLSRV_ID_ZERO to register service for process protection

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"));
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SHOW_TIP_BG), _T("JPG")))
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
	Draw();

	ModifyStyle(0, WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX );

	CRect	rect1;
	this->GetClientRect(rect1);

	CRgn RgnRect;
	RgnRect.CreateRectRgn(rect1.left,rect1.top,rect1.right -3,rect1.bottom -3);
	this->SetWindowRgn(RgnRect, TRUE);

	m_stTipOfDay.SetWindowPos(&wndTop,rect1.left + 125,90,320,80,SWP_NOREDRAW);
	m_stTipOfDay.SetTextColor(RGB(109,110,113));

	m_stDYK.SetWindowPos(&wndTop,rect1.left + 125,110,320,80,SWP_NOREDRAW);
	m_stDYK.SetTextColor(RGB(109,110,113));
	
	m_RichEdtCtrlTip.SetWindowPos(&wndTop,rect1.left + 97,165,368,120,SWP_NOREDRAW);
	m_RichEdtCtrlTip.SetBackgroundColor(0,RGB(243,243,244));

// Issue No .652 Rajil Yadav 09/06/2014
	m_bmpShowTipBulb = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_SHOWTIP_BULB));
	m_stShowTipBulb.SetWindowPos(&wndTop,rect1.left + 0,52,121,147,SWP_NOREDRAW);
	m_stShowTipBulb.SetBitmap(m_bmpShowTipBulb);


	m_bmpShowTipHeader = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_SHOWTIP_HEADER));
	m_stShowTipHeader.SetWindowPos(&wndTop,rect1.left + 0,0,586,42,SWP_NOREDRAW);
	m_stShowTipHeader.SetBitmap(m_bmpShowTipHeader);

	m_btnPrev.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnPrev.SetWindowPos(&wndTop,rect1.left + 280,310,57,21,SWP_NOREDRAW);
	m_btnPrev.SetTextColorA(RGB(0,0,0),1,1);
	m_btnPrev.EnableWindow(false);
	
	m_btnNext.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnNext.SetWindowPos(&wndTop,rect1.left + 345,310,57,21,SWP_NOREDRAW);
	m_btnNext.SetTextColorA(RGB(0,0,0),1,1);

	m_btnOk.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnOk.SetWindowPos(&wndTop,rect1.left + 410,310,57,21,SWP_NOREDRAW);
	m_btnOk.SetTextColorA(RGB(0,0,0),1,1);
	
	m_btnClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnClose.SetWindowPos(&wndTop,rect1.left +470,0,26,17,SWP_NOREDRAW);

	//ISSUE NO:- 177 RY Date :- 21/5/2014
	m_chkEnableTips.SetWindowPos(&wndTop,rect1.left + 80,310,13,13,SWP_NOREDRAW);
	m_chkEnableTips.SetCheck(true);
	m_stEnableTips.SetWindowPos(&wndTop,rect1.left + 95,310,150,15,SWP_NOREDRAW);
	m_stEnableTips.SetTextColor(RGB(255,255,255));

	//Setting fonts here
	m_stDYK.SetFont(&theApp.m_FontWWStartUpFontTitle);
	m_stTipOfDay.SetFont(&theApp.m_FontWWStartUpFontSubTitle);
	m_RichEdtCtrlTip.SetFont(&theApp.m_FontWWStartUpTips);

	//m_btnNext.SetFont(&theApp.m_FontWWStartUpTips);
	//m_btnOk.SetFont(&theApp.m_FontWWStartUpTips);
	//m_btnPrev.SetFont(&theApp.m_FontWWStartUpTips);
	m_stEnableTips.SetFont(&theApp.m_FontWWStartUpTips);

/*
	CScrollBar *cScrollBar =  m_RichEdtCtrlTip.GetScrollBarCtrl(1);
	cScrollBar->SetWindowPos(&wndTop,rect1.left + 445,160,5,125,SWP_NOREDRAW);
*/

	// TODO:  Add extra initialization here
	this->SetWindowText(L"WRDWIZAVUI");
	RefreshStrings();
	if(!GetStartUpTips())
	{		
		OnOK();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
// CWrdwizStartupTipsDlg message handlers
/***************************************************************************************************                    
*  Function Name  : RefreshStrings                                                     
*  Description    : this function is  called for setting the Text UI with different Language Support
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::RefreshStrings()
{
	m_stTipOfDay.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TIPS_OF_THE_DAY"));
	m_stDYK.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DYK"));
// Issue No .652 Rajil Yadav 09/06/2014
	m_btnPrev.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PREVIOUS"));
	m_btnNext.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_NEXT"));
	m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_OK"));
	m_btnOk.SetFont(&theApp.m_fontWWTextNormal);
	m_btnNext.SetFont(&theApp.m_fontWWTextNormal);
	m_btnPrev.SetFont(&theApp.m_fontWWTextNormal);
	m_stEnableTips.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_TIPS"));
	//m_RichEdtCtrlTip.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_TIPS"));
}

/***************************************************************************************************                    
*  Function Name  : GetStartUpTips                                                     
*  Description    : Get startup tips from language supported text file into array.
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
bool CWrdwizStartupTipsDlg::GetStartUpTips()
{
	DWORD dwLangID = theApp.m_objwardwizLangManager.GetSelectedLanguage();

	CString csFilePath = theApp.GetModuleFilePath() + L"\\WRDSETTINGS";
	switch(dwLangID)
	{
	case ENGLISH:
		csFilePath += L"\\ENGLISHTIPS.TXT";
		break;
	case HINDI:
		csFilePath += L"\\HINDITIPS.TXT";
		break;
	case GERMAN:
		csFilePath += L"\\GERMANTIPS.TXT";
		break;
	case CHINESE:
		csFilePath += L"\\CHINESETIPS.TXT";
		break;
	case SPANISH:
		csFilePath += L"\\SPANISHTIPS.TXT";
		break;
	case FRENCH:
		csFilePath += L"\\FRENCHTIPS.TXT";
		break;
	}

	if(!PathFileExists(csFilePath))
	{
		::MessageBox(NULL,csFilePath + theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TIPS_FILE_ERROR"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		return false;
	}

	// *****************ISSUE NO : 126 ,179, Neha Gharge all related to tooltip*****************************
	m_iLastTooltipShown = ReadRegistryEntryofLastToolTip();
	int iRegistryValue = m_iLastTooltipShown;

	CString strFileName = csFilePath;
    FILE *fStream;
	CStringArray csOriginalArray;

	_tfopen_s(&fStream, strFileName, _T("r, ccs=UNICODE"));
    CStdioFile File(fStream);
    CString strLine;
    while(File.ReadString(strLine))
    {
        csOriginalArray.Add(strLine);
    }

	File.Close();

	if(csOriginalArray.GetSize() == 0)
	{
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_TIP_NOT_PRESENT_IN"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		return false;
	}
	if(csOriginalArray.GetSize() == 1)
	{
		m_btnPrev.EnableWindow(false);
		m_btnNext.EnableWindow(false);
	}

	int iArrayLength = 0;
	int i;
	
	iArrayLength = static_cast<int>(csOriginalArray.GetSize());
	for(i = 0 ; i < iArrayLength ; i++)
	{
		m_csstartUpTips.Add(csOriginalArray.GetAt(m_iLastTooltipShown));
		m_iLastTooltipShown = (m_iLastTooltipShown % iArrayLength) + 1;
		if(m_iLastTooltipShown == iArrayLength)
		{
			m_iLastTooltipShown = 0;
		}
	}

	CHARFORMAT  cf;
	// Save number of lines before insertion of new text
	// Initialize character format structure
	cf.cbSize		= sizeof(CHARFORMAT2W);
	cf.dwMask		= CFM_COLOR;
	cf.dwEffects	= 0;	// To disable CFE_AUTOCOLOR
	//cf.crTextColor	= RGB(255,255,255);
	cf.crTextColor	= RGB(109,110,113);
	m_dwtipIndex = 0;
	m_iLastTooltipShown = iRegistryValue;
	strLine = m_csstartUpTips.GetAt(m_dwtipIndex);
	

	m_RichEdtCtrlTip.SetSel(-1,-1);
	m_RichEdtCtrlTip.SetSelectionCharFormat(cf);
	m_RichEdtCtrlTip.ReplaceSel(strLine);
	return true;
}

// this is commented because trying to implement new logic with array rotation.Implemnatation is going on
/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonPrev                                                     
*  Description    : Shows previous tool tip on previous button click
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::OnBnClickedButtonPrev()
{
	// *****************ISSUE NO : 126 ,179, Neha Gharge all related to tooltip*****************************
	int noOfTips = static_cast<int>(m_csstartUpTips.GetSize());
	//Clearing previous text:-
	CHARFORMAT  cf;
	// Save number of lines before insertion of new text
	// Initialize character format structure
	cf.cbSize		= sizeof(CHARFORMAT2W);
	cf.dwMask		= CFM_COLOR;
	cf.dwEffects	= 0;	// To disable CFE_AUTOCOLOR
	//cf.crTextColor	= RGB(255,255,255);
	cf.crTextColor	= RGB(109,110,113);
	
	long insertionPoint = m_RichEdtCtrlTip.GetWindowTextLengthW();
	m_RichEdtCtrlTip.SetSel(0,insertionPoint);
	m_RichEdtCtrlTip.ReplaceSel(L"");

	m_RichEdtCtrlTip.SetSel(-1,-1);
	m_RichEdtCtrlTip.SetSelectionCharFormat(cf);
	m_dwtipIndex--;
	m_iLastTooltipShown = (m_iLastTooltipShown % noOfTips) - 1;

	if(m_iLastTooltipShown <= -1)
	{
		m_iLastTooltipShown = noOfTips - 1;
	}

	CString CSline;
	//Inserting new Text
	if(m_dwtipIndex == 0)
	{
		m_btnNext.EnableWindow(true);
		m_btnPrev.EnableWindow(false);
		CSline = m_csstartUpTips.GetAt(m_dwtipIndex);//(noOfTips-1);
		m_RichEdtCtrlTip.ReplaceSel(CSline);
	}
	else
	{
		m_btnNext.EnableWindow(true);
		m_btnPrev.EnableWindow(true);
		CSline = m_csstartUpTips.GetAt(m_dwtipIndex);
		m_RichEdtCtrlTip.ReplaceSel(CSline);
	}
	Invalidate();
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonNext                                                     
*  Description    : Shows next tool tip on next button click
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::OnBnClickedButtonNext()
{
	// *****************ISSUE NO : 126 ,179, Neha Gharge all related to tooltip*****************************
	//Clearing previous text:-
	CHARFORMAT  cf;
	// Save number of lines before insertion of new text
	// Initialize character format structure
	cf.cbSize		= sizeof(CHARFORMAT2W);
	cf.dwMask		= CFM_COLOR;
	cf.dwEffects	= 0;	// To disable CFE_AUTOCOLOR
	//cf.crTextColor	= RGB(255,255,255);
	cf.crTextColor	= RGB(109,110,113);
	
	CString CSline;
	int noOfTips = static_cast<int>(m_csstartUpTips.GetSize());
	m_dwtipIndex++;
	m_iLastTooltipShown = (m_iLastTooltipShown % noOfTips) + 1;
	if(m_iLastTooltipShown >= noOfTips)
	{
		m_iLastTooltipShown = 0;
	}

	if(m_dwtipIndex >= static_cast<DWORD>(noOfTips - 1))
	{
		long insertionPoint = m_RichEdtCtrlTip.GetWindowTextLengthW();
		m_RichEdtCtrlTip.SetSel(0,insertionPoint);
		m_RichEdtCtrlTip.ReplaceSel(L"");
		m_RichEdtCtrlTip.SetSel(-1,-1);
		m_RichEdtCtrlTip.SetSelectionCharFormat(cf);
		m_btnNext.EnableWindow(false);
		m_btnPrev.EnableWindow(true);
		CSline = m_csstartUpTips.GetAt(m_dwtipIndex);
		m_RichEdtCtrlTip.ReplaceSel(CSline);
	}
	else
	{
		long insertionPoint = m_RichEdtCtrlTip.GetWindowTextLengthW();
		m_RichEdtCtrlTip.SetSel(0,insertionPoint);
		m_RichEdtCtrlTip.ReplaceSel(L"");
		m_RichEdtCtrlTip.SetSel(-1,-1);
		m_RichEdtCtrlTip.SetSelectionCharFormat(cf);
		m_btnNext.EnableWindow(true);
		m_btnPrev.EnableWindow(true);
		CSline = m_csstartUpTips.GetAt(m_dwtipIndex);
		m_RichEdtCtrlTip.ReplaceSel(CSline);
	}
	Invalidate();
}

/***************************************************************************************************                    
*  Function Name  : OnPaint                                                     
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::OnPaint()
{

	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CJpegDialog::OnPaint();
	}
}

/***************************************************************************************************                    
*  Function Name  : OnEnVscrollRichedit21Tip                                                     
*  Description    : Enable vertical scroll bar 
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::OnEnVscrollRichedit21Tip()
{
	Invalidate(true);
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonClose                                                     
*  Description    : Stored last tooltip no into registry and close UI.
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::OnBnClickedButtonClose()
{
	// *****************ISSUE NO : 126 ,179, Neha Gharge all related to tooltip*****************************
	WriteToReglastToolTipShown();
	OnOK();
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonOk                                                     
*  Description    : Stored last tooltip no into registry and close UI.
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::OnBnClickedButtonOk()
{
	// *****************ISSUE NO : 126 ,179, Neha Gharge all related to tooltip*****************************
	WriteToReglastToolTipShown();
	OnOK();
}

/***************************************************************************************************                    
*  Function Name  : OnCtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
HBRUSH CWrdwizStartupTipsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_ENABLE_TIPS ||
		ctrlID == IDC_STATIC_TIP_OF_DAY ||
		ctrlID == IDC_STATIC_DYK)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} return hbr;
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedCheckEnableStartupTips                                                     
*  Description    : Checks tick mark of startup tips check box and stored value.
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::OnBnClickedCheckEnableStartupTips()
{
	// TODO: Add your control notification handler code here
	int iCheck = m_chkEnableTips.GetCheck();
	if(iCheck == 0)
	{
		m_dwEnableStartUpTips = 0;
		WriteRegistryEntryofStartUpTips(m_dwEnableStartUpTips);
	}
	else
	{
		m_dwEnableStartUpTips = 1;
		WriteRegistryEntryofStartUpTips(m_dwEnableStartUpTips);
	}
}


/***************************************************************************************************                    
*  Function Name  : WriteRegistryEntryofStartUpTips                                                     
*  Description    : Write registry entry of enable/disable start up tip 
*  Author Name    : Prasanna Lotke
*  SR_NO
*  Date           : 27 April 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::WriteRegistryEntryofStartUpTips(DWORD dwChangeValue)
{
	AddLogEntry(L">>> GenXStartupTipsDlg : WriteRegistryEntryofStartUpTips", 0, 0, true, FIRSTLEVEL);

	LPCTSTR SubKey = TEXT("SOFTWARE\\Wardwiz Antivirus");

	if(!SetRegistrykeyUsingService(SubKey, L"dwShowStartupTips", REG_DWORD, dwChangeValue))
	{
		AddLogEntry(L"### Error in Setting Registry GenXStartupTipsDlg::EnableStartupTipsSettings", 0, 0, true, SECONDLEVEL);
	}
	return;
}


/***************************************************************************************************                    
*  Function Name  : SetRegistrykeyUsingService                                                     
*  Description    : Set registry key using service through pipe.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 5 May 2014
****************************************************************************************************/
bool CWrdwizStartupTipsDlg::SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD; 
	//szPipeData.hHey = HKEY_LOCAL_MACHINE;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName );
	szPipeData.dwSecondValue = dwData;

	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXStartupTipsDlg : SendGenXStartupTipsOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CGenXStartupTipsDlg : SendGenXStartupTipsOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ReadRegistryEntryofLastToolTip()                                                     
*  Description    :	Read tooltip changes or setting into registry 
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
DWORD CWrdwizStartupTipsDlg::ReadRegistryEntryofLastToolTip()
{
	m_iLastTooltipShown = 0; 
	HKEY hKey;
	LONG ReadReg;
	DWORD dwvalueSType;
	DWORD dwvalueSize = sizeof(DWORD);
	//DWORD Chkvalue;
	DWORD dwType=REG_DWORD;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus"),&hKey)!= ERROR_SUCCESS)
	{
		return 0;
	}
	
	ReadReg=RegQueryValueEx(hKey,L"dwLastToolTipShown",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
	if(ReadReg == ERROR_SUCCESS)
	{
		m_iLastTooltipShown=(DWORD)dwvalueSType;
	}
	RegCloseKey(hKey);
	return m_iLastTooltipShown;
}
	
/***************************************************************************************************                    
*  Function Name  : WriteToReglastToolTipShown                                                     
*  Description    : Stores number of last tool tip shown on system.
*  Author Name    : Neha Ghage
*  SR_NO
*  Date           : 5th May 2014
****************************************************************************************************/
void CWrdwizStartupTipsDlg::WriteToReglastToolTipShown()
{
	// *****************ISSUE NO : 126 ,179, Neha Gharge all related to tooltip*****************************
	AddLogEntry(L">>> GenXStartupTipsDlg : WriteRegistryEntryofStartUpTips", 0, 0, true, FIRSTLEVEL);

	LPCTSTR SubKey = TEXT("SOFTWARE\\Wardwiz Antivirus");


	if(m_iLastTooltipShown >= m_csstartUpTips.GetSize()-1)
	{
		m_iLastTooltipShown = 0;
	}
	else
	{
		m_iLastTooltipShown++;
	}

	//DWORD dwChangeValue = m_dwtipIndex;
	DWORD dwChangeValue  = m_iLastTooltipShown;

	if(!SetRegistrykeyUsingService(SubKey, L"dwLastToolTipShown", REG_DWORD, dwChangeValue))
	{
		AddLogEntry(L"### Error in Setting Registry GenXStartupTipsDlg::EnableStartupTipsSettings", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : OnClose
*  Description    : it use to close ToolTipGUI window from TrayExit.
*  Author Name    : Lalit kumawat
*  SR_NO
*  Date           : 4-5-2015
****************************************************************************************************/
void CWrdwizStartupTipsDlg::OnClose()
{
	OnCancel();
}
