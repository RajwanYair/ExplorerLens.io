//==============================================================================
// ExplorerLens Engine — Test Infrastructure Upgrade
// OpenCppCoverage integration, test corpus management, ASAN config.
//==============================================================================
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
// SanitizerMode enum comes from MemorySafetyIntegration.h
#include "MemorySafetyIntegration.h"

namespace ExplorerLens {
namespace Engine {

/// Coverage tool configuration
enum class CoverageTool : uint8_t {
  OpenCppCoverage,
  MSVCProfiler,
  LLVMCov,
  Manual
};

// SanitizerMode is defined in MemorySafetyIntegration.h

/// Test corpus file entry
struct TestCorpusFile {
  std::wstring extension; // e.g. L".webp"
  std::wstring filePath;  // e.g. L"test-corpus/webp/sample.webp"
  uint32_t fileSize = 0;  // bytes
  bool valid = true;      // true = well-formed, false = corrupt test
};

/// Test infrastructure manager
class TestInfrastructure {
public:
  /// Get coverage tool command line
  static std::wstring GetCoverageCommand(CoverageTool tool) {
    switch (tool) {
    case CoverageTool::OpenCppCoverage:
      return L"OpenCppCoverage.exe --sources Engine\\ --modules "
             L"EngineTests.exe -- build\\Tests\\Release\\EngineTests.exe";
    case CoverageTool::MSVCProfiler:
      return L"vsinstr.exe /coverage EngineTests.exe && vsperfmon.exe "
             L"/coverage /output:coverage.coverage";
    case CoverageTool::LLVMCov:
      return L"llvm-profdata merge -output=default.profdata default.profraw && "
             L"llvm-cov report EngineTests.exe";
    default:
      return L"echo Manual coverage check";
    }
  }

  /// Coverage tool name
  static const wchar_t *CoverageToolName(CoverageTool t) {
    switch (t) {
    case CoverageTool::OpenCppCoverage:
      return L"OpenCppCoverage";
    case CoverageTool::MSVCProfiler:
      return L"MSVC Profiler";
    case CoverageTool::LLVMCov:
      return L"LLVM Coverage";
    case CoverageTool::Manual:
      return L"Manual";
    default:
      return L"Unknown";
    }
  }

  /// Sanitizer mode name
  static const wchar_t *SanitizerModeName(SanitizerMode m) {
    switch (m) {
    case SanitizerMode::None:
      return L"None";
    case SanitizerMode::AddressSanitizer:
      return L"ASAN";
    case SanitizerMode::UndefinedBehavior:
      return L"UBSAN";
    case SanitizerMode::ThreadSanitizer:
      return L"TSAN";
    case SanitizerMode::MemorySanitizer:
      return L"MSAN";
    default:
      return L"Unknown";
    }
  }

  /// MSVC compiler flags for sanitizer
  static std::wstring GetSanitizerFlags(SanitizerMode m) {
    switch (m) {
    case SanitizerMode::AddressSanitizer:
      return L"/fsanitize=address";
    default:
      return L"";
    }
  }

  /// Register test corpus file
  void AddCorpusFile(const TestCorpusFile &f) { m_corpus.push_back(f); }

  /// Get corpus files for an extension
  std::vector<TestCorpusFile> GetCorpusFor(const std::wstring &ext) const {
    std::vector<TestCorpusFile> result;
    for (auto &f : m_corpus) {
      if (f.extension == ext)
        result.push_back(f);
    }
    return result;
  }

  /// Count corpus files
  size_t CorpusCount() const { return m_corpus.size(); }

  /// Coverage threshold config
  struct CoverageThresholds {
    float lineCoverage = 70.0f; // minimum %
    float branchCoverage = 50.0f;
    float functionCoverage = 80.0f;
  };

  CoverageThresholds GetCIThresholds() const { return {70.0f, 50.0f, 80.0f}; }
  CoverageThresholds GetReleaseThresholds() const {
    return {85.0f, 65.0f, 90.0f};
  }

  /// Format support status
  struct FormatTestStatus {
    std::wstring extension;
    bool hasCorpusFile = false;
    bool hasUnitTest = false;
    bool hasIntegrationTest = false;
  };

private:
  std::vector<TestCorpusFile> m_corpus;
};

} // namespace Engine
} // namespace ExplorerLens
