#pragma once

#include "stdafx.h"
#include "WinHttpClient.h"
#include "PipeConstants.h"
#include "ISpyCommunicator.h"
#include "WardWizRegistration.h"
#include "iTINRegWrapper.h"
#include "RegistrationSecondDlg.h"
#include "WWizSettingsWrapper.h"

typedef DWORD (*GETREGISTRATIONDATA)	(LPBYTE, DWORD & ,DWORD dwResType, TCHAR *pResName  ) ;
typedef DWORD (*GETINSTALLATIONCODE)(LPCTSTR lpSerialNumber, LPCTSTR lpMID, LPTSTR lpInstallationCode);
typedef DWORD (*VALIDATERESPONSE)(LPTSTR lpActivationCode, BYTE bProductID, SYSTEMTIME &ServetTime, WORD &wwDaysLeft);
//typedef DWORD (*GETACTIVATIONCODE)	(LPCTSTR lpInstallationCode, int iDaysLeft, LPTSTR lpActivationCode);//Temp input

#define				IDR_REGDATA					2000
#define				IDR_QUARDATA				2001

//GETREGISTRATIONDATA		GetRegistrationData = NULL ;
//GETINSTALLATIONCODE		g_GetInstallationCode = NULL;
//VALIDATERESPONSE		g_ValidateResponse = NULL;
//GETACTIVATIONCODE		GetActivationCode = NULL;//Temp Input

//DWORD WINAPI SendRequestThread(LPVOID lpThreadParam);

extern AVACTIVATIONINFO	m_RegDlg_ActInfo ;

//AVACTIVATIONINFO	g_ActInfo = {0} ;

IMPLEMENT_DYNAMIC(CRegistrationSecondDlg, CDialog)

CRegistrationSecondDlg::CRegistrationSecondDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CRegistrationSecondDlg::IDD, pParent)
	, m_pTextFont(NULL)
	, m_pBoldFont(NULL)
	, m_hSendRequestThread(NULL)
	,m_bMisMatchedEmailID(false)
	,m_bisThreadCompleted(false)
	,m_RegisterationDLL(NULL)
	,m_hOfflineNextEvent(NULL)
	,m_hOfflineDLL(NULL)
{
}

CRegistrationSecondDlg::~CRegistrationSecondDlg()
{
	if(m_pTextFont != NULL)
	{
		delete m_pTextFont;
		m_pTextFont = NULL;
	}
	if(m_pBoldFont != NULL)
	{
		delete m_pBoldFont;
		m_pBoldFont = NULL;
	}
	if(m_RegisterationDLL != NULL)
	{
		FreeLibrary(m_RegisterationDLL);
		m_RegisterationDLL = NULL;
	}
	if(m_hOfflineDLL != NULL)
	{
		FreeLibrary(m_hOfflineDLL);
		m_hOfflineDLL = NULL;
	}
}

void CRegistrationSecondDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CANCEl, m_btnCancel);
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_btnBack);
	DDX_Control(pDX, IDC_STATIC_INTERNET_TEXT, m_stInternetReqText);
	DDX_Control(pDX, IDC_STATIC_ACTIVEPRODUCT, m_stActiveProduct);
	DDX_Control(pDX, IDC_STATIC_TRYPRODUCT_TEXT, m_stTryProdText);
	DDX_Control(pDX, IDC_RADIO_ACTIVEPRODUCT, m_btnActiveProduct);
	DDX_Control(pDX, IDC_RADIO_TRY_PRODUCT, m_btnTryProduct);
	DDX_Control(pDX, IDC_EDIT_REGISTRATION_KEY, m_edtRegistrationkey);
	DDX_Control(pDX, IDC_STATIC_CLICK_TRYPRODUCT, m_stClickTryText);
	DDX_Control(pDX, IDC_STATIC_CLICKINFO_ACTIVEPRODUCT, m_stClickActiveText);
	DDX_Control(pDX, IDC_STATIC_EXAMPLE, m_stexample);
	DDX_Control(pDX, IDC_RADIO_ONLINE_ACTIVATION, m_btnOnlineActivation);
	DDX_Control(pDX, IDC_RADIO_OFFLINE_ACTIVATION, m_btnOfflineActivation);
	DDX_Control(pDX, IDC_STATIC_ONLINE_ACTIVATION, m_stOnlineActivation);
	DDX_Control(pDX, IDC_STATIC_OFFLINE_ACTIVATION, m_stOfflineActivation);
}

BEGIN_MESSAGE_MAP(CRegistrationSecondDlg, CJpegDialog)
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_CANCEl, &CRegistrationSecondDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CRegistrationSecondDlg::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CRegistrationSecondDlg::OnBnClickedButtonBack)
	ON_BN_CLICKED(IDC_RADIO_ACTIVEPRODUCT, &CRegistrationSecondDlg::OnBnClickedRadioActiveproduct)
	ON_BN_CLICKED(IDC_RADIO_TRY_PRODUCT, &CRegistrationSecondDlg::OnBnClickedRadioTryProduct)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_RADIO_ONLINE_ACTIVATION, &CRegistrationSecondDlg::OnBnClickedRadioOnlineActivation)
	ON_BN_CLICKED(IDC_RADIO_OFFLINE_ACTIVATION, &CRegistrationSecondDlg::OnBnClickedRadioOfflineActivation)
END_MESSAGE_MAP()


