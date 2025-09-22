/**********************************************************************************************************
Program Name          : WardWiz Email Scan Client
Description           : This file contains calling functions from WardwizEmailScn dll.
Author Name			  : Amol Jaware
Date Of Creation      : 01 Aug 2018
Special Logic Used    : Communication using named pipes & using Shared Memory.
Modification Log      :
***********************************************************************************************************/

#include "WardWizWebProtClient.h"
#include "WardWizDatabaseInterface.h"

/***********************************************************************************************
Function Name  : CWardWizWebProtClient
Description    : Const'r
Author Name    : Amol Jaware
Date           : 31 Jul 2018
***********************************************************************************************/
CWardWizWebProtClient::CWardWizWebProtClient() :
m_hModuleEmailDLL(NULL)
, m_lpWebProtInitialize(NULL)
, m_lpWebProtUnInitialize(NULL)
, m_lpWebProtReloadWebSecDB(NULL)
, m_lpWebProtReloadWebSettingDB(NULL)
, m_lpWebProtReloadSpecBlkWebDB(NULL)
, m_lpManageExclusionDB(NULL)
, m_lpClearURLCache(NULL)
{
}

/***********************************************************************************************
Function Name  : ~CWardWizWebProtClient
Description    : Dest'r
Author Name    : Ram Shelke
Date           : 27 Jun 2019
***********************************************************************************************/
CWardWizWebProtClient::~CWardWizWebProtClient()
{
	//Unload here
	if (m_lpWebProtUnInitialize != NULL)
	{
		m_lpWebProtUnInitialize();
	}

	if (m_hModuleEmailDLL != NULL)
	{
		FreeLibrary(m_hModuleEmailDLL);
		m_hModuleEmailDLL = NULL;
	}
}

