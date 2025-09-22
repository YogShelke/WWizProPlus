/**********************************************************************************************************
Program Name          : WWizSettingsWrapper.cpp
Description           : This class is for get user defined settings which are related to product functionality.
					    This class read from PRODUCTSETTINGS.INI file and provides the value wherever it is 
						required.
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 31st May 2016
Version No            : 2.0.0.14
Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "WWizSettingsWrapper.h"

/***********************************************************************************************
*  Function Name  : CWWizSettingsWrapper
*  Description    : Constructor
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 31st May 2016
*************************************************************************************************/
CWWizSettingsWrapper::CWWizSettingsWrapper()
{
}

/***********************************************************************************************
*  Function Name  : ~CWWizSettingsWrapper
*  Description    : Des'r
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date           : 31st May 2016
**********************************************************************************************/
CWWizSettingsWrapper::~CWWizSettingsWrapper()
{
}

/***********************************************************************************************
*  Function Name  : GetProductRegistryKey
*  Description    : Function which read the value for which is the product key name in the 
					registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 31st May 2016
**********************************************************************************************/
CString CWWizSettingsWrapper::GetProductRegistryKey()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"VibraniumRegPath", L"SOFTWARE\\Vibranium", szValue, sizeof(szValue), szFullFilePath);
		
		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetProductRegistryKey", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***************************************************************************
Function Name  : GetModulePath
Description    : Function returns module path
Author Name    : Neha G
Date           : 25th June 2014
SR_NO			 : WRDWIZUSBUI_0066
****************************************************************************/
bool CWWizSettingsWrapper::GetModulePath(TCHAR *szModulePath, DWORD dwSize)
{
	if (0 == GetModuleFileName(NULL, szModulePath, dwSize))
	{
		return false;
	}

	if (_tcsrchr(szModulePath, _T('\\')))
	{
		*_tcsrchr(szModulePath, _T('\\')) = 0;
	}
	return true;
}


/***********************************************************************************************
*  Function Name  : GetDLLModulePath
*  Description    : Function to get Module path of COM DLL which is loaded by EXPLORER
					Dont add any addlog entry here; otherwise will be in recursive loop.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 04th Jun 2016
**********************************************************************************************/
bool CWWizSettingsWrapper::GetCOMDLLModulePath(LPTSTR lpszPath, DWORD dwSize)
{
	bool bReturn = false;
	try
	{
		CString csKeyName = L"CLSID\\{5E2121EE-0300-11D4-8D3B-444553540000}\\InprocServer32";

		HKEY	hSubKey = NULL;
		DWORD	dwRet = 0x00, dwRegType = 0x00;
		RegOpenKeyEx(HKEY_CLASSES_ROOT, csKeyName, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey);
		if (!hSubKey)
		{
			return bReturn;
		}

		dwRet = RegQueryValueEx(hSubKey, NULL, 0, &dwRegType, (LPBYTE)lpszPath, &dwSize);
		RegCloseKey(hSubKey);
		hSubKey = NULL;

		if (_tcsrchr(lpszPath, _T('\\')))
		{
			*_tcsrchr(lpszPath, _T('\\')) = 0;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CSimpleShlExt::GetDLLModulePath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***********************************************************************************************
*  Function Name  : GetRegisteredCountry
*  Description    : Function to get country of registered product.
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetRegisteredCountry()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"Country", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetRegisteredCountry", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetRegisteredState
*  Description    : Function to get State of registered product.
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetRegisteredState()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"State", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetRegisteredState", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetRegisteredCity
*  Description    : Function to get State of registered product.
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetRegisteredCity()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"City", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetRegisteredCity", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetRegisteredPinCode
*  Description    : Function to get Pin code of registered product.
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetRegisteredPinCode()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"PinCode", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetRegisteredPinCode", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetCurrentUserName
*  Description    : Function to get current user name of registered product.
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetCurrentUserName()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"CurrentUserName", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetCurrentUserName", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetCurrentOSName
*  Description    : Function to get OS name of registered product.
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetCurrentOSName()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"OSName", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetCurrentOSName", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetCurrentRamSize
*  Description    : Function to get OS name of registered product.
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetCurrentRamSize()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"RamSize", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetCurrentRamSize", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetCurrentHardDiskSize
*  Description    : Function to get OS name of registered product.
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetCurrentHardDiskSize()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"HardDiskSize", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetCurrentHardDiskSize", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetCurrentProcessorSize
*  Description    : Function to get processor size
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetCurrentProcessorSize()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"Processor", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetCurrentProcessorSize", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}


/***********************************************************************************************
*  Function Name  : GetCurrentComputerName
*  Description    : Function to get current computer name
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetCurrentComputerName()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"ComputerName", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetCurrentComputerName", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}


/***********************************************************************************************
*  Function Name  : GetDealerCode
*  Description    : Function to get dealer code
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetDealerCode()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"DealerCode", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetDealerCode", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}


/***********************************************************************************************
*  Function Name  : GetReferenceID
*  Description    : Function to get Engineer reference ID
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetReferenceID()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"ReferenceID", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetReferenceID", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}


/***********************************************************************************************
*  Function Name  : GetEngineerName
*  Description    : Function to get Enginner Name
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetEngineerName()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"EngineerName", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetEngineerName", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}


/***********************************************************************************************
*  Function Name  : GetEngineerMobNo
*  Description    : Function to get engineer mobile number
*  Author Name    : Ramkrushna Shelke
*  Date           : 11 Feb 2023
**********************************************************************************************/
CString CWWizSettingsWrapper::GetEngineerMobNo()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };

#if SHELLEXTENSIONDLL
		if (!GetCOMDLLModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#else
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}
#endif 

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"EngineerMobNo", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettingsWrapper::GetEngineerMobNo", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}