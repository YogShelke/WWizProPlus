#pragma once

#ifndef _ADAPTER_HEADER_H_
#define _ADAPTER_HELPER_H_

/** 
	File: adapter_helper.h
	Description: Contains the helper functions needed for manipulating the network adapters
*/


extern "C"
{
#include "ptstatus.h"
}
#include "ignis_dll.h"


/**
	Cleanup all the adapters
*/
PTSTATUS CleanupAdaptersInfoClient(
__in        IGNIS_ADAPTER*      Adapters,
__in        DWORD               NoOfAdapters
);



/**
	Makes a deep copy of all the adapters.
	The client must call CleanupAdaptersInfoClient for each CopyAdaptersForClient call
*/
PTSTATUS CopyAdaptersClient(
__in    IGNIS_ADAPTER*          OldAdapters,
__in    DWORD                   NoOfAdapters,
__out   PIGNIS_ADAPTER*         NewAdapters
);


/** 
	Cleans up the information for one adapter
*/
static PTSTATUS FreeAdapterInfo(
__in        IGNIS_ADAPTER*      Adapter
);


/** 
	Makes a deep copy for one adapter
*/
static PTSTATUS DeepCopyAdapter(
__in        IGNIS_ADAPTER*      OldAdapter,
__out       IGNIS_ADAPTER*      NewAdapter
);

#endif // _ADAPTER_HELPER_H_