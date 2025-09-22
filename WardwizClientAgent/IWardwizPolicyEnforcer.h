#pragma once
//Contains Definitions for the Functions to be supported by the Adapter class.

#include <afxcoll.h>
#include "PipeConstants.h"

class IWardwizPolicyEnforcer
{
public:
	// Forbid copying
	IWardwizPolicyEnforcer(IWardwizPolicyEnforcer const &) = delete;
	IWardwizPolicyEnforcer & operator=(IWardwizPolicyEnforcer const &) = delete;

	//virtual destructor.
	virtual ~IWardwizPolicyEnforcer()=default;

	virtual BOOL SetDefaultGroupPolicy() = 0;
	
	virtual BOOL SetRoutineScanSchedule() = 0;
	
	virtual	BOOL DisableUSBDisablePolicy() = 0;

	virtual BOOL EnableUSBDisablePolicy() = 0;

protected :
	// allow construction for child classes only
	IWardwizPolicyEnforcer()=default;
};