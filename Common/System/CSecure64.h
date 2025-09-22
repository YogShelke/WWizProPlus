#pragma once
#include "stdafx.h"
#include "WardwizOSVersion.h"

class CSecure64
{
public:
	bool RegisterProcessId(ULONG processID);
	bool RegisterProcessIdSEH(ULONG processID);
	int InstallService(CString sysFilePath,int OsType);
	int StartServiceLocal(int OsType);
	int StartServiceLocalSEH(int iOsType);
	int RemoveService(int OsType);
	int RemoveServiceSEH(int iOsType);
	bool PauseDriverProtection(ULONG processID);
	bool PauseDriverProtectionSEH(ULONG processID);
	bool IsSecure64DriverRunning(int OsType);
	bool IsSecure64DriverRunningSEH(int iOsType);
	bool RegisterProcessIdAndCode(ULONG processID, ULONG processIDCode);
	bool RegisterProcessIdAndCodeSEH(ULONG processID, ULONG processIDCode);
};