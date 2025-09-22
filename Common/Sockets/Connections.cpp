#pragma once
#include "StdAfx.h"
#include "Connections.h"

#define TESTFILE _T("\\Test.txt")

/*-----------------------------------------------------------------------------
Function		: CConnection
In Parameters	: csRemoteMachineName: Machine name to which connection has to be establish.
csPipeName		 : Name of the Named Pipe useing for communication.
hMainFrm			 : Handle of mainframe class use to send message for displaying status.
bAuditFlag		 : Flag used for checking protected node option.
Out Parameters	: void
Purpose			: Initializing elements
Author			: Ram Shelke
-----------------------------------------------------------------------------*/
CConnection::CConnection(CString csRemoteMachineIP, CString csRemoteMachineName, CString csPipeName,
	HWND hMainFrm, bool bAuditFlag)
{
	m_csRemoteMachineIP = csRemoteMachineIP;
	m_csRemoteMachineName = csRemoteMachineName;
	m_csPipeName = csPipeName;
	m_hPipe = INVALID_HANDLE_VALUE;
	m_hMainFrm = hMainFrm;
	m_bAskPwd = false;
}

/*-----------------------------------------------------------------------------
Function		: ~CConnection
In Parameters	: void
Out Parameters	: void
Purpose			: Destructor
Author			: Ram Shelke
-----------------------------------------------------------------------------*/
CConnection::~CConnection(void)
{
}

/*-----------------------------------------------------------------------------
Function		: EstablishAdminConnection
In Parameters	: void
Out Parameters	: bool
Purpose			: Establish network connection with remote computer.It checks whether
admin$ is shared on the remote PC.If admin$ is shared on PC and it is
accessible from this server then returns suceess, if some error occures
returns failure.
Author			: Ram Shelke
-----------------------------------------------------------------------------*/
DWORD CConnection::EstablishAdminConnection(LPTSTR lpszTaskID)
{
	DWORD dwRet = NO_ERROR;
	try
	{
		if (!lpszTaskID)
			return SANITY_CHECK_FAILURE;

		DWORD dwRetVal = NO_ERROR;
		CString csResource = L"ADMIN$";
		TCHAR szRemoteResource[MAX_PATH];

		// Remote resource, \\remote\ipc$, remote\admin$, ...
		_stprintf_s(szRemoteResource, L"\\\\%s\\%s", m_csRemoteMachineIP.GetBuffer(0),
			csResource.GetBuffer(0));

		// connect to the resource, based on bEstablish    
		NETRESOURCE nr;
		nr.dwType = RESOURCETYPE_ANY;
		nr.lpLocalName = NULL;
		nr.lpRemoteName = (LPTSTR)&szRemoteResource;
		nr.lpProvider = NULL;

		//Establish connection (using username/pwd)
		if (m_csUserName.GetLength() > 0)
		{
			dwRetVal = ::WNetAddConnection3(m_hMainFrm, &nr, m_csPassword, m_csUserName, m_bAskPwd ? CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE : CONNECT_UPDATE_PROFILE);

			//WNetAddConnection3 function is called with explicit user credentials specified 
			//in the pUsername and lpPassword to establish a connection with a network resource 
			//on a specific server and then called again with either of these parameters as NULL(to use the default user name or default password) to the same server
			if (dwRetVal != NO_ERROR)
			{
				dwRetVal = ::WNetAddConnection3(m_hMainFrm, &nr, NULL, NULL, m_bAskPwd ? CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE : CONNECT_UPDATE_PROFILE);
			}
		}
		else
		{
			dwRetVal = ::WNetAddConnection3(m_hMainFrm, &nr, NULL, NULL, m_bAskPwd ? CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE : CONNECT_UPDATE_PROFILE);
		}

		if (dwRetVal != NO_ERROR)
		{
			CString csLogOnMsg;
			csLogOnMsg.Format(L"Logon Failure");
			if (dwRetVal == ERROR_LOGON_FAILURE)
			{
				dwRet = LOGON_FAILURE;
				csLogOnMsg.Format(L"Access denied! Please make sure that the user provided has administrative privileges, Username used: [%s]", m_csUserName);
				AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), csLogOnMsg.GetBuffer());
				OutputDebugString(csLogOnMsg);
				return dwRet;
			}

			CString strMsg;
			if (dwRetVal == ERROR_CANCELLED)
			{
				dwRet = ERROR_USER_CANCELLED;
				strMsg.Format(L"MachineIP: [%s] Connection attempt canceled by the user, , Username used: [%s]", m_csRemoteMachineIP, m_csUserName);
				AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), strMsg.GetBuffer());
			}
			else
			{
				dwRet = CONNECTION_FAILED;
				strMsg.Format(L"Access denied! Please make sure that the user provided has administrative privileges, Username used: [%s]", m_csUserName);
				AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), strMsg.GetBuffer());
			}

			OutputDebugString(strMsg);
			return dwRet;
		}

		CString		csTestFilePath = szRemoteResource;
		csTestFilePath += TESTFILE;

		CStdioFile objFile;
		if (!objFile.Open(csTestFilePath, CFile::modeCreate | CFile::modeRead | CFile::shareDenyNone))
		{
			dwRet = CONNECTION_FAILED;
			CString strMsg;
			strMsg.Format(L"Access denied! Please make sure that the user provided has administrative privileges, Username used: [%s]", m_csUserName);
			AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), strMsg.GetBuffer());
			OutputDebugString(strMsg);
			return dwRet;
		}
		objFile.Close();
		DeleteFile(csTestFilePath);

		// Open remote Service Manager
		SC_HANDLE hSCM = ::OpenSCManager(m_csRemoteMachineIP.GetBuffer(0), NULL, SC_MANAGER_ALL_ACCESS);

		if (hSCM == NULL)
		{
			dwRet = CONNECTION_FAILED;
			CString strMsg;
			strMsg.Format(L"Access denied! Please make sure that the user provided has administrative privileges, Username used: [%s]", m_csRemoteMachineIP, m_csUserName);
			AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), strMsg.GetBuffer());
			OutputDebugString(strMsg);
			return dwRet;
		}

		::CloseServiceHandle(hSCM);
		return dwRet;
	}
	catch (...)
	{
		//AddLogEntry(L"Exception Caught in EstablishAdminConnection");
	}
	return dwRet;
}

