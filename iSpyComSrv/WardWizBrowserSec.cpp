/**********************************************************************************************************
Program Name          : WardWiz Browser Security
Description           : This file contains logic for initializing Browser Security values and calling
						external functions in WardwizEmailScn.cpp/PFMailFilter.cpp. To reload values stored in 
						memory on change of controls on Browser Security UI
Author Name			  : Jeena Mariam Saji
Date Of Creation      : 04 Sept 2019
Special Logic Used    : Communication using named pipes & using Shared Memory.
Modification Log      :
***********************************************************************************************************/
#include "WardWizBrowserSec.h"

/***********************************************************************************************
Function Name  : CWardWizBrowserSec
Description    : Const'r
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
CWardWizBrowserSec::CWardWizBrowserSec():
m_hModuleBrowserSecDLL(NULL),
m_lpBrowserSecInitialize(NULL),
m_lpBrowserSecUninitialize(NULL),
m_lpBrowserSecReload(NULL),
m_lpBrowserSecExcReload(NULL),
m_lpBrowserSecSpecReload(NULL)
{
}

/***********************************************************************************************
Function Name  : ~CWardWizBrowserSec
Description    : Dest'r
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
CWardWizBrowserSec::~CWardWizBrowserSec()
{
	if (m_lpBrowserSecUninitialize != NULL)
	{
		m_lpBrowserSecUninitialize();
	}

	if (m_hModuleBrowserSecDLL != NULL)
	{
		FreeLibrary(m_hModuleBrowserSecDLL);
		m_hModuleBrowserSecDLL = NULL;
	}
}

/***********************************************************************************************
Function Name  : Initialize
Description    : To initialise values
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
bool CWardWizBrowserSec::Initialize()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleBrowserSecDLL == NULL)
			LoadRequiredModules();

		if (m_lpBrowserSecInitialize)
		{
			bReturn = m_lpBrowserSecInitialize();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizBrowserSec::Initialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : Initialize
Description    : To uninitialise values
Author Name    : Jeena Mariam Saji
Date           : 05 Sept 2019
***********************************************************************************************/
bool CWardWizBrowserSec::UnInitialize()
{
	bool bReturn = false;
	__try
	{
		if (m_lpBrowserSecUninitialize)
		{
			bReturn = m_lpBrowserSecUninitialize();
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizBrowserSec::UnInitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : LoadRequiredModules
Description    : To load required modules
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
bool CWardWizBrowserSec::LoadRequiredModules()
{
	bool bReturn = false;
	__try
	{
		if (m_hModuleBrowserSecDLL)
		{
			return true;
		}
		if (!m_hModuleBrowserSecDLL)
		{
			m_hModuleBrowserSecDLL = LoadLibrary(L"VBEMAILSCN.DLL");
			if (!m_hModuleBrowserSecDLL)
			{
				AddLogEntry(L"### Error in loading VBEMAILSCN.DLL", 0, 0, true, SECONDLEVEL);
				return false;
			}

			m_lpBrowserSecInitialize = (WRDWIZBROWSERSECINITILIAZE)GetProcAddress(m_hModuleBrowserSecDLL, "WardWizBrowserSecInitialize");
			if (!m_lpBrowserSecInitialize)
			{
				AddLogEntry(L"### CWardwizBrowserSec::LoadRequiredModules Error in GetProcAddress::m_lpBrowserSecInitialize", 0, 0, true, SECONDLEVEL);
				m_lpBrowserSecInitialize = NULL;
			}

			m_lpBrowserSecUninitialize = (WRDWIZBROWSERSECUNINITILIAZE)GetProcAddress(m_hModuleBrowserSecDLL, "WardWizBrowserSecUnnitialize");
			if (!m_lpBrowserSecUninitialize)
			{
				AddLogEntry(L"### CWardwizBrowserSec::LoadRequiredModules Error in GetProcAddress::m_lpBrowserSecUninitialize", 0, 0, true, SECONDLEVEL);
				m_lpBrowserSecUninitialize = NULL;
			}

			m_lpBrowserSecReload = (WRDWIZBROWSERSECRELOAD)GetProcAddress(m_hModuleBrowserSecDLL, "WardWizBrowserSecReload");
			if (!m_lpBrowserSecReload)
			{
				AddLogEntry(L"### CWardwizBrowserSec::LoadRequiredModules Error in GetProcAddress::m_lpBrowserSecReload", 0, 0, true, SECONDLEVEL);
				m_lpBrowserSecReload = NULL;
			}

			m_lpBrowserSecExcReload = (WRDWIZBROWSERSECEXCRELOAD)GetProcAddress(m_hModuleBrowserSecDLL, "WardWizBrowserSecExcReload");
			if (!m_lpBrowserSecExcReload)
			{
				AddLogEntry(L"### CWardwizBrowserSec::LoadRequiredModules Error in GetProcAddress::m_lpBrowserSecExcReload", 0, 0, true, SECONDLEVEL);
				m_lpBrowserSecExcReload = NULL;
			}

			m_lpBrowserSecSpecReload = (WRDWIZBROWSERSECSPECRELOAD)GetProcAddress(m_hModuleBrowserSecDLL, "WardWizBrowserSecSpecReload");
			if (!m_lpBrowserSecSpecReload)
			{
				AddLogEntry(L"### CWardwizBrowserSec::LoadRequiredModules Error in GetProcAddress::m_lpBrowserSecSpecReload", 0, 0, true, SECONDLEVEL);
				m_lpBrowserSecSpecReload = NULL;
			}
		}
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizBrowserSec::LoadRequiredModules", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadBrowserSecVal
Description    : To reload values for Browser Security
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
bool CWardWizBrowserSec::ReloadBrowserSecVal()
{
	bool bReturn = false;
	__try
	{
		if (m_lpBrowserSecReload != NULL)
		{
			bReturn = m_lpBrowserSecReload();
		}
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::ReloadBrowserSecVal", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadBrowserSecExcVal
Description    : To reload values for Browser Security exclusion values
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
bool CWardWizBrowserSec::ReloadBrowserSecExcVal()
{
	bool bReturn = false;
	__try
	{
		if (m_lpBrowserSecExcReload != NULL)
		{
			bReturn = m_lpBrowserSecExcReload();
		}
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in VibraniumWebProtClient::ReloadBrowserSecExcVal", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : ReloadBrowserSecSpecVal
Description    : To reload values for Browser Security specific values
Author Name    : Jeena Mariam Saji
Date           : 04 Sept 2019
***********************************************************************************************/
bool CWardWizBrowserSec::ReloadBrowserSecSpecVal()
{
	bool bReturn = false;
	__try
	{
		if (m_lpBrowserSecSpecReload != NULL)
		{
			bReturn = m_lpBrowserSecSpecReload();
		}
		bReturn = true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWebProtClient::ReloadBrowserSecSpecVal", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
