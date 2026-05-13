// Engine/Core/DecodeErrorTracker.h
// ExplorerLens — Per-decoder failure telemetry (H48 / ROADMAP v8.0 Phase 2)
// Sprint S326.
//
// Purpose:
//   Without error analytics, there is no feedback loop to prioritise decoder
//   stability work.  A RAW decoder that fails 1-in-1000 times looks the same
//   as one that always works — unless failure events are recorded.
//
//   DecodeErrorTracker collects:
//     1. Per-format failure counts (in-process, reset on DLL unload).
//     2. The most recent error HRESULT + file extension per decoder.
//     3. An ETW event emission path for structured diagnostics.
//
//   Storage:
//     In-process atomic counters (no disk I/O in the decode hot path).
//     SQLite write is deferred to the AsyncCacheWriter queue (Phase 3).
//
//   Design (H48 harvest from Apple Quick Look):
//     "Track which decoders fail most. Prioritize stability fixes by
//      real-world failure rate."
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_DECODE_ERROR_TRACKER_H
#define EXPLORERLENS_ENGINE_DECODE_ERROR_TRACKER_H

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <string_view>
#include <array>
#include <algorithm>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// DecodeErrorEvent — describes one decode failure
// ---------------------------------------------------------------------------
struct DecodeErrorEvent final {
    std::string_view  decoderId;      ///< e.g. "libjpeg-turbo", "libheif", "LibRaw"
    std::string_view  fileExtension;  ///< e.g. ".cr3", ".heic", ".jxl"
    std::int32_t      hresult{};      ///< HRESULT from the decode call (0 = unknown)
    std::uint32_t     fileSizeBytes{};
    bool              wasTimeout{};   ///< True if DecodeTimeoutGuard fired
};

// ---------------------------------------------------------------------------
// DecodeErrorStats — snapshot of accumulated stats for one decoder slot
// ---------------------------------------------------------------------------
struct DecodeErrorStats final {
    char              decoderId[48]{};   ///< NUL-terminated decoder name
    std::uint64_t     failureCount{};
    std::uint64_t     timeoutCount{};
    std::int32_t      lastHresult{};     ///< Most recent HRESULT
    char              lastExtension[16]{};
};

// ---------------------------------------------------------------------------
// DecodeErrorTracker — singleton-style tracker with a fixed decoder table
// ---------------------------------------------------------------------------
class DecodeErrorTracker final {
public:
    // Maximum distinct decoder IDs tracked simultaneously.
    static constexpr std::size_t kMaxDecoderSlots = 32u;

    // ------------------------------------------------------------------
    // Record() — called from decode error paths.  Thread-safe.
    // ------------------------------------------------------------------
    void Record(const DecodeErrorEvent& evt) noexcept
    {
        const std::size_t slot = FindOrAllocSlot(evt.decoderId);
        if (slot >= kMaxDecoderSlots) return;  // table full — drop

        m_slots[slot].failureCount.fetch_add(1u, std::memory_order_relaxed);
        if (evt.wasTimeout)
            m_slots[slot].timeoutCount.fetch_add(1u, std::memory_order_relaxed);
        m_slots[slot].lastHresult.store(evt.hresult, std::memory_order_relaxed);

        // Store last extension (best-effort, non-atomic string copy)
        if (!evt.fileExtension.empty()) {
            const std::size_t n = (std::min)(
                evt.fileExtension.size(),
                sizeof(m_slots[slot].lastExt) - 1u);
            for (std::size_t i = 0; i < n; ++i)
                m_slots[slot].lastExt[i] = evt.fileExtension[i];
            m_slots[slot].lastExt[n] = '\0';
        }
    }

