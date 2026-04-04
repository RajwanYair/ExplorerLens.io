#pragma once
//==============================================================================
// FarbfeldDecoder.h — Farbfeld Image Decoder
// ExplorerLens Engine v8.4.0 — Easy Format Wins
//
// Decodes Farbfeld (.ff) images — a simple lossless image format with
// 8-byte magic ("farbfeld"), 32-bit width/height, and raw 16-bit RGBA pixels.
// Spec: https://tools.suckless.org/farbfeld/
//==============================================================================

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include "../Core/IThumbnailDecoder.h"

namespace ExplorerLens {
namespace Engine {

/// Farbfeld (.ff) image decoder.
///
/// File format (big-endian):
/// Bytes 0-7: Magic "farbfeld"
/// Bytes 8-11: Width (uint32_t BE)
/// Bytes 12-15: Height (uint32_t BE)
/// Bytes 16+: Pixel data (16-bit RGBA per channel, big-endian)
///
/// Each pixel = 8 bytes (R16 G16 B16 A16 in big-endian).
/// Total data size = width * height * 8 bytes.
class FarbfeldDecoder
{
  public:
    FarbfeldDecoder() = default;
    ~FarbfeldDecoder() = default;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) const;
    HRESULT Decode(const wchar_t* filePath, uint32_t requestedSize, HBITMAP& hBitmap);
    const wchar_t* GetName() const
    {
        return L"FarbfeldDecoder";
    }
    bool SupportsGPU() const
    {
        return false;
    }
    bool IsArchiveDecoder() const
    {
        return false;
    }

    static constexpr const wchar_t* s_extensions[] = {L".ff"};
    static constexpr uint32_t s_extensionCount = 1;

    const wchar_t** GetSupportedExtensions() const
    {
        return const_cast<const wchar_t**>(s_extensions);
    }
    uint32_t GetExtensionCount() const
    {
        return s_extensionCount;
    }

  private:
    static constexpr char MAGIC[8] = {'f', 'a', 'r', 'b', 'f', 'e', 'l', 'd'};

    /// Read big-endian uint32
    static uint32_t ReadBE32(const uint8_t* p)
    {
        return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
    }

    /// Read big-endian uint16
    static uint16_t ReadBE16(const uint8_t* p)
    {
        return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
