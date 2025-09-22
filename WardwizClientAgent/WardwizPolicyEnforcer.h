#pragma once
#include "IWardwizPolicyEnforcer.h"

#include "CommonFunctions.h"


class CWardwizPolicyEnforcer : public IWardwizPolicyEnforcer
{
public:
	CWardwizPolicyEnforcer();
	~CWardwizPolicyEnforcer();

	BOOL SetDefaultGroupPolicy() override;

	BOOL SetRoutineScanSchedule() override;

	BOOL DisableUSBDisablePolicy() override;

	BOOL EnableUSBDisablePolicy() override;

private:
	DWORD UpdateUSBPORTsPolicy(BOOL bEnableDisable);
};

