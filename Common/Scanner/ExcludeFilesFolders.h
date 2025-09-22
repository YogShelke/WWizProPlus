/**********************************************************************************************************
	Program Name          :     ExcludeFilesFolders.cpp
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

typedef struct _tagExludeList
{
	WCHAR szData[MAX_PATH];
	BYTE  byType;
}STRUCTEXLCUDELIST;

typedef struct _tagExcludeFileExtList
{
	WCHAR szFileExt[0x32];
}STRUCTEXCLUDEFILEEXTLIST;

typedef std::vector <STRUCTEXLCUDELIST> EXCLUDELISTMAP;
typedef std::vector <STRUCTEXCLUDEFILEEXTLIST> EXCLUDELISTFILEEXTMAP;
typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
typedef  int(*SQLITE3_PREPARE)( sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
typedef  int(*SQLITE3_CLOSE)(sqlite3 *);

class CExcludeFilesFolders
{
public:
	CExcludeFilesFolders();
	virtual ~CExcludeFilesFolders();
	bool LoadExcludeDB();
	bool LoadExcludeDB4FileExt();
	bool ISExcludedPath(LPTSTR lpszPath, bool &bISSubFolderExcluded);
	bool ISExcludedFileExt(LPTSTR lpszFileExt);
	bool SaveEntriesInDB();
	bool SaveEntriesInDB4Ext();
	void LoadCustExcPaths();
	void ReloadExcludeDB();
private:
	void GetRecords();
	bool LoadRequiredLibrary();
	CString GetModuleFilePath();
	bool ISSubFilePath(LPTSTR lpString, LPTSTR lpSearchString);
	void GetRecordsSEH();
	void GetRecords4FileExtSEH();
private:
	sqlite3							*m_pdbfile;
	EXCLUDELISTMAP					m_vExludeList;
	EXCLUDELISTFILEEXTMAP			m_vExcludeFileExtList;
	HMODULE							m_hSQLiteDLL;
	SQLITE3_OPEN					m_pSQliteOpen;
	SQLITE3_PREPARE					m_pSQLitePrepare;
	SQLITE3_COLUMN_COUNT			m_pSQLiteColumnCount;
	SQLITE3_STEP					m_pSQLiteStep;
	SQLITE3_COLUMN_TEXT				m_pSQLiteColumnText;
	SQLITE3_CLOSE					m_pSQLiteClose;
	bool							m_bISDBLoaded;
	bool							m_bISDBLoaded4FileExt;
};

/*=========================================================================*/
/*							ENUMS										   */
/*=========================================================================*/

typedef enum ___COLUMN_NUMBER
{ 
	COLUMN_FILE_PATH = 1,
	COLUMN_IS_SUB_FOL
};
/*=========================================================================*/