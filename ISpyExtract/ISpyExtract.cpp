/**********************************************************************************************************         	  Program Name          : ISpyExtractApp.cpp
	  Description           : Class which uses third partz DLL for zip/unzip purpose
	  Author Name			: Ramkrushna Shelke                                                                     	  Date Of Creation      : 20 Jan 2014
	  Version No            : 1.0.0.8
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyExtract.h"

#include "unrar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT _declspec(dllimport)
#define DLLEXPORT _declspec(dllexport)

//******************************************************************************
// Password of Local DB File upload (Zip File)
//******************************************************************************
CHAR WRDWIZLOCALUPDATEZIPPASS[20] = "WrdWizHash@4550";


typedef HANDLE (PASCAL *RAROPENARCHIVEEX)	(struct RAROpenArchiveDataEx *) ;
typedef int (PASCAL* RARREADHEADER)			(HANDLE hArcData,struct RARHeaderData *HeaderData);
typedef int (PASCAL* RARPROCESSFILEW)		(HANDLE hArcData,int Operation,wchar_t *DestPath,wchar_t *DestName);
typedef int (PASCAL* RARCLOSEARCHIVE)		(HANDLE hArcData);
typedef int(PASCAL* RARSETPASSWORDEx)			(HANDLE hArcData, char *Password);

RAROPENARCHIVEEX	RAROpenArchiveEx = NULL ;
RARREADHEADER		RARReadHeader = NULL ;
RARPROCESSFILEW		RARProcessFileW = NULL ;
RARCLOSEARCHIVE		RARCloseArchive = NULL ;
RARSETPASSWORDEx		RARSetPasswordEx = NULL;
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

// CISpyExtractApp

BEGIN_MESSAGE_MAP(CISpyExtractApp, CWinApp)
END_MESSAGE_MAP()


/***********************************************************************************************
  Function Name  : CISpyExtractApp
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZEXTRACTDLLL_001
  Date           : 20 Jan 2014
***********************************************************************************************/
CISpyExtractApp::CISpyExtractApp():
m_bStopUnrarOperation(false)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	hRAR = NULL ;
}

/***********************************************************************************************
  Function Name  : ~CISpyExtractApp
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZEXTRACTDLLL_002
  Date           : 20 Jan 2014
***********************************************************************************************/
CISpyExtractApp::~CISpyExtractApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	if( hRAR )
		FreeLibrary( hRAR ) ;

	hRAR = NULL ;
}



// The one and only CISpyExtractApp object

CISpyExtractApp theApp;



/***********************************************************************************************
  Function Name  : InitInstance
  Description    : overrided function which gets called when any application loads the DLL.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZEXTRACTDLLL_003
  Date           : 20 Jan 2014
***********************************************************************************************/
BOOL CISpyExtractApp::InitInstance()
{
	CWinApp::InitInstance() ;

	m_csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
	theApp.GetWardWizPathFromRegistry( ) ;

	return TRUE;
}

/***************************************************************************************************                *  Function Name  :          UnzipFile()
*  Description    :          unzip files. 
*  Author Name    :           Vilas          
  SR.NO			  : WRDWIZEXTRACTDLLL_004
*  Date  :					  4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
extern "C" DLLEXPORT DWORD UnzipFile(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount )
{
	AddLogEntry(L">>> UnzipFile : %s ", pZipFile, 0, true, FIRSTLEVEL);
	DWORD	dwRet = 0x00 ;
	__try
	{
		DWORD	dwCount = 0x00 ;
		DWORD	dwFormat = 0x00 ;

		theApp.GetZipformat( pZipFile, dwFormat ) ;

		/*	TCHAR	szTemp[128] = {0} ;

		swprintf(szTemp, L"%X", dwFormat ) ;
		MessageBox( NULL, szTemp, L"", 0 ) ;
		*/
		switch( dwFormat )
		{
		case 0x01:
			dwRet = theApp.UnzipZipFile(pZipFile, pUnzipPath, dwCount ) ;
			dwUnzipCount = dwCount ;
			break ;

		case 0x02:
			dwRet = theApp.GetRaRExports() ;
			if( dwRet )
				break ;
			dwRet = theApp.UnRARrarFile( pZipFile, pUnzipPath, dwCount ) ;
			dwUnzipCount = dwCount ;
			break ;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in DLLEXPORT DWORD UnzipFile", 0, 0, true, SECONDLEVEL);
	}
	return dwRet ;
}

