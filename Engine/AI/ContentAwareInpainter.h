// ContentAwareInpainter.h — Neural Content-Aware Inpainting
// Copyright (c) 2026 ExplorerLens Project
//
// Fills missing or corrupted thumbnail regions using content-aware inpainting
// backed by neural network models including PatchMatch, DeepFill, LaMa, and Stable Diffusion.
//
#pragma once

#include <cstdint>
#include <optional>

namespace ExplorerLens {
namespace Engine {

enum class InpaintAlgorithm : uint8_t
{
    PatchMatch,
    DeepFill,
    LaMa,
    Stable
};

enum class InpaintQuality : uint8_t
{
    Draft,
    Standard,
    High,
    Ultra
};

struct InpaintRegion
{
    uint32_t        x          = 0;
    uint32_t        y          = 0;
    uint32_t        width      = 0;
    uint32_t        height     = 0;
    InpaintAlgorithm algorithm = InpaintAlgorithm::LaMa;
    InpaintQuality  quality    = InpaintQuality::Standard;
};

class ContentAwareInpainter
{
public:
    ContentAwareInpainter() = default;
    ~ContentAwareInpainter() = default;

    ContentAwareInpainter(ContentAwareInpainter const&)            = delete;
    ContentAwareInpainter& operator=(ContentAwareInpainter const&) = delete;
    ContentAwareInpainter(ContentAwareInpainter&&)                 = default;
    ContentAwareInpainter& operator=(ContentAwareInpainter&&)      = default;

    bool Inpaint(
        void const*           imageData,
        uint32_t              width,
        uint32_t              height,
        InpaintRegion const&  region);

    void SetQuality(InpaintQuality quality);

    [[nodiscard]] uint32_t GetEstimatedDurationMs(InpaintRegion const& region) const;
    [[nodiscard]] bool     IsProcessing() const;

    void Cancel();

private:
    InpaintQuality              m_quality          = InpaintQuality::Standard;
    bool                        m_cancelRequested  = false;
    std::optional<InpaintRegion> m_processingRegion;
};

} // namespace Engine
} // namespace ExplorerLens
