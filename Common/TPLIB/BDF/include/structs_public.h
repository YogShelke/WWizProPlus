
#ifndef _STRUCTS_PUBLIC_H_
#define _STRUCTS_PUBLIC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push)
#pragma pack(8)

    #ifndef ANYSIZE_ARRAY
    #define ANYSIZE_ARRAY 1
    #endif

    #define     MAX_QWORD           0xFFFFFFFFFFFFFFFFULL

    #define     SIZEOF_MD5          16

    #define     DEFAULT_MAC_LEN     6

    typedef enum _PORTSCAN_TYPE {
        portscanTypeUndefined = 0,
        portscanTypeTcpXmas = 1,
        portscanTypeTcpXmasFull = 2,
        portscanTypeTcpSynScan = 3,
        portscanTypeTcpNull = 4,
        portscanTypeTcpFin = 5,
        portscanTypeUdpNodata = 6,
    } PORTSCAN_TYPE;

    typedef struct _IGNIS_STRING
    {
        DWORD       Length;             // length in bytes of the STRING including the NULL terminator
        WCHAR       Buffer[ANYSIZE_ARRAY];
    }IGNIS_STRING, *PIGNIS_STRING;      // structure that holds the length of a NULL terminating string

    typedef struct _PORT_RANGE
    {
        WORD    Min;
        WORD    Max;
    } PORT_RANGE, *PPORT_RANGE;         // structure that is used for rules that apply to port intervals

    typedef struct  _IPV4
    {
        union{
            DWORD       Ipv4Value;
            BYTE        Ipv4ValueBytes[4];
        };
        union{
            DWORD       Mask;
            BYTE        MaskBytes[4];
        };
    }IPV4, *PIPV4;

    typedef struct _IPV6
    {
        UINT8       Ipv6Value[16];
        UINT8       Prefix;
    } IPV6, *PIPV6;

    typedef union _IGNIS_IP
    {
        IPV4    Ipv4;
        IPV6    Ipv6;
    } IGNIS_IP, *PIGNIS_IP;

    typedef struct _PORTSCAN_PORTS_ENTRY {
        BYTE            Protocol;
        WORD            Port;
        PORTSCAN_TYPE   ScanType;
        IGNIS_IP        LocalIp;
    } PORTSCAN_PORTS_ENTRY, *PPORTSCAN_PORTS_ENTRY;

    typedef enum _TRAFFIC_DIRECTION
    {
        DIR_INBOUND = 0x1,
        DIR_OUTBOUND = 0x2,
        DIR_BOTH = 0x3
    } TRAFFIC_DIRECTION, *PTRAFFIC_DIRECTION;

    typedef enum _RULE_ACTION
    {
        ACTION_ALLOW = 1,
        ACTION_DENY = 2,
    } RULE_ACTION, *PRULE_ACTION;

    typedef enum _TRAFFIC_ACTION
    {
        TRAFFIC_ACTION_ASK = 0,
        TRAFFIC_ACTION_ALLOW = 1,
        TRAFFIC_ACTION_DENY = 2,
    } TRAFFIC_ACTION, *PTRAFFIC_ACTION;

