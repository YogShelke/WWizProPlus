/********************************************************************************************************** 
   Program Name          : SetupDLL.cpp
   Description           : Defines the initialization routines for the DLL.
   Author Name           : Ramkrushna Shelke                                                                                  Date Of Creation      : 7/25/2014
   Version No            : 1.0.0
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description
***********************************************************************************************************/
#include "stdafx.h"
#include "CloseAll.h"
#include "Registry.h"
#include "EnumProcess.h"
#include "iSpySrvMgmt.h"
#include "ITinRegWrapper.h"
#include "CommonFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CWinApp theApp;

using namespace std;

bool CloseAllRunningApplication();
void RefreshTaskbarNotificationArea();
void UnregisterComDll(CString csAppPath);

/***************************************************************************
  Function Name  : _tmain
  Description    : main function
  Author Name    : Ramkrushna Shelke
  Date           : 2/10/2014
****************************************************************************/
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	CloseAllRunningApplication();

	return nRetCode;
}

/***************************************************************************
  Function Name  : CloseAllRunningApplication
  Description    : Function to close WardWiz running application
  Author Name    : Ramkrushna Shelke
  S.R. No        : WWCLOSEALL_0002
  Date           : 2/10/2014
  Modified		 : Neha Gharge 3 July,2015 . Retry for 3 times 
****************************************************************************/
bool CloseAllRunningApplication()
{
	bool bISFullPath = false;
	
	CITinRegWrapper objReg;
	TCHAR szValue[MAX_PATH] = {0};
	DWORD dwSize = sizeof(szValue);
	if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"AppFolder", szValue, dwSize) != 0)
	{
		AddLogEntry(L"### Error GetRegistryValueData in CSetupDLLApp::CSetupDLLApp", 0, 0, true, SECONDLEVEL);
	}  
	
	if(_tcslen(szValue) > 0)
	{
		bISFullPath = true;
	}

	CStringArray objcsaWardWizProcesses;
	objcsaWardWizProcesses.Add(L"WRDWIZAVUI.EXE");
	objcsaWardWizProcesses.Add(L"VBTRAY.EXE");
	objcsaWardWizProcesses.Add(L"VBAUTORUNSCN.EXE");
	objcsaWardWizProcesses.Add(L"VBTEMPCLR.EXE");
	objcsaWardWizProcesses.Add(L"VBUSBVAC.EXE");
	objcsaWardWizProcesses.Add(L"VBUSBDETECTUI.EXE");
	objcsaWardWizProcesses.Add(L"VBCRYPT.EXE");
	objcsaWardWizProcesses.Add(L"VBSCANNER.EXE");

	CString csModulePath = szValue;
	CString csAppPath; 
	CEnumProcess objEnumProcess;
	DWORD dwRetry = 0x00;
	for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
	{
		dwRetry = 0x00;
		csAppPath = csModulePath + objcsaWardWizProcesses.GetAt(iIndex);
		while (true)
		{
			if (objEnumProcess.IsProcessRunning(csAppPath, false, bISFullPath, true))
			{
				if (dwRetry < 0x03)
				{
					int iRet = MessageBox(NULL, L"Some of the application of Vibranium is running.\nPlease close & click on retry button.", L"Vibranium", MB_ICONEXCLAMATION | MB_RETRYCANCEL);
					if (iRet == IDCANCEL)
					{
						MessageBox(NULL, L"Please close all Vibranium running application(s) before uninstalling.", L"Vibranium", MB_ICONINFORMATION | MB_OK);
					}
					dwRetry++;
				}
				else
				{

					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	CISpySrvMgmt		iSpySrvMgmtObj ;
	DWORD dwReturn = iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME);
	if( dwReturn != 0x00)
	{
		AddLogEntry(L"### Failed to remove WardwizComsrv service",0,0,true,SECONDLEVEL);
	}

	dwReturn = iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME);
	if( dwReturn != 0x00)
	{
		AddLogEntry(L"### Failed to remove WardwizUPDATESERVICE",0,0,true,SECONDLEVEL);
	}

	csAppPath = csModulePath + L"VBSHELLEXT.DLL";
	if (PathFileExists(csAppPath))
	{
		UnregisterComDll(csAppPath);
	}

	csAppPath = csModulePath + L"VBSHELLEXT_OLD.DLL";
	if (PathFileExists(csAppPath))
	{
		UnregisterComDll(csAppPath);
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
void UnregisterComDll(CString csAppPath)
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