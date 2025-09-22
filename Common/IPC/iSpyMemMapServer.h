/**********************************************************************************************************         	  Program Name          : iSpyServerMemMap_Server.h
	  Description           : Wrapper class which used creates memory mapped files and 
							  acts as server to communicate between two processes.
	  Author Name			: Ramkrushna Shelke                                                                     	  Date Of Creation      : 21 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#ifndef		_ISPYAV_CREATE_MEM__MAPPED_SERVER_
#define		_ISPYAV_CREATE_MEM__MAPPED_SERVER_

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

class iSpyServerMemMap_Server
{
public:
	iSpyServerMemMap_Server(MEMMAPTYPE eType) ;
	~iSpyServerMemMap_Server() ;

	void CreateServerMappedSecurityAttribute( ) ;

	DWORD CreateServerMemoryMappedFile() ;
	DWORD UpdateServerMemoryMappedFile( ) ;
	DWORD UpdateServerMemoryMappedFile(PVOID pszData, DWORD dwDataLen ) ;

protected:

	MEMMAPTYPE				m_eType;
	HANDLE					m_hMapScanPath ;
	PVOID					m_pMapScanPath ;

	SECURITY_ATTRIBUTES		m_mutexAttributes;
	SECURITY_DESCRIPTOR		m_sd;
	bool					m_bSecAttrib ;

	HANDLE					m_hEvent_UpdateMemory ;
} ;


#endif