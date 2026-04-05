// EmbeddingPersistenceEngine.h — Persist CLIP embeddings across sessions
// Copyright (c) 2026 ExplorerLens Project
//
// Writes CLIP ViT-B/32 (512-dim float) embeddings and their file paths to an
// append-only binary journal on disk so the HNSW index survives application
// restart without re-embedding the entire library. Supports incremental flush
// and full-rebuild load at startup via LoadAll().
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct PersistedEmbedding {
    uint32_t itemId = 0;
    std::wstring filePath;
    float vector[512]{};
};

struct EmbeddingJournalStats {
    uint32_t totalEntries = 0;
    uint32_t flushCount = 0;
    float lastFlushMs = 0.0f;
    uint64_t journalBytes = 0;
};

class EmbeddingPersistenceEngine {
public:
    static EmbeddingPersistenceEngine& Instance();

    bool Open(const std::wstring& journalPath);
    bool Append(const PersistedEmbedding& entry);
    bool Flush();
    bool LoadAll(std::vector<PersistedEmbedding>& out) const;
    void Close();

    bool                  IsOpen() const noexcept { return m_isOpen; }
    EmbeddingJournalStats Stats()  const noexcept { return m_stats; }

private:
    bool m_isOpen = false;
    EmbeddingJournalStats m_stats{};
    std::wstring m_journalPath;
};

}} // namespace ExplorerLens::Engine
