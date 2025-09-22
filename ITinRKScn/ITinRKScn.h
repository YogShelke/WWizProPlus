#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include <Winsvc.h>
#include "PipeConstants.h"
#include "ISpyCommunicator.h"
#include "iSpyMemMapServer.h"
#include "ISpyCriticalSection.h"
#include "ISpyScanner.h"
#include "HiddenFilesNFoldersScan.h"
#include "WardwizLangManager.h"

class CITinRKScnApp : public CWinApp
{
public:
	CITinRKScnApp();
	~CITinRKScnApp() ;


	iSpyServerMemMap_Server m_objRootKitServer;

	DWORD	m_dwRootKitCount ;
	DWORD	m_dwScanProcCount ;
	DWORD	m_dwHiddenProcCount ;

	DWORD   m_totalScannedDriverCount;
	DWORD	m_totalCountProcess;

	DWORD	m_dwTotalScannedFiles ;
	DWORD	m_dwHiddenFilesCount ;

	DWORD	m_dwpercentage;
	DWORD	m_dwRootkitScanType;

	
	bool	m_bManualStop;
	bool	m_bProcess;
	bool	m_bRegistry;
	bool	m_bFileFolders;

	bool	m_bFlagResumeFileFolders;
	bool	m_bFlagResumeProcess;
	bool	m_bFlagResumeRegistry;

	BYTE	m_bScanEntries ;
	int		m_EachScanMaxPercentage ;

	//In antirootkit scan if we abort the scan,files scanned count is shown "0".
	//Niranjan Deshak - 05/03/2015.
	CISpyCommunicator		m_objCom;
	//Date			:	20 - 01 - 2015
	//Resolved by	:	Vilas
	//Issue			:	GUI not showing status because it's to fast operation
	bool	m_bRegSizeChanged;

	HANDLE	m_hThread_StartScanning ;
	HANDLE	m_hThread_StartHiddenProcScan ;
	HANDLE	m_hThread_StartHiddenDriversScan ;

	void IsWow64() ;
	DWORD ScanSystemForHiddenProcess() ;
	DWORD ScanSystemForHiddenDrivers() ;

	DWORD ReadRawRegistryInformation( ) ;
	void CompareBothDatabases( ) ;
	DWORD EnumerateDriversFromHIVEFile( ) ;
	bool SendRootKitInfoToGUI(int iMessageInfo , CString csFirstParam , CString csSecondParam , DWORD dwFirstParam , DWORD dwSecondParm);


	DWORD ChangeRootKitPath( TCHAR *pSrvName, TCHAR *pSrvPath ) ;
	DWORD DeleteDrvService( TCHAR *pszSrvName ) ;
	DWORD StopServiceManually( TCHAR *pszSrvName ) ;
	DWORD QueryDrvServiceStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus) ;

	void InitializeVariablesToZero() ;

	bool StartAntirootScan(DWORD dwType);
	bool StopAntirootScan();
	bool PauseAntirootScan();
	bool ResumeAntirootScan();
	bool SendMessage2UI(int iRequest);
	void ClearMemoryMapObjects();

	void GetScanOptionsFromDWORD( DWORD dwAntirootScanOpt) ;

	DWORD GetScanPercentage();
	DWORD GetDetectedEntriesofHiddenDrivers();
	DWORD GetDetectedEntriesofHiddenProcesses();
	DWORD GetDetectedEntriesofHiddenFileorFolders();

	//In antirootkit scan if we abort the scan,files scanned count is shown "0".
	//Niranjan Deshak - 05/03/2015.
	DWORD GetTotalCountOfAll();
	bool SendRootKitInfoToGuiThroughPipe(int iMessageInfo, DWORD dwTotalCntFile, DWORD dwTotalCntDrives, DWORD dwTotalCntProc, DWORD dwDetectedCntFile, DWORD dwDetectedCntDrives, DWORD dwDetectedCntProc);
	
	//For Process
	DWORD CheckInRunningProcess(DWORD dwPID, LPDWORD lpdwPIDs, DWORD dwProcesses ) ;
	DWORD EnumerateRunningProcessThread(  ) ;
	DWORD TerminateSpecifiedProcess( DWORD dwPID ) ;
	DWORD RepairHiddenProcess(DWORD dwPID, TCHAR *pProcPath) ;
	void BlockProcessFromExecution( TCHAR *pProcPath ) ;

	//For Rootkit
	DWORD ScanHiddenDriverss( ) ;
	DWORD RepairHiddenDriver(TCHAR *pDrvName, TCHAR *pDrvPath) ;
	bool SetStatus(CString csStatus);

	//For appending entry in Ini to delete after reboot
	void WriteToIniForRebootDelete(TCHAR *pPath, BYTE bType = 0x01 ) ;		//0x01	-> for Process

																			//0x02	-> for Driver( Rootkit)

	void LoadRequiredDLLs() ;
	void SetScanFunctionPtr(CISpyScanner *pobjISpyScanner);

	//For Files and Folders
	DWORD ScanHiddenFilesNFolders();

	void QuarantineFile( TCHAR *pFilePath ) ;

	HMODULE						m_hKernelDLL ;
	HMODULE						m_hPsApiDLL ;
	CISpyScanner				*m_pobjISpyScanner;
	CHiddenFilesNFoldersScan	m_objHiddenFiles;
	CString						m_csProdRegKey;
	CWardwizLangManager			m_objwardwizLangManager;
	bool						m_bISSuspended;

	void UpdateLastScanDateTime( );

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CITinRKScnApp theApp;
