/**********************************************************************************************************            			  Program Name          : WardWizDumpCreater.cpp
	  Description           : Wrapper class to create memory dump.
	  Author Name			: Ramkrushna Shelke                                                                        			  Date Of Creation      : 22 Sep 2014
	  Version No            : 1.0.0.8
	  Special Logic Used    : 
	  Modification Log      :    
***********************************************************************************************************/
#pragma once
#include "stdafx.h"
#include "WardWizDumpCreater.h"
#include <dbghelp.h>
#include <crtdbg.h>
#include <time.h>

#pragma comment ( lib, "dbghelp.lib" )

BOOL CALLBACK MiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT   pInput, PMINIDUMP_CALLBACK_OUTPUT        Output 
); 

VECTORFINDDATA CWardWizDumpCreater::m_vectorFileData(10);

/***********************************************************************************************
  Function Name  : CWardWizDumpCreater
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 22 Sep 2014
***********************************************************************************************/
CWardWizDumpCreater::CWardWizDumpCreater(void)
{
}

/***********************************************************************************************
  Function Name  : ~CWardWizDumpCreater
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 22 Sep 2014
***********************************************************************************************/
CWardWizDumpCreater::~CWardWizDumpCreater(void)
{
}

/***********************************************************************************************
  Function Name  : GetModulePath
  Description    : Function to get Application module path 
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 22 Sep 2014
***********************************************************************************************/
bool CWardWizDumpCreater::GetModulePath(TCHAR *szModulePath, DWORD dwSize)
{
	return GetWardWizPathFromRegistry(szModulePath, dwSize);
}

/***********************************************************************************************
  Function Name  : GetWardWizPathFromRegistry
  Description    : Function to get Application path from WardWiz Registry key.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 22 Sep 2014
***********************************************************************************************/
bool CWardWizDumpCreater::GetWardWizPathFromRegistry(TCHAR *pszModulePath, DWORD dwSize)
{
	HKEY	hSubKey = NULL ;
	TCHAR	szModulePath[MAX_PATH] = {0};

	TCHAR szRegKeyPath[MAX_PATH] = { 0 };
	_tcscpy_s(szRegKeyPath, L"SOFTWARE\\Vibranium");
	//if (!GetProductRegistryKey(szRegKeyPath, sizeof(szRegKeyPath)))
	//{
	//	return 0;
	//}

	if (RegOpenKey(HKEY_LOCAL_MACHINE, szRegKeyPath, &hSubKey) != ERROR_SUCCESS)
		return false;

	DWORD	dwPathSize = 511 ;
	DWORD	dwType = 0x00 ;

	RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwPathSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if(_tcslen(szModulePath) > 0)
	{
		_tcscpy_s(pszModulePath, dwSize, szModulePath);
	}
	return true;
}

