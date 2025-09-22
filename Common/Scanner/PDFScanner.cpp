/**********************************************************************************************************
Program Name          : PDFScanner.cpp
Description           : Class for PDF file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 25 Mar 2016
Version No            :
Special Logic Used    :

Modification Log      :
1. Sanjay	          : Created Class CPDFScanner       25-03-2016, 15-04-2016
***********************************************************************************************************/

#include "StdAfx.h"
#include "PDFScanner.h"


/***************************************************************************************************
*  Function Name  : CPDFScanner
*  Description    : Cont'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 15 Apr 2016
****************************************************************************************************/
CPDFScanner::CPDFScanner() : m_hPDFFileHandle(INVALID_HANDLE_VALUE)
							, m_bPDFSignatureLoaded(false)
							, m_csVirusName(L"")
							, m_dwPDFBufferOffset(0)
							, m_dwXrefAddress(0)
							, m_dwFileSize(0)
{
	memset(&m_szPDFFilePath, 0, sizeof(m_szPDFFilePath));
}


/***************************************************************************************************
*  Function Name  : ~CPDFScanner
*  Description    : Dest'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 15 Apr 2016
****************************************************************************************************/
CPDFScanner::~CPDFScanner()
{
	if (!m_MD5PDFScanner.UnLoadMD5Data())
	{
		AddLogEntry(L"### Failed to call UnloadMD5Data in CPDFScanner::~CPDFScanner", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : LoadPDFSignatures
*  Description    : Function to load signatures for PDF file type
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 15 Apr 2016
****************************************************************************************************/
bool CPDFScanner::LoadPDFSignatures(DWORD &dwSigCount)
{
	bool bReturn = false;

	__try
	{
		if (!m_MD5PDFScanner.LoadMD5Data(FILE_PDF, dwSigCount))
		{
			AddLogEntry(L"### Failed to call LoadMD5Data for PDF type in CPDFScanner::LoadPDFSignatures", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CPDFScanner::LoadPDFSignatures", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}

	return bReturn;
}


/*************************************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to scan the PDF file, where the scanning will start only if the PDF signatures are loaded
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 15 Apr 2016
**************************************************************************************************************************/
bool CPDFScanner::ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool bReturn = false;
	DWORD ctBeginTime, ctEndTime;

	ctBeginTime = GetTickCount();

	try
	{
		if (!m_bPDFSignatureLoaded)
		{
			DWORD dwSigCount = 0x00;
			m_bPDFSignatureLoaded = LoadPDFSignatures(dwSigCount);
		}

		if (m_bPDFSignatureLoaded)
		{
			bReturn = ScanFileSEH(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_PDF);
			ctEndTime = GetTickCount();
			DWORD ctDiffOfTime = ctEndTime - ctBeginTime;
			CString csDiffOfTime;
			csDiffOfTime.Format(L"%lu milisecs", ctDiffOfTime);
			AddLogEntry(L">>> CPDFScanner::ScanFile took %s for %s", csDiffOfTime, m_szPDFFilePath, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CPDFScanner::ScanFile Failed, FilePath: %s", pszFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}

	return bReturn;
}


/***************************************************************************************************
*  Function Name  : ScanFileSEH
*  Description    : Function to lead the actual scanning to a common class for scanning 
					and generating virus name if found
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 15 Apr 2016
****************************************************************************************************/
bool CPDFScanner::ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	__try
	{
		// Only for Debug purpose
		AddLogEntry(L">>> Inside CPDFScanner::ScanFileSEH", 0, 0, true, ZEROLEVEL);
		_tcscpy_s(m_szPDFFilePath, pszFilePath);

		if (!ReadBufferFromDiffSectionsOfPDF(pszVirusName, dwSpyID, bRescan))
		{
			return false;
		}
		
		return true;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ScanFileSEH, file: %s", pszFilePath, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}


/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Function to know how to many bytes are read from the PDF file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 15 Apr 2016
****************************************************************************************************/
bool CPDFScanner::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
{
	__try
	{
		if (m_hPDFFileHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		DWORD dwBytesRead = 0x00;
		DWORD dwSetFileOffSet = ::SetFilePointer(m_hPDFFileHandle, dwReadOffset, NULL, FILE_BEGIN);
		if (dwSetFileOffSet == dwReadOffset)
		{
			if (ReadFile(m_hPDFFileHandle, pbReadBuffer, dwBytesToRead, &dwBytesRead, NULL))
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
		AddLogEntry(L"### Exception in CPDFScanner::GetFileBuffer", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}


/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSectionsOfPDF
*  Description    : Function to read particular sections of the PDF file for scanning
					Ram: Checked with every GetFileBuffer a return value and Number of bytes read.
					if bytesRead = 0, break.
					if GetFileBuffer failed, break.
*  Author Name    : Sanjay, Ram
*  SR_NO		  :
*  Date			  : 15 Apr 2016
****************************************************************************************************/
bool CPDFScanner::ReadBufferFromDiffSectionsOfPDF(LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan)
{
	bool bReturn = false;

	__try
	{
		AddLogEntry(L">>> Inside CPDFScanner::ReadBufferFromDiffSectionsOfPDF", 0, 0, true, ZEROLEVEL);

		DWORD dwFileOffset = 0;
		DWORD dwBytes2Read = 0;
		DWORD dwBytesRead = 0;
		DWORD dwBytesSkipped = 0;
		DWORD dwBytesNewOffset = 0;
		bool EndStreamFound = false;

		//Initialize buffer offset to BEGIN (0)
		m_dwPDFBufferOffset = 0x00;

		// Skipping first 10 bytes of header
		dwBytesSkipped = m_dwPDFBufferOffset = 10;

		//If no Xref table found, consider that file is INVALID PDF and skip.
		if (m_dwXrefAddress == 0x00)
			goto Cleanup;

		// Calculating remaining file size from 10 bytes till the address of XRef table or object
		DWORD dwRemFileSize = 0;
		dwRemFileSize = m_dwXrefAddress - 10;

		while (m_dwPDFBufferOffset < dwRemFileSize)
		{
			BYTE byStream[0x6] = { 0 };
			BYTE byPDFStream[] = { 0x73, 0x74, 0x72, 0x65, 0x61, 0x6D };
			BYTE byPDFFileBuffer[0xC000] = { 0 };

			if (m_dwPDFBufferOffset > 0xBFFF)
				goto Cleanup;

			dwBytesRead = 0x00;
			if (!GetFileBuffer(byStream, m_dwPDFBufferOffset, 0x6, 0x6, &dwBytesRead))
			{
				break;
			}

			//if bytes read 0, break;
			if (dwBytesRead == 0x00)
				break;

			if (dwBytesRead > 0)
			{
				if (_memicmp(byStream, byPDFStream, sizeof(byPDFStream)) == 0) // if stream matches
				{
					m_dwPDFBufferOffset += dwBytesRead; // i.e. 6, if stream matches

					BYTE byLetter = 0;
					//BYTE byEndstream[0x10] = { 0 };

					dwBytesRead = 0x00;
					if (!GetFileBuffer(&byLetter, m_dwPDFBufferOffset, 0x1, 0x1, &dwBytesRead))
					{
						break;
					}

					if (dwBytesRead == 0x00)
						break;

					if (dwBytesRead > 0)
					{
						DWORD iIndex = 0;
						while (EndStreamFound == false)
						{
							BYTE byEndstream[0x10] = { 0 };
							BYTE byPDFEndstream[] = { 0x65, 0x6E, 0x64, 0x73, 0x74, 0x72, 0x65, 0x61, 0x6D };

							dwBytesRead = 0x00;
							if (!GetFileBuffer(byEndstream, m_dwPDFBufferOffset, 0x9, 0x9, &dwBytesRead))
							{
								break;
							}

							if (dwBytesRead == 0x00)
								break;

							if (dwBytesRead > 0)
							{
								if (_memicmp(byEndstream, byPDFEndstream, sizeof(byPDFEndstream)) == 0) // if endstream matches
								{
									EndStreamFound = true;
									m_dwPDFBufferOffset += dwBytesRead; // i.e. 9, if Endstream matches
									break;
								}
								else
								{
									//Need to check here buffer size limit, if exceed return
									if (iIndex > 0xBFFF || iIndex > dwRemFileSize)
										goto Cleanup;

									byPDFFileBuffer[iIndex++] = byLetter;
									m_dwPDFBufferOffset++;

									if (m_dwPDFBufferOffset > dwRemFileSize)
										goto Cleanup;
								}
								bool ret = GetFileBuffer(&byLetter, m_dwPDFBufferOffset, 0x1, 0x1, &dwBytesRead);
								
								//Code Commented by Nihar Deshpande on 28-09-2016
								//Due to unsatisfying conditions, the hash was not generated previously
								//Working now
								/*if (dwBytesRead == 0x00 || ret)
									break;
								*/
							}
						}
						if (EndStreamFound == true)
						{
							bool bRet = m_MD5PDFScanner.ScanFile(byPDFFileBuffer, iIndex, dwSpyID, bRescan, FILE_PDF);

							EndStreamFound = false;
							if ((dwSpyID > 0) && (bRet))
							{
								m_csVirusName.Format(L"W32.PDF-%lu", dwSpyID);
								_tcscpy_s(pszVirusName, MAX_PATH, m_csVirusName);
								dwSpyID = 0;
								bReturn = true;
								goto Cleanup;
							}
						}
					}
					else
					{
						m_dwPDFBufferOffset++;
						continue;
					}
				}
				else
				{
					m_dwPDFBufferOffset++;
					if (m_dwPDFBufferOffset > dwRemFileSize)
						goto Cleanup;
					continue;
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CPDFScanner::ReadBufferFromDiffSectionsOfPDF", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}

Cleanup:
	if (m_hPDFFileHandle != INVALID_HANDLE_VALUE)
	{
		if(CloseHandle(m_hPDFFileHandle))
			AddLogEntry(L">>> File: %s closed", m_szPDFFilePath, 0, true, ZEROLEVEL);
		m_hPDFFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}


/******************************************************************************************
*  Function Name  : IsValidPDFFile
*  Description    : Function to check whether a file being scanned is a PDF file 
					by checking its header, the presence of words, startxref and EOF
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 12 Apr 2016
*******************************************************************************************/
bool CPDFScanner::IsValidPDFFile(CString csFilePath)
{
	bool bReturn = false;
	try
	{

		if (!PathFileExists(csFilePath))
		{
			return bReturn;
		}

		BYTE byPDFHeader[]		= { 0x25, 0x50, 0x44, 0x46, 0x2D };		
		BYTE byPDFStartxref[]	= { 0x73, 0x74, 0x61, 0x72, 0x74, 0x78, 0x72, 0x65, 0x66 };
		BYTE bySta[]			= { 0x73, 0x74, 0x61 };
		BYTE byPDFEOF[]			= { 0x45, 0x4F, 0x46 };
		BYTE byHeader[0x5]		= { 0 };
		BYTE byStartxref[0x9]	= { 0 };
		BYTE byEOF[0x3]			= { 0 };		
		BYTE byNumber[0x5]		= { 0 };
		BYTE byNum[0x10]		= { 0 };
		BYTE byStart[0x3]		= { 0 };
		BYTE byLastChar;
		DWORD dwBytes2Read		= 15;
		DWORD dwRemFileSize		= 0;
		DWORD dwBytesRead		= 0;
		DWORD dwRefTableAddress = 0;
		DWORD dwStrToNumber		= 0;
		int NosRead				= 0;
		
		m_hPDFFileHandle = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (m_hPDFFileHandle == INVALID_HANDLE_VALUE)
		{
			goto Cleanup;
		}

		m_dwFileSize = GetFileSize(m_hPDFFileHandle, NULL);

		if (m_dwFileSize == 0x00)
		{
			goto Cleanup;
		}

		if (!GetFileBuffer(byHeader, 0, sizeof(byHeader), sizeof(byHeader)))
		{
			goto Cleanup;
		}

		if (_memicmp(byHeader, byPDFHeader, sizeof(byPDFHeader)) != 0)
		{
			goto Cleanup;
		}


		/********************************************************************************************************************/
		dwRemFileSize = m_dwFileSize - 40;

		if (GetFileBuffer(byStartxref, dwRemFileSize, 0x9, 0x9, &dwBytesRead))
		{
			if (dwBytesRead > 0)
			{
				bool bFlag = false;
				while (bFlag == false)
				{
					dwBytesRead = 0x00;
					if (_memicmp(byStartxref, byPDFStartxref, sizeof(byPDFStartxref)) == 0)
					{
						bFlag = true;
						dwRemFileSize += 0x9;
						break;
					}
					dwRemFileSize++;
					bool bRet = GetFileBuffer(byStartxref, dwRemFileSize, 0x9, 0x9, &dwBytesRead);
					if (dwBytesRead == 0x00)
					{
						break;
					}
				}
				if (bFlag == false)
					goto Cleanup;

				bFlag = false;
				if (GetFileBuffer(&byLastChar, dwRemFileSize, 1, 1, &dwBytesRead))
				{
					while (byLastChar == 0x0D || byLastChar == 0x0A)
					{
						dwBytesRead = 0x00;
						dwRemFileSize++;
						bool Ret = GetFileBuffer(&byLastChar, dwRemFileSize, 1, 1, &dwBytesRead);
						if (dwBytesRead == 0x00)
						{
							break;
						}
					}

					dwRemFileSize++;
					for (int i = 0 ; byLastChar >= 0x30 && byLastChar <= 0x39; i++, dwRemFileSize++)
					{
						dwBytesRead = 0x00;
						byNum[i] = byLastChar;
						bool Ret = GetFileBuffer(&byLastChar, dwRemFileSize, 1, 1, &dwBytesRead);
						if (dwBytesRead == 0x00)
						{
							break;
						}
						NosRead++;
					}
					byNum[NosRead+1] = '\0';
					dwStrToNumber = atoi((const char *)byNum);
					if (dwStrToNumber < m_dwFileSize)
					{
						m_dwXrefAddress = dwStrToNumber;

						while (dwRemFileSize < m_dwFileSize)
						{
							dwBytesRead = 0x00;
							if (GetFileBuffer(&byLastChar, dwRemFileSize, 1, 1, &dwBytesRead))
							{
								if (dwBytesRead == 0x00)
									break;

								dwBytesRead = 0x00;
								if (byLastChar == 0x45)
									if (GetFileBuffer(&byEOF, dwRemFileSize, 3, 3, &dwBytesRead))
										if (_memicmp(byEOF, byPDFEOF, sizeof(byPDFEOF)) == 0)
										{
											bFlag = true;
											goto Success;
										}
							}
							dwRemFileSize++;
						}
						if (bFlag == false)
						{
							bReturn = false;
							goto Cleanup;
						}
					}
					else
					{
						bReturn = false;
						goto Cleanup;
					}
				}
			}
		}

					
		//Do not close the handle as it is required to read the buffer
	Success:
		return true;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CPDFScanner::IsValidPDFFile, FilePath: %s", csFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}

Cleanup:

	if (m_hPDFFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hPDFFileHandle);
		m_hPDFFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSectionsOfPDFAddHash
*  Description    : Function to read particular sections of the PDF file for scanning
	Ram: Checked with every GetFileBuffer a return value and Number of bytes read.
	if bytesRead = 0, break.
	if GetFileBuffer failed, break.
*  Author Name    :Nihar
*  SR_NO		  :
*  Date			  : 16 09 2016
****************************************************************************************************/
DWORD CPDFScanner::ReadBufferFromDiffSectionsOfPDFAddHash(LPCTSTR pszFilePath)
{
	DWORD bReturn = 0x00;
	DWORD dwOffset = 0;
	DWORD dwChunkNumber = 0x01;
	DWORD dwAddress = 0x00;
	
	bBufferHash[0x10] = { 0 };
	
	try
	{
		
		m_hInstLibraryForHashDLL = LoadLibrary(L"VBHASH.DLL");
		
		if (!m_hInstLibraryForHashDLL)
		{
			AddLogEntry(L"### Failed to load library : VBHASH.DLL", 0, 0, true, SECONDLEVEL);

			bReturn++;
			goto Cleanup;
		}

		m_pGetBytesStringHash = (GetBytesStringHashFunc)GetProcAddress(m_hInstLibraryForHashDLL, "GetByteStringHash");
		if (!m_pGetBytesStringHash)
		{
			AddLogEntry(L"### Failed to load function : GetByteStringHash", 0, 0, true, SECONDLEVEL);
			
			bReturn++;
			goto Cleanup;
		}


		//AddLogEntry(L">>> Inside CPDFScanner::ReadBufferFromDiffSectionsOfPDF", 0, 0, true, ZEROLEVEL);

		DWORD dwFileOffset = 0;
		DWORD dwBytes2Read = 0;
		DWORD dwBytesRead = 0;
		DWORD dwBytesSkipped = 0;
		DWORD dwBytesNewOffset = 0;
		bool EndStreamFound = false;

		//Initialize buffer offset to BEGIN (0)
		m_dwPDFBufferOffset = 0x00;

		// Skipping first 10 bytes of header
		dwBytesSkipped = m_dwPDFBufferOffset = 10;

		//If no Xref table found, consider that file is INVALID PDF and skip.
		if (m_dwXrefAddress == 0x00)
			return bReturn;

		// Calculating remaining file size from 10 bytes till the address of XRef table or object
		DWORD dwRemFileSize = 0;
		dwRemFileSize = m_dwXrefAddress - 10;

		while (m_dwPDFBufferOffset < dwRemFileSize)
		{
			BYTE byStream[0x6] = { 0 };
			BYTE byPDFStream[] = { 0x73, 0x74, 0x72, 0x65, 0x61, 0x6D };
			BYTE byPDFFileBuffer[0xC000] = { 0 };

			if (m_dwPDFBufferOffset > 0xBFFF)
				goto Cleanup;

			dwBytesRead = 0x00;
			if (!GetFileBuffer(byStream, m_dwPDFBufferOffset, 0x6, 0x6, &dwBytesRead))
			{
				break;
			}

			//if bytes read 0, break;
			if (dwBytesRead == 0x00)
				break;

			if (dwBytesRead > 0)
			{
				if (_memicmp(byStream, byPDFStream, sizeof(byPDFStream)) == 0) // if stream matches
				{
					m_dwPDFBufferOffset += dwBytesRead; // i.e. 6, if stream matches
					dwAddress = m_dwPDFBufferOffset;
					BYTE byLetter = 0;
					//BYTE byEndstream[0x10] = { 0 };

					dwBytesRead = 0x00;
					if (!GetFileBuffer(&byLetter, m_dwPDFBufferOffset, 0x1, 0x1, &dwBytesRead))
					{
						break;
					}

					if (dwBytesRead == 0x00)
						break;

					if (dwBytesRead > 0)
					{
						DWORD iIndex = 0;
						while (EndStreamFound == false)
						{
							BYTE byEndstream[0x10] = { 0 };
							BYTE byPDFEndstream[] = { 0x65, 0x6E, 0x64, 0x73, 0x74, 0x72, 0x65, 0x61, 0x6D };

							dwBytesRead = 0x00;
							if (!GetFileBuffer(byEndstream, m_dwPDFBufferOffset, 0x9, 0x9, &dwBytesRead))
							{
								break;
							}

							if (dwBytesRead == 0x00)
								break;

							if (dwBytesRead > 0)
							{
								if (_memicmp(byEndstream, byPDFEndstream, sizeof(byPDFEndstream)) == 0) // if endstream matches
								{
									EndStreamFound = true;
									m_dwPDFBufferOffset += dwBytesRead; // i.e. 9, if Endstream matches
									break;
								}
								else
								{
									//Need to check here buffer size limit, if exceed return
									if (iIndex > 0xBFFF || iIndex > dwRemFileSize)
										goto Cleanup;

									byPDFFileBuffer[iIndex++] = byLetter;
									dwBufferSize = iIndex;
									m_dwPDFBufferOffset++;

									if (m_dwPDFBufferOffset > dwRemFileSize)
										goto Cleanup;
								}
								bool ret = GetFileBuffer(&byLetter, m_dwPDFBufferOffset, 0x1, 0x1, &dwBytesRead);
								
								
								//Code Commented by Nihar Deshpande
								//Due to unsatisfying conditions, the hash was not generated previously
								//Working now
								/*if (dwBytesRead == 0x00 || ret)
									break;*/
							}
						}
						if (EndStreamFound == true)
						{
							EndStreamFound = false;
							DWORD dwBytesWritten = 0x00;
							
							memset(bBufferHash, 0, sizeof(bBufferHash));
							
							
							GenerateBufferedFile( pszFilePath, byPDFFileBuffer, dwBufferSize, dwChunkNumber);
							
							ZeroFill(pszFilePath, dwAddress, sizeof(byPDFFileBuffer), dwChunkNumber, NULL);

							dwChunkNumber++;
							
						}
					}
					else
					{
						m_dwPDFBufferOffset++;
						continue; 
					}
				}
				else
				{
					m_dwPDFBufferOffset++;
					if (m_dwPDFBufferOffset > dwRemFileSize)
						goto Cleanup;
					continue;
				}
			}
		}
	}
	
	catch (...)
	{
		AddLogEntry(L"### Exception in CPDFScanner::ReadBufferFromDiffSectionsOfPDF", 0, 0, true, SECONDLEVEL);
		bReturn++;
	}

Cleanup:
	
	if (m_hPDFFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hPDFFileHandle);
		m_hPDFFileHandle = INVALID_HANDLE_VALUE;
	}
	
	return bReturn;
}




/******************************************************************************************
*  Function Name  : GenerateBufferedFile
*  Description    : Function to write chunks of streams in a separate file
by checking its header, the presence of words, startxref and EOF
*  Author Name    : Nihar
*  SR_NO		  :
*  Date			  : 28 Sept 2016
*******************************************************************************************/
DWORD CPDFScanner::GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber)
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


		TCHAR Buffer[10] = { 0 };


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
		//


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


/******************************************************************************************
*  Function Name  : ZeroFill
*  Description    : Function to write FillZeros of streams in a separate file
by checking its header, the presence of words, startxref and EOF
*  Author Name    : Nihar
*  SR_NO		  :
*  Date			  : 28 Sept 2016
*******************************************************************************************/


DWORD CPDFScanner::ZeroFill(LPCTSTR pszFilePath, DWORD dwStartAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile)
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
		delete pbyFillZeroBuff;

	pbyFillZeroBuff = NULL;

	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}

	return dwResult;

}
