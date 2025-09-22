/**********************************************************************************************************
Program Name          : XMLScanner.h
Description           : Class for XML file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 6 Apr 2016
Version No            :
Special Logic Used    : This class is created for the scanning of XML type of files

Modification Log      :
1. Sanjay	          : Created Class CXMLScanner       06 - 04 - 2016
***********************************************************************************************************/

#pragma once

#include "MD5Scanner.h"
typedef DWORD(*GetByteStringHashFunc)(LPBYTE, DWORD, LPBYTE);

class CXMLScanner
{
public:
	CXMLScanner();
	virtual ~CXMLScanner();
public:
	bool LoadXMLSignatures(DWORD &dwSigCount);
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PE);
	bool ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PE);
	bool IsValidXMLFile(LPCTSTR csFilePath);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool ScanForMD5AndName(LPBYTE m_szXMLSmallFileBuffer, UINT m_iXMLBufferOffset, LPTSTR pszVirusName, DWORD & dwSpyID, bool bRescan, FILETYPE filetype = FILE_XML);
	bool ReadBufferFromDiffSectionsOfXML(LPTSTR & pszVirusName, DWORD &dwSpyID, bool bRescan);
	//Added by Sagar on 22-09-2016
	//To Collect buffer for  XML samples
	DWORD GetScriptBufferHashforHTML(LPCTSTR pszFilePath);
	DWORD GetScriptBufferHashforXML(LPCTSTR pszFilePath);		DWORD ReadBufferFromDiffSectionOfXMLAndHash(LPCTSTR pszFilePath);
	DWORD GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber);
	DWORD ZeroFill(LPCTSTR pszFilePath, DWORD dwStartAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile = true);

public:
	BYTE			m_XMLHeader[0xF];
	DWORD			m_dwSectorSize;
	DWORD			m_dwDirectoryEntryOffset;
	HANDLE			m_hFileHandle;
	DWORD			m_dwFileSize;
	BYTE			m_szXMLFileBuffer[49152];
	UINT			m_iXMLBufferOffset;
	TCHAR			m_szXMLFilePath[MAX_PATH];
	bool			m_bXMLSignatureLoaded;
	CMD5Scanner		m_MD5XMLScanner;
	CString			m_csVirusName;
	
	GetByteStringHashFunc 	m_pGetbyteStrHash;
	HINSTANCE 				m_hInstLibrary;

};

