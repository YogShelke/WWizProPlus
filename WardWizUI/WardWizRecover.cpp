// CWardWizRecover.cpp : implementation file
/*********************************************************************
*  Program Name: CWardWizRecover.cpp
*  Description: While cleaning any infected file,WardWiz AV taking a
backup in backup folder but in encrypted format.So if
user wants to retrive the files then user can press recover
button to retrive file to its original location.
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 6th May 2016
*  Version No: 2.0.0.1

**********************************************************************/

#include "stdafx.h"
#include "WardWizRecover.h"
#include "WardWizDatabaseInterface.h"

DWORD WINAPI RecoverThread(LPVOID lpParam);
DWORD WINAPI DeleteRThread(LPVOID lpParam);
DWORD WINAPI ShowRecoverEntriesThread(LPVOID lpvThreadParam);
#define UPDATE_RECOVER_TABLE_EVENT (FIRST_APPLICATION_EVENT_CODE + 1)
#define ADD_RECOVER_TABLE_EVENT (FIRST_APPLICATION_EVENT_CODE + 2)
#define	SETRECOVERFILEFINISHED_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 26)


CWardWizRecover::CWardWizRecover() : behavior_factory("WardWizRecover")
, m_hThread_Recover(NULL)
, m_hThread_Delete(NULL)
, m_bRecoverStop(false)
, m_bRecoverBrowsepath(false)
, m_bRecoverThreadStart(false)
, m_bDeleteThreadStart(false)
, m_hShowRecoverEntriesThread(NULL)
, m_bShowRecoverEntries(false)
, m_objCom(SERVICE_SERVER, true)
, m_bExclude(false)
, m_bIsMultiRecoverFileFinish(false)
{
}


CWardWizRecover::~CWardWizRecover()
{
	if (m_hShowRecoverEntriesThread != NULL)
	{
		::SuspendThread(m_hShowRecoverEntriesThread);
		::TerminateThread(m_hShowRecoverEntriesThread, 0);
		m_hShowRecoverEntriesThread = NULL;
	}
}

/***********************************************************************************************
Function Name  : On_LoadRecoverEntries
Description    : Load entries from Recover.DB
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 6th May 2016
***********************************************************************************************/
json::value CWardWizRecover::On_LoadRecoverEntries(SCITER_VALUE svFunShowRecoverEntriesCB, SCITER_VALUE svLoadEntriesFinishedCB)
{
	try
	{
		m_svShowRecoverEntriesCB = svFunShowRecoverEntriesCB;
		m_svLoadEntriesFinishedCB = svLoadEntriesFinishedCB;
		OnBnClickedRecover();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::On_LoadRecoverEntries", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : On_ButtonRecoverEntries
Description    : Recovers entries from Recover.DB
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 6th May 2016
***********************************************************************************************/
json::value CWardWizRecover::On_ButtonRecoverEntries(SCITER_VALUE svArrRecoverEntries, SCITER_VALUE svFunUpdateRecoverEntryTableCB, SCITER_VALUE svFunOprFinishedStatusCB, SCITER_VALUE svbExclude)
{
	try
	{
		OnBnClickedRecover(svArrRecoverEntries, svFunUpdateRecoverEntryTableCB, svFunOprFinishedStatusCB, svbExclude);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::On_ButtonRecoverEntries", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : On_ButtonDeleteRecoverEntries
Description    : Deletes entries from Recover.DB
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 6th May 2016
***********************************************************************************************/
json::value CWardWizRecover::On_ButtonDeleteRecoverEntries(SCITER_VALUE svArrRecoverEntries, SCITER_VALUE svFunUpdateRecoverEntryTableCB, SCITER_VALUE svFunOprFinishedStatusCB)
{
	try
	{
		OnBnClickedDeleteRecover(svArrRecoverEntries, svFunUpdateRecoverEntryTableCB, svFunOprFinishedStatusCB);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::On_ButtonDeleteRecoverEntries", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/**********************************************************************************************************
*  Function Name  :	PauseRecoverOperation
*  Description    :	Terminates the thread to stop recover operation if user selects close button on UI
*  SR.NO		  :
*  Author Name    : Nitin K
*  Date           : 30 Jun 2016
**********************************************************************************************************/
json::value  CWardWizRecover::On_ButtonPauseRecoverOperation()
{
	try
	{
		if (m_hShowRecoverEntriesThread != NULL)
		{
			if (::SuspendThread(m_hShowRecoverEntriesThread) == -1)
			{
				AddLogEntry(L"### Failed to pause Recover operation.", 0, 0, true, SECONDLEVEL);
			}
			Sleep(500);
			AddLogEntry(L">>> Recover operation Paused.", 0, 0, true, ZEROLEVEL);
			return 0;
		}
		if (m_bRecoverThreadStart == false && m_bDeleteThreadStart == false)
		{
			return json::value(0);
		}

		int iRet = IDNO;
		if (m_bRecoverThreadStart == true)
		{
			if (m_hThread_Recover != NULL)
			{
				m_bRecoverStop = true;
				if (::SuspendThread(m_hThread_Recover) == -1)
				{
					AddLogEntry(L"### Failed to pause Recover operation.", 0, 0, true, SECONDLEVEL);
				}
				Sleep(500);
				AddLogEntry(L">>> Recover operation Paused.", 0, 0, true, ZEROLEVEL);
			}
		}
		else if (m_bDeleteThreadStart == true)
		{
			if (m_hThread_Delete != NULL)
			{
				m_bDeleteStop = true;
				if (::SuspendThread(m_hThread_Delete) == -1)
				{
					AddLogEntry(L"### Failed to pause Delete operation.", 0, 0, true, SECONDLEVEL);
				}
				Sleep(500);
				AddLogEntry(L">>> Delete operation Paused.", 0, 0, true, ZEROLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::PauseRecoverOperation", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/**********************************************************************************************************
*  Function Name  :	ResumeRecoverOperation
*  Description    :	Terminates the thread to stop recover operation if user selects close button on UI
*  SR.NO		  :
*  Author Name    : Nitin K
*  Date           : 30 Jun 2016
**********************************************************************************************************/
json::value CWardWizRecover::On_ButtonResumeRecoverOperation()
{
	try
	{
		if (m_hShowRecoverEntriesThread != NULL)
		{
			if (::ResumeThread(m_hShowRecoverEntriesThread) == -1)
			{
				AddLogEntry(L"### Failed to Resume Recover operation.", 0, 0, true, SECONDLEVEL);
			}
			Sleep(500);
			AddLogEntry(L">>> Recover operation Resumed.", 0, 0, true, ZEROLEVEL);
			return json::value(0);
		}
		if (m_bRecoverThreadStart == true)
		{
			if (m_hThread_Recover != NULL)
			{
				m_bRecoverStop = false;
				if (::ResumeThread(m_hThread_Recover) == -1)
				{
					AddLogEntry(L"### Failed to resume Recover operation.", 0, 0, true, SECONDLEVEL);
				}
				Sleep(500);
				AddLogEntry(L">>> Recover operation Resumed.", 0, 0, true, ZEROLEVEL);
			}
		}
		else if (m_bDeleteThreadStart == true)
		{
			if (m_hThread_Delete != NULL)
			{
				m_bDeleteStop = false;
				if (::ResumeThread(m_hThread_Delete) == -1)
				{
					AddLogEntry(L"### Failed to resume Delete operation.", 0, 0, true, SECONDLEVEL);
				}
				Sleep(500);
				AddLogEntry(L">>> Delete operation Resumed.", 0, 0, true, ZEROLEVEL);
			}
		}
		return json::value(0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::ResumeRecoverOperation", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/**********************************************************************************************************
*  Function Name  :	StopRecoverOperation
*  Description    :	Terminates the thread to stop recover operation if user selects close button on UI
*  SR.NO		  :
*  Author Name    : Nitin K
*  Date           : 30 Jun 2016
**********************************************************************************************************/
json::value CWardWizRecover::On_ButtonStopRecoverOperation()
{
	try
	{
		if (m_bShowRecoverEntries == true)
		{
			if (m_hShowRecoverEntriesThread != NULL)
			{
				if (::TerminateThread(m_hShowRecoverEntriesThread, 0) == -1)
				{
					AddLogEntry(L"### Failed to terminate Show Entries operation.", 0, 0, true, SECONDLEVEL);
				}
				AddLogEntry(L">>>Show Recover Entries operation terminated.", 0, 0, true, ZEROLEVEL);
			}
			m_bRecoverThreadStart = false;
			m_bDeleteThreadStart = false;
			m_bShowRecoverEntries = false;
			m_hShowRecoverEntriesThread = NULL;
		}
		else if (m_bRecoverThreadStart == true)
		{
			if (m_hThread_Recover != NULL)
			{
				m_bRecoverStop = true;
				if (::TerminateThread(m_hThread_Recover, 0) == -1)
				{
					AddLogEntry(L"### Failed to terminate Recover operation.", 0, 0, true, SECONDLEVEL);
				}
				AddLogEntry(L">>> Recover operation terminated.", 0, 0, true, ZEROLEVEL);
			}
			m_bRecoverThreadStart = false;
			m_bDeleteThreadStart = false;
			m_hThread_Recover = NULL;
		}
		else if (m_bDeleteThreadStart == true)
		{
			if (m_hThread_Delete != NULL)
			{
				m_bDeleteStop = true;
				if (::TerminateThread(m_hThread_Delete, 0) == -1)
				{
					AddLogEntry(L"### Failed to terminate Recover(Delete) operation.", 0, 0, true, SECONDLEVEL);
				}
				AddLogEntry(L">>> Recover(Delete) operation terminated.", 0, 0, true, ZEROLEVEL);
				m_bRecoverThreadStart = false;
				m_bDeleteThreadStart = false;
				m_hThread_Delete = NULL;
			}
		}

		if (!SendRecoverOperations2Service(SAVE_RECOVERDB, L"", L"", 0, 0, true))
		{
			AddLogEntry(L"### Failed to send SAVE_RECOVERDB in CVibraniumRecoverDlg::StopReportOperation", 0, 0, true, SECONDLEVEL);
		}
		Sleep(5);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::StopRecoverOperation", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  :           OnBnClickedRecover
*  Description    :           This function is called when user clicks on recover button from tab control
*  Author Name    :           Neha Gharge
*  SR_NO		  :
*  Date           :           10 Jun 2014
****************************************************************************************************/
void CWardWizRecover::OnBnClickedRecover()
{
	try
	{
		if (m_hShowRecoverEntriesThread != NULL)
		{
			return;
		}
		m_hShowRecoverEntriesThread = ::CreateThread(NULL, 0, ShowRecoverEntriesThread, (LPVOID) this, 0, NULL);
		Sleep(10);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::OnBnClickedRecover", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :           ShowRecoverEntriesThread
*  Description    :           Thread gets created for purpose that when user clicks on recover button
from tab controls at that time it loads recover entries from Db files.
*  Author Name    :           Neha Gharge
*  SR_NO		  :
*  Date           :           20 Nov 2013
****************************************************************************************************/
DWORD WINAPI ShowRecoverEntriesThread(LPVOID lpvThreadParam)
{
	try
	{
		CWardWizRecover *pThis = (CWardWizRecover*)lpvThreadParam;
		if (pThis)
		{
			pThis->m_bShowRecoverEntries = true;
			pThis->m_objRecoverdbToSave.RemoveAll();
			pThis->LoadExistingRecoverFile(true);
			pThis->PopulateList(true);
			pThis->LoadRemainingEntries();
			pThis->m_hShowRecoverEntriesThread = NULL;
			pThis->m_bShowRecoverEntries = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumAntiRootkit::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	LoadExistingRecoverFile
*  Description    :	Opens VBRECOVERENTRIES.DB file & retrieves its contents
*  SR.NO		  :
*  Author Name    : Neha Gharge
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CWardWizRecover::LoadExistingRecoverFile(bool bType)
{
	static CString csFilePath = L"";
	csFilePath = GetQuarantineFolderPath();
	csFilePath += _T("\\VBRECOVERENTRIES.DB");
	LoadDataContentFromFile(csFilePath);
}

/**********************************************************************************************************
*  Function Name  :	PopulateList
*  Description    :	Displays entries on recover dialog
*  SR.NO		  :
*  Author Name    : Neha Gharge
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CWardWizRecover::PopulateList(bool bCheckEntry)
{
	int nCurrentItem;
	const ContactList& contacts = m_objRecoverdb.GetContacts();
	POSITION pos = contacts.GetHeadPosition();
	nCurrentItem = 0;//m_lstRecover.GetItemCount();
	while (pos != NULL)
	{
		const CIspyList contact = contacts.GetNext(pos);
		if (!m_objRecoverdbToSave.AddContact(contact))
		{
			continue;//Continue to next entry.
		}
		//Check here if Repair completed sucessfully or not
		//	0 - Repair/Delete success
		//	1 - Delete failed.
		//	2 - Repair failed.
		if (contact.GetSeventhEntry() > 0)
		{
			continue;//Continue to next entry.
		}

		sciter::value map;
		map.set_item("one", (SCITER_STRING)contact.GetThirdEntry());
		map.set_item("two", (SCITER_STRING)contact.GetFirstEntry());
		map.set_item("three", L"");
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = ADD_RECOVER_TABLE_EVENT;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
		nCurrentItem++; //this line is for append entry;
	}
	m_svLoadEntriesFinishedCB.call();
}

/**********************************************************************************************************
*  Function Name  :	LoadRemainingEntries
*  Description    :	Function to get remaining entries of recover.
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
void CWardWizRecover::LoadRemainingEntries()
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

		//Load the file contents in memory and populate in list control
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{
			CString csFilePath;
			csFilePath.Format(L"%s\\VBRECOVERENTRIES_%d.DB", GetQuarantineFolderPath(), *it);
			if (PathFileExists(csFilePath))
			{
				LoadDataContentFromFile(csFilePath);
				PopulateList(true);
			}
		}

		//Save the DB file here
		SaveDBFile();
		//Remove the files which are already loaded and merged.
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{
			CString csFilePath;
			csFilePath.Format(L"%s\\VBRECOVERENTRIES_%d.DB", GetQuarantineFolderPath(), *it);
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
		AddLogEntry(L"### Exception in CVibraniumRecoverDlg::LoadRemainingEntries", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetSortedFileNames
*  Description    :	Function to get file ID's in sorted order.
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizRecover::GetSortedFileNames(vector<int> &vec)
{
	try
	{
		CString csQurFolderPath = GetQuarantineFolderPath();
		GetFileDigits(csQurFolderPath, vec);
		sort(vec.begin(), vec.end());

		if (vec.size() > 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecoverDlg::GetSortedFileNames", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :           GetQuarantineFolderPath
*  Description    :           Get quarantine folder(backup folder) path.
*  Author Name    :           Neha Gharge
*  SR_NO		  :
*  Date           :           10 Jun 2014
****************************************************************************************************/
CString CWardWizRecover::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/**********************************************************************************************************
*  Function Name  :	LoadDataContentFromFile
*  Description    :	Loads VBRECOVERENTRIES.DB file content into serialization object.
*  SR.NO		  :
*  Author Name    : Neha Gharge
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CWardWizRecover::LoadDataContentFromFile(CString csPathName)
{
	AddLogEntry(L">>> Loading recover db", 0, 0, true, FIRSTLEVEL);
	m_objRecoverdb.RemoveAll();

	if (!PathFileExists(csPathName))
		return false;

	CFile rFile(csPathName, CFile::modeRead);
	// Create a loading archive
	CArchive arLoad(&rFile, CArchive::load);
	m_objRecoverdb.Serialize(arLoad);
	// Close the loading archive
	arLoad.Close();
	rFile.Close();
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SaveDBFile
*  Description    :	According to type of module Save DB .
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0003
**********************************************************************************************************/
bool CWardWizRecover::SaveDBFile()
{
	try
	{
		CString csFilePath = GetQuarantineFolderPath();
		csFilePath += _T("\\VBRECOVERENTRIES.DB");

		int iFileVersion = 1;
		CFile wFile(csFilePath, CFile::modeCreate | CFile::modeWrite);

		CArchive arStore(&wFile, CArchive::store);

		m_objRecoverdbToSave.SetFileVersion(iFileVersion);
		m_objRecoverdbToSave.Serialize(arStore);

		arStore.Close();
		wFile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::SaveDBFile", 0, 0, true, SECONDLEVEL);
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
void CWardWizRecover::GetFileDigits(LPCTSTR pstr, vector<int> &vec)
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
		AddLogEntry(L"### Exception in CVibraniumRecover::GetFileDigits", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	OnBnClickedButtonRecover
*  Description    :	Recovers selected entry from reports
*  SR.NO          :
*  Author Name    : Neha Gharge
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CWardWizRecover::OnBnClickedRecover(SCITER_VALUE svArrRecoverEntries, SCITER_VALUE UpdateRecoverEntryTableCB, SCITER_VALUE svFunOprFinishedStatusCB, SCITER_VALUE svbExclude)
{
	try
	{
		bool bIsArray = false;
		theApp.m_bIsRecoverFileUIReceptive = true;
		m_bIsMultiRecoverFileFinish = false;
		svArrRecoverEntries.isolate();
		bIsArray = svArrRecoverEntries.is_array();
		if (!bIsArray)
		{
			return;
		}
		m_svUpdateRecoverTableCB = UpdateRecoverEntryTableCB;
		m_svArrRecoverEntries = svArrRecoverEntries;
		m_svFunOprFinishedStatusCB = svFunOprFinishedStatusCB;
		m_bExclude = svbExclude.get(false);
		AddLogEntry(L">>> Recover Started", 0, 0, true, FIRSTLEVEL);
		m_hThread_Recover = ::CreateThread(NULL, 0, RecoverThread, (LPVOID) this, 0, NULL);
		Sleep(1000);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::OnBnClickedRecover", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :           OnBnClickedDeleteRecover
*  Description    :           this function is called for deletion of records
*  Author Name    :           Nitin K. Kolapkar
*  SR_NO		  :
*  Date           :           6th May 2016
****************************************************************************************************/
void CWardWizRecover::OnBnClickedDeleteRecover(SCITER_VALUE svArrRecoverEntries, SCITER_VALUE UpdateRecoverEntryTableCB, SCITER_VALUE svFunOprFinishedStatusCB)
{
	try
	{
		bool bIsArray = false;
		theApp.m_bIsRecoverFileUIReceptive = true;
		m_bIsMultiRecoverFileFinish = false;
		svArrRecoverEntries.isolate();
		bIsArray = svArrRecoverEntries.is_array();
		if (!bIsArray)
		{
			return;
		}
		CString csVirusName, csDuplicatePath;
		m_svUpdateRecoverTableCB = UpdateRecoverEntryTableCB;
		m_svArrRecoverEntries = svArrRecoverEntries;
		m_svFunOprFinishedStatusCB = svFunOprFinishedStatusCB;
		if (!theApp.ShowRestartMsgOnProductUpdate())
		{
			return;
		}
		AddLogEntry(L">>> Delete Started", 0, 0, true, FIRSTLEVEL);

		m_hThread_Delete = ::CreateThread(NULL, 0, DeleteRThread, (LPVOID) this, 0, NULL);
		Sleep(1000);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::OnBnClickedDeleteRecover", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :           RecoverThread
*  Description    :           this function is called for recovering of records
*  Author Name    :			  Neha Gharge.
*  SR_NO		  :
*  Date           :           11th June 2014
****************************************************************************************************/
DWORD WINAPI RecoverThread(LPVOID lpParam)
{
	try
	{
		if (!lpParam)
		{
			return 0;
		}
		CWardWizRecover *pThis = (CWardWizRecover *)lpParam;
		pThis->m_bRecoverStop = false;
		pThis->RecoverEntries();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::RecoverThread", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***************************************************************************************************
*  Function Name  :           On_OnGetPathForExclude
*  Description    :           sending entries path for recover and exclude.
*  Author Name    :           Amol J
*  SR_NO		  :
*  Date           :           23 May 2017
****************************************************************************************************/
json::value CWardWizRecover::On_OnGetPathForExclude(SCITER_VALUE FunSendFilePath)
{
	m_svFunSendFilePathCB = FunSendFilePath;
	return 0;
}

/***************************************************************************************************
*  Function Name  :           RecoverEntries
*  Description    :           this function is called for recovering of files
*  Author Name    :           Neha Gharge
*  SR_NO		  :
*  Date           :           20 Nov 2013
****************************************************************************************************/
void CWardWizRecover::RecoverEntries()
{
	CString csFilePath = L"";
	CString csThreatPath = L"", csBrowsePath = L"";
	int iPos = 0;
	int iCurrentValue, count = 0;
	m_bRecoverThreadStart = true;
	DWORD dwRecoverCount = 0;
	bool	isAnyEntrySeletected = false;
	CString csMsgBox(L"");
	try
	{
		AddLogEntry(L">>> recovering entry started", 0, 0, true, FIRSTLEVEL);
		count = m_svArrRecoverEntries.length();
		do
		{
		for (iCurrentValue = count; iCurrentValue >= 0; iCurrentValue--)
		{
			if (m_bRecoverStop)
			{
				break;
			}
			const SCITER_VALUE EachEntryinArray = m_svArrRecoverEntries[iCurrentValue];
			bool bValue = EachEntryinArray[L"selected"].get(false);
			if (bValue)
			{
				const std::wstring chThreatPath = EachEntryinArray[L"FilePath"].get(L"");
				csThreatPath = chThreatPath.c_str();
				csFilePath = csThreatPath;
				iPos = csFilePath.Find(_T("\\"));
				csFilePath.Truncate(iPos);
				if (!PathFileExists(csFilePath) && m_bRecoverBrowsepath == false)
				{
					CString csMsgPathNotExist;
					csMsgPathNotExist.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_BROWSE_MSG"));
					BrowseFileToSaveRecover(csFilePath);
					csBrowsePath = csFilePath;
					m_bRecoverBrowsepath = true;
				}
				Sleep(100);

				//Exclude Recover file path logic
				if (m_bExclude == true)
				{
					CString	csWardWizModulePath = GetWardWizPathFromRegistry();
					CString	csWardWizReportsPath = L"";
					csWardWizReportsPath.Format(L"%s\\VBEXCLUDE.DB", csWardWizModulePath);
					CT2A dbPath(csWardWizReportsPath, CP_UTF8);

					//if table db is not created, it will create.
					if (!PathFileExists(csWardWizReportsPath))
					{
						CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
						objSqlDb.Open();
						if (!objSqlDb.TableExists("WWIZEXCLUDELST"))
						{
							objSqlDb.ExecDML("CREATE TABLE[WWIZEXCLUDELST](\
											 								[ID] INTEGER  NOT NULL PRIMARY KEY,\
																											[NAME] NVARCHAR(1000)  NOT NULL,\
																																			[ISUBFOL] INT NOT NULL\
																																											)");
						}
						objSqlDb.Close();
					}
					else
					{
						CStringA csQuery;
						if (PathIsDirectory(csThreatPath))
							csQuery.Format("insert into WWIZEXCLUDELST(NAME, ISUBFOL) values('%s' , %d)", CStringA(csThreatPath), 1);
						else
							csQuery.Format("insert into WWIZEXCLUDELST(NAME, ISUBFOL) values('%s' , %d)", CStringA(csThreatPath), 0);

						CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
						objSqlDb.Open();
						objSqlDb.ExecDML(csQuery.GetBuffer());
						objSqlDb.Close();
					}

					/*
					//insert data logic
					if (PathIsDirectory(csThreatPath))
						m_svFunSendFilePathCB.call((SCITER_STRING)csThreatPath, 1); //1 for folder
					else
						m_svFunSendFilePathCB.call((SCITER_STRING)csThreatPath, 0); //0 for file
					*/

				}
				ISPY_PIPE_DATA	szPipeData = { 0 };

				szPipeData.iMessageInfo = RECOVER_FILE;
				_tcscpy_s(szPipeData.szFirstParam, csThreatPath);
				_tcscpy_s(szPipeData.szSecondParam, csBrowsePath);
				szPipeData.dwValue = 0;
				szPipeData.dwSecondValue = 0;
				isAnyEntrySeletected = true;

				SendRecoverOperations2Service(&szPipeData, true);
				CString csCurrnetFile = L"";
				csCurrnetFile.Format(L"%d", iCurrentValue);
				if (!szPipeData.dwValue)
				{
					UpdateRecoverTable(1, iCurrentValue, L"Recovered");
					dwRecoverCount++;
				}
				else
				{
					bool bRecoverOprFailed = false;
					switch (szPipeData.dwValue)
					{
						case 0x02:
							UpdateRecoverTable(2, iCurrentValue, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_REPORTS_FAILED_FILE_IN_USE"));
							bRecoverOprFailed = true;
							break;

						default:
							UpdateRecoverTable(2, iCurrentValue, (SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"));
							bRecoverOprFailed = true;
							break;
					}
					if (bRecoverOprFailed)
						m_svFunSendFilePathCB.call((SCITER_STRING)csThreatPath, 3); //3 for failed

					AddLogEntry(L"### Error in CRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				}
			}
			count--;
		}
		if (iCurrentValue <= count)
			break;

		} while (isAnyEntrySeletected);		
		
		if (!isAnyEntrySeletected)
		{
			m_csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_NO_SELECT_ENTRY"));
			m_svFunOprFinishedStatusCB.call((SCITER_STRING)m_csMsgBox);
			m_bIsMultiRecoverFileFinish = true;
			if (theApp.m_bIsRecoverFileUIReceptive)
			{
				if (theApp.m_bIsRecoverFilePageSwitched)
				{
					OnCallSetFinishStatus();
				}
			}
			goto Cleanup;
		}			
		if (!m_bRecoverStop)
		{
			TCHAR szRecoverMsg[MAX_PATH] = { 0 };
			if (dwRecoverCount == 0)
			{
				m_csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_NO_FILE_RECOVERED"));
			}
			else
			{
				m_csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_FILES_RECOVERED_SUCCESSFULLY"));
			}
		}
		m_svFunOprFinishedStatusCB.call((SCITER_STRING)m_csMsgBox);

		m_bIsMultiRecoverFileFinish = true;
		if (theApp.m_bIsRecoverFileUIReceptive)
		{
			if (theApp.m_bIsRecoverFilePageSwitched)
			{
				OnCallSetFinishStatus();
			}
		}
		AddLogEntry(L">>> recovering entry finished", 0, 0, true, FIRSTLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::RecoverEntries", 0, 0, true, SECONDLEVEL);
	}
Cleanup:
	dwRecoverCount = 0;
	m_bRecoverThreadStart = false;
	m_bRecoverBrowsepath = false;
}

/**********************************************************************************************************
*  Function Name  :	SendRecoverOperations2Service
*  Description    :	Sends data to service to recover and delete entries which are selected and save remaining
entries which are not selected.
*  SR.NO		  :
*  Author Name    : Neha Gharge
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CWardWizRecover::SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry, CString csBrowseRecoverFileEntry, DWORD dwType, DWORD dwTypeofAction, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = dwMessageinfo;
		_tcscpy_s(szPipeData.szFirstParam, csRecoverFileEntry);
		_tcscpy_s(szPipeData.szSecondParam, csBrowseRecoverFileEntry);
		szPipeData.dwValue = dwType;
		szPipeData.dwSecondValue = dwTypeofAction;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CVibraniumRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CVibraniumRecover::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SendRecoverOperations2Service
*  Description    :	Sends data to service to recover and delete entries which are selected and save remaining entries which are not selected.
*  SR.NO		  :
*  Author Name    : Vilas Suvarnakar
*  Date           : 27 Mar 2013
**********************************************************************************************************/
bool CWardWizRecover::SendRecoverOperations2Service(ISPY_PIPE_DATA *pszPipeData, bool bWait)
{
	try
	{
		if (!m_objCom.SendData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CVibraniumRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!m_objCom.ReadData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in CVibraniumRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
/***************************************************************************************************
*  Function Name  :           BrowseFileToSaveRecover
*  Description    :           If original recover path drive is not exist,user browse folder where
user want to save recovered file.
*  Author Name    :           Neha Gharge
*  SR_NO		  :
*  Date           :           10 Jun 2014
****************************************************************************************************/
bool CWardWizRecover::BrowseFileToSaveRecover(CString &csBroswseFilepath)
{
	try
	{
		TCHAR *pszPath = new TCHAR[MAX_PATH];
		SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));
		CString csMessage = L"Please select folder for recover files.";
		BROWSEINFO        bi = { theApp.m_pMainWnd->GetSafeHwnd(), NULL, pszPath, csMessage, BIF_RETURNONLYFSDIRS, NULL, 0L, 0 };
		LPITEMIDLIST      pIdl = NULL; 
		LPITEMIDLIST  pRoot = NULL;
		SHGetFolderLocation(theApp.m_pMainWnd->GetSafeHwnd(), CSIDL_DRIVES, 0, 0, &pRoot);
		bi.pidlRoot = pRoot;
		pIdl = SHBrowseForFolder(&bi);
		if (NULL != pIdl)
		{
			SHGetPathFromIDList(pIdl, pszPath);
			size_t iLen = wcslen(pszPath);
			if (iLen > 0)
			{
				csBroswseFilepath = pszPath;
				if (!PathFileExists(csBroswseFilepath))
				{
					return false;
				}
			}
		}
		delete[] pszPath;
		pszPath = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::BrowseFileToSaveRecover", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :           DeleteRThread
*  Description    :           this function is called for deletion of records
*  Author Name    :           Nitin K. Kolapkar
*  SR_NO		  :
*  Date           :           13th June 2014
****************************************************************************************************/
DWORD WINAPI DeleteRThread(LPVOID lpParam)
{
	try
	{
		CWardWizRecover *pThis = (CWardWizRecover *)lpParam;
		pThis->m_bDeleteStop = false;
		pThis->DeleteREntries();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::BrowseFileToSaveRecover", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***************************************************************************************************
*  Function Name  :           DeleteREntries
*  Description    :           This function is called for deletion of records
*  Author Name    :           Nitin K. Kolapkar
*  SR_NO		  :
*  Date           :           13th June 2014
****************************************************************************************************/
void CWardWizRecover::DeleteREntries()
{
	//BOOL bCheck;
	bool bSuccess = true;
	CString csFilePath = L"";
	CString csMsgBox = L"";
	bool	bSelected = false;
	CString csThreatPath = L"", csVirusName = L"";
	int iCurrentValue, count = 0;
	m_bDeleteThreadStart = true;
	bool	isAnyEntrySeletected = false;
	try
	{
		AddLogEntry(L">>> Deleting entry started", 0, 0, true, FIRSTLEVEL);
		if (!SendRecoverOperations2Service(RELOAD_DBENTRIES, L"", L"", RECOVER, 0, true))
		{
			AddLogEntry(L"### Error in CRecoverDlg::OnBnClickedButtonDelete", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}
		DWORD dwDeleteCount = 0;
		do
		{
			count = m_svArrRecoverEntries.length();
			for (iCurrentValue = count; iCurrentValue >= 0; iCurrentValue--)
			{
				if (m_bDeleteStop)
				{
					break;
				}
				const SCITER_VALUE EachEntry = m_svArrRecoverEntries[iCurrentValue];
				bool bValue = EachEntry[L"selected"].get(false);
				if (bValue)
				{
					const std::wstring chThreatPath = EachEntry[L"FilePath"].get(L"");
					csThreatPath = chThreatPath.c_str();
					Sleep(5);
					ISPY_PIPE_DATA	szPipeData = { 0 };
					szPipeData.iMessageInfo = RECOVER_FILE;
					_tcscpy_s(szPipeData.szFirstParam, csThreatPath);
					_tcscpy_s(szPipeData.szSecondParam, L"");
					szPipeData.dwValue = 0;
					szPipeData.dwSecondValue = 1;
					isAnyEntrySeletected = true;
					SendRecoverOperations2Service(&szPipeData, true);
					CString csCurrnetFile = L"";
					csCurrnetFile.Format(L"%d", iCurrentValue);
					if (!szPipeData.dwValue)
					{
						UpdateRecoverTable(1, iCurrentValue, L"Deleted");
					}
					else
					{
						bSuccess = false;
						AddLogEntry(L"### Error in CRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
					}

					bSelected = true;
					dwDeleteCount++;
				}
				count--;
			}
			if (iCurrentValue <= count)
				break;
		} while (isAnyEntrySeletected);

		Sleep(5);
		if (!SendRecoverOperations2Service(SAVE_RECOVERDB, L"", L"", 0, 0, true))
		{
			AddLogEntry(L"### Error in CRecoverDlg::SendRecoverOperations2Service SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
		}
		Sleep(5);
		LoadExistingRecoverFile(false);
		if (!isAnyEntrySeletected)
		{
			m_csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_NO_SELECT_ENTRY"));
			m_svFunOprFinishedStatusCB.call((SCITER_STRING)m_csMsgBox);
			m_bIsMultiRecoverFileFinish = true;
			if (theApp.m_bIsRecoverFileUIReceptive)
			{
				if (theApp.m_bIsRecoverFilePageSwitched)
				{
					OnCallSetFinishStatus();
				}
			}
			goto Cleanup;
		}

		if (!m_bDeleteStop)
		{
			TCHAR szRecoverMsg[MAX_PATH] = { 0 };
			CString csMsgBox(L"");
			if (dwDeleteCount == 0)
			{
				m_csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_NO_FILE_DELETED"));
			}
			else
			{
				m_csMsgBox.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_FILES_DELETED_SUCCESSFULLY"));
			}
		}
		m_svFunOprFinishedStatusCB.call((SCITER_STRING)m_csMsgBox);
		m_bIsMultiRecoverFileFinish = true;
		if (theApp.m_bIsRecoverFileUIReceptive)
		{
			if (theApp.m_bIsRecoverFilePageSwitched)
			{
				OnCallSetFinishStatus();
			}
		}

		AddLogEntry(L">>> recovering entry finished", 0, 0, true, FIRSTLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::DeleteREntries", 0, 0, true, SECONDLEVEL);
	}
Cleanup:
	m_bDeleteThreadStart = false;
	return;
}

/***************************************************************************************************
*  Function Name  : UpdateRecoverTable
*  Description    : Calls fire event to update UI
*  Author Name    : Nitin Kolapkar
*  Date			  : 16 Oct 2016
****************************************************************************************************/
void CWardWizRecover::UpdateRecoverTable(int iMsgType, int iCurrentFileNo, SCITER_STRING csActionTaken)
{
	try
	{
		sciter::value map;
		map.set_item("one", iMsgType);
		map.set_item("two", iCurrentFileNo);
		map.set_item("three", csActionTaken);
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = UPDATE_RECOVER_TABLE_EVENT;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScan::CallUISetStatusfunction", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : This function is used send DB file path to UI
*  Author Name    : Amol J.
*  Date           :	24 May 2017
****************************************************************************************************/
json::value CWardWizRecover::GetDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBEXCLUDE.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : On_CallQuarantineRecover
*  Description    : This function is used to quarantine files by dragging/dropping
*  Author Name    : Akshay Patil
*  Date           :	05 October 2018
****************************************************************************************************/
json::value CWardWizRecover::On_CallQuarantineRecover(SCITER_VALUE svQuarantineFilePath)
{	
	try
	{
		CString csQuarFilePath = svQuarantineFilePath.get(L"").c_str();
		HANDLE hFile = CreateFile(csQuarFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD dwFileSize = GetFileSize(hFile,NULL);
		if (dwFileSize == 0x00)
		{
			CloseHandle(hFile);
			return false;
		}
		CloseHandle(hFile);
		if (!SendRecoverOperations2Service(ON_QUARANTNE_RECOVER, csQuarFilePath, L"", 0, 0, true))
		{
			AddLogEntry(L"### Error in CVibraniumRecover::SendRecoverOperations2Service ON_QUARANTNE_RECOVER", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::On_CallQuarantineRecover", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CheckIsNetworkPath
*  Description    : To check weather file is network file or not.
*  Author Name    : Akshay Patil
*  Date           :	05 October 2018
****************************************************************************************************/
json::value CWardWizRecover::CheckIsNetworkPath(SCITER_VALUE svFilePathQuarantine)
{
	try
	{
		const std::wstring  chFilePath = svFilePathQuarantine.get(L"");
		if (PathIsNetworkPath((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::CheckForNetworkPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : CheckIsFilePath
*  Description    : To check path is file or not.
*  Author Name    : Akshay Patil
*  Date           :	15 October 2018
****************************************************************************************************/
json::value CWardWizRecover::CheckIsFilePath(SCITER_VALUE svFilePathQuarantine)
{
	try
	{
		const std::wstring csPath = svFilePathQuarantine.get(L"");
		if (PathIsDirectory((LPTSTR)csPath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::CheckIsFilePath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : CheckIsWrdWizFile
*  Description    : To check path is WardWiz own file or not.
*  Author Name    : Akshay Patil
*  Date           :	16 October 2018
****************************************************************************************************/
json::value CWardWizRecover::CheckIsWrdWizFile(SCITER_VALUE svFilePathQuarantine)
{
	try
	{
		CString csQuarFilePath = svFilePathQuarantine.get(L"").c_str();
		CString csWardWizPath = GetWardWizPathFromRegistry();

		if (csWardWizPath.CompareNoCase(csQuarFilePath.Left(csWardWizPath.GetLength())) == 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::CheckIsGenXFile", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : On_CallContinueFullScan
Description    : used to continue recovering of file 
SR.NO		   :
Author Name    : Kunal Waghmare
Date           : 23th May 2019
***********************************************************************************************/
json::value CWardWizRecover::On_CallContinueRecoverFile()
{
	try
	{
		theApp.m_bIsRecoverFileUIReceptive = false;
		theApp.m_bIsRecoverFilePageSwitched = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::On_CallContinueRecoverFile", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : OnSetTimer
Description    : Called when recovering of file is to be continued
SR.NO		   :
Author Name    : Kunal Waghmare
Date           : 23th May 2019
***********************************************************************************************/
json::value CWardWizRecover::OnSetTimer()
{
	try
	{
		theApp.m_bIsRecoverFileUIReceptive = true;
		if (m_bIsMultiRecoverFileFinish)
		{
			OnCallSetFinishStatus();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::OnSetTimer", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : OnCallSetFinishStatus
Description    : Called when recovering of file is finished
SR.NO		   :
Author Name    : Kunal Waghmare
Date           : 23th May 2019
***********************************************************************************************/
void CWardWizRecover::OnCallSetFinishStatus()
{
	try
	{
		sciter::value map;
		map.set_item("msg", sciter::string(m_csMsgBox));

		//Send here event
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SETRECOVERFILEFINISHED_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::OnCallSetFinishStatus", 0, 0, true, SECONDLEVEL);
	}
}
/***************************************************************************************************
*  Function Name  : OnClickSetPage
*  Description    : This function is used to set the setting page name.
*  Author Name    : Swapnil Bhave
*  Date           :	17 Spt 2019
****************************************************************************************************/
json::value CWardWizRecover::OnClickSetPage()
{
	try
	{
		theApp.m_csPageName = L"#RECOVER_FILES";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::OnClickSetPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetTargetFilePath
*  Description    :	To get target path of '.lnk' file.
*  Author Name    : Nitin Shelar
*  Date           : 22/10/2019
**********************************************************************************************************/
json::value CWardWizRecover::GetTargetFilePath(SCITER_VALUE svFilePath)
{
	IShellLink* pIShellLink;
	try
	{
		CString csFilePath = svFilePath.get(L"").c_str();
		CString csTargetFilePath;
		HRESULT hr;
		hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pIShellLink);
		if (SUCCEEDED(hr))
		{
			IPersistFile* pIPersistFile;
			hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);
			if (SUCCEEDED(hr))
			{
				hr = pIPersistFile->Load(T2COLE(csFilePath), STGM_READ);
				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = pIShellLink->GetPath(csTargetFilePath.GetBuffer(MAX_PATH),
						MAX_PATH,
						&wfd,
						SLGP_UNCPRIORITY | SLGP_RAWPATH);
					csTargetFilePath.ReleaseBuffer(-1);
				}
				else
					return json::value(SCITER_STRING(L""));
			}
			pIPersistFile->Release();
			pIShellLink->Release();
		}
		return json::value(SCITER_STRING(csTargetFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRecover::GetTargetFilePath", 0, 0, true, SECONDLEVEL);
	}
	return json::value(SCITER_STRING(L""));
}