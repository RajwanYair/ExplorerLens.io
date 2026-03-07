// GuardPageProtector.h — Guard Page Buffer Overflow Protection
// Copyright (c) 2026 ExplorerLens Project
//
// Implements guard page protection around decode buffers to detect
// buffer overflows and underflows during thumbnail generation.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GuardPageMode : uint8_t {
    Disabled,
    OverflowOnly,
    UnderflowOnly,
    Both,
    Canary
};

struct ProtectedRegion {
    uint64_t regionId = 0;
    uint64_t baseAddress = 0;
    uint64_t sizeBytes = 0;
    GuardPageMode mode = GuardPageMode::Both;
    bool violated = false;
    std::string allocationContext;
};

struct GuardPageMetrics {
    uint64_t totalProtectedRegions = 0;
    uint64_t activeProtectedRegions = 0;
    uint64_t violationsDetected = 0;
    uint64_t totalBytesProtected = 0;
    uint64_t guardPagesAllocated = 0;
};

class GuardPageProtector {
public:
    explicit GuardPageProtector(GuardPageMode defaultMode = GuardPageMode::Both)
        : m_defaultMode(defaultMode) {
    }

    uint64_t ProtectRegion(uint64_t baseAddress, uint64_t sizeBytes,
        const std::string& context = "") {
        uint64_t id = ++m_nextId;
        ProtectedRegion region;
        region.regionId = id;
        region.baseAddress = baseAddress;
        region.sizeBytes = sizeBytes;
        region.mode = m_defaultMode;
        region.allocationContext = context;
        m_regions.push_back(region);
        m_metrics.totalProtectedRegions++;
        m_metrics.activeProtectedRegions++;
        m_metrics.totalBytesProtected += sizeBytes;
        m_metrics.guardPagesAllocated += (m_defaultMode == GuardPageMode::Both) ? 2 : 1;
        return id;
    }

    bool UnprotectRegion(uint64_t regionId) {
        for (auto it = m_regions.begin(); it != m_regions.end(); ++it) {
            if (it->regionId == regionId) {
                m_metrics.activeProtectedRegions--;
                m_metrics.totalBytesProtected -= it->sizeBytes;
                m_regions.erase(it);
                return true;
            }
        }
        return false;
    }

    void ReportViolation(uint64_t regionId) {
        for (auto& region : m_regions) {
            if (region.regionId == regionId) {
                region.violated = true;
                m_metrics.violationsDetected++;
                break;
            }
        }
    }

    bool HasViolations() const { return m_metrics.violationsDetected > 0; }
    GuardPageMetrics GetMetrics() const { return m_metrics; }
    GuardPageMode GetDefaultMode() const { return m_defaultMode; }
    void SetDefaultMode(GuardPageMode mode) { m_defaultMode = mode; }

    std::vector<ProtectedRegion> GetViolatedRegions() const {
        std::vector<ProtectedRegion> result;
        for (const auto& r : m_regions) {
            if (r.violated) result.push_back(r);
        }
        return result;
    }

private:
    std::vector<ProtectedRegion> m_regions;
    GuardPageMode m_defaultMode;
    uint64_t m_nextId = 0;
    GuardPageMetrics m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
