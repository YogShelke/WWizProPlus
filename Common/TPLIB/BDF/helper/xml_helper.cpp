#include "stdafx.h"
#include "xml_helper.h"
#include "imports.h"
#include "structs_public.h"
#include "ip_helper.h"

extern TiXmlElement * gXmlRoot;

int UpdateSettingsInXml(
	__in IGNIS_SETTINGS & settings
)
{
	TiXmlElement * pXmlSettings = gXmlRoot->FirstChildElement("settings");
	if (pXmlSettings == NULL)
	{
		AddLogEntryEx(SECONDLEVEL, L"### [ERROR]: Could not retrieve the settings node");
		return -1;
	}
	// Set the attributes for the corresponding settings
	pXmlSettings->SetAttribute("enabled", settings.Enabled);
	pXmlSettings->SetAttribute("defaultAction", settings.DefaultFirewallAction);
	pXmlSettings->SetAttribute("blockPortScans", settings.BlockPortScans);
	return 0;
}

BOOL ReadNetwork(
	__inout PIGNIS_IP network, 
	__in IP_VERSION ipVersion, 
	__in const char * text
)
{
	DWORD index, w7, w6, w5, w4, w3, w2, w1, w0;
	// Read the ip using a specific format
	if (ipVersion == ipVersion4)
	{
		if (sscanf_s(text, "%d.%d.%d.%d", &w3, &w2, &w1, &w0) == 4)
		{
			network->Ipv4.Ipv4ValueBytes[3] = (BYTE)w3;
			network->Ipv4.Ipv4ValueBytes[2] = (BYTE)w2;
			network->Ipv4.Ipv4ValueBytes[1] = (BYTE)w1;
			network->Ipv4.Ipv4ValueBytes[0] = (BYTE)w0;
			network->Ipv4.Mask = 0xffffffff;
		}
		else
		{
			AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: Wrong format for ipv4 address");
			return FALSE;
		}
	}
	else if (ipVersion == ipVersion6)
	{
		if (sscanf_s(text, "%X:%X:%X:%X:%X:%X:%X:%X", &w7, &w6, &w5, &w4, &w3, &w2, &w1, &w0) == 8)
		{
			*((PUINT16)&network->Ipv6.Ipv6Value[0]) = (UINT16)w7;
			*((PUINT16)&network->Ipv6.Ipv6Value[2]) = (UINT16)w6;
			*((PUINT16)&network->Ipv6.Ipv6Value[4]) = (UINT16)w5;
			*((PUINT16)&network->Ipv6.Ipv6Value[6]) = (UINT16)w4;
			*((PUINT16)&network->Ipv6.Ipv6Value[8]) = (UINT16)w3;
			*((PUINT16)&network->Ipv6.Ipv6Value[10]) = (UINT16)w2;
			*((PUINT16)&network->Ipv6.Ipv6Value[12]) = (UINT16)w1;
			*((PUINT16)&network->Ipv6.Ipv6Value[14]) = (UINT16)w0;

			for (index = 0; index < 8; index++)
			{
				BYTE t = network->Ipv6.Ipv6Value[2 * index];
				network->Ipv6.Ipv6Value[2 * index] = network->Ipv6.Ipv6Value[2 * index + 1];
				network->Ipv6.Ipv6Value[2 * index + 1] = t;
			}
			network->Ipv6.Prefix = 128;
		}
		else
		{
			AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: Wrong format for ipv6 address");
			return FALSE;
		}
	}
	return TRUE;
}
BOOL ReadPath(
	__inout PIGNIS_RULE rule, 
	__in const char * path
)
{
	// Take the length of the path
	int length = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
	// Allocate the memory needed for the process
	rule->Process = (PPROCESS_INFO)malloc(sizeof(PROCESS_INFO) + (length * sizeof(WCHAR)));
	if (NULL == rule->Process)
	{
		AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: error allocating memory for process %s", path);
		return FALSE;
	}
	memset((void*)rule->Process, 0, sizeof(PROCESS_INFO) + (length * sizeof(WCHAR)));
	MultiByteToWideChar(CP_UTF8, 0, path, -1, rule->Process->ProcessPath.Buffer, length);
	rule->Process->ProcessPath.Length = (DWORD)(length * sizeof(WCHAR));
	return TRUE;
}

