#pragma once
// ARM64Platform.h — Consolidated ARM64 Platform Support
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for all ARM64 platform concerns:
// - Architecture detection, NEON/SVE intrinsics, compile-time flags
// - CMake/MSBuild ARM64 toolchain configuration descriptors
// - Library cross-compilation status tracking
// - CI matrix entries and GitHub Actions workflow descriptors
// - Runtime CPU feature detection (NEON, CRC32, AES, SHA, SVE)
// - Cross-compile validation and binary compatibility verification
// - Performance baselines and NEON vs scalar comparison
// - NEON/SVE2 SIMD acceleration profiles, efficiency core scheduling
// - Platform validation (boot, decoders, GPU, SIMD, COM)
// - Runtime probes for decoder availability, GPU capability, shell registration
//
// Merged from: ARM64BuildConfig.h, ARM64CIIntegration.h,
//              ARM64LibraryMatrix.h, ARM64PerformanceBaseline.h,
//              ARM64PerformanceOptimizer.h, ARM64PlatformValidator.h,
//              ARM64RuntimeValidator.h
// ARM64HardwareValidator.h + .cpp kept separate (has .cpp translation unit).

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Section: ARM64 Build Configuration  (was ARM64BuildConfig.h)
// Namespace: ExplorerLens::Platform
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens::Platform {

// ─── Architecture detection ───────────────────────────────────────────────────

enum class TargetArchitecture : uint32_t {
    x86 = 0,
    x64 = 1,
    ARM = 2,
    ARM64 = 3,
    ARM64EC = 4, // ARM64 EC (emulation compatible)
    Unknown = 99,
};

inline std::string ToString(TargetArchitecture arch) {
    switch (arch) {
    case TargetArchitecture::x86: return "x86";
    case TargetArchitecture::x64: return "x64";
    case TargetArchitecture::ARM: return "ARM";
    case TargetArchitecture::ARM64: return "ARM64";
    case TargetArchitecture::ARM64EC: return "ARM64EC";
    default: return "Unknown";
    }
}

// Compile-time detection
inline constexpr TargetArchitecture CurrentArchitecture() {
#if defined(_M_ARM64) || defined(__aarch64__)
    return TargetArchitecture::ARM64;
#elif defined(_M_ARM64EC)
    return TargetArchitecture::ARM64EC;
#elif defined(_M_ARM) || defined(__arm__)
    return TargetArchitecture::ARM;
#elif defined(_M_X64) || defined(__x86_64__)
    return TargetArchitecture::x64;
#elif defined(_M_IX86) || defined(__i386__)
    return TargetArchitecture::x86;
#else
    return TargetArchitecture::Unknown;
#endif
}

// ─── SIMD capability flags ────────────────────────────────────────────────────

struct SIMDCapabilities {
    // x64
    bool hasSSE2{ false };
    bool hasSSE4_1{ false };
    bool hasAVX2{ false };
    bool hasAVX512{ false };
    // ARM64
    bool hasNEON{ false };
    bool hasSVE{ false }; // Scalable Vector Extension
    bool hasBFLOAT16{ false };

    static SIMDCapabilities DetectForArch(TargetArchitecture arch) {
        SIMDCapabilities c;
        if (arch == TargetArchitecture::x64 || arch == TargetArchitecture::x86) {
            c.hasSSE2 = true; // baseline on x64 Windows
            c.hasSSE4_1 = true; // assumed on modern x64
            // AVX2 requires runtime CPUID check
        }
        if (arch == TargetArchitecture::ARM64 || arch == TargetArchitecture::ARM64EC) {
            c.hasNEON = true; // mandatory on ARMv8-A
        }
        return c;
    }

    bool HasAnyVector() const { return hasSSE2 || hasNEON; }
};

// ─── ARM64 build config descriptor ───────────────────────────────────────────

struct ARM64BuildConfig {
    std::string cmakeToolset{ "v145" }; // VS18 ARM64 toolset
    std::string cmakePlatform{ "ARM64" };
    std::string platformToolset{ "v145" };
    bool crossCompileFromX64{ true };
    bool enableNEON{ true };
    bool enableSVE{ false }; // opt-in when supported
    bool enableWinRT{ false }; // future

    // Toolchain file path (relative to project root)
    std::string toolchainFilePath{ "cmake/toolchain-windows-arm64.cmake" };

    static ARM64BuildConfig Default() { return {}; }
    static ARM64BuildConfig WithSVE() {
        auto c = Default();
        c.enableSVE = true;
        return c;
    }
};

// ─── CMake ARM64 toolchain parameters ─────────────────────────────────────────

struct CMakeARM64Parameters {
    // CMAKE_SYSTEM_PROCESSOR
    std::string systemProcessor{ "ARM64" };
    // CMAKE_C_COMPILER / CMAKE_CXX_COMPILER
    // (handled by VS generator — not set explicitly)

    // Compile flags
    std::vector<std::string> compileFlags{ "/favor:ARM64" };
    // Define flags
    std::vector<std::string> defineFlags{ "_ARM64_", "WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP" };

    std::string ToCMakeArgs() const {
        std::string args = "-DCMAKE_SYSTEM_PROCESSOR=ARM64";
        for (const auto& f : compileFlags)
            args += " -DCMAKE_C_FLAGS=\"" + f + "\" -DCMAKE_CXX_FLAGS=\"" + f + "\"";
        return args;
    }
};

// ─── MSBuild ARM64 platform metadata ─────────────────────────────────────────

struct MSBuildARM64Config {
    std::string platform{ "ARM64" };
    std::string configuration{ "Release" };
    std::string platformToolset{ "v145" };
    std::string windowsTargetPlatformVersion{ "10.0" };

    std::string ToMSBuildArgs() const {
        return "/p:Platform=ARM64 /p:Configuration=" + configuration +
            " /p:PlatformToolset=" + platformToolset;
    }
};

// ─── Library cross-compilation status ─────────────────────────────────────────

enum class LibraryARM64Status : uint32_t {
    NotStarted = 0,
    BuildSuccess = 1,
    BuildFailed = 2,
    Fallback = 3, // using WIC / inbox codec instead
    NotRequired = 4,
};

struct LibraryARM64Entry {
    std::string libraryName;
    std::string version;
    LibraryARM64Status status{ LibraryARM64Status::NotStarted };
    std::string notes;
};

struct ARM64LibraryInventory {
    std::vector<LibraryARM64Entry> entries;

    uint32_t BuildSuccessCount() const {
        uint32_t n = 0;
        for (const auto& e : entries)
            if (e.status == LibraryARM64Status::BuildSuccess) ++n;
        return n;
    }

    static ARM64LibraryInventory CreateDefault() {
        ARM64LibraryInventory inv;
        inv.entries = {
            { "zlib", "1.3.1", LibraryARM64Status::NotStarted, "" },
            { "LZ4", "1.10.0", LibraryARM64Status::NotStarted, "" },
            { "zstd", "1.5.7", LibraryARM64Status::NotStarted, "" },
            { "LZMA SDK", "26.00", LibraryARM64Status::NotStarted, "" },
            { "minizip-ng", "4.0.10", LibraryARM64Status::NotStarted, "" },
            { "UnRAR", "7.2.2", LibraryARM64Status::NotStarted, "" },
            { "libwebp", "1.5.0", LibraryARM64Status::NotStarted, "" },
            { "libavif", "1.3.0", LibraryARM64Status::NotStarted, "" },
            { "libjxl", "0.11.1", LibraryARM64Status::NotStarted, "" },
            { "libheif", "1.19.5", LibraryARM64Status::NotStarted, "" },
            { "libde265", "1.0.15", LibraryARM64Status::NotStarted, "" },
            { "LibRaw", "0.21.3", LibraryARM64Status::NotStarted, "" },
        };
        return inv;
    }
};

} // namespace ExplorerLens::Platform

