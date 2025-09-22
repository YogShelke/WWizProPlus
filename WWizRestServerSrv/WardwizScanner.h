#pragma once
#include "ScannerContants.h"
#include "TreeConstants.h"
#include "PipeConstants.h"
#include "ISpyCommunicator.h"
#include <vector>
#include<atlstr.h>

using namespace std;

class CWardwizScanner 
{
private:

	SCANTYPE				m_eCurrentSelectedScanType;
	CISpyCommunicator		m_objCom;
	HMODULE					m_hPsApiDLL;
	ENUMPROCESSMODULESEX	EnumProcessModulesWWizEx;
	HANDLE					m_hThreadVirEntries;
	HANDLE					m_hThreadStatusEntry;
	HANDLE					m_hQuarantineThread;

	void LoadPSAPILibrary();
	
	bool InvokeStartScanFromCommService(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan);
	void ScanForSingleFile(CString csFilePath);
	
public:
	SCANTYPE				m_eScanType;
	bool					m_bIsPathExist;
	bool					m_bRescan;
	bool					m_bThreatDetected;
	DWORD					m_dwTotalFileCount;
	DWORD					m_dwVirusFoundCount;
	DWORD					m_iMemScanTotalFileCount;
	DWORD					m_FileScanned;
	DWORD					m_virusFound;

	CISpyCommunicator		m_objScanCom;
	int						m_iTotalFileCount;
	int						m_iThreatsFoundCount;
	CString					m_csPreviousPath;
	CString					m_csVirusName;

	CWardwizScanner();
	~CWardwizScanner();
	bool StartWardwizScanner(std::string strFileToScan);
	bool CheckFileIsInRepairRebootIni(CString csFilePath);
	bool CheckFileIsInRecoverIni(CString csFilePath);
	CString GetQuarantineFolderPath();
	bool GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath);
	bool CheckFileIsInDeleteIni(CString csQurFilePaths);
	bool TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName);
	bool TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName);

};

