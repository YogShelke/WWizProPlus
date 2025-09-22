#include "stdafx.h"
#include "WWizAdwareScanner.h"

CWWizAdwareScanner::CWWizAdwareScanner():
	m_bIsx64(false)
{
	m_pbyEncDecSig = NULL;
	m_pbyEncDecSig = (unsigned char *)calloc(MAX_SIG_SIZE, sizeof(unsigned char));
	m_pbyEncDecSig[0x00] = 'W';
	m_pbyEncDecSig[0x01] = 'R';
	m_pbyEncDecSig[0x02] = 'D';
	m_pbyEncDecSig[0x03] = 'W';
	m_pbyEncDecSig[0x04] = 'I';
	m_pbyEncDecSig[0x05] = 'Z';
	m_pbyEncDecSig[0x06] = 'D';
	m_pbyEncDecSig[0x07] = 'B';
	
	m_bIsx64 = IsWow64();
	PrepareSystemPaths();
	GetUsersList();
	GetRegistryUsersList();
}

CWWizAdwareScanner::~CWWizAdwareScanner()
{
	if (m_pbyEncDecSig != NULL)
	{
		free(m_pbyEncDecSig);
		m_pbyEncDecSig = NULL;
	}
	ClearVectorData();
}

bool CWWizAdwareScanner::LoadDB()
{
	bool bReturn = false;
	try
	{
		ClearVectorData();
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModulePath(szModulePath, sizeof(szModulePath));
		_tcscat_s(szModulePath, L"\\");
		_tcscat_s(szModulePath, ADWDBNAME);

		if (!PathFileExists(szModulePath))
		{
			return bReturn;
		}

		DWORD dwDBMajorVersion = 0x00;
		DWORD dwDBVersionLength = 0x00;
		if (!IsFileAlreadyEncrypted(szModulePath, dwDBVersionLength, dwDBMajorVersion))
		{
			return bReturn;
		}

		ClearVectorData();

		DWORD dwLoadedCount = 0x00;
		if (!LoadContaintFromFile(szModulePath, dwDBVersionLength, dwLoadedCount))
		{
			AddLogEntry(L"### Exception in CWrdwizEncDecManager::ParseDBVersion", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		bReturn = true;
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in CWWizAdwareScanner::ReLoadDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CWWizAdwareScanner::UnLoadDB()
{
	__try
	{
		ClearVectorData();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWWizAdwareScanner::UnLoadDB", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWWizAdwareScanner::ReLoadDB()
{
	bool bReturn = false;

	__try
	{
		ClearVectorData();
		bReturn = LoadDB();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWWizAdwareScanner::ReLoadDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CWWizAdwareScanner::IsFileAlreadyEncrypted(CString csFileName, DWORD &dwDBVersionLength, DWORD &dwDBMajorVersion)
{
	bool bReturn = false;
	char	bySigBuff[MAX_VERSIONCHKBUFF] = { 0x00 };

	if (csFileName.GetLength() == NULL || m_pbyEncDecSig == NULL)
	{
		return bReturn; //error
	}

	FILE *pFile = NULL;
	m_ulFileSize = 0x0;
	pFile = _wfsopen(csFileName, _T("r"), 0x20);
	if (!pFile)
	{
		return bReturn;

	}
	fseek(pFile, 0x00, SEEK_END);
	m_ulFileSize = ftell(pFile);
	if (m_ulFileSize < MAX_SIG_SIZE)
	{
		fclose(pFile);
		pFile = NULL;
		return bReturn;
	}

	fseek(pFile, 0x0, SEEK_SET);
	//
	//TCHAR szVersion[20] = {0};
	//GetVersionDetails(pFile);

	bool bValidSig = false;
	fread_s(&bySigBuff[0x00], MAX_SIG_SIZE, MAX_SIG_SIZE, 0x01, pFile);

	if (memcmp(&bySigBuff, &m_pbyEncDecSig[0x00], MAX_SIG_SIZE) == 0x00)
	{
		bValidSig = true;
	}

	fseek(pFile, 0x0, SEEK_SET);

	//Get here the version number by tokenizing.
	memset(&bySigBuff, 0, MAX_VERSIONCHKBUFF);
	fread_s(&bySigBuff[0x00], MAX_VERSIONCHKBUFF, MAX_VERSIONCHKBUFF, 0x01, pFile);

	//Tokenize buffer
	char seps[] = "|";
	char *token = NULL;
	char* context = NULL;
	token = strtok_s(bySigBuff, seps, &context);

	DWORD dwCount = 0;

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
		token = strtok_s(NULL, seps, &context);
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
				dwDBVersionLength = dwlength;
			}
		}
	}

	if (bValidSig && bValidVersion && bValidSigLength)
	{
		bReturn = true;
	}
	fclose(pFile);
	pFile = NULL;
	return bReturn;
}

void CWWizAdwareScanner::ClearVectorData()
{
	m_vecServicesLocations.clear();
	m_vecFoldersLocations.clear();
	m_vecFilesLocations.clear();
	m_vecShortcutsLocations.clear();
	m_vecsTasksLocations.clear();
	m_vecRegLocations.clear();
	m_vecBrowserLocations.clear();
	m_vecBrowserRegLocations.clear();

	m_vecServicesData.clear();
	m_vecFoldersData.clear();
	m_vecFilesData.clear();
	m_vecShortcutsData.clear();
	m_vecTasksData.clear();
	m_vecRegData.clear();
	m_vecBrowserData.clear();
	m_vecBrowserRegData.clear();
	m_mapSystemPaths.clear();
}

/**********************************************************************************************************
*  Function Name  :	ParseDBVersion
*  Description    :	This function will parse the DB version
*  Author Name    :	Prajakta
*  SR.NO		  : WRDWIZCOMMON_0103
*  Date           : 15 May 2014
**********************************************************************************************************/
bool CWWizAdwareScanner::ParseDBVersion(LPSTR lpszVersion, DWORD &dwMajorVersion)
{
	__try
	{
		if (!lpszVersion)
			return false;

		const char sToken[2] = ".";
		char *token;
		int iCount = 0;

		/* get the first token */
		token = strtok(lpszVersion, sToken);

		/* walk through other tokens */
		while (token != NULL && (iCount <= 3))
		{
			/* take major version from here */
			if (iCount == 1)
			{
				if (strlen(token) > 0)
				{
					dwMajorVersion = static_cast<DWORD>(atoi(token));
					break;
				}
			}
			iCount++;
			token = strtok(NULL, sToken);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWrdwizEncDecManager::ParseDBVersion", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : LoadContaintFromFile
*  Description    : Load Signature Database with new implementation, this applies from 1.1.0.0 DB version
and above.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	10 May 2016
****************************************************************************************************/
bool CWWizAdwareScanner::LoadContaintFromFile(LPTSTR szFilePath, DWORD dwVersionLength, DWORD &dwSigCount)
{
	bool bReturn = false;

	HANDLE	hInputFileHandle = NULL;
	BYTE	*bFileBuffer = NULL;
	try
	{
		hInputFileHandle = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFileHandle == INVALID_HANDLE_VALUE)
		{
			return bReturn;
		}

		/* Get file size here */
		DWORD dwFileSize = GetFileSize(hInputFileHandle, 0);

		/* If file size if 0 return */
		if (dwFileSize == 0x00)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DWORD dwBufferOffset = MAX_SIG_SIZE + 2 + dwVersionLength + 2;
		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		DWORD dwBytesRead = 0;
		if (!ReadFile(hInputFileHandle, &m_stAdwCleaner, sizeof(m_stAdwCleaner), &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != sizeof(m_stAdwCleaner))
		{
			bReturn = false;
			goto CLEANUP;
		}

		dwBufferOffset += dwBytesRead;
		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		//Read CRC here
		dwBytesRead = 0;
		ULONG64 ulFileCRC = 0x00;
		if (!ReadFile(hInputFileHandle, &ulFileCRC, sizeof(ulFileCRC), &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != sizeof(ulFileCRC))
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Read here remaining file
		dwBufferOffset += dwBytesRead;
		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		DWORD dwRemSize = dwFileSize - dwBufferOffset;
		bFileBuffer = (BYTE*)malloc(dwRemSize * sizeof(BYTE));

		if (bFileBuffer == NULL)
		{
			bReturn = false;
			goto CLEANUP;
		}

		dwBytesRead = 0;
		if (!ReadFile(hInputFileHandle, bFileBuffer, dwRemSize, &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != dwRemSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DecryptData(bFileBuffer, dwRemSize);

		//Check buffer CRC here
		ULONG64	ulBufCRC = 0x00;
		CWWizCRC64	objWWizCRC64;
		objWWizCRC64.CalcCRC64(ulBufCRC, bFileBuffer, dwRemSize);

		//verify here, if not matched means there is change in DB file.
		if (ulBufCRC != ulFileCRC)
		{
			CString csCRC;
			csCRC.Format(L"%04x", ulBufCRC);
			AddLogEntry(L"### CRC value not matched, Calculated CRC: [%s] , File: %s", szFilePath, csCRC, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		dwSigCount = 0x00;

		int iLineOffset = 0;
		DWORD dwLinesCount = 0x00;
		DWORD iBufOffset = 0x00;

		char szLine[MAX_PATH] = { 0 };
		while (iBufOffset < dwRemSize)
		{
			if (bFileBuffer[iBufOffset] != '*')
			{
				if (iLineOffset < MAX_PATH)
				{
					szLine[iLineOffset++] = bFileBuffer[iBufOffset];
				}
			}
			else
			{
				szLine[iLineOffset] = '\0';
				dwLinesCount++;

				if (dwLinesCount <= m_stAdwCleaner.stServices.dwLocationCount)
				{
					ParseRegExpressionRules(szLine, m_vecServicesLocations);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount))
				{
					m_vecServicesData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount))
				{
					ParseExpressionRules(szLine, m_vecFoldersLocations);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount))
				{
					m_vecFoldersData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount))
				{
					ParseExpressionRules(szLine, m_vecFilesLocations);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount))
				{
					m_vecFilesData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount))
				{

					ParseExpressionRules(szLine, m_vecShortcutsLocations);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount))
				{
					m_vecShortcutsData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount))
				{
					ParseExpressionRules(szLine, m_vecsTasksLocations);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount))
				{
					m_vecTasksData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount))
				{
					ParseRegExpressionRules(szLine, m_vecRegLocations);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stRegistry.dwRegistryCount))
				{
					m_vecRegData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stRegistry.dwRegistryCount + m_stAdwCleaner.stBrowsers.dwLocationCount))
				{
					ParseExpressionRules(szLine, m_vecBrowserLocations);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stRegistry.dwRegistryCount + m_stAdwCleaner.stBrowsers.dwLocationCount + m_stAdwCleaner.stBrowsers.dwBrowsersCount))
				{
					m_vecBrowserData.push_back(szLine);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stRegistry.dwRegistryCount + m_stAdwCleaner.stBrowsers.dwLocationCount + m_stAdwCleaner.stBrowsers.dwBrowsersCount + m_stAdwCleaner.stBrowsersReg.dwLocationCount))
				{
					ParseRegExpressionRules(szLine, m_vecBrowserRegLocations);
				}
				else if (dwLinesCount <= (m_stAdwCleaner.stServices.dwLocationCount + m_stAdwCleaner.stServices.dwServicesCount + m_stAdwCleaner.stFolders.dwLocationCount
					+ m_stAdwCleaner.stFolders.dwFoldersCount + m_stAdwCleaner.stFiles.dwLocationCount + m_stAdwCleaner.stFiles.dwFilesCount + m_stAdwCleaner.stShortcuts.dwLocationCount + m_stAdwCleaner.stShortcuts.dwShortcutsCount + m_stAdwCleaner.stSheduledTasks.dwLocationCount + m_stAdwCleaner.stSheduledTasks.dwScheduledTaskCount + m_stAdwCleaner.stRegistry.dwLocationCount + m_stAdwCleaner.stRegistry.dwRegistryCount + m_stAdwCleaner.stBrowsers.dwLocationCount + m_stAdwCleaner.stBrowsers.dwBrowsersCount + m_stAdwCleaner.stBrowsersReg.dwLocationCount + m_stAdwCleaner.stBrowsersReg.dwBrowsersCount))
				{
					m_vecBrowserRegData.push_back(szLine);
				}
				else
				{
					AddLogEntry(L"### UnHandled string, %s", A2BSTR(szLine), 0, true, SECONDLEVEL);
				}

				iLineOffset = 0;
				memset(&szLine, 0, sizeof(szLine));
			}
			iBufOffset++;
		}

		m_dwTotalLocCount = static_cast<DWORD>(m_vecServicesLocations.size()
		+ m_vecFoldersLocations.size()
		+ m_vecFilesLocations.size()
		+ m_vecShortcutsLocations.size()
		+ m_vecsTasksLocations.size()
		+ m_vecRegLocations.size()
		+ m_vecBrowserLocations.size()
		+ m_vecBrowserRegLocations.size());

		m_dwTotalCount = static_cast<DWORD>(m_vecServicesData.size()
		+ m_vecFoldersData.size()
		+ m_vecFilesData.size()
		+ m_vecShortcutsData.size()
		+ m_vecTasksData.size()
		+ m_vecRegData.size()
		+ m_vecBrowserData.size()
		+ m_vecBrowserRegData.size());

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareScanner::LoadContaintFromFile, %s", szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:
	//Cleanup here memory
	if (bFileBuffer != NULL)
	{
		delete[]bFileBuffer;
		bFileBuffer = NULL;
	}

	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : DecryptData
*  Description    : Decrypt data of buffer for registration data.
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
DWORD CWWizAdwareScanner::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
{
	try
	{
		if (IsBadWritePtr(lpBuffer, dwSize))
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			if (lpBuffer[iIndex] != 0)
			{
				if ((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
				{
					lpBuffer[iIndex] = lpBuffer[iIndex];
				}
				else
				{
					lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
					jIndex++;
				}
				if (jIndex == WRDWIZ_KEYSIZE)
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
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizAdwareScanner::DecryptData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

DWORD CWWizAdwareScanner::GetTotalLocationCount()
{
	m_dwTotalLocCount = static_cast<DWORD>(m_vecServicesLocations.size()
		+ m_vecFoldersLocations.size()
		+ m_vecFilesLocations.size()
		+ m_vecShortcutsLocations.size()
		+ m_vecsTasksLocations.size()
		+ m_vecRegLocations.size()
		+ m_vecBrowserLocations.size()
		+ m_vecBrowserRegLocations.size());

	return m_dwTotalLocCount;
}

DWORD CWWizAdwareScanner::GetServicesLocationCount()
{
	return static_cast<DWORD>(m_vecServicesLocations.size());
}

DWORD CWWizAdwareScanner::GetFoldersLocationCount()
{
	return  static_cast<DWORD>(m_vecFoldersLocations.size());
}

DWORD CWWizAdwareScanner::GetFilesLocationsCount()
{
	return  static_cast<DWORD>(m_vecFilesLocations.size());
}

DWORD CWWizAdwareScanner::GetShortcutsLocationsCount()
{
	return  static_cast<DWORD>(m_vecShortcutsLocations.size());
}

DWORD CWWizAdwareScanner::GetTasksLocationsCount()
{
	return  static_cast<DWORD>(m_vecsTasksLocations.size());
}

DWORD CWWizAdwareScanner::GetRegLocationsCount()
{
	return  static_cast<DWORD>(m_vecRegLocations.size());
}

DWORD CWWizAdwareScanner::GetBrowserLocationsCount()
{
	return  static_cast<DWORD>(m_vecBrowserLocations.size());
}

DWORD CWWizAdwareScanner::GetBrowserRegLocationsCount()
{
	return static_cast<DWORD>(m_vecBrowserRegLocations.size());
}

DWORD CWWizAdwareScanner::GetTotalDataCount()
{
	m_dwTotalCount = static_cast<DWORD>(m_vecServicesData.size()
		+ m_vecFoldersData.size()
		+ m_vecFilesData.size()
		+ m_vecShortcutsData.size()
		+ m_vecTasksData.size()
		+ m_vecRegData.size()
		+ m_vecBrowserData.size()
		+ m_vecBrowserRegData.size());

	return m_dwTotalCount;
}

DWORD CWWizAdwareScanner::GetServicesDataCount()
{
	return static_cast<DWORD>(m_vecServicesData.size());
}

DWORD CWWizAdwareScanner::GetFoldersDataCount()
{
	return static_cast<DWORD>(m_vecFoldersData.size());
}

DWORD CWWizAdwareScanner::GetFilesDataCount()
{
	return static_cast<DWORD>(m_vecFilesData.size());
}

DWORD CWWizAdwareScanner::GetShortcutsDataCount()
{
	return static_cast<DWORD>(m_vecShortcutsData.size());
}

DWORD CWWizAdwareScanner::GetTasksDataCount()
{
	return static_cast<DWORD>(m_vecTasksData.size());
}

DWORD CWWizAdwareScanner::GetRegDataCount()
{
	return static_cast<DWORD>(m_vecRegData.size());
}

DWORD CWWizAdwareScanner::GetBrowserDataCount()
{
	return static_cast<DWORD>(m_vecBrowserData.size());
}

DWORD CWWizAdwareScanner::GetBrowserRegDataCount()
{
	return static_cast<DWORD>(m_vecBrowserRegData.size());
}

VECSTRING & CWWizAdwareScanner::GetVectorDetails(ENUMTYPE eType)
{
	bool bReturn = false;
	switch (eType)
	{
	case SERVICESLOC:
		return m_vecServicesLocations;
	case SERVICESDATA:
		return m_vecServicesData;
	case FOLDERSLOC:
		return m_vecFoldersLocations;
	case FOLDERSDATA:
		return m_vecFoldersData;
	case FILESLOC:
		return m_vecFilesLocations;
	case FILESDATA:
		return m_vecFilesData;
	case SHORTCUTSLOC:
		return m_vecShortcutsLocations;
	case SHORTCUTSDATA:
		return m_vecShortcutsData;
	case TASKSLOC:
		return m_vecsTasksLocations;
	case TASKSDATA:
		return m_vecTasksData;
	case REGISTRYLOC:
		return m_vecRegLocations;
	case REGISTRYDATA:
		return m_vecRegData;
	case BROWSERSFILESLOC:
		return m_vecBrowserLocations;
	case BROWSERSFILESDATA:
		return m_vecBrowserData;
	case BROWSERSREGISTRYLOC:
		return m_vecBrowserRegLocations;
	case BROWSERSREGISTRYDATA:
		return m_vecBrowserRegData;
	default:
		break;
	}
	return m_vecEmpty;
}

bool CWWizAdwareScanner::ParseRegExpressionRules(CStringA csInString, VECSTRING &vec2Add)
{
	bool bResult = false;

	int iStartPos = csInString.Find('<');
	int iEndPos = csInString.Find('>', iStartPos+ 1);

	CStringA csMiddleString = csInString.Mid(iStartPos + 1, ( iEndPos - iStartPos) - 1);
	OutputDebugStringA(csMiddleString);

	if (csMiddleString == "ALLUSERS")
	{
		for (VECSTRING::iterator it = m_vecRegUsersList.begin(); it != m_vecRegUsersList.end(); ++it)
		{
			CStringA csOutString = csInString;
			csOutString.Replace("<ALLUSERS>", (*it).c_str());
			OutputDebugStringA(csOutString);
			vec2Add.push_back(csOutString.GetBuffer());
		}
	}
	else
	{
		OutputDebugStringA(csInString);
		vec2Add.push_back(csInString.GetBuffer());
	}
	return bResult;
}

bool CWWizAdwareScanner::ParseExpressionRules(CStringA csInString, VECSTRING &vec2Add)
{
	bool bResult = false;
	
	int iStartPos = csInString.Find('<');
	int iEndPos = csInString.Find('>');

	CStringA csMiddleString = csInString.Mid(iStartPos + 1, iEndPos - 1);

	if (csMiddleString == "ALLUSERS")
	{
		for (VECSTRING::iterator it = m_vecUsersList.begin(); it != m_vecUsersList.end(); ++it)
		{
			CHAR szUserProfile[MAX_PATH] = { 0 };
			GetEnvironmentVariableA("USERPROFILE", szUserProfile, MAX_PATH);
			if (strrchr(szUserProfile, _T('\\')))
			{
				*strrchr(szUserProfile, _T('\\')) = 0;
			}

			char szStrLine[MAX_PATH] = { 0 };

			if ((iEndPos + 1) == csInString.GetLength())
			{
				sprintf_s(szStrLine, "%s\\%s", szUserProfile, (*it).c_str());
			}
			else
			{
				sprintf_s(szStrLine, "%s\\%s%s", szUserProfile, (*it).c_str(), csInString.Right(csInString.GetLength() - ( iEndPos + 1 )));
			}
			vec2Add.push_back(szStrLine);
		}
	}

	MAPSYSVARIABLES::iterator it = m_mapSystemPaths.find(csMiddleString);
	if (it != m_mapSystemPaths.end())
	{
		bResult = true;
		vec2Add.push_back((*it).second.GetBuffer());
	}
	return bResult;
}

bool CWWizAdwareScanner::PrepareSystemPaths()
{
	bool bResult = false;
	
	CHAR szWinDir[MAX_PATH] = { 0 };
	GetWindowsDirectoryA(szWinDir, sizeof(szWinDir));
	m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("WINDIR", szWinDir));

	CStringA csOSDrive(szWinDir);
	csOSDrive = csOSDrive.Left(2);
	m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("OSDRIVE", csOSDrive));

	CHAR szUserProfile[MAX_PATH] = { 0 };
	GetEnvironmentVariableA("USERPROFILE", szUserProfile, MAX_PATH);
	m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("USERPROFILE", szUserProfile));

	CHAR szCommonProgramFiles[MAX_PATH] = { 0 };
	GetEnvironmentVariableA("COMMONPROGRAMFILES", szCommonProgramFiles, MAX_PATH);
	m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("COMMONPROGRAMFILES", szCommonProgramFiles));

	if (m_bIsx64)
	{
		GetEnvironmentVariableA("COMMONPROGRAMFILES(X86)", szCommonProgramFiles, MAX_PATH);
		if (szCommonProgramFiles[0])
		{
			_strupr_s(szCommonProgramFiles, strlen(szCommonProgramFiles)*sizeof(TCHAR));
			CHAR szCommonProgramFiles[MAX_PATH] = { 0 };
			m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("COMMONPROGRAMFILES(X86)", szCommonProgramFiles));
		}
	}

	CHAR szProgramDir[MAX_PATH] = { 0 };
	if (m_bIsx64)
	{
		GetEnvironmentVariableA("PROGRAMFILES(X86)", szProgramDir, MAX_PATH);
		_strupr_s(szProgramDir, strlen(szProgramDir)*sizeof(TCHAR));
		m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("PROGRAMFILES(X86)", szProgramDir));

		ExpandEnvironmentStringsA("%ProgramW6432%", szProgramDir, MAX_PATH);
		m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("PROGRAMFILES", szProgramDir));
	}
	else
	{
		GetEnvironmentVariableA("PROGRAMFILES", szProgramDir, MAX_PATH);
		m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("PROGRAMFILES", szProgramDir));
	}

	CHAR szProgramData[MAX_PATH] = { 0 };
	GetEnvironmentVariableA("PROGRAMDATA", szProgramData, MAX_PATH);
	m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("PROGRAMDATA", szProgramDir));


	CHAR szPublic[MAX_PATH] = { 0 };
	GetEnvironmentVariableA("PUBLIC", szPublic, MAX_PATH);
	m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("PUBLIC", szPublic));

	bool bVistaOnward = false;
	OSVERSIONINFO	OSVI = { 0 };
	OSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OSVI);

	if (OSVI.dwMajorVersion > 5)
	{
		bVistaOnward = true;
	}

	CHAR szAppData[MAX_PATH] = { 0 };
	GetEnvironmentVariableA("APPDATA", szAppData, MAX_PATH);
	if (!bVistaOnward)
	{
		strcat(szAppData, "\\Application Data");
		m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("APPDATA", szAppData));
	}
	else
	{
		m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("APPDATA", szAppData));
	}

	CHAR szCommmonDesktop[MAX_PATH] = { 0 };
	if (SHGetSpecialFolderPathA(NULL, szCommmonDesktop, CSIDL_COMMON_DESKTOPDIRECTORY, 0))
	{
		m_mapSystemPaths.insert(std::pair<CStringA, CStringA>("CSIDL_COMMON_DESKTOPDIRECTORY", szCommmonDesktop));
	}
	return bResult;
}

