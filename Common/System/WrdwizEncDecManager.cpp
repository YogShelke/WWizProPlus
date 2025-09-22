/****************************************************
*  Program Name: WrdwizEncDecManager.cpp                                                                                              
*  Author Name: Prajakta                                                                                                      
*  Date Of Creation: 15 May 2014
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#include "StdAfx.h"
#include "WrdwizEncDecManager.h"
#include <shlwapi.h>

/**********************************************************************************************************         
*  Function Name  :	CWrdwizEncDecManager                                                     
*  Description    :	C'tor
*  Author Name    :	Prajakta
*  SR.NO		  : WRDWIZCOMMON_0097
*  Date           : 15 May 2014
**********************************************************************************************************/
CWrdwizEncDecManager::CWrdwizEncDecManager(void)
{
	m_pbyEncDecSig = NULL;
	m_pbyEncDecSig = (unsigned char *)calloc(MAX_SIG_SIZE,sizeof(unsigned char));
	m_pbyEncDecSig[0x00] = 'W';
	m_pbyEncDecSig[0x01] = 'R';
	m_pbyEncDecSig[0x02] = 'D';
	m_pbyEncDecSig[0x03] = 'W';
	m_pbyEncDecSig[0x04] = 'I';
	m_pbyEncDecSig[0x05] = 'Z';
	m_pbyEncDecSig[0x06] = 'D';
	m_pbyEncDecSig[0x07] = 'B';

	m_pbyEncDecKey = NULL;
	m_pbyEncDecKey = (unsigned char *)calloc(MAX_KEY_SIZE,sizeof(unsigned char)); 

	m_pbyCryptBuffer = NULL;
	m_pbyCryptBuffer = (unsigned char *)calloc(MAX_BUFF_SIZE,sizeof(unsigned char)); 

	m_pbyTmpBuffer = NULL;
	m_pbyTmpBuffer = (unsigned char *)calloc(MAX_BUFF_SIZE,sizeof(unsigned char)); 
}

/**********************************************************************************************************         
*  Function Name  :	~CWrdwizEncDecManager                                                     
*  Description    :	Destructor
*  Author Name    :	Prajakta     
*  SR.NO		  : WRDWIZCOMMON_0098
*  Date           : 15 May 2014
**********************************************************************************************************/
CWrdwizEncDecManager::~CWrdwizEncDecManager(void)
{
	if(m_pbyEncDecKey != NULL)
	{
		free(m_pbyEncDecKey);
		m_pbyEncDecKey = NULL;
	}
	if(m_pbyEncDecSig != NULL)
	{
		free(m_pbyEncDecSig);
		m_pbyEncDecSig = NULL;
	}
	if(m_pbyCryptBuffer != NULL)
	{
		free(m_pbyCryptBuffer);
		m_pbyCryptBuffer = NULL;
	}
	if(m_pbyTmpBuffer != NULL)
	{
		free(m_pbyTmpBuffer);
		m_pbyTmpBuffer = NULL;
	}
}


