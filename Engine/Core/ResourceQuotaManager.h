#pragma once
// ============================================================================
// ResourceQuotaManager.h — Per-process resource quota enforcement
//                          (memory, CPU, handles)
//
// Purpose:   Per-process resource quota enforcement (memory, CPU, handles)
// Provides:  ResourceType, QuotaAction enums, ResourceQuota struct,
//            ResourceQuotaManager class
// Used by:   Sandbox and memory management
// ============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Resource type being metered
enum class QuotaResource : uint8_t {
    Memory = 0,     // Working set / committed memory
    CPU = 1,        // CPU time in milliseconds
    DiskRead = 2,   // Bytes read from storage
    DiskWrite = 3,  // Bytes written to storage
    GPUMemory = 4   // GPU VRAM / shared memory
};

inline const char* QuotaResourceName(QuotaResource r) noexcept
{
    switch (r) {
        case QuotaResource::Memory:
            return "Memory";
        case QuotaResource::CPU:
            return "CPU";
        case QuotaResource::DiskRead:
            return "DiskRead";
        case QuotaResource::DiskWrite:
            return "DiskWrite";
        case QuotaResource::GPUMemory:
            return "GPUMemory";
        default:
            return "Unknown";
    }
}

/// How quota violations are enforced
enum class QuotaEnforcement : uint8_t {
    SoftLimit = 0,  // Log warning, continue processing
    HardLimit = 1,  // Fail the operation immediately
    Warning = 2,    // Emit telemetry warning only
    Throttle = 3,   // Slow down processing rate
    Deny = 4        // Reject new operations entirely
};

inline const char* QuotaEnforcementName(QuotaEnforcement e) noexcept
{
    switch (e) {
        case QuotaEnforcement::SoftLimit:
            return "SoftLimit";
        case QuotaEnforcement::HardLimit:
            return "HardLimit";
        case QuotaEnforcement::Warning:
            return "Warning";
        case QuotaEnforcement::Throttle:
            return "Throttle";
        case QuotaEnforcement::Deny:
            return "Deny";
        default:
            return "Unknown";
    }
}

/// Describes a resource quota and its current usage
struct ResourceQuota
{
    QuotaResource resource = QuotaResource::Memory;
    QuotaEnforcement enforcement = QuotaEnforcement::SoftLimit;
    uint64_t limitValue = 0;    // Quota ceiling
    uint64_t currentUsage = 0;  // Current consumed amount
    std::string unit;           // "bytes", "ms", etc.
};

/// Manages per-session resource quotas to prevent runaway resource
/// consumption.  Supports five resource types with independently
/// configurable enforcement policies.
class ResourceQuotaManager
{
  public:
    ResourceQuotaManager() = default;
    ~ResourceQuotaManager() = default;

    ResourceQuotaManager(const ResourceQuotaManager&) = delete;
    ResourceQuotaManager& operator=(const ResourceQuotaManager&) = delete;
    ResourceQuotaManager(ResourceQuotaManager&&) noexcept = default;
    ResourceQuotaManager& operator=(ResourceQuotaManager&&) noexcept = default;

    /// Set or update a quota for a resource type
    void SetQuota(QuotaResource resource, QuotaEnforcement enforcement, uint64_t limitValue,
                  const std::string& unit = "bytes")
    {
        for (auto& q : m_quotas) {
            if (q.resource == resource) {
                q.enforcement = enforcement;
                q.limitValue = limitValue;
                q.unit = unit;
                return;
            }
        }
        ResourceQuota rq{};
        rq.resource = resource;
        rq.enforcement = enforcement;
        rq.limitValue = limitValue;
        rq.unit = unit;
        m_quotas.push_back(rq);
    }

    /// Check whether adding 'delta' bytes/ms would violate the quota
    QuotaEnforcement CheckQuota(QuotaResource resource, uint64_t delta) const
    {
        for (const auto& q : m_quotas) {
            if (q.resource == resource) {
                if (q.currentUsage + delta > q.limitValue) {
                    return q.enforcement;
                }
                return QuotaEnforcement::SoftLimit;  // Within limits
            }
        }
        return QuotaEnforcement::SoftLimit;  // No quota set
    }

    /// Record resource consumption
    bool RecordUsage(QuotaResource resource, uint64_t amount)
    {
        for (auto& q : m_quotas) {
            if (q.resource == resource) {
                q.currentUsage += amount;
                return true;
            }
        }
        return false;
    }

    /// Get current usage for a resource
    uint64_t GetUsage(QuotaResource resource) const noexcept
    {
        for (const auto& q : m_quotas) {
            if (q.resource == resource)
                return q.currentUsage;
        }
        return 0;
    }

    /// Returns true if any quota is currently exceeded
    bool IsOverQuota() const noexcept
    {
        for (const auto& q : m_quotas) {
            if (q.currentUsage > q.limitValue)
                return true;
        }
        return false;
    }

    /// Get all configured quotas
    const std::vector<ResourceQuota>& GetQuotas() const noexcept
    {
        return m_quotas;
    }

    /// Reset all usage counters to zero
    void ResetUsage() noexcept
    {
        for (auto& q : m_quotas)
            q.currentUsage = 0;
    }

  private:
    std::vector<ResourceQuota> m_quotas;
};

}  // namespace Engine
}  // namespace ExplorerLens
