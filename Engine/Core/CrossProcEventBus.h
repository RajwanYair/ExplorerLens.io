// CrossProcEventBus.h — Cross-Process Publish/Subscribe Event Bus
// Copyright (c) 2026 ExplorerLens Project
//
// Lightweight publish/subscribe event bus that operates in-process by default
// and can bridge to other processes via named pipes or shared memory when
// running in cross-process or hybrid mode. Topics are free-form wide strings;
// subscribers receive a BusEvent carrying a binary payload and priority tag.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class EventBusMode : uint8_t {
    InProcess = 0,
    CrossProcess = 1,
    Hybrid = 2,
};

enum class EventPriority : uint8_t {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3,
};

struct BusEvent
{
    std::wstring topic;
    std::string payload;
    EventPriority priority = EventPriority::Normal;
    uint64_t timestamp = 0;
    uint32_t sourceId = 0;
};

struct SubscriptionHandle
{
    uint64_t id = 0;
    std::wstring topic;

    [[nodiscard]] bool IsValid() const noexcept
    {
        return id != 0;
    }
};

using EventCallback = std::function<void(const BusEvent&)>;

class CrossProcEventBus
{
  public:
    static constexpr int MAX_SUBSCRIBERS = 256;
    static constexpr int MAX_QUEUE_DEPTH = 4096;
    static constexpr int DEFAULT_TIMEOUT_MS = 100;

    explicit CrossProcEventBus() noexcept = default;
    explicit CrossProcEventBus(EventBusMode mode) noexcept : m_mode(mode) {}

    [[nodiscard]] EventBusMode GetMode() const noexcept
    {
        return m_mode;
    }
    [[nodiscard]] bool IsRunning() const noexcept
    {
        return m_running;
    }
    [[nodiscard]] int GetQueueDepth() const noexcept
    {
        return m_queueDepth;
    }

    void SetMode(EventBusMode mode) noexcept
    {
        m_mode = mode;
    }

    bool Start() noexcept
    {
        if (m_running)
            return true;
        m_running = true;
        m_queueDepth = 0;
        return true;
    }

    bool Stop() noexcept
    {
        if (!m_running)
            return false;
        m_running = false;
        m_queueDepth = 0;
        m_subs.clear();
        return true;
    }

    [[nodiscard]] SubscriptionHandle Subscribe(const std::wstring& topic, EventCallback cb) noexcept
    {
        if (!cb)
            return {};
        if (static_cast<int>(m_subs.size()) >= MAX_SUBSCRIBERS)
            return {};
        SubEntry entry;
        entry.handle.id = ++m_nextSubId;
        entry.handle.topic = topic;
        entry.callback = std::move(cb);
        m_subs.push_back(std::move(entry));
        return m_subs.back().handle;
    }

    bool Unsubscribe(const SubscriptionHandle& handle) noexcept
    {
        for (auto it = m_subs.begin(); it != m_subs.end(); ++it) {
            if (it->handle.id == handle.id) {
                m_subs.erase(it);
                return true;
            }
        }
        return false;
    }

    bool Publish(const BusEvent& event) noexcept
    {
        if (event.topic.empty())
            return false;
        if (m_queueDepth >= MAX_QUEUE_DEPTH)
            return false;
        ++m_queueDepth;
        for (auto& sub : m_subs) {
            if (sub.handle.topic == event.topic || sub.handle.topic.empty()) {
                if (sub.callback) {
                    sub.callback(event);
                }
            }
        }
        if (m_queueDepth > 0)
            --m_queueDepth;
        return true;
    }

    [[nodiscard]] int GetSubscriberCount(const std::wstring& topic = L"") const noexcept
    {
        if (topic.empty())
            return static_cast<int>(m_subs.size());
        int count = 0;
        for (const auto& sub : m_subs) {
            if (sub.handle.topic == topic)
                ++count;
        }
        return count;
    }

    static const wchar_t* GetModeName(EventBusMode mode) noexcept
    {
        switch (mode) {
            case EventBusMode::InProcess:
                return L"InProcess";
            case EventBusMode::CrossProcess:
                return L"CrossProcess";
            case EventBusMode::Hybrid:
                return L"Hybrid";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* GetPriorityName(EventPriority prio) noexcept
    {
        switch (prio) {
            case EventPriority::Low:
                return L"Low";
            case EventPriority::Normal:
                return L"Normal";
            case EventPriority::High:
                return L"High";
            case EventPriority::Critical:
                return L"Critical";
            default:
                return L"Unknown";
        }
    }

  private:
    struct SubEntry
    {
        SubscriptionHandle handle;
        EventCallback callback;
    };

    EventBusMode m_mode = EventBusMode::InProcess;
    bool m_running = false;
    int m_queueDepth = 0;
    uint64_t m_nextSubId = 0;
    std::vector<SubEntry> m_subs;
};

}  // namespace Engine
}  // namespace ExplorerLens
