#ifndef __SCAN_API_H
#define __SCAN_API_H

#if defined (WIN32)
	#include "windows.h"
	#define CHAR_T wchar_t
	#define CALLING_CONVENTION_CDECL __cdecl 
#else
	typedef char	CHAR_T;
	#define CALLING_CONVENTION_CDECL	   /*no cdecl on Linux*/

	// This is a FAKE IStream structure which may be used on non-Windows
	// systems to perform IStream-like scans when the engine calls your 
	// functions to retrieve the content to scan. Note that while the 
	// interface is designed to be kept similar, it is NOT binary
	// compatible with the Windows structure and should not be used directly
	// in a cross-platform code.
	typedef struct
	{
		// This member must always be set to sizeof(IStream). 
		// Note that it is NOT present in the original Windows structure!
		unsigned int	size;
		
		// Context, passed as 'ctx' argument to all functions
		void *          context;
		
		// http://msdn.microsoft.com/en-us/library/aa380011%28v=VS.85%29.aspx
		// This function must read the bytestoread bytes into preallocated 
		// buffer, and store the number of actually read bytes into bytesread.
		// Note that the caller may request to read more bytes than available.
		// This function should return:
		//  - 0 (S_OK) if read succeeded and all bytestoread were read;
		//  - 1 (S_FALSE) if read succeeded but less bytes than bytestoread were read;
		//  - Any other value to indicate read error.
		int (*Read) ( void *buffer, unsigned long bytestoread, unsigned long *bytesread, void * ctx );

		// http://msdn.microsoft.com/en-us/library/aa380043%28v=VS.85%29.aspx
		// This function must seek the current file pointer to a specified position in
		// a file. offset is interpreted according to whence, which could be 0 (STREAM_SEEK_SET),
		// 1 (STREAM_SEEK_CUR) or 2 (STREAM_SEEK_END). newoffset must contain the new absolute
		// stream position from the beginning of the stream. Note that seeks past the end of the
		// stream is not an error (but subsequent read should return error), and that newoffset
		// may be NULL!
		// This function should return 0 if seek succeeed, otherwise error.
		int (*Seek) ( unsigned long long offset, unsigned int whence, unsigned long long *newoffset, void * ctx );

		// Those functions are only used during disinfection
		int (*Write) ( void const *buffer, unsigned long bytestowrite, unsigned long * byteswritten, void * ctx );
		int (*SetSize) ( unsigned long newsize, void * ctx );

	} IStream;
	
#endif


#ifndef SCAN_MIDL_INTERFACE

static const int INIT_FLAG_NO_EXCEPTION_HANDLER = 0x1;

static const int OBJECT_TYPE_FILE         = 0;
static const int OBJECT_TYPE_BOOT_SECTOR  = 1;
static const int OBJECT_TYPE_PROCESS      = 2;
static const int OBJECT_TYPE_REGISTRY_KEY = 3;
static const int OBJECT_TYPE_COOKIE       = 4;
#if defined(SCANAPI_DEPRECATED)
static const int OBJECT_TYPE_MIME_MESSAGE = 5;
#endif

static const int OBJECT_TYPE_STREAM       = 6;
static const int OBJECT_TYPE_MAILBOX_ELEMENT	= 7;


static const int OBJECT_FLAG_MODIFIED		= 0x1;
static const int OBJECT_FLAG_ARCHIVE		= 0x2;
static const int OBJECT_FLAG_PASSWORD		= 0x4;


static const int OBJECT_FLAG_CORRUPTED	= 0x10;
static const int OBJECT_FLAG_PACKED		= 0x20;
static const int OBJECT_FLAG_TEMPORARY	= 0x40;
static const int OBJECT_FLAG_UNKMETHOD	= 0x80;
#if defined(SCANAPI_DEPRECATED)
static const int OBJECT_FLAG_HASMACROS	= 0x100;
#endif
static const int OBJECT_FLAG_UNPACKED	= 0x200;

static const int OBJECT_FLAG_MOVE		= 0x800;           /* when the unpacked child has to be moved,
                                                   it will fail with OBJ_ST_MOVEFAILED
                                                   but the parent will have this flag set */
