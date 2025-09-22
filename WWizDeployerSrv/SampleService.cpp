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
#include "SampleService.h"
#include "ThreadPool.h"
#pragma endregion


CSampleService::CSampleService(PWSTR pszServiceName, 
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


CSampleService::~CSampleService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}


//
//   FUNCTION: CSampleService::OnStart(DWORD, LPWSTR *)
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
void CSampleService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
    // Log a service start message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStart", 
        EVENTLOG_INFORMATION_TYPE);

    // Queue the main service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
}


//
//   FUNCTION: CSampleService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void CSampleService::ServiceWorkerThread(void)
{
	DWORD dwRetry = 0x00;

	m_bIsWow64 = IsWow64();

	TCHAR szModulePath[MAX_PATH] = {0};
	if (!GetModulePath(szModulePath, sizeof(szModulePath)))
	{
		AddLogEntry(L"### GetModulePath failed in CSampleService::ServiceWorkerThread", szModulePath, 0, true, SECONDLEVEL);
		return;
	}

	TCHAR szTargetPath[MAX_PATH] = { 0 };
	if (m_bIsWow64)
		_stprintf_s(szTargetPath, MAX_PATH, L"%s\\%s", szModulePath, WARDWIZEPSCLIENTX64);
	else
		_stprintf_s(szTargetPath, MAX_PATH, L"%s\\%s", szModulePath, WARDWIZEPSCLIENTX86);

	if (PathFileExists(szTargetPath))
	{
		RunSetupInSilentMode(szTargetPath);
	}

	//wait for some time
	Sleep(3 * 1000);

	CString csPath;
	csPath.Format(L"%s\\%s", szModulePath, WARDWIZEPSCLIENTX64);
	if (PathFileExists(csPath))
	{
		SetFileAttributes(csPath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(csPath);
	}

	csPath.Format(L"%s\\%s", szModulePath, WARDWIZEPSCLIENTX86);
	if (PathFileExists(csPath))
	{
		SetFileAttributes(csPath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(csPath);
	}

	csPath.Format(L"%s\\%s", szModulePath, IP_SETTINGS_FILE);
	if (PathFileExists(csPath))
	{
		SetFileAttributes(csPath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(csPath);
	}

	csPath.Format(L"%s\\%s", szModulePath, REMOTE_CLIENT_SERVICE_FILENAME);
	if (PathFileExists(csPath))
	{
		SetFileAttributes(csPath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(csPath);
	}

    // Signal the stopped event.
    SetEvent(m_hStoppedEvent);
}


//
//   FUNCTION: CSampleService::OnStop(void)
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
void CSampleService::OnStop()
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


/***********************************************************************************************
Function Name  : RunUtililtyInSilentModeSEH
Description    : Run utility.exe setup with verysilent mode and add utilities in startup menu
Author Name    : Neha Gharge
SR.NO		   :
Date           : 30 Apr 2015
***********************************************************************************************/
bool CSampleService::RunSetupInSilentMode(LPCTSTR szSetupPath)
{
	try
	{

		CString csExePath, csCommandLine, csExePathExist, csLanguage;
		if (!PathFileExists(szSetupPath))
		{
			AddLogEntry(L"### File is not available File: [%s]", szSetupPath, 0, true, FIRSTLEVEL);
			return false;
		}

		csExePath = szSetupPath;
		csLanguage += L"LANG=english";
		csCommandLine.Format(L"%s%s", L"/verysilent /SUPPRESSMSGBOXES /NORESTART /", csLanguage);

		SHELLEXECUTEINFO ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = csExePath;
		ShExecInfo.lpParameters = csCommandLine;
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SWP_HIDEWINDOW;
		ShExecInfo.hInstApp = NULL;
		ShellExecuteEx(&ShExecInfo);
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

		AddLogEntry(L">>> [OK] RunSetupInSilentMode: %s, CommandLine: %s", csExePathExist, csCommandLine, true, SECONDLEVEL);

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"Exception in WrdWizAluSrv::RunUtililtyInSilentMode", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

bool CSampleService::IsWow64()
{
	m_bIsWow64 = false;

	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS		IsWow64Process = NULL;

	OSVERSIONINFO	OSVI = { 0 };

	OSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OSVI);

	IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),
		"IsWow64Process");
	if (!IsWow64Process)
	{
		AddLogEntry(L"Failed to get module handle IsWow64Process from kernel32");
		return false;
	}
	IsWow64Process(GetCurrentProcess(), &m_bIsWow64);

	return m_bIsWow64 ? true : false;
}