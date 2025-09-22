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
#ifdef WARDWIZ_EPS
	static CString WWizGetIPAddress();
#endif
	static CString WWizGetServerIPAddress();
	static CString WWizServerMachineName();
public:
	static CString	m_csProdRegKey;
	static CString	m_csMachineID;
	static CString	m_csBearerCode;
	static CString	m_csClientGUIID;
};

