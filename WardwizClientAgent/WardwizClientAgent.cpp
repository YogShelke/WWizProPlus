/****************************** Module Header ******************************\
* Module Name:  WardwizClientAgent.cpp
* Project:      WardwizClientAgent
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* Defaultfunction: http://wssr0011/WardwizEPS/api/Test/Mock_response 
* \***************************************************************************/

#pragma region Includes
#ifdef __windows__
#undef __windows__
#endif
#include "stdafx.h"
#include "WardwizClientAgent.h"
#include "WardwizServiceInvoker.h"
#include "ThreadPool.h"
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using boost::property_tree::ptree;

#pragma comment(lib, "cpprest120_2_9")

#pragma endregion

// cpprest provides macros for all streams but std::clog in basic_types.h
#ifdef _UTF16_STRINGS
// On Windows, all strings are wide
#define uclog std::wclog
#else
// On POSIX platforms, all strings are narrow
#define uclog std::clog
#endif // endif _UTF16_STRINGS

#define		NO_TASK		L"0"

TCHAR g_strIPAddress[256] = L"http://";

int g_iProcessCount;
//int g_iThreatsFound;
//int g_iTotalFiles;

CWardwizClientAgent * CWardwizClientAgent::m_pThis = NULL;

CWardwizClientAgent::CWardwizClientAgent(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
,m_objCommSrv(EPS_CLIENT_AGENT, &OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA))
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
	m_objCommSrv.Run();
    // Queue the main service function for execution in a worker thread.
	CClientAgentThreadPool::QueueUserWorkItem(&CWardwizClientAgent::ServiceWorkerThread, this);
	CClientAgentThreadPool::QueueUserWorkItem(&CWardwizClientAgent::GetBearereCodeThread, this);
	//CClientAgentThreadPool::QueueUserWorkItem(&CWardwizClientAgent::CheckRegistationStatusThread, this);
	//CClientAgentThreadPool::QueueUserWorkItem(&CWardwizClientAgent::CheckInstallSetup, this);
}
//
//void handle_request(http_request request,function<void(json::value &, json::value::field_map &)> action)
//{
//	json::value::field_map answer;
//
//	request
//		.extract_json()
//		.then([&answer, &action](pplx::task<json::value> task) {
//		try
//		{
//			auto & jvalue = task.get();
//
//			if (!jvalue.is_null())
//			{
//				action(jvalue, answer);
//			}
//		}
//		catch (http_exception const & e)
//		{
//			wcout << e.what() << endl;
//		}
//	})
//		.wait();
//
//	request.reply(status_codes::OK, json::value::object(answer));
//}
//
std::string ConvertATLCStringTOstring(const CString & strToConvert)
{
	std::string strConvertedString = "";
	CT2A temp(strToConvert);  // uses LPCTSTR conversion operator for CString and CT2A constructor
	strConvertedString = temp; // uses LPSTR conversion operator for CT2A
	return strConvertedString;
}

void CWardwizClientAgent::RespondForCustomScan(const http_request& request, const status_code& status, const json::value& response)
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

void CWardwizClientAgent::Respond(const http_request& request, const status_code& status, const json::value& response) {
	json::value resp;
	resp[U("status")] = json::value::number(status);
	resp[U("response")] = response;

	time_t now = time(0);

	utility::stringstream_t procCount;
	procCount << m_iTotalFileCount;

	resp[U("Files Scanned")] = json::value::string(procCount.str());

	utility::stringstream_t ThreatCount;
	if (m_iThreatsFoundCount <= 0)
		m_iThreatsFoundCount = 0;

	ThreatCount << m_iThreatsFoundCount;
	resp[U("Threats Found")] = json::value::string(ThreatCount.str());

	// Pack in the current time for debugging purposes
	//time_t now = time(0);
	//utility::stringstream_t procCount;
	//procCount << g_iProcessCount;
	//resp[U("process count is:")] = json::value::string(procCount.str());
	//utility::stringstream_t FileCount;
	//FileCount << "50";
	//resp[U("File count on Cdrive is:")] = json::value::string(FileCount.str());
	request.reply(status, resp);
}

bool CWardwizClientAgent::InvokeSettingsUpdater()
{
	bool bRetval =  objServiceInvoker.InvokeSettingsUpdaterFromService();
	return bRetval;
}

