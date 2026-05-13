// =============================================================================
// ExplorerLens Engine — ThumbnailStreamSerializer.h
// Sprint S360 | ROADMAP v8.0 Phase 3/4 (Shell Output Quality)
// Converts a decoded BGRA pixel buffer into a Windows HBITMAP / IStream
// suitable for return from IThumbnailProvider::GetThumbnail().
//
// The serializer performs:
//   1. Premultiplied alpha conversion (BGRA straight → BGRA premul)
//   2. DIB section allocation (CreateDIBSection or CreateCompatibleBitmap)
//   3. GDI+ PNG re-encode for IStream output path
//   4. HBITMAP ownership transfer (caller frees via DeleteObject)
//
// This closes the last step of the shell thumbnail pipeline:
//   Decoder → CpuLanczosResizer → ThumbnailStreamSerializer → COM return
// =============================================================================
#pragma once

#include <cstdint>
#include <span>

#ifndef EXPLORERLENS_ENGINE_THUMBNAILSTREAMSERIALIZER_H
#define EXPLORERLENS_ENGINE_THUMBNAILSTREAMSERIALIZER_H

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// StreamSerializeStatus — result of one serialization call
// ---------------------------------------------------------------------------
enum class StreamSerializeStatus : uint8_t {
    OK               = 0,
    NULL_PIXELS      = 1,  ///< Source pixel pointer is null
    ZERO_DIMENSION   = 2,  ///< Width or height is 0
    DIMENSION_TOO_LARGE = 3, ///< Exceeds kMaxSerializeWidth/Height
    ALLOC_FAIL       = 4,  ///< GDI DIB section or GDI+ bitmap alloc failed
    ENCODE_FAIL      = 5,  ///< GDI+ PNG encode returned error
    STREAM_WRITE_FAIL = 6, ///< IStream Write() returned error
    NULL_STREAM      = 7,  ///< Output IStream pointer is null
    NOT_WIN32        = 8,  ///< Non-Windows build
    PREMUL_OVERFLOW  = 9,  ///< Premultiplied alpha value > 255 (data error)
};

// ---------------------------------------------------------------------------
// ThumbnailPixelFormat — source pixel layout
// ---------------------------------------------------------------------------
enum class ThumbnailPixelFormat : uint8_t {
    BGRA8_STRAIGHT   = 0,  ///< B G R A — straight alpha (most decoders output this)
    BGRA8_PREMUL     = 1,  ///< B G R A — premultiplied alpha (Direct2D, WIC output)
    RGBA8_STRAIGHT   = 2,
    BGR8             = 3,  ///< No alpha channel
};

// ---------------------------------------------------------------------------
// ThumbnailStreamConfig — serialization policy
// ---------------------------------------------------------------------------
struct ThumbnailStreamConfig final {
    ThumbnailPixelFormat srcFormat{ThumbnailPixelFormat::BGRA8_STRAIGHT};
    bool   premultiplyAlpha{true};    ///< Convert to premul before DIB creation
    bool   encodeAsPng{false};        ///< Write PNG to IStream (default: return HBITMAP)
    uint32_t pngQuality{90u};         ///< GDI+ PNG quality hint (0–100)
    bool   flipVertical{false};       ///< Flip Y axis for bottom-up DIB compatibility

    [[nodiscard]] static ThumbnailStreamConfig Default() noexcept {
        return ThumbnailStreamConfig{};
    }

    [[nodiscard]] static ThumbnailStreamConfig ForShellReturn() noexcept {
        ThumbnailStreamConfig c;
        c.srcFormat       = ThumbnailPixelFormat::BGRA8_STRAIGHT;
        c.premultiplyAlpha = true;
        c.encodeAsPng      = false;
        c.flipVertical     = false;
        return c;
    }
};

// ---------------------------------------------------------------------------
// ThumbnailStreamResult — output of one serialization call
// ---------------------------------------------------------------------------
struct ThumbnailStreamResult final {
    StreamSerializeStatus status{StreamSerializeStatus::NOT_WIN32};
    void*    hBitmap{nullptr};    ///< HBITMAP (caller must DeleteObject) — non-null on OK
    uint32_t width{0};
    uint32_t height{0};
    uint64_t streamBytes{0};      ///< Bytes written to IStream (encodeAsPng only)

