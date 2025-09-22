/****************************************************************************
Program Name          : main.cpp
Description           : This file contains a startup function and file enumeration,
						Scanning functionality.
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#pragma once
#include "NativeLogger.h"
#include "Ntstrsafe.h"
#include "WardWizNTReg.h"
#include "CommonFunctions.h"
#include "WardWizHash.h"
#include "WardWizPEScanner.h"

int	LOGBASE = SECONDLEVEL;
WCHAR szAppPath[MAX_PATH] = { 0 };

#pragma warning(disable:4995)
#pragma warning(disable:4996)

#pragma comment(lib,"ntdll.lib")
#pragma comment(linker, "/ENTRY:NtProcessStartup")
#pragma comment(linker, "/SUBSYSTEM:NATIVE")
#pragma comment (linker, "/BASE:0x00010000")

#define		ALLOCSIZE					sizeof(FILE_FULL_DIR_INFORMATION) * 512
#define		BACKSLASH					L"\\"
#define		BLANK_STR					L" "
#define		NtCloseSafe(h)				{ if(h) { NtClose(h); h = NULL; } }
#define		STOP_KB_WAIT_INTERVAL		10000 /* ms */
#define 	WLSRV_ID_FOURTEEN			14  //Vibranium Boot Scanner
#define		WRDWIZBOOTRECOVERDB			L"VBBOOTRECOVER.DB"
#define		WRDSCANSESSDETAILDB			L"VBSCANSESSDETAIL.DB"
#define		WRDWIZSCANDETAILSDB			L"VBSCANDETAILS.DB"
#define		WRDWIZBTSCNSESSIONDB		L"VBBTSCNSESSION.DB"
#define		WRDWIZPRODUCTSETTINGINI		L"PRODUCTSETTINGS.INI"
#define		WRDWIZQUARANTINEDB			L"QUARANTINE.DB"

unsigned long			lFileCount				= 0;
HANDLE					keyboardHandle			= NULL;
HANDLE					hEvent					= NULL;
NTSTATUS				ntKeyboardStatus		= STATUS_UNSUCCESSFUL;
bool					bEscapeButtonPressed	= false;
bool					bScanCompleted			= false;
HANDLE					hThread					= NULL;
HANDLE					hStatusThread			= NULL;
unsigned long			lFilesScanned			= 0;
unsigned long			lThreatsFound			= 0;
unsigned long			lThreatsCleaned			= 0;
WCHAR					szProgramFilesPath[50]	= { 0 };
DWORD					m_dwLangID				= 0x00;
DWORD					m_dwBootScanType		= 0x00;
DWORD					m_dwProdID				= 0x00;
DWORD					m_dwQuarOption			= 0x00;
/*-----------------------------------------*/
typedef struct __TAGRECOVERENTRIES
{
	CHAR szFilePath[0x104];
	CHAR szBackupPath[0xC8];
	CHAR szThreatName[0x32];
}BOOTRECOVERENTRIES;

typedef struct __TAGWRDWIZSCANDETAILS
{
	LARGE_INTEGER	lIntstartDatetime;
	LARGE_INTEGER	lIntEndDateTime;
	CHAR			szThreatName[0x32];
	CHAR			szFilePath[0x104];
	DWORD 			dwActionTaken;
}WRDWIZSCANDETAILS;

typedef struct __TAGSCANSESSIONDETAILS
{
	LARGE_INTEGER 	SessionStartDatetime;
	LARGE_INTEGER 	SessionEndDate;
	DWORD 			TotalFilesScanned;
	DWORD 			TotalThreatsFound;
	DWORD 			TotalThreatsCleaned;
}SCANSESSIONDETAILS;

/*------------------ FUNCTION DECLARATION ------------------------*/
int ToWideString(WCHAR* &pwStr, const char* pStr, int len, BOOL IsEnd);
void TerminateProcess();
bool ReadKeyboardInput();
bool InitializeKeyBoard();
bool UnInitializeKeyBoard();
DWORD ReadScanSetting();
BOOLEAN DisplayShortPath(WCHAR* pwszFilePath);
bool AddRecoverEntry(LPTSTR lpFilePath, LPTSTR lpThreatName, LPTSTR lpBackupPath);
bool AddScanSessionDetails(LARGE_INTEGER startDatetime, LARGE_INTEGER EndDateTime, DWORD dwTotalFilesScanned, DWORD dwTotalThreatsFound, DWORD dwTotalThreatsCleaned);
bool AddScanDetails(LARGE_INTEGER startDatetime, LARGE_INTEGER EndDateTime, LPTSTR lpFilePath, LPTSTR lpThreatName, DWORD dwActionTaken);
bool GetProductSettings();
bool ParseSingleLine(LPSTR lpszLine);
DWORD CheckCreateScanSessionDetails();
bool DeleteScanSessionDetails();
NTSTATUS NTAPI RtlGetSystemTimeToLocalTime(IN PLARGE_INTEGER SystemTime, OUT PLARGE_INTEGER LocalTime);
char *RemoveSpaces(char *str);
bool QuarantineOnRestart(LPTSTR pszFilePath);
bool QuarantineFile(LPTSTR szVirusName, LPTSTR szThreatPath);
bool ParseQuarantineLine(LPSTR lpszLine);
/*------------------------------------------------------------------*/

/*------------------ THREAD FUNCTION DECLARATION ------------------------*/
DWORD WINAPI thread_proc(LPVOID parameter);
/*------------------------------------------------------------------*/

