/**********************************************************************************************************
Program Name          : CRegistrationDlg.cpp
Description           : This class contains the functionality for registering the product (Trial, Online, Offline) activation
Author Name			  : Nitin Kolapkar
Date Of Creation      : 27th May 2016
Version No            : 2.0.0.1
Modification Log      :
***********************************************************************************************************/
#pragma once

#include "stdafx.h"
#include "WardWizRegistration.h"
#include "ITinRegWrapper.h"
//#include "RegistrationDlg.h"
//#include "RegistrationSecondDlg.h"
#include "xSkinButton.h"
#include <Shlwapi.h>
#include <intrin.h>
//#include <wininet.h>
#include <Wbemidl.h>
#include <comutil.h>
#include "RegistrationDlg.h"
#include "ISpyCommunicator.h"
#include "WinHttpClient.h"
#include "WardWizDatabaseInterface.h"

#pragma comment(lib, "Wbemuuid.lib" )
#pragma comment(lib, "comsuppw.lib" )
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Rpcrt4.lib")

#define ERROR_SUCCESS_MSG	L"0"
#define ERROR_FAILURE_MSG	L"1"

AVACTIVATIONINFO	m_RegDlg_ActInfo = {0} ;


GETREGISTRATIONDATA		GetRegistrationData = NULL;
GETINSTALLATIONCODE		g_GetInstallationCode = NULL;
VALIDATERESPONSE		g_ValidateResponse = NULL;

AVACTIVATIONINFO	g_ActInfo = { 0 };

#define				IDR_REGDATA					2000
#define				IDR_QUARDATA				2001


DWORD WINAPI SendRequestThread(LPVOID lpThreadParam);

LPCTSTR g_OptionsForComboBox[3]={L"Mr.",L"Ms.",L"Mrs."};

IMPLEMENT_DYNAMIC(CRegistrationDlg, CDialog)

HWINDOW   CRegistrationDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CRegistrationDlg::get_resource_instance() { return theApp.m_hInstance; }

CRegistrationDlg::CRegistrationDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CRegistrationDlg::IDD, pParent)
	, m_pBoldFont(NULL)
	, m_pTextFont(NULL)
	, m_pNoteFont(NULL)
	, m_bIncorrectEmail(false)
	, m_bIncorrectContact(false)
	, m_bInvalidConact(false)
	, m_bConfirmEmail(false)
	, m_hOfflineNextEvent(NULL)
{
	memset(&m_szBIOSNumber, 0, sizeof(m_szBIOSNumber));
	memset(&m_szDealerCode, 0, sizeof(m_szDealerCode));
}

CRegistrationDlg::~CRegistrationDlg()
{
	if(m_pBoldFont != NULL)
	{
		delete m_pBoldFont;
		m_pBoldFont = NULL;
	}

	if(m_pTextFont != NULL)
	{
		delete m_pTextFont;
		m_pTextFont = NULL;
	}
	if(m_pNoteFont!= NULL)
	{
		delete m_pNoteFont;
		m_pNoteFont = NULL;
	}
}

void CRegistrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FIRSTNAME, m_stFirstName);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_stTitle);
	DDX_Control(pDX, IDC_STATIC_LASTNAME, m_stLastName);
	DDX_Control(pDX, IDC_STATIC_EMAIL_ID, m_stEmailID);
	DDX_Control(pDX, IDC_STATIC_TELEPHONENO, m_stTelephoneNo);
	DDX_Control(pDX, IDC_STATIC_REQUIREDFIELD, m_stRequiredFields);
	DDX_Control(pDX, IDC_STATIC_REGISTRATIONKEY, m_stRegistrationKey);
	DDX_Control(pDX, IDC_CHECK_NEWSLETTER, m_chkNewsLetter);
	//DDX_Control(pDX, IDC_EDIT_TITLE, m_edtForTitle);
	DDX_Control(pDX, IDC_EDIT_FIRSTNAME, m_edtFirstName);
	DDX_Control(pDX, IDC_EDIT_EMAILID, m_edtEmailID);
	DDX_Control(pDX, IDC_EDIT_REGISTATIONKEY, m_edtRegistrationKey);
	DDX_Control(pDX, IDC_EDIT_LASTNAME, m_edtLastName);
	DDX_Control(pDX, IDC_EDIT_TELEPHONENO, m_edtTelephoneNo);
	DDX_Control(pDX, IDC_BUTTON_REGISTRATION_NEXT, m_btnRegistrationNext);
	DDX_Control(pDX, IDC_BUTTON_REGISTRATION_CANCEL, m_btnRegistartionCancel);
	DDX_Control(pDX, IDC_BUTTON_MINIMIZE, m_btnMinimize);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_STATIC_INFO, m_stPersonalInformation);
	DDX_Control(pDX, IDC_STATIC_NEWSLETTER, m_stNewLetterText);
	DDX_Control(pDX, IDC_STATIC_PRELOADER, m_stPreloaderImage);
	DDX_Control(pDX, IDC_STATIC_CONNECTTOSERVER, m_stConnectToServer);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_stPicCorrect);
	DDX_Control(pDX, IDC_STATIC_SUCCESS, m_stSuccessMsg);
	DDX_Control(pDX, IDC_STATIC_REQUEST, m_stRequestText);
	DDX_Control(pDX, IDC_STATIC_REGFAILED, m_stFailedText);
	DDX_Control(pDX, IDC_STATIC_FAILED_PIC, m_stFailedPic);
	DDX_Control(pDX, IDC_COMBO_TITLE, m_comboTitle);
	DDX_Control(pDX, IDC_STATIC_KEYEXAMPLE, m_stForKeyExample);
	DDX_Control(pDX, IDC_STATIC_USER_TEXT, m_stUserText);
	DDX_Control(pDX, IDB_STATIC_REG_HEADER, m_stRegHeader);
	DDX_Control(pDX, IDC_STATIC_REG_HEADER, m_stRegHeaderText);
	DDX_Control(pDX, IDC_STATIC_LOGO, m_stLogo);
	DDX_Control(pDX, IDC_STATIC_REGISTRATION_RIGHTS, m_stRegistrationRights);
	DDX_Control(pDX, IDC_STATIC_EMAIL_ID_STAR, m_stEmailIdStar);
	DDX_Control(pDX, IDC_STATIC_STAR_CONTACTNO, m_stContactStar);
	DDX_Control(pDX, IDC_EDIT_REENTER_EMAIL, m_edtReEnterEmailID);
	DDX_Control(pDX, IDC_STATIC_CONFIRM_EMAIL, m_stConfirmEmail);
	DDX_Control(pDX, IDC_STATIC_STAR_CONFIRM_EMAIL, m_stConfirmEmailStar);
	DDX_Control(pDX, IDC_STATIC_SPLITIMAGE, m_stSplitImages);
}

// CRegistrationDlg message handlers
BEGIN_MESSAGE_MAP(CRegistrationDlg, CJpegDialog)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_REGISTRATION_NEXT, &CRegistrationDlg::OnBnClickedButtonRegistrationNext)
	ON_BN_CLICKED(IDC_BUTTON_REGISTRATION_CANCEL, &CRegistrationDlg::OnBnClickedButtonRegistrationCancel)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CRegistrationDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_MINIMIZE, &CRegistrationDlg::OnBnClickedButtonMinimize)
	ON_BN_CLICKED(IDC_CHECK_NEWSLETTER, &CRegistrationDlg::OnBnClickedCheckNewsletter)
	ON_WM_PAINT()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CRegistrationDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	//SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	//m_objRegistrationSecondDlg.Create(IDD_DIALOG_REGISTARTION_SECONGDLG, this);
	//m_objRegistrationSecondDlg.ShowWindow(SW_HIDE);

	ShowHideRegistration(true);
	theApp.ProductRenewalDetails();
	if(!RefreshString())
	{
		AddLogEntry(L">>> CRegistrationDlg::OnInitDialog :: Failed RefreshString()", 0, 0, true, FIRSTLEVEL);
	}
	HideAllElements();
	SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_REGISTRATION.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_REGISTRATION.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	root_el = root();
	SciterGetElementIntrinsicWidths(root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(root_el, pIntMinWidth, &pIntHeight);
	//GetWindowRect(rect);
	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	CenterWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CRegistrationDlg::OnBnClickedButtonRegistrationNext()
{
	m_bIncorrectEmail = false;
	m_bIncorrectContact = false;
	m_bInvalidConact = false;
	m_bConfirmEmail = false;
	bool flag=ValidationOfRegistrationForm();
	if(flag == 0 && m_bIncorrectEmail == true)
	{	
		m_stRequestText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_REQTEXT_EMAIL"));
		m_stRequestText.ShowWindow(SW_SHOW);
		m_stRequestText.RedrawWindow();
		return;
	}
	if (flag == 0 && m_bConfirmEmail == true)
	{
		m_stRequestText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_REQTEXT_EMAIL_CONFIRM"));
		m_stRequestText.ShowWindow(SW_SHOW);
		m_stRequestText.RedrawWindow();
		return;
	}
	if(flag == 0 && m_bIncorrectContact == true)
	{
		if (m_bInvalidConact)
		{
			m_stRequestText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_INVALID_CONTACT"));
		}
		else
		{
			m_stRequestText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_REQUIRE_CONTACTNO"));
		}
		
		m_stRequestText.ShowWindow(SW_SHOW);
		m_stRequestText.RedrawWindow();
		return;
	}
	else if(flag == 0)
	{
		m_stRequestText.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_REQTEXT_FIELDS"));
		m_stRequestText.ShowWindow(SW_SHOW);
		return;
	}
	//Niranjan Deshak.Issue No.-12 Need to change msg in Registration dialog as "This Email-ID will be used for any further communication".
	CString csMessage;
	if(theApp.m_dwProductID == BASIC)
	{	

		csMessage.Format(L"%s\n%s",theApp.m_objwardwizLangManager.GetString(L" IDS_REG_EMAIL_HINT_RECONFIRM"),theApp.m_objwardwizLangManager.GetString(L" IDS_REG_EMAIL_HINT_BASIC"),MAX_PATH);
	}
	else
	{
		csMessage.Format(L"%s\n%s",theApp.m_objwardwizLangManager.GetString(L"IDS_REG_EMAIL_HINT_RECONFIRM"),theApp.m_objwardwizLangManager.GetString(L" IDS_REG_EMAIL_HINT_BASIC"),MAX_PATH);
	}

	m_stRequestText.ShowWindow(SW_HIDE);

	

	TCHAR	szTitle[8] ={0} ;
	TCHAR	szUserFirstName[64] ={0} ;
	TCHAR	szUserLastName[64] ={0} ;
	TCHAR	szMobileNo[16] ={0} ;
	TCHAR	szEmailID[64] ={0} ;

	TCHAR	szKey[0x32] = { 0 };

	m_comboTitle.GetWindowTextW(szTitle, 8 ) ;
	m_edtFirstName.GetWindowTextW(szUserFirstName, 64 ) ;
	m_edtLastName.GetWindowTextW(szUserLastName, 64 ) ;
	m_edtEmailID.GetWindowTextW(szEmailID, 64 ) ;
	m_edtTelephoneNo.GetWindowTextW(szMobileNo, 16 ) ;

	//Added reenter button if it failes in any case.
	//Neha Gharge 10 Aug,2015
	ShowHideRegistration(false);
	SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_SEL_OPTION"));
	//m_objRegistrationSecondDlg.ShowHideSecondDlg(true);
	//m_objRegistrationSecondDlg.ShowWindow(SW_SHOW);
	//m_objRegistrationSecondDlg.m_bMisMatchedEmailID = false;

	memset(&m_RegDlg_ActInfo, 0x00, sizeof(AVACTIVATIONINFO) ) ;

	if( szTitle[0] )
		wcscpy(m_RegDlg_ActInfo.szTitle, szTitle ) ;

	if( szUserFirstName[0] )
		wcscpy(m_RegDlg_ActInfo.szUserFirstName, szUserFirstName ) ;

	if( szUserLastName[0] )
		wcscpy(m_RegDlg_ActInfo.szUserLastName, szUserLastName ) ;

	if( szEmailID[0] )
		wcscpy(m_RegDlg_ActInfo.szEmailID, szEmailID ) ;

	if( szMobileNo[0] )
		wcscpy(m_RegDlg_ActInfo.szMobileNo, szMobileNo ) ;


	CITinRegWrapper objReg;
	TCHAR szValue[0x80] = {0};
	DWORD dwSize = sizeof(szValue);
	if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"MVersion", szValue, dwSize) != 0)
	{
		AddLogEntry(L"### Error in GetRegistryValueData CRegistrationDlg::OnBnClickedButtonRegistrationNext", 0, 0, true, SECONDLEVEL);
		return;
	}
	_tcscpy_s(m_RegDlg_ActInfo.szClientID, 0x80, szValue);
}

void CRegistrationDlg::OnBnClickedButtonRegistrationCancel()
{
	if(theApp.m_hRegMutexHandle != NULL)	
	{
		CloseHandle(theApp.m_hRegMutexHandle);
		theApp.m_hRegMutexHandle = NULL;
	}
	m_stTelephoneNo.SetTextColor(RGB(0, 0, 0));
	m_stEmailID.SetTextColor(RGB(0,0,0));
	m_stConfirmEmail.SetTextColor(RGB(0, 0, 0));
	//m_objRegistrationSecondDlg.m_bMisMatchedEmailID = false;
	OnCancel();
}

void CRegistrationDlg::OnBnClickedButtonClose()
{
	HandleCloseButton();
}

bool CRegistrationDlg::HandleCloseButton()
{
	if(theApp.m_hRegMutexHandle)	
	{
		CloseHandle(theApp.m_hRegMutexHandle);
		theApp.m_hRegMutexHandle = NULL;
	}
	/*if(m_objRegistrationSecondDlg.m_bisThreadCompleted == true)
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_IN_PROCESS_WAIT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
		return false;
	}
	else
	{
		m_objRegistrationSecondDlg.ShutdownThread();
		OnCancel();
	}*/
	OnCancel();
	return true;
}

void CRegistrationDlg::OnBnClickedButtonMinimize()
{
	this->ShowWindow(SW_MINIMIZE);
}

