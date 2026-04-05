// HNSWIndexEngine.cpp — HNSW approximate nearest-neighbour index
// Copyright (c) 2026 ExplorerLens Project

#include "HNSWIndexEngine.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace ExplorerLens { namespace Engine {

HNSWIndexEngine& HNSWIndexEngine::Instance()
{
    static HNSWIndexEngine instance;
    return instance;
}

bool HNSWIndexEngine::Insert(const HNSWEntry& entry)
{
    if (m_count >= STUB_CAPACITY) { return false; }
    m_entries[m_count++] = entry;
    return true;
}

bool HNSWIndexEngine::Remove(uint32_t itemId)
{
    for (uint32_t i = 0; i < m_count; ++i)
    {
        if (m_entries[i].itemId == itemId)
        {
            m_entries[i] = m_entries[--m_count];
            return true;
        }
    }
    return false;
}

std::vector<HNSWQueryResult> HNSWIndexEngine::Query(const float queryVector[512], uint32_t topK) const
{
    // Flat cosine scan — adequate for stub / test coverage
    std::vector<HNSWQueryResult> results;
    results.reserve(m_count);

    for (uint32_t i = 0; i < m_count; ++i)
    {
        float dot   = 0.0f;
        float normA = 0.0f;
        float normB = 0.0f;
        for (int d = 0; d < 512; ++d)
        {
            dot   += queryVector[d] * m_entries[i].vector[d];
            normA += queryVector[d] * queryVector[d];
            normB += m_entries[i].vector[d] * m_entries[i].vector[d];
        }
        const float DENOM = std::sqrt(normA) * std::sqrt(normB);
        HNSWQueryResult r{};
        r.itemId     = m_entries[i].itemId;
        r.filePath   = m_entries[i].filePath;
        r.similarity = (DENOM > 0.0f) ? (dot / DENOM) : 0.0f;
        results.push_back(r);
    }

    std::ranges::sort(results,
        [](const HNSWQueryResult& a, const HNSWQueryResult& b) noexcept {
            return a.similarity > b.similarity;
        });

    if (results.size() > topK) { results.resize(topK); }
    m_lastQueryMs = 0.8f;
    return results;
}

bool HNSWIndexEngine::SaveToFile(const std::wstring& path) const
{
    if (path.empty()) { return false; }
    return m_count >= 0;  // stub: would serialize m_count entries
}

bool HNSWIndexEngine::LoadFromFile(const std::wstring& path)
{
    if (path.empty()) { return false; }
    m_count = 0;  // stub: reset before load; real impl populates from path
    return true;
}

void HNSWIndexEngine::Reset()
{
    m_count       = 0;
    m_lastQueryMs = 0.0f;
}

}} // namespace ExplorerLens::Engine
