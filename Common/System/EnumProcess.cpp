#include "stdafx.h"
#include "EnumProcess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEnumProcess::CEnumProcess()
{
	try
	{
		PSAPI = NULL;
		FEnumProcesses = NULL;		// Pointer to EnumProcess
		FEnumProcessModules = NULL; // Pointer to EnumProcessModules
		FGetModuleFileNameEx = NULL;// Pointer to GetModuleFileNameEx
		FGetModuleBaseName = NULL;// Pointer to GetModuleFileNameEx

		TOOLHELP = NULL;			//Handle to the module (Kernel32)
		FCreateToolhelp32Snapshot = NULL;
		FProcess32First = NULL;
		FProcess32Next = NULL;
		FModule32First = NULL;
		FModule32Next = NULL;

		// Retrieve the OS version
		osver.dwOSVersionInfoSize = sizeof(osver);
		GetVersionEx(&osver);

		// Load required Dll
		InitProcessDll();

		EnablePrivilege(SE_DEBUG_NAME);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::CEnumProcess"), 0, 0, true, SECONDLEVEL);
	}
}

CEnumProcess::~CEnumProcess()
{
	try
	{
		FreeProcessDll();
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::~CEnumProcess"), 0, 0, true, SECONDLEVEL);
	}
}

bool CEnumProcess::InitProcessDll()
{
	try
	{
		// If Windows NT 4.0
		if(osver.dwPlatformId == VER_PLATFORM_WIN32_NT && (osver.dwMajorVersion == 4 
					|| (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0)))
		{
			if(!PSAPI)//If not already Loaded
			{
				PSAPI = ::LoadLibrary(_TEXT("PSAPI"));
				if(PSAPI == NULL)
				{
					return false;
				}

				FEnumProcesses       = (PFEnumProcesses)::GetProcAddress(PSAPI,
															LPCSTR("EnumProcesses"));
				FEnumProcessModules  = (PFEnumProcessModules)::GetProcAddress(PSAPI,
														LPCSTR("EnumProcessModules"));
#ifdef _UNICODE
				FGetModuleFileNameEx = (PFGetModuleFileNameEx)::GetProcAddress(PSAPI,
															("GetModuleFileNameExW"));
				FGetModuleBaseName   = (PFGetModuleBaseName)::GetProcAddress(PSAPI, 
															("GetModuleBaseNameW"));
#else
				FGetModuleFileNameEx = (PFGetModuleFileNameEx)::GetProcAddress(PSAPI, 
															("GetModuleFileNameExA"));
				FGetModuleBaseName	 = (PFGetModuleBaseName)::GetProcAddress(PSAPI, 
															("GetModuleBaseNameW"));
#endif
				if((!FEnumProcesses) || (!FEnumProcessModules) 
							|| (!FGetModuleFileNameEx) ||(!FGetModuleBaseName))
				{
					AddLogEntry(_T("### NT: Could not get the ProcAddress for process enum!"), 0, 0, true, SECONDLEVEL);
					FreeLibrary(PSAPI);
					PSAPI = NULL;
					return false;
				}
			}
		}
		//Windows 9x, Windows 2000, Windows XP, Windows 2003
		else if(osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
			(osver.dwPlatformId == VER_PLATFORM_WIN32_NT && osver.dwMajorVersion > 4))
		{
			if(!TOOLHELP)	//If not already Loaded
			{
				TOOLHELP = ::LoadLibrary(_TEXT("Kernel32"));
				if(TOOLHELP == NULL)
				{
					return false;
				}

				// Find ToolHelp functions
				FCreateToolhelp32Snapshot = (PFCreateToolhelp32Snapshot)::GetProcAddress
										(TOOLHELP, LPCSTR("CreateToolhelp32Snapshot"));
#ifdef _UNICODE
				FProcess32First = (PFProcess32First)::GetProcAddress
										(TOOLHELP, LPCSTR("Process32FirstW"));
				FProcess32Next = (PFProcess32Next)::GetProcAddress
										(TOOLHELP, LPCSTR("Process32NextW"));
				FModule32First = (PFModule32First)::GetProcAddress
										(TOOLHELP, LPCSTR("Module32FirstW"));
				FModule32Next = (PFModule32Next)::GetProcAddress
										(TOOLHELP, LPCSTR("Module32NextW"));
#else
				FProcess32First = (PFProcess32First)::GetProcAddress
										(TOOLHELP, LPCSTR("Process32First"));
				FProcess32Next = (PFProcess32Next)::GetProcAddress
										(TOOLHELP, LPCSTR("Process32Next"));
				FModule32First = (PFModule32First)::GetProcAddress
										(TOOLHELP, LPCSTR("Module32First"));
				FModule32Next = (PFModule32Next)::GetProcAddress
										(TOOLHELP, LPCSTR("Module32Next"));
#endif

				//Memory leak
				if((!FCreateToolhelp32Snapshot) || (!FProcess32First) ||
					(!FProcess32Next) || (!FModule32First) || (!FModule32Next))
				{
					AddLogEntry(_T("### 2K, XP: Could not get the ProcAddress for process enum!"), 0, 0, true, SECONDLEVEL);
					FreeLibrary(TOOLHELP);
					TOOLHELP = NULL;
					return false;
				}
			}
		}
		return true;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::InitProcessDll"), 0, 0, true, SECONDLEVEL);
	}
	return false;
}