bool CRegistrationDlg::ValidationOfRegistrationForm()
{
	bool Correctdata=1;
	if(!ValidateTitle())
	{
		m_stTitle.SetTextColor(RGB(238,0,0));
		Correctdata=0;
	}
	if(!ValidateFirstName())
	{
		m_stFirstName.SetTextColor(RGB(238,0,0));
		Correctdata=0;
	}
	if(!ValidateLastName())
	{
		m_stLastName.SetTextColor(RGB(238,0,0));
		Correctdata=0;
	}
	if(!ValidateTelephone())
	{
		m_stTelephoneNo.SetTextColor(RGB(238,0,0));
		Correctdata=0;
	}
	if(!ValidateEmailID())
	{
		m_stEmailID.SetTextColor(RGB(238,0,0));
		m_stConfirmEmail.SetTextColor(RGB(238, 0, 0));
		Correctdata=0;
	}
	if (!ConfirmEmailID())
	{
		m_stEmailID.SetTextColor(RGB(238, 0, 0));
		m_stConfirmEmail.SetTextColor(RGB(238, 0, 0));
		Correctdata = 0;
	}
	if(!Correctdata)
	{
		return false;
	}

	m_stFirstName.SetTextColor(RGB(0,0,0));
	m_stLastName.SetTextColor(RGB(0,0,0));
	return true;
}

bool CRegistrationDlg::ValidateTitle()
{
	TCHAR InputString[8];
	m_comboTitle.GetWindowTextW(InputString,8);
	if(!(_tcscmp(InputString,L"")))
	{
		return false;	
	}
	if(!(_tcscmp(InputString,theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MR"))))
	{
		m_stTitle.SetTextColor(RGB(00,00,00));
		return true;
	}
	if (!(_tcscmp(InputString, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MRS"))))
	{
		m_stTitle.SetTextColor(RGB(00,00,00));
		return true;
	}
	if(!(_tcscmp(InputString,theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MS"))))
	{
		m_stTitle.SetTextColor(RGB(00,00,00));
		return true;
	}
	return false;
}

bool CRegistrationDlg::ValidateFirstName()
{
	TCHAR InputString[32];
	int icount=0;
	m_edtFirstName.GetWindowTextW(InputString,32);
	int len = static_cast<int>(_tcslen(InputString));

	if(len > 32)
	{
		return false;
	}
	m_stFirstName.SetTextColor(RGB(0,0,0));
	return true;	
}

bool CRegistrationDlg::ValidateLastName()
{
	TCHAR InputString[32];
	int icount=0;
	m_edtLastName.GetWindowTextW(InputString,32);
	int len = static_cast<int>(_tcslen(InputString));
	if(len > 32)
	{
		return false;
	}
	m_stLastName.SetTextColor(RGB(0,0,0));
	return true;	
}

//neha Gharge 19th Aug,2015 
//Implemetation : Mobile number should not be a mandatory field. 
bool CRegistrationDlg::ValidateTelephone()
{
	TCHAR InputString[21];
	int icount=0;
	m_edtTelephoneNo.GetWindowTextW(InputString,16);
	int len = static_cast<int>(_tcslen(InputString));
	bool bIsInvalid = false;
	if (len < 8 && len >= 1)
	{
		m_bIncorrectContact = true;
		m_bInvalidConact = true;
		return false;
	}
	if(len > 16)
	{
		m_bIncorrectContact = true;
		return false;
	}
	//issue Mobile Number field is not validated.
	 //It is accepting even 1 digit number.The maximum accepted value should be 16 and minimum should be 10.
	// resolve by - lalit kumawat 4-21-2015
	int iZeroCountFrmStart = 0;
	int iOnesCountFrmStart = 0;

	for(int i=0;i<len;i++)
	{
		if(!(isdigit(InputString[i])))
		{
			icount++ ;
		}
		
		if (InputString[i] == '0' && i< 4)
		{
			iZeroCountFrmStart++;
		}
		else if (i < 4)
		{
			
			iZeroCountFrmStart = 0;
			if (InputString[i] == '1')
			{
				iOnesCountFrmStart++;
			}
			else
			{
				iOnesCountFrmStart = 0;
			}
		}
		
	}

	if ((iOnesCountFrmStart == 4 && iZeroCountFrmStart == 0) || (iOnesCountFrmStart == 0 && iZeroCountFrmStart == 4))
	{
		m_bIncorrectContact = true;
		m_bInvalidConact = true;
		return false;
	}
	if(icount > 1)
	{
		m_bIncorrectContact = true;
		return false;
	}

	m_stTelephoneNo.SetTextColor(RGB(0,0,0));
	return true;
}

bool CRegistrationDlg::ValidateEmailID()
{
	bool flag = true;
	CString inputEmail,TempEmail;
	TCHAR email[64];
	m_edtEmailID.GetWindowText(email,64);
	inputEmail=email;

	if(inputEmail == L"")
	{
		return false;
	}
	int i=0;
	int posDot,posAt,posAt2,posDotSec;

	posAt = inputEmail.ReverseFind(_T('@'));
	posAt2 = inputEmail.Find(_T('@'));
	i=0;


	if(posAt == posAt2)
	{
		int pos = inputEmail.Find(_T(".."));
		if(pos > -1)
		{
			m_bIncorrectEmail = true;
			return false;
		}
		pos=inputEmail.Find(_T(".@"));
		if(pos > -1)
		{
			m_bIncorrectEmail = true;
			return false;
		}
		pos = inputEmail.Find(_T(".")); //no dot at start of email id
		if(pos == 0)
		{
			m_bIncorrectEmail = true;
			return false;
		}
		pos = inputEmail.ReverseFind(_T('.')); //no dot at end of email id
		int length= inputEmail.GetLength();
		if(pos == length-1)
		{
			m_bIncorrectEmail = true;
			return false;
		}
		if(!CheckSpecialCharInEmailID(inputEmail))
		{
			m_bIncorrectEmail = true;
			return false;
		}
		flag=0;
		TempEmail = inputEmail.Right(inputEmail.GetLength() - posAt);

		posDotSec = TempEmail.Find(_T('.'));

		posDot = TempEmail.ReverseFind(_T('.'));
		if(posDot == -1)
		{
			m_bIncorrectEmail = true;
			return false;
		}
		int diffDot = posDot-posDotSec; 
		if(diffDot == 1)
		{
			m_bIncorrectEmail = true;
			return false;
		}

		posAt = TempEmail.Find(_T('@'));
		int diff = posDotSec-posAt;
		if(diff == 1)
		{
			m_bIncorrectEmail = true;
			return false;
		}

		if(TempEmail==L"")
		{
			m_bIncorrectEmail = true;
			return false;
		}

		if(posAt > -1)
		{
			posAt = inputEmail.Find(_T('@'));
			inputEmail.Truncate(posAt);
			if(inputEmail==L"")
			{
				m_bIncorrectEmail = true;
				return false;
			}
		}

	}
	else
	{
		flag=1;
	}
	if(posAt == -1)
	{
		flag=1;
	}

	if(!flag)
	{
		m_stEmailID.SetTextColor(RGB(0,0,0));
		return true;
	}
	else
	{
		m_bIncorrectEmail = true;
	}
	return (false);
}

bool CRegistrationDlg::CheckSpecialCharInEmailID(CString csEmailID)
{
	int pos = csEmailID.Find(L' '); //no spaces
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L":");
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L";");
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L"<");
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L">");
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L' ');
	if(pos > -1)
	{
		return false;
	}
	pos= csEmailID.Find(L"("); 
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L")");
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L"[");
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L"]");
	if(pos > -1)
	{
		return false;
	}
	pos = csEmailID.Find(L",");
	if(pos > -1)
	{
		return false;
	}
	return true;
}
void CRegistrationDlg::OnBnClickedCheckNewsletter()
{
	DWORD dwNewsletter = 1;
	int iCheck = m_chkNewsLetter.GetCheck();
	if(iCheck == 0)
	{
		dwNewsletter = 0;
	}
	else
	{
		dwNewsletter = 1;
	}

	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey, &hKey) != ERROR_SUCCESS)
	{
		AddLogEntry(L"### Unable to open registry key", 0, 0, true, SECONDLEVEL);
	}

	LONG setRes = RegSetValueEx (hKey ,L"dwAllowNewLetter", 0, REG_DWORD, (LPBYTE)&dwNewsletter, sizeof(DWORD));
	if(setRes != ERROR_SUCCESS)
	{
		AddLogEntry(L"### Error in Setting Registry CScanDlg::OnBnClickedCheckNewsPaper", 0, 0, true, SECONDLEVEL);
	}
}

void CRegistrationDlg::ReadRegistryOfNewsletter()
{
	HKEY hKey;
	LONG ReadReg;
	DWORD dwvalue;
	DWORD dwvalueSize = sizeof(DWORD);
	DWORD Chkvalue;
	DWORD dwType=REG_DWORD;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey, &hKey) != ERROR_SUCCESS)
	{
		AddLogEntry(L"### Unable to open registry key", 0, 0, true, SECONDLEVEL);
	}
	
	ReadReg=RegQueryValueEx(hKey,L"dwAllowNewLetter",NULL,&dwType,(LPBYTE)&dwvalue,&dwvalueSize);
	if(ReadReg == ERROR_SUCCESS)
	{
		Chkvalue=(DWORD)dwvalue;
		if(Chkvalue==0)
		{
			m_chkNewsLetter.SetCheck(0);
		}
		else
		{
			m_chkNewsLetter.SetCheck(1);
		}
	}
}

BOOL CRegistrationDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(!pWnd)
		return FALSE;
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
// iSUUE nO- 275 rAJIL Y 22/05/2014
	if( 
		iCtrlID == IDC_BUTTON_REGISTRATION_NEXT	||
		iCtrlID == IDC_BUTTON_REGISTRATION_CANCEL	||
		iCtrlID == IDC_BUTTON_MINIMIZE     ||
		iCtrlID == IDC_BUTTON_CLOSE ||
		iCtrlID == IDC_STATIC_TELEPHONENO
	   )
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if(csClassName == _T("Button") && m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}
	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}

