/**********************************************************************************************************
Program Name          : PDFScanner.h
Description           : Class for PDF file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 12 Apr 2016
Version No            :
Special Logic Used    :

Modification Log      :
1. Sanjay	          : Created Class CPDFScanner       12-04-2016
***********************************************************************************************************/

#pragma once

#include "MD5Scanner.h"

typedef DWORD(*GetBytesStringHashFunc)(LPBYTE, DWORD, LPBYTE);
/***************************************************************************************************
*  Class Name	  : CPDFScanner
*  Description    : PDF file scanner class
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 12 Apr 2016
****************************************************************************************************/
class CPDFScanner
{
public:
	CPDFScanner();
	virtual ~CPDFScanner();

public:
	bool LoadPDFSignatures(DWORD &dwSigCount);
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PDF);
	bool ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PDF);
	bool IsValidPDFFile(CString szFilePath);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool ReadBufferFromDiffSectionsOfPDF(LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan);
	//Functions added by Nihar On 27-09-16
	//These Functions are used to generate hash , make chunks and fill zero

	DWORD ReadBufferFromDiffSectionsOfPDFAddHash(LPCTSTR pszFilePath);
	DWORD GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber);
	DWORD ZeroFill(LPCTSTR pszFilePath, DWORD dwAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile = true);
public:
	HANDLE			m_hPDFFileHandle;
	DWORD			m_dwFileSize;
	DWORD			m_dwXrefAddress;
	DWORD			m_dwPDFBufferOffset;
	TCHAR			m_szPDFFilePath[MAX_PATH];
	bool			m_bPDFSignatureLoaded;
	CMD5Scanner		m_MD5PDFScanner;
	CString			m_csVirusName;
	//Variables added by Nihar On 27-09-16 27 Sept 2016
	GetBytesStringHashFunc		m_pGetBytesStringHash;
	HINSTANCE					m_hInstLibraryForHashDLL;
	DWORD						dwBufferSize;
	BYTE						bBufferHash[0x10] ;
	
};

