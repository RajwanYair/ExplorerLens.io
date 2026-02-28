//==============================================================================
// ExplorerLens Engine - Thumbnail Decoder Interface
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once

#include "../Types.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// Interface for thumbnail decoders
/// 
/// All decoder implementations must implement this interface to participate
/// in the thumbnail generation pipeline.
/// 
/// Example usage:
/// @code
/// class MyDecoder : public IThumbnailDecoder {
/// public:
/// bool CanDecode(const wchar_t* filePath) override {
/// return wcsstr(filePath, L".myformat") != nullptr;
/// }
/// 
/// HRESULT Decode(const ThumbnailRequest& request, 
/// ThumbnailResult& result) override {
/// // Decode file and create HBITMAP
/// result.hBitmap = /* ... */;
/// result.status = S_OK;
/// return S_OK;
/// }
/// };
/// @endcode
//==============================================================================
class IThumbnailDecoder
{
public:
 virtual ~IThumbnailDecoder() = default;
 
 //==========================================================================
 /// Check if this decoder can handle the specified file
 /// 
 /// @param filePath Full path to the file to check
 /// @return true if this decoder supports the file format, false otherwise
 /// 
 /// @note This should be a lightweight check (extension or magic bytes)
 /// Do not attempt to fully decode the file here.
 //==========================================================================
 virtual bool CanDecode(const wchar_t* filePath) = 0;
 
 //==========================================================================
 /// Decode the file and generate a thumbnail
 /// 
 /// @param request Thumbnail generation request with file path and dimensions
 /// @param result Output result structure (decoder fills in hBitmap, width, height)
 /// @return S_OK on success, error HRESULT on failure
 /// 
 /// @note The decoder MUST create an HBITMAP and assign it to result.hBitmap
 /// The caller is responsible for deleting the bitmap with DeleteObject()
 /// 
 /// @note If the file is an archive (ZIP, RAR), extract the first image
 /// and decode that.
 //==========================================================================
 virtual HRESULT Decode(
 const ThumbnailRequest& request, 
 ThumbnailResult& result) = 0;
 
 //==========================================================================
 /// Get decoder information
 /// 
 /// @return Decoder capability information (name, version, extensions)
 //==========================================================================
 virtual DecoderInfo GetInfo() const = 0;
 
 //==========================================================================
 /// Get the decoder's display name
 /// 
 /// @return Human-readable name (e.g., "JPEG Image Decoder")
 //==========================================================================
 virtual const wchar_t* GetName() const = 0;
 
 //==========================================================================
 /// Get the file extensions this decoder supports
 /// 
 /// @return Null-terminated array of extensions (e.g., {L".jpg", L".jpeg", nullptr})
 /// 
 /// @note Extensions should include the leading dot
 /// @note Array must remain valid for the lifetime of the decoder
 //==========================================================================
 virtual const wchar_t** GetSupportedExtensions() const = 0;
 
 //==========================================================================
 /// Get the number of extensions supported
 /// 
 /// @return Count of extensions in GetSupportedExtensions() array
 //==========================================================================
 virtual uint32_t GetExtensionCount() const = 0;
 
 //==========================================================================
 /// Check if this decoder supports GPU acceleration
 /// 
 /// @return true if GPU acceleration is available, false otherwise
 //==========================================================================
 virtual bool SupportsGPU() const = 0;
 
 //==========================================================================
 /// Check if this decoder handles archive formats
 /// 
 /// @return true if this decoder extracts images from archives (ZIP, RAR, etc.)
 //==========================================================================
 virtual bool IsArchiveDecoder() const = 0;
};

} // namespace Engine
} // namespace ExplorerLens