static const int OBJECT_FLAG_ISBOOTIMAGE	= 0x1000;
static const int OBJECT_FLAG_ADWARE		= 0x2000;
static const int OBJECT_FLAG_SPYWARE	= 0x4000;
static const int OBJECT_FLAG_DIALER		= 0x8000;
static const int OBJECT_FLAG_APP		= 0x10000;
static const int OBJECT_FLAG_COOKIE		= 0x20000;
static const int OBJECT_FLAG_PROC		= 0x40000;
static const int OBJECT_FLAG_REG		= 0x80000;

static const int OBJECT_FLAG_FOLDER           = 0x200000;
static const int OBJECT_FLAG_UNABLETOREAD     = 0x400000;
static const int OBJECT_FLAG_NOTUPDATABLE	= 0x800000;
static const int OBJECT_FLAG_ENCRYPTED_FS	= 0x1000000;
static const int OBJECT_FLAG_CONTAINER        = 0x2000000;
static const int OBJECT_FLAG_ARCHBOMB         = 0x4000000;
static const int OBJECT_FLAG_KEYLOGGER        = 0x8000000;
static const int OBJECT_FLAG_STREAM           = 0x10000000;
static const int OBJECT_FLAG_SYSTEM_FILE      = 0x20000000;
static const int OBJECT_FLAG_DISINFECTABLE    = 0x40000000;
static const int OBJECT_FLAG_MAILBOX          = 0x80000000;

static const int OBJECT_STATUS_SCAN_FAILED  = 0;
static const int OBJECT_STATUS_CLEAN        = 1;
static const int OBJECT_STATUS_DISINFECTED  = 2;
static const int OBJECT_STATUS_INFECTED     = 3;
static const int OBJECT_STATUS_SUSPICIOUS   = 4;
static const int OBJECT_STATUS_DELETED      = 5;
static const int OBJECT_STATUS_MOVED        = 6;
static const int OBJECT_STATUS_HIDDEN       = 7;
static const int OBJECT_STATUS_UPDATEFAILED   = 8;
static const int OBJECT_STATUS_DELETED_REBOOT = 9;
static const int OBJECT_STATUS_PASSWORD_PROTECTED = 10;
static const int OBJECT_STATUS_MOVED_REBOOT  = 11;

static const int OBJECT_STATUS_FLAG_UPDATEFAILED	  = (0x80000000 | 0x1000);
static const int OBJECT_STATUS_FLAG_REBOOTNEEDED = 0x40000000;
static const int OBJECT_STATUS_FLAG_FAILED		  = 0x80000000;

static const int THREAT_TYPE_VIRUS   = 0;
static const int THREAT_TYPE_SPYWARE = 1;
static const int THREAT_TYPE_ADWARE  = 2;
static const int THREAT_TYPE_SPAM    = 3;
static const int THREAT_TYPE_ROOTKIT = 4;
static const int THREAT_TYPE_DIALER  = 5;
static const int THREAT_TYPE_APP     = 6;
static const int THREAT_TYPE_ARCHBOMB = 7;

static const int PRIORITY_NORMAL       = 0;
static const int PRIORITY_BELOW_NORMAL = 1;
static const int PRIORITY_LOW          = 2;
static const int PRIORITY_ABOVE_NORMAL = 3;
static const int PRIORITY_HIGH         = 4;

static const int SCAN_ACTION_DEFAULT   = 0;
static const int SCAN_ACTION_NONE      = 1;
static const int SCAN_ACTION_ABORT     = 2;
static const int SCAN_ACTION_DISINFECT = 3;
#if defined(SCANAPI_DEPRECATED)
static const int SCAN_ACTION_RENAME    = 4;
#endif
static const int SCAN_ACTION_DELETE    = 5;
static const int SCAN_ACTION_QUAR_COPY = 6;
static const int SCAN_ACTION_QUAR_MOVE = 7;
#if defined(SCANAPI_DEPRECATED)
static const int SCAN_ACTION_QUAR_COPY_ARCH = 8;
#endif
static const int SCAN_ACTION_QUAR_MOVE_ARCH = 9;
static const int SCAN_ACTION_DISINFECT_ONLY = 10;
static const int SCAN_ACTION_SKIP = 11;

