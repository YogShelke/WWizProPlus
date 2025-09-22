/*****************************************************************************************************************
*  Program Name: ISpyDBManipulation.cpp
*  Description: It manages data to get stored in DB with serialization process.
*  Author Name: Neha Gharge
*  Date Of Creation: 22 Jan 2014
*  Version No: 1.0.0.2
*******************************************************************************************************************/
#include "stdafx.h"
#include "ISpyDBManipulation.h"
#include "PipeConstants.h"

#define EMPTY_STRING _T("")

/**********************************************************************************************************
*  Function Name  :	CISpyDBManipulation
*  Description    :	C'tor
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0001
**********************************************************************************************************/
CISpyDBManipulation::CISpyDBManipulation(void) :
m_bUserDefFileNames(false)
{
}

CISpyDBManipulation::CISpyDBManipulation(bool bIsUserDefined)
{
	m_bUserDefFileNames = bIsUserDefined;
}


/**********************************************************************************************************
*  Function Name  :	~CISpyDBManipulation
*  Description    :	D'tor
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0002
**********************************************************************************************************/
CISpyDBManipulation::~CISpyDBManipulation(void)
{
}

/**********************************************************************************************************
*  Function Name  :	SaveDBFile
*  Description    :	According to type of module DB get save.
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0003
**********************************************************************************************************/
bool CISpyDBManipulation::SaveDBFile(ITINEMAIL_DATABASETYPE eDBType, CString csFilepath)
{
	try
	{
		int iFileVersion = 1;
		CFile wFile(csFilepath, CFile::modeCreate | CFile::modeWrite);

		CArchive arStore(&wFile, CArchive::store);

		switch (eDBType)
		{
		case RECOVER:
			m_RecoverDBEntries.SetFileVersion(iFileVersion);
			m_RecoverDBEntries.Serialize(arStore);
			break;
		case VIRUSSCAN:
			m_VirusScanDBEntries.SetFileVersion(iFileVersion);
			m_VirusScanDBEntries.Serialize(arStore);
			break;
		case SPAMFILTER:
			m_SpamFilterDBEntries.SetFileVersion(iFileVersion);
			m_SpamFilterDBEntries.Serialize(arStore);
			break;
		case CONTENTFILTER:
			m_ContentFilterDBEntries.SetFileVersion(iFileVersion);
			m_ContentFilterDBEntries.Serialize(arStore);
			break;
		case SIGNATURE:
			m_SignatureDBEntries.SetFileVersion(iFileVersion);
			m_SignatureDBEntries.Serialize(arStore);
			break;
		case REPORTS:
			m_ReportDBEntries.SetFileVersion(iFileVersion);
			m_ReportDBEntries.Serialize(arStore);
			break;
		case MALICIOUSSITES:
			iFileVersion = 3;
			m_MaliciousEntries.SetFileVersion(iFileVersion);
			m_MaliciousEntries.Serialize(arStore);
			break;
		case FOLDERLOCKER:
			m_FolderLockerDBEntries.SetFileVersion(iFileVersion);
			m_FolderLockerDBEntries.Serialize(arStore);
			break;
		}
		arStore.Close();
		wFile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::SaveDBFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ClearDBEntries
*  Description    :	Clear all DB entries as per module type
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0004
**********************************************************************************************************/
bool CISpyDBManipulation::ClearDBEntries(ITINEMAIL_DATABASETYPE eDBType)
{
	try
	{
		switch (eDBType)
		{
		case RECOVER:
			m_RecoverDBEntries.RemoveAll();
			break;
		case VIRUSSCAN:
			m_VirusScanDBEntries.RemoveAll();
			break;
		case SPAMFILTER:
			m_SpamFilterDBEntries.RemoveAll();
			break;
		case CONTENTFILTER:
			m_ContentFilterDBEntries.RemoveAll();
			break;
		case SIGNATURE:
			m_SignatureDBEntries.RemoveAll();
			break;
		case REPORTS:
			m_ReportDBEntries.RemoveAll();
			break;
		case MALICIOUSSITES:
			m_MaliciousEntries.RemoveAll();
			break;
		case FOLDERLOCKER:
			m_FolderLockerDBEntries.RemoveAll();
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::ClearDBEntries", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	LoadDBFile
*  Description    :	Load DB file as per type of module
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0005
**********************************************************************************************************/
bool CISpyDBManipulation::LoadDBFile(ITINEMAIL_DATABASETYPE eDBType, CString csFilePath)
{
	try
	{
		int iFileVersion = 1;

		if (!ClearDBEntries(eDBType))
			return false;

		if (!PathFileExists(csFilePath))
			return false;

		CFile rFile;
		if (!rFile.Open((LPCTSTR)csFilePath, CFile::modeRead | CFile::shareDenyWrite))
		{
			return false;
		}
		// Create a loading archive
		CArchive arLoad(&rFile, CArchive::load);
		switch (eDBType)
		{
		case RECOVER:
			m_RecoverDBEntries.SetFileVersion(iFileVersion);
			m_RecoverDBEntries.Serialize(arLoad);
			break;
		case VIRUSSCAN:
			m_VirusScanDBEntries.SetFileVersion(iFileVersion);
			m_VirusScanDBEntries.Serialize(arLoad);
			break;
		case SPAMFILTER:
			m_SpamFilterDBEntries.SetFileVersion(iFileVersion);
			m_SpamFilterDBEntries.Serialize(arLoad);
			break;
		case CONTENTFILTER:
			m_ContentFilterDBEntries.SetFileVersion(iFileVersion);
			m_ContentFilterDBEntries.Serialize(arLoad);
			break;
		case SIGNATURE:
			m_SignatureDBEntries.SetFileVersion(iFileVersion);
			m_SignatureDBEntries.Serialize(arLoad);
			break;
		case REPORTS:
			m_ReportDBEntries.SetFileVersion(iFileVersion);
			m_ReportDBEntries.Serialize(arLoad);
			break;
		case MALICIOUSSITES:
			m_MaliciousEntries.SetFileVersion(iFileVersion);
			m_MaliciousEntries.Serialize(arLoad);
			break;
		case FOLDERLOCKER:
			m_FolderLockerDBEntries.SetFileVersion(iFileVersion);
			m_FolderLockerDBEntries.Serialize(arLoad);
			break;
		}

		// Close the loading archive
		arLoad.Close();
		rFile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::LoadDBFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	SaveEntries
*  Description    :	SaveEntries according to type of module
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0006
**********************************************************************************************************/
bool CISpyDBManipulation::SaveEntries(DWORD dwType)
{
	bool bReturn = false;
	try
	{
		CString csRegAppPath, csFileName;
		ITINEMAIL_DATABASETYPE eDBTYPE;

		csRegAppPath = GetWardWizPathFromRegistry();

		switch (dwType)
		{
		case 0x00:	eDBTYPE = RECOVER;
			csRegAppPath = L"";
			csRegAppPath = GetQuarantineFolderPath();
			if (m_bUserDefFileNames)
			{
				csRegAppPath += L"\\";
				if (!GetNewFileName(eDBTYPE, csFileName))
				{
					AddLogEntry(L"### Failed GetNewFileName, Path: %s, FileName: %s", csRegAppPath, csFileName, true, SECONDLEVEL);
					return bReturn;
				}
			}
			else
			{
				csFileName += _T("\\VBRECOVERENTRIES.DB");
			}
			break;
		case 0x01:	eDBTYPE = VIRUSSCAN;
			csFileName = L"VBVIRUSSCANEMAIL.DB";
			break;
		case 0x02:	eDBTYPE = SPAMFILTER;
			csFileName = L"VBSPAMEMAIL.DB";
			break;
		case 0x03:	eDBTYPE = CONTENTFILTER;
			csFileName = L"VBCONTENTEMAIL.DB";
			break;
		case 0x04:	eDBTYPE = SIGNATURE;
			csFileName = L"VBSIGNATUREEMAIL.DB";
			break;
		case 0x05:	eDBTYPE = REPORTS;
			if (m_bUserDefFileNames)
			{
				csRegAppPath += L"REPORTS\\";
				if (!GetNewFileName(eDBTYPE, csFileName))
				{
					AddLogEntry(L"### Failed GetNewFileName, Path: %s, FileName: %s", csRegAppPath, csFileName, true, SECONDLEVEL);
					return bReturn;
				}
			}
			else
			{
				csFileName = L"VBREPORTS.DB";
			}
			break;
		case 0x06:	eDBTYPE = MALICIOUSSITES;
			csFileName = L"VBMALICIOUSSITES.DB";
			break;
		case 0x07: eDBTYPE = FOLDERLOCKER;
			csFileName = L"VBFOLDERLOCKERENTRIES.DB";
			break;
		default:
			AddLogEntry(L"### Invalid Entry Type in CWardwizDBManipulation::SaveEntry", 0, 0, true, SECONDLEVEL);
			return false;
		}

		csRegAppPath.Append(csFileName);
		bReturn = SaveDBFile(eDBTYPE, csRegAppPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::LoadDBFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	SaveEntries
*  Description    :	SaveEntries according to type of module
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0006
**********************************************************************************************************/
bool CISpyDBManipulation::SaveEntries(DWORD dwType, CString csDirPath)
{
	bool bReturn = false;
	try
	{
		CString csRegAppPath, csFileName;
		ITINEMAIL_DATABASETYPE eDBTYPE;
		csRegAppPath = csDirPath;
		switch (dwType)
		{
		case 0x00:	
			eDBTYPE = RECOVER;
			if (m_bUserDefFileNames)
			{
				csRegAppPath += L"\\";
				if (!GetNewFileName(eDBTYPE, csFileName))
				{
					AddLogEntry(L"### Failed GetNewFileName, Path: %s, FileName: %s", csRegAppPath, csFileName, true, SECONDLEVEL);
					return bReturn;
				}
			}
			else
			{
				csFileName += _T("\\VBRECOVERENTRIES.DB");
			}
			break;
		case 0x01:	eDBTYPE = VIRUSSCAN;
			csFileName = L"VBVIRUSSCANEMAIL.DB";
			break;
		case 0x02:	eDBTYPE = SPAMFILTER;
			csFileName = L"VBSPAMEMAIL.DB";
			break;
		case 0x03:	eDBTYPE = CONTENTFILTER;
			csFileName = L"VBCONTENTEMAIL.DB";
			break;
		case 0x04:	eDBTYPE = SIGNATURE;
			csFileName = L"VBSIGNATUREEMAIL.DB";
			break;
		case 0x05:	eDBTYPE = REPORTS;
			if (m_bUserDefFileNames)
			{
				csRegAppPath += L"REPORTS\\";
				if (!GetNewFileName(eDBTYPE, csFileName))
				{
					AddLogEntry(L"### Failed GetNewFileName, Path: %s, FileName: %s", csRegAppPath, csFileName, true, SECONDLEVEL);
					return bReturn;
				}
			}
			else
			{
				csFileName = L"VBREPORTS.DB";
			}
			break;
		case 0x06:	eDBTYPE = MALICIOUSSITES;
			csFileName = L"VBMALICIOUSSITES.DB";
			break;
		case 0x07: eDBTYPE = FOLDERLOCKER;
			csFileName = L"VBFOLDERLOCKERENTRIES.DB";
			break;
		default:
			AddLogEntry(L"### Invalid Entry Type in CWardwizDBManipulation::SaveEntry", 0, 0, true, SECONDLEVEL);
			return false;
		}

		csRegAppPath.Append(csFileName);
		bReturn = SaveDBFile(eDBTYPE, csRegAppPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::LoadDBFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	LoadEntries
*  Description    :	Load entries according to module type
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0007
**********************************************************************************************************/
bool CISpyDBManipulation::LoadEntries(DWORD dwType)
{
	bool bReturn = false;
	try
	{
		CString csRegAppPath, csFileName;
		ITINEMAIL_DATABASETYPE eDBTYPE;
		csRegAppPath = GetWardWizPathFromRegistry();

		switch (dwType)
		{
		case 0x00:	eDBTYPE = RECOVER;
			csRegAppPath = L"";
			csRegAppPath = GetQuarantineFolderPath();
			csFileName += _T("\\VBRECOVERENTRIES.DB");
			break;
		case 0x01:	eDBTYPE = VIRUSSCAN;
			csFileName = L"VBVIRUSSCANEMAIL.DB";
			break;
		case 0x02:	eDBTYPE = SPAMFILTER;
			csFileName = L"VBSPAMEMAIL.DB";
			break;
		case 0x03:	eDBTYPE = CONTENTFILTER;
			csFileName = L"VBCONTENTEMAIL.DB";
			break;
		case 0x04:	eDBTYPE = SIGNATURE;
			csFileName = L"VBSIGNATUREEMAIL.DB";
			break;
		case 0x05:	eDBTYPE = REPORTS;
			csFileName = L"VBREPORTS.DB";
			break;
		case 0x06:	eDBTYPE = MALICIOUSSITES;
			csFileName = L"VBMALICIOUSSITES.DB";
			break;
		case 0x07:	eDBTYPE = FOLDERLOCKER;
			csFileName = L"VBFOLDERLOCKERENTRIES.DB";
			break;
		default:
			AddLogEntry(L"### Invalid Entry Type in CWardwizDBManipulation::Load Entry", 0, 0, true, SECONDLEVEL);
			return false;
		}

		csRegAppPath.Append(csFileName);
		bReturn = LoadDBFile(eDBTYPE, csRegAppPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::LoadEntries", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	LoadEntries
*  Description    :	Load entries according to module type
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0007
**********************************************************************************************************/
bool CISpyDBManipulation::LoadEntries(DWORD dwType, CString csDirPath)
{
	bool bReturn = false;
	try
	{
		CString csRegAppPath, csFileName;
		ITINEMAIL_DATABASETYPE eDBTYPE;
		//csDirPath = GetWardWizPathFromRegistry();
		csRegAppPath = csDirPath;
		switch (dwType)
		{
		case 0x00:	eDBTYPE = RECOVER;
			csRegAppPath = L"";
			csRegAppPath = csDirPath;
			csFileName += _T("\\VBRECOVERENTRIES.DB");
			break;
		case 0x01:	eDBTYPE = VIRUSSCAN;
			csFileName = L"VBVIRUSSCANEMAIL.DB";
			break;
		case 0x02:	eDBTYPE = SPAMFILTER;
			csFileName = L"VBSPAMEMAIL.DB";
			break;
		case 0x03:	eDBTYPE = CONTENTFILTER;
			csFileName = L"VBCONTENTEMAIL.DB";
			break;
		case 0x04:	eDBTYPE = SIGNATURE;
			csFileName = L"VBSIGNATUREEMAIL.DB";
			break;
		case 0x05:	eDBTYPE = REPORTS;
			csFileName = L"VBREPORTS.DB";
			break;
		case 0x06:	eDBTYPE = MALICIOUSSITES;
			csFileName = L"VBMALICIOUSSITES.DB";
			break;
		case 0x07:	eDBTYPE = FOLDERLOCKER;
			csFileName = L"VBFOLDERLOCKERENTRIES.DB";
			break;
		default:
			AddLogEntry(L"### Invalid Entry Type in CWardwizDBManipulation::Load Entry", 0, 0, true, SECONDLEVEL);
			return false;
		}

		csRegAppPath.Append(csFileName);
		bReturn = LoadDBFile(eDBTYPE, csRegAppPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::LoadEntries", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	ReloadEntries
*  Description    :	Reload entries into data manager object
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0008
**********************************************************************************************************/
bool CISpyDBManipulation::ReloadEntries(DWORD dwType)
{
	bool bReturn = false;
	try
	{
		switch ((ITINEMAIL_DATABASETYPE)dwType)
		{
		case RECOVER:
			m_RecoverDBEntries.RemoveAll();
			bReturn = LoadEntries(dwType);
			break;
		case VIRUSSCAN:
			m_VirusScanDBEntries.RemoveAll();
			bReturn = LoadEntries(dwType);
			break;
		case SPAMFILTER:
			m_SpamFilterDBEntries.RemoveAll();
			bReturn = LoadEntries(dwType);
			break;
		case CONTENTFILTER:
			m_ContentFilterDBEntries.RemoveAll();
			bReturn = LoadEntries(dwType);
			break;
		case SIGNATURE:
			m_SignatureDBEntries.RemoveAll();
			bReturn = LoadEntries(dwType);
			break;
		case REPORTS:
			m_ReportDBEntries.RemoveAll();
			bReturn = LoadEntries(dwType);
			break;
		case MALICIOUSSITES:
			m_MaliciousEntries.RemoveAll();
			bReturn = LoadEntries(dwType);
			break;
		case FOLDERLOCKER:
			m_FolderLockerDBEntries.RemoveAll();
			bReturn = LoadEntries(dwType);
			break;
		default:
			AddLogEntry(L"### Invalid Entry Type in CWardwizDBManipulation::ReloadEntry", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::ReloadEntries", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	ProcessEntry
*  Description    :	Each entry get tokenize which get add save further into DB file
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0009
**********************************************************************************************************/
bool CISpyDBManipulation::ProcessEntry(DWORD dwActionType, TCHAR *szEntry, DWORD dwType)
{
	try
	{
		CString csEntry;
		int iPos = 0, i = 0;
		DWORD dwSeventhEntry = 0;

		CString csaTokenEntry[7];
		CString csFirstEntry, csSecondEntry, csThirdEntry, csForthEntry,
			csFifthEntry, csSixthEntry, csSeventhEntry;
		CString resToken;

		csEntry = szEntry;
		resToken = csEntry.Tokenize(_T("#"), iPos);
		while (resToken != _T(""))
		{
			csaTokenEntry[i++] = resToken;
			resToken = csEntry.Tokenize(_T("#"), iPos);
		}

		csFirstEntry = csaTokenEntry[0].Trim();
		csSecondEntry = csaTokenEntry[1].Trim();
		csThirdEntry = csaTokenEntry[2].Trim();
		csForthEntry = csaTokenEntry[3].Trim();
		csFifthEntry = csaTokenEntry[4].Trim();
		csSixthEntry = csaTokenEntry[5].Trim();
		csSeventhEntry = csaTokenEntry[6].Trim();

		if (csSeventhEntry.GetLength() != 0)
		{
			dwSeventhEntry = static_cast<DWORD>(_wtoi(csSeventhEntry));
		}

		CIspyList NewInsertEntry(csFirstEntry, csSecondEntry, csThirdEntry, csForthEntry,
			csFifthEntry, csSixthEntry, dwSeventhEntry);

		switch (dwActionType)
		{
		case ADD_EMAIL_ENTRY:
		case ADD_FOLDER_LOCKER_ENTRY:
		case REPORT_ANTIROOTKIT_ENTRY:
		case REPORT_USBSCAN_ENTRY:
			if (dwType == 0x04)
			{
				CIspyList NewSignatureEntry(csEntry, L"", L"", L"", L"", L"", 0);
				InsertEntry(NewSignatureEntry, (ITINEMAIL_DATABASETYPE)dwType);
			}
			else
			{
				InsertEntry(NewInsertEntry, (ITINEMAIL_DATABASETYPE)dwType);
			}
			break;
		case DELETE_EMAIL_ENTRY:
		case DELETE_RECOVER_ENTRIES:
		case DELETE_FOLDER_LOCKER_ENTRY:
			if (dwType == 0x00 || dwType == 0x02 || dwType == 0x03 || dwType == 0x07)
			{
				RemoveEntry((ITINEMAIL_DATABASETYPE)dwType, csFirstEntry, csSecondEntry, csThirdEntry, csForthEntry);
			}
			else
			{
				RemoveAllEntries((ITINEMAIL_DATABASETYPE)dwType);
			}
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::ProcessEntry", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ProcessEntry
*  Description    :	This function is only for when entry is not with # tokenize
*  Author Name    : Ram Shelke
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0009
**********************************************************************************************************/
bool CISpyDBManipulation::ProcessEntry(DWORD dwActionType, DWORD dwType, TCHAR * szFirstEntry, TCHAR * szSecondEntry, TCHAR * szThirdEntry, TCHAR * szForthEntry,
	TCHAR * szFifthEntry, TCHAR * szSixthEntry, TCHAR * szSeventhEntry)
{
	try
	{
		CString csFirstEntry = EMPTY_STRING;
		CString csSecondEntry = EMPTY_STRING;
		CString csThirdEntry = EMPTY_STRING;
		CString csForthEntry = EMPTY_STRING;
		CString csFifthEntry = EMPTY_STRING;
		CString csSixthEntry = EMPTY_STRING;
		CString csSeventhEntry = EMPTY_STRING;

		if (szFirstEntry != NULL)
		{
			csFirstEntry = szFirstEntry;
		}

		if (szSecondEntry != NULL)
		{
			csSecondEntry = szSecondEntry;
		}

		if (szThirdEntry != NULL)
		{
			csThirdEntry = szThirdEntry;
		}

		if (szForthEntry != NULL)
		{
			csForthEntry = szForthEntry;
		}

		if (szFifthEntry != NULL)
		{
			csFifthEntry = szFifthEntry;
		}

		if (szSixthEntry != NULL)
		{
			csSixthEntry = szSixthEntry;
		}

		if (szSeventhEntry != NULL)
		{
			csSeventhEntry = szSeventhEntry;
		}

		DWORD dwSeventhEntry = 0;
		if (csSeventhEntry.GetLength() != 0)
		{
			dwSeventhEntry = static_cast<DWORD>(_wtoi(csSeventhEntry));
		}

		CIspyList NewInsertEntry(csFirstEntry, csSecondEntry, csThirdEntry, csForthEntry,
			csFifthEntry, csSixthEntry, dwSeventhEntry);

		switch (dwActionType)
		{
		case ADD_EMAIL_ENTRY:
		case ADD_FOLDER_LOCKER_ENTRY:
		case REPORT_ANTIROOTKIT_ENTRY:
		case REPORT_USBSCAN_ENTRY:
			InsertEntry(NewInsertEntry, (ITINEMAIL_DATABASETYPE)dwType);
			break;
		case DELETE_EMAIL_ENTRY:
		case DELETE_RECOVER_ENTRIES:
		case DELETE_FOLDER_LOCKER_ENTRY:
			if (dwType == 0x00 || dwType == 0x02 || dwType == 0x03 || dwType == 0x07)
			{
				RemoveEntry((ITINEMAIL_DATABASETYPE)dwType, csFirstEntry, csSecondEntry, csThirdEntry, csForthEntry);
			}
			else
			{
				RemoveAllEntries((ITINEMAIL_DATABASETYPE)dwType);
			}
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::ProcessEntry", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	EditEntry
*  Description    :	edit single entry
*  Author Name    : Ram Shelke
*  Date           : 24 Mar 2014
*  SR_NO		  :
**********************************************************************************************************/
bool CISpyDBManipulation::EditEntry(CString csKey, const CIspyList& contact, ITINEMAIL_DATABASETYPE eDBType)
{
	try
	{
		switch (eDBType)
		{
		case RECOVER:
			m_RecoverDBEntries.EditContact(csKey, contact);
			break;
		default:
			AddLogEntry(L"### Unhandled case in CWardwizDBManipulation::EditEntry", csKey, 0, true, SECONDLEVEL);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::EditEntry, Key: %s", csKey, 0, true, SECONDLEVEL);
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	InsertEntry
*  Description    :	Insert each one entry into DB file
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0010
**********************************************************************************************************/
bool CISpyDBManipulation::InsertEntry(const CIspyList& contact, ITINEMAIL_DATABASETYPE eDBType)
{
	try
	{
		switch (eDBType)
		{
		case RECOVER:
			m_RecoverDBEntries.AddContact(contact);
			break;
		case VIRUSSCAN:
			LoadEntries(VIRUSSCAN);
			m_VirusScanDBEntries.AddContact(contact, true);
			SaveEntries(VIRUSSCAN);
			break;
		case SPAMFILTER:
			m_SpamFilterDBEntries.AddContact(contact);
			break;
		case CONTENTFILTER:
			m_ContentFilterDBEntries.AddContact(contact, true);
			break;
		case SIGNATURE:
			m_SignatureDBEntries.RemoveAll();
			m_SignatureDBEntries.AddContact(contact, true);
			break;
		case REPORTS:
			m_ReportDBEntries.AddContact(contact, true);
			break;
		case MALICIOUSSITES:
			m_MaliciousEntries.AddRecord(contact);
			break;
		case FOLDERLOCKER:
			m_FolderLockerDBEntries.AddContact(contact, true);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::InsertEntry", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}
//
///**********************************************************************************************************                     
//*  Function Name  :	EditEntry                                                     
//*  Description    :	Edit each one entry
//*  Author Name    : Neha Gharge                                                                                        
//*  Date           : 22 Jan 2014
//*  SR_NO		  : WRDWIZSERIALIZATION_0011
//**********************************************************************************************************/
//bool CISpyDBManipulation::EditEntry(const CIspyList& contact, ITINEMAIL_DATABASETYPE eDBType)
//{
//	switch(eDBType)
//	{
//	case RECOVER:	
//		m_RecoverDBEntries.AddContact(contact);
//		break;
//	case VIRUSSCAN:
//		m_VirusScanDBEntries.AddContact(contact,true);
//		break;
//	case SPAMFILTER:
//		m_SpamFilterDBEntries.bWithoutEdit = 0;
//		m_SpamFilterDBEntries.AddContact(contact);
//		break;
//	case CONTENTFILTER:
//		m_ContentFilterDBEntries.bWithoutEdit = 0;
//		m_ContentFilterDBEntries.AddContact(contact,true);
//		break;
//	case SIGNATURE:
//		m_SignatureDBEntries.RemoveAll();
//		m_SignatureDBEntries.AddContact(contact,true);
//		break;
//	case REPORTS:
//		m_ReportDBEntries.AddContact(contact,true);
//		break;
//	case MALICIOUSSITES:
//		m_MaliciousEntries.AddRecord(contact);
//		break;
//	case FOLDERLOCKER:
//		m_FolderLockerDBEntries.AddContact(contact,true);
//		break;
//	}
//	return true;
//}


/**********************************************************************************************************
*  Function Name  :	RemoveEntry
*  Description    :	Remove each one entry from DB object
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0012
**********************************************************************************************************/
bool CISpyDBManipulation::RemoveEntry(ITINEMAIL_DATABASETYPE eDBType, CString FirstParam, CString SecondParam, CString ThirdParam, CString ForthParam)
{
	try
	{
		switch (eDBType)
		{
		case RECOVER:
			m_RecoverDBEntries.RemoveContact(FirstParam);
			break;
		case SPAMFILTER:
			m_SpamFilterDBEntries.RemoveContact(FirstParam, SecondParam);
			break;
		case CONTENTFILTER:
			m_ContentFilterDBEntries.RemoveContact(FirstParam, SecondParam, ThirdParam);
			break;
		case SIGNATURE:
			m_SignatureDBEntries.RemoveContact(FirstParam, L"");
			break;
		case FOLDERLOCKER:
			m_FolderLockerDBEntries.RemoveContact(FirstParam, SecondParam);
			break;
		case REPORTS:
			m_ReportDBEntries.RemoveContact(FirstParam, SecondParam);
			break;
		case VIRUSSCAN:
			m_VirusScanDBEntries.RemoveContact(FirstParam, SecondParam, ThirdParam);
			break;
		default:
			AddLogEntry(L"### Invalid Entry in CWardwizDBManipulation::RemoveEntry", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::RemoveEntry", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	RemoveAllEntries
*  Description    :	Remoave All entries from DB files
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0013
**********************************************************************************************************/
bool CISpyDBManipulation::RemoveAllEntries(ITINEMAIL_DATABASETYPE eDBType)
{
	try
	{
		switch (eDBType)
		{
		case RECOVER:
			m_RecoverDBEntries.RemoveAll();
			break;
		case VIRUSSCAN:
			m_VirusScanDBEntries.RemoveAll();
			break;
		case SPAMFILTER:
			m_SpamFilterDBEntries.RemoveAll();
			break;
		case CONTENTFILTER:
			m_ContentFilterDBEntries.RemoveAll();
			break;
		case SIGNATURE:
			m_SignatureDBEntries.RemoveAll();
			break;
		case REPORTS:
			m_ReportDBEntries.RemoveAll();
			break;
		case MALICIOUSSITES:
			m_MaliciousEntries.RemoveAll();
			break;
		case FOLDERLOCKER:
			m_FolderLockerDBEntries.RemoveAll();
			break;
		default:
			AddLogEntry(L"### Invalid Entry in CWardwizDBManipulation::RemoveEntry", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::RemoveAllEntries", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	RemoveReportEntry
*  Description    :	Remove entry from report file
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0014
**********************************************************************************************************/
bool CISpyDBManipulation::RemoveReportEntry(LPTSTR csDateTime, LPTSTR csScanType, LPTSTR csFilePath)
{
	return m_ReportDBEntries.RemoveReportEntry(csDateTime, csScanType, csFilePath);
}

/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get quarantine folder path
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0015
**********************************************************************************************************/
CString CISpyDBManipulation::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/**********************************************************************************************************
*  Function Name  :	GetNewFileName
*  Description    :	Function to get new file name to create DB file
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CISpyDBManipulation::GetNewFileName(DWORD dwType, CString &csFilePath)
{
	bool bReturn = false;
	try
	{
		CString csReportsFolderPath = GetWardWizPathFromRegistry();

		if (dwType == REPORTS)
		{
			csReportsFolderPath += L"REPORTS\\";
		}
		else if (dwType == RECOVER)
		{
			csReportsFolderPath += L"QUARANTINE\\";
		}
		else
		{
			return bReturn;
		}

		//Count here number of files.
		DWORD dwFilesCount = GetMaximumDigitFromFiles(csReportsFolderPath.GetBuffer());

		//Increase here the count so that new file will create with new name.
		dwFilesCount++;

		CString csFileName;

		if (dwType == REPORTS)
		{
			csFileName.Format(L"%s_%d.%s", L"VibraniumREPORTS", dwFilesCount, L"DB");
		}
		else if (dwType == RECOVER)
		{
			csFileName.Format(L"%s_%d.%s", L"VibraniumRECOVERENTRIES", dwFilesCount, L"DB");
		}
		else
		{
			return bReturn;
		}

		if (csFileName.GetLength() > 0)
		{
			csFilePath = csFileName;
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDBManipulation::GetNewFileName", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	GetMaximumDigitFromFiles
*  Description    :	Function to get Maximum number from DB files
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
DWORD CISpyDBManipulation::GetMaximumDigitFromFiles(LPCTSTR pstr)
{
	DWORD dwMaxDigit = 0;
	try
	{
		CFileFind finder;
		bool bIsFolder = false;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.DB");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bIsFolder = true;
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				continue;
			}

			CString csFileName = finder.GetFileName();
			CString csDigit = csFileName.Left(csFileName.ReverseFind(L'.'));
			csDigit = csDigit.Right(csDigit.GetLength() - (csDigit.ReverseFind(L'_') + 1));
			if (csDigit.Trim().GetLength() != 0)
			{
				DWORD dwTemp = _wtoi(csDigit);
				if (dwTemp > dwMaxDigit)
				{
					dwMaxDigit = dwTemp;
				}
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::EnumTotalFolder", 0, 0, true, SECONDLEVEL);
	}
	return dwMaxDigit;
}