BOOL CRegistrationSecondDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();
	
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SECOND_REGISTRATIONUI), _T("JPG")))
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	}

	Draw();
	
	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect;
	this->GetClientRect(rect);
	SetWindowPos(NULL, 0, 97, rect.Width()-3, rect.Height() - 30, SWP_NOREDRAW);

	m_objRegistrationThirdDlg.Create(IDD_DIALOG_REGISTRATION_THIRD, this);
	m_objRegistrationThirdDlg.ShowWindow(SW_HIDE);

	m_objRegistrationForthDlg.Create(IDD_DIALOG_REGISTRATION_FORTH, this);
	m_objRegistrationForthDlg.ShowWindow(SW_HIDE);

	m_pTextFont=NULL;
	m_pTextFont = new(CFont);
	m_pTextFont->CreateFont(15,6,0,0,FW_NORMAL,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"verdana");

	m_pBoldFont=NULL;
	m_pBoldFont = new(CFont);
	m_pBoldFont->CreateFont(17,8,0,0,FW_BOLD,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"verdana");

		
	m_stInternetReqText.SetWindowPos(&wndTop, rect.left + 52, 100, 400,20, SWP_NOREDRAW);

	m_stInternetReqText.SetBkColor(RGB(255,0,0));
	m_stInternetReqText.SetFont(m_pBoldFont);
	m_stInternetReqText.ShowWindow(SW_HIDE);
	
	/*	ISSUE NO - 349 NAME - NITIN K. TIME - 24st May 2014 */
	m_btnActiveProduct.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE,IDB_BITMAP_SELECT,IDB_BITMAP_DISABLE,IDB_BITMAP_DISABLE,0,0,0,0,0);
	m_btnActiveProduct.SetWindowPos(&wndTop, rect.left + 52, 100, 20,20, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnActiveProduct.SetCheck(TRUE);
	m_bActiveProduct = true;

	m_stActiveProduct.SetWindowPos(&wndTop, rect.left + 90, 100,300,15, SWP_NOREDRAW | SWP_NOZORDER);
	m_stActiveProduct.SetBkColor(RGB(251,252,254));
	m_stActiveProduct.SetFont(m_pTextFont);

	m_stClickActiveText.SetWindowPos(&wndTop, rect.left + 90, 125, 580,15, SWP_NOREDRAW | SWP_NOZORDER);
	m_stClickActiveText.SetBkColor(RGB(251,252,254));
	m_stClickActiveText.SetFont(m_pTextFont);

	m_edtRegistrationkey.SetWindowPos(&wndTop, rect.left + 90, 155, 450,20, SWP_NOREDRAW | SWP_NOZORDER);
	m_edtRegistrationkey.SetLimitText(12);

	m_stexample.SetWindowPos(&wndTop, rect.left + 90, 185, 240,15, SWP_NOREDRAW | SWP_NOZORDER);
	m_stexample.SetBkColor(RGB(251,252,254));
	m_stexample.SetFont(m_pTextFont);

	/*	ISSUE NO - 349 NAME - NITIN K. TIME - 24st May 2014 */
	//Do not show Offline regisration for Non Clam Setup
	m_btnTryProduct.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
	m_btnTryProduct.SetWindowPos(&wndTop, rect.left + 52, 270, 20, 20, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnTryProduct.SetCheck(FALSE);

	m_stTryProdText.SetWindowPos(&wndTop, rect.left + 90, 270, 360, 15, SWP_NOREDRAW | SWP_NOZORDER);
	m_stTryProdText.SetBkColor(RGB(251, 252, 254));
	m_stTryProdText.SetFont(m_pTextFont);

	m_stClickTryText.SetWindowPos(&wndTop, rect.left + 90, 300, 360, 30, SWP_NOREDRAW | SWP_NOZORDER);
	m_stClickTryText.SetBkColor(RGB(251, 252, 254));
	m_stClickTryText.SetFont(m_pTextFont);

	m_bOnlineActivation = true;
	m_btnOfflineActivation.ShowWindow(SW_HIDE);
	m_stOfflineActivation.ShowWindow(SW_HIDE);
	m_btnOnlineActivation.ShowWindow(SW_HIDE);
	m_stOnlineActivation.ShowWindow(SW_HIDE);
	if (theApp.m_dwIsOffline)
	{
		m_btnOfflineActivation.ShowWindow(SW_SHOW);
		m_stOfflineActivation.ShowWindow(SW_SHOW);
		m_btnOnlineActivation.ShowWindow(SW_SHOW);
		m_stOnlineActivation.ShowWindow(SW_SHOW);
		m_btnOnlineActivation.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
		m_btnOnlineActivation.SetWindowPos(&wndTop, rect.left + 90, 210, 20, 20, SWP_NOREDRAW | SWP_NOZORDER);
		m_btnOnlineActivation.SetCheck(TRUE);
		m_bOnlineActivation = true;

		m_stOnlineActivation.SetWindowPos(&wndTop, rect.left + 128, 210, 580, 20, SWP_NOREDRAW | SWP_NOZORDER);
		m_stOnlineActivation.SetBkColor(RGB(251, 252, 254));
		m_stOnlineActivation.SetFont(m_pTextFont);
		m_bOfflineActivation = false;

		m_btnOfflineActivation.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
		m_btnOfflineActivation.SetWindowPos(&wndTop, rect.left + 90, 235, 20, 20, SWP_NOREDRAW | SWP_NOZORDER);
		m_btnOfflineActivation.SetCheck(FALSE);

		m_stOfflineActivation.SetWindowPos(&wndTop, rect.left + 128, 235, 580, 20, SWP_NOREDRAW | SWP_NOZORDER);
		m_stOfflineActivation.SetBkColor(RGB(251, 252, 254));
		m_stOfflineActivation.SetFont(m_pTextFont);
	}
	else
	{
		m_btnActiveProduct.SetWindowPos(&wndTop, rect.left + 52, 125, 20, 20, SWP_NOREDRAW | SWP_NOZORDER);
		m_stActiveProduct.SetWindowPos(&wndTop, rect.left + 90, 125, 300, 15, SWP_NOREDRAW | SWP_NOZORDER);
		m_stClickActiveText.SetWindowPos(&wndTop, rect.left + 90, 150, 580, 15, SWP_NOREDRAW | SWP_NOZORDER);
		m_edtRegistrationkey.SetWindowPos(&wndTop, rect.left + 90, 180, 450, 20, SWP_NOREDRAW | SWP_NOZORDER);
		m_stexample.SetWindowPos(&wndTop, rect.left + 90, 210, 240, 15, SWP_NOREDRAW | SWP_NOZORDER);

		m_btnTryProduct.SetWindowPos(&wndTop, rect.left + 52, 250, 20, 20, SWP_NOREDRAW | SWP_NOZORDER);
		m_stTryProdText.SetWindowPos(&wndTop, rect.left + 90, 250, 360, 15, SWP_NOREDRAW | SWP_NOZORDER);
		m_stClickTryText.SetWindowPos(&wndTop, rect.left + 90, 280, 360, 30, SWP_NOREDRAW | SWP_NOZORDER);
	}
	


	m_btnBack.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnBack.SetWindowPos(&wndTop, rect.left+ 52, 340,57,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnBack.SetTextColorA((0,0,0),1,1);

	m_btnNext.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnNext.SetWindowPos(&wndTop, rect.left + 555, 340, 57,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnNext.SetTextColorA((0,0,0),1,1);

	m_btnCancel.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnCancel.SetWindowPos(&wndTop, rect.left + 630, 340, 57,21, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnCancel.SetTextColorA((0,0,0),1,1);

	/*	ISSUE NO - 520 NAME - NITIN K. TIME - 3rd June 2014 */
	int iEmailIDLen = 0;
	//iEmailIDLen = static_cast<int>(_tcslen(g_ActInfo.szEmailID));
	DWORD dwGetDaysLeft = theApp.GetDaysLeft(theApp.m_dwProductID);
	if(dwGetDaysLeft > 0 && dwGetDaysLeft <= 30 || iEmailIDLen)
	{
		m_btnTryProduct.EnableWindow(false);
		m_stTryProdText.EnableWindow(false);
		m_stClickTryText.EnableWindow(false);
	}
	else
	{
		m_btnTryProduct.EnableWindow(true);
		m_stTryProdText.EnableWindow(true);
		m_stClickTryText.EnableWindow(true);

	}
		
	ShowHideSecondDlg(true);
	if(!RefreshString())
	{
		AddLogEntry(L">>> CRegistrationSecondDlg::OnInitDialog :: Failed RefreshString()", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CRegistrationSecondDlg::OnBnClickedButtonCancel()
{
	ShutdownThread();
	this->ShowWindow(SW_HIDE);
	CRegistrationDlg *pObjRegWindow = reinterpret_cast<CRegistrationDlg*>(this->GetParent());
	pObjRegWindow->OnBnClickedButtonRegistrationCancel();
}

void CRegistrationSecondDlg::OnBnClickedButtonNext()
{
	CString csChkWindowText;
	CString	csKey = L"" ;
	m_btnNext.GetWindowTextW(csChkWindowText);
	if(csChkWindowText == L"Finish")
	{
		OnBnClickedButtonCancel();
		return;
	}
	if(m_bActiveProduct == true)
	{
		m_hOfflineNextEvent = NULL;
		m_hOfflineNextEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if(!ValidateRegistrationKey())
		{
			//MessageBox(L"Registration key is not valid",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
			return;
		}
		TCHAR szKey[0x32] = {0};
		m_edtRegistrationkey.GetWindowText(szKey, 0x32);
		wcscpy(m_RegDlg_ActInfo.szKey, szKey) ;
	}

	TCHAR szGlobalIP[64] = {0};
	TCHAR	szInstallCode[32] = {0};

	if(m_bOfflineActivation == true && m_bActiveProduct == true)
	{
		if(!CheckForMIDInRegistry())
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		if(!LoadWrdWizOfflineRegDll())
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			AddLogEntry(L"### Failed to locate VBOFFLINEREG.DLL",0,0,true,SECONDLEVEL);
			return;
		}
		/*if( g_GetInstallationCode )
			g_GetInstallationCode(m_RegDlg_ActInfo.szKey, m_szMachineIDValue, szInstallCode );*/

		if(!szInstallCode[0])
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			AddLogEntry(L"### Generation of installation code is failed",0,0,true,SECONDLEVEL);
			return;
		}

		if(_tcslen(szInstallCode) > 17)
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INSTALL_FAILED_CONTACT_SUPPORT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			AddLogEntry(L"### Generation of invalid installation code",0,0,true,SECONDLEVEL);
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
		_tcscat(m_szInstallCode , szInstallCode);
		ShowHideSecondDlg(false);
		theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_FORTHDLG_HEADER"));
		m_objRegistrationForthDlg.ShowWindow(SW_SHOW);
		m_objRegistrationForthDlg.ShowHideForthDlg(true);
		m_objRegistrationForthDlg.RefreshString();
		Invalidate();
	}
	else if((m_bOnlineActivation == true && m_bActiveProduct == true) || m_bTryProduct == true)
	{
		//if(!GetGlobalIPAddrFromUrl(szGlobalIP))
		//{
		//	AddLogEntry(L"### Failed to GetGlobalIPAddrFromUrl %s ", szGlobalIP, 0, true, SECONDLEVEL);
		//}
		//wcscpy_s(m_RegDlg_ActInfo.szRegionCode, 64, szGlobalIP) ;
		wcscpy_s(m_RegDlg_ActInfo.szRegionCode, 64, L"") ;
	
		//Ram: To Set Header information
		//ShowHideSecondDlg(false);
		ShowHideSecondDlg(false);
		m_btnOfflineActivation.SetCheck(false);
		theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_PROCESSING"));
		m_objRegistrationThirdDlg.ShowWindow(SW_SHOW);
		m_objRegistrationThirdDlg.RefreshString();
		Invalidate();
	}

	DWORD	dwSendRequstThreadID = 0x00 ;

	dwDaysRemain = 0x00 ;
	//m_hSendRequestThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) SendRequestThread, this, 0, &dwSendRequstThreadID ) ;
	Sleep( 1000 ) ;
}

void CRegistrationSecondDlg::OnBnClickedButtonBack()
{
	m_btnActiveProduct.SetCheck(true);
	m_btnOnlineActivation.SetCheck(true);
	m_btnOfflineActivation.SetCheck(false);
	/*	ISSUE NO - 532 NAME - NITIN K. TIME - 30th May 2014 */
	m_edtRegistrationkey.SetWindowTextW(L"");
	m_edtRegistrationkey.EnableWindow(TRUE);
	m_btnOnlineActivation.EnableWindow(TRUE);
	m_btnOfflineActivation.EnableWindow(TRUE);
	m_stOnlineActivation.EnableWindow(TRUE);
	m_stOfflineActivation.EnableWindow(TRUE);
	m_btnTryProduct.SetCheck(false);
	m_bOnlineActivation = true;
	m_bOfflineActivation = false;
	m_bTryProduct = false;
	m_bActiveProduct = true;
	CRegistrationDlg *pObjRegWindow = reinterpret_cast<CRegistrationDlg*>(this->GetParent());
	this->ShowWindow(SW_HIDE);
	/*	ISSUE NO - 528 NAME - NITIN K. TIME - 30th May 2014 */
	pObjRegWindow->SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_WELCOME_TEXT"));
	pObjRegWindow->ShowHideRegistration(true);
}

void CRegistrationSecondDlg::OnBnClickedRadioActiveproduct()
{
	m_bActiveProduct = true;
	m_bTryProduct =false;
	m_btnTryProduct.SetCheck(FALSE);

	//Issue 1243. Flag should be set properly.
	if (m_btnOnlineActivation.GetCheck() == BST_CHECKED)
	{
		m_bOnlineActivation = true;
		m_bOfflineActivation = false;
	}
	if (m_btnOfflineActivation.GetCheck() == BST_CHECKED)
	{
		m_bOnlineActivation = false;
		m_bOfflineActivation = true;
	}
	/*	ISSUE NO - 350 NAME - NITIN K. TIME - 24st May 2014 */
	//issue :- When user is having 365 days registration after completing 365 days when we click on "renew",Activate Product becomes enabled when we click on it
	// resolve by lalit 4-28-2015
	if (!theApp.m_bIsProductRenewal)
	{
		m_edtRegistrationkey.EnableWindow(TRUE);
	}
	m_btnOnlineActivation.EnableWindow(TRUE);
	m_btnOfflineActivation.EnableWindow(TRUE);
	m_stOnlineActivation.EnableWindow(TRUE);
	m_stOfflineActivation.EnableWindow(TRUE);
}

bool CRegistrationSecondDlg::ValidateRegistrationKey()
{
	TCHAR InputString[16];
	int icount=0;
	int idigit=0;
	int iAlpha=0;
	m_edtRegistrationkey.GetWindowText(InputString,16);
	int ilen = static_cast<int>(_tcslen(InputString));

	if(!(_tcscmp(InputString,L"")))
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_KEY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
		return false;	
	}

	if(ilen != 12)
	{
		if(ilen != 12)
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_VALID_KEY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
			return false;
		}
	}
	if(m_bOfflineActivation)
	{
		if(ilen != 12)
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_VALID_KEY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
			return false;
		}
	}

	for(int i=0;i<ilen;i++)
	{
		if(!isdigit(InputString[i]))
		{
			idigit++;
		}
	}
	if(idigit > 0)
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_VALID_KEY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
		return false;
	}
	if(_tcscmp(InputString,L"000000000000")==0)
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_VALID_KEY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
		return false;
	}
	if(_tcscmp(InputString,L"000000000000")==0)
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_VALID_KEY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

void CRegistrationSecondDlg::OnBnClickedRadioTryProduct()
{
	m_bActiveProduct = false;
	m_bTryProduct = true;
	m_bOnlineActivation = false;
	m_bOfflineActivation = false;
	m_btnActiveProduct.SetCheck(FALSE);
	m_btnOfflineActivation.SetCheck(FALSE);
	/*	ISSUE NO - 350 NAME - NITIN K. TIME - 24st May 2014 */
	m_edtRegistrationkey.SetWindowTextW(L"");
	m_edtRegistrationkey.EnableWindow(FALSE);
	m_btnOnlineActivation.SetCheck(TRUE);
	m_btnOnlineActivation.EnableWindow(FALSE);
	m_btnOfflineActivation.EnableWindow(FALSE);
	m_stOnlineActivation.EnableWindow(FALSE);
	m_stOfflineActivation.EnableWindow(FALSE);
}

//DWORD WINAPI SendRequestThread(LPVOID lpThreadParam)
//{
//	CRegistrationSecondDlg	*pRegDlg = (CRegistrationSecondDlg *) lpThreadParam ;
//	if( !pRegDlg )
//		return 1 ;
//
//	if(pRegDlg->m_bOnlineActivation || pRegDlg->m_bTryProduct )
//	{
//		pRegDlg->m_bisThreadCompleted = true;
//	}
//	
//	pRegDlg->m_objRegistrationThirdDlg.EnableFinish(false);
//
//	DWORD	dwRet = 0x00 ;
//
//	//Using Host redirection product may get hacked
//	//We are checking with host file that if any redirection with wardwiz.com (or) www.wardwiz.com
//	//will be avoided.
//	//It will check only in case of online registration.
//	//Issue No: 1213 Trial product get register with 38 server.
//	if (pRegDlg->m_bOnlineActivation == true || pRegDlg->m_bTryProduct)
//	{
//		if (pRegDlg->ReadHostFileForWWRedirection())
//		{
//		AddLogEntry(L"### Hosts redirection found, please check hosts file present in Drivers folder", 0, 0, true, SECONDLEVEL);
//		goto FAILED;
//		}
//	}
//
//	if( pRegDlg->GetRegisteredUserInfo(theApp.m_dwProductID) || pRegDlg->m_bActiveProduct)
//	{
//		SYSTEMTIME		CurrTime = {0} ;
//
//		WORD		wDaysLeft = 0x00;
//		
//		memset(&pRegDlg->m_ActInfo, 0x00, sizeof(pRegDlg->m_ActInfo) ) ;
//		memcpy(&pRegDlg->m_ActInfo, &m_RegDlg_ActInfo, sizeof(m_RegDlg_ActInfo) ) ;
//		GetSystemTime( &CurrTime ) ;
//		pRegDlg->m_ActInfo.RegTime = CurrTime ;
//
//		DWORD				dwDaysLeft = 0x00 ;
//		AVACTIVATIONINFO	ActInfo ={0} ;
//		memcpy(&ActInfo, &pRegDlg->m_ActInfo, sizeof(ActInfo) ) ;
//
//		//Write here Product Number
//		pRegDlg->m_ActInfo.dwProductNo = theApp.m_dwProductID ;
//		ActInfo.dwProductNo = theApp.m_dwProductID ;
//
//		CString csProductID;
//		csProductID.Format(L">>> Product ID: %d", pRegDlg->m_ActInfo.dwProductNo);
//		AddLogEntry(csProductID);
//
//		//Send data to server & Get Server response
//		if(pRegDlg->m_bOnlineActivation == true || pRegDlg->m_bTryProduct == true)
//		{
//			//Send data to server & Get Server response
//			DWORD dwRes = pRegDlg->GetRegistrationDateFromServer( &CurrTime, &dwDaysLeft );
//			if( dwRes > 0)
//			{
//				pRegDlg->m_objRegistrationThirdDlg.ShowMsg(false);
//				pRegDlg->m_objRegistrationThirdDlg.EnableFinish(true);
//				theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_STATUS"));
//				pRegDlg->DisplayFailureMessage((REGFAILUREMSGS)dwRes);
//				goto FAILED;
//			}		
//
//			////Ram, Resolved Issue No: 0001075
//			pRegDlg->m_ActInfo.RegTimeServer = CurrTime;
//			ActInfo.RegTimeServer = CurrTime;
//			ActInfo.dwTotalDays = dwDaysLeft;
//			
//			//Local time gives your local time rather than UTC.
//			GetSystemTime(&CurrTime);
//			pRegDlg->m_ActInfo.RegTime = CurrTime;
//			ActInfo.RegTime = CurrTime;
//			pRegDlg->m_ActInfo.dwTotalDays = dwDaysLeft;
//		}
//
//		if(pRegDlg->m_bOfflineActivation == true)
//		{
//			TCHAR szTempActivationCode[32] = {0}; 
//			//code for offline registration.
//			WaitForSingleObject(pRegDlg->m_hOfflineNextEvent ,INFINITE);
//
//			pRegDlg->m_bisThreadCompleted = true;
//
//			/*if(g_ValidateResponse)
//			{
//				DWORD dwValidResponse = g_ValidateResponse(pRegDlg->m_objRegistrationForthDlg.m_szActivationCode,(BYTE)theApp.m_dwProductID,CurrTime,wDaysLeft);
//				if(dwValidResponse)
//				{
//					pRegDlg->DisplayFailureMessageForOfflineRegistration((REGFAILUREMSGS)dwValidResponse);
//					goto FAILED;
//				}
//			}*/
//			dwDaysLeft = static_cast<DWORD>(wDaysLeft);
//		
//			ActInfo.RegTime = pRegDlg->m_ActInfo.RegTime = CurrTime;
//			ActInfo.dwTotalDays = pRegDlg->m_ActInfo.dwTotalDays = dwDaysLeft;
//		}
//
//		ResetEvent(pRegDlg->m_hOfflineNextEvent);
//
//		//memcpy(&g_ActInfo, &ActInfo, sizeof(ActInfo) ) ;
//
//		pRegDlg->m_objRegistrationThirdDlg.m_btnThirdCancel.EnableWindow(false);
//		
//		Sleep(10);
//		if(!pRegDlg->SendRegisteredData2Service(ADD_REGISTRATION_DATA, (LPBYTE)&ActInfo, sizeof(ActInfo), IDR_REGDATA, TEXT("REGDATA"),true))
//		{
//			AddLogEntry(L"### Failed to SendRegisteredData2Service in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
//			goto FAILED;
//		}
//		Sleep(10);
//		if( pRegDlg->AddRegistrationDataInRegistry( ) )
//		{
//			AddLogEntry(L"### AddRegistrationDataInRegistry in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
//			goto FAILED;
//		}
//		
//		if( pRegDlg->AddRegistrationDataInFile( ) )
//		{
//			AddLogEntry(L"### AddRegistrationDataInFile failed in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
//		}
// 	}
//	else
//	{
//
//		//memcpy(&g_ActInfo, &pRegDlg->m_ActInfo, sizeof(g_ActInfo) ) ;
//		SYSTEMTIME		CurrTime = {0} ;
//		GetSystemTime( &CurrTime ) ;
//		CTime	Time_Reg( pRegDlg->m_ActInfo.RegTime ) ;
//		CTime	Time_Curr( CurrTime ) ;
//
//		DWORD	dwDays = 0x00 ;
//
//		if( Time_Curr > Time_Reg )
//		{
//			CTimeSpan	Time_Diff = Time_Curr - Time_Reg ;
//			if( Time_Diff.GetDays() < pRegDlg->m_ActInfo.dwTotalDays )
//				dwDays = pRegDlg->m_ActInfo.dwTotalDays - static_cast<DWORD>(Time_Diff.GetDays());
//		}
//
//		if( !dwDays )
//		{
//			CString csMessage;
//			pRegDlg->m_objRegistrationThirdDlg.ShowMsg(false);
//			pRegDlg->m_objRegistrationThirdDlg.EnableFinish(true);
//			theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_STATUS"));
//			csMessage.Format(L"\t%s \n%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL1"),theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL2"));
//			MessageBox(NULL,csMessage, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
//			goto FAILED;
//		}
//	}
//
//	pRegDlg->SpreadRegistrationFilesInSystem() ;
//
//	//No need to do check for Non Clam setup
//	if (theApp.m_dwIsOffline)
//	{
//		//After registration offline or online this flag should be zero. Commsrv decide whether WadWiz copy is genuine or not.
//		if (!pRegDlg->SendRegistryData2Service(SZ_DWORD, L"SOFTWARE\\Vibranium", L"dwNGC", (LPBYTE)L"", 0x00, true))
//		{
//			AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::SendRequestThread", 0, 0, true, SECONDLEVEL);
//		}
//	}
//	if(pRegDlg->m_bOfflineActivation)
//	{
//		//0x01 value for offline registration.
//		if(!pRegDlg->SendRegistryData2Service(SZ_DWORD, L"SOFTWARE\\Vibranium", L"dwRegUserType",(LPBYTE)L"" ,0x01, true))
//		{
//			AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::SendRequestThread", 0, 0, true, SECONDLEVEL);
//			goto FAILED ;
//		}
//	}
//	else if(pRegDlg->m_bOnlineActivation)
//	{
//		//0x00nvalue for online registration.
//		if(!pRegDlg->SendRegistryData2Service(SZ_DWORD, L"SOFTWARE\\Vibranium", L"dwRegUserType",(LPBYTE)L"" ,0x00, true))
//		{
//			AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::SendRequestThread", 0, 0, true, SECONDLEVEL);
//			goto FAILED ;
//		}
//	}
//
//	if(!pRegDlg->SendRegistrationInfo2Service(RELOAD_REGISTARTION_DAYS))
//	{
//		AddLogEntry(L"### Error in CRegistrationSecondDlg SendRequestThread::SendRegistrationInfo2Service", 0, 0, true, SECONDLEVEL);
//	}
//	Sleep(1000 * 3);
//
//	pRegDlg->m_objRegistrationThirdDlg.ShowMsg(true);
//	pRegDlg->m_objRegistrationThirdDlg.EnableFinish(true);
//	theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_STATUS"));
//	return dwRet ;
//
//FAILED:
//	pRegDlg->m_objRegistrationThirdDlg.ShowMsg(false);
//	pRegDlg->m_objRegistrationThirdDlg.EnableFinish(true);
//	theApp.m_objRegistrationDlg.SetRegistrationHeader(theApp.m_objwardwizLangManager.GetString(L"IDS_RES_STATUS"));
//	return dwRet ;
//}

DWORD CRegistrationSecondDlg::GetRegisteredUserInfo(DWORD dwProdID)
{
	DWORD	dwRet = 0x00 ;
	DWORD	dwSize = 0x00 ;
	DWORD	dwRegUserSize = 0x00 ;

	dwDaysRemain = 0x00 ;

	m_RegisterationDLL = NULL ;

	CString	strEvalRegDLL("") ;
	CString	strRegisterDLL("") ;

	CString	strAppPath = GetModuleFilePath() ;
	dwRet = GetRegistrationDataFromRegistry( ) ;

	if( !dwRet )
	{
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			dwRet = 0x06;
		}
		goto Cleanup;
	}

	//check in files
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

	strEvalRegDLL.Format( TEXT("%sVBEVALREG.DLL"), strAppPath ) ;
	if( !PathFileExists( strEvalRegDLL) )
	{
		dwRet = 0x01 ;
		AddLogEntry(L"### VBEVALREG.DLL not found in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
		goto Cleanup;
	}

	strRegisterDLL.Format( TEXT("%sVBREGISTERDATA.DLL"), strAppPath ) ;
	if( !PathFileExists( strRegisterDLL) )
	{
		dwRet = 0x02 ;
		AddLogEntry(L"### VBREGISTERDATA.DLL not found in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
		goto Cleanup;
	}

	m_RegisterationDLL = LoadLibrary( strRegisterDLL ) ;
	GETREGISTRATIONDATA GetRegistrationData = (GETREGISTRATIONDATA)GetProcAddress(m_RegisterationDLL, "GetRegisteredData");

	if( !GetRegistrationData )
	{
		dwRet = 0x04 ;
		AddLogEntry(L"### VBREGISTERDATA.DLL version is incorrect in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
		goto Cleanup;
	}

	dwSize = sizeof(m_ActInfo) ;
	dwRegUserSize = 0x00 ;


	if( GetRegistrationData((LPBYTE)&m_ActInfo, dwRegUserSize , IDR_REGDATA, L"REGDATA" ) == 0)
	{
		if (!CheckForMachineID(m_ActInfo))
		{
			memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
			dwRet = 0x06;
			goto Cleanup;
		}
		dwRet = 0x00 ;
		goto Cleanup;
	}

	if( dwSize != dwRegUserSize )
		dwRet = 0x05 ;

	//Match here the machine ID with Stored machineID
	if (theApp.m_dwProductID != m_ActInfo.dwProductNo)
	{
		dwRet = 0x05;
		return dwRet;
	}

	//Match here the machine ID with Stored machineID
	//if someone provides the DB files from other computer then it works so 
	//necessary to check for machine ID.
	if (!CheckForMachineID(m_ActInfo))
	{
		memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
		dwRet = 0x06;
		return dwRet;
	}

Cleanup:

	if( !dwRet )
	{
		//memcpy(&g_ActInfo, &m_ActInfo, sizeof(m_ActInfo) ) ;
	}

	return dwRet ;
}

/***************************************************************************************************                    
*  Function Name  : CheckForMachineID
*  Description    : function which compare with Machine ID present in DB files & machine ID in registry.
*  Author Name    : Ram 
*  SR_NO		  :
*  Date           : 18 Sep 2013
****************************************************************************************************/
bool CRegistrationSecondDlg::CheckForMachineID(const AVACTIVATIONINFO	&actInfo)
{
	CITinRegWrapper objReg;
	TCHAR szValue[0x80] = {0};
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

	return true;
}

DWORD CRegistrationSecondDlg::GetRegistrationDataFromRegistry( )
{
	DWORD	dwRet = 0x00 ;
	DWORD	dwRegType = 0x00, dwRetSize = 0x00 ;

	HKEY	h_iSpyAV = NULL ;
	HKEY	h_iSpyAV_User = NULL ;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h_iSpyAV) != ERROR_SUCCESS)
	{
		dwRet = 0x01 ;
		goto Cleanup ;
	}

	//if( RegCreateKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Vibranium"), 
	//if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows"), 
	//					0, KEY_READ, &h_iSpyAV ) != ERROR_SUCCESS )
	//{
	//	dwRet = 0x01 ;
	//	goto Cleanup ;
	//}

/*	if( RegCreateKeyEx(	h_iSpyAV, TEXT("UserInfo"), 
						0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
						NULL, &h_iSpyAV_User, NULL ) != ERROR_SUCCESS )
	{
		dwRet = 0x02 ;
		goto Cleanup ;
	}
*/

	dwRetSize = sizeof(m_ActInfo) ;
	/*if( RegQueryValueEx(h_iSpyAV, TEXT("UserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo, 
						&dwRetSize) != ERROR_SUCCESS )
	{
		dwRet = GetLastError() ;
		dwRet = 0x03 ;
		goto Cleanup ;
	}*/

	if( RegQueryValueEx(h_iSpyAV, TEXT("VibraniumUserInfo"), 0, &dwRegType, (LPBYTE)&m_ActInfo, 
						&dwRetSize) != ERROR_SUCCESS )
	{
		dwRet = GetLastError() ;
		dwRet = 0x03 ;
		goto Cleanup ;
	}

	if( DecryptData( (LPBYTE )&m_ActInfo, sizeof(m_ActInfo)) )
	{
		dwRet = 0x04 ;
		goto Cleanup ;
	}

	//Match here the machine ID with Stored machineID
	if (theApp.m_dwProductID != m_ActInfo.dwProductNo)
	{
		dwRet = 0x05;
		return dwRet;
	}

	//Match here the machine ID with Stored machineID
	//if someone provides the DB files from other computer then it works so 
	//necessary to check for machine ID.
	if (!CheckForMachineID(m_ActInfo))
	{
		memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
		dwRet = 0x06;
		return dwRet;
	}

	//Issue No 1243. This code is commented as thr no need to check title. In case of reinstallation/Uninstallation in different languges
	//DB and Registration contain data when user register. So it mismatched. 
	/*if( _tcscmp(m_ActInfo.szTitle,  theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MR")) != 0	&&
		_tcscmp(m_ActInfo.szTitle, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MRS")) != 0 &&
		_tcscmp(m_ActInfo.szTitle, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MS")) != 0)
	{
		dwRet = 0x5;
		goto Cleanup;
	}*/

Cleanup:

	if( h_iSpyAV_User )
		RegCloseKey( h_iSpyAV_User ) ;
	if( h_iSpyAV )
		RegCloseKey( h_iSpyAV ) ;

	h_iSpyAV_User = h_iSpyAV = NULL ;

	return dwRet ;
}

DWORD CRegistrationSecondDlg::AddRegistrationDataInRegistry()
{
	DWORD	dwRet = 0x00 ;
	DWORD	dwRegType = 0x00, dwRetSize = 0x00 ;

	HKEY	h_iSpyAV = NULL ;
	HKEY	h_iSpyAV_User = NULL ;

	AVACTIVATIONINFO	ActInfo = {0} ;

	memcpy(&ActInfo, &m_ActInfo, sizeof(ActInfo) ) ;

	if( DecryptData( (LPBYTE )&ActInfo, sizeof(ActInfo)) )
	{
		dwRet = 0x04 ;
		goto Cleanup ;
	}

	if(!SendRegistryData2Service(SZ_BINARY, L"SOFTWARE\\Microsoft\\Windows", L"VibraniumUserInfo", (LPBYTE)&ActInfo, sizeof(ActInfo), false))
	{
		dwRet = 0x05;
		AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::AddRegistrationDataInRegistry", 0, 0, true, SECONDLEVEL);
		goto Cleanup ;
	}

	//if( RegCreateKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Vibranium"), 
	//					0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
	//					NULL, &h_iSpyAV, NULL ) != ERROR_SUCCESS )
	//{
	//	dwRet = 0x01 ;
	//	goto Cleanup ;
	//}

/*	if( RegCreateKeyEx(	h_iSpyAV, TEXT("UserInfo"), 
						0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
						NULL, &h_iSpyAV_User, NULL ) != ERROR_SUCCESS )
	{
		dwRet = 0x02 ;
		goto Cleanup ;
	}
*/

	//if( RegSetValueEx(h_iSpyAV, TEXT("RegInfo"), 0, REG_BINARY, (LPBYTE)&ActInfo, 
	//					sizeof(ActInfo) ) != ERROR_SUCCESS )
	//{
	//	dwRet = 0x03 ;
	//	goto Cleanup ;
	//}

Cleanup:

	//if( h_iSpyAV_User )
	//	RegCloseKey( h_iSpyAV_User ) ;
	//if( h_iSpyAV )
	//	RegCloseKey( h_iSpyAV ) ;

	//h_iSpyAV_User = h_iSpyAV = NULL ;

	return dwRet ;
}

DWORD CRegistrationSecondDlg::GetRegistrationDatafromFile( )
{

	DWORD	dwRet = 0x01 ;

	CString	strUserRegFile = GetModuleFilePath() ;
	strUserRegFile = strUserRegFile + L"VBUSERREG.DB" ;
	dwRet = GetRegistrationDatafromFile( strUserRegFile ) ;
	if( !dwRet )
		return dwRet ;

	TCHAR	szAllUserPath[512] = {0} ;

	TCHAR	szSource[512] = {0} ;
	TCHAR	szSource1[512] = {0} ;
	TCHAR	szDestin[512] = {0} ;
	TCHAR	szDestin1[512] = {0} ;

	OSVERSIONINFO 	OSVer = {0} ;

	GetEnvironmentVariable( L"ALLUSERSPROFILE", szAllUserPath, 511 ) ;
	OSVer.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
	GetVersionEx( &OSVer ) ;
	if( OSVer.dwMajorVersion > 5 )
		wsprintf( szDestin, L"%s\\Wardwiz Antivirus", szAllUserPath ) ;
	else
		wsprintf( szDestin, L"%s\\Application Data\\Wardwiz Antivirus", szAllUserPath ) ;


	wcscpy( szDestin1, szDestin ) ;
	//wcscat( szDestin, L"\\VBEVALREG.DLL" ) ;
	wcscat( szDestin1, L"\\VBUSERREG.DB") ;

/*	dwRet = 0x01 ;
	dwRet = GetRegistrationDatafromFile( szDestin ) ;
	if( !dwRet )
		return dwRet ;
*/
	dwRet = 0x01 ;
	dwRet = GetRegistrationDatafromFile( szDestin1 ) ;
	if( !dwRet )
		return dwRet ;


	TCHAR	szDrives[256] = {0} ;
	GetLogicalDriveStrings( 255, szDrives ) ;

	TCHAR	*pDrive = szDrives ;

	while( wcslen(pDrive) > 2 )
	{
		dwRet = 0x01 ;
		//memset(szDestin, 0x00, 512*sizeof(TCHAR) ) ;
		//wsprintf( szDestin, L"%sVBEVALREG.DLL",	pDrive ) ;

		memset(szDestin1, 0x00, 512*sizeof(TCHAR) ) ;
		wsprintf( szDestin1, L"%sVBUSERREG.DB",	pDrive ) ;

		if( ( GetDriveType(pDrive) & DRIVE_FIXED ) == DRIVE_FIXED )
		{
			//dwRet = GetRegistrationDatafromFile( szDestin ) ;
			//if( !dwRet )
			//	return dwRet ;

			dwRet = GetRegistrationDatafromFile( szDestin1 ) ;
			if( !dwRet )
				return dwRet ;

		}

		pDrive += 4 ;
	}

	return 0x01 ;
}

DWORD CRegistrationSecondDlg::GetRegistrationDatafromFile( CString strUserRegFile )
{
	//CString	strUserRegFile = GetModuleFilePath() ;

	//strUserRegFile = strUserRegFile + L"\\VBUSERREG.DB" ;

	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	DWORD	dwRet = 0x00, dwBytesRead=0x00 ;

	DWORD	dwSize = sizeof(m_ActInfo) ;

	hFile = CreateFile(strUserRegFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( hFile == INVALID_HANDLE_VALUE )
	{
		dwRet = 0x01 ;
		goto Cleanup ;
	}

	ZeroMemory(&m_ActInfo, sizeof(m_ActInfo) ) ;
	ReadFile( hFile, &m_ActInfo, dwSize, &dwBytesRead, NULL ) ;
	if( dwSize != dwBytesRead )
	{
		dwRet = 0x02 ;
		goto Cleanup ;
	}

	if( DecryptData( (LPBYTE )&m_ActInfo, dwSize ) )
	{
		dwRet = 0x04 ;
		goto Cleanup ;
	}

	//Match here the machine ID with Stored machineID
	if (theApp.m_dwProductID != m_ActInfo.dwProductNo)
	{
		dwRet = 0x05;
		return dwRet;
	}

	//if someone provides the DB files from other computer then it works so 
	//necessary to check for machine ID.
	if (!CheckForMachineID(m_ActInfo))
	{
		memset(&m_ActInfo, 0x00, sizeof(m_ActInfo));
		dwRet = 0x06;
		return dwRet;
	}
Cleanup:

	if( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile ) ;
	hFile = INVALID_HANDLE_VALUE ;


	return dwRet ;
}

DWORD CRegistrationSecondDlg::AddRegistrationDataInFile( )
{
	DWORD	dwRet = 0x00;

	if(!SendRegisteredData2Service(ADD_REGISTRATION_DATAINFILE, (LPBYTE)&m_ActInfo, sizeof(m_ActInfo), 0, L"" , false))
	{
		AddLogEntry(L"### Failed to SendRegisteredData2Service in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		dwRet = 0x01 ;
	}

	Sleep(30);

	SpreadRegistrationFilesInSystem() ;

	return dwRet;
}

DWORD CRegistrationSecondDlg::GetRegistrationDateFromServer( LPSYSTEMTIME lpServerTime, LPDWORD lpDaysRemaining )
{
	DWORD	dwRet = 0x01 ;

	TCHAR	szUserInfo[512] = {0} ;
	TCHAR	szResponse[512] = {0} ;
	
	TCHAR	szDomainName[MAX_PATH] = {0} ;
	if(!GetDomainName(szDomainName, MAX_PATH))
	{
		AddLogEntry(L"### Failed to get GetDomainName in CRegistrationSecondDlg::GetRegistrationDateFromServer", 0, 0, true, SECONDLEVEL);
		return dwRet ;
	}

	if(!m_bActiveProduct)
	{
//		wsprintf(szUserInfo, L"http://%s/trial.php?FName=%s&LName=%s&Email=%s&RegCode=&ProdID=%d&CountryIP=%s&MachineID=%s", szDomainName, m_ActInfo.szUserFirstName, 
//			m_ActInfo.szUserLastName, m_ActInfo.szEmailID, m_ActInfo.dwProductNo, L"",  m_ActInfo.szClientID ) ;
		wsprintf(szUserInfo, L"http://%s/trial.php?FName=%s&LName=%s&Email=%s&RegCode=&ContactNo=%s&ProdID=%d&MachineID=%s", szDomainName, m_ActInfo.szUserFirstName, 
			m_ActInfo.szUserLastName, m_ActInfo.szEmailID, m_ActInfo.szMobileNo ,m_ActInfo.dwProductNo,  m_ActInfo.szClientID ) ;
	}
	else
	{
//		wsprintf(szUserInfo, L"http://%s/registration.php?FName=%s&LName=%s&Email=%s&RegCode=%s&CountryIP=%s&ProdID=%d&MachineID=%s", szDomainName, m_ActInfo.szUserFirstName,
//			m_ActInfo.szUserLastName, m_ActInfo.szEmailID, m_ActInfo.szKey, m_ActInfo.szRegionCode, m_ActInfo.dwProductNo, m_ActInfo.szClientID ) ;
		wsprintf(szUserInfo, L"http://%s/registration.php?FName=%s&LName=%s&Email=%s&RegCode=%s&ContactNo=%s&ProdID=%d&MachineID=%s", szDomainName, m_ActInfo.szUserFirstName,
			m_ActInfo.szUserLastName, m_ActInfo.szEmailID, m_ActInfo.szKey, m_ActInfo.szMobileNo, m_ActInfo.dwProductNo, m_ActInfo.szClientID ) ;
	}

	AddLogEntry(L">>>> Sending HTTP request", 0, 0, true, FIRSTLEVEL);
	AddLogEntry(szUserInfo);

	WinHttpClient client( szUserInfo ) ;

	//while registration after entering all the required details
	//click Next->popup appears if internet connection is not there as 'no internet connection
	//click 'OK' on that dialog box->there it should show Retry button
	//so that we not need enter all the details again
	//Neha Gharge 10th Aug ,2015
	if(!client.SendHttpRequest())
	{
		while (true)
		{
			//MessageBox(L"Please check internet connection.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
			if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_CHECK_NET"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_RETRYCANCEL) == IDCANCEL)
			{
				return dwRet;
			}
			else
			{
				if (client.SendHttpRequest())
				{
					break;
				}
			}
		}
		
	}

    // The response content.
    wstring httpResponseContent = client.GetResponseContent() ;

	AddLogEntry(L">>>> Getting response", 0, 0, true, FIRSTLEVEL);
	AddLogEntry(httpResponseContent.c_str());

	if( (httpResponseContent.length() > 22) && (httpResponseContent.length() < 512) )
	{

		//Issue : Online registration getting crashed due to response length is more than 512

		wcscpy_s( szResponse, 511, httpResponseContent.c_str() ) ;
		szResponse[wcslen(szResponse)] = '\0';
		//Neha Gharge 13/3/2015 Message Box for error handling message.
		dwRet = ExtractDate(szResponse, lpServerTime, lpDaysRemaining );
		/*switch(dwRet)  
		{
		case MACHINEIDMISMATCH:
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_KEY_USED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			break;
		case INVALIDEMAILID:
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_EMAILID_INVALID"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			break;
		case COUNTRYCODEINVALID:
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_COUNTRYID_INVALID"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			break;
		case INVALIDREGNUMBER:
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_REGKEY_INVAID"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			break;
		case INVALIDPRODVERSION:
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_PRODID_INVAID"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			break;
		case PRODUCTEXPIRED:
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_PROD_EXPIRED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			break;
		}*/

	}
	else
		dwRet = NULLRESPONSE;

	return dwRet ;
}

CString CRegistrationSecondDlg::GetModuleFilePath()
{
	HKEY	hSubKey = NULL ;
	TCHAR	szModulePath[MAX_PATH] = {0};

	if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey, &hSubKey) != ERROR_SUCCESS)
		return L"";

	DWORD	dwSize = 511 ;
	DWORD	dwType = 0x00 ;

	RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if(_tcslen(szModulePath) > 0)
	{
		return CString(szModulePath) ;
	}
	return L"";
}

DWORD CRegistrationSecondDlg::DecryptData( LPBYTE lpBuffer, DWORD dwSize )
{
	if( IsBadWritePtr( lpBuffer, dwSize ) )
		return 1 ;

	DWORD	iIndex = 0 ;
	DWORD jIndex = 0;

	if (lpBuffer == NULL || dwSize == 0x00)
	{
		return 1;
	}

	for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
	{
		if(lpBuffer[iIndex] != 0)
		{
			if((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
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

	//for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
	//{
	//	if(lpBuffer[iIndex] != 0)
	//	{
	//		lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex++] + WRDWIZ_KEYSIZE);
	//		if (jIndex == WRDWIZ_KEYSIZE)
	//		{
	//			jIndex = 0x00;
	//		}
	//		if (iIndex >= dwSize)
	//		{
	//			break;
	//		}
	//	}
	//}	
	//DWORD	i = 0 ;
	//DWORD dwEncKey = WRDWIZ_KEY;

	//if (lpBuffer == NULL || dwSize == 0x00)
	//{
	//	return 1;
	//}

	//for (i = 0x00;  i < dwSize; i+= 4)
	//{
	//	if(*((DWORD *)&lpBuffer[i]) != 0x0)
	//	{
	//		dwEncKey += LOWORD(dwEncKey);
	//		*((DWORD *)&lpBuffer[i]) = *((DWORD *)&lpBuffer[i]) ^ dwEncKey ;
	//	}
	//}
	return 0;
}

DWORD CRegistrationSecondDlg::ExtractDate(TCHAR *pTime, LPSYSTEMTIME lpServerTime, LPDWORD lpdwServerDays )
{
	__try
	{
		SYSTEMTIME	ServerTime = {0} ;
		TCHAR	szTemp[8] = {0} ;
		TCHAR	*chSep = L"#" ;
		TCHAR	pszTime[64] = {0} ;
		TCHAR	szTempResponse[100] = { 0 };

		
		_tcscpy_s(szTempResponse, _countof(szTempResponse), pTime);
		TCHAR	*pDateTime = wcstok(pTime, chSep ) ;
		if( !pDateTime )
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x01 ;
		}

		TCHAR	*pDays = wcstok(NULL, chSep ) ;
		TCHAR	*pResponseCode = wcstok(NULL, chSep ) ;

		if( !pDays )
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x02 ;
		}

		if( !pResponseCode && wcslen(pResponseCode) != 3)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x03 ;
		}

		DWORD	dwDays = 0x00 ;
		DWORD	dwResponseCode = 0x00 ;

		if( wcslen(pDateTime) > 0x3F )
		{
			AddLogEntry(L"### Invalid Response code from server Time ", 0, 0, true, SECONDLEVEL);
			return 0x14 ;
		}

		wcscpy(pszTime, pDateTime ) ;

		//Checking for if no of days is -ve.
		if(pDays[0] == '-')
		{
			AddLogEntry(L"### No of days is in -ve returning from CRegistrationSecondDlg::ExtractDate ", 0, 0, true, SECONDLEVEL);
			return 0x02 ;
		}

		swscanf(pResponseCode, L"%d", &dwResponseCode ) ;

		if( dwResponseCode == 0x01 )
		{
			AddLogEntry(L"### Registration key already been used", 0, 0, true, SECONDLEVEL);
			return MACHINEIDMISMATCH ;
		}

		if( dwResponseCode == 0x02 )
		{
			AddLogEntry(L"### Email ID for Registration is invalid", 0, 0, true, SECONDLEVEL);
			return INVALIDEMAILID ;
		}

		if( dwResponseCode == 0x03 )
		{
			AddLogEntry(L"### Country code invalid", 0, 0, true, SECONDLEVEL);
			return COUNTRYCODEINVALID ;
		}

		if( dwResponseCode == 0x04 )
		{
			AddLogEntry(L"### Invalid Registration Number", 0, 0, true, SECONDLEVEL);
			return INVALIDREGNUMBER ;
		}

		if( dwResponseCode == 0x05 )
		{
			AddLogEntry(L"### Invalid Vibranium Product key", 0, 0, true, SECONDLEVEL);
			return INVALIDPRODVERSION ;
		}

		//006 - Invalid Agent, Means the request is not came from Wardwiz client.
		//007 - Database connectivity fails.
		if( dwResponseCode == 0x08 )
		{
			AddLogEntry(L"### Failed to update the user information on server, need to resend", 0, 0, true, SECONDLEVEL);
			return USERINFOUPDATEFAILD;
		}

		if (dwResponseCode == 0x09)
		{
			AddLogEntry(L"### FOUND AS PIRACY, product will get un-registered, response: %s", szTempResponse, 0, true, SECONDLEVEL);
			return FOUNDPIRACY;
		}

		swscanf(pDays, L"%d", &dwDays ) ;
		if( !dwDays )
		{
			AddLogEntry(L"### Product Expired, Number of days left %s", pDays, 0, true, SECONDLEVEL);
			return PRODUCTEXPIRED ;
		}
		else
		{
			*lpdwServerDays = dwDays ;
		}

		memcpy(szTemp, pszTime, 2*sizeof(TCHAR) ) ;
		swscanf(szTemp, L"%d", &ServerTime.wDay ) ;
		if( !ServerTime.wDay )
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x12 ;
		}

		memset(szTemp, 0x00, 8*sizeof(TCHAR) ) ;
		memcpy(szTemp, &pszTime[3], 2*sizeof(TCHAR) ) ;
		swscanf(szTemp, L"%d", &ServerTime.wMonth ) ;
		if( !ServerTime.wMonth )
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x13 ;
		}

		DWORD	dwYear = 0x00 ;

		memset(szTemp, 0x00, 8*sizeof(TCHAR) ) ;
		memcpy(szTemp, &pszTime[6], 4*sizeof(TCHAR) ) ;
		swscanf(szTemp, L"%d", &dwYear ) ;
		if( !dwYear )
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x14;
		}

		ServerTime.wYear = (WORD )dwYear ;

		memset(szTemp, 0x00, 8*sizeof(TCHAR) ) ;
		memcpy(szTemp, &pszTime[11], 2*sizeof(TCHAR) ) ;
		swscanf(szTemp, L"%d", &ServerTime.wHour ) ;


		memset(szTemp, 0x00, 8*sizeof(TCHAR) ) ;
		memcpy(szTemp, &pszTime[14], 2*sizeof(TCHAR) ) ;
		swscanf(szTemp, L"%d", &ServerTime.wMinute ) ;


		memset(szTemp, 0x00, 8*sizeof(TCHAR) ) ;
		memcpy(szTemp, &pszTime[17], 2*sizeof(TCHAR) ) ;
		swscanf(szTemp, L"%d", &ServerTime.wSecond ) ;

		*lpServerTime = ServerTime ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::ExtractDate");
	}
	return 0 ;
}

