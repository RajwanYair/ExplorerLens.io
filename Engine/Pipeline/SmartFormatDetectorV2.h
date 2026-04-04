//==============================================================================
// ExplorerLens Engine — Smart Format Detector V2
// Signature-based, ML-assisted format detection with confidence scoring,
// ambiguous-format disambiguation, and per-decoder capability scoring.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Detection method
enum class DetectionMethod : uint8_t {
    Extension = 0,  // Fast path: file extension
    MagicBytes,     // Byte signature (MIME sniff)
    DeepScan,       // Full header parsing
    MLClassifier,   // ML-based classification
    COUNT
};

/// Detection confidence
enum class DetectionConfidence : uint8_t {
    None = 0,
    Low,      // < 50%
    Medium,   // 50-80%
    High,     // 80-95%
    Certain,  // > 95%
    COUNT
};

/// Detection hint for disambiguation
enum class DetectionHint : uint8_t {
    None = 0,
    PreferFast,                  // Prefer speed over accuracy
    PreferAccurate,              // Deep scan mandatory
    TrustExtension,              // Extension is authoritative
    Extension = TrustExtension,  // compat alias
    COUNT = TrustExtension + 1
};

/// Format detection result
struct FormatDetectionV2Result
{
    std::wstring detectedFormat;
    DetectionMethod method = DetectionMethod::Extension;
    DetectionConfidence confidence = DetectionConfidence::None;
    float score = 0.0f;  // 0.0-1.0
    bool ambiguous = false;
};

/// Smart format detector V2
class SmartFormatDetectorV2
{
  public:
    static const wchar_t* MethodName(DetectionMethod m)
    {
        switch (m) {
            case DetectionMethod::Extension:
                return L"Extension";
            case DetectionMethod::MagicBytes:
                return L"Magic Bytes";
            case DetectionMethod::DeepScan:
                return L"Deep Scan";
            case DetectionMethod::MLClassifier:
                return L"ML Classifier";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* ConfidenceName(DetectionConfidence c)
    {
        switch (c) {
            case DetectionConfidence::None:
                return L"None";
            case DetectionConfidence::Low:
                return L"Low (<50%)";
            case DetectionConfidence::Medium:
                return L"Medium (50-80%)";
            case DetectionConfidence::High:
                return L"High (80-95%)";
            case DetectionConfidence::Certain:
                return L"Certain (>95%)";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* HintName(DetectionHint h)
    {
        switch (h) {
            case DetectionHint::None:
                return L"None";
            case DetectionHint::PreferFast:
                return L"Prefer Fast";
            case DetectionHint::PreferAccurate:
                return L"Prefer Accurate";
            case DetectionHint::TrustExtension:
                return L"Trust Extension";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t MethodCount()
    {
        return static_cast<size_t>(DetectionMethod::COUNT);
    }
    static constexpr size_t ConfidenceCount()
    {
        return static_cast<size_t>(DetectionConfidence::COUNT);
    }
    static constexpr size_t HintCount()
    {
        return static_cast<size_t>(DetectionHint::COUNT);
    }

    static bool IsTrusted(const FormatDetectionV2Result& r)
    {
        return r.confidence >= DetectionConfidence::High && r.score >= 0.8f;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
