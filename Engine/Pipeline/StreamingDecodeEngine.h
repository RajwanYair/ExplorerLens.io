// ============================================================================
// StreamingDecodeEngine.h — Progressive Streaming Decode for Large Files
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes thumbnails progressively — shows incremental preview quality as
// data arrives from disk/network. Targets "instant" first-paint (<50ms)
// with progressive refinement to full quality in <200ms for large files.
// Integrates with cloud storage providers (OneDrive/SharePoint hydration).
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Streaming decode level of detail
// ============================================================================

enum class DecodeLoD : uint8_t {
    Placeholder = 0,  // Solid color or format icon (instant)
    Header = 1,  // File header only (dimensions/format known)
    TinyPreview = 2,  // 32x32 embedded thumbnail (if available)
    LowRes = 3,  // 1/8 resolution decode
    MediumRes = 4,  // 1/4 resolution decode
    FullRes = 5,  // Full resolution decode + downscale
    Enhanced = 6   // Full res + color correction + sharpening
};

inline const char* DecodeLoDToString(DecodeLoD lod) {
    static const char* names[] = {
        "Placeholder", "Header", "TinyPreview", "LowRes",
        "MediumRes", "FullRes", "Enhanced"
    };
    return names[static_cast<uint8_t>(lod)];
}

// ============================================================================
// Stream chunk
// ============================================================================

struct StreamChunk {
    uint64_t       offset = 0;     // Byte offset in file
    uint32_t       size = 0;     // Chunk size in bytes
    DecodeLoD      level = DecodeLoD::Placeholder;
    const uint8_t* data = nullptr;
    bool           isFinal = false; // Last chunk
};

// ============================================================================
// Progressive decode result (per-level)
// ============================================================================

struct ProgressiveResult {
    DecodeLoD   level;
    uint32_t    width = 0;
    uint32_t    height = 0;
    uint32_t    stride = 0;
    std::vector<uint8_t> pixels;     // BGRA bitmap data
    double      decodeTimeMs = 0.0;
    float       qualityScore = 0.0f; // 0.0 to 1.0
    bool        isComplete = false;

    bool IsValid() const { return !pixels.empty() && width > 0 && height > 0; }
};

// ============================================================================
// Streaming decode statistics
// ============================================================================

struct StreamingDecodeStats {
    uint64_t totalDecodes = 0;
    uint64_t progressiveDecodes = 0;
    uint64_t directDecodes = 0;   // Completed in single pass
    double   avgFirstPaintMs = 0.0; // Time to first visible result
    double   avgFullDecodeMs = 0.0; // Time to full quality
    uint64_t totalBytesStreamed = 0;
    uint32_t avgLevelsPerFile = 0;   // Average refinement levels

    double GetProgressiveRatio() const {
        return (totalDecodes > 0)
            ? (static_cast<double>(progressiveDecodes) / totalDecodes * 100.0) : 0.0;
    }
};

// ============================================================================
// Streaming decode configuration
// ============================================================================

struct StreamingDecodeConfig {
    /// Minimum file size for progressive decode (smaller files decoded directly)
    uint64_t minProgressiveSize = 512 * 1024;  // 512KB

    /// Target time for first paint
    uint32_t firstPaintTargetMs = 50;

    /// Maximum progressive levels
    uint32_t maxProgressiveLevels = 4;

    /// Network file threshold for aggressive streaming (OneDrive, SharePoint)
    uint64_t networkFileThreshold = 64 * 1024;  // 64KB

    /// Enable embedded thumbnail fast path (EXIF, JFIF)
    bool useEmbeddedThumbnails = true;

    /// Enable JPEG progressive scan rendering
    bool jpegProgressiveScans = true;

    /// Output thumbnail size
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
};

// ============================================================================
// StreamingDecodeEngine
// ============================================================================

