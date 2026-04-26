// Engine/Codec/IccProfileManager.h
// ExplorerLens — lcms2 ICC color profile pipeline wrapper (H32 / ROADMAP v7.0 Phase 3)
// Sprint S309.
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
