

#ifndef		ISPYAVACTIVATIONINFO_STRUCT_
#define		ISPYAVACTIVATIONINFO_STRUCT_



///////////////////////////////////////////////////////////////
//		AVACTIVATIONINFO structure contains user information while registering iSpy.
//
//		Created date	: 01-Dec-2013 04:00PM
//		Created By		: Vilas
///////////////////////////////////////////////////////////////
typedef struct _AVACTIVATIONINFO
{
	TCHAR	szTitle[8] ;
	TCHAR	szUserFirstName[64] ;
	TCHAR	szUserLastName[64] ;
	TCHAR	szMobileNo[16] ;
	TCHAR	szEmailID[64] ;
	TCHAR	szRegionCode[64] ;
	
	TCHAR	szKey[0x16] ;
	TCHAR	szClientID[0x80];
	TCHAR	szInstID[0x25];

	//DWORD	dwFuture ;
	DWORD	dwDaysUsed ;
	DWORD	dwProductNo ;

	SYSTEMTIME	RegTime ;
	//DWORD		dwDeltaDays ;
	DWORD		dwTotalDays ;
	SYSTEMTIME	RegTimeServer ;

} AVACTIVATIONINFO, *PAVACTIVATIONINFO ;

#endif