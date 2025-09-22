#pragma once
#include "ScannerContants.h"
#include "IWardwizScanAdapter.h"
#include "CommonFunctions.h"
#include "iSpyMemMapClient.h"
#include "TreeConstants.h"
#include "iTinEmailContants.h"
#include "PipeConstants.h"
#include "ISpyCommunicator.h"
#include "iSpyMemMapClient.h"

class CWardwizScanner :
	public IWardwizScanAdapter
{
private:

	SCANTYPE				m_eCurrentSelectedScanType;
	SCANLEVEL				m_eScanLevel;
	CISpyCommunicator		m_objCom;
	std::vector<CString>		  m_vecCustomScanSelectedEntries;
	std::vector<ISPY_PIPE_DATA>   m_vecArrayCleanEntries;

	//DWORD					m_dwTotalFileCount;
	//DWORD					m_iMemScanTotalFileCount;
	//DWORD					m_FileScanned;
	//DWORD					m_virusFound;
	HMODULE					m_hPsApiDLL;
	ENUMPROCESSMODULESEX	EnumProcessModulesWWizEx;
	HANDLE					m_hThreadVirEntries;
	HANDLE					m_hThreadStatusEntry;
	HANDLE					m_hQuarantineThread;

	CTime					m_tsScanStartTime;
	CTime					m_tsScanEndTime;
	CTime					m_tsScanPauseResumeTime;
	CTimeSpan				m_tsScanPauseResumeElapsedTime;
	CString					m_csRegKeyPath;

	void LoadPSAPILibrary();
	void OnDataReceiveCallBack(LPVOID lpParam);
	bool InvokeStartScanFromCommService(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan);
	bool InvokeRecoverOperationFromCommService(int dwMessageinfo, CString csRecoverFileEntry, DWORD dwType, bool bWait, bool bReconnect);
	bool InvokeRepairFileFromCommService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect);
	bool InvokeRepairFileFromCommService(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait, bool bReconnect);
	bool InvokeSetRegistrykeyCommService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait);
	bool MakeFullTokenizedScanPath(CStringArray &csaAllScanPaths, LPTSTR szScanPath);
	bool GetScanningPaths(CStringArray &csaReturn);
	bool GetAllDrivesList(CStringArray &csaReturn);
	bool Check4DBFiles();
	bool Check4ValidDBFiles(CString csDBFolderPath);
	bool SendRequestCommon(int iRequest);
	bool ShutDownScanning();
	void ScanForSingleFile(CString csFilePath);
public:
	SCANTYPE				m_eScanType;
//	CString					m_csRegKeyPath;
	CStringList				m_csaModuleList;
	CStringArray			m_csaAllScanPaths;
	bool					m_bQuickScan;
	bool					m_bFullScan;
	bool					m_bCustomscan;
	bool					m_bScnAborted;
	bool					m_bIsMemScan;
	bool					m_bStop;
	bool					m_bIsManualStop;
	bool					m_bIsPathExist;
	bool					m_bRescan;
	bool					m_ScanCount;
	bool					m_bThreatDetected;
	DWORD					m_dwTotalFileCount;
	DWORD					m_dwVirusFoundCount;
	DWORD					m_iMemScanTotalFileCount;
	DWORD					m_FileScanned;
	DWORD					m_virusFound;
	iSpyServerMemMap_Client m_objIPCClient;
	iSpyServerMemMap_Client m_objIPCClientVirusFound;
	HANDLE					m_hThread_ScanCount;
	HANDLE					m_hWardWizAVThread;
	CISpyCommunicator		m_objScanCom;
	int						m_iTotalFileCount;
	int						m_iThreatsFoundCount;
	CString					m_csPreviousPath;
	CString					m_csVirusName;

	CWardwizScanner();
	~CWardwizScanner();
	void EnumerateProcesses();

	// void CWardwizScanner::QuaratineEntries();
	 BOOL StartScan(CStringArray &csaAllScanPaths) override;

	 bool StartQuickScan();
	 
	 bool StartCustomScan(CStringArray &csaAllScanPaths);

	 bool IsDuplicateModule(LPTSTR szModulePath, DWORD dwSize);

	 void GetModuleCount();

	 BOOL PauseScan() override;

	 BOOL ResumeScan() override;
	 
	 BOOL StopScan() override;

	 BOOL BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath) override;

	 BOOL QuarantineFiles() override;

	 BOOL QuarantineSelectedfile() override;

	BOOL SearchForVirusAndQuarantine() override;

	BOOL RecoverOperations(int dwMessageinfo, CString csRecoverFileEntry, DWORD dwType, bool bWait = false, bool bReconnect = false) override;

	BOOL RepairFile(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect) override;

	BOOL RepairFile(int iMessage, CString csThreatPath, CString csThreatName, DWORD dwISpyID, bool bWait = false, bool bReconnect = false) override;
	bool CheckFileIsInRepairRebootIni(CString csFilePath);
	bool CheckFileIsInRecoverIni(CString csFilePath);
	CString GetQuarantineFolderPath();
	bool GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath);
	bool CheckFileIsInDeleteIni(CString csQurFilePaths);
	bool TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName);
	bool TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName);
	void EnumFolderForScanning(LPCTSTR pstr);
	void EnumFolder(LPCTSTR pstr);
};

