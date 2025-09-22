#pragma once
#include "stdafx.h"
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include "CommonFunctions.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define				IDR_REGDATA					2000
#define				IDR_QUARDATA				2001

using namespace web;
using namespace web::http;
using namespace web::http::client;
using boost::property_tree::ptree;

class CWWizRegistration
{
private:
	DWORD		m_dwDaysLeft;
	CString		m_csMachineID;
public:
	bool		m_bISRegistered;
public:
	CWWizRegistration();
	virtual ~CWWizRegistration();
	bool CheckRegistrationStatus();
	bool RequestRegistrationInfo();
	bool AddProdRegInfoToLocal(DWORD dwType, AVACTIVATIONINFO	ActInfo, DWORD dwResSize, DWORD dwResType, TCHAR *pResName, bool bRegWait);
	bool SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName, bool bRegWait);
	DWORD GetDaysLeft();
	bool IsWardWizRegistered();
	DWORD AddRegistrationDataInRegistry(AVACTIVATIONINFO pActInfo);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPBYTE byData, DWORD dwLen, bool bWait);
	pplx::task<void> GetRegistrationInfo();
	bool ParseRegistrationResponse(json::value& v);
};

