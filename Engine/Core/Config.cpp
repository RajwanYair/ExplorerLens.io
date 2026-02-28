//==============================================================================
// ExplorerLens Engine - Configuration Implementation
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "Config.h"
#include <mutex>

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// Registry Helpers
//==============================================================================

bool EngineConfig::ReadRegistryBool(HKEY key, const wchar_t* valueName, bool defaultValue) {
 DWORD value = defaultValue ? 1 : 0;
 DWORD size = sizeof(DWORD);
 DWORD type = REG_DWORD;
 
 if (RegQueryValueExW(key, valueName, nullptr, &type, 
 reinterpret_cast<BYTE*>(&value), &size) == ERROR_SUCCESS) {
 return (value != 0);
 }
 return defaultValue;
}

uint32_t EngineConfig::ReadRegistryDWORD(HKEY key, const wchar_t* valueName, uint32_t defaultValue) {
 DWORD value = defaultValue;
 DWORD size = sizeof(DWORD);
 DWORD type = REG_DWORD;
 
 if (RegQueryValueExW(key, valueName, nullptr, &type,
 reinterpret_cast<BYTE*>(&value), &size) == ERROR_SUCCESS) {
 return value;
 }
 return defaultValue;
}

bool EngineConfig::WriteRegistryBool(HKEY key, const wchar_t* valueName, bool value) {
 DWORD dword = value ? 1 : 0;
 return RegSetValueExW(key, valueName, 0, REG_DWORD,
 reinterpret_cast<const BYTE*>(&dword), sizeof(DWORD)) == ERROR_SUCCESS;
}

bool EngineConfig::WriteRegistryDWORD(HKEY key, const wchar_t* valueName, uint32_t value) {
 return RegSetValueExW(key, valueName, 0, REG_DWORD,
 reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) == ERROR_SUCCESS;
}

//==============================================================================
// Load/Save
//==============================================================================

