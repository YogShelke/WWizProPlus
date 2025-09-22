/********************************************************************************************************** 
   Program Name          : ITinEnumDriversHive.cpp
   Description           : This class finds the hidden rootkit from registry which compares the results of
						   Enumeration using WIAPI's and Read hive file from Hard Disk.
   Author Name           : Vilas Suvarnakar                                                                                
   Date Of Creation      : 7/24/2014
   Version No            : 1.0.0
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description
***********************************************************************************************************/

#include "stdafx.h"

#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <vector>

#include "ITinDriverInfo.h"
#include "ITinEnumDriversHive.h"


//FILE *fp = NULL ;

//TCHAR	szDriverPath[0x100] = {0} ;
TCHAR	szServiceKeyName[0x40] = {0} ;
TCHAR	szControlSet001[] = _T("ControlSet001") ;
TCHAR	szControlSet002[] = _T("ControlSet002") ;

/*
bool	bImagePath		= false ;
DWORD	dwDriverType	= 0x00 ;
DWORD	dwDriverPathLen	= 0x00 ;
DWORD	dwDriverPathOff = 0x00 ;

bool	bControlSet001	= false ;
bool	bControlSet002	= false ;
bool	bServices		= false ;
*/

/***************************************************************************
  Function Name  : ITinEnumDriversHive
  Description    : Constructor
  Author Name    : Vilas Suvarnakar
  S.R. No        : 
  Date           : 7/24/2014
****************************************************************************/
ITinEnumDriversHive::ITinEnumDriversHive()
{
	bImagePath		= false ;
	dwDriverType	= 0x00 ;
	dwDriverPathLen	= 0x00 ;
	dwDriverPathOff = 0x00 ;

	bControlSet001	= false ;
	bControlSet002	= false ;
	bServices		= false ;

	ZeroMemory(szDriverPath, sizeof(szDriverPath) ) ;
}

/***************************************************************************
  Function Name  : ITinEnumDriversHive
  Description    : Destructor
  Author Name    : Vilas Suvarnakar
  S.R. No        : 
  Date           : 7/24/2014
****************************************************************************/
ITinEnumDriversHive::~ITinEnumDriversHive()
{

}

/***************************************************************************
  Function Name  : IsDriverInDatabase
  Description    : Function which searches the drivers path in already created 
			       database to compare result.
  Author Name    : Vilas Suvarnakar
  S.R. No        : WRDWIZRKSCNDLL_061
  Date           : 7/24/2014
****************************************************************************/
bool ITinEnumDriversHive::IsDriverInDatabase(TCHAR *pDriverPath, DWORD dwLen )
{

	bool	bRet = false ;
	DWORD	iCount = 0x00, i=0x00 ;

	iCount = static_cast<DWORD>(vDriverInfo_Hive.size());
	if( !iCount )
		return bRet ;

	for(i=0x00; i<iCount; i++ )
	{
		if( vDriverInfo_Hive[i].dwPathLength == dwLen )
		{
			if( memcmp(vDriverInfo_Hive[i].szDriverPath, pDriverPath, dwLen) == 0 )
			{
				bRet = true ;
				break ;
			}
		}
	}

	return bRet ;
}

/***************************************************************************
  Function Name  : memcpy_Own
  Description    : This function takes source path and destination path as 
			       input and copies the string from source path to destination.
  Author Name    : Vilas Suvarnakar
  S.R. No        : WRDWIZRKSCNDLL_062
  Date           : 7/24/2014
****************************************************************************/
void ITinEnumDriversHive::memcpy_Own(TCHAR *pDest, char *pSou, short len )
{
	short	i=0 ;

	for(i=0; i<len; i++ )
		pDest[i] = (TCHAR )pSou[i] ;

	pDest[i] = '\0' ;
}

