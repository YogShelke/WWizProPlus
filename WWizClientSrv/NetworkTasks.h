#pragma once
#include "stdafx.h"
#include "CommonFunctions.h"
#include "Connections.h"
#include "WWizSettings.h"
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include "CommonFunctions.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using boost::property_tree::ptree;

class CNetworkTasks
{
public:
	CNetworkTasks();
	~CNetworkTasks();
	vector<STRUCT_MACHINE_INFO> CNetworkTasks::DignoseNetwork(string strStartIP, string strEndIP, bool biSDeepScan = false);
	CString  GetComputerName(CString IpAddress);
	bool SendData(std::string stdJsonData);
	bool SendData(std::vector<web::json::value> &IPList);
	bool SendDataSEH(std::vector<web::json::value> &IPList);
	bool CheckISAnyTaskforNetworkDignose();
	void StopRunningTasks();
	void ReInitializeVariables();
	bool PingNetworkIP(LPTSTR lpszIP);
	bool PingNetworkIPSEH(LPTSTR lpszIP);
private:
	vector<string> SplitIP(string& str, char delim);
	bool			m_bStop;
	CString			m_csMachineID;
};

