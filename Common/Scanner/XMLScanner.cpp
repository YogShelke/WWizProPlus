/**********************************************************************************************************
Program Name          : XMLScanner.cpp
Description           : Class for XML file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 5 Apr 2016
Version No            :
Special Logic Used    : This class is created for the scanning of XML type of files

Modification Log      :
1. Sanjay	          : Created Class CXMLScanner       05 - 04 - 2016
***********************************************************************************************************/

#include "StdAfx.h"
#include "XMLScanner.h"

/***************************************************************************************************
*  Function Name  : CXMLScanner
*  Description    : Cont'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
****************************************************************************************************/
CXMLScanner::CXMLScanner() : m_hFileHandle(INVALID_HANDLE_VALUE)
							, m_bXMLSignatureLoaded(false)
							, m_csVirusName(L"")
{
}


/***************************************************************************************************
*  Function Name  : CXMLScanner
*  Description    : Dest'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
****************************************************************************************************/
CXMLScanner::~CXMLScanner()
{
	if (!m_MD5XMLScanner.UnLoadMD5Data())
	{
		AddLogEntry(L"### Failed to call UnloadMD5Data in CXMLScanner::~CXMLScanner", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************************************
*  Function Name  : LoadXMLSignatures
*  Description    : Function to load signatures for XML file type
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
****************************************************************************************************/
bool CXMLScanner::LoadXMLSignatures(DWORD &dwSigCount)
{
	bool bReturn = false;

	__try
	{
		if (!m_MD5XMLScanner.LoadMD5Data(FILE_XML, dwSigCount))
		{
			AddLogEntry(L"### Failed to call LoadMD5Data for XML type in CXMLScanner::LoadXMLSignatures", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CXMLScanner::LoadXMLSignatures", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}

	return bReturn;
}


/*************************************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to scan the XML file, where the scanning will start only if the XML signatures are loaded
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
**************************************************************************************************************************/
bool CXMLScanner::ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool bReturn = false;
	DWORD ctBeginTime, ctEndTime;
	char szVirusName[MAX_PATH] = { 0 };


	ctBeginTime = GetTickCount();

	try
	{
		if (!m_bXMLSignatureLoaded)
		{
			DWORD dwSigCount = 0x00;
			m_bXMLSignatureLoaded = LoadXMLSignatures(dwSigCount);
		}

		if (m_bXMLSignatureLoaded)
		{
			bReturn = ScanFileSEH(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_XML);
			ctEndTime = GetTickCount();
			DWORD ctDiffOfTime = ctEndTime - ctBeginTime;
			CString csDiffOfTime;
			csDiffOfTime.Format(L"%lu milisecs", ctDiffOfTime);
			AddLogEntry(L">>> CXMLScanner::ScanFile took %s for %s", csDiffOfTime, m_szXMLFilePath, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CXMLScanner::ScanFile Failed for file [%s]", pszFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ScanFileSEH
*  Description    : Function to lead the actual XML file to a common class for scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
****************************************************************************************************/
bool CXMLScanner::ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, FILETYPE filetype)
{
	try
	{
		_tcscpy(m_szXMLFilePath, pszFilePath);

		if (!ReadBufferFromDiffSectionsOfXML(pszVirusName, dwSpyID, bRescan))
		{
			return false;
		}
		if (!m_szXMLFileBuffer)
		{
			return false;
		}

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### CXMLScanner::ScanFileSEH Failed for file %s", pszFilePath, 0, true, SECONDLEVEL);
	}

	return false;
}


/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Function to know how to many bytes are read from the XML file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
****************************************************************************************************/
bool CXMLScanner::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
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
*  Description    : Function to generate virus name if found in an XML file
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
****************************************************************************************************/
bool CXMLScanner::ScanForMD5AndName(LPBYTE m_szXMLFileBuffer, UINT m_iXMLBufferOffset, LPTSTR pszVirusName, DWORD & dwSpyID, bool bRescan, FILETYPE filetype)
{
	bool bRet = m_MD5XMLScanner.ScanFile(m_szXMLFileBuffer, m_iXMLBufferOffset, dwSpyID, bRescan, FILE_XML);

	if ((dwSpyID > 0) && (bRet))
	{
		m_csVirusName.Format(L"W32.Xml-%lu", dwSpyID);
		_tcscpy_s(pszVirusName, MAX_PATH, m_csVirusName);
		dwSpyID = 0;

		return true;
	}
	return false;
}


/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSectionsOfXML
*  Description    : Function to read particular sections of the XML file for scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
****************************************************************************************************/
bool CXMLScanner::ReadBufferFromDiffSectionsOfXML(LPTSTR & pszVirusName, DWORD &dwSpyID, bool bRescan)
{
	bool bReturn = false;

	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;
	DWORD dwBytesRead = 0;
	DWORD dwBytesSkipped = 0;
	DWORD dwRemFileSize = 0;

	m_iXMLBufferOffset = 0x00;
	memset(m_szXMLFileBuffer, 0, sizeof(m_szXMLFileBuffer));
	
	try
	{
		
		if (m_dwFileSize < 0x6400)					// if file is < 25 kb
		{
			if (GetFileBuffer(m_szXMLFileBuffer, m_iXMLBufferOffset, m_dwFileSize, m_dwFileSize, &dwBytesRead))
			{
				bReturn = ScanForMD5AndName(m_szXMLFileBuffer, m_dwFileSize, pszVirusName, dwSpyID, bRescan, FILE_XML);
				if (bReturn)
					goto Cleanup;
			}
		}
		else if (m_dwFileSize > 0x6400 && m_dwFileSize < 0xC800)// if file is > 25 kb and < 50 kb  i.e. 0xC800 - 50 kb
		{
			dwBytesSkipped = 0xF + 200; //Skip first 215 bytes
			dwRemFileSize = m_dwFileSize - dwBytesSkipped;
			DWORD dwFileDivided = dwRemFileSize / 2;

			while (dwBytesSkipped <= m_dwFileSize)
			{
				dwBytesRead = m_iXMLBufferOffset = 0x00;
				ZeroMemory(m_szXMLFileBuffer, _countof(m_szXMLFileBuffer));
				if (GetFileBuffer(m_szXMLFileBuffer, dwBytesSkipped, dwFileDivided, dwFileDivided, &dwBytesRead))
				{
					if (dwBytesRead == 0)
						break;

					dwBytesSkipped += dwBytesRead;
					bReturn = ScanForMD5AndName(m_szXMLFileBuffer, dwFileDivided, pszVirusName, dwSpyID, bRescan, FILE_XML);
					if (bReturn)
						goto Cleanup;
				}
			}
		}
		else    //Scanning XML files if files are bigger than 50 kb
		{
			dwBytesSkipped = 0xF + 200;
			dwRemFileSize = m_dwFileSize - dwBytesSkipped;
			m_iXMLBufferOffset = 0xF + 200;
			dwBytesRead = 0;

			if (dwRemFileSize > 0x2000)
			{
				dwBytes2Read = 0x00;
				
				if (GetFileBuffer(m_szXMLFileBuffer, dwRemFileSize - 0x2000, 0x2000, 0x2000, &dwBytesRead))
				{
					m_iXMLBufferOffset += dwBytesRead;

					//Calculate MD5 and check in database
					//If found, construct malware name and return from here
					bReturn = ScanForMD5AndName(m_szXMLFileBuffer, dwBytesRead, pszVirusName, dwSpyID, bRescan, FILE_XML);
					if (bReturn)
						goto Cleanup;
				}

				DWORD dwRemSize = dwRemFileSize - 0x2000;
				DWORD dwBytes2Read = 0xC000;

				while (m_iXMLBufferOffset < dwRemSize)
				{
					dwBytesRead = 0x00;
					ZeroMemory(m_szXMLFileBuffer, _countof(m_szXMLFileBuffer));
					if (!GetFileBuffer(m_szXMLFileBuffer, m_iXMLBufferOffset, dwBytes2Read, dwBytes2Read, &dwBytesRead))
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
						//strlwr((char *)m_szXMLFileBuffer);
						bReturn = ScanForMD5AndName(m_szXMLFileBuffer, dwBytesRead, pszVirusName, dwSpyID, bRescan, FILE_XML);
						if (bReturn)//Detected
						{
							goto Cleanup;
						}
					}
					m_iXMLBufferOffset += dwBytesRead;
				}
			}
			else
				if (GetFileBuffer(m_szXMLFileBuffer, dwBytesSkipped, dwRemFileSize, dwRemFileSize, &dwBytesRead))
				{
					//strlwr((char *)m_szXMLFileBuffer);
					bReturn = ScanForMD5AndName(m_szXMLFileBuffer, m_iXMLBufferOffset, pszVirusName, dwSpyID, bRescan, FILE_XML);
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

/********************************************************************************************************
*  Function Name  : IsValidXMLFile
*  Description    : Function to check whether a file being scanned is a valid XML file before scanning
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 6 Apr 2016
*********************************************************************************************************/
bool CXMLScanner::IsValidXMLFile(LPCTSTR csFilePath)
{
	bool bReturn = false;
	__try
	{	

		BYTE bXMLHeader[0xF] = { 0x3c, 0x3F, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F,
			0x6E, 0x3D, 0x22 };
	
		BYTE bHeader[0xF] = { 0 };
		DWORD dwBytesRead = 0;

		m_hFileHandle = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			//return bReturn;
			goto Cleanup;
		}

		m_dwFileSize = GetFileSize(m_hFileHandle, NULL);

		if (/*(m_dwFileSize == 0xFFFFFFFF) ||*/ (m_dwFileSize == 0x00))
		{
			goto Cleanup;
		}

		if (!GetFileBuffer(bHeader, 0, sizeof(bHeader), sizeof(bHeader)))
		{
			//return bReturn;
			goto Cleanup;
		}

		if (_memicmp(bHeader, bXMLHeader, sizeof(bXMLHeader)) != 0)
		{
			goto Cleanup;
		}
		
		return true;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CXMLScanner::IsValidXMLFile, File: %s", csFilePath, 0, true, SECONDLEVEL);
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

/****************************************************************************************************************
*  Function Name  : ReadBufferFromDiffSectionOfXMLAndHash
*  Description    : Read XML files and generate hash .
*  Author Name    : Sagar
*  SR_NO		  :s
*  Date			  :	21 sept 2016,
*****************************************************************************************************************/

DWORD CXMLScanner::ReadBufferFromDiffSectionOfXMLAndHash( LPCTSTR pszFilePath)
{
	DWORD dwReturn = 0x00;

	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;
	DWORD dwBytesRead = 0;
	DWORD dwBytesSkipped = 0;
	DWORD dwRemFileSize = 0;
	BYTE bMD5Buffer[0x10] = { 0x00 };
	DWORD iBuffOffset = 0x00;
	
	DWORD dwChunkNumber = 0x00;
	LPCTSTR pszString = NULL;

	try
	{
		
		m_iXMLBufferOffset = 0x00;
		memset(m_szXMLFileBuffer, 0, sizeof(m_szXMLFileBuffer));

		if (m_dwFileSize < 0x6400)					// if file is < 25 kb
		{
			if (GetFileBuffer(m_szXMLFileBuffer, m_iXMLBufferOffset, m_dwFileSize, m_dwFileSize, &dwBytesRead))
			{
				++dwChunkNumber;
				dwReturn=GenerateBufferedFile(pszFilePath, m_szXMLFileBuffer, dwBytesRead, dwChunkNumber);//function to generate chunk
				if (dwReturn != 0x00)
				{
					dwReturn++;
					goto Cleanup;
				}
						
				dwReturn=ZeroFill(pszFilePath, m_iXMLBufferOffset, dwBytesRead, dwChunkNumber, pszString);
				if (dwReturn != 0x00)
				{
					dwReturn++;
					goto Cleanup;
				}
				
				dwBytesSkipped += dwBytesRead;
									
			}
		}
		else if (m_dwFileSize > 0x6400 && m_dwFileSize < 0xC800)// if file is > 25 kb and < 50 kb  i.e. 0xC800 - 50 kb
		{
			dwBytesSkipped = 0xF + 200; //Skip first 215 bytes
			dwRemFileSize = m_dwFileSize - dwBytesSkipped;
			DWORD dwFileDivided = dwRemFileSize / 2;

			while (dwBytesSkipped <= m_dwFileSize)
			{
				dwBytesRead = m_iXMLBufferOffset = 0x00;
				ZeroMemory(m_szXMLFileBuffer, _countof(m_szXMLFileBuffer));
				if (GetFileBuffer(m_szXMLFileBuffer, dwBytesSkipped, dwFileDivided, dwFileDivided, &dwBytesRead))
				{
					
					if (dwBytesRead == 0x00)
						break;

					++dwChunkNumber;
					dwReturn=GenerateBufferedFile(pszFilePath, m_szXMLFileBuffer, dwBytesRead, dwChunkNumber);//function to generate chunk
					if (dwReturn != 0x00)
					{
						dwReturn++;
						goto Cleanup;
					}
					
					dwReturn=ZeroFill(pszFilePath, m_iXMLBufferOffset, dwBytesRead, dwChunkNumber, pszString);
					if (dwReturn != 0x00)
					{
						dwReturn++;
						goto Cleanup;
					}
					
					dwBytesSkipped += dwBytesRead;
						
				}
			}
		}
		else    //Scanning XML files if files are bigger than 50 kb
		{
			dwBytesSkipped = 0xF + 200;
			dwRemFileSize = m_dwFileSize - dwBytesSkipped;
			m_iXMLBufferOffset = 0xF + 200;
			dwBytesRead = 0;

			if (dwRemFileSize > 0x2000)
			{
				dwBytes2Read = 0x00;

				if (GetFileBuffer(m_szXMLFileBuffer, dwRemFileSize - 0x2000, 0x2000, 0x2000, &dwBytesRead))
				{
					++dwChunkNumber;
					dwReturn = GenerateBufferedFile(pszFilePath, m_szXMLFileBuffer, dwBytesRead, dwChunkNumber);//function to generate chunk
					if (dwReturn != 0x00)
					{
						dwReturn++;
						goto Cleanup;
					}
					
					dwReturn = ZeroFill(pszFilePath, m_iXMLBufferOffset, dwBytesRead, dwChunkNumber, pszString);
					if (dwReturn != 0x00)
					{
						dwReturn++;
						goto Cleanup;
					}
					
					m_iXMLBufferOffset += dwBytesRead;
					dwBytesSkipped += dwBytesRead;
						
					
				}

				DWORD dwRemSize = dwRemFileSize - 0x2000;
				DWORD dwBytes2Read = 0xC000;

				while (m_iXMLBufferOffset < dwRemSize)
				{
					dwBytesRead = 0x00;
					ZeroMemory(m_szXMLFileBuffer, _countof(m_szXMLFileBuffer));
					if (!GetFileBuffer(m_szXMLFileBuffer, m_iXMLBufferOffset, dwBytes2Read, dwBytes2Read, &dwBytesRead))
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
						++dwChunkNumber;
						dwReturn = GenerateBufferedFile(pszFilePath, m_szXMLFileBuffer, dwBytesRead, dwChunkNumber);//function to generate chunk
						if (dwReturn != 0x00)
						{
							dwReturn++;
							goto Cleanup;
						}
						
						dwReturn = ZeroFill(pszFilePath, m_iXMLBufferOffset, dwBytesRead, dwChunkNumber, pszString);
						if (dwReturn != 0x00)
						{
							dwReturn++;
							goto Cleanup;
						}
					
						dwBytesSkipped += dwBytesRead;
					}

					m_iXMLBufferOffset += dwBytesRead;
				}
			}
			else
				if (GetFileBuffer(m_szXMLFileBuffer, dwBytesSkipped, dwRemFileSize, dwRemFileSize, &dwBytesRead))
				{
					
					++dwChunkNumber;
					dwReturn = GenerateBufferedFile(pszFilePath, m_szXMLFileBuffer, dwBytesRead, dwChunkNumber);//function to generate chunk
					if (dwReturn != 0x00)
					{
						dwReturn++;
						goto Cleanup;
					}
					
					dwReturn = ZeroFill(pszFilePath, m_iXMLBufferOffset, dwBytesRead, dwChunkNumber, pszString);
					if (dwReturn != 0x00)
					{
						dwReturn++;
						goto Cleanup;
					}
					
					dwBytesSkipped += dwBytesRead;	
					
				}
		}
	}
	catch (...)
	{
		dwReturn = 0x0FF;
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
*  Description    : Functio to generate's chunks
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  : 22 sep 2016
****************************************************************************************************/

DWORD CXMLScanner::GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber)
{
	CString pszFilePathCopy = pszFilePath;


	CString FileName(L"");
	CString cPathName(L"");
	
	CString csFileFullPath(L"");
	
	CString csFileName(L"");
	DWORD dwBytesWritten = 0x00;
	DWORD dwResult = 0x00;
	HANDLE hFile=INVALID_HANDLE_VALUE;



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
*  Function Name  : GenerateBufferedFile
*  Description    : Functio to fill buffer with Zero
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  : 22 sep 2016
****************************************************************************************************/
DWORD CXMLScanner::ZeroFill(LPCTSTR pszFilePath, DWORD dwStartAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile)
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
				Sleep(500);
				bCopy = CopyFile(pszFilePath, csFileFullPath, 0);
				
				dwResult=0x01;
				goto Cleanup;
			}

		}
		
		m_hFileHandle = CreateFile(csFileFullPath, GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			dwResult=0x02;
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
			dwResult=0x03;

			goto Cleanup;
		}

		memset(pbyFillZeroBuff, 0, dwFillZeroCount);

		DWORD m_dwBytesWritten = 0x00;
		SetFilePointer(m_hFileHandle, dwStartAddress, 0, FILE_BEGIN);

		WriteFile(m_hFileHandle, pbyFillZeroBuff, dwFillZeroCount, &m_dwBytesWritten, 0);
		if (dwFillZeroCount != m_dwBytesWritten)
		{
			dwResult=0x04;
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