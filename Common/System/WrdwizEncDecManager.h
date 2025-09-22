/****************************************************
*  Program Name: WrdwizEncDecManager.h                                                                                                  
*  Author Name: Prajakta                                                                                                      
*  Date Of Creation: 15 May 2014
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once

#include "stdafx.h"
#include <Winbase.h>
#include <afxwin.h>
#include "ISpyDataManager.h"

const unsigned int MAX_KEY_SIZE = 0x10;
const unsigned int MAX_SIG_SIZE = 0x08;

const unsigned int MAX_VERSIONCHKBUFF = 0x50;
const unsigned int MAX_TOKENSTRINGLEN = 0x04;

const unsigned int MAX_BUFF_SIZE = 0x10 * 0x1000; 
const unsigned int MAX_DBVERSIONNO_SIZE = 0x7;

//CLASS & FUNCTIONS DECLARATION
class CWrdwizEncDecManager
{
public:
	CWrdwizEncDecManager(void);
	~CWrdwizEncDecManager(void);
	CDataManager	m_objEncDec;
	unsigned char	*m_pbyEncDecKey;
	unsigned long   m_ulFileSize;
	unsigned char   *m_pbyEncDecSig;
	unsigned char   *m_pbyCryptBuffer;
	unsigned char   *m_pbyTmpBuffer;
		
	bool CryptData(unsigned char *Buffer2Crypt, unsigned long ulBufferSize, unsigned char *Key, unsigned long ulKeySize);
	bool IsFileAlreadyEncrypted(CString csFileName, DWORD &dwDBVersionLength, DWORD &dwDBMajorVersion);
	bool ReadKeyFromEncryptedFile(FILE *pFile, DWORD dwDBVersionLength);
	bool DecryptDBFileData(CString csFileName, LPTSTR szTmpFilePath, DWORD &dwDBMajorVersion, DWORD &dwDBVerLength);
	bool LoadContentFromFile(CString csFilePath);
	bool GetVersionDetails(FILE * pFile);
private:
	bool ParseDBVersion(LPSTR lpszVersion, DWORD &dwMajorVersion);
};