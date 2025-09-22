#include "stdafx.h"
#include "WardWizPolyScan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT _declspec(dllimport)
#define DLLEXPORT _declspec(dllexport)

// CWardWizPolyScanApp
BEGIN_MESSAGE_MAP(CWardWizPolyScanApp, CWinApp)
END_MESSAGE_MAP()

// CWardWizPolyScanApp construction
CWardWizPolyScanApp::CWardWizPolyScanApp()
{
}
// The one and only CWardWizPolyScanApp object
CWardWizPolyScanApp theApp;

// CWardWizPolyScanApp initialization
BOOL CWardWizPolyScanApp::InitInstance()
{
	CWinApp::InitInstance();
	return TRUE;
}

/***************************************************************************************************
*  Function Name  : SAVAPI_initialize
*  Description    : Function to Initilize SAVAPI library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool SAVAPIInitialize()
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.SAVAPIInitialize();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in SAVAPIInitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SAVAPI_uninitialize
*  Description    : Function to un Initilize SAVAPI library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool SAVAPIUninitialize()
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.SAVAPIUninitialize();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in SAVAPIUninitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : SAVAPI_ScanFile
*  Description    : Function to scan file using SAVAPI library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool SAVAPIScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID)
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.SAVAPIScanFile(pszFilePath, pszVirusName, dwSpyID);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in SAVAPIScanFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SAVAPI_RepairFile
*  Description    : Function to repair file using SAVAPI library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
extern "C" DLLEXPORT bool SAVAPIRepairFile(LPCTSTR pszFilePath, DWORD dwSpyID)
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.SAVAPIRepairFile(pszFilePath, dwSpyID);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in SAVAPIRepairFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SAVAPI_initialize
*  Description    : Function to Initilize SAVAPI library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizPolyScanApp::SAVAPIInitialize()
{
	bool bReturn = false;
	try
	{
		bReturn = m_objSAVAPIScanner.SAVAPI_initialize();
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in SAVAPIInitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SAVAPI_uninitialize
*  Description    : Function to un Initilize SAVAPI library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizPolyScanApp::SAVAPIUninitialize()
{
	bool bReturn = false;
	try
	{
		bReturn = m_objSAVAPIScanner.SAVAPI_uninitialize();
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in SAVAPIUninitialize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : SAVAPI_ScanFile
*  Description    : Function to scan file using SAVAPI library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizPolyScanApp::SAVAPIScanFile(LPCTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID)
{
	bool bReturn = false;
	try
	{
		bReturn = m_objSAVAPIScanner.SAVAPI_ScanFile(pszFilePath, pszVirusName, dwSpyID);
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in SAVAPIScanFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SAVAPI_RepairFile
*  Description    : Function to repair file using SAVAPI library
*  Author Name    : Ram
*  SR_NO		  :
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWardWizPolyScanApp::SAVAPIRepairFile(LPCTSTR pszFilePath, DWORD dwSpyID)
{
	bool bReturn = false;
	try
	{
		bReturn = m_objSAVAPIScanner.SAVAPI_RepairFile(pszFilePath, dwSpyID);
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in SAVAPIRepairFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
