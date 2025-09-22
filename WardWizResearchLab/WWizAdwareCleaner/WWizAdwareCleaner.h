
// WWizAdwareCleaner.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "sqlite3.h"

typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
typedef  int(*SQLITE3_PREPARE)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
typedef  int(*SQLITE3_CLOSE)(sqlite3 *);

// CWWizAdwareCleanerApp:
// See WWizAdwareCleaner.cpp for the implementation of this class
//

class CWWizAdwareCleanerApp : public CWinApp
{
public:
	CWWizAdwareCleanerApp();

// Overrides
public:
	virtual BOOL InitInstance();
	HANDLE	m_hMutexHandle;
	bool SingleInstanceCheck();
	CString GetModuleFilePath();
	void OnCopyFolder(TCHAR* szSourceString, TCHAR* szDestString);
	bool CheckForReqdFiles();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWWizAdwareCleanerApp theApp;