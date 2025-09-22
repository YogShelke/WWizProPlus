/****************************************************
*  Program Name: CWardWizRecover.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 6th May 2016
*  Version No: 2.0.0.1
****************************************************/

/****************************************************
HEADER FILES
****************************************************/

#pragma once

#include "WardWizUI.h"
#include "ISpyDataManager.h"
#include "iTinEmailContants.h"
#include "WardWizDatabaseInterface.h"
class CWardWizRecover : sciter::event_handler,
	sciter::behavior_factory
{
	HELEMENT self;

public:
	CWardWizRecover();
	~CWardWizRecover();

	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}

public:
	HANDLE						m_hThread_Recover;
	HANDLE						m_hThread_Delete;
	HANDLE						m_hShowRecoverEntriesThread;

	CDataManager				m_objRecoverdb;
	CDataManager				m_objRecoverdbToSave;
	CISpyCommunicator			m_objCom;

	bool						m_bRecoverStop;
	bool						m_bRecoverBrowsepath;
	bool						m_bRecoverThreadStart;
	bool						m_bDeleteStop;
	bool						m_bDeleteThreadStart;
	bool						m_bShowRecoverEntries;
	int							m_iTotalEntriesCount;
	bool						m_bExclude;
	bool						m_bIsMultiRecoverFileFinish;
	sciter::dom::element		ela;
	CString						m_csMsgBox;
public:
	//Sciter related variables
	SCITER_VALUE				m_svUpdateRecoverTableCB;
	SCITER_VALUE				m_svShowRecoverEntriesCB;
	SCITER_VALUE				m_svArrRecoverEntries;
	SCITER_VALUE				m_svFunOprFinishedStatusCB;
	SCITER_VALUE				m_svLoadEntriesFinishedCB;
	SCITER_VALUE				m_svFunShowNotificationRecoverCB;
	SCITER_VALUE				m_svFunSendFilePathCB;

public:
	BEGIN_FUNCTION_MAP

		//"Recover" related functions
		FUNCTION_2("OnLoadRecoverEntries", On_LoadRecoverEntries) // On_LoadRecoverEntries()	
		FUNCTION_4("OnButtonRecoverEntries", On_ButtonRecoverEntries) // On_ButtonRecoverEntries()	
		FUNCTION_3("OnButtonDeleteRecoverEntries", On_ButtonDeleteRecoverEntries) // On_ButtonDeleteRecoverEntries()	
		FUNCTION_0("OnButtonPauseRecoverOperation", On_ButtonPauseRecoverOperation) // On_LoadRecoverEntries()	
		FUNCTION_0("OnButtonResumeRecoverOperation", On_ButtonResumeRecoverOperation) // On_LoadRecoverEntries()	
		FUNCTION_0("OnButtonStopRecoverOperation", On_ButtonStopRecoverOperation) // On_LoadRecoverEntries()
		FUNCTION_1("OnGetPathForExclude", On_OnGetPathForExclude);
		FUNCTION_0("GetDBPath", GetDBPath)
		FUNCTION_1("OnCallQuarantineRecover", On_CallQuarantineRecover)
		FUNCTION_1("CheckIsNetworkPath", CheckIsNetworkPath)
		FUNCTION_1("CheckIsFilePath", CheckIsFilePath)
		FUNCTION_1("CheckIsWrdWizFile", CheckIsWrdWizFile)
		FUNCTION_0("OnSetTimer", OnSetTimer)
		FUNCTION_0("CallContinueRecoverFile", On_CallContinueRecoverFile)
		FUNCTION_1("GetTargetFilePath", GetTargetFilePath)
		FUNCTION_0("SetPage", OnClickSetPage)
	END_FUNCTION_MAP

	//"Recover" related functions
	json::value On_LoadRecoverEntries(SCITER_VALUE svFunShowRecoverEntriesCB, SCITER_VALUE svLoadEntriesFinishedCB);
	json::value On_ButtonRecoverEntries(SCITER_VALUE svArrRecoverEntries, SCITER_VALUE svFunUpdateRecoverEntryTableCB, SCITER_VALUE svFunOprFinishedStatusCB,  SCITER_VALUE svbExclude);
	json::value On_ButtonDeleteRecoverEntries(SCITER_VALUE svArrRecoverEntries, SCITER_VALUE svFunUpdateRecoverEntryTableCB, SCITER_VALUE svFunOprFinishedStatusCB);
	json::value On_ButtonPauseRecoverOperation();
	json::value On_ButtonResumeRecoverOperation();
	json::value On_ButtonStopRecoverOperation();
	json::value On_OnGetPathForExclude(SCITER_VALUE FunSendFilePath);
	json::value GetDBPath();
	json::value On_CallQuarantineRecover(SCITER_VALUE svQuarantineFilePath);
	json::value CheckIsNetworkPath(SCITER_VALUE svFilePathQuarantine);
	json::value CheckIsFilePath(SCITER_VALUE svFilePathQuarantine);
	json::value CheckIsWrdWizFile(SCITER_VALUE svFilePathQuarantine);
	json::value OnSetTimer();
	json::value On_CallContinueRecoverFile();
	json::value OnClickSetPage();
	json::value GetTargetFilePath(SCITER_VALUE svFilePath);

	void OnBnClickedRecover();
	void OnBnClickedRecover(SCITER_VALUE svArrRecoverEntries, SCITER_VALUE UpdateRecoverEntryTableCB, SCITER_VALUE svFunOprFinishedStatusCB, SCITER_VALUE svbExclude);
	void OnBnClickedDeleteRecover(SCITER_VALUE svArrRecoverEntries, SCITER_VALUE UpdateRecoverEntryTableCB, SCITER_VALUE svFunOprFinishedStatusCB);
	
	CString GetQuarantineFolderPath();

	void RecoverEntries();
	void DeleteREntries();
	bool isAnyEntrySeletected();
	void LoadExistingRecoverFile(bool bType);
	void PopulateList(bool bCheckEntry = false);
	void LoadRemainingEntries();
	void GetFileDigits(LPCTSTR pstr, vector<int> &vec);

	bool GetSortedFileNames(vector<int> &vec);
	bool LoadDataContentFromFile(CString csPathName);
	bool SaveDBFile();
	bool SendRecoverOperations2Service(int iMessageInfo, CString csRecoverFileEntry, CString csBrowseRecoverFileEntry, DWORD dwType, DWORD dwTypeofAction, bool bWait = false);
	bool BrowseFileToSaveRecover(CString &csBroswseFilepath);
	bool SendRecoverOperations2Service(ISPY_PIPE_DATA *pszPipeData, bool bWait);
	void UpdateRecoverTable(int iMsgType, int iCurrentFileNo, SCITER_STRING csActionTaken);
	void OnCallSetFinishStatus();
};

