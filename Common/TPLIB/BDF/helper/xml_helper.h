#pragma once
#include "xpath_static.h"

/** 
	File: xml_helper.h
	Description: Contains the helper functions needed for manipulating the xml
*/

/**
	Gets the integer value situated at a specified path starting from pNode 
*/
BOOL GetXMLInt(
	__in const TiXmlNode *pNode, 
	__in const char *path, 
	__out int &rValue, 
	__out const char **pszFoundValue
);

/** 
	Reads the ip address in a specific format and stores it in network 
*/
BOOL  ReadNetwork(
	__inout PIGNIS_IP network,
	__in IP_VERSION ipVersion,
	__in const char * text
);

/** 
	Reads the process path and stores it in the rule 
*/
BOOL ReadPath(
	__inout PIGNIS_RULE rule,
	__in const char * path
);

/** 
	Reads the MD5 value in a specific format and stores it in the processInfo
*/
BOOL  ReadMD5(
	__inout PPROCESS_INFO processInfo,
	__in const char * text
);

/** 
	Deletes all the rules from xml
*/
int DeleteAllRulesFromXml(
void 
);

/**
	Deletes from xml the rule with the specified ruleId
*/
int DeleteRuleFromXml(
	__in QWORD ruleId
);

/**
	Inserts a new rule in the xml 
*/
int InsertRuleToXml(
	__in PIGNIS_RULE rule
);


/**
	Updates the firewall settings in the xml
*/
int UpdateSettingsInXml(
	__in IGNIS_SETTINGS & settings
);

/**
	Inserts a new rule in the xml with information collected from traffic info
*/
int InsertRuleToXmlFromTrafficInfo(
	__in PTRAFFIC_INFO trafficInfo, 
	__in int action, 
	__in QWORD flags, 
	__in QWORD ruleId
);

/**
	Prints the entire xml either to the console or to a file
*/
int PrintXml(
	__in char* xmlFileName
);

/**
	Adds all the rules from xml in ignis
*/
int AddRulesFromXml(
void
);