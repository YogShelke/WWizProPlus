/**********************************************************************************************************         	  Program Name          : iSpyServerMemMap_Client.cpp
	  Description           : Wrapper class which used to open memory mapped files and acts as client 
							  to communicate between two processes.
	  Author Name			: Ramkrushna Shelke                                                                     	  Date Of Creation      : 21 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "iSpyMemMapClient.h"
#include <shlwapi.h>

/***********************************************************************************************
  Function Name  : iSpyServerMemMap_Client
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0088
  Date           : 21 Jan 2014
***********************************************************************************************/
iSpyServerMemMap_Client::iSpyServerMemMap_Client(MEMMAPTYPE eType):
	m_eType(eType)
{
	m_hMapScanPath	= NULL ;
	m_pMapScanPath	= NULL ;

	m_hEventMapDataChanged = NULL ;

}

/***********************************************************************************************
  Function Name  : ~iSpyServerMemMap_Client
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0089
  Date           : 21 Jan 2014
***********************************************************************************************/
iSpyServerMemMap_Client::~iSpyServerMemMap_Client()
{
	if( m_pMapScanPath )
		UnmapViewOfFile( m_pMapScanPath ) ;

	if( m_hMapScanPath )
		CloseHandle( m_hMapScanPath ) ;

	if( m_hEventMapDataChanged )
		CloseHandle( m_hEventMapDataChanged ) ;

	m_pMapScanPath = NULL ;
	m_hMapScanPath = NULL ;

	m_hEventMapDataChanged = NULL ;
}

/***********************************************************************************************
  Function Name  : OpenServerMemoryMappedFile
  Description    : function which opens memory mapped files
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0090
  Date           : 21 Jan 2014
***********************************************************************************************/
DWORD iSpyServerMemMap_Client::OpenServerMemoryMappedFile( )
{
	DWORD	dwRet = 0x00 ;

	__try
	{
		TCHAR szMapFilePath[MAX_PATH] = {0};
		TCHAR szMapEventName[MAX_PATH] = {0};


		switch(m_eType)
		{
			case FILESYSTEMSCAN:
				_tcscpy_s(szMapFilePath, MAX_PATH, szMapScanPathName);
				_tcscpy_s(szMapEventName, MAX_PATH, szMapScanEventName);
				break;
			case REGISTRYOPTIMIZER:
				_tcscpy_s(szMapFilePath, MAX_PATH, szMapRegOptimzerName);
				_tcscpy_s(szMapEventName, MAX_PATH, szMapRegOptimzerEventName);
				break;
			case VIRUSFOUNDENTRY:
				_tcscpy_s(szMapFilePath, MAX_PATH, szMapVirFoundEntryName);
				_tcscpy_s(szMapEventName, MAX_PATH, szMapVirFoundEntryEventName);
				break;
			case ROOTKIT:
				_tcscpy_s(szMapFilePath, MAX_PATH, szMapRootkitName);
				_tcscpy_s(szMapEventName, MAX_PATH, szMapRootkitEventname);
				break;
			case ALUPDATE:
				_tcscpy_s(szMapFilePath	, MAX_PATH , szMapALUpdateName);
				_tcscpy_s(szMapEventName , MAX_PATH , szMapALUpdateEventName);
				break;
			default:
				dwRet = 0x01 ;
				goto Cleanup ;
				break;
		}

		m_hEventMapDataChanged = OpenEvent( SYNCHRONIZE, FALSE, szMapEventName ) ;
		if( !m_hEventMapDataChanged )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		m_hMapScanPath = OpenFileMapping(	FILE_MAP_READ, FALSE, szMapFilePath ) ;
		if( !m_hMapScanPath )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		m_pMapScanPath = (LPTSTR) MapViewOfFile(	m_hMapScanPath, FILE_MAP_READ, 0, 0, MAPSCANPATH_BUFSIZE) ;
		if( m_pMapScanPath )
			goto Cleanup ;

		dwRet = 0x03 ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in iWardwizServerMemMap_Client::OpenServerMemoryMappedFile", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet ;
}

/***********************************************************************************************
  Function Name  : GetServerMemoryMappedFileData
  Description    : function which gets memory mapped data 
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0091
  Date           : 21 Jan 2014
***********************************************************************************************/
DWORD iSpyServerMemMap_Client::GetServerMemoryMappedFileData( PVOID pszData, DWORD dwSize )
{
	DWORD	dwRet = 0x00, dwWaitState = 0x00 ;

	__try
	{
		if( (dwSize > MAPSCANPATH_BUFSIZE) || (!dwSize) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}
		
		if( !m_pMapScanPath )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		if( !m_hEventMapDataChanged )
		{
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		dwWaitState = WaitForSingleObject( m_hEventMapDataChanged, 5*1000 ) ;
		ZeroMemory(pszData, dwSize ) ;
		CopyMemory(pszData, m_pMapScanPath, dwSize ) ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in iWardwizServerMemMap_Client::GetServerMemoryMappedFileData", 0, 0, true, SECONDLEVEL);
	}
Cleanup:
	return dwRet ;
}