/***************************************************************************
  Function Name  : walk
  Description    : This function reads the hive file buffer from hard disk
		  		   and walks with hive Structure size to get key, value data part
				   from regitry hive file.
  Author Name    : Vilas Suvarnakar
  S.R. No        : WRDWIZRKSCNDLL_063
  Date           : 7/24/2014
****************************************************************************/
void ITinEnumDriversHive::walk (char* path,   key_block* key )
{
	if(!path || !key )
	{
		return;
	}

	static  char* root = ((char*)(key)) - 0x20, *full = path;
	__try
	{
		if(!root)
		{
			return;
		}

		// add current key name to printed path
		memcpy(path++,"/",2); 
		memcpy(path,&key->name,key->len); 
		path+=key->len;

		if( (key->len < 33) && (key->len > 0) )
		{
			memset(szServiceKeyName, 0x00, sizeof(szServiceKeyName) ) ;
			//wcscpy((wchar_t *)szServiceKeyName, (wchar_t *)&key->name ) ;
			memcpy_Own(szServiceKeyName, &key->name, key->len ) ;
			if( memcmp(szServiceKeyName, szControlSet001, 0x0D*sizeof(TCHAR) ) == 0x00 )
				bControlSet001 = true ;

			if( memcmp(szServiceKeyName, szControlSet002, 0x0D*sizeof(TCHAR) ) == 0x00 )
			{
				bControlSet002 = true ;
				bServices = false ;
			}
		}

		//if( memicmp(szServiceKeyName, L"services", 0x08) == 0 )
		if( (szServiceKeyName[0]=='s' || szServiceKeyName[0]=='S' )&& szServiceKeyName[1]=='e' &&szServiceKeyName[2]=='r' &&szServiceKeyName[3]=='v' &&
			szServiceKeyName[4]=='i' &&szServiceKeyName[5]=='c' &&szServiceKeyName[6]=='e' &&szServiceKeyName[7]=='s' && bControlSet001 )
			//wcscpy((wchar_t *)szServiceKeyName, L"services" ) ;
			bServices = true ;

		// print all contained values

		if( bServices )
		{

			dwDriverType	= 0x00 ;
			dwDriverPathOff = 0x00 ;
			dwDriverPathLen = 0x00 ;
			bImagePath		= false ;

			for(int o = 0; o < key->value_count; o++)
			{
				value_block* val = (value_block*)(((int*)(key->offsets+root+4))[o]+root); 
		        
				// we skip nodes without values
				if(!val->offset)  continue;
		        
				// data are usually in separate blocks without types
				char	*data = root+val->offset+4;
				TCHAR	*pValue = NULL ;

				if( data )
					pValue = (TCHAR* )data ;

				// notice that we use memcopy for key/value names everywhere instead of strcat
				// reason is that malware/wiruses often write non nulterminated strings to
				// hide from win32 api
				*path='/'; if(!val->name_len) *path=' '; 
				memcpy(path+1, &val->name, val->name_len) ;
				path[val->name_len+1] = 0 ;

				// but for small values MS added optimization where 
				// if bit 31 is set data are contained wihin the key itself to save space
				if( (val->size&1<<31) && (strcmp(path, "/Type")==0x00) )
				{
					data = (char*)&val->offset ;
					dwDriverType = val->offset ;
				}


				if( pValue )
				{
					if( (val->size < 0x100) && (val->size > 0x00 ) && (strcmp(path, "/ImagePath")==0x00) )
					{
						//wcscpy_s((wchar_t *)szTemp, 1023, (wchar_t *)pValue ) ;
						memset(szDriverPath, 0x00, sizeof(szDriverPath) ) ;
						//wcscpy((wchar_t *)szTemp, (wchar_t *)pValue ) ;
						memcpy(szDriverPath, pValue, val->size ) ;
						bImagePath		= true ;
						dwDriverPathLen	= val->size ;
						dwDriverPathOff = 0x1000 + val->offset + 0x04 ;
					}
				}

				//if( (dwDriverType == 0x01) && (bImagePath) )
				if( ( (dwDriverType == 0x01) || (dwDriverType == 0x02) || (dwDriverType == 0x08)) && (bImagePath) )
				{

					ZeroMemory(&sDriverInfo_Hive, sizeof(ITINDRIVERINFO_HIVE) ) ;
					sDriverInfo_Hive.dwPathLength = dwDriverPathLen ;
					sDriverInfo_Hive.dwPathOffset = dwDriverPathOff ;
					//wcscpy( sDriverInfo_Hive.szDriverName, szServiceKeyName ) ;
					//wcscpy( sDriverInfo_Hive.szDriverPath, szDriverPath ) ;

					wcscpy( (wchar_t *)sDriverInfo_Hive.szDriverName, (wchar_t *)szServiceKeyName ) ;
					wcscpy( (wchar_t *)sDriverInfo_Hive.szDriverPath, (wchar_t *)szDriverPath ) ;

					if( !IsDriverInDatabase(szDriverPath, dwDriverPathLen) )
						vDriverInfo_Hive.push_back( sDriverInfo_Hive ) ;

					break ;
				}

			}
		}
		// for simplicity we can imagine keys as directories in filesystem and values
		// as files.
		// and since we already dumped values for this dir we will now iterate 
		// thru subdirectories in the same way

		offsets* item = (offsets*)(root+key->subkeys) ;
		if( item )
		{
			for(int i=0;i<item->count;i++)
			{
				// in case of too many subkeys this list contain just other lists
				offsets* subitem = (offsets*)((&item->first)[i]+root);

				__try
				{
					/*
					ISSUE NO - 2:
					In Wardwiz Essential if we select windows registry to scan after scan is completed,
					the files scanned count is 0.
					NAME - Niranjan Deshak. - 13th Jan 2015
					*/
					//Issue : Crash in Rootkit scanner
					/*
					if( IsBadReadPtr(subitem, sizeof(offsets) ) )
						return;
					*/

					// usual directory traversal
					if(item->block_type[1]=='f'||item->block_type[1]=='h')
					{
						// for now we skip hash codes (used by regedit for faster search)
						walk(path,(key_block*)((&item->first)[i*2]+root));
					} 
					else for(int j=0;j<subitem->count;j++)
					{
						if(item->block_type[1]=='i')
						{
								int g=0;
						}
						// also ms had chosen to skip hashes altogether in this case 
						walk(path,(key_block*)((&subitem->first)[item->block_type[1]=='i'?j*2:j]+root));
					}
				}
				__except( EXCEPTION_EXECUTE_HANDLER )
				{
				}
			}
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//Issue : Crashing unlimited times and UI hangs so logging removed.
		//Date	: 17 Jan -2015
		//AddLogEntry(L"### Exception in ITinEnumDriversHive::walk", 0, 0, true, SECONDLEVEL);
	}
}

