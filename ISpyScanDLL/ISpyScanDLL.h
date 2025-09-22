// ISpyScanDLL.h : main header file for the ISpyScanDLL DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "PEScanner.h"
#include "JpegScanner.h"
#include "DocScanner.h"
#include "HTMLScanner.h"
#include "XMLScanner.h"
#include "PDFScanner.h"
#include "PHPScanner.h"
#include "WWizSplVirusScan.h"
#include "ISpyCriticalSection.h"
#include "CompressedFileScan.h"

// CISpyScanDLLApp
// See ISpyScanDLL.cpp for the implementation of this class
//

class CISpyScanDLLApp : public CWinApp
{
public:
	CISpyScanDLLApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	//Added by Vilas on 29 July 2015 to Get WardWiz Product Path from Registry
	void GetWardWizPath(LPTSTR lpszWardWizPath);

	bool LoadSignatures(LPTSTR lpFilePath, DWORD &dwSigCount);
	bool UnLoadSignatures();
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false);
	DWORD GetBytesOfBufferSEH(LPCTSTR pszFilePath, LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &dwExactBufferSize, DWORD dwKey);
	bool ReloadSettings(DWORD dwType, bool bValue);
	bool ReadHeuristicScanStatus();
	//Added by Nihar Deshpande
	//On 16-Sept-2016
	DWORD ReadBufferForExploitHashGen(LPCTSTR pszFilePath); 
	
	//Added by Sagar on 16-09-2016
	//To Collect buffer for HTML and XML samples
	DWORD GetScriptBufferHashforHTML(LPCTSTR pszFilePath);
	DWORD GetScriptBufferHashforXML(LPCTSTR pszFilePath);	
	
	// Added by Sukanya on 26-09-2016
	// For Getting buffer for hashes
	DWORD GetJPEGFileBuffer(LPCTSTR pszFilePath);

	//Added by Shodhan on 14 sept 2016 to Read Buffer for generating PHP hash.
	DWORD GetScriptBufferHashPHP(LPCTSTR pszFilePath);

	//Added by Pradip on 14 sept 2016 to Read Buffer for generating hash for Doc.
	DWORD GetDocBufferHash(LPCTSTR pszFilePath);

public:
	CPEScanner*				m_pPEScanner;
	CJPegScanner*			m_pJPEGScanner;
	CDocScanner*			m_pDOCScanner;
	CHTMLScanner*			m_pHTMLScanner;
	CXMLScanner*			m_pXMLScanner;
	CPDFScanner*			m_pPDFScanner;
	CPHPScanner*			m_pPHPScanner;
	CCompressedFileScan*	m_pCompressFileScanner;
	CWWizSplVirusScan*		m_pSplVirusScan;
	CISpyCriticalSection	m_objCriticalSection;
	bool					m_bIsHeuScan;
	CString					g_csRegKeyPath;
	CITinRegWrapper			m_objReg;
	DECLARE_MESSAGE_MAP()
};