/***********************************************************************************************
*  Function Name  : ListDirectory
*  Description    : Recursive function which gets the list of files/folders in directory
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
NTSTATUS ListDirectory(WCHAR * pszDirectoryName)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	if (!pszDirectoryName)
		return ntStatus;

	__try
	{
		//if an Escape key has been pressed then exit
		if (bEscapeButtonPressed)
			return ntStatus;


		RtlAddLogEntry(ZEROLEVEL, L">>> ListDirectory Scanning Path: %s", pszDirectoryName);

		OBJECT_ATTRIBUTES RootDirectoryAttributes;
		HANDLE RootDirectoryHandle = INVALID_HANDLE_VALUE;
		IO_STATUS_BLOCK Iosb;
		HANDLE Event = INVALID_HANDLE_VALUE;
		UCHAR Buffer[65535] = { 0 };

		PFILE_BOTH_DIR_INFORMATION DirInformation;
		UNICODE_STRING			 NtName;
		PWSTR					 PartName;
		CURDIR					 RelativeName;

		if (!RtlDosPathNameToNtPathName_U(pszDirectoryName, &NtName, (PWSTR*)&PartName, &RelativeName)) {
			RtlAddLogEntry(ZEROLEVEL, L"### Failed RtlDosPathNameToNtPathName_U : %s", NtName.Buffer);
			////DbgPrint("### Failed RtlDosPathNameToNtPathName_U : [%wZ]\n", &NtName);
			return ntStatus;
		}

		InitializeObjectAttributes(&RootDirectoryAttributes, &NtName, OBJ_CASE_INSENSITIVE, 0, 0);

		ntStatus = NtCreateFile(&RootDirectoryHandle,
			GENERIC_READ,
			&RootDirectoryAttributes,
			&Iosb,
			0,
			FILE_ATTRIBUTE_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			FILE_OPEN,
			FILE_DIRECTORY_FILE,
			0, 0);

		if (!NT_SUCCESS(ntStatus))
		{
			RtlAddLogEntry(ZEROLEVEL, L"Unable to open :[%s]", pszDirectoryName);
			//DbgPrint("Unable to open, Error: %x\n", ntStatus);
			return ntStatus;
		}

		//make Dir name
		UNICODE_STRING usDirName;
		RtlInitUnicodeString(&usDirName, pszDirectoryName);

		ntStatus = NtCreateEvent(&Event, GENERIC_ALL, 0, NotificationEvent, FALSE);
		if (!NT_SUCCESS(ntStatus))
		{
			RtlAddLogEntry(SECONDLEVEL, L"Event creation failed with error :[%d]", ntStatus);
			return ntStatus;
		}

		ntStatus = NtQueryDirectoryFile(RootDirectoryHandle,
			Event, 0, 0,
			&Iosb,
			Buffer,
			sizeof(Buffer),
			FileBothDirectoryInformation,
			FALSE,
			NULL,
			FALSE);

		if (ntStatus == STATUS_PENDING)
		{
			ntStatus = NtWaitForSingleObject(Event, TRUE, 0);
		}

		if (!NT_SUCCESS(ntStatus))
		{
			RtlAddLogEntry(SECONDLEVEL, L"Unable to query directory contents error :[%d]", ntStatus);
			return ntStatus;
		}
		DirInformation = (PFILE_BOTH_DIR_INFORMATION)Buffer;

		while (DirInformation != NULL && DirInformation->FileNameLength > 0)
		{
			//if an Escape key has been pressed then exit
			if (bEscapeButtonPressed)
				break;

			UNICODE_STRING EntryShortName;
			EntryShortName.MaximumLength = EntryShortName.Length = (USHORT)DirInformation->ShortNameLength;
			EntryShortName.Buffer = &DirInformation->ShortName[0];


			UNICODE_STRING EntryName;
			EntryName.MaximumLength = EntryName.Length = (USHORT)DirInformation->FileNameLength;
			EntryName.Buffer = &DirInformation->FileName[0];

			if (DirInformation->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				UNICODE_STRING usCurrentDir;
				RtlInitUnicodeString(&usCurrentDir, L".");

				UNICODE_STRING usParentDir;
				RtlInitUnicodeString(&usParentDir, L"..");

				if ((RtlCompareUnicodeString(&EntryName, &usCurrentDir, FALSE) == 0) || RtlCompareUnicodeString(&EntryName, &usParentDir, FALSE) == 0)
				{
					if (0 == DirInformation->NextEntryOffset)
					{
						break;
					}
					else
					{
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
						continue;
					}
				}

				UNICODE_STRING usFullPath;
				unsigned int dwSize = (usDirName.Length + EntryName.Length)* sizeof(WCHAR);
				//DbgPrint("Length: %d\n", dwSize);
				usFullPath.Length = usFullPath.MaximumLength = dwSize;
				LPBYTE lpBuffer = (LPBYTE)kmalloc(m_hHeapMem, dwSize);
				//If Allocation failed.
				if (lpBuffer == NULL)
				{
					//DbgPrint("\nBuffer Allocation failed.");
					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				memset(lpBuffer, 0, dwSize);
				usFullPath.Buffer = (PWSTR)lpBuffer;

				UNICODE_STRING usBackSlash;
				RtlInitUnicodeString(&usBackSlash, L"\\");

				RtlCopyUnicodeString(&usFullPath, &usDirName);
				RtlAppendUnicodeStringToString(&usFullPath, &usBackSlash);
				RtlAppendUnicodeStringToString(&usFullPath, &EntryName);

				//skip those folders which has more length
				if (wcslen(usFullPath.Buffer) >= MAX_FILEPATH_LENGTH)
				{
					if (!kfree(m_hHeapMem, lpBuffer))
					{
						//DbgPrint("### kfree Failed in ListDirectory\n");
					}

					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				WCHAR szFullPath[MAX_FILEPATH_LENGTH * 2] = { 0 };
				Replace(szFullPath, usFullPath.Buffer, L"\\\\", L"\\");

				if (wcslen(szFullPath) == 0)
				{
					if (!kfree(m_hHeapMem, lpBuffer))
					{
						//DbgPrint("### kfree Failed in ListDirectory\n");
					}
					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				//Display here short Path
				UNICODE_STRING usShortPath;
				dwSize = (usDirName.Length + EntryShortName.Length)* sizeof(WCHAR) + sizeof(WCHAR);
				usShortPath.Length = usShortPath.MaximumLength = dwSize;

				LPBYTE lpShortPathBuffer = (LPBYTE)kmalloc(m_hHeapMem, dwSize);

				//If Allocation failed.
				if (lpShortPathBuffer == NULL)
				{
					//DbgPrint("\nBuffer Allocation failed in ListDirectory");

					if (!kfree(m_hHeapMem, lpBuffer))
					{
						//DbgPrint("### kfree Failed in ListDirectory\n");
					}
					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				memset(lpShortPathBuffer, 0, dwSize);
				usShortPath.Buffer = (PWSTR)lpShortPathBuffer;

				RtlCopyUnicodeString(&usShortPath, &usDirName);
				RtlAppendUnicodeStringToString(&usShortPath, &usBackSlash);
				RtlAppendUnicodeStringToString(&usShortPath, &EntryShortName);
				
				WCHAR szShortPath[0x100] = { 0 };
				szShortPath[0] = L'\0';
				Replace(szShortPath, usShortPath.Buffer, L"\\\\", L"\\");
				DisplayShortPath(szShortPath);

				bool bIsSubFolderExcluded = false;
				if (ISFileExcluded(szFullPath, bIsSubFolderExcluded))
				{
					RtlCliDisplayString(SECONDLEVEL, L"\nExcluded: [%s]\n", szFullPath);

					if (bIsSubFolderExcluded)
					{
						if (!kfree(m_hHeapMem, lpBuffer))
						{
							//DbgPrint("### kfree Failed in ListDirectory\n");
						}

						if (!kfree(m_hHeapMem, lpShortPathBuffer))
						{
							//DbgPrint("### kfree Failed in ListDirectory\n");
						}

						if (0 == DirInformation->NextEntryOffset)
							break;
						else
							DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
						continue;
					}
				}

				if (usFullPath.Buffer != NULL)
				{
					ListDirectory(usFullPath.Buffer);
				}

				if (!kfree(m_hHeapMem, lpBuffer))
				{
					//DbgPrint("### kfree Failed in ListDirectory\n");
				}

				if (!kfree(m_hHeapMem, lpShortPathBuffer))
				{
					//DbgPrint("### kfree Failed in ListDirectory\n");
				}
			}
			else
			{
				lFilesScanned++;

				UNICODE_STRING usFullPath;
				unsigned int dwSize = (usDirName.Length + EntryName.Length)* sizeof(WCHAR) + sizeof(WCHAR);
				usFullPath.Length = usFullPath.MaximumLength = dwSize;
				
				LPBYTE lpBuffer = (LPBYTE)kmalloc(m_hHeapMem, dwSize);

				//If Allocation failed.
				if (lpBuffer == NULL)
				{
					//DbgPrint("\nBuffer Allocation failed in ListDirectory");
					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				memset(lpBuffer, 0, dwSize);
				usFullPath.Buffer = (PWSTR)lpBuffer;

				UNICODE_STRING usBackSlash;
				RtlInitUnicodeString(&usBackSlash, L"\\");

				RtlCopyUnicodeString(&usFullPath, &usDirName);
				RtlAppendUnicodeStringToString(&usFullPath, &usBackSlash);
				RtlAppendUnicodeStringToString(&usFullPath, &EntryName);

				//skip those files which has more length
				if (wcslen(usFullPath.Buffer) >= MAX_FILEPATH_LENGTH)
				{
					if (!kfree(m_hHeapMem, lpBuffer))
					{
						//DbgPrint("### kfree Failed in ListDirectory\n");
					}

					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				WCHAR szFullPath[MAX_FILEPATH_LENGTH * 2] = { 0 };
				Replace(szFullPath, usFullPath.Buffer, L"\\\\", L"\\");

				if (wcslen(szFullPath) == 0)
				{
					if (!kfree(m_hHeapMem, lpBuffer))
					{
						//DbgPrint("### kfree Failed in ListDirectory\n");
					}

					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				bool bIsSubFolderExcluded = false;
				if (ISFileExcluded(szFullPath, bIsSubFolderExcluded))
				{
					RtlCliDisplayString(SECONDLEVEL, L"\nExcluded: [%s]\n", szFullPath);

					if (!kfree(m_hHeapMem, lpBuffer))
					{
						//DbgPrint("### kfree Failed in ListDirectory");
					}
					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}
				
				if (ISExcludedExt(DirInformation->FileName))
				{
					RtlCliDisplayString(SECONDLEVEL, L"\nExcluded: [%s]\n", szFullPath);

					if (!kfree(m_hHeapMem, lpBuffer))
					{
						//DbgPrint("### kfree Failed in ListDirectory");
					}
					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				WCHAR szVirusName[MAX_PATH] = { 0 };
				DWORD dwSpyID = 0x00;

				//DbgPrint("\n FullPath : [%S]", szFullPath);

				WCHAR szFileName[0x96];
				WCHAR *ptrPos = wcsrchr(szFullPath, L'\\');
				if (!ptrPos)
				{
					if (!kfree(m_hHeapMem, lpBuffer))
					{
						//DbgPrint("### kfree Failed in ListDirectory");
					}
					if (0 == DirInformation->NextEntryOffset)
						break;
					else
						DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
					continue;
				}

				ptrPos++;
				wcscpy(szFileName, ptrPos);

				//DbgPrint("\n File Name : [%S]", szFileName);

				bool bFound = false;
				if (ISScanByFileName(szFileName))
				{
					wcscpy(szVirusName, L"USER_DEFINED");
					dwSpyID = 0x00;
					bFound = true;
				}
				else if (RtlScanFile(szFullPath, szVirusName, dwSpyID, false))			//Scan file here
				{
					bFound = true;
				}
				
				if (bFound)
				{
					if (m_dwQuarOption == 0x00)
					{
						RtlCliDisplayString(SECONDLEVEL, L"\nThreat Name: %s\n", szVirusName);
						RtlCliDisplayString(SECONDLEVEL, L"Threat Path: [%s]\n", szFullPath);
						RtlCliDisplayString(SECONDLEVEL, L"Action: %s\n", L"Skipped");
						lThreatsFound++;
					}
					else
					{
						RtlAddLogEntry(SECONDLEVEL, L">>> Threat Found: Virus Name: [%s], File: [%s], ", szVirusName, szFullPath);

						bool bBackupSuccess = true;
						WCHAR szFilePath[MAX_FILEPATH_LENGTH] = { 0 };
						wcscpy(szFilePath, szFullPath);
						WCHAR szBackupPath[MAX_FILEPATH_LENGTH] = { 0 };

						if (!BackUpBeforeQuarantineOrRepair(szFilePath, szBackupPath))
						{
							bBackupSuccess = false;
							RtlAddLogEntry(SECONDLEVEL, L"### Failed BackUpBeforeQuarantineOrRepair in ListDirectory, File: [%s]", szFilePath);
						}

						if (bBackupSuccess)
						{
							if (!AddRecoverEntry(szFilePath, szVirusName, szBackupPath))
							{
								bBackupSuccess = false;
								RtlAddLogEntry(SECONDLEVEL, L"### Failed AddRecoverEntry in ListDirectory, File: [%s], BackupPath:[%s]", szFilePath, szBackupPath);
							}
						}

						if (bBackupSuccess)
						{
							if (!NtFileDeleteFile(szFilePath))
							{
								bBackupSuccess = false;
								RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileDeleteFile in ListDirectory, File: [%s], BackupPath:[%s]", szFilePath, szBackupPath);
							}
							lThreatsCleaned++;
						}

						if (bBackupSuccess)
						{
							LARGE_INTEGER lIntSysTIme = { 0 };
							LARGE_INTEGER lIntTIme = { 0 };
							NtQuerySystemTime(&lIntSysTIme);
							RtlGetSystemTimeToLocalTime(&lIntSysTIme, &lIntTIme);

							if (!AddScanDetails(lIntTIme, lIntTIme, szFullPath, szVirusName, 0x01))
							{
								RtlAddLogEntry(SECONDLEVEL, L"### Failed AddScanDetails in ListDirectory, File: [%s], VirusName:[%s]", szFullPath, szVirusName);
							}

							RtlCliDisplayString(SECONDLEVEL, L"\nThreat Name: %s\n", szVirusName);
							RtlCliDisplayString(SECONDLEVEL, L"Threat Path: [%s]\n", szFullPath);
							RtlCliDisplayString(SECONDLEVEL, L"Action: %s\n", L"Quarantined");
							lThreatsFound++;
						}
					}
				}

				if (!kfree(m_hHeapMem, lpBuffer))
				{
					//DbgPrint("### kfree Failed in ListDirectory");
				}
			}

			if (0 == DirInformation->NextEntryOffset)
				break;
			else
				DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
		}
		NtClose(RootDirectoryHandle);
		RootDirectoryHandle = NULL;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ListDirectory, DirName:[%s]", pszDirectoryName);
	}	
	return ntStatus;
}

/***********************************************************************************************
*  Function Name  : NtProcessStartup
*  Description    : Entry point function
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
void NtProcessStartup() {
	__try
	{
		InitializeNativeLogger();
		
		//Register here with protection manager
		RegisterProcessId(WLSRV_ID_FOURTEEN);

		//---------------------------------------------------------------
		//Set here current directory to use in overall project
		WCHAR lpCurrentDirectory[MAX_PATH] = { 0 };
		if (!RtlGetCurrentDir(lpCurrentDirectory, MAX_PATH))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlGetCurrentDir function failed..");
			return;
		}

		WCHAR szOSDrive[0x02] = { 0 };
		szOSDrive[0x00] = lpCurrentDirectory[0x00];
		szOSDrive[0x01] = L'\0';

		wcscpy(szAppPath, szOSDrive);
		wcscat(szAppPath, L":\\Program Files\\Vibranium");
		//DbgPrint("\nCurrentDir: [%S]\n", szAppPath);
		
		//get here all product settings
		GetProductSettings();

		//if Scan type is 0x00, then no need to scan
		if (m_dwBootScanType == 0x00)
		{
			return;
		}

		RtlAddLogEntry(SECONDLEVEL, L"----------------------------------------------------------");
		RtlAddLogEntry(SECONDLEVEL, L"                Vibranium Boot Time Scanner");
		RtlAddLogEntry(SECONDLEVEL, L"----------------------------------------------------------");

		DWORD dwScanSession = CheckCreateScanSessionDetails();
		if (dwScanSession == SESSIONDBEXISTS || dwScanSession == DELTESESSIONDBFAIL)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Successful boot not happened for OS during last scan, so skipped current scan");
			return;
		}

		bool bKeyboardInitialized = true;
		if (!InitializeKeyBoard())
		{
			bKeyboardInitialized = false;
			RtlAddLogEntry(ZEROLEVEL, L"### InitializeKeyBoard Failed");
		}


		RtlAddLogEntry(ZEROLEVEL, L"### Before InitHeapMemory");
		//Initialize heap memory here
		m_hHeapMem = InitHeapMemory();

		if (!m_hHeapMem)
		{
			m_hHeapMem = RtlProcessHeap();
			if (!m_hHeapMem)
			{
				RtlAddLogEntry(ZEROLEVEL, L"### InitHeapMemory failed");
				return;
			}
		}

		//Issue resolved:0005928 as black line was getting displayed.
		DisplayString(L"\n", SECONDLEVEL);

		RtlCliDisplayString(SECONDLEVEL, L"Initializing Vibranium Boot Time Scanner...");

		//Load here database
		if (!LoadSignatureDB())
		{
			RtlAddLogEntry(SECONDLEVEL, L"### VibraniumPEScanner LoadSignatureDB Failed");
			return;
		}

		LoadExcludeDB();
		LoadExcludeExtDB();
		LoadScanByNameDB();

		//NtDisplayString does not provide get backspace, so this is hack.
		for (int ibackSpace = 0; ibackSpace <= 0x29; ibackSpace++)
		{
			RtlCliPutChar('\r');
			RtlClipBackspace();
		}


		//---------------------------------------------------------------

		DisplayString(L"----------------------------------------------------------\n", SECONDLEVEL);
		DisplayString(L"          Vibranium Boot Time Scanner\n\n", SECONDLEVEL);
		DisplayString(L"          All rights reserved Vibranium 2023\n", SECONDLEVEL);
		DisplayString(L"----------------------------------------------------------\n", SECONDLEVEL);

		if (m_dwBootScanType == 0x01)
		{
			RtlCliDisplayString(SECONDLEVEL, L"Quick Scan in progress, ");
		}
		else if (m_dwBootScanType == 0x02)
		{
			RtlCliDisplayString(SECONDLEVEL, L"Full Scan in progress, ");
		}
		
		//Check for WinXP here
		bool bISWinXP = false;
		DWORD dwMajor = 0x00;
		DWORD dwMinor = 0x00;
		DWORD dwBuild = 0x00;
		RtlGetNtVersionNumbers(&dwMajor, &dwMinor, &dwBuild);
		if (dwMajor == 5 && dwMinor == 1)
		{
			bISWinXP = true;
		}

		if (bKeyboardInitialized && !bISWinXP)
		{
			RtlCliDisplayString(SECONDLEVEL, L"press [ESC] button to abort Scan.\n\n\n");
		}
		else
		{
			RtlCliDisplayString(SECONDLEVEL, L"please do not power off or unplug your machine.\n\n\n");
		}

		WCHAR szQurDBPath[MAX_PATH] = { 0 };
		wcscpy(szQurDBPath, szAppPath);
		wcscat(szQurDBPath, L"\\QUARANTINE\\");
		wcscat(szQurDBPath, WRDWIZQUARANTINEDB);

		if (FileExists(szQurDBPath))
		{
			if (QuarantineOnRestart(szQurDBPath))
			{
				if (!NtFileDeleteFile(szQurDBPath))
				{
					RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileDeleteFile in ListDirectory, File: [%s]", szQurDBPath);
				}
			}
		}

		NTSTATUS status;
		PVOID parameter = NULL;

		if (bKeyboardInitialized)
		{
			status = RtlCreateUserThread(NtCurrentProcess(), NULL,
				0, 0, 0, 0, (PUSER_THREAD_START_ROUTINE)thread_proc, parameter, &hThread, NULL);

			if (!NT_SUCCESS(status)){
				return;
			}
		}

		RtlCliDisplayString(SECONDLEVEL, L"[Scanning Folder]\n");

		//Remove old reports DB file.
		WCHAR szReportsDBPath[MAX_PATH] = { 0 };
		wcscpy(szReportsDBPath, szAppPath);
		wcscat(szReportsDBPath, L"\\");
		wcscat(szReportsDBPath, WRDWIZSCANDETAILSDB);
		if (FileExists(szReportsDBPath))
		{
			NtFileDeleteFile(szReportsDBPath);
		}

		LARGE_INTEGER lStartSystime = { 0 };
		LARGE_INTEGER lStarttime = { 0 };
		NtQuerySystemTime(&lStartSystime);
		RtlGetSystemTimeToLocalTime(&lStartSystime, &lStarttime);

		if (!AddScanSessionDetails(lStarttime, lStarttime, lFilesScanned, lThreatsFound, lThreatsCleaned))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed AddScanSessionDetails in NtProcessStartup");
		}

		RtlAddLogEntry(ZEROLEVEL, L">>> BootScanType: %d", m_dwBootScanType);
		RtlAddLogEntry(ZEROLEVEL, L">>> LangID: %d", m_dwLangID);
		RtlAddLogEntry(ZEROLEVEL, L">>> ProdID: %d", m_dwProdID);
		RtlAddLogEntry(ZEROLEVEL, L">>> LOGLEVEL: %d", LOGBASE);
		RtlAddLogEntry(ZEROLEVEL, L">>> Auto QuarOption: %d", m_dwQuarOption);

		if (m_dwBootScanType == 0x01)
		{
			//Program files.
			WCHAR szPath[0x100] = { 0 };
			wcscpy(szPath, szOSDrive);
			wcscat(szPath, L":\\Program Files");
			if (FolderExists(szPath))
			{
				ListDirectory(szPath);
			}

			if (!bEscapeButtonPressed)
			{
				//Program files x86.
				memset(&szPath, 0, sizeof(szPath));;
				wcscpy(szPath, szOSDrive);
				wcscat(szPath, L":\\Program Files (x86)");
				if (FolderExists(szPath))
				{
					ListDirectory(szPath);
				}
			}

			if (!bEscapeButtonPressed)
			{
				//Windows
				memset(&szPath, 0, sizeof(szPath));;
				wcscpy(szPath, szOSDrive);
				wcscat(szPath, L":\\Windows");
				if (FolderExists(szPath))
				{
					ListDirectory(szPath);
				}
			}
		}
		else
		{
			for (int chDrive = 'C'; chDrive <= 'Z'; chDrive++)
			{
				WCHAR szOSDrive[0x02] = { 0 };
				szOSDrive[0x00] = chDrive;
				szOSDrive[0x01] = L'\0';

				WCHAR szPath[0x100] = { 0 };
				wcscpy(szPath, szOSDrive);
				wcscat(szPath, L":\\");

				if (bEscapeButtonPressed)
					break;

				if (!bEscapeButtonPressed)
				{
					{
						ListDirectory(szPath);
					}
				}
			}
		}

		if (!DeleteScanSessionDetails())
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed DeleteScanSessionDetails");
		}

		LARGE_INTEGER lEndSystime = { 0 };
		LARGE_INTEGER lEndtime = { 0 };
		NtQuerySystemTime(&lEndSystime);
		RtlGetSystemTimeToLocalTime(&lEndSystime, &lEndtime);
		
		if (!AddScanSessionDetails(lStarttime, lEndtime, lFilesScanned, lThreatsFound, lThreatsCleaned))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed AddScanSessionDetails in NtProcessStartup");
		}

		LARGE_INTEGER Interval;
		Interval.QuadPart = -10000000;
		NtDelayExecution(FALSE, &Interval);

		DisplayString(L"\n\n\n\n----------------------------------------------------------\n", SECONDLEVEL);

		if (bEscapeButtonPressed)
		{
			DisplayString(L"\n          Scanning Aborted\n\n", SECONDLEVEL);
		}
		else
		{
			bScanCompleted = true;
			DisplayString(L"\n          Scanning Completed\n\n", SECONDLEVEL);
		}

		DisplayString(L"----------------------------------------------------------\n", SECONDLEVEL);
		RtlCliDisplayString(SECONDLEVEL, L"          Total Files Scanned: %d\n", lFilesScanned);
		RtlCliDisplayString(SECONDLEVEL, L"          Threats Found: %d\n", lThreatsFound);
		DisplayString(L"----------------------------------------------------------\n", SECONDLEVEL);
		
		RtlAddLogEntry(SECONDLEVEL, L"----------------------------------------------------------");
		RtlAddLogEntry(SECONDLEVEL, L"          Total Files Scanned: %d", lFilesScanned);
		RtlAddLogEntry(SECONDLEVEL, L"          Threats Found: %d", lThreatsFound);
		RtlAddLogEntry(SECONDLEVEL, L"----------------------------------------------------------");

		//Before  program exit, we must deallocate memory
		UnLoadSignatureDB();
		FreeHeapAllocMemory();

		RtlCloseLogFile();
		if (hThread != NULL)
		{
			NtCloseSafe(hThread);
			hThread = NULL;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtProcessStartup");
	}

	//Wait till for some time.
	LARGE_INTEGER Interval;
	Interval.QuadPart = -150000000;
	NtDelayExecution(FALSE, &Interval);
	NtTerminateProcess(NtCurrentProcess(), 0);
}

/***********************************************************************************************
*  Function Name  : TerminateProcess
*  Description    : Function which terminates the process
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
void TerminateProcess()
{
	__try
	{
		//Wait till for some time.
		LARGE_INTEGER Interval;
		Interval.QuadPart = -150000000;
		NtDelayExecution(FALSE, &Interval);
		NtTerminateProcess(NtCurrentProcess(), 0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		NtTerminateProcess(NtCurrentProcess(), 0);
	}
}

/***********************************************************************************************
*  Function Name  : InitializeKeyBoard
*  Description    : Function which creates keyboard and initialize
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
bool InitializeKeyBoard()
{
	bool bReturn = false;
	__try
	{
		UNICODE_STRING keyboardDeviceName;
		OBJECT_ATTRIBUTES objectAttributes;
		IO_STATUS_BLOCK ioStatusBlock = { 0 };
		bEscapeButtonPressed = false;
		bScanCompleted = false;
		//DbgPrint("\nReadKeyboardInput Start");
		RtlAddLogEntry(ZEROLEVEL, L">>> InitializeKeyBoard Start");

		RtlInitUnicodeString(&keyboardDeviceName, L"\\Device\\KeyboardClass0");

		InitializeObjectAttributes(&objectAttributes,
			&keyboardDeviceName,
			OBJ_CASE_INSENSITIVE,
			NULL,
			NULL);

		memset(&ioStatusBlock, 0, sizeof(ioStatusBlock));

		ntKeyboardStatus = NtCreateFile(&keyboardHandle,
			GENERIC_READ | FILE_READ_ATTRIBUTES,
			&objectAttributes,
			&ioStatusBlock,
			0,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN,
			FILE_DIRECTORY_FILE,
			0,
			0);
		
		if (ntKeyboardStatus == STATUS_SUCCESS)
		{
			//DbgPrint("\nKeyboard created");
			RtlAddLogEntry(ZEROLEVEL, L"Keyboard created");

			InitializeObjectAttributes(&objectAttributes,
				NULL,
				0,
				NULL,
				NULL);

			ntKeyboardStatus = NtCreateEvent(&hEvent,
				STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x03,
				&objectAttributes,
				NotificationEvent,
				FALSE);

			if (ntKeyboardStatus == STATUS_SUCCESS)
			{
				bReturn = true;
				//DbgPrint("\nEvent created");
				RtlAddLogEntry(ZEROLEVEL, L"Event created");
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in InitializeKeyBoard");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : UnInitializeKeyBoard
*  Description    : Function which deinitilize keyboard and initialize
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
bool UnInitializeKeyBoard()
{
	bool bReturn = false;
	__try
	{
		if (hEvent != NULL)
		{
			NtClose(hEvent);
			hEvent = NULL;
		}

		if (keyboardHandle != NULL)
		{
			NtFileCloseFile(keyboardHandle);
			keyboardHandle = NULL;
		}

		ntKeyboardStatus = STATUS_UNSUCCESSFUL;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in UnInitializeKeyBoard");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ReadKeyboardInput
*  Description    : Function which reads input from keyboard
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
bool ReadKeyboardInput()
{
	bool bReturn = false;
	__try
	{
		 if (bEscapeButtonPressed || bScanCompleted)
		{
			return true;
		}

		IO_STATUS_BLOCK ioStatusBlock;
		NTSTATUS status;
		KEYBOARD_INPUT_DATA inputData;

		if (ntKeyboardStatus == STATUS_SUCCESS)
		{
			RtlAddLogEntry(ZEROLEVEL, L">>> ntKeyboardStatus STATUS_SUCCESS");

			LARGE_INTEGER byteOffset;
			byteOffset.LowPart = 0;
			byteOffset.HighPart = 0;
			memset(&ioStatusBlock, 0, sizeof(ioStatusBlock));

			if (keyboardHandle == NULL)
			{
				RtlAddLogEntry(ZEROLEVEL, L">>> >>> keyboardHandle  NULL");
				return bReturn;
			}

			status = NtReadFile(keyboardHandle,
				hEvent,
				NULL,
				NULL,
				&ioStatusBlock,
				&inputData,
				sizeof(KEYBOARD_INPUT_DATA),
				&byteOffset,
				NULL);

			if (status == STATUS_PENDING)
			{
				LARGE_INTEGER TimeOut;
				TimeOut.QuadPart = -150000000;
				status = NtWaitForSingleObject(hEvent, TRUE, &TimeOut);
				if (status == STATUS_SUCCESS)
				{
					//DbgPrint("Success");
					if (inputData.MakeCode == 0x01)
					{
						bReturn = true;
						bEscapeButtonPressed = true;
					}
					//DbgPrint("\n>>> Scan Code: %d\n", inputData.MakeCode);
				}
			}
			else
			{
				//DbgPrint("\nRead failed: ");
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ReadKeyboardInput");
	}
	return bReturn;
}


/***********************************************************************************************
*  Function Name  : thread_proc
*  Description    : Thread function which monitors keyboard press event.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
DWORD WINAPI thread_proc(LPVOID parameter)
{
	__try
	{
		while (1)
		{
			if (bScanCompleted)
				break;
			
			if (ReadKeyboardInput())
			{
				UnInitializeKeyBoard();
				break;
			}
		}
		LARGE_INTEGER Interval;
		Interval.QuadPart = -150000000;
		NtDelayExecution(FALSE, &Interval);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in thread_proc");
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : DisplayShortPath
*  Description    : function which converts file path to showrt and display.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN DisplayShortPath(WCHAR* pwszFilePath)
{
	__try
	{
		WCHAR szStatusString[0x33] = { 0 };
		szStatusString[0] = L'\0';

		int iInputLen = (int)wcslen(pwszFilePath);

		if (iInputLen > MAX_FILEPATH_LENGTH - 1)
			return FALSE;

		if (iInputLen > 0x32)
		{
			WCHAR szLastString[MAX_FILEPATH_LENGTH] = { 0 };
			szLastString[0] = L'\0';

			WCHAR szTempString[MAX_FILEPATH_LENGTH] = { 0 };
			szTempString[0] = L'\0';
			wcscpy(szTempString, pwszFilePath);

			WCHAR *cp = NULL;
			cp = &szTempString[0x15];
			if (cp == NULL)
			{
				return FALSE;
			}

			wcscpy(szLastString, cp);

			szTempString[0x16] = L'\0';
			wcscpy(szStatusString, szTempString);

			cp = &szLastString[(iInputLen - 0x15) - 0x18];

			wcscat(szStatusString, L"...");
			wcscat(szStatusString, cp);
		}
		else
		{
			wcscpy(szStatusString, pwszFilePath);
		}

		//NtDisplayString does not provide get backspace, so this is hack.
		for (int ibackSpace = 0; ibackSpace <= 0x32; ibackSpace++)
		{
			RtlCliPutChar('\r');
			RtlClipBackspace();
		}

		int iLen = (int)wcslen(szStatusString);

		//NtDisplayString does not provide get backspace, so this is hack.
		for (int iFwdSpace = iLen; iFwdSpace <= 0x32; iFwdSpace++)
		{
			wcscat(szStatusString, BLANK_STR);
		}

		//Print here the scanning file path
		RtlCliDisplayString(SECONDLEVEL, szStatusString);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in DisplayShortPath, Path: [%s]", pwszFilePath);
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : ReadScanSetting
*  Description    : function which read scan setting
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
DWORD ReadScanSetting()
{
	DWORD dwRet = 0x00;
	return dwRet;
}

/***********************************************************************************************
*  Function Name  : AddRecoverEntry
*  Description    : function which adds entry in recover DB.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 24-Oct-2017
*************************************************************************************************/
bool AddRecoverEntry(LPTSTR lpFilePath, LPTSTR lpThreatName, LPTSTR lpBackupPath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool bReturn = false;
	__try
	{
		if (!lpFilePath || !lpThreatName || !lpBackupPath)
			return bReturn;

		WCHAR szRecoverDBPath[MAX_PATH] = { 0 };
		wcscpy(szRecoverDBPath, szAppPath);
		wcscat(szRecoverDBPath, L"\\QUARANTINE\\");
		wcscat(szRecoverDBPath, WRDWIZBOOTRECOVERDB);

		if (!NtFileOpenFile(&hFile, szRecoverDBPath, TRUE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Unable to createfile, File: [%s]", szRecoverDBPath);
			return bReturn;
		}

		LONGLONG lFileSize = 0x00;
		if (!NtFileGetFileSize(hFile, &lFileSize))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileGetFileSize in AddRecoverEntry file: [%s]", szRecoverDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		if (!NtFileSeekFile(hFile, lFileSize))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileSeekFile in AddRecoverEntry file: [%s]", szRecoverDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		CHAR szFilePath[0x104] = { 0 };
		CHAR szBackupPath[0xC8] = { 0 };
		CHAR szThreatName[0x32] = { 0 };

		BOOTRECOVERENTRIES stBootRecEntry;
		memset(&stBootRecEntry, 0, sizeof(stBootRecEntry));

		//FilePath
		DECLARE_UNICODE_STRING_SIZE(usFilePath, 0x104);
		wcscpy(usFilePath.Buffer, lpFilePath);
		usFilePath.Length = (USHORT)wcslen(lpFilePath) * sizeof(WCHAR);
		usFilePath.MaximumLength = 0x104;
		ANSI_STRING ansFilePath;
		RtlUnicodeStringToAnsiString(&ansFilePath, &usFilePath, TRUE);
		strcpy(stBootRecEntry.szFilePath, ansFilePath.Buffer);
		RtlFreeAnsiString(&ansFilePath);

		//BackupPath
		DECLARE_UNICODE_STRING_SIZE(usBackupPath, 0xC8);
		wcscpy(usBackupPath.Buffer, lpBackupPath);
		usBackupPath.Length = (USHORT)wcslen(lpBackupPath) * sizeof(WCHAR);
		usBackupPath.MaximumLength = 0xC8;
		ANSI_STRING ansBackUpPath;
		RtlUnicodeStringToAnsiString(&ansBackUpPath, &usBackupPath, TRUE);
		strcpy(stBootRecEntry.szBackupPath, ansBackUpPath.Buffer);
		RtlFreeAnsiString(&ansBackUpPath);

		//ThreatName
		DECLARE_UNICODE_STRING_SIZE(usThreatName, 0x32);
		wcscpy(usThreatName.Buffer, lpThreatName);
		usThreatName.Length = (USHORT)wcslen(lpThreatName) * sizeof(WCHAR);
		usThreatName.MaximumLength = 0x32;
		ANSI_STRING ansThreatName;
		RtlUnicodeStringToAnsiString(&ansThreatName, &usThreatName, TRUE);
		strcpy(stBootRecEntry.szThreatName, ansThreatName.Buffer);
		RtlFreeAnsiString(&ansThreatName);

		DWORD dwBytesWritten = 0x00;
		if (!NtFileWriteFile(hFile, &stBootRecEntry, sizeof(stBootRecEntry), &dwBytesWritten))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile file: [%s]", szRecoverDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != sizeof(stBootRecEntry))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile file: [%s]", szRecoverDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception AddRecoverEntry FilePath: [%s], ThreatName: [%s], BackupPath: [%s]", lpFilePath, lpThreatName, lpBackupPath);
	}
CLEANUP:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : AddScanSessionDetails
*  Description    : function which adds records in ScanSession table
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 24-Oct-2017
*************************************************************************************************/
bool AddScanSessionDetails(LARGE_INTEGER startDatetime, LARGE_INTEGER EndDateTime, DWORD dwTotalFilesScanned, DWORD dwTotalThreatsFound, DWORD dwTotalThreatsCleaned)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool bReturn = false;
	__try
	{
		//---------------------------------------------------------------
		//Set here current directory to use in overall project
		WCHAR lpCurrentDirectory[MAX_PATH] = { 0 };
		if (!RtlGetCurrentDir(lpCurrentDirectory, MAX_PATH))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlGetCurrentDir function failed in AddScanSessionDetails");
			return bReturn;
		}

		WCHAR szOSDrive[0x02] = { 0 };
		szOSDrive[0x00] = lpCurrentDirectory[0x00];
		szOSDrive[0x01] = L'\0';

		wcscpy(szAppPath, szOSDrive);
		wcscat(szAppPath, L":\\Program Files\\Vibranium");

		WCHAR szReportsDBPath[MAX_PATH] = { 0 };
		wcscpy(szReportsDBPath, szAppPath);
		wcscat(szReportsDBPath, L"\\");
		wcscat(szReportsDBPath, WRDSCANSESSDETAILDB);

		if (FileExists(szReportsDBPath))
		{
			NtFileDeleteFile(szReportsDBPath);
		}

		if (!NtFileOpenFile(&hFile, szReportsDBPath, TRUE, TRUE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Unable to createfile, File: [%s]", szReportsDBPath);
			return bReturn;
		}

		if (!NtFileSeekFile(hFile, 0x00))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileSeekFile in AddScanSessionDetails file: [%s]", szReportsDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		SCANSESSIONDETAILS stScanSesstion;
		memset(&stScanSesstion, 0, sizeof(stScanSesstion));
		stScanSesstion.SessionStartDatetime = startDatetime;
		stScanSesstion.SessionEndDate = EndDateTime;
		stScanSesstion.TotalFilesScanned = dwTotalFilesScanned;
		stScanSesstion.TotalThreatsCleaned = dwTotalThreatsCleaned;
		stScanSesstion.TotalThreatsFound = dwTotalThreatsFound;

		DWORD dwBytesWritten = 0x00;
		if (!NtFileWriteFile(hFile, &stScanSesstion, sizeof(stScanSesstion), &dwBytesWritten))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile in AddScanSessionDetails file: [%s]", szReportsDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != sizeof(stScanSesstion))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile in AddScanSessionDetailsfile: [%s]", szReportsDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception AddScanSessionDetails");
	}
CLEANUP:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : AddScanDetails
*  Description    : function which adds Scan details.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 24-Oct-2017
*************************************************************************************************/
bool AddScanDetails(LARGE_INTEGER startDatetime, LARGE_INTEGER EndDateTime, LPTSTR lpFilePath, LPTSTR lpThreatName, DWORD dwActionTaken)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool bReturn = false;
	__try
	{
		if (!lpFilePath || !lpThreatName)
			return bReturn;

		//---------------------------------------------------------------
		//Set here current directory to use in overall project
		WCHAR lpCurrentDirectory[MAX_PATH] = { 0 };
		if (!RtlGetCurrentDir(lpCurrentDirectory, MAX_PATH))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlGetCurrentDir function failed in AddScanDetails");
			return bReturn;
		}

		WCHAR szOSDrive[0x02] = { 0 };
		szOSDrive[0x00] = lpCurrentDirectory[0x00];
		szOSDrive[0x01] = L'\0';

		wcscpy(szAppPath, szOSDrive);
		wcscat(szAppPath, L":\\Program Files\\Vibranium");

		WCHAR szReportsDBPath[MAX_PATH] = { 0 };
		wcscpy(szReportsDBPath, szAppPath);
		wcscat(szReportsDBPath, L"\\");
		wcscat(szReportsDBPath, WRDWIZSCANDETAILSDB);

		if (!NtFileOpenFile(&hFile, szReportsDBPath, TRUE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Unable to createfile in AddScanDetails, File: [%s]", szReportsDBPath);
			return bReturn;
		}

		LONGLONG lFileSize = 0x00;
		if (!NtFileGetFileSize(hFile, &lFileSize))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileGetFileSize in AddScanDetails file: [%s]", szReportsDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		if (!NtFileSeekFile(hFile, lFileSize))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileSeekFile in AddScanDetails file: [%s]", szReportsDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		WRDWIZSCANDETAILS stScanDetails;
		memset(&stScanDetails, 0, sizeof(stScanDetails));
		stScanDetails.lIntstartDatetime = startDatetime;
		stScanDetails.lIntEndDateTime = EndDateTime;

		DECLARE_UNICODE_STRING_SIZE(usFilePath, 0x104);
		wcscpy(usFilePath.Buffer, lpFilePath);
		usFilePath.Length = (USHORT)wcslen(lpFilePath) * sizeof(WCHAR);
		usFilePath.MaximumLength = 0x104;
		ANSI_STRING ansFilePath;
		RtlUnicodeStringToAnsiString(&ansFilePath, &usFilePath, TRUE);
		strcpy(stScanDetails.szFilePath, ansFilePath.Buffer);
		RtlFreeAnsiString(&ansFilePath);

		//ThreatName
		DECLARE_UNICODE_STRING_SIZE(usThreatName, 0x32);
		wcscpy(usThreatName.Buffer, lpThreatName);
		usThreatName.Length = (USHORT)wcslen(lpThreatName) * sizeof(WCHAR);
		usThreatName.MaximumLength = 0x32;
		ANSI_STRING ansThreatName;
		RtlUnicodeStringToAnsiString(&ansThreatName, &usThreatName, TRUE);
		strcpy(stScanDetails.szThreatName, ansThreatName.Buffer);
		RtlFreeAnsiString(&ansThreatName);

		DWORD dwBytesWritten = 0x00;
		if (!NtFileWriteFile(hFile, &stScanDetails, sizeof(stScanDetails), &dwBytesWritten))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile in AddScanDetails file: [%s]", szReportsDBPath);
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesWritten != sizeof(stScanDetails))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile in AddScanDetails: [%s]", szReportsDBPath);
			bReturn = false;
			goto CLEANUP;
		}
		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in  AddScanDetails FilePath: [%s], ThreatName: [%s]", lpFilePath, lpThreatName);
	}
