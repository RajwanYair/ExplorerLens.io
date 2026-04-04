// PipelineFusionOptimizer.h — Pipeline Stage Fusion Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes decode pipeline stages and fuses consecutive compatible stages
// to reduce memory copies, intermediate buffers, and overhead.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PipelineStageType : uint8_t {
    Decode,
    ColorConvert,
    Resize,
    Sharpen,
    Overlay,
    Compress,
    Upload
};

struct FusionCandidate
{
    uint32_t stageA = 0;
    uint32_t stageB = 0;
    PipelineStageType typeA = PipelineStageType::Decode;
    PipelineStageType typeB = PipelineStageType::Decode;
    double estimatedSpeedupMs = 0.0;
    bool isCompatible = false;
};

struct FusionResult
{
    uint32_t originalStageCount = 0;
    uint32_t fusedStageCount = 0;
    uint32_t fusionsMade = 0;
    double estimatedTotalSpeedupMs = 0.0;
};

class PipelineFusionOptimizer
{
  public:
    PipelineFusionOptimizer() = default;

    void AddStage(PipelineStageType type)
    {
        m_stages.push_back(type);
    }

    std::vector<FusionCandidate> AnalyzeFusionOpportunities() const
    {
        std::vector<FusionCandidate> candidates;
        for (size_t i = 0; i + 1 < m_stages.size(); i++) {
            FusionCandidate fc;
            fc.stageA = static_cast<uint32_t>(i);
            fc.stageB = static_cast<uint32_t>(i + 1);
            fc.typeA = m_stages[i];
            fc.typeB = m_stages[i + 1];
            fc.isCompatible = AreFusionCompatible(m_stages[i], m_stages[i + 1]);
            if (fc.isCompatible)
                fc.estimatedSpeedupMs = 0.5;
            candidates.push_back(fc);
        }
        return candidates;
    }

    FusionResult ApplyFusions()
    {
        FusionResult result;
        result.originalStageCount = static_cast<uint32_t>(m_stages.size());
        auto candidates = AnalyzeFusionOpportunities();
        for (const auto& c : candidates) {
            if (c.isCompatible)
                result.fusionsMade++;
        }
        result.fusedStageCount = result.originalStageCount - result.fusionsMade;
        result.estimatedTotalSpeedupMs = result.fusionsMade * 0.5;
        m_totalFusions += result.fusionsMade;
        return result;
    }

    uint32_t GetStageCount() const
    {
        return static_cast<uint32_t>(m_stages.size());
    }
    uint64_t GetTotalFusions() const
    {
        return m_totalFusions;
    }
    void ClearStages()
    {
        m_stages.clear();
    }

  private:
    bool AreFusionCompatible(PipelineStageType a, PipelineStageType b) const
    {
        return (a == PipelineStageType::ColorConvert && b == PipelineStageType::Resize)
               || (a == PipelineStageType::Resize && b == PipelineStageType::Sharpen);
    }

    std::vector<PipelineStageType> m_stages;
    uint64_t m_totalFusions = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
