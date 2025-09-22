#include "stdafx.h"


#include "rule_helper.h"
#include <stdlib.h>
#include <stdio.h>
#include "imports.h"
#include "structs_public.h"



PTSTATUS CreateIgnisRuleFromCmd(
    __in    char**          CommandLine,
    __in    int             CommandArgs,
    __in    LONG*           ParameterIndex,
    __out   PIGNIS_RULE     Rule
)
{
    PTSTATUS status;
    LONG paramIndex;
    int argc;
    char** argv;
    PPROCESS_INFO processInfo = NULL;
    DWORD remPort;
    DWORD category;
    TRAFFIC_DIRECTION direction = DIR_BOTH;
    RULE_ACTION action = ACTION_ALLOW;
    size_t i;
    PCHAR path= NULL;
    QWORD profile;
    DWORD protocol = 0;

    if( NULL == CommandLine )
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    if( NULL == ParameterIndex )
    {
        return STATUS_INVALID_PARAMETER_3;
    }

    if( NULL == Rule )
    {
        return STATUS_INVALID_PARAMETER_4;
    }

    status = STATUS_SUCCESS;
    paramIndex = *ParameterIndex;
    argc = CommandArgs;
    argv = CommandLine;
    category = RULE_CATEGORY_USER;

    __try
    {

        while (paramIndex < argc - 1)
        {
            ++paramIndex;
            printf("argv[%d]: %s \n", paramIndex, argv[paramIndex]);
			// Process Name
            if (!_stricmp(argv[paramIndex], "proc"))
            {
                path = argv[++paramIndex];
                if (paramIndex >= argc )
                {
                    status = STATUS_INVALID_PARAMETER;
                    __leave;
                }

				CHAR msg[1024] = { 0 };
				sprintf_s(msg, 1024, "[success]: read process name %s \n", path);
				//OutputDebugStringA(msg);

                processInfo = (PPROCESS_INFO)malloc(sizeof(PROCESS_INFO) + (strlen(path) + 1) * sizeof(WCHAR));
                if (NULL == processInfo)
                {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    __leave;
                }
                memset((void*)processInfo, 0, sizeof(PROCESS_INFO) + (strlen(path) + 1) * sizeof(WCHAR));
                for (i = 0; i < strlen(path); i++)
                {
                    processInfo->ProcessPath.Buffer[i] = (WCHAR)path[i];
                }

                processInfo->ProcessPath.Buffer[i] = L'\0';
                processInfo->ProcessPath.Length = (DWORD)((wcslen(processInfo->ProcessPath.Buffer) + 1) * sizeof(WCHAR));

                Rule->Flags |= RULE_CHECK_PROCESS;
                Rule->Process = processInfo;
                continue;
            }
			// Action
            else if (!_strnicmp(argv[paramIndex], "block", strlen("block")))
            {
                action = ACTION_DENY;
                continue;
            }

            else if (!_strnicmp(argv[paramIndex], "allow", strlen("allow")))
            {
                action = ACTION_ALLOW;
                continue;
            }
			// Direction
            else  if (!_strnicmp(argv[paramIndex], "inbound", strlen("inbound")))
            {
                direction = DIR_INBOUND;
                continue;
            }

            else if (!_strnicmp(argv[paramIndex], "outbound", strlen("outbound")))
            {
                direction = DIR_OUTBOUND;
                continue;
            }
			// Notify
            else if (!_strnicmp(argv[paramIndex], "notify", strlen("notify")))
            {
                Rule->Flags |= RULE_NOTIFY_ON_ACTION;
                continue;
            }
			// MD5
            else if (!_strnicmp(argv[paramIndex], "md5", strlen("md5")))
            {
                Rule->Flags |= RULE_CHECK_MD5;
                continue;
            }
			// Protocol
            else if (!_strnicmp(argv[paramIndex], "protocol=", strlen("protocol=")))
            {
                if (sscanf_s(argv[paramIndex], "protocol=%d", &protocol) == 1)
                {
                    Rule->Flags |= RULE_CHECK_PROTOCOL;
                    Rule->Protocol = (BYTE)protocol;
                    continue;
                }
                else
                {
                    status = STATUS_INVALID_PARAMETER;
                    __leave;
                }
            }
			// Remote Port
            else if (!_strnicmp(argv[paramIndex], "remport=", strlen("remport=")))
            {
                if (sscanf_s(argv[paramIndex], "remport=%d", &remPort) == 1)
                {
                    Rule->Flags |= RULE_CHECK_REM_PORT;
                    Rule->RemotePortRanges = (PPORT_RANGE)malloc(sizeof(PORT_RANGE));
                    if (NULL == Rule->RemotePortRanges)
                    {
						AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: can't allocate memory for remote port");
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        __leave;
                    }
                    Rule->RemotePortRangesCount = 1;
                    Rule->RemotePortRanges[0].Min = Rule->RemotePortRanges[0].Max = (WORD)remPort;
                    continue;
                }
                else
                {
					AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: can't read remote port");
                    status = STATUS_INVALID_PARAMETER;
                    __leave;
                }
            }
			// Remote Ipv4
            else if (!_strnicmp(argv[paramIndex], "remip=", strlen("remip=")))
            {
                DWORD b3, b2, b1, b0;
                if (sscanf_s(argv[paramIndex], "remip=%d.%d.%d.%d", &b3, &b2, &b1, &b0) == 4)
                {
                    Rule->IpVersion = ipVersion4;
                    Rule->Flags |= RULE_CHECK_REM_NETWORK;
                    Rule->RemoteNetworkCount = 1;
                    Rule->RemoteNetworks = (PIGNIS_IP)malloc(sizeof(IGNIS_IP));
                    if (NULL == Rule->RemoteNetworks)
                    {
						AddLogEntryEx(SECONDLEVEL, L"### error allocating memory for remote network %s", path);
                    }
                    Rule->RemoteNetworks[0].Ipv4.Ipv4ValueBytes[3] = (BYTE)b3;
                    Rule->RemoteNetworks[0].Ipv4.Ipv4ValueBytes[2] = (BYTE)b2;
                    Rule->RemoteNetworks[0].Ipv4.Ipv4ValueBytes[1] = (BYTE)b1;
                    Rule->RemoteNetworks[0].Ipv4.Ipv4ValueBytes[0] = (BYTE)b0;
                    Rule->RemoteNetworks[0].Ipv4.Mask = 0xffffffff;
                    continue;
                }
                else
                {
                    status = STATUS_INVALID_PARAMETER;
                    __leave;
                }
            }
			// Remote Ipv6
            else if (!_strnicmp(argv[paramIndex], "remip6=", strlen("remip6=")))
            {
                DWORD locali, w7, w6, w5, w4, w3, w2, w1, w0;
                if (sscanf_s(argv[paramIndex], "remip6=%X:%X:%X:%X:%X:%X:%X:%X", &w7, &w6, &w5, &w4, &w3, &w2, &w1, &w0) == 8)
                {
                    Rule->IpVersion = ipVersion6;
                    Rule->Flags |= RULE_CHECK_REM_NETWORK;
                    Rule->RemoteNetworkCount = 1;
                    Rule->RemoteNetworks = (PIGNIS_IP)malloc(sizeof(IGNIS_IP));
                    if (NULL == Rule->RemoteNetworks)
                    {
						AddLogEntryEx(SECONDLEVEL, L"### error allocating memory for remote network %s", path);
                    }
                    *((PUINT16)&Rule->RemoteNetworks[0].Ipv6.Ipv6Value[0]) = (UINT16)w7;
                    *((PUINT16)&Rule->RemoteNetworks[0].Ipv6.Ipv6Value[2]) = (UINT16)w6;
                    *((PUINT16)&Rule->RemoteNetworks[0].Ipv6.Ipv6Value[4]) = (UINT16)w5;
                    *((PUINT16)&Rule->RemoteNetworks[0].Ipv6.Ipv6Value[6]) = (UINT16)w4;
                    *((PUINT16)&Rule->RemoteNetworks[0].Ipv6.Ipv6Value[8]) = (UINT16)w3;
                    *((PUINT16)&Rule->RemoteNetworks[0].Ipv6.Ipv6Value[10]) = (UINT16)w2;
                    *((PUINT16)&Rule->RemoteNetworks[0].Ipv6.Ipv6Value[12]) = (UINT16)w1;
                    *((PUINT16)&Rule->RemoteNetworks[0].Ipv6.Ipv6Value[14]) = (UINT16)w0;

                    for (locali = 0; locali < 8; locali++)
                    {
                        BYTE t = Rule->RemoteNetworks[0].Ipv6.Ipv6Value[2 * locali];
                        Rule->RemoteNetworks[0].Ipv6.Ipv6Value[2 * locali] = Rule->RemoteNetworks[0].Ipv6.Ipv6Value[2 * locali + 1];
                        Rule->RemoteNetworks[0].Ipv6.Ipv6Value[2 * locali + 1] = t;
                    }
                    Rule->RemoteNetworks[0].Ipv6.Prefix = 128;
                    continue;
                }
                else
                {
                    status = STATUS_INVALID_PARAMETER;
                    __leave;
                }
            }
			// Category
            else if (!_strnicmp(argv[paramIndex], "category=", strlen("category=")))
            {
                if (sscanf_s(argv[paramIndex], "category=%x", &category) == 1)
                {
                    continue;
                }
                else
                {
                    status = STATUS_INVALID_PARAMETER;
                    __leave;
                }
            }
			// ProfileBitmap
            else if (!_strnicmp(argv[paramIndex], "profile=", strlen("profile=")))
            {
                if (sscanf_s(argv[paramIndex], "profile=%I64x", &profile) == 1)
                {
                    Rule->ProfileBitmap = profile;
                    Rule->Flags |= RULE_CHECK_PROFILE;
                    continue;
                }
                else
                {
                    status = STATUS_INVALID_PARAMETER;
                    __leave;
                }
            }
			// Sid
            else if (!_strnicmp(argv[paramIndex], "sid=", strlen("sid=")))
            {
                WCHAR sid[100];
                DWORD len = 0;

                len = (DWORD)strlen(argv[paramIndex]) - 4;

                if (len >= sizeof(sid) / sizeof(WCHAR))
                {
                    status = STATUS_INVALID_PARAMETER;
                    __leave;
                }

                for (i = 0; i < len; i++)
                {
                   sid[i] = (WCHAR)argv[paramIndex][i + 4];
                }
                sid[len] = L'\0';

                Rule->Sid = (PIGNIS_STRING)malloc(sizeof(IGNIS_STRING) + len * sizeof(WCHAR));
                if (NULL == Rule->Sid)
                {
					AddLogEntryEx(SECONDLEVEL, L"### ERROR: malloc failed");
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    __leave;
                }

                Rule->Sid->Length = (len + 1)  * sizeof(WCHAR);
                memcpy((VOID *)&Rule->Sid->Buffer[0], sid, Rule->Sid->Length);
                Rule->Flags = Rule->Flags | RULE_CHECK_SID;
                continue;
            }
            else
            {
				AddLogEntryEx(SECONDLEVEL, L"### WARNING: Invalid argument provided : %s", argv[paramIndex]);
                continue;
            }

        }
		// Complete the rest of the fields for the rule
        if (Rule->IpVersion != ipVersion4 && Rule->IpVersion != ipVersion6)
        {
            Rule->IpVersion = ipVersionAny;
        }

        if( Rule->Flags & RULE_CHECK_MD5 )
        {
            DWORD md5Size = SIZEOF_MD5;

            // we need to actually calculate the MD5 value
            status = PubIgnisComputeMD5(&Rule->Process->ProcessPath,
                                        Rule->Process->Md5Value,
                                        &md5Size);
            if( !PT_SUCCESS(status))
            {
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: PubIgnisComputeMD5 failed with status: 0x%X", status);
                __leave;
            }
            Rule->Process->Md5Valid = TRUE;
        }

        Rule->Action = action;
        Rule->Direction = direction;

        // if the user didn't specify the category => RULE_CATEGORY_USER will be automatically set
        Rule->Category = (BYTE)category;

    }
    __finally
    {
        PTSTATUS statusSup;

        if (!PT_SUCCESS(status))
        {
            statusSup = FreeClientRule(Rule);
            if (!PT_SUCCESS(statusSup))
            {
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: FreeClientRule failed with status: 0x%X", statusSup);
            }
        }

        *ParameterIndex = paramIndex;
    }

    return status;
}

PTSTATUS FreeClientRule(
    __in    PIGNIS_RULE     Rule
)
{
    if( NULL == Rule )
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    if (NULL != Rule->Process)
    {
        free((void*)Rule->Process);
        Rule->Process = NULL;
    }

    if (NULL != Rule->RemoteNetworks)
    {
        free((void*)Rule->RemoteNetworks);
        Rule->RemoteNetworks = NULL;
    }

    if (NULL != Rule->RemotePortRanges)
    {
        free((void*)Rule->RemotePortRanges);
        Rule->RemotePortRanges = NULL;
    }

    return STATUS_SUCCESS;
}