/**********************************************************************************************************
Program Name          : WWizUninstThirdDlg.h
Description           :
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 6th Feb 2015
Version No            : 1.9.0.0
Special Logic Used    : 
Modification Log      :           
***********************************************************************************************************/
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizLangManager.h"
#include "ISpyCommunicator.h"
#include "sqlite3.h"

typedef DWORD(*GetByteStringHashFunc)(TCHAR *, int, TCHAR *);

class CWardWizUninstallerApp : public CWinApp
{
public:
	CWardWizUninstallerApp();
	~CWardWizUninstallerApp();

// Overrides
public:
	virtual BOOL InitInstance();
	void LoadResourceDLL();
	CString GetModuleFilePath();
	bool SingleInstanceCheck();
	void LoadReqdLibrary();

public:
	HMODULE					m_hResDLL;
	DWORD					m_dwSelectedLangID;
	DWORD					m_dwProductID;
	CString					m_csProdKeyName;
	CWardwizLangManager		m_objwardwizLangManager;
	HANDLE					m_hMutex;
	CString					m_AppPath;
	DECLARE_MESSAGE_MAP()

public:
	CEvent						m_objCompleteEvent;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;
	HINSTANCE					m_hInstLibrary;
	GetByteStringHashFunc		m_pGetbyteStrHash;
	DWORD CalculateMD5(TCHAR *pString, int iStringlen, TCHAR *pFileHash);
};

extern CWardWizUninstallerApp theApp;