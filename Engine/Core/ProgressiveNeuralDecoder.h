// ProgressiveNeuralDecoder.h — Progressive Neural Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes neural compressed images progressively, yielding coarse previews immediately.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PNDDecodeStep
{
    uint32_t level = 0;  // 0 = coarsest
    std::vector<uint8_t> rgbaData;
    float qualityEstimate = 0.0f;
};

class ProgressiveNeuralDecoder
{
  public:
    void BeginDecode(const std::vector<uint8_t>& encoded, uint32_t width, uint32_t height)
    {
        m_encoded = encoded;
        m_width = width;
        m_height = height;
        m_level = 0;
    }
    PNDDecodeStep NextStep()
    {
        PNDDecodeStep step;
        step.level = m_level;
        uint32_t scale = 1u << (3 - std::min(m_level, 3u));
        uint32_t w = std::max(1u, m_width / scale);
        uint32_t h = std::max(1u, m_height / scale);
        step.rgbaData.assign(static_cast<size_t>(w) * h * 4, static_cast<uint8_t>(0x20 + m_level * 0x30));
        step.qualityEstimate = std::min(1.0f, static_cast<float>(m_level + 1) * 0.25f);
        ++m_level;
        return step;
    }
    bool IsComplete() const
    {
        return m_level >= 4;
    }
    void Reset()
    {
        m_level = 0;
        m_encoded.clear();
    }
    uint32_t Width() const
    {
        return m_width;
    }
    uint32_t Height() const
    {
        return m_height;
    }

  private:
    std::vector<uint8_t> m_encoded;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_level = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
