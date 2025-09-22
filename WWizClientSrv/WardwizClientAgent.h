/****************************** Module Header ******************************\
* Module Name:  SampleService.h
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once
#ifdef __windows__
#undef __windows__
#endif

#include <vector>
#include <cpprest/json.h>
#include "ServiceBase.h"
#include "ISpyCommServer.h"
#include "iTINRegWrapper.h"
#include "ISpyCriticalSection.h"
#include "NetworkTasks.h"
#include "NetworkInstallation.h"
#include "ScanTask.h"
#include "WWizRegistration.h"
#include "UninstallClient.h"
#include "WardWizDatabaseInterface.h"
#include <cpprest\http_listener.h>

using namespace web::http::experimental::listener;
using namespace web::http;
using namespace web;
using namespace concurrency;

class CWardwizClientAgent : public CServiceBase
{
public:
	CWardwizClientAgent(PWSTR pszServiceName,
		BOOL fCanStop = TRUE,
		BOOL fCanShutdown = TRUE,
		BOOL fCanPauseContinue = FALSE);
	virtual ~CWardwizClientAgent(void);
protected:
	virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
	virtual void OnStop();

	void ServiceWorkerThread(void);
	void ServiceWorkerThreadSEH(void);
	void FunctionGetTaskID(void);
	void FunctionGetTaskIDSEH(void);
private:
	BOOL					m_fStopping;
	HANDLE					m_hStoppedEvent;
	int						m_iTotalFileCount;
	int						m_iThreatsFoundCount;
	CString					m_csVirusName;
	bool					m_bThreatDetected;
	DWORD					m_dwTimeRequired;
	CNetworkTasks			m_objNetworkTasks;
	CNetworkInstallation	m_objNetworkInstall;
	CUninstallClient		m_objUnInstallClient;
	CScanTask				m_objScanTask;
	json::value				m_jsTask;
	json::value				m_jsInstallationTask;
	json::value				m_jsTaskDetails;
	CString					m_csMachineID;
	CWWizRegistration		m_objRegistration;
	CISpyCriticalSection	m_csNetworkDiscovery;
	CISpyCriticalSection	m_csNetworkInstall;
	CISpyCommunicatorServer	m_objCommSrv;
	static CWardwizClientAgent	 		*m_pThis;
	CWardWizSQLiteDatabase	m_objSqlDb;
public:
	static void OnDataReceiveCallBack(LPVOID lpParam);
	void RespondForCustomScan(const http_request& request, const status_code& status, const json::value& response);
	void Respond(const http_request& request, const status_code& status, const json::value& response);
	pplx::task<void> GetNetworkTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	pplx::task<void> GetInstallationTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool StartInstallationTask(json::value& v);
	void CheckRegistationStatusThread(void);
	void CheckInstallSetup(void);
	bool InvokeNetworkTask(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool InstallClientsTask(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool InvokeTask(json::value& v);
	bool StartNetworkDignoseTask(json::value& v);
	pplx::task<void> GetTaskID();
	void SignalAllThreads();
	bool UpdateTaskStatus(std::vector<web::json::value> &v);
	bool UpdateTaskStatusSEH(std::vector<web::json::value> &v);
	void GetBearerCode(bool bWait = false);
	pplx::task<void> GetBearerCodeTask();
	void GetBearereCodeThread(void);
	void GetBearereCodeThreadSEH(void);
	bool ScanTask(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	pplx::task<void> GetScanTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool StartScanTask(json::value& v);
	bool UnInstallClientsTask(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	pplx::task<void> GetUnInstallationTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool StartUnInstallationTask(json::value& v);
	bool RestartMachine(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	void UpdateMachineInfo(void);
	void UpdateMachineInfoSEH(void);
	pplx::task<void> UpdateMachineInfoTask();
	bool ApplyPolicyTask(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool ReconfigureFeature(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	pplx::task<void> GetApplyPolicyTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	pplx::task<void> GetReconfiguraionDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool StartApplyPolicyTask(json::value& v);
	bool StartApplyConfigTask(json::value& v);
	bool SendVirusEntry2Server(LPISPY_PIPE_DATA lpSpyData);
	bool UpdateScanFinishedTask(LPISPY_PIPE_DATA lpSpyData);
	bool UpdateFinishedTask(LPISPY_PIPE_DATA lpSpyData);
	pplx::task<void> SendVirusEntry2Server(CString csTaskID, CString csThreatName, CString csThreatPath, DWORD dwAction);
	pplx::task<void> GetRestartMachineTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool RestartMachine(json::value& v);
	BOOL WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);
	pplx::task<void> UpdateClientMachine(utility::string_t strTaskID, utility::string_t strTaskTypeID);
	bool UpdateTaskStatus(LPISPY_PIPE_DATA lpSpyData);
	bool UpdateMachine(json::value& v);
	bool AddClientErrorServerLog(LPISPY_PIPE_DATA lpSpyData);
	DWORD SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait = false);
	bool StartALUpdateUsingAlupdateService(CString csTaskID);
	concurrency::task<DWORD> InstallClientSetup(utility::string_t strTaskID, utility::string_t strTaskTypeID, utility::string_t strMachineIP, utility::string_t strUserName,
		utility::string_t strPassword);

	void InsertIntoDB(std::string &varSchedType, std::string &varLowBattery, std::string &varShutDown, std::string &varWakeUp, std::string &varSkipScan, std::string &varStartTime, std::string &bScanSunday,
	std::string &bScanMonday, std::string &bScanTuesday, std::string &bScanWednesday, std::string &bScanThursday, std::string &bScanFriday,
	std::string &bScanSaturday, std::string &varScanType, std::string &varCurrentDate);
	INT64 CWardwizClientAgent::InsertDataToTable(const char* szQuery);
	void InsertConfigIntoDB(std::string &varFeatureName, std::string &bSetValue, std::string &intFeatureID);
	void InsertPaswdConfigIntoDB(std::string varClientPaswd);
	INT64 CWardwizClientAgent::InsertDataToTableFeature(const char* szQuery);
	bool SendData2UI(int iMessageInfo, bool bWait = false);
};
