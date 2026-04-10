// PartialDecodeStateCache.cpp — Partial Decode State Cache
// Copyright (c) 2026 ExplorerLens Project
//
#include "PartialDecodeStateCache.h"
#include <algorithm>
#include <chrono>

namespace ExplorerLens { namespace Engine {

static uint64_t NowMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

PartialDecodeStateCache::PartialDecodeStateCache(const Config& cfg)
    : m_config(cfg)
{}

void PartialDecodeStateCache::TouchLRU(const std::wstring& key) const
{
    auto it = std::find(m_accessOrder.begin(), m_accessOrder.end(), key);
    if (it != m_accessOrder.end()) m_accessOrder.erase(it);
    m_accessOrder.insert(m_accessOrder.begin(), key);
}

void PartialDecodeStateCache::Put(const std::wstring& filePath, PartialDecodeState state)
{
    state.capturedAt = NowMs();
    m_store[filePath] = std::move(state);
    TouchLRU(filePath);
    Evict();
}

const PartialDecodeState* PartialDecodeStateCache::Get(
    const std::wstring& filePath, uint64_t currentMtimeMs) const
{
    auto it = m_store.find(filePath);
    if (it == m_store.end()) return nullptr;

    const auto& entry = it->second;

    // Staleness check
    if (entry.mtimeMs != currentMtimeMs) return nullptr;

    // TTL check
    if (m_config.ttlSeconds > 0) {
        const uint64_t ageMs = NowMs() - entry.capturedAt;
        if (ageMs > static_cast<uint64_t>(m_config.ttlSeconds) * 1000u) return nullptr;
    }

    TouchLRU(filePath);
    return &entry;
}

void PartialDecodeStateCache::Invalidate(const std::wstring& filePath)
{
    m_store.erase(filePath);
    auto it = std::find(m_accessOrder.begin(), m_accessOrder.end(), filePath);
    if (it != m_accessOrder.end()) m_accessOrder.erase(it);
}

uint32_t PartialDecodeStateCache::Evict()
{
    uint32_t evicted = 0;

    // Evict by LRU until within maxEntries
    while (m_store.size() > m_config.maxEntries && !m_accessOrder.empty()) {
        const auto& victim = m_accessOrder.back();
        m_store.erase(victim);
        m_accessOrder.pop_back();
        ++evicted;
    }

    // Evict by total blob size budget
    while (TotalBlobBytes() > m_config.maxBlobBytes && !m_accessOrder.empty()) {
        const auto& victim = m_accessOrder.back();
        m_store.erase(victim);
        m_accessOrder.pop_back();
        ++evicted;
    }

    return evicted;
}

uint32_t PartialDecodeStateCache::EntryCount() const
{
    return static_cast<uint32_t>(m_store.size());
}

uint64_t PartialDecodeStateCache::TotalBlobBytes() const
{
    uint64_t total = 0;
    for (const auto& kv : m_store) total += kv.second.headerBlob.size();
    return total;
}

void PartialDecodeStateCache::Clear()
{
    m_store.clear();
    m_accessOrder.clear();
}

const PartialDecodeStateCache::Config& PartialDecodeStateCache::GetConfig() const
{
    return m_config;
}

}} // namespace ExplorerLens::Engine
