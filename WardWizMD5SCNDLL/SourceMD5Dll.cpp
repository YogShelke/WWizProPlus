#include "stdafx.h"
#include "SourceMD5Dll.h"
#include "WrdwizEncDecManager.h"
#include "WWizCRC64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT _declspec(dllimport)
#define DLLEXPORT _declspec(dllexport)

BEGIN_MESSAGE_MAP(CMD5ScanDLLApp, CWinApp)
END_MESSAGE_MAP()

/***************************************************************************************************
*  Function Name  : CMD5ScanDLLApp
*  Description    : Cont'r
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
CMD5ScanDLLApp::CMD5ScanDLLApp():
m_dwProductID(0x00)
{
	memset(m_dwArrPESigCountPerDB, 0x00, sizeof(m_dwArrPESigCountPerDB));
	memset(m_dwArrJPGSigCountPerDB, 0x00, sizeof(m_dwArrJPGSigCountPerDB));
	memset(m_dwArrDOCSigCountPerDB, 0x00, sizeof(m_dwArrDOCSigCountPerDB));
	memset(m_dwArrHTMLSigCountPerDB, 0x00, sizeof(m_dwArrHTMLSigCountPerDB));
	memset(m_dwArrXMLigCountPerDB, 0x00, sizeof(m_dwArrXMLigCountPerDB));
	memset(m_dwArrPDFSigCountPerDB, 0x00, sizeof(m_dwArrPDFSigCountPerDB));
	memset(m_dwArrPHPSigCountPerDB, 0x00, sizeof(m_dwArrPHPSigCountPerDB));

	CWardwizLangManager	objLangManager;
	m_dwProductID = objLangManager.GetSelectedProductID();
}

/***************************************************************************************************
*  Function Name  : ~CMD5ScanDLLApp
*  Description    : Dest'r
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	11 Feb 2016
****************************************************************************************************/
CMD5ScanDLLApp::~CMD5ScanDLLApp()
{
	UnLoadSignatures();
}

CMD5ScanDLLApp theApp;

