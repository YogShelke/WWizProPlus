/**********************************************************************************************************
Program Name          : HTMLScanner.cpp
Description           : Class for HTML file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 31 Mar 2016
Version No            :
Special Logic Used    : This class is created for the scanning of HTML type of files

Modification Log      :
1. Sanjay	          : Created Class CHTMLScanner       31 - 03 - 2016
***********************************************************************************************************/

#include "StdAfx.h"
#include "HTMLScanner.h"
#include <math.h>


/***************************************************************************************************
*  Function Name  : CHTMLScanner
*  Description    : Cont'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 1 Apr 2016
****************************************************************************************************/
CHTMLScanner::CHTMLScanner() : m_hFileHandle(INVALID_HANDLE_VALUE)
								,m_bHTMLSignatureLoaded(false)
								,m_csVirusName(L"")
								,m_iHTMLBufferOffset(0)
{
	memset(&m_szHTMLFilePath, 0, sizeof(m_szHTMLFilePath));	
}


/***************************************************************************************************
*  Function Name  : ~CHTMLScanner
*  Description    : Dest'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 1 Apr 2016
****************************************************************************************************/
CHTMLScanner::~CHTMLScanner()
{
	if (!m_MD5HTMLScanner.UnLoadMD5Data())
	{
		AddLogEntry(L"### Failed to call UnloadMD5Data in CHTMLScanner::~CHTMLScanner", 0, 0, true, SECONDLEVEL);
	}
	
}


