/****************************************************
*  Program Name: ISpyRepair.cpp                                                                                                    
*  Description: Virus Repair Routines
*  Author Name: Prajakta                                                                                                      
*  Date Of Creation: 28 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#include "stdafx.h"
#include "ISpyRepair.h"
#include "Enumprocess.h"
/**********************************************************************************************************                     
*  Function Name  :	CISpyRepair                                                     
*  Description    :	C'tor
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
CISpyRepair::CISpyRepair(void):
m_b64bit(false),
m_dwFileSize(0),
m_dwAEPMapped(0),
m_wAEPSec(0),
m_bPEFile(false),
m_bMZFound(false),
m_bPEFound(false),
m_bSecFound(false),
m_dwAEPSectionSize(-1),
m_dwAEPSectionStartPos(-1),
m_byBuffer(NULL)
{
	m_hFileHandle = INVALID_HANDLE_VALUE;
	memset(&m_stPEHeader, 0, sizeof(m_stPEHeader));
	memset(&m_stPEOffsets, 0, sizeof(m_stPEOffsets));
	memset(&m_stPEHeaderOffsets, 0, sizeof(m_stPEHeaderOffsets));
	memset(m_stSectionHeader, 0, sizeof(IMAGE_SECTION_HEADER) * MAX_SEC_NO);
	wmemset(m_szFilePath, 0, MAX_PATH);
	m_dwBuffSize = 0;

	m_dwStartOffset = 0x00;
}

/**********************************************************************************************************                     
*  Function Name  :	~CISpyRepair                                                     
*  Description    :	Destructor
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
CISpyRepair::~CISpyRepair(void)
{
	if(m_byBuffer)
	{
		delete []m_byBuffer;
		m_byBuffer = NULL;
	}
	CloseFile();
}

/**********************************************************************************************************                     
*  Function Name  :	CloseFile                                                     
*  Description    :	Closes file handle
*  SR.NO		  : VIRUSREPAIR_0010
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
void CISpyRepair::CloseFile()
{
	m_dwAEPSectionSize = -1;
	m_dwAEPSectionStartPos = -1;
	if(INVALID_HANDLE_VALUE != m_hFileHandle)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
	wmemset(m_szFilePath, 0, MAX_PATH);
	m_dwFileSize = 0;
}

/**********************************************************************************************************                     
*  Function Name  :	OpenFile                                                     
*  Description    :	Creates/opens file in read/write mode
*  SR.NO		  : VIRUSREPAIR_0002
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::OpenFile(LPCTSTR szFilePath, bool bOpenToRepair)
{
	m_dwDesirecAccess = GENERIC_READ;
	m_dwSharedMode = FILE_SHARE_READ;
	if(bOpenToRepair)
	{
		m_dwDesirecAccess = GENERIC_READ | GENERIC_WRITE;
		SetFileAttributes(szFilePath, FILE_ATTRIBUTE_NORMAL);
	}

	wcscpy_s(m_szFilePath, szFilePath);
	m_csFilePath = m_szFilePath;
	m_hFileHandle = CreateFile(szFilePath, m_dwDesirecAccess, m_dwSharedMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE == m_hFileHandle)
	{
		return false;
	}
	return CheckAndLoadValidPEFile(m_hFileHandle, bOpenToRepair);
}

/**********************************************************************************************************                     
*  Function Name  :	CheckAndLoadValidPEFile                                                     
*  Description    :	Checks whether it is a valid PE file
*  SR.NO		  : VIRUSREPAIR_0003
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::CheckAndLoadValidPEFile(HANDLE hFileHandle, bool bOpenToRepair)
{
	m_bSecFound = m_bPEFound = m_bMZFound = false;

	if(INVALID_HANDLE_VALUE == hFileHandle)
	{
		return false;
	}

	m_hFileHandle = hFileHandle;
	m_dwFileSize = GetFileSize(m_hFileHandle,0);

	if(m_dwFileSize == 0 )
	{
		return false;
	}

	IMAGE_DOS_HEADER stImageDosHeader;		
	memset(&stImageDosHeader, 0, sizeof(stImageDosHeader));
	
	//Check MZ i.e 0x4d5a
	DWORD dwBytesRead = 0;
	if(!GetFileBuffer(&stImageDosHeader, 0, sizeof(stImageDosHeader), sizeof(stImageDosHeader)))
	{
		return false;
	}
	if(stImageDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{
		return false;
	}
	m_bMZFound = true;

	//Check PE i.e 0x00004550
	DWORD dwSignature = 0;
	if(!GetFileBuffer(&dwSignature, stImageDosHeader.e_lfanew, sizeof(DWORD), sizeof(DWORD)))
	{
		return false;
	}
	if(dwSignature != IMAGE_NT_SIGNATURE)
	{
		return false;
	}
	m_bPEFound = true;

	//File Header
	IMAGE_FILE_HEADER stImageFileHeader;
	memset(&stImageFileHeader, 0, sizeof(stImageFileHeader));
	
	if(!GetFileBuffer(&stImageFileHeader, stImageDosHeader.e_lfanew + 4, sizeof(stImageFileHeader), sizeof(stImageFileHeader)))
	{
		return false;
	}
	if(stImageFileHeader.NumberOfSections > _countof(m_stSectionHeader) || 0 == stImageFileHeader.NumberOfSections)
	{
		return false;
	}
	m_stPEHeader.e_csum					= stImageDosHeader.e_csum;
	m_stPEHeader.e_lfanew				= stImageDosHeader.e_lfanew;
	m_stPEHeader.NumberOfSections		= stImageFileHeader.NumberOfSections;
	m_stPEHeader.Characteristics		= stImageFileHeader.Characteristics;
	m_stPEHeader.SizeOfOptionalHeader	= stImageFileHeader.SizeOfOptionalHeader;
	m_stPEHeader.NumberOfSymbols		= stImageFileHeader.NumberOfSymbols;
	
	if(IMAGE_FILE_MACHINE_IA64 == stImageFileHeader.Machine || IMAGE_FILE_MACHINE_AMD64 == stImageFileHeader.Machine)
	{
		if(!Load64BitHeader())
		{
			return false;
		}
		m_b64bit = true;
	}
	else
	{
		if(!Load32BitHeader())
		{
			return false;
		}
	}	

	DWORD dwSectionStart = m_stPEHeader.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + m_stPEHeader.SizeOfOptionalHeader;
	memset(&m_stSectionHeader, 0, sizeof(m_stSectionHeader));
	if(!GetFileBuffer(m_stSectionHeader, dwSectionStart, sizeof(m_stSectionHeader[0])* m_stPEHeader.NumberOfSections, sizeof(m_stSectionHeader[0])* m_stPEHeader.NumberOfSections))
	{
		return false;
	}
	m_bSecFound = true;

	//Find AEP Section,AEPMapped Value,AEPSectionSRD,AEPSectionPRD
	m_wAEPSec = RVA2FileOff(m_stPEHeader.AddressOfEntryPoint, &m_dwAEPMapped);
	if(m_wAEPSec != OUT_OF_FILE)
	{
		m_dwAEPSectionSize = m_stSectionHeader[m_wAEPSec].SizeOfRawData;
		m_dwAEPSectionStartPos = m_stSectionHeader[m_wAEPSec].PointerToRawData;
	}

	if(bOpenToRepair)
	{
		LoadOffsets();
	}
	m_bPEFile = true;
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	Load32BitHeader                                                     
*  Description    :	Loads the 32-bit Image Optional Header fields
*  SR.NO		  : VIRUSREPAIR_0004
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::Load32BitHeader()
{
	DWORD dwBytesRead = 0;
	IMAGE_OPTIONAL_HEADER32 stImageOptionalHeader;
	memset(&stImageOptionalHeader, 0, sizeof(stImageOptionalHeader));
	GetFileBuffer(&stImageOptionalHeader, sizeof(stImageOptionalHeader), &dwBytesRead);
	if(dwBytesRead == sizeof(stImageOptionalHeader))
	{
		m_stPEHeader.AddressOfEntryPoint		= stImageOptionalHeader.AddressOfEntryPoint;
		m_stPEHeader.CheckSum					= stImageOptionalHeader.CheckSum;
		m_stPEHeader.FileAlignment				= (0 == stImageOptionalHeader.FileAlignment) ? 0x200 : stImageOptionalHeader.FileAlignment; 
		m_stPEHeader.ImageBase					= stImageOptionalHeader.ImageBase;
		m_stPEHeader.Magic						= stImageOptionalHeader.Magic;
		m_stPEHeader.MinorLinkerVersion			= stImageOptionalHeader.MinorLinkerVersion;
		m_stPEHeader.SectionAlignment			= (0 == stImageOptionalHeader.SectionAlignment) ? 0x1000 : stImageOptionalHeader.SectionAlignment;
		m_stPEHeader.MinorOSVersion				= stImageOptionalHeader.MinorOperatingSystemVersion;
		m_stPEHeader.MajorImageVersion			= stImageOptionalHeader.MajorImageVersion;;
		m_stPEHeader.MinorImageVersion			= stImageOptionalHeader.MinorImageVersion;;
		m_stPEHeader.MinorSubsystemVersion		= stImageOptionalHeader.MinorSubsystemVersion;
		m_stPEHeader.SizeOfCode					= stImageOptionalHeader.SizeOfCode;
		m_stPEHeader.SizeOfImage				= stImageOptionalHeader.SizeOfImage;
		m_stPEHeader.Subsystem					= stImageOptionalHeader.Subsystem;
		m_stPEHeader.SizeOfStackCommit			= stImageOptionalHeader.SizeOfStackCommit;
		m_stPEHeader.Win32VersionValue			= stImageOptionalHeader.Win32VersionValue;
		m_stPEHeader.SizeOfHeaders				= stImageOptionalHeader.SizeOfHeaders;
		m_stPEHeader.LoaderFlags				= stImageOptionalHeader.LoaderFlags;
		m_stPEHeader.NumberOfRvaAndSizes	= stImageOptionalHeader.NumberOfRvaAndSizes;
		memcpy(m_stPEHeader.DataDirectory, stImageOptionalHeader.DataDirectory, IMAGE_NUMBEROF_DIRECTORY_ENTRIES*sizeof(IMAGE_DATA_DIRECTORY));
		return true;
	}	
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	Load64BitHeader                                                     
*  Description    :	Loads the 64-bit Image Optional Header fields
*  SR.NO		  : VIRUSREPAIR_0004
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::Load64BitHeader()
{
	DWORD dwBytesRead = 0;
	IMAGE_OPTIONAL_HEADER64 stImageOptionalHeader;
	memset(&stImageOptionalHeader, 0, sizeof(stImageOptionalHeader));
	GetFileBuffer(&stImageOptionalHeader, sizeof(stImageOptionalHeader), &dwBytesRead);
	if(dwBytesRead == sizeof(stImageOptionalHeader))
	{
		m_stPEHeader.AddressOfEntryPoint	= stImageOptionalHeader.AddressOfEntryPoint;
		m_stPEHeader.CheckSum				= stImageOptionalHeader.CheckSum;
		m_stPEHeader.FileAlignment			= (0 == stImageOptionalHeader.FileAlignment) ? 0x200 : stImageOptionalHeader.FileAlignment; 
		m_stPEHeader.ImageBase				= HIDWORD64(stImageOptionalHeader.ImageBase);
		m_stPEHeader.Magic					= stImageOptionalHeader.Magic;
		m_stPEHeader.MinorLinkerVersion		= stImageOptionalHeader.MinorLinkerVersion;
		m_stPEHeader.SectionAlignment		= (0 == stImageOptionalHeader.SectionAlignment) ? 0x1000 : stImageOptionalHeader.SectionAlignment;
		m_stPEHeader.MinorOSVersion			= stImageOptionalHeader.MinorOperatingSystemVersion;
		m_stPEHeader.MajorImageVersion			= stImageOptionalHeader.MajorImageVersion;;
		m_stPEHeader.MinorImageVersion			= stImageOptionalHeader.MinorImageVersion;;
		m_stPEHeader.MinorSubsystemVersion	= stImageOptionalHeader.MinorSubsystemVersion;
		m_stPEHeader.SizeOfCode				= stImageOptionalHeader.SizeOfCode;
		m_stPEHeader.SizeOfImage			= stImageOptionalHeader.SizeOfImage;
		m_stPEHeader.Subsystem				= stImageOptionalHeader.Subsystem;
		m_stPEHeader.SizeOfStackCommit		= HIDWORD64(stImageOptionalHeader.SizeOfStackCommit);
		m_stPEHeader.Win32VersionValue		= stImageOptionalHeader.Win32VersionValue;
		m_stPEHeader.LoaderFlags			= stImageOptionalHeader.LoaderFlags;
		m_stPEHeader.NumberOfRvaAndSizes	= stImageOptionalHeader.NumberOfRvaAndSizes;
		memcpy(m_stPEHeader.DataDirectory, stImageOptionalHeader.DataDirectory, IMAGE_NUMBEROF_DIRECTORY_ENTRIES*sizeof(IMAGE_DATA_DIRECTORY));
		return true;
	}	
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	LoadOffsets                                                     
*  Description    :	Loads Offsets Of Image Optional Header
*  SR.NO		  : VIRUSREPAIR_0005
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
void CISpyRepair::LoadOffsets()
{		
	m_stPEOffsets.NumberOfSections		= m_stPEHeader.e_lfanew + 0x06;
	m_stPEOffsets.Magic					= m_stPEHeader.e_lfanew + 0x18;
	m_stPEOffsets.MajorLinkerVersion    = m_stPEHeader.e_lfanew + 0x1A;
	m_stPEOffsets.MinorLinkerVersion	= m_stPEHeader.e_lfanew + 0x1B;
	m_stPEOffsets.SizeOfCode			= m_stPEHeader.e_lfanew + 0x1C;
	m_stPEOffsets.AddressOfEntryPoint	= m_stPEHeader.e_lfanew + 0x28;
	m_stPEOffsets.SectionAlignment		= m_stPEHeader.e_lfanew + 0x38;
	m_stPEOffsets.MajorOSVersion        = m_stPEHeader.e_lfanew + 0x40;
	m_stPEOffsets.MinorOSVersion		= m_stPEHeader.e_lfanew + 0x42;
	m_stPEOffsets.MajorSubsystemVersion = m_stPEHeader.e_lfanew + 0x48;
	m_stPEOffsets.MinorSubsystemVersion	= m_stPEHeader.e_lfanew + 0x4A;
	m_stPEOffsets.Win32VersionValue 	= m_stPEHeader.e_lfanew + 0x4C;
	m_stPEOffsets.SizeOfImage			= m_stPEHeader.e_lfanew + 0x50;
	m_stPEOffsets.Checksum				= m_stPEHeader.e_lfanew + 0x58;		
		
	if(m_b64bit)
	{
		m_stPEOffsets.ImageBase				= m_stPEHeader.e_lfanew + 0x30;
		m_stPEOffsets.NoOfDataDirs			= m_stPEHeader.e_lfanew + 0x84;
	}
	else
	{
		m_stPEOffsets.BaseOfData			= m_stPEHeader.e_lfanew + 0x30;
		m_stPEOffsets.ImageBase				= m_stPEHeader.e_lfanew + 0x34;
		m_stPEOffsets.LoaderFlags			= m_stPEHeader.e_lfanew + 0x70;
		m_stPEOffsets.NoOfDataDirs			= m_stPEHeader.e_lfanew + 0x74;
	}
	return;
}

/**********************************************************************************************************                     
*  Function Name  :	GetFileBuffer                                                     
*  Description    :	Reads data from a file offset  
*  Author Name    :	Ram                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq, DWORD *pdwBytesRead)
{
	DWORD dwBytesRead = 0x00;
	if(dwReadOffset == OUT_OF_FILE)
		return false;
	DWORD dwSetFileOffSet = ::SetFilePointer(m_hFileHandle, dwReadOffset, NULL, FILE_BEGIN);
	if(dwSetFileOffSet == dwReadOffset)
	{
		if(ReadFile(m_hFileHandle, pbReadBuffer, dwBytesToRead, &dwBytesRead, NULL))
		{
			if (dwBytesRead && dwBytesRead >= dwMinBytesReq)
			{
				if(pdwBytesRead)
				{
					*pdwBytesRead = dwBytesRead;
				}
				return true;
			}
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	GetFileBuffer                                                     
*  Description    :	Reads data from a file  
*  Author Name    :	Ram                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::GetFileBuffer(LPVOID pbReadBuffer, DWORD dwBytesToRead, DWORD * pdwBytesRead)
{
	DWORD dwBytesRead = 0;

	if(!pbReadBuffer)
	{
		return false;
	}

	if(!ReadFile(m_hFileHandle, pbReadBuffer, dwBytesToRead, &dwBytesRead, NULL))
	{
		return false;
	}

	if(pdwBytesRead)
	{
		*pdwBytesRead = dwBytesRead;
	}

	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	RVA2FileOff                                                     
*  Description    :	Converts RVA to file offset & returns section number of that file offset
*  Author Name    :	Prajakta                                                                                          
*  Date           : 29 Nov 2013
**********************************************************************************************************/
WORD CISpyRepair::RVA2FileOff(DWORD dwRVAAddr, DWORD *pdwFileOff)
{	 
	WORD	wSec = 0x00 ;
	DWORD	dwFileOff = 0x00 ;

	if(m_stPEHeader.NumberOfSections > MAX_SEC_NO) 
	{
		return 0;
	}
	for(; wSec < m_stPEHeader.NumberOfSections; wSec++)
	{
		//Issue was :: RVA to File Offset was not getting correct due to RVA size is not equal to SRD
		//Modified by Vilas on 18 April 2015
		if(dwRVAAddr >= m_stSectionHeader[wSec].VirtualAddress && 
			(	(dwRVAAddr < (m_stSectionHeader[wSec].VirtualAddress + m_stSectionHeader[wSec].Misc.VirtualSize)) ||
				(dwRVAAddr < (m_stSectionHeader[wSec].VirtualAddress + m_stSectionHeader[wSec].SizeOfRawData)) )
			)
		{
			*pdwFileOff = dwRVAAddr - m_stSectionHeader[wSec].VirtualAddress + m_stSectionHeader[wSec].PointerToRawData;
			break;
		}
	}
	return wSec;
}
/**********************************************************************************************************                     
*  Function Name  :	GetMappedOff                                                     
*  Description    :	To get mapped offset
*  Author Name    :	Prajakta                                                                                          
*  Date           : 29 Nov 2013
**********************************************************************************************************/
DWORD CISpyRepair::GetMappedOff(DWORD dwRVAAddr)
{
	if(m_stPEHeader.NumberOfSections > 0 && dwRVAAddr < m_stSectionHeader[0].VirtualAddress) 
	{
		return dwRVAAddr;
	}
	for(WORD wSec = 0; wSec < m_stPEHeader.NumberOfSections; wSec++)
	{
		if(dwRVAAddr >= m_stSectionHeader[wSec].VirtualAddress && dwRVAAddr < (m_stSectionHeader[wSec].VirtualAddress + m_stSectionHeader[wSec].Misc.VirtualSize))
		{
			return (dwRVAAddr - m_stSectionHeader[wSec].VirtualAddress + m_stSectionHeader[wSec].PointerToRawData);
		}
	}
	return -1;
}

