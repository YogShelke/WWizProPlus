/**********************************************************************************************************
Program Name          : WardWizLiveUpdate.cpp
Description           : This class contains the functionality for updating product.
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
#include "WardWizLiveUpdate.h"
#include "WardWizUpdates.h"
#include "WardWizUI.h"
#include "WardWizDatabaseInterface.h"

//char* g_strWWzDatabasePath = "C:\\Program Files\\Vibranium\\VBALLREPORTS.DB";
CString					g_csPreviousListControlStatus = L"";
CString GetSQLiteDBFileSource();
CString g_csWardWizModulePath;
/***********************************************************************************************
*  Function Name  : CWardWizLiveUpdate
*  Description    : Const'r
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
CWardWizLiveUpdate::CWardWizLiveUpdate():
m_bIsStopFrmTaskbar(false),
m_bIsManualStop(false),
m_csDispTotSizeExt(_T("KB")),
m_csCurrentDownloadedbytes(_T("[0 KB]"))
{
	
}

/***********************************************************************************************
*  Function Name  : CWardWizLiveUpdate
*  Description    : Dest'r
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
CWardWizLiveUpdate::~CWardWizLiveUpdate()
{
}

/***********************************************************************************************
*  Function Name  : StartDownloading
*  Description    : Function which intialize variables which starts the download.
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
bool CWardWizLiveUpdate::StartDownloading()
{	
	bool bReturn = false;
	theApp.m_bUpdates = true;
	m_iFileCount = 0;
	m_iIntFilesSize = 0;
	m_iDBUpdateID = 0;
	try
	{
		ZeroMemory(m_szAllUserPath, sizeof(m_szAllUserPath));
		GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, 511);

		m_isDownloading = true;
		g_csPreviousListControlStatus = L"";
		m_bIsCheckingForUpdateEntryExistInList = true;
		m_iCount = 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::StartDownloading", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : SendRegistryData2Service
*  Description    : Send request to service to set registry through pipe.
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
bool CWardWizLiveUpdate::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = dwType;
		wcscpy_s(szPipeData.szFirstParam, szKey);
		wcscpy_s(szPipeData.szSecondParam, szValue);
		if (szData)
		{
			wcscpy_s(szPipeData.szThirdParam, szData);
		}
		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumLiveUpdate : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumLiveUpdate : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CVibraniumLiveUpdate::SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name	 : CopyFromLocalFolder2InstalledFolder
*  Description		 : Copy from the local folder to installation folder
*  Author Name		 : Nihar Deshpande
*  Date				 :  19-May-2016
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
*  Modified Date  : 6/2/2015 Neha Gharge FP file added
****************************************************************************************************/
DWORD CWardWizLiveUpdate::CopyFromLocalFolder2InstalledFolder(std::vector<CString> &csVectInputFiles)
{
	DWORD dwReturn = 0x00;
	try
	{
		m_bDownLoadInProgress = false;
		int iUpdateFileCount = 0;
		int iItemCount = static_cast<int>(csVectInputFiles.size());

		for (int iIndex = 0; iIndex < iItemCount; iIndex++)
		{
			CString csSourcePath = csVectInputFiles[iIndex];
			if (PathFileExists(csSourcePath))
			{
				CString csFileName;
				int iFound = csSourcePath.ReverseFind(L'\\');
				csFileName = csSourcePath.Right(csSourcePath.GetLength() - iFound - 1);
				CString csDestination;
				//Prajakta
				if (theApp.m_eScanLevel != WARDWIZSCANNER)
				{
					if ((csFileName == L"DAILY.CLD") || (csFileName == L"MAIN.CVD") || (csFileName == L"WRDWIZWHLST.FP"))
					{
						csDestination.Format(L"%s\\DB\\%s", theApp.GetModuleFilePath(), csFileName);
					}
					else
					{
						csDestination.Format(L"%s\\VBDB\\%s", theApp.GetModuleFilePath(), csFileName);
					}
				}
				else
				{
					CString csFileExt = csFileName.Right((csFileName.GetLength() - (csFileName.ReverseFind(L'.'))));
					if ((csFileName == L"DAILY.CLD") || (csFileName == L"MAIN.CVD") || (csFileName == L"WRDWIZWHLST.FP"))
					{
						continue;
					}
					if (csFileExt.CompareNoCase(L".DB") == 0)
					{
						csDestination.Format(L"%s\\VBDB\\%s", theApp.GetModuleFilePath(), csFileName);
					}
					else
					{
						csDestination.Format(L"%s\\PLUGINS\\%s", theApp.GetModuleFilePath(), csFileName);
					}
				}
				Sleep(10);
				if (MoveFileEx(csSourcePath, csDestination, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
				{
					iUpdateFileCount++;
				}
				else
				{
					AddLogEntry(L"### Error in CVibraniumLiveUpdate:FILE_OPERATIONS in SendLiveUpdateOperation2Service", 0, 0, true, SECONDLEVEL);
				}
			}
		}

		if (iUpdateFileCount <= 3)
		{
			dwReturn = 0x01;
			goto Cleanup;
		}
		else if (iUpdateFileCount >= 4)
		{
			dwReturn = 0x02;
			goto Cleanup;
		}
		else
		{
			dwReturn = 0x03;
			goto Cleanup;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::CopyDownloadedFiles2InstalledFolder", 0, 0, true, SECONDLEVEL);
		dwReturn = 0x03;
	}
Cleanup:
	return dwReturn;
}

/***************************************************************************************************
*  Function Name  : UpdateFromLocalFolder
*  Description    : Update from the local folder
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-05-2016
****************************************************************************************************/
DWORD CWardWizLiveUpdate::UpdateFromLocalFolder(std::vector<CString> &csVectInputFiles)
{
	try
	{
		return CopyFromLocalFolder2InstalledFolder(csVectInputFiles);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::UpdateFromLocalFolder", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : UpdateTimeDate
*  Description    : This function update date and time into registry after completion of download
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-05-2016
*************************************************************************************************/
void CWardWizLiveUpdate::UpdateTimeDate()
{
	try
	{
		CString  csDate, csTime;
		TCHAR szOutMessage[30] = { 0 };
		TCHAR tbuffer[9] = { 0 };
		TCHAR dbuffer[9] = { 0 };
		SYSTEMTIME  CurrTime = { 0 };
		GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
		CTime Time_Curr(CurrTime);
		int iMonth = Time_Curr.GetMonth();
		int iDate = Time_Curr.GetDay();
		int iYear = Time_Curr.GetYear();

		if (m_bisDateTime)
		{
			_wstrtime_s(tbuffer, 9);
			csTime.Format(L"%s\0\r\n", tbuffer);
			csDate.Format(L"%d/%d/%d", iMonth, iDate, iYear);
			_stprintf(szOutMessage, _T("%s %s\0"), csDate, tbuffer);
		}

		if (!SendRegistryData2Service(SZ_STRING, theApp.m_csRegKeyPath.GetBuffer(),
			_T("LastLiveupdatedt"), szOutMessage, true))
		{
			AddLogEntry(L"### Failed to LastLiveupdatedt SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
		}
		Sleep(10);
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in value  CVibraniumLiveUpdate::UpdateTimeDate", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertUpdaterData
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 InsertUpdaterData(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable in VibraniumAutoScnDlg- InsertUpdaterData entered", 0, 0, true, ZEROLEVEL);
	try
	{
		g_csWardWizModulePath = GetSQLiteDBFileSource();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", g_csWardWizModulePath);

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
		AddLogEntry(L"### Exception in value  CVibraniumLiveUpdate::InsertUpdaterData", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	UpdateUpdaterDBDetails
*  Description    :	Function to update Update module details into database for specific record.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 UpdateUpdaterDBDetails(int iFileCount, int iFileSize,INT64 iUpdateID)
{
	CString csInsertQuery = _T("UPDATE Wardwiz_UpdatesMaster VALUES (null,");
	try
	{
		g_csWardWizModulePath = GetSQLiteDBFileSource();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", g_csWardWizModulePath);

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		if (!PathFileExistsA(dbPath.m_psz))
		{
			CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
			objSqlDb.Open();
			objSqlDb.CreateWardwizSQLiteTables(theApp.m_dwProductID);
			objSqlDb.Close();
		}

		csInsertQuery.Format(_T("UPDATE Wardwiz_UpdatesMaster SET db_UpdateEndTime= datetime('now','localtime'), db_FilesDownloadCount= %d, db_DownloadFileSize = %d WHERE db_UpdateId = %d"), iFileCount, iFileSize, iUpdateID);

		CT2A ascii(csInsertQuery, CP_UTF8);
		INT64 iRowId = InsertUpdaterData(ascii.m_psz);
		return iRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value  CVibraniumLiveUpdate::UpdateUpdaterDBDetails. Query is : ", csInsertQuery, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	GetSQLiteDBFilePath
*  Description    :	Helper function to get Current working directory path
*  Author Name    : Gayatri A.
*  Date           : 12 Dec 2016
*  SR_NO		  :
**********************************************************************************************************/
CString GetSQLiteDBFileSource()
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
		AddLogEntry(L"### Exception in value CVibraniumUpdates::GetSQLiteDBFileSource", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************************************
*  Function Name  : StartDownLoadThread
*  Description    : This thread will show the status and percentage from ALU service to UI
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
*  Modified date  :	 19-May-2016 (Auto Live Update)
****************************************************************************************************/
bool CWardWizLiveUpdate::UpDateDowloadStatus(LPISPY_PIPE_DATA lpSpyData)
{
	DWORD dwFileSizeInKB = 0, dwMessageType;
	m_csTotalFileSize = L"";
	CString	csListControlStatus = L"";
	m_iRowCount = 0;
	CString l_csPercentage = L"";
	CITinRegWrapper objReg;
	DWORD dwProductUpdate = 0;
	CString csDisplaySize = _T("KB");
	int iSize = 0;
	int iTotalSize = 0;
	try
	{
		if (!lpSpyData)
			return false;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwAutoProductUpdate", dwProductUpdate) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwAutoDefUpdate in CVibraniumLiveUpdate::UpDateDowloadStatus", 0, 0, true, SECONDLEVEL);;
		}
		if (dwProductUpdate == 1) 
		{
			m_csUpdateType.Format(L"1");		//Product Update is ON in Settings
		}
		else
		{
			m_csUpdateType.Format(L"0");		//Product Update is OFF in Settings
		}
		m_svUpdateDownloadTypeCB.call((SCITER_STRING)m_csUpdateType);
		if (m_bIsCheckingForUpdateEntryExistInList)
		{
			InsertItem(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"), theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"));
		}
		switch (lpSpyData->iMessageInfo)
		{
		case SETTOTALFILESIZE:
			m_iTotalFileSize = lpSpyData->dwValue;
			dwFileSizeInKB = static_cast<DWORD>(m_iTotalFileSize / 1024);
			
			iSize = ConvertBytes2KB(m_iTotalFileSize);

			if (iSize > 1024)
			{
				m_csDispTotSizeExt = _T("MB");
				iSize = ConvertBytes2MB(m_iTotalFileSize);
			}
			if (iSize > 1024)
			{
				m_csDispTotSizeExt = _T("GB");
				iSize = ConvertBytes2GB(m_iTotalFileSize);
			}
			m_iTotalFileSizeCount = iSize;
			m_csTotalFileSize.Format(L"%d %s", iSize, m_csDispTotSizeExt);
			m_iIntFilesSize = (int)dwFileSizeInKB;
			m_svUpdateStatusFunctionCB.call((SCITER_STRING)m_csDownloadPercentage, (SCITER_STRING)m_csTotalFileSize, (SCITER_STRING)m_csCurrentDownloadedbytes);
			break;

		case SETDOWNLOADPERCENTAGE:
			m_iCurrentDownloadBytes = lpSpyData->dwValue;
			m_dwPercentage = lpSpyData->dwSecondValue;
			m_csDownloadPercentage.Format(L"%d %s", m_dwPercentage, L"%");

			iSize = ConvertBytes2KB(m_iCurrentDownloadBytes);

			if (iSize > 1024)
			{
				csDisplaySize = _T("MB");
				iSize = ConvertBytes2MB(m_iCurrentDownloadBytes);
			}
			if (iSize > 1024)
			{
				csDisplaySize = _T("GB");
				iSize = ConvertBytes2GB(m_iCurrentDownloadBytes);
			}

			m_csCurrentDownloadedbytes.Format(L"[%d %s]", static_cast<DWORD>(iSize), csDisplaySize);

			dwFileSizeInKB = static_cast<DWORD>(m_iTotalFileSize / 1024);
			m_csTotalFileSize.Format(L"%d %s", m_iTotalFileSizeCount, m_csDispTotSizeExt);
			//OutputDebugString(m_csDownloadPercentage);
			m_svUpdateStatusFunctionCB.call((SCITER_STRING)m_csDownloadPercentage, (SCITER_STRING)m_csTotalFileSize, (SCITER_STRING)m_csCurrentDownloadedbytes);
			break;

		case SETMESSAGE:
			m_csListControlStatus.Format(L"%s", lpSpyData->szFirstParam);
			if (m_csListControlStatus.CompareNoCase(L"Downloading files") == 0)
			{
				csListControlStatus.Format(L"%s: %d/%d", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DOWNLOADING"), lpSpyData->dwValue, lpSpyData->dwSecondValue);
				m_iFileCount = (int)lpSpyData->dwSecondValue;
			}
			else if (m_csListControlStatus.CompareNoCase(L"Updating files") == 0)
			{
				m_svLiveRowAddCb.call();
				csListControlStatus.Format(L"%s: %d/%d", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_UPDATING"), lpSpyData->dwValue, lpSpyData->dwSecondValue);
			}
			if (csListControlStatus.GetLength() > 0 && (g_csPreviousListControlStatus != csListControlStatus))
			{
				OutputDebugString(csListControlStatus);
				InsertItem(csListControlStatus, m_csListControlStatus);
			}
			g_csPreviousListControlStatus.SetString(csListControlStatus);
			break;

		case SETUPDATESTATUS:
			dwMessageType = lpSpyData->dwValue;
			switch (dwMessageType)
			{
			case ALUPDATEDSUCCESSFULLY:
				m_dwstatusMsg = ALUPDATEDSUCCESSFULLY;
				m_svLiveUpdateCompleteCb.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED"));
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_INTERNETCONNECTION:
				m_dwstatusMsg = ALUPDATEFAILED_INTERNETCONNECTION;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_DOWNLOADINIPARSE:
				m_dwstatusMsg = ALUPDATEFAILED_DOWNLOADINIPARSE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATED_UPTODATE:
				m_dwstatusMsg = ALUPDATED_UPTODATE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_DOWNLOADFILE:
				m_dwstatusMsg = ALUPDATEFAILED_DOWNLOADFILE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_EXTRACTFILE:
				m_dwstatusMsg = ALUPDATEFAILED_EXTRACTFILE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			case ALUPDATEFAILED_UPDATINGFILE:
				m_dwstatusMsg = ALUPDATEFAILED_UPDATINGFILE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				break;
			case ALUPDATEFAILED_LOWDISKSPACE:
				m_dwstatusMsg = ALUPDATEFAILED_LOWDISKSPACE;
				m_iFileCount = 0;
				m_iIntFilesSize = 0;
				UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
				m_iDBUpdateID = 0;
				break;
			}
			break;
		}
		csListControlStatus.ReleaseBuffer();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::UpDateDowloadStatus", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***********************************************************************************************
*  Function Name  : SendRequestCommon
*  Description    : Send request to auto live update services.

*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
bool CWardWizLiveUpdate::SendRequestCommon(int iRequest, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;
		szPipeData.dwValue = 1;
		CISpyCommunicator objCom(AUTOUPDATESRV_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to AUTOUPDATESRV_SERVER in CVibraniumLiveUpdate::SendRequestCommon", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData to AUTOUPDATESRV_SERVER in CVibraniumLiveUpdate::SendRequestCommon", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::SendRequestCommon", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  : PauseUpdate
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
*************************************************************************************************/
bool CWardWizLiveUpdate::PauseUpdateLiveUpdate()
{
	bool bReturn = false;
	try
	{
		if (!SendRequestCommon(PAUSE_UPDATE))
		{
			AddLogEntry(L"### Exception in CVibraniumLiveUpdate::PauseUpdateLiveUpdate", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::PauseUpdateLiveUpdate", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ResumeScan
*  Description    : Resume scanning if user click on stop/close button and click to 'No' for stop confirmation box.
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           :  19-May-2016
*************************************************************************************************/
bool CWardWizLiveUpdate::ResumeUpdateLiveUpdate()
{
	bool bReturn = false;
	try
	{
		if (!SendRequestCommon(RESUME_UPDATE))
		{
			AddLogEntry(L"### Exception in CVibraniumLiveUpdate::ResumeUpdateLiveUpdate", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::ResumeDownload", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ShutDownDownload
*  Description    : It shows message whether downloading should stop or not.
*  Author Name    :	Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
****************************************************************************************************/
bool CWardWizLiveUpdate::ShutDownDownloadLiveupdates()
{
	try
	{
		if (!m_isDownloading)
		{
			return true;
		}
		AddLogEntry(L">>> CLiveupdateDlg Stopping downloading", 0, 0, true, FIRSTLEVEL);
		if (!SendRequestCommon(STOP_UPDATE, true))
		{
			AddLogEntry(L"### Faild to stop live update:: CWardwizLiveUpSecondDlg::ShutDownDownloadLiveupdates", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::ShutDownDownload", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
*  Function Name  : InsertItem
*  Description    : Insert item In tables , An new parameter added m_idivsion , to keep track of
					rows
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
*************************************************************************************************/
void CWardWizLiveUpdate::InsertItem(CString csInsertItem, CString csActualStatus)
{
	try
	{
		if (csActualStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"))
		{
			if (m_bIsCheckingForUpdateEntryExistInList)
			{
				OnAddUpdateStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"));
				m_bIsCheckingForUpdateEntryExistInList = false;
			}
		}
		else if (csActualStatus == L"Already downloaded")
		{
			m_bISalreadyDownloadAvailable = true;
			OnAddUpdateStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_ALREADY_DOWNLOADED"));
		}
		else if (csActualStatus == L"Downloading files")
		{
			m_bISalreadyDownloadAvailable = false;
			OnAddUpdateStatus(csInsertItem);
		}
		else if (csActualStatus == L"Updating files")
		{
			OnAddUpdateStatus(csInsertItem);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::InsertItem", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : OnAddUpdateStatus
*  Description    : On adding the status into list control
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-May-2016
*************************************************************************************************/
void CWardWizLiveUpdate::OnAddUpdateStatus(CString csStatus )
{
	try
	{
		if (m_iCount > 0)
		{
			m_svLiveUpdateUpdateTableCb.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED"), (SCITER_STRING)csStatus, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"));
			m_iRowCount++;
			m_iCount++;
		}
		else
		{
			m_svLiveAddUpdateTableCb.call((SCITER_STRING)csStatus, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"));
			m_iRowCount++;
			m_iCount++;
		}
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::OnAddUpdateStatus", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  : ResumeUpdate
*  Description    : Pause scanning if user click on stop/close button.
*  Author Name    :Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardWizLiveUpdate::StopLiveUpdates()
{
	try
	{
		ShutDownDownloadLiveupdates();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::StopLiveUpdates", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name	 : ShowUpdateCompleteMessage
*  Description		 : It shows the message after completing a download
*  Author Name	     : Nihar Deshpande
*  SR_NO		     :
*  Date				 : 19-05-2016
*  Modification Date : 25-Jul-2014
*  Modified Date	 : Neha Gharge 5/5/2015 for new status message for failures and succesful cases.
****************************************************************************************************/
void CWardWizLiveUpdate::ShowUpdateCompleteMessage()
{
	bool bShowRedirectUI = false;
	DWORD dwCount = 0;
	TCHAR szIniFilePath[512] = { 0 };
	try
	{
		if (!m_bIsStopFrmTaskbar)
		{
			if (m_bIsManualStop)
			{
				CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPDATE_ABORTED"));
			}
			else
			{
				if (m_dwstatusMsg == ALUPDATEDSUCCESSFULLY)
				{
					//bool bmessgeBoxResponce = false;


					swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", m_szAllUserPath);

					if (!PathFileExists(szIniFilePath))
					{
						AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
						dwCount = 0;
					}
					else
					{
						dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
					}

					if (dwCount <= 0)
					{
						swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", m_szAllUserPath);

						if (!PathFileExists(szIniFilePath))
						{
							AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
							dwCount = 0;
						}
						else
						{
							dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
						}
					}


					if (!m_bISalreadyDownloadAvailable || dwCount > 0)
					{
						bShowRedirectUI = true;
						if (dwCount > 0 )
							CallNotificationMessage(5, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPDATED_SUCCESS")); //for restart
						else
							CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPDATED_SUCCESS"));
						/*if(theApp.m_bRetval == true)
						{
							bmessgeBoxResponce = true;
						}*/
					}
					else
					{
						dwCount = 0;
						CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPTODATE"));
					}

					//if (bmessgeBoxResponce)
					//{
						/*swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", m_szAllUserPath);

						if (!PathFileExists(szIniFilePath))
						{
							AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
							dwCount = 0;
						}
						else
						{
							dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
						}
						if (dwCount <= 0)
						{
							swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", m_szAllUserPath);

							if (!PathFileExists(szIniFilePath))
							{
								AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
								dwCount = 0;
							}
							else
							{
								dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
							}
						}*/
						if (dwCount > 0)
						{
							/*CallNotificationMessage(2, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_FOR_UPDATE"));
							if (theApp.m_bRetval == true)
							{
								CEnumProcess enumproc;
								enumproc.RebootSystem(0);
							}
							else
							{			
								UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
							}*/
							UpdateUpdaterDBDetails(m_iFileCount, m_iIntFilesSize, m_iDBUpdateID);
						}
					//}
				}
				else if (m_dwstatusMsg == ALUPDATED_UPTODATE)
				{
					CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPTODATE"));
				}
				else if (m_dwstatusMsg == ALUPDATEFAILED_INTERNETCONNECTION)
				{
					CallNotificationMessage(3, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_MSG_1"));
				}
				else if (m_dwstatusMsg == ALUPDATEFAILED_DOWNLOADINIPARSE || m_dwstatusMsg == ALUPDATEFAILED_DOWNLOADFILE || m_dwstatusMsg == ALUPDATEFAILED_EXTRACTFILE || m_dwstatusMsg == ALUPDATEFAILED_UPDATINGFILE)
				{
					CString csText = L"";
					csText.Format(L"%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_MSG_1"), theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_CONTACT_SUPPORT"));
					CallNotificationMessage(3, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_MSG_1"));
				}
				else if (m_dwstatusMsg == ALUPDATEFAILED_LOWDISKSPACE)
				{
					CallNotificationMessage(1, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPTODATE"));
				}
			}
		}
		m_isDownloading = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::ShowUpdateCompleteMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : CallNotificationMessage
Description    : Calls Light box on UI
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Sept 2016
/***************************************************************************************************/
void CWardWizLiveUpdate::CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString)
{
	try
	{
		m_csCurrentDownloadedbytes = L"0";
		theApp.m_objCompleteEvent.ResetEvent();
		m_svFunNotificationMessageCB.call(iMsgType, (SCITER_STRING)strMessageString);
		::WaitForSingleObject(theApp.m_objCompleteEvent, INFINITE);
		Sleep(700); 
		theApp.m_objCompleteEvent.ResetEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumLiveUpdate::CallNotificationMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function Name  : ConvertBytes2KB
Description    : converting bytes to KB
Author Name    : Amol J.
SR_NO		   :
Date           : 14th Dec 2017
/***************************************************************************************************/
int CWardWizLiveUpdate::ConvertBytes2KB(int iCurrentDownloadBytes)
{
	try
	{
		return (iCurrentDownloadBytes / 1024);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ConvertBytes2KB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : ConvertBytes2MB
Description    : converting KB to MB
Author Name    : Amol J.
SR_NO		   :
Date           : 14th Dec 2017
/***************************************************************************************************/
int CWardWizLiveUpdate::ConvertBytes2MB(int iCurrentDownloadKB)
{
	try
	{
		return (iCurrentDownloadKB / 1048576);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ConvertBytes2MB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : ConvertBytes2GB
Description    : converting MB to GB
Author Name    : Amol J.
SR_NO		   :
Date           : 14th Dec 2017
/***************************************************************************************************/
int CWardWizLiveUpdate::ConvertBytes2GB(int iCurrentDownloadMB)
{
	try
	{
		return (iCurrentDownloadMB / 1073741824);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ConvertBytes2GB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}