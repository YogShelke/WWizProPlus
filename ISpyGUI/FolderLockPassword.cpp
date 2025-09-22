/**********************************************************************************************************                     
	  Program Name          : FolderLockPassword.cpp
	  Description           : Folder locker password dialog
	  Author Name			: Neha Gharge                                                                                           
	  Date Of Creation      : 15th Jan 2014
	  Version No            : 1.0.0.2
	  Special Logic Used    : 

	  Modification Log      :           
	  1.            
***********************************************************************************************************/
// FolderLockPassword.cpp : implementation file
//

#include "stdafx.h"
#include "FolderLockPassword.h"
#include "ISpyGUI.h"
#include "WinHttpClient.h"
#include "ISpyFolderLocker.h"

AVACTIVATIONINFO	g_ActInfoNew = {0} ;

// CFolderLockPassword dialog
CString		g_csFolderLockerPassword;
#define		IDR_RESDATA_DES			7000
#define		IDR_REGDATA				2000
//#define		WRDWIZ_DATA_ENC_KEY		0x49546176
// CFolderLockPassword dialog

bool bFOLDER_DES = false ;
typedef DWORD (*GETREGISTEREDFOLDERDDATA) (LPBYTE lpResBuffer, DWORD &dwResDataSize, DWORD dwResType, TCHAR *pResName ) ;
GETREGISTEREDFOLDERDDATA	GetRegisteredFolderData = NULL ;

/*typedef bool (*SENDPASSWORDTOEMAILID)(LPTSTR szRecptAddress, LPTSTR szPassword);
SENDPASSWORDTOEMAILID SendPassword2EmailID	 = NULL */;

DWORD WINAPI StartThread(LPVOID arg);

IMPLEMENT_DYNAMIC(CFolderLockPassword, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CFolderLockPassword                                                     
*  Description    :	C'tor
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
CFolderLockPassword::CFolderLockPassword(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CFolderLockPassword::IDD, pParent)
	,m_dwNoofTypingChar(0)
	, m_stForgotPasswordlink(theApp.m_hResDLL)
{
	memset(&m_ActInfo,0x00,sizeof(m_ActInfo));
}

/**********************************************************************************************************                     
*  Function Name  :	~CISpyFolderLocker                                                    
*  Description    :	D'tor
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
CFolderLockPassword::~CFolderLockPassword()
{
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                  
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CFolderLockPassword::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FOLDER_PASS_LOCK, m_stFolderPassLockText);
	DDX_Control(pDX, IDC_STATIC_FOLDER_PASS_UNLOCK_TEXT, m_stFolderPassUnlockText);
	DDX_Control(pDX, IDC_STATIC_LOCK_IMAGE, m_stFolderPassLockPic);
	DDX_Control(pDX, IDC_STATIC_FOLDER_PASS_UNLOCK_PIC, m_stFolderPassUnlockPic);
	DDX_Control(pDX, IDOK, m_btnFolderPassOk);
	DDX_Control(pDX, IDCANCEL, m_btnFolderPassCancel);
	DDX_Control(pDX, IDC_STATIC_FILEISLOCKED, m_stFileIsLocked);
	DDX_Control(pDX, IDC_BUTTON_PASS_MINIMIZE, m_btnPassMinimize);
	DDX_Control(pDX, IDC_BUTTON_PASS_CLOSE, m_btnPassClose);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edtFolderPassword);
	//	DDX_Control(pDX, IDC_EDIT_FOLDER_EMAILID, m_edtFolderEmailID);
	DDX_Control(pDX, IDC_STATIC_LINK_FORGOT_PASSWORD, m_stForgotPasswordlink);
	DDX_Control(pDX, IDC_STATIC_FOLDER_EMAILID, m_stEmailIconImage);
	DDX_Control(pDX, IDC_BUTTON_SEND_BUTTON, m_btnSendPassword2EmailID);
	DDX_Control(pDX, IDC_BUTTON_FOLDER_PASS_BACK, m_btnFolderpassBack);
	DDX_Control(pDX, IDC_STATIC_FOLDERPASS_PRELOADER, m_stFolderPassPreloader);
	DDX_Control(pDX, IDC_STATIC_EMAIL_ID, m_stEmailID);
	DDX_Control(pDX, IDC_STATIC_SENDING_MAIL, m_stSendingMail);
	DDX_Control(pDX, IDC_STATIC_PASSWORD_NOTSENT_MSG, m_stNotSentMsg);
	DDX_Control(pDX, IDC_STATIC_PASSWORD_SENT_MSG, m_stPasswordSentMsg);
	DDX_Control(pDX, IDC_EDIT_RENTER_PASSWORD, m_EditRenterPassword);
	DDX_Control(pDX, IDC_STATIC_RENTER_PASSWORD, m_stRenterPassword);
	DDX_Control(pDX, IDC_STATIC_LOCK_MSG, m_stFolderLockText);
}

/***************************************************************************
* Function Name  : MESSAGE_MAP
* Description    : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
*  Author Name    : Neha Gharge    
*  SR_NO		  :
*  Date           : 11 Jan 2014
****************************************************************************/
BEGIN_MESSAGE_MAP(CFolderLockPassword, CJpegDialog)
	ON_WM_SETCURSOR()
	ON_MESSAGE(_HYPERLINK_EVENT,OnChildFire)
	ON_BN_CLICKED(IDC_BUTTON_PASS_MINIMIZE, &CFolderLockPassword::OnBnClickedButtonPassMinimize)
	ON_BN_CLICKED(IDC_BUTTON_PASS_CLOSE, &CFolderLockPassword::OnBnClickedButtonPassClose)
	ON_BN_CLICKED(IDOK, &CFolderLockPassword::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFolderLockPassword::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EDIT_PASSWORD, &CFolderLockPassword::OnEnChangeEditPassword)
	ON_BN_CLICKED(IDC_BUTTON_SEND_BUTTON, &CFolderLockPassword::OnBnClickedButtonSendButton)
	ON_BN_CLICKED(IDC_BUTTON_FOLDER_PASS_BACK, &CFolderLockPassword::OnBnClickedButtonFolderPassBack)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_EDIT_RENTER_PASSWORD, &CFolderLockPassword::OnEnChangeEditRenterPassword)
END_MESSAGE_MAP()


// CFolderLockPassword message handlers

