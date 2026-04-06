

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 05:14:07 2038
 */
/* Compiler settings for C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens.io\LENSShell\LENSShell.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __LENSShell_h__
#define __LENSShell_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __ILENSShell_FWD_DEFINED__
#define __ILENSShell_FWD_DEFINED__
typedef interface ILENSShell ILENSShell;

#endif 	/* __ILENSShell_FWD_DEFINED__ */


#ifndef __LENSShell_FWD_DEFINED__
#define __LENSShell_FWD_DEFINED__

#ifdef __cplusplus
typedef class LENSShell LENSShell;
#else
typedef struct LENSShell LENSShell;
#endif /* __cplusplus */

#endif 	/* __LENSShell_FWD_DEFINED__ */


#ifndef __LENSShellContextMenu_FWD_DEFINED__
#define __LENSShellContextMenu_FWD_DEFINED__

#ifdef __cplusplus
typedef class LENSShellContextMenu LENSShellContextMenu;
#else
typedef struct LENSShellContextMenu LENSShellContextMenu;
#endif /* __cplusplus */

#endif 	/* __LENSShellContextMenu_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ILENSShell_INTERFACE_DEFINED__
#define __ILENSShell_INTERFACE_DEFINED__

/* interface ILENSShell */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ILENSShell;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("677D0B3B-E0A8-4ECC-BD7E-1BCC96E388EA")
    ILENSShell : public IDispatch
    {
    public:
    };
    
    
#else 	/* C style interface */

    typedef struct ILENSShellVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILENSShell * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILENSShell * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILENSShell * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILENSShell * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILENSShell * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILENSShell * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILENSShell * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ILENSShellVtbl;

    interface ILENSShell
    {
        CONST_VTBL struct ILENSShellVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILENSShell_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILENSShell_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILENSShell_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILENSShell_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ILENSShell_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ILENSShell_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ILENSShell_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ILENSShell_INTERFACE_DEFINED__ */



#ifndef __LENSShellLib_LIBRARY_DEFINED__
#define __LENSShellLib_LIBRARY_DEFINED__

/* library LENSShellLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_LENSShellLib;

EXTERN_C const CLSID CLSID_LENSShell;

#ifdef __cplusplus

class DECLSPEC_UUID("9E6ECB90-5A61-42BD-B851-D3297D9C7F39")
LENSShell;
#endif

EXTERN_C const CLSID CLSID_LENSShellContextMenu;

#ifdef __cplusplus

class DECLSPEC_UUID("A7B3F4E1-9C2D-4E8F-B6A1-3D5E7F9C2B4A")
LENSShellContextMenu;
#endif
#endif /* __LENSShellLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


