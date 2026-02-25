// FuzzingCampaign.h — Fuzzing Campaign Configuration
// Copyright (c) 2026 ExplorerLens Project
//
// Configures automated fuzzing campaigns for decoder robustness testing.
// Targets: archive decoders, image decoders, font parsers, and 3D model
// loaders. Integrates with libFuzzer (via MSVC) and WinAFL.

#pragma once

#include <cstdint>
#include <string>

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
  WinAFL = 2,    ///< Windows AFL-based fuzzer
  OneFuzz = 3,   ///< Microsoft OneFuzz platform
  Jazzer = 4     ///< Coverage-guided (Java/native)
};

/// Fuzzing target category
enum class FuzzTarget : uint8_t {
  ArchiveParser = 0, ///< ZIP/7Z/RAR header parsing
  ImageDecoder = 1,  ///< WebP/AVIF/JXL/HEIF decode
  FontParser = 2,    ///< TTF/OTF table parsing
  PDFParser = 3,     ///< PDF cross-ref/object parsing
  ModelLoader = 4,   ///< STL/OBJ/glTF parsing
  COMInterface = 5,  ///< IExtractImage/IThumbnailProvider
  TargetCount = 6
};

/// Fuzzing campaign configuration
struct FuzzCampaignConfig {
  FuzzTarget target = FuzzTarget::ArchiveParser;
  FuzzingEngine engine = FuzzingEngine::LibFuzzer;
  uint32_t maxTotalTimeSec = 3600;      ///< 1 hour default
  uint32_t maxInputSizeBytes = 1048576; ///< 1MB max input
  uint32_t maxIterations = 0;           ///< 0 = unlimited
  uint32_t parallelJobs = 4;            ///< Parallel fuzzer instances
  const char *corpusDir = nullptr;      ///< Seed corpus directory
  const char *artifactDir = nullptr;    ///< Crash artifact output
  bool enableASAN = true;               ///< Address sanitizer
  bool enableUBSAN = false;             ///< Undefined behavior sanitizer
  bool enableCoverage = true;           ///< Track coverage feedback
  bool minimizeCrashes = true;          ///< Minimize crash inputs
};

/// Crash artifact
struct FuzzCrashArtifact {
  const char *inputFile = nullptr;  ///< Path to crash-triggering input
  const char *stackTrace = nullptr; ///< Stack trace at crash
  const char *crashType = nullptr;  ///< e.g., "heap-buffer-overflow"
  FuzzTarget target = FuzzTarget::ArchiveParser;
  uint32_t inputSizeBytes = 0;
  bool isMinimized = false;
  bool isTriaged = false;
  uint8_t severity = 0; ///< 0=info, 1=low, 2=med, 3=high, 4=critical
};

/// Fuzzing campaign manager
class FuzzingCampaign {
public:
  static FuzzingCampaign &Instance() {
    static FuzzingCampaign inst;
    return inst;
  }

  /// Get MSVC compile flags for fuzzer instrumentation
  static const char *GetLibFuzzerCompileFlags() {
    return "/fsanitize=address " // ASan
           "/Zi "                // Debug info
           "/Od "                // No optimization for better crash traces
           "/DFUZZING_BUILD";    // Preprocessor flag
  }

  /// Get target function name for each fuzz target
  static const char *GetTargetFunction(FuzzTarget t) {
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
  static const char *TargetName(FuzzTarget t) {
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
  std::string GetWinAFLCommand(const FuzzCampaignConfig &config,
                               const char *targetExe) const {
    std::string cmd = "afl-fuzz.exe";
    cmd += " -i " + std::string(config.corpusDir ? config.corpusDir : "corpus");
    cmd += " -o " +
           std::string(config.artifactDir ? config.artifactDir : "findings");
    cmd += " -t 5000+";  // 5s timeout
    cmd += " -M master"; // Master mode

    // Target module + function
    cmd += " -D \"C:\\DynamoRIO\\bin64\"";
    cmd += " -- ";
    cmd += targetExe;
    cmd += " @@"; // Input file placeholder
    return cmd;
  }

  /// Register crash artifacts
  void RecordCrash(const FuzzCrashArtifact &artifact) {
    if (m_crashCount < MAX_CRASHES) {
      m_crashes[m_crashCount++] = artifact;
    }
  }

  const FuzzCrashArtifact *GetCrashes() const { return m_crashes; }
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
  static const wchar_t *StrategyName(FuzzStrategy s) {
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
