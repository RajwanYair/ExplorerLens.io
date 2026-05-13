// Engine/Codec/IccProfileManager.h
// ExplorerLens — ICC color profile pipeline via WIC IWICColorTransform
// Sprint S309.
//
// Purpose:
//   Raw pixel data decoded from camera/print/medical files often carries an
//   embedded ICC profile that must be applied before the bitmap is displayed.
//   Without it, wide-gamut images (Display P3, ProPhoto RGB, AdobeRGB) look
//   washed-out or over-saturated on sRGB monitors.
//
//   IccProfileManager:
//     1. Parses an embedded ICC profile header from a decoded file's metadata
//        (pure C++ — ICC v2/v4 spec §7.2, 128-byte profile header).
//     2. Builds a WIC IWICColorContext-based transform: source → display sRGB.
//     3. Applies the transform in-place on the decoded BGRA-8 pixel buffer.
//
//   Uses WIC IWICColorTransform (always available on Windows Vista+) via the
//   existing windowscodecs.lib linkage.  lcms2 can replace this in Sprint
//   ≥ S340 for wider gamut and proofing support.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_ICC_PROFILE_MANAGER_H
#define EXPLORERLENS_ENGINE_ICC_PROFILE_MANAGER_H

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <new>
#include <span>
#include <string>
#include <vector>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <wincodec.h>
#  include <objbase.h>
#  include <wrl/client.h>
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// IccTransformProfile — describes the intent of a colour transform
// ---------------------------------------------------------------------------
enum class IccTransformProfile : std::uint8_t {
    SOURCE   = 0,   ///< Source (embedded in decoded file)
    DISPLAY  = 1,   ///< Display output (sRGB IEC 61966-2-1)
    PROOFING = 2,   ///< Soft-proofing (print simulation — requires lcms2)
};

// ---------------------------------------------------------------------------
// IccProfileInfo — parsed metadata from an ICC profile byte stream
// ---------------------------------------------------------------------------
struct IccProfileInfo final {
    std::string   profileDescription;   ///< Human-readable profile name
    std::string   colorSpace;           ///< e.g. "RGB", "CMYK", "Lab"
    std::string   connectionSpace;      ///< Usually "XYZ" or "Lab"
    std::uint32_t profileSizeBytes{};   ///< Byte length of the raw ICC blob
    bool          isValid{ false };     ///< True if parsing succeeded
};

// ---------------------------------------------------------------------------
// IccOpaqueTransform — internal state; opaque to callers
// ---------------------------------------------------------------------------
struct IccOpaqueTransform final {
    std::vector<std::byte> sourceIcc;   ///< Copy of source ICC blob
    std::vector<std::byte> displayIcc;  ///< Copy of display ICC blob (empty = sRGB)
};

// ---------------------------------------------------------------------------
// IccProfileManager
// ---------------------------------------------------------------------------
class IccProfileManager final {
public:
    IccProfileManager() noexcept  = default;
    ~IccProfileManager() noexcept = default;

    IccProfileManager(const IccProfileManager&)            = delete;
    IccProfileManager& operator=(const IccProfileManager&) = delete;

    // ── Profile loading ───────────────────────────────────────────────────────

