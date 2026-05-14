// DiffusionModelEngine.cpp — On-device diffusion model inference engine
// Copyright (c) 2026 ExplorerLens Project
//
#include "DiffusionModelEngine.h"

namespace ExplorerLens { namespace Engine {

DiffusionModelEngine DiffusionModelEngine::s_instance;

DiffusionModelEngine::DiffusionModelEngine()  = default;
DiffusionModelEngine::~DiffusionModelEngine() { UnloadModel(); }

DiffusionModelEngine& DiffusionModelEngine::Instance() noexcept { return s_instance; }

bool DiffusionModelEngine::LoadModel(const DiffusionModelConfig& config)
{
    m_state     = DiffusionModelState::Loading;
    m_modelName = config.modelPath.empty() ? "built-in-stub" : config.modelPath;
    m_state     = DiffusionModelState::Ready;
    return true;
}

void DiffusionModelEngine::UnloadModel()
{
    m_state     = DiffusionModelState::Unloaded;
    m_modelName.clear();
}

LatentVector DiffusionModelEngine::EncodePrompt(const std::string& prompt) const
{
    LatentVector lv;
    if (prompt.empty() || m_state != DiffusionModelState::Ready)
        return lv;
    lv.width    = 32;
    lv.height   = 32;
    lv.channels = 4;
    lv.data.assign(static_cast<size_t>(lv.width * lv.height * lv.channels), 0.5f);
    return lv;
}

std::vector<uint8_t> DiffusionModelEngine::Decode(const LatentVector& latent,
                                                    uint32_t width, uint32_t height) const
{
    if (latent.data.empty())
        return {};
    std::vector<uint8_t> pixels(static_cast<size_t>(width * height * 4), 0xCC);
    return pixels;
}

}} // namespace ExplorerLens::Engine
