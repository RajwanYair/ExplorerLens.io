//==============================================================================
// ExplorerLens Engine - Public API Implementation
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "EngineAPI.h"

namespace ExplorerLens {
namespace Engine {

ENGINE_API const wchar_t* ENGINE_CALL GetEngineVersion() {
 return L"15.0.0";
}

ENGINE_API const wchar_t* ENGINE_CALL GetEngineBuildDate() {
 return L"2026-07-16"; // Version 15.0.0 Zenith
}

} // namespace Engine
} // namespace ExplorerLens

