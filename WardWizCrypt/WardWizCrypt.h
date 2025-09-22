/**********************************************************************************************************
	Program Name			: WardWizCrypt.h
	Description				: Application classs which is derived from CWinApp
	Author Name				: Ramkrushna Shelke
	Date Of Creation		: 5th Jun 2015
	Version No				: 1.11.0.0
	Special Logic Used		: 
	Modification Log		:
***********************************************************************************************************/
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizLangManager.h"
#include "WardwizOSVersion.h"

typedef DWORD(*GETDAYSLEFT)		(DWORD);

class CWardWizCryptApp : public CWinApp
{
public:
	CWardWizCryptApp();
	~CWardWizCryptApp(void);

// Overrides
public:
	virtual BOOL InitInstance();


	//Added a function to check registration and single instance 
	//Neha Gharge 1st July,2015
	bool					m_bIsNoGui;
	CString					m_csaTokenEntryFilePath[7];
	HMODULE					m_hRegistrationDLL;
	GETDAYSLEFT				m_GetDaysLeft;
	DWORD					m_dwDaysLeft;
	DWORD					m_dwProductID;
	CWardwizLangManager		m_objwardwizLangManager;
	HANDLE					m_hMutexHandle;
	CString					m_csModulePath;
	CString					m_csPassword;
	CString					m_csDefaultDataEncVersion;
	HANDLE					m_hMutexHandleDriverInstallation;
	DWORD					m_OSType;

	CString GetModuleFilePath();
	DWORD CheckiSPYAVRegistered();
	bool SingleInstanceCheck();
	bool CheckMutexOfDriverInstallation();
// Implementation
	//void AddUserAndSystemInfoToLog();

	DECLARE_MESSAGE_MAP()
};

extern CWardWizCryptApp theApp;