// ═══════════════════════════════════════════════════════════════════════════════
// Section: ARM64 CI Integration  (was ARM64CIIntegration.h)
// Namespace: ExplorerLens::Platform
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens::Platform {

// ─── CI stage types ───────────────────────────────────────────────────────────

enum class CIStage : uint32_t {
    Checkout = 0,
    Setup = 1,
    Build = 2,
    UnitTest = 3,
    Deploy = 4,
    Validate = 5,
};

inline std::string ToString(CIStage s) {
    switch (s) {
    case CIStage::Checkout: return "checkout";
    case CIStage::Setup: return "setup";
    case CIStage::Build: return "build";
    case CIStage::UnitTest: return "unit-test";
    case CIStage::Deploy: return "deploy";
    case CIStage::Validate: return "validate";
    default: return "unknown";
    }
}

// ─── CI matrix dimension ─────────────────────────────────────────────────────

struct CIMatrixDimension {
    std::string name;
    std::vector<std::string> values;
};

struct CIMatrixEntry {
    std::string architecture; // e.g., "ARM64"
    std::string configuration; // "Release" / "Debug"
    std::string runner; // "windows-2025" / "self-hosted"
    std::string vsVersion; // "18.0"
    bool runTests{ false };
    bool uploadArtifacts{ true };

    static CIMatrixEntry ARM64BuildOnly() {
        return { "ARM64", "Release", "windows-2025", "18.0", false, true };
    }

    static CIMatrixEntry ARM64Full() {
        return { "ARM64", "Release", "self-hosted-arm64", "18.0", true, true };
    }
};

// ─── GitHub Actions workflow descriptor ──────────────────────────────────────

struct WorkflowJobDescriptor {
    std::string workflowFile{ ".github/workflows/arm64.yml" };
    std::string jobName{ "build-arm64" };
    std::vector<CIMatrixEntry> matrix;
    std::vector<CIStage> stages;
    bool continueOnError{ false };
    uint32_t timeoutMinutes{ 60 };

    static WorkflowJobDescriptor Default() {
        WorkflowJobDescriptor d;
        d.matrix = { CIMatrixEntry::ARM64BuildOnly() };
        d.stages = {
            CIStage::Checkout,
            CIStage::Setup,
            CIStage::Build,
            CIStage::UnitTest,
        };
        return d;
    }

    std::string ToYAMLFragment() const {
        std::string yaml = "    " + jobName + ":\n";
        yaml += "      runs-on: " + (matrix.empty() ? "windows-2025" : matrix[0].runner) + "\n";
        yaml += "      timeout-minutes: " + std::to_string(timeoutMinutes) + "\n";
        yaml += "      steps:\n";
        for (const auto& s : stages)
            yaml += "        - name: " + ToString(s) + "\n";
        return yaml;
    }
};

// ─── ARM64 documentation manifest ────────────────────────────────────────────

struct ARM64DocsManifest {
    struct DocEntry {
        std::string filePath;
        std::string description;
        bool required{ true };
    };

    std::vector<DocEntry> entries;

    static ARM64DocsManifest Required() {
        ARM64DocsManifest m;
        m.entries = {
            { "docs/ARM64_SUPPORT.md", "ARM64 status, limitations, library matrix", true },
            { "cmake/toolchain-windows-arm64.cmake", "CMake ARM64 toolchain file", true },
            { ".github/workflows/arm64.yml", "ARM64 CI workflow", true },
            { "docs/ARM64_SUPPORT.md", "ARM64 execution record", true },
        };
        return m;
    }

    bool AllRequiredPresent(const std::vector<std::string>& existingFiles) const {
        for (const auto& e : entries) {
            if (!e.required) continue;
            bool found = false;
            for (const auto& f : existingFiles)
                if (f == e.filePath) { found = true; break; }
            if (!found) return false;
        }
        return true;
    }
};

// ─── ARM64 CI integration summary ────────────────────────────────────────────

struct ARM64CIIntegration {
    WorkflowJobDescriptor workflow;
    ARM64DocsManifest docsManifest;
    bool masterPlanUpdated{ false };

    bool IsComplete(const std::vector<std::string>& existingFiles) const {
        return masterPlanUpdated && docsManifest.AllRequiredPresent(existingFiles);
    }

    static ARM64CIIntegration CreateDefault() {
        ARM64CIIntegration ci;
        ci.workflow = WorkflowJobDescriptor::Default();
        ci.docsManifest = ARM64DocsManifest::Required();
        ci.masterPlanUpdated = true;
        return ci;
    }
};

} // namespace ExplorerLens::Platform