/***************************************************************************************************
*  Function Name  : LoadHTMLSignatures
*  Description    : Function to load signatures for HTML file type
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 1 Apr 2016
****************************************************************************************************/
bool CHTMLScanner::LoadHTMLSignatures(DWORD &dwSigCount)
{
	bool bReturn = false;

	__try
	{
		if (!m_MD5HTMLScanner.LoadMD5Data(FILE_HTML, dwSigCount))
		{
			AddLogEntry(L"### Failed to call LoadMD5Data for HTML type in CHTMLScanner::LoadHTMLSignatures", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CHTMLScanner::LoadHTMLSignatures", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}

	return bReturn;
}



/*************************************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to scan the PHP file, where the scanning will start only if the PHP signatures are loaded
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 1 Apr 2016
**************************************************************************************************************************/
bool CHTMLScanner::ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool bReturn = false;
	DWORD ctBeginTime, ctEndTime;
	char szVirusName[MAX_PATH] = { 0 };


	ctBeginTime = GetTickCount();

	try
	{
		if (!m_bHTMLSignatureLoaded)
		{
			DWORD dwSigCount = 0x00;
			m_bHTMLSignatureLoaded = LoadHTMLSignatures(dwSigCount);
		}

		if (m_bHTMLSignatureLoaded)
		{
			bReturn = ScanFileSEH(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_PHP);
			ctEndTime = GetTickCount();
			DWORD ctDiffOfTime = ctEndTime - ctBeginTime;
			CString csDiffOfTime;
			csDiffOfTime.Format(L"%lu milisecs", ctDiffOfTime);
			AddLogEntry(L">>> CHTMLScanner::ScanFile took %s for %s", csDiffOfTime, m_szHTMLFilePath, true, ZEROLEVEL);
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
*  Description    : Function to lead the actual HMTL file scanning to a common class
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 1 Apr 2016
****************************************************************************************************/
bool CHTMLScanner::ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	
	try
	{
		_tcscpy_s(m_szHTMLFilePath, pszFilePath);

		if (!ReadBufferFromDiffSectionsOfHTML(pszVirusName, dwSpyID, bRescan))
		{
			return false;
		}

		if (!m_szHTMLFileBuffer)
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
*  Description    : Function to know how to many bytes are actually read from the HTML file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Mar 2016
****************************************************************************************************/
bool CHTMLScanner::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
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
*  Description    : Function to generate virus name if found in an HTML file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
****************************************************************************************************/
bool CHTMLScanner::ScanForMD5AndName(LPBYTE m_szHTMLFileBuffer, UINT m_iHTMLBufferOffset, LPTSTR pszVirusName, DWORD & dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool bRet = m_MD5HTMLScanner.ScanFile(m_szHTMLFileBuffer, m_iHTMLBufferOffset, dwSpyID, bRescan, FILE_HTML);

	if ((dwSpyID > 0) && (bRet))
	{
		m_csVirusName.Format(L"W32.Html-%lu", dwSpyID);
		_tcscpy_s(pszVirusName, MAX_PATH, m_csVirusName);
		dwSpyID = 0;

		return true;
	}

	return false;
}

/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSectionsOfHTML
*  Description    : Function to read particular sections of the HTML file for scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Mar 2016
****************************************************************************************************/
bool CHTMLScanner::ReadBufferFromDiffSectionsOfHTML(LPTSTR & pszVirusName, DWORD &dwSpyID, bool bRescan)
{
	bool bReturn = false;
	
	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;
	DWORD dwBytesRead = 0;
	DWORD dwBytesSkipped = 0;
	DWORD dwRemFileSize = 0;

	m_iHTMLBufferOffset = 0x00;
	memset(m_szHTMLFileBuffer, 0, sizeof(m_szHTMLFileBuffer));

	try
	{
		// if file is < 25 kb
		if (m_dwHTMLFileSize < 0x6400)
		{
			if (GetFileBuffer(m_szHTMLFileBuffer, m_iHTMLBufferOffset, m_dwHTMLFileSize, m_dwHTMLFileSize, &dwBytesRead))
			{
				_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);
				bReturn = ScanForMD5AndName(m_szHTMLFileBuffer, m_dwHTMLFileSize, pszVirusName, dwSpyID, bRescan, FILE_HTML);
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
		if (m_dwHTMLFileSize > 0x6400 && m_dwHTMLFileSize < 0xC800)
		{
			dwBytesSkipped = 16 + 200; //Skip here 216 bytes from start
			dwRemFileSize = m_dwHTMLFileSize - dwBytesSkipped;
			DWORD dwFileDivided = dwRemFileSize / 2;
			
			//while (dwFileDivided <= dwRemFileSize)
			while (dwBytesSkipped <= dwRemFileSize)
			{
				dwBytesRead = m_iHTMLBufferOffset = 0x00;
				ZeroMemory(m_szHTMLFileBuffer, _countof(m_szHTMLFileBuffer));
				if (GetFileBuffer(m_szHTMLFileBuffer, dwBytesSkipped, dwFileDivided, dwFileDivided, &dwBytesRead))
				{
					_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);
					dwBytesSkipped += dwBytesRead;
					bReturn = ScanForMD5AndName(m_szHTMLFileBuffer, dwFileDivided, pszVirusName, dwSpyID, bRescan, FILE_HTML);
					if (bReturn)//Detected
						goto Cleanup;
				
				}
			}			
		}
		else
		{
			dwBytesSkipped = 16 + 200; //Skip here 216 bytes from start
			dwRemFileSize = m_dwHTMLFileSize - dwBytesSkipped;
			m_iHTMLBufferOffset = 16 + 200;
			dwBytesRead = 0;
			
			if (dwRemFileSize > 0x5000)
			{
				dwBytes2Read = 0;
				if (GetFileBuffer(m_szHTMLFileBuffer, m_dwHTMLFileSize - 0x5000, 0x5000, 0x5000, &dwBytesRead))
				{
					_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);
					m_iHTMLBufferOffset += dwBytesRead;

					//Calculate MD5 and check in database
					//If found, construct malware name and return from here
					bReturn = ScanForMD5AndName(m_szHTMLFileBuffer, dwBytesRead, pszVirusName, dwSpyID, bRescan, FILE_HTML);
					if (bReturn)//Detected
					{
						goto Cleanup;
					}
				}

				//Read Remaining file buffer in 48 KB Chunk
				DWORD dwRemSize = dwRemFileSize - 0x5000;
				DWORD dwBytes2Read = 0xC000;
				while (m_iHTMLBufferOffset < dwRemSize)
				{
					dwBytesRead = 0x00;
					ZeroMemory(m_szHTMLFileBuffer, _countof(m_szHTMLFileBuffer));
					if (!GetFileBuffer(m_szHTMLFileBuffer, m_iHTMLBufferOffset, dwBytes2Read, dwBytes2Read, &dwBytesRead))
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
						//Scan buffer
						_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);
						bReturn = ScanForMD5AndName(m_szHTMLFileBuffer, dwBytesRead, pszVirusName, dwSpyID, bRescan, FILE_HTML);
						if (bReturn)//Detected
						{
							goto Cleanup;
						}
					}
					m_iHTMLBufferOffset += dwBytesRead;
				}
			}
			else if (GetFileBuffer(m_szHTMLFileBuffer, dwBytesSkipped, dwRemFileSize, dwRemFileSize, &dwBytesRead))
			{
				_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);
				bReturn = ScanForMD5AndName(m_szHTMLFileBuffer, m_iHTMLBufferOffset, pszVirusName, dwSpyID, bRescan, FILE_HTML);
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
*  Function Name  : IsValidHTMLFile
*  Description    : Function to check whether a file being scanned is a HTML file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Mar 2016
****************************************************************************************************/
bool CHTMLScanner::IsValidHTMLFile(LPCTSTR lpFilePath)
{										
	bool bReturn = false;
	__try
	{
		bReturn = IsValidHTMLFileSEH(lpFilePath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CHTMLScanner::IsValidHTMLFile, File: %s", lpFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : IsValidHTMLFileSEH
*  Description    : Function to check whether a file being scanned is a HTML file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 29 Mar 2016
****************************************************************************************************/
bool CHTMLScanner::IsValidHTMLFileSEH(LPCTSTR lpFilePath)
{
	bool bReturn = false;
	try
	{
		/*HTML files can have the header given in different ways, that need to be detected from the HTML file
		for the scanner to ascertain that the file, in consideration, indeed is a valid HTML file */

		BYTE bHTMLHeader1[0x16] = {	0x3c, 0x21, 0x64, 0x6f, 0x63, 0x74, 0x79, 0x70, 0x65, 0x20, 
									0x68, 0x74, 0x6d, 0x6c, 0x20, 0x70, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20};
		BYTE bHTMLHeader2[0xF] = { 0x3C, 0x21, 0x64, 0x6f, 0x63, 0x74, 0x79, 0x70, 
									0x65, 0x20, 0x68, 0x74, 0x6d, 0x6c, 0x3E };
		BYTE bHTMLHeader3[0xE] = { 0x3C, 0x64, 0x6f, 0x63, 0x74, 0x79, 0x70,
									0x65, 0x20, 0x68, 0x74, 0x6d, 0x6c, 0x3E };
		BYTE bHTMLHeader4[0x6] = { 0x3C, 0x68, 0x74, 0x6d, 0x6c, 0x3E };

		BYTE bHeader[0x16] = { 0 };

		BYTE bPHPHeader[0x5] = { 0x3C, 0x3F, 0x70, 0x68, 0x70 };

		m_hFileHandle = CreateFile(lpFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			goto Cleanup;
		}

		m_dwHTMLFileSize = GetFileSize(m_hFileHandle, NULL);

		if (/*(m_dwFileSize == 0xFFFFFFFF) ||*/ (m_dwHTMLFileSize == 0x00))
		{
			goto Cleanup;
		}
		
		DWORD dwBytesRead = 0x00;
		if (!GetFileBuffer(bHeader, 0, sizeof(bHeader), sizeof(bHeader), &dwBytesRead))
		{
			goto Cleanup;
		}

		if (dwBytesRead == 0x00)
		{
			goto Cleanup;
		}

		//convert to lower case only if the characters are in range 0 - 127
		bool bISAllText = false;
		if (_countof(bHeader) > 0)
		{
			bISAllText = true;
			for (int iCount = 0; iCount < _countof(bHeader); iCount++)
			{
				if (!(bHeader[iCount] >= 0x00 && bHeader[iCount] <= 0x7F))
				{
					bISAllText = false;
					break;
				}
			}
		}

		if (bISAllText)
		{
			_strlwr_s((char*)bHeader, strlen((char*)bHeader) + 1);
		}

		if (_memicmp(bHeader, bHTMLHeader1, sizeof(bHTMLHeader1)) == 0)// || ((CString)bHeader).Find(L"<!DOCTYPE HTML PUBLIC ") > 0)
			return true;
		if (_memicmp(bHeader, bHTMLHeader2, sizeof(bHTMLHeader2)) == 0)// || ((CString)bHeader).Find(L"<!DOCTYPE html") > 0)
			return true;
		if (_memicmp(bHeader, bHTMLHeader3, sizeof(bHTMLHeader3)) == 0)//<DOCTYPE html>
			return true;
		if (_memicmp(bHeader, bHTMLHeader4, sizeof(bHTMLHeader4)) == 0)//<html>
			return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CHTMLScanner::IsValidHTMLFileSEH, File: %s", lpFilePath, 0, true, SECONDLEVEL);
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
*  Function Name  : ReadBufferFromDiffSectionsOfHTMLAndHash
*  Description    : Function to Add Generatated Hash in vector
*  Author Name    : Sagar,Shodhan
*  SR_NO		  :
*  Date			  : 12 Sep 2016
****************************************************************************************************/

DWORD CHTMLScanner::ReadBufferFromDiffSectionOfHTMLAndHash(LPCTSTR pszFilePath)
{


	DWORD dwFileOffset = 0x00;
	DWORD dwBytes2Read = 0x00;
	DWORD dwBytesRead = 0x00;
	DWORD dwBytesSkipped = 0x00;
	DWORD dwRemFileSize = 0x00;
	DWORD	dwReadBytes = 0x00;
	DWORD dwResult = 0x00;
	DWORD iBuffOffset = 0x00;
	BYTE bMD5Buffer[0x10] = { 0x00 };
	DWORD dwChunk = 0x00;
	LPCTSTR pszString = NULL;

	try
	{


		// if file is < 25 kb
		if (m_dwHTMLFileSize < 0x6400)
		{
			m_iHTMLBufferOffset = 0x00;
			memset(m_szHTMLFileBuffer, 0x00, sizeof(m_szHTMLFileBuffer));

			if (GetFileBuffer(m_szHTMLFileBuffer, m_iHTMLBufferOffset, m_dwHTMLFileSize, m_dwHTMLFileSize, &dwBytesRead))
			{
				++dwChunk;
				_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);
				dwResult = ZeroFill(pszFilePath, m_iHTMLBufferOffset, dwBytesRead, dwChunk, pszString);
				if (dwResult != 0x00)
				{
					dwResult++;
					goto Cleanup;
				}
				dwResult = GenerateBufferedFile(pszFilePath, m_szHTMLFileBuffer, dwBytesRead, dwChunk);
				if (dwResult != 0x00)
				{
					dwResult++;
					goto Cleanup;
				}



			}
		}
		/* if file is > 25 kb and < 50 kb,
		0x6400 - 25 kb
		0xC800 - 50 kb*/
		else
			if (m_dwHTMLFileSize > 0x6400 && m_dwHTMLFileSize < 0xC800)
			{
				dwBytesSkipped = 16 + 200; //Skip here 216 bytes from start
				dwRemFileSize = m_dwHTMLFileSize - dwBytesSkipped;
				DWORD dwFileDivided = dwRemFileSize / 2;

				//while (dwFileDivided <= dwRemFileSize)
				while (dwBytesSkipped <= dwRemFileSize)
				{
					dwBytesRead = m_iHTMLBufferOffset = 0x00;
					ZeroMemory(m_szHTMLFileBuffer, _countof(m_szHTMLFileBuffer));
					if (GetFileBuffer(m_szHTMLFileBuffer, dwBytesSkipped, dwFileDivided, dwFileDivided, &dwBytesRead))
					{
						
							iBuffOffset = 0x00;
							dwBytesSkipped += dwBytesRead;
							++dwChunk;
							_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);
							dwResult = ZeroFill(pszFilePath, m_iHTMLBufferOffset, dwBytesRead, dwChunk, pszString);
							if (dwResult != 0x00)
							{
								dwResult++;
								goto Cleanup;
							}
							dwResult = GenerateBufferedFile(pszFilePath, m_szHTMLFileBuffer, dwBytesRead, dwChunk);
							if (dwResult != 0x00)
							{
								dwResult++;
								goto Cleanup;
							}

					}
				}
			}
			else
			{
				dwBytesSkipped = 16 + 200; //Skip here 216 bytes from start
				dwRemFileSize = m_dwHTMLFileSize - dwBytesSkipped;
				m_iHTMLBufferOffset = 16 + 200;
				dwBytesRead = 0;

				if (dwRemFileSize > 0x5000)
				{
					ZeroMemory(m_szHTMLFileBuffer, _countof(m_szHTMLFileBuffer));
					dwBytes2Read = 0;
					if (GetFileBuffer(m_szHTMLFileBuffer, m_dwHTMLFileSize - 0x5000, 0x5000, 0x5000, &dwBytesRead))
					{
							iBuffOffset = 0x00;
							m_iHTMLBufferOffset += dwBytesRead;

							++dwChunk;
							_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);

							dwResult = ZeroFill(pszFilePath, m_iHTMLBufferOffset, dwBytesRead, dwChunk, pszString);
							if (dwResult != 0x00)
							{
								dwResult++;
								goto Cleanup;
							}


							dwResult = GenerateBufferedFile(pszFilePath, m_szHTMLFileBuffer, dwBytesRead, dwChunk);
							if (dwResult != 0x00)
							{
								dwResult++;
								goto Cleanup;
							}

					}

					//Read Remaining file buffer in 48 KB Chunk
					DWORD dwRemSize = dwRemFileSize - 0x5000;
					DWORD dwBytes2Read = 0xC000;
					while (m_iHTMLBufferOffset < dwRemSize)
					{
						dwBytesRead = 0x00;
						ZeroMemory(m_szHTMLFileBuffer, _countof(m_szHTMLFileBuffer));
						if (!GetFileBuffer(m_szHTMLFileBuffer, m_iHTMLBufferOffset, dwBytes2Read, dwBytes2Read, &dwBytesRead))
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
							//Scan buffer
							iBuffOffset = 0x00;
							++dwChunk;
							_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);
							dwResult = ZeroFill(pszFilePath, m_iHTMLBufferOffset, dwBytesRead, dwChunk, pszString);
							if (dwResult != 0x00)
							{
								dwResult++;
								goto Cleanup;
							}


							dwResult = GenerateBufferedFile(pszFilePath, m_szHTMLFileBuffer, dwBytesRead, dwChunk);
							if (dwResult != 0x00)
							{
								dwResult++;
								goto Cleanup;
							}

						}
						m_iHTMLBufferOffset += dwBytesRead;
					}
				}
				else if (GetFileBuffer(m_szHTMLFileBuffer, dwBytesSkipped, dwRemFileSize, dwRemFileSize, &dwBytesRead))
				{

					iBuffOffset = 0x00;
					++dwChunk;
					_strlwr_s((char *)m_szHTMLFileBuffer, strlen((char*)m_szHTMLFileBuffer) + 1);

					dwResult = ZeroFill(pszFilePath, m_iHTMLBufferOffset, dwBytesRead, dwChunk, pszString);
					if (dwResult != 0x00)
					{
						dwResult++;
						goto Cleanup;
					}


					dwResult = GenerateBufferedFile(pszFilePath, m_szHTMLFileBuffer, dwBytesRead, dwChunk);
					if (dwResult != 0x00)
					{
						dwResult++;
						goto Cleanup;
					}


				}
			}
	}
	catch (...)
	{
		dwResult = 0xFF;
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
*  Description    : Functio to generate's chunks
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  : 22 sep 2016
****************************************************************************************************/

DWORD CHTMLScanner::GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber)
{
	CString FileName(L"");
	CString cPathName(L"");

	CString csFileFullPath(L"");

	CString csFileName(L"");
	DWORD dwBytesWritten = 0x00;
	DWORD dwResult = 0x00;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	CString pszFilePathCopy = pszFilePath;


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
*  Date			  : 24 sep 2016
****************************************************************************************************/
DWORD CHTMLScanner::ZeroFill(LPCTSTR pszFilePath, DWORD dwStartAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile)
{
	DWORD dwResult = 0x00;
	HANDLE hFileHandle = INVALID_HANDLE_VALUE;
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
				Sleep(1000);
				bCopy = CopyFile(pszFilePath, csFileFullPath, 0);
				Sleep(0);
				if (!bCopy)
				{
					dwResult = 0x01;
					goto Cleanup;
				}
			}

		}

		hFileHandle = CreateFile(csFileFullPath, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileHandle == INVALID_HANDLE_VALUE)
		{
			dwResult = 0x02;
			goto Cleanup;
		}

		DWORD m_dwFileSize = GetFileSize(hFileHandle, NULL);
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

		DWORD dwBytesWritten = 0x00;
		SetFilePointer(hFileHandle, dwStartAddress, 0, FILE_BEGIN);

		WriteFile(hFileHandle, pbyFillZeroBuff, dwFillZeroCount, &dwBytesWritten, 0);
		if (dwFillZeroCount != dwBytesWritten)
		{
			dwResult = 0x04;
			goto Cleanup;
		}

	}
	catch (...)
	{
		dwResult = 0x0FF;
	}

Cleanup:

	if (!pbyFillZeroBuff)
		delete pbyFillZeroBuff;

	pbyFillZeroBuff = NULL;

	if (hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFileHandle);
		hFileHandle = INVALID_HANDLE_VALUE;
	}

	return dwResult;

}