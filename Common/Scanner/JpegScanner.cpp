#include "StdAfx.h"
#include "JpegScanner.h"


/***************************************************************************************************
*  Function Name  : CJPegScanner
*  Description    : Cont'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
****************************************************************************************************/
CJPegScanner::CJPegScanner() : m_hFileHandle(INVALID_HANDLE_VALUE)
							  ,m_bJPEGSignatureLoaded(false)
{
	memset(&m_szJPEGFilePath, 0, sizeof(m_szJPEGFilePath));
}


/***************************************************************************************************
*  Function Name  : ~CJPegScanner
*  Description    : Dest'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
****************************************************************************************************/
CJPegScanner::~CJPegScanner()
{
	if (!m_MD5JPEGScanner.UnLoadMD5Data())
	{
		AddLogEntry(L"### Failed to call UnloadMD5Data in CJPegScanner::~CJPegScanner", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : LoadJPEGSignatures
*  Description    : Function to load signatures for JPEG file type
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
****************************************************************************************************/
bool CJPegScanner::LoadJPEGSignatures(DWORD &dwSigCount)
{
	bool bReturn = false;

	__try
	{
		if (!m_MD5JPEGScanner.LoadMD5Data(FILE_JPG, dwSigCount))
		{
			AddLogEntry(L"### Failed to call LoadMD5Data for JPEG type in CJPegScanner::LoadJPEGSignatures", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		bReturn = true;		
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CJPegScanner::LoadJPEGSignatures", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	
	return bReturn;
}


/*************************************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to scan the JPEG file, where the scanning will start only if the JPEG signatures are loaded
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
					Updated with minor changes
*  Modified Date  : 19 Mar 2016
**************************************************************************************************************************/
bool CJPegScanner::ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool bReturn = false;
	DWORD ctBeginTime, ctEndTime;
	char szVirusName[MAX_PATH] = { 0 };
	

	ctBeginTime = GetTickCount();

	try
	{
		if (!m_bJPEGSignatureLoaded)
		{
			DWORD dwSigCount = 0x00;
			m_bJPEGSignatureLoaded = LoadJPEGSignatures(dwSigCount);
		}

		if (m_bJPEGSignatureLoaded)
		{
			bReturn = ScanFileSEH(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_JPG);
			ctEndTime = GetTickCount();
			DWORD ctDiffOfTime = ctEndTime - ctBeginTime;
			CString csDiffOfTime;
			csDiffOfTime.Format(L"%lu milisecs", ctDiffOfTime);
			AddLogEntry(L">>> CJpegScanner::ScanFile took %s for %s", csDiffOfTime, m_szJPEGFilePath, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CJPegScanner::ScanFile Failed, FilePath: %s", pszFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ScanFileSEH
*  Description    : Function to lead the actual scanning to a common class for scanning and generating virus name if found
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
****************************************************************************************************/
bool CJPegScanner::ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	CString csVirusName = L"";
	try
	{
		_tcscpy(m_szJPEGFilePath, pszFilePath);

		if (!ReadBufferFromDiffSectionsOfJPEG())
		{
			return false;
		}
		if (!m_szJPEGFileBuffer)
		{
			return false;
		}

		//add here to scan buffer
		bool bRet = m_MD5JPEGScanner.ScanFile(m_szJPEGFileBuffer, m_iJPEGBufferOffset, dwSpyID, bRescan, FILE_JPG);
		
		if ((dwSpyID > 0) && (bRet))
		{
			csVirusName.Format(L"W32.Jpeg-%lu", dwSpyID);
			_tcscpy_s(pszVirusName, MAX_PATH, csVirusName);
			dwSpyID = 0;
			return true;
		}
		
	}
	catch (...)
	{
		AddLogEntry(L"### CJPegScanner::ScanFileSEH Failed, File: %s", pszFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Function to know how to many bytes are read from the JPEG file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
****************************************************************************************************/
bool CJPegScanner::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
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
		AddLogEntry(L"### Exception in CPEScanner::GetFileBuffer", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSectionsOfJPEG
*  Description    : Function to read particular sections of the JPEG file for scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
****************************************************************************************************/
bool CJPegScanner::ReadBufferFromDiffSectionsOfJPEG()
{
	bool bReturn = false;

	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;
	DWORD dwBytesRead = 0;

	m_iJPEGBufferOffset = 0x00;
	memset(m_szJPEGFileBuffer, 0, sizeof(m_szJPEGFileBuffer));

	try
	{
		//Read 0x200 bytes file start
		if (GetFileBuffer(&m_szJPEGFileBuffer[m_iJPEGBufferOffset], dwFileOffset + 0x40, 0x200, 0, &dwBytesRead))
		{
			m_iJPEGBufferOffset += dwBytesRead;
		}

		if (m_dwFileSize < 0x240)
		{
			goto Cleanup;
		}

		//Read 0xE00 bytes from file end
		if (m_dwFileSize > 0x1042)
		{
			dwFileOffset = m_dwFileSize - 0xE00;
		}
		else
		{
			dwFileOffset = 0x40 + 0x200;
		}

		if (GetFileBuffer(&m_szJPEGFileBuffer[m_iJPEGBufferOffset], dwFileOffset, 0xE00, 0, &dwBytesRead))
		{
			m_iJPEGBufferOffset += dwBytesRead;
		}
		bReturn = true;
	}
	catch (...)
	{
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
*  Function Name  : IsValidJPEGFile
*  Description    : Function to check whether a file being scanned is a JPEG file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
					Updated with minor changes
*  Modified Date  : 19 Mar 2016
****************************************************************************************************/
bool CJPegScanner::IsValidJPEGFile(LPCTSTR csFilePath)
{
	bool bReturn = false;
	__try
	{

		m_hFileHandle = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			//return bReturn;
			goto Cleanup;
		}

		m_dwFileSize = GetFileSize(m_hFileHandle, NULL);

		if ((m_dwFileSize == 0xFFFFFFFF) || (m_dwFileSize == 0x00))
		{
			goto Cleanup;
		}

		BYTE bJPEGHeader[] = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46 };
		BYTE bHeader[0x0A] = { 0 };


		DWORD dwBytesRead = 0;

		if (!GetFileBuffer(bHeader, 0, sizeof(bHeader), sizeof(bHeader)))
		{
			//return bReturn;
			goto Cleanup;
		}

		if (_memicmp(bHeader, bJPEGHeader, sizeof(bJPEGHeader)) != 0)
		{
			goto Cleanup;
		}

		//Do not close the handle as it is required to read the buffer
		return true;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CJPegScanner::IsValidJPEGFile, File: %s", csFilePath, 0, true, SECONDLEVEL);
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
*  Function Name  : ReadBufferFromDiffSectionsOfJPEGAndHash
*  Description    : Function to read particular sections of the JPEG file for scanning and calculating hashes
*  Author Name    : Sukanya
*  SR_NO		  :
*  Date			  : 16 Sept 2016
****************************************************************************************************/
DWORD CJPegScanner::ReadBufferFromDiffSectionsOfJPEGAddScanHash(LPCTSTR pszFilePath)
{
	DWORD dwReturn = 00;

	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;
	DWORD dwBytesRead = 0;
	
	BYTE bStoredEntryData[0x1000] = {0};
	BYTE bSectionsHash[0x10] = { 0 };
	DWORD dwReadBytes=0x00;
	DWORD dwBytestoHash = 0;
	DWORD dwOffset = 0x00;
	
	m_iJPEGBufferOffset = 0x00;
	memset(m_szJPEGFileBuffer, 0, sizeof(m_szJPEGFileBuffer));

	try
	{
		
		//Read 0x200 bytes file start
		if (GetFileBuffer(&m_szJPEGFileBuffer[m_iJPEGBufferOffset], dwFileOffset + 0x40, 0x200, 0, &dwBytesRead))
		{
			m_iJPEGBufferOffset += dwBytesRead;
			dwBytestoHash += dwBytesRead;
		}

		if (m_dwFileSize < 0x240)
		{
			goto Cleanup;
		}
		
		DWORD dwChunkNumber = 1;
		LPCTSTR pszString = NULL;
		
		dwReturn=ZeroFill(pszFilePath, dwFileOffset + 0x40, dwBytesRead, dwChunkNumber, pszString, true);

		
		//Read 0xE00 bytes from file end
		if (m_dwFileSize > 0x1042)
		{
			dwFileOffset = m_dwFileSize - 0xE00;
		}
		else
		{
			dwFileOffset = 0x40 + 0x200;
		}

		if(GetFileBuffer(&m_szJPEGFileBuffer[m_iJPEGBufferOffset], dwFileOffset, 0xE00, 0, &dwBytesRead))
		{

			ZeroFill(pszFilePath, dwFileOffset, dwBytesRead, dwChunkNumber, pszString, false);

			m_iJPEGBufferOffset += dwBytesRead;
			dwBytestoHash += dwBytesRead;
			GenerateBufferedFile(pszFilePath, m_szJPEGFileBuffer, 0x1000, dwChunkNumber);
							
		}
		
	}
	catch (...)
	{
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
*  Description    : Functio to fill buffer with Zero
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  : 22 sep 2016
****************************************************************************************************/
DWORD CJPegScanner::GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber)
{
	CString pszFilePathCopy = pszFilePath;


	CString FileName(L"");
	CString cPathName(L"");

	CString csFileFullPath(L"");

	CString csFileName(L"");
	DWORD dwBytesWritten = 0x00;
	DWORD dwResult = 0x00;
	HANDLE hFile = INVALID_HANDLE_VALUE;



	try
	{
		if (pszFilePath == NULL)
		{
			dwResult = 0x01;
			goto Cleanup;
		}
		
		if (pbReadBuffer == NULL)
		{
			dwResult = 0x02;
			goto Cleanup;
		}
		
		if (dwBufferSize == 0x00)
		{
			dwResult = 0x03;
			goto Cleanup;
		}

		int Lenght3 = 0;
		int length1 = pszFilePathCopy.GetLength();
		FileName = pszFilePathCopy.Mid(pszFilePathCopy.ReverseFind('\\') + 1);
		int length2 = FileName.GetLength();
		if (length1>length2)
			Lenght3 = length1 - length2 - 1;
		cPathName = pszFilePathCopy.Left(Lenght3);


		TCHAR Buffer[0x10] = { 0 };

		csFileFullPath = cPathName;
		
		csFileFullPath += L"_CHUNK\\";
		FileName += L"_CHUNK_";
		
		swprintf_s(Buffer, sizeof(dwChunkNumber), L"%d", dwChunkNumber);
		FileName += Buffer;
		
		if (!PathFileExists(csFileFullPath))
		{
			CreateDirectory(csFileFullPath, NULL);
		}
		
		csFileFullPath += FileName;
		
		hFile = CreateFile(csFileFullPath, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			dwResult = 0x03;
			goto Cleanup;
		}
		
		WriteFile(hFile, pbReadBuffer, dwBufferSize, &dwBytesWritten, 0);
		if (dwBufferSize != dwBytesWritten)
		{
			dwResult = 0x04;
			goto Cleanup;
		}

	}
	catch (...)
	{
		dwResult = 0xFF;
	}


Cleanup:

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return dwResult;

}


/***************************************************************************************************
*  Function Name  : ZeroFill
*  Description    : Functio to fill buffer with Zero
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  : 22 sep 2016
****************************************************************************************************/
DWORD CJPegScanner::ZeroFill(LPCTSTR pszFilePath, DWORD dwStartAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile)
{
	DWORD dwResult = 0x00;
	HANDLE m_hFileHandle = INVALID_HANDLE_VALUE;
	LPBYTE pbyFillZeroBuff = NULL;
	CString pszFilePathCopy = pszFilePath;

	CString FileName(L"");
	CString cPathName(L"");


	CString csFileFullPath(L"");
	CString csFileName(L"");


	int Length3 = 0;
	int length1 = pszFilePathCopy.GetLength();
	FileName = pszFilePathCopy.Mid(pszFilePathCopy.ReverseFind('\\') + 1);
	int length2 = FileName.GetLength();
	if (length1>length2)
		Length3 = length1 - length2 - 1;
	cPathName = pszFilePathCopy.Left(Length3);


	TCHAR Buffer[10] = { 0 };
	csFileFullPath = cPathName;
	csFileFullPath += L"_CHUNK\\";
	//	FileName += L"_";
	FileName += L"_FILL_ZERO_";
	swprintf_s(Buffer, sizeof(dwChunkNumber), L"%d", dwChunkNumber);
	FileName += Buffer;
	if (!pszString)
		FileName += pszString;
	if (!PathFileExists(csFileFullPath))
	{
		CreateDirectory(csFileFullPath, NULL);
	}
	csFileFullPath += FileName;
	DWORD dwBytesRead = 0;
	DWORD dwBytesWritten = 0;

	try
	{

		if (bCopyFile)
		{
			BOOL bCopy = CopyFile(pszFilePath, csFileFullPath, 0);
			if (!bCopy)
			{
				dwResult = 0x01;
				goto Cleanup;
			}

		}

		m_hFileHandle = CreateFile(csFileFullPath, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			dwResult = 0x02;
			goto Cleanup;
		}

		DWORD m_dwFileSize = GetFileSize(m_hFileHandle, NULL);
		if (m_dwFileSize < dwFillZeroCount)
		{
			dwResult = 0x03;
			goto Cleanup;
		}


		BYTE *pbyFillZeroBuff = new BYTE[dwFillZeroCount];

		if (!pbyFillZeroBuff)
		{
			dwResult = 0x03;

			goto Cleanup;
		}

		memset(pbyFillZeroBuff, 0, dwFillZeroCount);

		DWORD m_dwBytesWritten = 0x00;
		SetFilePointer(m_hFileHandle, dwStartAddress, 0, FILE_BEGIN);

		WriteFile(m_hFileHandle, pbyFillZeroBuff, dwFillZeroCount, &m_dwBytesWritten, 0);
		if (dwFillZeroCount != m_dwBytesWritten)
		{
			dwResult = 0x04;
			goto Cleanup;
		}

	}
	catch (...)
	{
		dwResult = 0x05;
	}

Cleanup:

	if (!pbyFillZeroBuff)
		delete [] pbyFillZeroBuff;

	pbyFillZeroBuff = NULL;

	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}

	return dwResult;

}

