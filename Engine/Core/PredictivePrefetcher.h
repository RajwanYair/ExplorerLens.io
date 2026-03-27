// PredictivePrefetcher.h — ML-Based Scroll-Direction Thumbnail Prefetch
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts which files will be viewed next based on scroll direction, access
// history, and a lightweight EWMA velocity model — issuing low-priority prefetch
// requests via ThumbnailPriorityQueue before the shell requests them.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <cmath>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class PredictorScrollDirection { Unknown, Forward, Backward, Random };

struct PrefetchCandidate {
    std::wstring filePath;
    float        confidence; // 0.0 – 1.0
    int          lookaheadSlots; // distance from current viewport
};

struct ViewportState {
    std::vector<std::wstring> visibleFiles;  // Files currently in view, top to bottom
    PredictorScrollDirection           direction = PredictorScrollDirection::Unknown;
    int                       viewportSize  = 10; // Visible row count estimate
};

// Lightweight exponential weighted moving average velocity tracker
struct EWMAVelocity {
    static constexpr float ALPHA = 0.3f;

    float   velocity = 0.0f;    // Files/sec (positive = forward, negative = backward)
    DWORD   lastTick = 0;

    void Update(int deltaFiles) {
        DWORD now = GetTickCount();
        float dt = (now - lastTick) / 1000.0f;
        if (dt < 0.01f) { lastTick = now; return; }
        float inst = (dt > 0) ? deltaFiles / dt : 0.0f;
        velocity = ALPHA * inst + (1.0f - ALPHA) * velocity;
        lastTick = now;
    }

    int LookaheadFrames(float windowSec = 0.5f) const {
        return static_cast<int>(std::abs(velocity) * windowSec + 0.5f);
    }
};

class PredictivePrefetcher {
public:
    static constexpr int MAX_LOOKAHEAD   = 30;  // files ahead
    static constexpr int MAX_LOOKBEHIND  = 5;   // files behind (for back-scroll)
    static constexpr float MIN_CONFIDENCE = 0.25f;

    // Full file list in the folder (sorted as Explorer shows them)
    void SetFileList(std::vector<std::wstring> files) {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_files = std::move(files);
    }

    // Notify prefetcher of new viewport (call on every Explorer scroll event)
    std::vector<PrefetchCandidate> OnViewport(const ViewportState& vs) {
        std::lock_guard<std::mutex> lk(m_mtx);

        // Find the first visible file index
        int firstIdx = FindIndex(vs.visibleFiles.empty() ? L"" : vs.visibleFiles.front());
        if (firstIdx < 0) return {};

        // Compute delta from previous viewport
        int delta = firstIdx - m_prevFirstIdx;
        m_prevFirstIdx = firstIdx;

        // Update scroll direction
        PredictorScrollDirection dir = vs.direction;
        if (dir == PredictorScrollDirection::Unknown) {
            if      (delta > 0) dir = PredictorScrollDirection::Forward;
            else if (delta < 0) dir = PredictorScrollDirection::Backward;
            else                dir = PredictorScrollDirection::Random;
        }
        m_velocity.Update(delta);

        // Compute lookahead count from velocity
        int lookahead = std::min(MAX_LOOKAHEAD, std::max(5, m_velocity.LookaheadFrames()));

        // Record access pattern
        for (const auto& f : vs.visibleFiles)
            m_accessFreq[f]++;

        // Build candidates
        std::vector<PrefetchCandidate> candidates;
        int base    = firstIdx + static_cast<int>(vs.visibleFiles.size()); // past viewport
        int step    = (dir == PredictorScrollDirection::Backward) ? -1 : 1;
        if (dir == PredictorScrollDirection::Backward) base = firstIdx - 1;

        for (int i = 0; i < lookahead; ++i) {
            int idx = base + step * i;
            if (idx < 0 || idx >= static_cast<int>(m_files.size())) break;
            float conf = 1.0f - (float)i / lookahead;
            if (conf < MIN_CONFIDENCE) break;
            candidates.push_back({ m_files[idx], conf, i+1 });
        }

        // Frequency boost: also prefetch frequently-accessed files not in viewport
        for (const auto& [f, freq] : m_accessFreq) {
            if (freq > 3) {
                bool already = false;
                for (const auto& c : candidates)
                    if (c.filePath == f) { already = true; break; }
                if (!already)
                    candidates.push_back({ f, 0.4f, 0 });
            }
        }

        // Record for stats
        m_prefetchCount += static_cast<int>(candidates.size());
        return candidates;
    }

    // Mark a prefetch as a hit (shell requested within prefetch window)
    void RecordHit(const std::wstring& path) {
        (void)path;
        m_hitCount++;
    }

    float HitRate() const {
        return (m_prefetchCount > 0)
            ? static_cast<float>(m_hitCount) / m_prefetchCount
            : 0.0f;
    }

    int PrefetchCount() const { return m_prefetchCount; }
    int HitCount()      const { return m_hitCount; }

    void Reset() {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_accessFreq.clear();
        m_prevFirstIdx = 0;
        m_prefetchCount = 0;
        m_hitCount = 0;
        m_velocity = {};
    }

private:
    int FindIndex(const std::wstring& path) const {
        for (int i = 0; i < static_cast<int>(m_files.size()); ++i)
            if (m_files[i] == path) return i;
        return -1;
    }

    std::mutex m_mtx;
    std::vector<std::wstring>               m_files;
    std::unordered_map<std::wstring, int>   m_accessFreq;
    EWMAVelocity                            m_velocity;
    int   m_prevFirstIdx  = 0;
    int   m_prefetchCount = 0;
    int   m_hitCount      = 0;
};

}} // namespace ExplorerLens::Engine
