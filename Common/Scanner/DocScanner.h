/**********************************************************************************************************
Program Name          : DOCScanner.h
Description           : Class for DOC file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 16th Mar 2016
Version No            :
Special Logic Used    : This class is created for the scanning of DOC type of files

Modification Log      :
1. Sanjay	          : Created Class CDOCScanner       16 - 03 - 2016
***********************************************************************************************************/

#pragma once

#include "MD5Scanner.h"

typedef DWORD(*GetByteStringHashFunc)(LPBYTE, DWORD, LPBYTE);


typedef struct _STRUCTUREDSTORAGEDIRECTORYENTRY{
	TCHAR	szEleName[0x20];
	WORD	wEleNameSize;
	BYTE	bEleType;
	BYTE	bFlag;
	DWORD	dwSIDLeft;
	DWORD	dwSIDRight;
	DWORD	dwSIDChild;
	BYTE	bSIDEleName[0x10];
	DWORD	dwUserFlags;
	BYTE	bCreationTime[0x08];
	BYTE	bModifyTime[0x08];
	DWORD	dwStartSector;
	DWORD	dwSizeLow;
	DWORD	dwSizeHigh;
} STRUCTUREDSTORAGEDIRECTORYENTRY, *PSTRUCTUREDSTORAGEDIRECTORYENTRY;



class CDocScanner
{
public:
	CDocScanner();
	virtual ~CDocScanner();

public:
	bool LoadDOCSignatures(DWORD &dwSigCount);
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR &pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PE);
	bool ScanFileSEH(LPCTSTR pszFilePath, LPTSTR &pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PE);
	bool IsValidDOCFile(LPCTSTR szFilePath);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool EnumerateDirEntriesforExtractionAndScan(LPTSTR & lpszVirusName, DWORD &dwVirusID, bool bRescan, bool &bRepairable);

	//Addded By Pradip on Dated 16-09-2016 
	// Function to scan each and every DOC file from the directory and generate chunks and Fill with zeros to specified location of chunk in file
	DWORD EnumerateDirEntriesforExtractionAndScanHash(LPCTSTR pszFilePath);
	DWORD GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber);
	DWORD ZeroFill(LPCTSTR pszFilePath, DWORD dwAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile = true);	

public:
	BYTE					m_DocHeader[0x200];
	DWORD					m_dwSectorSize;
	DWORD					m_dwDirectoryEntryOffset;
	HANDLE					m_hFileHandle;
	DWORD					m_dwFileSize;
	TCHAR					m_szDOCFilePath[MAX_PATH];
	bool					m_bDOCSignatureLoaded;
	CMD5Scanner				m_MD5DOCScanner;
	GetByteStringHashFunc	m_pGetByteStringHash;
	HINSTANCE				m_hInstHashDll;
	CString					m_csDBFilePath;

};