/********************************************************************************************************** 
   Program Name          : ITinRKScn.cpp
   Description           : Class which provided exported fucntionality for outside environment 
						   for RootKit Scanning.
   Author Name           : Ramkrushna Shelke                                                                                 
   Date Of Creation      : 7/24/2014
   Version No            : 1.0.0
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description
***********************************************************************************************************/

#include "stdafx.h"
#include "ITinRKScn.h"
#include "ITinEnumDriversAPI.h"
#include "ITinEnumDriversHive.h"
#include "HiddenFilesNFoldersScan.h"
#include "NTFS.h"

#include "TreeConstants.h"
#include "ITinRegWrapper.h"

#define MYEXTERN
#include "ITinDriverInfo.h"

#include <Psapi.h>
#pragma comment(lib, "Psapi.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLEXPORT __declspec(dllexport)

DWORD WINAPI Thread_StartScanning( LPVOID lpParam );
DWORD WINAPI Thread_StartHiddenProcessScan( LPVOID lpParam ) ;

CISpyCriticalSection	g_objcriticalSection ;
BOOL					isScanning = FALSE ;


typedef BOOL (WINAPI *ENUMPROCESSMODULESEX )	( HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded, DWORD dwFilterFlag ) ;
typedef BOOL (WINAPI *WOW64DISABLEWOW64FSREDIRECTION )	( PVOID *OldValue );
typedef BOOL (WINAPI *WOW64REVERTWOW64FSREDIRECTION )	( PVOID OlValue );

ENUMPROCESSMODULESEX			EnumProcessModulesEx_Our			= NULL ;
WOW64DISABLEWOW64FSREDIRECTION	Wow64DisableWow64FsRedirection_Our	= NULL ;
WOW64REVERTWOW64FSREDIRECTION	Wow64RevertWow64FsRedirection_Our	= NULL ;


//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CITinRKScnApp

BEGIN_MESSAGE_MAP(CITinRKScnApp, CWinApp)
END_MESSAGE_MAP()


/***************************************************************************
  Function Name  : CITinRKScnApp()
  Description    : Constuctor
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 7/24/2014
****************************************************************************/
CITinRKScnApp::CITinRKScnApp() :
m_objRootKitServer(ROOTKIT)
,m_bManualStop(false)
,m_bFileFolders(false)
,m_bRegistry(false)
,m_bProcess(false)
, m_bISSuspended(false)
, m_objCom(UI_SERVER)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	OutputDebugString(L">>> In CITinRKScnApp\n");

	m_bIsWow64 = FALSE ;
	bVistaOnward = false ;

	vDriverInfo.clear() ;

	pRegRawData = NULL ;
	dwRegRawSize = 0x00 ;

	m_dwRootKitCount = 0x00 ;

	m_bISSuspended = false;
	m_hThread_StartScanning = NULL ;
	m_hThread_StartHiddenProcScan = NULL ;
	m_hThread_StartHiddenDriversScan = NULL ;

	m_bScanEntries = 0x00 ;
	m_EachScanMaxPercentage = 0x00 ;

	m_dwScanProcCount = 0x00 ;

	m_objRootKitServer.CreateServerMemoryMappedFile();

	m_hKernelDLL = NULL ;
	m_hPsApiDLL = NULL ;
}

/***************************************************************************
  Function Name  : CITinRKScnApp()
  Description    : Destructor
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 7/24/2014
****************************************************************************/
CITinRKScnApp::~CITinRKScnApp()
{
	if( pRegRawData )
		delete pRegRawData ;

	pRegRawData = NULL ;

	if( m_hKernelDLL )
		FreeLibrary( m_hKernelDLL ) ;

	m_hKernelDLL = NULL ;

	if( m_hPsApiDLL )
		FreeLibrary( m_hPsApiDLL ) ;

	m_hPsApiDLL = NULL ;

}

// The one and only CITinRKScnApp object

CITinRKScnApp theApp;

/***************************************************************************
  Function Name  : InitInstance()
  Description    : Initialize CITinRKScnApp and calls IsWow64()
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_001
  Date           : 7/24/2014
****************************************************************************/
BOOL CITinRKScnApp::InitInstance()
{
	CWinApp::InitInstance() ;

	m_csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();

	OutputDebugString(L">>> In CITinRKScnApp::InitInstance\n");

	IsWow64() ;

	return TRUE;
}

/***************************************************************************
  Function Name  : IsWow64()
  Description    : Loads the corresponding Kernel for 32Bit or 64Bit Machine
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_002
  Date           : 7/24/2014
****************************************************************************/
void CITinRKScnApp::IsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL) ;

	LPFN_ISWOW64PROCESS		IsWow64Process = NULL ;

	OSVERSIONINFO	OSVI = {0} ;

	OSVI.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
	GetVersionEx( &OSVI ) ;

	if( OSVI.dwMajorVersion > 5 )
		bVistaOnward = true ;


	IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(	GetModuleHandle( TEXT("kernel32")),
															"IsWow64Process") ;
	if( !IsWow64Process )
		return ;

	IsWow64Process( GetCurrentProcess(), &m_bIsWow64 ) ;

}

/***************************************************************************
  Function Name  : LoadRequiredDLLs()
  Description    : Loads the corresponding DLL for 32Bit or 64Bit Machine
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_003
  Date           : 7/24/2014
****************************************************************************/
void CITinRKScnApp::LoadRequiredDLLs()
{
	TCHAR	szSystem32[256] = {0} ;
	TCHAR	szTemp[256] = {0} ;

	GetSystemDirectory( szSystem32, 255 ) ;

	wsprintf(szTemp, L"%s\\Kernel32.dll", szSystem32 ) ;
	if( !m_hKernelDLL )
		m_hKernelDLL = LoadLibrary( szTemp ) ;

	Wow64DisableWow64FsRedirection_Our	= (WOW64DISABLEWOW64FSREDIRECTION ) GetProcAddress( m_hKernelDLL, "Wow64DisableWow64FsRedirection" ) ;
	Wow64RevertWow64FsRedirection_Our	= (WOW64REVERTWOW64FSREDIRECTION ) GetProcAddress( m_hKernelDLL, "Wow64RevertWow64FsRedirection" ) ;

	ZeroMemory( szTemp, sizeof(szTemp) ) ;
	wsprintf(szTemp, L"%s\\Psapi.dll", szSystem32 ) ;
	if( !m_hPsApiDLL )
		m_hPsApiDLL = LoadLibrary( szTemp ) ;

	EnumProcessModulesEx_Our	= (ENUMPROCESSMODULESEX ) GetProcAddress( m_hPsApiDLL, "EnumProcessModulesEx" ) ;
}

/***************************************************************************
  Function Name  : StartAntirootScan
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_004
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT bool StartAntirootScan(DWORD dwType)
{
	return  theApp.StartAntirootScan(dwType) ;
}

/***************************************************************************
  Function Name  : StopAntirootScan()
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_005
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT bool StopAntirootScan()
{
	return  theApp.StopAntirootScan() ;
}

/***************************************************************************
  Function Name  : PauseAntirootScan()
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_006
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT bool PauseAntirootScan()
{
	return  theApp.PauseAntirootScan() ;
}

/***************************************************************************
  Function Name  : ResumeAntirootScan()
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_007
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT bool ResumeAntirootScan()
{
	return  theApp.ResumeAntirootScan() ;
}

/***************************************************************************
  Function Name  : GetScanPercentage()
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_008
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT DWORD GetScanPercentage()
{
	return  theApp.GetScanPercentage() ;
}

/***************************************************************************
  Function Name  : GetDetectedEntriesofHiddenDrivers()
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_009
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT DWORD GetDetectedEntriesofHiddenDrivers()
{
	//In antirootkit scan if we abort the scan,files scanned count is shown "0".
	//Niranjan Deshak - 05/03/2015.
	//return  theApp.GetDetectedEntriesofHiddenDrivers() ;
	return  theApp.GetTotalCountOfAll();
}

/***************************************************************************
  Function Name  : GetDetectedEntriesofHiddenProcesses()
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_010
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT DWORD GetDetectedEntriesofHiddenProcesses()
{
	//In antirootkit scan if we abort the scan,files scanned count is shown "0".
	//Niranjan Deshak - 05/03/2015.
	//return  theApp.GetDetectedEntriesofHiddenProcesses();
	return  theApp.GetTotalCountOfAll();
}

/***************************************************************************
  Function Name  : GetDetectedEntriesofHiddenFileorFolders()
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_011
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT DWORD GetDetectedEntriesofHiddenFileorFolders()
{
	//In antirootkit scan if we abort the scan,files scanned count is shown "0".
	//Niranjan Deshak - 05/03/2015.
	//return  theApp.GetDetectedEntriesofHiddenFileorFolders();
	return  theApp.GetTotalCountOfAll();
}

/***************************************************************************
  Function Name  : RepairHiddenDriver
  Description    : Extern Function
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_012
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT DWORD RepairHiddenDriver(TCHAR *pDrvName, TCHAR *pDrvPath)
{
	return  theApp.RepairHiddenDriver(pDrvName, pDrvPath) ;
}

/***************************************************************************
  Function Name  : RepairHiddenProcess
  Description    : Exported function which gets call from outside enviroment
				   which interns call APP class function to repair hidden process.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_013
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT DWORD RepairHiddenProcess(DWORD dwPID, TCHAR *pProcPath)
{
	return  theApp.RepairHiddenProcess(dwPID, pProcPath) ;
}

/***************************************************************************
  Function Name  : SetScanFunctionPtr
  Description    : Exported Function which sets the ScanFile Function pointer from
				   outside environment.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_014
  Date           : 7/24/2014
****************************************************************************/
extern "C" DLLEXPORT void SetScanFunctionPtr(CISpyScanner *pobjISpyScanner)
{
	if(pobjISpyScanner == NULL)
	{
		return;
	}
	theApp.SetScanFunctionPtr(pobjISpyScanner) ;
}

