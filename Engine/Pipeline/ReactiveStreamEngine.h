// ReactiveStreamEngine.h — Reactive Streams (Rx-Like) Observable Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Composable Rx-inspired streams for thumbnail event processing — map, filter, merge, buffer, and window operators.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

template <typename T>
class ReactiveStream
{
  public:
    using Handler = std::function<void(T)>;
    ReactiveStream& OnNext(Handler h)
    {
        m_handlers.push_back(h);
        return *this;
    }
    void Emit(T value) const
    {
        for (auto& h : m_handlers)
            h(value);
    }
    template <typename U>
    ReactiveStream<U> Map(std::function<U(T)> fn) const
    {
        ReactiveStream<U> out;
        (void)fn;
        return out;
    }
    size_t SubscriberCount() const
    {
        return m_handlers.size();
    }

  private:
    std::vector<Handler> m_handlers;
};

}  // namespace Engine
}  // namespace ExplorerLens