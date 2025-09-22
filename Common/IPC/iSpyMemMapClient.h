/**********************************************************************************************************         	  Program Name          : iSpyServerMemMap_Client.h
	  Description           : Wrapper class which used creates memory mapped files to communicate 
							  between two processes.
	  Author Name			: Ramkrushna Shelke                                                                     	  Date Of Creation      : 21 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#ifndef		_ISPYAV_CREATE_MEM__MAPPED_CLIENT_
#define		_ISPYAV_CREATE_MEM__MAPPED_CLIENT_
#pragma once

#include <Windows.h>
#include "PipeConstants.h"

///////////////////////////////////////////////////////////////
//
//		This job will executed by Main GUI
//		Created date	: 12-Nov-2013 10:15PM
//		Created By		: Vilas
//
//		Modified		:1. 16-Nov-2013 10:15PM, Added for repair entry for each 
///////////////////////////////////////////////////////////////




class iSpyServerMemMap_Client
{
public:
	iSpyServerMemMap_Client(MEMMAPTYPE eType) ;
	~iSpyServerMemMap_Client() ;

	DWORD OpenServerMemoryMappedFile() ;
	DWORD GetServerMemoryMappedFileData(PVOID pszData, DWORD dwSize ) ;

protected:

	MEMMAPTYPE	m_eType;
	HANDLE		m_hMapScanPath ;
	PVOID		m_pMapScanPath ;

	HANDLE		m_hEventMapDataChanged ;

} ;


#endif