/***************************************************************************
  Function Name  : StartAntirootScan
  Description    : Starts the Anti Rootkit Scan. Calls SendMessage2UI then InitializeVariablesToZero() and then 
				   LoadRequiredDLLs().
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_015
  Date           : 7/24/2014
****************************************************************************/
bool CITinRKScnApp::StartAntirootScan(DWORD dwType)
{
	try
	{

		//isScanning = FALSE ;
		if ( isScanning ) 
			return false ;

		SendMessage2UI(ANTIROOT_STARTED);
		InitializeVariablesToZero() ;
		LoadRequiredDLLs() ;
		//Date			:	20 - 01 - 2015
		//Resolved by	:	Vilas
		//Issue			:	GUI not showing status because it's to fast operation
		m_bRegSizeChanged = false;
		m_dwRootkitScanType = dwType ;

		m_hThread_StartScanning = ::CreateThread(	NULL, 0, (LPTHREAD_START_ROUTINE) Thread_StartScanning, 
													(LPVOID)this, 0, 0 ) ;
		//Neha Gharge & Varada Ikhar, Date:10-03-2015
		//Issue: If double clicked on Scan button, scanning completes but progrss bar shows 0%.
		isScanning = TRUE;
		Sleep( 1000 ) ;

		
		//Sleep(1000 * 100);
		::WaitForSingleObject(m_hThread_StartScanning, INFINITE);

		if(m_hThread_StartScanning )
		{
			//CloseHandle(pThis->m_hThread_StartScanning ) ;
			//TerminateThread(m_hThread_StartScanning, 0);
			//Sleep(50);
			CloseHandle( m_hThread_StartScanning ) ;
			m_hThread_StartScanning = NULL;
			m_bISSuspended = false;
			//Varada Ikhar, Date:04-03-2015, Issue: In C Drive->WardWiz Antivirus folder->Log->WRDWIZAVUI.LOG->spelled wrong(succesufully ).
			AddLogEntry(L"Terminate start scanning  thread successfully ", 0, 0, true, SECONDLEVEL);
		}

		if(!m_bManualStop)
		{
			//In antirootkit scan if we abort the scan,files scanned count is shown "0".
			//Niranjan Deshak - 05/03/2015.
			//GetDetectedEntriesofHiddenDrivers();
			//GetDetectedEntriesofHiddenProcesses();
			//GetDetectedEntriesofHiddenFileorFolders();
			GetTotalCountOfAll();
			//Varada Ikhar, Date: 26/02/2015, Issue:After rootkit scan completed,the message should have only one punctuation.
			SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_FINISHED"));

			//Varada Ikhar, Date:11-03-2015,
			//Issue: In Antirootkit scan->start & abort scan->only 'scanning started' is reflecting not 'scanning terminated'.
			AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
			AddLogEntry(L">>> Rootkit Scanning Finished.", 0, 0, true, SECONDLEVEL);
			CString csRKScanningComplete;
			csRKScanningComplete.Format(L">>> Total Scanned:		Processes = [%d]\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwScanProcCount, m_totalScannedDriverCount, m_dwTotalScannedFiles);
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			csRKScanningComplete.Format(L">>> Total Threats Found:	Processes = [%d]\t\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwHiddenProcCount, m_dwRootKitCount, m_dwHiddenFilesCount);
			AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
			AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
		
			InitializeVariablesToZero();
			ClearMemoryMapObjects();
			isScanning = FALSE;
			SendMessage2UI(ANTIROOT_FINISHED);
		}
		//Sleep(30);

	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::StartAntirootScan", 0, 0, true, SECONDLEVEL);
	}

	return true ;
}

/***************************************************************************
  Function Name  : InitializeVariablesToZero
  Description    : Initialize all the variables to Zero/False.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_016
  Date           : 7/24/2014
****************************************************************************/
void CITinRKScnApp::InitializeVariablesToZero()
{
	m_dwpercentage = 1;
	
	m_dwRootKitCount =  m_dwRootkitScanType = m_dwScanProcCount = 0x00 ;
	
	m_dwHiddenProcCount = 0x00 ;

	m_bManualStop = m_bProcess = m_bRegistry = m_bFileFolders = false ;

	m_bScanEntries = 0x00 ;

	m_EachScanMaxPercentage = 0x00 ;

	m_totalScannedDriverCount = 0x00 ;

	m_dwTotalScannedFiles = 0x00 ;
	m_dwHiddenFilesCount = 0x00 ; 
	
	m_objHiddenFiles.InitializeVariables();

	m_hThread_StartScanning = NULL ;
}

/***************************************************************************
  Function Name  : Thread_StartScanning
  Description    : Thread function to that Gets the selected option from UI and 
				   Start scanning as per settings.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_017
  Date           : 7/24/2014
****************************************************************************/
DWORD WINAPI Thread_StartScanning( LPVOID lpParam )
{
	CITinRKScnApp	*pThis = (CITinRKScnApp*)lpParam ;

	DWORD	dwRet = 0x00 ;

	__try
	{
		if( !pThis )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		//Issue : If we abort Antirootkit Scan its not reflecting on home UI Scan Type.
		//Name : Niranjan Deshak. - 4/2/2015.
		pThis->UpdateLastScanDateTime( );
		pThis->GetScanOptionsFromDWORD(pThis->m_dwRootkitScanType) ;
		/*Start Antirootkit scan->scanning hidden files & folders message is displayed below the progress bar->click on 'pause' button->then click on 'resume' button->then the 'scanning resumed' message is getting displayed.
			Niranjan Deshak - 27/02/2015. Description:The variables m_bFlagResumeFileFolders,m_bFlagResumeProcess,m_bFlagResumeRegistry are used here to get which function was running while antirootkit scan is resumed. */	
		pThis->m_bFlagResumeFileFolders = false;
		pThis->m_bFlagResumeProcess = false;
		pThis->m_bFlagResumeRegistry = false;

		AddLogEntry(L">>> In Thread_StartScanning", 0, 0, true, SECONDLEVEL);
		//Varada Ikhar, Date: 11-03-2015,
		//Issue:In Antirootkit scan->start & abort scan->only 'scanning started' is reflecting not 'scanning terminated'.
		AddLogEntry(L">>> Rootkit Scanning Started...", 0, 0, true, SECONDLEVEL);

		//for Hidden files and folders
		if (pThis->m_bFileFolders)
		{
			/*Start Antirootkit scan->scanning hidden files & folders message is displayed below the progress bar->click on 'pause' button->then click on 'resume' button->then the 'scanning resumed' message is getting displayed.
			Niranjan Deshak - 27/02/2015. Description:The variables m_bFlagResumeFileFolders,m_bFlagResumeProcess,m_bFlagResumeRegistry are used here to get which function was running while antirootkit scan is resumed. */
			pThis->m_bFlagResumeFileFolders = true;
			pThis->ScanHiddenFilesNFolders();

			pThis->m_bFlagResumeFileFolders = false;
		}
		//Scan Hidden processes
		if (pThis->m_bProcess)
		{
		
			pThis->m_bFlagResumeProcess = true;
			pThis->EnumerateRunningProcessThread();
		
			pThis->m_bFlagResumeProcess = false;
		}
		//Scan hidden drivers
		if (pThis->m_bRegistry)
		{
	
			pThis->m_bFlagResumeRegistry = true;
			pThis->ScanHiddenDriverss();
	
			pThis->m_bFlagResumeRegistry = false;
		}
		//Sleep( 1000 ) ;
		Sleep(10);
		if( (pThis->m_bScanEntries) && (pThis->m_dwpercentage != 100) )
		{
			pThis->m_dwpercentage = 100 ;
			GetScanPercentage() ;
			
			Sleep(10);
			//Sleep( 1000 ) ;
			pThis->m_dwpercentage = 0;
		//	GetScanPercentage();
		}

		//Issue : If we abort Antirootkit Scan its not reflecting on home UI Scan Type.
		//pThis->UpdateLastScanDateTime( );

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::Thread_StartScanning", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

/***************************************************************************
  Function Name  : StopAntirootScan
  Description    : To stop the AntiRootScan, making isScanning variable false.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_018
  Date           : 7/24/2014
****************************************************************************/
bool CITinRKScnApp::StopAntirootScan()
{
	if( !isScanning )
	{
		return true ;
	}

	/*	ISSUE NO - 724 725 698 NAME - Neha G. & NITIN K. TIME - 15th June 2014 */

	m_bManualStop = true;

	if(m_hThread_StartScanning!= NULL)
	{
	//	ClearMemoryMapObjects();
		::SuspendThread(m_hThread_StartScanning);
		::TerminateThread(m_hThread_StartScanning, 0);
	//	Sleep(100);
		CloseHandle( m_hThread_StartScanning ) ;
		m_hThread_StartScanning = NULL;
		m_bISSuspended = false;

		isScanning = FALSE ;
	}
	//In antirootkit scan if we abort the scan,files scanned count is shown "0".
	//Niranjan Deshak - 05/03/2015.
	//GetDetectedEntriesofHiddenFileorFolders();
	//GetDetectedEntriesofHiddenProcesses();
	//GetDetectedEntriesofHiddenDrivers();
	GetTotalCountOfAll();

	//Varada Ikhar, Date: 05/02/2015
	//Issue: In Antirootkit Scan, if scanning is in progress, tried to exit from tray and choosed to stop scanning,
	//Status is showing 'Scanning completed.'
	SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_ABORTED"));

	//Varada Ikhar, Date:11-03-2015,
	//Issue: In Antirootkit scan->start & abort scan->only 'scanning started' is reflecting not 'scanning terminated'.
	AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	AddLogEntry(L">>> Rootkit Scanning Aborted.", 0, 0, true, SECONDLEVEL);
	CString csRKScanningComplete;
	csRKScanningComplete.Format(L">>> Total Scanned:		Processes = [%d]	\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwScanProcCount, m_totalScannedDriverCount, m_dwTotalScannedFiles);
	AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
	csRKScanningComplete.Format(L">>> Total Threats Found:	Processes = [%d]\t\t:: Registry Entries = [%d]\t:: Files & Folders = [%d]", m_dwHiddenProcCount, m_dwRootKitCount, m_dwHiddenFilesCount);
	AddLogEntry(csRKScanningComplete, 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"---------------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	
	InitializeVariablesToZero();
	ClearMemoryMapObjects();
	isScanning = FALSE;
	SendMessage2UI(ANTIROOT_FINISHED);

	return true;
}

/***************************************************************************
  Function Name  : PauseAntirootScan
  Description    : To Pause the running scan.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_019
  Date           : 7/24/2014
****************************************************************************/
bool CITinRKScnApp::PauseAntirootScan()
{
	if (!isScanning) 
	{
		return true;
	}

	if (m_bISSuspended)
		return true;

	if(m_hThread_StartScanning!= NULL)
	{
		::SuspendThread(m_hThread_StartScanning);
		m_bISSuspended = true;
	}
	return true;
}

/***************************************************************************
  Function Name  : ResumeAntirootScan
  Description    : To Resume the running scan.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_020
  Date           : 7/24/2014
****************************************************************************/
bool CITinRKScnApp::ResumeAntirootScan()
{
	OutputDebugStringW(L"WrdWizRkScan::Scanning resumed");
	if (!isScanning) {return true;}

	if(m_hThread_StartScanning!= NULL)
	{
		/*Start Antirootkit scan->scanning hidden files & folders message is displayed below the progress bar->click on 'pause' button->then click on 'resume' button->then the 'scanning resumed' message is getting displayed.
		Niranjan Deshak - 27/02/2015. Description:The variables m_bFlagResumeFileFolders,m_bFlagResumeProcess,m_bFlagResumeRegistry are used here to get which function was running while antirootkit scan is resumed. */
		if (m_bFlagResumeFileFolders == true)
		{
			SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_FILES_FOLDERS"));
		}
		else if (m_bFlagResumeRegistry == true)
		{
			SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_DRIVERS"));
		}
		else if (m_bFlagResumeProcess == true)
		{
			SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_PROCESS"));
		}
		::ResumeThread(m_hThread_StartScanning);
		m_bISSuspended = false;
	}

	return true;
}

/***************************************************************************
  Function Name  : CheckInRunningProcess
  Description    : To Resume the running scan.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_021
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::CheckInRunningProcess(DWORD dwPID, LPDWORD lpdwPIDs, DWORD dwProcesses )
{
	DWORD	dwRet = 0x00, i= 0x00 ;

	__try
	{

		if( IsBadReadPtr( lpdwPIDs, sizeof(DWORD) * dwProcesses ) )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		for(; i<dwProcesses; i++ )
		{
			if( dwPID == lpdwPIDs[i] )
			{
				dwRet = 0x01 ;
				break ;
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::CheckInRunningProcess", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet ;
}

/***************************************************************************
  Function Name  : EnumerateRunningProcessThread
  Description    : It Checks for Number of threads in a Sungle Process.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_022
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::EnumerateRunningProcessThread(  )
{
	SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_PROCESS"));

	DWORD	dwPID[0x100] = {0} ;
	DWORD	dwProcesses = 0x00, dwTempPerc = 0x00, i=0x00 ;
	int		dwPercentage = 0x00 ;

	TCHAR	szProcPath[1024] = {0} ;
	TCHAR	szProcName[256] = {0} ;
	TCHAR	*pProcName = NULL, *pTemp ;

	try
	{
		AddLogEntry(L">>> scanning hidden process", 0, 0, true, ZEROLEVEL);
		EnumProcesses(dwPID, 0x400, &dwProcesses ) ;
		dwProcesses = dwProcesses/sizeof(DWORD) ;

		DWORD	dwHiddenProc = 0x100000 ; //0xFFFFFFFC ;

		m_dwHiddenProcCount = 0x00 ;
		
		DWORD dwPreviousPer = m_dwpercentage;

		//Ram: Issue Fixed: 0001114
		//Slight sleep is required here to update following message on UI.
		Sleep(1000);
		CString csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_PROCESS");
		SetStatus(csStatus);

		//SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_PROCESS"));
		//for( i=0x08 ; i<=dwHiddenProc; i+=0x04 )
		for( i=0x08 ; i<0x100004; i+=0x04 )
		{
			//Commented as because of performance issue, moved to out of this loop.
			//SetStatus(csStatus);
			//if( i < 0x08 )
			//	break ;
			m_dwScanProcCount++ ;
			dwPercentage = int(((float)i/dwHiddenProc)*100)  ;
			m_dwpercentage =  dwPreviousPer + (( m_EachScanMaxPercentage * dwPercentage ) / 100 );

			if( m_dwpercentage && (dwTempPerc != m_dwpercentage ) )
			{
				/*
				ISSUE NO - 2 In antirootkit scan ,while scanning "hidden items in windows registry" 
				and hidden items in running processess" the progress bar shows 0.
				NAME - Niranjan Deshak. - 10th Jan 2015
				*/
				Sleep( 100 ) ;
				GetScanPercentage() ;
				dwTempPerc = m_dwpercentage  ;
				//This line is only for testing purpose
				//SendRootKitInfoToGUI(SHOWVIRUSFOUND,L"Process Name",L"ProcessPath", 0x408 ,0x01) ;

				//Date	:	19 - 12 - 2014
				//Issue :	Progress bar shows 0% and dialog box shows "Rootkit scanning completed" when we select 
				//			"Hidden Items in Running Process" option only from Main GUI.
					
				if( (!m_bRegistry) && (!m_bFileFolders) && m_dwpercentage<50)
					Sleep(100);
					

			}

			if( CheckInRunningProcess(i, dwPID, dwProcesses ) )
				continue ;

			HMODULE		hMods[1024] = {0} ;
			HANDLE		hProcess = NULL ;

			DWORD		dwModules = 0x00 ;

			memset(szProcPath, 0x00, 1024 ) ;
			memset(szProcName, 0x00, 256 ) ;

	// This part is commented for just testing of communication code

			//hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, i ) ;
			hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, i ) ;
			if( !hProcess )
				continue ;

			if( EnumProcessModulesEx_Our )
			{
				if( !EnumProcessModulesEx_Our(hProcess, hMods, sizeof(hMods), &dwModules, LIST_MODULES_ALL ) )
				{
					DWORD error = GetLastError();
					CloseHandle( hProcess ) ;
					continue ;
				}
			}
			else
			{
				if( !EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules ) )
				{
					DWORD error = GetLastError();
					CloseHandle( hProcess ) ;
					continue ;
				}
			}

			GetModuleFileNameEx(hProcess, hMods[0], szProcPath, 1023 ) ;
			if( !szProcPath[0] )
			{
				CloseHandle( hProcess ) ;
				continue ;
			}

			CloseHandle( hProcess ) ;
			hProcess = NULL ;

			//SendProcessPathPID_ToGUI(szProcName, i ,0x01) ;//0x01 for process
			//Please send in 

			pProcName = wcsrchr(szProcPath, '\\' ) ;
			if( pProcName )
			{
				pTemp = pProcName++ ;
				wcscpy(szProcName, pProcName ) ;
				*pTemp = '\0' ;
				SendRootKitInfoToGUI(SHOWVIRUSFOUND, szProcName, szProcPath, i ,0x01) ; //format 0x01 is forp  process
			}

			m_dwHiddenProcCount++ ;
			Sleep(10);
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::EnumerateRunningProcessThread", 0, 0, true, SECONDLEVEL);
	}

	SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_PROCESS_FINISHED"));
	AddLogEntry(L">>> Hidden process scan finished", 0, 0, true, ZEROLEVEL);

	//Sleep( 1000 ) ;
	//Sleep(10);
	return 0 ;
}

/***************************************************************************
  Function Name  : ScanHiddenFilesNFolders
  Description    : Scans Hidden Files and Folders, Caculates the Percentage Scan of Hidden Files and Folder.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_023
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::ScanHiddenFilesNFolders()
{
	AddLogEntry(L">>> Scanning hidden files and folders", 0, 0, true, ZEROLEVEL);

	TCHAR szWindowsDir[MAX_PATH] = {0};
	GetWindowsDirectory( szWindowsDir, MAX_PATH) ;

	try
	{
		m_objHiddenFiles.InitializeVariables();
		m_objHiddenFiles.SetPreviousPercentage(m_dwpercentage);

		m_objHiddenFiles.CalculateScanPercentage();

		SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_FILES_FOLDERS"));//this is only for testing

		if(!m_objHiddenFiles.CreateDriveTreeByAPIs(szWindowsDir))
		{
			AddLogEntry(L"### Failed to CreateDriveTreeByAPIs in CVibraniumRKScnApp::ScanHiddenFilesNFolders", 0, 0, true, SECONDLEVEL);
			return 1;
		}

		m_objHiddenFiles.CalculateScanPercentage();

		if(!m_objHiddenFiles.ScanDrive(szWindowsDir))
		{
			AddLogEntry(L"### Failed ScanDrive in CVibraniumRKScnApp::ScanHiddenFilesNFolders", 0, 0, true, SECONDLEVEL);
			return 1;
		}

		m_objHiddenFiles.CalculateScanPercentage();
		//In antirootkit scan if we abort the scan,files scanned count is shown "0".
		//Niranjan Deshak - 05/03/2015.
		//GetDetectedEntriesofHiddenFileorFolders();
		GetTotalCountOfAll();

	}
	catch( ... )
	{
		AddLogEntry(L"### Failed ScanDrive in CVibraniumRKScnApp::ScanHiddenFilesNFolders", 0, 0, true, SECONDLEVEL);
	}

	SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_FILE_FOLDER_FINISHED"));//this is only for testing
	AddLogEntry(L">>> Hidden files and folders scan finished", 0, 0, true, ZEROLEVEL);

	return 0 ;
}

/***************************************************************************
  Function Name  : ScanHiddenDriverss
  Description    : Scans Hidden Drivers, Caculates the Percentage Scan of Hidden Files and Folder.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_024
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::ScanHiddenDriverss( )
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwPrevCompletedPerc = 0x00 ;
	

	AddLogEntry(L">>> Scanning hidden drivers", 0, 0, true, ZEROLEVEL);

	try
	{
		dwPrevCompletedPerc = m_dwpercentage ;
		//Enumerate Driver Info through WIN API ie using SCM
		CITinEnumDriversAPI		objEnumDriverAPI ;
		
		AddLogEntry(L">>> before getinial values", 0, 0, true, ZEROLEVEL);
		objEnumDriverAPI.GetInitialValues() ;
		AddLogEntry(L">>> after getinial values", 0, 0, true, ZEROLEVEL);
		
		AddLogEntry(L">>> before enumertateDriverServicesthtouthAPI values", 0, 0, true, ZEROLEVEL);
		objEnumDriverAPI.EnumerateDriverServicesThrouthAPI( ) ;
		AddLogEntry(L">>> after enumertateDriverServicesthtouthAPI values", 0, 0, true, ZEROLEVEL);
		
		Sleep(10);
		m_dwpercentage = dwPrevCompletedPerc + (int)(m_EachScanMaxPercentage/4) ;
		GetScanPercentage() ;
	
		SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_DRIVERS"));//this is only for testing

		//Date			:	19 - 12 - 2014
		//Resolved by	:	Vilas
		//Issue			:	GUI not showing status because it's to fast operation
		if( (!m_bProcess) && (!m_bFileFolders) )
			Sleep(1000);

		//Read Raw Registry Information
		if( ReadRawRegistryInformation( ) )
		{
			AddLogEntry(L"### Error in CVibraniumRKScnApp::ReadRawRegistryInformation", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01 ;
		}
		
		AddLogEntry(L">>> after ReadRawRegistryInformation", 0, 0, true, ZEROLEVEL);
		Sleep(10);
		
		m_dwpercentage = dwPrevCompletedPerc + (int)((2*m_EachScanMaxPercentage)/4) ;
		GetScanPercentage() ;

		//Date			:	19 - 12 - 2014
		//Resolved by	:	Vilas
		//Issue			:	GUI not showing status because it's to fast operation
		if( (!m_bProcess) && (!m_bFileFolders) )
			Sleep(1000);

		//Date			:	20 - 01 - 2015
		//Resolved by	:	Vilas
		//Issue			:	GUI not showing status because it's to fast operation
		if( !m_bRegSizeChanged )
			m_totalScannedDriverCount = static_cast<DWORD>(vDriverInfo_Hive.size());

		if( (!dwRet) && (m_bRegSizeChanged) )
		{
			if( EnumerateDriversFromHIVEFile( ) )
			{
				AddLogEntry(L"### Error in CVibraniumRKScnApp::EnumerateDriversFromHIVEFile", 0, 0, true, SECONDLEVEL);
				dwRet = 0x02 ;
			}
			AddLogEntry(L">>> after EnumerateDriversFromHIVEFile", 0, 0, true, ZEROLEVEL);
		}
		
		Sleep(10);
		m_dwpercentage = dwPrevCompletedPerc + (int)((3*m_EachScanMaxPercentage)/4) ;
		GetScanPercentage() ;

		//Date			:	19 - 12 - 2014
		//Resolved by	:	Vilas
		//Issue			:	GUI not showing status because it's to fast operation
		if( (!m_bProcess) && (!m_bFileFolders) )
			Sleep(1000);

		if( !dwRet )
		{
			CompareBothDatabases( ) ;
		}

		Sleep(10);
		m_dwpercentage = dwPrevCompletedPerc + m_EachScanMaxPercentage ;
		GetScanPercentage() ;

		//Date			:	19 - 12 - 2014
		//Resolved by	:	Vilas
		//Issue			:	GUI not showing status because it's to fast operation
		if( (!m_bProcess) && (!m_bFileFolders) )
			Sleep(1000);

		SetStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_ANTIROOTKIT_SCAN_HIDDEN_DRIVERS_FINISHED"));//this is only for testing

	}
	catch( ... )
	{
		AddLogEntry(L"### Error in CVibraniumRKScnApp::ScanHiddenDriverss", 0, 0, true, SECONDLEVEL);
	}

	return dwRet ;
}

/***************************************************************************
  Function Name  : ReadRawRegistryInformation
  Description    : Reads the Registry Information.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_025
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::ReadRawRegistryInformation( )
{
	DWORD	dwRet = 0x00 ;

	TCHAR	szRawRegPath[256] = {0} ;
	TCHAR	volname = 0x00 ;

	BOOL	bRet = FALSE ;
	PVOID	OldValue = FALSE ;

	try
	{

		GetWindowsDirectory( szRawRegPath,  255 ) ;
		if( !szRawRegPath[0] )
		{
			dwRet = 0x01 ;
			//return dwRet ;
			goto Cleanup ;
		}

		if( m_bIsWow64 )
		{
			wcscat(szRawRegPath, L"\\System32\\Config\\SYSTEM" ) ;
			if( Wow64DisableWow64FsRedirection_Our )
				bRet = Wow64DisableWow64FsRedirection_Our( &OldValue ) ;
		}
		else
			wcscat(szRawRegPath, L"\\System32\\Config\\SYSTEM" ) ;

		volname = szRawRegPath[0] ;
		CNTFSVolume		volume(volname) ;

		if (!volume.IsVolumeOK())
		{
			dwRet = 0x02 ;
			//return dwRet ;
			goto Cleanup ;
		}

		CFileRecord fr(&volume) ;

		fr.SetAttrMask(MASK_INDEX_ROOT | MASK_INDEX_ALLOCATION);

		if (!fr.ParseFileRecord(MFT_IDX_ROOT))
		{
			dwRet = 0x03 ;
			//return dwRet ;
			goto Cleanup ;
		}

		if (!fr.ParseAttrs())
		{
			dwRet = 0x04 ;
			//return dwRet ;
			goto Cleanup ;
		}

		// find subdirectory
		CIndexEntry		ie ;
		
		CString			m_filename ;

		m_filename.Format( L"%s", szRawRegPath ) ;

		int dirs = m_filename.Find(_T('\\'), 0);
		int dire = m_filename.Find(_T('\\'), dirs+1);
		while (dire != -1)
		{
			CString		pathname = m_filename.Mid(dirs+1, dire-dirs-1);

			if (fr.FindSubEntry((const _TCHAR*)pathname, ie))
			{
				if (!fr.ParseFileRecord(ie.GetFileReference()))
				{
					dwRet = 0x05 ;
					//return dwRet ;
					break ;
				}

				if (!fr.ParseAttrs())
				{
				
					dwRet = 0x06 ;
					//return dwRet ;
					break ;
				}
			}
			else
			{
				dwRet = 0x07 ;
				//return dwRet ;
				break ;
			}

			dirs = dire;
			dire = m_filename.Find(_T('\\'), dirs+1);
		}

		if( dwRet )
			goto Cleanup ;

		CString filename = m_filename.Right(m_filename.GetLength()-dirs-1);
		if( fr.FindSubEntry((const _TCHAR*)filename, ie) )
		{
			if (!fr.ParseFileRecord(ie.GetFileReference()))
			{
				dwRet = 0x08 ;
				//return dwRet ;
				goto Cleanup ;
			}

			// We only need DATA attribute and StdInfo
			fr.SetAttrMask(MASK_DATA);
			if (!fr.ParseAttrs())
			{
			
				dwRet = 0x09 ;
				//return dwRet ;
				goto Cleanup ;
			}

			const CAttrBase *data = fr.FindStream() ;
			if( !data )
			{
				dwRet = 0x0A ;
				//return dwRet ;
				goto Cleanup ;
			}

			//Issue	: Crashing due to multiple time scan from UI
			//Date	: 20 - Jan - 2015

			//dwRegRawSize = (DWORD)data->GetDataSize() ;
			DWORD	dwRegistryRawSize = (DWORD)data->GetDataSize() ;
			if( !dwRegistryRawSize )
			{
				dwRet = 0x0B ;
				//return dwRet ;
				goto Cleanup ;
			}

			if( dwRegistryRawSize != dwRegRawSize )
			{
				//Date			:	20 - 01 - 2015
				//Resolved by	:	Vilas
				//Issue			:	GUI not showing status because it's to fast operation
				m_bRegSizeChanged = true;
				dwRegRawSize = dwRegistryRawSize;
			}
			else
			{
				m_bRegSizeChanged = false;
				dwRet = 0x00 ;
				goto Cleanup ;
			}

			if( pRegRawData )
				delete pRegRawData ;

			pRegRawData = NULL ;
			pRegRawData = new BYTE[ dwRegRawSize +1 ];

			if( pRegRawData )
			{
				DWORD	len = 0x00 ;

				ZeroMemory(pRegRawData, dwRegRawSize+1 ) ;
				data->ReadData(0, pRegRawData, dwRegRawSize, &len) ;
				if( len == dwRegRawSize )
				{
					//WriteToFile( filebuf, datalen ) ;
					dwRet = 0x00 ;
					//return dwRet ;
					goto Cleanup ;
				}
				else
				{
					dwRet = 0x0C ;
					//return dwRet ;
					goto Cleanup ;
				}
			}
		}
		else
		{
			dwRet = 0x0D ;
		}

	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::ReadRawRegistryInformation", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	if( m_bIsWow64 && Wow64RevertWow64FsRedirection_Our)
			Wow64RevertWow64FsRedirection_Our( OldValue );

	return dwRet ;
}

/***************************************************************************
  Function Name  : EnumerateDriversFromHIVEFile
  Description    : Function which reads the hive file from hard disk, and maintains the list.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_026
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::EnumerateDriversFromHIVEFile( )
{
    char	path[0x4000] = {0} ;
	DWORD	dwRet = 0x00 ;

	TCHAR	szTemp[256] = {0} ;

	try
	{
		AddLogEntry(L">>> inside EnumerateDriversFromHIVEFile", 0, 0, true, ZEROLEVEL);
		if( !pRegRawData )
		{
			AddLogEntry(L"### returning EnumerateDriversFromHIVEFile dwRet = 0x01", 0, 0, true, ZEROLEVEL);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( !dwRegRawSize )
		{
			AddLogEntry(L"### returning EnumerateDriversFromHIVEFile dwRet = 0x02", 0, 0, true, SECONDLEVEL);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if( (*( (DWORD * )&pRegRawData[0] ) ) != 0x66676572 )
		{
			AddLogEntry(L"### returning EnumerateDriversFromHIVEFile dwRet = 0x03", 0, 0, true, ZEROLEVEL);
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		vDriverInfo_Hive.clear() ;

		ITinEnumDriversHive EnumDriverHive ;

		AddLogEntry(L">>> before EnumDriverHive.walk", 0, 0, true, ZEROLEVEL);
		EnumDriverHive.walk(path,(key_block*)(pRegRawData+0x1020));
		AddLogEntry(L">>> after EnumDriverHive.walk", 0, 0, true, ZEROLEVEL);

		if( vDriverInfo_Hive.empty() )
		{
			AddLogEntry(L"### returning EnumerateDriversFromHIVEFile dwRet = 0x04", 0, 0, true, ZEROLEVEL);
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		m_totalScannedDriverCount = static_cast<DWORD>(vDriverInfo_Hive.size());
		AddLogEntry(L">>> end of EnumerateDriversFromHIVEFile", 0, 0, true, ZEROLEVEL);
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::EnumerateDriversFromHIVEFile", 0, 0, true, SECONDLEVEL);
	}
Cleanup :
	wsprintf(szTemp, L"Scanned hidden Drivers Entries:%lu", m_totalScannedDriverCount ) ;
	AddLogEntry( szTemp ) ;
    return dwRet ;
}

/***************************************************************************
  Function Name  : CompareBothDatabases
  Description    : Compares the DataBases.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_027
  Date           : 7/24/2014
****************************************************************************/
void CITinRKScnApp::CompareBothDatabases( )
{
	DWORD	dwDrvAPI = static_cast<DWORD>(vDriverInfo.size());
	DWORD	dwDrvHive = static_cast<DWORD>(vDriverInfo_Hive.size());

	DWORD	i=0x00, j=0x00 ;
	bool	bFound = false ;

	TCHAR	szDrvPath_Hive[512] = {0} ;
	TCHAR	szDrvPath_API[512] = {0} ;

	CITinEnumDriversAPI		objDrvAPI ;

	TCHAR	szTemp[256] = {0} ;

	try
	{

		objDrvAPI.GetInitialValues() ;
		// this line is only for testing purpose
		
		for(i=0x00; i<dwDrvHive; i++ )
		{
			bFound = false ;
			ZeroMemory(szDrvPath_Hive, sizeof(szDrvPath_Hive) ) ;

			if( !objDrvAPI.GetServicePath(vDriverInfo_Hive[i].szDriverPath, szDrvPath_Hive) )
				continue ;

			for(j=0x00; j<dwDrvAPI; j++ )
			{
				ZeroMemory(szDrvPath_API, sizeof(szDrvPath_API) ) ;
				if( !objDrvAPI.GetServicePath(vDriverInfo[j].szDriverPath, szDrvPath_API) )
					continue ;

				if( _wcsicmp(szDrvPath_Hive, szDrvPath_API) == 0x00 )
				{
					bFound = true ;
					break ;
				}
			}

			bFound = true ;		//Forcefully done for this setup on 14/06/2014 by Vilas
			if( !bFound )
			{
				//Send to User GUI about RootKit detection
				m_dwRootKitCount++ ;
				//SendRootKitInfoToGUI(szDrvPath_Hive, vDriverInfo_Hive[i].szDriverName,0x03 )  ;
				//szDrvPath_Hive						-->	Driver Path
				//vDriverInfo_Hive[i].szDriverName		--> Driver Name
				SendRootKitInfoToGUI(SHOWVIRUSFOUND, szDrvPath_Hive, vDriverInfo_Hive[i].szDriverName, 0, 0x02) ; //format. 0x03 type for file/folders
			}

		}
	}

	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::CompareBothDatabases", 0, 0, true, SECONDLEVEL);
	}


	wsprintf(szTemp, L"Hidden Drivers Entries:%lu", m_dwRootKitCount ) ;
	AddLogEntry( szTemp ) ;

}

