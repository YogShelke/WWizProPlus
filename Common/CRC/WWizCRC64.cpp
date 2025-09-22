/**********************************************************************************************************
Program Name          : WWizCRC64
Description           : This defines to calculate CRC64 but hash index from buffer
Author Name           : Ramkrushna Shelke
Date Of Creation      : 04/06/2016
Version No            : 1.14.1.10
Special Logic Used    :
Modification Log      :
1. Name    : Description
***********************************************************************************************************/
#include "StdAfx.h"
#include "WWizCRC64.h"

CWWizCRC64::CWWizCRC64()
{

}

CWWizCRC64::~CWWizCRC64()
{

}

/***************************************************************************
Function Name  : InitCRC
Description    : Initialize a CRC accumulator
Author Name    : Ramkrushna Shelke
Date           : 04/06/2016
****************************************************************************/
void CWWizCRC64::InitCRC(UINT64 & crc) 
{
	crc = UINT64CONST(0xffffffffffffffff);
}

/***************************************************************************
Function Name  : FinishCRC
Description    : Finish a CRC calculation
Author Name    : Ramkrushna Shelke
Date           : 04/06/2016
****************************************************************************/
void CWWizCRC64::FinishCRC(UINT64 & crc)
{
	crc ^= UINT64CONST(0xffffffffffffffff);
}

/***************************************************************************
Function Name  : CalcCRC64
Description    : Accumulate some (more) bytes into a CRC 
Author Name    : Ramkrushna Shelke
Date           : 04/06/2016
****************************************************************************/
void CWWizCRC64::CalcCRC64(UINT64 & crc, PBYTE pbData, size_t len)
{
	InitCRC(crc);

	/* Constant table for CRC calculation */
	PBYTE		__pbData = pbData;
	size_t		__len = len;

	while (__len-- > 0)
	{
		UINT	__tab_index = ((UINT)(crc >> 56) ^ *__pbData++) & 0xFF;
		crc = crc64_table[__tab_index] ^ (crc << 8);
	}
}