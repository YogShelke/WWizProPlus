#pragma once
#ifndef _IP_HELPER_H_
#define _IP_HELPER_H_

/** 
	File: ip_helper.h
	Description: Contains the helper functions needed for manipulating the ip addresses
*/

extern "C"
{
    #include "ptstatus.h"
}
#include "ignis_dll.h"

#include <stdio.h>

#include <Objbase.h>
#include <stdlib.h>
#include <Ws2ipdef.h>
#include <Iphlpapi.h>


#ifdef GetField
#undef GetField
#endif

/**
	Displays the ip address to the console
*/
void DisplayNetInfo(
__in        NET_INFO*        Ip
);


/**
	Converts an array of CHARs to a GUID
*/
PTSTATUS ConvertStringToGUID(
__in CHAR*  GuidString,
__out GUID* Guid
);

/**
	Converts a GUID to an array of CHARs
*/
PTSTATUS ConvertGUIDToString(
	__in GUID*    Guid,
	__out CHAR*  GuidString
);

/**
	Displays the information about all network adapters
*/
void VisuallyDisplayNetwork(
    __in        IGNIS_ADAPTER*      Adapters,
    __in        DWORD               NoOfAdapters
);

/**
	Converts a IGNIS_IP structure to an array of CHARs
*/
PTSTATUS IgnisIpToCharArray(
__in    IGNIS_IP    Ip,
__in    IP_VERSION  Version,
__out   CHAR        (*ResultIp)[128]
);

/**
	Converts an array of BYTEs to an array of CHARs
*/
void MD5ValueToCharArray(
__in    BYTE*    MD5Value,
__out   CHAR(*text)[256]
);

PTSTATUS GetIPV4(
	__in    NET_INFO*        Ip,
	__out   CHAR(*ResultIp)[128]
);

PTSTATUS GetIPV6(
	__in    NET_INFO*        Ip,
	__out   CHAR(*ResultIp)[128]
);

PTSTATUS GetPhysAddr(
	__in    NET_INFO*        Ip,
	__out   CHAR(*ResultIp)[128]
	);
#endif // _IP_HELPER_H_