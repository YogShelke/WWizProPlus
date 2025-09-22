/**********************************************************************************************************
Program Name          : WardWiz Email Scan Client
Description           : This file contains calling functions from WardwizEmailScn dll.
Author Name			  : Amol Jaware
Date Of Creation      : 01 Aug 2018
Special Logic Used    : Communication using named pipes & using Shared Memory.
Modification Log      :
***********************************************************************************************************/

#include "WardWizEmailClient.h"
#include "WardWizDatabaseInterface.h"

/***********************************************************************************************
Function Name  : CWardWizEmailClient
Description    : Const'r
Author Name    : Amol Jaware
Date           : 31 Jul 2018
***********************************************************************************************/
CWardWizEmailClient::CWardWizEmailClient() :
m_hModuleEmailDLL(NULL)
, m_lpEmailInitialize(NULL)
, m_lpEmailUnInitialize(NULL)
, m_lpReloadAppRules(NULL)
, m_lpReloadEmailScanSettings(NULL)
, m_lpReloadParentalControlSettings(NULL)
{
}

/***********************************************************************************************
Function Name  : ~CWardWizEmailClient
Description    : Dest'r
Author Name    : Amol Jaware
Date           : 31 Jul 2018
***********************************************************************************************/
CWardWizEmailClient::~CWardWizEmailClient()
{
	//Unload here
	if (m_lpEmailUnInitialize != NULL)
	{
		m_lpEmailUnInitialize();
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
Author Name    : Amol Jaware
Date           : 31 Jul 2018
***********************************************************************************************/
bool CWardWizEmailClient::Initialize()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpEmailInitialize)
		{
			bReturn = m_lpEmailInitialize();
		}
		//creating tables for email's scan and parental control
		CreateTable4EmailScan();
		CreateTable4ParentalControl();
		CreateTable4BrowseSecurity();
		CreateTable4BrowserSec();
		InsertBrowseSessionReport();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::Initialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : UnInitialize
Description    : Function which uninitialize required library.
Author Name    : Amol Jaware
Date           : 01 Aug 2018
***********************************************************************************************/
bool CWardWizEmailClient::UnInitialize()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpEmailUnInitialize)
		{
			bReturn = m_lpEmailUnInitialize();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::UnInitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadRequiredModules
Description    : Function Loads required modules
Author Name    : Amol Jaware
Date           : 31 July 2018
***********************************************************************************************/
bool CWardWizEmailClient::LoadRequiredModules()
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
				AddLogEntryEx(SECONDLEVEL, L"### Error in Loading VBEMAILSCN.DLL, LastError:[%d]", GetLastError());
				return false;
			}

			m_lpEmailInitialize = (WRDWIZEMAILINITIALIZE)GetProcAddress(m_hModuleEmailDLL, "WrdWizEmailInitialize");
			if (!m_lpEmailInitialize)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::WardwizEmailInitialize", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpEmailInitialize = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpEmailUnInitialize = (WRDWIZEMAILUNINITIALIZE)GetProcAddress(m_hModuleEmailDLL, "WrdWizEmailUnInitialize");
			if (!m_lpEmailUnInitialize)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::WardwizEmailUnInitialize", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpEmailUnInitialize = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpReloadAppRules = (WRDWIZRELOADAPPRULES)GetProcAddress(m_hModuleEmailDLL, "ReloadApplicationRules");
			if (!m_lpReloadAppRules)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::ReloadApplicationRules", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpReloadAppRules = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpReloadEmailScanSettings = (RELOADEMAILSCANSETTINGS)GetProcAddress(m_hModuleEmailDLL, "ReLoadEmailScanSettings");
			if (!m_lpReloadEmailScanSettings)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::ReLoadEmailScanSettings", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpReloadEmailScanSettings = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpReloadParentalControlSettings = (RELOADPARENTALCONTROLSETTINGS)GetProcAddress(m_hModuleEmailDLL, "ReLoadParentalControlSettings");
			if (!m_lpReloadParentalControlSettings)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::ReLoadParentalControlSettings", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpReloadParentalControlSettings = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpStartParentalControlCheck = (STARTPARCONTROLCHECK)GetProcAddress(m_hModuleEmailDLL, "StartParControlCheck");
			if (!m_lpStartParentalControlCheck)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::StartParControlCheck", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpStartParentalControlCheck = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpStartInternetCheck = (STARTINTERNETCHECK)GetProcAddress(m_hModuleEmailDLL, "StartInternetCheck");
			if (!m_lpStartInternetCheck)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::StartInternetCheck", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpStartInternetCheck = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpStartINetUsageCheck = (STARTINETUSAGECHECK)GetProcAddress(m_hModuleEmailDLL, "StartINetUsageCheck");
			if (!m_lpStartINetUsageCheck)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::StartINetUsageCheck", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpStartINetUsageCheck = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpEnableDisableIU = (ENABLEDISABLEINTERNETUSAGE)GetProcAddress(m_hModuleEmailDLL, "EnableDisableInternetUsage");
			if (!m_lpEnableDisableIU)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::EnableDisableInternetUsage", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpEnableDisableIU = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpCheckEXEBlockedbyPC = (CHECKEXEISBLOCKED)GetProcAddress(m_hModuleEmailDLL, "CheckEXEBlockedbyPC");
			if (!m_lpCheckEXEBlockedbyPC)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::CheckEXEBlockedbyPC", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpCheckEXEBlockedbyPC = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpReloadApplicationList = (RELOADAPPLICATIONLIST)GetProcAddress(m_hModuleEmailDLL, "ReloadApplicationList");
			if (!m_lpReloadApplicationList)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::ReloadApplicationList", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpReloadApplicationList = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpReloadUserList = (RELOADUSERLIST)GetProcAddress(m_hModuleEmailDLL, "ReloadUserListforParCtrl");
			if (!m_lpReloadUserList)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::ReloadUserListforParCtrl", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpReloadUserList = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpReloadUserPermission = (RELOADUSERPERMISSION)GetProcAddress(m_hModuleEmailDLL, "ReloadUserPermission");
			if (!m_lpReloadUserPermission)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::ReloadUserPermission", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpReloadUserPermission = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpReloadUserResetValue = (RELOADUSERRESETVALUE)GetProcAddress(m_hModuleEmailDLL, "ReloadUserResetValue");
			if (!m_lpReloadUserResetValue)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::ReloadUserResetValue", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpReloadUserResetValue = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}

			m_lpCheckInternetValue = (CHECKINTERNETVALUE)GetProcAddress(m_hModuleEmailDLL, "CheckInternetAccessVal");
			if (!m_lpCheckInternetValue)
			{
				AddLogEntry(L"### CWardwizEmailClient:LoadRequiredModules Error in GetProcAddress::CheckInternetAccessVal", 0, 0, true, SECONDLEVEL);
				FreeLibrary(m_hModuleEmailDLL);
				m_lpCheckInternetValue = NULL;
				m_hModuleEmailDLL = NULL;
				return false;
			}
		}

		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::LoadRequiredModules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadUserList
Description    : Function reloads user list
Author Name    : Jeena Mariam Saji
Date           : 11 July 2019
***********************************************************************************************/
bool CWardWizEmailClient::ReloadUserList()
{
	bool bReturn = false;
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpReloadUserList != NULL)
		{
			bReturn = m_lpReloadUserList();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::ReloadUserList", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadUserList
Description    : Function reloads user list
Author Name    : Jeena Mariam Saji
Date           : 11 July 2019
***********************************************************************************************/
bool CWardWizEmailClient::ReloadUserPermission()
{
	bool bReturn = false;
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpReloadUserPermission != NULL)
		{
			bReturn = m_lpReloadUserPermission();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::ReloadUserPermission", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadApplicationRules4FW
Description    : Function Loads application rules for firewall.
Author Name    : Amol Jaware
Date           : 01 Aug 2018
***********************************************************************************************/
bool CWardWizEmailClient::ReloadApplicationRules4FW()
{
	bool bReturn = false;
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules(); 
		
		if (m_lpReloadAppRules != NULL)
		{
			bReturn = m_lpReloadAppRules();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::ReloadApplicationRules4FW", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ReLoadEmailScanSettingsTab
*  Description    : Function to reload Email scan settings tab.
*  Author Name    : Amol Jaware
*  Date			  :	07 Aug 2018
****************************************************************************************************/
void CWardWizEmailClient::ReLoadEmailScanSettingsTab(DWORD dwRegValue)
{
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpReloadEmailScanSettings)
			m_lpReloadEmailScanSettings(dwRegValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::ReLoadFWControlSettingsTab", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ReLoadEmailScanSettingsTab
*  Description    : Function to reload Email scan settings tab.
*  Author Name    : Amol Jaware
*  Date			  :	07 Aug 2018
****************************************************************************************************/
void CWardWizEmailClient::ReLoadParentalControlSettingsTab(DWORD dwRegValue)
{
	try
	{
		bool bReturn = false;

		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpReloadParentalControlSettings)
			bReturn = m_lpReloadParentalControlSettings(dwRegValue);

		if (bReturn)
			AddLogEntry(L"### Exception in CWardwizEmailClient::ReLoadParentalControlSettingsTab", 0, 0, true, SECONDLEVEL);
		else
			AddLogEntry(L"### Exception in CWardwizEmailClient::ReLoadParentalControlSettingsTab", 0, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::ReLoadFWControlSettingsTab", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : StartParentalControlCheck
*  Description    : Function to start Parental control check.
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
void CWardWizEmailClient::StartParentalControlCheck()
{
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpStartParentalControlCheck)
		{ 
			m_objISpyCriticalSection.Lock();
			m_lpStartParentalControlCheck();
			m_objISpyCriticalSection.Unlock();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::StartParentalControlCheck", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : StartPCInternetUsageCheck
*  Description    : Function to start Parental control Internet check.
*  Author Name    : Jeena Mariam Saji
*  Date			  :	07 Sept 2018
****************************************************************************************************/
void CWardWizEmailClient::StartPCInternetUsageCheck()
{
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpStartInternetCheck)
			m_lpStartInternetCheck();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::StartPCInternetUsageCheck", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : StartPCINetUsageCheck
*  Description    : Function to start Parental control internet usage check.
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
void CWardWizEmailClient::StartPCINetUsageCheck()
{
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpStartINetUsageCheck)
			m_lpStartINetUsageCheck();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::StartPCINetUsageCheck", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : EnableDisableInternetUsage
*  Description    : Function to enable disable Parental control internet usage check.
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
void CWardWizEmailClient::EnableDisableInternetUsage(bool bEnable)
{
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpEnableDisableIU)
			m_lpEnableDisableIU(bEnable);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::EnableDisableInternetUsage", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckIsEXEBlockedbyPC
*  Description    : Function to check if EXE is blocked.
*  Author Name    : Jeena Mariam Saji
*  Date			  :	13 Aug 2018
****************************************************************************************************/
bool CWardWizEmailClient::CheckIsEXEBlockedbyPC(LPTSTR lpszFilePath)
{
	bool bReturn = false;
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpCheckEXEBlockedbyPC)
		{
			bReturn = m_lpCheckEXEBlockedbyPC(lpszFilePath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::CheckIsEXEBlockedbyPC", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ReLoadBlockedAppList
*  Description    : Function to reload blocked exe list.
*  Author Name    : Jeena Mariam Saji
*  Date			  :	13 Aug 2018
****************************************************************************************************/
bool CWardWizEmailClient::ReLoadBlockedAppList()
{
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpReloadApplicationList)
			m_lpReloadApplicationList();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::ReLoadBlockedAppList", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReloadUserResetValue
*  Description    : Function to reload user reset value list.
*  Author Name    : Jeena Mariam Saji
*  Date			  :	01 Aug 2019
****************************************************************************************************/
bool CWardWizEmailClient::ReloadUserResetValue()
{
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpReloadUserResetValue)
			m_lpReloadUserResetValue();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::ReloadUserResetValue", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CreateTable4EmailScan
*  Description    : Function to create table for Emails scan.
*  Author Name    : Amol Jaware
*  Date			  :	12 Aug 2018
****************************************************************************************************/
void CWardWizEmailClient::CreateTable4EmailScan()
{
	try
	{
		CWardWizSQLiteDatabase ObjWardWizSQLiteDatabase(m_strDatabaseFilePath);
		ObjWardWizSQLiteDatabase.Open();
	
		if (!ObjWardWizSQLiteDatabase.TableExists("Wardwiz_EmailScanDetails"))
		{
			ObjWardWizSQLiteDatabase.ExecDML("CREATE TABLE [Wardwiz_EmailScanDetails] (\
															[db_EmailScanID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
															[db_EmailFrom] NVARCHAR(512) NOT NULL,\
															[db_EmailTo] NVARCHAR(512) NOT NULL,\
															[db_EmailSubject] NVARCHAR(512) NULL,\
															[db_EmailDate] DATE  NULL,\
															[db_EmailTime]  TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
															[db_EmailQurantinePath] NVARCHAR(512)  NULL,\
															[db_Reserved1] NVARCHAR(512)  NULL,\
															[db_Reserved2] NVARCHAR(128)  NULL,\
															[db_Reserved3] NVARCHAR(128)  NULL\
														)");
		}

		ObjWardWizSQLiteDatabase.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::CreateTable4EmailScan", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateTable4ParentalControl
*  Description    : Function to create table for Parental control.
*  Author Name    : Amol Jaware
*  Date			  :	12 Aug 2018
****************************************************************************************************/
void CWardWizEmailClient::CreateTable4ParentalControl()
{
	try
	{
		CWardWizSQLiteDatabase ObjWardWizSQLiteDatabase(m_strDatabaseFilePath);
		ObjWardWizSQLiteDatabase.Open();

		if (!ObjWardWizSQLiteDatabase.TableExists("Wardwiz_ParentalCtrl_Details"))
		{
			ObjWardWizSQLiteDatabase.ExecDML("CREATE TABLE [Wardwiz_ParentalCtrl_Details] (\
															[db_PCID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,\
															[db_PCDate] DATE  NULL,\
															[db_PCTime]  TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
															[db_PCActivity] INTEGER NULL,\
															[db_Username] NVARCHAR(512) NULL\
														)");
		}

		ObjWardWizSQLiteDatabase.Close();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::CreateTable4ParentalControl", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateTable4BrowseSecurity
*  Description    : Function to create table for Browser security and browse session.
*  Author Name    : Kunal Waghmare
*  Date			  :	16 July 2019
****************************************************************************************************/
void CWardWizEmailClient::CreateTable4BrowseSecurity()
{
	try
	{
		CWardWizSQLiteDatabase ObjWardWizSQLiteDatabase(m_strDatabaseFilePath);
		ObjWardWizSQLiteDatabase.Open();
		if (!ObjWardWizSQLiteDatabase.TableExists("Wardwiz_BrowseSession"))
		{
			ObjWardWizSQLiteDatabase.ExecDML("CREATE TABLE[Wardwiz_BrowseSession] (\
											BrowseSession_ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, SessionStartDate DATE DEFAULT CURRENT_DATE NULL,SessionStartTime TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
											Res1 NVARCHAR(512) NULL, Res2 NVARCHAR(512) NULL, Res3 NVARCHAR(512) NULL)");
		}
		if (!ObjWardWizSQLiteDatabase.TableExists("Wardwiz_Browse_Security"))
		{
			ObjWardWizSQLiteDatabase.ExecDML("CREATE TABLE [Wardwiz_Browse_Security] (\
											 [ID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL\
											 ,[PCDate] DATE NULL, [PCTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
											 [WebsiteName] NVARCHAR(512) NULL,[WebCategory] INTEGER NULL,[Username] NVARCHAR(512) NULL,\
											 BrowseSession_ID INTEGER NOT NULL,[Res1] NVARCHAR(512) NULL,[Res2] NVARCHAR(512) NULL,[Res3] NVARCHAR(512) NULL)");
		}
		ObjWardWizSQLiteDatabase.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::CreateTable4BrowseSecurity", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateTable4BrowserSec
*  Description    : Function to create table for Browser security and browse session.
*  Author Name    : Jeena Mariam Saji
*  Date			  :	04 Sept 2019
****************************************************************************************************/
void CWardWizEmailClient::CreateTable4BrowserSec()
{
	try
	{
		CWardWizSQLiteDatabase ObjWardWizSQLiteDatabase(m_strDatabaseFilePath);
		ObjWardWizSQLiteDatabase.Open();
		if (!ObjWardWizSQLiteDatabase.TableExists("Wardwiz_BrowseSession"))
		{
			ObjWardWizSQLiteDatabase.ExecDML("CREATE TABLE[Wardwiz_BrowseSession] (\
								BrowseSession_ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, SessionStartDate DATE DEFAULT CURRENT_DATE NULL,SessionStartTime TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
								Res1 NVARCHAR(512) NULL, Res2 NVARCHAR(512) NULL, Res3 NVARCHAR(512) NULL)");
		}
		if (!ObjWardWizSQLiteDatabase.TableExists("Wardwiz_Browser_Sec"))
		{
			ObjWardWizSQLiteDatabase.ExecDML("CREATE TABLE [Wardwiz_Browser_Sec] (\
											 		[ID] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL\
													,[db_BSDate] DATE NULL, [db_BSTime] TIMESTAMP DEFAULT CURRENT_TIMESTAMP NULL,\
													[WebsiteName] NVARCHAR(512) NULL,[WebCategory] INTEGER NULL,[Username] NVARCHAR(512) NULL,\
													BrowseSession_ID INTEGER NOT NULL,[Res1] NVARCHAR(512) NULL,[Res2] NVARCHAR(512) NULL,[Res3] NVARCHAR(512) NULL)");
		}
		ObjWardWizSQLiteDatabase.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::CreateTable4BrowserSec", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertBrowseSessionReport
*  Description    :	Insert into Browse Session table in report
*  Author Name    : Kunal Waghmare
*  Date           : 16 July 2019
**********************************************************************************************************/
void CWardWizEmailClient::InsertBrowseSessionReport()
{
	try
	{
		CWardWizSQLiteDatabase ObjWardWizSQLiteDatabase(m_strDatabaseFilePath);
		ObjWardWizSQLiteDatabase.Open();
		if (ObjWardWizSQLiteDatabase.TableExists("Wardwiz_BrowseSession"))
		{
			CStringA csInsertQuery;
			csInsertQuery = "INSERT INTO Wardwiz_BrowseSession(SessionStartDate, SessionStartTime)\
							   				VALUES (Date('now'),Datetime('now','localtime'));";
			ObjWardWizSQLiteDatabase.ExecDML(csInsertQuery.GetString());
		}
		ObjWardWizSQLiteDatabase.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::InsertBrowseSessionReport", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CheckInternetAccess
*  Description    : Function to check Internet Access
*  Author Name    : Jeena Mariam Saji
*  Date			  :	09 Dec 2019
****************************************************************************************************/
bool CWardWizEmailClient::CheckInternetAccess()
{
	bool bReturn = false;
	try
	{
		if (m_hModuleEmailDLL == NULL)
			LoadRequiredModules();

		if (m_lpCheckInternetValue)
		{
			bReturn = m_lpCheckInternetValue();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailClient::CheckInternetAccess", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}