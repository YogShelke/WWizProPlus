/****************************************************
*  Program Name: CWardWizFolderLocker.h
*  Author Name: Jeena Mariam Saji
*  Date Of Creation: 15 March 2017
*  Version No: 3.1.0.0
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


class CWardWizFolderLocker : sciter::event_handler,
	sciter::behavior_factory
{
	HELEMENT self;
public:
	CWardWizFolderLocker();
	virtual ~CWardWizFolderLocker();
	
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

public:
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);

BEGIN_FUNCTION_MAP
	FUNCTION_0("GetDBPath", GetDBPath);
	FUNCTION_1("CheckIsNetworkPath", CheckForNetworkPath);
	FUNCTION_1("CheckFileOrFolderPath", CheckFileOrFolderPath)
	//FUNCTION_1("funEncryptPassword", OnClickfunEncryptPassword)
	
END_FUNCTION_MAP

json::value GetDBPath();
json::value CheckForNetworkPath(SCITER_VALUE svFilePath);
json::value CheckFileOrFolderPath(SCITER_VALUE svFileFolderPath);
//json::value OnClickfunEncryptPassword(SCITER_VALUE svBuffer);

public:
	CISpyCommunicator m_objComService;

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
