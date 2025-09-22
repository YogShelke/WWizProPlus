#ifndef		ITINDRIVERINFO_STRUCT_
#define		ITINDRIVERINFO_STRUCT_

#ifndef MYEXTERN
	#define MYEXTERN extern
#endif

#pragma once
//#include <Windows.h>
#include <vector>

///////////////////////////////////////////////////////////////
//		ITINDRIVERINFO structure contains Driver Path and It's display name.
//		This Info is used for RootKit detection
//		Created date	: 23-Mar-2014 09:00AM
//		Created By		: Vilas
///////////////////////////////////////////////////////////////
typedef struct _ITINDRIVERINFO
{
	TCHAR	szDriverPath[256] ;
	TCHAR	szDriverDisplayName[128] ;

} ITINDRIVERINFO, *PITINDRIVERINFO ;

typedef std::vector<ITINDRIVERINFO> vecDRIVERINFO ;
MYEXTERN	vecDRIVERINFO	vDriverInfo ;

MYEXTERN LPBYTE		pRegRawData ;
MYEXTERN DWORD		dwRegRawSize ;

MYEXTERN BOOL	m_bIsWow64 ;
MYEXTERN bool	bVistaOnward ;

typedef struct _ITINDRIVERINFO_HIVE
{
	TCHAR	szDriverName[0x40] ;
	TCHAR	szDriverPath[0x100] ;
	DWORD	dwPathOffset ;
	DWORD	dwPathLength ;

} ITINDRIVERINFO_HIVE, *PITINDRIVERINFO_HIVE ;

typedef std::vector<ITINDRIVERINFO_HIVE> vecDRIVERINFO_HIVE ;
MYEXTERN	vecDRIVERINFO_HIVE	vDriverInfo_Hive ;

MYEXTERN	ITINDRIVERINFO_HIVE	sDriverInfo_Hive ;


#endif