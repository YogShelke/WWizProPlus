/**********************************************************************************************************
Program Name          : DOCScanner.cpp
Description           : Class for DOC file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 16th Mar 2016
Version No            :
Special Logic Used    : This class is created for the scanning of DOC type of files

Modification Log      :
1. Sanjay	          : Created Class CDOCScanner       16 - 03 - 2016
***********************************************************************************************************/

#include "StdAfx.h"
#include "DocScanner.h"
#include <math.h>

/***************************************************************************************************
*  Function Name  : CDocScanner
*  Description    : Cont'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 16 Mar 2016
****************************************************************************************************/
CDocScanner::CDocScanner() : m_hFileHandle(INVALID_HANDLE_VALUE)
							,m_bDOCSignatureLoaded(false)
{
	memset(&m_szDOCFilePath, 0, sizeof(m_szDOCFilePath));
	m_pGetByteStringHash = NULL;
	m_csDBFilePath = GetWardWizPathFromRegistry();
	m_csDBFilePath += L"VBHASH.DLL";
	m_hInstHashDll = LoadLibrary(m_csDBFilePath);
}

/***************************************************************************************************
*  Function Name  : ~CDocScanner
*  Description    : Dest'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 16 Mar 2016
****************************************************************************************************/
CDocScanner::~CDocScanner()
{
	if (!m_MD5DOCScanner.UnLoadMD5Data())
	{
		AddLogEntry(L"### Failed to call UnloadMD5Data in CDOCScanner::~CDOCScanner", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : LoadDOCSignatures
*  Description    : Function to load signatures for DOC file type
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 16 Mar 2016
****************************************************************************************************/
bool CDocScanner::LoadDOCSignatures(DWORD &dwSigCount)
{
	bool bReturn = false;

	__try
	{
		if (!m_MD5DOCScanner.LoadMD5Data(FILE_DOC, dwSigCount))
		{
			AddLogEntry(L"### Failed to call LoadMD5Data for DOC type in CDocScanner::LoadDOCSignatures", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}
		m_pGetByteStringHash = (GetByteStringHashFunc)GetProcAddress(m_hInstHashDll, "GetByteStringHash");
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDocScanner::LoadDOCSignatures", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	
	return bReturn;
}


/***********************************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to scan the DOC file, where the scanning will start only if the DOC signatures are loaded
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 19 Mar 2016
************************************************************************************************************************/
bool CDocScanner::ScanFile(LPCTSTR pszFilePath, LPTSTR &pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool bReturn = false;

	__try
	{
		if (!m_bDOCSignatureLoaded)
		{
			DWORD dwSigCount = 0x00;
			m_bDOCSignatureLoaded = LoadDOCSignatures(dwSigCount);
		}

		if (m_bDOCSignatureLoaded)
			bReturn = ScanFileSEH(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_DOC);
		
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### GetFileVersionInfoSize Failed, file Path: %s", m_szDOCFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ScanFileSEH
*  Description    : Function to lead the actual scanning to a common class for scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 19 Mar 2016
****************************************************************************************************/
bool CDocScanner::ScanFileSEH(LPCTSTR pszFilePath, LPTSTR & pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool	bRepairable = false;
	DWORD ctBeginTime, ctEndTime;
	ctBeginTime = GetTickCount();

	try
	{
		_tcscpy_s(m_szDOCFilePath, pszFilePath);

		if (!EnumerateDirEntriesforExtractionAndScan(pszVirusName, dwSpyID, bRescan, bRepairable))
		{
			return false;
		}
		ctEndTime = GetTickCount();
		DWORD ctDiffOfTime = ctEndTime - ctBeginTime;
		CString csDiffOfTime;

		csDiffOfTime.Format(L"%lu milisecs", ctDiffOfTime);
		AddLogEntry(L">>> CDocScanner::ScanFileSEH took %s for %s", csDiffOfTime, m_szDOCFilePath, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### CDocScanner::ScanFileSEH failed, FilePath: %s", pszFilePath, 0, true, SECONDLEVEL);
	}

	return true;
}


/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Function to know how to many bytes are read from the DOC file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 19 Mar 2016
****************************************************************************************************/
bool CDocScanner::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
{
	__try
	{
		if (!m_hFileHandle)
		{
			return false;
		}
		DWORD dwBytesRead = 0x00;
		DWORD dwSetFileOffSet = ::SetFilePointer(m_hFileHandle, dwReadOffset, NULL, FILE_BEGIN);
		if (dwSetFileOffSet == dwReadOffset)
		{
			if (ReadFile(m_hFileHandle, pbReadBuffer, dwBytesToRead, &dwBytesRead, NULL))
			{
				if (dwBytesRead && dwBytesRead >= dwMinBytesReq)
				{
					if (pdwBytesRead)
					{
						*pdwBytesRead = dwBytesRead;
					}
					return true;
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDocScanner::GetFileBuffer", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/***************************************************************************************************
*  Function Name  : EnumerateDirEntriesforExtractionAndScan
*  Description    : Function to scan each and every DOC file from the directory
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 19 Mar 2016
****************************************************************************************************/
bool CDocScanner::EnumerateDirEntriesforExtractionAndScan(LPTSTR & lpszVirusName, DWORD &dwVirusID, bool bRescan, bool &bRepairable)
{
	bool bReturn = false;

	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;
	DWORD dwBytesRead = 0;
	
	try
	{
		STRUCTUREDSTORAGEDIRECTORYENTRY		stDirectoryEntries[0x20] = { 0 };
		DWORD	dwDirEntriesSize = sizeof(stDirectoryEntries);

		GetFileBuffer(stDirectoryEntries, (m_dwDirectoryEntryOffset*m_dwSectorSize + m_dwSectorSize), dwDirEntriesSize, 0, &dwBytesRead);
		if (!dwBytesRead)
		{
			goto Cleanup;
		}

		DWORD	dwEntries = dwBytesRead / sizeof(STRUCTUREDSTORAGEDIRECTORYENTRY);
		DWORD	dwIndex = 0x00;

		DWORD	dwOffset = 0x00;
		BYTE	bStoredEntryData[0x5000] = { 0 };
		DWORD	dwReadBytes = 0x00;

		for (; dwIndex < dwEntries; dwIndex++)
		{
			if (stDirectoryEntries[dwIndex].bEleType != 0x02)
				continue;

			dwOffset = dwBytesRead = 0x00;

			if (stDirectoryEntries[dwIndex].dwStartSector)
			{
				dwOffset = stDirectoryEntries[dwIndex].dwStartSector*m_dwSectorSize + m_dwSectorSize;
			}
			else
			{
				//We need to check this 
				dwOffset = m_dwDirectoryEntryOffset*m_dwSectorSize + m_dwSectorSize +
					sizeof(STRUCTUREDSTORAGEDIRECTORYENTRY)*(dwIndex + 2);

				dwOffset = dwOffset - (dwOffset%m_dwSectorSize) + m_dwSectorSize + m_dwSectorSize;
				if (dwOffset == m_dwFileSize)
					dwOffset = m_dwSectorSize;
			}

			dwReadBytes = stDirectoryEntries[dwIndex].dwSizeLow;

			if (!dwReadBytes)
				continue;

			if (stDirectoryEntries[dwIndex].dwSizeLow > 0x5000)
				dwReadBytes = 0x5000;

			GetFileBuffer(bStoredEntryData, dwOffset, dwReadBytes, 0, &dwBytesRead);
			if (!dwBytesRead)
				continue;

			BYTE	bSectionsHash[0x10] = { 0 };
			if (!m_pGetByteStringHash)
			{
				goto Cleanup;				
			}

			DWORD dwRet = m_pGetByteStringHash(bStoredEntryData, dwReadBytes, bSectionsHash);
			if (dwRet)
			{
				goto Cleanup;
			}
				
			//matching of bSectionsHash with our database
			//if matched, generate virus name as per our logic and return
			//add here to scan buffer
			bool bRet = m_MD5DOCScanner.ScanFile(bStoredEntryData, dwReadBytes, dwVirusID, bRescan, FILE_DOC);
			char szVirusName[MAX_PATH] = { 0 };
			CString csVirusName = L"";
			if ((dwVirusID > 0) && (bRet))
			{
				csVirusName.Format(L"W32.Macro-%lu", dwVirusID);
				_tcscpy_s(lpszVirusName, MAX_PATH, csVirusName);
				dwVirusID = 0;
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in EnumerateDirEntriesforExtractionAndScan", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}

Cleanup:
	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : IsValidDOCFile
*  Description    : Function to check whether a file being scanned is a DOC file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 16 Mar 2016
****************************************************************************************************/
bool CDocScanner::IsValidDOCFile(LPCTSTR csFilePath)
{
	bool bReturn = false;
	__try
	{
		m_hFileHandle = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			goto Cleanup;
		}

		m_dwFileSize = GetFileSize(m_hFileHandle, NULL);

		if ((m_dwFileSize == 0xFFFFFFFF) ||
			(m_dwFileSize == 0x00)
			)
		{
			goto Cleanup;
		}

		BYTE	bHeader[0x0A] = { 0 };
		BYTE	bDocHeader[0x08] = { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1 };

		DWORD dwBytesRead = 0;

		if (!GetFileBuffer(bHeader, 0, sizeof(bHeader), sizeof(bHeader)))
		{
			goto Cleanup;
		}

		if (_memicmp(bHeader, bDocHeader, sizeof(bDocHeader)) != 0)
		{
			goto Cleanup;
		}

		ZeroMemory(m_DocHeader, _countof(m_DocHeader));
		if (!GetFileBuffer(m_DocHeader, 0, sizeof(m_DocHeader), sizeof(m_DocHeader)))
		{
			goto Cleanup;
		}

		int iPowIndex = (*(WORD *)&m_DocHeader[0x1E]);
		if ((!iPowIndex) || (iPowIndex > 0xFFFF)
			)
		{
			goto Cleanup;
		}

		m_dwSectorSize = (DWORD)pow((double)2.0, iPowIndex);

		if ((!m_dwSectorSize) || (m_dwSectorSize != 0x200)
			)
		{
			goto Cleanup;
		}

		m_dwDirectoryEntryOffset = (*(DWORD *)&m_DocHeader[0x30]);
		if ((!m_dwDirectoryEntryOffset) || (m_dwDirectoryEntryOffset > m_dwFileSize)
			)
		{
			goto Cleanup;
		}

		bReturn = true;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CDocScanner::IsValidDOCFileSEH, File: %s", csFilePath, 0, true, SECONDLEVEL);
	}
Cleanup:

	//Added by Vilas on 04-10-2016 to avoid closing of handle
	//If we close it, function "EnumerateDirEntriesforExtractionAndScanHash" will fail
	if ((m_hFileHandle != INVALID_HANDLE_VALUE) && 
		(!bReturn) )
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}

	return bReturn;
}
/***************************************************************************************************
*  Function Name  : EnumerateDirEntriesforExtractionAndScanHash
*  Description    : Function to scan each and every DOC file from the directory and generate chunks
*  Author Name    : Pradip
*  SR_NO		  :
*  Date			  : 16 Sept 2016
****************************************************************************************************/
DWORD CDocScanner::EnumerateDirEntriesforExtractionAndScanHash(LPCTSTR pszFilePath)
{
    
	DWORD dwReturn = 0x00;
	DWORD dwBytesRead = 0x00;
	DWORD dwChunkNumber = 0x00;
	try
	{

		STRUCTUREDSTORAGEDIRECTORYENTRY		stDirectoryEntries[0x20] = { 0 };
		DWORD	dwDirEntriesSize = sizeof(stDirectoryEntries);
		
		GetFileBuffer(stDirectoryEntries, (m_dwDirectoryEntryOffset*m_dwSectorSize + m_dwSectorSize), dwDirEntriesSize, 0, &dwBytesRead);
		if (!dwBytesRead)
		{
			goto Cleanup;
		}

		DWORD	dwEntries = dwBytesRead / sizeof(STRUCTUREDSTORAGEDIRECTORYENTRY);
		DWORD	dwIndex = 0x00;
		DWORD	dwOffset = 0x00;
		BYTE	bStoredEntryData[0x5000] = { 0 };
		DWORD	dwReadBytes = 0x00;

		for (; dwIndex < dwEntries; dwIndex++)
		{
			if (stDirectoryEntries[dwIndex].bEleType != 0x2)
				continue;

			dwOffset = dwBytesRead = 0x00;

			if (stDirectoryEntries[dwIndex].dwStartSector)
			{
				dwOffset = stDirectoryEntries[dwIndex].dwStartSector*m_dwSectorSize + m_dwSectorSize;
			}
			else
			{
				//We need to check this 
				dwOffset = m_dwDirectoryEntryOffset*m_dwSectorSize + m_dwSectorSize +
					sizeof(STRUCTUREDSTORAGEDIRECTORYENTRY)*(dwIndex + 2);

				dwOffset = dwOffset - (dwOffset%m_dwSectorSize) + m_dwSectorSize + m_dwSectorSize;
				if (dwOffset == m_dwFileSize)
					dwOffset = m_dwSectorSize;
			}

			dwReadBytes = stDirectoryEntries[dwIndex].dwSizeLow;

			if (!dwReadBytes)
				continue;

			if (stDirectoryEntries[dwIndex].dwSizeLow > 0x5000)
				dwReadBytes = 0x5000;

			BYTE	bSectionsHash[0x10] = { 0 };
			
			dwChunkNumber++;

			GetFileBuffer(bStoredEntryData, dwOffset, dwReadBytes, 0, &dwBytesRead);

			if (!m_pGetByteStringHash)
			{
				m_pGetByteStringHash = NULL;
				m_csDBFilePath = GetWardWizPathFromRegistry();
				m_csDBFilePath += L"VBHASH.DLL";
				m_hInstHashDll = LoadLibrary(m_csDBFilePath);
				m_pGetByteStringHash = (GetByteStringHashFunc)GetProcAddress(m_hInstHashDll, "GetByteStringHash");
			}

			if (!m_pGetByteStringHash)
			{
				dwReturn++;
				break;
			}
			
			DWORD dwRes = m_pGetByteStringHash(bStoredEntryData, dwReadBytes, bSectionsHash);
			if (dwRes == 0)
			{

				GenerateBufferedFile(pszFilePath, bStoredEntryData, dwReadBytes, dwChunkNumber);
				ZeroFill(pszFilePath, dwOffset, dwReadBytes, dwChunkNumber, NULL);
				
			}
		}
	}

	catch (...)
	{
		AddLogEntry(L"### Exception in EnumerateDirEntriesforExtractionAndScan", 0, 0, true, SECONDLEVEL);
		dwReturn = 0xFF;
	}

Cleanup:

	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}

	return dwReturn;
}



/***************************************************************************************************
*  Function Name  : GenerateBufferedFile
*  Description    : Generate Chunks
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  : 28 Sept 2016
****************************************************************************************************/

DWORD CDocScanner::GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber)
{
	CString pszFilePathCopy = pszFilePath;
   	CString FileName, cPathName;
	CString csFileFullPath;
	CString csFileName;

	DWORD dwBytesWritten = 0x00;
	DWORD dwResult = 0x00;
	
	try
	{
		if (pszFilePath == NULL)
		{
			++dwResult;
			goto Cleanup;
		}
		if (pbReadBuffer == NULL)
		{
			++dwResult;
			goto Cleanup;
		}
		if (dwBufferSize == 0x00)
		{
			++dwResult;
			goto Cleanup;
		}

		int length1 = pszFilePathCopy.GetLength();

		FileName = pszFilePathCopy.Mid(pszFilePathCopy.ReverseFind('\\') + 1);
		
		int length2 = FileName.GetLength();
		int a = length1 - length2 - 1;
		
		cPathName = pszFilePathCopy.Left(a);
        
		TCHAR Buffer[10] = { 0 };
		
		csFileFullPath = cPathName;
		csFileFullPath += L"_CHUNK\\";
		FileName += L"_CHUNK_";
		
		swprintf_s(Buffer, sizeof(dwChunkNumber), L"%d", dwChunkNumber);
		FileName += Buffer;
		
		BOOL bDirCreated = FALSE;
		
		bDirCreated = CreateDirectory(csFileFullPath, NULL);

		csFileFullPath += FileName;
		m_hFileHandle = CreateFile(csFileFullPath, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			DWORD e = GetLastError();

			++dwResult;
			goto Cleanup;
		}
		
		WriteFile(m_hFileHandle, pbReadBuffer, dwBufferSize, &dwBytesWritten, 0);
		if (dwBufferSize != dwBytesWritten)
		{
			++dwResult;
			goto Cleanup;
		}


	}
	catch (...)
	{
		AddLogEntry(L"### Exception in DocScanner::GenerateBufferedFile", 0, 0, true, SECONDLEVEL);
		dwResult = 0x05;
	}


Cleanup:

	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}


	return dwResult;


}
/***************************************************************************************************
*  Function Name  : GenerateBufferedFile
*  Description    : Fill with zeros to specified location of chunk in file
*  Author Name    :Sagar
*  SR_NO		  :
*  Date			  : 28 Sept 2016
****************************************************************************************************/

DWORD CDocScanner::ZeroFill(LPCTSTR pszFilePath, DWORD dwAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile)
{
	DWORD dwResult = 0x00;
    LPBYTE pbyFillZeroBuff = NULL;
	
	CString pszFilePathCopy = pszFilePath;
	CString FileName(L"");
	CString	cPathName(L"");
	
	CString csFileFullPath(L"");
	CString csFileName(L"");

	int length1 = pszFilePathCopy.GetLength();

	FileName = pszFilePathCopy.Mid(pszFilePathCopy.ReverseFind('\\') + 1);

	int length2 = FileName.GetLength();
	int a = length1 - length2 - 1;
	
	cPathName = pszFilePathCopy.Left(a);


	TCHAR Buffer[0x10] = { 0 };
	
	csFileFullPath = cPathName;
	csFileFullPath += L"_CHUNK\\";
	
	FileName += L"_FILL_ZERO_";
	swprintf_s(Buffer, sizeof(dwChunkNumber), L"%d", dwChunkNumber);
	FileName += Buffer;
	
	if (!pszString)
		FileName += pszString;

	CreateDirectory(csFileFullPath, NULL);
	if (!PathFileExists(csFileFullPath))
	{
		Sleep(1000);
		CreateDirectory(csFileFullPath, NULL);
	}

	csFileFullPath += FileName;
	DWORD dwBytesRead = 0x00;
	DWORD dwBytesWritten = 0x00;

	try
	{

		if (bCopyFile)
		{
			BOOL bCopy = CopyFile(pszFilePath, csFileFullPath, 0);
			if (!bCopy)
			{
				++dwResult;
				goto Cleanup;
			}

		}

		m_hFileHandle = CreateFile(csFileFullPath, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			++dwResult;
			goto Cleanup;
		}

		DWORD	m_dwFileSize = GetFileSize(m_hFileHandle, NULL);
		if (m_dwFileSize < dwFillZeroCount)
		{
			++dwResult;
			goto Cleanup;
		}

		BYTE *pbyFillZeroBuff = new BYTE[dwFillZeroCount];

		if (!pbyFillZeroBuff)
		{
			++dwResult;
			goto Cleanup;
		}

		memset(pbyFillZeroBuff, 0, dwFillZeroCount);

		DWORD m_dwBytesWritten = 0x00;
		SetFilePointer(m_hFileHandle, dwAddress, 0, FILE_BEGIN);

		WriteFile(m_hFileHandle, pbyFillZeroBuff, dwFillZeroCount, &m_dwBytesWritten, 0);
		if (dwFillZeroCount != m_dwBytesWritten)
		{
			++dwResult;
			goto Cleanup;

		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in DocScanner::ZeroFill", 0, 0, true, SECONDLEVEL);

		dwResult = 0xFF;
	}

Cleanup:

	if (!pbyFillZeroBuff)
		delete pbyFillZeroBuff;

	pbyFillZeroBuff = NULL;

	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}

	return dwResult;

}


