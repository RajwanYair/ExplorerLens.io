#ifndef _LENSShellCLASS_541926D5_D807_4CCB_9F35_8464657CC196_
#define _LENSShellCLASS_541926D5_D807_4CCB_9F35_8464657CC196_
#pragma once

#include "LENSShell.h" // MIDL generated - defines CLSID_LENSShell, ILENSShell, etc.
#include "resource.h" // main symbols

#include "EngineAdapter.h"
#include "LENSArchive.h"
#include "PropertyStoreImpl.h"
#include <propsys.h>    // IPropertyStore, IPropertyStoreCapabilities
#include <thumbcache.h> // IThumbnailProvider

/////////////////////////////////////////////////////////////////////////////
// CLENSShell

class ATL_NO_VTABLE CLENSShell
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CLENSShell, &CLSID_LENSShell>,
      public IDispatchImpl<ILENSShell, &IID_ILENSShell, &LIBID_LENSShellLib>,
      public IPersistFile,
      public IExtractImage2,
      public IThumbnailProvider,
      public IInitializeWithStream,
      public IQueryInfo,
      public IPropertyStore,
      public IPropertyStoreCapabilities {
public:
  HRESULT FinalConstruct(void);
  void FinalRelease(void);

  BEGIN_COM_MAP(CLENSShell)
  COM_INTERFACE_ENTRY(ILENSShell)
  COM_INTERFACE_ENTRY(IPersistFile)
  COM_INTERFACE_ENTRY(IQueryInfo)
  // Modern Windows 10/11 interfaces
  COM_INTERFACE_ENTRY(IThumbnailProvider)
  COM_INTERFACE_ENTRY(IInitializeWithStream)
  // Legacy Windows 7-8 interfaces
  COM_INTERFACE_ENTRY(IExtractImage)
  COM_INTERFACE_ENTRY(IExtractImage2)
  COM_INTERFACE_ENTRY(IDispatch)
  // Property store for Explorer Details Pane (v15.0.0)
  COM_INTERFACE_ENTRY(IPropertyStore)
  COM_INTERFACE_ENTRY(IPropertyStoreCapabilities)
  END_COM_MAP()

  DECLARE_REGISTRY_RESOURCEID(IDR_LENSShell)
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  // IPersistFile
  STDMETHOD(Load)(LPCOLESTR wszFile, DWORD dwMode) {
    return m_LENS.OnLoad(wszFile);
  }
  STDMETHOD(GetClassID)(LPCLSID clsid) { return E_NOTIMPL; }
  STDMETHOD(IsDirty)(VOID) { return E_NOTIMPL; }
  STDMETHOD(Save)(LPCOLESTR, BOOL) { return E_NOTIMPL; }
  STDMETHOD(SaveCompleted)(LPCOLESTR) { return E_NOTIMPL; }
  STDMETHOD(GetCurFile)(LPOLESTR FAR *) { return E_NOTIMPL; }

  // IExtractImage/IExtractImage2
  STDMETHOD(GetLocation)(LPWSTR pszPathBuffer, DWORD cchMax, DWORD *pdwPriority,
                         const SIZE *prgSize, DWORD dwRecClrDepth,
                         DWORD *pdwFlags) {
    return m_LENS.OnGetLocation(prgSize, pdwFlags);
  }
  STDMETHOD(Extract)(HBITMAP *phBmpThumbnail) {
    return m_LENS.OnExtract(phBmpThumbnail);
  }
  // IExtractImage2
  STDMETHOD(GetDateStamp)(FILETIME *pDateStamp) {
    return m_LENS.OnGetDateStamp(pDateStamp);
  }

  // IQueryInfo
  STDMETHOD(GetInfoTip)(DWORD dwFlags, LPWSTR *ppwszTip) {
    return m_LENS.OnGetInfoTip(ppwszTip);
  }
  STDMETHOD(GetInfoFlags)(LPDWORD pdwFlags) {
    *pdwFlags = 0;
    return E_NOTIMPL;
  }

  // IThumbnailProvider - Modern Windows 10/11 interface
  STDMETHOD(GetThumbnail)(UINT cx, HBITMAP *phBmpThumbnail,
                          WTS_ALPHATYPE *pdwAlpha);

  // IInitializeWithStream - Modern stream-based initialization
  STDMETHOD(Initialize)(IStream *pstream, DWORD grfMode);

  // IPropertyStore - Explorer Details Pane metadata (v15.0.0)
  STDMETHOD(GetCount)(DWORD *cProps);
  STDMETHOD(GetAt)(DWORD iProp, PROPERTYKEY *pkey);
  STDMETHOD(GetValue)(REFPROPERTYKEY key, PROPVARIANT *pv);
  STDMETHOD(SetValue)(REFPROPERTYKEY key, REFPROPVARIANT propvar);
  STDMETHOD(Commit)();

  // IPropertyStoreCapabilities
  STDMETHOD(IsPropertyWritable)(REFPROPERTYKEY key);

private:
  // Internal thumbnail generation (Sprint 22: SEH-safe implementation)
  HRESULT GetThumbnail_Internal(UINT cx, HBITMAP *phBmpThumbnail,
                                WTS_ALPHATYPE *pdwAlpha);

  __LENS::CLENSArchive m_LENS;
  CComPtr<IStream> m_spStream; // For IInitializeWithStream

  // Property store (v15.0.0)
  ExplorerLens::CLENSPropertyStore m_propertyStore;

  // Engine integration (v5.3.0)
  std::unique_ptr<ExplorerLens::EngineAdapter> m_engineAdapter;
  bool m_useEngine; // Toggle between Engine and legacy implementation
};

#endif //_LENSShellCLASS_541926D5_D807_4CCB_9F35_8464657CC196_