bool CEnumProcess::FreeProcessDll()
{
	try
	{
		if(PSAPI)
		{
			FreeLibrary (PSAPI);
			PSAPI = NULL;

		}

		if(TOOLHELP)
		{
			FreeLibrary(TOOLHELP);
			TOOLHELP = NULL;
		}
		return true;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::FreeProcessDll"), 0, 0, true, SECONDLEVEL);
	}
}

bool CEnumProcess::EnumRunningProcesses(PROCESSHANDLER lpProc, LPVOID pThis)
{
	try
	{
		return HandleRequest(_T(""), false, lpProc, pThis, NULL);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::EnumRunningProcesses"), 0, 0, true, SECONDLEVEL);
	}
	return false;
}

bool CEnumProcess::IsProcessRunning(CString sProcName, bool bTerminateProcess,
									bool bIsFullPath,bool bTerminateTree)
{
	try
	{
		return HandleRequest(sProcName, bTerminateProcess, NULL, NULL, NULL,
								bIsFullPath, bTerminateTree);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::IsProcessRunning"), 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : IsProcessRunning
Description    : Function to check whether process is running
Author Name    : Jeena Mariam Saji
Date           : 7 Sept 2018
****************************************************************************/
bool CEnumProcess::IsProcessRunning(DWORD dwSesionID, CString sProcName, bool bTerminateProcess,
	bool bIsFullPath, bool bTerminateTree)
{
	try
	{
		return HandleRequest(dwSesionID, sProcName, bTerminateProcess, NULL, NULL, NULL,
			bIsFullPath, bTerminateTree);
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::IsProcessRunning"), 0, 0, true, SECONDLEVEL);
	}
	return false;
}

bool CEnumProcess::KillProcess(DWORD ProcessID)
{
	try
	{
		HANDLE hResult;
		//to open an existing process
		hResult = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessID);
		if(hResult)
		{
			TerminateProcess(hResult, 0);
			::CloseHandle(hResult);
			return true;
		}
		return false;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::KillProcess"), 0, 0, true, SECONDLEVEL);
	}
}

bool CEnumProcess::GetProcessModule(DWORD dwPID, LPCTSTR pstrModule, 
									LPMODULEENTRY32 lpMe32, DWORD cbMe32)
{
	try
	{
		BOOL			bRet;
		bool			bFound      = false;
		HANDLE			hModuleSnap = NULL;
#ifdef _UNICODE
		MODULEENTRY32W	me32        = {0};
#else
		MODULEENTRY32   me32        = {0};
#endif
		int				nLen = static_cast<int>(wcslen(pstrModule));

		if(!TOOLHELP)
		{
			return false;
		}

		if(!nLen)
		{
			return false;
		}

		hModuleSnap = FCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
		if(hModuleSnap == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		me32.dwSize = sizeof(MODULEENTRY32);
		bRet = FModule32First(hModuleSnap, &me32);
		while (bRet && !bFound)
		{
			if(me32.hModule  != INVALID_HANDLE_VALUE)
			{
				// locate the given filename in the modulelist (usually its the first anyway)
				if((_tcsnicmp(me32.szModule, pstrModule, nLen)== 0) || // For Win 2000, XP, 2003
					(_tcsnicmp(me32.szExePath, pstrModule, nLen)== 0))// For Win 95/98
				{
					CopyMemory(lpMe32, &me32, cbMe32);
					bFound = true;
				}
				bRet = FModule32Next(hModuleSnap, &me32);
			}
			else
			{
				bRet = false;
			}
		}
		CloseHandle(hModuleSnap);
		return (bFound);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::GetProcessModule"), 0, 0, true, SECONDLEVEL);
	}
	return false;
}

BOOL CEnumProcess::EnablePrivilege(LPCTSTR szPrivilege)
{
	BOOL bReturn = FALSE;
	try
	{
		HANDLE hToken;
		TOKEN_PRIVILEGES tpOld;
		if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
								&hToken))
		{
			return (FALSE);
		}

		bReturn = (EnableTokenPrivilege(hToken, szPrivilege, &tpOld));
		CloseHandle(hToken);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::EnablePrivilege"), 0, 0, true, SECONDLEVEL);
	}
	return (bReturn);
}

