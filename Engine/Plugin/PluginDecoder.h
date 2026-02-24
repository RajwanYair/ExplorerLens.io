/******************************************************************************
 * ExplorerLens Plugin Decoder Adapter
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Adapter class that wraps plugin instances and provides IThumbnailDecoder
 * interface for seamless integration with the Engine pipeline.
 * 
 * Supports both isolation modes:
 * - In-Worker: Direct DLL loading for trusted plugins
 * - PluginHost: Separate process via PluginHostClient for security
 *****************************************************************************/

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include "PluginManager.h"
#include "IsolationModeSelector.h"
#include "PluginHostClient.h"
#include <memory>
#include <string>
#include <filesystem>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

//============================================================================
// Plugin Decoder Adapter
//============================================================================

/// Adapter that wraps a plugin and provides IThumbnailDecoder interface
class PluginDecoder : public IThumbnailDecoder {
public:
    /// Constructor for in-worker mode (direct plugin access)
    /// @param plugin_handle Pointer to loaded plugin (not owned)
    /// @param plugin_id Unique identifier for the plugin
    explicit PluginDecoder(PluginHandle* plugin_handle, 
                          const std::wstring& plugin_id);
    
    /// Constructor for PluginHost mode (isolated execution)
    /// @param plugin_path Path to plugin DLL
    /// @param plugin_id Unique identifier for the plugin
    PluginDecoder(const std::filesystem::path& plugin_path,
                 const std::wstring& plugin_id);
    
    ~PluginDecoder() override;
    
    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return info_.name; }
    const wchar_t** GetSupportedExtensions() const override { return info_.supportedExtensions; }
    uint32_t GetExtensionCount() const override { return info_.extensionCount; }
    bool SupportsGPU() const override { return info_.supportsGPU; }
    bool IsArchiveDecoder() const override { return info_.isArchiveDecoder; }
    
    /// Get plugin version (convenience method, also available via GetInfo().version)
    const wchar_t* GetVersion() const { return info_.version; }
    
    /// Get the isolation mode being used
    IsolationMode GetIsolationMode() const { return isolation_mode_; }
    
    /// Get plugin ID
    const std::wstring& GetPluginId() const { return plugin_id_; }
    
    /// Check if plugin is running in isolated mode
    bool IsIsolated() const { return isolation_mode_ == IsolationMode::PluginHost; }
    
    /// Get performance statistics
    struct Statistics {
        uint64_t total_decodes = 0;
        uint64_t successful_decodes = 0;
        uint64_t failed_decodes = 0;
        uint64_t total_decode_time_us = 0;
        uint64_t crashes = 0;
        uint64_t timeouts = 0;
    };
    
    Statistics GetStatistics() const;
    void ResetStatistics();

private:
    // In-Worker mode decoding
    HRESULT DecodeInWorker(const ThumbnailRequest& request, ThumbnailResult& result);
    
    // PluginHost mode decoding
    HRESULT DecodeInPluginHost(const ThumbnailRequest& request, ThumbnailResult& result);
    
    // Helper: Convert pixel data to HBITMAP
    HBITMAP CreateHBITMAPFromPixels(const uint8_t* pixels,
                                   uint32_t width, uint32_t height,
                                   PixelFormat format);
    
    // Plugin information
    std::wstring plugin_id_;
    std::filesystem::path plugin_path_;
    DecoderInfo info_;
    std::wstring plugin_name_;     // Storage for info_.name
    std::wstring plugin_version_;  // Storage for info_.version
    std::vector<const wchar_t*> extension_ptrs_;  // Storage for info_.supportedExtensions array
    std::vector<std::wstring> extension_strings_;  // Actual extension string storage
    
    // Execution mode
    IsolationMode isolation_mode_;
    
    // In-Worker mode: direct plugin handle (not owned)
    PluginHandle* plugin_handle_ = nullptr;
    
    // PluginHost mode: client for IPC
    std::unique_ptr<PluginHostClient> host_client_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Statistics
    Statistics stats_;
};

//============================================================================
// Plugin Decoder Factory
//============================================================================

/// Factory for creating PluginDecoders with automatic mode selection
class PluginDecoderFactory {
public:
    /// Create a decoder for a plugin with automatic mode selection
    /// @param plugin_handle Plugin handle from PluginManager
    /// @param plugin_id Unique plugin identifier
    /// @param plugin_path Path to plugin DLL
    /// @return Unique pointer to PluginDecoder or nullptr on failure
    static std::unique_ptr<PluginDecoder> CreateDecoder(
        PluginHandle* plugin_handle,
        const std::wstring& plugin_id,
        const std::filesystem::path& plugin_path);
    
    /// Create a decoder with explicit mode
    static std::unique_ptr<PluginDecoder> CreateDecoderWithMode(
        PluginHandle* plugin_handle,
        const std::wstring& plugin_id,
        const std::filesystem::path& plugin_path,
        IsolationMode mode);
};

} // namespace Engine
} // namespace ExplorerLens

