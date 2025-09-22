/****************************** Module Header ******************************\
* Module Name:  WardwizClientAgent.h
* Project:      WardwizClientAgent
* Copyright (c)
* 
* Provides a service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* \***************************************************************************/
#ifdef __windows__
#undef __windows__
#endif
#pragma once
#include "ISpyCriticalSection.h"
#include "NetworkTasks.h"
#include "NetworkInstallation.h"
#include "ScanTask.h"
#include "ISpyCommServer.h"
#include "UninstallClient.h"

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif


#include <tchar.h>
#include <iomanip>
#include <sstream>
#include <thread>
#include <ctime>
#include <cpprest\astreambuf.h>
#include <cpprest\http_listener.h>
#include "ServiceBase.h"
//#include "CommonFunctions.h"
#include "WardwizServiceInvoker.h"
#include "WWizRegistration.h"
#include "WWizProdDownloader.h"
#include "iTINRegWrapper.h"

using namespace web::http::experimental::listener;
using namespace web::http;
using namespace web;

using namespace concurrency;

class WardWizJsonValue
{
public:
	json::value value;
};

class CWardwizClientAgent : public CServiceBase
{
public:

    CWardwizClientAgent(PWSTR pszServiceName, 
        BOOL fCanStop = TRUE, 
        BOOL fCanShutdown = TRUE, 
        BOOL fCanPauseContinue = FALSE);
  virtual ~CWardwizClientAgent(void);

	CWardwizServiceInvoker objServiceInvoker;
	int GetProcessCount();
	bool InvokeQuickScan();
	bool InvokeCustomScan(CString csFileName);
	bool InvokeSettingsUpdater();
	bool DownloadFileContent(http_request req, std::string strFileToDownload);
	bool DownloadBinaryFileContent(http_request req, std::string strFileToDownload);
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
};
