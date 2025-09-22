// SettingsPassChangeDlg.cpp : implementation file
/****************************************************
*  Program Name: SettingsPassChangeDlg.cpp                                                                                                    
*  Description: setting for chaning password of folder locker and data encryption.
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 27 May 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "SettingsPassChangeDlg.h"
#define		IDR_RESDATA_DES_DATA_ENC			5000
#define		IDR_RESDATA_DES_FOLD_LOCKER			7000
#define		IDR_REGDATA							2000

# define IDS_DATA_ENC				"Data Encryption"
# define IDR_DATA_ENC				0
# define IDS_FOLDER_LOCK			"Folder Locker"
# define IDR_FOLDER_LOCK			1
// CSettingsPassChangeDlg dialog

IMPLEMENT_DYNAMIC(CSettingsPassChangeDlg, CDialog)
CString		g_csPassword;

/***************************************************************************************************                    
*  Function Name  : CSettingsPassChangeDlg                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
CSettingsPassChangeDlg::CSettingsPassChangeDlg(CWnd* pParent /*=NULL*/)
	:  CJpegDialog(CSettingsPassChangeDlg::IDD, pParent)
{

}

/***************************************************************************************************                    
*  Function Name  : ~CSettingsPassChangeDlg                                                     
*  Description    : D'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
CSettingsPassChangeDlg::~CSettingsPassChangeDlg()
{
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_cbSelectType);
	DDX_Control(pDX, IDC_BUTTON_OK, m_btnOk);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_OLD_PASS, m_stOldPass);
	DDX_Control(pDX, IDC_STATIC_NEW_PASS, m_stNewPass);
	DDX_Control(pDX, IDC_STATIC_CONF_PASS, m_stConfPass);
	DDX_Control(pDX, IDC_EDIT_OLD_PASS, m_edtOldPass);
	DDX_Control(pDX, IDC_EDIT_NEW_PASS, m_edtNewPass);
	DDX_Control(pDX, IDC_EDIT_CONF_PASS, m_edtConfPass);
	DDX_Control(pDX, IDC_STATIC_SELECT_TYPE, m_stChangePassFor);
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                    
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CSettingsPassChangeDlg, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CSettingsPassChangeDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CSettingsPassChangeDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CSettingsPassChangeDlg::OnBnClickedButtonOk)
	ON_EN_CHANGE(IDC_EDIT_OLD_PASS, &CSettingsPassChangeDlg::OnEnChangeEditOldPass)
	ON_EN_CHANGE(IDC_EDIT_CONF_PASS, &CSettingsPassChangeDlg::OnEnChangeEditConfPass)
	ON_EN_CHANGE(IDC_EDIT_NEW_PASS, &CSettingsPassChangeDlg::OnEnChangeEditNewPass)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CSettingsPassChangeDlg::OnCbnSelchangeComboType)
END_MESSAGE_MAP()


// CSettingsPassChangeDlg message handlers
/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global 
					dialog-box procedure common to all Microsoft Foundation Class Library
					dialog boxes
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BOOL CSettingsPassChangeDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_CHANGE_PASS), _T("JPG")))
	{
		::MessageBox(NULL, L"Failed", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
	}

	Draw();

	CRect rect;
	this->GetClientRect(rect);
	CRgn		 rgn;
	rgn.CreateRoundRectRgn(rect.left-1, rect.top, rect.right-2, rect.bottom-11,0,0);
	this->SetWindowRgn(rgn, TRUE);

	m_btnClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnClose.SetWindowPos(&wndTop,rect.left + 385,0,26,17,SWP_NOREDRAW);
	
	m_stChangePassFor.SetWindowPos(&wndTop,rect.left+30 ,83,120,32, SWP_NOREDRAW);
	m_stChangePassFor.SetFont(&theApp.m_fontWWTextNormal);
	m_stChangePassFor.SetBkColor(RGB(255,255,255));

	m_cbSelectType.SetWindowPos(&wndTop,rect.left + 170,80,220,17,SWP_NOREDRAW);
	m_cbSelectType.AddString((CString)IDS_DATA_ENC);
//	m_cbSelectType.AddString(m_objwardwizLangManager.GetString(L"IDS_SETTINGS_GENERAL_CHANGE_PASS_DATA_ENC"));
	if(theApp.m_dwProductID != 1)
	{
		m_cbSelectType.AddString((CString)IDS_FOLDER_LOCK);
	}
//	m_cbSelectType.AddString(m_objwardwizLangManager.GetString(L"IDS_SETTINGS_GENERAL_CHANGE_PASS_FOLDER_LOCK"));
	m_cbSelectType.SetCurSel(0);

	m_stOldPass.SetWindowPos(&wndTop,rect.left+30 ,116,100,21, SWP_NOREDRAW);
	m_stOldPass.SetFont(&theApp.m_fontWWTextNormal);
	m_stOldPass.SetBkColor(RGB(255,255,255));

	

	m_stNewPass.SetWindowPos(&wndTop,rect.left+30 ,158,100,21, SWP_NOREDRAW);
	m_stNewPass.SetFont(&theApp.m_fontWWTextNormal);
	m_stNewPass.SetBkColor(RGB(255,255,255));

	m_edtNewPass.SetWindowPos(&wndTop,rect.left+170 ,155,220,21, SWP_NOREDRAW);

	m_stConfPass.SetWindowPos(&wndTop,rect.left+30 ,197,130,21, SWP_NOREDRAW);
	m_stConfPass.SetFont(&theApp.m_fontWWTextNormal);
	m_stConfPass.SetBkColor(RGB(255,255,255));

	m_edtConfPass.SetWindowPos(&wndTop,rect.left+170 ,195,220,21, SWP_NOREDRAW);

	m_btnCancel.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnCancel.SetWindowPos(&wndTop,rect.left+333 ,225,57,21, SWP_NOREDRAW);
	m_btnCancel.SetTextColorA(BLACK,1,1);
	m_btnCancel.SetFont(&theApp.m_fontWWTextNormal);

	m_btnOk.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnOk.SetWindowPos(&wndTop,rect.left+266 ,225,57,21, SWP_NOREDRAW);
	m_btnOk.SetTextColorA(BLACK,1,1);
	m_btnOk.SetFont(&theApp.m_fontWWTextNormal);

	
	/*	ISSUE NO - 889 NAME - NITIN K. TIME - 3rd July 2014 */
	m_edtConfPass.SetWindowPos(&wndTop,rect.left+170 ,195,220,21, SWP_NOREDRAW);
	m_edtNewPass.SetWindowPos(&wndTop,rect.left+170 ,155,220,21, SWP_NOREDRAW);
	m_edtOldPass.SetWindowPos(&wndTop,rect.left+170 ,115,220,21, SWP_NOREDRAW);