/***************************************************************************
  Function Name  : SendRootKitInfoToGUI
  Description    : Sends the RootKit Scan Information to the GUI.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_028
  Date           : 7/24/2014
****************************************************************************/
bool CITinRKScnApp::SendRootKitInfoToGUI(int iMessageInfo , CString csFirstParam , CString csSecondParam , DWORD dwFirstParam , DWORD dwSecondParm)
{
	g_objcriticalSection.Lock();
	
	ITIN_MEMMAP_DATA iTinMemMap = {0};
	iTinMemMap.iMessageInfo = iMessageInfo;
	_tcscpy(iTinMemMap.szFirstParam,csFirstParam);
	_tcscpy(iTinMemMap.szSecondParam,csSecondParam);
	iTinMemMap.dwFirstValue = dwFirstParam;
	iTinMemMap.dwSecondValue = dwSecondParm;

	if(m_objRootKitServer.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap)))
	{
		AddLogEntry(L"### Server side failed in UpdateServerMemoryMappedFile", 0, 0, true, SECONDLEVEL);
	}

	//Sleep(50);
	g_objcriticalSection.Unlock();

	return true;

}



///Rootkit Repair
/***************************************************************************
  Function Name  : QueryDrvServiceStatus
  Description    : RootKit Repair
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_029
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::QueryDrvServiceStatus( SC_HANDLE hService, LPDWORD lpdwServiceStatus)
{
	LPQUERY_SERVICE_CONFIG	lpsc = NULL ;

	DWORD					dwStatus = 0x00 ;
	DWORD					dwBytesNeeded = 0x00, dwSize = 0x00 ;

	__try
	{

		if( hService == NULL )
		{
			dwStatus = 0x01 ;
			__leave ;
		}

		QueryServiceConfig( hService, NULL, 0, &dwBytesNeeded ) ;
		if( !dwBytesNeeded )
		{
			dwStatus = 0x02 ;
			__leave ;
		}

		dwSize = dwBytesNeeded ;

		lpsc = (LPQUERY_SERVICE_CONFIG ) LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT, dwSize ) ;
		if( !lpsc )
		{
			dwStatus = 0x03 ;
			__leave ;
		}

		if( !QueryServiceConfig(hService, lpsc, dwSize, &dwBytesNeeded ) )
		{
			dwStatus = 0x04 ;
			__leave ;
		}

		*lpdwServiceStatus = lpsc->dwStartType ;
	}
	__finally
	{
		if( lpsc )
		{
			LocalFree( lpsc ) ;
			lpsc = NULL ;
		}
	}

	return dwStatus ;
}

/***************************************************************************
  Function Name  : StopServiceManually
  Description    : Stop the Services Manually.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_030
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::StopServiceManually( TCHAR *pszSrvName )
{
	DWORD	dwRetValue = 0x00 ;


	SC_HANDLE	schSCManager = NULL ;
    SC_HANDLE	schService = NULL ;
	DWORD		dwServiceStartType = 0xFFFFFFFF ;

	SERVICE_STATUS	ServiceStatus = {0} ;

	__try
	{

		schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( schSCManager == NULL )
		{
			dwRetValue = 0x01 ;
			__leave ;
		}

		//schService = OpenService( schSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
		schService = OpenService( schSCManager, pszSrvName, SERVICE_STOP ) ;
		if( !schService )
		{
			dwRetValue = 0x02 ;
			__leave ;
		}

		//Sleep( 100 ) ;
		Sleep(10);

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
		if( (ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) ||
			(ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) )
		{
			//Sleep( 1000 ) ;
			Sleep(10);
			__leave ;
		}

		if( QueryDrvServiceStatus( schService, &dwServiceStartType ) )
		{
			dwRetValue = 0x03 ;
			__leave ;
		}

		if( dwServiceStartType == SERVICE_DISABLED ||
			(dwServiceStartType == SERVICE_DEMAND_START) )
		{
			if( !ChangeServiceConfig(	schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
										SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
										NULL, NULL, NULL ) )
			{
				dwRetValue = 0x04 ;
				__leave ;
			}

			//Sleep( 100 ) ;
			Sleep(30);
			if( !ControlService( schSCManager, SERVICE_CONTROL_STOP, &ServiceStatus ) )
			{
				dwRetValue = 0x05 ;
				__leave ;
			}
		}

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
		if( (ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) ||
			(ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING) )
		{
			dwRetValue = 0x00 ;
			__leave ;
		}

		dwRetValue = 0x06 ;
	}
	__finally
	{
		if( schService )
		{
			CloseServiceHandle(schService) ;
			schService = NULL ;
		}

		if( schSCManager )
		{
			CloseServiceHandle(schSCManager) ;
			schSCManager = NULL ;
		}

	}

	return dwRetValue ;
}

/***************************************************************************
  Function Name  : DeleteDrvService
  Description    : To Delete the Drv Services.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_031
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::DeleteDrvService( TCHAR *pszSrvName )
{
	DWORD	dwRetValue = 0x00 ;


	SC_HANDLE	schSCManager = NULL ;
    SC_HANDLE	schService = NULL ;
	DWORD		dwServiceStartType = 0xFFFFFFFF ;

	SERVICE_STATUS		ServiceStatus = {0} ;
	//TCHAR		szTemp[256] = {0} ;

	__try
	{

		schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ) ;
		if( schSCManager == NULL )
		{
			dwRetValue = 0x01 ;
			__leave ;
		}

		//schService = OpenService( schSCManager, pszSrvName, SERVICE_ALL_ACCESS ) ;
		schService = OpenService( schSCManager, pszSrvName, SERVICE_STOP | DELETE ) ;
		if( !schService )
		{
			dwRetValue = 0x02 ;
			__leave ;
		}

		//Sleep( 100 ) ;
		Sleep(10);

		ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
		//Sleep( 100 ) ;
		Sleep(10);
		if( ServiceStatus.dwCurrentState==SERVICE_RUNNING )
		{
			//Sleep( 1000 ) ;
			Sleep(10);
			StopServiceManually( pszSrvName ) ;
		}

		if( ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING )
			Sleep(10);
			//Sleep( 1000 ) ;

		if( !DeleteService( schService ) )
		{
			if( QueryDrvServiceStatus( schService, &dwServiceStartType ) )
			{
				dwRetValue = 0x03 ;
				__leave ;
			}

			if( dwServiceStartType == SERVICE_DISABLED ||
				(dwServiceStartType == SERVICE_DEMAND_START) )
			{
				if( !ChangeServiceConfig(	schService, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
											SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
											NULL, NULL, NULL ) )
				{
					dwRetValue = 0x04 ;
					__leave ;
				}

				//Sleep( 100 ) ;
				Sleep(10);
				ControlService( schSCManager, SERVICE_CONTROL_STOP, &ServiceStatus ) ;
				if( !DeleteService( schService ) )
				{
					dwRetValue = 0x05 ;
					__leave ;
				}
			}
		}
	}
	__finally
	{
		if( schService )
		{
			CloseServiceHandle(schService) ;
			schService = NULL ;
		}

		if( schSCManager )
		{
			CloseServiceHandle(schSCManager) ;
			schSCManager = NULL ;
		}

	}

	return dwRetValue ;
}

/***************************************************************************
  Function Name  : ChangeRootKitPath
  Description    : To change the Registry Service Path.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_032
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::ChangeRootKitPath( TCHAR *pSrvName, TCHAR *pSrvPath )
{
	DWORD	dwRet = 0x00 ;
	HKEY	hSubKey = NULL ;

	TCHAR	szSubKey[256] = {0} ;

	swprintf(szSubKey, L"SYSTEM\\CurrentControlSet\\services\\%s", pSrvName ) ;

	if( RegCreateKeyEx(	HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
						NULL, &hSubKey, NULL) != ERROR_SUCCESS )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::ChangeRootKitPath():Can't Open Registry Service Name", 0, 0, true, SECONDLEVEL);
		return 0x01 ;
	}

	TCHAR	szSrvPath[512] = {0} ;
	DWORD	dwSize = 0x00 ;

	wcscpy( szSrvPath, pSrvPath ) ;
	dwSize = static_cast<DWORD>(wcslen(szSrvPath));
	szSrvPath[ dwSize-3 ] = 'V' ;

	dwRet = SERVICE_DISABLED ;
	RegSetValueEx(hSubKey, L"Start", 0, REG_DWORD, (LPBYTE)&dwRet, sizeof(dwRet) ) ;
	dwRet = RegSetValueEx(hSubKey, L"ImagePath", 0, REG_SZ, (LPBYTE)szSrvPath, dwSize*sizeof(TCHAR) ) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if( dwRet != ERROR_SUCCESS )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::ChangeRootKitPath():Can't set Registry Service Path", 0, 0, true, SECONDLEVEL);
		return 0x02 ;
	}

	AddLogEntry(L"### Exception in CVibraniumRKScnApp::ChangeRootKitPath():Registry Service Path set successfully", 0, 0, true, SECONDLEVEL);
	return 0x00 ;
}

/***************************************************************************
  Function Name  : RepairHiddenDriver
  Description    : Function which repaires the detected hidden drivers.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_033
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::RepairHiddenDriver(TCHAR *pDrvName, TCHAR *pDrvPath)
{
	DWORD	dwRet = 0x00 ;

	try
	{

		dwRet = DeleteDrvService( pDrvName ) ;
		if( !dwRet )
		{
			//Quarantine the Driver File
			QuarantineFile( pDrvPath ) ;
			//return dwRet ;
			goto Cleanup ;
		}

		dwRet = ChangeRootKitPath(pDrvName, pDrvPath ) ;

		//Quarantine the Driver File in next reboot and tell user to reboot
		//We need to keep database which indicates to quarantine files in next reboot to our "ITinCommService" service
		WriteToIniForRebootDelete(pDrvPath, 0x02 ) ;
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::RepairHiddenDriver", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet ;
}

/***************************************************************************
  Function Name  : RepairHiddenProcess
  Description    : To repair a specified Process from execution.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_034
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::RepairHiddenProcess(DWORD dwPID, TCHAR *pProcPath)
{
	DWORD	dwRet = 0x00 ;

	try
	{

		dwRet = TerminateSpecifiedProcess( dwPID ) ;
		if( dwRet )
			WriteToIniForRebootDelete(pProcPath, 0x01 ) ;
		else
		{
			//Quarantine the Process File
			QuarantineFile( pProcPath ) ;
		}

		BlockProcessFromExecution( pProcPath ) ;
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::RepairHiddenProcess", 0, 0, true, SECONDLEVEL);
	}

	return dwRet ;
}

/***************************************************************************
  Function Name  : TerminateSpecifiedProcess
  Description    : To terminate a specified Process.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_035
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::TerminateSpecifiedProcess( DWORD dwPID )
{

	HANDLE	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwPID ) ;
	if( !hProcess )
		return 0x01 ;

	BOOL	bRet = FALSE ;

	bRet = TerminateProcess( hProcess, 0x00 ) ;
	CloseHandle( hProcess ) ;
	hProcess = NULL ;
	if( bRet )
		return 0x00 ;

	return 0x02 ;
}

/***************************************************************************
  Function Name  : BlockProcessFromExecution
  Description    : To Block a specified Process from Execution.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_036
  Date           : 7/24/2014
****************************************************************************/
void CITinRKScnApp::BlockProcessFromExecution( TCHAR *pProcPath )
{
	HKEY	hSubKey = NULL ;

	if( RegCreateKeyEx(	HKEY_LOCAL_MACHINE, 
						L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options" , 
						0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL) != ERROR_SUCCESS )
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::BlockProcessFromExecution", 0, 0, true, SECONDLEVEL);
		return ;
	}

	HKEY	hBlockSubKey = NULL ;
	TCHAR	*pProcName = wcsrchr( pProcPath, '\\' ) ;
	if( !pProcName )
	{
		RegCloseKey(hSubKey ) ;
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::BlockProcessFromExecution", 0, 0, true, SECONDLEVEL);
		return ;
	}

	pProcName++ ;
	if( RegCreateKeyEx(	hSubKey, pProcName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
						NULL, &hBlockSubKey, NULL) != ERROR_SUCCESS )
	{
		RegCloseKey(hSubKey ) ;
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::BlockProcessFromExecution", 0, 0, true, SECONDLEVEL);
		return ;
	}

	TCHAR	*pDebugger = L"ITinM" ;

	RegSetValueEx(hBlockSubKey, L"Debugger", 0, REG_SZ, (LPBYTE)pDebugger, static_cast<DWORD>(wcslen(pDebugger)*sizeof(TCHAR)));

	RegCloseKey( hBlockSubKey ) ;
	RegCloseKey( hSubKey ) ;

	hBlockSubKey = hSubKey = NULL ;
	AddLogEntry(L"### Exception in CVibraniumRKScnApp::Process Blocked Successfully", 0, 0, true, SECONDLEVEL);
}

