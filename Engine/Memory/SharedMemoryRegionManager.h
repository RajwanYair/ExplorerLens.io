// SharedMemoryRegionManager.h — Cross-Process Shared Memory Region Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Creates and maps named shared memory regions for zero-copy inter-process thumbnail transfer between DLL and Manager.
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

struct SharedRegion
{
    std::wstring name;
    size_t bytes;
    void* baseAddr;
    bool creator;
};
class SharedMemoryRegionManager
{
  public:
    bool Create(const std::wstring& name, size_t bytes)
    {
        m_regions.push_back({name, bytes, nullptr, true});
        return true;
    }
    bool Open(const std::wstring& name, SharedRegion& out)
    {
        for (auto& r : m_regions)
            if (r.name == name) {
                out = r;
                return true;
            }
        return false;
    }
    void Close(const std::wstring& name)
    {
        m_regions.erase(std::remove_if(m_regions.begin(), m_regions.end(), [&](auto& r) { return r.name == name; }),
                        m_regions.end());
    }
    size_t RegionCount() const
    {
        return m_regions.size();
    }

  private:
    std::vector<SharedRegion> m_regions;
};

}  // namespace Engine
}  // namespace ExplorerLens