#include "stdafx.h"
#include "ISpyScanDLL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT _declspec(dllimport)
#define DLLEXPORT _declspec(dllexport)

BEGIN_MESSAGE_MAP(CISpyScanDLLApp, CWinApp)
END_MESSAGE_MAP()

/***************************************************************************************************
*  Function Name  : CISpyScanDLLApp
*  Description    : Cont'r
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
CISpyScanDLLApp::CISpyScanDLLApp() :
m_pPEScanner(NULL)
, m_pJPEGScanner(NULL)
, m_pDOCScanner(NULL)
, m_pHTMLScanner(NULL)
, m_pXMLScanner(NULL)
, m_pPDFScanner(NULL)
, m_pPHPScanner(NULL)
, m_pCompressFileScanner(NULL)
, m_pSplVirusScan(NULL)
, m_bIsHeuScan(true)
{
}

CISpyScanDLLApp theApp;

/***************************************************************************************************
*  Function Name  : InitInstance
*  Description    : Function to Initialize variables, which gets called when application loads the DLL.
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
BOOL CISpyScanDLLApp::InitInstance()
{
	CWinApp::InitInstance();
	g_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
	ReadHeuristicScanStatus();
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : ExitInstance()
*  Description    : Release resources used by this DLL
*  Author Name    : Vilas
*  Date			  :	17-Mar-2015
****************************************************************************************************/
int CISpyScanDLLApp::ExitInstance()
{
	return 0;
}

/***************************************************************************************************
*  Function Name  : LoadSignatures
*  Description    : Function to load signatures
*  Author Name    : Ram
*  SR_NO		  : 
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool LoadSignatures(LPTSTR pszFilePath, DWORD &dwSigCount)
{
	return theApp.LoadSignatures(pszFilePath, dwSigCount);
}

/***************************************************************************************************
*  Function Name  : ReloadSettings
*  Description    : Function to Reload Settings
*  Author Name    : Yogeshwar Rasal
*  Date			  :	15 Sep 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool ReloadSettings(DWORD dwType, bool bValue)
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.ReloadSettings(dwType, bValue);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in ReloadSettings SCANDLL", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ReloadSettings
*  Description    : Function to reload settings
*  Author Name    : Yogeshwar Rasal
*  Date			  :	15 Sep 2016
****************************************************************************************************/
bool CISpyScanDLLApp::ReloadSettings(DWORD dwType, bool bValue)
{
	bool bReturn = false;
	try
	{
		switch (dwType)
		{
		case HEURISTICSCAN:
			ReadHeuristicScanStatus();
			bReturn = true;
			break;
		default:
			bReturn = false;
			break;
		}
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::ReloadSettings", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : UnLoadSignatures
*  Description    : Function to Unload signatures
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool UnLoadSignatures()
{
	return theApp.UnLoadSignatures();
}

/***************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Exported function to scan file.
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan = false)
{
	bool bReturn = false;
	__try
	{
		theApp.m_objCriticalSection.Lock();
		bReturn = theApp.ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan);
		theApp.m_objCriticalSection.Unlock();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		theApp.m_objCriticalSection.Unlock();
		AddLogEntry(L"### Exception in ScanFile, FilePath: %s", pszFilePath, 0, true, SECONDLEVEL);
		return bReturn;
	}
	return bReturn;
}

/****************************************************************************************************************
*  Function Name  : ScanFile
*  Description    : Function in application class to scan single file.
*  Author Name    : Ram, Sanjay
*  SR_NO		  :
*  Date			  :	20 Mar 2015, 
					updated with one more file type (DOC) introduced and validated on 19 Mar 2016,
					updated with one more file type (HTML) introduced and validated on 31 Mar 2016,
					updated with one more file type (XML) introduced and validated on 6 Apr 2016,
					updated with one more file type (PDF) introduced and validated on 15 Apr 2016,
					updated with one more file type (PHP) introduced and validated on 29 Apr 2016
*****************************************************************************************************************/
bool CISpyScanDLLApp::ScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan)
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
				//Commented by Nihar
				//if file is 64 bit then no  need to scan.
				//if (m_pPEScanner->IS64Bit())
				//{
				//return bReturn;
				//}
				bReturn = m_pPEScanner->ScanFile(pszFilePath, pszVirusName, dwSpyID, bRescan, m_bIsHeuScan);
				return bReturn;
			}
		}

		/*
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

		//check whether it's a valid PHP file
		if (m_pCompressFileScanner != NULL)
		{
			if (m_pCompressFileScanner->IsValidArchiveFile(pszFilePath))
			{
				bReturn = m_pCompressFileScanner->ScanArchiveFile((LPTSTR)pszFilePath, pszVirusName, dwSpyID, bRescan, FILE_ARCHIVE);
				return bReturn;
			}
		}
		*/

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
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::ScanFile, File: %s", pszFilePath, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************************
*  Function Name  : LoadSignatures
*  Description    : Function to load signature database.
*  Author Name    : Ram, Sanjay
*  SR_NO		  :
*  Date			  :	20 Mar 2015, 
					updated with creation of new object of new file type (DOC) on 19 Mar 2016,
					updated with creation of new object of new file type (HTML) on 31 Mar 2016,
					updated with creation of new object of new file type (XML) on 6 Apr 2016,
					updated with creation of new object of new file type (PDF) on 15 Apr 2016,
					updated with creation of new object of new file type (PHP) on 15 Apr 2016
