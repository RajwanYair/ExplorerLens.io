#pragma once
// VulkanMemoryAllocator.h — Vulkan-specific memory pool management
// Sprint 422 — ExplorerLens v15.0.0 Zenith

#include <cstdint>
#include <string>
#include <array>

namespace ExplorerLens {
namespace Engine {

/// Vulkan memory type tier, mapping to VkMemoryPropertyFlags categories
enum class VkMemoryTier : uint8_t {
    DeviceLocal = 0,   // GPU-only, fastest for shader access
    HostVisible = 1,   // CPU-mappable, used for staging uploads
    HostCached = 2,   // CPU-cached readback memory
    LazyAlloc = 3,   // Lazily allocated (transient attachments)
    Shared = 4    // Unified / shared memory (iGPU or ReBAR)
};

inline const char* VkMemoryTierName(VkMemoryTier t) noexcept {
    switch (t) {
    case VkMemoryTier::DeviceLocal: return "DeviceLocal";
    case VkMemoryTier::HostVisible: return "HostVisible";
    case VkMemoryTier::HostCached:  return "HostCached";
    case VkMemoryTier::LazyAlloc:   return "LazyAlloc";
    case VkMemoryTier::Shared:      return "Shared";
    default:                        return "Unknown";
    }
}

/// Strategy used for allocating within a memory pool
enum class VkAllocStrategy : uint8_t {
    BestFit = 0,   // Find tightest fitting free block
    FirstFit = 1,   // Use first sufficiently large free block
    PoolBased = 2,   // Fixed-size slab pool allocation
    Dedicated = 3,   // One VkDeviceMemory per allocation
    Suballocate = 4    // Sub-allocate from large VkDeviceMemory
};

inline const char* VkAllocStrategyName(VkAllocStrategy s) noexcept {
    switch (s) {
    case VkAllocStrategy::BestFit:     return "BestFit";
    case VkAllocStrategy::FirstFit:    return "FirstFit";
    case VkAllocStrategy::PoolBased:   return "PoolBased";
    case VkAllocStrategy::Dedicated:   return "Dedicated";
    case VkAllocStrategy::Suballocate: return "Suballocate";
    default:                           return "Unknown";
    }
}

/// Describes a single allocated memory block
struct VkMemoryBlock {
    VkMemoryTier    tier = VkMemoryTier::DeviceLocal;
    VkAllocStrategy strategy = VkAllocStrategy::BestFit;
    uint64_t        offsetBytes = 0;      // Offset within the parent allocation
    uint64_t        sizeBytes = 0;      // Size of this block
    bool            isMapped = false;  // Whether CPU-mapped
};

/// Pool-based Vulkan memory allocator managing typed memory heaps with
/// configurable allocation strategies.  Tracks usage statistics and
/// enforces a hard block-count limit.
class VulkanMemoryAllocator {
public:
    static constexpr uint32_t MAX_BLOCKS = 256;

    VulkanMemoryAllocator() = default;
    ~VulkanMemoryAllocator() = default;

    VulkanMemoryAllocator(const VulkanMemoryAllocator&) = delete;
    VulkanMemoryAllocator& operator=(const VulkanMemoryAllocator&) = delete;
    VulkanMemoryAllocator(VulkanMemoryAllocator&&) noexcept = default;
    VulkanMemoryAllocator& operator=(VulkanMemoryAllocator&&) noexcept = default;

    /// Allocate a block of the given tier and size
    bool Allocate(VkMemoryTier tier, VkAllocStrategy strategy,
        uint64_t sizeBytes, VkMemoryBlock& outBlock) {
        if (m_blockCount >= MAX_BLOCKS) return false;
        if (sizeBytes == 0) return false;

        outBlock.tier = tier;
        outBlock.strategy = strategy;
        outBlock.offsetBytes = m_totalUsedBytes;
        outBlock.sizeBytes = sizeBytes;
        outBlock.isMapped = (tier == VkMemoryTier::HostVisible ||
            tier == VkMemoryTier::HostCached);

        m_blocks[m_blockCount] = outBlock;
        m_blockCount++;
        m_totalUsedBytes += sizeBytes;
        return true;
    }

    /// Free a block by index
    bool Free(uint32_t blockIndex) {
        if (blockIndex >= m_blockCount) return false;
        uint64_t freed = m_blocks[blockIndex].sizeBytes;
        // Shift remaining blocks down
        for (uint32_t i = blockIndex; i + 1 < m_blockCount; ++i) {
            m_blocks[i] = m_blocks[i + 1];
        }
        m_blockCount--;
        m_totalUsedBytes -= freed;
        return true;
    }

    /// Total bytes currently allocated
    uint64_t GetUsageBytes() const noexcept { return m_totalUsedBytes; }

    /// Current number of live blocks
    uint32_t GetBlockCount() const noexcept { return m_blockCount; }

    /// Check remaining capacity
    uint32_t GetFreeSlots() const noexcept { return MAX_BLOCKS - m_blockCount; }

private:
    std::array<VkMemoryBlock, MAX_BLOCKS> m_blocks{};
    uint32_t m_blockCount = 0;
    uint64_t m_totalUsedBytes = 0;
};

} // namespace Engine
} // namespace ExplorerLens
