/**********************************************************************************************************                     
	  Program Name          : ISpyScanner.cpp
	  Description           : Functionality for Ispy Scanner
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 20 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna           Created Wrapper class for iSpy AV functionality implementation
***********************************************************************************************************/
#include "ISpyScanner.h"

#define MAX_INF_LEVEL	0x10

/***********************************************************************************************
Function Name  : CISpyScanner
Description    : Const'
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
CISpyScanner::CISpyScanner(void):
	  m_hModuleISpyScanDLL(NULL)
	, m_hModuleISpyRepairDLL(NULL)
	, m_lpLoadSigProc(NULL)
	, m_lpUnLoadSigProc(NULL)
	, m_lpScanFileProc(NULL)
	, m_lpRepairFileProc(NULL)
	, m_bSignatureLoaded(false)
	, m_bRescan(false)
{
}

/***********************************************************************************************
Function Name  : ~CISpyScanner
Description    : Const'
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
CISpyScanner::~CISpyScanner(void)
{
	//Unload signatures here
	if(m_lpUnLoadSigProc != NULL)
	{
		m_lpUnLoadSigProc();
	}

	if(m_hModuleISpyScanDLL != NULL)
	{
		FreeLibrary(m_hModuleISpyScanDLL);
		m_hModuleISpyScanDLL = NULL;
	}
	if(m_hModuleISpyRepairDLL != NULL)
	{
		FreeLibrary(m_hModuleISpyRepairDLL);
		m_hModuleISpyRepairDLL = NULL;
	}
}

/***********************************************************************************************
Function Name  : LoadRequiredModules
Description    : Function to load required DLL files
Author Name    : Ramkrushna Shelke
Date           : 29 Jun 2018
***********************************************************************************************/
bool CISpyScanner::LoadRequiredModules()
{
	if(m_hModuleISpyScanDLL && m_hModuleISpyRepairDLL)
	{
		return true;
	}

	if(!m_hModuleISpyScanDLL)
	{
		m_hModuleISpyScanDLL = LoadLibrary(L"VBSCANDLL.DLL");
		if(!m_hModuleISpyScanDLL)
		{
			AddLogEntry(L"### Error in Loading VBSCANDLL.DLL", 0, 0, true, SECONDLEVEL);
			return false;
		}

		m_lpLoadSigProc = (LOADSIGNATURE)GetProcAddress(m_hModuleISpyScanDLL, "LoadSignatures");
		if(!m_lpLoadSigProc)
		{
			AddLogEntry(L"### CWardwizSCANNER: Error in GetProcAddress::LoadSignatures", 0, 0, true, SECONDLEVEL);
			FreeLibrary(m_hModuleISpyScanDLL);
			m_lpLoadSigProc = NULL;
			m_hModuleISpyScanDLL = NULL;
			return false;
		}

		m_lpUnLoadSigProc = (UNLOADSIGNATURES)GetProcAddress(m_hModuleISpyScanDLL, "UnLoadSignatures");
		if(!m_lpUnLoadSigProc)
		{
			AddLogEntry(L"### CWardwizSCANNER: Error in GetProcAddress::UnLoadSignatures", 0, 0, true, SECONDLEVEL);
			FreeLibrary(m_hModuleISpyScanDLL);
			m_hModuleISpyScanDLL = NULL;
			m_lpUnLoadSigProc = NULL;
			return false;
		}

		m_lpScanFileProc = (SCANFILE)GetProcAddress(m_hModuleISpyScanDLL, "ScanFile");
		if(!m_lpScanFileProc)
		{
			AddLogEntry(L"### CWardwizSCANNER: Error in GetProcAddress::ScanFile", 0, 0, true, SECONDLEVEL);
			FreeLibrary(m_hModuleISpyScanDLL);
			m_hModuleISpyScanDLL = NULL;
			m_lpScanFileProc = NULL;
			return false;
		}

		m_lpReadSettingsProc = (RELOADSETTINGS)GetProcAddress(m_hModuleISpyScanDLL, "ReloadSettings");
		if (!m_lpReadSettingsProc)
		{
			AddLogEntry(L"### CWardwizSCANNER: Error in GetProcAddress::ReloadSettings", 0, 0, true, SECONDLEVEL);
			FreeLibrary(m_hModuleISpyScanDLL);
			m_hModuleISpyScanDLL = NULL;
			m_lpReadSettingsProc = NULL;
			return false;
		}
	}

	if(!m_hModuleISpyRepairDLL)
	{
		m_hModuleISpyRepairDLL = LoadLibrary(L"VBREPAIRDLL.DLL");
		if(!m_hModuleISpyRepairDLL)
		{
			AddLogEntry(L"### CWardwizSCANNER: Error in loading VBREPAIRDLL.DLL", 0, 0, true, SECONDLEVEL);
			return false;
		}

		m_lpRepairFileProc = (REPAIRFILE)GetProcAddress(m_hModuleISpyRepairDLL, "RepairFile");
		if(!m_lpRepairFileProc)
		{
			AddLogEntry(L"### CWardwizSCANNER: Error in GetProcAddress::RepairFile", 0, 0, true, SECONDLEVEL);
			FreeLibrary(m_hModuleISpyRepairDLL);
			return false;
		}
	}
	return true;
}