//	m_edtOldPass.SetFocus();
	
	m_bOldPass = false;
	CheckOldPassword();
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
BOOL CSettingsPassChangeDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonClose                                                     
*  Description    : Close dialog
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::OnBnClickedButtonClose()
{
	OnCancel();
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonCancel                                                     
*  Description    : Closes dialog
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::OnBnClickedButtonCancel()
{
	OnCancel();
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonOk                                                     
*  Description    : It will check old password is correct or not and then allow to add new password
					then new password get replaced.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::OnBnClickedButtonOk()
{
	

	if(m_cbSelectType.GetCurSel() == IDR_DATA_ENC)
	{
		//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
		TCHAR		szOldPass[0x19] = {0} ;
		m_edtOldPass.GetWindowTextW( szOldPass, 25 ) ;
		if(!szOldPass[0])
		{
			if(!m_bOldPass)
			{
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_OLD_PASS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
				m_edtOldPass.SetWindowTextW(L"");
				m_edtNewPass.SetWindowTextW(L"");
				m_edtConfPass.SetWindowTextW(L"");
				m_edtOldPass.SetFocus();
				return;
			}
		}

		TCHAR		szNewPass[0x19] = {0} ;//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12

		CString		strPass("") ;
		//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
		TCHAR		szPassRenter[0x19] = {0};

		m_edtNewPass.GetWindowTextW( szNewPass, 25 ) ;
		m_edtConfPass.GetWindowTextW(szPassRenter,25);
		
		if(!szNewPass[0])
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_NEW_PASS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtNewPass.SetFocus();
			return;
		}

		//Varada Ikhar, Date: 04-03-2015, Issue: If user does not enter password in Re-enter Password field, message needs to be dispplayed & it should get the foccus without clearing password field.
		if (szNewPass[0])
		{
			if (!szPassRenter[0] )
			{
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PASSPOPUP_RE_ENTER_PASSWORD_TEXT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
				m_edtConfPass.SetWindowTextW(TEXT(""));
				m_edtConfPass.SetFocus();
				return;
			}
		}
		
		if(_tcscmp(szNewPass,szPassRenter))
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_SAME_PASS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			//m_edtNewPass.SetWindowTextW(L""); //Varada Ikhar, Date:05-03-2015
			m_edtConfPass.SetWindowTextW(L"");
			//m_edtNewPass.SetFocus(); //Varada Ikhar, Date:05-03-2015
			m_edtConfPass.SetFocus();
			return;
		}
		//Varada Ikhar, Date: 04-03-2015, Issue: If user does not enter password in Re-enter Password field, message needs to be dispplayed & it should get the foccus without clearing password field.
		m_edtNewPass.GetWindowTextW(m_csEditText);
		if (ValidatePassword(m_csEditText))
		{
			m_edtNewPass.SetFocus();
			return;
		}

		FetchForgotPasswordForDataEnc(IDR_RESDATA_DES_DATA_ENC );
		if(szOldPass != m_csGetForgotPassword)
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_CORRECT_OLD_PASS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			m_edtOldPass.SetWindowTextW(L"");
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtOldPass.SetFocus();
			return;
		}
		else if(!_tcscmp(szOldPass,szNewPass))
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_OLD_NEW_PASS_SAME"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			//m_edtOldPass.SetWindowTextW(L""); //Varada Ikhar, Date:05-03-2015
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtNewPass.SetFocus();
			return;
		}

		else
		{
			DWORD			i = 0x00 ;
			DWORD			iPassLen = 0x00 ;
			DWORD			dwSize = 0x00 ;

			unsigned char	chPass = 0x00 ;
			BYTE			szPassWord[0x19] = {0} ;//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
			TCHAR			szPass[0x20] = {0} ;


			i = 0x00 ;
			m_edtNewPass.GetWindowTextW(m_csEditText);
			iPassLen = m_csEditText.GetLength() ;
			if( !dwSize ) 
			{
				BYTE	szPass[0x19] = {0} ; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12

				for(i=0; i<iPassLen; i++ )
				{
					chPass = static_cast<char>(m_csEditText.GetAt( i ));
					szPass[i] = chPass ;
				}
		
				if(!SendRegisteredData2Service(ADD_REGISTRATION_DATA, szPass, iPassLen, IDR_RESDATA_DES_DATA_ENC, TEXT("WRDWIZRESDES")))
				{
					AddLogEntry(L"### Dataencryption : Error in ADD_REGISTRATION_DATA in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_TRYAGAIN"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					return ;
				}
			}

			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PASS_UPDATE_SUCCESS_CRYPT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONINFORMATION);
			m_edtOldPass.SetWindowTextW(L"");
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtOldPass.SetFocus();
		}
		m_csGetForgotPassword = L"";
	}
	else if(m_cbSelectType.GetCurSel() == IDR_FOLDER_LOCK)
	{
		//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
		TCHAR		szOldPass[0x19] = {0} ;
		m_edtOldPass.GetWindowTextW( szOldPass,25 ) ;
		if(!szOldPass[0])
		{
			if(!m_bOldPass)
			{
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_OLD_PASS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
				m_edtOldPass.SetWindowTextW(L"");
				m_edtNewPass.SetWindowTextW(L"");
				m_edtConfPass.SetWindowTextW(L"");
				m_edtOldPass.SetFocus();
				return;
			}
		}

		TCHAR		szNewPass[0x19] = {0} ;//Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12

		CString		strPass("") ;
		
		TCHAR		szPassRenter[0x19] = {0}; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12

		m_edtNewPass.GetWindowTextW( szNewPass, 25 ) ; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
		m_edtConfPass.GetWindowTextW(szPassRenter,25); //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
		
		if(!szNewPass[0])
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_NEW_PASS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtNewPass.SetFocus();
			return;
		}

		m_edtNewPass.GetWindowTextW(m_csEditText);
		if(ValidatePassword(m_csEditText))
		{
			m_edtNewPass.SetFocus();
			return;
		}
		
		if(_tcscmp(szNewPass,szPassRenter))
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_SAME_PASS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtNewPass.SetFocus();
			return;
		}

		FetchForgotPasswordForDataEnc(IDR_RESDATA_DES_FOLD_LOCKER);
		if(szOldPass != m_csGetForgotPassword)
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_CORRECT_OLD_PASS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			m_edtOldPass.SetWindowTextW(L"");
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtOldPass.SetFocus();
			return;
		}
		else if(!_tcscmp(szOldPass,szNewPass))
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_OLD_NEW_PASS_SAME"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtNewPass.SetFocus();
			return;
		}
		else
		{
			DWORD			i = 0x00 ;
			DWORD			iPassLen = 0x00 ;
			DWORD			dwSize = 0x00 ;

			unsigned char	chPass = 0x00 ;
			BYTE			szPassWord[0x19] = {0} ; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
			TCHAR			szPass[0x20] = {0} ;


			i = 0x00 ;
			m_edtNewPass.GetWindowTextW(m_csEditText);
			iPassLen = m_csEditText.GetLength() ;
			if( !dwSize ) 
			{
				BYTE	szPass[0x19] = {0} ; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12

				for(i=0; i<iPassLen; i++ )
				{
					chPass = static_cast<char>(m_csEditText.GetAt(i));
					szPass[i] = chPass ;
				}
		
				if(!SendRegisteredData2Service(ADD_REGISTRATION_DATA, szPass, iPassLen, IDR_RESDATA_DES_FOLD_LOCKER, TEXT("WRDWIZOPTIONRESDES")))
				{
					AddLogEntry(L"### Dataencryption : Error in ADD_REGISTRATION_DATA in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_TRYAGAIN"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					return ;
				}
			}

			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PASS_UPDATE_SUCCESS_FOLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONINFORMATION);
			m_edtOldPass.SetWindowTextW(L"");
			m_edtNewPass.SetWindowTextW(L"");
			m_edtConfPass.SetWindowTextW(L"");
			m_edtOldPass.SetFocus();
		}
		m_csGetForgotPassword = L"";
	}
	OnOK();
}

