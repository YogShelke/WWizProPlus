#pragma once

#include "stdafx.h"
#include "WardwizLangManager.h"
#include "ISpyOutlookComm.h"

class COutlookAddinApp : public CWinApp
{
public:
	COutlookAddinApp();
	~COutlookAddinApp();

public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
	DWORD CheckiSPYAVRegistered( );
	void ShowEvaluationExpiredMsg();
	void ReloadNoofdaysLeft();
	bool UnloadRegistrationDLL( );
	bool GetProductSettings( );
public:
	DWORD								m_dwDaysLeft;
	HMODULE								m_hRegistrationDLL ;
	DWORD								m_dwSelectedLangID;
	DWORD								m_dwProductID;
	CWardwizLangManager					m_objwardwizLangManager;
	CISpyOutlookComm					m_objISpyOutlookComm;

	DECLARE_MESSAGE_MAP()
};
extern COutlookAddinApp theApp;
 