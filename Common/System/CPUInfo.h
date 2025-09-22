#pragma once
#include <winioctl.h>

class CCPUInfo
{
public:
	CCPUInfo();
	virtual ~CCPUInfo();

	CString GetVolumeSerialNo();
	CString GetDiskSerialNo();
	INT _GetHardDriveComputerID();
	INT _ReadPhysicalDriveInNT();
	INT _ReadIdeDriveAsScsiDriveInNT(void);
	void _PrintIdeInfo(int iDrive, DWORD diskdata[MAX_PATH]);
	CString _ConvertToString(DWORD diskdata[MAX_PATH], int firstIndex, int lastIndex);
	BOOL	_DoIDENTIFY(HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP,
		BYTE bIDCmd, BYTE bDriveNum, PDWORD lpcbBytesReturned);
public:
	CString			m_csDiskSerialNo;
	CString			m_chHardDriveSerialNumber;
};