/**********************************************************************************************************       *  Function Name  :	DecryptDBFileData                                                     
*  Description    :	1. Checks if file is encrypted (by checking existence of sig:WARDWIZ)
					2. If yes, reads encryption key from encrypted file 
					3. Decrypts data in chunks of 0x10000 & stores in temporary file
*  Author Name    :	Prajakta                                                                                        
*  SR.NO		  : WRDWIZCOMMON_0099
*  Date           : 15 May 2014
**********************************************************************************************************/
bool CWrdwizEncDecManager::DecryptDBFileData(CString csFileName, LPTSTR szTmpFilePath, DWORD &dwDBMajorVersion, DWORD &dwDBVerLength)
{
	if( !PathFileExists(csFileName))
		return false;

	OutputDebugString(L">>> In CWrdwizEncDecManager::DecryptDBFileData");

	unsigned long	ulTotalBytes = 0x00, ulBytes2Read = 0x00, ulBytesRead = 0x00, ulEncFileSize = 0x0, ulTmpTotalBytes = 0x0;
	
	if (csFileName.GetLength() == NULL || m_pbyCryptBuffer == NULL || m_pbyEncDecKey == NULL || m_pbyEncDecSig == NULL)
	{
		return false;
	}

	DWORD dwDBVersionLength = 0;
	if (!IsFileAlreadyEncrypted(csFileName, dwDBVersionLength, dwDBMajorVersion))
	{
		return false;
	}

	//Assign DB version length
	dwDBVerLength = dwDBVersionLength;

	//the new implemenatation for DB load has been added and which is in place from 1.1.0.0 and above version.
	//if this file version found then no need to decrypt file.
	if (dwDBMajorVersion == 0x01)
	{
		_tcscpy(szTmpFilePath, csFileName);
		return true;
	}

	FILE *pFile = NULL;
	pFile = _wfsopen(csFileName, _T("rb"), 0x20);
	if(!pFile)
	{
		return false;
	}

	fseek(pFile, 0x0, SEEK_END);
	ulEncFileSize = ftell(pFile);
	
	if(!ReadKeyFromEncryptedFile(pFile, dwDBVersionLength))
	{
		fclose(pFile);
		pFile = NULL; 
		return false;
	}

	//create temp file to store decrypted data
	TCHAR szTempFileName[MAX_PATH];  
	TCHAR lpTempPathBuffer[MAX_PATH];
	UINT RetUnique=0;
	//m_csDecTmpFilePath = L"";
	DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer); 
	if(dwRetVal)
	{
		RetUnique=GetTempFileName(lpTempPathBuffer, L"tmp", 0, szTempFileName);
		if(!RetUnique)
		{
			fclose(pFile);
			pFile = NULL;
			return false;
		}
		_tcscpy(szTmpFilePath, szTempFileName);
		//m_csDecTmpFilePath = szTempFileName;
	}

	FILE *pTmpFile = NULL;
	OutputDebugString(L">>> Temp file Path");
	OutputDebugString(szTmpFilePath);

	pTmpFile = _wfsopen(szTmpFilePath, _T("rb+"), 0x20);
	if(!pTmpFile)
	{
		return false;
	}


	ulTotalBytes = MAX_SIG_SIZE + dwDBVersionLength + MAX_KEY_SIZE + MAX_TOKENSTRINGLEN;
	//ulTotalBytes += (MAX_SIG_SIZE + MAX_KEY_SIZE + MAX_DBVERSIONNO_SIZE);

	fseek(pFile, (0 + ulTotalBytes), SEEK_SET);
	while(ulTotalBytes < ulEncFileSize)
	{
		if((ulEncFileSize - ulTotalBytes) > MAX_BUFF_SIZE)
		{
			ulBytes2Read = MAX_BUFF_SIZE;
		}
		else
		{
			ulBytes2Read = (ulEncFileSize - ulTotalBytes);
		}
		
		memset(&m_pbyCryptBuffer[0x00],0x00,MAX_BUFF_SIZE);
		ulBytesRead = 0x00;
		ulBytesRead = static_cast<unsigned long>(fread(&m_pbyCryptBuffer[0x00], sizeof(unsigned char), ulBytes2Read, pFile));

		if(ulBytesRead == 0x00)
		{
			break;
		}

		if(CryptData(&m_pbyCryptBuffer[0x00], ulBytesRead, &m_pbyEncDecKey[0x00], MAX_KEY_SIZE))
		{
			fseek(pTmpFile, ulTmpTotalBytes, SEEK_SET);
			fwrite(&m_pbyCryptBuffer[0x00], ulBytesRead, sizeof(unsigned char),pTmpFile);
		}

		ulTotalBytes += ulBytesRead;
		ulTmpTotalBytes += ulBytesRead;
		if (ulTotalBytes >= ulEncFileSize)
		{
			break;
		}
	}
	fclose(pFile);
	pFile = NULL;
	fclose(pTmpFile);
	pTmpFile = NULL;

	return true;
}

/**********************************************************************************************************         
*  Function Name  :	IsFileAlreadyEncrypted                                                     
*  Description    :	Checks if file is encrypted (by checking existence of sig:WARDWIZ)
*  SR.NO		  : WRDWIZCOMMON_0100
*  Author Name    :	Prajakta                                                                                        
*  Date           : 15 May 2014
**********************************************************************************************************/
bool CWrdwizEncDecManager::IsFileAlreadyEncrypted(CString csFileName, DWORD &dwDBVersionLength, DWORD &dwDBMajorVersion)
{	
	bool bReturn = false;
	char	bySigBuff[MAX_VERSIONCHKBUFF] = {0x00};

	if (csFileName.GetLength() == NULL || m_pbyEncDecSig == NULL)
	{
		return bReturn; //error
	}

	FILE *pFile = NULL;
	m_ulFileSize = 0x0;
	pFile = _wfsopen(csFileName, _T("r"), 0x20);
	if(!pFile)
	{
		return bReturn;

	}
	fseek(pFile, 0x00, SEEK_END);
	m_ulFileSize = ftell(pFile);
	if(m_ulFileSize < MAX_SIG_SIZE)
	{
		fclose(pFile);
		pFile = NULL; 
		return bReturn;
	}

	fseek(pFile, 0x0,SEEK_SET);
	//
	//TCHAR szVersion[20] = {0};
	//GetVersionDetails(pFile);
	
	bool bValidSig = false;
	fread_s(&bySigBuff[0x00], MAX_SIG_SIZE, MAX_SIG_SIZE, 0x01, pFile);
	
	if (memcmp(&bySigBuff,&m_pbyEncDecSig[0x00],MAX_SIG_SIZE) == 0x00)
	{
		bValidSig = true;
	}

	fseek(pFile, 0x0,SEEK_SET);

	//Get here the version number by tokenizing.
	memset(&bySigBuff, 0, MAX_VERSIONCHKBUFF);
	fread_s(&bySigBuff[0x00], MAX_VERSIONCHKBUFF, MAX_VERSIONCHKBUFF, 0x01, pFile);

	//Tokenize buffer
	char seps[]		= "|";
	char *token		= NULL;
	char* context	= NULL;
	token = strtok_s( bySigBuff, seps, &context);

	DWORD dwCount = 0;

	bool bValidSigLength = false;
	bool bValidVersion = false;
	while( token != NULL )
	{
		if(strlen(token) > 0)
		{
			if(dwCount == 0)
			{
				if(strlen(token) == 0x0A)
				{
					bValidSigLength = true;
				}
			}

			if(dwCount >= 1 )
			{
				break;
			}
			dwCount++;
		}
		token = strtok_s( NULL, seps, &context);
	}

	if(token != NULL)
	{
		DWORD dwlength = (DWORD)strlen(token);
		if( dwlength > 0 )
		{
			/* Parse here DB vesion and get major vers*/
			if (ParseDBVersion(token, dwDBMajorVersion))
			{
				bValidVersion = true;
				dwDBVersionLength = dwlength;
			}
		}
	}

	if(bValidSig && bValidVersion && bValidSigLength)
	{
		bReturn = true;
	}
	fclose(pFile);
	pFile = NULL; 
	return bReturn;
}


