#pragma once

#ifndef		WARDWIZALU_STRUCT_
#define		WARDWIZALU_STRUCT_

#include<vector>

///////////////////////////////////////////////////////////////
//		REGOPTSCANOPTIONS structure is used while Scanning & Repairing Registry Entries
//		This job will executed by Main GUI
//		Created date	: 12-Nov-2013 10:15PM
//		Created By		: Vilas
//
//		Modified		:1. 16-Nov-2013 10:15PM, Added for repair entry for each 
///////////////////////////////////////////////////////////////

typedef struct _ALUFileInfo
{
	TCHAR	szFileName[64];
	TCHAR	szFileHash[64];
	TCHAR	szFileShortPath[128];
	TCHAR	szServerLoc[32];
	DWORD	dwZipSize;
	DWORD	dwFullSize;
} ALUFILEINFO, *PALUFILEINFO ;

typedef struct _ALUZipFileInfo
{
	TCHAR	szFileName[64];
	TCHAR	szFileHash[64];
	TCHAR	szZipFileHash[64];
	TCHAR	szFileShortPath[128];
	TCHAR	szServerLoc[32];
	DWORD	dwZipSize;
	DWORD	dwFullSize;
} ALUZIPFILEINFO, *PALUZIPFILEINFO;

extern std::vector<ALUFILEINFO > vALUFileInfo;
extern std::vector<ALUZIPFILEINFO > vALUZipFileInfo;

#endif