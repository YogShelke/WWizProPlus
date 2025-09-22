
/**********************************************************************************************************
Program Name          : JpegScanner.h
Description           : Class for Jpeg file scanner
Author Name			  : Sanjay Khapre
Date Of Creation      : 10th Mar 2016
Version No            : 
Special Logic Used    :

Modification Log      :
1. Sanjay	          : Created Class CJpegScanner       10 - 03 - 2016
***********************************************************************************************************/

#pragma once

#include "MD5Scanner.h"

typedef struct _JFIFHeader
{
	BYTE SOI[2];          /* 00h  Start of Image Marker     */
	BYTE APP0[2];         /* 02h  Application Use Marker    */
	BYTE Length[2];       /* 04h  Length of APP0 Field      */
	BYTE Identifier[5];   /* 06h  "JFIF" (zero terminated) Id String */
	BYTE Version[2];      /* 07h  JFIF Format Revision      */
	BYTE Units;           /* 09h  Units used for Resolution */
	BYTE Xdensity[2];     /* 0Ah  Horizontal Resolution     */
	BYTE Ydensity[2];     /* 0Ch  Vertical Resolution       */
	BYTE XThumbnail;      /* 0Eh  Horizontal Pixel Count    */
	BYTE YThumbnail;      /* 0Fh  Vertical Pixel Count      */
} JFIFHEAD;



/***************************************************************************************************
*  Class Name	  : CJPegScanner
*  Description    : JPEG file scanner class
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 10 Mar 2016
****************************************************************************************************/
class CJPegScanner
{
public:
	CJPegScanner();
	~CJPegScanner();
	
public:
	bool LoadJPEGSignatures(DWORD &dwSigCount);
	//bool LoadJPEGSignaturesSEH(LPTSTR pszFilePath);
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PE);
	bool ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, FILETYPE filetype = FILE_PE);
	bool IsValidJPEGFile(LPCTSTR csFilePath);
	//bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwBytesToRead, DWORD * pdwBytesRead);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool ReadBufferFromDiffSectionsOfJPEG();

	// Added By Sukanya
	// For reading buffer and creating chunks,FillZero 
	DWORD ReadBufferFromDiffSectionsOfJPEGAddScanHash(LPCTSTR pszFilePath);
	DWORD GenerateBufferedFile(LPCTSTR pszFilePath, LPVOID pbReadBuffer, DWORD dwBufferSize, DWORD dwChunkNumber);
	DWORD ZeroFill(LPCTSTR pszFilePath, DWORD dwAddress, DWORD dwFillZeroCount, DWORD dwChunkNumber, LPCTSTR pszString, bool bCopyFile = true);

public:
	JFIFHEAD		m_JFIFHeader;
	HANDLE			m_hFileHandle;
	DWORD			m_dwFileSize;
	BYTE			m_szJPEGFileBuffer[49152];
	UINT			m_iJPEGBufferOffset;
	TCHAR			m_szJPEGFilePath[MAX_PATH];
	bool			m_bJPEGSignatureLoaded;
	CMD5Scanner		m_MD5JPEGScanner;
};

