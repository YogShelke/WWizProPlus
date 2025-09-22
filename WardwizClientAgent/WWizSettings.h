#pragma once
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

public:
	static CString	m_csProdRegKey;
	static CString	m_csMachineID;
	static CString	m_csBearerCode;
	static CString	m_csClientGUIID;
};

