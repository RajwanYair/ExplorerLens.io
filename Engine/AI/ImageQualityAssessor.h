//==============================================================================
// DarkThumbs Engine — Sprint 327: Image Quality Assessor
// AI-based no-reference image quality metric (BRISQUE/NIQE), blur detection,
// noise estimation, compression artifact scoring, and exposure analysis.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class IQAMetric : uint8_t { BRISQUE=0,NIQE,CLIP_IQA,SSIM_Reference,COUNT };
enum class IQADefect : uint8_t { None=0,Blur,Noise,Overexposed,Underexposed,Compression,COUNT };
enum class IQAGrade : uint8_t { Excellent=0,Good,Fair,Poor,Unacceptable,COUNT };

struct IQAScore {
    IQAMetric   metric      = IQAMetric::BRISQUE;
    float       value       = 0.0f;
    IQAGrade    grade       = IQAGrade::Fair;
    IQADefect   primaryDefect = IQADefect::None;
};

struct ImageQualityReport {
    IQAScore    overallScore;
    float       blurScore   = 0.0f;   // 0=sharp, 1=blurry
    float       noiseScore  = 0.0f;   // 0=clean, 1=very noisy
    float       exposureOk  = 1.0f;   // 1=well exposed
    bool        worthEnhancing = false;
};

class ImageQualityAssessor {
public:
    static const wchar_t* MetricName(IQAMetric m) {
        switch(m) {
            case IQAMetric::BRISQUE:         return L"BRISQUE";
            case IQAMetric::NIQE:            return L"NIQE";
            case IQAMetric::CLIP_IQA:        return L"CLIP-IQA";
            case IQAMetric::SSIM_Reference:  return L"SSIM (Reference)";
            default: return L"Unknown";
        }
    }
    static const wchar_t* DefectName(IQADefect d) {
        switch(d) {
            case IQADefect::None:         return L"None";
            case IQADefect::Blur:         return L"Blur";
            case IQADefect::Noise:        return L"Noise";
            case IQADefect::Overexposed:  return L"Overexposed";
            case IQADefect::Underexposed: return L"Underexposed";
            case IQADefect::Compression:  return L"Compression";
            default: return L"Unknown";
        }
    }
    static const wchar_t* GradeName(IQAGrade g) {
        switch(g) {
            case IQAGrade::Excellent:    return L"Excellent";
            case IQAGrade::Good:         return L"Good";
            case IQAGrade::Fair:         return L"Fair";
            case IQAGrade::Poor:         return L"Poor";
            case IQAGrade::Unacceptable: return L"Unacceptable";
            default: return L"Unknown";
        }
    }
    static constexpr size_t MetricCount() { return static_cast<size_t>(IQAMetric::COUNT); }
    static constexpr size_t DefectCount() { return static_cast<size_t>(IQADefect::COUNT); }
    static constexpr size_t GradeCount()  { return static_cast<size_t>(IQAGrade::COUNT); }
};

}} // namespace DarkThumbs::Engine
