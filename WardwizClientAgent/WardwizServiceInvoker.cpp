#include "stdafx.h"
#include "WardwizServiceInvoker.h"
#include "PipeConstants.h"

enum class _USBREGISTRYVALUE :int
{
	DISABLEUSB = 4,
	ENABLEUSB = 3
} USBREGISTRYVALUE;

// Handle to the Named Pipe
HANDLE m_hPipe;

CWardwizServiceInvoker::CWardwizServiceInvoker()
{
}


CWardwizServiceInvoker::~CWardwizServiceInvoker()
{
}
bool CWardwizServiceInvoker::InvokeSettingsUpdaterFromService()
{
	CWardwizSettingsUpdater objSettings;
    bool bRetVal = objSettings.UpdateWardwizSettings();
	return bRetVal;
}

bool  CWardwizServiceInvoker::InvokeScanFromService(CStringArray &csaAllScanPaths, SCANTYPE eScanType)
{
	CWardwizScanner objScanner;
	CStringArray csFilesForScanning;
	bool bScanComplete = false;

	switch (eScanType)
	{
	case QUICKSCAN:
		bScanComplete = objScanner.StartQuickScan();
		break;
	case CUSTOMSCAN:
		bScanComplete = objScanner.StartCustomScan(csaAllScanPaths);
		break;
	case FULLSCAN:
		break;
	}
    
	m_iThreatsFoundCount = objScanner.m_dwVirusFoundCount;
	m_iTotalFileCount = objScanner.m_iTotalFileCount;
	m_bThreatDetected = objScanner.m_bThreatDetected;
	m_csVirusName = objScanner.m_csVirusName;
	
	return bScanComplete;
}
//bool Connect(void)
//{
//
//	int nTryCount = 0;
//	bool bConnected = false;
//	bool m_bRetryConnection = true;
//	DWORD m_dwNoofRetry = 2;
//	//__try{
//		do{
//			m_hPipe = CreateFile(
//				SERVICE_SERVER,					// __in          LPCTSTR lpFileName,
//				GENERIC_WRITE | GENERIC_READ,	// __in          DWORD dwDesiredAccess,
//				0,								// __in          DWORD dwShareMode,
//				NULL,							// __in          LPSECURITY_ATTRIBUTES lpSecurityAttributes,
//				OPEN_EXISTING,					// __in          DWORD dwCreationDisposition,
//				FILE_ATTRIBUTE_NORMAL,			//				 DWORD dwFlagsAndAttributes,
//				NULL							// __in          HANDLE hTemplateFile
//				);
//
//			if (m_hPipe == INVALID_HANDLE_VALUE)
//			{
//				if (m_bRetryConnection)
//				{
//					if (nTryCount >= static_cast<int>(m_dwNoofRetry))// MAX_CONNECT_RETRY)
//					{
//						m_bRetryConnection = false;
//						break;
//					}
//					nTryCount++;
//					Sleep(MAX_CONNECT_TIMEOUT);
//				}
//			}
//			else
//			{
//				bConnected = true;
//				m_bRetryConnection = false;
//			}
//		} while ((!bConnected) && m_bRetryConnection);
//	//}
//	//__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
//	//{
//	//	bConnected = false;
//	//}
//	return bConnected;
//}
//void Close()
//{
//
//		if (m_hPipe != INVALID_HANDLE_VALUE)
//		{
//			FlushFileBuffers(m_hPipe);
//			DisconnectNamedPipe(m_hPipe);
//			CloseHandle(m_hPipe);
//			m_hPipe = INVALID_HANDLE_VALUE;
//		}
//}
//
//
//bool SendDataToService(LPVOID lpMaxData, DWORD dwSize)
//{
//	BOOL bReturn = FALSE;
//		if (m_hPipe == INVALID_HANDLE_VALUE)
//		{
//			if (false == Connect())
//			{
//				return false;
//			}
//		}
//
//		DWORD nWritten = 0;
//
//		bReturn = WriteFile(
//			m_hPipe,				// __in          HANDLE hFile,
//			(LPCVOID)lpMaxData,		// __in          LPCVOID lpBuffer,
//			dwSize,					// __in          DWORD nNumberOfBytesToWrite,
//			&nWritten,				// __out         LPDWORD lpNumberOfBytesWritten,
//			NULL					// __in          LPOVERLAPPED lpOverlapped
//			);
//		if (!bReturn)
//		{
//			Close();
//		}
//
////	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
//	//{
//
//	//}
//	return (bReturn == FALSE ? false : true);
//}
//
///**********************************************************************************************************
//*  Function Name  :	InvokeScanFromService
//*  Description    :	Used named pipe to give signal to service to start scanning .
//*  Author Name    : 
//*  SR_NO		  :
//*  Date           : 
//**********************************************************************************************************/
//bool CWardwizServiceInvoker::InvokeScanFromService(CStringArray &csaAllScanPaths)
//{
//	try
//	{
//
//	}
//	catch (...)
//	{
//		//AddLogEntry(L"### Exception in InvokeScanFromService", 0, 0, true, SECONDLEVEL);
//	}
//	return true;
//}
void CWardwizServiceInvoker::SetProcessNameAndID(DWORD processID, CString &szProcessDetails)
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	// Get a handle to the process.
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

	// Get the process name.
	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
			&cbNeeded))
		{
			GetModuleBaseNameW(hProcess, hMod, szProcessName,
				sizeof(szProcessName) / sizeof(TCHAR));
		}
	}

	TCHAR szTmp[128];
	//swprintf_s(szProcessDetails,sizeof(szProcessDetails), L"%d\;%s", processID, szProcessName);
	swprintf_s(szTmp, L"%d", processID);

	szProcessDetails.SetString(szTmp);
	szProcessDetails.Append(L";");
	szProcessDetails.Append(szProcessName);
	//swprintf_s(szProcessDetails,L"%s:%s",szTmp,szProcessName);
	// Release the handle to the process.
	CloseHandle(hProcess);

	return;
}


