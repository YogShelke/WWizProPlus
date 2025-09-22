/****************************************************
*  Program Name: CWardwizSettingsUpdater.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 7th May 2016
*  Version No: 2.0.0.1
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once

#include "iTINRegWrapper.h"
#include "ISpyCommunicator.h"

class CWardwizSettingsUpdater
{
public:
	CWardwizSettingsUpdater();
	~CWardwizSettingsUpdater();

public:
	CString			m_Key;
	CString			Section = L"WRDSETTINGS";
	CITinRegWrapper	 m_objReg;
	CString			m_csRegKeyPath;
	CString			m_AppPath;
	CString			INIPath;// = (theApp.m_AppPath) + (L"WRDSETTINGS\\PRODUCTSETTINGS.INI");

public:
	//void ReadCurrentSettingsRegValues(SCITER_VALUE scFunSetCurrentSettingsValueCB);
	//void On_ClickSettingsButton(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue);
	BOOL WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);//Set the Registry Key Value for Menu Items using Service
	bool ApplyDefaultSettings();

	enum ENUM_WARDWIZ_SETTINGS
	{
		TRAY_NOTIFICATION = 1,
		DEL_OLD_REPORTS,
		SHOW_TOOLTIP,
		SCAN_AT_STARTUP,
		ENABLE_SOUND,
		AUTO_LIVE_UPDATE,
		DO_CACHING,
		PREP_CACHE_IN_BG,
		USB_PROMPT,
		AUTO_QUARNTINE,
		HEURISTIC_SCAN,
		EMAIL_SCAN
	}WARDWIZ_SETTINGS;

	////"Settings" related functions
	//json::value On_ClickChangeSettings(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue);
	bool UpdateWardwizSettings();
	CString GetAppPath();
	//json::value ReadCurrentSettingsValues(SCITER_VALUE scFunSetCurrentSettingsValueCB);
	//json::value On_ClickApplyDefaultSettings();
	  void SetDelRepDays(int svDays);
	  void SetScanStartup(int scType);
	//json::value GetDeleteDaysRegVal();
	//json::value GetStartupScanType();
	//json::value GetDBPath();
	//json::value On_AddExcludeFileFolder();
	//json::value On_SelectFileFolder();
	//json::value CheckForNetworkPath(SCITER_VALUE svFilePath);
	//json::value CheckFileOrFolderPath(SCITER_VALUE svFileFolderPath);

	bool SendSettingsToService(int iMessageInfo, DWORD dwValue, DWORD dwSecondValue);
	bool GetInformationFromINI();

public:
	CISpyCommunicator m_objComService;
};
