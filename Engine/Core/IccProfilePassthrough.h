// Engine/Core/IccProfilePassthrough.h
// ExplorerLens — ICC profile passthrough contract: (pixels, icc_bytes) pair (H3 + H30 / ROADMAP v8.0 Phase 3 kickoff)
// Sprint S340.
//
// Purpose:
//   ROADMAP H3 (wide-gamut colour fidelity) and H30 (Display P3 + AdobeRGB
//   passthrough) require that each decoder emits not just BGRA pixels but also
//   the embedded ICC profile bytes.  The ICC-aware display path (lcms2 in Phase 3)
//   can then transform the pixels to the monitor's colour space, delivering
//   correct wide-gamut colours in Explorer thumbnails.
//
//   IccProfilePassthrough defines the **data contract** — the struct and enums
//   that bind decoders to the display pipeline.  It does NOT perform colour
//   transforms (that requires lcms2, scheduled for Phase 3 sprint S355+).
//
//   The pattern:
//     struct DecoderOutput {
//         HBITMAP            hbmp;      // decoded pixels
//         IccProfileData     icc;       // embedded profile (may be empty)
//     };
//
//   IccProfilePassthrough::ExtractFromWic() can pull the ICC profile bytes
//   from an `IWICBitmapDecoder` without needing a separate EXIF parser.
//
//   Integration checklist (Phase 3):
//     1. Each decoder fills `IccProfileData` in its return struct
//     2. Pipeline::ThumbnailRenderer checks `icc.IsValid()`
//     3. If valid and not sRGB, feed through lcms2 transform before returning HBITMAP
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_ICC_PROFILE_PASSTHROUGH_H
#define EXPLORERLENS_ENGINE_ICC_PROFILE_PASSTHROUGH_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string_view>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <wincodec.h>
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// IccPassthroughStatus
// ---------------------------------------------------------------------------
enum class IccPassthroughStatus : std::uint8_t {
    OK              = 0,  ///< Profile extracted and appears valid
    NOT_PRESENT     = 1,  ///< Image has no embedded ICC profile (assume sRGB)
    EMPTY_BYTES     = 2,  ///< Profile field exists but has zero bytes
    TOO_SMALL       = 3,  ///< Profile bytes below ICC minimum (128 bytes)
    TOO_LARGE       = 4,  ///< Profile exceeds kMaxProfileBytes
    INVALID_HEADER  = 5,  ///< First 4 bytes don't match expected profile size
    EXTRACT_FAILED  = 6,  ///< WIC COM call returned failure
    PASSTHROUGH_OFF = 7,  ///< Feature disabled in config
};

// ---------------------------------------------------------------------------
// IccColorSpaceId — simplified colour space hint (not full ICC intent)
// ---------------------------------------------------------------------------
enum class IccColorSpaceId : std::uint8_t {
    UNKNOWN       = 0,
    SRGB          = 1,  ///< Standard sRGB (no transform needed for sRGB monitor)
    DISPLAY_P3    = 2,  ///< Apple Display P3
    ADOBE_RGB     = 3,  ///< Adobe RGB (1998)
    PROPHOTO_RGB  = 4,  ///< ProPhoto / ROMM RGB
    REC2020       = 5,  ///< BT.2020 / UHDTV
    CMYK          = 6,  ///< CMYK — needs special handling
    GRAY          = 7,  ///< Grayscale ICC
    OTHER         = 8,  ///< Present but unrecognised
};

// ---------------------------------------------------------------------------
// IccProfileData — the passthrough payload emitted by each decoder
// ---------------------------------------------------------------------------
struct IccProfileData final {
    std::vector<std::uint8_t> profileBytes;         ///< Raw ICC bytes (empty = sRGB assumed)
    IccColorSpaceId           colorSpaceId{};        ///< Hinted colour space (best-effort)
    IccPassthroughStatus      status{ IccPassthroughStatus::NOT_PRESENT };

    [[nodiscard]] bool IsValid() const noexcept
    {
        return status == IccPassthroughStatus::OK &&
               profileBytes.size() >= kMinProfileBytes;
    }

    [[nodiscard]] bool IsEmpty() const noexcept { return profileBytes.empty(); }

    [[nodiscard]] bool NeedsTransform() const noexcept
    {
        return IsValid() && colorSpaceId != IccColorSpaceId::SRGB;
    }
};

// ---------------------------------------------------------------------------
// IccProfilePassthrough
// ---------------------------------------------------------------------------
class IccProfilePassthrough final {
public:
    IccProfilePassthrough() = delete;

    /// Minimum valid ICC profile size (ICC.1 header = 128 bytes)
    static constexpr std::size_t kMinProfileBytes = 128u;

    /// Maximum accepted profile size (16 MB — pathological profiles rejected)
    static constexpr std::size_t kMaxProfileBytes = 16u * 1024u * 1024u;

    /// sRGB IEC 61966-2.1 profile description string (in-memory identification)
    static constexpr std::string_view kSrgbDescriptor = "IEC 61966-2.1";
    static constexpr std::string_view kDisplayP3Descriptor = "Display P3";
    static constexpr std::string_view kAdobeRgbDescriptor  = "Adobe RGB (1998)";