/**********************************************************************************************************                     
*  Function Name  :	WriteBuffer                                                     
*  Description    :	Writes data into file from buffer
*  Author Name    :	Prajakta                                                                                          
*  Date           : 29 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::WriteBuffer(LPVOID pbWriteBufer, DWORD dwWriteOffset, DWORD dwBytes2Write, DWORD dwMinBytesReq, DWORD *pdwBytesWritten)
{
	if(pbWriteBufer)
	{
		DWORD dwBytesWritten = 0x00;
		DWORD dwSetFileOffSet = ::SetFilePointer(m_hFileHandle, dwWriteOffset, NULL, FILE_BEGIN);
		if(dwSetFileOffSet == dwWriteOffset)
		{
			if(WriteFile(m_hFileHandle, pbWriteBufer, dwBytes2Write, &dwBytesWritten, NULL))
			{
				if (dwBytesWritten && dwBytesWritten >= dwMinBytesReq)
				{
					if(pdwBytesWritten)
					{
						*pdwBytesWritten = dwBytesWritten;
					}
					return true;
				}
			}
		}
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	RepairDelete                                                     
*  Description    :	Close & Delete file
*  SR.NO		  : VIRUSREPAIR_0011
*  Author Name    :	Prajakta                                                                                          
*  Date           : 29 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::RepairDelete()
{	
	CloseFile();
	return (TRUE == DeleteFile(m_csFilePath)) ? true : false;
}

DWORD CISpyRepair::WriteAEP()
{
	return WriteAEP(static_cast<DWORD>(m_dwArgs[0]));
}
/**********************************************************************************************************                     
*  Function Name  :	WriteAEP                                                     
*  Description    :	Rewrites original AEP in Optional Header
*  SR.NO		  : VIRUSREPAIR_0012
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
DWORD CISpyRepair::WriteAEP(DWORD dwAEPToWrite)
{
	DWORD 	dwOff = 0x00;
	RVA2FileOff(dwAEPToWrite, &dwOff);
	if (dwOff > m_dwFileSize || dwAEPToWrite == 0x0)
	{
		if(RepairDelete())
		{
			return 0x1;
		}
		return 0x0;
	}
	if(!WriteBuffer(&dwAEPToWrite, m_stPEOffsets.AddressOfEntryPoint, sizeof(DWORD), sizeof(DWORD)))
	{
		return 0x0;
	}
	return 0x2;
}

bool CISpyRepair::SetFileEnd()
{
	return SetFileEnd(static_cast<DWORD>(m_dwArgs[0]));
}

/**********************************************************************************************************                     
*  Function Name  :	SetFileEnd                                                     
*  Description    :	Sets end of the file after file is truncated 
*  SR.NO		  : VIRUSREPAIR_0014
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::SetFileEnd(DWORD dwTruncateOffset)
{
	::SetFilePointer(m_hFileHandle, dwTruncateOffset, 0, FILE_BEGIN);
	if(SetEndOfFile(m_hFileHandle))
	{
		return true;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	WriteSectionCharacteristic                                                     
*  Description    :	Writes Section's Properties in File i.e SRD & PRD
*  SR.NO		  : VIRUSREPAIR_0029
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::WriteSectionCharacteristic(WORD wSectionNo, DWORD dwAttributeValue, DWORD dwAttributeOffset)
{
	DWORD dwOffset = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + wSectionNo * 40 + dwAttributeOffset;
	return WriteBuffer(&dwAttributeValue, dwOffset, sizeof(DWORD), sizeof(DWORD));
}

/**********************************************************************************************************                     
*  Function Name  :	CalculateLastSectionProperties                                                     
*  Description    :	Calculates Last Section's Properties i.e SRD & PRD
*  SR.NO		  : VIRUSREPAIR_0017
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::CalculateLastSectionProperties()
{
	if(m_stPEHeader.NumberOfSections != 0x0)
	{
		int iLastSection = 0; 	
		for(int iSec = m_stPEHeader.NumberOfSections - 1; iSec > 0; iSec--) 
		{
			if(m_stSectionHeader[iSec].SizeOfRawData != 0x00) 
			{
				iLastSection = iSec;
				break;
			}
		}	
		if(m_stSectionHeader[iLastSection].PointerToRawData > m_dwFileSize)
		{
			m_stSectionHeader[iLastSection].PointerToRawData = m_stSectionHeader[iLastSection - 1].PointerToRawData + m_stSectionHeader[iLastSection - 1].SizeOfRawData;
			WriteSectionCharacteristic(iLastSection, m_stSectionHeader[iLastSection].PointerToRawData, SEC_PRD);
		}
		DWORD dwSRD = GetFileSize(m_hFileHandle, 0) - m_stSectionHeader[iLastSection].PointerToRawData;
		WriteSectionCharacteristic(iLastSection, dwSRD, SEC_SRD);
	}
	return CalculateImageSize();
}

/**********************************************************************************************************                     
*  Function Name  :	CalculateImageSize                                                     
*  Description    :	Calculates File's Size of Image
*  SR.NO		  : VIRUSREPAIR_0018
*  Author Name    :	Prajakta                                                                                          
*  Date           : 28 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::CalculateImageSize()
{
	if(m_stPEHeader.NumberOfSections != 0x0)
	{
		DWORD dwLastSecRVA = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress;
		DWORD dwOffset = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + ((m_stPEHeader.NumberOfSections - 1) * IMAGE_SIZEOF_SECTION_HEADER) + 0x08;
		DWORD dwLastSecVS = 0;
		if(!GetFileBuffer(&dwLastSecVS, dwOffset, 4, 4))
		{
			return false;
		}
		dwOffset += 0x08;
		DWORD dwLastSecSRD = 0;
		if(!GetFileBuffer(&dwLastSecSRD, dwOffset, 4, 4))
		{
			return false;
		}
		
		DWORD dwSecAlignment = m_stPEHeader.SectionAlignment, dwImageSize = 0;
		if(dwLastSecVS % dwSecAlignment)
		{
			dwImageSize = dwLastSecVS - (dwLastSecVS % dwSecAlignment) + dwSecAlignment;
		}
		else
		{
			dwImageSize = dwLastSecVS;     
		}
		
		dwImageSize += dwLastSecRVA;
		if (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData == 0)
		{//virut
			DWORD dwFlashDataSize = GetFileSize(m_hFileHandle, 0) - m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData;
			dwImageSize = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData + dwFlashDataSize;

			//dwImageSize = m_stSectionHeader[m_stPEHeader.NumberOfSections - 2].SizeOfRawData + m_stSectionHeader[m_stPEHeader.NumberOfSections - 2].PointerToRawData + dwFlashDataSize;
		}

		if(WriteBuffer(&dwImageSize, m_stPEOffsets.SizeOfImage, 4, 4))
		{
			return true;
		}
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	GetSecNoOfFileOff                                                     
*  Description    :	Gets the section number where the file offset lies
*  SR.NO		  : VIRUSREPAIR_0030
*  Author Name    :	Prajakta                                                                                          
*  Date           : 29 Nov 2013
**********************************************************************************************************/
WORD CISpyRepair::GetSecNoOfFileOff(DWORD dwFileOff)
{
	WORD wSec = 0;
	for(wSec = 0; wSec < m_stPEHeader.NumberOfSections; wSec++)
	{
		if(dwFileOff >= m_stSectionHeader[wSec].PointerToRawData && dwFileOff < (m_stSectionHeader[wSec].PointerToRawData + m_stSectionHeader[wSec].SizeOfRawData))
		{
			break;
		}
	}
	return wSec;
}

/**********************************************************************************************************                     
*  Function Name  :	WriteNoOfSections                                                     
*  Description    :	Writes number of sections in Image File Header
*  SR.NO		  : VIRUSREPAIR_0031
*  Author Name    :	Prajakta                                                                                          
*  Date           : 30 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::WriteNoOfSections(WORD wNoOfSections)
{
	if(!WriteBuffer(&wNoOfSections, m_stPEOffsets.NumberOfSections, sizeof(WORD), sizeof(WORD)))
	{
		return false;
	}
	return true;
}

bool CISpyRepair::FillWithZeros()
{
	return FillWithZeros(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]));
}

/**********************************************************************************************************                     
*  Function Name  :	FillWithZeros                                                     
*  Description    :	Fill's zeros in file at specific offset as per size specified
*  SR.NO		  : VIRUSREPAIR_0019
*  Author Name    :	Prajakta                                                                                          
*  Date           : 30 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::FillWithZeros(DWORD dwStartOff, DWORD dwFillWithZeroSize)
{
	DWORD dwChunk = 0x1000;
	if(dwFillWithZeroSize < dwChunk)
	{
		dwChunk = dwFillWithZeroSize;
	}
	BYTE *pbyFillZeroBuff = new BYTE[dwChunk];
	
	if(!pbyFillZeroBuff)
	{
		return false;
	}
	memset(pbyFillZeroBuff, 0, dwChunk);
	
	DWORD dwBytes2Write = dwChunk, dwBytesWritten = 0x0;
	::SetFilePointer(m_hFileHandle, dwStartOff, 0, FILE_BEGIN);

	for(DWORD dwTotalBytesWritten = 0x00; dwTotalBytesWritten < dwFillWithZeroSize; dwTotalBytesWritten	+= dwBytesWritten)
	{		
		if(dwFillWithZeroSize - dwTotalBytesWritten < dwChunk)
		{
			dwBytes2Write = dwFillWithZeroSize - dwTotalBytesWritten;
		}
		WriteFile(m_hFileHandle, pbyFillZeroBuff, dwBytes2Write, &dwBytesWritten, 0);
		if(0 == dwBytesWritten)
		{
			break;
		}
	}
	delete []pbyFillZeroBuff;
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	CalculateCheckSum                                                     
*  Description    :	Calculates file's checksum
*  SR.NO		  : VIRUSREPAIR_0020
*  Author Name    :	Prajakta                                                                                          
*  Date           : 30 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::CalculateCheckSum()
{
	DWORD dwCheckSum = 0x0, dwHeaderSum = 0x0;
	MapFileAndCheckSum(m_szFilePath, &dwHeaderSum, &dwCheckSum);	
	return WriteBuffer(&dwCheckSum, m_stPEOffsets.Checksum, sizeof(DWORD), sizeof(DWORD));
}

/**********************************************************************************************************                     
*  Function Name  :	TruncateFromAEP                                                     
*  Description    :	Truncates file from AEP
*  SR.NO		  : VIRUSREPAIR_0013
*  Author Name    :	Prajakta                                                                                          
*  Date           : 30 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::TruncateFromAEP()
{
	if(m_dwAEPMapped == m_stSectionHeader[m_stPEHeader.NumberOfSections - 0x1].PointerToRawData)
	{
		return RemoveLastSections(1, true);	
	}
	if(m_dwAEPMapped > 0)
	{
		return SetFileEnd(m_dwAEPMapped);
	}
	return false;
}

bool CISpyRepair::GetBuffer4Decryption()
{
	return GetBuffer4Decryption(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]));
}

/**********************************************************************************************************                     
*  Function Name  :	GetBuffer4Decryption                                                     
*  Description    :	Gets buffer for decryption from file 
*  SR.NO		  : VIRUSREPAIR_0022
*  Author Name    :	Prajakta                                                                                          
*  Date           : 30 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::GetBuffer4Decryption(DWORD dwOffset,DWORD dwSize)
{
	m_byBuffer = new BYTE[dwSize];
	if(!m_byBuffer)
	{
		return false;
	}
	if(!GetFileBuffer(m_byBuffer, dwOffset, dwSize, 0, &m_dwBuffSize))
		return false;
	if(m_dwBuffSize == 0x0)
	{
		return false;
	}
	return true;
}

bool CISpyRepair::RepairOptionalHeader()
{
	if(m_dwArgs[0] == 0x0)
		return false;
	bool bSetDataDirSize = (m_dwArgs[3] == 0) ? false : true;
	return RepairOptionalHeader(static_cast<int>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]), static_cast<DWORD>(m_dwArgs[2]), bSetDataDirSize);
}

/**********************************************************************************************************                     
*  Function Name  :	RepairOptionalHeader                                                     
*  Description    :	Repairs fields of Image Optional Header
*  SR.NO		  : VIRUSREPAIR_0024
*  Author Name    :	Prajakta                                                                                          
*  Date           : 2 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::RepairOptionalHeader(int iFieldNo, DWORD dwFieldValue, DWORD dwDataDirSize,bool bSetDataDirSize)
{
	DWORD dwOffset = 0x00, dwBytes2Write = sizeof(DWORD);

	switch(iFieldNo)
	{
	case 1://Offset of Optional Header
		{
			dwOffset = 0x3C;
		}
		break;
	case 2://Offset of Major Linker Version
		{
			dwOffset = m_stPEOffsets.MajorLinkerVersion;
		}
		break;
	case 3://Offset of Minor Linker Version
		{
			dwOffset = m_stPEOffsets.MinorLinkerVersion;
		}
		break;
	case 4://Size Of Code
		{
			dwOffset = m_stPEOffsets.SizeOfCode;
		}
		break;
	case 9://Base Of Data
		{
			dwOffset = m_stPEOffsets.BaseOfData;
		}
		break;
	case 0x0A://Image Base
		{
			dwOffset = m_stPEOffsets.ImageBase;
		}
		break;
	case 0x0B://Section Alignment
		{
			dwOffset = m_stPEOffsets.SectionAlignment;
		}
		break;
	case 0xD://Major Operating System Version
		{
			dwOffset = m_stPEOffsets.MajorOSVersion;
			dwBytes2Write = sizeof(WORD);
		}
	case 0x0E://Minor Operating System Version
		{
			dwOffset = m_stPEOffsets.MinorOSVersion;
			dwBytes2Write = sizeof(WORD);
		}
		break;
	case 0x13://Win32 Version Value
		{
			dwOffset = m_stPEOffsets.Win32VersionValue;
		}
		break;
	case 0x16://Checksum
		{
			dwOffset = m_stPEOffsets.Checksum;
		}
		break;
	default:
		if(iFieldNo < 0x1E)
		{
			return false;
		}
		break;
	}
	//Data Directories
	if(iFieldNo > 0x1E)
	{
		dwOffset = m_stPEOffsets.NoOfDataDirs + 0x04 + (iFieldNo - 0x1F) * 8;		
	}
	if(dwOffset < m_stPEHeader.SizeOfOptionalHeader + m_stPEOffsets.Magic)
	{		
		if(!WriteBuffer(&dwFieldValue, dwOffset, dwBytes2Write, dwBytes2Write))
		{
			return false;
		}
		
		//RVA of DATA Directories 
		if(iFieldNo > 0x1E && (dwDataDirSize || bSetDataDirSize))
		{	
			//Size of Data Directories
			if(!WriteBuffer(&dwDataDirSize, dwOffset + 4, sizeof(DWORD), sizeof(DWORD)))
			{
				return false;
			}
		}
	}
	return true;	
}

bool CISpyRepair::DwordXOR()
{
	return DwordXOR(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]), static_cast<DWORD>(m_dwArgs[2]));
}

/**********************************************************************************************************                     
*  Function Name  :	DwordXOR                                                     
*  Description    :	Allows to perform XOR operation DWORD-by-DWORD with a (DWORD) XORKey
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Prajakta                                                                                          
*  Date           : 2 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::DwordXOR(DWORD dwOffset,DWORD dwLength,DWORD XORKey)
{
	bool bReturn = false;

	DWORD *pXORBuffer = new DWORD[(dwLength)/(sizeof(DWORD))];
	if(pXORBuffer == NULL)
	{
		goto CLEANUP;
	}
	memset(pXORBuffer, 0, dwLength);

	if(!GetFileBuffer(pXORBuffer, dwOffset, dwLength, dwLength))
	{
		goto CLEANUP;
	}
	for(DWORD i = 0x0; i < (dwLength/sizeof(DWORD)); i++)
	{
		pXORBuffer[i] ^= XORKey;
	}
	if(!WriteBuffer(pXORBuffer, dwOffset, dwLength, dwLength))
	{
		goto CLEANUP;
	}
	bReturn = true;

CLEANUP:
	if(pXORBuffer != NULL)
	{
		delete []pXORBuffer;
		pXORBuffer = NULL;
	}	
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	CopyData                                                     
*  Description    :	Allows to copy data of size dwSizeOfData from dwReadOff to dwWriteOff
*  Author Name    :	Prajakta                                                                                          
*  Date           : 2 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::CopyData(DWORD dwReadOff, DWORD dwWriteOff, DWORD dwSizeOfData)
{
	if(m_byBuffer) 
	{
		delete []m_byBuffer;
		m_byBuffer = NULL;
	}
	DWORD dwChunk = 0x10000;
	if(dwSizeOfData < dwChunk)
	{
		dwChunk = dwSizeOfData;
	}
	m_byBuffer = new BYTE[dwChunk];
	if(!m_byBuffer)
		return false;

	DWORD dwBytesRead = 0;
	for(DWORD dwOffset = 0; dwOffset < dwSizeOfData; dwOffset += dwChunk)
	{		
		memset(m_byBuffer, 0, dwChunk);
		if(!GetFileBuffer(m_byBuffer, dwReadOff + dwOffset, dwChunk, 0, &dwBytesRead))
		{
			return false;
		}
		if((dwOffset + dwChunk) > dwSizeOfData || dwBytesRead != dwChunk)
		{
			dwBytesRead = dwSizeOfData - dwOffset;
		}
		if(!WriteBuffer(m_byBuffer, dwWriteOff + dwOffset, dwBytesRead, dwBytesRead))
		{
			delete [] m_byBuffer;
			m_byBuffer = NULL;
			return false;
		}
	}
	if(m_byBuffer != NULL)
	{
		delete []m_byBuffer;
		m_byBuffer = NULL;
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ReplaceOriginalData                                                     
*  Description    :	Writes/replaces data in file at dwReplaceOff from dwReadOff of size dwReplaceDataSize
*  SR.NO		  : VIRUSREPAIR_0021
*  Author Name    :	Prajakta                                                                                          
*  Date           : 2 Dec 2013
**********************************************************************************************************/
DWORD CISpyRepair::ReplaceOriginalData()
{
	if(m_dwArgs[0] >= m_dwFileSize || m_dwArgs[2] == 0x0)
	{
		if(RepairDelete())
		{
			return 0x1;
		}
		return 0x0;
	}
	if(m_dwArgs[1] >= m_dwFileSize)
		return 0x0;
	if (!CopyData(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]), static_cast<DWORD>(m_dwArgs[2])))
	{
		return 0x0;
	}
	return 0x2;
}

bool CISpyRepair::RemoveSections()
{
	if(m_dwArgs[0] == 0)
	{
		m_dwArgs[0] = 1;
	}
	bool bTruncateFlashData = (m_dwArgs[1] == 0) ? false : true;
	return RemoveLastSections((WORD)m_dwArgs[0], bTruncateFlashData);
}

