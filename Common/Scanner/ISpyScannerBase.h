/**********************************************************************************************************                     
	  Program Name          : ISpyScannerBase.h
	  Description           : header file for Scanner
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 20 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna           Created Wrapper Base class for iSpy AV & Clam Scanner 
							  functionality implementation
***********************************************************************************************************/
#pragma once
#include "ClamScanner.h"
#include "WardWizScanner.h"
#include "ISpyDataManager.h"
#include "ISpyDBManipulation.h"
#include "ITinAntirootScanner.h"
#include "ISpyCriticalSection.h"
#include "ITinAntirootScanner.h"

class CISpyScannerBase
{
public:
	CISpyScannerBase(void);
	virtual ~CISpyScannerBase(void);
public:
	bool QuarentineFiles(CStringArray &csQurFilePaths);
	bool StartScan(SCANTYPE eScanType,DWORD dwType, LPCTSTR szParam);
	bool StartScanSEH(SCANTYPE eScanType, DWORD dwAntirootScanOpt, LPCTSTR szParam);
	bool GetScanningPaths(LPCTSTR szParam, CStringArray &csaScanPaths);
	//bool HandleVirusEntry(LPCTSTR szThreatPath,LPCTSTR szThreatName,LPCTSTR szThreatName1,DWORD dwISpyID,DWORD dwScanType);
	bool RescanAndRepairFile(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR pszBackUpPath, DWORD dwISpyID);

	//bool BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath);
	//DWORD Encrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus);
	//DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
	//bool RecoverCurrentFile(CString csThreatPath);
	//DWORD Decrypt_File(TCHAR* szRecoverFilePath, TCHAR* szOriginalThreatPath, DWORD &dwStatus);
	//bool LoadExistingRecoverFile();
	//bool LoadDataContentFromFile(CString csPathName);
	//CString GetQuarantineFolderPath();

	//Modified by Vilas on 27 March 2015 to handle more failure cases
	//bool RecoverFile(LPTSTR lpFilePath , LPTSTR lpBrowseFilePath,DWORD dwTypeofAction);
	DWORD RecoverFile(LPTSTR lpFilePath, LPTSTR lpBrowseFilePath, DWORD dwTypeofAction);
	
	
	//bool CreateFullDirectoryPath(wchar_t *szFullPath);
	bool StopScan(SCANTYPE eScanType);
	bool PauseScan(SCANTYPE eScanType);
	bool ResumeScan(SCANTYPE eScanType);
	bool SetManipulationPtr(CISpyDBManipulation *pISpyDBManpt);
	bool FileOperations(LPCTSTR SourcePath ,LPCTSTR DestinationPath  , DWORD dwType);
	bool CheckDestinationPathExists(LPCTSTR DestinationPath);

	//Ram: Functionality added to Load/Unload signature database
	DWORD LoadSignatureDatabase(DWORD &dwSigCount);
	DWORD UnLoadSignatureDatabase();
	DWORD ReLoadSignatureDatabase(DWORD &dwSigCount);

	//Signature Changed by Vilas on 07 April 2015 to handle more failure cases
	DWORD HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThreatName1, DWORD dwISpyID, DWORD dwScanType, CString &csBackupFilePath, DWORD &dwAction);
	DWORD ScanFile(LPTSTR lpszFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD dwScanType, DWORD &dwAction, DWORD &dwSignatureFailedToLoad, bool bRescan);
	void EnumFolder(LPCTSTR lpszFolderPath);
	bool SaveLocalDBFiles();
public:
	CClamScanner				m_objClamScanner;
	CWardWizScanner				m_objWardWizScanner;
	CISpyScanner				m_objCISpyScanner;
	CITinAntirootKitScanner		m_objITinAntirootKitScanner;
	SCANLEVEL					m_eScanLevel;
	SCANTYPE					m_eScanType;
	CISpyCriticalSection		m_objCriticalSectionHandleVEntry;
	CISpyCriticalSection		m_objCriticalSectionRecover;
	CISpyCriticalSection		m_objcsScanNRepair;
	CISpyCriticalSection		m_objcsScanUpdateSync;
	DWORD						m_dwFilesIndexed;
	bool						m_bStopScan;
	//CString					m_csDuplicateName;
	//DWORD						m_dwEnKey ;
	//TCHAR						m_szQuarantineFilePath[MAX_PATH];
	//TCHAR						m_szOriginalThreatPath[MAX_PATH];
	//CDataManager				m_objRecoverdb;
};

