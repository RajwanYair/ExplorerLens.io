// ============================================================================
// AppContainerPluginHostContract.h -- S286 / ROADMAP v6.0 S4 sandboxed host
//
// Phase 4 contract: move plugin-hosted decoders out-of-process into an
// AppContainer surrogate with a pared-down capability set.  Header-only.
// Declares capability bit-set, named-pipe IPC framing, and resource caps.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class AppContainerPluginHostCapability : uint32_t
{
    NONE                   = 0,
    INTERNET_CLIENT        = 1u << 0,
    PRIVATE_NETWORK_CLIENT = 1u << 1,
    PICTURES_LIBRARY_READ  = 1u << 2,   // thumbnail reads only
    DOCUMENTS_LIBRARY_READ = 1u << 3,
    REMOVABLE_STORAGE_READ = 1u << 4,
    TEMP_FOLDER_WRITE      = 1u << 5,   // redirected to per-plugin temp
    GPU_ADAPTER_READ       = 1u << 6,   // D3D11 readback
    USER_DATA_READ         = 1u << 7,
};

enum class AppContainerIpcFraming : uint8_t
{
    LENGTH_PREFIXED_V1 = 0,   // [u32 len][u16 op][bytes...]
    SHARED_MEMORY      = 1,   // for large bitmaps
};

enum class AppContainerHostStatus : uint8_t
{
    READY                 = 0,
    LAUNCH_FAILED         = 1,
    CAPABILITY_DENIED     = 2,
    PIPE_FAILED           = 3,
    BUDGET_EXCEEDED       = 4,
    CRASHED               = 5,
    KILLED_BY_WATCHDOG    = 6,
};

struct AppContainerHostPolicy
{
    uint32_t allowedCapabilityMask = 0;          // AppContainerPluginHostCapability bits
    uint32_t workingSetLimitMb     = 256;
    uint32_t cpuQuotaPercent       = 50;
    uint32_t handshakeBudgetMs     = 500;
    uint32_t decodeBudgetMs        = 2000;
    bool     requireLowIlToken     = true;
    bool     allowSharedMemoryBitmap = true;
    bool     logDeniedCapabilities = true;
};

struct AppContainerHostProbe
{
    AppContainerHostStatus status              = AppContainerHostStatus::READY;
    uint32_t               launchMs            = 0;
    uint32_t               peakWorkingSetMb    = 0;
    uint32_t               capabilityDenials   = 0;
    uint32_t               watchdogTerminations = 0;
};

inline constexpr uint32_t kAppContainerHostMaxWorkingSetMb = 2048;
inline constexpr uint32_t kAppContainerHostDefaultCpuQuota = 50;
inline constexpr uint32_t kAppContainerHostHandshakeHardMs = 2000;

static_assert(std::is_trivially_copyable_v<AppContainerHostPolicy>,
              "AppContainerHostPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<AppContainerHostProbe>,
              "AppContainerHostProbe must be trivially copyable");

} // namespace ExplorerLens::Engine
