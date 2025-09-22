#include "stdafx.h"
#include "WardWizFirewall.h"
#include "callbacks.h"
#include "options_public.h"
#include "resource.h"
#include "ip_helper.h"
#include "adapter_helper.h"
#include "imports.h"
#include <WinSock2.h>
#include "xml_helper.h"
#include "WardWizDatabaseInterface.h"

extern BOOLEAN gUnattendedMode;
extern BOOLEAN gDefaultActionInAllow;
extern BOOLEAN gDefaultActionOutAllow;

extern QWORD gDefaultInFlags;
extern QWORD gDefaultOutFlags;
extern QWORD gLogLevelForUnattendedMode;

extern CRITICAL_SECTION gXmlLock; 

std::vector<std::string>	g_vAppList;

FUNC_IgnisOnTrafficUnmatchedCallback    OnTrafficUnmatchedNotification;
FUNC_IgnisOnRuleActionCallback          OnRuleActionNotification;
FUNC_IgnisOnMd5ChangedCallback          OnProcessMd5ChangedNotification;
FUNC_IgnisOnAdaptersChangedCallback     OnAddressChangedNotification;
FUNC_IgnisOnPortscanAlertCallback       OnPortscanAlertNotification;

VOID OnTrafficUnmatchedNotification(
    __in PTRAFFIC_INFO TrafficInfo,
    __inout RULE_ACTION *Action,
    __in PVOID Context
    )
{
    CHAR ip[128];
    CHAR msg[1024];
    PTSTATUS status;
    QWORD ruleId;

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Action);
    UNREFERENCED_PARAMETER(TrafficInfo);

    // TRAFFIC_INFO can only have ipVersion4 or ipVersion6
    if (ipVersion4 == TrafficInfo->IpVersion)
    {
        sprintf_s(ip, 128, "%d.%d.%d.%d",
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[3],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[2],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[1],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[0]);
    }
    else
    {
        sprintf_s(ip, 128, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[0],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[1],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[2],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[3],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[4],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[5],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[6],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[7],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[8],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[9],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[10],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[11],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[12],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[13],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[14],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[15]
            );

    }

	// If the default flags are set by the auto commands then add a default rule using the default action and the default flags
    if ((0 != gDefaultInFlags && TrafficInfo->Direction == DIR_INBOUND) ||
        (0 != gDefaultOutFlags && TrafficInfo->Direction == DIR_OUTBOUND)
        )
    {
        sprintf_s(msg, 1024, "[Will add default rule] %s, %s %s port %d (Pid: %d, Path: %S)\n",
            (TrafficInfo->Direction == DIR_OUTBOUND) ? "OUTBOUND" : "INBOUND",
            (TrafficInfo->Direction == DIR_OUTBOUND) ? "to" : "from",
            ip,
            (TrafficInfo->Direction == DIR_OUTBOUND) ? TrafficInfo->RemotePort : TrafficInfo->LocalPort,
            (DWORD)TrafficInfo->Process.Pid,
            TrafficInfo->Process.ProcessPath.Buffer
            );
        //OutputDebugStringA(msg);

		BOOLEAN bDefaultActionAllow  = (TrafficInfo->Direction == DIR_INBOUND) ? gDefaultActionInAllow : gDefaultActionOutAllow;
		RULE_ACTION raDefaultAction = (bDefaultActionAllow == TRUE) ? ACTION_ALLOW : ACTION_DENY;
		QWORD bDefaultFlags = (TrafficInfo->Direction == DIR_INBOUND) ? gDefaultInFlags : gDefaultOutFlags;

		// Add a new rule from traffic info 
        status = PubIgnisCreateRuleFromTrafficInfo(TrafficInfo, raDefaultAction, bDefaultFlags, &ruleId);

        if (!PT_SUCCESS(status))
        {
			AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: IgnisCreateRuleFromTrafficInfo failed, status 0x%x", status);
        }
        else
        {
			AddLogEntryEx(ZEROLEVEL, L"### added new rule with id %I64d", ruleId);

			/*
			// Add a new rule from traffic info in the xml
			EnterCriticalSection(&gXmlLock);
			int iRet = InsertRuleToXmlFromTrafficInfo(TrafficInfo, raDefaultAction, bDefaultFlags, ruleId);
			if (iRet)
			{
				AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: Adding a new rule in xml failed");
			}
			LeaveCriticalSection(&gXmlLock);
			*/
        }
    }
    else if (TRUE == gUnattendedMode)
    {
		// If the unattended mode is on and the loglevel is different from 0 then print on the console the traffic info
        if (0 != gLogLevelForUnattendedMode)
        {
			CHAR msg[1024];
			sprintf_s(msg, 1024, "[UNATT] %s, %s %s port %d (Pid: %d, Path: %S). Action: %s\n",
                (TrafficInfo->Direction == DIR_OUTBOUND) ? "OUTBOUND" : "INBOUND",
                (TrafficInfo->Direction == DIR_OUTBOUND) ? "to" : "from",
                ip,
                (TrafficInfo->Direction == DIR_OUTBOUND) ? TrafficInfo->RemotePort : TrafficInfo->LocalPort,
                (DWORD)TrafficInfo->Process.Pid,
                TrafficInfo->Process.ProcessPath.Buffer,
                ((TrafficInfo->Direction == DIR_INBOUND) ? gDefaultActionInAllow : gDefaultActionOutAllow) ? "ALLOW" : "BLOCK"
                );
			//OutputDebugStringA(msg);
        }
		// Apply the default action for the unattended mode
        *Action = ((TrafficInfo->Direction == DIR_INBOUND) ? gDefaultActionInAllow : gDefaultActionOutAllow) ? ACTION_ALLOW : ACTION_DENY;
    }
    else
    {

		//CStringA csAlert;
		//csAlert.Format("[ALERT] %s, %s %s port %d (Pid: %d, Path: %S). Allow?\n",
		//	(TrafficInfo->Direction == DIR_OUTBOUND) ? "OUTBOUND" : "INBOUND",
		//	(TrafficInfo->Direction == DIR_OUTBOUND) ? "to" : "from",
		//	ip,
		//	(TrafficInfo->Direction == DIR_OUTBOUND) ? TrafficInfo->RemotePort : TrafficInfo->LocalPort,
		//	(DWORD)TrafficInfo->Process.Pid,
		//	TrafficInfo->Process.ProcessPath.Buffer
		//	);
		//OutputDebugStringA(csAlert);

		INT_PTR uiRet = UI_FLAG_ALLOW;
		//uiRet |= UI_FLAG_REMEMBER;
		uiRet |= UI_FLAG_NOTIFY;
		uiRet |= UI_FLAG_BY_IP;
		uiRet |= UI_FLAG_BY_PORT;
		uiRet |= UI_FLAG_BY_PROTO;
		uiRet |= UI_FLAG_BY_PATH;
		uiRet |= UI_FLAG_BY_MD5;
		uiRet |= UI_FLAG_BIDIRECTIONAL;

        QWORD uiFlags = 0;

		*Action = ((TrafficInfo->Direction == DIR_INBOUND) ? gDefaultActionInAllow : gDefaultActionOutAllow) ? ACTION_ALLOW : ACTION_DENY;

		if (*Action == ACTION_DENY)
		{
			CStringA csAlert;
			csAlert.Format("[BLOCKED] %s, %s %s port %d (Pid: %d, Path: %S). Allow?\n",
				(TrafficInfo->Direction == DIR_OUTBOUND) ? "OUTBOUND" : "INBOUND",
				(TrafficInfo->Direction == DIR_OUTBOUND) ? "to" : "from",
				ip,
				(TrafficInfo->Direction == DIR_OUTBOUND) ? TrafficInfo->RemotePort : TrafficInfo->LocalPort,
				(DWORD)TrafficInfo->Process.Pid,
				TrafficInfo->Process.ProcessPath.Buffer
				);
			//OutputDebugStringA(csAlert);
		}
		else
		{
			CStringA csAlert;
			csAlert.Format("[ALLOWED] %s, %s %s port %d (Pid: %d, Path: %S). Allow?\n",
				(TrafficInfo->Direction == DIR_OUTBOUND) ? "OUTBOUND" : "INBOUND",
				(TrafficInfo->Direction == DIR_OUTBOUND) ? "to" : "from",
				ip,
				(TrafficInfo->Direction == DIR_OUTBOUND) ? TrafficInfo->RemotePort : TrafficInfo->LocalPort,
				(DWORD)TrafficInfo->Process.Pid,
				TrafficInfo->Process.ProcessPath.Buffer
				);
			//OutputDebugStringA(csAlert);
		}

		CStringA csProcessPath;
		csProcessPath.Format("%S", TrafficInfo->Process.ProcessPath.Buffer);

		InsertApplicationsIntoDB(csProcessPath.GetBuffer());

        if (uiRet & UI_FLAG_REMEMBER)
        {
            uiFlags = 0;

            if (uiRet & UI_FLAG_BIDIRECTIONAL)
            {
                TrafficInfo->Direction = DIR_BOTH;
				uiFlags |= (uiRet & UI_FLAG_BY_IP) ? RULE_CHECK_LOCAL_NETWORK : 0;
				uiFlags |= (uiRet & UI_FLAG_BY_PORT) ? RULE_CHECK_LOCAL_PORT : 0;
				uiFlags |= (uiRet & UI_FLAG_BY_IP) ? RULE_CHECK_REM_NETWORK : 0;
				uiFlags |= (uiRet & UI_FLAG_BY_PORT) ? RULE_CHECK_REM_PORT : 0;
            }
            else
            {
                if (DIR_INBOUND == TrafficInfo->Direction)
                {
                    uiFlags |= (uiRet & UI_FLAG_BY_IP) ? RULE_CHECK_LOCAL_NETWORK : 0;
                    uiFlags |= (uiRet & UI_FLAG_BY_PORT) ? RULE_CHECK_LOCAL_PORT : 0;
                }
                else
                {
                    uiFlags |= (uiRet & UI_FLAG_BY_IP) ? RULE_CHECK_REM_NETWORK : 0;
                    uiFlags |= (uiRet & UI_FLAG_BY_PORT) ? RULE_CHECK_REM_PORT : 0;
                }
            }
            
            uiFlags |= (uiRet & UI_FLAG_BY_PATH) ? RULE_CHECK_PROCESS : 0;
            uiFlags |= (uiRet & UI_FLAG_BY_PROTO) ? RULE_CHECK_PROTOCOL : 0;
            uiFlags |= (uiRet & UI_FLAG_NOTIFY) ? RULE_NOTIFY_ON_ACTION : 0;
            uiFlags |= (uiRet & UI_FLAG_BY_MD5) ? RULE_CHECK_MD5: 0;

			// Add a new rule from traffic info 
            status = PubIgnisCreateRuleFromTrafficInfo(TrafficInfo, *Action, uiFlags, &ruleId);
            if (!PT_SUCCESS(status))
            {
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: IgnisCreateRuleFromTrafficInfo failed, status 0x%x", status);
            }
            else
            {
				AddLogEntryEx(ZEROLEVEL, L">>> [%s] Added new rule with id %I64d", (*Action) ? "ALLOW" : "BLOCK", ruleId);
				// Add a new rule from traffic info in xml
				//EnterCriticalSection(&gXmlLock);
				//int iRet = InsertRuleToXmlFromTrafficInfo(TrafficInfo, *Action, uiFlags, ruleId);
				//if (iRet)
				//	AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: Adding a new rule in xml failed");
				//LeaveCriticalSection(&gXmlLock);
            }
        }
    }
}

