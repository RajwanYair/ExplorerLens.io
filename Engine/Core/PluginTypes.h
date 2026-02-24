/******************************************************************************
 * ExplorerLens Plugin Types - Unified Type Bridge
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Shared type definitions and conversion utilities used by:
 *   - Engine pipeline (ThumbnailRequest/ThumbnailResult)
 *   - Plugin system (DecodeRequest/DecodeResult from SDK/plugin_api.h)
 *   - IPC layer (serialization/deserialization for PluginHost communication)
 * 
 * Purpose:
 *   This header centralizes the conversion logic between Engine core types
 *   and Plugin SDK types, ensuring consistent translation across all code
 *   paths (in-worker, PluginHost, and future network plugins).
 * 
 * Usage:
 *   #include "PluginTypes.h"
 *   
 *   // Convert Engine request to Plugin SDK request
 *   DecodeRequest pluginReq;
 *   PluginTypeConvert::ToPluginRequest(engineRequest, pluginReq);
 *   
 *   // Convert Plugin SDK result to Engine result  
 *   ThumbnailResult engineResult;
 *   PluginTypeConvert::ToEngineResult(pluginResult, engineResult);
 *****************************************************************************/

#pragma once

#include "Types.h"
#include "../../SDK/plugin_api.h"
#include <Windows.h>
#include <string>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

//============================================================================
// IPC Serialization Enums
//============================================================================

/// Serialization format for IPC data transfer between Engine and PluginHost.
/// SharedMemory is preferred for large bitmap data (> 4KB).
/// Pipe is used for small messages (requests, status, heartbeats).
enum class IPCTransferMode : uint32_t
{
    Pipe = 0,            ///< Named pipe (small payloads, e.g. < 4KB)
    SharedMemory = 1,    ///< Shared memory (large payloads, bitmap data)
};

/// Status of a plugin decode operation tracked across IPC boundary.
enum class PluginDecodeStatus : uint32_t
{
    Pending = 0,         ///< Request sent, waiting for response
    InProgress = 1,      ///< PluginHost acknowledged, decoding in progress
    Completed = 2,       ///< Decode finished successfully
    Failed = 3,          ///< Decode failed (check error_code)
    Timeout = 4,         ///< Response not received within timeout
    Crashed = 5,         ///< PluginHost process crashed during decode
    Cancelled = 6,       ///< Request was cancelled by the Engine
};

//============================================================================
// IPC Serializable Structures
//============================================================================

/// Flat, fixed-size representation of a decode request for IPC transfer.
/// All strings are inline char arrays (no pointers) for safe cross-process use.
/// 
/// This is the wire format sent from Engine -> PluginHost over named pipe
/// or shared memory.
struct alignas(8) SerializableDecodeRequest
{
    /// Source file path (UTF-8, null-terminated)
    char file_path[520];          // MAX_PATH * 2 for safety

    /// Desired output dimensions
    uint32_t target_width;
    uint32_t target_height;

    /// Desired output pixel format
    PixelFormat output_format;

    /// Decode options
    bool preserve_aspect_ratio;
    bool high_quality;
    uint32_t frame_index;

    /// Correlation ID for matching request/response across IPC
    uint64_t correlation_id;

    /// Reserved for future expansion (version, additional flags, etc.)
    uint8_t reserved[32];
};

/// Flat, fixed-size header for a decode result sent back from PluginHost.
/// The actual pixel data follows this header in shared memory or as a
/// separate pipe message.
struct alignas(8) SerializableDecodeResult
{
    /// Output dimensions
    uint32_t width;
    uint32_t height;
    uint32_t stride;

    /// Pixel format of the returned data
    PixelFormat pixel_format;

    /// Size of pixel data buffer (follows this header in shared memory)
    uint64_t pixel_data_size;

    /// Error information
    PluginErrorCode error_code;
    char error_message[256];

    /// Correlation ID matching the request
    uint64_t correlation_id;

    /// Operation status
    PluginDecodeStatus status;

    /// Decode timing (microseconds)
    uint64_t decode_time_us;

    /// Reserved for future expansion
    uint8_t reserved[24];
};

//============================================================================
// Type Conversion Utilities
//============================================================================

/// Stateless conversion functions between Engine types and Plugin SDK types.
/// All methods are static and thread-safe (no shared state).
struct PluginTypeConvert
{
    //------------------------------------------------------------------------
    /// Convert Engine ThumbnailRequest to Plugin SDK DecodeRequest
    /// 
    /// @param src  Engine-side request
    /// @param dst  Plugin SDK request (output, zero-initialized first)
    /// 
    /// @note file_path is converted from wide string to UTF-8.
    ///       The dst.file_path pointer references an internal static buffer;
    ///       caller must consume before next call or copy the string.
    //------------------------------------------------------------------------
    static void ToPluginRequest(const ThumbnailRequest& src, DecodeRequest& dst)
    {
        std::memset(&dst, 0, sizeof(dst));

        // Convert wide file path to UTF-8
        if (src.filePath)
        {
            thread_local static char utf8_buf[1040];
            int len = WideCharToMultiByte(CP_UTF8, 0,
                                          src.filePath, -1,
                                          utf8_buf, sizeof(utf8_buf),
                                          nullptr, nullptr);
            dst.file_path = (len > 0) ? utf8_buf : nullptr;
        }

        dst.target_width  = src.width;
        dst.target_height = src.height;
        dst.output_format = PIXEL_FORMAT_BGRA32;  // Windows native

        dst.preserve_aspect_ratio = 
            (src.flags & ThumbnailFlags::PreserveAspect);
        dst.high_quality = 
            (src.flags & ThumbnailFlags::HighQuality);
        dst.frame_index = 0;
    }

