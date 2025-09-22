/****************************** Module Header ******************************\
* Module Name:  VirusDetectorService.cpp
* Project:      WWVirusDetectorService
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
#include "VirusDetectorService.h"
#include "ThreadPool.h"
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <string>
#include <atlstr.h>


using namespace std;
using namespace CurlCppWrapper;

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"
#pragma endregion
//#pragma comment(lib, "cpprest110_1_1")
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

CVirusDetectorService::CVirusDetectorService(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
}


CVirusDetectorService::~CVirusDetectorService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}


//
//   FUNCTION: CVirusDetectorService::OnStart(DWORD, LPWSTR *)
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
void CVirusDetectorService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
    // Log a service start message to the Application log.
    WriteEventLogEntry(L"WWWindowsService in OnStart", 
        EVENTLOG_INFORMATION_TYPE);

    // Queue the main service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&CVirusDetectorService::ServiceWorkerThread, this);
}

bool InvokeTmpCustomScan(CString csFileName)
{
	ATL::CString csaFileList;
	//csaFileList.Add(csFileName);

	DWORD dwStartTime = GetTickCount();
	CWardwizScanner	objScanner;
	objScanner.StartWardwizScanner("C:\\MyFile.txt");
	DWORD dwEndTime = GetTickCount();

	//m_dwTimeRequired = dwEndTime - dwStartTime;
	//m_iThreatsFoundCount = objScanner.m_iThreatsFoundCount;
	//m_iTotalFileCount = objScanner.m_iTotalFileCount;
	//m_csVirusName = objScanner.m_csVirusName;
	//m_bThreatDetected = objScanner.m_bThreatDetected;

	return true;
}

std::string ConvertATLCStringTOstring(const CString & strToConvert)
{
	std::string strConvertedString = "";
	CT2A temp(strToConvert);  // uses LPCTSTR conversion operator for CString and CT2A constructor
	strConvertedString = temp; // uses LPSTR conversion operator for CT2A
	return strConvertedString;
}

void CVirusDetectorService::RespondForCustomScan(const http_request& request, const status_code& status, const json::value& response)
{
	json::value objRetVal;
	objRetVal[U("response")] = response;

	time_t now = time(0);

	utility::stringstream_t strTotalFileCount;
	strTotalFileCount << m_iTotalFileCount;

	//objRetVal[U("Files Scanned")] = json::value::string(strTotalFileCount.str());

	utility::stringstream_t strThreatCount;
	if (m_iThreatsFoundCount <= 0)
		m_iThreatsFoundCount = 0;

	strThreatCount << m_iThreatsFoundCount;

	objRetVal[U("Threats Found")] = json::value::string(strThreatCount.str());

	utility::stringstream_t strScanTime;
	//DWORD dwTimeInSeconds = ((m_dwTimeRequired + 500) / 1000);
	strScanTime << m_dwTimeRequired;

	objRetVal[U("ScanTime")] = json::value::string(strScanTime.str());

	utility::stringstream_t strThreatDetectedStatus;

	if (m_bThreatDetected)
		strThreatDetectedStatus << json::value::string(U("True"));
	else
		strThreatDetectedStatus << json::value::string(U("False"));


	objRetVal[U("DetectedStatus")] = json::value::string(strThreatDetectedStatus.str());


	utility::stringstream_t strVirusName;

	if (m_csVirusName.IsEmpty())
		strVirusName << json::value::string(U("No Threat Detected"));
	else
		strVirusName << ConvertATLCStringTOstring(m_csVirusName).c_str();


	objRetVal[U("VirusName")] = json::value::string(strVirusName.str());

	request.reply(status, objRetVal);
}


bool CVirusDetectorService::InvokeCustomScan(CString csFileName)
{
	DWORD dwStartTime = GetTickCount();
	CWardwizScanner	objScanner;
	std::string strFileName = CT2A(csFileName);
	
	objScanner.StartWardwizScanner(strFileName);

	DWORD dwEndTime = GetTickCount();

	m_dwTimeRequired = dwEndTime - dwStartTime;
	m_iThreatsFoundCount = objScanner.m_iThreatsFoundCount;
	m_iTotalFileCount = objScanner.m_iTotalFileCount;
	m_csVirusName = objScanner.m_csVirusName;
	m_bThreatDetected = objScanner.m_bThreatDetected;

	return true;
}

/* handlers implementation */

void handle_get(http_request request)
{
	auto http_get_vars = uri::split_query(request.request_uri().query());

	auto atRequestReceived = http_get_vars.find(U("customscan"));
			
		if (atRequestReceived == end(http_get_vars))
			{
				auto err = U("Request received with get var \"request\" omitted from query.");
				request.reply(status_codes::OK, json::value::string(err));
				return;
			}
		else
			{
				auto request_name = atRequestReceived->second;
				request.reply(status_codes::OK, json::value::string(request_name));
				return;
			}
		
}

