/**********************************************************************************************************                     
	  Program Name          : ISpyPasswordPopUpDlg.cpp
	  Description           : Password dialog for data encrypt and decrypt data.
	  Author Name			: Neha Gharge                                                                                          
	  Date Of Creation      : 4th Dec 2013
	  Version No            : 1.0.0.2
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna           Created ISpy GUI using GDI objects       11 - 11 - 2013              
***********************************************************************************************************/

#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyPasswordPopUpDlg.h"
#include "WinHttpClient.h"
#include "FolderLockPassword.h"
#define		IDR_RESDATA_DES			5000
const int TIMER_EMAILSENDINGFINISH = 1000;


CString		m_csPassword;
typedef DWORD (*GETREGISTEREDFOLDERDDATA) (LPBYTE lpResBuffer, DWORD &dwResDataSize, DWORD dwResType, TCHAR *pResName ) ;
GETREGISTEREDFOLDERDDATA	GetRegisteredFolderData1 = NULL ;
// CISpyPasswordPopUpDlg dialog

IMPLEMENT_DYNAMIC(CISpyPasswordPopUpDlg, CDialog)
DWORD WINAPI StartEncThread(LPVOID arg);
bool bFOLDER = false ;

/**********************************************************************************************************                     
*  Function Name  :	CISpyPasswordPopUpDlg                                                     
*  Description    :	C'tor
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 4 Dec 2013
**********************************************************************************************************/
CISpyPasswordPopUpDlg::CISpyPasswordPopUpDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyPasswordPopUpDlg::IDD, pParent)
	, m_stEncForgotPassword(theApp.m_hResDLL)
{
// issue resolved by lalit kumawat 12-24-2014 ,issue :Wardwiz crashes on clicking ok on sending mail for forgot password.
 m_bIsEmailSendingFinish = false;
}


/**********************************************************************************************************                     
*  Function Name  :	CISpyPasswordPopUpDlg                                                     
*  Description    :	D'tor
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 4 Dec 2013
**********************************************************************************************************/
CISpyPasswordPopUpDlg::~CISpyPasswordPopUpDlg()
{
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 4 Dec 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);

	//DDX_Control(pDX, IDC_STATIC_PASSWORD_HPIC, m_stPasswordHPic);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_BUTTON_PASSWORD_CLOSE, m_btnClose);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_PASSWORD, m_stPassword);
	DDX_Control(pDX, IDC_EDIT_ENTERPASSWORD, m_edtPasswordLetters);
	DDX_Control(pDX, IDC_STATIC_EXAMPLE, m_stExample);
	DDX_Control(pDX, IDC_STATIC_ENC_MSG, m_stEncryPassMsg);
	DDX_Control(pDX, IDC_STATIC_DEC_MSG, m_stDecPassMsg);
	DDX_Control(pDX, IDC_STATIC_ENCRYP_FORGOT_PASSWORD_LINK, m_stEncForgotPassword);
	DDX_Control(pDX, IDC_BUTTON_ENC_SEND, m_btnEncSend);
	DDX_Control(pDX, IDC_BUTTON_ENC_CANCEL, m_btnEncCancel);
	DDX_Control(pDX, IDC_STATIC_ENC_EAMIL_ID, m_stEncEmailId);
	DDX_Control(pDX, IDC_STATIC_MAILBOX_PIC, m_stMailboxPic);
	DDX_Control(pDX, IDC_EDIT_REENTER_PASSWORD, m_edtReEnterPassword);
	DDX_Control(pDX, IDC_STATIC_REENTER_PASS, m_stReEnterPass);
	DDX_Control(pDX, IDC_STATIC_SENDINGMAIL, m_stSendingMail);
	DDX_Control(pDX, IDC_STATIC_DATAENCPRELOADER, m_stDataEncPreLoader);
	DDX_Control(pDX, IDC_BTN_SENDCANCEL, m_btnSendCancel);
}

