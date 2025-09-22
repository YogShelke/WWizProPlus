// ISpyCommonTrayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISpyCommonTrayDlg.h"
#include "WardWizResource.h"

LPCTSTR g_OptionsForComboBox[2]={L"Repair",L"Quarantine"};
// CISpyCommonTrayDlg dialog



IMPLEMENT_DYNAMIC(CISpyCommonTrayDlg, CDialog)

CISpyCommonTrayDlg::CISpyCommonTrayDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyCommonTrayDlg::IDD, pParent)
	, m_pFontText(NULL)
	, m_pDetailText(NULL)
	, m_byteTranslucentLevel(DEFAULT_HUE) 
	, m_bIncreaseTranslucency(false)
	, m_bMakeTrans(false)
	,m_bLiveUpdateSucessfulMsg(false)
	,m_bLiveUpdateFailedMsg(false)
	,m_bEmailScanTreatMsg(false)
	,m_bTreatFoundMsg(false)
	, m_bExpiryDateMsg(false)
{

}

CISpyCommonTrayDlg::~CISpyCommonTrayDlg()
{
	if(m_pFontText != NULL)
	{
		delete m_pFontText;
		m_pFontText = NULL;
	}
	if(m_pDetailText != NULL)
	{
		delete m_pDetailText;
		m_pDetailText = NULL;
	}
}

void CISpyCommonTrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_LIVEUPDATEHEADER, m_stLiveUpdateheader);
	DDX_Control(pDX, IDC_STATIC_CHECKMARK, m_stCheckMark);
	DDX_Control(pDX, IDC_STATIC_SUCESS_UPDATE, m_stLiveupdateSuccess);
	DDX_Control(pDX, IDC_STATIC_EXCLAIMATION, m_stExClamationPic);
	DDX_Control(pDX, IDC_STATIC_FAILED, m_stLiveUpdateFailedMsg);
	DDX_Control(pDX, IDC_STATIC_INTERNETMSG, m_stInternetMsg);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClosetray);
	//DDX_Control(pDX, IDC_STATIC_TREATFOUNDFPIC, m_stTreatFoundPic);
	DDX_Control(pDX, IDC_STATIC_TREATNAME, m_stTreatName);
	DDX_Control(pDX, IDC_STATIC_ACTUAL_TREATNAME, m_stActualTreatName);
	DDX_Text(pDX, IDC_STATIC_ACTUAL_TREATNAME, m_csActualTreatName);
	DDX_Control(pDX, IDC_STATIC_FILEPATH, m_stFilePath);
	DDX_Control(pDX, IDC_STATIC_ACTUALPATH, m_stActualFilePath);
	DDX_Text(pDX, IDC_STATIC_ACTUALPATH, m_csActualFilePath);

	DDX_Control(pDX, IDC_STATIC_STATUS, m_stStatus);
	DDX_Control(pDX, IDC_STATIC_ACTUALSTATUS, m_stActualStatus);
	DDX_Text(pDX, IDC_STATIC_ACTUALSTATUS, m_csActualStatus);
	//	DDX_Control(pDX, IDC_STATIC_EMAILHEADR, m_stEmailHeader);
	DDX_Control(pDX, IDC_STATIC_SENDER_ADDR, m_stSenderAddr);
	DDX_Control(pDX, IDC_STATIC_ACTUALEMAILADDR, m_stActualEmailAddr);
	DDX_Text(pDX, IDC_STATIC_ACTUALEMAILADDR, m_csActualEmailAddr);

	DDX_Control(pDX, IDC_STATIC_ACTIONREQUIRED, m_stActionRequired);
	DDX_Control(pDX, IDC_COMBO_ACTION, m_comboActionReq);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_stTrayHeader);
//	DDX_Control(pDX, IDC_STATIC_EXCLAMATION, m_stNonUsedPictureControl);
}