static const int _RKAPI_SCAN_FILES =     1;
static const int _RKAPI_SCAN_PROCESSES = 2;

static const int _optScanPacked   = 25;
static const int _optScanArchives = 26;
static const int _optScanEmail    = 27;

static const int _optMaxArchiveDepthLevel = 40;
static const int _optExtensions   = 43;
static const int _optExcludeExtensions = 44;
static const int _optStopScan	=	56;
static const int _optExtractRatio = 62;
static const int _optFreeModuleInstances = 66;
static const int _optMaxArchiveFileSize	=	71;
static const int _optScanSys      = 76;
static const int _optScanCookies  = 78;
static const int _optScanReg      = 79;

static const int _optScanProc     = 80;
#if defined(SCANAPI_DEPRECATED)
static const int _optScanUTF8     = 82;
#endif

static const int _optBufName = 85;
static const int _actExcludeNames = 86;
static const int _optDeepScan     = 87;
static const int _optFollowLinks  = 88;

static const int _optReportOBJ_FL_REG = 106;
static const int _optCleanMalwareTraces = 107;
static const int _optAllowRebootOps = 110;
static const int _optCheckSignedObjects = 115;
static const int _optMinSizeForExtractRatio = 128;
#if defined(SCANAPI_DEPRECATED)
static const int _optUpdateEngines = 4077;
#endif
static const int _optSmartScan    = 4078;
static const int _optReportCleanArchiveMembers	= 4080;
static const int _optCbkFileOnlyStatus = 4081;
static const int _optComEnableServiceButtons = 4082;
static const int _optEnableCoreifOptions = 4083;
static const int _optCurrentUserPid = 4084;

static const int _optRebootCleanText = 4085;
static const int _optRebootDeletingText = 4086;
static const int _optRebootSuccessText = 4087;
static const int _optRebootFailedText = 4088;
static const int _optRebootExeBaseNameText = 4089;
static const int _optQuarantineRemoveReadOnly = 4090;
static const int _optCurrentDirTransactionId = 4091;
static const int _optSaveDeletedFile = 4092;

static const int _optDisinfectFlags	= 4093;
static const int _optReportEmailElement = 4094;
static const int _optSetUserSid = 4095;

static const int _infSmartScanSkippedFiles		= 5001;
static const int _optAllowNewFileCreation		= 5002;
static const int _optProtectScanBuffer			= 5003;
static const int _optSetMaxMappedSize			= 5005;
static const int _infQuarCopiedFiles			= 5006;
static const int _infQuarMovedFiles				= 5007;


#if defined (WIN32)
static const int _optSilentSubmitDump			= 6001;
static const int _optSilentSubmitDefaultAct		= 6002;
static const int _optEnableContentCallbackHandle = 6005;
static const int _optActivateAvSupport				= 6010;
#endif

static const int CORE_CBK_INIT_FLAG				= 0x80000000;

static const int CORE_CBK_ID_INIT_CACHE_000  = (CORE_CBK_INIT_FLAG | 1);
static const int CORE_CBK_ID_WRITE_CACHE_000 = (CORE_CBK_INIT_FLAG | 2);
static const int CORE_CBK_ID_CREATE_CACHE_CERTIFICATES = (CORE_CBK_INIT_FLAG | 3);

static const int CORE_CBK_ERROR_OK           = 0;
static const int CORE_CBK_ERROR_UNSUPPORTED  = 1;
static const int CORE_CBK_ERROR_GENERIC      = 2;

#endif
static const int optChangeTempPath = 103;

#define SCANAPI_SUCCESS 0

typedef unsigned int (CALLING_CONVENTION_CDECL * CoreInitCallback) (void * initCtx, unsigned int id, unsigned int dataSize, void * dataPtr);


typedef
struct _InitializeParams
{
	int nStructSize;
	void * pQuarHandle;
	CHAR_T * szQuarantinePath;
	CHAR_T * szQuarantineFolder;
	CHAR_T * szTempPath;
	unsigned int nInternalCacheKB;

