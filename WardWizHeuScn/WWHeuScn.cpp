// WWHeuScn.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "WWHeuScn.h"

#include "WWHeuristic.h"
#include <Shlwapi.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <Sfc.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLEXPORT _declspec(dllexport)

// CWWHeuScnApp

BEGIN_MESSAGE_MAP(CWWHeuScnApp, CWinApp)
END_MESSAGE_MAP()


// CWWHeuScnApp construction

CWWHeuScnApp::CWWHeuScnApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWWHeuScnApp object

CWWHeuScnApp theApp;


// CWWHeuScnApp initialization

BOOL CWWHeuScnApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

DWORD RepairVirus( LPCTSTR lpFilePath );
DWORD CheckInRunningProcess( LPCTSTR lpFilePath );
DWORD SuspendAllthreads( const DWORD dwPID );


/***************************************************************************************************
*  Function Name  : ScanFileForHeuristic()
*  Description    : Exported function used to sends file for heuristic scan
*  Author Name    : Vilas
*  Date			  :	16-Aug-2014
*  Date Modified  :	10-Mar-2015
*  SR No		  : WRDWIZHEURISTICSCANNER_0001
****************************************************************************************************/
//extern "C" DLLEXPORT DWORD ScanFileForHeuristic(LPCTSTR lpFilePath, LPTSTR lpVirusName, DWORD &dwVirusID, bool bRepair )

//extern "C" DLLEXPORT bool ScanFileForHeuristic(LPCTSTR lpFilePath, LPTSTR lpVirusName, DWORD &dwVirusID )
//Modified by Vilas on 07-Aug-2015 to support return value for PE file type and 64 bit file by reference
extern "C" DLLEXPORT bool ScanFileForHeuristic(LPCTSTR lpFilePath, LPTSTR lpVirusName, DWORD &dwVirusID, bool &bISPEFile, bool &bISX64PEFile)
{
	DWORD	dwRet = 0x00;
	bool	bVirus = false;

	try
	{
		if( !PathFileExists(lpFilePath) )
		{
			dwRet = 0x100;
			goto Cleanup;
		}

		//Code Modified : Vilas
		//Added Date	: 10 - 03 - 2015
		//Avoiding Heuristic detection for Windows SFC files
		//This will reduce FP for Windows file
		//This function fails when Windows SFC files are found at other location
		//means if calc.exe found at other than predefined location.
		
		//Commented out because PE file information is passing to Scan DLL
		//Added on 07 / 08 / 2015 by Vilas
	/*	if (SfcIsFileProtected(NULL, lpFilePath))
		{
			dwRet = 0x104;
			goto Cleanup;
		}
	*/
		if( IsBadWritePtr(lpVirusName, 128) )
		{
			dwRet = 0x101;
			goto Cleanup;
		}

		if( IsBadWritePtr(&dwVirusID, sizeof(DWORD)) )
		{
			dwRet = 0x102;
			goto Cleanup;
		}

		//dwVirusID = 0x00;

		WWHeuristic	HeuristicObj;

		if( HeuristicObj.GetPEFileInfo( lpFilePath ) )
		{
			dwRet = 0x103;
			goto Cleanup;
		}

		//
		bISPEFile = HeuristicObj.m_ISPEFile;
		bISX64PEFile = HeuristicObj.m_IS64Bit;

		//dwVirusID = HeuristicObj.GetVirusID();
		if( HeuristicObj.GetVirusID() )
		{
			wcscpy_s(lpVirusName, 127, HeuristicObj.GetVirusName() );

			bVirus = true;

			//Virus ID in the range of 1001 to 1500 is reserved for Heurisitc Detection and Repair
			//Added by Vilas on 17 March 2015
			if ( (HeuristicObj.GetVirusID() > 0x3E8) &&
				 (HeuristicObj.GetVirusID() < 0x5DD) )
				dwVirusID = HeuristicObj.GetVirusID();
		}

	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in ScanFileForHeuristic, FilePath: %s", lpFilePath, 0, true, SECONDLEVEL);
	}

Cleanup:

	//if( bRepair && dwVirusID )
	//	dwRet = RepairVirus( lpFilePath );

	return bVirus;
}

DWORD RepairVirus( LPCTSTR lpFilePath )
{
	DWORD dwRet = 0x00;

	try
	{
		if( !PathFileExists(lpFilePath) )
		{
			dwRet = 0x200;
			goto Cleanup;
		}

		CheckInRunningProcess( lpFilePath );

		if( DeleteFile( lpFilePath ) )
			goto Cleanup;

		if( _wremove( lpFilePath ) == 0 )
			goto Cleanup;

		MoveFileEx(lpFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in RepairVirus", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet;
}

DWORD CheckInRunningProcess( LPCTSTR lpFilePath )
{
	DWORD	dwRet = 0x00;
	DWORD	dwPID[0x100] = {0};
	DWORD	dwProcesses = 0x00, i = 0x00;

	TCHAR	szProcPath[1024] = {0} ;

	HANDLE hProcess = NULL;

	try
	{

		EnumProcesses(dwPID, 0x400, &dwProcesses ) ;

		if( dwProcesses > 0x400 )
		{
			dwRet = 0x500;
			goto Cleanup;
		}

		dwProcesses = dwProcesses/sizeof(DWORD) ;

		for(; i<dwProcesses; i++ )
		{
			if( hProcess )
				CloseHandle( hProcess );

			hProcess = NULL;

			hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SUSPEND_RESUME | PROCESS_TERMINATE, 
									FALSE, dwPID[i] ) ;
			if( !hProcess )
				continue;


			HMODULE		hMods[1024] = {0};
			DWORD		dwModules = 0x00;

			if( !EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwModules ) )
				continue;

			ZeroMemory( szProcPath, sizeof(szProcPath) );

			GetModuleFileNameEx(hProcess, hMods[0], szProcPath, 1023 ) ;
			if( _memicmp( szProcPath, lpFilePath, wcslen(lpFilePath)*sizeof(TCHAR) ) != 0 )
				continue;

			SuspendAllthreads( dwPID[i] );

			TerminateProcess( hProcess, 0x00 );

			break;
		}

	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CheckInRunningProcess", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( hProcess != NULL )
		CloseHandle( hProcess );

	hProcess = NULL;

	return dwRet;
}

DWORD SuspendAllthreads( const DWORD dwPID )
{
	DWORD dwRet = 0x00;

	HANDLE			hThreadSnap = INVALID_HANDLE_VALUE; 
	THREADENTRY32	te32 = {0};

	try
	{
		hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
		if( hThreadSnap == INVALID_HANDLE_VALUE ) 
		{
			dwRet = 0x400;
			goto Cleanup;
		}

		te32.dwSize = sizeof(THREADENTRY32 );

		if( !Thread32First( hThreadSnap, &te32 ) )
		{
			dwRet = 0x401;
			goto Cleanup;
		}

		do
		{
			if( te32.th32OwnerProcessID == dwPID )
			{
				HANDLE hThread = OpenThread( THREAD_SUSPEND_RESUME | THREAD_TERMINATE, FALSE, te32.th32ThreadID );

				if( hThread )
				{
					SuspendThread( hThread );
					CloseHandle( hThread );
					hThread = NULL;
				}
			}
		} while( Thread32Next(hThreadSnap, &te32 ) ); 

	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in SuspendAllthreads", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if( hThreadSnap != INVALID_HANDLE_VALUE )
		CloseHandle( hThreadSnap );

	hThreadSnap = INVALID_HANDLE_VALUE;

	return dwRet;
}