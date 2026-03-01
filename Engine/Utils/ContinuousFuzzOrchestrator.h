// ContinuousFuzzOrchestrator.h — Continuous Fuzzing Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates continuous fuzzing campaigns across decoder pipeline.
// Manages corpus databases, mutation strategies, coverage tracking,
// and crash triage with fuzzing-as-a-service CI integration.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Enums
// ============================================================================

/// Fuzzing mutation strategy
enum class FuzzMutationStrategy : uint8_t {
    BitFlip = 0,        ///< Random bit flips
    ByteReplace,        ///< Replace random bytes
    BlockShuffle,       ///< Shuffle data blocks
    HeaderCorrupt,      ///< Target format header fields
    BoundaryValue,      ///< Insert min/max/boundary values
    StructureAware,     ///< Grammar-based format mutations
    Havoc,              ///< AFL-style havoc (combined random ops)
    Splice,             ///< Cross-corpus splicing
    COUNT
};

/// Crash severity classification
enum class CrashSeverity : uint8_t {
    None = 0,
    Info,           ///< Non-crashing anomaly (e.g., slow decode)
    Low,            ///< Benign crash (null deref, recoverable)
    Medium,         ///< Potential DoS (infinite loop, OOM)
    High,           ///< Memory corruption (heap overflow, UAF)
    Critical,       ///< Code execution potential (RCE)
    COUNT
};

/// Corpus management action
enum class CorpusAction : uint8_t {
    Import = 0,         ///< Import seed files
    Minimize,           ///< Remove redundant inputs
    Merge,              ///< Combine corpora
    Export,             ///< Export interesting inputs
    Distill,            ///< Reduce to coverage-unique set
    COUNT
};

// ============================================================================
// String conversions
// ============================================================================

inline const char* FuzzMutationStrategyToString(FuzzMutationStrategy s) {
    static const char* names[] = {
        "BitFlip", "ByteReplace", "BlockShuffle", "HeaderCorrupt",
        "BoundaryValue", "StructureAware", "Havoc", "Splice"
    };
    auto idx = static_cast<uint8_t>(s);
    return (idx < static_cast<uint8_t>(FuzzMutationStrategy::COUNT)) ? names[idx] : "Unknown";
}

inline const char* CrashSeverityToString(CrashSeverity s) {
    static const char* names[] = {
        "None", "Info", "Low", "Medium", "High", "Critical"
    };
    auto idx = static_cast<uint8_t>(s);
    return (idx < static_cast<uint8_t>(CrashSeverity::COUNT)) ? names[idx] : "Unknown";
}

inline const char* CorpusActionToString(CorpusAction a) {
    static const char* names[] = {
        "Import", "Minimize", "Merge", "Export", "Distill"
    };
    auto idx = static_cast<uint8_t>(a);
    return (idx < static_cast<uint8_t>(CorpusAction::COUNT)) ? names[idx] : "Unknown";
}

// ============================================================================
// Structs
// ============================================================================

/// A crash finding from fuzzing
struct FuzzCrashReport {
    std::string     inputHash;      ///< SHA-256 of crashing input
    std::string     decoderName;    ///< Which decoder crashed
    CrashSeverity   severity = CrashSeverity::None;
    std::string     stackTrace;
    uint64_t        inputSizeBytes = 0;
    double          discoveryTimeSec = 0.0;
    bool            isDuplicate = false;
    bool            isFixed = false;
};

/// Fuzzing campaign statistics
struct FuzzCampaignStats {
    uint64_t totalExecutions = 0;
    uint64_t corpusSize = 0;
    uint64_t uniqueCrashes = 0;
    uint64_t duplicateCrashes = 0;
    double   execsPerSecond = 0.0;
    double   coveragePct = 0.0;
    double   campaignHours = 0.0;

    double CrashDensity() const {
        return (totalExecutions > 0) ?
            (static_cast<double>(uniqueCrashes) / totalExecutions * 1000000.0) : 0.0;
    }
};

/// Corpus entry metadata
struct CorpusEntry {
    std::string hash;
    uint64_t    sizeBytes = 0;
    bool        isInteresting = false;  ///< Covers new code paths
    double      executionTimeMs = 0.0;
    uint32_t    coverageEdges = 0;
};

// ============================================================================
// ContinuousFuzzOrchestrator class
// ============================================================================

class ContinuousFuzzOrchestrator {
public:
    /// Initialize with target decoder list
    void Initialize(const std::vector<std::string>& decoders) {
        m_targetDecoders = decoders;
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    /// Add seed corpus entries
    void AddCorpusEntry(const std::string& hash, uint64_t size, uint32_t edges) {
        CorpusEntry entry;
        entry.hash = hash;
        entry.sizeBytes = size;
        entry.coverageEdges = edges;
        entry.isInteresting = (edges > 0);
        m_corpus.push_back(std::move(entry));
        m_stats.corpusSize = m_corpus.size();
    }

    /// Record a crash finding
    void RecordCrash(const std::string& decoder, CrashSeverity severity,
        const std::string& inputHash, uint64_t inputSize) {
        // Check for duplicate
        for (const auto& c : m_crashes) {
            if (c.inputHash == inputHash) {
                m_stats.duplicateCrashes++;
                return;
            }
        }

        FuzzCrashReport report;
        report.decoderName = decoder;
        report.severity = severity;
        report.inputHash = inputHash;
        report.inputSizeBytes = inputSize;
        m_crashes.push_back(std::move(report));
        m_stats.uniqueCrashes = m_crashes.size();
    }

    /// Simulate fuzzing executions
    void RecordExecutions(uint64_t count, double durationSec) {
        m_stats.totalExecutions += count;
        m_stats.campaignHours += durationSec / 3600.0;
        if (durationSec > 0)
            m_stats.execsPerSecond = static_cast<double>(count) / durationSec;
    }

    /// Get campaign statistics
    const FuzzCampaignStats& GetStats() const { return m_stats; }

    /// Get all crashes
    const std::vector<FuzzCrashReport>& GetCrashes() const { return m_crashes; }

    /// Get corpus entries
    const std::vector<CorpusEntry>& GetCorpus() const { return m_corpus; }

    /// Get crashes by severity
    std::vector<FuzzCrashReport> GetCrashesBySeverity(CrashSeverity minSeverity) const {
        std::vector<FuzzCrashReport> filtered;
        for (const auto& c : m_crashes) {
            if (static_cast<uint8_t>(c.severity) >= static_cast<uint8_t>(minSeverity))
                filtered.push_back(c);
        }
        return filtered;
    }

    /// Minimize corpus (remove entries with no unique coverage)
    uint32_t MinimizeCorpus() {
        auto before = m_corpus.size();
        m_corpus.erase(
            std::remove_if(m_corpus.begin(), m_corpus.end(),
                [](const CorpusEntry& e) { return !e.isInteresting; }),
            m_corpus.end());
        m_stats.corpusSize = m_corpus.size();
        return static_cast<uint32_t>(before - m_corpus.size());
    }

    /// Get target decoders
    const std::vector<std::string>& GetTargetDecoders() const { return m_targetDecoders; }

private:
    bool m_initialized = false;
    std::vector<std::string>     m_targetDecoders;
    std::vector<CorpusEntry>     m_corpus;
    std::vector<FuzzCrashReport> m_crashes;
    FuzzCampaignStats            m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
