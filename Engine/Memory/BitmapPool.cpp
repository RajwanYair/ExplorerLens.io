// BitmapPool.cpp — Pre-allocated Bitmap Pool Implementation
// Copyright (c) 2026 ExplorerLens Project

#include "BitmapPool.h"
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

BitmapPool& BitmapPool::Instance()
{
    static BitmapPool instance;
    return instance;
}

BitmapPool::BitmapPool() = default;

BitmapPool::~BitmapPool()
{
    Shutdown();
}

void BitmapPool::Initialize()
{
    // Default tiers for common thumbnail sizes
    std::vector<BitmapPoolConfig> defaults = {
        {128, 128, 50, 32},
        {256, 256, 50, 32},
        {512, 512, 30, 32},
        {1024, 1024, 10, 32},
    };
    Initialize(defaults);
}

void BitmapPool::Initialize(const std::vector<BitmapPoolConfig>& configs)
{
    std::lock_guard<std::mutex> guard(m_tiersMutex);
    if (m_initialized)
        return;

    for (const auto& cfg : configs) {
        uint64_t key = MakeKey(cfg.width, cfg.height);
        PoolTier tier;
        tier.width = cfg.width;
        tier.height = cfg.height;
        tier.maxSize = cfg.poolSize;
        tier.available.reserve(cfg.poolSize);

        // Pre-allocate half the pool to reduce cold-start latency
        uint32_t prealloc = cfg.poolSize / 2;
        for (uint32_t i = 0; i < prealloc; i++) {
            HBITMAP bmp = CreatePoolBitmap(cfg.width, cfg.height);
            if (bmp) {
                tier.available.push_back(bmp);
            }
        }

        m_tiers[key] = std::move(tier);
    }

    // Reset stats
    AcquireSRWLockExclusive(&m_statsLock);
    m_stats = {};
    uint64_t totalPooled = 0;
    for (const auto& [key, tier] : m_tiers) {
        totalPooled += tier.available.size();
    }
    m_stats.currentPooled = totalPooled;
    m_stats.peakPooled = totalPooled;
    ReleaseSRWLockExclusive(&m_statsLock);

    m_initialized = true;
}

void BitmapPool::Shutdown()
{
    std::lock_guard<std::mutex> guard(m_tiersMutex);

    for (auto& [key, tier] : m_tiers) {
        AcquireSRWLockExclusive(&tier.lock);
        for (HBITMAP bmp : tier.available) {
            if (bmp)
                DeleteObject(bmp);
        }
        tier.available.clear();
        ReleaseSRWLockExclusive(&tier.lock);
    }
    m_tiers.clear();

    AcquireSRWLockExclusive(&m_originLock);
    m_bitmapOrigin.clear();
    ReleaseSRWLockExclusive(&m_originLock);

    AcquireSRWLockExclusive(&m_statsLock);
    m_stats.currentPooled = 0;
    ReleaseSRWLockExclusive(&m_statsLock);

    m_initialized = false;
}

HBITMAP BitmapPool::Acquire(uint32_t width, uint32_t height)
{
    uint64_t key = MakeKey(width, height);
    HBITMAP result = nullptr;
    bool fromPool = false;

    // Try to get from pool tier
    {
        std::lock_guard<std::mutex> guard(m_tiersMutex);
        auto it = m_tiers.find(key);
        if (it != m_tiers.end()) {
            PoolTier& tier = it->second;
            AcquireSRWLockExclusive(&tier.lock);
            if (!tier.available.empty()) {
                result = tier.available.back();
                tier.available.pop_back();
                fromPool = true;
            }
            ReleaseSRWLockExclusive(&tier.lock);
        }
    }

    // If pool miss, create new bitmap
    if (!result) {
        result = CreatePoolBitmap(width, height);
    }

    if (result) {
        // Track origin for proper release
        AcquireSRWLockExclusive(&m_originLock);
        m_bitmapOrigin[result] = key;
        ReleaseSRWLockExclusive(&m_originLock);

        // Update stats
        AcquireSRWLockExclusive(&m_statsLock);
        m_stats.acquireCount++;
        if (fromPool) {
            m_stats.poolHits++;
            m_stats.currentPooled--;
        } else {
            m_stats.poolMisses++;
        }
        ReleaseSRWLockExclusive(&m_statsLock);
    }

    return result;
}

void BitmapPool::Release(HBITMAP hBitmap)
{
    if (!hBitmap)
        return;

    // Find the origin key for this bitmap
    uint64_t key = 0;
    bool tracked = false;
    {
        AcquireSRWLockExclusive(&m_originLock);
        auto it = m_bitmapOrigin.find(hBitmap);
        if (it != m_bitmapOrigin.end()) {
            key = it->second;
            tracked = true;
            m_bitmapOrigin.erase(it);
        }
        ReleaseSRWLockExclusive(&m_originLock);
    }

    if (!tracked) {
        // Bitmap wasn't from our pool — just delete it
        DeleteObject(hBitmap);
        return;
    }

    bool returned = false;

    // Try to return to pool
    {
        std::lock_guard<std::mutex> guard(m_tiersMutex);
        auto it = m_tiers.find(key);
        if (it != m_tiers.end()) {
            PoolTier& tier = it->second;
            AcquireSRWLockExclusive(&tier.lock);
            if (tier.available.size() < tier.maxSize) {
                // Clear the bitmap data before pooling (security)
                BITMAP bm;
                if (GetObject(hBitmap, sizeof(bm), &bm) && bm.bmBits) {
                    memset(bm.bmBits, 0, bm.bmWidthBytes * bm.bmHeight);
                }
                tier.available.push_back(hBitmap);
                returned = true;
            }
            ReleaseSRWLockExclusive(&tier.lock);
        }
    }

    if (!returned) {
        // Pool is full — destroy bitmap
        DeleteObject(hBitmap);
    }

    // Update stats
    AcquireSRWLockExclusive(&m_statsLock);
    m_stats.releaseCount++;
    if (returned) {
        m_stats.currentPooled++;
        if (m_stats.currentPooled > m_stats.peakPooled) {
            m_stats.peakPooled = m_stats.currentPooled;
        }
    }
    ReleaseSRWLockExclusive(&m_statsLock);
}

BitmapPoolStats BitmapPool::GetStats() const
{
    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
    BitmapPoolStats copy = m_stats;
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
    return copy;
}

void BitmapPool::ResetStats()
{
    AcquireSRWLockExclusive(&m_statsLock);

    // Keep currentPooled and peakPooled, reset counters
    m_stats.acquireCount = 0;
    m_stats.releaseCount = 0;
    m_stats.poolHits = 0;
    m_stats.poolMisses = 0;
    m_stats.peakPooled = m_stats.currentPooled;

    ReleaseSRWLockExclusive(&m_statsLock);
}

HBITMAP BitmapPool::CreatePoolBitmap(uint32_t width, uint32_t height)
{
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(width);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(height);  // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = width * height * 4;

    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    return bmp;
}

}  // namespace Engine
}  // namespace ExplorerLens
