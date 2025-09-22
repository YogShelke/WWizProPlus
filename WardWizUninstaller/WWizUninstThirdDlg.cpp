/**********************************************************************************************************                     
	  Program Name          : WWizUninstThirdDlg.cpp
	  Description           : 
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 6th Feb 2015
	  Version No            : 1.9.0.0
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizUninstaller.h"
#include "WWizUninstThirdDlg.h"

IMPLEMENT_DYNAMIC(CWWizUninstThirdDlg, CDialog)

/***************************************************************************
Function Name  : CWWizUninstThirdDlg
Description    : C'tor
Author Name    : Nitin K
SR_NO			 :
Date           : 06th Feb 2015
****************************************************************************/
CWWizUninstThirdDlg::CWWizUninstThirdDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWWizUninstThirdDlg::IDD, pParent)
{

}
/***************************************************************************
Function Name  : CWardWizUninstallerDlg
Description    : D'tor
Author Name    : Nitin K
SR_NO			 :
Date           : 06th Feb 2015
****************************************************************************/
CWWizUninstThirdDlg::~CWWizUninstThirdDlg()
{
}
/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    :
SR_NO			 :
Date           : 06th Feb 2015
****************************************************************************/
void CWWizUninstThirdDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_STATIC_UNINSTALL_COMPLETED, m_stUninstallCompleted);
	DDX_Control(pDX, IDC_STATIC_FINISH_BMP, m_bmpFinishBMP);
	DDX_Control(pDX, IDC_BUTTON_RESTART_NOW, m_btnRestartNow);
	DDX_Control(pDX, IDC_STATIC_FINISH_SUCCESS_MSG, m_stSuccessMsg);
	DDX_Control(pDX, IDC_STATIC_FINISH_SUCCESS_MSG2, m_stSuccessMsg2);
	DDX_Control(pDX, IDC_STATIC_UNINTALL_CONFM_TITLE, m_stUninstallConfirmTitle);
	DDX_Control(pDX, IDC_STATIC_ALL_RIGHTS_RESERVED, m_stAllRightReserved);
	DDX_Control(pDX, IDC_BUTTON_RESTART_LATER, m_btRestartLetr);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    :
*  SR_NO
*  Date           : 06th Feb 2015
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWWizUninstThirdDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_RESTART_NOW, &CWWizUninstThirdDlg::OnBnClickedButtonRestartNow)
	ON_BN_CLICKED(IDC_BUTTON_RESTART_LATER, &CWWizUninstThirdDlg::OnBnClickedButtonRestartLater)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft Foundation Class Library dialog boxes
*  Author Name    :
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
BOOL CWWizUninstThirdDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	
	CRect rect;
	this->GetClientRect(rect);
	SetWindowPos(NULL, 0,50, rect.Width()+190, rect.Height()+40 , SWP_NOREDRAW);
	
	//CWardWizUninstallerDlg     objWardWizUninstallerDlg = (CWardWizUninstallerDlg)theApp.m_pMainWnd;
	////theApp.m_dlg.m_stUninstallTitle.SetWindowTextW(L"Done"); 
	//if(theApp.m_iPageNumber ==3 )
	//{
	//	objWardWizUninstallerDlg.m_stUninstallTitle.SetWindowTextW(L"Done");
	//}

	//Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	 m_stSuccessMsg.SetWindowPos(&wndTop,rect.left +150,rect.top +200,rect.Width(),50, SWP_NOREDRAW);
	// m_stSuccessMsg.SetWindowTextW(L"WardWiz 2015 has been successfully removed from your computer."); 
	 m_stSuccessMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString( L"IDS_UNINSTALLER_TXT_UNINSTL_SUCCESS_TXT"));
	 //m_stSuccessMsg.SetFont(&theApp.m_fontTextNormal);
	 //Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	 m_stSuccessMsg.SetFont(&theApp.m_FontArialFont);

	 //Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	 m_stSuccessMsg2.SetWindowPos(&wndTop,rect.left +150,rect.top +230,rect.Width(),50, SWP_NOREDRAW);
	 //m_stSuccessMsg2.SetWindowTextW(L"Please restart your computer to complete Uninstallation."); 
	 m_stSuccessMsg2.SetWindowTextW(theApp.m_objwardwizLangManager.GetString( L"IDS_UNINSTALLER_TXT_UNINSTL_RESTART_TXT"));
	 //m_stSuccessMsg2.SetFont(&theApp.m_fontTextNormal);
	 //Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	 m_stSuccessMsg2.SetFont(&theApp.m_FontArialFont);


	 m_bmpCompletedImg = LoadBitmap(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_CHK));
	 m_bmpFinishBMP.SetBitmap(m_bmpCompletedImg);
     m_bmpFinishBMP.SetWindowPos(&wndTop,rect.left+50,rect.top +190,60,60, SWP_NOREDRAW);

	//m_bmpTopLgo = LoadBitmap(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_CHK));
	//m_bmpWardwizLogo.SetBitmap(m_bmpTopLgo);
	//m_bmpWardwizLogo.SetWindowPos(&wndTop,rect.left,5,42,39, SWP_NOREDRAW);

	 //Varada Ikhar
	 LOGFONT lfInstallerTitle;
	 memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	 lfInstallerTitle.lfHeight = 15;
	 lfInstallerTitle.lfWeight = FW_BOLD;
	 lfInstallerTitle.lfWidth = 6;
	 m_BoldText.CreateFontIndirect(&lfInstallerTitle);
	 wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));	 //	   with	face name "Verdana".

	 m_stAllRightReserved.SetWindowPos(&wndTop,rect.left+25,rect.top +370,240,25, SWP_NOREDRAW);
 	// m_stAllRightReserved.SetWindowTextW(L"All rights reserved @ WardWiz 2015");
	 m_stAllRightReserved.SetWindowTextW(theApp.m_csAllRightReservedTxt);
	 //m_stAllRightReserved.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	 //Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	 m_stAllRightReserved.SetFont(&m_BoldText);

	 //Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed. 
	 m_stUninstallConfirmTitle.SetWindowPos(&wndTop,rect.left +390,rect.top +30,400 ,200, SWP_NOREDRAW);
	 //m_stUninstallConfirmTitle.SetWindowTextW(L"Uninstall Completed"); 
	 m_stUninstallConfirmTitle.SetWindowTextW(theApp.m_objwardwizLangManager.GetString( L"IDS_UNINSTALLER_TXT_UNINSTL_complete_TXT"));
	 m_stUninstallConfirmTitle.SetFont(&theApp.m_FontWWStartUpFontTitle); 

	// m_btnRestartNow.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	 //Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	 m_btnRestartNow.SetSkin(theApp.m_hResDLL, IDB_BITMAP_UNINSTALL_RESTARTNOW_HOVER, IDB_BITMAP_UNINSTALL_RESTARTNOW_HOVER, IDB_BITMAP_UNINSTALL_RESTART_NRM, IDB_BITMAP_UNINSTALL_RESTARTNOW_HOVER, 0, 0, 0, 0, 0);
	 m_btnRestartNow.SetWindowPos(&wndTop,rect.left+365,365,203,25, SWP_NOREDRAW);
	 m_btnRestartNow.SetTextColorA(RGB(0,0,0),1,1);
	// m_btnRestartNow.SetTextColorA(RGB(0,0,0),1,1);

	// m_btnRestartNow.SetFont(&theApp.m_fontWWTextNormal);
	//m_btnNextPage.SetWindowText(theApp.m_objwardwizLangManager.GetString( L"IDS_BUTTON_RESUME"));
	//m_btnNextPage.SetWindowText(L"Next");

	 //Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	 m_btRestartLetr.SetSkin(theApp.m_hResDLL, IDB_BITMAP_UNINSTALL_RESTARTLATER_NORMAL, IDB_BITMAP_UNINSTALL_RESTARTLATER_NORMAL, IDB_BITMAP_UNINSTALL_RESTARTLATER_HOVER, IDB_BITMAP_UNINSTALL_RESTARTLATER_NORMAL, 0, 0, 0, 0, 0); 
	// m_btRestartLetr.SetWindowPos(&wndTop,rect.left+580,340,150,25, SWP_NOREDRAW);
	 m_btRestartLetr.SetWindowPos(&wndTop,rect.left+580,365,152,25, SWP_NOREDRAW);
	 m_btRestartLetr.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_UNINSTALLER_RESTART_LATER"));
	 m_btRestartLetr.SetTextColorA(RGB(0,0,0),1,1);
     //m_btRestartLetr.SetFont(&theApp.m_fontWWTextNormal);


	 return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonRestartNow
*  Description    : Asks user for restart now
*  Author Name    :
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstThirdDlg::OnBnClickedButtonRestartNow()
{
	CEnumProcess enumproc;
	enumproc.RebootSystem(0);
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonRestartLater
*  Description    : Asks user for restart later
*  Author Name    :
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstThirdDlg::OnBnClickedButtonRestartLater()
{
	theApp.m_PtWWizUninstallerDlg->CloseApp();
}

/***************************************************************************
Function Name  : OnPaint
Description    : The framework calls this member function when Windows or an
application makes a request to repaint a portion of an application's window.
Author Name    : Ramkrushna Shelke
SR_NO			 :
Date           : 18th Nov 2013
****************************************************************************/
void CWWizUninstThirdDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	OnBackground(&theApp.m_bmpHeader02, &dc, 0, true);
	OnBackground(&theApp.m_bmpMiddleScndDlg, &dc, 180, true);
	OnBackground(&theApp.m_bmpMiddle01, &dc, 90, true);
	OnBackground(&theApp.m_bmpWWLogo, &dc, -10, false);
	OnBackground(&theApp.m_bmpFooter, &dc, 350, true);

	// TODO: Add your message handler code here
	// Do not call CJpegDialog::OnPaint() for painting messages
}

/***************************************************************************
Function Name  : OnCtlColor
Description    : The framework calls this member function when a child
control is about to be drawn.
Author Name    :
Date           : 18th Nov 2013
****************************************************************************/
HBRUSH CWWizUninstThirdDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int	ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();

	if(	ctrlID == IDC_STATIC_FINISH_SUCCESS_MSG ||
		ctrlID == IDC_STATIC_FINISH_SUCCESS_MSG2 ||
		ctrlID == IDC_STATIC_ALL_RIGHTS_RESERVED ||
		ctrlID == IDC_STATIC_UNINTALL_CONFM_TITLE
	//	ctrlID == IDC_STATIC_DES_USER_SETTING ||
		)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}	return hbr;
}

void CWWizUninstThirdDlg::OnBackground(CBitmap* bmpImg, CDC* pDC,int yHight,bool isStretch)
{
	CBitmap bmpObj;
	CRect rect;
	CDC visibleDC;
	//int iOldStretchBltMode;
	GetClientRect(&rect);
	visibleDC.CreateCompatibleDC(pDC);
	bmpObj.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	visibleDC.SelectObject(bmpImg);
	
	if(isStretch)
	{
		for(int i=0; i <=rect.Width()/5 ;i++)
		{
			//pDC->StretchBlt(i*5, 0, rect.Width(), rect.Height(), &srcDC, 0, 0, 1000,500, SRCCOPY);
			pDC->BitBlt(i*5, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
		}
	}
	else
	{
		pDC->BitBlt(0, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
	}

	bmpObj.DeleteObject();

	visibleDC.DeleteDC();

}
