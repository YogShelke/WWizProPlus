#include "stdafx.h"
#include "NetworkTasks.h"
#include "ping.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
using boost::property_tree::ptree;

using namespace web;

CNetworkTasks::CNetworkTasks():
m_bStop(false)
{
	m_csMachineID = CWWizSettings::GetMachineID();
}


CNetworkTasks::~CNetworkTasks()
{
}

bool CNetworkTasks::CheckISAnyTaskforNetworkDignose()
{
	bool bReturn = false;
	/*try
	{
		RestClient::init();
	}
	catch(...)	
	{

	}
*/
	//web::json::value JsonData;
	//JsonData[L"MachineID"] = json::value::string(L"");
	//JsonData[L"IP"] = json::value::string(L"");

	//// get a connection object
	//RestClient::Connection* conn = new RestClient::Connection("http://wssr0011");

	//// set headers
	//RestClient::HeaderFields headers;
	//headers["Content-Type"] = "application/x-www-form-urlencoded";
	//conn->SetHeaders(headers);

	//// append additional headers
	//std::string stdAuth = "bearer ";
	//stdAuth.append(CW2A(CWWizSettings::m_csBearerCode.GetBuffer()));
	//conn->AppendHeader("Authorization", stdAuth);

	//RestClient::Response r = conn->post("WardwizEPS/api/Test/Mock_response", "=" + CClientAgentCommonFunctions::get_string_from_wsz(JsonData.to_string().c_str()));
	//RestClient::disable();

	//// try/catch omitted for brevity
	//// The JSON enabled class as above
	//JSONDATA source;
	//source.text = r.body;
	//std::string json = JSON::producer<JSONDATA>::convert(source);

	return bReturn;
}

vector<STRUCT_MACHINE_INFO> CNetworkTasks::DignoseNetwork(string strStartIP, string strEndIP, bool bISDeepScan)
{
	OutputDebugString(L">>> In CNetworkTasks::DignoseNetwork");
	OutputDebugStringA(strStartIP.c_str());
	OutputDebugStringA(strEndIP.c_str());

	vector<STRUCT_MACHINE_INFO> IPList;
	try
	{
		CSocketComm m_SocketManager;
		CString strLocal;
		m_SocketManager.GetLocalAddress(strLocal.GetBuffer(256), 256);

		vector<string> startIPList = SplitIP(strStartIP, '.');
		vector<string> EndIPList = SplitIP(strEndIP, '.');

		if (startIPList.size() != 4)
		{
			return IPList;
		}

		if (EndIPList.size() != 4)
		{
			return IPList;
		}

		std::vector<web::json::value> arrayMachines;

		for (int iIndex = stoi(startIPList.at(0)); iIndex <= stoi(EndIPList.at(0)); iIndex++)
		{
			if (m_bStop)
				break;

			for (int jIndex = stoi(startIPList.at(1)); jIndex <= stoi(EndIPList.at(1)); jIndex++)
			{
				if (m_bStop)
					break;

				for (int kIndex = stoi(startIPList.at(2)); kIndex <= stoi(EndIPList.at(2)); kIndex++)
				{
					if (m_bStop)
						break;

					for (int lIndex = stoi(startIPList.at(3)); lIndex <= stoi(EndIPList.at(3)); lIndex++)
					{
						if (m_bStop)
							break;

						CString csIP;
						csIP.Format(L"%d.%d.%d.%d", iIndex, jIndex, kIndex, lIndex);
						OutputDebugString(csIP);

						if (bISDeepScan || PingNetworkIP(csIP.GetBuffer()))
						{
							STRUCT_MACHINE_INFO szMachineInfo = { 0 };
							_tcscpy(szMachineInfo.szIP, csIP);
							_tcscpy(szMachineInfo.szName, GetComputerName(csIP));
							IPList.push_back(szMachineInfo);

							web::json::value JsonData;
							JsonData[L"Name"] = json::value::string(szMachineInfo.szName);
							JsonData[L"IP"] = json::value::string(szMachineInfo.szIP);
							JsonData[L"OSName"] = json::value::string(L"");
							JsonData[L"RMachineIP"] = json::value::string(strLocal.GetBuffer());
							JsonData[L"RMachineID"] = json::value::string(m_csMachineID.GetBuffer());
							JsonData[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

							arrayMachines.push_back(JsonData);

							static int i = 0x00;
							if (i % 5 == 0x00)
							{
								SendData(arrayMachines);
								i = 0x00;
								arrayMachines.clear();
							}
							i++;
						}

						Sleep(100);
					}
				}
			}
		}

		if (arrayMachines.size() != 0)
			SendData(arrayMachines);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNetworkTasks::DignoseNetwork", 0, 0, true, SECONDLEVEL);
	}
	return IPList;
}

vector<string> CNetworkTasks::SplitIP(string& str, char delim)
{
	auto i = 0x00;
	vector<string> list;

	auto pos = str.find(delim);

	while (pos != string::npos)
	{
		list.push_back(str.substr(i, pos - i));
		i = ++pos;
		pos = str.find(delim, pos);
	}

	list.push_back(str.substr(i, str.length()));

	return list;
}

CString CNetworkTasks::GetComputerName(CString IpAddress)
{
	// Declare and initialize variables
	WSADATA wsaData = { 0 };
	int iResult = 0;

	DWORD dwRetval;

	struct sockaddr_in saGNI;
	char hostname[NI_MAXHOST];
	char servInfo[NI_MAXSERV];
	u_short port = 27015;


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return _T("");
	}

	//-----------------------------------------
	// Set up sockaddr_in structure which is passed
	// to the getnameinfo function
	saGNI.sin_family = AF_INET;
	saGNI.sin_addr.s_addr = inet_addr(CW2A(IpAddress));
	saGNI.sin_port = htons(port);

	//-----------------------------------------
	// Call getnameinfo
	dwRetval = getnameinfo((struct sockaddr *) &saGNI,
		sizeof (struct sockaddr),
		hostname,
		NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);

	if (dwRetval != 0) {
		WSACleanup();
		return _T("");
	}
	else {
		WSACleanup();
		return CString(CA2W(hostname));
	}

	return _T("");
}

