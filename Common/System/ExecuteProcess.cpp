/**********************************************************************************************************              	  Program Name          : ExecuteProcess.cpp
	  Description           : Wrapper class which executes the application úsing Explorer.
	  Author Name			: Ramkrushna Shelke                                                                          	  Date Of Creation      : 31st Aug 2014
	  Version No            : 1.0.0.8
	  Special Logic Used    : Need to execute the process under user context using service, we need to launch the process							  in current logged in user.
***********************************************************************************************************/
#include <stdafx.h>
#include "ExecuteProcess.h"
#include "WardwizOSVersion.h"
#include <winbase.h>
#include <tlhelp32.h>

#include <Wtsapi32.h>
#include <Userenv.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************
  Function Name  : CExecuteProcess
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  Date           : 31st Aug 2014
****************************************************************************/
CExecuteProcess::CExecuteProcess(void)
{
}

/***************************************************************************
  Function Name  : CExecuteProcess
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  Date           : 31st Aug 2014
****************************************************************************/
CExecuteProcess::~CExecuteProcess(void)
{
}

/***************************************************************************
  Function Name  : GetExplorerProcessHandle
  Description    : Function which enumerates the process and returns the handle of explorer
  Author Name    : Ramkrushna Shelke
  Date           : 31st Aug 2014
****************************************************************************/
HANDLE CExecuteProcess::GetExplorerProcessHandle(CString csAccessProcessName)
{
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;
	ZeroMemory(&pe32,sizeof(pe32));
	HANDLE temp = NULL;
	try
	{
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if(Process32First(hSnapshot,&pe32))
		{
			do
			{
				CString csExeName = pe32.szExeFile;

				//Version:	: 19.0.0.015
				//On Vista Explorer.exe run as standard user with limited rights
				//So it will not able to execute process as run as admin 
				CWardWizOSversion objOSVersionWrap;
				DWORD dwOSType = objOSVersionWrap.DetectClientOSVersion();
				if(dwOSType == WINOS_VISTA)
				{
					if(csExeName.CompareNoCase(_T("explorer.exe")) == 0)
					{
						temp = OpenProcess (PROCESS_ALL_ACCESS,FALSE, pe32.th32ProcessID);
					}
				}
				else if(csAccessProcessName)
				{
					if(csExeName.CompareNoCase(csAccessProcessName) == 0)
					{
						temp = OpenProcess (PROCESS_ALL_ACCESS,FALSE, pe32.th32ProcessID);
						break;
					}
				}
			}while(Process32Next(hSnapshot,&pe32));
		}
	}
	catch(...)
	{
		AddLogEntry(_T("Exception caught in CExecuteProcess::GetExplorerProcessHandle"));
	}
	return temp;
}

