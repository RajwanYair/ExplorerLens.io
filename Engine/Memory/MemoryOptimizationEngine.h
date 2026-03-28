//==============================================================================
// ExplorerLens Engine — Memory Optimization Engine
// Execution Optimization — Minimal Memory Footprint
// Copyright (c) 2026 — ExplorerLens Project
//
// PURPOSE:
// Holistic memory management layer that works alongside the modular codec
// system to achieve the absolute minimum memory footprint. Covers:
//
// 1. Working Set Trimming — proactively returns unused pages to the OS
// 2. Allocation Tracking — per-subsystem memory accounting
// 3. Thumbnail Pool — reuse HBITMAP buffers instead of alloc/free cycles
// 4. Decode Buffer Recycling — reuse pixel buffers across decode calls
// 5. Memory-Mapped I/O — avoid doubling file data in heap
// 6. System Memory Monitoring — react to low-memory conditions
//
// DESIGN GOAL:
// A folder of 1000 JPEGs at 256x256 should use < 15 MB total working set
// (WIC in-process, thumbnail cache, shell structures).
// No codec DLL should be loaded unless that format is actually present.
//==============================================================================

#pragma once

#include <windows.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <functional>

#pragma comment(lib, "psapi.lib")

namespace ExplorerLens {
namespace Engine {
namespace Memory {

//==============================================================================
// Memory Budget Configuration
//==============================================================================
struct MemoryBudgetConfig
{
    /// Maximum working set target (bytes). Engine will trim if exceeded.
    /// Default: 64 MB — enough for WIC + 2-3 codec DLLs + thumbnail cache
    uint64_t maxWorkingSetBytes = 64ULL * 1024 * 1024;

    /// Soft limit before warning (80% of max by default)
    uint64_t softLimitBytes = 51ULL * 1024 * 1024;

    /// Thumbnail bitmap pool size (number of pre-allocated 256x256 bitmaps)
    uint32_t bitmapPoolSize = 32;

    /// Decode buffer pool size (reusable pixel buffers)
    uint32_t decodeBufferPoolSize = 8;

    /// Default decode buffer size (256x256x4 = 256 KB)
    uint32_t decodeBufferSizeBytes = 256 * 256 * 4;

    /// Working set trim interval (ms) — how often to check and trim
    uint64_t trimIntervalMs = 10000; // 10 seconds

    /// Trim aggressiveness: 0.0 = gentle, 1.0 = aggressive
    double trimAggressiveness = 0.5;

    /// Enable working set size limits via SetProcessWorkingSetSizeEx
    bool enforceWorkingSetLimits = true;

    /// Enable memory-mapped file I/O for supported formats
    bool useMemoryMappedIO = true;

    /// Maximum file size for memory-mapped I/O (larger files use read-buffer)
    uint64_t maxMmapFileSizeBytes = 256ULL * 1024 * 1024; // 256 MB

    /// Enable zero-copy thumbnail delivery where possible
    bool enableZeroCopy = true;
};

//==============================================================================
// Per-Subsystem Memory Accounting
//==============================================================================
enum class MemorySubsystem : uint8_t
{
    Core, ///< Engine core structures
    DecoderWIC, ///< In-process WIC decoders
    DecoderCodec, ///< Modular codec DLLs (total)
    ThumbnailCache, ///< Cache database + bitmap cache
    FileIO, ///< File buffers and memory-mapped regions
    MemOptBitmapPool, ///< Reusable HBITMAP pool
    DecodeBuffers, ///< Reusable pixel buffer pool
    Metadata, ///< EXIF/XMP metadata
    GPU, ///< GPU-side resources
    Other, ///< Uncategorized

    COUNT
};

inline const char* SubsystemName(MemorySubsystem s) {
    static const char* names[] = {
    "Core", "DecoderWIC", "DecoderCodec", "ThumbnailCache",
    "FileIO", "MemOptBitmapPool", "DecodeBuffers", "Metadata", "GPU", "Other"
    };
    auto idx = static_cast<int>(s);
    return (idx < static_cast<int>(MemorySubsystem::COUNT)) ? names[idx] : "?";
}

struct SubsystemMemory
{
    std::atomic<int64_t> currentBytes{ 0 };
    std::atomic<int64_t> peakBytes{ 0 };
    std::atomic<uint64_t> allocCount{ 0 };
    std::atomic<uint64_t> freeCount{ 0 };

