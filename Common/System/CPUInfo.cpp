/**********************************************************************************************************
Program Name			:	CPUInfo.cpp
Description				:	This file contains functionality about the service
Author Name				:	Ramkrushna Shelke
Date Of Creation		:	10 Jan 2014
Version No				:	1.0.0.8
Special Logic Used		:	Communication using named pipes & using Shared Memory.
Modification Log		:
***********************************************************************************************************/

#include "stdafx.h"
#include "CPUInfo.h"

#define  MAX_BUFFER			512
#define  MAX_IDE_DRIVES		4    //  Max number of drives assuming primary/secondary, master/slave topology
#define  SENDIDLENGTH		sizeof (SENDCMDOUTPARAMS) + MAX_BUFFER

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

//  GETVERSIONOUTPARAMS contains the data returned from the GetDriverVersion function.
typedef struct _GETVERSIONOUTPARAMS
{
	BYTE bVersion;      // Binary driver version.
	BYTE bRevision;     // Binary driver revision.
	BYTE bReserved;     // Not used.
	BYTE bIDEDeviceMap; // Bit map of IDE devices.
	DWORD fCapabilities; // Bit mask of driver capabilities.
	DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

// The following struct defines the interesting part of the IDENTIFY
// buffer:
typedef struct _IDSECTOR
{
	USHORT  wGenConfig;
	USHORT  wNumCyls;
	USHORT  wReserved;
	USHORT  wNumHeads;
	USHORT  wBytesPerTrack;
	USHORT  wBytesPerSector;
	USHORT  wSectorsPerTrack;
	USHORT  wVendorUnique[3];
	CHAR    sSerialNumber[20];
	USHORT  wBufferType;
	USHORT  wBufferSize;
	USHORT  wECCSize;
	CHAR    sFirmwareRev[8];
	CHAR    sModelNumber[40];
	USHORT  wMoreVendorUnique;
	USHORT  wDoubleWordIO;
	USHORT  wCapabilities;
	USHORT  wReserved1;
	USHORT  wPIOTiming;
	USHORT  wDMATiming;
	USHORT  wBS;
	USHORT  wNumCurrentCyls;
	USHORT  wNumCurrentHeads;
	USHORT  wNumCurrentSectorsPerTrack;
	ULONG   ulCurrentSectorCapacity;
	USHORT  wMultSectorStuff;
	ULONG   ulTotalAddressableSectors;
	USHORT  wSingleWordDMA;
	USHORT  wMultiWordDMA;
	BYTE    bReserved[128];
} IDSECTOR, *PIDSECTOR;

typedef struct _SRB_IO_CONTROL
{
	ULONG HeaderLength;
	UCHAR Signature[9];
	ULONG Timeout;
	ULONG ControlCode;
	ULONG ReturnCode;
	ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;


//  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition


//  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.

BYTE IdOutCmd[sizeof(SENDCMDOUTPARAMS) + MAX_BUFFER - 1] = { 0 };

CCPUInfo::CCPUInfo()
{
}

CCPUInfo::~CCPUInfo()
{
}

/**************************************************************************************
Function		: GetVolumeSerialNo
In Parameters	: -
Out Parameters	: CString : string containg Volume serial number
Author Name		: Ramkrushna Shelke
Purpose			: This Function obtain Volume serial No.on local machine
***************************************************************************************/
CString CCPUInfo::GetVolumeSerialNo()
{
	try
	{
		DWORD dwVolumeSerialNum;
		long lRetVal;
		CString csHexSN("");
		CString strWinDir("");
		CString csSerialNum("");

		lRetVal = GetWindowsDirectory(strWinDir.GetBuffer(MAX_PATH), MAX_PATH);
		strWinDir.ReleaseBuffer();
		strWinDir = strWinDir.Left(1);
		strWinDir = strWinDir + _T(":\\");
		GetVolumeInformation(strWinDir, NULL, NULL, &dwVolumeSerialNum, NULL, NULL, NULL, NULL);
		csHexSN.Format(_T("%X"), dwVolumeSerialNum);
		if (csHexSN.GetLength() <= 8)
		{
			csSerialNum = _T("");
			csSerialNum = /*_T("[") +*/ csHexSN.Left(4) +/* _T("-") +*/ csHexSN.Right(4)/* + _T("]")*/;
		}
		return csSerialNum.Trim();
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception in CCPUInfo::GetVolumeSerialNo"), 0, 0, true, SECONDLEVEL);
	}
	return CString(_T(""));
}

/**************************************************************************************
Function		: GetDiskSerialNo
In Parameters	: -
Out Parameters	: CString : string containg Disk serial number
Author Name		: Ramkrushna Shelke
Purpose			: This Function obtain Disk serial No.on local machine
***************************************************************************************/
CString CCPUInfo::GetDiskSerialNo()
{
	try
	{
		m_csDiskSerialNo.Empty();

		_GetHardDriveComputerID();

		if (m_csDiskSerialNo == "$")
			m_csDiskSerialNo.Empty();
	}
	catch (...)
	{
		m_csDiskSerialNo.Empty();
		AddLogEntry(_T("### Exception in CCPUInfo::GetDiskSerialNo"), 0, 0, true, SECONDLEVEL);
	}
	return m_csDiskSerialNo;
}


/**************************************************************************************
Function		: _GetHardDriveComputerID
In Parameters	: -
Out Parameters	: INT : containg Hard Drive Id
Author Name		: Ramkrushna Shelke
Purpose			: This function obtains Hard Drive Id on local machine
**************************************************************************************/
INT CCPUInfo::_GetHardDriveComputerID()
{
	try
	{
		INT iRetVal = FALSE;

		m_chHardDriveSerialNumber.Empty();

		//  this works under WinNT4 or Win2K if you have admin rights
		iRetVal = _ReadPhysicalDriveInNT();

		//  this should work in WinNT or Win2K if previous did not work this is kind of
		//  back door via the SCSI mini port driver into  the IDE drives
		if (!iRetVal)
			iRetVal = _ReadIdeDriveAsScsiDriveInNT();

		return iRetVal;
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception in CCPUInfo::_GetHardDriveComputerID"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************
Function		: _ReadPhysicalDriveInNT
In Parameters	: -
Out Parameters	: INT : contain Physical drive on NT
Author Name		: Ramkrushna Shelke
Purpose			: This Function obtain Physical drive on NT machine
***************************************************************************************/
INT CCPUInfo::_ReadPhysicalDriveInNT()
{
	try
	{
		INT iRetVal = FALSE;
		INT iDrive = 0;

		for (iDrive = 0; iDrive < MAX_IDE_DRIVES; iDrive++)
		{
			HANDLE hPhysicalDriveIOCTL = 0;

			//  Try to get a handle to PhysicalDrive IOCTL, report failure and exit if can't.
			TCHAR driveName[MAX_PATH] = { 0 };

			swprintf_s(driveName, _countof(driveName), _T("\\\\.\\PhysicalDrive%d"), iDrive);

			//  Windows NT, Windows 2000, must have admin rights
			hPhysicalDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, 0, NULL);

			if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
			{
				GETVERSIONOUTPARAMS VersionParams = { 0 };
				DWORD               cbBytesReturned = 0;

				// Get the version, etc of PhysicalDrive IOCTL
				if (!DeviceIoControl(hPhysicalDriveIOCTL, DFP_GET_VERSION,
					NULL, 0, &VersionParams, sizeof(VersionParams), &cbBytesReturned, NULL))
				{
					CString csErr;
					csErr.Format(_T("DFP_GET_VERSION failed for drive %d and errorcode = %d"), iDrive, ::GetLastError());
					AddLogEntry(L"%s", csErr, 0, true, SECONDLEVEL);
					//	continue;
				}

				// If there is a IDE device at number "i" issue commands to the device
				if (VersionParams.bIDEDeviceMap > 0)
				{
					BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
					SENDCMDINPARAMS  scip = { 0 };

					// Now, get the ID sector for all IDE devices in the system.
					// If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
					// otherwise use the IDE_ATA_IDENTIFY command
					bIDCmd = (VersionParams.bIDEDeviceMap >> iDrive & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

					memset(IdOutCmd, 0, sizeof(IdOutCmd));

					if (_DoIDENTIFY(hPhysicalDriveIOCTL, &scip, (PSENDCMDOUTPARAMS)&IdOutCmd,
						(BYTE)bIDCmd, (BYTE)iDrive, &cbBytesReturned))
					{
						DWORD diskdata[MAX_PATH];
						USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer;

						for (int idx = 0; idx < MAX_PATH; idx++)
							diskdata[idx] = pIdSector[idx];

						_PrintIdeInfo(iDrive, diskdata);
						iRetVal = TRUE;
					}
				}
				CloseHandle(hPhysicalDriveIOCTL);
			}
		}
		return iRetVal;
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception in CCPUInfo::_ReadPhysicalDriveInNT"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************
Function		: _ReadIdeDriveAsScsiDriveInNT
In Parameters	: -
Out Parameters	: INT :
Author Name		: Ramkrushna Shelke
Purpose			: This Function obtain Volume serial No.on local machine
***************************************************************************************/
INT CCPUInfo::_ReadIdeDriveAsScsiDriveInNT(void)
{
	try
	{
		int iRetVal = FALSE;
		int controller = 0;

		for (controller = 0; controller < 2; controller++)
		{
			HANDLE hScsiDriveIOCTL = 0;
			TCHAR   driveName[MAX_PATH] = { 0 };

			//Try to get a handle to PhysicalDrive IOCTL, report failure and exit if can't.
			swprintf_s(driveName, _countof(driveName), _T("\\\\.\\Scsi%d:"), controller);

			//  Windows NT, Windows 2000, any rights should do
			hScsiDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, 0, NULL);

			if (hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
			{
				int drive = 0;

				for (drive = 0; drive < 2; drive++)
				{
					TCHAR buffer[sizeof(SRB_IO_CONTROL) + SENDIDLENGTH] = { 0 };
					SRB_IO_CONTROL *p = (SRB_IO_CONTROL *)buffer;
					SENDCMDINPARAMS *pin =
						(SENDCMDINPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
					DWORD dummy;

					//memset (buffer, 0, sizeof (buffer));
					p->HeaderLength = sizeof(SRB_IO_CONTROL);
					p->Timeout = 10000;
					p->Length = SENDIDLENGTH;
					p->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;

					//wcscpy cannot be used here
					//wcscpy_s((TCHAR *)p->Signature, _countof(p->Signature),_T("SCSIDISK"));
					strncpy_s((char *)p->Signature, sizeof(p->Signature), (char *)"SCSIDISK", 8);
					pin->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
					pin->bDriveNumber = drive;

					if (DeviceIoControl(hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT,
						buffer,
						sizeof(SRB_IO_CONTROL) +
						sizeof(SENDCMDINPARAMS) - 1,
						buffer,
						sizeof(SRB_IO_CONTROL) + SENDIDLENGTH,
						&dummy, NULL))
					{

						SENDCMDOUTPARAMS *pOut =
							(SENDCMDOUTPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
						IDSECTOR *pId = (IDSECTOR *)(pOut->bBuffer);
						if (pId->sModelNumber[0])
						{
							DWORD diskdata[MAX_PATH] = { 0 };
							int ijk = 0;
							USHORT *pIdSector = (USHORT *)pId;
							for (ijk = 0; ijk < MAX_PATH; ijk++)
								diskdata[ijk] = pIdSector[ijk];

							_PrintIdeInfo(controller * 2 + drive, diskdata);
							iRetVal = TRUE;
						}
					}
				}
				CloseHandle(hScsiDriveIOCTL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception in CCPUInfo::_ReadIdeDriveAsScsiDriveInNT"), 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


/***************************************************************************************
Function		: _PrintIdeInfo
In Parameters	: int - drive
DWORD - diskdata
Out Parameters	: void
Author Name		: Ramkrushna Shelke
Purpose			: to get Hard Drive Serial number
***************************************************************************************/
void CCPUInfo::_PrintIdeInfo(int iDrive, DWORD diskdata[MAX_PATH])
{
	try
	{
		// copy the hard driver serial number to the buffer
		m_chHardDriveSerialNumber = _ConvertToString(diskdata, 10, 19);

		CString csTemp;
		csTemp = m_csDiskSerialNo;
		if (!csTemp.IsEmpty())
			m_csDiskSerialNo += "$";

		CString csTempHDDNo;
		csTempHDDNo.Format(_T("%s"), static_cast<LPCTSTR>(m_chHardDriveSerialNumber));
		m_csDiskSerialNo += csTempHDDNo.Trim();// HardDriveSerialNumber; 
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception in CCPUInfo::_PrintIdeInfo"), 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************
Function		: _ConvertToString
In Parameters	: DWORD - disk data
int - first index
int - last index
Out Parameters	: char * - pointer to string
Author Name		: Ramkrushna Shelke
Purpose			: to convert diskdata to string
**************************************************************************************/
CString CCPUInfo::_ConvertToString(DWORD diskdata[MAX_PATH], int firstIndex, int lastIndex)
{
	try
	{
		TCHAR string[MAX_BUFFER];
		int index = 0;
		int position = 0;

		//  each integer has two characters stored in it backwards
		for (index = firstIndex; index <= lastIndex; index++)
		{
			//  get high byte for 1st character
			string[position] = (TCHAR)(diskdata[index] / 256);
			position++;

			//  get low byte for 2nd character
			string[position] = (TCHAR)(diskdata[index] % 256);
			position++;
		}

		//  end the string
		string[position] = '\0';

		//  cut off the trailing blanks
		for (index = position - 1; index > 0 && ' ' == string[index]; index--)
			string[index] = '\0';

		return string;
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception in CCPUInfo::_ConvertToString"), 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}


/***************************************************************************************
Function		: _DoIDENTIFY
In Parameters	: HANDLE - PhysicalDriveIOCTL handle
PSENDCMDINPARAMS - in param struct
PSENDCMDOUTPARAMS - out param struct
BYTE - ID
BYTE - drive number
PDWORD - return bytes
Out Parameters	: bool
Author Name		: Ramkrushna Shelke
Purpose			: to identify the command
***************************************************************************************/
BOOL CCPUInfo::_DoIDENTIFY(HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
	PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
	PDWORD lpcbBytesReturned)
{
	try
	{
		// Set up data structures for IDENTIFY command.
		pSCIP->cBufferSize = MAX_BUFFER;
		pSCIP->irDriveRegs.bFeaturesReg = 0;
		pSCIP->irDriveRegs.bSectorCountReg = 1;
		pSCIP->irDriveRegs.bSectorNumberReg = 1;
		pSCIP->irDriveRegs.bCylLowReg = 0;
		pSCIP->irDriveRegs.bCylHighReg = 0;

		// Compute the drive number.
		pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

		// The command can either be IDE identify or ATAPI identify.
		pSCIP->irDriveRegs.bCommandReg = bIDCmd;
		pSCIP->bDriveNumber = bDriveNum;
		pSCIP->cBufferSize = MAX_BUFFER;

		return (DeviceIoControl(hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
			(LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1, (LPVOID)pSCOP,
			sizeof(SENDCMDOUTPARAMS) + MAX_BUFFER - 1, lpcbBytesReturned, NULL));
	}
	catch (...)
	{
		AddLogEntry(_T("### Exception in CCPUInfo::_DoIDENTIFY"), 0, 0, true, SECONDLEVEL);
	}
	return false;
}