/**********************************************************************************************************         
*  Function Name  :	ReadKeyFromEncryptedFile                                                     
*  Description    :	Fetches encryption key from encrypted file
*  Author Name    :	Prajakta                                                                                        
*  SR.NO		  : WRDWIZCOMMON_0101
*  Date           : 15 May 2014
**********************************************************************************************************/
bool CWrdwizEncDecManager::ReadKeyFromEncryptedFile(FILE *pFile, DWORD dwDBVersionLength)
{
	bool	bReturn = false;
	int		iReadPos = MAX_SIG_SIZE + MAX_KEY_SIZE;
	int		iBytesRead = 0x00;

	memset(m_pbyEncDecKey, 0x00, MAX_KEY_SIZE * sizeof(unsigned char));
	
	fseek(pFile, (0 + MAX_SIG_SIZE + MAX_TOKENSTRINGLEN + dwDBVersionLength), SEEK_SET);
	iBytesRead =  static_cast<int>(fread(&m_pbyEncDecKey[0x00], sizeof(unsigned char), MAX_KEY_SIZE,pFile));
	if (iBytesRead != MAX_KEY_SIZE)
	{
		return bReturn;
	}
	return true;
}


/**********************************************************************************************************         
*  Function Name  :	CryptData                                                     
*  Description    :	Performs decryption on file data with encryption key & XOR as operation
*  Author Name    :	Prajakta    
*  SR.NO		  : WRDWIZCOMMON_0102
*  Date           : 15 May 2014
**********************************************************************************************************/
bool CWrdwizEncDecManager::CryptData(unsigned char *Buffer2Crypt, unsigned long ulBufferSize, unsigned char *Key, unsigned long ulKeySize)
{
	bool			bReturn = false;
	unsigned long	i = 0x00, j =0x00;

	if (Buffer2Crypt == NULL || ulBufferSize == 0x00 || Key == NULL  || ulKeySize == 0x00)
	{
		return bReturn;
	}
	for (i = 0x00, j = 0x00; i < ulBufferSize; i++)
	{
		Buffer2Crypt[i] ^= Key[j++];
		if (j == ulKeySize)
		{
			j = 0x00;
		}
		if (i >= ulBufferSize)
		{
			break;
		}
	}
	bReturn = true;
	return bReturn;
} 


/**********************************************************************************************************         
*  Function Name  :	LoadContentFromFile                                                     
*  Description    :	Loads serialized file data
*  Author Name    :	Prajakta     
*  SR.NO		  : WRDWIZCOMMON_0103
*  Date           : 15 May 2014
**********************************************************************************************************/
bool CWrdwizEncDecManager::LoadContentFromFile(CString csFilePath)
{
	m_objEncDec.RemoveAll();
	CFile rFile(csFilePath, CFile::modeRead);
	
	// Create a loading archive
	CArchive arLoad(&rFile, CArchive::load);
	m_objEncDec.SetFileVersion(2);
	m_objEncDec.Serialize(arLoad);
	
	// Close the loading archive
	arLoad.Close();
	rFile.Close();
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ParseDBVersion
*  Description    :	This function will parse the DB version
*  Author Name    :	Prajakta
*  SR.NO		  : WRDWIZCOMMON_0103
*  Date           : 15 May 2014
**********************************************************************************************************/
bool CWrdwizEncDecManager::ParseDBVersion(LPSTR lpszVersion, DWORD &dwMajorVersion)
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
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEncDecManager::ParseDBVersion", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
