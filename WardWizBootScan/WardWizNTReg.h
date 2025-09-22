/****************************************************************************
Program Name          : WardWizNTReg.cpp
Description           : This file is related for registry operation
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#pragma once
#ifndef NATIVEREGISTRY_FUNCTIONS_H
#define NATIVEREGISTRY_FUNCTIONS_H 1

#pragma warning(disable:4005)

#include "NativeLogger.h"
#include "CommonFunctions.h"

#define HKEY_CLASSES_ROOT           0x80000000
#define HKEY_CURRENT_USER           0x80000001
#define HKEY_LOCAL_MACHINE          0x80000002
#define HKEY_USERS                  0x80000003
#define HKEY_PERFORMANCE_DATA       0x80000004
#define HKEY_PERFORMANCE_TEXT       0x80000050
#define HKEY_PERFORMANCE_NLSTEXT    0x80000060
#define HKEY_CURRENT_CONFIG         0x80000005
#define HKEY_DYN_DATA               0x80000006

typedef unsigned long H_KEY;

WCHAR* NtRegGetRootPath(H_KEY hkRoot);
BOOLEAN NtRegOpenKey(HANDLE* phKey, H_KEY hkRoot, WCHAR* pwszSubKey, ACCESS_MASK DesiredAccess);
BOOLEAN NtRegWriteValue(HANDLE H_KEY, WCHAR* pwszValueName, PVOID pData, ULONG uLength, DWORD dwRegType);
BOOLEAN NtRegWriteString(HANDLE H_KEY, WCHAR* pwszValueName, WCHAR* pwszValue);
BOOLEAN NtRegDeleteValue(HANDLE H_KEY, WCHAR* pwszValueName);
BOOLEAN NtRegCloseKey(HANDLE H_KEY);
BOOLEAN NtRegReadValue(HANDLE H_KEY, HANDLE hHeapHandle, WCHAR* pszValueName, PKEY_VALUE_PARTIAL_INFORMATION* pRetBuffer, ULONG* pRetBufferSize);
void NtEnumKey(HANDLE hKey);

#endif