CLEANUP:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : GetProductSettings
*  Description    : function to get product settings from WRDWIZPRODUCTSETTINGINI
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 24-Oct-2017
*************************************************************************************************/
bool GetProductSettings()
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool bReturn = false;
	BYTE	*bFileBuffer = NULL;
	__try
	{
		//---------------------------------------------------------------
		//Set here current directory to use in overall project
		WCHAR lpCurrentDirectory[MAX_PATH] = { 0 };
		if (!RtlGetCurrentDir(lpCurrentDirectory, MAX_PATH))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlGetCurrentDir function failed in AddScanDetails");
			return bReturn;
		}

		WCHAR szOSDrive[0x02] = { 0 };
		szOSDrive[0x00] = lpCurrentDirectory[0x00];
		szOSDrive[0x01] = L'\0';

		wcscpy(szAppPath, szOSDrive);
		wcscat(szAppPath, L":\\Program Files\\Vibranium");

		WCHAR szSettingIniPath[MAX_PATH] = { 0 };
		wcscpy(szSettingIniPath, szAppPath);
		wcscat(szSettingIniPath, L"\\VBSETTINGS\\");
		wcscat(szSettingIniPath, WRDWIZPRODUCTSETTINGINI);

		if (!NtFileOpenFile(&hFile, szSettingIniPath, TRUE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Unable to createfile in GetProductSettings, File: [%s]", szSettingIniPath);
			return bReturn;
		}

		LONGLONG lFileSize = 0x00;
		if (!NtFileGetFileSize(hFile, &lFileSize))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileGetFileSize in GetProductSettings file: [%s]", szSettingIniPath);
			bReturn = false;
			goto CLEANUP;
		}

		bFileBuffer = (LPBYTE)RtlAllocateHeap(RtlProcessHeap(), 0, (SIZE_T)lFileSize);

		if (bFileBuffer == NULL)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DWORD dwBytesRead = 0;
		if (!NtFileReadFile(hFile, bFileBuffer, (DWORD)lFileSize, &dwBytesRead))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != (DWORD)lFileSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		//strlwr((char*)bFileBuffer);
		char szLine[MAX_PATH] = { 0 };
		DWORD iLineOffset = 0x00;
		DWORD iBufOffset = 0x00;
		while (iBufOffset < (DWORD)lFileSize)
		{
			if (bFileBuffer[iBufOffset] == '\r' && bFileBuffer[iBufOffset + 1] == '\n')
			{
				szLine[iLineOffset] = '\0';
				if (!ParseSingleLine(szLine))
				{
					RtlAddLogEntry(ZEROLEVEL, L"### Failed ParseSingleLine, Line: [%s]", szLine);
				}
				memset(&szLine, 0, sizeof(szLine));
				iLineOffset = 0x00;
				iBufOffset += 2;
				continue;
			}
			szLine[iLineOffset++] = bFileBuffer[iBufOffset];
			iBufOffset++;
		}
		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in  GetProductSettings");
	}