bool CNetworkTasks::SendData(std::vector<web::json::value> &IPList)
{
	OutputDebugString(L">>> In CNetworkTasks::SendData");

	bool bReturn = false;
	__try
	{
		if (m_bStop)
			bReturn;
	
		// initialize RestClient
		//RestClient::init();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	__try
	{
		SendDataSEH(IPList);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CNetworkTasks::SendData", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CNetworkTasks::SendDataSEH(std::vector<web::json::value> &IPList)
{
	bool bReturn = false;
	try
	{
		json::value JsonData;
		JsonData = web::json::value::array(IPList);

		utility::string_t Lreq = L"=" + JsonData.to_string();

		CString csDiscoverNetworkApi;
		csDiscoverNetworkApi.Format(L"http://%s:%s/api/V001/DiscoverNetwork", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
		OutputDebugString(csDiscoverNetworkApi);
		http_client client(csDiscoverNetworkApi.GetBuffer());

		// Manually build up an HTTP request with header and request URI.
		http_request request(methods::POST);
		utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
		request.headers().add(L"Authorization", auth);
		request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
		request.set_body(Lreq);

		client.request(request).then([](http_response response)
		{
			if (response.status_code() == status_codes::OK)
			{
				response.extract_json().then([](json::value jsonValue)
				{
					try
					{
						utility::string_t str = jsonValue.to_string();
						if (_tcscmp(str.c_str(), L"null") == 0)
						{
							return false;
						}

						std::string stringvalues = CW2A(str.c_str());
						std::istringstream iss(stringvalues);
						// Read json.
						ptree pt;
						read_json(iss, pt);

						std::size_t found = str.find(L"Result");
						if (found != std::string::npos)
						{
							for (auto& challenge : pt.get_child("Result"))
							{
								for (auto& prop : challenge.second)
								{
									if (strcmp(prop.first.c_str(), "1") == 0)
									{
										return true;
									}
								}
								return false;
							}
						}
					}
					catch (const http_exception& e)
					{
						// Print error.
						wostringstream ss;
						ss << e.what() << endl;
						return false;
						AddLogEntry(L"### Exception in CNetworkTasks::SendDataSEH, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
					}
					catch (...)
					{
						AddLogEntry(L"### Exception in CNetworkTasks::SendDataSEH", 0, 0, true, SECONDLEVEL);
						return false;
					}
					return true;
				});
			}
			return false;
		});
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::CNetworkTasks::SendDataSEH", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

void CNetworkTasks::StopRunningTasks()
{
	m_bStop = true;
}


void CNetworkTasks::ReInitializeVariables()
{
	m_bStop = false;
}

bool CNetworkTasks::PingNetworkIP(LPTSTR lpszIP)
{
	__try
	{
		if (!lpszIP)
			return false;

		return PingNetworkIPSEH(lpszIP);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CNetworkTasks::PingNetworkIP", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

bool CNetworkTasks::PingNetworkIPSEH(LPTSTR lpszIP)
{
	if (!lpszIP)
		return false;

	CPing p1;
	CPingReply pr1;
	if (p1.Ping1(lpszIP, pr1, '\n', 1000))
	{
		return true;
	}

	return false;
}
