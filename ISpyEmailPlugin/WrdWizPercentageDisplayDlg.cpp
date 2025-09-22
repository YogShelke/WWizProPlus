// WrdWizPercentageDisplayDlg.cpp : implementation file
/**********************************************************************************************************                     
Program Name : WrdWizPercentageDisplayDlg.cpp
Description : This dialog used to show progress bar for moving mails

Author Name : Neha Gharge
Date Of Creation : 1th July 2015
Version No : 1.12.0.0
***********************************************************************************************************/

#include "stdafx.h"
#include "WrdWizPercentageDisplayDlg.h"
//#include "afxdialogex.h"
//


// CWrdWizPercentageDisplayDlg dialog

IMPLEMENT_DYNAMIC(CWrdWizPercentageDisplayDlg, CDialog)
/***************************************************************************
Function Name  : CWrdWizPercentageDisplayDlg
Description    : C'tor
Author Name    : Neha Gharge
SR.NO		   :
Date           : 1st July 2015
****************************************************************************/
CWrdWizPercentageDisplayDlg::CWrdWizPercentageDisplayDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWrdWizPercentageDisplayDlg::IDD, pParent)
{
	m_bCancelProcess = false;
	m_dwCurrentMoveMailCount = 0;
	m_bInitializeDlg = false;
}

/***************************************************************************
Function Name  : ~CWrdWizPercentageDisplayDlg
Description    : D'tor
Author Name    : Neha Gharge
SR.NO		   :
Date           : 1st July 2015
****************************************************************************/
CWrdWizPercentageDisplayDlg::~CWrdWizPercentageDisplayDlg()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Neha Gharge
SR.NO		   :
Date           : 1st July 2015
****************************************************************************/
void CWrdWizPercentageDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_PrgToShowMovingMailProcess);
	DDX_Control(pDX, IDC_STATIC_PLEASE_WAIT_MSG, m_stPleaseWaitMsg);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_GIF_MOVEMAIL, m_GifMovingMail);
}


BEGIN_MESSAGE_MAP(CWrdWizPercentageDisplayDlg, CJpegDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CWrdWizPercentageDisplayDlg::OnBnClickedButtonCancel)
END_MESSAGE_MAP()


// CWrdWizPercentageDisplayDlg message handlers

/***************************************************************************
Function Name  : OnInitDialog
Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
				Foundation Class Library dialog boxes
Author Name    : Neha Gharge
SR.NO		   :
Date           : 1St July 2015
****************************************************************************/
BOOL CWrdWizPercentageDisplayDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	if (!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_PERCENTAGE_BG), _T("JPG")))
	{
		::MessageBox(NULL, L"Failed", L"Vibranium", MB_ICONERROR);
	}

	Draw();

	SetTimer(TIMER_MOVEMAIL_STATUS, 100, NULL);

	CRect rect1;
	this->GetClientRect(rect1);

	if (m_GifMovingMail.Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_GIF_MOVINGMAIL), _T("GIF")))
	{
		if (!m_GifMovingMail.IsPlaying())
		{
			m_GifMovingMail.Draw();
		}
	}
	m_GifMovingMail.SetWindowPos(&wndTop, rect1.left + 40, 35, 252, 56, SWP_NOREDRAW);

	m_stPleaseWaitMsg.SetWindowPos(&wndTop, rect1.left + 40, 91, 252, 20, SWP_NOREDRAW);
	m_stPleaseWaitMsg.SetTextColor(RGB(0, 0, 0));
	m_stPleaseWaitMsg.SetBkColor(RGB(255, 255, 255));
	m_stPleaseWaitMsg.SetWindowTextW(L"Please wait while moving mail...");

	m_PrgToShowMovingMailProcess.SetWindowPos(&wndTop, rect1.left + 40,109,252, 20, SWP_SHOWWINDOW);
	
	m_PrgToShowMovingMailProcess.SetBarColor(RGB(171, 238, 0));
	m_PrgToShowMovingMailProcess.SetBkColor(RGB(243, 239, 238));


	m_PrgToShowMovingMailProcess.SetRange(0, 100);
	m_PrgToShowMovingMailProcess.SetPos(0);
	m_PrgToShowMovingMailProcess.ShowWindow(SW_SHOW);


	m_btnCancel.SetSkin(IDB_BITMAP_BTN_WHTBG, IDB_BITMAP_BTN_WHTBG, IDB_BITMAP_HOVER_WHTBG, IDB_BITMAP_DISABLE_WHTBG, 0, 0, 0, 0, 0);
	m_btnCancel.SetWindowPos(&wndTop, rect1.left + 235, 144, 57, 21, SWP_NOREDRAW);
	m_btnCancel.SetTextColorA(RGB(0, 0, 0), 1, 1);

	m_dwCurrentMoveMailCount = 0;
	m_bCancelProcess = false;
	m_bInitializeDlg = true;
	return TRUE;  

}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : Translate window messages before they are dispatched to the TranslateMessage 
		and DispatchMessage Windows functions