DWORD CRegistrationDlg::GetCPUID( TCHAR *pszCPUID )
{
	__try
	{
		TCHAR	szData[0x10] = {0} ;
		int		b[4] = {0} ;

		wcscpy( pszCPUID, L"") ;

		for (int a = 0; a < 3; a++)
		{
			__cpuid(b, a) ;

			if( (a == 0 || a == 1) && b[0] )
			{
				wsprintf(szData, L"%X", b[0] ) ;
				wcscat(pszCPUID, szData) ;
			}

			if( a == 2 )
			{
				if( b[0] )
				{
					wsprintf(szData, L"%X", b[0] ) ;
					wcscat(pszCPUID, szData) ;
				}

				if( b[1] )
				{
					wsprintf(szData, L"%X", b[1] ) ;
					wcscat(pszCPUID, szData) ;
				}

				if( b[2] )
				{
					wsprintf(szData, L"%X", b[2] ) ;
					wcscat(pszCPUID, szData) ;
				}

				if( b[3] )
				{
					wsprintf(szData, L"%X", b[3] ) ;
					wcscat(pszCPUID, szData) ;
				}
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetCPUID", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

DWORD CRegistrationDlg::GetBIOSSerialNumber(TCHAR *pszBIOSSerialNumber )
{
	DWORD				dwRet = 0x00 ;
	try
	{
		static bool			bCoInitializeSecurityCalled = false;				

		HRESULT				hres = S_OK ;
		HRESULT				hr = S_OK ;
		IWbemLocator		*pLoc = NULL ;
		IWbemServices		*pSvc = NULL ;
		IEnumWbemClassObject* pEnumerator = NULL ;
		IWbemClassObject	*pclsObj = NULL ;
		ULONG uReturn		= 0 ;

		VARIANT				vtProp ;
		CString				hh = L"" ;

		hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if(!bCoInitializeSecurityCalled)
		{
			hres = CoInitializeSecurity(NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE,NULL) ;
			if( hres != S_OK )
			{
				ErrorDescription(hres);
				dwRet = 0x01 ;
				goto Cleanup ;
			}
			bCoInitializeSecurityCalled = true;
		}

		hres = CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator, (LPVOID *) &pLoc);
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),NULL,NULL,0,NULL,0,0,&pSvc);
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		hres = CoSetProxyBlanket(pSvc,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE);
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		hres = pSvc->ExecQuery(bstr_t("WQL"),bstr_t("SELECT * FROM Win32_BIOS"),WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,NULL,&pEnumerator);
		if( hres != S_OK )
		{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		while( pEnumerator )
		{
			hr = pEnumerator->Next(WBEM_INFINITE, 1,&pclsObj, &uReturn);
			if(0 == uReturn)
				break ;

			hr = pclsObj->Get( L"SerialNumber", 0, &vtProp, 0, 0);
			hh=vtProp.bstrVal;
			VariantClear(&vtProp);
			pclsObj->Release();
			if( hh.GetLength() )
			{
				wsprintf(pszBIOSSerialNumber, L"%s", hh.Trim() ) ;
				break ;
			}
		}
Cleanup:
		if( pSvc )
			pSvc->Release() ;

		if( pLoc )
			pLoc->Release() ;

		if( pEnumerator )
			pEnumerator->Release() ;

		CoUninitialize() ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetBIOSSerialNumber", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

DWORD CRegistrationDlg::RemoveCharsIfExists(TCHAR *pszBIOSSerialNumber, int iLen, int iSize, TCHAR chRemove )
{
	TCHAR	szTemp[56] = {0} ;

	if( (iLen <=0) || iLen > 56 )
		return 1 ;

	int i = 0x00, j = 0x00 ;

	for(i=0; i<iLen; i++ )
	{
		if( pszBIOSSerialNumber[i] != chRemove )
			szTemp[j++] = pszBIOSSerialNumber[i] ;
	}

	szTemp[j] = '\0' ;

	ZeroMemory(pszBIOSSerialNumber, iSize ) ;
	wcscpy(pszBIOSSerialNumber, szTemp ) ;

	return 0 ;
}

BOOL CRegistrationDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	if (pMsg->message == WM_CHAR || pMsg->message == WM_UNICHAR)
	{
		WindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

// issue no- 227 && 131 rajil yadav 22/05/2014
void CRegistrationDlg::ShowHideRegistration(bool bEnable)
{
	/*	ISSUE NO - 700 NAME - NITIN K. TIME - 15th June 2014 */
	m_stRequestText.SetWindowTextW(L"");
	m_stRequestText.ShowWindow(bEnable);
	m_stPersonalInformation.ShowWindow(bEnable);
	m_stTitle.ShowWindow(bEnable);
	m_comboTitle.ShowWindow(bEnable);
	m_stFirstName.ShowWindow(bEnable);
	m_edtFirstName.ShowWindow(bEnable);
	m_stLastName.ShowWindow(bEnable);
	m_edtLastName.ShowWindow(bEnable);
	m_stEmailID.ShowWindow(bEnable);
	m_edtEmailID.ShowWindow(bEnable);
	m_stRequiredFields.ShowWindow(bEnable);
	m_stTelephoneNo.ShowWindow(bEnable);
	m_edtTelephoneNo.ShowWindow(bEnable);
	m_chkNewsLetter.ShowWindow(SW_HIDE);
	m_stNewLetterText.ShowWindow(SW_HIDE);
	m_stSplitImages.ShowWindow(bEnable);
	m_btnRegistrationNext.ShowWindow(bEnable);
	m_btnRegistartionCancel.ShowWindow(bEnable);
	m_stUserText.ShowWindow(bEnable);
	m_edtReEnterEmailID.ShowWindow(bEnable);
	m_stConfirmEmail.ShowWindow(bEnable);
	if(bEnable)
	{
		/*if(m_objRegistrationSecondDlg.m_bMisMatchedEmailID == true)
		{
			m_stEmailID.SetTextColor(RGB(238,0,0));
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTER_REGISTERED_EMAIL_ID"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		}*/
	}
}


/***********************************************************************************************
  Function Name  : RefreshString
  Description    : this function is  called for setting the Text UI with different Language Support
  Author Name    : Nitin Kolapkar
  Date           : 29 April 2014
***********************************************************************************************/
BOOL CRegistrationDlg :: RefreshString()
{
	m_stRegHeaderText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_WELCOME_TEXT"));
	m_stRegHeaderText.SetFont(&theApp.m_fontWWTextTitle);	

	m_stPersonalInformation.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_PERSONAL_INFO"));
	m_stPersonalInformation.SetFont(&theApp.m_fontWWTextSubTitle);

	m_stRequiredFields.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_MANDETORY_FIELDS"));
	m_stRequiredFields.SetFont(&theApp.m_fontWWTextNormal);

	m_stTitle.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SALUTATION"));
	m_stTitle.SetFont(&theApp.m_fontWWTextNormal);

	m_stFirstName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_FIRST_NAME"));
	m_stFirstName.SetFont(&theApp.m_fontWWTextNormal);

	m_stLastName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_LAST_NAME"));
	m_stLastName.SetFont(&theApp.m_fontWWTextNormal);

	m_stEmailIdStar.SetWindowTextW(L"*");
	m_stEmailIdStar.SetFont(&theApp.m_fontWWTextNormal);
	
	m_stEmailID.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_EMAIL_ID"));
	m_stEmailID.SetFont(&theApp.m_fontWWTextNormal);

	m_stConfirmEmail.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_EMAIL_ID_CONFIRM"));
	m_stConfirmEmail.SetFont(&theApp.m_fontWWTextNormal);

	m_stContactStar.SetWindowTextW(L"*");
	m_stContactStar.SetFont(&theApp.m_fontWWTextNormal);

	m_stTelephoneNo.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_CONTACT_NO"));
	m_stTelephoneNo.SetFont(&theApp.m_fontWWTextNormal);
	
	m_stUserText.SetWindowText(L"");
	m_btnRegistrationNext.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_BUTTON_NEXT"));
	m_btnRegistrationNext.SetFont(&theApp.m_fontWWTextNormal);

	m_btnRegistartionCancel.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_BUTTON_CANCEL"));
	m_btnRegistartionCancel.SetFont(&theApp.m_fontWWTextNormal);

	if (theApp.m_objwardwizLangManager.GetSelectedLanguage() == GERMAN)
	{
		m_comboTitle.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MR"));
		m_comboTitle.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MRS"));
	}
	else
	{
		m_comboTitle.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MR"));
		m_comboTitle.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MS"));
		m_comboTitle.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MRS"));
	}
	m_comboTitle.SetCurSel(0);
	return true;	
}

/***************************************************************************
  Function Name  : SetRegistrationHeader
  Paramerter	 : CString csHeader
  Description    : This function will set the header
  Author Name    : Ramkrushna Shelke
  Date           : 23 May 2014
****************************************************************************/
void CRegistrationDlg::SetRegistrationHeader(CString csHeader)
{
	m_stRegHeaderText.SetWindowTextW(csHeader);
	Invalidate();
}

void CRegistrationDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}
/***************************************************************************
Function Name  : OnClose
Description    : It notify when wm_close event generate and close all necessary open handle.
Author Name    : Lalit kumawat
Date           : 4-10-2015
****************************************************************************/
//1.If we click on "Register Now" button then click alt + F4,again click on "Register Now" then it gives message registration is in process.
void CRegistrationDlg::OnClose()
{
	HandleCloseButton();

	CJpegDialog::OnClose();
}

/***************************************************************************
Function Name  : ProductRenewalDetails
Description    : After 1 Year In Registration Dialog User Info Should Appear and hide offline registration 
                 for german setup byDefault
Author Name    : Adil Sheikh 
Date           : 6th July 2016
****************************************************************************/
void CRegistrationDlg:: ProductRenewalDetails()
{
	int iOfflineDisabled = 0;
	CString csSalutation = L"";
	CString csIniFilePath = L"";
	CString csDealerCode = L"";
	CString csReferenceID = L"";
	CString csCountry = L"";
	CString csState = L"";
	CString csCity = L"";
	CString csPinCode = L"";
	CString csEngineerName = L"";
	CString csEngineerMobNo = L"";

	if (theApp.m_dwIsOffline== 1)
	{
		iOfflineDisabled = 1;
	}
	else
	{
		iOfflineDisabled = 2;
	}
	//New Implementation: After 1 Year In Registration Dialog User Info Should Appear byDefault
	//Implementated By : Nitin K. Date : 16th April 2015
	if (theApp.m_bIsProductRenewal)
	{
		m_comboTitle.SetCurSel(theApp.m_dwSalutation);
		m_edtFirstName.SetWindowTextW(theApp.m_csFirstName);
		m_edtLastName.SetWindowTextW(theApp.m_csLastName);
		m_edtEmailID.SetWindowTextW(theApp.m_csEmailID);
		m_edtReEnterEmailID.SetWindowTextW(theApp.m_csEmailID);
		m_edtTelephoneNo.SetWindowTextW(theApp.m_csContactNo);
		m_edtEmailID.EnableWindow(false);
		m_edtReEnterEmailID.EnableWindow(false);
		//m_objRegistrationSecondDlg.m_edtRegistrationkey.SetWindowTextW(theApp.m_csProductKeyNumber);

		csSalutation.Format(L"%d", theApp.m_dwSalutation);
		
		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		TCHAR szValue[MAX_PATH] = { 0 };
		TCHAR szReferenceID[MAX_PATH] = { 0 };
		TCHAR szCountry[MAX_PATH] = { 0 };
		TCHAR szState[MAX_PATH] = { 0 };
		TCHAR szCity[MAX_PATH] = { 0 };
		TCHAR szPinCode[MAX_PATH] = { 0 };
		TCHAR szEngineerName[MAX_PATH] = { 0 };
		TCHAR szEngineerMobNo[MAX_PATH] = { 0 };

		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			AddLogEntry(L"### CRegistrationDlg::ProductRenewalDetails::GetModulePath failed", 0, 0, true, SECONDLEVEL);
			return;
		}
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		GetPrivateProfileString(L"VBSETTINGS", L"DealerCode", L"", szValue, sizeof(szValue), szFullFilePath); 
		csDealerCode = szValue;

		GetPrivateProfileString(L"VBSETTINGS", L"ReferenceID", L"", szReferenceID, sizeof(szReferenceID), szFullFilePath);
		csReferenceID = szReferenceID;

		GetPrivateProfileString(L"VBSETTINGS", L"Country", L"", szCountry, sizeof(szCountry), szFullFilePath);
		csCountry = szCountry;

		GetPrivateProfileString(L"VBSETTINGS", L"State", L"", szState, sizeof(szState), szFullFilePath);
		csState = szState;

		GetPrivateProfileString(L"VBSETTINGS", L"City", L"", szCity, sizeof(szCity), szFullFilePath);
		csCity = szCity;

		GetPrivateProfileString(L"VBSETTINGS", L"PinCode", L"", szPinCode, sizeof(szPinCode), szFullFilePath);
		csPinCode = szPinCode;

		GetPrivateProfileString(L"VBSETTINGS", L"EngineerName", L"", szEngineerName, sizeof(szEngineerName), szFullFilePath);
		csEngineerName = szEngineerName;

		GetPrivateProfileString(L"VBSETTINGS", L"EngineerMobNo", L"", szEngineerMobNo, sizeof(szEngineerMobNo), szFullFilePath);
		csEngineerMobNo = szEngineerMobNo;
	}
	const sciter::value data[15] = { sciter::string(csSalutation), sciter::string(theApp.m_csFirstName),
		sciter::string(theApp.m_csLastName), sciter::string(theApp.m_csEmailID), sciter::string(theApp.m_csContactNo),
		sciter::string(theApp.m_csProductKeyNumber), (iOfflineDisabled), sciter::string(csDealerCode), sciter::string(csReferenceID), 
		sciter::string(csCountry), sciter::string(csState), sciter::string(csCity), sciter::string(csPinCode),
		sciter::string(csEngineerName), sciter::string(csEngineerMobNo) };
	sciter::value svArrUserDetails = sciter::value(data, 15);
	m_svFunSetUserDetailsCB.call(svArrUserDetails);
}

bool CRegistrationDlg::ConfirmEmailID()
{
	bool flag = true;
	CString csInputEmail, csReEnterEmail;
	TCHAR email[64];
	TCHAR ReEnteremail[64];
	m_edtEmailID.GetWindowText(email, 64);
	csInputEmail = email;
	m_edtReEnterEmailID.GetWindowText(ReEnteremail, 64);
	csReEnterEmail = ReEnteremail;
	if (csReEnterEmail == L"")
	{
		return false;
	}

	if (csInputEmail.Compare(csReEnterEmail) != 0)
	{
		m_bConfirmEmail = true;
		return false;
	}
	m_stConfirmEmail.SetTextColor(RGB(0, 0, 0));
	return flag;
}

/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : Handles Sciter and other Window calls
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
LRESULT CRegistrationDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;

	lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
	if (bHandled)      // if it was handled by the Sciter
		return lResult; // then no further processing is required.
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  : HideAllElements
*  Description    : Hides all Old UI elements
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
void CRegistrationDlg::HideAllElements()
{
	try
	{
		m_stFirstName.ShowWindow(SW_HIDE);
		m_stTitle.ShowWindow(SW_HIDE);
		m_stLastName.ShowWindow(SW_HIDE);
		m_stEmailID.ShowWindow(SW_HIDE);
		m_stTelephoneNo.ShowWindow(SW_HIDE);
		m_stRequiredFields.ShowWindow(SW_HIDE);
		m_stRegistrationKey.ShowWindow(SW_HIDE);
		m_chkNewsLetter.ShowWindow(SW_HIDE);
		m_edtFirstName.ShowWindow(SW_HIDE);
		m_edtEmailID.ShowWindow(SW_HIDE);
		m_edtRegistrationKey.ShowWindow(SW_HIDE);
		m_edtLastName.ShowWindow(SW_HIDE);
		m_edtTelephoneNo.ShowWindow(SW_HIDE);
		m_btnRegistrationNext.ShowWindow(SW_HIDE);
		m_btnRegistartionCancel.ShowWindow(SW_HIDE);
		m_btnMinimize.ShowWindow(SW_HIDE);
		m_btnClose.ShowWindow(SW_HIDE);
		m_stPersonalInformation.ShowWindow(SW_HIDE);
		m_stNewLetterText.ShowWindow(SW_HIDE);
		m_stPreloaderImage.ShowWindow(SW_HIDE);
		m_stConnectToServer.ShowWindow(SW_HIDE);
		m_stPicCorrect.ShowWindow(SW_HIDE);
		m_stSuccessMsg.ShowWindow(SW_HIDE);
		m_stRequestText.ShowWindow(SW_HIDE);
		m_stFailedText.ShowWindow(SW_HIDE);
		m_stFailedPic.ShowWindow(SW_HIDE);
		m_comboTitle.ShowWindow(SW_HIDE);
		m_stForKeyExample.ShowWindow(SW_HIDE);
		m_stUserText.ShowWindow(SW_HIDE);
		m_stRegHeader.ShowWindow(SW_HIDE);
		m_stRegHeaderText.ShowWindow(SW_HIDE);
		m_stLogo.ShowWindow(SW_HIDE);
		m_stRegistrationRights.ShowWindow(SW_HIDE);
		m_stEmailIdStar.ShowWindow(SW_HIDE);
		m_stContactStar.ShowWindow(SW_HIDE);
		m_edtReEnterEmailID.ShowWindow(SW_HIDE);
		m_stConfirmEmail.ShowWindow(SW_HIDE);
		m_stConfirmEmailStar.ShowWindow(SW_HIDE);
		m_stSplitImages.ShowWindow(SW_HIDE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::HideAllElements");
	}
}

/***************************************************************************************************
*  Function Name  : On_Close
*  Description    : to close the Application
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CRegistrationDlg::On_Close()
{
	try
	{
		HandleCloseButton();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_Close");
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_ClickTrialProduct
*  Description    : Trial activation of product
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CRegistrationDlg::On_ClickTrialProduct(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetRegStatusCB, SCITER_VALUE svFunSetNoInternetMsg, SCITER_VALUE svResponse)
{
	try
	{
		svArrUserDetails.isolate();
		bool bIsArray = false;
		bIsArray = svArrUserDetails.is_array();

		if (!bIsArray)
		{
			return false;
		}
		m_bActiveProduct = false;
		m_bTryProduct = true;
		m_bOnlineActivation = false;
		m_bOfflineActivation = false;
		m_svFunSetRegStatusCB = svFunSetRegStatusCB;
		m_svFunSetNoInternetMsg = svFunSetNoInternetMsg;
		m_csResponseData = svResponse.get(L"").c_str();
		PerformRegistration(svArrUserDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_ClickTrialProduct");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ClickActivateProduct
*  Description    : activation of product using product activation key
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CRegistrationDlg::On_ClickActivateProduct(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetRegStatusCB, SCITER_VALUE svFunSetNoInternetMsg, SCITER_VALUE svResponse)
{
	try
	{
		svArrUserDetails.isolate();
		bool bIsArray = false;
		bIsArray = svArrUserDetails.is_array();

		if (!bIsArray)
		{
			return false;
		}
		m_bOnlineActivation = true;
		m_bOfflineActivation = false;
		m_bTryProduct = false;
		m_bActiveProduct = true;
		m_svFunSetRegStatusCB = svFunSetRegStatusCB;
		m_svFunSetNoInternetMsg = svFunSetNoInternetMsg;
		m_csResponseData = svResponse.get(L"").c_str();
		PerformRegistration(svArrUserDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_ClickActivateProduct");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetOfflineActivationKey
*  Description    : Creates offline key for activaion and sends it to UI
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CRegistrationDlg::GetOfflineActivationKey(SCITER_VALUE svArrUserDetails, SCITER_VALUE svFunSetOfflineActKeyCB, SCITER_VALUE svFunSetNoInternetMsg)
{
	try
	{
		svArrUserDetails.isolate();
		bool bIsArray = false;
		bIsArray = svArrUserDetails.is_array();

		if (!bIsArray)
		{
			return false;
		}
		m_bOnlineActivation = false;
		m_bOfflineActivation = true;
		m_bTryProduct = false;
		m_bActiveProduct = true;
		m_svFunSetOfflineActKeyCB = svFunSetOfflineActKeyCB;
		m_svFunSetNoInternetMsg = svFunSetNoInternetMsg;
		PerformRegistration(svArrUserDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetOfflineActivationKey");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ClickOfflineRegistration
*  Description    : Registeres product using offline activation
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
json::value CRegistrationDlg::On_ClickOfflineRegistration(SCITER_VALUE svOfflineActKey, SCITER_VALUE svFunSetRegStatusCB)
{
	try
	{
		m_svFunSetRegStatusCB = svFunSetRegStatusCB;

		const wstring chOfflineActKey = svOfflineActKey.get(L"");
		memset(&m_szOfflineActivationCode, 0x00, sizeof(m_szOfflineActivationCode));
		memcpy(m_szOfflineActivationCode, chOfflineActKey.c_str(), 0x16 * sizeof(TCHAR));
		SetEvent(m_hOfflineNextEvent);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_ClickOfflineRegistration");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : PerformRegistration
*  Description    : Performs registration for different activation modes
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
void CRegistrationDlg::PerformRegistration(SCITER_VALUE svArrUserDetails)
{
	try
	{
		CString csChkWindowText;
		const SCITER_VALUE EachEntry = svArrUserDetails[0];

		const std::wstring Salutation = EachEntry[L"Salutation"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szTitle, Salutation.c_str());

		const std::wstring FirstName = EachEntry[L"FirstName"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szUserFirstName, FirstName.c_str());

		const std::wstring LastName = EachEntry[L"LastName"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szUserLastName, LastName.c_str());

		const std::wstring PhoneNumber = EachEntry[L"PhoneNumber"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szMobileNo, PhoneNumber.c_str());

		const std::wstring EmailID = EachEntry[L"EmailID"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szEmailID, EmailID.c_str());

		const std::wstring DealerCode = EachEntry[L"DealerCode"].get(L"");
		wcscpy(m_szDealerCode, DealerCode.c_str());

		const std::wstring ReferenceID = EachEntry[L"ReferenceID"].get(L"");
		wcscpy(m_szReferenceID, ReferenceID.c_str());

		const std::wstring Country = EachEntry[L"Country"].get(L"");
		wcscpy(m_szCountry, Country.c_str());

		const std::wstring State = EachEntry[L"State"].get(L"");
		wcscpy(m_szState, State.c_str());

		const std::wstring City = EachEntry[L"City"].get(L"");
		wcscpy(m_szCity, City.c_str());

		const std::wstring PinCode = EachEntry[L"PinCode"].get(L"");
		wcscpy(m_szPinCode, PinCode.c_str());

		const std::wstring EngineerName = EachEntry[L"EngineerName"].get(L"");
		wcscpy(m_szEngineerName, EngineerName.c_str());

		const std::wstring EngineerMobNo = EachEntry[L"EngineerMobNo"].get(L"");
		wcscpy(m_szEngineerMobNo, EngineerMobNo.c_str());

		const std::wstring OSName = EachEntry[L"OSName"].get(L"");
		wcscpy(m_szOSName, OSName.c_str());

		const std::wstring RamSize = EachEntry[L"RamSize"].get(L"");
		wcscpy(m_szRamSize, RamSize.c_str());

		const std::wstring HDSize = EachEntry[L"HDSize"].get(L"");
		wcscpy(m_szHDSize, HDSize.c_str());

		const std::wstring Processor = EachEntry[L"Processor"].get(L"");
		wcscpy(m_szProcessor, Processor.c_str());

		const std::wstring CompName = EachEntry[L"CompName"].get(L"");
		wcscpy(m_szCompName, CompName.c_str());

		const std::wstring UserName = EachEntry[L"UserName"].get(L"");
		wcscpy(m_szUserName, UserName.c_str());

		const std::wstring wcsInstID = EachEntry[L"InstallID"].get(L"");
		wcscpy(m_RegDlg_ActInfo.szInstID, wcsInstID.c_str());
		
		CITinRegWrapper objReg;
		TCHAR szValue[0x80] = { 0 };
		DWORD dwSize = sizeof(szValue);
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"MVersion", szValue, dwSize) != 0)
		{
			AddLogEntry(L"### Error in GetRegistryValueData CRegistrationDlg::OnBnClickedButtonRegistrationNext", 0, 0, true, SECONDLEVEL);
			DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
			return;
		}
		_tcscpy_s(m_RegDlg_ActInfo.szClientID, 0x80, szValue);

		if (m_bActiveProduct == true)
		{
			m_hOfflineNextEvent = NULL;
			m_hOfflineNextEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			const std::wstring ProdID = EachEntry[L"ProductID"].get(L"");
			memcpy(&m_RegDlg_ActInfo.szKey, ProdID.c_str(), sizeof(m_RegDlg_ActInfo.szKey));
		}

		if (m_bTryProduct == true)
		{
			memcpy(&m_RegDlg_ActInfo.szKey, L"", sizeof(m_RegDlg_ActInfo.szKey));
		}

		TCHAR szGlobalIP[64] = { 0 };
		TCHAR	szInstallCode[32] = { 0 };

		if (m_bOfflineActivation == true && m_bActiveProduct == true)
		{
			if (!CheckForMIDInRegistry())
			{
				DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
				return;
			}
			if (!LoadWrdWizOfflineRegDll())
			{
				DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
				AddLogEntry(L"### Failed to locate VBOFFLINEREG.DLL", 0, 0, true, SECONDLEVEL);
				return;
			}
			if (g_GetInstallationCode)
				g_GetInstallationCode(m_RegDlg_ActInfo.szKey, m_szMachineIDValue, szInstallCode);

			if (!szInstallCode[0])
			{
				DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
				AddLogEntry(L"### Generation of installation code is failed", 0, 0, true, SECONDLEVEL);
				return;
			}

			if ((_tcslen(szInstallCode) < 16) && (_tcslen(szInstallCode) > 28))
			{
				DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"));
				AddLogEntry(L"### Generation of invalid installation code", 0, 0, true, SECONDLEVEL);
				return;
			}
			if (theApp.m_dwProductID == 1)
			{
				_tcscpy(m_szInstallCode, L"1");
			}
			else if (theApp.m_dwProductID == 2)
			{
				_tcscpy(m_szInstallCode, L"2");
			}
			else if (theApp.m_dwProductID == 3)
			{
				_tcscpy(m_szInstallCode, L"3");
			}
			else if (theApp.m_dwProductID == 4)
			{
				_tcscpy(m_szInstallCode, L"4");
			}
			else if (theApp.m_dwProductID == 5)
			{
				_tcscpy(m_szInstallCode, L"5");
			}
			_tcscat(m_szInstallCode, szInstallCode);
			m_svFunSetOfflineActKeyCB.call(m_szInstallCode);
			theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_FORTHDLG_HEADER"));

		}
		else if ((m_bOnlineActivation == true && m_bActiveProduct == true) || m_bTryProduct == true)
		{
			wcscpy_s(m_RegDlg_ActInfo.szRegionCode, 64, L"");
			theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_PROCESSING"));
			Invalidate();
		}

		DWORD	dwSendRequstThreadID = 0x00;

		dwDaysRemain = 0x00;
		m_hSendRequestThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendRequestThread, this, 0, &dwSendRequstThreadID);
		Sleep(1000);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::PerformRegistration");
	}
}

/***************************************************************************************************
*  Function Name  : SendRequestThread
*  Description    : thread function to perform registration
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
DWORD WINAPI SendRequestThread(LPVOID lpThreadParam)
{
	CRegistrationDlg	*pRegDlg = (CRegistrationDlg *)lpThreadParam;
	CString		csIniFilePath;
	if (!pRegDlg)
		return 1;

	if (pRegDlg->m_bOnlineActivation || pRegDlg->m_bTryProduct)
	{
		pRegDlg->m_bisThreadCompleted = true;
	}

	DWORD	dwRet = 0x00;

	if (pRegDlg->m_bOnlineActivation == true || pRegDlg->m_bTryProduct)
	{
		if (pRegDlg->ReadHostFileForWWRedirection())
		{
			pRegDlg->DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_STATIC_FAIL"));
			AddLogEntry(L"### Hosts redirection found, please check hosts file present in Drivers folder", 0, 0, true, SECONDLEVEL);
			goto FAILED;
		}
	}

	if (pRegDlg->GetRegisteredUserInfo(theApp.m_dwProductID) || pRegDlg->m_bActiveProduct)
	{
		SYSTEMTIME		CurrTime = { 0 };

		WORD		wDaysLeft = 0x00;

		memset(&pRegDlg->m_ActInfo, 0x00, sizeof(pRegDlg->m_ActInfo));
		memcpy(&pRegDlg->m_ActInfo, &m_RegDlg_ActInfo, sizeof(m_RegDlg_ActInfo));
		GetSystemTime(&CurrTime);
		pRegDlg->m_ActInfo.RegTime = CurrTime;

		DWORD				dwDaysLeft = 0x00;
		AVACTIVATIONINFO	ActInfo = { 0 };
		memcpy(&ActInfo, &pRegDlg->m_ActInfo, sizeof(ActInfo));

		//Write here Product Number
		pRegDlg->m_ActInfo.dwProductNo = theApp.m_dwProductID;
		ActInfo.dwProductNo = theApp.m_dwProductID;

		CString csProductID;
		csProductID.Format(L">>> Product ID: %d", pRegDlg->m_ActInfo.dwProductNo);
		AddLogEntry(csProductID);

		//Send data to server & Get Server response
		if (pRegDlg->m_bOnlineActivation == true || pRegDlg->m_bTryProduct == true)
		{
			//Send data to server & Get Server response
			DWORD dwRes = pRegDlg->GetRegistrationDateFromServer(&CurrTime, &dwDaysLeft);
			if (dwRes > 0)
			{
				if (dwRes == PRODUCTEXPIRED)
				{
					pRegDlg->m_ActInfo.dwTotalDays = dwDaysLeft;
					pRegDlg->AddProdRegInfoToLocal(ADD_REGISTRATION_DATA, ActInfo, sizeof(ActInfo), IDR_REGDATA, TEXT("REGDATA"), false);
				}
				
				pRegDlg->DisplayFailureMessage((REGFAILUREMSGS)dwRes);
				goto FAILED;
			}

			////Ram, Resolved Issue No: 0001075
			pRegDlg->m_ActInfo.RegTimeServer = CurrTime;
			ActInfo.RegTimeServer = CurrTime;
			ActInfo.dwTotalDays = dwDaysLeft;

			//Local time gives your local time rather than UTC.
			GetSystemTime(&CurrTime);
			pRegDlg->m_ActInfo.RegTime = CurrTime;
			ActInfo.RegTime = CurrTime;
			pRegDlg->m_ActInfo.dwTotalDays = dwDaysLeft;
		}

		if (pRegDlg->m_bOfflineActivation == true)
		{
			TCHAR szTempActivationCode[32] = { 0 };
			//code for offline registration.
			WaitForSingleObject(pRegDlg->m_hOfflineNextEvent, INFINITE);

			pRegDlg->m_bisThreadCompleted = true;

			if (g_ValidateResponse)
			{
				//DWORD dwValidResponse = g_ValidateResponse(pRegDlg->m_objRegistrationForthDlg.m_szActivationCode, (BYTE)theApp.m_dwProductID, CurrTime, wDaysLeft);
				DWORD dwValidResponse = g_ValidateResponse(pRegDlg->m_szOfflineActivationCode, (BYTE)theApp.m_dwProductID, CurrTime, wDaysLeft);
				if (dwValidResponse)
				{
					pRegDlg->DisplayFailureMessageForOfflineRegistration((REGFAILUREMSGS)dwValidResponse);
					goto FAILED;
				}
			}
			dwDaysLeft = static_cast<DWORD>(wDaysLeft);

			ActInfo.RegTime = pRegDlg->m_ActInfo.RegTime = CurrTime;
			ActInfo.dwTotalDays = pRegDlg->m_ActInfo.dwTotalDays = dwDaysLeft;
		}

		ResetEvent(pRegDlg->m_hOfflineNextEvent);

		memcpy(&g_ActInfo, &ActInfo, sizeof(ActInfo));

		if (!pRegDlg->SendRegisteredData2Service(ADD_REGISTRATION_DATA, (LPBYTE)&ActInfo, sizeof(ActInfo), IDR_REGDATA, TEXT("REGDATA"), false))
		{
			AddLogEntry(L"### Failed to SendRegisteredData2Service in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}

		Sleep(10);
		
		if (pRegDlg->AddRegistrationDataInRegistry())
		{
			AddLogEntry(L"### AddRegistrationDataInRegistry in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}

		Sleep(10);

		if (pRegDlg->AddRegistrationDataInFile())
		{
			AddLogEntry(L"### AddRegistrationDataInFile failed in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		}
	}
	else
	{

		memcpy(&g_ActInfo, &pRegDlg->m_ActInfo, sizeof(g_ActInfo));
		SYSTEMTIME		CurrTime = { 0 };
		GetSystemTime(&CurrTime);
		CTime	Time_Reg(pRegDlg->m_ActInfo.RegTime);
		CTime	Time_Curr(CurrTime);

		DWORD	dwDays = 0x00;

		if (Time_Curr > Time_Reg)
		{
			CTimeSpan	Time_Diff = Time_Curr - Time_Reg;
			if (Time_Diff.GetDays() < pRegDlg->m_ActInfo.dwTotalDays)
				dwDays = pRegDlg->m_ActInfo.dwTotalDays - static_cast<DWORD>(Time_Diff.GetDays());
		}

		if (!dwDays)
		{
			CString csMessage;
			theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_STATUS"));
			csMessage.Format(L"\t%s \n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL1"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL2"));
			pRegDlg->DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, csMessage);
			goto FAILED;
		}
	}

	pRegDlg->SpreadRegistrationFilesInSystem();

	if (pRegDlg->m_ActInfo.dwTotalDays > 30)
		pRegDlg->SendRegInfo2Server();

	//No need to do check for Non Clam setup
	if (theApp.m_dwIsOffline)
	{
		//After registration offline or online this flag should be zero. Commsrv decide whether WadWiz copy is genuine or not.
		if (!pRegDlg->SendRegistryData2Service(SZ_DWORD, theApp.m_csProdRegKey.GetBuffer(), L"dwNGC", (LPBYTE)L"", 0x00, false))
		{
			AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::SendRequestThread", 0, 0, true, SECONDLEVEL);
		}
	}

	if (pRegDlg->m_bOfflineActivation)
	{
		//0x01 value for offline registration.
		if (!pRegDlg->SendRegistryData2Service(SZ_DWORD, theApp.m_csProdRegKey.GetBuffer(), L"dwRegUserType", (LPBYTE)L"", 0x01, false))
		{
			AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::SendRequestThread", 0, 0, true, SECONDLEVEL);
			goto FAILED;
		}
	}
	else if (pRegDlg->m_bOnlineActivation)
	{
		//0x00nvalue for online registration.
		if (!pRegDlg->SendRegistryData2Service(SZ_DWORD, theApp.m_csProdRegKey.GetBuffer(), L"dwRegUserType", (LPBYTE)L"", 0x00, false))
		{
			AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::SendRequestThread", 0, 0, true, SECONDLEVEL);
			goto FAILED;
		}
	}

	//check if not offline mode
	if (!pRegDlg->m_bOfflineActivation && !pRegDlg->bCheckCodeValue)
	{
		if (_tcslen(pRegDlg->m_szDealerCode) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"DealerCode", pRegDlg->m_szDealerCode, csIniFilePath);
		}

		if (_tcslen(pRegDlg->m_szReferenceID) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"ReferenceID", pRegDlg->m_szReferenceID, csIniFilePath);
		}

		if (_tcslen(pRegDlg->m_szEngineerName) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"EngineerName", pRegDlg->m_szEngineerName, csIniFilePath);
		}

		if (_tcslen(pRegDlg->m_szEngineerMobNo) != 0)
		{
			csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
			WritePrivateProfileString(L"VBSETTINGS", L"EngineerMobNo", pRegDlg->m_szEngineerMobNo, csIniFilePath);
		}
	}

	if (_tcslen(pRegDlg->m_szCountry) != 0)
	{
		csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"Country", pRegDlg->m_szCountry, csIniFilePath);
	}

	if (_tcslen(pRegDlg->m_szState) != 0)
	{
		csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"State", pRegDlg->m_szState, csIniFilePath);
	}

	if (_tcslen(pRegDlg->m_szCity) != 0)
	{
		csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"City", pRegDlg->m_szCity, csIniFilePath);
	}

	if (_tcslen(pRegDlg->m_szPinCode) != 0)
	{
		csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		WritePrivateProfileString(L"VBSETTINGS", L"PinCode", pRegDlg->m_szPinCode, csIniFilePath);
	}

	Sleep(1000);

	if (!pRegDlg->SendRegistrationInfo2Service(RELOAD_REGISTARTION_DAYS))
	{
		AddLogEntry(L"### Error in CRegistrationSecondDlg SendRequestThread::SendRegistrationInfo2Service", 0, 0, true, SECONDLEVEL);
	}
	
	if (pRegDlg->GetProxySettingsFromRegistry() || pRegDlg->m_bOfflineActivation)
	{
		pRegDlg->DisplayFailureMsgOnUI(ERROR_SUCCESS_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_STATIC_SUCCESS"));
	}
	
	return dwRet;
FAILED:
	return dwRet;
}


/**********************************************************************************************************
* Function Name      : CheckForMIDInRegistry
* Description        : Check Machine ID is present in registry or not
* Author Name		 : Neha Gharge
* Date Of Creation   : 10th Oct 2014
* SR_No              :
***********************************************************************************************************/
bool CRegistrationDlg::CheckForMIDInRegistry()
{
	try
	{
		CITinRegWrapper objReg;
		TCHAR szMachineIDValue[0x80] = { 0 };
		DWORD dwSize = sizeof(szMachineIDValue);

		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"MVersion", szMachineIDValue, dwSize) != 0)
		{
			AddLogEntry(L"### Error in GetRegistryValueData CRegistrationSecondDlg::CheckForMIDInRegistry", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!szMachineIDValue[0])
		{
			return false;
		}

		_tcscpy(m_szMachineIDValue, L"");
		_tcscpy(m_szMachineIDValue, szMachineIDValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::CheckForMIDInRegistry");
	}
	return true;
}

/**********************************************************************************************************
* Function Name      : LoadWrdWizOfflineRegDll()
* Description        : Loading WrdWizOfflineReg Dll from installation folder . and getprocaddr of some function.
* Author Name		 : Neha Gharge
* Date Of Creation   : 10th Oct 2014
* SR_No              :
***********************************************************************************************************/
bool CRegistrationDlg::LoadWrdWizOfflineRegDll()
{
	try
	{
		CString	csAppPath = GetModuleFilePath();
		CString csDllPath = L"";
		csDllPath.Format(L"%sVBOFFLINEREG.DLL", csAppPath);
		if (!PathFileExists(csDllPath))
		{
			return false;
		}

		if (!m_hOfflineDLL)
		{
			m_hOfflineDLL = LoadLibrary(csDllPath);
			if (!m_hOfflineDLL)
			{
				AddLogEntry(L"Locating error in VBOFFLINEREG.DLL", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		g_GetInstallationCode = (GETINSTALLATIONCODE)GetProcAddress(m_hOfflineDLL, "GetInstallationCode");
		if (!g_GetInstallationCode)
			return false;

		g_ValidateResponse = (VALIDATERESPONSE)GetProcAddress(m_hOfflineDLL, "ValidateResponse");
		if (!g_ValidateResponse)
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::LoadGenXOfflineRegDll");
	}
	return true;
}

CString CRegistrationDlg::GetModuleFilePath()
{
	try
	{
		HKEY	hSubKey = NULL;
		TCHAR	szModulePath[MAX_PATH] = { 0 };

		if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey, &hSubKey) != ERROR_SUCCESS)
			return L"";

		DWORD	dwSize = 511;
		DWORD	dwType = 0x00;

		RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize);
		RegCloseKey(hSubKey);
		hSubKey = NULL;

		if (_tcslen(szModulePath) > 0)
		{
			return CString(szModulePath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetModuleFilePath");
	}
	return L"";
}



/***************************************************************************************************
*  Function Name  : ReadHostFileForWWRedirection()
*  Description    : Checks hosts file for local redirection. if found, returns true else false.
*  Author Name    : Vilas
*  Date			  :	29-Sept-2014
*  Modified Date  :	30-Sept-2014
****************************************************************************************************/
bool CRegistrationDlg::ReadHostFileForWWRedirection()
{
	bool bRedirection = false;
	try
	{
		FILE *pFileStream = NULL;

		long lFileSize = 0x00, lReadSize = 0x00;

		TCHAR szHostPath[512] = { 0 };
		TCHAR szFileLine[1024] = { 0 };

		try
		{
			GetSystemDirectory(szHostPath, 511);
			wcscat_s(szHostPath, L"\\Drivers\\etc\\hosts");

			pFileStream = _wfsopen(szHostPath, _T("r"), _SH_DENYNO);
			if (!pFileStream)
				goto Cleanup;

			fseek(pFileStream, 0L, SEEK_END);
			lFileSize = ftell(pFileStream);

			if (!lFileSize)
				goto Cleanup;

			fseek(pFileStream, 0L, SEEK_SET);

			while (true)
			{
				if (fgetws(szFileLine, 1023, pFileStream) == NULL)
					break;

				if (!wcslen(szFileLine))
					break;

				if ((StrStrI(szFileLine, L"wardwiz.")) || (StrStrI(szFileLine, L"162.144.82.101")))
				{
					bRedirection = true;
					break;
				}

				if (feof(pFileStream))
					break;

				lReadSize += static_cast<long>(wcslen(szFileLine));
				if (lReadSize >= lFileSize)
					break;

				ZeroMemory(szFileLine, sizeof(szFileLine));
			}
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in ReadHostFileForWardwizRedirection", 0, 0, true, SECONDLEVEL);
		}

	Cleanup:

		if (pFileStream)
			fclose(pFileStream);

		pFileStream = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::ReadHostFileForWardwizRedirection");
	}
	return bRedirection;
}

/***************************************************************************************************
*  Function Name  : GetRegisteredUserInfo
*  Description    : Get registered user information
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
DWORD CRegistrationDlg::GetRegisteredUserInfo(DWORD dwProdID)
{

	DWORD	dwRet = 0x00;
	DWORD	dwSize = 0x00;
	DWORD	dwRegUserSize = 0x00;
	try
	{
		dwDaysRemain = 0x00;

		m_RegisterationDLL = NULL;

		CString	strEvalRegDLL("");
		CString	strRegisterDLL("");

		CString	strAppPath = GetModuleFilePath();
		dwRet = GetRegistrationDataFromRegistry();
		if (!dwRet)
		{
			if (!CheckForMachineID(m_ActInfo))
			{
				memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
				dwRet = 0x06;
			}
			goto Cleanup;
		}

		if (dwRet)
		{
			dwRet = GetRegistrationDatafromFile();
		}

		if (!dwRet)
		{
			if (!CheckForMachineID(m_ActInfo))
			{
				memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
				dwRet = 0x06;
			}
			goto Cleanup;
		}

		strEvalRegDLL.Format(TEXT("%sVBEVALREG.DLL"), strAppPath);
		if (!PathFileExists(strEvalRegDLL))
		{
			dwRet = 0x01;
			AddLogEntry(L"### VBEVALREG.DLL not found in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		strRegisterDLL.Format(TEXT("%sVBREGISTERDATA.DLL"), strAppPath);
		if (!PathFileExists(strRegisterDLL))
		{
			dwRet = 0x02;
			AddLogEntry(L"### VBREGISTERDATA.DLL not found in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		m_RegisterationDLL = LoadLibrary(strRegisterDLL);
		GetRegistrationData = (GETREGISTRATIONDATA)GetProcAddress(m_RegisterationDLL, "GetRegisteredData");

		if (!GetRegistrationData)
		{
			dwRet = 0x04;
			AddLogEntry(L"### VBREGISTERDATA.DLL version is incorrect in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		dwSize = sizeof(m_ActInfo);
		dwRegUserSize = 0x00;

		if (GetRegistrationData((LPBYTE)&m_ActInfo, dwRegUserSize, IDR_REGDATA, L"REGDATA") == 0)
		{
			if (!CheckForMachineID(m_ActInfo))
			{
				memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
				dwRet = 0x06;
				goto Cleanup;
			}
			dwRet = 0x00;
			goto Cleanup;
		}

		if (dwSize != dwRegUserSize)
			dwRet = 0x05;

		//Match here the machine ID with Stored machineID
		if (theApp.m_dwProductID != m_ActInfo.dwProductNo)
		{
			AddLogEntry(L"### Product ID not matched returning", 0, 0, true, ZEROLEVEL);
			dwRet = 0x05;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			AddLogEntry(L"### Machine ID not matched returning", 0, 0, true, SECONDLEVEL);
			dwRet = 0x06;
			goto Cleanup;
		}

	Cleanup:
		
		if (!dwRet)
		{
			memcpy(&g_ActInfo, &m_ActInfo, sizeof(m_ActInfo));
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetRegisteredUserInfo");
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : SendRegisteredData2Service
*  Description    : Send Data to servive
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
bool CRegistrationDlg::SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName, bool bRegWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = dwType;
		memcpy(szPipeData.byData, lpResBuffer, dwResSize);
		szPipeData.dwValue = dwResSize;
		szPipeData.dwSecondValue = dwResType;
		wcscpy_s(szPipeData.szFirstParam, pResName);

		CISpyCommunicator objCom(SERVICE_SERVER, true, 3);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bRegWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}

			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SendRegisteredData2Service");
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : AddRegistrationDataInFile
*  Description    : Adds registration data in File
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
DWORD CRegistrationDlg::AddRegistrationDataInFile()
{
	DWORD	dwRet = 0x00;
	try
	{
		if (!SendRegisteredData2Service(ADD_REGISTRATION_DATAINFILE, (LPBYTE)&m_ActInfo, sizeof(m_ActInfo), 0, L"", false))
		{
			AddLogEntry(L"### Failed to SendRegisteredData2Service in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
		}

		Sleep(30);

		SpreadRegistrationFilesInSystem();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::AddRegistrationDataInFile");
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : SpreadRegistrationFilesInSystem
*  Description    : Drops Registered user  .db file in system
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
DWORD CRegistrationDlg::SpreadRegistrationFilesInSystem()
{
	try
	{
		TCHAR	szAllUserPath[512] = { 0 };

		TCHAR	szSource[512] = { 0 };
		TCHAR	szSource1[512] = { 0 };
		TCHAR	szDestin[512] = { 0 };
		TCHAR	szDestin1[512] = { 0 };

		OSVERSIONINFO 	OSVer = { 0 };

		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 511);

		OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		GetVersionEx(&OSVer);

		if (OSVer.dwMajorVersion > 5)
		{
			wsprintf(szDestin, L"%s\\Wardwiz Antivirus", szAllUserPath);
		}
		else
		{
			wsprintf(szDestin, L"%s\\Application Data\\Wardwiz Antivirus", szAllUserPath);
		}

		TCHAR szModulePath[MAX_PATH] = { 0 };
		_tcscpy(szModulePath, GetWardWizPathFromRegistry());
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		wcscpy(szDestin1, szDestin);

		wsprintf(szSource1, L"%s\\VBUSERREG.DB", szModulePath);
		wcscat(szDestin1, L"\\VBUSERREG.DB");

		CopyFileToDestination(szSource1, szDestin1);

		memset(szDestin1, 0x00, 512 * sizeof(TCHAR));
		wsprintf(szDestin1, L"%c:\\VBUSERREG.DB", szAllUserPath[0]);

		CopyFileToDestination(szSource1, szDestin1);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SpreadRegistrationFilesInSystem");
	}
	return 0;
}

bool CRegistrationDlg::CopyFileToDestination(TCHAR *pszSource, TCHAR *pszDest)
{
	try
	{
		// Delete Old File Copy new File and Set attribute to hidden //Implemented inRegistration process to distribute UserRegDB files in system so 5 is used
		if (!CopyRegistrationFilesUsingService(FILE_OPERATIONS, pszSource, pszDest, 5, false))
		{
			AddLogEntry(L"### CopyRegistrationFilesUsingService Failed in CRegistrationSecondDlg::CopyFileToDestination", 0, 0, true, SECONDLEVEL);
			return false;
		}
		Sleep(30);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::CopyFileToDestination", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

bool CRegistrationDlg::CopyRegistrationFilesUsingService(DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = dwType;
		szPipeData.dwValue = dwValue;
		wcscpy_s(szPipeData.szFirstParam, csSrcFilePath);
		wcscpy_s(szPipeData.szSecondParam, csDestFilePath);

		CISpyCommunicator objCom(SERVICE_SERVER, true, 3);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : CopyRegistrationFilesUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : CopyRegistrationFilesUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::CopyRegistrationFilesUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CRegistrationDlg::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPBYTE byData, DWORD dwLen, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = dwType;
		wcscpy_s(szPipeData.szFirstParam, szKey);
		wcscpy_s(szPipeData.szSecondParam, szValue);
		memcpy(szPipeData.byData, byData, dwLen);
		szPipeData.dwSecondValue = dwLen;

		CISpyCommunicator objCom(SERVICE_SERVER, true, 3);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SendRegistryData2Service");
	}
	return true;
}

bool CRegistrationDlg::SendRegistrationInfo2Service(int iMessageInfo, DWORD dwType, bool bEmailPluginWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = iMessageInfo;

		CISpyCommunicator objComUI(UI_SERVER, true, 2);
		if (!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::UI_SERVER");
		}

		CISpyCommunicator objComEmail(EMAILPLUGIN_SERVER);
		if (!objComEmail.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::EMAILPLUGIN_SERVER");
		}

		CISpyCommunicator objComTray(TRAY_SERVER, true, 2);
		if (!objComTray.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::TRAY_SERVER");
		}

		CISpyCommunicator objComUSB(USB_SERVER, true, 2);
		if (!objComUSB.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::USB_SERVER");
		}

		CISpyCommunicator objComService(SERVICE_SERVER, true, 2);
		if (!objComService.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::USB_SERVER");
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SendRegistrationInfo2Service");
	}
	return true;
}

DWORD CRegistrationDlg::GetRegistrationDatafromFile()
{

	DWORD	dwRet = 0x01;
	try
	{
		CString	strUserRegFile = GetModuleFilePath();
		strUserRegFile = strUserRegFile + L"VBUSERREG.DB";
		dwRet = GetRegistrationDatafromFile(strUserRegFile);
		if (!dwRet)
			return dwRet;

		TCHAR	szAllUserPath[512] = { 0 };

		TCHAR	szSource[512] = { 0 };
		TCHAR	szSource1[512] = { 0 };
		TCHAR	szDestin[512] = { 0 };
		TCHAR	szDestin1[512] = { 0 };

		OSVERSIONINFO 	OSVer = { 0 };

		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 511);
		OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&OSVer);
		if (OSVer.dwMajorVersion > 5)
			wsprintf(szDestin, L"%s\\Wardwiz Antivirus", szAllUserPath);
		else
			wsprintf(szDestin, L"%s\\Application Data\\Wardwiz Antivirus", szAllUserPath);


		wcscpy(szDestin1, szDestin);
		wcscat(szDestin1, L"\\VBUSERREG.DB");
		dwRet = 0x01;
		dwRet = GetRegistrationDatafromFile(szDestin1);
		if (!dwRet)
			return dwRet;

		TCHAR	szDrives[256] = { 0 };
		GetLogicalDriveStrings(255, szDrives);

		TCHAR	*pDrive = szDrives;

		while (wcslen(pDrive) > 2)
		{
			dwRet = 0x01;
			memset(szDestin1, 0x00, 512 * sizeof(TCHAR));
			wsprintf(szDestin1, L"%VBUSERREG.DB", pDrive);

			if ((GetDriveType(pDrive) & DRIVE_FIXED) == DRIVE_FIXED)
			{
				dwRet = GetRegistrationDatafromFile(szDestin1);
				if (!dwRet)
					return dwRet;
			}
			pDrive += 4;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetRegistrationDatafromFile");
	}
	return 0x01;
}

DWORD CRegistrationDlg::GetRegistrationDatafromFile(CString strUserRegFile)
{
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	DWORD	dwRet = 0x00, dwBytesRead = 0x00;

	DWORD	dwSize = sizeof(m_ActInfo);

	hFile = CreateFile(strUserRegFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwRet = 0x01;
		goto Cleanup;
	}

	ZeroMemory(&m_ActInfo, sizeof(m_ActInfo));
	ReadFile(hFile, &m_ActInfo, dwSize, &dwBytesRead, NULL);
	if (dwSize != dwBytesRead)
	{
		dwRet = 0x02;
		goto Cleanup;
	}

	if (DecryptData((LPBYTE)&m_ActInfo, dwSize))
	{
		dwRet = 0x04;
		goto Cleanup;
	}

	//Match here the machine ID with Stored machineID
	if (theApp.m_dwProductID != m_ActInfo.dwProductNo)
	{
		AddLogEntry(L"### Product ID not matched returning", 0, 0, true, ZEROLEVEL);
		dwRet = 0x05;
		goto Cleanup;
	}

	//Match here the machine ID with Stored machineID
	//if someone provides the DB files from other computer then it works so 
	//necessary to check for machine ID.
	if (!CheckForMachineID(m_ActInfo))
	{
		memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
		AddLogEntry(L"### Machine ID not matched returning", 0, 0, true, SECONDLEVEL);
		dwRet = 0x06;
		goto Cleanup;
	}
Cleanup:

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : CheckForMachineID
*  Description    : function which compare with Machine ID present in DB files & machine ID in registry.
*  Author Name    : Ram
*  SR_NO		  :
*  Date           : 18 Sep 2013
****************************************************************************************************/
bool CRegistrationDlg::CheckForMachineID(const AVACTIVATIONINFO	&actInfo)
{
	try
	{
		CITinRegWrapper objReg;
		TCHAR szValue[0x80] = { 0 };
		DWORD dwSize = sizeof(szValue);
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"MVersion", szValue, dwSize) != 0)
		{
			AddLogEntry(L"### Error in GetRegistryValueData CRegistrationSecondDlg::CheckForMachineID", 0, 0, true, SECONDLEVEL);
			return false;
		}

		//Need to compare the string fron 2nd character, as we are storing first character with string length.
		//also need to compare with machine ID added with more charaters in the registry, example like
		//we firstly installed 1.8 setup in that MAC address not present, then we added MAC Address in further
		//releases then Machine ID string will get increased in registry but not in DB files.
		//In this case we need to compare with DB file machine ID with new generated Machine ID with same legnth 
		//of Machine id which is present in DB files.
		if (memcmp(&actInfo.szClientID[1], &szValue[1], wcslen(actInfo.szClientID) * sizeof(TCHAR)) != 0)
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::CheckForMachineID");
	}
	return true;
}

DWORD CRegistrationDlg::GetRegistrationDateFromServer(LPSYSTEMTIME lpServerTime, LPDWORD lpDaysRemaining)
{
	DWORD	dwRet = 0x01;
	try
	{	
		DWORD dwProxy_Sett = GetProxySettingsFromRegistry();
		if (dwProxy_Sett == 0x02)
		{
			TCHAR	szUserInfo[512] = { 0 };
			TCHAR	szKey[0x32] = { 0 };
			TCHAR	szDomainName[MAX_PATH] = { 0 };
			if (!GetDomainName(szDomainName, MAX_PATH))
			{
				AddLogEntry(L"### Failed to get GetDomainName in CRegistrationSecondDlg::GetRegistrationDateFromServer", 0, 0, true, SECONDLEVEL);
				return dwRet;
			}

			if (!m_bActiveProduct)
			{
				wsprintf(szUserInfo, L"http://%s/new_reg/get/trial.php?FName=%s&LName=%s&Email=%s&ContactNo=%s&ProdID=%d&MachineID=%s&OSName=%s&RamSize=%s&HDSize=%s&Processor=%s&CompName=%s&UserName=%s", szDomainName, m_ActInfo.szUserFirstName,
					m_ActInfo.szUserLastName, m_ActInfo.szEmailID, m_ActInfo.szMobileNo, m_ActInfo.dwProductNo, m_ActInfo.szClientID, m_szOSName, m_szRamSize, m_szHDSize, m_szProcessor, m_szCompName, m_szUserName);
			}
			else
			{
				memcpy(&szKey, m_ActInfo.szKey, sizeof(m_ActInfo.szKey));
				szKey[wcslen(szKey) + 1] = '\0';

				//If m_dwIsOffline = 0 , then url is changed for german setup
				if (theApp.m_dwIsOffline == 1)
				{
					//If registration key already, means product came for renewal.
					if (_tcslen(g_ActInfo.szKey) > 0)
					{
						wsprintf(szUserInfo, L"http://%s/new_reg/get/registration.php?FName=%s&LName=%s&Email=%s&RegCode=%s&ContactNo=%s&DealerCodeID=%s&ReferenceID=%s&ProdID=%d&MachineID=%s&Country=%s&State=%s&City=%s&PinCode=%s&EngineerName=%s&EngineerMobNo=%s&OSName=%s&RamSize=%s&HDSize=%s&Processor=%s&CompName=%s&UserName=%s&P10=1", szDomainName, m_ActInfo.szUserFirstName,
							m_ActInfo.szUserLastName, m_ActInfo.szEmailID, szKey, m_ActInfo.szMobileNo, m_szDealerCode, m_szReferenceID, m_ActInfo.dwProductNo, m_ActInfo.szClientID, m_szCountry, m_szState, m_szCity, m_szPinCode, m_szEngineerName, m_szEngineerMobNo, m_szOSName, m_szRamSize, m_szHDSize, m_szProcessor, m_szCompName, m_szUserName);
					}
					else
					{
						wsprintf(szUserInfo, L"http://%s/new_reg/get/registration.php?FName=%s&LName=%s&Email=%s&RegCode=%s&ContactNo=%s&DealerCodeID=%s&ReferenceID=%s&ProdID=%d&MachineID=%s&Country=%s&State=%s&City=%s&PinCode=%s&EngineerName=%s&EngineerMobNo=%s&OSName=%s&RamSize=%s&HDSize=%s&Processor=%s&CompName=%s&UserName=%s", szDomainName, m_ActInfo.szUserFirstName,
							m_ActInfo.szUserLastName, m_ActInfo.szEmailID, szKey, m_ActInfo.szMobileNo, m_szDealerCode, m_szReferenceID, m_ActInfo.dwProductNo, m_ActInfo.szClientID, m_szCountry, m_szState, m_szCity, m_szPinCode, m_szEngineerName, m_szEngineerMobNo, m_szOSName, m_szRamSize, m_szHDSize, m_szProcessor, m_szCompName, m_szUserName);
					}
				}
				else
				{
					if (_tcslen(g_ActInfo.szKey) > 0)
					{
						wsprintf(szUserInfo, L"http://%s/new_reg/get/de/registration.php?FName=%s&LName=%s&Email=%s&RegCode=%s&ContactNo=%s&DealerCodeID=%s&ReferenceID=%s&ProdID=%d&MachineID=%s&Country=%s&State=%s&City=%s&PinCode=%s&EngineerName=%s&EngineerMobNo=%s&OSName=%s&RamSize=%s&HDSize=%s&Processor=%s&CompName=%s&UserName=%s&Lang=%d&P10=1", szDomainName, m_ActInfo.szUserFirstName,
							m_ActInfo.szUserLastName, m_ActInfo.szEmailID, szKey, m_ActInfo.szMobileNo, m_szDealerCode, m_szReferenceID, m_ActInfo.dwProductNo, m_ActInfo.szClientID, m_szCountry, m_szState, m_szCity, m_szPinCode, m_szEngineerName, m_szEngineerMobNo, m_szOSName, m_szRamSize, m_szHDSize, m_szProcessor, m_szCompName, m_szUserName, theApp.m_dwLangID);
					}
					else
					{
						wsprintf(szUserInfo, L"http://%s/new_reg/get/de/registration.php?FName=%s&LName=%s&Email=%s&RegCode=%s&ContactNo=%s&DealerCodeID=%s&ReferenceID=%s&ProdID=%d&MachineID=%s&Country=%s&State=%s&City=%s&PinCode=%s&EngineerName=%s&EngineerMobNo=%s&OSName=%s&RamSize=%s&HDSize=%s&Processor=%s&CompName=%s&UserName=%s&Lang=%d", szDomainName, m_ActInfo.szUserFirstName,
							m_ActInfo.szUserLastName, m_ActInfo.szEmailID, szKey, m_ActInfo.szMobileNo, m_szDealerCode, m_szReferenceID, m_ActInfo.dwProductNo, m_ActInfo.szClientID, m_szCountry, m_szState, m_szCity, m_szPinCode, m_szEngineerName, m_szEngineerMobNo, m_szOSName, m_szRamSize, m_szHDSize, m_szProcessor, m_szCompName, m_szUserName, theApp.m_dwLangID);
					}
				}
			}

			AddLogEntry(L">>>> Sending HTTP request", 0, 0, true, FIRSTLEVEL);
			AddLogEntry(szUserInfo);

			WinHttpClient client(szUserInfo);
			
			if (GetProxyServerDetailsFromDB())
			{
				client.SetProxy(m_csServer.GetBuffer());
				client.SetProxyUsername(m_csUserName.GetBuffer());
				client.SetProxyPassword(m_csPassword.GetBuffer());
			}

			//while registration after entering all the required details
			//click Next->popup appears if internet connection is not there as 'no internet connection
			//click 'OK' on that dialog box->there it should show Retry button
			//so that we not need enter all the details again
			//Neha Gharge 10th Aug ,2015
			if (!client.SendHttpRequest())
			{
				while (true)
				{
					SCITER_VALUE sv_GetRetryCancelVal;
					m_svFunSetNoInternetMsg.call();
					::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
					theApp.m_objCompleteEvent.ResetEvent();
					if (theApp.m_bRetval == true)
					{
						sv_GetRetryCancelVal = 0;
					}
					else
					{
						sv_GetRetryCancelVal = 1;
					}
					if (sv_GetRetryCancelVal == 0)
					{
						if (client.SendHttpRequest())
						{
							break;
						}
					}
					else
					{
						DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_THIRDPG_STATIC_FAIL"));
						return dwRet;
					}
				}
			}
			
			// The response content.
			wstring httpResponseContent = client.GetResponseContent();
			
			AddLogEntry(L">>>> Getting response", 0, 0, true, FIRSTLEVEL);
			AddLogEntry(httpResponseContent.c_str());

			if (m_csResponseData.GetLength() > 0)
				m_csResponseData.Empty();

			m_csResponseData = httpResponseContent.c_str();
		}

		if ((m_csResponseData.GetLength() > 22) && (m_csResponseData.GetLength() < 512))
		{
			dwRet = ExtractDate(m_csResponseData.GetBuffer(), lpServerTime, lpDaysRemaining);
		}
		else
			dwRet = NULLRESPONSE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetRegistrationDateFromServer");
	}
	return dwRet;
}

/***********************************************************************************************
Function Name  : GetDomainName
Description    : Function to get wardwiz domain name.
Author Name    : Ramkrushna Shelke
SR.NO			 :
Date           : 10 Oct 2014
***********************************************************************************************/
bool CRegistrationDlg::GetDomainName(LPTSTR pszDomainName, DWORD dwSize)
{
	bool bReturn = false;
	try
	{
		if (_tcscmp(m_RegDlg_ActInfo.szRegionCode, L"114.143.233.154") == 0)
		{
			_tcscpy_s(pszDomainName, dwSize, L"www.vibranium.co.in");
			bReturn = true;
		}
		else if (_tcscmp(m_RegDlg_ActInfo.szRegionCode, L"217.91.178.129") == 0)//this change is for german country.
		{
			_tcscpy_s(pszDomainName, dwSize, L"www.vibranium.co.in");
			bReturn = true;
		}
		else
		{
			_tcscpy_s(pszDomainName, dwSize, L"www.vibranium.co.in");
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetDomainName");
	}
	return bReturn;
}

DWORD CRegistrationDlg::ExtractDate(TCHAR *pTime, LPSYSTEMTIME lpServerTime, LPDWORD lpdwServerDays)
{
	__try
	{
		SYSTEMTIME	ServerTime = { 0 };
		TCHAR	szTemp[8] = { 0 };
		TCHAR	*chSep = L"#";
		TCHAR	pszTime[64] = { 0 };

		//check here desired lenth else return with failed.
		if (wcslen(pTime) > 0x1E)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x01;
		}

		TCHAR	*pDateTime = wcstok(pTime, chSep);
		if (!pDateTime)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x01;
		}

		TCHAR	*pDays = wcstok(NULL, chSep);
		TCHAR	*pResponseCode = wcstok(NULL, chSep);

		if (!pDays)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x02;
		}

		if (!pResponseCode && wcslen(pResponseCode) != 3)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x03;
		}

		DWORD	dwDays = 0x00;
		DWORD	dwResponseCode = 0x00;

		if (wcslen(pDateTime) > 0x3F)
		{
			AddLogEntry(L"### Invalid Response code from server Time ", 0, 0, true, SECONDLEVEL);
			return 0x14;
		}

		wcscpy(pszTime, pDateTime);

		//Checking for if no of days is -ve.
		if (pDays[0] == '-')
		{
			AddLogEntry(L"### No of days is in -ve returning from CRegistrationSecondDlg::ExtractDate ", 0, 0, true, SECONDLEVEL);
			return 0x02;
		}

		swscanf(pResponseCode, L"%d", &dwResponseCode);

		if (dwResponseCode == 0x01)
		{
			AddLogEntry(L"### Registration key already been used", 0, 0, true, SECONDLEVEL);
			return MACHINEIDMISMATCH;
		}

		if (dwResponseCode == 0x02)
		{
			AddLogEntry(L"### Email ID for Registration is invalid", 0, 0, true, SECONDLEVEL);
			return INVALIDEMAILID;
		}

		if (dwResponseCode == 0x03)
		{
			AddLogEntry(L"### Country code invalid", 0, 0, true, SECONDLEVEL);
			return COUNTRYCODEINVALID;
		}

		if (dwResponseCode == 0x04)
		{
			AddLogEntry(L"### Invalid Registration Number", 0, 0, true, SECONDLEVEL);
			return INVALIDREGNUMBER;
		}

		if (dwResponseCode == 0x05)
		{
			AddLogEntry(L"### Invalid Vibranium Product key", 0, 0, true, SECONDLEVEL);
			return INVALIDPRODVERSION;
		}

		//006 - Invalid Agent, Means the request is not came from Wardwiz client.
		//007 - Database connectivity fails.
		if (dwResponseCode == 0x08)
		{
			AddLogEntry(L"### Failed to update the user information on server, need to resend", 0, 0, true, SECONDLEVEL);
			return USERINFOUPDATEFAILD;
		}

		swscanf(pDays, L"%d", &dwDays);
		if (!dwDays)
		{
			AddLogEntry(L"### Product Expired, Number of days left %s", pDays, 0, true, SECONDLEVEL);
			return PRODUCTEXPIRED;
		}
		else
		{
			*lpdwServerDays = dwDays;
		}

		memcpy(szTemp, pszTime, 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wDay);
		if (!ServerTime.wDay)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x12;
		}

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[3], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wMonth);
		if (!ServerTime.wMonth)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x13;
		}

		DWORD	dwYear = 0x00;

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[6], 4 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &dwYear);
		if (!dwYear)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x14;
		}

		ServerTime.wYear = (WORD)dwYear;

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[11], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wHour);


		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[14], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wMinute);


		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[17], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wSecond);

		*lpServerTime = ServerTime;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::ExtractDate");
	}
	return 0;
}