bool CWardwizClientAgent::InvokeQuickScan()
{
	CStringArray csaProcList;
	bool bRetval = objServiceInvoker.InvokeScanFromService(csaProcList,QUICKSCAN);

	m_iThreatsFoundCount = objServiceInvoker.m_iThreatsFoundCount;
	m_iTotalFileCount = objServiceInvoker.m_iTotalFileCount;
	m_csVirusName = objServiceInvoker.m_csVirusName;
	m_bThreatDetected = objServiceInvoker.m_bThreatDetected;

	return bRetval;
}

bool CWardwizClientAgent::DownloadFileContent(http_request req, std::string strFileToDownload)
{
	Concurrency::streams::istream bodyStream = req.body();

	concurrency::streams::container_buffer<std::string> inStringBuffer;

	// Open file stream
	std::string strFileName = "myFileDownloaded.txt";
	std::string strFilePath = "C:\\Program Files\\WardWiz\\" + strFileToDownload;

	concurrency::streams::basic_ostream<unsigned char> fileStream = concurrency::streams::file_stream<unsigned char>::open_ostream(utility::conversions::to_string_t(strFilePath)).get();

	//	bodyStream.read(inStringBuffer,255).then([inStringBuffer,fileStream](size_t bytesRead)
	bodyStream.read_to_end(inStringBuffer).then([inStringBuffer, fileStream](size_t bytesRead)
	{
		const std::string &text = inStringBuffer.collection();
		fileStream.print(text);
	}).wait();
	fileStream.close();

	return true;
}

bool CWardwizClientAgent::DownloadBinaryFileContent(http_request req, std::string strFileToDownload)
{
	Concurrency::streams::istream bodyStream = req.body();

	concurrency::streams::container_buffer<std::vector<char>> inStringBuffer;
	
	
	//FILE *fw = fopen("C:\\Program Files\\WardWiz\\myDownloadedfile.exe", "wb");
	
	bodyStream.read_to_end(inStringBuffer).then([inStringBuffer](size_t bytesRead)
	{
		std::ofstream outputFile("C:\\Program Files\\WardWiz\\myDownloadedfile.exe", std::ios::binary);
		//outputFile.write(inStringBuffer.create_istream(), inStringBuffer.size()
	}).wait();


	//rec[256] = '\0';
	//int size = atoi(rec);

	//while (size > 0)
	//{
	//	char buffer[1030];

	//	if (size >= 1024)
	//	{
	//		fwrite(buffer, 1024, 1, fw);
	//	}
	//	else
	//	{
	//		buffer[size] = '\0';
	//		fwrite(buffer, size, 1, fw);
	//	}


	//	size -= 1024;

	//}

	//fclose(fw);



	//// Open file stream
	//std::string strFilePath = "C:\\Program Files\\WardWiz\\" + strFileToDownload;

	//concurrency::streams::ostream fileStream = concurrency::streams::file_stream<unsigned char>::open_ostream(utility::conversions::to_string_t(strFilePath)).get();
	//

	//bodyStream.read(inStringBuffer,1024).then([inStringBuffer, fileStream](size_t bytesRead)
	//{
	//	concurrency::streams::istream is = concurrency::streams::container_stream<std::vector<unsigned char>>::open_istream(std::move(inStringBuffer.collection()));
	//	fileStream.print(is);

	//	/*const std::string &text = inStringBuffer.collection();
	//	fileStream.print(text);*/


	//}).wait();
	//fileStream.close();
	
	//json::value jsonValue = req.body();
	//for (auto iter = jsonValue.as_object().cbegin(); iter != jsonValue.as_object().cend(); ++iter)
	//{
	//	const utility::string_t &strFieldName = iter->first;

	//	const json::value &propertyValue = iter->second;

	//	const utility::string_t &strFieldValue = propertyValue.as_string();

	//	MessageBox(NULL, strFieldValue.c_str(), strFieldName.c_str(), 0);
	//}
	//std::ifstream imageToConvert("D:\\Personal\\Pictures\\GayatriAfre.jpg", std::ifstream::binary);

	//std::vector<char> data((std::istreambuf_iterator<char>(imageToConvert)), std::istreambuf_iterator<char>());

	//std::string Code = base64_encode((unsigned char*)&data[0], (unsigned int)data.size());

	//std::string DeCodedStr = base64_encode((unsigned char*)&Code[0], (unsigned int)Code.size());
	//std::stringstream strm;

	//if(imageToConvert.is_open())
	//{
	//	strm << imageToConvert.rdbuf();
	//	imageToConvert.close();
	//}

	//ofstream imageToWrite;
	//imageToWrite.open("C:\\Program Files\\WardWiz\\Gayatri.jpg", ios::out | ios::binary| ios::app);

	//COPYSTREAM()
	//char* buffer = NULL;
	//buffer << strm;
	//imageToWrite.write(, strm.length());
	//imageToWrite.close();
	
	return true;
}

