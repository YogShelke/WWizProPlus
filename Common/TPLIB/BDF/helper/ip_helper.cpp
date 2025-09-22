#include "stdafx.h"

#include "ip_helper.h"
#include "WardWizFirewall.h"

void DisplayNetInfo(
__in    NET_INFO*        Ip
)
{
    if (NULL == Ip)
    {
        return;
    }

    if (ipVersion4 == Ip->Version)
    {
		// ipVersion4
		CHAR msg[1024];
		sprintf_s(msg, 1024, "\t\tIPv4 Address : %d.%d.%d.%d\n",
            Ip->Ip.Ipv4.Ipv4ValueBytes[3],
            Ip->Ip.Ipv4.Ipv4ValueBytes[2],
            Ip->Ip.Ipv4.Ipv4ValueBytes[1],
            Ip->Ip.Ipv4.Ipv4ValueBytes[0]
            );
		//OutputDebugStringA(msg);
    }
    else
    {
		CHAR msg[1024];

        // ipVersion6
		sprintf_s(msg, 1024, "\t\tIPv6 Address : %X%X:%X%X:%X%X:%X%X:%X%X:%X%X:%X%X:%X%X\n",
            Ip->Ip.Ipv6.Ipv6Value[0],
            Ip->Ip.Ipv6.Ipv6Value[1],
            Ip->Ip.Ipv6.Ipv6Value[2],
            Ip->Ip.Ipv6.Ipv6Value[3],
            Ip->Ip.Ipv6.Ipv6Value[4],
            Ip->Ip.Ipv6.Ipv6Value[5],
            Ip->Ip.Ipv6.Ipv6Value[6],
            Ip->Ip.Ipv6.Ipv6Value[7],
            Ip->Ip.Ipv6.Ipv6Value[8],
            Ip->Ip.Ipv6.Ipv6Value[9],
            Ip->Ip.Ipv6.Ipv6Value[10],
            Ip->Ip.Ipv6.Ipv6Value[11],
            Ip->Ip.Ipv6.Ipv6Value[12],
            Ip->Ip.Ipv6.Ipv6Value[13],
            Ip->Ip.Ipv6.Ipv6Value[14],
            Ip->Ip.Ipv6.Ipv6Value[15]
            );
		//OutputDebugStringA(msg);
    }
	if(Ip->PhysAddrLen)
	{
		CHAR msg[1024];
		sprintf_s(msg, 1024, "\t\tPhysical Address : % 02X - % 02X - % 02X - % 02X - % 02X - % 02X\n",
			Ip->MacAddr[0],
			Ip->MacAddr[1],
			Ip->MacAddr[2],
			Ip->MacAddr[3],
			Ip->MacAddr[4],
			Ip->MacAddr[5],
			Ip->MacAddr[6]
			);
		//OutputDebugStringA(msg);
	}
}

PTSTATUS ConvertStringToGUID(
	__in CHAR*  GuidString,
	__out GUID* Guid
	)
{
	QWORD guidElements[5] = { 0 };
	DWORD i;

	if (NULL == GuidString)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	if (NULL == Guid)
	{
		return STATUS_INVALID_PARAMETER_2;
	}

	// Convert the string to an array of unsigned int64
	if (5 != sscanf_s(GuidString, "{%I64x-%I64x-%I64x-%I64x-%I64x}",
		&(guidElements[0]),
		&(guidElements[1]),
		&(guidElements[2]),
		&(guidElements[3]),
		&(guidElements[4])))
	{
		return STATUS_ITEM_DOES_NOT_FIT;
	}

	// Copy the GUID data from the array
	Guid->Data1 = (DWORD)guidElements[0];
	Guid->Data2 = (WORD)guidElements[1];
	Guid->Data3 = (WORD)guidElements[2];

	Guid->Data4[0] = *((BYTE*)&(guidElements[3]) + 1);
	Guid->Data4[1] = *((BYTE*)&(guidElements[3]));

	for (i = 0; i < 6; ++i)
	{
		Guid->Data4[2 + i] = *((BYTE*)&(guidElements[4]) + (5 - i));
	}

	return STATUS_SUCCESS;
}

PTSTATUS ConvertGUIDToString(
	__in GUID*    Guid,
	__out CHAR*  GuidString
	)
{
	QWORD guidElements[5];
	DWORD i;

	if (NULL == GuidString)
	{
		return STATUS_INVALID_PARAMETER_2;
	}

	RtlSecureZeroMemory(guidElements, sizeof(QWORD) * 5);

	// Populate an array of unsigned int64 with the GUID date 
	guidElements[0] = Guid->Data1;
	guidElements[1] = Guid->Data2;
	guidElements[2] = Guid->Data3;

	*((BYTE*)&(guidElements[3]) + 1) = Guid->Data4[0];
	*((BYTE*)&(guidElements[3])) = Guid->Data4[1];

	for (i = 0; i < 6; ++i)
	{
		*((BYTE*)&(guidElements[4]) + (5 - i)) = Guid->Data4[2 + i];
	}

	// Convert the array of unsigned int64 to a string
	if (-1 == sprintf(GuidString, "{%08x-%04x-%04x-%04x-%012I64x}", (DWORD)(guidElements[0]), (WORD)(guidElements[1]), (WORD)(guidElements[2]), (WORD)(guidElements[3]), (guidElements[4])))
	{
		return STATUS_ITEM_DOES_NOT_FIT;
	}

	return STATUS_SUCCESS;
}