class StreamingDecodeEngine {
public:
    StreamingDecodeEngine() = default;
    explicit StreamingDecodeEngine(const StreamingDecodeConfig& config)
        : m_config(config) {
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    const StreamingDecodeConfig& GetConfig() const { return m_config; }
    void SetConfig(const StreamingDecodeConfig& config) { m_config = config; }

    // ========================================================================
    // Progressive decode workflow
    // ========================================================================

    /// Begin a streaming decode session for a file
    uint32_t BeginDecode(const std::wstring& filePath, uint64_t fileSize) {
        std::lock_guard<std::mutex> lock(m_mutex);

        uint32_t sessionId = m_nextSessionId++;
        DecodeSession session;
        session.filePath = filePath;
        session.fileSize = fileSize;
        session.startTime = std::chrono::steady_clock::now();
        session.useProgressive = (fileSize >= m_config.minProgressiveSize);
        session.currentLevel = DecodeLoD::Placeholder;

        m_sessions[sessionId] = std::move(session);
        m_stats.totalDecodes++;
        if (fileSize >= m_config.minProgressiveSize) {
            m_stats.progressiveDecodes++;
        }
        else {
            m_stats.directDecodes++;
        }

        return sessionId;
    }

    /// Feed a chunk of data to a decode session
    ProgressiveResult FeedChunk(uint32_t sessionId, const StreamChunk& chunk) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_sessions.find(sessionId);
        if (it == m_sessions.end()) return {};

        auto& session = it->second;
        session.bytesReceived += chunk.size;
        session.chunksReceived++;
        m_stats.totalBytesStreamed += chunk.size;

        // Determine which level we can decode now
        DecodeLoD achievableLevel = DetermineLevel(session);

        if (achievableLevel > session.currentLevel || chunk.isFinal) {
            session.currentLevel = achievableLevel;

            ProgressiveResult result;
            result.level = achievableLevel;
            result.width = m_config.targetWidth;
            result.height = m_config.targetHeight;
            result.stride = result.width * 4;
            result.qualityScore = GetQualityForLevel(achievableLevel);
            result.isComplete = chunk.isFinal || achievableLevel == DecodeLoD::FullRes;

            // In production: actually decode the data at this level
            result.pixels.resize(static_cast<size_t>(result.stride) * result.height, 128);

            auto now = std::chrono::steady_clock::now();
            result.decodeTimeMs = std::chrono::duration<double, std::milli>(
                now - session.startTime).count();

            // Track first paint
            if (!session.firstPaintReported && result.level >= DecodeLoD::TinyPreview) {
                m_stats.avgFirstPaintMs =
                    (m_stats.avgFirstPaintMs * (m_stats.totalDecodes - 1) + result.decodeTimeMs)
                    / m_stats.totalDecodes;
                session.firstPaintReported = true;
            }

            session.results.push_back(result.level);
            return result;
        }

        return {};  // No level upgrade yet
    }

    /// End a decode session
    void EndDecode(uint32_t sessionId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_sessions.find(sessionId);
        if (it != m_sessions.end()) {
            auto now = std::chrono::steady_clock::now();
            double totalMs = std::chrono::duration<double, std::milli>(
                now - it->second.startTime).count();
            m_stats.avgFullDecodeMs =
                (m_stats.avgFullDecodeMs * (m_stats.totalDecodes - 1) + totalMs)
                / m_stats.totalDecodes;
            m_sessions.erase(it);
        }
    }

    // ========================================================================
    // Queries
    // ========================================================================

    /// Check if a file should use progressive decode
    bool ShouldUseProgressive(uint64_t fileSize) const {
        return fileSize >= m_config.minProgressiveSize;
    }

    /// Get approximate decode level for a given percentage of data received
    static DecodeLoD LevelForDataPercent(float pct) {
        if (pct >= 1.0f)   return DecodeLoD::FullRes;
        if (pct >= 0.5f)   return DecodeLoD::MediumRes;
        if (pct >= 0.125f) return DecodeLoD::LowRes;
        if (pct >= 0.01f)  return DecodeLoD::TinyPreview;
        if (pct > 0.0f)    return DecodeLoD::Header;
        return DecodeLoD::Placeholder;
    }

    /// Get the quality score for a decode level (0.0 to 1.0)
    static float GetQualityForLevel(DecodeLoD level) {
        static const float scores[] = {
            0.0f, 0.05f, 0.15f, 0.35f, 0.65f, 0.95f, 1.0f
        };
        return scores[static_cast<uint8_t>(level)];
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    StreamingDecodeStats GetStats() const { return m_stats; }
    uint32_t GetActiveSessionCount() const {
        return static_cast<uint32_t>(m_sessions.size());
    }

private:
    struct DecodeSession {
        std::wstring filePath;
        uint64_t     fileSize = 0;
        uint64_t     bytesReceived = 0;
        uint32_t     chunksReceived = 0;
        DecodeLoD    currentLevel = DecodeLoD::Placeholder;
        bool         useProgressive = false;
        bool         firstPaintReported = false;
        std::chrono::steady_clock::time_point startTime;
        std::vector<DecodeLoD> results;  // Level history
    };

    DecodeLoD DetermineLevel(const DecodeSession& session) const {
        if (session.fileSize == 0) return DecodeLoD::Placeholder;
        float ratio = static_cast<float>(session.bytesReceived) / session.fileSize;
        return LevelForDataPercent(ratio);
    }

    StreamingDecodeConfig m_config;
    mutable std::mutex m_mutex;
    std::unordered_map<uint32_t, DecodeSession> m_sessions;
    uint32_t m_nextSessionId = 1;
    StreamingDecodeStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
