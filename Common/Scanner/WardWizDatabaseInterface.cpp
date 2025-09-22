/**********************************************************************************************************
Program Name          : WardWizDatabaseInterface.cpp
Description           : Defines the functions required for SQLite connection and execution.
Author Name			  : Gayatri A.
Date Of Creation      : 16 Aug 2016
Version No            : 0.0.0.0
Special Logic Used    :
Modification Log      :
***********************************************************************************************************/

#include "stdafx.h"
#include "WardWizDatabaseInterface.h"

static const bool DONT_DELETE_MSG = false;
HMODULE	g_hSQLiteDLL = NULL;

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CString GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	try
	{
		GetModuleFileName(NULL, szModulePath, MAX_PATH);

		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetModuleFilePath", 0, 0, true, SECONDLEVEL);
	}
	return(CString(szModulePath));
}

/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary
*  Description    : Load required library required for SQLite functionality
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteException::LoadRequiredLibrary()
{
	bool bReturn = false;

	try
	{
		DWORD	dwRet = 0x00;
		CString	csDLLPath = L"";
		csDLLPath.Format(L"%s\\SQLITE3.DLL", GetModuleFilePath());

		if (!PathFileExists(csDLLPath))
		{
			return false;
		}

		if (!g_hSQLiteDLL)
		{
			g_hSQLiteDLL = LoadLibrary(csDLLPath);
			if (!g_hSQLiteDLL)
			{
				return false;
			}
		}

		m_pfnSQLiteFree = (SQLITE3_FREE)GetProcAddress(g_hSQLiteDLL, "sqlite3_open");
		m_pfnSQLitemPrintf = (SQLITE3_MPRINTF)GetProcAddress(g_hSQLiteDLL, "sqlite3_mprintf");

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteException::LoadRequiredLibrary()", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteException
*  Description    : CTOR CWardwizSQLiteException
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteException::CWardwizSQLiteException(const int nErrCode,	char* szErrMess,bool bDeleteMsg/*=true*/) :
	mnErrCode(nErrCode)
{
	try
	{
		if (!m_pfnSQLitemPrintf || !m_pfnSQLiteFree)
		{
			LoadRequiredLibrary();
		}

		mpszErrMess = m_pfnSQLitemPrintf("%s[%d]: %s", errorCodeAsString(nErrCode), nErrCode, szErrMess ? szErrMess : "");

		AddLogEntry(L"### Exception in CWardwizSQLiteException, Error: %s", CA2T(mpszErrMess), 0, true, SECONDLEVEL);

		if (bDeleteMsg && szErrMess)
		{
			m_pfnSQLiteFree(szErrMess);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteException::CWardwizSQLiteException, Error: %s", CA2T(szErrMess), 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteException
*  Description    : CTOR CWardwizSQLiteException
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteException::CWardwizSQLiteException(const CWardwizSQLiteException&  e) :
mnErrCode(e.mnErrCode)
{
	try
	{
		mpszErrMess = 0;
		if (e.mpszErrMess)
		{
			mpszErrMess = m_pfnSQLitemPrintf("%s", e.mpszErrMess);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteException::CWardwizSQLiteException, Error: %s", CA2T(mpszErrMess), 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : errorCodeAsString
*  Description    : Get SQLIte exception messages as string
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteException::errorCodeAsString(int nErrCode)
{
	switch (nErrCode)
	{
	case SQLITE_OK: return "SQLITE_OK";
	case SQLITE_ERROR: return "SQLITE_ERROR";
	case SQLITE_INTERNAL: return "SQLITE_INTERNAL";
	case SQLITE_PERM: return "SQLITE_PERM";
	case SQLITE_ABORT: return "SQLITE_ABORT";
	case SQLITE_BUSY: return "SQLITE_BUSY";
	case SQLITE_LOCKED: return "SQLITE_LOCKED";
	case SQLITE_NOMEM: return "SQLITE_NOMEM";
	case SQLITE_READONLY: return "SQLITE_READONLY";
	case SQLITE_INTERRUPT: return "SQLITE_INTERRUPT";
	case SQLITE_IOERR: return "SQLITE_IOERR";
	case SQLITE_CORRUPT: return "SQLITE_CORRUPT";
	case SQLITE_NOTFOUND: return "SQLITE_NOTFOUND";
	case SQLITE_FULL: return "SQLITE_FULL";
	case SQLITE_CANTOPEN: return "SQLITE_CANTOPEN";
	case SQLITE_PROTOCOL: return "SQLITE_PROTOCOL";
	case SQLITE_EMPTY: return "SQLITE_EMPTY";
	case SQLITE_SCHEMA: return "SQLITE_SCHEMA";
	case SQLITE_TOOBIG: return "SQLITE_TOOBIG";
	case SQLITE_CONSTRAINT: return "SQLITE_CONSTRAINT";
	case SQLITE_MISMATCH: return "SQLITE_MISMATCH";
	case SQLITE_MISUSE: return "SQLITE_MISUSE";
	case SQLITE_NOLFS: return "SQLITE_NOLFS";
	case SQLITE_AUTH: return "SQLITE_AUTH";
	case SQLITE_FORMAT: return "SQLITE_FORMAT";
	case SQLITE_RANGE: return "SQLITE_RANGE";
	case SQLITE_ROW: return "SQLITE_ROW";
	case SQLITE_DONE: return "SQLITE_DONE";
	case WardwizSQLITE_ERROR: return "SQLITE_ERROR";
	default: return "UNKNOWN_ERROR";
	}
}

/***************************************************************************************************
*  Function Name  : ~CWardwizSQLiteException
*  Description    : DTOR CWardwizSQLiteException
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteException::~CWardwizSQLiteException()
{
	if (mpszErrMess)
	{
		m_pfnSQLiteFree(mpszErrMess);
		mpszErrMess = 0;
	}
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteQuery
*  Description    : CTOR CWardwizSQLiteQuery
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteQuery::CWardwizSQLiteQuery()
{
	m_pobjSQLStmt = 0;
	m_bEof = true;
	m_nCols = 0;
	m_bOwnSQL= false;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteQuery
*  Description    : CTOR CWardwizSQLiteQuery
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteQuery::CWardwizSQLiteQuery(const CWardwizSQLiteQuery& objQuery)
{
	m_pobjSQLStmt = objQuery.m_pobjSQLStmt;
	// Only one object can own the statement
	const_cast<CWardwizSQLiteQuery&>(objQuery).m_pobjSQLStmt = 0;
	m_bEof = objQuery.m_bEof;
	m_nCols = objQuery.m_nCols;
	m_bOwnSQL = objQuery.m_bOwnSQL;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteQuery
*  Description    : CTOR CWardwizSQLiteQuery
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteQuery::CWardwizSQLiteQuery(sqlite3* pSQLiteDB,sqlite3_stmt* pSQLStmt,	bool bEof,	bool bOwnSQL/*=true*/)

{
	LoadRequiredLibrary();
	m_pobjSQLiteDB = pSQLiteDB;
	m_pobjSQLStmt = pSQLStmt;
	m_bEof = bEof;
	m_nCols = m_pfnSQLiteColumnCount(m_pobjSQLStmt);
	m_bOwnSQL = bOwnSQL;
	LoadRequiredLibrary();
}

/***************************************************************************************************
*  Function Name  : ~CWardwizSQLiteQuery
*  Description    : DTOR CWardwizSQLiteQuery
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteQuery::~CWardwizSQLiteQuery()
{
	try
	{
		Finalize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteQuery::~CWardwizSQLiteQuery()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : operator=
*  Description    : Overloaded operator=
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteQuery& CWardwizSQLiteQuery::operator=(const CWardwizSQLiteQuery& rQuery)
{
	try
	{
		Finalize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteQuery::operator=", 0, 0, true, SECONDLEVEL);
	}
	m_pobjSQLStmt = rQuery.m_pobjSQLStmt;
	// Only one object can own the VM
	const_cast<CWardwizSQLiteQuery&>(rQuery).m_pobjSQLStmt = 0;
	m_bEof = rQuery.m_bEof;
	m_nCols = rQuery.m_nCols;
	m_bOwnSQL = rQuery.m_bOwnSQL;
	return *this;
}

/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary
*  Description    : Load required library for SQLite connection and functionality
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteQuery::LoadRequiredLibrary()
{
	bool bReturn = false;

	try
	{
		DWORD	dwRet = 0x00;
		CString	csDLLPath = L"";
		csDLLPath.Format(L"%s\\SQLITE3.DLL", GetModuleFilePath());

		if (!PathFileExists(csDLLPath))
		{
			return false;
		}

		if (!g_hSQLiteDLL)
		{
			g_hSQLiteDLL = LoadLibrary(csDLLPath);
			if (!g_hSQLiteDLL)
			{
				return false;
			}
		}
		m_pfnSQLiteColumnCount = (SQLITE3_COLUMN_COUNT)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_count");
		m_pfnSQLiteColumnText = (SQLITE3_COLUMN_TEXT)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_text");
		m_pfnSQLiteColumnInt = (SQLITE3_COLUMN_INT)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_int");
		m_pfnSQLiteColumnInt64 = (SQLITE3_COLUMN_INT64)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_int64");
		m_pfnSQLiteColumnDouble = (SQLITE3_COLUMN_DOUBLE)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_double");
		m_pfnSQLiteColumnBytes = (SQLITE3_COLUMN_BYTES)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_bytes");
		m_pfnSQLiteColumnBlob = (SQLITE3_COLUMN_BLOB)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_blob");
		m_pfnSQLiteColumnName = (SQLITE3_COLUMN_NAME)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_name");
		m_pfnSQLiteColumnDeclType = (SQLITE3_COLUMN_DECLTYPE)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_decltype");
		m_pfnSQLiteColumnType = (SQLITE3_COLUMN_TYPE)GetProcAddress(g_hSQLiteDLL, "sqlite3_column_type");
		m_pfnSQLiteStep = (SQLITE3_STEP)GetProcAddress(g_hSQLiteDLL, "sqlite3_step");
		m_pfnSQLiteFinalize = (SQLITE3_FINALIZE)GetProcAddress(g_hSQLiteDLL, "sqlite3_finalize");
		m_pfnSQLiteErrmsg = (SQLITE3_ERRMSG)GetProcAddress(g_hSQLiteDLL, "sqlite3_errmsg");

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteQuery::operator=", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : GetNumFields
*  Description    : Get Numeric Fields
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteQuery::GetNumFields()
{
	CheckSQLiteStmt();
	return m_nCols;
}

/***************************************************************************************************
*  Function Name  : GetFieldValue
*  Description    : Get FieldValue
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteQuery::GetFieldValue(int nField)
{
	CheckSQLiteStmt();

	if (nField < 0 || nField > m_nCols - 1)
	{
		AddLogEntry(L"### Invalid field index requested in CWardwizSQLiteQuery::GetFieldValue", 0, 0, true, SECONDLEVEL);
		return "";
	}

	if (m_pfnSQLiteColumnText == NULL)
	{
		return "";
	}

	return (const char*)m_pfnSQLiteColumnText(m_pobjSQLStmt, nField);
}

/***************************************************************************************************
*  Function Name  : GetFieldValue
*  Description    : Get FieldValue
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteQuery::GetFieldValue(const char* szField)
{
	int nField = GetFieldIndex(szField);

	if (m_pfnSQLiteColumnText == NULL)
	{
		return "";
	}
	return (const char*)m_pfnSQLiteColumnText(m_pobjSQLStmt, nField);
}


/***************************************************************************************************
*  Function Name  : GetIntField
*  Description    : Get Int Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteQuery::GetIntField(int iField, int iNullValue/*=0*/)
{
	if (GetFieldDataType(iField) == SQLITE_NULL)
	{
		return iNullValue;
	}
	else
	{
		if (m_pfnSQLiteColumnInt == NULL)
		{
			return iNullValue;
		}
		return m_pfnSQLiteColumnInt (m_pobjSQLStmt, iField);
	}
}

/***************************************************************************************************
*  Function Name  : GetIntField
*  Description    : Get Int Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteQuery::GetIntField(const char* szField, int iNullValue/*=0*/)
{
	int iField = GetFieldIndex(szField);
	return GetIntField(iField, iNullValue);
}

/***************************************************************************************************
*  Function Name  : GetInt64Field
*  Description    : Get Int64 Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
sqlite_int64 CWardwizSQLiteQuery::GetInt64Field(int iField, sqlite_int64 iNullValue/*=0*/)
{
	if (GetFieldDataType(iField) == SQLITE_NULL)
	{
		return iNullValue;
	}
	else
	{
		if (m_pfnSQLiteColumnInt64 == NULL)
		{
			return iNullValue;
		}
		return m_pfnSQLiteColumnInt64(m_pobjSQLStmt, iField);
	}
}

/***************************************************************************************************
*  Function Name  : GetInt64Field
*  Description    : Get Int64 Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
sqlite_int64 CWardwizSQLiteQuery::GetInt64Field(const char* szField, sqlite_int64 iNullValue/*=0*/)
{
	int iField = GetFieldIndex(szField);
	return GetInt64Field(iField, iNullValue);
}

/***************************************************************************************************
*  Function Name  : GetFloatField
*  Description    : Get Float Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
double CWardwizSQLiteQuery::GetFloatField(int iField, double dNullValue/*=0.0*/)
{
	if (GetFieldDataType(iField) == SQLITE_NULL)
	{
		return dNullValue;
	}
	else
	{
		if (m_pfnSQLiteColumnDouble == NULL)
		{
			return dNullValue;
		}
		return m_pfnSQLiteColumnDouble(m_pobjSQLStmt, iField);
	}
}

/***************************************************************************************************
*  Function Name  : GetFloatField
*  Description    : Get Float Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
double CWardwizSQLiteQuery::GetFloatField(const char* szField, double dNullValue/*=0.0*/)
{
	int iField = GetFieldIndex(szField);
	return GetFloatField(iField, dNullValue);
}

/***************************************************************************************************
*  Function Name  : GetStringField
*  Description    : Get String Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteQuery::GetStringField(int iField, const char* szNullValue/*=""*/)
{
	__try
	{
		if (GetFieldDataType(iField) == SQLITE_NULL)
		{
			return szNullValue;
		}
		else
		{
			if (m_pfnSQLiteColumnText == NULL)
			{
				return szNullValue;
			}
			return (const char*)m_pfnSQLiteColumnText(m_pobjSQLStmt, iField);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteQuery::GetStringField", 0, 0, true, SECONDLEVEL);
	}
	return "";
}

/***************************************************************************************************
*  Function Name  : GetStringField
*  Description    : Get String Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteQuery::GetStringField(const char* szField, const char* szNullValue/*=""*/)
{
	int iField = GetFieldIndex(szField);
	return GetStringField(iField, szNullValue);
}

