#pragma once

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

/** 
	File: callbacks.h
	Description: Contains the definitions of the callback functions 
*/


/**
	Callback function for the unmatched traffic
*/
VOID OnTrafficUnmatchedNotification(
    __in PTRAFFIC_INFO TrafficInfo,
    __inout RULE_ACTION *Action,
    __in PVOID Context
);

/**
	Callback function for rule action notification
*/
VOID OnRuleActionNotification(
    __in PTRAFFIC_INFO TrafficInfo,
    __in RULE_ACTION Action,
    __in QWORD RuleId,
    __in PVOID Context
);

/**
	Callback function for process md5 changed notification
*/
VOID OnProcessMd5ChangedNotification(
    __in PTRAFFIC_INFO TrafficInfo,
    __in PBYTE OldMd5,
    __inout RULE_ACTION *Action,
    __in PVOID Context
);

/**
	Callback function for address changed notification
*/
void OnAddressChangedNotification(
__in    IGNIS_ADAPTER*  Adapters,
__in    DWORD           NoOfAdapters,
__out   QWORD*          Profiles,
__in    PVOID           Context
);

/**
	Callback function for portscan alert notification
*/
VOID OnPortscanAlertNotification(
__in    IP_VERSION  Version,
__in    IGNIS_IP    *RemoteIp,
__in    DWORD       ScannedPortCount,
__in    PORTSCAN_PORTS_ENTRY *ScannedPortArray,
__in    QWORD       StartTimestamp,
__in    QWORD       EndTimestamp,
__in    PVOID       Context
);

/**
	Setup all ignis callbacks
*/
PTSTATUS SetupCallbacks();
bool InsertApplicationsIntoDB(std::string strAppPath);
bool CheckApplicationExists(std::string strAppPath);
bool InsertIntoFWReports(char *);

#endif //_CALLBACKS_H_