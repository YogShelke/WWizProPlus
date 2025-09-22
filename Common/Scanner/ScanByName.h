/**********************************************************************************************************
	Program Name          :     ScanByName.h
	Description           :     This class is used implemented to exclude files and folders from scanning.
	Author Name			  :     Ramkrushna Shelke
	Date Of Creation      :
	Version No            :     1.14.0.1
	Special Logic Used    :     This class has following functionality
								a) Load Existing Exlude DB 
								b) Add/edit/delete exclude folders from scanning.
								c) Save Exclude DB.
	Modification Log      :
	1.Ramkrushna Shelke               Introduced class         25-APR-2016					
***********************************************************************************************************/
#pragma once
#include "sqlite3.h"

using namespace std;

#define MAX_FILE_PATH_LENGTH  MAX_PATH *2

typedef struct _tagScanByNameList
{
	WCHAR			szFileName[0x96];
}STRUCTSCANBYNAMELIST;

typedef std::vector <STRUCTSCANBYNAMELIST> SCANBYNAMEMAP;
typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
typedef  int(*SQLITE3_PREPARE)( sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
typedef  int(*SQLITE3_CLOSE)(sqlite3 *);

class CScanByName
{
public:
	CScanByName();
	virtual ~CScanByName();
	bool LoadScanByNameDB();
	bool LoadExcludeDB4FileExt();
	bool IsScanByNameFile(LPTSTR lpszFileName);
	bool SaveEntriesInDB();

private:
	void GetRecords();
	bool LoadRequiredLibrary();
	CString GetModuleFilePath();
	bool ISSubFilePath(LPSTR lpString, LPSTR lpSearchString);
	void GetRecordsSEH();
private:
	sqlite3							*m_pdbfile;
	SCANBYNAMEMAP					m_vScanByNameList;
	HMODULE							m_hSQLiteDLL;
	SQLITE3_OPEN					m_pSQliteOpen;
	SQLITE3_PREPARE					m_pSQLitePrepare;
	SQLITE3_COLUMN_COUNT			m_pSQLiteColumnCount;
	SQLITE3_STEP					m_pSQLiteStep;
	SQLITE3_COLUMN_TEXT				m_pSQLiteColumnText;
	SQLITE3_CLOSE					m_pSQLiteClose;
	bool							m_bISDBLoaded4ScanFileName;
};