CLEANUP:
	//Cleanup here memory
	if (bFileBuffer != NULL)
	{
		RtlFreeHeap(RtlProcessHeap(), 0, bFileBuffer);
		bFileBuffer = NULL;
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : RemoveSpaces
*  Description    : function which remove blank spaces in the string.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 08-Nov-2017
*************************************************************************************************/
char *RemoveSpaces(char *str)
{
	__try
	{
		int i = 0, j = 0;
		while (str[i])
		{
			if (str[i] != ' ')
				str[j++] = str[i];
			i++;
		}
		str[j] = '\0';
		return str;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RemoveSpaces");
	}
	return NULL;
}

/***********************************************************************************************
*  Function Name  : ParseSingleLine
*  Description    : function which parse single line and check for specific settings.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 26-Oct-2017 
*************************************************************************************************/
bool ParseSingleLine(LPSTR lpszLine)
{
	bool bReturn = false;
	__try
	{
		if (!lpszLine)
			return false;

		if (strlen(lpszLine) == 0)
			return false;

		//strupr(lpszLine);

		char szLeftPart[50] = { 0 };
		char szRightPart[50] = { 0 };

		char *chr = strrchr(lpszLine, '=');
		if (!chr)
			return false;

		if (chr)
		{
			chr++;
			strcpy(szRightPart, chr);
			chr--;
			*chr = '\0';
			strcpy(szLeftPart, lpszLine);
		}

		char szLeft[50] = { 0 };
		char szRight[50] = { 0 };

		char* pLeft = RemoveSpaces(szLeftPart);
		if (pLeft == NULL)
		{
			return false;
		}

		char* pRight = RemoveSpaces(szRightPart);
		if (pLeft == NULL)
		{
			return false;
		}

		strcpy(szLeft, pLeft);
		strcpy(szRight, pRight);

		if (strcmp(szLeft, "LanguageID") == 0)
		{
			if (strlen(szRight) != 0)
			{
				m_dwLangID = atoi(szRight);
			}
		}
		else if (strcmp(szLeft, "ProductID") == 0)
		{
			if (strlen(szRight) != 0)
			{
				m_dwProdID = atoi(szRight);
				SetProductID(m_dwProdID);
			}
		}
		else if (strcmp(szLeft, "BootTimeScan") == 0)
		{
			if (strlen(szRight) != 0)
			{
				m_dwBootScanType = atoi(szRight);
			}
		}
		else if (strcmp(szLeft, "BootTimeLogLevel") == 0)
		{
			if (strlen(szRight) != 0)
			{
				LOGBASE = atoi(szRight);
			}
		}
		else if (strcmp(szLeft, "AutoQuarOption") == 0)
		{
			if (strlen(szRight) != 0)
			{
				m_dwQuarOption = atoi(szRight);
			}
		}
		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ParseSingleLine");
	}
	return bReturn;
}



/***********************************************************************************************
*  Function Name  : CreateScanSessionDetails
*  Description    : function which creates temporary file as session, which gets created once scanning started
					and getting removed once scanning finished, if file is present means last scan was broken.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 06-Nov-2017
*************************************************************************************************/
DWORD CheckCreateScanSessionDetails()
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwReturn = 0x00;
	__try
	{
		//---------------------------------------------------------------
		//Set here current directory to use in overall project
		WCHAR lpCurrentDirectory[MAX_PATH] = { 0 };
		if (!RtlGetCurrentDir(lpCurrentDirectory, MAX_PATH))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlGetCurrentDir function failed in AddScanDetails");
			return dwReturn;
		}

		WCHAR szOSDrive[0x02] = { 0 };
		szOSDrive[0x00] = lpCurrentDirectory[0x00];
		szOSDrive[0x01] = L'\0';

		wcscpy(szAppPath, szOSDrive);
		wcscat(szAppPath, L":\\Program Files\\Vibranium");

		WCHAR szScnSessionDBPath[MAX_PATH] = { 0 };
		wcscpy(szScnSessionDBPath, szAppPath);
		wcscat(szScnSessionDBPath, L"\\");
		wcscat(szScnSessionDBPath, WRDWIZBTSCNSESSIONDB);

		if (FileExists(szScnSessionDBPath))
		{
			if (!NtFileDeleteFile(szScnSessionDBPath))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileDeleteFile in CheckCreateScanSessionDetails, File: [%s]", szScnSessionDBPath);
				dwReturn = DELTESESSIONDBFAIL;
				goto CLEANUP;
			}

			dwReturn = SESSIONDBEXISTS;
			goto CLEANUP;
		}

		if (!NtFileOpenFile(&hFile, szScnSessionDBPath, TRUE, TRUE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Unable to createfile in CheckCreateScanSessionDetails, File: [%s]", szScnSessionDBPath);
			dwReturn = CREATESCANDBFAIL;
			goto CLEANUP;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in  CreateScanSessionDetails");
	}
CLEANUP:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return dwReturn;
}


