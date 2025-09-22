/****************************** Module Header ******************************\
* Module Name:  SampleService.cpp
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

#pragma region Includes
#include "WardwizClientAgent.h"
#include "WWizProdDownloader.h"
#include "ThreadPool.h"
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Enumprocess.h"
#include "WardWizDatabaseInterface.h"
#pragma endregion

using namespace web;
using namespace web::http;
using namespace web::http::client;
using boost::property_tree::ptree;

CWardwizClientAgent * CWardwizClientAgent::m_pThis = NULL;

CWardwizClientAgent::CWardwizClientAgent(PWSTR pszServiceName,
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
, m_objCommSrv(EPS_CLIENT_AGENT, &OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA))
{
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
	m_csMachineID = CWWizSettings::GetMachineID();
}


CWardwizClientAgent::~CWardwizClientAgent(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}

//
//   FUNCTION: CWardwizClientAgent::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the 
//   service by the SCM or when the operating system starts (for a service 
//   that starts automatically). It specifies actions to take when the 
//   service starts. In this code sample, OnStart logs a service-start 
//   message to the Application log, and queues the main service function for 
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore, 
//   it usually polls or monitors something in the system. The monitoring is 
//   set up in the OnStart method. However, OnStart does not actually do the 
//   monitoring. The OnStart method must return to the operating system after 
//   the service's operation has begun. It must not loop forever or block. To 
//   set up a simple monitoring mechanism, one general solution is to create 
//   a timer in OnStart. The timer would then raise events in your code 
//   periodically, at which time your service could do its monitoring. The 
//   other solution is to spawn a new thread to perform the main service 
//   functions, which is demonstrated in this code sample.
//
void CWardwizClientAgent::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
	// Log a service start message to the Application log.
	WriteEventLogEntry(L"CWardwizClientAgent in OnStart", EVENTLOG_INFORMATION_TYPE);
	// Queue the main service function for execution in a worker thread.
	m_objCommSrv.Run();
	CClientAgentThreadPool::QueueUserWorkItem(&CWardwizClientAgent::ServiceWorkerThread, this);
	CClientAgentThreadPool::QueueUserWorkItem(&CWardwizClientAgent::GetBearereCodeThread, this);
	//CClientAgentThreadPool::QueueUserWorkItem(&CWardwizClientAgent::CheckRegistationStatusThread, this);
	//CClientAgentThreadPool::QueueUserWorkItem(&CWardwizClientAgent::CheckInstallSetup, this);
}


void CWardwizClientAgent::GetBearereCodeThread(void)
{
	OutputDebugString(L">>> In CWardwizClientAgent::ServiceWorkerThread");
	__try
	{
		GetBearereCodeThreadSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::GetBearereCodeThread", 0, 0, true, SECONDLEVEL);
	}
}



void CWardwizClientAgent::GetBearereCodeThreadSEH(void)
{
	try
	{
		OutputDebugString(L">>> Before GetBearereCodeThreadSEH");

		while (!m_fStopping)
		{
			GetBearerCode();
			WaitForSingleObject(m_hStoppedEvent, 60 * 1000);
		}
	}
	catch (exception const & e)
	{
		CString strError = CString(e.what());
		AddLogEntry(L"### Exception in CWardwizClientAgent::GetBearereCodeThreadSEH!! : %s", strError, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::GetBearereCodeThreadSEH!! ", 0, 0, true, SECONDLEVEL);
	}
}


//
//   FUNCTION: CWardwizClientAgent::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void CWardwizClientAgent::ServiceWorkerThread(void)
{
	OutputDebugString(L">>> In CWardwizClientAgent::ServiceWorkerThread");

	__try
	{
#ifdef _DEBUG
		//Debug purpose
		Sleep(10 * 1000);
#endif

		ServiceWorkerThreadSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::ServiceWorkerThread", 0, 0, true, SECONDLEVEL);
	}
}

void CWardwizClientAgent::ServiceWorkerThreadSEH(void)
{
	try
	{
		OutputDebugString(L">>> Before IsWardWizRegistered");

		//for temporary purpose
		//GetBearerCode(true);

		if (!m_objRegistration.IsWardWizRegistered())
		{
			if (!m_objRegistration.CheckRegistrationStatus())
			{
				if (!m_objRegistration.m_bISRegistered)
				{
					AddLogEntry(L"### Failed to register client", 0, 0, true, SECONDLEVEL);
					return;
				}
			}

			if (!m_objRegistration.m_bISRegistered)
			{
				AddLogEntry(L"### Failed to register client", 0, 0, true, SECONDLEVEL);
				return;
			}

			if (!SendData2UI(SEND_ACTIVE_PROTECTION_STATUS, false))
			{
				AddLogEntry(L"### Failed to SendData to UI CWardwizClientAgent::ServiceWorkerThreadSEH", 0, 0, true, SECONDLEVEL);
			}

			if (!SendData2Tray(RELOAD_WIDGETS_UI, RELOAD_UPDATES))
			{
				AddLogEntry(L"### Failed to SendData to Tray in CWardwizClientAgent::ServiceWorkerThreadSEH", 0, 0, true, SECONDLEVEL);
			}

			if (SendData2Tray(RELOAD_REGISTARTION_DAYS, 0x00, false) == 0x01)
			{
				AddLogEntry(L"### Exception in CWardwizClientAgent::ServiceWorkerThreadSEH", 0, 0, true, SECONDLEVEL);
			}
		}

		UpdateMachineInfo();

		while (!m_fStopping)
		{
			FunctionGetTaskID();
			WaitForSingleObject(m_hStoppedEvent, 20 * 1000);
		}
	}
	catch (exception const & e)
	{
		CString strError = CString(e.what());
		AddLogEntry(L"Exception Occured in Service Thread!! : %s", strError, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"Exception Occured in Service Thread!! ", 0, 0, true, SECONDLEVEL);
	}

	// Signal the stopped event.
	SetEvent(m_hStoppedEvent);
}

void CWardwizClientAgent::UpdateMachineInfo(void)
{
	__try
	{
		UpdateMachineInfoSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateMachineInfo", 0, 0, true, SECONDLEVEL);
	}
}


void CWardwizClientAgent::UpdateMachineInfoSEH(void)
{
	try
	{
		UpdateMachineInfoTask();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateMachineInfoSEH", 0, 0, true, SECONDLEVEL);
	}
}

void CWardwizClientAgent::FunctionGetTaskID(void)
{
	__try
	{
		FunctionGetTaskIDSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::FunctionGetTaskID", 0, 0, true, SECONDLEVEL);
	}
}

void CWardwizClientAgent::FunctionGetTaskIDSEH(void)
{
	try
	{
		GetTaskID();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::FunctionGetTaskID", 0, 0, true, SECONDLEVEL);
	}
}

//
//   FUNCTION: CWardwizClientAgent::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the 
//   service by SCM. It specifies actions to take when a service stops 
//   running. In this code sample, OnStop logs a service-stop message to the 
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with 
//   SERVICE_STOP_PENDING if the procedure is going to take long time. 
//
void CWardwizClientAgent::OnStop()
{
	// Log a service stop message to the Application log.
	WriteEventLogEntry(L"CWardwizClientAgent in OnStop", EVENTLOG_INFORMATION_TYPE);

	// Indicate that the service is stopping and wait for the finish of the 
	// main service function (ServiceWorkerThread).
	m_fStopping = TRUE;

	SignalAllThreads();

	if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
	{
		throw GetLastError();
	}
}

void CWardwizClientAgent::SignalAllThreads()
{
	m_objNetworkTasks.StopRunningTasks();
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::GetTaskID()
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetTaskID");

	std::vector<web::json::value> arrayMachines;
	web::json::value JsonData;
	JsonData[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	JsonData[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());
	arrayMachines.push_back(JsonData);

	json::value lValue;
	lValue = web::json::value::array(arrayMachines);

	utility::string_t Lreq = L"=" + lValue.to_string();

	CString	csTaskAPI;
	csTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/Task", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csTaskAPI);
	http_client client(csTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			InvokeTask(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			ss << e.what() << endl;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetTaskID, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetTaskID", 0, 0, true, SECONDLEVEL);
		}
	});
}

//   FUNCTION: CWardwizClientAgent::CheckRegistationStatusThread(void)
//   PURPOSE: Function which checks for registration status and acts accordingly
void CWardwizClientAgent::CheckRegistationStatusThread(void)
{
}

void CWardwizClientAgent::CheckInstallSetup(void)
{
	try
	{
		bool bInstall = false;
		DWORD dwProdID = 0x00;
		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wardwiz", L"dwProductID", dwProdID) != 0x00)
		{
			bInstall = true;
		}

		if (dwProdID != 0x05)
		{
			bInstall = true;
		}

		if (bInstall)
		{
			//check here if product is not installed then download and install setup.
			TCHAR szTargetPath[MAX_PATH]  {0};
			CWWizProdDownloader objProductDownload;
			if (objProductDownload.StartDownloadFile(L"http://172.168.1.29/alberta/WardWizEssentialSetupNCIx64.exe", szTargetPath))
			{
				if (PathFileExists(szTargetPath))
				{
					objProductDownload.RunSetupInSilentMode(szTargetPath);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::CheckInstallSetup", 0, 0, true, SECONDLEVEL);
	}
}

bool CWardwizClientAgent::ApplyPolicyTask(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	try
	{
		if (strTaskID.length() == 0 || strTaskTypeID.length() == 0)
			return false;

		OutputDebugString(L">>> In CWardwizClientAgent::ApplyPolicyTask");

		//Send both TaskID and TaskTypeID
		GetApplyPolicyTaskDetails(strTaskID.c_str(), strTaskTypeID.c_str());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::ApplyPolicyTask", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***********************************************************************************************
Function Name  : ReconfigureFeature
Description    : Function to reconfigure settings
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 28 March 2018
***********************************************************************************************/
bool CWardwizClientAgent::ReconfigureFeature(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	try
	{
		if (strTaskID.length() == 0 || strTaskTypeID.length() == 0)
			return false;

		OutputDebugString(L">>> In CWardwizClientAgent::ReconfigureFeature");

		//Send both TaskID and TaskTypeID
		GetReconfiguraionDetails(strTaskID.c_str(), strTaskTypeID.c_str());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::ReconfigureFeature", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::InvokeNetworkTask(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	try
	{
		if (strTaskID.length() == 0 || strTaskTypeID.length() == 0)
			return false;

		OutputDebugString(L">>> In CWardwizClientAgent::InvokeNetworkTask");
		//Send both TaskID and TaskTypeID
		GetNetworkTaskDetails(strTaskID.c_str(), strTaskTypeID.c_str());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::NetworkTasksThread", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

bool CWardwizClientAgent::StartApplyPolicyTask(json::value& v)
{
	OutputDebugString(L">>> In CWardwizClientAgent::StartApplyPolicyTask");
	try
	{
		utility::string_t str1 = v.to_string();
		std::string stringvalues = CW2A(str1.c_str());
		std::istringstream iss(stringvalues);

		if (_tcscmp(str1.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no StartApplyPolicyTask available");
			return false;
		}

		// Read json.
		ptree pt;
		read_json(iss, pt);

		std::size_t found = str1.find(L"Table");
		if (found != std::string::npos)
		{
			for (auto& challenge : pt.get_child("Table"))
			{
				std::string strTaskID;
				std::string strTaskTypeID;

				std::string strAllowBlockAutorunExecution;
				std::string strAllowExecuteBlock;
				std::string strAllowProductUpdate;
				std::string strAllowScanToExternalDevice;
				std::string strAllowThreatDetectionAction;
				std::string strAllowToPrepareCacheInBackground;
				std::string strAllowToUseCaching;
				std::string strAllowWriteBlock;
				std::string strAutoUpdate;
				std::string strDeleteOldReport;
				std::string strEnableSound;
				std::string strExcludeSelf;
				std::string strTryNotification;
				std::string strAllowHeuriticScan;
				std::string strScanComputerStartUp;
				std::string strShowToolTipStartup;
				std::string strShowWardwizGadget;


				CITinRegWrapper objReg;
				for (auto& prop : challenge.second)
				{
					if (prop.first == "TaskID")
					{
						strTaskID = prop.second.get_value<std::string>();
					}
					else if (prop.first == "TaskTypeID")
					{
						strTaskTypeID = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowBlockAutorunExecution")
					{
						strAllowBlockAutorunExecution = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowExecuteBlock")
					{
						strAllowExecuteBlock = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowProductUpdate")
					{
						strAllowProductUpdate = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowScanToExternalDevice")
					{
						strAllowScanToExternalDevice = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowThreatDetectionAction")
					{
						strAllowThreatDetectionAction = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowToPrepareCacheInBackground")
					{
						strAllowToPrepareCacheInBackground = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowToUseCaching")
					{
						strAllowToUseCaching = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowWriteBlock")
					{
						strAllowWriteBlock = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAutoUpdate")
					{
						strAutoUpdate = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isDeleteOldReport")
					{
						strDeleteOldReport = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isEnableSound")
					{
						strEnableSound = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isTryNotification")
					{
						strTryNotification = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_is_allowHeuriticScan")
					{
						strAllowHeuriticScan = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_is_scanComputerStartUp")
					{
						strScanComputerStartUp = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_is_showToolTipStartup")
					{
						strShowToolTipStartup = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_is_showWardwizGadget")
					{
						strShowWardwizGadget = prop.second.get_value<std::string>();
					}
				}

				//update here task status
				CString csTaskID(strTaskID.c_str());
				json::value lValue;
				lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
				lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
				lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);
				lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

				std::vector<web::json::value> arraylValue;
				arraylValue.push_back(lValue);
				UpdateTaskStatus(arraylValue);


				if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwScheduledScan", 0x01))
				{
					AddLogEntry(L"### Error in Setting Registry dwScheduledScan in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
				}

				if (strlen(strAllowBlockAutorunExecution.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowBlockAutorunExecution.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwBlockAutorun", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwBlockAutorun in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strAllowExecuteBlock.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowExecuteBlock.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwExecuteBlock", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwExecuteBlock in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}
				if (strlen(strAllowProductUpdate.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowProductUpdate.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwAutoProductUpdate", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwAutoProductUpdate in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strAllowScanToExternalDevice.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowScanToExternalDevice.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwUsbScan", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwUsbScan in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strAllowThreatDetectionAction.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowThreatDetectionAction.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwQuarantineOption", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwQuarantineOption in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strAllowToPrepareCacheInBackground.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowToPrepareCacheInBackground.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwBackgroundCaching", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwBackgroundCaching in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strAllowToUseCaching.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowToUseCaching.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwCachingMethod", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwCachingMethod in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strAllowWriteBlock.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowWriteBlock.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwWriteBlock", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwWriteBlock in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strAutoUpdate.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAutoUpdate.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwAutoDefUpdate", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwAutoDefUpdate in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strDeleteOldReport.c_str()) != 0)
				{
					DWORD dwValue = atoi(strDeleteOldReport.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwDeleteOldReports", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwDeleteOldReports in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strEnableSound.c_str()) != 0)
				{
					DWORD dwValue = atoi(strEnableSound.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwEnableSound", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwEnableSound in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strTryNotification.c_str()) != 0)
				{
					DWORD dwValue = atoi(strTryNotification.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwShowTrayPopup", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwShowTrayPopup in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}
				if (strlen(strAllowHeuriticScan.c_str()) != 0)
				{
					DWORD dwValue = atoi(strAllowHeuriticScan.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwHeuScan", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwHeuScan in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strScanComputerStartUp.c_str()) != 0)
				{
					DWORD dwValue = atoi(strScanComputerStartUp.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwStartUpScan", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwStartUpScan in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strShowToolTipStartup.c_str()) != 0)
				{
					DWORD dwValue = atoi(strShowToolTipStartup.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwShowStartupTips", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwShowStartupTips in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
				}

				if (strlen(strShowWardwizGadget.c_str()) != 0)
				{
					DWORD dwValue = atoi(strShowWardwizGadget.c_str());
					if (!WriteRegistryEntryOfSettingsTab(L"SOFTWARE\\WardWiz", L"dwWidgetsUIState", dwValue))
					{
						AddLogEntry(L"### Error in Setting Registry dwWidgetsUIState in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
					}
					if (dwValue == 0x00)
					{
						if (SendData2Tray(RELOAD_WIDGETS_UI, 0x04, true) == 0x01)
						{
							AddLogEntry(L"### Exception in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
						}
					}
					else if (dwValue == 0x01)
					{
						if (SendData2Tray(RELOAD_WIDGETS_UI, 0x03, true) == 0x01)
						{
							AddLogEntry(L"### Exception in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
						}
					}
				}

				arraylValue.clear();
				lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
				lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
				lValue[L"Status"] = web::json::value::number(TASK_COMPLETED);
				lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

				arraylValue.push_back(lValue);
				UpdateTaskStatus(arraylValue);
			}
		}

		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sWWIZSETTINGS.DB", csWardWizModulePath);
		if (PathFileExists(csWardWizReportsPath))
		{
			SetFileAttributes(csWardWizReportsPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(csWardWizReportsPath);
		}

		found = str1.find(L"PolicyScheduleDetails");
		if (found != std::string::npos)
		{
			for (auto& challenge : pt.get_child("PolicyScheduleDetails"))
			{
				std::string varSchedType = "0";
				std::string varLowBattery = "0";
				std::string varShutDown = "0";
				std::string varWakeUp = "0";
				std::string varSkipScan = "0";
				std::string varStartTime = "0";
				std::string bScanSunday = "0";
				std::string bScanMonday = "0";
				std::string bScanTuesday = "0";
				std::string bScanWednesday = "0";
				std::string bScanThursday = "0";
				std::string bScanFriday = "0";
				std::string bScanSaturday = "0";
				std::string varScanType = "0";
				std::string varCurrentDate = "0";

				for (auto& prop : challenge.second)
				{
					if (prop.first == "ScheduleType")
					{
						varSchedType = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isShowLowBatteryStatus")
					{
						varLowBattery = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isAllowShutDownAfterScan")
					{
						varShutDown = prop.second.get_value<std::string>();
					}
					else if (prop.first == "ScanType")
					{
						varWakeUp = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_IsShowPopUpOnMissedScan")
					{
						varSkipScan = prop.second.get_value<std::string>();
					}
					else if (prop.first == "StartTime")
					{
						varStartTime = prop.second.get_value<std::string>();
					}
					else if (prop.first == "SUN")
					{
						bScanSunday = prop.second.get_value<std::string>();
					}
					else if (prop.first == "MON")
					{
						bScanMonday = prop.second.get_value<std::string>();
					}
					else if (prop.first == "TUE")
					{
						bScanTuesday = prop.second.get_value<std::string>();
					}
					else if (prop.first == "WED")
					{
						bScanWednesday = prop.second.get_value<std::string>();
					}
					else if (prop.first == "THU")
					{
						bScanThursday = prop.second.get_value<std::string>();
					}
					else if (prop.first == "FRI")
					{
						bScanFriday = prop.second.get_value<std::string>();
					}
					else if (prop.first == "SAT")
					{
						bScanSaturday = prop.second.get_value<std::string>();
					}
					else if (prop.first == "ScanType")
					{
						varScanType = prop.second.get_value<std::string>();
					}
					else if (prop.first == "Date")
					{
						varCurrentDate = prop.second.get_value<std::string>();
					}
				}
				InsertIntoDB(varSchedType, varLowBattery, varShutDown, varWakeUp, varSkipScan, varStartTime, bScanSunday,
					bScanMonday, bScanTuesday, bScanWednesday,
					bScanThursday, bScanFriday, bScanSaturday, varScanType, varCurrentDate);
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::StartApplyPolicyTask", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : StartApplyConfigTask
Description    : Function to reconfigure settings
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 28 March 2018
***********************************************************************************************/
bool CWardwizClientAgent::StartApplyConfigTask(json::value& v)
{
	OutputDebugString(L">>> In CWardwizClientAgent::StartApplyConfigTask");
	try
	{
		utility::string_t str1 = v.to_string();
		std::string stringvalues = CW2A(str1.c_str());
		std::istringstream iss(stringvalues);

		if (_tcscmp(str1.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no StartApplyConfigTask available");
			return false;
		}

		// Read json.
		ptree pt;
		read_json(iss, pt);

		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sWWIZFEATURESLOCK.DB", csWardWizModulePath);
		if (PathFileExists(csWardWizReportsPath))
		{
			SetFileAttributes(csWardWizReportsPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(csWardWizReportsPath);
		}

		std::size_t found = str1.find(L"PolicyScheduleDetails");
		if (found != std::string::npos)
		{
			for (auto& challenge : pt.get_child("PolicyScheduleDetails"))
			{
				std::string strTaskID;
				std::string strTaskTypeID;

				CITinRegWrapper objReg;
				for (auto& prop : challenge.second)
				{
					if (prop.first == "TaskID")
					{
						strTaskID = prop.second.get_value<std::string>();
					}
					else if (prop.first == "TaskTypeID")
					{
						strTaskTypeID = prop.second.get_value<std::string>();
					}
				}

				CString csTaskID(strTaskID.c_str());
				json::value lValue;
				std::vector<web::json::value> arraylValue;
				lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
				lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
				lValue[L"Status"] = web::json::value::number(TASK_COMPLETED);
				lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

				arraylValue.push_back(lValue);
				UpdateTaskStatus(arraylValue);
			}
		}

		found = str1.find(L"PolicyScheduleDetails");
		if (found != std::string::npos)
		{
			for (auto& challenge : pt.get_child("PolicyScheduleDetails"))
			{
				std::string varQuickScanID = "1";
				std::string varFullScanID = "2";
				std::string varCustomScanID = "3";
				std::string varBootTimeScanID = "4";
				std::string varAntiRootKitScanID = "5";
				std::string varScheduledScanID = "6";
				std::string varDataEncryptionID = "7";
				std::string varRegistryOptimizerID = "8";
				std::string varRecoverFilesID = "9";
				std::string varAutorunScanID = "10";
				std::string varTempFileCleanerID = "11";
				std::string varUSBVaccinationID = "12";
				std::string varSettingsID = "13";
				std::string varUninstallID = "14";
				//Don't change the order of the features. 
				
				std::string bAntiMalware = "0";
				std::string bDataEncryption = "0";
				std::string bRegistryOptimizer = "0";
				std::string bRecoverFiles = "0";
				std::string bAutorunScan = "0";
				std::string bTempFileClean = "0";
				std::string bUSBVaccin = "0"; 
				std::string bSettings = "0"; 
				std::string bUninstall = "0";
				std::string varFeatureName = "0";
				std::string bEmailScan = "0";
				std::string varClientPassword = "0";
				std::string bConnectTo = "0";
				std::string bExchangeProtection = "0";
				std::string bMode = "0";
				std::string bPath = "0";
				std::string bPowerUser = "0";
				std::string bRelay = "0";
				std::string bScanBeforeInstall = "0";
				std::string varServerMachineIP = "0";
				std::string varServerMachineName = "0";
				std::string bIsSetPswd = "0";
				std::string bIsUseCustomInstall = "0";

				for (auto& prop : challenge.second)
				{
					if (prop.first == "Antimalware")
					{
						bAntiMalware = prop.second.get_value<std::string>();
						varFeatureName = "Quick Scan";
						InsertConfigIntoDB(varFeatureName, bAntiMalware, varQuickScanID);
						varFeatureName = "Full Scan";
						InsertConfigIntoDB(varFeatureName, bAntiMalware, varFullScanID);
						varFeatureName = "Custom Scan";
						InsertConfigIntoDB(varFeatureName, bAntiMalware, varCustomScanID);
						varFeatureName = "BootTime Scan";
						InsertConfigIntoDB(varFeatureName, bAntiMalware, varBootTimeScanID);
						varFeatureName = "AntiRootKit Scan";
						InsertConfigIntoDB(varFeatureName, bAntiMalware, varAntiRootKitScanID);
						varFeatureName = "Scheduled Scan";
						InsertConfigIntoDB(varFeatureName, bAntiMalware, varScheduledScanID);
					}
					else if (prop.first == "DataEncryption")
					{
						bDataEncryption = prop.second.get_value<std::string>();
						varFeatureName = "Data Encryption";
						InsertConfigIntoDB(varFeatureName, bDataEncryption, varDataEncryptionID);
					}
					else if (prop.first == "RegistryOptimizer")
					{
						bRegistryOptimizer = prop.second.get_value<std::string>();
						varFeatureName = "Registry Optimizer";
						InsertConfigIntoDB(varFeatureName, bRegistryOptimizer, varRegistryOptimizerID);
					}
					else if (prop.first == "RecoverFiles")
					{
						bRecoverFiles = prop.second.get_value<std::string>();
						varFeatureName = "Recover Files";
						InsertConfigIntoDB(varFeatureName, bRecoverFiles, varRecoverFilesID);
					}
					else if (prop.first == "AutorunScan")
					{
						bAutorunScan = prop.second.get_value<std::string>();
						varFeatureName = "Autorun Scan";
						InsertConfigIntoDB(varFeatureName, bAutorunScan, varAutorunScanID);
					}
					else if (prop.first == "TempFileCleaner")
					{
						bTempFileClean = prop.second.get_value<std::string>();
						varFeatureName = "Temp File Cleaner";
						InsertConfigIntoDB(varFeatureName, bTempFileClean, varTempFileCleanerID);
					}
					else if (prop.first == "USBVaccination")
					{
						bUSBVaccin = prop.second.get_value<std::string>();
						varFeatureName = "USB Vaccination";
						InsertConfigIntoDB(varFeatureName, bUSBVaccin, varUSBVaccinationID);
					}
					else if (prop.first == "Settings")
					{
						bSettings = prop.second.get_value<std::string>();
						varFeatureName = "Settings";
						InsertConfigIntoDB(varFeatureName, bSettings, varSettingsID);
					}
					else if (prop.first == "Uninstall")
					{
						bUninstall = prop.second.get_value<std::string>();
						varFeatureName = "Uninstall";
						InsertConfigIntoDB(varFeatureName, bUninstall, varUninstallID);
					}
					else if (prop.first == "ClientPassword")
					{
						varClientPassword = prop.second.get_value<std::string>();
						InsertPaswdConfigIntoDB(varClientPassword);
					}
					else if (prop.first == "ConnectTo")
					{
						bConnectTo = prop.second.get_value<std::string>();
					}
					else if (prop.first == "EmailScan")
					{
						bEmailScan = prop.second.get_value<std::string>();
					}
					else if (prop.first == "ExchangeProtection")
					{
						bExchangeProtection = prop.second.get_value<std::string>();
					}
					else if (prop.first == "Mode")
					{
						bMode = prop.second.get_value<std::string>();
					}
					else if (prop.first == "Path")
					{
						bPath = prop.second.get_value<std::string>();
					}
					else if (prop.first == "PowerUser")
					{
						bRelay = prop.second.get_value<std::string>();
					}
					else if (prop.first == "Relay")
					{
						bRelay = prop.second.get_value<std::string>();
					}
					else if (prop.first == "ScanBeforeInstalation")
					{
						bScanBeforeInstall = prop.second.get_value<std::string>();
					}
					else if (prop.first == "ServerMchineIP")
					{
						varServerMachineIP = prop.second.get_value<std::string>();
					}
					else if (prop.first == "ServerMchineName")
					{
						varServerMachineName = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isSetPassword")
					{
						bIsSetPswd = prop.second.get_value<std::string>();
					}
					else if (prop.first == "_isUseCustomInstallationPath")
					{
						bIsUseCustomInstall = prop.second.get_value<std::string>();
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::StartApplyConfigTask", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::StartNetworkDignoseTask(json::value& v)
{
	OutputDebugString(L">>> In CWardwizClientAgent::StartNetworkDignoseTask");
	try
	{
		utility::string_t str1 = v.to_string();
		std::string stringvalues = CW2A(str1.c_str());
		std::istringstream iss(stringvalues);

		if (_tcscmp(str1.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no StartNetworkDignoseTask available");
			return false;
		}

		// Read json.
		ptree pt;
		read_json(iss, pt);

		std::string strIsDeepScan;
		for (auto& challenge : pt.get_child("Table"))
		{
			std::string strStartIP;
			std::string strEndIP;
			std::string strTaskID;
			std::string strTaskTypeID;
			for (auto& prop : challenge.second)
			{
				if (prop.first == "TaskID")
				{
					strTaskID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "StartIP")
				{
					strStartIP = prop.second.get_value<std::string>();
				}
				else if (prop.first == "EndIP")
				{
					strEndIP = prop.second.get_value<std::string>();
				}
				else if (prop.first == "TaskTypeID")
				{
					strTaskTypeID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "IsDeepScan")
				{
					strIsDeepScan = prop.second.get_value<std::string>();
				}
			}

			CString csTaskID(strTaskID.c_str());			
			CString csTaskTypeID(strTaskTypeID.c_str());

			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);
			lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);
			
			bool bISDeepScan = false;
			if (strcmp(strIsDeepScan.c_str(), "1") == 0)
			{
				bISDeepScan = true;
			}

			m_objNetworkTasks.ReInitializeVariables();
			vector<STRUCT_MACHINE_INFO> IPList = m_objNetworkTasks.DignoseNetwork(strStartIP, strEndIP, bISDeepScan);

			arraylValue.clear();
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_COMPLETED);
			lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::StartNetworkDignoseTask", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::InvokeTask(json::value& v)
{
	OutputDebugString(L">>> In CWardwizClientAgent::InvokeTask");
	try
	{
		utility::string_t str = v.to_string();
		if (_tcscmp(str.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no task available");
			return false;
		}

		std::istringstream iss(CW2A(str.c_str()).m_psz);

		// Read json.
		ptree pt;
		read_json(iss, pt);

		std::size_t found = str.find(L"Message");
		if (found != std::string::npos)
		{
			for (auto& challenge : pt.get_child("Message"))
			{
				for (auto& prop : challenge.second)
				{
					//Send here the result
				}
				return false;
			}
		}

		found = str.find(L"Table");
		if (found != std::string::npos)
		{
			for (auto& challenge : pt.get_child("Table"))
			{
				std::string strTaskID;
				std::string strTaskTypeID;
				for (auto& prop : challenge.second)
				{
					if (prop.first == "TaskID")
					{
						strTaskID = prop.second.get_value<std::string>();
					}
					else if (prop.first == "TaskTypeID")
					{
						strTaskTypeID = prop.second.get_value<std::string>();
					}
				}

				if (strTaskID.length() == 0 || strTaskTypeID.length() == 0)
				{
					OutputDebugString(L">>> No task assigned to this machine");
					continue;
				}

				utility::string_t str2 = CA2W(strTaskID.c_str());
				if (_tcscmp(str2.c_str(), L"null") == 0)
				{
					OutputDebugString(L">>> no task available");
					return false;
				}

				utility::string_t str3 = CA2W(strTaskTypeID.c_str());
				if (_tcscmp(str3.c_str(), L"null") == 0)
				{
					OutputDebugString(L">>> no task available");
					return false;
				}

				int iTaskID = _wtoi(str3.c_str());
				switch (iTaskID)
				{
				case TASK_SCAN:
					ScanTask(str2, str3);
					break;
				case TASK_CLIENT_INSTALL:
					InstallClientsTask(str2, str3);
					break;
				case TASK_UNINSTALL_CLIENT:
					UnInstallClientsTask(str2, str3);
					break;
				case TASK_UPDATE_CLIENT:
					UpdateClientMachine(str2, str3);
					break;
				case TASK_RECONFIGURE_CLIENT:
					ReconfigureFeature(str2, str3);
					break;
				case TASK_RESTART_MACHINE:
					RestartMachine(str2, str3);
					break;
				case TASK_NETWORK_DISCOVERY:
					InvokeNetworkTask(str2, str3);
					break;
				case APPLY_POLICY:
					ApplyPolicyTask(str2, str3);
					break;
				default:
					AddLogEntry(L"!!! Unhandled task in CWardwizClientAgent::InvokeTask", 0, 0, true, SECONDLEVEL);
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InvokeTask", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::GetApplyPolicyTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetApplyPolicyTaskDetails");

	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"TaskID"] = web::json::value::string(strTaskID);
	lValue[L"TaskTypeID"] = web::json::value::string(strTaskTypeID);
	lValue[L"Mode"] = web::json::value::number(1);
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	CString	csSubTaskAPI;
	csSubTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/SubTask", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csSubTaskAPI);
	http_client client(csSubTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			//start ApplyPolicy
			StartApplyPolicyTask(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			ss << e.what() << endl;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetApplyPolicyTaskDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetApplyPolicyTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}

/***********************************************************************************************
Function Name  : GetReconfiguraionDetails
Description    : Retrieves a JSON value from an HTTP request.
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 28 March 2018
***********************************************************************************************/
pplx::task<void> CWardwizClientAgent::GetReconfiguraionDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetReconfiguraionDetails");

	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"TaskID"] = web::json::value::string(strTaskID);
	lValue[L"TaskTypeID"] = web::json::value::string(strTaskTypeID);
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	CString	csSubTaskAPI;
	csSubTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/SubTask", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csSubTaskAPI);
	http_client client(csSubTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			//start ApplyPolicy
			StartApplyConfigTask(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			ss << e.what() << endl;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetReconfiguraionDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetReconfiguraionDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::GetNetworkTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetNetworkTaskDetails");

	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"TaskID"] = web::json::value::string(strTaskID);
	lValue[L"TaskTypeID"] = web::json::value::string(strTaskTypeID);
	lValue[L"Mode"] = web::json::value::number(1);
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	CString	csSubTaskAPI;
	csSubTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/SubTask", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csSubTaskAPI);
	http_client client(csSubTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			StartNetworkDignoseTask(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			ss << e.what() << endl;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetNetworkTaskDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetNetworkTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}

bool CWardwizClientAgent::UpdateTaskStatus(std::vector<web::json::value>  &v)
{
	/*__try
	{
	RestClient::init();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	*/
	__try
	{
		UpdateTaskStatusSEH(v);

		//wait for some time so that status should get updated on server.
		Sleep(1000);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CNetworkTasks::SendData", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

bool CWardwizClientAgent::UpdateTaskStatusSEH(std::vector<web::json::value> &v)
{
	bool bReturn = false;

	try
	{
		json::value JsonData;
		JsonData = web::json::value::array(v);

		utility::string_t Lreq = L"=" + JsonData.to_string();

		CString	csTaskStatusAPI;
		csTaskStatusAPI.Format(L"http://%s:%s/api/V001/MyTask/TaskStatus", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
		OutputDebugString(csTaskStatusAPI);
		http_client client(csTaskStatusAPI.GetBuffer());

		// Manually build up an HTTP request with header and request URI.
		http_request request(methods::POST);
		utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
		request.headers().add(L"Authorization", auth);
		request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
		request.set_body(Lreq);

		client.request(request).then([](http_response response)
		{
			if (response.status_code() == status_codes::OK)
			{
				response.extract_json().then([](json::value jsonValue)
				{
					try
					{
						utility::string_t str = jsonValue.to_string();
						if (_tcscmp(str.c_str(), L"null") == 0)
						{
							return false;
						}

						std::string stringvalues = CW2A(str.c_str());
						std::istringstream iss(stringvalues);
						// Read json.
						ptree pt;
						read_json(iss, pt);

						std::size_t found = str.find(L"Result");
						if (found != std::string::npos)
						{
							for (auto& challenge : pt.get_child("Result"))
							{
								for (auto& prop : challenge.second)
								{
									if (strcmp(prop.first.c_str(), "1") == 0)
									{
										return true;
									}
								}
								return false;
							}
						}
					}
					catch (const http_exception& e)
					{
						// Print error.
						wostringstream ss;
						ss << e.what() << endl;
						return false;
						AddLogEntry(L"### Exception in CWardwizClientAgent::GetTaskID, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
					}
					catch (...)
					{
						AddLogEntry(L"### Exception in CWardwizClientAgent::GetTaskID", 0, 0, true, SECONDLEVEL);
						return false;
					}
					return true;
				});
			}
			return false;
		});
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateTaskStatusSEH", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

pplx::task<void> CWardwizClientAgent::GetScanTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetScanTaskDetails");

	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"TaskID"] = web::json::value::string(strTaskID);
	lValue[L"TaskTypeID"] = web::json::value::string(strTaskTypeID);
	lValue[L"Mode"] = web::json::value::number(2);
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	CString	csSubTaskAPI;
	csSubTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/SubTask", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csSubTaskAPI);
	http_client client(csSubTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			//StartInstallationTask(v);
			StartScanTask(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetInstallationTaskDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetInstallationTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::GetInstallationTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetInstallationTaskDetails");

	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"TaskID"] = web::json::value::string(strTaskID);
	lValue[L"TaskTypeID"] = web::json::value::string(strTaskTypeID);
	lValue[L"Mode"] = web::json::value::number(1);
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	CString	csSubTaskAPI;
	csSubTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/SubTask", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csSubTaskAPI);
	http_client client(csSubTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			StartInstallationTask(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetInstallationTaskDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetInstallationTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}

bool CWardwizClientAgent::StartScanTask(json::value& v)
{
	if (v == NULL)
		return false;

	OutputDebugString(L">>> In CWardwizClientAgent::StartScanTask");
	try
	{
		utility::string_t str1 = v.to_string();
		std::string stringvalues = CW2A(str1.c_str());
		std::istringstream iss(stringvalues);
		if (_tcscmp(str1.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no StartScan task available");
			return false;
		}

		// Read json.
		ptree pt;
		read_json(iss, pt);

		for (auto& challenge : pt.get_child("Table"))
		{
			std::string strMachineIP;
			std::string strTaskID;
			std::string strTaskTypeID;
			std::string strDiscoverID;
			std::string strScanType;
			std::string strScanPath;
			for (auto& prop : challenge.second)
			{
				if (prop.first == "MachineIP")
				{
					strMachineIP = prop.second.get_value<std::string>();
				}
				else if (prop.first == "TaskID")
				{
					strTaskID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "TaskTypeID")
				{
					strTaskTypeID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "DiscoverID")
				{
					strDiscoverID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "ScanType")
				{
					strScanType = prop.second.get_value<std::string>();
				}
				else if (prop.first == "ScanPath")
				{
					strScanPath = prop.second.get_value<std::string>();
				}
			}

			CString csTaskID(strTaskID.c_str());
			CString csDiscoverID(strDiscoverID.c_str());
			CString csTaskTypeID(strTaskTypeID.c_str());
			CString csScanTypeID(strScanType.c_str());
			CString csScanPath(strScanPath.c_str());

			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			lValue[L"ScanType"] = web::json::value::string(csScanTypeID.GetBuffer());
			lValue[L"ScanPath"] = web::json::value::string(csScanPath.GetBuffer());
			lValue[L"Domain"] = web::json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);

			CString csIP(strMachineIP.c_str());
			int iScanTypeID = _wtoi(csScanTypeID.GetBuffer());

			m_objScanTask.ReInitializeVariables();
			CStringArray	csPaths;
			if (!m_objScanTask.StartScan(iScanTypeID, csTaskID, csPaths, csScanPath))
			{
				AddLogEntry(L"### Failed to Start Scan: MachineIP: [%s]", csIP, 0, true, SECONDLEVEL);

				json::value lValueFailed;
				lValueFailed[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
				lValueFailed[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
				lValueFailed[L"Status"] = web::json::value::number(TASK_FAILD);
				lValueFailed[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
				lValueFailed[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

				std::vector<web::json::value> arraylValueFailed;
				arraylValueFailed.push_back(lValueFailed);
				UpdateTaskStatus(arraylValueFailed);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::StartScanTask", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::StartUnInstallationTask(json::value& v)
{
	if (v == NULL)
		return false;

	OutputDebugString(L">>> In CWardwizClientAgent::StartUnInstallationTask");
	try
	{
		utility::string_t str1 = v.to_string();
		std::string stringvalues = CW2A(str1.c_str());
		std::istringstream iss(stringvalues);
		if (_tcscmp(str1.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no Installation task available");
			return false;
		}

		// Read json.
		ptree pt;
		read_json(iss, pt);

		for (auto& challenge : pt.get_child("Table"))
		{
			std::string strMachineIP;
			std::string strTaskID;
			std::string strTaskTypeID;
			std::string strIskeepQurantinedFileID;
			std::string strKeepUserDefineSettingsID;
			std::string strMachineID;
			for (auto& prop : challenge.second)
			{
				if (prop.first == "TaskID")
				{
					strTaskID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "TaskTypeID")
				{
					strTaskTypeID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "MachineIP")
				{
					strMachineIP = prop.second.get_value<std::string>();
				}
				else if (prop.first == "_IskeepQurantinedFile")
				{
					strIskeepQurantinedFileID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "_IsKeepUserDefinedSettings")
				{
					strKeepUserDefineSettingsID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "MachineID")
				{
					strMachineID = prop.second.get_value<std::string>();
				}
			}

			CString csTaskID(strTaskID.c_str());
			CString csTaskTypeID(strTaskTypeID.c_str());

			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);

			CString csIP(strMachineIP.c_str());

			m_objUnInstallClient.ReInitializeVariables();
			if (!m_objUnInstallClient.UnInstallClientSetup(strKeepUserDefineSettingsID, strIskeepQurantinedFileID))
			{
				AddLogEntry(L"### Failed to Install Client: MachineIP: [%s]", csIP, 0, true, SECONDLEVEL);
				json::value lValueFailed;
				lValueFailed[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
				lValueFailed[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
				lValueFailed[L"Status"] = web::json::value::number(TASK_FAILD);
				lValueFailed[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
				lValueFailed[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

				std::vector<web::json::value> arraylValueFailed;
				arraylValueFailed.push_back(lValueFailed);
				UpdateTaskStatus(arraylValueFailed);
				return false;
			}

			json::value lValueSuccess;
			lValueSuccess[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValueSuccess[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValueSuccess[L"Status"] = web::json::value::number(TASK_COMPLETED);
			lValueSuccess[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			lValueSuccess[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());
			
			std::vector<web::json::value> arraylValueSuccess;
			arraylValueSuccess.push_back(lValueSuccess);
			UpdateTaskStatus(arraylValueSuccess);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::StartUnInstallationTask", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::StartInstallationTask(json::value& v)
{
	if (v == NULL)
		return false;

	OutputDebugString(L">>> In CWardwizClientAgent::StartInstallationTask");
	try
	{
		utility::string_t str1 = v.to_string();
		std::string stringvalues = CW2A(str1.c_str());
		std::istringstream iss(stringvalues);
		if (_tcscmp(str1.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no Installation task available");
			return false;
		}

		// Read json.
		ptree pt;
		read_json(iss, pt);

		for (auto& challenge : pt.get_child("Table1"))
		{
			std::string strMachineIP;
			std::string strTaskID;
			std::string strTaskTypeID;
			std::string strDiscoverID;
			for (auto& prop : challenge.second)
			{
				if (prop.first == "MachineIP")
				{
					strMachineIP = prop.second.get_value<std::string>();
				}
				else if (prop.first == "TaskID")
				{
					strTaskID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "TaskTypeID")
				{
					strTaskTypeID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "DiscoverID")
				{
					strDiscoverID = prop.second.get_value<std::string>();
				}
			}

			CString csTaskID(strTaskID.c_str());
			CString csDiscoverID(strDiscoverID.c_str());
			CString csTaskTypeID(strTaskTypeID.c_str());
			CString csMachineIP(strMachineIP.c_str());
			
			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(csMachineIP.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);

			std::string strUserName;
			std::string strPassword;
			for (auto& challenge : pt.get_child("Table"))
			{
				std::string strIP;
				for (auto& prop : challenge.second)
				{
					if (prop.first == "User_Name")
					{
						strUserName = prop.second.get_value<std::string>();
					}
					else if (prop.first == "PassWord")
					{
						strPassword = prop.second.get_value<std::string>();
					}
				}

				CString csIP(strMachineIP.c_str());
				CString csUserName(strUserName.c_str());
				CString csPassword(strPassword.c_str());

				std::wstring wstrMachineIP = csMachineIP;
				std::wstring wstrUserName = csUserName;
				std::wstring wstrPassword = csPassword;
				std::wstring wstrTaskID = csTaskID;
				std::wstring wstrTaskTypeID = csTaskTypeID;

				InstallClientSetup(wstrTaskID, wstrTaskTypeID, wstrMachineIP, wstrUserName, wstrPassword).then([](int value)
				{
					/*if (value != 0x00)
					{
						return false;
					}*/
				});
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::StartInstallationTask", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::ScanTask(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	try
	{
		if (strTaskID.length() == 0 || strTaskTypeID.length() == 0)
			return false;

		OutputDebugString(L">>> In CWardwizClientAgent::ScanTask");
		//Send both TaskID and TaskTypeID
		GetScanTaskDetails(strTaskID.c_str(), strTaskTypeID.c_str());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::NetworkTasksThread", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::UnInstallClientsTask(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	try
	{
		if (strTaskID.length() == 0 || strTaskTypeID.length() == 0)
			return false;

		OutputDebugString(L">>> In CWardwizClientAgent::UnInstallClientsTask");
		//Send both TaskID and TaskTypeID
		GetUnInstallationTaskDetails(strTaskID.c_str(), strTaskTypeID.c_str());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::NetworkTasksThread", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::InstallClientsTask(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	try
	{
		if (strTaskID.length() == 0 || strTaskTypeID.length() == 0)
			return false;

		OutputDebugString(L">>> In CWardwizClientAgent::NetworkTasksThread");
		//Send both TaskID and TaskTypeID
		GetInstallationTaskDetails(strTaskID.c_str(), strTaskTypeID.c_str());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::NetworkTasksThread", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

void CWardwizClientAgent::GetBearerCode(bool bWait)
{
	try
	{
		if (bWait)
			GetBearerCodeTask().wait();
		else
			GetBearerCodeTask();
	}
	catch (exception const & e)
	{
		CString strError = CString(e.what());
		AddLogEntry(L"### Exception in CWardwizClientAgent::GetBearerCode : %s", strError, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::GetBearerCode", 0, 0, true, SECONDLEVEL);
	}
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::GetBearerCodeTask()
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetBearerCodeTask");

	utility::string_t Lreq = L"username=admin&password=admin&grant_type=password";

	CString	cstokenAPI;
	cstokenAPI.Format(L"http://%s:%s/wardwizEps/token", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(cstokenAPI);
	http_client client(cstokenAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			std::string stringvalues = CW2A(str.c_str());
			std::istringstream iss(stringvalues);

			if (_tcscmp(str.c_str(), L"null") != 0)
			{
				// Read json.
				ptree pt;
				read_json(iss, pt);

				for (auto it : pt)
				{
					std::string strBearer;
					if (it.first == "access_token")
					{
						strBearer = it.second.get_value<std::string>();
						OutputDebugString(CString(strBearer.c_str()));

						CString csBearer(strBearer.c_str());
						if (!CWWizSettings::SetBearerCode(csBearer))
						{
							AddLogEntry(L"### Failed to SetBearerCode, code :[%s]", str.c_str(), 0, true, SECONDLEVEL);
						}
					}
				}
			}
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetInstallationTaskDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetBearerCodeTask", 0, 0, true, SECONDLEVEL);
		}
	});
}


/***************************************************************************************************
*  Function Name  : OnDataReceiveCallBack()
*  Description    : Recieves pipe data from UI.
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0056
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void CWardwizClientAgent::OnDataReceiveCallBack(LPVOID lpParam)
{
	DWORD dwReply = 0;
	__try
	{
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		if (!lpSpyData)
		{
			return;
		}
		switch (lpSpyData->iMessageInfo)
		{
		case SHOW_VIRUSENTRY:
			dwReply = 0;
			if (m_pThis->SendVirusEntry2Server(lpSpyData))
			{
				dwReply = 1;
			}
			m_pThis->m_objCommSrv.SendResponse(lpSpyData);
			break;
		case SCAN_FINISHED:
			dwReply = 0;
			if (m_pThis->UpdateScanFinishedTask(lpSpyData))
			{
				dwReply = 1;
			}
			m_pThis->m_objCommSrv.SendResponse(lpSpyData);
			break;
		case UPDATE_TASK_STATUS:
			dwReply = 0;
			if (m_pThis->UpdateTaskStatus(lpSpyData))
			{
				dwReply = 1;
			}
			m_pThis->m_objCommSrv.SendResponse(lpSpyData);
			break;
		case UPDATE_FINISHED:
			dwReply = 0;
			if (m_pThis->UpdateFinishedTask(lpSpyData))
			{
				dwReply = 1;
			}
			m_pThis->m_objCommSrv.SendResponse(lpSpyData);
			break;
		case SEND_ERROR_LOG:
			dwReply = 0;
			if (m_pThis->AddClientErrorServerLog(lpSpyData))
			{
				dwReply = 1;
			}
			m_pThis->m_objCommSrv.SendResponse(lpSpyData);
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WrdWizComSrv OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::GetUnInstallationTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetUnInstallationTaskDetails");

	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"TaskID"] = web::json::value::string(strTaskID);
	lValue[L"TaskTypeID"] = web::json::value::string(strTaskTypeID);
	lValue[L"Mode"] = web::json::value::number(1);
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	CString	csSubTaskAPI;
	csSubTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/SubTask", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csSubTaskAPI);
	http_client client(csSubTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			StartUnInstallationTask(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetUnInstallationTaskDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetUnInstallationTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}

pplx::task<void> CWardwizClientAgent::UpdateClientMachine(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetRestartMachineTaskDetails");
	
	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"TaskID"] = web::json::value::string(strTaskID);
	lValue[L"TaskTypeID"] = web::json::value::string(strTaskTypeID);
	lValue[L"Mode"] = web::json::value::number(1);
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	CString	csSubTaskAPI;
	csSubTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/SubTask", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csSubTaskAPI);
	http_client client(csSubTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			UpdateMachine(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetRestartMachineTaskDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetRestartMachineTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::GetRestartMachineTaskDetails(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetRestartMachineTaskDetails");

	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"TaskID"] = web::json::value::string(strTaskID);
	lValue[L"TaskTypeID"] = web::json::value::string(strTaskTypeID);
	lValue[L"Mode"] = web::json::value::number(1);
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	CString	csSubTaskAPI;
	csSubTaskAPI.Format(L"http://%s:%s/api/V001/MyTask/SubTask", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csSubTaskAPI);
	http_client client(csSubTaskAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			RestartMachine(v);
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetRestartMachineTaskDetails, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetRestartMachineTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}


bool CWardwizClientAgent::UpdateMachine(json::value& v)
{
	if (v == NULL)
		return false;

	OutputDebugString(L">>> In CWardwizClientAgent::UpdateMachine");
	try
	{
		utility::string_t str1 = v.to_string();
		std::string stringvalues = CW2A(str1.c_str());
		std::istringstream iss(stringvalues);
		if (_tcscmp(str1.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no UpdateMachine task available");
			return false;
		}

		// Read json.
		ptree pt;
		read_json(iss, pt);

		for (auto& challenge : pt.get_child("Table"))
		{
			std::string strMachineIP;
			std::string strTaskID;
			std::string strTaskTypeID;
			std::string strIskeepQurantinedFileID;
			std::string strKeepUserDefineSettingsID;
			std::string strMachineID;
			for (auto& prop : challenge.second)
			{
				if (prop.first == "TaskID")
				{
					strTaskID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "TaskTypeID")
				{
					strTaskTypeID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "MachineID")
				{
					strMachineID = prop.second.get_value<std::string>();
				}
			}

			CString csTaskID(strTaskID.c_str());
			CString csTaskTypeID(strTaskTypeID.c_str());

			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);

			StartALUpdateUsingAlupdateService(csTaskID);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateMachine", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::RestartMachine(json::value& v)
{
	if (v == NULL)
		return false;

	OutputDebugString(L">>> In CWardwizClientAgent::RestartMachine");
	try
	{
		utility::string_t str1 = v.to_string();
		std::string stringvalues = CW2A(str1.c_str());
		std::istringstream iss(stringvalues);
		if (_tcscmp(str1.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no RestartMachine task available");
			return false;
		}

		// Read json.
		ptree pt;
		read_json(iss, pt);

		for (auto& challenge : pt.get_child("Table"))
		{
			std::string strMachineIP;
			std::string strTaskID;
			std::string strTaskTypeID;
			std::string strIskeepQurantinedFileID;
			std::string strKeepUserDefineSettingsID;
			std::string strMachineID;
			for (auto& prop : challenge.second)
			{
				if (prop.first == "TaskID")
				{
					strTaskID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "TaskTypeID")
				{
					strTaskTypeID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "MachineID")
				{
					strMachineID = prop.second.get_value<std::string>();
				}
			}

			CString csTaskID(strTaskID.c_str());
			CString csTaskTypeID(strTaskTypeID.c_str());

			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_COMPLETED);
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);

			if (SendData2Tray(SHOW_PASSWORD_WIND, 0x00, true) == 0x01)
			{
				AddLogEntry(L"### Password is correct", 0, 0, true, ZEROLEVEL);
				return false;
			}

			CEnumProcess	objEnumProc;
			objEnumProc.RebootSystem(0);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::RestartMachine", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardwizClientAgent::RestartMachine(utility::string_t strTaskID, utility::string_t strTaskTypeID)
{
	try
	{
		if (strTaskID.length() == 0 || strTaskTypeID.length() == 0)
			return false;

		OutputDebugString(L">>> In CWardwizClientAgent::RestartMachine");
		//Send both TaskID and TaskTypeID
		GetRestartMachineTaskDetails(strTaskID.c_str(), strTaskTypeID.c_str());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::RestartMachine", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

pplx::task<void> CWardwizClientAgent::UpdateMachineInfoTask()
{
	OutputDebugString(L">>> In CWardwizClientAgent::UpdateMachineInfoTask");

	std::vector<web::json::value> arrayMachines;
	web::json::value JsonData;

	JsonData[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	JsonData[L"MachineName"] = json::value::string(CWWizSettings::WWizGetComputerName().GetBuffer());
	JsonData[L"OS"] = json::value::string(CWWizSettings::WWizGetOSDetails().GetBuffer());
	JsonData[L"IP"] = json::value::string(CWWizSettings::WWizGetIPAddress().GetBuffer());
	JsonData[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());
	arrayMachines.push_back(JsonData);

	json::value lValue;
	lValue = web::json::value::array(arrayMachines);

	utility::string_t Lreq = L"=" + lValue.to_string();

	CString	csUpdateMachineInfoAPI;
	csUpdateMachineInfoAPI.Format(L"http://%s:%s/api/V001/MachineReg/UpdateMachineInfo", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csUpdateMachineInfoAPI);
	http_client client(csUpdateMachineInfoAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			ss << e.what() << endl;
			AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateMachineInfoTask, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateMachineInfoTask", 0, 0, true, SECONDLEVEL);
		}
	});
}

bool CWardwizClientAgent::SendVirusEntry2Server(LPISPY_PIPE_DATA lpSpyData)
{
	bool bReturn = false;
	try
	{
		CString csTaskID(lpSpyData->szFirstParam); //First Param as taskID
		CString csThreatName(lpSpyData->szSecondParam); //Second Param as Threat Name
		CString csThreatPath(lpSpyData->szThirdParam); //Second Param as Threat Path
		
		//SendVirusEntry2Server(csTaskID, csThreatName, csThreatPath, lpSpyData->dwSecondValue);

		CString csThreatStatus = lpSpyData->dwSecondValue ? _T("1") : _T("0"); //check here other action status and send to server
		CString csDateTime = CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S"));

		std::vector<web::json::value> arrayMachines;
		web::json::value JsonData;

		CString csMachineID = CWWizSettings::GetMachineID();
		JsonData[L"MachineID"] = json::value::string(csMachineID.GetBuffer());
		JsonData[L"MachineName"] = json::value::string(CWWizSettings::WWizGetComputerName().GetBuffer());
		JsonData[L"IP"] = json::value::string(CWWizSettings::WWizGetIPAddress().GetBuffer());
		JsonData[L"TaskID"] = json::value::string(csTaskID.GetBuffer());
		JsonData[L"ThreatName"] = json::value::string(csThreatName.GetBuffer());
		JsonData[L"ThreatPath"] = json::value::string(csThreatPath.GetBuffer());
		JsonData[L"ThreatStatus"] = json::value::string(csThreatStatus.GetBuffer());
		JsonData[L"DateTime"] = json::value::string(csDateTime.GetBuffer());
		JsonData[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());
		arrayMachines.push_back(JsonData);

		json::value lValue;
		lValue = web::json::value::array(arrayMachines);

		utility::string_t Lreq = L"=" + lValue.to_string();

		CString	csUploadQuarantineInfoAPI;
		csUploadQuarantineInfoAPI.Format(L"http://%s:%s/api/V001/Quarantine/UploadQuarantineInfo", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
		OutputDebugString(csUploadQuarantineInfoAPI);
		http_client client(csUploadQuarantineInfoAPI.GetBuffer());

		// Manually build up an HTTP request with header and request URI.
		http_request request(methods::POST);
		utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
		request.headers().add(L"Authorization", auth);
		request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
		request.set_body(Lreq);

		client.request(request).then([](http_response response)
		{
			if (response.status_code() == status_codes::OK)
			{
				response.extract_json().then([](json::value jsonValue)
				{
					try
					{
						utility::string_t str = jsonValue.to_string();
						if (_tcscmp(str.c_str(), L"null") == 0)
						{
							return false;
						}

						std::string stringvalues = CW2A(str.c_str());
						std::istringstream iss(stringvalues);
						// Read json.
						ptree pt;
						read_json(iss, pt);

						std::size_t found = str.find(L"Result");
						if (found != std::string::npos)
						{
							for (auto& challenge : pt.get_child("Result"))
							{
								for (auto& prop : challenge.second)
								{
									if (strcmp(prop.first.c_str(), "1") == 0)
									{
										return true;
									}
								}
								return false;
							}
						}
					}
					catch (const http_exception& e)
					{
						// Print error.
						wostringstream ss;
						ss << e.what() << endl;
						return false;
						AddLogEntry(L"### Exception in CWardwizClientAgent::SendVirusEntry2Server, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
					}
					catch (...)
					{
						AddLogEntry(L"### Exception in CWardwizClientAgent::SendVirusEntry2Server", 0, 0, true, SECONDLEVEL);
						return false;
					}
					return true;
				});
			}
			return false;
		});
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::SendVirusEntry2Server", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CWardwizClientAgent::AddClientErrorServerLog(LPISPY_PIPE_DATA lpSpyData)
{
	bool bReturn = false;
	try
	{
		CString csTaskID(lpSpyData->szFirstParam); //First Param as taskID
		CString csRemoteMachineIP(lpSpyData->szSecondParam); //Second Param as taskTypeID
		CString csMessage(lpSpyData->szThirdParam); //Third Param as MachineIP
		AddErrorServerLog(csTaskID.GetBuffer(), csRemoteMachineIP.GetBuffer(), csMessage.GetBuffer());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateScanFinishedTask", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CWardwizClientAgent::UpdateTaskStatus(LPISPY_PIPE_DATA lpSpyData)
{
	bool bReturn = false;
	try
	{
		CString csTaskID(lpSpyData->szFirstParam); //First Param as taskID
		CString csTaskTypeID(lpSpyData->szSecondParam); //Second Param as taskTypeID
		CString csMachineIP(lpSpyData->szThirdParam); //Third Param as MachineIP

		json::value lValue;
		CString csMachineID = CWWizSettings::GetMachineID();

		if (csTaskTypeID == L"3")
		{
			lValue[L"MachineID"] = web::json::value::string(csMachineIP.GetBuffer());
		}
		else
		{
			lValue[L"MachineID"] = web::json::value::string(csMachineID.GetBuffer());
		}
		lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
		lValue[L"Status"] = web::json::value::number((uint64_t)lpSpyData->dwValue);
		lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
		lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

		std::vector<web::json::value> arraylValue;
		arraylValue.push_back(lValue);
		UpdateTaskStatus(arraylValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateScanFinishedTask", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CWardwizClientAgent::UpdateScanFinishedTask(LPISPY_PIPE_DATA lpSpyData)
{
	bool bReturn = false;
	try
	{
		CString csTaskID(lpSpyData->szFirstParam); //First Param as taskID
		json::value lValue;
		CString csMachineID = CWWizSettings::GetMachineID();
		lValue[L"MachineID"] = web::json::value::string(csMachineID.GetBuffer());
		lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
		lValue[L"Status"] = web::json::value::number(TASK_COMPLETED);
		lValue[L"TaskTypeID"] = web::json::value::string(L"1");
		lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

		std::vector<web::json::value> arraylValue;
		arraylValue.push_back(lValue);
		UpdateTaskStatus(arraylValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateScanFinishedTask", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CWardwizClientAgent::UpdateFinishedTask(LPISPY_PIPE_DATA lpSpyData)
{
	bool bReturn = false;

	try
	{
		CString csTaskID(lpSpyData->szFirstParam); //First Param as taskID
		CString csUpdtStatusMess;
		DWORD dwUpdtStatus = (lpSpyData->dwSecondValue);

		bool bupdateSuccess = false;
		switch (dwUpdtStatus)
		{
		case 0:
			csUpdtStatusMess = "Product updated successfully.";
			bupdateSuccess = true;
			break;
		case 1:
			csUpdtStatusMess = "Product up-to-date.";
			bupdateSuccess = true;
			break;
		case 2:
			csUpdtStatusMess = "Failed to update product. Please check internet connection.";
			break;
		case 3:
			csUpdtStatusMess = "Failed to update product. Please check internet connection.";
			break;
		default:
			csUpdtStatusMess = "Failed to update product.";
		}

		json::value lValue;
		CString csMachineID = CWWizSettings::GetMachineID();
		lValue[L"MachineID"] = web::json::value::string(csMachineID.GetBuffer());
		lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());

		if (bupdateSuccess)
			lValue[L"Status"] = web::json::value::number(TASK_COMPLETED);
		else
			lValue[L"Status"] = web::json::value::number(TASK_FAILD);
		lValue[L"TaskTypeID"] = web::json::value::string(L"6"); //update id 6

		lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

		std::vector<web::json::value> arraylValue;
		arraylValue.push_back(lValue);
		UpdateTaskStatus(arraylValue);

		CString csRemoteMachineIP(lpSpyData->szSecondParam); //Second Param as machine IP
		SendFailureLog(csTaskID.GetBuffer(), csMachineID.GetBuffer(), csUpdtStatusMess.GetBuffer());
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::UpdateFinishedTask", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::SendVirusEntry2Server(CString csTaskID, CString csThreatName, CString csThreatPath, DWORD dwAction)
{
	OutputDebugString(L">>> In CWardwizClientAgent::SendVirusEntry2Server");

	CString csThreatStatus = dwAction ? _T("1") : _T("0"); //check here other action status and send to server
	CString csDateTime = CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S"));

	std::vector<web::json::value> arrayMachines;
	web::json::value JsonData;

	CString csMachineID = CWWizSettings::GetMachineID();
	JsonData[L"MachineID"] = web::json::value::string(csMachineID.GetBuffer());
	JsonData[L"MachineName"] = web::json::value::string(CWWizSettings::WWizGetComputerName().GetBuffer());
	JsonData[L"IP"] = web::json::value::string(CWWizSettings::WWizGetIPAddress().GetBuffer());
	JsonData[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
	JsonData[L"ThreatName"] = web::json::value::string(csThreatName.GetBuffer());
	JsonData[L"ThreatPath"] = web::json::value::string(csThreatPath.GetBuffer());
	JsonData[L"ThreatStatus"] = web::json::value::string(csThreatStatus.GetBuffer());
	JsonData[L"DateTime"] = web::json::value::string(csDateTime.GetBuffer());
	JsonData[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());
	arrayMachines.push_back(JsonData);

	json::value lValue;
	lValue = web::json::value::array(arrayMachines);

	utility::string_t Lreq = L"=" + lValue.to_string();

	CString	csUploadQuarantineInfoAPI;
	csUploadQuarantineInfoAPI.Format(L"http://%s:%s/api/V001/Quarantine/UploadQuarantineInfo", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csUploadQuarantineInfoAPI);
	http_client client(csUploadQuarantineInfoAPI.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			ss << e.what() << endl;
			AddLogEntry(L"### Exception in CWardwizClientAgent::SendVirusEntry2Server, Error: %s", CA2W(e.what()), 0, true, ZEROLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::SendVirusEntry2Server", 0, 0, true, SECONDLEVEL);
		}
	});
}

/***************************************************************************************************
*  Function Name  : WriteRegistryEntryOfSettingsTab
*  Description    : this function is  called for Writing the Registry Key Value for Menu Items
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CWardwizClientAgent::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue)
{
	try
	{
		if (!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue, true))
		{
			AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
		}
		Sleep(20);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizSettings::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : this function is  called for setting the Registry Keys using the Services
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CWardwizClientAgent::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;
		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;

		CISpyCommunicator objCom(SERVICE_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizSettings::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : GetRecordsSEH
Description    : This function will fetch records from database and compare them
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 4 October 2017
***********************************************************************************************/
void CWardwizClientAgent::InsertIntoDB(std::string &varSchedType, std::string &varLowBattery, std::string &varShutDown, std::string &varWakeUp, std::string &varSkipScan, std::string &varStartTime, std::string &bScanSunday, 
	std::string &bScanMonday, std::string &bScanTuesday, std::string &bScanWednesday, std::string &bScanThursday, std::string &bScanFriday, 
	std::string &bScanSaturday, std::string &varScanType, std::string &varCurrentDate)
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sWWIZSETTINGS.DB", csWardWizModulePath);
		if (PathFileExists(csWardWizReportsPath))
		{
			SetFileAttributes(csWardWizReportsPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(csWardWizReportsPath);
		}

		CString csInsertQuery = _T("CREATE TABLE IF NOT EXISTS WWIZSCHEDSCANTIME(ID INTEGER PRIMARY KEY, INTSCHEDULEDTYPE INT, INTLOWBATTERY INT, INTSHUTDOWN INT, INTWAKEUP INT, INTSCANSKIP INT, SCHEDSCANTIME INT, BSUNDAY BOOL, BMONDAY BOOL, BTUESDAY BOOL, BWEDNESDAY BOOL, BTHURSDAY BOOL, BFRIDAY BOOL, BSATURDAY BOOL, INTSCANTYPE INT, SCHEDSCANDATE REAL, RESERVEDVAL1 INT, RESERVEDVAL2 INT, RESERVEDVAL3 INT)");
		CT2A ascii(csInsertQuery, CP_UTF8);
		InsertDataToTable(ascii.m_psz);

		CStringA csScheduleInsertQuery = "INSERT INTO WWIZSCHEDSCANTIME VALUES (null,";
		csScheduleInsertQuery.Format("INSERT INTO WWIZSCHEDSCANTIME(INTSCHEDULEDTYPE, INTLOWBATTERY, INTSHUTDOWN, INTWAKEUP, INTSCANSKIP, SCHEDSCANTIME, BSUNDAY, BMONDAY, BTUESDAY, BWEDNESDAY, BTHURSDAY, BFRIDAY, BSATURDAY, INTSCANTYPE, SCHEDSCANDATE) values('%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' , '%s' )", varSchedType.c_str(), varLowBattery.c_str(), varShutDown.c_str(), varWakeUp.c_str(), varSkipScan.c_str(), varStartTime.c_str(),
			bScanSunday.c_str(), bScanMonday.c_str(), bScanTuesday.c_str(), bScanWednesday.c_str(), bScanThursday.c_str(), bScanFriday.c_str(), bScanSaturday.c_str(),
			varScanType.c_str(), varCurrentDate.c_str());
		
		InsertDataToTable(csScheduleInsertQuery);
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertDataToTable, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertDataToTable", 0, 0, true, SECONDLEVEL);
	}
}


/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
INT64 CWardwizClientAgent::InsertDataToTable(const char* szQuery)
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sWWIZSETTINGS.DB", csWardWizModulePath);

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		m_objSqlDb.SetDataBaseFilePath(dbPath.m_psz);

		m_objSqlDb.Open();

		int iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = m_objSqlDb.GetLastRowId();

		m_objSqlDb.Close();

		return iLastRowId;
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertDataToTable, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
	return 0;
}

concurrency::task<DWORD> CWardwizClientAgent::InstallClientSetup(utility::string_t strTaskID, utility::string_t strTaskTypeID, utility::string_t strMachineIP, utility::string_t strUserName,
	utility::string_t strPassword)
{
	return concurrency::create_task(
		[this, strTaskID, strTaskTypeID, strMachineIP, strUserName, strPassword]()
	{
		DWORD dwRet = 0x01;
		try
		{
			CString csIP(strMachineIP.c_str());
			CString csUserName(strUserName.c_str());
			CString csPassword(strPassword.c_str());
			CString csTaskID(strTaskID.c_str());
			CString csTaskTypeID(strTaskTypeID.c_str());

			m_objNetworkInstall.ReInitializeVariables();
			dwRet = m_objNetworkInstall.InstallClientSetup(csTaskID.GetBuffer(), csIP.GetBuffer(),
				csUserName.GetBuffer(), csPassword.GetBuffer());
			if (dwRet == 0x00)
			{
				json::value lValueFailed;
				lValueFailed[L"MachineID"] = web::json::value::string(csIP.GetBuffer());
				lValueFailed[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
				lValueFailed[L"Status"] = web::json::value::number(TASK_COMPLETED);
				lValueFailed[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
				lValueFailed[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

				std::vector<web::json::value> arraylValueFailed;
				arraylValueFailed.push_back(lValueFailed);
				UpdateTaskStatus(arraylValueFailed);
			}
			else
			{
				dwRet = m_objNetworkInstall.InstallClientSetupUsingTray(CString(strTaskID.c_str()).GetBuffer(), csIP.GetBuffer(), csUserName.GetBuffer(), csPassword.GetBuffer());
				if (dwRet != 0x00)
				{
					AddLogEntry(L"### Failed to Install Client: MachineIP: [%s]", csIP, 0, true, SECONDLEVEL);
					json::value lValueFailed;
					lValueFailed[L"MachineID"] = web::json::value::string(csIP.GetBuffer());
					lValueFailed[L"TaskID"] = web::json::value::string(CString(strTaskID.c_str()).GetBuffer());
					lValueFailed[L"Status"] = web::json::value::number(TASK_FAILD);
					lValueFailed[L"TaskTypeID"] = web::json::value::string(CString(strTaskTypeID.c_str()).GetBuffer());
					lValueFailed[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

					std::vector<web::json::value> arraylValueFailed;
					arraylValueFailed.push_back(lValueFailed);
					UpdateTaskStatus(arraylValueFailed);
				}
			}
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::InstallClientSetup", 0, 0, true, SECONDLEVEL);
			return dwRet;
		}
		return dwRet;
	});
}

/***************************************************************************************************
*  Function Name  : SendData2Tray
*  Description    : Function which send message data to Tray application.
*  Author Name    : Amol Jaware
*  Date           : 10th Nov,2017
****************************************************************************************************/
DWORD CWardwizClientAgent::SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait)
{
	DWORD dwRet = 0x00;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwValue;

		CISpyCommunicator objCom(TRAY_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardWizSettings::SendData2Tray", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CWardWizSettings::SendData2Tray", 0, 0, true, FIRSTLEVEL);
				return false;
			}
			dwRet = szPipeData.dwValue;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizSettings::SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***********************************************************************************************
Function Name  : InsertConfigIntoDB
Description    : This function will insert value into database
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 28 March 2018
***********************************************************************************************/
void CWardwizClientAgent::InsertConfigIntoDB(std::string &varFeatureName, std::string &bSetValue, std::string &intFeatureID)
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sWWIZFEATURESLOCK.DB", csWardWizModulePath);

		CString csInsertQuery = _T("CREATE TABLE IF NOT EXISTS WWIZFEATURESACCESS(ID INTEGER PRIMARY KEY, VARFEATURE NVARCHAR(128), BACCESSVAL INT, MODULE_ID INTEGER)");
		CT2A ascii(csInsertQuery, CP_UTF8);
		InsertDataToTableFeature(ascii.m_psz);

		CStringA csScheduleInsertQuery = "INSERT INTO WWIZFEATURESACCESS VALUES (null,";
		csScheduleInsertQuery.Format("INSERT INTO WWIZFEATURESACCESS(VARFEATURE, BACCESSVAL, MODULE_ID) values('%s' , '%s', '%s')", varFeatureName.c_str(), bSetValue.c_str(), intFeatureID.c_str());

		InsertDataToTableFeature(csScheduleInsertQuery);
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertConfigIntoDB, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertConfigIntoDB", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : InsertPaswdConfigIntoDB
Description    : This function will insert password value into database
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 28 March 2018
***********************************************************************************************/
void CWardwizClientAgent::InsertPaswdConfigIntoDB(std::string varClientPaswd)
{
	try
	{
		CString csInsertQuery = _T("CREATE TABLE IF NOT EXISTS WWIZFEATURESACCESSPWD(VARFEATUREPWD NVARCHAR(128))");
		CT2A ascii(csInsertQuery, CP_UTF8);
		InsertDataToTableFeature(ascii.m_psz);

		CStringA csScheduleInsertQuery = "INSERT INTO WWIZFEATURESACCESSPWD VALUES (null,";
		csScheduleInsertQuery.Format("INSERT INTO WWIZFEATURESACCESSPWD(VARFEATUREPWD) values('%s')", varClientPaswd.c_str());

		InsertDataToTableFeature(csScheduleInsertQuery);
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertPaswdConfigIntoDB, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertPaswdConfigIntoDB", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : InsertDataToTableFeature
Description    : This function will insert data into table
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 28 March 2018
***********************************************************************************************/
INT64 CWardwizClientAgent::InsertDataToTableFeature(const char* szQuery)
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sWWIZFEATURESLOCK.DB", csWardWizModulePath);

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);

		CWardWizSQLiteDatabase	objSqlDb;
		objSqlDb.SetDataBaseFilePath(dbPath.m_psz);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = objSqlDb.GetLastRowId();

		objSqlDb.Close();

		return iLastRowId;
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertDataToTableFeature, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertDataToTableFeature", 0, 0, true, SECONDLEVEL);
		return 0;
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : StartALUpdateUsingALupdateService
*  Description    : Send a request to start Autp live update to ALU service through named pipe
*  Author Name    : Nihar Deshpande
*  SR_NO		  :
*  Date           : 19-05-2016
*************************************************************************************************/
bool CWardwizClientAgent::StartALUpdateUsingAlupdateService(CString csTaskID)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = START_UPDATE;
		_tcscpy(szPipeData.szFirstParam, csTaskID);
		CISpyCommunicator objCom(AUTOUPDATESRV_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizClientAgent::StartALUpdateUsingALupdateService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))//jump to liveupdate file UpDateDowloadStatus
		{
			AddLogEntry(L"### Failed to ReadData in CWardwizClientAgent::StartALUpdateUsingAlupdateService", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		OutputDebugString(L">>> Out CWardwizClientAgent::StartALUpdateUsingALupdateService");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::StartALUpdateUsingALupdateService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : SendData2UI
Description    : Send Data to UI
Author Name    : Nitin Kolapkar
Date           : 25th June 2014
SR_NO		     : WRDWIZTRAY_0146
***********************************************************************************************/
bool CWardwizClientAgent::SendData2UI(int iMessageInfo, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));
	szPipeData.iMessageInfo = iMessageInfo;
	CISpyCommunicator objComUI(UI_SERVER);
	if (!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send Data in CWardwizClientAgent::SendData2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}