/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.NO          : 
*  Author Name    : Amit                                                                                          
*  Date           : may 2014
* Modified		  :	Rajil yadav
**********************************************************************************************************/
BOOL CFolderLockPassword::OnInitDialog()
{
	CJpegDialog::OnInitDialog();


	/*	ISSUE NO - 865 NAME - NITIN K. TIME - 28th June 2014 */
	if(m_bSelectComboOption == true)
	{
		if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_PASSWORD_DATA_FOLDER), _T("JPG")))
		{
			::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_FAILMSG")
				, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		}
	}
	else
	{
		if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_PASSWORD_BG), _T("JPG")))
		{
			::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_FAILMSG")
				, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		}
	}
	Draw();

	CRect rect;
	this->GetClientRect(rect);
	//SetWindowPos(&wndTop, rect.top + 430,rect.top + 260, rect.Width()-2, rect.Height()-2, SWP_NOREDRAW);

	
	m_stFolderLockText.SetWindowPos(&wndTop,rect.left + 20,60,350,42,SWP_NOREDRAW);
	m_stFolderLockText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_FOLDERLOCK_TEXT"));
	m_stFolderLockText.SetFont(&theApp.m_fontWWTextNormal);
	m_stFolderLockText.SetBkColor(RGB(255,255,255));




	// ISSUE NO : 428 Neha gharge 26/5/2014*******************************************/
	CRgn		 rgn;
	rgn.CreateRoundRectRgn(rect.left, rect.top, rect.right - 2, rect.bottom - 2,0,0);
	this->SetWindowRgn(rgn, TRUE);

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	if (m_stFolderPassPreloader.Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_GIF_32x32_PRELOADER),_T("GIF")))
	{
		if(!m_stFolderPassPreloader.IsPlaying())
		{
			m_stFolderPassPreloader.Draw();
		}
	}

	if(m_stFolderPassPreloader.IsPlaying())
	{
		m_stFolderPassPreloader.Stop();
	}
	//m_stFolderPassPreloader.SetWindowPos(&wndTop,rect.left+180,65,32,32, SWP_NOREDRAW);
	m_stFolderPassPreloader.SetWindowPos(&wndTop,rect.left+170,65,32,32, SWP_NOREDRAW);
	m_stFolderPassPreloader.ShowWindow(SW_HIDE);
	m_stFolderPassPreloader.SetBkColor(RGB(255,255,255));

	m_bmpFolderPassLockPic = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_LOCK));
	m_stFolderPassLockPic.SetWindowPos(&wndTop,rect.left +20,110,52,60, SWP_NOREDRAW);
	m_stFolderPassLockPic.SetBitmap(m_bmpFolderPassLockPic);
	
	m_bmpFolderPassUnlockPic = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_UNLOCK));
	m_stFolderPassUnlockPic.SetWindowPos(&wndTop,rect.left +20,70,52,60, SWP_NOREDRAW);
	m_stFolderPassUnlockPic.SetBitmap(m_bmpFolderPassUnlockPic);

	
	m_bmpFolderEmailIconPic = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_FOLDEREMAIL_ICON));
	m_stEmailIconImage.SetWindowPos(&wndTop,rect.left +20,70,52,60, SWP_NOREDRAW);
    m_stEmailIconImage.SetBitmap(m_bmpFolderEmailIconPic);
	m_stEmailIconImage.ShowWindow(SW_HIDE);
	
	m_stFolderPassLockText.SetWindowPos(&wndTop,rect.left +87,100,250,20, SWP_NOREDRAW);
	m_stFolderPassLockText.SetFont(&theApp.m_fontWWTextNormal);
	m_stFolderPassLockText.SetBkColor(RGB(255,255,255));
	

	m_stRenterPassword.SetWindowPos(&wndTop,rect.left +87,150,250,20, SWP_NOREDRAW);
	m_stRenterPassword.SetFont(&theApp.m_fontWWTextNormal);
	m_stRenterPassword.SetBkColor(RGB(255,255,255));
	m_stRenterPassword.SetWindowTextW(L"Please re-enter password");
	m_stRenterPassword.ShowWindow(FALSE);


	m_stFolderPassUnlockText.SetWindowPos(&wndTop,rect.left +87,85,280,20, SWP_NOREDRAW);
	m_stFolderPassUnlockText.SetFont(&theApp.m_fontWWTextNormal);
	m_stFolderPassUnlockText.SetBkColor(RGB(255,255,255));
	

	m_stFileIsLocked.SetWindowPos(&wndTop,rect.left +87,65,150,20, SWP_NOREDRAW);
	m_stFileIsLocked.SetFont(&theApp.m_fontWWTextNormal);
	m_stFileIsLocked.SetBkColor(RGB(255,255,255));

	m_btnFolderPassOk.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnFolderPassOk.SetWindowPos(&wndTop,rect.left + 233,220,57,21,SWP_NOREDRAW);
    m_btnFolderPassOk.SetFont(&theApp.m_fontWWTextNormal);
	//m_btnFolderPassOk.SetWindowTextW(L"Ok");
	m_btnFolderPassOk.SetTextColorA(BLACK,1,1);
	//m_btnFolderPassOk.ShowWindow(SW_HIDE);

	m_btnFolderPassCancel.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnFolderPassCancel.SetWindowPos(&wndTop,rect.left + 300,220,57,21,SWP_NOREDRAW);
	//m_btnFolderPassCancel.SetWindowTextW(L"Cancel");
    m_btnFolderPassCancel.SetFont(&theApp.m_fontWWTextNormal);
	m_btnFolderPassCancel.SetTextColorA(BLACK,1,1);

	m_btnSendPassword2EmailID.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnSendPassword2EmailID.SetWindowPos(&wndTop,rect.left + 230,165,57,21,SWP_NOREDRAW);
	//m_btnSendPassword2EmailID.SetWindowTextW(L"Send");
    m_btnSendPassword2EmailID.SetFont(&theApp.m_fontWWTextNormal);
	m_btnSendPassword2EmailID.SetTextColorA(BLACK,1,1);
	m_btnSendPassword2EmailID.ShowWindow(SW_HIDE);

	m_btnFolderpassBack.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnFolderpassBack.SetWindowPos(&wndTop,rect.left +295,165,57,21,SWP_NOREDRAW);
	//m_btnFolderpassBack.SetWindowTextW(L"Back");
	m_btnFolderpassBack.SetFont(&theApp.m_fontWWTextNormal);
	m_btnFolderpassBack.SetTextColorA(BLACK,1,1);
	m_btnFolderpassBack.ShowWindow(SW_HIDE);

	m_stForgotPasswordlink.SetWindowPos(&wndTop,rect.left +87,165,150,21,SWP_NOREDRAW);
	//m_stForgotPasswordlink.ActiveToolTip(1);
	m_stForgotPasswordlink.SetFont(&theApp.m_fontWWTextNormal);
	//m_stForgotPasswordlink.SetTootTipText(L"An e-mail will sent to you that contains your password.");
    //m_stForgotPasswordlink.SetFont(&theApp.m_fontWWTextNormal);
	//m_stForgotPasswordlink.SetToolTipTextColor(RGB(0, 255, 0));
	//m_stForgotPasswordlink.SetLinkColor(RGB(0, 0, 255));
	//m_stForgotPasswordlink.SetHoverColor(RGB(255, 0, 0));
	m_stForgotPasswordlink.SetFireChild(1);

	/*	ISSUE NO - 172 NAME - NITIN K. TIME - 23st May 2014 */
	m_btnPassMinimize.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnPassMinimize.SetWindowPos(&wndTop,rect.left +350,0,30,17,SWP_NOREDRAW);
	m_btnPassMinimize.ShowWindow(FALSE);


	m_btnPassClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnPassClose.SetWindowPos(&wndTop,rect.left +350,0,30,17,SWP_NOREDRAW);
	
	m_csEmailID = L"";
	g_csFolderLockerPassword = "" ;
	



	m_stEmailID.SetWindowPos(&wndTop,rect.left +87,115,270,20, SWP_NOREDRAW);
	m_stEmailID.ShowWindow(FALSE);
	m_stEmailID.SetBkColor(RGB(255,255,255));
	m_stEmailID.SetFont(&theApp.m_fontWWTextNormal);

	m_stSendingMail.SetWindowPos(&wndTop,rect.left +142,120,200,20, SWP_NOREDRAW);
	m_stSendingMail.ShowWindow(FALSE);
	m_stSendingMail.SetBkColor(RGB(255,255,255));
	m_stSendingMail.SetFont(&theApp.m_fontWWTextNormal);

	m_stNotSentMsg.SetWindowPos(&wndTop,rect.left +67,115,270,20, SWP_NOREDRAW);
	m_stNotSentMsg.ShowWindow(FALSE);
	m_stNotSentMsg.SetBkColor(RGB(255,255,255));
	m_stNotSentMsg.SetFont(&theApp.m_fontWWTextNormal);
	m_stNotSentMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLODERLOCKER_NOTSENT_MSG"));

	m_stPasswordSentMsg.SetWindowPos(&wndTop,rect.left +67,115,270,20, SWP_NOREDRAW);
	m_stPasswordSentMsg.ShowWindow(FALSE);
	m_stPasswordSentMsg.SetBkColor(RGB(255,255,255));
	m_stPasswordSentMsg.SetFont(&theApp.m_fontWWTextNormal);
	m_stPasswordSentMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLODERLOCKER_SENT_MSG"));

	
	
	m_EditRenterPassword.SetWindowTextW( TEXT("") ) ;
	m_EditRenterPassword.SetWindowPos(&wndTop,rect.left +87,170,270,20, SWP_NOREDRAW);


	m_edtFolderPassword.SetWindowTextW( TEXT("") ) ;
	m_edtFolderPassword.SetWindowPos(&wndTop,rect.left +87,120,270,20, SWP_NOREDRAW);
	m_edtFolderPassword.SetFocus();

	if(m_bSelectComboOption == true)
	{
		m_stFolderPassUnlockPic.ShowWindow(SW_HIDE);
		m_stFolderPassUnlockText.ShowWindow(SW_HIDE);
		m_stFileIsLocked.ShowWindow(SW_HIDE);
		m_stFolderPassLockPic.ShowWindow(SW_SHOW);
		m_stFolderPassLockText.ShowWindow(SW_SHOW);
		m_stEmailIconImage.ShowWindow(SW_HIDE);
		m_stForgotPasswordlink.ShowWindow(SW_HIDE);
		m_stRenterPassword.ShowWindow(true);
		m_edtFolderPassword.ShowWindow(true);
		m_EditRenterPassword.ShowWindow(true);
	}
	else
	{
		m_stFolderLockText.ShowWindow(SW_HIDE);
		m_stEmailIconImage.ShowWindow(SW_HIDE);
		m_stFolderPassLockPic.ShowWindow(SW_HIDE);
		m_stFolderPassLockText.ShowWindow(SW_HIDE);
		m_stFolderPassUnlockPic.ShowWindow(SW_SHOW);
		m_stFolderPassUnlockText.ShowWindow(SW_SHOW);
		m_stFileIsLocked.ShowWindow(SW_SHOW);
		m_stForgotPasswordlink.ShowWindow(SW_SHOW);
		m_stRenterPassword.ShowWindow(false);
		m_EditRenterPassword.ShowWindow(false);
		m_btnFolderPassOk.SetWindowPos(&wndTop,rect.left + 230,165,57,21,SWP_NOREDRAW);
		m_btnFolderPassCancel.SetWindowPos(&wndTop,rect.left + 295,165,57,21,SWP_NOREDRAW);
		m_edtFolderPassword.SetWindowPos(&wndTop,rect.left +87,115,270,20, SWP_SHOWWINDOW);
		m_edtFolderPassword.SetFocus();
	}

	//m_edtFolderPassword.SetLimitText(7);
	m_edtFolderPassword.SetFocus( ) ;
	RefreshStrings();
	m_dwNoofTypingChar = 0;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage                                                     
