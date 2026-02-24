/******************************************************************************
 * ExplorerLens Plugin API v1.0
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * This is the stable C ABI for ExplorerLens thumbnail decoder plugins.
 * Plugins written against this API will remain binary-compatible across
 * ExplorerLens minor versions.
 * 
 * License: MIT
 *****************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

//============================================================================
// API Version and Compatibility
//============================================================================

#define EXPLORERLENS_PLUGIN_API_VERSION_MAJOR 1
#define EXPLORERLENS_PLUGIN_API_VERSION_MINOR 0
#define EXPLORERLENS_PLUGIN_API_VERSION_PATCH 0

// Plugin API version (major.minor format for ABI compatibility checks)
#define EXPLORERLENS_PLUGIN_API_VERSION \
    ((EXPLORERLENS_PLUGIN_API_VERSION_MAJOR << 16) | EXPLORERLENS_PLUGIN_API_VERSION_MINOR)

//============================================================================
// Platform Export Macros
//============================================================================

#ifdef _WIN32
    #ifdef EXPLORERLENS_PLUGIN_EXPORTS
        #define PLUGIN_API __declspec(dllexport)
    #else
        #define PLUGIN_API __declspec(dllimport)
    #endif
    #define PLUGIN_CALL __stdcall
#else
    #define PLUGIN_API __attribute__((visibility("default")))
    #define PLUGIN_CALL
#endif

//============================================================================
// Error Codes
//============================================================================

typedef enum {
    PLUGIN_SUCCESS = 0,
    PLUGIN_ERROR_INVALID_PARAMETER = -1,
    PLUGIN_ERROR_UNSUPPORTED_FORMAT = -2,
    PLUGIN_ERROR_OUT_OF_MEMORY = -3,
    PLUGIN_ERROR_FILE_NOT_FOUND = -4,
    PLUGIN_ERROR_READ_ERROR = -5,
    PLUGIN_ERROR_DECODE_ERROR = -6,
    PLUGIN_ERROR_UNSUPPORTED_SIZE = -7,
    PLUGIN_ERROR_CORRUPTED_DATA = -8,
    PLUGIN_ERROR_NOT_INITIALIZED = -9,
    PLUGIN_ERROR_ALREADY_INITIALIZED = -10,
    PLUGIN_ERROR_UNKNOWN = -999
} PluginErrorCode;

//============================================================================
// Plugin Capability Flags
//============================================================================

typedef enum {
    PLUGIN_CAP_NONE = 0,
    PLUGIN_CAP_STILL_IMAGE = (1 << 0),      // Can decode still images
    PLUGIN_CAP_ANIMATION = (1 << 1),        // Can decode animated images
    PLUGIN_CAP_MULTIPAGE = (1 << 2),        // Can handle multi-page files
    PLUGIN_CAP_PROGRESSIVE = (1 << 3),      // Supports progressive decoding
    PLUGIN_CAP_METADATA = (1 << 4),         // Can extract EXIF/metadata
    PLUGIN_CAP_THUMBNAIL = (1 << 5),        // Has embedded thumbnail support
    PLUGIN_CAP_GPU_DECODE = (1 << 6),       // GPU-accelerated decoding
    PLUGIN_CAP_INCREMENTAL = (1 << 7),      // Supports incremental loading
    PLUGIN_CAP_VECTOR = (1 << 8),           // Vector graphics (SVG, etc.)
    PLUGIN_CAP_HDR = (1 << 9),              // High Dynamic Range support
    PLUGIN_CAP_WIDE_GAMUT = (1 << 10),      // Wide color gamut support
    PLUGIN_CAP_ALPHA = (1 << 11),           // Alpha channel support
    PLUGIN_CAP_ARCHIVE = (1 << 12),         // Archive format support
} PluginCapabilityFlags;

//============================================================================
// Pixel Format
//============================================================================

typedef enum {
    PIXEL_FORMAT_UNKNOWN = 0,
    PIXEL_FORMAT_BGRA32,        // 32-bit BGRA (Windows native)
    PIXEL_FORMAT_RGBA32,        // 32-bit RGBA
    PIXEL_FORMAT_BGR24,         // 24-bit BGR
    PIXEL_FORMAT_RGB24,         // 24-bit RGB
    PIXEL_FORMAT_GRAY8,         // 8-bit grayscale
    PIXEL_FORMAT_GRAY16,        // 16-bit grayscale
    PIXEL_FORMAT_RGB48,         // 48-bit RGB (16-bit per channel)
    PIXEL_FORMAT_RGBA64,        // 64-bit RGBA (16-bit per channel)
} PixelFormat;

//============================================================================
// Plugin Information Structure
//============================================================================

typedef struct {
    // Plugin identification
    const char* plugin_name;            // e.g., "WebP Decoder"
    const char* plugin_version;         // e.g., "1.0.0"
    const char* plugin_author;          // Plugin developer name
    const char* plugin_description;     // Brief description
    const char* plugin_license;         // License type (e.g., "MIT", "GPL")
    
    // API compatibility
    uint32_t api_version;               // EXPLORERLENS_PLUGIN_API_VERSION
    
    // Supported formats (null-terminated array)
    const char** supported_extensions;  // e.g., {".webp", ".webm", NULL}
    const char** mime_types;            // e.g., {"image/webp", NULL}
    
    // Capabilities
    uint32_t capabilities;              // Bitfield of PluginCapabilityFlags
    
    // Performance hints
    uint32_t max_threads;               // 0 = unlimited, >0 = thread count
    bool requires_gpu;                  // True if GPU is required
    bool supports_background_loading;   // Can decode in background thread
    
    // Reserved for future use
    void* reserved[4];
} PluginInfo;

//============================================================================
// Image Metadata
//============================================================================

typedef struct {
    uint32_t width;                 // Image width in pixels
    uint32_t height;                // Image height in pixels
    uint32_t bit_depth;             // Bits per pixel
    PixelFormat pixel_format;       // Pixel format
    
    bool has_alpha;                 // Has transparency
    bool is_animated;               // Is animated image
    bool has_thumbnail;             // Has embedded thumbnail
    
    uint32_t frame_count;           // Number of frames (1 for still)
    uint32_t loop_count;            // Animation loop count (0 = infinite)
    
    // Optional metadata
    const char* color_space;        // e.g., "sRGB", "AdobeRGB"
    const char* icc_profile;        // ICC profile name
    
    // Reserved for future use
    void* reserved[8];
} ImageMetadata;

//============================================================================
// Decode Request
//============================================================================

typedef struct {
    // Input
    const char* file_path;          // Full path to file (UTF-8)
    const uint8_t* data;            // Or raw data buffer (if file_path is NULL)
    size_t data_size;               // Size of data buffer
    
    // Output requirements
    uint32_t target_width;          // Desired thumbnail width
    uint32_t target_height;         // Desired thumbnail height
    PixelFormat output_format;      // Desired pixel format (BGRA32 preferred)
    
    // Options
    bool preserve_aspect_ratio;     // Maintain aspect ratio
    bool high_quality;              // Use high-quality scaling
    uint32_t frame_index;           // Frame to decode (0-based, for animations)
    
    // Context for callbacks
    void* user_data;                // User-provided context pointer
    
    // Reserved for future use
    void* reserved[4];
} DecodeRequest;

//============================================================================
// Decode Result
//============================================================================

typedef struct {
    // Output buffer (allocated by plugin, freed by host with plugin_free_result)
    uint8_t* pixels;                // Pixel data buffer
    size_t buffer_size;             // Total buffer size in bytes
    uint32_t stride;                // Row stride in bytes
    
    // Output dimensions
    uint32_t width;                 // Actual output width
    uint32_t height;                // Actual output height
    PixelFormat pixel_format;       // Actual pixel format
    
    // Metadata (optional)
    ImageMetadata* metadata;        // Decoded metadata (or NULL)
    
    // Error information
    PluginErrorCode error_code;     // Error code (PLUGIN_SUCCESS if ok)
    const char* error_message;      // Human-readable error (or NULL)
    
    // Reserved for future use
    void* reserved[4];
} DecodeResult;

//============================================================================
// Memory Allocation Callbacks (provided by host)
//============================================================================

typedef void* (*PluginAllocFunc)(size_t size, void* user_data);
typedef void (*PluginFreeFunc)(void* ptr, void* user_data);

typedef struct {
    PluginAllocFunc alloc;          // Allocate memory
    PluginFreeFunc free;            // Free memory
    void* user_data;                // User data for callbacks
} PluginAllocator;

//============================================================================
// Progress Callback (for long operations)
//============================================================================

typedef void (*PluginProgressCallback)(float progress, void* user_data);

//============================================================================
// Required Plugin Export Functions
//============================================================================

// Get plugin information (called once at load time)
PLUGIN_API const PluginInfo* PLUGIN_CALL plugin_get_info(void);

// Initialize plugin (called once after loading)
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_init(const PluginAllocator* allocator);

// Cleanup and shutdown plugin (called once before unloading)
PLUGIN_API void PLUGIN_CALL plugin_cleanup(void);

// Check if plugin can decode this file
PLUGIN_API bool PLUGIN_CALL plugin_can_decode(const char* file_path, 
                                                const uint8_t* data, 
                                                size_t data_size);

// Decode image to thumbnail
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_decode(const DecodeRequest* request,
                                                       DecodeResult* result,
                                                       PluginProgressCallback progress);

// Free decode result (allocated by plugin)
PLUGIN_API void PLUGIN_CALL plugin_free_result(DecodeResult* result);

//============================================================================
// Optional Plugin Export Functions
//============================================================================

// Get image metadata without full decode (optional, faster)
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_get_metadata(const char* file_path,
                                                             const uint8_t* data,
                                                             size_t data_size,
                                                             ImageMetadata* metadata);

// Decode specific frame from animation (optional)
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_decode_frame(const DecodeRequest* request,
                                                             uint32_t frame_index,
                                                             DecodeResult* result);

// Get embedded thumbnail (optional, faster than full decode)
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_get_thumbnail(const char* file_path,
                                                              const uint8_t* data,
                                                              size_t data_size,
                                                              DecodeResult* result);

//============================================================================
// Utility Functions (provided by host for plugins to use)
//============================================================================

// Helper to convert error codes to strings
PLUGIN_API const char* PLUGIN_CALL plugin_error_to_string(PluginErrorCode error);

#ifdef __cplusplus
}
#endif

//============================================================================
// C++ Wrapper (optional, for C++ plugin developers)
//============================================================================

#ifdef __cplusplus
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {

class IPluginDecoder {
public:
    virtual ~IPluginDecoder() = default;
    
    // Plugin information
    virtual const PluginInfo& GetInfo() const = 0;
    
    // Initialization
    virtual PluginErrorCode Initialize(const PluginAllocator* allocator) = 0;
    virtual void Cleanup() = 0;
    
    // Decoding
    virtual bool CanDecode(const std::string& path, 
                          const std::vector<uint8_t>& data) const = 0;
    
    virtual PluginErrorCode Decode(const DecodeRequest& request,
                                   DecodeResult& result,
                                   PluginProgressCallback progress = nullptr) = 0;
    
    virtual void FreeResult(DecodeResult& result) = 0;
    
    // Optional methods
    virtual PluginErrorCode GetMetadata(const std::string& path,
                                       const std::vector<uint8_t>& data,
                                       ImageMetadata& metadata) {
        (void)path; (void)data; (void)metadata; // Suppress C4100 warnings
        return PLUGIN_ERROR_UNSUPPORTED_FORMAT;
    }
};

} // namespace ExplorerLens

#endif // __cplusplus