    void TrackAlloc(int64_t bytes) {
        int64_t current = currentBytes.fetch_add(bytes, std::memory_order_relaxed) + bytes;
        allocCount.fetch_add(1, std::memory_order_relaxed);

        // Update peak (lock-free)
        int64_t peak = peakBytes.load(std::memory_order_relaxed);
        while (current > peak) {
            if (peakBytes.compare_exchange_weak(peak, current, std::memory_order_relaxed))
                break;
        }
    }

    void TrackFree(int64_t bytes) {
        currentBytes.fetch_sub(bytes, std::memory_order_relaxed);
        freeCount.fetch_add(1, std::memory_order_relaxed);
    }
};

//==============================================================================
// Reusable Bitmap Pool — avoids GDI CreateDIBSection / DeleteObject churn
//==============================================================================
struct PooledBitmap
{
    HBITMAP hBitmap = nullptr;
    void* bits = nullptr; ///< Direct pointer to DIB bits
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t stride = 0;
    bool inUse = false;
};

class MemOptBitmapPool
{
public:
    explicit MemOptBitmapPool(uint32_t defaultWidth = 256, uint32_t defaultHeight = 256,
        uint32_t poolSize = 32)
        : m_defaultWidth(defaultWidth)
        , m_defaultHeight(defaultHeight)
        , m_poolSize(poolSize) {
    }

    ~MemOptBitmapPool() {
        Clear();
    }

    /// Pre-allocate pool bitmaps
    void Initialize() {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (uint32_t i = 0; i < m_poolSize; i++) {
            auto bitmap = AllocateBitmap(m_defaultWidth, m_defaultHeight);
            if (bitmap.hBitmap) {
                m_pool.push_back(std::move(bitmap));
            }
        }
    }

    /// Acquire a bitmap from the pool (or allocate new if pool empty)
    PooledBitmap Acquire(uint32_t width, uint32_t height) {
        std::lock_guard<std::mutex> lk(m_mutex);

        // Try to find a matching-size bitmap in pool
        for (auto& b : m_pool) {
            if (!b.inUse && b.width == width && b.height == height) {
                b.inUse = true;
                m_activeCount++;
                return b;
            }
        }

        // Try to find a larger bitmap and reuse it
        for (auto& b : m_pool) {
            if (!b.inUse && b.width >= width && b.height >= height) {
                b.inUse = true;
                m_activeCount++;
                return b;
            }
        }

        // Pool exhausted — allocate new (will not be returned to pool)
        m_overflowCount++;
        auto bitmap = AllocateBitmap(width, height);
        bitmap.inUse = true;
        return bitmap;
    }

    /// Return a bitmap to the pool
    void Release(const PooledBitmap& bitmap) {
        std::lock_guard<std::mutex> lk(m_mutex);

        for (auto& b : m_pool) {
            if (b.hBitmap == bitmap.hBitmap) {
                b.inUse = false;
                if (m_activeCount > 0) m_activeCount--;
                return;
            }
        }

        // Not from pool — free it
        if (bitmap.hBitmap) {
            ::DeleteObject(bitmap.hBitmap);
        }
    }

    /// Clear all pool bitmaps
    void Clear() {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& b : m_pool) {
            if (b.hBitmap) {
                ::DeleteObject(b.hBitmap);
                b.hBitmap = nullptr;
            }
        }
        m_pool.clear();
        m_activeCount = 0;
    }

    uint32_t GetPoolSize() const { return static_cast<uint32_t>(m_pool.size()); }
    uint32_t GetActiveCount() const { return m_activeCount; }
    uint32_t GetOverflowCount() const { return m_overflowCount; }

    uint64_t GetPoolMemoryBytes() const {
        uint64_t total = 0;
        for (auto& b : m_pool) {
            total += static_cast<uint64_t>(b.stride) * b.height;
        }
        return total;
    }

private:
    static PooledBitmap AllocateBitmap(uint32_t width, uint32_t height) {
        PooledBitmap result;
        result.width = width;
        result.height = height;
        result.stride = width * 4;

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = static_cast<LONG>(width);
        bmi.bmiHeader.biHeight = -static_cast<LONG>(height);
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        HDC hdc = ::GetDC(nullptr);
        result.hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS,
            &result.bits, nullptr, 0);
        ::ReleaseDC(nullptr, hdc);
        return result;
    }

