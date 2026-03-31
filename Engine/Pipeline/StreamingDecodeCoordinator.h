// StreamingDecodeCoordinator.h — Streaming Decode Pipeline Coordinator
// Copyright (c) 2026 ExplorerLens Project
//
// Coordinates streaming decode operations for large files by breaking them
// into progressive chunks. Provides partial thumbnail rendering while decode
// continues. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class StreamingDecodeState : uint8_t {
    Idle,
    HeaderParsed,
    PartialDecode,
    FullDecode,
    Complete,
    Failed
};

enum class StreamingChunkPriority : uint8_t {
    Header,
    Preview,
    Detail,
    Metadata,
    Background
};

struct StreamingDecodeRequest {
    std::wstring filePath;
    uint64_t fileSize = 0;
    uint32_t chunkSizeKB = 64;
    bool allowPartialRender = true;
};

struct StreamingDecodeProgress {
    StreamingDecodeState state = StreamingDecodeState::Idle;
    float progressPercent = 0.0f;
    uint32_t chunksProcessed = 0;
    uint32_t totalChunks = 0;
    bool partialRenderAvailable = false;
    float elapsedMs = 0.0f;
};

struct StreamCoordinatorStats {
    uint64_t totalStreams = 0;
    uint64_t completedStreams = 0;
    uint64_t partialRenders = 0;
    uint64_t failedStreams = 0;
    float averageChunksPerStream = 0.0f;
    bool initialized = false;
};

class StreamingDecodeCoordinator {
public:
    static StreamingDecodeCoordinator& Instance() {
        static StreamingDecodeCoordinator instance;
        return instance;
    }

    void Initialize(uint32_t defaultChunkSizeKB = 64) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_defaultChunkSizeKB = defaultChunkSizeKB;
        m_stats = {};
        m_stats.initialized = true;
        m_progress = {};
    }

    StreamingDecodeProgress StartStream(const StreamingDecodeRequest& request) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalStreams++;

        uint32_t chunkSize = request.chunkSizeKB > 0 ? request.chunkSizeKB : m_defaultChunkSizeKB;
        uint32_t totalChunks = static_cast<uint32_t>(
            (request.fileSize + (chunkSize * 1024ULL - 1)) / (chunkSize * 1024ULL));
        if (totalChunks == 0) totalChunks = 1;

        m_progress.state = StreamingDecodeState::HeaderParsed;
        m_progress.totalChunks = totalChunks;
        m_progress.chunksProcessed = 1;
        m_progress.progressPercent = 100.0f / totalChunks;
        m_progress.partialRenderAvailable = request.allowPartialRender;

        if (request.allowPartialRender) m_stats.partialRenders++;

        return m_progress;
    }

    StreamingDecodeProgress AdvanceChunk() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_progress.state == StreamingDecodeState::Complete ||
            m_progress.state == StreamingDecodeState::Idle) {
            return m_progress;
        }

        m_progress.chunksProcessed++;
        m_progress.progressPercent =
            (static_cast<float>(m_progress.chunksProcessed) /
             static_cast<float>(m_progress.totalChunks)) * 100.0f;

        if (m_progress.chunksProcessed >= m_progress.totalChunks) {
            m_progress.state = StreamingDecodeState::Complete;
            m_progress.progressPercent = 100.0f;
            m_stats.completedStreams++;
            UpdateAverageChunks();
        } else {
            m_progress.state = StreamingDecodeState::PartialDecode;
        }

        return m_progress;
    }

    StreamingDecodeProgress GetProgress() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_progress;
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    StreamCoordinatorStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_progress = {};
    }

private:
    StreamingDecodeCoordinator() = default;
    ~StreamingDecodeCoordinator() = default;
    StreamingDecodeCoordinator(const StreamingDecodeCoordinator&) = delete;
    StreamingDecodeCoordinator& operator=(const StreamingDecodeCoordinator&) = delete;

    void UpdateAverageChunks() {
        float n = static_cast<float>(m_stats.completedStreams);
        m_stats.averageChunksPerStream =
            m_stats.averageChunksPerStream * ((n - 1.0f) / n) +
            static_cast<float>(m_progress.totalChunks) / n;
    }

    mutable std::mutex m_mutex;
    uint32_t m_defaultChunkSizeKB = 64;
    StreamingDecodeProgress m_progress;
    StreamCoordinatorStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