VOID OnRuleActionNotification(
    __in PTRAFFIC_INFO TrafficInfo,
    __in RULE_ACTION Action,
    __in QWORD RuleId,
    __in PVOID Context
    )
{
    CHAR msg[1024];
    CHAR ip[128];

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Action);
    UNREFERENCED_PARAMETER(TrafficInfo);

    // TRAFFIC_INFO can only have ipVersion4 or ipVersion6
    if (ipVersion4 == TrafficInfo->IpVersion)
    {
        sprintf_s(ip, 128, "%d.%d.%d.%d",
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[3],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[2],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[1],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[0]);
    }
    else
    {
        sprintf_s(ip, 128, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[0],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[1],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[2],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[3],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[4],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[5],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[6],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[7],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[8],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[9],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[10],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[11],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[12],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[13],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[14],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[15]
            );
    }

    sprintf_s(msg, 1024, "[TRAFFIC MATCHED by RuleId %I64d][RULE ACTION][%s] %s, %s %s port %d (Pid: %d, Path: %S)\n",
            RuleId,
            (Action == ACTION_ALLOW) ? "ALLOW" : "BLOCK",
            (TrafficInfo->Direction == DIR_OUTBOUND) ? "OUTBOUND" : "INBOUND",
            (TrafficInfo->Direction == DIR_OUTBOUND) ? "to" : "from",
            ip,
            (TrafficInfo->Direction == DIR_OUTBOUND) ? TrafficInfo->RemotePort : TrafficInfo->LocalPort,
            (DWORD)TrafficInfo->Process.Pid,
            TrafficInfo->Process.ProcessPath.Buffer
            );

	//OutputDebugStringA(msg);
}

