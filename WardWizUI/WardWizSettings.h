/****************************************************
*  Program Name: CWardWizSettings.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 7th May 2016
*  Version No: 2.0.0.1
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once

#include "WardWizUI.h"
#include "iTINRegWrapper.h"
#include "ISpyCommunicator.h"
#include "WardWizDatabaseInterface.h"

class CWardWizSettings : sciter::event_handler,
	sciter::behavior_factory
{
	HELEMENT self;

public:
	CWardWizSettings();
	~CWardWizSettings();

	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}

public:
	CString	m_Key;
	CString Section = L"VBSETTINGS";
	CString INIPath = (theApp.m_AppPath)+(L"VBSETTINGS\\PRODUCTSETTINGS.INI");
	CITinRegWrapper	 m_objReg;

public:
	void ReadCurrentSettingsRegValues(SCITER_VALUE scFunSetCurrentSettingsValueCB);
	void On_ClickSettingsButton(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue);
	BOOL WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey, CString strKey, DWORD dwChangeValue);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = false);//Set the Registry Key Value for Menu Items using Service
	bool ApplyDefaultSettings();
	bool CallTrayForWidget(bool bFlagValue);
	bool SendData2Tray(DWORD dwMessage, DWORD dwValue, bool bWait = false);
	bool UpdateWidgetUI();
	void InsertUserTempPathIntoDB();
	void UpdateUserTempPathIntoDB();
	INT64 InsertDataToTable(const char* szQuery);
	SCITER_VALUE m_svGetCurrentDay;

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
		 EMAIL_SCAN,
		 AUTO_PRODUCUT_UPDATE,
		 BLOCK_AUTORUN,
		 EXECUTE_BLOCK,
		 WRITE_BLOCK,
		 SCHED_SCAN,
		 WIDGET_UI
	}WARDWIZ_SETTINGS;

	BEGIN_FUNCTION_MAP
		//"Settings" related function
		FUNCTION_3("OnClickChangeSettings", On_ClickChangeSettings) // On_ClickChangeSettings()
		FUNCTION_1("ReadCurrentSettingsValues", ReadCurrentSettingsValues) // ReadCurrentSettingsValues()
		FUNCTION_0("OnClickApplyDefaultSettings", On_ClickApplyDefaultSettings) // On_ClickApplyDefaultSettings()
		FUNCTION_1("SetDelRepDays", SetDelRepDays)
		FUNCTION_1("SetScanStartup", SetScanStartup)
		FUNCTION_0("GetDelRepDays", GetDeleteDaysRegVal)
		FUNCTION_0("GetStartupScan", GetStartupScanType)
		FUNCTION_0("GetDBPath", GetDBPath)
		FUNCTION_0("OnAddExcludeFileFolder", On_AddExcludeFileFolder)
		FUNCTION_0("AddExcludeFileExtension", On_AddExcludeFileExtension)
		FUNCTION_0("SelectFileFolder", On_SelectFileFolder)
		FUNCTION_1("CheckIsNetworkPath", CheckForNetworkPath)
		FUNCTION_1("CheckFileOrFolderPath", CheckFileOrFolderPath)
		FUNCTION_1("CheckFileFolderPathExist", IsFileFolderPathExist)
		FUNCTION_0("GetDBPathforSched", GetDBPathforSched)
		FUNCTION_0("GetCurrentDay", On_GetCurrentDay)
		FUNCTION_4("CheckForMonthlyScan", On_CheckForMonthlyScan)
		FUNCTION_0("CheckforTimeFormat", On_CheckforTimeFormat)
		FUNCTION_0("OnChangeSchedule", On_ChangeSchedule)
		FUNCTION_0("funFromExclude", On_funFromExclude)
		FUNCTION_0("funFromExclude4FileExt", On_funFromExclude4FileExt)
		FUNCTION_0("funFromScanByName", On_funFromScanByName)
		FUNCTION_0("GetScanNameDBPath", GetScanNameDBPath)
		FUNCTION_0("OnLoadScanByNameDB", On_LoadScanByNameDB)
		FUNCTION_1("IsFilePath", IsFilePath)
		FUNCTION_0("ReadUpdateServerName", ReadUpdateServerName)
		FUNCTION_1("WriteServerNameinINI", WriteServerNameinINI)
		FUNCTION_1("OnResetPriority", OnResetPriority)
		FUNCTION_1("CheckForZeroKB", CheckForZeroKB)
		FUNCTION_1("OnClickChangeUsbBlock", OnSetRegVal4UsbBlock)
		FUNCTION_0("ReadCurrentUSBRegValue", ReadCurrentUSBRegValue);
		FUNCTION_1("CheckIsWrdWizFile", CheckIsWrdWizFile)
		FUNCTION_0("SetPage", OnClickSetPage)
		FUNCTION_0("SetOtherPage", OnClickSetOtherPage)
		FUNCTION_1("SetRegvalforProxy", OnSetRegvalforProxy)
		FUNCTION_0("GetRegvalforProxy", OnGetRegvalforProxy)
		FUNCTION_2("OnChangeTheme", On_ChangeTheme)

		//for select language
		FUNCTION_1("FunSelectLanguage", On_FunSelectLanguage);
		FUNCTION_0("FunGetLanguageID", On_FunGetLanguageID);
		FUNCTION_0("GetDBPathforEPS", GetDBPathforEPS);
		FUNCTION_2("ChangeLanguageID", ChangeLanguageID);

	END_FUNCTION_MAP

	//"Settings" related functions
    json::value On_ClickChangeSettings(SCITER_VALUE svIndexValue, SCITER_VALUE svBoolSettingsTrayNotify, SCITER_VALUE svintSettingsRegValue);
	json::value ReadCurrentSettingsValues(SCITER_VALUE scFunSetCurrentSettingsValueCB);
	json::value On_ClickApplyDefaultSettings();
	json::value SetDelRepDays(SCITER_VALUE svDays);
	json::value SetScanStartup(SCITER_VALUE scType);
	json::value GetDeleteDaysRegVal();
	json::value GetStartupScanType();
	json::value GetDBPath();
	json::value On_AddExcludeFileFolder();
	json::value On_AddExcludeFileExtension();
	json::value On_SelectFileFolder();
	json::value CheckForNetworkPath(SCITER_VALUE svFilePath);
	json::value CheckFileOrFolderPath(SCITER_VALUE svFileFolderPath);
	json::value IsFileFolderPathExist(SCITER_VALUE svFileFolderPathExist);
	json::value GetDBPathforSched();
	json::value On_GetCurrentDay();
	json::value On_CheckForMonthlyScan(SCITER_VALUE svScanYear, SCITER_VALUE svScanMonth, SCITER_VALUE svScanDay, SCITER_VALUE svScanTime);
	json::value On_CheckforTimeFormat();
	json::value On_ChangeSchedule(); 
	json::value GetScanNameDBPath();
	json::value On_LoadScanByNameDB();
	json::value IsFilePath(SCITER_VALUE svPath);
	json::value ReadUpdateServerName();
	json::value WriteServerNameinINI(SCITER_VALUE svServerNameVal);
	json::value OnResetPriority(SCITER_VALUE svServerNameArray);
	json::value CheckForZeroKB(SCITER_VALUE svScanByFileName);
	json::value CheckIsWrdWizFile(SCITER_VALUE svScanByFileName);
	json::value OnClickSetPage();
	json::value OnClickSetOtherPage();

	json::value On_FunSelectLanguage(SCITER_VALUE svLangId);
	json::value On_FunGetLanguageID();
	json::value On_funFromExclude();
	json::value On_funFromExclude4FileExt();
	json::value On_funFromScanByName();
	json::value GetDBPathforEPS();
	json::value OnSetRegVal4UsbBlock(SCITER_VALUE svRegValue);
	json::value ReadCurrentUSBRegValue();
	json::value OnSetRegvalforProxy(SCITER_VALUE svRegValue);
	json::value OnGetRegvalforProxy();
	json::value ChangeLanguageID(SCITER_VALUE svLangValue, SCITER_VALUE svBoolVal);
	json::value On_ChangeTheme(SCITER_VALUE svThemeValue, SCITER_VALUE svBoolVal);

	bool SendSettingsToService(int iMessageInfo, DWORD dwValue, DWORD dwSecondValue);
	bool GetInformationFromINI();
	CString OnBnClickedButtonBrowsefolder();
	void CreateRegistryEntryForExternalDevicesSettings(CString strKey, int iDefaultValue);
	bool ApplyAutorunSettings();
	int CheckForMonth(int intMonth, int intYear);
	bool SetAutoQurantineOptionInReg(int iRegistryValue);
	bool IS12HoursFormat();
public:
	CISpyCommunicator			m_objComService;
	SCITER_VALUE				m_svServerNameArray;
	void SendServerNametoUI(CString csServerName);
};
