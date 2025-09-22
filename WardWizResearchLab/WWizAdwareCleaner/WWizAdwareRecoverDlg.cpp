#include "stdafx.h"
#include "WWizAdwareRecoverDlg.h"


WWizAdwareRecoverDlg::WWizAdwareRecoverDlg() : behavior_factory("WardWizAdwareCleaner")
{
}


WWizAdwareRecoverDlg::~WWizAdwareRecoverDlg()
{
}

/***************************************************************************************************
*  Function Name  : OnLoadRecoverEntries
*  Description    : Function to recover entries
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 July 2017
****************************************************************************************************/
json::value WWizAdwareRecoverDlg::OnLoadRecoverEntries(SCITER_VALUE svRecoverArrayFn)
{
	try
	{
		m_svRecoverArrayFn = svRecoverArrayFn;
		GetRecordsSEH();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WWizAdwareRecoverDlg::OnLoadRecoverEntries()", 0, 0, true, SECONDLEVEL);
		return 0;
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : GetRecords
*  Description    : Function which get records from WWIZADWCLEANER table which is present in WWIZADWCLEANER.DB
*  Author Name    : Jeena Mariam Saji
*  Date           : 13 July 2017
****************************************************************************************************/
void WWizAdwareRecoverDlg::GetRecordsSEH()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;

		CString	csWardWizModulePath = theApp.GetModuleFilePath();
		CString	csWardWizAdwPath = L"";
		csWardWizAdwPath.Format(L"%s\\WWIZADWCLEANER.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizAdwPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		CString csThreatName;
		CString csOrgPath;
		CString csFileFold;
		CString csQuarPath;
		CString csEntryid;

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from WWIZADWCLEANER;");

		Sleep(20);
		int a = qResult.GetNumRows();
		char szData[512] = { 0 };
		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);
			if (qResult.GetFieldIsNull(0))
			{
				continue;
			}
			strcpy_s(szData, qResult.GetFieldValue(0));
			csEntryid = CString(szData);

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}
			strcpy_s(szData, qResult.GetFieldValue(1));
			csThreatName = CString(szData);
			
			if (qResult.GetFieldIsNull(2))
			{
				continue;
			}
			strcpy_s(szData, qResult.GetFieldValue(2));
			csOrgPath = CString(szData);

			if (qResult.GetFieldIsNull(3))
			{
				continue;
			}
			strcpy_s(szData, qResult.GetFieldValue(3));
			csQuarPath = CString(szData);

			if (qResult.GetFieldIsNull(4))
			{
				continue;
			}
			strcpy_s(szData, qResult.GetFieldValue(4));
			csFileFold = CString(szData);

			const sciter::value data[5] = {sciter::string(csEntryid), sciter::string(csThreatName), sciter::string(csOrgPath), sciter::string(csQuarPath), sciter::string(csFileFold)};
			sciter::value svArrayEntries = sciter::value(data, 5);
			m_svRecoverArrayFn.call(svArrayEntries);
		}
		dbSQlite.Close();
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in WWizAdwareRecoverDlg::GetRecordsSEH, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBtnClickRecover
*  Description    : Function to recover entries
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 July 2017
****************************************************************************************************/
json::value WWizAdwareRecoverDlg::OnBtnClickRecover(SCITER_VALUE svArrayRecover)
{
	try
	{
		bool bArray = false;
		svArrayRecover.isolate();
		bArray = svArrayRecover.is_array();
		m_svArrayRecover = svArrayRecover;
		if (!bArray)
		{
			return false;
		}
		OnBtnClickRecoverEntries();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WWizAdwareRecoverDlg::OnBtnClickRecover()", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : OnBtnClickRecoverEntries
*  Description    : Function to recover entries
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 July 2017
****************************************************************************************************/
void WWizAdwareRecoverDlg::OnBtnClickRecoverEntries()
{
	try
	{
		CString csInsertQuery;
		DWORD dwTotRecoverEntries = 0x00;
		dwTotRecoverEntries = m_svArrayRecover.length();

		for (DWORD dwIndex = 0x00; dwIndex < dwTotRecoverEntries; dwIndex++)
		{
			const SCITER_VALUE svEachEntry = m_svArrayRecover[dwIndex];

			bool bValue = svEachEntry[L"selected"].get(false);
			if (!bValue)
				continue;

			const std::wstring chThreatId = svEachEntry["ThreatId"].get(L"");
			const std::wstring chThreatName = svEachEntry["ThreatName"].get(L"");
			const std::wstring chFilePath = svEachEntry["FilePath"].get(L"");
			const std::wstring chQuarPath = svEachEntry["QuarPath"].get(L"");
			const std::wstring chFileFold = svEachEntry["FileFolder"].get(L"");

			CString csThreatID = chThreatId.c_str();
			CString csThreatName = chThreatName.c_str();
			CString csFilePath = chFilePath.c_str();
			CString csQuarPath = chQuarPath.c_str();
			CString csFileFold = chFileFold.c_str();

			if (csFileFold == L"0")
			{
				TCHAR szSourceString[1000];
				lstrcpyn(szSourceString, csQuarPath, 1000);

				TCHAR szDestString[1000];
				lstrcpyn(szDestString, csFilePath, 1000);

				theApp.OnCopyFolder(szSourceString, szDestString);
				EmptyDirectory(csQuarPath.GetBuffer(), true);
				RemoveDirectory(csQuarPath);
			}
			else if (csFileFold == L"1")
			{
				CopyFile(csQuarPath, csFilePath, false);
				RemoveFile(csQuarPath);
			}

			csInsertQuery = _T("DELETE FROM WWIZADWCLEANER (null,");
			csInsertQuery.Format(_T("DELETE FROM WWIZADWCLEANER WHERE [db_ScanID] = ('%s');"), csThreatID);
			CT2A ascii(csInsertQuery, CP_UTF8);
			INT64 iScanId = InsertDataToTable(ascii.m_psz);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WWizAdwareRecoverDlg::OnBtnClickRecoverEntries()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : InsertDataToTable
*  Description    : Function to insert data into table
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 July 2017
****************************************************************************************************/
INT64 WWizAdwareRecoverDlg::InsertDataToTable(const char* szQuery)
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;

		CString	csWardWizModulePath = theApp.GetModuleFilePath();
		CString	csWardWizAdwPath = L"";
		csWardWizAdwPath.Format(L"%s\\WWIZADWCLEANER.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizAdwPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		int iRows = dbSQlite.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = dbSQlite.GetLastRowId();
		dbSQlite.Close();

		return iLastRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WWizAdwareRecoverDlg::InsertDataToTable()", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : EmptyDirectory
*  Description    : Function to empty directory
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 July 2017
****************************************************************************************************/
void WWizAdwareRecoverDlg::EmptyDirectory(TCHAR* folderPath, bool bTakeBackup)
{
	try
	{
		TCHAR szfileFound[MAX_PATH] = { 0 };
		WIN32_FIND_DATA info;
		HANDLE hp;
		swprintf(szfileFound, L"%s\\*.*", folderPath);
		hp = FindFirstFile(szfileFound, &info);
		do
		{
			if (!((_tcscmp(info.cFileName, L".") == 0) ||
				(_tcscmp(info.cFileName, L"..") == 0)))
			{
				if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ==
					FILE_ATTRIBUTE_DIRECTORY)
				{
					wstring subFolder = folderPath;
					subFolder.append(L"\\");
					subFolder.append(info.cFileName);
					EmptyDirectory((TCHAR*)subFolder.c_str(), bTakeBackup);
					RemoveDirectory(subFolder.c_str());
				}
				else
				{
					swprintf(szfileFound, L"%s\\%s", folderPath, info.cFileName);
					SetFileAttributes(szfileFound, FILE_ATTRIBUTE_NORMAL);
					if (!DeleteFile(szfileFound))
					{
						if (!MoveFileEx(szfileFound, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
						{
							AddLogEntry(L"### MoveFileEx failed for file: [%s]", szfileFound, 0, true, SECONDLEVEL);
						}
					}
				}
			}

		} while (FindNextFile(hp, &info));
		FindClose(hp);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WWizAdwareRecoverDlg::EmptyDirectory()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : RemoveFile
*  Description    : Function to remove file
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 July 2017
****************************************************************************************************/
bool WWizAdwareRecoverDlg::RemoveFile(CString csPath)
{
	try
	{
		bool bReturn = false;
		SetFileAttributes(csPath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(csPath))
		{
			if (!MoveFileEx(csPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
			{
				AddLogEntry(L"### MoveFileEx failed for file: [%s]", csPath, 0, true, SECONDLEVEL);
				return bReturn;
			}
		}
		bReturn = true;
		return bReturn;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in WWizAdwareRecoverDlg::RemoveFile()", 0, 0, true, SECONDLEVEL);
	}
}
