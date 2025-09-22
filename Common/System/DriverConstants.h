#include <shlwapi.h>  
#include <Windows.h>
#include <stdio.h>
#include <Winsvc.h>
#include <stdlib.h> 
#include "winioctl.h"
#include "subauth.h"

#ifndef MAX_FILE_PATH
#define MAX_FILE_PATH	256
#endif


//=============================================================================
//							DRIVER SERVICE NAMES
//=============================================================================
const CString csDXpProcServiceName = L"WrdWizXpProc";
const CString csDSecure64ServiceName = L"WrdWizSecure64";
const CString csDScannerServiceName = L"wrdwizscanner";

//=============================================================================

//******************************************************************************
//							PROTECTION DRIVER CONSTANTS
//******************************************************************************
const TCHAR WRDWIZXPPROCSYS[20] = L"VBXPPROC.SYS";
const TCHAR WRDWIZSCANNERSYS[20] = L"VBFILEPROT.SYS";
const TCHAR WRDWIZSECURE64SYS[20] = L"VBREGPROT.SYS";
//******************************************************************************

//=============================================================================
//							SECURE 64 CONTSTANTS
//=============================================================================
//#define SERVICE_DISPLAY_NAME	L"WrdWizSecure64"
#define DRIVER_INSTALLATION_FAILED		0
#define DRIVER_SUCCESSFULLY_INSTALLED	1
#define DRIVER_ALREADY_INSTALLED		2
#define DRIVER_START_FAILED				0
#define DRIVER_SUCCESSFULLY_STARTED		1
#define DRIVER_UNINSTALL_FAILED			0
#define DRIVER_UNINSTALL_SUCCESS		1
#define DRIVER_REMOVAL_FAILED			0
#define DRIVER_REMOVAL_SUCCESS			1
//=============================================================================


#define SVC_ERROR                        ((DWORD)0xC0020001L)
#define SECURE_STRING "01010101010101010101010101010101"

//=============================================================================
//							PROCESS ID CONTSTANTS
//=============================================================================
#define 	WLSRV_ID_ZERO			0   //main GUI
#define 	WLSRV_ID_ONE			1	//tray
#define 	WLSRV_ID_TWO			2	//crypt utility
#define 	WLSRV_ID_THREE			3	//usb GUI
#define 	WLSRV_ID_FOUR			4	//AutoRun
#define 	WLSRV_ID_FIVE			5	//TempClean
#define 	WLSRV_ID_SIX			6   //usbVaccine
#define 	WLSRV_ID_SEVEN			7	//Alu service
#define 	WLSRV_ID_EIGHT			8	//com service
#define 	WLSRV_ID_NINE			9	// uninstaller
#define 	WLSRV_ID_TEN			10  // setup dll 
#define 	WLSRV_ID_ELEVEN			11	
#define 	WLSRV_ID_TWELVE			12  // WrdWizTroubleShooter
#define 	WLSRV_ID_THIRTEEN		13  //UnRegisterUtility
#define 	WLSRV_ID_FOURTEEN		14  //WardWiz Boot Scanner
#define 	WLSRV_ID_FIFTEEN		15	//WardWiz Preinstallation Scan
#define 	WLSRV_ID_SIXTEEN		16	//WrdWizRescueDisk
#define 	WLSRV_ID_SEVENTEEN		17  //WardWiz Installer
#define 	WLSRV_ID_EIGHTEEN		18
#define 	WLSRV_ID_NINETEEN		19
#define 	WLSRV_ID_TWENTY			20
#define 	WLSRV_ID_TWENTYONE		21
#define 	WLSRV_ID_TWENTYTWO		22
#define 	WLSRV_ID_TWENTYTHREE	23
#define 	WLSRV_ID_TWENTYFOUR		24
#define 	WLSRV_ID_TWENTYFIVE		25
#define 	WLSRV_ID_TWENTYSIX		26
#define 	WLSRV_ID_TWENTYSEVEN	27
#define 	WLSRV_MAX_PROCESS_ID	28

//=============================================================================
//							PROCESS ID CONTSTANTS
//=============================================================================
#define IOCTL_REGISTER_PROCESSID     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_SET_PROTECTION_STATE   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_SET_PROTECTED_FOLDER   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_REGISTER_PROCESSID_EX  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_SET_BLOCKED_FOLDER     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_SET_AUTORUN_BLOCK      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_REG_VOLUME_TRACKING	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_UNREG_VOLUME_TRACKING  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_CHECK_FOBJ_VALIDITY    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
//=============================================================================

//=============================================================================
//							PROTECTION DRIVER STRUCT
//=============================================================================
typedef struct _REGISTER_PROCESS_ID
{
	ULONG ProcessIdCode;
	UCHAR SecureString[40];
}REGISTER_PROCESS_ID, *PREGISTER_PROCESS_ID;

typedef struct _SET_PROTECTION_STATE
{
	ULONG ProtectionState;
	UCHAR SecureString[40];
}SET_PROTECTION_STATE, *PSET_PROTECTION_STATE;

typedef struct _SET_PROTECTED_FOLDER_NAME
{
	PWSTR pFileInfo; //Example \\WINDOWS\\SYSTEM32\\DRIVERS  OR  \\WardWiz Antivirus  
	UCHAR Padding[4];
	UCHAR SecureString[40];
}SET_PROTECTED_FOLDER_NAME, *PSET_PROTECTED_FOLDER_NAME;

typedef struct _REGISTER_PROCESS_ID_EX
{
	ULONG ProcessId;
	ULONG ProcessIdCode;
	UCHAR SecureString[40];
}REGISTER_PROCESS_ID_EX, *PREGISTER_PROCESS_ID_EX;


typedef struct _SET_PROTECTED_FOLDER_NAME32B
{
	PWSTR pFileInfo; //Example \\WINDOWS\\SYSTEM32\\DRIVERS  OR  \\WardWiz Antivirus  
	UCHAR Padding[8];
	UCHAR SecureString[40];
}SET_PROTECTED_FOLDER_NAME32B, *PSET_PROTECTED_FOLDER_NAME32;

//struct for autorun scanner.
typedef struct _AUTORUN_BLOCK_DATA
{
	BOOLEAN			IsAutoRunBlocked;
	BOOLEAN			IsUSBExecBlocked;
	BOOLEAN			IsUSBWriteBlocked;
	UCHAR 			Padding[4];
	UCHAR 			SecureString[40];
}AUTORUN_BLOCK_DATA, *PAUTORUN_BLOCK_DATA;

//=============================================================================


//=============================================================================
//						PROTECTION DRIVER PIPE CONSTANTS
//=============================================================================
const CString csPipeXPProtection	= L"\\\\.\\xpProc";
const CString csPipeSecure64Prot	= L"\\\\.\\Secure64";
const CString csPipeScannerProt		= L"\\\\.\\WlsrvScanner";
//=============================================================================
