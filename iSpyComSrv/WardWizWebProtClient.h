#pragma once
#include "stdafx.h"
#include "ISpyCriticalSection.h"

class CWardWizWebProtClient
{
public:
	CWardWizWebProtClient();
	virtual ~CWardWizWebProtClient();
	bool Initialize();
	bool UnInitialize();
	bool LoadRequiredModules();
	bool ReloadWebSecDB();
	bool ReloadBrowseProtDB();
	bool ReloadBlkSpecWebDB();
	bool ReloadMngExcDB();
	bool ClearURLCache();
	char*						m_strDatabaseFilePath = ".\\VBALLREPORTS.DB";
	CISpyCriticalSection	m_objISpyCriticalSection;

private:
	HMODULE								m_hModuleEmailDLL;
	WRDWIZWEBPROTINITIALIZE				m_lpWebProtInitialize;
	WRDWIZWEBPROTUNINITIALIZE			m_lpWebProtUnInitialize;
	WRDWIZWEBPROTRELOADWEBSECDB			m_lpWebProtReloadWebSecDB;
	WRDWIZWEBPROTRELOADBROWSEPROTDB		m_lpWebProtReloadWebSettingDB;
	WRDWIZWEBPROTRELOADBLKSPECWEB		m_lpWebProtReloadSpecBlkWebDB;
	WRDWIZWEBPROTRELOADMNGEXCDB			m_lpManageExclusionDB;
	WRDWIZWEBPROTCLEARCACHE				m_lpClearURLCache;
};

