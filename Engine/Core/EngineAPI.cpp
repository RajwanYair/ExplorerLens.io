//==============================================================================
// DarkThumbs Engine - Public API Implementation
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include "EngineAPI.h"

namespace DarkThumbs {
namespace Engine {

ENGINE_API const wchar_t* ENGINE_CALL GetEngineVersion() {
    return L"6.2.0";
}

#define _WIDE2(x) L##x
#define WIDE(x) _WIDE2(x)

ENGINE_API const wchar_t* ENGINE_CALL GetEngineBuildDate() {
    return WIDE(__DATE__) L" " WIDE(__TIME__);
}

} // namespace Engine
} // namespace DarkThumbs