/***************************************************************************
  Function Name  : GetScanOptionsFromDWORD
  Description    : Function which get the Selected option for rootkit scan from UI in DOWRD
				   and makes the flags for scanning.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_037
  Date           : 7/24/2014
****************************************************************************/
void CITinRKScnApp::GetScanOptionsFromDWORD( DWORD dwAntirootScanOpt)
{
	DWORD	dwValue = 0x00 ;
	bool	bOpt = false ;

	m_bScanEntries = 0x00 ;
	for(DWORD i=0x00; i<0x03; i++ )
	{
		bOpt = false ;
		dwValue = dwAntirootScanOpt>>i ;
		if( (dwValue&0x01) == 0x01 )
			bOpt = true ;
		
		switch( i )
		{
		case 0:
			m_bProcess = bOpt ;
			if(m_bProcess == true)
			{
				AddLogEntry(L">>> Process boolean checked", 0, 0, true, FIRSTLEVEL);
				m_bScanEntries++ ;
			}
			break ;
		case 1:
			m_bRegistry = bOpt ;
			if(m_bRegistry == true)
			{
				AddLogEntry(L">>> Registry boolean checked", 0, 0, true, FIRSTLEVEL);
				m_bScanEntries++ ;
			}
			break ;
		case 2:
			m_bFileFolders = bOpt ;
			if(m_bFileFolders == true)
			{
				AddLogEntry(L">>> FileFolders boolean checked", 0, 0, true, FIRSTLEVEL);
				m_bScanEntries++ ;
			}
			break ;

		}
	}

	switch( m_bScanEntries )
	{
		case 0x01:
			m_EachScanMaxPercentage = 100 ;
			break ;

		case 0x02:
			m_EachScanMaxPercentage = 50 ;
			break ;

		case 0x03:
			m_EachScanMaxPercentage = 33 ;
			break ;
	}
}

