#ifndef _CONSTANTS_
#define _CONSTANTS_

//******************************************************************************
//Encryption constants
const int			WRDWIZ_KEYSIZE = 4;
const unsigned char WRDWIZ_KEY[WRDWIZ_KEYSIZE + 1] = "AK";
const unsigned int WRDWIZ_KEY_SIZE = 0x10;
const unsigned int WRDWIZ_SIG_SIZE = 0x07;
const unsigned char WRDWIZ_SIG[WRDWIZ_SIG_SIZE + 1] = "ACUTOK";
const unsigned char WRDWIZ_SIG_NEW[WRDWIZ_SIG_SIZE + 1] = "ACUTOK";

#define		MAX_FILEPATH_LENGTH			MAX_PATH * 4

typedef enum _LOGGING_LEVEL
{
	ZEROLEVEL,
	FIRSTLEVEL,
	SECONDLEVEL,
}LOGGINGLEVEL;

//******************************************************************************
//Neha Gharge 20/1/2015.
//Wardwiz and Clam Scanner on the basis of dwScanLevel registry entry.
//******************************************************************************
typedef enum _SCANLEVEL
{
	WARDWIZSCANNER = 0x01,
	CLAMSCANNER
}SCANLEVEL;
//******************************************************************************

//******************************************************************************
// DB FILE NAMES
//******************************************************************************
const TCHAR WRDWIZAV1[20] = L"VIBRANIUMAV1.DB";
const TCHAR WRDWIZAVR[20] = L"VIBRANIUMAVR.DB";
//******************************************************************************

//******************************************************************************
// INI FILE NAMES
//******************************************************************************
const TCHAR WRDWIZRECOVERINI[16] = L"WWRECOVER.INI";
const TCHAR WRDWIZREPAIRINI[16] = L"WWREPAIR.INI";
const TCHAR WRDWIZALUDELINI[16] = L"ALUDEL.INI";
//******************************************************************************

//******************************************************************************
// REPAIR ERROR CODES
//******************************************************************************
#define REBOOTREPAIR			L"Reboot repair"
#define REBOOTDELETE			L"Reboot quarantine"
//******************************************************************************


//******************************************************************************
// DATA ENC/DEC CODES
//******************************************************************************
#define WARDWIZSERVICENAME				L"VibraniumScan"
#define WARDWIZDISPLAYNAME				L"Vibranium Scanner Module"
#define WARDWIZUPDATESERVICENAME		L"VBSmartUpdate"
#define WARDWIZUPDATEDISPLAYNAME		L"Vibranium Smart update"
#define WARDWIZEPSCLIENTSERVNAME		L"VibraniumClientAgent"
#define WARDWIZEPSCLIENTSERVPLAYNAME	L"Vibranium Client Agent Module"
//******************************************************************************

//******************************************************************************
// DATA ENC/DEC CODES
//******************************************************************************
#define WRDWIZCRYPT			L"\\VBCRYPT.EXE"
//******************************************************************************

typedef enum _DATACRYPTOPR
{
	ENCRYPTION = 0x01,
	DECRYPTION,
	KEEPORIGINAL,
	DELETEORIGINAL,
	INPROGRESS,
	OPR_FAILED,
	ENCRYPT_SUCCESS,
	DECRYPT_SUCCESS,
	ALREADY_ENCRYPTED,
	NOT_ENCRYPTED,
	CRYPT_FINISHED,
	INVALID_FILE,
	PASS_MISMATCH,
	ENC_INPROGRESS,
	DEC_INPROGRESS,
	INSERT_NEW_ITEM,
	OPR_CANCELED,
	SAVE_AS,
	OPR_FILE_COUNT,
	FILE_SIZE_MORETHEN_3GB,
	FILE_NOT_FOUND,
	OPR_VERSION_MISMATCH,
	FILE_LOCKING,
	FILE_LOCKING_SUCCESS,
	INTIGRITY_CHECKING,
	INTIGRITY_FAILED,
	INTIGRITY_ROLLBACK_FAILED,
	SYSTEM_FILE_FOLDER_PATH,
	WARDWIZ_DB_FILE,
	ZERO_KB_FILE,
	DISK_SPACE_LOW,
	FILE_ENC_USING_OLD_VERSION,
	SHOW_STATUS_ON_UI,
	OPR_ABORTED,
	FILE_REMOVBL_DEVICE

}DATACRYPTOPR;
//******************************************************************************


