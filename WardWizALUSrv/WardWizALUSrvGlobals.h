
#ifndef _WARDWIZALUSRVGLOBALS_H_
#define _WARDWIZALUSRVGLOBALS_H_
#endif


///////////////////////////////////////////////////////////////
//		REGOPTSCANOPTIONS structure is used while Scanning & Repairing Registry Entries
//		This job will executed by Main GUI
//		Created date	: 12-Nov-2013 10:15PM
//		Created By		: Vilas
//
//		Modified		:1. 16-Nov-2013 10:15PM, Added for repair entry for each 
///////////////////////////////////////////////////////////////

//Global Variables
BOOL	g_bIsWow64;
bool	g_bVistaOnward;
TCHAR	g_szAVPath[512];
HMODULE	g_hHashDLL;

typedef DWORD (*GETFILEHASH)	(TCHAR *pFilePath, TCHAR *pFileHash);

GETFILEHASH		GetFileHash = NULL ;
