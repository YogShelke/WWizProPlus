/**********************************************************************************************************
	Program Name          :     ScanByName.cpp
	Description           :     This class is used implemented to add files and folders for scanning.
	Author Name			  :     Amol Jaware
	Date Of Creation      :
	Version No            :     1.14.0.1
	Special Logic Used    :     This class has following functionality
								a) Load Existing Scan By Name DB
								b) Add/edit/delete files for scanning.
								c) Save ScanByName DB.
	Modification Log      :
		1.Amol Jaware               Introduced class         05-OCT-2018
------------------------------------------------------------------------------------------------------------
							*** SQL statement to create VBSCANNAME.DB *** 
------------------------------------------------------------------------------------------------------------
				CREATE TABLE WWIZSCANNAME(NAME VARCHAR (20) NOT NULL, FILENAME INT NOT NULL);

			DUMMY:
			INSERT INTO WWIZSCANNAME (FILENAME) VALUES ('system-explorer.png');
			INSERT INTO WWIZSCANNAME (FILENAME) VALUES ('temp.pdf);
			INSERT INTO WWIZSCANNAME (FILENAME) VALUES ('UnRegister.bat');

***********************************************************************************************************/
#include "StdAfx.h"
#include "ScanByName.h"
#include "WardWizDatabaseInterface.h"

#define		WWIZSCANNAMECLONEDB			L"VBSCANNAMECLONE.DB"
//#define		WRDWIZEXTEXCLUDECLONEDB			L"VBEXCLUDEEXT.DB"

