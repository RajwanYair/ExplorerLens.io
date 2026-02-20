// =============================================================================
// ResourcePoolEngine.cpp — Sprint 240: Object Pool for Decoder/GPU Resource Reuse
// DarkThumbs Engine — Core Module
// =============================================================================

#include "ResourcePoolEngine.h"
#include <algorithm>
#include <chrono>

namespace DarkThumbs {

static uint64_t NowMs() {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

ResourcePoolEngine::ResourcePoolEngine() : m_config{} {}

ResourcePoolEngine::ResourcePoolEngine(const PoolConfig& config) : m_config(config) {}

bool ResourcePoolEngine::Initialize() {
    if (m_initialized) return false;
    m_resources.clear();
    m_stats = {};
    m_nextId = 1;
    if (m_config.enablePrewarming) {
        for (uint32_t t = 0; t < static_cast<uint32_t>(ResourceType::Count); ++t) {
            Prewarm(static_cast<ResourceType>(t), m_config.minPoolSize);
        }
    }
    m_initialized = true;
    return true;
}

void ResourcePoolEngine::Shutdown() {
    m_stats.totalDestroyed += static_cast<uint32_t>(m_resources.size());
    m_resources.clear();
    m_stats.currentSize = 0;
    m_initialized = false;
}

PooledResource ResourcePoolEngine::CreateResource(ResourceType type) {
    PooledResource r;
    r.id = m_nextId++;
    r.type = type;
    r.state = ResourceState::Available;
    r.createdAt = NowMs();
    r.lastUsedAt = r.createdAt;
    r.checkoutCount = 0;
    r.sizeBytes = 4096; // Default size estimate
    m_stats.totalCreated++;
    return r;
}

PooledResource ResourcePoolEngine::Checkout(ResourceType type) {
    // Try to find an available resource of the requested type
    for (auto& r : m_resources) {
        if (r.type == type && r.state == ResourceState::Available) {
            r.state = ResourceState::InUse;
            r.lastUsedAt = NowMs();
            r.checkoutCount++;
            m_stats.totalCheckouts++;
            m_stats.cacheHits++;
            if (m_stats.totalCheckouts > 0) {
                m_stats.hitRate = static_cast<double>(m_stats.cacheHits) / m_stats.totalCheckouts;
            }
            return r;
        }
    }
    // No available resource — create new one if under limit
    m_stats.cacheMisses++;
    m_stats.totalCheckouts++;
    if (m_stats.totalCheckouts > 0) {
        m_stats.hitRate = static_cast<double>(m_stats.cacheHits) / m_stats.totalCheckouts;
    }
    if (m_resources.size() < m_config.maxPoolSize) {
        auto r = CreateResource(type);
        r.state = ResourceState::InUse;
        r.checkoutCount = 1;
        m_resources.push_back(r);
        m_stats.currentSize = static_cast<uint32_t>(m_resources.size());
        if (m_stats.currentSize > m_stats.peakSize) m_stats.peakSize = m_stats.currentSize;
        return r;
    }
    // Pool exhausted — return empty resource
    PooledResource empty;
    empty.id = 0;
    empty.state = ResourceState::Corrupted;
    return empty;
}

bool ResourcePoolEngine::Return(uint64_t resourceId) {
    for (auto& r : m_resources) {
        if (r.id == resourceId && r.state == ResourceState::InUse) {
            r.state = ResourceState::Available;
            r.lastUsedAt = NowMs();
            m_stats.totalReturns++;
            return true;
        }
    }
    return false;
}

uint32_t ResourcePoolEngine::EvictExpired() {
    uint64_t now = NowMs();
    uint32_t evicted = 0;
    auto it = std::remove_if(m_resources.begin(), m_resources.end(),
        [&](const PooledResource& r) {
            if (r.state == ResourceState::Available &&
                (now - r.lastUsedAt) > (static_cast<uint64_t>(m_config.ttlSeconds) * 1000)) {
                evicted++;
                return true;
            }
            if (r.state == ResourceState::Corrupted) {
                evicted++;
                return true;
            }
            return false;
        });
    m_resources.erase(it, m_resources.end());
    m_stats.totalDestroyed += evicted;
    m_stats.currentSize = static_cast<uint32_t>(m_resources.size());
    return evicted;
}

uint32_t ResourcePoolEngine::Prewarm(ResourceType type, uint32_t count) {
    uint32_t created = 0;
    for (uint32_t i = 0; i < count && m_resources.size() < m_config.maxPoolSize; ++i) {
        m_resources.push_back(CreateResource(type));
        created++;
    }
    m_stats.currentSize = static_cast<uint32_t>(m_resources.size());
    if (m_stats.currentSize > m_stats.peakSize) m_stats.peakSize = m_stats.currentSize;
    return created;
}

uint32_t ResourcePoolEngine::GetAvailableCount(ResourceType type) const {
    uint32_t count = 0;
    for (const auto& r : m_resources) {
        if (r.type == type && r.state == ResourceState::Available) count++;
    }
    return count;
}

uint32_t ResourcePoolEngine::GetInUseCount(ResourceType type) const {
    uint32_t count = 0;
    for (const auto& r : m_resources) {
        if (r.type == type && r.state == ResourceState::InUse) count++;
    }
    return count;
}

PoolStats ResourcePoolEngine::GetStats() const { return m_stats; }

const wchar_t* ResourcePoolEngine::GetResourceTypeName(ResourceType type) {
    switch (type) {
        case ResourceType::DecoderContext: return L"Decoder Context";
        case ResourceType::GPUTexture:    return L"GPU Texture";
        case ResourceType::RenderTarget:  return L"Render Target";
        case ResourceType::ComputeBuffer: return L"Compute Buffer";
        case ResourceType::StagingBuffer: return L"Staging Buffer";
        case ResourceType::CommandList:   return L"Command List";
        default:                          return L"Unknown";
    }
}

const wchar_t* ResourcePoolEngine::GetResourceStateName(ResourceState state) {
    switch (state) {
        case ResourceState::Available: return L"Available";
        case ResourceState::InUse:     return L"In Use";
        case ResourceState::Expired:   return L"Expired";
        case ResourceState::Corrupted: return L"Corrupted";
        default:                       return L"Unknown";
    }
}

} // namespace DarkThumbs
