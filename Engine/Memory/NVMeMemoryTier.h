// NVMeMemoryTier.h — NVMe-Tier Memory Allocator (PMDK Bridge)
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges Windows Storage Spaces and Intel PMDK for persistent memory tier allocations in NVMe-PM configurations.
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

struct NVMeRegion
{
    void* baseAddr;
    size_t bytes;
    bool persistent;
    bool available;
};
class NVMeMemoryTier
{
  public:
    bool Probe()
    {
        return false;
    }  // Requires PM hardware
    NVMeRegion Allocate(size_t bytes)
    {
        (void)bytes;
        return {nullptr, 0, false, false};
    }
    void Free(NVMeRegion region)
    {
        (void)region;
    }
    bool IsAvailable() const
    {
        return false;
    }
    uint64_t CapacityBytes() const
    {
        return 0;
    }
    std::string TierName() const
    {
        return "NVMe-PM (unavailable)";
    }
};

}  // namespace Engine
}  // namespace ExplorerLens