// ═══════════════════════════════════════════════════════════════════════════════
// Section: ARM64 Library Cross-Compilation Matrix  (was ARM64LibraryMatrix.h)
// Namespace: ExplorerLens::Platform
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens::Platform {

// ─── Library build result ─────────────────────────────────────────────────────

enum class LibBuildStatus : uint32_t {
    NotStarted = 0,
    Success = 1,
    Failed = 2,
    Skipped = 3, // library not needed for ARM64 target
    Fallback = 4, // using alternative implementation
};

inline std::string ToString(LibBuildStatus s) {
    switch (s) {
    case LibBuildStatus::NotStarted: return "NotStarted";
    case LibBuildStatus::Success: return "Success";
    case LibBuildStatus::Failed: return "Failed";
    case LibBuildStatus::Skipped: return "Skipped";
    case LibBuildStatus::Fallback: return "Fallback";
    default: return "Unknown";
    }
}

// ─── Library matrix entry ─────────────────────────────────────────────────────

struct LibMatrixEntry {
    std::string name;
    std::string version;
    std::string buildScript; // relative path from project root
    LibBuildStatus x64Status{ LibBuildStatus::Success }; // baseline
    LibBuildStatus arm64Status{ LibBuildStatus::NotStarted };
    std::string outputLib; // expected .lib path for ARM64
    std::string notes;

    bool IsArm64Ready() const {
        return arm64Status == LibBuildStatus::Success ||
            arm64Status == LibBuildStatus::Fallback;
    }
};

// ─── Build parameter ─────────────────────────────────────────────────────────

struct CrossBuildParameters {
    std::string architecture{ "ARM64" };
    std::string toolset{ "v145" };
    std::string generator{ "Visual Studio 18 2026" };
    std::string toolchainFile; // e.g., cmake/toolchain-windows-arm64.cmake
    bool clean{ false };
    bool verbose{ false };

    std::string ToPSArgString() const {
        return "-Architecture " + architecture + " -Toolset " + toolset;
    }
};

// ─── Full ARM64 library matrix ────────────────────────────────────────────────

struct ARM64LibraryMatrix {
    std::vector<LibMatrixEntry> libraries;
    CrossBuildParameters buildParams;

    uint32_t ReadyCount() const {
        uint32_t n = 0;
        for (const auto& e : libraries)
            if (e.IsArm64Ready()) ++n;
        return n;
    }

    uint32_t FailedCount() const {
        uint32_t n = 0;
        for (const auto& e : libraries)
            if (e.arm64Status == LibBuildStatus::Failed) ++n;
        return n;
    }

    static constexpr uint32_t kMinPassThreshold = 7; // at least 7 of 12 must succeed

    bool MeetsPassThreshold() const { return ReadyCount() >= kMinPassThreshold; }

    std::string SummaryReport() const {
        return "ARM64 Library Matrix: " + std::to_string(ReadyCount()) + "/" +
            std::to_string(libraries.size()) + " ready — " +
            (MeetsPassThreshold() ? "PASS" : "FAIL");
    }

    static ARM64LibraryMatrix CreateDefault() {
        ARM64LibraryMatrix m;
        m.buildParams = CrossBuildParameters{};
        m.libraries = {
            { "zlib", "1.3.1", "build-scripts/external-libs/Build-Zlib.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/compression-libs/zlib-1.3.1/lib/ARM64/Release/zlibstatic.lib", "" },

            { "LZ4", "1.10.0", "build-scripts/external-libs/Build-LZ4.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/compression-libs/lz4-1.10.0/lib/ARM64/Release/lz4.lib", "" },

            { "zstd", "1.5.7", "build-scripts/external-libs/Build-Zstd.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/compression-libs/zstd-1.5.7/lib/ARM64/Release/zstd_static.lib", "" },

            { "LZMA SDK", "26.00", "build-scripts/external-libs/Build-LZMA-SDK-26.00.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/compression-libs/lzma-26.00/lib/ARM64/Release/7za.lib", "" },

            { "minizip-ng", "4.0.10", "build-scripts/external-libs/Build-MinizipNG.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/compression-libs/minizip-ng-4.0.10/lib/ARM64/Release/minizip.lib", "" },

            { "UnRAR", "7.2.2", "build-scripts/external-libs/Build-UnRAR.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/compression-libs/unrar-7.2.2/lib/ARM64/Release/unrar.lib",
                "LGPL source — ARM64 MSVC not fully tested" },

            { "libwebp", "1.5.0", "build-scripts/external-libs/Build-LibWebP-NMake.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/image-libs/libwebp-1.5.0/lib/ARM64/Release/webp.lib", "" },

            { "libavif", "1.3.0", "build-scripts/external-libs/build-libavif.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/image-libs/libavif-1.3.0/lib/ARM64/Release/avif.lib", "" },

            { "libjxl", "0.11.1", "build-scripts/external-libs/build-libjxl.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/image-libs/libjxl-0.11.1/lib/ARM64/Release/jxl.lib",
                "Requires Brotli + HWY ARM64 support" },

            { "libheif", "1.19.5", "build-scripts/external-libs/Build-LibHEIF.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/image-libs/libheif-1.19.5/lib/ARM64/Release/heif.lib", "" },

            { "libde265", "1.0.15", "build-scripts/external-libs/Build-LibHEIF.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/image-libs/libde265-1.0.15/lib/ARM64/Release/de265.lib", "" },

            { "LibRaw", "0.21.3", "build-scripts/external-libs/Build-LibRaw.ps1",
                LibBuildStatus::Success, LibBuildStatus::NotStarted,
                "external/camera-libs/libraw-0.21.3/lib/ARM64/Release/raw.lib", "" },
        };
        return m;
    }
};

} // namespace ExplorerLens::Platform

