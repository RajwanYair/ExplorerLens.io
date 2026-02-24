//==============================================================================
// ExplorerLens — SEH Fuzzing Engine & Worker Isolation
//
// Structured Exception Handling (SEH) fuzzing infrastructure to validate
// that LENSShell survives malformed / corrupt payloads without crashing
// Explorer. Wraps every decoder call in __try/__except, generates diverse
// corrupt test payloads, and records results per-iteration.
//==============================================================================

#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <random>
#include <chrono>
#include <functional>
#include <atomic>
#include <mutex>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {
namespace Isolation {

//==============================================================================
// Corrupt Payload Generation Strategy
//==============================================================================
enum class CorruptionStrategy : uint32_t {
    ZeroFill         = 0,   // All zero payload
    RandomBytes      = 1,   // Pure random data
    ValidHeaderGarbage = 2, // Valid magic bytes + random tail
    Truncated        = 3,   // Valid header truncated at random offset
    BitFlip          = 4,   // Valid file with random bit flips
    OversizeField    = 5,   // Inject oversized dimension/length fields
    NullTerminated   = 6,   // Fill with 0x00 at specific offsets
    RepeatingPattern = 7,   // Repeating 4-byte pattern
    MaxEnum          = 8
};

inline const char* CorruptionStrategyName(CorruptionStrategy s) {
    static const char* names[] = {
        "ZeroFill", "RandomBytes", "ValidHeaderGarbage", "Truncated",
        "BitFlip", "OversizeField", "NullTerminated", "RepeatingPattern"
    };
    auto idx = static_cast<uint32_t>(s);
    return idx < static_cast<uint32_t>(CorruptionStrategy::MaxEnum)
        ? names[idx] : "Unknown";
}

//==============================================================================
// File Format Magic Bytes (for ValidHeaderGarbage strategy)
//==============================================================================
struct FormatMagic {
    const char* extension;
    std::vector<uint8_t> magic;
    size_t minSize;
};

inline std::vector<FormatMagic> GetKnownMagicBytes() {
    return {
        { ".zip",  {0x50, 0x4B, 0x03, 0x04}, 30 },
        { ".rar",  {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07}, 20 },
        { ".7z",   {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C}, 32 },
        { ".png",  {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}, 67 },
        { ".jpg",  {0xFF, 0xD8, 0xFF, 0xE0}, 20 },
        { ".gif",  {0x47, 0x49, 0x46, 0x38, 0x39, 0x61}, 13 },
        { ".bmp",  {0x42, 0x4D}, 54 },
        { ".webp", {0x52, 0x49, 0x46, 0x46}, 12 },
        { ".tiff", {0x49, 0x49, 0x2A, 0x00}, 8 },
        { ".psd",  {0x38, 0x42, 0x50, 0x53}, 26 },
        { ".pdf",  {0x25, 0x50, 0x44, 0x46}, 15 },
        { ".heif", {0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70}, 12 },
        { ".jxl",  {0xFF, 0x0A}, 4 },
        { ".avif", {0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70}, 12 },
        { ".exr",  {0x76, 0x2F, 0x31, 0x01}, 8 },
        { ".dds",  {0x44, 0x44, 0x53, 0x20}, 128 },
        { ".ico",  {0x00, 0x00, 0x01, 0x00}, 22 },
        { ".svg",  {0x3C, 0x73, 0x76, 0x67}, 10 },
        { ".qoi",  {0x71, 0x6F, 0x69, 0x66}, 14 },
        { ".tga",  {0x00, 0x00, 0x02, 0x00}, 18 },
    };
}

//==============================================================================
// Fuzz Iteration Result
//==============================================================================
struct FuzzResult {
    uint64_t          iterationId       = 0;
    CorruptionStrategy strategy         = CorruptionStrategy::ZeroFill;
    std::string       targetFormat;
    size_t            payloadSize       = 0;
    bool              sehCaught         = false;   // __except fired
    bool              crashed           = false;   // Should always be false
    bool              gracefulReject    = false;   // Decoder returned error cleanly
    bool              circuitBroken     = false;   // Circuit breaker tripped
    DWORD             exceptionCode     = 0;       // If SEH fired
    double            elapsedMs         = 0.0;

