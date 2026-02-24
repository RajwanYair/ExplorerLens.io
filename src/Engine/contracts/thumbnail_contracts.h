#pragma once

#include <cstdint>
#include <string>
#include <windows.h>

namespace ExplorerLens::Engine::Contracts {

    // Bit flags for ThumbnailRequest::flags
    enum RequestFlags : uint32_t {
        RequestFlags_None = 0,
        RequestFlags_AllowGpu = 1 << 0,
        RequestFlags_AllowNetwork = 1 << 1,
        RequestFlags_AllowPlugins = 1 << 2,
        RequestFlags_ForceRefresh = 1 << 3,
        RequestFlags_PreferEmbedded = 1 << 4
    };

    // Bit masks for ThumbnailResult::stageMask
    enum StageMask : uint32_t {
        StageMask_None = 0,
        StageMask_Detection = 1 << 0,
        StageMask_CacheLookup = 1 << 1,
        StageMask_Decode = 1 << 2,
        StageMask_Transform = 1 << 3,
        StageMask_Render = 1 << 4,
        StageMask_CacheStore = 1 << 5
    };

    struct ThumbnailRequest {
        uint32_t contractVersion;   // bump only on breaking changes
        uint64_t correlationId;     // trace across logs/ETW/processes
        std::wstring path;
        uint32_t sizePx;
        uint32_t flags;             // e.g., allowGpu, allowNetwork, allowPlugins
    };

    struct ThumbnailResult {
        HRESULT hr;
        uint32_t width;
        uint32_t height;
        uint32_t pixelFormat;       // canonical format
        uint64_t elapsedUs;
        uint32_t stageMask;         // which stages executed
    };

    struct CacheKeyV2 {
        uint32_t version;
        uint64_t fileIdHash;
        uint64_t contentHash;       // can be partial for perf
        uint32_t sizePx;
        uint32_t rendererFlags;
        uint32_t pipelineVersion;
    };

}