	unsigned int nMaxAllocatedBlock;
	int	   reserved1; //must be 0
	void * pSmartDBInitParams; //must be NULL at this point
	CHAR_T * reserved3; //must be NULL at this point
	void * pInitCbkContext;
	CoreInitCallback fnInitCallback;
	void * reserved4;
	void * reserved5;

	CHAR_T * szTrufosPath;
	int quarMaxFileSize;

	unsigned int nDisableCloudUpload;
	CHAR_T * szBdncClientId;
	unsigned int nDisableCloud;
	unsigned int nEnableCloudUploadExtra;

	unsigned int nInitFlags;
	
	CHAR_T * szCacheControl;
	
	CHAR_T * reserved6;
} InitializeParams;

typedef
struct _ScanCbkParams
{
	int nStructSize;

	int nObjectType;
	const CHAR_T * szObjectPath;
	const CHAR_T * szParentContainers;
	const CHAR_T * szInnerObject;
	unsigned int uObjectFlags;		//composed of one or more OBJECT_FLAG_...
	int nArchiveFlags;
	int nScanStatus;
	int nScanStatusFlags; //in case nScanStatus indicates failure
	int nThreatType;
	unsigned int uPid;
	unsigned int uRegHKey;
	const CHAR_T * szThreatName;
	const CHAR_T * szSysObjParentContainers;
	const CHAR_T * szSysObjInnerObject;

	const CHAR_T * szTempPath;
	unsigned int   uDepthLevel;
} ScanCbkParams;


typedef
struct _ScanCbkExtScanParams
{
	int nStructSize;

	int nObjectType;
	const CHAR_T * szObjectPath;
	const CHAR_T * szParentContainers;
	const CHAR_T * szInnerObject;
	unsigned int uObjectFlags;		//composed of one or more OBJECT_FLAG_...
	int nScanStatus;
	int nScanStatusFlags; //in case nScanStatus indicates failure
	unsigned int uPid;
	unsigned int uRegHKey;
	const CHAR_T *szSysObjParentContainers;
	const CHAR_T *szSysObjInnerObject;
	void * reserved;
	unsigned int   uDepthLevel;
} ScanCbkExtScanParams;


typedef struct __SCAN_FILE_SIZE 
{
	unsigned int Low, High;
} SCAN_FILE_SIZE;

typedef struct __ScanStatistics
{
	unsigned int dwDirectories;
	unsigned int dwBoots;
	unsigned int dwFiles;
	unsigned int dwArchives;
	unsigned int dwPacked;
	unsigned int dwInfected;
	unsigned int dwSuspected;
	unsigned int dwDisinfected;

	unsigned int unused_dwWarnings;

	unsigned int dwVirusesNumber;
	unsigned int dwDeleted;
	unsigned int dwMoved;
	unsigned int dwRenamed;
	unsigned int dwCopied;
	unsigned int dwIOErrors;
	unsigned int dwOpenErrors;
	unsigned int dwReadErrors;
	unsigned int dwWriteErrors;
	unsigned int StartTime;
	unsigned int EndTime;
	unsigned int CurrentTime;
	SCAN_FILE_SIZE TotalSize;
	unsigned int dwCookies;
	unsigned int dwProcesses;
	unsigned int dwRegKeys;
	unsigned int dwInfCookies;
	unsigned int dwInfProcesses;
	unsigned int dwInfRegKeys;
} ScanStatistics;

/*callbacks needed for ext scan callback*/
typedef int (CALLING_CONVENTION_CDECL *pfnBufOpenFile) (void *pContext);
typedef int (CALLING_CONVENTION_CDECL *pfnBufCloseFile) (void *pContext);
typedef int (CALLING_CONVENTION_CDECL *pfnBufReadFile64) (unsigned char *pBuffer, unsigned int dwFilePointerLow, unsigned int dwFilePointerHigh, unsigned int dwBytesToRead, unsigned int *pdwReadActual, void *pContext);
typedef int (CALLING_CONVENTION_CDECL *pfnBufWriteFile64) (unsigned char *pBuffer, unsigned int dwFilePointerLow, unsigned int dwFilePointerHigh, unsigned int dwBytesToWrite, unsigned int *pdwWriteActual, void *pContext);
typedef int (CALLING_CONVENTION_CDECL *pfnBufGetFileSize64) (unsigned int *pdwFileSizeLow, unsigned int *pdwFileSizeHigh, void *pContext);
typedef int (CALLING_CONVENTION_CDECL *pfnBufSetFileSize64) (unsigned int dwFileSizeLow, unsigned int dwFileSizeHigh, void *pContext);