/*-----------------------------------------------------------------------------
Function		: CopyReqFilesToRemoteMachine
In Parameters	: void
Out Parameters	: void
Purpose			: Copies the all files that are required for scanning to remote computer's
system32 folder.
Author			: Ram Shelke
Modified		: Changed for new setup ( only on installer exe being copied )
-----------------------------------------------------------------------------*/
bool CConnection::CopyReqFilesToRemoteMachine(LPTSTR lpszTaskID, CString csMachineID)
{
	try
	{
		if (!lpszTaskID)
			return SANITY_CHECK_FAILURE;

		HKEY	hkey;
		DWORD	size = MAX_PATH;
		DWORD	dwType = REG_SZ;
		CString csStatusMsg;
		TCHAR szInstalledPath[MAX_PATH] = { 0 };
		CString csError;

		// Read path of exe from registry
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WEPS_SERVER_REG_KEY, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
		{
			if (RegQueryValueEx(hkey, APP_FOLDER_KEY, NULL, &dwType, (BYTE *)szInstalledPath, &size) == ERROR_SUCCESS)
			{
				RegCloseKey(hkey);
		
				CString csInstalledPath;
				csInstalledPath = CString(szInstalledPath);
				
				if (!CopyFileToRemoteMachine(csInstalledPath.GetBuffer(), IP_SETTINGS_FILE, false))
				{
					csError.Format(L"Failed Copy file to remote machine, FileName: [%s]", IP_SETTINGS_FILE);
					AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), csError.GetBuffer());

					csInstalledPath.ReleaseBuffer();
					return false;
				}
				csInstalledPath.ReleaseBuffer();

				if (!CopyFileToRemoteMachine(szInstalledPath, REMOTE_CLIENT_SERVICE_FILENAME, false))
				{
					csError.Format(L"Failed Copy file to remote machine, FileName: [%s]", REMOTE_CLIENT_SERVICE_FILENAME);
					AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), csError.GetBuffer());
					return false;
				}

				if (!CopyFileToRemoteMachine(szInstalledPath, WARDWIZEPSCLIENTX86, false))
				{
					csError.Format(L"Failed Copy file to remote machine, FileName: [%s]", WARDWIZEPSCLIENTX86);
					AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), csError.GetBuffer());
					return false;
				}

				if (!CopyFileToRemoteMachine(szInstalledPath, WARDWIZEPSCLIENTX64, false))
				{
					csError.Format(L"Failed Copy file to remote machine, FileName: [%s]", WARDWIZEPSCLIENTX64);
					AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), csError.GetBuffer());
					return false;
				}

				OutputDebugString(L"Start Installer service!");
				if (!StartRemoteService(REMOTE_CLIENT_SERVICE))
				{
					csError.Format(L"Failed to Start Installer service, FileName: [%s]", REMOTE_CLIENT_SERVICE);
					AddErrorServerLog(lpszTaskID, L"3", m_csRemoteMachineIP.GetBuffer(), csError.GetBuffer());
					return false;
				}
			}
			else
			{
				RegCloseKey(hkey);
			}
		}
		return true;
	}
	catch (...)
	{
		//AddLogEntry(L"Exception caught in CopyReqFilesToRemoteMachine");
	}
	return false;
}

