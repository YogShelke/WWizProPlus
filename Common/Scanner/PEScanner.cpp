#include "StdAfx.h"
#include "PEScanner.h"

/***************************************************************************************************
*  Function Name  : CPEScanner
*  Description    : Cont'r
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
CPEScanner::CPEScanner(void):
		  m_pPETreeManager(NULL)
		, m_iBufferOffset(0)
		, m_NumberOfSections(0)
		, m_IS64Bit(false)
		, m_dwOffset2NewEXEHeader(0)
		, m_bPESignatureLoaded(false)
		, m_hHeuristicScan(NULL)
		, m_ScanFileForHeuristic(NULL)
{
	m_hFileHandle = INVALID_HANDLE_VALUE;
	memset(&m_stImageOptionalHeader32, 0, sizeof(m_stImageOptionalHeader32));
	memset(&m_stImageOptionalHeader64, 0, sizeof(m_stImageOptionalHeader64));
	memset(&m_szFilePath, 0, sizeof(m_szFilePath));
	memset(&m_stSectionHeader,0,sizeof(m_stSectionHeader));

	//Load heuristic scan DLL
	LoadHeuristicScanDLL();
}


/***************************************************************************************************
*  Function Name  : ~CPEScanner
*  Description    : Dest'r
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
CPEScanner::~CPEScanner(void)
{
	if (m_hHeuristicScan)
	{
		FreeLibrary(m_hHeuristicScan);
		m_hHeuristicScan = NULL;
	}

	m_ScanFileForHeuristic = NULL;

	if (!m_MD5Scanner.UnLoadMD5Data())
	{
		AddLogEntry(L"### Failed to UnloadMD5Data in CPEScanner::~CPEScanner", 0, 0, true, SECONDLEVEL);
	}

	if(m_pPETreeManager != NULL)
	{
		delete m_pPETreeManager;
		m_pPETreeManager = NULL;
	}
}


/***************************************************************************************************
*  Function Name  : LoadPESignatures
*  Description    : Function to load PE Signatures
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::LoadPESignatures(LPCTSTR pszFilePath, DWORD &dwSigCount)
{
	bool bReturn = false;
	
	__try
	{
		if (!LoadPESignaturesSEH(pszFilePath))
		{
			AddLogEntry(L"### Failed to load DB in CPEScanner::LoadPESignatures: %s", pszFilePath, 0, true, SECONDLEVEL);
			return bReturn;
		}

		if (!m_MD5Scanner.LoadMD5Data(FILE_PE, dwSigCount))
		{
			AddLogEntry(L"### Failed to load LoadMD5Data in CPEScanner::LoadPESignatures", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}
	
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CPEScanner::LoadPESignatures", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : LoadPESignaturesSEH
*  Description    : Function to load PE Signatures
*  Author Name    : Ramkrushna Shelke, Sanjay Khapre
*  SR_NO		  :
*  Date			  : 18 Sep 2013, 
					updated with minor changes on 19 Mar 2016
****************************************************************************************************/
bool CPEScanner::LoadPESignaturesSEH(LPCTSTR pszFilePath)
{
	if (!pszFilePath)
		return false;

	if (!PathFileExists(pszFilePath))
	{
		return false;
	}

	//Need to reset Signature loaded flag 
	m_bPESignatureLoaded = false;

	//De-allocate the existing memory
	if (m_pPETreeManager != NULL)
	{
		delete m_pPETreeManager;
		m_pPETreeManager = NULL;
	}

	m_pPETreeManager = new CISpyTreeManager();

	TCHAR szTempDBFilePath[MAX_PATH] = { 0 };
	m_csTmpDBFileName = "";

	AddLogEntry(L">>> Before DecryptDBFileData", 0, 0, true, ZEROLEVEL);

	//Decrypt DB files

	DWORD dwVersionLength = 0x00;
	DWORD dwMajorVersion = 0x00;
	if (!m_objWrdwizEncDecManager.DecryptDBFileData(pszFilePath, szTempDBFilePath, dwVersionLength, dwMajorVersion))
	{
		AddLogEntry(L"### Failed to DecryptDBFileData in CPEScanner::LoadPESignatures", 0, 0, true, SECONDLEVEL);
		return false;
	}

	AddLogEntry(L">>> Before SetPESignatureFilePath", 0, 0, true, ZEROLEVEL);


	//m_csTmpDBFileName = szTempDBFilePath;
	if (!m_pPETreeManager->SetPESignatureFilePath(szTempDBFilePath))
	{
		AddLogEntry(L"### Failed to SetPESignatureFilePath in CPEScanner::LoadPESignatures", 0, 0, true, SECONDLEVEL);
		return false;
	}

	AddLogEntry(L">>> Before LoadContentFromFile", 0, 0, true, ZEROLEVEL);

	if (!m_objWrdwizEncDecManager.LoadContentFromFile(szTempDBFilePath))
	{
		AddLogEntry(L"### Failed to LoadContentFromFile in CPEScanner::LoadPESignatures", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (PathFileExists(szTempDBFilePath))
	{
		AddLogEntry(L">>> Before DeleteFile", 0, 0, true, ZEROLEVEL);

		SetFileAttributes(szTempDBFilePath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(szTempDBFilePath))
		{
			AddLogEntry(L"### Failed to DeleteFile in CPEScanner::LoadPESignatures", 0, 0, true, SECONDLEVEL);
		}
	}

	CString csFirstEntry = NULL, csSecondEntry = NULL;

	const ContactList& contacts = m_objWrdwizEncDecManager.m_objEncDec.GetContacts();

	AddLogEntry(L">>> Before CopyDataByRef", 0, 0, true, ZEROLEVEL);

	if (!m_pPETreeManager->CopyDataByRef(&m_objWrdwizEncDecManager.m_objEncDec))
	{
		AddLogEntry(L"### Failed to CopyDataByRef in CPEScanner::LoadPESignatures", 0, 0, true, SECONDLEVEL);
		return false;
	}

	OutputDebugString(L">>> After CopyDataByRef");

	// iterate over all contacts & add them to the list
	POSITION pos = contacts.GetHeadPosition();
	while (pos != NULL)
	{
		const CIspyList contact = contacts.GetNext(pos);

		csFirstEntry = contact.GetFirstEntry();
		csSecondEntry = contact.GetSecondEntry();

		char szSigOnly[1024] = { 0 };
		wcstombs(szSigOnly, csSecondEntry.GetBuffer(), sizeof(szSigOnly) + 1);

		char seps[] = "*";
		char *token = NULL;
		char* context = NULL;

		token = strtok_s(szSigOnly, seps, &context);

		while (token != NULL)
		{
			if (strlen(token) > 0)
			{
				trim(token, static_cast<int>(strlen(token)));
				m_pPETreeManager->AddString(token);
			}
			token = strtok_s(NULL, seps, &context);
		}
	}
	m_bPESignatureLoaded = true;
	//return true;
	return m_bPESignatureLoaded;
}


/***************************************************************************************************
*  Function Name  : UnLoadPESignatures
*  Description    : Function to unload PE Signatures
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::UnLoadHtmlSignatures()
{
	if (m_pPETreeManager != NULL)
	{
		delete m_pPETreeManager;
		m_pPETreeManager = NULL;
	}
	return false;
}


/***************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to start scanning PE files, where the scanning will start only if the PE signatures are loaded
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, bool bIsHeuScan)
{
	bool bReturn = false;
	
	__try
	{
		if (!m_bPESignatureLoaded)
		{
			DWORD dwSigCount = 0x00;
			m_bPESignatureLoaded = LoadPESignatures(pszFilePath, dwSigCount);
		}

		if (m_bPESignatureLoaded)
			bReturn = ScanFileSEH(pszFilePath, pszVirusName, dwSpyID, bRescan, bIsHeuScan);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### GetFileVersionInfoSize Failed, Corrupt file: %s", m_szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : ScanFileSEH
*  Description    : Function to start scanning PE files and lead to common scanner class
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, bool bIsHeuScan)
{
	try
	{
		_tcscpy(m_szFilePath, pszFilePath);

		if (!ReadBufferFromDiffSections())
		{
			return false;
		}

		if (!m_szFileBuffer)
		{
			return false;
		}

		//Check here files already indexed?
		UINT64 uiCRCValue = 0;
		if (m_objWWizIndexer.ISWhiteListed(m_szFileBuffer, m_iBufferOffset, uiCRCValue))
		{
			return false;
		}

		//Check if Threat already detected, If Yes then no need to scan again.
		//Return the last status.
		THREATINFO pThreatInfo = { 0 };
		if (ISAlreadyDetected(uiCRCValue, &pThreatInfo, bIsHeuScan))
		{
			_tcscpy(pszVirusName, pThreatInfo.szThreatName);
			dwSpyID = pThreatInfo.dwSpyID;
			return true;
		}
		
		if (bIsHeuScan)
		{
			//Heuristic Scan
			//Added by	: Vilas
			//Date		: 14-Mar-2015
			//Modified on 07 / 08 / 2015 by Vilas
			//to support return value for PE file type and 64 bit file by reference
			bool bISPEFile = false;
			bool bISX64PEFile = false;
			if (ScanFileHeur(pszFilePath, pszVirusName, dwSpyID, bISPEFile, bISX64PEFile))
			{
				THREATINFO stThreatInfo = { 0 };
				_tcscpy_s(stThreatInfo.szThreatName, pszVirusName);
				stThreatInfo.dwSpyID = dwSpyID;
				stThreatInfo.bIsHeuDetection = true;
				m_mapDetectedTheatInfo.insert(make_pair(uiCRCValue, stThreatInfo));
				return true;
			}
		}

		//add here to scan buffer
		bool bRet = false;
		if (!m_IS64Bit)
		{
			bRet = m_MD5Scanner.ScanFile(m_szFileBuffer, m_iBufferOffset, dwSpyID, bRescan);
		}

		char szVirusName[MAX_PATH] = { 0 };
		CString csVirusName = L"";
		if ((dwSpyID > 0) && (bRet))
		{
			csVirusName.Format(L"W32.Mal-%lu", dwSpyID);
			_tcscpy_s(pszVirusName, MAX_PATH, csVirusName);
			dwSpyID = 0;
			return true;
		}

		//Virus name is required during rescan.
		csVirusName = pszVirusName;

		if (bRescan)
		{
			wcstombs(szVirusName, csVirusName.GetBuffer(), sizeof(szVirusName) + 1);
		}

		if (!m_IS64Bit && ScanFileBuffer(m_szFileBuffer, m_iBufferOffset, szVirusName, dwSpyID, bRescan))
		{
			//convert multibyte string to unicode
			size_t convertedChars;
			mbstowcs_s(&convertedChars, pszVirusName, strlen(szVirusName) + 1, szVirusName, _TRUNCATE);

			THREATINFO stThreatInfo = { 0 };
			_tcscpy_s(stThreatInfo.szThreatName, pszVirusName);
			stThreatInfo.dwSpyID = dwSpyID;
			m_mapDetectedTheatInfo.insert(make_pair(uiCRCValue, stThreatInfo));
			return true;
		}
		else
		{
			if (!m_objWWizIndexer.InsertIntoWhiteDB(pszFilePath[0], uiCRCValue))
			{
				CString csLog;
				csLog.Format(L"### Indexing failed Index: [0x%016I64X], File: [%s]", uiCRCValue, pszFilePath);
				AddLogEntry(L"%s", csLog, 0, true, SECONDLEVEL);
			}
			return false;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### CPEScanner::ScanFileSEH failed, FilePath: %s", pszFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSections
*  Description    : Function to read from different sections while scanning PE files
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::ReadBufferFromDiffSections()
{
	bool bReturn = false;
	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;

	memset(m_szFileBuffer, 0, sizeof(m_szFileBuffer));

	//Read 0x300 bytes file start
	DWORD dwBytesRead = 0;
	dwFileOffset = 0;
	if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x300, 0, &dwBytesRead))
	{
		m_iBufferOffset += dwBytesRead;
		bReturn = true;
	}

	//Read 0x1000 bytes start from AEP
	dwBytesRead = 0;
	dwFileOffset = 0;
	DWORD rvaAEP = m_IS64Bit?m_stImageOptionalHeader64.AddressOfEntryPoint:m_stImageOptionalHeader32.AddressOfEntryPoint;
	WORD wSec = (WORD)RVA2FileOffset(rvaAEP, m_NumberOfSections, &dwFileOffset);

	dwBytes2Read = m_dwSRD[wSec];
	if(m_dwSRD[wSec] > 0x1000)
	{
		dwBytes2Read = 0x1000;
	}

	if(dwFileOffset > 0)
	{
		if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwBytes2Read, 0, &dwBytesRead))
		{
			m_iBufferOffset += dwBytesRead;
			bReturn = true;
		}
	}

	//Read 0x1000 bytes start of last section
	dwBytesRead = 0;
	int iLastSection = m_NumberOfSections - 1;
	dwFileOffset = m_dwPRD[iLastSection];
	if(dwFileOffset > 0)
	{
		if(m_dwSRD[iLastSection] < 0x1000)
		{
			if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, m_dwSRD[iLastSection], 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
		else
		{
			if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x1000, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}

	//Read 0x1000 bytes from End of last section
	dwBytesRead = 0;
	if(m_dwSRD[iLastSection] > 0x1000)
	{
		dwFileOffset = m_dwPRD[iLastSection] + m_dwSRD[iLastSection] - 0x1000;
		if(dwFileOffset > 0)
		{
			dwBytes2Read = 0x1000;
			/*if((m_dwSRD[iLastSection] - 0x1000) < 0x1000)
			{
				dwBytes2Read = m_dwSRD[iLastSection] - 0x1000;
			}*/
			if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwBytes2Read, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}

	//Read 0x1000 bytes from Flash Data
	dwBytesRead = 0;
	DWORD dwFlashDataSize =  m_dwFileSize - (m_dwPRD[iLastSection] + m_dwSRD[iLastSection]);
	if(dwFlashDataSize > 0)
	{
		dwFileOffset = m_dwPRD[iLastSection] + m_dwSRD[iLastSection];
		if(dwFlashDataSize > 0x1000)
		{
			if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x1000, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
		else
		{
			if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwFlashDataSize, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}
	
	__try
	{
		//File Version Information
		dwBytesRead = 0x0;
		dwBytesRead = GetFileVersionInfoSize(m_szFilePath,NULL);
		if(dwBytesRead)
		{
			dwBytes2Read = dwBytesRead;
			if(dwBytesRead > 0x1000)
			{
				dwBytes2Read = 0x1000;
			}
			if(GetFileVersionInfo(m_szFilePath, NULL, dwBytes2Read, &m_szFileBuffer[m_iBufferOffset]))
			{
				m_iBufferOffset += dwBytes2Read;
				bReturn = true;
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### GetFileVersionInfoSize Failed, Corrupt file: %s", m_szFilePath, 0, true, SECONDLEVEL);
	}
	
	//0x300 bytes from .Data Section
	dwBytesRead = 0x0;
	// Base of data offset
	DWORD dwOffset = m_dwOffset2NewEXEHeader + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + 0x18;
	if(GetFileBuffer(&dwOffset, dwOffset, 4, 4)) 
	{
		if(dwOffset)
		{
			RVA2FileOffset(dwOffset,m_NumberOfSections, &dwOffset);
			if(dwOffset < m_dwFileSize)
			{
				if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwOffset, 0x300, 0, &dwBytesRead))
				{
					m_iBufferOffset += dwBytesRead;
				}
			}
		}
	}
	// Read 0x300 bytes from .data section PRD if .data section is not Base of Data 
	for (int iSecCnt = 0; iSecCnt < m_NumberOfSections; iSecCnt++)
	{		
		BYTE byDataSecName[] = {0x2E, 0x64, 0x61, 0x74, 0x61}; // .data
		if(memcmp(m_stSectionHeader[iSecCnt].Name,byDataSecName, sizeof(byDataSecName)) == 0)
		{
			if(dwOffset != m_dwPRD[iSecCnt]) 
			{
				dwBytesRead = 0;
				if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], m_dwPRD[iSecCnt], 0x300, 0, &dwBytesRead))
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
	DWORD dwAEPUnmapped = m_IS64Bit?m_stImageOptionalHeader64.AddressOfEntryPoint:m_stImageOptionalHeader32.AddressOfEntryPoint;
	WORD wAEPSec = (WORD)RVA2FileOffset(dwAEPUnmapped, m_NumberOfSections, &dwAEPMapped);

	if(m_dwSRD[wAEPSec] > 0x300)
	{
		dwBytes2Read = 0x300;
	}
	if(dwAEPMapped > 0 && dwAEPMapped < m_dwFileSize)
	{
		if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwAEPMapped - 0x300, dwBytes2Read, 0, &dwBytesRead))
		{
			m_iBufferOffset += dwBytesRead;
			bReturn = true;
		}
	}
	
	// Read 0x300 bytes from Cavity
	dwBytesRead = 0x0;
	dwFileOffset = m_dwOffset2NewEXEHeader + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
	DWORD dwOptHeaderSize = m_IS64Bit?sizeof(m_stImageOptionalHeader64):sizeof(m_stImageOptionalHeader32);
	dwFileOffset += dwOptHeaderSize + m_NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	if(dwFileOffset < m_dwFileSize)
	{
		if(GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x300, 0, &dwBytesRead))
		{
			m_iBufferOffset += dwBytesRead;
			bReturn = true;
		}
	}
	
	//Need to close file handle after collecting buffer
	if(m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : GetFileBufferNScan
*  Description    : Function to check whether get the file buffer while scanning PE files
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::GetFileBufferNScan(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID)
{
	return false;
}


/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Function to get the file buffer while scanning PE files
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwBytesToRead, DWORD * pdwBytesRead)
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

		if (!ReadFile(m_hFileHandle, pbReadBuffer, dwBytesToRead, &dwBytesRead, NULL))
		{
			return false;
		}

		if (pdwBytesRead)
		{
			*pdwBytesRead = dwBytesRead;
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
*  Function Name  : GetFileBuffer
*  Description    : Function to get the file buffer while scanning PE files
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
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
*  Function Name  : ScanFileBuffer
*  Description    : Function to scan the file buffer when the buffer is full
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::ScanFileBuffer(unsigned char *szFileBuffer, DWORD dwBytesRead, char *pszVirusName, DWORD &dwSpyID, bool bRescan)
{
	__try
	{
		if (m_pPETreeManager == NULL)
		{
			return false;
		}

		if (m_pPETreeManager->ScanBuffer(szFileBuffer, dwBytesRead, pszVirusName, dwSpyID, bRescan))
		{
			return true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CPEScanner::ScanFileBuffer", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************************************
*  Function Name  : IsValidPEFile
*  Description    : Function to check whether the file is a valid PE file
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
bool CPEScanner::IsValidPEFile(LPCTSTR szFilePath)
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
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
		}

		m_hFileHandle = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
		{
			return bReturn;
		}

		m_dwFileSize = GetFileSize(m_hFileHandle, NULL);

		if (m_dwFileSize == 0xFFFFFFFF)
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		//if file is big (more than 600 MB) no need to scan
		ULONG ulMaxFileSizeLimit = 600 * 1024 * 1024;
		if (m_dwFileSize > ulMaxFileSizeLimit)
		{
			CloseHandle(m_hFileHandle);
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
		if (!GetFileBuffer(&stImageDosHeader, 0, sizeof(stImageDosHeader), sizeof(stImageDosHeader)))
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		if (stImageDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		DWORD dwSignature = 0;
		m_dwOffset2NewEXEHeader = stImageDosHeader.e_lfanew;
		if (!GetFileBuffer(&dwSignature, stImageDosHeader.e_lfanew, sizeof(DWORD), sizeof(DWORD)))
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}
		if (dwSignature != IMAGE_NT_SIGNATURE)
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		IMAGE_FILE_HEADER stImageFileHeader;
		memset(&stImageFileHeader, 0, sizeof(stImageFileHeader));

		if (!GetFileBuffer(&stImageFileHeader, stImageDosHeader.e_lfanew + 4, sizeof(stImageFileHeader), sizeof(stImageFileHeader)))
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}

		if (stImageFileHeader.NumberOfSections > 0x63 || 0 == stImageFileHeader.NumberOfSections)
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
			return false;
		}
		
		m_NumberOfSections = stImageFileHeader.NumberOfSections;

		if (IMAGE_FILE_MACHINE_IA64 == stImageFileHeader.Machine || IMAGE_FILE_MACHINE_AMD64 == stImageFileHeader.Machine)
		{
			dwBytesRead = 0;
			memset(&m_stImageOptionalHeader64, 0, sizeof(m_stImageOptionalHeader64));
			if (!GetFileBuffer(&m_stImageOptionalHeader64, sizeof(m_stImageOptionalHeader64), &dwBytesRead))
			{
				CloseHandle(m_hFileHandle);
				m_hFileHandle = INVALID_HANDLE_VALUE;
				return false;
			}
			m_IS64Bit = true;
		}
		else
		{
			dwBytesRead = 0;
			memset(&m_stImageOptionalHeader32, 0, sizeof(m_stImageOptionalHeader32));
			if (!GetFileBuffer(&m_stImageOptionalHeader32, sizeof(m_stImageOptionalHeader32), &dwBytesRead))
			{
				CloseHandle(m_hFileHandle);
				m_hFileHandle = INVALID_HANDLE_VALUE;
				return false;
			}
		}

		memset(&m_stSectionHeader, 0, sizeof(m_stSectionHeader));
		if (!GetFileBuffer(&m_stSectionHeader, sizeof(m_stSectionHeader), &dwBytesRead))
		{
			CloseHandle(m_hFileHandle);
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
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CPEScanner::IsValidPEFile, File: %s", szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
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
DWORD CPEScanner::RVA2FileOffset(DWORD dwRva, WORD nSections ,DWORD *pdwFileOff)
{
	WORD	wSec = 0x00;
	__try
	{
		//If number of sections are greater than 100, we leave
		if (nSections > 0x63)
			return 0;

		/*
		for( ; wSec < nSections; wSec++ )
		{
		if( dwRva >= m_dwRVA[wSec] && dwRva < (m_dwRVA[wSec]+ m_dwVS[wSec]) )
		{
		*pdwFileOff = dwRva - m_dwRVA[wSec] + m_dwPRD[wSec] ;
		break ;
		}
		}
		*/

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
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CPEScanner::RVA2FileOffset", 0, 0, true, SECONDLEVEL);
	}
	return wSec;
}

/***************************************************************************************************
*  Function Name  : ScanFileHeuristically()
*  Description    : Sends file for heuristic scan
*  Author Name    : Vilas
*  Date			  :	14-Mar-2015
*  Date Modified  : 07-Aug-2015
****************************************************************************************************/
//bool CISpyScanDLLApp::ScanFileHeur(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID)
//Modified by Vilas on 07-Aug-2015 to support return value for PE file type and 64 bit file by reference
bool CPEScanner::ScanFileHeur(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool &bISPEFile, bool &bISX64PEFile)
{
	bool	bRetValue = false;

	__try
	{
		if (!m_ScanFileForHeuristic)
			return bRetValue;

		bRetValue = m_ScanFileForHeuristic(pszFilePath, pszVirusName, dwSpyID, bISPEFile, bISX64PEFile);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CPEScanner::ScanFileHeur, FilePath: %s", pszFilePath, 0, true, SECONDLEVEL);
		bRetValue = false;
	}
	return bRetValue;
}

/***************************************************************************************************
*  Function Name  : LoadHeuristicScanDLL()
*  Description    : Load Heuristic Scan DLL
*  Author Name    : Vilas
*  Date			  :	14-Mar-2015
*  Modified Date  :	29-July-2015
****************************************************************************************************/
bool CPEScanner::LoadHeuristicScanDLL()
{

	__try
	{
		TCHAR	szAppPath[512] = { 0 };

		//WardWiz path is obtained from registry because 
		//when we load heuristic from Outlook, path is like C:\Program Files (x86)\Microsoft Office\Office14\OUTLOOK.EXE
		//Which fails loading of heuristic DLL
		//Added by Vilas on 29 July 2015
		GetWardWizPath(szAppPath);

		if (wcslen(szAppPath) == 0x00)
		{
			GetModuleFileName(NULL, szAppPath, 511);

			TCHAR *pTemp = wcsrchr(szAppPath, '\\');

			if (!pTemp)
				return false;

			pTemp++;
			*pTemp = '\0';
		}

#ifdef MSOUTLOOK32
		wcscat_s(szAppPath, 511, L"WRDWIZHEUSCN32.DLL");
#else
		wcscat_s(szAppPath, 511, L"VBHEUSCN.DLL");
#endif

		m_hHeuristicScan = LoadLibrary(szAppPath);
		if (m_hHeuristicScan)
			m_ScanFileForHeuristic = (SCANFILEHEUR)GetProcAddress(m_hHeuristicScan, "ScanFileForHeuristic");

		if (!m_ScanFileForHeuristic)
			return false;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::LoadHeuristicScanDLL", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : GetWardWizPath()
*  Description    : Gets WardWiz Product Path from Registry
*  Author Name    : Vilas
*  Date			  :	29-July-2015
****************************************************************************************************/
void CPEScanner::GetWardWizPath(LPTSTR lpszWardWizPath)
{
	try
	{
		CString csWardWizPath = GetWardWizPathFromRegistry();

		if (csWardWizPath.GetLength())
			swprintf_s(lpszWardWizPath, 511, L"%s", csWardWizPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::GetWardWizPath", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ISAlreadyDetected
*  Description    : Check if file is already detected
*  Author Name    : Ram Shelke
*  Date			  :	30-July-2016
****************************************************************************************************/
bool CPEScanner::ISAlreadyDetected(ULONG64 uiCRCValue, PTHREATINFO pThreatInfo, bool bHeuScan)
{
	try
	{
		MAPTHREATINORMATION::const_iterator aIterator;
		aIterator = m_mapDetectedTheatInfo.find(uiCRCValue);

		//Do we have it?
		if (aIterator != m_mapDetectedTheatInfo.end())
		{
			_tcscpy_s(pThreatInfo->szThreatName, (*aIterator).second.szThreatName);			
			pThreatInfo->dwSpyID = (*aIterator).second.dwSpyID;
			pThreatInfo->bIsHeuDetection = (*aIterator).second.bIsHeuDetection;

			if (!bHeuScan && pThreatInfo->bIsHeuDetection)
			{
				return false;
			}
		  return true;
		}
	}
	catch (...)
	{
		CString csCRCValue;
		csCRCValue.Format(L"%d", uiCRCValue);
		AddLogEntry(L"### Exception in CPEScanner::ISAlreadyScanned, CRCValue: %s", csCRCValue, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : ReadBufferFromDiffSections
*  Description    : Function to read from different sections while scanning PE files
*  Author Name    : Ramkrushna Shelke
*  Updated by	  : Nitin Kolapkar (25th August 2016)
*  SR_NO		  :
*  Date			  : 18 Sep 2013
****************************************************************************************************/
DWORD CPEScanner::ReadBufferFromDiffSectionsForUtility(LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &dwExactBufferSize)
{
	bool bReturn = false;
	
	DWORD dwFileOffset = 0;
	DWORD dwBytes2Read = 0;

	memset(m_szFileBuffer, 0, sizeof(m_szFileBuffer));

	//Read 0x300 bytes file start
	DWORD dwBytesRead = 0;
	dwFileOffset = 0;
	if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x300, 0, &dwBytesRead))
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
		if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwBytes2Read, 0, &dwBytesRead))
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
			if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, m_dwSRD[iLastSection], 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
		else
		{
			if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x1000, 0, &dwBytesRead))
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
			if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwBytes2Read, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}

	//Read 0x1000 bytes from Flash Data
	dwBytesRead = 0;
	DWORD dwFlashDataSize = m_dwFileSize - (m_dwPRD[iLastSection] + m_dwSRD[iLastSection]);
	if (dwFlashDataSize > 0)
	{
		dwFileOffset = m_dwPRD[iLastSection] + m_dwSRD[iLastSection];
		if (dwFlashDataSize > 0x1000)
		{
			if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x1000, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
		else
		{
			if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, dwFlashDataSize, 0, &dwBytesRead))
			{
				m_iBufferOffset += dwBytesRead;
				bReturn = true;
			}
		}
	}

	__try
	{
		//File Version Information
		dwBytesRead = 0x0;
		dwBytesRead = GetFileVersionInfoSize(m_szFilePath, NULL);
		if (dwBytesRead)
		{
			dwBytes2Read = dwBytesRead;
			if (dwBytesRead > 0x1000)
			{
				dwBytes2Read = 0x1000;
			}
			if (GetFileVersionInfo(m_szFilePath, NULL, dwBytes2Read, &m_szFileBuffer[m_iBufferOffset]))
			{
				m_iBufferOffset += dwBytes2Read;
				bReturn = true;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### GetFileVersionInfoSize Failed, Corrupt file: %s", m_szFilePath, 0, true, SECONDLEVEL);
	}

	//0x300 bytes from .Data Section
	dwBytesRead = 0x0;
	// Base of data offset
	DWORD dwOffset = m_dwOffset2NewEXEHeader + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + 0x18;
	if (GetFileBuffer(&dwOffset, dwOffset, 4, 4))
	{
		if (dwOffset)
		{
			RVA2FileOffset(dwOffset, m_NumberOfSections, &dwOffset);
			if (dwOffset < m_dwFileSize)
			{
				if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwOffset, 0x300, 0, &dwBytesRead))
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
				if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], m_dwPRD[iSecCnt], 0x300, 0, &dwBytesRead))
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
		if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwAEPMapped - 0x300, dwBytes2Read, 0, &dwBytesRead))
		{
			m_iBufferOffset += dwBytesRead;
			bReturn = true;
		}
	}

	// Read 0x300 bytes from CavityIsValidPEFile
	dwBytesRead = 0x0;
	dwFileOffset = m_dwOffset2NewEXEHeader + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
	DWORD dwOptHeaderSize = m_IS64Bit ? sizeof(m_stImageOptionalHeader64) : sizeof(m_stImageOptionalHeader32);
	dwFileOffset += dwOptHeaderSize + m_NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	if (dwFileOffset < m_dwFileSize)
	{
		if (GetFileBuffer(&m_szFileBuffer[m_iBufferOffset], dwFileOffset, 0x300, 0, &dwBytesRead))
		{
			m_iBufferOffset += dwBytesRead;
			bReturn = true;
		}
	}

	//Need to close file handle after collecting buffer
	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
	
	memcpy_s(lpBuffer, dwBufferSize, m_szFileBuffer, m_iBufferOffset);
	dwExactBufferSize = m_iBufferOffset;
	
	return 0x00;
}


/***************************************************************************************************
*  Function Name  : ClearIndedexes
*  Description    : Function to clear indexes
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
bool CPEScanner::ClearIndedexes()
{
	bool bReturn = false;
	try
	{
		bReturn = m_objWWizIndexer.ClearIndedexes();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizIndexer::ClearIndedexes", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : IS64Bit
*  Description    : Function to check whether the file is 64 bit.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date			  : 2 Dec 2016
****************************************************************************************************/
bool CPEScanner::IS64Bit()
{
	bool bReturn = false;
	__try
	{
		bReturn = m_IS64Bit;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CPEScanner::IS64Bit()", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}