// CWardwizSettingsUpdater.cpp : implementation file
/*********************************************************************
*  Program Name		: CWardwizSettingsUpdater.cpp
*  Description		: For setting Wardwiz settings.
*  Author Name		: 
*  Date Of Creation	: 19th Dec 2016
*  Version No		:2.0.0.1
**********************************************************************/

#include "stdafx.h"
#include "WardwizSettingsUpdater.h"

CString CWardwizSettingsUpdater::GetAppPath()
{
	try
	{
		HKEY	hSubKey = NULL;
		TCHAR	szModulePath[MAX_PATH] = { 0 };

		if (m_csRegKeyPath == "")
		{
			m_csRegKeyPath = L"SOFTWARE\\Wardwiz";//CWWizSettingsWrapper::GetProductRegistryKey();
		}

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
		{
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WardWiz Antivirus", 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
			{
				return L"";
			}
		}

		DWORD	dwSize = 511;
		DWORD	dwType = 0x00;

		RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize);
		RegCloseKey(hSubKey);
		hSubKey = NULL;

		if (_tcslen(szModulePath) > 0)
		{
			return CString(szModulePath);
		}
		return L"";
		m_AppPath = szModulePath;
		//TCHAR szValueAppVersion[MAX_PATH] = { 0 };
		//DWORD dwSizeAppVersion = sizeof(szValueAppVersion);

		//if (m_objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, m_csRegKeyPath.GetBuffer(), L"AppFolder", szValueAppVersion, dwSizeAppVersion) != 0x00)
		//{
		//	AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CWardwizSettingsUpdater::GetAppPath", 0, 0, true, SECONDLEVEL);
		//	return false;
		//}
		//m_AppPath = szValueAppVersion;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXUIApp::GetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return m_AppPath;
}

/***********************************************************************************************
Function Name  : CWardwizSettingsUpdater
Description    : Constructor
SR.NO		   :
Author Name    : Gayatri Afre
Date           : 19th Dec 2016
***********************************************************************************************/
CWardwizSettingsUpdater::CWardwizSettingsUpdater() : m_objComService(SERVICE_SERVER, true, 2)
{
	GetAppPath();
	INIPath = (m_AppPath) + (L"WRDSETTINGS\\PRODUCTSETTINGS.INI");
}

/***********************************************************************************************
Function Name  : CWardwizSettingsUpdater
Description    : Destructor
SR.NO		   :
Author Name    : Gayatri Afre
Date           : 19th Dec 2016
***********************************************************************************************/
CWardwizSettingsUpdater::~CWardwizSettingsUpdater()
{
}