bool EngineConfig::LoadFromRegistry(HKEY rootKey, const wchar_t* subKey) {
 HKEY hKey = nullptr;
 if (RegOpenKeyExW(rootKey, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
 return false; // No registry settings, use defaults
 }
 
 // Feature toggles
 enableGPU = ReadRegistryBool(hKey, L"EnableGPU", true);
 enableCache = ReadRegistryBool(hKey, L"EnableCache", true);
 enableParallelDecode = ReadRegistryBool(hKey, L"EnableParallelDecode", true);
 enablePlugins = ReadRegistryBool(hKey, L"EnablePlugins", true);
 
 // Legacy compatibility
 allowLegacyFallback = ReadRegistryBool(hKey, L"AllowLegacyFallback", false);
 
 // Performance
 maxConcurrentDecodes = ReadRegistryDWORD(hKey, L"MaxConcurrentDecodes", 4);
 cacheMaxSizeMB = ReadRegistryDWORD(hKey, L"CacheMaxSizeMB", 512);
 cacheTTLSeconds = ReadRegistryDWORD(hKey, L"CacheTTLSeconds", 3600);
 
 // Decoder toggles
 enableJXL = ReadRegistryBool(hKey, L"EnableJXL", true);
 enableHEIF = ReadRegistryBool(hKey, L"EnableHEIF", true);
 enableRAW = ReadRegistryBool(hKey, L"EnableRAW", true);
 enableSVG = ReadRegistryBool(hKey, L"EnableSVG", true);
 enablePDF = ReadRegistryBool(hKey, L"EnablePDF", true);
 enableVideo = ReadRegistryBool(hKey, L"EnableVideo", true);
 enableAudio = ReadRegistryBool(hKey, L"EnableAudio", true);
 enableDocuments = ReadRegistryBool(hKey, L"EnableDocuments", true);
 enableFonts = ReadRegistryBool(hKey, L"EnableFonts", true);
 enable3DModels = ReadRegistryBool(hKey, L"Enable3DModels", false);
 enableArchives = ReadRegistryBool(hKey, L"EnableArchives", true);
 
 // Cache behavior
 enableCachePreWarming = ReadRegistryBool(hKey, L"EnableCachePreWarming", false);
 enableSmartCache = ReadRegistryBool(hKey, L"EnableSmartCache", true);
 cacheWriteDelay = ReadRegistryDWORD(hKey, L"CacheWriteDelay", 100);
 
 // GPU
 preferD3D12 = ReadRegistryBool(hKey, L"PreferD3D12", true);
 enableGPUBatchProcessing = ReadRegistryBool(hKey, L"EnableGPUBatchProcessing", true);
 gpuBatchSize = ReadRegistryDWORD(hKey, L"GPUBatchSize", 8);
 
 // Threading
 useWindowsThreadPool = ReadRegistryBool(hKey, L"UseWindowsThreadPool", false);
 ioCompletionThreads = ReadRegistryDWORD(hKey, L"IoCompletionThreads", 2);
 
 // Memory
 maxImageMemoryMB = ReadRegistryDWORD(hKey, L"MaxImageMemoryMB", 512);
 enableMemoryMappedIO = ReadRegistryBool(hKey, L"EnableMemoryMappedIO", true);
 mmapThresholdKB = ReadRegistryDWORD(hKey, L"MmapThresholdKB", 1024);
 
 // Diagnostics
 enablePerformanceLogging = ReadRegistryBool(hKey, L"EnablePerformanceLogging", false);
 enableVerboseLogging = ReadRegistryBool(hKey, L"EnableVerboseLogging", false);
 enableETWTracing = ReadRegistryBool(hKey, L"EnableETWTracing", false);
 enableHealthMonitoring = ReadRegistryBool(hKey, L"EnableHealthMonitoring", true);
 healthCheckIntervalSec = ReadRegistryDWORD(hKey, L"HealthCheckIntervalSec", 300);
 
 RegCloseKey(hKey);
 return true;
}

bool EngineConfig::SaveToRegistry(HKEY rootKey, const wchar_t* subKey) const {
 HKEY hKey = nullptr;
 DWORD disposition = 0;
 
 if (RegCreateKeyExW(rootKey, subKey, 0, nullptr, 0, KEY_WRITE, nullptr, 
 &hKey, &disposition) != ERROR_SUCCESS) {
 return false;
 }
 
 // Feature toggles
 WriteRegistryBool(hKey, L"EnableGPU", enableGPU);
 WriteRegistryBool(hKey, L"EnableCache", enableCache);
 WriteRegistryBool(hKey, L"EnableParallelDecode", enableParallelDecode);
 WriteRegistryBool(hKey, L"EnablePlugins", enablePlugins);
 
 // Legacy
 WriteRegistryBool(hKey, L"AllowLegacyFallback", allowLegacyFallback);
 
 // Performance
 WriteRegistryDWORD(hKey, L"MaxConcurrentDecodes", maxConcurrentDecodes);
 WriteRegistryDWORD(hKey, L"CacheMaxSizeMB", cacheMaxSizeMB);
 WriteRegistryDWORD(hKey, L"CacheTTLSeconds", cacheTTLSeconds);
 
 // Decoders
 WriteRegistryBool(hKey, L"EnableJXL", enableJXL);
 WriteRegistryBool(hKey, L"EnableHEIF", enableHEIF);
 WriteRegistryBool(hKey, L"EnableRAW", enableRAW);
 WriteRegistryBool(hKey, L"EnableSVG", enableSVG);
 WriteRegistryBool(hKey, L"EnablePDF", enablePDF);
 WriteRegistryBool(hKey, L"EnableVideo", enableVideo);
 WriteRegistryBool(hKey, L"EnableAudio", enableAudio);
 WriteRegistryBool(hKey, L"EnableDocuments", enableDocuments);
 WriteRegistryBool(hKey, L"EnableFonts", enableFonts);
 WriteRegistryBool(hKey, L"Enable3DModels", enable3DModels);
 WriteRegistryBool(hKey, L"EnableArchives", enableArchives);
 
 // Cache
 WriteRegistryBool(hKey, L"EnableCachePreWarming", enableCachePreWarming);
 WriteRegistryBool(hKey, L"EnableSmartCache", enableSmartCache);
 WriteRegistryDWORD(hKey, L"CacheWriteDelay", cacheWriteDelay);
 
 // GPU
 WriteRegistryBool(hKey, L"PreferD3D12", preferD3D12);
 WriteRegistryBool(hKey, L"EnableGPUBatchProcessing", enableGPUBatchProcessing);
 WriteRegistryDWORD(hKey, L"GPUBatchSize", gpuBatchSize);
 
 // Threading
 WriteRegistryBool(hKey, L"UseWindowsThreadPool", useWindowsThreadPool);
 WriteRegistryDWORD(hKey, L"IoCompletionThreads", ioCompletionThreads);
 
 // Memory
 WriteRegistryDWORD(hKey, L"MaxImageMemoryMB", maxImageMemoryMB);
 WriteRegistryBool(hKey, L"EnableMemoryMappedIO", enableMemoryMappedIO);
 WriteRegistryDWORD(hKey, L"MmapThresholdKB", mmapThresholdKB);
 
 // Diagnostics
 WriteRegistryBool(hKey, L"EnablePerformanceLogging", enablePerformanceLogging);
 WriteRegistryBool(hKey, L"EnableVerboseLogging", enableVerboseLogging);
 WriteRegistryBool(hKey, L"EnableETWTracing", enableETWTracing);
 WriteRegistryBool(hKey, L"EnableHealthMonitoring", enableHealthMonitoring);
 WriteRegistryDWORD(hKey, L"HealthCheckIntervalSec", healthCheckIntervalSec);
 
 RegCloseKey(hKey);
 return true;
}

//==============================================================================
// Global Instance
//==============================================================================

ENGINE_API EngineConfig& GetEngineConfig() {
 static EngineConfig instance;
 static std::once_flag initFlag;
 
 std::call_once(initFlag, []() {
 instance.LoadFromRegistry();
 });
 
 return instance;
}

} // namespace Engine
} // namespace ExplorerLens

