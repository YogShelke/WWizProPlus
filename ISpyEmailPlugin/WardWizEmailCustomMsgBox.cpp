// WardWizEmailCustomMsgBox.cpp : implementation file
/**********************************************************************************************************
Program Name          : WardWizEmailCustomMsgBox.cpp
Description           : Custom message box to confirm from user whether user want to move mail spam to not spam and not spam to spam
Author Name			: Neha Gharge
Date Of Creation      : 29th July 2015
Version No            : 
Special Logic Used    :
***********************************************************************************************************/

#include "stdafx.h"
#include "WardWizEmailCustomMsgBox.h"
#include "OutlookAddinApp.h"
//#include "afxdialogex.h"


// CWardWizEmailCustomMsgBox dialog

IMPLEMENT_DYNAMIC(CWardWizEmailCustomMsgBox, CDialog)
/***************************************************************************************************
*  Function Name  : CWrdWizCustomMsgBox
*  Description    : C'tor
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 30th July,2015
****************************************************************************************************/
CWardWizEmailCustomMsgBox::CWardWizEmailCustomMsgBox(CWnd* pParent /*=NULL*/)
	: CDialog(CWardWizEmailCustomMsgBox::IDD, pParent)
	, m_bcheckBoxtick(false)
	, m_bClickOnSpamMsgBox(false)
{

}

/***************************************************************************************************
*  Function Name  : ~CWrdWizCustomMsgBox
*  Description    : D'tor
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 30th July,2015
****************************************************************************************************/
CWardWizEmailCustomMsgBox::~CWardWizEmailCustomMsgBox()
{
}

/***********************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 30th July,2015
*************************************************************************************************/
void CWardWizEmailCustomMsgBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_QUESTION_FOR_MAKING_SPAM, m_stMessageToMakeMailSpam);
	DDX_Control(pDX, IDC_STATIC_MSG_MAKE_MAIL_NOTSPAM, m_stMessageToMakeMailNotSpam);
	DDX_Control(pDX, IDC_CHECK_ADD_BLACK_WHITE_LIST, m_chkForBlackOrWhiteList);
	DDX_Control(pDX, IDC_STATIC_CHKTEXT_FOR_BLACK_LIST, m_stTextForFutureBlackList);
	DDX_Control(pDX, IDC_STATIC_CHKTEXT_FOR_WHITE_LIST, m_stTextForFutureWhiteList);
	DDX_Control(pDX, IDC_STATIC_QUESTIONMARK, m_stQuestionMark);
}

/***********************************************************************************************
*  Function Name  : MAPPING MESSAGES
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 30th July,2015
*************************************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizEmailCustomMsgBox, CDialog)
	ON_BN_CLICKED(IDOK, &CWardWizEmailCustomMsgBox::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWardWizEmailCustomMsgBox::OnBnClickedCancel)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CWardWizEmailCustomMsgBox message handlers
/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Windows calls the OnInitDialog function through the standard global
dialog-box procedure common to all Microsoft Foundation Class Library
dialog boxes
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 30th July,2015
****************************************************************************************************/
BOOL CWardWizEmailCustomMsgBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 510, 150, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE);

	CRect rect;
	this->GetClientRect(rect);

	m_bmpMsgBoxImage.LoadBitmapW(IDB_BITMAP_BK_IMAGE_FOR_CUSTOM_SCAN);

	m_bmpQuestionMark = LoadBitmapW(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_BITMAP_QUESTIONMARK));
	m_stQuestionMark.SetBitmap(m_bmpQuestionMark);
	m_stQuestionMark.SetWindowPos(&wndTop, rect.left + 10, 15, 42, 39, SWP_SHOWWINDOW);

	if (m_bClickOnSpamMsgBox == true)
	{
		m_stMessageToMakeMailSpam.SetWindowPos(&wndTop, rect.left + 62, 30, 300, 45, SWP_SHOWWINDOW);
		m_stMessageToMakeMailSpam.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_SPAM_BTN_CLICK"));
		m_stMessageToMakeMailNotSpam.ShowWindow(SW_HIDE);
		m_stTextForFutureBlackList.SetWindowPos(&wndTop, rect.left + 23, 85, 300, 45, SWP_SHOWWINDOW);
		m_stTextForFutureBlackList.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_CHK_BLACK_LIST_TEXT"));
		m_stTextForFutureWhiteList.ShowWindow(SW_HIDE);

	}
	else
	{
		m_stMessageToMakeMailNotSpam.SetWindowPos(&wndTop, rect.left + 62, 30, 300, 45, SWP_SHOWWINDOW);
		m_stMessageToMakeMailNotSpam.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_NOT_SPAM_BTN_CLICK"));
		m_stMessageToMakeMailSpam.ShowWindow(SW_HIDE);
		m_stTextForFutureWhiteList.SetWindowPos(&wndTop, rect.left + 23, 85, 300, 45, SWP_SHOWWINDOW);
		m_stTextForFutureWhiteList.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_CHK_WHITE_LIST_TEXT"));
		m_stTextForFutureBlackList.ShowWindow(SW_HIDE);
	}

	m_chkForBlackOrWhiteList.SetWindowPos(&wndTop, rect.left + 8, 85, 13, 13, SWP_SHOWWINDOW);
	m_chkForBlackOrWhiteList.SetCheck(1);

	m_btnCancel.SetWindowPos(&wndTop, rect.left + 412, 82, 65, 21, SWP_SHOWWINDOW);
	m_btnCancel.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USBPOPUP_BUTTON_NO"));

	m_btnOk.SetWindowPos(&wndTop, rect.left + 339, 82, 65, 21, SWP_SHOWWINDOW);
	m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_USBPOPUP_BUTTON_YES"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


