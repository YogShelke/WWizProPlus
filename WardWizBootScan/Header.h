/****************************************************************************
Program Name          : Header.h
Description           : File which contains required constants, ENUMS, structures.
Author Name			  : Ram Shelke
Date Of Creation      : 27th SEP 2017
Version No            : 2.6.0.1
Modification Log      :
*****************************************************************************/
#pragma once

#include "stdafx.h"
#include "ntdll.h"

#define		BUFFER_SIZE					1024
#define		MAX_FILEPATH_LENGTH			MAX_PATH * 8

// end_winnt
//
// This works "generically" for Unicode and Ansi/Oem strings.
// Usage:
//   const static UNICODE_STRING FooU = RTL_CONSTANT_STRING(L"Foo");
//   const static         STRING Foo  = RTL_CONSTANT_STRING( "Foo");
// instead of the slower:
//   UNICODE_STRING FooU;
//           STRING Foo;
//   RtlInitUnicodeString(&FooU, L"Foo");
//          RtlInitString(&Foo ,  "Foo");
//
// Or:
//   const static char szFoo[] = "Foo";
//   const static STRING sFoo = RTL_CONSTANT_STRING(szFoo);
//
// This will compile without error or warning in C++. C will get a warning.
//
#ifdef __cplusplus
extern "C++"
{
	char _RTL_CONSTANT_STRING_type_check(const char *s);
	char _RTL_CONSTANT_STRING_type_check(const WCHAR *s);
	// __typeof would be desirable here instead of sizeof.
	template <size_t N> class _RTL_CONSTANT_STRING_remove_const_template_class;
	template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(char)>  { public: typedef  char T; };
	template <> class _RTL_CONSTANT_STRING_remove_const_template_class<sizeof(WCHAR)> { public: typedef WCHAR T; };
#define _RTL_CONSTANT_STRING_remove_const_macro(s) \
    (const_cast<_RTL_CONSTANT_STRING_remove_const_template_class<sizeof((s)[0])>::T*>(s))
}
#else
char _RTL_CONSTANT_STRING_type_check(const void *s);
#define _RTL_CONSTANT_STRING_remove_const_macro(s) (s)
#endif

#define DECLARE_UNICODE_STRING_SIZE(_var, _size) \
WCHAR _var ## _buffer[_size]; \
UNICODE_STRING _var = { 0, _size * sizeof(WCHAR) , _var ## _buffer }

#define RTL_CONSTANT_STRING(s) \
{ \
    sizeof( s ) - sizeof( (s)[0] ), \
    sizeof( s ) / sizeof(_RTL_CONSTANT_STRING_type_check(s)), \
    _RTL_CONSTANT_STRING_remove_const_macro(s) \
}

typedef struct _KEYBOARD_INPUT_DATA {
	USHORT	UnitId;
	USHORT	MakeCode;
	USHORT	Flags;
	USHORT	Reserved;
	ULONG	ExtraInformation;
}KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;

typedef enum __LOGING_LEVEL
{
	ZEROLEVEL,
	FIRSTLEVEL,
	SECONDLEVEL
}LOGGING_LEVEL;

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

typedef enum __SCAN_SESSION_DETAILS
{
	NOSESSIONDB,
	DELTESESSIONDBFAIL,
	SESSIONDBEXISTS,
	CREATESCANDBFAIL
};
extern int		LOGBASE; 
extern WCHAR	szAppPath[MAX_PATH];