BOOL  ReadMD5(
	__inout PPROCESS_INFO processInfo, 
	__in const char * text
)
{
	DWORD w0;
	for (int i = 0; i < 16; i++)
	{
		// Takes two hexadecimal digits from text and adds them to the Md5Value array 
		if (sscanf_s(&text[i * 2], "%02x", &w0) == 1)
			processInfo->Md5Value[i] = (BYTE)w0;
		else
		{
			AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: Wrong format for md5");
			return FALSE;
		}
	}
	return TRUE;
}

BOOL GetXMLInt(
	__in const TiXmlNode *pNode,
	__in const char *path,
	__out int &rValue,
	__out const char **pszFoundValue
)
{
	static TiXmlString string;
	// Read the string value of the path starting from pNode
	if (TinyXPath::o_xpath_string(pNode, path, string) == FALSE)
	{
		(*pszFoundValue) = "none";
		return FALSE;
	}

	(*pszFoundValue) = string.c_str();

	// Retrieve the integer value from the string
	if (sscanf(string.c_str(), "%d", &rValue) != 1)
		return FALSE;

	// Make sure the integer value is equivalent with the string value
	char buf[11];
	sprintf(buf, "%d", rValue);
	if (strcmp(buf, string.c_str()) != 0)
		return FALSE;

	return TRUE;
}

int InsertRuleToXml(
	__in PIGNIS_RULE rule
)
{
	CHAR        ip[128] = { 0 };
	CHAR        text[256] = { 0 };
	CHAR		ruleId[20] = { 0 };
	PTSTATUS status;
	TiXmlElement * pRulesRoot = gXmlRoot->FirstChildElement("rules");
	if (pRulesRoot == NULL)
	{
		AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: Could not retrieve the rules node");
		return -1;
	}
	// Create a new rule node
	TiXmlElement * pXmlRule = new TiXmlElement("rule");
	pRulesRoot->LinkEndChild(pXmlRule);
	// RuleId
	sprintf_s(ruleId, 20, "%llu", rule->RuleId);
	pXmlRule->SetAttribute("id", ruleId);
	// Category
	pXmlRule->SetAttribute("category", rule->Category);
	// IpVersion
	pXmlRule->SetAttribute("ipVersion",rule->IpVersion);
	// Action
	pXmlRule->SetAttribute("action", rule->Action);
	// Direction
	pXmlRule->SetAttribute("direction", rule->Direction);
	// Protocol
	if (rule->Flags & RULE_CHECK_PROTOCOL)
		pXmlRule->SetAttribute("protocol", rule->Protocol);
	// Notify
	if (rule->Flags & RULE_NOTIFY_ON_ACTION)
		pXmlRule->SetAttribute("notify", "1");
	// LocalPort
	if (rule->Flags & RULE_CHECK_LOCAL_PORT)
	{
		TiXmlElement * pXmlLocalPort = new TiXmlElement("localPort");
		pXmlRule->LinkEndChild(pXmlLocalPort);
		pXmlLocalPort->SetAttribute("Min", rule->LocalPorts.Min);
		pXmlLocalPort->SetAttribute("Max", rule->LocalPorts.Max);
	}
	// LocalNetwork
	if (rule->Flags & RULE_CHECK_LOCAL_NETWORK)
	{
		RtlSecureZeroMemory(ip, 128);
		status = IgnisIpToCharArray(rule->LocalNetwork, rule->IpVersion, &ip);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: The local ip is incorrect");
			return -1;
		}
		pXmlRule->SetAttribute("localIp", ip);
	}
	// RemotePortRanges
	if (rule->Flags & RULE_CHECK_REM_PORT)
	{
		TiXmlElement * pXmlRemotePortRanges = new TiXmlElement("remotePorts");
		pXmlRule->LinkEndChild(pXmlRemotePortRanges);
		for (WORD i = 0; i < rule->RemotePortRangesCount; i++)
		{
			TiXmlElement * pXmlRemotePortRange = new TiXmlElement("remotePort");
			pXmlRemotePortRanges->LinkEndChild(pXmlRemotePortRange);
			pXmlRemotePortRange->SetAttribute("Min", rule->RemotePortRanges[i].Min);
			pXmlRemotePortRange->SetAttribute("Max", rule->RemotePortRanges[i].Max);
		}
	}
	// RemoteNetworks
	if (rule->Flags & RULE_CHECK_REM_NETWORK)
	{
		TiXmlElement * pXmlRemoteNetworks = new TiXmlElement("remoteNetworks");
		pXmlRule->LinkEndChild(pXmlRemoteNetworks);
		for (WORD i = 0; i < rule->RemoteNetworkCount; i++)
		{
			TiXmlElement * pXmlRemoteNetwork = new TiXmlElement("remoteNetwork");
			pXmlRemoteNetworks->LinkEndChild(pXmlRemoteNetwork);
			RtlSecureZeroMemory(ip, 128);
			status = IgnisIpToCharArray(rule->RemoteNetworks[i], rule->IpVersion, &ip);
			if (!PT_SUCCESS(status))
			{
				AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: The remote ip is incorrect");
				return -1;
			}
			pXmlRemoteNetwork->SetAttribute("remoteIp", ip);
		}
	}
	// Process
	char * buffer = NULL;
	if (rule->Flags & RULE_CHECK_PROCESS)
	{
		TiXmlElement * pXmlProcess = new TiXmlElement("process");
		pXmlRule->LinkEndChild(pXmlProcess);
		// Path
		buffer = (char *)malloc(rule->Process->ProcessPath.Length * sizeof(char));
		WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)rule->Process->ProcessPath.Buffer, -1, buffer, rule->Process->ProcessPath.Length, 0, NULL);
		pXmlProcess->SetAttribute("path", buffer);
		// MD5
		if (rule->Flags & RULE_CHECK_MD5)
		{
			MD5ValueToCharArray(rule->Process->Md5Value, &text);
			pXmlProcess->SetAttribute("md5Value", text);
			pXmlProcess->SetAttribute("md5Valid", rule->Process->Md5Valid);
		}
	}
	// SID
	if (rule->Flags & RULE_CHECK_SID)
	{
		WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)rule->Sid->Buffer, -1, buffer, rule->Sid->Length, 0, NULL);
		pXmlRule->SetAttribute("sid", buffer);
	}
	// ProfileBitmap
	if (rule->Flags & RULE_CHECK_PROFILE)
	{
		char profile[20];
		sprintf_s(profile, 20, "%I64X", rule->ProfileBitmap);
		pXmlRule->SetAttribute("profileBitmap", profile);
	}
	if (buffer) free(buffer);
	return 0;
}

