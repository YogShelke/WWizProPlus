// WWizHash.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "WWizHash.h"

#include <Windows.h>
#include <shlwapi.h>

#include "Hash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		DLLEXPORT	extern "C" __declspec(dllexport)

extern void PrintMD5(uchar md5Digest[16], char *);

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CWWizHashApp

BEGIN_MESSAGE_MAP(CWWizHashApp, CWinApp)
END_MESSAGE_MAP()


// CWWizHashApp construction

CWWizHashApp::CWWizHashApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWWizHashApp object

CWWizHashApp theApp;


// CWWizHashApp initialization

BOOL CWWizHashApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

/***************************************************************************************************                    
*  Function Name  :GetFileHash()
*  Description    :Get MD5 of files. 
*  Author Name    :Vilas                                                                                   
*  Date           :4- Jul -2014 - 12 jul -2014
*  SR No          :WWIZHASH_0001
****************************************************************************************************/

DLLEXPORT DWORD GetFileHash(TCHAR *pFilePath, TCHAR *pFileHash)
{
	DWORD	dwResult = 0x00 ;

	FILE	*file = NULL ;
	int		nLen = 0x00 ;
	md5		alg ;

	char			szFileHash[64] = {0} ;
	unsigned char	chBuffer[1024] = {0} ;

	try
	{

		if( !PathFileExists(pFilePath) )
		{
			dwResult = 0x01 ;
			goto Cleanup ;
		}

		if( IsBadWritePtr( pFileHash, sizeof(TCHAR)*32 ))
		{
			dwResult = 0x02 ;
			goto Cleanup ;
		}


		memset(chBuffer, 0, 1024) ;
		file = _wfopen( pFilePath, L"rb" ) ;
		if( !file )
		{
			dwResult = 0x03 ;
			goto Cleanup ;
		}

		while (nLen = static_cast<int>(fread(chBuffer, 1, 1024, file)))
		{
			alg.Update(chBuffer, nLen) ;
			memset(chBuffer, 0x00, sizeof(chBuffer) ) ;
		}

		alg.Finalize() ;

		PrintMD5(alg.Digest(), szFileHash) ;
		if( szFileHash[0] )
			MultiByteToWideChar( CP_ACP, 0, szFileHash, -1, pFileHash, sizeof(TCHAR)*32 ) ;
		else
			dwResult = 0x04 ;

	}
	catch( ... )
	{

	}

Cleanup:

	if( file )
		fclose( file ) ;

	file = NULL ;

	return dwResult ;
}


/***************************************************************************************************                    
*  Function Name  :GetStringHash()
*  Description    :Get MD5 of string. 
*  Author Name    :Neha Gharge                                                                                   
*  Date           :11-9-2014
*  SR No          :WWIZHASH_0002
****************************************************************************************************/

DLLEXPORT DWORD GetStringHash(TCHAR *pString,int iStringlen, TCHAR *pFileHash)
{
	DWORD	dwResult = 0x00 ;

	int		nLen = 0x00 ;
	md5		alg ;

	char			szFileHash[64] = {0} ;
	unsigned char	chBuffer[1024] = {0} ;

	try
	{
		nLen = iStringlen;

		if( IsBadWritePtr( pFileHash, sizeof(TCHAR)*32 ))
		{
			dwResult = 0x01 ;
			goto Cleanup ;
		}

		WideCharToMultiByte(CP_ACP,	// code page
								0,						// performance and mapping flags
								(LPCWSTR) pString,		// wide-character string
								-1,						// number of chars in string
								(char*)chBuffer,					// buffer for new string
								MAX_PATH*2-2,			// size of buffer
								NULL,					// default for unmappable chars
								NULL);					// set when default char used

		//while( nLen )
		//{
			alg.Update(chBuffer, nLen) ;
		//	memset(chBuffer, 0x00, sizeof(chBuffer) ) ;
		//}

		alg.Finalize() ;

		PrintMD5(alg.Digest(), szFileHash) ;
		if( szFileHash[0] )
			MultiByteToWideChar( CP_ACP, 0, szFileHash, -1, pFileHash, sizeof(TCHAR)*32 ) ;
		else
			dwResult = 0x04 ;

	}
	catch( ... )
	{

	}
Cleanup:
	return dwResult ;
}

/***************************************************************************************************
*  Function Name  :GetStringHash()
*  Description    :Get MD5 of string, ths function is created to use in C# .NET
*  Author Name    :Ram Shelke
*  Date           :11-9-2014
*  SR No          :WWIZHASH_0002
****************************************************************************************************/
DLLEXPORT HRESULT  __stdcall FuncGetStringHash(char * strInputString, BSTR *bstrHash)
{
	bool bReturn = false;
	try
	{
		int iBufferSize = strlen(strInputString);
		md5		alg;
		alg.Update((uchar*)strInputString, iBufferSize);
		alg.Finalize();

		char			szBufHash[64] = { 0 };
		PrintMD5(alg.Digest(), szBufHash);
		CString csHash(szBufHash);
		*bstrHash = SysAllocString(csHash.GetBuffer());
		return S_OK;
	}
	catch (...)
	{
	}
	return S_FALSE;
}

/***************************************************************************************************
*  Function Name  :GetByteStringHash()
*  Description    :Get MD5 of BYTE array.
*  Author Name    :Vilas Suvarnakar
*  Date           :12-01-2016
*  SR No          :WWIZHASH_0003
****************************************************************************************************/
DLLEXPORT DWORD GetByteStringHash(LPBYTE lpBuffer, DWORD dwBufferSize, LPBYTE lpBufferHash)
{
	DWORD	dwResult = 0x00;

	int		nLen = 0x00;
	md5		alg;

	char			szFileHash[64] = { 0 };
	unsigned char	chBuffer[1024] = { 0 };

	try
	{

		if (IsBadWritePtr(lpBufferHash, sizeof(BYTE)*0x10))
		{
			dwResult = 0x01;
			goto Cleanup;
		}

		if (IsBadReadPtr(lpBuffer, dwBufferSize) )
		{
			dwResult = 0x02;
			goto Cleanup;
		}
		
		alg.Update(lpBuffer, dwBufferSize);
		
		alg.Finalize();

		memcpy(lpBufferHash, alg.Digest(), 0x10);
		
	}
	catch (...)
	{
		dwResult = 0x03;
	}

Cleanup:

	return dwResult;
}