DWORD CRegistrationDlg::AddRegistrationDataInRegistry()
{
	DWORD	dwRet = 0x00;
	try
	{
		
		DWORD	dwRegType = 0x00, dwRetSize = 0x00;

		HKEY	h_iSpyAV = NULL;
		HKEY	h_iSpyAV_User = NULL;

		AVACTIVATIONINFO	ActInfo = { 0 };

		memcpy(&ActInfo, &m_ActInfo, sizeof(ActInfo));

		if (DecryptData((LPBYTE)&ActInfo, sizeof(ActInfo)))
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!SendRegistryData2Service(SZ_BINARY, L"SOFTWARE\\Microsoft\\Windows", L"VibraniumUserInfo", (LPBYTE)&ActInfo, sizeof(ActInfo), false))
		{
			dwRet = 0x05;
			AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::AddRegistrationDataInRegistry", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::AddRegistrationDataInRegistry");
	}
Cleanup:
	return dwRet;
}


DWORD CRegistrationDlg::GetRegistrationDataFromRegistry()
{
	DWORD	dwRet = 0x00;
	try
	{

		DWORD	dwRegType = 0x00, dwRetSize = 0x00;

		HKEY	h_iSpyAV = NULL;
		HKEY	h_iSpyAV_User = NULL;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h_iSpyAV) != ERROR_SUCCESS)
		{
			dwRet = 0x01;
			goto Cleanup;
		}
		dwRetSize = sizeof(m_ActInfo);
		if (RegQueryValueEx(h_iSpyAV, TEXT("VibraniumUserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo,
			&dwRetSize) != ERROR_SUCCESS)
		{
			dwRet = GetLastError();
			dwRet = 0x03;
			goto Cleanup;
		}

		if (DecryptData((LPBYTE)&m_ActInfo, sizeof(m_ActInfo)))
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		//Match here the machine ID with Stored machineID
		if (theApp.m_dwProductID != m_ActInfo.dwProductNo)
		{
			AddLogEntry(L"### Product ID not matched returning", 0, 0, true, ZEROLEVEL);
			dwRet = 0x05;
			return dwRet;
		}

		//Match here the machine ID with Stored machineID
		//if someone provides the DB files from other computer then it works so 
		//necessary to check for machine ID.
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			AddLogEntry(L"### Machine ID not matched returning", 0, 0, true, SECONDLEVEL);
			dwRet = 0x06;
			return dwRet;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetRegistrationDataFromRegistry");
	}
