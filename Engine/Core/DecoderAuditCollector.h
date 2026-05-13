// Engine/Core/DecoderAuditCollector.h
#pragma once

// DecoderAuditCollector — per-decoder error telemetry (S366)
//
// Implements H48: "Track which decoders fail most. Prioritize stability fixes
// by real-world failure rate." (ROADMAP v8.0 §5, Phase 2/3).
//
// The collector records decode failures by decoder ID, aggregates counts,
// and surfaces a ranked table of decoder stability. Data is stored in-process
// and periodically written to the SQLite decode_errors table.
//
// Thread safety: all methods are thread-safe (internal SRWLOCK / shared_mutex).
//
// Usage:
//   DecoderAuditCollector& audit = DecoderAuditCollector::Global();
//   audit.Record(L"LibRawDecoder", L"cr3", HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
//   auto report = audit.Summary();
//   // report.entries[0] is the decoder with most failures

#ifndef EXPLORERLENS_ENGINE_DECODERAUDITCOLLECTOR_H
#define EXPLORERLENS_ENGINE_DECODERAUDITCOLLECTOR_H

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

/// Maximum number of unique decoder IDs tracked simultaneously.
inline constexpr std::uint32_t kDecoderAuditMaxDecoders = 64u;
/// Maximum recent failures retained per decoder (ring buffer, oldest discarded).
inline constexpr std::uint32_t kDecoderAuditMaxRecentPerDecoder = 16u;

// ---------------------------------------------------------------------------
// DecoderAuditStatus
// ---------------------------------------------------------------------------
enum class DecoderAuditStatus : std::uint8_t {
    OK                  = 0,
    RECORDER_FULL       = 1,  ///< kDecoderAuditMaxDecoders reached; entry dropped
    NULL_DECODER_ID     = 2,
    FLUSH_IO_ERROR      = 3,  ///< Could not write to SQLite
};

// ---------------------------------------------------------------------------
// DecoderAuditEntry — aggregated failure record per decoder
// ---------------------------------------------------------------------------
struct DecoderAuditEntry {
    std::wstring  decoderId;             ///< Decoder class name (e.g. L"LibRawDecoder")
    std::uint64_t totalDecodes    = 0u;  ///< Total decode invocations recorded
    std::uint64_t totalFailures   = 0u;  ///< Total failure events recorded
    std::uint64_t totalSuccesses  = 0u;  ///< Inferred: totalDecodes - totalFailures
    std::uint32_t uniqueExtsFailed = 0u; ///< Distinct file extensions that produced failures
    std::int32_t  lastHresult     = 0;   ///< HRESULT of the most recent failure

    [[nodiscard]] double FailureRate() const noexcept {
        return totalDecodes > 0u
            ? static_cast<double>(totalFailures) / static_cast<double>(totalDecodes)
            : 0.0;
    }

    [[nodiscard]] bool IsClean() const noexcept { return totalFailures == 0u; }
};

// ---------------------------------------------------------------------------
// DecoderAuditSummary — snapshot report across all decoders
// ---------------------------------------------------------------------------
struct DecoderAuditSummary {
    std::vector<DecoderAuditEntry> entries;   ///< Sorted descending by failure count
    std::uint64_t totalDecodes    = 0u;       ///< Grand total decode invocations
    std::uint64_t totalFailures   = 0u;       ///< Grand total failures
    std::uint32_t cleanDecoders   = 0u;       ///< Decoders with zero failures
    std::uint32_t failingDecoders = 0u;       ///< Decoders with at least one failure

    [[nodiscard]] bool AllClean() const noexcept { return failingDecoders == 0u; }
};

// ---------------------------------------------------------------------------
// DecoderAuditConfig
// ---------------------------------------------------------------------------
struct DecoderAuditConfig {
    std::uint32_t maxDecoders            = kDecoderAuditMaxDecoders;
    std::uint32_t maxRecentPerDecoder    = kDecoderAuditMaxRecentPerDecoder;
    bool          flushToSqliteOnEvict   = false; ///< Write evicted records to DB
    bool          trackSuccesses         = true;  ///< Also count successful decodes

    [[nodiscard]] static DecoderAuditConfig Default() noexcept {
        return DecoderAuditConfig{};
    }

    [[nodiscard]] static DecoderAuditConfig ForProduction() noexcept {
        DecoderAuditConfig cfg{};
        cfg.flushToSqliteOnEvict = true;
        cfg.trackSuccesses       = true;
        return cfg;
    }
};

// ---------------------------------------------------------------------------
// DecoderAuditCollector — singleton audit recorder
// ---------------------------------------------------------------------------
class DecoderAuditCollector final {
public:
    DecoderAuditCollector(const DecoderAuditCollector&)            = delete;
    DecoderAuditCollector& operator=(const DecoderAuditCollector&) = delete;

    /// Returns the process-wide singleton.
    [[nodiscard]] static DecoderAuditCollector& Global() noexcept;

    /// Applies configuration. Call before first Record.
    void Configure(const DecoderAuditConfig& cfg) noexcept;

    /// Records a successful decode. decoderId must be non-null.
    [[nodiscard]] DecoderAuditStatus RecordSuccess(
        const wchar_t* decoderId,
        const wchar_t* fileExtension = nullptr) noexcept;

    /// Records a decode failure with optional file extension and HRESULT.
    [[nodiscard]] DecoderAuditStatus RecordFailure(
        const wchar_t* decoderId,
        const wchar_t* fileExtension = nullptr,
        std::int32_t   hresult       = 0) noexcept;

    /// Returns a snapshot summary sorted by failure count descending.
    [[nodiscard]] DecoderAuditSummary Summary() const noexcept;

    /// Returns the entry for a specific decoder, or nullptr if not found.
    [[nodiscard]] const DecoderAuditEntry* Lookup(const std::wstring& decoderId) const noexcept;

    /// Clears all recorded data.
    void Reset() noexcept;

    /// Returns true if no decoders have any recorded failures.
    [[nodiscard]] bool AllClean() const noexcept;

    /// Returns the count of tracked decoders.
    [[nodiscard]] std::uint32_t DecoderCount() const noexcept;

private:
    DecoderAuditCollector() noexcept = default;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DECODERAUDITCOLLECTOR_H
