#include "stdafx.h"
#include <stdlib.h>

#include "adapter_helper.h"


PTSTATUS CopyAdaptersClient(
__in    IGNIS_ADAPTER*          OldAdapters,
__in    DWORD                   NoOfAdapters,
__out   PIGNIS_ADAPTER*         NewAdapters
)
{
    PTSTATUS status;
    DWORD i;
    IGNIS_ADAPTER* pAdapters;

    if (NULL == OldAdapters)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (0 == NoOfAdapters)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    if (NULL == NewAdapters)
    {
        return STATUS_INVALID_PARAMETER_3;
    }

    status = STATUS_SUCCESS;
    pAdapters = NULL;

    __try
    {
		// Allocate the memory for the new adapters
        pAdapters = (IGNIS_ADAPTER*)malloc(NoOfAdapters*sizeof(IGNIS_ADAPTER));
        if (NULL == pAdapters)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        for (i = 0; i < NoOfAdapters; ++i)
        {
            status = DeepCopyAdapter(&(OldAdapters[i]), &(pAdapters[i]));
            if ( !PT_SUCCESS( status ) )
            {
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: DeepCopyAdapter failed with status: 0x%X", status);
                __leave;
            }
        }
    }
    __finally
    {
        if ( !PT_SUCCESS( status ))
        {
            if (NULL != pAdapters)
            {
                free(pAdapters);
                pAdapters = NULL;
            }
        }
        else
        {
            *NewAdapters = pAdapters;
        }
    }

    return status;
}

static PTSTATUS DeepCopyAdapter(
__in        IGNIS_ADAPTER*      OldAdapter,
__out       IGNIS_ADAPTER*      NewAdapter
)
{
    BOOLEAN failed;
    size_t adapterNameLength;

    if( NULL == OldAdapter )
    {
        return STATUS_INVALID_PARAMETER_1;
    }
    
    if( NULL == NewAdapter )
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    failed = FALSE;
    adapterNameLength = 0;
    memcpy(NewAdapter, OldAdapter, sizeof(IGNIS_ADAPTER));

	// Make NULL every dynamically stored structure
    NewAdapter->AdapterName = NULL;
    NewAdapter->DhcpServers = NULL;
    NewAdapter->DnsServers = NULL;
    NewAdapter->Gateways = NULL;
    NewAdapter->LocalIpAddresses = NULL;

	// Allocate the memory for the new adapter's dynamically stored structures and copy the information from the old adapter 
    __try
    {
		// AdapterName
        if (NULL != OldAdapter->AdapterName)
        {
            adapterNameLength = OldAdapter->AdapterName->Length;

            NewAdapter->AdapterName = (IGNIS_STRING*)malloc((DWORD)(sizeof(IGNIS_STRING) + adapterNameLength - sizeof(WCHAR)));
            if (NULL == NewAdapter->AdapterName)
            {
                failed = TRUE;
                __leave;
            }

            wcscpy_s(NewAdapter->AdapterName->Buffer, adapterNameLength / sizeof(WCHAR), OldAdapter->AdapterName->Buffer);
            NewAdapter->AdapterName->Length = OldAdapter->AdapterName->Length;
        }

		// LocalIpAddresses
        if (OldAdapter->NoOfLocalIpAddresses > 0)
        {
            NewAdapter->LocalIpAddresses = (NET_INFO*)malloc(sizeof(NET_INFO)*OldAdapter->NoOfLocalIpAddresses);
            if (NULL == NewAdapter->LocalIpAddresses)
            {
                failed = TRUE;
                __leave;
            }

            memcpy(NewAdapter->LocalIpAddresses, OldAdapter->LocalIpAddresses, sizeof(NET_INFO)*OldAdapter->NoOfLocalIpAddresses);
        }

		// Gateways
        if (OldAdapter->NoOfGateways > 0)
        {
            NewAdapter->Gateways = (NET_INFO*)malloc(sizeof(NET_INFO)*OldAdapter->NoOfGateways);
            if (NULL == NewAdapter->Gateways)
            {
                failed = TRUE;
                __leave;
            }

            memcpy(NewAdapter->Gateways, OldAdapter->Gateways, sizeof(NET_INFO)*OldAdapter->NoOfGateways);
        }

		// DnsServers
        if (OldAdapter->NoOfDnsServers > 0)
        {
            NewAdapter->DnsServers = (NET_INFO*)malloc(sizeof(NET_INFO)*OldAdapter->NoOfDnsServers);
            if (NULL == NewAdapter->DnsServers)
            {
                failed = TRUE;
                __leave;
            }

            memcpy(NewAdapter->DnsServers, OldAdapter->DnsServers, sizeof(NET_INFO)*OldAdapter->NoOfDnsServers);
        }

		// DhcpServers
        if (OldAdapter->NoOfDhcpServers > 0)
        {
            NewAdapter->DhcpServers = (NET_INFO*)malloc(sizeof(NET_INFO)*OldAdapter->NoOfDhcpServers);
            if (NULL == NewAdapter->DhcpServers)
            {
                failed = TRUE;
                __leave;
            }

            memcpy(NewAdapter->DhcpServers, OldAdapter->DhcpServers, sizeof(NET_INFO)*OldAdapter->NoOfDhcpServers);
        }
    }
    __finally
    {
        if (failed)
        {
			// If any of the memory allocations failed free the memory
            FreeAdapterInfo(NewAdapter);
        }

    }

    return !failed;
}

PTSTATUS CleanupAdaptersInfoClient(
__in        IGNIS_ADAPTER*      Adapters,
__in        DWORD               NoOfAdapters
)
{
    PTSTATUS status;
    DWORD i;

    status = STATUS_SUCCESS;

    if (NULL == Adapters)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (0 == NoOfAdapters)
    {
        return STATUS_INVALID_PARAMETER_2;
    }
	// Free the memory for every adapter in the list
    for (i = 0; i < NoOfAdapters; ++i)
    {
        status = FreeAdapterInfo(&(Adapters[i]));
        if( !PT_SUCCESS( status ) )
        {
            return status;
        }
    }

    free(Adapters);

    return status;
}

static PTSTATUS FreeAdapterInfo(
__in        IGNIS_ADAPTER*      Adapter
)
{
    if (NULL == Adapter)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (NULL != Adapter->AdapterName)
    {
        free(Adapter->AdapterName);
        Adapter->AdapterName = NULL;
    }

    if (NULL != Adapter->LocalIpAddresses)
    {
        free(Adapter->LocalIpAddresses);
        Adapter->LocalIpAddresses = NULL;
    }

    if (NULL != Adapter->Gateways)
    {
        free(Adapter->Gateways);
        Adapter->Gateways = NULL;
    }

    if (NULL != Adapter->DnsServers)
    {
        free(Adapter->DnsServers);
        Adapter->DnsServers = NULL;
    }

    if (NULL != Adapter->DhcpServers)
    {
        free(Adapter->DhcpServers);
        Adapter->DhcpServers = NULL;
    }

    return STATUS_SUCCESS;
}