/*-----------------------------------------------------------------------------
Function		: CopyFileToRemoteMachine
In Parameters	: szInstalledPath: Installed path of application.
FileName: Name of file to copy.
bCheckAlreadyPresent: Flag for whether to check file already present.
Out Parameters	:
Purpose			: Copies the particular file passed to the function to remote computer's
system32 folder.
Author			:
-----------------------------------------------------------------------------*/
bool CConnection::CopyFileToRemoteMachine(TCHAR* szInstalledPath, CString FileName, bool  bCheckAlreadyPresent,
	bool	bDataBaseFile)
{
	try
	{
		TCHAR szSrcFilePath[MAX_PATH] = { 0 }, szDestFilePath[MAX_PATH] = { 0 };

		_stprintf_s(szSrcFilePath, L"%s\\%s", szInstalledPath, FileName);

		if (bCheckAlreadyPresent)
		{
			_stprintf_s(szDestFilePath, L"\\\\%s\\ADMIN$\\", m_csRemoteMachineIP.GetBuffer(0));
		}
		else
		{
			_stprintf_s(szDestFilePath, L"\\\\%s\\ADMIN$%s",
				m_csRemoteMachineIP.GetBuffer(0), SYSTEM_FOLDER);

			if (!PathIsDirectory(szDestFilePath))
			{
				if (!CreateDirectory(szDestFilePath, NULL))
				{
					ShowErrorMessage(L"Can not create folder");
				}
			}
		}
		_tcscat_s(szDestFilePath, L"\\");
		_tcscat_s(szDestFilePath, FileName);

		if (bCheckAlreadyPresent)
		{
			CFileFind findFile;
			if (findFile.FindFile(szDestFilePath))
			{
				findFile.Close();
				return true;
			}
			findFile.Close();
		}

		CString csStatusMsg;
		csStatusMsg.Format(L"[%s] Copying file: %s!", m_csRemoteMachineName, szDestFilePath);
		OutputDebugString(csStatusMsg);

		if (CopyFile(szSrcFilePath, szDestFilePath, FALSE))
		{
			return true;
		}

		csStatusMsg.Format(L"[%s] Could not copy %s ! The file may be in use!", m_csRemoteMachineName, FileName);
		OutputDebugString(csStatusMsg);
	}
	catch (...)
	{
		//AddLogEntry(L"Exception caught in CopyFileToRemoteMachine");
	}
	return false;
}

