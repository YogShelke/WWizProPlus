#pragma once
#include "stdafx.h"
#include "ISpyCriticalSection.h"

class CWardWizEmailClient
{
public:
	CWardWizEmailClient();
	virtual ~CWardWizEmailClient();
	bool Initialize();
	bool UnInitialize();
	bool LoadRequiredModules();
	bool ReloadApplicationRules4FW();
	void ReLoadEmailScanSettingsTab(DWORD dwRegValue);
	void ReLoadParentalControlSettingsTab(DWORD dwRegValue);
	void StartParentalControlCheck();
	void StartPCCompUsageCheck();
	void StartPCInternetUsageCheck();
	void StartPCINetUsageCheck();
	void EnableDisableInternetUsage(bool bEnable);
	bool CheckIsEXEBlockedbyPC(LPTSTR lpszFilePath);
	bool ReLoadBlockedAppList();
	void CreateTable4EmailScan();
	void CreateTable4ParentalControl();
	bool ReloadUserList();
	void InsertBrowseSessionReport();
	bool ReloadUserPermission();
	bool ReloadUserResetValue();
	void CreateTable4BrowserSec();
	void CreateTable4BrowseSecurity();
	bool CheckInternetAccess();
	char*						m_strDatabaseFilePath = ".\\VBALLREPORTS.DB";

	CISpyCriticalSection	m_objISpyCriticalSection;

private:
	HMODULE							m_hModuleEmailDLL;
	WRDWIZEMAILINITIALIZE			m_lpEmailInitialize;
	WRDWIZEMAILUNINITIALIZE			m_lpEmailUnInitialize;
	WRDWIZRELOADAPPRULES			m_lpReloadAppRules;
	RELOADEMAILSCANSETTINGS			m_lpReloadEmailScanSettings;
	RELOADPARENTALCONTROLSETTINGS	m_lpReloadParentalControlSettings;
	STARTPARCONTROLCHECK			m_lpStartParentalControlCheck;
	STARTCOMPUSAGECHECK				m_lpStartCompUsageCheck;
	STARTINTERNETCHECK				m_lpStartInternetCheck;
	STARTINETUSAGECHECK				m_lpStartINetUsageCheck;
	ENABLEDISABLEINTERNETUSAGE		m_lpEnableDisableIU;
	CHECKEXEISBLOCKED				m_lpCheckEXEBlockedbyPC;
	RELOADAPPLICATIONLIST			m_lpReloadApplicationList;
	RELOADUSERLIST					m_lpReloadUserList;
	RELOADUSERPERMISSION			m_lpReloadUserPermission;
	RELOADUSERRESETVALUE			m_lpReloadUserResetValue;
	CHECKINTERNETVALUE				m_lpCheckInternetValue;
};