/***************************************************************************
* Function Name   : MESSAGE_MAP
* Description     : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
*  Author Name    : Neha Gharge  
*  SR_NO		  :
*  Date           : 4 Dec 2013
****************************************************************************/
BEGIN_MESSAGE_MAP(CISpyPasswordPopUpDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(_HYPERLINK_EVENT,OnChildFire)
	ON_BN_CLICKED(IDOK, &CISpyPasswordPopUpDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CISpyPasswordPopUpDlg::OnBnClickedCancel)
	
	ON_BN_CLICKED(IDC_BUTTON_PASSWORD_CLOSE, &CISpyPasswordPopUpDlg::OnBnClickedButtonPasswordClose)
	ON_EN_CHANGE(IDC_EDIT_ENTERPASSWORD, &CISpyPasswordPopUpDlg::OnEnChangeEditEnterpassword)
	ON_STN_CLICKED(IDC_STATIC_ENCRYP_FORGOT_PASSWORD_LINK, &CISpyPasswordPopUpDlg::OnStnClickedStaticEncrypForgotPasswordLink)
	ON_BN_CLICKED(IDC_BUTTON_ENC_SEND, &CISpyPasswordPopUpDlg::OnBnClickedButtonEncSend)
	ON_BN_CLICKED(IDC_BUTTON_ENC_CANCEL, &CISpyPasswordPopUpDlg::OnBnClickedButtonEncCancel)
	ON_WM_SETCURSOR()
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_EDIT_REENTER_PASSWORD, &CISpyPasswordPopUpDlg::OnEnChangeEditReenterPassword)
	ON_BN_CLICKED(IDC_BTN_SENDCANCEL, &CISpyPasswordPopUpDlg::OnBnClickedBtnSendcancel)
   // issue resolved by lalit kumawat 12-24-2014 ,issue :Wardwiz crashes on clicking ok on sending mail for forgot password.	
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CISpyPasswordPopUpDlg message handlers
/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.N0		  : 
*  Author Name    : Neha Gharge                                                                                          
*  Date           : 4 Dec 2013
**********************************************************************************************************/
BOOL CISpyPasswordPopUpDlg::OnInitDialog()
{
	
	CJpegDialog::OnInitDialog();
	//SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	/*	ISSUE NO - 865 NAME - NITIN K. TIME - 28th June 2014 */
	if((m_bEncryptionOption))
	{
		if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_PASSWORD_DATA_FOLDER), _T("JPG")))
		{
			MessageBox( theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		}
	}
	if(!(m_bEncryptionOption))
	{
		if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_PASSWORD_BG), _T("JPG")))
		{
			::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_FAILMSG")
				, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		}
	}
	Draw();

	//To find a single instance of UI.
	//Issue no : 664 If i right click and say as encrypt, then we select another file and say as encrypt, it shows two messagebox
	//Neha Gharge 6 July,2015 
	this->SetWindowText(L"WRDWIZAVUI");

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect1;
	this->GetClientRect(rect1);
	//SetWindowPos(&wndTop, rect1.top + 430,rect1.top + 260, rect1.Width()-2, rect1.Height()-2, SWP_NOREDRAW);
	
	CRgn rgn;
	rgn.CreateRectRgn(rect1.top, rect1.left ,rect1.Width()-3,rect1.Height()-3);
	this->SetWindowRgn(rgn, TRUE);

	m_pBoldFont = new(CFont);
	m_pBoldFont->CreateFont(15,6,0,0,FW_SEMIBOLD,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"verdana");

	if(m_stDataEncPreLoader.Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_GIF_32x32_PRELOADER),_T("GIF")))
	{
		if(!m_stDataEncPreLoader.IsPlaying())
		{
			m_stDataEncPreLoader.Draw();
		}
	}

	if(m_stDataEncPreLoader.IsPlaying())
	{
		m_stDataEncPreLoader.Stop();
	}
	m_stDataEncPreLoader.SetWindowPos(&wndTop,rect1.left+170,65,32,32, SWP_NOREDRAW);
	m_stDataEncPreLoader.ShowWindow(SW_HIDE);
	m_stDataEncPreLoader.SetBkColor(RGB(255,255,255));

	m_stSendingMail.SetWindowPos(&wndTop,rect1.left +142,120,200,20, SWP_NOREDRAW);
	m_stSendingMail.ShowWindow(FALSE);
	m_stSendingMail.SetBkColor(RGB(255,255,255));
	m_stSendingMail.SetFont(&theApp.m_fontWWTextNormal);

	/*m_stEncryPassMsg.SetFont(m_pBoldFont);
	m_stEncryPassMsg.SetTextAlign(1);
	m_stEncryPassMsg.SetColor(RGB(200,249,247));
	m_stEncryPassMsg.SetGradientColor(RGB(200,249,247));
	m_stEncryPassMsg.SetTextColor(BLACK);

	m_stDecPassMsg.SetFont(m_pBoldFont);
	m_stDecPassMsg.SetTextAlign(1);
	m_stDecPassMsg.SetColor(RGB(200,249,247));
	m_stDecPassMsg.SetGradientColor(RGB(200,249,247));
	m_stDecPassMsg.SetTextColor(BLACK);*/


	//m_bmpPasswordHPic.LoadBitmapW(IDB_BITMAP_PASS_HEADERPIC);
	//m_stPasswordHPic.SetBitmap(m_bmpPasswordHPic);
	//m_stPasswordHPic.SetWindowPos(&wndTop,rect1.left + 25,50,341,46,SWP_SHOWWINDOW);
	//m_stPasswordHPic.ShowWindow(FALSE);

	m_stDecPassMsg.SetWindowPos(&wndTop,rect1.left + 30,60,350,46,SWP_NOREDRAW);
	//m_stDecPassMsg.SetFont(m_pBoldFont);
	m_stDecPassMsg.SetFont(&theApp.m_fontWWTextNormal);
	/*	ISSUE NO - 217 NAME - NITIN K. TIME - 21st May 2014 :: 8 pm*/
	m_stDecPassMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_DECRPASSMSG_TEXT"));	
	m_stEncryPassMsg.SetWindowPos(&wndTop,rect1.left + 30,60,350,46,SWP_NOREDRAW);
	//m_stEncryPassMsg.SetFont(m_pBoldFont);
	m_stEncryPassMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_ENCRPASSMSG_TEXT"));
	m_stEncryPassMsg.SetFont(&theApp.m_fontWWTextNormal);
	//m_stDecPassMsg.SetTextColor(RGB(127,126,126));
	
	
	
	m_stPassword.SetWindowPos(&wndTop,rect1.left + 30,100,80,20,SWP_NOREDRAW);
	m_stPassword.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_PASS_TEXT"));
	m_stPassword.SetFont(&theApp.m_fontWWTextNormal);
	
	

	m_stReEnterPass.SetWindowPos(&wndTop,rect1.left + 30,150,200,20,SWP_NOREDRAW);
	m_stReEnterPass.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_RE_ENTER_PASS_TEXT"));
	m_stReEnterPass.SetFont(&theApp.m_fontWWTextNormal);
	m_stReEnterPass.SetBkColor(RGB(255,255,255));
	
	m_stExample.SetWindowPos(&wndTop,rect1.left + 30,205,240,20,SWP_NOREDRAW);
	m_stExample.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_EXAMPLE_TEXT"));
	m_stExample.SetFont(&theApp.m_fontWWTextNormal);

	m_btnOk.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnOk.SetWindowPos(&wndTop,rect1.left + 233,235,57,21,SWP_NOREDRAW);
	m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_BUTTON_OK"));
	m_btnOk.SetTextColorA(BLACK,1,1);
	m_btnOk.SetFont(&theApp.m_fontWWTextNormal);

	m_btnCancel.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnCancel.SetWindowPos(&wndTop,rect1.left + 300,235,57,21,SWP_NOREDRAW);
	m_btnCancel.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_BUTTON_CANCEL"));
	m_btnCancel.SetTextColorA(BLACK,1,1);
	m_btnCancel.SetFont(&theApp.m_fontWWTextNormal);

	m_btnEncSend.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnEncSend.SetWindowPos(&wndTop,rect1.left + 220,165,57,21,SWP_NOREDRAW);
	m_btnEncSend.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FLDRLCKPSSWRD_SEND"));
	m_btnEncSend.SetTextColorA(BLACK,1,1);
	m_btnEncSend.SetFont(&theApp.m_fontWWTextNormal);
	m_btnEncSend.ShowWindow(FALSE);
	
	m_btnEncCancel.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnEncCancel.SetWindowPos(&wndTop,rect1.left + 295,165,57,21,SWP_NOREDRAW);
	m_btnEncCancel.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_BUTTON_CANCEL"));
	m_btnEncCancel.SetTextColorA(BLACK,1,1);
	m_btnEncCancel.SetFont(&theApp.m_fontWWTextNormal);
	m_btnEncCancel.ShowWindow(FALSE);

	m_btnSendCancel.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnSendCancel.SetWindowPos(&wndTop,rect1.left + 295,165,57,21,SWP_NOREDRAW);
	m_btnSendCancel.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_BUTTON_CANCEL"));
	m_btnSendCancel.SetTextColorA(BLACK,1,1);
	m_btnSendCancel.SetFont(&theApp.m_fontWWTextNormal);
	m_btnSendCancel.ShowWindow(FALSE);

	m_btnClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnClose.SetWindowPos(&wndTop,rect1.left + 348,0,26,17,SWP_NOREDRAW);

	//Added By Nitin K.
	//For new implementation of Encrypt, Forgot Pass not required
	m_stEncForgotPassword.SetWindowPos(&wndTop,rect1.left + 30,165,150,25,SW_HIDE);
	m_stEncForgotPassword.SetFont(&theApp.m_fontWWTextNormal);
	m_stEncForgotPassword.ShowWindow(SW_HIDE);
	//m_stEncForgotPassword.SetFireChild(1);


	m_bmpEmailIconPic = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_FOLDEREMAIL_ICON));
	m_stMailboxPic.SetWindowPos(&wndTop,rect1.left +20,70,52,60, SWP_NOREDRAW);
    m_stMailboxPic.SetBitmap(m_bmpEmailIconPic);
	m_stMailboxPic.ShowWindow(SW_HIDE);


	m_stEncEmailId.SetWindowPos(&wndTop,rect1.left + 92,80,250,100,SWP_NOREDRAW);
	m_stEncEmailId.SetFont(&theApp.m_fontWWTextNormal);
	m_stEncEmailId.SetBkColor(RGB(255,255,255));
	m_stEncEmailId.ShowWindow(SW_HIDE);
	
	m_csPassword = "" ;
	
	m_edtPasswordLetters.SetWindowTextW( TEXT("") ) ;
	m_edtReEnterPassword.SetWindowPos(&wndTop,rect1.left + 30,170,330,20,SWP_NOREDRAW);
	m_edtPasswordLetters.SetWindowPos(&wndTop,rect1.left + 30,120,330,20,SWP_NOREDRAW);
	// issue- if we try multiple time with with more then 25 char and after n attempt we cancel it , now it will show n password pop dlg
	// resolved by lalit kumawat 7-30-2015
	m_edtReEnterPassword.LimitText(24);
	m_edtPasswordLetters.LimitText(24);
	m_edtPasswordLetters.SetFocus( ) ;
	
	if((m_bEncryptionOption))
	{
		m_stEncryPassMsg.ShowWindow(TRUE);
		m_stDecPassMsg.ShowWindow(FALSE);
		m_stEncForgotPassword.ShowWindow(FALSE);
		m_stMailboxPic.ShowWindow(SW_HIDE);
	}
	if(!(m_bEncryptionOption))
	{
		m_stDecPassMsg.ShowWindow(TRUE);
		m_stEncryPassMsg.ShowWindow(FALSE);
		m_stMailboxPic.ShowWindow(SW_HIDE);
		m_stReEnterPass.ShowWindow(SW_HIDE);
		m_edtReEnterPassword.ShowWindow(SW_HIDE);
		m_stPassword.SetWindowPos(&wndTop,rect1.left + 30,90,80,20,SWP_NOREDRAW);
		m_edtPasswordLetters.SetWindowPos(&wndTop,rect1.left + 30,110,320,20,SWP_NOREDRAW);
		m_stExample.SetWindowPos(&wndTop,rect1.left + 30,135,240,20,SWP_NOREDRAW);
		m_btnOk.SetWindowPos(&wndTop,rect1.left + 220,165,57,21,SWP_NOREDRAW);
		m_btnCancel.SetWindowPos(&wndTop,rect1.left + 295,165,57,21,SWP_NOREDRAW);
	}

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedOk                                                     
*  Description    :	After clicking on OK button, It will check for validate password.
*  SR.N0		  : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 4 Dec 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//OnOK();
	
	TCHAR		szPass[0x19] = {0} ;	//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
	TCHAR		szPassReEnter[0x19] = {0} ;	//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12

	CString		strPass("") ;
	CString		strPassReEnter("") ;

	m_edtPasswordLetters.GetWindowTextW( szPass, 25 ) ;	//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
	if( !szPass[0] )
	{
		//While decrypting file->select file for decryption->don't enter password->click OK the message should be "Please enter password for data decryption.".
		//Niranjan Deshak.- 03/03/2015.
		if (m_bEncryptionOption)
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENCRY_PASSWRD"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		else if (!(m_bEncryptionOption))
		{
			//IDS_STATIC_DATA_DYCRY_PASSWRD
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_DYCRY_PASSWRD"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		
	}
	if( szPass[0] )
	{
		strPass.Format( TEXT("%s"), szPass ) ;
		//Varada Ikhar, Date: 04-03-2015, Issue: If user does not enter password in Re-enter Password field, message needs to be dispplayed & it should get the foccus without clearing password field.
		m_edtReEnterPassword.GetWindowTextW(szPassReEnter, 25); 
		strPassReEnter.Format(TEXT("%s"), szPassReEnter);
		if ( !szPassReEnter[0] && m_bEncryptionOption == TRUE )
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_RE_ENTER_PASSWORD_TEXT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_edtReEnterPassword.SetWindowTextW(TEXT(""));
			m_edtReEnterPassword.SetFocus();
			return;
		}
		if( !ValidatePassword( strPass ) )
		{
			if(m_bEncryptionOption)
			{
				//Varada Ikhar, Date: 04-03-2015, Issue: If user does not enter password in Re-enter Password field, message needs to be dispplayed & it should get the foccus without clearing password field.
				//m_edtReEnterPassword.GetWindowTextW( szPassReEnter, 25 ) ; //varada
				//strPassReEnter.Format( TEXT("%s"), szPassReEnter ) ;
				if(strPass != strPassReEnter)
				{
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_PASS_MISMATCH_TEXT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
					m_csPassword = strPass =strPassReEnter  = "" ;
					m_edtPasswordLetters.SetWindowTextW( TEXT("") ) ;
					m_edtReEnterPassword.SetWindowTextW( TEXT("") ) ;
					m_edtPasswordLetters.SetFocus( ) ;
					return;
				}
			}
			m_csPassword = strPass ;
			OnOK();
		}
		else
		{
			//ISSUE NO - 11	In Data encryption, if we enter more than 8 characters in password set dialog, the 1st textfield gets blanked but the second textfield does not gets blanked.
			//NAME - NITIN K. TIME - 15th Aug 2014
			m_csPassword = strPass  = "" ;
			m_edtReEnterPassword.SetWindowTextW( TEXT("") ) ;
			m_edtPasswordLetters.SetWindowTextW( TEXT("") ) ;
			m_edtPasswordLetters.SetFocus( ) ;
		}
	}
	else
	{
		//ISSUE NO - 11	In Data encryption, if we enter more than 8 characters in password set dialog, the 1st textfield gets blanked but the second textfield does not gets blanked.
		//NAME - NITIN K. TIME - 15th Aug 2014
		m_csPassword = strPass  = "" ;
		m_edtReEnterPassword.SetWindowTextW( TEXT("") ) ;
		m_edtPasswordLetters.SetWindowTextW( TEXT("") ) ;
		m_edtPasswordLetters.SetFocus( ) ;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCancel                                                     
*  Description    :	Close the password dialog.
*  SR.N0		  : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 4 Dec 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonPasswordClose                                                     
*  Description    :	Close the password dialog.
*  SR.N0		  : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 4 Dec 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnBnClickedButtonPasswordClose()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    : The framework calls this member function when a child
				    control is about to be drawn.
*  SR.N0		  : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 4 Dec 2013
**********************************************************************************************************/
HBRUSH CISpyPasswordPopUpDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_PASSWORD ||
		ctrlID == IDC_STATIC_EXAMPLE  ||
		ctrlID == IDC_STATIC_ENC_MSG  ||
		ctrlID == IDC_STATIC_DEC_MSG
		 )
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} return hbr;
}

