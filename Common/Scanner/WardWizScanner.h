/**********************************************************************************************************                     
	  Program Name          : WardWizScanner.h
	  Description           : Implementation of WardWiz  Scanner functionality 
	  Author Name			: Neha Gharge                                                                                           
	  Date Of Creation      : 4 Dec 2014
	  Version No            : 1.9.0.0
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Neha Gharge           Created Wrapper class for WardWiz functionality implementation
***********************************************************************************************************/
#pragma once
#include "stdafx.h"
#include "TreeConstants.h"
#include "ISpyScanner.h"
#include "iSpyMemMapServer.h"
#include "ISpyDataManager.h"
#include "ISpyDBManipulation.h"
#include "ISpyCommunicator.h"
#include "WardwizLangManager.h"
#include "ExcludeFilesFolders.h"
#include "ScanByName.h"

class CWardWizScanner
{
public:
	CWardWizScanner(void);
	virtual ~CWardWizScanner(void);
public:
	void EnumFolder(LPCTSTR pstr);
	void WriteStdOut(LPTSTR msg, BOOL freshclam);
	//BOOL LaunchClamAV(LPTSTR pszCmdLine, HANDLE hStdOut, HANDLE hStdErr);
	void RedirectStdOutput(BOOL freshclam);
	bool StartScanning(SCANTYPE eScanType, CStringArray &csaAllScanPaths);
	bool SetScanStatus(CString csStatus);
	void GetModuleCount( );
	bool QuarantineEntry(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath);
	bool GetVirusNameAndPath(CString csStatus, CString &csVirusName, CString &csVirusPath);
	bool SendMessage2UI(int iRequest);
	bool HandleVirusFoundEntries(CString strVirusName, CString strScanFileName, DWORD &dwAction, DWORD dwSpyID);
	void SendTotalFileCount();
	bool PauseScan();
	bool StopScan();
	bool ResumeScan();
	//bool HandleVirusEntry(LPCTSTR szThreatPath,LPCTSTR szThreatName, DWORD dwISpyID);
	bool GetWardwizRegistryDetails(DWORD &dwQuarantineOpt, DWORD &dwHeuScanOpt);
	void AddEntriesInReportsDB(SCANTYPE csScanType,CString csThreatName,CString csFilePath,CString csAction);
	bool SaveInReportsDB();
	void LoadReportsDBFile();
	bool SetManipulationPtr(CISpyDBManipulation *pISpyDBManpt);
	bool BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath, LPTSTR lpszBackupFilePath);
	DWORD Encrypt_File(TCHAR *m_szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus);
	DWORD Decrypt_File(TCHAR* szRecoverFilePath, TCHAR* szOriginalThreatPath, DWORD &dwStatus, bool bDeleteBackupFile);
	DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
	CString GetQuarantineFolderPath();
	bool InsertRecoverEntry(LPCTSTR szThreatPath,LPCTSTR m_csDuplicateName,LPCTSTR szThreatName, DWORD dwShowStatus);
	bool SaveInRecoverDB();
	void LoadRecoversDBFile();

	//Modified by Vilas on 28 March 2015
	//Signature Modified to handle failure cases
	DWORD RecoverFile(LPTSTR lpFilePath , LPTSTR lpBrowseFilePath, DWORD dwTypeofAction);
	DWORD RecoverCurrentFile(CString csThreatPath, CString csBrowseFilePath, DWORD dwTypeofAction);
	bool CreateFullDirectoryPath(wchar_t *szFullPath);
	bool CheckForDuplicates(CString csFilePath);
	void SetLastScanDatetime();
	void ClearMemoryMapObjects();
	bool CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize);
	bool ReadKeyFromEncryptedFile(HANDLE hFile);
	bool IsFileAlreadyEncrypted(HANDLE hFile);
	void EnumFolderForScanning(LPCTSTR pstr);
	void ScanForSingleFile(CString csFilePath);
	bool ShutdownScan();
	void EnumerateProcesses();
	bool IsDuplicateModule( LPTSTR szModulePath, DWORD dwSize );
	void LoadPSAPILibrary();
	bool AddDeleteFailEntryToINI(CString csQurFilePaths, CString csVirusName, CString csBackupFilePath);

	//Added by Vilas on 23 Mar 2015 for reboot repair
	void AddRepairEntryAfterReboot(LPCTSTR szThreatPath, LPCTSTR szThreatName, CString csDuplicateName, DWORD dwISpyID);
	bool RescanAndRepairFile(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szDupName, DWORD dwISpyID);
	bool GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath);
	bool CheckFileIsInRepairRebootIni(CString csFilePath);

	//Added by Vilas on 28 Mar 2015 for Recover in use file
	DWORD RecoverInUseFileIfPossible(LPCTSTR szRecoverFilePath, LPCTSTR szOriginalThreatPath, DWORD &dwStatus, bool bDeleteBackupFile);
	DWORD RenameFile(LPCTSTR szOriginalThreatPath, LPTSTR lpszRenamedFilePath);
	void AddRenamedFileEntryInIni(LPCTSTR lpszInUseFilePath);
	bool CheckFileIsInRecoverIni(CString csFilePath);

	//Added by Ram to update Repair Entry
	bool UpdateRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowEntryStatus);
	bool TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName);
	//Added by Vilas on 07 April 2015 for handling failure cases
	DWORD HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThreatName1, DWORD dwISpyID, CString &csBackupFilePath, DWORD &dwAction);

	//added by Neha Gharge for adding entry into delete ini
	bool MakeConcatStringforrecover(CString csFilename, CString csQuarantinePath, CString csVirusName, TCHAR * szConcateDeleteString, DWORD dwsizeofConcatenateString);
	bool TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName);
	bool CheckFileIsInDeleteIni(CString csQurFilePaths);

	//Function added by Ram to Load/UnLoad Signature Database.
	DWORD LoadSignatureDatabase(DWORD &dwSigCount);
	DWORD UnLoadSignatureDatabase();

	//Function added by Neha to see the enough space to take a backup
	bool IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize);
	DWORD CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath);
	DWORD ScanFile(LPTSTR lpszFilePath, LPTSTR szVirusName, DWORD& dwISpyID, DWORD dwScanType, DWORD &dwAction, DWORD& dwSignatureFailedToLoad, bool bRescan);
	void SetAutoQuarentineOption(DWORD dwOption);
	bool ReloadExcludeDB();
	DWORD GetDaysLeft();
	CString GetActionIDFromAction(CString csMessage);
	bool CheckIFAlreadyBackupTaken(LPCTSTR szFileHash, LPTSTR szBackupPath);
	bool GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash);
	bool CheckEntryPresent(LPCTSTR szFileHash, LPTSTR szBackupPath);
	bool CheckForDuplicateEntry(TCHAR *pFilePath, TCHAR *pBackupPath);
	bool ISExcludedPath(LPTSTR lpszPath, bool &bISSubFolderExcluded);
	bool ISExcludedFileExt(LPTSTR lpszFileExt);
	bool ReadBootRecoverEntries();
	bool AddScanSessionDetails(INT64 &iSessionID);
	bool AddScanDetails(INT64 iSessionID);
	bool OnRecoverQuarantineEntries(LPTSTR szThreatPath);
	bool IsScanByFileName(PCTSTR szFilePath);
	void ReloadScanByName();
	bool MoveFile2TempRestartDelete(LPCTSTR szThreatPath);
