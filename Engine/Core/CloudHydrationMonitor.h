// CloudHydrationMonitor.h — Windows Cloud Files Hydration Monitor
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors the Windows Cloud Files API (CfApi) hydration state for OneDrive /
// SharePoint placeholder files.  Before initiating a decode, the engine checks
// whether the local bytes needed for format detection are available; if not, it
// schedules a deferred decode callback once hydration passes the required threshold.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>

namespace ExplorerLens { namespace Engine {

/// Hydration state of a cloud placeholder file.
enum class HydrationState : uint8_t {
    FULLY_LOCAL      = 0,  ///< All bytes present locally
    PARTIALLY_LOCAL  = 1,  ///< Some bytes downloaded (range available)
    GHOST_PLACEHOLDER= 2,  ///< Zero bytes local; only metadata present
    NOT_A_PLACEHOLDER= 3,  ///< Regular file, not a cloud placeholder
    UNKNOWN          = 4,
};

/// Callback invoked when file hydration crosses the minimum threshold.
using HydrationReadyCallback = std::function<void(const std::wstring& filePath, HydrationState state)>;

/// Cloud hydration monitor.
/// On non-Windows builds and in unit tests the monitor uses file-size probing
/// as a substitute for actual CfApi calls.
class CloudHydrationMonitor {
public:
    struct Config {
        uint64_t minimumBytesRequired = 4096;   ///< Bytes needed for format detection
        uint32_t pollIntervalMs       = 200;    ///< Polling interval when waiting
        uint32_t maxWaitMs            = 5000;   ///< Abandon deferred decode after this
    };

    explicit CloudHydrationMonitor(const Config& cfg = {});

    /// Probe the hydration state of a file without waiting.
    HydrationState Probe(const std::wstring& filePath) const;

    /// Returns true if the file has enough local bytes  for decode to begin.
    bool IsDecodeable(const std::wstring& filePath) const;

    /// Register a callback to fire when the file becomes decodeable.
    /// Returns immediately; callback fires on a background thread when ready.
    void RequestWhenReady(const std::wstring& filePath, HydrationReadyCallback cb);

    /// Human-readable state label.
    static const wchar_t* StateLabel(HydrationState state);

    /// Cancel any pending deferred callbacks for the given path.
    void CancelDeferred(const std::wstring& filePath);

    const Config& GetConfig() const;

private:
    Config m_config;
};

}} // namespace ExplorerLens::Engine
