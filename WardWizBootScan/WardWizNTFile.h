/****************************************************************************
Program Name          : WardWizNTFile.h
Description           : This file contains related file operation using Native API
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#pragma once
#ifndef NATIVEFILE_FUNCTIONS_H
#define NATIVEFILE_FUNCTIONS_H 1

#include "NativeLogger.h"

#define STATUS_OBJECT_NAME_COLLISION 0xC0000035

BOOLEAN NtFileOpenFile(HANDLE* phRetFile, WCHAR* pwszFileName, BOOLEAN bWrite, BOOLEAN bOverwrite, bool bErrorLog = true);
BOOLEAN NtFileOpenDirectory(HANDLE* phRetFile, WCHAR* pwszFileName, BOOLEAN bWrite, BOOLEAN bOverwrite);

BOOLEAN NtFileReadFile(HANDLE hFile, LPVOID pOutBuffer, DWORD dwOutBufferSize, DWORD* pRetReadedSize);
BOOLEAN NtFileWriteFile(HANDLE hFile, LPVOID lpData, DWORD dwBufferSize, DWORD* pRetWrittenSize);

BOOLEAN NtFileSeekFile(HANDLE hFile, LONGLONG lAmount);
BOOLEAN NtFileGetFilePosition(HANDLE hFile, LONGLONG* pRetCurrentPosition);
BOOLEAN NtFileGetFileSize(HANDLE hFile, LONGLONG* pRetFileSize);

BOOLEAN NtFileCloseFile(HANDLE hFile);

BOOLEAN NtFileCopyFile(WCHAR* pszSrc, WCHAR* pszDst);

BOOLEAN NtFileDeleteFile(PWSTR filename);
BOOLEAN NtFileCreateDirectory(PWSTR dirname);

BOOLEAN NtFileMoveFile(IN LPCWSTR lpExistingFileName, IN LPCWSTR lpNewFileName, BOOLEAN ReplaceIfExists);
#endif