/***********************************************************************************************
*  Function Name  : DeleteScanSessionDetails
*  Description    : function will remove the seesion DB once scanning completed successfully.
*  SR_NO		  : 
*  Date           : 06-Nov-2017
*************************************************************************************************/
bool DeleteScanSessionDetails()
{
	bool bReturn = false;
	__try
	{
		//---------------------------------------------------------------
		//Set here current directory to use in overall project
		WCHAR lpCurrentDirectory[MAX_PATH] = { 0 };
		if (!RtlGetCurrentDir(lpCurrentDirectory, MAX_PATH))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlGetCurrentDir function failed in AddScanDetails");
			return bReturn;
		}

		WCHAR szOSDrive[0x02] = { 0 };
		szOSDrive[0x00] = lpCurrentDirectory[0x00];
		szOSDrive[0x01] = L'\0';

		wcscpy(szAppPath, szOSDrive);
		wcscat(szAppPath, L":\\Program Files\\Vibranium");

		WCHAR szScnSessionDBPath[MAX_PATH] = { 0 };
		wcscpy(szScnSessionDBPath, szAppPath);
		wcscat(szScnSessionDBPath, L"\\");
		wcscat(szScnSessionDBPath, WRDWIZBTSCNSESSIONDB);

		if (FileExists(szScnSessionDBPath))
		{
			if (!NtFileDeleteFile(szScnSessionDBPath))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileDeleteFile in CheckCreateScanSessionDetails, File: [%s]", szScnSessionDBPath);
				return bReturn;
			}

			bReturn = true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in  DeleteScanSessionDetails");
	}
	return bReturn;
}