    /// Parse ICC profile bytes extracted from a decoded file's metadata.
    /// Reads the 128-byte ICC header per ICC v2/v4 spec §7.2.
    [[nodiscard]] IccProfileInfo
    ParseProfile(std::span<const std::byte> iccBytes) const noexcept
    {
        // ICC profile header is exactly 128 bytes.
        // Byte  0- 3: profile size (big-endian uint32)
        // Byte 16-19: colour space signature (e.g. 'RGB ', 'CMYK', 'Lab ')
        // Byte 20-23: profile connection space ('XYZ ', 'Lab ')
        // Byte 80-99: profile description tag offset (simplified parse)
        static constexpr std::size_t kIccHeaderSize = 128u;

        if (iccBytes.size() < kIccHeaderSize) {
            return {};
        }

        const auto* h = reinterpret_cast<const std::uint8_t*>(iccBytes.data());

        // Profile size (big-endian).
        const std::uint32_t profileSize =
            (static_cast<std::uint32_t>(h[0]) << 24)
            | (static_cast<std::uint32_t>(h[1]) << 16)
            | (static_cast<std::uint32_t>(h[2]) <<  8)
            |  static_cast<std::uint32_t>(h[3]);

        // Colour space (bytes 16–19).
        char cs[5]{};
        memcpy(cs, h + 16, 4);
        cs[4] = '\0';
        // Trim trailing spaces from the 4-byte signature.
        for (int i = 3; i >= 0 && cs[i] == ' '; --i) { cs[i] = '\0'; }

        // Profile connection space (bytes 20–23).
        char pcs[5]{};
        memcpy(pcs, h + 20, 4);
        pcs[4] = '\0';
        for (int i = 3; i >= 0 && pcs[i] == ' '; --i) { pcs[i] = '\0'; }

        IccProfileInfo info;
        info.profileSizeBytes = profileSize;
        info.colorSpace       = cs;
        info.connectionSpace  = pcs;
        info.isValid          = (profileSize > 0 && profileSize <= iccBytes.size());

        // Profile description: located via the 'desc' tag in the tag table.
        // Tag table starts at byte 128; each entry is 12 bytes.
        if (iccBytes.size() >= kIccHeaderSize + 4u) {
            const std::uint32_t tagCount =
                (static_cast<std::uint32_t>(h[128]) << 24)
                | (static_cast<std::uint32_t>(h[129]) << 16)
                | (static_cast<std::uint32_t>(h[130]) <<  8)
                |  static_cast<std::uint32_t>(h[131]);

            const std::size_t tagTableEnd =
                kIccHeaderSize + 4u + static_cast<std::size_t>(tagCount) * 12u;

            if (iccBytes.size() >= tagTableEnd) {
                for (std::uint32_t t = 0; t < tagCount; ++t) {
                    const std::size_t te = kIccHeaderSize + 4u + t * 12u;
                    char sig[5]{};
                    memcpy(sig, h + te, 4);
                    sig[4] = '\0';

                    if (memcmp(sig, "desc", 4) == 0) {
                        const std::uint32_t offset =
                            (static_cast<std::uint32_t>(h[te + 4]) << 24)
                            | (static_cast<std::uint32_t>(h[te + 5]) << 16)
                            | (static_cast<std::uint32_t>(h[te + 6]) <<  8)
                            |  static_cast<std::uint32_t>(h[te + 7]);
                        const std::uint32_t len =
                            (static_cast<std::uint32_t>(h[te + 8]) << 24)
                            | (static_cast<std::uint32_t>(h[te + 9]) << 16)
                            | (static_cast<std::uint32_t>(h[te + 10]) << 8)
                            |  static_cast<std::uint32_t>(h[te + 11]);

                        // 'mluc' / 'desc' tag body starts at offset+8 for
                        // ICC v4 multiLocalizedUnicodeType.
                        if (offset + 8u + 4u <= iccBytes.size() && len > 8u) {
                            const std::size_t strOff = offset + 8u;
                            const std::size_t strLen =
                                (std::min)(static_cast<std::size_t>(len - 8u),
                                           iccBytes.size() - strOff);
                            info.profileDescription.assign(
                                reinterpret_cast<const char*>(h + strOff),
                                strLen);
                            // Strip null bytes from the description.
                            info.profileDescription.erase(
                                std::remove(info.profileDescription.begin(),
                                            info.profileDescription.end(), '\0'),
                                info.profileDescription.end());
                        }
                        break;
                    }
                }
            }
        }

        return info;
    }

    // ── Transform creation ────────────────────────────────────────────────────

    /// Prepare a source→display colour transform (opaque handle).
    /// @param sourceIcc   Raw ICC bytes from the decoded file (empty = passthrough).
    /// @param displayIcc  Target display ICC bytes (empty = sRGB IEC 61966-2-1).
    /// @returns  Heap-allocated IccOpaqueTransform*; caller must pass to
    ///           DestroyTransform() when done.  Returns nullptr on allocation
    ///           failure.
    [[nodiscard]] void*
    CreateTransform(
        std::span<const std::byte> sourceIcc,
        std::span<const std::byte> displayIcc) const noexcept
    {
        if (sourceIcc.empty()) {
            return nullptr;   // No source profile → passthrough, no transform needed
        }

        auto* xform = new (std::nothrow) IccOpaqueTransform{};
        if (!xform) { return nullptr; }

        xform->sourceIcc.assign(sourceIcc.begin(), sourceIcc.end());
        xform->displayIcc.assign(displayIcc.begin(), displayIcc.end());
        return xform;
    }

    /// Release a transform handle obtained from CreateTransform().
    void DestroyTransform(void* transform) const noexcept
    {
        delete static_cast<IccOpaqueTransform*>(transform);
    }

    // ── In-place pixel conversion ─────────────────────────────────────────────