    [[nodiscard]] bool Success()    const noexcept { return status == StreamSerializeStatus::OK; }
    [[nodiscard]] bool HasBitmap()  const noexcept { return hBitmap != nullptr; }
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kMaxSerializeWidth    = 8192u;
static constexpr uint32_t kMaxSerializeHeight   = 8192u;
static constexpr uint32_t kBytesPerPixelBgra    = 4u;
static constexpr uint32_t kThumbnailStride(uint32_t w) noexcept {
    // DWORD-aligned row stride for DIB sections
    return (w * kBytesPerPixelBgra + 3u) & ~3u;
}

// ---------------------------------------------------------------------------
// ThumbnailStreamSerializer — converts pixel buffer to HBITMAP / IStream
// ---------------------------------------------------------------------------
class ThumbnailStreamSerializer final {
public:
    ThumbnailStreamSerializer() = delete;

    /// Convert a BGRA pixel buffer to an HBITMAP for IThumbnailProvider return.
    /// @param pixels   Source pixel data
    /// @param pixelCount Width * Height pixels
    /// @param width    Image width in pixels
    /// @param height   Image height in pixels
    /// @param cfg      Serialization options
    [[nodiscard]] static ThumbnailStreamResult ToHBitmap(
        const uint8_t* pixels,
        uint32_t       pixelCount,
        uint32_t       width,
        uint32_t       height,
        const ThumbnailStreamConfig& cfg = ThumbnailStreamConfig::ForShellReturn()) noexcept;

    /// std::span overload for ergonomic usage
    [[nodiscard]] static ThumbnailStreamResult ToHBitmap(
        std::span<const uint8_t> pixels,
        uint32_t width,
        uint32_t height,
        const ThumbnailStreamConfig& cfg = ThumbnailStreamConfig::ForShellReturn()) noexcept
    {
        return ToHBitmap(pixels.data(),
                         static_cast<uint32_t>(pixels.size() / kBytesPerPixelBgra),
                         width, height, cfg);
    }

    /// Encode a pixel buffer to PNG and write to an IStream.
    /// @param pStream   IStream* (cast to void* for COM-free header)
    [[nodiscard]] static ThumbnailStreamResult ToPngStream(
        const uint8_t* pixels,
        uint32_t       width,
        uint32_t       height,
        void*          pStream,
        const ThumbnailStreamConfig& cfg = ThumbnailStreamConfig::Default()) noexcept;

    /// Premultiply alpha in-place for a BGRA8 buffer.
    /// Returns false if any sample exceeds source alpha (data integrity error).
    static bool PremultiplyAlpha(
        uint8_t* pixels,
        uint32_t pixelCount) noexcept;

    /// Reverse premultiplied alpha (approximate un-premul).
    static void UnpremultiplyAlpha(
        uint8_t* pixels,
        uint32_t pixelCount) noexcept;
};

// ---------------------------------------------------------------------------
// Inline non-Windows stubs
// ---------------------------------------------------------------------------
#ifndef _WIN32

inline ThumbnailStreamResult ThumbnailStreamSerializer::ToHBitmap(
    const uint8_t* /*pixels*/,
    uint32_t /*pixelCount*/,
    uint32_t /*width*/,
    uint32_t /*height*/,
    const ThumbnailStreamConfig& /*cfg*/) noexcept
{
    ThumbnailStreamResult r;
    r.status = StreamSerializeStatus::NOT_WIN32;
    return r;
}

inline ThumbnailStreamResult ThumbnailStreamSerializer::ToPngStream(
    const uint8_t* /*pixels*/,
    uint32_t /*width*/,
    uint32_t /*height*/,
    void*    /*pStream*/,
    const ThumbnailStreamConfig& /*cfg*/) noexcept
{
    ThumbnailStreamResult r;
    r.status = StreamSerializeStatus::NOT_WIN32;
    return r;
}

inline bool ThumbnailStreamSerializer::PremultiplyAlpha(
    uint8_t* /*pixels*/,
    uint32_t /*pixelCount*/) noexcept
{
    return true; // no-op
}

inline void ThumbnailStreamSerializer::UnpremultiplyAlpha(
    uint8_t* /*pixels*/,
    uint32_t /*pixelCount*/) noexcept {}

#endif // !_WIN32

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_THUMBNAILSTREAMSERIALIZER_H
