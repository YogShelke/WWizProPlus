#pragma once
#include "json_writer.h"
#include "json_reader.h"
#include "json_data.h"
#include "CommonFunctions.h"
#include "Connections.h"

class CNetworkTasks
{
public:
	CNetworkTasks();
	~CNetworkTasks();
	vector<STRUCT_MACHINE_INFO> CNetworkTasks::DignoseNetwork(string strStartIP, string strEndIP);
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
	bool m_bStop;
};

