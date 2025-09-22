// CWardWizSettings.cpp : implementation file
/*********************************************************************
*  Program Name		: CWardWizSettings.cpp
*  Description		: For changing default settings.
*  Author Name		: Nitin Kolapkar
*  Date Of Creation	: 6th May 2016
*  Version No		: 2.0.0.1
**********************************************************************/

#include "stdafx.h"
#include "WardWizSettings.h"
#include "CScannerLoad.h"
#include "ExcludeFilesFolders.h"
#include "ScanByName.h"

#define ADD_SERVER_LIST_EVENT (FIRST_APPLICATION_EVENT_CODE + 12)

/***********************************************************************************************
Function Name  : CWardWizSettings
Description    : Constructor
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
CWardWizSettings::CWardWizSettings() : behavior_factory("WardWizSettings")
, m_objComService(SERVICE_SERVER, true, 3)
{
}

/***********************************************************************************************
Function Name  : CWardWizSettings
Description    : Destructor
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
CWardWizSettings::~CWardWizSettings()
{
}

/***********************************************************************************************
Function Name  : On_ClickChangeSettings
Description    : called when user change in settings entry
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
json::value CWardWizSettings::On_ClickChangeSettings(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue)
{
	try
	{
		On_ClickSettingsButton(svIndexValue, svBoolSettingsTrayNotify, svintSettingsRegValue);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_ButtonDeleteRecoverEntries", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : ReadCurrentSettingsValues
Description    : reads all registry entries
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
json::value CWardWizSettings::ReadCurrentSettingsValues(SCITER_VALUE scFunSetCurrentSettingsValueCB)
{
	try
	{
		ReadCurrentSettingsRegValues(scFunSetCurrentSettingsValueCB);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_ButtonDeleteRecoverEntries", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : On_ClickApplyDefaultSettings
Description    : Apply default settings
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
json::value CWardWizSettings::On_ClickApplyDefaultSettings()
{
	try
	{
		ApplyDefaultSettings();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_ClickApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : ReadCurrentSettingsRegValues
Description    : Read Current settings values from Registry
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
void CWardWizSettings::ReadCurrentSettingsRegValues(SCITER_VALUE scFunSetCurrentSettingsValueCB)
{
	try
	{
		CString		strKey;
		int			iDefaultValue = 0;

		m_Key = theApp.m_csRegKeyPath;
		for (DWORD dwValue = 1; dwValue <= 18; dwValue++)
		{
			switch (dwValue)
			{
			case 1:
				strKey = L"dwShowTrayPopup";
				break;
			case 2:
				strKey = L"dwDeleteOldReports";
				break;
			case 3:
				strKey = L"dwShowStartupTips";
				break;
			case 4:
				strKey = L"dwStartUpScan";
				break;
			case 5:
				strKey = L"dwEnableSound";
				break;
			case 6:
				strKey = L"dwAutoDefUpdate";
				break;
			case 7:
				strKey = L"dwUsbScan";
				break;
			case 8:
				strKey = L"dwQuarantineOption";
				break;
			case 9:
				strKey = L"dwHeuScan";
				break;
			case 10:
				strKey = L"dwBackgroundCaching"; 
				break;
			case 11:
				strKey = L"dwCachingMethod";
				break;
			case 12:
				strKey = L"dwAutoProductUpdate";
				break;
			case 13:	// Keep this case @ Last.
				strKey = L"dwEmailScan";
				break;
			case 14:
				strKey = L"dwBlockAutorun";
				iDefaultValue = 1;
				break;
			case 15:
				strKey = L"dwExecuteBlock";
				iDefaultValue = 0;
				break;
			case 16:
				strKey = L"dwWriteBlock";
				iDefaultValue = 0;
				break;
			case 17:
				strKey = L"dwScheduledScan";
				iDefaultValue = 0;
				break;
			case 18:
				strKey = L"dwWidgetsUIState";
				iDefaultValue = 1;
				break;
			default:
				break;
			}

			DWORD dwType = REG_DWORD;
			DWORD returnDWORDValue;
			DWORD dwSize = sizeof(returnDWORDValue);
			HKEY hKey;
			returnDWORDValue = 0;
			LONG lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_Key, 0, KEY_READ, &hKey);
			if (lResult == ERROR_SUCCESS)
			{
				lResult = ::RegQueryValueEx(hKey, strKey, NULL, &dwType, (LPBYTE)&returnDWORDValue, &dwSize);

				if (lResult != ERROR_SUCCESS)
				{
					CString csKey = L"";
					csKey.Format(L"%d", dwValue);
					CString csValue = L"";
					csValue.Format(L"%d", iDefaultValue);
					CreateRegistryEntryForExternalDevicesSettings(strKey, iDefaultValue);
					scFunSetCurrentSettingsValueCB.call((SCITER_STRING)csKey, (SCITER_STRING)csValue);
				}
			}

			if (lResult == ERROR_SUCCESS)
			{
				::RegCloseKey(hKey);
				CString csKey = L"";
				csKey.Format(L"%d", dwValue);
				CString csValue = L"";
				csValue.Format(L"%d", returnDWORDValue);
				scFunSetCurrentSettingsValueCB.call((SCITER_STRING)csKey, (SCITER_STRING)csValue);
			}
			else
			{
				::RegCloseKey(hKey);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizSettings::ReadCurrentSettingsRegValues", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***********************************************************************************************
Function Name  : CreateRegistryEntryForExternalDevicesSettings
Description    : Write the default registry entry for given Registry values
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 06th Jan 2017
***********************************************************************************************/
void CWardWizSettings::CreateRegistryEntryForExternalDevicesSettings(CString strKey, int iDefaultValue)
{
	try
	{
		LPCTSTR lpszsettingsPath = (LPCTSTR)theApp.m_csRegKeyPath;
		bool bFlagValue = false;
		if (strKey == L"dwBlockAutorun" || strKey == "dwExecuteBlock" || strKey == "dwWriteBlock")
		{
			iDefaultValue == 1 ? bFlagValue = true : bFlagValue = false;
			if (!WriteRegistryEntryOfSettingsTab(lpszsettingsPath, strKey, bFlagValue))
			{
				AddLogEntry(L"### Error in writing Registry in CWardwizSettings::CreateRegistryEntryForExternalDevicesSettings for %s", strKey, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizSettings::CreateRegistryEntryForExternalDevicesSettings for %s", strKey, 0, true, SECONDLEVEL);
	}
}
/***********************************************************************************************
Function Name  : On_ClickSettingsButton
Description    : Write Current settings values from Registry
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 10th May 2016
***********************************************************************************************/
void CWardWizSettings::On_ClickSettingsButton(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue)
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		int iButtonClickID = static_cast<int>(svIndexValue.d);
		bool bFlagValue = svBoolSettingsTrayNotify.get(false);
		int iRegistryValue = static_cast<int>(svintSettingsRegValue.d);
		switch (iButtonClickID)
		{
		case TRAY_NOTIFICATION:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowTrayPopup", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwShowTrayPopup in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case DEL_OLD_REPORTS:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDeleteOldReports", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwDeleteOldReports in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case SHOW_TOOLTIP:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowStartupTips", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwShowStartupTips in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case SCAN_AT_STARTUP:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStartUpScan", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwStartUpScan in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case ENABLE_SOUND:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEnableSound", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwEnableSound in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case AUTO_LIVE_UPDATE:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoDefUpdate", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwAutoDefUpdate in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case DO_CACHING:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwCachingMethod", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwCachingMethod in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}

			// bFlagValue: true => momentary cache
			// bFlagValue: false => tenacious cache
			if (!bFlagValue)
			{
				//Send here the settings to service.
				if (!SendSettingsToService(CLEAR_INDEXING, 0x00, 0x00))
				{
					AddLogEntry(L"### Failed to SendSettingsToService in CLEAR_INDEXING", 0, 0, true, SECONDLEVEL);
				}
			}
			break;

		case PREP_CACHE_IN_BG:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBackgroundCaching", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwBackgroundCaching in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}

			if (!SendSettingsToService(START_STOP_INDEXING, bFlagValue ? 0x01 : 0x00, 0x00))
			{
				AddLogEntry(L"### Failed to SendSettingsToService in START_STOP_INDEXING", 0, 0, true, SECONDLEVEL);
			}
			break;

		case USB_PROMPT:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwUsbScan", iRegistryValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwUsbScan in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case AUTO_QUARNTINE:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwQuarantineOption", iRegistryValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwQuarantineOption in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}

			SetAutoQurantineOptionInReg(iRegistryValue);

			if (!SendSettingsToService(APPLYPRODUCTSETTINGS, AUTOQUARENTINEOPTION, iRegistryValue))
			{
				AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
			}
			break;

		case HEURISTIC_SCAN:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwHeuScan", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwHeuScan in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}

			if (!SendSettingsToService(APPLYPRODUCTSETTINGS, RELOADSETTINGHEUSCAN, bFlagValue))
			{
				AddLogEntry(L"### Failed to SendSettingsToService in APPLYPRODUCTSETTINGS, RELOADSETTINGHEUSCAN", 0, 0, true, SECONDLEVEL);
			}
			break;

		case EMAIL_SCAN:
		{
			if (theApp.m_dwProductID == 0x02)
			{
				if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScan", bFlagValue))
				{
					AddLogEntry(L"### Error in Setting Registry dwEmailScan in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
				}
			}
			break;
		}

		case AUTO_PRODUCUT_UPDATE:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoProductUpdate", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwAutoProductUpdate in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case BLOCK_AUTORUN:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBlockAutorun", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwBlockAutorun in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}

			if(!ApplyAutorunSettings())
			{
				AddLogEntry(L"### Failed: Function ApplyAutorunSettings CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case EXECUTE_BLOCK:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwExecuteBlock", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwExecuteBlock in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}

			if (!ApplyAutorunSettings())
			{
				AddLogEntry(L"### Failed: Function ApplyAutorunSettings CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case WRITE_BLOCK:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwWriteBlock", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwWriteBlock in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			
			}

			if (!ApplyAutorunSettings())
			{
				AddLogEntry(L"### Failed: Function ApplyAutorunSettings CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
			}
			break;

		case SCHED_SCAN:
			if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwScheduledScan", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwScheduledScan in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);

			}
			break;
		case WIDGET_UI:
			CallTrayForWidget(bFlagValue);
			break;
		default:
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_ClickSettingsButton", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : WriteRegistryEntryOfSettingsTab
*  Description    : this function is  called for Writing the Registry Key Value for Menu Items
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 25 April 2014
****************************************************************************************************/
BOOL CWardWizSettings::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue)
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
		AddLogEntry(L"### Exception in CWardwizSettings::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
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
bool CWardWizSettings::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;
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
		AddLogEntry(L"### Exception in CWardwizSettings::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
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
bool CWardWizSettings::ApplyDefaultSettings()
{
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		bool bFlagValue = true;
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowTrayPopup", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwShowTrayPopup in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDeleteOldReports", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwDeleteOldReports in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDaysToDelRep", 30))
		{
			AddLogEntry(L"### Error in Setting Registry dwDaysToDelRep in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwShowStartupTips", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwShowStartupTips in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwUsbScan", 2))
		{
			AddLogEntry(L"### Error in Setting Registry dwUsbScan in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStartUpScan", false))
		{
			AddLogEntry(L"### Error in Setting Registry dwStartUpScan in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEnableSound", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwEnableSound in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoDefUpdate", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwAutoDefUpdate in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwCachingMethod", 0x00))
		{
			AddLogEntry(L"### Error in Setting Registry dwCachingMethod in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBackgroundCaching", 0x00))
		{
			AddLogEntry(L"### Error in Setting Registry dwBackgroundCaching in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwQuarantineOption", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwQuarantineOption in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwEmailScan", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwEmailScan in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwHeuScan", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwHeuScan in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwAutoProductUpdate", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwAutoProductUpdate in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwBlockAutorun", bFlagValue))
		{
			AddLogEntry(L"### Error in Setting Registry dwBlockAutorun in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwExecuteBlock", false))
		{
			AddLogEntry(L"### Error in Setting Registry dwExecuteBlock in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwWriteBlock", false))
		{
			AddLogEntry(L"### Error in Setting Registry dwWriteBlock in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		if (!ApplyAutorunSettings())
		{
			AddLogEntry(L"### Failed: Function ApplyAutorunSettings in CWardwizSettings::ApplyDefaultSettingsn", 0, 0, true, SECONDLEVEL);
		}
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwScheduledScan", false))
		{
			AddLogEntry(L"### Error in Setting Registry dwScheduledScan in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		LPCTSTR UsbBlockPath = _T("SYSTEM\\CurrentControlSet\\services\\USBSTOR");
		if (!WriteRegistryEntryOfSettingsTab(UsbBlockPath, L"Start", 0x03))
		{
			AddLogEntry(L"### Error in Setting Registry dwScheduledScan in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
		}

		CallTrayForWidget(false);
		//CallTrayForWidget(bFlagValue);
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizSettings::ApplyDefaultSettings", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SetDelRepDays
*  Description    : This function is  called for setting Registry Keys of Delete Old Reports.
*  Author Name    : Yogeshwar S. Rasal
*  Date           :	11 July 2016
****************************************************************************************************/
json::value CWardWizSettings::SetDelRepDays(SCITER_VALUE svDaysToDelRep)
{
	try
	{
		int iValue = 0;
		int	m_iValue = svDaysToDelRep.get(iValue);
		CString csDays;
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;

		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwDaysToDelRep", m_iValue))
		{
			csDays.Format(L"%d",m_iValue);
			AddLogEntry(L"### Error in Setting Registry for dwDaysToDelRep in CWardwizSettings::SetDelRepDays as %s", csDays, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::SetDelRepDays", 0, 0, true, SECONDLEVEL);
	}
		return json::value(0);
}

/***************************************************************************************************
*  Function Name  : SetScanStartup
*  Description    : This function is  called for setting Registry Keys of Startup Scan
*  Author Name    : Yogeshwar S. Rasal
*  Date           :	11 July 2016
****************************************************************************************************/
json::value CWardWizSettings::SetScanStartup(SCITER_VALUE svScanType)
{
	int iScanType = 0;
	int m_iScanType = svScanType.get(iScanType);
	CString csType = L"";
	try
	{
		LPCTSTR settingsTabPath = (LPCTSTR)theApp.m_csRegKeyPath;
		if (!WriteRegistryEntryOfSettingsTab(settingsTabPath, L"dwStartUpScan", m_iScanType))
		{
			csType.Format(L"%d", m_iScanType);
			AddLogEntry(L"### Error in Setting Registry for dwStartUpScan in CWardwizSettings::SetScanStartup as %s", csType, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::SetScanStartup", 0, 0, true, SECONDLEVEL);
	}
	return json::value(m_iScanType);
}

/***************************************************************************************************
*  Function Name  : GetDeleteDaysRegVal
*  Description    : This function is used for reading Registry Keys of Days To Delete Reports.
*  Author Name    : Yogeshwar S. Rasal
*  Date           :	11 July 2016
****************************************************************************************************/
json::value CWardWizSettings::GetDeleteDaysRegVal()
{
	DWORD dwDaysToDelRep = 0x1e;
	int iValue = 0;
	try
	{
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwDaysToDelRep", dwDaysToDelRep) != 0x00)
		{
			AddLogEntry(L"### Failed to read DWORD Value dwDaysToDelRep in CWardwizSettings::GetDeleteDaysRegVal", 0, 0, true, SECONDLEVEL);
			return json::value(0);
		}
		iValue = static_cast<int>(dwDaysToDelRep);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::GetDeleteDaysRegVal", 0, 0, true, SECONDLEVEL);
	}
	return json::value(iValue);
}

/***************************************************************************************************
*  Function Name  : GetStartupScanType
*  Description    : This function is used for setting Startup scan type.
*  Author Name    : Yogeshwar S. Rasal
*  Date           :	11 July 2016
****************************************************************************************************/
json::value CWardWizSettings::GetStartupScanType()
{
	DWORD dwStartUpScan = 0x01;
	int iValue = 0;
	try
	{
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwStartUpScan", dwStartUpScan) != 0x00)
		{
			AddLogEntry(L"### Failed to read DWORD Value dwStartUpScan in CWardwizSettings::GetStartupScanType", 0, 0, true, SECONDLEVEL);
			return json::value(0);
		}
		iValue = static_cast<int>(dwStartUpScan);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::GetStartupScanType", 0, 0, true, SECONDLEVEL);
	}
	return json::value(iValue);
}

/***************************************************************************************************
*  Function Name  : GetDBPath
*  Description    : This function is used send DB file path to UI
*  Author Name    : Adil Sheikh
*  Date           :	22 July 2016
****************************************************************************************************/
json::value CWardWizSettings::GetDBPath()
{ 
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{ 
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBEXCLUDE.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::GetDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : GetScanNameDBPath
*  Description    : This function is used to create and send the path of VBSCANNAME.DB.
*  Author Name    : Amol Jaware
*  Date           :	04 Oct 2018
****************************************************************************************************/
json::value CWardWizSettings::GetScanNameDBPath()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBSCANNAME.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::GetScanNameDBPath", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***************************************************************************************************
*  Function Name  : SendSettingsToService
*  Description    : Function to send settings to service.
*  Author Name    : Ramkrushna shelke
*  Date           :	05 Aug 2016
****************************************************************************************************/
bool CWardWizSettings::SendSettingsToService(int iMessageInfo, DWORD dwValue, DWORD dwSecondValue)
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
		AddLogEntry(L"### Exception in CWardwizSettings::SendSettingsToService, %s", szMessage, 0, true, SECONDLEVEL);
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
json::value CWardWizSettings::On_AddExcludeFileFolder()
{
	try
	{
		//Send here the settings to service.
		if (!SendSettingsToService(APPLYPRODUCTSETTINGS, EXCLUDEFILESSCANUPDATED, 0x01))
		{
			AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_AddExcludeFileFolder()", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : On_AddExcludeFileExtension
Description    : Apply default settings
SR.NO		   :
Author Name    : Amol Jaware
Date           : 09 Oct 2018
***********************************************************************************************/
json::value CWardWizSettings::On_AddExcludeFileExtension()
{
	try
	{
		//Send here the settings to service.
		if (!SendSettingsToService(APPLYPRODUCTSETTINGS, EXCLUDEFILESSCANUPDATED, 0x01))
		{
			AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_AddExcludeFileExtension", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : On_AddExcludeFileFolder
Description    : Apply default settings
SR.NO		   :
Author Name    : Amol Jaware
Date           : 05 Oct 2018
***********************************************************************************************/
json::value CWardWizSettings::On_LoadScanByNameDB()
{
	try
	{
		//Send here the settings to service.
		if (!SendSettingsToService(APPLYPRODUCTSETTINGS, SCANBYNAMEFILE, 0x01))
		{
			AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_LoadScanByNameDB", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************
Function Name  : GetInformationFromINI
Description    : used to read PRODUCTSETTINGS.ini file.
Author Name    : Adil Sheikh
SR_NO		   :
Date           : 8th july 2016
****************************************************************************/
bool CWardWizSettings::GetInformationFromINI()
{
	try
	{
		TCHAR  szActualINIPath[MAX_PATH] = { 0 };
		if (theApp.m_dwProductID)
		{
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%sVBSETTINGS\\%s", theApp.m_AppPath, L"PRODUCTSETTINGS.ini");
		}

		if (szActualINIPath == NULL)
		{
			AddLogEntry(L"### File not found : %s in GetInformationFromINI ", szActualINIPath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in CWardwizSettings::GetInformationFromINI ", 0, 0, true, SECONDLEVEL);
	}
  return true;
}

/***************************************************************************************************
*  Function Name  :  On_SelectFileFolder
*  Description    :  Used to select File/Folder to Unhide.
*  Author Name    :  Adil Sheikh
*  Date           :  7th Sept 2016
****************************************************************************************************/
json::value CWardWizSettings::On_SelectFileFolder()
{
	CString csReturn = L"";
	try
	{
		csReturn = OnBnClickedButtonBrowsefolder();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_SelectFileFolder", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)csReturn);
}

/***************************************************************************************************
*  Function Name  :  OnBnClickedButtonBrowsefolder
*  Description    :  Used to Handle Action for Browse Button
*  Author Name    :
*  Date           :  
****************************************************************************************************/
CString CWardWizSettings::OnBnClickedButtonBrowsefolder()
{
	CString csReturn = L"";
	try
	{
		TCHAR szPath[MAX_PATH] = { 0 };
		CString csTemp;
		BROWSEINFO        bi = { NULL, NULL, szPath, 0, BIF_BROWSEINCLUDEFILES, NULL, 0L, 0 };		//BIF_RETURNONLYFSDIRS
		LPITEMIDLIST      pIdl = NULL;
		LPITEMIDLIST  pRoot = NULL;
		bi.pidlRoot = pRoot;
		pIdl = SHBrowseForFolder(&bi);
		if (NULL != pIdl)
		{
			SHGetPathFromIDList(pIdl, szPath);
			size_t iLen = wcslen(szPath);
			if (iLen > 0)
			{
				csTemp = szPath;
				if (csTemp.Right(1) == L"\\")
				{
					csTemp = csTemp.Left(static_cast<int>(iLen)-1);
				}
				csReturn = csTemp;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::OnBnClickedButtonBrowsefolder", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/**********************************************************************************************************
*  Function Name  :	CheckForNetworkPath
*  Description    :	To check weather file is network file or not.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 9 Sept. 2016
**********************************************************************************************************/
json::value CWardWizSettings::CheckForNetworkPath(SCITER_VALUE svFilePath)
{
	try
	{
		const std::wstring  chFilePath = svFilePath.get(L"");
		if (PathIsNetworkPath((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::CheckForNetworkPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileOrFolderPath
*  Description    :	To check weather path is filepath or folderpath.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 23 Sept. 2016
**********************************************************************************************************/
json::value CWardWizSettings::CheckFileOrFolderPath(SCITER_VALUE svFileFolderPath)
{
	try
	{
		const std::wstring  chFilePath = svFileFolderPath.get(L"");
		if (PathIsDirectory((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::CheckFileOrFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	IsFileFolderPathExist
*  Description    :	To check weather file folder path is present or not.
*  Author Name    : Adil Sheikh
*  SR_NO		  :
*  Date           : 10 OCT. 2016
**********************************************************************************************************/
json::value CWardWizSettings::IsFileFolderPathExist(SCITER_VALUE svFileFolderPathExist)
{
	try
	{
		const std::wstring  chFilePath = svFileFolderPathExist.get(L"");
		if (!PathFileExists((LPTSTR)chFilePath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::IsFileFolderPathExist", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	ApplyAutorunSettings
*  Description    :	Function to apply autorun settings.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 09 Jan 2017
**********************************************************************************************************/
bool CWardWizSettings::ApplyAutorunSettings()
{
	bool bReturn = false;
	try
	{
		bool bAutorunBlocked = false;
		bool bUSBExecBlocked = false;
		bool bUSBWriteBlocked = false;

		CScannerLoad objScanDrv;

		//Get here registry setting for autorun block
		DWORD dwBlockAutorun = 0x00;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwBlockAutorun", dwBlockAutorun) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwBlockAutorun in CWardwizSettings::ApplyAutorunSettings KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		if (dwBlockAutorun == 0x01)
		{
			bAutorunBlocked = true;
		}

		//Get here registry setting for USB Exec Block
		DWORD dwUSBExecBlockAutorun = 0x01;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwExecuteBlock", dwUSBExecBlockAutorun) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwExecuteBlock in CWardwizSettings::ApplyAutorunSettings KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		//check with USB Exec Block setting
		if (dwUSBExecBlockAutorun == 0x01)
		{
			bUSBExecBlocked = true;
		}

		//Get here registry setting for USB write block
		DWORD dwWriteBlock = 0x01;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwWriteBlock", dwWriteBlock) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwWriteBlock in CWardwizSettings::ApplyAutorunSettings KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);;
		}

		//check with USB Exec Block setting
		if (dwWriteBlock == 0x01)
		{
			bUSBWriteBlocked = true;
		}

		if (objScanDrv.SetAutorunSettings(bAutorunBlocked, bUSBExecBlocked, bUSBWriteBlocked))
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::ApplyAutorunSettings", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : On_FunSelectLanguage
Description    : Function to modify language id in productsetting.ini file.
Author Name    : Amol J.
Date           : 26th May 2017
SR_NO		   :
***********************************************************************************************/
json::value CWardWizSettings::On_FunSelectLanguage(SCITER_VALUE svLangId)
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		sciter::string strLandID = svLangId.to_string();

		if (!PathFileExists(csIniFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetSelectedLanguage", csIniFilePath, 0, true, SECONDLEVEL);
			return 0;
		}
		
		WritePrivateProfileString(L"VBSETTINGS", L"LanguageID", strLandID.c_str(), csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_FunSelectLanguage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : On_FunGetLanguageID
Description    : Function to get language id from productsetting.ini file.
Author Name    : Amol J.
Date           : 26th May 2017
SR_NO		   :
***********************************************************************************************/
json::value CWardWizSettings::On_FunGetLanguageID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"LanguageID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUIDlg::On_FunGetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : GetDBPathforSched
Description    : Function to get path for Scheduled Scan
Author Name    : Jeena Mariam Saji
Date           : 04 October 2017
***********************************************************************************************/
json::value CWardWizSettings::GetDBPathforSched()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBSETTINGS.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::GetDBPathforSched", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : On_GetCurrentDay
Description    : Function to get current Day
Author Name    : Jeena Mariam Saji
Date           : 25 October 2017
***********************************************************************************************/
json::value CWardWizSettings::On_GetCurrentDay()
{
	TCHAR  szDayofWeek[MAX_PATH] = { 0 };
	try
	{
		int iDayWeek = CTime::GetCurrentTime().GetDayOfWeek();
		swprintf_s(szDayofWeek, L"%d", iDayWeek);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_GetCurrentDay", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szDayofWeek);
}

/***********************************************************************************************
Function Name  : On_ChangeSchedule
Description    : Function to check for Schedules within 2 min
Author Name    : Jeena Mariam Saji
Date           : 06 December 2017
***********************************************************************************************/
json::value CWardWizSettings::On_ChangeSchedule()
{
	try
	{
		if (!SendSettingsToService(SCHEDULED_SCAN_CHANGE, 0x00, 0x00))
		{
			AddLogEntry(L"### Failed to SendSettingsToService in SCHEDULED_SCAN_CHANGE", 0, 0, true, SECONDLEVEL);
		}

		TCHAR	m_szSpecificPath[512];
		int		iRow = 0;
		bool	bFlag = false;

		ZeroMemory(m_szSpecificPath, sizeof(m_szSpecificPath));
		GetEnvironmentVariable(L"TEMP", m_szSpecificPath, 511);

		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBALLREPORTS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		
		dbSQlite.Open();
		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from Wardwiz_TempFilesCleanerDetails;");

		Sleep(20);

		char szData[512] = { 0 };
		for (iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);

			if (qResult.GetFieldIsNull(0))
			{
				continue;
			}

			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(6), -1, NULL, 0);
			wchar_t *wstrDbData = new wchar_t[wchars_num];
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(6), -1, wstrDbData, wchars_num);

			if (_tcscmp(wstrDbData, m_szSpecificPath) != 0)
			{
				bFlag = true;
			}
			
		}
		dbSQlite.Close();

		if (bFlag)
			UpdateUserTempPathIntoDB();
		else if ( iRow == 0)
			InsertUserTempPathIntoDB();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_ChangeSchedule", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************************
*  Function Name  :	InsertUserTempPathIntoDB
*  Description    :	Invokes appropriate method from Database wrapper class and update data into SQLite tables.
*  Author Name    : Amol Jaware
*  Date           : 15 March 2018
*  SR_NO		  :
****************************************************************************************************************/
void CWardWizSettings::InsertUserTempPathIntoDB()
{
	try
	{

		CString		csInsertQuery;
		TCHAR		m_szSpecificPath[512];
		int			iScanID = 1;

		ZeroMemory(m_szSpecificPath, sizeof(m_szSpecificPath));
		GetEnvironmentVariable(L"TEMP", m_szSpecificPath, 511);

		csInsertQuery = _T("INSERT INTO Wardwiz_TempFilesCleanerDetails VALUES (null,");
		
		csInsertQuery.Format(_T("INSERT INTO Wardwiz_TempFilesCleanerDetails VALUES (null,%I64d,Datetime('now','localtime'),Datetime('now','localtime'),Date('now'),Date('now'),'%s','%s','%s');"), iScanID, m_szSpecificPath, L"", L"");
		
		CT2A ascii(csInsertQuery, CP_UTF8);

		INT64 iScanId = InsertDataToTable(ascii.m_psz);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::InsertUserTempPathIntoDB", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************************
*  Function Name  :	UpdateUserTempPathIntoDB
*  Description    :	Invokes appropriate method from Database wrapper class and update data into SQLite tables.
*  Author Name    : Amol Jaware
*  Date           : 15 March 2018
*  SR_NO		  :
****************************************************************************************************************/
void CWardWizSettings::UpdateUserTempPathIntoDB()
{
	try
	{
		TCHAR		m_szSpecificPath[512];

		ZeroMemory(m_szSpecificPath, sizeof(m_szSpecificPath));
		GetEnvironmentVariable(L"TEMP", m_szSpecificPath, 511);

		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBALLREPORTS.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		dbSQlite.Open();

		CString csInsertQuery = _T("UPDATE Wardwiz_TempFilesCleanerDetails VALUES (null,");
		csInsertQuery.Format(_T("UPDATE Wardwiz_TempFilesCleanerDetails SET db_VirusName = %s WHERE ID = 1;"), m_szSpecificPath);
		CT2A asciiSession(csInsertQuery, CP_UTF8);

		INT64 iScanSessionId = InsertDataToTable(asciiSession.m_psz);

		dbSQlite.Close();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::UpdateUserTempPathIntoDB", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into SQLite tables.
*  Author Name    : Amol Jaware
*  Date           : 15 March 2018
*  SR_NO		  :
****************************************************************************************************************/
INT64 CWardWizSettings::InsertDataToTable(const char* szQuery)
{
	AddLogEntry(L"InsertDataToTable in VibraniumAutoScnDlg- TempFileCleaner entered", 0, 0, true, ZEROLEVEL);
	try
	{
		char* g_strDatabaseFilePath = ".\\VBALLREPORTS.DB";
		CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = objSqlDb.GetLastRowId();
		objSqlDb.Close();

		return iLastRowId;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::InsertDataToTable()", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***********************************************************************************************
Function Name  : On_CheckForMonthlyScan
Description    : Function to get current Day
Author Name    : Jeena Mariam Saji
Date           : 25 October 2017
***********************************************************************************************/
json::value CWardWizSettings::On_CheckForMonthlyScan(SCITER_VALUE svScanYear, SCITER_VALUE svScanMonth, SCITER_VALUE svScanDay, SCITER_VALUE svScanTime)
{
	int iSpan = 0;
	try
	{
		int intYear = 0;
		int intMonth = 0;
		int intDay = 0;

		sciter::string strYear = svScanYear.to_string();
		sciter::string strMonth = svScanMonth.to_string();
		sciter::string strDay = svScanDay.to_string();
		sciter::string strScanTime = svScanTime.to_string();

		CString csYear;
		csYear = strYear.c_str();

		CString csMonth;
		csMonth = strMonth.c_str();

		CString csDay;
		csDay = strDay.c_str();

		CString csScanType = strScanTime.c_str();

		if (csYear.GetLength() != 0)
		{
			intYear = _ttoi(csYear);
		}
		if (csMonth.GetLength() != 0)
		{
			intMonth = _ttoi(csMonth);
		}
		if (csDay.GetLength() != 0)
		{
			intDay = _ttoi(csDay);
		}
		
		CTime cTime = CTime::GetCurrentTime();
		int iDay = cTime.GetDay();
		int iMonth = cTime.GetMonth();
		int iYear = cTime.GetYear();

		CTime Time_DBEntry(intYear, intMonth, intDay, 0, 0, 0);

		CTimeSpan Time_Diff = Time_DBEntry - cTime;

		int iSpan = static_cast<int>(Time_Diff.GetDays());

		if (iSpan == 0)
		{
			if((intYear == iYear) && (intMonth == iMonth) && (intDay == iDay))
			{
				if (csScanType == L"true")
				{
					if (intMonth > 0 && intMonth < 12)
					{
						intMonth++;
					}
					else if (intMonth == 12)
					{
						intYear++;
						intMonth = 1;
					}
					CTime Time_OrgEntry(intYear, intMonth, intDay, 0, 0, 0);
					CTimeSpan Time_Diff = Time_OrgEntry - Time_DBEntry;
					iSpan = static_cast<int>(Time_Diff.GetDays());
				}
				return iSpan;
			}
			else
			{
				return 1;
			}
		}
		else if (iSpan < 0)
		{
			iSpan = 0 - iSpan;
			if (csScanType == L"true")
			{
				if (intMonth > 0 && intMonth < 12)
				{
					intMonth++;
				}
				else if (intMonth == 12)
				{
					intYear++;
					intMonth = 1;
				}
			}
			CTime Time_OrgEntry(intYear, intMonth, intDay, 0, 0, 0);
			CTimeSpan Time_Diff = Time_DBEntry - Time_OrgEntry;
			iSpan = static_cast<int>(Time_Diff.GetDays());
			return iSpan;
		}
		else if (iSpan > 0)
		{
			return iSpan + 1;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_CheckForMonthlyScan", 0, 0, true, SECONDLEVEL);
	}
	return iSpan;
}

/***********************************************************************************************
Function Name  : CheckForMonth
Description    : Function to get current Day
Author Name    : Jeena Mariam Saji
Date           : 25 October 2017
***********************************************************************************************/
int CWardWizSettings::CheckForMonth(int intMonth, int intYear)
{
	int iDiffDays = 0;
	try
	{
		if (intMonth == 4 || intMonth == 6 || intMonth == 9 || intMonth == 11)
		{
			iDiffDays = 30;
		}
		else if (intMonth == 2)
		{
			bool isLeapYear = (intYear % 4 == 0 && intYear % 100 != 0) || (intYear % 400 == 0);
			if (isLeapYear)
			{
				iDiffDays = 29;
			}
			else
			{
				iDiffDays = 28;
			}
		}
		else
		{
			iDiffDays = 31;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::CheckForMonth", 0, 0, true, SECONDLEVEL);
	}
	return iDiffDays;
}

/***********************************************************************************************
Function Name  : SetAutoQurantineOptionInReg
Description    : Function which sets AutoQuarentine option in registry
Author Name    : Ramkrushna Shelke
Date           : 08 Nov 2017
***********************************************************************************************/
bool CWardWizSettings::SetAutoQurantineOptionInReg(int iQurOption)
{
	bool bReturn = false;
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		if (!PathFileExists(csIniFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizSettings::SetAutoQurantineOptionInReg", csIniFilePath, 0, true, SECONDLEVEL);
			return 0;
		}

		CString csQurOption;
		csQurOption.Format(L"%d", iQurOption);

		WritePrivateProfileString(L"VBSETTINGS", L"AutoQuarOption", csQurOption, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::SetAutoQurantineOptionInReg", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : CallTrayForWidget
Description    : Function which calls tray for widget.
Author Name    : Amol J.
Date           : 10 Nov 2017
***********************************************************************************************/
bool CWardWizSettings::CallTrayForWidget(bool bFlagValue)
{
	try
	{
		CITinRegWrapper objReg;
		if (bFlagValue == 0x00)
		{
			if (!WriteRegistryEntryOfSettingsTab(theApp.m_csRegKeyPath, L"dwWidgetsUIState", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwWidgetsUIState in CWardwizSettings::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
			}
			
			if (!SendData2Tray(RELOAD_WIDGETS_UI, WIDGET_HIDE_UI))
			{
				AddLogEntry(L"### Exception in CWardwizSettings::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
			}
		}
		else if (bFlagValue == 0x01)
		{
			if (!WriteRegistryEntryOfSettingsTab(theApp.m_csRegKeyPath, L"dwWidgetsUIState", bFlagValue))
			{
				AddLogEntry(L"### Error in Setting Registry dwWidgetsUIState in CWardwizSettings::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
			}

			if (!SendData2Tray(RELOAD_WIDGETS_UI, WIDGET_SHOW_UI))
			{
				AddLogEntry(L"### Exception in CWardwizSettings::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
			}
		}
		else
		{
			AddLogEntry(L"### Unhandled case in CWardwizSettings::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::CallTrayForWidget", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SendData2Tray
*  Description    : Function which send message data to Tray application.
*  Author Name    : Amol Jaware
*  Date           : 10th Nov,2017
****************************************************************************************************/
bool CWardWizSettings::SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwValue;

		CISpyCommunicator objCom(TRAY_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CWardwizSettings::SendData2Tray", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CWardwizSettings::SendData2Tray", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::SendData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : On_funFromExclude
*  Description    : Function which gets called once any files/folder added in exclude dialog.
*  Author Name    : Ram Shelke
*  Date           : 13th Nov,2017
****************************************************************************************************/
json::value CWardWizSettings::On_funFromExclude()
{
	try
	{
		CExcludeFilesFolders objExcludeDB;
		bool bRet = objExcludeDB.SaveEntriesInDB();
		//Send here the settings to service.
		if (!SendSettingsToService(APPLYPRODUCTSETTINGS, RELOAD_EXCLUDE_DB, 0x00))
		{
			AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
		}
		return bRet ? 1 : 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_funFromExclude", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_funFromExclude4FileExt
*  Description    : Function which gets called once any file extensions added in exclude dialog.
*  Author Name    : Amol Jaware
*  Date           : 09 Oct 2018
****************************************************************************************************/
json::value CWardWizSettings::On_funFromExclude4FileExt()
{
	try
	{
		CExcludeFilesFolders objExcludeDB;
		bool bRet = objExcludeDB.SaveEntriesInDB();
		//Send here the settings to service.
		if (!SendSettingsToService(APPLYPRODUCTSETTINGS, RELOAD_EXCLUDE_DB, 0x00))
		{
			AddLogEntry(L"### Failed to SendSettingsToService in AUTO_QUARNTINE", 0, 0, true, SECONDLEVEL);
		}
		return bRet ? 1 : 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_funFromExclude4FileExt", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_funFromScanByName
*  Description    : Function which gets called once any file names added in scan by name dialog.
*  Author Name    : Amol Jaware
*  Date           : 09 Oct 2018
****************************************************************************************************/
json::value CWardWizSettings::On_funFromScanByName()
{
	try
	{
		CScanByName objScanByName;
		bool bRet = objScanByName.SaveEntriesInDB();
		return bRet ? 1 : 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_funFromScanByName", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : On_CheckforTimeFormat
Description    : Function to check for Time format whether 12/24 Hour format
Author Name    : Jeena Mariam Saji
Date           : 01 December 2017
***********************************************************************************************/
json::value CWardWizSettings::On_CheckforTimeFormat()
{
	try
	{
		if (IS12HoursFormat())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_CheckforTimeFormat", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : IS12HoursFormat
Description    : Function to check for Time format whether 12/24 Hour format
Author Name    : Jeena Mariam Saji
Date           : 01 December 2017
***********************************************************************************************/
bool CWardWizSettings::IS12HoursFormat()
{
	bool bReturn = false;
	try
	{
		SYSTEMTIME stBuffer = { 0 };
		TCHAR	   strT[128] = { 0 };
		TCHAR	   strData[128] = { 0 };

		stBuffer.wHour = 13;
		int iRet = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOTIMEMARKER, &stBuffer, NULL, strT, 128);
		if (iRet == 0x00)
			return false;

		CString csT(strT);
		CString csHour = csT.Left(csT.Find(':', 0) + 1);
		if (wcslen(csHour.GetBuffer()) == 0)
			return false;

		int iHour = _wtoi(csHour);

		if (iHour > 12)
		{
			bReturn = false;
		}
		else
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::IS12HoursFormat", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : GetDBPath
Description    : This function will get Database Path
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 01 February 2018
***********************************************************************************************/
json::value CWardWizSettings::GetDBPathforEPS()
{
	TCHAR  szActualIPath[MAX_PATH] = { 0 };
	try
	{
		swprintf_s(szActualIPath, L"%s%s", theApp.m_AppPath, L"VBFEATURESLOCK.DB");
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::GetDBPathforSched", 0, 0, true, SECONDLEVEL);
	}
	return json::value((SCITER_STRING)szActualIPath);
}

/***********************************************************************************************
Function Name  : IsFilePath
Description    : To check path is file or not.
SR.NO		   :
Author Name    : Amol Jaware
Date           : 11 Oct 2018
***********************************************************************************************/
json::value CWardWizSettings::IsFilePath(SCITER_VALUE svPath)
{
	try
	{
		const std::wstring csPath = svPath.get(L"");
		if (PathIsDirectory((LPTSTR)csPath.c_str()))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::IsFilePath", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : ReadUpdateServerName
Description    : This function will read Server Name from ProductSettings.ini file
SR.NO		   :
Author Name    : Jeena Mariam Saji
Date           : 08 October 2018
***********************************************************************************************/
json::value CWardWizSettings::ReadUpdateServerName()
{
	TCHAR	szUpdateServerName[255] = { 0 };
	try
	{
		TCHAR			szActualINIPath[255] = { 0 };
		TCHAR			szServerName[255] = { 0 };
		TCHAR			szUpdateServerCount[16] = { 0 };
		CString			csIniFilePath;
		int				iCount;
		CString			csCountIndex;
		CString			csTempVal;
		CString			csServerList[10];

		csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\UPDATESERVERS.ini";
		GetPrivateProfileString(L"UPDATESERVERS", L"Count", L"", szUpdateServerCount, 511, csIniFilePath);
		iCount = _ttoi(szUpdateServerCount);
		
		if (!PathFileExists(csIniFilePath) || iCount == 0)
		{
			WritePrivateProfileString(L"UPDATESERVERS", L"Count", L"1", csIniFilePath);
			WritePrivateProfileString(L"UPDATESERVERS", L"1", L"www.vibranium.co.in", csIniFilePath);
		}
		GetPrivateProfileString(L"UPDATESERVERS", L"Count", L"", szUpdateServerCount, 511, csIniFilePath);
		iCount = _ttoi(szUpdateServerCount);
		for (int i = 1; i <= iCount; i++)
		{
			csCountIndex.Format(L"%d", i);
			GetPrivateProfileString(L"UPDATESERVERS", csCountIndex, L"", szServerName, 511, csIniFilePath);
			csTempVal = CString(szServerName);
			Sleep(10);
			SendServerNametoUI(csTempVal);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::ReadUpdateServerName", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : SendServerNametoUI
Description    : This function will send Server Name to UI
Author Name    : Jeena Mariam Saji
Date           : 17 October 2018
***********************************************************************************************/
void CWardWizSettings::SendServerNametoUI(CString csServerName)
{
	try
	{
		sciter::value map;
		map.set_item("one", sciter::string(csServerName));
		map.set_item("two", L"");
		map.set_item("three", L"");
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = ADD_SERVER_LIST_EVENT;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::SendServerNametoUI", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : WriteServerNameinINI
Description    : This function will write Server Name in ProductSettings.ini file
Author Name    : Jeena Mariam Saji
Date           : 08 October 2018
***********************************************************************************************/
json::value CWardWizSettings::WriteServerNameinINI(SCITER_VALUE svServerNameVal)
{
	try
	{
		CString csServerNameVal = svServerNameVal.get(L"").c_str();
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"UPDATESERVERS" + L"\\ProductSettings.ini";

		if (!PathFileExists(csIniFilePath))
		{
			AddLogEntry(L"### %s file not found, In CWardwizSettings::WriteServerNameinINI", csIniFilePath, 0, true, SECONDLEVEL);
			return 0;
		}
		WritePrivateProfileString(L"VBSETTINGS", L"UpdateServer", csServerNameVal, csIniFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::WriteServerNameinINI", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : OnResetPriority
Description    : This function will reset Server Name priority in ProductSettings.ini file
Author Name    : Jeena Mariam Saji
Date           : 08 October 2018
***********************************************************************************************/
json::value CWardWizSettings::OnResetPriority(SCITER_VALUE svServerNameArray)
{
	try
	{
		CString csINIFilePath;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		HANDLE hFile = NULL;
		bool bIsArray = false;
		svServerNameArray.isolate();
		bIsArray = svServerNameArray.is_array();
		if (!bIsArray)
		{
			return false;
		}
		m_svServerNameArray = svServerNameArray;
		int iCurrentValue, count = 0;
		count = m_svServerNameArray.length();
		CString csServerName;
		CString csCountVal;
		CString csCount;
		GetModulePath(szModulePath, MAX_PATH);
		csINIFilePath = szModulePath;
		csINIFilePath = csINIFilePath + L"\\VBSETTINGS\\UPDATESERVERS.INI";
		csCount.Format(L"%d", count);
		DeleteFile(csINIFilePath);

		WritePrivateProfileString(L"UPDATESERVERS", L"Count", csCount, csINIFilePath);
		for (iCurrentValue = 0; iCurrentValue < count; iCurrentValue++)
		{
			csCountVal.Format(L"%d", iCurrentValue + 1);
			const SCITER_VALUE EachEntryinArray = m_svServerNameArray[iCurrentValue];
			const std::wstring chThreatPath = EachEntryinArray[L"ServerName"].get(L"");
			csServerName = chThreatPath.c_str();
			WritePrivateProfileString(L"UPDATESERVERS", csCountVal, csServerName, csINIFilePath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::OnResetPriority", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : CheckForZeroKB
Description    : This function will chek if file size is of 0KB
Author Name    : Jeena Mariam Saji
Date           : 25 October 2018
***********************************************************************************************/
json::value CWardWizSettings::CheckForZeroKB(SCITER_VALUE svScanByFileName)
{
	try
	{
		DWORD dwFileSize = 0x00;
		CString csFileNameVal = svScanByFileName.get(L"").c_str();
		HANDLE hFile = CreateFile(csFileNameVal, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			dwFileSize = GetFileSize(hFile, NULL);
		}
		if (dwFileSize == 0x00)
		{
			CloseHandle(hFile);
			return false;
		}
		else
		{
			CloseHandle(hFile);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::CheckForZeroKB", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : OnSetRegVal4UsbBlock
Description    : This function will set registry value on toggler button
Author Name    : Akshay Patil
Date           : 25 October 2018
***********************************************************************************************/
json::value CWardWizSettings::OnSetRegVal4UsbBlock(SCITER_VALUE svRegValue)
{
	try
	{
		bool bFlagValue = svRegValue.get(false);
		LPCTSTR path = _T("SYSTEM\\CurrentControlSet\\services\\USBSTOR");
		
		DWORD dwRegKey = 0x03;
		if (!bFlagValue)
		{
			dwRegKey = 0x03;
		}
		else
		{	
			dwRegKey = 0x04;
		}

		if (!WriteRegistryEntryOfSettingsTab(path, L"Start", dwRegKey))
		{
			AddLogEntry(L"### Error in Setting Registry dwFirewallEnableState in CVibraniumFirewall::OnSetRegVal", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::OnSetRegVal4UsbBlock", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : ReadCurrentUSBRegValue
Description    : Read registry entry
Author Name    : Akshay Patil
Date           : 25 October 2018
***********************************************************************************************/
json::value CWardWizSettings::ReadCurrentUSBRegValue()
{
	try
	{
		bool bReturn = false;
		DWORD dwStartType = 0x00;
		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\USBSTOR", L"Start", dwStartType) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for Start in CWardwizSettings::ReadCurrentUSBRegValue", 0, 0, true, SECONDLEVEL);;
		}

		if (dwStartType == 0x04)
			return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::ReadCurrentUSBRegValue", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : CheckIsWrdWizFile
*  Description    : To check path is WardWiz own file or not.
*  Author Name    : Akshay Patil
*  Date           :	30 October 2018
****************************************************************************************************/
json::value CWardWizSettings::CheckIsWrdWizFile(SCITER_VALUE svScanByFileName)
{
	try
	{
		CString csFileNameVal = svScanByFileName.get(L"").c_str();
		CString csWardWizPath = GetWardWizPathFromRegistry();

		if (_tcscmp(csWardWizPath, csFileNameVal.Left(csWardWizPath.GetLength())) == 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::CheckIsVibraniumFile", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : OnClickSetPage
*  Description    : This function is used to set the setting page name.
*  Author Name    : Akshay Patil
*  Date           :	30 Oct 2018
****************************************************************************************************/
json::value CWardWizSettings::OnClickSetPage()
{
	try
	{
		theApp.m_csPageName = L"#SETTINGS";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::OnClickSetPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnClickSetOtherPage
*  Description    : This function is used to set the none page name.
*  Author Name    : Akshay Patil
*  Date           :	31 Oct 2018
****************************************************************************************************/
json::value CWardWizSettings::OnClickSetOtherPage()
{
	try
	{
		theApp.m_csPageName = L"#NONE";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::OnClickSetOtherPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : OnSetRegvalforProxy
Description    : This function will set registry value for proxy setting
Author Name    : Kunal Waghmare
Date           : 23 Jan 2019
***********************************************************************************************/
json::value CWardWizSettings::OnSetRegvalforProxy(SCITER_VALUE svRegValue)
{
	try
	{
		bool bFlagValue = svRegValue.get(false);
		LPCTSTR path = theApp.m_csRegKeyPath.GetBuffer();

		DWORD dwRegKey = 0x00;
		if (!bFlagValue)
		{
			dwRegKey = 0x00;
		}
		else
		{
			dwRegKey = 0x02;
		}

		if (!WriteRegistryEntryOfSettingsTab(path, L"dwProxySett", dwRegKey))
		{
			AddLogEntry(L"### Error in Setting Registry dwProxySett in CWardwizSettings::OnSetRegvalforProxy", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::OnSetRegvalforProxy", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***********************************************************************************************
Function Name  : OnGetRegvalforProxy
Description    : Read registry entry for proxy setting
Author Name    : Kunal Waghmare
Date           : 23 Jan 2019
***********************************************************************************************/
json::value CWardWizSettings::OnGetRegvalforProxy()
{
	try
	{
		bool bReturn = false;
		DWORD dwProxySet = 0x00;
		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwProxySett", dwProxySet) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwProxySett in CWardwizSettings::OnGetRegvalforProxy", 0, 0, true, SECONDLEVEL);;
		}

		if (dwProxySet == 0x02)
			return json::value(0x02);
		else
			return json::value(0x00);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::OnGetRegvalforProxy", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0x00);
}

/***************************************************************************************************
*  Function Name  : UpdateWidgetUI
*  Description    : This function is used to update widget UI
*  Author Name    : Kunal Waghmare
*  Date           :	31 Jan 2019
****************************************************************************************************/
bool CWardWizSettings::UpdateWidgetUI()
{
	try
	{
		/*DWORD dwRegVal = 0x00;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwWidgetsUIState", dwRegVal) != 0x00)
		{
			AddLogEntry(L"### Failed to read DWORD Value dwWidgetsUIState in CWardWizSettings::UpdateWidgetUI", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (dwRegVal == 0x01)
		{*/
			if (!SendData2Tray(RELOAD_WIDGETS_UI, WIDGET_HIDE_UI))
			{
				AddLogEntry(L"### Exception in CWardwizSettings::UpdateWidgetUI", 0, 0, true, SECONDLEVEL);
			}
			Sleep(200);
			if (!SendData2Tray(RELOAD_WIDGETS_UI, WIDGET_SHOW_UI))
			{
				AddLogEntry(L"### Exception in CWardwizSettings::UpdateWidgetUI", 0, 0, true, SECONDLEVEL);
			}
		//}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::UpdateWidgetUI", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : OnClickSetOtherPage
*  Description    : This function is used to change language ID
*  Author Name    : Jeena Mariam Saji
*  Date           :	22 Nov 2018
****************************************************************************************************/
json::value CWardWizSettings::ChangeLanguageID(SCITER_VALUE svLangValue, SCITER_VALUE svBoolVal)
{
	try
	{
		int iLangValue = static_cast<int>(svLangValue.d);
		bool bFlagValue = svBoolVal.get(false);
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		if (iLangValue == 0)
		{
			WritePrivateProfileString(L"VBSETTINGS", L"LanguageID", L"0", csIniFilePath);
		}
		else if (iLangValue == 2)
		{
			WritePrivateProfileString(L"VBSETTINGS", L"LanguageID", L"2", csIniFilePath);
		}
		if (bFlagValue == 1)
			UpdateWidgetUI();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::ChangeLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ChangeTheme
*  Description    : This function is used to change Theme ID
*  Author Name    : Jeena Mariam Saji
*  Date           :	04 Dec 2018
*  Updated By     : Kunal Waghmare
*  Date           :	07 Feb 2019
****************************************************************************************************/
json::value CWardWizSettings::On_ChangeTheme(SCITER_VALUE svThemeValue, SCITER_VALUE svBoolVal)
{
	try
	{
		CString csThemeVal = svThemeValue.get(L"").c_str();
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		bool bFlagValue = svBoolVal.get(false);
		if (csThemeVal == L"defaultTheme")
		{
			WritePrivateProfileString(L"VBSETTINGS", L"ThemeID", L"1", csIniFilePath);
		}
		else if (csThemeVal == L"darkTheme")
		{
			WritePrivateProfileString(L"VBSETTINGS", L"ThemeID", L"2", csIniFilePath);
		}
		else if (csThemeVal == L"crystalTheme")
		{
			WritePrivateProfileString(L"VBSETTINGS", L"ThemeID", L"3", csIniFilePath);
		}
		//if (bFlagValue == 1)  //Commented for now, as widget does not change its theme.
			//UpdateWidgetUI();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSettings::On_ChangeTheme", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}