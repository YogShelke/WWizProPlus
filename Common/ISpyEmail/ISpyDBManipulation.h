#pragma once

#include "stdafx.h"
#include "Shlwapi.h"
#include "ISpyDataManager.h"
#include "ISpyList.h"
#include "iTinEmailContants.h"

class CISpyDBManipulation 
{
public:
	CISpyDBManipulation(void);
	~CISpyDBManipulation(void);
	CISpyDBManipulation(bool bIsUserDefined);
	CDataManager m_VirusScanDBEntries,m_SpamFilterDBEntries,m_ContentFilterDBEntries,m_SignatureDBEntries,m_RecoverDBEntries,m_ReportDBEntries,m_FolderLockerDBEntries,m_MaliciousEntries;
	bool SaveDBFile(ITINEMAIL_DATABASETYPE eDBType , CString csFilepath);
	bool LoadDBFile(ITINEMAIL_DATABASETYPE eDBType , CString csFilePath);
	bool ProcessEntry(DWORD dwActionType,TCHAR *szEntry , DWORD Type);
	bool ProcessEntry(DWORD dwActionType, DWORD dwType, TCHAR * csFirstEntry, TCHAR * csSecondEntry, TCHAR * csThirdEntry = NULL, TCHAR * csForthEntry = NULL,
		TCHAR * csFifthEntry = NULL, TCHAR * csSixthEntry = NULL, TCHAR * csSeventhEntry = NULL);
	bool InsertEntry(const CIspyList& contact, ITINEMAIL_DATABASETYPE eDBType);
	bool EditEntry(CString csKey, const CIspyList& contact, ITINEMAIL_DATABASETYPE eDBType);
	//bool EditEntry(const CIspyList& contact, ITINEMAIL_DATABASETYPE eDBType);
	bool RemoveEntry(ITINEMAIL_DATABASETYPE eDBType , CString FirstParam , CString SecondParam, CString ThirdParam, CString ForthParam);
	bool RemoveAllEntries(ITINEMAIL_DATABASETYPE eDBType);
	bool SaveEntries(DWORD dwType);
	bool LoadEntries(DWORD dwType);
	bool ReloadEntries(DWORD dwType);
	bool LoadEntries(DWORD dwType, CString csDirPath);
	bool SaveEntries(DWORD dwType, CString lpDirPath);
	bool RemoveReportEntry(LPTSTR csDateTime, LPTSTR csScanType, LPTSTR csFilePath);
	bool ClearDBEntries( ITINEMAIL_DATABASETYPE eDBType );
	CString GetQuarantineFolderPath();
public:
	bool m_bUserDefFileNames;
private:
	bool GetNewFileName(DWORD dwType, CString &csFilePath);
	DWORD GetMaximumDigitFromFiles(LPCTSTR pstr);
};