/***************************************************************************************************                *  Function Name  : MakeZipFile()
*  Description    : zip files. 
*  Author Name    : Vilas
*  SR.NO		  : WRDWIZEXTRACTDLLL_005
*  Date			  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
extern "C" DLLEXPORT DWORD MakeZipFile(TCHAR *pFile, TCHAR *pZipPath )
{
	AddLogEntry(L">>> MakeZipFile : %s ", pFile, 0, true, FIRSTLEVEL);

	DWORD	dwRet = 0x00 ;
	__try
	{

		DWORD	dwCount = 0x00 ;
		HZIP	hZip = 0 ;


		hZip = CreateZip(pZipPath, 0, ZIP_FILENAME) ;
		if( hZip == 0 )
			return 1 ;

		if( PathIsDirectory( pFile ) )
			dwRet = theApp.EnumFolder( hZip, pFile ) ;
		else
			dwRet = theApp.AddZipFile(hZip, pFile ) ;

		//dwUnzipCount = dwCount ;

		CloseZip( hZip ) ;
		hZip = 0 ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in MakeZipFile: File Path: %s, Zip Path: %s ", pFile, pZipPath, true, FIRSTLEVEL);
	}
	return dwRet ;
}

/***************************************************************************************************                *  Function Name  : GetRaRExports()
*  Description    : Function which load unrar.dll and used its exported function to zip/Unzip.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_006
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
DWORD CISpyExtractApp::GetRaRExports()
{
	__try
	{

		TCHAR	szAppPath[512] = {0} ;
		//CString csAppFolder = GetWardWizPathFromRegistry( );

		/*	GetModuleFileName(NULL, szModulePath, MAX_PATH);
		TCHAR *szTemp = _tcsrchr(szModulePath, '\\');
		if( szTemp )
		*szTemp = '\0' ;
		_tcscat_s(szModulePath, 511, L"\\unrar.dll");
		*/

#ifdef MSOUTLOOK32
	swprintf(szAppPath, L"%sunrar32.dll", szModulePath ) ;
#else
	swprintf(szAppPath, L"%sunrar.dll", szModulePath ) ;
#endif

		if( !hRAR )
			hRAR = LoadLibrary( szAppPath ) ;

		RAROpenArchiveEx = (RAROPENARCHIVEEX) GetProcAddress(hRAR, "RAROpenArchiveEx") ;
		if( !RAROpenArchiveEx )
			return 1 ;

		RARReadHeader = (RARREADHEADER) GetProcAddress(hRAR, "RARReadHeader") ;
		if( !RARReadHeader)
			return 2 ;
		RARProcessFileW = (RARPROCESSFILEW) GetProcAddress(hRAR, "RARProcessFileW") ;
		if( !RARProcessFileW )
			return 3 ;

		RARCloseArchive = (RARCLOSEARCHIVE) GetProcAddress(hRAR, "RARCloseArchive") ;
		if( !RARCloseArchive )
			return 4 ;

		RARSetPasswordEx = (RARSETPASSWORDEx) GetProcAddress(hRAR, "RARSetPassword");
		if (!RARSetPasswordEx)
			return 5;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetRaRExports", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

