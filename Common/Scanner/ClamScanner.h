/**********************************************************************************************************                     
	  Program Name          : CClamScanner.h
	  Description           : Implementation of Clam AV Scanner functionality 
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 20 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna           Created Wrapper class for Clam AV functionality implementation
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
#include "WardWizScanner.h"

class CClamScanner
{
public:
	CClamScanner(void);
	virtual ~CClamScanner(void);
public:
	void EnumFolder(LPCTSTR pstr);
	void WriteStdOut(LPTSTR msg, BOOL freshclam);
	BOOL LaunchClamAV(LPTSTR pszCmdLine, HANDLE hStdOut, HANDLE hStdErr);
	void RedirectStdOutput(BOOL freshclam);
	bool StartScanning(SCANTYPE eScanType, CStringArray &csaAllScanPaths);
	bool SetScanStatus(CString csStatus);
	void GetModuleCount( );
	bool QuarantineEntry(CString csQurFilePaths, CString csVirusName);
	bool GetVirusNameAndPath(CString csStatus, CString &csVirusName, CString &csVirusPath);
	void ProcessCallBackEntries(CString csStatus);
	CString GetFileNameOnly(CString csInputPath);
	bool IsFullString(CString csInputPath);
	bool SendMessage2UI(int iRequest);
	bool HandleVirusFoundEntries(CString strVirusName, CString strScanFileName, CString csAction, DWORD dwSpyID);
	void SendTotalFileCount(DWORD dwCount, bool bIsMemOnly = false);
	bool PauseScan();
	bool StopScan();
	bool ResumeScan();
	//bool HandleVirusEntry(LPCTSTR szThreatPath,LPCTSTR szThreatName, DWORD dwISpyID);
	void AddEntriesInReportsDB(SCANTYPE csScanType,CString csThreatName,CString csFilePath,CString csAction);
	bool SaveInReportsDB();
	void LoadReportsDBFile();
	bool SetManipulationPtr(CISpyDBManipulation *pISpyDBManpt);
	bool BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath);
	DWORD Encrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus);
	DWORD Decrypt_File(TCHAR* szRecoverFilePath, TCHAR* szOriginalThreatPath, DWORD &dwStatus);
	DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
	CString GetQuarantineFolderPath();
	bool InsertRecoverEntry(LPCTSTR szThreatPath, LPCTSTR m_csDuplicateName, LPCTSTR szThreatName, DWORD dwShowStatus);
	bool SaveInRecoverDB();
	void LoadRecoversDBFile();
	//bool RecoverFile(LPTSTR lpFilePath , LPTSTR lpBrowseFilePath, DWORD dwTypeofAction);
	//bool RecoverCurrentFile(CString csThreatPath , CString csBrowseFilePath, DWORD dwTypeofAction);
	bool CreateFullDirectoryPath(wchar_t *szFullPath);
	bool CheckForDuplicates(CString csFilePath);
	void SetLastScanDatetime();
	void ClearMemoryMapObjects();
	bool CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize);
	bool ReadKeyFromEncryptedFile(HANDLE hFile);
	bool IsFileAlreadyEncrypted(HANDLE hFile);
	bool SendMessage2Tray(int iRequest);
	bool IsDuplicateModule( LPTSTR szModulePath);
	void RemoveModuleFromList(LPTSTR pszModulePath);
	void ScanFilesExcludedByClam();
	void ScanForSingleFile(CString csFilePath);
	void LoadPSAPILibrary();
	bool AddDeleteFailEntryToINI(CString csQurFilePaths, CString csVirusName);

	//Added by Vilas on 23 Mar 2015 for reboot repair
	void AddRepairEntryAfterReboot(LPCTSTR szThreatPath, LPCTSTR szThreatName, CString csDuplicateName, DWORD dwISpyID);
	bool RescanAndRepairFile(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szDupName, DWORD dwISpyID);
	bool GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath);
	bool CheckFileIsInRepairRebootIni(CString csFilePath);
	bool UpdateRecoverEntry(LPCTSTR szThreatPath, LPCTSTR csDuplicateName, LPCTSTR szThreatName, DWORD dwShowEntryStatus);
	bool TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName);

	//Added by Vilas on 26 Mar 2015 for Recover in use file
	DWORD RecoverInUseFileIfPossible(LPCTSTR szRecoverFilePath, LPCTSTR szOriginalThreatPath, DWORD &dwStatus);
	DWORD RenameFile(LPCTSTR szOriginalThreatPath, LPTSTR lpszRenamedFilePath);

	DWORD RecoverFile(LPTSTR lpFilePath, LPTSTR lpBrowseFilePath, DWORD dwTypeofAction);
	DWORD RecoverCurrentFile(CString csThreatPath, CString csBrowseFilePath, DWORD dwTypeofAction);

	//Added by Vilas on 27 Mar 2015 for Recover in use file
	void AddRenamedFileEntryInIni(LPCTSTR lpszInUseFilePath);
	bool CheckFileIsInRecoverIni(CString csFilePath);

	//Added by Vilas on 07 April 2015 for handling failure cases
	DWORD HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThirdParam, DWORD dwISpyID, CString &csBackupFilePath, CString &csQurStatus);

	//added by Neha Gharge for adding entry into delete ini
	bool MakeConcatStringforrecover(CString csFilename, CString csQuarantinePath, CString csVirusName, TCHAR * szConcateDeleteString, DWORD dwsizeofConcatenateString);
	bool TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName);
	bool CheckFileIsInDeleteIni(CString csQurFilePaths);
	
		//Function added by Ram to Load/UnLoad Signature Database.'
	DWORD LoadSignatureDatabase(DWORD &dwSigCount);
	DWORD UnLoadSignatureDatabase();

	//Suspend and resume clam Scanning.
	bool SuspendClamScan();
	bool ResumeClamScan();

	void MemoryScanFinished();
	void EnumerateProcesses();
	void ReInitializeVariables();

	bool IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize);
	DWORD CheckForDiscSpaceAvail(CString csQuaratineFolderPath, CString csThreatPath);
	bool SendProcessID2Protect(PROCESS_INFORMATION pi);
	bool ScanFile(LPTSTR lpszFilePath, LPTSTR szVirusName, DWORD& dwISpyID, DWORD& dwSignatureFailedToLoad, bool bRescan);
public:
	CStringArray			m_csaAllScanPaths;
	bool					m_ScanCount ;
	DWORD				    m_dwTotalScanPathCount;
	DWORD					m_iTotalFileCount;
	DWORD					m_iMemScanFileCount;
	SCANTYPE				m_eScanType;
	bool					m_bSendScanMessage;
	CISpyScanner			m_objISpyScanner;
	iSpyServerMemMap_Server m_objServ;
	iSpyServerMemMap_Server m_objServViruEntry;
	//CDataManager			m_objReportsDB;
	int						m_iContactVersion;
	bool					m_bSuccess;
	int						m_iThreatsFoundCount;
	CISpyDBManipulation		*m_ptrISpyDBManipulation;
	//DWORD					m_dwEnKey ;
	CString					m_csDuplicateName;
	DWORD					m_dwTotalFileCount;
	DWORD					m_dwTotalMemScanFileCount;
	TCHAR					m_szQuarantineFilePath[MAX_PATH];
	TCHAR					m_szOriginalThreatPath[MAX_PATH];
	std::vector<CString>	m_vFileDuplicates;
	bool					m_bManualStop;
	bool					m_bRescan;
	bool					m_bFailedToLoadSignature;
	unsigned char			*m_pbyEncDecKey;
	CISpyCommunicator		m_objCom;
	HANDLE					m_hThread_Output;
	CStringList				m_csaModuleList;
	HMODULE					m_hPsApiDLL ;
	ENUMPROCESSMODULESEX	EnumProcessModulesWWizEx;
	DWORD					m_dwFailedDeleteFileCount;
	bool					m_bFileFailedToDelete;
	CWardwizLangManager		m_objwardwizLangManager;
	bool					m_bMemScanCompleted;
	bool					m_bOnlyMemScan;
	bool					m_bIsClamLoaded;
	bool					m_bIsClamSuspended;
	bool					m_bISOnlyMemScan;
	HANDLE					m_hEventMemScanFinished;
	ULARGE_INTEGER			m_uliFreeBytesAvailable;     // bytes disponiveis no disco associado a thread de chamada
	ULARGE_INTEGER			m_uliTotalNumberOfBytes;     // bytes no disco
	ULARGE_INTEGER			m_uliTotalNumberOfFreeBytes; // bytes livres no disco
	
};

