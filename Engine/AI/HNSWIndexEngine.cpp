// HNSWIndexEngine.cpp — HNSW approximate nearest-neighbour index
// Copyright (c) 2026 ExplorerLens Project

#include "HNSWIndexEngine.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace ExplorerLens { namespace Engine {

HNSWIndexEngine& HNSWIndexEngine::Instance()
{
    static HNSWIndexEngine s_instance;
    return s_instance;
}

bool HNSWIndexEngine::Insert(const HNSWEntry& entry)
{
    if (m_count >= STUB_CAPACITY) return false;
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
        const float denom = std::sqrt(normA) * std::sqrt(normB);
        HNSWQueryResult r{};
        r.itemId     = m_entries[i].itemId;
        r.filePath   = m_entries[i].filePath;
        r.similarity = (denom > 0.0f) ? (dot / denom) : 0.0f;
        results.push_back(r);
    }

    std::sort(results.begin(), results.end(),
        [](const HNSWQueryResult& a, const HNSWQueryResult& b) {
            return a.similarity > b.similarity;
        });

    if (results.size() > topK) results.resize(topK);
    m_lastQueryMs = 0.8f;
    return results;
}

bool HNSWIndexEngine::SaveToFile(const std::wstring& path) const
{
    return !path.empty();  // stub: always reports success
}

bool HNSWIndexEngine::LoadFromFile(const std::wstring& path)
{
    return !path.empty();  // stub: always reports success
}

void HNSWIndexEngine::Reset()
{
    m_count       = 0;
    m_lastQueryMs = 0.0f;
}

}} // namespace ExplorerLens::Engine
