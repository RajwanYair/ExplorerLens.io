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

ENGINE_API const wchar_t* ENGINE_CALL GetEngineBuildDate() {
    return L__DATE__ L" " L__TIME__;
}

} // namespace Engine
} // namespace DarkThumbs