    std::mutex m_mutex;
    std::vector<PooledBitmap> m_pool;
    uint32_t m_defaultWidth;
    uint32_t m_defaultHeight;
    uint32_t m_poolSize;
    uint32_t m_activeCount = 0;
    uint32_t m_overflowCount = 0;
};

//==============================================================================
// Decode Buffer Pool — reuse raw pixel buffers to avoid heap fragmentation
//==============================================================================
class DecodeBufferPool
{
public:
    explicit DecodeBufferPool(uint32_t bufferSize = 256 * 256 * 4,
        uint32_t poolSize = 8)
        : m_bufferSize(bufferSize)
        , m_poolSize(poolSize) {
    }

    ~DecodeBufferPool() {
        Clear();
    }

    void Initialize() {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (uint32_t i = 0; i < m_poolSize; i++) {
            auto buf = std::make_unique<uint8_t[]>(m_bufferSize);
            m_available.push(buf.get());
            m_owned.push_back(std::move(buf));
        }
    }

    /// Acquire a buffer (returns nullptr if pool exhausted)
    uint8_t* Acquire(uint32_t requiredSize) {
        std::lock_guard<std::mutex> lk(m_mutex);

        if (requiredSize <= m_bufferSize && !m_available.empty()) {
            uint8_t* buf = m_available.front();
            m_available.pop();
            m_activeCount++;
            return buf;
        }

        // Pool can't satisfy — return nullptr (caller allocs from heap)
        return nullptr;
    }

    /// Release a buffer back to the pool
    void Release(uint8_t* buffer) {
        if (!buffer) return;
        std::lock_guard<std::mutex> lk(m_mutex);

        // Only accept back buffers we own
        for (auto& owned : m_owned) {
            if (owned.get() == buffer) {
                m_available.push(buffer);
                if (m_activeCount > 0) m_activeCount--;
                return;
            }
        }
        // Not ours — caller must free it
    }

    void Clear() {
        std::lock_guard<std::mutex> lk(m_mutex);
        while (!m_available.empty()) m_available.pop();
        m_owned.clear();
        m_activeCount = 0;
    }

    uint32_t GetBufferSize() const { return m_bufferSize; }
    uint32_t GetAvailableCount() const { return static_cast<uint32_t>(m_available.size()); }
    uint32_t GetActiveCount() const { return m_activeCount; }
    uint64_t GetPoolMemoryBytes() const {
        return static_cast<uint64_t>(m_owned.size()) * m_bufferSize;
    }

private:
    std::mutex m_mutex;
    std::queue<uint8_t*> m_available;
    std::vector<std::unique_ptr<uint8_t[]>> m_owned;
    uint32_t m_bufferSize;
    uint32_t m_poolSize;
    uint32_t m_activeCount = 0;
};

//==============================================================================
// Memory-Mapped File Handle — RAII wrapper for MapViewOfFile
//==============================================================================
class MemOptMappedFile
{
public:
    MemOptMappedFile() = default;

    ~MemOptMappedFile() {
        Close();
    }

    // Non-copyable, movable
    MemOptMappedFile(const MemOptMappedFile&) = delete;
    MemOptMappedFile& operator=(const MemOptMappedFile&) = delete;
    MemOptMappedFile(MemOptMappedFile&& other) noexcept {
        m_view = other.m_view; other.m_view = nullptr;
        m_mapping = other.m_mapping; other.m_mapping = nullptr;
        m_file = other.m_file; other.m_file = INVALID_HANDLE_VALUE;
        m_size = other.m_size; other.m_size = 0;
    }

    /// Open a file as memory-mapped read-only
    bool Open(const wchar_t* filePath) {
        Close();

        m_file = ::CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
            nullptr);
        if (m_file == INVALID_HANDLE_VALUE) return false;

        LARGE_INTEGER fileSize;
        if (!::GetFileSizeEx(m_file, &fileSize)) {
            Close();
            return false;
        }
        m_size = static_cast<uint64_t>(fileSize.QuadPart);

