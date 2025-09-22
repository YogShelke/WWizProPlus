#include "stdafx.h"
#include "CompressedFileScan.h"


/***************************************************************************************************
*  Function Name  : CCompressedFileScan
*  Description    : Cont'r
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 03rd Mar 2018
****************************************************************************************************/
CCompressedFileScan::CCompressedFileScan():
m_hFileHandle(INVALID_HANDLE_VALUE)
, m_pPEScanner(NULL)
, m_pJPEGScanner(NULL)
, m_pDOCScanner(NULL)
, m_pHTMLScanner(NULL)
, m_pXMLScanner(NULL)
, m_pPDFScanner(NULL)
, m_pPHPScanner(NULL)
, m_pCompressFileScanner(NULL)
, m_pSplVirusScan(NULL)
{
}

CCompressedFileScan::CCompressedFileScan(CPEScanner *pPEScanner, CJPegScanner *pJPEGScanner, CDocScanner *pDOCScanner, 
	CHTMLScanner *pHTMLScanner, CXMLScanner* pXMLScanner, CPDFScanner* pPDFScanner, CPHPScanner*pPHPScanner, CWWizSplVirusScan* pSplVirusScan):
m_hFileHandle(INVALID_HANDLE_VALUE)
, m_pPEScanner(NULL)
, m_pJPEGScanner(NULL)
, m_pDOCScanner(NULL)
, m_pHTMLScanner(NULL)
, m_pXMLScanner(NULL)
, m_pPDFScanner(NULL)
, m_pPHPScanner(NULL)
, m_pCompressFileScanner(NULL)
, m_pSplVirusScan(NULL)
{
	m_pPEScanner = pPEScanner;
	m_pJPEGScanner = pJPEGScanner;
	m_pDOCScanner = pDOCScanner;
	m_pHTMLScanner = pHTMLScanner;
	m_pXMLScanner = pXMLScanner;
	m_pPDFScanner = pPDFScanner;
	m_pPHPScanner = pPHPScanner;
	m_pSplVirusScan = pSplVirusScan;

	LoadRequiredDLLs();
}

/***************************************************************************************************
*  Function Name  : ~CCompressedFileScan
*  Description    : Dest'r
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 03rd Mar 2018
****************************************************************************************************/
CCompressedFileScan::~CCompressedFileScan()
{
	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
}

/***************************************************************************************************
*  Function Name  : ScanArchiveFile
*  Description    : Function to start scanning PE files, where the scanning will start only if the PE signatures are loaded
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 03rd Mar 2018
****************************************************************************************************/
bool CCompressedFileScan::ScanArchiveFile(LPTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, bool bIsHeuScan)
{
	bool bReturn = false;
	DWORD	dwZipFileCount = 0x00;
	DWORD	dwStatus = 0x00;
	TCHAR	szUnzipPath[512] = { 0 };
	bool	bZipModified = false;
	DWORD	dwModifiedCount = 0x00;

	__try
	{
		if (!UnzipFile)
			return 0;

		dwStatus = UnzipFile(pszFilePath, szUnzipPath, dwZipFileCount);
		if (!dwZipFileCount)
			return bReturn;

		if(ScanZipFiles(szUnzipPath, pszVirusName, dwSpyID, bZipModified, dwModifiedCount))
		{
			bReturn = true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excep CCompressedFileScan::ScanArchiveFile, Corrupt file: %s", pszFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function to start scanning PE files, where the scanning will start only if the PE signatures are loaded
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 03rd Mar 2018
****************************************************************************************************/
bool CCompressedFileScan::ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, bool bIsHeuScan, FILETYPE filetype)
{
	bool bReturn = false;

	try
	{
		//Check here for valid PE files
		if (m_pPEScanner != NULL)
		{
			//check here does its valid pe file
			if (m_pPEScanner->IsValidPEFile(pszFilePath))
			{
				//if file is 64 bit then no  need to scan.
				if (m_pPEScanner->IS64Bit())
				{
					return bReturn;
				}
				bReturn = m_pPEScanner->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan, bIsHeuScan);
				return bReturn;
			}
		}

		//check whether it's a valid Jpeg file
		if (m_pJPEGScanner != NULL)
		{
			if (m_pJPEGScanner->IsValidJPEGFile(pszFilePath))
			{
				bReturn = m_pJPEGScanner->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_JPG);
				return bReturn;
			}
		}

		//check whether it's a valid doc file
		if (m_pDOCScanner != NULL)
		{
			if (m_pDOCScanner->IsValidDOCFile(pszFilePath))
			{
				bReturn = m_pDOCScanner->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_DOC);
				return bReturn;
			}
		}

		//check whether it's a valid HTML file
		if (m_pHTMLScanner != NULL)
		{
			if (m_pHTMLScanner->IsValidHTMLFile(pszFilePath))
			{
				bReturn = m_pHTMLScanner->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_HTML);
				return bReturn;
			}
		}

		//check whether it's a valid XML file
		if (m_pXMLScanner != NULL)
		{
			if (m_pXMLScanner->IsValidXMLFile(pszFilePath))
			{
				bReturn = m_pXMLScanner->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_XML);
				return bReturn;
			}
		}

		//check whether it's a valid PDF file
		if (m_pPDFScanner != NULL)
		{
			if (m_pPDFScanner->IsValidPDFFile(pszFilePath))
			{
				bReturn = m_pPDFScanner->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_PDF);
				return bReturn;
			}
		}

		//check whether it's a valid PHP file
		if (m_pPHPScanner != NULL)
		{
			if (m_pPHPScanner->IsValidPHPFile(pszFilePath))
			{
				bReturn = m_pPHPScanner->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_PHP);
				return bReturn;
			}
		}

		//Please keep this scanner type @last.
		//scan here special type of viruses
		if (m_pSplVirusScan == NULL)
		{
			m_pSplVirusScan = new CWWizSplVirusScan();
		}

		if (m_pSplVirusScan != NULL)
		{
			bReturn = m_pSplVirusScan->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan) == 0x00 ? true : false;
			return bReturn;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excep CCompressedFileScan::ScanFile, Corrupt file: %s", pszFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : ScanZipFiles
