// Engine/Pipeline/ReadaheadIOContract.h
// ExplorerLens — Parallel I/O readahead N=8 contract (H9 / ROADMAP v7.0 Phase 2)
// Sprint S316.
//
// Purpose:
//   When Explorer opens a folder with 200+ thumbnails, each file is opened,
//   read, decoded, and closed sequentially.  Storage I/O is the dominant
//   bottleneck for HDD and even NVMe on cold paths.
//
//   The readahead pattern (Harvested from IrfanView batch mode — H9) solves
//   this by:
//     1. Accepting a ring-buffer of N=8 IStream slots.
//     2. Prefetching the next 8 files while the GPU decodes the current one.
//     3. Delivering IStream* already at position 0 — zero seek latency.
//
//   Expected throughput improvement (H9 harvest):
//     HDD:  140 → 350 thumbnails/sec  (2.5×, I/O-bound → compute-bound)
//     NVMe: 235 → 400 thumbnails/sec  (1.7×, overlap decode + I/O)
//
// Phase 2 wiring:
//   - DecodePipeline::ProcessBatch() will call ReadaheadIOContract::Enqueue()
//     for the next N items while decoding the current item.
//   - Requires ATL atlbase.h for IStream — not included here.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_READAHEAD_IO_CONTRACT_H
#define EXPLORERLENS_ENGINE_READAHEAD_IO_CONTRACT_H

#include <cstdint>
#include <array>
#include <atomic>

// Forward declaration — avoids pulling in objidl.h
struct IStream;

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// ReadaheadSlotState
// ---------------------------------------------------------------------------
enum class ReadaheadSlotState : std::uint8_t {
    EMPTY    = 0,   ///< Slot is available for a new file path
    PENDING  = 1,   ///< Async I/O prefetch in flight
    READY    = 2,   ///< IStream ready for consumer
    CONSUMED = 3,   ///< Consumer has taken the stream; slot can be reused
    ERROR    = 4,   ///< Prefetch failed (file not found, access denied, etc.)
};

// ---------------------------------------------------------------------------
// ReadaheadSlot
// ---------------------------------------------------------------------------
struct ReadaheadSlot final {
    ReadaheadSlotState   state{ ReadaheadSlotState::EMPTY };
    IStream*             stream{ nullptr };    ///< Non-owning; AddRef'd by prefetch
    std::uint64_t        filePathHash{};       ///< CacheKeyV2 hash of the path
    std::uint32_t        fileSizeBytes{};      ///< 0 if unknown at queue time
    std::uint32_t        prefetchDurationUs{}; ///< Measured prefetch wall time
};

// ---------------------------------------------------------------------------
// ReadaheadIOConfig
// ---------------------------------------------------------------------------
struct ReadaheadIOConfig final {
    // Number of concurrent prefetch slots.  8 matches Explorer's typical
    // parallelism and saturates most NVMe queues without overcommitting.
    static constexpr std::uint32_t kDefaultSlotCount = 8u;
    static constexpr std::uint32_t kMaxSlotCount     = 32u;
    static constexpr std::uint32_t kMinSlotCount     = 1u;

    std::uint32_t slotCount = kDefaultSlotCount;

    // Maximum file size to prefetch into RAM.  Files larger than this are
    // opened but not read ahead (the IStream is returned at position 0).
    std::uint32_t maxPrefetchBytes = 32u * 1024u * 1024u;  // 32 MiB

    // If true, prefetch I/O runs on the thread pool; false = synchronous
    // (synchronous mode used in unit tests and low-memory environments).
    bool asyncPrefetch = true;
};

// ---------------------------------------------------------------------------
// ReadaheadIOStatus
// ---------------------------------------------------------------------------
enum class ReadaheadIOStatus : std::uint8_t {
    OK              = 0,
    QUEUE_FULL      = 1,   ///< All slots occupied; caller should wait
    INVALID_PATH    = 2,   ///< Null or empty file path
    ALREADY_QUEUED  = 3,   ///< Same file hash already in a slot
    NOT_INITIALISED = 4,   ///< Call Initialize() first
    DISABLED        = 5,   ///< Phase 2 stub
};

// ---------------------------------------------------------------------------
// ReadaheadIOContract
// ---------------------------------------------------------------------------
// Phase 2 stub — Enqueue and Consume are no-ops that return DISABLED until
// the thread-pool + IStream prefetch is wired.
//
class ReadaheadIOContract final {
public:
    ReadaheadIOContract() noexcept  = default;
    ~ReadaheadIOContract() noexcept = default;

    ReadaheadIOContract(const ReadaheadIOContract&)            = delete;
    ReadaheadIOContract& operator=(const ReadaheadIOContract&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────────

    /// Initialize the slot pool.  Must be called before Enqueue().
    /// Phase 2 stub: always returns OK.
    [[nodiscard]] ReadaheadIOStatus Initialize(
        const ReadaheadIOConfig& cfg = ReadaheadIOConfig{}) noexcept
    {
        (void)cfg;
        m_initialised.store(true, std::memory_order_release);
        return ReadaheadIOStatus::OK;
    }

    void Shutdown() noexcept
    {
        m_initialised.store(false, std::memory_order_release);
    }

    [[nodiscard]] bool IsInitialised() const noexcept
    {
        return m_initialised.load(std::memory_order_acquire);
    }

    // ── Queue management ──────────────────────────────────────────────────────

    /// Enqueue a file path hash for prefetch.
    /// Phase 2 stub: returns DISABLED.
    [[nodiscard]] ReadaheadIOStatus Enqueue(
        std::uint64_t filePathHash,
        std::uint32_t fileSizeHintBytes = 0u) noexcept
    {
        (void)filePathHash; (void)fileSizeHintBytes;
        return ReadaheadIOStatus::DISABLED;  // Phase 2 stub
    }

    /// Consume a ready IStream for a given file hash.
    /// Returns nullptr (stub); caller opens the file itself.
    [[nodiscard]] IStream* Consume(
        [[maybe_unused]] std::uint64_t filePathHash) noexcept
    {
        return nullptr;   // Phase 2 stub
    }

    /// Cancel all pending prefetches.
    void CancelAll() noexcept {}

    // ── Diagnostics ───────────────────────────────────────────────────────────

    [[nodiscard]] std::uint32_t ReadySlotCount() const noexcept { return 0u; }
    [[nodiscard]] std::uint32_t PendingSlotCount() const noexcept { return 0u; }

    // ── Constants ─────────────────────────────────────────────────────────────

    static constexpr std::uint32_t kDefaultSlotCount  = ReadaheadIOConfig::kDefaultSlotCount;
    static constexpr std::uint32_t kMaxSlotCount       = ReadaheadIOConfig::kMaxSlotCount;

private:
    std::atomic<bool> m_initialised{ false };
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_READAHEAD_IO_CONTRACT_H
