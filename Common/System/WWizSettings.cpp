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
			AddLogEntry(L"### Error in GetRegistryValueData CWardwizSettings::GetMachineID()", 0, 0, true, SECONDLEVEL);
			return csReturn;
		}
		csReturn = m_csMachineID = szMachineIDValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettings::GetMachineID", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Excpetion in CWardwizSettings::GetMachineID", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Excpetion in CWardwizSettings::SetBearerCode", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Excpetion in CWardwizSettings::VibraniumGetComputerName", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Excpetion in CWardwizSettings::VibraniumGetOSDetails", 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}

#ifdef WARDWIZ_EPS
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
		AddLogEntry(L"### Excpetion in CWardwizSettings::VibraniumGetOSDetails", 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}
#endif

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
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\EPSSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"ServerMchineIP", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettings::VibraniumGetServerIPAddress", 0, 0, true, SECONDLEVEL);
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
		swprintf_s(szFullFilePath, L"%s\\VBSETTINGS\\EPSSETTINGS.INI", szModulePath);

		TCHAR szValue[MAX_PATH] = { 0 };
		GetPrivateProfileString(L"VBSETTINGS", L"ServerMchineName", L"", szValue, sizeof(szValue), szFullFilePath);

		csReturn = szValue;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizSettings::VibraniumServerMachineName", 0, 0, true, SECONDLEVEL);
	}
	return csReturn;
}