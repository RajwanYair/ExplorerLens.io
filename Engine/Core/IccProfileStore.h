// Engine/Core/IccProfileStore.h
#pragma once

// IccProfileStore — in-process ICC colour-profile store (S362)
//
// Role in the Phase-3 ICC pipeline:
//   LcmsColorTransform (S342) applies lcms2 transforms; IccProfileStore holds the
//   raw ICC profile bytes extracted from decoded images so the transform layer can
//   retrieve them without re-parsing the file.
//
// Thread safety: all public methods are guarded by an internal SRWLOCK (Windows)
//               or std::shared_mutex (non-Windows). Safe for concurrent decoder threads.
//
// ROADMAP ref: Phase 3 exit criterion — "ICC color management end-to-end via lcms2 (H12, H30)"
//
// Usage:
//   IccProfileStore& store = IccProfileStore::Global();
//   store.Insert(fileKey, profileBytes, byteCount);
//   const IccProfileEntry* e = store.Lookup(fileKey);
//   if (e && e->IsValid()) { cmsOpenProfileFromMem(e->Data(), e->Size()); }

#ifndef EXPLORERLENS_ENGINE_ICCPROFILESTORE_H
#define EXPLORERLENS_ENGINE_ICCPROFILESTORE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace ExplorerLens::Engine {

/// Maximum cached profiles before LRU eviction kicks in.
inline constexpr std::uint32_t kIccProfileStoreMaxEntries = 64u;

/// Largest raw ICC profile accepted.
/// Real-world profiles are < 4 MB; reject pathologically large embedded blobs.
inline constexpr std::size_t kIccProfileMaxBytes = 4u * 1024u * 1024u;

// ---------------------------------------------------------------------------
// IccProfileId — predefined well-known colour spaces baked into the store
// ---------------------------------------------------------------------------
enum class IccProfileId : std::uint8_t {
    UNKNOWN           = 0,  ///< Source profile unknown — use sRGB fallback
    SRGB_IEC61966     = 1,  ///< sRGB IEC 61966-2.1 (most common)
    DISPLAY_P3        = 2,  ///< DCI-P3 / Display P3 (Apple wide-gamut displays)
    ADOBE_RGB_1998    = 3,  ///< Adobe RGB 1998 (camera RAW output)
    PRO_PHOTO_RGB     = 4,  ///< ProPhoto RGB (high-gamut workflow)
    LINEAR_SRGB       = 5,  ///< Linear light sRGB (HDR / EXR sources)
    BT2020            = 6,  ///< ITU-R BT.2020 (HEIF/AVIF HDR)
    CMYK_FOGRA39      = 7,  ///< CMYK FOGRA39 (PDF/print source files)
    EMBEDDED_CUSTOM   = 8,  ///< Custom profile embedded in the file
};

// ---------------------------------------------------------------------------
// IccProfileEntry — one cached profile record
// ---------------------------------------------------------------------------
struct IccProfileEntry {
    std::wstring              fileKey;                              ///< Owning file path (cache key)
    IccProfileId              profileId   = IccProfileId::UNKNOWN;
    std::vector<std::uint8_t> bytes;                               ///< Raw ICC bytes (empty for well-known)
    std::uint64_t             accessCount = 0u;                    ///< LRU counter
    bool                      isEmbedded  = false;                 ///< true = parsed from file bytes

    /// Returns the raw byte pointer, or nullptr for well-known profiles.
    [[nodiscard]] const std::uint8_t* Data()    const noexcept { return bytes.empty() ? nullptr : bytes.data(); }
    /// Returns the byte count (0 for well-known profiles).
    [[nodiscard]] std::size_t         Size()    const noexcept { return bytes.size(); }
    /// Returns true if the entry holds a usable profile (known ID or non-empty bytes).
    [[nodiscard]] bool                IsValid() const noexcept {
        return profileId != IccProfileId::UNKNOWN || !bytes.empty();
    }
};

// ---------------------------------------------------------------------------
// IccProfileStoreConfig
// ---------------------------------------------------------------------------
struct IccProfileStoreConfig {
    std::uint32_t maxEntries       = kIccProfileStoreMaxEntries;
    std::size_t   maxProfileBytes  = kIccProfileMaxBytes;

    [[nodiscard]] static IccProfileStoreConfig Default() noexcept {
        return IccProfileStoreConfig{};
    }

    [[nodiscard]] static IccProfileStoreConfig ForLowMemory() noexcept {
        IccProfileStoreConfig cfg{};
        cfg.maxEntries      = 16u;
        cfg.maxProfileBytes = 512u * 1024u; // 512 KB cap
        return cfg;
    }
};

// ---------------------------------------------------------------------------
// IccProfileStoreStatus
// ---------------------------------------------------------------------------
enum class IccProfileStoreStatus : std::uint8_t {
    OK          = 0,
    NOT_FOUND   = 1,  ///< Key not present in store
    OVERSIZED   = 2,  ///< Profile exceeds maxProfileBytes
    STORE_FULL  = 3,  ///< Eviction limit reached (should not occur)
    NULL_KEY    = 4,
    NULL_BYTES  = 5,
};

// ---------------------------------------------------------------------------
// IccProfileStore — singleton in-process profile cache
// ---------------------------------------------------------------------------
class IccProfileStore final {
public:
    IccProfileStore(const IccProfileStore&)            = delete;
    IccProfileStore& operator=(const IccProfileStore&) = delete;

    /// Returns the process-wide singleton.
    [[nodiscard]] static IccProfileStore& Global() noexcept;

    /// Inserts or replaces an embedded ICC profile for fileKey.
    /// Returns OVERSIZED if byteCount > config.maxProfileBytes.
    [[nodiscard]] IccProfileStoreStatus Insert(
        const std::wstring& fileKey,
        const std::uint8_t* profileBytes,
        std::size_t         byteCount,
        IccProfileId        hint = IccProfileId::EMBEDDED_CUSTOM) noexcept;

    /// Associates a well-known built-in profile (no raw bytes) with fileKey.
    [[nodiscard]] IccProfileStoreStatus InsertWellKnown(
        const std::wstring& fileKey,
        IccProfileId        profileId) noexcept;

    /// Returns nullptr if not found. Do NOT cache the pointer across Insert calls.
    [[nodiscard]] const IccProfileEntry* Lookup(const std::wstring& fileKey) noexcept;

    /// Removes the entry for fileKey. Returns NOT_FOUND if absent.
    [[nodiscard]] IccProfileStoreStatus Evict(const std::wstring& fileKey) noexcept;

    /// Clears all cached entries.
    void Clear() noexcept;

    /// Current entry count.
    [[nodiscard]] std::uint32_t Count() const noexcept;

    /// Applies configuration. Call before first Insert.
    void Configure(const IccProfileStoreConfig& cfg) noexcept;

private:
    IccProfileStore() noexcept = default;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_ICCPROFILESTORE_H