void handle_get_bkup(http_request request)
{
	auto http_get_vars = uri::split_query(request.request_uri().query());

	auto atRequestReceived = http_get_vars.find(U("customscan"));

	if (atRequestReceived == end(http_get_vars))
	{
		auto err = U("Request received with get var \"request\" omitted from query.");
		//Respond(request, status_codes::BadRequest, json::value::string(err));
		request.reply(status_codes::OK, json::value::string(err));
		return;
	}
	else
	{
		auto request_name = atRequestReceived->second;
		bool bScan = InvokeTmpCustomScan(request_name.c_str());
		//RespondForCustomScan(req, status_codes::OK, json::value::string(U(" ") + request_name));
		request.reply(status_codes::OK, json::value::string(request_name));
		return;
	}

}
bool CVirusDetectorService::DownloadFileFromURL(CString strFileURL, CString csBearer)
{
	CCurlWrapper objCurl;
	objCurl.InitCurl();
	bool bHttpsURL = false;

	CString csURLType = strFileURL.Left(5);
	if (csURLType.MakeLower() == L"https")
	{
		bHttpsURL = true;
	}

	bool bDownloadSuccess = true;
	//bDownloadSuccess= objCurl.DownloadFileHTTPS("https://cdn.fbsbx.com/v/t59.2708-21/16711363_1562743923766340_3807388679979139072_n.txt/dbversion-1.txt?oh=5b5fa15e653df2f5c629a8209859ab01&oe=58B90FB6 ", "C://MyFile.txt");
	std::string strURLToDownload = ConvertATLCStringTOstring(strFileURL);
	objCurl.strURLToDownload = strFileURL;
	objCurl.m_csBearer = csBearer;
	
	if (bHttpsURL)
	{
		bDownloadSuccess = objCurl.DownloadFileHTTPS(strURLToDownload, "C://dbversion-1.txt");
	}
	else
	{
		bDownloadSuccess = objCurl.DownloadFileHTTP(strURLToDownload, "C://dbversion-1.txt");
	}

	return bDownloadSuccess;
}

void CVirusDetectorService::InvokeListener()
{
	try
	{
		http_listener listener(L"http://172.168.1.164/api:8888");

		DWORD dwError = GetLastError();
		wcout << "After listener GetLastError returned : " << dwError << endl;

		listener.open().wait();

		listener.support(methods::GET, [this](http_request request) {
			auto http_get_vars = uri::split_query(request.request_uri().query());

			auto atRequestReceived = http_get_vars.find(U("customscan"));

			if (atRequestReceived == end(http_get_vars))
			{
				//Nowheck if this request is from our webpage,,
				auto atWebRequest = http_get_vars.find(U("ftpscan"));
				if (atWebRequest == end(http_get_vars))
				{
					auto err = U("Request received with get var \"request\" omitted from query.");
					request.reply(status_codes::OK, json::value::string(err));
					return;
				}
				else
				{
					auto request_name = atWebRequest->second;
					utility::string_t strFilePath = web::uri::decode(request_name);
					CString csFilePath(strFilePath.c_str());
					bool bScan = InvokeCustomScan(csFilePath);
					RespondForCustomScan(request, status_codes::OK, json::value::string(U(" ") + request_name));
					return;
				}
			}
			else
			{
				CString csBearere;
				auto atBearer = http_get_vars.find(U("bearer"));
				if (atBearer == end(http_get_vars))
				{
					auto bearerCode = atBearer->second;
					utility::string_t strFilePath = web::uri::decode(bearerCode);
					csBearere = strFilePath.c_str();
				}

				auto request_name = atRequestReceived->second;
				utility::string_t strFilePath = web::uri::decode(request_name);
				CString csFilePath(strFilePath.c_str());
				m_sFileURL = ConvertATLCStringTOstring(csFilePath);
	
				bool bDownload = DownloadFileFromURL(csFilePath, csBearere);

				if (bDownload)
					bool bScan = InvokeCustomScan(L"C://dbversion-1.txt");
				else
				{
					auto err = U("Requested request could not be processed due to internal download error!.");
					request.reply(status_codes::InternalError, json::value::string(err));
					return;
				}
				RespondForCustomScan(request, status_codes::OK, json::value::string(U(" ") + request_name));
				return;
			}
		});
		while (true) {
			::Sleep(2000);
		}
	}
	catch (exception const & e)
	{
		DWORD dwError = GetLastError();
		wcout << "GetLastError returned : " << dwError << endl;
		wcout << e.what() << endl;
		getchar();
	}

	return ;
}

//
//   FUNCTION: CVirusDetectorService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void CVirusDetectorService::ServiceWorkerThread(void)
{
    // Periodically check if the service is stopping.
    while (!m_fStopping)
    {
        // Perform main service function here...
		InvokeListener();
        ::Sleep(2000);  // Simulate some lengthy operations.
    }

    // Signal the stopped event.
    SetEvent(m_hStoppedEvent);
}


//
//   FUNCTION: CVirusDetectorService::OnStop(void)
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
void CVirusDetectorService::OnStop()
{
    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}