************************************************************************************************************/
bool CISpyScanDLLApp::LoadSignatures(LPTSTR lpFilePath, DWORD &dwTotalSigCount)
{
	bool bReturn = false;
	try
	{
		// if m_pPEScanner exists, free it and then give it fresh memory
		if (m_pPEScanner != NULL)
		{
			delete m_pPEScanner;
			m_pPEScanner = NULL;
		}

		m_pPEScanner = new CPEScanner();
		if (m_pPEScanner != NULL)
		{
			DWORD dwSigCount = 0x00;
			m_pPEScanner->m_bPESignatureLoaded = m_pPEScanner->LoadPESignatures(lpFilePath, dwSigCount);
			dwTotalSigCount += dwSigCount;
		}

		// if m_pJPEGScanner exists, free it and then give it fresh memory
		if (m_pJPEGScanner != NULL)
		{
			delete m_pJPEGScanner;
			m_pJPEGScanner = NULL;
		}

		m_pJPEGScanner = new CJPegScanner();
		if (m_pJPEGScanner)
		{
			DWORD dwSigCount = 0x00;
			m_pJPEGScanner->m_bJPEGSignatureLoaded = m_pJPEGScanner->LoadJPEGSignatures(dwSigCount);
			dwTotalSigCount += dwSigCount;
		}

		// if m_pDOCScanner exists, free it and then give it fresh memory
		if (m_pDOCScanner != NULL)
		{
			delete m_pDOCScanner;
			m_pDOCScanner = NULL;
		}

		m_pDOCScanner = new CDocScanner();
		if (m_pDOCScanner)
		{
			DWORD dwSigCount = 0x00;
			m_pDOCScanner->m_bDOCSignatureLoaded = m_pDOCScanner->LoadDOCSignatures(dwSigCount);
			dwTotalSigCount += dwSigCount;
		}

		// if m_pHTMLScanner exists, free it and then give it fresh memory
		if (m_pHTMLScanner != NULL)
		{
			delete m_pHTMLScanner;
			m_pHTMLScanner = NULL;
		}

		m_pHTMLScanner = new CHTMLScanner();
		if (m_pHTMLScanner)
		{
			DWORD dwSigCount = 0x00;
			m_pHTMLScanner->m_bHTMLSignatureLoaded = m_pHTMLScanner->LoadHTMLSignatures(dwSigCount);
			dwTotalSigCount += dwSigCount;
		}

		// if m_pXMLScanner exists, free it and then give it fresh memory
		if (m_pXMLScanner != NULL)
		{
			delete m_pXMLScanner;
			m_pXMLScanner = NULL;
		}

		m_pXMLScanner = new CXMLScanner();
		if (m_pXMLScanner)
		{
			DWORD dwSigCount = 0x00;
			m_pXMLScanner->m_bXMLSignatureLoaded = m_pXMLScanner->LoadXMLSignatures(dwSigCount);
			dwTotalSigCount += dwSigCount;
		}

		// if m_pPDFScanner exists, free it and then give it fresh memory
		if (m_pPDFScanner != NULL)
		{
			delete m_pPDFScanner;
			m_pPDFScanner = NULL;
		}

		m_pPDFScanner = new CPDFScanner();
		if (m_pPDFScanner)
		{
			DWORD dwSigCount = 0x00;
			m_pPDFScanner->m_bPDFSignatureLoaded = m_pPDFScanner->LoadPDFSignatures(dwSigCount);
			dwTotalSigCount += dwSigCount;
		}

		// if m_pPHPScanner exists, free it and then give it fresh memory
		if (m_pPHPScanner != NULL)
		{
			delete m_pPHPScanner;
			m_pPHPScanner = NULL;
		}

		m_pPHPScanner = new CPHPScanner();
		if (m_pPHPScanner)
		{
			DWORD dwSigCount = 0x00;
			m_pPHPScanner->m_bPHPSignatureLoaded = m_pPHPScanner->LoadPHPSignatures(dwSigCount);
			dwTotalSigCount += dwSigCount;
		}

		//This must be last entry to create object as because it required all created objects for its scanning.
		// if m_pPHPScanner exists, free it and then give it fresh memory
		if (m_pCompressFileScanner != NULL)
		{
			delete m_pCompressFileScanner;
			m_pCompressFileScanner = NULL;
		}

		m_pCompressFileScanner = new CCompressedFileScan(m_pPEScanner, m_pJPEGScanner, m_pDOCScanner, m_pHTMLScanner, m_pXMLScanner, m_pPDFScanner, m_pPHPScanner, m_pSplVirusScan);
		if (!m_pCompressFileScanner)
		{
			AddLogEntry(L"### Failed to create object of CCompressedFileScan", 0, 0, true, SECONDLEVEL);
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::LoadSignatures", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return m_pPEScanner->m_bPESignatureLoaded || 
		m_pJPEGScanner->m_bJPEGSignatureLoaded ||
		m_pDOCScanner->m_bDOCSignatureLoaded ||
		m_pHTMLScanner->m_bHTMLSignatureLoaded ||	
		m_pXMLScanner->m_bXMLSignatureLoaded ||
		m_pPDFScanner->m_bPDFSignatureLoaded ||
		m_pPHPScanner->m_bPHPSignatureLoaded ? true : false;
}

/************************************************************************************************************
*  Function Name  : UnLoadSignatures
*  Description    : Function to unload signatures
*  Author Name    : Ram, Sanjay
*  SR_NO		  :
*  Date			  :	20 Mar 2015, 
					updated with unloading of database of file type, DOC,
					updated with unloading of database of file type, JPEG,
					updated with unloading of database of file type, HTML,
					updated with unloading of database of file type, XML,
					updated with unloading of database of file type, PDF,
					updated with unloading of database of file type, PHP

*  Date Modified  :	20 Mar 2016, 15 Apr 2016, 29 Apr 2016
*************************************************************************************************************/
bool CISpyScanDLLApp::UnLoadSignatures()
{
	if (m_pPEScanner != NULL)
	{
		delete m_pPEScanner;
		m_pPEScanner = NULL;
	}

	if (m_pJPEGScanner != NULL)
	{
		delete m_pJPEGScanner;
		m_pJPEGScanner = NULL;
	}

	if (m_pDOCScanner != NULL)
	{
		delete m_pDOCScanner;
		m_pDOCScanner = NULL;
	}

	if (m_pHTMLScanner != NULL)
	{
		delete m_pHTMLScanner;
		m_pHTMLScanner = NULL;
	}

	if (m_pXMLScanner != NULL)
	{
		delete m_pXMLScanner;
		m_pXMLScanner = NULL;
	}

	if (m_pPDFScanner != NULL)
	{
		delete m_pPDFScanner;
		m_pPDFScanner = NULL;
	}

	if (m_pPHPScanner != NULL)
	{
		delete m_pPHPScanner;
		m_pPHPScanner = NULL;
	}

	if (m_pSplVirusScan != NULL)
	{
		delete m_pSplVirusScan;
		m_pSplVirusScan = NULL;
	}

	if (m_pCompressFileScanner != NULL)
	{
		delete m_pCompressFileScanner;
		m_pCompressFileScanner = NULL;
	}

	return true;
}


/***************************************************************************************************
*  Function Name  : GetWardWizPath()
*  Description    : Gets WardWiz Product Path from Registry
*  Author Name    : Vilas
*  Date			  :	29-July-2015
****************************************************************************************************/
void CISpyScanDLLApp::GetWardWizPath(LPTSTR lpszWardWizPath)
{
	try
	{
		CString csWardWizPath = GetWardWizPathFromRegistry();

		if (csWardWizPath.GetLength())
			swprintf_s(lpszWardWizPath, 511, L"%s", csWardWizPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::GetWardwizPath", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetBytesOfBuffer()
*  Description    : Gets buffer from File
*  Author Name    : Nitin K
*  Date			  :	25-August-2016
****************************************************************************************************/
extern "C" DLLEXPORT DWORD GetBytesOfBuffer(LPCTSTR pszFilePath, LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &dwExactBufferSize, DWORD dwKey)
{

	DWORD	dwRet = 0x00;

	__try
	{
		dwRet = theApp.GetBytesOfBufferSEH(pszFilePath, lpBuffer, dwBufferSize, dwExactBufferSize, dwKey);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::GetBytesOfBuffer", 0, 0, true, SECONDLEVEL);
		
		dwRet = 0xFF;
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetBytesOfBufferSEH()
*  Description    : Gets buffer from PE File and generate Hash
*  Author Name    : Nitin & Vilas
*  Date			  :	25-August-2016
****************************************************************************************************/
DWORD CISpyScanDLLApp::GetBytesOfBufferSEH(LPCTSTR pszFilePath, LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &dwExactBufferSize, DWORD dwKey)
{
	DWORD	dwRet = 0x01;

	try
	{

		if (!m_pPEScanner)
		{
			m_pPEScanner = new CPEScanner();
		}

		
		if (!m_pPEScanner)
		{
			m_pPEScanner = NULL;
			return dwRet;
		}

		if (theApp.m_pPEScanner->IsValidPEFile(pszFilePath))
		{
			wcscpy_s(theApp.m_pPEScanner->m_szFilePath, (MAX_PATH-1), pszFilePath );
			dwRet = theApp.m_pPEScanner->ReadBufferFromDiffSectionsForUtility(lpBuffer, dwBufferSize, dwExactBufferSize);
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::LoadSignatures", 0, 0, true, SECONDLEVEL);
		dwRet = 0xFF;
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : ReadHeuristicScanStatus
*  Description    : Function to get Registry for Heuristic Scan.
*  Author Name    : Yogeshwar Rasal
*  Date			  :	15 Sep 2016
****************************************************************************************************/
bool CISpyScanDLLApp::ReadHeuristicScanStatus()
{
	try
	{
		//Get here registry setting for Heuristic Scan.
		DWORD dwIsHeuScan = 0x01;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csRegKeyPath.GetBuffer(), L"dwHeuScan", dwIsHeuScan) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwHeuScan in ReadHeuristicScanStatus", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (dwIsHeuScan == 0x00)
		{
			m_bIsHeuScan = false;
		}
		else if (dwIsHeuScan == 0x01)
		{
			m_bIsHeuScan = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed in ReadHeuristicScanStatus", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}
/***************************************************************************************************
*  Function Name  : ReadBufferForExploitHashGen
*  Description    : Exported function to read buffer and generate hash from a file.
*  Author Name    : Nihar
*  SR_NO		  :
*  Date			  :	16 09 2016
****************************************************************************************************/
extern "C" DLLEXPORT DWORD ReadBufferForExploitHashGen(LPCTSTR pszFilePath)
{
	return theApp.ReadBufferForExploitHashGen(pszFilePath);
}


/***************************************************************************************************
*  Function Name  : ReadBufferForHashGen
*  Description    : Reading Buffer to generate hashes
*  Author Name    : Nihar
*  SR_NO		  :
*  Date			  :	16 09 2016
****************************************************************************************************/
DWORD CISpyScanDLLApp::ReadBufferForExploitHashGen(LPCTSTR pszFilePath)
{
	DWORD dwReturn = 0x00;

	m_pPDFScanner = new CPDFScanner();

	try
	{
		if (m_pPDFScanner != NULL)
		{
			if (m_pPDFScanner->IsValidPDFFile(pszFilePath))
			{
				dwReturn = theApp.m_pPDFScanner->ReadBufferFromDiffSectionsOfPDFAddHash(pszFilePath);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::ReadBufferForHashGen", 0, 0, true, SECONDLEVEL);
		dwReturn = 0xFF;
	}

	return dwReturn;
}

/****************************************************************************************************************
*  Function Name  : GetScriptBufferHashforXML
*  Description    : Read Buffer for generating hash of XML files.
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  :	21 sept 2016,
*****************************************************************************************************************/

extern "C" DLLEXPORT DWORD GetScriptBufferHashforXML(LPCTSTR pszFilePath)
{
	return theApp.GetScriptBufferHashforXML(pszFilePath);

}


/****************************************************************************************************************
*  Function Name  : GetScriptBufferHashforXML
*  Description    : Read Buffer for generating hash of XML file.
*  Author Name    : Sagar
*  SR_NO		  :
*  Date			  :	21 sept 2016,
*****************************************************************************************************************/
DWORD CISpyScanDLLApp::GetScriptBufferHashforXML(LPCTSTR pszFilePath)//, vector<STRUCTHashXMLData> & m_vHashCollection)// , LPSTR &pszVirusName, DWORD &dwSpyId, bool bRescan)
{
	
	DWORD	dwRet = 0x00;
	
	try
	{

		if (!m_pXMLScanner)
		{
			m_pXMLScanner = new CXMLScanner();
		}


		if (!m_pXMLScanner)
		{
			m_pXMLScanner = NULL;
			dwRet++;

			goto Cleanup;
		}

		if (theApp.m_pXMLScanner->IsValidXMLFile(pszFilePath))
		{
			wcscpy_s(theApp.m_pXMLScanner->m_szXMLFilePath, (MAX_PATH - 1), pszFilePath);
			dwRet = theApp.m_pXMLScanner->ReadBufferFromDiffSectionOfXMLAndHash(pszFilePath);
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::GetScriptBufferHashforXML", 0, 0, true, SECONDLEVEL);
		dwRet = 0xFF;
	}


Cleanup:

	return dwRet;

}

/****************************************************************************************************************
*  Function Name  : GetScriptBufferHashforHTML
*  Description    : Read Buffer for generating hash for HTML file .
*  Author Name    : Sagar, Shodhan
*  SR_NO		  :
*  Date			  :	12 sept 2016,
*****************************************************************************************************************/

extern "C" DLLEXPORT DWORD GetScriptBufferHashforHTML(LPCTSTR pszFilePath) //LPSTR pszVirusName, DWORD &dwSpyId, bool bRescan)
{

	return theApp.GetScriptBufferHashforHTML(pszFilePath);
}

/****************************************************************************************************************
*  Function Name  : GetScriptBufferHashforHTML
*  Description    : Read Buffer for generating hash of HTML file.
*  Author Name    : Sagar, Shodhan
*  SR_NO		  :
*  Date			  :	12 sept 2016,
*****************************************************************************************************************/

DWORD CISpyScanDLLApp::GetScriptBufferHashforHTML(LPCTSTR pszFilePath)// , LPSTR &pszVirusName, DWORD &dwSpyId, bool bRescan)
{
	//return 1;
	DWORD	dwRet = 0x00;
	//CHTMLScanner cHTMLScanner;
	try
	{

		if (!m_pHTMLScanner)
		{
			m_pHTMLScanner = new CHTMLScanner();
		}


		if (!m_pHTMLScanner)
		{
			m_pHTMLScanner = NULL;
			dwRet++;

			goto Cleanup;
		}

		if (theApp.m_pHTMLScanner->IsValidHTMLFile(pszFilePath))
		{
			wcscpy_s(theApp.m_pHTMLScanner->m_szHTMLFilePath, (MAX_PATH - 1), pszFilePath);
			dwRet = theApp.m_pHTMLScanner->ReadBufferFromDiffSectionOfHTMLAndHash(pszFilePath);

		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::GetScriptBufferHashforHTML", 0, 0, true, SECONDLEVEL);
		dwRet = 0xFF;
	}


Cleanup:

	return dwRet;

}


/***************************************************************************************************
*  Function Name  : GetFileBuffer
*  Description    : Read Buffer for generating hash 
*  Author Name    : Sukanya
*  SR_NO		  : 
*  Date			  :	16 Sept 2016
****************************************************************************************************/
extern "C" DLLEXPORT DWORD GetJPEGFileBuffer(LPCTSTR pszFilePath)
{
	return theApp.GetJPEGFileBuffer(pszFilePath);
}

DWORD CISpyScanDLLApp::GetJPEGFileBuffer(LPCTSTR pszFilePath)
{
	DWORD	dwRet = 0x00;
	CString cFile = pszFilePath;
	
	try
	{	
		if (!m_pJPEGScanner)
		{
			m_pJPEGScanner = new CJPegScanner();
		}

		if (!m_pJPEGScanner)
		{
			m_pJPEGScanner = NULL;
			++dwRet;
			goto Cleanup;
		}

		if (m_pJPEGScanner->IsValidJPEGFile(pszFilePath))
		{		
			wcscpy_s(m_pJPEGScanner->m_szJPEGFilePath, (MAX_PATH - 1), pszFilePath);
			dwRet = m_pJPEGScanner->ReadBufferFromDiffSectionsOfJPEGAddScanHash(pszFilePath);
			
			
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::GetJPEGFileBuffer, File: %s", pszFilePath, 0, true, SECONDLEVEL);
		dwRet = 0xFF;
	}

Cleanup:

	return dwRet;
}

/****************************************************************************************************************
*  Function Name  : GetScriptBufferHashPHP
*  Description    : Read Buffer for generating PHP hash.
*  Author Name    : Shodhan
*  SR_NO		  :
*  Date			  :	14 sept 2016,
*****************************************************************************************************************/
extern "C" DLLEXPORT DWORD GetScriptBufferHashPHP(LPCTSTR pszFilePath)
{
	return theApp.GetScriptBufferHashPHP(pszFilePath);
}


/****************************************************************************************************************
*  Function Name  : GetScriptBufferHashPHP
*  Description    : Read Buffer for generating hash.
*  Author Name    : Shodhan
*  SR_NO		  :
*  Date			  :	14 sept 2016,
*****************************************************************************************************************/

DWORD CISpyScanDLLApp::GetScriptBufferHashPHP(LPCTSTR pszFilePath)
{
	DWORD	dwRet = 0x00;
	try
	{
		if (!m_pPHPScanner)
		{
			m_pPHPScanner = new CPHPScanner();
		}

		if (!m_pPHPScanner)
		{
			m_pPHPScanner = NULL;
			dwRet++;
			goto Cleanup;
		}
		
		if (m_pPHPScanner != NULL)
		{
			if (m_pPHPScanner->IsValidPHPFile(pszFilePath))
			{
				m_objCriticalSection.Unlock();
				wcscpy_s(theApp.m_pPHPScanner->m_szPHPFilePath, (MAX_PATH - 1), pszFilePath);
				dwRet = theApp.m_pPHPScanner->ReadBufferFromDiffSectionOfPHPAndHash(pszFilePath);
				if (dwRet != 0x00)
				{
					dwRet++;
					goto Cleanup;
				}
			
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::LoadSignatures", 0, 0, true, SECONDLEVEL);
		dwRet = 0xFF;
	}

Cleanup:
	return dwRet;
}


/****************************************************************************************************************
*  Function Name  : GetDocBufferHash
*  Description    : Read Buffer for generating hash.
*  Author Name    : Pradip
*  SR_NO		  :
*  Date			  :	16 sept 2016,
*****************************************************************************************************************/
extern "C" DLLEXPORT DWORD GetDocBufferHash(LPCTSTR pszFilePath) 
{
	return theApp.GetDocBufferHash(pszFilePath);
}


/***********************************************************************************************************
*  Function Name  : GetDocBufferHash
*  Description    :Read Buffer for generating hash of DOC file.
*  Author Name    : Pradip
*  SR_NO		  :
*  Date			  :	16 Sept 2016
************************************************************************************************************/
DWORD CISpyScanDLLApp::GetDocBufferHash(LPCTSTR pszFilePath)
{
	
	DWORD	dwRet = 0x01;
	
	try
	{

		if (!m_pDOCScanner)
		{
			m_pDOCScanner = new CDocScanner();
		}


		if (!m_pDOCScanner)
		{
			m_pDOCScanner = NULL;
			dwRet++;
			
			goto Cleanup;
		}

		if (theApp.m_pDOCScanner->IsValidDOCFile(pszFilePath))
		{
			wcscpy_s(theApp.m_pDOCScanner->m_szDOCFilePath, (MAX_PATH - 1), pszFilePath);
		    dwRet = theApp.m_pDOCScanner->EnumerateDirEntriesforExtractionAndScanHash( pszFilePath);

		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanDLLApp::LoadSignatures", 0, 0, true, SECONDLEVEL);
		dwRet = 0xFF;
	}


Cleanup:

	return dwRet;
}

