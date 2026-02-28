/******************************************************************************
 * ExplorerLens Plugin Manager Implementation
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Manages plugin discovery, loading, and lifecycle.
 *****************************************************************************/

#include "PluginManager.h"
#include "PluginDecoder.h"
#include "IsolationModeSelector.h"
#include <Windows.h>
#include <ShlObj.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <mutex>

namespace ExplorerLens {

//============================================================================
// PluginHandle Implementation
//============================================================================

PluginHandle::PluginHandle(const std::filesystem::path& path)
 : path_(path)
{
 LoadPlugin();
}

PluginHandle::~PluginHandle() {
 UnloadPlugin();
}

PluginHandle::PluginHandle(PluginHandle&& other) noexcept
 : path_(std::move(other.path_))
 , module_(other.module_)
 , get_info_(other.get_info_)
 , init_(other.init_)
 , cleanup_(other.cleanup_)
 , can_decode_(other.can_decode_)
 , decode_(other.decode_)
 , free_result_(other.free_result_)
 , get_metadata_(other.get_metadata_)
 , get_thumbnail_(other.get_thumbnail_)
 , info_(other.info_)
 , initialized_(other.initialized_)
{
 other.module_ = nullptr;
 other.info_ = nullptr;
 other.initialized_ = false;
}

PluginHandle& PluginHandle::operator=(PluginHandle&& other) noexcept {
 if (this != &other) {
 UnloadPlugin();
 
 path_ = std::move(other.path_);
 module_ = other.module_;
 get_info_ = other.get_info_;
 init_ = other.init_;
 cleanup_ = other.cleanup_;
 can_decode_ = other.can_decode_;
 decode_ = other.decode_;
 free_result_ = other.free_result_;
 get_metadata_ = other.get_metadata_;
 get_thumbnail_ = other.get_thumbnail_;
 info_ = other.info_;
 initialized_ = other.initialized_;
 
 other.module_ = nullptr;
 other.info_ = nullptr;
 other.initialized_ = false;
 }
 return *this;
}

bool PluginHandle::LoadPlugin() {
 // Load the plugin DLL
 module_ = LoadLibraryW(path_.c_str());
 if (!module_) {
 return false;
 }
 
 // Get required function pointers
 get_info_ = reinterpret_cast<decltype(get_info_)>(
 GetProcAddress(static_cast<HMODULE>(module_), "plugin_get_info"));
 init_ = reinterpret_cast<decltype(init_)>(
 GetProcAddress(static_cast<HMODULE>(module_), "plugin_init"));
 cleanup_ = reinterpret_cast<decltype(cleanup_)>(
 GetProcAddress(static_cast<HMODULE>(module_), "plugin_cleanup"));
 can_decode_ = reinterpret_cast<decltype(can_decode_)>(
 GetProcAddress(static_cast<HMODULE>(module_), "plugin_can_decode"));
 decode_ = reinterpret_cast<decltype(decode_)>(
 GetProcAddress(static_cast<HMODULE>(module_), "plugin_decode"));
 free_result_ = reinterpret_cast<decltype(free_result_)>(
 GetProcAddress(static_cast<HMODULE>(module_), "plugin_free_result"));
 
 // Optional function pointers
 get_metadata_ = reinterpret_cast<decltype(get_metadata_)>(
 GetProcAddress(static_cast<HMODULE>(module_), "plugin_get_metadata"));
 get_thumbnail_ = reinterpret_cast<decltype(get_thumbnail_)>(
 GetProcAddress(static_cast<HMODULE>(module_), "plugin_get_thumbnail"));
 
 // Validate required functions
 if (!ValidatePlugin()) {
 UnloadPlugin();
 return false;
 }
 
 // Get plugin info
 info_ = get_info_();
 if (!info_) {
 UnloadPlugin();
 return false;
 }
 
 // Check API version compatibility (major version must match)
 uint32_t plugin_major = (info_->api_version >> 16) & 0xFFFF;
 uint32_t host_major = (EXPLORERLENS_PLUGIN_API_VERSION >> 16) & 0xFFFF;
 
 if (plugin_major != host_major) {
 UnloadPlugin();
 return false;
 }
 
 return true;
}

void PluginHandle::UnloadPlugin() {
 if (module_) {
 if (initialized_ && cleanup_) {
 cleanup_();
 initialized_ = false;
 }
 
 FreeLibrary(static_cast<HMODULE>(module_));
 module_ = nullptr;
 info_ = nullptr;
 
 get_info_ = nullptr;
 init_ = nullptr;
 cleanup_ = nullptr;
 can_decode_ = nullptr;
 decode_ = nullptr;
 free_result_ = nullptr;
 get_metadata_ = nullptr;
 get_thumbnail_ = nullptr;
 }
}

bool PluginHandle::ValidatePlugin() {
 return get_info_ && init_ && cleanup_ && can_decode_ && decode_ && free_result_;
}

bool PluginHandle::CanDecode(const std::filesystem::path& file_path) const {
 if (!module_ || !can_decode_) {
 return false;
 }
 
 auto u8path = file_path.u8string();
 std::string path_utf8(u8path.begin(), u8path.end());
 return can_decode_(path_utf8.c_str(), nullptr, 0);
}

bool PluginHandle::CanDecode(const uint8_t* data, size_t size) const {
 if (!module_ || !can_decode_) {
 return false;
 }
 
 return can_decode_(nullptr, data, size);
}

PluginErrorCode PluginHandle::Decode(const DecodeRequest* request,
 DecodeResult* result,
 PluginProgressCallback progress) {
 if (!module_ || !decode_ || !request || !result) {
 return PLUGIN_ERROR_INVALID_PARAMETER;
 }
 
 if (!initialized_) {
 return PLUGIN_ERROR_NOT_INITIALIZED;
 }
 
 return decode_(request, result, progress);
}

void PluginHandle::FreeResult(DecodeResult* result) {
 if (module_ && free_result_ && result) {
 free_result_(result);
 }
}

PluginErrorCode PluginHandle::GetMetadata(const std::filesystem::path& file_path,
 ImageMetadata* metadata) {
 if (!module_ || !get_metadata_ || !metadata) {
 return PLUGIN_ERROR_UNSUPPORTED_FORMAT;
 }
 
 auto u8path = file_path.u8string();
 std::string path_utf8(u8path.begin(), u8path.end());
 return get_metadata_(path_utf8.c_str(), nullptr, 0, metadata);
}

PluginErrorCode PluginHandle::GetThumbnail(const std::filesystem::path& file_path,
 DecodeResult* result) {
 if (!module_ || !get_thumbnail_ || !result) {
 return PLUGIN_ERROR_UNSUPPORTED_FORMAT;
 }
 
 auto u8path = file_path.u8string();
 std::string path_utf8(u8path.begin(), u8path.end());
 return get_thumbnail_(path_utf8.c_str(), nullptr, 0, result);
}

//============================================================================
// PluginManager Implementation
//============================================================================

PluginManager::PluginManager() {
 // Initialize memory allocator for plugins
 allocator_.alloc = &PluginManager::PluginAlloc;
 allocator_.free = &PluginManager::PluginFree;
 allocator_.user_data = this;
}

PluginManager::~PluginManager() {
 UnloadAllPlugins();
}

size_t PluginManager::ScanPluginDirectory(const std::filesystem::path& directory) {
 if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
 return 0;
 }
 
