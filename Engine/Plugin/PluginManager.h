/******************************************************************************
 * ExplorerLens Plugin Manager
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Manages plugin discovery, loading, and lifecycle.
 *****************************************************************************/

#pragma once

#include "../SDK/plugin_api.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <functional>

namespace ExplorerLens {
namespace Engine {
 class PluginDecoder; // Forward declaration
}

//============================================================================
// Plugin Handle (loaded plugin instance)
//============================================================================

class PluginHandle {
public:
 PluginHandle(const std::filesystem::path& path);
 ~PluginHandle();
 
 // Non-copyable
 PluginHandle(const PluginHandle&) = delete;
 PluginHandle& operator=(const PluginHandle&) = delete; 
 // Make PluginManager a friend so it can access init_ and initialized_
 friend class PluginManager; 
 // Movable
 PluginHandle(PluginHandle&&) noexcept;
 PluginHandle& operator=(PluginHandle&&) noexcept;
 
 // Plugin information
 const PluginInfo* GetInfo() const { return info_; }
 const std::filesystem::path& GetPath() const { return path_; }
 bool IsLoaded() const { return module_ != nullptr; }
 
 // Check if this plugin can decode a file
 bool CanDecode(const std::filesystem::path& file_path) const;
 bool CanDecode(const uint8_t* data, size_t size) const;
 
 // Decode operations
 PluginErrorCode Decode(const DecodeRequest* request,
 DecodeResult* result,
 PluginProgressCallback progress = nullptr);
 
 void FreeResult(DecodeResult* result);
 
 // Optional operations
 PluginErrorCode GetMetadata(const std::filesystem::path& file_path,
 ImageMetadata* metadata);
 
 PluginErrorCode GetThumbnail(const std::filesystem::path& file_path,
 DecodeResult* result);

private:
 bool LoadPlugin();
 void UnloadPlugin();
 bool ValidatePlugin();
 
 std::filesystem::path path_;
 void* module_ = nullptr; // HMODULE on Windows, void* on Linux
 
 // Plugin function pointers
 decltype(&plugin_get_info) get_info_ = nullptr;
 decltype(&plugin_init) init_ = nullptr;
 decltype(&plugin_cleanup) cleanup_ = nullptr;
 decltype(&plugin_can_decode) can_decode_ = nullptr;
 decltype(&plugin_decode) decode_ = nullptr;
 decltype(&plugin_free_result) free_result_ = nullptr;
 decltype(&plugin_get_metadata) get_metadata_ = nullptr;
 decltype(&plugin_get_thumbnail) get_thumbnail_ = nullptr;
 
 const PluginInfo* info_ = nullptr;
 bool initialized_ = false;
};

//============================================================================
// Plugin Manager
//============================================================================

class PluginManager {
public:
 static PluginManager& Instance() {
 static PluginManager instance;
 return instance;
 }
 
 // Non-copyable, non-movable (singleton)
 PluginManager(const PluginManager&) = delete;
 PluginManager& operator=(const PluginManager&) = delete;
 PluginManager(PluginManager&&) = delete;
 PluginManager& operator=(PluginManager&&) = delete;
 
 // Plugin discovery and loading
 size_t ScanPluginDirectory(const std::filesystem::path& directory);
 bool LoadPlugin(const std::filesystem::path& plugin_path);
 bool UnloadPlugin(const std::string& plugin_name);
 void UnloadAllPlugins();
 
 // Plugin query
 size_t GetPluginCount() const { return plugins_.size(); }
 std::vector<std::string> GetPluginNames() const;
 const PluginInfo* GetPluginInfo(const std::string& plugin_name) const;
 PluginHandle* GetPluginHandle(const std::string& plugin_name);
 
 // Find suitable plugin for a file
 PluginHandle* FindPluginForFile(const std::filesystem::path& file_path);
 PluginHandle* FindPluginForExtension(const std::string& extension);
 PluginHandle* FindPluginForMimeType(const std::string& mime_type);
 
