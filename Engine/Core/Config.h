//==============================================================================
// DarkThumbs Engine - Configuration & Feature Flags
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#pragma once

#include "EngineAPI.h"
#include <windows.h>
#include <string>

namespace DarkThumbs {
namespace Engine {

//==============================================================================
// Configuration Flags
//==============================================================================

/// Runtime configuration loaded from registry
struct EngineConfig {
    // Feature toggles - ALL ENABLED BY DEFAULT for maximum functionality
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
    
    // Decoder-specific flags - ALL ENABLED BY DEFAULT
    bool enableJXL = true;
    bool enableHEIF = true;
    bool enableRAW = true;
    bool enableSVG = true;
    bool enablePDF = true;          // PDF thumbnails enabled
    bool enableVideo = true;
    bool enableAudio = true;
    bool enableDocuments = true;
    bool enableFonts = true;
    bool enable3DModels = true;     // 3D formats (.obj, .stl, .gltf) ENABLED
    bool enableArchives = true;     // CBZ, CBR, CB7, etc.
    
    // Cache behavior - ENABLED for maximum performance
    bool enableCachePreWarming = true;      // Background pre-generation ENABLED
    bool enableSmartCache = true;           // Intelligent cache prioritization
    uint32_t cacheWriteDelay = 100;         // ms - Batch writes for performance
    
    // GPU options - OPTIMIZED
    bool preferD3D12 = true;                // Try D3D12 first, fallback to D3D11
    bool enableGPUBatchProcessing = true;   // Batch multiple thumbnails on GPU
    uint32_t gpuBatchSize = 16;             // Increased to 16 for better throughput
    
    // Threading - USE ALL CORES
    bool useWindowsThreadPool = true;       // Windows Thread Pool ENABLED
    uint32_t ioCompletionThreads = 0;       // 0 = auto-detect based on CPU
    
    // Memory management - INCREASED LIMITS
    uint32_t maxImageMemoryMB = 2048;       // 2GB max per image (vs 512MB)
    bool enableMemoryMappedIO = true;       // Memory-mapped I/O enabled
    uint32_t mmapThresholdKB = 4096;        // 4MB threshold (vs 1MB)
    
    // Debug/diagnostics - ENABLED
    bool enablePerformanceLogging = true;   // ENABLED for optimization
    bool enableVerboseLogging = false;      // Keep disabled (too noisy)
    bool enableETWTracing = true;           // Event Tracing ENABLED
    bool enableHealthMonitoring = true;     // Decoder health checks
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
ENGINE_API EngineConfig& GetEngineConfig();

} // namespace Engine
} // namespace DarkThumbs