 size_t loaded_count = 0;
 
 try {
 for (const auto& entry : std::filesystem::directory_iterator(directory)) {
 if (entry.is_regular_file()) {
 auto ext = entry.path().extension();
 if (ext == L".dll" || ext == L".dtplugin") {
 if (LoadPlugin(entry.path())) {
 loaded_count++;
 }
 }
 }
 }
 } catch (const std::filesystem::filesystem_error&) {
 // Ignore filesystem errors
 }
 
 return loaded_count;
}

bool PluginManager::LoadPlugin(const std::filesystem::path& plugin_path) {
 // Check if already loaded
 std::string plugin_name = plugin_path.stem().string();
 if (plugins_.find(plugin_name) != plugins_.end()) {
 return false;
 }
 
 // Create plugin handle
 auto plugin = std::make_unique<PluginHandle>(plugin_path);
 if (!plugin->IsLoaded()) {
 stats_.plugins_failed++;
 return false;
 }
 
 // Initialize plugin
 auto error = plugin->init_(&allocator_);
 if (error != PLUGIN_SUCCESS) {
 stats_.plugins_failed++;
 return false;
 }
 plugin->initialized_ = true;
 
 // Update extension/MIME maps
 UpdateExtensionMap(plugin.get());
 
 // Store plugin
 plugins_[plugin_name] = std::move(plugin);
 stats_.plugins_loaded++;
 
 // Notify callback
 if (on_plugin_loaded_) {
 on_plugin_loaded_(plugin_name);
 }
 
 return true;
}