/***************************************************************************************************                *  Function Name  : GetWardWizPathFromRegistry
*  Description    : Function gets the Appfolder path from registry.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_007
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
bool CISpyExtractApp::GetWardWizPathFromRegistry( )
{
	__try
	{
		HKEY	hSubKey = NULL ;

		memset(szModulePath, 0x00, sizeof(TCHAR)*512 ) ;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_csRegKeyPath, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
		{
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_csRegKeyPath, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
				return false ;
		}

		DWORD	dwSize = 511 ;
		DWORD	dwType = 0x00 ;

		RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize) ;
		RegCloseKey( hSubKey ) ;
		hSubKey = NULL ;

		if( PathFileExists( szModulePath) )
		{
			return true ;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::GetWardwizPathFromRegistry", 0, 0, true, SECONDLEVEL);
	}
	return false ;
}

/***************************************************************************************************                *  Function Name  : GetZipformat
*  Description    : Function which get the zip format.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_008
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
DWORD CISpyExtractApp::GetZipformat( TCHAR *pUnzipPath, DWORD &dwFormat )
{
	AddLogEntry(L">>> GetZipformat : %s ", pUnzipPath, 0, true, FIRSTLEVEL);

	DWORD	dwRet = 0x00 ;
	__try
	{

		DWORD	dwReadData = 0x00 ;
		DWORD	dwReadBytes = 0x00 ;

		HANDLE	hFile = INVALID_HANDLE_VALUE ;

		dwFormat = 0x00 ;
		hFile = CreateFile(	pUnzipPath, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			dwRet = 0x02 ;
			return dwRet ;
		}

		ReadFile( hFile, &dwReadData, 0x04, &dwReadBytes, NULL ) ;
		CloseHandle( hFile ) ;
		hFile = INVALID_HANDLE_VALUE ;

		/*	TCHAR	szTEmp[512] = {0} ;

		swprintf(szTEmp, L"%X", dwReadData ) ;
		AfxMessageBox(szTEmp);
		*/
		if( dwReadData == 0x21726152 )
			dwFormat = 0x02 ;

		if( (dwReadData&0xFFFF4B50) == dwReadData )
			dwFormat = 0x01 ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::GetZipformat", 0, 0, true, SECONDLEVEL);
	}
	return dwRet ;
}

