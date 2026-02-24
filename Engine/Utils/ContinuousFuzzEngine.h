#pragma once
// Continuous Fuzzing Engine
// Automated fuzz test framework with crash budget gating, corpus management,
// and per-decoder mutation strategies for malformed payload hardening.

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <functional>
#include <random>
#include <unordered_map>

namespace ExplorerLens::Utils {

// ─── Mutation strategies ─────────────────────────────────────────
enum class MutationStrategy : uint8_t {
    BitFlip       = 0,   // Random bit flips
    ByteFlip      = 1,   // Random byte overwrites
    Truncation    = 2,   // Truncate file at random position
    HeaderCorrupt = 3,   // Corrupt magic bytes / header fields
    Insertion     = 4,   // Insert random bytes
    Deletion      = 5,   // Delete random byte ranges
    Boundary      = 6,   // Replace values with boundary values (0, 0xFF, MAX_INT)
    Dictionary    = 7,   // Use format-specific dictionary tokens
    Havoc         = 8,   // Combined random mutations
    COUNT         = 9
};

inline const char* MutationName(MutationStrategy s) {
    switch (s) {
        case MutationStrategy::BitFlip:       return "BitFlip";
        case MutationStrategy::ByteFlip:      return "ByteFlip";
        case MutationStrategy::Truncation:    return "Truncation";
        case MutationStrategy::HeaderCorrupt: return "HeaderCorrupt";
        case MutationStrategy::Insertion:     return "Insertion";
        case MutationStrategy::Deletion:      return "Deletion";
        case MutationStrategy::Boundary:      return "Boundary";
        case MutationStrategy::Dictionary:    return "Dictionary";
        case MutationStrategy::Havoc:         return "Havoc";
        default: return "Unknown";
    }
}

// ─── Fuzz target result ──────────────────────────────────────────
enum class FuzzResult : uint8_t {
    NoError       = 0,   // Decoder handled gracefully
    ErrorHandled  = 1,   // Error returned but no crash
    Timeout       = 2,   // Exceeded time limit
    Crash         = 3,   // Unhandled exception or access violation
    Hang          = 4,   // Deadlock or infinite loop
    MemoryLeak    = 5,   // Significant memory growth
    Assertion     = 6    // Debug assertion failure
};

inline const char* FuzzResultName(FuzzResult r) {
    switch (r) {
        case FuzzResult::NoError:      return "NoError";
        case FuzzResult::ErrorHandled: return "ErrorHandled";
        case FuzzResult::Timeout:      return "Timeout";
        case FuzzResult::Crash:        return "Crash";
        case FuzzResult::Hang:         return "Hang";
        case FuzzResult::MemoryLeak:   return "MemoryLeak";
        case FuzzResult::Assertion:    return "Assertion";
        default: return "Unknown";
    }
}

inline bool IsFailure(FuzzResult r) {
    return r == FuzzResult::Crash || r == FuzzResult::Hang || r == FuzzResult::Assertion;
}

// ─── Corpus entry ────────────────────────────────────────────────
struct CorpusEntry {
    std::string          filePath;
    std::string          format;        // "zip", "rar", "webp", etc.
    size_t               fileSize = 0;
    uint32_t             mutationCount = 0;
    uint32_t             crashesFound = 0;
    bool                 isMinimized = false;

    bool IsInteresting() const { return crashesFound > 0; }
};

// ─── Fuzz execution record ───────────────────────────────────────
struct FuzzExecution {
    uint64_t         id = 0;
    std::string      inputFile;
    std::string      decoderName;
    MutationStrategy strategy = MutationStrategy::BitFlip;
    FuzzResult       result = FuzzResult::NoError;
    double           durationMs = 0.0;
    size_t           memoryPeakBytes = 0;
    std::string      errorMessage;

