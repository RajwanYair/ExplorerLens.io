// ContentAwareInpainter.cpp — Content-Aware Inpainting Engine
// Copyright (c) 2026 ExplorerLens Project
//
#include "ContentAwareInpainter.h"

namespace ExplorerLens::Engine {

bool ContentAwareInpainter::Inpaint(const void*, uint32_t, uint32_t, const InpaintRegion&)
{
    return false;
}
void ContentAwareInpainter::SetQuality(InpaintQuality) {}
uint32_t ContentAwareInpainter::GetEstimatedDurationMs(const InpaintRegion&) const
{
    return 0;
}
bool ContentAwareInpainter::IsProcessing() const
{
    return false;
}
void ContentAwareInpainter::Cancel() {}

}  // namespace ExplorerLens::Engine