bool CISpyScanner::ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID,DWORD &dwFailedToLoadSignature, bool bRescan)
{
	__try
	{
		//if ScanDLL & Repair DLL not loaded then need to load here
		if(!m_hModuleISpyScanDLL || !m_hModuleISpyRepairDLL)
		{
			LoadRequiredModules();
		}

		if(!m_bSignatureLoaded)
		{
			TCHAR szModulePath[MAX_PATH] = {0};
			if(!GetModulePath(szModulePath, MAX_PATH))
			{
				return false;
			}
			wcscat(szModulePath, L"\\VBDB\\");
			wcscat(szModulePath, WRDWIZAVDBNAME);
		
			//Need to lock here with critical section as because if database loading is already 
			//in progress then need to wait here.
			m_objcsScanLoad.Lock();

			if (!m_bSignatureLoaded)
			{
				DWORD dwSigLoad = 0x00;
				if (m_lpLoadSigProc(szModulePath, dwSigLoad))
				{
					m_bSignatureLoaded = true;
				}
			}

			m_objcsScanLoad.Unlock();
		}
		
		/* If DB is not valid issue Neha Gharge 1/1/2014*/
		if(m_bSignatureLoaded)
		{
			if(m_lpScanFileProc)
			{
				if(m_lpScanFileProc(szFilePath, szVirusName, dwISpyID, bRescan))
				{
					_tcsupr(szVirusName);
					m_objcsScanFile.Unlock();
					return true;
				}								
			}
		
			//scan file here using SAVAPI
			if (m_objBDclient.BD_ScanFile(szFilePath, szVirusName, dwISpyID))
			{
				_tcsupr(szVirusName);
				return true;
			}
		}
		else
		{
			dwFailedToLoadSignature = 0x01;
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::ScanFile, FilePath: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	
	return false;
}

bool CISpyScanner::RepairFile(LPCTSTR szThreatPath, DWORD dwISpyID)
{
	bool bReturn = false;
	try
	{
		//create temp file to store decrypted data
		TCHAR szNewFilePath[MAX_PATH * 2];
		CString csFilePath = szThreatPath;
		csFilePath = csFilePath.Left(csFilePath.ReverseFind(L'\\'));

		UINT RetUnique = 0;
		RetUnique = GetTempFileName(csFilePath, L"WWTMP", 0, szNewFilePath);
		if (!RetUnique)
		{
			return false;
		}

		if (!DeleteFile(szNewFilePath))
			return false;

		//copy file path to diff location.
		if (!CopyFile(szThreatPath, szNewFilePath, FALSE))
		{
			return false;
		}

		//0xFFFFF means that file is scanned using SAVAPI
		if (dwISpyID == 0xFFFFF)
		{
			//scan file here using SAVAPI
			if (!m_objBDclient.BD_RepairFile(szNewFilePath, dwISpyID))
			{
				DeleteFile(szNewFilePath);
				return bReturn;
			}

			bReturn = true;

			if (!Replace2OriginalFile(szThreatPath, szNewFilePath))
			{
				DeleteFile(szNewFilePath);
			}
			return bReturn;
		}

		//recursive repair in case of multi-level infection
		DWORD dwLevel = 0x00;
		while (dwLevel < MAX_INF_LEVEL)
		{
			if (!m_lpRepairFileProc)
				return bReturn;

			if (!m_lpRepairFileProc(szNewFilePath, dwISpyID))
			{
				DeleteFile(szNewFilePath);
				return bReturn;
			}

			bReturn = true;

			DWORD dwISpyID = 0;
			TCHAR szVirusName[MAX_PATH] = { 0 };

			/* If DB is not valid issue Neha Gharge 1/1/2014*/
			if (m_bSignatureLoaded)
			{
				if (m_lpScanFileProc)
				{
					if (!m_lpScanFileProc(szNewFilePath, szVirusName, dwISpyID, false))
					{
						if (!Replace2OriginalFile(szThreatPath, szNewFilePath))
						{
							if (!DeleteFile(szNewFilePath))
							{
								return bReturn;
							}
						}
						return bReturn;
					}

					if (dwISpyID == 0x00)
					{
						if (!Replace2OriginalFile(szThreatPath, szNewFilePath))
						{
							if (!DeleteFile(szNewFilePath))
							{
								return bReturn;
							}
						}

						SetFileAttributes(szNewFilePath, FILE_ATTRIBUTE_NORMAL);
						if (!DeleteFile(szNewFilePath))
						{
							return bReturn;
						}
						return true;
					}
				}
			}
			dwLevel++;
		}

		if (!Replace2OriginalFile(szThreatPath, szNewFilePath))
		{
			if (!DeleteFile(szNewFilePath))
			{
				return bReturn;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::RepairFile FilePath: [%s]", szThreatPath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}
/**********************************************************************************************************
*  Function Name  :	LoadSignatureDatabase
*  Description    :	Function to load Signature database.
*  Author Name    : Ram Shelke
*  Date           : 27 April 2015
*  SR_NO		  :
**********************************************************************************************************/
DWORD CISpyScanner::LoadSignatureDatabase(DWORD &dwSigCount)
{
	DWORD dwRet = 0x00;
	__try
	{
		m_objBDclient.BD_initialize();

		//if ScanDLL & Repair DLL not loaded then need to load here
		if (!m_hModuleISpyScanDLL)
		{
			if (!LoadRequiredModules())
			{
				dwRet = 0x01;
				AddLogEntry(L"### Failed to Load LoadRequiredModules in CWardwizScanner::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				goto FAILED;
			}
		}

		if (!m_bSignatureLoaded)
		{
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (!GetModulePath(szModulePath, MAX_PATH))
			{
				dwRet = 0x02;
				AddLogEntry(L"### Failed to Load GetModulePath in CWardwizScanner::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				goto FAILED;
			}

			wcscat(szModulePath, L"\\VBDB\\");
			wcscat(szModulePath, WRDWIZAVDBNAME);

			//Need to lock here with critical section as because if database loading is already 
			//in progress then need to wait here.
			m_objcsScanLoad.Lock();

			if (!m_lpLoadSigProc(szModulePath, dwSigCount))
			{
				dwRet = 0x02;
				AddLogEntry(L"### Failed to Load signatures in CWardwizScanner::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				m_objcsScanLoad.Unlock();
				goto FAILED;
			}
			m_bSignatureLoaded = true;


			m_objcsScanLoad.Unlock();
		}

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x03;
		AddLogEntry(L"### Exception in CWardwizScanner::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
	
FAILED:
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  : UnLoadSignatureDatabase
*  Description    :	Function to Unload Signature database.
*  Author Name    : Ram Shelke
*  Date           : 27 April 2015
*  SR_NO		  :
**********************************************************************************************************/
DWORD CISpyScanner::UnLoadSignatureDatabase()
{
	DWORD dwRet = 0x00;
	__try
	{
		//if ScanDLL & Repair DLL not loaded then need to load here
		if (!m_hModuleISpyScanDLL)
		{
			if (!LoadRequiredModules())
			{
				dwRet = 0x01;
				AddLogEntry(L"### Failed to Load LoadRequiredModules in CWardwizScanner::UnLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				goto FAILED;
			}
		}

		if (m_bSignatureLoaded)
		{
			if (!m_lpUnLoadSigProc())
			{
				dwRet = 0x02;
				AddLogEntry(L"### Failed to UnLoad signatures in CWardwizScanner::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				goto FAILED;
			}
			m_bSignatureLoaded = false;
		}

		//m_objBDclient.BD_uninitialize();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x03;
		AddLogEntry(L"### Exception in CWardwizScanner::UnLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
FAILED:
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  : ReloadSettingsForScanDLL
*  Description    :	Function to Reload Communication Service.
*  Author Name    : Yogeshwar Rasal
*  Date           : 16 Sept. 2016
**********************************************************************************************************/
bool CISpyScanner::ReloadSettingsForScanDLL()
{
	bool bIsServReload = false;
	try
	{
		m_objBDclient.ReadHeuristicScanStatus();

		if (!m_lpReadSettingsProc)
		{
			AddLogEntry(L"### m_lpReadSettingsProc is NULL in CWardwizScanner::ReloadSettingsForScanDLL", 0, 0, true, SECONDLEVEL);
			return false;
		}
		bIsServReload = m_lpReadSettingsProc(HEURISTICSCAN, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::ReloadSettingsForScanDLL", 0, 0, true, SECONDLEVEL);
	}
	return (bIsServReload);
}

/**********************************************************************************************************
*  Function Name  : Replace2OriginalFile
*  Description    :	Function which replace again repaired file to original location.
*  Author Name    : Ram Shelke
*  Date           : 30 Aug 2019
**********************************************************************************************************/
bool CISpyScanner::Replace2OriginalFile(LPCTSTR szThreatPath, LPCTSTR szNewThreatPath)
{
	bool bReturn = false;
	try
	{
		//move original file to temp location
		TCHAR szTempFileName[MAX_PATH];
		TCHAR lpTempPathBuffer[MAX_PATH];
		UINT RetUnique = 0;
		DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
		if (!dwRetVal)
		{
			return bReturn;
		}

		RetUnique = GetTempFileName(lpTempPathBuffer, L"WWTMP", 0, szTempFileName);
		if (!RetUnique)
		{
			return bReturn;
		}

		if (!DeleteFile(szTempFileName))
		{
			return bReturn;
		}

		if (PathFileExists(szThreatPath))
		{
			if (!MoveFileEx(szThreatPath, szTempFileName, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
			{
				return bReturn;
			}

			if (!DeleteFile(szTempFileName))
			{
				MoveFileEx(szTempFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
		}

		if (PathFileExists(szNewThreatPath))
		{
			if (!MoveFileEx(szNewThreatPath, szThreatPath, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
			{
				return bReturn;
			}
		}
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::Replace2OriginalFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}