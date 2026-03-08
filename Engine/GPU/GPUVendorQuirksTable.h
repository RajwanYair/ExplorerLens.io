// GPUVendorQuirksTable.h — Vendor-specific GPU quirks and workarounds registry
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains a table of known GPU vendor quirks (NVIDIA, AMD, Intel) that
// require special handling — driver version ranges, feature blacklists, etc.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct GPUVendorQuirksTableConfig {
    bool enabled = true;
    uint32_t maxQuirks = 128;
    std::string label = "GPUVendorQuirksTable";
};

class GPUVendorQuirksTable {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    GPUVendorQuirksTableConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Vendor : uint8_t { Unknown, NVIDIA, AMD, Intel, Qualcomm };

    struct Quirk {
        Vendor vendor = Vendor::Unknown;
        std::string description;
        uint32_t minDriverVersion = 0;
        uint32_t maxDriverVersion = UINT32_MAX;
    };

    bool RegisterQuirk(const Quirk& q) {
        if (m_quirks.size() >= m_config.maxQuirks) return false;
        m_quirks.push_back(q);
        return true;
    }

    bool HasQuirks(Vendor v) const {
        for (const auto& q : m_quirks)
            if (q.vendor == v) return true;
        return false;
    }

private:
    bool m_initialized = false;
    GPUVendorQuirksTableConfig m_config;
    std::vector<Quirk> m_quirks;
};

}
} // namespace ExplorerLens::Engine