// ═══════════════════════════════════════════════════════════════════════════════
// Section: ARM64 Performance Baseline  (was ARM64PerformanceBaseline.h)
// Namespace: ExplorerLens::Platform
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens::Platform {

// ─── KPI constants ────────────────────────────────────────────────────────────

struct ARM64KPIs {
    // Single thumbnail decode p95 (ms)
    static constexpr double kSingleThumbP95Ms = 150.0; // ARM64 initial target
    // Batch throughput (images/sec)
    static constexpr double kBatchThroughputImgSec = 100.0; // ARM64 initial target
    // Cache-hit average (ms)
    static constexpr double kCacheHitAvgMs = 5.0;
    // Cold start (ms)
    static constexpr double kColdStartMs = 750.0; // ARM64 first load
};

// ─── Benchmark sample ─────────────────────────────────────────────────────────

struct BenchmarkSample {
    std::string label;
    double minMs{ 0.0 };
    double maxMs{ 0.0 };
    double meanMs{ 0.0 };
    double p95Ms{ 0.0 };
    double p99Ms{ 0.0 };
    uint32_t samples{ 0 };
    double throughput{ 0.0 }; // images/sec for batch tests

    bool MeetsP95Budget(double budgetMs) const { return p95Ms <= budgetMs; }
};

// ─── NEON vs scalar comparison ────────────────────────────────────────────────

struct SIMDComparisonResult {
    std::string operation; // e.g., "RGB rescale 256x256"
    BenchmarkSample neonPath;
    BenchmarkSample scalarPath;
    double speedupRatio{ 1.0 }; // neon / scalar (>1 = NEON faster)
    bool neonWins{ false };

    static constexpr double kMinExpectedSpeedup = 1.0; // NEON must not be slower

    std::string Summary() const {
        return operation + ": NEON=" + std::to_string(neonPath.meanMs) + "ms" +
            " scalar=" + std::to_string(scalarPath.meanMs) + "ms" +
            " ratio=" + std::to_string(speedupRatio) +
            (neonWins ? " [NEON wins]" : " [scalar faster!]");
    }
};

// ─── ARM64 baseline record ────────────────────────────────────────────────────

struct ARM64PerformanceBaseline {
    std::string capturedAt; // ISO-8601 timestamp
    std::string deviceModel; // e.g., "Surface Pro X"
    std::string cpuModel; // e.g., "Qualcomm Snapdragon 8cx Gen 3"

    BenchmarkSample singleThumb;
    BenchmarkSample batchThroughput;
    BenchmarkSample cacheHit;
    BenchmarkSample coldStart;

    std::vector<SIMDComparisonResult> simdComparisons;

    bool MeetsAllKPIs() const {
        return singleThumb.MeetsP95Budget(ARM64KPIs::kSingleThumbP95Ms) &&
            batchThroughput.throughput >= ARM64KPIs::kBatchThroughputImgSec &&
            cacheHit.MeetsP95Budget(ARM64KPIs::kCacheHitAvgMs) &&
            coldStart.MeetsP95Budget(ARM64KPIs::kColdStartMs);
    }

    std::string Summary() const {
        return "ARM64 Baseline: thumb.p95=" + std::to_string(singleThumb.p95Ms) + "ms" +
            " batch=" + std::to_string(batchThroughput.throughput) + "img/s" +
            " cache=" + std::to_string(cacheHit.meanMs) + "ms" +
            " cold=" + std::to_string(coldStart.meanMs) + "ms — " +
            (MeetsAllKPIs() ? "PASS" : "FAIL");
    }

    static ARM64PerformanceBaseline CreateMock() {
        ARM64PerformanceBaseline b;
        b.capturedAt = "2026-02-19T00:00:00Z";
        b.deviceModel = "Surface Pro X (emulated)";
        b.cpuModel = "Qualcomm Snapdragon 8cx Gen 3";
        b.singleThumb = { "SingleThumb", 15.0, 95.0, 42.0, 88.0, 93.0, 200 };
        b.batchThroughput = { "Batch20", 0.0, 0.0, 0.0, 0.0, 0.0, 20, 115.0 };
        b.cacheHit = { "CacheHit", 0.5, 4.8, 2.1, 4.2, 4.6, 100 };
        b.coldStart = { "ColdStart", 600.0, 720.0, 650.0, 710.0, 718.0, 5 };
        b.simdComparisons = {
            { "RGB rescale 256x256",
                { "NEON", 0.8, 2.1, 1.1, 1.8, 2.0, 500 },
                { "Scalar", 1.5, 4.2, 2.3, 3.8, 4.1, 500 },
                2.09, true },
        };
        return b;
    }
};

// ─── Trend comparator ────────────────────────────────────────────────────────

struct BaselineTrendResult {
    double deltaP95Pct{ 0.0 }; // +ve = regression, -ve = improvement
    double deltaThroughputPct{ 0.0 };
    bool hasRegression{ false };

    static constexpr double kRegressionThreshold = 10.0; // >10% = regression

    static BaselineTrendResult Compare(const ARM64PerformanceBaseline& prev,
        const ARM64PerformanceBaseline& curr) {
        BaselineTrendResult r;
        if (prev.singleThumb.p95Ms > 0) {
            r.deltaP95Pct = 100.0 * (curr.singleThumb.p95Ms - prev.singleThumb.p95Ms)
                / prev.singleThumb.p95Ms;
        }
        if (prev.batchThroughput.throughput > 0) {
            r.deltaThroughputPct = 100.0 * (curr.batchThroughput.throughput - prev.batchThroughput.throughput)
                / prev.batchThroughput.throughput;
        }
        r.hasRegression = r.deltaP95Pct > kRegressionThreshold ||
            r.deltaThroughputPct < -kRegressionThreshold;
        return r;
    }
};

} // namespace ExplorerLens::Platform

