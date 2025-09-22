/**********************************************************************************************************         	  Program Name          : iSpyServerMemMap_Server.cpp
	  Description           : Wrapper class which used creates memory mapped files and 
							  acts as server to communicate between two processes.
	  Author Name			: Ramkrushna Shelke                                                                     	  Date Of Creation      : 21 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "iSpyMemMapServer.h"

#include <shlwapi.h>

/***********************************************************************************************
  Function Name  : iSpyServerMemMap_Server
  Description    : Cont'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0092
  Date           : 21 Jan 2014
***********************************************************************************************/
iSpyServerMemMap_Server::iSpyServerMemMap_Server(MEMMAPTYPE eType):
	m_eType(eType)
{
	m_hMapScanPath	= NULL ;
	m_pMapScanPath	= NULL ;

	m_hEvent_UpdateMemory = NULL ;

	ZeroMemory(&m_mutexAttributes, sizeof(m_mutexAttributes)) ;
	ZeroMemory(&m_sd, sizeof(m_sd) ) ;

	m_bSecAttrib = false ;
}

/***********************************************************************************************
  Function Name  : ~iSpyServerMemMap_Server
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0093
  Date           : 21 Jan 2014
***********************************************************************************************/
iSpyServerMemMap_Server::~iSpyServerMemMap_Server()
{
	if( m_pMapScanPath )
		UnmapViewOfFile( m_pMapScanPath ) ;

	if( m_hMapScanPath )
		CloseHandle( m_hMapScanPath ) ;

	if( m_hEvent_UpdateMemory )
		CloseHandle( m_hEvent_UpdateMemory ) ;

	m_pMapScanPath = NULL ;
	m_hMapScanPath = NULL ;

	m_hEvent_UpdateMemory = NULL ;
}

/***********************************************************************************************
  Function Name  : CreateServerMappedSecurityAttribute
  Description    : Function which Creates security attributes to create memory mapped file
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0094
  Date           : 21 Jan 2014
***********************************************************************************************/
void iSpyServerMemMap_Server::CreateServerMappedSecurityAttribute( )
{
	__try
	{

		m_mutexAttributes.nLength = sizeof( SECURITY_ATTRIBUTES ) ;
		m_mutexAttributes.bInheritHandle = FALSE ;

		BOOL	bOk = InitializeSecurityDescriptor( &m_sd, SECURITY_DESCRIPTOR_REVISION ) ;
		if( bOk == 0 )
		{
			//AddToLog("Failed To InitializeSecurityDescriptor");
			return ;
		}

		bOk = SetSecurityDescriptorDacl(&m_sd, TRUE, (PACL)NULL, FALSE ) ;
		if( bOk == 0 )
		{
			//AddToLog("Failed SetSecurityDescriptorDacl");
			return ;
		}

		m_mutexAttributes.lpSecurityDescriptor = &m_sd ;
		m_bSecAttrib = true ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizServerMemMap_Server::CreateServerMappedSecurityAttribute", 0, 0, true, 2);
	}
}

/***********************************************************************************************
  Function Name  : CreateServerMemoryMappedFile
  Description    : Function which Creates memory mapped files
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0089
  Date           : 21 Jan 2014
***********************************************************************************************/
DWORD iSpyServerMemMap_Server::CreateServerMemoryMappedFile( )
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
				_tcscpy_s(szMapFilePath	, MAX_PATH , szMapRootkitName);
				_tcscpy_s(szMapEventName , MAX_PATH , szMapRootkitEventname);
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

		CreateServerMappedSecurityAttribute() ;

		m_hEvent_UpdateMemory = CreateEvent(&m_mutexAttributes, FALSE, TRUE, szMapEventName ) ;
		if( !m_hEvent_UpdateMemory )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		m_hMapScanPath = CreateFileMapping(	INVALID_HANDLE_VALUE, &m_mutexAttributes, PAGE_READWRITE,
											0, MAPSCANPATH_BUFSIZE, szMapFilePath) ;
		if( !m_hMapScanPath )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		m_pMapScanPath = (LPTSTR) MapViewOfFile(	m_hMapScanPath, FILE_MAP_ALL_ACCESS, 0, 0, MAPSCANPATH_BUFSIZE) ;
		if( m_pMapScanPath )
			goto Cleanup ;

		dwRet = 0x03 ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizServerMemMap_Server::CreateServerMemoryMappedFile", 0, 0, true, 2);
	}
Cleanup:
	return dwRet;
}

/***********************************************************************************************
  Function Name  : UpdateServerMemoryMappedFile
  Description    : Function which updates the buffer in memory mapped files.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0095
  Date           : 21 Jan 2014
***********************************************************************************************/
DWORD iSpyServerMemMap_Server::UpdateServerMemoryMappedFile( PVOID pszData, DWORD dwDataLen )
{
	DWORD	dwRet = 0x00 ;

	__try
	{
		if( (dwDataLen > MAPSCANPATH_BUFSIZE) || (!dwDataLen) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( IsBadReadPtr(pszData, dwDataLen) )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if( !m_pMapScanPath )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		CopyMemory( m_pMapScanPath, pszData, dwDataLen ) ;

		//SetEvent to
		SetEvent( m_hEvent_UpdateMemory ) ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WardwizServerMemMap_Server::UpdateServerMemoryMappedFile", 0, 0, true, 2);
	}
Cleanup:
	return dwRet ;
}

/***********************************************************************************************
  Function Name  : UpdateServerMemoryMappedFile
  Description    : Function which updates ZERO MEMORY in memory mapped file
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0096
  Date           : 21 Jan 2014
***********************************************************************************************/
DWORD iSpyServerMemMap_Server::UpdateServerMemoryMappedFile( )
{
	DWORD	dwRet = 0x00 ;

	__try
	{
		if( m_pMapScanPath )
			ZeroMemory(m_pMapScanPath, MAPSCANPATH_BUFSIZE ) ;

		goto Cleanup ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup:

	return dwRet ;
}
