//==============================================================================
// DarkThumbs Engine — Sprint 309: Release Gate V24
// Format Intelligence phase gate — validates smart detection, extended video,
// audio visualization, and 3D renderer V2 KPIs.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class GateV24KPI : uint32_t {
    BuildClean = 0, TestsPass, ZeroWarnings,
    SmartFormatDetectorV2, ExtendedVideoDecoder,
    AudioVisualizationV2, Model3DRendererV2,
    DetectionAccuracy, VideoHWAccel, AudioExtraction,
    Model3DPBRQuality, FormatCoverage,
    COUNT
};

struct GateV24Result { GateV24KPI kpi; bool passed = false; std::wstring detail; };
struct GateV24Verdict {
    bool approved = false; uint32_t passed = 0; uint32_t failed = 0;
    std::wstring version = L"14.0.0";
    std::wstring milestone = L"v14.0 P2 - Format Intelligence";
};

class ReleaseGateV24 {
public:
    static const wchar_t* KPIName(GateV24KPI k) {
        switch (k) {
            case GateV24KPI::BuildClean:           return L"Build Clean";
            case GateV24KPI::TestsPass:            return L"Tests Pass";
            case GateV24KPI::ZeroWarnings:         return L"Zero Warnings";
            case GateV24KPI::SmartFormatDetectorV2:return L"Smart Format Detector V2";
            case GateV24KPI::ExtendedVideoDecoder: return L"Extended Video Decoder";
            case GateV24KPI::AudioVisualizationV2: return L"Audio Visualization V2";
            case GateV24KPI::Model3DRendererV2:    return L"3D Model Renderer V2";
            case GateV24KPI::DetectionAccuracy:    return L"Detection Accuracy";
            case GateV24KPI::VideoHWAccel:         return L"Video HW Accel";
            case GateV24KPI::AudioExtraction:      return L"Audio Extraction";
            case GateV24KPI::Model3DPBRQuality:    return L"3D PBR Quality";
            case GateV24KPI::FormatCoverage:       return L"Format Coverage";
            default: return L"Unknown";
        }
    }
    static constexpr size_t KPICount() { return static_cast<size_t>(GateV24KPI::COUNT); }
    static GateV24Verdict Evaluate(const std::vector<GateV24Result>& r) {
        GateV24Verdict v;
        for (const auto& x : r) { if (x.passed) v.passed++; else v.failed++; }
        v.approved = (v.failed == 0 && v.passed == KPICount());
        return v;
    }
};

}} // namespace DarkThumbs::Engine
