// ISpyAVTray.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizLangManager.h"
#include "WardwizOSVersion.h"
#include "WardWizSplashWindow.h"
#include "AVRegInfo.h"
#include "ScannerContants.h"
#include "iTINRegWrapper.h"
// CISpyAVTrayApp:
// See ISpyAVTray.cpp for the implementation of this class
//
typedef DWORD (*GETDAYSLEFT)		(DWORD) ;
typedef bool (*PERFORMREGISTRATION)	() ;

class CISpyAVTrayApp : public CWinApp
{
public:
	CISpyAVTrayApp();
	~CISpyAVTrayApp();
public:
	void	CreateFonts();
	DWORD	GetDaysLeft();
public:
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
	CFont				m_FontWWRelNoteTitle;				//For the App-DB version title of Release Note Pop-up
	
	DWORD				m_dwSelectedLangID;
	DWORD				m_dwProductID;
	AVACTIVATIONINFO	m_ActInfo;

public:
	virtual BOOL InitInstance();
public:
	CString GetModuleFilePath();
	bool SingleInstanceCheck(CString csUserName);
	void LoadResourceDLL();
	HMODULE				m_hResDLL;
	CWardwizLangManager	m_objwardwizLangManager;
	int m_iPos;
	CWardWizOSversion		m_objOSVersionWrap;
	void CreateFontsFor(DWORD OSType,DWORD LanguageType);
	DWORD CheckiSPYAVRegistered( ) ;
	HMODULE				m_hRegistrationDLL ;
	GETDAYSLEFT			m_GetDaysLeft;
	EXPIRYMSGBOX		m_lpLoadDoregistrationProc;
	HMODULE				m_hRegisteredDataDLL ;
	PERFORMREGISTRATION	m_PerformRegistration;
	DOREGISTRATION		m_lpLoadProductInformation;
	DWORD				m_dwDaysLeft;
	GETREGISTEREFOLDERDDATA m_lpLoadEmail;
	CString				m_AppPath;
	CITinRegWrapper		m_objReg;

	LPTSTR				GetRegisteredEmailID();
	LPTSTR				GetRegisteredUserInformation();
	DWORD				GetRegistrationDatafromFile();
	DWORD				GetRegistrationDataFromRegistry();
	DWORD				GetRegistrationDatafromFile(CString strUserRegFile);
	DWORD				DecryptRegistryData(LPBYTE lpBuffer, DWORD dwSize);
	TCHAR				m_szRegKey[32];

	//For the Sub titles.
	CFont				m_FontWWStartUpFontTitle;			//For Did you know...?
	CFont				m_FontWWStartUpFontSubTitle;		//For Tip of the Day
	CFont				m_FontWWStartUpTips;				//For Tips in Rich edit Control and other buttons
	HANDLE			    m_hMutexHandleDriverInstallation;
	CString				m_csRegKeyPath;
	bool				GetAppPath();
	DWORD				m_dwOsVersion;
	DWORD				m_dwShowTrayPopup;
	//Adding User, computer name and OS details
	//Added by Vilas on 05 May 2015
	void AddUserAndSystemInfoToLog();
	bool CheckMutexOfDriverInstallation();
	bool CheckForMachineID(const AVACTIVATIONINFO	&actInfo);
	bool m_bIsShowRegTrue;
	DECLARE_MESSAGE_MAP()
};

extern CISpyAVTrayApp theApp;