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
// RAII usage:
//   {
//       OOMKillGuard guard{ OOMKillGuardConfig{} };
//       guard.Arm();
//       // Decode pipeline runs under guard
//   }  // ~OOMKillGuard() calls Release() automatically
//
// Platform constraint:
//   SetProcessWorkingSetSizeEx() is Windows-only (Kernel32).
//   On non-Windows builds (macOS/Linux stubs) the working-set calls are
//   no-ops; VirtualAlloc/VirtualFree are also no-ops.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_OOM_KILL_GUARD_H
#define EXPLORERLENS_ENGINE_OOM_KILL_GUARD_H

#include <cstdint>
#include <cstddef>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <memoryapi.h>    // VirtualAlloc, VirtualFree
#  include <psapi.h>        // QUOTA_LIMITS_HARDWS_*
#endif

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
class OOMKillGuard final {
public:
    OOMKillGuard() noexcept = default;

    explicit OOMKillGuard(const OOMKillGuardConfig& cfg) noexcept
        : m_config{ cfg }
    {}

    // RAII: automatically release working-set limits on destruction.
    ~OOMKillGuard() noexcept
    {
        if (m_armed) { static_cast<void>(Release()); }
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
    [[nodiscard]] OOMKillGuardStatus Arm() noexcept
    {
        if (m_armed) { return OOMKillGuardStatus::ALREADY_ACTIVE; }

#ifdef _WIN32
        // 1. Reserve the emergency bailout region.
        if (m_config.emergencyRegionBytes > 0) {
            m_emergencyRegion = VirtualAlloc(
                nullptr,
                m_config.emergencyRegionBytes,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE);
            if (!m_emergencyRegion) {
                return OOMKillGuardStatus::RESERVATION_FAILED;
            }
        }

        // 2. Apply working-set limits via SetProcessWorkingSetSizeEx.
        //    Resolve dynamically — function exists on XP SP2+ but we guard
        //    for robustness when running on very old Wine/Proton builds.
        using FnType = BOOL(WINAPI*)(HANDLE, SIZE_T, SIZE_T, DWORD);
        const auto fn = reinterpret_cast<FnType>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
                           "SetProcessWorkingSetSizeEx"));

        if (fn && (m_config.minWorkingSetBytes > 0 || m_config.maxWorkingSetBytes > 0)) {
            const BOOL ok = fn(
                GetCurrentProcess(),
                static_cast<SIZE_T>(m_config.minWorkingSetBytes),
                static_cast<SIZE_T>(m_config.maxWorkingSetBytes),
                static_cast<DWORD>(m_config.workingSetFlags));

            if (!ok) {
                const DWORD err = GetLastError();
                if (err == ERROR_ACCESS_DENIED) {
                    // Free the already-reserved emergency region and bail.
                    if (m_emergencyRegion) {
                        VirtualFree(m_emergencyRegion, 0, MEM_RELEASE);
                        m_emergencyRegion = nullptr;
                    }
                    return OOMKillGuardStatus::PERMISSION_DENIED;
                }
                // Other non-fatal errors: working-set capping failed but the
                // emergency region is still useful — continue armed.
            }
        }
#endif // _WIN32

        m_armed = true;
        return OOMKillGuardStatus::OK;
    }

    /// Restore default working-set limits and free the emergency region.
    /// Safe to call multiple times.
    [[nodiscard]] OOMKillGuardStatus Release() noexcept
    {
        if (!m_armed) { return OOMKillGuardStatus::NOT_ACTIVE; }

#ifdef _WIN32
        // Restore OS-managed working-set by passing (SIZE_T)-1 sentinels.
        using FnType = BOOL(WINAPI*)(HANDLE, SIZE_T, SIZE_T, DWORD);
        const auto fn = reinterpret_cast<FnType>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
                           "SetProcessWorkingSetSizeEx"));
        if (fn) {
            fn(GetCurrentProcess(),
               static_cast<SIZE_T>(-1),
               static_cast<SIZE_T>(-1),
               0u);  // clear flags → OS manages
        }

        FreeEmergencyRegion();
#endif // _WIN32

        m_armed = false;
        return OOMKillGuardStatus::OK;
    }

    /// Free the emergency region early to give the allocator headroom.
    /// Call from a low-memory notification handler.
    void FreeEmergencyRegion() noexcept
    {
#ifdef _WIN32
        if (m_emergencyRegion) {
            VirtualFree(m_emergencyRegion, 0, MEM_RELEASE);
            m_emergencyRegion = nullptr;
        }
#endif
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