int InsertRuleToXmlFromTrafficInfo(
	__in PTRAFFIC_INFO trafficInfo,
	__in int action,
	__in QWORD flags,
	__in QWORD ruleId
)
{
	CHAR        ip[128] = { 0 };
	CHAR        text[256] = { 0 };
	CHAR		id[20] = { 0 };
	PTSTATUS status;
	TiXmlElement * pRulesRoot = gXmlRoot->FirstChildElement("rules");
	if(pRulesRoot == NULL)
	{
		AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: Could not retrieve the rules node");
		return -1;
	}
	// Create a new rule node
	TiXmlElement * pXmlRule = new TiXmlElement("rule");
	pRulesRoot->LinkEndChild(pXmlRule);
	// RuleId
	sprintf_s(id, 20, "%llu", ruleId);
	pXmlRule->SetAttribute("id", id);
	// IpVersion
	pXmlRule->SetAttribute("ipVersion", trafficInfo->IpVersion);
	// Action
	pXmlRule->SetAttribute("action", action);
	// Direction
	pXmlRule->SetAttribute("direction", trafficInfo->Direction);
	// Protocol
	if (flags & RULE_CHECK_PROTOCOL)
		pXmlRule->SetAttribute("protocol", trafficInfo->Protocol);
	// Notify
	if (flags & RULE_NOTIFY_ON_ACTION)
		pXmlRule->SetAttribute("notify", "1");
	// LocalPort
	if (flags & RULE_CHECK_LOCAL_PORT)
	{
		TiXmlElement * pXmlLocalPort = new TiXmlElement("localPort");
		pXmlRule->LinkEndChild(pXmlLocalPort);
		pXmlLocalPort->SetAttribute("Min", trafficInfo->LocalPort);
		pXmlLocalPort->SetAttribute("Max", trafficInfo->LocalPort);
	}
	// LocalNetwork
	if (flags & RULE_CHECK_LOCAL_NETWORK)
	{
		RtlSecureZeroMemory(ip, 128);
		status = IgnisIpToCharArray(trafficInfo->LocalIp, trafficInfo->IpVersion, &ip);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(ZEROLEVEL, L"### [ERROR]: The local ip from trafficInfo is incorrect");
			return -1;
		}
		pXmlRule->SetAttribute("localIp", ip);
	}
	// RemotePortRanges
	if (flags & RULE_CHECK_REM_PORT)
	{
		TiXmlElement * pXmlRemotePortRanges = new TiXmlElement("remotePorts");
		pXmlRule->LinkEndChild(pXmlRemotePortRanges);
		TiXmlElement * pXmlRemotePortRange = new TiXmlElement("remotePort");
		pXmlRemotePortRanges->LinkEndChild(pXmlRemotePortRange);
		pXmlRemotePortRange->SetAttribute("Min", trafficInfo->RemotePort);
		pXmlRemotePortRange->SetAttribute("Max", trafficInfo->RemotePort);
	}
	// RemoteNetworks
	if (flags & RULE_CHECK_REM_NETWORK)
	{
		RtlSecureZeroMemory(ip, 128);
		TiXmlElement * pXmlRemoteNetworks = new TiXmlElement("remoteNetworks");
		pXmlRule->LinkEndChild(pXmlRemoteNetworks);
		TiXmlElement * pXmlRemoteNetwork = new TiXmlElement("remoteNetwork");
		pXmlRemoteNetworks->LinkEndChild(pXmlRemoteNetwork);
		status = IgnisIpToCharArray(trafficInfo->RemoteIp, trafficInfo->IpVersion, &ip);
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(ZEROLEVEL, L"### The remote ip from trafficInfo is incorrect");
			return -1;
		}
		pXmlRemoteNetwork->SetAttribute("remoteIp", ip);
	}
	// Process
	if (flags & RULE_CHECK_PROCESS)
	{
		TiXmlElement * pXmlProcess = new TiXmlElement("process");
		pXmlRule->LinkEndChild(pXmlProcess);
		// Path
		char * processPath = NULL;
		processPath = (char *)malloc(trafficInfo->Process.ProcessPath.Length * sizeof(char));
		WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)trafficInfo->Process.ProcessPath.Buffer, -1, processPath, trafficInfo->Process.ProcessPath.Length, 0, NULL);
		pXmlProcess->SetAttribute("path", processPath);
		if (processPath) free(processPath);
		// MD5
		if (flags & RULE_CHECK_MD5)
		{
			MD5ValueToCharArray(trafficInfo->Process.Md5Value, &text);
			pXmlProcess->SetAttribute("md5Value", text);
			pXmlProcess->SetAttribute("md5Valid", trafficInfo->Process.Md5Valid);
		}
	}
	return 0;
}
BOOL ReadRule(
	__inout IGNIS_RULE * newRule, 
	__in TiXmlNode *pXmlRule
)
{
	BOOL bRes;
	int value;
	const char *strValue;
	TiXmlString xmlString;
	// Check if the category attribute exists
	if (pXmlRule->ToElement()->Attribute("category"))
	{
		// Category
		bRes = GetXMLInt(pXmlRule, "@category", value, &strValue);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Invalid rule category; value: %s", strValue);
			return FALSE;
		}
		if (value != 0 && value != 15 && value != 240 && value != 250)
		{
			CHAR msg[1024] = { 0 };
			sprintf_s(msg, 1024, "Rule category: 0 - user 15 - standard 240 - machine  250 - critical value : %d\n", value);
			//OutputDebugStringA(msg);

			return FALSE;
		}
		newRule->Category = (RULE_CATEGORY)value;
	}

	// IpVersion
	bRes = GetXMLInt(pXmlRule, "@ipVersion", value, &strValue);
	if (bRes == FALSE)
	{
		AddLogEntryEx(ZEROLEVEL, L"### Invalid ipVersion value; value: %s", strValue);
		return FALSE;
	}
	if (value < 1 || value > 3)
	{
		CHAR msg[1024] = { 0 };
		sprintf_s(msg, 1024, "Ip version allowed values: 1 - ipv4, 2 - ipv6, 3 - both; provided: %d\n", value);
		//OutputDebugStringA(msg);
		return FALSE;
	}
	newRule->IpVersion = (IP_VERSION)value;

	// Action
	bRes = GetXMLInt(pXmlRule, "@action", value, &strValue);
	if (bRes == FALSE)
	{
		AddLogEntryEx(ZEROLEVEL, L"### Invalid rule action; value: %s", strValue);
		return FALSE;
	}
	if (value < 1 || value > 2)
	{
		CHAR msg[1024] = { 0 };
		sprintf_s(msg, 1024, "Rule action: 1 - allow 2 - deny value : %d\n", value);
		//OutputDebugStringA(msg);
		return FALSE;
	}
	newRule->Action = (RULE_ACTION)value;

	// Direction
	bRes = GetXMLInt(pXmlRule, "@direction", value, &strValue);
	if (bRes == FALSE)
	{
		AddLogEntryEx(ZEROLEVEL, L"### Invalid direction value: %s", strValue);
		return FALSE;
	}
	if (value < 1 || value > 3)
	{
		CHAR msg[1024] = { 0 };
		sprintf_s(msg, 1024, "Direction values: 1 - inbound, 2 - outbound, 3 - both; provided: %d\n", value);
		//OutputDebugStringA(msg);
		return FALSE;
	}
	newRule->Direction = (TRAFFIC_DIRECTION)value;

	// Check if the protocol attribute exists
	if (pXmlRule->ToElement()->Attribute("protocol"))
	{
		// Protocol
		bRes = GetXMLInt(pXmlRule, "@protocol", value, &strValue);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Invalid rule protocol value; value: %s", strValue);
			return FALSE;
		}
		if (value < 0 || value > 142)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Invalid protocol: %d", value);
			return FALSE;
		}
		newRule->Protocol = (BYTE)value;
		newRule->Flags |= RULE_CHECK_PROTOCOL;
	}
	// Check if the notify attribute exists
	if (pXmlRule->ToElement()->Attribute("notify"))
	{
		// Notify
		bRes = GetXMLInt(pXmlRule, "@notify", value, &strValue);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Invalid notify value; value: %s", strValue);
			return FALSE;
		}
		if (value < 0 || value > 1)
		{
			CHAR msg[1024] = { 0 };
			sprintf_s(msg, 1024, "Notify values : 0 - off 1 - on ; provided %d\n", value);
			//OutputDebugStringA(msg);
			return FALSE;
		}
		if (value)
			newRule->Flags |= RULE_NOTIFY_ON_ACTION;
	}

	// Check if the profileBitmap attribute exists
	if (pXmlRule->ToElement()->Attribute("profileBitmap"))
	{
		// ProfileBitmap
		QWORD profile;
		bRes = TinyXPath::o_xpath_string(pXmlRule, "@profileBitmap", xmlString);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"### profile Bitmap attribute not found in process");
			return FALSE;
		}
		if (sscanf_s(xmlString.c_str(), "%I64X", &profile) == 1)
			newRule->ProfileBitmap = profile;
		else
		{
			AddLogEntryEx(ZEROLEVEL, L"### Wrong format for profileBitmap");
			return FALSE;
		}
		newRule->Flags |= RULE_CHECK_PROFILE;
	}

	// Check if the localPort node exists
	if (pXmlRule->ToElement()->FirstChild("localPort"))
	{
		// LocalPorts
		TiXmlNode * pXmlLocalPort = pXmlRule->FirstChild("localPort");
		bRes = GetXMLInt(pXmlLocalPort, "@Min", value, &strValue);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Invalid local ports minimum value; value: %s", strValue);
			return FALSE;
		}
		if (value < 0)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Local ports values must be > 0; provided: %d", value);
			return FALSE;
		}
		newRule->LocalPorts.Min = (WORD)value;
		bRes = GetXMLInt(pXmlLocalPort, "@Max", value, &strValue);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Invalid local ports maximum value; value: %s", strValue);
			return FALSE;
		}
		if (value < 0)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Local ports values must be > 0; provided: %d", value);
			return FALSE;
		}
		newRule->LocalPorts.Max = (WORD)value;
		newRule->Flags |= RULE_CHECK_LOCAL_PORT;
	}

	// Check if the sid attribute exists
	if (pXmlRule->ToElement()->Attribute("sid"))
	{
		// Sid
		bRes = TinyXPath::o_xpath_string(pXmlRule, "@sid", xmlString);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"### Invalid value for sid attribute");
			return FALSE;
		}
		DWORD len = MultiByteToWideChar(CP_UTF8, 0, xmlString.c_str(), -1, NULL, 0);
		newRule->Sid = (PIGNIS_STRING)malloc(sizeof(IGNIS_STRING) + (len - 1) * sizeof(WCHAR));
		memset(newRule->Sid, 0, sizeof(IGNIS_STRING) + (len - 1) * sizeof(WCHAR));
		MultiByteToWideChar(CP_UTF8, 0, xmlString.c_str() , -1, newRule->Sid->Buffer, len);
		newRule->Sid->Length = len * sizeof(WCHAR);
		newRule->Flags |= RULE_CHECK_SID;
	}

	// Check if the protocol attribute exists
	if (pXmlRule->ToElement()->FirstChild("remotePorts"))
	{
		// RemotePorts
		TiXmlNode * pXmlRemotePorts = pXmlRule->FirstChild("remotePorts");
		TiXmlNode * pXmlRemotePort = NULL;
		// Take the number of the remotePorts
		int nrOfRemotePorts = TinyXPath::i_xpath_int(pXmlRemotePorts, "count(/remotePorts/remotePort)");
		newRule->RemotePortRanges = (PPORT_RANGE)malloc(sizeof(PORT_RANGE) * nrOfRemotePorts);
		newRule->RemotePortRangesCount = nrOfRemotePorts; 
		// Read all the remotePort nodes
		while ((pXmlRemotePort = pXmlRemotePorts->IterateChildren("remotePort", pXmlRemotePort)) && nrOfRemotePorts > 0)
		{
			// Decrease the index for the array
			nrOfRemotePorts--;
			bRes = GetXMLInt(pXmlRemotePort, "@Min", value, &strValue);
			if (bRes == FALSE)
			{
				AddLogEntryEx(ZEROLEVEL, L"### Invalid remote ports minimum value; value: %s", strValue);
				return FALSE;
			}
			if (value < 0)
			{
				AddLogEntryEx(ZEROLEVEL, L"### Remote ports values must be > 0; provided: %d", value);
				return FALSE;
			}
			newRule->RemotePortRanges[nrOfRemotePorts].Min = (WORD)value;
			bRes = GetXMLInt(pXmlRemotePort, "@Max", value, &strValue);
			if (bRes == FALSE)
			{
				AddLogEntryEx(ZEROLEVEL, L"Invalid remote ports maximum value; value: %s", strValue);
				return FALSE;
			}
			if (value < 0)
			{
				AddLogEntryEx(ZEROLEVEL, L"Remote ports values must be > 0; provided: %d", value);
				return FALSE;
			}
			newRule->RemotePortRanges[nrOfRemotePorts].Max = (WORD)value;
		}
		newRule->Flags |= RULE_CHECK_REM_PORT;
	}

	// Check if the localIp attribute exists
	if (pXmlRule->ToElement()->Attribute("localIp"))
	{
		// IpVersion
		// Check if the ipVersion is correct
		if (newRule->IpVersion == ipVersionAny)
		{
			AddLogEntryEx(ZEROLEVEL, L"Local network is incompatible with ipVersionAny. Please choose an ip version or delete the localIp attribute");
			return FALSE;
		}
		bRes = TinyXPath::o_xpath_string(pXmlRule, "@localIp", xmlString);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"localIp attribute not found in rule");
			return FALSE;
		}
		bRes = ReadNetwork(&newRule->LocalNetwork, newRule->IpVersion, xmlString.c_str());
		if (bRes == FALSE)
		{
			AddLogEntryEx(SECONDLEVEL, L"Could not read local ip");
			return FALSE;
		}
		newRule->Flags |= RULE_CHECK_LOCAL_NETWORK;
	}

	// Check if the remoteNetworks node exists
	if (pXmlRule->ToElement()->FirstChild("remoteNetworks"))
	{
		// RemoteNetworks
		// Check if the ipVersion is correct
		if (newRule->IpVersion == ipVersionAny)
		{
			AddLogEntryEx(ZEROLEVEL, L"Remote networks are incompatible with ipVersionAny. Please choose an ip version or delete the remoteNetworks node");
			return FALSE;
		}
		TiXmlNode * pXmlRemoteNetworks = pXmlRule->FirstChild("remoteNetworks");
		TiXmlNode * pXmlRemoteNetwork = NULL;
		// Take the number of the remoteNetwork nodes
		int nrOfRemoteNetworks = TinyXPath::i_xpath_int(pXmlRemoteNetworks, "count(/remoteNetworks/remoteNetwork)");
		newRule->RemoteNetworks = (PIGNIS_IP)malloc(sizeof(IGNIS_IP) * nrOfRemoteNetworks);
		newRule->RemoteNetworkCount = nrOfRemoteNetworks;
		// Read all the remoteNetwork nodes
		while ((pXmlRemoteNetwork = pXmlRemoteNetworks->IterateChildren("remoteNetwork", pXmlRemoteNetwork)) && nrOfRemoteNetworks > 0)
		{
			// Decrease the index for the array
			nrOfRemoteNetworks--;
			bRes = TinyXPath::o_xpath_string(pXmlRemoteNetwork, "@remoteIp", xmlString);
			if (bRes == FALSE)
			{
				AddLogEntryEx(ZEROLEVEL, L"RemoteIp attribute not found in remoteNetwork");
				return FALSE;
			}
			bRes = ReadNetwork(&newRule->RemoteNetworks[nrOfRemoteNetworks], newRule->IpVersion, xmlString.c_str());
			if (bRes == FALSE)
			{
				AddLogEntryEx(ZEROLEVEL, L"Could not read remote ip");
				return FALSE;
			}
		}
		newRule->Flags |= RULE_CHECK_REM_NETWORK;
	}

	// Check if the process node exists
	if (pXmlRule->ToElement()->FirstChild("process"))
	{
		// Process 
		TiXmlNode * pXmlProcess = pXmlRule->FirstChild("process");
		bRes = TinyXPath::o_xpath_string(pXmlProcess, "@path", xmlString);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"Path attribute not found in process");
			return FALSE;
		}

		bRes = ReadPath(newRule, xmlString.c_str());
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"Could not read path");
			return FALSE;
		}

		// Check if both md5Value and md5Valid attributes exist
		if (pXmlProcess->ToElement()->Attribute("md5Value") && pXmlProcess->ToElement()->Attribute("md5Valid"))
		{
			bRes = GetXMLInt(pXmlProcess, "@md5Valid", value, &strValue);
			if (bRes == FALSE)
			{
				AddLogEntryEx(ZEROLEVEL, L"Invalid md5Valid value; value: %s", strValue);
				return FALSE;
			}
			if (value < 0 || value > 2)
			{
				AddLogEntryEx(ZEROLEVEL, L"Md5Valid : 0 - invalid 1 - valid : value : %d", value);
				return FALSE;
			}
			// MD5
			if (value)
			{
				bRes = TinyXPath::o_xpath_string(pXmlProcess, "@md5Value", xmlString);
				if (bRes == FALSE)
				{
					AddLogEntryEx(ZEROLEVEL, L"Md5Value attribute not found in process");
					return FALSE;
				}
				bRes = ReadMD5(newRule->Process, xmlString.c_str());
				if (bRes == FALSE)
				{
					AddLogEntryEx(ZEROLEVEL, L"Could not read md5");
					return FALSE;
				}
				newRule->Process->Md5Valid = value;
				newRule->Flags |= RULE_CHECK_MD5;
			}
		}
		newRule->Flags |= RULE_CHECK_PROCESS;
	}
	return TRUE;
}

