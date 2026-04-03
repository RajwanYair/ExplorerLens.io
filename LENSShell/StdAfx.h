#ifndef _STDAFX_C5E3BAAB_1D97_4C7F_AB90_2D4397D95F13_
#define _STDAFX_C5E3BAAB_1D97_4C7F_AB90_2D4397D95F13_
#pragma once

#ifndef UNICODE
    #define UNICODE  // UNICODE-only project
#endif

#define STRICT

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#include <atlapp.h>  // WTL project
#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>
#include <shlobj.h>

#include <wrl/client.h>

// Core infrastructure includes
#include "enhanced_cache.h"
#include "error_logger.h"
#include "memory_utils.h"
#include "performance_profiler.h"

#endif  //_STDAFX_C5E3BAAB_1D97_4C7F_AB90_2D4397D95F13_
