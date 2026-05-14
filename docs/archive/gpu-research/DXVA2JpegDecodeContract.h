// Engine/GPU/DXVA2JpegDecodeContract.h
// ExplorerLens — DXVA2 JPEG hardware-decode contract (ROADMAP v7.0 Phase 2)
// Sprint S312.
//
// Purpose:
//   DXVA2 (DirectX Video Acceleration 2) enables GPU-accelerated JPEG decode
//   on Intel/AMD/NVIDIA integrated graphics, reducing CPU load significantly:
//
//     CPU libjpeg-turbo:  ~4-8 ms / 4K JPEG
//     DXVA2 HW decode:    ~0.5-1.5 ms / 4K JPEG  (if IHV driver supports JPEG)
//
//   This header defines the contract/stub API.  Phase 2 implementation wires
//   the actual DXVA2 decoder service and Direct3D surface allocation.
//
// Prerequisites:
//   - D3D11DeviceManager::IsReady() == true
//   - Feature level >= D3D11DeviceManager::kMinDXVA2FeatureLevel (FL_10_1)
//   - IHV driver must advertise DXVA2_ModeMPEG2_VLD or JPEG GUID
//
// Phase 2 wiring (Sprint ≥ S330):
//   1. Probe IDirectXVideoDecoderService for DXVA_CODECGuid_JPEG.
//   2. Create IDirect3DSurface9 decode targets.
//   3. Submit bitstream via DXVA2_DecodeBufferPicture.
//   4. Readback to system memory via IDirect3DSurface9::LockRect.
//   Falls back to libjpeg-turbo (S318) when hardware is unavailable.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_DXVA2_JPEG_DECODE_CONTRACT_H
#define EXPLORERLENS_ENGINE_DXVA2_JPEG_DECODE_CONTRACT_H

#include <cstdint>
#include <span>
#include <vector>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// DXVA2DecodeStatus
// ---------------------------------------------------------------------------
enum class DXVA2DecodeStatus : std::uint8_t {
    OK                   = 0,
    HARDWARE_UNAVAILABLE = 1,  ///< No DXVA2 JPEG GUID on this adapter
    DEVICE_NOT_READY     = 2,  ///< D3D11DeviceManager not initialised
    JPEG_NOT_SUPPORTED   = 3,  ///< Driver present but no JPEG decode GUID
    DECODE_FAILURE       = 4,  ///< IHV decode error; fall back to SW
    INVALID_BITSTREAM    = 5,  ///< Input is not a valid JFIF bitstream
    OUTPUT_TOO_SMALL     = 6,  ///< Caller-provided output buffer insufficient
};

// ---------------------------------------------------------------------------
// DXVA2JpegDecodeConfig
// ---------------------------------------------------------------------------
struct DXVA2JpegDecodeConfig final {
    // Maximum width/height this decoder session accepts (set at session open).
    // Larger frames require a new session.
    std::uint32_t maxWidthPixels  = 4096u;
    std::uint32_t maxHeightPixels = 4096u;

    // Output pixel format:
    //   0 = BGRA-8 (default, matches GDI+ DIBSection)
    //   1 = RGB-24  (LibRaw / libjpeg-turbo fallback format)
    std::uint8_t outputFormat = 0u;

    // Maximum surfaces to preallocate (amortises surface creation cost)
    std::uint32_t surfacePoolSize = 4u;
};

// ---------------------------------------------------------------------------
// DXVA2JpegDecodeResult
// ---------------------------------------------------------------------------
struct DXVA2JpegDecodeResult final {
    DXVA2DecodeStatus     status{ DXVA2DecodeStatus::HARDWARE_UNAVAILABLE };
    std::uint32_t         widthPixels{};
    std::uint32_t         heightPixels{};
    std::uint32_t         strideBytes{};          ///< Row stride in output buffer
    std::vector<std::byte> pixels;                ///< Decoded BGRA/RGB pixels
    std::uint64_t         decodeTimeUs{};         ///< Measured wall-clock µs
};

// ---------------------------------------------------------------------------
// DXVA2JpegDecoder
// ---------------------------------------------------------------------------
// Phase 2 stub — all operations return HARDWARE_UNAVAILABLE until the
// full DXVA2 decoder surface and bitstream pipeline is implemented.
//
class DXVA2JpegDecoder final {
public:
    DXVA2JpegDecoder() noexcept  = default;
    ~DXVA2JpegDecoder() noexcept = default;

    DXVA2JpegDecoder(const DXVA2JpegDecoder&)            = delete;
    DXVA2JpegDecoder& operator=(const DXVA2JpegDecoder&) = delete;

    // ── Capability probe ──────────────────────────────────────────────────────

    /// True when DXVA2 JPEG decode is supported on the current adapter.
    /// Phase 2 stub: always returns false until driver probe is wired.
    [[nodiscard]] static bool IsHardwareSupported() noexcept { return false; }

    /// DXVA2 GUID string for JPEG decode (used in driver capability tables).
    static constexpr const char* kDXVA2JpegGuid =
        "{A0C7EB99-B57A-4D3C-8A98-D3B8B8A73BC0}";

    // ── Session lifecycle ────────────────────────────────────────────────────

    /// Open a decode session for JPEGs up to maxWidth × maxHeight.
    /// @returns false (stub); true when hardware is available in Phase 2.
    [[nodiscard]] bool Open(
        const DXVA2JpegDecodeConfig& cfg = DXVA2JpegDecodeConfig{}) noexcept
    {
        (void)cfg;
        return false;   // Phase 2 stub
    }

    void Close() noexcept {}

    [[nodiscard]] bool IsOpen() const noexcept { return false; }

    // ── Decode ────────────────────────────────────────────────────────────────

    /// Decode a JFIF bitstream to BGRA/RGB pixels.
    /// Falls back to DXVA2DecodeStatus::HARDWARE_UNAVAILABLE when stubbed.
    [[nodiscard]] DXVA2JpegDecodeResult
    Decode(std::span<const std::byte> jpegBitstream) const noexcept
    {
        (void)jpegBitstream;
        return DXVA2JpegDecodeResult{
            .status = DXVA2DecodeStatus::HARDWARE_UNAVAILABLE
        };
    }

    // ── Constants ─────────────────────────────────────────────────────────────

    /// Maximum JPEG side length DXVA2 can handle (driver limit, typ. 8192)
    static constexpr std::uint32_t kMaxDecodeSidePixels = 8192u;

    /// Minimum valid JPEG bitstream length (SoI + SoF + minimal markers)
    static constexpr std::uint32_t kMinBitstreamBytes = 32u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DXVA2_JPEG_DECODE_CONTRACT_H
