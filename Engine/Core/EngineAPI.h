//==============================================================================
// ExplorerLens Engine - Public API Export Definitions
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once

// DLL export/import macros for Windows
#ifdef _WIN32
    #ifdef ENGINE_STATIC
        // Static library: no dllexport/dllimport
        #define ENGINE_API
    #elif defined(ENGINE_EXPORTS)
        // Building DLL: export symbols
        #define ENGINE_API __declspec(dllexport)
    #else
        // Using DLL: import symbols
        #define ENGINE_API __declspec(dllimport)
    #endif
#else
    #define ENGINE_API
#endif

// Calling convention for Engine API functions
#ifdef _WIN32
    #define ENGINE_CALL __stdcall
#else
    #define ENGINE_CALL
#endif

// Version information
#define ENGINE_VERSION_MAJOR 15
#define ENGINE_VERSION_MINOR 0
#define ENGINE_VERSION_PATCH 0

namespace ExplorerLens {
namespace Engine {

/// Get engine version as wide string
ENGINE_API const wchar_t* ENGINE_CALL GetEngineVersion();

/// Get engine build date as wide string
ENGINE_API const wchar_t* ENGINE_CALL GetEngineBuildDate();

}  // namespace Engine
}  // namespace ExplorerLens
