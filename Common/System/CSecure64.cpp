/********************************************************************************************************** 
   Program Name          : PSecure64.cpp
   Description           : Defines the functionality require to manage secure64/xpproc32 for OnAccess protectio.
   Author Name           : Lalit                                                                               
   Date Of Creation      : 26-12-2014
   Version No            : 
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description
***********************************************************************************************************/

//#include "public.h"
#include "stdafx.h"
#include "CSecure64.h"
#include "DriverConstants.h"
#include "WardwizOSVersion.h"


#define DRIVER_INSTALLATION_FAILED		0
#define DRIVER_SUCCESSFULLY_INSTALLED	1
#define DRIVER_ALREADY_INSTALLED		2

#define DRIVER_START_FAILED				0
#define DRIVER_SUCCESSFULLY_STARTED		1

#define DRIVER_UNINSTALL_FAILED			0
#define DRIVER_UNINSTALL_SUCCESS		1

#define DRIVER_REMOVAL_FAILED			0
#define DRIVER_REMOVAL_SUCCESS			1

DWORD startType = SERVICE_AUTO_START;			

/***************************************************************************
  Function Name  : RegisterProcessId
  Description    : Function to handle structured exception hadling
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           : 26-12-2014
****************************************************************************/
bool CSecure64::RegisterProcessId(ULONG processID)
{
	__try
	{
		return RegisterProcessIdSEH(processID);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### CSecure64::Exception in CSecure64::RegisterProcessId", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : RegisterProcessIdSEH
Description    : it use to register secure64/xpproc32 ,so it will allow to start/stop serices 
Author Name    : lalit kumawat
S.R. No        :
Date           : 26-12-2014
****************************************************************************/
bool CSecure64::RegisterProcessIdSEH(ULONG processID)
{
	bool bIsRegistrationSuccess		=	 false;
	CWardWizOSversion	objGetOSVersion;
	int iOsType	 =		0;

	try
	{
		iOsType = objGetOSVersion.DetectClientOSVersion();
		CString csdServicePath = L"";
		// resolved by lalit issue:- secure64 bit registry key not getting deleted during uninstallation ,
		//process protection not works in xp64 bit
		if (iOsType == WINOS_XP64)
		{
			//No Need to install Secure64 drivers in case of WinXP 64 bit.
			return bIsRegistrationSuccess;
		}
		
		if (iOsType == WINOS_XP)
		{
			csdServicePath = csPipeXPProtection;
		}
		else
		{
			csdServicePath = csPipeSecure64Prot;
		}

		HANDLE hFileHandle = NULL;
		DWORD dwReturn;
		ULONG ulIndex = processID;  //WLSRV_ID_ZERO;
		LPCWSTR SERVICE_PATH = (LPCWSTR)csdServicePath;
		hFileHandle = CreateFile(SERVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFileHandle == INVALID_HANDLE_VALUE)
		{
			if (!IsSecure64DriverRunning(iOsType))
			{
				if (StartServiceLocal(iOsType) == 0)
				{
					AddLogEntry(L"### In CSecure64::RegisterProcessId, Invalid handle for Service %s", csdServicePath, 0, true, SECONDLEVEL);
					return false;
				}

				hFileHandle = CreateFile(SERVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			}
		}

		if (!hFileHandle)
		{
			return false;
		}

		REGISTER_PROCESS_ID *pStruct = NULL;

		pStruct = (REGISTER_PROCESS_ID*)malloc(sizeof(REGISTER_PROCESS_ID));

		if (pStruct != NULL)
		{
			memset(pStruct, '\0', sizeof(REGISTER_PROCESS_ID));
			memcpy(pStruct->SecureString, SECURE_STRING, sizeof(SECURE_STRING));
			pStruct->ProcessIdCode = ulIndex;

			if (DeviceIoControl(hFileHandle, IOCTL_REGISTER_PROCESSID, pStruct, sizeof(REGISTER_PROCESS_ID), pStruct, sizeof(REGISTER_PROCESS_ID), &dwReturn, NULL))
			{
				bIsRegistrationSuccess = true;//Return a success value
			}
		}

		delete[]pStruct;
		pStruct = NULL;

		CloseHandle(hFileHandle);
		hFileHandle = NULL;
	}
	catch (...)
	{
		AddLogEntry(L"### CSecure64::Exception in CSecure64::RegisterProcessId", 0, 0, true, SECONDLEVEL);
	}
	return bIsRegistrationSuccess;
}

/***************************************************************************
Function Name  : InstallService
Description    : it use to install secure64/xpproc32.sys
Author Name    : Lalit
S.R. No        :
Date           : 26-12-2014
****************************************************************************/
int CSecure64::InstallService(CString sysFilePath, int iOsType)
{
	int iReturnStatus = DRIVER_INSTALLATION_FAILED;
	CString csServiceName = L"";

	if (iOsType == 5)
	{
		csServiceName = csDXpProcServiceName;
	}
	else
	{
		csServiceName = csDSecure64ServiceName;
	}
	try
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hService = NULL;
		SERVICE_STATUS ss = { 0 };
		LPCWSTR  DRIVER_LOCATION = (LPCWSTR)sysFilePath;
		LPCWSTR SERVICE_NAME = (LPCWSTR)csServiceName;

		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (!hSCManager)
		{
			return iReturnStatus;
		}

		hService = CreateServiceW(hSCManager, SERVICE_NAME, SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP, SERVICE_KERNEL_DRIVER, startType, SERVICE_ERROR_IGNORE, DRIVER_LOCATION, NULL, NULL, NULL, NULL, NULL);
		if (!hService)
		{
			hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP);
		}
		else
		{
			iReturnStatus = DRIVER_SUCCESSFULLY_INSTALLED;
			AddLogEntry(L">>> CSecure64:: DRIVER_SUCCESSFULLY_INSTALLED", 0, 0, true, ZEROLEVEL);
		}

		if (hService)
		{
			iReturnStatus = DRIVER_ALREADY_INSTALLED;
		}

		if (hSCManager)
		{
			CloseServiceHandle(hSCManager);
		}
	}
	catch (...)
	{

		AddLogEntry(L"### CSecure64:: Exception in InstallService", 0, 0, true, SECONDLEVEL);
	}
	return iReturnStatus;
}

/***************************************************************************
  Function Name  : StartServiceLocal
  Description    : Function which handles Structured exception handling.
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           : 26-12-2014
****************************************************************************/
int CSecure64::StartServiceLocal(int iOsType)
{
	__try
	{
		return StartServiceLocalSEH(iOsType);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSecure64::StartServiceLocal", 0, 0, true, SECONDLEVEL);
	}
	return DRIVER_START_FAILED;
}

/***************************************************************************
Function Name  : StartServiceLocalSEH
Description    : it use to start services secure64/xpproc32.sys
Author Name    : lalit kumawat
S.R. No        :
Date           : 26-12-2014
****************************************************************************/
int CSecure64::StartServiceLocalSEH(int iOsType)
{
	int iReturnStatus = DRIVER_START_FAILED;
	CString csServiceName , csLastError;

	if (iOsType == 5)
	{
		csServiceName = csDXpProcServiceName;
	}
	else
	{
		csServiceName = csDSecure64ServiceName;
	}

	try
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hService = NULL;
		SERVICE_STATUS ss = { 0 };
		LPCWSTR SERVICE_NAME = (LPCWSTR)csServiceName;

		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (!hSCManager)
		{
			return iReturnStatus;
		}

		hService = OpenServiceW(hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
		if (!hService)
		{
			return iReturnStatus;
		}

		if (!StartServiceW(hService, 0, NULL))
		{
			DWORD dwErrorCode = 0;
			dwErrorCode = GetLastError();
			csLastError.Format(L"%d", dwErrorCode);
			AddLogEntry(L"### CSecure64:: Error during CSecure64 service start with error %s", csLastError, 0, true, SECONDLEVEL);
			CloseServiceHandle(hSCManager);
			return iReturnStatus;
		}

		iReturnStatus = DRIVER_SUCCESSFULLY_STARTED;

		//else
		//{
		//	DWORD dwErrorCode = 0;
		//	dwErrorCode = GetLastError();
		//	AddLogEntry(L"### CSecure64:: Error during CSecure64 service start", 0, 0, true, SECONDLEVEL);
		//}

		if (hSCManager)
		{
			CloseServiceHandle(hSCManager);
		}

		if (hSCManager && hService)
		{
			RegisterProcessId(0);
		}
	}
	catch (...)
	{

		AddLogEntry(L"### CSecure64::Exception in StartServiceLocal", 0, 0, true, SECONDLEVEL);
	}
	return iReturnStatus;
}

/***************************************************************************
	  Function Name  : RemoveService
	  Description    : Function which handles strucutured exception handling.
	  Author Name    : lalit kumawat
	  S.R. No        : 
	  Date           : 26-12-2014
****************************************************************************/
int CSecure64::RemoveService(int iOsType)
{
	__try
	{
		return RemoveServiceSEH(iOsType);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSecure64::RemoveService", 0, 0, true, SECONDLEVEL);
	}
	return DRIVER_REMOVAL_FAILED;
}

/***************************************************************************
	Function Name  : RemoveService
	Description    : it use to remove services secure64/xpproc32.sys
	Author Name    : lalit kumawat
	S.R. No        :
	Date           : 26-12-2014
****************************************************************************/
int CSecure64::RemoveServiceSEH(int iOsType)
{
	int iReturnStatus = DRIVER_REMOVAL_FAILED;
	CString csServiceName;

	if (iOsType == 5)
	{
		csServiceName = csDXpProcServiceName;
	}
	else
	{
		csServiceName = csDSecure64ServiceName;
	}

	try
	{
		RegisterProcessId(0);

		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hService = NULL;
		SERVICE_STATUS ss = { 0 };
		LPCWSTR SERVICE_NAME = (LPCWSTR)csServiceName;
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (!hSCManager)
		{
			return iReturnStatus;
		}

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

		if (hSCManager)
		{
			CloseServiceHandle(hSCManager);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CSecure64:: Exception in RemoveService", 0, 0, true, SECONDLEVEL);
	}
	return iReturnStatus;
}

/***************************************************************************
	Function Name  : PauseDriverProtection
	Description    : Function which handles structured exception handling.
	Author Name    : lalit kumawat
	S.R. No        :
	Date           : 26-12-2014
****************************************************************************/
bool CSecure64::PauseDriverProtection(ULONG processID)
{
	__try
	{
		return PauseDriverProtectionSEH(processID);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSecure64::PauseDriverProtection", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
	Function Name  : PauseDriverProtection
	Description    : it use to register secure64/xpproc32 ,so it will allow to start/stop serices
	Author Name    : lalit kumawat
	S.R. No        :
	Date           : 26-12-2014
****************************************************************************/
bool CSecure64::PauseDriverProtectionSEH(ULONG processID)
{
	bool	bIsPaused = false;
	try
	{
		int iOsType				=	 0;
		CString csdServicePath  =	L"";
		CWardWizOSversion		 objGetOSVersion;

		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == 5)
		{
			csdServicePath = csPipeXPProtection;
		}
		else
		{
			csdServicePath = csPipeSecure64Prot;
		}

		HANDLE hFileHandle = NULL;
		DWORD dwReturn = 0;
		ULONG ulIndex = processID;  //WLSRV_ID_ZERO;
		LPCWSTR SERVICE_PATH = (LPCWSTR)csdServicePath;
		hFileHandle = CreateFile(SERVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (!hFileHandle)
		{
			return bIsPaused;
		}

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
					hFileHandle = NULL;
					bIsPaused = true;
				}
			}

		if (hFileHandle)
		{
			CloseHandle(hFileHandle);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CSecure64::Exception in PauseDriverProtection", 0, 0, true, SECONDLEVEL);
	}

	return bIsPaused;
}

/***************************************************************************
Function Name  : IsSecure64DriverRunning
Description    : it use to check whether secure64 service runing or not
Author Name    : lalit kumawat
S.R. No        :
Date           : 
****************************************************************************/
bool CSecure64::IsSecure64DriverRunning(int iOsType)
{
	__try
	{
		return IsSecure64DriverRunningSEH(iOsType);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSecure64::IsSecure64DriverRunning", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : IsSecure64DriverRunning
Description    : it use to check whether secure64 service runing or not
Author Name    : lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
bool CSecure64::IsSecure64DriverRunningSEH(int iOsType)
{
	bool bReturn = false;
	CString csServiceName = L"";

	if (iOsType == 5)
	{
		csServiceName = csDXpProcServiceName;
	}
	else
	{
		csServiceName = csDSecure64ServiceName;
	}

	try
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hService = NULL;
		SERVICE_STATUS serviceStatus = { 0 };
		LPCWSTR SERVICE_NAME = (LPCWSTR)csServiceName;
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

		if (hSCManager == NULL)
		{
			return bReturn;
		}

		if (hSCManager != NULL)
		{
			hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
			if (hService != NULL)
			{
				if (!QueryServiceStatus(hService, &serviceStatus))  // return nonzero if success
				{
					CloseServiceHandle(hSCManager);
					CloseServiceHandle(hService);
					bReturn = false;
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
		AddLogEntry(L"### Exception in CSecure64::IsSecure64DriverRunning", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}


/***************************************************************************
Function Name  : RegisterProcessId
Description    : Function which handles structured exception handling.
Author Name    : lalit kumawat
S.R. No        :
Date           : 26-12-2014
****************************************************************************/
bool CSecure64::RegisterProcessIdAndCode(ULONG processID, ULONG processIDCode)
{
	__try
	{
		return RegisterProcessIdAndCodeSEH(processID, processIDCode);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSecure64::IsSecure64DriverRunning", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name	: RegisterProcessId
Description		: it use to register secure64/xpproc32 ,so it will allow to start/stop serices
Author Name		: lalit kumawat
S.R. No			:
Date			: 26-12-2014
****************************************************************************/
bool CSecure64::RegisterProcessIdAndCodeSEH(ULONG processID, ULONG processIDCode)
{
	bool bIsProcessProtected = false;
	CWardWizOSversion  objGetOSVersion;
	int iOsType  =  0;

	try
	{
		CString csdServicePath = L"";
		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == 5)
		{
			csdServicePath = csPipeXPProtection;
		}
		else
		{
			csdServicePath = csPipeSecure64Prot;
		}

		HANDLE hFileHandle = NULL;
		DWORD dwReturn = 0;
		ULONG ulIndex = processID;  //WLSRV_ID_ZERO;
		ULONG ulIndexCode = processIDCode;

		LPCWSTR SERVICE_PATH = (LPCWSTR)csdServicePath;
		hFileHandle = CreateFile(SERVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (!hFileHandle)
		{
			return bIsProcessProtected;
		}

		if (hFileHandle)
		{
			REGISTER_PROCESS_ID_EX *pStruct = NULL;

			pStruct = (REGISTER_PROCESS_ID_EX*)malloc(sizeof(REGISTER_PROCESS_ID_EX));
			if (pStruct != NULL)
			{
				memset(pStruct, '\0', sizeof(REGISTER_PROCESS_ID_EX));
				memcpy(pStruct->SecureString, SECURE_STRING, sizeof(SECURE_STRING));
				pStruct->ProcessIdCode = ulIndex;
				pStruct->ProcessId = ulIndexCode;

				if (DeviceIoControl(hFileHandle, IOCTL_REGISTER_PROCESSID_EX, pStruct, sizeof(REGISTER_PROCESS_ID_EX), pStruct, sizeof(REGISTER_PROCESS_ID_EX), &dwReturn, NULL))
				{
					CloseHandle(hFileHandle);
					hFileHandle = NULL;
					bIsProcessProtected = true;
				}

			}
		}

		if (hFileHandle)
		{
			CloseHandle(hFileHandle);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSecure64::RegisterProcessIdAndCodeSEH", 0, 0, true, SECONDLEVEL);
		bIsProcessProtected = false;
	}
	return bIsProcessProtected;
}