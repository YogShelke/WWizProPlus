#pragma once

#ifndef _LIBAPIS_H_
#define _LIBAPIS_H_

#ifdef __cplusplus
extern "C"
{
#endif
#ifndef KERNEL_MODE
#include <Windows.h>
#endif // !KERNEL_MODE


#ifdef __cplusplus
}
#endif

#include "structs_public.h"



#ifdef DLL_EXPORTS
#define IGNIS_DLL_API __declspec(dllexport)
#else
#define IGNIS_DLL_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"{
#endif

// IgnisSetOpt

/*
Parameters  :   Opt - the option whose value to be set
                Flags - Additional option flags
                Value - the value to be set for the specified option
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisSetOpt)(
    __in DWORD Opt,
    __in DWORD Flags,
    __in QWORD Value
);


IGNIS_DLL_API   FUNC_IgnisSetOpt    IgnisSetOpt;

typedef         FUNC_IgnisSetOpt*   PFUNC_IgnisSetOpt;


/*
Description :   The function retrieves the value of an option. Only fixed size option values can be retrieved by this function. 

Parameters  :   Opt - the option whose value to be retrieved
                Flags - pointer to a variable that will receive the option’s flags
                Value - pointer to a variable that will receive the option’s value
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisGetOpt)(
    __in DWORD Opt,
    __out_opt DWORD *Flags,
    __out QWORD *Value
    );

IGNIS_DLL_API   FUNC_IgnisGetOpt    IgnisGetOpt;

typedef         FUNC_IgnisGetOpt*   PFUNC_IgnisGetOpt;


/*
Description : Deletes an existing rule.

Parameters  :   RuleId -    value returned in the IGNIS_RULE structure when
                            adding a new rule
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisDeleteRule)(
    __in QWORD RuleId
    );

IGNIS_DLL_API   FUNC_IgnisDeleteRule    IgnisDeleteRule;

typedef         FUNC_IgnisDeleteRule*   PFUNC_IgnisDeleteRule;

/*
Description :   Deletes all rules added by the client.
                NOTE: if a lockdown is in place or the firewall is disabled
                those rules will remain set.

Parameters  :   void
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisDeleteAllRules)(void);

IGNIS_DLL_API   FUNC_IgnisDeleteAllRules    IgnisDeleteAllRules;

typedef         FUNC_IgnisDeleteAllRules*   PFUNC_IgnisDeleteAllRules;

typedef
PTSTATUS
(__cdecl FUNC_IgnisCreateRuleFromTrafficInfo)(
    __in PTRAFFIC_INFO TrafficInfo,
    __in RULE_ACTION   ActionForRule,
    __in QWORD         Flags,
    __out QWORD*       RuleId          
);

IGNIS_DLL_API   FUNC_IgnisCreateRuleFromTrafficInfo     IgnisCreateRuleFromTrafficInfo;

typedef         FUNC_IgnisCreateRuleFromTrafficInfo*    PFUNC_IgnisCreateRuleFromTrafficInfo;

/*
Description :   Adds a new firewall rule.

Parameters  :   NewRule - rule to be added, the fields are described in structs_public.h in
                          the IGNIS_RULE structure.
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisAddRule)(
    __inout PIGNIS_RULE NewRule
);

IGNIS_DLL_API   FUNC_IgnisAddRule   IgnisAddRule;

typedef         FUNC_IgnisAddRule*  PFUNC_IgnisAddRule;

typedef
PTSTATUS
(__cdecl FUNC_IgnisReplaceRule)(
    __in    QWORD          OldRuleId,
    __inout PIGNIS_RULE    NewRule
);

IGNIS_DLL_API   FUNC_IgnisReplaceRule   IgnisReplaceRule;

typedef         FUNC_IgnisReplaceRule*  PFUNC_IgnisReplaceRule;


/*
Description :   The function sets a file handle where the SDK will log all information, errors and warnings

Parameters  :   LogFile - A handle to a file with write rights
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisSetLogFile)(
    __in HANDLE LogFile
    );

IGNIS_DLL_API   FUNC_IgnisSetLogFile    IgnisSetLogFile;

typedef         FUNC_IgnisSetLogFile*   PFUNC_IgnisSetLogFile;


/*
Description :   The function sets a file handle where the SDK will log all traffic events

Parameters  :   LogFile - A handle to a file with write rights
*/
typedef
VOID
(__cdecl FUNC_IgnisSetTrafficLogFile)(
    __in HANDLE LogFile
    );

IGNIS_DLL_API   FUNC_IgnisSetTrafficLogFile     IgnisSetTrafficLogFile;

typedef         FUNC_IgnisSetTrafficLogFile*    PFUNC_IgnisSetTrafficLogFile;

