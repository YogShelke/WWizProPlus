// WardWizUnInstCon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include "afxdialogex.h"
#include "iSpySrvMgmt.h"
#include <Wtsapi32.h>
#include "iTinRegWrapper.h"
#include "CScannerLoad.h"
#include "CSecure64.h"
#include "DriverConstants.h"
#include "WardWizOSversion.h"
#include "Enumprocess.h"
#include "ISpyCommunicator.h"
#include "sqlite3.h"
#include "Constants.h"
#include "WardwizLangManager.h"
#include "nfapi.h"

#ifndef _C_API
using namespace nfapi;
#endif

void InitializeDataMember();
bool StartUninstallProgram();
void StopProtectionDrivers();
bool CloseAllAplication(bool bIsReInstall);
void CloseApp();

bool DeleteFiles(std::vector<CString>	&vFilePathLists);
bool DeleteFolder(std::vector<CString>	&vFolderPathLists);
bool DeleteRegistryKeys();
void DeleteAllAppShortcut();
void DeleteFinished();
void UninstallationMessage();
void UnregisterNFDrivers();
void GetNFAPIDriverPath();
void DeleteNFDrivers(CString csWrdWizFLTDriverPath);
void RemoveDriverRegistryKeyAndPauseDrivers();
void ShowFinishedPage();
void RestoreWindow();
void EnumFolderToCountFiles(LPCTSTR pstr);
DWORD RemoveFilesUsingSHFileOperation(TCHAR *pFolder);
bool GetCloseAction4OutlookIfRunning(bool bIsReInstall);
CString GetModuleFilePath();
BOOL RegDelnode(HKEY hKeyRoot, CString csSubKey);
BOOL RegDelnodeRecurse(HKEY hKeyRoot, CString csSubKey);
void UnregisterComDll(CString csAppPath);

int							m_iTotalDeletedFileCount;
bool						m_bMoveFileExFlag;
bool						m_bUninstallCmd;
bool						m_bRestartReq;
bool						m_bUserSettings;
bool						m_bPassDbMngr;
bool						m_bQuarantine;
CString						m_csProdKeyName;
DWORD						m_dwProductID;
CString						m_csAppPath;
CString						m_csProductName;
CString						m_csWardWizPath;
TCHAR						m_szTempPath[512];
int							m_iTotalFileCount;

CStringArray				m_csArrRegistryEntriesDel;
std::vector<CString>		m_vFolderPathLists;
std::vector<CString>		m_vFilePathLists;

CWardwizLangManager			m_objwardwizLangManager;


/***************************************************************************************************
*  Function Name  : InitializeDataMember
*  Description    : All data members are initialized by initial values. 
*  Author Name    : Amol J.
*  Date           : 16 Mar 2018
****************************************************************************************************/
void InitializeDataMember()
{
	m_iTotalDeletedFileCount = 0;
	m_iTotalFileCount = 0;
	m_bMoveFileExFlag = false;
	m_bUninstallCmd = false;
	m_bRestartReq = false;
	m_bUserSettings = false;
	m_bPassDbMngr = false;
	m_bQuarantine = false;
	m_dwProductID = 0x00;
	m_csProdKeyName = L"SOFTWARE\\Vibranium";
	m_csAppPath = L"";
	m_csProductName = L"";
	m_csWardWizPath = L"";
	m_dwProductID = 3;

	m_csArrRegistryEntriesDel.RemoveAll();
	m_csArrRegistryEntriesDel.Add(L"AppFolder");
	m_csArrRegistryEntriesDel.Add(L"AppVersion");
	m_csArrRegistryEntriesDel.Add(L"DataBaseVersion");
	m_csArrRegistryEntriesDel.Add(L"dwScanLevel");
	m_csArrRegistryEntriesDel.Add(L"dwProductID");
}