BEGIN_MESSAGE_MAP(CISpyCommonTrayDlg, CJpegDialog)
	ON_BN_CLICKED(IDOK, &CISpyCommonTrayDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CISpyCommonTrayDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CISpyCommonTrayDlg::OnBnClickedButtonClose)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CISpyCommonTrayDlg message handlers

BOOL CISpyCommonTrayDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW ); 
	BringWindowToTop();
	SetForegroundWindow();
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_TRAY), _T("JPG")))
	{
		AddLogEntry(L"### Failed to load IDR_JPG_TRAY In CWardwizCommonTrayDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	
	Draw();

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	int ixRect = cxIcon - 316;
	int iyRect = cyIcon  - 175;
	
	CRect rect;
	this->GetClientRect(rect);
	
	try{
	APPBARDATA abd;
	abd.cbSize = sizeof(abd);

	SHAppBarMessage (ABM_GETTASKBARPOS, &abd);
	
   switch (abd.uEdge)
   {
      case ABE_TOP:
//			AfxMessageBox (L"Taskbar is at top");
			SetWindowPos(NULL,ixRect,i ,rect.Width(),rect.Height(),SWP_NOREDRAW);
           break;

      case ABE_BOTTOM:
//			AfxMessageBox (L"Taskbar is at bottom");
			SetWindowPos(NULL,ixRect,iyRect+20 ,rect.Width(),rect.Height(),SWP_NOREDRAW);
           break;

      case ABE_LEFT:
//			AfxMessageBox (L"Taskbar is at left");
			SetWindowPos(NULL,j,iyRect,rect.Width(),rect.Height(),SWP_NOREDRAW);
           break;

      case ABE_RIGHT:
  //         AfxMessageBox (L"Taskbar is at right");
		   SetWindowPos(NULL,ixRect ,iyRect,rect.Width(),rect.Height(),SWP_NOREDRAW);
           break;
   } 
	}
	catch(... ){
		
	}
	
	m_pFontText=NULL;
	m_pFontText = new(CFont);
	m_pFontText->CreateFont(15,6,0,0,FW_SEMIBOLD,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"verdana");

	m_pDetailText=NULL;
	m_pDetailText = new(CFont);
	m_pDetailText->CreateFont(13,6,0,0,FW_MEDIUM,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"verdana");
	// TODO:  Add extra initialization here
	m_bmpLiveUpdateheader = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_MSGICON));
	m_stLiveUpdateheader.SetBitmap(m_bmpLiveUpdateheader);
	m_stLiveUpdateheader.SetWindowPos(&wndTop,rect.left+40,50,42,39, SWP_NOREDRAW);

	m_stTrayHeader.SetWindowPos(&wndTop,rect.left+55,10,217,20, SWP_NOREDRAW);
	m_stTrayHeader.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stTrayHeader.SetBkColor(RGB(216, 217, 219));

	m_bmpCheckMark = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_REG_CORRECT));
	m_stCheckMark.SetBitmap(m_bmpCheckMark);
	m_stCheckMark.SetWindowPos(&wndTop,rect.left+20,70,62,60, SWP_NOREDRAW);


	m_bmpExClamationPic = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_ON_ACESS_HEADER));
	m_stExClamationPic.SetBitmap(m_bmpExClamationPic);
	m_stExClamationPic.SetWindowPos(&wndTop,rect.left+0,0,48,44, SWP_NOREDRAW);
	

	m_stLiveupdateSuccess.SetWindowPos(&wndTop,rect.left+100,80,177,55, SWP_NOREDRAW);
	m_stLiveupdateSuccess.SetFont(&theApp.m_fontWWTextNormal);
	//m_stLiveupdateSuccess.SetWindowTextW(L"Virus database updated successfully!");
	//m_stLiveupdateSuccess.SetBkColor(RGB(245,246,246));


	m_stLiveUpdateFailedMsg.SetWindowPos(&wndTop,rect.left+100,80,183,30, SWP_NOREDRAW);
	m_stLiveUpdateFailedMsg.SetFont(&theApp.m_fontWWTextNormal);
