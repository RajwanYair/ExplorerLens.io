// CBXshell.cpp : Implementation of DLL Exports.

#include "CBXshell.h"
#include "StdAfx.h"
#include "resource.h"
#include <initguid.h>

#include "CBXShellClass.h"
#include "CBXShell_i.c"
#include "gpu_accelerator.h" // v5.2.0 GPU Acceleration

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_CBXShell, CCBXShell)
END_OBJECT_MAP()

HICON zipIcon;
/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason,
                               LPVOID lpReserved) {
  zipIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

  if (dwReason == DLL_PROCESS_ATTACH) {
    _Module.Init(ObjectMap, hInstance, &LIBID_CBXSHELLLib);
    DisableThreadLibraryCalls(hInstance);

    // v5.2.0: Initialize GPU accelerator (optional, graceful fallback to CPU)
    DarkThumbs::GPUAccelerator::Instance().Initialize(true);
    OutputDebugString(L"[DarkThumbs] GPU accelerator initialized\n");
  } else if (dwReason == DLL_PROCESS_DETACH) {
    // v5.2.0: Shutdown GPU accelerator
    DarkThumbs::GPUAccelerator::Instance().Shutdown();
    OutputDebugString(L"[DarkThumbs] GPU accelerator shut down\n");

    _Module.Term();
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) {
  return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void) {
  return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Adds entries to the system registry
STDAPI DllRegisterServer(void) {
  // registers object, typelib and all interfaces in typelib
  return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Removes entries from the system registry
STDAPI DllUnregisterServer(void) { return _Module.UnregisterServer(TRUE); }
