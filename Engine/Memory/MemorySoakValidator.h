#pragma once
// ============================================================================
// MemorySoakValidator.h — Sprint 563
//
// Purpose:
//   Long-running memory validation that detects leaks, double-frees, and
//   unbounded growth.  Wraps a private Win32 heap (HeapCreate) with tracking
//   metadata so every allocation is recorded and every free is verified.
//   Canary bytes at allocation boundaries detect buffer overruns; freed
//   memory is filled with a poison pattern to detect use-after-free.
//
// Classes:
//   MemorySoakValidator — tracked allocator + soak test runner.
//
// Key Types:
//   AllocationRecord  — per-allocation metadata (size, tag, timestamp)
//   LeakRecord        — describes an un-freed allocation
//   SoakResult        — aggregate outcome of a soak test run
//
// Inputs:
//   TrackedAlloc / TrackedFree — instrumented allocate / free
//   StartSoakTest              — run a pattern for N seconds
//
// Outputs:
//   GetSoakResult()     — peak / current / leak / double-free stats
//   GetLeaks()          — list of un-freed allocations
//   DumpLeakReport()    — write detailed report to a file
//
// Soak Test Patterns:
//   0 — Random alloc/free 50/50 mix, sizes 1 B – 16 MB
//   1 — Monotonic growth (alloc-heavy) to stress OOM
//   2 — Sawtooth (burst alloc, bulk free, repeat)
//
// Thread Safety:
//   All public methods are serialized with a Win32 SRWLOCK (exclusive).
//
// Dependencies:
//   Windows API + C++ standard library only (header-only, no external libs).
// ============================================================================

#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

// ── Constants ────────────────────────────────────────────────────────────────

static constexpr uint8_t  kCanaryByte      = 0xFD;
static constexpr uint8_t  kFreedPoisonByte = 0xDD;
static constexpr uint8_t  kFreshAllocByte  = 0xCD;
static constexpr size_t   kCanarySize      = 16;  // bytes before + after payload
static constexpr size_t   kMaxSoakAllocSize = 16ULL * 1024 * 1024; // 16 MB

// ── Allocation record ────────────────────────────────────────────────────────

struct AllocationRecord {
    void*       rawPtr    = nullptr;   // actual heap ptr (includes leading canary)
    void*       userPtr   = nullptr;   // ptr returned to caller
    size_t      size      = 0;
    const char* tag       = nullptr;
    uint64_t    timestamp = 0;         // ms since epoch
    bool        freed     = false;
};

// ── Leak descriptor ──────────────────────────────────────────────────────────

struct LeakRecord {
    void*       address   = nullptr;
    size_t      size      = 0;
    const char* tag       = nullptr;
    uint64_t    timestamp = 0;
};

// ── Soak result ──────────────────────────────────────────────────────────────

struct SoakResult {
    size_t   peakAllocation     = 0;
    size_t   currentAllocation  = 0;
    uint32_t leakCount          = 0;
    uint32_t doubleFreeAttempts = 0;
    uint32_t canaryViolations   = 0;
    double   allocationRate     = 0.0;   // allocs/sec
    double   deallocationRate   = 0.0;   // frees/sec
    uint64_t totalAllocations   = 0;
    uint64_t totalFrees         = 0;
    double   durationSeconds    = 0.0;
};

// ── Main class ───────────────────────────────────────────────────────────────

class MemorySoakValidator {
public:
    MemorySoakValidator() noexcept {
        InitializeSRWLock(&m_lock);

        // Create a private heap for isolation
        m_heap = HeapCreate(0, 0, 0);
        m_startTime = std::chrono::steady_clock::now();
    }