/**********************************************************************************************************                     
*  Function Name  :	RemoveLastSections                                                     
*  Description    :	Removes section's & flash data if reqd.
                    Default no.of sections removed = 1
*  SR.NO		  : VIRUSREPAIR_0016
*  Author Name    :	Prajakta                                                                                          
*  Date           : 2 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::RemoveLastSections(WORD wNoOfSec2Remove, bool bTruncateFlashData)
{	
	WORD wSec2Remove = m_stPEHeader.NumberOfSections - wNoOfSec2Remove;
	while(wSec2Remove < m_stPEHeader.NumberOfSections - 1 && m_stSectionHeader[wSec2Remove].PointerToRawData == 0)
	{
		wSec2Remove++;
	}
	WORD wLastSec = m_stPEHeader.NumberOfSections - 0x1;
	DWORD dwEndOfLastSec = m_stSectionHeader[wLastSec].PointerToRawData + m_stSectionHeader[wLastSec].SizeOfRawData;				
	while(wLastSec > wSec2Remove && dwEndOfLastSec == 0)
	{
		wLastSec--;
		dwEndOfLastSec = m_stSectionHeader[wLastSec].PointerToRawData + m_stSectionHeader[wLastSec].SizeOfRawData;
	}
	if(dwEndOfLastSec == 0)
	{
		return false;
	}
	
	// Calculate truncate offset
	DWORD dwSec2RemovePRD = m_stSectionHeader[wSec2Remove].PointerToRawData; 
	if(dwEndOfLastSec < m_dwFileSize && !bTruncateFlashData)
	{
		DWORD dwFlashDataSize = GetFileSize(m_hFileHandle, 0) - dwEndOfLastSec;

		dwSec2RemovePRD = dwSec2RemovePRD - (dwSec2RemovePRD % m_stPEHeader.SectionAlignment) + m_stPEHeader.SectionAlignment;

		if(!CopyData(dwEndOfLastSec, dwSec2RemovePRD, dwFlashDataSize))
		{
			return false;
		}
		dwSec2RemovePRD  += dwFlashDataSize;
	}		
	
	::SetFilePointer(m_hFileHandle, dwSec2RemovePRD, 0, FILE_BEGIN);
	if(!SetEndOfFile(m_hFileHandle))
	{
		return false;
	}
	
	// Fill the removed section header with zeros
	DWORD dwSecHeaderOff = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + (wSec2Remove * IMAGE_SIZEOF_SECTION_HEADER);	
	if(!FillWithZeros(dwSecHeaderOff, wNoOfSec2Remove * IMAGE_SIZEOF_SECTION_HEADER))
	{
		return false;
	}

	// Rewrite number of sections in Image File Header
	if(!WriteNoOfSections(wSec2Remove))
	{
		return false;
	}
	DWORD dwBoundImportAddr = 0;
	if(GetFileBuffer(&dwBoundImportAddr,  m_stPEOffsets.NoOfDataDirs + 0x5C, sizeof(DWORD), sizeof(DWORD)))
	{
		if(dwBoundImportAddr >= dwSecHeaderOff && dwBoundImportAddr < dwSecHeaderOff + IMAGE_SIZEOF_SECTION_HEADER)
		{
			RepairOptionalHeader(42, 0, 0, true);			
		}
	}
	return CalculateImageSize();
}

bool CISpyRepair::TruncateFile()
{
	bool bTruncateFlashData = (m_dwArgs[1] == 0) ? false : true;
	return TruncateFile(static_cast<DWORD>(m_dwArgs[0]), bTruncateFlashData);
}

/**********************************************************************************************************                     
*  Function Name  :	TruncateFile                                                     
*  Description    :	Truncates file from dwTruncateOff & also copies the flash data before truncating the file
*  SR.NO		  : VIRUSREPAIR_0015
*  Author Name    :	Prajakta                                                                                          
*  Date           : 3 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::TruncateFile(DWORD dwTruncateOff, bool bTruncateFlashData)
{
	//Check if Flash data is present in file.
	//If yes calculate flash data start offset & size
	DWORD dwStartOfFlashData = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData;
	DWORD dwSizeOfFlashData = 0x0;
	bool bFlashDataExists = false;
	if(m_dwFileSize > dwStartOfFlashData) 
	{
		dwSizeOfFlashData = m_dwFileSize - dwStartOfFlashData;
		bFlashDataExists  = true;
	}
	
	// Get the section number where truncate offset lies
	WORD wTruncateSec =  GetSecNoOfFileOff(dwTruncateOff);
	if(OUT_OF_FILE != wTruncateSec)
	{		
		// If the truncate offset matches with start of the section i.e PRD then remove that section
		if(dwTruncateOff == m_stSectionHeader[wTruncateSec].PointerToRawData)
		{
			return RemoveLastSections(m_stPEHeader.NumberOfSections  - wTruncateSec, bTruncateFlashData);
		}
		// If the truncate offset is not in the last section then remove the sections below it
		if(wTruncateSec < m_stPEHeader.NumberOfSections - 1)
		{
			RemoveLastSections(m_stPEHeader.NumberOfSections  - wTruncateSec - 1, bTruncateFlashData);
		}
		// Reduce the size of raw data of the section
		WriteSectionCharacteristic(wTruncateSec, dwTruncateOff - m_stSectionHeader[wTruncateSec].PointerToRawData, SEC_SRD);

		// Move the flash data before truncating the file
		if(bFlashDataExists && !bTruncateFlashData)
		{
			dwStartOfFlashData = m_stSectionHeader[wTruncateSec].PointerToRawData + m_stSectionHeader[wTruncateSec].SizeOfRawData;			
			//dwSec2RemovePRD = dwSec2RemovePRD - (dwSec2RemovePRD % m_stPEHeader.FileAlignment) + m_stPEHeader.FileAlignment;
			dwTruncateOff = dwTruncateOff - (dwTruncateOff % m_stPEHeader.SectionAlignment) + m_stPEHeader.SectionAlignment;
			if(!CopyData(dwStartOfFlashData, dwTruncateOff, dwSizeOfFlashData))
			{
				return false;
			}
			dwTruncateOff  += dwSizeOfFlashData;
		}	
	}
	// Truncate the file
	::SetFilePointer(m_hFileHandle, dwTruncateOff, 0, FILE_BEGIN);
	if(SetEndOfFile(m_hFileHandle))
	{
		return CalculateImageSize();
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	Search4MZPE                                                     
*  Description    :	Searches for MZ & PE from a specific start-end offset 
*  Author Name    :	Prajakta                                                                                          
*  Date           : 3 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::Search4MZPE(BYTE *bySearchString, DWORD dwStringLen, DWORD dwStartAddr, DWORD dwEndAddr)
{
	const int SEARCH_BUFF_SIZE = 0x1000;
	BYTE bySearchBuffer[SEARCH_BUFF_SIZE];

	// Search the string from dwStartAddr to the dwEndAddrs in chunks of 0x1000
	for(DWORD dwOffset = dwStartAddr; dwOffset < dwEndAddr; dwOffset += SEARCH_BUFF_SIZE)
	{
		memset(bySearchBuffer, 0, SEARCH_BUFF_SIZE);
		DWORD dwBytesRead = 0x0;
		if(!GetFileBuffer(bySearchBuffer, dwOffset, SEARCH_BUFF_SIZE, 0,&dwBytesRead))
		{
			return false;
		}
		for(unsigned int iOffset = 0x00; iOffset < dwBytesRead; iOffset++)
		{
			if(!memcmp(&bySearchBuffer[iOffset], bySearchString, dwStringLen))
			{
				DWORD dwFoundOffset = dwOffset + iOffset;
				DWORD dwPEOffset = 0, dwPE = 0;
				GetFileBuffer(&dwPEOffset, dwFoundOffset + 0x3C, sizeof(DWORD));

				if (dwPEOffset < m_dwFileSize)
				{
					GetFileBuffer(&dwPE, dwFoundOffset + dwPEOffset, sizeof(WORD));
					if (dwPE == 0x4550)
					{
						m_dwReturnValue[0] = dwFoundOffset;
						return true;
					}
				}
			}
		}
	}
	return false; 
}

/**********************************************************************************************************                     
*  Function Name  :	SearchString                                                     
*  Description    :	Searches for a specific string from start offset till end of file in chunks of 0x1000  
					Default search string is MZ if no string is present in expression
					Return ID's:
						Function Failed = 0x00
						RepairDelete	= 0x01
						GetOffset		= 0x02
*  SR.NO		  : VIRUSREPAIR_0025
*  Author Name    :	Prajakta                                                                                          
*  Date           : 3 Dec 2013
**********************************************************************************************************/
DWORD CISpyRepair::SearchString()
{
	DWORD	dwBytesRead = 0;
	if(m_dwArgs[0] == 0x0 || m_dwArgs[0] > m_dwFileSize)
	{
		if(RepairDelete())
		{
			return 0x01;
		}
		return 0x00;
	}
	
	// Default search string is MZ if no string is present in the expression
	bool bCheckForMZ = false;
	if(!m_ibyArgLength)
	{
		bCheckForMZ = true;
		m_ibyArgLength = 0x2;
		memcpy(m_byArg, "MZ", m_ibyArgLength);
	}
	const int SEARCH_BUFF_SIZE = 0x1000;
	BYTE bySearchBuffer[SEARCH_BUFF_SIZE];
	WORD wLastSec = m_stPEHeader.NumberOfSections - 0x1;
	
	// Search the string from file offset send to the end of file in chunks of 0x1000
	for (DWORD dwOffset = static_cast<DWORD>(m_dwArgs[0]); dwOffset < m_dwFileSize; dwOffset += SEARCH_BUFF_SIZE)
	{
		memset(bySearchBuffer, 0, SEARCH_BUFF_SIZE);
		if(!GetFileBuffer(bySearchBuffer, dwOffset, SEARCH_BUFF_SIZE, 0, &dwBytesRead))
		{
			return 0x00;
		}
		for(DWORD iOffset = 0x00; iOffset < dwBytesRead; iOffset++)
		{
			if(!memcmp(&bySearchBuffer[iOffset], m_byArg, m_ibyArgLength))
			{
				m_dwReturnValue[0] = dwOffset + iOffset;
				return 0x02;
			}
		}
	}
	if(bCheckForMZ)
	{		
		int iCnt;
		for(iCnt = wLastSec; iCnt > 0; iCnt--)
		{	
			if(0 != m_stSectionHeader[iCnt].SizeOfRawData)
			{
				break;
			}
		}
		DWORD dwEndAddr = m_stSectionHeader[iCnt].PointerToRawData + m_stSectionHeader[iCnt].SizeOfRawData;
		if(Search4MZPE(m_byArg, m_ibyArgLength, m_stSectionHeader[iCnt].PointerToRawData, dwEndAddr))
		{
			return 0x02;
		}
	}
	if(m_dwArgs[1] == 1 || bCheckForMZ)
	{
		if(RepairDelete())
		{
			return 0x01;
		}
	}
	return 0x00;
}

/**********************************************************************************************************                     
*  Function Name  :	GetParameters                                                     
*  Description    :	To get parameters from the expression that are required to repair file 
*  SR.NO		  : VIRUSREPAIR_0008
*  Author Name    :	Prajakta                                                                                          
*  Date           : 4 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::GetParameters(CString csParam)
{
	CString csParameters = csParam;
	CStringA csToEval;
	int iStart, iFurther, iLen, iArgs;
	
	iStart = iFurther = iLen = iArgs = 0;
	memset(m_dwArgs, 0, sizeof(m_dwArgs));

	memset(m_byArg, 0, sizeof(m_byArg));
	m_ibyArgLength = 0x0;
	m_iStep = 0;
	csParameters = csParameters.Tokenize(_T(","), m_iStep);

	while(csParameters != "")
	{
		iLen = csParameters.GetLength();

		iFurther = csParameters.Find(_T(']'), iStart);
		if(iFurther == -1)
			break;

		if((iFurther + 1) > iLen)
		{
			return false;
		}

		if((csParameters.GetAt(iFurther + 1) == _T('[')) || (csParameters.GetAt(iFurther + 1) == _T('M')) 
			||(iLen == iFurther + 1))
		{
			csToEval = csParameters.Mid(0, iFurther + 1);
			
			m_dwArgs[iArgs] = EvaluateExpression(csToEval);
			iArgs++;

			csParameters = csParameters.Mid(iFurther + 1);
			iStart = 0;
		}
		else
		{
			iStart = iFurther + 1;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	EvaluateExpression                                                     
*  Description    :	Returns result of the expression after evaluating 
*  SR.NO		  : VIRUSREPAIR_0009
*  Author Name    :	Prajakta                                                                                          
*  Date           : 4 Dec 2013
**********************************************************************************************************/
DWORD_PTR CISpyRepair::EvaluateExpression(const char * str)
{
	DWORD_PTR dwResult = 0;
	DWORD_PTR dwValue = 0;
	DWORD dwOffset = 0;

	int index = 0;
	const char * start = 0;
	const char * end = 0;
	char szNumber[20]={0};
	char opr = 0;	
	bool bMapResult = false;

	if(*str == 'M')
	{
		bMapResult = true;
		str++;
	}

	while(str && *str)
	{
		if(*str == '[')
		{
			str++;
			start = str;
			while(*str && *str != ']')
			{
				end = str;
				str++;
			}
			end++;

			switch(*start)
			{
				// To get AEP Mapped,AEP UnMapped,Value from an offset ahead of AEP
			case 'A' :
				{
					start ++;
					//AM
					if(*start == 'M')
					{
						if(start + 1 == end)
						{
							dwValue = m_dwAEPMapped; 
						}
						else
						{
							start++;
							if(end - start < sizeof(szNumber))
							{
								DWORD_PTR dwAEP = 0;
								strncpy_s(szNumber, sizeof(szNumber), start, end - start);
								dwValue = atoi(szNumber);
								dwAEP = m_dwAEPMapped;
								GetFileBuffer(&dwValue, static_cast<DWORD>(dwAEP + dwValue), sizeof(DWORD));
							}
							else
							{
								; 
							}
						}
					}
					//AU
					else if(*start == 'U')
					{
						if(start + 1 == end)
						{
							dwValue =  m_stPEHeader.AddressOfEntryPoint;
						}
						else
						{
							start++;
							if(end - start < sizeof(szNumber))
							{
								strncpy_s(szNumber, sizeof(szNumber), start, end - start);		
								dwValue = atoi(szNumber);
								GetFileBuffer(&dwValue, m_stPEHeader.AddressOfEntryPoint + static_cast<DWORD>(dwValue), sizeof(DWORD));
							}
							else
							{
								; 
							}
						}
					}
				}
				break;
				// To get last section PRD
			case 'P' :
				{
					start++;
					szNumber[0]= 0;
					index = atoi(szNumber);

					if(m_stPEHeader.NumberOfSections >= index - 1)
					{
						index = m_stPEHeader.NumberOfSections - index - 1;
					}
					if(start == end)
					{
						dwValue = m_stSectionHeader[index].PointerToRawData;
					}
				}
				break;
				// To get last section SRD
			case 'D' :
				{
					start++;
					szNumber[0]= 0;
					index = atoi(szNumber);

					if(m_stPEHeader.NumberOfSections >= index - 1)
					{
						index = m_stPEHeader.NumberOfSections - index - 1;
					}					
					if(start == end)
					{
						dwValue = m_stSectionHeader[index].SizeOfRawData;
					}					
				}
				break;
				// To get ImageBase
			case 'I' :
				{
					start++;
					dwValue = m_stPEHeader.ImageBase;
				}
				break;
			    // To get Offset 2 new EXE Header
			case 'H' :
				{
					start++;
					dwValue = m_stPEHeader.e_lfanew;
				}
				break;
				// To get flash data size
			case 'Z': 
				{
					start ++;
					int iSecNo = m_stPEHeader.NumberOfSections - 1;
					for(int i = iSecNo; i >= 0; i--)
					{
						if(m_stSectionHeader[i].SizeOfRawData)
						{
							DWORD dwFlashDataStart = m_stSectionHeader[i].PointerToRawData + 
													m_stSectionHeader[i].SizeOfRawData;
							if(m_dwFileSize > dwFlashDataStart)
							{
								dwValue = m_dwFileSize - dwFlashDataStart;
							}
							else
							{
								dwValue = 0;
							}
							break;
						}
					}
				}
				break;
				// To get value of previous function
			case 'R':
				{
					start ++;					
					dwValue = 0;					
					if(start == end)
					{
						dwValue = m_dwReturnValue[0];
					}
					else
					{
						if(((end - start) + 1) < sizeof(szNumber))
						{
							strncpy_s(szNumber, sizeof(szNumber), start,((end - start) + 1));
							index = atoi(szNumber);
							dwValue = m_dwReturnValue[index];
						}
					}	
				}
				break;
				// To pass string in expression 
			case 'Q' :
				{
					start ++;
					if(end - start < MAX_INPUT_STR_PARAM)
					{		
						memset(m_byArg, 0x00, MAX_INPUT_STR_PARAM);
						
						char szBuff[3]={0};
						char *pHex = NULL;
						
						m_ibyArgLength = 0;
						for(int i = 0; i < end - start; i += 2)
						{
							szBuff[0] = start[i];
							szBuff[1] = start[i+1];
							szBuff[2] = '\0';
							int iChar = strtol(szBuff, &pHex, 0x10);
							m_byArg[m_ibyArgLength++] = iChar;
						}
					}					
				}
				break;
			    // To get FileSize (FL), FileName (FN), Value from an offset from the end of the file (FL - offset)
			case 'F' :
				{
					start ++;
					if(*start == 'L')
					{
						dwValue = m_dwFileSize;
						start ++;
						if(*start == '-')
						{
							start++;
							if(end - start < sizeof(szNumber))
							{
								strncpy_s(szNumber, sizeof(szNumber), start, end - start);
								dwValue = atoi(szNumber);
								GetFileBuffer(&dwValue, m_dwFileSize - static_cast<DWORD>(dwValue), sizeof(DWORD));
							}
							else
							{
								; 
							}
						}
						else
						{
							;
						}
					}
					else if(*start == 'N')
					{
						dwValue = m_csFilePath.GetLength() - m_csFilePath.ReverseFind('\\') - 1;
					}
					else
					{
						szNumber[0]= 0;
						index = atoi(szNumber);

						if(index >= m_stPEHeader.NumberOfSections)
						{
							break; 
						}

						if(start == end)
						{
							dwValue = m_stSectionHeader[index].PointerToRawData;
						}
						else
						{
							if(end - start < sizeof(szNumber))
							{
								memset(szNumber, 0, sizeof(szNumber));
								strncpy_s(szNumber, sizeof(szNumber), start, end - start);
								dwValue = atoi(szNumber);
								GetFileBuffer(&dwValue, m_stSectionHeader[index].PointerToRawData + static_cast<DWORD>(dwValue), sizeof(DWORD));
							}
							else
							{
								; 
							}
						}

					}
				}
				break;
				// To pick BYTE,WORD,DWORD,Mapped value
			case 'C' :
				{
					start++;
					DWORD dwBytes2Read = 4;
					if(*start == 'B')
					{
						start++;
						dwBytes2Read = 1;
						dwOffset = static_cast<DWORD>(dwResult);
						dwValue = dwResult = 0;
						GetFileBuffer(&dwResult,dwOffset,dwBytes2Read);
					}
					else if(*start == 'W')
					{
						start++;
						dwBytes2Read = 2;
						dwOffset = static_cast<DWORD>(dwResult);
						dwValue = dwResult = 0;
						GetFileBuffer(&dwResult,dwOffset,dwBytes2Read);
					}					
					if(*start == 'M')
					{
						dwOffset = static_cast<DWORD>(dwResult);
						dwValue = dwResult = 0;
						RVA2FileOff(dwOffset, &dwOffset);
						GetFileBuffer(&dwResult, dwOffset, dwBytes2Read);
					}
					else
					{
						if(*start == 'D')
						{
							dwOffset = static_cast<DWORD>(dwResult);
							dwValue = dwResult = 0;
							GetFileBuffer(&dwResult, dwOffset, dwBytes2Read);//dwBytes2Read = 0x4
						}
						else
						if (*start == 'E')
						{
							start++;
							if (((end - start) + 1)< sizeof(szNumber))
							{
								strncpy_s(szNumber, sizeof(szNumber), start, ((end - start) + 1));
								dwValue = atoi(szNumber);
							}
							else
							{
								;
							}

							dwOffset = static_cast<DWORD>(dwResult + dwValue);
							dwValue = dwResult = 0;
							RVA2FileOff(dwOffset, &dwOffset);
							GetFileBuffer(&dwValue, dwOffset, dwBytes2Read);//dwBytes2Read = 0x4
						}
						else if(*start == '-')
						{
							start++;
							if(((end - start) + 1) < sizeof(szNumber))
							{
								strncpy_s(szNumber, sizeof(szNumber), start, ((end - start) + 1));
								dwValue = atoi(szNumber);
								dwValue -= dwValue; 
							}

						}
						else
						{
							if(((end - start) + 1)< sizeof(szNumber))
							{
								strncpy_s(szNumber, sizeof(szNumber), start,((end - start) + 1));
								dwValue = atoi(szNumber);
							}
							else
							{
								; 
							}
						}
					}
				}
				break;
				// To save mapped/unmapped evaluated value of function 
			case 'S':
				{
					start++;
					dwValue = 0;					
					if(start == end)
					{
						m_dwSaveArgs[m_iSaveArg] = dwResult;
						m_iSaveArg++;
					}
					else if(*start == 'M')
					{
						//RVA2FileOff(dwResult,&dwResult);
						dwResult = GetMappedOff(static_cast<DWORD>(dwResult));
						m_dwSaveArgs[m_iSaveArg] = dwResult;
						//m_dwReturnValue[0] = dwResult;
						m_dwStartOffset = (DWORD )dwResult;
						m_iSaveArg++;
					}
					else
					{
						if(((end - start) + 1) < sizeof(szNumber))
						{
							strncpy_s(szNumber, sizeof(szNumber), start,((end - start) + 1));
							index = atoi(szNumber);
							dwValue = m_dwSaveArgs[index];
						}
					}									
				}
				break;			
				// To get EntryPoint
			case 'E':
				{
					start++;
					dwValue = m_stPEHeader.AddressOfEntryPoint;
					if (*start == 'V')
					{
						;
					}
					else
					{
					
						RVA2FileOff(static_cast<DWORD>(dwValue), &dwOffset);
						dwValue = dwOffset;
					}
				}
				break;
				//GetLineNumber of last section
			case 'L':
				{
					start++;
					szNumber[0] = 0;
					index = atoi(szNumber);

					if (m_stPEHeader.NumberOfSections >= index - 1)
					{
						index = m_stPEHeader.NumberOfSections - index - 1;
					}
					if (start == end)
					{
						dwValue = m_stSectionHeader[index].PointerToLinenumbers;
					}
				}
				break;
				//To get Sizeof code
			case 'T':
				{
					start++;
					dwValue = m_stPEHeader.SizeOfCode;
				}
				break;
			case 'O':
				{
					start++;
					dwValue = m_stPEOffsets.AddressOfEntryPoint;
				}
				break;
				//get .rsrc sizeof raw data
			case 'U':
				{
					start++;
					for (index = 0; index < m_stPEHeader.NumberOfSections; index++)
					{
						if (*(DWORD *)&m_stSectionHeader[index].Name == 0x7273722E)
						{
							dwValue = m_stSectionHeader[index].SizeOfRawData;
						}
					}					
				}
				break;
				//last section VA
			case 'V':
				{
					start++;
					szNumber[0] = 0;
					index = atoi(szNumber);

					if (m_stPEHeader.NumberOfSections >= index - 1)
					{
						index = m_stPEHeader.NumberOfSections - index - 1;
					}
					if (start == end)
					{
						dwValue = m_stSectionHeader[index].VirtualAddress;
					}
				}
				break;

			case 'Y':
				{
					dwValue = m_dwStartOffset;
				}
				break;
			
			//To get last SectionHeader offset
			//for file-infectors Agent.DP, QVod.C, QVod.G, DZan, Henky
			case 'B':
				{
					start++;
					szNumber[0] = 0;
					index = atoi(szNumber);

					if (m_stPEHeader.NumberOfSections >= index - 1)
					{
						index = m_stPEHeader.NumberOfSections - index - 1;
					}
					if (start == end)
					{
						DWORD dwSecHeaderOff = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + (index * IMAGE_SIZEOF_SECTION_HEADER);
						dwValue = dwSecHeaderOff;
					}
					else
					{
						if ((end - start) < sizeof(szNumber))
						{
							DWORD dwSecHeaderOff = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + (index * IMAGE_SIZEOF_SECTION_HEADER);
						
							memset(szNumber, 0, sizeof(szNumber));
							strncpy_s(szNumber, sizeof(szNumber), start, end - start);
							dwValue = atoi(szNumber);
							GetFileBuffer(&dwValue, dwSecHeaderOff + static_cast<DWORD>(dwValue), sizeof(DWORD));
						}
						else
						{
							;
						}
					}

				}
				break;

			default:
				{
					; 
				}
				break;
			} //end of switch

			if(opr == 1) //Operation: +
			{
				dwResult = dwValue + dwResult;
			}
			else if(opr == 2) //Operation: -
			{
				if(dwResult > dwValue)
					dwResult = dwResult - dwValue;
				else
					dwResult = dwValue - dwResult;
			}
			else if(opr == 3) //Operation: ^
			{
				dwResult = dwResult ^ dwValue;
			}
			else if(opr == 4) //Operation: ~
			{
				dwResult = ~dwValue;
			}
			else if(opr == 5) //Operation: >
			{
				//dwResult >>= dwValue;
				//dwResult = dwResult >> dwValue | dwResult << (32 - dwValue);//this also works
				dwResult = _lrotr(static_cast<unsigned long>(dwResult), static_cast<int>(dwValue));
			}			
			else if(opr == 6) //Operation: * 
			{
			  dwResult = dwResult * dwValue;
			}
			else if(opr == 7) //Operation: |
			{
			  dwResult = dwResult | dwValue;
			}
			else if(opr == 8) //Operation: <
			{
				//dwResult <<= dwValue;
				dwResult = dwResult << dwValue | dwResult >> (32 - dwValue);//this also works
				//dwResult = _lrotl(dwResult,dwValue);
			}	
			else
			{
				dwResult = dwValue;
			}
		}

		if(*str == '+')
		{
			opr = 1;
		}
		else if(*str == '-')
		{
			opr = 2;
		}
		else if(*str == '^')
		{
			opr = 3;
		}
		else if(*str == '~')
		{
			opr = 4;
		}
		else if(*str == '>')
		{
			opr = 5;
		}
		else if(*str == '*')
		{
		   opr = 6;
		}
		else if(*str == '|')
		{
		   opr = 7;
		}
		else if(*str == '<')
		{
			opr = 8;
		}
		str++;
	}

	if(bMapResult)
	{
		dwResult = GetMappedOff(static_cast<DWORD>(dwResult));
	}
	return (dwResult);
}