bool PluginManager::UnloadPlugin(const std::string& plugin_name) {
 auto it = plugins_.find(plugin_name);
 if (it == plugins_.end()) {
 return false;
 }
 
 // Remove from extension maps
 RemoveFromExtensionMap(plugin_name);
 
 // Notify callback
 if (on_plugin_unloaded_) {
 on_plugin_unloaded_(plugin_name);
 }
 
 // Remove plugin (destructor handles cleanup)
 plugins_.erase(it);
 
 return true;
}

void PluginManager::UnloadAllPlugins() {
 std::vector<std::string> plugin_names;
 plugin_names.reserve(plugins_.size());
 
 for (const auto& [name, _] : plugins_) {
 plugin_names.push_back(name);
 }
 
 for (const auto& name : plugin_names) {
 UnloadPlugin(name);
 }
}

std::vector<std::string> PluginManager::GetPluginNames() const {
 std::vector<std::string> names;
 names.reserve(plugins_.size());
 
 for (const auto& [name, _] : plugins_) {
 names.push_back(name);
 }
 
 return names;
}

const PluginInfo* PluginManager::GetPluginInfo(const std::string& plugin_name) const {
 auto it = plugins_.find(plugin_name);
 if (it == plugins_.end()) {
 return nullptr;
 }
 
 return it->second->GetInfo();
}

PluginHandle* PluginManager::GetPluginHandle(const std::string& plugin_name) {
 auto it = plugins_.find(plugin_name);
 if (it == plugins_.end()) {
 return nullptr;
 }
 
 return it->second.get();
}

PluginHandle* PluginManager::FindPluginForFile(const std::filesystem::path& file_path) {
 // First try extension lookup (fast path)
 auto ext = file_path.extension().string();
 std::transform(ext.begin(), ext.end(), ext.begin(), 
 [](char c) { return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });
 
 auto it = extension_map_.find(ext);
 if (it != extension_map_.end()) {
 if (it->second->CanDecode(file_path)) {
 return it->second;
 }
 }
 
 // Fallback: try all plugins
 for (auto& [name, plugin] : plugins_) {
 if (plugin->CanDecode(file_path)) {
 return plugin.get();
 }
 }
 
 return nullptr;
}

PluginHandle* PluginManager::FindPluginForExtension(const std::string& extension) {
 std::string ext = extension;
 std::transform(ext.begin(), ext.end(), ext.begin(), 
 [](char c) { return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });
 
 if (ext.empty() || ext[0] != '.') {
 ext = "." + ext;
 }
 
 auto it = extension_map_.find(ext);
 return (it != extension_map_.end()) ? it->second : nullptr;
}

PluginHandle* PluginManager::FindPluginForMimeType(const std::string& mime_type) {
 auto it = mime_map_.find(mime_type);
 return (it != mime_map_.end()) ? it->second : nullptr;
}

PluginErrorCode PluginManager::DecodeFile(const std::filesystem::path& file_path,
 uint32_t target_width,
 uint32_t target_height,
 DecodeResult* result,
 PluginProgressCallback progress) {
 if (!result) {
 return PLUGIN_ERROR_INVALID_PARAMETER;
 }
 
 // Find suitable plugin
 auto* plugin = FindPluginForFile(file_path);
 if (!plugin) {
 stats_.failed_decodes++;
 return PLUGIN_ERROR_UNSUPPORTED_FORMAT;
 }
 
 // Prepare decode request
 DecodeRequest request = {};
 auto u8path = file_path.u8string();
 std::string path_utf8(u8path.begin(), u8path.end());
 request.file_path = path_utf8.c_str();
 request.data = nullptr;
 request.data_size = 0;
 request.target_width = target_width;
 request.target_height = target_height;
 request.output_format = PIXEL_FORMAT_BGRA32;
 request.preserve_aspect_ratio = true;
 request.high_quality = true;
 request.frame_index = 0;
 request.user_data = nullptr;
 
 // Decode
 auto error = plugin->Decode(&request, result, progress);
 
 if (error == PLUGIN_SUCCESS) {
 stats_.total_decodes++;
 } else {
 stats_.failed_decodes++;
 }
 
 return error;
}

