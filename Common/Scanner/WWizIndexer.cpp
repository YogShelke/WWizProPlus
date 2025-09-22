/**********************************************************************************************************
Program Name			: WWizIndexer.cpp
Description				: This class is written to load/Unload and get function pointers of WRDWIZINDEXER.DLL
Author Name				: Ramkrushna Shelke
Date Of Creation		: 04/06/2016
Version No				: 1.14.1.10
Special Logic Used		:
Modification Log		:
1. Name					: Description
***********************************************************************************************************/
#include "stdafx.h"
#include "WWizIndexer.h"
#include "WardWizIniWrapper.h"
#include "iTinRegWrapper.h"

/***************************************************************************************************
*  Function Name  : CWWizIndexer
*  Description    : Cont'r
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
CWWizIndexer::CWWizIndexer():
	  m_hModule(NULL)
	, m_pIsWhiteListed(NULL)
{
	LoadIndexLibrary();
}

/***************************************************************************************************
*  Function Name  : ~CWWizIndexer
*  Description    : Dest'r
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
CWWizIndexer::~CWWizIndexer()
{
	if (m_hModule != NULL)
	{
		FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

/***************************************************************************************************
*  Function Name  : LoadIndexLibrary
*  Description    : Function to load VBINDEX.DLL and get function pointers from DLL
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
void CWWizIndexer::LoadIndexLibrary()
{
	__try
	{
		if (!m_hModule)
		{
			m_hModule = LoadLibrary(L"VBINDEX.DLL");
		}

		if (!m_hModule)
		{
			AddLogEntry(L"### File not found %s", L"VBINDEX.DLL", 0, true, SECONDLEVEL);
			return;
		}

		m_pIsWhiteListed = (ISWHITELISTED)GetProcAddress(m_hModule, "ISWhiteListed");

		if (!m_pIsWhiteListed)
		{
			AddLogEntry(L"### Function ISWhiteListed not found in file: %s", L"VBINDEX.DLL", 0, true, SECONDLEVEL);
			return;
		}

		m_pInserIntoWhiteDB = (INSERTINTOWHITEDB)GetProcAddress(m_hModule, "InsertIntoWhiteDB");
		if (!m_pInserIntoWhiteDB)
		{
			AddLogEntry(L"### Function InsertIntoWhiteDB not found in file: %s", L"VBINDEX.DLL", 0, true, SECONDLEVEL);
			return;
		}

		m_pLoadLocalDB = (LOADLOCALDATABASE)GetProcAddress(m_hModule, "LoadLocalDatabase");
		if (!m_pLoadLocalDB)
		{
			AddLogEntry(L"### Function LoadLocalDatabase not found in file: %s", L"VBINDEX.DLL", 0, true, SECONDLEVEL);
			return;
		}

		m_pSaveLocalDB = (SAVELOCALDATABASE)GetProcAddress(m_hModule, "SaveLocalDatabase");
		if (!m_pSaveLocalDB)
		{
			AddLogEntry(L"### Function SaveLocalDatabase not found in file: %s", L"VBINDEX.DLL", 0, true, SECONDLEVEL);
			return;
		}

		m_pClearIndexes = (SAVELOCALDATABASE)GetProcAddress(m_hModule, "ClearIndexes");
		if (!m_pClearIndexes)
		{
			AddLogEntry(L"### Function ClearIndexes not found in file: %s", L"VBINDEX.DLL", 0, true, SECONDLEVEL);
			return;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizIndexer::LoadIndexLibrary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : InsertIntoWhiteDB
*  Description    : Function to Insert Index in to white DB
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
bool CWWizIndexer::InsertIntoWhiteDB(TCHAR pDrive, UINT64 &uiCRCValue)
{
	bool bReturn = false;
	__try
	{
		if (!m_pInserIntoWhiteDB)
		{
			return bReturn;
		}

		bReturn = m_pInserIntoWhiteDB(pDrive, uiCRCValue);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizIndexer::InsertIntoWhiteDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ISWhiteListed
*  Description    : Function to check is Index white listed or not
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
bool CWWizIndexer::ISWhiteListed(PBYTE pbData, size_t len, UINT64 &uiCRCValue)
{
	bool bReturn = false;
	__try
	{
		if (!m_pIsWhiteListed)
		{
			return bReturn;
		}

		bReturn = m_pIsWhiteListed(pbData, len, uiCRCValue);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizIndexer::ISWhiteListed", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadLocalDatabase
*  Description    : Function to load local white database into binary tree
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
bool CWWizIndexer::LoadLocalDatabase()
{
	bool bReturn = false;
	__try
	{
		bReturn = LoadLocalDatabaseSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizIndexer::LoadLocalDatabase", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadLocalDatabaseSEH
*  Description    : Function to load local white database into binary tree
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
bool CWWizIndexer::LoadLocalDatabaseSEH()
{
	bool bReturn = false;
	try
	{
		if (!m_pLoadLocalDB)
		{
			return bReturn;
		}

		DWORD dwCachingMethod = 0x01;
		CITinRegWrapper				g_objReg;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwCachingMethod", dwCachingMethod) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwCachingMethod in CWardwizIndexer::LoadLocalDatabaseSEH KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		//If cache method is momentray then no need to save.
		if (dwCachingMethod == 0x00)
		{
			return true;
		}

		bReturn = m_pLoadLocalDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizIndexer::LoadLocalDatabaseSEH", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SaveLocalDatabase
*  Description    : Function to save binary tree object into file.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
bool CWWizIndexer::SaveLocalDatabase()
{
	bool bReturn = false;
	try
	{
		if (!m_pSaveLocalDB)
		{
			return bReturn;
		}

		DWORD dwCachingMethod = 0x01;
		CITinRegWrapper				g_objReg;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwCachingMethod", dwCachingMethod) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwCachingMethod in CWardwizIndexer::SaveLocalDatabase KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		//If cache method is momentray then no need to save.
		if (dwCachingMethod == 0x00)
		{
			return true;
		}
	
		bReturn = m_pSaveLocalDB();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizIndexer::SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ClearIndedexes
*  Description    : Function to clear indexes
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date			  : 12 APR 2016
****************************************************************************************************/
bool CWWizIndexer::ClearIndedexes()
{
	bool bReturn = false;
	try
	{
		if (!m_pClearIndexes)
		{
			return bReturn;
		}

		bReturn = m_pClearIndexes();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizIndexer::ClearIndedexes", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
