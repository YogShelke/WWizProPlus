#include "WardwizPolicyEnforcer.h"


CWardwizPolicyEnforcer::CWardwizPolicyEnforcer()
{
}


CWardwizPolicyEnforcer::~CWardwizPolicyEnforcer()
{
}

BOOL CWardwizPolicyEnforcer::SetDefaultGroupPolicy()
{
	return TRUE;
}

BOOL CWardwizPolicyEnforcer::SetRoutineScanSchedule()
{
	return TRUE;
}

BOOL CWardwizPolicyEnforcer::DisableUSBDisablePolicy()
{
	UpdateUSBPORTsPolicy(FALSE);
	return TRUE;
}

BOOL CWardwizPolicyEnforcer::EnableUSBDisablePolicy()
{
	UpdateUSBPORTsPolicy(TRUE);
	return TRUE;
}

DWORD CWardwizPolicyEnforcer::UpdateUSBPORTsPolicy(BOOL bEnableDisable)
{
	HKEY	hKeyUSB;
	DWORD	dwError;
	DWORD	dwValue(0);

	//if (bEnableDisable)
	//	dwValue = static_cast<int>(_USBREGISTRYVALUE::ENABLEUSB);
	//else
	//	dwValue = static_cast<int>(_USBREGISTRYVALUE::DISABLEUSB);

	dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\UsbStor", 0, KEY_ALL_ACCESS | KEY_WRITE, &hKeyUSB);
	
	if (ERROR_SUCCESS == dwError)
	{
		// Got a valid handle to USBSTOR..
		// Now set the required value to enable or disable USB...
		dwError = ::RegSetValueEx(hKeyUSB, L"Start", 0, REG_DWORD, reinterpret_cast<BYTE *>(&dwValue), sizeof(dwValue));

		if (ERROR_SUCCESS == dwError)
		{
			RegCloseKey(hKeyUSB);
			return dwError;
		}
	}
	else
	{
		RegCloseKey(hKeyUSB);
		return dwError;
	}

	return ERROR_SUCCESS;
}