/***********************************************************************************************
  Function Name  : CreateMiniDump
  Description    : Function to create Memory dump.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 22 Sep 2014
***********************************************************************************************/
void CWardWizDumpCreater::CreateMiniDump( EXCEPTION_POINTERS* pep )
{
// issue- in case of debug mode miniDump not getting create.
// now dump will get create in debug mode
// lalit kumawat 7-7-2015
//#ifndef NDEBUG
//	return;
//#endif
	AddLogEntry(L"### !!!.. Application has been crashed...", 0, 0, true, SECONDLEVEL);
	__try
	{
		DeleteOldDumpFiles();
		
		TCHAR szModulePath[MAX_PATH] = {0};
		GetModulePath(szModulePath, MAX_PATH);

		if(_tcslen(szModulePath) == 0)
		{
			return;
		}

		TCHAR szPath[MAX_PATH] = {0};
		_tcscpy_s(szPath, szModulePath);
		
		TCHAR szFileName[MAX_PATH] = {0};

		time_t now = time(0);
		struct tm timeinfo;
		localtime_s(&timeinfo, &now);

		_tcsftime(szFileName, sizeof(szFileName), L"log\\VIBRANIUM_%Y%m%d_%H%M%S.dmp", &timeinfo);

		TCHAR szFullPath[MAX_PATH] = {0};
		_stprintf_s(szFullPath, L"%s%s", szModulePath, szFileName);

		// Open the file 
		HANDLE hFile = CreateFile( szFullPath, GENERIC_READ | GENERIC_WRITE, 
			0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

		if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
		{
			// Create the mini-dump 

			MINIDUMP_EXCEPTION_INFORMATION mdei; 

			mdei.ThreadId           = GetCurrentThreadId(); 
			mdei.ExceptionPointers  = pep; 
			mdei.ClientPointers     = FALSE; 

			MINIDUMP_CALLBACK_INFORMATION mci; 

			mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback; 
			mci.CallbackParam       = 0; 
			MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory); 

			BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
				hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci ); 

			if( !rv ) 
			{
				TCHAR szLastError[MAX_PATH] = {0};
				_stprintf_s(szLastError, L"### MiniDumpWriteDump failed. Error: %u ", GetLastError());
				AddLogEntry(szLastError);
			}
			// Close the file 
			CloseHandle( hFile ); 
		}
		else 
		{
			AddLogEntry(L"### Failed to create dump file", 0, 0, true, SECONDLEVEL);
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizDumpCreater::CreateMiniDump", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
  Function Name  : MiniDumpCallback
  Description    : Call back function to create minidump file.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 22 Sep 2014
***********************************************************************************************/
BOOL CALLBACK MiniDumpCallback(
	PVOID                            pParam, 
	const PMINIDUMP_CALLBACK_INPUT   pInput, 
	PMINIDUMP_CALLBACK_OUTPUT        pOutput ) 
{

	BOOL bRet = FALSE; 

	// Check parameters 
	if( pInput == 0 ) 
		return FALSE; 

	if( pOutput == 0 ) 
		return FALSE; 

	// Process the callbacks 
	switch( pInput->CallbackType ) 
	{
		case IncludeModuleCallback: 
		{
			// Include the module into the dump 
			bRet = TRUE; 
		}
		break; 
		case IncludeThreadCallback: 
		{
			// Include the thread into the dump 
			bRet = TRUE; 
		}
		break; 
		case ModuleCallback: 
		{
			// Does the module have ModuleReferencedByMemory flag set ? 

			if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) ) 
			{
				// No, it does not - exclude it 

				wprintf( L"Excluding module: %s \n", pInput->Module.FullPath ); 

				pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
			}
			bRet = TRUE; 
		}
		break; 
		case ThreadCallback: 
		{
			// Include all thread information into the minidump 
			bRet = TRUE;  
		}
		break; 
		case ThreadExCallback: 
		{
			// Include this information 
			bRet = TRUE;  
		}
		break; 
		case MemoryCallback: 
		{
			// We do not include any information here -> return FALSE 
			bRet = FALSE; 
		}
		break; 
		case CancelCallback: 
			break; 
	}
	return bRet; 
}

