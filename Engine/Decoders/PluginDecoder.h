/******************************************************************************
 * ExplorerLens Engine - Plugin Decoder Wrapper
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Wraps a PluginHandle to implement the IThumbnailDecoder interface,
 * allowing plugins to participate in the thumbnail generation pipeline.
 *****************************************************************************/

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include "../Plugin/PluginManager.h"
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

//============================================================================
/// Decoder wrapper for plugin-based format handlers
/// 
/// This class wraps a PluginHandle and adapts it to the IThumbnailDecoder
/// interface, allowing third-party plugins to seamlessly integrate with
/// the ExplorerLens thumbnail pipeline.
/// 
/// The wrapper handles:
/// - Conversion between wchar_t paths and UTF-8 for plugin API
/// - Conversion from plugin pixel formats to HBITMAP
/// - Error code translation between plugin and engine
/// - Proper memory management and cleanup
//============================================================================
class PluginDecoder : public IThumbnailDecoder
{
public:
 /// Construct from a plugin handle
 /// @param plugin Pointer to plugin handle (does not take ownership)
 /// @param pluginName Name of the plugin for identification
 explicit PluginDecoder(PluginHandle* plugin, const std::string& pluginName);
 
 virtual ~PluginDecoder() = default;
 
 // IThumbnailDecoder interface
 bool CanDecode(const wchar_t* filePath) override;
 HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
 DecoderInfo GetInfo() const override;
 const wchar_t* GetName() const override;
 const wchar_t** GetSupportedExtensions() const override;
 uint32_t GetExtensionCount() const override;
 bool SupportsGPU() const override;
 bool IsArchiveDecoder() const override;
 
private:
 /// Convert plugin pixel data (BGRA32) to HBITMAP
 /// @param pixels Pixel data buffer
 /// @param width Image width in pixels
 /// @param height Image height in pixels
 /// @param stride Row stride in bytes
 /// @return HBITMAP handle, or nullptr on failure
 HBITMAP CreateBitmapFromPixels(const uint8_t* pixels, 
 uint32_t width, 
 uint32_t height, 
 uint32_t stride);
 
 /// Convert wchar_t path to UTF-8 string
 std::string WideToUtf8(const wchar_t* wstr);
 
 /// Convert UTF-8 string to wchar_t
 std::wstring Utf8ToWide(const char* str);
 
 PluginHandle* plugin_; // Non-owning pointer to plugin
 std::string pluginName_; // Plugin name for display
 std::wstring displayName_; // Wide-char display name
 std::vector<std::wstring> extensions_; // Cached extensions as wide strings
 std::vector<const wchar_t*> extensionPtrs_; // Pointers for GetSupportedExtensions()
 DecoderInfo info_; // Cached decoder info
};

} // namespace Engine
} // namespace ExplorerLens

