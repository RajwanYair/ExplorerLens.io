// GenerativeThumbnailEngine.cpp — Generative Thumbnail Engine
// Copyright (c) 2026 ExplorerLens Project
//
#include "GenerativeThumbnailEngine.h"

namespace ExplorerLens::Engine {

bool GenerativeThumbnailEngine::Generate(const GenerationRequest&)
{
    return false;
}
void GenerativeThumbnailEngine::SetBackend(GenerativeBackend) {}
std::vector<GenerationMode> GenerativeThumbnailEngine::GetSupportedModes() const
{
    return {};
}
bool GenerativeThumbnailEngine::IsAcceleratorAvailable(GenerativeBackend) const
{
    return false;
}
void GenerativeThumbnailEngine::Shutdown() {}

}  // namespace ExplorerLens::Engine
