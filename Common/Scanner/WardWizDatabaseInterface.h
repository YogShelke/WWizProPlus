/**********************************************************************************************************
Program Name          : WardWizDatabaseInterface.h
Description           : Header file for Wardwiz database interface implementation.
Author Name			  : Gayatri A.
Date Of Creation      : 16 Aug 2016
Version No            : 0.0.0.0
Special Logic Used    :

Modification Log      :
***********************************************************************************************************/
#ifndef _WRDWZDBINTERFACE_H_
#define _WRDWZDBINTERFACE_H_


#include "..\SQLiteReports\sqlite3.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include "Shlwapi.h"

#define WardwizSQLITE_ERROR 1000

// Required Function pointers for class CWardWizSQLiteDatabase
typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
typedef int(*SQLITE3_CLOSE)(sqlite3 *);
typedef int (*SQLITE3_EXEC)(sqlite3*, const char *sql, int(*callback)(void*, int, char**, char**), void *, char **errmsg); 
typedef int(*SQLITE3_CHANGES)(sqlite3*);
typedef int(*SQLITE3_STEP)(sqlite3_stmt*);
typedef int(*SQLITE3_FINALIZE)(sqlite3_stmt *pStmt);
typedef const char * (*SQLITE3_ERRMSG)(sqlite3*);
typedef int(*SQLITE3_GET_TABLE)(sqlite3*, const char *sql, char ***resultp, int *nrow, int *ncolumn, char **errmsg);
typedef sqlite_int64(*SQLITE3_LAST_INSERT_ROWID)(sqlite3*);
typedef int(*SQLITE_BUSY_TIMEOUT)(sqlite3*, int ms);
typedef int(*SQLITE3_PREPARE_V2)(sqlite3 *db, const char *zSql, int nBytes, sqlite3_stmt **ppStmt, const char **pzTail);
typedef int(*SQLITE3_GET_AUTOCOMMIT)(sqlite3*);

// Required Function pointers for class CWardwizSQLiteBuffer
typedef void(*SQLITE3_FREE)(void*);
typedef char * (*SQLITE3_VMPRINTF)(const char*, va_list);

// Required Function pointers for class CWardwizSQLiteStatement
typedef int(*SQLITE3_CHANGES)(sqlite3*);
typedef int(*SQLITE3_RESET)(sqlite3_stmt *pStmt);
typedef const char *(*SQLITE3_ERRMSG)(sqlite3*);
typedef int(*SQLITE3_BIND_TEXT)(sqlite3_stmt*, int, const char*, int n, void(*)(void*));
typedef int(*SQLITE3_BIND_INT)(sqlite3_stmt*, int, int);
typedef int(*SQLITE3_BIND_DOUBLE)(sqlite3_stmt*, int, double);
typedef int(*SQLITE3_BIND_BLOB)(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
typedef int(*SQLITE3_BIND_NULL)(sqlite3_stmt*, int);
typedef int(*SQLITE3_BIND_PARAMETER_INDEX)(sqlite3_stmt*, const char *zName);
typedef int(*SQLITE3_BIND_PARAMETER_CONT)(sqlite3_stmt*);
typedef const char *(*SQLITE3_BIND_PARAMETER_NAME)(sqlite3_stmt*, int);

// Required Function pointers for class CWardwizSQLiteTable
typedef void(*SQLITE3_FREE_TABLE)(char **result);

// Required Function pointers for class CWardwizSQLiteQuery
typedef int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
typedef int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt *pStmt);