/***************************************************************************************************                    
*  Function Name  : OnEnChangeEditOldPass                                                     
*  Description    : Password character for old password
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::OnEnChangeEditOldPass()
{
	m_edtOldPass.SetPasswordChar('*');
}

/***************************************************************************************************                    
*  Function Name  : OnEnChangeEditConfPass                                                     
*  Description    : password character for new confirmation password
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::OnEnChangeEditConfPass()
{
	m_edtConfPass.SetPasswordChar('*');
	//Name:Varada Ikhar, Date:15/01/2015
	//Version: 1.8.3.17
	//Description:Data Encryption password length should be reduced. min:6 max:24
	m_edtConfPass.GetWindowTextW(m_csEditText);

	m_dwNoofTypingChar = m_edtConfPass.LineLength();

	if(m_dwNoofTypingChar >= 25) //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
	{
		m_edtConfPass.GetWindowTextW(m_csEditText);
		ValidatePassword(m_csEditText);
		m_dwNoofTypingChar = 0;
	}
}

/***************************************************************************************************                    
*  Function Name  : OnEnChangeEditNewPass                                                     
*  Description    : password character for new password , give it for validation.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::OnEnChangeEditNewPass()
{
	m_edtNewPass.SetPasswordChar('*');
	m_edtNewPass.GetWindowTextW(m_csEditText);

	m_dwNoofTypingChar = m_edtNewPass.LineLength();

	if(m_dwNoofTypingChar >= 25) //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
	{
		m_edtNewPass.GetWindowTextW(m_csEditText);
		ValidatePassword(m_csEditText);
		m_dwNoofTypingChar = 0;
	}
}

/***************************************************************************************************                    
*  Function Name  : ValidatePassword                                                     
*  Description    : Validating given password
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CSettingsPassChangeDlg::ValidatePassword(CString csPassword)
{
	int			i,count=0 ;
	CString		csInputKey ;
	csInputKey = csPassword;

	int		Validlenght = csInputKey.GetLength();

	m_dwNoofTypingChar = 0;
	if(Validlenght==0)
	{
		//Varada Ikhar, Date:03/02/2015, Version:1.8.3.6, Issue: Message box for password validation comes as a seperate window.
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENTER_PSSWRD"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
		//PopUpDialog();
		return 1 ;
	}
	
		if(Validlenght<=5) //Name:Varada Ikhar, Date:15/01/2015 Version: 1.8.3.17
		//Description:Data Encryption password length should be reduced. min:6 max:24
		{
			//Varada Ikhar, Date:03/02/2015, Version:1.8.3.6, Issue: Message box for password validation comes as a seperate window.
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALID5"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			m_edtNewPass.SetWindowTextW( TEXT("") ) ;
			//Varada Ikhar, Date:28/02/2015, Issue:In Settings->General->change password->enter old password->enter new password in incorrect format->pop up appears->click ok->only new password field is getting refreshed where as confirm password not.
			m_edtConfPass.SetWindowTextW(_TEXT(""));
			
			//PopUpDialog();
			return 2 ;
		}
		if(Validlenght>=25) //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
		{
			//Varada Ikhar, Date:03/02/2015, Version:1.8.3.6, Issue: Message box for password validation comes as a seperate window.
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALID25"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			m_edtNewPass.SetWindowTextW( TEXT("") ) ;
			//Varada Ikhar, Date:28/02/2015, Issue:In Settings->General->change password->enter old password->enter new password in incorrect format->pop up appears->click ok->only new password field is getting refreshed where as confirm password not.
			m_edtConfPass.SetWindowTextW(_TEXT("")); 
			//PopUpDialog();
			return 3 ;
		}
		if((csInputKey.FindOneOf(L"~`!@#$%^&*()_-+={}[]|\?/.,':;<>")==-1))
		{
			//Varada Ikhar, Date:03/02/2015, Version:1.8.3.6, Issue: Message box for password validation comes as a seperate window.
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALIDSPL"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			m_edtNewPass.SetWindowTextW( TEXT("") ) ;
			//Varada Ikhar, Date:28/02/2015, Issue:In Settings->General->change password->enter old password->enter new password in incorrect format->pop up appears->click ok->only new password field is getting refreshed where as confirm password not.
			m_edtConfPass.SetWindowTextW(_TEXT(""));
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
			m_edtNewPass.SetWindowTextW( TEXT("") ) ;
			//Varada Ikhar, Date:28/02/2015, Issue:In Settings->General->change password->enter old password->enter new password in incorrect format->pop up appears->click ok->only new password field is getting refreshed where as confirm password not.
			m_edtConfPass.SetWindowTextW(_TEXT(""));
			//Varada Ikhar, Date:03/02/2015, Version:1.8.3.6, Issue: Message box for password validation comes as a seperate window.
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FLDRLCKPSSWRD_VALIDNUM"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
			//PopUpDialog();
			return 5 ;
		}
	
	/*else 
	{
		MessageBox(L"Invalid Password\nPlease try again",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
		return 2;
	}*/