    bool IsPass() const { return !crashed; }
};

//==============================================================================
// Fuzz Campaign Summary
//==============================================================================
struct FuzzCampaignReport {
    uint64_t totalIterations   = 0;
    uint64_t sehExceptions     = 0;
    uint64_t gracefulRejects   = 0;
    uint64_t circuitBreaks     = 0;
    uint64_t crashes           = 0;    // Must be 0 to pass
    uint64_t timeouts          = 0;
    double   totalElapsedMs    = 0.0;
    double   avgIterationMs    = 0.0;
    std::vector<FuzzResult> failures;  // Only failed iterations

    bool     OverallPass() const { return crashes == 0; }

    double   SEHRate() const {
        return totalIterations > 0
            ? 100.0 * sehExceptions / totalIterations : 0.0;
    }

    double   GracefulRate() const {
        return totalIterations > 0
            ? 100.0 * gracefulRejects / totalIterations : 0.0;
    }
};

//==============================================================================
// Corrupt Payload Generator
//==============================================================================
class CorruptPayloadGenerator {
public:
    explicit CorruptPayloadGenerator(uint64_t seed = 0)
        : m_rng(seed ? seed : std::random_device{}())
    {}

    std::vector<uint8_t> Generate(CorruptionStrategy strategy,
                                   const std::string& extension,
                                   size_t targetSize)
    {
        switch (strategy) {
        case CorruptionStrategy::ZeroFill:
            return GenerateZeroFill(targetSize);
        case CorruptionStrategy::RandomBytes:
            return GenerateRandom(targetSize);
        case CorruptionStrategy::ValidHeaderGarbage:
            return GenerateValidHeaderGarbage(extension, targetSize);
        case CorruptionStrategy::Truncated:
            return GenerateTruncated(extension, targetSize);
        case CorruptionStrategy::BitFlip:
            return GenerateBitFlip(extension, targetSize);
        case CorruptionStrategy::OversizeField:
            return GenerateOversizeField(extension, targetSize);
        case CorruptionStrategy::NullTerminated:
            return GenerateNullTerminated(targetSize);
        case CorruptionStrategy::RepeatingPattern:
            return GenerateRepeatingPattern(targetSize);
        default:
            return GenerateRandom(targetSize);
        }
    }

    CorruptionStrategy RandomStrategy() {
        auto max = static_cast<uint32_t>(CorruptionStrategy::MaxEnum);
        std::uniform_int_distribution<uint32_t> dist(0, max - 1);
        return static_cast<CorruptionStrategy>(dist(m_rng));
    }

    size_t RandomPayloadSize(size_t minSize = 4, size_t maxSize = 65536) {
        std::uniform_int_distribution<size_t> dist(minSize, maxSize);
        return dist(m_rng);
    }

private:
    std::mt19937_64 m_rng;

    std::vector<uint8_t> GenerateZeroFill(size_t sz) {
        return std::vector<uint8_t>(sz, 0x00);
    }

    std::vector<uint8_t> GenerateRandom(size_t sz) {
        std::vector<uint8_t> buf(sz);
        std::uniform_int_distribution<int> dist(0, 255);
        for (auto& b : buf) b = static_cast<uint8_t>(dist(m_rng));
        return buf;
    }

    std::vector<uint8_t> GenerateValidHeaderGarbage(const std::string& ext, size_t sz) {
        auto magics = GetKnownMagicBytes();
        std::vector<uint8_t> buf = GenerateRandom(sz);
        for (auto& m : magics) {
            if (ext == m.extension && !m.magic.empty()) {
                size_t copyLen = (std::min)(m.magic.size(), sz);
                std::copy_n(m.magic.begin(), copyLen, buf.begin());
                break;
            }
        }
        return buf;
    }

    std::vector<uint8_t> GenerateTruncated(const std::string& ext, size_t sz) {
        auto buf = GenerateValidHeaderGarbage(ext, sz);
        std::uniform_int_distribution<size_t> dist(1, (std::max)(sz / 2, size_t(1)));
        size_t truncAt = dist(m_rng);
        buf.resize(truncAt);
        return buf;
    }

    std::vector<uint8_t> GenerateBitFlip(const std::string& ext, size_t sz) {
        auto buf = GenerateValidHeaderGarbage(ext, sz);
        size_t flips = (std::max)(sz / 32, size_t(1));
        std::uniform_int_distribution<size_t> posDist(0, buf.size() - 1);
        std::uniform_int_distribution<int> bitDist(0, 7);
        for (size_t i = 0; i < flips; ++i) {
            size_t pos = posDist(m_rng);
            buf[pos] ^= (1u << bitDist(m_rng));
        }
        return buf;
    }

