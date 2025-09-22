/********************************************************************************************************** 
   Program Name          : SetupDLL.cpp
   Description           : Defines the initialization routines for the DLL.
   Author Name           : Ramkrushna Shelke                                                                                
   Date Of Creation      : 7/25/2014
   Version No            : 1.0.0
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description
***********************************************************************************************************/
// SetupDLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SetupDLL.h"
#include "Enumprocess.h"
#include "iTINRegWrapper.h"
#include "iSpySrvMgmt.h"
#include "WardwizLangManager.h"
#include "WardwizOSVersion.h"
#include "CommonFunctions.h"
#include "ExecuteProcess.h"
#include "SocketComm.h"

#include "Aclapi.h"
#include "Sddl.h"
#include <io.h>
#include <sys/stat.h>
#include <srrestoreptapi.h>
#include <Strsafe.h>

#include "CSecure64.h"
#include "CScannerLoad.h"
#include "DriverConstants.h"
#include "MigFailedDlg.h"

#include "WinHttpClient.h"
#include <wbemcli.h>
#include <winioctl.h>
#include <setupapi.h>

#pragma warning(disable : 4995)  

#pragma comment(lib, "Advapi32.lib") 
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Rpcrt4.lib")

typedef BOOL(WINAPI *PFN_SETRESTOREPTW) (PRESTOREPOINTINFOW, PSTATEMGRSTATUS);

bool bUninstallDone = false;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)

DWORD WINAPI WatchClickYesThread(LPVOID lParam);
DWORD WINAPI WatchClickOKThread(LPVOID lParam);

BEGIN_MESSAGE_MAP(CSetupDLLApp, CWinApp)
END_MESSAGE_MAP()


bool g_bCoInitializeSecurityCalled = false;

/***************************************************************************
  Function Name  : CSetupDLLApp
  Description    : Constructor
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 7/25/2014
****************************************************************************/
CSetupDLLApp::CSetupDLLApp():
m_csAppPath(L""),
m_hSQLiteDLL(NULL),
m_bISDBLoaded(false),
m_bUninstallation(false),
m_objCom(WWIZ_INSTALLER_SERVER, true, 0x02)
{
	m_csVectInputStrings.clear();
	//m_vOtherAVList.clear();
}

CSetupDLLApp theApp;

/***************************************************************************
  Function Name  : InitInstance
  Description    : Overrided Function which gets called when module gets loaded.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0001
  Date           : 7/25/2014
****************************************************************************/
BOOL CSetupDLLApp::InitInstance()
{
	CWinApp::InitInstance();
	g_csRegKeyPath = L"SOFTWARE\\Vibranium";//Need to be hardcode value 
	return TRUE;
}

/***************************************************************************
  Function Name  : GetModuleFilePath
  Description    : Function which get the current Application path.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0002
  Date           : 7/25/2014
****************************************************************************/
CString CSetupDLLApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}
/***************************************************************************
  Function Name  : CloseAllAplication
  Description    : Function which closes Runing Application.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0003
  Date           : 7/25/2014
  Modification Date : 11/3/15 Neha Gharge USB close issue.
  Modification Date : 03/07/2015 Neha Gharge Closing all exe with asking message 
  with yes, yesall, no , noall , cancel.
****************************************************************************/
bool CSetupDLLApp::CloseAllAplication(bool bIsReInstall)
{
	bool bISFullPath = false;

	if(_tcslen(m_csAppPath) > 0)
	{
		bISFullPath = true;
	}

	CString csModulePath = m_csAppPath;

	/*if(bIsReInstall)
	{*/
	/*if (!GetCloseAction4OutlookIfRunning(bIsReInstall))
	{
		return false;
	}
	*/
	//}

	//CString csAppPath = csModulePath + L"WRDWIZAVUI.EXE";
	CEnumProcess objEnumProcess;
	
	////While Installation or Uninstalltion if crypt exe is running it should close that exe first
	////Neha Gharge 1st July 2015

	CStringArray objcsaWardWizProcesses;
	objcsaWardWizProcesses.Add(L"WRDWIZAVUI.EXE");
	objcsaWardWizProcesses.Add(L"VBUI.EXE");
	objcsaWardWizProcesses.Add(L"VBTRAY.EXE");
	objcsaWardWizProcesses.Add(L"VBAUTORUNSCN.EXE");
	objcsaWardWizProcesses.Add(L"VBTEMPCLR.EXE");
	objcsaWardWizProcesses.Add(L"VBUSBVAC.EXE");
	objcsaWardWizProcesses.Add(L"VBUSBDETECTUI.EXE");
	objcsaWardWizProcesses.Add(L"VBCRYPT.EXE");
	objcsaWardWizProcesses.Add(L"VBSCANNER.EXE");
	objcsaWardWizProcesses.Add(L"VBUI.EXE");
	//objcsaWardWizProcesses.Add(L"VBCOMMSRV.EXE");//Issue resolved: 0001145

	bool bIsAnyProcTerminated = false;
	bool bIsYesAllReply = false;
	CString csMessageContent(L"");
	DWORD dwRetMessage = 0x02;//Bu default YesAll option selected.

	for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
	{
		CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);

		if (objEnumProcess.IsProcessRunning(csProcessName, false, false, false))
		{
			if (!bIsYesAllReply)
			{
				if (m_bUninstallation)
				{
					csMessageContent.Format(L"%s", m_csVectInputStrings.at(CM_0_WWSetupDllAppRunningUnInstall));
				}
				else
				{
					csMessageContent.Format(L"%s", m_csVectInputStrings.at(CM_1_WWSetupDllAppRunningReInstall));
				}
				dwRetMessage = ShowYesAllNoAllMessagebox(csMessageContent);

				switch (dwRetMessage)
				{
				case 0x01:bIsYesAllReply = false;
					//YES
					break;
				case 0x02:bIsYesAllReply = true;
					//YESALL
					break;
				case 0x03://NO
					return false;
					//break;
				case 0x04://NOALL
					return false;
					//break;
				case 0x05://CANCEL
					return false;
					//break;
				}
			}
			if (objEnumProcess.IsProcessRunning(csProcessName, true, false, false))
			{
				bIsAnyProcTerminated = true;
				AddLogEntry(L">>> %s was running, Terminated", csProcessName, 0, true, SECONDLEVEL);
			}
			else
			{
				bIsAnyProcTerminated = true;
				AddLogEntry(L"### %s was running, Failed to terminated", csProcessName, 0, true, SECONDLEVEL);
			}

		}
	}

	//Neha Gharge Unregister com dll 20/4/2015
	CString csAppPath(L"");
	csAppPath = m_csAppPath + L"VBSHELLEXT.DLL";
	if (PathFileExists(csAppPath))
	{
		UnregisterComDll(csAppPath);
	}

	csAppPath = csModulePath + L"VBSHELLEXT_OLD.DLL";
	if (PathFileExists(csAppPath))
	{
		UnregisterComDll(csAppPath);
	}



	//ISSUE NO - 8 After re- installation showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
	//NAME - Niranjan Deshak. - 29th Jan 2015.	
	if(bIsAnyProcTerminated)
	{
		CCommonFunctions objCCommonFunctions;
		objCCommonFunctions.RefreshTaskbarNotificationArea();
	}

	//Dwret is add into addlogentry So that we enable to know the problem while installation
	//Neha Gharge
	//close and remove service here
	CISpySrvMgmt		iSpySrvMgmtObj ;
	DWORD dwRet = 0x00;
	CString csFailureCase(L"");

	dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZSERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}
	//Issue: 1145: 	Issue with Reinstallation, while reinstallation Retry popup appears after clicking on "Retry" button installation get complete.
	//Resolved By: Nitin Kolapkar
	//Need to put sleep because In inno setup we are not able to find whether Comm service is stopped or not so putting 5 seconds sleep (Approx)
	Sleep(5 * 1000);
	dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}
	//Issue: 1145: 	Issue with Reinstallation, while reinstallation Retry popup appears after clicking on "Retry" button installation get complete.
	//Resolved By: Nitin Kolapkar
	//Need to put sleep because In inno setup we are not able to find whether Comm service is stopped or not so putting 5 seconds sleep (Approx)
	Sleep(5 * 1000);
	//close and remove service here
	dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZUPDATESERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Update Service WardwizALUsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}

	dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Update Service WardwizALUsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************
Function Name  : CloseAllAplicationForElite
Description    : Function which closes Runing Application.
Author Name    : Jeena Mariam Saji
Date           : 14/02/2018
****************************************************************************/
bool CSetupDLLApp::CloseAllAplicationForElite(bool bIsReInstall)
{
	bool bISFullPath = false;
	if (_tcslen(m_csAppPath) > 0)
	{
		bISFullPath = true;
	}

	CString csModulePath = m_csAppPath;

	/*if (!GetCloseAction4OutlookIfRunning(bIsReInstall))
	{
		return false;
	}*/

	CEnumProcess objEnumProcess;

	////While Installation or Uninstalltion if crypt exe is running it should close that exe first
	////Neha Gharge 1st July 2015

	CStringArray objcsaWardWizProcesses;
	objcsaWardWizProcesses.Add(L"WRDWIZAVUI.EXE");
	objcsaWardWizProcesses.Add(L"VBUI.EXE");
	objcsaWardWizProcesses.Add(L"VBTRAY.EXE");
	objcsaWardWizProcesses.Add(L"VBAUTORUNSCN.EXE");
	objcsaWardWizProcesses.Add(L"VBTEMPCLR.EXE");
	objcsaWardWizProcesses.Add(L"VBUSBVAC.EXE");
	objcsaWardWizProcesses.Add(L"VBUSBDETECTUI.EXE");
	objcsaWardWizProcesses.Add(L"VBCRYPT.EXE");
	objcsaWardWizProcesses.Add(L"VBSCANNER.EXE");
	objcsaWardWizProcesses.Add(L"VBUI.EXE");
	//objcsaWardWizProcesses.Add(L"VBCOMMSRV.EXE");//Issue resolved: 0001145

	bool bIsAnyProcTerminated = false;
	bool bIsYesAllReply = false;
	CString csMessageContent(L"");
	DWORD dwRetMessage = 0x02;//Bu default YesAll option selected.

	for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
	{
		CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);

		if (objEnumProcess.IsProcessRunning(csProcessName, false, false, false))
		{
			if (!bIsYesAllReply)
			{
				switch (dwRetMessage)
				{
				case 0x01:bIsYesAllReply = false;
					//YES
					break;
				case 0x02:bIsYesAllReply = true;
					//YESALL
					break;
				case 0x03://NO
					return false;
					//break;
				case 0x04://NOALL
					return false;
					//break;
				case 0x05://CANCEL
					return false;
					//break;
				}
			}
			if (objEnumProcess.IsProcessRunning(csProcessName, true, false, false))
			{
				bIsAnyProcTerminated = true;
				AddLogEntry(L">>> %s was running, Terminated", csProcessName, 0, true, SECONDLEVEL);
			}
			else
			{
				bIsAnyProcTerminated = true;
				AddLogEntry(L"### %s was running, Failed to terminated", csProcessName, 0, true, SECONDLEVEL);
			}

		}
	}

	//Neha Gharge Unregister com dll 20/4/2015
	CString csAppPath(L"");
	csAppPath = m_csAppPath + L"VBSHELLEXT.DLL";
	if (PathFileExists(csAppPath))
	{
		UnregisterComDll(csAppPath);
	}

	csAppPath = csModulePath + L"VBSHELLEXT_OLD.DLL";
	if (PathFileExists(csAppPath))
	{
		UnregisterComDll(csAppPath);
	}

	//ISSUE NO - 8 After re- installation showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
	//NAME - Niranjan Deshak. - 29th Jan 2015.	
	if (bIsAnyProcTerminated)
	{
		CCommonFunctions objCCommonFunctions;
		objCCommonFunctions.RefreshTaskbarNotificationArea();
	}

	//Dwret is add into addlogentry So that we enable to know the problem while installation
	//Neha Gharge
	//close and remove service here
	CISpySrvMgmt		iSpySrvMgmtObj;
	DWORD dwRet = 0x00;
	CString csFailureCase(L"");
	dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZSERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}
	//Issue: 1145: 	Issue with Reinstallation, while reinstallation Retry popup appears after clicking on "Retry" button installation get complete.
	//Resolved By: Nitin Kolapkar
	//Need to put sleep because In inno setup we are not able to find whether Comm service is stopped or not so putting 5 seconds sleep (Approx)
	Sleep(5 * 1000);
	dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}
	//Issue: 1145: 	Issue with Reinstallation, while reinstallation Retry popup appears after clicking on "Retry" button installation get complete.
	//Resolved By: Nitin Kolapkar
	//Need to put sleep because In inno setup we are not able to find whether Comm service is stopped or not so putting 5 seconds sleep (Approx)
	Sleep(5 * 1000);
	//close and remove service here
	dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZUPDATESERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Update Service WardwizALUsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}

	dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Update Service WardwizALUsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}

	//close and remove service here
	dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZEPSCLIENTSERVNAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Update Service WardwizEPSCLIENTSERVICE with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}

	dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZEPSCLIENTSERVNAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Update Service WardwizEPSCLIENTSERVICE with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
  Function Name  : GetCloseAction4OutlookIfRunning
  Description    : Closing outloog If Running.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0004
  Date           : 7/25/2014
****************************************************************************/
bool CSetupDLLApp::GetCloseAction4OutlookIfRunning(bool bIsReInstall)
{
	//Varada Ikhar, Date:05/02/2015, Version:1.8.3.10
	//No need to show the prompt for outlook close if the product is BASIC/ESSENTIAL while installing or uninstalling.
	CITinRegWrapper objReg;
	DWORD dwProdID = 0;
	// need to rename the extension dll as it is loaded into explorer.exe
	/*if (bIsReInstall)
	{*/
		CString csSetupDllName = m_csAppPath + L"VBSHELLEXT.DLL";
		CString csRenameSetupDllName = m_csAppPath + L"VBSHELLEXT_OLD.DLL";
		if (!PathFileExists(csSetupDllName))
		{
			AddLogEntry(L"### VBSHELLEXT.DLL file is not present", 0, 0, true, SECONDLEVEL);
		}
		else
		{
			_wrename(csSetupDllName, csRenameSetupDllName);
		}
	//}

	if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"dwProductID", dwProdID) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwProductID from %s", theApp.g_csRegKeyPath, 0, true, SECONDLEVEL);
	}
	if(dwProdID != 0)
	{
		if(dwProdID == 1 || dwProdID == 4)
		{
			return true;
		}
	}
	else
	{
		if(m_csProductName == L"BASIC" || m_csProductName == L"ESSENTIAL")
		{
			return true;
		}
	}
		
	while(true)
	{
		CEnumProcess objEnumProcess;
		if(objEnumProcess.IsProcessRunning(L"OUTLOOK.EXE", false, false, false))
		{
			CString csMessage = L"";//L"Outlook application needs to be closed before";
			if (bIsReInstall)
			{	//Issue: While installing setup for first time if outlook is opened it gives pop-up"Outlook application needs to be closed before reinstalling WardWiz.Please close and click on Retry button." it should be "installing".
				//Resolved By: Nitin Kolapkar Date: 17th August 2015
				//csMessage += L" installing ";
				csMessage.Format(L"%s", m_csVectInputStrings.at(CM_2_WWSetupDllOutlookCloseInstall));
			}
			else
			{
				//csMessage += L" uninstalling ";
				csMessage.Format(L"%s", m_csVectInputStrings.at(CM_3_WWSetupDllOutlookCloseUnInstall));
			}
	
			//csMessage += L"WardWiz.\r\nPlease close and click on retry button.";
			
			int iRet = MessageBox(GetMainWnd()->m_hWnd, csMessage,
				L"Vibranium", MB_ICONEXCLAMATION | MB_RETRYCANCEL);
			if(iRet == IDCANCEL)
			{
				return false;
			}
		}
		else
		{
			break;
		}
		//wait for few second to close outlook. Even though it is not closed it will ask again.
		//Neha Gharge 
		Sleep(2 * 1000);
	}
	return true;
}

/***************************************************************************
  Function Name  : SetApplicationPath
  Description    : Extern Function, Set the path of the application.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0005
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT void SetApplicationPath(LPCTSTR szAppPath)
{
	if(szAppPath && *szAppPath)
	{
		if(_tcslen(szAppPath) > 0)
		{
			theApp.m_csAppPath = szAppPath;
		}
	}
}

/***************************************************************************
  Function Name  : InstallService
  Description    : Exported function Which Installs current process as a Services
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0006
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool InstallService(LPCTSTR szAppPath)
{
	TCHAR szFullPath[MAX_PATH] = {0};

	_tcscpy_s(szFullPath, _countof(szFullPath), szAppPath);
	_tcscat_s(szFullPath, _countof(szFullPath), L"\\ISPYCOMMSERVICE.EXE");
		
	CISpySrvMgmt		iSpySrvMgmtObj ;
	if( iSpySrvMgmtObj.InstallService(WARDWIZSERVICENAME, WARDWIZDISPLAYNAME, szFullPath) == 0x00)
	{
		return true;
	}
	return false;
}

/***************************************************************************
  Function Name  : StopServices
  Description    : Exported function Which stop ComServices current process as a Services
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool StopServices()
{
	TCHAR szFullPath[MAX_PATH] = {0};
		
	CISpySrvMgmt		iSpySrvMgmtObj ;

	// to close the tray during patch installation
	CEnumProcess objEnumProcess;

	if(objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", false, false, false))
	{
		int iRet = MessageBox(theApp.GetMainWnd()->m_hWnd, theApp.m_csVectInputStrings.at(CM_4_WWSetupDllInstallPatches), L"Vibranium", MB_ICONQUESTION | MB_YESNO);
		if(iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", true, false, false);
		}
		else
		{
			return false;
		}
	}
	if (objEnumProcess.IsProcessRunning(L"VBAUTORUNSCN.EXE", false, false, false))
	{
		int iRet = MessageBox(theApp.GetMainWnd()->m_hWnd, theApp.m_csVectInputStrings.at(CM_5_WWSetupDllClose), L"Vibranium", MB_ICONQUESTION | MB_YESNO);
		if (iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"VBAUTORUNSCN.EXE", true, false, false);
		}
		else
		{
			return false;
		}
	}

	if (objEnumProcess.IsProcessRunning(L"VBTEMPCLR.EXE", false, false, false))
	{
		int iRet = MessageBox(theApp.GetMainWnd()->m_hWnd, theApp.m_csVectInputStrings.at(CM_5_WWSetupDllClose), L"Vibranium", MB_ICONQUESTION | MB_YESNO);
		if (iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"VBTEMPCLR.EXE", true, false, false);
		}
		else
		{
			return false;
		}
	}

	if (objEnumProcess.IsProcessRunning(L"VBUSBVAC.EXE", false, false, false))
	{
		int iRet = MessageBox(theApp.GetMainWnd()->m_hWnd, theApp.m_csVectInputStrings.at(CM_5_WWSetupDllClose), L"Vibranium", MB_ICONQUESTION | MB_YESNO);
		if (iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"VBUSBVAC.EXE", true, false, false);
		}
		else
		{
			return false;
		}
	}

	if(objEnumProcess.IsProcessRunning(L"VBTRAY.EXE", false, false, false))
	{
	   objEnumProcess.IsProcessRunning(L"VBTRAY.EXE", true, false, false);
		//ISSUE NO - After installation of Patch Setup showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
		//NAME - Niranjan Deshak. - 10th Feb 2015.
		CCommonFunctions objCCommonFunctions;
		objCCommonFunctions.RefreshTaskbarNotificationArea();	
	}
  if(objEnumProcess.IsProcessRunning(L"VBALUSRV.EXE", false, false, false))
  {
	iSpySrvMgmtObj.StopServiceManually(WARDWIZUPDATESERVICENAME);  // to stop ALU service
  }

   if(objEnumProcess.IsProcessRunning(L"VBCOMMSRV.EXE", false, false, false))
   {
	iSpySrvMgmtObj.StopServiceManually(WARDWIZSERVICENAME);       // to stop Wardwiz COM service 
	
   }
   if (objEnumProcess.IsProcessRunning(L"WRDWIZCLIENTAGENT.EXE", false, false, false))
   {
	   iSpySrvMgmtObj.StopServiceManually(WARDWIZEPSCLIENTSERVNAME);       // to stop Wardwiz Client Agent 
   }
	 return true;
}


/***************************************************************************
  Function Name  : StartServices
  Description    : Exported function Which start ComServices current process as a Services
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool StartServices()
{
	CISpySrvMgmt		iSpySrvMgmtObj ;
// to start the tray during patch installation

	CITinRegWrapper objReg;
	TCHAR szValue[MAX_PATH] = {0};
	DWORD dwSize = sizeof(szValue);

	objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize);
	
	
	CString csAppPath = L""; //szValue + L"VBTRAY.EXE";

	csAppPath.Format(L"%s%s", szValue, L"VBTRAY.EXE");

	::ShellExecute(NULL, L"Open", csAppPath, L"-NOSPSCRN", NULL, 0);

	 iSpySrvMgmtObj.StartServiceManually(WARDWIZUPDATESERVICENAME); // to start ALU service
	
	if( iSpySrvMgmtObj.StartServiceManually(WARDWIZSERVICENAME) == 0x00)
	{
		return true;
	}
	return false;
}

/***************************************************************************
  Function Name  : InstallDriverService
  Description    : Exported function to install driver services
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           : 26-12-2014
****************************************************************************/
extern "C" DLLEXPORT int InstallDriverService()
{  
	int returnStatus = 0 ;
	try
	{
		CScannerLoad objScannerload;
		CSecure64  objPSecure;
		CWardWizOSversion objGetOSVersion;

		int iOsType = 0 ;

		TCHAR szPath[MAX_PATH] = {0};
		CString sSystem32Path = theApp.GetModuleFilePath();
		if (sSystem32Path != L"")
		{
			iOsType = objGetOSVersion.DetectClientOSVersion();

			if (iOsType == WINOS_XP)
			{
				returnStatus = objPSecure.InstallService(theApp.m_csAppPath + L"DRIVERS\\" + WRDWIZXPPROCSYS, iOsType);
				returnStatus = objScannerload.InstallScanner(theApp.m_csAppPath + L"DRIVERS\\" + WRDWIZSCANNERSYS);   // scanner
			}
			else  if (iOsType == WINOS_XP64) // xp64
			{
				//resolved by lalit issue:- secure64 bit registry key not getting deleted during uninstallation , process protection not works in xp64 bit
				//returnStatus = objPSecure.InstallService(sSystem32Path + L"\\DRIVERS\\" + WRDWIZXPPROCSYS, iOsType);     
				returnStatus = objScannerload.InstallScanner(theApp.m_csAppPath + L"DRIVERS\\" + WRDWIZSCANNERSYS);   // scanner
			}
			else
			{
				returnStatus = objPSecure.InstallService(theApp.m_csAppPath + L"DRIVERS\\" + WRDWIZSECURE64SYS, iOsType);     //Secure64
				returnStatus = objScannerload.InstallScanner(theApp.m_csAppPath + L"DRIVERS\\" + WRDWIZSCANNERSYS); // Scanner
			}
		}
		AddLogEntry(L">>> after scanner and secure64 installed", 0, 0, true, SECONDLEVEL);
		
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in InstallDriverService", 0, 0, true, SECONDLEVEL);
	}
	return returnStatus;
}