Cleanup:
	return dwRet;
}
DWORD CRegistrationDlg::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
{
	try
	{
		if (IsBadWritePtr(lpBuffer, dwSize))
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			if (lpBuffer[iIndex] != 0)
			{
				if ((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
				{
					lpBuffer[iIndex] = lpBuffer[iIndex];
				}
				else
				{
					lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
					jIndex++;
				}
				if (jIndex == WRDWIZ_KEYSIZE)
				{
					jIndex = 0x00;
				}
				if (iIndex >= dwSize)
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::DecryptData");
	}
	return 0;
}

void CRegistrationDlg::DisplayFailureMessage(REGFAILUREMSGS dwRes)
{
	CString csMessage;
	try
	{
		UINT iType = MB_ICONEXCLAMATION | MB_OK;
		switch (dwRes)
		{
		case 0x01:
		case 0x02:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE");
			break;
		case 0x03:
			iType = MB_ICONEXCLAMATION | MB_OK;
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_RES_CODE_INVALID");
			break;
		case 0x04:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE");
			break;
		case MACHINEIDMISMATCH:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_USED_REGISTRATION_KEY");
			break;
		case INVALIDEMAILID:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_EMAIL_ID");
			break;
		case COUNTRYCODEINVALID:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_VALID_SUPPORT");
			break;
		case INVALIDREGNUMBER:
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_REGNO");
			break;
		case INVALIDPRODVERSION:
		{
			/*	ISSUE NO - 753 NAME - NITIN K. TIME - 17th June 2014 */
			switch (theApp.m_dwProductID)
			{
			case ESSENTIAL:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Essential");
				break;
			case PRO:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Pro");
				break;
			case ELITE:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Elite");
				break;
			case BASIC:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Basic");
				break;
			case ESSENTIALPLUS:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"), L"Essential Plus");
				break;

			default:
				csMessage.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INVALID_PROD_KEY"));
				break;
			}
		}
		break;
		case USERINFOUPDATEFAILD:
			csMessage.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE"), theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_CONTACT_SUPPORT"));
			//User Information update failed on server side
			break;
		case PRODUCTEXPIRED:
			csMessage.Format(L"%s \n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_PROD_EXP1"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL2"));
			break;
		case NULLRESPONSE:
			csMessage.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE"), theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_CONTACT_SUPPORT"));
			break;
		default:
			break;
		}
		DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, csMessage);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::DisplayFailureMessage");
	}
}

