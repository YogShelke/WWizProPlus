/****************************************************
*  Program Name: CWardWizEmailScan.h
*  Author Name: Amol Jaware
*  Date Of Creation: 21 March 2017
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once

#include "WardWizUI.h"
#include "iTINRegWrapper.h"
#include "ISpyCommunicator.h"
#include "sqlite3.h"

typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
typedef  int(*SQLITE3_PREPARE)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
typedef  int(*SQLITE3_CLOSE)(sqlite3 *);


class CWardWizEmailScan : sciter::event_handler,
	sciter::behavior_factory
{
	HELEMENT self;
public:
	CWardWizEmailScan();
	virtual ~CWardWizEmailScan();


	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {

	}
	virtual void detached(HELEMENT he) {
	}

	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetDBPath", GetDBPath)
		FUNCTION_1("WriteSubjectInINI", On_WriteSubjectInINI)
		FUNCTION_3("OnClickChangeSettings", On_ClickChangeSettings) // On_ClickChangeSettings()
		FUNCTION_0("GetSubjectFromINI", GetSubjectFromINI)
		FUNCTION_0("SetPage",OnClickSetPage)
		FUNCTION_0("SetOtherPage", OnClickSetOtherPage)
		FUNCTION_1("OnSetRegVal", OnSetRegVal)
		FUNCTION_0("GetRegVal", GetRegVal)
	END_FUNCTION_MAP

	json::value GetDBPath();
	json::value On_WriteSubjectInINI(SCITER_VALUE svSubject);
	json::value On_ClickChangeSettings(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue);
	json::value GetSubjectFromINI();
	json::value OnClickSetPage();
	json::value OnClickSetOtherPage();
	json::value OnSetRegVal(SCITER_VALUE svbToggleState);
	json::value GetRegVal();
public:
	enum ENUM_WWIZ_EMAIL_SETTINGS
	{
		AUTO_QUARNTINE = 1
	}WWIZ_EMAIL_SETTINGS;

	CISpyCommunicator m_objComService;

	bool SendSettingsToService(int iMessageInfo, DWORD dwValue, DWORD dwSecondValue);
	BOOL WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);//Set the Registry Key Value for Menu Items using Service
	void On_ClickSettingsButton(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue);
	bool SetAutoQurantineOptionInReg(int iRegistryValue);
	bool SendData2ComService(int iMessageInfo, DWORD dwValue, bool bWait = false);

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

};
