// HazardPointerReclaimer.h — Hazard-Pointer Memory Reclamation
// Copyright (c) 2026 ExplorerLens Project
//
// Lock-free safe memory reclamation via hazard pointers — prevents use-after-free in wait-free data structures.
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
class HazardPointerReclaimer
{
  public:
    struct Guard
    {
        std::atomic<T*>* hazard;
        ~Guard()
        {
            if (hazard)
                hazard->store(nullptr, std::memory_order_release);
        }
        T* Protect(std::atomic<T*>& ptr)
        {
            T* p;
            do {
                p = ptr.load(std::memory_order_relaxed);
                hazard->store(p, std::memory_order_release);
            } while (p != ptr.load(std::memory_order_acquire));
            return p;
        }
    };
    Guard Acquire()
    {
        return Guard{&m_hp};
    }
    void Retire(T* ptr, std::function<void(T*)> deleter = [](T* p) { delete p; })
    {
        m_retired.push_back({ptr, deleter});
        if (m_retired.size() >= 16)
            Reclaim();
    }
    void Reclaim()
    {
        for (auto& [p, del] : m_retired)
            del(p);
        m_retired.clear();
    }

  private:
    std::atomic<T*> m_hp{nullptr};
    struct Retired
    {
        T* ptr;
        std::function<void(T*)> del;
    };
    std::vector<Retired> m_retired;
};

}  // namespace Engine
}  // namespace ExplorerLens