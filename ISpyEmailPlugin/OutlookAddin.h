

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Fri Aug 29 19:35:22 2025
 */
/* Compiler settings for OutlookAddin.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __OutlookAddin_h__
#define __OutlookAddin_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IAddin_FWD_DEFINED__
#define __IAddin_FWD_DEFINED__
typedef interface IAddin IAddin;
#endif 	/* __IAddin_FWD_DEFINED__ */


#ifndef __Addin_FWD_DEFINED__
#define __Addin_FWD_DEFINED__

#ifdef __cplusplus
typedef class Addin Addin;
#else
typedef struct Addin Addin;
#endif /* __cplusplus */

#endif 	/* __Addin_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IAddin_INTERFACE_DEFINED__
#define __IAddin_INTERFACE_DEFINED__

/* interface IAddin */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IAddin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8A0A29F8-D8C8-4F6D-ABA3-1A8B1B7A463E")
    IAddin : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IAddinVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAddin * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAddin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAddin * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAddin * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAddin * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAddin * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAddin * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IAddinVtbl;

    interface IAddin
    {
        CONST_VTBL struct IAddinVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAddin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAddin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAddin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAddin_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IAddin_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IAddin_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IAddin_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAddin_INTERFACE_DEFINED__ */



#ifndef __OUTLOOKADDINLib_LIBRARY_DEFINED__
#define __OUTLOOKADDINLib_LIBRARY_DEFINED__

/* library OUTLOOKADDINLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_OUTLOOKADDINLib;

EXTERN_C const CLSID CLSID_Addin;

#ifdef __cplusplus

class DECLSPEC_UUID("AA4D594A-D7A6-457E-A583-E7F3303415BE")
Addin;
#endif
#endif /* __OUTLOOKADDINLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


