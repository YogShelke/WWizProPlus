/****************************************************************************
Program Name          : WardWizPEScanner.cpp
Description           : This file contains scanning mechanism of PE files
Author Name			  : Ram Shelke
Date Of Creation      : 05th Oct 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#include "WardWizPEScanner.h"
#include "WardWizHash.h"

#pragma warning(disable:4995)

#define		WRDWIZEXCLUDECLONEDB			L"VBEXCLUDECLONE.DB"
#define		WRDWIZEXTEXCLUDECLONEDB			L"VBEXCLUDEEXT.DB"
#define		WRDWIZSCANNAMECLONEDB			L"VBSCANNAMECLONE.DB"

const unsigned int				WRDWIZ_SIG_SIZE = 0x07;
const unsigned char				WRDWIZ_SIG[WRDWIZ_SIG_SIZE + 1] = "VIBRONM";
HANDLE							m_hFileHandle = INVALID_HANDLE_VALUE;
LONGLONG						m_dwFileSize = 0x00;
int								m_iBufferOffset = 0x00;
WORD							m_NumberOfSections;
bool							m_IS64Bit;
IMAGE_OPTIONAL_HEADER32			m_stImageOptionalHeader32 = { 0 };;
IMAGE_OPTIONAL_HEADER64			m_stImageOptionalHeader64 = { 0 };;
DWORD							m_dwOffset2NewEXEHeader;
IMAGE_SECTION_HEADER			m_stSectionHeader[100] = { 0 };;
DWORD							m_dwRVA[100] = { 0 };;
DWORD							m_dwVS[100] = { 0 };;
DWORD							m_dwPRD[100] = { 0 };
DWORD							m_dwSRD[100] = { 0 };
BYTE							m_szFileBuffer[25000] = { 0 };
TCHAR							m_szFilePath[MAX_FILEPATH_LENGTH] = { 0 };
DWORD							m_dwArrPESigCountPerDB[0x200] = { 0 };
DWORD							m_dwTotalPESigCount = 0x00;
DWORD							m_dwPEScanProdID = 0x01;

/***********************************************************************************************
*  Function Name  : LoadSignatureDB
*  Description    : Function which loads WardWiz signatures database
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool LoadSignatureDB()
{
	bool bReturn = false;
	__try
	{
		m_dwTotalPESigCount = 0x00;
		DWORD dwFileno = 1;

		//Added more DB load support ( 512 DB files )
		for (dwFileno = 1; dwFileno < 0x200; dwFileno++)
		{
			DECLARE_UNICODE_STRING_SIZE(uFileName, 10);

			WCHAR szFileName[0x50] = { 0 };
			if (dwFileno >= 1 && dwFileno <= 9)
			{
				RtlIntegerToUnicodeString((unsigned long)dwFileno, 0, &uFileName);
				wcscpy(szFileName, L"00");
				wcscat(szFileName, uFileName.Buffer);
			}
			else  if (dwFileno >= 10 && dwFileno <= 99)
			{
				RtlIntegerToUnicodeString((unsigned long)dwFileno, 0, &uFileName);
				wcscpy(szFileName, L"0");
				wcscat(szFileName, uFileName.Buffer);
			}
			else  if (dwFileno >= 100 && dwFileno <= 999)
			{
				RtlIntegerToUnicodeString((unsigned long)dwFileno, 0, &uFileName);
				wcscat(szFileName, uFileName.Buffer);
			}
			else
			{
				break;
			}

			WCHAR szFilePath[0x50] = { 0 };
			wcscpy(szFilePath, szAppPath);
			wcscat(szFilePath, L"\\VBDB\\");

				wcscat(szFilePath, AVPE32DBNAME);

			wcscat(szFilePath, szFileName);
			wcscat(szFilePath, L".DB");

			//DbgPrint("FileName: [%S]\n", szFilePath);

			if (!FileExists(szFilePath))
			{
				RtlAddLogEntry(ZEROLEVEL, L"### File Does not exist: [%s]", szFilePath);
				break;
			}

			DWORD dwMajorversion = 0x00;
			DWORD dwVersionLength = 0x00;
			if (!ISValidDBFile(szFilePath, dwMajorversion, dwVersionLength))
			{
				RtlAddLogEntry(ZEROLEVEL, L"### Invalid DB file: [%s]", szFilePath);
				continue;
			}

			//Check if DB file is required version.
			if (dwMajorversion >= 0x01)
			{
				DWORD dwSigCount = 0x00;
				if (!LoadContaintFromFile(szFilePath, FILE_PE, dwVersionLength, dwSigCount))
				{
					RtlAddLogEntry(ZEROLEVEL, L"### Failed LoadContaintFromFile file: [%s]", szFilePath);
					continue;
				}

				SetSigCountAsPerListID(dwFileno - 1, dwSigCount);
				m_dwTotalPESigCount += dwSigCount;
			}
		}

		//DbgPrint("\nTotal Signatures Loaded: [%d]", m_dwTotalPESigCount);

		bReturn = true;
		//DbgPrint("\n>>> Out LoadSignatureDB\n");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in LoadSignatureDB");
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ISValidDBFile
*  Description    : Function which check wheather the file is valid or not.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool ISValidDBFile(LPTSTR szFilePath, DWORD &dwDBMajorVersion, DWORD &dwVersionLength)
{
	bool bReturn = false;
	HANDLE hFile = NULL;
	__try
	{
		//DbgPrint("\n>>> In ISValidDBFile, File: [%S]\n", szFilePath);

		if (szFilePath == NULL)
		{
			return bReturn; //error
		}

		if (!NtFileOpenFile(&hFile, szFilePath, FALSE, FALSE))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Unable to open DB File: [%s]", szFilePath);
			return bReturn;
		}

		LONGLONG lFileSize = 0x00;
		NtFileGetFileSize(hFile, &lFileSize);
		if (lFileSize < MAX_SIG_SIZE)
		{
			NtFileCloseFile(hFile);
			hFile = NULL;
			return bReturn;
		}

		NtFileSeekFile(hFile, 0x00);

		bool bValidSig = false;
		char	bySigBuff[MAX_VERSIONCHKBUFF] = { 0x00 };

		DWORD dwBytesRead = 0x00;
		if (!NtFileReadFile(hFile, &bySigBuff, MAX_SIG_SIZE, &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### NtFileReadFile Failed in ISValidDBFile, File: [%s]", szFilePath);
			goto CLEANUP;
		}

		char	byEncDecSig[MAX_SIG_SIZE + 1] = "WRDWIZDB";
		if (memcmp(&bySigBuff, &byEncDecSig[0x00], MAX_SIG_SIZE) == 0x00)
		{
			bValidSig = true;
		}

		NtFileSeekFile(hFile, 0x00);

		//Get here the version number by tokenizing.
		memset(&bySigBuff, 0, MAX_VERSIONCHKBUFF);
		
		dwBytesRead = 0x00;
		if (!NtFileReadFile(hFile, &bySigBuff, MAX_VERSIONCHKBUFF, &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### NtFileReadFile Failed in ISValidDBFile, File: [%s]", szFilePath);
			goto CLEANUP;
		}

		//Tokenize buffer
		char seps[] = "|";
		char *token = NULL;
		char* context = NULL;
		token = xstrtok(bySigBuff, seps);

		DWORD dwCount = 0x00;
		bool bValidSigLength = false;
		bool bValidVersion = false;
		while (token != NULL)
		{
			if (strlen(token) > 0)
			{
				if (dwCount == 0)
				{
					if (strlen(token) == 0x0A)
					{
						bValidSigLength = true;
					}
				}

				if (dwCount >= 1)
				{
					break;
				}
				dwCount++;
			}
			token = xstrtok(NULL, seps);
		}

		if (token != NULL)
		{
			DWORD dwlength = (DWORD)strlen(token);
			if (dwlength > 0)
			{
				/* Parse here DB vesion and get major vers*/
				if (ParseDBVersion(token, dwDBMajorVersion))
				{
					bValidVersion = true;
					dwVersionLength = dwlength;
				}
			}
		}

		if (bValidSig && bValidVersion && bValidSigLength)
		{
			bReturn = true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ISValidDBFile");
	}