typedef
struct _ScanExtCbks
{
	int nStructSize;
	pfnBufOpenFile			fnOpenFile;
	pfnBufCloseFile			fnCloseFile;
	pfnBufReadFile64		fnReadFile64;
	pfnBufWriteFile64		fnWriteFile64;
	pfnBufGetFileSize64		fnGetFileSize64;
	pfnBufSetFileSize64		fnSetFileSize64;
	void *					context;
} ScanExtCbks;


typedef
struct _ScanUnload
{
	int nStructSize;
	const CHAR_T * wszEnginesUnloadedPath;
}ScanUnload;

typedef struct _GZFLT_FUNCTIONS GzfltFunctions;
			

typedef void (CALLING_CONVENTION_CDECL *SCAN_CALLBACK)(int objectType, const CHAR_T * objectPath, const CHAR_T * objectName, int archiveFlags, int scanStatus, int threatType, const CHAR_T * threatName, int *scanAction, void * context);

typedef void (CALLING_CONVENTION_CDECL *SCAN2_CALLBACK)( ScanCbkParams * scanParams, int * pnScanAction, void * pContext );
typedef void (CALLING_CONVENTION_CDECL *EXTSCAN_CALLBACK)( ScanCbkExtScanParams * scanParams, ScanExtCbks * extCbks, int * pnScanAction, void * pContext );

typedef void (CALLING_CONVENTION_CDECL *PASSWORD_CALLBACK)(int objectType, const CHAR_T * objectName, CHAR_T * passwordBuf, int * passBufLen, void * context);
typedef void (CALLING_CONVENTION_CDECL *DB_ENUM_CALLBACK)(const CHAR_T * threatName, void * context);
typedef void (CALLING_CONVENTION_CDECL *OBJECT_CALLBACK)(const CHAR_T * objectName, unsigned int id, unsigned int data_size, void *data_ptr, unsigned int * result, void * context, void * core_info);
typedef void (CALLING_CONVENTION_CDECL *ENGINES_UNLOAD_CALLBACK)( ScanUnload * pUnloadParams, void *pContext );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function prototypes

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
typedef struct CThreatScannerStruct CThreatScanner;
typedef	struct BDAntiRansomInfoStruct BDAntiRansomInfo;			
#else
#define CThreatScanner void
#define BDAntiRansomInfo void
#endif

#ifdef WIN32
int CALLING_CONVENTION_CDECL ThreatScanner_GetKeys(CThreatScanner * scanner, void * pKeys, int nrFiles, void * pReserved);
typedef int (CALLING_CONVENTION_CDECL *pfnSMARTDBGETBYFILEID)(CHAR_T *wName, HANDLE hFile, BY_HANDLE_FILE_INFORMATION * bhfi, void * pKeys, void * reserved);
typedef int (CALLING_CONVENTION_CDECL *pfnSMARTDBSETBYFILEID)(CHAR_T *wName, HANDLE hFile, BY_HANDLE_FILE_INFORMATION * bhfi, void * pKeys, void * reserved);
typedef int (CALLING_CONVENTION_CDECL *pfnSMARTDBSEARCHDAT)(HANDLE hFile, BYTE * pBuffer, unsigned int * datFlags, unsigned int addFlags, void * pKeys, BY_HANDLE_FILE_INFORMATION * bfi, void * reserved);


#define BDSMARTDB_VS_OK					0
#define BDSMARTDB_VS_UNUSABLE			1
#define BDSMARTDB_VS_INIT_IN_PROGRESS	2

typedef int (CALLING_CONVENTION_CDECL *SMARTDB_VOLUME_STATE_CALLBACK)(wchar_t driverLetter, const wchar_t* driverGuid, int state);
typedef SMARTDB_VOLUME_STATE_CALLBACK* (CALLING_CONVENTION_CDECL *pfnSMARTDB_REGISTER_VOLUME_STATE_CALLBACK)(SMARTDB_VOLUME_STATE_CALLBACK* pfnCallback, bool bAdd);