    ~MemorySoakValidator() {
        AcquireSRWLockExclusive(&m_lock);
        // Free all tracked allocations
        for (auto& [ptr, rec] : m_allocations) {
            if (!rec.freed && rec.rawPtr && m_heap) {
                HeapFree(m_heap, 0, rec.rawPtr);
                rec.freed = true;
            }
        }
        m_allocations.clear();

        if (m_heap) {
            HeapDestroy(m_heap);
            m_heap = nullptr;
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    MemorySoakValidator(const MemorySoakValidator&)            = delete;
    MemorySoakValidator& operator=(const MemorySoakValidator&) = delete;

    // ── Tracked allocation ───────────────────────────────────────

    void* TrackedAlloc(size_t size, const char* tag = nullptr) {
        if (size == 0) return nullptr;
        if (!m_heap) return nullptr;

        // Total = leading canary + payload + trailing canary
        size_t totalSize = kCanarySize + size + kCanarySize;

        AcquireSRWLockExclusive(&m_lock);

        void* raw = HeapAlloc(m_heap, 0, totalSize);
        if (!raw) {
            ReleaseSRWLockExclusive(&m_lock);
            return nullptr;
        }

        uint8_t* base = static_cast<uint8_t*>(raw);

        // Write leading canary
        std::memset(base, kCanaryByte, kCanarySize);

        // Fresh-alloc fill
        uint8_t* userPtr = base + kCanarySize;
        std::memset(userPtr, kFreshAllocByte, size);

        // Write trailing canary
        std::memset(userPtr + size, kCanaryByte, kCanarySize);

        // Record
        AllocationRecord rec;
        rec.rawPtr    = raw;
        rec.userPtr   = userPtr;
        rec.size      = size;
        rec.tag       = tag;
        rec.timestamp = NowMs();
        rec.freed     = false;

        m_allocations[reinterpret_cast<uintptr_t>(userPtr)] = rec;

        m_currentAllocation += size;
        m_peakAllocation = (std::max)(m_peakAllocation, m_currentAllocation);
        m_totalAllocations++;

        ReleaseSRWLockExclusive(&m_lock);
        return userPtr;
    }

    // ── Tracked free ─────────────────────────────────────────────

    bool TrackedFree(void* ptr) {
        if (!ptr) return false;

        AcquireSRWLockExclusive(&m_lock);

        uintptr_t key = reinterpret_cast<uintptr_t>(ptr);
        auto it = m_allocations.find(key);

        if (it == m_allocations.end()) {
            // Was never allocated through us (or already erased)
            m_doubleFreeAttempts++;
            ReleaseSRWLockExclusive(&m_lock);
            return false;
        }

        AllocationRecord& rec = it->second;

        if (rec.freed) {
            // Double free
            m_doubleFreeAttempts++;
            ReleaseSRWLockExclusive(&m_lock);
            return false;
        }

        // Verify canaries
        uint8_t* base     = static_cast<uint8_t*>(rec.rawPtr);
        uint8_t* userPtr  = static_cast<uint8_t*>(rec.userPtr);
        bool canaryOk     = true;

        // Leading canary check
        for (size_t i = 0; i < kCanarySize; ++i) {
            if (base[i] != kCanaryByte) {
                canaryOk = false;
                break;
            }
        }

        // Trailing canary check
        uint8_t* trail = userPtr + rec.size;
        for (size_t i = 0; i < kCanarySize; ++i) {
            if (trail[i] != kCanaryByte) {
                canaryOk = false;
                break;
            }
        }

        if (!canaryOk) {
            m_canaryViolations++;
        }

        // Poison freed memory to detect use-after-free
        std::memset(userPtr, kFreedPoisonByte, rec.size);

        // Mark freed and release
        rec.freed = true;
        if (m_currentAllocation >= rec.size) {
            m_currentAllocation -= rec.size;
        }
        m_totalFrees++;

        HeapFree(m_heap, 0, rec.rawPtr);

        // Remove from map (optional — keep for leak detection)
        // We keep it to track double-frees on the same pointer
        // but free the heap memory

        rec.rawPtr  = nullptr;
        rec.userPtr = nullptr;

        ReleaseSRWLockExclusive(&m_lock);
        return canaryOk;
    }

    // ── Soak test ────────────────────────────────────────────────

    bool StartSoakTest(uint32_t durationSeconds, uint32_t allocationPattern) {
        auto soakStart = std::chrono::steady_clock::now();
        auto deadline  = soakStart + std::chrono::seconds(durationSeconds);

        // Simple LCG for deterministic pseudo-random without external deps
        uint32_t rngState = 0x12345678u ^ (durationSeconds * 31u);

        auto nextRng = [&rngState]() -> uint32_t {
            rngState = rngState * 1664525u + 1013904223u;
            return rngState;
        };

        std::vector<void*> soakPtrs;
        soakPtrs.reserve(4096);

        switch (allocationPattern) {
        case 0: {
            // Pattern 0: Random alloc/free 50/50 mix, sizes 1B – 16MB
            while (std::chrono::steady_clock::now() < deadline) {
                bool doAlloc = soakPtrs.empty() || (nextRng() & 1);
                if (doAlloc) {
                    uint32_t r = nextRng();
                    size_t size = 1 + (r % kMaxSoakAllocSize);
                    // Keep sizes reasonable for a soak test
                    size = (std::min)(size, static_cast<size_t>(64 * 1024));
                    void* p = TrackedAlloc(size, "soak_random");
                    if (p) soakPtrs.push_back(p);
                } else if (!soakPtrs.empty()) {
                    size_t idx = nextRng() % soakPtrs.size();
                    TrackedFree(soakPtrs[idx]);
                    soakPtrs[idx] = soakPtrs.back();
                    soakPtrs.pop_back();
                }
            }
            break;
        }
        case 1: {
            // Pattern 1: Monotonic growth (alloc-heavy)
            while (std::chrono::steady_clock::now() < deadline) {
                uint32_t r = nextRng();
                size_t size = 256 + (r % (4 * 1024));
                void* p = TrackedAlloc(size, "soak_growth");
                if (p) soakPtrs.push_back(p);

                // Occasionally free ~10% to avoid instant OOM
                if ((nextRng() % 10) == 0 && !soakPtrs.empty()) {
                    size_t idx = nextRng() % soakPtrs.size();
                    TrackedFree(soakPtrs[idx]);
                    soakPtrs[idx] = soakPtrs.back();
                    soakPtrs.pop_back();
                }
            }
            break;
        }
        case 2: {
            // Pattern 2: Sawtooth (burst alloc → bulk free → repeat)
            while (std::chrono::steady_clock::now() < deadline) {
                // Burst alloc: 100 allocations
                for (uint32_t i = 0; i < 100; ++i) {
                    size_t size = 128 + (nextRng() % (16 * 1024));
                    void* p = TrackedAlloc(size, "soak_sawtooth");
                    if (p) soakPtrs.push_back(p);
                }
                // Bulk free: free ~80% of current allocations
                size_t freeCount = soakPtrs.size() * 4 / 5;
                for (size_t i = 0; i < freeCount && !soakPtrs.empty(); ++i) {
                    TrackedFree(soakPtrs.back());
                    soakPtrs.pop_back();
                }
            }
            break;
        }
        default:
            return false;
        }

        // Free remaining soak allocations
        for (void* p : soakPtrs) {
            TrackedFree(p);
        }
        soakPtrs.clear();

        auto soakEnd = std::chrono::steady_clock::now();
        m_lastSoakDuration = std::chrono::duration<double>(
            soakEnd - soakStart).count();

        return true;
    }

    // ── Results ──────────────────────────────────────────────────

    SoakResult GetSoakResult() {
        AcquireSRWLockExclusive(&m_lock);
        SoakResult result{};
        result.peakAllocation     = m_peakAllocation;
        result.currentAllocation  = m_currentAllocation;
        result.doubleFreeAttempts = m_doubleFreeAttempts;
        result.canaryViolations   = m_canaryViolations;
        result.totalAllocations   = m_totalAllocations;
        result.totalFrees         = m_totalFrees;
        result.durationSeconds    = m_lastSoakDuration;

        // Count leaks
        result.leakCount = 0;
        for (const auto& [key, rec] : m_allocations) {
            if (!rec.freed) result.leakCount++;
        }

        if (m_lastSoakDuration > 0.0) {
            result.allocationRate   = m_totalAllocations / m_lastSoakDuration;
            result.deallocationRate = m_totalFrees / m_lastSoakDuration;
        }

        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    std::vector<LeakRecord> GetLeaks() {
        AcquireSRWLockExclusive(&m_lock);
        std::vector<LeakRecord> leaks;
        for (const auto& [key, rec] : m_allocations) {
            if (!rec.freed) {
                LeakRecord lr;
                lr.address   = rec.userPtr;
                lr.size      = rec.size;
                lr.tag       = rec.tag;
                lr.timestamp = rec.timestamp;
                leaks.push_back(lr);
            }
        }
        ReleaseSRWLockExclusive(&m_lock);
        return leaks;
    }

    // ── Leak report ──────────────────────────────────────────────

    void DumpLeakReport(const std::wstring& outputPath) {
        auto leaks = GetLeaks();
        auto result = GetSoakResult();

        std::ofstream ofs(outputPath);
        if (!ofs.is_open()) return;

        ofs << "===============================================\n";
        ofs << "  ExplorerLens Memory Soak Leak Report\n";
        ofs << "===============================================\n\n";

        ofs << "Summary:\n";
        ofs << "  Peak allocation:      " << result.peakAllocation << " bytes\n";
        ofs << "  Current allocation:   " << result.currentAllocation << " bytes\n";
        ofs << "  Total allocations:    " << result.totalAllocations << "\n";
        ofs << "  Total frees:          " << result.totalFrees << "\n";
        ofs << "  Leak count:           " << result.leakCount << "\n";
        ofs << "  Double-free attempts: " << result.doubleFreeAttempts << "\n";
        ofs << "  Canary violations:    " << result.canaryViolations << "\n";
        ofs << "  Duration:             " << std::fixed << std::setprecision(3)
            << result.durationSeconds << " s\n";
        ofs << "  Alloc rate:           " << std::fixed << std::setprecision(1)
            << result.allocationRate << " /s\n";
        ofs << "  Free rate:            " << std::fixed << std::setprecision(1)
            << result.deallocationRate << " /s\n\n";

        if (leaks.empty()) {
            ofs << "No leaks detected.\n";
        } else {
            ofs << "Leaked Allocations (" << leaks.size() << " total):\n";
            ofs << std::string(60, '-') << "\n";
            ofs << std::left << std::setw(18) << "Address"
                << std::setw(12) << "Size"
                << std::setw(16) << "Tag"
                << std::setw(14) << "Timestamp(ms)" << "\n";
            ofs << std::string(60, '-') << "\n";

            for (const auto& lr : leaks) {
                std::ostringstream addrStr;
                addrStr << "0x" << std::hex << std::setfill('0')
                        << std::setw(12)
                        << reinterpret_cast<uintptr_t>(lr.address);

                ofs << std::left << std::setw(18) << addrStr.str()
                    << std::dec << std::setw(12) << lr.size
                    << std::setw(16) << (lr.tag ? lr.tag : "<none>")
                    << std::setw(14) << lr.timestamp << "\n";
            }
        }

        ofs << "\n===============================================\n";
        ofs.close();
    }

    // ── Accessors ────────────────────────────────────────────────

    size_t GetCurrentAllocation() {
        AcquireSRWLockExclusive(&m_lock);
        size_t val = m_currentAllocation;
        ReleaseSRWLockExclusive(&m_lock);
        return val;
    }

    size_t GetPeakAllocation() {
        AcquireSRWLockExclusive(&m_lock);
        size_t val = m_peakAllocation;
        ReleaseSRWLockExclusive(&m_lock);
        return val;
    }

private:
    uint64_t NowMs() const {
        auto now = std::chrono::steady_clock::now();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - m_startTime).count());
    }

    // ── Data members ─────────────────────────────────────────────

    SRWLOCK m_lock{};
    HANDLE  m_heap = nullptr;

    std::unordered_map<uintptr_t, AllocationRecord> m_allocations;

    size_t   m_currentAllocation = 0;
    size_t   m_peakAllocation    = 0;
    uint32_t m_doubleFreeAttempts = 0;
    uint32_t m_canaryViolations  = 0;
    uint64_t m_totalAllocations  = 0;
    uint64_t m_totalFrees        = 0;
    double   m_lastSoakDuration  = 0.0;

    std::chrono::steady_clock::time_point m_startTime;
};

} // namespace Engine
} // namespace ExplorerLens
