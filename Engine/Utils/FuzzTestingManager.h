//==============================================================================
// DarkThumbs Engine — Sprint 272: Fuzz Testing Activation
// WinAFL/LibFuzzer harness for all decoders. Corrupt file handling validation.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Fuzzer backend type
enum class FuzzerBackend : uint8_t {
    WinAFL,         // Windows AFL (DynamoRIO)
    LibFuzzer,      // LLVM LibFuzzer
    Honggfuzz,      // Google Honggfuzz
    Manual,         // Manual mutation testing
    COUNT
};

/// Mutation strategy
enum class MutationStrategy : uint8_t {
    BitFlip,        // Random bit flips
    ByteFlip,       // Random byte flips
    BlockDelete,    // Remove random blocks
    BlockInsert,    // Insert random data
    HeaderCorrupt,  // Corrupt file header/magic
    Truncate,       // Truncate at random point
    Duplicate,      // Duplicate large sections
    CrossOver,      // Mix two valid inputs
    COUNT
};

/// Fuzz target configuration
struct FuzzTargetConfig {
    std::wstring    decoderName;        // Target decoder
    std::wstring    corpusDir;          // Seed corpus directory
    std::wstring    crashDir;           // Crash output directory
    FuzzerBackend   backend             = FuzzerBackend::WinAFL;
    uint32_t        maxInputSize        = 10 * 1024 * 1024; // 10 MB
    uint32_t        timeoutMs           = 5000;
    uint64_t        maxIterations       = 1000000;
    bool            enableASAN          = true;
    bool            enableUBSAN         = false;
};

/// Fuzz run statistics
struct FuzzRunStats {
    uint64_t totalIterations    = 0;
    uint64_t crashesFound       = 0;
    uint64_t timeoutsFound      = 0;
    uint64_t ooms               = 0;
    uint64_t newCoverage        = 0;
    double   execsPerSecond     = 0;
    double   runtimeSeconds     = 0;
    std::vector<std::wstring> crashPaths;
};

/// Fuzz testing manager
class FuzzTestingManager {
public:
    /// Backend name
    static const wchar_t* BackendName(FuzzerBackend b) {
        switch (b) {
            case FuzzerBackend::WinAFL:     return L"WinAFL";
            case FuzzerBackend::LibFuzzer:  return L"LibFuzzer";
            case FuzzerBackend::Honggfuzz:  return L"Honggfuzz";
            case FuzzerBackend::Manual:     return L"Manual";
            default: return L"Unknown";
        }
    }

    /// Mutation strategy name
    static const wchar_t* MutationName(MutationStrategy m) {
        switch (m) {
            case MutationStrategy::BitFlip:       return L"Bit Flip";
            case MutationStrategy::ByteFlip:      return L"Byte Flip";
            case MutationStrategy::BlockDelete:   return L"Block Delete";
            case MutationStrategy::BlockInsert:   return L"Block Insert";
            case MutationStrategy::HeaderCorrupt: return L"Header Corrupt";
            case MutationStrategy::Truncate:      return L"Truncate";
            case MutationStrategy::Duplicate:     return L"Duplicate";
            case MutationStrategy::CrossOver:     return L"Cross-Over";
            default: return L"Unknown";
        }
    }

    /// Backend count
    static constexpr size_t BackendCount() { return static_cast<size_t>(FuzzerBackend::COUNT); }

    /// Strategy count
    static constexpr size_t StrategyCount() { return static_cast<size_t>(MutationStrategy::COUNT); }

    /// Validate fuzz config
    static bool ValidateConfig(const FuzzTargetConfig& cfg) {
        if (cfg.decoderName.empty()) return false;
        if (cfg.maxInputSize == 0 || cfg.maxInputSize > 100 * 1024 * 1024) return false;
        if (cfg.timeoutMs < 100) return false;
        return true;
    }
};

}} // namespace DarkThumbs::Engine
