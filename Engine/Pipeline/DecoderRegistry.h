//==============================================================================
// ExplorerLens Engine - Decoder Registry
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once

#include <memory>
#include <vector>
#include "../Core/IThumbnailDecoder.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// Registry for managing thumbnail decoders
///
/// The DecoderRegistry maintains a collection of decoder instances and provides
/// functionality to find the appropriate decoder for a given file.
///
/// Example usage:
/// @code
/// DecoderRegistry registry;
/// registry.RegisterDecoder(new ImageDecoder());
/// registry.RegisterDecoder(new ArchiveDecoder());
///
/// IThumbnailDecoder* decoder = registry.FindDecoder(L"image.jpg");
/// if (decoder) {
/// ThumbnailResult result;
/// decoder->Decode(request, result);
/// }
/// @endcode
//==============================================================================
class DecoderRegistry
{
  public:
    DecoderRegistry();
    ~DecoderRegistry();

    //==========================================================================
    /// Register a decoder with the registry
    ///
    /// @param decoder Pointer to decoder instance (registry takes ownership)
    /// @return true on success, false if decoder is null or already registered
    ///
    /// @note The registry takes ownership of the decoder and will delete it
    /// when the registry is destroyed
    //==========================================================================
    bool RegisterDecoder(IThumbnailDecoder* decoder);

    //==========================================================================
    /// Unregister a decoder from the registry
    ///
    /// @param decoder Pointer to decoder to remove
    /// @return true if decoder was found and removed, false otherwise
    ///
    /// @note The decoder will be deleted after removal
    //==========================================================================
    bool UnregisterDecoder(IThumbnailDecoder* decoder);

    //==========================================================================
    /// Find a decoder that can handle the specified file
    ///
    /// @param filePath Full path to the file
    /// @return Pointer to a decoder that can handle the file, or nullptr if none found
    ///
    /// @note Returns the first decoder that reports CanDecode() == true
    /// @note Decoders are checked in registration order
    //==========================================================================
    IThumbnailDecoder* FindDecoder(const wchar_t* filePath) const;

    //==========================================================================
    /// Find a decoder by name
    ///
    /// @param name Decoder name to search for
    /// @return Pointer to decoder with matching name, or nullptr if not found
    //==========================================================================
    IThumbnailDecoder* FindDecoderByName(const wchar_t* name) const;

    //==========================================================================
    /// Get the number of registered decoders
    ///
    /// @return Count of registered decoders
    //==========================================================================
    size_t GetDecoderCount() const;

    //==========================================================================
    /// Get a decoder by index
    ///
    /// @param index Zero-based decoder index
    /// @return Pointer to decoder at index, or nullptr if index out of range
    //==========================================================================
    IThumbnailDecoder* GetDecoder(size_t index) const;

    //==========================================================================
    /// Get all registered decoders
    ///
    /// @return Vector of decoder pointers
    //==========================================================================
    const std::vector<IThumbnailDecoder*>& GetAllDecoders() const;

    //==========================================================================
    /// Clear all registered decoders
    ///
    /// @note This will delete all decoder instances
    //==========================================================================
    void Clear();

    //==========================================================================
    /// Get statistics about registered decoders
    ///
    /// @param outTotalDecoders Output: Total number of decoders
    /// @param outImageDecoders Output: Number of image decoders
    /// @param outArchiveDecoders Output: Number of archive decoders
    /// @param outTotalExtensions Output: Total number of supported extensions
    //==========================================================================
    void GetStats(size_t* outTotalDecoders, size_t* outImageDecoders, size_t* outArchiveDecoders,
                  size_t* outTotalExtensions) const;

  private:
    // Non-copyable
    DecoderRegistry(const DecoderRegistry&) = delete;
    DecoderRegistry& operator=(const DecoderRegistry&) = delete;

    std::vector<IThumbnailDecoder*> m_decoders;
};

}  // namespace Engine
}  // namespace ExplorerLens