void CRegistrationSecondDlg::ShowHideSecondDlg(bool bEnable)
{
	m_stInternetReqText.ShowWindow(SW_HIDE);
	m_stActiveProduct.ShowWindow(bEnable);
	m_btnTryProduct.ShowWindow(bEnable);
	m_btnActiveProduct.ShowWindow(bEnable);
	m_stClickActiveText.ShowWindow(bEnable);
	m_edtRegistrationkey.ShowWindow(bEnable);
	m_stexample.ShowWindow(bEnable);
	m_stTryProdText.ShowWindow(bEnable);
	m_stClickTryText.ShowWindow(bEnable);
	m_btnBack.ShowWindow(bEnable);
	m_btnNext.ShowWindow(bEnable);
	m_btnCancel.ShowWindow(bEnable);
	
	
	if (theApp.m_dwIsOffline)
	{
		m_btnOnlineActivation.ShowWindow(bEnable);
		//m_btnOnlineActivation.SetCheck(true);
		//m_btnOfflineActivation.SetCheck(false);
		m_stOnlineActivation.ShowWindow(bEnable);
		m_btnOfflineActivation.ShowWindow(bEnable);
		//m_btnOfflineActivation.SetCheck(!bEnable);
		m_stOfflineActivation.ShowWindow(bEnable);
	}
	Invalidate();
}