/***************************************************************************
  Function Name  : StartDriverServiceLocal
  Description    : Exported function  to start driver services
  Author Name    : lalit kumawat
  S.R. No        : 
  Date           : 26-12-2014
****************************************************************************/
extern "C" DLLEXPORT int StartDriverServiceLocal()
{
	CScannerLoad	 objScannerload;
	CSecure64   objPSecure;
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	int returnStatus = 0;
	CISpySrvMgmt		iSpySrvMgmtObj;
	TCHAR	szWrdWizScanSrvName[MAX_PATH] = { 0 };
	TCHAR	szWrdWizSecure64Name[MAX_PATH] = { 0 };
	TCHAR	szWardWizXpProcName[MAX_PATH] = { 0 };

	try
	{
		theApp.SendMsgToTroubleShooter(SENDMSGSTARTDRIVER);
		swprintf_s(szWrdWizScanSrvName, _countof(szWrdWizScanSrvName), L"%s", csDScannerServiceName);
		swprintf_s(szWrdWizSecure64Name, _countof(szWrdWizSecure64Name), L"%s", csDSecure64ServiceName);
		swprintf_s(szWardWizXpProcName, _countof(szWardWizXpProcName), L"%s", csDXpProcServiceName);

		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == WINOS_XP)
		{
			if (objPSecure.IsSecure64DriverRunning(iOsType))
			{
				iSpySrvMgmtObj.StopServiceManually(szWardWizXpProcName);
			}

			if (objScannerload.IsScannerDriverRunning())
			{
				iSpySrvMgmtObj.StopServiceManually(szWrdWizScanSrvName);
			}

			returnStatus = objScannerload.StartServiceLocal();   // scanner
			returnStatus = objPSecure.StartServiceLocal(iOsType);     //Secure64

		}
		else if (iOsType == WINOS_XP64) // xp64
			{
			  
			   if (objScannerload.IsScannerDriverRunning())
				{
				iSpySrvMgmtObj.StopServiceManually(szWrdWizScanSrvName);
				}


				returnStatus = objScannerload.StartServiceLocal();   // scanner	
			}
		else
		{
			if (objPSecure.IsSecure64DriverRunning(iOsType))
			{
				iSpySrvMgmtObj.StopServiceManually(szWrdWizSecure64Name);
			}
			if (objScannerload.IsScannerDriverRunning())
			{
				iSpySrvMgmtObj.StopServiceManually(szWrdWizScanSrvName);
			}
			// first secure64 then scanner
			returnStatus = objPSecure.StartServiceLocal(iOsType);     //Secure64
			returnStatus = objScannerload.StartServiceLocal(); // Scanner
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartDriverServiceLocal", 0, 0, true, SECONDLEVEL);
	}
	return returnStatus;
}

/***************************************************************************
  Function Name  : RemoveDriverService
  Description    : Exported function to remove proction services during uninstall
  Author Name    : Lalit kumawat
  S.R. No        : 
  Date           : 26-12-2014
****************************************************************************/
extern "C" DLLEXPORT int RemoveDriverService()
{

	CScannerLoad objScannerload;
	CSecure64  objPSecure;
	int returnStatus = 0;
	CWardWizOSversion objGetOSVersion;

	int OsType = 0;

	try
	{

		OsType = objGetOSVersion.DetectClientOSVersion();
		if (OsType == WINOS_XP)
		{
			returnStatus = objPSecure.RemoveService(OsType);     //Secure64
			returnStatus = objScannerload.RemoveService();   // scanner
		}
		else
			if (OsType == WINOS_XP64)
			{
				returnStatus = objScannerload.RemoveService();   // scanner
			}
			else
			{
				returnStatus = objPSecure.RemoveService(OsType);     //Secure64
				returnStatus = objScannerload.RemoveService(); // Scanner

			}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RemoveDriverService", 0, 0, true, SECONDLEVEL);
	}

	return returnStatus;
}

/***************************************************************************
  Function Name  : RemoveService
  Description    : Exported fucntion which Removes current process Services.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0007
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool RemoveService()
{
	CISpySrvMgmt		iSpySrvMgmtObj ;
	DWORD dwReturn = iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME);
	if( dwReturn != 0x00)
	{
		AddLogEntry(L"### Failed to remove WardwizComsrv service", 0, 0, true, SECONDLEVEL);
	}

	dwReturn = iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME);
	if( dwReturn != 0x00)
	{
		AddLogEntry(L"### Failed to remove WardwizUpdatesrv service", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************
  Function Name  : GetCloseActionBeforeOutlookIfRunning
  Description    : Closing Action Before outlook running.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0008
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool GetCloseActionBeforeOutlookIfRunning()
{
	try
	{
		return true;// theApp.GetCloseAction4OutlookIfRunning();
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CloseAllApplication", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************
  Function Name  : CloseAllApplication
  Description    : Extern Function .
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0009
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool CloseAllApplication()
{
	try
	{
		return theApp.CloseAllAplication(true);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CloseAllApplication", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : CloseAllApplicationforElite
Description    : Extern Function .
Author Name    : Jeena Mariam Saji
S.R. No        :
Date           : 14/02/2018
****************************************************************************/
extern "C" DLLEXPORT bool CloseAllApplicationforElite()
{
	try
	{
		return theApp.CloseAllAplicationForElite(true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CloseAllApplicationforElite", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : CloseAllApplicationForUninstall
Description    : Extern Function .
Author Name    : Neha Gharge 
S.R. No        : 
Date           : 5/23/2015
****************************************************************************/
extern "C" DLLEXPORT bool CloseAllApplicationForUninstall(LPCTSTR szAppPath)
{
	try
	{
		TCHAR szAllUserPath[256] = { 0 };
		TCHAR szIniFilePath[512] = { 0 };

		if (szAppPath && *szAppPath)
		{
			if (_tcslen(szAppPath) > 0)
			{
				theApp.m_csAppPath = szAppPath;
			}
		}
		theApp.m_bUninstallation = true;

		// Issue no 1130 Neha Gharge When uninstation done after updating a product and not restart the machine.
		ZeroMemory(szAllUserPath, sizeof(szAllUserPath));

		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 255);

		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\Vibranium\\ALUDel.ini", szAllUserPath);

		if (PathFileExists(szIniFilePath))
		{
			CString csMsgToRestart;
			csMsgToRestart.Format(L"%s\n\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_SETUPDLL_UPDATE_RESTART_REQ"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REBOOT_DELETEREPAIR_PART3"));

			if (MessageBox(theApp.GetMainWnd()->m_hWnd, csMsgToRestart, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				theApp.RestartNow();
				return false;
			}
			else
			{
				return false;
			}
		}
		//issue: 0000746 While uninstalling setup if outlook is opened,pop-up appears "Outlook application needs to be closed before reinstalling WardWiz".Instead of reinstalling it should be uninstalling.
		// resolved by Nitin Kolapkar, Date 21st July 2015
		return theApp.CloseAllAplication(false);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CloseAllApplicationForUninstall", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
/***************************************************************************************************                    
*  Function Name  : ParseVersionString                                                    
*  Description    : To parse CString version strings and store it in Array of integers.
*  Author Name    : Niranjan Deshak
*  SR_NO
*  Date           : 01 jan 2015
****************************************************************************************************/
bool CSetupDLLApp::ParseVersionString(int iDigits[4], CString& csVersion)
{
	try
	{
		int iTokenPos = 0;
		csVersion.Insert(0,_T("."));
		CString csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
		int iVersion = _ttoi(csToken);
		int iSubVersion=0;
		int iCount = 0;

		iDigits[iCount] = iVersion;
		iCount++;
		while((!csToken.IsEmpty()) && (iCount<=3) )
		{
			csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
			iSubVersion = _ttoi(csToken);
			iDigits[iCount] = iSubVersion;
			iCount++;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::ParseVersionString", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : CompareVersions                                                    
*  Description    : To compare two products versions.
*  Author Name    : Niranjan Deshak
*  SR_NO		  :
*  Date           : 01 jan 2015
****************************************************************************************************/
int CSetupDLLApp::CompareVersions(int iVersion1[4], int iVersion2[4])
{
	try
	{
		for (int iIndex = 0; iIndex < 4; ++iIndex)
		{
			if (iVersion1[iIndex] < iVersion2[iIndex])
				return -1;
			if (iVersion1[iIndex] > iVersion2[iIndex])
				return 1;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CompareVersions", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
  Function Name  : CheckForPreviousVersion
  Description    : Extern Function. 
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0010
  Date           : 7/25/2014
  // Issue No : 1160 Added a flag to check whether it is a patch or setup 
****************************************************************************/
//Resolved Issue No. 24 While installing the same setup again it should be "Latest version of Wardwiz is already installed".- Niranjan Deshak.
		//-Niranjan Deshak. -21/12/2014. Argument added : LPCTSTR CurrentVersion
extern "C" DLLEXPORT bool CheckForPreviousVersion(LPCTSTR szAppPath,LPCTSTR CurrentVersion, bool IsPatch)
{
	try
	{
		bool bPreviousVersionFound = false;

		//Set here the Product Name came from setup
		theApp.m_csProductName.Format(L"%s", szAppPath);

		CITinRegWrapper objReg;
		TCHAR szValue[MAX_PATH] = {0};
		//Resolved Issue No. 24 While installing the same setup again it should be "Latest version of Wardwiz is already installed".- Niranjan Deshak.
		//-Niranjan Deshak. -21/12/2014.
		TCHAR szCurrentVersion[MAX_PATH] = {0};
		DWORD dwSize = sizeof(szValue);
		
		TCHAR szValueAppVersion[MAX_PATH] = {0};
		DWORD dwSizeAppVersion = sizeof(szValueAppVersion);
		
		if(objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\iTin Antivirus", L"AppFolder", szValue, dwSize) == 0)
		{
			MessageBox(theApp.GetMainWnd()->m_hWnd, L"iTin Antivirus is installed on your machine\n Please uninstall it before installing Vibranium.", L"Vibranium", MB_ICONINFORMATION | MB_OK );
			return false;
		}

		if(objReg.GetRegistryValueData32(HKEY_LOCAL_MACHINE, L"SOFTWARE\\iTin Antivirus", L"AppFolder", szValue,dwSize) == 0)
		{
			MessageBox(theApp.GetMainWnd()->m_hWnd, L"32 bit version of iTin Antivirus is installed on your machine\n Please uninstall it before installing Vibranium.", L"Vibranium", MB_ICONINFORMATION | MB_OK );
			return false;
		}

#ifndef _WIN32
		if(objReg.GetRegistryValueData32(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue,dwSize) == 0)
		{
			MessageBox(theApp.GetMainWnd()->m_hWnd, L"32 bit version of Vibranium is installed on your machine\n Please uninstall 32 bit setup before installing Vibranium 64 bit setup.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK );
			return false;
		}
#endif

		//Need to check here the product version, we need to uninstall other wardwiz product version
		//before installing any warwiz product.
		CString csProduct = L"";
		TCHAR szProductName[MAX_PATH] = {0};
		DWORD dwProdID = 0;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"dwProductID", dwProdID) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwProductID from %s", theApp.g_csRegKeyPath, 0, true, SECONDLEVEL);;
			CString strOldAppEntry = L"";
			strOldAppEntry.Format(L"%s Antivirus", theApp.g_csRegKeyPath);
			if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, strOldAppEntry.GetBuffer(), L"dwProductID", dwProdID) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwProductID from %s", strOldAppEntry, 0, true, SECONDLEVEL);;
			}
		}
		switch(dwProdID)
		{
			case ESSENTIAL:
				_tcscpy_s(szProductName, _countof(szProductName), L"ESSENTIAL");
			   csProduct = L"WardWiz Essential";
			   break;
		   case ESSENTIALPLUS:
				_tcscpy_s(szProductName, _countof(szProductName), L"ESSENTIALPLUS");
				csProduct = L"WardWiz Essential Plus";
				break;
			case PRO:
				_tcscpy_s(szProductName, _countof(szProductName), L"PRO");
			   csProduct = L"WardWiz Pro";
			   break;
			case ELITE:
				_tcscpy_s(szProductName, _countof(szProductName), L"ELITE");
			   csProduct = L"WardWiz Elite";
			   theApp.m_bIsEliteVersion = true;
			   break;
			case BASIC:
				_tcscpy_s(szProductName, _countof(szProductName), L"BASIC");
			   csProduct = L"WardWiz Basic";
			   break;
		}
		
		if(csProduct.GetLength() != 0)
		{
			if(_tcscmp(szProductName, szAppPath) != 0)
			{
				CString csMessage;
				csMessage.Format(theApp.m_csVectInputStrings.at(CM_6_WWSetupDllUnInstallContinue), csProduct);
				MessageBox(theApp.GetMainWnd()->m_hWnd, csMessage, L"Vibranium", MB_ICONINFORMATION | MB_OK );
				return false;
			}
		}
		//Niranjan Deshak. -31/12/2014.
		//Resolved Issue No. 24 While installing the same setup again it should be "Latest version of Wardwiz is already installed".
		DWORD dwAppKey = objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppVersion", szValueAppVersion, dwSizeAppVersion);
		if(dwAppKey == 0)
		{
			CString csCurrentVersion(CurrentVersion);
			CString csValueAppVersion(szValueAppVersion);

			int iCurrentVersion[4]={0}, iAppVersion[4]={0};
			if ((theApp.ParseVersionString(iAppVersion, csValueAppVersion)) && (theApp.ParseVersionString(iCurrentVersion, csCurrentVersion)))
			{
				int iRes = theApp.CompareVersions(iAppVersion, iCurrentVersion);
				if(  iRes == 1)
				{
					MessageBox(theApp.GetMainWnd()->m_hWnd, theApp.m_csVectInputStrings.at(CM_7_WWSetupDllLatestInstalled), L"Vibranium", MB_ICONINFORMATION);
					return false;
				}
				else if( iRes == 0 ||iRes == -1)
				{
					if (IsPatch == false)
					{
						if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize) == 0)
						{
							if(theApp.m_bIsEliteVersion)
							{
								return true;
							}
							else
							{
								int iRet = MessageBox(theApp.GetMainWnd()->m_hWnd, theApp.m_csVectInputStrings.at(CM_8_WWSetupDllLatestInstalled), L"Vibranium", MB_ICONQUESTION | MB_YESNO);
								if (iRet != IDYES)
								{
									return false;
								}
							}
						}
					}
				}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception CheckForPreviousVersion", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
  Function Name  : StartStartUpApplications
  Description    : Extern function.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0011
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool StartStartUpApplications(LPCTSTR szAppPath)
{
	return theApp.StartStartUpApplications(szAppPath);
}

/***************************************************************************
Function Name  : StartEPSStartUpApplications
Description    : Extern function.
Author Name    : Ramkrushna Shelke
S.R. No        : WRDWIZSETUPDLL_0011
Date           : 15/02/2018
****************************************************************************/
extern "C" DLLEXPORT bool StartEPSStartUpApplications(LPCTSTR szAppPath)
{
	return theApp.StartEPSStartUpApplications(szAppPath);
}

/***************************************************************************
  Function Name  : SetAttributesToFolder
  Description    : Set the atributes to folder.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0012
  Date           : 7/25/2014
****************************************************************************/
bool CSetupDLLApp::SetAttributesToFolder(CString path) 
{
	if(path.GetLength() == 0)
	{
		return false;
	}

    PACL pDacl,pNewDACL;
    EXPLICIT_ACCESS ExplicitAccess;
    PSECURITY_DESCRIPTOR ppSecurityDescriptor;
    PSID psid;

    LPTSTR lpStr;
    CString str = path;
    lpStr = str.GetBuffer();

    GetNamedSecurityInfo(lpStr, SE_FILE_OBJECT,DACL_SECURITY_INFORMATION, NULL, NULL, &pDacl, NULL, &ppSecurityDescriptor);
    ConvertStringSidToSid(L"S-1-1-0", &psid);

    ExplicitAccess.grfAccessMode = SET_ACCESS;
    ExplicitAccess.grfAccessPermissions = GENERIC_ALL;
    ExplicitAccess.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    ExplicitAccess.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ExplicitAccess.Trustee.pMultipleTrustee = NULL;
    ExplicitAccess.Trustee.ptstrName = (LPTSTR) psid;
    ExplicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess.Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;

    SetEntriesInAcl(1, &ExplicitAccess, pDacl, &pNewDACL);
    SetNamedSecurityInfo(lpStr,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL,NULL,pNewDACL,NULL);

    LocalFree(pNewDACL);
    LocalFree(psid);

    str.ReleaseBuffer();

	return true;
}

/***************************************************************************
  Function Name  : StartStartUpApplications
  Description    : Start the Start up Applications.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0013
  Date           : 7/25/2014
****************************************************************************/
bool CSetupDLLApp::StartStartUpApplications(CString csAppFolder) 
{
  	
	CITinRegWrapper objReg;
	TCHAR szValue[MAX_PATH] = {0};
	DWORD dwSize = sizeof(szValue);
	objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize);
	CString csAppPathValue = L""; //szValue + L"VBTRAY.EXE";
	csAppPathValue.Format(L"%s", szValue);
	CString csFullPath = csAppPathValue;
	
	//CString csFullPath = csAppFolder;

	if(!SetAttributesToFolder(csFullPath))
	{
		AddLogEntry(L"### Failed to set all user attribute to path %s", csFullPath, 0, true, SECONDLEVEL);
	}

	csFullPath += L"VBCOMMSRV.EXE";
	CISpySrvMgmt		iSpySrvMgmtObj ;
	/*if( !iSpySrvMgmtObj.InstallService(WARDWIZSERVICENAME, WARDWIZDISPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
		AddLogEntry(L"### Error in InstallService %s ", WARDWIZSERVICENAME, 0, true, SECONDLEVEL);
	}*/
	
	if (!iSpySrvMgmtObj.CreateUserService(WARDWIZSERVICENAME, WARDWIZDISPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
		AddLogEntry(L"### Error in InstallService %s ", WARDWIZSERVICENAME, 0, true, SECONDLEVEL);
	}

	csFullPath = csAppPathValue;
	csFullPath += L"VBALUSRV.EXE";
	/*if( !iSpySrvMgmtObj.InstallService(WARDWIZUPDATESERVICENAME, WARDWIZUPDATEDISPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
		AddLogEntry(L"### Error in InstallService %s ", WARDWIZSERVICENAME, 0, true, SECONDLEVEL);
	}*/
	
	if (!iSpySrvMgmtObj.CreateUserService(WARDWIZUPDATESERVICENAME, WARDWIZUPDATEDISPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
		AddLogEntry(L"### Error in InstallService %s ", WARDWIZSERVICENAME, 0, true, SECONDLEVEL);
	}

	//Issue: After launching Main UI And Tray Custom scan Drag drop not working properly, If we close UI and Start again from Desktop then It works Resolved by : Nitin K Date : 23rd April 2015
	//TCHAR szDatebuff[9];
	//_wstrdate_s(szDatebuff,9);
	//CString csAppPath = csAppPathValue + L"VBTRAY.EXE";;
	//::ShellExecute(NULL, L"Open", csAppPath, L"-NOSPSCRN", NULL, 0);
	CString csExePath, csCommandLine;
	csExePath.Format(L"%s%s", csAppPathValue, L"VBTRAY.EXE");
	
	csCommandLine.Format(L"%s", L"-NOSPSCRN");
	CExecuteProcess objEnumprocess;
	objEnumprocess.StartProcessWithTokenExplorerWait(csExePath, csCommandLine, L"explorer.exe");
	AddLogEntry(L"###  Lauch Tray using Executeprocess", 0, 0, true, SECONDLEVEL);
	

	return true;
}

/***************************************************************************
Function Name  : StartStartUpApplications
Description    : Start the Start up Applications.
Author Name    : Ramkrushna Shelke
S.R. No        : WRDWIZSETUPDLL_0013
Date           : 15/02/2018
****************************************************************************/
bool CSetupDLLApp::StartEPSStartUpApplications(CString csAppFolder)
{
	CITinRegWrapper objReg;
	TCHAR szValue[MAX_PATH] = { 0 };
	DWORD dwSize = sizeof(szValue);
	objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize);
	CString csAppPathValue = L""; //szValue + L"VBTRAY.EXE";
	csAppPathValue.Format(L"%s", szValue);
	CString csFullPath = csAppPathValue;

	//CString csFullPath = csAppFolder;

	if (!SetAttributesToFolder(csFullPath))
	{
		AddLogEntry(L"### Failed to set all user attribute to path %s", csFullPath, 0, true, SECONDLEVEL);
	}

	csFullPath += L"VBCOMMSRV.EXE";
	CISpySrvMgmt		iSpySrvMgmtObj;
	/*if( !iSpySrvMgmtObj.InstallService(WARDWIZSERVICENAME, WARDWIZDISPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
	AddLogEntry(L"### Error in InstallService %s ", WARDWIZSERVICENAME, 0, true, SECONDLEVEL);
	}*/

	if (!iSpySrvMgmtObj.CreateUserService(WARDWIZSERVICENAME, WARDWIZDISPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
		AddLogEntry(L"### Error in InstallService %s ", WARDWIZSERVICENAME, 0, true, SECONDLEVEL);
	}

	csFullPath = csAppPathValue;
	csFullPath += L"VBALUSRV.EXE";
	/*if( !iSpySrvMgmtObj.InstallService(WARDWIZUPDATESERVICENAME, WARDWIZUPDATEDISPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
	AddLogEntry(L"### Error in InstallService %s ", WARDWIZSERVICENAME, 0, true, SECONDLEVEL);
	}*/

	if (!iSpySrvMgmtObj.CreateUserService(WARDWIZUPDATESERVICENAME, WARDWIZUPDATEDISPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
		AddLogEntry(L"### Error in InstallService %s ", WARDWIZSERVICENAME, 0, true, SECONDLEVEL);
	}

	csFullPath = csAppPathValue;
	csFullPath += L"WRDWIZCLIENTAGENT.EXE";
	if (!iSpySrvMgmtObj.CreateUserService(WARDWIZEPSCLIENTSERVNAME, WARDWIZEPSCLIENTSERVPLAYNAME, csFullPath.GetBuffer()) == 0x00)
	{
		AddLogEntry(L"### Error in InstallService %s ", WARDWIZEPSCLIENTSERVNAME, 0, true, SECONDLEVEL);
	}

	/*
	if (!iSpySrvMgmtObj.InstallService(WARDWIZEPSCLIENTSERVNAME, WARDWIZEPSCLIENTSERVPLAYNAME, csFullPath.GetBuffer(), SERVICE_AUTO_START, L"", L"NT AUTHORITY\\LocalService", NULL) == 0x00)
	{
		AddLogEntry(L"### Error in InstallService %s ", WARDWIZEPSCLIENTSERVNAME, 0, true, SECONDLEVEL);
	}*/

	//Issue: After launching Main UI And Tray Custom scan Drag drop not working properly, If we close UI and Start again from Desktop then It works Resolved by : Nitin K Date : 23rd April 2015
	//TCHAR szDatebuff[9];
	//_wstrdate_s(szDatebuff,9);
	//CString csAppPath = csAppPathValue + L"VBTRAY.EXE";;
	//::ShellExecute(NULL, L"Open", csAppPath, L"-NOSPSCRN", NULL, 0);
	CString csExePath, csCommandLine;
	csExePath.Format(L"%s%s", csAppPathValue, L"VBTRAY.EXE");

	csCommandLine.Format(L"%s", L"-NOSPSCRN");
	CExecuteProcess objEnumprocess;
	objEnumprocess.StartProcessWithTokenExplorerWait(csExePath, csCommandLine, L"explorer.exe");
	AddLogEntry(L"###  Lauch Tray using Executeprocess", 0, 0, true, SECONDLEVEL);
	return true;
}

/***************************************************************************
  Function Name  : SetSelectedLanguage
  Description    : Exported function to set the languages selected from setup.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0014
  Date           : 05th May 2014
****************************************************************************/
extern "C" DLLEXPORT bool SetSelectedLanguage(LPTSTR szLanguageName)
{
	return theApp.SetSelectedLanguage(szLanguageName);
}

/***************************************************************************
  Function Name  : SetSelectedLanguage
  Description    : Function which set selected language from user selection from setup language
				   selection pop up.
				   ENGLISH =	0,
				   HINDI	=	1,
				   GERMAN	=	2,
				   CHINESE	=	3,
				   SPANISH	=	4,
				   FRENCH	=	5,
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0015
  Date           : 05th May 2014
****************************************************************************/
bool CSetupDLLApp::SetSelectedLanguage(LPTSTR szLanguageName)
{
	DWORD dwLangID = 0;
	
	CString csLanguage;
	csLanguage.Format(L"%s", szLanguageName);

	csLanguage.MakeLower();
	if( csLanguage.CompareNoCase( L"english") == 0 )
	{
		dwLangID = 0;
	}
	else if( csLanguage.CompareNoCase( L"hindi") == 0 )
	{
		dwLangID = 1;
	}
	else if( csLanguage.CompareNoCase( L"german") == 0 )
	{
		dwLangID = 2;
	}
	else if( csLanguage.CompareNoCase( L"chinese") == 0 )
	{
		dwLangID = 3;
	}
	else if( csLanguage.CompareNoCase( L"spanish") == 0 )
	{
		dwLangID = 4;
	}
	else if( csLanguage.CompareNoCase( L"french") == 0 )
	{
		dwLangID = 5;
	}
	else
	{
		return false; //INVALID CASE
	}

	CString csLangID = L"0";
	csLangID.Format(L"%d", dwLangID);
	
	CString csFilePath  = m_csAppPath;
	csFilePath += L"VBSETTINGS\\PRODUCTSETTINGS.INI";
	return (WritePrivateProfileString(L"VBSETTINGS", L"LanguageID", csLangID, csFilePath) != 0);
}

/***************************************************************************
  Function Name  : GetOSVersion
  Description    : Extern function.
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT int GetOSVersion()
{
	try
	{
		CWardWizOSversion objGetOSVersion;
		return static_cast<int>(objGetOSVersion.DetectClientOSVersion());
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in GetOSVersion", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
  Function Name  : StartStartUpApplications
  Description    : Extern function.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZSETUPDLL_0011
  Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool SingleInstanceCheck()
{
	return theApp.SingleInstanceCheck();
}

/***************************************************************************************************                    
*  Function Name  : SingleInstanceCheck                                                     
*  Description    : Check whether wardwiz instance is exist or not
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 sep 2013
****************************************************************************************************/
bool CSetupDLLApp::SingleInstanceCheck()
{
	HANDLE hHandle = CreateMutex(NULL, TRUE, L"{8D45F4F7-91DC-4138-B786-1CA027F2BC40}");
	DWORD dwError = GetLastError();
	if(dwError == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

/***************************************************************************
Function Name  : RegisterSetupWithDrivers
Description    : to register setup with protection drivers
Author Name    : Lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
extern "C" DLLEXPORT void RegisterSetupWithDrivers()
{
	// TODO: Add your control notification handler code here
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	bool returnStatus = false;

	try
	{

		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == WINOS_XP)
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.RegisterProcessId(WLSRV_ID_TEN); // Issue resolved: 0001288, duplicate ID's was present
			}

			if (scannerObj.IsScannerDriverRunning())
			{
				scannerObj.RegisterProcessId(WLSRV_ID_TEN); // Issue resolved: 0001288, duplicate ID's was present
			}


		}
		else
			if (iOsType == WINOS_XP64)
			{

				if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
				{
					cSecure64Obj.RegisterProcessId(WLSRV_ID_TEN); // Issue resolved: 0001288, duplicate ID's was present
				}

			}
			else
			{
				if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
				{
					cSecure64Obj.RegisterProcessId(WLSRV_ID_TEN); // Issue resolved: 0001288, duplicate ID's was present
				}

				if (scannerObj.IsScannerDriverRunning())
				{
					scannerObj.RegisterProcessId(WLSRV_ID_TEN); // Issue resolved: 0001288, duplicate ID's was present
				}
				
			}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RegisterSetupWithDrivers", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : PauseProtectionDrivers
Description    : to pause the driver protection  so setup can re-install setup
Author Name    : Lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
extern "C" DLLEXPORT void PauseProtectionDrivers()
{
	// TODO: Add your control notification handler code here
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	bool returnStatus = false;

	try
	{

		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == WINOS_XP)
		{
			scannerObj.PauseDriverProtection(0);
			cSecure64Obj.PauseDriverProtection(0);

		}
		else
			if (iOsType == WINOS_XP64)
			{

				scannerObj.PauseDriverProtection(0);
			}
			else
			{
				scannerObj.PauseDriverProtection(0);
				cSecure64Obj.PauseDriverProtection(0);
			}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in PauseProtectionDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : ResumeProtectionDrivers
Description    : to resume the driver after setup installation
Author Name    : Lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
extern "C" DLLEXPORT void ResumeProtectionDrivers()
{
	// TODO: Add your control notification handler code here
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	bool returnStatus = false;

	try
	{
		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == WINOS_XP)
		{
			scannerObj.PauseDriverProtection(1);
			cSecure64Obj.PauseDriverProtection(1);
		}
		else
			if (iOsType == WINOS_XP64)
			{
				scannerObj.PauseDriverProtection(1);
			}
			else
			{
				scannerObj.PauseDriverProtection(1);
				cSecure64Obj.PauseDriverProtection(1);

			}
		theApp.SendMsgToTroubleShooter(SENDMSGINSTALLCOMPLETED);
		//theApp.SendMsgToTroubleShooter(SENDMSGCLOSETLB);
		Sleep(100);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ResumeProtectionDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : StartProductServices
Description    : to start services
Author Name    : Lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
extern "C" DLLEXPORT bool StartProductServices()
{
	CISpySrvMgmt		iSpySrvMgmtObj;
	// to start the tray during patch installation

	iSpySrvMgmtObj.StartServiceManually(WARDWIZUPDATESERVICENAME); // to start ALU service

	if (iSpySrvMgmtObj.StartServiceManually(WARDWIZSERVICENAME) == 0x00)
	{
		return true;
	}
	return false;
}

/***************************************************************************
Function Name  : StartProductServices
Description    : to start services
Author Name    : Ram Shelke
S.R. No        :
Date           : 15/02/2018
****************************************************************************/
extern "C" DLLEXPORT bool StartEPSProductServices()
{
	CISpySrvMgmt		iSpySrvMgmtObj;
	
	// to start the tray during patch installation
	iSpySrvMgmtObj.StartServiceManually(WARDWIZUPDATESERVICENAME); // to start ALU service
	iSpySrvMgmtObj.StartServiceManually(WARDWIZSERVICENAME);
	iSpySrvMgmtObj.StartServiceManually(WARDWIZEPSCLIENTSERVNAME);

	return true;
}

/***************************************************************************
Function Name  : CheckNeedOfRestart
Description    : Export the function checks fro previous version
Author Name    : Neha Gharge
Date           : 23th March 2015
****************************************************************************/
extern "C" DLLEXPORT bool CheckNeedOfRestart()
{
	try
	{
		return theApp.CheckNeedOfRestart();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CheckNeedOfRestart", 0, 0, true, SECONDLEVEL);
		return false;
	}
}

/***************************************************************************
Function Name  : CheckNeedOfRestart
Description    : checks fro previous version.
Author Name    : Neha Gharge
Date           : 23th March 2015
****************************************************************************/
bool CSetupDLLApp::CheckNeedOfRestart()
{
	CITinRegWrapper objReg;
	TCHAR szValue[MAX_PATH] = { 0 };
	DWORD dwSize = sizeof(szValue);

	try
	{
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.g_csRegKeyPath.GetBuffer(), L"AppFolder", szValue, dwSize) == 0)
		{
			//Issue: 0000216: On Reinstallation Restart Now message should be within Setup window and not as MessageBox
			//Resolved By: Nitin K. Date: 28th April 2015
			CString csSetupDllName = m_csAppPath + L"VBSHELLEXT.DLL";
			if (PathFileExists(csSetupDllName))
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CheckNeedOfRestart", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/***************************************************************************
Function Name  : RestartNow
Description    : Export the function if user click yes on restart popup then it restart immediately.
Author Name    : Neha Gharge
Date           : 23th March 2015
****************************************************************************/
extern "C" DLLEXPORT bool RestartNow()
{
	try
	{
		return theApp.RestartNow();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RestartNow", 0, 0, true, SECONDLEVEL);
		return false;
	}
}

/***************************************************************************
Function Name  : RestartNow
Description    : if user click yes on restart popup then it restart immediately.
Author Name    : Neha Gharge
Date           : 23th March 2015
****************************************************************************/
bool CSetupDLLApp::RestartNow()
{
	try
	{
		CEnumProcess objenumproc;
		objenumproc.RebootSystem();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RestartNow", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/***************************************************************************
Function Name  : UnregisterComDll
Description    : Unregister COM Dll
Author Name    : Neha Gharge
S.R. No        :
Date           : 3/28/2015
****************************************************************************/
void CSetupDLLApp::UnregisterComDll(CString csAppPath)
{
	try
	{
		CWardWizOSversion		objOSVersionWrap;
		CString csExePath, csCommandLine;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		csExePath.Format(L"%s\\%s", systemDirPath, L"regsvr32.exe");

		//On xp runas parameter never work It will not unregister the VBSHELLEXT.DLL
		//So NUll parameter send.
		DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
		//Neha Gharge Message box showing of register successful on reinstallation.
		switch (OSType)
		{
		case WINOS_XP:
		case WINOS_XP64:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, NULL, csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		default:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		}

		if (PathFileExists(csAppPath))
		{
			if (!DeleteFile(csAppPath))
			{
				AddLogEntry(L"### DeleteFile Failed for file: %s .", csAppPath, 0, true, SECONDLEVEL);
                if (!MoveFileEx(csAppPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
				{
					AddLogEntry(L"### MoveFileEx Failed for file: %s.", csAppPath, 0, true, SECONDLEVEL);
				}												
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UnRegisterComDLL", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : MoveFileEXtoRestartDlt
Description    : it provides functionality to delete setup dll and app directory at system restart.
Author Name    : Lalit kumawat
Date           : 5-22-2015
****************************************************************************/
extern "C" DLLEXPORT bool MoveFileEXtoRestartDlt(LPCTSTR szAppPath)
{
	try
	{
		if (szAppPath && *szAppPath)
		{
			if (_tcslen(szAppPath) > 0)
			{
				theApp.m_csAppPath = szAppPath;
			}
		}
		return theApp.MoveFileEXtoRestartDlt();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MoveFileEXtoRestartDlt", 0, 0, true, SECONDLEVEL);
		return false;
	}
}

/***************************************************************************
Function Name  : MoveFileEXtoRestartDlt
Description    : it provides functionality to delete setup dll and app directory at system restart.
Author Name    : Lalit kumawat
Date           : 5-22-2015
****************************************************************************/
bool CSetupDLLApp::MoveFileEXtoRestartDlt()
{
	try
	{
		// when setup dll is loaded by exploarer.
		CString csSetupDllPath(L"");
		csSetupDllPath = m_csAppPath + L"VBSETUPDLL.dll";

		if (!MoveFileEx(csSetupDllPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
		{
			AddLogEntry(L"### MoveFileEx Failed for file: %s.", csSetupDllPath, 0, true, SECONDLEVEL);
		}

		CString csAppDirectoryPath = L"";
		csAppDirectoryPath = m_csAppPath.Left(m_csAppPath.GetAllocLength() - 1);

		// to delete app folder at window restart.
		if (!MoveFileEx(csAppDirectoryPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
		{
			AddLogEntry(L"### MoveFileEx Failed for file: %s.", csAppDirectoryPath, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RestartNow", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
Function Name  : GetOptionsYesNOYESALLNOALLMessageBox
Description    : Get option to show yes, yesall, No, Noall options
Author Name    : Neha Gharge
Date           : 3-july-2015
****************************************************************************/
BOOL CSetupDLLApp::GetOptionsYesNOYESALLNOALLMessageBox(CString& strText,
	UINT *pnButton,
	UINT *pnDefButton,
	UINT *pnIconType,
	UINT *pnDontAsk,
	UINT *pnHelpId,
	CString& strCustomButtons,
	int  *pnTimeout,
	int  *pnDisabled,
	UINT * pnIdIcon)
{
	try
	{

		strCustomButtons = _T("");

		*pnButton = (MB_YESNOCANCEL | MB_YESTOALL | MB_NOTOALL | MB_SETFOREGROUND | MB_TOPMOST);

		*pnIconType = 0;
		*pnIdIcon = 0;

		//Issue no: 1192 Question Mark icon is not display
		*pnIconType = MB_ICONQUESTION | MB_ICONMASK;

		*pnDefButton = MB_DEFBUTTON2 | MB_ICONQUESTION | MB_ICONMASK;

		strText = _T("Test");

		*pnTimeout = _ttoi(L"0");

		*pnDisabled = _ttoi(L"0");
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CSetupDLLApp::GetOptionsYesNOYESALLNOALLMessageBox", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : ShowYesAllNoAllMessagebox
Description    : show yes, yesall, No, Noall Message box
Author Name    : Neha Gharge
Date           : 3-july-2015
****************************************************************************/
DWORD CSetupDLLApp::ShowYesAllNoAllMessagebox(CString cMessageContent)
{
	CString strText, strCustomButtons;
	strText.Empty();
	UINT nButton, nDefButton, nIconType, nDontAsk, nHelpContextId;
	int nTimeout = 0;
	int nDisabled = 0;
	UINT nIdIcon = 0;

	GetOptionsYesNOYESALLNOALLMessageBox(strText, &nButton, &nDefButton, &nIconType, &nDontAsk,
		&nHelpContextId, strCustomButtons, &nTimeout, &nDisabled, &nIdIcon);

	int rc = 0;

	XMSGBOXPARAMS xmb;
	
	//----------------------------------------
	CString strButtonCaption = L"";
	int iTotalSize = m_csVectInputStrings.size();
	if (iTotalSize)
	{
		xmb.bUseUserDefinedButtonCaptions = true;
		strButtonCaption.Format(L"%s", m_csVectInputStrings.at(CM_9_WWSetupDllCustomMsgBoxButtonYes));
		_tcscpy_s(xmb.UserDefinedButtonCaptions.szYes, _countof(xmb.UserDefinedButtonCaptions.szYes), strButtonCaption);
		strButtonCaption.Format(L"%s", m_csVectInputStrings.at(CM_10_WWSetupDllCustomMsgBoxButtonYesToAll));
		_tcscpy_s(xmb.UserDefinedButtonCaptions.szYesToAll, _countof(xmb.UserDefinedButtonCaptions.szYesToAll), strButtonCaption);
		strButtonCaption.Format(L"%s", m_csVectInputStrings.at(CM_11_WWSetupDllCustomMsgBoxButtonNo));
		_tcscpy_s(xmb.UserDefinedButtonCaptions.szNo, _countof(xmb.UserDefinedButtonCaptions.szNo), strButtonCaption);
		strButtonCaption.Format(L"%s", m_csVectInputStrings.at(CM_12_WWSetupDllCustomMsgBoxButtonNoToAll));
		_tcscpy_s(xmb.UserDefinedButtonCaptions.szNoToAll, _countof(xmb.UserDefinedButtonCaptions.szNoToAll), strButtonCaption);
		strButtonCaption.Format(L"%s", m_csVectInputStrings.at(CM_13_WWSetupDllCustomMsgBoxButtonCancel));
		_tcscpy_s(xmb.UserDefinedButtonCaptions.szCancel, _countof(xmb.UserDefinedButtonCaptions.szCancel), strButtonCaption);
	}
	
	xmb.nTimeoutSeconds = nTimeout;
	xmb.nDisabledSeconds = nDisabled;
	xmb.nIdIcon = 0;
	xmb.dwReportUserData = 1234;
	xmb.nIdHelp = nHelpContextId;
	xmb.lpszModule = _T(__FILE__);
	xmb.nLine = 12345;
	_tcscpy_s(xmb.szCompanyName, _countof(xmb.szCompanyName), _T("Vibranium"));

	_tcscpy_s(xmb.szReportButtonCaption, _countof(xmb.szReportButtonCaption), _T("Message Box"));
	if (!strCustomButtons.IsEmpty())
		_tcscpy_s(xmb.szCustomButtons, _countof(xmb.szCustomButtons), strCustomButtons);

	strText = cMessageContent;
	rc = ::XMessageBox(GetMainWnd()->m_hWnd, strText, _T("Vibranium"),
		nDefButton | nButton , &xmb);

	CString str = _T("");
	DWORD dwMessageValue =0x00;
	//int rc = rc & 0xFF;

	switch (rc)
	{
	case IDYES:			dwMessageValue = 0x01; break;
	case IDYESTOALL:	dwMessageValue = 0x02; break;
	case IDNO:			dwMessageValue = 0x03; break;
	case IDNOTOALL:		dwMessageValue = 0x04; break;
	case IDCANCEL:		dwMessageValue = 0x05; break;
	default:			str.Format(_T("Unknown result:")); break;
	}
	return dwMessageValue;
}

/***************************************************************************
Function Name  : ProtectFolder
Description    : to protect any selected folder
Author Name    : Lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
extern "C" DLLEXPORT void ProtectFolder(CString appPath)
{
	TCHAR CommonAppPath[MAX_PATH] = { 0 };
	CString csActualPath = NULL;
	CString csConvertedPath = NULL;
	TCHAR CommonAppPathWithDosDriveFormat[MAX_PATH] = { 0 };
	CScannerLoad scannerObj;
	try
	{
		swprintf_s(CommonAppPath, _countof(CommonAppPath), L"%s", appPath);
		csActualPath = (CString)CommonAppPath;
		csConvertedPath = scannerObj.GetPathToDriveVolume(csActualPath);
		if (!csActualPath.IsEmpty())
		{
			if (!(GetFileAttributes(csActualPath) == INVALID_FILE_ATTRIBUTES))
			{
				swprintf_s(CommonAppPathWithDosDriveFormat, _countof(CommonAppPathWithDosDriveFormat), L"%s", csConvertedPath);
				if (!scannerObj.ProtectFolderSEH(CommonAppPathWithDosDriveFormat))
				{
					AddLogEntry(L"### Failed to Protect tFolder %s", CommonAppPathWithDosDriveFormat, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ProtectFolder", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : SendAllStrings
Description    : Extern function. for for Setup installation messagebox strings
Author Name    : Nitin K
S.R. No        : 
Date           : 
****************************************************************************/
extern "C" DLLEXPORT bool SendAllStrings(LPCTSTR szAppPath)
{
	return theApp.SendAllStrings(szAppPath);
}

/***************************************************************************
Function Name  : SendAllStrings
Description    : Set Strings for Setup installation messagebox 
Author Name    : Nitin K
S.R. No        : 
Date           : 
****************************************************************************/
bool CSetupDLLApp::SendAllStrings(CString csStrings)
{
	int iSize = m_csVectInputStrings.size();
	m_csVectInputStrings.push_back(csStrings);
	int iSizeAfterPushBack = m_csVectInputStrings.size();
	if ( iSizeAfterPushBack > iSize )
		return true; 
	else
		return false;
}
/***************************************************************************
Function Name  : CreateMutexForTroubleshooter
Description    : Create mutex for installation of driver
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
extern "C" DLLEXPORT bool CreateMutexForTroubleshooter()
{
	try
	{
		theApp.SendMsgToTroubleShooter(SENDMSGINSTALLATIONDRIVER);
		HANDLE hHandle = CreateMutex(NULL, TRUE, L"{E7E0EC4A-DE70-409C-8405-33F19CB0B74F}");
		DWORD dwError = GetLastError();
		if (dwError == ERROR_ALREADY_EXISTS)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CreateMutexForTroubleshooter", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : SendMsgToTroubleShooter(DWORD dwMsg)
Description    : Send message to troubleshooter EXE
Author Name    : Neha Gharge
S.R. No        :
Date           : 31/10/2015
****************************************************************************/
bool CSetupDLLApp::SendMsgToTroubleShooter(DWORD dwMsg)
{
	__try
	{
		SendData2TLB(SHOWSTATUSOFINSTALLATIONDRIVER, dwMsg);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::SendMsgToTroubleShooter", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : SendData2UI
Description    : Send Data to UI
Author Name    : Neha Gharge
Date           : 31/10/2015

***********************************************************************************************/
bool CSetupDLLApp::SendData2TLB(int iMessageInfo, DWORD dwMsg,bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = iMessageInfo;
	szPipeData.dwValue = dwMsg;
	bool bUIExists = true;
	CISpyCommunicator objComUI(WWIZ_TROUBLESHOOT_SERVER);

	if (!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		bUIExists = false;
		AddLogEntry(L"### Failed to send Data in CSetupDLLApp::SendData2UI", 0, 0, true, SECONDLEVEL);
	}

	if (bWait && bUIExists)
	{
		if (!objComUI.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CSystemTray::SendData2UI", 0, 0, true, SECONDLEVEL);
		}
		if (szPipeData.dwValue == 0)
		{
			return false;
		}
	}
	return true;
}

/***************************************************************************
Function Name  : UnloadFltDrivers
Description    : Function to load minifilter driver.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 07th Jun 2016
****************************************************************************/
extern "C" DLLEXPORT bool StartFltDrivers(LPTSTR lpszDriverName)
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.StartDriverService(lpszDriverName);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in LoadFltDrivers, Driver Name: %s", lpszDriverName, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : LoadFltDrivers
Description    : Function to unload minifilter driver.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 07th Jun 2016
****************************************************************************/
extern "C" DLLEXPORT bool StopFltDrivers(LPTSTR lpszDriverName)
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.StopDriverService(lpszDriverName);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in UnloadFltDrivers, Driver Name: %s", lpszDriverName, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : StopDriverService
Description    : Function to stop driver service.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 08th Jun 2016
****************************************************************************/
bool CSetupDLLApp::StopDriverService(LPTSTR lpszDriverName)
{
	bool bReturn = false;
	try
	{
		CISpySrvMgmt		iSpySrvMgmtObj;
		DWORD				dwRet = 0x00;
		CString				csFailureCase(L"");

		dwRet = iSpySrvMgmtObj.StopServiceManually(lpszDriverName);
		csFailureCase.Format(L"%d", dwRet);
		if (dwRet != 0x00)
		{
			AddLogEntry(L"### Unable to Stop Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
			return bReturn;
		}
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::StopDriverService, Driver Name: %s", lpszDriverName, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : StartDriverService
Description    : Function to start driver service
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 08th Jun 2016
****************************************************************************/
bool CSetupDLLApp::StartDriverService(LPTSTR lpszDriverName)
{
	bool bReturn = false;
	try
	{
		CISpySrvMgmt		iSpySrvMgmtObj;
		DWORD				dwRet = 0x00;
		CString				csFailureCase(L"");

		dwRet = iSpySrvMgmtObj.StartServiceManually(lpszDriverName);
		csFailureCase.Format(L"%d", dwRet);
		if (dwRet != 0x00)
		{
			AddLogEntry(L"### Unable to Start Service WardwizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
			return bReturn;
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::StartDriverService, Driver Name: %s", lpszDriverName, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : SetApplicationPath
Description    : Extern Function, Set the path of the application.
Author Name    : Ramkrushna Shelke
S.R. No        : WRDWIZSETUPDLL_0005
Date           : 7/25/2014
****************************************************************************/
extern "C" DLLEXPORT bool CreateSystemRestorePoint()
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.CreateWardWizRestorePoint()?true:false;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CreateSystemRestorePoint", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : InitializeCOMSecurity
Description    : Function to Initialize COM.
Author Name    : Ramkrushna Shelke
S.R. No        : 
Date           : 08th Mar 2017
****************************************************************************/
BOOL CSetupDLLApp::InitializeCOMSecurity()
{
	ACL        *pAcl = NULL;
	BOOL       fRet = FALSE;
	__try
	{
		// Create the security descriptor explicitly as follows because
		// CoInitializeSecurity() will not accept the relative security descriptors  
		// returned by ConvertStringSecurityDescriptorToSecurityDescriptor().

		SECURITY_DESCRIPTOR securityDesc = { 0 };
		EXPLICIT_ACCESS   ea[5] = { 0 };
		ULONGLONG  rgSidBA[(SECURITY_MAX_SID_SIZE + sizeof(ULONGLONG) - 1) / sizeof(ULONGLONG)] = { 0 };
		ULONGLONG  rgSidLS[(SECURITY_MAX_SID_SIZE + sizeof(ULONGLONG) - 1) / sizeof(ULONGLONG)] = { 0 };
		ULONGLONG  rgSidNS[(SECURITY_MAX_SID_SIZE + sizeof(ULONGLONG) - 1) / sizeof(ULONGLONG)] = { 0 };
		ULONGLONG  rgSidPS[(SECURITY_MAX_SID_SIZE + sizeof(ULONGLONG) - 1) / sizeof(ULONGLONG)] = { 0 };
		ULONGLONG  rgSidSY[(SECURITY_MAX_SID_SIZE + sizeof(ULONGLONG) - 1) / sizeof(ULONGLONG)] = { 0 };
		DWORD      cbSid = 0;
		DWORD      dwRet = ERROR_SUCCESS;
		HRESULT    hrRet = S_OK;

		//
		// This creates a security descriptor that is equivalent to the following 
		// security descriptor definition language (SDDL) string:
		//
		//   O:BAG:BAD:(A;;0x1;;;LS)(A;;0x1;;;NS)(A;;0x1;;;PS)(A;;0x1;;;SY)(A;;0x1;;;BA)
		//

		// Initialize the security descriptor.
		fRet = ::InitializeSecurityDescriptor(&securityDesc, SECURITY_DESCRIPTOR_REVISION);
		if (!fRet)
		{
			goto exit;
		}

		// Create an administrator group security identifier (SID).
		cbSid = sizeof(rgSidBA);
		fRet = ::CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, rgSidBA, &cbSid);
		if (!fRet)
		{
			goto exit;
		}

		// Create a local service security identifier (SID).
		cbSid = sizeof(rgSidLS);
		fRet = ::CreateWellKnownSid(WinLocalServiceSid, NULL, rgSidLS, &cbSid);
		if (!fRet)
		{
			goto exit;
		}

		// Create a network service security identifier (SID).
		cbSid = sizeof(rgSidNS);
		fRet = ::CreateWellKnownSid(WinNetworkServiceSid, NULL, rgSidNS, &cbSid);
		if (!fRet)
		{
			goto exit;
		}

		// Create a personal account security identifier (SID).
		cbSid = sizeof(rgSidPS);
		fRet = ::CreateWellKnownSid(WinSelfSid, NULL, rgSidPS, &cbSid);
		if (!fRet)
		{
			goto exit;
		}

		// Create a local service security identifier (SID).
		cbSid = sizeof(rgSidSY);
		fRet = ::CreateWellKnownSid(WinLocalSystemSid, NULL, rgSidSY, &cbSid);
		if (!fRet)
		{
			goto exit;
		}

		// Setup the access control entries (ACE) for COM. You may need to modify 
		// the access permissions for your application. COM_RIGHTS_EXECUTE and
		// COM_RIGHTS_EXECUTE_LOCAL are the minimum access rights required.

		ea[0].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
		ea[0].grfAccessMode = SET_ACCESS;
		ea[0].grfInheritance = NO_INHERITANCE;
		ea[0].Trustee.pMultipleTrustee = NULL;
		ea[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
		ea[0].Trustee.ptstrName = (LPTSTR)rgSidBA;

		ea[1].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
		ea[1].grfAccessMode = SET_ACCESS;
		ea[1].grfInheritance = NO_INHERITANCE;
		ea[1].Trustee.pMultipleTrustee = NULL;
		ea[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
		ea[1].Trustee.ptstrName = (LPTSTR)rgSidLS;

		ea[2].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
		ea[2].grfAccessMode = SET_ACCESS;
		ea[2].grfInheritance = NO_INHERITANCE;
		ea[2].Trustee.pMultipleTrustee = NULL;
		ea[2].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
		ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
		ea[2].Trustee.ptstrName = (LPTSTR)rgSidNS;

		ea[3].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
		ea[3].grfAccessMode = SET_ACCESS;
		ea[3].grfInheritance = NO_INHERITANCE;
		ea[3].Trustee.pMultipleTrustee = NULL;
		ea[3].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
		ea[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[3].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
		ea[3].Trustee.ptstrName = (LPTSTR)rgSidPS;

		ea[4].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
		ea[4].grfAccessMode = SET_ACCESS;
		ea[4].grfInheritance = NO_INHERITANCE;
		ea[4].Trustee.pMultipleTrustee = NULL;
		ea[4].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
		ea[4].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[4].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
		ea[4].Trustee.ptstrName = (LPTSTR)rgSidSY;

		// Create an access control list (ACL) using this ACE list.
		dwRet = ::SetEntriesInAcl(ARRAYSIZE(ea), ea, NULL, &pAcl);
		if (dwRet != ERROR_SUCCESS || pAcl == NULL)
		{
			fRet = FALSE;
			goto exit;
		}

		// Set the security descriptor owner to Administrators.
		fRet = ::SetSecurityDescriptorOwner(&securityDesc, rgSidBA, FALSE);
		if (!fRet)
		{
			goto exit;
		}

		// Set the security descriptor group to Administrators.
		fRet = ::SetSecurityDescriptorGroup(&securityDesc, rgSidBA, FALSE);
		if (!fRet)
		{
			goto exit;
		}

		// Set the discretionary access control list (DACL) to the ACL.
		fRet = ::SetSecurityDescriptorDacl(&securityDesc, TRUE, pAcl, FALSE);
		if (!fRet)
		{
			goto exit;
		}

		// Initialize COM. You may need to modify the parameters of
		// CoInitializeSecurity() for your application. Note that an
		// explicit security descriptor is being passed down.

		hrRet = ::CoInitializeSecurity(&securityDesc,
			-1,
			NULL,
			NULL,
			RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
			RPC_C_IMP_LEVEL_IDENTIFY,
			NULL,
			EOAC_DISABLE_AAA | EOAC_NO_CUSTOM_MARSHAL,
			NULL);
		if (FAILED(hrRet))
		{
			fRet = FALSE;
			goto exit;
		}

		fRet = TRUE;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CreateSystemRestorePoint", 0, 0, true, SECONDLEVEL);
	}
exit:
	::LocalFree(pAcl);
	return fRet;
}

/***************************************************************************
Function Name  : CreateWardWizRestorePoint
Description    : Function to create system restore point.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 08th Mar 2017
****************************************************************************/
BOOL CSetupDLLApp::CreateWardWizRestorePoint()
{
	BOOL fRet = FALSE;
	HMODULE hSrClient = NULL;
	try
	{
		RESTOREPOINTINFOW RestorePtInfo;
		STATEMGRSTATUS SMgrStatus;
		PFN_SETRESTOREPTW fnSRSetRestorePointW = NULL;
		DWORD dwErr = ERROR_SUCCESS;
		HRESULT hr = S_OK;
		CString csLog;

		//hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		hr = CoInitialize(NULL);
		if (FAILED(hr))
		{
			csLog.Format(L"Unexpected error: CoInitializeEx() failed with 0x%08x", hr);
			AddLogEntry(L"### %s", csLog, 0, true, SECONDLEVEL);
			goto exit;
		}

		// Initialize COM security to enable NetworkService,
		// LocalService and System to make callbacks to the process 
		// calling  System Restore. This is required for any process
		// that calls SRSetRestorePoint.

		fRet = InitializeCOMSecurity();
		if (!fRet)
		{
			AddLogEntry(L"### Unexpected error: failed to initialize COM security", 0, 0, true, SECONDLEVEL);
			goto exit;
		}

		// Initialize the RESTOREPOINTINFO structure
		RestorePtInfo.dwEventType = BEGIN_SYSTEM_CHANGE;

		// Notify the system that changes are about to be made.
		// An application is to be installed.
		RestorePtInfo.dwRestorePtType = APPLICATION_INSTALL;

		// RestPtInfo.llSequenceNumber must be 0 when creating a restore point.
		RestorePtInfo.llSequenceNumber = 0;

		// String to be displayed by System Restore for this restore point.
		StringCbCopyW(RestorePtInfo.szDescription,
			sizeof(RestorePtInfo.szDescription),
			L"WardWiz Restore Point");

		// Load the DLL, which may not exist on Windows server
		hSrClient = LoadLibraryW(L"srclient.dll");
		if (NULL == hSrClient)
		{
			AddLogEntry(L"### System Restore is not present.", 0, 0, true, SECONDLEVEL);
			goto exit;
		}

		// If the library is loaded, find the entry point
		fnSRSetRestorePointW = (PFN_SETRESTOREPTW)GetProcAddress(
			hSrClient, "SRSetRestorePointW");
		if (NULL == fnSRSetRestorePointW)
		{
			AddLogEntry(L"### Failed to find SRSetRestorePointW in CSetupDLLApp::CreateWardwizRestorePoint", 0, 0, true, SECONDLEVEL);
			goto exit;
		}

		fRet = fnSRSetRestorePointW(&RestorePtInfo, &SMgrStatus);
		if (!fRet)
		{
			dwErr = SMgrStatus.nStatus;
			if (dwErr == ERROR_SERVICE_DISABLED)
			{
				AddLogEntry(L"### System Restore is turned off.", 0, 0, true, SECONDLEVEL);
				goto exit;
			}

			csLog.Format(L"Failure to create the restore point; error=%u.", dwErr);
			AddLogEntry(L"### %s", csLog, 0, true, SECONDLEVEL);
			goto exit;
		}

		csLog.Format(L"Restore point created; number=%I64d.\n", SMgrStatus.llSequenceNumber);
		AddLogEntry(L">>> %s", csLog, 0, true, SECONDLEVEL);


		// The application performs some installation operations here.

		// It is not necessary to call SrSetRestorePoint to indicate that the 
		// installation is complete except in the case of ending a nested 
		// restore point. Every BEGIN_NESTED_SYSTEM_CHANGE must have a 
		// corresponding END_NESTED_SYSTEM_CHANGE or the application cannot 
		// create new restore points.

		// Update the RESTOREPOINTINFO structure to notify the 
		// system that the operation is finished.
		RestorePtInfo.dwEventType = END_SYSTEM_CHANGE;

		// End the system change by using the sequence number 
		// received from the first call to SRSetRestorePoint.
		RestorePtInfo.llSequenceNumber = SMgrStatus.llSequenceNumber;

		// Notify the system that the operation is done and that this
		// is the end of the restore point.
		fRet = fnSRSetRestorePointW(&RestorePtInfo, &SMgrStatus);
		if (!fRet)
		{
			dwErr = SMgrStatus.nStatus;
			csLog.Format(L"Failure to end the restore point; error=%u.", dwErr);
			AddLogEntry(L"### %s", csLog, 0, true, SECONDLEVEL);
			goto exit;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CreateWardwizRestorePoint", 0, 0, true, SECONDLEVEL);
	}
exit:

	if (hSrClient != NULL)
	{
		FreeLibrary(hSrClient);
		hSrClient = NULL;
	}

	return 0;
}

extern "C" DLLEXPORT bool WriteXMLValIntoINI(LPCTSTR NodeName, LPCTSTR NodeVal)
{
	__try
	{
		return theApp.WriteXMLValIntoINI(NodeName, NodeVal);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WriteXMLValIntoINI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

bool CSetupDLLApp::WriteXMLValIntoINI(LPCTSTR NodeName, LPCTSTR NodeVal)
{
	try
	{
		CString csFilePath = theApp.m_csAppPath;
		csFilePath += L"VBSETTINGS\\EPSSettings.ini";

		CString csNodeName;
		csNodeName.Format(L"%s", NodeName);
		CString csNodeVal;
		csNodeVal.Format(L"%s", NodeVal);

		if (csNodeName == L"ClientPassword")
		{
			CString	csWardWizModulePath = GetWardWizPathFromRegistry();
			CString	csWardWizReportsPath = L"";
			csWardWizReportsPath.Format(L"%sVBFEATURESLOCK.DB", csWardWizModulePath);

			if (PathFileExists(csWardWizReportsPath))
			{
				SetFileAttributes(csWardWizReportsPath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(csWardWizReportsPath);
			}

			CString csInsertQuery = _T("create table IF NOT EXISTS WWIZFEATURESACCESSPWD(VARFEATUREPWD NVARCHAR(256))");
			CT2A ascii(csInsertQuery, CP_UTF8);
			theApp.InsertDataToTable(ascii.m_psz);

			CString csScheduleInsertQuery = L"INSERT INTO WWIZFEATURESACCESSPWD VALUES (null,";
			csScheduleInsertQuery.Format(L"INSERT INTO WWIZFEATURESACCESSPWD(VARFEATUREPWD) values('%s')", csNodeVal);
			CT2A asScheduleInser(csScheduleInsertQuery, CP_UTF8);
			theApp.InsertDataToTable(asScheduleInser);

			CString csCreateQueryFeature = _T("create table IF NOT EXISTS WWIZFEATURESACCESS(ID INTEGER PRIMARY KEY, VARFEATURE NVARCHAR(256), BACCESSVAL BOOL, MODULE_ID INTEGER)");
			CT2A asciiEPS(csCreateQueryFeature, CP_UTF8);
			theApp.InsertDataToTable(asciiEPS.m_psz);

			CString csQuickScan;
			csQuickScan.Format(L"Quick Scan");
			CString csQuickScanVal;
			csQuickScanVal.Format(L"1");
			CString csQuickScanID;
			csQuickScanID.Format(L"1");
			CString csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csQuickScan, csQuickScanVal, csQuickScanID);
			CT2A asFeatureInsert(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsert);

			CString csFullScan;
			csFullScan.Format(L"Full Scan");
			CString csFullScanVal;
			csFullScanVal.Format(L"1");
			CString csFullScanID;
			csFullScanID.Format(L"2");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csFullScan, csFullScanVal, csFullScanID);
			CT2A asFeatureInsertFull(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertFull);

			CString csCustomScan;
			csCustomScan.Format(L"Custom Scan");
			CString csCustomScanVal;
			csCustomScanVal.Format(L"1");
			CString csCustomScanID;
			csCustomScanID.Format(L"3");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csCustomScan, csCustomScanVal, csCustomScanID);
			CT2A asFeatureInsertCustom(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertCustom);

			CString csBootTimeScan;
			csBootTimeScan.Format(L"BootTime Scan");
			CString csBootTimeScanVal;
			csBootTimeScanVal.Format(L"1");
			CString csBootTimeScanID;
			csBootTimeScanID.Format(L"4");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csBootTimeScan, csBootTimeScanVal, csBootTimeScanID);
			CT2A asFeatureInsertBoot(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertBoot);

			CString csAntirootKitScan;
			csAntirootKitScan.Format(L"AntiRootKit Scan");
			CString csAntirootKitScanVal;
			csAntirootKitScanVal.Format(L"1");
			CString csAntirootKitScanID;
			csAntirootKitScanID.Format(L"5");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csAntirootKitScan, csAntirootKitScanVal, csAntirootKitScanID);
			CT2A asFeatureInsertAntiRoot(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertAntiRoot);

			CString csScheduledScan;
			csScheduledScan.Format(L"Scheduled Scan");
			CString csScheduledScanVal;
			csScheduledScanVal.Format(L"1"); 
			CString csScheduledScanID;
			csScheduledScanID.Format(L"6");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csScheduledScan, csScheduledScanVal, csScheduledScanID);
			CT2A asFeatureInsertSched(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertSched);

			CString csDataEncryption;
			csDataEncryption.Format(L"Data Encryption");
			CString csDataEncryptionVal;
			csDataEncryptionVal.Format(L"1");
			CString csDataEncryptionID;
			csDataEncryptionID.Format(L"7");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csDataEncryption, csDataEncryptionVal, csDataEncryptionID);
			CT2A asFeatureInsertDataEnc(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertDataEnc);

			CString csRegistryOpt;
			csRegistryOpt.Format(L"Registry Optimizer");
			CString csRegistryOptVal;
			csRegistryOptVal.Format(L"1");
			CString csRegistryOptID;
			csRegistryOptID.Format(L"8");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csRegistryOpt, csRegistryOptVal, csRegistryOptID);
			CT2A asFeatureInsertRegOpt(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertRegOpt);

			CString csRecoverFiles;
			csRecoverFiles.Format(L"Recover Files");
			CString csRecoverFilesVal;
			csRecoverFilesVal.Format(L"1");
			CString csRecoverFilesID;
			csRecoverFilesID.Format(L"9");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csRecoverFiles, csRecoverFilesVal, csRecoverFilesID);
			CT2A asFeatureInsertRecover(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertRecover);

			CString csAutorunScan;
			csAutorunScan.Format(L"Autorun Scan");
			CString csAutorunScanVal;
			csAutorunScanVal.Format(L"1");
			CString csAutorunScanID;
			csAutorunScanID.Format(L"10");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csAutorunScan, csAutorunScanVal, csAutorunScanID);
			CT2A asFeatureInsertAutorun(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertAutorun);

			CString csTempFileCleaner;
			csTempFileCleaner.Format(L"Temp File Cleaner");
			CString csTempFileCleanerVal;
			csTempFileCleanerVal.Format(L"1");
			CString csTempFileCleanerID;
			csTempFileCleanerID.Format(L"11");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csTempFileCleaner, csTempFileCleanerVal, csTempFileCleanerID);
			CT2A asFeatureInsertTemp(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertTemp);

			CString csUSBVaccin;
			csUSBVaccin.Format(L"USB Vaccination");
			CString csUSBVaccinVal;
			csUSBVaccinVal.Format(L"1");
			CString csUSBVaccinID;
			csUSBVaccinID.Format(L"12");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csUSBVaccin, csUSBVaccinVal, csUSBVaccinID);
			CT2A asFeatureInsertUSB(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertUSB);

			CString csSettings;
			csSettings.Format(L"Settings");
			CString csSettingsVal;
			csSettingsVal.Format(L"1");
			CString csSettingsID;
			csSettingsID.Format(L"13");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csSettings, csSettingsVal, csSettingsID);
			CT2A asFeatureInsertSettings(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertSettings);

			CString csUninstall;
			csUninstall.Format(L"Uninstall");
			CString csUninstallVal;
			csUninstallVal.Format(L"1");
			CString csUninstallID;
			csUninstallID.Format(L"14");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csUninstall, csUninstallVal, csUninstallID);
			CT2A asFeatureInsertUninstall(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertUninstall);
		
			CString csFirewall;
			csFirewall.Format(L"Firewall");
			CString csFirewallVal;
			csFirewallVal.Format(L"1");
			CString csFirewallID;
			csFirewallID.Format(L"15");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csFirewall, csFirewallVal, csFirewallID);
			CT2A asFeatureInsertFirewall(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertFirewall);
			
			CString csParControl;
			csParControl.Format(L"Parental Control");
			CString csParControlVal;
			csParControlVal.Format(L"1");
			CString csParControlID;
			csParControlID.Format(L"16");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csParControl, csParControlVal, csParControlID);
			CT2A asFeatureInsertParControl(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertParControl);
			
			CString csEmailScan;
			csEmailScan.Format(L"Email Scan");
			CString csEmailScanVal;
			csEmailScanVal.Format(L"1");
			CString csEmailScanID;
			csEmailScanID.Format(L"17");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csEmailScan, csEmailScanVal, csEmailScanID);
			CT2A asFeatureInsertEmailScan(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertEmailScan);
			
			CString csFolderLocker;
			csFolderLocker.Format(L"Folder Locker");
			CString csFolderLockerVal;
			csFolderLockerVal.Format(L"1");
			CString csFolderLockerID;
			csFolderLockerID.Format(L"18");
			csInsertQueryEPS = L"INSERT INTO WWIZFEATURESACCESS VALUES (null,";
			csInsertQueryEPS.Format(L"INSERT INTO WWIZFEATURESACCESS(VARFEATURE,  BACCESSVAL, MODULE_ID) values('%s', '%s', '%s')", csFolderLocker, csFolderLockerVal, csFolderLockerID);
			CT2A asFeatureInsertFoldLocker(csInsertQueryEPS, CP_UTF8);
			theApp.InsertDataToTable(asFeatureInsertFoldLocker);

			//Don't change the order of the features. 
		}
		else
		{
			WritePrivateProfileString(L"VBSETTINGS", csNodeName, csNodeVal, csFilePath);
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::WriteXMLValIntoINI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

INT64 CSetupDLLApp::InsertDataToTable(const char* szQuery)
{
	CWardWizSQLiteDatabase			objSqlDb;
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBFEATURESLOCK.DB", csWardWizModulePath);

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		objSqlDb.SetDataBaseFilePath(dbPath.m_psz);

		objSqlDb.Open();

		int iRows = objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = objSqlDb.GetLastRowId();

		objSqlDb.Close();

		return iLastRowId;
	}
	catch (CWardwizSQLiteException& e)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertDataToTable, ErrorMessage: %s", CA2T(e.errorMessage()), 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		objSqlDb.Close();
		AddLogEntry(L"### Exception in CWardwizClientAgent::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
	return 0;
}

/***************************************************************************
Function Name  : GetIPAddress
Description    : This Function Find the Current IP Address of Machine.
Author Name    : Tejas Tanaji Shinde
Date           : 12th July 2018
****************************************************************************/
extern "C" DLLEXPORT bool GetIPAddress(LPTSTR strHostName)         
{
	bool bReturn = false;
	try
	{
		if (!strHostName)
			return bReturn;

		CSocketComm m_SocketManager;
		TCHAR szIPAdddress[0x30];
		m_SocketManager.GetLocalAddress(szIPAdddress, 0x30);
		_tcscpy(strHostName, szIPAdddress);
		bReturn = true;
	}
	catch (...)
	{	
		AddLogEntry(L"### Excpetion in GetIPAddress", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : GetLocalName
Description    : This Function Find the Current Local Computer Name.
Author Name    : Tejas Tanaji Shinde
Date           : 12th July 2018
****************************************************************************/
bool  CSetupDLLApp::GetLocalName(LPTSTR strName, UINT nSize)
{
	try
	{
		if (strName != NULL && nSize > 0)
		{
			char strHost[HOSTNAME_SIZE] = { 0 };

			// get host name, if fail, SetLastError is set
			if (SOCKET_ERROR != gethostname(strHost, sizeof(strHost)))
			{
				struct hostent* hp;
				hp = gethostbyname(strHost);
				if (hp != NULL) {
					strncpy(strHost, hp->h_name, HOSTNAME_SIZE);
				}

				// check if user provide enough buffer
				if (strlen(strHost) > nSize)
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
					return false;
				}

				// Unicode conversion
				#ifdef _UNICODE
					return (0 != MultiByteToWideChar(CP_ACP, 0, strHost, -1, strName, nSize));
				#else
					_tcscpy(strName, strHost);
					return true;
				#endif
			}
		}
		else
		{
			SetLastError(ERROR_INVALID_PARAMETER);
		}
	}
	catch (...)
	{	
		AddLogEntry(L"### Excpetion in GetLocalName", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/***************************************************************************
Function Name  : UninstallOtherAV
Description    : This Function is used to uninstall other AV while installing WardWiz.
Author Name    : Amol Jaware
Date           : 18 Oct 2018
****************************************************************************/
/*extern "C" DLLEXPORT bool UninstallOtherAV(LPCTSTR szAppPath)
{
	CITinRegWrapper objReg;
	try
	{
		if (theApp.GetOtherAVDetails(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall")))
			return true;
		else
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in UninstallOtherAV", 0, 0, true, SECONDLEVEL);
	}
	return true;
}*/

/***************************************************************************
Function Name  : GetOtherAVDetails
Description    : This Function is used to uninstall other AV from x64 bit while installing WardWiz.
Author Name    : Amol Jaware
Date           : 18 Oct 2018
****************************************************************************/
/*DWORD CSetupDLLApp::GetOtherAVDetails(LPCTSTR lpszSubKey)
{
	GetRegOtherAVValues();
	DWORD	dwRet = 0x00;
	HKEY	hKey = NULL, hSubKey = NULL;
	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00;
	TCHAR	szSubKeyName[512] = { 0 };
	TCHAR	szPublName[0x64] = { 0 };
	TCHAR	szUninstallStr[0xc8] = { 0 };
	bool	bResult = false;
	try
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
		{
			dwRet = 0;
			goto Cleanup;
		}

		bool	bRegInvalidFound = false;

		while (true)
		{

			bRegInvalidFound = false;
			i = dwSubKeys = 0x00;

			if (RegQueryInfoKey(hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL) != ERROR_SUCCESS)
			{
				dwRet = 2;
				goto Cleanup;
			}

			for (; i<dwSubKeys; i++)
			{
				dwType = dwRet = 0x00;
				dwSize = 511;
				memset(szSubKeyName, 0x00, 512 * sizeof(TCHAR));

				RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL);
				if (!szSubKeyName[0])
					continue;

				if (RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
					continue;

				dwSize = 511;
				memset(szPublName, 0x00, 100 * sizeof(TCHAR));
				RegQueryValueEx(hSubKey, TEXT("Publisher"), 0, &dwType, (LPBYTE)szPublName, &dwSize);

				OTHERAVMAP::iterator vOtherAVContent;
				for (vOtherAVContent = m_vOtherAVList.begin(); vOtherAVContent != m_vOtherAVList.end(); vOtherAVContent++)
				{
					char szPublisherName[0x64] = { 0 };
					wcstombs(szPublisherName, szPublName, sizeof(szPublisherName) + 1);

					if (szPublName != NULL)
					{
						if (strcmp(szPublisherName, (*vOtherAVContent).szPublisherName) == 0)
						{
							CString csUninstallPath = NULL;
							memset(szUninstallStr, 0x00, 200 * sizeof(TCHAR));
							dwType = 0x00;
							dwSize = 511;
							RegQueryValueEx(hSubKey, TEXT("UninstallString"), 0, &dwType, (LPBYTE)szUninstallStr, &dwSize);
							LPCWSTR lpUninstallString(szUninstallStr);
							csUninstallPath = szUninstallStr;

							int iFindExe = csUninstallPath.Find(L".exe");
							iFindExe += 4; //.exe 4 letters
							CString csUninstallArg = NULL;
							int i = csUninstallPath.GetLength();
							if (iFindExe < csUninstallPath.GetLength())
							{
								csUninstallArg = csUninstallPath.Right(i - ++iFindExe);
							}
							CString csUninstallString = csUninstallPath.Left(iFindExe);
							csUninstallString.Trim(L"\"");
							if (!PathFileExists(csUninstallString) && wcscmp(csUninstallString, L"MsiExec.exe") == 0)
							{
								continue;
							}

							csUninstallPath.Trim(L"\"");
							int iTemp = csUninstallPath.Find(_T('"'));
							csUninstallPath.Delete(iTemp);
							TCHAR	szDisplayName[512] = { 0 };
							dwType = 0x00;
							dwSize = 511;
							memset(szDisplayName, 0x00, 512 * sizeof(TCHAR));
							RegQueryValueEx(hSubKey, TEXT("DisplayName"), 0, &dwType, (LPBYTE)szDisplayName, &dwSize);
							CString csMessageContent;
							csMessageContent.Format(L"%s %s\n\n%s", szDisplayName, m_csVectInputStrings.at(CM_14_WWSetupDllOtherAV), m_csVectInputStrings.at(CM_15_WWSetupDllOtherAV));

							if (MessageBox(theApp.GetMainWnd()->m_hWnd, csMessageContent, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
							{
								ShellExecute(NULL, L"open", csUninstallString, csUninstallArg, NULL, SW_SHOW);
								bResult = true;
								return bResult;
							}
							else
							{
								bResult = false;
								return bResult;
							}
						}
					}
				}
				
				RegCloseKey(hSubKey);
				hSubKey = NULL;

				if (bRegInvalidFound)
					break;
			}
			if (!bRegInvalidFound)
				break;
		}

		if (bResult == false)
		{ 
			lpszSubKey = (TEXT("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));
			if( GetOtherAVDetails4Wow64(lpszSubKey))
				return true;
			else
				return false;
		}
		goto Cleanup;
	}
	catch (...)
	{
		dwRet = 0;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidUninstallEntries", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if (hKey)
		RegCloseKey(hKey);
	hKey = NULL;
	
	return false;
}
*/
/***************************************************************************
Function Name  : GetOtherAVDetails4Wow64
Description    : This Function is used to uninstall from Wow62 other AV while installing WardWiz.
Author Name    : Amol Jaware
Date           : 18 Oct 2018
****************************************************************************/
/*
DWORD CSetupDLLApp::GetOtherAVDetails4Wow64(LPCTSTR lpszSubKey)
{
	HKEY	hKey = NULL, hSubKey = NULL;
	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00;
	TCHAR	szSubKeyName[512] = { 0 };
	TCHAR	szPublName[0x64] = { 0 };
	TCHAR	szUninstallStr[0xc8] = { 0 };
	bool	bResult = false;
	bool	bRegInvalidFound = false;
	try
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
		{
			bResult = false;
			goto Cleanup;
		}

		while (true)
		{

			bRegInvalidFound = false;
			i = dwSubKeys = 0x00;

			if (RegQueryInfoKey(hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL) != ERROR_SUCCESS)
			{
				bResult = false;
				goto Cleanup;
			}

			for (; i<dwSubKeys; i++)
			{
				dwType = 0x00;
				dwSize = 511;
				memset(szSubKeyName, 0x00, 512 * sizeof(TCHAR));

				RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL);
				if (!szSubKeyName[0])
					continue;

				if (RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
					continue;

				dwSize = 511;
				memset(szPublName, 0x00, 100 * sizeof(TCHAR));
				RegQueryValueEx(hSubKey, TEXT("Publisher"), 0, &dwType, (LPBYTE)szPublName, &dwSize);

				OTHERAVMAP::iterator vOtherAVContent;
				for (vOtherAVContent = m_vOtherAVList.begin(); vOtherAVContent != m_vOtherAVList.end(); vOtherAVContent++)
				{
					char szPublisherName[0x64] = { 0 };
					wcstombs(szPublisherName, szPublName, sizeof(szPublisherName) + 1);

					if (szPublName != NULL)
					{
						if (strcmp(szPublisherName, (*vOtherAVContent).szPublisherName) == 0)
						{
							CString csUninstallPath = NULL;
							memset(szUninstallStr, 0x00, 200 * sizeof(TCHAR));
							dwType = 0x00;
							dwSize = 511;
							RegQueryValueEx(hSubKey, TEXT("UninstallString"), 0, &dwType, (LPBYTE)szUninstallStr, &dwSize);
							LPCWSTR lpUninstallString(szUninstallStr);
							csUninstallPath = szUninstallStr;

							int iFindExe = csUninstallPath.Find(L".exe");
							iFindExe += 4; //.exe 4 letters
							CString csUninstallArg = NULL;
							int i = csUninstallPath.GetLength();
							if (iFindExe < csUninstallPath.GetLength())
							{
								csUninstallArg = csUninstallPath.Right(i - ++iFindExe);
							}
							CString csUninstallString = csUninstallPath.Left(iFindExe);
							csUninstallString.Trim(L"\"");
							if (!PathFileExists(csUninstallString) && wcscmp(csUninstallString, L"MsiExec.exe") == 0)
							{
								continue;
							}

							csUninstallPath.Trim(L"\"");
							int iTemp = csUninstallPath.Find(_T('"'));
							csUninstallPath.Delete(iTemp);
							TCHAR	szDisplayName[512] = { 0 };
							dwType = 0x00;
							dwSize = 511;
							memset(szDisplayName, 0x00, 512 * sizeof(TCHAR));
							RegQueryValueEx(hSubKey, TEXT("DisplayName"), 0, &dwType, (LPBYTE)szDisplayName, &dwSize);
							CString csMessageContent;
							csMessageContent.Format(L"%s %s\n\n%s", szDisplayName, m_csVectInputStrings.at(CM_14_WWSetupDllOtherAV), m_csVectInputStrings.at(CM_15_WWSetupDllOtherAV));
							
							if (MessageBox(theApp.GetMainWnd()->m_hWnd, csMessageContent, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
							{
								ShellExecute(NULL, L"open", csUninstallString, csUninstallArg, NULL, SW_SHOW);
								bResult = true;
								return bResult;
							}
							else
							{
								bResult = false;
								return bResult;
							}
						}
					}
				}

				RegCloseKey(hSubKey);
				hSubKey = NULL;

				if (bRegInvalidFound)
					break;
			}
			if (!bRegInvalidFound)
				break;
		}
		goto Cleanup;
	}
	catch (...)
	{
		return false;
		AddLogEntry(L"### Exception in WardwizSrvMgmt_RegOpt::CheckInvalidUninstallEntries", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if (hKey)
		RegCloseKey(hKey);
	hKey = NULL;

	return false;
}

void CSetupDLLApp::GetRegOtherAVValues()
{
	TCHAR	szUserOSDrivePath[512] = { 0 };
	TCHAR	szAVDestRegistryValuePath[512] = { 0 };
	CString csGetOSDrive = L"";
	GetEnvironmentVariable(L"ALLUSERSPROFILE", szUserOSDrivePath, 511);
	CString csSelectedDrive = szUserOSDrivePath;
	int iPos = csSelectedDrive.Find(L"\\");
	csGetOSDrive = csSelectedDrive.Left(2);

	try
	{
		m_vOtherAVList.clear();
		for (int iCount = 0; iCount < 1; iCount++)
		{
			STRUCTOTHERAVLIST szOtherAVContent;

			strcpy(szOtherAVContent.szPublisherName, "Quick Heal Technologies Ltd.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Quick Heal\\Quick Heal Total Security\\Uninst.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Symantec Corporation");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\NortonInstaller\\{0C55C096-0F1D-4F28-AAA2-85EF591126E7}\\NGC\\A5E82D02\\22.15.0.88\\InstStub.exe /X /ARP", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "McAfee, Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\McAfee\\WebAdvisor\\Uninstaller.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "K7 Computing Pvt Ltd");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\K7 Computing\\K7TSecurity\\K7TSecurityUninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Bitdefender");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Common Files\\Bitdefender\\SetupInformation\\CL-23-B0B358F8-BF6F-4430-A7E6-8FAB49055BE1\\installer.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Bitdefender");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Common Files\\Bitdefender\\SetupInformation\\CL-23-23D7E3F5-A8F0-43AC-84F8-F6D723A8BEAD\\installer.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Bitdefender");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Common Files\\Bitdefender\\SetupInformation\\CL-23-CE2445C7-EE57-41D0-BB3F-6B71EE38EC7D\\installer.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Bitdefender");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Common Files\\Bitdefender\\SetupInformation\\CL-23-8D057196-D386-4CE2-983F-1CAF121F630F\\installer.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Avira Operations GmbH & Co. KG");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\Avira\\System Speedup\\unins000.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "adaware");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Common Files\\adaware\\adaware antivirus\\updater\\12.5.961.11619\\AdAwareUpdater.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "AhnLab, Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\AhnLab\\V3IS80\\Uninst.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Baidu, Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\Baidu Security\\Baidu Antivirus\\5.4.3.148966.0\\Uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "alch");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\ClamWin\\unins000.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Doctor Web, Ltd.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\ProgramData\\Doctor Web\\Setup\\drweb-katana\\katana-setup.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Malwarebytes");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Malwarebytes\\Anti-Malware\\unins000.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "G DATA Software AG");
			wsprintf(szAVDestRegistryValuePath, L"%s\\ProgramData\\G Data\\Setups\\G DATA TOTAL SECURITY\\setup.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Max Secure Software");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Max PC Tuner\\unins000.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "NANO Security");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\NANO Antivirus\\uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "K7 Computing Pvt Ltd");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\K7 Computing\\K7TSecurity\\K7TSecurityUninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "MicroWorld Technologies Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\eScan\\unins000.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Check Point");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\CheckPoint\\Install\\Install.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Webroot");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Webroot\\WRSA.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Trend Micro Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Trend Micro\\AirSupport\\Uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Trend Micro Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Trend Micro\\Titanium\\Remove", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Trend Micro Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Trend Micro\\TMIDS\\unins000.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Tencent Technology(Shenzhen) Company Limited");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\Tencent\\QQPCMgr\\12.3.26586.901\\Uninst.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "SUPERAntiSpyware.com");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\CheckPoint\\Install\\Install.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Sophos Limited");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\Sophos\\Management Communications System\\Endpoint\\Uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Sophos Limited");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Sophos\\Sophos File Scanner\\Uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "360 Security Center");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\360\\Total Security\\Uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Panda Security");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Panda Security URL Filtering\\uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Panda Safe Web");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\pandasecuritytb\\uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Cylance, Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\ProgramData\\Package Cache\\{e501278d-fed7-465d-b7d2-3f4fa966eeda}\\CylanceProtectSetup.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "AVAST Software");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\AVAST Software\\Browser\\Application\\69.0.829.82\\Installer\\setup.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "AVAST Software");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\AVAST Software\\Avasr\\Setup\\Instup.exe//control_panel", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Quick Heal Technologies Ltd.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Quick Heal AntiVirus Pro\\Uninst.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Quick Heal Technologies Ltd.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Quick Heal Internet Security\\Uninst.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Biz Secure Labs Pvt. Ltd.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Net Protector 2018\\NPSetup.exe /UNINSTALL", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Max Secure Software");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Max Secure Anti Virus Plus\\MaxUninstaller.exe -AVPLUS", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Max Secure Software");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Max Internet Security\\MaxUninstaller.exe -IS", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "G DATA Software AG");
			wsprintf(szAVDestRegistryValuePath, L"%s\\ProgramData\\G Data\\Setups\\G DATA ANTIVIRUS\\setup.exe /InstallMode=Uninstall /_DoNotShowChange=true", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Kaspersky Lab");
			wsprintf(szAVDestRegistryValuePath, L"%s\\MsiExec.exe /I{F10AA188-7166-430E-8810-FEAB2AD73DE3} REMOVE=ALL", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Kaspersky Lab");
			wsprintf(szAVDestRegistryValuePath, L"%s\\MsiExec.exe /I{718613F4-492D-4272-ACC3-D04A8EF0F883} REMOVE=ALL", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "G DATA Software AG");
			wsprintf(szAVDestRegistryValuePath, L"%s\\ProgramData\\G Data\\Setups\\G DATA INTERNET SECURITY\\setup.exe /InstallMode=Uninstall /_DoNotShowChange=true", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "AVG Technologies");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\AVG\\Antivirus\\Setup\\Instup.exe /control_panel", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "COMODO");
			wsprintf(szAVDestRegistryValuePath, L"%s\\ProgramData\\COMODO\\CCAV\\installer\\ccavstart.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "COMODO Security Solutions Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\COMODO\\COMODO Internet Security\\cmdinstall.exe -type local -uninstall -theme lycia -log", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "SUPERAntiSpyware.com");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\SUPERAntiSpyware\\Uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Comodo");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\Comodo\\Dragon\\uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Armor Antivirus Pvt. Ltd.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\ArmorAV\\UnChk.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Check Point");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\CheckPoint\\Install\\Install.exe\" /s uninstall", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Panda Security and Visicom Media Inc.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\pandasecuritytb\\uninstall.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Sophos Limited");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files\\Sophos\\Sophos Endpoint Agent\\uninstallgui.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "ESET, spol. s r.o.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\MsiExec.exe /I{30AAEA0C-2993-4ED6-8ABC-48499DA53D87}", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "FRISK Software");
			wsprintf(szAVDestRegistryValuePath, L"%s\\MsiExec.exe /I{D0C17D81-D40D-4C23-B8FA-95E817D0B7BE}", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Beijing Jiangmin New Sci. & Tech. Co., Ltd");
			wsprintf(szAVDestRegistryValuePath, L"%s\\Program Files (x86)\\JiangMin\\Install\\Setup.exe /uninstall", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Fortinet Technologies Inc");
			wsprintf(szAVDestRegistryValuePath, L"%s\\MsiExec.exe /X{E869338F-FD3D-4A12-9C1A-5583D1AE23FC}", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "Kingsoft Internet Security");
			wsprintf(szAVDestRegistryValuePath, L"%s\\program files (x86)\\kingsoft\\kingsoft antivirus\\uninst.exe", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);

			strcpy(szOtherAVContent.szPublisherName, "VirusBlokAda Ltd.");
			wsprintf(szAVDestRegistryValuePath, L"%s\\MsiExec.exe /I{BA0657F9-03DB-4BE2-8D37-3F811FA792BB}", csGetOSDrive);
			strcpy(szOtherAVContent.szUninstallString, (const char *)szAVDestRegistryValuePath);
			m_vOtherAVList.push_back(szOtherAVContent);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CSetupDLLApp::GetOtherAVDetails", 0, 0, true, SECONDLEVEL);
	}
}*/

/*************************************************************************************
Function Name  : LaunchProcessThrCommandLine
Description    : This Function Used to Install the Sql Server Through CommandLine.
Author Name    : Tejas Shinde
Date           :03 December 2018
*************************************************************************************/
extern "C" DLLEXPORT bool LaunchProcessThrCommandLine(LPTSTR pszAppPath, LPTSTR pszCmdLine)
{
	try
	{
		return theApp.LaunchProcessThrCommandLine(pszAppPath, pszCmdLine);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LaunchProcessThrCommandLine", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : LaunchProcessThrCommandLine
Description    : This function will get CommandLine As Paramaters and Execute the Command
Author Name    : Tejas Shinde
Date           : 03 December 2018
***********************************************************************************************/
bool CSetupDLLApp::LaunchProcessThrCommandLine(LPTSTR pszSqlServerTempPath, LPTSTR pszConfigInstallSqlServerTempPath)
{
	if (!pszSqlServerTempPath)
		return FALSE;

	if (!pszConfigInstallSqlServerTempPath)
		return FALSE;

	STARTUPINFO			si = { 0 };
	PROCESS_INFORMATION	pi = { 0 };

	try
	{
		si.cb = sizeof(STARTUPINFO);

		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		TCHAR commandLine[2 * MAX_PATH + 16] = { 0 };
		swprintf_s(commandLine, _countof(commandLine), L"\"%s\" %s ", pszSqlServerTempPath, pszConfigInstallSqlServerTempPath);

		if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		{
			AddLogEntry(L"### Failed CSetupDLLApp::LaunchProcessThrCommandLine : [%s]", commandLine);
			return TRUE;
		}

		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;


	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CSetupDLLApp::LaunchProcessThrCommandLine");
		return TRUE;
	}

	return FALSE;
}

/*****************************************************************************************************
Function Name  : LaunchInstallFilesProgressBar
Description    : This Function Used to Show Setup Files Installtion Progress bar With Files Details.
Author Name    : Tejas Shinde
Date           : 1 April 2019
*****************************************************************************************************/
extern "C" DLLEXPORT bool  LaunchInstallFilesProgressBar(LPTSTR pszWWIZInstalltionFilePath, int iPercentage)
{
	try
	{
		return theApp.LaunchInstallFilesProgressBar(pszWWIZInstalltionFilePath, iPercentage);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in LaunchInstallFilesProgressBar", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/****************************************************************************************************************
Function Name  : LaunchInstallFilesProgressBar
Description    : This Function Used to Show Setup Files Installtion Progress bar With File Path Details.
Author Name    : Tejas Shinde
Date           : 1 April 2019
*****************************************************************************************************************/
bool CSetupDLLApp::LaunchInstallFilesProgressBar(LPTSTR pszWWIZInstalltionFilePath, int iPercentage)
{
	try
	{
		if (!PostData2WWizInstaller(SEND_INSTALLER_STATUS_DETAILS, FILE_PATH_DETAILS, pszWWIZInstalltionFilePath, iPercentage))
		{
			AddLogEntry(L"### Exception in CSetupDLLApp::LaunchInstallFilesProgressBar", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CSetupDLLApp::LaunchInstallFilesProgressBar", 0, 0, true, SECONDLEVEL);
	}

	return true;
}


/***************************************************************************************************
*  Function Name  : PostData2WWizInstaller
*  Description    : Function which post message data to Installer application.( async )
*  Author Name    : Tejas Shinde
*  Date           : 16 April 2019
****************************************************************************************************/
bool CSetupDLLApp::PostData2WWizInstaller(DWORD dwMessage, DWORD dwStatusState, LPTSTR pszWWIZInstalltionFilePath, int iPercentage, bool bWait)
{
	try
	{
		if (!pszWWIZInstalltionFilePath)
			return false;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwStatusState;
		wcscpy_s(szPipeData.szFirstParam, pszWWIZInstalltionFilePath);
		szPipeData.dwSecondValue = (DWORD)iPercentage;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CSetupDLLApp::PostData2WardwizInstaller", 0, 0, true, FIRSTLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::PostData2WardwizInstaller", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/***************************************************************************************************
*  Function Name  : SendData2WWizInstaller
*  Description    : Function which send message data to Installer application.
*  Author Name    : Tejas Shinde
*  Date           : 16 April 2019
****************************************************************************************************/
bool CSetupDLLApp::SendData2WWizInstaller(DWORD dwMessage, DWORD dwStatusState, LPTSTR pszWWIZInstalltionFilePath, int iPercentage, bool bWait)
{
	try
	{
		if (!pszWWIZInstalltionFilePath)
			return false;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwStatusState;		
		wcscpy_s(szPipeData.szFirstParam, pszWWIZInstalltionFilePath);
		szPipeData.dwSecondValue = (DWORD)iPercentage;

		CISpyCommunicator objCom(WWIZ_INSTALLER_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendData2WardwizInstaller", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{	
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendData2WardwizInstaller", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception inCSetupDLLApp::SendData2WardwizInstaller", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/*****************************************************************************************************
* Function Name  : XMLListScanningFinished
* Description    : This Function Used to Show Av Setup Xml File Node Scanning Completed.
* Author Name    : Tejas Shinde
* Date           : 26 April 2019
*****************************************************************************************************/
extern "C" DLLEXPORT bool XMLListScanningFinished()
{
	try
	{
		theApp.XMLListScanningFinished();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in XMLListScanningFinished", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/*****************************************************************************************************
* Function Name  : XMLListScanningFinished
* Description    : This Function Used to Show Av Setup Xml File Node Scanning Completed
then pass Installer Callback status.
* Author Name    : Tejas Shinde
* Date           : 26 April 2019
*****************************************************************************************************/
void CSetupDLLApp::XMLListScanningFinished()
{
	try
	{
		DWORD dwFinishFlag = 0;

		if (!SendUninstallAvData2WWizInstaller(SEND_OTHER_AV_DETAILS, FINISH_UNINSTALL_AV_DETAILS, dwFinishFlag))
		{
			AddLogEntry(L"### Exception in CSetupDLLApp::XMLListScanningFinished", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CSetupDLLApp::XMLListScanningFinished", 0, 0, true, SECONDLEVEL);
	}
}

/*****************************************************************************************************
* Function Name  : GetUninstallAvInfo
* Description    : This Function Used to Show Setup Files Installtion Progress bar With Files Details.
* Author Name    : Tejas Shinde
* Date           : 26 April 2019
*****************************************************************************************************/
extern "C" DLLEXPORT bool  GetUninstallAvInfo(LPTSTR pszUninstallAvDetails)
{
	try
	{
		return theApp.GetUninstallAvInfo(pszUninstallAvDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetUninstallAvInfo", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/****************************************************************************************************************
* Function Name  : GetUninstallAvInfo
* Description    : This Function Used to Show Setup Files Installtion Progress bar With File Path Details.
* Author Name    : Tejas Shinde
* Date           : 26 April 2019
*****************************************************************************************************************/
bool CSetupDLLApp::GetUninstallAvInfo(LPTSTR pszUninstallAvDetails)
{
	try
	{
		if (!SendUninstallAvData2WWizInstaller(SEND_OTHER_AV_DETAILS, UNINSTALL_AV_DETAILS, pszUninstallAvDetails))
		{
			AddLogEntry(L"### Exception in CSetupDLLApp::SendUninstallAvData2WardwizInstaller", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CSetupDLLApp::GetUninstallAvInfo", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : SendUninstallAvData2WWizInstaller
*  Description    : Function which send message Uninstall Av Registry data to Installer application.
*  Author Name    : Tejas Shinde
*  Date           : 26 April 2019
****************************************************************************************************/
bool CSetupDLLApp::SendUninstallAvData2WWizInstaller(DWORD dwMessage, DWORD dwStatusState, LPTSTR pszUninstallAvDetails, bool bWait)
{
	try
	{
		if (!pszUninstallAvDetails)
			return false;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwStatusState;
		wcscpy_s(szPipeData.szFirstParam, pszUninstallAvDetails);

		CISpyCommunicator objCom(WWIZ_INSTALLER_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendUninstallAvData2WardwizInstaller", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendUninstallAvData2WardwizInstaller", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception inCSetupDLLApp::SendUninstallAvData2WardwizInstaller", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SendUninstallAvData2WWizInstaller
*  Description    : Function which send message Uninstall Av Registry data to Installer application.
*  Author Name    : Tejas Shinde
*  Date           : 26 April 2019
****************************************************************************************************/
bool CSetupDLLApp::SendUninstallAvData2WWizInstaller(DWORD dwMessage, DWORD dwStatusState, DWORD dwFinishFlag, bool bWait)
{
	try
	{

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwStatusState;
		szPipeData.dwSecondValue = dwFinishFlag;

		CISpyCommunicator objCom(WWIZ_INSTALLER_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendData2WardwizInstaller", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendData2WardwizInstaller", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception inCSetupDLLApp::SendData2WardwizInstaller", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/*****************************************************************************************************
* Function Name  : SendTempPathNExtractFinishedFlag
* Description    : This Function Used to Installer Extracting Files Completed Get the Flag.
* Author Name    : Tejas Shinde
* Date           : 25 June 2019
*****************************************************************************************************/
extern "C" DLLEXPORT bool SendTempPathNExtractFinishedFlag(LPTSTR pszExtracttempfolderDetails)
{
	try
	{
		theApp.SendTempPathNExtractFinishedFlag(pszExtracttempfolderDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SendTempPathNExtractFinishedFlag", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/*****************************************************************************************************
* Function Name  : SendTempPathNExtractFinishedFlag
* Description    : This Function Used to Installer Extracting Files Completed Get the Flag
then pass Installer Callback status.
* Author Name    : Tejas Shinde
* Date           : 25 June 2019
*****************************************************************************************************/
void CSetupDLLApp::SendTempPathNExtractFinishedFlag(LPTSTR pszExtracttempfolderDetails)
{
	try
	{
		if (!SendUninstallAvData2WWizInstaller(SEND_OTHER_AV_DETAILS, FINISH_EXTRACT_FILES, pszExtracttempfolderDetails))

		{
			AddLogEntry(L"### Exception in CSetupDLLApp::SendUninstallAvData2WardwizInstaller", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CSetupDLLApp::SendTempPathNExtractFinishedFlag", 0, 0, true, SECONDLEVEL);
	}
}
/*****************************************************************************************************
* Function Name  : SendInstallerLocationPath
* Description    : This Function Used to Send Installer .Exe Path .
* Author Name    : Tejas Shinde
* Date           : 12 Nov 2019
*****************************************************************************************************/
extern "C" DLLEXPORT bool SendInstallerLocationPath(LPTSTR pszExtracttempfolderDetails)
{
	try
	{
		theApp.SendInstallerLocationPath(pszExtracttempfolderDetails);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SendInstallerLocationPath", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/*****************************************************************************************************
* Function Name  : SendInstallerLocationPath
* Description    : This Function Used to Send Installer .Exe Path then pass Installer Callback status.
* Author Name    : Tejas Shinde
* Date           : 12 Nov 2019
*****************************************************************************************************/
void CSetupDLLApp::SendInstallerLocationPath(LPTSTR pszInstallerLocationPath)
{
	try
	{
		if (!SendUninstallAvData2WWizInstaller(SEND_OTHER_AV_DETAILS, INSTALLER_LOCATION_PATH, pszInstallerLocationPath))

		{
			AddLogEntry(L"### Exception in CSetupDLLApp::SendUninstallAvData2WardwizInstaller", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CSetupDLLApp::SendInstallerLocationPath", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
* Function Name  : InstallerDriverService
* Description    : Exported function to install driver services
* Author Name    : Tejas Shinde
* Date           : 13 Nov 2019
****************************************************************************/
extern "C" DLLEXPORT int InstallerDriverService(LPTSTR pszExtracttempfolderDetails)
{
	int returnStatus = 0;
	try
	{
		CScannerLoad objScannerload;
		CSecure64  objPSecure;
		CWardWizOSversion objGetOSVersion;
		CString GetTempExtractPath = pszExtracttempfolderDetails;

		int iOsType = 0;

		if (GetTempExtractPath != L"")
		{
			iOsType = objGetOSVersion.DetectClientOSVersion();
			if (iOsType == WINOS_XP)
			{
				returnStatus = objPSecure.InstallService(GetTempExtractPath + L"\\" + WRDWIZXPPROCSYS, iOsType);
				returnStatus = objScannerload.InstallScanner(GetTempExtractPath + L"\\" + WRDWIZSCANNERSYS);   
			}
			else  if (iOsType == WINOS_XP64)
			{
				returnStatus = objScannerload.InstallScanner(GetTempExtractPath + L"\\" + WRDWIZSCANNERSYS);   
			}
			else
			{
				returnStatus = objPSecure.InstallService(GetTempExtractPath + L"\\" + WRDWIZSECURE64SYS, iOsType);     
				returnStatus = objScannerload.InstallScanner(GetTempExtractPath + L"\\" + WRDWIZSCANNERSYS); 
			}
		}
		AddLogEntry(L">>> after scanner and secure64 installed", 0, 0, true, SECONDLEVEL);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in InstallerDriverService", 0, 0, true, SECONDLEVEL);
	}
	return returnStatus;
}

/***************************************************************************
* Function Name  : StopInstallerFltDrivers
* Description    : Function to unload minifilter driver.
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
****************************************************************************/
extern "C" DLLEXPORT bool StopInstallerFltDrivers(LPTSTR lpszDriverName)
{
	bool bReturn = false;
	__try
	{
		bReturn = theApp.StopInstallerDriversService(lpszDriverName);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in StopInstallerFltDrivers, Driver Name: %s", lpszDriverName, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
* Function Name  : StopDriverService
* Description    : Function to stop driver service.
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
****************************************************************************/
bool CSetupDLLApp::StopInstallerDriversService(LPTSTR lpszDriverName)
{
	bool bReturn = false;
	try
	{
		CISpySrvMgmt		iSpySrvMgmtObj;
		DWORD				dwRet = 0x00;
		CString				csFailureCase(L"");

		dwRet = iSpySrvMgmtObj.StopServiceManually(lpszDriverName);
		csFailureCase.Format(L"%d", dwRet);
		if (dwRet != 0x00)
		{
			AddLogEntry(L"### Unable to Stop Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
			return bReturn;
		}

		dwRet = iSpySrvMgmtObj.UnInstallService(lpszDriverName);
		csFailureCase.Format(L"%d", dwRet);
		if (dwRet != 0x00)
		{
			AddLogEntry(L"### Unable to UnInstallService WardwizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
			return bReturn;
		}


		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::StopInstallerDriversService, Driver Name: %s", lpszDriverName, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***********************************************************************************
* Function Name  : FindProcessId
* Description    : Find Process id and register setup with protection drivers
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
************************************************************************************/
DWORD FindProcessId(const std::wstring& processName)
{
	try
	{
		PROCESSENTRY32 processInfo;
		processInfo.dwSize = sizeof(processInfo);

		HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (processesSnapshot == INVALID_HANDLE_VALUE) {
			return 0;
		}

		Process32First(processesSnapshot, &processInfo);
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}

		while (Process32Next(processesSnapshot, &processInfo))
		{
			if (!processName.compare(processInfo.szExeFile))
			{
				CloseHandle(processesSnapshot);
				return processInfo.th32ProcessID;
			}
		}

		CloseHandle(processesSnapshot);
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in FindProcessId", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/*************************************************************************************
* Function Name  : FindParentProcessId
* Description    : Find Parent Process id and register setup with protection drivers
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
**************************************************************************************/
DWORD FindParentProcessId(const std::wstring& processName)
{
	try
	{
		PROCESSENTRY32 processInfo;
		processInfo.dwSize = sizeof(processInfo);

		HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (processesSnapshot == INVALID_HANDLE_VALUE) {
			return 0;
		}

		Process32First(processesSnapshot, &processInfo);
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ParentProcessID;
		}

		while (Process32Next(processesSnapshot, &processInfo))
		{
			if (!processName.compare(processInfo.szExeFile))
			{
				CloseHandle(processesSnapshot);
				return processInfo.th32ParentProcessID;
			}
		}

		CloseHandle(processesSnapshot);
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in FindProcessId", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***************************************************************************
* Function Name  : RegisterInstallerWithDrivers
* Description    : to register setup with protection drivers
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
****************************************************************************/
extern "C" DLLEXPORT void RegisterInstallerWithDrivers(LPTSTR pszInstallSetupName)
{
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	CEnumProcess objEnumProcess;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	bool returnStatus = false;
	DWORD  GetProFirstID = 0;
	DWORD GetProcSndID = 0;
	DWORD GetProThirdID = 0;
	DWORD GetProcForthID = 0;
	DWORD GetProcFifthID = 0;
	CString csGetInstallSetupName, GetInstallFirsttmpPath, GetInstallExePath, GetInstallFirstExePath, GetInstallSecondExePath, GetInstallSecondtmpPath;
	csGetInstallSetupName = pszInstallSetupName;
	CString				csFailureCase(L"");
	DWORD  GetProcID = 0;

	try
	{
		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == WINOS_XP)
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.RegisterProcessId(WLSRV_ID_SEVENTEEN); 
			}

			if (scannerObj.IsScannerDriverRunning())
			{
				scannerObj.RegisterProcessId(WLSRV_ID_SEVENTEEN); 
			}
		}
		else if (iOsType == WINOS_XP64)
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.RegisterProcessId(WLSRV_ID_SEVENTEEN); 
			}
		}
		else
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				GetInstallFirstExePath.Format(L"%s%s", L"WRDWIZINSTALLER", L".exe");
				std::wstring GetFinalFirstExePath = GetInstallFirstExePath;
				GetProFirstID = FindProcessId(GetFinalFirstExePath);
				csFailureCase.Format(L"%d", GetProFirstID);
				cSecure64Obj.RegisterProcessIdAndCode(WLSRV_ID_SEVENTEEN, GetProFirstID);

				GetInstallFirsttmpPath.Format(L"%s%s", csGetInstallSetupName, L".tmp");
				std::wstring GetFinalFirstExetmpPath = GetInstallFirsttmpPath;
				GetProcSndID = FindParentProcessId(GetFinalFirstExetmpPath);
				csFailureCase.Format(L"%d", GetProcSndID);
				cSecure64Obj.RegisterProcessIdAndCode(WLSRV_ID_EIGHTEEN, GetProcSndID);

				GetInstallSecondtmpPath.Format(L"%s%s", csGetInstallSetupName, L".tmp");
				std::wstring GetFinalSecondExetmpPath = GetInstallSecondtmpPath;
				GetProThirdID = FindProcessId(GetFinalSecondExetmpPath);
				csFailureCase.Format(L"%d", GetProThirdID);
				cSecure64Obj.RegisterProcessIdAndCode(WLSRV_ID_NINETEEN, GetProThirdID);
				
				GetInstallSecondExePath.Format(L"%s%s", csGetInstallSetupName, L".exe");
				std::wstring GetFinalSecondExePath = GetInstallSecondExePath;
				GetProcForthID = FindProcessId(GetFinalSecondExePath);
				csFailureCase.Format(L"%d", GetProcForthID);
				cSecure64Obj.RegisterProcessIdAndCode(WLSRV_ID_TWENTY, GetProcForthID);

			}

			if (scannerObj.IsScannerDriverRunning())
			{
				scannerObj.RegisterProcessId(WLSRV_ID_SEVENTEEN);
				scannerObj.RegisterProcessId(WLSRV_ID_EIGHTEEN);
				scannerObj.RegisterProcessId(WLSRV_ID_NINETEEN);
				scannerObj.RegisterProcessId(WLSRV_ID_TWENTY);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RegisterInstallerWithDrivers", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
* Function Name  : RegisterAvInstallerWithDrivers
* Description    : to register Av setup with protection drivers
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
****************************************************************************/
extern "C" DLLEXPORT void RegisterAvInstallerWithDrivers(LPTSTR pszAvSetupName)
{
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	CEnumProcess objEnumProcess;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	bool returnStatus = false;
	CString				csFailureCase(L"");
	DWORD  GetProFirstID = 0;
	DWORD GetProcSndID = 0;
	CString GetAvSetupName, GetAvExetmpPath, GetAvExePath;
	GetAvSetupName=pszAvSetupName;
	try
	{
		if (GetAvSetupName!= L"")
		{
			iOsType = objGetOSVersion.DetectClientOSVersion();
			if (iOsType == WINOS_XP)
			{
				if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
				{
					cSecure64Obj.RegisterProcessId(WLSRV_ID_SEVENTEEN); 
				}

				if (scannerObj.IsScannerDriverRunning())
				{
					scannerObj.RegisterProcessId(WLSRV_ID_SEVENTEEN); 
				}
			}
			else
			if (iOsType == WINOS_XP64)
			{
				if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
				{
					cSecure64Obj.RegisterProcessId(WLSRV_ID_SEVENTEEN); 
				}
			}
			else
			{
				if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
				{
					GetAvExetmpPath.Format(L"%s%s", GetAvSetupName, L".tmp");
					std::wstring GetFinalAvExetmpPath = GetAvExetmpPath;
					if (objEnumProcess.IsProcessRunning(GetAvExetmpPath, false, false, false))
					{
						GetProFirstID = FindProcessId(GetFinalAvExetmpPath);
						cSecure64Obj.RegisterProcessIdAndCode(WLSRV_ID_EIGHTEEN, GetProFirstID);
					}
					GetAvExePath.Format(L"%s%s", GetAvSetupName, L".exe");
					std::wstring GetFinalAvExePath = GetAvExePath;
					if (objEnumProcess.IsProcessRunning(GetAvExePath, false, false, false))
					{
						GetProcSndID = FindProcessId(GetFinalAvExePath);
						cSecure64Obj.RegisterProcessIdAndCode(WLSRV_ID_NINETEEN, GetProcSndID);
					}
				}

				if (scannerObj.IsScannerDriverRunning())
				{
					scannerObj.RegisterProcessId(WLSRV_ID_EIGHTEEN);
					scannerObj.RegisterProcessId(WLSRV_ID_NINETEEN);
				}

			}
		}
		else
		{
			AddLogEntry(L"### Exception in CSetupDLLApp::RegisterAvInstallerWithDrivers, Av SetupName: %s Not Exists", GetAvSetupName, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RegisterAvInstallerWithDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
* Function Name  : RegisterInstallerFinalSetupWithDrivers
* Description    : to register Final setup with protection drivers
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
****************************************************************************/
extern "C" DLLEXPORT void RegisterInstallerFinalSetupWithDrivers(LPTSTR pszAvSetupName)
{
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	CEnumProcess objEnumProcess;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	bool returnStatus = false;
	CString				csFailureCase(L"");
	DWORD  GetProFirstID = 0;
	DWORD GetProcSndID = 0;
	CString GetSetupName, GetSetupExePath, GetSetuptmpPath;
	GetSetupName = pszAvSetupName;
	try
	{
		if (GetSetupName != L"")
		{
			iOsType = objGetOSVersion.DetectClientOSVersion();
			if (iOsType == WINOS_XP)
			{
				if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
				{
					cSecure64Obj.RegisterProcessId(WLSRV_ID_TWENTY);
					cSecure64Obj.RegisterProcessId(WLSRV_ID_TWENTYONE);
				}

				if (scannerObj.IsScannerDriverRunning())
				{
					scannerObj.RegisterProcessId(WLSRV_ID_TWENTY);
					scannerObj.RegisterProcessId(WLSRV_ID_TWENTYONE);
				}
			}
			else
			if (iOsType == WINOS_XP64)
			{
				if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
				{
					cSecure64Obj.RegisterProcessId(WLSRV_ID_TWENTY);
					cSecure64Obj.RegisterProcessId(WLSRV_ID_TWENTYONE);
				}
			}
			else
			{
				if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
				{
					GetSetuptmpPath.Format(L"%s%s", GetSetupName, L".tmp");
					std::wstring GetFinalSetupTmpPath = GetSetuptmpPath;
					if (objEnumProcess.IsProcessRunning(GetSetuptmpPath, false, false, false))
					{
						GetProFirstID = FindProcessId(GetFinalSetupTmpPath);
						cSecure64Obj.RegisterProcessIdAndCode(WLSRV_ID_TWENTY, GetProFirstID);
					}
					GetSetupExePath.Format(L"%s%s", GetSetupName, L".exe");
					std::wstring GetFinalSetupExePath = GetSetupExePath;
					if (objEnumProcess.IsProcessRunning(GetSetupExePath, false, false, false))
					{
						GetProcSndID = FindProcessId(GetFinalSetupExePath);
						cSecure64Obj.RegisterProcessIdAndCode(WLSRV_ID_TWENTYONE, GetProcSndID);
					}
				}

				if (scannerObj.IsScannerDriverRunning())
				{
					scannerObj.RegisterProcessId(WLSRV_ID_TWENTY);
					scannerObj.RegisterProcessId(WLSRV_ID_TWENTYONE);
				}
			}
		}
		else
		{
			AddLogEntry(L"### Exception in CSetupDLLApp::RegisterInstallerFinalSetupWithDrivers, Final SetupName: %s Not Exists", GetSetupName, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RegisterInstallerFinalSetupWithDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
* Function Name  : RegisterInstallerthroughDrivers
* Description    : to register setup with protection drivers
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
****************************************************************************/
extern "C" DLLEXPORT void RegisterInstallerthroughDrivers()
{
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	CEnumProcess cenumprocessobj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	bool returnStatus = false;
	DWORD  GetProcID = 0;

	try
	{
		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == WINOS_XP)
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.RegisterProcessId(WLSRV_ID_TWENTYTWO); 
			}

			if (scannerObj.IsScannerDriverRunning())
			{
				scannerObj.RegisterProcessId(WLSRV_ID_TWENTYTWO); 
			}
		}
		else
		if (iOsType == WINOS_XP64)
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.RegisterProcessId(WLSRV_ID_TWENTYTWO); 
			}
		}
		else
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.RegisterProcessId(WLSRV_ID_TWENTYTWO); 
			}

			if (scannerObj.IsScannerDriverRunning())
			{
				scannerObj.RegisterProcessId(WLSRV_ID_TWENTYTWO); 
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RegisterInstallerthroughDrivers", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
* Function Name  : PauseInstallerProtectionDrivers
* Description    : to register setup with protection drivers
* Author Name    : Tejas Shinde
* Date           : 26th Nov 2019
****************************************************************************/
extern "C" DLLEXPORT void PauseInstallerProtectionDrivers(int szProcessID)
{
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	CEnumProcess cenumprocessobj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int iOsType = 0;
	bool returnStatus = false;
	CString				csFailureCase(L"");
	DWORD  GetProcID = 0;
	
    try
	{
		GetProcID = szProcessID;
		iOsType = objGetOSVersion.DetectClientOSVersion();
		if (iOsType == WINOS_XP)
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.PauseDriverProtection(GetProcID);
			}

			if (scannerObj.IsScannerDriverRunning())
			{
				scannerObj.PauseDriverProtection(GetProcID); 
			}
		}
		else
		if (iOsType == WINOS_XP64)
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.PauseDriverProtection(GetProcID); 
			}
		}
		else
		{
			if (cSecure64Obj.IsSecure64DriverRunning(iOsType))
			{
				cSecure64Obj.PauseDriverProtection(GetProcID);
			}

			if (scannerObj.IsScannerDriverRunning())
			{
				scannerObj.PauseDriverProtection(GetProcID);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in PauseInstallerProtectionDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CloseAllwwizAplication
*  Description    : Function which Close Warwiz Running  application through Installer
*  Author Name    : Tejas Shinde
*  Date           : 15 Dec 2019
****************************************************************************************************/
extern "C" DLLEXPORT bool CloseAllwwizAplication()
{
	try
	{
		return theApp.CloseAllwwizAplication(true);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CloseAllWardwizAplication", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CloseAllwwizAplication
*  Description    : Function which Close Warwiz Running  application through Installer
*  Author Name    : Tejas Shinde
*  Date           : 15 Dec 2019
****************************************************************************************************/
bool CSetupDLLApp::CloseAllwwizAplication(bool bIsReInstall)
{
	bool bISFullPath = false;

	if (_tcslen(m_csAppPath) > 0)
	{
		bISFullPath = true;
	}

	CString csModulePath = m_csAppPath;

	/*if(bIsReInstall)
	{*/
	/*if (!GetCloseAction4OutlookIfRunning(bIsReInstall))
	{
	return false;
	}
	*/
	//}

	//CString csAppPath = csModulePath + L"WRDWIZAVUI.EXE";
	CEnumProcess objEnumProcess;

	////While Installation or Uninstalltion if crypt exe is running it should close that exe first
	////Neha Gharge 1st July 2015

	CStringArray objcsaWardWizProcesses;
	objcsaWardWizProcesses.Add(L"WRDWIZAVUI.EXE");
	objcsaWardWizProcesses.Add(L"VBUI.EXE");
	objcsaWardWizProcesses.Add(L"VBTRAY.EXE");
	objcsaWardWizProcesses.Add(L"VBAUTORUNSCN.EXE");
	objcsaWardWizProcesses.Add(L"VBTEMPCLR.EXE");
	objcsaWardWizProcesses.Add(L"VBUSBVAC.EXE");
	objcsaWardWizProcesses.Add(L"VBUSBDETECTUI.EXE");
	objcsaWardWizProcesses.Add(L"VBCRYPT.EXE");
	objcsaWardWizProcesses.Add(L"VBSCANNER.EXE");
	objcsaWardWizProcesses.Add(L"VBUI.EXE");
	//objcsaWardWizProcesses.Add(L"VBCOMMSRV.EXE");//Issue resolved: 0001145

	bool bIsAnyProcTerminated = false;
	bool bIsYesAllReply = false;
	CString csMessageContent(L"");
	DWORD dwRetMessage = 0x02;//Bu default YesAll option selected.

	for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
	{
		CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);

		if (objEnumProcess.IsProcessRunning(csProcessName, false, false, false))
		{
			if (!bIsYesAllReply)
			{
				if (m_bUninstallation)
				{
					csMessageContent.Format(L"%s", m_csVectInputStrings.at(CM_0_WWSetupDllAppRunningUnInstall));
				}
				else
				{
					csMessageContent.Format(L"%s", m_csVectInputStrings.at(CM_1_WWSetupDllAppRunningReInstall));
				}

				dwRetMessage = 0x02;
				switch (dwRetMessage)
				{
				case 0x01:bIsYesAllReply = false;
					//YES
					break;
				case 0x02:bIsYesAllReply = true;
					//YESALL
					break;
				case 0x03://NO
					return false;
					//break;
				case 0x04://NOALL
					return false;
					//break;
				case 0x05://CANCEL
					return false;
					//break;
				}
			}
			if (objEnumProcess.IsProcessRunning(csProcessName, true, false, false))
			{
				bIsAnyProcTerminated = true;
				AddLogEntry(L">>> %s was running, Terminated", csProcessName, 0, true, SECONDLEVEL);
			}
			else
			{
				bIsAnyProcTerminated = true;
				AddLogEntry(L"### %s was running, Failed to terminated", csProcessName, 0, true, SECONDLEVEL);
			}

		}
	}

	//Neha Gharge Unregister com dll 20/4/2015
	CString csAppPath(L"");
	csAppPath = m_csAppPath + L"VBSHELLEXT.DLL";
	if (PathFileExists(csAppPath))
	{
		InstallerThroughUnregisterComDll(csAppPath);
	}

	csAppPath = csModulePath + L"VBSHELLEXT_OLD.DLL";
	if (PathFileExists(csAppPath))
	{
		InstallerThroughUnregisterComDll(csAppPath);
	}



	//ISSUE NO - 8 After re- installation showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
	//NAME - Niranjan Deshak. - 29th Jan 2015.	
	if (bIsAnyProcTerminated)
	{
		CCommonFunctions objCCommonFunctions;
		objCCommonFunctions.RefreshTaskbarNotificationArea();
	}

	//Dwret is add into addlogentry So that we enable to know the problem while installation
	//Neha Gharge
	//close and remove service here
	CISpySrvMgmt		iSpySrvMgmtObj;
	DWORD dwRet = 0x00;
	CString csFailureCase(L"");

	dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZSERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}
	//Issue: 1145: 	Issue with Reinstallation, while reinstallation Retry popup appears after clicking on "Retry" button installation get complete.
	//Resolved By: Nitin Kolapkar
	//Need to put sleep because In inno setup we are not able to find whether Comm service is stopped or not so putting 5 seconds sleep (Approx)
	Sleep(5 * 1000);
	dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}
	//Issue: 1145: 	Issue with Reinstallation, while reinstallation Retry popup appears after clicking on "Retry" button installation get complete.
	//Resolved By: Nitin Kolapkar
	//Need to put sleep because In inno setup we are not able to find whether Comm service is stopped or not so putting 5 seconds sleep (Approx)
	Sleep(5 * 1000);
	//close and remove service here
	dwRet = iSpySrvMgmtObj.StopServiceManually(WARDWIZUPDATESERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Update Service WardwizALUsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}

	dwRet = iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME);
	csFailureCase.Format(L"%d", dwRet);
	if (dwRet != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Update Service WardwizALUsrv with failure dword %s", csFailureCase, 0, true, SECONDLEVEL);
	}

	for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
	{
		CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);
		if (objEnumProcess.IsProcessRunning(csProcessName, false, false, false))
		{
			return false;
		}
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SendData2WWizInstaller4CloseMsg
*  Description    : Function which send message data to Installer application.
*  Author Name    : Tejas Shinde
*  Date           : 11 Dec 2019
****************************************************************************************************/
bool CSetupDLLApp::SendData2WWizInstaller4CloseMsg(DWORD dwMessage, DWORD dwStatusState, CString pszCloseAppMsg, bool bWait)
{
	try
	{
		if (!pszCloseAppMsg)
			return false;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwStatusState;
		wcscpy_s(szPipeData.szFirstParam, pszCloseAppMsg);

		CISpyCommunicator objCom(WWIZ_INSTALLER_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendData2WardwizInstaller4CloseMsg", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendData2WardwizInstaller4CloseMsg", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::SendData2WardwizInstaller4CloseMsg", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/*****************************************************************************************************
* Function Name  : InstallerCheckForPreviousVersion
* Description    : This Function Used to Check Previous Version Product Through Installer.
* Author Name    : Tejas Shinde
* Date           : 09 Jan 2020
*****************************************************************************************************/
extern "C" DLLEXPORT bool InstallerCheckForPreviousVersion(LPTSTR GetFirstAppMsg, LPTSTR GetSecondAppMsg, LPCTSTR szAppPath, LPCTSTR CurrentVersion, bool IsPatch)
{
	try
	{
		return theApp.InstallerCheckForPreviousVersion(GetFirstAppMsg, GetSecondAppMsg,szAppPath, CurrentVersion, IsPatch);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in InstallerCheckForPreviousVersion", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/******************************************************************************************
* Function Name  : InstallerCheckForPreviousVersion
* Description    : This Function Used to Check Previous Version Product Through Installer.
* Author Name    : Tejas Shinde
* Date           : 09 Jan 2020
********************************************************************************************/
bool CSetupDLLApp::InstallerCheckForPreviousVersion(LPTSTR GetFirstAppMsg, LPTSTR GetSecondAppMsg, LPCTSTR szAppPath, LPCTSTR iCurrentVersion, bool IsPatch)
{ 
	try
	{
		CString csFirstMessageContent(L""), csSecondMessageContent(L""), csAppPathContent(L"") ,csGetCurrentVersion(L"");

		csFirstMessageContent.Format(L"%s", GetFirstAppMsg);
		csSecondMessageContent.Format(L"%s", GetSecondAppMsg);
		csAppPathContent.Format(L"%s//%s", szAppPath, iCurrentVersion);

		if (!SendData2WWizInstallerCheckForPreviousVersionMsg(SEND_INSTALLER_STATUS_DETAILS, CHKVER_MSG, csFirstMessageContent.GetBuffer(csFirstMessageContent.GetLength()), csSecondMessageContent.GetBuffer(csSecondMessageContent.GetLength()), csAppPathContent.GetBuffer(csAppPathContent.GetLength()), 0))
		{
			AddLogEntry(L"### Exception in CSetupDLLApp::SendData2WardwizInstaller4CloseMsg", 0, 0, true, SECONDLEVEL);
			return false;
		}
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception InstallerCheckForPreviousVersion", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SendData2WWizInstallerCheckForPreviousVersionMsg
*  Description    : Function which send message data to Installer application For Check Previous version .
*  Author Name    : Tejas Shinde
*  Date           : 11 Dec 2019
****************************************************************************************************/
bool  CSetupDLLApp::SendData2WWizInstallerCheckForPreviousVersionMsg(DWORD dwMessage, DWORD dwStatusState, LPTSTR pszFirstAppMsg, LPTSTR pszSecondAppMsg, LPTSTR pszAppPath, bool bWait)
{
	try
	{
		if ((!pszFirstAppMsg) && (!pszSecondAppMsg) && (!pszAppPath))
	    return false;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = dwMessage;
		szPipeData.dwValue = dwStatusState;
		wcscpy_s(szPipeData.szFirstParam, pszFirstAppMsg);
		wcscpy_s(szPipeData.szSecondParam, pszSecondAppMsg);
		wcscpy_s(szPipeData.szThirdParam, pszAppPath);

		CISpyCommunicator objCom(WWIZ_INSTALLER_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendData2WardwizInstallerCheckForPreviousVersionMsg", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to SendData in CSetupDLLApp::SendData2WardwizInstallerCheckForPreviousVersionMsg", 0, 0, true, FIRSTLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSetupDLLApp::SendData2WardwizInstallerCheckForPreviousVersionMsg", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
/***************************************************************************
Function Name  : InstallerThroughUnregisterComDll
Description    : Unregister COM Dll
Author Name    : Tejas Shinde
Date           : 24 Jan 2020
****************************************************************************/
void CSetupDLLApp::InstallerThroughUnregisterComDll(CString csAppPath)
{
	try
	{
		CWardWizOSversion		objOSVersionWrap;
		CString csExePath, csCommandLine;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		CString csMessageContent(L"");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		csExePath.Format(L"%s\\%s", systemDirPath, L"regsvr32.exe");

		//On xp runas parameter never work It will not unregister the VBSHELLEXT.DLL
		//So NUll parameter send.
		DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
		//Neha Gharge Message box showing of register successful on reinstallation.
		switch (OSType)
		{
		case WINOS_XP:
		case WINOS_XP64:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, NULL, csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		default:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		}

		if (PathFileExists(csAppPath))
		{
			if (!DeleteFile(csAppPath))
			{
				AddLogEntry(L"### DeleteFile Failed for file: %s .", csAppPath, 0, true, SECONDLEVEL);
                
				if (PathFileExists(csAppPath))
				{				
					CString csRenameSetupDllName = csAppPath + L"_DEL";
					if (PathFileExists(csRenameSetupDllName))
					{
						DeleteFile(csRenameSetupDllName);
					}
					_wrename(csAppPath, csRenameSetupDllName);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in InstallerThroughUnregisterComDll", 0, 0, true, SECONDLEVEL);
	}

}
/*****************************************************************************************************
* Function Name  : XMLListScanningFinished
* Description    : This Function Used to Show Av Setup Xml File Node Scanning Completed.
* Author Name    :  Tejas Shinde
* Date           :  27 Jan 2020
*****************************************************************************************************/
extern "C" DLLEXPORT bool SetupInstallingFinished(bool bInstallSuccess)
{
	try
	{
		theApp.SetupInstallingFinished(bInstallSuccess);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in XMLListScanningFinished", 0, 0, true, SECONDLEVEL);
	}

	return true;
}


/*****************************************************************************************************
*  Function Name  : SetupInstallingFinished
*  Description    : This Function Used to Show Installing Completed then pass Installer Callback status.
*  Author Name    :  Tejas Shinde
*  Date           :  27 Jan 2020
*****************************************************************************************************/
void CSetupDLLApp::SetupInstallingFinished(bool bInstallSuccess)
{
	try
	{
		if (!SendUninstallAvData2WWizInstaller(SEND_INSTALLER_STATUS_SUCCESS, FINISH_INSTALLATIONS, bInstallSuccess))
		{
			AddLogEntry(L"### Exception in CSetupDLLApp::SetupInstallingsFinished", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CSetupDLLApp::XMLListScanningFinished", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : UnInstallVibranium1_0
Description    : Function will uninstall older 1.0 Vibranium version.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 06/22/2024
****************************************************************************/
extern "C" DLLEXPORT bool UnInstallVibranium1_0()
{
	bool bReturn = false;
	int count = 0;
	const int MAX_COUNT = 0x03;
	while (count < MAX_COUNT)
	{
		CITinRegWrapper objReg;
		TCHAR szVibroPath[MAX_PATH] = { 0 };
		DWORD dwVibroSize = sizeof(szVibroPath);


		TCHAR szValue[MAX_PATH] = { 0 };
		DWORD dwSize = sizeof(szValue);

		objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\VibraniumHome", L"Path", szVibroPath, dwVibroSize);

		CString csvbui = CString(szVibroPath) + L"\\vbui.exe";
		if (!PathFileExists(csvbui) || !PathFileExists(szVibroPath) || wcslen(szVibroPath) == 0x00)
		{
			AddLogEntry(L"### Vibranium old version 1.0 does not exists", 0, 0, true, SECONDLEVEL);
			return false;
		}

		//check here vibranium registy exists or not
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\VibraniumHome\\Global", L"Name", szValue, dwSize) == 0x00)
		{
			//Store value somewhere else in registy key for further use.
			if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibro", L"Name", szValue) != 0x00)
			{
				AddLogEntry(L"### Unable to set registry value [%s] in SOFTWARE\\Vibro ", L"Name", 0, true, SECONDLEVEL);
			}
		}

		//check here vibranium registy exists or not
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\VibraniumHome\\Global", L"Name", szValue, dwSize) == 0x00)
		{
			//Store value somewhere else in registy key for further use.
			if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibro", L"Name", szValue) != 0x00)
			{
				AddLogEntry(L"### Unable to set registry value [%s] in SOFTWARE\\Vibro ", L"Name", 0, true, SECONDLEVEL);
			}
		}

		HANDLE hYesThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WatchClickYesThread,
			NULL, 0, 0);

		HANDLE hOKThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WatchClickOKThread,
			NULL, 0, 0);

		CString csExePath = CString(szVibroPath) + L"\\uninstall.exe";
		theApp.InstallWWizSetupThrCommandLine(csExePath.GetBuffer(), L"/uninstall");

		Sleep(1000);

		theApp.InstallWWizSetupThrCommandLine(csExePath.GetBuffer(), L"/uninstall", true);

		Sleep(4000);

		bUninstallDone = true;

		if (hYesThread)
		{
			SuspendThread(hYesThread);
			TerminateThread(hYesThread, 0);
			CloseHandle(hYesThread);
			hYesThread = NULL;
		}

		if (hOKThread)
		{
			SuspendThread(hOKThread);
			TerminateThread(hOKThread, 0);
			CloseHandle(hOKThread);
			hOKThread = NULL;
		}

		count++;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : InstallWWizSetupThrCommandLine
*  Description    : This function will get CommandLine As Paramaters and Execute the Command
*  Author Name    : Tejas Shinde
*  Date           : 22 March 2019
***********************************************************************************************/
BOOL CSetupDLLApp::InstallWWizSetupThrCommandLine(LPTSTR pszWWIZSetupPath, LPTSTR pszCmdLine, bool bWait)
{
	if (!pszWWIZSetupPath)
		return FALSE;

	if (!pszCmdLine)
		return FALSE;

	STARTUPINFO			si = { 0 };
	PROCESS_INFORMATION	pi = { 0 };

	try
	{

		si.cb = sizeof(STARTUPINFO);

		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		TCHAR commandLine[2 * MAX_PATH + 16] = { 0 };
		swprintf_s(commandLine, _countof(commandLine), L"\"%s\" %s ", pszWWIZSetupPath, pszCmdLine);
		if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		{
			AddLogEntry(L"### Failed CWardwizInstallerDlg::InstallWardwizSetupThrCommandLine : [%s]", commandLine);
			return TRUE;
		}

		if (bWait)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			pi.hProcess = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CWardwizInstallerDlg::InstallWardwizSetupThrCommandLine");
		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI WatchClickYesThread(LPVOID lParam)
{
	while (true)
	{
		HWND hWindow = ::FindWindow(NULL, L"Vibranium Advanced Security Uninstall");
		if (hWindow)
		{
			HWND hHandle = FindWindowEx(hWindow, NULL, TEXT("Button"), TEXT("OK"));
			if (hHandle)
			{
				SendMessage(hHandle, BM_CLICK, 0, 0);
			}
		}

		HWND hWindow1 = ::FindWindow(NULL, L"Vibranium Threat Security Uninstall");
		if (hWindow1)
		{
			HWND hHandle1 = FindWindowEx(hWindow1, NULL, TEXT("Button"), TEXT("OK"));
			if (hHandle1)
			{
				SendMessage(hHandle1, BM_CLICK, 0, 0);
			}
		}

		if (bUninstallDone) break;

		Sleep(50);
	}
	return 0;
}

DWORD WINAPI WatchClickOKThread(LPVOID lParam)
{
	while (true)
	{
		HWND hWindow = ::FindWindow(NULL, L"Vibranium Advanced Security");
		if (hWindow)
		{
			ShowWindow(hWindow, SW_MINIMIZE);
			HWND ButtonHandle = FindWindowEx(hWindow, NULL, TEXT("Button"), TEXT("&Yes"));
			if (ButtonHandle)
			{
				Sleep(50);
				SendMessage(ButtonHandle, BM_CLICK, 0, 0);
			}
		}

		HWND hWindow1 = ::FindWindow(NULL, L"Vibranium Threat Security");
		if (hWindow1)
		{
			ShowWindow(hWindow1, SW_MINIMIZE);
			HWND ButtonHandle1 = FindWindowEx(hWindow1, NULL, TEXT("Button"), TEXT("&Yes"));
			if (ButtonHandle1)
			{
				Sleep(50);
				SendMessage(ButtonHandle1, BM_CLICK, 0, 0);
			}
		}

		if (bUninstallDone) break;

		Sleep(50);
	}

	return 0;
}

/***************************************************************************
Function Name  : UnInstallVibranium1_0
Description    : Function will uninstall older 1.0 Vibranium version.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 06/22/2024
****************************************************************************/
extern "C" DLLEXPORT bool RegisterVibro_2_0_to_1_0(int prod_id)
{
	bool bReturn = false;

	CITinRegWrapper objReg;
	TCHAR szValue[MAX_PATH] = { 0 };
	DWORD dwSize = sizeof(szValue);
	if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibro", L"Name", szValue, dwSize) != 0x00)
	{
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Vibro", L"Name", szValue, dwSize) != 0x00)
		{
			AddLogEntry(L"### Unable to get registry value [%s] in SOFTWARE\\Vibro ", L"Name", 0, true, SECONDLEVEL);
			return false;
		}
	}

	TCHAR	szUserInfo[512] = { 0 };
	TCHAR	szDomainName[MAX_PATH] = { 0 };
	if (!theApp.GetDomainName(szDomainName, MAX_PATH))
	{
		AddLogEntry(L"### Failed to get GetDomainName in RegisterVibro_2_0_to_1_0", 0, 0, true, SECONDLEVEL);
		return false;
	}

	//A = old_key
	//B = Machine Id
	//C = UUID
	//D = product ID%

	CString csMachineID = theApp.GetMachineID();
	CString csGUIID = theApp.GenerateGUIID();

	if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium", L"MVersion", csMachineID.GetBuffer()) != 0)
	{
		AddLogEntry(L"### Error in GetRegistryValueData CRegistrationSecondDlg::CheckForMachineID", 0, 0, true, SECONDLEVEL);
	}

	wsprintf(szUserInfo, L"http://%s/Prodmigration.aspx?A=%s&B=%s&C=%s&D=%d", szDomainName, szValue, csMachineID,
		csGUIID, prod_id);

	AddLogEntry(L">>>> Sending HTTP request", 0, 0, true, FIRSTLEVEL);
	AddLogEntry(szUserInfo);

	WinHttpClient client(szUserInfo);

	bool bssucess = true;
	if (!client.SendHttpRequest())
	{
		if (!client.SendHttpRequest())
		{
			bssucess = false;
		}
	}
	if (!bssucess)
	{
		CMigFailedDlg objMgFfailed;
		objMgFfailed.DoModal();
		AddLogEntry(L"### Failed to send request", 0, 0, true, SECONDLEVEL);
	}
	
	// The response content.
	wstring httpResponseContent = client.GetResponseContent();

	AddLogEntry(L">>>> Getting response", 0, 0, true, FIRSTLEVEL);
	AddLogEntry(httpResponseContent.c_str());

	CString m_csResponseData = httpResponseContent.c_str();

	LPSYSTEMTIME lpServerTime = {0};
	DWORD lpDaysRemaining = 0x00;
	TCHAR szNewKey[MAX_PATH] = { 0 };
	if ((m_csResponseData.GetLength() > 22) && (m_csResponseData.GetLength() < 512))
	{
		DWORD dwRet = theApp.ExtractDate(m_csResponseData.GetBuffer(), szNewKey, lpServerTime, &lpDaysRemaining);
		if (dwRet > 0x00)
		{
			CMigFailedDlg objMgFfailed;
			objMgFfailed.DoModal();
			AddLogEntry(L"### Failed to Activate product, response: [%s]", m_csResponseData, 0, true, SECONDLEVEL);
			return false;
		}
	}
	else
	{
		CMigFailedDlg objMgFfailed;
		objMgFfailed.DoModal();
		AddLogEntry(L"### Failed to Activate product, response: [%s]", m_csResponseData, 0, true, SECONDLEVEL);
		return false;
	}

	SYSTEMTIME		CurrTime = { 0 };
	GetSystemTime(&CurrTime);

	AVACTIVATIONINFO ActInfo = { 0 };
	ActInfo.dwProductNo = prod_id;
	wcscpy(ActInfo.szKey, szNewKey);
	wcscpy(ActInfo.szClientID, csMachineID);
	wcscpy(ActInfo.szInstID, csGUIID);
	ActInfo.dwTotalDays = lpDaysRemaining;
	ActInfo.RegTimeServer = CurrTime;
	ActInfo.RegTime = CurrTime;

	if (theApp.AddRegistrationDataInFile((LPBYTE)&ActInfo, sizeof(ActInfo)))
	{
		AddLogEntry(L"### AddRegistrationDataInFile failed in CRegistrationDlg AddProdRegInfoToLocal", 0, 0, true, SECONDLEVEL);
	}

	theApp.SpreadRegistrationFilesInSystem();

	return bReturn;
}


/***********************************************************************************************
Function Name  : GetDomainName
Description    : Function to get wardwiz domain name.
Author Name    : Ramkrushna Shelke
SR.NO			 :
Date           : 10 Oct 2014
***********************************************************************************************/
bool CSetupDLLApp::GetDomainName(LPTSTR pszDomainName, DWORD dwSize)
{
	bool bReturn = false;
	try
	{
		_tcscpy_s(pszDomainName, dwSize, L"act.vibraniumav.in");
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetDomainName");
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : GetMachineID
Description    : Function which writes Machine ID into Registry.
SR.NO			 : WRDWIZCOMMSRV_59
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
CString CSetupDLLApp::GetMachineID()
{
	CString csMachineID;
	try
	{
		TCHAR	szBIOSSerialNumber[0x38] = { 0 };		//56 bytes considered
		TCHAR	szCPUID[0x40] = { 0 };
		TCHAR	szClientID[0x80] = { 0 };

		TCHAR	szMACAddress[0x40] = { 0 };

		GetCPUID(szCPUID);
		if (!szCPUID[0])
		{
			AddLogEntry(L"### Failed to WriteMachineID2Registry::CPUID", 0, 0, true, SECONDLEVEL);
			return false;
		}

		GetBIOSSerialNumberSEH(szBIOSSerialNumber);
		if (!szBIOSSerialNumber[0])
		{

			//	ISSUE No : 163
			//	Some customers were not getting BIOS Serial Number, So we added MotherBoard Serial Number
			//	to make Unique Machine ID

			GetMotherBoardSerialNumberSEH(szBIOSSerialNumber);
			if (!szBIOSSerialNumber[0])
			{
				//GetCPUIDEx( szBIOSSerialNumber );
				AddLogEntry(L"### Before MAC", 0, 0, true, SECONDLEVEL);
				//GetMACAddress(szBIOSSerialNumber);
				GetMACAddress(szMACAddress);
				AddLogEntry(L"### After MAC", 0, 0, true, SECONDLEVEL);

				//GetVGAAdapterID( szBIOSSerialNumber );
				//if( !szBIOSSerialNumber[0] )		Changed on 21 10 2014 by Vilas
				if (!szMACAddress[0])
				{
					//GetMACAddress(szBIOSSerialNumber);
					AddLogEntry(L"### Before VGA", 0, 0, true, SECONDLEVEL);
					GetVGAAdapterID(szBIOSSerialNumber);
					if (!szBIOSSerialNumber[0])
					{
						AddLogEntry(L"### Failed to retrieve VGA ID", 0, 0, true, SECONDLEVEL);
						return false;
					}

					AddLogEntry(L"### After VGA", 0, 0, true, SECONDLEVEL);
				}

			}
		}

		if (szBIOSSerialNumber[0])
		{
			RemoveCharsIfExists(szBIOSSerialNumber, static_cast<int>(wcslen(szBIOSSerialNumber)), static_cast<int>(sizeof(szBIOSSerialNumber)), 0x20);
		}

		if (!szMACAddress[0])
			GetMACAddress(szMACAddress);

		int	i = static_cast<int>(wcslen(szCPUID));

		szClientID[0] = (TCHAR)i;
		//wsprintf( &szClientID[1], L"%s%s", szCPUID, szBIOSSerialNumber ) ;
		swprintf(&szClientID[1], 0x7E, L"%s%s%s", szCPUID, szBIOSSerialNumber, szMACAddress);
		//StringCbPrintf
		//Added to terminate MID 
		i = static_cast<int>(wcslen(szClientID));
		if (i > 0x7F)
			i = 0x7F;

		szClientID[i] = '\0';

		csMachineID = CString(szClientID);

	}
	catch(...)
	{
		AddLogEntry(L"### Exception in WriteMachineID2Registry", 0, 0, true, SECONDLEVEL);
		return L"";
	}
	return csMachineID;
}

/***********************************************************************************************
Function Name  : GetCPUID
Description    : Function which get the CPU from hardware ID.
SR.NO			 : WRDWIZCOMMSRV_60
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
DWORD CSetupDLLApp::GetCPUID(TCHAR *pszCPUID)
{
	__try
	{
		TCHAR	szData[0x10] = { 0 };
		int		b[4] = { 0 };

		wcscpy(pszCPUID, L"");

		for (int a = 0; a < 3; a++)
		{
			__cpuid(b, a);

			if ((a == 0 || a == 1) && b[0])
			{
				wsprintf(szData, L"%X", b[0]);
				//i = wcslen( szTemp ) ;
				wcscat(pszCPUID, szData);
			}

			if (a == 2)
			{
				if (b[0])
				{
					wsprintf(szData, L"%X", b[0]);
					wcscat(pszCPUID, szData);
				}

				if (b[1])
				{
					wsprintf(szData, L"%X", b[1]);
					wcscat(pszCPUID, szData);
				}

				if (b[2])
				{
					wsprintf(szData, L"%X", b[2]);
					wcscat(pszCPUID, szData);
				}

				if (b[3])
				{
					wsprintf(szData, L"%X", b[3]);
					wcscat(pszCPUID, szData);
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::GetCPUID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : GetBIOSSerialNumber
Description    : Function which gets BIOS Number.
SR.NO			 : WRDWIZCOMMSRV_61
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
DWORD CSetupDLLApp::GetBIOSSerialNumberSEH(TCHAR *psMotherBoardSerialNumber)
{
	__try
	{
		return GetBIOSSerialNumber(psMotherBoardSerialNumber);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return 0x01;
	}

	return 0x02;
}


DWORD CSetupDLLApp::GetBIOSSerialNumber(TCHAR *pszBIOSSerialNumber)
{
	DWORD				dwRet = 0x00;

	HRESULT				hres = S_OK;
	HRESULT				hr = S_OK;
	IWbemLocator		*pLoc = NULL;
	IWbemServices		*pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject	*pclsObj = NULL;
	ULONG uReturn = 0;

	VARIANT				vtProp;
	CString				hh = L"";

	try
	{
		//static bool			g_bCoInitializeSecurityCalled = false;
		/*
		hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
		if( hres != S_OK )
		{
		ErrorDescription(hres);
		dwRet = 0x01 ;
		goto Cleanup ;
		}
		*/
		if (!g_bCoInitializeSecurityCalled)
		{

			hres = CoInitializeEx(0, COINIT_MULTITHREADED);
			/*if( hres != S_OK )
			{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
			}*/

			hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
			/*if( hres != S_OK )
			{
			ErrorDescription(hres);
			dwRet = 0x01 ;
			goto Cleanup ;
			}*/

			g_bCoInitializeSecurityCalled = true;
		}

		hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
		if (hres != S_OK)
		{
			ErrorDescription(hres);
			dwRet = 0x02;
			goto Cleanup;
		}

		if (!pLoc)
		{
			ErrorDescription(hres);
			dwRet = 0x03;
			goto Cleanup;
		}

		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
		if (hres != S_OK)
		{
			ErrorDescription(hres);
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!pSvc)
		{
			ErrorDescription(hres);
			dwRet = 0x05;
			goto Cleanup;
		}

		hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
		if (hres != S_OK)
		{
			ErrorDescription(hres);
			dwRet = 0x06;
			goto Cleanup;
		}

		hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_BIOS"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
		if (hres != S_OK)
		{
			ErrorDescription(hres);
			dwRet = 0x07;
			goto Cleanup;
		}

		while (pEnumerator)
		{
			hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
			if (0 == uReturn)
				break;

			if (NULL == pclsObj)
				break;

			hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
			hh = vtProp.bstrVal;
			VariantClear(&vtProp);
			pclsObj->Release();

			hh.Trim();
			if (hh.GetLength())
			{
				wsprintf(pszBIOSSerialNumber, L"%s", hh.Trim());
				break;
			}

		}
	}
	catch (...)
		//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetBIOSSerialNumber", 0, 0, true, SECONDLEVEL);
	}
Cleanup:
	if (pSvc)
		pSvc->Release();

	if (pLoc)
		pLoc->Release();

	if (pEnumerator)
		pEnumerator->Release();

	CoUninitialize();

	return dwRet;
}



/***********************************************************************************************
Function Name  : RemoveCharsIfExists
Description    : Function which removes blank characters from BIOS Number.
SR.NO			 : WRDWIZCOMMSRV_62
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
DWORD CSetupDLLApp::RemoveCharsIfExists(TCHAR *pszBIOSSerialNumber, int iLen, int iSize, TCHAR chRemove)
{
	__try
	{
		TCHAR	szTemp[56] = { 0 };

		if ((iLen <= 0) || iLen > 56)
			return 1;

		int i = 0x00, j = 0x00;

		for (i = 0; i<iLen; i++)
		{
			if (pszBIOSSerialNumber[i] != chRemove)
				szTemp[j++] = pszBIOSSerialNumber[i];
		}

		szTemp[j] = '\0';

		ZeroMemory(pszBIOSSerialNumber, iSize);
		wcscpy(pszBIOSSerialNumber, szTemp);
	}
	//catch(...)
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in RemoveCharsIfExists", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return 0;
}


/***************************************************************************************************
*  Function Name  : GetMotherBoardSerialNumberSEH()
*  Description    : Gets MotherBoard number through WMI
*  Author Name    : Vilas                                                                                      *
*  Date			  :	08- Sept-2014 - 12 jul -2014
*  Modified Date  :
****************************************************************************************************/
DWORD CSetupDLLApp::GetMotherBoardSerialNumberSEH(TCHAR *psMotherBoardSerialNumber)
{
	__try
	{
		return GetMotherBoardSerialNumber(psMotherBoardSerialNumber);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		return 0x01;
	}

	return 0x02;
}

DWORD CSetupDLLApp::GetMotherBoardSerialNumber(TCHAR *psMotherBoardSerialNumber)
{
	//AddLogEntry(L">>> Inside GetBIOSSerialNumber");

	DWORD				dwRet = 0x00;

	HRESULT				hres = S_OK;
	HRESULT				hr = S_OK;
	IWbemLocator		*pLoc = NULL;
	IWbemServices		*pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject	*pclsObj = NULL;
	ULONG uReturn = 0;

	VARIANT				vtProp;
	CString				hh = L"";

	try
	{
		//static bool			g_bCoInitializeSecurityCalled = false;

		//AddLogEntry(L">>> in GetBIOSSerialNumber::CoInitializeEx before");
		/*
		hres = CoInitializeEx(0, COINIT_MULTITHREADED) ;
		if( hres != S_OK )
		{
		AddLogEntry(L"### Exception in GetBIOSSerialNumber::CoInitializeEx");
		//ErrorDescription(hres);
		dwRet = 0x01 ;
		goto Cleanup ;
		}
		*/
		if (!g_bCoInitializeSecurityCalled)
		{
			hres = CoInitializeEx(0, COINIT_MULTITHREADED);
			//if( hres != S_OK )
			//{
			//	AddLogEntry(L"### Exception in GetBIOSSerialNumber::CoInitializeEx");
			//	//ErrorDescription(hres);
			//	dwRet = 0x01 ;
			//	goto Cleanup ;
			//}

			//AddLogEntry(L">>>> in before GetBIOSSerialNumber::CoInitializeSecurity ");
			hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
			//if( hres != S_OK )
			//{
			//	AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::CoInitializeSecurity");
			//	//ErrorDescription(hres);
			//	dwRet = 0x01 ;
			//	goto Cleanup ;
			//}

			g_bCoInitializeSecurityCalled = true;
		}

		//AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::CoCreateInstance");
		hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
		if (hres != S_OK)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::CoCreateInstance");
			//ErrorDescription(hres);
			dwRet = 0x02;
			goto Cleanup;
		}

		if (!pLoc)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::Locator failed");
			//ErrorDescription(hres);
			dwRet = 0x03;
			goto Cleanup;
		}

		AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::ConnectServer");
		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
		if (hres != S_OK)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ConnectServer");
			//ErrorDescription(hres);
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!pSvc)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::WebServicce failed");
			//ErrorDescription(hres);
			dwRet = 0x05;
			goto Cleanup;
		}

		AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::CoSetProxyBlanket");
		hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
		if (hres != S_OK)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ConnectServer::CoSetProxyBlanket");
			//ErrorDescription(hres);
			dwRet = 0x06;
			goto Cleanup;
		}

		//AddLogEntry(L">>>> in before GetBIOSSerialNumber::ExecQuery");
		hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_BaseBoard"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
		if (hres != S_OK)
		{
			AddLogEntry(L"### Exception in GetMotherBoardSerialNumber::ExecQuery");
			//ErrorDescription(hres);
			dwRet = 0x07;
			goto Cleanup;
		}

		//AddLogEntry(L">>>> in before GetMotherBoardSerialNumber::while( pEnumerator )");
		while (pEnumerator)
		{

			//AddLogEntry(L">>> inside GetBIOSSerialNumber::while( pEnumerator )");

			hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
			if (0 == uReturn)
			{
				AddLogEntry(L"### Failed in GetMotherBoardSerialNumber::pEnumerator->Next");
				break;
			}

			if (NULL == pclsObj)
			{
				AddLogEntry(L"### Failed in GetMotherBoardSerialNumber::Web Class Object");
				break;
			}

			hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
			hh = vtProp.bstrVal;
			VariantClear(&vtProp);
			pclsObj->Release();

			//AddLogEntry(L">>> inside GetMotherBoardSerialNumber::before hh.GetLength()");

			hh.Trim();
			if (hh.GetLength() /* wcslen(vtProp.bstrVal) > 0x02 */)
			{

				wsprintf(psMotherBoardSerialNumber, L"%s", hh.Trim());
				//wsprintf(psMotherBoardSerialNumber, L"%s", vtProp.bstrVal ) ;
				AddLogEntry(L">>> Got GetMotherBoardSerialNumber::%s", psMotherBoardSerialNumber);

				break;
			}

			//AddLogEntry(L">>> inside GetBIOSSerialNumber::after hh.GetLength()");
		}
	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch (...)
	{
		AddLogEntry(L"### Exception in GetMotherBoardSerialNumber");
	}

Cleanup:

	//AddLogEntry(L">>> inside GetBIOSSerialNumber::before Cleanup");

	if (pSvc)
		pSvc->Release();

	if (pLoc)
		pLoc->Release();

	if (pEnumerator)
		pEnumerator->Release();

	CoUninitialize();

	//AddLogEntry(L">>> inside GetBIOSSerialNumber::after Cleanup");

	//AddLogEntry(L">>> Out GetBIOSSerialNumber");

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : GetMACAddress()
*  Description    : returns MAC Address which is 6 bytes
*  Author Name    : Vilas                                                                                      *
*  Date			  :	26-Sept-2014
*  Modified Date  :	27-Sept-2014
****************************************************************************************************/
DWORD CSetupDLLApp::GetMACAddress(TCHAR *pMacAddress, LPTSTR lpszOldMacID)
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapInfo = NULL;

	DWORD dwBufLen = 0x00;
	DWORD dwCount = 0x00;
	DWORD dwRet = 0x00, dwDivisor = 0x00, dwStatus = 0x00;

	__try
	{
		dwStatus = GetAdaptersInfo(pAdapterInfo, &dwBufLen);
		if (dwStatus != ERROR_BUFFER_OVERFLOW)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		dwDivisor = sizeof IP_ADAPTER_INFO;

		if (sizeof time_t == 0x08)
			dwDivisor -= 8;

		dwCount = dwBufLen / dwDivisor;
		if (!dwCount)
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		pAdapterInfo = new IP_ADAPTER_INFO[dwCount];
		if (!pAdapterInfo)
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		//TCHAR	szMacAddress[64] = {0};

		ZeroMemory(pAdapterInfo, dwBufLen);
		if (GetAdaptersInfo(pAdapterInfo, &dwBufLen) != ERROR_SUCCESS)
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		//TCHAR	szMacAddress[0x400] = {0};
		TCHAR	szDescp[0x200] = { 0 };
		//bool	bTypeIEEE80211 = false;

		pAdapInfo = pAdapterInfo;

		while (pAdapInfo)
		{
			//MultiByteToWideChar( CP_ACP, 0, pAdapInfo->Description, -1, szDescp, sizeof(TCHAR)*0x1FF ) ;

			//Added to get only Ethernet address
			if ((strstr(pAdapInfo->Description, "Virtual ") == NULL) &&
				(strstr(pAdapInfo->Description, "Bluetooth ") == NULL) &&
				(strstr(pAdapInfo->Description, "Wireless ") == NULL) &&
				(strstr(pAdapInfo->Description, "(PAN)") == NULL) &&
				(strstr(pAdapInfo->Description, "Wi-Fi ") == NULL) &&
				(strstr(pAdapInfo->Description, "WiFi ") == NULL))
			{
				wsprintf(pMacAddress, L"%02X%02X%02X%02X%02X%02X", pAdapInfo->Address[0],
					pAdapInfo->Address[1], pAdapInfo->Address[2],
					pAdapInfo->Address[3], pAdapInfo->Address[4],
					pAdapInfo->Address[5]);

				if (lpszOldMacID == NULL)
				{
					AddLogEntry(L">>> MACID: %s", pMacAddress, 0, true, ZEROLEVEL);
					break;
				}

				int iLen = _tcslen(lpszOldMacID);
				if (iLen != 0x00 && (iLen - 0x0C) >= 0x0C)
				{
					if (memcmp(&pMacAddress[0], &lpszOldMacID[iLen - 0x0C], 0x0C) == 0)
					{
						AddLogEntry(L">>> MACID: %s", pMacAddress, 0, true, ZEROLEVEL);
						break;
					}
				}
			}

			pAdapInfo = pAdapInfo->Next;
		}

		if (!wcslen(pMacAddress))
		{
			AddLogEntry(L"### Failed in GetMACAddress", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetMACAddress", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	if (pAdapterInfo)
		delete[] pAdapterInfo;

	pAdapterInfo = NULL;

	return dwRet;
}


bool CSetupDLLApp::GetVGAAdapterID(TCHAR *pszDispAdapterID)
{

	HDEVINFO        hDevInfo = 0L;
	SP_DEVINFO_DATA spDevInfoData = { 0 };
	SP_CLASSIMAGELIST_DATA _spImageData = { 0 };

	TCHAR	szMotherBoradRes[512] = { 0 };

	short	wIndex = 0;
	bool	bVGA = false;

	_try
	{

		hDevInfo = SetupDiGetClassDevs(0L, 0L, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_PROFILE);
		if (hDevInfo == (void*)-1)
		{
			AddLogEntry(L"#### Failed in GetVGAAdapterID::SetupDiGetClassDevs");
			return 1;
		};

		wIndex = 0;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		int iDev = 0x00;

		while (1)
		{

			TCHAR  szBuf[MAX_PATH] = { 0 };
			TCHAR  szID[LINE_LEN + 1] = { 0 };
			TCHAR                   szName[64] = { 0 };

			if (SetupDiEnumDeviceInfo(hDevInfo,
				wIndex,
				&spDevInfoData))
			{

				short wImageIdx = 0;
				short wItem = 0;

				if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
					&spDevInfoData,
					SPDRP_CLASS, //SPDRP_DEVICEDESC,
					0L,
					(PBYTE)szBuf,
					2048,
					0))
				{
					wIndex++;
					continue;
				};

				SetupDiGetClassImageIndex(&_spImageData, &spDevInfoData.ClassGuid, (int*)&wImageIdx);

				TCHAR                   szPath[MAX_PATH] = { 0 };
				DWORD                  dwRequireSize = 0x00;

				if (!SetupDiGetClassDescription(&spDevInfoData.ClassGuid,
					szBuf,
					MAX_PATH,
					&dwRequireSize))
				{
					wIndex++;
					continue;
				};


				SetupDiGetDeviceInstanceId(hDevInfo, &spDevInfoData, szID, LINE_LEN, 0);
				if (SetupDiGetDeviceRegistryProperty(hDevInfo,
					&spDevInfoData,
					SPDRP_FRIENDLYNAME,
					0L,
					(PBYTE)szName,
					63,
					0))
				{
					//DisplayDriverDetailInfo(hItem, nIdTree, szName, wImageIdx, wImageIdx);
					//AddNewDeviceNode(spDevInfoData.ClassGuid, szName, szID, szPath, wIndex, wOrder);
				}
				else if (SetupDiGetDeviceRegistryProperty(hDevInfo,
					&spDevInfoData,
					SPDRP_DEVICEDESC,
					0L,
					(PBYTE)szName,
					63,
					0))
				{
					//DisplayDriverDetailInfo(hItem, nIdTree, szName, wImageIdx, wImageIdx);
					//AddNewDeviceNode(spDevInfoData.ClassGuid, szName, szID, szPath, wIndex, wOrder);
					//                    if (!GetFirmwareEnvironmentVariable(szName, (LPCSTR)&spDevInfoData.ClassGuid, szBuf, 127))
					//                        ShowErrorMsg(_hDlg, GetLastError(), "GetFirmwareEnvironmentVariable");
				};

				if ((_wcsicmp(szBuf, L"Display adapters") == 0) &&
					(_wcsicmp(szName, L"Standard VGA Graphics Adapter") == 0))
				{

					if (wcslen(szID) > 0x04)
					{
						wcscpy_s(pszDispAdapterID, 0x37, &szID[0x08]);
						bVGA = true;
					}

					break;
				}

			}

			wIndex++;
			if (wIndex > 0x80)
				break;
		}

		if (!pszDispAdapterID[0])
			//AddLogEntry(L"### GetVGAAdapterID::failed to VGA not found");
			wcscpy_s(pszDispAdapterID, 0x37, L"_NULL");
	}
	//catch(...)
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetVGAAdapterID", 0, 0, true, SECONDLEVEL);
	}

	return bVGA;
}

/***************************************************************************
Function Name  : ErrorDescription
Description    : fucntion which returns the COM error
Author Name    : Ram Shelke
Date           : 25th June 2014
****************************************************************************/
void CSetupDLLApp::ErrorDescription(HRESULT hr)
{
	if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
		hr = HRESULT_CODE(hr);

	TCHAR szErrMsg[MAX_PATH] = { 0 };

	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&szErrMsg, 0, NULL) != 0)
	{
		AddLogEntry(L"### COM Error: %s\n", szErrMsg);
		LocalFree(szErrMsg);
	}
}


/***************************************************************************************************
*  Function Name  : GenerateGUIID
*  Description    : Function to use GUI ID
*  Author Name    : Ramkrushna Shelke
*  Date           :	28 Jan 2023
****************************************************************************************************/
CString CSetupDLLApp::GenerateGUIID()
{
	CString csRetString = EMPTY_STRING;
	// Create a new uuid
	UUID uuid;
	RPC_STATUS ret_val = ::UuidCreate(&uuid);

	if (ret_val == RPC_S_OK)
	{
		// convert UUID to LPWSTR
		WCHAR* wszUuid = NULL;
		::UuidToStringW(&uuid, (RPC_WSTR*)&wszUuid);
		if (wszUuid != NULL)
		{
			//TODO: do something with wszUuid
			csRetString.Format(L"%s", wszUuid);
			csRetString.MakeUpper();

			// free up the allocated string
			::RpcStringFreeW((RPC_WSTR*)&wszUuid);
			wszUuid = NULL;
		}
	}
	return csRetString;
}

DWORD CSetupDLLApp::ExtractDate(TCHAR *pTime, TCHAR *newKey, LPSYSTEMTIME lpServerTime, LPDWORD lpdwServerDays)
{
	__try
	{
		SYSTEMTIME	ServerTime = { 0 };
		TCHAR	szTemp[8] = { 0 };
		TCHAR	*chSep = L"#";
		TCHAR	pszTime[64] = { 0 };

		//check here desired lenth else return with failed.
		if (wcslen(pTime) > 0x32)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x01;
		}

		TCHAR *nKey = wcstok(pTime, chSep);
		if (!nKey)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x01;
		}

		memcpy(newKey, nKey, 50);

		TCHAR	*pDateTime = wcstok(NULL, chSep);
		if (!pDateTime)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x01;
		}

		TCHAR	*pDays = wcstok(NULL, chSep);
		TCHAR	*pResponseCode = wcstok(NULL, chSep);

		if (!pDays)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x02;
		}

		if (!pResponseCode && wcslen(pResponseCode) != 3)
		{
			AddLogEntry(L"### Invalid Response code from server %s ", pTime, 0, true, SECONDLEVEL);
			return 0x03;
		}

		DWORD	dwDays = 0x00;
		DWORD	dwResponseCode = 0x00;

		if (wcslen(pDateTime) > 0x3F)
		{
			AddLogEntry(L"### Invalid Response code from server Time ", 0, 0, true, SECONDLEVEL);
			return 0x14;
		}

		wcscpy(pszTime, pDateTime);

		//Checking for if no of days is -ve.
		if (pDays[0] == '-')
		{
			AddLogEntry(L"### No of days is in -ve returning from CRegistrationSecondDlg::ExtractDate ", 0, 0, true, SECONDLEVEL);
			return 0x02;
		}

		swscanf(pResponseCode, L"%d", &dwResponseCode);

		if (dwResponseCode == 0x01)
		{
			AddLogEntry(L"### Registration key already been used", 0, 0, true, SECONDLEVEL);
			return MACHINEIDMISMATCH;
		}

		if (dwResponseCode == 0x02)
		{
			AddLogEntry(L"### Email ID for Registration is invalid", 0, 0, true, SECONDLEVEL);
			return INVALIDEMAILID;
		}

		if (dwResponseCode == 0x03)
		{
			AddLogEntry(L"### Country code invalid", 0, 0, true, SECONDLEVEL);
			return COUNTRYCODEINVALID;
		}

		if (dwResponseCode == 0x04)
		{
			AddLogEntry(L"### Invalid Registration Number", 0, 0, true, SECONDLEVEL);
			return INVALIDREGNUMBER;
		}

		if (dwResponseCode == 0x05)
		{
			AddLogEntry(L"### Invalid Vibranium Product key", 0, 0, true, SECONDLEVEL);
			return INVALIDPRODVERSION;
		}

		//006 - Invalid Agent, Means the request is not came from Wardwiz client.
		//007 - Database connectivity fails.
		if (dwResponseCode == 0x08)
		{
			AddLogEntry(L"### Failed to update the user information on server, need to resend", 0, 0, true, SECONDLEVEL);
			return USERINFOUPDATEFAILD;
		}

		swscanf(pDays, L"%d", &dwDays);
		if (!dwDays)
		{
			AddLogEntry(L"### Product Expired, Number of days left %s", pDays, 0, true, SECONDLEVEL);
			return PRODUCTEXPIRED;
		}
		else
		{
			*lpdwServerDays = dwDays;
		}

		memcpy(szTemp, pszTime, 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wDay);
		if (!ServerTime.wDay)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x12;
		}

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[3], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wMonth);
		if (!ServerTime.wMonth)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x13;
		}

		DWORD	dwYear = 0x00;

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[6], 4 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &dwYear);
		if (!dwYear)
		{
			AddLogEntry(L"### Invalid Date from Server: %s ", pTime, 0, true, SECONDLEVEL);
			return 0x14;
		}

		ServerTime.wYear = (WORD)dwYear;

		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[11], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wHour);


		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[14], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wMinute);


		memset(szTemp, 0x00, 8 * sizeof(TCHAR));
		memcpy(szTemp, &pszTime[17], 2 * sizeof(TCHAR));
		swscanf(szTemp, L"%d", &ServerTime.wSecond);

		*lpServerTime = ServerTime;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::ExtractDate");
	}
	return 0;
}

DWORD CSetupDLLApp::AddRegistrationDataInFile(LPBYTE lpData, DWORD dwSize)
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	if (!GetModulePath(szModulePath, MAX_PATH))
	{
		AddLogEntry(L"### Faile to GetModulePath in CWardwizRegDataOperations::AddRegistrationDataInFile", 0, 0, true, SECONDLEVEL);
		return false;
	}

	CString	strUserRegFile = szModulePath;
	strUserRegFile = strUserRegFile + L"\\VBUSERREG.DB";

	HANDLE	hFile = INVALID_HANDLE_VALUE;
	DWORD	dwRet = 0x00, dwBytesWrite;

	AVACTIVATIONINFO	ActInfo = { 0 };

	hFile = CreateFile(strUserRegFile, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwRet = 0x01;
		goto Cleanup;
	}

	memcpy(&ActInfo, lpData, dwSize);

	if (DecryptData((LPBYTE)&ActInfo, dwSize))
	{
		dwRet = 0x02;
		goto Cleanup;
	}

	dwBytesWrite = 0x00;

	WriteFile(hFile, &ActInfo, dwSize, &dwBytesWrite, NULL);
	if (dwSize != dwBytesWrite)
		dwRet = 0x03;

Cleanup:

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;


	return dwRet;
}

DWORD CSetupDLLApp::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
{
	try
	{
		if (IsBadWritePtr(lpBuffer, dwSize))
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			if (lpBuffer[iIndex] != 0)
			{
				if ((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
				{
					lpBuffer[iIndex] = lpBuffer[iIndex];
				}
				else
				{
					lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
					jIndex++;
				}
				if (jIndex == WRDWIZ_KEYSIZE)
				{
					jIndex = 0x00;
				}
				if (iIndex >= dwSize)
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::DecryptData");
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SpreadRegistrationFilesInSystem
*  Description    : Drops Registered user  .db file in system
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
DWORD CSetupDLLApp::SpreadRegistrationFilesInSystem()
{
	try
	{
		TCHAR	szAllUserPath[512] = { 0 };

		TCHAR	szSource[512] = { 0 };
		TCHAR	szSource1[512] = { 0 };
		TCHAR	szDestin[512] = { 0 };
		TCHAR	szDestin1[512] = { 0 };

		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 511);
		wsprintf(szDestin, L"%s\\Vibranium", szAllUserPath);
		
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L"### Faile to GetModulePath in CWardwizRegDataOperations::AddRegistrationDataInFile", 0, 0, true, SECONDLEVEL);
			return false;
		}

		CString	strUserRegFile = szModulePath;

		wcscpy(szDestin1, szDestin);

		wsprintf(szSource1, L"%s\\VBUSERREG.DB", szModulePath);
		wcscat(szDestin1, L"\\VBUSERREG.DB");

		CopyFile(szSource1, szDestin1, FALSE);

		memset(szDestin1, 0x00, 512 * sizeof(TCHAR));
		wsprintf(szDestin1, L"%c:\\VBUSERREG.DB", szAllUserPath[0]);

		CopyFile(szSource1, szDestin1, FALSE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SpreadRegistrationFilesInSystem");
	}
	return 0;
}