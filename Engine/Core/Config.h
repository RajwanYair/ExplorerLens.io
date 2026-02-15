//==============================================================================
// DarkThumbs Engine - Configuration & Feature Flags
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#pragma once

#include <windows.h>
#include <string>

namespace DarkThumbs {
namespace Engine {

//==============================================================================
// Configuration Flags
//==============================================================================

/// Runtime configuration loaded from registry
struct EngineConfig {
    // Feature toggles
    bool enableGPU = true;
    bool enableCache = true;
    bool enableParallelDecode = true;
    bool enablePlugins = true;
    
    // Legacy compatibility
    bool allowLegacyFallback = false;  // DEFAULT: Engine-only, no fallback
    
    // Performance tuning
    uint32_t maxConcurrentDecodes = 4;
    uint32_t cacheMaxSizeMB = 512;
    uint32_t cacheTTLSeconds = 3600;
    
    // Decoder-specific flags
    bool enableJXL = true;
    bool enableHEIF = true;
    bool enableRAW = true;
    bool enableSVG = true;
    bool enablePDF = true;
    bool enableVideo = true;
    bool enableAudio = true;
    bool enableDocuments = true;
    bool enableFonts = true;
    bool enable3DModels = false;  // 3D formats (.obj, .stl, .gltf) - experimental
    bool enableArchives = true;   // CBZ, CBR, CB7, etc.
    
    // Cache behavior
    bool enableCachePreWarming = false;     // Background pre-generation
    bool enableSmartCache = true;           // Intelligent cache prioritization
    uint32_t cacheWriteDelay = 100;         // ms - Batch writes for performance
    
    // GPU options
    bool preferD3D12 = true;             // Try D3D12 first, fallback to D3D11
    bool enableGPUBatchProcessing = true;  // Batch multiple thumbnails on GPU
    uint32_t gpuBatchSize = 8;            // Thumbnails per GPU batch
    
    // Threading
    bool useWindowsThreadPool = false;    // Use Windows Thread Pool API instead of std::async
    uint32_t ioCompletionThreads = 2;     // Threads for async IO completion
    
    // Memory management
    uint32_t maxImageMemoryMB = 512;      // Max memory per image decode
    bool enableMemoryMappedIO = true;     // Use memory-mapped I/O for large files
    uint32_t mmapThresholdKB = 1024;      // Files larger than this use mmap
    
    // Debug/diagnostics
    bool enablePerformanceLogging = false;
    bool enableVerboseLogging = false;
    bool enableETWTracing = false;        // Event Tracing for Windows
    bool enableHealthMonitoring = true;   // Decoder health checks
    uint32_t healthCheckIntervalSec = 300;  // Health check frequency
    
    /// Load configuration from registry
    /// @param rootKey Registry root (HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE)
    /// @param subKey Registry subkey path
    /// @return true if at least one setting was loaded
    bool LoadFromRegistry(
        HKEY rootKey = HKEY_CURRENT_USER,
        const wchar_t* subKey = L"Software\\DarkThumbs\\Engine");
    
    /// Save configuration to registry
    /// @param rootKey Registry root
    /// @param subKey Registry subkey path  
    /// @return true if saved successfully
    bool SaveToRegistry(
        HKEY rootKey = HKEY_CURRENT_USER,
        const wchar_t* subKey = L"Software\\DarkThumbs\\Engine") const;
    
private:
    static bool ReadRegistryBool(HKEY key, const wchar_t* valueName, bool defaultValue);
    static uint32_t ReadRegistryDWORD(HKEY key, const wchar_t* valueName, uint32_t defaultValue);
    static bool WriteRegistryBool(HKEY key, const wchar_t* valueName, bool value);
    static bool WriteRegistryDWORD(HKEY key, const wchar_t* valueName, uint32_t value);
};

//==============================================================================
// Global Configuration Instance
//==============================================================================

/// Get the global engine configuration
/// Thread-safe singleton pattern
EngineConfig& GetEngineConfig();

} // namespace Engine
} // namespace DarkThumbs
