#ifndef		ITINDRIVERINFO_ENUMHIVE_
#define		ITINDRIVERINFO_ENUMHIVE_

#pragma once

struct offsets
{ 
    long  block_size;
    char  block_type[2]; // "lf" "il" "ri"
    short count;   
    long  first; 
    long  hash; 
};

struct key_block
{ 
    long  block_size;
    char  block_type[2]; // "nk"
    char  dummya[18];
    int   subkey_count;
    char  dummyb[4];
    int   subkeys;
    char  dummyc[4];
    int   value_count;
    int   offsets;
    char  dummyd[28];
    short len;
    short du;
    char  name; 
};

struct value_block
{
    long  block_size;
    char  block_type[2]; // "vk"
    short name_len;
    long  size;
    long  offset;
    long  value_type;
    short flags;
    short dummy;
    char  name; 
};




class ITinEnumDriversHive
{
public:
		ITinEnumDriversHive() ;
		~ITinEnumDriversHive() ;

		bool IsDriverInDatabase(TCHAR *pDriverPath, DWORD dwLen ) ;
		void memcpy_Own(TCHAR *pDest, char *pSou, short len ) ;
		void walk ( char* path,   key_block* key ) ;

protected:
		bool	bImagePath ;
		DWORD	dwDriverType ;
		DWORD	dwDriverPathLen ;
		DWORD	dwDriverPathOff ;

		bool	bControlSet001 ;
		bool	bControlSet002 ;
		bool	bServices ;

		TCHAR	szDriverPath[0x100] ;

};

#endif