/***************************************************************************
  Function Name  : GetScanPercentage
  Description    : Retrive the Scan Percentage. 
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_038
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::GetScanPercentage()
{
	SendRootKitInfoToGUI(SETPERCENTAGE,L"",L"",m_dwpercentage,0);
	Sleep(1);
	return 0;
}

/***************************************************************************
  Function Name  : GetDetectedEntriesofHiddenDrivers
  Description    : Get Detected Entries From the Hidden Drivers.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_039
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::GetDetectedEntriesofHiddenDrivers()
{
	//m_totalCountDriver = 56;
	//return m_dwRootKitCount ;

	if(!m_bRegistry)
		return 0;

	TCHAR	szTemp[256] = {0} ;

	wsprintf(szTemp, L"Hidden Entries:%lu", m_totalScannedDriverCount ) ;
	AddLogEntry( szTemp ) ;

	SendRootKitInfoToGUI(SHOWTOTALCOUNT,L"",L"",m_totalScannedDriverCount,0x02);
	Sleep( 100 );
	//m_dwRootKitCount = 33;//this line is for example

	//SendRootKitInfoToGUI(SHOWDETECTEDENTRIES,L"",L"",m_dwRootKitCount,0x02);
	//Sleep( 100 );
	
	return 0;
}

/***************************************************************************
  Function Name  : GetDetectedEntriesofHiddenProcesses
  Description    : Get Detected Entries From the Hidden Process.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_040
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::GetDetectedEntriesofHiddenProcesses()
{
	if(!m_bProcess)
		return 0;

	//m_totalCountProcess = 8;
	SendRootKitInfoToGUI(SHOWTOTALCOUNT,L"",L"",m_dwScanProcCount,0x01);

	Sleep( 100 );
	//m_dwHiddenProcCount = 5;//this line is for example
	//SendRootKitInfoToGUI(SHOWDETECTEDENTRIES,L"",L"",m_dwHiddenProcCount,0x01);
	//Sleep( 100 );

	return 0;
}

/***************************************************************************
  Function Name  : GetDetectedEntriesofHiddenFileorFolders
  Description    : Get Detected Entries From the Hidden Files/Folders.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_041
  Date           : 7/24/2014
****************************************************************************/
DWORD CITinRKScnApp::GetDetectedEntriesofHiddenFileorFolders()
{
	if(!m_bFileFolders)
		return 0;
	//Issue : In With clam setup, if we abort "Hidden Files and Folders" in antirootkit scan at 4%,it shows larger file scanned count.
	//Niranjan Deshak - 3/2/2015. Description - current count is used instead of total count. i.e. - m_dwFilesCountSecondPhase.
	m_dwTotalScannedFiles = m_objHiddenFiles.m_dwFilesCountSecondPhase;
	m_dwHiddenFilesCount = m_objHiddenFiles.m_dwFileNFolderRootkitFound;

	//m_totalCountProcess = 8;
	SendRootKitInfoToGUI(SHOWTOTALCOUNT,L"",L"",m_dwTotalScannedFiles,0x03);

	Sleep( 300 );
	//m_dwHiddenProcCount = 5;//this line is for example
	//SendRootKitInfoToGUI(SHOWDETECTEDENTRIES,L"",L"",m_dwHiddenFilesCount,0x03);

	//Sleep( 300 );

	return 1;
}