typedef int(*SQLITE3_COLUMN_INT)(sqlite3_stmt*, int iCol);
typedef sqlite_int64(*SQLITE3_COLUMN_INT64)(sqlite3_stmt*, int iCol);
typedef double(*SQLITE3_COLUMN_DOUBLE)(sqlite3_stmt*, int iCol);
typedef int(*SQLITE3_COLUMN_BYTES)(sqlite3_stmt*, int iCol);
typedef const void * (*SQLITE3_COLUMN_BLOB)(sqlite3_stmt*, int iCol);
typedef const char * (*SQLITE3_COLUMN_NAME)(sqlite3_stmt*, int N);
typedef const char * (*SQLITE3_COLUMN_DECLTYPE)(sqlite3_stmt *, int i);
typedef int(*SQLITE3_COLUMN_TYPE)(sqlite3_stmt*, int iCol);

//Required for Exceptions
typedef char *(*SQLITE3_MPRINTF)(const char*, ...);

class CWardwizSQLiteException
{
public:

	CWardwizSQLiteException(const int nErrCode,
		char* szErrMess,
		bool bDeleteMsg = true);

	CWardwizSQLiteException(const CWardwizSQLiteException&  e);

	virtual ~CWardwizSQLiteException();

	const int errorCode() { return mnErrCode; }

	const char* errorMessage() { return mpszErrMess; }

	static const char* errorCodeAsString(int nErrCode);

private:

	int mnErrCode;
	char*			mpszErrMess;
	SQLITE3_FREE	m_pfnSQLiteFree;
	SQLITE3_MPRINTF m_pfnSQLitemPrintf;
	bool		    LoadRequiredLibrary();
};

class CWardwizSQLiteBuffer
{
public:

	CWardwizSQLiteBuffer();

	~CWardwizSQLiteBuffer();

	const char* Format(const char* szFormat, ...);

	operator const char*() { return m_pBuf; }

	void Clear();

private:
	bool LoadRequiredLibrary();
	SQLITE3_FREE	m_pfnSQLiteFree;
	SQLITE3_VMPRINTF	m_pfnSQLiteVmPrintf;

	char* m_pBuf;
};

class CWardwizSQLiteQuery
{
public:

	CWardwizSQLiteQuery();

	CWardwizSQLiteQuery(const CWardwizSQLiteQuery& objQuery);

	CWardwizSQLiteQuery(sqlite3* pSQLiteDB,
		sqlite3_stmt* pSQLStmt,
		bool bEof,
		bool bOwnVM = true);

	CWardwizSQLiteQuery& operator=(const CWardwizSQLiteQuery& objQuery);

	virtual ~CWardwizSQLiteQuery();

	int GetNumFields();

	int GetFieldIndex(const char* szField);
	const char* GetFieldName(int iCol);

	const char* GetFieldDeclType(int iCol);
	int GetFieldDataType(int iCol);

	const char* GetFieldValue(int iField);
	const char* GetFieldValue(const char* szField);

	int GetIntField(int iField, int iNullValue = 0);
	int GetIntField(const char* szField, int iNullValue = 0);

	sqlite_int64 GetInt64Field(int iField, sqlite_int64 iNullValue = 0);
	sqlite_int64 GetInt64Field(const char* szField, sqlite_int64 iNullValue = 0);

	double GetFloatField(int iField, double fNullValue = 0.0);
	double GetFloatField(const char* szField, double fNullValue = 0.0);

	const char* GetStringField(int nField, const char* szNullValue = "");
	const char* GetStringField(const char* szField, const char* szNullValue = "");

	const unsigned char* GetBlobField(int nField, int& nLen);
	const unsigned char* GetBlobField(const char* szField, int& nLen);

	bool GetFieldIsNull(int nField);
	bool GetFieldIsNull(const char* szField);

	bool IsEOF();

	void NextRow();