std::unique_ptr<Engine::PluginDecoder> PluginManager::CreateDecoderForPlugin(
 const std::string& plugin_name)
{
 // Get plugin handle
 auto* plugin_handle = GetPluginHandle(plugin_name);
 if (!plugin_handle || !plugin_handle->IsLoaded()) {
 return nullptr;
 }
 
 // Convert plugin name to wide string for plugin ID
 std::wstring plugin_id(plugin_name.begin(), plugin_name.end());
 
 // Get plugin path
 std::filesystem::path plugin_path = plugin_handle->GetPath();
 
 // Use PluginDecoderFactory to create decoder with automatic mode selection
 return Engine::PluginDecoderFactory::CreateDecoder(
 plugin_handle, plugin_id, plugin_path);
}

std::unique_ptr<Engine::PluginDecoder> PluginManager::CreateDecoderForFile(
 const std::filesystem::path& file_path)
{
 // Find suitable plugin
 auto* plugin_handle = FindPluginForFile(file_path);
 if (!plugin_handle || !plugin_handle->IsLoaded()) {
 return nullptr;
 }
 
 // Get plugin name for ID
 for (const auto& [name, plugin] : plugins_) {
 if (plugin.get() == plugin_handle) {
 return CreateDecoderForPlugin(name);
 }
 }
 
 return nullptr;
}

void* PluginManager::PluginAlloc(size_t size, void* user_data) {
 (void)user_data; // Unused parameter
 return malloc(size);
}

void PluginManager::PluginFree(void* ptr, void* user_data) {
 (void)user_data; // Unused parameter
 free(ptr);
}

void PluginManager::BuildExtensionMaps() {
 extension_map_.clear();
 mime_map_.clear();
 
 for (auto& [name, plugin] : plugins_) {
 UpdateExtensionMap(plugin.get());
 }
}

void PluginManager::UpdateExtensionMap(PluginHandle* plugin) {
 if (!plugin || !plugin->GetInfo()) {
 return;
 }
 
 const auto* info = plugin->GetInfo();
 
 // Map extensions
 if (info->supported_extensions) {
 for (size_t i = 0; info->supported_extensions[i] != nullptr; ++i) {
 std::string ext = info->supported_extensions[i];
 std::transform(ext.begin(), ext.end(), ext.begin(), 
 [](char c) { return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });
 extension_map_[ext] = plugin;
 }
 }
 
 // Map MIME types
 if (info->mime_types) {
 for (size_t i = 0; info->mime_types[i] != nullptr; ++i) {
 mime_map_[info->mime_types[i]] = plugin;
 }
 }
}

void PluginManager::RemoveFromExtensionMap(const std::string& plugin_name) {
 auto it = plugins_.find(plugin_name);
 if (it == plugins_.end()) {
 return;
 }
 
 PluginHandle* plugin = it->second.get();
 if (!plugin || !plugin->GetInfo()) {
 return;
 }
 
 const auto* info = plugin->GetInfo();
 
 // Remove extensions
 if (info->supported_extensions) {
 for (size_t i = 0; info->supported_extensions[i] != nullptr; ++i) {
 std::string ext = info->supported_extensions[i];
 std::transform(ext.begin(), ext.end(), ext.begin(), 
 [](char c) { return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });
 extension_map_.erase(ext);
 }
 }
 
 // Remove MIME types
 if (info->mime_types) {
 for (size_t i = 0; info->mime_types[i] != nullptr; ++i) {
 mime_map_.erase(info->mime_types[i]);
 }
 }
}

//============================================================================
// PluginDiscovery Implementation
//============================================================================

std::vector<std::filesystem::path> PluginDiscovery::FindPlugins(
 const std::filesystem::path& directory,
 bool recursive) {
 
 std::vector<std::filesystem::path> plugins;
 
 if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
 return plugins;
 }
 
 try {
 if (recursive) {
 for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
 if (entry.is_regular_file() && IsPluginFile(entry.path())) {
 plugins.push_back(entry.path());
 }
 }
 } else {
 for (const auto& entry : std::filesystem::directory_iterator(directory)) {
 if (entry.is_regular_file() && IsPluginFile(entry.path())) {
 plugins.push_back(entry.path());
 }
 }
 }
 } catch (const std::filesystem::filesystem_error&) {
 // Ignore errors
 }
 
 return plugins;
}

bool PluginDiscovery::IsPluginFile(const std::filesystem::path& file_path) {
 auto ext = file_path.extension();
 return ext == L".dll" || ext == L".dtplugin";
}