/***************************************************************************************************                *  Function Name  : UnzipSingleFile
*  Description    : Exported function to zip single file.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_009
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
extern "C" DLLEXPORT DWORD UnzipSingleFile(TCHAR *pZipFile, TCHAR *pUnzipFile )
{
	AddLogEntry(L">>> CWardwizExtractApp::UnzipZipFile : %s to path : %s", pZipFile, pUnzipFile, true, FIRSTLEVEL);

	HZIP		hZip = NULL ;
	DWORD		dwRet = 0x00 ;

	__try
	{
		TCHAR	szUserPath[512] = {0} ;
		TCHAR	szUnzipPath[512] = {0} ;
		TCHAR	szUnzipFileName[256] = {0};
		TCHAR	*pFileName = NULL ;
		TCHAR	*pTemp = NULL ;

		ZIPENTRYW	ze = {0} ;

		if( !PathFileExists(pZipFile) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		GetEnvironmentVariable( L"ALLUSERSPROFILE", szUserPath, 511 ) ;
		wcscat( szUserPath, L"\\WRDWIZAV" ) ;
		if( !PathFileExists( szUserPath ) )
			CreateDirectory( szUserPath, NULL ) ;
		if( !PathFileExists( szUserPath ) )
		{
			Sleep( 100 ) ;
			CreateDirectory( szUserPath, NULL ) ;
		}

		swprintf(szUnzipPath, L"%s", pUnzipFile ) ;

		pTemp = wcsrchr(szUnzipPath, '\\' );
		if( pTemp )
		{
			wcscpy_s(szUnzipFileName, 255, pTemp+1 );
			*pTemp = '\0';
		}
		else
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if( !PathFileExists( szUnzipPath ) )
			CreateDirectory( szUnzipPath, NULL ) ;
		if( !PathFileExists( szUnzipPath ) )
		{
			Sleep( 100 ) ;
			_wmkdir( szUnzipPath ) ;
		}

		if( !PathFileExists( szUnzipPath ) )
			CreateDirectory( szUnzipPath, NULL ) ;

		if( !PathFileExists( szUnzipPath ) )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		SetCurrentDirectory( szUnzipPath ) ;

		if( PathFileExists(pUnzipFile) )
		{
			DeleteFile(pUnzipFile);
		}

		hZip = OpenZip( pZipFile, 0, ZIP_FILENAME) ;
		if( !hZip )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		GetZipItem(hZip, -1, &ze);

		int		numitems = ze.index ;
		for (int i=0; i<numitems; i++)
		{
			GetZipItem(hZip, i, &ze) ;
			pFileName = wcsrchr( ze.name, '/' ) ;
			if( pFileName )
			{
				pFileName++ ;
				if( wcslen( pFileName) )
					UnzipItem(hZip, i, szUnzipFileName, 0, ZIP_FILENAME) ;
			}
			else
				UnzipItem(hZip, i, szUnzipFileName, 0, ZIP_FILENAME) ;

			break;
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::GetZipformat", 0, 0, true, SECONDLEVEL);
	}

Cleanup :
	if( hZip )
		CloseZip( hZip ) ;
	hZip = NULL ;

	return dwRet ;
}

/***************************************************************************************************                *  Function Name  : UnzipZipFile
*  Description    : Exported function to unzip single file.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_010
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
DWORD CISpyExtractApp::UnzipZipFile(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount )
{
	AddLogEntry(L">>> CWardwizExtractApp::UnzipZipFile : %s to path : %s", pZipFile, pUnzipPath, true, FIRSTLEVEL);

	HZIP		hZip = NULL ;
	DWORD	dwRet = 0x00 ;

	__try
	{
		TCHAR	szUserPath[512] = {0} ;
		TCHAR	szUnzipPath[512] = {0} ;
		TCHAR	*pFileName = NULL ;
		TCHAR	*pTemp = NULL ;

		ZIPENTRYW	ze = {0} ;

		dwUnzipCount = 0x00 ;

		if( !PathFileExists(pZipFile) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		GetEnvironmentVariable( L"ALLUSERSPROFILE", szUserPath, 511 ) ;
		wcscat( szUserPath, L"\\WRDWIZAV" ) ;
		if( !PathFileExists( szUserPath ) )
			CreateDirectory( szUserPath, NULL ) ;
		if( !PathFileExists( szUserPath ) )
		{
			Sleep( 100 ) ;
			CreateDirectory( szUserPath, NULL ) ;
		}

		swprintf(szUnzipPath, L"%s\\%X", szUserPath, GetTickCount() ) ;
		if( !PathFileExists( szUnzipPath ) )
			CreateDirectory( szUnzipPath, NULL ) ;
		if( !PathFileExists( szUnzipPath ) )
		{
			Sleep( 100 ) ;
			_wmkdir( szUnzipPath ) ;
		}

		if( !PathFileExists( szUnzipPath ) )
			CreateOtherDirectory( szUnzipPath ) ;

		if( !PathFileExists( szUnzipPath ) )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		wcscpy( pUnzipPath, szUnzipPath ) ;
		SetCurrentDirectory( pUnzipPath ) ;

		hZip = OpenZip( pZipFile, 0, ZIP_FILENAME) ;
		if( !hZip )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		GetZipItem(hZip, -1, &ze);

		int		numitems = ze.index ;
		for (int i=0; i<numitems; i++)
		{
			GetZipItem(hZip, i, &ze) ;
			pFileName = wcsrchr( ze.name, '/' ) ;
			if( pFileName )
			{
				pFileName++ ;
				if( wcslen( pFileName) )
					UnzipItem(hZip, i, pFileName, 0, ZIP_FILENAME) ;
			}
			else
				UnzipItem(hZip, i, ze.name, 0, ZIP_FILENAME) ;

			memset(&ze, 0x00, sizeof(ze) ) ;
			pFileName = NULL ;
		}

		dwUnzipCount = numitems ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::GetZipformat", 0, 0, true, SECONDLEVEL);
	}

Cleanup :
	if( hZip )
		CloseZip( hZip ) ;
	hZip = NULL ;

	return dwRet ;
}

/***************************************************************************************************                *  Function Name  : CreateOtherDirectory
*  Description    : function which creates WardWiz directory in temp folder.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_011
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
DWORD CISpyExtractApp::CreateOtherDirectory( TCHAR *pUnzipDir )
{
	__try
	{

		if( PathFileExists( pUnzipDir ) )
			return 0x00 ;

		TCHAR	szTemp[512] = {0} ;
		TCHAR	szUnzipPath[512] = {0} ;

		GetEnvironmentVariable( L"TEMP", szTemp, 511 ) ;
		swprintf(szUnzipPath, L"%s\\%X", szTemp, GetTickCount() ) ;
		if( !PathFileExists( szUnzipPath ) )
			CreateDirectory( szUnzipPath, NULL ) ;

		if( !PathFileExists( szUnzipPath ) )
		{
			Sleep( 100 ) ;
			CreateDirectory( szUnzipPath, NULL ) ;
		}

		if( !PathFileExists( szUnzipPath ) )
		{
			Sleep( 100 ) ;
			_wmkdir( szUnzipPath ) ;
		}

		if( PathFileExists( szUnzipPath ) )
		{
			memset(pUnzipDir, 0x00, sizeof(TCHAR)*512 ) ;
			wcscpy(  pUnzipDir, szUnzipPath ) ;
			return 0x00 ;
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::CreateOtherDirectory", 0, 0, true, SECONDLEVEL);
	}
	return 0x01 ;
}

/***************************************************************************************************                
*  Function Name  : UnRARrarFile
*  Description    : function which used RAR DLL functionality to unrar a file
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_012
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
DWORD CISpyExtractApp::UnRARrarFile(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount )
{
	AddLogEntry(L">>> CWardwizExtractApp::UnRARrarFile : %s to path : %s", pZipFile, pUnzipPath, true, FIRSTLEVEL);

	DWORD	dwRet = 0x00 ;
	HANDLE		m_hArchive = NULL ;

	__try
	{
		TCHAR	szUserPath[512] = {0} ;
		TCHAR	szUnzipPath[512] = {0} ;
		TCHAR	*pFileName = NULL ;
		TCHAR	*pTemp = NULL ;

		char		CmtBuf[16384] = {0} ;


		int		ReadHeaderCode = 0x00 ;
		int		nReturnCode = 0x00 ;

		RAROpenArchiveDataEx	ArchiveData = {0} ;
		RARHeaderData			HeaderData = {0} ;

		dwUnzipCount = 0x00 ;

		if( !PathFileExists(pZipFile) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		GetEnvironmentVariable( L"ALLUSERSPROFILE", szUserPath, 511 ) ;
		wcscat( szUserPath, L"\\WRDWIZAV" ) ;
		if( !PathFileExists( szUserPath ) )
			CreateDirectory( szUserPath, NULL ) ;
		if( !PathFileExists( szUserPath ) )
		{
			Sleep( 100 ) ;
			CreateDirectory( szUserPath, NULL ) ;
		}

		swprintf(szUnzipPath, L"%s\\%X", szUserPath, GetTickCount() ) ;
		if( !PathFileExists( szUnzipPath ) )
			CreateDirectory( szUnzipPath, NULL ) ;
		if( !PathFileExists( szUnzipPath ) )
		{
			Sleep( 100 ) ;
			_wmkdir( szUnzipPath ) ;
		}

		if( !PathFileExists( szUnzipPath ) )
			CreateOtherDirectory( szUnzipPath ) ;

		if( !PathFileExists( szUnzipPath ) )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		wcscpy( pUnzipPath, szUnzipPath ) ;
		SetCurrentDirectory( pUnzipPath ) ;

		ArchiveData.ArcNameW = pZipFile ;
		ArchiveData.CmtBuf=CmtBuf;
		ArchiveData.CmtBufSize=sizeof(CmtBuf);
		ArchiveData.OpenMode=RAR_OM_EXTRACT;	
		m_hArchive = RAROpenArchiveEx(&ArchiveData) ;

		if( !m_hArchive )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		dwUnzipCount = 0x00 ;
		while( (ReadHeaderCode=RARReadHeader(m_hArchive,&HeaderData)==0) )
		{
			nReturnCode = RARProcessFileW(m_hArchive, RAR_EXTRACT, pUnzipPath, NULL) ;
			if( nReturnCode )
			{
				dwRet = 0x04 ;
				break ;
			}

			dwUnzipCount++ ;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::UnRARrarFile", 0, 0, true, SECONDLEVEL);
	}
Cleanup :
	if( m_hArchive )
		RARCloseArchive( m_hArchive ) ;
	m_hArchive = NULL ;

	return 0 ;
}

/***************************************************************************************************                *  Function Name  : AddZipFile
*  Description    : function which adds a file one by one in zip
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_013
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
DWORD	CISpyExtractApp::AddZipFile( HZIP hZipFile, TCHAR *pFileToAdd )
{
	DWORD	dwRet = 0x00 ;

	TCHAR	szFileName[128] = {0} ;
	TCHAR	*pFileName = NULL ;

	__try
	{

		pFileName = PathFindFileName( pFileToAdd ) ;
		if( !pFileName )
		{
			dwRet = 0x01 ;
			__leave ;
		}

		wcscpy(szFileName, pFileName ) ;
		if( ZipAdd(hZipFile, szFileName, pFileToAdd, 0, ZIP_FILENAME) )
			dwRet = 0x02 ;

		//dwUnzipCount++ ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::AddZipFile", 0, 0, true, SECONDLEVEL);
		dwRet = dwRet ;
	}
	return dwRet ;
}

/***************************************************************************************************                *  Function Name  : EnumFolder
*  Description    : Recursive function to enumerate folder.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZEXTRACTDLLL_014
*  Date  		  :	4- Jul -2014 - 12 jul -2014
****************************************************************************************************/
DWORD CISpyExtractApp::EnumFolder( HZIP hZipFile, TCHAR *pFolderPath )
{
	DWORD				dwRet = 0x00 ;
	__try
	{
		WIN32_FIND_DATA		FindFileData ;
		HANDLE				hFind = INVALID_HANDLE_VALUE ;

		TCHAR	szTemp[1024] = {0} ;

		swprintf(szTemp, L"%s\\*.*", pFolderPath ) ;

		hFind = FindFirstFile(szTemp, &FindFileData) ;
		if( hFind == INVALID_HANDLE_VALUE )
			return 1 ;

		TCHAR	szFile[1024] = {0} ;

		while( FindNextFile(hFind, &FindFileData) )
		{
			if( ( _wcsicmp(FindFileData.cFileName, L".") == 0 ) ||
				( _wcsicmp(FindFileData.cFileName, L"..") == 0 ) )
				continue ;

			if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				== FILE_ATTRIBUTE_DIRECTORY )
			{
				swprintf(szFile, L"%s\\%s", pFolderPath, FindFileData.cFileName ) ;
				EnumFolder( hZipFile, szFile ) ;
				/*dwRet = 0x02 ;
				break ;*/
			}

			swprintf(szFile, L"%s\\%s", pFolderPath, FindFileData.cFileName ) ;

			if( AddZipFile( hZipFile, szFile ) )
			{
				dwRet = 0x03 ;
				break ;
			}

			memset(szFile, 0x00, 1024 ) ;
		}

		FindClose( hFind ) ;
		hFind = INVALID_HANDLE_VALUE ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::EnumFolder, Folder Name: %s ", pFolderPath, 0, true, SECONDLEVEL);
	}
	return dwRet ;
}