// ═══════════════════════════════════════════════════════════════════════════════
// Section: ARM64 Runtime Validator  (was ARM64RuntimeValidator.h)
// Namespace: ExplorerLens::Platform
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens::Platform {

// ─── Platform probe ──────────────────────────────────────────────────────────

struct PlatformProbeResult {
    std::string osVersion;
    std::string processorArch; // from SYSTEM_INFO
    bool isNativeARM64{ false };
    bool isARM64EC{ false };
    bool isX64OnARM64{ false }; // x64 emulation
    bool isWow64{ false };
    uint32_t cpuCoreCount{ 0 };
    uint64_t availableMemoryMB{ 0 };

    std::string Summary() const {
        return "Platform: " + processorArch + " | OS: " + osVersion +
            " | Native=" + (isNativeARM64 ? "yes" : "no") +
            " | Cores=" + std::to_string(cpuCoreCount);
    }
};

// ─── Decoder confidence test ─────────────────────────────────────────────────

enum class DecoderConfidenceLevel : uint32_t {
    FullPass = 0, // decode produced correct output
    Partial = 1, // decode succeeded but with degraded quality
    Fallback = 2, // used alternative decode path
    Skip = 3, // decoder not available on ARM64
    Fail = 4,
};

inline std::string ToString(DecoderConfidenceLevel l) {
    switch (l) {
    case DecoderConfidenceLevel::FullPass: return "FullPass";
    case DecoderConfidenceLevel::Partial: return "Partial";
    case DecoderConfidenceLevel::Fallback: return "Fallback";
    case DecoderConfidenceLevel::Skip: return "Skip";
    case DecoderConfidenceLevel::Fail: return "Fail";
    default: return "Unknown";
    }
}

struct DecoderConfidenceEntry {
    std::string extension;
    std::string decoderName;
    DecoderConfidenceLevel level{ DecoderConfidenceLevel::Fail };
    double decodeMs{ 0.0 };
    std::string notes;

    bool Passed() const {
        return level == DecoderConfidenceLevel::FullPass ||
            level == DecoderConfidenceLevel::Partial ||
            level == DecoderConfidenceLevel::Fallback;
    }
};

// ─── GPU capability on ARM64 ──────────────────────────────────────────────────

struct ARM64GPUCapability {
    bool d3d11Available{ false };
    bool d3d12Available{ false };
    std::string adapterName;
    uint64_t dedicatedVRAMMB{ 0 };
    bool isIntegratedGPU{ true };

    bool CanRenderThumbnails() const { return d3d11Available || d3d12Available; }
    std::string Summary() const {
        return "GPU: " + adapterName + " | D3D11=" + (d3d11Available ? "yes" : "no") +
            " D3D12=" + (d3d12Available ? "yes" : "no");
    }
};

// ─── COM registration check ──────────────────────────────────────────────────

struct COMRegistrationCheck {
    std::string clsid{ "9E6ECB90-5A61-42BD-B851-D3297D9C7F39" };
    bool isRegistered{ false };
    std::string registeredPath;
    bool pathExists{ false };

    bool IsValid() const { return isRegistered && pathExists; }
};

// ─── Runtime validation report ───────────────────────────────────────────────

struct ARM64RuntimeValidationReport {
    PlatformProbeResult platform;
    ARM64GPUCapability gpu;
    COMRegistrationCheck comRegistration;
    std::vector<DecoderConfidenceEntry> decoderTests;
    uint32_t decoderPassCount{ 0 };
    uint32_t decoderFailCount{ 0 };

    static constexpr uint32_t kMinDecoderPassCount = 5; // JPEG/PNG/WebP/HEIF/RAW

    bool MeetsCriteria() const {
        return decoderPassCount >= kMinDecoderPassCount &&
            gpu.CanRenderThumbnails();
    }

    std::string Summary() const {
        return "ARM64 Runtime: decoders=" + std::to_string(decoderPassCount) + "/" +
            std::to_string(decoderTests.size()) + " | GPU=" +
            (gpu.CanRenderThumbnails() ? "OK" : "NoGPU") + " | COM=" +
            (comRegistration.IsValid() ? "OK" : "NotReg") + " — " +
            (MeetsCriteria() ? "PASS" : "FAIL");
    }

    static ARM64RuntimeValidationReport CreateMock() {
        ARM64RuntimeValidationReport r;
        r.platform = { "Windows 11 24H2", "ARM64", true, false, false, false, 8, 8192 };
        r.gpu = { true, true, "Qualcomm Adreno 690", 0, true };
        r.comRegistration = { "9E6ECB90-5A61-42BD-B851-D3297D9C7F39", true,
            "C:\\Windows\\System32\\LENSShell.dll", true };
        r.decoderTests = {
            { ".jpg", "ImageDecoder", DecoderConfidenceLevel::FullPass, 12.0, "" },
            { ".png", "ImageDecoder", DecoderConfidenceLevel::FullPass, 9.0, "" },
            { ".webp", "WebPDecoder", DecoderConfidenceLevel::FullPass, 15.0, "" },
            { ".heic", "HEIFDecoder", DecoderConfidenceLevel::FullPass, 22.0, "" },
            { ".raw", "RAWDecoder", DecoderConfidenceLevel::FullPass, 45.0, "" },
        };
        r.decoderPassCount = 5;
        r.decoderFailCount = 0;
        return r;
    }
};

} // namespace ExplorerLens::Platform

