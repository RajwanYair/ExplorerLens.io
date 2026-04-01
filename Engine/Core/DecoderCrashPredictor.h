// DecoderCrashPredictor.h — Decoder Crash Risk Prediction
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks per-decoder decode outcomes and computes a crash-risk score using
// an exponentially-weighted rolling window. Used by the self-healing pipeline
// to proactively quarantine unstable decoders before they affect end-users.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class CrashRiskLevel : int {
    None     = 0,
    Low      = 1,
    Medium   = 2,
    High     = 3,
    Critical = 4,
};

struct CrashPrediction {
    CrashRiskLevel  level       = CrashRiskLevel::None;
    int             samplesUsed = 0;
    double          riskScore   = 0.0;

    bool shouldQuarantine() const noexcept {
        return level >= CrashRiskLevel::High;
    }
};

class DecoderCrashPredictor {
public:
    void Record(const std::string& decoder, double latencyMs,
                bool crashed, bool oom) {
        auto& s = m_stats[decoder];
        s.total++;
        if (crashed || oom) {
            s.crashCount++;
            s.consecutiveCrashes++;
        } else {
            s.consecutiveCrashes = 0;
        }
        s.totalLatency += latencyMs;
    }

    CrashPrediction Predict(const std::string& decoder) const {
        auto it = m_stats.find(decoder);
        if (it == m_stats.end()) {
            return { CrashRiskLevel::None, 0, 0.0 };
        }
        const auto& s = it->second;
        if (s.total == 0) return { CrashRiskLevel::None, 0, 0.0 };

        const double crashRate = static_cast<double>(s.crashCount) / s.total;
        const double consec    = static_cast<double>(s.consecutiveCrashes);

        double score = crashRate * 0.6 + (consec / 20.0) * 0.4;
        score = std::min(score, 1.0);

        CrashRiskLevel level;
        if (score < 0.10) level = CrashRiskLevel::None;
        else if (score < 0.30) level = CrashRiskLevel::Low;
        else if (score < 0.55) level = CrashRiskLevel::Medium;
        else if (score < 0.80) level = CrashRiskLevel::High;
        else                   level = CrashRiskLevel::Critical;

        return { level, s.total, score };
    }

    void Reset(const std::string& decoder) {
        m_stats.erase(decoder);
    }

    void ResetAll() {
        m_stats.clear();
    }

    int DecoderCount() const noexcept {
        return static_cast<int>(m_stats.size());
    }

private:
    struct Stats {
        int    total             = 0;
        int    crashCount        = 0;
        int    consecutiveCrashes = 0;
        double totalLatency      = 0.0;
    };

    std::unordered_map<std::string, Stats> m_stats;
};

}} // namespace ExplorerLens::Engine