    std::vector<uint8_t> GenerateOversizeField(const std::string& ext, size_t sz) {
        auto buf = GenerateValidHeaderGarbage(ext, sz);
        // Inject 0xFFFFFFFF at 4-byte-aligned offsets to trigger dimension overflows
        if (buf.size() >= 8) {
            std::uniform_int_distribution<size_t> posDist(4, buf.size() - 4);
            size_t pos = posDist(m_rng) & ~size_t(3);
            buf[pos + 0] = 0xFF; buf[pos + 1] = 0xFF;
            buf[pos + 2] = 0xFF; buf[pos + 3] = 0xFF;
        }
        return buf;
    }

    std::vector<uint8_t> GenerateNullTerminated(size_t sz) {
        std::vector<uint8_t> buf(sz, 0x41); // Fill with 'A'
        std::uniform_int_distribution<size_t> dist(0, sz - 1);
        for (size_t i = 0; i < sz / 4; ++i) {
            buf[dist(m_rng)] = 0x00;
        }
        return buf;
    }

    std::vector<uint8_t> GenerateRepeatingPattern(size_t sz) {
        std::vector<uint8_t> buf(sz);
        uint32_t pattern = 0xDEADBEEF;
        for (size_t i = 0; i + 3 < sz; i += 4) {
            std::memcpy(&buf[i], &pattern, 4);
        }
        return buf;
    }
};

//==============================================================================
// SEH Isolation Wrapper
//==============================================================================
// Wraps a callable in __try/__except to prevent decoder crashes from
// propagating to the Explorer host process.
//==============================================================================
class SEHIsolationWrapper {
public:
    struct ExecutionResult {
        bool    completed     = false;
        bool    sehCaught     = false;
        DWORD   exceptionCode = 0;
        double  elapsedMs     = 0.0;
    };

    // Execute a callable within SEH protection
    // The callable should return bool (true = success, false = error)
    static ExecutionResult Execute(std::function<bool()> callable,
                                    uint32_t timeoutMs = 5000)
    {
        ExecutionResult result{};
        auto start = std::chrono::steady_clock::now();

#ifdef _MSC_VER
        __try {
            result.completed = callable();
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            result.sehCaught = true;
            result.exceptionCode = GetExceptionCode();
            result.completed = false;
        }
#else
        // Non-MSVC fallback — standard try/catch
        try {
            result.completed = callable();
        } catch (...) {
            result.sehCaught = true;
            result.completed = false;
        }
#endif

        auto end = std::chrono::steady_clock::now();
        result.elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        return result;
    }
};

//==============================================================================
// Worker Process Isolation Monitor
//==============================================================================
// Tracks per-worker health across multiple decode requests.
// If a worker accumulates too many SEH exceptions or timeouts,
// it's flagged for recycling.
//==============================================================================
class WorkerIsolationMonitor {
public:
    struct WorkerHealth {
        uint32_t workerId         = 0;
        uint64_t totalRequests    = 0;
        uint64_t sehExceptions    = 0;
        uint64_t timeouts         = 0;
        uint64_t successfulDecodes = 0;
        bool     markedForRecycle = false;

        double FailureRate() const {
            if (totalRequests == 0) return 0.0;
            return 100.0 * (sehExceptions + timeouts) / totalRequests;
        }
    };

    explicit WorkerIsolationMonitor(uint32_t maxFailuresBeforeRecycle = 10,
                                      double maxFailureRatePercent = 5.0)
        : m_maxFailures(maxFailuresBeforeRecycle)
        , m_maxFailureRate(maxFailureRatePercent)
    {}

    void RegisterWorker(uint32_t workerId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_workers.find(workerId) == m_workers.end()) {
            m_workers[workerId] = WorkerHealth{ workerId };
        }
    }

    void RecordSuccess(uint32_t workerId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_workers.find(workerId);
        if (it != m_workers.end()) {
            it->second.totalRequests++;
            it->second.successfulDecodes++;
        }
    }