/***************************************************************************************************
*  Function Name  : InitInstance
*  Description    : Function to Initialize variables, which gets called when application loads the DLL.
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
BOOL CMD5ScanDLLApp::InitInstance()
{
	CWinApp::InitInstance();
	__try
	{
		LoadReqdLibrary();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : LoadReqdLibrary
*  Description    : Function to load the required library, which gets called when application loads the DLL.
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
void CMD5ScanDLLApp::LoadReqdLibrary()
{
	__try
	{
		m_hInstLibrary = LoadLibrary(L"VBHASH.DLL");
		if (!m_hInstLibrary)
		{
			AddLogEntry(L"### Failed to load library : VBHASH.DLL", 0, 0, true, SECONDLEVEL);
			return;
		}
		
		m_pGetbyteStrHash = (GetByteStringHashFunc)GetProcAddress(m_hInstLibrary, "GetByteStringHash");
		if (!m_pGetbyteStrHash)
		{
			AddLogEntry(L"### Failed to load function : GetByteStringHash", 0, 0, true, SECONDLEVEL);
			return;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::LoadReqdLibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : UnloadLibrary
*  Description    : Function to unload the library.
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
void CMD5ScanDLLApp::UnloadLibrary()
{
	if (m_hInstLibrary != NULL)
	{
		FreeLibrary(m_hInstLibrary);
		m_hInstLibrary = NULL;
	}
}

/***************************************************************************************************
*  Function Name  : ExitInstance()
*  Description    : Release resources used by this DLL
*  Author Name    : Vilas
*  Date			  :	17-Mar-2015
****************************************************************************************************/
int CMD5ScanDLLApp::ExitInstance()
{
	if (m_hInstLibrary != NULL)
	{
		UnloadLibrary();
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : LoadMD5Database
*  Description    : Function to load the MD5 database, a DLL function .
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool LoadMD5Database(FILETYPE filetype, DWORD &dwSigCount)
{
	bool bReturn = false;

	__try
	{
		bReturn = theApp.LoadMD5DatabaseSEH(filetype, dwSigCount);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in LoadMD5Database MD5DLL", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : LoadMD5DatabaseSEH
*  Description    : Function to load the MD5 database.
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	11 Feb 2016
					Updated with introduction of vector of new file type, DOC, to be scanned.
*  Modified Date  : 19 Mar 2016
****************************************************************************************************/
bool CMD5ScanDLLApp::LoadMD5DatabaseSEH(FILETYPE filetype, DWORD &dwSigCount)
{
	bool bResult = true;
	CTime ctBeginTime, ctEndTime;
	DWORD dwIndex = 0;
	CString csIndex;

	try
	{
		//Decrypt DB files
		ctBeginTime = CTime::GetCurrentTime();
		TCHAR szDecryptedFilePath[MAX_PATH] = { 0 };
		TCHAR tcdatafile[MAX_PATH] = { 0 }, tclogstr[MAX_PATH] = { 0 };
		DWORD	dwReadBytes = 0x00;
		BYTE strFileData[34] = { 0 }, strToCopy[32] = { 0 };
		DWORD dwFileno = 1;
		CString csFileno;
		DWORD dwTotalSigCount = 0x00;

		//Vector given memory
		//Hard coded value to load around 20 Lakh signatures.
		//in Furture this may increase.

		if (filetype == FILE_PE)
			m_vMD5PECollection.reserve(2378205);
		else if (filetype == FILE_JPG)
			m_vMD5JPEGCollection.reserve(2378205);
		else if (filetype == FILE_DOC)
			m_vMD5DOCCollection.reserve(2378205);
		else if (filetype == FILE_HTML)
			m_vMD5HTMLCollection.reserve(2378205);
		else if (filetype == FILE_XML)
			m_vMD5XMLCollection.reserve(2378205);
		else if (filetype == FILE_PDF)
			m_vMD5PDFCollection.reserve(2378205);
		else if (filetype == FILE_PHP)
			m_vMD5PHPCollection.reserve(2378205);
		
		
		//Added more DB load support ( 512 DB files )
		for (dwFileno = 1; dwFileno < 0x200; dwFileno++)
		{
			if (dwFileno >= 1 && dwFileno <= 9)
				csFileno.Format(L"00%d", dwFileno);
			else  if (dwFileno >= 10 && dwFileno <= 99)
				csFileno.Format(L"0%d", dwFileno);
			else  if (dwFileno >= 100 && dwFileno <= 999)
				csFileno.Format(L"%d", dwFileno);
			else
			{
				break;
			}

			CString csDBFilePath = GetWardWizPathFromRegistry();
			csDBFilePath += L"VBDB\\";

			if (filetype == FILE_PE)
			{
				csDBFilePath += AVPE32DBNAME;
			}
			else if (filetype == FILE_JPG)
				csDBFilePath += WRDWIZAVJPEGDBNAME;
			else if (filetype == FILE_DOC)
				csDBFilePath += WRDWIZAVDOCDBNAME;
			else if (filetype == FILE_HTML)
				csDBFilePath += WRDWIZAVHTMLDBNAME;
			else if (filetype == FILE_XML)
				csDBFilePath += WRDWIZAVXMLDBNAME;
			else if (filetype == FILE_PDF)
				csDBFilePath += WRDWIZAVPDFDBNAME;
			else if (filetype == FILE_PHP)
				csDBFilePath += WRDWIZAVPHPDBNAME;
			else
			{
				AddLogEntry(L"### The filepath is : %s", csDBFilePath, 0, true, ZEROLEVEL);
				break;
			}

			csDBFilePath += csFileno;
			csDBFilePath += L".DB";

			if (!PathFileExists(csDBFilePath))
			{
				AddLogEntry(L"### File Does not exist: %s", csDBFilePath, 0, true, ZEROLEVEL);
				break;
			}

			DWORD dwMajorVersionNo = 0x00;
			DWORD dwVersionLength = 0x00;
			CWrdwizEncDecManager        objWrdwizEncDecManager;
			if (!objWrdwizEncDecManager.DecryptDBFileData(csDBFilePath, szDecryptedFilePath, dwMajorVersionNo, dwVersionLength))
			{
				AddLogEntry(L"### Failed to call DecryptDBFileData in LoadMD5DatabaseSEH MD5DLL, File: %s", csDBFilePath, 0, true, SECONDLEVEL);
				continue;
			}

			/* This check specially to load signture database more faster, here check Major version number and to loading in different way */
			if (dwMajorVersionNo >= 0x01)
			{
				DWORD dwCurDBSigCount = 0x00;
				if (!LoadContaintFromFile(szDecryptedFilePath, filetype, dwVersionLength, dwCurDBSigCount))
				{
					AddLogEntry(L"### Failed to LoadContaintFromFile LoadMD5DatabaseSEH MD5DLL, File: %s", szDecryptedFilePath, 0, true, SECONDLEVEL);
					continue;
				}

				SetSigCountAsPerListID(filetype, dwFileno - 1, dwCurDBSigCount);
				dwTotalSigCount += dwCurDBSigCount;
			}
			else
			{
				if (!objWrdwizEncDecManager.LoadContentFromFile(szDecryptedFilePath))
				{
					AddLogEntry(L"### Failed to call LoadContentFromFile in LoadMD5DatabaseSEH MD5DLL, File: %s", szDecryptedFilePath, 0, true, SECONDLEVEL);
					continue;
				}

				//Delete temporary created file
				if (PathFileExists(szDecryptedFilePath))
				{
					SetFileAttributes(szDecryptedFilePath, FILE_ATTRIBUTE_NORMAL);
					if (!DeleteFile(szDecryptedFilePath))
					{
						AddLogEntry(L"### Failed to DeleteFile in CMD5ScanDLLApp::LoadMD5DatabaseSEH, File [%s]", szDecryptedFilePath, 0, true, SECONDLEVEL);
					}
				}

				DWORD dwCount = static_cast<DWORD>(objWrdwizEncDecManager.m_objEncDec.m_cContactsList.GetCount());

				CString csContactList;
				csContactList.Format(L">>> MD5: The count of Contactlist : %lu", dwCount);
				OutputDebugString(csContactList);

				CString csFirstEntry = NULL, csSecondEntry = NULL;
				const ContactList& contacts = objWrdwizEncDecManager.m_objEncDec.GetContacts();

				DWORD dwCurDBSigCount = 0x00;
				DWORD dwIdxForMD5Coll = 0;
				POSITION pos = contacts.GetHeadPosition();
				while (pos != NULL)
				{
					const CIspyList contact = contacts.GetNext(pos);

					csFirstEntry = contact.GetFirstEntry();

					size_t   tSize;
					char szSigOnly[1024] = { 0 };
					wcstombs_s(&tSize, szSigOnly, sizeof(szSigOnly), csFirstEntry.GetBuffer(), sizeof(szSigOnly) + 1);

					if (strlen(szSigOnly) > 0)
					{
						BYTE szMD5Data[16] = { 0 };

						char *szDummy = NULL;
						int iMD5Index = 0;
						for (int iIndex = 0; iIndex < 32;)
						{
							char szHex[3] = { 0 };
							szHex[0] = szSigOnly[iIndex];
							szHex[1] = szSigOnly[iIndex + 1];
							szHex[2] = '\0';

							szMD5Data[iMD5Index] = static_cast<BYTE>(strtol(szHex, &szDummy, 0x10));

							iMD5Index++;
							iIndex += 2;
						}

						memcpy(&theApp.m_stMD5Item.byMD5Hex, &szMD5Data, sizeof(theApp.m_stMD5Item.byMD5Hex));
						if (filetype == FILE_PE)
						{
							m_vMD5PECollection.push_back(theApp.m_stMD5Item);
							dwCurDBSigCount++;
						}
						else if (filetype == FILE_JPG)
						{
							m_vMD5JPEGCollection.push_back(theApp.m_stMD5Item);
							dwCurDBSigCount++;
						}
						else if (filetype == FILE_DOC)
						{
							m_vMD5DOCCollection.push_back(theApp.m_stMD5Item);
							dwCurDBSigCount++;
						}
						else if (filetype == FILE_HTML)
						{
							m_vMD5HTMLCollection.push_back(theApp.m_stMD5Item);
							dwCurDBSigCount++;
						}
						else if (filetype == FILE_XML)
						{
							m_vMD5XMLCollection.push_back(theApp.m_stMD5Item);
							dwCurDBSigCount++;
						}
						else if (filetype == FILE_PDF)
						{
							m_vMD5PDFCollection.push_back(theApp.m_stMD5Item);
							dwCurDBSigCount++;
						}
						else if (filetype == FILE_PHP)
						{
							m_vMD5PHPCollection.push_back(theApp.m_stMD5Item);
							dwCurDBSigCount++;
						}
						else
						{
							AddLogEntry(L"### Invalid file type in LoadMD5DatabaseSEH MD5DLL, File: %s", csDBFilePath, 0, true, SECONDLEVEL);
							return false;
						}

						dwIndex++;

						csIndex.Format(L">>> The index of data loaded is : %lu", dwIndex);
						csIndex += ",  ";
						csIndex += csFirstEntry;
						OutputDebugString(csIndex);
					}
				}

				SetSigCountAsPerListID(filetype, dwFileno - 1, dwCurDBSigCount);
				dwTotalSigCount += dwCurDBSigCount;
			}
		}

		bResult = true;

		TCHAR csNoOfSignsforPE[MAX_PATH], csNoOfSignsforJPEG[MAX_PATH], csNoOfSignsforDOC[MAX_PATH];
		TCHAR csNoOfSignsforHTML[MAX_PATH], csNoOfSignsforXML[MAX_PATH], csNoOfSignsforPDF[MAX_PATH], csNoOfSignsforPHP[MAX_PATH];

		if (filetype == FILE_PE)
		{
			dwSigCount = static_cast<DWORD>(theApp.m_vMD5PECollection.size());

			if (dwTotalSigCount != dwSigCount)
			{
				AddLogEntry(L"### Mismatched Count: The count of signatures loaded for PE is : %s", csNoOfSignsforPE, 0, true, SECONDLEVEL);
			}

			swprintf(csNoOfSignsforPE, MAX_PATH, L"%d", dwSigCount);
			AddLogEntry(L">>> MD5: The count of signatures loaded for PE is : %s", csNoOfSignsforPE, 0, true, ZEROLEVEL);
		}
		else if (filetype == FILE_JPG)
		{
			dwSigCount = static_cast<DWORD>(theApp.m_vMD5JPEGCollection.size());

			if (dwTotalSigCount != dwSigCount)
			{
				AddLogEntry(L"### Mismatched Count: The count of signatures loaded for JPEG is : %s", csNoOfSignsforPE, 0, true, SECONDLEVEL);
			}

			swprintf(csNoOfSignsforJPEG, MAX_PATH, L"%d", dwSigCount);
			AddLogEntry(L">>> MD5: The count of signatures loaded for JPEG is : %s", csNoOfSignsforJPEG, 0, true, ZEROLEVEL);
		}
		else if (filetype == FILE_DOC)
		{
			dwSigCount = static_cast<DWORD>(theApp.m_vMD5DOCCollection.size());

			if (dwTotalSigCount != dwSigCount)
			{
				AddLogEntry(L"### Mismatched Count: The count of signatures loaded for DOC is : %s", csNoOfSignsforPE, 0, true, SECONDLEVEL);
			}

			swprintf(csNoOfSignsforDOC, MAX_PATH, L"%d", dwSigCount);
			AddLogEntry(L">>> MD5: The count of signatures loaded for DOC is : %s", csNoOfSignsforDOC, 0, true, ZEROLEVEL);
		}
		else if (filetype == FILE_HTML)
		{
			dwSigCount = static_cast<DWORD>(theApp.m_vMD5HTMLCollection.size());

			if (dwTotalSigCount != dwSigCount)
			{
				AddLogEntry(L"### Mismatched Count: The count of signatures loaded for HTML is : %s", csNoOfSignsforPE, 0, true, SECONDLEVEL);
			}

			swprintf(csNoOfSignsforHTML, MAX_PATH, L"%d", dwSigCount);
			AddLogEntry(L">>> MD5: The count of signatures loaded for HTML is : %s", csNoOfSignsforHTML, 0, true, ZEROLEVEL);
		}
		else if (filetype == FILE_XML)
		{
			dwSigCount = static_cast<DWORD>(theApp.m_vMD5XMLCollection.size());

			if (dwTotalSigCount != dwSigCount)
			{
				AddLogEntry(L"### Mismatched Count: The count of signatures loaded for XML is : %s", csNoOfSignsforPE, 0, true, SECONDLEVEL);
			}

			swprintf(csNoOfSignsforXML, MAX_PATH, L"%d", dwSigCount);
			AddLogEntry(L">>> MD5: The count of signatures loaded for XML is : %s", csNoOfSignsforXML, 0, true, ZEROLEVEL);
		}
		else if (filetype == FILE_PDF)
		{
			dwSigCount = static_cast<DWORD>(theApp.m_vMD5PDFCollection.size());

			if (dwTotalSigCount != dwSigCount)
			{
				AddLogEntry(L"### Mismatched Count: The count of signatures loaded for PDF is : %s", csNoOfSignsforPE, 0, true, SECONDLEVEL);
			}

			swprintf(csNoOfSignsforPDF, MAX_PATH, L"%d", dwSigCount);
			AddLogEntry(L">>> MD5: The count of signatures loaded for PDF is : %s", csNoOfSignsforPDF, 0, true, ZEROLEVEL);
		}
		else if (filetype == FILE_PHP)
		{
			dwSigCount = static_cast<DWORD>(theApp.m_vMD5PHPCollection.size());

			if (dwTotalSigCount != dwSigCount)
			{
				AddLogEntry(L"### Mismatched Count: The count of signatures loaded for PHP is : %s", csNoOfSignsforPE, 0, true, SECONDLEVEL);
			}

			swprintf(csNoOfSignsforPHP, MAX_PATH, L"%d", dwSigCount);
			AddLogEntry(L">>> MD5: The count of signatures loaded for PHP is : %s", csNoOfSignsforPHP, 0, true, ZEROLEVEL);
		}


		ctEndTime = CTime::GetCurrentTime();

		CTimeSpan ctDiffOfTime = ctEndTime - ctBeginTime;
		CString csDiffOfTime;
		csDiffOfTime.Format(L"%d min %d sec", ctDiffOfTime.GetMinutes(), ctDiffOfTime.GetSeconds());
		AddLogEntry(L">>> MD5: Database loaded in %s ", csDiffOfTime, 0, true, SECONDLEVEL);

	}
	catch (...)
	{
		bResult = false;
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::LoadMD5DatabaseSEH", 0, 0, true, SECONDLEVEL);
	}
	return bResult;
}


/***************************************************************************************************
*  Function Name  : FindPosition
*  Description    : Function to find the position of the hash item in the vector
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	11 Feb 2016
****************************************************************************************************/
bool CMD5ScanDLLApp::FindPosition(vector<STRUCTMD5Data> & m_vMD5Collection, BYTE *byMD5Buffer, DWORD & dwIndex, DWORD & dwVirusID)
{
	bool bFound = false;
	try
	{
		if (byMD5Buffer == NULL)
		{
			return bFound;
		}

		for (unsigned int iIndex = dwIndex; iIndex < m_vMD5Collection.size(); iIndex++)
		{
			LPBYTE lpHash = m_vMD5Collection[iIndex].byMD5Hex;
			if (lpHash)
			{
				if (lpHash[0x00] == byMD5Buffer[0x00] &&
					lpHash[0x03] == byMD5Buffer[0x03])
				{

					if (memcmp(lpHash, byMD5Buffer, 0x04) == 0)
					{
						bFound = true;
						dwVirusID = dwIndex;
						break;
					}
				}
			}

			dwIndex++;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::FindPosition MD5DLL", 0, 0, true, SECONDLEVEL);
	}
	return bFound;
}


/***************************************************************************************************
*  Function Name  : ScanForMD5Data
*  Description    : Function to scan the MD5 database, a DLL function.
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool ScanForMD5Data(LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &dwVirusID, bool Rescan, FILETYPE filetype)
{
	bool bFound = false;

	__try
	{
		bFound = theApp.ScanForMD5DataSEH(lpBuffer, dwBufferSize, dwVirusID, Rescan, filetype);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in LoadMD5Database MD5DLL", 0, 0, true, SECONDLEVEL);
	}

	return bFound;
}


/********************************************************************************************************
*  Function Name  : ScanForMD5DataSEH
*  Description    : Function to scan the MD5 database, a DLL function.
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
					Updated with checking for MD5 data while scanning of vector of new file type, DOC.
*  Modified Date  : 19 Mar 2016
*********************************************************************************************************/
bool CMD5ScanDLLApp::ScanForMD5DataSEH(LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &dwVirusID, bool Rescan, FILETYPE filetype)
{
	bool bFound = false;
	BYTE byMD5Buffer[16];

	DWORD dwIndex = 0;

	try
	{
		DWORD ctBeginTime, ctEndTime;
		ctBeginTime = GetTickCount();

		// Calculate MD5Buffer
		DWORD dwStatus = theApp.CalculateMD5(lpBuffer, dwBufferSize, byMD5Buffer);

		DWORD dwVecSize = 0x00;
		switch (filetype)
		{
		case FILE_PE:
			dwVecSize = static_cast<DWORD>(theApp.m_vMD5PECollection.size());
			break;
		case FILE_JPG:
			dwVecSize = static_cast<DWORD>(theApp.m_vMD5JPEGCollection.size());
			break;
		case FILE_DOC:
			dwVecSize = static_cast<DWORD>(theApp.m_vMD5DOCCollection.size());
			break;
		case FILE_HTML:
			dwVecSize = static_cast<DWORD>(theApp.m_vMD5HTMLCollection.size());
			break;
		case FILE_XML:
			dwVecSize = static_cast<DWORD>(theApp.m_vMD5XMLCollection.size());
			break;
		case FILE_PDF:
			dwVecSize = static_cast<DWORD>(theApp.m_vMD5PDFCollection.size());
			break;
		case FILE_PHP:
			dwVecSize = static_cast<DWORD>(theApp.m_vMD5PHPCollection.size());
			break;
		default:
			AddLogEntry(L"### UnHandled case in CMD5ScanDLLApp::ScanForMD5DataSEH", 0, 0, true, SECONDLEVEL);
			break;
		}

		while (dwIndex < dwVecSize)
		{

			if (filetype == FILE_PE)
				bFound = theApp.FindPosition(theApp.m_vMD5PECollection, byMD5Buffer, dwIndex, dwVirusID);
			else
				if (filetype == FILE_JPG)
					bFound = theApp.FindPosition(theApp.m_vMD5JPEGCollection, byMD5Buffer, dwIndex, dwVirusID);
				else
					if (filetype == FILE_DOC)
						bFound = theApp.FindPosition(theApp.m_vMD5DOCCollection, byMD5Buffer, dwIndex, dwVirusID);
					else
						if (filetype == FILE_HTML)
							bFound = theApp.FindPosition(theApp.m_vMD5HTMLCollection, byMD5Buffer, dwIndex, dwVirusID);
						else
							if (filetype == FILE_XML)
								bFound = theApp.FindPosition(theApp.m_vMD5XMLCollection, byMD5Buffer, dwIndex, dwVirusID);
							else
								if (filetype == FILE_PDF)
									bFound = theApp.FindPosition(theApp.m_vMD5PDFCollection, byMD5Buffer, dwIndex, dwVirusID);
								else
									if (filetype == FILE_PHP)
										bFound = theApp.FindPosition(theApp.m_vMD5PHPCollection, byMD5Buffer, dwIndex, dwVirusID);


			//If found first 4 bytes then go for remaining 12 bytes.
			if (bFound)
			{
				CString csVirusID;
				csVirusID.Format(L"VirusID: %d", dwVirusID);
				AddLogEntry(L">>> Matching Secondary signature : [%s] ", csVirusID, 0, true, ZEROLEVEL);
				bFound = MatchSecondarySig(filetype, byMD5Buffer, dwIndex++, dwVirusID); //Index - 1 as because while loading adding vector starts from 1
				if (bFound)
				{
					break;
				}

				csVirusID.Format(L"VirusID: %d", dwVirusID + 1); //as we have passed -1 in above statement.
				AddLogEntry(L">>> Matched Secondary signature : [%s] ", csVirusID, 0, true, ZEROLEVEL);
			}
			else
			{
				break;
			}
		}

		ctEndTime = GetTickCount();
		DWORD ctDiffOfTime = ctEndTime - ctBeginTime;
		CString csDiffOfTime;
		csDiffOfTime.Format(L"%lu miliseconds", ctDiffOfTime);
		AddLogEntry(L">>> Data Collection in ScanForMD5Data including CalculateMD5 took %s ", csDiffOfTime, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		bFound = false;
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::ScanForMD5DataSEH", 0, 0, true, SECONDLEVEL);
	}

	return bFound;
}

/***************************************************************************************************
*  Function Name  : UnLoadSignatures
*  Description    : Function to Unload MD5 Signatures
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool UnLoadSignatures()
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.UnLoadSignatures();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in UnLoadSignatures", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : CalculateMD5
*  Description    : Function to calculate MD5, a DLL function .
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
DWORD CMD5ScanDLLApp::CalculateMD5(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpMD5Buffer)
{
	DWORD	dwReadBytes = 0x00;
	DWORD dwResult = 0x00;
	__try
	{
		if (m_pGetbyteStrHash != NULL)
		{
			dwResult = m_pGetbyteStrHash(lpBuffer, dwBufferSize, lpMD5Buffer);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return dwResult;
	}
	return dwResult;
}


/***************************************************************************************************
*  Function Name  : UnLoadSignatures
*  Description    : Function to unload signatures
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
					Updated by with clearing of DOC vector along with other file types, once scanning is done.
*  Modified Date  : 19 Mar 2016
****************************************************************************************************/
bool CMD5ScanDLLApp::UnLoadSignatures()
{
	bool bReturn = false;
	__try
	{
		m_vMD5PECollection.clear();
		m_vMD5JPEGCollection.clear();
		m_vMD5DOCCollection.clear();
		m_vMD5HTMLCollection.clear();
		m_vMD5XMLCollection.clear();
		m_vMD5PDFCollection.clear();
		m_vMD5PHPCollection.clear();

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::UnLoadSignatures", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadContaintFromFile
*  Description    : Load Signature Database with new implementation, this applies from 1.1.0.0 DB version
					and above.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	10 May 2016
****************************************************************************************************/
bool CMD5ScanDLLApp::LoadContaintFromFile(LPTSTR szFilePath, FILETYPE filetype, DWORD dwVersionLength, DWORD &dwSigCount)
{
	bool bReturn = false;
	__try
	{
		bReturn = LoadContaintFromFileSEH(szFilePath, filetype, dwVersionLength, dwSigCount);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::LoadContaintFromFile, %s", szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadContaintFromFileSEH
*  Description    : Load Signature Database with new implementation, this applies from 1.1.0.0 DB version
and above.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	10 May 2016
****************************************************************************************************/
bool CMD5ScanDLLApp::LoadContaintFromFileSEH(LPTSTR szFilePath, FILETYPE filetype, DWORD dwVersionLength, DWORD &dwSigCount)
{
	bool bReturn = false;

	HANDLE	hInputFileHandle	= NULL;
	BYTE	*bFileBuffer		= NULL;
	try
	{
		hInputFileHandle = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFileHandle == INVALID_HANDLE_VALUE)
		{
			return bReturn;
		}

		/* Get file size here */
		DWORD dwFileSize = GetFileSize(hInputFileHandle, 0);

		/* If file size if 0 return */
		if (dwFileSize == 0x00)
		{
			bReturn = false;
			goto CLEANUP;
		}

		DWORD dwBufferOffset = MAX_SIG_SIZE + 2 + dwVersionLength + 2;
		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		ULONG64 ulFileCRC = 0x00;
		DWORD dwBytesRead = 0;
		if (!ReadFile(hInputFileHandle, &ulFileCRC, sizeof(ulFileCRC), &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != sizeof(ulFileCRC))
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Read here remaining file
		dwBufferOffset += dwBytesRead;
		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		DWORD dwRemSize = dwFileSize - dwBufferOffset;
		bFileBuffer = (BYTE*)malloc(dwRemSize * sizeof(BYTE));
		
		if (bFileBuffer == NULL)
		{
			bReturn = false;
			goto CLEANUP;
		}

		dwBytesRead = 0;
		if (!ReadFile(hInputFileHandle, bFileBuffer, dwRemSize, &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != dwRemSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Check buffer CRC here
		ULONG64	ulBufCRC = 0x00;
		CWWizCRC64	objWWizCRC64;
		objWWizCRC64.CalcCRC64(ulBufCRC, bFileBuffer, dwRemSize);
		
		//This check is added as because the XML CRC mismatch.
		if (filetype != FILE_XML)
		{
			//verify here, if not matched means there is change in DB file.
			if (ulBufCRC != ulFileCRC)
			{
				CString csCRC;
				csCRC.Format(L"%04x", ulBufCRC);
				AddLogEntry(L"### CRC value not matched, Calculated CRC: [%s] , File: %s", csCRC, szFilePath, true, SECONDLEVEL);
				bReturn = false;
				goto CLEANUP;
			}
		}

		dwSigCount = 0x00;

		DWORD iBufOffset = 0x00;
		while (iBufOffset < dwRemSize)
		{
			memcpy(&m_stMD5Item.byMD5Hex, &bFileBuffer[iBufOffset], sizeof(m_stMD5Item.byMD5Hex));

			if (filetype == FILE_PE)
			{
				m_vMD5PECollection.push_back(theApp.m_stMD5Item);
				dwSigCount++;
			}
			else if (filetype == FILE_JPG)
			{
				m_vMD5JPEGCollection.push_back(theApp.m_stMD5Item);
				dwSigCount++;
			}
			else if (filetype == FILE_DOC)
			{
				m_vMD5DOCCollection.push_back(theApp.m_stMD5Item);
				dwSigCount++;
			}
			else if (filetype == FILE_HTML)
			{
				m_vMD5HTMLCollection.push_back(theApp.m_stMD5Item);
				dwSigCount++;
			}
			else if (filetype == FILE_XML)
			{
				m_vMD5XMLCollection.push_back(theApp.m_stMD5Item);
				dwSigCount++;
			}
			else if (filetype == FILE_PDF)
			{
				m_vMD5PDFCollection.push_back(theApp.m_stMD5Item);
				dwSigCount++;
			}
			else if (filetype == FILE_PHP)
			{
				m_vMD5PHPCollection.push_back(theApp.m_stMD5Item);
				dwSigCount++;
			}

			else
			{
				AddLogEntry(L"### Invalid file type in LoadMD5DatabaseSEH MD5DLL, File: %s", szFilePath, 0, true, SECONDLEVEL);
				bReturn = false;
				goto CLEANUP;
			}

			iBufOffset += 0x10;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::LoadContaintFromFile, %s", szFilePath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
CLEANUP:
	//Cleanup here memory
	if (bFileBuffer != NULL)
	{
		delete[]bFileBuffer;
		bFileBuffer = NULL;
	}

	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : GetDBDwordName
*  Description    : Function to get DB ID using Threat Index ID
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	14 Dec 2016
****************************************************************************************************/
DWORD CMD5ScanDLLApp::GetDBDwordName(FILETYPE filetype, DWORD dwIndex)
{
	DWORD dwRet = 0x00;
	__try
	{
		DWORD dwCount = 0x00;
		//Check here the Index with count mentained in list.
		for (DWORD dwListID = 0x00; dwListID < 0x200; dwListID++)
		{
			if (dwIndex < dwCount)
			{
				dwRet = dwListID;
				break;
			}

			DWORD dwSigCountPerDB= GetSigCountAsPerListID(filetype, dwListID);
			if (dwSigCountPerDB == 0x00)
				break;

			dwCount += dwSigCountPerDB;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		TCHAR szIndex[MAX_PATH] = { 0 };
		swprintf_s(szIndex, L"Index: %d", dwIndex);
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::GetDBDwordName, %s", szIndex, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : MatchSecondarySig
*  Description    : Function to match seconday DB
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date			  :	14 Dec 2016
****************************************************************************************************/
bool CMD5ScanDLLApp::MatchSecondarySig(FILETYPE filetype, LPBYTE lpFileBuffer, DWORD dwIndex, DWORD &dwVirusID)
{
	bool bReturn = false;
	HANDLE	hInputFileHandle = INVALID_HANDLE_VALUE;
	try
	{
		//Sanity check
		if (!lpFileBuffer)
		{
			return false;
		}

		DWORD  dwDBID = GetDBDwordName(filetype, dwIndex);

		CString csFileno;
		if (dwDBID >= 1 && dwDBID <= 9)
			csFileno.Format(L"00%d", dwDBID);
		else  if (dwDBID >= 10 && dwDBID <= 99)
			csFileno.Format(L"0%d", dwDBID);
		else  if (dwDBID >= 100 && dwDBID <= 999)
			csFileno.Format(L"%d", dwDBID);
		else
		{
			csFileno.Format(L"%d", dwDBID);
			AddLogEntry(L"### Unhandled DBID %s", csFileno, 0, true, SECONDLEVEL);
			return false;
		}

		CString csDBFilePath = GetWardWizPathFromRegistry();
		csDBFilePath += L"VBDB\\";

		if (filetype == FILE_PE)
		{
			csDBFilePath += AVPE32DBNAME;
		}
		else if (filetype == FILE_JPG)
			csDBFilePath += WRDWIZAVJPEGDBNAME;
		else if (filetype == FILE_DOC)
			csDBFilePath += WRDWIZAVDOCDBNAME;
		else if (filetype == FILE_HTML)
			csDBFilePath += WRDWIZAVHTMLDBNAME;
		else if (filetype == FILE_XML)
			csDBFilePath += WRDWIZAVXMLDBNAME;
		else if (filetype == FILE_PDF)
			csDBFilePath += WRDWIZAVPDFDBNAME;
		else if (filetype == FILE_PHP)
			csDBFilePath += WRDWIZAVPHPDBNAME;
		else
		{
			AddLogEntry(L"### The filepath is : %s", csDBFilePath, 0, true, ZEROLEVEL);
			return false;
		}

		csDBFilePath += csFileno;
		csDBFilePath += L".DB";

		if (!PathFileExists(csDBFilePath))
		{
			AddLogEntry(L"### File Does not exist: %s", csDBFilePath, 0, true, ZEROLEVEL);
			return false;
		}

		DWORD dwMajorVersionNo = 0x00;
		DWORD dwVersionLength = 0x00;
		TCHAR szDecryptedFilePath[MAX_PATH] = { 0 };
		CWrdwizEncDecManager        objWrdwizEncDecManager;
		if (!objWrdwizEncDecManager.DecryptDBFileData(csDBFilePath, szDecryptedFilePath, dwMajorVersionNo, dwVersionLength))
		{
			AddLogEntry(L"### Failed to call CMD5ScanDLLApp::MatchSecondarySig in LoadMD5DatabaseSEH MD5DLL, File: %s", csDBFilePath, 0, true, SECONDLEVEL);
			bReturn = false;
			goto CLEANUP;
		}

		hInputFileHandle = CreateFile(csDBFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFileHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		/* Get file size here */
		DWORD dwFileSize = GetFileSize(hInputFileHandle, 0);

		/* If file size if 0 return */
		if (dwFileSize == 0x00)
		{
			bReturn = false;
			goto CLEANUP;
		}

		//Get the actual offset of file.
		DWORD dwBufferOffset = MAX_SIG_SIZE + 0x02 + dwVersionLength + 0x02 + sizeof(ULONG64);//Sig Length + PIPE + DBVersion + PIPE + CRC.
		DWORD dwActualFileOffset = GetActualFileOffsetFromIndex(filetype, dwIndex);
		
		dwBufferOffset += (dwActualFileOffset * 0x10);

		/* If calculated offset is beyond the file size then return*/
		if (dwBufferOffset > dwFileSize)
		{
			bReturn = false;
			goto CLEANUP;
		}

		SetFilePointer(hInputFileHandle, dwBufferOffset, NULL, FILE_BEGIN);

		//Read Remaining 12 bytes
		BYTE byRemainingBytes[0x10] = { 0 };
		DWORD dwBytesRead = 0;
		if (!ReadFile(hInputFileHandle, &byRemainingBytes, sizeof(byRemainingBytes), &dwBytesRead, NULL))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (dwBytesRead != sizeof(byRemainingBytes))
		{
			bReturn = false;
			goto CLEANUP;
		}

		if (filetype == FILE_PE)
			bReturn = MatchElementUsingIndex(m_vMD5PECollection, lpFileBuffer, byRemainingBytes, dwIndex, dwVirusID);
		else if (filetype == FILE_JPG)
			bReturn = MatchElementUsingIndex(m_vMD5JPEGCollection, lpFileBuffer, byRemainingBytes, dwIndex, dwVirusID);
		else if (filetype == FILE_DOC)
			bReturn = MatchElementUsingIndex(m_vMD5DOCCollection, lpFileBuffer, byRemainingBytes, dwIndex, dwVirusID);
		else  if (filetype == FILE_HTML)
			bReturn = MatchElementUsingIndex(m_vMD5HTMLCollection, lpFileBuffer, byRemainingBytes, dwIndex, dwVirusID);
		else if (filetype == FILE_XML)
			bReturn = MatchElementUsingIndex(m_vMD5XMLCollection, lpFileBuffer, byRemainingBytes, dwIndex, dwVirusID);
		else  if (filetype == FILE_PDF)
			bReturn = MatchElementUsingIndex(m_vMD5PDFCollection, lpFileBuffer, byRemainingBytes, dwIndex, dwVirusID);
		else if (filetype == FILE_PHP)
			bReturn = MatchElementUsingIndex(m_vMD5PHPCollection, lpFileBuffer, byRemainingBytes, dwIndex, dwVirusID);
		else
		{
			bReturn = false;
			AddLogEntry(L"### UnHandled file type in CMD5ScanDLLApp::MatchSecondarySig", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		CString csIndex;
		csIndex.Format(L"Index: %d", dwIndex);
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::MatchSecondarySig, %s", csIndex, 0, true, SECONDLEVEL);
	}

CLEANUP:
	//Need to close file handle after collecting buffer
	if (hInputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInputFileHandle);
		hInputFileHandle = INVALID_HANDLE_VALUE;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : MatchElementUsingIndex
*  Description    : Function to Match the signature buffer and file MD5 buffer
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	11 Feb 2016
****************************************************************************************************/
bool CMD5ScanDLLApp::MatchElementUsingIndex(vector<STRUCTMD5Data> & vMD5Collection, BYTE *byMD5Buffer, BYTE *byDBSigBuffer, DWORD dwIndex, DWORD & dwVirusID)
{
	bool bFound = false;
	__try
	{
		//Sanity check for buffer
		if (byMD5Buffer == NULL || byDBSigBuffer == NULL)
		{
			return bFound;
		}

		//Compare 16 bytes buffer.
		if (memcmp(byMD5Buffer, byDBSigBuffer, 0x10) == 0)
		{
			bFound = true;
			dwVirusID = dwIndex;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		TCHAR szIndex[MAX_PATH] = { 0 };
		swprintf_s(szIndex, L"Index: %d", dwIndex);
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::MatchElementUsingIndex, %s", szIndex, 0, true, SECONDLEVEL);
	}
	return bFound;
}

/***************************************************************************************************
*  Function Name  : GetActualFileOffsetFromIndex
*  Description    : The index is passed as parameter is calculated when all DB files are loaded,
					now we can get the actual signature index per DB. 
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	15 Dec 2016
****************************************************************************************************/
DWORD CMD5ScanDLLApp::GetActualFileOffsetFromIndex(FILETYPE filetype, DWORD dwIndex)
{
	DWORD dwRet = 0x00;
	__try
	{
		DWORD dwCount = 0x00;
		DWORD dwListID = 0x00;
		//Check here the Index with count mentained in list.
		for (; dwListID < 0x200; dwListID++)
		{
			dwCount += GetSigCountAsPerListID(filetype, dwListID);
			if (dwIndex < dwCount)
			{
				dwRet = dwListID;
				break;
			}
		}

		//if the Index is from first DB the its actual offset.
		if (dwListID == 0)
		{
			dwRet = dwIndex;
		}
		else
		{
			dwRet = dwIndex - (dwCount - GetSigCountAsPerListID(filetype, dwListID));
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		TCHAR szIndex[MAX_PATH] = { 0 };
		swprintf_s(szIndex, L"Index: %d", dwIndex);
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::GetActualFileOffsetFromIndex, %s", szIndex, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : SetSigCountAsPerListID
*  Description    : Function to set signature count as per list ID.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	15 Dec 2016
****************************************************************************************************/
void CMD5ScanDLLApp::SetSigCountAsPerListID(FILETYPE filetype, DWORD dwListID, DWORD dwCount)
{
	DWORD dwRet = 0x00;
	__try
	{
		switch (filetype)
		{
		case FILE_PE:
			if (dwListID > 0x200)
				break;
			m_dwArrPESigCountPerDB[dwListID] = dwCount;
			break;
		case FILE_JPG:
			if (dwListID > 0x32)
				break;
			m_dwArrJPGSigCountPerDB[dwListID] = dwCount;
			break;
		case FILE_DOC:
			if (dwListID > 0x32)
				break;
			m_dwArrDOCSigCountPerDB[dwListID] = dwCount;
			break;
		case FILE_HTML:
			if (dwListID > 0x32)
				break;
			m_dwArrHTMLSigCountPerDB[dwListID] = dwCount;
			break;
		case FILE_XML:
			if (dwListID > 0x32)
				break;
			m_dwArrXMLigCountPerDB[dwListID] = dwCount;
			break;
		case FILE_PDF:
			if (dwListID > 0x32)
				break;
			m_dwArrPDFSigCountPerDB[dwListID] = dwCount;
			break;
		case FILE_PHP:
			if (dwListID > 0x32)
				break;
			m_dwArrPHPSigCountPerDB[dwListID] = dwCount;
			break;
		default:
			AddLogEntry(L"### UnHandled case in CMD5ScanDLLApp::SetSigCountAsPerListID", 0, 0, true, SECONDLEVEL);
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		TCHAR szLog[MAX_PATH] = { 0 };
		swprintf_s(szLog, L"filetype: %d, ListID: %d", filetype, dwListID);
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::SetSigCountAsPerListID, %s", szLog, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : GetSigCountAsPerListID
*  Description    : Function which returns signature count as per list ID.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date			  :	15 Dec 2016
****************************************************************************************************/
DWORD CMD5ScanDLLApp::GetSigCountAsPerListID(FILETYPE filetype, DWORD dwListID)
{
	DWORD dwRet = 0x00;
	__try
	{
		switch (filetype)
		{
		case FILE_PE:
			if (dwListID > 0x200)
				break;
			dwRet = m_dwArrPESigCountPerDB[dwListID];
			break;
		case FILE_JPG:
			if (dwListID > 0x32)
				break;
			dwRet = m_dwArrJPGSigCountPerDB[dwListID];
			break;
		case FILE_DOC:
			if (dwListID > 0x32)
				break;
			dwRet = m_dwArrDOCSigCountPerDB[dwListID];
			break;
		case FILE_HTML:
			if (dwListID > 0x32)
				break;
			dwRet = m_dwArrHTMLSigCountPerDB[dwListID];
			break;
		case FILE_XML:
			if (dwListID > 0x32)
				break;
			dwRet = m_dwArrXMLigCountPerDB[dwListID];
			break;
		case FILE_PDF:
			if (dwListID > 0x32)
				break;
			dwRet = m_dwArrPDFSigCountPerDB[dwListID];
			break;
		case FILE_PHP:
			if (dwListID > 0x32)
				break;
			dwRet = m_dwArrPHPSigCountPerDB[dwListID];
			break;
		default:
			AddLogEntry(L"### UnHandled case in CMD5ScanDLLApp::GetSigCountAsPerListID", 0 , 0, true, SECONDLEVEL);
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		TCHAR szLog[MAX_PATH] = { 0 };
		swprintf_s(szLog, L"filetype: %d, ListID: %d", filetype, dwListID);
		AddLogEntry(L"### Exception in CMD5ScanDLLApp::GetSigCountAsPerListID, %s", szLog, 0, true, SECONDLEVEL);
	}
	return dwRet;
}