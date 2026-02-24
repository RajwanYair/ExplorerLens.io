/**
 * ExplorerLensPlugin.h - Official ExplorerLens Plugin SDK
 * Version: 1.0.0
 * ABI Version: 1
 * 
 * Copyright (c) 2026 ExplorerLens Project
 * Licensed under MIT License
 * 
 * This is the stable ABI for ExplorerLens thumbnail decoder plugins.
 * Plugin developers should target a specific ABI version for compatibility.
 */

#pragma once

#include <stdint.h>
#include <wtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Current ABI version - plugins must match this for binary compatibility
 * Only incremented on breaking changes to the plugin interface.
 */
#define DT_PLUGIN_ABI_VERSION 1

/**
 * Engine version compatibility - plugins can specify min/max engine versions
 * Format: (major << 24) | (minor << 16) | (patch << 8) | build
 */
#define DT_ENGINE_VERSION_5_4_0  0x05040000
#define DT_ENGINE_VERSION_CURRENT DT_ENGINE_VERSION_5_4_0

/**
 * Plugin export macro - use for all plugin entry points
 */
#ifdef _WIN32
  #define DT_PLUGIN_API extern "C" __declspec(dllexport)
#else
  #define DT_PLUGIN_API extern "C" __attribute__((visibility("default")))
#endif

/**
 * Plugin Capability Flags
 * These define what resources/permissions a plugin requires.
 * The engine will enforce these at load time and runtime.
 */
typedef enum DT_Capability : uint32_t {
    DT_CAP_NONE         = 0,
    DT_CAP_READ_FILE    = 1 << 0,  ///< Can read local files directly
    DT_CAP_NETWORK      = 1 << 1,  ///< Can make network requests
    DT_CAP_GPU          = 1 << 2,  ///< Can use GPU acceleration
    DT_CAP_DECODE       = 1 << 3,  ///< Primary decoder capability
    DT_CAP_TRANSFORM    = 1 << 4,  ///< Can transform/filter images
    DT_CAP_METADATA     = 1 << 5,  ///< Can extract metadata
    DT_CAP_ARCHIVE      = 1 << 6,  ///< Can extract from archives
    DT_CAP_STREAMING    = 1 << 7,  ///< Supports streaming/progressive decode
    DT_CAP_MULTITHREADED = 1 << 8, ///< Uses multiple threads internally
    DT_CAP_EXTERNAL_PROCESS = 1 << 9 ///< Spawns external processes
} DT_Capability;

/**
 * Plugin Status Codes
 */
typedef enum DT_Status : int32_t {
    DT_SUCCESS                  = 0,
    DT_ERROR_GENERIC            = -1,
    DT_ERROR_NOT_SUPPORTED      = -2,
    DT_ERROR_INVALID_ARGUMENT   = -3,
    DT_ERROR_OUT_OF_MEMORY      = -4,
    DT_ERROR_FILE_NOT_FOUND     = -5,
    DT_ERROR_CORRUPT_DATA       = -6,
    DT_ERROR_TIMEOUT            = -7,
    DT_ERROR_ABI_MISMATCH       = -8,
    DT_ERROR_NOT_INITIALIZED    = -9,
    DT_ERROR_ALREADY_INITIALIZED = -10,
    DT_ERROR_CAPABILITY_DENIED  = -11
} DT_Status;

/**
 * Plugin Information Structure
 * This is returned by DT_GetPluginInfo() and must remain valid for the lifetime of the plugin.
 */
typedef struct DT_PluginInfo {
    uint32_t abiVersion;           ///< Must be DT_PLUGIN_ABI_VERSION
    const wchar_t* id;             ///< Unique stable ID (e.g., "explorerlens.plugin.psd")
    const wchar_t* name;           ///< Display name (e.g., "Photoshop Document Decoder")
    const wchar_t* version;        ///< Plugin version (semantic versioning: "1.2.3")
    const wchar_t* vendor;         ///< Vendor/author name
    const wchar_t* description;    ///< Brief description
    uint32_t capabilities;         ///< Bitwise OR of DT_Capability flags
    uint32_t minEngineVersion;     ///< Minimum compatible engine version
    uint32_t maxEngineVersion;     ///< Maximum compatible engine version (0 = no limit)
} DT_PluginInfo;