/*-----------------------------------------------------------------------------
Function		: StartRemoteService
In Parameters	: csServiceName: Service name to start
Out Parameters	: bool
Purpose			: Starts the service on remote PC.
Author			:
-----------------------------------------------------------------------------*/
bool CConnection::StartRemoteService(CString csServiceName, bool bShowMessage)
{
	try
	{
		bool bReturn = false;
		CString strMsg;

		OutputDebugString(L"In StartRemoteService: " + csServiceName);

		SetLastError(0);
		// Open remote Service Manager
		SC_HANDLE hSCM = ::OpenSCManager(m_csRemoteMachineIP.GetBuffer(0), NULL, SC_MANAGER_ALL_ACCESS);

		if (hSCM == NULL)
		{
			if (bShowMessage)
			{
				strMsg = L"[" + m_csRemoteMachineName + L"] Could not open the service control manager!";
				ShowErrorMessage(strMsg);
			}
			return bReturn;
		}
		SetLastError(0);

		// Maybe it's already there and installed, let's try to run
		SC_HANDLE hService = ::OpenService(hSCM, csServiceName, SERVICE_ALL_ACCESS);

		// Creates service on remote machine, if it's not installed yet
		if (hService == NULL)
		{
			CString scServiceExe;
			scServiceExe = _T("%SystemRoot%\\");//CSystemInfo::m_strProgramFilesDir;//
			scServiceExe += SYSTEM_FOLDER;
			scServiceExe += L"\\";
			scServiceExe += REMOTE_CLIENT_SERVICE_FILENAME;
			SetLastError(0);
			hService = ::CreateService(hSCM, csServiceName, csServiceName, SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
				SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
				scServiceExe, NULL, NULL, NULL, NULL, NULL);
			OutputDebugString(L"CreateRemoteService: " + csServiceName + _T(" - ") + scServiceExe);
		}

		if (hService == NULL)
		{
			if (bShowMessage)
			{
				strMsg.Format(L"[%s] Could not create/open the client service!", m_csRemoteMachineName);
			}
			::CloseServiceHandle(hSCM);
			return bReturn;
		}

		if (bShowMessage)
		{
			if (csServiceName == REMOTE_CLIENT_SERVICE)
			{
				strMsg.Format(L"[%s] Starting client service: %s", m_csRemoteMachineName, csServiceName);
				OutputDebugString(strMsg);
			}
		}
		SetLastError(0);

		// Start service
		if (!::StartService(hService, 0, NULL))
		{
			DWORD dwErrorCode = GetLastError();
			// service already running
			if (dwErrorCode != ERROR_SERVICE_ALREADY_RUNNING)
			{
				if (bShowMessage)
				{
					strMsg.Format(L"[%s] Could not start the %s service! Please restart %s and try again!", m_csRemoteMachineName, csServiceName, m_csRemoteMachineName);
				}
				::CloseServiceHandle(hService);
				::CloseServiceHandle(hSCM);
				return bReturn;
			}
		}

		SERVICE_STATUS ssStatus;
		DWORD dwOldCheckPoint;
		DWORD dwStartTickCount;
		DWORD dwWaitTime;

		// Check the status until the service is no longer start pending. 
		if (!QueryServiceStatus(hService,   // handle to service 
			&ssStatus))  // address of status information structure
		{
			if (bShowMessage)
			{
				strMsg.Format(L"[%s] Could not query the status of the client service!", m_csRemoteMachineName);
				ShowErrorMessage(strMsg);
			}
			::CloseServiceHandle(hService);
			::CloseServiceHandle(hSCM);
			return false;
		}

		// Save the tick count and initial checkpoint.
		dwStartTickCount = GetTickCount();
		dwOldCheckPoint = ssStatus.dwCheckPoint;

		dwWaitTime = 30000;
		while (ssStatus.dwCurrentState != SERVICE_RUNNING)
		{
			// Check the status again. 
			if (!QueryServiceStatus(hService,   // handle to service 
				&ssStatus))  // address of structure
			{
				break;
			}

			if (ssStatus.dwCheckPoint > dwOldCheckPoint)
			{
				// The service is making progress.
				dwStartTickCount = GetTickCount();
				dwOldCheckPoint = ssStatus.dwCheckPoint;
			}
			else
			{
				if (GetTickCount() - dwStartTickCount > dwWaitTime)
				{
					// No progress made within the wait hint
					break;
				}
			}
			Sleep(500);
		}

		if (ssStatus.dwCurrentState == SERVICE_RUNNING)
		{
			bReturn = true;
		}
		else
		{
			if (bShowMessage)
			{
				strMsg.Format(L"[%s] Could not start the %s service! Please restart %s and try again!", m_csRemoteMachineName, csServiceName, m_csRemoteMachineName);
				ShowErrorMessage(strMsg);
			}
			bReturn = false;
		}

		::CloseServiceHandle(hService);
		::CloseServiceHandle(hSCM);

		if (bReturn)
		{
			OutputDebugString(L"Remote Service Started successfully: " + m_csRemoteMachineName + L" - " + csServiceName);
			strMsg.Format(L"[%s] Client service started successfully! (%s)", m_csRemoteMachineName, csServiceName);
			OutputDebugString(strMsg);
			return true; //Avinash B since client service is started successfully and the return value is used to change the image.
		}
		else
		{
			OutputDebugString(L"Failed to start Remote Service: " + m_csRemoteMachineName + L" - " + csServiceName);
			strMsg.Format(L"[%s] Starting client service failed! (%s)", m_csRemoteMachineName, csServiceName);
			OutputDebugString(strMsg);
		}
	}
	catch (...)
	{
		//AddLogEntry(L"Exception Caught in StartRemoteService");
	}
	return false;
}