/***************************************************************************************************
*  Function Name  :  StopUnrarOperation
*  Description    :  Function which stop UNRAR operation in middle.
*  Author Name    :  Amol Jaware
* SR.NO			  :  
*  Date			  :	 28th Sep 2016
****************************************************************************************************/
extern "C" DLLEXPORT bool StopUnrarOperation()
{
	bool bReturn = false;
	__try
	{
		theApp.m_bStopUnrarOperation = true;
		return true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in StopUnrarOperation", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************                
*  Function Name  :          UnRarForUpdates()
*  Description    :          Unrar the file for local updates
*  Author Name    :           Nitin Kolapkar
* SR.NO			  : 
*  Date  :					 15th Feb 2016
****************************************************************************************************/
extern "C" DLLEXPORT DWORD UnRarForUpdates(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount)
{
	AddLogEntry(L">>> UnzipFile : %s ", pZipFile, 0, true, FIRSTLEVEL);
	DWORD	dwRet = 0x00;
	__try
	{
		DWORD	dwCount = 0x00;
		DWORD	dwFormat = 0x00;

		theApp.GetZipformat(pZipFile, dwFormat);

		switch (dwFormat)
		{
		case 0x02:
			dwRet = theApp.GetRaRExports();
			if (dwRet)
				break;
			dwRet = theApp.UnRARrarFileForUpdates(pZipFile, pUnzipPath, dwCount);
			dwUnzipCount = dwCount;
			break;
		case 0x00:
			pUnzipPath = L"";
			dwUnzipCount = 0x00;

		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in DLLEXPORT DWORD UnRarForUpdates", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************************************                
*  Function Name  : UnRARrarFileForUpdates
*  Description    : UnRar the file for local updates 
*  Author Name    : Nitin Kolapkar
*  SR.NO		  : 
*  Date  		  :	15th Feb 2016
****************************************************************************************************/
DWORD CISpyExtractApp::UnRARrarFileForUpdates(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount)
{
	AddLogEntry(L">>> CWardwizExtractApp::UnRARrarFileForUpdates : %s to path : %s", pZipFile, pUnzipPath, true, FIRSTLEVEL);

	DWORD	dwRet = 0x00;
	HANDLE		m_hArchive = NULL;

	__try
	{
		TCHAR	szUserPath[512] = { 0 };
		TCHAR	szUnzipPath[512] = { 0 };
		TCHAR	*pFileName = NULL;
		TCHAR	*pTemp = NULL;

		char		CmtBuf[16384] = { 0 };


		int		ReadHeaderCode = 0x00;
		int		nReturnCode = 0x00;

		RAROpenArchiveDataEx	ArchiveData = { 0 };
		RARHeaderData			HeaderData = { 0 };

		dwUnzipCount = 0x00;

		if (!PathFileExists(pZipFile))
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		GetEnvironmentVariable(L"ALLUSERSPROFILE", szUserPath, 511);
		wcscat(szUserPath, L"\\Wardwiz Antivirus");
		if (!PathFileExists(szUserPath))
			CreateDirectory(szUserPath, NULL);
		if (!PathFileExists(szUserPath))
		{
			Sleep(100);
			CreateDirectory(szUserPath, NULL);
		}

		swprintf(szUnzipPath, L"%s\\%X", szUserPath, GetTickCount());
		if (!PathFileExists(szUnzipPath))
			CreateDirectory(szUnzipPath, NULL);
		if (!PathFileExists(szUnzipPath))
		{
			Sleep(100);
			_wmkdir(szUnzipPath);
		}

		if (!PathFileExists(szUnzipPath))
			CreateOtherDirectory(szUnzipPath);

		if (!PathFileExists(szUnzipPath))
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		ArchiveData.ArcNameW = pZipFile;
		ArchiveData.CmtBuf = CmtBuf;
		ArchiveData.CmtBufSize = sizeof(CmtBuf);
		ArchiveData.OpenMode = RAR_OM_EXTRACT;
		m_hArchive = RAROpenArchiveEx(&ArchiveData);
		RARSetPasswordEx(m_hArchive, WRDWIZLOCALUPDATEZIPPASS);

		if (!m_hArchive)
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		dwUnzipCount = 0x00;
		while ((ReadHeaderCode = RARReadHeader(m_hArchive, &HeaderData) == 0))
		{
			nReturnCode = RARProcessFileW(m_hArchive, RAR_EXTRACT, szUnzipPath, NULL);
			if (nReturnCode)
			{
				dwRet = 0x04;
				break;
			}

			//Need to stop unrar operation if user press cancel button in middle and break.
			if (m_bStopUnrarOperation)
			{
				dwUnzipCount = 0x00;
				m_bStopUnrarOperation = false;
				dwRet = 0x05;
				break;
			}
			dwUnzipCount++;
		}

		//copy here extract path
		wcscpy(pUnzipPath, szUnzipPath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizExtractApp::UnRARrarFileForUpdates", 0, 0, true, SECONDLEVEL);
	}
Cleanup:
	if (m_hArchive)
		RARCloseArchive(m_hArchive);
	m_hArchive = NULL;

	return 0;
}