void CRegistrationDlg::DisplayFailureMsgOnUI(CString csMessageType, CString strMessage)
{
	try
	{
		m_svFunSetRegStatusCB.call((SCITER_STRING)csMessageType, (SCITER_STRING)strMessage);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::DisplayFailureMsgOnUI");
	}
}

json::value CRegistrationDlg::GetRegisteredUserDetails(SCITER_VALUE svFunSetUserDetailsCB)
{
	try
	{
		m_svFunSetUserDetailsCB = svFunSetUserDetailsCB;
		ProductRenewalDetails();
	
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetRegisteredUserDetails");
	}
	return 0;
}

/***********************************************************************************************
Function Name  : DisplayFailureMessageForOfflineRegistration
Description    : Displays detailed error message in case offline registration is failed
Author Name    : Nitin K
SR.NO		   :
Date           : 3rd Dec 2015
***********************************************************************************************/
void CRegistrationDlg::DisplayFailureMessageForOfflineRegistration(REGFAILUREMSGS dwRes)
{
	CString csMessage = L"";
	try
	{
		switch (dwRes)
		{
		case 0x01: //failed due to memory corruption
		case 0x02: //failed due to invalid length
		case 0x03: //failed due to decryption buffer failed
		case 0x05: //failed due to CRC failed
		case 0x06: //failed due to CheckSum failed
			csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_ACTIVATION1");
			break;
		case 0x04: //Activation code is invalid
			csMessage.Format(L"%s \n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_ACTIVATION1"),
				theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_ACTIVATION2"));
			break;
		case 07://Expired or Invalid Machine date
			csMessage.Format(L"%s \n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_PROD_EXP1"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL2"));
			break;
		default:
			csMessage.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_ACTIVATION1"));
			break;
		}
		DisplayFailureMsgOnUI(ERROR_FAILURE_MSG, csMessage);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::DisplayFailureMessageForOfflineRegistration");
	}

}

/***************************************************************************
Function Name  : On_GetProductID
Description    : To send product id to UI to Switch between products 
Author Name    : Adil Sheikh
Date           : 29th July 2016
****************************************************************************/
json::value CRegistrationDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}

	return iProdValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CRegistrationDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : Amol Jaware
