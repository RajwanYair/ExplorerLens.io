// CLIPQueryProcessor.cpp — Natural language to CLIP embedding query processor
// Copyright (c) 2026 ExplorerLens Project

#include "CLIPQueryProcessor.h"

#include <string>
#include <string_view>
#include <vector>

namespace ExplorerLens { namespace Engine {

CLIPQueryProcessor& CLIPQueryProcessor::Instance()
{
    static CLIPQueryProcessor instance;
    return instance;
}

bool CLIPQueryProcessor::LoadModel(const std::wstring& modelPath)
{
    m_modelLoaded = !modelPath.empty();
    return m_modelLoaded;
}

std::vector<CLIPQueryMatch> CLIPQueryProcessor::Query(const CLIPQueryRequest& req) const
{
    // Stub: return an empty result set when model is not loaded;
    // real implementation dispatches to DirectML/ONNX text encoder.
    if (!m_modelLoaded || req.queryText.empty()) { return {}; }

    m_lastEmbedMs = 18.0f;  // simulated INT8 NPU embed latency
    std::vector<CLIPQueryMatch> matches;
    // No actual HNSW query in stub — return placeholder
    return matches;
}

std::string_view CLIPQueryProcessor::BackendName(CLIPTextBackend b) noexcept
{
    switch (b)
    {
        case CLIPTextBackend::DIRECTML: return "DirectML";
        case CLIPTextBackend::ONNX:     return "ONNX";
        default:                        return "CPU";
    }
}

}} // namespace ExplorerLens::Engine