bool CWardwizClientAgent::InvokeCustomScan(CString csFileName)
{
	CStringArray csaProcList;
	csaProcList.Add(csFileName);

	DWORD dwStartTime = GetTickCount();
	
	objServiceInvoker.InvokeScanFromService(csaProcList,CUSTOMSCAN);

	DWORD dwEndTime = GetTickCount();

	m_dwTimeRequired = dwEndTime - dwStartTime;

	m_iThreatsFoundCount = objServiceInvoker.m_iThreatsFoundCount;
	m_iTotalFileCount = objServiceInvoker.m_iTotalFileCount;
	m_csVirusName = objServiceInvoker.m_csVirusName;
	m_bThreatDetected = objServiceInvoker.m_bThreatDetected;

	return true;
}
int CWardwizClientAgent::GetProcessCount()
{
	CStringArray csaProcList;
	objServiceInvoker.InvokeScanFromService(csaProcList,QUICKSCAN);

	objServiceInvoker.InvokeSettingsUpdaterFromService();

	int iProcCount = objServiceInvoker.GetRunningProcessList(csaProcList);
	m_iThreatsFoundCount = objServiceInvoker.m_iThreatsFoundCount;
	m_iTotalFileCount = objServiceInvoker.m_iTotalFileCount;

	return iProcCount;
}
DWORD SetUSBPolicy(BOOL bEnableDisable)
{
	DWORD dwError = 0;

	CWardwizServiceInvoker objServiceInvoker;
	dwError = objServiceInvoker.UpdateUSBPORTsPolicy(bEnableDisable);
	return dwError;
}

TCHAR* GetExePath() {
	TCHAR* buffer = new TCHAR[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return buffer;
}

TCHAR* ReadString(TCHAR* szSection, TCHAR* szKey, const TCHAR* szDefaultValue)
{
	TCHAR* szResult = new TCHAR[255];
	DWORD dwResult = 0;
	TCHAR* strPath = GetExePath();
	TCHAR* strIniFilePath = new TCHAR[512];

	wsprintf(strIniFilePath, L"%s.ini", strPath);

	dwResult = GetPrivateProfileString(szSection, szKey, szDefaultValue, szResult, 255, strIniFilePath);
	
	return szResult;
}

int GetLocalIPAddress()
{
	USES_CONVERSION;
	char ac[80];
	if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
		return 1;
	}

	struct hostent *phe = gethostbyname(ac);
	if (phe == 0) {
		return 1;
	}

	for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));

		_swprintf(g_strIPAddress, L"%s%s", g_strIPAddress, A2T(inet_ntoa(addr)));
	}

	return 0;
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
		AddLogEntry(L"### Exception in CGenXClientAgent::GetBearereCodeThread", 0, 0, true, SECONDLEVEL);
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
		CString strError = e.what();
		AddLogEntry(L"### Exception in CGenXClientAgent::GetBearereCodeThreadSEH!! : %s", strError, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::GetBearereCodeThreadSEH!! ", 0, 0, true, SECONDLEVEL);
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
	OutputDebugString(L">>> In CGenXClientAgent::ServiceWorkerThread");

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
		AddLogEntry(L"### Exception in CGenXClientAgent::ServiceWorkerThread", 0, 0, true, SECONDLEVEL);
	}
}

void CWardwizClientAgent::ServiceWorkerThreadSEH(void)
{
	try
	{
		OutputDebugString(L">>> Before IsWardWizRegistered");

		GetBearerCode(true);

		if (!m_objRegistration.IsWardWizRegistered())
		{
			OutputDebugString(L">>> Before CheckRegistrationStatus");
			if (!m_objRegistration.CheckRegistrationStatus())
			{
				OutputDebugString(L">>> Failed to register client");
				AddLogEntry(L"### Failed to register client", 0, 0, true, SECONDLEVEL);
				return;
			}
		}

		while (!m_fStopping)
		{
			FunctionGetTaskID();
			WaitForSingleObject(m_hStoppedEvent, 2 * 1000);
		}
	}
	catch (exception const & e)
	{
		CString strError = e.what();
		AddLogEntry(L"Exception Occured in Service Thread!! : %s", strError, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"Exception Occured in Service Thread!! ", 0, 0, true, SECONDLEVEL);
	}

	// Signal the stopped event.
	SetEvent(m_hStoppedEvent); 
}