    /// Apply ICC transform to a BGRA-8 pixel buffer in-place using WIC.
    /// @param transform   Handle from CreateTransform(); no-op when nullptr.
    /// @param pixels      BGRA-8 pixel buffer (width * height * 4 bytes).
    /// @param pixelCount  Number of pixels (used to derive width for WIC bitmap).
    void ApplyTransform(
        void*                  transform,
        std::span<std::byte>   pixels,
        std::uint32_t          pixelCount) const noexcept
    {
        if (!transform || pixels.empty() || pixelCount == 0) { return; }

#ifdef _WIN32
        const auto* xform = static_cast<const IccOpaqueTransform*>(transform);
        if (xform->sourceIcc.empty()) { return; }

        using Microsoft::WRL::ComPtr;

        ComPtr<IWICImagingFactory> factory;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory,
                                    nullptr, CLSCTX_INPROC_SERVER,
                                    IID_PPV_ARGS(&factory)))) {
            return;
        }

        // Build source color context from the embedded ICC bytes.
        ComPtr<IWICColorContext> srcCtx;
        if (FAILED(factory->CreateColorContext(&srcCtx))) { return; }
        if (FAILED(srcCtx->InitializeFromMemory(
                reinterpret_cast<const BYTE*>(xform->sourceIcc.data()),
                static_cast<UINT>(xform->sourceIcc.size())))) {
            return;
        }

        // Build display color context (sRGB when displayIcc is empty).
        ComPtr<IWICColorContext> dstCtx;
        if (FAILED(factory->CreateColorContext(&dstCtx))) { return; }
        if (xform->displayIcc.empty()) {
            // Use the predefined sRGB space.
            dstCtx->InitializeFromExifColorSpace(1 /*sRGB*/);
        } else {
            if (FAILED(dstCtx->InitializeFromMemory(
                    reinterpret_cast<const BYTE*>(xform->displayIcc.data()),
                    static_cast<UINT>(xform->displayIcc.size())))) {
                return;
            }
        }

        // Create a WIC bitmap backed by the caller's pixel buffer.
        const std::uint32_t width = pixelCount;  // Treat as 1×N for transform
        ComPtr<IWICBitmap> bitmap;
        if (FAILED(factory->CreateBitmapFromMemory(
                width, 1u,
                GUID_WICPixelFormat32bppBGRA,
                width * 4u,
                static_cast<UINT>(pixels.size()),
                reinterpret_cast<BYTE*>(pixels.data()),
                &bitmap))) {
            return;
        }

        // Apply the color transform in-place.
        ComPtr<IWICColorTransform> colorXform;
        if (FAILED(factory->CreateColorTransformer(&colorXform))) { return; }
        if (FAILED(colorXform->Initialize(
                bitmap.Get(), srcCtx.Get(), dstCtx.Get(),
                GUID_WICPixelFormat32bppBGRA))) {
            return;
        }

        WICRect rect{ 0, 0, static_cast<INT>(width), 1 };
        colorXform->CopyPixels(&rect, width * 4u,
                               static_cast<UINT>(pixels.size()),
                               reinterpret_cast<BYTE*>(pixels.data()));
#else
        (void)transform; (void)pixels; (void)pixelCount;
#endif
    }

    // ── Capability query ─────────────────────────────────────────────────────

    /// True when ICC correction is available (WIC is always present on Windows).
    [[nodiscard]] static constexpr bool IsEnabled() noexcept
    {
#ifdef _WIN32
        return true;
#else
        return false;
#endif
    }

    /// Backend name string.
    [[nodiscard]] static constexpr const char* BackendName() noexcept
    {
        return "WIC";
    }
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_ICC_PROFILE_MANAGER_H

//
// Purpose:
//   Raw pixel data decoded from camera/print/medical files often carries an
//   embedded ICC profile that must be applied before the bitmap is displayed.
//   Without it, wide-gamut images (Display P3, ProPhoto RGB, AdobeRGB) look
//   washed-out or over-saturated on sRGB monitors.
//
//   IccProfileManager wraps lcms2 (Little Color Management System) to:
//     1. Parse an embedded ICC profile from a decoded file's metadata.
//     2. Build an lcms2 transform: source profile → display profile.
//     3. Apply the transform in-place on the decoded BGRA/RGB pixel buffer.
//
//   Phase 3 plan (ROADMAP v7.0 §8):
//     - vcpkg: add "lcms2" to vcpkg.json when Phase 3 starts.
//     - Replace #if EXPLORERLENS_ICC_ENABLED 0 guard with 1.
//     - Implement CreateTransform() / ApplyTransform() bodies.
//     - Hook into DecodePipeline post-decode colour correction stage.
//
// STATUS: DISABLED — Phase 3 stub.  All methods are no-ops.
//         Do not enable until lcms2 is built and linked (Sprint ≥ S340).
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_ICC_PROFILE_MANAGER_H
#define EXPLORERLENS_ENGINE_ICC_PROFILE_MANAGER_H

#include <cstdint>
#include <cstddef>
#include <span>
#include <string>

