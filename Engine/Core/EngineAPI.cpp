//==============================================================================
// ExplorerLens Engine - Public API Implementation
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "EngineAPI.h"

namespace ExplorerLens {
namespace Engine {

ENGINE_API const wchar_t* ENGINE_CALL GetEngineVersion() {
    return L"6.2.0";
}

ENGINE_API const wchar_t* ENGINE_CALL GetEngineBuildDate() {
    return L"2026-02-15";  // Version 6.2.0 build
}

} // namespace Engine
} // namespace ExplorerLens

