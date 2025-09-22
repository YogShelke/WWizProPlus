/**********************************************************************************************************
Program Name          : PHPScanner.h
Description           : Class for PHP file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 29 Apr 2016
Version No            :
Special Logic Used    : This class is created for scanning files of HTML types

Modification Log      :
1. Sanjay	          : Created Class CHTMLScanner       29 - 04 - 2016
***********************************************************************************************************/


#pragma once

#include "MD5Scanner.h"

class CPHPScanner
{
public:
	CPHPScanner();
	virtual ~CPHPScanner();

public:
	bool LoadPHPSignatures(DWORD &dwSigCount);
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PHP);
	bool ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PHP);
	bool IsValidPHPFile(LPCTSTR lpFilePath);
	bool IsValidPHPFileSEH(LPCTSTR lpFilePath);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool ScanForMD5AndName(LPBYTE m_szHTMLSmallFileBuffer, UINT m_iHTMLBufferOffset, LPTSTR pszVirusName, DWORD & dwSpyID, bool bRescan, FILETYPE filetype = FILE_HTML);
	bool ReadBufferFromDiffSectionsOfPHP(LPTSTR & pszVirusName, DWORD &dwSpyID, bool bRescan = false);

	//Added by Shodhan on 12 sept 2016 Sept to read particular sections of the PHP file for Generating Hash
	DWORD ReadBufferFromDiffSectionOfPHPAndHash(LPCTSTR pszFilePath);
	DWORD ZeroFill(LPCTSTR pszFilePath, DWORD dwAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString);
	DWORD GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber);

public:
	HANDLE			m_hFileHandle;
	DWORD			m_dwPHPFileSize;
	BYTE			m_szPHPFileBuffer[0xC000];
	DWORD			m_dwPHPBufferOffset;
	TCHAR			m_szPHPFilePath[MAX_PATH];
	bool			m_bPHPSignatureLoaded;
	CMD5Scanner		m_MD5PHPScanner;
	CString			m_csVirusName;
};

