#pragma once
// Sprint 423: Content Inspection Gateway
// DLP-compatible content inspection for enterprise environments — scans
// thumbnails and source files for sensitive content before display.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Content classification result
enum class ContentClassification : uint8_t {
  Safe = 0,
  InternalOnly,
  Confidential,
  Restricted,
  Blocked,
  COUNT
};

/// Inspection method
enum class InspectionMethod : uint8_t {
  MetadataOnly = 0, // Check EXIF/XMP metadata fields
  HeaderScan,       // Scan file header bytes
  FullContent,      // Full content analysis
  MLClassifier,     // ML-based content classification
  COUNT
};

struct InspectionResult {
  ContentClassification classification = ContentClassification::Safe;
  InspectionMethod method = InspectionMethod::MetadataOnly;
  float confidence = 0.0f;
  bool containsPII = false;
  bool containsWatermark = false;
  double inspectionTimeMs = 0.0;
};

struct InspectionConfig {
  bool enabled = false;
  bool blockOnFailure = true;
  bool logAllInspections = false;
  InspectionMethod method = InspectionMethod::MetadataOnly;
  float blockThreshold = 0.9f;
  uint32_t maxScanSizeBytes = 10 * 1024 * 1024;
};

class ContentInspectionGateway {
public:
  static constexpr size_t ClassificationCount() {
    return static_cast<size_t>(ContentClassification::COUNT);
  }
  static constexpr size_t MethodCount() {
    return static_cast<size_t>(InspectionMethod::COUNT);
  }

  static const wchar_t *ClassificationName(ContentClassification c) {
    switch (c) {
    case ContentClassification::Safe:
      return L"Safe";
    case ContentClassification::InternalOnly:
      return L"Internal Only";
    case ContentClassification::Confidential:
      return L"Confidential";
    case ContentClassification::Restricted:
      return L"Restricted";
    case ContentClassification::Blocked:
      return L"Blocked";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *MethodName(InspectionMethod m) {
    switch (m) {
    case InspectionMethod::MetadataOnly:
      return L"Metadata Only";
    case InspectionMethod::HeaderScan:
      return L"Header Scan";
    case InspectionMethod::FullContent:
      return L"Full Content";
    case InspectionMethod::MLClassifier:
      return L"ML Classifier";
    default:
      return L"Unknown";
    }
  }

  /// Determine if thumbnail should be blocked
  static bool ShouldBlock(const InspectionResult &result,
                          const InspectionConfig &config) {
    if (!config.enabled)
      return false;
    if (result.classification == ContentClassification::Blocked)
      return true;
    if (config.blockOnFailure &&
        result.classification >= ContentClassification::Restricted)
      return true;
    return false;
  }
};

} // namespace Engine
} // namespace ExplorerLens
