/**********************************************************************************************************
Program Name          : WardWiz Indexing DLL
Description           : This DLL application is to make file indexing DB.
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 05th Apr 2016
Version No            : 1.14.0.12
Special Logic Used    : This DLL has export function to calculate CRC from buffer and find in local Indexing 
						database, if present no need to scan file.
Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizIndexDLL.h"
#include "WrdwizEncDecManager.h"
#include "iTinRegWrapper.h"
#include "WWizCRC64.h"
#include <atltime.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT _declspec(dllimport)
#define DLLEXPORT _declspec(dllexport)

BEGIN_MESSAGE_MAP(CWWizIndexDLLApp, CWinApp)
END_MESSAGE_MAP()

/***************************************************************************************************
*  Function Name  : CWWizIndexDLLApp
*  Description    : Cont'r
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
CWWizIndexDLLApp::CWWizIndexDLLApp()
{
}

/***************************************************************************************************
*  Function Name  : CWWizIndexDLLApp
*  Description    : Dest'r
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	11 Feb 2016
****************************************************************************************************/
CWWizIndexDLLApp::~CWWizIndexDLLApp()
{
}

CWWizIndexDLLApp theApp;

/***************************************************************************************************
*  Function Name  : InitInstance
*  Description    : Function to Initialize variables, which gets called when application loads the DLL.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
BOOL CWWizIndexDLLApp::InitInstance()
{
	CWinApp::InitInstance();
	__try
	{
		if (!theApp.LoadLocalDatabase())
		{
			AddLogEntry(L"### Failed to LoadLocalDatabase in CVibraniumIndexDLLApp::InitInstance", 0, 0, true, SECONDLEVEL);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : ExitInstance()
*  Description    : Release resources used by this DLL
*  Author Name    : Ram Shelke
*  Date			  :	17-Mar-2015
****************************************************************************************************/
int CWWizIndexDLLApp::ExitInstance()
{
	__try
	{
		if (!theApp.SaveLocalDatabase())
		{
			AddLogEntry(L"### Failed to SaveLocalDatabase in CVibraniumIndexDLLApp::ExitInstance", 0, 0, true, SECONDLEVEL);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : ScanForMD5Data
*  Description    : Function to check for entry in file indexing if not present scan and add in database.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool ISWhiteListed(PBYTE pbData, size_t len, UINT64 &uiCRCValue)
{
	bool bFound = false;
	__try
	{
		return theApp.CheckWLStatus(pbData, len, uiCRCValue);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SearchInWhiteList MD5DLL", 0, 0, true, SECONDLEVEL);
	}
	return bFound;
}

/***************************************************************************************************
*  Function Name  : SaveLocalDatabase
*  Description    : Function to clear saved indexes
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date			  :	14 Oct 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool ClearIndexes()
{
	bool bReturn = false;
	__try
	{
		return theApp.ClearDatabase();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ClearIndexes", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ClearDatabase
*  Description    : Function to clear saved indexes
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date			  :	14 Oct 2016
****************************************************************************************************/
bool CWWizIndexDLLApp::ClearDatabase()
{
	bool bReturn = false;
	try
	{
		for (int iIndex = 0; iIndex < 50; iIndex++)
		{
			if (!m_pBalBst[iIndex])
			{
				break;
			}

			m_pBalBst[iIndex]->removeAll();
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumIndexDLLApp::ClearDatabase", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SaveLocalDatabase
*  Description    : Function to save local indexing in to file, so that will get loaded next time.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool SaveLocalDatabase()
{
	bool bReturn = false;
	__try
	{
		return theApp.SaveLocalDatabase();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SaveLocalDatabase", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SaveLocalDatabase
*  Description    : Function to save local indexing in to file, so that will get loaded next time.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool LoadLocalDatabase()
{
	bool bReturn = false;
	__try
	{
		return theApp.LoadLocalDatabase();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in LoadLocalDatabase", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : CheckWLStatus
*  Description    : Function to check the entry in white database
					@retrun true if found else false 
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	8 Mar 2016
****************************************************************************************************/
bool CWWizIndexDLLApp::CheckWLStatus(PBYTE pbData, size_t len, UINT64 &uiCRCValue)
{
	bool bReturn = false;
	try
	{
		CWWizCRC64 objCRC64;
		objCRC64.CalcCRC64(uiCRCValue, pbData, len);

		for (int iIndex = 0; iIndex < 50; iIndex++)
		{
			if (!m_pBalBst[iIndex])
			{
				break;
			}

			const UINT64 *dwPos = m_pBalBst[iIndex]->find(&uiCRCValue);
			if (dwPos != NULL)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumIndexDLLApp::CheckWLStatus", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : LoadLocalDatabase
*  Description    : Function to load local database which generally calls as WHITE database.
					@retrun true if loaded successfully else false
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	8 Mar 2016
****************************************************************************************************/
bool CWWizIndexDLLApp::LoadLocalDatabase()
{
	bool bReturn = true;
	try
	{

		int iIndex = 0;
		for (iIndex = 0; iIndex < 50; iIndex++)
		{
			if (m_pBalBst[iIndex] != NULL)
			{
				delete m_pBalBst[iIndex];
				m_pBalBst[iIndex] = NULL;
			}
		}

		DWORD dwCachingMethod = 0x01;
		CITinRegWrapper				g_objReg;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwCachingMethod", dwCachingMethod) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwCachingMethod in CWardwizIndexer::LoadLocalDatabase KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		iIndex = 0;
		TCHAR	szDrives[256] = { 0 };
		GetLogicalDriveStrings(255, szDrives);
		TCHAR	*pDrive = szDrives;
		while (wcslen(pDrive) > 2)
		{
			if ((GetDriveType(pDrive) & DRIVE_FIXED) == DRIVE_FIXED)
			{
				CString strWildcard(pDrive);
				
				if (strWildcard[strWildcard.GetLength() - 1] != '\\')
					strWildcard += _T("\\");
				strWildcard += L"VBIND.DAT";

				if (m_pBalBst[iIndex] == NULL)
				{
					//0x61A80 == 400000 Decimal
					m_pBalBst[iIndex] = new CWWizBalBst < UINT64 >(0x61A80);
					
					//If cache method is momentray then no need to load.
					if (dwCachingMethod != 0x00)
					{
						if (PathFileExists(strWildcard))
						{
							if (!m_pBalBst[iIndex]->Load(strWildcard.GetBuffer()))
							{
								bReturn = true;
							}
						}
					}

					if (islower(pDrive[0]))
						pDrive[0] = _toupper(pDrive[0]);
					m_mapBalbst.insert(make_pair(pDrive[0], m_pBalBst[iIndex]));
					iIndex++;
				}
			}
			pDrive += 0x04;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumIndexDLLApp::LoadLocalDatabase", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SaveLocalDatabase
*  Description    : Function to SAVE local database which generally calls as WHITE database.
					@retrun true if saved successfully else false
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	8 Mar 2016
****************************************************************************************************/
bool CWWizIndexDLLApp::SaveLocalDatabase()
{
	bool bReturn = true;
	try
	{
		for (BALBSTMAP::const_iterator it = m_mapBalbst.begin(); it != m_mapBalbst.end(); ++it) 
		{
			CString strWildcard(it->first);

			if (strWildcard[strWildcard.GetLength() - 1] != ':')
				strWildcard += _T(":");

			if (strWildcard[strWildcard.GetLength() - 1] != '\\')
				strWildcard += _T("\\");

			strWildcard += L"VBIND.DAT";

			CWWizBalBst<UINT64>	*pBalBst = it->second;

			if (pBalBst != NULL)
			{
				if (!pBalBst->Save(strWildcard.GetBuffer()))
				{
					bReturn = false;
					continue;
				}

				bReturn = true;
				SetFileAttributes(strWildcard, FILE_ATTRIBUTE_HIDDEN);
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumIndexDLLApp::LoadLocalDatabase", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ScanForMD5Data
*  Description    : Function to check for entry in file indexing if not present scan and add in database.
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	8 Feb 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool InsertIntoWhiteDB(TCHAR pDrive, UINT64 &uiCRCValue)
{
	bool bFound = false;
	__try
	{
		return theApp.InsertIntoWhiteDB(pDrive, uiCRCValue);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SearchInWhiteList MD5DLL", 0, 0, true, SECONDLEVEL);
	}
	return bFound;
}


/***************************************************************************************************
*  Function Name  : CheckWLStatus
*  Description    : Function to check the entry in white database
					@retrun true if found else false
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date			  :	8 Mar 2016
****************************************************************************************************/
bool CWWizIndexDLLApp::InsertIntoWhiteDB(TCHAR pDrive, UINT64 &uiCRCValue)
{
	bool bReturn = false;
	try
	{
		if (islower(pDrive))
			pDrive = _toupper(pDrive);

		BALBSTMAP::iterator iterMap;
		iterMap = m_mapBalbst.find(pDrive);
		if (iterMap == m_mapBalbst.end())
		{
			return bReturn;
		}

		CString strWildcard(iterMap->first);

		if (strWildcard[strWildcard.GetLength() - 1] != ':')
			strWildcard += _T(":");

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\");

		strWildcard += L"VBIND.DAT";

		CWWizBalBst<UINT64>	*pBalBst = iterMap->second;

		if (pBalBst != NULL)
		{
			//Check here if insert function returns 0x00 then its SUCCESS
			if (pBalBst->insert(&uiCRCValue) == 0x00)
			{
				bReturn = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumIndexDLLApp::CheckWLStatus", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}