bool CISpyRepair::ReturnDecryptedValue()
{
	return ReturnDecryptedValue(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]), static_cast<DWORD>(m_dwArgs[2]));
}

/**********************************************************************************************************                     
*  Function Name  :	ReturnDecryptedValue                                                     
*  Description    :	Returns original value after performing decryption.
*  SR.NO		  : VIRUSREPAIR_0026
*  Author Name    :	Prajakta                                                                                          
*  Date           : 9 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::ReturnDecryptedValue(DWORD dwCaseNo,DWORD dwSubCaseNo,DWORD dwKey)
{
	switch (dwCaseNo)
	{
		case 1:
			{	
				if(!m_dwBuffSize)
				{
					return false;
				}
				BYTE byDecryptionKey = LOBYTE(dwKey);
				DWORD dwKeyObt = 0;
				QWORD dwTemp = 0, dwTemp1 = 0;
				if (dwSubCaseNo == 7)
				{
					byDecryptionKey = 0;
				}
				else if (dwSubCaseNo == 8)
				{
					DWORD dwReadByte = 0;		
					if (m_dwBuffSize > 0x4D2)
					{
						dwKeyObt = *(DWORD *)&m_byBuffer[0x4D2];
					}
				}
				for (int i = 0; i < static_cast<int>(m_dwBuffSize); i++)
				{
					//SubCases for case 1
					if (dwSubCaseNo == 1)
					{
						m_byBuffer[i] += (BYTE)dwKey;
					}
					else if (dwSubCaseNo == 2)
					{
						m_byBuffer[i] ^= (BYTE)dwKey;
					}
					else if (dwSubCaseNo == 3)
					{
						m_byBuffer[i] -= (BYTE)dwKey;
					}
					else if (dwSubCaseNo == 4)
					{  					
						m_byBuffer[i] ^=  byDecryptionKey;
						byDecryptionKey -= 1;
					}
					else if (dwSubCaseNo == 5)
					{
						m_byBuffer[i] =  ~m_byBuffer[i];
					}
					else if (dwSubCaseNo == 6) //Mudant.887 & Bondage.968.A
					{
						byDecryptionKey += HIBYTE(dwKey);
						m_byBuffer[i] ^= byDecryptionKey;
					}
					else
					if (dwSubCaseNo == 7)
					{//Hidrag	- Jyotsna	- 25-04-2015
						m_byBuffer[i] -= byDecryptionKey;
						if (byDecryptionKey == 0xFF)
						{
							byDecryptionKey = 0;
						}
						else
						{
							byDecryptionKey++;
						}
					}
					else
					if (dwSubCaseNo == 8)
					{//Neshta.A - Jyotsna - 28-04-2015
						if (i == 0x3E8)
						{
							break;
						}
						dwTemp = (DWORD)(dwKeyObt * 0x8088405);
						dwTemp++;
						dwKeyObt = (DWORD)dwTemp;
						dwTemp1 = DWORD((dwTemp * 0xFF) >> 32);
						m_byBuffer[i] = (BYTE)((m_byBuffer[i]) ^ (BYTE)(dwTemp1 & 0x000000FF));					
					}
				}
				m_dwReturnValue[0] = *(DWORD *)&m_byBuffer[0];
			}
			break;
		case 2://Win32.Evar.3582
			{
				for(DWORD i = 0x0; i < m_dwBuffSize; i+= 4)
				{
					DWORD dwRotateLeftKey = static_cast<DWORD>(m_dwArgs[2]);
					*(DWORD *)&m_byBuffer[i] = _lrotl((*(DWORD *)&m_byBuffer[i]),dwRotateLeftKey);
				}
				m_dwReturnValue[0] = *(DWORD *)&m_byBuffer[3];
			}
			break;
		case 3://Win32.Artelad.2173
			{
				DWORD dwXORKey = static_cast<DWORD>(m_dwArgs[1]);
				DWORD dwADDKey = static_cast<DWORD>(m_dwArgs[2]);
				DWORD dwROLCounter = 0x21 % sizeof(DWORD);
				for(DWORD i = 0x0;i < m_dwBuffSize; i += 0x4)
				{
					*((DWORD *) &m_byBuffer[i]) = _lrotl(*((DWORD *) &m_byBuffer[i]), dwROLCounter);
					*((DWORD *) &m_byBuffer[i]) = 0x00 - *((DWORD *) &m_byBuffer[i]);
					*((DWORD *) &m_byBuffer[i]) = (~(*((DWORD *) &m_byBuffer[i]))); 
					*((DWORD *) &m_byBuffer[i]) = *((DWORD *) &m_byBuffer[i]) ^ dwXORKey;
					*((DWORD *) &m_byBuffer[i]) = *((DWORD *) &m_byBuffer[i]) + dwADDKey;
				}
				m_dwReturnValue[0] = *(DWORD *)&m_byBuffer[3];
			}
			break;
		case 4://Magic.1590
			{
				DWORD dwEAX = 0x0, dwECX = 0x0, dwBytesRead = 0x0;

				GetFileBuffer(&dwEAX,m_dwAEPMapped + 6, 1);
				BYTE *byValEAX;
				WORD *wValECX, *wValEAX;
				DWORD dwCounter = dwECX = static_cast<DWORD>(m_dwArgs[2]), dwVal = 0x0;
				for(DWORD dwCnt = 0; dwCnt < dwCounter; dwCnt++)   
				{
					// byte ^= AL
					byValEAX			=(BYTE *)&dwEAX; 
					m_byBuffer[dwCnt] ^= byValEAX[0];  

					// AX += CX
					wValEAX				=(WORD *)&dwEAX;
					wValECX				=(WORD *)&dwECX;
					wValEAX[0] 			= wValEAX[0] + wValECX[0]; 	
					dwEAX				= wValEAX[0];
					//CX--
					dwECX--;
				}
				DWORD dwMul = 0x1;
				for(int j = 0x1C7; j <= 0x1CA; j++, dwMul *= 0x100)
				{
					dwVal += m_byBuffer[j] * dwMul;
				}
				m_dwReturnValue[0] = m_stPEHeader.AddressOfEntryPoint + dwVal;
			}
			break;
		default:
			break;
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	RepairVirusStub                                                     
*  Description    :	Checks & deletes virus stub.
					m_dwArgs[0]:FileSize
					m_dwArgs[1]:String offset in file
					m_dwArgs[2]:Unique String
*  SR.NO		  : VIRUSREPAIR_0028
*  Author Name    :	Prajakta                                                                                          
*  Date           : 20 Dec 2013
**********************************************************************************************************/
DWORD CISpyRepair::RepairVirusStub()
{
	if(m_byBuffer)
	{
		delete []m_byBuffer;
		m_byBuffer = NULL;
	}
	if((m_dwFileSize >= (m_dwArgs[0]- 0x50) && m_dwFileSize <= (m_dwArgs[0] + 0x100)) ||
		(m_dwArgs[1] > m_dwArgs[0]) && m_dwArgs[2])           
	{
		if(m_dwArgs[0] == 0 || m_dwArgs[1] == 0)
		{
			return 0x0;
		}
		else
		{
			m_byBuffer = new BYTE[m_ibyArgLength];
			if(!m_byBuffer)
				return 0x0;
			memset(m_byBuffer, 0, m_ibyArgLength);
			if (GetFileBuffer(m_byBuffer, static_cast<DWORD>(m_dwArgs[1]), m_ibyArgLength, m_ibyArgLength))
			{
				if(memcmp(m_byBuffer, m_byArg, m_ibyArgLength) == 0)
				{
					if(RepairDelete())
					{
						return 0x1;
					}
					return 0x0;
				}
			}
		}
	}
	return 0x2;
}