/***************************************************************************************************
*  Function Name  : OnBnClickedOk
*  Description    : It closes dialog.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 30th July,2015
****************************************************************************************************/
void CWardWizEmailCustomMsgBox::OnBnClickedOk()
{
	__try
	{
		if (m_chkForBlackOrWhiteList.GetCheck() == BST_CHECKED)
		{
			m_bcheckBoxtick = true;
		}
		CDialog::OnOK();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailCustomMsgBox::OnBnClickedOk()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCancel
*  Description    : It closes dialog
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 30th July,2015
****************************************************************************************************/
void CWardWizEmailCustomMsgBox::OnBnClickedCancel()
{
	__try
	{
		CDialog::OnCancel();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailCustomMsgBox::OnBnClickedOk()", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : OnBackground
*  Description    : It will draw the image in background and also stretch if we set bisStretch = true.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 30th July,2015
****************************************************************************************************/
void CWardWizEmailCustomMsgBox::OnBackground(CBitmap* bmpImg, CDC* pDC, int yHight, bool bisStretch)
{
	try
	{
		CBitmap bmpObj;
		CRect rect;
		CDC visibleDC;
		//int iOldStretchBltMode;
		GetClientRect(&rect);
		visibleDC.CreateCompatibleDC(pDC);
		bmpObj.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		visibleDC.SelectObject(bmpImg);



		if (bisStretch)
		{
			for (int i = 0; i <= rect.Width() / 2; i++)
			{
				//pDC->StretchBlt(i*5, 0, rect.Width(), rect.Height(), &srcDC, 0, 0, 1000,500, SRCCOPY);
				pDC->BitBlt(i * 2, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
			}
		}
		else
		{
			pDC->BitBlt(0, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
		}

		bmpObj.DeleteObject();

		visibleDC.DeleteDC();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailCustomMsgBox::OnBackground", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 30th July,2015
****************************************************************************************************/
void CWardWizEmailCustomMsgBox::OnPaint()
{
	try
	{
		CPaintDC dc(this); // device context for painting
		if (IsIconic())
		{
			CPaintDC dc(this); // device context for painting

			SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

			// Center icon in client rectangle
			int cxIcon = GetSystemMetrics(SM_CXICON);
			int cyIcon = GetSystemMetrics(SM_CYICON);
			CRect rect;
			GetClientRect(&rect);
			int x = (rect.Width() - cxIcon + 1) / 2;
			int y = (rect.Height() - cyIcon + 1) / 2;

			// Draw the icon
			//dc.DrawIcon(x, y, m_hIcon);
		}
		else
		{

			OnBackground(&m_bmpMsgBoxImage, &dc, -43, true);

			CDialog::OnPaint();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::OnPaint", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnCtlColor
*  Description    : Give traspaent background to give controls
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 30th July,2015
****************************************************************************************************/
HBRUSH CWardWizEmailCustomMsgBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	try
	{


		int ctrlID;
		ctrlID = pWnd->GetDlgCtrlID();
		if (ctrlID == IDC_STATIC_CHKTEXT_FOR_BLACK_LIST ||
			ctrlID == IDC_STATIC_CHKTEXT_FOR_WHITE_LIST ||
			ctrlID == IDC_STATIC_QUESTION_FOR_MAKING_SPAM||
			ctrlID == IDC_STATIC_MSG_MAKE_MAIL_NOTSPAM
			)

		{
			pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::OnCtlColor", 0, 0, true, SECONDLEVEL);
	}
	return hbr;
}