// ═══════════════════════════════════════════════════════════════════════════════
// Section: ARM64 Performance Optimizer  (was ARM64PerformanceOptimizer.h)
// Namespace: ExplorerLens::Engine
// ═══════════════════════════════════════════════════════════════════════════════

namespace ExplorerLens {
namespace Engine {

enum class ARM64SIMDExt : uint8_t { NEON = 0, SVE, SVE2, SME, COUNT };
enum class ARM64CoreType : uint8_t { BigCore = 0, LittleCore, MiddleCore, COUNT };
enum class ARM64ThermalHint : uint8_t { Burst = 0, Sustained, Balanced, Efficiency, Silent, COUNT };

struct ARM64TopologyInfo {
    uint8_t bigCores = 0;
    uint8_t littleCores = 0;
    uint8_t middleCores = 0;
    uint32_t cacheSizeKB = 0; // L2 per-cluster
    bool simd128Bit = false;
    ARM64SIMDExt bestSIMD = ARM64SIMDExt::NEON;
};

struct ARM64DecodeProfile {
    ARM64SIMDExt simdExt = ARM64SIMDExt::NEON;
    ARM64CoreType preferCore = ARM64CoreType::BigCore;
    ARM64ThermalHint thermal = ARM64ThermalHint::Balanced;
    float expectedSpeedup = 1.0f; // vs scalar baseline
};

class ARM64PerformanceOptimizer {
public:
    static const wchar_t* SIMDExtName(ARM64SIMDExt s) {
        switch (s) {
        case ARM64SIMDExt::NEON: return L"NEON (128-bit)";
        case ARM64SIMDExt::SVE: return L"SVE";
        case ARM64SIMDExt::SVE2: return L"SVE2";
        case ARM64SIMDExt::SME: return L"SME";
        default: return L"Scalar";
        }
    }
    static const wchar_t* CoreTypeName(ARM64CoreType c) {
        switch (c) {
        case ARM64CoreType::BigCore: return L"Performance Core";
        case ARM64CoreType::LittleCore: return L"Efficiency Core";
        case ARM64CoreType::MiddleCore: return L"Balance Core";
        default: return L"Unknown";
        }
    }
    static const wchar_t* ThermalHintName(ARM64ThermalHint h) {
        switch (h) {
        case ARM64ThermalHint::Burst: return L"Burst";
        case ARM64ThermalHint::Sustained: return L"Sustained";
        case ARM64ThermalHint::Balanced: return L"Balanced";
        case ARM64ThermalHint::Efficiency: return L"Efficiency";
        case ARM64ThermalHint::Silent: return L"Silent";
        default: return L"Unknown";
        }
    }
    static constexpr size_t SIMDExtCount() { return static_cast<size_t>(ARM64SIMDExt::COUNT); }
    static constexpr size_t CoreTypeCount() { return static_cast<size_t>(ARM64CoreType::COUNT); }
    static constexpr size_t ThermalHintCount() { return static_cast<size_t>(ARM64ThermalHint::COUNT); }
};

}
} // namespace ExplorerLens::Engine

