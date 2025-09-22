#pragma once
#include "stdafx.h"
#include <vector>

typedef std::vector<WIN32_FIND_DATA> VECTORFINDDATA;

class CWardWizDumpCreater
{
public:
	CWardWizDumpCreater(void);
	virtual ~CWardWizDumpCreater(void);

	static bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
	static bool GetWardWizPathFromRegistry(TCHAR *pszModulePath, DWORD dwSize);
	static void CreateMiniDump( EXCEPTION_POINTERS* pep );
	static bool GetProductRegistryKey(TCHAR *szRegKeyValue, DWORD dwSize);
	static void EnumFolder(LPCTSTR pstr);
	static bool DeleteOldDumpFiles();
	static bool DeleteOlderDumps();

	static DWORD EnumFolder( TCHAR *pFolderPath );
public:
	//Vector to find older files.
	static VECTORFINDDATA m_vectorFileData;
};

