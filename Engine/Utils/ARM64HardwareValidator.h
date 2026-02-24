#pragma once
//==============================================================================
// ARM64HardwareValidator
// ARM64 hardware CI validation, feature detection, and performance baseline
//
// Architecture:
//   1. Runtime CPU feature detection (NEON, CRC32, AES, SHA, SVE)
//   2. Cross-compile validation for ARM64 targets
//   3. Performance baseline collection on real hardware
//   4. CI integration with GitHub Actions ARM64 runners
//   5. Binary compatibility verification for ARM64EC
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

/// ARM64 CPU feature flags (bitmask)
enum class ARM64Feature : uint32_t {
    None        = 0,
    NEON        = 1 << 0,    ///< Advanced SIMD (mandatory on AArch64)
    CRC32       = 1 << 1,    ///< CRC32 instructions
    AES         = 1 << 2,    ///< AES encryption acceleration
    SHA1        = 1 << 3,    ///< SHA-1 acceleration
    SHA256      = 1 << 4,    ///< SHA-256 acceleration
    PMULL       = 1 << 5,    ///< Polynomial multiply long
    FP16        = 1 << 6,    ///< Half-precision floating point
    DotProd     = 1 << 7,    ///< Dot product instructions (ARMv8.2)
    SVE         = 1 << 8,    ///< Scalable Vector Extension
    SVE2        = 1 << 9,    ///< SVE2
    LSE         = 1 << 10,   ///< Large System Extensions (atomics)
    BF16        = 1 << 11,   ///< BFloat16 support
    I8MM        = 1 << 12,   ///< Int8 matrix multiply
    JSCVT       = 1 << 13,   ///< JavaScript type conversion
    FLAGM       = 1 << 14,   ///< Flag manipulation
    All         = 0x7FFF
};

inline ARM64Feature operator|(ARM64Feature a, ARM64Feature b) {
    return static_cast<ARM64Feature>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline ARM64Feature operator&(ARM64Feature a, ARM64Feature b) {
    return static_cast<ARM64Feature>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline bool HasFeature(ARM64Feature set, ARM64Feature flag) {
    return (static_cast<uint32_t>(set) & static_cast<uint32_t>(flag)) != 0;
}

/// ARM64 build target
enum class ARM64Target : uint8_t {
    Native,     ///< Native AArch64
    ARM64EC,    ///< ARM64 Emulation Compatible (x64 interop)
    ARM64X,     ///< Universal ARM64X binary
    CrossCompile ///< Cross-compiled from x64 host
};

/// Performance baseline category
enum class PerfCategory : uint8_t {
    SingleDecode,    ///< Single thumbnail decode time
    BatchDecode,     ///< Batch throughput (img/sec)
    GPUScaling,      ///< GPU resize/scale time
    CacheHit,        ///< Cache lookup latency
    MemoryMapping,   ///< Memory-mapped I/O speed
    COMRegistration, ///< COM registration time
    ShellResponse    ///< Explorer shell response latency
};

/// Performance baseline result
struct PerfBaseline {
    PerfCategory category = PerfCategory::SingleDecode;
    double valueMs = 0.0;         ///< Measured value in ms
    double targetMs = 0.0;        ///< Target value
    double x64ReferenceMs = 0.0;  ///< x64 reference for comparison
    bool passed = false;
    std::wstring description;
};

/// ARM64 CI configuration
struct ARM64CIConfig {
    std::wstring runnerLabel = L"windows-arm64";
    std::wstring toolchain = L"cmake/toolchain-windows-arm64.cmake";
    std::wstring msvcArch = L"amd64_arm64";
    bool enableCrossCompile = true;
    bool enableNativeTests = true;
    bool enablePerfBaseline = true;
    uint32_t testTimeoutSec = 300;
    double perfRegressionThreshold = 1.2; ///< Allow 20% slower than x64
};

/// ARM64 validation result
struct ARM64ValidationResult {
    bool isARM64 = false;
    bool isARM64EC = false;
    ARM64Feature detectedFeatures = ARM64Feature::None;
    ARM64Target buildTarget = ARM64Target::CrossCompile;
    uint32_t featureCount = 0;
    std::vector<PerfBaseline> perfResults;
    bool allTestsPassed = false;
    bool allPerfTargetsMet = false;
    std::wstring cpuModel;
    uint32_t coreCount = 0;
    uint64_t memoryMB = 0;
};

//==============================================================================
// ARM64HardwareValidator
//==============================================================================
class ARM64HardwareValidator {
public:
    ARM64HardwareValidator() = default;
    explicit ARM64HardwareValidator(const ARM64CIConfig& config);

    /// Detect current platform
    static bool IsRunningOnARM64();
    static bool IsRunningAsARM64EC();
    static bool IsRunningUnderEmulation();

    /// Detect CPU features
    static ARM64Feature DetectFeatures();
    static uint32_t CountFeatures(ARM64Feature features);

    /// Generate performance baselines
    std::vector<PerfBaseline> GenerateBaselines() const;

    /// Run full validation suite
    ARM64ValidationResult RunValidation() const;

    /// Verify cross-compile setup
    static bool VerifyToolchain(const std::wstring& toolchainPath);

    /// Generate CI workflow YAML fragment
    static std::wstring GenerateCIWorkflow(const ARM64CIConfig& config);

    /// Verify binary compatibility
    static bool VerifyBinaryCompat(const std::wstring& dllPath);

    /// Get config
    const ARM64CIConfig& GetConfig() const { return m_config; }

    /// Static name helpers
    static const wchar_t* GetFeatureName(ARM64Feature feature);
    static const wchar_t* GetTargetName(ARM64Target target);
    static const wchar_t* GetPerfCategoryName(PerfCategory category);

    /// Get x64 reference baselines
    static std::vector<PerfBaseline> GetX64ReferenceBaselines();

private:
    ARM64CIConfig m_config;
};

}} // namespace ExplorerLens::Engine

