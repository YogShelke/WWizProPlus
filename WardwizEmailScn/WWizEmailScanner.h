/******************************************************************************************************
*  Program Name	: WWizEmailScanner.h
*  Author Name	: Amol J.
*  Date Of Creation: 08 Feb 2018
*  Version No	: 3.1.0.0
/******************************************************************************************************/

#pragma once
#include "ISpyCommunicator.h"
#include "ScannerContants.h"
#include "WardWizDatabaseInterface.h"

struct VirusScanStruct
{
	CString	strEmailID;
	CString	strSubject;
	DWORD	strScanStatus;
};

struct VirusFoundZip
{
	CString	strFilePath;
	CString	strVirusName;
	DWORD	dwRepairID;
};

class CWWizEmailScanner
{
protected:

	CString	strScanStatus;

	typedef std::map<DWORD, VirusScanStruct> VirusScanDataBase;
	typedef std::map<DWORD, VirusFoundZip> VirusFoundZipDataBase;


	VirusScanDataBase		VirusScanDB;
	VirusFoundZipDataBase	VirusFoundDB;

	typedef bool(*LOADSIGNATURE)		(LPCTSTR);
	typedef bool(*UNLOADSIGNATURES)	(void);
	typedef bool(*SCANFILE)			(LPCTSTR, LPTSTR, DWORD&, bool);
	typedef bool(*REPAIRFILE)			(LPCTSTR, DWORD);
	typedef DWORD(*UNZIPFILE)			(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount);

	LOADSIGNATURE		m_lpLoadSigProc;
	UNLOADSIGNATURES	m_lpUnLoadSigProc;
	UNZIPFILE			UnzipFile;
	HMODULE				m_hZip;

public:
	CWWizEmailScanner();
	virtual ~CWWizEmailScanner();
	void ScanData(CString csFilePath, VirusFoundZipDataBase &mapVirusFound);
	bool ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan = false);
	int ExtractZipForScanning(TCHAR *pZipFile, CString csSenderName, CString csAttachmentName);
	FILETYPE GetFileType(LPCTSTR pszFilePath, DWORD &dwSubType, DWORD &dwVersion);
	void ScanZipFiles(TCHAR *pZipPath, bool &bModified, DWORD &dwCount, VirusFoundZipDataBase &mapVirusFound);
	bool SendEmailData2Tray(int iMessage, CString csThreatName, CString csAttachmentName, CString csSenderAddr, DWORD dwAction, bool bWait = false);
	DWORD RemoveFileOnly(TCHAR *pFile);
	DWORD RemoveAllZipFiles(TCHAR *pFile);
	DWORD RemoveFilesUsingSHFileOperation(TCHAR *pFolder);
	bool LoadRequiredDLLs();
	INT64 InsertDataToTable(const char* szQuery);

	CWardWizSQLiteDatabase	m_objSqlDb;
	CISpyCommunicator		m_objScanCom;
	bool					m_bRescan;
	DWORD					m_dwVirusCleanedCount;
	CString					strSenderMailID;
	CString					m_ViruName;
	DWORD					dwRemovedZips;
	CStringArray			csaReAttachZipFiles;
	CStringList				cslReAttachments;
	CStringList				cslRemoveAttachments;
	bool					bZipFileModified;
	int						m_iThreatAction;

};