// Phase 3 enable gate — flip to 1 when lcms2 is linked.
#define EXPLORERLENS_ICC_ENABLED 0

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// IccTransformProfile — describes the intent of a colour transform
// ---------------------------------------------------------------------------
enum class IccTransformProfile : std::uint8_t {
    SOURCE   = 0,   ///< Source (embedded in decoded file)
    DISPLAY  = 1,   ///< Display output (e.g. sRGB IEC 61966-2-1)
    PROOFING = 2,   ///< Soft-proofing (print simulation)
};

// ---------------------------------------------------------------------------
// IccProfileInfo — parsed metadata from an ICC profile byte stream
// ---------------------------------------------------------------------------
struct IccProfileInfo final {
    std::string  profileDescription;   ///< Human-readable profile name
    std::string  colorSpace;           ///< e.g. "RGB", "CMYK", "Lab"
    std::string  connectionSpace;      ///< Usually "XYZ" or "Lab"
    std::uint32_t profileSizeBytes{};  ///< Byte length of the raw ICC blob
    bool          isValid{ false };    ///< True if parsing succeeded
};

// ---------------------------------------------------------------------------
// IccProfileManager
// ---------------------------------------------------------------------------
// Phase 3 stub.  All operations are no-ops until EXPLORERLENS_ICC_ENABLED=1.
//
class IccProfileManager final {
public:
    IccProfileManager() noexcept  = default;
    ~IccProfileManager() noexcept = default;

    // Non-copyable, non-movable.
    IccProfileManager(const IccProfileManager&)            = delete;
    IccProfileManager& operator=(const IccProfileManager&) = delete;

    // ── Profile loading ───────────────────────────────────────────────────────

    /// Parse ICC profile bytes extracted from a decoded file's metadata.
    /// @returns  IccProfileInfo; isValid=false if disabled or parsing fails.
    [[nodiscard]] IccProfileInfo
    ParseProfile(std::span<const std::byte> iccBytes) const noexcept
    {
#if EXPLORERLENS_ICC_ENABLED
        // Phase 3: call cmsOpenProfileFromMem(iccBytes.data(), iccBytes.size())
        //          and populate IccProfileInfo.
        (void)iccBytes;
        return {};
#else
        (void)iccBytes;
        return {};  // stub
#endif
    }

    // ── Transform creation ────────────────────────────────────────────────────

    /// Prepare a source→display colour transform.
    /// @param sourceIcc   Raw ICC bytes from the decoded file (may be empty).
    /// @param displayIcc  Raw ICC bytes of the display profile (may be empty;
    ///                    defaults to sRGB when empty).
    /// @returns  Opaque transform handle (void*); nullptr when disabled.
    [[nodiscard]] void*
    CreateTransform(
        std::span<const std::byte> sourceIcc,
        std::span<const std::byte> displayIcc) const noexcept
    {
#if EXPLORERLENS_ICC_ENABLED
        // Phase 3: cmsCreateTransform(src, TYPE_BGRA_8, dst, TYPE_BGRA_8,
        //                             INTENT_PERCEPTUAL, 0)
        (void)sourceIcc; (void)displayIcc;
        return nullptr;
#else
        (void)sourceIcc; (void)displayIcc;
        return nullptr;  // stub
#endif
    }

    /// Release a transform handle obtained from CreateTransform().
    void DestroyTransform(void* transform) const noexcept
    {
#if EXPLORERLENS_ICC_ENABLED
        // Phase 3: cmsDeleteTransform(transform)
        (void)transform;
#else
        (void)transform;
#endif
    }

    // ── In-place pixel conversion ─────────────────────────────────────────────

    /// Apply transform to a BGRA-8 pixel buffer in-place.
    /// @param transform   Handle from CreateTransform(); ignored when nullptr.
    /// @param pixels      BGRA pixel buffer (width * height * 4 bytes).
    /// @param pixelCount  Number of pixels.
    void ApplyTransform(
        void*                  transform,
        std::span<std::byte>   pixels,
        std::uint32_t          pixelCount) const noexcept
    {
#if EXPLORERLENS_ICC_ENABLED
        // Phase 3: cmsDoTransform(transform, pixels.data(), pixels.data(),
        //                         pixelCount)
        (void)transform; (void)pixels; (void)pixelCount;
#else
        (void)transform; (void)pixels; (void)pixelCount;
#endif
    }

    // ── Capability query ─────────────────────────────────────────────────────

    /// True when lcms2 is compiled in and ICC correction is active.
    [[nodiscard]] static constexpr bool IsEnabled() noexcept
    {
        return EXPLORERLENS_ICC_ENABLED != 0;
    }

    /// lcms2 major version this was compiled against (0 = stub).
    [[nodiscard]] static constexpr int BackendVersion() noexcept
    {
#if EXPLORERLENS_ICC_ENABLED
        return LCMS_VERSION;   // defined by lcms2.h
#else
        return 0;
#endif
    }
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_ICC_PROFILE_MANAGER_H
