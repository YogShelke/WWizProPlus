#pragma once
#include "stdafx.h"

#include "WardwizOSVersion.h"
#include <winbase.h>
#include <tlhelp32.h>

#include <Wtsapi32.h>
#include <Userenv.h>


#define _DEF_WIN32_WINNT_NT4                    0x0400
#define _DEF_WIN32_WINNT_WIN2K                  0x0500
#define _DEF_WIN32_WINNT_WINXP                  0x0501
#define _DEF_WIN32_WINNT_WS03                   0x0502
#define _DEF_WIN32_WINNT_WIN6                   0x0600
#define _DEF_WIN32_WINNT_VISTA                  0x0600
#define _DEF_WIN32_WINNT_WS08                   0x0600
#define _DEF_WIN32_WINNT_LONGHORN               0x0600
#define _DEF_WIN32_WINNT_WIN7                   0x0601
#define _DEF_WIN32_WINNT_WIN8                   0x0602
#define _DEF_WIN32_WINNT_WINBLUE                0x0603
#define _DEF_WIN32_WINNT_WIN10                  0x0A00
#define VER_SUITE_WH_SERVER                 0x00008000

class WardWizSystemInfo
{
public:
	WardWizSystemInfo();
	~WardWizSystemInfo();

	DWORD GetSystemInformation();

	DWORD GetProcessorType();
	DWORD GetComputerAndUserName();
	DWORD GetOSInformation();
	DWORD GetOSOtherDetails();

	LPCTSTR	GetSystemName();
	LPCTSTR	GetUserNameOfSystem();
	LPCTSTR	GetOSDetails();
	bool GetOSType();

	DWORD GetUserName(LPTSTR lpUserName);
	HANDLE GetExplorerProcessHandle(CString csAccessProcessName = _T("explorer.exe"));

	
	BOOL  _IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor);
	BOOL _IsWindows8OrGreater();
	BOOL _IsWindows8Point1OrGreater();
	BOOL _IsWindows10OrGreater();

protected:

	TCHAR	m_szCompName[64];
	TCHAR	m_szUserName[64];
	TCHAR	m_szOSName[512];			//OS Name buffer increased
	TCHAR	m_szSPVersion[128];
	TCHAR	m_szWinDir[256];

	DWORD	m_dwMajorVersion;
	DWORD	m_dwMinorVersion;
	bool	m_bX86;

};