/***************************************************************************************************
*  Function Name  : GetBlobField
*  Description    : Get Blob Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const unsigned char* CWardwizSQLiteQuery::GetBlobField(int iField, int& iLen)
{
	CheckSQLiteStmt();

	if (iField < 0 || iField > m_nCols - 1)
	{
		return (const unsigned char*)"";
	}
	iLen = m_pfnSQLiteColumnBytes(m_pobjSQLStmt, iField);

	return (const unsigned char*)m_pfnSQLiteColumnBlob(m_pobjSQLStmt, iField);
}

/***************************************************************************************************
*  Function Name  : GetBlobField
*  Description    : Get Blob Field
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const unsigned char* CWardwizSQLiteQuery::GetBlobField(const char* szField, int& iLen)
{
	int iField = GetFieldIndex(szField);
	return GetBlobField(iField, iLen);
}

/***************************************************************************************************
*  Function Name  : GetFieldIsNull
*  Description    : Check Field IsNull
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteQuery::GetFieldIsNull(int iField)
{
	return (GetFieldDataType(iField) == SQLITE_NULL);
}

/***************************************************************************************************
*  Function Name  : GetFieldIsNull
*  Description    : Check if Field is null
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteQuery::GetFieldIsNull(const char* szField)
{
	int iField = GetFieldIndex(szField);
	return (GetFieldDataType(iField) == SQLITE_NULL);
}

/***************************************************************************************************
*  Function Name  : GetFieldIndex
*  Description    : Get Field Index
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteQuery::GetFieldIndex(const char* szField)
{
	__try
	{
		CheckSQLiteStmt();

		if (szField)
		{
			for (int iField = 0; iField < m_nCols; iField++)
			{
				const char* szTemp = m_pfnSQLiteColumnName(m_pobjSQLStmt, iField);

				if (strcmp(szField, szTemp) == 0)
				{
					return iField;
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteQuery::GetFieldIndex, Field: %s", A2BSTR(szField), 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetFieldName
*  Description    : Get Field Name
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteQuery::GetFieldName(int iCol)
{
	CheckSQLiteStmt();

	if (iCol < 0 || iCol > m_nCols - 1)
	{
		return (const char*)"";
	}
	return m_pfnSQLiteColumnName(m_pobjSQLStmt, iCol);
}

/***************************************************************************************************
*  Function Name  : GetFieldDeclType
*  Description    : Get Field Decl Type
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteQuery::GetFieldDeclType(int iCol)
{
	CheckSQLiteStmt();

	if (iCol < 0 || iCol > m_nCols - 1)
	{
		return (const char*)"";
	}
	return m_pfnSQLiteColumnDeclType(m_pobjSQLStmt, iCol);
}

/***************************************************************************************************
*  Function Name  : GetFieldDataType
*  Description    : Get Field DataType
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteQuery::GetFieldDataType(int iCol)
{
	CheckSQLiteStmt();

	if (iCol < 0 || iCol > m_nCols - 1)
	{
		return 0;
	}
	return m_pfnSQLiteColumnType(m_pobjSQLStmt, iCol);
}

/***************************************************************************************************
*  Function Name  : IsEOF
*  Description    : Check if EOF
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteQuery::IsEOF()
{
	CheckSQLiteStmt();
	return m_bEof;
}

/***************************************************************************************************
*  Function Name  : NextRow
*  Description    : Get NextRow
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteQuery::NextRow()
{
	CheckSQLiteStmt();
	int iRet = m_pfnSQLiteStep(m_pobjSQLStmt);

	if (iRet == SQLITE_DONE)
	{
		// no rows
		m_bEof = true;
	}
	else if (iRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		iRet = m_pfnSQLiteFinalize(m_pobjSQLStmt);
		m_pobjSQLStmt = 0;
		const char* szError = m_pfnSQLiteErrmsg(m_pobjSQLiteDB);
		AddLogEntry(L"### Exception in CWardwizSQLiteQuery::NextRow, Error: %s", A2BSTR(szError), 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : Finalize
*  Description    : Release system resources.
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteQuery::Finalize()
{
	if (m_pobjSQLStmt && m_bOwnSQL)
	{
		int nRet = m_pfnSQLiteFinalize(m_pobjSQLStmt);

		m_pobjSQLStmt = 0;

		if (nRet != SQLITE_OK)
		{
			const char* szError = m_pfnSQLiteErrmsg(m_pobjSQLiteDB);
			AddLogEntry(L"### Exception in CWardwizSQLiteQuery::Finalize, Error: %s", A2BSTR(szError), 0, true, SECONDLEVEL);
		}
	}
}

/***************************************************************************************************
*  Function Name  : CheckSQLiteStmt
*  Description    : Check SQLite Stmt for validity
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteQuery::CheckSQLiteStmt()
{
	if (m_pobjSQLStmt == 0)
	{
		AddLogEntry(L"### Null Virtual Machine pointer", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteTable
*  Description    : CTOR CWardwizSQLiteTable
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteTable::CWardwizSQLiteTable()
{
	mpaszResults = 0;
	m_iRows = 0;
	m_iCols = 0;
	m_iCurrentRow = 0;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteTable
*  Description    : CTOR CWardwizSQLiteTable
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteTable::CWardwizSQLiteTable(const CWardwizSQLiteTable& rTable)
{
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<CWardwizSQLiteTable&>(rTable).mpaszResults = 0;
	m_iRows = rTable.m_iRows;
	m_iCols = rTable.m_iCols;
	m_iCurrentRow = rTable.m_iCurrentRow;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteTable
*  Description    : CTOR CWardwizSQLiteTable 
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteTable::CWardwizSQLiteTable(char** paszResults, int nRows, int nCols)
{
	mpaszResults = paszResults;
	m_iRows = nRows;
	m_iCols = nCols;
	m_iCurrentRow = 0;
	LoadRequiredLibrary();
}

/***************************************************************************************************
*  Function Name  : ~CWardwizSQLiteTable
*  Description    : DTOR CWardwizSQLiteTable
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteTable::~CWardwizSQLiteTable()
{
	try
	{
		Finalize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteTable::~CWardwizSQLiteTable()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : operator=
*  Description    : operator= overloaded
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteTable& CWardwizSQLiteTable::operator=(const CWardwizSQLiteTable& rTable)
{
	try
	{
		Finalize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteTable& CWardwizSQLiteTable::operator=", 0, 0, true, SECONDLEVEL);
	}
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<CWardwizSQLiteTable&>(rTable).mpaszResults = 0;
	m_iRows = rTable.m_iRows;
	m_iCols = rTable.m_iCols;
	m_iCurrentRow = rTable.m_iCurrentRow;
	return *this;
}

/***************************************************************************************************
*  Function Name  : Finalize()
*  Description    : Finalizer code
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteTable::Finalize()
{
	if (mpaszResults)
	{
		m_pfnSQLiteFreeTable(mpaszResults);
		mpaszResults = 0;
	}
}

/***************************************************************************************************
*  Function Name  : GetNumFields
*  Description    : Get number of fields
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteTable::GetNumFields()
{
	CheckResults();
	return m_iCols;
}


/***************************************************************************************************
*  Function Name  : GetNumRows
*  Description    : Get number of Rows
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteTable::GetNumRows()
{
	CheckResults();
	return m_iRows;
}

/***************************************************************************************************
*  Function Name  : GetFieldValue
*  Description    : Get field value
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteTable::GetFieldValue(int nField)
{
	CheckResults();

	if (nField < 0 || nField > m_iCols - 1)
	{
		AddLogEntry(L"### Invalid field index requested in CWardwizSQLiteTable::GetFieldValue", 0, 0, true, SECONDLEVEL);
		return (const char *)"";
	}

	int nIndex = (m_iCurrentRow*m_iCols) + m_iCols + nField;
	return mpaszResults[nIndex];
}

/***************************************************************************************************
*  Function Name  : GetFieldValue
*  Description    : Get field value 
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteTable::GetFieldValue(const char* szField)
{
	CheckResults();

	if (szField)
	{
		for (int nField = 0; nField < m_iCols; nField++)
		{
			if (strcmp(szField, mpaszResults[nField]) == 0)
			{
				int nIndex = (m_iCurrentRow*m_iCols) + m_iCols + nField;
				return mpaszResults[nIndex];
			}
		}
	}
	AddLogEntry(L"### Invalid field index requested CWardwizSQLiteTable::GetFieldValue", 0, 0, true, SECONDLEVEL);
	return "";
}

/***************************************************************************************************
*  Function Name  : GetIntField
*  Description    : Get the int where module is exist
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteTable::GetIntField(int nField, int nNullValue/*=0*/)
{
	if (GetFieldIsNull(nField))
	{
		return nNullValue;
	}
	else
	{
		return atoi(GetFieldValue(nField));
	}
}