    void RecordSEHException(uint32_t workerId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_workers.find(workerId);
        if (it != m_workers.end()) {
            it->second.totalRequests++;
            it->second.sehExceptions++;
            CheckRecycleThreshold(it->second);
        }
    }

    void RecordTimeout(uint32_t workerId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_workers.find(workerId);
        if (it != m_workers.end()) {
            it->second.totalRequests++;
            it->second.timeouts++;
            CheckRecycleThreshold(it->second);
        }
    }

    bool ShouldRecycle(uint32_t workerId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_workers.find(workerId);
        return it != m_workers.end() && it->second.markedForRecycle;
    }

    std::vector<WorkerHealth> GetAllWorkers() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<WorkerHealth> result;
        result.reserve(m_workers.size());
        for (auto& [id, health] : m_workers) result.push_back(health);
        return result;
    }

    size_t WorkerCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_workers.size();
    }

    size_t RecycleCandidateCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t count = 0;
        for (auto& [id, h] : m_workers)
            if (h.markedForRecycle) ++count;
        return count;
    }

private:
    void CheckRecycleThreshold(WorkerHealth& w) {
        uint64_t failures = w.sehExceptions + w.timeouts;
        if (failures >= m_maxFailures || w.FailureRate() > m_maxFailureRate) {
            w.markedForRecycle = true;
        }
    }

    mutable std::mutex m_mutex;
    std::unordered_map<uint32_t, WorkerHealth> m_workers;
    uint32_t m_maxFailures;
    double   m_maxFailureRate;
};

//==============================================================================
// Fuzz Campaign Runner
//==============================================================================
// Orchestrates a full fuzzing campaign: generates corrupt payloads for
// every supported format, runs them through SEH-wrapped decode, and
// produces a campaign report.
//==============================================================================
class FuzzCampaignRunner {
public:
    using DecoderCallback = std::function<bool(const uint8_t* data, size_t size,
                                                const std::string& extension)>;

    explicit FuzzCampaignRunner(uint64_t seed = 42)
        : m_generator(seed)
    {}

    // Attach a decode callback that simulates the thumbnail pipeline
    void SetDecoderCallback(DecoderCallback cb) { m_decoder = std::move(cb); }

    // Run fuzzing campaign with specified iterations per format
    FuzzCampaignReport Run(uint64_t iterationsPerFormat,
                            const std::vector<std::string>& formats)
    {
        FuzzCampaignReport report{};
        auto campaignStart = std::chrono::steady_clock::now();
        uint64_t iterationId = 0;

        for (const auto& ext : formats) {
            for (uint64_t i = 0; i < iterationsPerFormat; ++i) {
                FuzzResult res = RunSingleIteration(iterationId++, ext);
                AccumulateResult(report, res);
            }
        }

        auto campaignEnd = std::chrono::steady_clock::now();
        report.totalElapsedMs = std::chrono::duration<double, std::milli>(
            campaignEnd - campaignStart).count();
        report.avgIterationMs = report.totalIterations > 0
            ? report.totalElapsedMs / report.totalIterations : 0.0;

        return report;
    }

    // Quick smoke test: N random iterations, random formats
    FuzzCampaignReport QuickSmoke(uint64_t totalIterations)
    {
        auto magics = GetKnownMagicBytes();
        std::vector<std::string> formats;
        for (auto& m : magics) formats.push_back(m.extension);

        FuzzCampaignReport report{};
        auto start = std::chrono::steady_clock::now();

        for (uint64_t i = 0; i < totalIterations; ++i) {
            std::uniform_int_distribution<size_t> fmtDist(0, formats.size() - 1);
            const auto& ext = formats[fmtDist(m_rng)];
            FuzzResult res = RunSingleIteration(i, ext);
            AccumulateResult(report, res);
        }

        auto end = std::chrono::steady_clock::now();
        report.totalElapsedMs = std::chrono::duration<double, std::milli>(
            end - start).count();
        report.avgIterationMs = report.totalIterations > 0
            ? report.totalElapsedMs / report.totalIterations : 0.0;

        return report;
    }

