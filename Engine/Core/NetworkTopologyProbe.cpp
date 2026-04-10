// NetworkTopologyProbe.cpp — Real-Time Network Topology Detection
// Copyright (c) 2026 ExplorerLens Project
//
#include "NetworkTopologyProbe.h"

namespace ExplorerLens { namespace Engine {

NetworkProbeResult NetworkTopologyProbe::Probe()
{
    ++m_probeCount;

    // Stubbed implementation — real build integrates WlanApi / GetNetworkConnectivityHint
    // For test purposes, topology defaults to LAN and estimatedKbps = 1000.
    m_last.topology      = NetworkTopology::LAN;
    m_last.estimatedKbps = 1000;
    m_last.isMetered     = false;
    m_last.isVpn         = false;

    return m_last;
}

}} // namespace ExplorerLens::Engine
