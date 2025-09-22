/**********************************************************************************************************              	  Program Name          : WardwizLangManager.cpp
	  Description           : Wrapper class to handle languages, and provide specific string from ini files.
							  
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 22nd Apr 2014
	  Version No            : 1.0.0.4
	  Special Logic Used    : Need to maintain last seletected language DWORD which is stopred in ini file
							  when user launches UI, it shows string from selected languages and shows on UI.
	  
	  Modification Log			 :      
	  1. Ramkrushna (22-04-2014) : Created Wrapper class for language support.
***********************************************************************************************************/
#include "StdAfx.h"
#include <Shlwapi.h>
#include "WardwizLangManager.h"
#include "WWizSettingsWrapper.h"

/***************************************************************************
  Function Name  : CWardwizLangManager
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  Date           : 22nd Apr 2014
  SR_NO			 : WRDWIZCOMMON_0262
****************************************************************************/
CWardwizLangManager::CWardwizLangManager(void):
	m_csiniFilePath(L"")
{

}

/***************************************************************************
  Function Name  : CWardwizLangManager
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  Date           : 22nd Apr 2014
  SR_NO			 : WRDWIZCOMMON_0263
****************************************************************************/
CWardwizLangManager::~CWardwizLangManager(void)
{
}

/***************************************************************************
  Function Name  : InitializeVariables
  Description    : Function which initializes the variables.
  Author Name    : Ramkrushna Shelke
  Date           : 22nd Apr 2014
  SR_NO			 : WRDWIZCOMMON_0264
****************************************************************************/
void CWardwizLangManager::InitializeVariables()
{
}

/***************************************************************************
  Function Name  : GetString
  Description    : Function which returns the string from selected language ini
				   file.
  Author Name    : Ramkrushna Shelke
  Date           : 22nd Apr 2014
  SR_NO			 : WRDWIZCOMMON_0265
****************************************************************************/
CString CWardwizLangManager::GetString(CString csStringID)
{
	DWORD dwLangID = GetSelectedLanguage();

	CString csFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS";
	switch(dwLangID)
	{
	case ENGLISH:
		csFilePath += L"\\ENGLISH.INI";
		break;
	case HINDI:
		csFilePath += L"\\HINDI.INI";
		break;
	case GERMAN:
		csFilePath += L"\\GERMAN.INI";
		break;
	case CHINESE:
		csFilePath += L"\\CHINESE.INI";
		break;
	case SPANISH:
		csFilePath += L"\\SPANISH.INI";
		break;
	case FRENCH:
		csFilePath += L"\\FRENCH.INI";
		break;
	}

	if(!PathFileExists(csFilePath))
	{
		AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetString", csFilePath, 0, true, SECONDLEVEL);
		return EMPTY_STRING;
	}

	TCHAR szValue[2000] = {0};
	GetPrivateProfileString(L"VBSETTINGS", csStringID, L"", szValue, 2000, csFilePath);

	return szValue;	
}

/***************************************************************************
  Function Name  : GetSelectedLanguage
  Description    : Function returns DWORD value
				   0 - ENGLISH
				   1 - HINDI
				   2 - GERMAN
				   3 - CHINESE
				   4 - SPANISH
				   5 - FRENCH
  Author Name    : Ramkrushna Shelke
  Date           : 22nd Apr 2014
  SR_NO			 : WRDWIZCOMMON_0266
****************************************************************************/
DWORD CWardwizLangManager::GetSelectedLanguage()
{
	CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";

	if(!PathFileExists(csIniFilePath))
	{
		AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetSelectedLanguage", csIniFilePath, 0, true, SECONDLEVEL);
		return 0xFFFF;
	}

	return GetPrivateProfileInt(L"VBSETTINGS", L"LanguageID", 0, csIniFilePath);
}

/***************************************************************************
  Function Name  : SetSelectedLanguage
  Description    : Function which writes DWORD value in ini file.
  Author Name    : Ramkrushna Shelke
  Date           : 22nd Apr 2014
  SR_NO			 : WRDWIZCOMMON_0267
****************************************************************************/
void CWardwizLangManager::SetSelectedLanguage(DWORD dwLangID)
{
	CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";

	CString csLangID;
	csLangID.Format(L"%d", dwLangID);
	WritePrivateProfileString(L"VBSETTINGS", L"LanguageID", csLangID, csIniFilePath);
}

/***************************************************************************
  Function Name  : GetWardWizPathFromRegistry
  Description    : To get the string from registry.
  Author Name    : Ramkrushna Shelke
  Date           : 22nd Apr 2014
  SR_NO			 : WRDWIZCOMMON_0268
****************************************************************************/
CString CWardwizLangManager::GetWardWizPathFromRegistry( )
{
	HKEY	hSubKey = NULL ;
	TCHAR	szModulePath[MAX_PATH] = {0};

	if (m_csRegKeyPath == EMPTY_STRING)
	{
		m_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
	}

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium", 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
		{
			return L"";
		}
	}

	DWORD	dwSize = 511 ;
	DWORD	dwType = 0x00 ;

	RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if(_tcslen(szModulePath) > 0)
	{
		return CString(szModulePath) ;
	}
	return L"";
}

/***************************************************************************
  Function Name  : GetSelectedProductEdition
  Description    : Function returns DWORD value
				   1 - ESSENTIAL
				   2 - PRO
				   3 - ELITE
				   4 - BASIC
  Author Name    : Neha Gharge
  Date           : 27nd May 2014
  SR_NO			 : WRDWIZCOMMON_0269
****************************************************************************/
DWORD CWardwizLangManager::GetSelectedProductID()
{
	CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";

	if(!PathFileExists(csIniFilePath))
	{
		AddLogEntry(L"### %s file not found, In CWardwizLangManager::GetSelectedProductID", csIniFilePath, 0, true, SECONDLEVEL);
		return 0xFFFF;
	}

	return GetPrivateProfileInt(L"VBSETTINGS", L"ProductID", 1, csIniFilePath);
}

/***************************************************************************
  Function Name  : SetSelectedProductID
  Description    : Function which writes DWORD value in ini file.
  Author Name    : Neha Gharge
  Date           : 27nd May 2014
  SR_NO			 : WRDWIZCOMMON_0270
****************************************************************************/
void CWardwizLangManager::SetSelectedProductID(DWORD dwProductID)
{
	CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";

	CString csProductID;
	csProductID.Format(L"%d", dwProductID);
	WritePrivateProfileString(L"VBSETTINGS", L"ProductID", csProductID, csIniFilePath);
}