/***************************************************************************************************
*  Function Name  : CScanByName
*  Description    : Const'r
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
CScanByName::CScanByName() :
m_hSQLiteDLL(NULL)
, m_bISDBLoaded4ScanFileName(false)
//, m_bISDBLoaded4FileExt(false)
{
	LoadRequiredLibrary();
}

/***************************************************************************************************
*  Function Name  : ~CScanByName
*  Description    : Dest'r
*  Author Name    : Ram Shelke
*  SR_NO
*  Date           : 26 Apr,2016
****************************************************************************************************/
CScanByName::~CScanByName()
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
bool CScanByName::LoadRequiredLibrary()
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
		AddLogEntry(L"### Exception in CScanByName::LoadRequiredLibrary", 0, 0, true, SECONDLEVEL);
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadScanByNameDB
*  Description    : Function which loads Scan by Name files DB in memory from file.
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 05 Oct 2018
****************************************************************************************************/
bool CScanByName::LoadScanByNameDB()
{
	try
	{
		GetRecords();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanByName::LoadScanByNameDB", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : IsScanByNameFile
*  Description    : Function which checks input file name in scan by name list and returns
					@true, if found @else false.
*  Author Name    : Amol Jaware
*  Date           : 05 Oct 2018
****************************************************************************************************/
bool CScanByName::IsScanByNameFile(LPTSTR lpszFileName)
{
	bool bReturn = false;
	try
	{
		if (!lpszFileName)
		{
			return bReturn;
		}

		//Load here Exclude file extension list if not loaded.
		if (!m_bISDBLoaded4ScanFileName)
		{
			if (!LoadScanByNameDB())
			{
				return bReturn;
			}
			m_bISDBLoaded4ScanFileName = true;
		}

		SCANBYNAMEMAP ::iterator vExCludeIter;
		for (vExCludeIter = m_vScanByNameList.begin(); vExCludeIter != m_vScanByNameList.end(); vExCludeIter++)
		{	
			TCHAR szTempFileName[0x32] = { 0 };
			wcscpy_s(szTempFileName, (*vExCludeIter).szFileName);
			if (CompareStringW(LOCALE_SYSTEM_DEFAULT, LINGUISTIC_IGNORECASE, szTempFileName, -1, lpszFileName, -1) == CSTR_EQUAL)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanByName::IsScanByNameFile, Ext: %s", lpszFileName, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;

}

/***************************************************************************************************
*  Function Name  : GetRecords
*  Description    : Function which get records from AKSCANNAMEFILELIST table which is present in VBSCANNAME.DB
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 05 Oct 2018
****************************************************************************************************/
void CScanByName::GetRecords()
{
	__try
	{
		GetRecordsSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CScanByName::GetRecords", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetRecords
*  Description    : Function which get records from AKSCANNAMEFILELIST table which is present in VBSCANNAME.DB
*  Author Name    : Amol Jaware
*  SR_NO
*  Date           : 05 Oct 2018
****************************************************************************************************/
void CScanByName::GetRecordsSEH()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBSCANNAME.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from WWIZSCANNAMEFILELIST;");

		Sleep(20);

		//Clear here loaded exclude strings
		m_vScanByNameList.clear();

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);
			STRUCTSCANBYNAMELIST szExcludeFileExtList;

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}
			
			//to read unicode string from db
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(1), -1, NULL, 0);
			wchar_t *wstrFileName = new wchar_t[wchars_num];
			if (wstrFileName == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CScanByName::GetRecordsSEH", 0, 0, true, SECONDLEVEL);
				continue;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(1), -1, wstrFileName, wchars_num);
			
			wcscpy_s(szExcludeFileExtList.szFileName, wstrFileName);
			delete[] wstrFileName;

			_wcslwr(szExcludeFileExtList.szFileName);

			m_vScanByNameList.push_back(szExcludeFileExtList);
		}

		dbSQlite.Close();
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CScanByName::GetRecords4FileExtSEH, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanByName::GetRecordsSEH", 0, 0, true, SECONDLEVEL);
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
CString CScanByName::GetModuleFilePath()
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
****************************************************************************************************/
bool CScanByName::ISSubFilePath(LPSTR lpString, LPSTR lpSearchString)
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
		char szString[MAX_FILE_PATH_LENGTH] = { 0 };
		char szSearchString[MAX_FILE_PATH_LENGTH] = { 0 };
		strcpy_s(szString, lpString);
		strcpy_s(szSearchString, lpSearchString);

		//Tokenize buffer
		char seps[] = "\\";
		char *tokenFirst = NULL;
		char *tokenSecond = NULL;
		char* contextFirst = NULL;
		char* contextSecond = NULL;

		tokenFirst = strtok_s(szString, seps, &contextFirst);
		tokenSecond = strtok_s(szSearchString, seps, &contextSecond);

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
			if (strcmp(tokenFirst, tokenSecond) != 0)
			{
				bReturn = false;
				break;
			}

			bReturn = true;

			tokenFirst = strtok_s(NULL, seps, &contextFirst);
			tokenSecond = strtok_s(NULL, seps, &contextSecond);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in CScanByName::ISSubFilePath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SaveEntriesInDB
*  Description    : Function which makes clone of Scan By Name DB to use for boot time scanner for file name, as sqlite can not be
					use in boot time scanner.
*  Author Name    : Amol Jaware
*  Date           : 05 Oct 2018
****************************************************************************************************/
bool CScanByName::SaveEntriesInDB()
{
	bool bReturn = false;
	HANDLE hOutputFileHandle = INVALID_HANDLE_VALUE;
	try
	{
		//Load here Exclude file extension list if not loaded.
		if (!m_bISDBLoaded4ScanFileName)
		{
			if (!LoadScanByNameDB())
			{
				return bReturn;
			}
			m_bISDBLoaded4ScanFileName = true;
		}

		CString csExcludeExtDB = GetWardWizPathFromRegistry();
		csExcludeExtDB += WWIZSCANNAMECLONEDB;

		if (PathFileExists(csExcludeExtDB))
		{
			SetFileAttributes(csExcludeExtDB, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(csExcludeExtDB);
		}

		if (m_vScanByNameList.size() == 0x00)
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

		SCANBYNAMEMAP::iterator vScanByNameIter;
		for (vScanByNameIter = m_vScanByNameList.begin(); vScanByNameIter != m_vScanByNameList.end(); vScanByNameIter++)
		{
			WRDWIZSCANBYNAME stScanByNameScan = { 0 };
			wcscpy_s(stScanByNameScan.szFileName, vScanByNameIter->szFileName);

			DWORD dwBytesWritten = 0x00;
			if (!WriteFile(hOutputFileHandle, &stScanByNameScan, sizeof(stScanByNameScan), &dwBytesWritten, NULL))
			{
				AddLogEntry(L"### Failed to write data into file: [%s]", csExcludeExtDB, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanByName::SaveEntriesInDB", 0, 0, true, SECONDLEVEL);
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
