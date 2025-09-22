/****************************************************************************
Program Name          : CommonFunctions.h
Description           : This file which contains all necessary common functions which
are required for boot time scanner
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#pragma once
#include "NativeLogger.h"
#include "WWizLinkedList.h"

#define SECURE_STRING "01010101010101010101010101010101"
#define IOCTL_REGISTER_PROCESSID     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define Maximum(a,b)            (((a) > (b)) ? (a) : (b))

typedef struct _REGISTER_PROCESS_ID
{
	ULONG ProcessIdCode;
	UCHAR SecureString[40];
}REGISTER_PROCESS_ID, *PREGISTER_PROCESS_ID;

char * xstrtok(char * s, const char * ct);
char * xstrtoktwo(char * s, const char * ct);
UINT StringToArguments(CHAR *str);
ULONG GetFileAttributesNt(PWSTR filename);
BOOL FolderExists(PCWSTR foldername);
BOOL FileExists(PCWSTR filename);
BOOLEAN SetUnicodeString(UNICODE_STRING* pustrRet, WCHAR* pwszData);
HANDLE InitHeapMemory(void);
BOOLEAN DeinitHeapMemory(HANDLE hHeap);
PVOID kmalloc(HANDLE hHeap, int nSize);
BOOLEAN kfree(HANDLE hHeap, PVOID pMemory);
BOOLEAN AppendString(WCHAR* pszInput, WCHAR* pszAppend);
UINT GetStringLength(WCHAR* pszInput);
void FillUnicodeStringWithAnsi(OUT PUNICODE_STRING us, IN PCHAR as);
bool RtlGetEnvironmentVariable(PWSTR VariableName, PWSTR pszWinDir, DWORD dwSize);
bool RtlGetCurrentDir(PWSTR pszWinDir, DWORD dwSize);
bool RegisterProcessId(ULONG processID);
bool AnsiToLower(LPSTR lpString, int iLength);
void Replace(TCHAR* buffer, const TCHAR* source, const TCHAR* oldStr, const TCHAR* newStr);
