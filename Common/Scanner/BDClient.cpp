/**********************************************************************************************************
Program Name			: BDClient.cpp
Description				: this file contains implementation of Bitdefender scan engine with below functionalities
*							1) Scan
*							2) Repair
*							3) Smart Scan
Author Name				: Ramkrushna Shelke
Date Of Creation		: 28 Aug 2019
Version No				: 3.6.0.3
Special Logic Used		:

Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "BDClient.h"

/***********************************************************************************************
Function Name  : CBDClient
Description    : Const'
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
CBDClient::CBDClient()
{
	m_pModule = NULL;
	m_pfnThreatScanner_InitializeEx = NULL;
	m_pfnThreatScanner_CreateInstance = NULL;
	m_pfnThreatScanner_SetIntOption = NULL;
	m_pfnThreatScanner_SetStringOption = NULL;
	m_pfnThreatScanner_SetScanCallback2 = NULL;
	m_pfnThreatScanner_SetEnginesUnloadCallback = NULL;
	m_pfnThreatScanner_ScanPath = NULL;
	m_pfnThreatScanner_SetPasswordCallback = NULL;
	m_pfnThreatScanner_SetExtCallback = NULL;
	m_pfnThreatScanner_GetOption = NULL;
	m_pfnThreatScanner_GetScanStatistics = NULL;
	m_pfnThreatScanner_ScanObject = NULL;
	m_pfnThreatScanner_DestroyInstance = NULL;
	m_pfnThreatScanner_Uninitialize = NULL;
	m_pfnThreatScanner_ScanMemoryEx = NULL;
	m_pfnThreatScanner_InitializeMemoryScan = NULL;
	m_pfnThreatScanner_UninitializeMemoryScan = NULL;
	m_pfnThreatScanner_ScanObjectByHandle = NULL;
	m_pfnThreatScanner_GetSmartDBFunctions = NULL;
	m_pfnThreatScanner_GetKeys = NULL;
	m_bIsHeuScan = false;
	ReadHeuristicScanStatus();
}

/***********************************************************************************************
Function Name  : ~CBDClient
Description    : Dest'
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
CBDClient::~CBDClient()
{
	BD_uninitialize();
}

/***********************************************************************************************
Function Name  : BD_initialize
Description    : Function to initialize Bitdefender scan Engine
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
bool CBDClient::BD_initialize()
{
	bool bReturn = false;

	if (m_pModule == NULL)	
	{
		if (!LoadRequiredModules())
			return false;
	}

	int status = 0;
	if (m_pfnThreatScanner_InitializeEx)
	{
		InitializeParams p;
		memset(&p, 0, sizeof(InitializeParams));
		p.nStructSize = sizeof(InitializeParams);

		// This disables Bitdefender’s builtin exception handler on Windows
		p.nInitFlags = INIT_FLAG_NO_EXCEPTION_HANDLER;
		int status = m_pfnThreatScanner_InitializeEx(_TAV("."), _T("8F3803303909201940B5"), NULL, &p);
		if (status != SCANAPI_SUCCESS)
		{
			AddLogEntryEx(SECONDLEVEL, L"Error initializing scanner. Error status is [%x]", status);
			return false;
		}
	}

	//
	// Initialize the scanner. If multithreading, use one instance per thread ! (it will not multiply the memory usage)
	//
	if (!m_pfnThreatScanner_CreateInstance)
	{
		return false;
	}

	status = m_pfnThreatScanner_CreateInstance(&m_pscanner);
	if (status != SCANAPI_SUCCESS)
	{
		status = m_pfnThreatScanner_CreateInstance(&m_pscanner);
		if (status != SCANAPI_SUCCESS)
		{
			AddLogEntryEx(SECONDLEVEL, L"Error creating instance. Error status is [%x]", status);
			return false;
		}
	}

	if (!m_pscanner)
	{
		AddLogEntryEx(SECONDLEVEL, L"Error creating instance");
		return false;
	}

	memset(&m_pSmartDBApi, 0, sizeof(SmartDBExportedFunctions));
	m_pSmartDBApi.nStructSize = sizeof(SmartDBExportedFunctions);


	if (m_pfnThreatScanner_GetSmartDBFunctions)
		m_pfnThreatScanner_GetSmartDBFunctions(&m_pSmartDBApi);

	//
	// Connect to scanner's events in order to receive detailed information during the scan process
	// You can provide a callback context here
	//
	if (m_pfnThreatScanner_SetScanCallback2)
		m_pfnThreatScanner_SetScanCallback2(m_pscanner, OnObjectScanComplete2, NULL);

	//
	// Set some scanner options.
	//
	if (m_pfnThreatScanner_SetIntOption)
	{
		m_pfnThreatScanner_SetIntOption(m_pscanner, _optFreeModuleInstances, 1);
		m_pfnThreatScanner_SetIntOption(m_pscanner, _optCbkFileOnlyStatus, 1);
		m_pfnThreatScanner_SetIntOption(m_pscanner, _optReportOBJ_FL_REG, 1);
		m_pfnThreatScanner_SetIntOption(m_pscanner, _optSmartScan, 1);
		m_pfnThreatScanner_SetIntOption(m_pscanner, _optScanPacked, 1);
		m_pfnThreatScanner_SetIntOption(m_pscanner, _optDeepScan, 0);
		m_pfnThreatScanner_SetIntOption(m_pscanner, _optScanArchives, 0);
		m_pfnThreatScanner_SetIntOption(m_pscanner, _optQuarantineRemoveReadOnly, 1);
	}

	return true;
}

/***********************************************************************************************
Function Name  : OnObjectScanComplete2
Description    : 
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
void CBDClient::OnObjectScanComplete2(ScanCbkParams *params, int *pnScanAction, void *context)
{
}

/***********************************************************************************************
Function Name  : BD_ScanFile
Description    : Function which scan file using bitdefender Scan engine
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
bool CBDClient::BD_ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID)
{
	bool bReturn = false;
	__try
	{
		TCHAR	szThreatName[0x64];
		if (!m_pfnThreatScanner_ScanPath)
		{
			return bReturn;
		}

		if (!m_pscanner)
			return bReturn;

		int nObjectStatus = OBJECT_STATUS_CLEAN;

		BOOL bRet = ScanFileInMemory(m_pscanner, szFilePath, szThreatName);
		if (bRet)
		{
			if (_tcslen(szThreatName) != 0x00)
			{
				_tcscpy(szVirusName, szThreatName);
				_tcsupr(szVirusName);
				dwISpyID = 0xFFFFF;
				return true;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		m_pModule = NULL;
		BD_initialize();
		AddLogEntry(L"### Exception in BD_ScanFile, File Path: [%s]", szFilePath, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : BD_uninitialize
Description    : Function which will un-initialize bidefender scan engine
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
bool CBDClient::BD_uninitialize()
{
	__try
	{
		//
		// Uninitialize
		//
		if (m_pscanner)
		{
			if (m_pfnThreatScanner_DestroyInstance)
				m_pfnThreatScanner_DestroyInstance(m_pscanner);
		}

		if (m_pfnThreatScanner_Uninitialize)
			m_pfnThreatScanner_Uninitialize();

		if (m_pModule != NULL)
		{
			FreeLibrary(m_pModule);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in BD_uninitialize", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : BD_RepairFile
Description    : Function which repair file using bitdfender repair module
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
bool CBDClient::BD_RepairFile(LPCTSTR szFilePath, DWORD dwSpyID)
{
	bool bReturn = false;
	HANDLE file = INVALID_HANDLE_VALUE;
	__try
	{
		if (!m_pfnThreatScanner_ScanObjectByHandle)
		{
			return bReturn;
		}

		if (!m_pscanner)
			return bReturn;

		INT scanStatus = OBJECT_STATUS_CLEAN;
		INT threatType;
		const CHAR_T * threatInfo;
		file = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE)
		{
			return bReturn;
		}

		if (m_pfnThreatScanner_ScanObjectByHandle)
		{
			int status = m_pfnThreatScanner_ScanObjectByHandle(m_pscanner, file, szFilePath, TRUE, &scanStatus, &threatType, &threatInfo, GetCurrentProcessId());
			if (status == 0)
			{
				if (scanStatus == OBJECT_STATUS_INFECTED || scanStatus == OBJECT_STATUS_DELETED || scanStatus == OBJECT_STATUS_DISINFECTED || scanStatus == OBJECT_STATUS_MOVED_REBOOT)
				{
					bReturn = true;
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CBDClient::BD_RepairFile, File Path: [%s]", szFilePath, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadRequiredModules
Description    : Function will load required modules and functions
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
bool CBDClient::LoadRequiredModules()
{
	bool bReturn = false;
	__try
	{
		m_pModule = LoadLibrary(L"SCAN.DLL");
		if (!m_pModule)
		{
			AddLogEntry(L"### Error in Loading SCAN.DLL", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		m_pfnThreatScanner_InitializeEx = (fnThreatScanner_InitializeEx)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_InitializeEx");
		m_pfnThreatScanner_CreateInstance = (fnThreatScanner_CreateInstance)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_CreateInstance");
		m_pfnThreatScanner_SetIntOption = (fnThreatScanner_SetIntOption)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_SetIntOption");
		m_pfnThreatScanner_SetStringOption = (fnThreatScanner_SetStringOption)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_SetStringOption");
		m_pfnThreatScanner_SetScanCallback2 = (fnThreatScanner_SetScanCallback2)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_SetScanCallback2");
		m_pfnThreatScanner_ScanPath = (fnThreatScanner_ScanPath)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_ScanPath");
		m_pfnThreatScanner_ScanObject = (fnThreatScanner_ScanObject)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_ScanObject");
		m_pfnThreatScanner_SetPasswordCallback = (fnThreatScanner_SetPasswordCallback)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_SetPasswordCallback");
		m_pfnThreatScanner_SetExtCallback = (fnThreatScanner_SetExtCallback)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_SetExtCallback");
		m_pfnThreatScanner_GetOption = (fnThreatScanner_GetOption)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_GetOption");
		m_pfnThreatScanner_GetScanStatistics = (fnThreatScanner_GetScanStatistics)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_GetScanStatistics");
		m_pfnThreatScanner_DestroyInstance = (fnThreatScanner_DestroyInstance)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_DestroyInstance");
		m_pfnThreatScanner_Uninitialize = (fnThreatScanner_Uninitialize)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_Uninitialize");
		m_pfnThreatScanner_ScanMemoryEx = (fnThreatScanner_ScanMemoryEx)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_ScanMemoryEx");
		m_pfnThreatScanner_InitializeMemoryScan = (fnThreatScanner_InitializeMemoryScan)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_InitializeMemoryScan");
		m_pfnThreatScanner_UninitializeMemoryScan = (fnThreatScanner_UninitializeMemoryScan)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_UninitializeMemoryScan");
		m_pfnThreatScanner_ScanObjectByHandle = (fnThreatScanner_ScanObjectByHandle)GetProcAddress(m_pModule, (LPCSTR)"ThreatScanner_ScanObjectByHandle");
		m_pfnThreatScanner_GetSmartDBFunctions = (fnThreatScanner_GetSmartDBFunctions)GetProcAddress(m_pModule, "ThreatScanner_GetSmartDBFunctions");
		m_pfnThreatScanner_GetKeys = (fnThreatScanner_GetKeys)GetProcAddress(m_pModule, "ThreatScanner_GetKeys");

		if (!m_pfnThreatScanner_InitializeEx || !m_pfnThreatScanner_CreateInstance || !m_pfnThreatScanner_SetIntOption ||
			!m_pfnThreatScanner_SetStringOption || !m_pfnThreatScanner_SetScanCallback2 || !m_pfnThreatScanner_ScanObject ||
			!m_pfnThreatScanner_SetPasswordCallback || !m_pfnThreatScanner_SetExtCallback || !m_pfnThreatScanner_GetOption ||
			!m_pfnThreatScanner_ScanMemoryEx || !m_pfnThreatScanner_InitializeMemoryScan || !m_pfnThreatScanner_ScanObjectByHandle || 
			!m_pfnThreatScanner_GetSmartDBFunctions || !m_pfnThreatScanner_GetKeys ||
			!m_pfnThreatScanner_GetScanStatistics || !m_pfnThreatScanner_ScanPath || !m_pfnThreatScanner_DestroyInstance || !m_pfnThreatScanner_Uninitialize)
		{
			return bReturn;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CBDClient::LoadRequiredModules", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : ScanFileInMemory
Description    : Function which will load file into process memory and scan.
Author Name    : Ramkrushna Shelke
Date           : 28 Aug 2019
***********************************************************************************************/
BOOL CBDClient::ScanFileInMemory(CThreatScanner * scanner, LPCTSTR filePath, LPTSTR lpVirusName)
{
	bool bReturn = false;
	HANDLE mapping = NULL;
	LPVOID sharedMemPtr = NULL;
	HANDLE file = INVALID_HANDLE_VALUE;
	bool bScannedbyFileHandle = false;

	__try
	{
		if (!filePath)
			return bReturn;

		if (!lpVirusName)
			return bReturn;

		if (!scanner)
			return bReturn;

		DWORD sharedMemorySize = 2 * 1024 * 1024;
		INT scanStatus = OBJECT_STATUS_CLEAN;
		INT threatType;
		const CHAR_T * threatInfo;
		//
		// Create a shared memory object 2MB in size
		//
		mapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0L, sharedMemorySize, _T("ScannerSharedMemory"));

		if ((sharedMemPtr = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0)) == NULL)
		{
			CloseHandle(mapping);
			return bReturn;
		}

		//
		// Tell the scanner to use our created shared memory object when asked to scan in-memory
		//
		if (!m_pfnThreatScanner_InitializeMemoryScan)
		{
			if (sharedMemPtr)
				UnmapViewOfFile(sharedMemPtr);

			if (mapping)
				CloseHandle(mapping);

			return bReturn;
		}

		m_pfnThreatScanner_InitializeMemoryScan(scanner, L"ScannerSharedMemory", sharedMemorySize);

		//
		// Copy the file's contents in memory
		//
		file = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0L, NULL);
		if (file == INVALID_HANDLE_VALUE)
		{
			bReturn = FALSE;
			goto CLEANUP;
		}

		GZ_SCAN_RESULT ScanResult = SCAN_STATE_UNKNOWN;
		CacheKeys pKeys;
		BY_HANDLE_FILE_INFORMATION szByFileInfo = { 0 };
		if (GetFileInformationByHandle(file, &szByFileInfo))
		{
			m_pfnThreatScanner_GetKeys(m_pscanner, &pKeys, 1, NULL);
			if (m_pSmartDBApi.fnSmartDBGetAVByFileID && m_pSmartDBApi.fnSmartDBGetAVByFileID(NULL, INVALID_HANDLE_VALUE,
				&szByFileInfo, &pKeys, NULL) > 0)
			{
				ScanResult = SCAN_STATE_SCANNED;
			}
		}

		//if file is already scanned then no need to scan it again
		if (ScanResult == SCAN_STATE_SCANNED)
		{
			//OutputDebugString(L"\n>>> Already Scanned");
			//OutputDebugString(filePath);
			bReturn = FALSE;
			goto CLEANUP;
		}

		ULONG objectSize = GetFileSize(file, NULL);

		//if file is big (more than 600 MB) no need to scan
		ULONG ulMaxFileSizeLimit = 600 * 1024 * 1024;
		if (objectSize > ulMaxFileSizeLimit)
		{
			bReturn = FALSE;
			goto CLEANUP;
		}

		if (objectSize == INVALID_FILE_SIZE || objectSize > sharedMemorySize)
		{
			//
			// The file is either too big or could its size could not be determined. Give up memory scan and instead scan the file by path.
			// Make sure you do NOT close the handle (the server closes it) UNLESS the server is used out-of-process, in which case you should 
			// duplicate the handle using DuplicateHandle() with DUPLICATE_CLOSE_SOURCE flag before passing it to the scanner
			//
			if (m_pfnThreatScanner_ScanObjectByHandle)
			{
				bScannedbyFileHandle = true;
				int status = m_pfnThreatScanner_ScanObjectByHandle(scanner, file, filePath, FALSE, &scanStatus, &threatType, &threatInfo, GetCurrentProcessId());
				if (status == 0)
				{
					if (scanStatus == OBJECT_STATUS_INFECTED || (scanStatus == OBJECT_STATUS_SUSPICIOUS && m_bIsHeuScan))
					{
						if (threatInfo)
						{
							_tcscpy(lpVirusName, threatInfo);
							bReturn = TRUE;

							if (m_pfnThreatScanner_UninitializeMemoryScan)
								m_pfnThreatScanner_UninitializeMemoryScan(scanner);

							if (sharedMemPtr)
								UnmapViewOfFile(sharedMemPtr);

							if (mapping)
								CloseHandle(mapping);

							return bReturn;
						}
					}
				}
			}
		}
		else
		{
			//
			// Read the file contents into the shared memory
			//
#define BUF_SIZE 100000
			DWORD actualRead;
			DWORD totalRead = 0;
			DWORD readCount = BUF_SIZE;
			char buffer[BUF_SIZE];
			while (totalRead < objectSize)
			{
				readCount = objectSize - totalRead;
				if (readCount > BUF_SIZE)
					readCount = BUF_SIZE;

				if (!ReadFile(file, buffer, readCount, &actualRead, NULL))
				{
					goto CLEANUP;
				}

				memcpy((char *)sharedMemPtr + totalRead, buffer, actualRead);

				totalRead += readCount;
			}

			if (ScanResult != SCAN_STATE_SCANNED)
			{
				if (m_pSmartDBApi.fnSmartDBSearchDat && m_pSmartDBApi.fnSmartDBSearchDat(INVALID_HANDLE_VALUE, (BYTE *)sharedMemPtr,
					NULL, 1, &pKeys, &szByFileInfo, NULL) > 0)
				{
					ScanResult = SCAN_STATE_SCANNED;
				}
			}

			//if file is already scanned then no need to scan it again
			if (ScanResult == SCAN_STATE_SCANNED)
			{
				//OutputDebugString(L"\n>>> Already Scanned");
				//OutputDebugString(filePath);
				bReturn = FALSE;
				goto CLEANUP;
			}

			//
			// Scan the shared memory. Of course, you can call ScanMemory() repeatedly on the same shared memory object
			//
			if (m_pfnThreatScanner_ScanMemoryEx)
			{
				int status = m_pfnThreatScanner_ScanMemoryEx(scanner, filePath, OBJECT_TYPE_FILE, &objectSize, FALSE, &scanStatus, &threatType, &threatInfo);

				if (scanStatus == OBJECT_STATUS_CLEAN)
				{
					SetAVByFileIDExtParams setAVExtParams;
					memset(&setAVExtParams, 0, sizeof(SetAVByFileIDExtParams));
					setAVExtParams.nStructSize = sizeof(SetAVByFileIDExtParams);
					////setAVExtParams.llCurrentUSN = ScanFileRq->FileData.Usn;
					//if ((ScanFileRq->FileData.Reserved1 & 0xFFFF0000) >> 16)
					//setAVExtParams.pwzVolumeGuid = (ScanFileRq->FileData.FileName + ScanFileRq->FileData.FileNameLength / 2 + 1 + ScanFileRq->FileData.SourceFileNameLength / 2 + 1);
					m_pSmartDBApi.fnSmartDBSetAVByFileIDEx ? m_pSmartDBApi.fnSmartDBSetAVByFileIDEx(NULL, INVALID_HANDLE_VALUE, &szByFileInfo, &pKeys, &setAVExtParams) : 0;
				}

				if (status == 0x00)
				{
					if (scanStatus == OBJECT_STATUS_INFECTED || (scanStatus == OBJECT_STATUS_SUSPICIOUS && m_bIsHeuScan))
					{
						if (threatInfo)
						{
							_tcscpy(lpVirusName, threatInfo);
							bReturn = TRUE;
							goto CLEANUP;
						}
					}
				}
			}
		}

		//
		// if the file was altered in memory (for example because of cleaning) you should copy the shared memory contents back to the file
		//
		// <COPY THE SHARED MEMORY CONTENTS BACK TO THE FILE HERE>
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		m_pModule = NULL;
		BD_initialize();
		AddLogEntry(L"### Exception in CBDClient::ScanFileInMemory, File Path: [%s]", filePath, 0, true, SECONDLEVEL);
	}