BOOL CEnumProcess::EnableTokenPrivilege(HANDLE htok, LPCTSTR szPrivilege,
										TOKEN_PRIVILEGES *tpOld)
{
	try
	{
		if(htok != INVALID_HANDLE_VALUE)
		{
			TOKEN_PRIVILEGES tp;
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if(LookupPrivilegeValue(0, szPrivilege, &tp.Privileges[0].Luid))
			{
				DWORD cbOld = sizeof (*tpOld);
				if(AdjustTokenPrivileges(htok, FALSE, &tp, cbOld, tpOld, &cbOld))
				{
					return (ERROR_NOT_ALL_ASSIGNED != GetLastError());
				}
				else
				{
					return (FALSE);
				}
			}
			else
			{
				return (FALSE);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::EnableTokenPrivilege"), 0, 0, true, SECONDLEVEL);
		return (FALSE);
	}
	return (FALSE);
}

bool CEnumProcess::HandleRequest(CString sProcName, bool bTerminateProcess,
								 PROCESSHANDLER lpProc, LPVOID pThis, 
								 LPDWORD pdwProdID, bool bIsFullPath, bool bTerminateTree)
{
	try
	{
		bool bReturnVal = false;
		bool bStopEnum = false;
		DWORD dwParentId = 0;
		// If Windows NT 4.0
		if(osver.dwPlatformId == VER_PLATFORM_WIN32_NT && (osver.dwMajorVersion == 4 
							|| (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0)))
		{
			if(PSAPI)
			{
				try
				{
					DWORD aProcesses[1024], cbNeeded, cProcesses;
					if(!FEnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
						throw;
					// Calculate how many process identifiers were returned.
					cProcesses = cbNeeded / sizeof(DWORD);
					for(unsigned int i=0; i <= cProcesses; i++)
					{
						CString csProcessName;
						bool bFound = false;
						TCHAR szProcessName[MAX_PATH]={0};
						// Get a handle to the process.
						HANDLE hProcess =  OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcesses[i]);
						if(hProcess)
						{
							HMODULE hMod = NULL;
							DWORD cbytesNeed=0;
							TCHAR    szModuleName[MAX_PATH]={0};
							// Get the process name.
							if(FEnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbytesNeed))
							{
								FGetModuleFileNameEx(hProcess, hMod, szProcessName,
													_countof(szProcessName));
								GetLongPathName(szProcessName, szProcessName, MAX_PATH);
								if(lpProc)//Request for Names only
								{
									if(FGetModuleBaseName)
									{
										FGetModuleBaseName(hProcess, hMod, szModuleName,
															_countof(szModuleName));
									}

									lpProc(szModuleName, szProcessName, aProcesses[i],
											pThis, bStopEnum);
									bReturnVal = true;
									if(bStopEnum)
									{
										::CloseHandle(hProcess);
										break;
									}
								}
								else
								{
									csProcessName = szProcessName;
									if(bIsFullPath)
									{
										if(!csProcessName.CompareNoCase(sProcName))
										{
											bFound = true;
										}
									}
									else
									{
										int iFind  = csProcessName.ReverseFind('\\');
										if(iFind != -1)
										{
											csProcessName = csProcessName.Mid(iFind +1);
										}
										if(!csProcessName.CompareNoCase(sProcName))
										{
											bFound = true;
										}
									}
									if(!(CString(szProcessName).CompareNoCase(sProcName)))
									{
										bReturnVal = true;
									}
								}
							}
							else
							{
								if(!lpProc)
								{
									int iFind  = csProcessName.ReverseFind('\\');
									if(iFind != -1)
									{
										csProcessName = csProcessName.Mid(iFind +1);
									}
									if(!csProcessName.CompareNoCase(sProcName))
									{
										bFound = true;
									}
								}
							}
							if(bFound)
							{
								if(pdwProdID)//requested only procid
								{
									memcpy(pdwProdID, &aProcesses[i], sizeof(DWORD));
									bReturnVal = true;
								}
								else
								{
									if(bTerminateProcess)
									{
										dwParentId = aProcesses[i];
										SuspendProcess( hProcess );
										bReturnVal = (TerminateProcess(hProcess, 0)== FALSE 
														? false : true);
									}
									else
									{
										bReturnVal = true; // Process found running!
									}
								}
							}
							///TODO: to terminate process tree.
							::CloseHandle(hProcess);
						}//if(NULL != hProcess)
					}//for(i = 0; i <= cProcesses; i++)
				}
				catch(...)
				{}
			} //if(PSAPI)
		}
		//Windows 9x, Windows 2000, Windows XP, Windows 2003
		else if(osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
			(osver.dwPlatformId == VER_PLATFORM_WIN32_NT && osver.dwMajorVersion > 4))
		{
			if(TOOLHELP)
			{
				HANDLE hProcessSnap;
				try
				{
#ifdef _UNICODE
					PROCESSENTRY32W pe32 = {0};
					MODULEENTRY32W  me32 = {0};
#else
					PROCESSENTRY32 pe32 = {0};
					MODULEENTRY32  me32 = {0};
#endif

					// Setup variables
					pe32.dwSize = sizeof(pe32);
					me32.dwSize = sizeof(me32);

					hProcessSnap = FCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
					if(INVALID_HANDLE_VALUE == hProcessSnap)
					{
						return false;
					}

					if(!FProcess32First(hProcessSnap, &pe32))
					{
						return false;
					}

					while(true)
					{
						bool bFound = false;
						DWORD ProcID = pe32.th32ProcessID;
						if(osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)//IN Win95/98 we already have the full path!
						{
							if(CString(pe32.szExeFile).Right(4) != _T(".DLL"))//Ignore files with.DLL extensions!
							{
								if(lpProc)//Request for Names only
								{
									lpProc(pe32.szExeFile, pe32.szExeFile, ProcID,
											pThis, bStopEnum);
									bReturnVal = true;
									if(bStopEnum)
									{
										break;
									}
								}
								else
								{
									if(!(CString(pe32.szExeFile).CompareNoCase(sProcName)))
									{
										bFound = true;
									}
								}
							}
						}
						else if(GetProcessModule(ProcID, pe32.szExeFile, &me32, 
												sizeof(MODULEENTRY32)))
						{
							GetLongPathName(me32.szExePath, me32.szExePath, MAX_PATH);
							if(lpProc)//Request for Names only
							{
								lpProc(pe32.szExeFile, me32.szExePath, ProcID, pThis, bStopEnum);
								bReturnVal = true;
								if(bStopEnum)
								{
									break;
								}
							}
							else
							{
								if(bIsFullPath)
								{
									if(!(CString(me32.szExePath).CompareNoCase(sProcName)))
									{
										bFound = true;
									}
								}
								else
								{
									if(!(CString(pe32.szExeFile).CompareNoCase(sProcName)))
									{
										bFound = true;
									}
								}
							}
						}
						else
						{
							if(!lpProc)
							{
								//if not full path then match only names
								if(!bIsFullPath)
								{
									if(!(CString(pe32.szExeFile).CompareNoCase(sProcName)))
									{
										bFound = true;
									}
								}
							}
							else
							{
								lpProc(pe32.szExeFile, L"", ProcID, pThis, bStopEnum);
								bReturnVal = true;
								if(bStopEnum)
								{
									break;
								}
							}
						}

						if(bFound)
						{
							if(pdwProdID)//requested only procid
							{
								memcpy(pdwProdID, &ProcID, sizeof(DWORD));
								bReturnVal = true;
							}
							else
							{
								if(bTerminateProcess)
								{
									dwParentId = ProcID;
									HANDLE hResult;
									//to open an existing process
									hResult = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcID);
									if(hResult)
									{
										SuspendProcess( hResult );
										if(TerminateProcess(hResult, 0))
										{
											bReturnVal = true;		// Process Terminated Success!
										}
										else
										{
											bReturnVal = false;		// Process Termination Failed!
										}
										::CloseHandle(hResult);
									}
								}
								else
								{
									bReturnVal = true; // Process found running!
								}
							}
						}
						//To kill processes tree
						if(bTerminateTree && dwParentId != 0)
						{
							if(dwParentId == pe32.th32ParentProcessID)
							{
								HANDLE hResult;
								//to open an existing process
								hResult = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcID);
								if(hResult)
								{
									SuspendProcess( hResult );
									if(TerminateProcess(hResult, 0))
									{
										bReturnVal = true;		// Process Terminated Success!
									}
									else
									{
										bReturnVal = false;		// Process Termination Failed!
									}
									::CloseHandle(hResult);
								}
							}
						}
						if(!FProcess32Next(hProcessSnap, &pe32))
						{
							break;
						}
					}
					CloseHandle(hProcessSnap);
				}
				catch(...)
				{
					AddLogEntry(_T("### Exception caught in HandleRequest"), 0, 0, true, SECONDLEVEL);
				}
			}
		}
		return bReturnVal;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::HandleRequest"), 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : HandleRequest
Description    : Function to handle if process already running
Author Name    : Jeena Mariam Saji
Date           : 7 Sept 2018
****************************************************************************/
bool CEnumProcess::HandleRequest(DWORD dwSessionID, CString sProcName, bool bTerminateProcess,
	PROCESSHANDLER lpProc, LPVOID pThis,
	LPDWORD pdwProdID, bool bIsFullPath, bool bTerminateTree)
{
	try
	{
		bool bReturnVal = false;
		bool bStopEnum = false;
		DWORD dwParentId = 0;
		// If Windows NT 4.0
		if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT && (osver.dwMajorVersion == 4
			|| (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0)))
		{
			if (PSAPI)
			{
				try
				{
					DWORD aProcesses[1024], cbNeeded, cProcesses;
					if (!FEnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
						throw;
					// Calculate how many process identifiers were returned.
					cProcesses = cbNeeded / sizeof(DWORD);
					for (unsigned int i = 0; i <= cProcesses; i++)
					{
						CString csProcessName;
						bool bFound = false;
						TCHAR szProcessName[MAX_PATH] = { 0 };
						// Get a handle to the process.
						HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcesses[i]);

						DWORD dwProcessSessionID = 0x00;
						ProcessIdToSessionId(aProcesses[i], &dwProcessSessionID);

						if (hProcess)
						{
							HMODULE hMod = NULL;
							DWORD cbytesNeed = 0;
							TCHAR    szModuleName[MAX_PATH] = { 0 };
							// Get the process name.
							if (FEnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbytesNeed))
							{
								FGetModuleFileNameEx(hProcess, hMod, szProcessName,
									_countof(szProcessName));
								GetLongPathName(szProcessName, szProcessName, MAX_PATH);
								if (lpProc)//Request for Names only
								{
									if (FGetModuleBaseName)
									{
										FGetModuleBaseName(hProcess, hMod, szModuleName,
											_countof(szModuleName));
									}

									lpProc(szModuleName, szProcessName, aProcesses[i],
										pThis, bStopEnum);
									bReturnVal = true;
									if (bStopEnum)
									{
										::CloseHandle(hProcess);
										break;
									}
								}
								else
								{
									csProcessName = szProcessName;
									if (bIsFullPath)
									{
										if (!csProcessName.CompareNoCase(sProcName))
										{
											if (dwSessionID == dwProcessSessionID)
												bFound = true;
										}
									}
									else
									{
										int iFind = csProcessName.ReverseFind('\\');
										if (iFind != -1)
										{
											csProcessName = csProcessName.Mid(iFind + 1);
										}
										if (!csProcessName.CompareNoCase(sProcName))
										{
											if (dwSessionID == dwProcessSessionID)
												bFound = true;
										}
									}
									if (!(CString(szProcessName).CompareNoCase(sProcName)))
									{
										bReturnVal = true;
									}
								}
							}
							else
							{
								if (!lpProc)
								{
									int iFind = csProcessName.ReverseFind('\\');
									if (iFind != -1)
									{
										csProcessName = csProcessName.Mid(iFind + 1);
									}
									if (!csProcessName.CompareNoCase(sProcName))
									{
										if (dwSessionID == dwProcessSessionID)
											bFound = true;
									}
								}
							}
							if (bFound)
							{
								if (pdwProdID)//requested only procid
								{
									memcpy(pdwProdID, &aProcesses[i], sizeof(DWORD));
									bReturnVal = true;
								}
								else
								{
									if (bTerminateProcess)
									{
										dwParentId = aProcesses[i];
										SuspendProcess(hProcess);
										bReturnVal = (TerminateProcess(hProcess, 0) == FALSE
											? false : true);
									}
									else
									{
										bReturnVal = true; // Process found running!
									}
								}
							}
							///TODO: to terminate process tree.
							::CloseHandle(hProcess);
						}//if(NULL != hProcess)
					}//for(i = 0; i <= cProcesses; i++)
				}
				catch (...)
				{
				}
			} //if(PSAPI)
		}
		//Windows 9x, Windows 2000, Windows XP, Windows 2003
		else if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
			(osver.dwPlatformId == VER_PLATFORM_WIN32_NT && osver.dwMajorVersion > 4))
		{
			if (TOOLHELP)
			{
				HANDLE hProcessSnap;
				try
				{
#ifdef _UNICODE
					PROCESSENTRY32W pe32 = { 0 };
					MODULEENTRY32W  me32 = { 0 };
#else
					PROCESSENTRY32 pe32 = { 0 };
					MODULEENTRY32  me32 = { 0 };
#endif

					// Setup variables
					pe32.dwSize = sizeof(pe32);
					me32.dwSize = sizeof(me32);

					hProcessSnap = FCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
					if (INVALID_HANDLE_VALUE == hProcessSnap)
					{
						return false;
					}

					if (!FProcess32First(hProcessSnap, &pe32))
					{
						return false;
					}

					while (true)
					{
						bool bFound = false;
						DWORD ProcID = pe32.th32ProcessID;
						DWORD dwProcessSessionID = 0x00;
						ProcessIdToSessionId(ProcID, &dwProcessSessionID);

						if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)//IN Win95/98 we already have the full path!
						{
							if (CString(pe32.szExeFile).Right(4) != _T(".DLL"))//Ignore files with.DLL extensions!
							{
								if (lpProc)//Request for Names only
								{
									lpProc(pe32.szExeFile, pe32.szExeFile, ProcID,
										pThis, bStopEnum);
									bReturnVal = true;
									if (bStopEnum)
									{
										break;
									}
								}
								else
								{
									if (!(CString(pe32.szExeFile).CompareNoCase(sProcName)))
									{
										if (dwSessionID == dwProcessSessionID)
											bFound = true;
									}
								}
							}
						}
						else if (GetProcessModule(ProcID, pe32.szExeFile, &me32,
							sizeof(MODULEENTRY32)))
						{
							GetLongPathName(me32.szExePath, me32.szExePath, MAX_PATH);
							if (lpProc)//Request for Names only
							{
								lpProc(pe32.szExeFile, me32.szExePath, ProcID, pThis, bStopEnum);
								bReturnVal = true;
								if (bStopEnum)
								{
									break;
								}
							}
							else
							{
								if (bIsFullPath)
								{
									if (!(CString(me32.szExePath).CompareNoCase(sProcName)))
									{
										if (dwSessionID == dwProcessSessionID)
											bFound = true;
									}
								}
								else
								{
									if (!(CString(pe32.szExeFile).CompareNoCase(sProcName)))
									{
										if (dwSessionID == dwProcessSessionID)
											bFound = true;
									}
								}
							}
						}
						else
						{
							if (!lpProc)
							{
								//if not full path then match only names
								if (!bIsFullPath)
								{
									if (!(CString(pe32.szExeFile).CompareNoCase(sProcName)))
									{
										if (dwSessionID == dwProcessSessionID)
											bFound = true;
									}
								}
							}
							else
							{
								lpProc(pe32.szExeFile, L"", ProcID, pThis, bStopEnum);
								bReturnVal = true;
								if (bStopEnum)
								{
									break;
								}
							}
						}

						if (bFound)
						{
							if (pdwProdID)//requested only procid
							{
								memcpy(pdwProdID, &ProcID, sizeof(DWORD));
								bReturnVal = true;
							}
							else
							{
								if (bTerminateProcess)
								{
									dwParentId = ProcID;
									HANDLE hResult;
									//to open an existing process
									hResult = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcID);
									if (hResult)
									{
										SuspendProcess(hResult);
										if (TerminateProcess(hResult, 0))
										{
											bReturnVal = true;		// Process Terminated Success!
										}
										else
										{
											bReturnVal = false;		// Process Termination Failed!
										}
										::CloseHandle(hResult);
									}
								}
								else
								{
									bReturnVal = true; // Process found running!
								}
							}
						}
						//To kill processes tree
						if (bTerminateTree && dwParentId != 0)
						{
							if (dwParentId == pe32.th32ParentProcessID)
							{
								HANDLE hResult;
								//to open an existing process
								hResult = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcID);
								if (hResult)
								{
									SuspendProcess(hResult);
									if (TerminateProcess(hResult, 0))
									{
										bReturnVal = true;		// Process Terminated Success!
									}
									else
									{
										bReturnVal = false;		// Process Termination Failed!
									}
									::CloseHandle(hResult);
								}
							}
						}
						if (!FProcess32Next(hProcessSnap, &pe32))
						{
							break;
						}
					}
					CloseHandle(hProcessSnap);
				}
				catch (...)
				{
					AddLogEntry(_T("### Exception caught in HandleRequest"), 0, 0, true, SECONDLEVEL);
				}
			}
		}
		return bReturnVal;
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::HandleRequest"), 0, 0, true, SECONDLEVEL);
	}
	return false;
}