VOID OnProcessMd5ChangedNotification(
    __in PTRAFFIC_INFO TrafficInfo,
    __in PBYTE OldMd5,
    __inout RULE_ACTION *Action,
    __in PVOID Context
    )
{
    CHAR msg[1024];
    CHAR ip[128];

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(OldMd5);

    // TRAFFIC_INFO can only have ipVersion4 or ipVersion6
    if (ipVersion4 == TrafficInfo->IpVersion)
    {
        sprintf_s(ip, 128, "%d.%d.%d.%d",
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[3],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[2],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[1],
            TrafficInfo->RemoteIp.Ipv4.Ipv4ValueBytes[0]);
    }
    else
    {
        sprintf_s(ip, 128, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[0],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[1],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[2],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[3],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[4],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[5],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[6],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[7],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[8],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[9],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[10],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[11],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[12],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[13],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[14],
            TrafficInfo->RemoteIp.Ipv6.Ipv6Value[15]
            );
    }

	// If the unattended mode is on then print on the console the information about the md5 changed and set the action to allow
    if (TRUE == gUnattendedMode)
    {
        sprintf_s(msg, 1024, "[PROC CHANGED] %s, %s %s port %d (Pid: %d, Path: %S). Allow?\n",
            (TrafficInfo->Direction == DIR_OUTBOUND) ? "OUTBOUND" : "INBOUND",
            (TrafficInfo->Direction == DIR_OUTBOUND) ? "to" : "from",
            ip,
            (TrafficInfo->Direction == DIR_OUTBOUND) ? TrafficInfo->RemotePort : TrafficInfo->LocalPort,
            (DWORD)TrafficInfo->Process.Pid,
            TrafficInfo->Process.ProcessPath.Buffer
            );

		//OutputDebugStringA(msg);

        *Action = ACTION_ALLOW;
    }
    else
    {
		/*

        INT_PTR uiRet = 0;
        HMODULE hModule = ::GetModuleHandle(0);
        HINSTANCE hInst = hModule;
        PROC_CHANGED_CONTEXT procCtx = { 0 };

		// Find and load the resource for the process changed GUI window
        HRSRC hrsrc = FindResource(hModule, MAKEINTRESOURCE(IDD_PROCESS_CHANGED), RT_DIALOG);
        HGLOBAL hglobal = ::LoadResource(hModule, hrsrc);
        
        procCtx.OldMd5 = OldMd5;
        procCtx.TrafficInfo = (PVOID)TrafficInfo;

		// Display the process changed GUI window
        uiRet = ::DialogBoxIndirectParam(hInst, (LPCDLGTEMPLATE)hglobal, 0, (DLGPROC)ProcessChangedWndProc, (LPARAM)&procCtx);

		// Set the action
        if (uiRet & UI_FLAG_ALLOW)
        {
            *Action = ACTION_ALLOW;
        }
        else
        {
            *Action = ACTION_DENY;
        }
		*/
    }
}

