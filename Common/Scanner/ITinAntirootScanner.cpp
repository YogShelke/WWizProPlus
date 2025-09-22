/**********************************************************************************************************                     
	  Program Name          : ITinAntirootScanner.cpp
	  Description           : Implementation of Antiroot Scanner functionality 
	  Author Name			: Neha Gharge                                                                                           
	  Date Of Creation      : 26 March 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  
***********************************************************************************************************/
#include "ITinAntirootScanner.h"

CITinAntirootKitScanner::CITinAntirootKitScanner(void)
{
	m_hAntirootScanDLL	= NULL;
	m_pSetFunctionPtr	= NULL;
}

CITinAntirootKitScanner::~CITinAntirootKitScanner(void)
{
	if(m_hAntirootScanDLL != NULL)
	{
		FreeLibrary(m_hAntirootScanDLL);
		m_hAntirootScanDLL = NULL;
	}
}

bool CITinAntirootKitScanner::LoadLibraryOfAntirootkit()
{
	DWORD	dwRet = 0x00 ;

	CString	csiSPYAVPath = GetWardWizPathFromRegistry() ;

	CString	csITinAntirootScannerDll = L"" ;

	csITinAntirootScannerDll.Format( L"%sVBRKSCN.DLL", csiSPYAVPath ) ;
	if( !PathFileExists( csITinAntirootScannerDll ) )
	{
		MessageBox(NULL, L"VBRKSCN.DLL module not found.\nPlease reinstall Vibranium to fix this problem.", L"Vibranium", MB_ICONEXCLAMATION);
		return false;
	}
	
	AddLogEntry(L">>> path exist %s",csITinAntirootScannerDll, 0, true, FIRSTLEVEL);
	
	if( !m_hAntirootScanDLL )
	{
		m_hAntirootScanDLL = LoadLibrary( csITinAntirootScannerDll ) ;
		if(!m_hAntirootScanDLL)
		{
			OutputDebugString(L"Failed to load library\n");
		}
	}

	m_StartAntirootscan	= (STARTANTIROOTSCAN ) GetProcAddress( m_hAntirootScanDLL, "StartAntirootScan" ) ;
	m_StopAntirootScan	= (STOPANTIROOTSCAN ) GetProcAddress( m_hAntirootScanDLL, "StopAntirootScan" ) ;
	m_GetPercentage	= (GETDWORDVALUES) GetProcAddress(m_hAntirootScanDLL , "GetScanPercentage");
	m_GetdetectedEntriesofDrivers = (GETDWORDVALUES) GetProcAddress(m_hAntirootScanDLL , "GetDetectedEntriesofHiddenDrivers");
	m_GetdetectedEntriesofHiddenProcesses = (GETDWORDVALUES) GetProcAddress(m_hAntirootScanDLL , "GetDetectedEntriesofHiddenProcesses");
	m_GetdtectedEntriesofHiddenFileorFolder = (GETDWORDVALUES) GetProcAddress(m_hAntirootScanDLL , "GetDetectedEntriesofHiddenFileorFolders");
	m_PauseAntirootKit = (PAUSERESUME) GetProcAddress(m_hAntirootScanDLL , "PauseAntirootScan");
	m_ResumeAntirootKit = (PAUSERESUME) GetProcAddress(m_hAntirootScanDLL , "ResumeAntirootScan");
	m_RepairProcess = (REPAIRPROCESS) GetProcAddress(m_hAntirootScanDLL, "RepairHiddenProcess");
	m_RepairDriver =(REPAIRDRIVER)GetProcAddress(m_hAntirootScanDLL,"RepairHiddenDriver");
	m_pSetFunctionPtr =(SETSCANFUNCTIONPTR)GetProcAddress(m_hAntirootScanDLL,"SetScanFunctionPtr");

	if( !m_StartAntirootscan )
	{
		UnloadLibrary();
		AddLogEntry(L"### m_StartAntirootscan not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if( !m_PauseAntirootKit )
	{
		UnloadLibrary();
		AddLogEntry(L"### m_PauseAntirootscan not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if( !m_ResumeAntirootKit)
	{
		UnloadLibrary();
		AddLogEntry(L"### m_ResumeAntirootscan not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		false;
	}

	if( !m_StopAntirootScan )
	{
		UnloadLibrary();
		AddLogEntry(L"### m_StopAntirootScan not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if( !m_RepairProcess)
	{
		UnloadLibrary();
		AddLogEntry(L"### m_RepairProcess not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if( !m_RepairDriver)
	{
		UnloadLibrary();
		AddLogEntry(L"### m_RepairDriver not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(!m_pSetFunctionPtr)
	{
		UnloadLibrary();
		AddLogEntry(L"### m_pSetFunctionPtr not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

bool CITinAntirootKitScanner::StartAntirootScan(SCANTYPE eScanType,DWORD dwAntirootScanOpt, CISpyScanner *pObjScanner)
{
	OutputDebugString(L">>> Before LoadLibraryOfAntirootkit\n");

	if(pObjScanner == NULL)
	{
		return false;
	}

	if(!LoadLibraryOfAntirootkit())
	{
		UnloadLibrary();
		OutputDebugString(L">>> Failed to load library\n");
		AddLogEntry(L"### Failed to load library", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if( !m_StartAntirootscan )
	{
		UnloadLibrary();
		AddLogEntry(L"###m_StartAntirootscan not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(m_pSetFunctionPtr != NULL)
	{
		m_pSetFunctionPtr(pObjScanner);
	}

	if(!m_StartAntirootscan(dwAntirootScanOpt))
	{
		AddLogEntry(L"###Error in CWardwizAntirootKitScanner::StartAntirootScan()", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


bool CITinAntirootKitScanner::PauseAntirootScan()
{
	if( !m_PauseAntirootKit )
	{
		UnloadLibrary();
		AddLogEntry(L"###m_PauseAntirootscan not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(!m_PauseAntirootKit())
	{
		AddLogEntry(L"###Error in CWardwizAntirootKitScanner::PauseAntirootScan()", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

bool CITinAntirootKitScanner::ResumeAntirootScan()
{
	if( !m_ResumeAntirootKit)
	{
		UnloadLibrary();
		AddLogEntry(L"###m_ResumeAntirootscan not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		false;
	}

	if(!m_ResumeAntirootKit())
	{
		AddLogEntry(L"###Error in CWardwizAntirootKitScanner::ResumeAntirootScan()", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

bool CITinAntirootKitScanner::StopAntirootScan()
{
	if( !m_StopAntirootScan )
	{
		UnloadLibrary();
		AddLogEntry(L"###m_StopAntirootScan not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if(!m_StopAntirootScan())
	{
		AddLogEntry(L"###Error in CWardwizAntirootKitScanner::StartAntirootScan()", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

//bool CITinAntirootKitScanner::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, DWORD dwISpyID)
//Signature Changed by Vilas on 07 April 2015
//Added more error codes means failing cases
DWORD CITinAntirootKitScanner::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, DWORD dwISpyID)
{

	DWORD	dwRet = 0x00;

	DWORD dwPID = 0;
	//bool bReturn = false;
	if(!LoadLibraryOfAntirootkit())
	{
		//return false;

		dwRet = 0x01;
		return dwRet;
	}

	switch(dwISpyID)
	{
	case 0x01:	
		dwPID = _wtol((LPCTSTR)szThreatName);			
		if(!m_RepairProcess)//repair process
		{
			AddLogEntry(L"### m_RepairProcess not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
			//return false;
			dwRet = 0x01;
		}
		if(!m_RepairProcess(dwPID,(TCHAR*)szThreatPath))
		{
			AddLogEntry(L"###Error in CWardwizAntirootKitScanner::m_RepairProcess()", 0, 0, true, SECONDLEVEL);
			
			//return false;
			dwRet = 0x01;
		}
		//bReturn = true;
		break;
	case 0x03: 
		if(!m_RepairDriver)//repair driver
		{
			AddLogEntry(L"### m_RepairProcess not found in VBRKSCN.DLL module", 0, 0, true, SECONDLEVEL);
			//return false;
			dwRet = 0x01;
		}
		if(!m_RepairDriver((TCHAR*)szThreatPath,(TCHAR*)szThreatName))
		{
			AddLogEntry(L"###Error in CWardwizAntirootKitScanner::m_RepairProcess()", 0, 0, true, SECONDLEVEL);
			//return false;
			dwRet = 0x01;
		}
		//bReturn = true;
		break;
	}

	UnloadLibrary();

	return dwRet;
}


bool CITinAntirootKitScanner::UnloadLibrary()
{
	if(m_hAntirootScanDLL != NULL)
	{
		FreeLibrary(m_hAntirootScanDLL);
		m_hAntirootScanDLL = NULL;
	}
	return true;
}