/**********************************************************************************************************                     
*  Function Name  :	RepairRenamer                                                     
*  Description    :	To repair renamer type viruses
					Case 0: if extension is appended to original file name
					Case 1: if string is prepended to original file name
*  SR.NO		  : VIRUSREPAIR_0027
*  Author Name    :	Prajakta                                                                                          
*  Date           : 6 Jan 2014
**********************************************************************************************************/
bool CISpyRepair::RepairRenamer()
{
	wchar_t szExtension[MAX_INPUT_STR_PARAM] = {0};
	if(m_ibyArgLength)
	{
		size_t iSize = 0;
		mbstowcs_s(&iSize,szExtension,MAX_INPUT_STR_PARAM,(const char *)m_byArg,MAX_INPUT_STR_PARAM);
	}
	CString csRenamedFilePath = m_csFilePath;
	
	//Read the virus file before deleting
	const int CHK_SIZE = 0x300;
	BYTE byVirusFileData[CHK_SIZE] = {0};
	DWORD dwBytes2Read = m_dwFileSize > CHK_SIZE ? CHK_SIZE : m_dwFileSize;
	if(!GetFileBuffer(byVirusFileData,0,dwBytes2Read,0))
		return false;
	
	DWORD dwVirusFileSize = m_dwFileSize;
	// delete virus file
	if(!RepairDelete())
		return false;
	
	switch(m_dwArgs[0])
	{
		//Extension appended to original file name
		case 0:
			{
				int iLength = csRenamedFilePath.ReverseFind('.');
				if(-1 == iLength)
					return true;
				csRenamedFilePath = csRenamedFilePath.Left(iLength);
				csRenamedFilePath.Append(szExtension);
				if(PathFileExists(csRenamedFilePath)) // check for existence of renamed file
				{
					break;
				}
				break;
			}
			
			//String prepended to original file name
		case 1:
			{
				int iLength = csRenamedFilePath.ReverseFind('\\');
				if(-1 == iLength)
					return true;
				iLength++;
				CString csFileName = csRenamedFilePath.Right(csRenamedFilePath.GetLength() - iLength);			
				csRenamedFilePath = csRenamedFilePath.Left(iLength);
				csRenamedFilePath.AppendFormat(L"%s%s",szExtension, csFileName);
				if(PathFileExists(csRenamedFilePath)) // check for existence of renamed file
				{
					break;
				}
				break;
			}
		default:
			break;
	}

	DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	SetFileAttributes(csRenamedFilePath, FILE_ATTRIBUTE_NORMAL);	
	HANDLE	hOriginalFileHandle = CreateFile(csRenamedFilePath, dwDesiredAccess, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);	
	if (INVALID_HANDLE_VALUE != hOriginalFileHandle)
	{
		//Delete renamed file if its virus stub
		DWORD dwBytesRead = 0;
		if(dwVirusFileSize == GetFileSize(hOriginalFileHandle, 0))
		{
			// Check if the file is virus stub
			BYTE byCleanFileData[CHK_SIZE] = {0};			
			SetFilePointer(hOriginalFileHandle, 0, 0, FILE_BEGIN);
			ReadFile(hOriginalFileHandle, byCleanFileData, dwBytes2Read, &dwBytesRead, 0x00);
			if(dwBytesRead == dwBytes2Read)
			{
				if(memcmp(byCleanFileData, byVirusFileData, dwBytes2Read) == 0)
				{
					if (INVALID_HANDLE_VALUE != hOriginalFileHandle)
					{
						CloseHandle(hOriginalFileHandle);
					}
					DeleteFile(csRenamedFilePath);
					return true;
				}
			}
		}
		
		DWORD	dwMZ = 0;
		SetFilePointer(hOriginalFileHandle, 0, 0, FILE_BEGIN);
		ReadFile(hOriginalFileHandle, &dwMZ, sizeof(WORD), &dwBytesRead, 0x00);
		if (INVALID_HANDLE_VALUE != hOriginalFileHandle)
		{
			CloseHandle(hOriginalFileHandle);
		}
		if(dwBytesRead == sizeof(WORD))
		{
			if(dwMZ == 0x5A4D)
			{
				MoveFile(csRenamedFilePath, m_csFilePath);
			}
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ReInitializeParam                                                     
*  Description    :	To reinitailize the parameters
*  Author Name    :	Prajakta                                                                                          
*  Date           : 27 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::ReInitializeParam()
{
	m_iSaveArg = 0;
	memset(m_dwSaveArgs, 0, sizeof(m_dwSaveArgs));
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ReplaceDecryptedDataInFile                                                     
*  Description    :	Replaces decrypted data from buffer to file as per size specified.
*  SR.NO		  : VIRUSREPAIR_0032
*  Author Name    :	Prajakta                                                                                          
*  Date           : 4 Jan 2014
**********************************************************************************************************/
bool CISpyRepair::ReplaceDecryptedDataInFile()
{
	DWORD dwWriteOffset = static_cast<DWORD>(m_dwArgs[0]);
	DWORD dwBytes2Write = static_cast<DWORD>(m_dwArgs[1]);
	if(dwWriteOffset == OUT_OF_FILE || dwBytes2Write == 0x0)
		return false;
	if(m_byBuffer == NULL)
		return false;

	if(!WriteBuffer(m_byBuffer, dwWriteOffset, dwBytes2Write, dwBytes2Write))
	{
		return false;
	}
	return true;
}
bool CISpyRepair::GetAddrToGetEntryData()
{
	return GetAddrToGetEntryData((DWORD)m_dwArgs[1], (DWORD)m_dwArgs[2], (DWORD)m_dwArgs[3]);
}
/**********************************************************************************************************
*  Function Name  :	GetAddrToGetEntryData
*  Description    :	Fetch the addres after decryption
*  SR.NO		  : VIRUSREPAIR_0032
*  Author Name    :	Jyotsna
*  Date           : 19 Mar 2015
**********************************************************************************************************/
bool CISpyRepair::GetAddrToGetEntryData(DWORD dwSrcAddr, DWORD dwSize, DWORD dwDstAddr)
{
	DWORD	dwReadByte = 0;
	
	if (((dwSrcAddr + dwSize) > m_dwFileSize))
	{
		return FALSE;
	}


	m_byBuffer = new BYTE[dwSize];
	if (!m_byBuffer)
	{
		return FALSE;
	}

	bool bRet = GetFileBuffer(m_byBuffer, dwSrcAddr, dwSize, dwSize, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != dwSize))
	{
		return FALSE;
	}

	DWORD dwOff = 0x00;
	RVA2FileOff(dwDstAddr, &dwOff);
	if ((dwOff + dwSize) > m_dwFileSize)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
		return FALSE;
	}
	if (!WriteBuffer(m_byBuffer, dwOff, dwSize, dwSize))
	{
		if (m_byBuffer != NULL)
		{
			delete[]m_byBuffer;
			m_byBuffer = NULL;
		}
		return FALSE;
	}
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	return TRUE;
}

/**********************************************************************************************************
*  Function Name : GetAddrToGetEntryData
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*  Date : 19 Mar 2015
*  Modified date  : 22-07-2015
* *********************************************************************************************************/
DWORD CISpyRepair::GetFileEnd()
{
	m_byBuffer = new BYTE[0x20];
	if (!m_byBuffer)
	{
		return FALSE;
	}
	DWORD	dwReadByte = 0xFFFFFFFF, dwOff = 0x00;

	RVA2FileOff(m_stPEHeader.AddressOfEntryPoint, &dwOff);
	bool bRet = GetFileBuffer(m_byBuffer, dwOff, 0x20, 0x20, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != 0x20))
	{
		return dwReadByte;
	}

	DWORD	dwAddr = 0, dwRet = 0;

	if (m_byBuffer[0] == 0xBB)
	{
		if ((*(WORD *)&m_byBuffer[5] == 0xE3FF) &&
			((*(DWORD *)&m_byBuffer[1] - m_stPEHeader.ImageBase) >= m_stSectionHeader[m_wAEPSec].VirtualAddress) &&
			((*(DWORD *)&m_byBuffer[1] - m_stPEHeader.ImageBase) <= (m_stSectionHeader[m_wAEPSec].VirtualAddress + m_stSectionHeader[m_wAEPSec].Misc.VirtualSize)))
		{
			//BB8D080201                     mov         ebx, 00102088D --?1
			//FFE3                           jmp         ebx
			//
			dwAddr = *(DWORD *)&m_byBuffer[1] - m_stPEHeader.ImageBase;
			ZeroMemory(m_byBuffer, sizeof(m_byBuffer));
			RVA2FileOff(dwAddr, &dwOff);
			dwRet = GetFileBuffer(m_byBuffer, dwOff, 0x20, 0x20, &dwReadByte);
			if (dwRet == 0 || (0x20 != dwReadByte))
			{
				return FALSE;
			}
			dwRet = 0x02;
		}

		if ((m_byBuffer[5] == 0x93) && (*(DWORD *)&m_byBuffer[6] == 0x000120E9) && (m_byBuffer[10] == 0x00))
		{
			/*BBD84C1B64                     mov         ebx, 0641B4CD8; 'd?L+'
			93                             xchg        ebx, eax
			E920010000                     jmp        .00044D737 --?1*/
			if (dwRet == 0)
			{
				dwRet = 0x01;
				dwAddr = m_stPEHeader.AddressOfEntryPoint;
			}
		}
	}
	

	if (dwRet == 0x00)
	{
		return 0xFFFFFFFF;
	}

	BYTE	byKey = 0x77 - m_byBuffer[0xB];

	m_byBuffer[0x13] = m_byBuffer[0x13] + byKey;
	m_byBuffer[0x14] = m_byBuffer[0x14] + byKey;
	m_byBuffer[0x15] = m_byBuffer[0x15] + byKey;
	m_byBuffer[0x16] = m_byBuffer[0x16] + byKey;
	dwReadByte = *(DWORD *)&m_byBuffer[0x13];
	
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	if (dwRet == 2)
	{
		m_dwArgs[1] = dwReadByte + 0x86E8;
		m_dwArgs[2] = 8;
		m_dwArgs[3] = m_stPEHeader.AddressOfEntryPoint;
		bRet = GetAddrToGetEntryData();
	}

	m_dwArgs[1] = dwReadByte + 0x8000;
	m_dwArgs[2] = 0x6E8;
	m_dwArgs[3] = dwAddr;
	m_dwArgs[0] = dwReadByte;
	
	return dwReadByte;
}
/**********************************************************************************************************
*  Function Name : Runouce
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*  Date : 19 Mar 2015
* *********************************************************************************************************/
bool CISpyRepair::Runouce()
{
	m_dwArgs[1] = m_stPEHeader.AddressOfEntryPoint;
	
	DWORD	dwOff = 0, dwReadByte = 0;
	
	dwReadByte = (DWORD)(m_dwArgs[1]) + 0x11;
	RVA2FileOff(dwReadByte, &dwOff);
	if (dwOff == 0 || dwOff > m_dwFileSize)
	{
		return FALSE;
	}
	dwReadByte = 0;
	m_byBuffer = new BYTE[4];
	if (!m_byBuffer)
	{
		return FALSE;
	}
	bool bRet = GetFileBuffer(&m_byBuffer[0], dwOff, 4, 4, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != 4))
	{
		return FALSE;
	}
	m_dwArgs[0] = *(DWORD *)&m_byBuffer[0] - m_stPEHeader.ImageBase;
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	dwReadByte = WriteAEP((DWORD)m_dwArgs[0]);
	if (dwReadByte != 2)
	{
		return FALSE;
	}

	RVA2FileOff((DWORD)m_dwArgs[1], &dwOff);
	SetFileEnd(dwOff);
	//It will give actual virus code size
	dwReadByte = m_stPEHeader.SizeOfImage - (DWORD)(m_dwArgs[1]);

	if (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress == m_stPEHeader.AddressOfEntryPoint)
	{
		return RemoveLastSections(1, FALSE);
	}
	else
	{
		dwOff = 0;
		dwOff = static_cast<DWORD>(m_dwArgs[1] - m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress);
		if (!WriteSectionCharacteristic(m_stPEHeader.NumberOfSections - 1, dwOff, 0x10))
		{
			return FALSE;
		}
	
		return CalculateImageSize();
	}
	return TRUE;
}
/**********************************************************************************************************
*  Function Name : LuderEntryReplace
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*  Date : 10 April 2015
* *********************************************************************************************************/
bool CISpyRepair::LuderEntryReplace()
{
	m_dwArgs[1] = m_stPEHeader.AddressOfEntryPoint;

	DWORD	dwOff = 0, dwReadByte = 0;

	dwOff = (DWORD)(m_dwArgs[1]) + 0x96;
	if (dwOff == 0 || dwOff > m_dwFileSize)
	{
		return FALSE;
	}

	dwReadByte = 0;
	m_byBuffer = new BYTE[4];
	if (!m_byBuffer)
	{
		return FALSE;
	}
	bool bRet = GetFileBuffer(&m_byBuffer[0], dwOff, 4, 4, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != 4))
	{
		return FALSE;
	}
	m_dwArgs[0] = *(DWORD *)&m_byBuffer[0] - m_stPEHeader.ImageBase;
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}
	m_dwArgs[1] = m_stPEHeader.AddressOfEntryPoint;

	dwReadByte = WriteAEP((DWORD)m_dwArgs[0]);
	if (dwReadByte != 2)
	{
		return FALSE;
	}
	
	bRet = FillWithZeros((DWORD)m_dwArgs[1], 0x9B);
	if (bRet == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}