/***************************************************************************************************
*  Function Name  : GetIntField
*  Description    : Get the int where module is exist
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteTable::GetIntField(const char* szField, int nNullValue/*=0*/)
{
	if (GetFieldIsNull(szField))
	{
		return nNullValue;
	}
	else
	{
		return atoi(GetFieldValue(szField));
	}
}

/***************************************************************************************************
*  Function Name  : GetFloatField
*  Description    : Get Float value
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
double CWardwizSQLiteTable::GetFloatField(int iField, double dNullValue/*=0.0*/)
{
	if (GetFieldIsNull(iField))
	{
		return dNullValue;
	}
	else
	{
		return atof(GetFieldValue(iField));
	}
}

/***************************************************************************************************
*  Function Name  : GetFloatField
*  Description    : Get Float value
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
double CWardwizSQLiteTable::GetFloatField(const char* szField, double fNullValue/*=0.0*/)
{
	if (GetFieldIsNull(szField))
	{
		return fNullValue;
	}
	else
	{
		return atof(GetFieldValue(szField));
	}
}

/***************************************************************************************************
*  Function Name  : GetStringField
*  Description    : Get String value
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteTable::GetStringField(int iField, const char* szNullValue/*=""*/)
{
	if (GetFieldIsNull(iField))
	{
		return szNullValue;
	}
	else
	{
		return GetFieldValue(iField);
	}
}

/***************************************************************************************************
*  Function Name  : GetStringField
*  Description    : Get String value
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteTable::GetStringField(const char* szField, const char* szNullValue/*=""*/)
{
	if (GetFieldIsNull(szField))
	{
		return szNullValue;
	}
	else
	{
		return GetFieldValue(szField);
	}
}

