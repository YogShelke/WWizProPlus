// UsbDetect.cpp : Defines the entry point for the DLL application.
//************************************************************************************************
//*  Program Name:  USBDetect.cpp                                                                                                 
//*  Description:  In this file, When USB inserts,It gets detect with OnMyDevice()function and
//*				   Load the Main UI.When USB removes,It shows simple message box
//*				   
//*  Author Name:Neha Gharge                                                                                                       
//*  Date Of Creation:  18th October,2013                                                                                               
//*  Version No:                                                                                                            
//*                                                                                                                         
//*  Special Logic Used:                                                                                             
//*                                                                                                                                      
//*                                                                                                                                      
//*  Modification Log:                                                                                                
//*  1. Modified xyz function in main        Date modified         CSR NO   
//*
//*																				
//*                                                                               
//************************************************************************************************
#define WIN32_LEAN_AND_MEAN
//************************************************************************************************
// HEADER FILES
//************************************************************************************************
#include "stdafx.h"
#include "UsbDetect.h"
#include "resource.h"
#include <vector>

//************************************************************************************************
// MAPPING OF THE WINDOW MESSAGES.
//************************************************************************************************

#define MESSAGE_MAP(MessageParam, MessageCallback)\
		if(message == MessageParam)\
			return MessageCallback(hwnd, message, wParam, lParam)
//************************************************************************************************
// FUNCTIONS
//************************************************************************************************
BOOL OnDialogInit(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT OnUSBDeviceChange(HWND hwnd, UINT message,WPARAM wParam, LPARAM lParam);
void UpdateDevice(HWND hwnd,PDEV_BROADCAST_VOLUME pDevVolume,WPARAM wParam);
bool LaunchMainUI4USBScanning(TCHAR *szParam);
bool ReadUSBScansettingFromReg();
bool UpdateDrive(TCHAR szDrive, bool bIsAdd = true);
bool CheckRemovableDevices();
//void EnumTotalFolder(LPCTSTR pstr);

extern "C" __declspec(dllexport)INT_PTR DetectUSB(void);
extern "C" __declspec(dllexport)bool ResetUSBVariable(void);
extern "C" __declspec(dllexport)bool ISRemovableDevice(LPTSTR lpDriveName);
PHANDLE GetCurrentUserToken();
BOOL Run(TCHAR *pEXEPath, TCHAR *pCmdLine);

//************************************************************************************************
// GLOBAL VARIABLES
//************************************************************************************************
HINSTANCE hInstance = 0;
TCHAR g_szPrevDrvName[4] = {0};
bool g_bEnableUSBScan = false;
std::vector<TCHAR>				m_vcsListOfDrive;

//int g_iTotalFileCount = 0; 

#ifdef _X86_
extern "C" { int _afxForceUSRDLL; }
#else
extern "C" { int __afxForceUSRDLL; }
#endif

/***************************************************************************
  Function Name  : DllMain	
  Description    : dll main function
  Author Name    : Ram
  S.R. No        :WRDWIZUSBDLL_001
  Date           : 25th June 2014
****************************************************************************/
BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	wprintf(L"DLLMAIN\r\n");
	
	if(ul_reason_for_call==DLL_PROCESS_ATTACH)
	{	
			hInstance = (HINSTANCE) hModule;
			wprintf(L"DLL_Process_attach\r\n");

	}	
	return TRUE ;
}

//************************************************************************************************
// LOAD LIBRARY LOADS INT_PTR DetectUSB() FUNCTION THROUGH FUNCTION POINTER.
// This is exported function to create dialog box and can be used by external application.
//************************************************************************************************
__declspec(dllexport)INT_PTR DetectUSB(void)
{
	wprintf(L"In USB function\r\n");
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOGBOX), NULL, DialogProc);

}

