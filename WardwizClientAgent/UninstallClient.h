#pragma once
class CUninstallClient
{
public:
	CUninstallClient();
	virtual ~CUninstallClient();
	void ReInitializeVariables();
	void StopRunningTasks();
	bool UnInstallClientSetup(std::string strKeepUserDefineSettingsID, std::string strIskeepQurantinedFileID);
private:
	bool m_bStop;
};

