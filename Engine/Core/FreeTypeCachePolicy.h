// Engine/Core/FreeTypeCachePolicy.h
#pragma once

// FreeTypeCachePolicy — FreeType FTC_Manager cache sizing and eviction policy (S364)
//
// FreeType's internal FTC_Manager cache requires explicit configuration of maximum
// face slots, size slots, and bitmap cache entries.  This header defines the policy
// types consumed by FreeTypeRenderer (Engine/Core/FreeTypeRenderer.h) so the cache
// parameters can be tuned per deployment scenario (shell extension vs. batch mode).
//
// Phase 3 exit criterion: FreeType font preview for TTF/OTF (ROADMAP §3).
//
// Usage:
//   FreeTypeCachePolicy policy = FreeTypeCachePolicy::ForShellExtension();
//   // Pass to FreeTypeRenderer::Initialize(config, policy)

#ifndef EXPLORERLENS_ENGINE_FREETYPECACHEPOLICY_H
#define EXPLORERLENS_ENGINE_FREETYPECACHEPOLICY_H

#include <cstdint>

namespace ExplorerLens::Engine {

/// Maximum FTC_Manager face-slot count before FreeType recycles the oldest face.
inline constexpr std::uint32_t kFtcMaxFaceSlots    = 8u;
/// Maximum FTC_Manager size-slot count.
inline constexpr std::uint32_t kFtcMaxSizeSlots    = 16u;
/// Maximum number of bitmaps held in the FTC_ImageCache.
inline constexpr std::uint32_t kFtcMaxCacheBytes   = 512u * 1024u; // 512 KB
/// Maximum number of distinct glyphs cached per typeface.
inline constexpr std::uint32_t kFtcMaxGlyphsPerFace = 128u;

// ---------------------------------------------------------------------------
// FreeTypeCacheStatus — result of cache operations
// ---------------------------------------------------------------------------
enum class FreeTypeCacheStatus : std::uint8_t {
    OK              = 0,
    NOT_INITIALISED = 1,  ///< FreeType library not yet initialised
    FACE_NOT_FOUND  = 2,  ///< Requested font file could not be opened
    GLYPH_NOT_FOUND = 3,  ///< Codepoint not present in font
    CACHE_FULL      = 4,  ///< FTC_Manager evicted the face mid-render
    INVALID_SIZE    = 5,  ///< Requested pixel size is 0 or > kFtcMaxPixelSize
    LIBRARY_ERROR   = 6,  ///< FT_Error propagated from FreeType
};

/// Maximum pixel size for a single font preview glyph render.
inline constexpr std::uint32_t kFtcMaxPixelSize = 512u;
/// Minimum pixel size (below this GDI renders faster).
inline constexpr std::uint32_t kFtcMinPixelSize = 8u;

// ---------------------------------------------------------------------------
// FreeTypeFaceEntry — a cached font-face descriptor used by FTC_Manager
// ---------------------------------------------------------------------------
struct FreeTypeFaceEntry {
    const wchar_t*  filePath   = nullptr;  ///< Font file path (must outlive the manager)
    std::uint32_t   faceIndex  = 0u;       ///< Face index within TTC / OTF collection
    std::uint32_t   pixelSize  = 24u;      ///< Requested render size in pixels
    bool            antiAlias  = true;     ///< Enable FT_LOAD_TARGET_NORMAL anti-alias
    bool            forceGreek = false;    ///< Force Greek alphabet sample for symbol fonts

    [[nodiscard]] bool IsValid() const noexcept {
        return filePath != nullptr
            && pixelSize >= kFtcMinPixelSize
            && pixelSize <= kFtcMaxPixelSize;
    }
};

// ---------------------------------------------------------------------------
// FreeTypeCachePolicy — per-scenario FTC_Manager sizing
// ---------------------------------------------------------------------------
struct FreeTypeCachePolicy {
    std::uint32_t maxFaceSlots    = kFtcMaxFaceSlots;
    std::uint32_t maxSizeSlots    = kFtcMaxSizeSlots;
    std::uint32_t maxCacheBytes   = kFtcMaxCacheBytes;
    std::uint32_t maxGlyphsPerFace = kFtcMaxGlyphsPerFace;
    bool          warmOnInit      = false; ///< Pre-cache ASCII glyphs for default face

    /// Default: balanced for interactive use.
    [[nodiscard]] static FreeTypeCachePolicy Default() noexcept {
        return FreeTypeCachePolicy{};
    }

    /// Minimal policy for LENSShell.dll (low-memory shell host).
    [[nodiscard]] static FreeTypeCachePolicy ForShellExtension() noexcept {
        FreeTypeCachePolicy p{};
        p.maxFaceSlots     = 4u;
        p.maxSizeSlots     = 8u;
        p.maxCacheBytes    = 128u * 1024u;  // 128 KB
        p.maxGlyphsPerFace = 64u;
        p.warmOnInit       = false;
        return p;
    }

    /// Generous policy for lens.exe batch font preview generation.
    [[nodiscard]] static FreeTypeCachePolicy ForBatchMode() noexcept {
        FreeTypeCachePolicy p{};
        p.maxFaceSlots     = 32u;
        p.maxSizeSlots     = 64u;
        p.maxCacheBytes    = 4u * 1024u * 1024u; // 4 MB
        p.maxGlyphsPerFace = 512u;
        p.warmOnInit       = true;
        return p;
    }
};

// ---------------------------------------------------------------------------
// FreeTypeCacheManager — lifecycle wrapper for FTC_Manager
// ---------------------------------------------------------------------------
class FreeTypeCacheManager final {
public:
    explicit FreeTypeCacheManager(const FreeTypeCachePolicy& policy = FreeTypeCachePolicy::Default()) noexcept;

    FreeTypeCacheManager(const FreeTypeCacheManager&)            = delete;
    FreeTypeCacheManager& operator=(const FreeTypeCacheManager&) = delete;
    FreeTypeCacheManager(FreeTypeCacheManager&&)                 noexcept;
    FreeTypeCacheManager& operator=(FreeTypeCacheManager&&)      noexcept;

    ~FreeTypeCacheManager() noexcept;

    /// Initialises the FTC_Manager with the configured policy.
    [[nodiscard]] FreeTypeCacheStatus Initialise() noexcept;

    /// Returns true if the manager is initialised and ready.
    [[nodiscard]] bool IsReady() const noexcept { return m_ready; }

    /// Renders a font preview glyph strip into a caller-owned BGRA buffer.
    /// bufferBytes must be >= pixelSize * pixelSize * 4.
    [[nodiscard]] FreeTypeCacheStatus RenderGlyphStrip(
        const FreeTypeFaceEntry& face,
        std::uint8_t*            buffer,
        std::size_t              bufferBytes) noexcept;

    /// Removes all cached faces and bitmaps.
    void Flush() noexcept;

    /// Returns the active policy.
    [[nodiscard]] const FreeTypeCachePolicy& Policy() const noexcept { return m_policy; }

private:
    FreeTypeCachePolicy m_policy{};
    bool                m_ready = false;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_FREETYPECACHEPOLICY_H