/**********************************************************************************************************
*  Function Name : Gamrue
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*  Date : 10 April 2015
* *********************************************************************************************************/
bool CISpyRepair::Gamrue()
{
	TCHAR	szCurPath[MAX_PATH] = { 0 }, szValue[MAX_PATH] = { 0 };

	//CurrentPath
	DWORD dwRet = wcscpy_s(szCurPath, MAX_PATH, m_szFilePath);
	if (dwRet != 0)
	{
		return FALSE;
	}

	TCHAR *lpszFName = wcsrchr(szCurPath, '\\');
	if (lpszFName)
		*lpszFName = '\0';

	DWORD	dwAttributes = 0;
	HANDLE	hFileHandle = 0;
	
	//Actual data containing folder
	dwRet = swprintf_s(szValue, MAX_PATH, L"%s\\ ", szCurPath);
	if (dwRet != -1)
	{
		dwRet = PathFileExists(szValue);
		if (dwRet == TRUE)
		{
			dwAttributes = GetFileAttributes(szValue);

			if (INVALID_FILE_ATTRIBUTES != dwAttributes)
				SetFileAttributes(szValue, (dwAttributes & (~FILE_ATTRIBUTE_HIDDEN)));

			dwRet = SetFileAttributes(szValue, FILE_ATTRIBUTE_NORMAL);
		}
	}

	//Thumbs.db
	ZeroMemory(szValue, MAX_PATH);
	swprintf_s(szValue, MAX_PATH, L"%s\\thumbs.db", szCurPath);
	if (PathFileExists(szValue))
	{
		dwAttributes = 0;
		hFileHandle = CreateFile(szValue, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileHandle != INVALID_HANDLE_VALUE)
		{
			dwAttributes = GetFileAttributes(szValue);

			if (INVALID_FILE_ATTRIBUTES != dwAttributes)
				SetFileAttributes(szValue, (dwAttributes & (~FILE_ATTRIBUTE_HIDDEN)));

			dwRet = SetFileAttributes(szValue, FILE_ATTRIBUTE_NORMAL);
			if (dwRet != 0)
			{
				dwRet = GetFileSize(hFileHandle, NULL);
				if (dwRet > 0xC800)
				{
					CloseHandle(hFileHandle);
					DeleteFile(szValue);
				}
			}
		}
	}

	ZeroMemory(szValue, MAX_PATH);
	//Desktop.ini
	dwRet = swprintf_s(szValue, MAX_PATH, L"%s\\desktop.ini", szCurPath);
	if (dwRet != -1)
	{
		if (PathFileExists(szValue))
		{
			dwAttributes = 0;
			hFileHandle = CreateFile(szValue, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFileHandle != INVALID_HANDLE_VALUE)
			{
				dwAttributes = GetFileAttributes(szValue);

				if (INVALID_FILE_ATTRIBUTES != dwAttributes)
					SetFileAttributes(szValue, (dwAttributes & (~FILE_ATTRIBUTE_HIDDEN)));

				dwRet = SetFileAttributes(szValue, FILE_ATTRIBUTE_NORMAL);
				if (dwRet != 0)
				{
					dwRet = GetFileSize(hFileHandle, NULL);
					if (dwRet > 0xC00 && dwRet < 0x3400)
					{
						CloseHandle(hFileHandle);
						DeleteFile(szValue);
					}
				}
			}
		}
	}

	ZeroMemory(szValue, MAX_PATH);
	//rundll32
	BOOL	bRet = FALSE;
	CEnumProcess	cProc;
	dwRet = GetSystemDirectory(szValue, MAX_PATH);
	if (dwRet != 0)
	{
		dwRet = swprintf_s(szValue, MAX_PATH, L"%s\\rundll32.exe", szValue);
		if (dwRet != -1)
		{
			bRet = cProc.IsProcessRunning(szValue, TRUE, FALSE, TRUE);
			if (bRet == FALSE)
			{
				AddLogEntry(L"###Process already running", 0, 0, TRUE, SECONDLEVEL);
			}
		}
	}

	ZeroMemory(szValue, MAX_PATH);
	TCHAR	szValue1[MAX_PATH] = { 0 };
	dwRet = GetEnvironmentVariable(L"HOMEDRIVE", szValue, MAX_PATH);
	if (0 != dwRet)
	{
		swprintf_s(szValue1, MAX_PATH, L"%sMSI\\TrustedInstaller.exe", szValue);

		if (PathFileExists(szValue1))
		{
			dwAttributes = GetFileAttributes(szValue);

			if (INVALID_FILE_ATTRIBUTES != dwAttributes)
				SetFileAttributes(szValue, (dwAttributes & (~FILE_ATTRIBUTE_HIDDEN)));

			dwRet = SetFileAttributes(szValue, FILE_ATTRIBUTE_NORMAL);
			bRet = cProc.IsProcessRunning(szValue1, TRUE, FALSE, TRUE);
			if (bRet == TRUE)
				DeleteFile(szValue);
		}
	}

	ZeroMemory(szValue, MAX_PATH);

	//autorun.inf
	dwRet = swprintf_s(szValue, MAX_PATH, L"%s\\autorun.inf", szCurPath);
	if (dwRet != -1)
	{
		if (PathFileExists(szValue))
		{
			dwAttributes = 0;
			dwAttributes = GetFileAttributes(szValue);

			if (INVALID_FILE_ATTRIBUTES != dwAttributes)
				SetFileAttributes(szValue, (dwAttributes & (~FILE_ATTRIBUTE_HIDDEN)));

			dwRet = SetFileAttributes(szValue, FILE_ATTRIBUTE_NORMAL);
			if (dwRet != 0)
			{
				DeleteFile(szValue);
			}
		}
	}

	return TRUE;
}
bool CISpyRepair::GetEntryAddr()
{
	return GetEntryAddr(static_cast<DWORD>(m_dwArgs[0]));
}
/**********************************************************************************************************
*  Function Name : GetEntryAddr
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*  Date : 19 Mar 2015
* *********************************************************************************************************/
bool CISpyRepair::GetEntryAddr(DWORD dwSrcOff)
{
	DWORD	dwOff = 0, dwReadByte = 0;

	m_byBuffer = new BYTE[4];
	if (!m_byBuffer)
	{
		return FALSE;
	}

	bool bRet = GetFileBuffer(&m_byBuffer[0], dwSrcOff, 4, 4, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != 4))
	{
		return FALSE;
	}

	m_dwArgs[0] = m_stPEHeader.AddressOfEntryPoint - *(DWORD *)&m_byBuffer[0];
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	dwReadByte = WriteAEP((DWORD)m_dwArgs[0]);
	if (dwReadByte != 2)
	{

		return FALSE;
	}

	return TRUE;
}
/**********************************************************************************************************
*  Function Name : GetEntryAddr
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna(Not Done)
*  Date : 19 Mar 2015
* *********************************************************************************************************/
//bool CISpyRepair::Hidrag()
//{
//	//Encrypted MZ location
//	m_dwArgs[0] = m_dwFileSize - m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData - 0x8E00;
//	m_dwArgs[1] = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData + 0x8A00;
//	m_dwArgs[5] = m_dwArgs[0];
//	GetBuffer4Decryption(m_dwArgs[0], m_dwArgs[1]);
//	ReturnDecryptedValue(1, 7, 1);
//
//	////Actual FileSize
//	m_dwArgs[3] = m_dwFileSize - 0x8E00;
//	
//	DWORD dwSize = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData;
//	BYTE *byBuffer = new BYTE[dwSize];
//	if (!byBuffer)
//	{
//		return false;
//	}
//
//	DWORD dwReadByte = 0;
//	if (!GetFileBuffer(byBuffer, 0x8A00, dwSize, 0, &dwReadByte))
//		return false;
//	if (dwReadByte == 0x0)
//	{
//		return false;
//	}
//
//
//	m_dwArgs[5] = m_dwArgs[0];
//	m_dwArgs[0] = 0;
//	ReplaceDecryptedDataInFile();
//	CheckAndLoadValidPEFile(m_hFileHandle, TRUE);
//	
//	DWORD i = 0, dwInd = 0;
//	for (i = 0; i < m_stPEHeader.NumberOfSections; i++)
//	{
//		if (*(DWORD *)&m_stSectionHeader[i].Name == 0x7273722E)
//		{
//			dwInd = i;
//			break;
//		}
//	}
//
//	m_dwArgs[0] = m_stSectionHeader[dwInd].PointerToRawData;
//	m_dwArgs[6] = m_dwArgs[3] - m_dwArgs[5];
//	//If file dont have appended data then no need to read data
//	if (m_dwArgs[3] > (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData))
//	{
//		GetBuffer4Decryption(m_dwArgs[0], m_dwArgs[6]);
//	}
//
//	DWORD	dwWriteOffset = 0, dwBytes2Write = 0;
//	
//	dwWriteOffset = m_stSectionHeader[dwInd].PointerToRawData;
//	dwBytes2Write = dwSize;
//
//	if (dwWriteOffset == OUT_OF_FILE || dwBytes2Write == 0x0)
//		return false;
//	if (byBuffer == NULL)
//		return false;
//
//	if (!WriteBuffer(byBuffer, dwWriteOffset, dwBytes2Write, dwBytes2Write))
//	{
//		return false;
//	}
//	
//	m_dwArgs[0] = m_dwArgs[5];
//	m_dwArgs[1] = m_dwArgs[6];
//	//If file dont have appended data then no need to replace data
//	if (m_dwArgs[3] > (m_stSectionHeader[m_stPEHeader.NumberOfSections].PointerToRawData + m_stSectionHeader[m_stPEHeader.NumberOfSections].PointerToRawData))
//	{
//		ReplaceDecryptedDataInFile();
//	}
//	SetFileEnd(m_dwArgs[3]);
//	return TRUE;
//}
/**********************************************************************************************************
*  Function Name : Hidrag
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*  Date : 22-04-2015
* *********************************************************************************************************/
bool CISpyRepair::Hidrag()
{
	if ((m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData) == 0x8E00)
	{
		return RepairDelete();
	}
	//Encrypted MZ location
	m_dwArgs[0] = m_dwFileSize - m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData - 0x8E00;
	m_dwArgs[1] = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData + 0x8A00;
	m_dwArgs[5] = m_dwArgs[0];
	GetBuffer4Decryption(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]));
	ReturnDecryptedValue(1, 7, 1);

	////Actual FileSize
	m_dwArgs[3] = m_dwFileSize - 0x8E00;
	DWORD dwRsrcVA = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress;

	DWORD dwSize = static_cast<DWORD>(m_dwArgs[3]);
	BYTE *byBuffer = new BYTE[dwSize];
	if (!byBuffer)
	{
		return false;
	}

	DWORD dwReadByte = 0;
	if (!GetFileBuffer(byBuffer, 0x8A00, dwSize, 0, &dwReadByte))
		return false;
	if (dwReadByte == 0x0)
	{
		return false;
	}


	m_dwArgs[5] = m_dwArgs[0];
	m_dwArgs[0] = 0;
	ReplaceDecryptedDataInFile();
	CheckAndLoadValidPEFile(m_hFileHandle, TRUE);

	DWORD i = 0, dwInd = 0xFFFFFFFF;
	for (i = 0; i < m_stPEHeader.NumberOfSections; i++)
	{
		if (*(DWORD *)&m_stSectionHeader[i].Name == 0x7273722E)
		{
			//.rsrc section number
			dwInd = i;
			break;
		}
	}

	if (dwInd == 0xFFFFFFFF)
	{
		//If no resource found
		return FALSE;
	}

	DWORD dwKey = m_stSectionHeader[dwInd].VirtualAddress - dwRsrcVA;
	if (!GetRsrc(&byBuffer[0], m_stSectionHeader[dwInd].SizeOfRawData, 0, dwKey))
	{
		if (byBuffer != NULL)
		{
			delete[]byBuffer;
			byBuffer = NULL;
		}
		return FALSE;
	}
	if (!WriteBuffer(&byBuffer[0], m_stSectionHeader[dwInd].PointerToRawData, m_stSectionHeader[dwInd].SizeOfRawData, m_stSectionHeader[dwInd].SizeOfRawData))
	{
		if (byBuffer != NULL)
		{
			delete[]byBuffer;
			byBuffer = NULL;
		}
		return FALSE;
	}

	i = 0;
	if (m_stSectionHeader[dwInd].PointerToRawData > 0x8A00)
	{
		i = m_stSectionHeader[dwInd].PointerToRawData - 0x8A00;
	}
	else
	{
		i = m_stSectionHeader[dwInd].SizeOfRawData;
	}
	if ((i >= dwSize) || (i <= 0))
	{
		if (byBuffer != NULL)
		{
			delete[]byBuffer;
			byBuffer = NULL;
		}
		return FALSE;
	}

	dwSize = dwSize - i;
	DWORD dwRemainByte = static_cast<DWORD>(m_dwArgs[5] - m_stSectionHeader[dwInd].PointerToRawData);
	//case where there is no appended data & following section
	if (dwRemainByte > 0)
	{
		//.rsrc is last section & appended data exist
		if (dwInd == (m_stPEHeader.NumberOfSections - 1))
		{
			if (!WriteBuffer(&byBuffer[i], static_cast<DWORD>(m_dwArgs[5]), dwRemainByte, dwRemainByte))
			{
				if (byBuffer != NULL)
				{
					delete[]byBuffer;
					byBuffer = NULL;
				}
				return FALSE;
			}
		}
		//If there is next section after resource
		else if ((m_dwArgs[5] > m_stSectionHeader[dwInd + 1].PointerToRawData) && (m_dwArgs[5] < (m_stSectionHeader[dwInd + 1].PointerToRawData + m_stSectionHeader[dwInd + 1].SizeOfRawData)))
		{
			if (!WriteBuffer(&byBuffer[i], static_cast<DWORD>(m_dwArgs[5]), dwRemainByte, dwRemainByte))
			{
				if (byBuffer != NULL)
				{
					delete[]byBuffer;
					byBuffer = NULL;
				}
				return FALSE;
			}
		}
		else if (!WriteBuffer(&byBuffer[i], m_stSectionHeader[dwInd + 1].PointerToRawData, dwRemainByte, dwRemainByte))
		{
			if (byBuffer != NULL)
			{
				delete[]byBuffer;
				byBuffer = NULL;
			}
			return FALSE;
		}
	}

	if (byBuffer != NULL)
	{
		delete[]byBuffer;
		byBuffer = NULL;
	}

	SetFileEnd(static_cast<DWORD>(m_dwArgs[3]));
	return TRUE;
}
/***************************************************************************************************
*  Function Name  : GetRsrc()
*  Description    : Traverses resource data directory recursively from given address
*  Author Name    : Jyotsna
*  Date			  : 27-Apr-2015
*  SR No		  : WRDWIZHEURISTICSCANNER_0027
****************************************************************************************************/
bool CISpyRepair::GetRsrc(BYTE *pbyBuffer, DWORD dwSize, DWORD dwAddr, DWORD dwKey)
{
	IMAGE_RESOURCE_DIRECTORY	Image_Rsrc_Dir;
	if ( (dwAddr + sizeof(Image_Rsrc_Dir) ) >= dwSize)
	{
		return FALSE;
	}

	memcpy(&Image_Rsrc_Dir, &pbyBuffer[dwAddr], sizeof(Image_Rsrc_Dir));

	DWORD	i = 0, dwInd = 0, dwAddr1 = 0;
	IMAGE_RESOURCE_DATA_ENTRY	Image_Rsrc_Data_Entry;
	IMAGE_RESOURCE_DIRECTORY_ENTRY	Image_Rsrc_Dir_Entry;
	DWORD dwTotEntry = Image_Rsrc_Dir.NumberOfIdEntries + Image_Rsrc_Dir.NumberOfNamedEntries;

	dwAddr1 = dwAddr + 0x10;
	for (i = 0; (i < dwTotEntry) && (dwAddr1 < dwSize); i++)
	{
		if ((dwAddr1 + (i * 8)) >= dwSize)
		{
			return FALSE;
		}

		memcpy(&Image_Rsrc_Dir_Entry, &pbyBuffer[dwAddr1 + (i * 8)], sizeof(Image_Rsrc_Dir_Entry));
		if ((Image_Rsrc_Dir_Entry.OffsetToData & 0xF0000000) != 0)
		{
			//Again Directory
			if( !GetRsrc(pbyBuffer, dwSize, (Image_Rsrc_Dir_Entry.OffsetToData & 0x0FFFFFFF), dwKey) )
				return FALSE;
		}
		else
		{
			if ( (Image_Rsrc_Dir_Entry.OffsetToData + sizeof(Image_Rsrc_Data_Entry) ) >= dwSize)
			{
				return FALSE;
			}

			memcpy(&Image_Rsrc_Data_Entry, &pbyBuffer[Image_Rsrc_Dir_Entry.OffsetToData], sizeof(Image_Rsrc_Data_Entry));

			Image_Rsrc_Data_Entry.OffsetToData = Image_Rsrc_Data_Entry.OffsetToData + dwKey;
			if (dwTotEntry == 1)
			{
				dwAddr1 = dwAddr1 + 8;
			}

			if ((Image_Rsrc_Dir_Entry.OffsetToData + sizeof(DWORD)) >= dwSize)
			{
				return FALSE;
			}

			*(DWORD *)&pbyBuffer[Image_Rsrc_Dir_Entry.OffsetToData] = Image_Rsrc_Data_Entry.OffsetToData;
		}
	}

	return TRUE;
}
/***************************************************************************************************
*  Function Name  : QVod() - Signture WRDWIZAVR-1008
*  Description    : Checks for QVod Variant
*  Author Name    : Jyotsna
*  Date			  : 27-Apr-2015
*  SR No		  : WRDWIZHEURISTICSCANNER_0027
****************************************************************************************************/
bool CISpyRepair::QVod()
{
	DWORD	dwReadBytes = 0, dwInd = 0;
	BYTE	byBuf[0x300] = { 0 };
	dwInd = m_stPEHeader.NumberOfSections - 1;
	DWORD dwRet = GetFileBuffer(byBuf, (m_stSectionHeader[dwInd].PointerToRawData), sizeof(byBuf), sizeof(byBuf), &dwReadBytes);
	if (dwRet == 0 || (sizeof(byBuf) != dwReadBytes))
	{
		return FALSE;
	}

	dwRet = 0;
	DWORD	i = 0, dwAddr = 0, dwTemp = 0;
	dwReadBytes = dwReadBytes - 0x10;
	for (i = 0; (i < dwReadBytes) && (dwRet < 4) && (dwAddr == 0); i++)
	{
		if ((byBuf[i] == 0xC9) && (*(DWORD *)&byBuf[i + 1] == 0xE904C483) && (*(WORD *)&byBuf[i + 9] == 0x5A4D) && (byBuf[i + 11] == 0x90) && (byBuf[i + 8] == 0xFF))
		{
			/*
			C9                                      leave
			83 C4 04                                add     esp, 4
			E9 24 AD FF FF                          jmp     loc_448001
			4D 5A 90                                db 4Dh, 5Ah, 90h*/

			dwAddr = *(DWORD *)&byBuf[i + 5] + m_stSectionHeader[dwInd].VirtualAddress + 9 + i;
			dwRet = 4;
			break;
		}
		else
		if ((dwRet != 1) && (byBuf[i] == 0xC7) && ((*(DWORD *)&byBuf[i + 2] == 0xE904C483) || (*(DWORD *)&byBuf[i + 2] == 0xE904C483)))
		{
			//C7 00 83 C4 04 E9                       mov     dword ptr ds:(locret_4062B2 - 4062B2h)[eax], 0E904C483h
			dwRet = 1;
		}
		else if ((dwRet == 1) && (byBuf[i] == 0xC7) && (byBuf[i + 6] == 0xFF))
		{
			//C7 40 04 A4 BF FF FF                    mov     ds:(dword_4062B6 - 4062B2h)[eax], 0FFFFBFA4h
			dwRet = 2;
			dwTemp = *(DWORD *)&byBuf[i + 3];
		}
		else if ((dwRet == 2) && (*(WORD *)&byBuf[i] == 0xC3C9))
		{
			dwRet = 3;			
		}
		else if ((dwRet == 3) && (*(WORD *)&byBuf[i] == 0x5A4D) && (byBuf[i + 2] == 0x90))
		{
			dwRet = 4;
			dwAddr = dwTemp + m_stSectionHeader[dwInd].VirtualAddress + i;
			break;
		}
	}

	if ((dwAddr == 0) || (dwRet != 4))
	{
		return FALSE;
	}

	WriteAEP(dwAddr);
	return TRUE;
}
/***************************************************************************************************
*  Function Name  : Resur
*  Description    : It adds duplicate sections at the end 
*  Author Name    : Jyotsna
*  Date			  : 04-May-2015
*  SR No		  : WRDWIZHEURISTICSCANNER_0027
****************************************************************************************************/
bool CISpyRepair::Resur()
{
	DWORD dwSec = 0, dwSecNo = m_stPEHeader.NumberOfSections - 4;
	DWORD dwImageSize = m_dwFileSize;
	//Read,Write,Sizeof data
	m_dwArgs[0] = m_stSectionHeader[dwSecNo + 2].PointerToRawData + 0x2740;
	m_dwArgs[1] = m_stPEHeader.e_lfanew;
	m_dwArgs[2] = 0xE0;
	if (!ReplaceOriginalData())
	{
		return FALSE;
	}


	m_byBuffer = new BYTE[4];
	if (!m_byBuffer)
		return false;

	DWORD dwRet = GetFileBuffer(m_byBuffer, (m_stSectionHeader[dwSecNo + 2].PointerToRawData + 0x2570), 4, 4, &m_dwBuffSize);
	if (dwRet == 0 || (4 != m_dwBuffSize))
	{
		return FALSE;
	}

	//AEP repair
	dwSec = *(DWORD *)&m_byBuffer[0];
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	dwSec = WriteAEP(dwSec);
	if (dwSec != 2)
	{
		return FALSE;
	}

	dwSec = 0;

	if (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData > 0x800)
	{
		m_dwArgs[0] = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData;
		m_dwArgs[1] = m_stSectionHeader[dwSecNo].PointerToRawData;
		m_dwArgs[2] = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData;
		if (!ReplaceOriginalData())
		{
			return FALSE;
		}
		if (!FillWithZeros(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[2])))
		{
			return FALSE;
		}
		m_stSectionHeader[dwSecNo + 3].PointerToRawData = m_stSectionHeader[dwSecNo].PointerToRawData;
		m_stSectionHeader[dwSecNo + 3].VirtualAddress = m_stSectionHeader[dwSecNo].VirtualAddress;

		m_dwArgs[0] = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + ((dwSecNo + 3) * IMAGE_SIZEOF_SECTION_HEADER);
		m_dwArgs[1] = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + ((dwSecNo) * IMAGE_SIZEOF_SECTION_HEADER);
		m_dwArgs[2] = IMAGE_SIZEOF_SECTION_HEADER;

		if (!WriteBuffer(&m_stSectionHeader[dwSecNo + 3], static_cast<DWORD>(m_dwArgs[1]), static_cast<DWORD>(m_dwArgs[2]), static_cast<DWORD>(m_dwArgs[2]), &dwRet))
		{
			return FALSE;
		}
		
		if (dwRet != m_dwArgs[2])
		{
			return FALSE;
		}

		dwSecNo++;
		dwSec = 3;
		dwRet = m_stSectionHeader[dwSecNo + 2].PointerToRawData + m_stSectionHeader[dwSecNo + 2].SizeOfRawData;
		if (!SetFileEnd(dwRet))
		{
			return FALSE;
		}
	}
	else
	{
		if (!SetFileEnd(m_stSectionHeader[dwSecNo].PointerToRawData))
		{
			return FALSE;
		}
		dwSec = 4;
	}
	DWORD dwSecHeaderOff = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + (dwSecNo * IMAGE_SIZEOF_SECTION_HEADER);
	if (!FillWithZeros(dwSecHeaderOff, dwSec * IMAGE_SIZEOF_SECTION_HEADER))
	{
		return false;
	}

	return TRUE;
}
/***************************************************************************************************
*  Function Name  : Crytex - It replaces flash data
*  Description    : It changes entrypoint & actual entrypoint is present in encrypted form
*  Author Name    : Jyotsna
*  Date			  : 06-May-2015
*  SR No		  : WRDWIZHEURISTICSCANNER_0027
****************************************************************************************************/
bool CISpyRepair::Crytex()
{
	if (m_stPEHeader.Win32VersionValue != 0x0000F846)
	{
		return FALSE;
	}
	DWORD dwRet = 0;
	if (!RVA2FileOff(m_stPEHeader.AddressOfEntryPoint, &dwRet))
	{
		return FALSE;
	}

	if (!GetBuffer4Decryption((dwRet + 0x56), (0x130 * 4)))
	{
		return FALSE;
	}
	DWORD i = 0, dwEdx = 0, dwEax = 0;
	dwEdx = *(DWORD *)&m_byBuffer[0];
	for (i = 4; i < (m_dwBuffSize - 4); i = i + 4)
	{
		
		dwEax = dwEdx * 0x10DCD;
		dwEax = dwEax + 0x116C5;
		dwEdx = dwEax % 0x7FFFFFFF;
		dwEax = *(DWORD *)&m_byBuffer[i];
		*(DWORD *)&m_byBuffer[i] = dwEax ^ dwEdx;
	}

	if ((*(DWORD *)&m_byBuffer[0x37C] < m_stPEHeader.ImageBase))
	{
		return FALSE;
	}

	dwEax = WriteAEP(*(DWORD *)&m_byBuffer[0x37C] - m_stPEHeader.ImageBase);
	if (dwEax != 2)
	{
		return FALSE;
	}

	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	if (!RepairOptionalHeader(0x13, 0, 0, FALSE))
	{
		return FALSE;
	}

	if (!TruncateFile(dwRet, FALSE))
	{
		return FALSE;
	}
	return TRUE;
}
bool CISpyRepair::GetEntryAddr_Image()
{
	return GetEntryAddr_Image(static_cast<DWORD>(m_dwArgs[0]));
}
/**********************************************************************************************************
*  Function Name : GetEntryAddr_Image
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*  Date : 08 May 2015
* *********************************************************************************************************/
bool CISpyRepair::GetEntryAddr_Image(DWORD dwSrcOff)
{
	DWORD	dwOff = 0, dwReadByte = 0;

	m_byBuffer = new BYTE[4];
	if (!m_byBuffer)
	{
		return FALSE;
	}

	bool bRet = GetFileBuffer(&m_byBuffer[0], dwSrcOff, 4, 4, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != 4))
	{
		return FALSE;
	}

	m_dwArgs[0] = *(DWORD *)&m_byBuffer[0] - m_stPEHeader.ImageBase;
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	dwReadByte = WriteAEP((DWORD)m_dwArgs[0]);
	if (dwReadByte != 2)
	{
		return FALSE;
	}

	RVA2FileOff((DWORD)m_dwArgs[1], &dwOff);

	return TRUE;
}


bool CISpyRepair::XorAla_Entry()
{
	return XorAla_Entry(static_cast<DWORD>(m_dwArgs[0]));
}
/**********************************************************************************************************
*  Function Name : XorAla_Entry
*  Description : Fetch the addres after decryption
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*  Date : 11 May 2015
* *********************************************************************************************************/
bool CISpyRepair::XorAla_Entry(DWORD dwOff)
{
	m_byBuffer = new BYTE[8];
	if (!m_byBuffer)
	{
		return FALSE;
	}

	DWORD dwSrcOff = m_stPEHeader.AddressOfEntryPoint + dwOff;
	if (!RVA2FileOff(m_stPEHeader.AddressOfEntryPoint + dwOff, &dwSrcOff))
	{
		return FALSE;
	}

	DWORD dwReadByte = 0;
	bool bRet = GetFileBuffer(&m_byBuffer[0], dwSrcOff, 4, 4, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != 4))
	{
		return FALSE;
	}

	dwSrcOff = *(DWORD *)&m_byBuffer[0] + m_stPEHeader.AddressOfEntryPoint;
	if (!RVA2FileOff(dwSrcOff, &dwSrcOff))
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
		return FALSE;
	}

	if (!ZeroMemory(&m_byBuffer[0], 8))
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
		return FALSE;
	}

	bRet = GetFileBuffer(&m_byBuffer[0], dwSrcOff, 8, 8, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != 8))
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
		return FALSE;
	}

	dwSrcOff = *(DWORD *)&m_byBuffer[4] + m_stPEHeader.AddressOfEntryPoint;
	
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	dwReadByte = WriteAEP(dwSrcOff);
	if (dwReadByte != 2)
	{		
		return FALSE;
	}

	return TRUE;
}
DWORD CISpyRepair::ReplaceDataSize()
{
	return ReplaceDataSize(static_cast<DWORD>(m_dwArgs[0]));
}
/**********************************************************************************************************
*  Function Name : ReplaceDataSize
*  Description : Fetch actual data size to replace
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*	Modified By	:	Jyotsna
*  Date : 13 May 2015
* *********************************************************************************************************/
DWORD CISpyRepair::ReplaceDataSize(DWORD dwAddVal)
{
	if (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].Characteristics < 0xE0000000)
	{
		return FALSE;
	}

	m_dwBuffSize = 0x800;
	m_byBuffer = new BYTE[m_dwBuffSize];
	if (!m_byBuffer)
	{
		return FALSE;
	}

	DWORD dwSrcOff = 0;
	RVA2FileOff(m_stPEHeader.AddressOfEntryPoint, &dwSrcOff);
	
	DWORD dwReadByte = 0;
	bool bRet = GetFileBuffer(&m_byBuffer[0], dwSrcOff, m_dwBuffSize, m_dwBuffSize, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != m_dwBuffSize))
	{
		return FALSE;
	}

	DWORD	i = 0, dwAddr = 0;
	bool bFlag = 0;

	bRet = FALSE;
	dwReadByte = dwReadByte - 0x40;
	for (i = 0; (i < dwReadByte); i++)
	{
		if ((m_byBuffer[i + 3] == 0xC9) && (m_byBuffer[i + 4] == 0xE9) && (m_byBuffer[i + 8] < 0x80))
		{
			/*.text : 0101EFE6 C9                                      leave
			.text : 0101EFE7 E9 C7 8C 04 00                          jmp     near ptr byte_1067CB3*/
			dwAddr = m_stPEHeader.AddressOfEntryPoint + i + 9 + *(DWORD *)&m_byBuffer[i + 5];
			if ((dwAddr > m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress) &&
				(dwAddr < (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].Misc.VirtualSize)))
			{
				bRet = TRUE;
				dwAddr = i + 9 + dwAddVal;
				break;
			}
		}
		else if((*(WORD *)&m_byBuffer[i] == 0x68C9) && (m_byBuffer[i + 6] == 0x90) && (m_byBuffer[i + 7] == 0xC3) &&
			((*(DWORD *)&m_byBuffer[i + 2] - m_stPEHeader.ImageBase) >= m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress) &&
			((*(DWORD *)&m_byBuffer[i + 2] - m_stPEHeader.ImageBase) < (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].Misc.VirtualSize)))
		{
			//C9                                      leave
			//68 92 F2 DE 4A                          push    offset word_4ADEF292	--Control passes to last section
			//90                                      nop
			//C3                                      retn
			bRet = TRUE;
			dwAddr = i + 8 + dwAddVal;
			break;
		}
		else if ((m_byBuffer[i] >= 0xB8) && (m_byBuffer[i] <= 0xBF) && (m_byBuffer[i + 10] >= 0x50) && (m_byBuffer[i + 10] <= 0x57) && (m_byBuffer[i + 11] == 0xC3) &&
			(m_byBuffer[i] - 0xB8) == (m_byBuffer[i + 10] - 0x50))
		{
			/*C9                                      leave
			B8 C9 0D 0A 01                          mov     eax, offset byte_10A0DC9
			05 A0 50 00 00                          add     eax, 50A0h
			50                                      push    eax
			C3                                      retn*/
			dwAddr = *(DWORD *)&m_byBuffer[i + 2] - m_stPEHeader.ImageBase;
			
			if((((*(DWORD *)&m_byBuffer[i + 2] - m_stPEHeader.ImageBase) >= m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress) &&
				((*(DWORD *)&m_byBuffer[i + 2] - m_stPEHeader.ImageBase) < (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].Misc.VirtualSize))) || 
				((*(DWORD *)&m_byBuffer[i + 2] - m_stPEHeader.ImageBase) > m_dwFileSize))
			{
				dwAddr = i + 12 + dwAddVal;
				bRet = TRUE;
				bFlag = 1;
				break;
			}
		}
		else if (((((m_byBuffer[i + 5] == 0x01) || (m_byBuffer[i + 5] == 0x03)) && (m_byBuffer[i + 7] == 0x81)) || (((m_byBuffer[i + 11] == 0x01) || (m_byBuffer[i + 11] == 0x03)) && (m_byBuffer[i + 5] == 0x81))) &&
			(m_byBuffer[i + 13] >= 0x50) && (m_byBuffer[i + 13] <= 0x57))
		{
			/*B8 85 68 00 00                          mov     eax, 6885h
			01 C6                                   add     esi, eax
			81 C6 72 9C 51 00                       add     esi, offset word_519C72
			56                                      push    esi
			8B F0                                   mov     esi, eax
			2B C0                                   sub     eax, eax
			C3                                      retn*/
			if (m_byBuffer[i + 5] == 0x81)
			{
				if (((*(DWORD *)&m_byBuffer[i + 7] - m_stPEHeader.ImageBase) >= m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress) &&
					((*(DWORD *)&m_byBuffer[i + 7] - m_stPEHeader.ImageBase) < (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].Misc.VirtualSize)))
				{
					dwAddr = i + 14;
					bRet = TRUE;					
					bFlag = 1;
					break;
				}
			}
			else
			{
				if (((*(DWORD *)&m_byBuffer[i + 9] - m_stPEHeader.ImageBase) >= m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress) &&
					((*(DWORD *)&m_byBuffer[i + 9] - m_stPEHeader.ImageBase) < (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].Misc.VirtualSize)))
				{
					dwAddr = i + 18;
					bRet = TRUE;
					bFlag = 1;
					break;
				}
			}
		}
	}
	
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	if (bRet == FALSE)
	{
		return FALSE;
	}
	bRet = FALSE;
	if (bFlag == 1)
	{
		m_dwBuffSize = 0x1000;
		m_byBuffer = new BYTE[m_dwBuffSize];
		if (!m_byBuffer)
		{
			return FALSE;
		}
		DWORD dwAddr1 = (m_dwFileSize - m_dwBuffSize);
		dwReadByte = 0;
		bRet = GetFileBuffer(&m_byBuffer[0], dwAddr1, m_dwBuffSize, m_dwBuffSize, &dwReadByte);
		if ((bRet == FALSE) || (dwReadByte != m_dwBuffSize))
		{
			return FALSE;
		}

		bRet = FALSE;
		bFlag = 0;
		for (i = dwReadByte - 4; i > 4; i--)
		{
			if (m_byBuffer[i] != 0)
			{
				bFlag = 1;
				break;
			}
		}

		if (bFlag != 1)
		{
			return FALSE;
		}

		if ((*(DWORD *)&m_byBuffer[i - 4]) > (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData))
		{
			return FALSE;
		}

		m_dwArgs[0] = *(DWORD *)&m_byBuffer[i - 4] + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData;
		if (m_dwArgs[0] > m_dwFileSize)
		{
			return FALSE;
		}
		bRet = 1;
	}
	else
	{
		m_dwArgs[0] = m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].PointerToRawData;
	}
	m_dwArgs[1] = dwSrcOff;
	m_dwArgs[2] = dwAddr;
	DWORD dwTemp = ReplaceOriginalData();
	if (dwTemp != 2)
	{
		return 2;
	}

	if (bRet == 1)
	{
		TruncateFile(static_cast<DWORD>(m_dwArgs[0]));
	}
	return TRUE;
}

