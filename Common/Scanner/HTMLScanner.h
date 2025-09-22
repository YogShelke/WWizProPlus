/**********************************************************************************************************
Program Name          : HTMLScanner.h
Description           : Class for HTML file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 31 Mar 2016
Version No            :
Special Logic Used    : This class is created for scanning files of HTML types

Modification Log      :
1. Sanjay	          : Created Class CHTMLScanner       31 - 03 - 2016
***********************************************************************************************************/


#pragma once

#include "MD5Scanner.h"

class CHTMLScanner
{
public:
	CHTMLScanner();
	virtual ~CHTMLScanner();
public:
	bool LoadHTMLSignatures(DWORD &dwSigCount);
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PE);
	bool ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PE);
	bool IsValidHTMLFile(LPCTSTR lpFilePath);
	bool IsValidHTMLFileSEH(LPCTSTR lpFilePath);	
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool ScanForMD5AndName(LPBYTE m_szHTMLSmallFileBuffer, UINT m_iHTMLBufferOffset, LPTSTR pszVirusName, DWORD & dwSpyID, bool bRescan, FILETYPE filetype = FILE_HTML);
	bool ReadBufferFromDiffSectionsOfHTML(LPTSTR & pszVirusName, DWORD &dwSpyID, bool bRescan = false);
	
	//Added by Sagar on 22-09-2016
	//To Collect buffer for HTML samples
	DWORD ReadBufferFromDiffSectionOfHTMLAndHash(LPCTSTR pszFilePath);
	DWORD GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber);
	DWORD ZeroFill(LPCTSTR pszFilePath, DWORD dwAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile = true);

public:
	BYTE			m_HTMLHeader[0x16];
	DWORD			m_dwSectorSize;
	DWORD			m_dwDirectoryEntryOffset;
	HANDLE			m_hFileHandle;
	DWORD			m_dwHTMLFileSize;	
	BYTE			m_szHTMLFileBuffer[0xC000];	
	UINT			m_iHTMLBufferOffset;	
	TCHAR			m_szHTMLFilePath[MAX_PATH];	
	bool			m_bHTMLSignatureLoaded;	
	CMD5Scanner		m_MD5HTMLScanner;	
	CString			m_csVirusName;
};

