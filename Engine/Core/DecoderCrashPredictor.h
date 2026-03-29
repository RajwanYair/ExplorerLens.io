// DecoderCrashPredictor.h — ML-Based Decoder Crash Predictor
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts decoder crash probability from recent error/latency history using a
// lightweight exponential-smoothing model, enabling proactive quarantine decisions.
//
#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <chrono>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class CrashRiskLevel { None = 0, Low, Medium, High, Critical };

struct DecoderHealthSample {
    double latencyMs         = 0.0;
    bool   hadError          = false;
    bool   hadCrash          = false;
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
};

struct CrashPrediction {
    std::string  decoderName;
    double       riskScore   = 0.0; // 0.0–1.0
    CrashRiskLevel level     = CrashRiskLevel::None;
    int          samplesUsed = 0;
    bool         shouldQuarantine() const noexcept { return level >= CrashRiskLevel::High; }
};

class DecoderCrashPredictor {
public:
    static constexpr double ALPHA              = 0.3;   // EMA smoothing factor
    static constexpr double HIGH_RISK_THRESH   = 0.65;
    static constexpr double CRITICAL_THRESH    = 0.85;
    static constexpr int    MIN_SAMPLES        = 5;
    static constexpr int    MAX_HISTORY        = 50;

    explicit DecoderCrashPredictor() = default;

    void Record(const std::string& decoderName, double latencyMs, bool error, bool crash) {
        auto& hist = m_history[decoderName];
        if ((int)hist.size() >= MAX_HISTORY) hist.pop_front();
        hist.push_back({ latencyMs, error, crash });
    }

    CrashPrediction Predict(const std::string& decoderName) const {
        CrashPrediction p;
        p.decoderName = decoderName;
        auto it = m_history.find(decoderName);
        if (it == m_history.end() || (int)it->second.size() < MIN_SAMPLES) return p;

        const auto& hist = it->second;
        p.samplesUsed = (int)hist.size();

        double ema = 0.0;
        for (const auto& s : hist) {
            double contrib = (s.hadCrash ? 1.0 : 0.0) * 0.6 + (s.hadError ? 1.0 : 0.0) * 0.3
                           + (s.latencyMs > 5000.0 ? 0.1 : 0.0);
            ema = ALPHA * contrib + (1.0 - ALPHA) * ema;
        }
        p.riskScore = std::min(1.0, ema);

        if      (p.riskScore >= CRITICAL_THRESH) p.level = CrashRiskLevel::Critical;
        else if (p.riskScore >= HIGH_RISK_THRESH) p.level = CrashRiskLevel::High;
        else if (p.riskScore >= 0.35)             p.level = CrashRiskLevel::Medium;
        else if (p.riskScore >= 0.15)             p.level = CrashRiskLevel::Low;
        return p;
    }

    void Reset(const std::string& decoderName) { m_history.erase(decoderName); }
    void ResetAll() noexcept { m_history.clear(); }
    int  DecoderCount() const noexcept { return (int)m_history.size(); }

private:
    std::unordered_map<std::string, std::deque<DecoderHealthSample>> m_history;
};

} // namespace Engine
} // namespace ExplorerLens
