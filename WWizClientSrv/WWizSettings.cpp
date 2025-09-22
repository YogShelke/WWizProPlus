#include "stdafx.h"
#include "WWizSettings.h"

CString	CWWizSettings::m_csProdRegKey = EMPTY_STRING;
CString	CWWizSettings::m_csMachineID = EMPTY_STRING;
CString	CWWizSettings::m_csClientGUIID = EMPTY_STRING;
CString	CWWizSettings::m_csBearerCode = _T("NA");

CWWizSettings::CWWizSettings()
{
}

CWWizSettings::~CWWizSettings()
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
CString CWWizSettings::GetProductRegistryKey()
{
	CString csReturn = L"";
	try
	{
		if (m_csProdRegKey.GetLength() > 0)
			return m_csProdRegKey;

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
		swprintf_s(szFullFilePath, L"%s\\WRDSETTINGS\\PRODUCTSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"WRDSETTINGS", L"WrdWizRegPath", L"SOFTWARE\\WardWiz", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettingsWrapper::GetProductRegistryKey", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetMachineID
*  Description    : Function which returns machine ID
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 31st May 2016
**********************************************************************************************/
CString CWWizSettings::GetMachineID()
{
	CString csReturn = L"";
	try
	{
		if (m_csProdRegKey.GetLength() == 0)
		{
			m_csProdRegKey = GetProductRegistryKey();
		}

		if (m_csMachineID.GetLength() > 0)
			return m_csMachineID;

		//CITinRegWrapper objReg;
		TCHAR szMachineIDValue[0x80] = { 0 };
		DWORD dwSize = sizeof(szMachineIDValue);

		CITinRegWrapper objReg;
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, m_csProdRegKey.GetBuffer(), L"MVersion", szMachineIDValue, dwSize) != 0)
		{
			AddLogEntry(L"### Error in GetRegistryValueData CWWizSettings::GetMachineID()", 0, 0, true, SECONDLEVEL);
			return csReturn;
		}
		csReturn = m_csMachineID = szMachineIDValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::GetMachineID", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetMachineID
*  Description    : Function which returns machine ID
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 31st May 2016
**********************************************************************************************/
CString CWWizSettings::GetClientGUIID()
{
	CString csReturn = L"";
	try
	{
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::GetMachineID", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : SetBearerCode
*  Description    : Function to setBearer code
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 31st May 2016
**********************************************************************************************/
bool CWWizSettings::SetBearerCode(CString csBearerCode)
{
	bool bReturn = false;
	try
	{
		if (csBearerCode.GetLength() != 0)
		{
			m_csBearerCode = csBearerCode;
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::SetBearerCode", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : WWizGetComputerName
*  Description    : Function which returns computer name.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date           : 03 March 2016
**********************************************************************************************/
CString CWWizSettings::WWizGetComputerName()
{
	try
	{
		WardWizSystemInfo		objSysInfo;
		return CString(objSysInfo.GetSystemName());
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::WWizGetComputerName", 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}


/***********************************************************************************************
*  Function Name  : WWizGetOSDetails
*  Description    : Function which returns OS Name.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 03 March 2016
**********************************************************************************************/
CString CWWizSettings::WWizGetOSDetails()
{
	try
	{
		WardWizSystemInfo		objSysInfo;
		return CString(objSysInfo.GetOSDetails());
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::WWizGetOSDetails", 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}

/***********************************************************************************************
*  Function Name  : WWizGetIPAddress
*  Description    : Function which returns IP Address.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date           : 03 March 2016
**********************************************************************************************/
CString CWWizSettings::WWizGetIPAddress()
{
	try
	{
		CSocketComm m_SocketManager;
		CString strLocal;
		m_SocketManager.GetLocalAddress(strLocal.GetBuffer(256), 256);
		return strLocal;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::WWizGetOSDetails", 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}

/***********************************************************************************************
*  Function Name  : WWizGetServerIPAddress
*  Description    : Function which returns Server IP Address.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 03 March 2016
**********************************************************************************************/
CString CWWizSettings::WWizGetServerIPAddress()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\WRDSETTINGS\\EPSSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"WRDSETTINGS", L"ServerMchineIP", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::WWizGetServerIPAddress", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : WWizServerMachineName
*  Description    : Function which returns server Name.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 03 March 2016
**********************************************************************************************/
CString CWWizSettings::WWizServerMachineName()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\WRDSETTINGS\\EPSSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"WRDSETTINGS", L"ServerMchineName", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::WWizServerMachineName", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : WWizGetDomainValue
*  Description    : Function which returns Domain Name.
*  Author Name    : Jeena Mariam Saji
*  SR_NO		  :
*  Date           : 09 April 2018
**********************************************************************************************/
CString CWWizSettings::WWizGetDomainValue()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\WRDSETTINGS\\EPSSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"WRDSETTINGS", L"Domain", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::WWizGetDomainValue", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetServerHttpPort
*  Description    : Function which returns server http port. DEFAULT: [8096]
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 25 May 2018
**********************************************************************************************/
CString CWWizSettings::GetServerHttpPort()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\WRDSETTINGS\\EPSSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"WRDSETTINGS", L"ServerHttpPort", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::GetServerHttpPort", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/***********************************************************************************************
*  Function Name  : GetServerSSLPort
*  Description    : Function which returns server SSL port. DEFAULT: [9106]
*  Author Name    : Ram Shelke
*  SR_NO		  :
*  Date           : 25 May 2018
**********************************************************************************************/
CString CWWizSettings::GetServerSSLPort()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\WRDSETTINGS\\EPSSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"WRDSETTINGS", L"ServerSSLPort", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::GetServerSSLPort", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}


/***********************************************************************************************
*  Function Name  : GetHttpCommunicationPort
*  Description    : Function which returns server port number as per client settings.
*  Author Name    : Ram Shelke
*  SR_NO		  : 
*  Date           : 25 May 2018
**********************************************************************************************/
CString CWWizSettings::GetHttpCommunicationPort()
{
	CString csReturn = L"";
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, sizeof(szModulePath)))
		{
			return L"";
		}

		TCHAR szFullFilePath[MAX_PATH] = { 0 };
		swprintf_s(szFullFilePath, L"%s\\WRDSETTINGS\\EPSSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"WRDSETTINGS", L"EnableSSL", L"0", szValue, sizeof(szValue), szFullFilePath);

		if (_tcscmp(szValue, L"1") == 0)
		{
			csReturn = GetServerSSLPort();
		}
		else
		{
			csReturn = GetServerHttpPort();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWWizSettings::GetHttpCommunicationPort", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}