void OnAddressChangedNotification(
__in    IGNIS_ADAPTER*  Adapters,
__in    DWORD           NoOfAdapters,
__out   QWORD*          Profiles,
__in    PVOID           Context
)
{
    PTSTATUS status;
    DWORD i;

    UNREFERENCED_PARAMETER(Context);

    EnterCriticalSection(&(gAdapterListClientLock));

    __try
    {
        //printf("OnAddressChangedNotification called with %d adapters\n", NoOfAdapters);

        if (NULL != gAdapters)
        {
			// Clean the list of the network adapters
            status = CleanupAdaptersInfoClient(gAdapters, gNumberOfAdapters);
            if( !PT_SUCCESS( status ) )
            {
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: CleanupAdaptersInfoClient failed with status: 0%X", status);
                __leave;
            }
        }

        gAdapters = NULL;

        if (NULL != Adapters)
        {
			// Copy the new adapters in the global adapters list
            status = CopyAdaptersClient(Adapters, NoOfAdapters, &gAdapters);
            if (!PT_SUCCESS(status))
            {
				AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: CopyAdaptersClient failed with status: 0x%X", status);
                __leave;
            }
        }

        gNumberOfAdapters = NoOfAdapters;

        for (i = 0; i < NoOfAdapters; ++i)
        {
			// Set the active profile manually for the new adapters
            if (0 == Adapters[i].ActiveProfile)
            {
                Profiles[i] = ((QWORD)1 << i);

                // also update our internal representation
                gAdapters[i].ActiveProfile = Profiles[i];
            }
            else
            {
                Profiles[i] = Adapters[i].ActiveProfile;
            }
        }

		// Display the networks
        VisuallyDisplayNetwork(gAdapters, gNumberOfAdapters); 	
    }
    __finally
    {
        LeaveCriticalSection(&(gAdapterListClientLock));
    }
}

