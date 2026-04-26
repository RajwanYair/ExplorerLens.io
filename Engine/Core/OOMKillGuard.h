// Engine/Core/OOMKillGuard.h
// ExplorerLens — OOM kill protection via SetProcessWorkingSetSizeEx (ROADMAP v7.0 Phase 2)
// Sprint S314.
//
// Purpose:
//   Windows Explorer hosts LENSShell.dll in-process.  If the thumbnail decode
//   pipeline exhausts virtual address space or triggers out-of-memory (OOM),
//   the crash kills the entire Explorer process — a catastrophic UX failure.
//
//   OOMKillGuard uses two Windows APIs to limit damage:
//     1. SetProcessWorkingSetSizeEx() — caps the process working-set (physical
//        RAM pages) so large decodes are paged out before committing.
//     2. VirtualAlloc pre-reservation — reserves a 64 MB "emergency bailout"
//        region that can be freed to give the allocator headroom for cleanup.
//
// RAII usage (Phase 2):
//   {
//       OOMKillGuard guard{ OOMKillGuardConfig{} };
//       // Decode pipeline runs under guard
//   }  // guard.Release() called automatically on scope exit
//
// Platform constraint:
//   SetProcessWorkingSetSizeEx() is Windows-only (Kernel32).
//   On non-Windows builds (macOS/Linux stubs) all operations are no-ops.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_OOM_KILL_GUARD_H
#define EXPLORERLENS_ENGINE_OOM_KILL_GUARD_H

#include <cstdint>
#include <cstddef>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// OOMKillGuardConfig
// ---------------------------------------------------------------------------
struct OOMKillGuardConfig final {
    // Working-set floor (minimum physical pages pinned to RAM).
    // Default: 32 MiB.  Set 0 to use the OS default.
    std::size_t minWorkingSetBytes = 32u * 1024u * 1024u;

    // Working-set ceiling.
    // Default: 512 MiB.  Limits RAM footprint under heavy thumbnail load.
    std::size_t maxWorkingSetBytes = 512u * 1024u * 1024u;

    // Emergency bailout region size reserved at construction.
    // Freed first when OOM is detected.  0 = disable.
    std::size_t emergencyRegionBytes = 64u * 1024u * 1024u;

    // QUOTA_LIMITS_HARDWS flags to pass to SetProcessWorkingSetSizeEx:
    //   QUOTA_LIMITS_HARDWS_MIN_DISABLE = 0x00000002
    //   QUOTA_LIMITS_HARDWS_MAX_ENABLE  = 0x00000004
    // Default: hard maximum, soft minimum.
    std::uint32_t workingSetFlags = 0x00000004u;
};

// ---------------------------------------------------------------------------
// OOMKillGuardStatus
// ---------------------------------------------------------------------------
enum class OOMKillGuardStatus : std::uint8_t {
    OK                  = 0,
    ALREADY_ACTIVE      = 1,
    API_NOT_AVAILABLE   = 2,   ///< SetProcessWorkingSetSizeEx missing (< WinXP)
    PERMISSION_DENIED   = 3,   ///< Requires SE_INCREASE_QUOTA_NAME privilege
    RESERVATION_FAILED  = 4,   ///< VirtualAlloc for emergency region failed
    NOT_ACTIVE          = 5,   ///< Release() called before Arm()
};

// ---------------------------------------------------------------------------
// OOMKillGuard
// ---------------------------------------------------------------------------
// Phase 2 stub — Arm() and Release() are no-ops until Kernel32 wiring.
// IsArmed() is always false in the stub.
//
class OOMKillGuard final {
public:
    OOMKillGuard() noexcept = default;

    explicit OOMKillGuard(const OOMKillGuardConfig& cfg) noexcept
        : m_config{ cfg }
    {}

    // RAII: automatically release working-set limits on destruction.
    ~OOMKillGuard() noexcept
    {
        if (m_armed) { Release(); }
    }

    // Non-copyable, movable.
    OOMKillGuard(const OOMKillGuard&)            = delete;
    OOMKillGuard& operator=(const OOMKillGuard&) = delete;

    OOMKillGuard(OOMKillGuard&& other) noexcept
        : m_config{ other.m_config }
        , m_armed{ other.m_armed }
        , m_emergencyRegion{ other.m_emergencyRegion }
    {
        other.m_armed           = false;
        other.m_emergencyRegion = nullptr;
    }

    // ── Lifecycle ─────────────────────────────────────────────────────────────

    /// Set working-set limits and reserve the emergency bailout region.
    /// Phase 2 stub: always returns OK without touching OS APIs.
    [[nodiscard]] OOMKillGuardStatus Arm() noexcept
    {
        // TODO(S314/Phase2): call SetProcessWorkingSetSizeEx + VirtualAlloc
        m_armed = true;
        return OOMKillGuardStatus::OK;
    }

    /// Restore original working-set limits and free the emergency region.
    /// Safe to call multiple times.
    [[nodiscard]] OOMKillGuardStatus Release() noexcept
    {
        if (!m_armed) { return OOMKillGuardStatus::NOT_ACTIVE; }
        // TODO(S314/Phase2): restore SetProcessWorkingSetSizeEx(-1,-1,0)
        //                    + VirtualFree(m_emergencyRegion)
        m_armed           = false;
        m_emergencyRegion = nullptr;
        return OOMKillGuardStatus::OK;
    }

    /// Free the emergency region early (before scope exit) to give the
    /// allocator headroom to run cleanup.  Call from a low-memory handler.
    void FreeEmergencyRegion() noexcept
    {
        if (m_emergencyRegion) {
            // TODO(S314/Phase2): VirtualFree(m_emergencyRegion, 0, MEM_RELEASE)
            m_emergencyRegion = nullptr;
        }
    }

    // ── Observers ────────────────────────────────────────────────────────────

    [[nodiscard]] bool IsArmed() const noexcept { return m_armed; }

    [[nodiscard]] const OOMKillGuardConfig& Config() const noexcept
    {
        return m_config;
    }

    // ── Constants ─────────────────────────────────────────────────────────────

    /// Absolute maximum working-set ceiling (256 GB — sanity cap).
    static constexpr std::size_t kAbsoluteMaxWorkingSetBytes =
        256ull * 1024u * 1024u * 1024u;

    /// Minimum meaningful emergency region (must be at least one alloc granule).
    static constexpr std::size_t kMinEmergencyRegionBytes = 65536u;

private:
    OOMKillGuardConfig  m_config{};
    bool                m_armed{ false };
    void*               m_emergencyRegion{ nullptr };
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_OOM_KILL_GUARD_H