    bool IsFatal() const { return IsFailure(result); }
};

// ─── Crash budget ────────────────────────────────────────────────
struct CrashBudget {
    uint32_t maxCrashes = 0;       // 0 = zero tolerance
    uint32_t maxTimeouts = 5;
    uint32_t maxMemoryLeaks = 3;
    uint32_t currentCrashes = 0;
    uint32_t currentTimeouts = 0;
    uint32_t currentMemoryLeaks = 0;

    bool IsExhausted() const {
        return currentCrashes > maxCrashes ||
               currentTimeouts > maxTimeouts ||
               currentMemoryLeaks > maxMemoryLeaks;
    }

    bool IsClean() const {
        return currentCrashes == 0 && currentTimeouts == 0 && currentMemoryLeaks == 0;
    }

    void Record(FuzzResult r) {
        switch (r) {
            case FuzzResult::Crash:
            case FuzzResult::Assertion:
                currentCrashes++; break;
            case FuzzResult::Timeout:
            case FuzzResult::Hang:
                currentTimeouts++; break;
            case FuzzResult::MemoryLeak:
                currentMemoryLeaks++; break;
            default: break;
        }
    }

    static CrashBudget ZeroTolerance() { return {}; }
    static CrashBudget Lenient() {
        CrashBudget b;
        b.maxCrashes = 3;
        b.maxTimeouts = 10;
        b.maxMemoryLeaks = 5;
        return b;
    }
};

// ─── Fuzz campaign config ────────────────────────────────────────
struct FuzzConfig {
    uint64_t maxIterations = 10000;
    uint32_t timeoutMs = 5000;         // per-input timeout
    size_t   memoryLimitBytes = 256 * 1024 * 1024;
    bool     minimizeCrashes = true;
    bool     collectCoverage = false;
    CrashBudget budget;
    std::vector<MutationStrategy> strategies;

    static FuzzConfig Quick() {
        FuzzConfig c;
        c.maxIterations = 1000;
        c.timeoutMs = 2000;
        c.strategies = {MutationStrategy::BitFlip, MutationStrategy::HeaderCorrupt,
                        MutationStrategy::Truncation};
        return c;
    }

    static FuzzConfig Full() {
        FuzzConfig c;
        c.maxIterations = 100000;
        c.timeoutMs = 10000;
        c.collectCoverage = true;
        c.strategies.resize(static_cast<size_t>(MutationStrategy::COUNT));
        for (size_t i = 0; i < c.strategies.size(); i++)
            c.strategies[i] = static_cast<MutationStrategy>(i);
        return c;
    }
};

// ─── Fuzz campaign statistics ────────────────────────────────────
struct FuzzStats {
    uint64_t totalIterations = 0;
    uint64_t totalCrashes = 0;
    uint64_t totalTimeouts = 0;
    uint64_t totalNoErrors = 0;
    uint64_t totalErrorsHandled = 0;
    uint64_t uniqueCrashes = 0;
    double   totalDurationMs = 0.0;
    size_t   corpusSize = 0;

    double CrashRate() const {
        return totalIterations > 0
            ? static_cast<double>(totalCrashes) / totalIterations
            : 0.0;
    }

    double IterationsPerSecond() const {
        return totalDurationMs > 0
            ? totalIterations / (totalDurationMs / 1000.0)
            : 0.0;
    }

    std::string Summary() const {
        return "Iterations=" + std::to_string(totalIterations) +
               " Crashes=" + std::to_string(totalCrashes) +
               " Rate=" + std::to_string(static_cast<int>(CrashRate() * 100)) + "%" +
               " Speed=" + std::to_string(static_cast<int>(IterationsPerSecond())) + "/s";
    }
};

// ─── Byte mutator ────────────────────────────────────────────────
class ByteMutator {
public:
    explicit ByteMutator(uint32_t seed = 42) : m_rng(seed) {}

