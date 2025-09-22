/********************************************************************************************************** 
   Program Name          : ScannerLoad.cpp
   Description           : Defines the functionality require to manage scanner for OnAccess protectio.
   Author Name           : Lalit                                                                               
   Date Of Creation      : 26-12-2014
   Version No            : 
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description
***********************************************************************************************************/

#include "stdafx.h"
#include "CSecure64.h"
#include "DriverConstants.h"
#include "CScannerLoad.h"

#define SERVICE_KEY				L"SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner"
#define SERVICE_NAME 			L"wrdwizscanner"
//#define SERVICE_DISPLAY_NAME	L"wrdwizscanner"
#define INSTANCE_KEY			L"wrdwizscanner"
#define INSTANCE_LOCAL_KEY		L"Instances\\wrdwizscanner"
#define SCANNER_ALTITUDE		L"383251"
//=============================================================================

/***************************************************************************
  Function Name  : RegisterProcessId
  Description    : To register scanner before any operation so it will allow 
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           :26-12-2014 
****************************************************************************/
bool CScannerLoad:: RegisterProcessId(ULONG processID)
{
	bool bIsRegisterdSuccess = false;
	__try
	{
		HANDLE hFileHandle = NULL;
		DWORD dwReturn;
		ULONG ulIndex =  processID; //WLSRV_ID_ZERO;
	    
		hFileHandle = CreateFile(csPipeScannerProt, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (!hFileHandle)
		{
			return bIsRegisterdSuccess;
		}

		if(hFileHandle)
		{
			REGISTER_PROCESS_ID *pStruct = NULL;

			pStruct = (REGISTER_PROCESS_ID*)malloc(sizeof(REGISTER_PROCESS_ID));

			if(pStruct != NULL)
			{
				memset(pStruct,'\0', sizeof(REGISTER_PROCESS_ID));
				memcpy(pStruct->SecureString,SECURE_STRING,sizeof(SECURE_STRING));
				pStruct->ProcessIdCode = ulIndex;
				
				if(DeviceIoControl(hFileHandle, IOCTL_REGISTER_PROCESSID,pStruct, sizeof(REGISTER_PROCESS_ID), pStruct, sizeof(REGISTER_PROCESS_ID), &dwReturn, NULL))
				{
					CloseHandle(hFileHandle);
					bIsRegisterdSuccess = true;
				}
			}	
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### CScannerLoad:: Exception in RegisterProcessId", 0, 0, true, SECONDLEVEL);
	}
	return bIsRegisterdSuccess;
}

/***************************************************************************
  Function Name  : InstallService
  Description    : it uses to install scanner.sys
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           :26-12-2014 
****************************************************************************/
int CScannerLoad:: InstallService(CString sysFilePath)
{
	int iReturnStatus = 0;
	try
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hService = NULL;
		DWORD        tagid = 0;
		LPCWSTR DRIVER_LOCATION = (LPCWSTR)sysFilePath;
		
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

		if (!hSCManager)
		{
			return iReturnStatus;
		}
		
		if (hSCManager)
		{
			hService = CreateServiceW(
				hSCManager,
				SERVICE_NAME,
				SERVICE_NAME,
				SERVICE_START | DELETE | SERVICE_STOP, 
				SERVICE_FILE_SYSTEM_DRIVER,
				SERVICE_SYSTEM_START,
				SERVICE_ERROR_NORMAL, 
				DRIVER_LOCATION, 
				L"FSFilter Activity Monitor",
				&tagid,
				L"FltMgr",
				NULL,
				NULL
				);
			//hService = CreateServiceW(hSCManager, SERVICE_NAME, SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP, SERVICE_FILE_SYSTEM_DRIVER, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, DRIVER_LOCATION, L"FSFilter Activity Monitor", NULL, NULL, NULL, NULL);
			if (!hService)
			{
				hService = OpenServiceW(hSCManager, SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP);	
			}
			
			if (hService)
			{
				iReturnStatus = 1;
			}
			
		}
			
		if (hSCManager)
		{
			CloseServiceHandle(hSCManager);	
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CScannerLoad::Exception in InstallService", 0, 0, true, SECONDLEVEL);
	}
	
	return iReturnStatus;
}

/***************************************************************************
  Function Name  : StartServiceLocal
  Description    : it uses to start scanner.sys 
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           :26-12-2014 
****************************************************************************/
 int CScannerLoad:: StartServiceLocal()
{
	 int iReturnStatus = DRIVER_START_FAILED;
	 __try
	{
		 TCHAR szError[16] = { 0 };
		 SC_HANDLE hSCManager = NULL;
		 SC_HANDLE hService = NULL;
		
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

		 if (!hSCManager)
		{
			 return iReturnStatus;
		 }

		 if (hSCManager)
		 {
			hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
			 if (hService)
			{
				 if (StartService(hService, 0, NULL))
				{
					 iReturnStatus = DRIVER_SUCCESSFULLY_STARTED;
				}
				else
				{
					DWORD dwErrorCode = 0;
					dwErrorCode = GetLastError();
					 wsprintf(szError, L"%d", dwErrorCode);
					 AddLogEntry(L"### CScannerLoad:: Error during CScannerLoad service start with error %s", szError, 0, true, SECONDLEVEL);
				}
			}
		}
		
		 if (hSCManager)
		{
			CloseServiceHandle(hSCManager);	
		}

		RegisterProcessId(1);	
	}
	 __except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		 AddLogEntry(L"### CScannerLoad::Exception in CScannerLoad:: StartServiceLocal", 0, 0, true, SECONDLEVEL);
	}
	 return iReturnStatus;
}

 /***************************************************************************
  Function Name  : RemoveService
  Description    : it uses to remove scanner.sys service 
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           :26-12-2014 
****************************************************************************/
int CScannerLoad:: RemoveService()
{
	int iReturnStatus = DRIVER_REMOVAL_FAILED;
	__try
	{
		SC_HANDLE hSCManager;
		SC_HANDLE hService;
		SERVICE_STATUS ss;
		
		RegisterProcessId(1);
		
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (!hSCManager)
		{
			return iReturnStatus;
		}

		if (hSCManager)
		{
			hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
			if (hService)
			{
				if (!ControlService(hService, SERVICE_CONTROL_STOP, &ss))
				{
					CloseServiceHandle(hSCManager);
					CloseServiceHandle(hService);
					return iReturnStatus;
				}
			
				if (DeleteService(hService))
				{
					iReturnStatus = DRIVER_REMOVAL_SUCCESS;
				}
			}
		}
		
		if (hSCManager)
		{
			CloseServiceHandle(hSCManager);	
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### CScannerLoad::Exception in RemoveService", 0, 0, true, SECONDLEVEL);
	}
	return iReturnStatus;
}

 
 /***************************************************************************
  Function Name  : InstallScanner
  Description    : it uses to install scanner
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           :26-12-2014 
****************************************************************************/
int CScannerLoad:: InstallScanner(CString sysFilePath)
{
	try
	{
		if (sysFilePath.Trim().GetLength() == 0)
		{
			return 0;
		}

			HKEY            	phkey;
			DWORD            	lpdwDisposition = 0;
			LPWSTR    			ServiceDescription = L"File system mini filter";
		ULONG    			DebugLevel = 0xffffffff;
			HKEY        		insthkey;
			HKEY        		instscannerhkey;
			LPWSTR    			InstancesKey = INSTANCE_KEY;
		LPWSTR 				Altitude = SCANNER_ALTITUDE;
			ULONG    			Flags = 0x0;
			LPCWSTR				 DRIVER_LOCATION = (LPCWSTR)sysFilePath;

			InstallService(sysFilePath);

		if ((RegCreateKeyEx(
				HKEY_LOCAL_MACHINE,
				SERVICE_KEY,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, NULL,
				&phkey,
				&lpdwDisposition
			)) != ERROR_SUCCESS)
					{
					return 0;
					}

		   
		if ((RegSetValueEx(
				phkey,
				L"Description",
				0,
				REG_SZ,
				(CONST BYTE *)ServiceDescription,
				((static_cast<DWORD>(wcslen(ServiceDescription)) * sizeof(WCHAR)) + sizeof(WCHAR))  // size including NULL TERMINATOR
				)) != ERROR_SUCCESS)
					{
					RegCloseKey(phkey);
					return 0;
					}


			lpdwDisposition = 0;
		if ((RegSetValueEx(
				phkey,
				L"DebugLevel",
				0,
				REG_DWORD,
				(CONST BYTE *)&DebugLevel,
				sizeof(DebugLevel)
			)) != ERROR_SUCCESS)
					{
					RegCloseKey(phkey);
					return 0;
					}

		   
		if ((RegCreateKeyEx(
				phkey,
				L"Instances",
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,
				NULL,
				&insthkey,
				&lpdwDisposition
			)) != ERROR_SUCCESS)
					{
					RegCloseKey(phkey);
					return 0;
					}

		
		if ((RegSetValueEx(
				insthkey,
				L"DefaultInstance",
				0,
				REG_SZ,
				(CONST BYTE *)InstancesKey,
				((static_cast<DWORD>(wcslen(InstancesKey)) * sizeof(WCHAR)) + sizeof(WCHAR)) // size including NULL TERMINATOR
				))!= ERROR_SUCCESS) 
					{
					RegCloseKey(phkey);
					RegCloseKey(insthkey);
					return 0;
					} 

		    
			lpdwDisposition = 0;
		if ((RegCreateKeyEx(
				phkey,
				INSTANCE_LOCAL_KEY,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,
				NULL,
				&instscannerhkey,
				&lpdwDisposition
			)) != ERROR_SUCCESS)
					{
					RegCloseKey(phkey);
					RegCloseKey(insthkey);
					return 0;
					} 
			

		if ((RegSetValueEx(
				instscannerhkey,
				L"Altitude",
				0,
				REG_SZ,
				(CONST BYTE *)Altitude,
				((static_cast<DWORD>(wcslen(Altitude)) * sizeof(WCHAR)) + sizeof(WCHAR)) // size including NULL TERMINATOR
				))!= ERROR_SUCCESS ) 
					{
					RegCloseKey(phkey);
					RegCloseKey(insthkey);
					RegCloseKey(instscannerhkey);
					return 0;
					} 

		    
		if ((RegSetValueEx(
				instscannerhkey,
				L"Flags",
				0,
				REG_DWORD,
				(CONST BYTE *)&Flags,
				sizeof(Flags)
			)) != ERROR_SUCCESS)
					{
					RegCloseKey(phkey);
					RegCloseKey(insthkey);
					RegCloseKey(instscannerhkey);
					return 0;
					}

			RegCloseKey(phkey);
			RegCloseKey(insthkey);
			RegCloseKey(instscannerhkey);
			
			}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScannerLoad:: InstallScanner", 0, 0, true, SECONDLEVEL);
	}
    return 1;
}


/***************************************************************************
Function Name  : PauseDriverProtection
Description    : To register scanner before any operation so it will allow
Author Name    : lalit kumawat
S.R. No        :
Date           :26-12-2014
****************************************************************************/
bool CScannerLoad::PauseDriverProtection(ULONG processID)
{
	bool bIsScnnerPaused = false;
	__try
	{
		HANDLE hFileHandle = NULL;
		DWORD dwReturn;
		ULONG ulIndex = processID; //WLSRV_ID_ZERO;

		hFileHandle = CreateFile(csPipeScannerProt, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFileHandle)
		{
			SET_PROTECTION_STATE *pStruct = NULL;

			pStruct = (SET_PROTECTION_STATE*)malloc(sizeof(SET_PROTECTION_STATE));

			if (pStruct != NULL)
			{
				memset(pStruct, '\0', sizeof(SET_PROTECTION_STATE));
				memcpy(pStruct->SecureString, SECURE_STRING, sizeof(SECURE_STRING));
				pStruct->ProtectionState = ulIndex;

				if (DeviceIoControl(hFileHandle, IOCTL_SET_PROTECTION_STATE, pStruct, sizeof(SET_PROTECTION_STATE), pStruct, sizeof(SET_PROTECTION_STATE), &dwReturn, NULL))
				{
					CloseHandle(hFileHandle);
					bIsScnnerPaused = true;
				}
				
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### CScannerLoad:: Exception in PauseDriverProtection", 0, 0, true, SECONDLEVEL);
	}

	return bIsScnnerPaused;
}

/***************************************************************************
Function Name  : IsScannerDriverRunning
Description    : Function which handles structured exception handling
Author Name    : lalit kumawat
S.R. No        :
Date           :26-12-2014
****************************************************************************/
bool CScannerLoad::IsScannerDriverRunning()
{
	__try
	{
		return IsScannerDriverRunningSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CScannerLoad::IsScannerDriverRunning", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : IsScannerDriverRunningSEH
Description    : it use to check wheather scanner driver running or not
Author Name    : lalit kumawat
S.R. No        :
Date           :26-12-2014
****************************************************************************/
bool CScannerLoad::IsScannerDriverRunningSEH()
{
	bool bReturn = false;
	CString csServiceName  = L"";

	try
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hService = NULL;
		SERVICE_STATUS serviceStatus = { 0 };
		//LPCWSTR SERVICE_NAME = (LPCWSTR) csServiceName;
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

		if (!hSCManager)
		{
			return bReturn;
		}

		if (hSCManager != NULL)
		{
			hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
			if (hService != NULL)
			{
				if (!QueryServiceStatus(hService, &serviceStatus))
				{
					CloseServiceHandle(hSCManager);
					CloseServiceHandle(hService);
					return bReturn;
				}

				if (serviceStatus.dwCurrentState == SERVICE_RUNNING)
				{
					bReturn = true;
				}
			}
		}
		if (hSCManager != NULL)
		{
			CloseServiceHandle(hSCManager);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScannerLoad::IsSecure64DriverRunning", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************
Function Name  : ProtectFolder
Description    : To register scanner before any operation so it will allow
Author Name    : lalit kumawat
S.R. No        :
Date           :26-12-2014
****************************************************************************/
bool CScannerLoad::ProtectFolderSEH(LPTSTR lpszFolderPath)
{
	bool bReturn = false;
	__try
	{	
		bReturn = ProtectFolder(lpszFolderPath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CScannerLoad::ProtectFolderSEH", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : ProtectFolder
Description    : to protect any give folder, user will not delete or modify any file or folder after protection apply
Author Name    : lalit kumawat
S.R. No        :
Date           :26-12-2014
****************************************************************************/
bool CScannerLoad::ProtectFolder(LPTSTR lpszFolderPath)
{
	bool bReturn = false;
	OutputDebugString(lpszFolderPath);

	try
	{
		HANDLE hFileHandle = NULL;
		DWORD dwReturn = 0;
		hFileHandle = CreateFile(csPipeScannerProt, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFileHandle)
		{
			DWORD	dwStructSize = 0;
			PVOID	pstructProtectedFoler = NULL;
			PWSTR	pstrFileInfo = NULL;

			if (IsWow64())
			{
				// resolved by lalit kumawat
				//issue- this block executes for 64 bit os + 32 bit binary because at time of sending secureString to driver for folder protection , secureString size getting 48 bit instead of 65 bit
				dwStructSize = sizeof(SET_PROTECTED_FOLDER_NAME32B);
				pstructProtectedFoler = (SET_PROTECTED_FOLDER_NAME32B*)malloc(dwStructSize);

				if (pstructProtectedFoler != NULL)
			{
					memset(pstructProtectedFoler, '\0', sizeof(SET_PROTECTED_FOLDER_NAME32B));
					memcpy(((SET_PROTECTED_FOLDER_NAME32B*)pstructProtectedFoler)->SecureString, &SECURE_STRING, sizeof(SECURE_STRING));
					DWORD dwLegth = (DWORD)(_tcslen(lpszFolderPath) + 1); //+1 for carriage return.
					pstrFileInfo = (PWSTR)malloc(dwLegth * 2);
					memset(pstrFileInfo, '\0', dwLegth * 2 );
					memcpy(pstrFileInfo, lpszFolderPath, dwLegth * 2);
					((SET_PROTECTED_FOLDER_NAME32B*)pstructProtectedFoler)->pFileInfo = pstrFileInfo;
				}
			}
			else
			{
				dwStructSize = sizeof(SET_PROTECTED_FOLDER_NAME);
				pstructProtectedFoler = (SET_PROTECTED_FOLDER_NAME*)malloc(dwStructSize);

				if (pstructProtectedFoler != NULL)
				{
					memset(pstructProtectedFoler, '\0', sizeof(SET_PROTECTED_FOLDER_NAME));
					memcpy(((SET_PROTECTED_FOLDER_NAME*)pstructProtectedFoler)->SecureString, &SECURE_STRING, sizeof(SECURE_STRING));

					DWORD dwLegth = (DWORD)_tcslen(lpszFolderPath) + 1; //+1 for carriage return.
					pstrFileInfo = (PWSTR)malloc(dwLegth * 2);
					memset(pstrFileInfo, '\0', dwLegth * 2);
					memcpy(pstrFileInfo, lpszFolderPath, dwLegth * 2);
					((SET_PROTECTED_FOLDER_NAME*)pstructProtectedFoler)->pFileInfo = pstrFileInfo;
				}
			}

			if (dwStructSize > 0 && pstructProtectedFoler != NULL && pstrFileInfo!= NULL)
			{
				DWORD dwErrorCode = 0;
				if (!DeviceIoControl(hFileHandle, IOCTL_SET_PROTECTED_FOLDER, pstructProtectedFoler, dwStructSize, pstructProtectedFoler, dwStructSize, &dwReturn, NULL))
				{
					CString csLastError;
					csLastError.Format(L"%d", GetLastError());
					AddLogEntry(L"### DeviceIoControl Failed in CScannerLoad::ProtectFolder, Last Error : %s ", csLastError);
					goto FAILED;
				}
					CloseHandle(hFileHandle);
					bReturn = true;

				//Delete allocated memory of Folder path
				delete[]pstrFileInfo;
				pstrFileInfo = NULL;

				//Deallocation of structure
				delete[]pstructProtectedFoler;
				pstructProtectedFoler = NULL;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScannerLoad::ProtectFolder", 0, 0, true, SECONDLEVEL);
	}
FAILED:
	return bReturn;;
}

/***************************************************************************
Function Name  : IsWow64
Description    : this function return true if we run 32 bit exe on 64 bit os
Author Name    : Lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
bool CScannerLoad::IsWow64()
{
	BOOL bReturn = FALSE;
	__try
	{
		typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
		LPFN_ISWOW64PROCESS		IsWow64Process = NULL;
		OSVERSIONINFO	OSVI = { 0 };
		OSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&OSVI);
		IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
		if (!IsWow64Process)
			return false;

		IsWow64Process(GetCurrentProcess(), &bReturn);
		
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CScannerLoad::IsWow64", 0, 0, true, SECONDLEVEL);
	}

	return bReturn?true:false;
}

/***************************************************************************
Function Name  : GetPathToDriveVolume
Description    : it use to convert normal folder path to DosDrive path
Author Name    : Lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
CString CScannerLoad::GetPathToDriveVolume(CString path)
{
	CString csSelectedDrive = L"";
	CString csSelectedPathExcludingDrive = L"";
	CString csCompletedWithDosDrive = L"";
	TCHAR DosDriveVolume[MAX_PATH];
	try
	{
		int iPos = path.Find(L':');
		TCHAR FolderPath[MAX_PATH];
		csSelectedDrive = path.Left(2);
		QueryDosDevice(csSelectedDrive, DosDriveVolume, MAX_PATH);
		csSelectedPathExcludingDrive = path.Right(path.GetLength() - (iPos + 1));
		swprintf_s(FolderPath, _countof(FolderPath), L"%s%s", DosDriveVolume, csSelectedPathExcludingDrive);
		csCompletedWithDosDrive = FolderPath;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetPathToDriveVolume", 0, 0, true, SECONDLEVEL);
	}

	return csCompletedWithDosDrive;
}

/***************************************************************************
Function Name  : LockFolder
Description    : It will lock folder.
Author Name    : Neha Gharge
S.R. No        :
Date           : 15 oct,2015
****************************************************************************/
bool CScannerLoad::LockFolder(LPTSTR lpszFolderPath)
{
	bool bReturn = false;
	OutputDebugString(lpszFolderPath);

	try
	{
		HANDLE hFileHandle = NULL;
		DWORD dwReturn = 0;
		hFileHandle = CreateFile(csPipeScannerProt, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFileHandle)
		{
			DWORD	dwStructSize = 0;
			PVOID	pstructProtectedFoler = NULL;
			PWSTR	pstrFileInfo = NULL;

			if (IsWow64())
			{
				// resolved by lalit kumawat
				//issue- this block executes for 64 bit os + 32 bit binary because at time of sending secureString to driver for folder protection , secureString size getting 48 bit instead of 65 bit
				dwStructSize = sizeof(SET_PROTECTED_FOLDER_NAME32B);
				pstructProtectedFoler = (SET_PROTECTED_FOLDER_NAME32B*)malloc(dwStructSize);

				if (pstructProtectedFoler != NULL)
				{
					memset(pstructProtectedFoler, '\0', sizeof(SET_PROTECTED_FOLDER_NAME32B));
					memcpy(((SET_PROTECTED_FOLDER_NAME32B*)pstructProtectedFoler)->SecureString, &SECURE_STRING, sizeof(SECURE_STRING));
					DWORD dwLegth = (DWORD)(_tcslen(lpszFolderPath) + 1); //+1 for carriage return.
					pstrFileInfo = (PWSTR)malloc(dwLegth * 2);
					memset(pstrFileInfo, '\0', dwLegth * 2);
					memcpy(pstrFileInfo, lpszFolderPath, dwLegth * 2);
					((SET_PROTECTED_FOLDER_NAME32B*)pstructProtectedFoler)->pFileInfo = pstrFileInfo;
				}
				}
				else
				{
				dwStructSize = sizeof(SET_PROTECTED_FOLDER_NAME);
				pstructProtectedFoler = (SET_PROTECTED_FOLDER_NAME*)malloc(dwStructSize);

				if (pstructProtectedFoler != NULL)
				{
					memset(pstructProtectedFoler, '\0', sizeof(SET_PROTECTED_FOLDER_NAME));
					memcpy(((SET_PROTECTED_FOLDER_NAME*)pstructProtectedFoler)->SecureString, &SECURE_STRING, sizeof(SECURE_STRING));

					DWORD dwLegth = (DWORD)_tcslen(lpszFolderPath) + 1; //+1 for carriage return.
					pstrFileInfo = (PWSTR)malloc(dwLegth * 2);
					memset(pstrFileInfo, '\0', dwLegth * 2);
					memcpy(pstrFileInfo, lpszFolderPath, dwLegth * 2);
					((SET_PROTECTED_FOLDER_NAME*)pstructProtectedFoler)->pFileInfo = pstrFileInfo;
				}
			}

			if (dwStructSize > 0 && pstructProtectedFoler != NULL && pstrFileInfo != NULL)
			{
				DWORD dwErrorCode = 0;
				if (!DeviceIoControl(hFileHandle, IOCTL_SET_BLOCKED_FOLDER, pstructProtectedFoler, dwStructSize, pstructProtectedFoler, dwStructSize, &dwReturn, NULL))
				{
					CString csLastError;
					csLastError.Format(L"%d", GetLastError());
					//AddLogEntry(L"### DeviceIoControl Failed in CScannerLoad::ProtectFolder, Last Error : %s ", csLastError);
					goto FAILED;
				}
				CloseHandle(hFileHandle);
				bReturn = true;

				//Delete allocated memory of Folder path
				delete[]pstrFileInfo;
				pstrFileInfo = NULL;

				//Deallocation of structure
				delete[]pstructProtectedFoler;
				pstructProtectedFoler = NULL;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScannerLoad::LockFolder", 0, 0, true, SECONDLEVEL);
	}
FAILED:
	return bReturn;;
}

/***************************************************************************
Function Name  : SetAutorunSettings
Description    : Function to set following settings in autorun scanner.
				 IsAutoRunBlocked;
				 IsUSBExecBlocked;
				 IsUSBWriteBlocked
Author Name    : Ram Shelke
S.R. No        :
Date           :09-1-2017
****************************************************************************/
bool CScannerLoad::SetAutorunSettings(bool IsAutoRunBlocked, bool IsUSBExecBlocked, bool IsUSBWriteBlocked)
{
	bool bReturn = false;
	__try
	{
		HANDLE hFileHandle = NULL;
		DWORD dwReturn;

		hFileHandle = CreateFile(csPipeScannerProt, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (!hFileHandle)
		{
			return bReturn;
		}

		if (hFileHandle)
		{
			AUTORUN_BLOCK_DATA *pStruct = NULL;

			pStruct = (AUTORUN_BLOCK_DATA*)malloc(sizeof(AUTORUN_BLOCK_DATA));

			if (pStruct != NULL)
			{
				memset(pStruct, '\0', sizeof(AUTORUN_BLOCK_DATA));
				memcpy(pStruct->SecureString, SECURE_STRING, sizeof(SECURE_STRING));
				pStruct->IsAutoRunBlocked = IsAutoRunBlocked;
				pStruct->IsUSBExecBlocked = IsUSBExecBlocked;
				pStruct->IsUSBWriteBlocked = IsUSBWriteBlocked;

				if (DeviceIoControl(hFileHandle, IOCTL_SET_AUTORUN_BLOCK, pStruct, sizeof(AUTORUN_BLOCK_DATA), pStruct, sizeof(AUTORUN_BLOCK_DATA), &dwReturn, NULL))
				{
					CloseHandle(hFileHandle);
					bReturn = true;
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CScannerLoad::SetAutorunSettings", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
