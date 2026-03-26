// PublicSDKSurface.h — Public Plugin SDK API Surface v2.0 (ExplorerLens 17.0)
// Copyright (c) 2026 ExplorerLens Project
//
// Stable public API surface document for the ExplorerLens Plugin SDK v2.0.
// This header documents which interfaces are part of the frozen public ABI
// and which are considered internal (subject to change without notice).
//
// API stability guarantees (SDK v2.0 / ExplorerLens 17.0+):
//   - All interfaces in the IExplorerLens* namespace are public + stable
//   - Engine/Plugin/plugin_api.h C ABI is frozen at version 2
//   - All other headers (Engine/Core/, Engine/AI/, etc.) are internal
//
// Plugin developers MUST only include:
//   - SDK/plugin_api.h  (C ABI — most stable)
//   - SDK/PluginSDK.h   (C++ wrapper over C ABI)
//   - SDK/ThumbnailSDK.h (thumbnail output sink helpers)
//
#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// PUBLIC SDK INTERFACE INVENTORY (v2.0 — Frozen with ExplorerLens 17.0.0 Nova)
// ─────────────────────────────────────────────────────────────────────────────

// SDK/plugin_api.h  —  C ABI (stable v1 → v2)
//   LENS_PluginInfo        struct  version 0x0200
//   LENS_FormatInfo        struct
//   LENS_DecodeRequest     struct
//   LENS_DecodeResult      struct
//   LENS_PluginEntryFn     function pointer typedef
//   LENS_GetPluginInfo()   exported function
//   LENS_Decode()          exported function
//   LENS_Destroy()         exported function
//   LENS_SDK_VERSION       macro  (0x0200 for v2)

// SDK/PluginSDK.h  —  C++ helper wrappers (header-only)
//   ExplorerLens::SDK::PluginBase   CRTP base class for plugins
//   ExplorerLens::SDK::FormatList   FormatInfo builder helper
//   ExplorerLens::SDK::OutputSink   BGRA + JPEG output helper

// SDK/ThumbnailSDK.h  —  Thumbnail output utilities
//   ExplorerLens::SDK::ThumbnailOutput  decode result builder
//   ExplorerLens::SDK::ResizeHelper     GPU-assisted resize guidance

// ─────────────────────────────────────────────────────────────────────────────
// INTERNAL HEADERS — DO NOT USE IN PLUGINS
// ─────────────────────────────────────────────────────────────────────────────
// Engine/Core/*          — Internal pipeline, not ABI-stable
// Engine/AI/*            — AI/upscaling internals
// Engine/Cache/*         — Cache implementation details
// Engine/Decoders/*      — Format-specific decoder internals
// Engine/GPU/*           — GPU dispatch internals
// Engine/Memory/*        — Memory pressure system internals
// Engine/Pipeline/*      — Pipeline stage internals

// ─────────────────────────────────────────────────────────────────────────────
// SDK v2.0 CHANGES FROM v1.0
// ─────────────────────────────────────────────────────────────────────────────
// Breaking changes (require plugin recompile):
//   - LENS_FormatInfo.flags field added (uint32_t, must be zero for v1 compat)
//   - LENS_DecodeRequest.upscaleHint field added (enum, must be zero for v1 compat)

// New optional interfaces (not required for basic plugins):
//   - LENS_GetPluginInfoV2()  Extended plugin info with telemetry tag + a11y description
//   - LENS_Capabilities()     Capability flags (LENS_CAP_ANIMATED, LENS_CAP_HDR, ...)
//   - LENS_GetMetadata()      Rich metadata extraction beyond thumbnail pixels

// ─────────────────────────────────────────────────────────────────────────────
// FORWARD DECLARATIONS (for IDE / static analysis tools)
// ─────────────────────────────────────────────────────────────────────────────
extern "C" {
    struct LENS_PluginInfo;
    struct LENS_FormatInfo;
    struct LENS_DecodeRequest;
    struct LENS_DecodeResult;

    void   LENS_GetPluginInfo(LENS_PluginInfo* pInfo);
    int    LENS_Decode(const LENS_DecodeRequest* pReq, LENS_DecodeResult* pResult);
    void   LENS_Destroy(LENS_DecodeResult* pResult);
}