int DeleteAllRulesFromXml(
void
)
{
	TiXmlElement * pRulesRoot = gXmlRoot->FirstChildElement("rules");
	if (pRulesRoot == NULL)
	{
		AddLogEntryEx(ZEROLEVEL, L"Could not retrieve the rules node");
		return -1;
	}
	// Clear the rules node
	pRulesRoot->Clear();
	return 0;
}

int DeleteRuleFromXml(
	__in QWORD ruleId
)
{
	TiXmlElement * pRulesRoot = gXmlRoot->FirstChildElement("rules");
	if (pRulesRoot == NULL)
	{
		AddLogEntryEx(ZEROLEVEL, L"Could not retrieve the rules node");
		return -1;
	}
	TiXmlNode * pXmlRule = NULL;
	//Search through all the rules
	while (pXmlRule = pRulesRoot->IterateChildren("rule", pXmlRule))
	{
		BOOL bRes;
		QWORD value;
		TiXmlString xmlString;
		bRes = TinyXPath::o_xpath_string(pXmlRule, "@id", xmlString);
		if (bRes == FALSE)
		{
			AddLogEntryEx(ZEROLEVEL, L"Id attribute was not found in rule node");
			return FALSE;
		}
		if (sscanf_s(xmlString.c_str(), "%llu", &value) != 1)
		{
			AddLogEntryEx(ZEROLEVEL, L"Wrong format for rule id");
			return FALSE;
		}

		// If the rule is found remove the child 
		if(value == ruleId)
		{
			bRes = pRulesRoot->RemoveChild(pXmlRule);
			if (bRes == FALSE)
			{
				AddLogEntryEx(SECONDLEVEL, L"Error deleting rule from xml");
				return -1;
			}
			else
			{
				AddLogEntryEx(ZEROLEVEL, L"Rule %d deleted from xml", ruleId);
				break;
			}
		}
	}
	return 0;
}

