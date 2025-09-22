/**********************************************************************************************************         	  Program Name          : ISpyExtractApp.h
	  Description           : Class which uses third partz DLL for zip/unzip purpose
	  Author Name			: Ramkrushna Shelke                                                                     	  Date Of Creation      : 20 Jan 2014
	  Version No            : 1.0.0.8
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "Zip.h"
#include "XUnzip.h"
#include "unrar.h"


#include <tchar.h>
#include <Shlwapi.h>

// CISpyExtractApp
// See ISpyExtract.cpp for the implementation of this class
//

class CISpyExtractApp : public CWinApp
{
public:
	CISpyExtractApp();
	~CISpyExtractApp();

// Overrides
public:

	TCHAR	szModulePath[512] ;
	HMODULE		hRAR  ;

	virtual BOOL InitInstance();

	DWORD GetRaRExports() ;
	bool GetWardWizPathFromRegistry( ) ;

	DWORD UnzipZipFile(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount ) ;
	DWORD UnRARrarFile(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount ) ;
	DWORD UnRARrarFileForUpdates(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount);

	DWORD CreateOtherDirectory( TCHAR *pUnzipDir ) ;
	DWORD GetZipformat( TCHAR *pUnzipPath, DWORD &dwFormat ) ;

	DWORD EnumFolder( HZIP hZipFile, TCHAR *pFolderPath ) ;
	DWORD AddZipFile( HZIP hZipFile, TCHAR *pFileToAdd ) ;
public:
	CString			m_csRegKeyPath;
	bool			m_bStopUnrarOperation;
	DECLARE_MESSAGE_MAP()
};
