/***************************************************************************                      
   Program Name          : WardWizValCheck
   Description           : 
   Author Name           : Ram                                                                              
   Date Of Creation      : 
   Version No            : 
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/

#include "stdafx.h"
#include "ValCheck.h"
#include "AVRegInfo.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		IDR_REGDATA			2000
#define		IDR_QUARDATA		2001
#define		DLLEXPORT		__declspec( dllexport) 

BEGIN_MESSAGE_MAP(CValCheckApp, CWinApp)
END_MESSAGE_MAP()

CValCheckApp::CValCheckApp()
{
}

CValCheckApp theApp;

BOOL CValCheckApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

extern "C" DWORD DLLEXPORT GetQuarantineData( DWORD &dwCount ) ;


TCHAR	strAppPath[512] = {0} ;
/***************************************************************************
  Function Name  : GetiTinPathFromReg
  Description    : Gets application folder path from registry, returns boolean.
  Author Name    : Ram
  S.R. No        : WRDWIZVALCHECKDLL_005
  Date           : 25th June 2014
****************************************************************************/
bool GetiTinPathFromReg()
{
	try
	{
		HKEY	hSubKey = NULL ;
		DWORD	dwSize = 511 ;
		DWORD	dwType = 0x00 ;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
			return true ;

		RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)strAppPath, &dwSize) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

		if( !PathFileExists( strAppPath) )
		{
			return true ;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in GetiTinPathFromReg", 0, 0, true, SECONDLEVEL);
	}
	return  false ;
}