Description    : Function which takes zip file as a input path starts enumeration
of files for scanning.
Author Name    : Ramkrushna Shelke
Date           : 03rd Mar 2018
****************************************************************************/
bool CCompressedFileScan::ScanZipFiles(TCHAR *pZipPath, TCHAR *pszVirusName, DWORD dwSpyID, bool &bModified, DWORD &dwCount)
{
	bool bReturn = false;
	try
	{
		AddLogEntry(L">>> CCompressedFileScan::ScanZipFiles zip file started", 0, 0, true, FIRSTLEVEL);
		TCHAR	szTemp[512] = { 0 };
		TCHAR	szFile[512] = { 0 };
		CFileFind	finder;
		bool bRescan = false;
		DWORD dwSpyID = 0;
		bool bIsHeuScan = false;
		bModified = false;
		dwCount = 0x00;
		DWORD	dwAttributes = 0;
		CString strWildcard(pZipPath);
		dwAttributes = GetFileAttributes(pZipPath);
		if (((FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY) == dwAttributes) || (FILE_ATTRIBUTE_DIRECTORY == dwAttributes) || ((FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_DIRECTORY) == dwAttributes))
		{
			//strWildcard += _T("\\*.*");
			if (strWildcard[strWildcard.GetLength() - 1] != '\\')
				strWildcard += _T("\\*.*");
			else
				strWildcard += _T("*.*");
		}
		else
		{
			wsprintf(szFile, L"%s", pZipPath);
		}

		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory())
			{
				wsprintf(szFile, L"%s", finder.GetFilePath());
				if (ScanZipFiles(szFile, pszVirusName, dwSpyID, bModified, dwCount))
				{
					bReturn = true;
				}
			}
			else
			{
				wsprintf(szFile, L"%s", finder.GetFilePath());
				//ScanData(szFile); //send file to scan.
				if (ScanFile(szFile, pszVirusName, dwSpyID, bRescan, bIsHeuScan, FILE_PHP))
				{
					//if anyone file detected as malicious in compressed file.
					//whole compressed file will be considered as malicious.
					bReturn = true;
					break;
				}

				memset(szFile, 0x00, sizeof(TCHAR) * 512);
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanner::ScanZipFiles", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : IsValidArchiveFile
*  Description    : Function to check whether the file is a valid Archine file or not.
*  Author Name    : Ramkrushna Shelke
*  Date			  : 03rd Mar 2018
****************************************************************************************************/
bool CCompressedFileScan::IsValidArchiveFile(LPCTSTR szFilePath)
{
	bool bReturn = false;
	__try
	{
		if (!szFilePath)
		{
			return bReturn;
		}

		if (m_hFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hFileHandle);
			m_hFileHandle = INVALID_HANDLE_VALUE;
		}

		DWORD	dwSubType = 0x00, dwVersion = 0x00;
		FILETYPE	fileType = FILE_NOTKNOWN;
		DWORD dwFileType = 0x00;

		fileType = GetFileType(szFilePath, dwSubType, dwVersion);

		if (fileType == FILE_ARCHIVE)
		{
			bReturn = true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CCompressedFileScan::IsValidArchiveFile, File: %s", szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : GetFileType
Description    : Getting file type.
Author Name    : Ramkrushna Shelke
Date           : 03rd Mar 2018
****************************************************************************/
FILETYPE CCompressedFileScan::GetFileType(LPCTSTR pszFilePath, DWORD &dwSubType, DWORD &dwVersion)
{
	FILETYPE	fileType = FILE_NOTKNOWN;
	try
	{
		DWORD	dwReadData = 0x00;
		DWORD	dwReadBytes = 0x00;

		m_hFileHandle = CreateFile(pszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFileHandle == INVALID_HANDLE_VALUE)
			goto Cleanup;

		DWORD dwFileSize = GetFileSize(m_hFileHandle, NULL);
		//if file size is greater than 2 MB will be skipped.
		if (dwFileSize >= 0x200000)
		{
			goto Cleanup;
		}

		ReadFile(m_hFileHandle, &dwReadData, 0x04, &dwReadBytes, NULL);

		//Checking for Compressed file means ZIP or RAR
		if ((dwReadData == 0x21726152) || ((dwReadData & 0xFFFF4B50) == dwReadData))
		{
			fileType = FILE_ARCHIVE;
			goto Cleanup;
		}

		//Checking for PE file
		if (((dwReadData & 0xFFFF5A4D) == dwReadData))
		{
			dwReadData = 0x00;
			SetFilePointer(m_hFileHandle, 0x3C, NULL, FILE_BEGIN);
			ReadFile(m_hFileHandle, &dwReadData, 0x04, &dwReadBytes, NULL);

			if (dwReadData)
			{
				SetFilePointer(m_hFileHandle, dwReadData, NULL, FILE_BEGIN);

				dwReadData = 0x00;
				ReadFile(m_hFileHandle, &dwReadData, 0x04, &dwReadBytes, NULL);

				if (dwReadData == 0x00004550)
				{
					fileType = FILE_PE;
					goto Cleanup;
				}
			}
		}
	}
	catch (...)
	{
		fileType = FILE_NOTKNOWN;
		AddLogEntry(L"### Exception in CCompressedFileScan::GetFileType, File: %s", pszFilePath, 0, true, SECONDLEVEL);
	}

Cleanup:

	if (m_hFileHandle != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileHandle);

	m_hFileHandle = INVALID_HANDLE_VALUE;

	return fileType;
}

/***************************************************************************
Function Name  : LoadRequiredDLLs
Description    : Fucntion which Load's s VBEXTRACT.DLL and calls the LoadSignature
exported function.
Author Name    : Ramkrushna Shelke
Date           : 03rd Mar 2018
****************************************************************************/
bool CCompressedFileScan::LoadRequiredDLLs()
{

	TCHAR	szModulePath[512];
	_tcscpy(szModulePath, GetWardWizPathFromRegistry());

	TCHAR	szTemp[512] = { 0 };
	memset(szTemp, 0x00, sizeof(TCHAR) * 512);
	wsprintf(szTemp, L"%sVBEXTRACT.DLL", szModulePath);

	m_hZip = LoadLibrary(szTemp);
	if (!m_hZip)
	{
		AddLogEntry(L"### LoadLibrary fails in CCompressedFileScan::LoadRequiredDLLs, FilePath: [%s]", szTemp, 0, true, ZEROLEVEL);
		m_hZip = NULL;
		return false;
	}

	UnzipFile = (UNZIPFILE)GetProcAddress(m_hZip, "UnzipFile");
	if (!UnzipFile)
	{
		AddLogEntry(L"### GetProcAddress UnzipFile fails in CCompressedFileScan::LoadRequiredDLLs, FilePath: [%s]", szTemp, 0, true, ZEROLEVEL);
		FreeLibrary(m_hZip);
		m_hZip = NULL;
		return false;
	}

	return true;
}