    std::vector<uint8_t> Mutate(const std::vector<uint8_t>& input,
                                 MutationStrategy strategy) {
        if (input.empty()) return input;
        auto output = input;

        switch (strategy) {
            case MutationStrategy::BitFlip: {
                size_t pos = m_rng() % output.size();
                uint8_t bit = 1 << (m_rng() % 8);
                output[pos] ^= bit;
                break;
            }
            case MutationStrategy::ByteFlip: {
                size_t pos = m_rng() % output.size();
                output[pos] = static_cast<uint8_t>(m_rng() % 256);
                break;
            }
            case MutationStrategy::Truncation: {
                size_t newLen = 1 + (m_rng() % output.size());
                output.resize(newLen);
                break;
            }
            case MutationStrategy::HeaderCorrupt: {
                size_t headerLen = std::min<size_t>(16, output.size());
                for (size_t i = 0; i < headerLen; i++) {
                    if (m_rng() % 4 == 0)
                        output[i] = static_cast<uint8_t>(m_rng() % 256);
                }
                break;
            }
            case MutationStrategy::Boundary: {
                size_t pos = m_rng() % output.size();
                static const uint8_t boundaries[] = {0, 1, 127, 128, 255};
                output[pos] = boundaries[m_rng() % 5];
                break;
            }
            default:
                // Havoc: multiple random mutations
                for (int i = 0; i < 3; i++) {
                    size_t pos = m_rng() % output.size();
                    output[pos] = static_cast<uint8_t>(m_rng() % 256);
                }
                break;
        }
        return output;
    }

    uint64_t MutationCount() const { return m_count; }

private:
    std::mt19937 m_rng;
    uint64_t     m_count = 0;
};

// ─── Continuous Fuzzing Engine ───────────────────────────────────
class ContinuousFuzzEngine {
public:
    explicit ContinuousFuzzEngine(FuzzConfig config = FuzzConfig::Quick())
        : m_config(config), m_mutator(42) {}

    void AddCorpusEntry(const CorpusEntry& entry) {
        m_corpus.push_back(entry);
        m_stats.corpusSize = m_corpus.size();
    }

    FuzzExecution RunSingle(const std::vector<uint8_t>& input,
                             const std::string& decoderName,
                             MutationStrategy strategy) {
        FuzzExecution exec;
        exec.id = ++m_nextId;
        exec.decoderName = decoderName;
        exec.strategy = strategy;

        auto mutated = m_mutator.Mutate(input, strategy);

        // Stub: In production, would invoke actual decoder with SEH wrapper
        exec.result = FuzzResult::NoError;
        exec.durationMs = 1.0;
        exec.memoryPeakBytes = mutated.size() * 4;

        m_config.budget.Record(exec.result);
        RecordResult(exec);

        return exec;
    }

    bool IsBudgetExhausted() const { return m_config.budget.IsExhausted(); }
    bool IsBudgetClean() const { return m_config.budget.IsClean(); }

    FuzzStats GetStats() const { return m_stats; }
    FuzzConfig GetConfig() const { return m_config; }
    size_t CorpusSize() const { return m_corpus.size(); }

    static ContinuousFuzzEngine Create(FuzzConfig config = FuzzConfig::Quick()) {
        return ContinuousFuzzEngine(config);
    }

private:
    void RecordResult(const FuzzExecution& exec) {
        m_stats.totalIterations++;
        m_stats.totalDurationMs += exec.durationMs;

        switch (exec.result) {
            case FuzzResult::NoError:
                m_stats.totalNoErrors++; break;
            case FuzzResult::ErrorHandled:
                m_stats.totalErrorsHandled++; break;
            case FuzzResult::Crash:
            case FuzzResult::Assertion:
                m_stats.totalCrashes++;
                m_stats.uniqueCrashes++; break;
            case FuzzResult::Timeout:
            case FuzzResult::Hang:
                m_stats.totalTimeouts++; break;
            default: break;
        }
    }

    FuzzConfig              m_config;
    FuzzStats               m_stats;
    ByteMutator             m_mutator;
    std::vector<CorpusEntry> m_corpus;
    uint64_t                m_nextId = 0;
};

} // namespace ExplorerLens::Utils