#ifndef __SETAVBYFILEID_EXT_PARAMS
#define __SETAVBYFILEID_EXT_PARAMS
typedef struct __SetAVByFileIDExtParams
{
	int nStructSize;
	USN llCurrentUSN;
	const wchar_t* pwzVolumeGuid;
} SetAVByFileIDExtParams;
#endif

typedef int (CALLING_CONVENTION_CDECL *pfnSMARTDBSETBYFILEIDEX)(CHAR_T *wName, HANDLE hFile, BY_HANDLE_FILE_INFORMATION * bhfi, void * pKeys, SetAVByFileIDExtParams * pExtraParams);

typedef
struct _SmartDBExportedFunctions
{
	int nStructSize;
	pfnSMARTDBGETBYFILEID fnSmartDBGetAVByFileID;
	pfnSMARTDBSETBYFILEID fnSmartDBSetAVByFileID;
	pfnSMARTDBSEARCHDAT   fnSmartDBSearchDat;
	pfnSMARTDB_REGISTER_VOLUME_STATE_CALLBACK fnSmartDBRegisterVolumeCbk;
	pfnSMARTDBSETBYFILEIDEX fnSmartDBSetAVByFileIDEx;
}SmartDBExportedFunctions;
#endif

//FIXME: add Doxygen comments here	
int CALLING_CONVENTION_CDECL ThreatScanner_Initialize(const CHAR_T * enginesFolder, const CHAR_T * licenseKey);
int CALLING_CONVENTION_CDECL ThreatScanner_InitializeEx(const CHAR_T *, const CHAR_T *, CHAR_T *, InitializeParams *);
int CALLING_CONVENTION_CDECL ThreatScanner_Uninitialize();
int CALLING_CONVENTION_CDECL ThreatScanner_CreateInstance(CThreatScanner **scanner);
int CALLING_CONVENTION_CDECL ThreatScanner_CreateInstanceEx(CThreatScanner **scanner, int iBlockingMode);
int CALLING_CONVENTION_CDECL ThreatScanner_DestroyInstance(CThreatScanner * scanner);
int CALLING_CONVENTION_CDECL ThreatScanner_GetQuarantine( CThreatScanner * scanner, void ** quarantine );


/**
 * Enables the on-access disinfection support integration
 */
int CALLING_CONVENTION_CDECL ThreatScanner_EnableGZfltIntegration( GzfltFunctions * oasfunctions );

/**
 * Disables the on-access disinfection support integration
 */
int CALLING_CONVENTION_CDECL ThreatScanner_DisableGZfltIntegration();


/**
 * Sets the scan thread priority
 * @param nPriority Specifies how much processor time would the scan process consume.
 */
int CALLING_CONVENTION_CDECL ThreatScanner_SetScanPriority(CThreatScanner * scanner, int priority);

/**
 * Sets the callback called when finished scanning
 * @param pfnCallback The callback to be called for each scan finish or archive member
 * @param pContext This pointer will be passed to the scan callback at each call
 */

int CALLING_CONVENTION_CDECL ThreatScanner_SetScanCallback(CThreatScanner * scanner, SCAN_CALLBACK pfnCallback, void * pContext);
/**
 * Sets the callback2 called when finished scanning
 * @param pfnCallback2 The callback to be called for each scan finish or archive member
 * @param pContext This pointer will be passed to the scan callback at each call
 * this function is called only for archives that are not contained in other archives and for files that are not archives
 */
int CALLING_CONVENTION_CDECL ThreatScanner_SetScanCallback2(CThreatScanner * scanner, SCAN2_CALLBACK pfnCallback2, void * pContext);



int CALLING_CONVENTION_CDECL ThreatScanner_SetExtCallback(CThreatScanner * scanner, EXTSCAN_CALLBACK pfnExtCallback, void * pContext);




/**
 * Scans a path by calling the callback function at the end of scan.
 * @param szPath The path to scan
 */