//******************************************************************************
// AUTO-LIVE UPDATE ERROR CODES
//******************************************************************************
typedef enum _ALUPDATESTATUS
{
	ALUPDATEDSUCCESSFULLY = 0x00,
	ALUPDATED_UPTODATE,
	ALUPDATEFAILED_INTERNETCONNECTION,
	ALUPDATEFAILED_DOWNLOADINIPARSE,
	ALUPDATEFAILED_DOWNLOADFILE,
	ALUPDATEFAILED_EXTRACTFILE,
	ALUPDATEFAILED_UPDATINGFILE,
	ALUPDATEFAILED_LOWDISKSPACE
}ALUPDATESTATUS;
//******************************************************************************

//******************************************************************************
// AUTO-LIVE UPDATE ERROR CODES
//******************************************************************************
typedef enum _tagSYSTEMTRAY_SETTING_INDEX
{ 
	eEMAIL = 1, 
	eTRAYNOTIFICATION, 
	eSTARTUPSCAN, 
	eAUTOLIVESCAN, 
	eUSBDECTION, 
	eEXIT 
} SYSTEMTRAY_SETTING_INDEX;


/******************************************************************************/
//							MESSAGE HANDLERS
//******************************************************************************
#define WM_MESSAGESHOWREG (WM_USER + 100)
#define WM_MESSAGEMAXIMWND (WM_USER + 101)
//******************************************************************************

/******************************************************************************/
// DATA ENC/DEC CODES
//******************************************************************************
#define WWUPDATECMPLTEVENT			L"UpdateOprCmpltedEvent"
//******************************************************************************


#ifdef _UNICODE
//******************************************************************************
/*									REGISTRATION CONSTANTS					   */
//*********************************************************************/
static const TCHAR * WARDWIZDOMAINNAME						= L"vibranium.co.in";
static const TCHAR * WARDWIZLOCALDOMAINNAME					= L"192.168.2.101/projects/vibranium";
static const TCHAR * UPDATEUSERINFOPAGE						= L"UpdateRegInfo.php";
//******************************************************************************
#endif

//******************************************************************************
// Dialogtype for show /.../ file path status
//******************************************************************************
typedef enum _DIALOGTYPE
{
	ESCANDIALOG = 0x01,
	EUSBDETECTDIALOG,
} _DIALOGTYPE;

/*=========================================================================*/
/*							ENUM for file type							   */
/*=========================================================================*/
typedef enum __FILE_TYPE
{
	FILE_AIF,
	FILE_ARCHIVE,
	FILE_IIF,
	FILE_CSV,
	FILE_DAT,
	FILE_DOC,
	FILE_DOCX,
	FILE_MSG,
	FILE_ODT,
	FILE_PAGES,
	FILE_RTF,
	FILE_TEX,
	FILE_TEXT,
	FILE_WPD,
	FILE_WPS,
	FILE_PE,
	FILE_PDF,
	FILE_PPS,
	FILE_PPT,
	FILE_PPTX,
	FILE_XML,
	FILE_XLR,
	FILE_XLS,
	FILE_XLSX,
	FILE_XLT,
	FILE_HTML,
	FILE_XHTML,
	FILE_ASP,
	FILE_ASPX,
	FILE_JS,
	FILE_JSP,
	FILE_RSS,
	FILE_VB,
	FILE_PHP,
	FILE_AUTOIT,
	FILE_NSIS,
	FILE_SHELLCODE,
	FILE_PERL,
	FILE_PYTHON,
	FILE_RUBY,
	FILE_BAT,
	FILE_IMAGE,				//Picture files
	FILE_JAR,
	FILE_M3U,
	FILE_M4A,
	FILE_MID,
	FILE_MPA,
	FILE_MP3,
	FILE_RA,
	FILE_WAV,
	FILE_WMA,
	FILE_3G2,
	FILE_3GP,
	FILE_AVI,
	FILE_FLV,
	FILE_M4V,
	FILE_MOV,
	FILE_MP4,
	FILE_RM,
	FILE_SRT,
	FILE_SWF,
	FILE_VOB,
	FILE_WMV,
	FILE_3DM,
	FILE_3DS,
	FILE_MAX,
	FILE_OBJ,
	FILE_BMP,
	FILE_DDS,
	FILE_GIF,
	FILE_JPG,
	FILE_PNG,
	FILE_PSD,
	FILE_TIF,
	FILE_THM,
	FILE_TIFF,
	FILE_TTF,
	FILE_AI,
	FILE_EPS,
	FILE_PS,
	FILE_SVG,
	FILE_INDD,
	FILE_PCT,
	FILE_OTF,
	FILE_FNT,
	FILE_FON,
	FILE_DRV,
	FILE_CUR,
	FILE_DESKTHEMEPACK,
	FILE_ICO,
	FILE_LNK,
	FILE_CFG,
	FILE_INI,
	FILE_INF,
	FILE_PRF,
	FILE_MIM,
	FILE_7Z,
	FILE_GZ,
	FILE_SITX,
	FILE_TARGZ,
	FILE_MSI,
	FILE_ICS,
	FILE_ACCDB,
	FILE_DBF,
	FILE_MDB,
	FILE_PDB,
	FILE_SQL,
	FILE_CGI,
	FILE_COM,
	FILE_GADGET,
	FILE_PIF,
	FILE_WSF,
	FILE_CER,
	FILE_CSS,
	FILE_NOTKNOWN
}FILETYPE;


