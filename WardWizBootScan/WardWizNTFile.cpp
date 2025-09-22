/****************************************************************************
Program Name          : WardWizNTFile.cpp
Description           : This file contains related file operation using Native API
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#include "WardWizNTFile.h"

/***********************************************************************************************
*  Function Name  : NtFileOpenDirectory
*  Description    : Function to query directory.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileOpenDirectory(HANDLE* phRetFile, WCHAR* pwszFileName, BOOLEAN bWrite, BOOLEAN bOverwrite)
{
	__try
	{
		if (pwszFileName == NULL || phRetFile == NULL)
			return FALSE;

		HANDLE hFile;
		UNICODE_STRING ustrFileName;
		IO_STATUS_BLOCK IoStatusBlock;
		ULONG CreateDisposition = 0;
		WCHAR wszFileName[1024] = L"\\??\\";
		OBJECT_ATTRIBUTES ObjectAttributes;

		wcscat(wszFileName, pwszFileName);

		RtlInitUnicodeString(&ustrFileName, wszFileName);

		InitializeObjectAttributes(&ObjectAttributes,
			&ustrFileName,
			OBJ_CASE_INSENSITIVE,
			NULL,
			NULL);

		if (bWrite)
		{
			if (bOverwrite)
			{
				CreateDisposition = FILE_OVERWRITE_IF;
			}
			else
			{
				CreateDisposition = FILE_OPEN_IF;
			}
		}
		else
		{
			CreateDisposition = FILE_OPEN;
		}

		NtCreateFile(&hFile,
			FILE_LIST_DIRECTORY | SYNCHRONIZE | FILE_OPEN_FOR_BACKUP_INTENT,
			&ObjectAttributes,
			&IoStatusBlock,
			0,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			FILE_CREATE,
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
			NULL,
			0);

		*phRetFile = hFile;

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileOpenDirectory, FilePath: [%s]", pwszFileName);
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtFileOpenFile
*  Description    : Function to open file, as per provided parameters
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileOpenFile(HANDLE* phRetFile, WCHAR* pwszFileName, BOOLEAN bWrite, BOOLEAN bOverwrite, bool bErrorLog)
{
	__try
	{
		HANDLE hFile;
		UNICODE_STRING ustrFileName;
		IO_STATUS_BLOCK IoStatusBlock;
		ULONG CreateDisposition = 0;
		WCHAR wszFileName[1024] = L"\\??\\";
		OBJECT_ATTRIBUTES ObjectAttributes;
		NTSTATUS ntStatus;

		wcscat(wszFileName, pwszFileName);

		RtlInitUnicodeString(&ustrFileName, wszFileName);

		InitializeObjectAttributes(&ObjectAttributes,
			&ustrFileName,
			OBJ_CASE_INSENSITIVE,
			NULL,
			NULL);

		if (bWrite)
		{
			if (bOverwrite)
			{
				CreateDisposition = FILE_OVERWRITE_IF;
			}
			else
			{
				CreateDisposition = FILE_OPEN_IF;
			}
		}
		else
		{
			CreateDisposition = FILE_OPEN;
		}

		ntStatus = NtCreateFile(&hFile, GENERIC_WRITE | SYNCHRONIZE | GENERIC_READ,
			&ObjectAttributes, &IoStatusBlock, 0, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			CreateDisposition, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

		if (!NT_SUCCESS(ntStatus))
		{
			//DbgPrint("\nNtCreateFile() failed 0x%.8X, FilePath: [%S]", ntStatus, pwszFileName);
			if (bErrorLog)
			{
				RtlAddLogEntry(SECONDLEVEL, L"NtCreateFile() failed 0x%.8X, FilePath: [%s]", ntStatus, pwszFileName);
			}
			return FALSE;
		}

		*phRetFile = hFile;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (bErrorLog)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileOpenFile, FilePath: [%s]", pwszFileName);
		}
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtFileWriteFile
*  Description    : Function to write buffer in file
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileWriteFile(HANDLE hFile, LPVOID lpData, DWORD dwBufferSize, DWORD* pRetWrittenSize)
{
	__try
	{
		IO_STATUS_BLOCK sIoStatus;
		NTSTATUS ntStatus = 0;

		memset(&sIoStatus, 0, sizeof(IO_STATUS_BLOCK));

		ntStatus = NtWriteFile(hFile, NULL, NULL, NULL, &sIoStatus, lpData, dwBufferSize, NULL, NULL);

		if (ntStatus == STATUS_SUCCESS)
		{
			if (pRetWrittenSize)
			{
				*pRetWrittenSize = (DWORD)sIoStatus.Information;
			}
			return TRUE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileOpenFile");
		return FALSE;
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : NtFileCopyFile
*  Description    : Function which accepts source and destination path and copies file buffer
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileCopyFile(WCHAR* pszSrc, WCHAR* pszDst)
{
	__try
	{
		HANDLE hSrc = NULL;
		HANDLE hDst = NULL;
		BYTE byData[8192];
		LONGLONG lFileSize = 0;
		LONGLONG lWrittenSizeTotal = 0;
		DWORD dwReadedSize = 0;
		DWORD dwWrittenSize = 0;
		BOOLEAN bResult = 0;

		bResult = NtFileOpenFile(&hSrc, pszSrc, FALSE, FALSE);
		if (bResult == FALSE)
		{
			return FALSE;
		}

		bResult = NtFileOpenFile(&hDst, pszDst, TRUE, TRUE);

		if (bResult == FALSE)
		{
			NtFileCloseFile(hSrc);
			return FALSE;
		}

		if (NtFileGetFileSize(hSrc, &lFileSize) == FALSE)
		{
			NtFileCloseFile(hSrc);
			NtFileCloseFile(hDst);
			return FALSE;
		}

		lWrittenSizeTotal = 0;
		while (1)
		{
			dwReadedSize = 0;

			if (NtFileReadFile(hSrc, byData, 8192, &dwReadedSize) == FALSE)
			{
				NtFileCloseFile(hSrc);
				NtFileCloseFile(hDst);
				return FALSE;
			}

			if (NtFileWriteFile(hDst, byData, dwReadedSize, &dwWrittenSize) == FALSE)
			{
				NtFileCloseFile(hSrc);
				NtFileCloseFile(hDst);
				return FALSE;
			}

			if (dwReadedSize != dwWrittenSize)
			{
				NtFileCloseFile(hSrc);
				NtFileCloseFile(hDst);
				return FALSE;
			}

			lWrittenSizeTotal += dwWrittenSize;
			if (lWrittenSizeTotal == lFileSize)
			{
				// End of File...
				break;
			}
		}

		NtFileCloseFile(hSrc);
		NtFileCloseFile(hDst);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileCopyFile, SourcePath: [%s], Dest: [%s]", pszSrc, pszDst);
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtFileReadFile
*  Description    : Function which read file buffere as with buffer size.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileReadFile(HANDLE hFile, LPVOID pOutBuffer, DWORD dwOutBufferSize, DWORD* pRetReadedSize)
{
	__try
	{
		if (hFile == NULL)
			return FALSE;

		IO_STATUS_BLOCK sIoStatus;
		NTSTATUS ntStatus = 0;

		memset(&sIoStatus, 0, sizeof(IO_STATUS_BLOCK));

		ntStatus = NtReadFile(hFile, NULL, NULL, NULL, &sIoStatus, pOutBuffer, dwOutBufferSize, NULL, NULL);
		if (ntStatus == STATUS_SUCCESS)
		{
			if (pRetReadedSize)
			{
				*pRetReadedSize = (DWORD)sIoStatus.Information;
			}

			return TRUE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileReadFile");
		return FALSE;
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : NtFileGetFilePosition
*  Description    : Function which returns current position in file.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileGetFilePosition(HANDLE hFile, LONGLONG* pRetCurrentPosition)
{
	__try
	{
		if (hFile == NULL)
			return FALSE;

		IO_STATUS_BLOCK sIoStatus;
		FILE_POSITION_INFORMATION sFilePosition;
		NTSTATUS ntStatus = 0;

		memset(&sIoStatus, 0, sizeof(IO_STATUS_BLOCK));
		memset(&sFilePosition, 0, sizeof(FILE_POSITION_INFORMATION));

		ntStatus = NtQueryInformationFile(hFile, &sIoStatus, &sFilePosition,
			sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
		if (ntStatus == STATUS_SUCCESS)
		{
			if (pRetCurrentPosition)
			{
				*pRetCurrentPosition = (sFilePosition.CurrentByteOffset.QuadPart);
			}
			return TRUE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileGetFilePosition");
		return FALSE;
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : NtFileGetFileSize
*  Description    : Function which returns file size.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileGetFileSize(HANDLE hFile, LONGLONG* pRetFileSize)
{
	__try
	{
		if (hFile == NULL)
			return FALSE;

		IO_STATUS_BLOCK sIoStatus;
		FILE_STANDARD_INFORMATION sFileInfo;
		NTSTATUS ntStatus = 0;

		memset(&sIoStatus, 0, sizeof(IO_STATUS_BLOCK));
		memset(&sFileInfo, 0, sizeof(FILE_STANDARD_INFORMATION));

		ntStatus = NtQueryInformationFile(hFile, &sIoStatus, &sFileInfo,
			sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
		if (ntStatus == STATUS_SUCCESS)
		{
			if (pRetFileSize)
			{
				*pRetFileSize = (sFileInfo.EndOfFile.QuadPart);
			}
			return TRUE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileGetFileSize");
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : NtFileSeekFile
*  Description    : Function which set file position
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileSeekFile(HANDLE hFile, LONGLONG lAmount)
{
	__try
	{
		if (hFile == NULL)
			return FALSE;

		IO_STATUS_BLOCK sIoStatus;
		FILE_POSITION_INFORMATION sFilePosition;
		NTSTATUS ntStatus = 0;

		memset(&sIoStatus, 0, sizeof(IO_STATUS_BLOCK));

		sFilePosition.CurrentByteOffset.QuadPart = lAmount;
		ntStatus = NtSetInformationFile(hFile, &sIoStatus, &sFilePosition,
			sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
		if (ntStatus == STATUS_SUCCESS)
		{
			return TRUE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileSeekFile");
		return FALSE;
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : NtFileCloseFile
*  Description    : Function which close file handle safely
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileCloseFile(HANDLE hFile)
{
	__try
	{
		if (hFile == NULL)
			return FALSE;

		NTSTATUS ntStatus = 0;
		ntStatus = NtClose(hFile);

		if (ntStatus == STATUS_SUCCESS)
		{
			return TRUE;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileCloseFile");
		return FALSE;
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : NtFileDeleteFile
*  Description    : Function which deletes specified file
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileDeleteFile(PWSTR filename)
{
	__try
	{
		if (filename == NULL)
			return FALSE;

		UNICODE_STRING us;
		NTSTATUS status;
		OBJECT_ATTRIBUTES oa;

		RtlInitUnicodeString(&us, filename);

		if (!RtlDosPathNameToNtPathName_U(filename, &us, NULL, NULL))
		{
			return FALSE;
		}

		InitializeObjectAttributes(&oa, &us, OBJ_CASE_INSENSITIVE, NULL, NULL);
		status = NtDeleteFile(&oa);

		if (!NT_SUCCESS(status))
		{
			RtlFreeUnicodeString(&us);
			return FALSE;
		}

		RtlFreeUnicodeString(&us);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileDeleteFile, FilePath: [%s]", filename);
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : NtFileCreateDirectory
*  Description    : Function which creates directory on given path
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileCreateDirectory(PWSTR dirname)
{
	__try
	{
		UNICODE_STRING us;
		NTSTATUS status;
		HANDLE hFile;
		OBJECT_ATTRIBUTES oa;
		IO_STATUS_BLOCK iosb;

		if (!RtlDosPathNameToNtPathName_U(dirname, &us, NULL, NULL))
		{
			return FALSE;
		}

		InitializeObjectAttributes(&oa, &us, OBJ_CASE_INSENSITIVE, NULL, NULL);

		status = NtCreateFile(&hFile,
			FILE_LIST_DIRECTORY | SYNCHRONIZE | FILE_OPEN_FOR_BACKUP_INTENT,
			&oa,
			&iosb,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			FILE_CREATE,
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
			NULL,
			0
			);

		if (NT_SUCCESS(status))
		{
			NtClose(hFile);
			RtlFreeUnicodeString(&us);
			return TRUE;
		}

		/* if it already exists then return success */
		if (status == STATUS_OBJECT_NAME_COLLISION)
		{
			RtlFreeUnicodeString(&us);
			return TRUE;
		}

		RtlFreeUnicodeString(&us);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileCreateDirectory, FilePath: [%s]", dirname);
		return FALSE;
	}
	return FALSE;
}

