#include "WWizFirewallFilter.h"
#include "WardWizDatabaseInterface.h"
#include "iTinRegWrapper.h"
#include "WWizSettingsWrapper.h"

CWWizFirewallFilter::CWWizFirewallFilter()
{
	ReLoadBlockedApplicationList();
}

CWWizFirewallFilter::~CWWizFirewallFilter()
{
}

bool CWWizFirewallFilter::LoadBlockedApplicationList()
{
	try
	{
		TCHAR	szPath[512] = { 0 };
		DWORD	dwSize = sizeof(szPath);
		CITinRegWrapper	objReg;
		CWardWizSQLiteDatabase dbSQlite;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"AppFolder", szPath, dwSize);
		CString csWardWizModulePath = szPath;
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBFIREWALL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("SELECT * FROM WWIZ_FIREWALL_APP_RULE_CONFIG WHERE ACCESS = '1';");

		DWORD dwRows = qResult.GetNumRows();
		if (dwRows != 0)
		{
			for (DWORD dwIndex = 0x00; dwIndex < dwRows; dwIndex++)
			{
				qResult.SetRow(dwIndex);
				if (qResult.GetFieldIsNull(0))
				{
					continue;
				}

				char szPath[MAX_PATH] = { 0 };
				strcpy_s(szPath, qResult.GetFieldValue(1)); //Application path
				strlwr(szPath);
				m_vBlockedAppList.push_back(szPath);
			}
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::LoadBlockedApplicationList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

bool CWWizFirewallFilter::ReLoadBlockedApplicationList()
{
	try
	{
		m_vBlockedAppList.clear();
		LoadBlockedApplicationList();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::ReLoadBlockedApplicationList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

std::wstring StringToWString(const std::string& s)
{
	std::wstring temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

bool CWWizFirewallFilter::ISApplicationBlocked(std::wstring strProcessName)
{
	bool bFound = false;
	try
	{
		//system process can not be blocked.
		if (strProcessName.length() == 0x00)
			return false;

		for (std::vector<std::string>::iterator it = m_vBlockedAppList.begin(); it != m_vBlockedAppList.end(); ++it)
		{
			std::wstring strProcessPath = StringToWString(*it);

			if (_tcscmp(strProcessPath.c_str(), strProcessName.c_str()) == 0)
			{
				bFound = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::ISApplicationBlocked", 0, 0, true, SECONDLEVEL);
	}
	return bFound;
}