/*
Description :   Retrieves the profile of an adapter.

Parameters  :   AdapterID - a pointer to the GUID of a valid network adapter
                Profile - a SINGLE set bit identifying the profile of the adapter

*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisGetProfileForAdapter)(
    __in    GUID*   AdapterID,
    __out   QWORD*  Profile
);

IGNIS_DLL_API   FUNC_IgnisGetProfileForAdapter  IgnisGetProfileForAdapter;

typedef         FUNC_IgnisGetProfileForAdapter* PFUNC_IgnisGetProfileForAdapter;

/*
Description :   Modifies the profile of a network adapter. Depending on the new profile
                set, some of the old rules may be removed and some new ones may be added.

Parameters  :   AdapterID - a pointer to the GUID of a valid network adapter
                Profile - a SINGLE set bit identifying the new profile of the adapter

*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisSetProfileForAdapter)(
    __in    GUID*   AdapterID,
    __in    QWORD   Profile
);

IGNIS_DLL_API   FUNC_IgnisSetProfileForAdapter  IgnisSetProfileForAdapter;

typedef         FUNC_IgnisSetProfileForAdapter* PFUNC_IgnisSetProfileForAdapter;

/*
Description :   Enables/disables portscan feature on a network adapter.

Parameters  :   AdapterID - a pointer to the GUID of a valid network adapter
                Enable - a boolean used to enable or disable portscanning feature on this adapter
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisSetPortscanForAdapter)(
    __in    GUID*       AdapterID,
    __in    BOOLEAN     Enable
    );

IGNIS_DLL_API   FUNC_IgnisSetPortscanForAdapter     IgnisSetPortscanForAdapter;

typedef         FUNC_IgnisSetPortscanForAdapter*    PFUNC_IgnisSetPortscanForAdapter;

/*
    Description : Enables/disables stealth feature for a network adapter
    
    Parameters  : AdapterID - pointer to the GUID that identifies the network adapter
                  Enable    - TRUE - enable stealth feature,
                              FALSE - disable stealth feature.
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisSetStealthForAdapter)(
__in    GUID*       AdapterID,
__in    BOOLEAN     Enable
);

IGNIS_DLL_API   FUNC_IgnisSetStealthForAdapter  IgnisSetStealthForAdapter;

typedef         FUNC_IgnisSetStealthForAdapter* PFUNC_IgnisSetStealthForAdapter;


/*
    Description : Check the state of the stealth feature on a network adapter
    
    Parameters  : AdapterID - pointer to the GUID that identifies the network adapter
                  Enable    - pointer to a boolean which value is TRUE if stealth is enabled,
                                                                  FALSE if stealth is disabled.
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisGetStealthForAdapter)(
__in    GUID*       AdapterID,
__in    BOOLEAN*    Enable
);

IGNIS_DLL_API   FUNC_IgnisGetStealthForAdapter  IgnisGetStealthForAdapter;

typedef         FUNC_IgnisGetStealthForAdapter* PFUNC_IgnisGetStealthForAdapter;

/*
Description : Retrieves the current settings of the firewall.

Parameters  : Settings - current settings
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisGetCurrentSettings)(
    __out       IGNIS_SETTINGS*   Settings
);

IGNIS_DLL_API   FUNC_IgnisGetCurrentSettings    IgnisGetCurrentSettings;

typedef         FUNC_IgnisGetCurrentSettings*   PFUNC_IgnisGetCurrentSettings;

/*
Description : Changes the current settings of the firewall.

Parameters  : Settings - new settings to be applied

*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisUpdateSettings)(
    __in        IGNIS_SETTINGS*   Settings
);

IGNIS_DLL_API   FUNC_IgnisUpdateSettings    IgnisUpdateSettings;

typedef         FUNC_IgnisUpdateSettings*   PFUNC_IgnisUpdateSettings;

/*
Description : Enables/disables blocking ICMP from outside the local network

Parameters  : AdapterID - pointer to the GUID that identifies the network adapter
Enable    - TRUE - enable feature,
FALSE     - disable stealth feature.
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisBlockExternalIcmpForAdapter)(
    _In_    GUID*       AdapterID,
    _In_    BOOLEAN     Enable
    );

IGNIS_DLL_API   FUNC_IgnisBlockExternalIcmpForAdapter  IgnisBlockExternalIcmpForAdapter;

typedef         FUNC_IgnisBlockExternalIcmpForAdapter* PFUNC_IgnisBlockExternalIcmpForAdapter;


/*
Description : Check the state of the block external ICMP feature on a network adapter

Parameters  : AdapterID - pointer to the GUID that identifies the network adapter
Enable    - pointer to a boolean which value is TRUE if the feature is enabled,
FALSE if ICMP from outside the local network are permitted.
*/
typedef
PTSTATUS
(__cdecl FUNC_IgnisGetBlockExternalIcmpForAdapter)(
    _In_     GUID*       AdapterID,
    _Inout_  BOOLEAN*    Enabled
    );

IGNIS_DLL_API   FUNC_IgnisGetBlockExternalIcmpForAdapter  IgnisGetBlockExternalIcmpForAdapter;

typedef         FUNC_IgnisGetBlockExternalIcmpForAdapter* PFUNC_IgnisGetBlockExternalIcmpForAdapter;

#ifdef __cplusplus
}
#endif

#endif // _LIBAPIS_H_