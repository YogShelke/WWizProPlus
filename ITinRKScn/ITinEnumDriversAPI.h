#ifndef _ENUMDRIVERS_H_
#define _ENUMDRIVERS_H_

#pragma once
#include <Windows.h>
#include <Shlwapi.h>
#include <Winsvc.h>

#pragma comment(lib, "Shlwapi.lib" )

class CITinEnumDriversAPI
{
public:
	CITinEnumDriversAPI();
	~CITinEnumDriversAPI();

	TCHAR	szAllUserDir[256] ;
	TCHAR	szComnProgFilesDir[256] ;
	TCHAR	szProgFilesDir[256] ;
	TCHAR	szWinDir[256] ;
	TCHAR	szTempDir[256] ;
	TCHAR	szPublicDir[256] ;
	TCHAR	szUserProfileDir[256] ;
	TCHAR	szSystem32Dir[256] ;
	TCHAR	szProgramDirX86[256] ;

	void GetInitialValues() ;

	bool GetServicePath( TCHAR *pShortPath, TCHAR *pNormalPath ) ;
	DWORD EnumerateDriverServicesThrouthAPI( ) ;

public :
	HANDLE	m_hDLL ;

};

#endif