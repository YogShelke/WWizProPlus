/**********************************************************************************************************
Program Name          : PHPScanner.cpp
Description           : Class for PHP file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 29 Apr 2016
Version No            :
Special Logic Used    : This class is created for the scanning of PHP type of files

Modification Log      :
1. Sanjay	          : Created Class CHTMLScanner       29 - 04 - 2016
***********************************************************************************************************/

#include "StdAfx.h"
#include "PHPScanner.h"
#include <math.h>

/***************************************************************************************************
*  Function Name  : CPHPScanner
*  Description    : Cont'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Apr 2016
****************************************************************************************************/
CPHPScanner::CPHPScanner() : m_hFileHandle(INVALID_HANDLE_VALUE)
							, m_bPHPSignatureLoaded(false)
							, m_csVirusName(L"")
							, m_dwPHPBufferOffset(0)
{
	memset(&m_szPHPFilePath, 0, sizeof(m_szPHPFilePath));
}


/***************************************************************************************************
*  Function Name  : ~CPHPScanner
*  Description    : Dest'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Apr 2016
****************************************************************************************************/
CPHPScanner::~CPHPScanner()
{
	if (!m_MD5PHPScanner.UnLoadMD5Data())
	{
		AddLogEntry(L"### Failed to call UnloadMD5Data in CHTMLScanner::~CHTMLScanner", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : LoadPHPSignatures
*  Description    : Function to load signatures for PHP file type
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Apr 2016
****************************************************************************************************/
bool CPHPScanner::LoadPHPSignatures(DWORD &dwSigCount)
{
	bool bReturn = false;

	__try
	{
		if (!m_MD5PHPScanner.LoadMD5Data(FILE_PHP, dwSigCount))
		{
			AddLogEntry(L"### Failed to call LoadMD5Data for PHP type in CHTMLScanner::LoadPHPSignatures", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CHTMLScanner::LoadPHPSignatures", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}

	return bReturn;
}


/*************************************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to scan the PHP file, where the scanning will start only if the PHP signatures are loaded
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Apr 2016
**************************************************************************************************************************/
bool CPHPScanner::ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool bReturn = false;
	DWORD ctBeginTime, ctEndTime;
	char szVirusName[MAX_PATH] = { 0 };


	ctBeginTime = GetTickCount();

	try
	{
		if (!m_bPHPSignatureLoaded)
		{
			DWORD dwSigCount = 0x00;
			m_bPHPSignatureLoaded = LoadPHPSignatures(dwSigCount);
		}

		if (m_bPHPSignatureLoaded)
		{
			bReturn = ScanFileSEH(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_PHP);
			ctEndTime = GetTickCount();
			DWORD ctDiffOfTime = ctEndTime - ctBeginTime;
			CString csDiffOfTime;
			csDiffOfTime.Format(L"%lu milisecs", ctDiffOfTime);
			AddLogEntry(L">>> CPHPScanner::ScanFile took %s for %s", csDiffOfTime, m_szPHPFilePath, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CHTMLScanner::ScanFile Failed, FilePath: %s", pszFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}

	return bReturn;
}


/***************************************************************************************************
*  Function Name  : ScanFileSEH
*  Description    : Function to lead the actual PHP file scanning to a common class
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Apr 2016
****************************************************************************************************/
bool CPHPScanner::ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	try
	{
		_tcscpy_s(m_szPHPFilePath, pszFilePath);

		if (!ReadBufferFromDiffSectionsOfPHP(pszVirusName, dwSpyID, bRescan))
		{
			return false;
		}

		if (!m_szPHPFileBuffer)
		{
			return false;
		}

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### CHTMLScanner::ScanFileSEH Failed, FilePath: %s", pszFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Function to know how to many bytes are actually read from the PHP file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Apr 2016
****************************************************************************************************/
bool CPHPScanner::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
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
*  Function Name  : ScanForMD5AndName
*  Description    : Function to generate virus name if found in an PHP file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Apr 2016
****************************************************************************************************/
bool CPHPScanner::ScanForMD5AndName(LPBYTE m_szHTMLSmallFileBuffer, UINT m_iHTMLBufferOffset, LPTSTR pszVirusName, DWORD & dwSpyID, bool bRescan, FILETYPE filetype)
{

	bool bRet = m_MD5PHPScanner.ScanFile(m_szPHPFileBuffer, m_iHTMLBufferOffset, dwSpyID, bRescan, FILE_PHP);

	if ((dwSpyID > 0) && (bRet))
	{
		m_csVirusName.Format(L"W32.PHP-%lu", dwSpyID);
		_tcscpy_s(pszVirusName, MAX_PATH, m_csVirusName);
		dwSpyID = 0;

		return true;
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSectionsOfPHP
*  Description    : Function to read particular sections of the PHP file for scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Apr 2016
****************************************************************************************************/
bool CPHPScanner::ReadBufferFromDiffSectionsOfPHP(LPTSTR & pszVirusName, DWORD &dwSpyID, bool bRescan)
{
	bool bReturn = false;

	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;
	DWORD dwBytesRead = 0;
	DWORD dwBytesSkipped = 0;
	DWORD dwRemFileSize = 0;

	m_dwPHPBufferOffset = 0x00;
	memset(m_szPHPFileBuffer, 0, sizeof(m_szPHPFileBuffer));

	try
	{
		// if file is < 25 kb
		if (m_dwPHPFileSize <= 0x6400)
		{
			if (GetFileBuffer(m_szPHPFileBuffer, m_dwPHPBufferOffset, m_dwPHPFileSize, m_dwPHPFileSize, &dwBytesRead))
			{
				_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
				bReturn = ScanForMD5AndName(m_szPHPFileBuffer, m_dwPHPFileSize, pszVirusName, dwSpyID, bRescan, FILE_PHP);

				if (bReturn)//Detected
				{
					goto Cleanup;
				}
			}
		}
		/* if file is > 25 kb and < 50 kb,
		0x6400 - 25 kb
		0xC800 - 50 kb*/
		else
			if (m_dwPHPFileSize > 0x6400 && m_dwPHPFileSize <= 0xC800)
			{
				dwBytesSkipped = 5 + 100; //Skip here 105 bytes from start
				dwRemFileSize = m_dwPHPFileSize - dwBytesSkipped;
				DWORD dwFileDivided = dwRemFileSize / 2;

				//while (dwFileDivided <= dwRemFileSize)
				while (dwBytesSkipped <= dwRemFileSize)
				{
					dwBytesRead = m_dwPHPBufferOffset = 0x00;
					ZeroMemory(m_szPHPFileBuffer, _countof(m_szPHPFileBuffer));
					if (GetFileBuffer(m_szPHPFileBuffer, dwBytesSkipped, dwFileDivided, dwFileDivided, &dwBytesRead))
					{
						_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
						dwBytesSkipped += dwBytesRead;
						bReturn = ScanForMD5AndName(m_szPHPFileBuffer, dwFileDivided, pszVirusName, dwSpyID, bRescan, FILE_PHP);
						if (bReturn)//Detected
							goto Cleanup;

					}
				}
			}
		// For files greater than 50 kb, i.e. large files
			else
			{
				dwBytesSkipped = 5 + 100; //Skip here 105 bytes from the beginning
				dwRemFileSize = m_dwPHPFileSize - dwBytesSkipped;
				m_dwPHPBufferOffset = 5 + 100;
				dwBytesRead = 0;

				if (dwRemFileSize > 0x3000)
				{
					dwBytes2Read = 0;
					if (GetFileBuffer(m_szPHPFileBuffer, m_dwPHPFileSize - 0x3000, 0x3000, 0x3000, &dwBytesRead))
					{
						_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
						m_dwPHPBufferOffset += dwBytesRead;

						//Calculate MD5 of 0x3000 bytes from bottom and scan for viruses
						//If found, construct malware name and return from here
						bReturn = ScanForMD5AndName(m_szPHPFileBuffer, dwBytesRead, pszVirusName, dwSpyID, bRescan, FILE_PHP);

						if (bReturn)//Detected
						{
							goto Cleanup;
						}
					}

					//Read Remaining file buffer in 48 KB Chunk
					DWORD dwRemSize = dwRemFileSize - 0x3000;
					DWORD dwBytes2Read = 0xC000;

					while (m_dwPHPBufferOffset < (m_dwPHPFileSize - 0x3000))
					{
						dwBytesRead = 0x00;
						ZeroMemory(m_szPHPFileBuffer, _countof(m_szPHPFileBuffer));
						if (!GetFileBuffer(m_szPHPFileBuffer, m_dwPHPBufferOffset, dwBytes2Read, dwBytes2Read, &dwBytesRead))
						{
							break;
						}

						if (dwBytesRead == 0)
						{
							break;
						}

						//If file buffer size is less than 3 KB then no need to scan.
						if (dwBytesRead > 0xC00)
						{
							// buffer converted to lowercase and scanned for viruses
							_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
							bReturn = ScanForMD5AndName(m_szPHPFileBuffer, dwBytesRead, pszVirusName, dwSpyID, bRescan, FILE_PHP);
							if (bReturn)//Detected
							{
								goto Cleanup;
							}
						}
						m_dwPHPBufferOffset += dwBytesRead;
						DWORD dwBytesRemaining = dwRemSize - m_dwPHPBufferOffset;
						if (dwBytesRemaining < dwBytes2Read)
							dwBytes2Read = dwBytesRemaining;
					}
				}
				else if (GetFileBuffer(m_szPHPFileBuffer, dwBytesSkipped, dwRemFileSize, dwRemFileSize, &dwBytesRead))
				{
					_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
					bReturn = ScanForMD5AndName(m_szPHPFileBuffer, m_dwPHPBufferOffset, pszVirusName, dwSpyID, bRescan, FILE_PHP);
					goto Cleanup;
				}
			}
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
*  Function Name  : IsValidPHPFile
*  Description    : Function to check whether a file is a valid PHP file before scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 28 Apr 2016
****************************************************************************************************/
bool CPHPScanner::IsValidPHPFile(LPCTSTR lpFilePath)
{
	bool bReturn = false;
	__try
	{
		bReturn = IsValidPHPFileSEH(lpFilePath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CHTMLScanner::IsValidPHPFile, FilePath: %s", lpFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : IsValidPHPFileSEH
*  Description    : Function to check whether a file being scanned is a PHP file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 28 Apr 2016
****************************************************************************************************/
bool CPHPScanner::IsValidPHPFileSEH(LPCTSTR lpFilePath)
{
	bool bReturn = false;
	
	// On 32 bit machines PHP scanner crashes, so Temparary PHP Scanner removed.
	return bReturn;

	try
	{
	
		BYTE byHeader[0x5] = { 0 };
		BYTE byPHPHeader[0x5] = { 0x3C, 0x3F, 0x70, 0x68, 0x70 };

		m_hFileHandle = CreateFile(lpFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			goto Cleanup;
		}

		m_dwPHPFileSize = GetFileSize(m_hFileHandle, NULL);

		if (m_dwPHPFileSize == 0x00)
		{
			goto Cleanup;
		}

		DWORD dwBytesRead = 0x00;
		if (!GetFileBuffer(byHeader, 0, sizeof(byHeader), sizeof(byHeader), &dwBytesRead))
		{
			goto Cleanup;
		}

		if (dwBytesRead == 0x00)
		{
			goto Cleanup;
		}
	
		//convert to lower case only if the characters are in range 0 - 127
		bool bIsAllText = false;
		if (_countof(byHeader) > 0)
		{
			bIsAllText = true;
			for (int iCount = 0; iCount < _countof(byHeader); iCount++)
			{
				if (!(byHeader[iCount] >= 0x00 && byHeader[iCount] <= 0x7F))
				{
					bIsAllText = false;
					break;
				}
			}
		}

		if (bIsAllText)
		{
			_strlwr_s((char*)byHeader, strlen((char*)byHeader) + 1);
		}

		if (_memicmp(byHeader, byPHPHeader, sizeof(byPHPHeader)) == 0)
			return true;

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
*  Function Name  : ReadBufferFromDiffSectionsOfPHP
*  Description    : Function to read particular sections of the PHP file for Generating Hash
*  Author Name    : SHodhan
*  SR_NO		  :
*  Date			  : 12 sept 2016
****************************************************************************************************/
DWORD CPHPScanner::ReadBufferFromDiffSectionOfPHPAndHash(LPCTSTR pszFilePath)
{	
	DWORD dwFileOffset = 0x00;
	DWORD dwBytes2Read = 0x00;
	DWORD dwBytesRead = 0x00;
	DWORD dwBytesSkipped = 0x00;
	DWORD dwRemFileSize = 0x00;
	DWORD dwResult = 0x00;
	DWORD dwOffset = 0x00;
	DWORD dwReturn=0x00;
	BYTE bMD5Buffer[0x10] = { 0x00 };
	DWORD dwChunk = 0x00;
	
	m_dwPHPBufferOffset = 0x00;
	
	memset(m_szPHPFileBuffer, 0, sizeof(m_szPHPFileBuffer));
	
	try
	{
		// if file is < 25 kb
		if (m_dwPHPFileSize <= 0x6400)
		{
			if (GetFileBuffer(m_szPHPFileBuffer, m_dwPHPBufferOffset, m_dwPHPFileSize, m_dwPHPFileSize, &dwBytesRead))
			{
				LPCTSTR pszString = NULL;
				++dwChunk;
									 
				_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
				dwReturn =ZeroFill(pszFilePath, m_dwPHPBufferOffset, dwBytesRead, dwChunk,pszString);
				if (dwReturn != 0x00)
				{
					dwReturn++;
					goto Cleanup;
				}
				dwReturn=GenerateBufferedFile(pszFilePath, m_szPHPFileBuffer, dwBytesRead, dwChunk);
				if (dwReturn != 0x00)
				{
					dwReturn++;
					goto Cleanup;
				}

			}
		}
		/* if file is > 25 kb and < 50 kb,
		0x6400 - 25 kb
		0xC800 - 50 kb*/
		else
			if (m_dwPHPFileSize > 0x6400 && m_dwPHPFileSize <= 0xC800)
			{
				dwBytesSkipped = 5 + 100; //Skip here 105 bytes from start
				dwRemFileSize = m_dwPHPFileSize - dwBytesSkipped;
				DWORD dwFileDivided = dwRemFileSize / 2;
				dwOffset = 0x00;
				//while (dwFileDivided <= dwRemFileSize)
				while (dwBytesSkipped <= dwRemFileSize)
				{
					dwBytesRead = m_dwPHPBufferOffset = 0x00;
					ZeroMemory(m_szPHPFileBuffer, _countof(m_szPHPFileBuffer));
					if (GetFileBuffer(m_szPHPFileBuffer, dwBytesSkipped, dwFileDivided, dwFileDivided, &dwBytesRead))
					{
						LPCTSTR pszString = NULL;
						++dwChunk;

						_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
						dwBytesSkipped += dwBytesRead;
						dwReturn =ZeroFill(pszFilePath, m_dwPHPBufferOffset, dwBytesRead, dwChunk,pszString);
						if (dwReturn != 0x00)
						{
							dwReturn++;
							goto Cleanup;
						}
						dwReturn=GenerateBufferedFile(pszFilePath, m_szPHPFileBuffer, dwBytesRead, dwChunk);
						if (dwReturn != 0x00)
						{
							dwReturn++;
							goto Cleanup;
						}
					}
				}
			}
		// For files greater than 50 kb, i.e. large files
			else
			{
				dwBytesSkipped = 5 + 100; //Skip here 105 bytes from the beginning
				dwRemFileSize = m_dwPHPFileSize - dwBytesSkipped;
				m_dwPHPBufferOffset = 5 + 100;
				dwBytesRead = 0;
				dwOffset = 0x00;
				if (dwRemFileSize > 0x3000)
				{
					dwBytes2Read = 0;
					if (GetFileBuffer(m_szPHPFileBuffer, m_dwPHPFileSize - 0x3000, 0x3000, 0x3000, &dwBytesRead))
					{
						LPCTSTR pszString = NULL;
						++dwChunk;
						
						_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
						m_dwPHPBufferOffset += dwBytesRead;

						//Calculate MD5 of 0x3000 bytes from bottom and scan for viruses
						//If found, construct malware name and return from here
						
						dwReturn =ZeroFill(pszFilePath, m_dwPHPBufferOffset, dwBytesRead, dwChunk,pszString);
						if (dwReturn != 0x00)
						{
							dwReturn++;
							goto Cleanup;
						}
						dwReturn=GenerateBufferedFile(pszFilePath, m_szPHPFileBuffer, dwBytesRead, dwChunk);
						if (dwReturn != 0x00)
						{
							dwReturn++;
							goto Cleanup;
						}
					}

					//Read Remaining file buffer in 48 KB Chunk
					DWORD dwRemSize = dwRemFileSize - 0x3000;
					DWORD dwBytes2Read = 0xC000;
					dwOffset = 0x00;
					while (m_dwPHPBufferOffset < (m_dwPHPFileSize - 0x3000))
					{
						dwBytesRead = 0x00;
						//ZeroMemory(m_szPHPFileBuffer, _countof(m_szPHPFileBuffer));
						if (!GetFileBuffer(m_szPHPFileBuffer, m_dwPHPBufferOffset, dwBytes2Read, dwBytes2Read, &dwBytesRead))
						{
							break;
						}

						if (dwBytesRead == 0)
						{
							break;
						}

						//If file buffer size is less than 3 KB then no need to scan.
						if (dwBytesRead > 0xC00)
						{
							LPCTSTR pszString = NULL;
							++dwChunk;
							
							// buffer converted to lowercase and scanned for viruses
							_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
							dwReturn =ZeroFill(pszFilePath, m_dwPHPBufferOffset, dwBytesRead, dwChunk,pszString);
							if (dwReturn != 0x00)
							{
								dwReturn++;
								goto Cleanup;
							}
							dwReturn=GenerateBufferedFile(pszFilePath, m_szPHPFileBuffer, dwBytesRead, dwChunk);
							if (dwReturn != 0x00)
							{
								dwReturn++;
								goto Cleanup;
							}
						}
						m_dwPHPBufferOffset += dwBytesRead;
						DWORD dwBytesRemaining = dwRemSize - m_dwPHPBufferOffset;
						if (dwBytesRemaining < dwBytes2Read)
							dwBytes2Read = dwBytesRemaining;
					}
				}
				else if (GetFileBuffer(m_szPHPFileBuffer, dwBytesSkipped, dwRemFileSize, dwRemFileSize, &dwBytesRead))
				{
					LPCTSTR pszString = NULL;
					++dwChunk;
					dwOffset = 0x00;
					
					_strlwr_s((char *)m_szPHPFileBuffer, strlen((char*)m_szPHPFileBuffer) + 1);
					dwReturn =ZeroFill(pszFilePath, m_dwPHPBufferOffset, dwBytesRead, dwChunk,pszString);
					if (dwReturn != 0x00)
					{
						dwReturn++;
						goto Cleanup;
					}
					dwReturn=GenerateBufferedFile(pszFilePath, m_szPHPFileBuffer, dwBytesRead, dwChunk);
					if (dwReturn != 0x00)
					{
						dwReturn++;
						goto Cleanup;
					}
				}
			}
	}
	catch (...)
	{
		dwResult = 0x0FF;
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
*  Description    : It generate's chunks
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  : 22 sep 2016
****************************************************************************************************/

DWORD CPHPScanner::GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber)
{
	CString pszFilePathCopy = pszFilePath;
	DWORD dwRet=0x00;
	CString FileName=NULL, cPathName=NULL;
	CString csFileFullPath=NULL;
	CString csFileName=NULL;
	DWORD dwBytesWritten = 0x00;
	DWORD dwResult = 0x00;
	HANDLE hFile;

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
		hFile = CreateFile(csFileFullPath, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			++dwResult;
			goto Cleanup;
		}
		WriteFile(hFile, pbReadBuffer, dwBufferSize, &dwBytesWritten, 0);
		if (dwBufferSize != dwBytesWritten)
		{
			++dwResult;
			goto Cleanup;
		}
	}
	catch (...)
	{
		dwResult = 0x05;
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
*  Description    : To replace chunk data with zero
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  : 22 sep 2016
****************************************************************************************************/

DWORD CPHPScanner::ZeroFill(LPCTSTR pszFilePath, DWORD dwAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString)
{
	DWORD dwResult = 0x00;
	HANDLE m_hFileHandle;
	
	LPBYTE pbyFillZeroBuff = NULL;
	

	CString pszFilePathCopy = pszFilePath;
	CString FileName=NULL, cPathName=NULL;
	DWORD BUFSIZE = 500;
	TCHAR szHash[0x40] = { 0x00 };
	CString csFileFullPath=NULL;
	CString csFileName=NULL;
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
	BOOL bDirCreated = FALSE;
	bDirCreated = CreateDirectory(csFileFullPath, NULL);
	DWORD e = GetLastError();

	csFileFullPath += FileName;
	DWORD dwBytesRead = 0;
	DWORD dwBytesWritten = 0;

	try
	{
		BOOL copy = CopyFile(pszFilePath, csFileFullPath, 0);

		m_hFileHandle = CreateFile(csFileFullPath, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			++dwResult;
			goto Cleanup;
		}

		DWORD m_dwFileSize = GetFileSize(m_hFileHandle, NULL);

		if (m_dwFileSize < dwAddress )
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