bool CISpyRepair::CalcCallAddr()
{
	return CalcCallAddr(static_cast<DWORD>(m_dwArgs[0]));
}
/**********************************************************************************************************
*  Function Name : CalcCallAddr
*  Description : Fetching actual Entrypiont from jmp instruction
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*	Modified By	:	Jyotsna
*  Date : 15 May 2015
* *********************************************************************************************************/
bool CISpyRepair::CalcCallAddr(DWORD dwAddr)
{
	DWORD dwAddr1 = 0;
	m_dwBuffSize = 0x20;
	m_byBuffer = new BYTE[m_dwBuffSize];
	if (!m_byBuffer)
	{
		return FALSE;
	}

	RVA2FileOff(dwAddr, &dwAddr1);
	DWORD dwReadByte = 0;
	bool bRet = GetFileBuffer(&m_byBuffer[0], dwAddr1, m_dwBuffSize, m_dwBuffSize, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != m_dwBuffSize))
	{
		return FALSE;
	}
	DWORD	i = 0;
	bRet = FALSE;
	dwReadByte = dwReadByte - 10;
	for (i = 0; i < dwReadByte; i++)
	{
		if (m_byBuffer[i] == 0xE9)
		{
			//jmp
			dwReadByte = *(DWORD *)&m_byBuffer[i + 1] + 5 + dwAddr + i;
			if ((dwReadByte < m_stSectionHeader[m_stPEHeader.NumberOfSections - 2].VirtualAddress) ||
				(dwReadByte > (m_stSectionHeader[m_stPEHeader.NumberOfSections - 2].VirtualAddress + m_stSectionHeader[m_stPEHeader.NumberOfSections - 2].Misc.VirtualSize)))
			{
				bRet = TRUE;
				break;
			}
		}		
	}
	
	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	if (bRet == FALSE)
	{
		return FALSE;
	}

	if (!WriteAEP(dwReadByte))
	{
		return FALSE;
	}
	return TRUE;
}
/**********************************************************************************************************
*  Function Name : VirutEntryPt
*  Description : Fetching actual Entrypiont from call at entrypoint (virut.be, Virut.A,Virut.AC,Virut.BW,Virut.AV,virut.Af,Ao,U,BU etc.)
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*	Modified By	:	Jyotsna
*  Date : 19 May 2015,(25-May)
* *********************************************************************************************************/
bool CISpyRepair::VirutEntryPt()
{
	DWORD dwAddr1 = 0, dwAddr = 0;
	m_dwBuffSize = 0x200;
	m_byBuffer = new BYTE[m_dwBuffSize];
	if (!m_byBuffer)
	{
		return FALSE;
	}

	//Entry point lies in last section
	dwAddr = m_stPEHeader.AddressOfEntryPoint;
	if ((dwAddr < m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress) ||
		(dwAddr >(m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].Misc.VirtualSize)))
	{
		return FALSE;
	}

	RVA2FileOff(dwAddr, &dwAddr1);
	DWORD dwReadByte = 0;
	bool bRet = GetFileBuffer(&m_byBuffer[0], dwAddr1, m_dwBuffSize, m_dwBuffSize, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte != m_dwBuffSize))
	{
		return FALSE;
	}

	if ((m_byBuffer[0] != 0xE8) || (m_byBuffer[4] >= 0x80) || ((*(DWORD *)&m_byBuffer[1] + 0x30) > m_dwBuffSize))
	{
		if ((m_byBuffer[1] != 0xE8) || (m_byBuffer[5] >= 0x80) || ((*(DWORD *)&m_byBuffer[2] + 0x30) > m_dwBuffSize))
		{
			return FALSE;
		}
	}
	
	if (m_byBuffer[1] == 0xE8)
	{
		if (m_byBuffer[0] == 0x90 || ((m_byBuffer[0] >= 0xF5) && (m_byBuffer[0] <= 0xF9)) || (m_byBuffer[0] == 0xFC))
		{
			dwAddr1 = *(DWORD *)&m_byBuffer[2] + 1;
			dwAddr = dwAddr + 1;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		dwAddr1 = *(DWORD *)&m_byBuffer[1];
	}

	if ((dwAddr1 + 0x20) >= dwReadByte)
	{
		return FALSE;
	}

	DWORD	dwTemp = 0xFFFFFFFF, i = 0;
	dwReadByte = dwReadByte - 20;
	bRet = false;
	if ((*(DWORD *)&m_byBuffer[dwAddr1 + 5] == 0x246C8B55) && (*(DWORD *)&m_byBuffer[dwAddr1 + 9] == 0x246C8104) && (m_byBuffer[dwAddr1 + 13] == 0x04))
	{//Virut.A,U
		//55                                      push    ebp
		//8B 6C 24 04                             mov     ebp, [esp + 4]
		//81 6C 24 04 AF 1C 00 00                 sub     dword ptr[esp + 4], 1CAFh
		dwTemp = dwAddr + 5 - *(DWORD *)&m_byBuffer[dwAddr1 + 14];
	}
	else if ((m_byBuffer[dwAddr1 + 5] == 0x85) && ((m_byBuffer[dwAddr1 + 6] & 0x07) == ((m_byBuffer[dwAddr1 + 6] & 0x38) >> 12)))
	{
		//85 C0                                   test    eax, eax
		bRet = false;
		for (i = dwAddr1 + 5; i < dwReadByte; i++)
		{
			if ((*(DWORD *)&m_byBuffer[i] == 0x246C8B55) && (m_byBuffer[i + 4] == 0x04) && (*(DWORD *)&m_byBuffer[i + 5] == 0x04246C81))
			{//Virut.Ac
				//55                                      push    ebp
				//8B 6C 24 04                             mov     ebp, [esp + 4]		
				//81 6C 24 04 AF 1C 00 00                 sub     dword ptr[esp + 4], 1CAFh
				dwTemp = dwAddr + 5 - *(DWORD *)&m_byBuffer[i + 9];
				bRet = true;
				break;
			}
			else if ((m_byBuffer[i] == 0x55) && (*(DWORD *)&m_byBuffer[i + 7] == 0x04246C81) &&
				(((*(DWORD *)&m_byBuffer[i + 3] == 0x04246C8B) && (m_byBuffer[i + 1] == 0x03)) ||
				((*(DWORD *)&m_byBuffer[i + 1] == 0x04246C8B) && (m_byBuffer[i + 5] == 0x03))))
			{
				/*Virut.Af
				55                                      push    ebp
				03 C3                                   add     eax, ebx
				8B 6C 24 04                             mov     ebp, [esp + 4]
				81 6C 24 04 69 C6 00 00                 sub     dword ptr[esp + 4], 0C669h*/
				//or
				/*Virut.Ao
				55                                      push    ebp
				8B 6C 24 04                             mov     ebp, [esp + 4]
				03 C1                                   add     eax, ecx
				81 6C 24 04 C7 22 00 00                 sub     dword ptr[esp + 4], 22C7h*/

				dwTemp = dwAddr + 5 - *(DWORD *)&m_byBuffer[i + 11];
				bRet = true;
				break;
			}
			else if ((*(DWORD *)&m_byBuffer[i] == 0x04246C8B) && (*(DWORD *)&m_byBuffer[i + 4] == 0x04246C81))
			{
				//8B 6C 24 04                             mov     ebp, [esp+4]
				//81 6C 24 04 4F 25 00 00                 sub     dword ptr[esp + 4], 254Fh
				dwTemp = dwAddr + 5 - *(DWORD *)&m_byBuffer[i + 8];
				bRet = true;
				break;
			}
			else if ((*(DWORD *)&m_byBuffer[i] == 0x04246C89) && (*(DWORD *)&m_byBuffer[i + 4] == 0x04246C81))
			{
				//rsrc:01013E63 89 6C 24 04                             mov[esp + 4], ebp
				//.rsrc : 01013E67 81 6C 24 04 E0 70 00 00                 sub     dword ptr[esp + 4], 70E0h
				dwTemp = dwAddr + 5 - *(DWORD *)&m_byBuffer[i + 8];
				bRet = true;
				break;
			}
			else if ((*(DWORD *)&m_byBuffer[i] == 0x0424548B) && (*(WORD *)&m_byBuffer[i + 6] == 0xEA87) && (*(DWORD *)&m_byBuffer[i + 8] == 0x04246C81))
			{//virut.be
				 /*8B 54 24 04             mov     edx, [esp + 4]
				 2B C1                   sub     eax, ecx
				 87 EA                   xchg    ebp, edx
				 81 6C 24 04 11 BB 00 00 sub     dword ptr[esp + 4], 0BB11h*/
				dwTemp = dwAddr + 5 - *(DWORD *)&m_byBuffer[i + 12];
				bRet = true;
				break;
			}
			else if ((*(DWORD *)&m_byBuffer[i] == 0x04246C87) && (*(DWORD *)&m_byBuffer[i + 6] == 0x04246C89) && (*(DWORD *)&m_byBuffer[i + 10] == 0x04246C81))
			{//.Bu
				/*87 6C 24 04             xchg    ebp, [esp + 4]
				F7 D1                   not     ecx
				89 6C 24 04             mov[esp + 4], ebp
				81 6C 24 04 5E 38 00 00 sub     dword ptr[esp + 4], 385Eh*/

				dwTemp = dwAddr + 5 - *(DWORD *)&m_byBuffer[i + 14];
				bRet = true;
				break;
			}
		}
		if (bRet == false)
		{
			return false;
		}
	}

	if (m_byBuffer != NULL)
	{
		delete[]m_byBuffer;
		m_byBuffer = NULL;
	}

	if (dwTemp == 0xFFFFFFFF)
	{
		return FALSE;
	}

	dwAddr1 = 0;
	RVA2FileOff(dwTemp, &dwAddr1);
	if ((dwAddr1 > m_dwFileSize) || (dwAddr1 == 0))
	{
		return FALSE;
	}

	if (!WriteAEP(dwTemp))
	{
		return FALSE;
	}

	return TRUE;
}
/**********************************************************************************************************
*  Function Name : VirutCE
*  Description : Fetching actual Entrypiont from call at entrypoint 
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Jyotsna
*	Modified By	:	Jyotsna
*  Date : 12 June 2015
* *********************************************************************************************************/
bool CISpyRepair::VirutCE()
{
	DWORD dwAddr1 = 0, dwAddr = 0;
	BYTE byBuf[0x200] = {0};

	//Entry point lies in last section
	dwAddr = m_stPEHeader.AddressOfEntryPoint;
	RVA2FileOff(dwAddr, &dwAddr1);
	DWORD dwReadByte = 0;
	bool bRet = GetFileBuffer(&byBuf[0], dwAddr1, sizeof(byBuf), 0x8, &dwReadByte);
	if ((bRet == FALSE) || (dwReadByte < 0x08))
	{
		return FALSE;
	}

	bool bFlag = FALSE, bFlag1 = FALSE;
	DWORD i = 0, dwRet = 0, dwAddr2 = 0;
	
	dwRet = 0;
	bRet = FALSE;
	dwAddr1 = 0;
	dwReadByte = dwReadByte - 0x8;
	for (i = 0; (i < dwReadByte) && (dwRet < 4); i++)
	{
		if ((dwRet == 0) && (*(WORD *)&byBuf[i] == 0xEC83) && (byBuf[i + 2] == 0xDC))
		{
			//83 EC DC                sub     esp, 0FFFFFFDCh
			dwRet = 1;
		}
		else if ((dwRet == 1) && (byBuf[i] == 0xE8) && ((*(WORD *)&byBuf[i + 3] == 0x0000) || (*(WORD *)&byBuf[i + 3] == 0xFFFF)))
		{

			//E8 60 01 00 00          call    sub_417A55
			dwAddr1 = dwAddr + i + 5;
			dwAddr2 = dwAddr + i + 5 + *(DWORD *)&byBuf[i + 1];
			dwRet = 2;
			if (((dwAddr + dwReadByte) < (dwAddr2 + 0x30)) || (dwAddr > dwAddr2))
			{
				bRet = TRUE;
				break;
			}
		}
		else if ((dwRet == 2) && (*(DWORD *)&byBuf[i] == 0x24244481))
		{
			//81 44 24 24 8B 7C FF FF add     dword ptr[esp + 24h], 0FFFF7C8Bh
			bFlag1 = TRUE;
			dwAddr1 = dwAddr1 + *(DWORD *)&byBuf[i + 4];
			dwRet = 3;
		}
		else if ((dwRet == 3) && (byBuf[i] == 0xE9))
		{
			//E9 90 69 00 00                          jmp     loc_673D8A
			dwAddr2 = dwAddr + i + 5 + *(DWORD *)&byBuf[i + 1];
			bRet = TRUE;
			dwRet = 3;
			break;
		}
		else if ((dwRet == 3) && (*(WORD *)&byBuf[i] == 0xE5FF))
		{
			//FF E5                   jmp     ebp
			dwRet = 4;
			break;
		}
	}

	if (dwRet != 4 && bRet == FALSE)
	{
		return FALSE;
	}

	else if (bRet == TRUE && (dwRet == 2))
	{
		RVA2FileOff(dwAddr2, &dwAddr);
		bRet = GetFileBuffer(&byBuf[0], dwAddr, 0x100, 0x100, &dwReadByte);
		if ((bRet == FALSE) || (dwReadByte != 0x100))
		{
			return FALSE;
		}
		bRet = FALSE;
		dwReadByte = dwReadByte - 0x20;
		for (i = 0; (i < dwReadByte) && (dwRet < 4); i++)
		{
			if ((dwRet == 2) && (*(DWORD *)&byBuf[i] == 0x24244481))
			{
				//81 44 24 24 8B 7C FF FF add     dword ptr[esp + 24h], 0FFFF7C8Bh

				dwAddr1 = dwAddr1 + *(DWORD *)&byBuf[i + 4];
				bFlag1 = TRUE;
				dwRet = 3;
			}
			else if (((dwRet == 2) || (dwRet == 3)) && (byBuf[i] == 0xE9))
			{
				//E9 90 69 00 00                          jmp     loc_673D8A
				if ((dwRet == 2))
				{
					bFlag = TRUE;
				}
				dwAddr2 = dwAddr2 + i + 5 + *(DWORD *)&byBuf[i + 1];
				bRet = TRUE;
				dwRet = 3;
				break;
			}
			else if ((dwRet == 3) && (*(WORD *)&byBuf[i] == 0xE5FF))
			{
				//FF E5                   jmp     ebp
				dwRet = 4;
				break;
			}
		}
	}
	if (bRet == TRUE && (dwRet == 3))
	{
		RVA2FileOff(dwAddr2, &dwAddr);
		bRet = GetFileBuffer(&byBuf[0], dwAddr, 0x50, 0x50, &dwReadByte);
		if ((bRet == FALSE) || (dwReadByte != 0x50))
		{
			return FALSE;
		}
		bRet = FALSE;
		dwReadByte = dwReadByte - 0x10;
		for (i = 0; (i < dwReadByte) && (dwRet < 4); i++)
		{

			if ((bFlag == TRUE) && ((dwRet == 2) || (dwRet == 3)) && (*(DWORD *)&byBuf[i] == 0x24244481))
			{
				//81 44 24 24 8B 7C FF FF add     dword ptr[esp + 24h], 0FFFF7C8Bh

				dwAddr1 = dwAddr1 + *(DWORD *)&byBuf[i + 4];
				bFlag1 = TRUE;
				dwRet = 3;
			}
			else if ((bFlag1 == TRUE) && (dwRet == 3) && (*(WORD *)&byBuf[i] == 0xE5FF))
			{
				//FF E5                   jmp     ebp
				dwRet = 4;
				break;
			}
		}
	}

	if (dwRet != 4)
	{
		return FALSE;
	}

	dwRet = WriteAEP(dwAddr1);
	if (dwRet != 2)
	{
		return FALSE;
	}

	return TRUE;
}