/***************************************************************************
  Function Name  : SetStatus
  Description    : Sets the RootKit Scan status in the GUI mode.  
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_042
  Date           : 7/24/2014
****************************************************************************/
bool CITinRKScnApp::SetStatus(CString csStatus)
{
	
	SendRootKitInfoToGUI(SETSCANSTATUS,csStatus,L"",m_dwHiddenProcCount,0x01);
	Sleep(10);
	return true;
}

/***************************************************************************
  Function Name  : ClearMemoryMapObjects
  Description    : Update Server with Memory Mapped Files.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_043
  Date           : 7/24/2014
****************************************************************************/
void CITinRKScnApp::ClearMemoryMapObjects()
{
	ITIN_MEMMAP_DATA iTinMemMap = {0};
	m_objRootKitServer.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
}

/***************************************************************************
  Function Name  : SendMessage2UI
  Description    : Send Respective Message to UI.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_044
  Date           : 7/24/2014
****************************************************************************/
bool CITinRKScnApp::SendMessage2UI(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(UI_SERVER, true);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CClamScanner::SendMessage2UI", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::StartScanUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
  Function Name  : WriteToIniForRebootDelete
  Description    : Update the ini file 
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_045
  Date           : 7/24/2014
****************************************************************************/
void CITinRKScnApp::WriteToIniForRebootDelete(TCHAR *pPath, BYTE bType )
{
	TCHAR	szIniPath[256] = {0} ;
	TCHAR	szCount[64] = {0} ;
	DWORD	dwCount = 0x00 ;

	GetModuleFileName(NULL, szIniPath, 255 ) ; 

	TCHAR	*pTemp = wcsrchr(szIniPath, '\\' ) ;
	if( pTemp )
	{
		pTemp++ ;
		*pTemp = '\0' ;

		wcscat(szIniPath, L"WardWizRDel.ini" ) ;
	}
	else
	{
		AddLogEntry(L"### Exception in CVibraniumRKScnApp::WriteToIniForRebootDelete", 0, 0, true, SECONDLEVEL);
		return ;
	}

	if( bType == 0x01 )
		GetPrivateProfileString(L"PCount", L"Count", L"", szCount, sizeof(szCount), szIniPath ) ;

	if( bType == 0x02 )
		GetPrivateProfileString(L"DCount", L"Count", L"", szCount, sizeof(szCount), szIniPath ) ;

	if( szCount[0] )
		swscanf(szCount, L"%lu", &dwCount ) ;

	dwCount++ ;
	ZeroMemory(szCount, sizeof( szCount) ) ;
	swprintf(szCount, L"%lu", dwCount ) ;

	if( bType == 0x01 )
	{
		WritePrivateProfileString(L"PCount", L"Count",szCount, szIniPath ) ;
		WritePrivateProfileString(L"PEntry", szCount, pPath, szIniPath ) ;
	}

	if( bType == 0x02 )
	{
		WritePrivateProfileString(L"DCount", L"Count", szCount, szIniPath ) ;
		WritePrivateProfileString(L"DEntry", szCount, pPath, szIniPath ) ;
	}


/***************************************************************************
  Function Name  : SetScanFunctionPtr
  Description    : Sets the Scan Function Pointer. 
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_046
  Date           : 7/24/2014
****************************************************************************/}
void CITinRKScnApp::SetScanFunctionPtr(CISpyScanner *pobjISpyScanner)
{
	if(pobjISpyScanner == NULL)
	{
		return;
	}
	m_pobjISpyScanner = pobjISpyScanner;
}

void CITinRKScnApp::QuarantineFile( TCHAR *pFilePath )
{
	//Please Write Quarantine Logic Here

}


/***************************************************************************
  Function Name  : UpdateLastScanDateTime
  Description    : Sets the last Scan Type and Scan Date & time as a Antirootkit Scan
  Author Name    : Vilas Suvarnakar
  S.R. No        : WRDWIZRKSCNDLL_047
  Date           : 28/10/2014
****************************************************************************/
void CITinRKScnApp::UpdateLastScanDateTime( )
{
	SYSTEMTIME  CurrTime = {0} ;

	TCHAR	szTemp[256] = {0}, szTime[16] = {0};

	try
	{

		GetSystemTime( &CurrTime ) ;
		_wstrtime_s( szTime, 15);

		swprintf_s(szTemp, _countof(szTemp), L"%d/%d/%d %s", CurrTime.wMonth, CurrTime.wDay, CurrTime.wYear, szTime);


		CITinRegWrapper	objReg;

		if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"LastScandt",
			szTemp) != 0)
		{
			AddLogEntry(L"### SetRegistryValueData failed in LastScandt UpdateLastScanDateTime", 0, 0, true, SECONDLEVEL);
			//AddLogEntry(L"### Error in CWRDWIZRKScnApp::ScanHiddenDriverss", 0, 0, true, SECONDLEVEL);
		}

		DWORD	dwScanType = ANTIROOTKITSCAN;

		if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"ScanType",
				dwScanType) != 0)
		{
			AddLogEntry(L"### SetRegistryValueData failed in ScanType UpdateLastScanDateTime", 0, 0, true, SECONDLEVEL);
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in VibraniumRootKitScan::UpdateLastScanDateTime", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : GetTotalCountOfAll
Description    : Get Total Count and Detected Entries From the Hidden Files/Folders and Hidden Processes and Hidden Drivers.
Author Name    : Niranjan Deshak
S.R. No        : WRDWIZRKSCNDLL_041
Date           : 05/03/2015
****************************************************************************/
DWORD CITinRKScnApp::GetTotalCountOfAll()
{
	try
	{
		//Issue : In With clam setup, if we abort "Hidden Files and Folders" in antirootkit scan at 4%,it shows larger file scanned count.
		//Niranjan Deshak - 3/2/2015. Description - current count is used instead of total count. i.e. - m_dwFilesCountSecondPhase.
		m_dwTotalScannedFiles = m_objHiddenFiles.m_dwFilesCountSecondPhase;
		m_dwHiddenFilesCount = m_objHiddenFiles.m_dwFileNFolderRootkitFound;
		//m_totalCountProcess = 8;
		SendRootKitInfoToGuiThroughPipe(SET_TOTALFILECOUNT, m_dwTotalScannedFiles, m_totalScannedDriverCount, m_dwScanProcCount, m_dwHiddenFilesCount, m_dwRootKitCount, m_dwHiddenProcCount);	
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumRootKitScan::UpdateLastScanDateTime", 0, 0, true, SECONDLEVEL);
	}
	return 1;

}

/***************************************************************************
Function Name  : SendRootKitInfoToGuiThroughPipe
Description    : To Send Total Count and Detected Entries From the Hidden Files/Folders and Hidden Processes and Hidden Drivers to GUI through Pipe Communication.
Author Name    : Niranjan Deshak
S.R. No        : WRDWIZRKSCNDLL_041
Date           : 05/03/2015
****************************************************************************/
bool CITinRKScnApp::SendRootKitInfoToGuiThroughPipe(int iMessageInfo, DWORD dwTotalCntFile, DWORD dwTotalCntDrives, DWORD dwTotalCntProc, DWORD dwDetectedCntFile, DWORD dwDetectedCntDrives, DWORD dwDetectedCntProc)
{
	try
	{
		
		g_objcriticalSection.Lock();

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = iMessageInfo;
		(*(DWORD *)&szPipeData.byData[0]) = dwTotalCntFile;
		(*(DWORD *)&szPipeData.byData[8]) = dwTotalCntDrives;
		(*(DWORD *)&szPipeData.byData[16]) = dwTotalCntProc;


		(*(DWORD *)&szPipeData.byData[24]) = dwDetectedCntFile;
		(*(DWORD *)&szPipeData.byData[32]) = dwDetectedCntDrives;
		(*(DWORD *)&szPipeData.byData[40]) = dwDetectedCntProc;

		//szPipeData.dwThirdValue = dwThirdParam;
		//OutputDebugString(L"--------------------------------------");
		//CString csMsg;
		////csMsg.Format(L"iMessageInfo : %d , dwTotalCntFile : %lu,  dwTotalCntDrives : %lu, dwTotalCntProc : %lu,, dwTotalCntProc : %lu, dwTotalCntProc : %lu, szPipeData.iMessageInfo, szPipeData.dwValue, szPipeData.dwSecondValue", (*(DWORD *)&szPipeData.byData[0]));
		//csMsg.Format(L"iMessageInfo : %d \n , dwTotalCntFile : %lu \n, dwTotalCntDrives : %lu\n, dwTotalCntProc : %lu\n, dwDetectedCntFile : %lu \n, dwDetectedCntDrives : %lu \n, dwDetectedCntProc : %lu \n", szPipeData.iMessageInfo, (*(DWORD *)&szPipeData.byData[0]), (*(DWORD *)&szPipeData.byData[8]), (*(DWORD *)&szPipeData.byData[16]), (*(DWORD *)&szPipeData.byData[24]), (*(DWORD *)&szPipeData.byData[32]), (*(DWORD *)&szPipeData.byData[40]));
		//OutputDebugString(csMsg);
		//OutputDebugString(L"--------------------------------------");

		//Neha Gharge Ticket 36 :Antirootkit gets hanged on the next antiroot kit scan
		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data::SendRootKitInfoToGuiThroughPipe", 0, 0, true, SECONDLEVEL);

				return false;
			}
		}

		Sleep(10);
		g_objcriticalSection.Unlock();
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumRootKitScan::SendRootKitInfoToGuiThroughPipe", 0, 0, true, SECONDLEVEL);
	}
	return true;

}