*  Date			  : 17 Nov 2016
****************************************************************************************************/
json::value CRegistrationDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : AddProdRegInfoToLocal
*  Description    : Adding product registration information locally.
*  Author Name    : Amol Jaware
*  Date			  : 22 June 2017
****************************************************************************************************/
void CRegistrationDlg::AddProdRegInfoToLocal(DWORD dwType, AVACTIVATIONINFO	ActInfo, DWORD dwResSize, DWORD dwResType, TCHAR *pResName, bool bRegWait)
{
	try
	{
		bool bRegFailed = false;

		if (SendRegisteredData2Service(ADD_REGISTRATION_DATA, (LPBYTE)&ActInfo, sizeof(ActInfo), IDR_REGDATA, TEXT("REGDATA"), false))
		{
			AddLogEntry(L"### Failed to SendRegisteredData2Service in CRegistrationDlg AddProdRegInfoToLocal", 0, 0, true, SECONDLEVEL);
			bRegFailed = true;
		}

		Sleep(10);

		if (AddRegistrationDataInRegistry())
		{
			AddLogEntry(L"### AddRegistrationDataInRegistry in CRegistrationDlg AddProdRegInfoToLocal", 0, 0, true, SECONDLEVEL);
			bRegFailed = true;
		}

		Sleep(10);

		if (AddRegistrationDataInFile())
		{
			AddLogEntry(L"### AddRegistrationDataInFile failed in CRegistrationDlg AddProdRegInfoToLocal", 0, 0, true, SECONDLEVEL);
			bRegFailed = true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::AddProdRegInfoToLocal", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
Function Name  : onModalLoop
Description    : for reseting the Lightbox event msgbox
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 7th Oct 2016
/***************************************************************************************************/
json::value CRegistrationDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;

			theApp.m_objCompleteEvent.SetEvent();
			Sleep(200);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SetCheckForDealerRefCode
*  Description    : Function to check if dealer code is entered
*  Author Name    : Jeena Mariam Saji
*  Date			  : 12 Feb 2017
****************************************************************************************************/
json::value CRegistrationDlg::SetCheckForDealerRefCode(SCITER_VALUE svCheckDealerReferralCode)
{
	try
	{
		bCheckCodeValue = false;
		sciter::value m_svCheckDealerReferralCode = svCheckDealerReferralCode.get(false);
		if (m_svCheckDealerReferralCode == true)
		{
			bCheckCodeValue = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SetCheckForDealerRefCode", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function       : GetProxySettingsFromRegistry
Description    : This function to get settings which user has set
Author Name    : Kunal
Date           : 2nd Nov 2018
****************************************************************************/
DWORD CRegistrationDlg::GetProxySettingsFromRegistry()
{
	DWORD dwRet = 0x00;
	try
	{
		CRegKey oRegKey;
		if (oRegKey.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium", KEY_READ) != ERROR_SUCCESS)
		{
			return 0x00;
		}

		DWORD dwProxySett = 0x00;
		if (oRegKey.QueryDWORDValue(L"dwProxySett", dwProxySett) != ERROR_SUCCESS)
		{
			oRegKey.Close();
			return 0x00;
		}

		//0x01 - use internet explorer settings will be added in future
		if (dwProxySett == 0x01)
		{
			dwProxySett = 0x00;
		}

		dwRet = dwProxySett;

		oRegKey.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWinHttpManager::GetProxySettingsFromRegistry", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************
Function       : GetProxyServerDetailsFromDB
Description    : This function will get Proxy details here from sqlite DB
Author Name    : Kunal
Date           : 2ndth Nov 2018
****************************************************************************/
bool CRegistrationDlg::GetProxyServerDetailsFromDB()
{
	try
	{
		//get Proxy details here from sqlite DB
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csSettingsDB = L"";
		csSettingsDB.Format(L"%sVBSETTINGS.DB", csWardWizModulePath);
		if (!PathFileExists(csSettingsDB))
		{
			return false;
		}

		INTERNET_PORT nPort;
		CT2A dbPath(csSettingsDB, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
		objSqlDb.Open();
		CWardwizSQLiteTable qResult = objSqlDb.GetTable("SELECT SERVER, PORT, USERNAME, PASSWORD FROM WWIZPROXYSETTINGS;");
		DWORD dwRows = qResult.GetNumRows();

		if (dwRows != 0)
		{
			m_csServer = qResult.GetFieldValue(0);
			const char *pszPort = qResult.GetFieldValue(1);
			if (!pszPort)
				return false;

			if (strlen(pszPort) == 0x00)
			{
				objSqlDb.Close();
				return false;
			}

			nPort = atoi(pszPort);

			//to read unicode string from db
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(2), -1, NULL, 0);
			wchar_t *wstrDbData = new wchar_t[wchars_num];
			if (wstrDbData == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CRegistrationDlg::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
				objSqlDb.Close();
				return false;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(2), -1, wstrDbData, wchars_num);
			m_csUserName = wstrDbData;
			delete[] wstrDbData;

			//to read unicode string from db
			wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(3), -1, NULL, 0);
			wstrDbData = new wchar_t[wchars_num];
			if (wstrDbData == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CRegistrationDlg::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
				objSqlDb.Close();
				return false;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(3), -1, wstrDbData, wchars_num);
			m_csPassword = wstrDbData;
			delete[] wstrDbData;
		}

		objSqlDb.Close();
		if (dwRows > 0)
		{
			m_csServer.AppendFormat(L":%d", nPort);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : Kunal Waghmare
*  Date			  : 26 Dec 2018
****************************************************************************************************/
json::value CRegistrationDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : This function is used to send DB file path to UI
*  Author Name    : NITIN SHELAR
*  Date           :	10 August 2018
****************************************************************************************************/
json::value CRegistrationDlg::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		CString csModulePath = GetModuleFilePath();
		swprintf_s(szActualIPath, L"%s\\%s", csModulePath, L"RLOC.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : FunGetSetRegisterData
*  Description    : This function is used to get data for product registration page
*  Author Name    : NITIN SHELAR
*  Date           :	17 August 2018
****************************************************************************************************/
json::value CRegistrationDlg::FunGetMachineId()
{
	TCHAR szMachinId[MAX_PATH] = { 0 };
	try
	{
		CITinRegWrapper objReg;
		TCHAR szValue[0x80] = { 0 };
		DWORD dwSize = sizeof(szValue);
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"MVersion", szValue, dwSize) != 0)
		{
			AddLogEntry(L"### Error in CRegistrationDlg::FunGetMachineId", 0, 0, true, SECONDLEVEL);
		}
		_tcscpy_s(szMachinId, 0x80, szValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::FunGetMachineId", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szMachinId);
}

/***************************************************************************************************
*  Function Name  : GenerateGUIID
*  Description    : Function to use GUI ID
*  Author Name    : Ramkrushna Shelke
*  Date           :	28 Jan 2023
****************************************************************************************************/
CString CRegistrationDlg::GenerateGUIID()
{
	CString csRetString = EMPTY_STRING;
	// Create a new uuid
	UUID uuid;
	RPC_STATUS ret_val = ::UuidCreate(&uuid);

	if (ret_val == RPC_S_OK)
	{
		// convert UUID to LPWSTR
		WCHAR* wszUuid = NULL;
		::UuidToStringW(&uuid, (RPC_WSTR*)&wszUuid);
		if (wszUuid != NULL)
		{
			//TODO: do something with wszUuid
			csRetString.Format(L"%s", wszUuid);
			csRetString.MakeUpper();
			
			// free up the allocated string
			::RpcStringFreeW((RPC_WSTR*)&wszUuid);
			wszUuid = NULL;
		}
	}
	return csRetString;
}

/***************************************************************************************************
*  Function Name  : FunGetUUID
*  Description    : This function is used to get data for product registration page
*  Author Name    : Ramkrushna Shelke
*  Date           :	28 Jan 2023
****************************************************************************************************/
json::value CRegistrationDlg::FunGetUUID()
{
	TCHAR szUUId[MAX_PATH] = { 0 };
	try
	{
		if (_tcslen(m_ActInfo.szInstID) > 0)
			return json::value((SCITER_STRING)m_ActInfo.szInstID);

		CString csGUIID = GenerateGUIID();

		if (csGUIID.GetLength() == 0)
		{
			CString csLastError;
			csLastError.Format(L"%d", GetLastError());
			AddLogEntry(L"### Failed to generate Installation ID, Last Error: [%s]", csLastError, 0, true, SECONDLEVEL);
			return json::value("");
		}

		TCHAR szValue[0x25] = { 0 };
		_tcscpy_s(szUUId, 0x25, csGUIID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::FunGetMachineId", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szUUId);
}

/***************************************************************************************************
*  Function Name  : On_GetInternetConnection
*  Description    : This function is used to check internet connection
*  Author Name    : Akshay Patil
*  Date           :	11 April 2019
****************************************************************************************************/
json::value CRegistrationDlg::On_GetInternetConnection()
{
	try
	{
		WinHttpClient client(L"http://www.google.com", NULL, 0x01);
		client.SetTimeouts(HTTP_RESOLVE_TIMEOUT, HTTP_CONNECT_TIMEOUT, HTTP_SEND_TIMEOUT, HTTP_RECEIVE_TIMEOUT);
		return (client.SendHttpRequest());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_GetInternetConnection", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  : On_GetProxyRegistry
*  Description    : This function is used to get proxy registry entry. If found returns true else false.
*  Author Name    : Akshay Patil
*  Date           :	11 April 2019
****************************************************************************************************/
json::value CRegistrationDlg::On_GetProxyRegistry()
{
	try
	{
		DWORD dwProxy_Sett = GetProxySettingsFromRegistry();
		if (dwProxy_Sett == 0x02)
		{
			return true;
		}		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::On_GetProxyRegistry", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : GetHostFileForWWRedirectionFlag
*  Description    : Checks hosts file for local redirection. if found, returns true else false.
*  Author Name    : Akshay Patil
*  Date           :	11 April 2019
****************************************************************************************************/
json::value CRegistrationDlg::GetHostFileForWWRedirectionFlag()
{
	try
	{
		if (ReadHostFileForWWRedirection())
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetHostFileForWWRedirectionFlag", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : CheckForProductRenewal
*  Description    : Checks if product came for renewal or not.
*  Author Name    : Akshay Patil
*  Date           : 11 April 2019
****************************************************************************************************/
json::value CRegistrationDlg::CheckForProductRenewal()
{
	try
	{
		if (theApp.m_bIsProductRenewal)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::CheckForProductRenewal", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : GetSystemInformation
*  Description    : Function to get System Info from INI
*  Author Name    : Akshay Patil
*  Date           : 15 May 2019
****************************************************************************************************/
json::value CRegistrationDlg::GetSystemInformation(SCITER_VALUE svSetSystemInfoCB)
{
	try
	{
		m_svSetSystemInfoCB = svSetSystemInfoCB;
		SystemInfoDetails();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetSystemInformation");
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : SystemInfoDetails
*  Description    : Function to get System Info from INI
*  Author Name    : Akshay Patil
*  Date           : 15 May 2019
****************************************************************************************************/
void CRegistrationDlg::SystemInfoDetails()
{
	try
	{
		CString csOSName = L"";
		CString csRamSize = L"";
		CString csHardDiskSize = L"";
		CString csProcessor = L"";
		CString csComputerName = L"";
		CString csLoggedInUser = L"";
		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		TCHAR szOSName[MAX_PATH] = { 0 };
		TCHAR szRamSize[MAX_PATH] = { 0 };
		TCHAR szHardDiskSize[MAX_PATH] = { 0 };
		TCHAR szProcessor[MAX_PATH] = { 0 };
		TCHAR szComputerName[MAX_PATH] = { 0 };
		TCHAR szLoggedInUser[MAX_PATH] = { 0 };

		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			AddLogEntry(L"### CRegistrationDlg::SystemInfoDetails::GetModulePath failed", 0, 0, true, SECONDLEVEL);
			return;
		}

		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		GetPrivateProfileString(L"VBSETTINGS", L"OSName", L"", szOSName, sizeof(szOSName), szFullFilePath);
		csOSName = szOSName;

		GetPrivateProfileString(L"VBSETTINGS", L"RamSize", L"", szRamSize, sizeof(szRamSize), szFullFilePath);
		csRamSize = szRamSize;

		GetPrivateProfileString(L"VBSETTINGS", L"HardDiskSize", L"", szHardDiskSize, sizeof(szHardDiskSize), szFullFilePath);
		csHardDiskSize = szHardDiskSize;

		GetPrivateProfileString(L"VBSETTINGS", L"Processor", L"", szProcessor, sizeof(szProcessor), szFullFilePath);
		csProcessor = szProcessor;

		GetPrivateProfileString(L"VBSETTINGS", L"ComputerName", L"", szComputerName, sizeof(szComputerName), szFullFilePath);
		csComputerName = szComputerName;

		GetPrivateProfileString(L"VBSETTINGS", L"CurrentUserName", L"", szLoggedInUser, sizeof(szLoggedInUser), szFullFilePath);
		csLoggedInUser = szLoggedInUser;

		const sciter::value data[6] = { sciter::string(csOSName), sciter::string(csRamSize), sciter::string(csHardDiskSize),
			sciter::string(csProcessor), sciter::string(csComputerName), sciter::string(csLoggedInUser) };

		sciter::value svArrSystemDetails = sciter::value(data, 6);
		m_svSetSystemInfoCB.call(svArrSystemDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SystemInfoDetails", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : FunCheckInternetAccessBlock
*  Description    : To check internet access block
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 Dec 2019
****************************************************************************************************/
json::value CRegistrationDlg::FunCheckInternetAccessBlock()
{
	bool RetVal = false;
	try
	{
		DWORD dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
		if (dwProductID == BASIC || dwProductID == ESSENTIAL)
		{
			return false;
		}

		CString csRegKeyVal;
		csRegKeyVal = CWWizSettingsWrapper::GetProductRegistryKey();
		CITinRegWrapper objReg;
		DWORD dwParentalControl = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyVal.GetBuffer(), L"dwParentalCntrlFlg", dwParentalControl) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CRegistrationDlg::FunCheckInternetAccessBlock", 0, 0, true, SECONDLEVEL);
		}

		if (dwParentalControl == 1)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = ON_CHECK_INTERNET_ACCESS;

			CISpyCommunicator objCom(SERVICE_SERVER);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send Data in CRegistrationDlg::SendData", 0, 0, true, SECONDLEVEL);
			}

			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read Data in CRegistrationDlg::ReadData", 0, 0, true, SECONDLEVEL);
			}

			DWORD dwVal = szPipeData.dwValue;
			if (dwVal == 0x01)
			{
				RetVal = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::FunCheckInternetAccessBlock()", 0, 0, true, SECONDLEVEL);
	}
	return RetVal;
}

void CRegistrationDlg::SendRegInfo2Server()
{
	try
	{
		//send reload message to service
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = UPDATE_REGISTRATION;

		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationDlg::SendData", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SendRegInfo2Server()", 0, 0, true, SECONDLEVEL);
	}
}