void CRegistrationSecondDlg::ResetVariable()
{
	m_bOnlineActivation = true;
	m_bOfflineActivation = false;
	ResetEvent(m_hOfflineNextEvent);
}
void CRegistrationSecondDlg::ShutdownThread()
{
	if(theApp.m_hRegMutexHandle) 
	{
		CloseHandle(theApp.m_hRegMutexHandle);
		theApp.m_hRegMutexHandle = NULL;
	}
	if(m_hSendRequestThread !=NULL)
	{
		::SuspendThread(m_hSendRequestThread);
		::TerminateThread(m_hSendRequestThread,0);
		m_hSendRequestThread =NULL;
		ResetEvent(m_hOfflineNextEvent);
	}
}

LRESULT CRegistrationSecondDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

BOOL CRegistrationSecondDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(!pWnd)
		return FALSE;
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();

	if( 
		iCtrlID == IDC_BUTTON_BACK	||
		iCtrlID == IDC_BUTTON_NEXT	||
		iCtrlID == IDC_BUTTON_CANCEl     ||
		iCtrlID == IDC_RADIO_ACTIVEPRODUCT||
		iCtrlID == IDC_RADIO_TRY_PRODUCT ||
		iCtrlID == IDC_RADIO_ONLINE_ACTIVATION ||
		iCtrlID == IDC_RADIO_OFFLINE_ACTIVATION
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

BOOL CRegistrationSecondDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);

}

