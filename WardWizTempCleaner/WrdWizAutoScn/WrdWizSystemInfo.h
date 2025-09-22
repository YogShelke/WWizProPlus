
#include "stdafx.h"

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

protected:

	TCHAR	m_szCompName[64];
	TCHAR	m_szUserName[64];
	TCHAR	m_szOSName[128];
	TCHAR	m_szSPVersion[128];
	TCHAR	m_szWinDir[256];

	DWORD	m_dwMajorVersion;
	DWORD	m_dwMinorVersion;
	bool	m_bX86;

};