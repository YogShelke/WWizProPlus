#ifndef _DOWNLOADCONSTANTS_
#define _DOWNLOADCONSTANTS_
#endif

#pragma once
//Enum
enum eLIVE_UPDATE_TYPE
{
	NONE					= 0,
	UPDATEFROMINTERNT		= 1,
	UPDATEFROMLOCALFOLDER	= 2
};

//Constants
const int URL_SIZE			= 100;
/*	ISSUE NO - 867 NAME - NITIN K. TIME - 30th June 2014 */
const int MAX_RETRY_COUNT			= 30;
const int LMDT_SIZE					= 50;
const int DOMAIN_SIZE				= 200;
const int ETAG_SIZE					= 50;
const int MAX_BINARY_SIZE			= 1024;
const int STATUS_INTERVAL			= 3;
const int INTERNET_CHECK_INTERVAL	= 5000;
const int DEFAULT_DOWNLOAD_THREAD	= 15;

#define MAX_PATH_LENGTH		MAX_PATH * 2

enum ConnectionHandle
{
	eSessionHandle = 0,
	eConnectHandle,
	eRequestHandle
};

#pragma pack(1)
typedef struct DownloadFileInfo
{
	DWORD dwFileSize;
	DWORD dwDownloadThreadCount;
	TCHAR szMainUrl[URL_SIZE];
	TCHAR szLocalTempDownloadPath[MAX_PATH_LENGTH];
	TCHAR szLocalPath[MAX_PATH_LENGTH];
	TCHAR szSectionName[50];
	TCHAR szFileMD5[MAX_PATH];
	TCHAR szETAG[MAX_PATH];
	bool bCheckMD5;
	bool bCheckETag;
	TCHAR szExeName[MAX_BINARY_SIZE];
	WORD wPriority;
}STRUCT_DOWNLOAD_INFO, *LPSTRUCT_DOWNLOAD_INFO;
#pragma pack()