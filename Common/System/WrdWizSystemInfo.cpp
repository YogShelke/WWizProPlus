#include "StdAfx.h"
#include "WrdWizSystemInfo.h"

#include "iTinRegWrapper.h"

WardWizSystemInfo::WardWizSystemInfo()
{
	ZeroMemory(m_szUserName, sizeof(m_szUserName));
	ZeroMemory(m_szCompName, sizeof(m_szCompName));
	ZeroMemory(m_szOSName, sizeof(m_szOSName));
	ZeroMemory(m_szSPVersion, sizeof(m_szSPVersion));
	ZeroMemory(m_szWinDir, sizeof(m_szWinDir));

	m_dwMajorVersion = 0x00;
	m_dwMinorVersion = 0x00;
	m_bX86 = false;
}

WardWizSystemInfo::~WardWizSystemInfo()
{

}

DWORD WardWizSystemInfo::GetSystemInformation()
{
	DWORD	dwRet = 0x00;

	try
	{
		
		//GetProcessorType();
		GetComputerAndUserName();
		GetOSInformation();
		GetOSOtherDetails();

	}
	catch (...)
	{

	}

	return dwRet;
}

DWORD WardWizSystemInfo::GetProcessorType()
{
	DWORD	dwRet = 0x00;

	SYSTEM_INFO	sysInfo = { 0 };

	//GetSystemInfo(&sysInfo);
	GetNativeSystemInfo(&sysInfo);

	if ((sysInfo.wProcessorArchitecture&PROCESSOR_ARCHITECTURE_AMD64) == PROCESSOR_ARCHITECTURE_AMD64)
		m_bX86 = true;
	else
		m_bX86 = false;

	return dwRet;
}

DWORD WardWizSystemInfo::GetComputerAndUserName()
{
	DWORD	dwRet = 0x3F;

	try
	{

		GetComputerName(m_szCompName, &dwRet);
		if (m_szCompName[0])
			_wcsupr_s(m_szCompName, 63);

		dwRet = 0x3F;
		GetUserName(m_szUserName);

		if (wcslen(m_szUserName) == 0x00)
			::GetUserName(m_szUserName, &dwRet);

		if (m_szUserName[0])
			_wcsupr_s(m_szUserName, 63);

	}
	catch (...)
	{

	}

	return 0;
}

BOOL  WardWizSystemInfo::_IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
	DWORDLONG        const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
		VerSetConditionMask(
		0, VER_MAJORVERSION, VER_GREATER_EQUAL),
		VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

BOOL WardWizSystemInfo::_IsWindows8OrGreater()
{
	return _IsWindowsVersionOrGreater(HIBYTE(_DEF_WIN32_WINNT_WIN8), LOBYTE(_DEF_WIN32_WINNT_WIN8), 0);
}

BOOL WardWizSystemInfo::_IsWindows8Point1OrGreater()
{
	return _IsWindowsVersionOrGreater(HIBYTE(_DEF_WIN32_WINNT_WINBLUE), LOBYTE(_DEF_WIN32_WINNT_WINBLUE), 0);
}

BOOL WardWizSystemInfo::_IsWindows10OrGreater()
{
	return _IsWindowsVersionOrGreater(HIBYTE(_DEF_WIN32_WINNT_WIN10), LOBYTE(_DEF_WIN32_WINNT_WIN10), 0);
}

