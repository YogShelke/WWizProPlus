// WardwizEmailScn.h : main header file for the WardwizEmailScn DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include <mutex>

using namespace std;

enum FILTERFLAGS
{
	FILTER_NONE = 0,
	FILTER_APPLICATION = 1,
	FILTER_WEB = 2
};

class CWardwizEmailScnApp : public CWinApp
{
public:
	CWardwizEmailScnApp();

	enum {
		EN_SUNDAY = 1,
		EN_MONDAY,
		EN_TUESDAY,
		EN_WEDNESDAY,
		EN_THURSDAY,
		EN_FRIDAY,
		EN_SATURDAY,
	};

	typedef struct _tagParCtrlValCheck{
		int iWDayCompUsage;
		int iWEndCompUsage;
	}PARCTRLUSAGECHECK;


	typedef struct _tagParCtrlINetValCheck{
		int iWDayINetUsage;
		int iWEndINetUsage;
	}PARCTRLINETUSAGECHECK;

	typedef std::map<std::string, bool>MAPUSERLISTRESET;
	MAPUSERLISTRESET				m_mapParCtrlUserList;
	bool m_bBlockInternetUsage;
	bool m_bRestrictInternetUsage;
// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	void EnableFilter(FILTERFLAGS filter);
	void DisableFilter(FILTERFLAGS filter);

	bool WrdWizEmailInitialize();
	bool ReloadApplicationRules();
	bool WrdWizEmailUnInitialize();
	void WrdWizEmailNFDllUnlink();

	void UnregisterNFDrivers();
	void GetNFAPIDriverPath();
	void DeleteNFDrivers(CString csWrdWizFLTDriverPath);

	bool SendMessage2UI(int iRequest);
	bool SendParCtrlMessage2Tray(int iRequest);
	void WardWizParCtrlCheck();
	CString GetCurrentUsrName();
	void WardWizParCtrlCompUsageCheck();
	INT64 InsertSQLiteDataForPC(const char* szQuery);
	void WardWizParCtrlINetUsageCheck();
	bool CheckEXEBlocked(LPTSTR lpszFilePath);
	void EnableDisableEmailScan(bool bEmailScanState);
	bool ParseVersionString(int iDigits[4], CString& csVersion);
	bool SetDefaultAppBehaviour(DWORD dwValue);
	DWORD WrdWizFwUnInitialize();
	DWORD WrdWizFwInitialize();
	DWORD WrdWizFwInitializeSEH();
	DWORD CreateRule(LPSTR lpProcessPath, RULE_ACTION dwAction, TRAFFIC_DIRECTION dwDirection, bool bCheckMD5, BYTE dwCheckProtocol,
		BYTE Protocol, BYTE bCheckLocalIP, LPTSTR csLocalIP, BYTE bCheckRemoteIP, LPTSTR csRemoteIP, BYTE bCheckLocalPort,
		LPTSTR csLocalPortRange, BYTE bCheckRemotePort, LPTSTR csRemotePortRange);
	bool CreateFirewallDB();
	bool DeleteRule(LONGLONG id);
	bool ClearAllRules();
	bool EnableDisablePortScan(bool bEnable);
	bool SendParCtrlMessage2Tray(int iRequest, bool bWait);
	bool EnableDisableStealthMode(bool bEnable);
	void DisablePFFilter(FILTERFLAGS filter);
	bool IsFilterEnabled(FILTERFLAGS filter) const; 
	void EnablePFFilter(FILTERFLAGS filter);
	bool WrdWizInitializeWebFilter();
	bool WrdWizUnInitializeWebFilter();
	bool WrdWizInitializeBrowserSec();
	void UninitializWWPCWebFileter();
	bool WardWizUnnitializeBrowserSec();
	void EnableDisableParentalControl(bool bPControlState);
	void LoadUserPermission();
	bool CheckUserResetValue(std::string strUserName);
	void FunSetParCtrlUserResetVal(std::string strUserName);
	bool CheckFilterFlagsDisabled();
	bool LoadDefaultRules();
public:
	bool m_bIsEmailScanEnabled;
	bool m_bIsWebFilterEnabled;
	bool m_bIsBrowserSecEnabled;
	bool m_bPControlState;
	bool			m_bInitialized;
	int				m_iEnabledFilters;
	std::mutex		m_MutexLock;

	DECLARE_MESSAGE_MAP()
};

extern CWardwizEmailScnApp theApp;