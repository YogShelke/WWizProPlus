/****************************************************************************
Program Name          : CommonFunctions.cpp
Description           : This file which contains all necessary common functions which
						are required for boot time scanner
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#include "CommonFunctions.h"

char *xargv[BUFFER_SIZE];
unsigned int xargc;
WCHAR *szPipeScannerProt = L"\\Device\\WlsrvScanner";

#define FILE_SHARE_VALID_FLAGS 0x00000007

/***********************************************************************************************
*  Function Name  : xstrtok
*  Description    : String tokenizer
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
char * xstrtok(char * s, const char * ct)
{
	__try
	{
		static char * old_strtok;
		char *sbegin, *send;
		sbegin = s ? s : old_strtok;
		if (!sbegin)
		{
			return NULL;
		}
		sbegin += strspn(sbegin, ct);
		if (*sbegin == '\0')
		{
			old_strtok = NULL;
			return(NULL);
		}
		send = strpbrk(sbegin, ct);
		if (send && *send != '\0')
		{
			*send++ = '\0';
		}
		old_strtok = send;
		return (sbegin);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in xstrtok");
	}
	return NULL;
}

/***********************************************************************************************
*  Function Name  : xstrtoktwo
*  Description    : another function for String tokenizer
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
char * xstrtoktwo(char * s, const char * ct)
{
	__try
	{
		static char * old_strtok;
		char *sbegin, *send;
		sbegin = s ? s : old_strtok;
		if (!sbegin)
		{
			return NULL;
		}
		sbegin += strspn(sbegin, ct);
		if (*sbegin == '\0')
		{
			old_strtok = NULL;
			return(NULL);
		}
		send = strpbrk(sbegin, ct);
		if (send && *send != '\0')
		{
			*send++ = '\0';
		}
		old_strtok = send;
		return (sbegin);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in xstrtoktwo");
	}
	return NULL;
}

/***********************************************************************************************
*  Function Name  : StringToArguments
*  Description    : Function which converts string to arguments
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
UINT StringToArguments(CHAR *str)
{
	__try
	{
		BOOL q = FALSE;

		char *p;

		p = str;

		// 0x01 as a separator

		do
		{
			switch (*p)
			{
			case ' ':
				if (q)
				{
					;
				}
				else
				{
					*p = '\1';
				}
				break;
			case '\"':
				if (q)
				{
					q = FALSE;
				}
				else
				{
					q = TRUE;
				}
				*p = '\1';
				break;
			default:

				break;
			}
			*p++;
		} while (*p);

		xargc = 0;

		p = xstrtok(str, "\1");
		if (p)
		{
			xargv[++xargc] = p;
			while (p != NULL)
			{
				p = xstrtok(NULL, "\1");
				xargv[++xargc] = p;
			}

			return xargc;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in StringToArguments");
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : GetFileAttributesNt
*  Description    : Function which retrives file attribute
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
ULONG GetFileAttributesNt(PWSTR filename)
{
	__try
	{
		OBJECT_ATTRIBUTES oa;
		FILE_BASIC_INFORMATION fbi;
		UNICODE_STRING nt_filename;

		RtlDosPathNameToNtPathName_U(filename, &nt_filename, NULL, NULL);
		InitializeObjectAttributes(&oa, &nt_filename, OBJ_CASE_INSENSITIVE, 0, 0);

		fbi.FileAttributes = 0;
		NtQueryAttributesFile(&oa, &fbi);

		return fbi.FileAttributes;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in GetFileAttributesNt, filename: [%s]", filename);
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : FolderExists
*  Description    : Function which Checks if folder exists
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOL FolderExists(PCWSTR foldername)
{
	__try
	{
		BOOL retval = FALSE;
		UNICODE_STRING u_filename, nt_filename;
		FILE_BASIC_INFORMATION fbi;
		OBJECT_ATTRIBUTES oa;
		NTSTATUS st;

		RtlInitUnicodeString(&u_filename, foldername);

		PWSTR					 PartName;
		CURDIR					 RelativeName;
		if (!RtlDosPathNameToNtPathName_U(u_filename.Buffer, &nt_filename, (PWSTR*)&PartName, &RelativeName))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed RtlDosPathNameToNtPathName_U in FolderExists : [%wZ]\n", &nt_filename);
			//DbgPrint("### Failed RtlDosPathNameToNtPathName_U in FolderExists : [%wZ]\n", &nt_filename);
			return FALSE;
		}

		InitializeObjectAttributes(&oa, &nt_filename, OBJ_CASE_INSENSITIVE, 0, 0);
		st = NtQueryAttributesFile(&oa, &fbi);

		retval = NT_SUCCESS(st);

		if (retval && (fbi.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			return TRUE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in FolderExists, FolderPath: [%s]", foldername);
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : FileExists
*  Description    : Function which Checks if file exists
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOL FileExists(PCWSTR filename)
{
	__try
	{
		UNICODE_STRING u_filename, nt_filename;
		FILE_BASIC_INFORMATION fbi;
		OBJECT_ATTRIBUTES oa;
		NTSTATUS st;

		RtlCreateUnicodeString(&u_filename, filename);
		
		PWSTR					 PartName;
		CURDIR					 RelativeName;
		if (!RtlDosPathNameToNtPathName_U(u_filename.Buffer, &nt_filename, (PWSTR*)&PartName, &RelativeName)) 
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed RtlDosPathNameToNtPathName_U in FileExists : [%s]\n", nt_filename.Buffer);
			//DbgPrint("### Failed RtlDosPathNameToNtPathName_U inFileExists : [%wZ]\n", &nt_filename);
			return FALSE;
		}

		RtlFreeUnicodeString(&u_filename);

		InitializeObjectAttributes(&oa, &nt_filename, OBJ_CASE_INSENSITIVE, 0, 0);
		st = NtQueryAttributesFile(&oa, &fbi);
		if (!NT_SUCCESS(st))
		{
			RtlAddLogEntry(ZEROLEVEL, L"### Failed NtQueryAttributesFile in FileExists : [%s]\n", nt_filename.Buffer);
			//DbgPrint("### Failed NtQueryAttributesFile in FileExists : [%wZ]\n", &nt_filename);
			return false;
		}
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in FileExists, FilePath: [%s]", filename);
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : SetUnicodeString
*  Description    : Function which creates unicode string from WCHAR string
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN SetUnicodeString(UNICODE_STRING* pustrRet, WCHAR* pwszData)
{
	__try
	{
		if (pustrRet == NULL || pwszData == NULL) {
			return FALSE;
		}

		pustrRet->Buffer = pwszData;
		pustrRet->Length = (USHORT)wcslen(pwszData) * sizeof(WCHAR);
		pustrRet->MaximumLength = pustrRet->Length + sizeof(WCHAR);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in SetUnicodeString");
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : InitHeapMemory
*  Description    : Function which creates creats heap memory
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
HANDLE InitHeapMemory(void)
{
	HANDLE hHeap = NULL;
	__try
	{
		RTL_HEAP_PARAMETERS		sHeapDef;
 
		// Init Heap Memory
		memset(&sHeapDef, 0, sizeof(RTL_HEAP_PARAMETERS));
		sHeapDef.Length = sizeof(RTL_HEAP_PARAMETERS);
		hHeap = RtlCreateHeap(HEAP_GROWABLE, NULL, 0x100000, 0x1000, NULL, &sHeapDef);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in InitHeapMemory");
		hHeap = RtlProcessHeap();
	}
	return hHeap;
}

/***********************************************************************************************
*  Function Name  : DeinitHeapMemory
*  Description    : Function which deinitialize already created heap memory
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN DeinitHeapMemory(HANDLE hHeap)
{
	__try
	{
		PVOID pRet;
		pRet = (PVOID)RtlDestroyHeap(hHeap);
		if (pRet == NULL)
			return TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in DeinitHeapMemory");
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : kmalloc
*  Description    : Function which allocated required amout of memory from heap
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
PVOID kmalloc(HANDLE hHeap, int nSize)
{
	PVOID pRet = NULL;
	__try
	{
		// if you wanna set new memory to zero, use HEAP_ZERO_MEMORY.
		pRet = RtlAllocateHeap(hHeap, 0, nSize);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in kmalloc");
	}
	return pRet;
}

/***********************************************************************************************
*  Function Name  : kfree
*  Description    : Function which deallocates memory from heap
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN kfree(HANDLE hHeap, PVOID pMemory)
{
	BOOLEAN bRet = FALSE;
	__try
	{
		bRet = RtlFreeHeap(hHeap, 0, pMemory);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in kfree");
	}
	return bRet;
}

/***********************************************************************************************
*  Function Name  : AppendString
*  Description    : Function which appends string to string
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN AppendString(WCHAR* pszInput, WCHAR* pszAppend)
{
	__try
	{
		int i, nAppendIndex;

		for (i = 0;; i++){
			if (pszInput[i] == 0x0000) {
				break;
			}
		}

		nAppendIndex = 0;
		for (;;) {
			if (pszAppend[nAppendIndex] == 0x0000) {
				break;
			}
			pszInput[i] = pszAppend[nAppendIndex];

			nAppendIndex++;
			i++;
		}

		pszInput[i] = 0x0000; // set end of string.
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in AppendString");
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : GetStringLength
*  Description    : Function which retrives string lengths
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
UINT GetStringLength(WCHAR* pszInput)
{
	int i = 0;
	__try
	{
		for (i = 0;; i++){
			if (pszInput[i] == 0x0000) {
				break;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in GetStringLength");
	}
	return i;
}

/***********************************************************************************************
*  Function Name  : FillUnicodeStringWithAnsi
*  Description    : This function allocates memory for "us" variable.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
void FillUnicodeStringWithAnsi(OUT PUNICODE_STRING us, IN PCHAR as)
{
	__try
	{
		ANSI_STRING ansi_string;

		RtlInitAnsiString(&ansi_string, as);

		if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(us, &ansi_string, TRUE)))
		{
			RtlAddLogEntry(SECONDLEVEL, L"RtlAnsiStringToUnicodeString() failed\n");
			return;
		}

		//Bug fixed, as it crashes on Windows Vista and onwards.
		//RtlFreeAnsiString(&ansi_string);
		return;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in FillUnicodeStringWithAnsi");
	}
}

/***********************************************************************************************
*  Function Name  : RtlGetEnvironmentVariable
*  Description    : Function which loads ntdll.dll and get the program files path
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
bool RtlGetEnvironmentVariable(PWSTR VariableName, PWSTR pszWinDir, DWORD dwSize)
{
	__try
	{
		if (!VariableName || !pszWinDir || dwSize == 0)
			return false;

		typedef NTSTATUS(NTAPI *QUERYENVIRONMENT)(DWORD, PUNICODE_STRING, PUNICODE_STRING);

		HMODULE moduleHandle = NULL;
		UNICODE_STRING moduleName;
		RtlInitUnicodeString(&moduleName, L"ntdll.dll");
		LdrGetDllHandle(NULL, NULL, &moduleName, (PVOID*)&moduleHandle);

		if (moduleHandle == NULL)
		{
			RtlAddLogEntry(SECONDLEVEL, L"#### NTDLL.DLL not loaded");
			return false;
		}

		ANSI_STRING asQueryEnvironmentVariable;
		RtlInitAnsiString(&asQueryEnvironmentVariable, "RtlQueryEnvironmentVariable_U");

		QUERYENVIRONMENT RtlQueryEnvironmentVariable_U = NULL;
		(QUERYENVIRONMENT)LdrGetProcedureAddress(moduleHandle, &asQueryEnvironmentVariable, 0, (PVOID*)&RtlQueryEnvironmentVariable_U);

		if (RtlQueryEnvironmentVariable_U == NULL)
		{
			RtlAddLogEntry(SECONDLEVEL, L"#### RtlQueryEnvironmentVariable_U function does not found");
			return false;
		}

		DWORD Return = 0;
		WCHAR              Temp[512] = { 0 };
		UNICODE_STRING     str, OutputString;

		//Set up output variable.
		OutputString.Buffer = Temp;
		OutputString.Length = 512;
		OutputString.MaximumLength = sizeof(Temp);
		RtlInitUnicodeString(&str, VariableName);
		Return = RtlQueryEnvironmentVariable_U(NULL, &str, &OutputString);

		if (dwSize < wcslen(OutputString.Buffer))
		{
			if (moduleHandle != NULL)
			{
				LdrUnloadDll(moduleHandle);
			}
			return false;
		}

		wcscpy(pszWinDir, OutputString.Buffer);
		if (moduleHandle != NULL)
		{
			LdrUnloadDll(moduleHandle);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlGetEnvironmentVariable");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : RtlGetCurrentDir
*  Description    : Function which loads ntdll.dll and get current directory
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
bool RtlGetCurrentDir(PWSTR pszWinDir, DWORD dwSize)
{
	__try
	{
		if (!pszWinDir || dwSize == 0)
			return false;

		typedef NTSTATUS(NTAPI *GETCURRENTDIR)(ULONG, LPWSTR);
		
		HMODULE moduleHandle = NULL;
		UNICODE_STRING moduleName;
		RtlInitUnicodeString(&moduleName, L"ntdll.dll");
		LdrGetDllHandle(NULL, NULL, &moduleName, (PVOID*)&moduleHandle);

		if (moduleHandle == NULL)
		{
			RtlAddLogEntry(SECONDLEVEL, L"#### NTDLL.DLL not loaded");
			return false;
		}

		ANSI_STRING asGetCurrentDirectory;
		RtlInitAnsiString(&asGetCurrentDirectory, "RtlGetCurrentDirectory_U");

		GETCURRENTDIR RtlQueryEnvironmentVariable_U = NULL;
		(GETCURRENTDIR)LdrGetProcedureAddress(moduleHandle, &asGetCurrentDirectory, 0, (PVOID*)&RtlQueryEnvironmentVariable_U);

		if (RtlQueryEnvironmentVariable_U == NULL)
		{
			RtlAddLogEntry(SECONDLEVEL, L"#### RtlGetCurrentDirectory_U function does not found");
			if (moduleHandle != NULL)
			{
				LdrUnloadDll(moduleHandle);
			}
			return false;
		}

		NTSTATUS ntStatus = RtlQueryEnvironmentVariable_U(dwSize, pszWinDir);
		if (!NT_SUCCESS(ntStatus))
		{
			if (moduleHandle != NULL)
			{
				LdrUnloadDll(moduleHandle);
			}
			return false;
		}

		if (moduleHandle != NULL)
		{
			LdrUnloadDll(moduleHandle);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlGetCurrentDir");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : RegisterProcessId
*  Description    : Function which register @processID to Protection diver
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           :  27-Sep-2017
*************************************************************************************************/
bool RegisterProcessId(ULONG processID)
{
	bool bReturn = false;
	HANDLE testEvent = NULL;
	REGISTER_PROCESS_ID *pStruct = NULL;
	HANDLE mmgrHandle = NULL;
	__try
	{
		OBJECT_ATTRIBUTES mmgrObjectAttributes;
		NTSTATUS status;
		IO_STATUS_BLOCK iosb;
		ULONG ulIndex = processID;

		//
		// First, we need to obtain a handle to the mount manager, so we must:
		//
		//  - Initialize the unicode string with the mount manager name
		//  - Build an object attributes structure
		//  - Open the mount manager
		//
		// This should yield a valid handle for calling the mount manager
		//
		// Initialize the unicode string with the mount manager's name
		//
		UNICODE_STRING			 NtName;
		RtlInitUnicodeString(&NtName, szPipeScannerProt);

		//
		// Initialize object attributes.
		//
		mmgrObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
		mmgrObjectAttributes.RootDirectory = NULL;
		mmgrObjectAttributes.ObjectName = &NtName;

		// Note: in a kernel driver, we’d add OBJ_KERNEL_HANDLE
		// as another attribute.
		mmgrObjectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
		mmgrObjectAttributes.SecurityDescriptor = NULL;
		mmgrObjectAttributes.SecurityQualityOfService = NULL;
		
		//
		// Open the mount manager
		//
		status = NtCreateFile(&mmgrHandle,
			FILE_READ_DATA | FILE_WRITE_DATA,
			&mmgrObjectAttributes,
			&iosb,
			0, // allocation is meaningless
			0, // no attributes specified
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // we're willing to share
			FILE_OPEN, // must already exist
			FILE_NON_DIRECTORY_FILE, // must NOT be a directory
			NULL, // no EA buffer
			0); // no EA buffer size...

		if (!NT_SUCCESS(status) ||
			!NT_SUCCESS(iosb.Status)) {
			//DbgPrint("\n Unable to open %wZ, error = 0x%x\n", &NtName, status);
			return false;
		}

		//
		// If we get to here, we assume it was successful.  We need an event object
		// for monitoring the completion of I/O operations.
		//
		status = NtCreateEvent(&testEvent,
			GENERIC_ALL,
			0, // no object attributes
			NotificationEvent,
			FALSE);

		if (!NT_SUCCESS(status)) {
			//
			// Bummer.
			//
			//DbgPrint("\n Cannot create event (0x%x)\n", status);
			bReturn = false;
			goto CLEANUP;
		}

		if (mmgrHandle)
		{

			pStruct = (REGISTER_PROCESS_ID*)RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, sizeof(REGISTER_PROCESS_ID));
			if (pStruct != NULL)
			{
				memset(pStruct, 0, sizeof(REGISTER_PROCESS_ID));
				memcpy(pStruct->SecureString, SECURE_STRING, sizeof(SECURE_STRING));
				pStruct->ProcessIdCode = ulIndex;

				status = NtDeviceIoControlFile(mmgrHandle,
					testEvent,
					0, // no apc
					0, // no apc context
					&iosb,
					IOCTL_REGISTER_PROCESSID,
					pStruct, // input buffer
					sizeof(REGISTER_PROCESS_ID), // size of input buffer
					pStruct, // output buffer
					sizeof(REGISTER_PROCESS_ID)); // size of output buffer

				if (STATUS_PENDING == status) {
					// Must wait for the I/O operation to complete
					//
					status = NtWaitForSingleObject(testEvent, TRUE, 0);
					if (NT_SUCCESS(status)) {
						status = iosb.Status;
					}
				}
				//
				// Regardless of the results, we are done with the mount manager and event
				// handles so discard them.
				//
				if (!NT_SUCCESS(status)) {
					//DbgPrint("\n DeviceIoControlFile failed 0x%x\n", status);
					bReturn = false;
					goto CLEANUP;
				}

				bReturn = true;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RegisterProcessId, ProcessID: [%d]", processID);
		bReturn = false;
	}
CLEANUP:
	if (testEvent != NULL)
	{
		NtClose(testEvent);
		testEvent = NULL;
	}

	if (mmgrHandle != NULL)
	{
		ZwClose(mmgrHandle);
		mmgrHandle = NULL;
	}

	if (pStruct != NULL)
	{
		if (!RtlFreeHeap(RtlProcessHeap(), 0, pStruct))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlFreeHeap Failed in RegisterProcessId");
		}
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : AnsiToLower
*  Description    : Function which converts ANSI string to lower case
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 14-Nov-2017
*************************************************************************************************/
bool AnsiToLower(LPSTR lpString, int iLength)
{
	__try
	{
		if (!lpString || iLength == 0)
			return false;

		for (int i = 0; i < iLength; ++i) {
			lpString[i] = tolower(lpString[i]);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in AnsiToLower");
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : Replace
*  Description    : replaced text will be in buffer.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 14-Nov-2017
*************************************************************************************************/
void Replace(TCHAR* buffer, const TCHAR* source, const TCHAR* oldStr, const TCHAR* newStr)
{
	__try
	{
		if (buffer == NULL || source == NULL || oldStr == NULL || newStr == NULL) return;

		int slen = (int)wcslen(source);
		int olen = (int)wcslen(oldStr);
		int nlen = (int)wcslen(newStr);

		if (olen > slen) return;
		int ix = 0;

		for (int i = 0; i < slen; i++)
		{
			if (oldStr[0] == source[i])
			{
				bool found = true;
				for (int j = 1; j < olen; j++)
				{
					if (source[i + j] != oldStr[j])
					{
						found = false;
						break;
					}
				}

				if (found)
				{
					for (int j = 0; j < nlen; j++)
						buffer[ix++] = newStr[j];

					i += (olen - 1);
				}
				else
				{
					buffer[ix++] = source[i];
				}
			}
			else
			{
				buffer[ix++] = source[i];
			}
		}

		buffer[ix++] = '\0';

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Replace");
	}
}