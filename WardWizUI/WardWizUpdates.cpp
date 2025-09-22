/**********************************************************************************************************
Program Name          : WardWizLiveUpdate.cpp
Description           : This class contains the functionality for updating product from local folder.
						It has 2 options
						1) Update from internet.
						2) Update from Local Folder.
Author Name			  : Nihar Deshpande
Date Of Creation      : 27th May 2016
Version No            : 2.0.0.14
Special Logic Used    :

Modification Log      :
1. Nihar           Created WardWizLiveUpdate class   27th May 2016
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizUI.h"
#include "WardWizUpdates.h"
#include "WardWizUIDlg.h"
#include "WrdwizEncDecManager.h"
#include "WardWizDatabaseInterface.h"

DWORD WINAPI UpdateFromLocalFolderThread(LPVOID lpvThreadParam);
INT64 EnterUpdateDetails(int iFileCount, int iFileSize);
/***********************************************************************************************
*  Function Name  : CWardWizUpdates
*  Description    : Const'r
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
CWardWizUpdates::CWardWizUpdates() : behavior_factory("WardWizUpdates")
, m_objIPCALUpdateClient(ALUPDATE)
, m_dwCurrentLocalFileSize(0x00)
, m_iTotalNoofFiles(0x00)
{

}

/***********************************************************************************************
*  Function Name  : CWardWizUpdates
*  Description    : dest'r
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
CWardWizUpdates::~CWardWizUpdates()
{
}

/***********************************************************************************************
Function Name  : On_ClickUpdateFromInternet
Description    :On line update
SR.NO		   :
Author Name    : Nihar Deshpande
Date           : 19-05-2016
***********************************************************************************************/
json::value CWardWizUpdates::On_ClickUpdateFromInternet(SCITER_VALUE pSetUpdateStatusCb, SCITER_VALUE m_pAddUpdateTableCb, SCITER_VALUE m_pUpdateUpdateTableCb, SCITER_VALUE m_pRowAddCb, SCITER_VALUE m_pUpdateCompleteCb)
{
	try
	{
		m_svLiveUpdateStatusFunctionCB = pSetUpdateStatusCb;
		m_svAddUpdateTableCb = m_pAddUpdateTableCb;
		m_svUpdateUpdateTableCb = m_pUpdateUpdateTableCb;
		m_svRowAddCb = m_pRowAddCb;
		m_svUpdateCompleteCb = m_pUpdateCompleteCb;
		m_updateType = UPDATEFROMINTERNT;
		m_dwCurrentLocalFileSize = 0x00;
		m_iTotalNoofFiles = 0;
		m_iUpdateID = EnterUpdateDetails(0, 0);
		OnBnClickedButtonNext();
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::On_ClickUpdateFromInternet", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_PasueResumeUpdate
*  Description    : Accepts the request from UI and pauses and resumes the updates
*  Author Name    : Nihar Deshpande
*  Date			  : 19-05-2016
****************************************************************************************************/
json::value CWardWizUpdates::On_PauseUpdates()
{
	try
	{
		if (theApp.m_bUpdates)
		{
			PauseUpdates();
			theApp.m_bUpdates = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::On_PauseUpdates", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : On_PasueResumeUpdate
*  Description    : Accepts the request from UI and pauses and resumes the updates
*  Author Name    : Nihar Deshpande
*  Date			  : 19-05-2016
****************************************************************************************************/
json::value CWardWizUpdates::On_ResumeUpdates()
{
	try
	{
		if (!theApp.m_bUpdates)
		{
			ResumeUpdates();
			theApp.m_bUpdates = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::On_ResumeUpdates", 0, 0, true, SECONDLEVEL);
	}
	return json::value();
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonSTOP
*  Description    : It shows message whether downloading should stop or not.
*  Author Name    :	Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
****************************************************************************************************/
json::value CWardWizUpdates::On_ClickStopUpdates(SCITER_VALUE svIsStopFrmTaskbar, SCITER_VALUE svbIsManualStop)
{
	try
	{
		StopUpdates(svIsStopFrmTaskbar, svbIsManualStop);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizUIDlg::On_ClickStopUpdates", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : On_ClickUpdatesFromLocalFolder
Description    : Local Folder Updates
SR.NO		   :
Author Name    : Nihar Deshpande
Date           : 19-05-2016
***********************************************************************************************/
json::value CWardWizUpdates::On_ClickUpdatesFromLocalFolder(SCITER_VALUE svFilePath, SCITER_VALUE svNotificationMessageFromLocalCB)
{
	try
	{
		m_UnzipFile = NULL;
		m_StopUnRarOperation = NULL;
		LoadExtractDll();  //load extractdll
		m_bAborted = false;

		m_bOfflineUpdateStarted = false;
		m_svNotificationMessageFromLocalCB = svNotificationMessageFromLocalCB;
		SCITER_STRING  strFilePath = svFilePath.get(L"");
		CString csFilePath = strFilePath.c_str();
		m_updateType = UPDATEFROMLOCALFOLDER;
		m_csInputFolderPath = csFilePath;
		OnBnClickedButtonNext();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CVibraniumUpdates::On_ClickUpdatesFromLocalFolder", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/**********************************************************************************************************
*  Function Name  :	GetSQLiteDBFilePath
*  Description    :	Helper function to get Current working directory path
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
CString GetSQLiteDBFilePath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';
		return(CString(szModulePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CVibraniumUpdates::GetSQLiteDBFilePath", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertSQLData
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 InsertSQLData(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable in VibraniumAutoScnDlg- AutoScanner entered", 0, 0, true, ZEROLEVEL);
	try
	{
		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
		objSqlDb.Open();
		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iRowId = objSqlDb.GetLastRowId();
		objSqlDb.Close();
		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUpdates::InsertSQLData", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	UpdateUpdaterDetails
*  Description    :	Helper function, invoked to update the record inserted with appropriate update related details
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 UpdateUpdaterDetails(INT64 iUpdateID , int iFileCount, int iFileSize)
{
	CString csInsertQuery = _T("UPDATE Wardwiz_UpdatesMaster VALUES (null,");
	try
	{
		csInsertQuery.Format(_T("UPDATE Wardwiz_UpdatesMaster SET db_UpdateEndTime= datetime('now','localtime'), db_FilesDownloadCount= %d, db_DownloadFileSize = %d WHERE db_UpdateId = %d"), iFileCount, iFileSize, iUpdateID);
		CT2A ascii(csInsertQuery, CP_UTF8);
		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		if (!PathFileExists(csWardWizReportsPath))
		{
			CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			objSqlDb.Close();
		}

		INT64 iRowId = InsertSQLData(ascii.m_psz);
		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUpdates::UpdateUpdaterDetails. The query is : ", csInsertQuery, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	EnterUpdateDetails
*  Description    :	Helper function, invoked to INSERT the record appropriate update related details
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 EnterUpdateDetails(int iFileCount, int iFileSize)
{
	try
	{
		CString csInsertQuery = _T("INSERT INTO Wardwiz_UpdatesMaster VALUES (null,");
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_UpdatesMaster VALUES(null,datetime('now','localtime'), datetime('now','localtime'),date('now'),date('now'),%d,%d);"), iFileCount, iFileSize);

		CT2A ascii(csInsertQuery, CP_UTF8);

		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);

		if (!PathFileExists(csWardWizReportsPath))
		{
			CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			objSqlDb.Close();
		}

		INT64 iRowId = InsertSQLData(ascii.m_psz);
		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUpdates::EnterUpdateDetails", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}
/***********************************************************************************************
*  Function Name  : OnBnClickedButtonNext
*  Description    : After clicking Next button. It shows it's child dialog
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-5-2016
*************************************************************************************************/
void CWardWizUpdates::OnBnClickedButtonNext()
{
	m_bOlderdatabase = false;
	m_bdatabaseuptodate = false;
	try
	{
		m_objWardWizLiveUpdate.m_svUpdateDownloadTypeCB = m_svFunDisplayDownloadType;
		if (m_updateType == NONE)
		{
			//MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_LIVE_UPDATE_OPTION"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			//m_svNotificationMessageFromLocalCB.call(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_LIVE_UPDATE_OPTION"));
			CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_LIVE_UPDATE_OPTION"));
			return;
		}
		
		switch (m_updateType)
		{
		case UPDATEFROMINTERNT:
			m_objWardWizLiveUpdate.m_svUpdateStatusFunctionCB = m_svLiveUpdateStatusFunctionCB;
			m_objWardWizLiveUpdate.StartDownloading();
			m_objWardWizLiveUpdate.m_svLiveAddUpdateTableCb = m_svAddUpdateTableCb;
			m_objWardWizLiveUpdate.m_svLiveUpdateUpdateTableCb = m_svUpdateUpdateTableCb;
			m_objWardWizLiveUpdate.m_svLiveRowAddCb = m_svRowAddCb;
			m_objWardWizLiveUpdate.m_svLiveUpdateCompleteCb = m_svUpdateCompleteCb;
			m_objWardWizLiveUpdate.m_iDBUpdateID = m_iUpdateID;
			m_objWardWizLiveUpdate.m_bIsStopFrmTaskbar = false;
			m_objWardWizLiveUpdate.m_bIsManualStop = false;
			m_objWardWizLiveUpdate.m_iTotalFileSize = 0;
			m_objWardWizLiveUpdate.m_iCurrentDownloadBytes = 0;
			m_objWardWizLiveUpdate.m_csDownloadPercentage = L"";
			m_objWardWizLiveUpdate.m_dwPercentage = 0;
			StartALUpdateUsingALupdateService();
			break;
		case UPDATEFROMLOCALFOLDER:
			m_hUpdateFromLocalFolderThread = NULL;
			m_hUpdateFromLocalFolderThread = ::CreateThread(NULL, 0, UpdateFromLocalFolderThread, (LPVOID) this, 0, NULL);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUpdates::OnBnClickedButtonNext", 0, 0, true, SECONDLEVEL);
	}
}

DWORD WINAPI UpdateFromLocalFolderThread(LPVOID lpvThreadParam)
{
	CWardWizUpdates *pThis = (CWardWizUpdates*)lpvThreadParam;
	if (!pThis)
		return 1;
	CString csInputFileName = pThis->m_csInputFolderPath;
	pThis->m_dwMaxVersionInZip = 0x00;
	pThis->m_bOfflineUpdateStarted = true;
	if (pThis->m_csInputFolderPath.Compare(L"") == 0)
	{
		pThis->m_csInputFolderPath = L"";
		pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_FOLDER_PATH"));
		return 0;
	}

	if (!PathFileExists(csInputFileName))
	{
		pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_FOLDER_NO_PATH"));
		return 0;
	}

	DWORD dwUnzipCount = 0;
	csInputFileName = pThis->ExtractRARFile(csInputFileName, dwUnzipCount);
	if (dwUnzipCount == 0)
	{
		pThis->m_bOfflineUpdateStarted = false;
		pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_INVALID_MSG1"));
		//break;
		return 0;
	}
	std::vector<CString> csVectInputFiles;
	if (!pThis->CheckForValidUpdatedFiles(csInputFileName, csVectInputFiles))
	{
		if (pThis->m_bOlderdatabase)
		{
			pThis->m_bOlderdatabase = false;
			pThis->EnumAndDeleteTempFolder(csInputFileName);
			pThis->m_bOfflineUpdateStarted = false;
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_DB_FILES_ARE_OLDER"));
			return 0;
		}
		else if (pThis->m_bdatabaseuptodate)
		{
			pThis->m_bdatabaseuptodate = false;
			pThis->EnumAndDeleteTempFolder(csInputFileName);
			pThis->m_bOfflineUpdateStarted = false;
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDC_STATIC_UPDATED_DATABASE"));
			return 0;
		}
		else
		{
			pThis->EnumAndDeleteTempFolder(csInputFileName);
			pThis->m_bOfflineUpdateStarted = false;
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_INVALID_MSG1"));
		}
		return 0;
	}

	int dwUpdateLocalRet = pThis->m_objWardWizLiveUpdate.UpdateFromLocalFolder(csVectInputFiles);
	pThis->EnumAndDeleteTempFolder(csInputFileName);
	if (dwUpdateLocalRet == 0x01)
	{
		if (!pThis->UpdateVersionIntoRegistry())
		{
			AddLogEntry(L"### Failed to update database version into registry", 0, 0, true, SECONDLEVEL);
		}
		pThis->m_objWardWizLiveUpdate.m_bisDateTime = true;
		pThis->m_objWardWizLiveUpdate.UpdateTimeDate();

		pThis->m_bOfflineUpdateStarted = false;
		pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_DB_UPDATE_SUCCESS"));
		return 0;
	}
	if (dwUpdateLocalRet == 0x03)
	{
		if (pThis->m_bOlderdatabase)
		{
			pThis->m_bOlderdatabase = false;
			
			pThis->m_bOfflineUpdateStarted = false;
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_DB_FILES_ARE_OLDER"));
			return 0;
		}
		else
		{
			pThis->m_bOfflineUpdateStarted = false;
			pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE"));
			return 0;
		}
	}
	if (dwUpdateLocalRet == 0x02)
	{
		if (!pThis->UpdateVersionIntoRegistry())
		{
			AddLogEntry(L"### Failed to update database version into registry", 0, 0, true, SECONDLEVEL);
		}
		pThis->m_objWardWizLiveUpdate.m_bisDateTime = true;
		pThis->m_objWardWizLiveUpdate.UpdateTimeDate();
		
		pThis->m_bOfflineUpdateStarted = false;
		pThis->CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_SUCCESS"));

	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : UpdateVersionIntoRegistry
*  Description    : Upadte Version from registry
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdates::UpdateVersionIntoRegistry()
{
	CString csVersionregValue = L"";
	csVersionregValue = m_csVersionNo;
	csVersionregValue.Trim();
	try
	{
		if (SendData2CommService(RELOAD_SIGNATURE_DATABASE, false) != 0x00)
		{
			AddLogEntry(L"### Failed to send reload Database message to service CVibraniumUpdates::UpdateVersionIntoRegistry", 0, 0, true, SECONDLEVEL);
		}
		if (!m_objWardWizLiveUpdate.SendRegistryData2Service(SZ_STRING, theApp.m_csRegKeyPath.GetBuffer(),
        			_T("DataBaseVersion"), (LPTSTR)csVersionregValue.GetBuffer(), true))
		{
			AddLogEntry(L"### Failed to DataBaseVersion SendRegistryData2Service CVibraniumUpdates::UpdateVersionIntoRegistry", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUpdates::UpdateVersionIntoRegistry", 0, 0, true, SECONDLEVEL);

	}
	return true;
}

/***********************************************************************************************
*  Function Name  : CheckForValidUpdatedFiles
*  Description    : Checks files in folder are valid or not
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdates::CheckForValidUpdatedFiles(CString csInputFolder, std::vector<CString> &csVectInputFiles)
{
	bool bReturn = false;
	int iMatchCount = 0;
	int iMisMatchedCount = 0;

	TCHAR szDataBaseFolder[MAX_PATH] = { 0 };
	try
	{
		if (theApp.m_eScanLevel == CLAMSCANNER)
		{
			_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"DB");
			if (!PathFileExists(szDataBaseFolder))
			{
				if (CreateDirectoryFocefully(szDataBaseFolder))
				{
					AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
				}
			}

			_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"VBDB");
			if (!PathFileExists(szDataBaseFolder))
			{
				if (CreateDirectoryFocefully(szDataBaseFolder))
				{
					AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
				}
			}
		}
		else
		{
			_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"VBDB");
			if (!PathFileExists(szDataBaseFolder))
			{
				if (CreateDirectoryFocefully(szDataBaseFolder))
				{
					AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
				}
			}
		}

		if (m_csFilesList.GetCount() > 0)
		{
			m_csFilesList.RemoveAll();
		}

		EnumFolder(csInputFolder);

		for (int iIndex = 0; iIndex < m_csFilesList.GetCount(); iIndex++)
		{
			CString csInputPathProgramData = NULL;
			CString csInputPath = csInputFolder + L"\\" + m_csFilesList[iIndex];
			if (PathFileExists(csInputPath))
			{
				DWORD	dwStatus = 0x00;
				dwStatus = ValidateDB_File((LPTSTR)csInputPath.GetBuffer(), dwStatus, csInputPathProgramData);
				if (dwStatus == 0x08)
				{
					iMisMatchedCount++;
				}
				if (dwStatus == 0x00)
				{
					csInputPath = csInputPathProgramData;
					csVectInputFiles.push_back(csInputPath);
					iMatchCount++;
				}
			}
		}
		
		//UpdateUpdaterDetails(m_iUpdateID,(int) m_csFilesList.GetCount(), m_dwCurrentLocalFileSize);
		if (iMatchCount >= 1)
		{
			bReturn = true;
		}
		else
		{
			bReturn = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUpdates::CheckForValidUpdatedFiles", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
/***********************************************************************************************
*  Function Name  : ExtractRARFile
*  Description    : Function to Unrar the file for Local updates
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
CString CWardWizUpdates::ExtractRARFile(CString csInputFileName, DWORD &dwUnzipCount)
{
	TCHAR szUnzipPath[512] = { 0 };
	try
	{
		if (m_UnzipFile != NULL)
		{
			if (m_UnzipFile(csInputFileName.GetBuffer(), szUnzipPath, dwUnzipCount) != 0x00)
			{
				if (!m_bAborted)
				{
					AddLogEntry(L"### m_UnzipFile Failed in CVibraniumUpdates::ExtractRARFile", 0, 0, true, SECONDLEVEL);
				}
				return CString(L"");
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::ExtractRARFile", 0, 0, true, SECONDLEVEL);
	}
	return CString(szUnzipPath);
}

/***************************************************************************************************
*  Function Name  :   GetAppFolderPath
*  Description    :   Get App folder path.
*  Author Name    :   Nihar Deshpande
*  SR_NO		  :
*  Date           :   19-05-2016
****************************************************************************************************/
CString CWardWizUpdates::GetAppFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csAppFolderPath = szModulePath;
		return csAppFolderPath;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::GetAppFolderPath()", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************************************
*  Function Name  : CreateDirectoryFocefully()
*  Description    : It will check directory. If not present, it will create that directory.
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date			  :	19-05-2016
****************************************************************************************************/
bool CWardWizUpdates::CreateDirectoryFocefully(LPTSTR lpszPath)
{
	__try
	{
		CreateDirectory(lpszPath, NULL);
		if (PathFileExists(lpszPath))
			return false;

		_wmkdir(lpszPath);
		if (PathFileExists(lpszPath))
			return false;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### VibraniumALUSrv::CreateDirectoryFocefully::Exception", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***********************************************************************************************
*  Function Name  : ValidateDB_File
*  Description    : checking signature and valid version no and size of files
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*  Modifiled date :	6/6/2014 neha gharge
//Modified:
//Issue No 107	Doesn’t check the contents of the DB file. Even if DB contains garbage its accepted
//ITin_Developement_Release_1.0.0.3_Patches
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
*  Modified Date  : 6/2/2015 Neha Gharge FP file added
*************************************************************************************************/
DWORD CWardWizUpdates::ValidateDB_File(TCHAR *m_szFilePath, DWORD &dwStatus, CString &csInputPathProgramData)
{
	DWORD	dwRet = 0x00;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00, dwDecBytesRead = 0x00;
	TCHAR	szTemp[1024] = { 0 }, szFileName[1024] = { 0 };
	TCHAR	szExt[16] = { 0 };
	DWORD	dwLen = 0x00;
	LPBYTE	lpFileData = NULL;
	LPBYTE	lpEncryptionSignature = (LPBYTE)"WRDWIZDB";
	DWORD	dwDecKeySize = 0x00;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE;
	TCHAR   szWholeSignature[0x30] = { 0 };
	DWORD   dwRetCheckversion = 0x00;
	CString csFileName, csActualWRDWIZFilePath, csActualClamFilePath;
	int j = 0;
	try
	{
		if (!m_szFilePath)
			return 0x05;
		m_CurrentFilePath = L"";
		m_CurrentFilePath.Format(L"%s", m_szFilePath);
		csFileName = m_CurrentFilePath.Mid(m_CurrentFilePath.ReverseFind('\\') + 1);
		if (theApp.m_eScanLevel == CLAMSCANNER)
		{
			if ((csFileName.CompareNoCase(L"DAILY.CLD") == 0) || csFileName.CompareNoCase(L"MAIN.CVD") == 0 || csFileName.CompareNoCase(L"WRDWIZWHLST.FP") == 0)
			{
				csActualClamFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"DB", csFileName);
				if (!PathFileExists(csActualClamFilePath))
				{
					AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
					csInputPathProgramData = m_szFilePath;
					dwRet = 0x00;
					goto Cleanup;
				}
			}
			else
			{
				csActualWRDWIZFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"VBDB", csFileName);
				if (!PathFileExists(csActualWRDWIZFilePath))
				{
					AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
					csInputPathProgramData = m_szFilePath;
					dwRet = 0x00;
					goto Cleanup;
				}
			}
		}
		else
		{
			if ((csFileName.CompareNoCase(L"DAILY.CLD") == 0) || csFileName.CompareNoCase(L"MAIN.CVD") == 0 || csFileName.CompareNoCase(L"WRDWIZWHLST.FP") == 0)
			{
				dwRet = 0x08;
				goto Cleanup;
			}
			csActualWRDWIZFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"VBDB", csFileName);
			if (!PathFileExists(csActualWRDWIZFilePath))
			{
				AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
				csInputPathProgramData = m_szFilePath;
				dwRet = 0x00;
				goto Cleanup;
			}
		}
		if (!PathFileExists(m_szFilePath))
		{
			dwRet = 0x01;
			goto Cleanup;
		}
		dwLen = static_cast<DWORD>(wcslen(m_szFilePath));
		if ((dwLen < 0x08) || (dwLen > 0x400))
		{
			dwRet = 0x02;
			goto Cleanup;
		}
		DWORD dwDBVersionLength = 0;
		DWORD dwDBMajorVersion= 0;
		CWrdwizEncDecManager objWrdwizEncDecMgr;
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(m_szFilePath, dwDBVersionLength, dwDBMajorVersion))
		{
			AddLogEntry(L"### Wardwiz Signature criteria not matched");
			dwRet = 0x08;		//Invalid files
			goto Cleanup;
		}
		dwDecKeySize = MAX_SIG_SIZE + dwDBVersionLength + MAX_TOKENSTRINGLEN;
		hFile = CreateFile(m_szFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### Updates : Error in opening existing Database file %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup;
		}
		dwFileSize = GetFileSize(hFile, NULL);
		if (!dwFileSize)
		{
			AddLogEntry(L"### Updates : Error in GetFileSize of %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x03;
			goto Cleanup;
		}
		m_dwCurrentLocalFileSize = dwFileSize;
		if (lpFileData != NULL)
		{
			free(lpFileData);
			lpFileData = NULL;
		}
		dwBytesRead = 0x00;
		unsigned char bySigBuff[0x30] = { 0x00 };
		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
		ReadFile(hFile, &bySigBuff[0x0], 0x30, &dwBytesRead, NULL);
		if (dwBytesRead != 0x30)
		{
			AddLogEntry(L"### Updates : Error in ReadFile while reading signature %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
		char szFileName[0x11] = { 0 };
		memcpy(szFileName, &bySigBuff[0], 0x0A);
		TCHAR szDBFileName[0x11] = { 0 };
		//convert multibyte string to unicode
		size_t convertedChars;
		mbstowcs_s(&convertedChars, szDBFileName, strlen(szFileName) + 1, szFileName, _TRUNCATE);
		char *szVersionNumber = new char[dwDBVersionLength + 1];
		memset(szVersionNumber, 0, dwDBVersionLength + 1);
		memcpy(szVersionNumber, &bySigBuff[0x0B], dwDBVersionLength);
		//szVersionNumber[strlen(szVersionNumber) + 1] = '\0';
		TCHAR * szDBVersionNumber = new TCHAR[dwDBVersionLength + 1];
		mbstowcs_s(&convertedChars, szDBVersionNumber, strlen(szVersionNumber) + 1, szVersionNumber, _TRUNCATE);
		if (!ValidateFileNVersion(szDBFileName, szDBVersionNumber, dwDBVersionLength))
		{
			AddLogEntry(L"### Updates : Invalidate File", 0, 0, true, SECONDLEVEL);
			dwRet = 0x08;
			goto Cleanup;
		}
		if (szVersionNumber != NULL)
		{
			delete[]szVersionNumber;
			szVersionNumber = NULL;
		}
		if (szDBVersionNumber != NULL)
		{
			delete[]szDBVersionNumber;
			szDBVersionNumber = NULL;
		}
		lpFileData = (LPBYTE)malloc(dwFileSize - dwDecKeySize);
		if (!lpFileData)
		{
			AddLogEntry(L"### Updates : Error in allocation of memory", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
		memset(lpFileData, 0x00, (dwFileSize - dwDecKeySize));
		SetFilePointer(hFile, (0x00 + dwDecKeySize), NULL, FILE_BEGIN);
		ReadFile(hFile, lpFileData, (dwFileSize - dwDecKeySize), &dwBytesRead, NULL);
		if ((dwFileSize - dwDecKeySize) != dwBytesRead)
		{
			AddLogEntry(L"### Updates : Error in ReadFile %s", m_szFilePath, 0, true, FIRSTLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
		CString csScanLogFullPath;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		memset(szModulePath, 0x00, MAX_PATH * sizeof(TCHAR));
		GetEnvironmentVariable(L"ALLUSERSPROFILE", szModulePath, MAX_PATH);
		csScanLogFullPath = szModulePath;
		csScanLogFullPath += L"\\Wardwiz Antivirus";
		CString FileName = L"";
		csInputPathProgramData = m_szFilePath;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdatesDlg::Database_File_Updation", 0, 0, true, SECONDLEVEL);
		return false;
	}
Cleanup:

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	if (lpFileData)
	free(lpFileData);
	lpFileData = NULL;
	return dwRet;
}

/***********************************************************************************************
*  Function Name  : ValidateFileNVersion
*  Description    : checks for validate signature
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdates::ValidateFileNVersion(CString csFileName, CString csVersion, DWORD dwDBVersionLength)
{
	bool bReturn = false;
	m_csFileName = csFileName;
	try
	{
		int iRet = CheckForValidVersion(csVersion);
		if (iRet == 0x07)
		{
			m_bdatabaseuptodate = true;
			return false;
		}

		if (iRet == 0x08)
		{
			if (CheckForMaxVersionInZip(csVersion))
			{
				m_csVersionNo = csVersion;
			}
			return true;
		}

		if (iRet == 0x09)
		{
			m_bdatabaseuptodate = true;
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::ValidateFileNVersion", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : CheckForValidVersion
*  Description    : checking  valid version no
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-05-2016
*************************************************************************************************/
DWORD CWardWizUpdates::CheckForValidVersion(CString csVersionNo)
{
	TCHAR szVersionNo[16] = { 0 };
	TCHAR szVersion[5] = { 0 };
	DWORD dwVersionNoInRegistry = 0;
	DWORD dwVersionNo = 0;
	DWORD dwRet = 0x00;
	int j = 0;
	try
	{
		_tcscpy_s(szVersionNo, csVersionNo);
		for (int i = 0; i < _tcslen(szVersionNo); i++)
		{
			if (isdigit(szVersionNo[i]))
			{
				szVersion[j] = szVersionNo[i];
				j++;
			}
		}
		dwVersionNo = static_cast<DWORD>(wcstod(szVersion, _T('\0')));
		dwVersionNoInRegistry = ReadDBVersionFromReg();
		if (dwVersionNoInRegistry > dwVersionNo)
		{
			dwRet = 0x07;
			goto Cleanup;
		}
		if (dwVersionNoInRegistry < dwVersionNo)
		{
			dwRet = 0x08;
			goto Cleanup;
		}
		if (dwVersionNoInRegistry == dwVersionNo)
		{
			dwRet = 0x09;
			goto Cleanup;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::CheckForValidVersion", 0, 0, true, SECONDLEVEL);
	}
Cleanup: 
	return dwRet;
}

/***********************************************************************************************
*  Function Name  : EnumAndDeleteTempFolder
*  Description    : Function to delete the db files from temp location
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-05-2016
*************************************************************************************************/
bool CWardWizUpdates::EnumAndDeleteTempFolder(CString csInputFileName)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(csInputFileName);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString str = finder.GetFilePath();
				EnumAndDeleteTempFolder(str);
			}
			else
			{
				CString strFilePath = finder.GetFilePath();
				SetFileAttributes(strFilePath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(strFilePath);
			}
		}
		finder.Close();
		RemoveDirectory(csInputFileName);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::EnumAndDeleteTempFolder", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
*  Function Name  : CheckForMaxVersionInZip
*  Description    : 
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdates::CheckForMaxVersionInZip(CString csVersionNo)
{
	try
	{
		TCHAR szVersionNo[10] = { 0 };
		DWORD dwVersionNo = 0;
		TCHAR szVersion[5] = { 0 };
		int j = 0;

		_tcscpy_s(szVersionNo, csVersionNo);

		for (int i = 0; i < 9; i++)
		{
			if (isdigit(szVersionNo[i]))
			{
				szVersion[j] = szVersionNo[i];
				j++;
			}
		}
		dwVersionNo = static_cast<DWORD>(wcstod(szVersion, _T('\0')));
		if (m_dwMaxVersionInZip < dwVersionNo)
		{
			m_dwMaxVersionInZip = dwVersionNo;
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdatesDlg::CheckForMaxVersionInZip", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : ReadDBVersionFromReg
*  Description    : ReadVersion from registry
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
DWORD CWardWizUpdates::ReadDBVersionFromReg()
{
	DWORD dwRegVersionNo = 0x00;
	TCHAR szRegVersionNo[1024] = { 0 };
	TCHAR szRegVersion[10] = { 0 };
	DWORD dwvalue_length = 1024;
	DWORD dwtype = REG_SZ;
	HKEY key;
	int j = 0;
	try
	{
		if (RegOpenKey(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath, &key) != ERROR_SUCCESS)
		{
			//AddLogEntry(L"Unable to open registry key");
		}

		long ReadReg = RegQueryValueEx(key, L"DataBaseVersion", NULL, &dwtype, (LPBYTE)&szRegVersionNo, &dwvalue_length);

		if (ReadReg == ERROR_SUCCESS)
		{
			for (int i = 0; i < 9; i++)
			{
				if (isdigit(szRegVersionNo[i]))
				{
					szRegVersion[j] = szRegVersionNo[i];
					j++;
				}
			}
		}
		dwRegVersionNo = static_cast<DWORD>(wcstod(szRegVersion, _T('\0')));
		RegCloseKey(key);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUpdates::ReadDBVersionFromReg", 0, 0, true, SECONDLEVEL);

	}
	return dwRegVersionNo;
}

/***********************************************************************************************
*  Function Name  : EnumFolder
*  Description    : Enumrate all the files in folder.
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
void CWardWizUpdates::EnumFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		int i = 0;
		m_iTotalNoofFiles = 0;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				continue;
			}
			else
			{
				m_csFilesList.Add(finder.GetFileName());
				m_iTotalNoofFiles++;
			}
		}
		finder.Close();
		i = 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : StartALUpdateUsingALupdateService
*  Description    : Send a request to start Autp live update to ALU service through named pipe
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdates::StartALUpdateUsingALupdateService()
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = START_UPDATE;
		CISpyCommunicator objCom(AUTOUPDATESRV_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumUpdatesDlg::StartALUpdateUsingALupdateService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))//jump to liveupdate file UpDateDowloadStatus
		{
			AddLogEntry(L"### Failed to ReadData in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		OutputDebugString(L">>> Out CVibraniumUpdatesDlg::StartALUpdateUsingALupdateService");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdatesDlg::StartALUpdateUsingALupdateService", 0, 0, true, SECONDLEVEL);
	}

	m_objIPCALUpdateClient.OpenServerMemoryMappedFile();
	return true;
}

/***************************************************************************************************
*  Function Name  : ShutDownDownload
*  Description    : It shows message whether downloading should stop or not.
*  Author Name    :	Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
****************************************************************************************************/
bool CWardWizUpdates::ShutDownDownload()
{
	try
	{
		if (m_objWardWizLiveUpdate.ShutDownDownloadLiveupdates())
		{
			m_objWardWizLiveUpdate.m_isDownloading = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::ShutDownDownload", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  : PauseUpdate
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdates::PauseUpdates()
{
	try
	{ 
		m_objWardWizLiveUpdate.PauseUpdateLiveUpdate();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::PauseUpdates", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  : ResumeUpdate
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizUpdates::ResumeUpdates()
{
	try
	{
		m_objWardWizLiveUpdate.ResumeUpdateLiveUpdate();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::ResumeUpdates", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  : StopUpdate
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
/*********************************************************************************************************/
bool CWardWizUpdates::StopUpdates(SCITER_VALUE svIsStopFrmTaskbar, SCITER_VALUE svbIsManualStop)
{
	try
	{
		m_objWardWizLiveUpdate.m_bIsStopFrmTaskbar = svIsStopFrmTaskbar.get(false);
		m_objWardWizLiveUpdate.m_bIsManualStop = svbIsManualStop.get(false);
		m_objWardWizLiveUpdate.StopLiveUpdates();
		UpdateUpdaterDetails(m_iUpdateID, m_iTotalNoofFiles, m_dwCurrentLocalFileSize);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::StopUpdates", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  : On_SendNotificationMessageCB
*  Description    : Show notifications messages from backend.
*  Author Name    :	Amol Jaware
*  SR_NO		  :
*  Date           : 05-08-2016
/*********************************************************************************************************/
json::value CWardWizUpdates::On_SendNotificationMessageCB(SCITER_VALUE svFunNotificationMessageCB)
{
	try
	{
		m_objWardWizLiveUpdate.m_svFunNotificationMessageCB = svFunNotificationMessageCB;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::On_SendNotificationMessageCB()", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  : On_PauaseOfflineUpdate
*  Description    : To pause the thread while update is on.
*  Author Name    :	Amol Jaware
*  SR_NO		  :
*  Date           : 20-09-2016
/*********************************************************************************************************/
json::value CWardWizUpdates::On_PauaseOfflineUpdate()
{
	try
	{
		if (m_bOfflineUpdateStarted == true)
		{
			if (m_hUpdateFromLocalFolderThread != NULL)
			{
				::SuspendThread(m_hUpdateFromLocalFolderThread);
			}
			m_bOfflineUpdateStarted = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::On_PauaseOfflineUpdate()", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  : On_ResumeOfflineUpdate
*  Description    : To resume the thread while update is on.
*  Author Name    :	Amol Jaware
*  SR_NO		  :
*  Date           : 20-09-2016
/*********************************************************************************************************/
json::value CWardWizUpdates::On_ResumeOfflineUpdate()
{
	try
	{
		if (m_bOfflineUpdateStarted == false)
		{
			if (m_hUpdateFromLocalFolderThread != NULL)
			{
				::ResumeThread(m_hUpdateFromLocalFolderThread);
			}
			m_bOfflineUpdateStarted = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::On_ResumeOfflineUpdate()", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  : On_CloseOfflineUpdate
*  Description    : To terminate the thread.
*  Author Name    :	Amol Jaware
*  SR_NO		  :
*  Date           : 20-09-2016
/*********************************************************************************************************/
json::value CWardWizUpdates::On_CloseOfflineUpdate()
{
	try
	{
		if (m_hUpdateFromLocalFolderThread != NULL)
		{
			m_csInputFolderPath = L""; //.def file removed from this variable
			m_bAborted = true;
			::ResumeThread(m_hUpdateFromLocalFolderThread);
			if (!m_StopUnRarOperation)
			{
				AddLogEntry(L"### StopUnrarOperation Function Address failed in CVibraniumUpdates::On_CloseOfflineUpdate", 0, 0, true, SECONDLEVEL);
				goto CLEANUP;
			}

			if (!m_StopUnRarOperation())
			{
				::SuspendThread(m_hUpdateFromLocalFolderThread);
				::TerminateThread(m_hUpdateFromLocalFolderThread, 0x00);
				goto CLEANUP;
			}

			DWORD dwTimeOut = 2 * 1000;//wait for 2 second
			DWORD dwWaitResult = WaitForSingleObject(m_hUpdateFromLocalFolderThread, dwTimeOut);  // no time-out interval

			switch (dwWaitResult)
			{
				// The thread got ownership of the mutex
			case WAIT_OBJECT_0:
				//Addlog
				m_hUpdateFromLocalFolderThread = NULL;
				break;
				// The database is in an indeterminate state
			case WAIT_TIMEOUT:
				//addlog
				break;
			}

			m_bOfflineUpdateStarted = false;

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::On_CloseOfflineUpdate()", 0, 0, true, SECONDLEVEL);
	}

CLEANUP:
	if (m_hZip != NULL) //unload extractdll library
	{
		FreeLibrary(m_hZip);
		m_hZip = NULL;
	}
	if (m_UnzipFile != NULL)
	{
		m_UnzipFile = NULL;
	}
	if (m_StopUnRarOperation != NULL)
	{
		m_StopUnRarOperation = NULL;
	}
	return 0;
}

bool CWardWizUpdates::LoadExtractDll()
{
	try
	{
		CString	csWardWizModulePath = GetModuleFilePath();
		CString	csWardWizExtractDLL = L"";
		csWardWizExtractDLL.Format(L"%s\\VBEXTRACT.DLL", csWardWizModulePath);
		if (!PathFileExists(csWardWizExtractDLL))
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_LOAD_FAILED_EXTRACT_DLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}

		m_hZip = LoadLibrary(csWardWizExtractDLL);
		if (!m_hZip)
		{
			AddLogEntry(L"### CExtractAttchForScan::Failed in loading VBEXTRACT.DLL", 0, 0, true, SECONDLEVEL);
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_LOAD_FAILED_EXTRACT_DLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_hZip = NULL;
			return false;
		}

		m_UnzipFile = (UNZIPFILE)GetProcAddress(m_hZip, "UnRarForUpdates");
		if (!m_UnzipFile)
		{
			AddLogEntry(L"### GetProcAddress UnRarForUpdates failed in CVibraniumUpdates::LoadExtractDll", 0, 0, true, SECONDLEVEL);
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_FUN_UNZIPFILE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_hZip = NULL;
			return false;
		}

		m_StopUnRarOperation = (STOPUNRAROPERATION)GetProcAddress(m_hZip, "StopUnrarOperation");
		if (!m_StopUnRarOperation)
		{
			AddLogEntry(L"### GetProcAddress StopUnrarOperation failed in CVibraniumUpdates::LoadExtractDll", 0, 0, true, SECONDLEVEL);
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_FUN_UNZIPFILE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_hZip = NULL;
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizGUIApp::LoadExtractDll", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


CString CWardWizUpdates::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';
	return(CString(szModulePath));
}

void CWardWizUpdates::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
{
	try
	{
		m_svNotificationMessageFromLocalCB.call(iMsgType, (SCITER_STRING)strMessageString);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		Sleep(700);
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdates::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : SendData2CommService
Description    : Send data to communication service.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 08 Apr 2015
***********************************************************************************************/
DWORD CWardWizUpdates::SendData2CommService(int iMesssageInfo, bool bWait)
{
	DWORD dwRet = 0x00;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMesssageInfo;
		CISpyCommunicator objCom(SERVICE_SERVER, false);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in SendData2CommService", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
			goto FAILED;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in SendData2CommService", 0, 0, true, SECONDLEVEL);
				dwRet = 0x02;
				goto FAILED;
			}

			if (szPipeData.dwValue != 1)
			{
				dwRet = 0x03;
				goto FAILED;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SendData2CommService", 0, 0, true, SECONDLEVEL);
		dwRet = 0x04;
	}
FAILED:
	return dwRet;
}

/***************************************************************************************************
Function Name  : Send_UpdateType
Description    : Send Update Type(Product update / only Database update)
Author Name    : Jeena Mariam Saji
SR_NO		   :
Date           : 12th Dec 2016
/***************************************************************************************************/
json::value CWardWizUpdates::Send_UpdateType(SCITER_VALUE m_csUpdateType)
{
	try
	{
		m_svFunDisplayDownloadType = m_csUpdateType;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::Send_UpdateType()", 0, 0, true, SECONDLEVEL);
	}
	return true;
}