/***************************************************************************************************          
*  Function Name  : DeleteOldDumpFiles
*  Description    : Function which checks with old dump files.
*  Author Name    : Ramkrushna Shelke
*  SR.NO		  : 
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
bool CWardWizDumpCreater::DeleteOldDumpFiles()
{
	bool bReturn = false;
	__try
	{
		TCHAR szModulePath[MAX_PATH] = {0};
		GetModulePath(szModulePath, MAX_PATH);

		if(_tcslen(szModulePath) == 0)
		{
			return bReturn;
		}

		TCHAR szEnumPath[MAX_PATH] = {0};
		_tcscpy_s(szEnumPath, szModulePath);
		_tcscat_s(szEnumPath, L"\\Log");

		m_vectorFileData.clear();

		EnumFolder(szEnumPath);

		DeleteOlderDumps();
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizDumpCreater::DeleteOldDumpFiles", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/***************************************************************************************************         
*  Function Name  : EnumFolder
*  Description    : Recursive function to enumerate folder.
*  Author Name    : Ram
*  SR.NO		  : 
*  Date  		  :	10-Oct-2014
****************************************************************************************************/
DWORD CWardWizDumpCreater::EnumFolder( TCHAR *pFolderPath )
{
	DWORD				dwRet = 0x00 ;
	__try
	{
		WIN32_FIND_DATA		FindFileData = {0};
		HANDLE				hFind = INVALID_HANDLE_VALUE ;
		DWORD				dwFileCount = 0;

		TCHAR	szTemp[1024] = {0} ;

		swprintf_s(szTemp, L"%s\\*.*", pFolderPath ) ;

		hFind = FindFirstFile(szTemp, &FindFileData) ;
		if( hFind == INVALID_HANDLE_VALUE )
		{
			return 1 ;
		}

		TCHAR	szFile[1024] = {0} ;

		while( FindNextFile(hFind, &FindFileData) )
		{
			if( ( _wcsicmp(FindFileData.cFileName, L".") == 0 ) ||
				( _wcsicmp(FindFileData.cFileName, L"..") == 0 ) )
				continue ;

			TCHAR szFileName[MAX_PATH] = {0};
			_tcscpy_s(szFileName, FindFileData.cFileName);
			
			DWORD dwLength = (DWORD)_tcslen(szFileName);
			if(	_memicmp(&szFileName[dwLength - 4], L".dmp", 4) == 0)
			{
				m_vectorFileData.push_back(FindFileData);
			}
		}

		FindClose( hFind ) ;
		hFind = INVALID_HANDLE_VALUE ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizDumpCreater::EnumFolder", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return dwRet ;
}

/***************************************************************************************************          
*  Function Name  : DeleteOlderDumps
*  Description    : Function which checks with old dump files and keeps only 2 dump files.
*  Author Name    : Ram Shelke
*  SR.NO		  : 
*  Date  		  :	10th Oct 2014
****************************************************************************************************/
bool CWardWizDumpCreater::DeleteOlderDumps()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = {0};
		GetModulePath(szModulePath, MAX_PATH);

		if(_tcslen(szModulePath) == 0)
		{
			return false;
		}

		TCHAR szEnumPath[MAX_PATH] = {0};
		_tcscpy_s(szEnumPath, szModulePath);
		_tcscat_s(szEnumPath, L"\\Log");

		if(m_vectorFileData.size() == 0)
		{
			return false;
		}

		while(m_vectorFileData.size() > 1)
		{
			DWORD dwOldFileIndex = 0;
			DWORD dwVecMaxElment = (DWORD)(m_vectorFileData.size() - 1);

			for(DWORD iIndex = 0; iIndex <= dwVecMaxElment; iIndex++)
			{
				for(DWORD jIndex = 0; jIndex <= dwVecMaxElment; jIndex++)
				{
					if(iIndex != jIndex)
					{
						WIN32_FIND_DATA  stFirstFileData = m_vectorFileData[iIndex];
						WIN32_FIND_DATA  stSecondFileData = m_vectorFileData[jIndex];
						if(CompareFileTime(&stFirstFileData.ftCreationTime, &stSecondFileData.ftCreationTime) == 1)
						{
							dwOldFileIndex = jIndex;
						}
					}
				}
			}

			WIN32_FIND_DATA  stFirstFileData =  m_vectorFileData[dwOldFileIndex];

			TCHAR szFullPath[MAX_PATH] = {0};
			swprintf_s(szFullPath, L"%s\\%s", szEnumPath, stFirstFileData.cFileName);

			SetFileAttributes(szFullPath, FILE_ATTRIBUTE_NORMAL);
			if(!DeleteFile(szFullPath))
			{
				AddLogEntry(L"### Delete File Failed in DeleteLargeFiles", 0, 0, true, SECONDLEVEL);
			}
			m_vectorFileData.erase( m_vectorFileData.begin() + dwOldFileIndex );
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizDumpCreater::DeleteOlderDumps", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

bool CWardWizDumpCreater::GetProductRegistryKey(TCHAR *szRegKeyValue, DWORD dwSize)
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return false;
		}

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"VibraniumRegPath", L"SOFTWARE\\Vibranium", szValue, sizeof(szValue), szFullFilePath);

		_tcscpy_s(szRegKeyValue, dwSize, szValue);
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetProductRegistryKey", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}