#pragma once

#ifndef _SCANNERCONSTANTS_
#define _SCANNERCONSTANTS_
#endif

#include "AVRegInfo.h"

//DB FILE NAMES

#define WRDWIZAVDBNAME			L"VIBRANIUMAV1.DB"
#define WRDWIZREPAIRDBNAME		L"VIBRANIUMAVR.DB"

//EXPORTED FUNCTION PROTOTYPE HERE
typedef bool (*LOADSIGNATURE) (LPCTSTR, DWORD &dwSigCount);
typedef bool (*UNLOADSIGNATURES) (void);
typedef bool (*SCANFILE) (LPCTSTR, LPTSTR, DWORD&, bool);
typedef bool (*RELOADSETTINGS)(DWORD,bool);
typedef bool (*REPAIRFILE) (LPCTSTR, DWORD);
typedef DWORD (*GETNOOFDAYS) (void);
typedef bool(*DOREGISTRATION) (void);
typedef bool(*GETREGISTRATIONINFO) (AVACTIVATIONINFO&);
typedef DWORD(*EXPIRYMSGBOX)(bool);
typedef DWORD (*GETREGISTEREFOLDERDDATA) (LPBYTE lpResBuffer, DWORD &dwResDataSize, DWORD dwResType, TCHAR *pResName ) ;
typedef bool(*SAVAPIINITIALIZE) (void);
typedef bool(*SAVAPIUNINITIALIZE) (void);

typedef bool(*WRDWIZFWINITIALIZE) (void);
typedef bool(*WRDWIZFWUNINITIALIZE) (void);

typedef bool(*WRDWIZEMAILINITIALIZE) (void);
typedef bool(*WRDWIZEMAILUNINITIALIZE) (void);

typedef bool(*WRDWIZBROWSERSECINITILIAZE) (void);
typedef bool(*WRDWIZBROWSERSECUNINITILIAZE) (void);
typedef bool(*WRDWIZBROWSERSECRELOAD) (void);
typedef bool(*WRDWIZBROWSERSECEXCRELOAD) (void);
typedef bool(*WRDWIZBROWSERSECSPECRELOAD) (void);

typedef bool(*WRDWIZWEBPROTINITIALIZE) (void);
typedef bool(*WRDWIZWEBPROTUNINITIALIZE) (void);
typedef bool(*WRDWIZWEBPROTRELOADWEBSECDB) (void);
typedef bool(*WRDWIZWEBPROTRELOADBROWSEPROTDB) (void);
typedef bool(*WRDWIZWEBPROTRELOADBLKSPECWEB) (void);
typedef bool(*WRDWIZWEBPROTRELOADMNGEXCDB) (void);
typedef bool(*WRDWIZWEBPROTCLEARCACHE) (void);

typedef bool(*WRDWIZRELOADAPPRULES) (void);
typedef bool(*WRDWIZEMAILNFDLLUNLINK) (void);
typedef bool(*RELOADEMAILSCANSETTINGS) (DWORD dwRegValue);
typedef bool(*RELOADPARENTALCONTROLSETTINGS) (DWORD dwRegValue);
typedef bool(*STARTPARCONTROLCHECK) (void);
typedef bool(*STARTCOMPUSAGECHECK) (void);
typedef bool(*STARTINTERNETCHECK) (void);
typedef bool(*STARTINETUSAGECHECK) (void);
typedef bool(*ENABLEDISABLEINTERNETUSAGE) (bool);
typedef bool(*CHECKEXEISBLOCKED) (LPTSTR lpszFilePath);
typedef bool(*RELOADAPPLICATIONLIST) (void);
typedef bool(*CLEARALLRULES) (void);
typedef bool(*SETDEFAULTAPPBEHAVIOUR) (DWORD);
typedef bool(*RELOADUSERLIST)(void);
typedef bool(*RELOADUSERPERMISSION)(void);
typedef bool(*RELOADUSERRESETVALUE)(void);
typedef bool(*CHECKINTERNETVALUE)(void);
/******************************************************************************/
/*	 				CONSTANTS FOR ENUMPROCESSMODULE FROM PSAPI.DLL 			  */
/******************************************************************************/

typedef enum _RULE_ACTION
{
	ACTION_ALLOW = 1,
	ACTION_DENY = 2,
} RULE_ACTION, *PRULE_ACTION;

typedef enum _TRAFFIC_DIRECTION
{
	DIR_INBOUND = 0x1,
	DIR_OUTBOUND = 0x2,
	DIR_BOTH = 0x3
} TRAFFIC_DIRECTION, *PTRAFFIC_DIRECTION;

typedef DWORD(*CREATEFWRULES) (LPSTR , RULE_ACTION , TRAFFIC_DIRECTION , bool , BYTE ,
	BYTE , BYTE , LPTSTR , BYTE , LPTSTR , BYTE ,
	LPTSTR , BYTE , LPTSTR );
typedef void(*ENABLEDISFWPORTSCNPROT)(DWORD dwRegValue, LPTSTR szGUID);
typedef void(*ENABLEDISFWSTEALTHMODE)(DWORD dwRegValue, LPTSTR szGUID);
typedef void(*ENABLEDISFIREWALLSETTINGSTAB)(DWORD dwRegValue);

/******************************************************************************/


/******************************************************************************/
/*	 				CONSTANTS FOR ENUMPROCESSMODULE FROM PSAPI.DLL 			  */
/******************************************************************************/
typedef BOOL (WINAPI *ENUMPROCESSMODULESEX )	( HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded, DWORD dwFilterFlag ) ;
/******************************************************************************/

/******************************************************************************/
/*	 				ENUM FOR AUTOQUARENTINE OPTIONS				 			  */
/******************************************************************************/
typedef enum {
	FILESKIPPED				= 0x0101,
	FILEREPAIRED			= 0x0102,
	FILEQURENTINED			= 0x0103,
	FILEREBOOTQUARENTINE	= 0x0104,
	FILEREBOOTREPAIR		= 0x0105,
}QUARANTINESTATUS;

typedef enum {
	SKIP		= 0x00,
	REPAIRE		= 0x01,
	QURENTINE	= 0x02
}QUARANTINEOPTION;

/******************************************************************************/
/*	 				ENUM FOR CLEANFAILURE ID'S		 						 */
/******************************************************************************/
typedef enum {
	SANITYCHECKFAILED			= 0x01,
	FILENOTEXISTS				= 0x02,
	CREATEDIRECTORYFAILED		= 0x03,
	REPAIRFILEFAILED			= 0x04,
	BACKUPFILEFAILED		    = 0x07,
	LOWDISKSPACE				= 0x09,
	DELETEFILEFAILED			= 0x05,
	INSERTINTORECOVERFAILED		= 0x06,
	SAVERECOVERDBFAILED			= 0x07
}SCANERRORID;

