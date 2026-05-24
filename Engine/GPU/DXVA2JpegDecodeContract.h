// Engine/GPU/DXVA2JpegDecodeContract.h
// ExplorerLens — DXVA2 hardware JPEG decode contract (ROADMAP v7.0 Phase 2)
// Sprint S312.
//
// Purpose:
//   Provides hardware-accelerated JPEG decode using the DirectX Video
//   Acceleration 2.0 API (IDirectXVideoDecoder).  DXVA2 JPEG decode is
//   supported on Intel HD Graphics (gen6+), NVIDIA (Maxwell+), and AMD
//   (RDNA+) at feature level D3D_FEATURE_LEVEL_10_1+.
//
//   On CI machines and virtual environments without DXVA2-capable GPUs,
//   IsHardwareSupported() returns false and Decode() returns HARDWARE_UNAVAILABLE.
//   The pipeline falls back to WIC or libjpeg-turbo automatically.
//
// Phase 2 stub: IsHardwareSupported() returns false; Decode() returns
// HARDWARE_UNAVAILABLE.  Wired in Sprint S355 (Dxva2SurfacePool).
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_DXVA2_JPEG_DECODE_CONTRACT_H
#define EXPLORERLENS_ENGINE_DXVA2_JPEG_DECODE_CONTRACT_H

#include <cstdint>
#include <cstddef>
#include <span>
#include <vector>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// DXVA2DecodeStatus
// ---------------------------------------------------------------------------
enum class DXVA2DecodeStatus : std::uint8_t {
    OK                    = 0,
    HARDWARE_UNAVAILABLE  = 1,   ///< No DXVA2-capable adapter (stub / CI)
    NOT_OPEN              = 2,   ///< Open() not called
    INVALID_BITSTREAM     = 3,   ///< Corrupt or unsupported JPEG bitstream
    SURFACE_ALLOC_FAILED  = 4,   ///< DXVA2 surface allocation error
    DECODE_FAILED         = 5,   ///< Hardware decoder returned error
    DEVICE_LOST           = 6,   ///< DXGI_ERROR_DEVICE_REMOVED
};

// ---------------------------------------------------------------------------
// DXVA2JpegDecodeResult
// ---------------------------------------------------------------------------
struct DXVA2JpegDecodeResult final {
    DXVA2DecodeStatus      status{ DXVA2DecodeStatus::HARDWARE_UNAVAILABLE };
    std::uint32_t          widthPixels{};
    std::uint32_t          heightPixels{};
    std::uint32_t          strideBytes{};
    std::vector<std::byte> pixels;           ///< BGRA-8 decoded output
    std::uint64_t          decodeTimeUs{};
};

// ---------------------------------------------------------------------------
// DXVA2JpegDecoder
// ---------------------------------------------------------------------------
class DXVA2JpegDecoder final {
public:
    DXVA2JpegDecoder() noexcept  = default;
    ~DXVA2JpegDecoder() noexcept = default;

    DXVA2JpegDecoder(const DXVA2JpegDecoder&)            = delete;
    DXVA2JpegDecoder& operator=(const DXVA2JpegDecoder&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────────

    /// Initialise DXVA2 decoder session.
    /// Returns false when IsHardwareSupported() == false.
    [[nodiscard]] bool Open() noexcept { return false; }  // Phase 2 stub

    void Close() noexcept {}

    [[nodiscard]] bool IsOpen() const noexcept { return false; }  // Phase 2 stub

    // ── Primary API ───────────────────────────────────────────────────────────

    /// Decode a JPEG bitstream using DXVA2 hardware acceleration.
    /// Returns HARDWARE_UNAVAILABLE when IsHardwareSupported() == false.
    [[nodiscard]] DXVA2JpegDecodeResult Decode(
        std::span<const std::byte> jpegBitstream) const noexcept
    {
        (void)jpegBitstream;
        return { .status = DXVA2DecodeStatus::HARDWARE_UNAVAILABLE };
    }

    // ── Capability ────────────────────────────────────────────────────────────

    /// True when a DXVA2-capable D3D10.1+ adapter is present and initialised.
    [[nodiscard]] static bool IsHardwareSupported() noexcept
    {
        return false;  // Phase 2 stub; wired in Sprint S355
    }

    // ── Constants ─────────────────────────────────────────────────────────────

    /// Maximum JPEG side length decodable via DXVA2 (hardware limit).
    static constexpr std::uint32_t kMaxDecodeSidePixels = 8192u;

    /// Minimum JPEG bitstream size (SOI marker + some data).
    static constexpr std::uint32_t kMinBitstreamBytes = 32u;

    /// DXVA2 requires feature level >= D3D_FEATURE_LEVEL_10_1.
    static constexpr std::uint32_t kRequiredFeatureLevelValue = 0xa100u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DXVA2_JPEG_DECODE_CONTRACT_H