public:
	CStringArray			m_csaAllScanPaths;
	bool					m_ScanCount ;
	DWORD				    m_dwTotalScanPathCount;
	DWORD					m_iTotalFileCount;
	SCANTYPE				m_eScanType;
	bool					m_bSendScanMessage;
	CISpyScanner			m_objISpyScanner;
	iSpyServerMemMap_Server m_objServ;
	iSpyServerMemMap_Server m_objServViruEntry;
	int						m_iContactVersion;
	bool					m_bSuccess;
	int						m_iThreatsFoundCount;
	CISpyDBManipulation		*m_ptrISpyDBManipulation;
	//CString					m_csDuplicateName;
	DWORD					m_dwTotalFileCount;
	TCHAR					m_szQuarantineFilePath[MAX_PATH];
	TCHAR					m_szOriginalThreatPath[MAX_PATH];
	std::vector<CString>	m_vFileDuplicates;
	bool					m_bManualStop;
	bool					m_bRescan;
	unsigned char			*m_pbyEncDecKey;
	CStringList				m_csaModuleList;
	CISpyCommunicator		m_objCom;
	HMODULE					m_hPsApiDLL ;
	ENUMPROCESSMODULESEX	EnumProcessModulesWWizEx;
	bool					m_bFailedToLoadSignature;
	DWORD					m_dwFailedDeleteFileCount;
	bool					m_bFileFailedToDelete;
	CWardwizLangManager		m_objwardwizLangManager;
	bool					m_bISOnlyMemScan;
	ULARGE_INTEGER			m_uliFreeBytesAvailable;     // bytes disponiveis no disco associado a thread de chamada
	ULARGE_INTEGER			m_uliTotalNumberOfBytes;     // bytes no disco
	ULARGE_INTEGER			m_uliTotalNumberOfFreeBytes; // bytes livres no disco
	CExcludeFilesFolders	m_objExcludeFilesFolders;
	DWORD					m_dwAutoQuarOption;
	CISpyCriticalSection	m_csQuarentineEntries; 
	CISpyCriticalSection	m_csHandleVirusFoundEntries;
	CISpyCriticalSection	m_csScanFile;
	DWORD					m_dwDaysLeft;
	CScanByName				m_objScanByName;
};

