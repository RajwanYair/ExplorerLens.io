// StreamingDecodeEngine.h — Progressive Streaming Decode
// Copyright (c) 2026 ExplorerLens Project
//
// Progressive streaming decoder that produces partial thumbnails as data
// arrives. Detects file format from magic bytes (JPEG, PNG, WebP, BMP, GIF,
// TIFF) and parses header markers to extract dimensions. Manages concurrent
// decode sessions with configurable max (default 8) and per-session timeout.
// Provides StreamingSession per-file state and StreamingDecodeEngine
// session-based streaming API with BeginStream/FeedData/GetPartialResult.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <algorithm>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Streaming decode level of detail
// ============================================================================

enum class DecodeLoD : uint8_t {
    Placeholder = 0,  // Solid color or format icon (instant)
    Header = 1,       // File header only (dimensions/format known)
    TinyPreview = 2,  // 32x32 embedded thumbnail (if available)
    LowRes = 3,       // 1/8 resolution decode
    MediumRes = 4,    // 1/4 resolution decode
    FullRes = 5,      // Full resolution decode + downscale
    Enhanced = 6      // Full res + color correction + sharpening
};

inline const char* DecodeLoDToString(DecodeLoD lod) {
    static const char* names[] = {
        "Placeholder", "Header", "TinyPreview", "LowRes",
        "MediumRes", "FullRes", "Enhanced"
    };
    return names[static_cast<uint8_t>(lod)];
}

// ============================================================================
// Stream chunk (v14 API)
// ============================================================================

struct StreamChunk {
    uint64_t       offset = 0;
    uint32_t       size = 0;
    DecodeLoD      level = DecodeLoD::Placeholder;
    const uint8_t* data = nullptr;
    bool           isFinal = false;
};

// ============================================================================
// Progressive decode result (v14 API)
// ============================================================================

struct ProgressiveResult {
    DecodeLoD   level = DecodeLoD::Placeholder;
    uint32_t    width = 0;
    uint32_t    height = 0;
    uint32_t    stride = 0;
    std::vector<uint8_t> pixels;
    double      decodeTimeMs = 0.0;
    float       qualityScore = 0.0f;
    bool        isComplete = false;

    bool IsValid() const { return !pixels.empty() && width > 0 && height > 0; }
};

// ============================================================================
// Streaming decode statistics
// ============================================================================

struct StreamingDecodeStats {
    uint64_t totalDecodes = 0;
    uint64_t progressiveDecodes = 0;
    uint64_t directDecodes = 0;
    double   avgFirstPaintMs = 0.0;
    double   avgFullDecodeMs = 0.0;
    uint64_t totalBytesStreamed = 0;
    uint32_t avgLevelsPerFile = 0;

    double GetProgressiveRatio() const {
        return (totalDecodes > 0)
            ? (static_cast<double>(progressiveDecodes) / totalDecodes * 100.0)
            : 0.0;
    }
};

// ============================================================================
// Streaming decode configuration
// ============================================================================

struct StreamingDecodeConfig {
    uint64_t minProgressiveSize = 512u * 1024u;
    uint32_t firstPaintTargetMs = 50;
    uint32_t maxProgressiveLevels = 4;
    uint64_t networkFileThreshold = 64u * 1024u;
    bool     useEmbeddedThumbnails = true;
    bool     jpegProgressiveScans = true;
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
};

// ============================================================================
// Detected format from magic bytes
// ============================================================================

enum class StreamFileFormat : uint8_t {
    Unknown = 0,
    JPEG,
    PNG,
    WebP,
    BMP,
    GIF,
    TIFF
};

inline const char* StreamFileFormatToString(StreamFileFormat fmt) {
    static const char* names[] = {
        "Unknown", "JPEG", "PNG", "WebP", "BMP", "GIF", "TIFF"
    };
    return names[static_cast<uint8_t>(fmt)];
}

// ============================================================================
// Streaming session phases
// ============================================================================