	void Finalize();

private:
	SQLITE3_COLUMN_COUNT	m_pfnSQLiteColumnCount;
	SQLITE3_COLUMN_TEXT		m_pfnSQLiteColumnText;
	SQLITE3_COLUMN_INT		m_pfnSQLiteColumnInt;
	SQLITE3_COLUMN_INT64	m_pfnSQLiteColumnInt64;
	SQLITE3_COLUMN_DOUBLE	m_pfnSQLiteColumnDouble;
	SQLITE3_COLUMN_BYTES	m_pfnSQLiteColumnBytes;
	SQLITE3_COLUMN_BLOB		m_pfnSQLiteColumnBlob;
	SQLITE3_COLUMN_NAME		m_pfnSQLiteColumnName;
	SQLITE3_COLUMN_DECLTYPE	m_pfnSQLiteColumnDeclType;
	SQLITE3_COLUMN_TYPE		m_pfnSQLiteColumnType;
	SQLITE3_STEP			m_pfnSQLiteStep;
	SQLITE3_FINALIZE		m_pfnSQLiteFinalize;
	SQLITE3_ERRMSG			m_pfnSQLiteErrmsg;

	bool LoadRequiredLibrary();
	void CheckSQLiteStmt();

	sqlite3* m_pobjSQLiteDB;
	sqlite3_stmt* m_pobjSQLStmt;
	bool m_bEof;
	int m_nCols;
	bool m_bOwnSQL;
};

class CWardwizSQLiteTable
{
public:

	CWardwizSQLiteTable();

	CWardwizSQLiteTable(const CWardwizSQLiteTable& objSQLiteTable);

	CWardwizSQLiteTable(char** paszResults, int iRows, int iCols);

	virtual ~CWardwizSQLiteTable();

	CWardwizSQLiteTable& operator=(const CWardwizSQLiteTable& objSQLiteTable);

	int GetNumFields();

	int GetNumRows();

	const char* GetFieldName(int iCol);

	const char* GetFieldValue(int iField);
	const char* GetFieldValue(const char* szField);

	int GetIntField(int iField, int iNullValue = 0);
	int GetIntField(const char* szField, int iNullValue = 0);

	double GetFloatField(int iField, double dNullValue = 0.0);
	double GetFloatField(const char* szField, double dNullValue = 0.0);

	const char* GetStringField(int iField, const char* szNullValue = "");
	const char* GetStringField(const char* szField, const char* szNullValue = "");

	bool GetFieldIsNull(int iField);
	bool GetFieldIsNull(const char* szField);

	void SetRow(int iRow);

	void Finalize();

private:
	SQLITE3_FREE_TABLE	m_pfnSQLiteFreeTable;

	void CheckResults();
	bool LoadRequiredLibrary();

	int m_iCols;
	int m_iRows;
	int m_iCurrentRow;
	char** mpaszResults;
};

class CWardwizSQLiteStatement
{
public:

	CWardwizSQLiteStatement();

	CWardwizSQLiteStatement(const CWardwizSQLiteStatement& objSQLStatement);

	CWardwizSQLiteStatement(sqlite3* pobjDB, sqlite3_stmt* pobjSQLStmt);

	virtual ~CWardwizSQLiteStatement();

	CWardwizSQLiteStatement& operator=(const CWardwizSQLiteStatement& objSQLStatement);

	int ExecDML();

	CWardwizSQLiteQuery ExecQuery();

	void Bind(int nParam, const char* szValue);
	void Bind(int nParam, const int nValue);
	void Bind(int nParam, const double dwValue);
	void Bind(int nParam, const unsigned char* blobValue, int nLen);
	void BindNull(int nParam);

	int BindParameterIndex(const char* szParam);
	void Bind(const char* szParam, const char* szValue);
	void Bind(const char* szParam, const int nValue);
	void Bind(const char* szParam, const double dwValue);
	void Bind(const char* szParam, const unsigned char* blobValue, int nLen);
	void BindNull(const char* szParam);

	void Reset();

	void Finalize();

private:
	void CheckDB();
	void CheckVM();
	bool LoadRequiredLibrary();
	sqlite3* m_pSQLiteDB;
	sqlite3_stmt* m_pSQLiteStmt;