return 0 ;
}

/***************************************************************************************************                    
*  Function Name  : FetchForgotPasswordForDataEnc                                                     
*  Description    : fetch pasdword for data encryption.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::FetchForgotPasswordForDataEnc(DWORD PassType)
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
	if(PassType == IDR_RESDATA_DES_DATA_ENC)
	{
		theApp.m_lpLoadEmail(szPassWord, dwSize, IDR_RESDATA_DES_DATA_ENC, TEXT("WRDWIZRESDES") ) ;//WRDWIZOPTIONRESDES
	}
	else if(PassType == IDR_RESDATA_DES_FOLD_LOCKER)
	{
		theApp.m_lpLoadEmail(szPassWord, dwSize, IDR_RESDATA_DES_FOLD_LOCKER, TEXT("WRDWIZOPTIONRESDES") ) ;//WRDWIZOPTIONRESDES
	}

	if( szPassWord[0] && dwSize)
	{
		while( szPassWord[i] != 0x00 )
		{
			szPass[i] = szPassWord[i] ;
			i++ ;
		}
		m_csGetForgotPassword.Format( TEXT("%s"), szPass ) ;
//		bFOLDER = true;
	}
}

/***************************************************************************************************                    
*  Function Name  : SendRegisteredData2Service                                                     
*  Description    : Send the password to service to store into WrdWizEvalreg.dll
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
bool CSettingsPassChangeDlg::SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = dwType;
	memcpy ( szPipeData.byData, lpResBuffer, dwResSize);
	szPipeData.dwValue = dwResSize;
	szPipeData.dwSecondValue = dwResType;
	wcscpy_s(szPipeData.szFirstParam, pResName);
	
	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(szPipeData.dwValue != 1)
	{
		return false;
	}

	return true;
}

/***************************************************************************************************                    
*  Function Name  : CheckOldPassword                                                     
*  Description    : Checking old password is correct or not
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::CheckOldPassword()
{
	int Selection = m_cbSelectType.GetCurSel();
	if(Selection == 0)
	{
		FetchForgotPasswordForDataEnc(IDR_RESDATA_DES_DATA_ENC);
		if(m_csGetForgotPassword.GetLength() == 0)
		{
			m_bOldPass = true;
			m_edtOldPass.EnableWindow(false);
			m_edtNewPass.SetFocus();
		}
		else
		{
			m_bOldPass = false;
			m_edtOldPass.EnableWindow(true);
			m_edtOldPass.SetFocus();
		}
		m_csGetForgotPassword = L"";
	}
	if(Selection == 1)
	{
		FetchForgotPasswordForDataEnc(IDR_RESDATA_DES_FOLD_LOCKER);
		if(m_csGetForgotPassword.GetLength() == 0)
		{
			m_bOldPass = true;
			m_edtOldPass.EnableWindow(false);
			m_edtNewPass.SetFocus();
		}
		else
		{
			m_bOldPass = false;
			m_edtOldPass.EnableWindow(true);
			m_edtOldPass.SetFocus();
		}
		m_csGetForgotPassword = L"";
	}
}

/***************************************************************************************************                    
*  Function Name  : OnCbnSelchangeComboType                                                     
*  Description    : on change of combo type option
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
void CSettingsPassChangeDlg::OnCbnSelchangeComboType()
{
	CheckOldPassword();
}