*  Description    : To translate window messages before they are dispatched 
				    to the TranslateMessage and DispatchMessage Windows functions
*  SR.N0		  : 
*  Author Name    :	Neha Gharge                                                                                   
*  Date           : 11 Jan 2014
**********************************************************************************************************/
BOOL CFolderLockPassword::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	/*	ISSUE NO - 667 NAME - NITIN K. TIME - 11th June 2014 */
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonPassMinimize                                                     
*  Description    : Folder locker password gets minimized.
*  SR.N0		  : 
*  Author Name    :	Nitin                                                                                   
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CFolderLockPassword::OnBnClickedButtonPassMinimize()
{
	// TODO: Add your control notification handler code here
	OnClose();
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonPassClose                                                     
*  Description    : Close folder locker password dialog.
*  SR.N0		  : 
*  Author Name    :	Nitin                                                                                   
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CFolderLockPassword::OnBnClickedButtonPassClose()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedOk                                                     
*  Description    :	After clicking on OK button, It will check for validate password.
*  SR.N0		  : 
*  Author Name    : Nitin                                                                                         
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CFolderLockPassword::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	TCHAR		szPass[0x10] = {0} ;

	CString		strPass("") ;
	TCHAR		szPassRenter[0x10] = {0};

	m_edtFolderPassword.GetWindowTextW( szPass, 15 ) ;
	m_EditRenterPassword.GetWindowTextW(szPassRenter,15);
	if( !szPass[0] )
	{
		m_dwNoofTypingChar = 0;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_PSSWRD"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		return ;
	}

	if((m_bSelectComboOption == true) && _tcscmp(szPass,szPassRenter))
	{
		MessageBox(L"Please enter same password",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONEXCLAMATION);
		m_edtFolderPassword.SetWindowTextW(L"");
		m_EditRenterPassword.SetWindowTextW(L"");
		return;
	}


	if( szPass[0] )
	{
		strPass.Format( TEXT("%s"), szPass ) ;
		if( !ValidatePassword( strPass ) )
		{
			g_csFolderLockerPassword = strPass ;
			m_dwNoofTypingChar = 0;
			OnOK();
		}
		else
		{
			g_csFolderLockerPassword = strPass  = "" ;
			m_edtFolderPassword.SetWindowTextW( TEXT("") ) ;
			m_edtFolderPassword.SetFocus( ) ;
			m_EditRenterPassword.SetWindowTextW( TEXT("") ) ;
			m_dwNoofTypingChar = 0;
		}
	}
	else
	{
		g_csFolderLockerPassword = strPass  = "" ;
		m_edtFolderPassword.SetWindowTextW( TEXT("") ) ;
		m_edtFolderPassword.SetFocus( ) ;
		m_EditRenterPassword.SetWindowTextW( TEXT("") ) ;
		m_dwNoofTypingChar = 0;
	}

}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCancel                                                     
*  Description    :	Close folder locker password dialog
*  SR.N0		  : 
*  Author Name    : Nitin                                                                                         
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CFolderLockPassword::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