    // ------------------------------------------------------------------
    // ExtractFromWic() — pull ICC profile bytes from IWICBitmapDecoder
    // ------------------------------------------------------------------
    [[nodiscard]]
    static IccProfileData ExtractFromWic(
#ifdef _WIN32
        IWICBitmapDecoder* pDecoder
#else
        void* /*pDecoder*/
#endif
    ) noexcept
    {
        IccProfileData result{};

#ifdef _WIN32
        if (!pDecoder) {
            result.status = IccPassthroughStatus::EXTRACT_FAILED;
            return result;
        }

        IWICBitmapFrameDecode* pFrame = nullptr;
        if (FAILED(pDecoder->GetFrame(0u, &pFrame))) {
            result.status = IccPassthroughStatus::EXTRACT_FAILED;
            return result;
        }

        IWICColorContext* pCtx = nullptr;
        UINT ctxCount = 0u;
        HRESULT hr = pFrame->GetColorContexts(1u, &pCtx, &ctxCount);

        if (FAILED(hr) || ctxCount == 0u || !pCtx) {
            pFrame->Release();
            result.status = IccPassthroughStatus::NOT_PRESENT;
            return result;
        }

        UINT profileSize = 0u;
        hr = pCtx->GetProfileBytes(0u, nullptr, &profileSize);
        if (FAILED(hr) || profileSize == 0u) {
            pCtx->Release();
            pFrame->Release();
            result.status = IccPassthroughStatus::EMPTY_BYTES;
            return result;
        }
        if (profileSize < static_cast<UINT>(kMinProfileBytes)) {
            pCtx->Release();
            pFrame->Release();
            result.status = IccPassthroughStatus::TOO_SMALL;
            return result;
        }
        if (profileSize > static_cast<UINT>(kMaxProfileBytes)) {
            pCtx->Release();
            pFrame->Release();
            result.status = IccPassthroughStatus::TOO_LARGE;
            return result;
        }

        result.profileBytes.resize(profileSize);
        hr = pCtx->GetProfileBytes(profileSize, result.profileBytes.data(), &profileSize);

        pCtx->Release();
        pFrame->Release();

        if (FAILED(hr)) {
            result.profileBytes.clear();
            result.status = IccPassthroughStatus::EXTRACT_FAILED;
            return result;
        }

        result.status = IccPassthroughStatus::OK;
        result.colorSpaceId = HintColorSpaceFromProfile(result.profileBytes);
        return result;
#else
        result.status = IccPassthroughStatus::NOT_PRESENT;
        return result;
#endif
    }

    // ------------------------------------------------------------------
    // Validate() — check that profileBytes look like a real ICC profile
    // ------------------------------------------------------------------
    [[nodiscard]]
    static IccPassthroughStatus Validate(
        const std::vector<std::uint8_t>& profileBytes) noexcept
    {
        if (profileBytes.empty())
            return IccPassthroughStatus::EMPTY_BYTES;

        if (profileBytes.size() < kMinProfileBytes)
            return IccPassthroughStatus::TOO_SMALL;

        if (profileBytes.size() > kMaxProfileBytes)
            return IccPassthroughStatus::TOO_LARGE;

        // ICC spec: first 4 bytes = big-endian profile size
        const std::uint32_t embeddedSize =
            (static_cast<std::uint32_t>(profileBytes[0]) << 24u) |
            (static_cast<std::uint32_t>(profileBytes[1]) << 16u) |
            (static_cast<std::uint32_t>(profileBytes[2]) <<  8u) |
             static_cast<std::uint32_t>(profileBytes[3]);

        if (embeddedSize != static_cast<std::uint32_t>(profileBytes.size()))
            return IccPassthroughStatus::INVALID_HEADER;

        return IccPassthroughStatus::OK;
    }

    // ------------------------------------------------------------------
    // MakeSrgb() — return a "known sRGB" passthrough result (no bytes;
    //   caller treats absent profile as sRGB per ICC spec)
    // ------------------------------------------------------------------
    [[nodiscard]]
    static IccProfileData MakeSrgb() noexcept
    {
        IccProfileData result{};
        result.status       = IccPassthroughStatus::NOT_PRESENT;
        result.colorSpaceId = IccColorSpaceId::SRGB;
        return result;
    }

private:
    // Best-effort colour space hinting by scanning description tag at fixed offsets
    [[nodiscard]]
    static IccColorSpaceId HintColorSpaceFromProfile(
        const std::vector<std::uint8_t>& bytes) noexcept
    {
        if (bytes.size() < kMinProfileBytes) return IccColorSpaceId::UNKNOWN;

        // ICC tag count at offset 128 (4 bytes big-endian)
        // Real identification requires parsing the tag table — out of scope for Phase 2.
        // Phase 3 will do a proper 'desc' tag scan.  For now, return UNKNOWN unless
        // we detect the well-known "GRAY " colour space signature at offset 16.
        if (bytes.size() >= 20u) {
            const char csig[5] = {
                static_cast<char>(bytes[16]),
                static_cast<char>(bytes[17]),
                static_cast<char>(bytes[18]),
                static_cast<char>(bytes[19]),
                '\0'
            };
            std::string_view sig{ csig, 4 };
            if (sig == "GRAY") return IccColorSpaceId::GRAY;
            if (sig == "CMYK") return IccColorSpaceId::CMYK;
        }
        return IccColorSpaceId::OTHER;
    }
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_ICC_PROFILE_PASSTHROUGH_H
