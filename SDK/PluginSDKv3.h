// PluginSDKv3.h — Universal Plugin SDK v3
// Copyright (c) 2026 ExplorerLens Project
//
// Unified C ABI header for all ExplorerLens plugin types (SDK v3, binary compatible with v1/v2).
//
#pragma once
#include <cstdint>

// C ABI — no namespace, no C++ features below this line
#ifdef __cplusplus
extern "C" {
#endif

#define LENS_PLUGIN_SDK_VERSION 3

typedef struct LensPluginInfo {
    uint32_t    sdkVersion;
    uint32_t    pluginVersion;
    const char* name;
    const char* author;
} LensPluginInfo;

typedef int (*LensThumbnailFn)(const wchar_t* path, void* outBuf, uint32_t* outW, uint32_t* outH);

typedef struct LensPluginVTable {
    void          (*GetInfo)(LensPluginInfo* out);
    LensThumbnailFn GenerateThumbnail;
    void          (*Shutdown)(void);
} LensPluginVTable;

#ifdef __cplusplus
} // extern "C"
#endif

// C++ wrapper helpers (not exported)
#ifdef __cplusplus
#include <string>
namespace ExplorerLens { namespace Engine {

struct PluginSDKv3Info {
    uint32_t    sdkVersion  = LENS_PLUGIN_SDK_VERSION;
    std::string name;
    std::string author;
};

}} // namespace ExplorerLens::Engine
#endif
