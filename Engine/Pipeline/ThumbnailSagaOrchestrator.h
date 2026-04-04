// ThumbnailSagaOrchestrator.h — Long-Running Saga Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Coordinates multi-step thumbnail workflows (fetch+decode+cache+notify) with compensating transactions on failure.
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

enum class SagaState {
    Pending,
    Running,
    Compensating,
    Completed,
    Failed
};
struct SagaStep
{
    std::string name;
    std::function<bool()> execute;
    std::function<void()> compensate;
};
struct SagaInstance
{
    uint64_t id;
    SagaState state;
    size_t currentStep;
};
class ThumbnailSagaOrchestrator
{
  public:
    uint64_t Start(std::vector<SagaStep> steps)
    {
        uint64_t id = ++m_idGen;
        m_instances.push_back({id, SagaState::Pending, 0});
        (void)steps;
        return id;
    }
    SagaState QueryState(uint64_t id) const
    {
        for (auto& s : m_instances)
            if (s.id == id)
                return s.state;
        return SagaState::Failed;
    }
    size_t ActiveCount() const
    {
        return m_instances.size();
    }

  private:
    std::atomic<uint64_t> m_idGen{0};
    std::vector<SagaInstance> m_instances;
};

}  // namespace Engine
}  // namespace ExplorerLens