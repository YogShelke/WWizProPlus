/**********************************************************************************************************
Program Name          : WardWiz browser Security Client header file
Author Name			  : Jeena Mariam Saji
Date Of Creation      : 04 Sept 2019
Modification Log      :
***********************************************************************************************************/
#pragma once
#include "stdafx.h"

class CWardWizBrowserSec
{

public:
	CWardWizBrowserSec();
	virtual ~CWardWizBrowserSec();
	bool Initialize();
	bool UnInitialize();
	bool LoadRequiredModules();
	bool ReloadBrowserSecVal();
	bool ReloadBrowserSecExcVal();
	bool ReloadBrowserSecSpecVal();

private:
	HMODULE							m_hModuleBrowserSecDLL;
	WRDWIZBROWSERSECINITILIAZE		m_lpBrowserSecInitialize;
	WRDWIZBROWSERSECUNINITILIAZE	m_lpBrowserSecUninitialize;
	WRDWIZBROWSERSECRELOAD			m_lpBrowserSecReload;
	WRDWIZBROWSERSECEXCRELOAD		m_lpBrowserSecExcReload;
	WRDWIZBROWSERSECSPECRELOAD		m_lpBrowserSecSpecReload;
};

