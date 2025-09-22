#pragma once
#include <vector>

class CWWizFirewallFilter
{
public:
	CWWizFirewallFilter();
	virtual ~CWWizFirewallFilter();
	bool LoadBlockedApplicationList();
	bool ReLoadBlockedApplicationList();
	bool ISApplicationBlocked(std::wstring strProcessName);
private:
	std::vector<std::string>	m_vBlockedAppList;
};

