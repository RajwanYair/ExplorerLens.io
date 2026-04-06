#ifndef _STDAFX_2AA16305_D8E3_4296_9A26_5407C9BF9DEC_
#define _STDAFX_2AA16305_D8E3_4296_9A26_5407C9BF9DEC_
#pragma once

#ifndef UNICODE
    #define UNICODE  // UNICODE-only project
#endif

//runs on min Windows 10 (v15.0.0 Zenith — modern Windows targeting)
#ifndef WINVER
    #define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0A00
#endif

//#define _WTL_SUPPORT_SDK_ATL3
#pragma comment(linker, "/NODEFAULTLIB:atlthunk.lib")

#include <atlbase.h>
#include <atlstdthunk.h>

namespace ATL {
inline void* __stdcall __AllocStdCallThunk()
{
    return ::VirtualAlloc(0, sizeof(_stdcallthunk), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

inline void __stdcall __FreeStdCallThunk(void* p)
{
    if (p != NULL)
        ::VirtualFree(p, 0, MEM_RELEASE);
}
};  // namespace ATL

#include "atlapp.h"

extern CAppModule _Module;

#include <Windowsx.h>
// Undefine macros from Windowsx.h that conflict with WTL method names under MSVC v145
#ifdef SubclassWindow
#    undef SubclassWindow
#endif
#ifdef SelectFont
#    undef SelectFont
#endif
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlframe.h>
#include <atlstr.h>
#include <atlwin.h>

using namespace ATL;

// Common Controls v6 dependency is declared in LENSManager.manifest — do not
// duplicate with #pragma comment(linker, "/manifestdependency:...") here as
// that would generate a second manifest snippet with a conflicting execution
// level and cause mt.exe to fail with c1010001.

#endif  //_STDAFX_2AA16305_D8E3_4296_9A26_5407C9BF9DEC_