CLEANUP:
	if (hFile != NULL)
	{
		NtFileCloseFile(hFile);
		hFile = NULL;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ParseDBVersion
*  Description    : Function which parse DB file version.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool ParseDBVersion(LPSTR lpszVersion, DWORD &dwMajorVersion)
{
	bool bReturn = false;
	__try
	{
		if (!lpszVersion)
			return false;

		const char sToken[2] = ".";
		char *token;
		int iCount = 0;

		/* get the first token */
		token = xstrtok(lpszVersion, sToken);

		/* walk through other tokens */
		while (token != NULL && (iCount <= 3))
		{
			/* take major version from here */
			if (iCount == 1)
			{
				if (strlen(token) > 0)
				{
					dwMajorVersion = (DWORD)(atoi(token));
					break;
				}
			}
			iCount++;
			token = xstrtok(NULL, sToken);
		}
		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ParseDBVersion");
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : LoadContaintFromFile
*  Description    : Function which Loads contents from db file and adds in linked list.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool LoadContaintFromFile(LPTSTR szFilePath, FILETYPE filetype, DWORD dwVersionLength, DWORD &dwSigCount)
{
	bool	bReturn = false;
	HANDLE	hInputFileHandle = NULL;
	BYTE	*bFileBuffer = NULL;
	__try
	{
		//DbgPrint(">>> In LoadContaintFromFile, File: [%S]\n", szFilePath);

		if (!NtFileOpenFile(&hInputFileHandle, szFilePath, FALSE, FALSE))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileOpenFile in LoadContaintFromFile, File: [%s]", szFilePath);
			return false;
		}

		/* Get file size here */
		LONGLONG lFileSize = 0x00;
		if (!NtFileGetFileSize(hInputFileHandle, &lFileSize))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileGetFileSize in LoadContaintFromFile, File: [%s]", szFilePath);
			goto CLEANUP;
		}

		/* If file size if 0 return */
		if (lFileSize == 0x00)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DWORD dwBufferOffset = MAX_SIG_SIZE + 0x02 + dwVersionLength + 0x02;
		NtFileSeekFile(hInputFileHandle, dwBufferOffset);

		//Read here remaining file
		dwBufferOffset += sizeof(ULONG64);
		NtFileSeekFile(hInputFileHandle, dwBufferOffset);

		DWORD dwRemSize = (DWORD)lFileSize - dwBufferOffset;
		bFileBuffer = (LPBYTE)RtlAllocateHeap(RtlProcessHeap(), 0, dwRemSize);
		
		if (bFileBuffer == NULL)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DWORD dwBytesRead = 0;
		if (!NtFileReadFile(hInputFileHandle, bFileBuffer, dwRemSize, &dwBytesRead))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != dwRemSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		dwSigCount = 0x00;
		DWORD iBufOffset = 0x00;
		while (iBufOffset < dwRemSize)
		{
			BYTE byMD5Hex[0x04] = { 0 };
			memcpy(&byMD5Hex, &bFileBuffer[iBufOffset], sizeof(byMD5Hex));
			
			InsertItem(byMD5Hex, sizeof(byMD5Hex));
			
			dwSigCount++;
			iBufOffset += 0x10;
		}

		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in LoadContaintFromFile, %s", szFilePath);
		bReturn = false;
	}
CLEANUP:
	//Cleanup here memory
	if (bFileBuffer != NULL)
	{
		RtlFreeHeap(RtlProcessHeap(), 0, bFileBuffer);
		bFileBuffer = NULL;
	}

	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}
	
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : UnLoadSignatureDB
*  Description    : Function which Unloads signature database
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool UnLoadSignatureDB()
{
	bool bReturn = false;
	__try
	{
		bReturn = UnInitializeTree();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in UnLoadSignatureDB");
	}
	return bReturn;
}


/***********************************************************************************************
*  Function Name  : FreeHeapAllocMemory
*  Description    : Function which Unloads memory which allocated on heap
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 05-Oct-2017
*************************************************************************************************/
bool FreeHeapAllocMemory()
{
	bool bReturn = false;
	__try
	{
		bReturn = FreeHeapAllocTreeMemory();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in UnLoadSignatureDB");
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : IsValidPEFile
*  Description    : Function to check whether the file is a valid PE file
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool RtlIsValidPEFile(LPTSTR szFilePath)
{
	bool bReturn = false;
	__try
	{
		if (!szFilePath)
		{
			return bReturn;
		}

		if (m_hFileHandle != INVALID_HANDLE_VALUE)
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
		}

		if (!NtFileOpenFile(&m_hFileHandle, szFilePath, FALSE, FALSE, FALSE))
		{
			return bReturn;
		}

		if (!NtFileGetFileSize(m_hFileHandle, &m_dwFileSize))
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		if (m_dwFileSize == 0xFFFFFFFF)
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		m_iBufferOffset = 0;
		m_NumberOfSections = 0;
		m_IS64Bit = false;

		memset(&m_stImageOptionalHeader32, 0, sizeof(m_stImageOptionalHeader32));
		memset(&m_stImageOptionalHeader64, 0, sizeof(m_stImageOptionalHeader64));

		memset(&m_stImageOptionalHeader32, 0, sizeof(m_stImageOptionalHeader32));
		memset(&m_stImageOptionalHeader64, 0, sizeof(m_stImageOptionalHeader64));

		IMAGE_DOS_HEADER stImageDosHeader;
		memset(&stImageDosHeader, 0, sizeof(stImageDosHeader));

		DWORD dwBytesRead = 0;
		if (!RtlGetFileBuffer(&stImageDosHeader, 0, sizeof(stImageDosHeader), sizeof(stImageDosHeader)))
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		if (stImageDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		DWORD dwSignature = 0;
		m_dwOffset2NewEXEHeader = stImageDosHeader.e_lfanew;
		if (!RtlGetFileBuffer(&dwSignature, stImageDosHeader.e_lfanew, sizeof(DWORD), sizeof(DWORD)))
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		if (dwSignature != IMAGE_NT_SIGNATURE)
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		IMAGE_FILE_HEADER stImageFileHeader;
		memset(&stImageFileHeader, 0, sizeof(stImageFileHeader));

		if (!RtlGetFileBuffer(&stImageFileHeader, stImageDosHeader.e_lfanew + 4, sizeof(stImageFileHeader), sizeof(stImageFileHeader)))
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		if (stImageFileHeader.NumberOfSections > 0x63 || 0 == stImageFileHeader.NumberOfSections)
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		m_NumberOfSections = stImageFileHeader.NumberOfSections;

		if (IMAGE_FILE_MACHINE_IA64 == stImageFileHeader.Machine || IMAGE_FILE_MACHINE_AMD64 == stImageFileHeader.Machine)
		{
			dwBytesRead = 0;
			memset(&m_stImageOptionalHeader64, 0, sizeof(m_stImageOptionalHeader64));
			if (!RtlGetFileBuffer(&m_stImageOptionalHeader64, sizeof(m_stImageOptionalHeader64), &dwBytesRead))
			{
				NtFileCloseFile(m_hFileHandle);
				m_hFileHandle = INVALID_HANDLE_VALUE;
				return false;
			}
			m_IS64Bit = true;
		}
		else
		{
			dwBytesRead = 0;
			memset(&m_stImageOptionalHeader32, 0, sizeof(m_stImageOptionalHeader32));
			if (!RtlGetFileBuffer(&m_stImageOptionalHeader32, sizeof(m_stImageOptionalHeader32), &dwBytesRead))
			{
				NtFileCloseFile(m_hFileHandle);
				m_hFileHandle = INVALID_HANDLE_VALUE;
				return false;
			}
		}

		memset(&m_stSectionHeader, 0, sizeof(m_stSectionHeader));
		if (!RtlGetFileBuffer(&m_stSectionHeader, sizeof(m_stSectionHeader), &dwBytesRead))
		{
			NtFileCloseFile(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}
		for (int i = 0; i < m_NumberOfSections; i++)
		{
			m_dwRVA[i] = m_stSectionHeader[i].VirtualAddress;
			m_dwVS[i] = m_stSectionHeader[i].Misc.VirtualSize;
			m_dwPRD[i] = m_stSectionHeader[i].PointerToRawData;
			m_dwSRD[i] = m_stSectionHeader[i].SizeOfRawData;

		}
		bReturn = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlIsValidPEFile, File: %s", szFilePath);
		bReturn = false;
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Function to get the file buffer while scanning PE files
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool RtlGetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
{
	__try
	{
		if (!m_hFileHandle)
		{
			return false;
		}

		DWORD dwBytesRead = 0x00;

		if (!NtFileSeekFile(m_hFileHandle, dwReadOffset))
		{
			return false;
		}

		if (!NtFileReadFile(m_hFileHandle, pbReadBuffer, dwBytesToRead, &dwBytesRead))
		{
			return false;
		}

		if (dwBytesRead && dwBytesRead >= dwMinBytesReq)
		{
			if (pdwBytesRead)
			{
				*pdwBytesRead = dwBytesRead;
			}
			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlGetFileBuffer");
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Function to get the file buffer while scanning PE files
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool RtlGetFileBuffer(LPVOID pbReadBuffer, DWORD dwBytesToRead, DWORD * pdwBytesRead)
{
	__try
	{
		if (!m_hFileHandle)
		{
			return false;
		}

		DWORD dwBytesRead = 0;

		if (!pbReadBuffer)
		{
			return false;
		}

		if (!NtFileReadFile(m_hFileHandle, pbReadBuffer, dwBytesToRead, &dwBytesRead))
		{
			return false;
		}

		if (pdwBytesRead)
		{
			*pdwBytesRead = dwBytesRead;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in CPEScanner::GetFileBuffer");
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : RtlScanFile
*  Description    : Function to Which scans input file and returns result with @true & @false.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 06 Oct 2017
****************************************************************************************************/
bool RtlScanFile(LPTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, bool bIsHeuScan)
{
	bool bReturn = false;
	__try
	{
		if (pszFilePath == NULL || pszVirusName == NULL)
		{
			return bReturn;
		}

		//don't scan the file which has length more than we used.
		if (wcslen(pszFilePath) >= MAX_FILEPATH_LENGTH)
		{
			return bReturn;
		}

		if (!FileExists(pszFilePath))
		{
			return bReturn;
		}

		if (!RtlIsValidPEFile(pszFilePath))
		{
			return bReturn;
		}

		//if file is 64 bit then no  need to scan.
		if (m_IS64Bit)
		{
			return bReturn;
		}

		wcscpy(m_szFilePath, pszFilePath);

		if (!ReadBufferFromDiffSections())
		{
			return false;
		}

		if (!m_szFileBuffer)
		{
			return false;
		}

		BYTE byMD5Buffer[16] = { 0 };
		if (GetByteStringHash(m_szFileBuffer, m_iBufferOffset, byMD5Buffer) != 0x00)
		{
			return false;
		}

		DWORD dwIndex = 0x00;
		bool bFound = false;
		while (dwIndex < m_dwTotalPESigCount)
		{
			if (FindPosition(byMD5Buffer, sizeof(byMD5Buffer), dwIndex, dwSpyID))
			{
				bFound = true;
			}

			//If found first 4 bytes then go for remaining 12 bytes.
			if (bFound)
			{
				RtlAddLogEntry(ZEROLEVEL, L">>> Matching Secondary signature for Index: [%d] ", dwSpyID);
				bFound = MatchSecondarySig(byMD5Buffer, dwIndex++, dwSpyID); //Index - 1 as because while loading adding vector starts from 1
				if (bFound)
				{
					DECLARE_UNICODE_STRING_SIZE(uFileName, 20);
					RtlIntegerToUnicodeString((unsigned long)dwIndex, 0, &uFileName);
					wcscpy(pszVirusName, L"W32.Mal-");
					wcscat(pszVirusName, uFileName.Buffer);
					bReturn = true;
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlScanFile");
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSections
*  Description    : Function to read from different sections while scanning PE files
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool ReadBufferFromDiffSections()
{
	bool bReturn = false;
	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;

	memset(m_szFileBuffer, 0, sizeof(m_szFileBuffer));

	//Read 0x300 bytes file start
	DWORD dwBytesRead = 0;
	dwFileOffset = 0;
	if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x300, 0, &dwBytesRead))
	{
		m_iBufferOffset += dwBytesRead;
		bReturn = true;
	}

	//Read 0x1000 bytes start from AEP
	dwBytesRead = 0;
	dwFileOffset = 0;
	DWORD rvaAEP = m_IS64Bit ? m_stImageOptionalHeader64.AddressOfEntryPoint : m_stImageOptionalHeader32.AddressOfEntryPoint;
	WORD wSec = (WORD)RVA2FileOffset(rvaAEP, m_NumberOfSections, &dwFileOffset);

	dwBytes2Read = m_dwSRD[wSec];
	if (m_dwSRD[wSec] > 0x1000)
	{
		dwBytes2Read = 0x1000;
	}

	if (dwFileOffset > 0)
	{
		if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwBytes2Read, 0, &dwBytesRead))
		{
			m_iBufferOffset += dwBytesRead;
			bReturn = true;
		}
	}

	//Read 0x1000 bytes start of last section
	dwBytesRead = 0;
	int iLastSection = m_NumberOfSections - 1;
	dwFileOffset = m_dwPRD[iLastSection];
	if (dwFileOffset > 0)
	{
		if (m_dwSRD[iLastSection] < 0x1000)
		{
			if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, m_dwSRD[iLastSection], 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
		else
		{
			if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x1000, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}

	//Read 0x1000 bytes from End of last section
	dwBytesRead = 0;
	if (m_dwSRD[iLastSection] > 0x1000)
	{
		dwFileOffset = m_dwPRD[iLastSection] + m_dwSRD[iLastSection] - 0x1000;
		if (dwFileOffset > 0)
		{
			dwBytes2Read = 0x1000;
			/*if((m_dwSRD[iLastSection] - 0x1000) < 0x1000)
			{
			dwBytes2Read = m_dwSRD[iLastSection] - 0x1000;
			}*/
			if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwBytes2Read, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}

	//Read 0x1000 bytes from Flash Data
	dwBytesRead = 0;
	DWORD dwFlashDataSize = (DWORD)m_dwFileSize - (m_dwPRD[iLastSection] + m_dwSRD[iLastSection]);
	if (dwFlashDataSize > 0)
	{
		dwFileOffset = m_dwPRD[iLastSection] + m_dwSRD[iLastSection];
		if (dwFlashDataSize > 0x1000)
		{
			if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x1000, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
		else
		{
			if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwFlashDataSize, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}

	/*

	__try
	{
		DWORD dwOffset = 0x00;
		//File Version Information
		dwBytesRead = 0x0;
		dwBytesRead = GetFileVersionInfoSizeW(m_szFilePath, NULL, &dwOffset);
		//DbgPrint("\n >>> GetFileVersionInfoSize: [%d]", dwBytesRead);
		if (dwBytesRead)
		{
			dwBytes2Read = dwBytesRead;
			if (dwBytesRead > 0x1000)
			{
				dwBytes2Read = 0x1000;
			}

			if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwOffset, dwBytes2Read, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### GetFileVersionInfoSizeW Failed, Corrupt file: %s", m_szFilePath);
	}


	*/

	//0x300 bytes from .Data Section
	dwBytesRead = 0x0;
	// Base of data offset
	DWORD dwOffset = m_dwOffset2NewEXEHeader + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + 0x18;
	if (RtlGetFileBuffer(&dwOffset, dwOffset, 4, 4))
	{
		if (dwOffset)
		{
			RVA2FileOffset(dwOffset, m_NumberOfSections, &dwOffset);
			if (dwOffset < m_dwFileSize)
			{
				if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwOffset, 0x300, 0, &dwBytesRead))
				{
					m_iBufferOffset += dwBytesRead;
				}
			}
		}
	}
	// Read 0x300 bytes from .data section PRD if .data section is not Base of Data 
	for (int iSecCnt = 0; iSecCnt < m_NumberOfSections; iSecCnt++)
	{
		BYTE byDataSecName[] = { 0x2E, 0x64, 0x61, 0x74, 0x61 }; // .data
		if (memcmp(m_stSectionHeader[iSecCnt].Name, byDataSecName, sizeof(byDataSecName)) == 0)
		{
			if (dwOffset != m_dwPRD[iSecCnt])
			{
				dwBytesRead = 0;
				if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], m_dwPRD[iSecCnt], 0x300, 0, &dwBytesRead))
				{
					m_iBufferOffset += dwBytesRead;
				}
				break;
			}
		}
	}
	bReturn = true;

	// Read 0x300 bytes from AEP - 0x300
	dwBytesRead = 0x0;
	DWORD dwAEPMapped = 0x0;
	DWORD dwAEPUnmapped = m_IS64Bit ? m_stImageOptionalHeader64.AddressOfEntryPoint : m_stImageOptionalHeader32.AddressOfEntryPoint;
	WORD wAEPSec = (WORD)RVA2FileOffset(dwAEPUnmapped, m_NumberOfSections, &dwAEPMapped);

	if (m_dwSRD[wAEPSec] > 0x300)
	{
		dwBytes2Read = 0x300;
	}
	if (dwAEPMapped > 0 && dwAEPMapped < m_dwFileSize)
	{
		if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwAEPMapped - 0x300, dwBytes2Read, 0, &dwBytesRead))
		{
			m_iBufferOffset += dwBytesRead;
			bReturn = true;
		}
	}

	// Read 0x300 bytes from Cavity
	dwBytesRead = 0x0;
	dwFileOffset = m_dwOffset2NewEXEHeader + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
	DWORD dwOptHeaderSize = m_IS64Bit ? sizeof(m_stImageOptionalHeader64) : sizeof(m_stImageOptionalHeader32);
	dwFileOffset += dwOptHeaderSize + m_NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	if (dwFileOffset < m_dwFileSize)
	{
		if (RtlGetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x300, 0, &dwBytesRead))
		{
			m_iBufferOffset += dwBytesRead;
			bReturn = true;
		}
	}

	//Need to close file handle after collecting buffer
	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : RVA2FileOffset
*  Description    : Function to convert virtual address to file offset
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
DWORD RVA2FileOffset(DWORD dwRva, WORD nSections, DWORD *pdwFileOff)
{
	WORD	wSec = 0x00;
	__try
	{
		//If number of sections are greater than 100, we leave
		if (nSections > 0x63)
			return 0;

		//Issue was :: RVA to File Offset was not getting correct due to RVA size is not equal to SRD
		//Modified by Vilas on 18 April 2015
		for (; wSec < nSections; wSec++)
		{
			if ((dwRva >= m_dwRVA[wSec]) &&
				((dwRva < (m_dwRVA[wSec] + m_dwVS[wSec])) || (dwRva < (m_dwRVA[wSec] + m_dwSRD[wSec]))))
			{
				*pdwFileOff = dwRva - m_dwRVA[wSec] + m_dwPRD[wSec];
				break;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in CPEScanner::RVA2FileOffset");
	}
	return wSec;
}


/***************************************************************************************************
*  Function Name  : GetFileVersionInfoSizeW
*  Description    : Function which returns File version information size from PE  file
					for IMAGE_OS2_SIGNATURE not implemented.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
DWORD WINAPI GetFileVersionInfoSizeW(LPCWSTR filename, LPDWORD handle, DWORD *offset)
{
	__try
	{
		return GetFileVersionInfoSizeExW(0, filename, handle, offset);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in CPEScanner::RVA2FileOffset");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetFileVersionInfoSizeExW
*  Description    : Function which returns File version information size from PE  file
					for IMAGE_OS2_SIGNATURE not implemented.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
DWORD WINAPI GetFileVersionInfoSizeExW(DWORD flags, LPCWSTR filename, LPDWORD handle, DWORD *offset)
{
	__try
	{
		DWORD len, offset, magic = 1;
		if (flags)
		{
			return 0;
		}

		if (handle) *handle = 0;

		if (!filename)
		{
			RtlAddLogEntry(SECONDLEVEL, L"ERROR_INVALID_PARAMETER, File:[%s]", filename);
			return 0;
		}

		if (!*filename)
		{
			RtlAddLogEntry(SECONDLEVEL, L"ERROR_BAD_PATHNAME, File:[%s]", filename);
			return 0;
		}

		//DbgPrint("\n >>> File: [%S]", filename);

		HANDLE hFile = NULL;
		if (!NtFileOpenFile(&hFile, (LPWSTR)filename, FALSE, FALSE, FALSE))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileOpenFile, File:[%s]", filename);
			return 0;
		}

		if (hFile != NULL)
		{
			magic = find_version_resource(hFile, &len, &offset);
			NtFileCloseFile(hFile);
		}

		/*if ((magic == 1) && (hModule = LoadLibraryExW(filename, 0, LOAD_LIBRARY_AS_DATAFILE)))
		{
		HRSRC hRsrc = FindResourceW(hModule, MAKEINTRESOURCEW(VS_VERSION_INFO),
		(LPWSTR)VS_FILE_INFO);
		if (hRsrc)
		{
		magic = IMAGE_NT_SIGNATURE;
		len = SizeofResource(hModule, hRsrc);
		}
		FreeLibrary(hModule);
		}*/

		switch (magic)
		{
		case IMAGE_OS2_SIGNATURE:
			/* We have a 16bit resource.
			*
			* XP/W2K/W2K3 uses a buffer which is more than the actual needed space:
			*
			* (info->wLength - sizeof(VS_FIXEDFILEINFO)) * 4
			*
			* This extra buffer is used for ANSI to Unicode conversions in W-Calls.
			* info->wLength should be the same as len. Currently it isn't but that
			* doesn't seem to be a problem (len is bigger than info->wLength).
			*/
			return (len - sizeof(VS_FIXEDFILEINFO)) * 4;
		case IMAGE_NT_SIGNATURE:
			/* We have a 32bit resource.
			*
			* XP/W2K/W2K3 uses a buffer which is 2 times the actual needed space + 4 bytes "FE2X"
			* This extra buffer is used for Unicode to ANSI conversions in A-Calls
			*/
			return (len * 2) + 4;

		default:
			RtlAddLogEntry(SECONDLEVEL, L"ERROR_RESOURCE_DATA_NOT_FOUND, File:[%s]", filename);
			return 0;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in GetFileVersionInfoSizeExW");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : find_version_resource
*  Description    : Function which determines file type and calculate resource length and its 
					offset.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
DWORD find_version_resource(HANDLE lzfd, DWORD *reslen, DWORD *offset)
{
	DWORD magic = 0x00;
	__try
	{
		magic = read_xx_header(lzfd);
		switch (magic)
		{
		case IMAGE_OS2_SIGNATURE:
			magic = 0;
			break;
		case IMAGE_NT_SIGNATURE:
			if (!find_pe_resource(lzfd, reslen, offset)) magic = 0;
			break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in read_xx_header");
	}
	return magic;
}

/***************************************************************************************************
*  Function Name  : read_xx_header
*  Description    : Function which determines file type
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
int read_xx_header(HANDLE lzfd)
{
	__try
	{
		IMAGE_DOS_HEADER mzh;
		char magic[3];

		if (!NtFileSeekFile(lzfd, 0))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileSeekFile in read_xx_header");
			return 0;
		}

		DWORD dwBytesRead = 0x00;
		if (!NtFileReadFile(lzfd, &mzh, sizeof(mzh), &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileReadFile in read_xx_header");
			return 0;
		}

		if (sizeof(mzh) != dwBytesRead)
			return 0;

		if (mzh.e_magic != IMAGE_DOS_SIGNATURE)
		{
			if (!memcmp(&mzh, "\177ELF", 4)) return 1;  /* ELF */
			if (*(UINT *)&mzh == 0xfeedface || *(UINT *)&mzh == 0xcefaedfe) return 1;  /* Mach-O */
			return 0;
		}

		if (!NtFileSeekFile(lzfd, mzh.e_lfanew))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileSeekFile in read_xx_header");
			return 0;
		}

		dwBytesRead = 0x00;
		if (!NtFileReadFile(lzfd, magic, 2, &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileReadFile in read_xx_header");
			return 0;
		}

		if (2 != dwBytesRead)
			return 0;

		if (!NtFileSeekFile(lzfd, mzh.e_lfanew))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileSeekFile in read_xx_header");
			return 0;
		}

		if (magic[0] == 'N' && magic[1] == 'E')
			return IMAGE_OS2_SIGNATURE;
		if (magic[0] == 'P' && magic[1] == 'E')
			return IMAGE_NT_SIGNATURE;

		magic[2] = '\0';

		RtlAddLogEntry(SECONDLEVEL, L"Can't handle %s files.\n", magic);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in read_xx_header");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : find_pe_resource
*  Description    : Function which gets PE file resource information.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
BOOL find_pe_resource(HANDLE lzfd, DWORD *resLen, DWORD *resOff)
{
	union
	{
		IMAGE_NT_HEADERS32 nt32;
		IMAGE_NT_HEADERS64 nt64;
	} pehd;

	BOOL ret = FALSE;
	PIMAGE_DATA_DIRECTORY resDataDir = NULL;
	LPBYTE resSection = NULL;
	__try
	{
		DWORD pehdoffset;
		DWORD section_size, data_size;
		const void *resDir;
		IMAGE_RESOURCE_DIRECTORY *resPtr;
		IMAGE_RESOURCE_DATA_ENTRY *resData;
		int i= 0, len = 0;

		LONGLONG lPostion = 0x00;
		if (!NtFileGetFilePosition(lzfd, &lPostion))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileGetFilePosition in find_pe_resource");
			return 0;
		}

		pehdoffset = (DWORD)lPostion;

		DWORD dwBytesRead = 0x00;
		if (!NtFileReadFile(lzfd, &pehd, sizeof(pehd), &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileReadFile in find_pe_resource");
			return 0;
		}

		len = dwBytesRead;

		if (dwBytesRead < sizeof(pehd.nt32.FileHeader)) return FALSE;
		if (dwBytesRead < sizeof(pehd)) memset((char *)&pehd + dwBytesRead, 0, sizeof(pehd) - dwBytesRead);

		switch (pehd.nt32.OptionalHeader.Magic)
		{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			resDataDir = pehd.nt32.OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
			break;
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			resDataDir = pehd.nt64.OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_RESOURCE;
			break;
		default:
			return FALSE;
		}

		if (!resDataDir->Size)
		{
			return FALSE;
		}

		/* Find resource section */
		for (i = 0; i < m_NumberOfSections; i++)
		{
			if (resDataDir->VirtualAddress >= m_dwRVA[i] && resDataDir->VirtualAddress < m_dwRVA[i] + m_dwSRD[i])
				break;
		}

		if (i == m_NumberOfSections)
		{
			return FALSE;
		}

		/* Read in resource section */
		data_size = m_dwSRD[i];// sections[i].SizeOfRawData;
		section_size = data_size > m_dwVS[i] ? data_size : m_dwVS[i];

		resSection = (LPBYTE)kmalloc(m_hHeapMem, section_size);
		if (!resSection)
		{
			return FALSE;
		}

		memset(resSection, 0, section_size);

		if (!NtFileSeekFile(lzfd, m_dwPRD[i]))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileSeekFile in find_pe_resource");
			return 0;
		}

		dwBytesRead = 0x00;
		if (!NtFileReadFile(lzfd, resSection, section_size, &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed NtFileReadFile in find_pe_resource");
			return 0;
		}

		if (section_size != dwBytesRead) goto done;

		if (data_size < section_size)
		{
			memset((LPBYTE)resSection + data_size, 0, section_size - data_size);
		}

		//DbgPrint("\n >>> resDataDir->VirtualAddress [%x], m_dwRVA: [%x]", resDataDir->VirtualAddress, m_dwRVA[i]);

		/* Find resource */
		resDir = resSection + (resDataDir->VirtualAddress - m_dwRVA[i]);

		resPtr = (IMAGE_RESOURCE_DIRECTORY*)resDir;
		
		//DbgPrint("\n >>> NumberOfNamedEntries: [%d]", resPtr->NumberOfNamedEntries);

		resPtr = find_entry_by_id(resPtr, (WORD)RT_VERSION, resDir);
		if (!resPtr)
		{
			RtlAddLogEntry(SECONDLEVEL, L"No typeid entry found");
			goto done;
		}

		resPtr = find_entry_by_id(resPtr, VS_VERSION_INFO, resDir);
		if (!resPtr)
		{
			RtlAddLogEntry(SECONDLEVEL, L"No resid entry found");
			goto done;
		}
		
		resPtr = find_entry_language(resPtr, resDir);
		if (!resPtr)
		{
			RtlAddLogEntry(SECONDLEVEL, L"No default language entry found");
			goto done;
		}

		/* Find resource data section */
		resData = (IMAGE_RESOURCE_DATA_ENTRY*)resPtr;
		for (i = 0; i < m_NumberOfSections; i++)
		{
			if (resData->OffsetToData >= m_dwRVA[i]	&& resData->OffsetToData < m_dwRVA[i] +	m_dwSRD[i])
				break;
		}

		//DbgPrint("\n >>> i: [%d], resData->Size: [%d]", i, resData->Size);

		if (i == m_NumberOfSections)
		{
			RtlAddLogEntry(SECONDLEVEL, L"Couldn't find resource data section");
			goto done;
		}

		/* Return resource data */
		if (resLen) *resLen = resData->Size;
		if (resOff) *resOff = resData->OffsetToData - m_dwRVA[i]
			+ m_dwPRD[i];
		ret = TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in find_pe_resource");
	}
done:
	if (resSection != NULL)
	{
		if (m_hHeapMem != NULL)
		{
			kfree(m_hHeapMem, resSection);
			resSection = NULL;
		}
	}
	return ret;
}

/***************************************************************************************************
*  Function Name  : find_entry_by_id
*  Description    : Find an entry by id in a resource directory
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
IMAGE_RESOURCE_DIRECTORY *find_entry_by_id(IMAGE_RESOURCE_DIRECTORY *dir,
	WORD id, const void *root)
{
	__try
	{
		if (dir == NULL)
		{
			return NULL;
		}

		IMAGE_RESOURCE_DIRECTORY_ENTRY *entry;
		int min, max, pos;

		entry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)(dir + 1);
		min = dir->NumberOfNamedEntries;

		max = min + dir->NumberOfIdEntries - 1;

		while (min <= max)
		{
			pos = (min + max) / 2;
			//DbgPrint("\n entry[pos].Id: [%d]  |  id: [%d]", entry[pos].Id, id);
			if (entry[pos].Id == id)
			{
				return (IMAGE_RESOURCE_DIRECTORY *)((const char *)root + entry[pos].OffsetToDirectory);
			}

			if (entry[pos].Id > id) max = pos - 1;
			else min = pos + 1;
		}
		return NULL;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in find_entry_by_id");
	}
	return NULL;
}

/***************************************************************************************************
*  Function Name  : find_entry_default
*  Description    : Find a default entry in a resource directory
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
IMAGE_RESOURCE_DIRECTORY *find_entry_default(const IMAGE_RESOURCE_DIRECTORY *dir, const void *root)
{
	__try
	{
		const IMAGE_RESOURCE_DIRECTORY_ENTRY *entry;
		entry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)(dir + 1);
		return (IMAGE_RESOURCE_DIRECTORY *)((const char *)root + entry->OffsetToDirectory);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in find_entry_default");
	}
	return NULL;
}

/***************************************************************************************************
*  Function Name  : RtlGetUserDefaultLCID
*  Description    : Returns the locale identifier for the user default locale.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
LCID RtlGetUserDefaultLCID(void)
{
	__try
	{
		LCID lcid;
		NtQueryDefaultLocale(TRUE, &lcid);
		return lcid;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlGetUserDefaultLCID");
		return 0;
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : RtlGetUserDefaultLangID
*  Description    : Returns the locale identifier for the user default locale.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
WORD RtlGetUserDefaultLangID(void)
{
	__try
	{ 
		return RTLLANGIDFROMLCID(RtlGetUserDefaultLCID());
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in RtlGetUserDefaultLangID");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : find_entry_language
*  Description    : Find a language entry in a resource directory
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
IMAGE_RESOURCE_DIRECTORY* find_entry_language(IMAGE_RESOURCE_DIRECTORY *dir,	const void *root)
{
	IMAGE_RESOURCE_DIRECTORY *ret = NULL;
	__try
	{
		WORD list[9];
		int i, pos = 0;

		/* cf. LdrFindResource_U */
		pos = push_language(list, pos, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
		//pos = push_language(list, pos, LANGIDFROMLCID(NtCurrentTeb()->CurrentLocale));
		pos = push_language(list, pos, RtlGetUserDefaultLangID());
		pos = push_language(list, pos, MAKELANGID(PRIMARYLANGID(RtlGetUserDefaultLangID()), SUBLANG_NEUTRAL));
		pos = push_language(list, pos, MAKELANGID(PRIMARYLANGID(RtlGetUserDefaultLangID()), SUBLANG_DEFAULT));
		//pos = push_language(list, pos, GetSystemDefaultLangID());
		//pos = push_language(list, pos, MAKELANGID(PRIMARYLANGID(GetSystemDefaultLangID()), SUBLANG_NEUTRAL));
		//pos = push_language(list, pos, MAKELANGID(PRIMARYLANGID(GetSystemDefaultLangID()), SUBLANG_DEFAULT));
		pos = push_language(list, pos, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));

		for (i = 0; i < pos; i++)
			if ((ret = find_entry_by_id(dir, list[i], root)))
				return ret;
		
		return find_entry_default(dir, root);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in find_entry_language");
	}
	return ret;
}

/***************************************************************************************************
*  Function Name  : push_language
*  Description    : push a language onto the list of languages to try
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
int push_language(WORD *list, int pos, WORD lang)
{
	__try
	{
		int i;
		for (i = 0; i < pos; i++) if (list[i] == lang) return pos;
		list[pos++] = lang;
		return pos;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in push_language");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : MatchSecondarySig
*  Description    : Function to match signature from second level of signature matching.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date			  :	14 Dec 2016
****************************************************************************************************/
bool MatchSecondarySig(LPBYTE lpFileBuffer, DWORD dwIndex, DWORD &dwVirusID)
{
	bool bReturn = false;
	HANDLE	hInputFileHandle = INVALID_HANDLE_VALUE;
	__try
	{
		//Sanity check
		if (!lpFileBuffer)
		{
			return false;
		}

		DWORD  dwDBID = GetDBDwordName(dwIndex);

		DECLARE_UNICODE_STRING_SIZE(uFileName, 10);

		WCHAR szFileName[0x50] = { 0 };
		if (dwDBID >= 1 && dwDBID <= 9)
		{
			RtlIntegerToUnicodeString((unsigned long)dwDBID, 0, &uFileName);
			wcscpy(szFileName, L"00");
			wcscat(szFileName, uFileName.Buffer);
		}
		else  if (dwDBID >= 10 && dwDBID <= 99)
		{
			RtlIntegerToUnicodeString((unsigned long)dwDBID, 0, &uFileName);
			wcscpy(szFileName, L"0");
			wcscat(szFileName, uFileName.Buffer);
		}
		else  if (dwDBID >= 100 && dwDBID <= 999)
		{
			RtlIntegerToUnicodeString((unsigned long)dwDBID, 0, &uFileName);
			wcscat(szFileName, uFileName.Buffer);
		}
		else
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Unhandled DBID %d", dwDBID);
			return false;
		}

		//---------------------------------------------------------------
		//Set here current directory to use in overall project
		WCHAR lpCurrentDirectory[MAX_PATH] = { 0 };
		if (!RtlGetCurrentDir(lpCurrentDirectory, MAX_PATH))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### RtlGetCurrentDir function failed..");
			return false;
		}

		WCHAR szOSDrive[0x02] = { 0 };
		szOSDrive[0x00] = lpCurrentDirectory[0x00];
		szOSDrive[0x01] = L'\0';
		wcscpy(szAppPath, szOSDrive);
		wcscat(szAppPath, L":\\Program Files\\Vibranium");

		WCHAR szDBFilePath[50] = { 0 };
		wcscpy(szDBFilePath, szAppPath);
		wcscat(szDBFilePath, L"\\VBDB\\");
		wcscat(szDBFilePath, WRDWIZAVPEDBNAME);
		wcscat(szDBFilePath, szFileName);
		wcscat(szDBFilePath, L".DB");

		if (!FileExists(szDBFilePath))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### File Does not exist: %s", szDBFilePath, 0, true, ZEROLEVEL);
			return false;
		}

		DWORD dwMajorversion = 0x00;
		DWORD dwVersionLength = 0x00;
		if (!ISValidDBFile(szDBFilePath, dwMajorversion, dwVersionLength))
		{
			RtlAddLogEntry(ZEROLEVEL, L"### Invalid DB file: [%s]", szDBFilePath);
			return false;
		}

		if (!NtFileOpenFile(&hInputFileHandle, szDBFilePath, FALSE, FALSE, TRUE))
		{
			RtlAddLogEntry(ZEROLEVEL, L"### NtFileOpenFile failed, DB file: [%s]", szDBFilePath);
			return false;
		}
		
		LONGLONG lFileSize = 0x00;
		if (!NtFileGetFileSize(hInputFileHandle, &lFileSize))
		{
			RtlAddLogEntry(ZEROLEVEL, L"### NtFileGetFileSize failed, DB file: [%s]", szDBFilePath);
			bReturn = false;
			goto CLEANUP;
		}

		/* Get file size here */
		DWORD dwFileSize = (DWORD)lFileSize;

		/* If file size if 0 return */
		if (dwFileSize == 0x00)
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Get the actual offset of file.
		DWORD dwBufferOffset = MAX_SIG_SIZE + 0x02 + dwVersionLength + 0x02 + sizeof(ULONG64);//Sig Length + PIPE + DBVersion + PIPE + CRC.
		DWORD dwActualFileOffset = GetActualFileOffsetFromIndex(dwIndex);

		dwBufferOffset += (dwActualFileOffset * 0x10);

		/* If calculated offset is beyond the file size then return*/
		if (dwBufferOffset > dwFileSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (!NtFileSeekFile(hInputFileHandle, dwBufferOffset))
		{
			RtlAddLogEntry(ZEROLEVEL, L"### NtFileSeekFile failed, DB file: [%s]", szDBFilePath);
			bReturn = false;
			goto CLEANUP;
		}

		//Read Remaining 12 bytes
		BYTE byRemainingBytes[0x10] = { 0 };
		DWORD dwBytesRead = 0;

		if (!NtFileReadFile(hInputFileHandle, &byRemainingBytes, sizeof(byRemainingBytes), &dwBytesRead))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != sizeof(byRemainingBytes))
		{
			bReturn = false;
			goto CLEANUP;
		}

		bReturn = MatchElementUsingIndex(lpFileBuffer, byRemainingBytes, dwIndex, dwVirusID);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in MatchSecondarySig, %s", dwIndex);
	}
CLEANUP:
	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : MatchElementUsingIndex
*  Description    : Function to Match the signature buffer and file MD5 buffer
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	11 Feb 2016
****************************************************************************************************/
bool MatchElementUsingIndex(BYTE *byMD5Buffer, BYTE *byDBSigBuffer, DWORD dwIndex, DWORD & dwVirusID)
{
	bool bFound = false;
	__try
	{
		//Sanity check for buffer
		if (byMD5Buffer == NULL || byDBSigBuffer == NULL)
		{
			return bFound;
		}

		//Compare 16 bytes buffer.
		if (memcmp(byMD5Buffer, byDBSigBuffer, 0x10) == 0)
		{
			bFound = true;
			dwVirusID = dwIndex;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in MatchElementUsingIndex, %d", dwIndex);
	}
	return bFound;
}

/***************************************************************************************************
*  Function Name  : GetActualFileOffsetFromIndex
*  Description    : The index is passed as parameter is calculated when all DB files are loaded,
now we can get the actual signature index per DB.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	15 Dec 2016
****************************************************************************************************/
DWORD GetActualFileOffsetFromIndex(DWORD dwIndex)
{
	DWORD dwRet = 0x00;
	__try
	{
		DWORD dwCount = 0x00;
		DWORD dwListID = 0x00;
		//Check here the Index with count mentained in list.
		for (; dwListID < 0x200; dwListID++)
		{
			dwCount += GetSigCountAsPerListID(dwListID);
			if (dwIndex < dwCount)
			{
				dwRet = dwListID;
				break;
			}
		}

		//if the Index is from first DB the its actual offset.
		if (dwListID == 0)
		{
			dwRet = dwIndex;
		}
		else
		{
			dwRet = dwIndex - (dwCount - GetSigCountAsPerListID(dwListID));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in GetActualFileOffsetFromIndex, Index: %d", dwIndex);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetSigCountAsPerListID
*  Description    : Function which returns signature count as per list ID.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	15 Dec 2016
****************************************************************************************************/
DWORD GetSigCountAsPerListID(DWORD dwListID)
{
	DWORD dwRet = 0x00;
	__try
	{
		if (dwListID > 0x200)
			return dwRet;

		dwRet = m_dwArrPESigCountPerDB[dwListID];
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in GetSigCountAsPerListID, PE_FILE ListID: %d", dwListID);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetDBDwordName
*  Description    : Function to get DB ID using Threat Index ID
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	14 Dec 2016
****************************************************************************************************/
DWORD GetDBDwordName(DWORD dwIndex)
{
	DWORD dwRet = 0x00;
	__try
	{
		DWORD dwCount = 0x00;
		//Check here the Index with count mentained in list.
		for (DWORD dwListID = 0x00; dwListID < 0x200; dwListID++)
		{
			if (dwIndex < dwCount)
			{
				dwRet = dwListID;
				break;
			}

			DWORD dwSigCountPerDB = GetSigCountAsPerListID(dwListID);
			if (dwSigCountPerDB == 0x00)
				break;

			dwCount += dwSigCountPerDB;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in GetDBDwordName, Index: [%d]", dwIndex);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : SetSigCountAsPerListID
*  Description    : Function to set signature count as per list ID.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	15 Dec 2016
****************************************************************************************************/
void SetSigCountAsPerListID(DWORD dwListID, DWORD dwCount)
{
	DWORD dwRet = 0x00;
	__try
	{
		if (dwListID > 0x200)
			return;

		m_dwArrPESigCountPerDB[dwListID] = dwCount;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in SetSigCountAsPerListID PE_FILE ListID: %d", dwListID);
	}
}

/**********************************************************************************************************
*  Function Name  :	Encrypt_File
*  Description    :	Encrypt file and keep into quarantine folder as temp file.
*  Author Name    : Ram Shelke
*  Date           : 24 Oct 2017
*  SR_NO		  :
**********************************************************************************************************/
DWORD Encrypt_File(TCHAR *szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus)
{

	DWORD	dwRet = 0x00;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00;
	TCHAR	szExt[16] = { 0 };
	DWORD	dwLen = 0x00;
	LPBYTE	lpFileData = NULL;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE;

	__try
	{
		//Sanity check
		if (!szFilePath || !szQurFolderPath || !lpszFileHash || !lpszTargetFilePath)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//Check is valid paths
		if (!FileExists(szFilePath))
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		if (!NtFileOpenFile(&hFile, szFilePath, FALSE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in opening existing file %s", szFilePath);
			dwRet = 0x02;
			goto Cleanup;

		}

		LONGLONG lFileSize = 0x00;
		if (!NtFileGetFileSize(hFile, &lFileSize))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in GetFileSize of file %s", szFilePath);
			dwRet = 0x03;
			goto Cleanup;
		}

		dwFileSize = (DWORD)lFileSize;
		if (lpFileData)
		{
			kfree(m_hHeapMem, lpFileData);
			lpFileData = NULL;
		}

		lpFileData = (LPBYTE)kmalloc(m_hHeapMem, dwFileSize);
		if (!lpFileData)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in allocating memory");
			dwRet = 0x04;
			goto Cleanup;
		}

		memset(lpFileData, 0x00, dwFileSize);

		if (!NtFileSeekFile(hFile, 0x00))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileSeekFile");
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!NtFileReadFile(hFile, lpFileData, dwFileSize, &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileReadFile");
			dwRet = 0x04;
			goto Cleanup;
		}

		if (dwFileSize != dwBytesRead)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileReadFile file %s", szFilePath);
			dwRet = 0x04;
			goto Cleanup;
		}

		BYTE byKey[WRDWIZ_KEY_SIZE] = { 0 };
		if (!CreateRandomKeyFromFile(hFile, dwFileSize, byKey))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in CreateRandomKeyFromFile, File: [%s]", szFilePath);
			dwRet = 0x08;
			goto Cleanup;
		}

		NtFileCloseFile(hFile);
		hFile = NULL;

		if (DecryptData((LPBYTE)lpFileData, dwBytesRead, byKey))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in DecryptData, File: [%s]", szFilePath);
			dwRet = 0x05;
			goto Cleanup;
		}

		WCHAR szTargetFilePath[MAX_FILEPATH_LENGTH] = { 0 };
		wcscpy(szTargetFilePath, szQurFolderPath);
		wcscat(szTargetFilePath, L"\\");
		wcscat(szTargetFilePath, lpszFileHash);
		wcscat(szTargetFilePath, L".tmp");

		//copy here into output parameter
		wcscpy(lpszTargetFilePath, szTargetFilePath);

		if (!NtFileOpenFile(&hFileEnc, lpszTargetFilePath, TRUE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in creating file %s", lpszTargetFilePath);
			dwRet = 0x06;
			goto Cleanup;
		}

		if (hFileEnc == INVALID_HANDLE_VALUE)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in creating file %s", lpszTargetFilePath);
			dwRet = 0x06;
			goto Cleanup;
		}

		dwBytesRead = 0x00;

		if (!NtFileSeekFile(hFileEnc, 0x00))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileSeekFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x06;
			goto Cleanup;
		}

		if (!NtFileWriteFile(hFileEnc, (LPVOID)WRDWIZ_SIG, WRDWIZ_SIG_SIZE, &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x06;
			goto Cleanup;
		}

		if (dwBytesRead != WRDWIZ_SIG_SIZE)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x9;
			goto Cleanup;
		}

		if (!NtFileSeekFile(hFileEnc, 0x00 + WRDWIZ_SIG_SIZE))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileSeekFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x06;
			goto Cleanup;
		}

		if (!NtFileWriteFile(hFileEnc, byKey, WRDWIZ_KEY_SIZE, &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x06;
			goto Cleanup;
		}

		if (dwBytesRead != WRDWIZ_KEY_SIZE)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x9;
			goto Cleanup;
		}

		if (!NtFileSeekFile(hFileEnc, 0x0 + WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileSeekFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x06;
			goto Cleanup;
		}

		if (!NtFileWriteFile(hFileEnc, lpFileData, dwFileSize, &dwBytesRead))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x06;
			goto Cleanup;
		}

		if (dwFileSize != dwBytesRead)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileWriteFile file: [%s]", lpszTargetFilePath);
			dwRet = 0x07;
			goto Cleanup;
		}

		NtFileCloseFile(hFile);
		hFile = NULL;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in Encrypt_File FilePath: [%s]", szFilePath);
	}
Cleanup:

	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = NULL;
	}

	if (lpFileData != NULL)
	{
		kfree(m_hHeapMem, lpFileData);
		lpFileData = NULL;
	}
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	CreateRandomKeyFromFile
*  Description    :	Create a random key to insert into encrypted file.
*  Author Name    : Ram Shelke
*  Date           : 24 Oct 2017
*  SR_NO		  :
**********************************************************************************************************/
bool CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize, LPBYTE lpKey)
{
	__try
	{

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		bool			bReturn = false;
		int				iTmp = 0x00;
		int				iIndex = 0x00, jIndex = 0x00;
		int				iRandValue = 0x00, iReadPos = 0x00;
		unsigned char	szChar = 0x0;

		iTmp = dwFileSize / WRDWIZ_KEY_SIZE;

		for (iIndex = 0x00, jIndex = 0x00; iIndex < iTmp; iIndex++, jIndex++)
		{
			if (jIndex >= WRDWIZ_KEY_SIZE)
			{
				break;
			}

			iRandValue = 0x10;
			iRandValue = iRandValue % WRDWIZ_KEY_SIZE;

			iReadPos = (iIndex *  WRDWIZ_KEY_SIZE) + iRandValue;

			DWORD dwBytesRead = 0x0;

			if (!NtFileSeekFile(hFile, iReadPos))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### failed NtFileSeekFile");
				return false;
			}

			if (!NtFileReadFile(hFile, &szChar, sizeof(BYTE), &dwBytesRead))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### failed NtFileReadFile");
				return false;
			}

			if (szChar == 0x00)
			{
				szChar = iRandValue;
			}
			lpKey[jIndex] = szChar;

			if (iIndex == (iTmp - 0x01) && jIndex < WRDWIZ_KEY_SIZE)
			{
				iIndex = 0x00;
			}
		}
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in CreateRandomKeyFromFile");
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	DecryptData
*  Description    :	Encrypt/Decrypt data.
*  Author Name    : Ram Shelke
*  Date           : 24 Oct 2017
*  SR_NO		  :
**********************************************************************************************************/
DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize, LPBYTE pbyEncDecKey)
{
	__try
	{
		if (!lpBuffer)
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00 || pbyEncDecKey == NULL)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			//if(lpBuffer[iIndex] != 0)
			{
				lpBuffer[iIndex] ^= pbyEncDecKey[jIndex++];
				if (jIndex == WRDWIZ_KEY_SIZE)
				{
					jIndex = 0x00;
				}
				if (iIndex >= dwSize)
				{
					break;
				}
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in DecryptData");
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	BackUpBeforeQuarantineOrRepair
*  Description    :	Taking a backup before taking any action on detected files.
*  Author Name    : Ram Shelke
*  Date           : 23 OCT 2017
*  SR_NO		  :
**********************************************************************************************************/
bool BackUpBeforeQuarantineOrRepair(LPTSTR szOriginalThreatPath, LPTSTR lpszBackupFilePath)
{
	__try
	{
		if (!szOriginalThreatPath || !lpszBackupFilePath)
			return false;

		DWORD dwStatus = 0;
		WCHAR szQuarantineFolderpath[MAX_PATH] = { 0 };
		WCHAR szOrigThreatPath[MAX_FILEPATH_LENGTH] = { 0 };
		UINT RetUnique = 0;

		wcscpy(szQuarantineFolderpath, szAppPath);
		wcscat(szQuarantineFolderpath, L"\\QUARANTINE");
		
		wcscpy(szOrigThreatPath, szOriginalThreatPath);
		if (!FolderExists(szQuarantineFolderpath))
		{
			if (!NtFileCreateDirectory(szQuarantineFolderpath))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### NtFileCreateDirectory failed, Path: [%s]", szQuarantineFolderpath);
				return false;
			}
		}

		RtlAddLogEntry(SECONDLEVEL, L">>> Before FileExists, [%s]", szOrigThreatPath);
		if (!FileExists(szOrigThreatPath))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### BackUpBeforeQuarantineOrRepair [%s] File not exists", szOrigThreatPath);
			return false;
		}

		//Get here file hash
		WCHAR szFileHash[130] = { 0 };
		if (GetFileHash(szOrigThreatPath, szFileHash) != 0x00)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed GetFileHash, FilePath: [%s]", szOrigThreatPath);
			return false;
		}

		WCHAR szTargetFilePath[MAX_FILEPATH_LENGTH] = { 0 };
		wcscpy(szTargetFilePath, szQuarantineFolderpath);
		wcscat(szTargetFilePath, L"\\");
		wcscat(szTargetFilePath, szFileHash);
		wcscat(szTargetFilePath, L".tmp");
		if (FileExists(szTargetFilePath))
		{
			wcscpy(lpszBackupFilePath, szTargetFilePath);
			return true;
		}

		if (Encrypt_File(szOrigThreatPath, szQuarantineFolderpath, lpszBackupFilePath, szFileHash, dwStatus) != 0x00)
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Failed Encrypt_File, FilePath: [%s]", szOrigThreatPath);
			return false;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in BackUpBeforeQuarantineOrRepair, FilePath: %s", szOriginalThreatPath);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : LoadExcludeDB
*  Description    : Function to load exclude DB.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date			  : 13 nov 2017
****************************************************************************************************/
bool LoadExcludeDB()
{
	bool bReturn = false;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	__try
	{
		WCHAR szFilePath[MAX_PATH] = { 0 };
		wcscpy(szFilePath, szAppPath);
		wcscat(szFilePath, L"\\");
		wcscat(szFilePath, WRDWIZEXCLUDECLONEDB);
		
		//Check is valid paths
		if (!FileExists(szFilePath))
		{
			return bReturn;
		}

		if (!NtFileOpenFile(&hFile, szFilePath, FALSE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in opening existing file %s", szFilePath);
			goto Cleanup;

		}

		if (!NtFileSeekFile(hFile, 0x00))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileSeekFile in LoadExcludeDB");
			goto Cleanup;
		}

		while (true)
		{
			WRDWIZEXCLUDESCAN stExCludeEntry = { 0 };

			DWORD dwBytesRead = 0x00;
			if (!NtFileReadFile(hFile, &stExCludeEntry, sizeof(stExCludeEntry), &dwBytesRead))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileReadFile in LoadExcludeDB");
				break;
			}

			if (sizeof(stExCludeEntry) != dwBytesRead)
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileReadFile in LoadExcludeDB file %s", szFilePath);
				break;
			}

			//DbgPrint("\n Exclude Entry: [%s]", stExCludeEntry.szFilePath);

			if (!InsertItem(stExCludeEntry.szFilePath, sizeof(stExCludeEntry.szFilePath), stExCludeEntry.byIsSubFolder))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Failed to InsertItem in LoadExcludeDB");
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in LoadExcludeDB");
		return false;
	}
Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = NULL;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : LoadExcludeExtDB
*  Description    : Function to load exclude Extension DB.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date			  : 10 OCT 2018
****************************************************************************************************/
bool LoadExcludeExtDB()
{
	bool bReturn = false;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	__try
	{
		WCHAR szFilePath[MAX_PATH] = { 0 };
		wcscpy(szFilePath, szAppPath);
		wcscat(szFilePath, L"\\");
		wcscat(szFilePath, WRDWIZEXTEXCLUDECLONEDB);

		//Check is valid paths
		if (!FileExists(szFilePath))
		{
			return bReturn;
		}

		if (!NtFileOpenFile(&hFile, szFilePath, FALSE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in opening existing file %s", szFilePath);
			goto Cleanup;

		}

		if (!NtFileSeekFile(hFile, 0x00))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileSeekFile in LoadExcludeDB");
			goto Cleanup;
		}

		while (true)
		{
			STRUCTEXCLUDEFILEEXTLIST stExCludeExtEntry = { 0 };

			DWORD dwBytesRead = 0x00;
			if (!NtFileReadFile(hFile, &stExCludeExtEntry, sizeof(stExCludeExtEntry), &dwBytesRead))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileReadFile in LoadExcludeDB");
				break;
			}

			if (sizeof(stExCludeExtEntry) != dwBytesRead)
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileReadFile in LoadExcludeDB file %s", szFilePath);
				break;
			}

			//DbgPrint("\n Exclude Ext: [%s]", stExCludeExtEntry.szFileExt);

			if (!InsertFileExtItem(stExCludeExtEntry.szFileExt, sizeof(stExCludeExtEntry.szFileExt)))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Failed to InsertItem in LoadExcludeDB");
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in LoadExcludeDB");
		return false;
	}
Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = NULL;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ISExcludedExt
*  Description    : Function to check is file extension is excluded.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 13 nov 2017
****************************************************************************************************/
bool ISExcludedExt(LPTSTR szFileName)
{
	bool bReturn = false;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	__try
	{
		if (!szFileName)
			return bReturn;

		DECLARE_UNICODE_STRING_SIZE(ulFileName, MAX_FILEPATH_LENGTH);
		ulFileName.Length = (USHORT)wcslen(szFileName) * sizeof(WCHAR);
		ulFileName.MaximumLength = MAX_FILEPATH_LENGTH;
		wcscpy(ulFileName.Buffer, szFileName);

		ANSI_STRING asFileName;
		RtlUnicodeStringToAnsiString(&asFileName, &ulFileName, TRUE);

		char szFileName[MAX_FILEPATH_LENGTH] = { 0 };
		strcpy(szFileName, asFileName.Buffer);

		int iLen = (int)strlen(szFileName);
		AnsiToLower(szFileName, iLen);

		char * pch;
		pch = strrchr(szFileName, '.');
		if (!pch)
			return bReturn;

		if (pch)
		{
			pch++;
		}

		char szFileExt[0x32] = { 0 };
		strcpy(szFileExt, pch);

		DECLARE_UNICODE_STRING_SIZE(ulFileExt, MAX_FILEPATH_LENGTH);
		FillUnicodeStringWithAnsi(&ulFileExt, szFileExt);
		
		int iExtLen = (int)strlen(szFileExt);

		//check here is the file path exluded
		bReturn = Find(szFileExt, iExtLen);
	
		RtlFreeAnsiString(&asFileName);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ISExcludedExt, FileName: %s", szFileName);
		return false;
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : ISScanByFileName
*  Description    : Function to check is file included in scan
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 13 nov 2017
****************************************************************************************************/
bool ISScanByFileName(LPTSTR szFileName)
{
	bool bReturn = false;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	__try
	{
		if (!szFileName)
			return bReturn;

		if (wcslen(szFileName) >= 0x96 * 2)
		{
			return bReturn;
		}

		DECLARE_UNICODE_STRING_SIZE(ulFileName, 0x96 * 2);
		ulFileName.Length = (USHORT)wcslen(szFileName) * sizeof(WCHAR);
		ulFileName.MaximumLength = 0x96 * 2;
		wcscpy(ulFileName.Buffer, szFileName);

		ANSI_STRING asFileName;
		RtlUnicodeStringToAnsiString(&asFileName, &ulFileName, TRUE);

		char szFileName[0x96 * 2] = { 0 };
		strcpy(szFileName, asFileName.Buffer);

		int iLen = (int)strlen(szFileName);
		AnsiToLower(szFileName, iLen);

		//check here is the file path exluded
		bReturn = FindScanByName(szFileName, iLen);

		RtlFreeAnsiString(&asFileName);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ISScanByFileName, FileName: %s", szFileName);
		return false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ISFileExcluded
*  Description    : Function to check is file path excluded.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date			  : 13 nov 2017
****************************************************************************************************/
bool ISFileExcluded(LPTSTR szFilePath, bool &bISSubFolderExcluded)
{
	//DbgPrint("\n szFilePath: [%S]", szFilePath);
	
	bool bReturn = false;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	__try
	{
		if (!szFilePath)
			return bReturn;

		DECLARE_UNICODE_STRING_SIZE(ulFilePath, MAX_FILEPATH_LENGTH);
		ulFilePath.Length = (USHORT)wcslen(szFilePath) * sizeof(WCHAR);
		ulFilePath.MaximumLength = MAX_FILEPATH_LENGTH;
		wcscpy(ulFilePath.Buffer, szFilePath);

		ANSI_STRING asFilePath;
		RtlUnicodeStringToAnsiString(&asFilePath, &ulFilePath, TRUE);

		char szFilePath[MAX_FILEPATH_LENGTH] = { 0 };
		strcpy(szFilePath, asFilePath.Buffer);

		int iLen = (int)strlen(szFilePath);
		AnsiToLower(szFilePath, iLen);

		//check here is the file path exluded
		bReturn = Find(szFilePath, iLen, bISSubFolderExcluded);

		RtlFreeAnsiString(&asFilePath);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in ISFileExcluded, FilePath: %s", szFilePath);
		return false;
	}
	return bReturn;
}

void SetProductID(DWORD dwProdID)
{
	__try
	{
		m_dwPEScanProdID = dwProdID;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in SetProductID");
	}
}

/***************************************************************************************************
*  Function Name  : LoadExcludeExtDB
*  Description    : Function to load exclude Extension DB.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 10 OCT 2018
****************************************************************************************************/
bool LoadScanByNameDB()
{
	//DbgPrint("\n >>> LoadScanByNameDB");

	bool bReturn = false;
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	__try
	{
		WCHAR szFilePath[MAX_PATH] = { 0 };
		wcscpy(szFilePath, szAppPath);
		wcscat(szFilePath, L"\\");
		wcscat(szFilePath, WRDWIZSCANNAMECLONEDB);

		//Check is valid paths
		if (!FileExists(szFilePath))
		{
			return bReturn;
		}

		if (!NtFileOpenFile(&hFile, szFilePath, FALSE, FALSE, true))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in opening existing file %s", szFilePath);
			goto Cleanup;

		}

		if (!NtFileSeekFile(hFile, 0x00))
		{
			RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileSeekFile in LoadExcludeDB");
			goto Cleanup;
		}

		while (true)
		{
			WRDWIZSCANBYNAME stExCludeExtEntry = { 0 };

			DWORD dwBytesRead = 0x00;
			if (!NtFileReadFile(hFile, &stExCludeExtEntry, sizeof(stExCludeExtEntry), &dwBytesRead))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileReadFile in LoadExcludeDB");
				break;
			}

			if (sizeof(stExCludeExtEntry) != dwBytesRead)
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Error in NtFileReadFile in LoadExcludeDB file %s", szFilePath);
				break;
			}

			//DbgPrint("\n Scan by Name: [%s]", stExCludeExtEntry.szFileName);

			if (!InsertScanByNameItem(stExCludeExtEntry.szFileName, sizeof(stExCludeExtEntry.szFileName)))
			{
				RtlAddLogEntry(SECONDLEVEL, L"### Failed to InsertItem in LoadScanByNameDB");
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		RtlAddLogEntry(SECONDLEVEL, L"### Exception in LoadScanByNameDB");
		return false;
	}
Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		NtFileCloseFile(hFile);
		hFile = NULL;
	}
	return true;
}