void CEnumProcess::RebootSystem(DWORD dwType)
{
	try
	{
		HANDLE hToken;
		TOKEN_PRIVILEGES tkp; 
		// Get a token for this process.
		if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{

			// Get the LUID for the shutdown privilege.
			LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

			tkp.PrivilegeCount = 1;  // one privilege to set
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			// Get the shutdown privilege for this process.
			AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
		}
		else
		{
			CString cs;
			cs.Format(L"%d",::GetLastError());
			AddLogEntry(L">>> In CEnumProcess::RebootSystem()OpenProcessToken false %s",cs, 0, true, FIRSTLEVEL);
		}


		if(dwType == 0)
		{
			// Shut down the system and force all applications to close.
			if(!ExitWindowsEx(EWX_REBOOT| EWX_FORCE, 0))
			{
				CString cs;
				cs.Format(L"%d",::GetLastError());
				AddLogEntry(L">>> In CEnumProcess::RebootSystem for Restart ExitWindowsEx return false = %d",cs, 0, true, FIRSTLEVEL);
			}
		}
		if(dwType == 1)
		{
			if(!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE | EWX_POWEROFF, 0))
			{
				CString cs;
				cs.Format(L"%d",::GetLastError());
				AddLogEntry(L">>> In CEnumProcess::RebootSystem for ShutDown ExitWindowsEx return false = %d",cs, 0, true, FIRSTLEVEL);
			}
		}
		if(dwType == 2)
		{
			if(!ExitWindowsEx(EWX_LOGOFF| EWX_FORCE, 0))
			{
				CString cs;
				cs.Format(L"%d",::GetLastError());
				AddLogEntry(L">>> In CEnumProcess::RebootSystem for Log off ExitWindowsEx return false = %d",cs, 0, true, FIRSTLEVEL);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::RebootSystem"), 0, 0, true, SECONDLEVEL);
	}
}

bool CEnumProcess::KillExplorer()
{
	try
	{
		bool isKilled = false;
		EnablePrivilege(SE_DEBUG_NAME);

		HANDLE  hProcessSnap = NULL;
		PROCESSENTRY32 pe32  = {0};
		HANDLE hExplorer;

		//  Take a snapshot of all processes in the system.
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if(hProcessSnap == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(_T("### failed to take Snapshot"), 0, 0, true, SECONDLEVEL);
		}
		//  Fill in the size of the structure before using it.
		pe32.dwSize = sizeof(PROCESSENTRY32);

		hExplorer = _GetProcID(CString(_T("Explorer.exe")), pe32, hProcessSnap); // Getting Process Ids

		if(hExplorer != NULL)
		{
			CreateRemoteThread(hExplorer, NULL, 0, (LPTHREAD_START_ROUTINE)ExitProcess,
								(LPVOID)1, 0, NULL);
			CloseHandle(hExplorer);
			isKilled = true;
		}
		return isKilled;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::KillExplorer"), 0, 0, true, SECONDLEVEL);
	}
	return false;

}

HANDLE CEnumProcess::_GetProcID(CString csProcName, PROCESSENTRY32 pe32, HANDLE hSnapshot)
{
	try
	{
		HANDLE dProcId = 0;
		if(Process32First(hSnapshot, &pe32))
		{
			do
			{
				if((CString(pe32.szExeFile)).CompareNoCase (csProcName)== 0)
				{
					return dProcId =  OpenProcess(PROCESS_ALL_ACCESS, TRUE, pe32.th32ProcessID);
				}
			}while(Process32Next(hSnapshot, &pe32));
		}
		return dProcId;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::_GetProcID"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

DWORD CEnumProcess::GetProcessIDByName(CString csProcName)
{
	try
	{
		DWORD dwProcID = 0;
		HandleRequest(csProcName, false, 0, 0, &dwProcID, true, false);
		return dwProcID;
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::_GetProcID"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

void CEnumProcess::RestoreExplorer()
{
	try
	{
		WinExec("Explorer.exe",SW_NORMAL);
	}
	catch(...)
	{
		AddLogEntry(_T("### Exception caught in CEnumProcess::RestoreExplorer"), 0, 0, true, SECONDLEVEL);
	}

}


/*****************************************************
Function Name 	: SuspendProcess()
Description		: Suspends given process
Author Name		: Prajakta, Vilas
Date			: 29 - Nov - 2014
******************************************************/
bool CEnumProcess::SuspendProcess(HANDLE hProcess)
{

	typedef LONG (NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);

	bool bReturn = false;

	if( hProcess != NULL )
	{
		NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(
												GetModuleHandleA("ntdll"), "NtSuspendProcess");
		if( pfnNtSuspendProcess )
		{
			pfnNtSuspendProcess( hProcess );
			bReturn = true;
		}
		else
		{
			bReturn = false;
		}

	}

	return bReturn;
}