VOID OnPortscanAlertNotification(
    __in    IP_VERSION  Version,
    __in    IGNIS_IP    *RemoteIp,
    __in    DWORD       ScannedPortCount,
    __in    PORTSCAN_PORTS_ENTRY *ScannedPortArray,
    __in    QWORD       StartTimestamp,
    __in    QWORD       EndTimestamp,
    __in    PVOID       Context
    )
{
    PTSTATUS    status;
    CHAR        ip[128] = { 0 };
    DWORD       i = 0;

    UNREFERENCED_PARAMETER(Context);
	// TRAFFIC_INFO can only have ipVersion4 or ipVersion6
    if (ipVersion4 == Version)
    {
        sprintf_s(ip, 128, "%d.%d.%d.%d",
            RemoteIp->Ipv4.Ipv4ValueBytes[3],
            RemoteIp->Ipv4.Ipv4ValueBytes[2],
            RemoteIp->Ipv4.Ipv4ValueBytes[1],
            RemoteIp->Ipv4.Ipv4ValueBytes[0]);
    }
    else
    {
        sprintf_s(ip, 128, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
            RemoteIp->Ipv6.Ipv6Value[0],
            RemoteIp->Ipv6.Ipv6Value[1],
            RemoteIp->Ipv6.Ipv6Value[2],
            RemoteIp->Ipv6.Ipv6Value[3],
            RemoteIp->Ipv6.Ipv6Value[4],
            RemoteIp->Ipv6.Ipv6Value[5],
            RemoteIp->Ipv6.Ipv6Value[6],
            RemoteIp->Ipv6.Ipv6Value[7],
            RemoteIp->Ipv6.Ipv6Value[8],
            RemoteIp->Ipv6.Ipv6Value[9],
            RemoteIp->Ipv6.Ipv6Value[10],
            RemoteIp->Ipv6.Ipv6Value[11],
            RemoteIp->Ipv6.Ipv6Value[12],
            RemoteIp->Ipv6.Ipv6Value[13],
            RemoteIp->Ipv6.Ipv6Value[14],
            RemoteIp->Ipv6.Ipv6Value[15]
            );

    }

	CHAR msg[1024];
	sprintf_s(msg, 1024, "[ALERT] Portscan in progress from %s(%I64d ms)\n", ip, (EndTimestamp - StartTimestamp) / 10000ULL);
	//OutputDebugStringA(msg);
	
	// Display on the console every port scanned 
    for (i = 0; i < ScannedPortCount; i++)
    {
        RtlSecureZeroMemory(ip, 128);
        status = IgnisIpToCharArray(ScannedPortArray[i].LocalIp, Version, &ip);
        if (PT_SUCCESS(status))
        {
			CHAR msg[1024];
			sprintf_s(msg, 1024, "    *Port: %d, ScanType: %d Protocol: %d LocalIp: %s\n", (DWORD)ScannedPortArray[i].Port, ScannedPortArray[i].ScanType, ScannedPortArray[i].Protocol, ip);
			//OutputDebugStringA(msg);	
		}
    }
	InsertIntoFWReports(ip);
	theApp.SendParCtrlMessage2Tray(SHOW_PORT_SCAN_POPUP);	
}

