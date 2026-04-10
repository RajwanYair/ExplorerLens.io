// NetworkTopologyProbe.h — Real-Time Network Topology Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Detects the active network topology (LAN/WiFi/Cell/VPN/Offline) at runtime
// so the streaming cache tier can adapt its aggressiveness accordingly.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class NetworkTopology : uint8_t {
    UNKNOWN   = 0,
    OFFLINE   = 1,
    CELL      = 2,  // Metered cellular — conserve bandwidth
    WIFI      = 3,  // WiFi — moderate aggressiveness
    LAN       = 4,  // Wired LAN — aggressive pre-fetch
    VPN       = 5,  // VPN tunnel — treat as LAN unless marked metered
};

struct NetworkProbeResult {
    NetworkTopology topology      = NetworkTopology::UNKNOWN;
    uint32_t        estimatedKbps = 0;   // 0 = unknown
    bool            isMetered     = false;
    bool            isVpn         = false;
};

class NetworkTopologyProbe {
public:
    struct Config {
        uint32_t probeIntervalMs  = 30'000;  // How often to refresh
        uint32_t pingTimeoutMs    = 2'000;
    };

    explicit NetworkTopologyProbe(const Config& cfg = {}) : m_cfg(cfg) {}

    NetworkProbeResult Probe();
    NetworkProbeResult LastResult() const { return m_last; }

    NetworkTopology  GetTopology()     const { return m_last.topology; }
    bool             IsMetered()       const { return m_last.isMetered; }
    uint32_t         EstimatedKbps()   const { return m_last.estimatedKbps; }

    void             ForceTopology(NetworkTopology t) { m_last.topology = t; }
    const Config&    GetConfig()       const { return m_cfg; }

    uint32_t         ProbeCount()      const { return m_probeCount; }

private:
    Config             m_cfg;
    NetworkProbeResult m_last;
    uint32_t           m_probeCount = 0;
};

}} // namespace ExplorerLens::Engine