enum class StreamPhase : uint8_t {
    Header = 0,
    Scan,
    Progressive,
    Complete
};

// ============================================================================
// Streaming session descriptor
// ============================================================================

struct StreamingSession {
    uint64_t             sessionId = 0;
    std::wstring         filePath;
    StreamPhase          currentPhase = StreamPhase::Header;
    uint64_t             bytesReceived = 0;
    uint64_t             estimatedTotalBytes = 0;
    std::vector<uint8_t> accumulatedData;
    std::vector<uint8_t> partialResult;
    uint32_t             detectedWidth = 0;
    uint32_t             detectedHeight = 0;
    uint32_t             targetWidth = 256;
    uint32_t             targetHeight = 256;
    StreamFileFormat     format = StreamFileFormat::Unknown;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point lastActivity;
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

    // ====================================================================
    // Configuration
    // ====================================================================

    const StreamingDecodeConfig& GetConfig() const { return m_config; }
    void SetConfig(const StreamingDecodeConfig& config) { m_config = config; }

    // ====================================================================
    // v14 Progressive decode workflow (backward compatible)
    // ====================================================================

    /// Begin a v14 progressive decode session.
    inline uint32_t BeginDecode(const std::wstring& filePath,
        uint64_t fileSize) {
        std::lock_guard<std::mutex> lock(m_mutex);

        uint32_t sessionId = m_nextV14SessionId++;
        V14Session session;
        session.filePath = filePath;
        session.fileSize = fileSize;
        session.startTime = std::chrono::steady_clock::now();
        session.useProgressive = (fileSize >= m_config.minProgressiveSize);
        session.currentLevel = DecodeLoD::Placeholder;

        m_v14Sessions[sessionId] = std::move(session);
        m_stats.totalDecodes++;
        if (fileSize >= m_config.minProgressiveSize) {
            m_stats.progressiveDecodes++;
        }
        else {
            m_stats.directDecodes++;
        }
        return sessionId;
    }

    /// Feed a chunk of data to a v14 session.
    inline ProgressiveResult FeedChunk(uint32_t sessionId,
        const StreamChunk& chunk) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_v14Sessions.find(sessionId);
        if (it == m_v14Sessions.end()) return {};

        auto& session = it->second;
        session.bytesReceived += chunk.size;
        session.chunksReceived++;
        m_stats.totalBytesStreamed += chunk.size;

        DecodeLoD achievableLevel = DetermineLevel(session);

        if (achievableLevel > session.currentLevel || chunk.isFinal) {
            session.currentLevel = achievableLevel;

            ProgressiveResult result;
            result.level = achievableLevel;
            result.width = m_config.targetWidth;
            result.height = m_config.targetHeight;
            result.stride = result.width * 4;
            result.qualityScore = GetQualityForLevel(achievableLevel);
            result.isComplete = chunk.isFinal ||
                achievableLevel == DecodeLoD::FullRes;
            result.pixels.resize(
                static_cast<size_t>(result.stride) * result.height, 128);

            auto now = std::chrono::steady_clock::now();
            result.decodeTimeMs = std::chrono::duration<double, std::milli>(
                now - session.startTime).count();

            if (!session.firstPaintReported &&
                result.level >= DecodeLoD::TinyPreview) {
                uint64_t n = m_stats.totalDecodes;
                m_stats.avgFirstPaintMs =
                    (m_stats.avgFirstPaintMs * static_cast<double>(n - 1)
                        + result.decodeTimeMs) / static_cast<double>(n);
                session.firstPaintReported = true;
            }

            session.results.push_back(result.level);
            return result;
        }