	SQLITE3_STEP			m_pfnSQLiteStep;
	SQLITE3_FINALIZE		m_pfnSQLiteFinalize;
	SQLITE3_CHANGES			m_pfnSQLiteChanges;
	SQLITE3_RESET			m_pfnSQLiteReset;
	SQLITE3_ERRMSG			m_pfnSQLiteErrMsg;
	SQLITE3_BIND_TEXT		m_pfnSQLiteBindText;
	SQLITE3_BIND_INT		m_pfnSQLiteBindInt;
	SQLITE3_BIND_DOUBLE		m_pfnSQLiteBindDouble;
	SQLITE3_BIND_BLOB		m_pfnSQLiteBindBlob;
	SQLITE3_BIND_NULL		m_pfnSQLiteBindNull;
	SQLITE3_BIND_PARAMETER_INDEX	m_pfnSQLiteBindParamIndex;
	SQLITE3_BIND_PARAMETER_CONT	m_pfnSQLiteBindParamCount;
	SQLITE3_BIND_PARAMETER_NAME	m_pfnSQLiteBindParamName;

};

class CWardWizSQLiteDatabase
{
public:
	CWardWizSQLiteDatabase();
	CWardWizSQLiteDatabase( char* szFile);
	void SetDataBaseFilePath( char* szFile);
	virtual ~CWardWizSQLiteDatabase();
	
	bool IsAutoCommitOn();

	void Open();

	void Close();

	bool TableExists(const char* szTable);

	int ExecDML(const char* szSQL);

	CWardwizSQLiteQuery ExecQuery(const char* szSQL);

	int ExecScalar(const char* szSQL, int nNullValue = 0);

	CWardwizSQLiteTable GetTable(const char* szSQL);

	CWardwizSQLiteStatement CompileSQLStatement(const char* szSQL);

	sqlite_int64 GetLastRowId();
	
	void CreateWardwizSQLiteTables(DWORD dwProdID);
	void AlterWardwizSQLiteTables(const char* strTableName, const char* strColumnName, const char* strDatatype);
	void interrupt() { sqlite3_interrupt(m_pSQLiteDB); }

	void setBusyTimeout(int nMillisecs);

	static const char* SQLiteVersion() { return SQLITE_VERSION; }
	static const char* SQLiteHeaderVersion() { return SQLITE_VERSION; }
	static const char* SQLiteLibraryVersion() { return sqlite3_libversion(); }
	static int SQLiteLibraryVersionNumber() { return sqlite3_libversion_number(); }
	
private:
	SQLITE3_OPEN			m_pfnSQLiteOpen;
	SQLITE3_CLOSE			m_pfnSQLiteClose;
	SQLITE3_EXEC			m_pfnSQLiteExec;
	SQLITE3_CHANGES			m_pfnSQLiteChanges;
	SQLITE3_STEP			m_pfnSQLiteStep;
	SQLITE3_FINALIZE		m_pfnSQLiteFinalize;
	SQLITE3_ERRMSG			m_pfnSQLiteErrMsg;
	SQLITE3_GET_TABLE		m_pfnSQLiteGetTable;
	SQLITE3_LAST_INSERT_ROWID	m_pfnSQLiteLastInsertRowID;
	SQLITE_BUSY_TIMEOUT			m_pfnSQLiteBusyTimeOut;
	SQLITE3_PREPARE_V2			m_pfnSQLitePrepareV2;
	SQLITE3_GET_AUTOCOMMIT		m_pfnSQLiteAutoCommit;

	sqlite3* m_pSQLiteDB;
	int m_iBusyTimeoutMs;
	char * pszDatabaseFileName;

	bool LoadRequiredLibrary();

	CWardWizSQLiteDatabase(const CWardWizSQLiteDatabase& objDBInterface);
	CWardWizSQLiteDatabase& operator=(const CWardWizSQLiteDatabase& objDBInterface);

	sqlite3_stmt* Compile(const char* szSQLQuery);

	bool CheckDataBase();
	int ExecDMLWithNoException(const char* szSQL);
};

#endif;