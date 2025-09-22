/****************************************************************************
Program Name          : WardWizNTReg.cpp
Description           : This file is related for registry operation
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#include "WardWizNTReg.h"

/***********************************************************************************************
*  Function Name  : NtRegGetRootPath
*  Description    : Function which returns required registry root path which is required @NATIVE lvl.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
WCHAR* NtRegGetRootPath(H_KEY hkRoot)
{
	if (hkRoot == HKEY_LOCAL_MACHINE) {
		return L"\\Registry\\Machine";
	}
	else if (hkRoot == HKEY_CLASSES_ROOT) {
		return L"\\Registry\\Machine\\SOFTWARE\\Classes";
	}
	else if (hkRoot == HKEY_CURRENT_CONFIG) {
		return L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current";
	}
	else if (hkRoot == HKEY_USERS) {
		return L"\\Registry\\User";
	}
	return NULL;
}

/***********************************************************************************************
*  Function Name  : NtRegOpenKey
*  Description    : Function which open registry key.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtRegOpenKey(HANDLE* phKey, H_KEY hkRoot, WCHAR* pwszSubKey, ACCESS_MASK DesiredAccess)
{
	NTSTATUS				nRet = 0;
	HANDLE					hReg = 0;
	UNICODE_STRING			ustrKeyName;
	WCHAR					wszKeyName[4096] = { 0, };
	WCHAR*					pwszRootKey = NULL;
	OBJECT_ATTRIBUTES		ObjectAttributes;

	pwszRootKey = NtRegGetRootPath(hkRoot);
	if (!pwszRootKey) {
		return FALSE;
	}

	// Set RootKey
	AppendString(wszKeyName, pwszRootKey);

	// Set SubKey
	AppendString(wszKeyName, L"\\");
	AppendString(wszKeyName, pwszSubKey);

	// Setup Unicode String
	SetUnicodeString(&ustrKeyName, wszKeyName);

	InitializeObjectAttributes(&ObjectAttributes,
		&ustrKeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);


	nRet = NtOpenKey(phKey,
		DesiredAccess,
		&ObjectAttributes);

	if (!NT_SUCCESS(nRet)) {
		RtlAddLogEntry(SECONDLEVEL, L"NtOpenKey Error : %X\n", nRet);
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtRegWriteValue
*  Description    : Function which write regisry key value.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtRegWriteValue(HANDLE hKey, WCHAR* pwszValueName, PVOID pData, ULONG uLength, DWORD dwRegType)
{
	UNICODE_STRING			ustrValueName;
	NTSTATUS				nRet;

	SetUnicodeString(&ustrValueName, pwszValueName);

	nRet = NtSetValueKey(hKey,
		&ustrValueName,
		0,
		dwRegType,		// i.e. REG_BINARY
		pData,
		uLength);

	//printf("NtRegWriteValue : %X\n", nRet);
	if (!NT_SUCCESS(nRet))
	{
		return FALSE;
	}

	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtRegWriteString
*  Description    : Function which write regisry key string value.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtRegWriteString(HANDLE hKey, WCHAR* pwszValueName, WCHAR* pwszValue)
{
	BOOLEAN bRet = FALSE;

	bRet = NtRegWriteValue(
		hKey, pwszValueName,
		pwszValue, (GetStringLength(pwszValue) + 1) * sizeof(WCHAR), REG_SZ
		);

	return bRet;
}

/***********************************************************************************************
*  Function Name  : NtRegDeleteValue
*  Description    : Function which delete regisry key value.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtRegDeleteValue(HANDLE hKey, WCHAR* pwszValueName)
{
	UNICODE_STRING ustrValueName;
	int nRet = 0;

	SetUnicodeString(&ustrValueName, pwszValueName);

	nRet = NtDeleteValueKey(hKey, &ustrValueName);
	if (!NT_SUCCESS(nRet)) {
		return FALSE;
	}

	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtRegReadValue
*  Description    : Function which reads registry key value.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtRegReadValue(HANDLE hKey, HANDLE hHeapHandle, WCHAR* pszValueName, PKEY_VALUE_PARTIAL_INFORMATION* pRetBuffer, ULONG* pRetBufferSize)
{
	UNICODE_STRING ustrValueName;
	BYTE* pBuffer = NULL;
	ULONG uSize = 1024;
	ULONG uRetSize;
	NTSTATUS ntStatus = 0;
	int i = 0;

	SetUnicodeString(&ustrValueName, pszValueName);

	for (i = 0; i < 4096; i++) {
		pBuffer = (BYTE*)kmalloc(hHeapHandle, uSize);

		ntStatus = NtQueryValueKey(hKey, &ustrValueName, KeyValuePartialInformation, pBuffer, uSize, &uRetSize);

		if (ntStatus == STATUS_SUCCESS) {
			break;
		}
		else if (ntStatus == STATUS_INVALID_PARAMETER) {
			kfree(hHeapHandle, pBuffer);
			pBuffer = NULL;

			return FALSE;
		}

		kfree(hHeapHandle, pBuffer);
		pBuffer = NULL;

		uSize += 4;
	}

	*pRetBuffer = (PKEY_VALUE_PARTIAL_INFORMATION)pBuffer;
	*pRetBufferSize = uSize;

	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtRegCloseKey
*  Description    : Function which closes registry handle
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtRegCloseKey(HANDLE hKey)
{
	int nRet = 0;

	nRet = NtClose(hKey);

	if (!NT_SUCCESS(nRet)) {
		return FALSE;
	}

	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtEnumKey
*  Description    : Function which enumrates registry entries on given path.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
void NtEnumKey(HANDLE hKey)
{
	__try
	{
		NTSTATUS nRet = 0;
		char buf[BUFFER_SIZE];
		PKEY_VALUE_BASIC_INFORMATION pbi;
		PKEY_NODE_INFORMATION pki;
		ULONG ResultLength;
		UINT i;

		RtlCliDisplayString(SECONDLEVEL, L"=========\n");

		i = 0;
		memset(buf, 0x00, BUFFER_SIZE);
		pki = (PKEY_NODE_INFORMATION)buf;

		while (STATUS_SUCCESS == NtEnumerateKey(hKey, i++, KeyNodeInformation, pki, BUFFER_SIZE, &ResultLength))
		{
			if (pki->NameLength)
			{
				RtlCliDisplayString(SECONDLEVEL, L"[%S]\n", pki->Name);
			}
			else
			{
				RtlCliDisplayString(SECONDLEVEL, L"[null]\n");
			}
			memset(buf, 0x00, BUFFER_SIZE);
		}

		RtlCliDisplayString(SECONDLEVEL, L"---------\n");

		i = 0;
		memset(buf, 0x00, BUFFER_SIZE);
		pbi = (PKEY_VALUE_BASIC_INFORMATION)buf;

		while (STATUS_SUCCESS == NtEnumerateValueKey(hKey, i++, KeyValueBasicInformation, pbi, BUFFER_SIZE, &ResultLength))
		{
			RtlCliDisplayString(SECONDLEVEL, (pbi->Type == REG_SZ)
				? L" REG_SZ" : (pbi->Type == REG_MULTI_SZ)
				? L" REG_MULTI_SZ" : (pbi->Type == REG_DWORD)
				? L" REG_DWORD" : L"Other type (%d)", pbi->Type);
			if (pbi->NameLength)
			{
				RtlCliDisplayString(SECONDLEVEL, L"    %S\n", pbi->Name);
			}
			else
			{
				RtlCliDisplayString(SECONDLEVEL, L"    (null)\n");
			}
			memset(buf, 0x00, BUFFER_SIZE);
		}

		RtlCliDisplayString(SECONDLEVEL, L"=========\n");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return;
}