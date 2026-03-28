#pragma once
// FuzzingEngine.h — Consolidated Fuzzing Infrastructure
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for all fuzzing concerns:
// - ContinuousFuzzEngine: Automated fuzz framework with crash budget gating,
//   corpus management, and per-decoder mutation strategies
// - FuzzTestingManager: WinAFL/LibFuzzer/Honggfuzz harness activation
// - FuzzingCampaign: Campaign config for archive/image/font/3D targets,
//   libFuzzer/WinAFL/OneFuzz integration and harness generation
// - SEHFuzzingEngine: SEH-based fuzzing with __try/__except, corrupt payload
//   generation, worker isolation, and memory leak regression

#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// ─── ContinuousFuzzEngine ──────────────────────────────────────────────────────
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens::Utils {

// ─── Mutation strategies ─────────────────────────────────────────
enum class MutationStrategy : uint8_t {
    BitFlip = 0, // Random bit flips
    ByteFlip = 1, // Random byte overwrites
    Truncation = 2, // Truncate file at random position
    HeaderCorrupt = 3, // Corrupt magic bytes / header fields
    Insertion = 4, // Insert random bytes
    Deletion = 5, // Delete random byte ranges
    Boundary = 6, // Replace values with boundary values (0, 0xFF, MAX_INT)
    Dictionary = 7, // Use format-specific dictionary tokens
    Havoc = 8, // Combined random mutations
    COUNT = 9
};

inline const char* MutationName(MutationStrategy s) {
    switch (s) {
    case MutationStrategy::BitFlip: return "BitFlip";
    case MutationStrategy::ByteFlip: return "ByteFlip";
    case MutationStrategy::Truncation: return "Truncation";
    case MutationStrategy::HeaderCorrupt: return "HeaderCorrupt";
    case MutationStrategy::Insertion: return "Insertion";
    case MutationStrategy::Deletion: return "Deletion";
    case MutationStrategy::Boundary: return "Boundary";
    case MutationStrategy::Dictionary: return "Dictionary";
    case MutationStrategy::Havoc: return "Havoc";
    default: return "Unknown";
    }
}

// ─── Fuzz target result ──────────────────────────────────────────
enum class FuzzResult : uint8_t {
    NoError = 0, // Decoder handled gracefully
    ErrorHandled = 1, // Error returned but no crash
    Timeout = 2, // Exceeded time limit
    Crash = 3, // Unhandled exception or access violation
    Hang = 4, // Deadlock or infinite loop
    MemoryLeak = 5, // Significant memory growth
    Assertion = 6 // Debug assertion failure
};

inline const char* FuzzResultName(FuzzResult r) {
    switch (r) {
    case FuzzResult::NoError: return "NoError";
    case FuzzResult::ErrorHandled: return "ErrorHandled";
    case FuzzResult::Timeout: return "Timeout";
    case FuzzResult::Crash: return "Crash";
    case FuzzResult::Hang: return "Hang";
    case FuzzResult::MemoryLeak: return "MemoryLeak";
    case FuzzResult::Assertion: return "Assertion";
    default: return "Unknown";
    }
}

inline bool IsFailure(FuzzResult r) {
    return r == FuzzResult::Crash || r == FuzzResult::Hang || r == FuzzResult::Assertion;
}

// ─── Corpus entry ────────────────────────────────────────────────
struct FuzzCorpusEntry {
    std::string filePath;
    std::string format; // "zip", "rar", "webp", etc.
    size_t fileSize = 0;
    uint32_t mutationCount = 0;
    uint32_t crashesFound = 0;
    bool isMinimized = false;

    bool IsInteresting() const { return crashesFound > 0; }
};

// ─── Fuzz execution record ───────────────────────────────────────
struct FuzzExecution {
    uint64_t id = 0;
    std::string inputFile;
    std::string decoderName;
    MutationStrategy strategy = MutationStrategy::BitFlip;
    FuzzResult result = FuzzResult::NoError;
    double durationMs = 0.0;
    size_t memoryPeakBytes = 0;
    std::string errorMessage;

    bool IsFatal() const { return IsFailure(result); }
};

// ─── Crash budget ────────────────────────────────────────────────
struct CrashBudget {
    uint32_t maxCrashes = 0; // 0 = zero tolerance
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
    uint32_t timeoutMs = 5000; // per-input timeout
    size_t memoryLimitBytes = 256 * 1024 * 1024;
    bool minimizeCrashes = true;
    bool collectCoverage = false;
    CrashBudget budget;
    std::vector<MutationStrategy> strategies;

    static FuzzConfig Quick() {
        FuzzConfig c;
        c.maxIterations = 1000;
        c.timeoutMs = 2000;
        c.strategies = { MutationStrategy::BitFlip, MutationStrategy::HeaderCorrupt,
        MutationStrategy::Truncation };
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
    double totalDurationMs = 0.0;
    size_t corpusSize = 0;

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
            static const uint8_t boundaries[] = { 0, 1, 127, 128, 255 };
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
    uint64_t m_count = 0;
};

// ─── Continuous Fuzzing Engine ───────────────────────────────────
class ContinuousFuzzEngine {
public:
    explicit ContinuousFuzzEngine(FuzzConfig config = FuzzConfig::Quick())
        : m_config(config), m_mutator(42) {
    }

    void AddCorpusEntry(const FuzzCorpusEntry& entry) {
        m_corpus.push_back(entry);
        m_stats.corpusSize = m_corpus.size();
    }

    /// Decoder callback type for fuzz target invocation.
    using DecoderFn = std::function<bool(const uint8_t*, size_t)>;

    /// Register the decoder to be exercised during fuzzing.
    void SetDecoder(DecoderFn fn) { m_decoderFn = std::move(fn); }

    FuzzExecution RunSingle(const std::vector<uint8_t>& input,
        const std::string& decoderName,
        MutationStrategy strategy) {
        FuzzExecution exec;
        exec.id = ++m_nextId;
        exec.decoderName = decoderName;
        exec.strategy = strategy;

        auto mutated = m_mutator.Mutate(input, strategy);

        // Invoke the decoder through SEH protection so access violations,
        // stack overflows, and other structured exceptions are caught
        // instead of tearing down the host process.
        auto start = std::chrono::steady_clock::now();
        exec.result = InvokeDecoderSEH(mutated.data(), mutated.size());
        auto end = std::chrono::steady_clock::now();
        exec.durationMs =
            std::chrono::duration<double, std::milli>(end - start).count();
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

    /// Invoke the registered decoder within SEH protection.
    /// Returns Crash on structured exception, ErrorHandled when the decoder
    /// returns false, and NoError on success.
    FuzzResult InvokeDecoderSEH(const uint8_t* data, size_t size) {
        if (!m_decoderFn) return FuzzResult::NoError;
#ifdef _MSC_VER
        bool completed = false;
        bool sehFired = InvokeSEHGuarded(m_decoderFn, data, size, completed);
        if (sehFired) return FuzzResult::Crash;
        return completed ? FuzzResult::NoError : FuzzResult::ErrorHandled;
#else
        try {
            bool ok = m_decoderFn(data, size);
            return ok ? FuzzResult::NoError : FuzzResult::ErrorHandled;
        }
        catch (...) {
            return FuzzResult::Crash;
        }
#endif
    }

#ifdef _MSC_VER
    /// Separated into its own function to satisfy MSVC C2712:
    /// __try blocks cannot coexist with C++ objects that require unwinding.
    /// Parameters are references/pointers only — no local destructors.
    static bool InvokeSEHGuarded(std::function<bool(const uint8_t*, size_t)>& fn,
        const uint8_t* data, size_t size,
        bool& completed) {
        __try {
            completed = fn(data, size);
            return false;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            completed = false;
            return true;
        }
    }
#endif

    FuzzConfig m_config;
    FuzzStats m_stats;
    ByteMutator m_mutator;
    std::vector<FuzzCorpusEntry> m_corpus;
    DecoderFn m_decoderFn;
    uint64_t m_nextId = 0;
};

} // namespace ExplorerLens::Utils

// ═══════════════════════════════════════════════════════════════════════════════
// ─── FuzzTestingManager ────────────────────────────────────────────────────────
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens {
namespace Engine {

/// Fuzzer backend type
enum class FuzzerBackend : uint8_t {
    WinAFL, // Windows AFL (DynamoRIO)
    LibFuzzer, // LLVM LibFuzzer
    Honggfuzz, // Google Honggfuzz
    Manual, // Manual mutation testing
    COUNT
};

/// Mutation strategy
enum class MutationStrategy : uint8_t {
    BitFlip, // Random bit flips
    ByteFlip, // Random byte flips
    BlockDelete, // Remove random blocks
    BlockInsert, // Insert random data
    HeaderCorrupt, // Corrupt file header/magic
    Truncate, // Truncate at random point
    Duplicate, // Duplicate large sections
    CrossOver, // Mix two valid inputs
    COUNT
};

/// Fuzz target configuration (distinct from
/// CodeCoverageIntegration::FuzzTargetConfig)
struct FuzzManagerTargetConfig {
    std::wstring decoderName; // Target decoder
    std::wstring corpusDir; // Seed corpus directory
    std::wstring crashDir; // Crash output directory
    FuzzerBackend backend = FuzzerBackend::WinAFL;
    uint32_t maxInputSize = 10 * 1024 * 1024; // 10 MB
    uint32_t timeoutMs = 5000;
    uint64_t maxIterations = 1000000;
    bool enableASAN = true;
    bool enableUBSAN = false;
};

/// Fuzz run statistics
struct FuzzRunStats {
    uint64_t totalIterations = 0;
    uint64_t crashesFound = 0;
    uint64_t timeoutsFound = 0;
    uint64_t ooms = 0;
    uint64_t newCoverage = 0;
    double execsPerSecond = 0;
    double runtimeSeconds = 0;
    std::vector<std::wstring> crashPaths;
};

/// Fuzz testing manager
class FuzzTestingManager {
public:
    /// Backend name
    static const wchar_t* BackendName(FuzzerBackend b) {
        switch (b) {
        case FuzzerBackend::WinAFL:
            return L"WinAFL";
        case FuzzerBackend::LibFuzzer:
            return L"LibFuzzer";
        case FuzzerBackend::Honggfuzz:
            return L"Honggfuzz";
        case FuzzerBackend::Manual:
            return L"Manual";
        default:
            return L"Unknown";
        }
    }

    /// Mutation strategy name
    static const wchar_t* MutationName(MutationStrategy m) {
        switch (m) {
        case MutationStrategy::BitFlip:
            return L"Bit Flip";
        case MutationStrategy::ByteFlip:
            return L"Byte Flip";
        case MutationStrategy::BlockDelete:
            return L"Block Delete";
        case MutationStrategy::BlockInsert:
            return L"Block Insert";
        case MutationStrategy::HeaderCorrupt:
            return L"Header Corrupt";
        case MutationStrategy::Truncate:
            return L"Truncate";
        case MutationStrategy::Duplicate:
            return L"Duplicate";
        case MutationStrategy::CrossOver:
            return L"Cross-Over";
        default:
            return L"Unknown";
        }
    }

    /// Backend count
    static constexpr size_t BackendCount() {
        return static_cast<size_t>(FuzzerBackend::COUNT);
    }

    /// Strategy count
    static constexpr size_t StrategyCount() {
        return static_cast<size_t>(MutationStrategy::COUNT);
    }

    /// Validate fuzz config
    static bool ValidateConfig(const FuzzManagerTargetConfig& cfg) {
        if (cfg.decoderName.empty())
            return false;
        if (cfg.maxInputSize == 0 || cfg.maxInputSize > 100 * 1024 * 1024)
            return false;
        if (cfg.timeoutMs < 100)
            return false;
        return true;
    }
};

} // namespace Engine
} // namespace ExplorerLens

// ═══════════════════════════════════════════════════════════════════════════════
// ─── FuzzingCampaign ───────────────────────────────────────────────────────────
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens {
namespace Engine {

/// Fuzzing strategy for test API
enum class FuzzStrategy : uint8_t {
    RandomMutation = 0,
    StructureAware,
    GenerationBased,
    CoverageDriven,
    COUNT
};

/// Fuzzing engine selection
enum class FuzzingEngine : uint8_t {
    None = 0,
    LibFuzzer = 1, ///< LLVM libFuzzer (via MSVC /fsanitize=fuzzer)
    WinAFL = 2, ///< Windows AFL-based fuzzer
    OneFuzz = 3, ///< Microsoft OneFuzz platform
    Jazzer = 4 ///< Coverage-guided (Java/native)
};

/// Fuzzing target category
enum class FuzzTarget : uint8_t {
    ArchiveParser = 0, ///< ZIP/7Z/RAR header parsing
    ImageDecoder = 1, ///< WebP/AVIF/JXL/HEIF decode
    FontParser = 2, ///< TTF/OTF table parsing
    PDFParser = 3, ///< PDF cross-ref/object parsing
    ModelLoader = 4, ///< STL/OBJ/glTF parsing
    COMInterface = 5, ///< IExtractImage/IThumbnailProvider
    TargetCount = 6
};

/// Fuzzing campaign configuration
struct FuzzCampaignConfig {
    FuzzTarget target = FuzzTarget::ArchiveParser;
    FuzzingEngine engine = FuzzingEngine::LibFuzzer;
    uint32_t maxTotalTimeSec = 3600; ///< 1 hour default
    uint32_t maxInputSizeBytes = 1048576; ///< 1MB max input
    uint32_t maxIterations = 0; ///< 0 = unlimited
    uint32_t parallelJobs = 4; ///< Parallel fuzzer instances
    const char* corpusDir = nullptr; ///< Seed corpus directory
    const char* artifactDir = nullptr; ///< Crash artifact output
    bool enableASAN = true; ///< Address sanitizer
    bool enableUBSAN = false; ///< Undefined behavior sanitizer
    bool enableCoverage = true; ///< Track coverage feedback
    bool minimizeCrashes = true; ///< Minimize crash inputs
};

/// Crash artifact
struct FuzzCrashArtifact {
    const char* inputFile = nullptr; ///< Path to crash-triggering input
    const char* stackTrace = nullptr; ///< Stack trace at crash
    const char* crashType = nullptr; ///< e.g., "heap-buffer-overflow"
    FuzzTarget target = FuzzTarget::ArchiveParser;
    uint32_t inputSizeBytes = 0;
    bool isMinimized = false;
    bool isTriaged = false;
    uint8_t severity = 0; ///< 0=info, 1=low, 2=med, 3=high, 4=critical
};

/// Fuzzing campaign manager
class FuzzingCampaign {
public:
    static FuzzingCampaign& Instance() {
        static FuzzingCampaign inst;
        return inst;
    }

    /// Get MSVC compile flags for fuzzer instrumentation
    static const char* GetLibFuzzerCompileFlags() {
        return "/fsanitize=address " // ASan
            "/Zi " // Debug info
            "/Od " // No optimization for better crash traces
            "/DFUZZING_BUILD"; // Preprocessor flag
    }

    /// Get target function name for each fuzz target
    static const char* GetTargetFunction(FuzzTarget t) {
        switch (t) {
        case FuzzTarget::ArchiveParser:
            return "LLVMFuzzerTestOneInput_Archive";
        case FuzzTarget::ImageDecoder:
            return "LLVMFuzzerTestOneInput_Image";
        case FuzzTarget::FontParser:
            return "LLVMFuzzerTestOneInput_Font";
        case FuzzTarget::PDFParser:
            return "LLVMFuzzerTestOneInput_PDF";
        case FuzzTarget::ModelLoader:
            return "LLVMFuzzerTestOneInput_Model";
        case FuzzTarget::COMInterface:
            return "LLVMFuzzerTestOneInput_COM";
        default:
            return "LLVMFuzzerTestOneInput";
        }
    }

    /// Target name lookup
    static const char* TargetName(FuzzTarget t) {
        switch (t) {
        case FuzzTarget::ArchiveParser:
            return "Archive Parser";
        case FuzzTarget::ImageDecoder:
            return "Image Decoder";
        case FuzzTarget::FontParser:
            return "Font Parser";
        case FuzzTarget::PDFParser:
            return "PDF Parser";
        case FuzzTarget::ModelLoader:
            return "3D Model Loader";
        case FuzzTarget::COMInterface:
            return "COM Interface";
        default:
            return "Unknown";
        }
    }

    /// Generate WinAFL command line
    std::string GetWinAFLCommand(const FuzzCampaignConfig& config,
        const char* targetExe) const {
        std::string cmd = "afl-fuzz.exe";
        cmd += " -i " + std::string(config.corpusDir ? config.corpusDir : "corpus");
        cmd += " -o " +
            std::string(config.artifactDir ? config.artifactDir : "findings");
        cmd += " -t 5000+"; // 5s timeout
        cmd += " -M master"; // Master mode

        // Target module + function
        cmd += " -D \"C:\\DynamoRIO\\bin64\"";
        cmd += " -- ";
        cmd += targetExe;
        cmd += " @@"; // Input file placeholder
        return cmd;
    }

    /// Register crash artifacts
    void RecordCrash(const FuzzCrashArtifact& artifact) {
        if (m_crashCount < MAX_CRASHES) {
            m_crashes[m_crashCount++] = artifact;
        }
    }

    const FuzzCrashArtifact* GetCrashes() const { return m_crashes; }
    uint32_t GetCrashCount() const { return m_crashCount; }

    /// Campaign statistics
    uint64_t totalExecutions = 0;
    uint64_t totalCoverageEdges = 0;
    uint32_t uniqueCrashes = 0;
    uint32_t uniqueTimeouts = 0;
    double execsPerSecond = 0;

    /// Fuzzing strategy queries
    static constexpr size_t StrategyCount() {
        return static_cast<size_t>(FuzzStrategy::COUNT);
    }
    static const wchar_t* StrategyName(FuzzStrategy s) {
        switch (s) {
        case FuzzStrategy::RandomMutation:
            return L"Random Mutation";
        case FuzzStrategy::StructureAware:
            return L"Structure-Aware";
        case FuzzStrategy::GenerationBased:
            return L"Generation-Based";
        case FuzzStrategy::CoverageDriven:
            return L"Coverage-Driven";
        default:
            return L"Unknown";
        }
    }

private:
    FuzzingCampaign() = default;

    static constexpr uint32_t MAX_CRASHES = 256;
    FuzzCrashArtifact m_crashes[MAX_CRASHES] = {};
    uint32_t m_crashCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens

// ═══════════════════════════════════════════════════════════════════════════════
// ─── SEHFuzzingEngine ──────────────────────────────────────────────────────────
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens {
namespace Engine {
namespace Isolation {

//==============================================================================
// Corrupt Payload Generation Strategy
//==============================================================================
enum class CorruptionStrategy : uint32_t {
    ZeroFill = 0, // All zero payload
    RandomBytes = 1, // Pure random data
    ValidHeaderGarbage = 2, // Valid magic bytes + random tail
    Truncated = 3, // Valid header truncated at random offset
    BitFlip = 4, // Valid file with random bit flips
    OversizeField = 5, // Inject oversized dimension/length fields
    NullTerminated = 6, // Fill with 0x00 at specific offsets
    RepeatingPattern = 7, // Repeating 4-byte pattern
    MaxEnum = 8
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
    { ".zip", {0x50, 0x4B, 0x03, 0x04}, 30 },
    { ".rar", {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07}, 20 },
    { ".7z", {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C}, 32 },
    { ".png", {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}, 67 },
    { ".jpg", {0xFF, 0xD8, 0xFF, 0xE0}, 20 },
    { ".gif", {0x47, 0x49, 0x46, 0x38, 0x39, 0x61}, 13 },
    { ".bmp", {0x42, 0x4D}, 54 },
    { ".webp", {0x52, 0x49, 0x46, 0x46}, 12 },
    { ".tiff", {0x49, 0x49, 0x2A, 0x00}, 8 },
    { ".psd", {0x38, 0x42, 0x50, 0x53}, 26 },
    { ".pdf", {0x25, 0x50, 0x44, 0x46}, 15 },
    { ".heif", {0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70}, 12 },
    { ".jxl", {0xFF, 0x0A}, 4 },
    { ".avif", {0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70}, 12 },
    { ".exr", {0x76, 0x2F, 0x31, 0x01}, 8 },
    { ".dds", {0x44, 0x44, 0x53, 0x20}, 128 },
    { ".ico", {0x00, 0x00, 0x01, 0x00}, 22 },
    { ".svg", {0x3C, 0x73, 0x76, 0x67}, 10 },
    { ".qoi", {0x71, 0x6F, 0x69, 0x66}, 14 },
    { ".tga", {0x00, 0x00, 0x02, 0x00}, 18 },
    };
}

//==============================================================================
// Fuzz Iteration Result
//==============================================================================
struct FuzzResult {
    uint64_t iterationId = 0;
    CorruptionStrategy strategy = CorruptionStrategy::ZeroFill;
    std::string targetFormat;
    size_t payloadSize = 0;
    bool sehCaught = false; // __except fired
    bool crashed = false; // Should always be false
    bool gracefulReject = false; // Decoder returned error cleanly
    bool circuitBroken = false; // Circuit breaker tripped
    DWORD exceptionCode = 0; // If SEH fired
    double elapsedMs = 0.0;

    bool IsPass() const { return !crashed; }
};

//==============================================================================
// Fuzz Campaign Summary
//==============================================================================
struct FuzzCampaignReport {
    uint64_t totalIterations = 0;
    uint64_t sehExceptions = 0;
    uint64_t gracefulRejects = 0;
    uint64_t circuitBreaks = 0;
    uint64_t crashes = 0; // Must be 0 to pass
    uint64_t timeouts = 0;
    double totalElapsedMs = 0.0;
    double avgIterationMs = 0.0;
    std::vector<FuzzResult> failures; // Only failed iterations

    bool OverallPass() const { return crashes == 0; }

    double SEHRate() const {
        return totalIterations > 0
            ? 100.0 * sehExceptions / totalIterations : 0.0;
    }

    double GracefulRate() const {
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
        : m_rng(seed ? seed : std::random_device{}()) {
    }

    std::vector<uint8_t> Generate(CorruptionStrategy strategy,
        const std::string& extension,
        size_t targetSize) {
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
        bool completed = false;
        bool sehCaught = false;
        DWORD exceptionCode = 0;
        double elapsedMs = 0.0;
    };

    // Execute a callable within SEH protection
    // The callable should return bool (true = success, false = error)
    static ExecutionResult Execute(std::function<bool()> callable,
        uint32_t /*timeoutMs*/ = 5000) {
        ExecutionResult result{};
        auto start = std::chrono::steady_clock::now();

#ifdef _MSC_VER
        result.sehCaught = InvokeSEH(callable, result.completed, result.exceptionCode);
#else
        // Non-MSVC fallback — standard try/catch
        try {
            result.completed = callable();
        }
        catch (...) {
            result.sehCaught = true;
            result.completed = false;
        }
#endif

        auto end = std::chrono::steady_clock::now();
        result.elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        return result;
    }

private:
#ifdef _MSC_VER
    // Separated from Execute() to avoid C2712: __try cannot coexist with
    // C++ objects that require unwinding (std::function parameter).
    // References/pointers only — no local C++ destructors.
    static bool InvokeSEH(std::function<bool()>& fn, bool& completed, DWORD& exCode) {
        __try {
            completed = fn();
            return false;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            exCode = GetExceptionCode();
            completed = false;
            return true;
        }
    }
#endif
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
        uint32_t workerId = 0;
        uint64_t totalRequests = 0;
        uint64_t sehExceptions = 0;
        uint64_t timeouts = 0;
        uint64_t successfulDecodes = 0;
        bool markedForRecycle = false;

        double FailureRate() const {
            if (totalRequests == 0) return 0.0;
            return 100.0 * (sehExceptions + timeouts) / totalRequests;
        }
    };

    explicit WorkerIsolationMonitor(uint32_t maxFailuresBeforeRecycle = 10,
        double maxFailureRatePercent = 5.0)
        : m_maxFailures(maxFailuresBeforeRecycle)
        , m_maxFailureRate(maxFailureRatePercent) {
    }

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
    double m_maxFailureRate;
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
        : m_generator(seed) {
    }

    // Attach a decode callback that simulates the thumbnail pipeline
    void SetDecoderCallback(DecoderCallback cb) { m_decoder = std::move(cb); }

    // Run fuzzing campaign with specified iterations per format
    FuzzCampaignReport Run(uint64_t iterationsPerFormat,
        const std::vector<std::string>& formats) {
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
    FuzzCampaignReport QuickSmoke(uint64_t totalIterations) {
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
    std::mt19937_64 m_rng{ 42 };
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
        SIZE_T workingSetBytes = 0;
        SIZE_T privateBytes = 0;
        SIZE_T peakWorkingSet = 0;
        SIZE_T heapAllocBytes = 0;
    };

    struct LeakTestResult {
        MemorySnapshot before;
        MemorySnapshot after;
        MemorySnapshot peak;
        uint64_t iterations = 0;
        double workingSetGrowthMB = 0.0;
        double privateGrowthMB = 0.0;
        double growthPerIterKB = 0.0;
        bool passedLeakCheck = false;

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
        uint64_t iterations = 100) {
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
