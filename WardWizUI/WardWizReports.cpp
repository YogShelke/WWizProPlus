/*********************************************************************
*  Program Name: CWardWizRecover.cpp
*  Description: To display reports entries and to delete entries.
*  Author Name: Jeena Saji
*  Date Of Creation: 6th June 2016
*  Version No: 2.0.0.1
**********************************************************************/
#include "stdafx.h"
#include "WardWizReports.h"
#include "WardWizDatabaseInterface.h"

char* g_strSqliteDBFile = ".\\VBALLREPORTS.DB";

DWORD WINAPI ShowReportsThread(LPVOID lpParam);
DWORD WINAPI DeleteReportThread(LPVOID lpParam);

/**********************************************************************************************************
*  Function Name  :	CWardWizReports
*  Description    :	Constructor
*  SR.NO		  :
*  Author Name    : Jeena Saji
*  Date           : 03 May 2016
**********************************************************************************************************/
CWardWizReports::CWardWizReports(): behavior_factory("WardWizReports")
, m_hThreadLoadReports(NULL)
, m_hThreadDeleteReports(NULL)
, m_bReportEnableDisable(false)
, m_objCom(SERVICE_SERVER, true)
{
}

/**********************************************************************************************************
*  Function Name  :	~CWardWizReports
*  Description    :	Destructor
*  SR.NO		  :
*  Author Name    : Jeena Saji
*  Date           : 03 May 2016
**********************************************************************************************************/
CWardWizReports::~CWardWizReports()
{
}