void CWardwizClientAgent::FunctionGetTaskID(void)
{
	__try
	{
		FunctionGetTaskIDSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::FunctionGetTaskID", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CGenXClientAgent::FunctionGetTaskID", 0, 0, true, SECONDLEVEL);
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
	arrayMachines.push_back(JsonData);

	json::value lValue;
	lValue = web::json::value::array(arrayMachines);

	utility::string_t Lreq = L"=" + lValue.to_string();

	http_client client(L"http://wssr0011/WardwizEPS/api/MyTask/Task");

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
			AddLogEntry(L"### Exception in CGenXClientAgent::GetTaskID, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CGenXClientAgent::GetTaskID", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CGenXClientAgent::CheckInstallSetup", 0, 0, true, SECONDLEVEL);
	}
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
		AddLogEntry(L"### Exception in CGenXClientAgent::NetworkTasksThread", 0, 0, true, SECONDLEVEL);
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
			}

			CString csTaskID(strTaskID.c_str());

			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);

			m_objNetworkTasks.ReInitializeVariables();
			vector<STRUCT_MACHINE_INFO> IPList = m_objNetworkTasks.DignoseNetwork(strStartIP, strEndIP);

			arraylValue.clear();
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_COMPLETED);

			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::StartNetworkDignoseTask", 0, 0, true, SECONDLEVEL);
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
				case TASK_UPGRADE_CLIENT:
					break;
				case TASK_UNINSTALL_CLIENT:
					UnInstallClientsTask(str2, str3);
					break;
				case TASK_UPDATE_CLIENT:
					break;
				case TASK_RECONFIGURE_CLIENT:
					break;
				case TASK_RESTART_MACHINE:
					break;
				case TASK_NETWORK_DISCOVERY:
					InvokeNetworkTask(str2, str3);
					break;
				case TASK_UPDATE_SECURITY_SERVER:
					break;
				default:
					AddLogEntry(L"!!! Unhandled task in CGenXClientAgent::InvokeTask", 0, 0, true, SECONDLEVEL);
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::InvokeTask", 0, 0, true, SECONDLEVEL);
	}
	return true;
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

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	http_client client(L"http://wssr0011/WardwizEPS/api/MyTask/SubTask");

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
			AddLogEntry(L"### Exception in CGenXClientAgent::GetNetworkTaskDetails, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CGenXClientAgent::GetNetworkTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}

bool CWardwizClientAgent::UpdateTaskStatus(std::vector<web::json::value>  &v)
{
	__try
	{
		RestClient::init();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	__try
	{
		UpdateTaskStatusSEH(v);
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

		RestClient::init();

		// get a connection object
		RestClient::Connection* conn = new RestClient::Connection("http://wssr0011");

		// set headers
		RestClient::HeaderFields headers;
		headers["Content-Type"] = "application/x-www-form-urlencoded";
		conn->SetHeaders(headers);

		// append additional headers
		std::string stdAuth = "bearer ";
		stdAuth.append(CW2A(CWWizSettings::m_csBearerCode.GetBuffer()));
		conn->AppendHeader("Authorization", stdAuth);

		RestClient::Response r = conn->post("/WardwizEPS/api/MyTask/TaskStatus", "=" + CClientAgentCommonFunctions::get_string_from_wsz(JsonData.to_string().c_str()));
		RestClient::disable();

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::UpdateTaskStatusSEH", 0, 0, true, SECONDLEVEL);
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

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	http_client client(L"http://wssr0011/WardwizEPS/api/MyTask/SubTask");

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
			AddLogEntry(L"### Exception in CGenXClientAgent::GetInstallationTaskDetails, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CGenXClientAgent::GetInstallationTaskDetails", 0, 0, true, SECONDLEVEL);
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

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	http_client client(L"http://wssr0011/WardwizEPS/api/MyTask/SubTask");

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
			AddLogEntry(L"### Exception in CGenXClientAgent::GetInstallationTaskDetails, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CGenXClientAgent::GetInstallationTaskDetails", 0, 0, true, SECONDLEVEL);
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

			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(m_csMachineID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);

			CString csIP(strMachineIP.c_str());

			m_objScanTask.ReInitializeVariables();
			CStringArray	csPaths;
			if (!m_objScanTask.StartScan(1, csTaskID, csPaths))
			{
				AddLogEntry(L"### Failed to Start Scan: MachineIP: [%s]", csIP, 0, true, SECONDLEVEL);

				json::value lValueFailed;
				lValueFailed[L"MachineID"] = web::json::value::string(csDiscoverID.GetBuffer());
				lValueFailed[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
				lValueFailed[L"Status"] = web::json::value::number(TASK_FAILD);
				lValueFailed[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
				std::vector<web::json::value> arraylValueFailed;
				arraylValueFailed.push_back(lValueFailed);
				UpdateTaskStatus(arraylValueFailed);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::StartScanTask", 0, 0, true, SECONDLEVEL);
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
			std::vector<web::json::value> arraylValueSuccess;
			arraylValueSuccess.push_back(lValueSuccess);
			UpdateTaskStatus(arraylValueSuccess);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::StartUnInstallationTask", 0, 0, true, SECONDLEVEL);
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
		}

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

			json::value lValue;
			lValue[L"MachineID"] = web::json::value::string(csDiscoverID.GetBuffer());
			lValue[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValue[L"Status"] = web::json::value::number(TASK_IN_PROCESS);
			lValue[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());

			std::vector<web::json::value> arraylValue;
			arraylValue.push_back(lValue);
			UpdateTaskStatus(arraylValue);

			CString csIP(strMachineIP.c_str());
			CString csUserName(strUserName.c_str());
			CString csPassword(strPassword.c_str());

			m_objNetworkInstall.ReInitializeVariables();
			if (!m_objNetworkInstall.InstallClientSetup(csIP.GetBuffer(), csUserName.GetBuffer(), csPassword.GetBuffer()))
			{
				AddLogEntry(L"### Failed to Install Client: MachineIP: [%s]", csIP, 0, true, SECONDLEVEL);
				json::value lValueFailed;
				lValueFailed[L"MachineID"] = web::json::value::string(csDiscoverID.GetBuffer());
				lValueFailed[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
				lValueFailed[L"Status"] = web::json::value::number(TASK_FAILD);
				lValueFailed[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
				std::vector<web::json::value> arraylValueFailed;
				arraylValueFailed.push_back(lValueFailed);
				UpdateTaskStatus(arraylValueFailed);
				return false;
			}

			json::value lValueSuccess;
			lValueSuccess[L"MachineID"] = web::json::value::string(csDiscoverID.GetBuffer());
			lValueSuccess[L"TaskID"] = web::json::value::string(csTaskID.GetBuffer());
			lValueSuccess[L"Status"] = web::json::value::number(TASK_COMPLETED);
			lValueSuccess[L"TaskTypeID"] = web::json::value::string(csTaskTypeID.GetBuffer());
			std::vector<web::json::value> arraylValueSuccess;
			arraylValueSuccess.push_back(lValueSuccess);
			UpdateTaskStatus(arraylValueSuccess);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::StartInstallationTask", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CGenXClientAgent::NetworkTasksThread", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CGenXClientAgent::NetworkTasksThread", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CGenXClientAgent::NetworkTasksThread", 0, 0, true, SECONDLEVEL);
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
		CString strError = e.what();
		AddLogEntry(L"### Exception in CGenXClientAgent::GetBearerCode : %s", strError, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXClientAgent::GetBearerCode", 0, 0, true, SECONDLEVEL);
	}
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWardwizClientAgent::GetBearerCodeTask()
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetBearerCodeTask");

	utility::string_t Lreq = L"username=admin&password=admin&grant_type=password";

	http_client client(L"http://wssr0011/wardwizEps/token");

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
						if (!CWWizSettings::SetBearerCode(strBearer.c_str()))
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
			AddLogEntry(L"### Exception in CGenXClientAgent::GetInstallationTaskDetails, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CGenXClientAgent::GetBearerCodeTask", 0, 0, true, SECONDLEVEL);
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
		case START_UPDATE:
			m_pThis->m_objCommSrv.SendResponse(lpSpyData);
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GenXComSrv OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
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

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);

	utility::string_t Lreq = L"=" + JsonData.to_string();

	http_client client(L"http://wssr0011/WardwizEPS/api/MyTask/SubTask");

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
			AddLogEntry(L"### Exception in CGenXClientAgent::GetUnInstallationTaskDetails, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CGenXClientAgent::GetUnInstallationTaskDetails", 0, 0, true, SECONDLEVEL);
		}
	});
}