/*-----------------------------------------------------------------------------
Function		: StopRemoteService
In Parameters	: csServiceName: Service name to stop.
bDelete: flag for whether only stop or atop and delete service.
Out Parameters	: DWORD
Purpose			: Stops the service on remote PC.Returns success if service stopped
sucessfully otherwise returns failure.
Author			:
-----------------------------------------------------------------------------*/
DWORD CConnection::StopRemoteService(CString csServiceName, bool	bDelete)
{
	try
	{
		CString strMsg;
		// Open remote Service Manager
		SC_HANDLE hSCM = ::OpenSCManager(m_csRemoteMachineIP.GetBuffer(0), NULL,
			SC_MANAGER_ALL_ACCESS);

		if (hSCM == NULL)
		{
			return FALSE;
		}

		// OPen	service
		SC_HANDLE hService = ::OpenService(hSCM, csServiceName, SERVICE_ALL_ACCESS);

		if (hService == NULL)
		{
			::CloseServiceHandle(hSCM);
			return FALSE;
		}

		SERVICE_STATUS ssStatus;
		DWORD dwOldCheckPoint;
		DWORD dwStartTickCount;
		DWORD dwWaitTime;

		// Make	sure the service is	not	already	stopped
		if (!QueryServiceStatus(hService, &ssStatus))
		{
			::CloseServiceHandle(hSCM);
			::CloseServiceHandle(hService);
			return FALSE;
		}

		if (ssStatus.dwCurrentState == SERVICE_STOPPED)
		{
			// Deletes service from	service	database
			if (bDelete)
				DeleteService(hService);

			::CloseServiceHandle(hSCM);
			::CloseServiceHandle(hService);
			return TRUE;
		}

		// Send	a stop code	to the main	service
		if (!ControlService(hService, SERVICE_CONTROL_STOP, &ssStatus))
		{
			::CloseServiceHandle(hSCM);
			::CloseServiceHandle(hService);
			return FALSE;
		}

		dwStartTickCount = GetTickCount();
		dwOldCheckPoint = ssStatus.dwCheckPoint;

		dwWaitTime = 30000;

		// If a	stop is	pending, just wait for it
		while (ssStatus.dwCurrentState != SERVICE_STOPPED)
		{
			// Check the status again. 
			if (!QueryServiceStatus(hService,   // handle to service 
				&ssStatus))  // address of structure
			{
				break;
			}

			if (ssStatus.dwCheckPoint > dwOldCheckPoint)
			{
				// The service is making progress.
				dwStartTickCount = GetTickCount();
				dwOldCheckPoint = ssStatus.dwCheckPoint;
			}
			else
			{
				if (GetTickCount() - dwStartTickCount > dwWaitTime)
				{
					// No progress made within the wait hint
					break;
				}
			}
			Sleep(500);
		}

		// Deletes service from	service	database
		if (bDelete)
		{
			DeleteService(hService);
		}

		::CloseServiceHandle(hSCM);
		::CloseServiceHandle(hService);

		return  TRUE;
	}
	catch (...)
	{
		//AddLogEntry(L"Exception Caught in StopremoteService");
	}
	return FALSE;
}