/***************************************************************************************************
*  Function Name  : InsertIntoFWReports
*  Description    : To insert data in Firewall reports when port scan alert notification generated  
*  Author Name    : Kunal Waghmare
*  Date			  :	16 Aug 2018
****************************************************************************************************/
bool InsertIntoFWReports(char * LocalIP)
{
	try
	{
		if (!LocalIP)
		{
			return false;
		}
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csAppRulesDB = L"";
		csAppRulesDB.Format(L"%sVBALLREPORTS.DB", csWardWizModulePath);		
		CT2A dbPath(csAppRulesDB, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
		CStringA csInsertQuery = "";
		csInsertQuery.AppendFormat("INSERT INTO Wardwiz_FirewallDetails(db_FirewallDate, db_FirewallTime, db_FW_LocalIP) VALUES (Date('now'), Datetime('now','localtime'), '%s');", LocalIP);
		objSqlDb.Open();
		objSqlDb.ExecDML(csInsertQuery);
		objSqlDb.Close();
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in InsertIntoFWReports");
		return false;
	}
	return true;
}

PTSTATUS SetupCallbacks()
{
    PTSTATUS status = STATUS_SUCCESS;
    IGNIS_CALLBACK_REGISTRATION callbacks = { 0 };

	// OnTrafficUnmatchedCallback 
    callbacks.ModifyIgnisOnTrafficUnmatchedCallback = TRUE;
    callbacks.IgnisOnTrafficUnmatchedCallback = OnTrafficUnmatchedNotification;
    callbacks.IgnisOnAdaptersChangedContext = NULL;

	// OnRuleActionCallback
    callbacks.ModifyIgnisOnRuleActionCallback = TRUE;
    callbacks.IgnisOnRuleActionCallback = OnRuleActionNotification;
    callbacks.IgnisOnRuleActionContext = NULL;

	// OnMd5ChangedCallback 
    callbacks.ModifyIgnisOnMd5ChangedCallback = TRUE;
    callbacks.IgnisOnMd5ChangedCallback = OnProcessMd5ChangedNotification;
    callbacks.IgnisOnMd5ChangedContext = NULL;

	// OnAdaptersChangedCallback
    callbacks.ModifyIgnisOnAdaptersChangedCallback = TRUE;
    callbacks.IgnisOnAdaptersChangedCallback = OnAddressChangedNotification;
    callbacks.IgnisOnAdaptersChangedContext = NULL;

	// OnPortscanAlertCallback
    callbacks.ModifyIgnisOnPortscanAlertCallback = TRUE;
    callbacks.IgnisOnPortscanAlertCallback = OnPortscanAlertNotification;
    callbacks.IgnisOnPortscanAlertContext = NULL;

	// Register the callbacks in ignis
    status = PubIgnisRegisterCallbacks(&callbacks);
    if( !PT_SUCCESS( status ))
    {
		AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: PubIgnisRegisterCallbacks failed with status: 0x%X", status);
        return status;
    }

    return status;
}

bool InsertApplicationsIntoDB(std::string strAppPath)
{
	try
	{
		if (CheckApplicationExists(strAppPath))
		{
			return false;
		}

		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csAppRulesDB = L"";
		csAppRulesDB.Format(L"%sVBFIREWALL.DB", csWardWizModulePath);

		CT2A dbPath(csAppRulesDB, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);

		//check here for duplicate entries in database

		CStringA csApplicationPath = strAppPath.c_str();
		CStringA csInsertQuery;
		csInsertQuery.Format("SELECT PROGRAMPATH FROM WWIZ_FIREWALL_PACKET_RULE_CONFIG WHERE PROGRAMPATH = '%s'", csApplicationPath.MakeLower());
		
		objSqlDb.Open();
		CWardwizSQLiteTable qResult = objSqlDb.GetTable(csInsertQuery);
		objSqlDb.Close();

		DWORD dwRows = qResult.GetNumRows();
		if (dwRows != 0)
		{
			return false;
		}

		CStringA csAppName(csApplicationPath.MakeLower());
		csAppName = csAppName.Right(csAppName.GetLength() - csAppName.ReverseFind('\\') - 1);

		csInsertQuery.Format("INSERT INTO WWIZ_FIREWALL_PACKET_RULE_CONFIG (LOCAL_IP, LOCAL_IP_DATA, LOCAL_PORT, LOCAL_PORT_DATA, REMOTE_IP, REMOTE_IP_DATA, REMOTE_PORT, REMOTE_PORT_DATA,	PROGRAMPATH, PROGRAMNAME, PROTOCOL,	ACCESS,	NETWORKTYPE, DIRECTION, SETRULES, RESERVED1, RESERVED2, RESERVED3) VALUES(%d,'%s',%d,'%s',%d,'%s',%d,'%s','%s','%s',%d,%d,'%s',%d, %d,'%s','%s','%s');",0x00, "", 0x00, "", 0x00, "", 0x00, "", csApplicationPath.MakeLower(), csAppName, 0x00, 0x01, "1", 0x03, 0x00, "", "", "");

		objSqlDb.Open();
		objSqlDb.ExecDML(csInsertQuery);
		objSqlDb.Close();
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in InsertApplicationsIntoDB");
		return false;
	}
	return true;
}


bool CheckApplicationExists(std::string strAppPath)
{
	bool bFound = false;
	try
	{
		_wcslwr((wchar_t*)strAppPath.c_str());
		for (std::vector<std::string>::iterator it = g_vAppList.begin(); it != g_vAppList.end(); ++it)
		{
			if (strAppPath == (*it))
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			g_vAppList.push_back(strAppPath);
		}
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in CheckApplicationExists");
	}
	return bFound;
}