/***********************************************************************************************
Function Name  : Initialize
Description    : Function which initialize required library and variables.
Author Name    : Ram Shelke
Date           : 27 Jun 2019
***********************************************************************************************/
bool CWardWizWebProtClient::Initialize()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpWebProtInitialize)
		{
			bReturn = m_lpWebProtInitialize();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::Initialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : UnInitialize
Description    : Function which uninitialize required library.
Author Name    : Ram Shelke
Date           : 27 Jun 2019
***********************************************************************************************/
bool CWardWizWebProtClient::UnInitialize()
{
	bool bReturn = false;
	__try
	{
		if (m_lpWebProtUnInitialize)
		{
			bReturn = m_lpWebProtUnInitialize();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::UnInitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadRequiredModules
Description    : Function Loads required modules
Author Name    : Ram Shelke
Date           : 27 Jun 2019
***********************************************************************************************/
bool CWardWizWebProtClient::LoadRequiredModules()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL)
		{
			return true;
		}

		if (!m_hModuleEmailDLL)
		{
			m_hModuleEmailDLL = LoadLibrary(L"VBEMAILSCN.DLL");
			if (!m_hModuleEmailDLL)
			{
				AddLogEntry(L"### Error in Loading VBEMAILSCN.DLL", 0, 0, true, SECONDLEVEL);
				return false;
			}

			m_lpWebProtInitialize = (WRDWIZWEBPROTINITIALIZE)GetProcAddress(m_hModuleEmailDLL, "VibraniumWebProtInitialize");
			if (!m_lpWebProtInitialize)
			{
				AddLogEntry(L"### CWardwizWebProtClient:LoadRequiredModules Error in GetProcAddress::WardwizEmailInitialize", 0, 0, true, SECONDLEVEL);
				m_lpWebProtInitialize = NULL;
			}

			m_lpWebProtUnInitialize = (WRDWIZWEBPROTUNINITIALIZE)GetProcAddress(m_hModuleEmailDLL, "VibraniumWebProtUnInitialize");
			if (!m_lpWebProtUnInitialize)
			{
				AddLogEntry(L"### CWardwizWebProtClient:LoadRequiredModules Error in GetProcAddress::WardwizEmailUnInitialize", 0, 0, true, SECONDLEVEL);
				m_lpWebProtUnInitialize = NULL;
			}

			m_lpWebProtReloadWebSecDB = (WRDWIZWEBPROTRELOADWEBSECDB)GetProcAddress(m_hModuleEmailDLL, "ReloadWebSecDB");
			if (!m_lpWebProtReloadWebSecDB)
			{
				AddLogEntry(L"### CWardwizWebProtClient:LoadRequiredModules Error in GetProcAddress::ReloadWebSecDB", 0, 0, true, SECONDLEVEL);
				m_lpWebProtReloadWebSecDB = NULL;
			}

			m_lpWebProtReloadWebSettingDB = (WRDWIZWEBPROTRELOADBROWSEPROTDB)GetProcAddress(m_hModuleEmailDLL, "ReloadBrowseProtDB");
			if (!m_lpWebProtReloadWebSettingDB)
			{
				AddLogEntry(L"### CWardwizWebProtClient:LoadRequiredModules Error in GetProcAddress::ReloadBrowseProtDB", 0, 0, true, SECONDLEVEL);
				m_lpWebProtReloadWebSettingDB = NULL;
			}

			m_lpWebProtReloadSpecBlkWebDB = (WRDWIZWEBPROTRELOADBLKSPECWEB)GetProcAddress(m_hModuleEmailDLL, "ReloadBlkSpecWebDB");
			if (!m_lpWebProtReloadSpecBlkWebDB)
			{
				AddLogEntry(L"### CWardwizWebProtClient:LoadRequiredModules Error in GetProcAddress::ReloadBlkSpecWebDB", 0, 0, true, SECONDLEVEL);
				m_lpWebProtReloadSpecBlkWebDB = NULL;
			}

			m_lpManageExclusionDB = (WRDWIZWEBPROTRELOADMNGEXCDB)GetProcAddress(m_hModuleEmailDLL, "ReloadMngExcDB");
			if (!m_lpManageExclusionDB)
			{
				AddLogEntry(L"### CWardwizWebProtClient:LoadRequiredModules Error in GetProcAddress::ReloadMngExcDB", 0, 0, true, SECONDLEVEL);
				m_lpManageExclusionDB = NULL;
			}

			m_lpClearURLCache = (WRDWIZWEBPROTCLEARCACHE)GetProcAddress(m_hModuleEmailDLL, "ClearURLCache");
			if (!m_lpClearURLCache)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::ClearURLCache", 0, 0, true, SECONDLEVEL);
				m_lpClearURLCache = NULL;
			}
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::LoadRequiredModules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***********************************************************************************************
Function Name  : ReloadWebSecDB
Description    : Function reloads web protection database
Author Name    : Ram Shelke
Date           : 27 Jun 2019
***********************************************************************************************/
bool CWardWizWebProtClient::ReloadWebSecDB()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();
		
		if (m_lpWebProtReloadWebSecDB != NULL)
		{
			bReturn = m_lpWebProtReloadWebSecDB();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::ReloadWebSecDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadBrowseProtDB
Description    : Function reloads web protection settings database
Author Name    : Ram Shelke
Date           : 27 Jun 2019
***********************************************************************************************/
bool CWardWizWebProtClient::ReloadBrowseProtDB()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpWebProtReloadWebSettingDB != NULL)
		{
			bReturn = m_lpWebProtReloadWebSettingDB();
		}
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::ReloadBrowseProtDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***********************************************************************************************
Function Name  : ReloadBlkSpecWebDB
Description    : Function reloads web protection settings database
Author Name    : Ram Shelke
Date           : 27 Jun 2019
***********************************************************************************************/
bool CWardWizWebProtClient::ReloadBlkSpecWebDB()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpWebProtReloadSpecBlkWebDB != NULL)
		{
			bReturn = m_lpWebProtReloadSpecBlkWebDB();
		}
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::ReloadBlkSpecWebDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadMngExcDB
Description    : Function reloads web protection settings database
Author Name    : Ram Shelke
Date           : 27 Jun 2019
***********************************************************************************************/
bool CWardWizWebProtClient::ReloadMngExcDB()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpManageExclusionDB != NULL)
		{
			bReturn = m_lpManageExclusionDB();
		}
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::ReloadMngExcDB", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function       : ClearCache
Description    : This function will clean prepared cache
Author Name    : Ram Shelke
Date           : 30 Jul 2019
****************************************************************************/
bool CWardWizWebProtClient::ClearURLCache()
{
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (!m_lpClearURLCache)
		{
			return false;
		}

		return m_lpClearURLCache();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ClearURLCache", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}