/**********************************************************************************************************
*  Function Name  :	On_LoadReportsDB
*  Description    :	Shows all  report entries after clicking report button
*  SR.NO		  :
*  Author Name    : Jeena Saji
*  Date           : 03 May 2016
**********************************************************************************************************/
json::value CWardWizReports::On_LoadReportsDB(SCITER_VALUE svFunDispReportsCB, SCITER_VALUE svFunLoadReportsFinishedCB, SCITER_VALUE svFunShowNotificationReportsCB)
{
	try
	{
		m_svFunDispReportsCB = svFunDispReportsCB;
		m_svFunLoadReportsFinishedCB = svFunLoadReportsFinishedCB;
		m_svFunShowNotificationReportsCB = svFunShowNotificationReportsCB;
		if (m_hThreadLoadReports != NULL)
		{
			return 0;
		}
		m_hThreadLoadReports = ::CreateThread(NULL, 0, ShowReportsThread, (LPVOID) this, 0, NULL);
			Sleep(10);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_LoadReportsDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_ClickReportsDelete
*  Description    :	Delete report entries after clicking report delete button
*  SR.NO		  :
*  Author Name    : Jeena Saji
*  Date           : 03 May 2016
**********************************************************************************************************/
json::value CWardWizReports::On_ClickReportsDelete(SCITER_VALUE svarrReports, SCITER_VALUE svFunDeleteReportsEntriesinTableCB, SCITER_VALUE svFunDeleteReportsFinishedCB)
{
	try
	{
		m_svArrReportsDelete = svarrReports;
		m_svFunDeleteReportsEntriesinTableCB = svFunDeleteReportsEntriesinTableCB;
		m_svFunDeleteReportsFinishedCB = svFunDeleteReportsFinishedCB;
		OnBnClickedBtnDelete();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_ClickReportsDelete", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : ShowReportsThread
*  Description    : When user clicks on reports button from tab control.This thread gets call
Loading reports and populate on list control functionality gets occured.
*  Author Name    : Prassana
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
DWORD WINAPI ShowReportsThread(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizReports *pThis = (CWardWizReports*)lpvThreadParam;
		if (pThis)
		{
			//Remove all the entries now
			pThis->m_objReportsDBToSave.RemoveAll();

			//Load other entries here
			if (pThis->LoadDBFile()) // Means migration is required..
			{
			pThis->PopulateList();
			}
			//Load Remaining entries first
			pThis->LoadRemainingEntries();

			pThis->m_hThreadLoadReports = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::ShowReportsThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	LoadDBFile
*  Description    :	Creates VBREPORTS.DB file & retrieves its contents
*  SR.NO		  : ITINAVREPORTS_0002
*  Author Name    : Prajakta
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CWardWizReports::LoadDBFile()
{
	try
	{
		AddLogEntry(L">>> Loading entry from DB files", 0, 0, true, FIRSTLEVEL);
		CString csFilePath;
		CString csMsgBox(L"");
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_GETMODULEPATH_ERR"));
			m_svFunShowNotificationReportsCB.call((SCITER_STRING)csMsgBox);
			return false;
		}
		csFilePath = szModulePath;
		csFilePath.Append(L"\\VBREPORTS.DB");

		//clear all reports entries here
		m_objReportsDB.RemoveAll();

		if (!PathFileExists(csFilePath))
		{
			return false;
		}

		CFile rFile(csFilePath, CFile::modeRead);
		// Create a loading archive
		CArchive arLoad(&rFile, CArchive::load);
		m_objReportsDB.Serialize(arLoad);

		// Close the storing archive
		arLoad.Close();
		rFile.Close();
		rFile.Remove(csFilePath);
	
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::LoadDBFile", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	InsertIntoSQLite
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
void InsertIntoSQLite(const char* szQuery)
{
	try
	{
		CWardWizSQLiteDatabase objSqlDb(g_strSqliteDBFile);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);

		objSqlDb.Close();

		return;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::InsertIntoSQLite", 0, 0, true, SECONDLEVEL);

	}

}

//Get ID from string
CString GetLanguageIDFromAction(CString csMessage)
{
	CString csLanguageID = 0;

	if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCANNING_ABORTED")) == 0)
	{
		csLanguageID = "IDS_USB_SCANNING_ABORTED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND")) == 0)
	{
		csLanguageID = "IDS_USB_SCAN_NO_THREAT_FOUND";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_DETECTED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_DETECTED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_QUARANTINED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_REPAIRED";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_NO_FILE_FOUND")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_NO_FILE_FOUND";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_CANT_REPAIR")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_CANT_REPAIR";
	}
	else if (csMessage.CompareNoCase(theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_FAILED")) == 0)
	{
		csLanguageID = "IDS_CONSTANT_THREAT_FAILED";
	}

	return csLanguageID;
}
/**********************************************************************************************************
*  Function Name  :	PopulateList
*  Description    :	Displays entries on reports dialog
*  SR.NO		  : ITINAVREPORTS_0003
*  Author Name    : Prajakta
*  Date           : 20 Nov 2013
Issue Resolved: 0001349
**********************************************************************************************************/
void CWardWizReports::PopulateList()
{
	CString csInsertQuery =0;
	try
	{
		// get a reference to the contacts list
		const ContactList& contacts = m_objReportsDB.GetContacts();

		// iterate over all contacts & add them to the list
		POSITION pos = contacts.GetHeadPosition();
		while (pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);

			//Add here the contact one by one
			m_objReportsDBToSave.AddContact(contact, true);

			CString csDateTime = contact.GetFirstEntry();
			CString csScanType = contact.GetSecondEntry();
			CString csScanStatus = contact.GetFifthEntry();
			CString csThreatName = contact.GetThirdEntry();
			CString csScanFilePath = contact.GetForthEntry();
			SCANTYPE eScan ;

			eScan = FULLSCAN;

			if (csScanType.CompareNoCase(L"RootKit Scan")==0)
				eScan = ANTIROOTKITSCAN;
			else if (csScanType.CompareNoCase(L"Full Scan")==0)
				eScan = FULLSCAN;
			else if (csScanType.CompareNoCase(L"Quick Scan")==0)
				eScan = QUICKSCAN;
			else if (csScanType.CompareNoCase(L"Custom Scan")==0)
				eScan = CUSTOMSCAN;


			const sciter::value data[5] = { (SCITER_STRING)csScanType, (SCITER_STRING)csDateTime, (SCITER_STRING)csScanStatus, (SCITER_STRING)csThreatName, (SCITER_STRING)csScanFilePath };
			sciter::value arrReportEntry = sciter::value(data, 5);
			m_svFunDispReportsCB.call(arrReportEntry);
	
			CTime objNewEntryDTime;
			if (!GetDateTimeFromString(csDateTime, objNewEntryDTime))
			{
				continue;
			}
			CString csScanDateTime = 0;
			csScanDateTime += objNewEntryDTime.Format("%Y-%m-%d %H:%M:%S");

			CString csScanDate = 0;
			csScanDate += objNewEntryDTime.Format("%Y-%m-%d");

			CString csActionID = GetLanguageIDFromAction(csScanStatus);

			csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");
			
			csScanFilePath.Replace(L"'", L"''");
			csThreatName.Replace(L"'", L"''");

			csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%d,'%s','%s','%s','%s','%s','%s','%s',0,1,NULL);"), eScan, csScanDate, csScanDateTime, csScanDate, csScanDateTime, csScanFilePath, csThreatName, csActionID);

			if (!PathFileExistsA(g_strSqliteDBFile))
			{
				CWardWizSQLiteDatabase objSqlDb(g_strSqliteDBFile);
				objSqlDb.Open();
				objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
				objSqlDb.Close();
			}

			CT2A ascii(csInsertQuery, CP_UTF8);
			InsertIntoSQLite(ascii.m_psz);
		}
		m_svFunLoadReportsFinishedCB.call();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::PopulateList. The query is : ", csInsertQuery, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetMaximumDigitFromFiles
*  Description    :	Function to get Maximum number from files
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizReports::LoadRemainingEntries()
{
	try
	{
		vector <int > vecNumList;

		//get the file ids and sort
		GetSortedFileNames(vecNumList);

		if (vecNumList.size() == 0)
		{
			return;
		}

		CString csFolderPath;
		if (!GetReportsFolderPath(csFolderPath))
		{
			return;
		}

		//Load the file contents in memory and populate in list control
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{

			CString csFilePath;
			csFilePath.Format(L"%s\\VBREPORTS_%d.DB", csFolderPath, *it);
			if (PathFileExists(csFilePath))
			{
				LoadDataContentFromFile(csFilePath);
				PopulateList();
			}
		}

		//Save the DB file here
		SaveDBFile();

		//Remove the files which are already loaded and merged.
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{
			CString csFilePath;
			csFilePath.Format(L"%s\\VBREPORTS_%d.DB", csFolderPath, *it);
			if (PathFileExists(csFilePath))
			{
				SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
				if (DeleteFile(csFilePath) == FALSE)
				{
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::LoadRemainingEntries", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetDateTimeFromString
*  Description    :	Function to get fill CTime object filled from string.
*  SR.NO		  :
*  Author Name    : Ram Shelke
*  Date           : 09 Mar 2016
**********************************************************************************************************/
bool CWardWizReports::GetDateTimeFromString(CString csItemEntry, CTime &objDateTime)
{
	bool bReturn = false;
	try
	{
		if (csItemEntry.GetLength() == 0)
		{
			return bReturn;
		}

		CString csTime = csItemEntry.Right(8);

		csItemEntry.Truncate(10);
		csItemEntry.Trim();
		CString csDBTmpDate = L"";
		int iPos1 = 0; int j = 0;
		int iDBArr[6] = { 0 };

		while (csItemEntry.Trim().GetLength() != 0)
		{
			if (iPos1 == -1)
				break;

			csDBTmpDate = csItemEntry.Tokenize(_T("/"), iPos1);

			if (csDBTmpDate.GetLength() == 0)
			{
				continue;
			}

			iDBArr[j] = _wtoi(csDBTmpDate);
			j++;

			if (j > 3)
			{
				break;
			}
		}

		if (iDBArr[2] == 0 || iDBArr[0] == 0 || iDBArr[1] == 0)
			false;

		iPos1 = 0;
		while (csTime.Trim().GetLength() != 0)
		{
			if (iPos1 == -1)
				break;

			csDBTmpDate = csTime.Tokenize(_T(":"), iPos1);

			if (csDBTmpDate.GetLength() == 0)
			{
				continue;
			}

			iDBArr[j] = _wtoi(csDBTmpDate);
			j++;

			if (j > 5)
			{
				break;
			}
		}

		CTime objDBDate(iDBArr[2], iDBArr[0], iDBArr[1], iDBArr[3], iDBArr[4], iDBArr[5]);
		objDateTime = objDBDate;
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::GetDateTimeFromString", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	GetSortedFileNames
*  Description    :	Function to get file name in sorted manner
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizReports::GetSortedFileNames(vector<int> &vec)
{
	try
	{
		CString csReprtsFolderPath;
		if (!GetReportsFolderPath(csReprtsFolderPath))
		{
			return false;
		}

		GetFileDigits(csReprtsFolderPath, vec);
		sort(vec.begin(), vec.end());

		if (vec.size() > 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::GetSortedFileNames", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetReportsFolderPath
*  Description    :	Function to get reports folder path
*  Author Name    : Ramkrushna Shelke
*  Date           : 06 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizReports::GetReportsFolderPath(CString &csFolderPath)
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	try
	{
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return false;
		}
		csFolderPath.Format(L"%s\\%s", szModulePath, L"REPORTS");
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::GetReportsFolderPath", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return false;
}

/**********************************************************************************************************
*  Function Name  :	LoadDataContentFromFile
*  Description    :	Loads VBRECOVERENTRIES.DB file content into serialization object.
*  SR.NO		  :
*  Author Name    : Neha Gharge
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CWardWizReports::LoadDataContentFromFile(CString csPathName)
{
	try
	{
		AddLogEntry(L">>> Loading Reports db", 0, 0, true, FIRSTLEVEL);

		m_objReportsDB.RemoveAll();

		if (!PathFileExists(csPathName))
			return false;

		CFile rFile(csPathName, CFile::modeRead);

		// Create a loading archive
		CArchive arLoad(&rFile, CArchive::load);
		m_objReportsDB.Serialize(arLoad);

		// Close the loading archive
		arLoad.Close();
		rFile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::LoadDataContentFromFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
	//PopulateList();
}

/**********************************************************************************************************
*  Function Name  :	SaveDBFile
*  Description    :	According to type of module DB get save.
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0003
**********************************************************************************************************/
bool CWardWizReports::SaveDBFile()
{
	try
	{
		CString csFilePath;
		CString csMsgBox(L"");
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_GETMODULEPATH_ERR"));
			m_svFunShowNotificationReportsCB.call((SCITER_STRING)csMsgBox);
			return false;
		}
		csFilePath = szModulePath;
		csFilePath.Append(L"\\VBREPORTS.DB");

		int iFileVersion = 1;
		CFile wFile(csFilePath, CFile::modeCreate | CFile::modeWrite);

		CArchive arStore(&wFile, CArchive::store);

		m_objReportsDBToSave.SetFileVersion(iFileVersion);
		m_objReportsDBToSave.Serialize(arStore);

		arStore.Close();
		wFile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::SaveDBFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetMaximumDigitFromFiles
*  Description    :	Function to get Maximum number from files
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizReports::GetFileDigits(LPCTSTR pstr, vector<int> &vec)
{
	try
	{
		CFileFind finder;
		bool bIsFolder = false;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.DB");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bIsFolder = true;
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				continue;
			}

			CString csFileName = finder.GetFileName();
			CString csDigit = csFileName.Left(csFileName.ReverseFind(L'.'));
			csDigit = csDigit.Right(csDigit.GetLength() - (csDigit.ReverseFind(L'_') + 1));
			if (csDigit.Trim().GetLength() != 0)
			{
				DWORD dwDigit = _wtoi(csDigit);
				if (dwDigit != 0)
				{
					vec.push_back(dwDigit);
				}
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::EnumTotalFolder", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	OnBnClickedBtnDelete
*  Description    :	Deletes selected entry from reports
*  SR.NO          :
*  Author Name    : Prajakta
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CWardWizReports::OnBnClickedBtnDelete()
{
	CString csMsgBox(L"");
	try
	{
		int		index = 0;
		bool bIsArray = false;
		m_svArrReportsDelete.isolate();
		bIsArray = m_svArrReportsDelete.is_array();
		if (!bIsArray)
		{
			return;
		}
		index = m_svArrReportsDelete.length();

		if (!theApp.ShowRestartMsgOnProductUpdate())
		{
			return;
		}
		if (index == 0x00)
		{
			m_bDeleteEntriesFinished = true;
			csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_NO_ENTRY_TO_DELETE"));
			m_svFunShowNotificationReportsCB.call((SCITER_STRING)csMsgBox);
			m_svFunDeleteReportsFinishedCB.call();
		}
		else
		{
			m_bDeleteEntriesFinished = false;
			m_hThreadDeleteReports = ::CreateThread(NULL, 0, DeleteReportThread, (LPVOID) this, 0, NULL);
			Sleep(1000);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::OnBnClickedBtnDelete.", 0, 0, true, SECONDLEVEL);
	}

}

/**********************************************************************************************************
*  Function Name  :	DeleteReportThread
*  Description    :	Allows single/multiple entries deletion from reports & accordingly updates the reports log
*  SR.NO          :
*  Author Name    : Vilas
*  Date           : 20 Nov 2013
**********************************************************************************************************/
DWORD WINAPI DeleteReportThread(LPVOID lpParam)
{
	CWardWizReports		*pRptDlg = (CWardWizReports *)lpParam;
	DWORD	dwRet = 0x00;
	CString csMsgBox(L"");
	try
	{
		if (!pRptDlg)
			return 0;
		pRptDlg->m_bReportEnableDisable = true;
		AddLogEntry(L">>> Deleting report entry started", 0, 0, true, FIRSTLEVEL);

		pRptDlg->m_bReportStop = false;
		pRptDlg->m_bReportThreadStart = true;
		bool	bSelected = false;
		

		int iReportCount = 0x00;
		CString csReportEntry = L"";
		FILE *pFile = NULL;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		TCHAR szFilePath[MAX_PATH] = { 0 };
		int		index = 0;
		DWORD	dwDeletedEntries = 0x00;
		CString csDateTime = L"", csScanType = L"";
		try
		{
			if (!GetModulePath(szModulePath, MAX_PATH))
			{
				pRptDlg->m_bDeleteEntriesFinished = true;
				csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_GETMODULEPATH_ERR"));
				pRptDlg->m_svFunShowNotificationReportsCB.call((SCITER_STRING)csMsgBox);
				dwRet = 0x01;
				goto Cleanup;
			}

			CString csFilePath(szModulePath);
			csFilePath += _T("\\VBREPORTS.DB");

			if (!pRptDlg->SendReportsData2Service(RELOAD_DBENTRIES, REPORTS, L"", true))
			{
				AddLogEntry(L"### failed to RELOAD_DBENTRIES SendReportsData2Service", 0, 0, true, SECONDLEVEL);
				dwRet = 0x01;
				goto Cleanup;
			}

			int iCurrentReportEntry = 0;
			do
			{
				iReportCount = pRptDlg->m_svArrReportsDelete.length();
				iReportCount--;
				//for (index = 0; index < pRptDlg->m_lstListView.GetItemCount(); index++)
				for (iCurrentReportEntry = iReportCount; iCurrentReportEntry >=0 ; iCurrentReportEntry--)
				{
					const SCITER_VALUE svEachEntry = pRptDlg->m_svArrReportsDelete[iCurrentReportEntry];
					const std::wstring chScanType = svEachEntry["ScanType"].get(L"");
					const std::wstring chDateTime = svEachEntry["DateTime"].get(L"");
					const std::wstring chScanStatus = svEachEntry["ScanStatus"].get(L"");
					const std::wstring chScanFilePath = svEachEntry["ScanFilePath"].get(L"");
					bool bValue = svEachEntry[L"selected"].get(false);

					if (pRptDlg->m_bReportStop)
					{
						break;
					}
					//if (pRptDlg->m_lstListView.GetCheck(index))
					if (bValue)
					{
						csScanType = chScanType.c_str();
						csDateTime = chDateTime.c_str();
						csFilePath = chScanFilePath.c_str();

						if (pRptDlg->SendReportsOperation2Service(DELETE_REPORTS_ENTRIES, csDateTime, csScanType, csFilePath, true))
						{
							pRptDlg->m_svFunDeleteReportsEntriesinTableCB.call(iCurrentReportEntry);
						}
						Sleep(10);
						bSelected = true;
						dwDeletedEntries++;
					}
				}
				if (iCurrentReportEntry <= iReportCount)
					break;

				if (pRptDlg->m_bReportStop)
				{
					break;
				}

			} while (bSelected);

			if (!bSelected)
			{
				pRptDlg->m_bDeleteEntriesFinished = true;
				csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_SELECTED_FOR_DELETION"));
				pRptDlg->m_svFunShowNotificationReportsCB.call((SCITER_STRING)csMsgBox);
				pRptDlg->m_svFunDeleteReportsFinishedCB.call();
				dwRet = 0x02;
				goto Cleanup;
			}

			if (!pRptDlg->SendReportsOperation2Service(SAVE_REPORTS_ENTRIES, L"", L"", L"", true))
			{
				AddLogEntry(L"### Failed to send SAVE_REPORTS_ENTRIES in CNextGenExReports : DeleteReportThread", 0, 0, true, SECONDLEVEL);
			}
			pRptDlg->m_svFunDeleteReportsFinishedCB.call();
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizReports::DeleteReportThread", 0, 0, true, SECONDLEVEL);
		}

		pRptDlg->m_bDeleteEntriesFinished = true;

		if (!pRptDlg->m_bReportStop)
		{
			if (dwDeletedEntries > 0x01)
			{
				csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_SELECTED_ENTRIES"));
				pRptDlg->m_svFunShowNotificationReportsCB.call((SCITER_STRING)csMsgBox);
			}
			else
			{
				csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_SELECTED_ENTRY"));
				pRptDlg->m_svFunShowNotificationReportsCB.call((SCITER_STRING)csMsgBox);
			}
		}
		pRptDlg->m_svFunDeleteReportsFinishedCB.call();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumReportsDlg::OnBnClickedBtnDelete.", 0, 0, true, SECONDLEVEL);
	}
	AddLogEntry(L">>>Report entry deleted successfully", 0, 0, true, FIRSTLEVEL);
Cleanup:
	pRptDlg->m_bReportThreadStart = false;
	pRptDlg->m_bReportEnableDisable = false;
	return dwRet;
}


/**********************************************************************************************************
*  Function Name  :	SendReportsData2Service
*  Description    :	Sends data to service to reload the existing db file before performing deletion of selected entries
*  SR.NO		  :
*  Author Name    : Prajakta
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CWardWizReports::SendReportsData2Service(DWORD dwMessageInfo, DWORD dwType, CString csEntry, bool bReportsWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = static_cast<int>(dwMessageInfo);
		szPipeData.dwValue = dwType;
		_tcscpy(szPipeData.szFirstParam, csEntry);
		CString csErrorMessage = L"";
		csErrorMessage.Format(L"### Failed to send data in CWardWizReports::SendReportsData2Service:: MessagInfo:: %d", dwMessageInfo);
		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(csErrorMessage, 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bReportsWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CWardwizReports::SendReportsData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::SendReportsData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
/**********************************************************************************************************
*  Function Name  :	SendReportsOperation2Service
*  Description    :	Sends data to service to save the remaining entries in db file after selected entries are deleted succesfully
*  SR.NO		  :
*  Author Name    : Prajakta
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CWardWizReports::SendReportsOperation2Service(DWORD dwType, CString csDateTime, CString csScanType, CString csFilePath, bool bReportsWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = dwType;
		szPipeData.dwValue = 0x05;
		wcscpy_s(szPipeData.szFirstParam, csDateTime);
		wcscpy_s(szPipeData.szSecondParam, csScanType);
		wcscpy_s(szPipeData.szThirdParam, csFilePath);

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumReports : SendReportsOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bReportsWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumReports : SendReportsOperation2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::SendReportsOperation2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	On_PauseReportsLoad
*  Description    :	Pause Loading of Reports
*  SR.NO		  :
*  Author Name    : Jeena Mariam Saji
*  Date           : 02 August 2016
**********************************************************************************************************/
json::value CWardWizReports::On_PauseReportsLoad()
{
	try
	{
		if (m_hThreadLoadReports != NULL && m_bDeleteEntriesFinished != true)
		{
			m_bReportStop = true;
			if (::SuspendThread(m_hThreadLoadReports) == -1)
			{
				AddLogEntry(L"### Failed to pause reports load operation in CWardwizReports::On_PauseReportsLoad", 0, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Reports loading operation Paused.", 0, 0, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_PauseReportsLoad", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_ResumeReportsLoad
*  Description    : Resume Loading of Reports
*  SR.NO		  :
*  Author Name    : Jeena Mariam Saji
*  Date           : 02 August 2016
**********************************************************************************************************/
json::value CWardWizReports::On_ResumeReportsLoad()
{
	try
	{
		if (m_hThreadLoadReports != NULL && m_bDeleteEntriesFinished != true)
		{
			m_bReportStop = false;
			if (::ResumeThread(m_hThreadLoadReports))
			{
				AddLogEntry(L"### Failed to resume reports loading operation in CWardwizReports::On_ResumeReportsLoad", 0, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Reports loading operation Resumed.", 0, 0, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_ResumeReportsLoad", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_StopReportsLoad
*  Description    : Stop Loading of Reports
*  SR.NO		  :
*  Author Name    : Jeena Mariam Saji
*  Date           : 02 August 2016
**********************************************************************************************************/
json::value CWardWizReports::On_StopReportsLoad()
{
	try
	{
		if (m_hThreadLoadReports)
		{
			::SuspendThread(m_hThreadLoadReports);
			if (::TerminateThread(m_hThreadLoadReports, 0x00))
			{
				AddLogEntry(L">>> Loading Reports thread stopped successfully in CWardwizReports::On_StopReportsLoad", 0, 0, true, ZEROLEVEL);
				m_hThreadLoadReports = NULL;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_StopReportsLoad", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_PauseReportsDelete
*  Description    :	Pause Deleting of Reports
*  SR.NO		  :
*  Author Name    : Jeena Mariam Saji
*  Date           : 02 August 2016
**********************************************************************************************************/
json::value CWardWizReports::On_PauseReportsDelete()
{
	try
	{
		if (m_hThreadDeleteReports != NULL && m_bDeleteEntriesFinished != true)
		{
			m_bReportStop = true;
			if (::SuspendThread(m_hThreadDeleteReports) == -1)
			{
				AddLogEntry(L"### Failed to pause reports delete operation in CWardwizReports::On_PauseReportsDelete", 0, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Reports deletion operation Paused.", 0, 0, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_PauseReportsDelete", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_ResumeReportsDelete
*  Description    : Resume Deleting of Reports
*  SR.NO		  :
*  Author Name    : Jeena Mariam Saji
*  Date           : 02 August 2016
**********************************************************************************************************/
json::value CWardWizReports::On_ResumeReportsDelete()
{
	try
	{
		if (m_hThreadDeleteReports != NULL && m_bDeleteEntriesFinished != true)
		{
			m_bReportStop = false;
			if (::ResumeThread(m_hThreadDeleteReports))
			{
				AddLogEntry(L"### Failed to resume reports delete operation in CWardwizReports::On_ResumeReportsDelete", 0, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Reports deletion operation Resumed.", 0, 0, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_ResumeReportsDelete", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_StopReportsDelete
*  Description    : Stop Deleting of Reports
*  SR.NO		  :
*  Author Name    : Jeena Mariam Saji
*  Date           : 02 August 2016
**********************************************************************************************************/
json::value CWardWizReports::On_StopReportsDelete()
{
	try
	{
		if (m_hThreadDeleteReports)
		{
			::SuspendThread(m_hThreadDeleteReports);
			if (::TerminateThread(m_hThreadDeleteReports, 0x00))
			{
				AddLogEntry(L">>> Deleting Reports thread stopped successfully in CWardwizReports::On_StopReportsDelete", 0, 0, true, ZEROLEVEL);
				m_hThreadDeleteReports = NULL;
			}
			if (!SendReportsOperation2Service(SAVE_REPORTS_ENTRIES, L"", L"", L"", true))
			{
				AddLogEntry(L"### Failed to send SAVE_REPORTS_ENTRIES in CWardwizReports::On_StopReportsDelete", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_StopReportsDelete", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : This function is used send reports DB file path to UI
*  Author Name    : Adil Sheikh
*  Date           :	19 Sept 2016
****************************************************************************************************/
json::value CWardWizReports::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBALLREPORTS.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : On_funOnSaveRecords
*  Description    : This function is used send reports DB file path to UI
*  Author Name    : Ram Shelke
*  Date           :	21 Nov 2017
****************************************************************************************************/
json::value CWardWizReports::On_funOnSaveRecords()
{
	try
	{
		static TCHAR szEncFilter[] = L"All Files(*.*)|*.*|";
		CFileDialog fileDlg(FALSE, _T("txt"), _T("*.txt"),
			OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szEncFilter);
		
		if (fileDlg.DoModal() != IDOK)
		{
			return "";
		}

		CString csFilePath = fileDlg.GetPathName();
		CString csValue = L"file://";
		csValue += csFilePath;
		csValue.Replace(L"\\", L"//");

		sciter::value svalue(csValue.GetBuffer());
		return svalue;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizReports::On_funOnSaveRecords", 0, 0, true, SECONDLEVEL);
	}
	return "";
}