/**********************************************************************************************************                     
*  Function Name  :	ValidatePassword                                                     
*  Description    :	Check the valid password, whether it contains atlst 1 no,1 special char,password length.
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/

DWORD CFolderLockPassword::ValidatePassword(CString csPassword)
{
	int			i,count=0 ;
	CString		csInputKey ;
	csInputKey = csPassword;

	int		Validlenght = csInputKey.GetLength();

	m_dwNoofTypingChar = 0;
	if(Validlenght==0)
	{
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_PSSWRD"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
		//PopUpDialog();
		return 1 ;
	}
	if(m_bSelectComboOption == true)
	{
		if(Validlenght<=4)
		{
			
			::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALID4"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			m_edtFolderPassword.SetWindowTextW( TEXT("") ) ;
			m_EditRenterPassword.SetWindowTextW( TEXT("") ) ;
			//PopUpDialog();
			return 2 ;
		}
		if(Validlenght>=8)
		{
			
			::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALID8"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			m_EditRenterPassword.SetWindowTextW( TEXT("") ) ;
			m_edtFolderPassword.SetWindowTextW( TEXT("") ) ;
			//PopUpDialog();
			return 3 ;
		}
		if((csInputKey.FindOneOf(L"~`!@#$%^&*()_-+={}[]|\?/.,':;<>")==-1))
		{
			
			::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALIDSPL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			m_EditRenterPassword.SetWindowTextW( TEXT("") ) ;
			m_edtFolderPassword.SetWindowTextW( TEXT("") ) ;
			//PopUpDialog();
			return 4 ;
		}

		for(i=0;i<Validlenght;i++)
		{
			if((isdigit(csInputKey[i])))
			{
				count++ ;
			}
		}
		if( count<=0 )
		{
			m_edtFolderPassword.SetWindowTextW( TEXT("") ) ;
			m_EditRenterPassword.SetWindowTextW( TEXT("") ) ;
			::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALIDNUM"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			//PopUpDialog();
			return 5 ;
		}
	}
	/*else 
	{
		MessageBox(L"Invalid Password\nPlease try again",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
		return 2;
	}*/

return 0 ;
}

