#include "stdafx.h"
#include "WardWizCRC32.h"
#include <shlwapi.h>
#include <tchar.h>


CWardWizCRC32::CWardWizCRC32()
{
	Init();
}


CWardWizCRC32::~CWardWizCRC32()
{
}

void CWardWizCRC32::Init()
{

	try
	{

		ZeroMemory(m_dwCRC32Table, sizeof(m_dwCRC32Table) );
		ZeroMemory(m_State, sizeof(m_State) );

		m_State[0] = 0x6E4523B1;
		m_State[1] = 0xE2C5AB89;
		m_State[2] = 0x98BA75FE;
		m_State[3] = 0x1A32547C;
		m_State[4] = 0x68F521A4;
		m_State[5] = 0xD390B674;
		m_State[6] = 0x95A265FC;
		m_State[7] = 0x5729B1ED;

		DWORD	dwCrc, i, j;

		for(i = 0x00; i < 0x100; i++)
		{
			dwCrc = i;
			j = 0x07;

			while( true )
			{
				dwCrc += ( (dwCrc>>i) ^ m_State[j] ) + ((j << i) ^ m_State[j]) + ( i*m_State[j] );

				if( (0x00 == j) || (j > 0x07) )
					break;

				j--;
			}

			m_dwCRC32Table[i] = dwCrc;
		}
	}
	catch( ... )
	{
	}

}

void CWardWizCRC32::CalcCrc32(const BYTE byte, DWORD &dwCRC32)
{
	dwCRC32 = ((dwCRC32) >> 0x08) ^ m_dwCRC32Table[(byte) ^ (dwCRC32 & 0x000000FF)];
}


DWORD CWardWizCRC32::GetCheckSum(LPBYTE lpbBuffer, DWORD dwLen, DWORD &dwCheckSum )
{
	DWORD	dwRet = 0x00;
	DWORD	dwCRC = 0xFFFFFFFF;

	try
	{
		if( IsBadReadPtr(lpbBuffer, dwLen) )
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		if( !dwLen )
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		if( IsBadReadPtr(&dwCheckSum, sizeof(DWORD)) )
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		for(DWORD i=0x00; i<dwLen; i++ )
			CalcCrc32(lpbBuffer[i], dwCRC );

		dwCheckSum = dwCRC;

	}
	catch( ... )
	{
		dwRet = 0x04;
	}

Cleanup:

	if( dwRet>0x00 && dwRet<0x03 )
		dwCheckSum = 0x00;

	return dwRet;
}