//	m_stLiveUpdateFailedMsg.SetWindowTextW(L"Virus database update failed");
	//m_stLiveUpdateFailedMsg.SetBkColor(RGB(245,246,246));


	m_stInternetMsg.SetWindowPos(&wndTop,rect.left+80,80,200,100, SWP_NOREDRAW);
	//m_stLiveupdateSuccess.SetFont(&theApp.m_fontWWTextNormal);
	m_stInternetMsg.SetFont(&theApp.m_fontWWTextNormal);
//	m_stInternetMsg.SetWindowTextW(L"Please check internet connection");

	m_btnClosetray.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnClosetray.SetWindowPos(&wndTop,rect.left+290,0,26,17, SWP_NOREDRAW);
	m_btnClosetray.ShowWindow(SW_SHOW);


	m_stTreatName.SetWindowPos(&wndTop,rect.left+40,140,80,15, SWP_NOREDRAW);
	//m_stTreatName.SetWindowTextW(L"Threat Name:");
	//m_stTreatName.SetBkColor(RGB(245,246,246));
	m_stTreatName.SetFont(&theApp.m_fontWWTextNormal);

	
	m_stActualTreatName.SetWindowPos(&wndTop,rect.left+130,140,180,15, SWP_NOREDRAW);
	//m_csActualTreatName.Format(L"%s",L"Virus.WIN32.Small3380.");
	m_stActualTreatName.SetWindowTextW(m_csActualTreatName);
	m_stActualTreatName.SetFont(&theApp.m_fontWWTextNormal);

	m_stFilePath.SetWindowPos(&wndTop,rect.left+40,120,60,15, SWP_NOREDRAW);
	//m_stFilePath.SetWindowTextW(L"File Path:");
	m_stFilePath.SetFont(&theApp.m_fontWWTextNormal);

	m_stActualFilePath.SetWindowPos(&wndTop,rect.left+130,120,160,15, SWP_NOREDRAW);
	//m_csActualFilePath.Format(L"%s",L"C:/ProgramFile/neha gHatge");
	m_stActualFilePath.SetWindowTextW(m_csActualFilePath);
	//m_stActualFilePath.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_FILE_PATH_DESCRIPTION"));
	m_stActualFilePath.SetFont(&theApp.m_fontWWTextNormal);

	m_stSenderAddr.SetWindowPos(&wndTop,rect.left+40,100,140,15, SWP_NOREDRAW);
	//m_stSenderAddr.SetWindowTextW(L"Sender Addr:");
	m_stSenderAddr.SetFont(&theApp.m_fontWWTextNormal);

	m_stActualEmailAddr.SetWindowPos(&wndTop,rect.left+130,100,180,15, SWP_NOREDRAW);
	//m_csActualEmailAddr.Format(L"%s",L"neha@yahoo.com");m_csActualEmailAddr
	//m_stActualEmailAddr.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_EMAIL_ID"));
	m_stActualEmailAddr.SetWindowTextW(m_csActualEmailAddr);
	//m_stActualEmailAddr.SetBkColor(RGB(245,246,246));
	m_stActualEmailAddr.SetFont(&theApp.m_fontWWTextNormal);

	m_stStatus.SetWindowPos(&wndTop,rect.left+40,100,80,15, SWP_NOREDRAW);
	//m_stSenderAddr.SetWindowTextW(L"Sender Addr:");
	//m_stStatus.SetBkColor(RGB(245,246,246));
	m_stStatus.SetFont(&theApp.m_fontWWTextNormal);
	//m_stStatus.ShowWindow(SW_SHOW);

	m_stActualStatus.SetWindowPos(&wndTop,rect.left+130,100,180,15, SWP_NOREDRAW);
	//m_csActualStatus.Format(L"%s",L"quarantined");
	m_stActualStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_QUARANTINED"));
	//m_stActualStatus.SetBkColor(RGB(245,246,246));
	m_stActualStatus.SetFont(&theApp.m_fontWWTextNormal);

	m_stActionRequired.SetWindowPos(&wndTop,rect.left+5,183,80,15, SWP_NOREDRAW);
	//m_stActionRequired.SetWindowTextW(L"Action Required:");
	//m_stActionRequired.SetBkColor(RGB(245,246,246));
	m_stActionRequired.SetFont(&theApp.m_fontWWTextNormal);

	//m_comboActionReq.AddString(g_OptionsForComboBox[0]);
	//m_comboActionReq.AddString(g_OptionsForComboBox[1]);
	//m_comboActionReq.SetCurSel(0);
	//m_comboActionReq.SetWindowPos(&wndTop,rect.left+100,183,80,10, SWP_NOREDRAW);
	m_comboActionReq.ShowWindow(SW_HIDE);