/***************************************************************************************************
*  Function Name  : StopProtectionDrivers
*  Description    : stops the driver protection
*  Author Name    : Lalit K.
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void StopProtectionDrivers()
{
	// TODO: Add your control notification handler code here
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int OsType = 0;
	bool returnStatus = false;

	try
	{
		OsType = objGetOSVersion.DetectClientOSVersion();
		if (OsType == 5)
		{
			cSecure64Obj.RegisterProcessId(WLSRV_ID_NINE); // self protection
			scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection
			//StopAndUninstallProcessProtection(OsType);
			//StopAndUninstallScanner();
		}
		else
			if (OsType == 10)
			{
				//StopAndUninstallScanner();
				scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection

			}
			else
			{
				cSecure64Obj.RegisterProcessId(WLSRV_ID_NINE); // self protection
				scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection
				//StopAndUninstallProcessProtection(OsType);
				//StopAndUninstallScanner();
			}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StopProtectionDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : CloseAllAplication
Description    : Function which closes Runing Application.
Author Name    :
Date           :
****************************************************************************/
bool CloseAllAplication(bool bIsReInstall)
{
	AddLogEntry(L">>> CWardwizUninstSecondDlg : CloseAllAplication", 0, 0, true, ZEROLEVEL);
	bool bISFullPath = false;
	if (_tcslen(m_csAppPath) > 0)
	{
		bISFullPath = true;
	}

	CString csModulePath = m_csAppPath;

	if (bIsReInstall)
	{
		if (!GetCloseAction4OutlookIfRunning(true))
		{
			return false;
		}
	}

	//CString csAppPath = csModulePath + L"WRDWIZAVUI.EXE";
	CEnumProcess objEnumProcess;
	if (objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", false, false, false))
	{
		/*SCITER_VALUE svReturn = 0; //= m_svShowWrdWizRunningCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		if (theApp.m_bRetval == true)
		{
			svReturn = 1;
		}
		if (svReturn == 1)
		{
			objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", true, false, false);
		}
		else
		{
			return false;
		}*/
	}
	if (objEnumProcess.IsProcessRunning(L"VBUI.EXE", false, false, false))
	{
		/*SCITER_VALUE svReturn = 0;//= m_svShowWrdWizRunningCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		if (theApp.m_bRetval == true)
		{
			svReturn = 1;
		}
		if (svReturn == 1)
		{
			objEnumProcess.IsProcessRunning(L"VibraniumUI.EXE", true, false, false);

		}
		else
		{
			return false;
		}*/
	}
	//Issue Resolved : When we insert usb and uninstall the setup still usb scan is in process.
	//Resolved By : Nitin K. Date: 11th March 2015
	if (objEnumProcess.IsProcessRunning(L"VBUSBDETECTUI.EXE", false, false, false))
	{
		/*SCITER_VALUE svReturn = 0; // = m_svShowWrdWizRunningCB.call((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		CallNotificationMessage((SCITER_STRING)theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"));
		if (theApp.m_bRetval == true)
		{
			svReturn = 1;
		}
		if (svReturn == 1)
		{
			objEnumProcess.IsProcessRunning(L"VBUSBDETECTUI.EXE", true, false, false);
		}
		else
		{
			return false;
		}*/
	}
	CStringArray objcsaWardWizProcesses;
	objcsaWardWizProcesses.Add(L"VBSCANNER.EXE");
	objcsaWardWizProcesses.Add(L"VBTRAY.EXE");

	for (int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
	{
		CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);
		if (objEnumProcess.IsProcessRunning(csProcessName, true, false, false))
		{
			AddLogEntry(L">>> %s was running, Terminated", csProcessName, 0, true, SECONDLEVEL);
		}

	}

	//close and remove service here
	CISpySrvMgmt		iSpySrvMgmtObj;
	if (iSpySrvMgmtObj.StopServiceManually(WARDWIZSERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Service WardWizComsrv", 0, 0, true, SECONDLEVEL);
	}


	if (iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv", 0, 0, true, SECONDLEVEL);

	}

	//close and remove service here
	if (iSpySrvMgmtObj.StopServiceManually(WARDWIZUPDATESERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Update Service WardWizComsrv", 0, 0, true, SECONDLEVEL);
	}

	if (iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Update Service WardWizComsrv", 0, 0, true, SECONDLEVEL);

	}

	if (m_dwProductID == ELITE)
	{
		if (iSpySrvMgmtObj.UnInstallService(WARDWIZEPSCLIENTSERVNAME) != 0x00)
		{
			AddLogEntry(L"### Unable to UnInstall WardWizEPSCLIENTSERVNAME Service", 0, 0, true, SECONDLEVEL);
		}
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : GetModuleFilePath
*  Description    : Get the path where module is exist
*  Author Name    : Ramkrushna Shelke
*  SR_NO
*  Date           : 18 Sep 2013
****************************************************************************************************/
CString GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***************************************************************************
Function Name  : GetCloseAction4OutlookIfRunning
Description    : Closing outloog If Running.
Author Name    : Ramkrushna Shelke
S.R. No        : WRDWIZSETUPDLL_0004
Date           : 7/25/2014
****************************************************************************/
bool GetCloseAction4OutlookIfRunning(bool bIsReInstall)
{
	m_csProductName = L"BASIC";
	//No need to show the prompt for outlook close if the product is BASIC/ESSENTIAL
	if (m_csProductName == L"BASIC" || m_csProductName == L"ESSENTIAL")
	{
		return true;
	}

	while (true)
	{
		CEnumProcess objEnumProcess;
		if (objEnumProcess.IsProcessRunning(L"OUTLOOK.EXE", false, false, false))
		{
			CString csMessage;
			if (bIsReInstall)
				csMessage = m_objwardwizLangManager.GetString(L"IDS_OUTLOOK_CLOSE_REINSTALL_MSG");
			else
				csMessage = m_objwardwizLangManager.GetString(L"IDS_OUTLOOK_CLOSE_UNINSTALL_MSG");

			/*int iRet = MessageBox(csMessage, m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_RETRYCANCEL);
			if (iRet == IDCANCEL)
			{
				return false;
			}*/
		}
		else
		{
			break;
		}
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DeleteFiles
*  Description    : Deletes all the application files one by one
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
bool DeleteFiles(std::vector<CString>	&vFilePathLists)
{
	try
	{
		AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteFiles", 0, 0, true, ZEROLEVEL);
		CString csFileName = L"";
		int size = static_cast<int>(vFilePathLists.size());
		while (!vFilePathLists.empty())
		{
			csFileName = vFilePathLists.back();
			if (PathFileExistsW(csFileName))
			{
				SetFileAttributes(csFileName, FILE_ATTRIBUTE_NORMAL);
				if (DeleteFile(csFileName))
				{

					m_iTotalDeletedFileCount++;

					//Issue resolved: 0001864, Check if there are more files no need to wait.
					if (size < 0xC8)
					{
						Sleep(500);
					}
				}
				else
				{
					DWORD err = GetLastError();
					m_iTotalDeletedFileCount++;
					m_bRestartReq = true;

					CString csOldFileName = csFileName;
					CString csRenameFileName = csFileName + L"_OLD";
					if (!PathFileExists(csOldFileName))
					{
						AddLogEntry(L"### VBSHELLEXT.DLL file is not present :: DeleteFiles", 0, 0, true, SECONDLEVEL);
					}
					else
					{
						_wrename(csOldFileName, csRenameFileName);
					}

				}
			}
			vFilePathLists.pop_back();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CuninstallDlg::DeleteFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DeleteFolder
*  Description    : Deletes all the application folder one by one
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
bool DeleteFolder(std::vector<CString>	&vFolderPathLists)
{
	try
	{
		AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteFolder", 0, 0, true, ZEROLEVEL);
		CString csFolderName = NULL;
		int size = static_cast<int>(vFolderPathLists.size());
		while (!vFolderPathLists.empty())
		{
			csFolderName = vFolderPathLists.back();
			if (PathFileExistsW(csFolderName))
			{
				if (RemoveDirectory(csFolderName) == 0x00)
				{
					AddLogEntry(L">>> Directory removed successfully.", 0, 0, true, ZEROLEVEL);
				}
			}
			else
			{
				CString szMessage = NULL;
				szMessage.Format(L"### CVibraniumzUninstSecondDlg::Folder to be deleted on Restart : %s", csFolderName);
				AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
			}
			vFolderPathLists.pop_back();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CuninstallDlg::DeleteFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : DeleteRegistryKeys
*  Description    : Enumrate all the files in folder to check whether folder is empty or not.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 03 Feb 2015
*************************************************************************************************/
bool DeleteRegistryKeys()
{
	LONG		lResult = 0x00;
	HKEY hKey = NULL;
	DWORD dwRegCount = 0x00;
	CString szSubkey = NULL;
	CITinRegWrapper g_objReg;
	HKEY	hRootKey = NULL;
	hRootKey = HKEY_LOCAL_MACHINE;
	TCHAR	szSubKey[512] = { 0 };
	TCHAR	szValueName[512] = { 0 };
	_tcscpy(szSubKey, L"SOFTWARE\\Microsoft\\Windows");
	_tcscpy(szValueName, L"VibraniumUserInfo");

	AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteRegistryKeys", 0, 0, true, ZEROLEVEL);
	if (RegOpenKey(HKEY_LOCAL_MACHINE, m_csProdKeyName, &hKey) != ERROR_SUCCESS)
	{
		AddLogEntry(L"### Unable to open registry key", 0, 0, true, SECONDLEVEL);
		return FALSE;
	}

	dwRegCount = static_cast<DWORD>(m_csArrRegistryEntriesDel.GetCount());
	for (dwRegCount = 0; dwRegCount < static_cast<DWORD>(m_csArrRegistryEntriesDel.GetCount()); dwRegCount++)
	{
		szSubkey = L"";
		szSubkey = m_csArrRegistryEntriesDel.GetAt(dwRegCount);

		lResult = RegDeleteValue(hKey, szSubkey);
		if (lResult != ERROR_SUCCESS)
		{
			CString szMessage = NULL;
			szMessage.Format(L">>> CVibraniumUninstSecondDlg : DeleteRegistryKeys:: Unable to delete Registry key %s", szSubkey);
			AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
			return FALSE;

		}
	}

	if (!m_bUserSettings)
	{
		if (m_dwProductID == 2)
		{
			CString csKeyName = m_csProdKeyName + L"\\EmailScanSetting";
			lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, csKeyName);
			if (lResult == ERROR_SUCCESS)
			{
				AddLogEntry(L">>> successfully deleted key %s", csKeyName, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### failed to delete key %s", csKeyName, 0, true, SECONDLEVEL);
			}
		}

		CString csDelKey = L"SOFTWARE\\Vibranium";
		RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);

	}


	lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F2935F4-F652-4E3E-BD71-0F1B4C91D5B1}_is1"));
	if (lResult == ERROR_SUCCESS)
	{
		AddLogEntry(L">>> successfully deleted key SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F2935F4-F652-4E3E-BD71-0F1B4C91D5B1}_is1", 0, 0, true, FIRSTLEVEL);
	}
	else
	{
		AddLogEntry(L"### failed to delet key SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F2935F4-F652-4E3E-BD71-0F1B4C91D5B1}_is1 ", 0, 0, true, SECONDLEVEL);
	}

	if (g_objReg.DelRegistryValueName(hRootKey, szSubKey, szValueName))
		AddLogEntry(L">>> No Registry value for SOFTWARE\\Microsoft\\Windows", szSubKey, szValueName, true, FIRSTLEVEL);
	else
		AddLogEntry(L">>> Deleted Registry value for SOFTWARE\\Microsoft\\Windows", szSubKey, szValueName, true, FIRSTLEVEL);

	return TRUE;
}

/***************************************************************************************************
*  Function Name  : DeleteAllAppShortcut
*  Description    : Deletes all shortcuts of application
*  Author Name    : Lalit K
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void DeleteAllAppShortcut()
{
	try
	{
		AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteAllAppShortcut", 0, 0, true, ZEROLEVEL);
		TCHAR CommonAppPath[MAX_PATH];
		HRESULT result = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, CommonAppPath);
		CString obj = CommonAppPath;
		swprintf_s(CommonAppPath, _countof(CommonAppPath), L"%s\\%s", CommonAppPath, L"Vibranium");
		RemoveFilesUsingSHFileOperation(CommonAppPath);

		TCHAR CommonAppPathWardWiz[MAX_PATH];
		HRESULT hResult = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, CommonAppPathWardWiz);
		CString csObj = CommonAppPathWardWiz;
		swprintf_s(CommonAppPathWardWiz, _countof(CommonAppPathWardWiz), L"%s\\%s", CommonAppPathWardWiz, L"Vibranium");
		RemoveFilesUsingSHFileOperation(CommonAppPathWardWiz);

		TCHAR DeskTopIconPath[MAX_PATH];
		HRESULT result1 = SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, DeskTopIconPath);
		swprintf_s(DeskTopIconPath, _countof(DeskTopIconPath), L"%s\\%s", DeskTopIconPath, L"Vibranium.lnk");
		DeleteFile(DeskTopIconPath);
		//CString obj2= DeskTopIconPath;
		//AfxMessageBox(obj2);

		TCHAR CommDeskTopIconPath[MAX_PATH];
		HRESULT result2 = SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, CommDeskTopIconPath);
		swprintf_s(CommDeskTopIconPath, _countof(CommDeskTopIconPath), L"%s\\%s", CommDeskTopIconPath, L"Vibranium.lnk");
		DeleteFile(CommDeskTopIconPath);

		TCHAR QuickLaunchIconPath[512] = { 0 };
		HRESULT result3 = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, QuickLaunchIconPath);
		swprintf_s(QuickLaunchIconPath, _countof(QuickLaunchIconPath), L"%s\\%s", QuickLaunchIconPath, L"Microsoft\\Internet Explorer\\Quick Launch\\Vibranium.lnk");
		DeleteFile(QuickLaunchIconPath);

		TCHAR IconCacheDB[512] = { 0 };
		HRESULT result6 = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, IconCacheDB);
		swprintf_s(IconCacheDB, _countof(IconCacheDB), L"%s\\%s", IconCacheDB, L"IconCache.db");
		DeleteFile(IconCacheDB);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}
	catch (...)
	{
	}
}

/***************************************************************************
Function Name  : RemoveFilesUsingSHFileOperation
Description    : Function which removes file using SHFileOperation
This function can be used to copy, move, rename, or
delete a file system object.
Author Name    : Ramkrushna Shelke
SR.NO			 : EMAILSCANPLUGIN_0037
Date           : 07th Aug 2014
****************************************************************************/
DWORD RemoveFilesUsingSHFileOperation(TCHAR *pFolder)
{
	HRESULT			hr = 0x00;
	try
	{
		SHFILEOPSTRUCT	sfo = { 0 };

		TCHAR	szTemp[512] = { 0 };

		wsprintf(szTemp, L"%s\\*\0", pFolder);

		sfo.hwnd = NULL;
		sfo.wFunc = FO_DELETE;
		sfo.pFrom = szTemp;
		sfo.pTo = NULL;
		sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

		hr = SHFileOperation(&sfo);
		if (!hr)
		{
			_wrmdir((wchar_t *)pFolder);
			RemoveDirectory(pFolder);
		}
	}
	catch (...)
	{

	}
	return hr;
}

/***************************************************************************************************
*  Function Name  : DeleteFinished
*  Description    : called to kill the timer of status bar
*  Author Name    :	Nitin K
*  SR_NO          :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void DeleteFinished()
{
	AddLogEntry(L">>> CWardwizUninstSecondDlg : DeleteFinished", 0, 0, true, ZEROLEVEL);
	//KillTimer(TIMER_DELETE_STATUS);
}

/***************************************************************************************************
*  Function Name  : UninstallationMessage
*  Description    : it shows the third dialog of uninstallation finished
*  Author Name    :	Nitin K
*  SR_NO          :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void UninstallationMessage()
{
	if (!m_bPassDbMngr || !m_bQuarantine)
	{
		AddLogEntry(L">>> Move file delay untill reboot ::UninstallationMessage.",0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : RemoveDriverRegistryKeyAndPauseDrivers
Description    : removes registry keys for drivers
Author Name    : lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
void RemoveDriverRegistryKeyAndPauseDrivers()
{
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int OsType = 0;
	bool returnStatus = false;

	try
	{
		OsType = objGetOSVersion.DetectClientOSVersion();
		if (OsType == 5)
		{
			scannerObj.PauseDriverProtection(0); // self protection
			cSecure64Obj.PauseDriverProtection(0); // self protection

			CString csDelKey = L"SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);

			CString csDelSecure64Key = L"SYSTEM\\CurrentControlSet\\Services\\WrdWizSecure64";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelSecure64Key);

		}
		else if (OsType == 10)
		{
			scannerObj.PauseDriverProtection(0); // self protection

			CString csDelKey = L"SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);
		}
		else
		{

			scannerObj.PauseDriverProtection(0); // self protection
			cSecure64Obj.PauseDriverProtection(0); // self protection							\\scanner

			CString csDelKey = L"SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);

			CString csDelSecure64Key = L"SYSTEM\\CurrentControlSet\\Services\\WrdWizSecure64";
			RegDelnode(HKEY_LOCAL_MACHINE, csDelSecure64Key);
		}

		CString csDelKey = L"SYSTEM\\CurrentControlSet\\services\\WardwizALUSrv";
		RegDelnode(HKEY_LOCAL_MACHINE, csDelKey);

		CString csDelComSrvKey = L"SYSTEM\\CurrentControlSet\\services\\VibraniumComSrv";
		RegDelnode(HKEY_LOCAL_MACHINE, csDelComSrvKey);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RemoveDriverService", 0, 0, true, SECONDLEVEL);
	}
}

//*************************************************************
//  RegDelnode()
//  Purpose:    Deletes a registry key and all its subkeys / values.
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//*************************************************************
BOOL RegDelnode(HKEY hKeyRoot, CString csSubKey)
{
	try
	{
		/*	TCHAR szDelKey[MAX_PATH * 2] = { 0 };
		_tcscpy_s(szDelKey, MAX_PATH * 2, lpSubKey);*/
		return RegDelnodeRecurse(hKeyRoot, csSubKey);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CVibraniumUninstallerDlg::RegDelnode, SubKey[%s]", csSubKey, 0, true, SECONDLEVEL);
	}
	return FALSE;
}

//*************************************************************
//  RegDelnodeRecurse()
//  Purpose:    Deletes a registry key and all its subkeys / values.
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//*************************************************************
BOOL RegDelnodeRecurse(HKEY hKeyRoot, CString csSubKey)
{
	try
	{
		LPTSTR lpEnd = NULL;
		LONG lResult;
		DWORD dwSize;
		TCHAR szName[MAX_PATH];
		HKEY hKey;
		FILETIME ftWrite;

		// First, see if we can delete the key without having
		// to recurse.

		lResult = RegDeleteKey(hKeyRoot, csSubKey);

		if (lResult == ERROR_SUCCESS)
			return TRUE;

		lResult = RegOpenKeyEx(hKeyRoot, csSubKey, 0, KEY_READ, &hKey);

		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_FILE_NOT_FOUND) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}

		// Check for an ending slash and add one if it is missing.
		if (csSubKey.GetAt(csSubKey.GetLength()) != L'\\')
		{
			csSubKey += L'\\';
		}
		// Enumerate the keys

		dwSize = MAX_PATH;
		lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
			NULL, NULL, &ftWrite);

		if (lResult == ERROR_SUCCESS)
		{
			do {
				csSubKey += CString(szName);
				if (!RegDelnodeRecurse(hKeyRoot, csSubKey)) {
					break;
				}

				dwSize = MAX_PATH;

				lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
					NULL, NULL, &ftWrite);

			} while (lResult == ERROR_SUCCESS);
		}

		csSubKey = csSubKey.Left(csSubKey.ReverseFind('\\'));
		RegCloseKey(hKey);

		// Try again to delete the key.

		lResult = RegDeleteKey(hKeyRoot, csSubKey);

		if (lResult == ERROR_SUCCESS)
			return TRUE;

	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in RegDelnodeRecurse, SubKey[%s]", csSubKey, 0, true, SECONDLEVEL);
	}
	return FALSE;
}

/***************************************************************************
Function Name  : OnTimer
Description    : Function which get called when uninstallation get finished.
Author Name    : Ram Shelke
SR_NO		   :
Date           : 06 FEB 2015
****************************************************************************/
void ShowFinishedPage()
{
}

/***************************************************************************************************
*  Function Name  : StartUninstallProgram
*  Description    : Starts the uninstallation process by calling StartUninstallProgram()
*  Author Name    : 
*  Date           : 
****************************************************************************************************/
bool StartUninstallProgram()
{
	try
	{
		StopProtectionDrivers();

		if (!CloseAllAplication(true))
		{
			CloseApp();
		}
		else
		{
			m_iTotalDeletedFileCount = 0;
			m_bMoveFileExFlag = FALSE;
			DeleteFiles(m_vFilePathLists);
			DeleteFolder(m_vFolderPathLists);
			DeleteRegistryKeys();
			DeleteAllAppShortcut();
			DeleteFinished();
			UninstallationMessage();
			UnregisterNFDrivers();
			GetNFAPIDriverPath();
			RemoveDriverRegistryKeyAndPauseDrivers();
			if (m_bUninstallCmd)
			{
				exit(0);
				return false;
			}
			ShowFinishedPage();
		}

	}
	catch (...)
	{
		AddLogEntry(L"### error in creating value ", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CloseApp
*  Description    : for closing application
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CloseApp()
{
	//OnCancel();
}

/***********************************************************************************************
*  Function Name  : EnumFolderToCountFiles
*  Description    : Enumrate all the files in folder to check whether folder is empty or not.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 03 Feb 2015
*************************************************************************************************/
void EnumFolderToCountFiles(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString csFilePath = finder.GetFilePath();
				SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
				CString csFileName = finder.GetFileName();
				if (csFileName == "QUARANTINE")
				{
					if (m_bQuarantine)
					{
						continue;
					}
					else
					{
						m_vFolderPathLists.push_back(csFilePath);
						m_bMoveFileExFlag = FALSE;
						EnumFolderToCountFiles(csFilePath);//, theApp.m_bQuarantine, theApp.m_bPassDbMngr);
					}
				}
					m_vFolderPathLists.push_back(csFilePath);
					EnumFolderToCountFiles(csFilePath);
			}
			else
			{
				CString csFileName = finder.GetFileName();
				CString csFilePath = finder.GetFilePath();
				SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
				if (csFileName == "VBEVALREG.DLL")
				{
					if (m_bPassDbMngr)
					{
						continue;
					}
				}
				if (csFileName == "VBSHELLEXT.DLL")
				{
					if (PathFileExists(csFilePath))
					{
						UnregisterComDll(csFilePath);
					}
				}
				if (m_bMoveFileExFlag)
				{
					continue;
				}
				m_vFilePathLists.push_back(csFilePath);
				m_iTotalFileCount++;
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::EnumFolderToCountFiles", 0, 0, true, SECONDLEVEL);
	}

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
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UnRegisterComDLL", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : _tmain
Description    : starting point function.
Author Name    : Amol J.
S.R. No        :
Date           : 18/03/2018
****************************************************************************/
int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		InitializeDataMember();

		CStringArray csArrCommandLines;
		CString csCommandLine = L"";
		int iTotArg = argc;
		if (!argc)
		{
			return false;
		}
		
		for (int iCmdCount = 0; iCmdCount < argc; iCmdCount++)
			csArrCommandLines.Add(argv[iCmdCount]);

		if (csArrCommandLines.IsEmpty())
			return false;

		for (int iCmdCount = 0; iCmdCount < argc; iCmdCount++)
			csCommandLine += ' ' + csArrCommandLines[iCmdCount];
		
		csCommandLine.Trim();
		csCommandLine.MakeUpper();

		ZeroMemory(m_szTempPath, sizeof(m_szTempPath));
		GetEnvironmentVariable(L"TEMP", m_szTempPath, 511);
		m_csAppPath = GetModuleFilePath();
		m_csAppPath += "\\";
		m_csWardWizPath = GetWardWizPathFromRegistry();
		
		//WardWiz UnInstallation path should not be blank else it will remove all files from the system.
		if (m_csWardWizPath.Trim().GetLength() == 0x00)
		{
			AddLogEntry(L"### Failed to get Vibraniumpath from registry.", 0, 0, true, SECONDLEVEL);
			exit(0);
			return false;
		}

		m_csAppPath.MakeUpper();
		m_csWardWizPath.MakeUpper();
		m_csAppPath.Trim();
		m_csWardWizPath.Trim();

		if (m_csAppPath.GetLength() == 0x00 || m_csWardWizPath.GetLength() == 0x00)
		{
			AddLogEntry(L"### App Path or Wardwiz path empty, AppPath: [%s], WardWizPath: [%s]", m_csAppPath, m_csWardWizPath, true, SECONDLEVEL);
			exit(0);
			return false;
		}

		if (m_csAppPath == m_csWardWizPath )
		{
			TCHAR szTargetPath[MAX_PATH] = { 0 };
			CString csWardwizPath = m_csWardWizPath;
			csWardwizPath += "WARDWIZUNINSTCON.EXE";
			_stprintf_s(szTargetPath, MAX_PATH, L"%s\\WARDWIZUNINSTCON.EXE", m_szTempPath);

			if (csCommandLine.CompareNoCase(TEXT("WARDWIZUNINSTCON.EXE")) >= 0)
			{
				csCommandLine.Replace(TEXT("WARDWIZUNINSTCON.EXE"), L"");
				csCommandLine.Trim();
			}

			if (!CopyFile(csWardwizPath, szTargetPath, FALSE))
			{
				AddLogEntry(L"### Failed to CopyFile, SourcePath: [%s], TargetPath: [%s]", csWardwizPath, szTargetPath, true, SECONDLEVEL);
				return 0;
			}

			ShellExecute(NULL, NULL, szTargetPath, csCommandLine, NULL, SWP_HIDEWINDOW);

			Sleep(200);
			return 0;
		}

		if (csCommandLine.Find(TEXT("-EPSNOUI")) != -1)
		{
			csCommandLine.Replace(TEXT("-EPSNOUI"), L"");
			csCommandLine.Trim();
			
			if (csCommandLine.Find(TEXT("-UNINSTALL")) != -1)
			{
				csCommandLine.Replace(TEXT("-UNINSTALL"), L"");
				csCommandLine.Trim();
			}
			
			if (csArrCommandLines[iTotArg-2].Find(TEXT("-1")) != -1)
				m_bUserSettings = true;
			else
				m_bUserSettings = false;
			
			if (csArrCommandLines[iTotArg-1].Find(TEXT("-1")) != -1)
				m_bQuarantine = true;
			else
				m_bQuarantine = false;
			
			EnumFolderToCountFiles(m_csWardWizPath);
			
			StartUninstallProgram();
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in _tmain", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : UnregisterNFDrivers
Description    : removes NFAPI drivers
Author Name    : Amol Jaware
Date           : 26/07/2018
****************************************************************************/
void UnregisterNFDrivers()
{
	CWardWizOSversion objGetOSVersion;
	const char* csDriverName = "VBFLT";
	int OsType = 0;
	try
	{
		//for all os VBFLT unregister
		csDriverName = "VBFLT";
		if (strlen(csDriverName))
			nf_unRegisterDriver(csDriverName);
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to unregister VibraniumFLT drivers in::UnregisterNFDrivers", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : GetNFAPIDriverPath
Description    : This function used to get path of drivers folder to delete nfapi drivers.
Author Name    : Amol Jaware
Date           : 23 July 2018
***********************************************************************************************/
void GetNFAPIDriverPath()
{
	CString csWrdWizFLTDriverPath;
	TCHAR systemDirPath[MAX_PATH] = _T("");
	try
	{
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));
		csWrdWizFLTDriverPath.Format(L"%s\\drivers\\%s", systemDirPath, L"VBFLT.SYS"); //for all OS
		if (PathFileExists(csWrdWizFLTDriverPath))
			DeleteNFDrivers(csWrdWizFLTDriverPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetNFAPIDriverPath", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : DeleteNFDrivers
Description    : This function will delete all nfapi drivers from drivers folder.
Author Name    : Amol Jaware
Date           : 26 July 2018
***********************************************************************************************/
void DeleteNFDrivers(CString csWrdWizFLTDriverPath)
{
	try
	{
		SetFileAttributes(csWrdWizFLTDriverPath, FILE_ATTRIBUTE_NORMAL);
		if (!DeleteFile(csWrdWizFLTDriverPath))
		{
			AddLogEntry(L"### Failed to delete file CVibraniumUninstallerDlg::DeleteNFDrivers %s", csWrdWizFLTDriverPath, 0, true, FIRSTLEVEL);
			if (PathFileExists(csWrdWizFLTDriverPath))
			{
				MoveFileEx(csWrdWizFLTDriverPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in DeleteNFDrivers", 0, 0, true, SECONDLEVEL);
	}
}