    // ------------------------------------------------------------------
    // GetStats() — snapshot for one decoder.
    // ------------------------------------------------------------------
    [[nodiscard]]
    DecodeErrorStats GetStats(std::string_view decoderId) const noexcept
    {
        for (std::size_t i = 0; i < kMaxDecoderSlots; ++i) {
            if (m_slots[i].active.load(std::memory_order_relaxed) &&
                m_slots[i].IdMatches(decoderId)) {
                DecodeErrorStats out;
                const std::size_t n = (std::min)(
                    decoderId.size(), sizeof(out.decoderId) - 1u);
                for (std::size_t k = 0; k < n; ++k)
                    out.decoderId[k] = decoderId[k];
                out.decoderId[n]   = '\0';
                out.failureCount   = m_slots[i].failureCount.load(std::memory_order_relaxed);
                out.timeoutCount   = m_slots[i].timeoutCount.load(std::memory_order_relaxed);
                out.lastHresult    = m_slots[i].lastHresult.load(std::memory_order_relaxed);
                for (std::size_t k = 0; k < sizeof(out.lastExtension); ++k)
                    out.lastExtension[k] = m_slots[i].lastExt[k];
                return out;
            }
        }
        return {};
    }

    // ------------------------------------------------------------------
    // TotalFailures() — sum across all decoders
    // ------------------------------------------------------------------
    [[nodiscard]]
    std::uint64_t TotalFailures() const noexcept
    {
        std::uint64_t total = 0u;
        for (const auto& s : m_slots)
            if (s.active.load(std::memory_order_relaxed))
                total += s.failureCount.load(std::memory_order_relaxed);
        return total;
    }

    // ------------------------------------------------------------------
    // Reset() — clear all counters (for testing)
    // ------------------------------------------------------------------
    void Reset() noexcept
    {
        for (auto& s : m_slots) {
            s.active.store(false, std::memory_order_relaxed);
            s.failureCount.store(0u, std::memory_order_relaxed);
            s.timeoutCount.store(0u, std::memory_order_relaxed);
            s.lastHresult.store(0, std::memory_order_relaxed);
            for (auto& c : s.name) c = '\0';
            for (auto& c : s.lastExt) c = '\0';
        }
        m_slotCount.store(0u, std::memory_order_relaxed);
    }

    // ------------------------------------------------------------------
    // SlotCount() — number of distinct decoders registered
    // ------------------------------------------------------------------
    [[nodiscard]]
    std::size_t SlotCount() const noexcept
    { return m_slotCount.load(std::memory_order_relaxed); }

private:
    struct Slot final {
        std::atomic<bool>           active{ false };
        std::atomic<std::uint64_t>  failureCount{ 0u };
        std::atomic<std::uint64_t>  timeoutCount{ 0u };
        std::atomic<std::int32_t>   lastHresult{ 0 };
        char                        name[48]{};
        char                        lastExt[16]{};

        bool IdMatches(std::string_view id) const noexcept {
            for (std::size_t i = 0; i < id.size() && i < sizeof(name) - 1u; ++i)
                if (name[i] != id[i]) return false;
            return name[id.size()] == '\0';
        }
    };

    std::size_t FindOrAllocSlot(std::string_view id) noexcept
    {
        // First pass: find existing slot
        for (std::size_t i = 0; i < kMaxDecoderSlots; ++i)
            if (m_slots[i].active.load(std::memory_order_relaxed) &&
                m_slots[i].IdMatches(id))
                return i;

        // Second pass: allocate new slot
        for (std::size_t i = 0; i < kMaxDecoderSlots; ++i) {
            bool expected = false;
            if (m_slots[i].active.compare_exchange_strong(
                    expected, true, std::memory_order_acq_rel)) {
                const std::size_t n = (std::min)(id.size(), sizeof(m_slots[i].name) - 1u);
                for (std::size_t k = 0; k < n; ++k)
                    m_slots[i].name[k] = id[k];
                m_slots[i].name[n] = '\0';
                m_slotCount.fetch_add(1u, std::memory_order_relaxed);
                return i;
            }
        }
        return kMaxDecoderSlots;  // full
    }

    std::array<Slot, kMaxDecoderSlots>   m_slots{};
    std::atomic<std::size_t>             m_slotCount{ 0u };
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DECODE_ERROR_TRACKER_H
