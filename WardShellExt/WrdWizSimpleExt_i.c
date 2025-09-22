

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Thu Sep 18 17:55:38 2014
 */
/* Compiler settings for .\WrdWizSimpleExt.idl:
    Oicf, W1, Zp8, env=Win64 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IWrdWizShlExt,0x5E2121ED,0x0300,0x11D4,0x8D,0x3B,0x44,0x45,0x53,0x54,0x00,0x00);


MIDL_DEFINE_GUID(IID, LIBID_SIMPLEEXTLib,0x5E2121E1,0x0300,0x11D4,0x8D,0x3B,0x44,0x45,0x53,0x54,0x00,0x00);


MIDL_DEFINE_GUID(CLSID, CLSID_SimpleShlExt,0x5E2121EE,0x0300,0x11D4,0x8D,0x3B,0x44,0x45,0x53,0x54,0x00,0x00);

// {5C8FA94F-F274-49B9-A5E5-D34C093F7846}
MIDL_DEFINE_GUID(CLSID, CLSID_FilePropSheetExt, 0x5C8FA94F, 0xF274, 0x49B9, 0xA5, 0xE5, 0xD3, 0x4C, 0x09, 0x3F, 0x78, 0x46);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



