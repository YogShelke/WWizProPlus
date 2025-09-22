/****************************************************
*  Program Name: CWardWizFirewall.h
*  Author Name: Kunal Waghmare
*  Date Of Creation: 30 April 2018
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once	

#include "WardWizUI.h"
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


class CWardWizFirewall : sciter::event_handler,
	sciter::behavior_factory
{
	HELEMENT self;
public:
	CWardWizFirewall();
	virtual ~CWardWizFirewall();

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
		FUNCTION_0("GetDBPath", GetDBPath);
		FUNCTION_0("OnClickApplyNow", OnClickApplyNow);
		FUNCTION_0("OnClickEdit", OnClickEdit);
		FUNCTION_1("OnSetRegVal", OnSetRegVal)
		FUNCTION_0("GetRegVal", GetRegVal)
		FUNCTION_1("CheckIsNetworkPath", CheckForNetworkPath)
		FUNCTION_1("GetTargetFilePath", GetTargetFilePath)
		FUNCTION_1("OnSetRegVal4PortScn", OnSetRegVal4PortScn)
		FUNCTION_1("OnSetRegVal4StealthMode", OnSetRegVal4StealthMode)
		FUNCTION_0("GetRegVal4PortScn", GetRegVal4PortScn)
		FUNCTION_0("GetRegVal4StealthMode", GetRegVal4StealthMode)
		FUNCTION_1("OnSetRegVal4DefAppBehavior", OnSetRegVal4DefAppBehavior)
		FUNCTION_0("GetRegVal4DefAppBehavior", GetRegVal4DefAppBehavior)
		FUNCTION_0("SetPage", OnClickSetPage)
		FUNCTION_0("SetOtherPage", OnClickSetOtherPage)
	END_FUNCTION_MAP

	json::value GetDBPath();
	json::value OnClickApplyNow();
	json::value OnClickEdit();
	json::value OnSetRegVal(SCITER_VALUE svbToggleState);
	json::value GetRegVal();
	json::value OnSetRegVal4PortScn(SCITER_VALUE svbToggleState);
	json::value OnSetRegVal4StealthMode(SCITER_VALUE svbToggleState);
	json::value GetRegVal4PortScn();
	json::value GetRegVal4StealthMode();
	json::value CheckForNetworkPath(SCITER_VALUE svFilePath);
	json::value GetTargetFilePath(SCITER_VALUE svFilePath);
	json::value OnSetRegVal4DefAppBehavior(SCITER_VALUE sviDropDownVal);
	json::value GetRegVal4DefAppBehavior();
	json::value OnClickSetPage();
	json::value OnClickSetOtherPage();

public:
	CISpyCommunicator m_objComService;
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);
	BOOL WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue);
	bool SendData2ComService(int iMessageInfo, bool bWait = false);
	bool SendData2ComService(int iMessageInfo, DWORD dwValue, DWORD dwSecondValue, LPTSTR szFirstParam, bool bWait = false);
	INT64 InsertDataToTable(const char* szQuery);
	CString GetSQLiteDBFilePath();

private:
	bool LoadRequiredLibrary();

private:
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