Author Name    : Neha Gharge
SR.NO		   :
Date           : 1St July 2015
****************************************************************************/
BOOL CWrdWizPercentageDisplayDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : OnTimer
Description    : The framework calls this member function after each interval specified in the SetTimer 
	member function used to install a timer.
Author Name    : Neha Gharge
SR.NO		   :
Date           : 1St July 2015
****************************************************************************/
void CWrdWizPercentageDisplayDlg::OnTimer(UINT_PTR nIDEvent)
{
	try
	{
		if (nIDEvent == TIMER_MOVEMAIL_STATUS)
		{
			int	iPercentage = int(((float)(m_dwCurrentMoveMailCount) / m_dwTotalMailCount) * 100);
			m_PrgToShowMovingMailProcess.SetPos(iPercentage);
			m_PrgToShowMovingMailProcess.RedrawWindow();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPercentageDisplayDlg::OnTimer", 0, 0, true, SECONDLEVEL);
	}
	CJpegDialog::OnTimer(nIDEvent);

}

/***************************************************************************
Function Name  : EndDialogOFDisplay
Description    : End dialog after completing process
Author Name    : Neha Gharge
SR.NO		   :
Date           : 1St July 2015
****************************************************************************/
void CWrdWizPercentageDisplayDlg::EndDialogOFDisplay()
{
	try
	{
		if (m_GifMovingMail.IsPlaying())
		{
			m_GifMovingMail.Stop();
		}

		if (m_dwTotalMailCount == 0 || m_bCancelProcess == true)
		{
			KillTimer(TIMER_MOVEMAIL_STATUS);
			m_bCancelProcess = false;
			EndDialog(IDOK);

		}
		else if ((m_dwCurrentMoveMailCount + 1) == m_dwTotalMailCount && m_bInitializeDlg == true)
		{
			int iLowRange = 0, iHighRange = 0;
			m_PrgToShowMovingMailProcess.GetRange(iLowRange, iHighRange);
			if (iHighRange > 0)
			{
				m_PrgToShowMovingMailProcess.SetPos(iHighRange);
			}
			m_PrgToShowMovingMailProcess.RedrawWindow();
			KillTimer(TIMER_MOVEMAIL_STATUS);
			AddLogEntry(L">>> Before end dialog", 0, 0, true, FIRSTLEVEL);
			EndDialog(IDOK);
			AddLogEntry(L">>> After end dialog", 0, 0, true, FIRSTLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizPercentageDisplayDlg::EndDialogOFDisplay", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : OnBnClickedButtonCancel
Description    : Action to be taken on cancel button
Author Name    : Neha Gharge
SR.NO		   :
Date           : 2nd July 2015
****************************************************************************/
void CWrdWizPercentageDisplayDlg::OnBnClickedButtonCancel()
{
	m_bCancelProcess = true;
}