/**********************************************************************************************************                     
*  Function Name  :	OnEnChangeEditPassword                                                     
*  Description    :	Edit box for folder locker password
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CFolderLockPassword::OnEnChangeEditPassword()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CJpegDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	m_edtFolderPassword.SetPasswordChar('*');
	m_edtFolderPassword.GetWindowTextW(m_csEditText);

	m_dwNoofTypingChar = m_edtFolderPassword.LineLength();

	if(m_dwNoofTypingChar >= 7)
	{
		m_edtFolderPassword.GetWindowTextW(m_csEditText);
		ValidatePassword(m_csEditText);
		m_dwNoofTypingChar = 0;
		//return;
	}
	//m_dwNoofTypingChar ++;

}

/**********************************************************************************************************                     
*  Function Name  :	OnChildFire                                                     
*  Description    :	When user click on forgot password hyperlink static.WM_COMMAND calls OnChildFire Function.
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 11 April 2014
**********************************************************************************************************/
LRESULT CFolderLockPassword::OnChildFire(WPARAM wparam,LPARAM lparam)
{
	//Ram, Issue No: 0001216, Resolved
	//Extra check has been added to get number of days left.
	if (theApp.m_dwDaysLeft == 0)
	{
		theApp.GetDaysLeft();
	}

	if(theApp.m_dwDaysLeft == 0)
	{
		if(!theApp.ShowEvaluationExpiredMsg())
		{
			theApp.GetDaysLeft();
			return 0;
		}
	}

	m_szRegEmail = theApp.GetRegisteredEmailID();
	if(m_szRegEmail == L"")
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_EMAIL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK);
		return 0;
	}
	m_stEmailID.SetWindowTextW(const_cast<LPCTSTR>(m_szRegEmail));
	m_btnSendPassword2EmailID.EnableWindow(1);
	m_stEmailID.ShowWindow(TRUE);
	m_stFileIsLocked.ShowWindow(SW_HIDE);
	m_stFolderPassLockText.ShowWindow(SW_HIDE);
	m_stFolderPassUnlockText.ShowWindow(SW_HIDE);
	m_btnFolderPassOk.ShowWindow(SW_HIDE);
	m_btnSendPassword2EmailID.ShowWindow(SW_SHOW);

	/*	ISSUE NO - 667 NAME - NITIN K. TIME - 11th June 2014 */
	 // get the style
    DWORD style = m_btnFolderPassOk.GetStyle();
    // remove default push button style
    style&=~BS_DEFPUSHBUTTON;
    // set the style
    ::SendMessage( m_btnFolderPassOk.GetSafeHwnd(), BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

    // inform the dialog about the new default control id
    SendMessage(DM_SETDEFID, m_btnSendPassword2EmailID.GetDlgCtrlID());

    // get the style
    style=m_btnSendPassword2EmailID.GetStyle();
    // add the default push button style
    style|=BS_DEFPUSHBUTTON;
    // set the style
	::SendMessage( m_btnSendPassword2EmailID.GetSafeHwnd(), BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

	//	m_btnSendPassword2EmailID.SetFocus();
//	m_btnFolderPassCancel.ShowWindow(SW_HIDE);
//	m_btnFolderpassBack.ShowWindow(SW_SHOW);
	m_stFileIsLocked.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_EMAILINFO"));
	m_stFileIsLocked.ShowWindow(SW_HIDE);
	m_stFolderPassUnlockText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_PSSWRDINFO"));
	m_stFolderPassUnlockText.ShowWindow(SW_SHOW);
	m_edtFolderPassword.ShowWindow(SW_HIDE);
	m_EditRenterPassword.ShowWindow(SW_HIDE);
//	m_edtFolderEmailID.ShowWindow(SW_SHOW);
	m_stEmailIconImage.ShowWindow(SW_SHOW);	
	m_stFolderPassUnlockPic.ShowWindow(SW_HIDE);
	m_stForgotPasswordlink.ShowWindow(SW_HIDE);
	//m_edtFolderEmailID.SetWindowTextW(L"");

	return 0;
}


/**********************************************************************************************************                     
*  Function Name  :	ValidateEmailID                                                     
*  Description    :	Validation Email ID
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 11 April 2014
**********************************************************************************************************/
bool CFolderLockPassword::ValidateEmailID()
{
	bool flag = true;
	CString csInputEmail,csTempEmail;
	TCHAR email[64];
	_tcscpy(email,m_csEmailID);
	csInputEmail = email;
 
	int i=0;
	int posDot,posAt,posAt2,posDotSec;

	posAt = csInputEmail.ReverseFind(_T('@'));
	posAt2 = csInputEmail.Find(_T('@'));
	i=0;
		
	if(posAt == posAt2)
	{ 
		flag=0;
		csTempEmail = csInputEmail.Right(csInputEmail.GetLength() - posAt);
	
		posDotSec = csTempEmail.Find(_T('.'));

		posDot = csTempEmail.ReverseFind(_T('.'));
		if(posDot == -1)
		{
			return false;
		}
		int diffDot = posDot - posDotSec; 
		if(diffDot == 1)
		{
			return false;
		}

		posAt = csTempEmail.Find(_T('@'));
		int diff = posDotSec - posAt;
		if(diff == 1)
		{
			return false;
		}

		if(csTempEmail == L"")
		{
			return false;
		}

		if(posAt > -1)
		{
			posAt = csInputEmail.Find(_T('@'));
			csInputEmail.Truncate(posAt);
			if(csInputEmail == L"")
			{
				return false;
			}
		}

	}
	else
	{
		flag = 1;
	}
	if(posAt == -1)
	{
		flag=1;
	}
	
	if(!flag)
	{
		return true;
	}
	else
	return (false);
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonSendButton                                                     
*  Description    :	When user click on Send button.Password will be send on registered email id .
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 11 April 2014
**********************************************************************************************************/
void CFolderLockPassword::OnBnClickedButtonSendButton()
{
	// TODO: Add your control notification handler code here
	CRect m_rect;
	m_btnSendPassword2EmailID.ShowWindow(SW_SHOW);
	m_szRegEmail = theApp.GetRegisteredUserInfo();
	if(m_szRegEmail == NULL)
	{
		AddLogEntry(L"### Not Registered Email-Id", 0, 0, true, SECONDLEVEL);
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_EMAIL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONEXCLAMATION);
	}

		if(!LoadRegistrationDll())
		{
			MessageBox(L"WrdWizEmailDLL.DLL Missing",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION | MB_OK);
			return ;
		}
		FetchForgotPassword();
		m_btnSendPassword2EmailID.EnableWindow(FALSE);
		m_stEmailIconImage.ShowWindow(FALSE);
		m_stEmailID.ShowWindow(FALSE);
		m_stFileIsLocked.ShowWindow(FALSE);
		m_stFolderPassUnlockText.ShowWindow(FALSE);
		m_stForgotPasswordlink.ShowWindow(SW_HIDE);
		m_stFolderPassPreloader.ShowWindow(TRUE);
		m_btnFolderPassCancel.ShowWindow(SW_HIDE);
		m_btnFolderpassBack.ShowWindow(SW_SHOW);
		m_stSendingMail.ShowWindow(SW_SHOW);
		m_btnPassClose.ShowWindow(FALSE);
		m_btnPassMinimize.ShowWindow(TRUE);
		m_btnFolderPassOk.ShowWindow(SW_HIDE);
		m_btnFolderPassCancel.ShowWindow(SW_HIDE);

		m_Thread = CreateThread(NULL, 0, StartThread, (LPVOID)this, 0, &m_dwThread);
		
		if(!m_stFolderPassPreloader.IsPlaying())
		{
			m_stFolderPassPreloader.Draw();
		}



}


/**********************************************************************************************************                     
*  Function Name  :	LoadRegistartionDll                                                     
*  Description    :	Load WRDWIZREGISTERDATA.DLL and proc address of GetfolderRegisteredData
*  Author Name    :	Nitin 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
BOOL CFolderLockPassword::LoadRegistrationDll()
{
	BOOL bReturn = FALSE;
	DWORD	dwSize = 0x00 ,i = 0x00;
	BYTE	szPassWord[0x10] = {0} ;
	TCHAR	szTemp[512] = {0} ;
	TCHAR	szEmail[512] = {0} ;
	TCHAR	szInternetConnState[512] = {0} ;
	TCHAR	*pName = NULL ;
	m_hRegisterDLL = NULL;
	m_hEmailDLL = NULL;
	GetiTinPathFromReg();
	//GetModuleFileName(NULL, szTemp, 511 ) ;
	_tcscpy(szTemp,m_szAppPath);
	
	GetModuleFileName(NULL, szInternetConnState, 511 ) ;
	pName = wcsrchr(szInternetConnState, '\\' ) ;
	if( pName )
		*pName = '\0' ;
	wcscat( szInternetConnState, TEXT("\\wininet.dll")) ;
	
	
	
	GetModuleFileName(NULL, szTemp, 511 ) ;
	pName = wcsrchr(szTemp, '\\' ) ;
	if( pName )
		*pName = '\0' ;

	wcscat( szTemp, TEXT("\\WRDWIZREGISTERDATA.DLL") ) ;

	_tcscpy(szEmail,L"");
	_tcscpy(szEmail,m_szAppPath);
	//GetModuleFileName(NULL, szEmail, 511 ) ;
	pName = wcsrchr(szEmail, '\\' ) ;
	if( pName )
		*pName = '\0' ;

	/*wcscat( szEmail  , TEXT("\\WardWizSendEmailDLL.DLL"));
	if( PathFileExists( szEmail ) )
	{
		if( !m_hEmailDLL )
			m_hEmailDLL = LoadLibrary( szEmail ) ;

		if( !SendPassword2EmailID)
			SendPassword2EmailID = (SENDPASSWORDTOEMAILID) GetProcAddress(m_hEmailDLL, "SendEmail") ;
	
	}*/
	if( PathFileExists( szTemp ) )
	{
		if( !m_hRegisterDLL )
			m_hRegisterDLL = LoadLibrary( szTemp ) ;

		if( !GetRegisteredFolderData )
			GetRegisteredFolderData = (GETREGISTEREFOLDERDDATA) GetProcAddress(m_hRegisterDLL, "GetRegisteredData") ;

		if( !GetRegisteredFolderData )
		{	
			MessageBox(L"WRDWIZREGISTERDATA.DLL not found.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
			return FALSE;
		}
		GetRegisteredFolderData(szPassWord, dwSize, IDR_RESDATA_DES, TEXT("WRDWIZOPTIONRESDES") ) ;
		if(dwSize == 0)
		{
			bReturn = FALSE;
		}
		else
		{
			bReturn = TRUE;
		}
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	FetchForgotPassword                                                     
*  Description    :	Fetch the password from WRDWIZEVALREG.DLL
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CFolderLockPassword::FetchForgotPassword()
{
	TCHAR	szTemp[512] = {0} ;
	TCHAR	*pName = NULL ;
	
	GetModuleFileName( NULL, szTemp, 511 ) ;
	pName = wcsrchr( szTemp, '\\' ) ;
	if( !pName )
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_PATH"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		return ;
	}

	*pName = '\0' ;
	wcscat( szTemp, TEXT("\\WRDWIZEVALREG.DLL") ) ;
	if( !PathFileExists( szTemp ) )
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_REGDLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		return ;
	}

	if( !GetRegisteredFolderData )
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_REGDATA"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		return ;
	}

	DWORD			dwSize = 0x00 ,i = 0x00;
	BYTE			szPassWord[0x10] = {0} ;
	TCHAR			szPass[0x20] = {0} ;
	
	GetRegisteredFolderData(szPassWord, dwSize, IDR_RESDATA_DES, TEXT("WRDWIZOPTIONRESDES") ) ;
	if( szPassWord[0] && dwSize)
	{
		while( szPassWord[i] != 0x00 )
		{
			szPass[i] = szPassWord[i] ;
			i++ ;
		}
		m_csGetForgotPassword.Format( TEXT("%s"), szPass ) ;
		bFOLDER_DES = true ;
	}
}