//******************************************************************************
//Nitin Kolapkar 07th March 2016.
//India and German differenciation on the basis of dwLocID registry entry.
//******************************************************************************
typedef enum _SETUPLOCID
{
	WARDWIZINDIA = 0x00,
	WARDWIZGERMANY
}SETUPLOCID;

/*=========================================================================*/
/*							ENUM RESET SETTINGS							   */
/*=========================================================================*/
typedef enum __RESETSETTINGS
{
	HEURISTICSCAN,
}RESETSETTINGS;

/*=========================================================================*/
/*							ENUM WARDWI UI PAGE INDEXES					   */
/*=========================================================================*/
typedef enum __TAGPAGEINDEX
{
	HOMEPAGE,
	SCANPAGE,
	TOOLS,
	LIVEUPDATE,
	UTILITIES,
	REPORT,
	SUPPORT
}PAGEINDEX;


/*=========================================================================*/
/*							BOOT TIME SCANNER ENTRIES					   */
/*=========================================================================*/
typedef struct __TAGRECOVERENTRIES
{
	CHAR szFilePath[0x104];
	CHAR szBackupPath[0xC8];
	CHAR szThreatName[0x32];
}BOOTRECOVERENTRIES;


typedef struct __TAGWRDWIZSCANDETAILS
{
	LARGE_INTEGER	lIntstartDatetime;
	LARGE_INTEGER	lIntEndDateTime;
	CHAR			szThreatName[0x32];
	CHAR			szFilePath[0x104];
	DWORD 			dwActionTaken;
}WRDWIZSCANDETAILS;

typedef struct __TAGSCANSESSIONDETAILS
{
	LARGE_INTEGER 	SessionStartDatetime;
	LARGE_INTEGER 	SessionEndDate;
	DWORD 			TotalFilesScanned;
	DWORD 			TotalThreatsFound;
	DWORD 			TotalThreatsCleaned;
}SCANSESSIONDETAILS;

typedef struct __TAGWRDWIZEXCLUDESCAN
{
	WCHAR			szFilePath[MAX_FILEPATH_LENGTH];
	BYTE 			byIsSubFolder;
}WRDWIZEXCLUDESCAN;

typedef struct __TAGWRDWIZEXCLUDEEXT
{
	WCHAR			szExt[0x32];
}WRDWIZEXCLUDEEXT;

typedef struct __TAGWRDWIZSCANBYNAME
{
	WCHAR			szFileName[0x96];
}WRDWIZSCANBYNAME;

/*=========================================================================*/
/*							SEND USER INFO TYPE							   */
/*=========================================================================*/
typedef enum __TAGINFORATIONTYPE
{
	UPDATE_INFO,
	CHECK_PIRACY
}REGINFOTYPE;


#endif