/***************************************************************************
Function Name  : GetExplorerProcessHandle
Description    : Function which enumerates the process and returns the handle of explorer
Author Name    : Jeena Mariam Saji
Date           : 7th Sept 2018
****************************************************************************/
HANDLE CExecuteProcess::GetExplorerProcessHandle(CString csAccessProcessName, DWORD dwSessionID)
{
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;
	ZeroMemory(&pe32, sizeof(pe32));
	HANDLE temp = NULL;
	try
	{
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				CString csExeName = pe32.szExeFile;

				DWORD dwSID = 0x00;
				ProcessIdToSessionId(pe32.th32ProcessID, &dwSID);

				//Version:	: 19.0.0.015
				//On Vista Explorer.exe run as standard user with limited rights
				//So it will not able to execute process as run as admin 
				CWardWizOSversion objOSVersionWrap;
				DWORD dwOSType = objOSVersionWrap.DetectClientOSVersion();
				if (dwOSType == WINOS_VISTA)
				{
					if (csExeName.CompareNoCase(_T("explorer.exe")) == 0)
					{
						if (dwSessionID == dwSID)
						{
							temp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
						}
					}
				}
				else if (csAccessProcessName)
				{
					if (csExeName.CompareNoCase(csAccessProcessName) == 0)
					{
						if (dwSessionID == dwSID)
						{
							temp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
							break;
						}
					}
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
	}
	catch (...)
	{
		AddLogEntry(_T("Exception caught in CExecuteProcess::GetExplorerProcessHandle"));
	}
	return temp;
}

/***************************************************************************
  Function Name  : StartProcessWithToken
  Description    : Function which launch the process using explorer token.
  Author Name    : Ramkrushna Shelke
  Date           : 31st Aug 2014
****************************************************************************/
BOOL CExecuteProcess::StartProcessWithToken(CString csProcessPath, CString csCommandLineParam,
											CString csAccessProcessName, bool bWait)
{
	try
	{
		HANDLE				hToken = NULL;
		TOKEN_USER          oUser[16];
		DWORD               u32Needed;
		TCHAR               sUserName[256], domainName[256];
		DWORD               userNameSize, domainNameSize;
		SID_NAME_USE        sidType;

		ZeroMemory(oUser,sizeof(oUser));
		BOOL bRet = OpenProcessToken(GetExplorerProcessHandle(csAccessProcessName), TOKEN_ALL_ACCESS, &hToken);
		if(!bRet)
		{
			return FALSE;
		}

		if(hToken == NULL)
		{
			if(csAccessProcessName.CompareNoCase(L"explorer.exe") !=0)
			{
				if(!OpenProcessToken(GetExplorerProcessHandle(L"explorer.exe"), TOKEN_ALL_ACCESS, &hToken))
					return FALSE;

				if(hToken == NULL)
				{
					return FALSE;
				}
			}
		}

		GetTokenInformation(hToken, TokenUser, &oUser[0], sizeof(oUser), &u32Needed);
		userNameSize		= _countof (sUserName) - 1;
		domainNameSize      = _countof (domainName) - 1;

		LookupAccountSid (NULL, oUser[0].User.Sid, sUserName, &userNameSize, domainName, &domainNameSize, &sidType);
		HDESK       hdesk = NULL;
		HWINSTA     hwinsta = NULL, hwinstaSave = NULL;
		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		BOOL bResult = FALSE;
		// Save a handle to the caller's current window station.
		if((hwinstaSave = GetProcessWindowStation()) == NULL)
		{
			CloseHandle(hToken);
			return FALSE;
		}

		// Get a handle to the interactive window station.
		hwinsta = OpenWindowStation(
			_T("winsta0"),                   // the interactive window station
			FALSE,							// handle is not inheritable
			READ_CONTROL | WRITE_DAC);		// rights to read/write the DACL

		if(hwinsta == NULL)
		{
			SetProcessWindowStation (hwinstaSave);
			CloseHandle(hToken);
			return FALSE;
		}

		// To get the correct default desktop, set the caller's
		// window station to the interactive window station.
		if(!SetProcessWindowStation(hwinsta))
		{
			SetProcessWindowStation (hwinstaSave);
			CloseWindowStation(hwinsta);
			CloseHandle(hToken);
			return FALSE;
		}

		// Get a handle to the interactive desktop.
		hdesk = OpenDesktop(
			_T("default"),     // the interactive window station
			0,             // no interaction with other desktop processes
			FALSE,         // handle is not inheritable
			READ_CONTROL | // request the rights to read and write the DACL
			WRITE_DAC |
			DESKTOP_WRITEOBJECTS |
			DESKTOP_READOBJECTS);

		if(hdesk == NULL)
		{
			SetProcessWindowStation(hwinstaSave);
			CloseWindowStation(hwinsta);
			CloseHandle(hToken);
			return FALSE;
		}

		// Restore the caller's window station.
		if(!SetProcessWindowStation(hwinstaSave))
		{
			SetProcessWindowStation (hwinstaSave);
			CloseWindowStation(hwinsta);
			CloseDesktop(hdesk);
			CloseHandle(hToken);
			return FALSE;
		}

		// Impersonate client to ensure access to executable file.
		if(!ImpersonateLoggedOnUser(hToken))
		{
			SetProcessWindowStation (hwinstaSave);
			CloseWindowStation(hwinsta);
			CloseDesktop(hdesk);
			CloseHandle(hToken);
			return FALSE;
		}

		// Initialize the STARTUPINFO structure.
		// Specify that the process runs in the interactive desktop.
		ZeroMemory(	&si, sizeof(STARTUPINFO));
		si.cb		=  sizeof(STARTUPINFO);
		si.lpDesktop =  _T("winsta0\\default");

		TCHAR   csCmdParam[MAX_PATH] = {0};
		wcscpy_s(csCmdParam, _countof(csCmdParam), csCommandLineParam);

		bResult = CreateProcessAsUser(
			hToken,            // client's access token
			csProcessPath,     // file to execute
			csCmdParam,		 // command line
			NULL,              // pointer to process SECURITY_ATTRIBUTES
			NULL,              // pointer to thread SECURITY_ATTRIBUTES
			FALSE,             // handles are not inheritable
			NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,   // creation flags
			NULL,              // pointer to new environment block
			NULL,              // name of current directory
			&si,               // pointer to STARTUPINFO structure
			&pi                // receives information about new process
			);

		if(bResult && bWait && pi.hProcess)
		{
			::WaitForSingleObject(pi.hProcess, 1000 * 60 * 2);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}

		if(hwinstaSave)
			SetProcessWindowStation (hwinstaSave);
		if(hwinsta)
			CloseWindowStation(hwinsta);
		if(hdesk)
			CloseDesktop(hdesk);
		if(hToken)
			CloseHandle(hToken);
		// End impersonation of client.
		RevertToSelf();
		return bResult;
	}
	catch(...)
	{
		AddLogEntry(_T("Exception caught in KeyLoggerScannerDll.cpp StartProcess "));
	}
	return false;
}

/***************************************************************************
Function Name  : GetUserNamefromProcessID
Description    : Function which fetches the user name using explorer token.
Author Name    : Jeena Mariam Saji
Date           : 04 July 2019
****************************************************************************/
CString CExecuteProcess::GetUserNamefromProcessID(unsigned long ProcessID)
{
	try
	{
		TOKEN_USER          oUser[16];
		DWORD               u32Needed;
		TCHAR               sUserName[256], domainName[256];
		DWORD               userNameSize, domainNameSize;
		SID_NAME_USE        sidType;

		ZeroMemory(oUser, sizeof(oUser));

		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
		if (!hProc)
			return EMPTY_STRING;

		BOOL bRet = OpenProcessToken(hProc, TOKEN_ALL_ACCESS, &hProc);
		
		if (!bRet)
		{
			return L"";
		}

		GetTokenInformation(hProc, TokenUser, &oUser[0], sizeof(oUser), &u32Needed);
		userNameSize = _countof(sUserName) - 1;
		domainNameSize = _countof(domainName) - 1;

		LookupAccountSid(NULL, oUser[0].User.Sid, sUserName, &userNameSize, domainName, &domainNameSize, &sidType);
		return CString(sUserName);
	}
	catch (...)
	{
		AddLogEntry(_T("Exception caught in CExecuteProcess::GetUserNamefromProcessID"));
		return L"";
	}
}

/***************************************************************************
Function Name  : StartProcessWithToken
Description    : Function which launch the process using explorer token.
Author Name    : Ramkrushna Shelke
Updated By	   : Nitin K 14Th March 2015
//Additional Includes Required : Psapi.lib;Userenv.lib;Wtsapi32.lib;
Date           : 31st Aug 2014
****************************************************************************/
BOOL CExecuteProcess::StartProcessWithTokenExplorerWait(CString csProcessPath, CString csCommandLineParam,
	CString csAccessProcessName, bool bWait)
{
	BOOL					bResult = FALSE;
	try
	{
		PROCESS_INFORMATION		pi = { 0 };
		STARTUPINFO				si = { 0 };
		DWORD					dwSessionId = 0x00, winlogonPid = 0x00;
		HANDLE					hUserToken = NULL;
		HANDLE					hUserTokenDup = NULL;
		HANDLE					hPToken = NULL;
		HANDLE					hProcess = NULL;
		HANDLE					hToken = NULL;

		DWORD					dwCreationFlags = 0x00;
		DWORD					dwLastError = 0x00;

		// Log the client on to the local computer.

		dwSessionId = WTSGetActiveConsoleSessionId();

		DWORD dwRetryCount = 0;
		BOOL bRet = FALSE;
		bRet = OpenProcessToken(GetExplorerProcessHandle(csAccessProcessName), TOKEN_ALL_ACCESS, &hToken);
		if (!bRet)
		{
			AddLogEntry(_T("OpenProcessToken Failed for process: %s"), csProcessPath);
			goto Cleanup;
		}
		//////////////////////////////////////////
		// Get the explorer process handle
		////////////////////////////////////////

		PROCESSENTRY32 procEntry;

		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap == INVALID_HANDLE_VALUE)
		{
			//return bResult;
			goto Cleanup;
		}

		procEntry.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(hSnap, &procEntry))
		{
			//return bResult;
			goto Cleanup;
		}

		do
		{
			if (_wcsicmp(procEntry.szExeFile, csAccessProcessName) == 0)
			{
				// We found a explorer process...
				// make sure it's running in the console session
				DWORD winlogonSessId = 0;
				if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessId))
				{
					//This issue fixed specifically for WIN_VISTA, as session id for explorer is :2 and 
					//WTSGetActiveConsoleSessionId returns :1  so there is mismatch.
					//for reference: www.codeproject.com/Articles/36581/Interaction-between-services-and-applications-at-u
					//social.msdn.microsoft.com/Forums/en-US/e7bc506b-0822-4d59-88f4-fe8f19114393/strange-behavior-of-wtsgetactiveconsolesessionid-issue-on-vista-and-above?forum=windowsgeneraldevelopmentissues
					winlogonPid = procEntry.th32ProcessID;
					break;
				}
			}

		} while (Process32Next(hSnap, &procEntry));

		////////////////////////////////////////////////////////////////////////

		::WTSQueryUserToken(dwSessionId, &hUserToken);
		dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.lpDesktop = L"winsta0\\default";

		ZeroMemory(&pi, sizeof(pi));

		TOKEN_PRIVILEGES tp;
		LUID luid;

		hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, winlogonPid);

		if (!::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
			| TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID
			| TOKEN_READ | TOKEN_WRITE, &hPToken))
		{
			dwLastError = GetLastError();
			goto Cleanup;
			//return bResult;
		}

		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
		{
			//printf("Lookup Privilege value Error: %u\n", GetLastError());
			dwLastError = GetLastError();
			//return bResult;
			goto Cleanup;
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL,
			SecurityIdentification, TokenPrimary, &hUserTokenDup);

		dwLastError = GetLastError();

		//Adjust Token privilege
		SetTokenInformation(hUserTokenDup,
			TokenSessionId, (void*)dwSessionId, sizeof(DWORD));

		if (!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
			(PTOKEN_PRIVILEGES)NULL, NULL))
		{
			dwLastError = GetLastError();
			//return bResult;
			goto Cleanup;
		}

		//dwLastError = GetLastError();
		//if (dwLastError == ERROR_NOT_ALL_ASSIGNED)
		//{
		//	//return bResult;
		//	goto Cleanup;
		//}

		LPVOID	pEnv = NULL;

		if (::CreateEnvironmentBlock(&pEnv, hUserTokenDup, TRUE))
		{
			dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
		}
		else
			pEnv = NULL;

		// Launch the process in the client's logon session.

		bResult = CreateProcessAsUser(
			hUserTokenDup,						// client's access token
			csProcessPath,						// file to execute
			csCommandLineParam.GetBuffer(csCommandLineParam.GetLength()),                 // command line
			NULL,				// pointer to process SECURITY_ATTRIBUTES
			NULL,               // pointer to thread SECURITY_ATTRIBUTES
			FALSE,              // handles are not inheritable
			dwCreationFlags,     // creation flags
			pEnv,               // pointer to new environment block
			NULL,               // name of current directory
			&si,               // pointer to STARTUPINFO structure
			&pi                // receives information about new process
			);
		csCommandLineParam.ReleaseBuffer();
		dwLastError = GetLastError();

		if (bResult)
			bResult = TRUE;
		//Perform All the Close Handles tasks

	Cleanup:

		if (hProcess)
			CloseHandle(hProcess);

		if (hUserToken)
			CloseHandle(hUserToken);

		if (hUserTokenDup)
			CloseHandle(hUserTokenDup);

		if (hPToken)
			CloseHandle(hPToken);

		if (hToken)
			CloseHandle(hToken);
	}
	catch (...)
	{
		AddLogEntry(_T("Exception caught in KeyLoggerScannerDll.cpp StartProcess :: StartProcessWithTokenNew"));
	}
	return bResult;
}