/***************************************************************************
  Function Name  : DecryptData
  Description    : this function is use to decrypt data
  Author Name    : Ram
  S.R. No        :WRDWIZVALCHECKDLL_006
  Date           : 25th June 2014
****************************************************************************/
DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize )
{
	if( IsBadWritePtr( lpBuffer, dwSize ) )
		return 1 ;

	__try
	{

		DWORD	iIndex = 0 ;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			if(lpBuffer[iIndex] != 0)
			{
				if((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
				{
					lpBuffer[iIndex] = lpBuffer[iIndex];
				}
				else
				{
					lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
					jIndex++;
				}
				if (jIndex == WRDWIZ_KEYSIZE)
				{
					jIndex = 0x00;
				}
				if (iIndex >= dwSize)
				{
					break;
				}
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizVal Check DecryptData", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
/***************************************************************************
  Function Name  : AddRegisteredData
  Description    : this is exported function is use to add registered data
  Author Name    : Ram
  S.R. No        :WRDWIZVALCHECKDLL_007
  Date           : 25th June 2014
****************************************************************************/

extern "C" DWORD DLLEXPORT AddRegisteredData( LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() ) ;

	try
	{
		TCHAR	pDllPath[512] = {0} ;
		TCHAR	*pName = NULL ;

		//SYSTEMTIME		ActInfo = {0} ;
		LPBYTE	ActInfo = NULL ;

		ZeroMemory(strAppPath, sizeof(strAppPath) ) ;
		GetiTinPathFromReg() ;

		wsprintf( pDllPath, TEXT("%sVBEVALREG.DLL"), strAppPath) ;
		if( !PathFileExists( pDllPath ) )
			return 0x02 ;


		if( !dwResSize )
			return 0x03 ;

		if( !dwResType )
			return 0x04 ;

		if( !wcslen(pResName) )
			return 0x05 ;

		ActInfo = new BYTE[dwResSize + 1] ;
		if( !ActInfo )
			return 0x06 ;

		memset(ActInfo, 0, dwResSize + 1);
		memcpy(ActInfo, lpResBuffer, dwResSize + 1) ;

		ActInfo[dwResSize] = 0x00;

		if( DecryptData( (LPBYTE)ActInfo, dwResSize ) )
		{
			delete[] ActInfo ;
			return 0x07 ;
		}

		HANDLE	hUpdateRes = NULL ;

		hUpdateRes = BeginUpdateResource( pDllPath, FALSE ) ;
		if( !hUpdateRes )
		{
			delete[] ActInfo ;
			return 0x08 ;
		}

		if( !UpdateResource(hUpdateRes, MAKEINTRESOURCE(dwResType), pResName,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			ActInfo, dwResSize ) )
		{
			delete[] ActInfo ;
			return 0x09 ;
		}

		if( !EndUpdateResource(hUpdateRes, FALSE) )
		{
			delete[] ActInfo ;
			return 0x0A ;
		}
		delete[] ActInfo ;
		ActInfo = NULL;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in WardwizValCheck AddRegisteredData", 0, 0, true, SECONDLEVEL);
	}
	return 0x00 ;
}

/***************************************************************************
  Function Name  : GetRegisteredData
  Description    : this an Exported function is use to access specific registry data
  Author Name    : Ram
  S.R. No        :WRDWIZVALCHECKDLL_008	
  Date           : 25th June 2014
****************************************************************************/
extern "C" DWORD DLLEXPORT GetRegisteredData(LPBYTE lpResBuffer, DWORD &dwResDataSize, DWORD dwResType, TCHAR *pResName )
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() ) ;
	
	HMODULE	hRegDataDll = NULL ;
	DWORD	dwRet		= 0x00 ;

	try
	{
		HRSRC	hResource	= 0 ;
		HGLOBAL	hResData	= 0 ;
		DWORD	dwResSize	= 0 ;
		LPVOID	lpData		= NULL ;


		TCHAR	pDllPath[512] = {0} ;
		TCHAR	*pName = NULL ;

		ZeroMemory(strAppPath, sizeof(strAppPath) ) ;
		GetiTinPathFromReg() ;
		wsprintf( pDllPath, TEXT("%sVBEVALREG.DLL"), strAppPath) ;
		if( !PathFileExists( pDllPath ) )
			return 0x02 ;


		if( !dwResType )
			return 0x03 ;

		if( !wcslen(pResName) )
			return 0x04 ;

		hRegDataDll = LoadLibrary( pDllPath ) ;

		hResource = FindResourceEx(	hRegDataDll, MAKEINTRESOURCE(dwResType), 
			pResName, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL) ) ;
		dwResSize = SizeofResource( hRegDataDll, hResource ) ;
		if( !dwResSize )
		{
			dwRet = 0x05 ;
			//return dwRet ;
			goto Cleanup ;
		}
		hResData = LoadResource(hRegDataDll, hResource) ;
		if( hResData == NULL )
		{
			dwRet = 0x06 ;
			//return dwRet ;
			goto Cleanup ;
		}

		lpData = LockResource( hResData ) ;
		if( lpData == NULL )
		{
			dwRet = 0x07 ;
			goto Cleanup ;
			//return dwRet ;
		}

		if( IsBadWritePtr( lpResBuffer, dwResSize ) )
		{
			dwRet = 0x08 ;
			goto Cleanup ;
			//return 0x04 ;
		}

		memcpy(lpResBuffer, (LPBYTE)lpData, dwResSize ) ;
		if( DecryptData( (LPBYTE)lpResBuffer, dwResSize ) )
		{
			dwRet = 0x09 ;
			goto Cleanup ;
			//return dwRet ;
		}
		//Rajil Yadav 6.6.2014**********************/
		dwResDataSize = dwResSize ;
		//Need to check decrypted data is proper or not
		if(*((WORD *)&lpResBuffer[0]) != 0x004D)
		{
			dwRet = 0x5;
			goto Cleanup;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in WardwizValCheck GetRegisteredData", 0, 0, true, SECONDLEVEL);
	}
Cleanup :
	if( hRegDataDll != NULL)
	{
		FreeLibrary( hRegDataDll ) ;
		hRegDataDll = NULL ;
	}
	return dwRet ;
}