bool CRegistrationSecondDlg::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPBYTE byData, DWORD dwLen, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = dwType; 
	//szPipeData.hHey = hKey;
	wcscpy_s(szPipeData.szFirstParam, szKey);
	wcscpy_s(szPipeData.szSecondParam, szValue);
	memcpy(szPipeData.byData, byData, dwLen);
	szPipeData.dwSecondValue = dwLen; 

	CISpyCommunicator objCom(SERVICE_SERVER, true,3);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if(szPipeData.dwValue != 1)
		{
			return false;
		}
	}
	return true;
}

bool CRegistrationSecondDlg::SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName,bool bRegWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = dwType;
	memcpy ( szPipeData.byData, lpResBuffer, dwResSize);
	szPipeData.dwValue = dwResSize;
	szPipeData.dwSecondValue = dwResType;
	wcscpy_s(szPipeData.szFirstParam, pResName);
	
	CISpyCommunicator objCom(SERVICE_SERVER, true,3);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bRegWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if(szPipeData.dwValue != 1)
		{
			return false;
		}
	}

	return true;
}

bool CRegistrationSecondDlg::GetGlobalIPAddrFromUrl(LPTSTR szResponse)
{
	try
	{
		TCHAR	szUserInfo[512] = {0} ;
		wsprintf(szUserInfo, L"http://ipecho.net/plain") ;

		WinHttpClient client( szUserInfo ) ;
		if(!client.SendHttpRequest())
		{
			AddLogEntry(L"### SendHttpRequest %s failed in CRegistrationSecondDlg::GetGlobalIPAddrFromUrl", szUserInfo, 0, true, SECONDLEVEL);
			return false;
		}
	
		// The response header.
		wstring httpResponseContent = client.GetResponseContent() ;
		wcscpy( szResponse, httpResponseContent.c_str() ) ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::GetGlobalIPAddrFromUrl", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

DWORD CRegistrationSecondDlg::SpreadRegistrationFilesInSystem( )
{
	TCHAR	szAllUserPath[512] = {0} ;

	TCHAR	szSource[512] = {0} ;
	TCHAR	szSource1[512] = {0} ;
	TCHAR	szDestin[512] = {0} ;
	TCHAR	szDestin1[512] = {0} ;

	OSVERSIONINFO 	OSVer = {0} ;

	GetEnvironmentVariable( L"ALLUSERSPROFILE", szAllUserPath, 511 ) ;

	OSVer.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;

	GetVersionEx( &OSVer ) ;

	if( OSVer.dwMajorVersion > 5 )
	{
		wsprintf( szDestin, L"%s\\Vibranium", szAllUserPath ) ;
	}
	else
	{
		wsprintf( szDestin, L"%s\\Application Data\\Vibranium", szAllUserPath ) ;
	}

	TCHAR szModulePath[MAX_PATH] = {0};
	_tcscpy( szModulePath, GetWardWizPathFromRegistry());
	//GetModuleFileName(NULL, szModulePath, MAX_PATH);
	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	wcscpy( szDestin1, szDestin ) ;

	////wsprintf( szSource, L"%s\\VBEVALREG.DLL",	szModulePath ) ;
	//wcscat( szDestin, L"\\VBEVALREG.DLL" ) ;
	wsprintf(szSource1, L"%s\\VBUSERREG.DB",	szModulePath ) ;
	//CopyFileToDestination( szSource, szDestin ) ;

	wcscat( szDestin1, L"\\VBUSERREG.DB") ;

	CopyFileToDestination( szSource1, szDestin1 ) ;

	TCHAR	szDrives[256] = {0} ;
	GetLogicalDriveStrings( 255, szDrives ) ;

	TCHAR	*pDrive = szDrives ;

	while( wcslen(pDrive) > 2 )
	{
		memset(szDestin, 0x00, 512*sizeof(TCHAR) ) ;
		wsprintf( szDestin, L"%sVBEVALREG.DLL",	pDrive ) ;

		memset(szDestin1, 0x00, 512*sizeof(TCHAR) ) ;
		wsprintf( szDestin1, L"%sVBUSERREG.DB",	pDrive ) ;

		if( ( GetDriveType(pDrive) & DRIVE_FIXED ) == DRIVE_FIXED )
		{
			//CopyFileToDestination( szSource, szDestin ) ;
			CopyFileToDestination( szSource1, szDestin1 ) ;
		}
		pDrive += 4 ;
	}

	return 0 ;
}

bool CRegistrationSecondDlg::CopyFileToDestination( TCHAR *pszSource, TCHAR *pszDest )
{
	try
	{
		// Delete Old File Copy new File and Set attribute to hidden //Implemented inRegistration process to distribute UserRegDB files in system so 5 is used
		if(!CopyRegistrationFilesUsingService(FILE_OPERATIONS, pszSource, pszDest ,5, true))
		{
			AddLogEntry(L"### CopyRegistrationFilesUsingService Failed in CRegistrationSecondDlg::CopyFileToDestination", 0, 0, true, SECONDLEVEL);
			return false;
		}

		Sleep(30);

		//if(!CopyRegistrationFilesUsingService(FILE_OPERATIONS, pszDest,  L"", 4, true))
		//{
		//	AddLogEntry(L"### CopyRegistrationFilesUsingService Failed in CRegistrationSecondDlg::CopyFileToDestination", 0, 0, true, SECONDLEVEL);
		//	return false;
		//}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::CopyFileToDestination", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true ;
}

bool CRegistrationSecondDlg::CopyRegistrationFilesUsingService(DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = dwType;
		szPipeData.dwValue = dwValue;
		wcscpy_s(szPipeData.szFirstParam, csSrcFilePath);
		wcscpy_s(szPipeData.szSecondParam, csDestFilePath);

		CISpyCommunicator objCom(SERVICE_SERVER, true,3);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : CopyRegistrationFilesUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if(bWait)
		{
			if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : CopyRegistrationFilesUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::CopyRegistrationFilesUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
  Function Name  : RefreshString
  Description    : this function is  called for setting the Text UI with different Language Support
  Author Name    : Nitin Kolapkar
  Date           : 29 April 2014
***********************************************************************************************/

BOOL CRegistrationSecondDlg :: RefreshString()
{
	m_stInternetReqText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_INTERNETREQTEXT"));
	m_stInternetReqText.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stInternetReqText.ShowWindow(SW_HIDE);

	m_stActiveProduct.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_ACTIVEPRODUCT"));
	m_stActiveProduct.SetFont(&theApp.m_fontWWTextNormal);

	m_stClickActiveText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_CLICKACTIVETEXT"));
	m_stClickActiveText.SetFont(&theApp.m_fontWWTextNormal);

	m_stOnlineActivation.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_ONLINE_ACTIVATION"));
	m_stOnlineActivation.SetFont(&theApp.m_fontWWTextNormal);
	
	m_stOfflineActivation.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_OFFLINE_ACTIVATION"));
	m_stOfflineActivation.SetFont(&theApp.m_fontWWTextNormal);

	m_stexample.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_EXAMPLE"));
	m_stexample.SetFont(&theApp.m_fontWWTextNormal);
	
	m_stTryProdText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_TRYPRODTEXT"));
	m_stTryProdText.SetFont(&theApp.m_fontWWTextNormal);
		
	m_stClickTryText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_CLICKTRYTEXT"));
	m_stClickTryText.SetFont(&theApp.m_fontWWTextNormal);

	m_btnNext.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_BUTTON_NEXT"));
	m_btnNext.SetFont(&theApp.m_fontWWTextNormal);

	m_btnBack.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_BUTTON_BACK"));
	m_btnBack.SetFont(&theApp.m_fontWWTextNormal);

	m_btnCancel.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_SECPG_BUTTON_CANCEL"));
	m_btnCancel.SetFont(&theApp.m_fontWWTextNormal);
	return true;
}

bool CRegistrationSecondDlg::SendRegistrationInfo2Service(int iMessageInfo, DWORD dwType,bool bEmailPluginWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = iMessageInfo;
	
	CISpyCommunicator objComUI(UI_SERVER);
	if(!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		if (!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::UI_SERVER");
		}
	}

	CISpyCommunicator objComEmail(EMAILPLUGIN_SERVER);
	if(!objComEmail.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		if (!objComEmail.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::EMAILPLUGIN_SERVER");
		}
	}

	CISpyCommunicator objComTray(TRAY_SERVER);
	if(!objComTray.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		if (!objComTray.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::TRAY_SERVER");
		}
	}

	CISpyCommunicator objComUSB(USB_SERVER);
	if(!objComUSB.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		if (!objComUSB.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CRegistrationSecondDlg::SendRegistrationInfo2Service::USB_SERVER");
		}
	}
	return true;
}

void CRegistrationSecondDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::OnPaint();
}

/***********************************************************************************************
  Function Name  : GetDomainName
  Description    : Function to get wardwiz domain name.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10 Oct 2014
***********************************************************************************************/
bool CRegistrationSecondDlg::GetDomainName(LPTSTR pszDomainName, DWORD dwSize)
{
	bool bReturn = false;
	TCHAR	szRegDomain[100] = { 0 };
	CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\UpdateServers.ini";
	GetPrivateProfileString(L"REGSERVER", L"regdomain", L"act.vibraniumav.in", szRegDomain, 100, csIniFilePath);
	_tcscpy_s(pszDomainName, dwSize, szRegDomain);
	bReturn = true;
	return bReturn;
}

/***********************************************************************************************
  Function Name  : SendUserInformation2Server
  Description    : Function to send user information to server.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10 Oct 2014
***********************************************************************************************/
DWORD CRegistrationSecondDlg::SendUserInformation2Server(int iInfoType)
{
	//bool bReturn = false;
	DWORD  dwRet = 0x00;
	SYSTEMTIME szServerTime = { 0 };

	if(GetRegisteredUserInfo(theApp.m_dwProductID) != 0x00)
	{
		AddLogEntry(_T("### Failed to GetRegisteredUserInfo In CRegistrationSecondDlg::SendUserInformation2Server"));
		dwRet = 0x13;//Failed to get registred user information.
		goto FAILED;
	}

	DWORD dwDays = 0;

	DWORD dwUpdateRes = SendHTTPUpdateRequest(iInfoType, &szServerTime, &dwDays);
	dwRet = dwUpdateRes;
	//Machine ID if mismatch that condition we will check in next release for this release only two conditions are consider. From offline to online condition.
	//13 Dec,2015
	if (dwUpdateRes == PRODUCTEXPIRED || dwUpdateRes == FOUNDPIRACY)
	{
		if(!UpdateUserInformationInLocalFiles(&dwDays))
		{
			AddLogEntry(L"### Failed to UpdateUserInformationInLocalFiles in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
			dwRet = 0x14; //Update in file and registry failed.
			goto FAILED;
		}
		//bReturn = true;
	}
FAILED:
	return dwRet;
}

void CRegistrationSecondDlg::OnBnClickedRadioOnlineActivation()
{
	m_bOnlineActivation = true;
	m_bOfflineActivation = false;
	m_bTryProduct = false;
	m_bActiveProduct = true;
	m_btnOfflineActivation.SetCheck(FALSE);
	m_edtRegistrationkey.EnableWindow(TRUE);
}

void CRegistrationSecondDlg::OnBnClickedRadioOfflineActivation()
{
	m_bOnlineActivation = false;
	m_bOfflineActivation = true;
	m_bTryProduct = false;
	m_bActiveProduct = true;
	m_btnOnlineActivation.SetCheck(FALSE);
	m_edtRegistrationkey.EnableWindow(TRUE);
}


/**********************************************************************************************************                     
* Function Name      : CheckForMIDInRegistry
* Description        : Check Machine ID is present in registry or not
* Author Name		 : Neha Gharge	                                                                                           
* Date Of Creation   : 10th Oct 2014
* SR_No              : 
***********************************************************************************************************/
bool CRegistrationSecondDlg::CheckForMIDInRegistry()
{
	CITinRegWrapper objReg;
	TCHAR szMachineIDValue[0x80] = {0};
	DWORD dwSize = sizeof(szMachineIDValue);
	
	if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"MVersion", szMachineIDValue, dwSize) != 0)
	{
		AddLogEntry(L"### Error in GetRegistryValueData CRegistrationSecondDlg::CheckForMIDInRegistry", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(!szMachineIDValue[0])
	{
		return false;
	}

	_tcscpy(m_szMachineIDValue , L"");
	_tcscpy(m_szMachineIDValue , szMachineIDValue);
	return true;
}

/**********************************************************************************************************                     
* Function Name      : LoadWrdWizOfflineRegDll()
* Description        : Loading WrdWizOfflineReg Dll from installation folder . and getprocaddr of some function.
* Author Name		 : Neha Gharge	                                                                                           
* Date Of Creation   : 10th Oct 2014
* SR_No              : 
***********************************************************************************************************/
bool CRegistrationSecondDlg::LoadWrdWizOfflineRegDll()
{
	CString	csAppPath = GetModuleFilePath() ;

	CString csDllPath = L"";

	csDllPath.Format( L"%sVBOFFLINEREG.DLL", csAppPath ) ;

	if(!PathFileExists(csDllPath))
	{
		return false;
	}

	if(!m_hOfflineDLL)
	{
		m_hOfflineDLL = LoadLibrary(csDllPath);
		if( !m_hOfflineDLL )
		{
			AddLogEntry(L"Locating error in VBOFFLINEREG.DLL",0,0,true,SECONDLEVEL);
			return false;
		}
	}

	/*g_GetInstallationCode = ( GETINSTALLATIONCODE ) GetProcAddress(m_hOfflineDLL, "GetInstallationCode") ;
	if(!g_GetInstallationCode)
		return false;

	g_ValidateResponse = ( VALIDATERESPONSE ) GetProcAddress(m_hOfflineDLL, "ValidateResponse") ;
	if(!g_ValidateResponse)
		return false;*/

	//testing code
	//GetActivationCode = ( GETACTIVATIONCODE ) GetProcAddress(m_hOfflineDLL, "GetActivationCode") ;
	//if(!GetActivationCode)
	//	return false;

	return true;
}


/***************************************************************************************************                    
*  Function Name  : ReadHostFileForWWRedirection()
*  Description    : Checks hosts file for local redirection. if found, returns true else false.
*  Author Name    : Vilas
*  Date			  :	29-Sept-2014
*  Modified Date  :	30-Sept-2014
****************************************************************************************************/
bool CRegistrationSecondDlg::ReadHostFileForWWRedirection()
{
	bool bRedirection = false;

	FILE *pFileStream = NULL;

	long lFileSize = 0x00, lReadSize = 0x00;

	TCHAR szHostPath[512] = {0};
	TCHAR szFileLine[1024] = {0};

	try
	{
		GetSystemDirectory(szHostPath, 511 );
		wcscat_s(szHostPath, L"\\Drivers\\etc\\hosts");

		pFileStream = _wfsopen(szHostPath, _T("r"), _SH_DENYNO);
		if( !pFileStream )
			goto Cleanup;

		fseek( pFileStream, 0L, SEEK_END);
		lFileSize = ftell( pFileStream );

		if( !lFileSize )
			goto Cleanup;

		fseek( pFileStream, 0L, SEEK_SET);

		while( true )
		{
			if( fgetws(szFileLine, 1023, pFileStream ) == NULL )
				break;

			if( !wcslen(szFileLine) )
				break;

			if( (StrStrI(szFileLine, L"wardwiz." )) || (StrStrI(szFileLine, L"162.144.82.101" )) )
			{
				bRedirection = true;
				break;
			}

			if( feof(pFileStream) )
				break;

			lReadSize += static_cast<long>(wcslen(szFileLine));
			if( lReadSize >= lFileSize )
				break;

			ZeroMemory(szFileLine, sizeof(szFileLine) );
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in ReadHostFileForWWRedirection", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( pFileStream )
		fclose(pFileStream);

	pFileStream = NULL;

	return bRedirection;
}

/***********************************************************************************************
  Function Name  : SendHTTPUpdateRequest
  Description    : Function to Send request to server using HTTP connection
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10 Oct 2014
***********************************************************************************************/
DWORD CRegistrationSecondDlg::SendHTTPUpdateRequest(int iInfoType, LPSYSTEMTIME lpServerTime, LPDWORD lpDaysRemaining)
{
	DWORD dwRet = 0x00;

	try
	{

		TCHAR	szUserInfo[1024] = {0} ;
		TCHAR	szResponse[512] = {0} ;
		DWORD	dwResponseLen = 0x00;

		TCHAR	szDomainName[MAX_PATH] = {0} ;
		if(!GetDomainName(szDomainName, MAX_PATH))
		{
			AddLogEntry(L"### Failed to get GetDomainName in CRegistrationSecondDlg::GetRegistrationDateFromServer", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
			goto CleanUP;
		}

		const TCHAR * szPhpUpdateFileName = NULL;
#ifdef RELEASELOCAL
		szPhpUpdateFileName = UPDATEUSERINFOLOCALPAGE;
#else
		szPhpUpdateFileName = UPDATEUSERINFOPAGE;
#endif
		/*
			A = RegCode, 
			B = MachineID, 
			C = UUID, 
			D = Name,
			E = Email, 
			F = ContactNo, 
			G = Country, 
			H = State, 
			I = City, 
			J = PinCode, 
			K = ProdID, 
			L = OSName,
			M = HDSize,
			N = Processor, 
			O = CompName, 
			P = UserName, 
			Q = ProductType, 
			R = DealerCode, 
			S = RefferalCode, 
			T = EngineerName, 
			U = EngineerMobNo
		*/

		if (iInfoType == CHECK_PIRACY)
		{
			wsprintf(szUserInfo,
				L"http://%s/ProdPiracy.aspx?A=%s&B=%s&C=%s&D=%d",
				szDomainName,
				m_ActInfo.szKey,
				m_ActInfo.szClientID,
				m_ActInfo.szInstID,
				m_ActInfo.dwProductNo
				);
		}
		else
		{
			wsprintf(szUserInfo,
				L"http://%s/DUP.aspx?A=%s&B=%s&C=%s&D=%s&E=%s&F=%s&G=%s&H=%s&I=%s&J=%s&K=%d&L=%d%&M=%s&N=%s&O=%s&P=%s&Q=%s&R=%s",
				szDomainName,
				m_ActInfo.szKey,
				m_ActInfo.szClientID,
				m_ActInfo.szInstID,
				m_ActInfo.szUserFirstName,
				m_ActInfo.szUserLastName,
				m_ActInfo.szEmailID,
				m_ActInfo.szMobileNo,
				CWWizSettingsWrapper::GetRegisteredCountry(),
				CWWizSettingsWrapper::GetRegisteredState(),
				CWWizSettingsWrapper::GetRegisteredCity(),
				CWWizSettingsWrapper::GetRegisteredPinCode(),
				m_ActInfo.dwProductNo,
				CWWizSettingsWrapper::GetCurrentOSName(),
				CWWizSettingsWrapper::GetCurrentHardDiskSize(),
				CWWizSettingsWrapper::GetCurrentProcessorSize(),
				CWWizSettingsWrapper::GetCurrentComputerName(),
				CWWizSettingsWrapper::GetCurrentUserName(),
				CWWizSettingsWrapper::GetDealerCode()
				);
		}
		AddLogEntry(L">>>> Sending Update request : %s", szUserInfo, 0, true, SECONDLEVEL);

		WinHttpClient client( szUserInfo ) ;

		if(!client.SendHttpRequest())
		{
			AddLogEntry(L"### Failed SendHttpRequest in CRegistrationSecondDlg::SendHTTPUpdateRequest");
			dwRet = 0x02;
			goto CleanUP ;
		}

		// The response content.
		wstring httpResponseContent = client.GetResponseContent() ;	
		
		if (httpResponseContent.length() <= 511)
		{
			wcscpy_s(szResponse, 511, httpResponseContent.c_str());
			szResponse[wcslen(szResponse)] = '\0';
			AddLogEntry(L">>> Response : %s", szResponse, 0, true, SECONDLEVEL);
		}
		if( httpResponseContent.length() <= 22 )
		{
			AddLogEntry(L"### Invalid Length in SendUser Information Response");
			dwRet = 0x03;
			goto CleanUP ;
		}

	if( dwResponseLen > 0x0FFF )
		{
			AddLogEntry(L"### Length in SendUser Information Response is too high");
			dwRet = 0x04;
			goto CleanUP ;
		}

		//MultiByteToWideChar( CP_ACP, 0, httpResponseContent.c_str() , -1, szResponse, dwResponseLen ) ;
		//const wchar_t	*pszResponse = httpResponseContent.c_str();
		//wcscpy( szResponse, httpResponseContent.c_str() ) ;
		//szResponse[dwResponseLen] = '\0';
	if (httpResponseContent.length() <= 511)
	{
		wcscpy_s(szResponse, 511, httpResponseContent.c_str());
		szResponse[wcslen(szResponse)] = '\0';
		dwRet = ExtractDate(szResponse, lpServerTime, lpDaysRemaining);
	}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::SendHTTPUpdateRequest");
		goto CleanUP;
	}
CleanUP:
	return dwRet; 
}

/***********************************************************************************************
  Function Name  : UpdateUserInformationInLocalFiles
  Description    : Function to update servertime & Number of days, if any wardwiz applicatio is running 
				   Number of days left will get reflected immediately.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10 Oct 2014
***********************************************************************************************/
bool CRegistrationSecondDlg::UpdateUserInformationInLocalFiles(LPDWORD lpDaysRemaining)
{
	bool bReturn = false;

	if(!lpDaysRemaining)
	{
		return false;
	}

	m_ActInfo.dwTotalDays = *lpDaysRemaining ;

	if(!SendRegisteredData2Service(ADD_REGISTRATION_DATA, (LPBYTE)&m_ActInfo, sizeof(m_ActInfo), IDR_REGDATA, TEXT("REGDATA"),false))
	{
		AddLogEntry(L"### Failed to SendRegisteredData2Service in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		goto FAILED;
	}

	Sleep(1000);

	if( AddRegistrationDataInRegistry( ) )
	{
		AddLogEntry(L"### AddRegistrationDataInRegistry in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
		goto FAILED;
	}

	if( AddRegistrationDataInFile( ) )
	{
		AddLogEntry(L"### AddRegistrationDataInFile failed in CRegistrationSecondDlg SendRequestThread", 0, 0, true, SECONDLEVEL);
	}

	//Send Days left to each running application
	if(!SendRegistrationInfo2Service(RELOAD_REGISTARTION_DAYS))
	{
		AddLogEntry(L"### Error in CRegistrationSecondDlg SendRequestThread::SendRegistrationInfo2Service", 0, 0, true, SECONDLEVEL);
	}

	bReturn = true;

FAILED:
	return bReturn;
}


void CRegistrationSecondDlg::DisplayFailureMessage(REGFAILUREMSGS dwRes)
{
	CString csMessage;
	UINT iType = MB_ICONEXCLAMATION | MB_OK;
	switch(dwRes)
	{
	case 0x01:
		return;
	case 0x02:
		return;
	case 0x03:
		iType = MB_ICONEXCLAMATION | MB_OK;
		csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_RES_CODE_INVALID");
		break;
	case 0x04:
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
			switch(theApp.m_dwProductID)
			{
			case ESSENTIAL:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"),L"Essential version");
				break;
			case PRO:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"),L"pro version");
				break;
			case ELITE:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"),L"elite version");
				break;
			case BASIC:
				csMessage.Format(L"%s \n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT1"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_INVALID_PRODUCT2"),L"basic version");
				break;

			default:   
				csMessage.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_INVALID_PROD_KEY"));
				break;
			}
		}
		break;
	case USERINFOUPDATEFAILD:
		//User Information update failed on server side
		return;
	case PRODUCTEXPIRED:
		csMessage.Format(L"%s \n%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_PROD_EXP1"),theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_TRIAL2"));
		break;
	case NULLRESPONSE:
		csMessage.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_NULL_RESPONSE"), theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_CONTACT_SUPPORT"));
		break;
	default:
		return;
	}
	MessageBox(csMessage, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
}

/***********************************************************************************************
Function Name  : DisplayFailureMessageForOfflineRegistration
Description    : Displays detailed error message in case offline registration is failed
Author Name    : Nitin K
SR.NO		   :
Date           : 3rd Dec 2015
***********************************************************************************************/
void CRegistrationSecondDlg::DisplayFailureMessageForOfflineRegistration(REGFAILUREMSGS dwRes)
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

		MessageBox(csMessage, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::DisplayFailureMessageForOfflineRegistration");
	}
	
}