int CALLING_CONVENTION_CDECL ThreatScanner_ScanPath(CThreatScanner * scanner, int objectType, const CHAR_T * szPath, int accessorPID);

/**
 * Scans a file synchronously.
 * @param szFilePath The complete file path to scan.
 * @param bAttemptClean Specifies if the scanner should attempt to clean an infected object.
 * @param pnScanStatus The object status (clean, infected, disinfected, suspicious)
 * @param pnThreatType The object threat type (virus, spyware, etc.)
 * @param szThreatInfo Usually the name of the virus if infected. If it's an archive with multiple viruses, the virus names are concatenated up to a specified string length (see implementation)
 */
int CALLING_CONVENTION_CDECL ThreatScanner_ScanObject(CThreatScanner * scanner, int objectType, const CHAR_T * szObjectPath, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo, int accessorPID, const CHAR_T * szObjectName);

/**
 * Scans a file synchronously.
 * @param szFilePath The complete file path to scan.
 * @param bAttemptClean Specifies if the scanner should attempt to clean an infected object.
 * @param pnScanStatus The object status (clean, infected, disinfected, suspicious)
 * @param pnThreatType The object threat type (virus, spyware, etc.)
 * @param szThreatInfo Usually the name of the virus if infected. If it's an archive with multiple viruses, the virus names are concatenated up to a specified string length (see implementation)
 */
int CALLING_CONVENTION_CDECL ThreatScanner_ScanObjectByHandle(CThreatScanner * scanner, void * hObject, const CHAR_T * szObjectName, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo, int accessorPID);

/**
 * Prepares the scanner to do in-memory scan using shared memory. Only on Windows by now.
 * @param The shared memory name.
 * @param The shared memory size.
 */
int CALLING_CONVENTION_CDECL ThreatScanner_InitializeMemoryScan(CThreatScanner * scanner, const CHAR_T * mappingName, unsigned long mappingSize);

/**
 * Closes the handles to the shared memory object.
 */
int CALLING_CONVENTION_CDECL ThreatScanner_UninitializeMemoryScan(CThreatScanner * scanner);

/**
 * Scans the shared memory specified with InitializeMemoryScan().
 * @param nObjectType The scan object type (file, mime message, etc.)
 * @param bAttemptClean Specifies if the scanner should attempt to clean an infected object.
 * @param pnScanStatus The object status (clean, infected, disinfected, suspicious)
 * @param pnThreatType The object threat type (virus, spyware, etc.)
 * @param szThreatInfo Usually the name of the virus if infected. If it's an archive with multiple viruses, the virus names are concatenated up to a specified string length (see implementation)
 */
int CALLING_CONVENTION_CDECL ThreatScanner_ScanMemory(CThreatScanner * scanner, int nObjectType, unsigned long * pdwObjectSize, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo);

/**
 * Scans the shared memory specified with InitializeMemoryScan().
 * @param objectName The complete file path to scan.
 * @param nObjectType The scan object type (file, mime message, etc.)
 * @param bAttemptClean Specifies if the scanner should attempt to clean an infected object.
 * @param pnScanStatus The object status (clean, infected, disinfected, suspicious)
 * @param pnThreatType The object threat type (virus, spyware, etc.)
 * @param szThreatInfo Usually the name of the virus if infected. If it's an archive with multiple viruses, the virus names are concatenated up to a specified string length (see implementation)
 */
int CALLING_CONVENTION_CDECL ThreatScanner_ScanMemoryEx(CThreatScanner * scanner, const CHAR_T * objectName, int nObjectType, unsigned long * pdwObjectSize, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo);

/**
 * Sets various scanning options
 */
int CALLING_CONVENTION_CDECL ThreatScanner_SetIntOption(CThreatScanner * scanner, int option, int value);

/**
 * Sets various scanning options
 */
int CALLING_CONVENTION_CDECL ThreatScanner_SetStringOption(CThreatScanner * scanner, int option, const CHAR_T * value);

/**
 * Get information about license expiration time. If 0 is returned, then the license is expired.
 */
int CALLING_CONVENTION_CDECL ThreatScanner_GetLicenseInformation(CThreatScanner * scanner, unsigned long * expirationTime);

