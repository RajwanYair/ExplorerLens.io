// StyleTransferRenderer.cpp — AI Style Transfer Renderer
// Copyright (c) 2026 ExplorerLens Project
//
#include "StyleTransferRenderer.h"

namespace ExplorerLens::Engine {

bool StyleTransferRenderer::ApplyStyle(void*, uint32_t, uint32_t, const StyleParams&)
{
    return false;
}
bool StyleTransferRenderer::LoadStyleModel(const std::string&)
{
    return false;
}
void StyleTransferRenderer::UnloadStyleModel() {}
std::vector<ArtisticStyle> StyleTransferRenderer::GetAvailableStyles() const
{
    return {};
}
std::optional<ArtisticStyle> StyleTransferRenderer::GetLastAppliedStyle() const
{
    return {};
}

}  // namespace ExplorerLens::Engine