//	m_btnOk.SetSkin(IDB_BITMAP_BTNOK,IDB_BITMAP_BTNOK,IDB_BITMAP_BTNOK,IDB_BITMAP_BTNOK,0,0,0,0);
	//m_btnOk.SetWindowTextW(L"Ok");
	m_btnOk.SetTextColorA(RGB(0,0,0),1,1);
	m_btnOk.SetWindowPos(&wndTop, rect.left + 230, 143, 58, 21, SWP_NOREDRAW);
	m_btnOk.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_DISABLE_WHITE_BG, 0, 0, 0, 0, 0);

//	m_btnCancel.SetSkin(IDB_BITMAP_BTNOK,IDB_BITMAP_BTNOK,IDB_BITMAP_BTNOK,IDB_BITMAP_BTNOK,0,0,0,0);
	//m_btnCancel.SetWindowTextW(L"Cancel");
	m_btnCancel.SetTextColor(RGB(255,255,255));
	m_btnCancel.SetWindowPos(&wndTop,rect.left+160,210, 58,21, SWP_NOREDRAW);

	if(m_bLiveUpdateSucessfulMsg)
	{
		ShowControlForSucessLiveUpdate();
	}
	if(m_bLiveUpdateFailedMsg)
	{
		ShowControlForFailedLiveUpdate();
	}
	if(m_bEmailScanTreatMsg)
	{
		ShowControlForEmailScan();
	}
	if(m_bTreatFoundMsg)
	{
		ShowControlForTreatFound();
	}
	if (m_bExpiryDateMsg)
	{
		ShowControlForExpiryMsg();
	}

	SetTimer(TIMER_ID, INTERVAL, NULL);
	m_byteTranslucentLevel = DEFAULT_HUE;
	m_bMakeTrans = false;
	//MakeTranslucent();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CISpyCommonTrayDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString csbtnText;
	m_btnOk.GetWindowTextW(csbtnText);
	if (csbtnText == theApp.m_objwardwizLangManager.GetString(L"IDS_REGMSGPOPUP_BUTTON_BUY_NOW"))
	{
		ShellExecute(NULL, L"open", L"http://www.vibranium.co.in/", NULL, NULL, SW_SHOW);
	}
	KillTimer(KILL_UI_TIMER_ID);
	KillTimer(TIMER_ID);
	OnOK();
}

void CISpyCommonTrayDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CISpyCommonTrayDlg::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CISpyCommonTrayDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(m_bMakeTrans)
	{		
		MakeLighter();
	}
	else
	{
		MakeTranslucent();
	}
	// Timer kills UI if user not take any action for 30 sec
	//4/12/2015 Neha
	if (nIDEvent == KILL_UI_TIMER_ID)
	{
		SetTimer(TIMER_ID, INTERVAL, NULL);
	}
	
	CJpegDialog::OnTimer(nIDEvent);
}

bool CISpyCommonTrayDlg::MakeTranslucent(void)
{
	if (NeedMoreChange())
	{
		return true;
	}

	//WaitForSingleObject(m_hStopEvent, 5000);
	m_byteTranslucentLevel = DEFAULT_HUE;
	m_bMakeTrans = true;

	//if( NeedMoreChange() )
	//{
	//	//SetWindowLongPtr(m_hWnd, GWL_EXSTYLE,GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	//	//::SetLayeredWindowAttributes(m_hWnd, 255, m_byteTranslucentLevel, LWA_ALPHA);
	//	//m_bMakeTrans =false;
	//	return true;
	//}
	////Sleep(3000);
	//m_bMakeTrans = true;
	return false;
}


int CISpyCommonTrayDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
//	if (CJpegDialog::OnCreate(lpCreateStruct) == -1)
//		return -1;
		
//	SetTimer(TIMER_ID, INTERVAL , NULL); 
	
//	MakeTranslucent();
	//IncreaseTranslucency();
	GdiFlush();

	lpCreateStruct = NULL;	//just to appease the compiler
	return 0;

	// TODO:  Add your specialized creation code here

}

bool CISpyCommonTrayDlg::NeedMoreChange(void)
{
	bool bRet = false;

	if (m_byteTranslucentLevel >= DEFAULT_HUE)
	{
		bRet = false;
	}
	else
	{
		m_byteTranslucentLevel += INTERVAL;
		bRet = true;
	}
	return bRet;
}

void CISpyCommonTrayDlg::MakeLighter()
{
	//while( m_byteTranslucentLevel >=MIN_HUE)
	if(m_byteTranslucentLevel <= MIN_HUE)
	{
		KillTimer(TIMER_ID);
		KillTimer(KILL_UI_TIMER_ID);
		OnCancel();
		return;
	}
	else
	{
		m_byteTranslucentLevel -=INTERVAL;

		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE,GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	
		::SetLayeredWindowAttributes(m_hWnd, 255, m_byteTranslucentLevel, LWA_ALPHA);

		m_bMakeTrans = true;
	}

}

void CISpyCommonTrayDlg::MakeDarker()
{
	while( m_byteTranslucentLevel < MAX_HUE && m_bDecreaseTranslucency )
	{
		if( ::SetLayeredWindowAttributes( m_hWnd, 255, m_byteTranslucentLevel, LWA_ALPHA ) )
		{
			m_bDecreaseTranslucency = false;
			m_byteTranslucentLevel += INTERVAL;
		}
	}
}

void CISpyCommonTrayDlg::IncreaseTranslucency(void)
{
	//if it is completely transparent, nothing more to do
	if( m_byteTranslucentLevel == MIN_HUE )
	{
		KillTimer(TIMER_ID);
		OnCancel();
		return;
	}
	if( m_bIncreaseTranslucency == false )
		m_bIncreaseTranslucency = true;

	MakeLighter();

}