/*-----------------------------------------------------------------------------
Function		: WriteinPipe
In Parameters	: lpBuffer: data to write
nNumberOfBytesToRead: number of bytes to write
Out Parameters	: BOOL , lpNumberOfBytesRead: number of bytes written successfully
Purpose			: Writes data to Named Pipe which is used for the communication between
client and server.Returns true if writing operation is successful
otherwise false
Author			:
-----------------------------------------------------------------------------*/
BOOL CConnection::WriteinPipe(void*  lpBuffer, DWORD  nNumberOfBytesToWrite,
	DWORD* lpNumberOfBytesWritten)
{
	try
	{
		BOOL bRet = ::WriteFile(m_hPipe, lpBuffer, nNumberOfBytesToWrite,
			lpNumberOfBytesWritten, NULL);

		FlushFileBuffers(m_hPipe);
		return bRet;
	}
	catch (...)
	{
		//AddLogEntry(L"Exception Caught in WriteinPipe");
	}
	return FALSE;
}

/*-----------------------------------------------------------------------------
Function		: InstallClient
In Parameters	: void
Out Parameters	: bool
Purpose			:
Author			:
-----------------------------------------------------------------------------*/
DWORD CConnection::InstallClient(LPTSTR lpszTaskID, CString csMachineId)
{
	DWORD dwRet = 0x00;
	try
	{
		if (!lpszTaskID)
			return SANITY_CHECK_FAILURE;

		//stop remote service if running.
		if (StopRemoteService(REMOTE_CLIENT_SERVICE, true))
		{
			Sleep(2 * 1000);
		}

		OutputDebugString(L"Copy Files!");
		if (!CopyReqFilesToRemoteMachine(lpszTaskID, csMachineId))
		{
			dwRet = 0x01;
			return dwRet;
		}

		OutputDebugString(L"Wait for Installer service to finish!");	
		Sleep(60000); // Wait for the client setup to finish!

		OutputDebugString(L"Stop Installer service!");
		StopRemoteService(REMOTE_CLIENT_SERVICE, true);

		CString csTemp;
		csTemp.Format(L"MachineName: [%s] Client service installed successfully!", m_csRemoteMachineName);
		OutputDebugString(csTemp);

		return dwRet;
	}
	catch (...)
	{
		//AddLogEntry(L"Eception caught in CConnection::InstallClient");
	}
	return dwRet;
}

/*-----------------------------------------------------------------------------
Function		: ShowErrorMessage
In Parameters	: CString
Out Parameters	: void
Purpose			: Shows Error Message
Author			:
-----------------------------------------------------------------------------*/
void CConnection::ShowErrorMessage(CString str)
{
	try
	{
		LPVOID lpMsgBuf;
		CString csStatusMsg;
		DWORD dwErrorCode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwErrorCode, 0, (LPTSTR)&lpMsgBuf, 0, NULL);
		csStatusMsg.Format(L"%s %s (Error code: %d)", str, (LPCTSTR)lpMsgBuf, dwErrorCode);
		LocalFree(lpMsgBuf);
		csStatusMsg.Replace(L"\r", L""); csStatusMsg.Replace(L"\n", L"");
		OutputDebugString(csStatusMsg);
		SetLastError(0);
	}
	catch (...)
	{
		//AddLogEntry(L"Exception caught in ShowErrorMessage");
	}
}