int CALLING_CONVENTION_CDECL ThreatScanner_EnumerateDatabaseRecords(CThreatScanner * scanner, DB_ENUM_CALLBACK enumCallback, void * context);

int CALLING_CONVENTION_CDECL ThreatScanner_GetDatabaseInformation(CThreatScanner * scanner, unsigned long * numberOfSignatures, unsigned long * lastUpdateTime);

int CALLING_CONVENTION_CDECL ThreatScanner_SetPasswordCallback(CThreatScanner * scanner, PASSWORD_CALLBACK passwordCallback, void * context);

int CALLING_CONVENTION_CDECL ThreatScanner_ScanBuffer(CThreatScanner * scanner, void * bufPtr, int objectType, unsigned long bufferSize, unsigned long * pulObjectSize, int attemptClean, int * objectStatus, int * pnObjectThreatType, const CHAR_T **objectThreatInfo);

int CALLING_CONVENTION_CDECL ThreatScanner_ScanBufferEx(CThreatScanner * scanner, void * bufPtr, const CHAR_T * objectName, int objectType, unsigned long bufferSize, unsigned long * pulObjectSize, int attemptClean, int * objectStatus, int * pnObjectThreatType, const CHAR_T **objectThreatInfo);

int CALLING_CONVENTION_CDECL ThreatScanner_ScanStream(CThreatScanner * scanner, IStream * stream, int objectType, int attemptClean, int * objectStatus, int * objectThreatType, const CHAR_T **objectThreatInfo);

void CALLING_CONVENTION_CDECL ThreatScanner_DeleteException(void * exception);

int CALLING_CONVENTION_CDECL ThreatScanner_GetOption(CThreatScanner * scanner, int option, void * value);

int CALLING_CONVENTION_CDECL ThreatScanner_GetVersion(int * versionMajor, int * versionMinor);

int CALLING_CONVENTION_CDECL ThreatScanner_SetObjectCallback(CThreatScanner * scanner, OBJECT_CALLBACK objectCallback, void * context);

int CALLING_CONVENTION_CDECL ThreatScanner_SetEnginesUnloadCallback( ENGINES_UNLOAD_CALLBACK enginesUnloadCbk, void *pContext );

int CALLING_CONVENTION_CDECL ThreatScanner_GetScanStatistics(CThreatScanner * scanner, ScanStatistics **ppStatistics);

/**
 * Prepares the scanner to do in-memory scan using shared memory.
 * @param The shared memory name.
 * @param The shared memory size.
 * @param The mapping is done read only or read-write
 */
int CALLING_CONVENTION_CDECL ThreatScanner_InitializeMemoryScanEx(CThreatScanner * scanner, const CHAR_T * mappingName, unsigned long mappingSize, int bIsReadOnly);

int CALLING_CONVENTION_CDECL ThreatScanner_SetExtraOption(CThreatScanner * scanner, int option , void * value);

int CALLING_CONVENTION_CDECL ThreatScanner_ScanQuarantineFile(CThreatScanner * scanner,  void * quarHandle, const CHAR_T * quarId, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo, int pid);

int CALLING_CONVENTION_CDECL ThreatScanner_SetGlobalOption(int option, void * value);
#ifdef WIN32
int CALLING_CONVENTION_CDECL ThreatScanner_GetSmartDBFunctions(SmartDBExportedFunctions * pFunctions);
int CALLING_CONVENTION_CDECL ThreatScanner_GetAntiRansomInfo(BDAntiRansomInfo **ppAntiRansomInfo);
int CALLING_CONVENTION_CDECL ThreatScanner_DestroyAntiRansomInfo(BDAntiRansomInfo *pAntiRansomInfo);
int CALLING_CONVENTION_CDECL ThreatScanner_ScanMappedMemory(CThreatScanner * scanner, int nObjectType, const CHAR_T * szObjectName, UINT64 hGzHandle, LONGLONG dwObjectSize, int * pnScanStatus, int * pnThreatType, const CHAR_T ** szThreatInfo);
#endif
/*
Scan Cbk Params & related
*/

#ifdef __cplusplus
}; // extern "C"
#endif


#endif