// ═══════════════════════════════════════════════════════════════════════════════
// Section: ARM64 Platform Validator  (was ARM64PlatformValidator.h)
// Namespace: ExplorerLens::Engine
// Depends on ARM64HardwareValidator.h for ARM64Feature enum.
// ═══════════════════════════════════════════════════════════════════════════════

// ARM64Feature enum is defined in ARM64HardwareValidator.h
#include "ARM64HardwareValidator.h"

namespace ExplorerLens {
namespace Engine {

// ARM64Feature is imported from ARM64HardwareValidator.h (uint32_t bitmask)

/// ARM64 validation test category
enum class ARM64TestCategory : uint8_t {
    BasicBoot, // DLL loads on ARM64
    DecoderFunctional, // All decoders produce correct output
    GPURendering, // D3D11/GDI rendering works
    MemoryLayout, // Struct alignment correct
    SIMDPaths, // NEON code paths active
    Endianness, // File format byte order handling
    Performance, // Meets baseline perf targets
    COMRegistration, // Shell extension registers correctly
    COUNT
};

/// ARM64 validation config
struct ARM64ValidationConfig {
    bool isEmulated = false; // Running under x64 emulation
    bool isNativeARM64 = false; // True ARM64 hardware
    uint32_t coreCount = 0;
    uint64_t systemMemory = 0;
    std::wstring processorName;
    std::wstring osVersion;
};

/// ARM64 platform validation result (distinct from
/// ARM64HardwareValidator::ARM64ValidationResult)
struct ARM64PlatValidationResult {
    ARM64TestCategory category;
    bool passed = false;
    std::wstring detail;
    double perfMs = 0;
};

/// ARM64 platform validator
class ARM64PlatformValidator {
public:
    /// Feature name
    static const wchar_t* FeatureName(ARM64Feature f) {
        switch (f) {
        case ARM64Feature::NEON:
            return L"NEON";
        case ARM64Feature::CRC32:
            return L"CRC32";
        case ARM64Feature::AES:
            return L"AES";
        case ARM64Feature::SHA1:
            return L"SHA-1";
        case ARM64Feature::SHA256:
            return L"SHA-256";
        case ARM64Feature::LSE:
            return L"LSE Atomics";
        case ARM64Feature::FP16:
            return L"FP16";
        case ARM64Feature::DotProd:
            return L"Dot Product";
        case ARM64Feature::SVE:
            return L"SVE";
        default:
            return L"Unknown";
        }
    }

