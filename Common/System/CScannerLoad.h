#pragma once
#pragma warning (disable:4267)
#include "stdafx.h"

class CScannerLoad
{
public:

	BOOL	m_bIsWow64;
	bool RegisterProcessId(ULONG processID);
	int InstallService(CString sysFilePath);
	int StartServiceLocal();
	int RemoveService();
	int InstallScanner(CString sysFilePath);
	bool PauseDriverProtection(ULONG processID);
	bool IsScannerDriverRunning();
	bool IsScannerDriverRunningSEH();
	bool ProtectFolderSEH(LPTSTR lpszFolderPath);
	bool ProtectFolder(LPTSTR lpszFolderPath);
	bool LockFolder(LPTSTR lpszFolderPath);
	CString GetPathToDriveVolume(CString path);
	bool IsWow64();
	bool SetAutorunSettings(bool IsAutoRunBlocked = true, bool IsUSBExecBlocked = false, bool IsUSBWriteBlocked = false);
};