/***************************************************************************************************
*  Function Name  : IsWow64()
*  Description    : It will check client machine is 64 bit or 32bit.
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0030
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWWizAdwareScanner::IsWow64()
{
	BOOL bReturn = FALSE;
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS		IsWow64Processes = NULL;
	OSVERSIONINFO			OSVI = { 0 };
	TCHAR szValue[64] = { 0 };

	try
	{

		OSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&OSVI);

		IsWow64Processes = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),
			"IsWow64Process");
		if (!IsWow64Processes)
		{
			AddLogEntry(L"### IsWow64::Failed to get address of IsWow64Process()", 0, 0, true, SECONDLEVEL);
			return false;
		}

		IsWow64Processes(GetCurrentProcess(), &bReturn);
		GetEnvironmentVariable(L"PROCESSOR_ARCHITECTURE", szValue, 63);
		if (wcsstr(szValue, L"64"))
		{
			bReturn = TRUE;
		}
	}
	catch (...)
	{
		bReturn = FALSE;
		AddLogEntry(L"### WardWizAluSrv::IsWow64::Exception", 0, 0, true, SECONDLEVEL);
	}
	return bReturn == TRUE ? true : false;
}

bool CWWizAdwareScanner::GetUsersList()
{
	bool bReturn = false;

	LPUSER_INFO_0 pBuf = NULL;
	LPUSER_INFO_0 pTmpBuf;
	DWORD dwLevel = 0;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	//DWORD i;
	DWORD dwTotalCount = 0;
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = NULL;

	// Call the NetUserEnum() function, specifying level 0;
	//   enumerate global user account types only.
	do // begin do
	{

		nStatus = NetUserEnum((LPCWSTR)pszServerName,
			dwLevel,
			FILTER_NORMAL_ACCOUNT, // global users
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);

		// If the call succeeds,
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				// Loop through the entries.
				for (int i = 0; (i < dwEntriesRead); i++)
				{
					if (pTmpBuf == NULL)
					{
						break;
					}

					char szUserName[MAX_PATH] = { 0 };
					wcstombs(szUserName, pTmpBuf->usri0_name, wcslen(pTmpBuf->usri0_name));
					m_vecUsersList.push_back(szUserName);
					pTmpBuf++;
					dwTotalCount++;
				}
			}
			bReturn = true;
		}
		// Otherwise, print the system error.
		else
		{
			bReturn = false;
			CString csError;
			csError.Format(L"NetUserEnum() failed!, A system error has occurred: %d", nStatus);
			AddLogEntry(L"### %s", csError);
		}

		// Free the allocated buffer.
		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	}
	// Continue to call NetUserEnum while there are more entries.
	while (nStatus == ERROR_MORE_DATA); // end do

	// Check again for allocated memory.
	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
	}

	CHAR szUserProfile[MAX_PATH] = { 0 };
	GetEnvironmentVariableA("USERPROFILE", szUserProfile, MAX_PATH);
	CStringA csCurrentUser = szUserProfile;

	csCurrentUser = csCurrentUser.Right(csCurrentUser.GetLength() - ( csCurrentUser.ReverseFind('\\') + 1 ));
	m_vecUsersList.push_back(csCurrentUser.GetBuffer());
	m_vecUsersList.push_back("Public");
	m_vecUsersList.push_back("All Users");

	 return bReturn;
}

bool CWWizAdwareScanner::GetRegistryUsersList()
{
	EnumerateSubKeys(HKEY_USERS, "");
	return true;
}

void CWWizAdwareScanner::EnumerateSubKeys(HKEY RootKey, char* subKey, unsigned int tabs)
{
	HKEY hKey;
	DWORD cSubKeys;        //Used to store the number of Subkeys
	DWORD maxSubkeyLen;    //Longest Subkey name length
	DWORD cValues;        //Used to store the number of Subkeys
	DWORD maxValueLen;    //Longest Subkey name length
	DWORD retCode;        //Return values of calls

	RegOpenKeyExA(RootKey, subKey, 0, KEY_ALL_ACCESS, &hKey);

	RegQueryInfoKey(hKey,            // key handle
		NULL,            // buffer for class name
		NULL,            // size of class string
		NULL,            // reserved
		&cSubKeys,        // number of subkeys
		&maxSubkeyLen,    // longest subkey length
		NULL,            // longest class string 
		&cValues,        // number of values for this key 
		&maxValueLen,    // longest value name 
		NULL,            // longest value data 
		NULL,            // security descriptor 
		NULL);            // last write time

	if (cSubKeys > 0)
	{
		char currentSubkey[MAX_PATH];

		for (int i = 0; i < cSubKeys; i++){
			DWORD currentSubLen = MAX_PATH;

			retCode = RegEnumKeyExA(hKey,    // Handle to an open/predefined key
				i,                // Index of the subkey to retrieve.
				currentSubkey,            // buffer to receives the name of the subkey
				&currentSubLen,            // size of that buffer
				NULL,                // Reserved
				NULL,                // buffer for class string 
				NULL,                // size of that buffer
				NULL);                // last write time

			if (retCode == ERROR_SUCCESS)
			{
				char subKeyPath[MAX_PATH] = { 0 };
				if (strlen(subKey) == 0)
					sprintf(subKeyPath, "%s", currentSubkey);
				else
					sprintf(subKeyPath, "%s\\%s", subKey, currentSubkey);
				m_vecRegUsersList.push_back(subKeyPath);
			}
		}
	}
	RegCloseKey(hKey);
}