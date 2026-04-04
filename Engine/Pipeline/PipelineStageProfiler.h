// PipelineStageProfiler.h — Per-Stage Pipeline Performance Profiler
// Copyright (c) 2026 ExplorerLens Project
//
// Instruments each stage of the thumbnail pipeline (detect, decode, resize,
// encode, cache) with nanosecond-precision timing. Identifies bottlenecks
// and generates flame-graph-compatible output.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PSProfileStage : uint8_t {
    FileOpen,
    FormatDetect,
    HeaderParse,
    Decode,
    ColorConvert,
    Resize,
    OverlayRender,
    Encode,
    CacheLookup,
    CacheStore,
    GPUUpload,
    GPUCompute,
    COUNT
};

struct ProfileStageTiming
{
    PSProfileStage stage = PSProfileStage::FileOpen;
    double startUs = 0.0;
    double durationUs = 0.0;
    uint64_t bytesIn = 0;
    uint64_t bytesOut = 0;
};

struct PipelineProfile
{
    std::vector<ProfileStageTiming> stages;
    double totalUs = 0.0;
    PSProfileStage bottleneck = PSProfileStage::Decode;
    double bottleneckUs = 0.0;
    bool cacheHit = false;
};

class PipelineStageProfiler
{
  public:
    void BeginProfile()
    {
        m_profile = {};
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    void BeginStage(PSProfileStage stage)
    {
        ProfileStageTiming t;
        t.stage = stage;
        auto now = std::chrono::high_resolution_clock::now();
        t.startUs = std::chrono::duration<double, std::micro>(now - m_startTime).count();
        m_current = t;
    }

    void EndStage(uint64_t bytesIn = 0, uint64_t bytesOut = 0)
    {
        auto now = std::chrono::high_resolution_clock::now();
        m_current.durationUs = std::chrono::duration<double, std::micro>(now - m_startTime).count() - m_current.startUs;
        m_current.bytesIn = bytesIn;
        m_current.bytesOut = bytesOut;
        m_profile.stages.push_back(m_current);
    }

    PipelineProfile EndProfile()
    {
        auto now = std::chrono::high_resolution_clock::now();
        m_profile.totalUs = std::chrono::duration<double, std::micro>(now - m_startTime).count();
        // Find bottleneck
        for (auto& s : m_profile.stages) {
            if (s.durationUs > m_profile.bottleneckUs) {
                m_profile.bottleneckUs = s.durationUs;
                m_profile.bottleneck = s.stage;
            }
        }
        return m_profile;
    }

    static const wchar_t* StageName(PSProfileStage s)
    {
        switch (s) {
            case PSProfileStage::FileOpen:
                return L"FileOpen";
            case PSProfileStage::FormatDetect:
                return L"FormatDetect";
            case PSProfileStage::HeaderParse:
                return L"HeaderParse";
            case PSProfileStage::Decode:
                return L"Decode";
            case PSProfileStage::ColorConvert:
                return L"ColorConvert";
            case PSProfileStage::Resize:
                return L"Resize";
            case PSProfileStage::OverlayRender:
                return L"OverlayRender";
            case PSProfileStage::Encode:
                return L"Encode";
            case PSProfileStage::CacheLookup:
                return L"CacheLookup";
            case PSProfileStage::CacheStore:
                return L"CacheStore";
            case PSProfileStage::GPUUpload:
                return L"GPUUpload";
            case PSProfileStage::GPUCompute:
                return L"GPUCompute";
            default:
                return L"Unknown";
        }
    }
    static size_t StageCount()
    {
        return static_cast<size_t>(PSProfileStage::COUNT);
    }

  private:
    PipelineProfile m_profile;
    ProfileStageTiming m_current;
    std::chrono::high_resolution_clock::time_point m_startTime;
};

}  // namespace Engine
}  // namespace ExplorerLens
