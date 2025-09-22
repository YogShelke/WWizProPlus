#pragma once
#include "PEScanner.h"
#include "JpegScanner.h"
#include "DocScanner.h"
#include "HTMLScanner.h"
#include "XMLScanner.h"
#include "PDFScanner.h"
#include "PHPScanner.h"
#include "WWizSplVirusScan.h"

class CCompressedFileScan
{
protected:
	typedef DWORD(*UNZIPFILE)			(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount);

	UNZIPFILE			UnzipFile;
	HMODULE				m_hZip;

public:
	CCompressedFileScan();
	CCompressedFileScan(CPEScanner *pPEScanner, CJPegScanner *pJPEGScanner, CDocScanner *pDOCScanner, CHTMLScanner *pHTMLScanner, CXMLScanner* pXMLScanner, CPDFScanner* pPDFScanner, CPHPScanner*pPHPScanner, CWWizSplVirusScan* pSplVirusScan);
	virtual ~CCompressedFileScan();
	bool ScanArchiveFile(LPTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, bool bIsHeuScan = false);
	bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false, bool bIsHeuScan = false, FILETYPE filetype = FILE_ARCHIVE);
	bool IsValidArchiveFile(LPCTSTR szFilePath);
	FILETYPE GetFileType(LPCTSTR pszFilePath, DWORD &dwSubType, DWORD &dwVersion);
	bool ScanZipFiles(TCHAR *pZipPath, TCHAR *pszVirusName, DWORD dwSpyID, bool &bModified, DWORD &dwCount);
	bool LoadRequiredDLLs();

private:
	HANDLE	m_hFileHandle;
	CPEScanner*				m_pPEScanner;
	CJPegScanner*			m_pJPEGScanner;
	CDocScanner*			m_pDOCScanner;
	CHTMLScanner*			m_pHTMLScanner;
	CXMLScanner*			m_pXMLScanner;
	CPDFScanner*			m_pPDFScanner;
	CPHPScanner*			m_pPHPScanner;
	CCompressedFileScan*	m_pCompressFileScanner;
	CWWizSplVirusScan*		m_pSplVirusScan;
};

