// USBDetectUI.h : main header file for the PROJECT_NAME application
//

#pragma once

#include "WardwizLangManager.h"
#include "WardWizOSVersion.h"
#include "iTINRegWrapper.h"
#include "WardWizCrashHandler.h"
#include "EnumProcess.h"
#include "afxmt.h"

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

typedef DWORD (*GETDAYSLEFT)		(DWORD) ;
typedef bool (*PERFORMREGISTRATION)	() ;
typedef bool (*DOREGISTRATION) (void);
typedef DWORD(*EXPIRYMSGBOX)(bool);
typedef DWORD(*GetByteStringHashFunc)(TCHAR *, int, TCHAR *);
// CUSBDetectUIApp:
// See USBDetectUI.cpp for the implementation of this class
//

class CUSBDetectUIApp : public CWinApp
{
public:
	CUSBDetectUIApp();
	~CUSBDetectUIApp();
// Overrides
public:
	virtual BOOL InitInstance();
public:
	CWardWizOSversion m_objOSVersionWrap;
	void CreateFonts();
	void CreateFontsFor(DWORD OSType,DWORD LanguageType);
	void LoadReqdLibrary();

public:
	CFont				m_fontInnerDialogTitle;
	CFont				m_fontText;
	CFont				m_FontNonProtectMsg;
	CFont				m_fontTextNormal;
	CFont				m_fontHyperlink;

	CFont				m_fontWWTextNormal;					//For normal Text in the GUI
	CFont				m_fontWWTextStatus;					//For Registry Optimizer and Data Encryption
	CFont				m_fontWWLogoHeader ;//For the logo header
	CFont				m_fontWWTextSubTitle;

// Implementation
	CWardwizLangManager	m_objwardwizLangManager;
	HMODULE				m_hResDLL;
	DWORD				m_dwSelectedLangID;
	DWORD				m_dwProdID;
	CString				m_csAppPath;

	//CWardWizCrashHandler	m_objWardWizCrashHandler;
	bool				m_bAllowDemoEdition;
	CITinRegWrapper		m_objReg;
	SCANLEVEL			m_eScanLevel;

	void LoadResourceDLL();
	CString GetModuleFilePath();
	DWORD CheckiSPYAVRegistered( ) ;
	bool ShowEvaluationExpiredMsg();
	bool ReadDemoEditionEntry();
	DWORD GetDaysLeft();
	static void CrashCallback(LPVOID lpvState);
	void CheckScanLevel();
	bool ShowRestartMsgOnProductUpdate();
	void OnBnClickedButtonHelp();
	bool CheckMutexOfDriverInstallation();
	bool SendData2ComService(int iMessageInfo, DWORD dwType = 0, bool bWait = false);

	HMODULE				m_hRegistrationDLL ;
	GETDAYSLEFT			m_GetDaysLeft;
	EXPIRYMSGBOX		m_lpLoadDoregistrationProc;
	PERFORMREGISTRATION	m_PerformRegistration;
	DWORD				m_dwDaysLeft;
	HANDLE				m_hMutexHandleDriverInstallation;
	CString				m_csRegKeyPath;
	DECLARE_MESSAGE_MAP()

public:
	CEvent						m_objCompleteEvent;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;
	CString						m_csTaskID;
	HINSTANCE					m_hInstLibrary;
	GetByteStringHashFunc		m_pGetbyteStrHash;
	DWORD						CalculateMD5(TCHAR *pString, int iStringlen, TCHAR *pFileHash);
};

extern CUSBDetectUIApp theApp;