#define BDF_MD5_MAX_SCAN_LENGTH        (1024 * 1024)

    typedef struct _PROCESS_INFO
    {
        BYTE               Md5Value[SIZEOF_MD5];
        BOOLEAN            Md5Valid;
        QWORD              Pid;
        PIGNIS_STRING      CommandLine;
        IGNIS_STRING       ProcessPath;
    } PROCESS_INFO, *PPROCESS_INFO;     // structure that holds the process image path, pid, command line and MD5 (which is performed asynchronously)

    typedef enum _IP_VERSION
    {
        ipVersion4 = 0x1,
        ipVersion6 = 0x2,
        ipVersionAny = 0x3,
    } IP_VERSION, *PIP_VERSION;

    typedef struct _TRAFFIC_INFO
    {
        IGNIS_IP                LocalIp;
        IGNIS_IP                RemoteIp;
        WORD                    LocalPort;
        WORD                    RemotePort;
        IP_VERSION              IpVersion;
        BYTE                    Protocol;
        TRAFFIC_DIRECTION       Direction;
        DWORD                   InterfaceIndex;
        PIGNIS_STRING           ParentProcessPath;
        BOOLEAN                 ParentProcessMd5Valid;
        BYTE                    ParentProcessMd5Value[SIZEOF_MD5];
        PIGNIS_STRING           UserSid;
        PROCESS_INFO            Process;
        
    } TRAFFIC_INFO, *PTRAFFIC_INFO;

   

    typedef enum _RULE_FLAGS
    {
        RULE_CHECK_PROCESS              = 0x1,
        RULE_CHECK_LOCAL_PORT           = 0x2,
        RULE_CHECK_LOCAL_NETWORK        = 0x4,
        RULE_CHECK_REM_NETWORK          = 0x8,
        RULE_CHECK_PROTOCOL             = 0x10,
        RULE_CHECK_REM_PORT             = 0x20,
        RULE_CHECK_PROFILE              = 0x40,
        RULE_CHECK_SID                  = 0x80,
        RULE_CHECK_MD5                  = 0x100,
        RULE_CHECK_COMMAND_LINE         = 0x200,
        RULE_CHECK_REM_MAC              = 0x400,

        RULE_NOTIFY_ON_ACTION           = 0x10000000,
        RULE_NOTIFY_ON_ACTION_ONCE      = 0x20000000,
        RULE_CHECK_ALL                  = 0xFFFFFFFF
    } RULE_FLAGS, *PRULE_FLAGS;

    typedef enum _RULE_CATEGORY
    {
        RULE_CATEGORY_USER              = 0x00,
        RULE_CATEGORY_STANDARD          = 0x0F,
        RULE_CATEGORY_MACHINE           = 0xF0,
        RULE_CATEGORY_CRITICAL          = 0xFA
    } RULE_CATEGORY, *PRULE_CATEGORY;   // determines the precedence of the rule, the higher value the category
                                        // the higher the priority

    typedef struct _IGNIS_MAC
    {
        BYTE    PhysicalAddress[DEFAULT_MAC_LEN];
    }IGNIS_MAC, *PIGNIS_MAC;

    typedef struct _IGNIS_RULE
    {
        QWORD               RuleId;                 // uniquely identifies a public rule, used for IgnisDeleteRule
        BYTE                Category;               // defaults to RULE_CATEGORY_USER

        IP_VERSION          IpVersion;              // must be set
        RULE_ACTION         Action;                 // must be set
        BYTE                Protocol;               // optional, used if RULE_CHECK_PROTOCOL is set
        TRAFFIC_DIRECTION   Direction;              // must be set
        IGNIS_IP            LocalNetwork;           // optional, used if RULE_CHECK_LOCAL_NETWORK is set
        IGNIS_IP            *RemoteNetworks;        // optional, used if RULE_CHECK_REM_NETWORK is set
        DWORD               RemoteNetworkCount;     // must be set
        PORT_RANGE          LocalPorts;             // optional, used if RULE_CHECK_LOCAL_PORT is set
        PORT_RANGE          *RemotePortRanges;      // optional, used if RULE_CHECK_REM_PORT is set
        DWORD               RemotePortRangesCount;  // must be set
        IGNIS_MAC           *RemoteMacs;
        DWORD               RemoteMacsCount;        
        LIST_ENTRY          Link;                   // reserved for internal use
        QWORD               Flags;                  // must be set, specifies which optional fields are set

        // at least one optional member must be set
        PVOID               InternalRule;           // reserved for internal use 
        PPROCESS_INFO       Process;                // optional, used if RULE_CHECK_PROCESS is set

        // optional, used if RULE_CHECK_SID is set 
        // pointer to the sid string
        PIGNIS_STRING       Sid;                    
        QWORD               ProfileBitmap;          // optional, used if RULE_CHECK_PROFILE is set

    } IGNIS_RULE, *PIGNIS_RULE;

    typedef struct _NET_INFO
    {
        IP_VERSION          Version;
        IGNIS_IP            Ip;

        BYTE                MacAddr[DEFAULT_MAC_LEN];             /* for 6-byte hardware addresses */
        ULONG               PhysAddrLen;                          // represents the number of bytes in the MAC address
    } NET_INFO, *PNET_INFO;

    typedef struct _IGNIS_ADAPTER
    {
        // used by Windows to uniquely identify the adapter
        GUID                AdapterID;

        // Useless for settings or etc, used only for commodity
        IGNIS_STRING*       AdapterName;

        // used for setting rules based on LOCAL_INTERFACE_INDEX
        // different for Ipv4 and Ipv6
        DWORD               Ipv4IfIndex;
        DWORD               Ipv6IfIndex;

        NET_INFO*           LocalIpAddresses;
        DWORD               NoOfLocalIpAddresses;

        NET_INFO*           DhcpServers;
        DWORD               NoOfDhcpServers;

        NET_INFO*           DnsServers;
        DWORD               NoOfDnsServers;

        NET_INFO*           Gateways;
        DWORD               NoOfGateways;

        // E.g:
        // 0n6  = IF_TYPE_ETHERNET_CSMACD,
        // 0n71 = IF_TYPE_IEEE80211 a.k.a WiFi
        // All defines can be found in ipifcons.h
        DWORD               Medium;

        // only 1 bit can be set at all times
        QWORD               ActiveProfile;

    } IGNIS_ADAPTER, *PIGNIS_ADAPTER;

    typedef struct _IGNIS_SETTINGS
    {
        
        BOOLEAN             Enabled;                    // if not set the firewall's rules are not active, i.e. they have no effect

        TRAFFIC_ACTION      DefaultFirewallAction;      // will determine if for traffic we have no rules for do we block it, allow or send an notification asking that action to take

        BOOLEAN             BlockPortScans;             

    } IGNIS_SETTINGS, *PIGNIS_SETTINGS;

    typedef struct _IGNIS_INIT
    {
        // for proper initialization use LIBRARY_INTERFACE_VERSION from ignis_dll.h
        // this is used to check if the DLL client is compatible with the DLL version
        __in        DWORD               LibraryVersionUsed;

        // Initial settings
        __in        IGNIS_SETTINGS      StartSettings;
        __in        BYTE                OemClientId[16];
    } IGNIS_INIT, *PIGNIS_INIT;

#pragma pack(pop)
#ifdef __cplusplus
}
#endif

#endif // _STRUCTS_PUBLIC_H_