void CISpyCommonTrayDlg::ShowControlForSucessLiveUpdate()
{
	CRect rect;
	this->GetClientRect(rect);
	m_stExClamationPic.ShowWindow(SW_HIDE);
	
	m_stTrayHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_LIVE_UPDATE"));
	m_stTrayHeader.SetWindowPos(&wndTop,rect.left+15,13,200,20, SWP_NOREDRAW);
	m_stLiveUpdateFailedMsg.ShowWindow(SW_HIDE);
	m_stInternetMsg.ShowWindow(SW_HIDE);
	m_stTreatName.ShowWindow(SW_HIDE);
	m_stActualTreatName.ShowWindow(SW_HIDE);
	m_stFilePath.ShowWindow(SW_HIDE);
	m_stActualFilePath.ShowWindow(SW_HIDE);
	m_stSenderAddr.ShowWindow(SW_HIDE);
	m_stActualEmailAddr.ShowWindow(SW_HIDE);
	m_stStatus.ShowWindow(SW_HIDE);
	m_stActualStatus.ShowWindow(SW_HIDE);
	m_stActionRequired.ShowWindow(SW_HIDE);
//	m_comboActionReq.ShowWindow(SW_HIDE);
	m_btnOk.ShowWindow(SW_HIDE);
	m_btnCancel.ShowWindow(SW_HIDE);
	m_stLiveUpdateheader.ShowWindow(SW_HIDE);
	m_stCheckMark.ShowWindow(SW_SHOW);
	m_stLiveupdateSuccess.ShowWindow(SW_SHOW);
	m_stLiveupdateSuccess.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_DATABSE_UPDATE"));
	m_btnClosetray.ShowWindow(SW_SHOW);
}
void CISpyCommonTrayDlg::ShowControlForFailedLiveUpdate()
{
	CRect rect;
	this->GetClientRect(rect);
	m_stLiveUpdateFailedMsg.ShowWindow(FALSE);
	m_stExClamationPic.ShowWindow(SW_HIDE);
	m_stTrayHeader.SetWindowPos(&wndTop,rect.left+15,13,217,20, SWP_NOREDRAW);
	m_stTrayHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_LIVE_UPDATE"));
	m_stLiveUpdateheader.SetWindowPos(&wndTop,rect.left+20,90,42,39, SWP_NOREDRAW);
	m_stLiveUpdateheader.ShowWindow(TRUE);
	m_stCheckMark.ShowWindow(SW_HIDE);
	m_stLiveupdateSuccess.ShowWindow(SW_HIDE);
	m_stTreatName.ShowWindow(SW_HIDE);
	m_stActualTreatName.ShowWindow(SW_HIDE);
	m_stFilePath.ShowWindow(SW_HIDE);
	m_stActualFilePath.ShowWindow(SW_HIDE);
	m_stSenderAddr.ShowWindow(SW_HIDE);
	m_stActualEmailAddr.ShowWindow(SW_HIDE);
	m_stStatus.ShowWindow(SW_HIDE);
	m_stActualStatus.ShowWindow(SW_HIDE);
	m_stActionRequired.ShowWindow(SW_HIDE);
	//m_comboActionReq.ShowWindow(SW_HIDE);
	m_btnOk.ShowWindow(SW_HIDE);
	m_btnCancel.ShowWindow(SW_HIDE);
	m_stInternetMsg.ShowWindow(SW_SHOW);
	m_stInternetMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_FAILED_LIVE_UPDATE"));
	m_btnClosetray.ShowWindow(SW_SHOW);
}

void CISpyCommonTrayDlg::ShowControlForEmailScan()
{
	CRect rect;
	this->GetClientRect(rect);
	m_stExClamationPic.ShowWindow(SW_HIDE);
	m_stTrayHeader.SetWindowPos(&wndTop,rect.left+15,13,217,20, SWP_NOREDRAW);
	m_stCheckMark.ShowWindow(SW_HIDE);
	m_stLiveupdateSuccess.ShowWindow(SW_HIDE);
	m_stLiveUpdateFailedMsg.ShowWindow(SW_HIDE);
	m_stInternetMsg.ShowWindow(SW_HIDE);
	m_btnClosetray.ShowWindow(SW_SHOW);
	m_stTreatName.SetWindowPos(&wndTop,rect.left+40,140,80,15, SWP_NOREDRAW);
	m_stTreatName.ShowWindow(SW_SHOW);
	m_stActualTreatName.ShowWindow(SW_SHOW);
	m_stActualTreatName.SetWindowPos(&wndTop,rect.left+130,140,180,15, SWP_NOREDRAW);
	m_stFilePath.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_ATTACH_NAME"));
	m_stTreatName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_NAME"));
	m_stFilePath.SetWindowPos(&wndTop,rect.left+40,120,60,15, SWP_NOREDRAW);
	m_stFilePath.ShowWindow(SW_SHOW);
	m_stActualFilePath.SetWindowPos(&wndTop,rect.left+130,120,160,15, SWP_NOREDRAW);
	m_stActualFilePath.ShowWindow(SW_SHOW);
	m_stSenderAddr.ShowWindow(SW_SHOW);
	m_stActualEmailAddr.ShowWindow(SW_SHOW);
	m_stStatus.ShowWindow(SW_HIDE);
	m_stActualStatus.SetWindowPos(&wndTop,rect.left + 100,150,100,40, SWP_NOREDRAW);
	m_stActualStatus.ShowWindow(SW_HIDE);
	m_stActionRequired.ShowWindow(SW_SHOW);
	m_stActionRequired.SetWindowPos(&wndTop,rect.left + 100,60,100,40, SWP_NOREDRAW);
	m_stActionRequired.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_FOUND"));
	m_stActionRequired.SetFont(&theApp.m_fontWWTextNormal);
	//m_comboActionReq.ShowWindow(SW_HIDE);
	m_btnOk.ShowWindow(SW_HIDE);
	m_btnCancel.ShowWindow(SW_HIDE);
	m_stLiveUpdateheader.ShowWindow(TRUE);
	m_stTrayHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_EMAIL_SCAN"));
	m_stSenderAddr.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_SENDER_ADDR"));
	
}