/**********************************************************************************************************
*  Function Name : WriteElfanew
*  Description : Fetching actual E-Lfanew value
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Nihar
	
*  Date : 14 Dec 2016
* *********************************************************************************************************/
DWORD CISpyRepair::WriteElfanew()
{
	DWORD dwReadByte = 0x00;
	DWORD dwBytesWritten = 0x00;
	DWORD dwPELocation = 0x00;
	DWORD dwArg[1] = {0x00};

	BYTE bBuffer[0x100] = { 0 };

	bool bPELocFound = false;
	

	SetFilePointer(m_hFileHandle, 0x00, NULL, FILE_BEGIN);
	ReadFile(m_hFileHandle, bBuffer, 0x100, &dwReadByte, NULL);
	if (dwReadByte != 0x100)
		return 0;
	
	DWORD dwOffset = 0x00;

	for (dwOffset = 0x00; dwOffset < 0x100; dwOffset++)
	{

		//if (!memcmp(&m_byBuffer[dwOffset], "PE", 0x02))
		if ((0x4550 == (*((DWORD*)&bBuffer[dwOffset]))))
		{
			dwPELocation = dwOffset;
			bPELocFound = true;
			break;
		}

	}

	if (!bPELocFound)
		return 0;

	 WriteBuffer(&dwPELocation, 0x3C, 0x04, 0x04, &dwBytesWritten);
	 if (dwBytesWritten != 0x00)
	 {
		 return 0x02;
	 }

	 return 0;
}

DWORD CISpyRepair::JumpToOffset()
{
	return JumpToOffset(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]));
}


/**********************************************************************************************************
*  Function Name : JumpToOffset
*  Description : Jumping to a certain offset and saving it in a variable
*  SR.NO : VIRUSREPAIR_0032
*  Author Name : Nihar
*  Date : 16 Dec 2016
* *********************************************************************************************************/
DWORD CISpyRepair::JumpToOffset(DWORD dwReadOffset, DWORD dwBytesToRead)
{

	DWORD dwOffset = 0x00;
	DWORD dwReadByte = 0x00;

	if (dwReadOffset > m_dwFileSize || dwBytesToRead <= 0x00)
		return 0x00;

	SetFilePointer(m_hFileHandle, dwReadOffset, NULL, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwOffset, 0x04, &dwReadByte, NULL);
	if (dwReadByte != 0x04)
		return 0x00;

	m_dwReturnValue[0] = dwOffset;

	return 0x02;
}





bool CISpyRepair::ByteXOR()
{
	return ByteXOR(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]), static_cast<BYTE>(m_dwArgs[2]));
}

/**********************************************************************************************************
*  Function Name  :	ByteXOR
*  Description    :	Allows to perform XOR operation Byte-by-Byte with a (Byte) XORKey
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 2 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::ByteXOR(DWORD dwOffset, DWORD dwLength, BYTE XORKey)
{
	bool bReturn = false;

	if (dwOffset > m_dwFileSize)
	{
		return false;
	}

	BYTE *pXORBuffer = new BYTE[dwLength];
	if (pXORBuffer == NULL)
	{
		goto CLEANUP;
	}

	memset(pXORBuffer, 0, dwLength);

	if (!GetFileBuffer(pXORBuffer, dwOffset, dwLength, dwLength))
	{
		goto CLEANUP;
	}

	for (DWORD i = 0x0; i < (dwLength); i++)
	{
		pXORBuffer[i] ^= XORKey;
	}

	if (!WriteBuffer(pXORBuffer, dwOffset, dwLength, dwLength))
	{
		goto CLEANUP;
	}

	bReturn = true;

CLEANUP:

	if (pXORBuffer != NULL)
	{
		delete[]pXORBuffer;
		pXORBuffer = NULL;
	}
	
	return bReturn;
}


/**********************************************************************************************************
*  Function Name  :	GetAEPFromLastSecName
*  Description    :	Used by Virus.Win32.Kate.a, as the  AEP is stored in last 4 bytes of Last section name
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 2 Dec 2013
**********************************************************************************************************/
DWORD CISpyRepair::GetAEPFromLastSecName()
{
	DWORD dwOffset = 0x00;
	DWORD dwReadByte = 0x00;
	DWORD dwAEP = 0x00;

	if (m_stPEHeader.NumberOfSections != 0x0)
	{
	
		dwOffset = m_stPEOffsets.Magic + m_stPEHeader.SizeOfOptionalHeader + (((m_stPEHeader.NumberOfSections) - 1) * sizeof(IMAGE_SECTION_HEADER)) + 0x04;

		SetFilePointer(m_hFileHandle, dwOffset, NULL, FILE_BEGIN);
		ReadFile(m_hFileHandle, &dwAEP, 0x04, &dwReadByte, NULL);
		
		if (dwReadByte == 0x00)
		{
			return 0x00;
		}

		if ((dwAEP <= 0x00) ||
			(dwAEP > (m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].VirtualAddress + m_stSectionHeader[m_stPEHeader.NumberOfSections - 1].SizeOfRawData)))
		{
			return 0x00;
			
		}
		else
		{
			return WriteAEP(dwAEP);
	
		}
	}
	else
	{
		return 0x00;
	}
}



/**********************************************************************************************************
*  Function Name  :	Savior1904
*  Description    :	Used by Virus.Win32.Savior.1904
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 2 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::Savior()
{
	DWORD	dwRet = Savior(static_cast<DWORD>(m_dwArgs[0]));

	if (dwRet == 0x00)
		return false;

	return true;
}


DWORD CISpyRepair::Savior(DWORD dwAEP)
{
	DWORD dwReadByte = 0x00;
	DWORD dwEAX = 0x00;
	DWORD64 dwEBX = 0x00;
	DWORD dwECX = 0x00;
	DWORD dwEBP = 0x00;
	DWORD64 dwCarry = 0x00;
	DWORD dwTempSwap = 0x00;
	DWORD dwDistance = 0x00;

	SetFilePointer(m_hFileHandle, (dwAEP + 0x1F), NULL, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwDistance, 0x04, &dwReadByte, NULL);
	
	if (dwReadByte == 0x00)
	{
		return 0x00;
	}

	if (dwDistance < 0x31)
	{
		return 0x00;
	}

	SetFilePointer(m_hFileHandle, (dwAEP + 0xE), NULL, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwEAX, 0x04, &dwReadByte, NULL);

	if (dwReadByte == 0x00)
	{
		return 0x00;
	}

	dwEBX = dwEAX;

	dwReadByte = 0x00;
	SetFilePointer(m_hFileHandle, (dwAEP + 0x18), NULL, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwECX, 0x04, &dwReadByte, NULL);
	if (dwReadByte == 0x00)
	{
		return 0x00;
	}

	
	DWORD  dwBuffLen = dwDistance-0x30;

	dwBuffLen = dwBuffLen / 4;

	for (DWORD iCount = 1; iCount < dwBuffLen; dwBuffLen--)
	{

		DWORD dwLoc = dwAEP + 0x30 + (dwBuffLen * 4);
		DwordXOR(dwLoc, 0x04, dwEAX);
		
		dwEAX = dwEAX + dwEBX;
		dwCarry = dwEBX * 2;

		if ((dwEBX * 2) > 0xFFFFFFFF)
		{
			dwTempSwap = dwCarry;
			dwTempSwap = dwTempSwap + 1;
			dwEBX = dwTempSwap;
		}
		else
		{
			dwEBX = dwCarry;
		
		}
	}
	
	return 0x02;
}
/**********************************************************************************************************
*  Function Name  :	FillWithZeros
*  Description    :	Fill's 0x90 in file at specific offset as per size specified
*  SR.NO		  : VIRUSREPAIR_0019
*  Author Name    :	Nihar
*  Date           : 30 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::FillWithNOP()
{
	return FillWithNOP(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]));
}

/**********************************************************************************************************
*  Function Name  :	FillWithZeros
*  Description    :	Fill's 0x90 in file at specific offset as per size specified
*  SR.NO		  : VIRUSREPAIR_0019
*  Author Name    :	Nihar
*  Date           : 30 Nov 2013
**********************************************************************************************************/
bool CISpyRepair::FillWithNOP(DWORD dwStartOff, DWORD dwFillWithZeroSize)
{
	DWORD dwChunk = 0x1000;
	if (dwFillWithZeroSize < dwChunk)
	{
		dwChunk = dwFillWithZeroSize;
	}
	BYTE *pbyFillZeroBuff = new BYTE[dwChunk];

	if (!pbyFillZeroBuff)
	{
		return false;
	}
	memset(pbyFillZeroBuff, 0x90, dwChunk);

	DWORD dwBytes2Write = dwChunk, dwBytesWritten = 0x0;
	::SetFilePointer(m_hFileHandle, dwStartOff, 0, FILE_BEGIN);

	for (DWORD dwTotalBytesWritten = 0x00; dwTotalBytesWritten < dwFillWithZeroSize; dwTotalBytesWritten += dwBytesWritten)
	{
		if ((dwFillWithZeroSize - dwTotalBytesWritten) < dwChunk)
		{
			dwBytes2Write = dwFillWithZeroSize - dwTotalBytesWritten;
		}
		WriteFile(m_hFileHandle, pbyFillZeroBuff, dwBytes2Write, &dwBytesWritten, 0);
		if (0 == dwBytesWritten)
		{
			break;
		}
	}
	
	delete[]pbyFillZeroBuff;
	pbyFillZeroBuff = NULL;

	return true;
}

bool CISpyRepair::ByteNOT()
{
	return ByteNOT(static_cast<DWORD>(m_dwArgs[0]), static_cast<DWORD>(m_dwArgs[1]));
}

/**********************************************************************************************************
*  Function Name  :	ByteNOT
*  Description    :	Allows to perform XOR operation Byte-by-Byte with a (Byte) XORKey
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 2 Dec 2013
**********************************************************************************************************/
bool CISpyRepair::ByteNOT(DWORD dwOffset, DWORD dwLength)
{
	bool bReturn = false;

	if (dwOffset > m_dwFileSize)
	{
		return false;
	}

	BYTE *pNOTBuffer = new BYTE[dwLength];
	if (pNOTBuffer == NULL)
	{
		goto CLEANUP;
	}

	memset(pNOTBuffer, 0, dwLength);

	if (!GetFileBuffer(pNOTBuffer, dwOffset, dwLength, dwLength))
	{
		goto CLEANUP;
	}

	for (DWORD i = 0x0; i < (dwLength); i++)
	{
		pNOTBuffer[i] = ~(pNOTBuffer[i]);
	}

	if (!WriteBuffer(pNOTBuffer, dwOffset, dwLength, dwLength))
	{
		goto CLEANUP;
	}

	bReturn = true;

CLEANUP:

	if (pNOTBuffer != NULL)
	{
		delete[]pNOTBuffer;
		pNOTBuffer = NULL;
	}

	return bReturn;
}


/**********************************************************************************************************
*  Function Name  :	Ramlide
*  Description    :	Used by Virus.Win32.Ramlide
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 5-7-17
**********************************************************************************************************/
bool CISpyRepair::Ramlide()
{
	DWORD	dwRet = Ramlide(static_cast<DWORD>(m_dwArgs[0]));

	if (dwRet == 0x00)
		return false;

	return true;
}

/**********************************************************************************************************
*  Function Name  :	Ramlide
*  Description    :	Used by Virus.Win32.Ramlide
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 5-7-17
**********************************************************************************************************/
DWORD CISpyRepair::Ramlide(DWORD dwAEP)
{
	DWORD dwKeyLoc = 0x00;
	DWORD dwEAX = 0x00;//key
	DWORD dwEDI = 0x00;
	DWORD dwECX = 0x2950;//Buffer Size is constant
	DWORD dwReadByte = 0x00;
	DWORD dwCount = 0x00;
	//Start of the buffer
	dwEDI = dwAEP + 0x21;

	//Performing NOT operation on the entire Buffer
	ByteNOT(dwEDI, dwECX);

	dwKeyLoc = dwAEP + 0x2959;

	SetFilePointer(m_hFileHandle, dwKeyLoc, 0, FILE_BEGIN);

	ReadFile(m_hFileHandle, &dwEAX, 0x04, &dwReadByte, NULL);
	if (dwReadByte == 0x00)
	{
		return 0x00;
	}

	dwECX = 0x0A4B;// dwECX * 4 = Buffer size for XOR operation with key dwEAX

	for (; dwCount < dwECX; dwCount++)
	{

		DwordXOR(dwEDI, 0x04, dwEAX);
		dwEAX -= 0x16;
		dwEDI += 0x04;

	}
	
	return 0x02;

}


/**********************************************************************************************************
*  Function Name  :	Adson
*  Description    :	Used by Virus.Win32.Adson
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 11-7-17
**********************************************************************************************************/
bool CISpyRepair::Adson()
{
	DWORD	dwRet = Adson(static_cast<DWORD>(m_dwArgs[0]));

	if (dwRet == 0x00)
		return false;

	return true;
}

/**********************************************************************************************************
*  Function Name  :	Adson
*  Description    :	Used by Virus.Win32.Adson
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 11-7-17
**********************************************************************************************************/
DWORD CISpyRepair::Adson(DWORD dwAEP)
{

	DWORD dwEBP = 0x00;
	DWORD dwESI = 0x00;
	DWORD dwEDI = 0x00;
	DWORD dwECX = 0x00;
	DWORD64 dwEAX = 0x00;
	DWORD64 dwEDX = 0x00;
	DWORD64 dwKey = 0x00;
	DWORD dwReadBytes = 0x00;
	DWORD dwCount = 0x00;
	DWORD64 dwMulResult = 0x00;

	SetFilePointer(m_hFileHandle, (dwAEP + 0x03), 0, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwEBP, 0x04, &dwReadBytes, NULL);

	if (dwReadBytes == 0x00)
		return 0x00;
	

	SetFilePointer(m_hFileHandle, (dwAEP + 0x09), 0, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwESI, 0x04, &dwReadBytes, NULL);

	if (dwReadBytes == 0x00)
		return 0x00;
	

	dwESI = dwEBP + dwESI;

	dwESI = dwESI - (m_stPEHeader.ImageBase);

	RVA2FileOff(dwESI, &dwESI);

	
	SetFilePointer(m_hFileHandle, (dwAEP + 0x10), 0, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwECX, 0x04, &dwReadBytes, NULL);

	if (dwReadBytes == 0x00)
		return 0x00;

	if (dwECX == 0x00)
		return 0x00;

	SetFilePointer(m_hFileHandle, (dwAEP + 0x15), 0, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwKey, 0x04, &dwReadBytes, NULL);

	if (dwReadBytes == 0x00)
		return 0x00;

	if (dwKey == 0x00)
		return 0x00;

	for (; dwCount < dwECX; dwECX--)
	{
		SetFilePointer(m_hFileHandle, dwESI, 0, FILE_BEGIN);
		ReadFile(m_hFileHandle, &dwEDX, 0x04, &dwReadBytes, NULL);

		if (dwReadBytes == 0x00)
			return 0x00;

		dwEDI = dwESI;
		dwESI = dwESI + 0x04;

		SetFilePointer(m_hFileHandle, dwESI, 0, FILE_BEGIN);
		ReadFile(m_hFileHandle, &dwEAX, 0x04, &dwReadBytes, NULL);

		if (dwReadBytes == 0x00)
			return 0x00;


		if (dwEDX > dwKey)
		{	
			if (!WriteBuffer(&dwEAX, dwEDI, 0x04, 0x04))
				return 0x00;

			if (!WriteBuffer(&dwEDX, dwESI, 0x04, 0x04))
				return 0x00;

		}
		else
		{
			DWORD dwEBX = 0x00;
			DWORD dwPushEDX = 0x00;

			dwEBX = dwKey;
			dwPushEDX = dwEDX;

			dwMulResult = dwEAX * dwKey;
			
			dwEAX = (DWORD)(dwMulResult & 0xFFFFFFFF);
			dwEDX = (DWORD)(dwMulResult >> 32);


			dwEAX = dwEAX + dwPushEDX;
			if (dwEAX > 0xFFFFFFFF)
			{

				dwEDX = dwEDX + 0x01;
				dwEAX = dwEAX - 0x100000000;
			}

			if (!WriteBuffer(&dwEAX, dwEDI, 0x04, 0x04))
				return 0x00;

			if (!WriteBuffer(&dwEDX, dwESI, 0x04, 0x04))
				return 0x00;

		}

		dwEDI = dwESI;
		dwESI = dwESI + 0x04;

	}

	return 0x02;
}

/**********************************************************************************************************
*  Function Name  :	Priest_1521
*  Description    :	Used by Virus.Win32.Priest.1521
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 11-7-17
**********************************************************************************************************/
bool CISpyRepair::Priest_1521()
{
	DWORD	dwRet = Priest_1521(static_cast<DWORD>(m_dwArgs[0]));

	if (dwRet == 0x00)
		return false;

	return true;
}

/**********************************************************************************************************
*  Function Name  :	Priest_1521
*  Description    :	Used by Virus.Win32.Priest.1521
*  SR.NO		  : VIRUSREPAIR_0023
*  Author Name    :	Nihar
*  Date           : 11-7-17
**********************************************************************************************************/
DWORD CISpyRepair::Priest_1521(DWORD dwAEP)
{
	DWORD dwEBP = 0x00;
	DWORD dwEAX = 0x00;
	DWORD dwESI = 0x00;
	DWORD dwECX = 0x00;
	DWORD dwReadBytes = 0x00;
	DWORD dwCount = 0x00;
	

	SetFilePointer(m_hFileHandle, (dwAEP + 0x08), 0, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwEBP, 0x04, &dwReadBytes, NULL);

	if (dwReadBytes == 0x00)
		return 0x00;


	dwEBP = dwEBP - (m_stPEHeader.ImageBase);
	RVA2FileOff(dwEBP, &dwEBP);

	dwEBP = (dwAEP + 0x05) - dwEBP;

	SetFilePointer(m_hFileHandle, (dwAEP + 0xE), 0, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwEAX, 0x04, &dwReadBytes, NULL);

	if (dwReadBytes == 0x00)
		return 0x00;

	dwECX = 0x5C5;

	SetFilePointer(m_hFileHandle, (dwAEP + 0x14), 0, FILE_BEGIN);
	ReadFile(m_hFileHandle, &dwESI, 0x04, &dwReadBytes, NULL);

	if (dwReadBytes == 0x00)
		return 0x00;

	dwESI = dwESI - (m_stPEHeader.ImageBase);
	RVA2FileOff(dwESI, &dwESI);

	dwESI = dwEBP + dwESI;

	for (; dwCount < dwECX; dwCount++)
	{

		DwordXOR(dwESI, 0x04, dwEAX);
		
		dwESI++;
		
		DWORD dwEAXTemp = 0x00;
		
		WORD wTemp = 0x00;
		WORD wTemp1 = 0x00;
		WORD wTemp2 = 0x00;
		
		BYTE bMask = 0x00;


		dwEAXTemp = dwEAX;
		dwEAX = dwEAX & 0xFFFF0000;

		wTemp = dwEAXTemp & 0xFFFF;

		wTemp1 = wTemp & 0x00FF;
		wTemp1 = wTemp1 << 8;

		wTemp2 = wTemp & 0XFF00;
		wTemp2 = wTemp2 >> 8;


		wTemp = wTemp1 + wTemp2;

		dwEAX = dwEAX + wTemp;
		//dwEAXTemp = dwEAX;

		dwEAXTemp = (dwEAX >> 1);

		bMask = (dwEAX & 0x00000001);

		if (bMask == 0x1)
		{
			dwEAXTemp = dwEAXTemp + 0x80000000;
		}
		dwEAX = dwEAXTemp;

	}

	return 0x02;
	
}