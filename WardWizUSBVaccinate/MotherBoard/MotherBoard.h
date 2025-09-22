// MotherBoard.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizOSVersion.h"
#include "WardwizLangManager.h"
#include "ColorStatic.h"
#include "xSkinButton.h"
#include "PictureEx.h"
#include "GdipButton.h"
#include "PictureEx.h"
#include "afxmt.h"

// CMotherBoardApp:
// See MotherBoard.cpp for the implementation of this class
//

class CMotherBoardApp : public CWinApp
{
public:
	CMotherBoardApp();
	~CMotherBoardApp(void);
	void CreateFonts();
	void LoadResourceDLL();
	CString GetModuleFilePath();
	void CreateFontsFor(DWORD OSType, DWORD LanguageType);
	void OnBnClickedButtonHelp();

	HMODULE					m_hResDLL;
	CWardWizOSversion		m_objOSVersionWrap;
	DWORD					m_dwOSType;
	DWORD					m_dwSelectedLangID;
	CWardwizLangManager		m_objwardwizLangManager;
	DWORD					m_dwProdID;

	CFont				m_fontInnerDialogTitle;
	CFont				m_fontText;
	CFont				m_FontNonProtectMsg;
	CFont				m_fontTextNormal;
	CFont				m_fontHyperlink;

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
	

	void AddUserAndSystemInfoToLog();
	void AddUserAndSystemInfoToLogSEH();
	bool CheckMutexOfDriverInstallation();
// Overrides
	public:
	virtual BOOL InitInstance();
	bool SingleInstanceCheck();
// Implementation

public:
	CEvent						m_objCompleteEvent;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;

	DECLARE_MESSAGE_MAP()
};

extern CMotherBoardApp theApp;