std::vector<std::filesystem::path> PluginDiscovery::GetPluginSearchPaths() {
 std::vector<std::filesystem::path> paths;
 
 // 1. User's local app data
 wchar_t local_appdata[MAX_PATH];
 if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, local_appdata))) {
 auto user_plugins = std::filesystem::path(local_appdata) / L"ExplorerLens" / L"Plugins";
 paths.push_back(user_plugins);
 }
 
 // 2. Program Files installation
 wchar_t program_files[MAX_PATH];
 if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, program_files))) {
 auto system_plugins = std::filesystem::path(program_files) / L"ExplorerLens" / L"Plugins";
 paths.push_back(system_plugins);
 }
 
 // 3. Current executable directory
 wchar_t exe_path[MAX_PATH];
 if (GetModuleFileNameW(nullptr, exe_path, MAX_PATH) > 0) {
 auto exe_dir = std::filesystem::path(exe_path).parent_path() / L"Plugins";
 paths.push_back(exe_dir);
 }
 
 return paths;
}

std::filesystem::path PluginDiscovery::GetDefaultPluginDirectory() {
 wchar_t local_appdata[MAX_PATH];
 if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, local_appdata))) {
 return std::filesystem::path(local_appdata) / L"ExplorerLens" / L"Plugins";
 }
 
 return std::filesystem::path();
}

//============================================================================
// PluginManifest Implementation (Basic JSON Parser)
//============================================================================

std::optional<PluginManifest> PluginManifest::LoadFromFile(
 const std::filesystem::path& manifest_path) {
 
 if (!std::filesystem::exists(manifest_path)) {
 return std::nullopt;
 }
 
 std::ifstream file(manifest_path);
 if (!file) {
 return std::nullopt;
 }
 
 std::stringstream buffer;
 buffer << file.rdbuf();
 
 return LoadFromJSON(buffer.str());
}

// Simple JSON parser (basic implementation - should use proper JSON library in production)
std::optional<PluginManifest> PluginManifest::LoadFromJSON(const std::string& json_data) {
 PluginManifest manifest;
 
 // This is a very basic parser - in production, use nlohmann/json or similar
 // For now, we'll implement a minimal parser for basic key-value pairs
 
 auto extract_string = [](const std::string& json, const std::string& key) -> std::string {
 std::string search = "\"" + key + "\"";
 size_t pos = json.find(search);
 if (pos == std::string::npos) return "";
 
 pos = json.find(":", pos);
 if (pos == std::string::npos) return "";
 
 pos = json.find("\"", pos);
 if (pos == std::string::npos) return "";
 pos++;
 
 size_t end = json.find("\"", pos);
 if (end == std::string::npos) return "";
 
 return json.substr(pos, end - pos);
 };
 
 auto extract_uint = [](const std::string& json, const std::string& key) -> uint32_t {
 std::string search = "\"" + key + "\"";
 size_t pos = json.find(search);
 if (pos == std::string::npos) return 0;
 
 pos = json.find(":", pos);
 if (pos == std::string::npos) return 0;
 pos++;
 
 // Skip whitespace
 while (pos < json.length() && std::isspace(json[pos])) pos++;
 
 return static_cast<uint32_t>(std::atoi(&json[pos]));
 };
 
 auto extract_bool = [](const std::string& json, const std::string& key) -> bool {
 std::string search = "\"" + key + "\"";
 size_t pos = json.find(search);
 if (pos == std::string::npos) return false;
 
 pos = json.find(":", pos);
 if (pos == std::string::npos) return false;
 
 size_t true_pos = json.find("true", pos);
 size_t false_pos = json.find("false", pos);
 
 if (true_pos != std::string::npos && (false_pos == std::string::npos || true_pos < false_pos)) {
 return true;
 }
 return false;
 };
 
 manifest.name = extract_string(json_data, "name");
 manifest.version = extract_string(json_data, "version");
 manifest.author = extract_string(json_data, "author");
 manifest.description = extract_string(json_data, "description");
 manifest.license = extract_string(json_data, "license");
 manifest.api_version = extract_uint(json_data, "api_version");
 manifest.requires_gpu = extract_bool(json_data, "requires_gpu");
 
 // Extract arrays (simplified - just look for comma-separated values)
 // In production, use a proper JSON library
 
 return manifest;
}

} // namespace ExplorerLens