int PrintXml(
	__in char* xmlFileName
)
{
	// If the parameter is NULL, print the xml to the console otherwise print the xml in a file
	if (xmlFileName == NULL)
	{
		TiXmlPrinter xmlPrinter;
		gXmlRoot->Accept(&xmlPrinter);
		AddLogEntryEx(ZEROLEVEL, L"%s", xmlPrinter.CStr());
	}
	else
	{
		FILE * xmlFile;
		errno_t err = fopen_s(&xmlFile, xmlFileName, "w");
		if (err)
		{
			AddLogEntryEx(SECONDLEVEL, L"Could not open file %s", xmlFileName);
			return -1;
		}
		fprintf_s(xmlFile, "%s", "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
		gXmlRoot->Print(xmlFile, 0);
		if (ferror(xmlFile))
		{
			AddLogEntryEx(SECONDLEVEL, L"Could not print in the xml file %s", xmlFileName);
			fclose(xmlFile);
			return -1;
		}
		fclose(xmlFile);
	}
	return 0;
}

int AddRulesFromXml(
void
)
{
	PTSTATUS status = STATUS_SUCCESS;
	TiXmlNode *pXmlRule = NULL;
	TiXmlElement * pRulesRoot = gXmlRoot->FirstChildElement("rules");
	if (pRulesRoot == NULL)
	{
		AddLogEntryEx(ZEROLEVEL, L"Could not retrieve the rules node");
		return -1;
	}
	// Read all the rules
	while (pXmlRule = pRulesRoot->IterateChildren("rule", pXmlRule))
	{
		IGNIS_RULE newRule = { 0 };
		ZeroMemory(&newRule, sizeof(IGNIS_RULE));
		// Read the rule from xml
		if (ReadRule(&newRule, pXmlRule) == FALSE)
		{
			AddLogEntryEx(SECONDLEVEL, L"[ERROR]: Cannot read rule in the configuration file.");
			return -1;
		}

		// Add the rule in ignis
		status = PubIgnisAddRule(&newRule);
		
		if (!PT_SUCCESS(status))
		{
			AddLogEntryEx(SECONDLEVEL, L"[ERROR]: Error adding ignis rule! status=0x%x", status);
			pXmlRule->ToElement()->SetAttribute("id", -1); // Set id -1 for all invalid rules 
			continue;
		}
		else
		{
			// Set the rule id for the added rule
			char ruleId[20];
			sprintf_s(ruleId, 20, "%llu", newRule.RuleId);
			pXmlRule->ToElement()->SetAttribute("id", ruleId);
			AddLogEntryEx(ZEROLEVEL, L"Rule added successfully Id : %s", ruleId);
		}
	}
	
	return 0;
}