// Gets a list of processes and returns a count.
int CWardwizServiceInvoker::GetRunningProcessList(CStringArray &csaProcessList)
{
	// Get the list of process identifiers.

	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		return 1;
	}


	// Calculate how many process identifiers were returned.

	cProcesses = cbNeeded / sizeof(DWORD);

	// Print the name and process identifier for each process.
	TCHAR strProcessDetails[4096] = { 0 };
	CString strProcData;

	for (i = 0; i < cProcesses; i++)
	{
		if (aProcesses[i] != 0)
		{
			SetProcessNameAndID(aProcesses[i], strProcData);
			//_tcscat_s(strProcessDetails, L"#");
			//_tcscat_s(strProcessDetails, strProcData);
			csaProcessList.Add(strProcData);
		}
	}

	return cProcesses;
}

// Gets a list of attached devices
int CWardwizServiceInvoker::GetAttachedDeviceList(const std::string &refcstrRootDirectory, const std::string &refcstrExtension)
{
	int             iCount = 0;
	//std::string     strFilePath;          // Filepath
	//std::string     strPattern;           // Pattern
	//HANDLE          hFile;                // Handle to file
	//WIN32_FIND_DATA FileInformation;      // File information


	//strPattern = refcstrRootDirectory + "\\*." + refcstrExtension;
	//hFile = ::FindFirstFile((LPCWSTR)strPattern.c_str(), &FileInformation);
	//if (hFile != INVALID_HANDLE_VALUE)
	//{
	//	do
	//	{
	//		if (FileInformation.cFileName[0] != '.')
	//		{
	//			strFilePath.erase();
	//			strFilePath.erase();
	//			strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

	//			if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	//			{
	//				// Search subdirectory
	//				int iRC = GetAttachedDeviceList(strFilePath, refcstrExtension);
	//				if (iRC != -1)
	//					iCount += iRC;
	//				else
	//					return -1;
	//			}
	//			else
	//			{
	//				// Increase counter
	//				++iCount;
	//			}
	//		}
	//	} while (::FindNextFile(hFile, &FileInformation) == TRUE);

	//	// Close handle
	//	::FindClose(hFile);
	//}

	return iCount;
}

DWORD CWardwizServiceInvoker::UpdateUSBPORTsPolicy(BOOL bEnableDisable)
{
	HKEY hKeyUSB;
	DWORD dwError;
	DWORD dwValue(0);

	if (bEnableDisable)
		dwValue = static_cast<int>(_USBREGISTRYVALUE::ENABLEUSB);
	else
		dwValue = static_cast<int>(_USBREGISTRYVALUE::DISABLEUSB);

	dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\UsbStor", 0, KEY_ALL_ACCESS | KEY_WRITE, &hKeyUSB);
	if (ERROR_SUCCESS == dwError)
	{
		// Got a valid handle to USBSTOR..
		// Now set the required value to enable or disable USB...
		dwError = ::RegSetValueEx(hKeyUSB, L"Start", 0, REG_DWORD, reinterpret_cast<BYTE *>(&dwValue), sizeof(dwValue));
		if (ERROR_SUCCESS == dwError)
		{
			RegCloseKey(hKeyUSB);
			return dwError;
		}
	}
	else
	{
		RegCloseKey(hKeyUSB);
		return dwError;
	}

	return ERROR_SUCCESS;
}

DWORD CWardwizServiceInvoker::UpdateCDROMsPolicy(BOOL bEnableDisable)
{
	HKEY hKeyUSB;
	DWORD dwError;
	DWORD dwValue(0);

	if (bEnableDisable)
		dwValue = static_cast<int>(_USBREGISTRYVALUE::ENABLEUSB);
	else
		dwValue = static_cast<int>(_USBREGISTRYVALUE::DISABLEUSB);

	dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\UsbStor", 0, KEY_ALL_ACCESS | KEY_WRITE, &hKeyUSB);
	if (ERROR_SUCCESS == dwError)
	{
		// Got a valid handle to USBSTOR..
		// Now set the required value to enable or disable USB...
		dwError = ::RegSetValueEx(hKeyUSB, L"Start", 0, REG_DWORD, reinterpret_cast<BYTE *>(&dwValue), sizeof(dwValue));
		if (ERROR_SUCCESS == dwError)
		{
			RegCloseKey(hKeyUSB);
			return dwError;
		}
	}
	else
	{
		RegCloseKey(hKeyUSB);
		return dwError;
	}

	return ERROR_SUCCESS;
}