    //------------------------------------------------------------------------
    /// Convert Plugin SDK DecodeResult to Engine ThumbnailResult
    /// 
    /// @param src  Plugin SDK result (pixels owned by plugin)
    /// @param dst  Engine-side result (output)
    /// @return S_OK if conversion succeeded, error HRESULT otherwise
    /// 
    /// @note This does NOT create an HBITMAP. The caller must call
    ///       CreateHBITMAPFromPixels separately if needed.
    //------------------------------------------------------------------------
    static HRESULT ToEngineResult(const DecodeResult& src, ThumbnailResult& dst)
    {
        dst.width  = src.width;
        dst.height = src.height;
        dst.usedGPU  = false;
        dst.fromCache = false;
        dst.hBitmap = nullptr;  // Caller creates HBITMAP from pixels

        if (src.error_code != PLUGIN_SUCCESS)
        {
            dst.status = TranslateErrorCode(src.error_code);
            return dst.status;
        }

        if (!src.pixels || src.width == 0 || src.height == 0)
        {
            dst.status = E_UNEXPECTED;
            return E_UNEXPECTED;
        }

        dst.status = S_OK;
        return S_OK;
    }

    //------------------------------------------------------------------------
    /// Convert Engine ThumbnailRequest to IPC-safe SerializableDecodeRequest
    /// 
    /// @param src            Engine request
    /// @param dst            Serializable output (zero-initialized first)
    /// @param correlation_id Unique ID for request/response matching
    //------------------------------------------------------------------------
    static void ToSerializable(const ThumbnailRequest& src,
                               SerializableDecodeRequest& dst,
                               uint64_t correlation_id)
    {
        std::memset(&dst, 0, sizeof(dst));

        if (src.filePath)
        {
            WideCharToMultiByte(CP_UTF8, 0,
                                src.filePath, -1,
                                dst.file_path, sizeof(dst.file_path),
                                nullptr, nullptr);
        }

        dst.target_width  = src.width;
        dst.target_height = src.height;
        dst.output_format = PIXEL_FORMAT_BGRA32;
        dst.preserve_aspect_ratio = 
            (src.flags & ThumbnailFlags::PreserveAspect);
        dst.high_quality = 
            (src.flags & ThumbnailFlags::HighQuality);
        dst.frame_index   = 0;
        dst.correlation_id = correlation_id;
    }

    //------------------------------------------------------------------------
    /// Convert IPC SerializableDecodeRequest to Plugin SDK DecodeRequest
    /// 
    /// @param src  Serializable wire-format request
    /// @param dst  Plugin SDK request (output)
    //------------------------------------------------------------------------
    static void FromSerializable(const SerializableDecodeRequest& src,
                                 DecodeRequest& dst)
    {
        std::memset(&dst, 0, sizeof(dst));

        dst.file_path     = src.file_path;
        dst.target_width  = src.target_width;
        dst.target_height = src.target_height;
        dst.output_format = src.output_format;
        dst.preserve_aspect_ratio = src.preserve_aspect_ratio;
        dst.high_quality  = src.high_quality;
        dst.frame_index   = src.frame_index;
    }

    //------------------------------------------------------------------------
    /// Translate Plugin SDK error code to HRESULT
    //------------------------------------------------------------------------
    static HRESULT TranslateErrorCode(PluginErrorCode code)
    {
        switch (code)
        {
        case PLUGIN_SUCCESS:                    return S_OK;
        case PLUGIN_ERROR_INVALID_PARAMETER:    return E_INVALIDARG;
        case PLUGIN_ERROR_UNSUPPORTED_FORMAT:   return DT_E_UNSUPPORTED_FORMAT;
        case PLUGIN_ERROR_OUT_OF_MEMORY:        return E_OUTOFMEMORY;
        case PLUGIN_ERROR_FILE_NOT_FOUND:       return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        case PLUGIN_ERROR_READ_ERROR:           return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
        case PLUGIN_ERROR_DECODE_ERROR:         return DT_E_INVALID_IMAGE_DATA;
        case PLUGIN_ERROR_CORRUPTED_DATA:       return DT_E_INVALID_IMAGE_DATA;
        case PLUGIN_ERROR_NOT_INITIALIZED:      return E_NOT_VALID_STATE;
        default:                                return E_FAIL;
        }
    }

    //------------------------------------------------------------------------
    /// Translate PluginDecodeStatus to human-readable string
    //------------------------------------------------------------------------
    static const wchar_t* StatusToString(PluginDecodeStatus status)
    {
        switch (status)
        {
        case PluginDecodeStatus::Pending:     return L"Pending";
        case PluginDecodeStatus::InProgress:  return L"In Progress";
        case PluginDecodeStatus::Completed:   return L"Completed";
        case PluginDecodeStatus::Failed:      return L"Failed";
        case PluginDecodeStatus::Timeout:     return L"Timeout";
        case PluginDecodeStatus::Crashed:     return L"Crashed";
        case PluginDecodeStatus::Cancelled:   return L"Cancelled";
        default:                              return L"Unknown";
        }
    }
};

} // namespace Engine
} // namespace ExplorerLens

