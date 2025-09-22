#pragma once
class CUninstallClient
{
public:
	CUninstallClient();
	virtual ~CUninstallClient();
	void ReInitializeVariables();
	void StopRunningTasks();
	bool UnInstallClientSetup(std::string strKeepUserDefineSettingsID, std::string strIskeepQurantinedFileID);
	DWORD SendMessage2Service(int iRequest, CString csAppPath, CString csCommand, bool bWait = false);
private:
	bool m_bStop;
};

