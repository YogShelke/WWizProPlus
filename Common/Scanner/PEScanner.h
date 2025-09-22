/**********************************************************************************************************
Program Name          : PEScanner.h
Description           : Class for scanning Portable Executable (.exe or .dll)
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 18th Sep 2013
Version No            : 0.0.0.1
Special Logic Used    :

Modification Log      :
1. Ramkrushna           Created ISpy GUI using GDI objects       11 - 11 - 2013
***********************************************************************************************************/

#pragma once
#include "ISpyTreeManager.h"
#include "WrdwizEncDecManager.h"
#include "MD5Scanner.h"
#include "WWizIndexer.h"
#include "iTinRegWrapper.h"
#include <map>

#define MAX_BUF_SIZE			10000
#define MAX_THREAT_FILE_PATH	260 *2

typedef struct __tagThreatInfo
{
	TCHAR szThreatName[MAX_PATH];
	DWORD dwSpyID;
	bool bIsHeuDetection;
}THREATINFO, *PTHREATINFO;

typedef std::map<UINT64, THREATINFO> MAPTHREATINORMATION;

//Modified on 07 / 08 / 2015 by Vilas
typedef bool(*SCANFILEHEUR)	(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool &bISPEFile, bool &bISX64PEFile);

class CPEScanner
{
public:
	CPEScanner(void);
	virtual ~CPEScanner(void);
public:
	bool LoadPESignatures(LPCTSTR pszFilePath, DWORD &dwSigCount);
	bool LoadPESignaturesSEH(LPCTSTR pszFilePath);
	bool UnLoadHtmlSignatures();
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, bool bIsHeuScan = true);
	bool ScanFileSEH(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, bool bIsHeuScan = true);
	bool GetFileBufferNScan(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwBytesToRead, DWORD * pdwBytesRead);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool ScanFileBuffer(unsigned char *szFileBuffer, DWORD dwBytesRead, char *pszVirusName, DWORD &dwSpyID, bool bRescan = false);
	bool IsValidPEFile(LPCTSTR szFilePath);
	bool ReadBufferFromDiffSections();
	DWORD RVA2FileOffset(DWORD dwRva, WORD nSections, DWORD *pdwFileOff );
	bool LoadHeuristicScanDLL(); 	//Heuristic Scan, 	//Added by Vilas
	bool ScanFileHeur(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool &bISPEFile, bool &bISX64PEFile);//Modified on 07 / 08 / 2015 by Vilas
	void GetWardWizPath(LPTSTR lpszWardWizPath);
	bool ISAlreadyDetected(ULONG64 uiCRCValue, PTHREATINFO pThreatInfo, bool bHeuScan = true);
	DWORD ReadBufferFromDiffSectionsForUtility(LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &dwExactBufferSize);
	bool ClearIndedexes();
	bool IS64Bit();
public:
	CWrdwizEncDecManager        m_objWrdwizEncDecManager;
	CISpyTreeManager			*m_pPETreeManager;
	HANDLE						m_hFileHandle;
	DWORD						m_dwFileSize;
	IMAGE_SECTION_HEADER		m_stSectionHeader[100];
	BYTE						m_szFileBuffer[25000];
	UINT						m_iBufferOffset;
    WORD						m_NumberOfSections;
	bool						m_IS64Bit;
	TCHAR						m_szFilePath[MAX_PATH];
	DWORD						m_dwOffset2NewEXEHeader;
	bool						m_bPESignatureLoaded;
	MAPTHREATINORMATION			m_mapDetectedTheatInfo;
	DWORD						m_dwRVA[100] ;
	DWORD						m_dwVS[100] ;
	DWORD						m_dwPRD[100];
	DWORD						m_dwSRD[100] ;

	IMAGE_OPTIONAL_HEADER32		m_stImageOptionalHeader32;
	IMAGE_OPTIONAL_HEADER64		m_stImageOptionalHeader64;
	CString						m_csTmpDBFileName;	
	CMD5Scanner					m_MD5Scanner;
	CWWizIndexer				m_objWWizIndexer;
	HMODULE						m_hHeuristicScan;
	SCANFILEHEUR				m_ScanFileForHeuristic;
	CITinRegWrapper				m_objReg;
};