/**
 * Format Support Structure
 * Describes which file formats this plugin can handle.
 */
typedef struct DT_FormatInfo {
    const wchar_t* extension;      ///< File extension (e.g., L".psd")
    const wchar_t* mimeType;       ///< MIME type (e.g., L"image/vnd.adobe.photoshop")
    const wchar_t* description;    ///< Format description
    uint32_t priority;             ///< Priority when multiple plugins support same format (higher = preferred)
} DT_FormatInfo;

/**
 * Thumbnail Request Structure
 * Passed to DT_GenerateThumbnail() with all request details.
 */
typedef struct DT_ThumbnailRequest {
    uint32_t structSize;           ///< Size of this structure (for versioning)
    uint64_t correlationId;        ///< Correlation ID for tracing/logging
    const wchar_t* filePath;       ///< Full path to the file
    IStream* stream;               ///< IStream to the file data (may be NULL if filePath provided)
    uint32_t sizePx;               ///< Requested thumbnail size in pixels (width or height, whichever is larger)
    uint32_t flags;                ///< Request flags (reserved for future use)
    uint32_t timeoutMs;            ///< Timeout in milliseconds (0 = no timeout)
} DT_ThumbnailRequest;

/**
 * Thumbnail Result Structure
 * Returned by DT_GenerateThumbnail() with the generated thumbnail and metadata.
 */
typedef struct DT_ThumbnailResult {
    uint32_t structSize;           ///< Size of this structure (for versioning)
    HBITMAP hBitmap;               ///< Generated thumbnail (caller takes ownership)
    uint32_t width;                ///< Actual thumbnail width
    uint32_t height;               ///< Actual thumbnail height
    uint32_t pixelFormat;          ///< Pixel format (DXGI_FORMAT_* value)
    uint64_t elapsedUs;            ///< Elapsed time in microseconds
    uint32_t stageMask;            ///< Bitfield of pipeline stages executed
} DT_ThumbnailResult;

/**
 * Plugin Statistics Structure
 * Returned by DT_GetStatistics() to report plugin performance metrics.
 */
typedef struct DT_PluginStatistics {
    uint32_t structSize;           ///< Size of this structure
    uint64_t totalRequests;        ///< Total thumbnail requests processed
    uint64_t successfulRequests;   ///< Successful requests
    uint64_t failedRequests;       ///< Failed requests
    uint64_t totalElapsedUs;       ///< Total processing time (microseconds)
    uint64_t peakMemoryBytes;      ///< Peak memory usage
    uint64_t currentMemoryBytes;   ///< Current memory usage
} DT_PluginStatistics;

/**
 * Required Plugin Entry Points
 * All plugins must export these functions.
 */

/**
 * Get plugin information
 * Called once during plugin discovery/loading.
 * 
 * @return Pointer to DT_PluginInfo structure (must remain valid for plugin lifetime)
 */
DT_PLUGIN_API const DT_PluginInfo* DT_GetPluginInfo(void);

/**
 * Get supported formats
 * Called during plugin loading to determine which formats this plugin handles.
 * 
 * @param outCount [out] Receives the number of formats returned
 * @return Array of DT_FormatInfo structures (must remain valid for plugin lifetime)
 */
DT_PLUGIN_API const DT_FormatInfo* DT_GetSupportedFormats(uint32_t* outCount);

/**
 * Initialize the plugin
 * Called once after plugin is loaded and capabilities are verified.
 * Plugin should allocate resources and perform one-time initialization.
 * 
 * @return DT_SUCCESS on success, error code on failure
 */
DT_PLUGIN_API DT_Status DT_Initialize(void);

/**
 * Shutdown the plugin
 * Called once before plugin is unloaded.
 * Plugin should free all resources and perform cleanup.
 * 
 * @return DT_SUCCESS on success, error code on failure
 */