void CISpyCommonTrayDlg::ShowControlForTreatFound()
{
	CRect rect;
	this->GetClientRect(rect);
	m_stTreatName.SetWindowPos(&wndTop,rect.left+40,90,80,15, SWP_NOREDRAW);
	m_stActualTreatName.SetWindowPos(&wndTop,rect.left+130,90,180,15, SWP_NOREDRAW);
	m_stStatus.SetWindowPos(&wndTop,rect.left+40,130,80,15, SWP_NOREDRAW);
	m_stActualStatus.SetWindowPos(&wndTop,rect.left+130,130,180,15, SWP_NOREDRAW);
	m_stFilePath.SetWindowPos(&wndTop,rect.left+40,110,60,15, SWP_NOREDRAW);
	m_stActualFilePath.SetWindowPos(&wndTop,rect.left+130,110,160,15, SWP_NOREDRAW);
	
	
	m_stTrayHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_FOUND"));
	m_stTreatName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_NAME"));
	m_stStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_STATUS"));
	m_stLiveUpdateheader.ShowWindow(SW_HIDE);
	m_stCheckMark.ShowWindow(SW_HIDE);
	m_stLiveupdateSuccess.ShowWindow(SW_HIDE);
	m_stLiveUpdateFailedMsg.ShowWindow(SW_HIDE);
	m_stInternetMsg.ShowWindow(SW_HIDE);
	m_btnClosetray.ShowWindow(SW_SHOW);
	m_stTreatName.ShowWindow(SW_SHOW);
	m_stActualTreatName.ShowWindow(SW_SHOW);
	m_stFilePath.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_FILE_PATH"));
	m_stFilePath.ShowWindow(SW_SHOW);
	m_stActualFilePath.ShowWindow(SW_SHOW);
	m_stSenderAddr.ShowWindow(SW_HIDE);
	m_stActualEmailAddr.ShowWindow(SW_HIDE);
	m_stStatus.ShowWindow(SW_SHOW);
	m_stActualStatus.ShowWindow(SW_SHOW);
	m_stActionRequired.ShowWindow(SW_HIDE);
	//m_comboActionReq.ShowWindow(SW_HIDE);
	m_btnOk.ShowWindow(SW_HIDE);
	m_btnCancel.ShowWindow(SW_HIDE);
}

HBRUSH CISpyCommonTrayDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
 HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
 int ctrlID;
 ctrlID = pWnd->GetDlgCtrlID();
 if(	ctrlID == IDC_STATIC_TREATNAME      || 
		ctrlID == IDC_STATIC_HEADER 		||
		ctrlID == IDC_STATIC_ACTUAL_TREATNAME   ||
		ctrlID == IDC_STATIC_ACTUALSTATUS  ||
		ctrlID == IDC_STATIC_STATUS  ||
		ctrlID == IDC_STATIC_INTERNETMSG  ||
		ctrlID == IDC_STATIC_FAILED  ||
		ctrlID == IDC_STATIC_SENDER_ADDR  ||
		ctrlID == IDC_STATIC_SUCESS_UPDATE  ||
		ctrlID == IDC_STATIC_ACTUALEMAILADDR  ||
		ctrlID == IDC_STATIC_FILEPATH  ||
		ctrlID == IDC_STATIC_ACTUALPATH  ||
		ctrlID == IDC_STATIC_ACTIONREQUIRED  
     )
  
 {
  pDC->SetBkMode(TRANSPARENT);
  hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
 } return hbr;
}