DWORD WardWizSystemInfo::GetOSInformation()
{
	DWORD	dwRet = 0x00;


	try
	{

		OSVERSIONINFOEX		osvi = { 0 };
		SYSTEM_INFO			sysInfo = { 0 };

		TCHAR				szOSName[256] = { 0 };
		TCHAR				szOSVer[16] = { 0 };

		//GetSystemInfo(&sysInfo);
		GetNativeSystemInfo(&sysInfo);

		if ((sysInfo.wProcessorArchitecture&PROCESSOR_ARCHITECTURE_AMD64) == PROCESSOR_ARCHITECTURE_AMD64)
		{
			m_bX86 = true;
			wcscpy_s(szOSVer, 15, L"64 bit");
		}
		else
		{
			wcscpy_s(szOSVer, 15, L"32 bit");
		}

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);

		if ( !bOsVersionInfoEx)
			dwRet = 0x01;

		//Win 8, Win 8.1 and 10, we can't use function "GetVersionEx" directly
		//We've to use VersionHelpers function
		//Modified by Vilas on 26 May 2015

		BOOL bWin8 = _IsWindows8OrGreater();
		BOOL bWin8Point1 = _IsWindows8Point1OrGreater();
		BOOL bWin10orGre = _IsWindows10OrGreater();

		//Added by Vilas on 16/09/2015 and 
		//modified on 17 / 11 / 2015 by Vilas
		//Failing to fetch version on Win10
		TCHAR	szOSProductName[256] = { 0 };
		DWORD	dwSize = 255;

		CITinRegWrapper	objReg;
		
		DWORD dwErr = objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName", szOSProductName, dwSize);
		if (szOSProductName[0])
		{
			_wcsupr_s(szOSProductName, 255);

		}

		if ((bWin8) && (!bWin8Point1))
			osvi.dwMinorVersion = 0x02;

		if ((bWin8Point1) && (!bWin10orGre))
			osvi.dwMinorVersion = 0x03;

		if ((bWin10orGre) || (wcsstr(szOSProductName, L"WINDOWS 10")) )
			osvi.dwMajorVersion = 0x0A;
		
		switch (osvi.dwMajorVersion)
		{
			case 5:
				switch (osvi.dwMinorVersion)
				{
					case 0:
						wcscpy_s(szOSName, 63, L"Windows 2000 ");
						break;

					case 1:
						wcscpy_s(szOSName, 63, L"Windows XP ");
						break;

					case 2:
						if ((osvi.wProductType == VER_NT_WORKSTATION) && (m_bX86) )
							wcscpy_s(szOSName, 63, L"Windows XP Professional ");
						else
						{
							if ((osvi.wSuiteMask& VER_SUITE_WH_SERVER) == VER_SUITE_WH_SERVER)
								wcscpy_s(szOSName, 63, L"Windows Home Server ");
							else
							{
								if (GetSystemMetrics(SM_SERVERR2) == 0x00)
									wcscpy_s(szOSName, 63, L"Windows Server 2003 ");
								else
									wcscpy_s(szOSName, 63, L"Windows Server 2003 R2 ");
							}
						}

						break;
				}
				break;

			case 6:
				switch (osvi.dwMinorVersion)
				{
					case 0:
						if( osvi.wProductType == VER_NT_WORKSTATION )
							wcscpy_s(szOSName, 63, L" Windows Vista ");
						else
							wcscpy_s(szOSName, 63, L"Windows Server 2008 ");
						break;

					case 1:
						if (osvi.wProductType == VER_NT_WORKSTATION)
							wcscpy_s(szOSName, 63, L"Windows 7 ");
						else
							wcscpy_s(szOSName, 63, L"Windows Server 2012 ");
						break;

					case 2:
						if (osvi.wProductType == VER_NT_WORKSTATION)
							wcscpy_s(szOSName, 63, L"Windows 8 ");
						else
							wcscpy_s(szOSName, 63, L"Windows Server 2012 ");
						break;

					case 3:
						if (osvi.wProductType == VER_NT_WORKSTATION)
							wcscpy_s(szOSName, 63, L"Windows 8.1 ");
						else
							wcscpy_s(szOSName, 63, L"Windows Server 2012 R2 ");
						break;
				}
				break;

			case 10:
				//Modified on 07 / 10 / 2015 by Vilas
				//to get OS name from registry
				//switch (osvi.dwMinorVersion)
				//{

					/*case 0:
						if (osvi.wProductType == VER_NT_WORKSTATION)
							wcscpy_s(szOSName, 63, L"Windows 10 Insider Preview ");
						else
							wcscpy_s(szOSName, 63, L"Windows Server Technical Preview ");
						break;
					*/
					if (szOSProductName[0])
						wcscpy_s(szOSName, 255, szOSProductName);
					else
						wcscpy_s(szOSName, 255, L"WINDOWS 10");
					wcscat_s(szOSName, 255, L" ");
				//}

				break;

			default:
				dwRet = 0x02;
		}

		if (szOSName[0])
		{
			if (wcslen(osvi.szCSDVersion) )
				swprintf_s(m_szOSName, 511, L"%s%s with %s", szOSName, szOSVer, osvi.szCSDVersion);
			else
				swprintf_s(m_szOSName, 511, L"%s%s", szOSName, szOSVer);

			_wcsupr_s(m_szOSName, 511);
		}
	 }
	 catch (...)
	 {

	 }

	return dwRet;
}

