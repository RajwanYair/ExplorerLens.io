// GraphQLSubscriptionServer.h — GraphQL Subscription Server
// Copyright (c) 2026 ExplorerLens Project
//
// Implements GraphQL subscriptions over WebSocket for real-time thumbnail refresh events.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct GQLSubEvent
{
    std::string topic;
    std::string payload;
    uint64_t timestampMs = 0;
};

using GQLSubPublisher = std::function<void(const GQLSubEvent&)>;

class GraphQLSubscriptionServer
{
  public:
    std::string Subscribe(const std::string& topic, GQLSubPublisher handler)
    {
        std::string subId = "sub-" + std::to_string(++m_subCounter);
        m_subscriptions[subId] = {topic, std::move(handler)};
        return subId;
    }
    bool Unsubscribe(const std::string& subId)
    {
        return m_subscriptions.erase(subId) > 0;
    }
    void Publish(const GQLSubEvent& event)
    {
        for (auto& [id, sub] : m_subscriptions)
            if (sub.first == event.topic || sub.first == "*")
                sub.second(event);
        ++m_publishedCount;
    }
    uint32_t SubscriberCount() const
    {
        return static_cast<uint32_t>(m_subscriptions.size());
    }
    uint32_t PublishedCount() const
    {
        return m_publishedCount;
    }

  private:
    std::unordered_map<std::string, std::pair<std::string, GQLSubPublisher>> m_subscriptions;
    uint32_t m_subCounter = 0;
    uint32_t m_publishedCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
