// FormatStatusProvider.h — Per-format decode status tracking
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks success, failure, and fallback rates for each format decoder.
// Feeds the 'lens doctor' diagnostics command and ETW telemetry.
// Thread-safe singleton suitable for use from the COM shell extension.
//
#pragma once
#include <atomic>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DecoderHealth {
    HEALTHY,        // <1% failure rate
    DEGRADED,       // 1-10% failure rate or frequent fallback
    FAILING,        // >10% failure rate
    UNAVAILABLE,    // decoder not loaded / library missing
    UNKNOWN,        // no data collected yet
};

struct FormatDecoderStats {
    std::string      formatId;
    std::atomic<uint64_t> decodeAttempts{0};
    std::atomic<uint64_t> decodeSuccesses{0};
    std::atomic<uint64_t> decodeFailures{0};
    std::atomic<uint64_t> fallbackUsed{0};     // Used embedded preview or CPU fallback
    std::atomic<uint64_t> totalDecodeUs{0};    // Cumulative microseconds

    // Non-atomic summary (protected by registry mutex for reads across fields)
    std::string      lastError;
    uint64_t         lastFailureTimestampMs = 0;

    double FailureRate() const noexcept {
        uint64_t total = decodeAttempts.load();
        return total > 0 ? static_cast<double>(decodeFailures.load()) / total : 0.0;
    }

    double AverageDecodeMs() const noexcept {
        uint64_t n = decodeSuccesses.load();
        return n > 0 ? static_cast<double>(totalDecodeUs.load()) / n / 1000.0 : 0.0;
    }

    DecoderHealth Health() const noexcept {
        if (decodeAttempts.load() == 0) return DecoderHealth::UNKNOWN;
        double fr = FailureRate();
        if (fr > 0.10) return DecoderHealth::FAILING;
        if (fr > 0.01 || fallbackUsed.load() * 3 > decodeAttempts.load())
            return DecoderHealth::DEGRADED;
        return DecoderHealth::HEALTHY;
    }
};

struct FormatStatusSnapshot {
    std::string   formatId;
    DecoderHealth health;
    uint64_t      attempts;
    uint64_t      successes;
    uint64_t      failures;
    uint64_t      fallbacks;
    double        failureRate;
    double        avgDecodeMs;
    std::string   lastError;
};

class FormatStatusProvider {
public:
    // Singleton access
    static FormatStatusProvider& Instance();

    // Record decode attempt results
    void RecordSuccess(const std::string& formatId, uint64_t decodeUs);
    void RecordFailure(const std::string& formatId, const std::string& errorMsg);
    void RecordFallback(const std::string& formatId);

    // Mark a decoder as unavailable (library not loaded at startup)
    void MarkUnavailable(const std::string& formatId);

    // Get current health for a format
    DecoderHealth GetHealth(const std::string& formatId) const;

    // Get a snapshot of all tracked formats
    std::vector<FormatStatusSnapshot> GetAllSnapshots() const;

    // Get snapshot for a single format
    FormatStatusSnapshot GetSnapshot(const std::string& formatId) const;

    // Reset counters for all formats (e.g. after a deliberate decoder reload)
    void Reset();

    // Print a doctor-style report to stdout
    void PrintReport() const;

    // Returns true if any decoder is FAILING
    bool HasFailingDecoders() const;

private:
    FormatStatusProvider() = default;
    FormatStatusProvider(const FormatStatusProvider&) = delete;
    FormatStatusProvider& operator=(const FormatStatusProvider&) = delete;

    mutable std::shared_mutex                              m_mutex;
    std::unordered_map<std::string, FormatDecoderStats*>   m_formats;

    FormatDecoderStats& GetOrCreate(const std::string& formatId);
};

} // namespace Engine
} // namespace ExplorerLens