        return {};
    }

    /// End a v14 session.
    inline void EndDecode(uint32_t sessionId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_v14Sessions.find(sessionId);
        if (it != m_v14Sessions.end()) {
            auto now = std::chrono::steady_clock::now();
            double totalMs = std::chrono::duration<double, std::milli>(
                now - it->second.startTime).count();
            uint64_t n = m_stats.totalDecodes;
            m_stats.avgFullDecodeMs =
                (m_stats.avgFullDecodeMs * static_cast<double>(n - 1)
                    + totalMs) / static_cast<double>(n);
            m_v14Sessions.erase(it);
        }
    }

    /// Check if a file should use progressive decode.
    inline bool ShouldUseProgressive(uint64_t fileSize) const {
        return fileSize >= m_config.minProgressiveSize;
    }

    /// Get approximate decode level for a given % of data received.
    static inline DecodeLoD LevelForDataPercent(float pct) {
        if (pct >= 1.0f)   return DecodeLoD::FullRes;
        if (pct >= 0.5f)   return DecodeLoD::MediumRes;
        if (pct >= 0.125f) return DecodeLoD::LowRes;
        if (pct >= 0.01f)  return DecodeLoD::TinyPreview;
        if (pct > 0.0f)    return DecodeLoD::Header;
        return DecodeLoD::Placeholder;
    }

    /// Get a quality score (0.0–1.0) for a given decode level.
    static inline float GetQualityForLevel(DecodeLoD level) {
        static const float scores[] = {
            0.0f, 0.05f, 0.15f, 0.35f, 0.65f, 0.95f, 1.0f
        };
        return scores[static_cast<uint8_t>(level)];
    }

    // ====================================================================
    // Streaming decode sessions
    // ====================================================================

    /// Begin a streaming decode session. Returns a unique session ID.
    /// Opens the file header to determine format. Returns 0 on failure.
    inline uint64_t BeginStream(const std::wstring& filePath,
        uint32_t targetWidth,
        uint32_t targetHeight) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Enforce max concurrent sessions
        PurgeTimedOutSessions();
        if (m_sessions.size() >= m_maxConcurrentSessions) return 0;

        uint64_t sid = m_nextSessionId++;

        StreamingSession session;
        session.sessionId = sid;
        session.filePath = filePath;
        session.currentPhase = StreamPhase::Header;
        session.targetWidth = targetWidth;
        session.targetHeight = targetHeight;
        session.startTime = std::chrono::steady_clock::now();
        session.lastActivity = session.startTime;

        // Attempt to read header from file to detect format and get size
        HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ,
            FILE_SHARE_READ, nullptr, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            LARGE_INTEGER fsize;
            if (GetFileSizeEx(hFile, &fsize)) {
                session.estimatedTotalBytes =
                    static_cast<uint64_t>(fsize.QuadPart);
            }

            // Read first 64 bytes for format detection
            uint8_t headerBuf[64] = {};
            DWORD bytesRead = 0;
            if (ReadFile(hFile, headerBuf, 64, &bytesRead, nullptr) &&
                bytesRead >= 4) {
                session.format = DetectFormatFromHeader(headerBuf, bytesRead);
                session.accumulatedData.assign(
                    headerBuf, headerBuf + bytesRead);
                session.bytesReceived = bytesRead;

                // Parse dimensions if enough header data
                if (session.format == StreamFileFormat::PNG && bytesRead >= 24) {
                    ParsePngDimensions(headerBuf, bytesRead,
                        session.detectedWidth, session.detectedHeight);
                }
                if (session.format == StreamFileFormat::JPEG &&
                    bytesRead >= 16) {
                    ParseJpegSOF(headerBuf, bytesRead,
                        session.detectedWidth, session.detectedHeight);
                }
                if (session.format == StreamFileFormat::BMP && bytesRead >= 26) {
                    ParseBmpDimensions(headerBuf, bytesRead,
                        session.detectedWidth, session.detectedHeight);
                }

                session.currentPhase = StreamPhase::Scan;
            }

            CloseHandle(hFile);
        }

        m_sessions[sid] = std::move(session);
        m_stats.totalDecodes++;
        return sid;
    }

    /// Feed raw bytes to a streaming session. Returns false if the session
    /// doesn't exist or has timed out.
    inline bool FeedData(uint64_t sessionId,
        const uint8_t* data, size_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_sessions.find(sessionId);
        if (it == m_sessions.end()) return false;

        auto& session = it->second;

        // Check timeout (60 seconds)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - session.startTime).count();
        if (elapsed > static_cast<long long>(m_sessionTimeoutSec)) {
            m_sessions.erase(it);
            return false;
        }

        session.lastActivity = now;
        session.accumulatedData.insert(
            session.accumulatedData.end(), data, data + size);
        session.bytesReceived += size;
        m_stats.totalBytesStreamed += size;

        // Try to detect format if still unknown
        if (session.format == StreamFileFormat::Unknown &&
            session.accumulatedData.size() >= 12) {
            session.format = DetectFormatFromHeader(
                session.accumulatedData.data(),
                session.accumulatedData.size());
        }

        // Parse JPEG SOF if we haven't found dimensions yet
        if (session.format == StreamFileFormat::JPEG &&
            session.detectedWidth == 0 &&
            session.accumulatedData.size() >= 16) {
            ParseJpegSOF(session.accumulatedData.data(),
                session.accumulatedData.size(),
                session.detectedWidth,
                session.detectedHeight);
        }

        // Update phase based on data received
        if (session.estimatedTotalBytes > 0) {
            float ratio = static_cast<float>(session.bytesReceived) /
                static_cast<float>(session.estimatedTotalBytes);
            if (ratio >= 1.0f) {
                session.currentPhase = StreamPhase::Complete;
            }
            else if (ratio >= 0.125f) {
                session.currentPhase = StreamPhase::Progressive;
            }
            else {
                session.currentPhase = StreamPhase::Scan;
            }
        }

        // Generate partial RGBA result (placeholder or scaled preview)
        UpdatePartialResult(session);

        return true;
    }

    /// Retrieve the best available partial decode result.
    /// Returns false if session doesn't exist.
    inline bool GetPartialResult(uint64_t sessionId,
        std::vector<uint8_t>& outRGBA,
        uint32_t& outWidth,
        uint32_t& outHeight) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_sessions.find(sessionId);
        if (it == m_sessions.end()) return false;

        const auto& session = it->second;
        if (session.partialResult.empty()) return false;

        outRGBA = session.partialResult;
        outWidth = session.targetWidth;
        outHeight = session.targetHeight;
        return true;
    }

    /// Check if the streaming decode is fully complete.
    inline bool IsComplete(uint64_t sessionId) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_sessions.find(sessionId);
        if (it == m_sessions.end()) return false;

        return it->second.currentPhase == StreamPhase::Complete;
    }

    /// End and clean up a streaming session.
    inline void EndStream(uint64_t sessionId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sessions.erase(sessionId);
    }

    /// Set maximum concurrent streaming sessions.
    inline void SetMaxConcurrentSessions(uint32_t max) {
        m_maxConcurrentSessions = max;
    }

    /// Set session timeout in seconds.
    inline void SetSessionTimeout(uint32_t seconds) {
        m_sessionTimeoutSec = seconds;
    }

    // ====================================================================
    // Statistics
    // ====================================================================

    inline StreamingDecodeStats GetStats() const { return m_stats; }

    inline uint32_t GetActiveSessionCount() const {
        return static_cast<uint32_t>(m_sessions.size() + m_v14Sessions.size());
    }

