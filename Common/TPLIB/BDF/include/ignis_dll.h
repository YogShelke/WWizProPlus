#ifndef _IGNIS_DLL_H_
#define _IGNIS_DLL_H_

#ifndef QWORD
#define QWORD unsigned __int64
#endif

#include "libapis.h"
#include "structs_public.h"

#define LIBRARY_INTERFACE_VERSION             ((DWORD)13)


#ifdef __cplusplus
extern "C"{
#endif

#pragma pack(push)
#pragma pack(8)

    
/*
Description :   This is the main initialization routine in the dll component.
It must be the first routine called by the user mode client in order to setup the environment for the rest of the library routines.

Parameters  :   Initialization - A set of default settings used by the library
*/
typedef
PTSTATUS
(__cdecl FUNC_InitializeIgnis)(
__in   IGNIS_INIT*       Initialization
);

IGNIS_DLL_API   FUNC_InitializeIgnis    InitializeIgnis;

typedef         FUNC_InitializeIgnis*   PFUNC_InitializeIgnis;


/*
Description :   This routine cleans up any resources allocated and used by the dll component. 
                After a call to this routine is made, the client application should not call any other component routine except InitializeIgnis.

*/
typedef
PTSTATUS
(__cdecl FUNC_UninitializeIgnis)(
   void
    );

IGNIS_DLL_API   FUNC_UninitializeIgnis  UninitializeIgnis;

typedef         FUNC_UninitializeIgnis* PFUNC_UninitializeIgnis;

//callbacks definitions

/*
Description :   Procedure signature of the callback executed every time the firewall encounters unmatched traffic.
                This callback is set using option OPT_UNMATCHED_TRAFFIC_NOTIFICATION_CALLBACK

Parameters  :   TrafficInfo - A structure describing the unmatched traffic
                Action - The action returned by the integrator regarding current TrafficInfo (ACTION_ALLOW | ACTION_BLOCK)
                Context - Integrator defined context with OPT_UNMATCHED_TRAFFIC_NOTIFICATION_CALLBACK_CTX
*/
typedef
VOID
(__cdecl FUNC_IgnisOnTrafficUnmatchedCallback) (
    __in PTRAFFIC_INFO TrafficInfo,
    __inout RULE_ACTION *Action,
    __in PVOID Context
    );

typedef FUNC_IgnisOnTrafficUnmatchedCallback* PFUNC_IgnisOnTrafficUnmatchedCallback;

/*
Description :   Procedure signature of the callback executed every time the firewall finds a matching rule with notifications enabled.
                This callback is set using option OPT_RULE_ACTION_CALLBACK

Parameters  :   TrafficInfo - A structure describing the traffic matched by RuleId
                Action - The action taken by SKD regarding current TrafficInfo (ACTION_ALLOW | ACTION_BLOCK)
                Context - Integrator defined context with OPT_RULE_ACTION_CALLBACK_CTX
*/
typedef
VOID
(__cdecl FUNC_IgnisOnRuleActionCallback) (
    __in PTRAFFIC_INFO TrafficInfo,
    __in RULE_ACTION Action,
    __in QWORD RuleId,
    __in PVOID Context
    );

typedef FUNC_IgnisOnRuleActionCallback* PFUNC_IgnisOnRuleActionCallback;

/*
Description :   Procedure signature of the callback executed every time a new process (matching existing rules) 
                is trying to connect/accept connections but the MD5 doesn't match with the one set in current rules.
                This callback is set using option OPT_PROCESS_CHANGED_CALLBACK

Parameters  :   TrafficInfo - A structure describing the unmatched traffic
                OldMd5 - This is the original MD5 value stored in existing traffic rule. (the traffic matches but MD5 is changed)
                Action - The action returned by the integrator regarding current TrafficInfo (ACTION_ALLOW | ACTION_BLOCK)
                Context - Integrator defined context with OPT_PROCESS_CHANGED_CALLBACK_CTX
*/
typedef
VOID
(__cdecl FUNC_IgnisOnMd5ChangedCallback) (
    __in PTRAFFIC_INFO TrafficInfo,
    __in PBYTE OldMd5,
    __inout RULE_ACTION *Action,
    __in PVOID Context
    );

typedef FUNC_IgnisOnMd5ChangedCallback* PFUNC_IgnisOnMd5ChangedCallback;

/*
Description : Procedure signature of the callback executed every time a network
              adapter changes. The client will receive the list of the new adapters
              and will have to complete the profile for each of the adapter if the
              function finishes with success.
              This callback is set using option OPT_ADAPTER_CHANGED_CALLBACK

Parameters  : Adapters - array of active network adapters, NULL if NoOfAdapters is ZERO
              NoOfAdapters - number of adapters
              Profiles - array of profiles to be set on the adapters, NULL if NoOfAdapters is ZERO
              Context - set using the OPT_ADAPTER_CHANGED_CALLBACK_CTX option
*/
typedef
VOID
(__cdecl FUNC_IgnisOnAdaptersChangedCallback) (
    __in_opt    IGNIS_ADAPTER*  Adapters,
    __in        DWORD           NoOfAdapters,
    __out_opt   QWORD*          Profiles,
    __in        PVOID           Context
);

typedef FUNC_IgnisOnAdaptersChangedCallback* PFUNC_IgnisOnAdaptersChangedCallback;

/*
Description :   Procedure signature of the callback executed every time a portscan is detected.
                The client will receive informations about detected portscan.
                This callback is set using option OPT_PORTSCAN_ALERT_CALLBACK

Parameters  :   Version - IP version on which the portscan was detected
                RemoteIp - The attacker's IP address
                ScannedPortCount - number of ports scanned until the alert was triggered (with a limit of first 16 ports)
                ScannedPortsArray - An array of PORTSCAN_PORTS_ENTRY where each scanned port has details about the method of scanning
                StartTimestamp - Start time of current portscan in 100NS increments
                EndTimestamp - Alert time of current portscan in 100NS increments
                Context - set using the OPT_PORTSCAN_ALERT_CALLBACK_CTX option
*/
typedef
VOID
(__cdecl FUNC_IgnisOnPortscanAlertCallback) (
__in    IP_VERSION  Version,
__in    IGNIS_IP    *RemoteIp,
__in    DWORD       ScannedPortCount,
__in    PORTSCAN_PORTS_ENTRY *ScannedPortsArray,
__in    QWORD       StartTimestamp,
__in    QWORD       EndTimestamp,
__in    PVOID       Context
);

typedef FUNC_IgnisOnPortscanAlertCallback* PFUNC_IgnisOnPortscanAlertCallback;

typedef struct _IGNIS_CALLBACK_REGISTRATION
{
    BOOLEAN                                     ModifyIgnisOnTrafficUnmatchedCallback;
    PFUNC_IgnisOnTrafficUnmatchedCallback       IgnisOnTrafficUnmatchedCallback;
    PVOID                                       IgnisOnTrafficUnmatchedContext;

    BOOLEAN                                     ModifyIgnisOnRuleActionCallback;
    PFUNC_IgnisOnRuleActionCallback             IgnisOnRuleActionCallback;
    PVOID                                       IgnisOnRuleActionContext;

    BOOLEAN                                     ModifyIgnisOnMd5ChangedCallback;
    PFUNC_IgnisOnMd5ChangedCallback             IgnisOnMd5ChangedCallback;
    PVOID                                       IgnisOnMd5ChangedContext;

    BOOLEAN                                     ModifyIgnisOnAdaptersChangedCallback;
    PFUNC_IgnisOnAdaptersChangedCallback        IgnisOnAdaptersChangedCallback;
    PVOID                                       IgnisOnAdaptersChangedContext;

    BOOLEAN                                     ModifyIgnisOnPortscanAlertCallback;
    PFUNC_IgnisOnPortscanAlertCallback          IgnisOnPortscanAlertCallback;
    PVOID                                       IgnisOnPortscanAlertContext;
} IGNIS_CALLBACK_REGISTRATION, *PIGNIS_CALLBACK_REGISTRATION;

/*

*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisRegisterCallbacks)(
    __in        IGNIS_CALLBACK_REGISTRATION*    Callbacks
);

IGNIS_DLL_API   FUNC_IgnisRegisterCallbacks     IgnisRegisterCallbacks;

typedef         FUNC_IgnisRegisterCallbacks*    PFUNC_IgnisRegisterCallbacks;

typedef
PTSTATUS
(__cdecl FUNC_IgnisQueryCallbacks)(
    __out       IGNIS_CALLBACK_REGISTRATION*    Callbacks
);

IGNIS_DLL_API   FUNC_IgnisQueryCallbacks        IgnisQueryCallbacks;

typedef         FUNC_IgnisQueryCallbacks*       PFUNC_IgnisQueryCallbacks;


/*
    Description :   The procedure computes the MD5 hash of the first MB of an executable file, 
                    given its path.

    Parameters  :   Path            - Path of the executable file.
                    MD5Buffer       - Pointer to the memory location where the MD5 hash will be stored.
                    BufferLength    - Pointer to the length in bytes of the buffer. If the buffer is too
                                      small, the required buffer size is written in BufferLength.
*/

typedef
PTSTATUS
(__cdecl FUNC_IgnisComputeMD5)(
    __in        PIGNIS_STRING   Path,
    __inout     PBYTE           MD5Buffer,
    __inout     PDWORD          BufferLength
);

IGNIS_DLL_API   FUNC_IgnisComputeMD5        IgnisComputeMD5;

typedef         FUNC_IgnisComputeMD5*       PFUNC_IgnisComputeMD5;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif //_IGNIS_DLL_H_