/* Modified By rajil Yadav on GUI Issue*/
/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within 
					the CWnd object.
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
BOOL CFolderLockPassword::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( iCtrlID == IDCANCEL ||
		iCtrlID == IDC_BUTTON_SEND_BUTTON ||
		iCtrlID == IDC_BUTTON_PASS_CLOSE ||
		iCtrlID == IDC_BUTTON_PASS_MINIMIZE ||
		iCtrlID == IDC_BUTTON_FOLDER_PASS_BACK ||
		iCtrlID == IDC_STATIC_LINK_FORGOT_PASSWORD ||
		iCtrlID == IDOK
	   )
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if( csClassName == _T("Button") && m_hButtonCursor)
			//||
			//csClassName == _T("Static") && 
			//m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}
	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonFolderPassBack                                                     
*  Description    :	Back button on Folder locker password
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CFolderLockPassword::OnBnClickedButtonFolderPassBack()
{
	OnClose();
}

/***********************************************************************************************
*  Function Name  : RefreshString
*  Description    : this function is  called for setting the Text UI with different Language Support
*  Author Name    : Amit Dutta
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
void CFolderLockPassword::RefreshStrings()
{
  	

	m_btnFolderPassOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FLDRLCKPSSWRD_OK"));
	m_btnFolderPassCancel.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FLDRLCKPSSWRD_CANCEL"));
	m_btnSendPassword2EmailID.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FLDRLCKPSSWRD_SEND"));
	m_btnFolderpassBack.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_BUTTON_CANCEL"));
	//m_stFileIsLocked.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_EMAILINFO"));
	m_stFolderPassLockText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_PSSWRD"));
	m_stForgotPasswordlink.SetTootTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_FORGOTPSSWRDINFO"));
    m_stForgotPasswordlink.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_FORGOT_PSSWRD"));
	m_stFileIsLocked.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_LOCKINFO"));
	m_stFolderPassUnlockText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_UNLOCKINFO"));
}


/***********************************************************************************************
*  Function Name  : GetRegisteredUserInfo
*  Description    : Get registered user info from registry and WWIZUSERREG.DB file
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
LPTSTR CFolderLockPassword::GetRegisteredUserInfo( )
{
	
	DWORD	dwRet = 0x00 ;
	DWORD	dwSize = 0x00 ;
	DWORD	dwRegUserSize = 0x00 ;
	dwRet = GetRegistrationDataFromRegistry( ) ;
	if( !dwRet )
	{
		goto Cleanup;
	}


	dwSize = sizeof(m_ActInfo) ;
	dwRegUserSize = 0x00 ;

	memset(&m_ActInfo,0x00,sizeof(m_ActInfo));

	if( GetRegisteredFolderData((LPBYTE)&m_ActInfo, dwRegUserSize , IDR_REGDATA, L"REGDATA" ) == 0)
	{
		dwRet = 0x00 ;
		goto Cleanup;
	}
	AfxMessageBox(m_ActInfo.szEmailID,0,0);


Cleanup:

	if( dwRet )
		dwRet = GetRegistrationDataFromRegistry( ) ;

	

	if( !dwRet )
		memcpy(&g_ActInfoNew, &m_ActInfo, sizeof(m_ActInfo) ) ;
	
	return m_ActInfo.szEmailID ;
}

/***********************************************************************************************
*  Function Name  : GetRegistrationDataFromRegistry
*  Description    : Get registered user info from registry
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
DWORD CFolderLockPassword::GetRegistrationDataFromRegistry( )
{
	DWORD	dwRet = 0x00 ;
	DWORD	dwRegType = 0x00, dwRetSize = 0x00 ;

	HKEY	h_iSpyAV = NULL ;
	HKEY	h_iSpyAV_User = NULL ;

	
	if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows"), 
						0, KEY_READ, &h_iSpyAV ) != ERROR_SUCCESS )
	{
		dwRet = 0x01 ;
		goto Cleanup ;
	}
	dwRetSize = sizeof(m_ActInfo) ;
	/*if( RegQueryValueEx(h_iSpyAV, TEXT("UserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo, 
						&dwRetSize) != ERROR_SUCCESS )*/
	if( RegQueryValueEx(h_iSpyAV, TEXT("WardWizUserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo, 
						&dwRetSize) != ERROR_SUCCESS )
	{
		dwRet = GetLastError() ;
		dwRet = 0x03 ;
		goto Cleanup ;
	}

	if( theApp.DecryptRegistryData( (LPBYTE )&m_ActInfo, sizeof(m_ActInfo)) )
	{
		dwRet = 0x04 ;
		goto Cleanup ;
	}

Cleanup:

	if( h_iSpyAV_User )
		RegCloseKey( h_iSpyAV_User ) ;
	if( h_iSpyAV )
		RegCloseKey( h_iSpyAV ) ;

	h_iSpyAV_User = h_iSpyAV = NULL ;

	return dwRet ;
}


