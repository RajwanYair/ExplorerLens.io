#ifndef _STDAFX_C5E3BAAB_1D97_4C7F_AB90_2D4397D95F13_
#define _STDAFX_C5E3BAAB_1D97_4C7F_AB90_2D4397D95F13_
#pragma once

#ifndef UNICODE
#define UNICODE// UNICODE-only project
#endif

#define STRICT

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
#include <atlapp.h>// WTL project

extern CComModule _Module;

#include <atlcom.h>
#include <shlobj.h>
#include <wrl/client.h>

// Phase 10 Infrastructure (v5.2.0)
#include "error_logger.h"
#include "performance_profiler.h"
#include "memory_utils.h"
#include "enhanced_cache.h"

#endif//_STDAFX_C5E3BAAB_1D97_4C7F_AB90_2D4397D95F13_
