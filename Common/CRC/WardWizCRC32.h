#pragma once

#ifndef _WARDWIZ_CRC32_H_
#define _WARDWIZ_CRC32_H_

#include <Windows.h>
#include <stdio.h>


///////////////////////////////////////////////////////////////
//		CWardWizCRC32	: class used to calculate CRC32 of a given Buffer of type BYTE
//
//		Created date	: 07-Oct-2014
//		Modified date	:
//		Created By		: Vilas
///////////////////////////////////////////////////////////////
class CWardWizCRC32
{
public:
	CWardWizCRC32() ;
	~CWardWizCRC32() ;

	void Init( );

	DWORD GetCheckSum(LPBYTE lpbBuffer, DWORD dwLen, DWORD &dwCheckSum );
	void CalcCrc32(const BYTE byte, DWORD &dwCRC32);

protected:

	DWORD m_dwCRC32Table[0x100];
	DWORD m_State[0x08];
};

//HOW TO USE THIS CLASS
/*

	BYTE	bInupt[] = {0x10, 0x20, 0x30, 0x40, 0x050, 0x60, 0x70, 0x80 } ;

	DWORD	dwLen = 0x08;
	DWORD	dwCRC = 0x00;

	CWardWizCRC32	objCRC32;

	DWORD	dwRet = objCRC32.GetCheckSum(bInupt, dwLen, dwCRC);

	if( dwRet )
	{
		//Error while calculating CRC of a given buffer "bInupt"
	}

*/

#endif
