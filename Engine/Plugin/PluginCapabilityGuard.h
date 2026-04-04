// PluginCapabilityGuard.h — Capability-Based Security for Plugins
// Copyright (c) 2026 ExplorerLens Project
//
// Implements capability-based access control for third-party plugins.
// Each plugin receives a CapabilitySet defining permitted operations.
// CapabilityGuard provides RAII validation before sensitive operations.
// All capability checks are recorded in an audit trail.
//
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Capability flags — each represents a privileged operation class
enum class PluginCapability : uint32_t {
    None = 0,
    ReadFile = 1u << 0,
    WriteFile = 1u << 1,
    Network = 1u << 2,
    GPU = 1u << 3,
    Memory = 1u << 4,
    Registry = 1u << 5,
    ProcessSpawn = 1u << 6,
    All = 0x7Fu
};

inline PluginCapability operator|(PluginCapability a, PluginCapability b)
{
    return static_cast<PluginCapability>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline PluginCapability operator&(PluginCapability a, PluginCapability b)
{
    return static_cast<PluginCapability>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline PluginCapability operator~(PluginCapability a)
{
    return static_cast<PluginCapability>(~static_cast<uint32_t>(a));
}

inline bool HasCapability(PluginCapability set, PluginCapability cap)
{
    return (static_cast<uint32_t>(set) & static_cast<uint32_t>(cap)) == static_cast<uint32_t>(cap);
}

// Audit record for a single capability check
struct CapabilityCheckRecord
{
    std::chrono::steady_clock::time_point timestamp;
    std::wstring pluginId;
    PluginCapability requested = PluginCapability::None;
    bool granted = false;
    std::wstring context;
};

// Returns human-readable name for a capability flag
inline const wchar_t* CapabilityName(PluginCapability cap)
{
    switch (cap) {
        case PluginCapability::ReadFile:
            return L"ReadFile";
        case PluginCapability::WriteFile:
            return L"WriteFile";
        case PluginCapability::Network:
            return L"Network";
        case PluginCapability::GPU:
            return L"GPU";
        case PluginCapability::Memory:
            return L"Memory";
        case PluginCapability::Registry:
            return L"Registry";
        case PluginCapability::ProcessSpawn:
            return L"ProcessSpawn";
        case PluginCapability::All:
            return L"All";
        case PluginCapability::None:
            return L"None";
        default:
            return L"Unknown";
    }
}

// ========================================================================
// CapabilitySet — bitmask container with grant/revoke/check operations
// ========================================================================
class CapabilitySet
{
  public:
    CapabilitySet() : m_capabilities(PluginCapability::None) {}
    explicit CapabilitySet(PluginCapability caps) : m_capabilities(caps) {}

    void Grant(PluginCapability cap)
    {
        m_capabilities = m_capabilities | cap;
    }

    void Revoke(PluginCapability cap)
    {
        m_capabilities = m_capabilities & (~cap);
    }

    bool Check(PluginCapability cap) const
    {
        return HasCapability(m_capabilities, cap);
    }

    bool CheckAll(PluginCapability caps) const
    {
        return HasCapability(m_capabilities, caps);
    }

    bool CheckAny(PluginCapability caps) const
    {
        return (static_cast<uint32_t>(m_capabilities) & static_cast<uint32_t>(caps)) != 0;
    }

    PluginCapability GetRaw() const
    {
        return m_capabilities;
    }

    uint32_t Count() const
    {
        uint32_t bits = static_cast<uint32_t>(m_capabilities);
        uint32_t count = 0;
        while (bits) {
            count += (bits & 1u);
            bits >>= 1;
        }
        return count;
    }

    void Clear()
    {
        m_capabilities = PluginCapability::None;
    }
    bool IsEmpty() const
    {
        return m_capabilities == PluginCapability::None;
    }

  private:
    PluginCapability m_capabilities;
};

// ========================================================================
// CapabilityGuard — RAII wrapper validating capabilities before ops
// ========================================================================
class CapabilityGuard
{
  public:
    CapabilityGuard(const CapabilitySet& caps, PluginCapability required, const std::wstring& pluginId = L"",
                    const std::wstring& context = L"")
        : m_granted(caps.Check(required))
        , m_pluginId(pluginId)
        , m_requested(required)
    {
        CapabilityCheckRecord record;
        record.timestamp = std::chrono::steady_clock::now();
        record.pluginId = pluginId;
        record.requested = required;
        record.granted = m_granted;
        record.context = context;

        AuditTrail().AddRecord(std::move(record));
    }

    bool IsGranted() const
    {
        return m_granted;
    }
    explicit operator bool() const
    {
        return m_granted;
    }

    // Nested audit trail manager (singleton-per-process)
    class AuditTrailManager
    {
      public:
        void AddRecord(CapabilityCheckRecord record)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_records.push_back(std::move(record));
            m_totalChecks.fetch_add(1, std::memory_order_relaxed);
        }

        std::vector<CapabilityCheckRecord> GetRecords() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_records;
        }

        uint64_t GetTotalChecks() const
        {
            return m_totalChecks.load(std::memory_order_relaxed);
        }

        uint64_t GetDeniedCount() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            uint64_t denied = 0;
            for (auto& r : m_records) {
                if (!r.granted)
                    denied++;
            }
            return denied;
        }

        void Clear()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_records.clear();
            m_totalChecks.store(0, std::memory_order_relaxed);
        }

        std::vector<CapabilityCheckRecord> GetDeniedRecords() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::vector<CapabilityCheckRecord> denied;
            for (auto& r : m_records) {
                if (!r.granted)
                    denied.push_back(r);
            }
            return denied;
        }

      private:
        mutable std::mutex m_mutex;
        std::vector<CapabilityCheckRecord> m_records;
        std::atomic<uint64_t> m_totalChecks{0};
    };

    static AuditTrailManager& AuditTrail()
    {
        static AuditTrailManager instance;
        return instance;
    }

  private:
    bool m_granted;
    std::wstring m_pluginId;
    PluginCapability m_requested;
};

}  // namespace Engine
}  // namespace ExplorerLens
