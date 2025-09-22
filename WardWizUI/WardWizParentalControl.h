/****************************************************
*  Program Name: CWardWizParentalControl.h
*  Author Name: Jeena Mariam Saji
*  Date Of Creation: 30th April 2018
****************************************************/
#pragma once

#include "WardWizUI.h"
#include <winhttp.h>
#include "iTINRegWrapper.h"
#include "ISpyCommunicator.h"
#include "sqlite3.h"
#include "WardWizDatabaseInterface.h"

typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
typedef  int(*SQLITE3_PREPARE)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
typedef  int(*SQLITE3_CLOSE)(sqlite3 *);

class CWardWizParentalControl : sciter::event_handler,
	sciter::behavior_factory
{
	HELEMENT self;
public:
	CWardWizParentalControl();
	virtual ~CWardWizParentalControl();

	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}

	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetDBPath", GetDBPath)
		FUNCTION_0("GetDBPathBlockList", GetDBPathBlockList)
		FUNCTION_0("SetRectTrackChange", SetRectTrackChange)
		FUNCTION_0("SetRectTrackChangeList", SetRectTrackChangeList)
		FUNCTION_0("SetRectTrackChangeListINet", SetRectTrackChangeListINet)
		FUNCTION_0("SetRectTrackChangeForINet", SetRectTrackChangeForINet)
		FUNCTION_1("GetWindowsUserList", GetWindowsUserList)
		FUNCTION_0("GetSetRegistryVal", On_GetSetRegistryVal)
		FUNCTION_1("OnChangeRegVal", OnChangeRegVal)
		FUNCTION_2("GetSetRectTracker", GetSetRectTracker)
		FUNCTION_0("OnSetRestriction", OnSetRestriction)
		FUNCTION_0("OnSetInternetRestriction", OnSetInternetRestriction)
		FUNCTION_0("OnSetINetRestriction", OnSetINetRestriction)
		FUNCTION_0("OnChangeAppRestriction", OnChangeAppRestriction)
		FUNCTION_1("CheckIsNetworkPath", CheckForNetworkPath)
		FUNCTION_1("GetTargetFilePath", GetTargetFilePath)
		FUNCTION_2("OnSetUserTogglerVal", OnSetUserTogglerVal)
		FUNCTION_1("CheckIsWrdWizFile", CheckIsWrdWizFile)
		FUNCTION_1("CheckDomainName", CheckDomainName)
		FUNCTION_0("SetPage", OnClickSetPage)
		FUNCTION_0("SetOtherPage", OnClickSetOtherPage)
		FUNCTION_0("CreatePCBrowseSecDB", CreatePCBrowseSecDB)
		FUNCTION_0("GetPCBrowseSecDBPath", GetPCBrowseSecDBPath)
		FUNCTION_0("ReloadDB4WebAgeSec", ReloadDB4WebAgeSec)
		FUNCTION_0("ReloadBrowseProtDB", ReloadBrowseProtDB)
		FUNCTION_0("ReloadDB4BlkSpecWeb", ReloadDB4BlkSpecWeb)
		FUNCTION_0("ReloadDB4MngExc", ReloadDB4MngExc)
		FUNCTION_2("IsDomainName", IsDomainName)
	END_FUNCTION_MAP

	json::value SetRectTrackChange();
	json::value SetRectTrackChangeList();
	json::value SetRectTrackChangeListINet();
	json::value SetRectTrackChangeForINet();
	json::value GetDBPath();
	json::value GetWindowsUserList(SCITER_VALUE svGetUserList);
	json::value On_GetSetRegistryVal();
	json::value OnChangeRegVal(SCITER_VALUE svToggleState);
	json::value GetSetRectTracker(SCITER_VALUE svCheckRectTrackChange, SCITER_VALUE svCheckRectTrackINetChange);
	json::value OnSetRestriction();
	json::value OnSetInternetRestriction();
	json::value OnSetINetRestriction();
	json::value OnChangeAppRestriction();
	json::value CheckForNetworkPath(SCITER_VALUE svFilePath);
	json::value GetTargetFilePath(SCITER_VALUE svFilePath);
	json::value GetDBPathBlockList();
	json::value OnSetUserTogglerVal(SCITER_VALUE svUserValue, SCITER_VALUE svUserStatusValue);
	json::value CheckIsWrdWizFile(SCITER_VALUE svFilePathQuarantine);
	json::value OnClickSetPage();
	json::value OnClickSetOtherPage();
	json::value CreatePCBrowseSecDB();
	json::value GetPCBrowseSecDBPath();
	json::value ReloadDB4WebAgeSec();
	json::value ReloadBrowseProtDB();
	json::value ReloadDB4BlkSpecWeb();
	json::value ReloadDB4MngExc();
	json::value CheckDomainName(SCITER_VALUE);
	json::value IsDomainName(SCITER_VALUE, SCITER_VALUE);

	bool SendData2Service(DWORD dwMsg, bool bWait = false);
	void ChangeParCtrlSetting();
	void CallCheckRectTrackChange();
	void CallCheckRectTrackChangeINet();
	void ChangeParCtrlSettingOnToggler(bool bTogglerVal);

	SCITER_VALUE m_svCheckRectTrackChange;
	SCITER_VALUE m_svCheckRectTrackINetChange;

	INT64 InsertDataToTable(const char* szQuery);
	CString GetSQLiteDBFilePath();
	CString GetDomainNameFromURL(CString csURL);

public:
	CISpyCommunicator m_objComService;
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);//Set the Registry Key Value for Menu Items using Service
	BOOL WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue);
	bool SendData2ComService(int iMessageInfo, DWORD dwValue, bool bWait = false);
	bool SendParCtrlMessage2Tray(int iRequest, CString csUserName);

private:
	bool LoadRequiredLibrary();

private:
	sqlite3							*m_pdbfile;
	HMODULE							m_hSQLiteDLL;
	SQLITE3_OPEN					m_pSQliteOpen;
	SQLITE3_PREPARE					m_pSQLitePrepare;
	SQLITE3_COLUMN_COUNT			m_pSQLiteColumnCount;
	SQLITE3_STEP					m_pSQLiteStep;
	SQLITE3_COLUMN_TEXT				m_pSQLiteColumnText;
	SQLITE3_CLOSE					m_pSQLiteClose;
	bool							m_bISDBLoaded;
public:
	CWardWizSQLiteDatabase	m_objSqlDb;
};

