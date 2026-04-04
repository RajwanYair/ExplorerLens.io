// LazyDecodeInitializer.h — Deferred Decoder Initialization
// Copyright (c) 2026 ExplorerLens Project
//
// Delays heavy decoder initialization until first actual use,
// reducing startup time and memory footprint for unused decoders.
//
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class DecoderInitState : uint8_t {
    NotStarted = 0,
    Initializing = 1,
    Ready = 2,
    Failed = 3
};

struct LazyInitStats
{
    uint32_t totalDecoders = 0;
    uint32_t initializedCount = 0;
    uint32_t failedCount = 0;
    uint64_t savedStartupMs = 0;
    uint64_t totalInitMs = 0;
};

class LazyDecodeInitializer
{
  public:
    using InitFunc = std::function<bool()>;

    struct Entry
    {
        std::string decoderName;
        InitFunc initFn;
        DecoderInitState state = DecoderInitState::NotStarted;
        uint64_t initDurationMs = 0;
        uint32_t initAttempts = 0;
    };

    void Register(const std::string& name, InitFunc fn)
    {
        std::lock_guard lock(m_mutex);
        Entry e;
        e.decoderName = name;
        e.initFn = std::move(fn);
        m_entries.push_back(std::move(e));
    }

    bool EnsureInitialized(const std::string& name)
    {
        std::lock_guard lock(m_mutex);
        for (auto& e : m_entries) {
            if (e.decoderName == name) {
                if (e.state == DecoderInitState::Ready)
                    return true;
                if (e.state == DecoderInitState::Failed)
                    return false;
                e.state = DecoderInitState::Initializing;
                e.initAttempts++;
                bool ok = e.initFn ? e.initFn() : false;
                e.state = ok ? DecoderInitState::Ready : DecoderInitState::Failed;
                return ok;
            }
        }
        return false;
    }

    LazyInitStats GetStats() const
    {
        LazyInitStats stats;
        stats.totalDecoders = static_cast<uint32_t>(m_entries.size());
        for (const auto& e : m_entries) {
            if (e.state == DecoderInitState::Ready)
                stats.initializedCount++;
            if (e.state == DecoderInitState::Failed)
                stats.failedCount++;
            stats.totalInitMs += e.initDurationMs;
        }
        return stats;
    }

  private:
    std::mutex m_mutex;
    std::vector<Entry> m_entries;
};

}  // namespace Engine
}  // namespace ExplorerLens
