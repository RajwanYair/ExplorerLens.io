//==============================================================================
// DarkThumbs Engine - Build Configuration
// Copyright (c) 2026 - DarkThumbs Project  
// Task A23: Compile-time optimization and debug control
//==============================================================================

#pragma once

namespace DarkThumbs {
namespace Engine {

    // Build configuration detection
    #if defined(_DEBUG) || defined(DEBUG)
        #define DT_DEBUG_BUILD 1
        #define DT_RELEASE_BUILD 0
    #else
        #define DT_DEBUG_BUILD 0
        #define DT_RELEASE_BUILD 1
    #endif

    // Profile build detection
    #if defined(NDEBUG) && defined(ENABLE_PROFILING)
        #define DT_PROFILE_BUILD 1
    #else
        #define DT_PROFILE_BUILD 0
    #endif

    // Debug logging control (set to 0 in release for performance)
    #if DT_DEBUG_BUILD
        #define DT_ENABLE_DEBUG_LOGS 1
        #define DT_ENABLE_TRACE_LOGS 1
        #define DT_ENABLE_ASSERTS 1
    #else
        #define DT_ENABLE_DEBUG_LOGS 0
        #define DT_ENABLE_TRACE_LOGS 0
        #define DT_ENABLE_ASSERTS 0
    #endif

    // Performance settings
    #if DT_RELEASE_BUILD
        #define DT_AGGRESSIVE_INLINE __forceinline
        #define DT_NO_INLINE __declspec(noinline)
        #define DT_LIKELY(x) (x)
        #define DT_UNLIKELY(x) (x)
    #else
        #define DT_AGGRESSIVE_INLINE inline
        #define DT_NO_INLINE
        #define DT_LIKELY(x) (x)
        #define DT_UNLIKELY(x) (x)
    #endif

    // Debug-only code
    #if DT_DEBUG_BUILD
        #define DT_DEBUG_ONLY(x) x
    #else
        #define DT_DEBUG_ONLY(x)
    #endif

    // Assertions
    #if DT_ENABLE_ASSERTS
        #include <cassert>
        #define DT_ASSERT(expr) assert(expr)
        #define DT_ASSERT_MSG(expr, msg) assert((expr) && (msg))
    #else
        #define DT_ASSERT(expr) ((void)0)
        #define DT_ASSERT_MSG(expr, msg) ((void)0)
    #endif

    // Conditional logging macros
    #if DT_ENABLE_DEBUG_LOGS
        #define DT_DEBUG_LOG(msg) OutputDebugStringW(msg)
    #else
        #define DT_DEBUG_LOG(msg) ((void)0)
    #endif

    #if DT_ENABLE_TRACE_LOGS
        #define DT_TRACE_LOG(msg) OutputDebugStringW(msg)
    #else
        #define DT_TRACE_LOG(msg) ((void)0)
    #endif

    // Compile-time constants for optimization
    namespace BuildConfig {
        constexpr bool IsDebugBuild = DT_DEBUG_BUILD;
        constexpr bool IsReleaseBuild = DT_RELEASE_BUILD;
        constexpr bool EnableDebugLogs = DT_ENABLE_DEBUG_LOGS;
        constexpr bool EnableTraceLogs = DT_ENABLE_TRACE_LOGS;
        constexpr bool EnableAsserts = DT_ENABLE_ASSERTS;
        
        // Version info
        constexpr int VersionMajor = 10;
        constexpr int VersionMinor = 5;
        constexpr int VersionPatch = 0;
        constexpr const wchar_t* VersionString = L"10.5.0";
        constexpr int SprintCount = 248;
        constexpr int TestCount = 687;
        constexpr const wchar_t* BuildDate = __TIMESTAMP__;
    }

} // namespace Engine
} // namespace DarkThumbs
