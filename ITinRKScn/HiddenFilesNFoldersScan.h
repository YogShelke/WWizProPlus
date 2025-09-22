#pragma once
#include <vector>
#include <algorithm>

#ifndef _NTDEF_
typedef LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;
#endif

typedef std::vector<CString> vecFILESNFOLDERSINFO ;

class CHiddenFilesNFoldersScan
{
public:
	CHiddenFilesNFoldersScan(void);
	virtual ~CHiddenFilesNFoldersScan(void);
	void StopScan(void);
	bool ScanDrive(const WCHAR *wcDriveLetter);
	bool GetFolderList(WCHAR *wsFolderPath);
	bool CreateDriveTreeByAPIs(TCHAR *chDrive);
	bool EnumerateFolderByAPIs(LPTSTR szPath, vecFILESNFOLDERSINFO &vFilesList);
	bool CheckIfRootKit(const WCHAR *szParentName, const WCHAR *szModuleName, bool bDirectory);
	bool CheckFileExtention(CString csFilePath);
	bool InitializeVariables();
	DWORD CalculateScanPercentage();
	DWORD GetScanPercentage();
	void SetPreviousPercentage(DWORD dwPercentage);
public:
	bool					m_bStopScan;
	vecFILESNFOLDERSINFO	m_vFilesNFoldersListByNTDLL;
	vecFILESNFOLDERSINFO	m_vFilesNFoldersListByAPI;
	DWORD					m_dwFilesCountFirstPhase;
	DWORD					m_dwFilesCountSecondPhase;
	DWORD					m_dwFileNFolderRootkitFound;
	DWORD					m_dwPercentage;
	DWORD					m_dwPrevCompletedPerc;
};