NTSTATUS
NTAPI
RtlGetSystemTimeToLocalTime(IN PLARGE_INTEGER SystemTime,
OUT PLARGE_INTEGER LocalTime)
{
	__try
	{
		typedef NTSTATUS(NTAPI *RTLSYSTEMTIMETOLOCALTIME)(IN PLARGE_INTEGER SystemTime, OUT PLARGE_INTEGER LocalTime);

		HMODULE moduleHandle = NULL;
		UNICODE_STRING moduleName;
		RtlInitUnicodeString(&moduleName, L"ntdll.dll");
		LdrGetDllHandle(NULL, NULL, &moduleName, (PVOID*)&moduleHandle);

		if (moduleHandle == NULL)
		{
			RtlAddLogEntry(SECONDLEVEL, L"#### NTDLL.DLL not loaded");
			return STATUS_UNSUCCESSFUL;
		}

		ANSI_STRING asRtlSystemTimeToLocalTime;
		RtlInitAnsiString(&asRtlSystemTimeToLocalTime, "RtlSystemTimeToLocalTime");

		RTLSYSTEMTIMETOLOCALTIME RtlSystemTimeToLocalTime = NULL;
		(RTLSYSTEMTIMETOLOCALTIME)LdrGetProcedureAddress(moduleHandle, &asRtlSystemTimeToLocalTime, 0, (PVOID*)&RtlSystemTimeToLocalTime);

		if (RtlSystemTimeToLocalTime == NULL)
		{
			RtlAddLogEntry(SECONDLEVEL, L"#### RtlSystemTimeToLocalTime function does not found");
			if (moduleHandle != NULL)
			{
				LdrUnloadDll(moduleHandle);
			}
			return STATUS_UNSUCCESSFUL;
		}

		NTSTATUS ntStatus = RtlSystemTimeToLocalTime(SystemTime, LocalTime);

		if (moduleHandle != NULL)
		{
			LdrUnloadDll(moduleHandle);
		}

		if (!NT_SUCCESS(ntStatus))
		{
			return STATUS_UNSUCCESSFUL;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in  RtlGetSystemTimeToLocalTime");
		return STATUS_UNSUCCESSFUL;
	}
	return STATUS_SUCCESS;
}

/***********************************************************************************************
*  Function Name  : QuarantineOnRestart
*  Description    : function to Quarantine files when boot time scan get started.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 19-Dec-2018
*************************************************************************************************/
bool QuarantineOnRestart(LPTSTR pszFilePath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool bReturn = false;
	BYTE	*bFileBuffer = NULL;
	__try
	{
		if (!NtFileOpenFile(&hFile, pszFilePath, FALSE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Unable to readfile in QuarantineOnRestart, File: [%s]", pszFilePath);
			return bReturn;
		}

		LONGLONG lFileSize = 0x00;
		if (!NtFileGetFileSize(hFile, &lFileSize))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileGetFileSize in QuarantineOnRestart file: [%s]", pszFilePath);
			bReturn = false;
			goto CLEANUP;
		}

		bFileBuffer = (LPBYTE)RtlAllocateHeap(RtlProcessHeap(), 0, (SIZE_T)lFileSize);

		if (bFileBuffer == NULL)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DWORD dwBytesRead = 0;
		if (!NtFileReadFile(hFile, bFileBuffer, (DWORD)lFileSize, &dwBytesRead))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != (DWORD)lFileSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		//strlwr((char*)bFileBuffer);
		char szLine[MAX_PATH] = { 0 };
		DWORD iLineOffset = 0x00;
		DWORD iBufOffset = 0x00;
		while (iBufOffset < (DWORD)lFileSize)
		{
			if (bFileBuffer[iBufOffset] == '\r' && bFileBuffer[iBufOffset + 1] == '\n')
			{
				szLine[iLineOffset] = '\0';
				if (!ParseQuarantineLine(szLine))
				{
					RtlAddLogEntry(ZEROLEVEL, L"### Failed QuarantineOnRestart, Line: [%s]", szLine);
				}
				memset(&szLine, 0, sizeof(szLine));
				iLineOffset = 0x00;
				iBufOffset += 2;
				continue;
			}
			szLine[iLineOffset++] = bFileBuffer[iBufOffset];
			iBufOffset++;
		}
		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in  QuarantineOnRestart");
	}
CLEANUP:
	//Cleanup here memory
	if (bFileBuffer != NULL)
	{
		RtlFreeHeap(RtlProcessHeap(), 0, bFileBuffer);
		bFileBuffer = NULL;
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : QuarantineFile
*  Description    : function which takes threat name and its path to quarantine with backup.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 19-Dec-2018
*************************************************************************************************/
bool QuarantineFile(LPTSTR szVirusName, LPTSTR szThreatPath)
{
	bool bReturn = false;
	__try
	{
		bool bBackupSuccess = true;
		WCHAR szFilePath[MAX_FILEPATH_LENGTH] = { 0 };
		wcscpy(szFilePath, szThreatPath);
		WCHAR szBackupPath[MAX_FILEPATH_LENGTH] = { 0 };

		if (!BackUpBeforeQuarantineOrRepair(szFilePath, szBackupPath))
		{
			bBackupSuccess = false;
			RtlAddLogEntry(SECONDLEVEL, L"### Failed BackUpBeforeQuarantineOrRepair in QuarantineFile, File: [%s]", szFilePath);
		}

		if (bBackupSuccess)
		{
			if (!AddRecoverEntry(szFilePath, szVirusName, szBackupPath))
			{
				bBackupSuccess = false;
				RtlAddLogEntry(SECONDLEVEL, L"### Failed AddRecoverEntry in QuarantineFile, File: [%s], BackupPath:[%s]", szFilePath, szBackupPath);
			}
		}

		if (bBackupSuccess)
		{
			if (!NtFileDeleteFile(szFilePath))
			{
				bBackupSuccess = false;
				RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileDeleteFile in QuarantineFile, File: [%s], BackupPath:[%s]", szFilePath, szBackupPath);
			}

			if (bBackupSuccess)
			{
				RtlCliDisplayString(SECONDLEVEL, L"\nThreat Name: %s\n", szVirusName);
				RtlCliDisplayString(SECONDLEVEL, L"Threat Path: [%s]\n", szFilePath);
				RtlCliDisplayString(SECONDLEVEL, L"Action: %s\n", L"Quarantined");
				lThreatsFound++;
				bReturn = true;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in QuarantineFile");
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ParseQuarantineLine
*  Description    : function which parse single line and check for Quarantine entries.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 26-Oct-2017
*************************************************************************************************/
bool ParseQuarantineLine(LPSTR lpszLine)
{
	bool bReturn = false;
	__try
	{
		if (!lpszLine)
			return false;

		if (strlen(lpszLine) == 0)
			return false;

		char szLeftPart[50] = { 0 };
		char szRightPart[50] = { 0 };

		char *chr = strrchr(lpszLine, '|');
		if (!chr)
			return false;

		if (chr)
		{
			chr++;
			strcpy(szRightPart, chr);
			chr--;
			*chr = '\0';
			strcpy(szLeftPart, lpszLine);
		}

		char szLeft[50] = { 0 };
		char szRight[50] = { 0 };

		char* pLeft = RemoveSpaces(szLeftPart);
		if (pLeft == NULL)
		{
			return false;
		}

		char* pRight = RemoveSpaces(szRightPart);
		if (pLeft == NULL)
		{
			return false;
		}

		strcpy(szLeft, pLeft);
		strcpy(szRight, pRight);

		DECLARE_UNICODE_STRING_SIZE(ulThreatName, MAX_PATH);
		FillUnicodeStringWithAnsi(&ulThreatName, szLeft);

		DECLARE_UNICODE_STRING_SIZE(ulFilePath, MAX_FILEPATH_LENGTH);
		FillUnicodeStringWithAnsi(&ulFilePath, szRight);

		if (!QuarantineFile(ulThreatName.Buffer, ulFilePath.Buffer))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed to Quarantine file in ParseQuarantineLine");
			return false;
		}
		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ParseQuarantineLine");
	}
	return bReturn;
}