/***************************************************************************
Function Name  : StartProcessWithTokenExplorer
Description    : Function which launch the process using explorer token.
Author Name    : Jeena Mariam Saji
Date           : 7 Sept 2018
****************************************************************************/
BOOL CExecuteProcess::StartProcessWithTokenExplorer(CString csProcessPath, DWORD dwSessionID, CString csCommandLineParam,
	CString csAccessProcessName, bool bWait)
{
	BOOL					bResult = FALSE;
	try
	{
		PROCESS_INFORMATION		pi = { 0 };
		STARTUPINFO				si = { 0 };
		DWORD					dwSessionId = 0x00, winlogonPid = 0x00;
		HANDLE					hUserToken = NULL;
		HANDLE					hUserTokenDup = NULL;
		HANDLE					hPToken = NULL;
		HANDLE					hProcess = NULL;
		HANDLE					hToken = NULL;

		DWORD					dwCreationFlags = 0x00;
		DWORD					dwLastError = 0x00;

		// Log the client on to the local computer.

		dwSessionId = WTSGetActiveConsoleSessionId();

		DWORD dwRetryCount = 0;
		BOOL bRet = FALSE;
		bRet = OpenProcessToken(GetExplorerProcessHandle(csAccessProcessName, dwSessionID), TOKEN_ALL_ACCESS, &hToken);
		if (!bRet)
		{
			AddLogEntry(_T("OpenProcessToken Failed for process: %s"), csProcessPath);
			goto Cleanup;
		}
		//////////////////////////////////////////
		// Get the explorer process handle
		////////////////////////////////////////

		PROCESSENTRY32 procEntry;

		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap == INVALID_HANDLE_VALUE)
		{
			//return bResult;
			goto Cleanup;
		}

		procEntry.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(hSnap, &procEntry))
		{
			//return bResult;
			goto Cleanup;
		}

		do
		{
			if (_wcsicmp(procEntry.szExeFile, csAccessProcessName) == 0)
			{
				// We found a explorer process...
				// make sure it's running in the console session
				DWORD winlogonSessId = 0;
				if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessId))
				{
					if (dwSessionID == winlogonSessId)
					{
						//This issue fixed specifically for WIN_VISTA, as session id for explorer is :2 and 
						//WTSGetActiveConsoleSessionId returns :1  so there is mismatch.
						//for reference: www.codeproject.com/Articles/36581/Interaction-between-services-and-applications-at-u
						//social.msdn.microsoft.com/Forums/en-US/e7bc506b-0822-4d59-88f4-fe8f19114393/strange-behavior-of-wtsgetactiveconsolesessionid-issue-on-vista-and-above?forum=windowsgeneraldevelopmentissues
						winlogonPid = procEntry.th32ProcessID;
						break;
					}
				}
			}

		} while (Process32Next(hSnap, &procEntry));

		////////////////////////////////////////////////////////////////////////

		::WTSQueryUserToken(dwSessionId, &hUserToken);
		dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.lpDesktop = L"winsta0\\default";

		ZeroMemory(&pi, sizeof(pi));

		TOKEN_PRIVILEGES tp;
		LUID luid;

		hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, winlogonPid);

		if (!::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
			| TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID
			| TOKEN_READ | TOKEN_WRITE, &hPToken))
		{
			dwLastError = GetLastError();
			goto Cleanup;
			//return bResult;
		}

		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
		{
			//printf("Lookup Privilege value Error: %u\n", GetLastError());
			dwLastError = GetLastError();
			//return bResult;
			goto Cleanup;
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL,
			SecurityIdentification, TokenPrimary, &hUserTokenDup);

		dwLastError = GetLastError();

		//Adjust Token privilege
		SetTokenInformation(hUserTokenDup,
			TokenSessionId, (void*)dwSessionId, sizeof(DWORD));

		if (!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
			(PTOKEN_PRIVILEGES)NULL, NULL))
		{
			dwLastError = GetLastError();
			//return bResult;
			goto Cleanup;
		}

		//dwLastError = GetLastError();
		//if (dwLastError == ERROR_NOT_ALL_ASSIGNED)
		//{
		//	//return bResult;
		//	goto Cleanup;
		//}

		LPVOID	pEnv = NULL;

		if (::CreateEnvironmentBlock(&pEnv, hUserTokenDup, TRUE))
		{
			dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
		}
		else
			pEnv = NULL;

		// Launch the process in the client's logon session.

		bResult = CreateProcessAsUser(
			hUserTokenDup,						// client's access token
			csProcessPath,						// file to execute
			csCommandLineParam.GetBuffer(csCommandLineParam.GetLength()),                 // command line
			NULL,				// pointer to process SECURITY_ATTRIBUTES
			NULL,               // pointer to thread SECURITY_ATTRIBUTES
			FALSE,              // handles are not inheritable
			dwCreationFlags,     // creation flags
			pEnv,               // pointer to new environment block
			NULL,               // name of current directory
			&si,               // pointer to STARTUPINFO structure
			&pi                // receives information about new process
			);
		csCommandLineParam.ReleaseBuffer();
		dwLastError = GetLastError();

		if (bResult)
			bResult = TRUE;
		//Perform All the Close Handles tasks

	Cleanup:

		if (hProcess)
			CloseHandle(hProcess);

		if (hUserToken)
			CloseHandle(hUserToken);

		if (hUserTokenDup)
			CloseHandle(hUserTokenDup);

		if (hPToken)
			CloseHandle(hPToken);

		if (hToken)
			CloseHandle(hToken);
	}
	catch (...)
	{
		AddLogEntry(_T("Exception caught in KeyLoggerScannerDll.cpp StartProcess :: StartProcessWithTokenNew"));
	}
	return bResult;
}