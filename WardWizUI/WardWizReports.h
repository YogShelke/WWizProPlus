/****************************************************
*  Program Name: CWardWizRecover.h
*  Author Name: Jeena Saji
*  Date Of Creation: 6th June 2016
*  Version No: 2.0.0.1
****************************************************/
#pragma once
#include "WardWizUI.h"

class CWardWizReports : sciter::event_handler,
						sciter::behavior_factory
{
public:
	CWardWizReports();
	~CWardWizReports();

	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {

	}
	virtual void detached(HELEMENT he) {
	}

	BEGIN_FUNCTION_MAP
		//Reports Page related functions
		FUNCTION_3("OnLoadReportsDB", On_LoadReportsDB)
		FUNCTION_0("OnPauseReportsLoad", On_PauseReportsLoad) 	
		FUNCTION_0("OnResumeReportsLoad", On_ResumeReportsLoad) 	
		FUNCTION_0("OnStopReportsLoad", On_StopReportsLoad) 
		
		FUNCTION_3("OnClickReportsDelete", On_ClickReportsDelete)
		FUNCTION_0("OnPauseReportsDelete", On_PauseReportsDelete)
		FUNCTION_0("OnResumeReportsDelete", On_ResumeReportsDelete)
		FUNCTION_0("OnStopReportsDelete", On_StopReportsDelete)
		FUNCTION_0("ReadReportsDBPath",GetDBPath)
		FUNCTION_0("funOnSaveRecords", On_funOnSaveRecords)
	END_FUNCTION_MAP

	//Report Page Related Functions
	json::value On_LoadReportsDB(SCITER_VALUE svFunDispReportsCB, SCITER_VALUE svFunLoadReportsFinishedCB, SCITER_VALUE svFunShowNotificationReportsCB);
	json::value On_ClickReportsDelete(SCITER_VALUE svarrReports, SCITER_VALUE svFunDeleteReportsEntriesinTableCB, SCITER_VALUE svFunDeleteReportsFinishedCB);
	json::value On_PauseReportsLoad();
	json::value On_ResumeReportsLoad();
	json::value On_StopReportsLoad();

	json::value On_ResumeReportsDelete();
	json::value On_PauseReportsDelete();
	json::value On_StopReportsDelete();

	json::value GetDBPath();
	json::value On_funOnSaveRecords();

	SCITER_VALUE		m_svFunDispReportsCB;
	SCITER_VALUE		m_svFunDeleteReportsEntriesinTableCB;
	SCITER_VALUE		m_svArrReportsDelete;
	SCITER_VALUE	    m_svFunLoadReportsFinishedCB;
	SCITER_VALUE		m_svFunShowNotificationReportsCB;
	SCITER_VALUE		m_svFunDeleteReportsFinishedCB;
	SCITER_VALUE        m_svFunValuesFromINICB;

	HANDLE				m_hThreadLoadReports;
	HANDLE				m_hThreadDeleteReports;
	CDataManager		m_objReportsDBToSave;
	CDataManager		m_objReportsDB;
	CISpyCommunicator	m_objCom;


	bool				m_bReportEnableDisable;
	bool				m_bDeleteEntriesFinished;
	bool				m_bReportStop;
	bool				m_bReportThreadStart;

	bool LoadDBFile();
	bool SaveDBFile();
	bool GetSortedFileNames(vector<int> &vec);
	bool GetReportsFolderPath(CString &csFolderPath);
	bool LoadDataContentFromFile(CString csPathName);
	bool GetDateTimeFromString(CString csItemEntry, CTime &objDateTime);
	bool SendReportsData2Service(DWORD dwMessageInfo, DWORD dwType, CString csEntry, bool bReportsWait = false);
	bool SendReportsOperation2Service(DWORD dwType, CString csDateTime, CString csScanType, CString csFilePath, bool bReportsWait = false);

	void PopulateList();
	void LoadRemainingEntries();
	void GetFileDigits(LPCTSTR pstr, vector<int> &vec);
	void OnBnClickedBtnDelete();
};

