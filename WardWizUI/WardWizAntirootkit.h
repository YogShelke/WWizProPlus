/****************************************************
*  Program Name: CWardWizAntirootkit.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28th March 2016
*  Version No: 2.0.0.1
****************************************************/
#pragma once
#include "WardWizUI.h"
#include "iSpyMemMapClient.h"
#include "iTinEmailContants.h"

#define DETECTEDSTATUS			L"Detected"

class CWardWizAntirootkit : sciter::event_handler,
							sciter::behavior_factory
{
	HELEMENT self;
public:
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
	BEGIN_FUNCTION_MAP
		//"Recover" related functions
		FUNCTION_3("OnStartAntirootkitScan", On_StartAntirootkitScan)
		FUNCTION_0("OnPauseAntirootScan", On_PauseAntirootScan)
		FUNCTION_0("OnResumeAntirootScan", On_ResumeAntirootScan)
		FUNCTION_1("OnStopAntirootScan", On_StopAntirootScan)
		FUNCTION_4("OnClickAntirootCleanBtn", On_ClickAntirootCleanBtn)
		FUNCTION_1("OnClickShutDownScan", On_ClickShutDownScan)
		FUNCTION_0("OnCallContinueARootScan", OnCallContinueARootScan)
		FUNCTION_0("OnSetTimer", OnSetTimer)
		FUNCTION_1("ScanFinishedCall", On_ScanFinishedCall)
		FUNCTION_1("GetAntirootScanStatus", GetAntirootScanStatus)
	END_FUNCTION_MAP

	//Antirootkit related functions
	json::value On_StartAntirootkitScan(SCITER_VALUE svAntirootkit, SCITER_VALUE svShowScanFinshedPageCB, SCITER_VALUE svAddVirusFoundEntryCB);
	json::value On_PauseAntirootScan();
	json::value On_ResumeAntirootScan();
	
	json::value On_StopAntirootScan(SCITER_VALUE svbIsManualStop);
	json::value On_ClickAntirootCleanBtn(SCITER_VALUE scRootkitScanOption, SCITER_VALUE svArrCleanVirusEntries, SCITER_VALUE svFunUpdateVirusFoundEntryCB, SCITER_VALUE svFunNotificationMsgCB);
	json::value On_ClickShutDownScan(SCITER_VALUE svIsShutDownPC);
	json::value OnCallContinueARootScan();
	json::value OnSetTimer();
	json::value On_ScanFinishedCall(SCITER_VALUE svGetScanFinishedFlag);
	json::value GetAntirootScanStatus(SCITER_VALUE svIsScanRunning);
public:
	CWardWizAntirootkit();
	~CWardWizAntirootkit();

	int						m_icount;
	CString					csTotalFileCnt = L"";
	CString					csTotalDriveCnt = L"";
	CString					csTotalProcCnt = L"";
	CString					csDetectedFileCnt = L"";
	CString					csDetectedDriveCnt = L"";
	CString					csDetectedProcCnt = L"";

	bool					m_bAntirootkitScan;
	bool					m_bCheckFileFolder;
	bool					m_bCheckProcess;
	bool					m_bCheckRegistry;
	bool					m_bAntirootScanningInProgress;
	bool					m_bScanningFinished;
	bool					m_bAntirootClose;
	bool					m_bAntirootkitHome;
	bool					m_bRedFlag;
	bool					m_bValue;
	bool				    m_bProcessTabSelected;
	bool					m_bRegistryTabSelected;
	bool					m_bFilesFoldersTabSelected;
	bool					m_bScanningStopped;
	bool					m_bStop;
	bool					m_bIsManualStop;
	bool					m_bIsShutDownPC;
	bool					m_bIsMultiAScanFinish;
	bool					ShutDownRootkitScanning();
	bool					SendRequestCommon(int iRequest);
	bool					isAnyEntrySeletected(SCITER_VALUE svAntirootkit);
	bool					StartAntirootScanUsingService();
	bool					RootKitScanningStarted();
	bool					RootKitScanningFinished();
	bool					PauseScan();
	bool					ResumeScan();
	bool					SendReportOperations2Service(int dwMessageinfo, CString csReportFileEntry, DWORD dwType, bool bWait = false);
	bool					SetUIStatus(DWORD dwCntrlID, CString csStatus = EMPTY_STRING);
	bool					SendFile4RepairUsingService(int iMessage, CString csEntryOne, CString csEntryTwo, CString csEntryThree, DWORD dwISpyID, DWORD dwScanType, bool bWait = false);
	bool					SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);
	bool					GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt);
	DWORD					m_dwType;
	DWORD					m_dwScannedFileFolder;
	DWORD					m_dwPercentage;
	DWORD					m_dwGetCheckValues;
	DWORD					m_dwScannedCntRegistry;
	DWORD					m_dwScannedCnt;
	DWORD					m_dwThreatCntFilefolder;
	DWORD					m_dwThreatCnt;
	DWORD					m_dwThreatCntRegistry;
	DWORD					m_dwRootKitOption;
	DWORD					m_dwTotalType;
	DWORD					m_dwForInsertItem;
	INT64					m_iScanSessionId;
	
	iSpyServerMemMap_Client m_objIPCRootkitClient;
	
	HANDLE					m_hGetPercentage;
	HANDLE			 		m_hCleaningThread;
	
	SCITER_VALUE			m_svArrCleanVirusEntries;
	SCITER_VALUE			m_svSetScanPercentageCB;
	SCITER_VALUE			m_pSetPauseStatusCb;
	SCITER_VALUE			m_svFunScanningFinishedCB;
	SCITER_STRING			AllParam;
	SCITER_VALUE			m_svUpdateFileCountCB;
	SCITER_VALUE			m_svAddVirusFoundEntryCB;
	SCITER_VALUE			m_svFunUpdateVirusFoundEntryCB;
	SCITER_VALUE			m_svFunNotificationMsgCB;
	SCITER_VALUE			m_svGetScanFinishedFlag;
	SCITER_VALUE			m_svIsScanRunning;
	CISpyCriticalSection	m_csSyncUICntrls;
	sciter::dom::element	ela;
public:
	void OnBnClickedBtnScan(SCITER_VALUE svAntirootkit);
	void GetDWORDFromRootKitScanOptions(DWORD &dwRegScanOpt);
	void AddDetectedCount(DWORD dwDetectedEntries);
	void InsertItem(CString csFirstParam, CString csSecondParam, CString csThirdParam, CString csForthParam);
	void AddTotalCount(DWORD dwTotalEntries);
	void StopScan();
	void OnBnClickedBtnantirootkitDelete(CString RootkitScanOption, SCITER_VALUE svArrCleanVirusEntries, SCITER_VALUE svFunUpdateVirusFoundEntryCB, SCITER_VALUE  svFunNotificationMsgCB);
	void DeleteEntries();
	void CallRootkitUIFileCountfunction(DWORD dwTotalFileCnt, DWORD dwTotalDriveCnt, DWORD dwTotalProcCnt, DWORD dwDetectedFileCnt, DWORD dwDetectedDriveCnt, DWORD dwDetectedProcCnt);
	void CallRootkitUIReportsfunction();
	void AddEntriesInReportsDB(CString eScanType, CString csThreatName, CString csFilePath, CString csAction);

	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
	void OnCallSetPercentage(CString csPercentage);
	void OnCallSetFinishStatus();
	bool CheckIsScanShutdown();
};