/***********************************************************************************************
*  Function Name  : NtFileMoveFile
*  Description    : Function which moves file from one location to another
					lpExistingFileName - full path in DOS format
					lpNewFileName - full path in DOS format, or filename
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           :  27-Sep-2017
*************************************************************************************************/
BOOLEAN NtFileMoveFile(IN LPWSTR lpExistingFileName, IN LPCWSTR lpNewFileName, BOOLEAN ReplaceIfExists)
{
	__try
	{
		PFILE_RENAME_INFORMATION FileRenameInfo;
		OBJECT_ATTRIBUTES ObjectAttributes;
		IO_STATUS_BLOCK IoStatusBlock;
		UNICODE_STRING ExistingFileNameU;
		WCHAR NewFileName[MAX_FILEPATH_LENGTH] = L"\\??\\";
		HANDLE FileHandle;
		DWORD FileNameSize;

		NTSTATUS Status;

		if (!lpExistingFileName || !lpNewFileName)
		{
			return FALSE;
		}

		RtlDosPathNameToNtPathName_U(lpExistingFileName, &ExistingFileNameU, NULL, NULL);

		if ((wcslen(lpNewFileName) > 2) && L':' == lpNewFileName[1])
		{
			wcsncat(NewFileName, lpNewFileName, MAX_FILEPATH_LENGTH);
		}
		else
		{
			wcsncpy(NewFileName, lpNewFileName, MAX_FILEPATH_LENGTH);
		}

		InitializeObjectAttributes(&ObjectAttributes,
			&ExistingFileNameU,
			OBJ_CASE_INSENSITIVE,
			NULL,
			NULL);

		Status = NtCreateFile(&FileHandle,
			FILE_ALL_ACCESS,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			FILE_OPEN,
			FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);

		if (!NT_SUCCESS(Status))
		{
			RtlAddLogEntry(SECONDLEVEL, L"NtCreateFile() failed (Status %lx)\n", Status);
			return FALSE;
		}

		FileNameSize = (DWORD)wcslen(NewFileName) * sizeof(*NewFileName);

		FileRenameInfo = (PFILE_RENAME_INFORMATION)RtlAllocateHeap(RtlProcessHeap(),
			HEAP_ZERO_MEMORY, sizeof(FILE_RENAME_INFORMATION) + FileNameSize);

		if (!FileRenameInfo)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlAllocateHeap failed in NtFileMoveFile\n");
			NtClose(FileHandle);
			return FALSE;
		}

		FileRenameInfo->RootDirectory = NULL;
		FileRenameInfo->ReplaceIfExists = ReplaceIfExists;
		FileRenameInfo->FileNameLength = FileNameSize;
		RtlCopyMemory(FileRenameInfo->FileName, NewFileName, FileNameSize);

		Status = NtSetInformationFile(
			FileHandle,
			&IoStatusBlock,
			FileRenameInfo,
			sizeof(FILE_RENAME_INFORMATION) + FileNameSize,
			FileRenameInformation);

		RtlFreeHeap(RtlProcessHeap(), 0, FileRenameInfo);

		NtClose(FileHandle);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in NtFileMoveFile, Existing: [%s], NewFileName: [%s]", lpExistingFileName, lpNewFileName);
		return FALSE;
	}
	return TRUE;
}

