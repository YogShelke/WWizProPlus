#include "WWizRegistration.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace web;
using boost::property_tree::ptree;

CWWizRegistration::CWWizRegistration()
: m_dwDaysLeft(0x00)
, m_bISRegistered(false)
{
	m_csMachineID = CWWizSettings::GetMachineID();
}

CWWizRegistration::~CWWizRegistration()
{
}

bool CWWizRegistration::IsWardWizRegistered()
{
	m_dwDaysLeft = GetDaysLeft();
	if (m_dwDaysLeft == 0x00)
	{
		return false;
	}
	return true;
}

bool CWWizRegistration::CheckRegistrationStatus()
{
	bool bReturn = false;
	__try
	{
		bReturn = RequestRegistrationInfo();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWWizRegistration::CheckRegistrationStatus", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

// Retrieves a JSON value from an HTTP request.
bool CWWizRegistration::RequestRegistrationInfo()
{
	bool bReturn = false;
	try
	{
		m_bISRegistered = false;
		GetRegistrationInfo().wait();
		bReturn = true;
	}
	catch (exception const & e)
	{
		CString strError = CString(e.what());
		AddLogEntry(L"### Exception in CWardwizClientAgent::RequestRegistrationInfo : %s", strError, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::RequestRegistrationInfo", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : AddProdRegInfoToLocal
*  Description    : Adding product registration information locally.
*  Author Name    : Amol Jaware
*  Date			  : 22 June 2017
****************************************************************************************************/
bool CWWizRegistration::AddProdRegInfoToLocal(DWORD dwType, AVACTIVATIONINFO	ActInfo, DWORD dwResSize, DWORD dwResType, TCHAR *pResName, bool bRegWait)
{
	bool bRegFailed = false;
	try
	{
		if (!SendRegisteredData2Service(ADD_REGISTRATION_DATA, (LPBYTE)&ActInfo, sizeof(ActInfo), IDR_REGDATA, TEXT("REGDATA"), false))
		{
			AddLogEntry(L"### Failed to SendRegisteredData2Service in CRegistrationDlg AddProdRegInfoToLocal", 0, 0, true, SECONDLEVEL);
			bRegFailed = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizRegistration::AddProdRegInfoToLocal", 0, 0, true, SECONDLEVEL);
	}
	return bRegFailed;
}

/***************************************************************************************************
*  Function Name  : SendRegisteredData2Service
*  Description    : Send Data to servive
*  Author Name    : Nitin Kolapkar
*  Date			  : 27 May 2016
****************************************************************************************************/
bool CWWizRegistration::SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName, bool bRegWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = dwType;
		memcpy(szPipeData.byData, lpResBuffer, dwResSize);
		szPipeData.dwValue = dwResSize;
		szPipeData.dwSecondValue = dwResType;
		wcscpy_s(szPipeData.szFirstParam, pResName);

		CISpyCommunicator objCom(SERVICE_SERVER, true, 3);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWWizRegistration::SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bRegWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CWWizRegistration::SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}

			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWWizRegistration::SendRegisteredData2Service");
	}
	return true;
}

/***********************************************************************************************
Function Name  : GetDaysLeft
Description    : Function to get days left
SR.NO		   : 
Author Name    : Ramkrushna Shelke
Date           : 20 Jan 2014
***********************************************************************************************/
DWORD CWWizRegistration::GetDaysLeft()
{
	OutputDebugString(L">>> In CWWizRegistration::GetDaysLeft");

	DWORD dwRet = 0x00;
	__try
	{
	/*	typedef DWORD(*GETDAYSLEFT)		(DWORD);

		GETDAYSLEFT	GetDaysLeft = NULL;

		TCHAR	szTemp[512] = { 0 };

		GetModuleFileName(NULL, szTemp, 511);

		TCHAR	*pTemp = wcsrchr(szTemp, '\\');
		if (pTemp)
		{
			pTemp++;
			*pTemp = '\0';
		}

		wcscat(szTemp, L"WRDWIZREGISTRATION.DLL");
		if (!PathFileExists(szTemp))
		{
			OutputDebugString(L"### File: [%s] Does not exists");
			OutputDebugString(szTemp);
			AddLogEntry(L"### File: [%s] Does not exists", szTemp, 0, true, SECONDLEVEL);
			return dwRet;
		}

		OutputDebugString(L">>> Before LoadLibrary");
		HMODULE	hMod = LoadLibrary(szTemp);

		OutputDebugString(L">>> Before GetProcAddress");
		if (hMod)
			GetDaysLeft = (GETDAYSLEFT)GetProcAddress(hMod, "GetDaysLeft");

		if (GetDaysLeft)
			dwRet = GetDaysLeft(0x05);

		if (hMod)
		{
			OutputDebugString(L">>> Before FreeLibrary");
			FreeLibrary(hMod);
			hMod = NULL;
		}*/
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in WriteNumberOfDays", 0, 0, true, SECONDLEVEL);
	}

	return dwRet;
}

DWORD CWWizRegistration::AddRegistrationDataInRegistry(AVACTIVATIONINFO pActInfo)
{
	DWORD	dwRet = 0x00;
	try
	{

		DWORD	dwRegType = 0x00, dwRetSize = 0x00;

		HKEY	h_iSpyAV = NULL;
		HKEY	h_iSpyAV_User = NULL;

		AVACTIVATIONINFO	ActInfo = { 0 };

		memcpy(&ActInfo, &pActInfo, sizeof(ActInfo));

		if (DecryptData((LPBYTE)&ActInfo, sizeof(ActInfo)))
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		if (!SendRegistryData2Service(SZ_BINARY, L"SOFTWARE\\Microsoft\\Windows", L"WardWizUserInfo", (LPBYTE)&ActInfo, sizeof(ActInfo), false))
		{
			dwRet = 0x05;
			AddLogEntry(L"### Failed to send SendRegistryData2Service in CRegistrationSecondDlg::AddRegistrationDataInRegistry", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::AddRegistrationDataInRegistry");
	}
Cleanup:
	return dwRet;
}

DWORD CWWizRegistration::DecryptData(LPBYTE lpBuffer, DWORD dwSize)
{
	try
	{
		if (IsBadWritePtr(lpBuffer, dwSize))
			return 1;

		DWORD	iIndex = 0;
		DWORD jIndex = 0;

		if (lpBuffer == NULL || dwSize == 0x00)
		{
			return 1;
		}

		for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
		{
			if (lpBuffer[iIndex] != 0)
			{
				if ((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
				{
					lpBuffer[iIndex] = lpBuffer[iIndex];
				}
				else
				{
					lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
					jIndex++;
				}
				if (jIndex == WRDWIZ_KEYSIZE)
				{
					jIndex = 0x00;
				}
				if (iIndex >= dwSize)
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::DecryptData");
	}
	return 0;
}

bool CWWizRegistration::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPBYTE byData, DWORD dwLen, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = dwType;
		wcscpy_s(szPipeData.szFirstParam, szKey);
		wcscpy_s(szPipeData.szSecondParam, szValue);
		memcpy(szPipeData.byData, byData, dwLen);
		szPipeData.dwSecondValue = dwLen;

		CISpyCommunicator objCom(SERVICE_SERVER, true, 3);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CRegistrationSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}
			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistrationDlg::SendRegistryData2Service");
	}
	return true;
}

// Retrieves a JSON value from an HTTP request.
pplx::task<void> CWWizRegistration::GetRegistrationInfo()
{
	OutputDebugString(L">>> In CWardwizClientAgent::GetTaskID");

	CSocketComm m_SocketManager;
	CString strLocal;
	m_SocketManager.GetLocalAddress(strLocal.GetBuffer(256), 256);

	json::value lValue;
	lValue[L"MachineID"] = json::value::string(m_csMachineID.GetBuffer());
	lValue[L"MachineName"] = json::value::string(CWWizSettings::WWizGetComputerName().GetBuffer());
	lValue[L"IP"] = json::value::string(strLocal.GetBuffer());
	lValue[L"RIP"] = json::value::string(CWWizSettings::WWizGetServerIPAddress().GetBuffer());
	lValue[L"OS"] = json::value::string(CWWizSettings::WWizGetOSDetails().GetBuffer());
	lValue[L"Domain"] = json::value::string(CWWizSettings::WWizGetDomainValue().GetBuffer());

	std::vector<web::json::value> arraylValue;
	arraylValue.push_back(lValue);

	json::value JsonData;
	JsonData = web::json::value::array(arraylValue);
	
	utility::string_t Lreq = L"=" + JsonData.to_string();

	OutputDebugString(Lreq.c_str());

	CString csClientMachineReg;
	csClientMachineReg.Format(L"http://%s:%s/api/V001/MachineReg/ClientMachineReg", CWWizSettings::WWizGetServerIPAddress(), CWWizSettings::GetHttpCommunicationPort());
	OutputDebugString(csClientMachineReg);
	http_client client(csClientMachineReg.GetBuffer());

	// Manually build up an HTTP request with header and request URI.
	http_request request(methods::POST);
	utility::string_t auth = L"bearer " + CWWizSettings::m_csBearerCode;
	request.headers().add(L"Authorization", auth);
	request.headers().add(L"Content-Type", L"application/x-www-form-urlencoded; charset=UTF-8");
	request.set_body(Lreq);

	return client.request(request).then([](http_response response) -> pplx::task<json::value>
	{
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(json::value());
	})
		.then([this](pplx::task<json::value> previousTask)
	{
		try
		{
			json::value& v = previousTask.get();
			utility::string_t str = v.to_string();
			OutputDebugString(str.c_str());
			if (ParseRegistrationResponse(v))
			{
				m_bISRegistered = true;
				Sleep(1000);
			}
		}
		catch (const http_exception& e)
		{
			// Print error.
			wostringstream ss;
			ss << e.what() << endl;
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetTaskID, Error: %s", CA2W(e.what()), 0, true, SECONDLEVEL);
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizClientAgent::GetTaskID", 0, 0, true, SECONDLEVEL);
		}
	});
}

bool CWWizRegistration::ParseRegistrationResponse(json::value& v)
{
	OutputDebugString(L">>> In CWardwizClientAgent::ParseRegistrationResponse");
	try
	{
		utility::string_t str = v.to_string();
		if (_tcscmp(str.c_str(), L"null") == 0)
		{
			OutputDebugString(L">>> no task available");
			return false;
		}

		std::istringstream iss(CW2A(str.c_str()).m_psz);

		// Read json.
		ptree pt;
		read_json(iss, pt);

		std::string strEmailID;
		std::string Days;
		std::string strRegKeyID;
		for (auto& challenge : pt.get_child("Table"))
		{
			for (auto& prop : challenge.second)
			{
				if (prop.first == "Email")
				{
					strEmailID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "Key")
				{
					strRegKeyID = prop.second.get_value<std::string>();
				}
				else if (prop.first == "Days")
				{
					Days = prop.second.get_value<std::string>();
				}
			}
		}

		if (Days.length() == 0)
		{
			return false;
		}

		int iDays = atoi(Days.c_str());

		//If number of days is 0, then no need to register.
		if (iDays == 0x00)
		{
			return false;
		}

		CITinRegWrapper objReg;
		TCHAR szValue[0x80] = { 0 };
		DWORD dwSize = sizeof(szValue);
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WARDWIZ", L"MVersion", szValue, dwSize) != 0)
		{
			AddLogEntry(L"### Error in GetRegistryValueData CWWizRegistration::RequestRegistrationInfo", 0, 0, true, SECONDLEVEL);
			return false;
		}

		AVACTIVATIONINFO	ActInfo = { 0 };
		memset(&ActInfo, 0, sizeof(ActInfo));
		memcpy(&ActInfo.szClientID, szValue, dwSize);

		CString csKey = CA2W(strRegKeyID.c_str());
		CString csEmailID = CA2W(strEmailID.c_str());

		memcpy(&ActInfo.szKey, csKey, sizeof(ActInfo.szKey));
		memcpy(&ActInfo.szEmailID, csEmailID, sizeof(ActInfo.szEmailID));

		SYSTEMTIME		CurrTime = { 0 };
		GetSystemTime(&CurrTime);
		ActInfo.RegTime = CurrTime;
		ActInfo.RegTimeServer = CurrTime;
		ActInfo.dwTotalDays = iDays;
		ActInfo.dwProductNo = 0x03;//Product ID : 03 for ELITE

		if (AddRegistrationDataInRegistry(ActInfo) != 0x00)
		{
			return false;
		}
	}
	catch (exception const & e)
	{
		CString strError = CString(e.what());
		AddLogEntry(L"### Exception in CWardwizClientAgent::ParseRegistrationResponse : %s", strError, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizClientAgent::ParseRegistrationResponse", 0, 0, true, SECONDLEVEL);
	}
	return true;
}