DWORD WardWizSystemInfo::GetOSOtherDetails()
{
	DWORD	dwRet = 0x00;

	GetWindowsDirectory(m_szWinDir, 255);

	return dwRet;
}


LPCTSTR	WardWizSystemInfo::GetSystemName()
{
	if (!m_szCompName[0])
		GetComputerAndUserName();

	return m_szCompName;
}


LPCTSTR	WardWizSystemInfo::GetUserNameOfSystem()
{
	if (!m_szUserName[0])
		GetComputerAndUserName();

	return m_szUserName;
}


LPCTSTR	WardWizSystemInfo::GetOSDetails()
{
	if (!m_szOSName[0])
		GetSystemInformation();

	return m_szOSName;
}


bool WardWizSystemInfo::GetOSType()
{
	if (!m_bX86)
		GetProcessorType();

	return m_bX86;
}



/***************************************************************************
Function Name  : GetExplorerProcessHandle
Description    : Function which enumerates the process and returns the handle of explorer
Author Name    : Ramkrushna Shelke
Date           : 31st Aug 2014
****************************************************************************/
HANDLE WardWizSystemInfo::GetExplorerProcessHandle(CString csAccessProcessName)
{
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;
	ZeroMemory(&pe32, sizeof(pe32));
	HANDLE temp = NULL;

	try
	{
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				CString csExeName = pe32.szExeFile;

				//Version:	: 19.0.0.015
				//On Vista Explorer.exe run as standard user with limited rights
				//So it will not able to execute process as run as admin 
				CWardWizOSversion objOSVersionWrap;
				DWORD dwOSType = objOSVersionWrap.DetectClientOSVersion();
				if (dwOSType == WINOS_VISTA)
				{
					if (csExeName.CompareNoCase(_T("explorer.exe")) == 0)
					{
						temp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
					}
				}
				else if (csAccessProcessName)
				{
					if (csExeName.CompareNoCase(csAccessProcessName) == 0)
					{
						temp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
						break;
					}
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
	}
	catch (...)
	{
	}

	return temp;
}

DWORD WardWizSystemInfo::GetUserName(LPTSTR lpUserName)
{

	try
	{

		HANDLE				hToken = NULL;
		TOKEN_USER          oUser[16] = { 0 };
		DWORD               u32Needed = 0x00;
		TCHAR               domainName[256] = { 0 };
		DWORD               userNameSize = 0x3F, domainNameSize;
		SID_NAME_USE        sidType;

		BOOL bRet = OpenProcessToken(GetExplorerProcessHandle(), TOKEN_ALL_ACCESS, &hToken);
		if (!bRet)
		{
			return 1;
		}

		GetTokenInformation(hToken, TokenUser, &oUser[0], sizeof(oUser), &u32Needed);

		domainNameSize = _countof(domainName) - 1;

		LookupAccountSid(NULL, oUser[0].User.Sid, lpUserName, &userNameSize, domainName, &domainNameSize, &sidType);

		if (hToken)
			CloseHandle(hToken);

		if (lpUserName[0])
			return 0;

	}
	catch (...)
	{

	}

	return 2;
}
