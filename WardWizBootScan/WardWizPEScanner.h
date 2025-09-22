/****************************************************************************
Program Name          : WardWizPEScanner.h
Description           : This file contains scanning mechanism of PE files.
Author Name			  : Ram Shelke
Date Of Creation      : 05th Oct 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#pragma once

#include "WWizLinkedList.h"
#include "WWizCRC64.h"

using namespace std;

#define RT_VERSION      MAKEINTRESOURCE(16)
#define RTLLANGIDFROMLCID(l)  ((WORD)(l))

typedef struct __TAGWRDWIZEXCLUDESCAN
{
	CHAR			szFilePath[MAX_FILEPATH_LENGTH];
	BYTE 			byIsSubFolder;
}WRDWIZEXCLUDESCAN;

typedef struct _tagExcludeFileExtList
{
	char szFileExt[0x32];
}STRUCTEXCLUDEFILEEXTLIST;

typedef struct __TAGWRDWIZSCANBYNAME
{
	char			szFileName[0x96];
}WRDWIZSCANBYNAME;

const PWCHAR WRDWIZAVPEDBNAME = L"WRDWIZHPE";
const PWCHAR AVPE32DBNAME = L"2";
const PWCHAR AVPE64DBNAME = L"4";

const unsigned int MAX_VERSIONCHKBUFF = 0x50;
const unsigned int MAX_SIG_SIZE = 0x08;
const unsigned int WRDWIZ_KEY_SIZE = 0x10;

bool LoadSignatureDB();
bool UnLoadSignatureDB();
bool LoadContaintFromFile(LPTSTR szFilePath, FILETYPE filetype, DWORD dwVersionLength, DWORD &dwSigCount);
bool ISValidDBFile(LPTSTR szFilePath, DWORD &dwDBMajorVersion, DWORD &dwVersionLength);
bool ParseDBVersion(LPSTR lpszVersion, DWORD &dwMajorVersion);
bool RtlGetFileBuffer(LPVOID pbReadBuffer, DWORD dwReadOffset, DWORD dwBytesToRead, DWORD dwMinBytesReq = 0, DWORD *pdwBytesRead = NULL);
bool RtlGetFileBuffer(LPVOID pbReadBuffer, DWORD dwBytesToRead, DWORD * pdwBytesRead);
bool RtlIsValidPEFile(LPTSTR szFilePath);
bool RtlScanFile(LPTSTR pszFilePath, LPTSTR pszVirusName, DWORD &dwSpyID, bool bRescan, bool bIsHeuScan = false);
bool ReadBufferFromDiffSections();
DWORD RVA2FileOffset(DWORD dwRva, WORD nSections, DWORD *pdwFileOff);
DWORD WINAPI GetFileVersionInfoSizeW(LPCWSTR filename, LPDWORD handle, DWORD *offset);
DWORD WINAPI GetFileVersionInfoSizeExW(DWORD flags, LPCWSTR filename, LPDWORD handle, DWORD *offset);
DWORD find_version_resource(HANDLE lzfd, DWORD *reslen, DWORD *offset);
int read_xx_header(HANDLE lzfd);
BOOL find_pe_resource(HANDLE lzfd, DWORD *resLen, DWORD *resOff);
IMAGE_RESOURCE_DIRECTORY *find_entry_by_id(IMAGE_RESOURCE_DIRECTORY *dir, WORD id, const void *root);
IMAGE_RESOURCE_DIRECTORY *find_entry_default(const IMAGE_RESOURCE_DIRECTORY *dir, const void *root);
IMAGE_RESOURCE_DIRECTORY* find_entry_language(IMAGE_RESOURCE_DIRECTORY *dir, const void *root);
int push_language(WORD *list, int pos, WORD lang);
LCID RtlGetUserDefaultLCID(void);
bool MatchSecondarySig(LPBYTE lpFileBuffer, DWORD dwIndex, DWORD &dwVirusID);
bool MatchElementUsingIndex(BYTE *byMD5Buffer, BYTE *byDBSigBuffer, DWORD dwIndex, DWORD & dwVirusID);
DWORD GetActualFileOffsetFromIndex(DWORD dwIndex);
DWORD GetSigCountAsPerListID(DWORD dwListID);
DWORD GetDBDwordName(DWORD dwIndex);
void SetSigCountAsPerListID(DWORD dwListID, DWORD dwCount);
DWORD Encrypt_File(TCHAR *m_szFilePath, TCHAR *szQurFolderPath, TCHAR *lpszTargetFilePath, TCHAR *lpszFileHash, DWORD &dwStatus);
bool CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize, LPBYTE lpKey);
DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize, LPBYTE pbyEncDecKey);
bool BackUpBeforeQuarantineOrRepair(LPTSTR szOriginalThreatPath, LPTSTR lpszBackupFilePath);
bool LoadExcludeDB();
bool ISFileExcluded(LPTSTR szFilePath, bool &bISSubFolderExcluded);
void SetProductID(DWORD dwProdID);
bool LoadExcludeExtDB();
bool ISExcludedExt(LPTSTR szFileName);
bool LoadScanByNameDB();
bool ISScanByFileName(LPTSTR szFileName);
bool FreeHeapAllocMemory();