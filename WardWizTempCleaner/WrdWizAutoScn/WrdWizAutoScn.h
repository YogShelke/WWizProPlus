
// WrdWizAutoScn.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizOSVersion.h"


// CWrdWizAutoScnApp:
// See WrdWizAutoScn.cpp for the implementation of this class
//

class CWrdWizAutoScnApp : public CWinApp
{
public:
	CWrdWizAutoScnApp();
	~CWrdWizAutoScnApp(void);

// Overrides
public:
	virtual BOOL InitInstance();
	HMODULE					m_hResDLL;
	CWardwizLangManager	m_objwardwizLangManager;
	DWORD					m_dwSelectedLangID;
	DWORD					m_dwOSType;
	CWardWizOSversion		m_objOSVersionWrap;
	DWORD					m_dwProdID;
	bool					m_bTmpFilePage;


	CFont               m_fontWWTextMediumSize;             // For bigger font than normal text         
	CFont				m_fontWWTextNormal;					//For normal Text in the GUI
	CFont				m_fontWWTextTitle;					//For Registry Optimizer and Data Encryption
	CFont				m_fontWWTextSmallTitle;				//For all other titles
	CFont				m_fontWWTextSubTitleDescription;	//For the Description given below the titles
	CFont				m_fontWWTextSubTitle;				//For the Sub titles.
	CFont				m_fontWWLogoHeader;//For the logo header
	CFont				m_FontWWStartUpFontTitle;			//For Did you know...?
	CFont				m_FontWWStartUpFontSubTitle;		//For Tip of the Day
	CFont				m_FontWWStartUpTips;				//For Tips in Rich edit Control and other buttons
	CFont				m_FontWWDaysLeftFont;				//For No of Days left (Digits)
	HANDLE				m_hMutex;
	HANDLE				m_hMutexHandleDriverInstallation;

	bool CheckMutexOfDriverInstallation();
	void AddUserAndSystemInfoToLog();
	void AddUserAndSystemInfoToLogSEH();

	void CreateFonts();
	void LoadResourceDLL();
	CString GetModuleFilePath();
	void CreateFontsFor(DWORD OSType, DWORD LanguageType);
	void OnBnClickedButtonHelp();
	bool SingleInstanceCheck();
// Implementation

public:
	CEvent						m_objCompleteEvent;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;

	DECLARE_MESSAGE_MAP()
};

extern CWrdWizAutoScnApp theApp;