void VisuallyDisplayNetwork(
__in        IGNIS_ADAPTER*      Adapters,
__in        DWORD               NoOfAdapters
)
{
	theApp.VisuallyDisplayNetwork(Adapters, NoOfAdapters);
}

PTSTATUS GetIPV4(
	__in    NET_INFO*        Ip,
	__out   CHAR(*ResultIp)[128]
)
{
	__try{
		sprintf_s(*ResultIp, 128, "%d.%d.%d.%d", Ip->Ip.Ipv4.Ipv4ValueBytes[3], Ip->Ip.Ipv4.Ipv4ValueBytes[2], Ip->Ip.Ipv4.Ipv4ValueBytes[1], Ip->Ip.Ipv4.Ipv4ValueBytes[0]);
	}__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetIPV4", 0, 0, true, SECONDLEVEL);
		return STATUS_WIN32_ERROR;
	}
	return STATUS_SUCCESS;
}

PTSTATUS GetIPV6(
	__in    NET_INFO*        Ip,
	__out   CHAR(*ResultIp)[128]
)
{
	__try{
	sprintf_s(*ResultIp, 128, "%X%X:%X%X:%X%X:%X%X:%X%X:%X%X:%X%X:%X%X", Ip->Ip.Ipv6.Ipv6Value[0], Ip->Ip.Ipv6.Ipv6Value[1], Ip->Ip.Ipv6.Ipv6Value[2], Ip->Ip.Ipv6.Ipv6Value[3], Ip->Ip.Ipv6.Ipv6Value[4], Ip->Ip.Ipv6.Ipv6Value[5], Ip->Ip.Ipv6.Ipv6Value[6], Ip->Ip.Ipv6.Ipv6Value[7], Ip->Ip.Ipv6.Ipv6Value[8], Ip->Ip.Ipv6.Ipv6Value[9], Ip->Ip.Ipv6.Ipv6Value[10], Ip->Ip.Ipv6.Ipv6Value[11], Ip->Ip.Ipv6.Ipv6Value[12], Ip->Ip.Ipv6.Ipv6Value[13], Ip->Ip.Ipv6.Ipv6Value[14], Ip->Ip.Ipv6.Ipv6Value[15]);
	}__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetIPV6", 0, 0, true, SECONDLEVEL);
		return STATUS_WIN32_ERROR;
	}
	return STATUS_SUCCESS;
}

PTSTATUS GetPhysAddr(
	__in    NET_INFO*        Ip,
	__out   CHAR(*ResultIp)[128]
)
{
	__try{
	sprintf_s(*ResultIp, 128, "% 02X - % 02X - % 02X - % 02X - % 02X - % 02X", Ip->MacAddr[0], Ip->MacAddr[1], Ip->MacAddr[2], Ip->MacAddr[3], Ip->MacAddr[4], Ip->MacAddr[5], Ip->MacAddr[6]);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetPhysAddr", 0, 0, true, SECONDLEVEL);
		return STATUS_WIN32_ERROR;
	}
	return STATUS_SUCCESS;
}

PTSTATUS IgnisIpToCharArray(
__in    IGNIS_IP    Ip,
__in    IP_VERSION  Version,
__out   CHAR        (*ResultIp)[128]
)
{
    if (NULL == ResultIp)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (ipVersion4 == Version)
    {
        sprintf_s(*ResultIp, 128, "%d.%d.%d.%d",
            Ip.Ipv4.Ipv4ValueBytes[3],
            Ip.Ipv4.Ipv4ValueBytes[2],
            Ip.Ipv4.Ipv4ValueBytes[1],
            Ip.Ipv4.Ipv4ValueBytes[0]);
    }
    else
    {
        sprintf_s(*ResultIp, 128, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
            Ip.Ipv6.Ipv6Value[0],
            Ip.Ipv6.Ipv6Value[1],
            Ip.Ipv6.Ipv6Value[2],
            Ip.Ipv6.Ipv6Value[3],
            Ip.Ipv6.Ipv6Value[4],
            Ip.Ipv6.Ipv6Value[5],
            Ip.Ipv6.Ipv6Value[6],
            Ip.Ipv6.Ipv6Value[7],
            Ip.Ipv6.Ipv6Value[8],
            Ip.Ipv6.Ipv6Value[9],
            Ip.Ipv6.Ipv6Value[10],
            Ip.Ipv6.Ipv6Value[11],
            Ip.Ipv6.Ipv6Value[12],
            Ip.Ipv6.Ipv6Value[13],
            Ip.Ipv6.Ipv6Value[14],
            Ip.Ipv6.Ipv6Value[15]
            );

    }

    return STATUS_SUCCESS;
}

void MD5ValueToCharArray(
__in    BYTE*    MD5Value,
__out   CHAR			(*text)[256]
)
{
	sprintf_s(*text, 256, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		MD5Value[0],
		MD5Value[1],
		MD5Value[2],
		MD5Value[3],
		MD5Value[4],
		MD5Value[5],
		MD5Value[6],
		MD5Value[7],
		MD5Value[8],
		MD5Value[9],
		MD5Value[10],
		MD5Value[11],
		MD5Value[12],
		MD5Value[13],
		MD5Value[14],
		MD5Value[15]
		);
}
