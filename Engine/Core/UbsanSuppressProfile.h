// Engine/Core/UbsanSuppressProfile.h
// ExplorerLens Engine — S384 (Phase 4, Sprint 4)
//
// Purpose:
//   UBSAN (Undefined Behavior Sanitizer) suppression profile registry.
//   Phase 4 exit criterion: "UBSAN clean in CI"
//
//   Tracks runtime UBSAN findings, categorizes them as:
//   - APPROVED: known false positive (e.g. bit-shift in decode libraries)
//   - PENDING: new finding, needs investigation
//   - FIXED: root cause fixed in source; entry kept for regression tracking
//
//   Feeds the CI workflow ubsan-clean.yml which:
//   1. Builds with -fsanitize=undefined (Clang CI lane)
//   2. Runs EngineTests with UBSAN_OPTIONS=halt_on_error=0:print_stacktrace=1
//   3. Compares findings against this profile's APPROVED list
//   4. Fails if any PENDING finding is found
//
//   Note: MSVC cl.exe 19.50 does not have native UBSAN; this profile is used
//   by the Clang CI lane (ubuntu-latest, clang-18).

#pragma once
#ifndef EXPLORERLENS_ENGINE_UBSANSUPPRESSPROFILE_H
#define EXPLORERLENS_ENGINE_UBSANSUPPRESSPROFILE_H

#include <cstdint>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── Finding category ────────────────────────────────────────────────────────

enum class UbsanFindingCategory : uint8_t {
    UNDEFINED_SHIFT        = 0,
    SIGNED_OVERFLOW        = 1,
    NULL_DEREF             = 2,
    INVALID_ENUM           = 3,
    ARRAY_BOUNDS           = 4,
    INTEGER_OVERFLOW       = 5,
    FLOAT_CAST_OVERFLOW    = 6,
    ALIGNMENT              = 7,
};

// ─── Finding disposition ──────────────────────────────────────────────────────

enum class UbsanDisposition : uint8_t {
    APPROVED    = 0,   // known false positive or third-party; suppressed
    PENDING     = 1,   // newly discovered; CI blocks on this
    FIXED       = 2,   // root cause fixed; kept for regression tracking
    WONT_FIX    = 3,   // accepted risk (e.g. legacy library)
};

// ─── Suppression entry ────────────────────────────────────────────────────────

struct UbsanEntry final {
    UbsanFindingCategory category     = UbsanFindingCategory::UNDEFINED_SHIFT;
    UbsanDisposition     disposition  = UbsanDisposition::PENDING;
    const char*          sourceFile   = nullptr;   // e.g. "Engine/Decoders/StbImageDecoder.h"
    uint32_t             sourceLine   = 0;
    const char*          description  = nullptr;
    const char*          sprintFixed  = nullptr;   // e.g. "S384" if FIXED

    bool IsBlocking() const noexcept { return disposition == UbsanDisposition::PENDING; }
    bool IsSuppressed() const noexcept {
        return disposition == UbsanDisposition::APPROVED ||
               disposition == UbsanDisposition::WONT_FIX;
    }
};

// ─── Profile ─────────────────────────────────────────────────────────────────

struct UbsanProfileStats final {
    uint32_t total      = 0;
    uint32_t approved   = 0;
    uint32_t pending    = 0;
    uint32_t fixed      = 0;
    uint32_t wontFix    = 0;

    bool IsCiClean() const noexcept { return pending == 0; }
};

// ─── Profile registry ────────────────────────────────────────────────────────

class UbsanSuppressProfile final {
public:
    UbsanSuppressProfile() = default;
    ~UbsanSuppressProfile() = default;

    UbsanSuppressProfile(const UbsanSuppressProfile&) = delete;
    UbsanSuppressProfile& operator=(const UbsanSuppressProfile&) = delete;

    static UbsanSuppressProfile& Global() noexcept {
        static UbsanSuppressProfile s_instance;
        return s_instance;
    }

    // Register a known finding
    bool Register(const UbsanEntry& entry) noexcept;

    // Check if a given source+line is approved/suppressed
    bool IsSuppressed(const char* sourceFile, uint32_t line) const noexcept;

    // Count findings by disposition
    UbsanProfileStats Stats() const noexcept;

    // Is CI clean? (zero PENDING findings)
    bool IsCiClean() const noexcept { return Stats().IsCiClean(); }

    // Total registered entries
    uint32_t EntryCount() const noexcept { return m_entryCount; }

private:
    static constexpr uint32_t kMaxEntries = 256u;
    UbsanEntry  m_entries[kMaxEntries]{};
    uint32_t    m_entryCount = 0;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline bool UbsanSuppressProfile::Register(const UbsanEntry& entry) noexcept {
    if (m_entryCount >= kMaxEntries) return false;
    m_entries[m_entryCount++] = entry;
    return true;
}

inline bool UbsanSuppressProfile::IsSuppressed(
    const char* sourceFile, uint32_t line) const noexcept
{
    if (!sourceFile) return false;
    std::string_view sv{sourceFile};
    for (uint32_t i = 0; i < m_entryCount; ++i) {
        const auto& e = m_entries[i];
        if (e.sourceLine == line && e.sourceFile &&
            sv == std::string_view{e.sourceFile}) {
            return e.IsSuppressed();
        }
    }
    return false;
}

inline UbsanProfileStats UbsanSuppressProfile::Stats() const noexcept {
    UbsanProfileStats s{};
    s.total = m_entryCount;
    for (uint32_t i = 0; i < m_entryCount; ++i) {
        switch (m_entries[i].disposition) {
            case UbsanDisposition::APPROVED:  ++s.approved; break;
            case UbsanDisposition::PENDING:   ++s.pending;  break;
            case UbsanDisposition::FIXED:     ++s.fixed;    break;
            case UbsanDisposition::WONT_FIX:  ++s.wontFix;  break;
        }
    }
    return s;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kUbsanMaxSuppressEntries = 256u;
static constexpr const char* kUbsanCiOptionsStr    =
    "halt_on_error=0:print_stacktrace=1:report_error_type=1";

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_UBSANSUPPRESSPROFILE_H