    size_t SupportedFormatCount() const { return GetKnownMagicBytes().size(); }

private:
    FuzzResult RunSingleIteration(uint64_t id, const std::string& ext) {
        FuzzResult res{};
        res.iterationId = id;
        res.strategy = m_generator.RandomStrategy();
        res.targetFormat = ext;
        res.payloadSize = m_generator.RandomPayloadSize();

        auto payload = m_generator.Generate(res.strategy, ext, res.payloadSize);

        auto sehResult = SEHIsolationWrapper::Execute([&]() {
            return m_decoder
                ? m_decoder(payload.data(), payload.size(), ext)
                : false;
        });

        res.sehCaught = sehResult.sehCaught;
        res.exceptionCode = sehResult.exceptionCode;
        res.elapsedMs = sehResult.elapsedMs;
        res.gracefulReject = !sehResult.sehCaught && !sehResult.completed;
        res.crashed = false; // If we got here, we didn't crash

        return res;
    }

    void AccumulateResult(FuzzCampaignReport& report, const FuzzResult& res) {
        report.totalIterations++;
        if (res.sehCaught) report.sehExceptions++;
        if (res.gracefulReject) report.gracefulRejects++;
        if (res.circuitBroken) report.circuitBreaks++;
        if (res.crashed) {
            report.crashes++;
            report.failures.push_back(res);
        }
    }

    CorruptPayloadGenerator m_generator;
    std::mt19937_64 m_rng{42};
    DecoderCallback m_decoder;
};

//==============================================================================
// Memory Leak Regression Harness
//==============================================================================
// Runs N iterations of decode, measures peak heap size, and asserts
// that memory growth stays within acceptable bounds (no leaks).
//==============================================================================
class MemoryLeakRegressionHarness {
public:
    struct MemorySnapshot {
        SIZE_T workingSetBytes   = 0;
        SIZE_T privateBytes      = 0;
        SIZE_T peakWorkingSet    = 0;
        SIZE_T heapAllocBytes    = 0;
    };

    struct LeakTestResult {
        MemorySnapshot before;
        MemorySnapshot after;
        MemorySnapshot peak;
        uint64_t       iterations         = 0;
        double         workingSetGrowthMB  = 0.0;
        double         privateGrowthMB     = 0.0;
        double         growthPerIterKB     = 0.0;
        bool           passedLeakCheck     = false;

        // Growth threshold: < 1 KB per iteration average = no significant leak
        static constexpr double MAX_GROWTH_PER_ITER_KB = 1.0;
    };

    static MemorySnapshot CaptureSnapshot() {
        MemorySnapshot snap{};
        PROCESS_MEMORY_COUNTERS_EX pmc{};
        pmc.cb = sizeof(pmc);
        if (GetProcessMemoryInfo(GetCurrentProcess(),
                reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
            snap.workingSetBytes = pmc.WorkingSetSize;
            snap.privateBytes = pmc.PrivateUsage;
            snap.peakWorkingSet = pmc.PeakWorkingSetSize;
        }
        return snap;
    }

    static LeakTestResult RunLeakTest(std::function<void()> decodeFunc,
                                       uint64_t iterations = 100)
    {
        LeakTestResult result{};
        result.iterations = iterations;
        result.before = CaptureSnapshot();

        SIZE_T peakWS = result.before.workingSetBytes;
        SIZE_T peakPrivate = result.before.privateBytes;

        for (uint64_t i = 0; i < iterations; ++i) {
            decodeFunc();

            // Sample every 10 iterations
            if (i % 10 == 0) {
                auto snap = CaptureSnapshot();
                peakWS = (std::max)(peakWS, snap.workingSetBytes);
                peakPrivate = (std::max)(peakPrivate, snap.privateBytes);
            }
        }

        result.after = CaptureSnapshot();
        result.peak.workingSetBytes = peakWS;
        result.peak.privateBytes = peakPrivate;
        result.peak.peakWorkingSet = result.after.peakWorkingSet;

        result.workingSetGrowthMB = static_cast<double>(
            result.after.workingSetBytes - result.before.workingSetBytes)
            / (1024.0 * 1024.0);
        result.privateGrowthMB = static_cast<double>(
            result.after.privateBytes - result.before.privateBytes)
            / (1024.0 * 1024.0);
        result.growthPerIterKB = (result.workingSetGrowthMB * 1024.0) / iterations;
        result.passedLeakCheck =
            result.growthPerIterKB < LeakTestResult::MAX_GROWTH_PER_ITER_KB;

        return result;
    }
};

} // namespace Isolation
} // namespace Engine
} // namespace ExplorerLens