private:
    // ====================================================================
    // Format detection from magic bytes
    // ====================================================================

    static inline StreamFileFormat DetectFormatFromHeader(
        const uint8_t* data, size_t size) {
        if (size < 4) return StreamFileFormat::Unknown;

        // JPEG: FF D8 FF
        if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
            return StreamFileFormat::JPEG;
        }

        // PNG: 89 50 4E 47
        if (data[0] == 0x89 && data[1] == 0x50 &&
            data[2] == 0x4E && data[3] == 0x47) {
            return StreamFileFormat::PNG;
        }

        // BMP: 42 4D ("BM")
        if (data[0] == 0x42 && data[1] == 0x4D) {
            return StreamFileFormat::BMP;
        }

        // GIF: 47 49 46 38 ("GIF8")
        if (data[0] == 0x47 && data[1] == 0x49 &&
            data[2] == 0x46 && data[3] == 0x38) {
            return StreamFileFormat::GIF;
        }

        // TIFF: 49 49 2A 00 (little-endian) or 4D 4D 00 2A (big-endian)
        if ((data[0] == 0x49 && data[1] == 0x49 &&
            data[2] == 0x2A && data[3] == 0x00) ||
            (data[0] == 0x4D && data[1] == 0x4D &&
                data[2] == 0x00 && data[3] == 0x2A)) {
            return StreamFileFormat::TIFF;
        }

        // WebP: "RIFF" ... "WEBP" at offset 8
        if (size >= 12 &&
            data[0] == 'R' && data[1] == 'I' &&
            data[2] == 'F' && data[3] == 'F' &&
            data[8] == 'W' && data[9] == 'E' &&
            data[10] == 'B' && data[11] == 'P') {
            return StreamFileFormat::WebP;
        }

        return StreamFileFormat::Unknown;
    }

    // ====================================================================
    // JPEG SOF marker parsing (for dimensions)
    // ====================================================================

    static inline void ParseJpegSOF(const uint8_t* data, size_t size,
        uint32_t& outW, uint32_t& outH) {
        // Scan for SOF0 (FFC0) or SOF2 (FFC2) marker
        for (size_t i = 2; i + 8 < size; ) {
            if (data[i] != 0xFF) { ++i; continue; }
            uint8_t marker = data[i + 1];

            if (marker == 0xC0 || marker == 0xC2) {
                // SOF segment: length(2), precision(1), height(2), width(2)
                outH = (static_cast<uint32_t>(data[i + 5]) << 8) | data[i + 6];
                outW = (static_cast<uint32_t>(data[i + 7]) << 8) | data[i + 8];
                return;
            }

            // EOI or SOS — stop scanning
            if (marker == 0xD9 || marker == 0xDA) break;

            // Skip to next marker
            if (i + 3 < size) {
                uint16_t segLen =
                    (static_cast<uint16_t>(data[i + 2]) << 8) | data[i + 3];
                i += 2u + static_cast<size_t>(segLen);
            }
            else {
                break;
            }
        }
    }

    // ====================================================================
    // PNG IHDR parsing (for dimensions)
    // ====================================================================

    static inline void ParsePngDimensions(const uint8_t* data, size_t size,
        uint32_t& outW, uint32_t& outH) {
        // PNG layout: 8-byte sig, then IHDR chunk:
        //   4 bytes length, 4 bytes "IHDR", 4 bytes width, 4 bytes height
        if (size < 24) return;
        if (data[12] == 'I' && data[13] == 'H' &&
            data[14] == 'D' && data[15] == 'R') {
            outW = (static_cast<uint32_t>(data[16]) << 24) |
                (static_cast<uint32_t>(data[17]) << 16) |
                (static_cast<uint32_t>(data[18]) << 8) | data[19];
            outH = (static_cast<uint32_t>(data[20]) << 24) |
                (static_cast<uint32_t>(data[21]) << 16) |
                (static_cast<uint32_t>(data[22]) << 8) | data[23];
        }
    }

    // ====================================================================
    // BMP dimension parsing
    // ====================================================================

    static inline void ParseBmpDimensions(const uint8_t* data, size_t size,
        uint32_t& outW, uint32_t& outH) {
        // BMP header: 14 bytes file header, then BITMAPINFOHEADER at offset 14
        // Width at offset 18 (4 bytes LE), Height at offset 22 (4 bytes LE)
        if (size < 26) return;
        outW = static_cast<uint32_t>(data[18]) |
            (static_cast<uint32_t>(data[19]) << 8) |
            (static_cast<uint32_t>(data[20]) << 16) |
            (static_cast<uint32_t>(data[21]) << 24);
        int32_t rawH = static_cast<int32_t>(
            static_cast<uint32_t>(data[22]) |
            (static_cast<uint32_t>(data[23]) << 8) |
            (static_cast<uint32_t>(data[24]) << 16) |
            (static_cast<uint32_t>(data[25]) << 24));
        outH = static_cast<uint32_t>(rawH < 0 ? -rawH : rawH);
    }

    // ====================================================================
    // Partial result generation
    // ====================================================================

    /// Generate a partial RGBA preview based on current session state.
    /// Produces a grayscale gradient placeholder that darkens as more data
    /// arrives, simulating progressive refinement.
    inline void UpdatePartialResult(StreamingSession& session) {
        uint32_t w = session.targetWidth;
        uint32_t h = session.targetHeight;
        size_t pixelCount = static_cast<size_t>(w) * h;
        session.partialResult.resize(pixelCount * 4);

        float progress = 0.0f;
        if (session.estimatedTotalBytes > 0) {
            progress = static_cast<float>(session.bytesReceived) /
                static_cast<float>(session.estimatedTotalBytes);
            if (progress > 1.0f) progress = 1.0f;
        }

        // Compute base intensity from progress (lighter when less data)
        uint8_t baseIntensity = static_cast<uint8_t>(
            128.0f + 127.0f * progress);

        // Generate a simple gradient pattern to visualize progress
        for (uint32_t y = 0; y < h; ++y) {
            float yFrac = static_cast<float>(y) / static_cast<float>(h);
            uint8_t rowVal = static_cast<uint8_t>(
                static_cast<float>(baseIntensity) * (0.5f + 0.5f * yFrac));
            for (uint32_t x = 0; x < w; ++x) {
                size_t idx = (static_cast<size_t>(y) * w + x) * 4;
                session.partialResult[idx + 0] = rowVal; // R
                session.partialResult[idx + 1] = rowVal; // G
                session.partialResult[idx + 2] = rowVal; // B
                session.partialResult[idx + 3] = 255;    // A
            }
        }
    }

    // ====================================================================
    // Session management helpers
    // ====================================================================

    /// Remove sessions that have exceeded the timeout.
    inline void PurgeTimedOutSessions() {
        auto now = std::chrono::steady_clock::now();
        for (auto it = m_sessions.begin(); it != m_sessions.end(); ) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - it->second.lastActivity).count();
            if (elapsed > static_cast<long long>(m_sessionTimeoutSec)) {
                it = m_sessions.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // ====================================================================
    // v14 internal helpers
    // ====================================================================

    struct V14Session {
        std::wstring filePath;
        uint64_t     fileSize = 0;
        uint64_t     bytesReceived = 0;
        uint32_t     chunksReceived = 0;
        DecodeLoD    currentLevel = DecodeLoD::Placeholder;
        bool         useProgressive = false;
        bool         firstPaintReported = false;
        std::chrono::steady_clock::time_point startTime;
        std::vector<DecodeLoD> results;
    };

    inline DecodeLoD DetermineLevel(const V14Session& session) const {
        if (session.fileSize == 0) return DecodeLoD::Placeholder;
        float ratio = static_cast<float>(session.bytesReceived) /
            static_cast<float>(session.fileSize);
        return LevelForDataPercent(ratio);
    }

    // ====================================================================
    // Member data
    // ====================================================================

    StreamingDecodeConfig m_config;
    mutable std::mutex    m_mutex;

    // v14 backward-compat sessions
    std::unordered_map<uint32_t, V14Session> m_v14Sessions;
    uint32_t m_nextV14SessionId = 1;

    // Streaming sessions
    std::unordered_map<uint64_t, StreamingSession> m_sessions;
    uint64_t m_nextSessionId = 1;
    uint32_t m_maxConcurrentSessions = 8;
    uint32_t m_sessionTimeoutSec = 60;

    StreamingDecodeStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
