/****************************************************
*  Program Name: ISpyRepair.h                                                                                                   
*  Author Name: Prajakta                                                                                                      
*  Date Of Creation: 28 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once
#include "stdafx.h"
#include "Imagehlp.h"
#include "atlstr.h"

class CISpyRepair;

const int OUT_OF_FILE	= 0xFF;
const int MAX_SEC_NO	= 0x64; //100 in decimal

const int SEC_VS	= 0x8;
const int SEC_SRD	= 0x10;
const int SEC_PRD	= 0x14;
const int MAX_INPUT_STR_PARAM	= 50;
#define HIDWORD64(l) ((DWORD)(((DWORDLONG)(l)>>32) & 0xFFFFFFFF))

typedef struct _tagPE_HEADER
{
    WORD		e_csum;                      
	LONG		e_lfanew;                   

    WORD		NumberOfSections;
    WORD		Characteristics;
    WORD		SizeOfOptionalHeader;
    DWORD		NumberOfSymbols;

	WORD        Magic;
	BYTE		MinorLinkerVersion;
    DWORD       SizeOfCode;
    DWORD       AddressOfEntryPoint;
    DWORD		ImageBase;
    DWORD       SectionAlignment;
    DWORD       FileAlignment;
    WORD		MinorOSVersion;
	WORD		MajorImageVersion;
    WORD		MinorImageVersion;
    WORD        MinorSubsystemVersion;
	DWORD       Win32VersionValue;
    DWORD       SizeOfImage;
    DWORD       CheckSum;
    WORD        Subsystem;
	DWORD		SizeOfStackCommit;
	DWORD		SizeOfHeaders;
    DWORD		LoaderFlags;
    DWORD       NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
}PE_HEADER;

typedef struct _tagPE_OFFSETS
{
	DWORD	NumberOfSections;
	DWORD	Magic;
	DWORD   MajorLinkerVersion;
	DWORD	MinorLinkerVersion;
	DWORD	SizeOfCode;
	DWORD	AddressOfEntryPoint;
	DWORD	BaseOfData;
	DWORD	ImageBase;
	DWORD	SectionAlignment;
	DWORD   MajorOSVersion;
	DWORD	MinorOSVersion;
	DWORD   MajorSubsystemVersion;
	DWORD	MinorSubsystemVersion;
	DWORD	Win32VersionValue;
	DWORD	SizeOfImage;
	DWORD	Checksum;
    DWORD	LoaderFlags;
	DWORD	NoOfDataDirs;
}PE_OFFSETS;

typedef struct _tagPE_HEADER_OFFSETS
{
	_tagPE_HEADER PEHeader;
	_tagPE_OFFSETS PEOffsets;
}PE_HEADER_OFFSETS;

class CISpyRepair
{
	HANDLE	m_hFileHandle;
	bool Load32BitHeader();
	bool Load64BitHeader();
	void LoadOffsets();
		
	DWORD m_dwAEPSectionSize;
	DWORD m_dwAEPSectionStartPos;
	BYTE *m_byBuffer;	

public:
	CISpyRepair(void);
	~CISpyRepair(void);
	
	PE_HEADER	m_stPEHeader;
	PE_OFFSETS	m_stPEOffsets;
	PE_HEADER_OFFSETS m_stPEHeaderOffsets;
	IMAGE_SECTION_HEADER m_stSectionHeader[MAX_SEC_NO];
	
	TCHAR	m_szFilePath[MAX_PATH];
	CString m_csFilePath;

	DWORD	m_dwDesirecAccess;
	DWORD	m_dwSharedMode;
	DWORD	m_dwFileSize;
	DWORD	m_dwAEPMapped;
	DWORD	m_dwStartOffset;
	WORD	m_wAEPSec;
	bool	m_b64bit;
	bool	m_bPEFile;
	bool	m_bMZFound;
	bool	m_bPEFound;
	bool	m_bSecFound;
	DWORD_PTR	m_dwReturnValue[10];
	DWORD_PTR	m_dwArgs[15];
	BYTE		m_byArg[MAX_INPUT_STR_PARAM];
    int			m_ibyArgLength;
	CString 	m_csModParam;
	int			m_iStep;
	int			m_iSaveArg;
	DWORD_PTR	m_dwSaveArgs[10];
	DWORD   m_dwBuffSize;	
	//Function Declarations
	bool OpenFile(LPCTSTR szFilePath, bool bOpenToRepair);
	bool CheckAndLoadValidPEFile(HANDLE hFileHandle, bool bOpenToRepair = false);
	void CloseFile();

	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwBytesToRead, DWORD * pdwBytesRead);
	bool GetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
	bool WriteBuffer(LPVOID pbWriteBufer, DWORD dwWriteOffset, DWORD dwBytesToWrite, DWORD dwMinBytesReq = 0, DWORD *pdwBytesWritten = NULL);
	
	WORD RVA2FileOff(DWORD dwRVAAddr, DWORD *pdwFileOff);
	DWORD GetMappedOff(DWORD dwRVAAddr);
	bool SetFileEnd();
	bool SetFileEnd(DWORD dwTruncateOffset);
	bool CalculateLastSectionProperties();
	bool CalculateImageSize();
	bool WriteSectionCharacteristic(WORD wSectionNo, DWORD dwAttributeValue, DWORD dwAttributeOffset);
	DWORD WriteAEP();
	DWORD WriteAEP(DWORD dwAEPToWrite);
	bool RepairDelete();
	WORD GetSecNoOfFileOff(DWORD dwFileOff);
	bool WriteNoOfSections(WORD wNoOfSections);
	bool FillWithZeros();
	bool FillWithZeros(DWORD dwStartOff, DWORD dwFillWithZeroSize);
	bool CalculateCheckSum();
	bool TruncateFromAEP();
	bool GetBuffer4Decryption();
	bool GetBuffer4Decryption(DWORD dwOffset,DWORD dwSize);
	bool RepairOptionalHeader();
	bool RepairOptionalHeader(int iFieldNo, DWORD dwFieldValue, DWORD dwDataDirSize,bool bSetDataDirSize = false);
	bool DwordXOR();
	bool DwordXOR(DWORD dwOffset,DWORD dwLength,DWORD XORKey);
	bool CopyData(DWORD dwReadOff, DWORD dwWriteOff, DWORD dwSizeOfData);
	DWORD ReplaceOriginalData();
	bool RemoveSections();
	bool RemoveLastSections(WORD wNoOfSec2Remove = 1,bool TruncateFlashData = false);
	bool TruncateFile();
	bool TruncateFile(DWORD dwTruncateOff,bool TruncateFlashData = false);
	bool Search4MZPE(BYTE *bySearchString, DWORD dwStringLen, DWORD dwStartAddr, DWORD dwEndAddr);
	DWORD SearchString();
	bool GetParameters(CString csParam); 
	//DWORD EvaluateExpression(const char* string);
	DWORD_PTR EvaluateExpression(const char* string);
	bool ReturnDecryptedValue();
	bool ReturnDecryptedValue(DWORD CaseNo,DWORD SubCaseNo,DWORD dwKey);
	bool RepairRenamer();
	DWORD RepairVirusStub();
	bool ReInitializeParam();
	bool ReplaceDecryptedDataInFile();
	bool GetAddrToGetEntryData();
	bool GetAddrToGetEntryData(DWORD dwSrcAddr, DWORD dwSize, DWORD dwDstAddr);
	DWORD GetFileEnd();
	bool TruncateFileFromFlash();
	bool Runouce();
	bool LuderEntryReplace();
	bool Gamrue();
	bool GetEntryAddr();
	bool GetEntryAddr(DWORD dwSrcOff);
	bool Hidrag();
	bool QVod();
	bool Resur();
	bool Crytex();
	bool GetEntryAddr_Image();
	bool GetEntryAddr_Image(DWORD dwAddr);
	bool RepairSectionHdr();
	bool RepairSectionHdr(DWORD dwSec, DWORD dwSize);
	bool XorAla_Entry();
	bool XorAla_Entry(DWORD dwOff);
	DWORD ReplaceDataSize();
	DWORD ReplaceDataSize(DWORD dwAddVal);
	bool CalcCallAddr();
	bool CalcCallAddr(DWORD dwAddVal);
	bool VirutEntryPt();
	bool VirutCE();
	bool GetRsrc(BYTE *pbyBuffer, DWORD dwSize, DWORD dwAddr, DWORD dwKey);

	//Added by Nihar for file-infectors Savior, Redemption and Poson
	//on 22-06-2017
	DWORD WriteElfanew();
	DWORD JumpToOffset();
	DWORD JumpToOffset(DWORD dwReadOffset, DWORD dwBytesToRead);
	DWORD GetAEPFromLastSecName();
	bool Savior();
	DWORD Savior(DWORD dwAEP);
	bool ByteXOR();
	bool ByteXOR(DWORD dwOffset, DWORD dwLength, BYTE XORKey);

	//Written by Nihar
	//Fill 0x90 in a buffer
	bool FillWithNOP();
	bool FillWithNOP(DWORD dwStartOff, DWORD dwFillWithZeroSize);
	//Written by Nihar
	//Byte Not operation
	bool ByteNOT();
	bool ByteNOT(DWORD dwOffset, DWORD dwLength);
	//Written by Nihar
	//For Ramlide
	bool Ramlide();
	DWORD Ramlide(DWORD dwAEP);

	//Written by Nihar
	bool Adson();
	DWORD Adson(DWORD dwAEP);

	//Written by Nihar
	//For Virus.Win32.Priest.1521
	bool Priest_1521();
	DWORD Priest_1521(DWORD dwAEP);
};