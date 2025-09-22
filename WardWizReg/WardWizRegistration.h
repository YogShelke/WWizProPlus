// WardWizRegistration.h : main header file for the WardWizRegistration DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "RegistrationDlg.h"		// main symbols
#include "RegistrationMsgPopupDlg.h"
#include "ProductInformation.h"
#include "WardwizLangManager.h"
#include "iTINRegWrapper.h"
#include "afxmt.h"

class CWardWizRegistrationApp : public CWinApp
{
public:
	CWardWizRegistrationApp();
	~CWardWizRegistrationApp();

// Overrides
public:
	virtual BOOL InitInstance();
	BOOL PerformRegistration();
	BOOL CloseRegistrationWindow();
	DWORD ShowEvalutionExpiredMsg(bool bShowAtStartUp = false);
	BOOL ShowProductInformation();
	BOOL GetRegisteredUserInfo(AVACTIVATIONINFO &ActInfo);
	DWORD GetDaysLeft(DWORD dwProdID);
	DWORD GetNumberOfDaysLeft(CRegistrationDlg *objRegistrationDlg, bool *bUsedDate = NULL );
	DWORD GetDaysLeft_Service() ;
	DWORD AddToEvalRegDll(LPBYTE, DWORD ) ;
	DWORD AddToUserDB(LPBYTE, DWORD ) ;
	DWORD AddRegistrationDataInRegistry(LPBYTE pActInfo, DWORD dwSize) ;
	DWORD SpreadRegistrationFilesInSystem( ) ;
	bool CopyFileToDestination( TCHAR *pszSource, TCHAR *pszDest ) ;
	void LoadResourceDLL();
	CString GetModuleFilePath();
	void CreateFonts();
	void ReadProductVersion4mRegistry();
	bool SingleInstanceCheck();
	void ProductRenewalDetails();
	DWORD SendUserInformation2Server(int iInfoType);
	bool Check4OfflineActivationFlag();
	bool SetActivationFlagAsOnline();
	void CheckScanLevel();
	void CheckIsOffline();
public:
	CRegistrationDlg m_objRegistrationDlg;
	HMODULE				m_hResDLL;
	CWardwizLangManager	m_objwardwizLangManager;
	CFont				m_fontInnerDialogTitle;
	CFont				m_fontText;
	CFont				m_FontNonProtectMsg;
	CFont				m_fontTextNormal;
	CFont				m_fontHyperlink;

	CFont				m_fontWWTextNormal;					//For normal Text in the GUI
	CFont				m_fontWWTextTitle;					//For Registry Optimizer and Data Encryption
	CFont				m_fontWWTextSmallTitle;				//For all other titles
	CFont				m_fontWWTextSubTitleDescription;	//For the Description given below the titles
	CFont				m_fontWWTextSubTitle;				//For the Sub titles.
	DWORD				m_dwProductID;
	CString				m_csProdRegKey;
	CString				m_csRegProductVer;
	CString				m_csDataEncVer;
	HANDLE				m_hRegMutexHandle;
	BOOL				m_bRegEmail;
	bool				m_bAllowDemoEdition;
	CDialog	            *m_objProdInfoPopup;

	bool				m_bIsProductRenewal;
	DWORD				m_dwSalutation;
	CString				m_csEmailID;
	CString				m_csFirstName;
	CString				m_csLastName;
	CString				m_csContactNo;
	CString				m_csProductKeyNumber;

	CITinRegWrapper		m_objReg;
	SCANLEVEL			m_eScanLevel;
	DWORD				m_dwIsOffline;
	DWORD				m_dwLangID;
	
public:
	CEvent						m_objCompleteEvent;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;
	DECLARE_MESSAGE_MAP()
};

extern CWardWizRegistrationApp theApp;