 // Decode using best available plugin
 PluginErrorCode DecodeFile(const std::filesystem::path& file_path,
 uint32_t target_width,
 uint32_t target_height,
 DecodeResult* result,
 PluginProgressCallback progress = nullptr);
 
 // Create IThumbnailDecoder wrapper for a plugin with automatic mode selection
 std::unique_ptr<Engine::PluginDecoder> CreateDecoderForPlugin(
 const std::string& plugin_name);
 
 // Create IThumbnailDecoder wrapper for best plugin for file
 std::unique_ptr<Engine::PluginDecoder> CreateDecoderForFile(
 const std::filesystem::path& file_path);
 
 // Plugin events (optional callbacks)
 using PluginLoadedCallback = std::function<void(const std::string&)>;
 using PluginUnloadedCallback = std::function<void(const std::string&)>;
 
 void SetPluginLoadedCallback(PluginLoadedCallback callback) {
 on_plugin_loaded_ = std::move(callback);
 }
 
 void SetPluginUnloadedCallback(PluginUnloadedCallback callback) {
 on_plugin_unloaded_ = std::move(callback);
 }
 
 // Memory allocation for plugins
 static void* PluginAlloc(size_t size, void* user_data);
 static void PluginFree(void* ptr, void* user_data);

private:
 PluginManager();
 ~PluginManager();
 
 // Plugin storage
 std::unordered_map<std::string, std::unique_ptr<PluginHandle>> plugins_;
 
 // Extension to plugin mapping (for fast lookup)
 std::unordered_map<std::string, PluginHandle*> extension_map_;
 std::unordered_map<std::string, PluginHandle*> mime_map_;
 
 // Callbacks
 PluginLoadedCallback on_plugin_loaded_;
 PluginUnloadedCallback on_plugin_unloaded_;
 
 // Memory allocator for plugins
 PluginAllocator allocator_;
 
 // Helper methods
 void BuildExtensionMaps();
 void UpdateExtensionMap(PluginHandle* plugin);
 void RemoveFromExtensionMap(const std::string& plugin_name);
 
 // Statistics
 struct {
 size_t plugins_loaded = 0;
 size_t plugins_failed = 0;
 size_t total_decodes = 0;
 size_t failed_decodes = 0;
 } stats_;
};

//============================================================================
// Plugin Discovery Helper
//============================================================================

class ManagerPluginDiscovery {
public:
 // Find all plugin DLLs in a directory (recursive optional)
 static std::vector<std::filesystem::path> FindPlugins(
 const std::filesystem::path& directory,
 bool recursive = false);
 
 // Check if file is a valid plugin DLL
 static bool IsPluginFile(const std::filesystem::path& file_path);
 
 // Get plugin search paths (system + user)
 static std::vector<std::filesystem::path> GetPluginSearchPaths();
 
 // Get default plugin directory
 static std::filesystem::path GetDefaultPluginDirectory();
};

//============================================================================
// Plugin Manifest Parser (for .dtplugin packages)
//============================================================================

struct PluginManifest {
 std::string name;
 std::string version;
 std::string author;
 std::string description;
 std::string license;
 std::vector<std::string> supported_extensions;
 std::vector<std::string> mime_types;
 std::vector<std::string> dependencies;
 uint32_t api_version;
 bool requires_gpu;
 
 static std::optional<PluginManifest> LoadFromFile(
 const std::filesystem::path& manifest_path);
 
 static std::optional<PluginManifest> LoadFromJSON(
 const std::string& json_data);
};

//============================================================================
// RAII Plugin Loader
//============================================================================

class ScopedPluginLoader {
public:
 explicit ScopedPluginLoader(const std::filesystem::path& directory) {
 count_ = PluginManager::Instance().ScanPluginDirectory(directory);
 }
 
 ~ScopedPluginLoader() {
 PluginManager::Instance().UnloadAllPlugins();
 }
 
 size_t GetLoadedCount() const { return count_; }
 
private:
 size_t count_ = 0;
};

} // namespace ExplorerLens