CLEANUP:
	if (!bScannedbyFileHandle)
	{
		//close the file handle here
		if (file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(file);
			file = INVALID_HANDLE_VALUE;
		}
	}

	//
	// Uninitialize
	//
	if (m_pfnThreatScanner_UninitializeMemoryScan)
		m_pfnThreatScanner_UninitializeMemoryScan(scanner);

	if (sharedMemPtr)
		UnmapViewOfFile(sharedMemPtr);

	if (mapping)
		CloseHandle(mapping);
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  : SetHeuristicSetting
*  Description    :	Function to set heuristic Settings
*  Author Name    : Ram Shelke
*  Date           : 28 AUG 2019
**********************************************************************************************************/
bool CBDClient::SetHeuristicSetting(int iHeuFlag)
{
	bool bReturn = false;
	try
	{
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::ReloadSettingsForScanDLL", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ReadHeuristicScanStatus
*  Description    : Function to get Registry for Heuristic Scan.
*  Author Name    : Ram Shelke
*  Date			  :	15 Sep 2016
****************************************************************************************************/
bool CBDClient::ReadHeuristicScanStatus()
{
	try
	{
		//Get here registry setting for Heuristic Scan.
		DWORD dwIsHeuScan = 0x01;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwHeuScan", dwIsHeuScan) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwHeuScan in ReadHeuristicScanStatus", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (dwIsHeuScan == 0x00)
		{
			m_bIsHeuScan = false;
		}
		else if (dwIsHeuScan == 0x01)
		{
			m_bIsHeuScan = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed in CBDClient::ReadHeuristicScanStatus", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}