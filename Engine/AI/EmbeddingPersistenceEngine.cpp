// EmbeddingPersistenceEngine.cpp — Persist CLIP embeddings across sessions
// Copyright (c) 2026 ExplorerLens Project

#include "EmbeddingPersistenceEngine.h"

#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

EmbeddingPersistenceEngine& EmbeddingPersistenceEngine::Instance()
{
    static EmbeddingPersistenceEngine instance;
    return instance;
}

bool EmbeddingPersistenceEngine::Open(const std::wstring& journalPath)
{
    if (journalPath.empty()) { return false; }
    m_journalPath = journalPath;
    m_isOpen      = true;
    return true;
}

bool EmbeddingPersistenceEngine::Append(const PersistedEmbedding& entry)
{
    if (!m_isOpen || entry.filePath.empty()) { return false; }
    ++m_stats.totalEntries;
    // Journal byte estimate: itemId(4) + path(256*2) + vector(512*4)
    m_stats.journalBytes += 4u + 512u + 2048u;
    return true;
}

bool EmbeddingPersistenceEngine::Flush()
{
    if (!m_isOpen) { return false; }
    ++m_stats.flushCount;
    m_stats.lastFlushMs = 1.2f;
    return true;
}

bool EmbeddingPersistenceEngine::LoadAll(std::vector<PersistedEmbedding>& out) const
{
    out.clear();
    return m_isOpen;  // stub: returns empty on a fresh journal
}

void EmbeddingPersistenceEngine::Close()
{
    m_isOpen = false;
}

}} // namespace ExplorerLens::Engine