/***************************************************************************************************
*  Function Name  : GetFieldIsNull
*  Description    : Check if field is null
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteTable::GetFieldIsNull(int nField)
{
	CheckResults();
	return (GetFieldValue(nField) == 0);
}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteTable::GetFieldIsNull(const char* szField)
{
	CheckResults();
	return (GetFieldValue(szField) == 0);
}

/***************************************************************************************************
*  Function Name  : GetFieldName
*  Description    : Get Field name from column index
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteTable::GetFieldName(int iCol)
{
	CheckResults();

	if (iCol < 0 || iCol > m_iCols - 1)
	{
		AddLogEntry(L"### Invalid field index requested in CWardwizSQLiteTable::GetFieldName", 0, 0, true, SECONDLEVEL);
		return (const char *)"";
		//throw CWardwizSQLiteException(WardwizSQLITE_ERROR,
		//	"Invalid field index requested",
		//	DONT_DELETE_MSG);
	}

	return mpaszResults[iCol];
}

/***************************************************************************************************
*  Function Name  : SetRow
*  Description    : Request for specific row index
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteTable::SetRow(int nRow)
{
	CheckResults();

	if (nRow < 0 || nRow > m_iRows - 1)
	{
		AddLogEntry(L"### Invalid field index requested in CWardwizSQLiteTable::SetRow", 0, 0, true, SECONDLEVEL);
	}

	m_iCurrentRow = nRow;
}

/***************************************************************************************************
*  Function Name  : CheckResults()
*  Description    : Check if results pointer is valid
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteTable::CheckResults()
{
	if (mpaszResults == 0)
	{
		AddLogEntry(L"### Null Results pointer in CWardwizSQLiteTable::CheckResults", 0, 0, true, ZEROLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary
*  Description    : Load Required Library for SQLite
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteTable::LoadRequiredLibrary()
{
	bool bReturn = false;

	try
	{
		DWORD	dwRet = 0x00;
		CString	csDLLPath = L"";
		csDLLPath.Format(L"%s\\SQLITE3.DLL", GetModuleFilePath());

		if (!PathFileExists(csDLLPath))
		{
			return false;
		}

		if (!g_hSQLiteDLL)
		{
			g_hSQLiteDLL = LoadLibrary(csDLLPath);
			if (!g_hSQLiteDLL)
			{
				return false;
			}
		}

		m_pfnSQLiteFreeTable = (SQLITE3_FREE_TABLE)GetProcAddress(g_hSQLiteDLL, "sqlite3_free_table");

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteTable::LoadRequiredLibrary()", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteStatement
*  Description    : CTOR CWardwizSQLiteStatement
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteStatement::CWardwizSQLiteStatement()
{
	m_pSQLiteDB = 0;
	m_pSQLiteStmt = 0;
}

/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary()
*  Description    : Load Required Library for SQLite
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardwizSQLiteStatement::LoadRequiredLibrary()
{
	bool bReturn = false;

	try
	{
		DWORD	dwRet = 0x00;
		CString	csDLLPath = L"";
		csDLLPath.Format(L"%s\\SQLITE3.DLL", GetModuleFilePath());

		if (!PathFileExists(csDLLPath))
		{
			return false;
		}

		if (!g_hSQLiteDLL)
		{
			g_hSQLiteDLL = LoadLibrary(csDLLPath);
			if (!g_hSQLiteDLL)
			{
				return false;
			}
		}

		m_pfnSQLiteChanges = (SQLITE3_CHANGES)GetProcAddress(g_hSQLiteDLL, "sqlite3_changes");
		m_pfnSQLiteReset = (SQLITE3_RESET)GetProcAddress(g_hSQLiteDLL, "sqlite3_reset");
		m_pfnSQLiteErrMsg = (SQLITE3_ERRMSG)GetProcAddress(g_hSQLiteDLL, "sqlite3_errmsg");
		m_pfnSQLiteBindText = (SQLITE3_BIND_TEXT)GetProcAddress(g_hSQLiteDLL, "sqlite3_bind_text");
		m_pfnSQLiteBindInt = (SQLITE3_BIND_INT)GetProcAddress(g_hSQLiteDLL, "sqlite3_bind_int");
		m_pfnSQLiteBindDouble = (SQLITE3_BIND_DOUBLE)GetProcAddress(g_hSQLiteDLL, "sqlite3_bind_double");
		m_pfnSQLiteBindBlob = (SQLITE3_BIND_BLOB)GetProcAddress(g_hSQLiteDLL, "sqlite3_bind_blob");
		m_pfnSQLiteBindNull = (SQLITE3_BIND_NULL)GetProcAddress(g_hSQLiteDLL, "sqlite3_bind_null");
		m_pfnSQLiteBindParamIndex = (SQLITE3_BIND_PARAMETER_INDEX)GetProcAddress(g_hSQLiteDLL, "sqlite3_bind_parameter_index");
		m_pfnSQLiteBindParamCount = (SQLITE3_BIND_PARAMETER_CONT)GetProcAddress(g_hSQLiteDLL, "sqlite3_bind_parameter_count");
		m_pfnSQLiteBindParamName = (SQLITE3_BIND_PARAMETER_NAME)GetProcAddress(g_hSQLiteDLL, "sqlite3_bind_parameter_name");
		m_pfnSQLiteStep = (SQLITE3_STEP)GetProcAddress(g_hSQLiteDLL, "sqlite3_step");
		m_pfnSQLiteFinalize = (SQLITE3_FINALIZE)GetProcAddress(g_hSQLiteDLL, "sqlite3_finalize");
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteStatement::LoadRequiredLibrary()", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteStatement
*  Description    : CTOR CWardwizSQLiteStatement
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteStatement::CWardwizSQLiteStatement(const CWardwizSQLiteStatement& rStatement)
{
	m_pSQLiteDB = rStatement.m_pSQLiteDB;
	m_pSQLiteStmt = rStatement.m_pSQLiteStmt;
	// Only one object can own VM
	const_cast<CWardwizSQLiteStatement&>(rStatement).m_pSQLiteStmt = 0;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteStatement
*  Description    : CTOR CWardwizSQLiteStatement
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteStatement::CWardwizSQLiteStatement(sqlite3* pDB, sqlite3_stmt* pVM)
{
	m_pSQLiteDB = pDB;
	m_pSQLiteStmt = pVM;
	LoadRequiredLibrary();
}

/***************************************************************************************************
*  Function Name  : ~CWardwizSQLiteStatement
*  Description    : DTOR CWardwizSQLiteStatement
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteStatement::~CWardwizSQLiteStatement()
{
	try
	{
		Finalize();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ~CWardwizSQLiteStatement", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : operator=
*  Description    : Overloaded  operator=
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteStatement& CWardwizSQLiteStatement::operator=(const CWardwizSQLiteStatement& rStatement)
{
	m_pSQLiteDB = rStatement.m_pSQLiteDB;
	m_pSQLiteStmt = rStatement.m_pSQLiteStmt;
	const_cast<CWardwizSQLiteStatement&>(rStatement).m_pSQLiteStmt = 0;
	return *this;
}

/***************************************************************************************************
*  Function Name  : ExecDML
*  Description    : Exec DML statement
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteStatement::ExecDML()
{
	CheckDB();
	CheckVM();

	const char* szError = 0;

	int nRet = m_pfnSQLiteStep(m_pSQLiteStmt);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = m_pfnSQLiteChanges(m_pSQLiteDB);

		nRet = m_pfnSQLiteReset(m_pSQLiteStmt);

		if (nRet != SQLITE_OK)
		{
			szError = m_pfnSQLiteErrMsg(m_pSQLiteDB);
			AddLogEntry(L"### Failed in CWardwizSQLiteStatement::ExecDML, Error: %s", A2BSTR(szError), 0, true, SECONDLEVEL);
		}

		return nRowsChanged;
	}
	else
	{
		nRet = m_pfnSQLiteReset(m_pSQLiteStmt);
		szError = m_pfnSQLiteErrMsg(m_pSQLiteDB);
		AddLogEntry(L"### Failed in CWardwizSQLiteStatement::ExecDML, Error: %s", A2BSTR(szError), 0, true, SECONDLEVEL);
	}
	return 0;
}


/***************************************************************************************************
*  Function Name  : ExecQuery
*  Description    : Execute Query
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteQuery CWardwizSQLiteStatement::ExecQuery()
{
	CheckDB();
	CheckVM();

	int nRet = m_pfnSQLiteStep(m_pSQLiteStmt);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CWardwizSQLiteQuery(m_pSQLiteDB, m_pSQLiteStmt, true/*eof*/, false);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CWardwizSQLiteQuery(m_pSQLiteDB, m_pSQLiteStmt, false/*eof*/, false);
	}
	else
	{

		nRet = m_pfnSQLiteReset(m_pSQLiteStmt);
		const char* szError = m_pfnSQLiteErrMsg(m_pSQLiteDB);
		AddLogEntry(L"### Failed in CWardwizSQLiteStatement::ExecQuery, Error: %s", A2BSTR(szError), 0, true, SECONDLEVEL);
	}
	return CWardwizSQLiteQuery();
}