/***********************************************************************************************
Function Name  : UpdateWardwizSettings
Description    : called when user requests change in settings entry
SR.NO		   :
Author Name    : Gayatri Afre
Date           : 19th Dec 2016
***********************************************************************************************/
bool CWardwizSettingsUpdater::UpdateWardwizSettings()
{
	try
	{
		int iButtonClickID = HEURISTIC_SCAN;
		//m_csRegKeyPath = L"SOFTWARE\\Wardwiz";
		//LPCTSTR settingsTabPath = L"COMPUTER\\HKEY_LOCAL_MACHINE\\SOFTWARE\\Wardwiz";//m_csRegKeyPath;
		LPCTSTR settingsTabPath = m_csRegKeyPath; //  L"\\SOFTWARE\\Wardwiz";

		bool bFlagValue = true;
		/*LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		const std::wstring chButtonClickID = svIndexValue.get(L"");
		int iButtonClickID = static_cast<int>(svIndexValue.d);
		bool bFlagValue = svBoolSettingsTrayNotify.get(false);
		int i = std::stoi(chButtonClickID);
		int iRegistryValue = static_cast<int>(svintSettingsRegValue.d);*/
		int iRegistryValue = 3;

		switch (iButtonClickID)
		{
		case TRAY_NOTIFICATION:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowTrayPopup", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwShowTrayPopup in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case DEL_OLD_REPORTS:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDeleteOldReports", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwDeleteOldReports in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case SHOW_TOOLTIP:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowStartupTips", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwShowStartupTips in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case SCAN_AT_STARTUP:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStartUpScan", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwStartUpScan in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case ENABLE_SOUND:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEnableSound", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwEnableSound in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case AUTO_LIVE_UPDATE:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoDefUpdate", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwAutoDefUpdate in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case DO_CACHING:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwCachingMethod", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwCachingMethod in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case PREP_CACHE_IN_BG:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBackgroundCaching", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwBackgroundCaching in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case USB_PROMPT:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwUsbScan", iRegistryValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwUsbScan in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case AUTO_QUARNTINE:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwQuarantineOption", iRegistryValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwQuarantineOption in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
		
			//Send here the settings to service.
			if (!SendSettingsToService(APPLYPRODUCTSETTINGS, AUTOQUARENTINEOPTION, iRegistryValue))
			{
				AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case HEURISTIC_SCAN:

			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwHeuScan", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwHeuScan in CGenXSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
		
			//Send here the settings to service.
			if (!SendSettingsToService(APPLYPRODUCTSETTINGS, RELOADSETTINGHEUSCAN, bFlagValue))
			{
				AddLogEntry(L"### Failed to SendSettingsToService in APPLYPRODUCTSETTINGS, RELOADSETTINGHEUSCAN", 0, 0, true, SECONDLEVEL);
			}
			break;
		
		case EMAIL_SCAN:
		{
			//if (m_dwProductID == 0x02)
			//{
			//	if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScan", bFlagValue))
			//	{
			//		AddLogEntry(L"### Error in Setting Registry dwEmailScan in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			//	}
			//}
			break;
		}
		
		default:
			break;
		}	
}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXSettingsUpdater::UpdateWardwizSettings", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : ReadCurrentSettingsValues
Description    : reads all registry entries
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
//json::value CWardwizSettingsUpdater::ReadCurrentSettingsValues(SCITER_VALUE scFunSetCurrentSettingsValueCB)
//{
//	try
//	{
//		ReadCurrentSettingsRegValues(scFunSetCurrentSettingsValueCB);
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::On_ButtonDeleteRecoverEntries", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value(0);
//}

/***********************************************************************************************
Function Name  : On_ClickApplyDefaultSettings
Description    : Apply default settings
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
************************************************************************************************/
//json::value CWardwizSettingsUpdater::On_ClickApplyDefaultSettings()
//{
//	try
//	{
//		ApplyDefaultSettings();
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::On_ClickApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value(0);
//}*/

/***********************************************************************************************
Function Name  : ReadCurrentSettingsRegValues
Description    : Read Current settings values from Registry
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
//void CWardwizSettingsUpdater::ReadCurrentSettingsRegValues(SCITER_VALUE scFunSetCurrentSettingsValueCB)
//{
//	try
//	{
//		CString strKey;
//		m_Key = theApp.m_csRegKeyPath;
//		for (DWORD dwValue = 1; dwValue <= 12; dwValue++)
//		{
//			switch (dwValue)
//			{
//			case 1:
//				strKey = L"dwShowTrayPopup";
//				break;
//			case 2:
//				strKey = L"dwDeleteOldReports";
//				break;
//			case 3:
//				strKey = L"dwShowStartupTips";
//				break;
//			case 4:
//				strKey = L"dwStartUpScan";
//				break;
//			case 5:
//				strKey = L"dwEnableSound";
//				break;
//			case 6:
//				strKey = L"dwAutoDefUpdate";
//				break;
//			case 7:
//				strKey = L"dwUsbScan";
//				break;
//			case 8:
//				strKey = L"dwQuarantineOption";
//				break;
//			case 9:
//				strKey = L"dwHeuScan";
//				break;
//			case 10:
//				strKey = L"dwBackgroundCaching";
//				break;
//			case 11:
//				strKey = L"dwCachingMethod";
//				break;
//			case 12:	// Keep this case @ Last.
//				strKey = L"dwEmailScan";
//				break;
//
//			default:
//				break;
//			}
//
//			DWORD dwType = REG_DWORD;
//			DWORD returnDWORDValue;
//			DWORD dwSize = sizeof(returnDWORDValue);
//			HKEY hKey;
//			returnDWORDValue = 0;
//			LONG lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_Key, 0, KEY_READ, &hKey);
//			if (lResult == ERROR_SUCCESS)
//				lResult = ::RegQueryValueEx(hKey, strKey, NULL, &dwType, (LPBYTE)&returnDWORDValue, &dwSize);
//			if (lResult == ERROR_SUCCESS)
//			{
//				::RegCloseKey(hKey);
//				//return returnDWORDValue;
//				CString csKey = L"";
//				csKey.Format(L"%d", dwValue);
//				CString csValue = L"";
//				csValue.Format(L"%d", returnDWORDValue);
//				scFunSetCurrentSettingsValueCB.call((SCITER_STRING)csKey, (SCITER_STRING)csValue);
//			}
//			else
//			{
//				::RegCloseKey(hKey);
//				//return;
//			}
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"Exception in CWardwizSettingsUpdater::ReadCurrentSettingsRegValues", 0, 0, true, SECONDLEVEL);
//	}
//	return;
//}
//
///***********************************************************************************************
//Function Name  : On_ClickSettingsButton
//Description    : Write Current settings values from Registry
//SR.NO		   :
//Author Name    : Nitin Kolapkar
//Date           : 10th May 2016
//***********************************************************************************************/
//void CWardwizSettingsUpdater::On_ClickSettingsButton(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue)
//{
//	try
//	{
//		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
//		//const std::wstring chButtonClickID = svIndexValue.get(L"");
//		int iButtonClickID = static_cast<int>(svIndexValue.d);
//		bool bFlagValue = svBoolSettingsTrayNotify.get(false);
//		//int i = std::stoi(chButtonClickID);
//		int iRegistryValue = static_cast<int>(svintSettingsRegValue.d);
//
//		switch (iButtonClickID)
//		{
//		case TRAY_NOTIFICATION:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowTrayPopup", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwShowTrayPopup in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case DEL_OLD_REPORTS:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDeleteOldReports", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwDeleteOldReports in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case SHOW_TOOLTIP:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowStartupTips", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwShowStartupTips in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case SCAN_AT_STARTUP:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStartUpScan", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwStartUpScan in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case ENABLE_SOUND:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEnableSound", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwEnableSound in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case AUTO_LIVE_UPDATE:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoDefUpdate", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwAutoDefUpdate in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case DO_CACHING:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwCachingMethod", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwCachingMethod in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case PREP_CACHE_IN_BG:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBackgroundCaching", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwBackgroundCaching in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case USB_PROMPT:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwUsbScan", iRegistryValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwUsbScan in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case AUTO_QUARNTINE:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwQuarantineOption", iRegistryValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwQuarantineOption in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//
//			//Send here the settings to service.
//			if (!SendSettingsToService(APPLYPRODUCTSETTINGS, AUTOQUARENTINEOPTION, iRegistryValue))
//			{
//				AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case HEURISTIC_SCAN:
//			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwHeuScan", bFlagValue))
//			{
//				AddLogEntry(L"### Error in Setting Registry dwHeuScan in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//			}
//
//			//Send here the settings to service.
//			if (!SendSettingsToService(APPLYPRODUCTSETTINGS, RELOADSETTINGHEUSCAN, bFlagValue))
//			{
//				AddLogEntry(L"### Failed to SendSettingsToService in APPLYPRODUCTSETTINGS, RELOADSETTINGHEUSCAN", 0, 0, true, SECONDLEVEL);
//			}
//			break;
//
//		case EMAIL_SCAN:
//		{
//			if (theApp.m_dwProductID == 0x02)
//			{
//				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScan", bFlagValue))
//				{
//					AddLogEntry(L"### Error in Setting Registry dwEmailScan in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//				}
//			}
//			break;
//		}
//
//		default:
//			break;
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
//	}
//}

/***************************************************************************************************
*  Function Name  : WriteRegistryEntryOfSettingsTab
*  Description    : this function is  called for Writing the Registry Key Value for Menu Items
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CWardwizSettingsUpdater::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue)
{
	try
	{
		if (!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue, true))
		{
			AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
		}
		Sleep(20);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXSettingsUpdater::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : this function is  called for setting the Registry Keys using the Services
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CWardwizSettingsUpdater::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;
		//szPipeData.hHey = HKEY_LOCAL_MACHINE;

		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;

		CISpyCommunicator objCom(SERVICE_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXSettingsUpdater::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ApplyDefaultSettings
*  Description    : this function is  called for setting default the Registry Keys using the Services
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
bool CWardwizSettingsUpdater::ApplyDefaultSettings()
{
	try
	{
		//LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		bool bFlagValue = true;
		//if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowTrayPopup", bFlagValue))
		/*{
			AddLogEntry(L"### Error in Setting Registry dwShowTrayPopup in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDeleteOldReports", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwDeleteOldReports in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDaysToDelRep", 30))
		{
			AddLogEntry(L"### Error in Setting Registry dwDaysToDelRep in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowStartupTips", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwShowStartupTips in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwUsbScan", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwUsbScan in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStartUpScan", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwStartUpScan in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEnableSound", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwEnableSound in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoDefUpdate", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwAutoDefUpdate in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwCachingMethod", !bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwCachingMethod in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBackgroundCaching", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwBackgroundCaching in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwUsbScan", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwUsbScan in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwQuarantineOption", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwQuarantineOption in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScan", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwEmailScan in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwHeuScan", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwHeuScan in CWardwizSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}*/
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CGenXSettingsUpdater::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SetDelRepDays
*  Description    : This function is  called for setting Registry Keys of Delete Old Reports.
*  Author Name    : Yogeshwar S. Rasal
*  Date           :	11 July 2016
****************************************************************************************************/
//json::value CWardwizSettingsUpdater::SetDelRepDays(SCITER_VALUE svDaysToDelRep)
//{
//	try
//	{
//		int iValue = 0;
//		int	m_iValue = svDaysToDelRep.get(iValue);
//		CString csDays;
//		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
//
//		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDaysToDelRep", m_iValue))
//		{
//			csDays.Format(L"%d", m_iValue);
//			AddLogEntry(L"### Error in Setting Registry for dwDaysToDelRep in CWardwizSettingsUpdater::SetDelRepDays as %s", csDays, 0, true, SECONDLEVEL);
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::SetDelRepDays", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value(0);
//}
//
///***************************************************************************************************
//*  Function Name  : SetScanStartup
//*  Description    : This function is  called for setting Registry Keys of Startup Scan
//*  Author Name    : Yogeshwar S. Rasal
//*  Date           :	11 July 2016
//****************************************************************************************************/
//json::value CWardwizSettingsUpdater::SetScanStartup(SCITER_VALUE svScanType)
//{
//	int iScanType = 0;
//	int m_iScanType = svScanType.get(iScanType);
//	CString csType = L"";
//	try
//	{
//		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
//
//		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStartUpScan", m_iScanType))
//		{
//			csType.Format(L"%d", m_iScanType);
//			AddLogEntry(L"### Error in Setting Registry for dwStartUpScan in CWardwizSettingsUpdater::SetScanStartup as %s", csType, 0, true, SECONDLEVEL);
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::SetScanStartup", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value(m_iScanType);
//}

//
///***************************************************************************************************
//*  Function Name  : GetDeleteDaysRegVal
//*  Description    : This function is used for reading Registry Keys of Days To Delete Reports.
//*  Author Name    : Yogeshwar S. Rasal
//*  Date           :	11 July 2016
//****************************************************************************************************/
//json::value CWardwizSettingsUpdater::GetDeleteDaysRegVal()
//{
//	DWORD dwDaysToDelRep = 0x1e;
//	int iValue = 0;
//	try
//	{
//		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwDaysToDelRep", dwDaysToDelRep) != 0x00)
//		{
//			AddLogEntry(L"### Failed to read DWORD Value dwDaysToDelRep in CWardwizSettingsUpdater::GetDeleteDaysRegVal", 0, 0, true, SECONDLEVEL);
//			return json::value(0);
//		}
//		iValue = static_cast<int>(dwDaysToDelRep);
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::GetDeleteDaysRegVal", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value(iValue);
//}
//
///***************************************************************************************************
//*  Function Name  : GetStartupScanType
//*  Description    : This function is used for setting Startup scan type.
//*  Author Name    : Yogeshwar S. Rasal
//*  Date           :	11 July 2016
//****************************************************************************************************/
//json::value CWardwizSettingsUpdater::GetStartupScanType()
//{
//
//	DWORD dwStartUpScan = 0x01;
//	int iValue = 0;
//	try
//	{
//		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwStartUpScan", dwStartUpScan) != 0x00)
//		{
//			AddLogEntry(L"### Failed to read DWORD Value dwStartUpScan in CWardwizSettingsUpdater::GetStartupScanType", 0, 0, true, SECONDLEVEL);
//			return json::value(0);
//		}
//		iValue = static_cast<int>(dwStartUpScan);
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::GetStartupScanType", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value(iValue);
//}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : This function is used send DB file path to UI
*  Author Name    : Adil Sheikh
*  Date           :	22 July 2016
****************************************************************************************************/
//json::value CWardwizSettingsUpdater::GetDBPath()
//{
//	TCHAR  szActualIPath[MAX_PATH] = { 0 };
//	try
//	{
//		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"WWIZEXCLUDE.DB");
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::GetDBPath", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value((SCITER_STRING)szActualIPath);
//}

/***************************************************************************************************
*  Function Name  : SendSettingsToService
*  Description    : Function to send settings to service.
*  Author Name    : Ramkrushna shelke
*  Date           :	05 Aug 2016
****************************************************************************************************/
bool CWardwizSettingsUpdater::SendSettingsToService(int iMessageInfo, DWORD dwValue, DWORD dwSecondValue)
{
	__try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = dwValue;
		szPipeData.dwSecondValue = dwSecondValue;
		if (!m_objComService.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in SendSettingsToService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		TCHAR szMessage[MAX_PATH] = { 0 };
		swprintf_s(szMessage, L"MessageID: %d, Value: %d, SecondValue: %d", iMessageInfo, dwValue, dwSecondValue);
		AddLogEntry(L"### Exception in CGenXSettingsUpdater::SendSettingsToService, %s", szMessage, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : On_AddExcludeFileFolder
Description    : Apply default settings
SR.NO		   :
Author Name    : Adil Sheikh
Date           : 6th Aug 2016
***********************************************************************************************/
//json::value CWardwizSettingsUpdater::On_AddExcludeFileFolder()
//{
//	try
//	{
//		//Send here the settings to service.
//		if (!SendSettingsToService(APPLYPRODUCTSETTINGS, EXCLUDEFILESSCANUPDATED, 0x01))
//		{
//			AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::On_AddExcludeFileFolder()", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value(0);
//}

/***************************************************************************
Function Name  : GetInformationFromINI
Description    : used to read PRODUCTSETTINGS.ini file.
Author Name    : Adil Sheikh
SR_NO		   :
Date           : 8th july 2016
****************************************************************************/
bool CWardwizSettingsUpdater::GetInformationFromINI()
{
	try
	{
		TCHAR  szActualINIPath[MAX_PATH] = { 0 };
	/*	if (theApp.m_dwProductID)
		{
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%sWRDSETTINGS\\%s", theApp.m_AppPath, L"PRODUCTSETTINGS.ini");
		}*/

		if (szActualINIPath == NULL)
		{
			AddLogEntry(L"### File not found : %s in GetInformationFromINI ", szActualINIPath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in CGenXSettingsUpdater::GetInformationFromINI ", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  On_SelectFileFolder
*  Description    :  Used to select File/Folder to Unhide.
*  Author Name    :  Adil Sheikh
*  Date           :  7th Sept 2016
****************************************************************************************************/
//json::value CWardwizSettingsUpdater::On_SelectFileFolder()
//{
//	CString csReturn = L"";
//	try
//	{
//		csReturn = OnBnClickedButtonBrowsefolder();
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::On_SelectFileFolder", 0, 0, true, SECONDLEVEL);
//	}
//	return json::value((SCITER_STRING)csReturn);
//}

/***************************************************************************************************
*  Function Name  :  OnBnClickedButtonBrowsefolder
*  Description    :  Used to Handle Action for Browse Button
*  Author Name    :
*  Date           :
****************************************************************************************************/
//CString CWardwizSettingsUpdater::OnBnClickedButtonBrowsefolder()
//{
//	CString csReturn = L"";
//	try
//	{
//		TCHAR szPath[MAX_PATH] = { 0 };
//		CString csTemp;
//
//		BROWSEINFO        bi = { NULL, NULL, szPath, 0, BIF_BROWSEINCLUDEFILES, NULL, 0L, 0 };		//BIF_RETURNONLYFSDIRS
//		LPITEMIDLIST      pIdl = NULL;
//
//		LPITEMIDLIST  pRoot = NULL;
//		bi.pidlRoot = pRoot;
//		pIdl = SHBrowseForFolder(&bi);
//		if (NULL != pIdl)
//		{
//			SHGetPathFromIDList(pIdl, szPath);
//			size_t iLen = wcslen(szPath);
//			if (iLen > 0)
//			{
//				csTemp = szPath;
//				if (csTemp.Right(1) == L"\\")
//				{
//					csTemp = csTemp.Left(static_cast<int>(iLen)-1);
//				}
//				csReturn = csTemp;
//			}
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::OnBnClickedButtonBrowsefolder", 0, 0, true, SECONDLEVEL);
//	}
//	return csReturn;
//}

/**********************************************************************************************************
*  Function Name  :	CheckForNetworkPath
*  Description    :	To check weather file is network file or not.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 9 Sept. 2016
**********************************************************************************************************/
//json::value CWardwizSettingsUpdater::CheckForNetworkPath(SCITER_VALUE svFilePath)
//{
//	try
//	{
//		const std::wstring  chFilePath = svFilePath.get(L"");
//		if (PathIsNetworkPath((LPTSTR)chFilePath.c_str()))
//		{
//			return true;
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::CheckForNetworkPath", 0, 0, true, SECONDLEVEL);
//	}
//	return false;
//}

/**********************************************************************************************************
*  Function Name  :	CheckFileOrFolderPath
*  Description    :	To check weather path is filepath or folderpath.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 23 Sept. 2016
**********************************************************************************************************/
//json::value CWardwizSettingsUpdater::CheckFileOrFolderPath(SCITER_VALUE svFileFolderPath)
//{
//	try
//	{
//		const std::wstring  chFilePath = svFileFolderPath.get(L"");
//		if (PathIsDirectory((LPTSTR)chFilePath.c_str()))
//		{
//			return true;
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardwizSettingsUpdater::CheckFileOrFolderPath", 0, 0, true, SECONDLEVEL);
//	}
//	return false;
//}