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
------------------------------------------------------------------------------------------------------------
							*** SQL statement to create VBEXCLUDE.DB *** 
------------------------------------------------------------------------------------------------------------
				CREATE TABLE WWIZEXCLUDELST(NAME VARCHAR (20) NOT NULL, ISUBFOL INT NOT NULL);

			DUMMY:
			INSERT INTO WWIZEXCLUDELST (NAME, ISUBFOL) VALUES ('C:\Program Files', 1);
			INSERT INTO WWIZEXCLUDELST (NAME, ISUBFOL) VALUES ('D:\zv', 0);
			INSERT INTO WWIZEXCLUDELST (NAME, ISUBFOL) VALUES ('D:\Other', 0);

***********************************************************************************************************/
#include "StdAfx.h"
#include "ExcludeFilesFolders.h"
#include "WardWizDatabaseInterface.h"

#define		WRDWIZEXCLUDECLONEDB			L"VBEXCLUDECLONE.DB"
#define		WRDWIZEXTEXCLUDECLONEDB			L"VBEXCLUDEEXT.DB"

/***************************************************************************************************
*  Function Name  : CExcludeFilesFolders
*  Description    : Const'r
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
CExcludeFilesFolders::CExcludeFilesFolders() :
m_hSQLiteDLL(NULL)
, m_bISDBLoaded(false)
, m_bISDBLoaded4FileExt(false)
{
	LoadRequiredLibrary();
}

/***************************************************************************************************
*  Function Name  : ~CExcludeFilesFolders
*  Description    : Dest'r
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
CExcludeFilesFolders::~CExcludeFilesFolders()
{
	if (m_hSQLiteDLL != NULL)
	{
		FreeLibrary(m_hSQLiteDLL);
		m_hSQLiteDLL = NULL;
	}
}

/***************************************************************************************************
*  Function Name  : LoadRequiredLibrary
*  Description    : Function which load required DLL files
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
bool CExcludeFilesFolders::LoadRequiredLibrary()
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

		if (!m_hSQLiteDLL)
		{
			m_hSQLiteDLL = LoadLibrary(csDLLPath);
			if (!m_hSQLiteDLL)
			{
				AddLogEntry(L"### Failed to LoadLibrary [%s]", csDLLPath, 0, true, SECONDLEVEL);
				return false;
			}
		}

		m_pSQliteOpen = (SQLITE3_OPEN)GetProcAddress(m_hSQLiteDLL, "sqlite3_open");
		m_pSQLitePrepare = (SQLITE3_PREPARE)GetProcAddress(m_hSQLiteDLL, "sqlite3_prepare");
		m_pSQLiteColumnCount = (SQLITE3_COLUMN_COUNT)GetProcAddress(m_hSQLiteDLL, "sqlite3_column_count");
		m_pSQLiteStep = (SQLITE3_STEP)GetProcAddress(m_hSQLiteDLL, "sqlite3_step");
		m_pSQLiteColumnText = (SQLITE3_COLUMN_TEXT)GetProcAddress(m_hSQLiteDLL, "sqlite3_column_text");
		m_pSQLiteClose = (SQLITE3_CLOSE)GetProcAddress(m_hSQLiteDLL, "sqlite3_close");

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadExcludeDB
*  Description    : Function which loads Excluded folders and files DB in memory from file.
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
bool CExcludeFilesFolders::LoadExcludeDB()
{
	try
	{
		GetRecords();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::LoadExcludeDB", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : LoadExcludeDB4FileExt
*  Description    : Function which loads Excluded files extensions DB in memory from file.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 26 Sep 2018
****************************************************************************************************/
bool CExcludeFilesFolders::LoadExcludeDB4FileExt()
{
	try
	{
		GetRecords4FileExtSEH();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::LoadExcludeDB4FileExt", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReloadExcludeDB
*  Description    : Function to reload exclude DB
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
void CExcludeFilesFolders::ReloadExcludeDB()
{
	GetRecordsSEH();
}

/***************************************************************************************************
*  Function Name  : ISExcludedPath
*  Description    : Function which checks input file path in exlude list and returns
					@true, if found
					@else false.
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
bool CExcludeFilesFolders::ISExcludedPath(LPTSTR lpszPath, bool &bISSubFolderExcluded)
{
	bool bReturn = false;
	try
	{
		if (!lpszPath)
		{
			return bReturn;
		}

		//Load here Exclude list if not loaded.
		if (!m_bISDBLoaded)
		{
			if (!LoadExcludeDB())
			{
				return bReturn;
			}
			m_bISDBLoaded = true;
		}

		EXCLUDELISTMAP::iterator vExCludeIter;
		for (vExCludeIter = m_vExludeList.begin(); vExCludeIter != m_vExludeList.end(); vExCludeIter++)
		{
			TCHAR szTempFileName[MAX_PATH] = { 0 };
			wcscpy_s(szTempFileName, (*vExCludeIter).szData);
			_wcslwr(lpszPath);
			
			if ((szTempFileName[0x00] == lpszPath[0x00]))
			{
				BYTE lpHash = vExCludeIter->byType;
				if (lpHash == '1')//flag to check for subfolders.
				{
					if (ISSubFilePath(lpszPath, szTempFileName))
					{
						bReturn = true;
						bISSubFolderExcluded = true;
						break;
					}
				}
				else
				{
					//Check here for Parent directory if its file
					if (!PathIsDirectory(lpszPath) && PathIsDirectory(szTempFileName))
					{
						TCHAR tszDirPath[MAX_FILE_PATH_LENGTH] = { 0 };
						wcscpy(tszDirPath, lpszPath);
						TCHAR *tszDir = wcsrchr(tszDirPath, '\\');
						tszDir[0] = '\0';
						if (tszDirPath != NULL)
						{
							if (CompareStringW(LOCALE_SYSTEM_DEFAULT, LINGUISTIC_IGNORECASE, szTempFileName, -1, tszDirPath, -1) == CSTR_EQUAL)
							{
								bReturn = true;
								break;
							}
						}
					}
					else
					{
						if (CompareStringW(LOCALE_SYSTEM_DEFAULT, LINGUISTIC_IGNORECASE, szTempFileName, -1, lpszPath, -1) == CSTR_EQUAL)
						{
							bReturn = true;
							break;
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::ISExcludedPath, path: %s", lpszPath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ISExcludedFileExt
*  Description    : Function which checks input file path in exlude list and returns
@true, if found
@else false.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 26 Sep 2018
****************************************************************************************************/
bool CExcludeFilesFolders::ISExcludedFileExt(LPTSTR lpszFileExt)
{
	bool bReturn = false;
	try
	{
		if (!lpszFileExt)
		{
			return bReturn;
		}

		//Load here Exclude file extension list if not loaded.
		if (!m_bISDBLoaded4FileExt)
		{
			if (!LoadExcludeDB4FileExt())
			{
				return bReturn;
			}
			m_bISDBLoaded4FileExt = true;
		}

		EXCLUDELISTFILEEXTMAP::iterator vExCludeIter;
		for (vExCludeIter = m_vExcludeFileExtList.begin(); vExCludeIter != m_vExcludeFileExtList.end(); vExCludeIter++)
		{
			if (lpszFileExt != NULL)
			{
				if (CompareStringW(LOCALE_SYSTEM_DEFAULT, LINGUISTIC_IGNORECASE, lpszFileExt, -1, (*vExCludeIter).szFileExt, -1) == CSTR_EQUAL)
				{
					bReturn = true;
					break;
				}
			}			
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::ISExcludedFileExt, Ext: %s", lpszFileExt, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : GetRecords
*  Description    : Function which get records from AKEXCLUDELST table which is present in VBEXCLUDE.DB
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
void CExcludeFilesFolders::GetRecords()
{
	__try
	{
		GetRecordsSEH();

		//Load custom folder Exclusion
		LoadCustExcPaths();
		
		//Load exclude extensions
		LoadExcludeDB4FileExt();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::GetRecords", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetRecords
*  Description    : Function which get records from AKEXCLUDELST table which is present in VBEXCLUDE.DB
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
void CExcludeFilesFolders::GetRecordsSEH()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBEXCLUDE.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from WWIZEXCLUDELST;");

		Sleep(20);

		//Clear here loaded exclude strings
		m_vExludeList.clear();

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);
			STRUCTEXLCUDELIST szExcludeList;
			
			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}

			//to read unicode string from db
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(1), -1, NULL, 0);
			wchar_t *wstrFileName = new wchar_t[wchars_num];
			if (wstrFileName == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CExcludeFilesFolders::GetRecordsSEH", 0, 0, true, SECONDLEVEL);
				continue;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(1), -1, wstrFileName, wchars_num);

			wcscpy_s(szExcludeList.szData, wstrFileName);
			delete[] wstrFileName;

			_wcslwr(szExcludeList.szData);

			if (qResult.GetFieldIsNull(2))
			{
				continue;
			}

			char szISSubFol[10] = { 0 };
			strcpy_s(szISSubFol, qResult.GetFieldValue(2));
			szExcludeList.byType = szISSubFol[0];
		
			m_vExludeList.push_back(szExcludeList);
		}
		
		dbSQlite.Close();
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::GetRecordsSEH, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::GetRecordsSEH", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetRecords4FileExtSEH
*  Description    : Function which get records from AKEXCLUDEFILEEXT table which is present in VBEXCLUDE.DB
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 27 Sep 2018
****************************************************************************************************/
void CExcludeFilesFolders::GetRecords4FileExtSEH()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBEXCLUDE.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from WWIZEXCLUDEFILEEXT;");

		Sleep(20);

		//Clear here loaded exclude strings
		m_vExcludeFileExtList.clear();

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);
			STRUCTEXCLUDEFILEEXTLIST szExcludeFileExtList;

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}
			//to read unicode string from db
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(1), -1, NULL, 0);
			wchar_t *wstrDbData = new wchar_t[wchars_num];
			if (wstrDbData == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CExcludeFilesFolders::GetRecordsSEH", 0, 0, true, SECONDLEVEL);
				continue;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(1), -1, wstrDbData, wchars_num);
			wcscpy_s(szExcludeFileExtList.szFileExt, wstrDbData);
			delete[] wstrDbData;

			_wcslwr(szExcludeFileExtList.szFileExt);

			m_vExcludeFileExtList.push_back(szExcludeFileExtList);
		}

		dbSQlite.Close();
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::GetRecords4FileExtSEH, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::GetRecords4FileExtSEH", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Neha Gharge
*  SR_NO
*  Date           : 11 May,2015
****************************************************************************************************/
CString CExcludeFilesFolders::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***************************************************************************************************
*  Function Name  : ComparePaths
*  Description    : Compare file paths with Excluded paths if found
					@return true else
					@return false.
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
*  Updated By     : Kunal Waghmare
*  Updated Date   : 20 Feb,2019
****************************************************************************************************/
bool CExcludeFilesFolders::ISSubFilePath(LPTSTR lpString, LPTSTR lpSearchString)
{
	bool bReturn = false;
	__try
	{
		//sanity check
		if (!lpString || !lpSearchString)
		{
			return bReturn;
		}

		//make here copy to tokenize
		WCHAR szString[MAX_FILE_PATH_LENGTH] = { 0 };
		WCHAR szSearchString[MAX_FILE_PATH_LENGTH] = { 0 };
		wcscpy_s(szString, lpString);
		wcscpy_s(szSearchString, lpSearchString);

		//Tokenize buffer
		WCHAR seps[] = L"\\";
		WCHAR *tokenFirst = NULL;
		WCHAR *tokenSecond = NULL;
		WCHAR *contextFirst = NULL;
		WCHAR *contextSecond = NULL;

		tokenFirst = wcstok_s(szString, seps, &contextFirst);
		tokenSecond = wcstok_s(szSearchString, seps, &contextSecond);

		if (tokenFirst == NULL || tokenSecond == NULL)
		{
			return bReturn;
		}

		//check is search string token is exists.
		while (tokenSecond != NULL)
		{
			//if token not exists to compare means searchstring is shorter than actual string
			if (!tokenFirst)
			{
				bReturn = false;
				break;
			}

			//compare token here
			if (wcscmp(tokenFirst, tokenSecond) != 0)
			{
				bReturn = false;
				break;
			}

			bReturn = true;

			tokenFirst = wcstok_s(NULL, seps, &contextFirst);
			tokenSecond = wcstok_s(NULL, seps, &contextSecond);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in CExcludeFilesFolders::ISSubFilePath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SaveEntriesInDB
*  Description    : Function which makes clone of Exclude DB to use for boot time scanner, as sqlite can not be
use in boot time scanner.
*  Author Name    : Ram Shelke
*  Date           : 13th Nov 2017
****************************************************************************************************/
bool CExcludeFilesFolders::SaveEntriesInDB()
{
	bool bReturn = false;
	HANDLE hOutputFileHandle = INVALID_HANDLE_VALUE;
	try
	{
		//Load here Exclude list if not loaded.
		if (!m_bISDBLoaded)
		{
			if (!LoadExcludeDB())
			{
				return bReturn;
			}
			m_bISDBLoaded = true;
		}

		CString csExcludeDBPath = GetWardWizPathFromRegistry();
		csExcludeDBPath += WRDWIZEXCLUDECLONEDB;

		if (PathFileExists(csExcludeDBPath))
		{
			SetFileAttributes(csExcludeDBPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(csExcludeDBPath);
		}

		if (m_vExludeList.size() == 0x00 && m_vExcludeFileExtList.size() == 0x00)
		{
			return bReturn;
		}

		hOutputFileHandle = CreateFile(csExcludeDBPath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutputFileHandle == INVALID_HANDLE_VALUE)
		{
			goto CLEANUP;
		}

		//Set here file offset to begining of file.
		SetFilePointer(hOutputFileHandle, 0x00, NULL, FILE_BEGIN);

		EXCLUDELISTMAP::iterator vExCludeIter;
		for (vExCludeIter = m_vExludeList.begin(); vExCludeIter != m_vExludeList.end(); vExCludeIter++)
		{
			WRDWIZEXCLUDESCAN stScheduleScan = { 0 };
			stScheduleScan.byIsSubFolder = vExCludeIter->byType;
			wcscpy_s(stScheduleScan.szFilePath, vExCludeIter->szData);

			DWORD dwBytesWritten = 0x00;
			if (!WriteFile(hOutputFileHandle, &stScheduleScan, sizeof(stScheduleScan), &dwBytesWritten, NULL))
			{
				AddLogEntry(L"### Failed to write data into file: [%s]", csExcludeDBPath, 0, true, SECONDLEVEL);
			}
		}
		if (!SaveEntriesInDB4Ext())
		{
			AddLogEntry(L"### Failed to SaveEntriesInDB4Ext in CExcludeFilesFolders::SaveEntriesInDB", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::SaveEntriesInDB", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:
	//Need to close file handle after Writting buffer
	if (hOutputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOutputFileHandle);
		hOutputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SaveEntriesInDB4Ext
*  Description    : Function which makes clone of Exclude DB to use for boot time scanner for file extesion, as sqlite can not be
use in boot time scanner.
*  Author Name    : Amol Jaware
*  Date           : 3rd Oct 2018
****************************************************************************************************/
bool CExcludeFilesFolders::SaveEntriesInDB4Ext()
{
	bool bReturn = false;
	HANDLE hOutputFileHandle = INVALID_HANDLE_VALUE;
	try
	{
		//Load here Exclude file extension list if not loaded.
		if (!m_bISDBLoaded4FileExt)
		{
			if (!LoadExcludeDB())
			{
				return bReturn;
			}
			m_bISDBLoaded4FileExt = true;
		}

		CString csExcludeExtDB = GetWardWizPathFromRegistry();
		csExcludeExtDB += WRDWIZEXTEXCLUDECLONEDB;

		if (PathFileExists(csExcludeExtDB))
		{
			SetFileAttributes(csExcludeExtDB, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(csExcludeExtDB);
		}

		if (m_vExcludeFileExtList.size() == 0x00)
		{
			return bReturn;
		}

		hOutputFileHandle = CreateFile(csExcludeExtDB, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutputFileHandle == INVALID_HANDLE_VALUE)
		{
			goto CLEANUP;
		}

		//Set here file offset to begining of file.
		SetFilePointer(hOutputFileHandle, 0x00, NULL, FILE_BEGIN);

		EXCLUDELISTFILEEXTMAP::iterator vExCludeIter;
		for (vExCludeIter = m_vExcludeFileExtList.begin(); vExCludeIter != m_vExcludeFileExtList.end(); vExCludeIter++)
		{
			WRDWIZEXCLUDEEXT stScheduleScan = { 0 };
			wcscpy_s(stScheduleScan.szExt, vExCludeIter->szFileExt);

			DWORD dwBytesWritten = 0x00;
			if (!WriteFile(hOutputFileHandle, &stScheduleScan, sizeof(stScheduleScan), &dwBytesWritten, NULL))
			{
				AddLogEntry(L"### Failed to write data into file: [%s]", csExcludeExtDB, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::SaveEntriesInDB4Ext", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:
	//Need to close file handle after Writting buffer
	if (hOutputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOutputFileHandle);
		hOutputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadCustExcPaths
*  Description    : Function to exclude customized folders from scanning
*  Author Name    : Ram
*  Date           : 06 JAN 2020
****************************************************************************************************/
void CExcludeFilesFolders::LoadCustExcPaths()
{
	try
	{
		TCHAR	szUserOSDrivePath[512] = { 0 };
		CString csGetOSDrive = L"";
		GetEnvironmentVariable(L"ALLUSERSPROFILE", szUserOSDrivePath, 511);
		CString csSelectedDrive = szUserOSDrivePath;
		int iPos = csSelectedDrive.Find(L"\\");
		csGetOSDrive = csSelectedDrive.Left(2);

		//C:\\Windows\\SoftwareDistribution
		CString csFolder = csGetOSDrive + L"\\Windows\\SoftwareDistribution";
		if (PathFileExists(csFolder))
		{
			STRUCTEXLCUDELIST szExcludeList;
			wcscpy_s(szExcludeList.szData, csFolder);
			_wcslwr(szExcludeList.szData);

			char szISSubFol[10] = { 0 };
			strcpy_s(szISSubFol, "1");
			szExcludeList.byType = szISSubFol[0];

			m_vExludeList.push_back(szExcludeList);
		}

		//C:\\Morpho
		csFolder = csGetOSDrive + L"\\Morpho";
		if (PathFileExists(csFolder))
		{
			STRUCTEXLCUDELIST szExcludeList;
			wcscpy_s(szExcludeList.szData, csFolder);
			_wcslwr(szExcludeList.szData);

			char szISSubFol[10] = { 0 };
			strcpy_s(szISSubFol, "1");
			szExcludeList.byType = szISSubFol[0];

			m_vExludeList.push_back(szExcludeList);
		}


		//C:\\MorphoRdServiceL0Soft
		csFolder = csGetOSDrive + L"\\MorphoRdServiceL0Soft";
		if (PathFileExists(csFolder))
		{
			STRUCTEXLCUDELIST szExcludeList;
			wcscpy_s(szExcludeList.szData, csFolder);
			_wcslwr(szExcludeList.szData);

			char szISSubFol[10] = { 0 };
			strcpy_s(szISSubFol, "1");
			szExcludeList.byType = szISSubFol[0];

			m_vExludeList.push_back(szExcludeList);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CExcludeFilesFolders::LoadCustExcPaths", 0, 0, true, SECONDLEVEL);
	}
}