/***************************************************************************
Function Name  : ISRemovableDevice
Description    : Function to check is removable device.
Author Name    : Ram
Date           : 11th Dec 2015
****************************************************************************/
extern "C" __declspec(dllexport)bool ISRemovableDevice(LPTSTR lpDriveName)
{
	bool bReturn = false;
	__try
	{
		if (lpDriveName == NULL)
		{
			return bReturn;
		}

		for (int iIndex = 0; iIndex < static_cast<int>(m_vcsListOfDrive.size()); iIndex++)
		{
			TCHAR szDrive[5] = { 0 };
			swprintf(szDrive, L"%c:", m_vcsListOfDrive.at(iIndex));
			if (PathFileExists(szDrive))
			{
				_wcslwr(lpDriveName);
				_wcslwr(szDrive);
				if (lpDriveName[0] == szDrive[0])
				{
					bReturn = true;
					break;
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in ISRemovableDevice", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : ResetUSBVariable	
  Description    : it is use to reset usb variable.
  Author Name    : Ram
  S.R. No        :WRDWIZUSBDLL_002
  Date           : 25th June 2014
****************************************************************************/
_declspec(dllexport)bool ResetUSBVariable(void)
{
	if(g_szPrevDrvName && *g_szPrevDrvName)
	{
		_tcscpy(g_szPrevDrvName,L"");
	}
	g_bEnableUSBScan = false;
	return true;
}

/***************************************************************************
  Function Name  : DialogProc	
  Description    : it use to handle window message.
  Author Name    : Ram
  S.R. No        :WRDWIZUSBDLL_003
  Date           : 25th June 2014
****************************************************************************/
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ShowWindow(hwnd, SW_HIDE);
    MESSAGE_MAP(WM_INITDIALOG, OnDialogInit);
    MESSAGE_MAP(WM_DEVICECHANGE, OnUSBDeviceChange);
    return FALSE;
}

/***************************************************************************
  Function Name  : OnDialogInit	
  Description    : INITIALIZED DIALOG AND REGISTER NOTIFICATION OF DEVICES INTO ARRAY GUID_DEVINTERFACE_LIST
  Author Name    : Ram
  S.R. No        : WRDWIZUSBDLL_004
  Date           : 25th June 2014
****************************************************************************/
BOOL OnDialogInit(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	wprintf(L"In INIT function\r\n");
	//Handle to register device
	HDEVNOTIFY hDevNotify;
	//structure which contain the data of device
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
	//fills block of memory as zero
    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
	//Copy the size of structure into object structure size
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	//Copy the device type: structure of class of device
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	
	//Check here removable devices and add in vector
	CheckRemovableDevices();

	//take 16bytes
	for(int i=0; i<sizeof(GUID_DEVINTERFACE_LIST)/sizeof(GUID);i++)
	{
		wprintf(L"Register notification %d\r\n",i);
		//Guid for interface device class.Copy each 16byte GUID in the local stucture
		NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
		//register all device one by one.
		hDevNotify = RegisterDeviceNotification(hwnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
		if( !hDevNotify )
		{
			wprintf(L"can't register device notification\r\n");
			//if it cannot register device notification
			//Get the message box with Exclaimation mark and msg.
			//MessageBox(NULL,L"Can't register device notification:",L"Error",MB_ICONEXCLAMATION);
			return FALSE;
		}
				
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


/***************************************************************************
  Function Name  : OnUSBDeviceChange	
  Description    :  it NOTIFICATION OF ARRIVAL OF DEVICES and DBT_DEVICEREMOVECOMPLETE: NOTIFICATION OF REMOVAL OF DEVICE. It will check the devicetype and work accordingly
  Author Name    : Ram
  S.R. No        :WRDWIZUSBDLL_005
  Date           : 25th June 2014
****************************************************************************/
LRESULT OnUSBDeviceChange(HWND hwnd, UINT message,WPARAM wParam, LPARAM lParam)
{
	//switch select the type of device. according to that function is written.
	if( DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam )
	{
		PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
		//handle for different device types. For USB interfacing pDevInf.
		PDEV_BROADCAST_VOLUME pDevVolume;
		switch( pHdr->dbch_devicetype ) 
		{
			case DBT_DEVTYP_DEVICEINTERFACE:
 
			    //Update into tree view and take a information and show it on task bar
			     //pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
			     break;

			case DBT_DEVTYP_HANDLE:
				//pDevHnd = (PDEV_BROADCAST_HANDLE)pHdr;
				break;

			case DBT_DEVTYP_OEM:
				//pDevOem = (PDEV_BROADCAST_OEM)pHdr;
				break;

			case DBT_DEVTYP_PORT:
				//pDevPort = (PDEV_BROADCAST_PORT)pHdr;
				break;

			case DBT_DEVTYP_VOLUME:
				pDevVolume = (PDEV_BROADCAST_VOLUME)pHdr;
				UpdateDevice(hwnd,pDevVolume,wParam);
				break;
		}
	}
	
	return TRUE;
}

/***************************************************************************
  Function Name  : IsBitSet	
  Description    : MASKING THE UNSIGNED 32 BITS
  Author Name    : Ram
  S.R. No        :WRDWIZUSBDLL_006
  Date           : 25th June 2014
****************************************************************************/
BOOL IsBitSet (DWORD dwMask, UINT nTHBit)
{
	DWORD dwBit = 1;
	dwBit <<= nTHBit;
	dwMask &= dwBit;
	return dwMask ? TRUE:FALSE;
}


/***************************************************************************
  Function Name  : UpdateDevice	
  Description    : IN THIS FUNCTION, WHEN USB INSERTED, IT RETURNS THE DRIVE NAME AND LOAD THE MAIN UI APPLICATION WITH PARAMETERS.
  Author Name    : Ram
  S.R. No        :WRDWIZUSBDLL_007
  Date           : 25th June 2014
****************************************************************************/
void UpdateDevice(HWND hwnd,PDEV_BROADCAST_VOLUME pDevVolume,WPARAM wParam)
{
	DWORD dCreateProcess = 0;
	wprintf(L"Update devicee\r\n");
	TCHAR szParam[MAX_PATH] = {0};
	char DrvName;

	if(DBT_DEVICEARRIVAL == wParam)
	{		 
		//masking drive name
		for (int n = 0; n < 32; n++)
		{
			if(pDevVolume->dbcv_devicetype ==DBT_DEVTYP_VOLUME)
			{
				if (IsBitSet (pDevVolume->dbcv_unitmask, n))
				{
					wprintf(L"Drive %c:/ Inserted\n", n + 'A');
					DrvName = (n +'A');
					wprintf(L"Drive %c:/ Inserted\n",DrvName);

				}
			}
		}
		_tcscat_s(szParam,MAX_PATH,L"-USBSCAN");
		_tcscat_s(szParam,MAX_PATH,L" ");

		TCHAR szDrive = DrvName;

		TCHAR szTemp[4] = {0};
		TCHAR szUSBParam[4] = {0};
		szTemp[0] = DrvName;
		szTemp[1] = ':';
		szTemp[3] = '\0';

		//add here the drive name
		UpdateDrive(szDrive, true);

		//CString csEnumPath = L"";
		//csEnumPath = szTemp;
		//EnumTotalFolder(csEnumPath);
		//_tcscat_s(szParam,MAX_PATH,szTemp);

		
		if(!ReadUSBScansettingFromReg())
		{
			AddLogEntry(L"### Error in ReadUSBScansettingFromReg()", 0, 0, true, SECONDLEVEL);
		}

		// *****************Issue not reported Neha Gharge 23/5/2014************************************************//
		if(g_bEnableUSBScan == true)
		{
			// resolve by lalit kumawat 4-2-2015
			//Issue with WardWiz USB Scan:-When USB is removed the WardWiz USB Detected Window
			//should get close as soon as USB is removed.
			// sometime the usbDetectMsg not displayed

			/*if(_tcscmp(szTemp , g_szPrevDrvName))
			{*/
				int iDriveType = GetDriveType(szTemp);
				if(iDriveType == DRIVE_REMOVABLE || iDriveType ==  DRIVE_FIXED)
				{
					//Issue 1174 : If any path doen't have access mode. It will not launch Pop-up 
					if ((_waccess(szTemp, 0)) == 0)
					{
						//Issue not reported : USB scanning not working 
						//Tokenizer changed from ";" to "|"
						//Added By Nitin K Date 6th July 2015
						_tcscpy(szUSBParam, L"");
						szUSBParam[0] = DrvName;
						szUSBParam[1] = ':';
						szUSBParam[2] = '|';
						szUSBParam[3] = '\0';
						_tcscat_s(szParam, MAX_PATH, szUSBParam);
						LaunchMainUI4USBScanning(szParam);
					}
				}
			/*}
			_tcscpy(g_szPrevDrvName , szTemp);*/
		}
	}
	else if(DBT_DEVICEREMOVECOMPLETE == wParam)
	{	
		for (int n = 0; n < 32; n++)
		{
			if(pDevVolume->dbcv_devicetype == DBT_DEVTYP_VOLUME)
			{
				if (IsBitSet (pDevVolume->dbcv_unitmask, n))
				{
					wprintf(L"Drive %c:/ Removed\n", n + 'A');
					DrvName = (n +'A');
					wprintf(L"Drive %c:/ Removed\n",DrvName);

				}
			}
		}

		//Remove here the drive name.
		TCHAR szDrive = DrvName;
		UpdateDrive(szDrive, false);

		//Issue Resolved: Insert usb drive->click on scan->remove usb->message should be usb scan aborted.
		//Resolved By : Nitin K. Date 11th March 2015
		__try
		{
			HWND hWindow = FindWindow(NULL, L"VBUSB");
			SendMessage(hWindow, DRIVEREMOVALSCANSTATUS, DrvName, 0);
			//resolved by lalit kumawat 4-2-2015
			//Issue with WardWiz USB Scan : -When USB is removed the WardWiz USB Detected Window
			//should get close as soon as USB is removed.
			while ( hWindow != NULL )
			{

				hWindow = FindWindowEx(NULL, hWindow, NULL, L"VBUSB");

				SendMessage(hWindow, DRIVEREMOVALSCANSTATUS, DrvName, 0);
			}

		}
		__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
		{
			AddLogEntry(L"### Exception in CSystemTray::SendEnableEmailScanMessage2UI", 0, 0, true, SECONDLEVEL);

		}
	}
}
/***************************************************************************
  Function Name  : LaunchMainUI4USBScanning	
  Description    : it use to launch main UI for USB scanning
  Author Name    : Ram
  S.R. No        :WRDWIZUSBDLL_08
  Date           : 25th June 2014
****************************************************************************/
bool LaunchMainUI4USBScanning(TCHAR *szParam)
{
	bool bReturn = false;
	try
	{
		TCHAR szModulePath[MAX_PATH] = {0};
		TCHAR szFullPath[MAX_PATH] = {0};

		GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		_tcscpy(szFullPath, szModulePath);
		_tcscat(szFullPath, L"\\VBUSBDETECTUI.EXE");

		::ShellExecute(NULL, L"Open", szFullPath, szParam, NULL, 0);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in LaunchMainUI4USBScanning", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : ReadUSBScansettingFromReg	
  Description    : it use to read USB scanning setting from registry.
  Author Name    : Ram
  S.R. No        :WRDWIZUSBDLL_09
  Date           : 25th June 2014
****************************************************************************/
bool ReadUSBScansettingFromReg()
{
	try
	{
		HKEY hKey;
		LONG ReadReg;
		DWORD dwvalueSType;
		DWORD dwvalueSize = sizeof(DWORD);
		DWORD ChkvalueForUSBScan;
		DWORD dwType=REG_DWORD;

		TCHAR szPath[MAX_PATH] = { 0 };
		if (!GetProductRegistryKey(szPath, _countof(szPath)))
		{
			AddLogEntry(L"### GetProductRegistryKey Failed in ReadUSBScansettingFromReg", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
		{
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Vibranium"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
			{
				return false;
			}
		}

		ReadReg=RegQueryValueEx(hKey,L"dwUsbScan",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
		ChkvalueForUSBScan=(DWORD)dwvalueSType;
		if(ChkvalueForUSBScan==0)
		{
			g_bEnableUSBScan = false;
		}
		else
		{
			g_bEnableUSBScan = true;
		}
		RegCloseKey(hKey);
		return true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in ReadUSBScansettingFromReg", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : UpdateDrive
Description    : Function to add (or) remove removable device.
Author Name    : Ram
Date           : 11th Dec 2015
****************************************************************************/
bool UpdateDrive(TCHAR szDrive, bool bIsAdd)
{
	if (bIsAdd)
	{
		bool bFound = false;
		for (std::vector<TCHAR>::iterator it = m_vcsListOfDrive.begin(); it != m_vcsListOfDrive.end(); ++it)
		{
			if (szDrive == (*it))
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			m_vcsListOfDrive.push_back(szDrive);
		}
	}
	else
	{
		for (std::vector<TCHAR>::iterator it = m_vcsListOfDrive.begin(); it != m_vcsListOfDrive.end(); ++it)
		{
			if (szDrive == (*it))
			{
				m_vcsListOfDrive.erase(it);
				break;
			}
		}
	}
	return true;
}

/***************************************************************************
Function Name  : CheckRemovableDevices
Description    : Function to check in initialize, if removable devices are attached.
Author Name    : Ram
Date           : 11th Dec 2015
****************************************************************************/
bool CheckRemovableDevices()
{
	TCHAR cDrive;
	UINT  uDriveType;
	TCHAR szDriveRoot[] = _T("x:\\");
	DWORD dwDrivesOnSystem = GetLogicalDrives();

	try
	{
		if (m_vcsListOfDrive.size() > 0)
		{
			m_vcsListOfDrive.clear();
		}

		for (cDrive = 'A'; cDrive <= 'Z'; cDrive++, dwDrivesOnSystem >>= 1)
		{
			if (!(dwDrivesOnSystem & 1))
				continue;

			// Get the type for the next drive, and check dwFlags to determine
			// if we should show it in the list.

			szDriveRoot[0] = cDrive;

			uDriveType = GetDriveType(szDriveRoot);

			if (uDriveType == DRIVE_REMOVABLE)
			{
				m_vcsListOfDrive.push_back(cDrive);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in CheckRemovableDevices", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

//************************************************************************************************
//END OF CODE...
//************************************************************************************************