        m_mapping = ::CreateFileMappingW(m_file, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!m_mapping) {
            Close();
            return false;
        }

        m_view = ::MapViewOfFile(m_mapping, FILE_MAP_READ, 0, 0, 0);
        if (!m_view) {
            Close();
            return false;
        }

        return true;
    }

    void Close() {
        if (m_view) { ::UnmapViewOfFile(m_view); m_view = nullptr; }
        if (m_mapping) { ::CloseHandle(m_mapping); m_mapping = nullptr; }
        if (m_file != INVALID_HANDLE_VALUE) { ::CloseHandle(m_file); m_file = INVALID_HANDLE_VALUE; }
        m_size = 0;
    }

    const void* Data() const { return m_view; }
    uint64_t Size() const { return m_size; }
    bool IsOpen() const { return m_view != nullptr; }

    /// Advise the OS about sequential access pattern
    void AdviseSequential() {
        if (m_view && m_size > 0) {
            // Prefetch the first 64KB for header parsing
            WIN32_MEMORY_RANGE_ENTRY range;
            range.VirtualAddress = m_view;
            range.NumberOfBytes = static_cast<SIZE_T>((std::min)(m_size, 65536ULL));
            ::PrefetchVirtualMemory(::GetCurrentProcess(), 1, &range, 0);
        }
    }

private:
    HANDLE m_file = INVALID_HANDLE_VALUE;
    HANDLE m_mapping = nullptr;
    void* m_view = nullptr;
    uint64_t m_size = 0;
};

//==============================================================================
// Working Set Monitor — tracks and trims process working set
//==============================================================================
struct WorkingSetSnapshot
{
    uint64_t workingSetBytes = 0;
    uint64_t privateBytes = 0;
    uint64_t peakWorkingSetBytes = 0;
    uint64_t pagedPoolBytes = 0;
    uint64_t nonPagedPoolBytes = 0;
    uint64_t pagefileUsageBytes = 0;
    double workingSetMB = 0.0;
    double privateMB = 0.0;

    bool ExceedsBudget(uint64_t budgetBytes) const {
        return workingSetBytes > budgetBytes;
    }
};

class WorkingSetMonitor
{
public:
    /// Take a snapshot of the current process memory counters
    static WorkingSetSnapshot GetCurrentSnapshot() {
        WorkingSetSnapshot snap;
        PROCESS_MEMORY_COUNTERS_EX pmc{};
        pmc.cb = sizeof(pmc);

        if (::GetProcessMemoryInfo(::GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
            sizeof(pmc))) {
            snap.workingSetBytes = pmc.WorkingSetSize;
            snap.privateBytes = pmc.PrivateUsage;
            snap.peakWorkingSetBytes = pmc.PeakWorkingSetSize;
            snap.pagedPoolBytes = pmc.QuotaPagedPoolUsage;
            snap.nonPagedPoolBytes = pmc.QuotaNonPagedPoolUsage;
            snap.pagefileUsageBytes = pmc.PagefileUsage;
            snap.workingSetMB = snap.workingSetBytes / (1024.0 * 1024.0);
            snap.privateMB = snap.privateBytes / (1024.0 * 1024.0);
        }

        return snap;
    }

    /// Attempt to trim the working set to the specified target
    static bool TrimWorkingSet(uint64_t targetBytes) {
        // Set minimum and maximum working set sizes
        SIZE_T minWS = 1024 * 1024; // 1 MB minimum
        SIZE_T maxWS = static_cast<SIZE_T>(targetBytes);

        return ::SetProcessWorkingSetSizeEx(
            ::GetCurrentProcess(), minWS, maxWS,
            QUOTA_LIMITS_HARDWS_MIN_DISABLE | QUOTA_LIMITS_HARDWS_MAX_ENABLE) != 0;
    }

    /// Empty the working set entirely (returns pages to standby list)
    /// Use sparingly — causes page faults on next access
    static bool EmptyWorkingSet() {
        return ::EmptyWorkingSet(::GetCurrentProcess()) != 0;
    }