DT_PLUGIN_API DT_Status DT_Shutdown(void);

/**
 * Generate thumbnail for a file
 * This is the primary function that generates thumbnails.
 * 
 * @param request [in] Thumbnail request details
 * @param result [out] Receives the generated thumbnail and metadata
 * @return DT_SUCCESS on success, error code on failure
 */
DT_PLUGIN_API DT_Status DT_GenerateThumbnail(
    const DT_ThumbnailRequest* request,
    DT_ThumbnailResult* result
);

/**
 * Optional Plugin Entry Points
 * Plugins may optionally export these functions for enhanced functionality.
 */

/**
 * Can handle file?
 * Optional fast check to determine if plugin can handle a file without full decode.
 * 
 * @param filePath Full path to the file
 * @param stream IStream to the file data (may be NULL)
 * @param outConfidence [out] Confidence level (0-100, 100 = definitely can handle)
 * @return DT_SUCCESS if plugin can handle, DT_ERROR_NOT_SUPPORTED otherwise
 */
DT_PLUGIN_API DT_Status DT_CanHandle(
    const wchar_t* filePath,
    IStream* stream,
    uint32_t* outConfidence
);

/**
 * Get plugin statistics
 * Optional function to report performance metrics.
 * 
 * @param stats [out] Receives plugin statistics
 * @return DT_SUCCESS on success, error code on failure
 */
DT_PLUGIN_API DT_Status DT_GetStatistics(DT_PluginStatistics* stats);

/**
 * Get plugin configuration schema
 * Optional function to expose plugin-specific settings.
 * 
 * @param outSchema [out] Receives JSON schema describing configuration options
 * @return DT_SUCCESS on success, error code on failure
 */
DT_PLUGIN_API DT_Status DT_GetConfigSchema(const wchar_t** outSchema);

/**
 * Set plugin configuration
 * Optional function to apply plugin-specific settings.
 * 
 * @param configJson JSON string with configuration values
 * @return DT_SUCCESS on success, error code on failure
 */
DT_PLUGIN_API DT_Status DT_SetConfig(const wchar_t* configJson);

#ifdef __cplusplus
}
#endif

/**
 * C++ Helper Macros
 * For C++ plugins, these macros simplify plugin implementation.
 */
#ifdef __cplusplus

/**
 * Use this macro to begin plugin implementation in C++
 * 
 * Example:
 * DT_BEGIN_PLUGIN_IMPL(PSDPlugin)
 *   // Implement virtual methods
 * DT_END_PLUGIN_IMPL()
 */
#define DT_BEGIN_PLUGIN_IMPL(ClassName) \
    namespace { \
    class ClassName { \
    public: \
        static const DT_PluginInfo* GetInfo(); \
        static const DT_FormatInfo* GetFormats(uint32_t* count); \
        static DT_Status Initialize(); \
        static DT_Status Shutdown(); \
        static DT_Status GenerateThumbnail(const DT_ThumbnailRequest* req, DT_ThumbnailResult* result); \
    }; \
    } \
    DT_PLUGIN_API const DT_PluginInfo* DT_GetPluginInfo(void) { return ClassName::GetInfo(); } \
    DT_PLUGIN_API const DT_FormatInfo* DT_GetSupportedFormats(uint32_t* c) { return ClassName::GetFormats(c); } \
    DT_PLUGIN_API DT_Status DT_Initialize(void) { return ClassName::Initialize(); } \
    DT_PLUGIN_API DT_Status DT_Shutdown(void) { return ClassName::Shutdown(); } \
    DT_PLUGIN_API DT_Status DT_GenerateThumbnail(const DT_ThumbnailRequest* r, DT_ThumbnailResult* res) { \
        return ClassName::GenerateThumbnail(r, res); \
    }

#define DT_END_PLUGIN_IMPL()

#endif // __cplusplus

/*
 * VERSION HISTORY
 * 
 * ABI Version 1 (v6.0.0) - Initial release
 *   - Core plugin interface
 *   - Capability system
 *   - Format registration
 *   - Basic thumbnail generation
 */