void CISpyCommonTrayDlg::RefreshStrings()
{
	m_stLiveupdateSuccess.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_DATABSE_UPDATE"));
	m_stLiveUpdateFailedMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_FAILED_LIVE_UPDATE"));
	m_stInternetMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_FAILED_LIVE_UPDATE"));
	m_stTreatName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_NAME"));
	m_stFilePath.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_FILE_PATH"));
	m_stSenderAddr.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_SENDER_ADDR"));
	m_stActionRequired.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_TRAY_ACTION"));
	m_stTrayHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_THREAT_FOUND"));
	m_stFilePath.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_ATTACH_NAME "));
	m_stFilePath.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_FILE_PATH"));
}


int CISpyCommonTrayDlg::GetTaskBarHeight() 
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

int CISpyCommonTrayDlg::GetTaskBarWidth() 
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
LRESULT CISpyCommonTrayDlg::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	return HTCLIENT;
	//return CJpegDialog::OnNcHitTest(point);
}

BOOL CISpyCommonTrayDlg::PreTranslateMessage(MSG* pMsg)
{

	if( pMsg->wParam == VK_ESCAPE )
		return TRUE;

	return CJpegDialog::PreTranslateMessage(pMsg);
}


void CISpyCommonTrayDlg::ShowControlForExpiryMsg()
{
	CRect rect;
	this->GetClientRect(rect);
	

	m_stTrayHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_REGISTRATION_EXPIRED_HEADER"));
	m_stTrayHeader.SetWindowPos(&wndTop, rect.left + 15, 13, 200, 20, SWP_NOREDRAW);
	m_stLiveUpdateFailedMsg.ShowWindow(SW_HIDE);
	m_stInternetMsg.ShowWindow(SW_HIDE);
	m_stTreatName.ShowWindow(SW_HIDE);
	m_stActualTreatName.ShowWindow(SW_HIDE);
	m_stFilePath.ShowWindow(SW_HIDE);
	m_stActualFilePath.ShowWindow(SW_HIDE);
	m_stSenderAddr.ShowWindow(SW_HIDE);
	m_stActualEmailAddr.ShowWindow(SW_HIDE);
	m_stStatus.ShowWindow(SW_HIDE);
	m_stActualStatus.ShowWindow(SW_HIDE);
	m_stActionRequired.ShowWindow(SW_HIDE);
	//	m_comboActionReq.ShowWindow(SW_HIDE);
	m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REGMSGPOPUP_BUTTON_BUY_NOW"));
	m_btnOk.ShowWindow(SW_SHOW);
	m_btnCancel.ShowWindow(SW_HIDE);
	m_stLiveUpdateheader.ShowWindow(SW_SHOW);
	m_stLiveUpdateheader.SetWindowPos(&wndTop, rect.left + 20, 70, 62, 60, SWP_NOREDRAW);
	m_stCheckMark.ShowWindow(SW_HIDE);
	m_stExClamationPic.ShowWindow(SW_HIDE);
	m_stLiveupdateSuccess.ShowWindow(SW_SHOW);
	m_stLiveupdateSuccess.SetWindowPos(&wndTop, rect.left + 110, 80, 177, 55, SWP_NOREDRAW);
	m_stLiveupdateSuccess.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TRAY_REGISTRATION_EXPIRED_MSG"));
	m_btnClosetray.ShowWindow(SW_SHOW);
}

void CISpyCommonTrayDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	KillTimer(TIMER_ID);
	m_byteTranslucentLevel = MAX_HUE;
	::SetLayeredWindowAttributes(m_hWnd, 255, m_byteTranslucentLevel, LWA_ALPHA);
	SetTimer(KILL_UI_TIMER_ID, 4000, NULL);
	m_byteTranslucentLevel = DEFAULT_HUE;
	CJpegDialog::OnMouseMove(nFlags, point);
}