/**********************************************************************************************************                     
*  Function Name  :	OnEnChangeEditEnterpassword                                                     
*  Description    : It calls this function after changing any text in edit box 
*  SR.N0		  : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 4 Dec 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnEnChangeEditEnterpassword()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CJpegDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	m_edtPasswordLetters.SetPasswordChar('*');
	m_edtPasswordLetters.GetWindowTextW(m_csEditText);
	//Name:Varada Ikhar, Date:15/01/2015
	//Version: 1.8.3.17
	//Description:Data Encryption password length should be reduced. min:6 max:24
	m_dwNoofTypingChar = m_edtPasswordLetters.LineLength();

	if(m_dwNoofTypingChar >= 25) //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:6
	{
		m_edtPasswordLetters.GetWindowTextW(m_csEditText);
		ValidatePassword(m_csEditText);
		m_dwNoofTypingChar = 0;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	ValidatePassword                                                     
*  Description    :	Check the valid password, whether it contains atlst 1 no,1 special char,password length.
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 4 Dec 2013
**********************************************************************************************************/

DWORD CISpyPasswordPopUpDlg::ValidatePassword(CString csPassword)
{
	int			i,count=0 ;
	CString		csInputKey ;
	csInputKey = csPassword;

	int		Validlenght = csInputKey.GetLength();

	if(Validlenght==0)
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENTER_PSSWRD"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
		//PopUpDialog();
		return 1 ;
	}
	
	if(m_bEncryptionOption)
	{

		//Name:Varada Ikhar, 
		//Date:15/01/2015 Version: 1.8.3.17
		//Description:Data Encryption password length should be reduced. min:6 max:24
		if(Validlenght <= 5) 
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALID5"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			//PopUpDialog();
			return 2 ;
		}

		if(Validlenght>=25)  //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALID25"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			//PopUpDialog();
			return 3 ;
		}
		if((csInputKey.FindOneOf(L"~`!@#$%^&*()_-+={}[]|\?/.,':;<>")==-1))
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALIDSPL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
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
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALIDNUM"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
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
*  Function Name  :	PreTranslateMessage                                                     
*  Description    :	To translate window messages before they are dispatched to the TranslateMessage 
					and DispatchMessage Windows functions
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 4 Dec 2013
**********************************************************************************************************/
BOOL CISpyPasswordPopUpDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/**********************************************************************************************************                     
*  Function Name  :	OnStnClickedStaticEncrypForgotPasswordLink                                                     
*  Description    :	When user click on forgot password static.
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 20th April 2014
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnStnClickedStaticEncrypForgotPasswordLink()
{

	return;
}

/**********************************************************************************************************                     
*  Function Name  :	OnChildFire                                                     
*  Description    :	When user click on forgot password hyperlink static.WM_COMMAND calls OnChildFire Function.
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 20th April 2014
**********************************************************************************************************/
LRESULT CISpyPasswordPopUpDlg::OnChildFire(WPARAM wparam,LPARAM lparam)
{
	LPTSTR csEmail = theApp.GetRegisteredUserInfo();
	m_stExample.ShowWindow(FALSE);
	m_edtPasswordLetters.ShowWindow(FALSE);
	m_stPassword.ShowWindow(FALSE);
	m_stEncryPassMsg.ShowWindow(FALSE);
	m_stDecPassMsg.ShowWindow(FALSE);
	m_btnCancel.ShowWindow(FALSE);
	m_btnOk.ShowWindow(FALSE);
	m_stEncForgotPassword.ShowWindow(FALSE);
	m_btnEncSend.ShowWindow(TRUE);
	m_btnEncCancel.ShowWindow(TRUE);
	m_stEncEmailId.ShowWindow(TRUE);
	m_csEmailId.Format(L"%s\n\n%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_PSSWRDINFO"),csEmail);
	//m_csEmailId.Append(csEmail);
	m_stEncEmailId.SetWindowTextW(m_csEmailId);

	// get the style
    DWORD style = m_btnOk.GetStyle();
    // remove default push button style
    style&=~BS_DEFPUSHBUTTON;
    // set the style
    ::SendMessage( m_btnOk.GetSafeHwnd(), BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

    // inform the dialog about the new default control id
    SendMessage(DM_SETDEFID, m_btnEncSend.GetDlgCtrlID());

    // get the style
    style=m_btnEncSend.GetStyle();
    // add the default push button style
    style|=BS_DEFPUSHBUTTON;
    // set the style
	::SendMessage( m_btnEncSend.GetSafeHwnd(), BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

	m_stMailboxPic.ShowWindow(SW_SHOW);

	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonEncSend                                                     
*  Description    :	When user click on Send button.Password will be send on registered email id .
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 20th April 2014
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnBnClickedButtonEncSend()
{
	m_csEmail = theApp.GetRegisteredUserInfo();
	
	if(!theApp.m_lpLoadEmail)
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_LOAD_FILED_REGISTERDATA"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	FetchForgotPassword();
		
	ShowHideControls4ForgotPassword();
	
	m_Thread = CreateThread(NULL, 0, StartEncThread, (LPVOID)this, 0, &m_dwThread);
	
	// issue resolved by lalit kumawat 12-24-2014 ,issue :Wardwiz crashes on clicking ok on sending mail for forgot password.	
	SetTimer(TIMER_EMAILSENDINGFINISH, 200, NULL);	
	
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonEncCancel                                                     
*  Description    :	Close password dialog .
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 20th April 2014
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnBnClickedButtonEncCancel()
{
	OnCancel();
}

/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within 
					the CWnd object.
*  Author Name    :	Neha Gharge
*  SR_NO		  :
*  Date           : 4th Dec 2013
**********************************************************************************************************/
BOOL CISpyPasswordPopUpDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( iCtrlID == IDC_STATIC_ENCRYP_FORGOT_PASSWORD_LINK || iCtrlID == IDC_BUTTON_ENC_SEND || iCtrlID == IDC_BUTTON_ENC_CANCEL )
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
*  Function Name  :	OnClose                                                     
*  Description    : The framework calls this member function as a signal 
				   that the CWnd or an application is to terminate.
*  Author Name    :	Neha Gharge
*  SR_NO		  :
*  Date           : 4th Dec 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnClose()
{
	KillTimer(TIMER_EMAILSENDINGFINISH);
	OnCancel();
	CJpegDialog::OnClose();
}

/**********************************************************************************************************                     
*  Function Name  :	FetchForgotPassword                                                     
*  Description    :	Fetch the password from WRDWIZEVALREG.DLL
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th Dec 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::FetchForgotPassword()
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
	wcscat_s( szTemp, TEXT("\\WRDWIZEVALREG.DLL") ) ;
	if( !PathFileExists( szTemp ) )
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_REGDLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		return ;
	}

	//if( !GetRegisteredFolderData1 )
	//{
	//	MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_REGDATA"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//	return ;
	//}

	DWORD			dwSize = 0x00 ,i = 0x00;
	BYTE			szPassWord[0x19] = {0} ; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
	TCHAR			szPass[0x20] = {0} ;
	
	theApp.m_lpLoadEmail(szPassWord, dwSize, IDR_RESDATA_DES, TEXT("WRDWIZRESDES") ) ;
	if( szPassWord[0] && dwSize)
	{
		while( szPassWord[i] != 0x00 )
		{
			szPass[i] = szPassWord[i] ;
			i++ ;
		}
		m_csGetForgotPassword.Format( TEXT("%s"), szPass ) ;
		bFOLDER = true ;
	}
}


/**********************************************************************************************************                     
*  Function Name  :	SendPassword2EmailID                                                     
*  Description    :	send fetch password to user registered email id
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th Dec 2013
**********************************************************************************************************/
DWORD CISpyPasswordPopUpDlg::SendPassword2EmailID(LPTSTR csEmail,LPTSTR csPassword)
{
	DWORD	dwRet = 0x01 ;

	TCHAR	szUserInfo[512] = {0} ;
	TCHAR	szResponse[512] = {0} ;
	CStringW csEncString = XOREncrypt(csPassword, static_cast<DWORD>(_tcslen(csPassword)));
	wsprintf(szUserInfo, L"http://wardwiz.com/email/send_email.php?param1=%s&param2=%s&param3=%s",csEmail, csEncString,L"0") ;

	AddLogEntry(L">>>> Sending Mail", 0, 0, true, FIRSTLEVEL);
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

	AddLogEntry(L">>>> Getting response", 0, 0, true, FIRSTLEVEL);
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

/**********************************************************************************************************                     
*  Function Name  :	XOREncrypt                                                     
*  Description    :	Encrypt data with xor logic
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
CString CISpyPasswordPopUpDlg::XOREncrypt(CString a_sValue,DWORD a_sPasswordSize)
{
	DWORD i = 0 , j = 0 ;
	
	BYTE	szPass[0x19] = {0} ; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
	unsigned char	chPass = 0x00 ;
	CString EncryptPassRet = _T("");
	TCHAR szPassword[0x20] = {0};

	for(i=0; i<a_sPasswordSize; i++ )
	{
		chPass = static_cast<char>(a_sValue.GetAt( i ));
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
*  Function Name  :	OnEnChangeEditReenterPassword                                                     
*  Description    :	edit box for reenter password
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnEnChangeEditReenterPassword()
{
	m_edtReEnterPassword.SetPasswordChar('*');
	m_edtReEnterPassword.GetWindowTextW(m_csEditText);
	//Name:Varada Ikhar, Date:15/01/2015
	//Version: 1.8.3.17
	//Description:Data Encryption password length should be reduced. min:6 max:24
	m_dwNoofTypingChar = m_edtReEnterPassword.LineLength();

	if(m_dwNoofTypingChar >= 25) //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
	{
		m_edtReEnterPassword.GetWindowTextW(m_csEditText);
		ValidatePassword(m_csEditText);
		m_dwNoofTypingChar = 0;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnClosePopup                                                     
*  Description    :	Close password dialog after showing messges. 
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnClosePopup()
{
	// TODO: Add your message handler code here and/or call default
	if(m_stDataEncPreLoader.IsPlaying())
	{
		m_stDataEncPreLoader.Stop();
	}
	if(MessageBoxW(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_EMAIL_QUESTION"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		::SuspendThread(m_Thread);
		::TerminateThread(m_Thread,0);
		m_stDataEncPreLoader.ShowWindow(SW_SHOW);
		m_stSendingMail.SetWindowTextW(L"Sending failed");
		m_stSendingMail.ShowWindow(SW_SHOW);
		m_btnEncSend.ShowWindow(FALSE);
		m_btnEncCancel.ShowWindow(FALSE);
		m_btnSendCancel.EnableWindow(false);
		m_btnSendCancel.ShowWindow(false);
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_FAILEDTOSEND_MAIL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONEXCLAMATION);
	}
	else 
	{
		if(!m_stDataEncPreLoader.IsPlaying())
		{
			m_stDataEncPreLoader.Draw();
		}
		::ResumeThread(m_Thread);
	}
	OnDialogClose();
}

/**********************************************************************************************************                     
*  Function Name  :	OnDialogClose                                                     
*  Description    :	Close password dialog. 
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnDialogClose()
{
	/*Issue No: Issue Desc: On Send Button click of Forgot password UI was getting crashed
	Nitin K.*/
	CJpegDialog::OnClose();
	OnCancel();
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnSendcancel                                                     
*  Description    :	Close password dialog. 
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnBnClickedBtnSendcancel()
{
	OnClosePopup();
}


/**********************************************************************************************************                     
*  Function Name  :	ShowHideControls4ForgotPassword                                                     
*  Description    :	Show and Hide controls after clicking on forgot password link 
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::ShowHideControls4ForgotPassword()
{
	m_btnEncSend.EnableWindow(false);
	m_stMailboxPic.ShowWindow(false);
	m_stEncEmailId.ShowWindow(false);
	m_stEncForgotPassword.ShowWindow(SW_HIDE);
	m_stDecPassMsg.ShowWindow(SW_HIDE);
	m_stEncryPassMsg.ShowWindow(SW_HIDE);
	m_btnEncCancel.ShowWindow(SW_HIDE);
	if(!m_stDataEncPreLoader.IsPlaying())
	{
		m_stDataEncPreLoader.Draw();
	}
	m_stDataEncPreLoader.ShowWindow(TRUE);
	m_stSendingMail.ShowWindow(SW_SHOW);
	m_btnSendCancel.EnableWindow(true);
	m_btnSendCancel.ShowWindow(SW_SHOW);
	
}


/**********************************************************************************************************                     
*  Function Name  :	StartEncThread                                                     
*  Description    :	Animation of process of sending password to email ID is shown in the thread and shows message
					of sending failed and susceed.
*  Author Name    :	Nitin
*  SR_NO		  :
*  Date           : 4th April 2013
**********************************************************************************************************/
DWORD WINAPI StartEncThread(LPVOID arg)
{

	CISpyPasswordPopUpDlg *pThis = (CISpyPasswordPopUpDlg*)arg;
	if(!pThis)
		return 0;

	CRect rect;
	pThis->GetClientRect(rect);
	// issue resolved by lalit kumawat 12-24-2014 ,issue :Wardwiz crashes on clicking ok on sending mail for forgot password.
	 pThis->m_bIsEmailSendingFinish = false;

	pThis->m_csPass = pThis->m_csGetForgotPassword.GetBuffer();

	DWORD dwRet = pThis->SendPassword2EmailID(pThis->m_csEmail,pThis->m_csPass);
	if(dwRet == 2)
	{
		if(pThis->m_stDataEncPreLoader.IsPlaying())
		{
			pThis->m_stDataEncPreLoader.Stop();
			pThis->m_stDataEncPreLoader.ShowWindow(TRUE);
			pThis->m_stSendingMail.SetWindowTextW(L"Sending completed");
			pThis->m_stSendingMail.ShowWindow(SW_SHOW);
			pThis->m_btnEncCancel.ShowWindow(SW_HIDE);
			pThis->m_btnEncSend.ShowWindow(false);
			pThis->m_btnSendCancel.ShowWindow(false);

		}
		pThis->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLODERLOCKER_SENT_MSG"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONINFORMATION);
		// issue resolved by lalit kumawat 12-24-2014 ,issue :Wardwiz crashes on clicking ok on sending mail for forgot password.	
		pThis->m_bIsEmailSendingFinish = true;
	
		
	}
	else
	{
		if(pThis->m_stDataEncPreLoader.IsPlaying())
		{
			pThis->m_stDataEncPreLoader.Stop();
			pThis->m_stDataEncPreLoader.ShowWindow(TRUE);
			pThis->m_stSendingMail.SetWindowTextW(L"Sending failed");
			pThis->m_stSendingMail.ShowWindow(SW_SHOW);
			pThis->m_btnEncCancel.ShowWindow(SW_HIDE);
			pThis->m_btnEncSend.ShowWindow(false);
			pThis->m_btnSendCancel.ShowWindow(false);

		}
		pThis->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_FOLDER_LOCKER_MESSAGE_FAILEDTOSEND_MAIL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONEXCLAMATION);
		// issue resolved by lalit kumawat 12-24-2014 ,issue :Wardwiz crashes on clicking ok on sending mail for forgot password.				
		pThis->m_bIsEmailSendingFinish = true;
		
	  
		
	}
   
	return 0;
}

// issue resolved by lalit kumawat 12-24-2014 ,issue :Wardwiz crashes on clicking ok on sending mail for forgot password.
/**********************************************************************************************************                     
*  Function Name  :	OnTimer                                                     
*  Description    :	This function continously check email sending finish or not ,if email sending finish then terminate the 
					email sending thread and close PasswordPopUpDlg					
*  Author Name    :	Lalit kumawat
*  SR_NO		  :
*  Date           : 12-24-2014
**********************************************************************************************************/
void CISpyPasswordPopUpDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == TIMER_EMAILSENDINGFINISH)
		{
			if(m_bIsEmailSendingFinish)
			{
			 if(m_Thread!=NULL)
				{
					TerminateThread(m_Thread,0);
					m_stDataEncPreLoader.ShowWindow(SW_HIDE);
				}

			 OnClose();
		 }
	   }
	CJpegDialog::OnTimer(nIDEvent);
}