/***************************************************************************************************
*  Function Name  : Bind
*  Description    : Bind value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Bind(int nParam, const char* szValue)
{
	CheckVM();

	int nRes = m_pfnSQLiteBindText(m_pSQLiteStmt, nParam, szValue, -1, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		AddLogEntry(L"### Error binding string param in CWardwizSQLiteStatement::Bind", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : Bind
*  Description    : Bind value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Bind(int nParam, const int nValue)
{
	CheckVM();

	int nRes = m_pfnSQLiteBindInt(m_pSQLiteStmt, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
		AddLogEntry(L"### Error binding int param in CWardwizSQLiteStatement::Bind", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : Bind
*  Description    : Bind value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Bind(int nParam, const double dValue)
{
	CheckVM();
	int nRes = m_pfnSQLiteBindDouble(m_pSQLiteStmt, nParam, dValue);

	if (nRes != SQLITE_OK)
	{
		AddLogEntry(L"### Error binding double param in CWardwizSQLiteStatement::Bind", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : Bind
*  Description    : Bind value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Bind(int nParam, const unsigned char* blobValue, int nLen)
{
	CheckVM();

	int nRes = m_pfnSQLiteBindBlob(m_pSQLiteStmt, nParam, (const void*)blobValue, nLen, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		AddLogEntry(L"### Error binding blob param in CWardwizSQLiteStatement::Bind", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : BindNull
*  Description    : Bind Null value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::BindNull(int nParam)
{
	CheckVM();

	int nRes = m_pfnSQLiteBindNull(m_pSQLiteStmt, nParam);

	if (nRes != SQLITE_OK)
	{
		AddLogEntry(L"### Error binding NULL param in CWardwizSQLiteStatement::BindNull", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : BindParameterIndex
*  Description    : Bind parameter by index
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardwizSQLiteStatement::BindParameterIndex(const char* szParam)
{
	CheckVM();

	int nParam = m_pfnSQLiteBindParamIndex(m_pSQLiteStmt, szParam);

	int nn = m_pfnSQLiteBindParamCount(m_pSQLiteStmt);

	const char* sz1 = m_pfnSQLiteBindParamName(m_pSQLiteStmt, 1);

	const char* sz2 = m_pfnSQLiteBindParamName(m_pSQLiteStmt, 2);

	if (!nParam)
	{
		char buf[128];
		sprintf_s(buf, "Parameter '%s' is not valid for this statement", szParam);
		AddLogEntry(L"### %s in CWardwizSQLiteStatement::BindParameterIndex", A2BSTR(buf), 0, true, SECONDLEVEL);
	}

	return nParam;
}

/***************************************************************************************************
*  Function Name  : Bind
*  Description    : Bind value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Bind(const char* szParam, const char* szValue)
{
	int nParam = BindParameterIndex(szParam);
	Bind(nParam, szValue);
}

/***************************************************************************************************
*  Function Name  : Bind
*  Description    : Bind value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Bind(const char* szParam, const int nValue)
{
	int nParam = BindParameterIndex(szParam);
	Bind(nParam, nValue);
}

/***************************************************************************************************
*  Function Name  : Bind
*  Description    : Bind value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Bind(const char* szParam, const double dwValue)
{
	int nParam = BindParameterIndex(szParam);
	Bind(nParam, dwValue);
}

/***************************************************************************************************
*  Function Name  : Bind
*  Description    : Bind value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Bind(const char* szParam, const unsigned char* blobValue, int nLen)
{
	int nParam = BindParameterIndex(szParam);
	Bind(nParam, blobValue, nLen);
}

/***************************************************************************************************
*  Function Name  : BindNull
*  Description    : Bind Null value to parameter
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::BindNull(const char* szParam)
{
	int nParam = BindParameterIndex(szParam);
	BindNull(nParam);
}

/***************************************************************************************************
*  Function Name  : Reset
*  Description    : Reset DB connection
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Reset()
{
	if (m_pSQLiteStmt)
	{
		int nRet = m_pfnSQLiteReset(m_pSQLiteStmt);

		if (nRet != SQLITE_OK)
		{
			const char* szError = m_pfnSQLiteErrMsg(m_pSQLiteDB);
			AddLogEntry(L"### Failed in CWardwizSQLiteStatement::Reset, Error: %s", A2BSTR(szError), 0, true, SECONDLEVEL);
		}
	}
}

/***************************************************************************************************
*  Function Name  : Finalize
*  Description    : Release acquired resources.
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::Finalize()
{
	if (!m_pSQLiteStmt)
		return;

	if (!m_pfnSQLiteFinalize)
		return;

	int nRet = m_pfnSQLiteFinalize(m_pSQLiteStmt);

	m_pSQLiteStmt = 0;

	if (nRet != SQLITE_OK)
	{
		const char* szError = m_pfnSQLiteErrMsg(m_pSQLiteDB);
		AddLogEntry(L"### Failed in CWardwizSQLiteStatement::Finalize, Error: %s", A2BSTR(szError), 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckDB
*  Description    : Check if DB connection is valid
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::CheckDB()
{
	if (m_pSQLiteDB == 0)
	{
		AddLogEntry(L"### Database not open in CWardwizSQLiteStatement::CheckDB", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckVM
*  Description    : Check if Statement is Valid
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteStatement::CheckVM()
{
	if (m_pSQLiteStmt == 0)
	{
		AddLogEntry(L"### Null Virtual Machine pointer in CWardwizSQLiteStatement::CheckVM", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CWardWizSQLiteDatabase
*  Description    : Default C'TOR CWardWizSQLiteDatabase
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardWizSQLiteDatabase::CWardWizSQLiteDatabase() 
{
	m_pSQLiteDB = 0;
	m_iBusyTimeoutMs = 60000; // 60 seconds
	LoadRequiredLibrary();
}


/***************************************************************************************************
*  Function Name  : CWardWizSQLiteDatabase
*  Description    : C'TOR CWardWizSQLiteDatabase 
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardWizSQLiteDatabase::CWardWizSQLiteDatabase(char* szFile) :pszDatabaseFileName(szFile)
{
	m_pSQLiteDB = 0;
	m_iBusyTimeoutMs = 60000; // 60 seconds
	LoadRequiredLibrary();
}

/***************************************************************************************************
*  Function Name  : CWardWizSetDBPath
*  Description    : Sets WardWizSQLiteDatabase
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardWizSQLiteDatabase::SetDataBaseFilePath(char* szFile) 
{
	//pszDatabaseFileName = (char*)malloc(sizeof(szFile) + 1);
	pszDatabaseFileName = szFile;

	//strcpy(pszDatabaseFileName ,szFile);
}
/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary
*  Description    : Load Required library for SQLite
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardWizSQLiteDatabase::LoadRequiredLibrary()
{
	bool bReturn = false;

	try
	{
		DWORD	dwRet = 0x00;
		CString	csDLLPath = L"";
#ifdef __SETUPDLL
		csDLLPath.Format(L"%s\\SQLITE3_32.DLL", GetWardWizPathFromRegistry());
#else
		csDLLPath.Format(L"%s\\SQLITE3.DLL", GetWardWizPathFromRegistry());
#endif

		if (!PathFileExists(csDLLPath))
		{
			return false;
		}

		if (!g_hSQLiteDLL)
		{
			g_hSQLiteDLL = LoadLibrary(csDLLPath);
			if (!g_hSQLiteDLL)
			{
				return false;
			}
		}

		m_pfnSQLiteOpen = (SQLITE3_OPEN)GetProcAddress(g_hSQLiteDLL, "sqlite3_open");
		m_pfnSQLiteClose = (SQLITE3_CLOSE)GetProcAddress(g_hSQLiteDLL, "sqlite3_close");
		m_pfnSQLiteExec = (SQLITE3_EXEC)GetProcAddress(g_hSQLiteDLL, "sqlite3_exec");
		m_pfnSQLiteChanges = (SQLITE3_CHANGES)GetProcAddress(g_hSQLiteDLL, "sqlite3_changes");
		m_pfnSQLiteStep = (SQLITE3_STEP)GetProcAddress(g_hSQLiteDLL, "sqlite3_step");
		m_pfnSQLiteFinalize = (SQLITE3_FINALIZE)GetProcAddress(g_hSQLiteDLL, "sqlite3_finalize");
		m_pfnSQLiteErrMsg = (SQLITE3_ERRMSG)GetProcAddress(g_hSQLiteDLL, "sqlite3_errmsg");
		m_pfnSQLiteGetTable = (SQLITE3_GET_TABLE)GetProcAddress(g_hSQLiteDLL, "sqlite3_get_table");
		m_pfnSQLiteLastInsertRowID = (SQLITE3_LAST_INSERT_ROWID)GetProcAddress(g_hSQLiteDLL, "sqlite3_last_insert_rowid");
		m_pfnSQLiteBusyTimeOut = (SQLITE_BUSY_TIMEOUT)GetProcAddress(g_hSQLiteDLL, "sqlite3_busy_timeout");
		m_pfnSQLitePrepareV2 = (SQLITE3_PREPARE_V2)GetProcAddress(g_hSQLiteDLL, "sqlite3_prepare_v2");
		m_pfnSQLiteAutoCommit = (SQLITE3_GET_AUTOCOMMIT)GetProcAddress(g_hSQLiteDLL, "sqlite3_get_autocommit");

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::LoadRequiredLibrary()", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CWardWizSQLiteDatabase
*  Description    : CTOR CWardWizSQLiteDatabase
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardWizSQLiteDatabase::CWardWizSQLiteDatabase(const CWardWizSQLiteDatabase& db)
{
	m_pSQLiteDB = db.m_pSQLiteDB;
	m_iBusyTimeoutMs = 60000; // 60 seconds
	LoadRequiredLibrary();
}

/***************************************************************************************************
*  Function Name  : CWardWizSQLiteDatabase
*  Description    : D'TOR CWardWizSQLiteDatabase
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardWizSQLiteDatabase::~CWardWizSQLiteDatabase()
{
	try
	{
		Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::~CWardwizSQLiteDatabase()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : operator=
*  Description    : Overloaded Operator
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardWizSQLiteDatabase& CWardWizSQLiteDatabase::operator=(const CWardWizSQLiteDatabase& db)
{
	m_pSQLiteDB = db.m_pSQLiteDB;
	m_iBusyTimeoutMs = 60000; // 60 seconds
	return *this;
}

/***************************************************************************************************
*  Function Name  : AlterWardwizSQLiteTables
*  Description    : Alter SQLite tables for Wardwiz
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 14-Oct-2016
****************************************************************************************************/
void CWardWizSQLiteDatabase::AlterWardwizSQLiteTables(const char* strTableName,const char* strColumnName,const char* strDatatype)
{
	__try
	{
		Open();

		char szSQL[512];
		sprintf_s(szSQL, "ALTER TABLE %s ADD COLUMN %s %s", strTableName, strColumnName, strDatatype);
		int iRet = ExecDMLWithNoException(szSQL);
		
		Close();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::AlterWardwizSQLiteTables", 0, 0, true, SECONDLEVEL);
	}
	return;
}
/***************************************************************************************************
*  Function Name  : CreateWardwizSQLiteTables
*  Description    : Create SQLite tables for Wardwiz
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardWizSQLiteDatabase::CreateWardwizSQLiteTables(DWORD dwProdID)
{
	__try
	{

		Open();
	
		if (!TableExists("Wardwiz_ScanSessionDetails"))
		{	
			ExecDML("CREATE TABLE [Wardwiz_ScanSessionDetails] (\
					[db_ScanSessionID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
					[db_ScanTypeId] INTEGER  NOT NULL,\
					[db_ScanSessionStartDate] DATE DEFAULT CURRENT_DATE NULL,\
					[db_ScanSessionStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
					[db_ScanSessionEndDate] DATE  NULL,\
					[db_ScanSessionEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
					[db_TotalFilesScanned] INTEGER  NULL,\
					[db_TotalThreatsFound] INTEGER  NULL,\
					[db_QuarantineOpt] INTEGER  NULL,\
					[db_IsHeuristicEnb] BOOLEAN  NULL,\
					[db_TotalThreatsCleaned] INTEGER  NULL\
					)");
		}
		if (!TableExists("Wardwiz_ScanDetails"))
		{
			ExecDML("CREATE TABLE [Wardwiz_ScanDetails] (\
				[db_ScanID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
				[db_ScanSessionID] INTEGER  NOT NULL,\
				[db_ScanStartDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_ScanStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_ScanEndDate] DATE  NULL,\
				[db_ScanEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_VirusName] NVARCHAR(256)  NULL,\
				[db_ThreatPath] NVARCHAR(512)  NULL,\
				[db_ActionTaken] NVARCHAR(128)  NULL,\
				[db_EmailSubject] NVARCHAR(512) NULL\
				)");
		}
		else
		{
			int iRet = ExecDML("ALTER TABLE Wardwiz_ScanDetails ADD COLUMN [db_EmailSubject] NVARCHAR(512) NULL");
			AddLogEntry(L">>> Alter table for added db_EmailSubject in Vibranium_ScanDetails", 0, 0, true, ZEROLEVEL);
		}

		if (!TableExists("Wardwiz_ScanTypeMaster"))
		{
			ExecDML("CREATE TABLE [Wardwiz_ScanTypeMaster] (\
						 			[db_ScanTypeId] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,\
									[db_ScanType] NVARCHAR(128)  NULL,\
									[db_ScanTypeDecsription] NVARCHAR(256)  NULL\
								)");
		
			// Now enter Default values in this table..
			const char* csQuery = "INSERT INTO Wardwiz_ScanTypeMaster \
						  			SELECT 0 AS 'db_ScanTypeId', 'FULLSCAN' AS 'db_ScanType', 'FULLSCAN' AS 'db_ScanTypeDescription'\
									UNION ALL SELECT 1 AS 'db_ScanTypeId', 'CUSTOMSCAN' AS 'db_ScanType', 'CUSTOMSCAN' AS 'db_ScanTypeDescription'\
									UNION ALL SELECT 2 AS 'db_ScanTypeId', 'QUICKSCAN' AS 'db_ScanType', 'QUICKSCAN' AS 'db_ScanTypeDescription'\
									UNION ALL SELECT 3 AS 'db_ScanTypeId', 'USBSCAN' AS 'db_ScanType', 'USBSCAN' AS 'db_ScanTypeDescription'\
									UNION ALL SELECT 4 AS 'db_ScanTypeId', 'USBDETECT' AS 'db_ScanType', 'USBDETECT' AS 'db_ScanTypeDescription'\
									UNION ALL SELECT 5 AS 'db_ScanTypeId', 'ANTIROOTKITSCAN' AS 'db_ScanType', 'ANTIROOTKITSCAN' AS 'db_ScanTypeDescription'\
									UNION ALL SELECT 6 AS 'db_ScanTypeId', 'ACTIVESCAN' AS 'db_ScanType', 'ANTIROOTKITSCAN' AS 'db_ScanTypeDescription'\
									UNION ALL SELECT 11 AS 'db_ScanTypeId', 'BOOTTIMESCAN' AS 'db_ScanType', 'BOOTTIMESCAN' AS 'db_ScanTypeDescription'\
									UNION ALL SELECT 12 AS 'db_ScanTypeId', 'EMAILSCAN' AS 'db_ScanType', 'EMAILSCAN' AS 'db_ScanTypeDescription'";
			ExecDML(csQuery);
		}
		else
		{
			char szSQL[256] = { 0 };
			sprintf_s(szSQL, "select count(*) from Wardwiz_ScanTypeMaster;");
			int iRet = ExecScalar(szSQL);
			if (iRet <= 7)
			{
				const char* csQuery = "INSERT INTO Wardwiz_ScanTypeMaster \
									  										SELECT 11 AS 'db_ScanTypeId','BOOTTIMESCAN' AS 'db_ScanType', 'BOOTTIMESCAN' AS 'db_ScanTypeDescription'";
				ExecDML(csQuery);
				AddLogEntry(L">>> BOOTTIMESCAN entry added in Wardwiz_ScanTypeMaster", 0, 0, true, SECONDLEVEL);
			}
			if (iRet <= 8)
			{
				const char* csQuery = "INSERT INTO Wardwiz_ScanTypeMaster \
									  	SELECT 12 AS 'db_ScanTypeId','EMAILSCAN' AS 'db_ScanType', 'EMAILSCAN' AS 'db_ScanTypeDescription'";
				ExecDML(csQuery);
				AddLogEntry(L">>> EMAILSCAN entry added in Wardwiz_ScanTypeMaster", 0, 0, true, SECONDLEVEL);
			}
		}

		if (!TableExists("Wardwiz_AutorunScanSessionDetails"))
		{
			ExecDML("CREATE TABLE [Wardwiz_AutorunScanSessionDetails] (\
				[db_ScanSessionID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
				[db_ScanSessionStartDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_ScanSessionStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_ScanSessionEndDate] DATE NULL,\
				[db_ScanSessionEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_ThreatsFoundCount] INTEGER,\
				[db_FilesScannedCount] INTEGER,\
				[db_FilesCleanedCount] INTEGER\
				); ");
		}

		if (!TableExists("Wardwiz_AutorunScanDetails"))
		{
			ExecDML("CREATE TABLE [Wardwiz_AutorunScanDetails] (\
				[db_AutorunScanId] INTEGER NOT NULL PRIMARY KEY,\
				[db_ScanSessionID] INTEGER,\
				[db_ScanStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP,\
				[db_ScanStartDate] DATE DEFAULT CURRENT_DATE,\
				[db_ScanEndTime] TIME DEFAULT CURRENT_TIMESTAMP,\
				[db_ScanEndDate] DATE DEFAULT CURRENT_DATE,\
				[db_VirusName][NVARCHAR2(128)],\
				[db_ThreatPath][NVARCHAR2(512)],\
				[db_ActionTaken][NVARCHAR2(128)]\
				); ");
		}

		if (!TableExists("Wardwiz_TempFilesCleanerSessionDetails"))
		{
			ExecDML(" CREATE TABLE[Wardwiz_TempFilesCleanerSessionDetails](\
				[db_TempCleanerSessionId] INTEGER  NOT NULL PRIMARY KEY,\
				[db_CleanerSessionStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_CleanerSessionEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_CleanerSessionStartDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_CleanerSessionEndDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_FilesScanCount] INTEGER  NULL,\
				[db_FilesCleanCount] INTEGER  NULL\
				);");
		}

		if (!TableExists("Wardwiz_TempFilesCleanerDetails"))
		{
			ExecDML("CREATE TABLE [Wardwiz_TempFilesCleanerDetails] (\
				[db_TempCleanerId] INTEGER NOT NULL PRIMARY KEY,\
				[db_TempCleanerSessionId] INTEGER,\
				[db_CleanerStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_CleanerEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_CleanerStartDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_CleanerEndDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_VirusName][NVARCHAR2(128)],\
				[db_ThreatPath][NVARCHAR2(512)],\
				[db_ActionTaken][NVARCHAR2(128)]\
				);");
		}

		if (!TableExists("Wardwiz_USBVaccinatorSessionDetails"))
		{
			ExecDML("CREATE TABLE [Wardwiz_USBVaccinatorSessionDetails] (\
				[db_USBVaccinatorSessionId] INTEGER  NOT NULL PRIMARY KEY,\
				[db_USBSessionStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_USBSessionEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_USBSessionStartDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_USBSessionEndDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_FilesScanCount] INTEGER  NULL,\
				[db_FilesUnHideCount] INTEGER  NULL,\
				[db_VaccinatorType] NVARCHAR(50)  NULL,\
				[db_USBSessionPathSelected] NVARCHAR(256)  NULL\
				); ");
		}
		else
		{
			//check if the required column which is newly added exists..
			
		}

		if (!TableExists("Wardwiz_USBVaccinatorDetails"))
		{
			ExecDML("CREATE TABLE [Wardwiz_USBVaccinatorDetails] (\
				[db_USBVaccinatorId] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
				[db_USBVaccinatorSessionId] INTEGER,\
				[db_VaccinatorStartDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_VaccinatorEndDate] DATE DEFAULT CURRENT_DATE NULL,\
				[db_VaccinatorStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_VaccinatorEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
				[db_VaccinatedDrive] NVARCHAR(512)  NULL\
				); ");
		}

		if (!TableExists("Wardwiz_UpdatesMaster"))
		{
			ExecDML("CREATE TABLE [Wardwiz_UpdatesMaster] (\
						 		  [db_UpdateId] INTEGER  NOT NULL PRIMARY KEY,\
								  [db_UpdateStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
								  [db_UpdateEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
								  [db_UpdateStartDate] DATE DEFAULT CURRENT_DATE NULL,\
								  [db_UpdateEndDate] DATE DEFAULT CURRENT_DATE NULL,\
								  [db_FilesDownloadCount] INTEGER  NULL,\
								  [db_DownloadFileSize] REAL  NULL\
								)");
		}

		if (!TableExists("Wardwiz_RegistryOptimizerDetails"))
		{
			ExecDML("CREATE TABLE[Wardwiz_RegistryOptimizerDetails]( \
								[db_RegistryScanId] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,\
								[db_ScanStartTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
								[db_ScanStartDate] DATE DEFAULT CURRENT_DATE NULL,\
								[db_ScanEndTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
								[db_ScanEndDate] DATE DEFAULT CURRENT_DATE NULL,\
								[db_FileScanCount] INTEGER  NULL,\
								[db_ThreatFoundCount] INTEGER  NULL,\
								[db_ComActivexEnabled] BOOLEAN  NULL,\
								[db_UninstallEntriesEnabled] BOOLEAN  NULL,\
								[db_FontEntriesEnabled] BOOLEAN  NULL,\
								[db_SharedDllsEnabled] BOOLEAN  NULL,\
								[db_ApplicationPathsEnabled] BOOLEAN  NULL,\
								[db_HelpFileInfoEnabled] BOOLEAN  NULL,\
								[db_WindowsStartupItemsEnabled] BOOLEAN  NULL,\
								[db_WindowsServicesEnabled] BOOLEAN  NULL,\
								[db_InvalidExtensionsEnabled] BOOLEAN  NULL,\
								[db_RootkitsEnabled] BOOLEAN  NULL,\
								[db_RogueAppsEnabled] BOOLEAN  NULL,\
								[db_WormsEnabled] BOOLEAN  NULL,\
								[db_SpywareThreatsEnabled] BOOLEAN  NULL,\
								[db_AdwareThreatsEnabled] BOOLEAN  NULL,\
								[db_KeyLoggersEnabled] BOOLEAN  NULL,\
								[db_BHOEnabled] BOOLEAN  NULL,\
								[db_ExplorerEntriesEnabled] BOOLEAN  NULL,\
								[db_InternetExpEntriesEnabled] BOOLEAN  NULL\
								)");
		}

		if (!TableExists("Wardwiz_ReportOptions"))
		{
			ExecDML("CREATE TABLE[Wardwiz_ReportOptions](\
					[db_ReportTypeId] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,\
					[db_ReportType] NVARCHAR(128)  NULL,\
					[db_IncludeForDisplay] BOOLEAN DEFAULT 'true' NULL\
				)");


			if (dwProdID == 0x04)
			{
				//Now insert default data for this table..
				ExecDML(" INSERT INTO Wardwiz_ReportOptions\
					SELECT null AS 'db_ReportTypeId', 'Full Scan'        AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Quick Scan'         AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Custom Scan'          AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Active Scan'        AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Antirootkit Scan'   AS 'db_ReportType', 'false' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Registry Optimizer' AS 'db_ReportType', 'false' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Updates'            AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Temp file Cleaner'  AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'USB Vaccinator'     AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Autorun scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Boottime scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Email scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'");
			}
			else if (dwProdID == 0x01)
			{
				//Now insert default data for this table..
				ExecDML(" INSERT INTO Wardwiz_ReportOptions\
					SELECT null AS 'db_ReportTypeId', 'Full Scan'        AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Quick Scan'         AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Custom Scan'          AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Active Scan'        AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Antirootkit Scan'   AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Registry Optimizer' AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Updates'            AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Temp file Cleaner'  AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'USB Vaccinator'     AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Autorun scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Boottime scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Email scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'");
			}
			else
			{
				//Now insert default data for this table..
				ExecDML(" INSERT INTO Wardwiz_ReportOptions\
					SELECT null AS 'db_ReportTypeId', 'Full Scan'        AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Quick Scan'         AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Custom Scan'          AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Active Scan'        AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Antirootkit Scan'   AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Registry Optimizer' AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Updates'            AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Temp file Cleaner'  AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'USB Vaccinator'     AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Autorun scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Boottime scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Email scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Firewall'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'\
					UNION ALL SELECT null AS 'db_ReportTypeId', 'Parental Control'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'");
			}
		}
		else
		{
			char szSQL[256] = { 0 };
			sprintf_s(szSQL, "select count(*) from Wardwiz_ReportOptions;");
			int iRet = ExecScalar(szSQL);
			if (iRet <= 10)
			{
				ExecDML(" INSERT INTO Wardwiz_ReportOptions\
													SELECT 11 AS 'db_ReportTypeId', 'Boottime scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'");
				AddLogEntry(L">>> Boottime scanner entry added in Wardwiz_ReportOptions", 0, 0, true, SECONDLEVEL);
			}
			if (iRet <= 11)
			{
				ExecDML(" INSERT INTO Wardwiz_ReportOptions\
							SELECT 12 AS 'db_ReportTypeId', 'Email scanner'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'");
				AddLogEntry(L">>> Email scanner entry added in Wardwiz_ReportOptions", 0, 0, true, SECONDLEVEL);
			}
			if (iRet <= 12)
			{
				ExecDML(" INSERT INTO Wardwiz_ReportOptions\
													SELECT 13 AS 'db_ReportTypeId', 'Firewall'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'");
				AddLogEntry(L">>> Firewall entry added in Wardwiz_ReportOptions", 0, 0, true, SECONDLEVEL);
			}
			if (iRet <= 13)
			{
				ExecDML(" INSERT INTO Wardwiz_ReportOptions\
													SELECT 14 AS 'db_ReportTypeId', 'Parental Control'    AS 'db_ReportType', 'true' AS 'db_IncludeForDisplay'");
				AddLogEntry(L">>> Parental Control entry added in Wardwiz_ReportOptions", 0, 0, true, SECONDLEVEL);
			}
		}
		
		Close();
		
		AlterWardwizSQLiteTables("Wardwiz_ScanSessionDetails","db_TotalThreatsCleaned", "INTEGER");
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::CreateWardwizSQLiteTables", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : Open
*  Description    : Open database connection
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardWizSQLiteDatabase::Open()
{
	if (!m_pfnSQLiteOpen)
		return;
	
	int nRet = m_pfnSQLiteOpen(pszDatabaseFileName, &m_pSQLiteDB);

	if (nRet != SQLITE_OK)
	{
		const char* szError = m_pfnSQLiteErrMsg(m_pSQLiteDB);
		AddLogEntry(L"### Failed in CWardwizSQLiteDatabase::Open, Error: %s", A2BSTR(szError), 0, true, SECONDLEVEL);
	}

	setBusyTimeout(m_iBusyTimeoutMs);
}

/***************************************************************************************************
*  Function Name  : Close
*  Description    : Close database connection
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardWizSQLiteDatabase::Close()
{
	if (!m_pSQLiteDB)
		return;

	if (m_pSQLiteDB)
	{
		if (!m_pfnSQLiteClose)
		{
			return;
		}

		if (m_pfnSQLiteClose(m_pSQLiteDB) == SQLITE_OK)
		{
			m_pSQLiteDB = 0;
		}
		else
		{
			AddLogEntry(L"### Unable to close database in CWardwizSQLiteDatabase::Close", 0, 0, true, SECONDLEVEL);
		}
	}
}

/***************************************************************************************************
*  Function Name  : CompileSQLStatement
*  Description    : Compile input SQL Statement
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteStatement CWardWizSQLiteDatabase::CompileSQLStatement(const char* szSQL)
{
	if (!CheckDataBase())
	{
		return CWardwizSQLiteStatement(m_pSQLiteDB, NULL);;
	}

	sqlite3_stmt* pObjStmt = Compile(szSQL);
	return CWardwizSQLiteStatement(m_pSQLiteDB, pObjStmt);
}

/***************************************************************************************************
*  Function Name  : TableExists
*  Description    : Check if Table exists.
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardWizSQLiteDatabase::TableExists(const char* szTable)
{
	char szSQL[256];
	sprintf_s(szSQL,"select count(*) from sqlite_master where type='table' and name='%s'",
		szTable);
	int iRet = ExecScalar(szSQL);

	return (iRet > 0);
}

/***************************************************************************************************
*  Function Name  : ExecDMLWithNoException
*  Description    : Execute a DML statement without throwing exception.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardWizSQLiteDatabase::ExecDMLWithNoException(const char* szSQL)
{
	__try
	{
		if (!CheckDataBase())
		{
			return 0;
		}

		char* szError = 0;


		if (!m_pfnSQLiteExec)
			return 0;

		int nRet = m_pfnSQLiteExec(m_pSQLiteDB, szSQL, 0, 0, &szError);

		if (nRet == SQLITE_OK)
		{
			return m_pfnSQLiteChanges(m_pSQLiteDB);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ExecDML
*  Description    : Execute a DML statement
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardWizSQLiteDatabase::ExecDML(const char* szSQL)
{
	__try
	{
		if (!CheckDataBase())
		{
			return 0;
		}

		char* szError = 0;


		if (!m_pfnSQLiteExec)
			return 0;

		int nRet = m_pfnSQLiteExec(m_pSQLiteDB, szSQL, 0, 0, &szError);

		if (nRet == SQLITE_OK)
		{
			return m_pfnSQLiteChanges(m_pSQLiteDB);
		}
		else
		{
			//AddLogEntry(L"### Failed in CWardwizSQLiteDatabase::ExecDML, Query: %s, Error: %s", A2BSTR(szSQL), A2BSTR(szError), true, SECONDLEVEL);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::ExecDML", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ExecQuery
*  Description    : Exec Query
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteQuery CWardWizSQLiteDatabase::ExecQuery(const char* szSQL)
{
	if (!CheckDataBase())
	{
		return CWardwizSQLiteQuery();
	}

	sqlite3_stmt* pObjStmt = Compile(szSQL);

	int nRet = m_pfnSQLiteStep(pObjStmt);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CWardwizSQLiteQuery(m_pSQLiteDB, pObjStmt, true/*eof*/);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CWardwizSQLiteQuery(m_pSQLiteDB, pObjStmt, false/*eof*/);
	}
	else
	{
		nRet = m_pfnSQLiteFinalize(pObjStmt);
		const char* szError = m_pfnSQLiteErrMsg(m_pSQLiteDB);
		AddLogEntry(L"### Failed in CWardwizSQLiteDatabase::ExecQuery, Query: %s, Error: %s", A2BSTR(szSQL), A2BSTR(szError), true, SECONDLEVEL);
		return CWardwizSQLiteQuery();
	}
}