    /// Gentle trim: only if over budget, trim to 90% of budget
    static bool GentleTrim(uint64_t budgetBytes) {
        auto snap = GetCurrentSnapshot();
        if (!snap.ExceedsBudget(budgetBytes)) return false;

        uint64_t target = static_cast<uint64_t>(budgetBytes * 0.9);
        return TrimWorkingSet(target);
    }
};

//==============================================================================
// Memory Optimization Engine — Unified Manager
//==============================================================================
class MemoryOptimizationEngine
{
public:
    explicit MemoryOptimizationEngine(const MemoryBudgetConfig& config = {})
        : m_config(config)
        , m_bitmapPool(256, 256, config.bitmapPoolSize)
        , m_decodeBufferPool(config.decodeBufferSizeBytes,
            config.decodeBufferPoolSize) {
        // Initialize subsystem accounting
        for (int i = 0; i < static_cast<int>(MemorySubsystem::COUNT); i++) {
            (void)m_subsystems[static_cast<MemorySubsystem>(i)]; // default-construct entries
        }
    }

    ~MemoryOptimizationEngine() {
        Shutdown();
    }

    /// Initialize pools and start monitoring
    void Initialize() {
        m_bitmapPool.Initialize();
        m_decodeBufferPool.Initialize();

        // Record pool memory in accounting
        TrackAlloc(MemorySubsystem::MemOptBitmapPool,
            static_cast<int64_t>(m_bitmapPool.GetPoolMemoryBytes()));
        TrackAlloc(MemorySubsystem::DecodeBuffers,
            static_cast<int64_t>(m_decodeBufferPool.GetPoolMemoryBytes()));

        // Set initial working set limits
        if (m_config.enforceWorkingSetLimits) {
            WorkingSetMonitor::TrimWorkingSet(m_config.maxWorkingSetBytes);
        }

        m_initialized = true;
    }

    void Shutdown() {
        m_bitmapPool.Clear();
        m_decodeBufferPool.Clear();
        m_initialized = false;
    }

    //--------------------------------------------------------------------------
    // Memory Tracking
    //--------------------------------------------------------------------------

    void TrackAlloc(MemorySubsystem subsystem, int64_t bytes) {
        m_subsystems[subsystem].TrackAlloc(bytes);
    }

    void TrackFree(MemorySubsystem subsystem, int64_t bytes) {
        m_subsystems[subsystem].TrackFree(bytes);
    }

    int64_t GetSubsystemMemory(MemorySubsystem subsystem) const {
        auto it = m_subsystems.find(subsystem);
        return (it != m_subsystems.end())
            ? it->second.currentBytes.load(std::memory_order_relaxed) : 0;
    }

    int64_t GetTotalTrackedMemory() const {
        int64_t total = 0;
        for (auto& [_, sm] : m_subsystems) {
            total += sm.currentBytes.load(std::memory_order_relaxed);
        }
        return total;
    }

    //--------------------------------------------------------------------------
    // Bitmap Pool Access
    //--------------------------------------------------------------------------

    PooledBitmap AcquireBitmap(uint32_t width, uint32_t height) {
        return m_bitmapPool.Acquire(width, height);
    }

    void ReleaseBitmap(const PooledBitmap& bitmap) {
        m_bitmapPool.Release(bitmap);
    }

    //--------------------------------------------------------------------------
    // Decode Buffer Pool Access
    //--------------------------------------------------------------------------

    uint8_t* AcquireDecodeBuffer(uint32_t size) {
        return m_decodeBufferPool.Acquire(size);
    }

    void ReleaseDecodeBuffer(uint8_t* buffer) {
        m_decodeBufferPool.Release(buffer);
    }

    //--------------------------------------------------------------------------
    // Memory-Mapped File I/O
    //--------------------------------------------------------------------------

    /// Open file as memory-mapped (returns empty if file too large or mmap disabled)
    MemOptMappedFile OpenMapped(const wchar_t* filePath) {
        MemOptMappedFile mmf;

        if (!m_config.useMemoryMappedIO) return mmf;

        // Check file size first
        WIN32_FILE_ATTRIBUTE_DATA attrs;
        if (!::GetFileAttributesExW(filePath, GetFileExInfoStandard, &attrs))
            return mmf;

        uint64_t fileSize = (static_cast<uint64_t>(attrs.nFileSizeHigh) << 32) |
            attrs.nFileSizeLow;
        if (fileSize > m_config.maxMmapFileSizeBytes)
            return mmf;

        if (mmf.Open(filePath)) {
            mmf.AdviseSequential();
            TrackAlloc(MemorySubsystem::FileIO, static_cast<int64_t>(mmf.Size()));
        }

        return mmf;
    }

