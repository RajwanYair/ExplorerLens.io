// DecoderPriorityManager.h — Decoder priority routing and fallback selection
// Copyright (c) 2026 ExplorerLens Project
//
// Singleton that manages decoder priorities per file extension, enabling
// primary/fallback decoder selection and runtime availability toggling.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DecoderPriority : uint8_t {
    Critical = 0,
    High = 1,
    Normal = 2,
    Low = 3,
    Fallback = 4
};

class DecoderPriorityManager
{
  public:
    static DecoderPriorityManager& GetInstance() noexcept
    {
        static DecoderPriorityManager instance;
        return instance;
    }

    void RegisterDecoder(const std::wstring& decoderName, const std::vector<std::wstring>& extensions,
                         DecoderPriority priority)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_decoderPriority[decoderName] = priority;
        m_available[decoderName] = true;
        for (auto& ext : extensions) {
            m_extDecoders[ext].push_back({decoderName, priority});
        }
    }

    std::wstring GetPrimaryDecoder(const std::wstring& ext) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_extDecoders.find(ext);
        if (it == m_extDecoders.end() || it->second.empty())
            return {};
        const DecoderEntry* best = nullptr;
        for (auto& e : it->second) {
            if (!IsAvailableLocked(e.name))
                continue;
            if (!best || e.priority < best->priority)
                best = &e;
        }
        return best ? best->name : std::wstring{};
    }

    std::wstring GetFallbackDecoder(const std::wstring& ext, const std::wstring& primary) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_extDecoders.find(ext);
        if (it == m_extDecoders.end())
            return {};
        const DecoderEntry* fallback = nullptr;
        for (auto& e : it->second) {
            if (e.name == primary)
                continue;
            if (!IsAvailableLocked(e.name))
                continue;
            if (!fallback || e.priority < fallback->priority)
                fallback = &e;
        }
        return fallback ? fallback->name : std::wstring{};
    }

    void SetDecoderAvailable(const std::wstring& name, bool avail)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_available[name] = avail;
    }

  private:
    DecoderPriorityManager() = default;

    struct DecoderEntry
    {
        std::wstring name;
        DecoderPriority priority;
    };

    bool IsAvailableLocked(const std::wstring& name) const
    {
        auto it = m_available.find(name);
        return it == m_available.end() || it->second;
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, DecoderPriority> m_decoderPriority;
    std::unordered_map<std::wstring, bool> m_available;
    std::unordered_map<std::wstring, std::vector<DecoderEntry>> m_extDecoders;
};

// Priority-based decode task scheduler
enum class DecodeUrgency : uint8_t {
    Immediate = 0,
    Soon = 1,
    Background = 2,
    Idle = 3
};

class PriorityDecodeScheduler
{
  public:
    struct Stats
    {
        uint64_t totalScheduled = 0;
        uint64_t totalCompleted = 0;
    };

    uint64_t Submit(const std::wstring& /*path*/, DecodeUrgency /*urgency*/, uint32_t /*targetSize*/)
    {
        m_stats.totalScheduled++;
        return m_stats.totalScheduled;
    }

    Stats GetStats() const noexcept
    {
        return m_stats;
    }

  private:
    Stats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
