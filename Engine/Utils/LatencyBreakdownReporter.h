// Utils\LatencyBreakdownReporter.h - LatencyBreakdownReporter
// Copyright (c) 2026 ExplorerLens Project
//
// Generates per-stage latency breakdown reports for decode pipeline
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class LatencyBreakdownReporter {
public:
    struct Config {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "LatencyBreakdownReporter";
    };

    LatencyBreakdownReporter() = default;
    explicit LatencyBreakdownReporter(const Config& cfg) : m_config(cfg) {}

    bool Initialize() {
        if (!m_config.enabled) return false;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }
    const Config& GetConfig() const { return m_config; }
    const std::string& GetName() const { return m_config.label; }

private:
    Config m_config{};
    bool m_initialized = false;
    std::string m_stages{};
    std::string m_measurements{};
    std::string m_p99Latency{};
};

} // namespace Engine
} // namespace ExplorerLens