    //--------------------------------------------------------------------------
    // Working Set Management
    //--------------------------------------------------------------------------

    /// Check if process memory exceeds budget and trim if necessary
    bool CheckAndTrimWorkingSet() {
        if (!m_config.enforceWorkingSetLimits) return false;

        auto snap = WorkingSetMonitor::GetCurrentSnapshot();
        m_lastSnapshot = snap;
        m_checkCount++;

        if (snap.ExceedsBudget(m_config.maxWorkingSetBytes)) {
            m_trimCount++;
            return WorkingSetMonitor::GentleTrim(m_config.maxWorkingSetBytes);
        }

        return false;
    }

    WorkingSetSnapshot GetLastSnapshot() const { return m_lastSnapshot; }

    //--------------------------------------------------------------------------
    // Diagnostics
    //--------------------------------------------------------------------------

    /// Get a summary of memory usage across all subsystems
    std::string GetMemoryReport() const {
        auto snap = WorkingSetMonitor::GetCurrentSnapshot();
        std::string report;
        report += "=== ExplorerLens Memory Report ===\n";
        report += "Process Working Set: " +
            std::to_string(static_cast<int>(snap.workingSetMB)) + " MB\n";
        report += "Private Bytes: " +
            std::to_string(static_cast<int>(snap.privateMB)) + " MB\n";
        report += "Peak Working Set: " +
            std::to_string(snap.peakWorkingSetBytes / (1024 * 1024)) + " MB\n";
        report += "Budget: " +
            std::to_string(m_config.maxWorkingSetBytes / (1024 * 1024)) + " MB\n";
        report += "\nPer-Subsystem:\n";

        for (int i = 0; i < static_cast<int>(MemorySubsystem::COUNT); i++) {
            auto sub = static_cast<MemorySubsystem>(i);
            auto it = m_subsystems.find(sub);
            if (it != m_subsystems.end()) {
                int64_t bytes = it->second.currentBytes.load(std::memory_order_relaxed);
                report += " " + std::string(SubsystemName(sub)) + ": " +
                    std::to_string(bytes / 1024) + " KB\n";
            }
        }

        report += "\nPools:\n";
        report += " Bitmap Pool: " + std::to_string(m_bitmapPool.GetPoolSize()) +
            " slots, " + std::to_string(m_bitmapPool.GetActiveCount()) + " active\n";
        report += " Decode Buffers: " +
            std::to_string(m_decodeBufferPool.GetAvailableCount()) +
            " available, " + std::to_string(m_decodeBufferPool.GetActiveCount()) +
            " active\n";

        report += "\nWS Checks: " + std::to_string(m_checkCount) +
            ", Trims: " + std::to_string(m_trimCount) + "\n";

        return report;
    }

    /// Check if we're within budget
    bool IsWithinBudget() const {
        auto snap = WorkingSetMonitor::GetCurrentSnapshot();
        return !snap.ExceedsBudget(m_config.maxWorkingSetBytes);
    }

    /// Check if we're near the soft limit (warning zone)
    bool IsNearSoftLimit() const {
        auto snap = WorkingSetMonitor::GetCurrentSnapshot();
        return snap.workingSetBytes > m_config.softLimitBytes &&
            snap.workingSetBytes <= m_config.maxWorkingSetBytes;
    }

    const MemoryBudgetConfig& GetConfig() const { return m_config; }

private:
    MemoryBudgetConfig m_config;
    MemOptBitmapPool m_bitmapPool;
    DecodeBufferPool m_decodeBufferPool;
    bool m_initialized = false;

    mutable std::unordered_map<MemorySubsystem, SubsystemMemory> m_subsystems;

    WorkingSetSnapshot m_lastSnapshot;
    uint64_t m_checkCount = 0;
    uint64_t m_trimCount = 0;
};

} // namespace Memory
} // namespace Engine
} // namespace ExplorerLens
