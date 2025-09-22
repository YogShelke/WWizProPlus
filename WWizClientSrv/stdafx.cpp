#include "stdafx.h"
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

//CWWizSettings theApp;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using boost::property_tree::ptree;

#define LOG_FILE					_T("WWIZCLIENTSRV.LOG")

/**********************************************************************************************************
*  Function Name  :	AddLogEntry
*  Description    :	Add log entry into respective log files
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1, const TCHAR *sEntry2,
	bool isDateTime, DWORD dwLogLevel)
{
	try
	{
		static CString csScanLogFullPath;
		FILE *pRtktOutFile = NULL;

		if (csScanLogFullPath.GetLength() == 0)
		{
			TCHAR szModulePath[MAX_PATH] = { 0 };
			memset(szModulePath, 0x00, MAX_PATH * sizeof(TCHAR));
			if (!GetModulePath(szModulePath, MAX_PATH))
			{
				//MessageBox(NULL, L"Error in GetModulePath", L"WardWiz", MB_ICONERROR);
				return;
			}

			//ISSUE : Log for WARDWIZUI is getting created at incorrect path.
			//NAME - Niranjan Deshak. - 23th Jan 2015
			csScanLogFullPath = szModulePath;
			csScanLogFullPath += L"\\Log\\";
			if (!PathFileExists(csScanLogFullPath))
			{
				CreateDirectory(csScanLogFullPath, NULL);
			}
			csScanLogFullPath += LOG_FILE;
		}

		if (!pRtktOutFile)
			pRtktOutFile = _wfsopen(csScanLogFullPath, _T("a"), 0x40);

		if (pRtktOutFile != NULL)
		{
			CString szMessage;
			if (sFormatString && sEntry1 && sEntry2)
				szMessage.Format(sFormatString, sEntry1, sEntry2);
			else if (sFormatString && sEntry1)
				szMessage.Format(sFormatString, sEntry1);
			else if (sFormatString && sEntry2)
				szMessage.Format(sFormatString, sEntry2);
			else if (sFormatString)
				szMessage = sFormatString;

			if (isDateTime)
			{
				TCHAR tbuffer[9] = { 0 };
				TCHAR dbuffer[9] = { 0 };
				_wstrtime_s(tbuffer, 9);
				_wstrdate_s(dbuffer, 9);

				CString szOutMessage;
				szOutMessage.Format(_T("[%s %s] [WWIZDEPLOYERSRV] %s\r\n"), dbuffer, tbuffer, static_cast<LPCTSTR>(szMessage));
				fputws((LPCTSTR)szOutMessage, pRtktOutFile);
			}
			else
			{
				fputws((LPCTSTR)szMessage, pRtktOutFile);
			}
			fflush(pRtktOutFile);
			fclose(pRtktOutFile);
		}
	}
	catch (...)
	{
		AddLogEntry(L"###WRDWIZCLIENTAGENT : Error in AddLogEntry", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	GetModulePath
*  Description    :	Get path where module exist
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool GetModulePath(TCHAR *szModulePath, DWORD dwSize)
{
	if (0 == GetModuleFileName(NULL, szModulePath, dwSize))
	{
		return false;
	}

	if (_tcsrchr(szModulePath, _T('\\')))
	{
		*_tcsrchr(szModulePath, _T('\\')) = 0;
	}
	return true;
}

bool AddErrorServerLog(LPTSTR lpszTaskID, LPTSTR lpszRemoteIP, PWCH Message, ...)
{
	bool bReturn = false;
	try
	{
		bReturn = true;

		//Functionality disabled as it fails we retry using Tray application.
		/*
		va_list MessageList;

		TCHAR szMessageBuffer[2048] = { 0 };
		//
		// First, combine the message
		//
		va_start(MessageList, Message);
		_vsnwprintf(szMessageBuffer, 2048, Message, MessageList);
		va_end(MessageList);

		std::vector<web::json::value> arrayMachines;
		web::json::value JsonData;

		CString csMachineID = CWWizSettings::GetMachineID();
		JsonData[L"MachineID"] = json::value::string(lpszRemoteIP);
		JsonData[L"TaskID"] = json::value::string(lpszTaskID);
		JsonData[L"Error"] = json::value::string(szMessageBuffer);
		arrayMachines.push_back(JsonData);

		json::value lValue;
		lValue = web::json::value::array(arrayMachines);

		utility::string_t Lreq = L"=" + lValue.to_string();

		CString csErrorlogAPI;
		csErrorlogAPI.Format(L"http://%s:%s/Wardwizeps/api/V001/ErrorLog/ErrorInfo", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
		http_client client(csErrorlogAPI.GetBuffer());

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
						AddLogEntry(L"### Exception in AddErrorServerLog, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
					}
					catch (...)
					{
						AddLogEntry(L"### Exception in AddErrorServerLog", 0, 0, true, SECONDLEVEL);
						return false;
					}
					return true;
				});
			}
			return false;
		});
		*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in AddErrorServerLog", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool SendFailureLog(LPTSTR lpszTaskID, LPTSTR lpszRemoteIP, PWCH Message, ...)
{
	bool bReturn = false;
	try
	{
		//Functionality disabled as it fails we retry using Tray application.
		va_list MessageList;

		TCHAR szMessageBuffer[2048] = { 0 };
		//
		// First, combine the message
		//
		va_start(MessageList, Message);
		_vsnwprintf(szMessageBuffer, 2048, Message, MessageList);
		va_end(MessageList);

		std::vector<web::json::value> arrayMachines;
		web::json::value JsonData;

		CString csMachineID = CWWizSettings::GetMachineID();
		JsonData[L"MachineID"] = json::value::string(lpszRemoteIP);
		JsonData[L"TaskID"] = json::value::string(lpszTaskID);
		JsonData[L"Error"] = json::value::string(szMessageBuffer);
		JsonData[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

		arrayMachines.push_back(JsonData);

		json::value lValue;
		lValue = web::json::value::array(arrayMachines);

		utility::string_t Lreq = L"=" + lValue.to_string();

		CString csErrorlogAPI;
		csErrorlogAPI.Format(L"http://%s:%s/api/V001/ErrorLog/ErrorInfo", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
		OutputDebugString(csErrorlogAPI);
		http_client client(csErrorlogAPI.GetBuffer());

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
						AddLogEntry(L"### Exception in AddErrorServerLog, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
					}
					catch (...)
					{
						AddLogEntry(L"### Exception in AddErrorServerLog", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in AddErrorServerLog", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : GetWardWizPathFromRegistry
Description    : function returns wardwiz app folder path from registry
Author Name    : Ram Shelke
Date           : 25th June 2014
****************************************************************************/
CString GetWardWizPathFromRegistry()
{
	HKEY	hSubKey = NULL;
	TCHAR	szModulePath[MAX_PATH] = { 0 };

	if (RegOpenKey(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), &hSubKey) != ERROR_SUCCESS)
		return L"";

	DWORD	dwSize = 511;
	DWORD	dwType = 0x00;

	RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize);
	RegCloseKey(hSubKey);
	hSubKey = NULL;

	if (_tcslen(szModulePath) > 0)
	{
		return CString(szModulePath);
	}
	return L"";
}