//DWORD CFolderLockPassword::DecryptData( LPBYTE lpBuffer, DWORD dwSize )
//{
//	if( IsBadWritePtr( lpBuffer, dwSize ) )
//		return 1 ;
//	
//	DWORD	iIndex = 0 ;
//	DWORD jIndex = 0;
//
//	if (lpBuffer == NULL || dwSize == 0x00)
//	{
//		return 1;
//	}
//
//	for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
//	{
//		if(lpBuffer[iIndex] != 0)
//		{
//			lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex++] + WRDWIZ_KEYSIZE);
//			if (jIndex == WRDWIZ_KEYSIZE)
//			{
//				jIndex = 0x00;
//			}
//			if (iIndex >= dwSize)
//			{
//				break;
//			}
//		}
//	}	
//
//	//DWORD	i = 0 ;
//	//DWORD	dwKey = WRDWIZ_DATA_ENC_KEY ;
//
//	//for(; i<dwSize; )
//	//{
//	//	if( (dwSize - i) < 0x04 )
//	//		break ;
//
//	//	*((DWORD *)&lpBuffer[i]) = *((DWORD *)&lpBuffer[i]) ^ dwKey ;
//
//	//	i += sizeof(DWORD) ;
//	//}
//
//	return 0 ;
//}
/***********************************************************************************************
*  Function Name  : StartThread
*  Description    : Thread shows animation of sending password to email id and shows successful and
					failed messages
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
DWORD WINAPI StartThread(LPVOID arg)
{

	CFolderLockPassword *pThis = (CFolderLockPassword*)arg;
	if(!pThis)
		return 0;

	CRect rect;
	pThis->GetClientRect(rect);


	pThis->m_lpEmailID = pThis->m_csPass.GetBuffer();
	pThis->m_lpPassword = pThis->m_csGetForgotPassword.GetBuffer();

	DWORD dwRet = pThis->SendPassword2EmailID(pThis->m_szRegEmail,pThis->m_lpPassword);
	if(dwRet == 0x2)
	{
		if(bFOLDER_DES)
		{
			if(pThis->m_stFolderPassPreloader.IsPlaying())
			{
				pThis->m_btnSendPassword2EmailID.ShowWindow(FALSE);
				pThis->m_stFolderPassPreloader.Stop();
				pThis->m_stFolderPassPreloader.ShowWindow(SW_SHOW);
				pThis->m_stSendingMail.SetWindowTextW(L"Sending completed");
				pThis->m_stSendingMail.ShowWindow(SW_SHOW);
				pThis->m_btnPassClose.ShowWindow(TRUE);
				pThis->m_btnPassMinimize.ShowWindow(FALSE);
				pThis->m_btnFolderpassBack.ShowWindow(FALSE);

				//Issue No:888 Resolved by Prajakta on 2 July 2014
				pThis->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_SENT_EMAIL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONINFORMATION);
				pThis->OnDialogClose();
			}	

		}
	}
	else
	{
		if(pThis->m_stFolderPassPreloader.IsPlaying())
		{
			pThis->m_stFolderPassPreloader.Stop();
			pThis->m_btnSendPassword2EmailID.ShowWindow(FALSE);
			pThis->m_stFolderPassPreloader.ShowWindow(SW_SHOW);
			pThis->m_stSendingMail.SetWindowTextW(L"Sending failed");
			pThis->m_stSendingMail.ShowWindow(SW_SHOW);
			pThis->m_btnFolderPassCancel.ShowWindow(SW_SHOW);
			pThis->m_btnPassClose.ShowWindow(TRUE);
			pThis->m_btnPassMinimize.ShowWindow(FALSE);
			pThis->m_btnFolderpassBack.ShowWindow(FALSE);
			
			//Issue No:888 Resolved by Prajakta on 2 July 2014
			pThis->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_FAILEDTOSEND_MAIL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONEXCLAMATION);
			pThis->OnDialogClose();
		}
	}


	if(pThis->m_Thread!=NULL)
	{
		TerminateThread(pThis->m_Thread,0);
		pThis->m_stFolderPassPreloader.ShowWindow(SW_HIDE);
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : GetiTinPathFromReg
*  Description    : Get production folder path from registry
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
bool CFolderLockPassword::GetiTinPathFromReg()
{
	HKEY	hSubKey = NULL ;
	DWORD	dwSize = 511 ;
	DWORD	dwType = 0x00 ;

	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WardWiz Antivirus", 0, KEY_READ, &hSubKey ) != ERROR_SUCCESS )
		return false ;

	RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)m_szAppPath, &dwSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if( !PathFileExists(m_szAppPath) )
	{
		return false ;
	}
	return true;
}


/***********************************************************************************************
*  Function Name  : OnClose
*  Description    : The framework calls this member function as a signal 
				   that the CWnd or an application is to terminate.
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
void CFolderLockPassword::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	::SuspendThread(m_Thread);
	if(m_stFolderPassPreloader.IsPlaying())
	{
		m_stFolderPassPreloader.Stop();
	}
	if(MessageBoxW(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_EMAIL_QUESTION"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		::SuspendThread(m_Thread);
		::TerminateThread(m_Thread,0);
		m_stFolderPassPreloader.ShowWindow(SW_SHOW);
		m_stSendingMail.SetWindowTextW(L"Sending failed");
		m_stSendingMail.ShowWindow(SW_SHOW);
		m_btnSendPassword2EmailID.ShowWindow(FALSE);
		m_btnPassMinimize.ShowWindow(FALSE);
		m_btnFolderpassBack.ShowWindow(FALSE);
		m_btnFolderPassCancel.ShowWindow(FALSE);
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_FAILEDTOSEND_MAIL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONEXCLAMATION);
		//m_btnFolderPassCancel.SetWindowTextW(L"Ok");
		//m_stNotSentMsg.ShowWindow(SW_SHOW);
	}
	else 
	{
		if(!m_stFolderPassPreloader.IsPlaying())
		{
			m_stFolderPassPreloader.Draw();
		}
		::ResumeThread(m_Thread);
	}
	OnDialogClose();
}

/***********************************************************************************************
*  Function Name  : CloseOnOk
*  Description    : On ok button
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
void CFolderLockPassword::CloseOnOk()
{
	OnOK();
}

/***********************************************************************************************
*  Function Name  : CloseOnCancel
*  Description    : OnCancel call to close Dialog
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
void CFolderLockPassword::CloseOnCancel()
{
	OnCancel();
}

/***********************************************************************************************
*  Function Name  : OnBnClickedButtonAfterSent
*  Description    : After sending password close password UI
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
void CFolderLockPassword::OnBnClickedButtonAfterSent()
{
	OnOK();
}

//Rajil Yadav Password Sending code date /6/6/2014
/***********************************************************************************************
*  Function Name  : SendPassword2EmailID
*  Description    : sending password password to registered email ID
*  Author Name    : Nitin
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
DWORD CFolderLockPassword::SendPassword2EmailID(LPTSTR m_szRegEmail,LPTSTR m_lpPassword)
{
	DWORD	dwRet = 0x01 ;

	TCHAR	szUserInfo[512] = {0} ;
	TCHAR	szResponse[512] = {0} ;

	CStringW csEncString = XOREncrypt(m_lpPassword, static_cast<DWORD>(_tcslen(m_lpPassword)));
	wsprintf(szUserInfo, L"http://wardwiz.com/email/send_email.php?param1=%s&param2=%s&param3=%s",m_szRegEmail, csEncString,L"1") ;

	AddLogEntry(L">>>> Sending Mail", 0, 0, true, ZEROLEVEL);
	AddLogEntry(szUserInfo);

	WinHttpClient client( szUserInfo);

	if(!client.SendHttpRequest())
	{
		//MessageBox(L"Please check internet connection.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_CHECK_NET"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		return dwRet ;
	}

    // The response content.
    wstring httpResponseContent = client.GetResponseContent() ;

	AddLogEntry(L">>>> Getting response", 0, 0, true, ZEROLEVEL);
	AddLogEntry(httpResponseContent.c_str());

	if( httpResponseContent  == L"1")/* == L"SUCCESS" )*/
	{
		dwRet = 0x2;
	}
	else
	{ 
		dwRet = 0x1;
	}
	return dwRet;
}

  
// Rajil Yadav Encrypting the data.,
/**********************************************************************************************************                     
*  Function Name  :	XOREncrypt                                                     
*  Description    :	Encrypt data with xor logic
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
CString CFolderLockPassword::XOREncrypt(CString a_sValue, DWORD a_sPasswordSize)
{
	DWORD i = 0 , j = 0 ;
	
	BYTE	szPass[0x10] = {0} ;
	unsigned char	chPass = 0x00 ;
	CString EncryptPassRet = _T("");
	TCHAR szPassword[0x20] = {0};

	for(i=0; i<a_sPasswordSize; i++ )
	{
		chPass = static_cast<BYTE>(a_sValue.GetAt(i));
		szPass[i] = chPass ;
	}

	theApp.DecryptRegistryData(szPass,a_sPasswordSize);

	while( szPass[j] != 0x00 )
	{
		szPassword[j] = szPass[j] ;
		j++ ;
	}
	
	EncryptPassRet.Format( TEXT("%s"), szPassword ) ;
    return EncryptPassRet;

}

/**********************************************************************************************************                     
*  Function Name  :	OnEnChangeEditRenterPassword                                                     
*  Description    :	Call when Re-enter password edit box has any changes.
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
void CFolderLockPassword::OnEnChangeEditRenterPassword()
{
	m_EditRenterPassword.SetPasswordChar('*');
	m_EditRenterPassword.GetWindowTextW(m_csEditText);

	m_dwNoofTypingChar = m_EditRenterPassword.LineLength();

	if(m_dwNoofTypingChar >= 7)
	{
		m_EditRenterPassword.GetWindowTextW(m_csEditText);
		ValidatePassword(m_csEditText);
		m_dwNoofTypingChar = 0;
		//return;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnDialogClose                                                     
*  Description    :	Close folder locker password dialog.
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
void CFolderLockPassword::OnDialogClose()
{
	OnCancel();
	CJpegDialog::OnClose();
}