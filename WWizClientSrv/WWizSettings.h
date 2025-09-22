#pragma once
#include "WrdWizSystemInfo.h"

class CWWizSettings
{
public:
	CWWizSettings();
	virtual ~CWWizSettings();
public:
	static CString GetProductRegistryKey();
	static CString GetMachineID();
	static CString GetClientGUIID();
	static bool	SetBearerCode(CString);
	static CString WWizGetComputerName();
	static CString WWizGetOSDetails();
	static CString WWizGetIPAddress();
	static CString WWizGetServerIPAddress();
	static CString WWizServerMachineName();
	static CString WWizGetDomainValue();
	static CString GetHttpCommunicationPort();
private:
	static CString GetServerHttpPort();
	static CString GetServerSSLPort();
public:
	static CString	m_csProdRegKey;
	static CString	m_csMachineID;
	static CString	m_csBearerCode;
	static CString	m_csClientGUIID;
};