    /// Test category name
    static const wchar_t* CategoryName(ARM64TestCategory c) {
        switch (c) {
        case ARM64TestCategory::BasicBoot:
            return L"Basic Boot";
        case ARM64TestCategory::DecoderFunctional:
            return L"Decoder Functional";
        case ARM64TestCategory::GPURendering:
            return L"GPU Rendering";
        case ARM64TestCategory::MemoryLayout:
            return L"Memory Layout";
        case ARM64TestCategory::SIMDPaths:
            return L"SIMD Paths";
        case ARM64TestCategory::Endianness:
            return L"Endianness";
        case ARM64TestCategory::Performance:
            return L"Performance";
        case ARM64TestCategory::COMRegistration:
            return L"COM Registration";
        default:
            return L"Unknown";
        }
    }

    /// Feature count (matches ARM64HardwareValidator.h enum values)
    static constexpr size_t FeatureCount() { return 9; } // NEON..FLAGM

    /// Test category count
    static constexpr size_t CategoryCount() {
        return static_cast<size_t>(ARM64TestCategory::COUNT);
    }

    /// Check if running on ARM64
    static bool IsARM64() {
#if defined(_M_ARM64) || defined(__aarch64__)
        return true;
#else
        return false;
#endif
    }
};

} // namespace Engine
} // namespace ExplorerLens
