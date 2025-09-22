/**********************************************************************************************************
Program Name          : WardWizUpdate.cpp
Description           : This class contains the functionality for updating product from local folder as well as live update.
It has 2 options	  : 1) Update from internet.
						2) Update from Local Folder.
Author Name			  : Amol Jaware
Date Of Creation      : 15th Jan 2019
Version No			  : 4.1.0.1
***********************************************************************************************************/

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizOSVersion.h"
#include "WardwizLangManager.h"
#include "EnumProcess.h"
#include "WardWizCrashHandler.h"
#include "iTINRegWrapper.h"
#include "ScannerContants.h"
#include "DownloadController.h"

// CWardWizUpdateApp:
// See WardWizUpdate.cpp for the implementation of this class
//

class CWardWizUpdateApp : public CWinApp
{
public:
	CWardWizUpdateApp();
	~CWardWizUpdateApp();
// Overrides
public:
	virtual BOOL InitInstance();

	typedef DWORD(*GETFILEHASH)		(TCHAR *pFilePath, TCHAR *pFileHash);
	typedef DWORD(*UNZIPSINGLEFILE)	(TCHAR *pZipFile, TCHAR *pUnzipFile);

	GETFILEHASH				GetFileHash = NULL;
	UNZIPSINGLEFILE			UnzipSingleFile = NULL;
	CEvent					m_objCompleteEvent;
	HMODULE					m_hResDLL;
	HANDLE					m_hMutex;
	HMODULE					m_hExtractDLL;
	HMODULE					m_hHashDLL;
	CWardwizLangManager		m_objwardwizLangManager;
	DWORD					m_dwProductID;

	// Implementation

	DECLARE_MESSAGE_MAP()

public:
	bool		SingleInstanceCheck();
	bool		GetAppPath();
	bool		LoadDLLs();
	void		CheckScanLevel();
	CString		GetModuleFilePath();

	CITinRegWrapper			m_objReg;
	
	CString					m_AppPath;
	CString					m_csRegKeyPath;
	TCHAR					m_szAVPath[512];
	bool					m_bCmdLineUpdate;
	bool					m_bEPSLiveUpdateNoUI;
	bool					m_bUpdates;
	bool					m_bRequestFromUI;
	SCANLEVEL				m_eScanLevel;
};

//UPDATE STATUS
enum
{
	EMPTYSTRING = 1,
	CHECKING4UPDATES,
	DOWNLOADINGFILES,
	UPDATINGFILES,
	ALREADYDOWNLOADED
};

extern CWardWizUpdateApp theApp;