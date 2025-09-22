
#ifndef _OPTIONS_PUBLIC_H_
#define _OPTIONS_PUBLIC_H_

// IGNIS.SYS
#define OPT_FIREWALL_TRAFFIC_ACTION                         0x02000003  // this is the default action in case the user doesn't responds to callbacks

#define OPT_FIREWALL_DEFAULT_ACTION_WITHOUT_CLIENT          0x02000006  // allows to specify the default action taken in case no client is connected

#define OPT_LOCKDOWN_STATUS                                 0x1200010B      // set(value = 1)/reset(value = 0) lockdown state

#define OPT_CLEAR_PORTSCAN_LOCKED_HOSTS                     0x1200010E  // remove lockdown for all hosts

#endif //_OPTIONS_PUBLIC_H_