#pragma once
#include "stdafx.h"
#include "json_writer.h"
#include "json_reader.h"
#include "json_data.h"
#include "CommonFunctions.h"

#define				IDR_REGDATA					2000
#define				IDR_QUARDATA				2001

class CWWizRegistration
{
private:
	DWORD	m_dwDaysLeft;
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
};

