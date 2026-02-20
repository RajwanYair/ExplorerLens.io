#pragma once
// =============================================================================
// ResourcePoolEngine.h — Sprint 240: Object Pool for Decoder/GPU Resource Reuse
// DarkThumbs Engine — Core Module
// =============================================================================

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <functional>
#include <memory>

namespace DarkThumbs {

/// Resource types managed by the pool
enum class ResourceType : uint32_t {
    DecoderContext  = 0,   ///< Archive/image decoder context
    GPUTexture      = 1,   ///< DirectX texture resource
    RenderTarget    = 2,   ///< Render target surface
    ComputeBuffer   = 3,   ///< Compute shader buffer
    StagingBuffer   = 4,   ///< CPU-GPU staging buffer
    CommandList     = 5,   ///< GPU command list
    Count           = 6
};

/// Resource state in the pool
enum class ResourceState : uint32_t {
    Available   = 0,   ///< Ready for checkout
    InUse       = 1,   ///< Currently checked out
    Expired     = 2,   ///< Past TTL, awaiting cleanup
    Corrupted   = 3    ///< Failed validation, must be destroyed
};

/// Configuration for resource pool behavior
struct PoolConfig {
    uint32_t    maxPoolSize       = 64;
    uint32_t    minPoolSize       = 4;
    uint32_t    ttlSeconds        = 300;    ///< Time-to-live for idle resources
    uint32_t    growthFactor      = 2;      ///< How many to create when pool is empty
    bool        enablePrewarming  = true;   ///< Pre-create minPoolSize on init
};

/// Represents a pooled resource entry
struct PooledResource {
    uint64_t        id          = 0;
    ResourceType    type        = ResourceType::DecoderContext;
    ResourceState   state       = ResourceState::Available;
    uint64_t        createdAt   = 0;    ///< Timestamp ms
    uint64_t        lastUsedAt  = 0;    ///< Timestamp ms
    uint32_t        checkoutCount = 0;  ///< Number of times reused
    size_t          sizeBytes   = 0;    ///< Memory footprint
};

/// Pool statistics
struct PoolStats {
    uint32_t    totalCreated    = 0;
    uint32_t    totalDestroyed  = 0;
    uint32_t    totalCheckouts  = 0;
    uint32_t    totalReturns    = 0;
    uint32_t    cacheHits       = 0;
    uint32_t    cacheMisses     = 0;
    uint32_t    currentSize     = 0;
    uint32_t    peakSize        = 0;
    double      hitRate         = 0.0;
};

/// ResourcePoolEngine — manages reusable resource pools for decoder and GPU objects
class ResourcePoolEngine {
public:
    ResourcePoolEngine();
    explicit ResourcePoolEngine(const PoolConfig& config);

    // Pool lifecycle
    bool Initialize();
    void Shutdown();

    // Resource checkout/return
    PooledResource Checkout(ResourceType type);
    bool Return(uint64_t resourceId);

    // Pool maintenance
    uint32_t EvictExpired();
    uint32_t Prewarm(ResourceType type, uint32_t count);

    // Queries
    uint32_t GetAvailableCount(ResourceType type) const;
    uint32_t GetInUseCount(ResourceType type) const;
    PoolStats GetStats() const;
    const PoolConfig& GetConfig() const { return m_config; }

    // Static helpers
    static const wchar_t* GetResourceTypeName(ResourceType type);
    static const wchar_t* GetResourceStateName(ResourceState state);
    static constexpr uint32_t GetResourceTypeCount() { return static_cast<uint32_t>(ResourceType::Count); }

private:
    PoolConfig                  m_config;
    std::vector<PooledResource> m_resources;
    PoolStats                   m_stats;
    uint64_t                    m_nextId = 1;
    bool                        m_initialized = false;

    PooledResource CreateResource(ResourceType type);
};

} // namespace DarkThumbs