/***************************************************************************************************
*  Function Name  : ExecScalar
*  Description    : Execute Scalar Query
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
int CWardWizSQLiteDatabase::ExecScalar(const char* szSQL, int nNullValue/*=0*/)
{
	int iRet = 0;
	try
	{
		CWardwizSQLiteQuery objQuery = ExecQuery(szSQL);

		if (objQuery.IsEOF() || objQuery.GetNumFields() < 1)
		{
			AddLogEntry(L"### Invalid scalar query, Query: %s", A2BSTR(szSQL), 0, true, SECONDLEVEL);
		}
		iRet = objQuery.GetIntField(0, nNullValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::ExecScalar, Query: %s", CA2T(szSQL), 0, true, SECONDLEVEL);
	}
	return iRet;
}

/***************************************************************************************************
*  Function Name  : GetTable
*  Description    : Get table details
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteTable CWardWizSQLiteDatabase::GetTable(const char* szSQL)
{
	char* szError = 0;
	char** paszResults = 0;
	int nRet;
	int nRows(0);
	int nCols(0);

	try
	{
		if (!CheckDataBase())
		{
			return CWardwizSQLiteTable();
		}
		nRet = m_pfnSQLiteGetTable(m_pSQLiteDB, szSQL, &paszResults, &nRows, &nCols, &szError);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::GetTable, Query: %s", CA2T(szSQL), 0, true, SECONDLEVEL);
	}

	if (nRet == SQLITE_OK)
	{
		return CWardwizSQLiteTable(paszResults, nRows, nCols);
	}
	else
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::GetTable, Query: %s, Error: %s", A2BSTR(szSQL), A2BSTR(szError), true, ZEROLEVEL);
		return CWardwizSQLiteTable();
	}
}

/***************************************************************************************************
*  Function Name  : GetLastRowId
*  Description    : Get Last row Id from specific table
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
sqlite_int64 CWardWizSQLiteDatabase::GetLastRowId()
{
	__try
	{
		if (m_pfnSQLiteLastInsertRowID == NULL)
		{
			return 0;
		}

		return m_pfnSQLiteLastInsertRowID(m_pSQLiteDB);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::GetLastRowId", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : setBusyTimeout
*  Description    : Set Busy timeout time.
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardWizSQLiteDatabase::setBusyTimeout(int nMillisecs)
{
	__try
	{
		if (m_pfnSQLiteBusyTimeOut == NULL)
		{
			return;
		}

		m_iBusyTimeoutMs = nMillisecs;
		m_pfnSQLiteBusyTimeOut(m_pSQLiteDB, m_iBusyTimeoutMs);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::setBusyTimeout", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckDataBase
*  Description    : Check if Database connect is valid.
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardWizSQLiteDatabase::CheckDataBase()
{
	if (!m_pSQLiteDB)
	{
		AddLogEntry(L"### Database not open in CWardwizSQLiteDatabase::CheckDataBase", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : Compile
*  Description    : Compile SQL statement.
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
sqlite3_stmt* CWardWizSQLiteDatabase::Compile(const char* szSQL)
{
	const char* szTail = 0;
	sqlite3_stmt* pobjSQLiteStmt = NULL;
	int nRet = SQLITE_OK;
	__try
	{
		if (!CheckDataBase())
		{
			return pobjSQLiteStmt;
		}

		int nRet = m_pfnSQLitePrepareV2(m_pSQLiteDB, szSQL, -1, &pobjSQLiteStmt, &szTail);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::Compile", 0, 0, true, SECONDLEVEL);
		return pobjSQLiteStmt;
	}

	if (nRet != SQLITE_OK)
	{
		const char* szError = m_pfnSQLiteErrMsg(m_pSQLiteDB);
		AddLogEntry(L"### CWardwizSQLiteDatabase::Compile, Query: %s, Error: %s", A2BSTR(szSQL), A2BSTR(szError), true, SECONDLEVEL);
	}
	return pobjSQLiteStmt;
}

/***************************************************************************************************
*  Function Name  : IsAutoCommitOn
*  Description    : Check if AutoCommit is ON
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool CWardWizSQLiteDatabase::IsAutoCommitOn()
{
	bool bReturn = false;
	__try
	{
		if (!CheckDataBase())
		{
			return bReturn;
		}

		bReturn = m_pfnSQLiteAutoCommit(m_pSQLiteDB) ? true : false;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteDatabase::IsAutoCommitOn", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CWardwizSQLiteBuffer
*  Description    : CTor for SQLite Buffer
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteBuffer::CWardwizSQLiteBuffer()
{
	m_pBuf = 0;
}

/***************************************************************************************************
*  Function Name  : ~CWardwizSQLiteBuffer
*  Description    : DTor for SQLite Buffer
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
CWardwizSQLiteBuffer::~CWardwizSQLiteBuffer()
{
	Clear();
}

/***************************************************************************************************
*  Function Name  : Clear()
*  Description    : Clear buffer
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
void CWardwizSQLiteBuffer::Clear()
{
	if (m_pBuf)
	{
		m_pfnSQLiteFree(m_pBuf);
		m_pBuf = 0;
	}
}

/***************************************************************************************************
*  Function Name  : Format
*  Description    : Format the inout buffer.
*  Author Name    : Gayatri A.
*  SR_NO
*  Date           : 16-Aug-2016
****************************************************************************************************/
const char* CWardwizSQLiteBuffer::Format(const char* szFormat, ...)
{
	__try
	{
		Clear();
		va_list va;
		va_start(va, szFormat);
		m_pBuf = m_pfnSQLiteVmPrintf(szFormat, va);
		va_end(va);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizSQLiteBuffer::Format", 0, 0, true, SECONDLEVEL);
	}
	return m_pBuf;
}
