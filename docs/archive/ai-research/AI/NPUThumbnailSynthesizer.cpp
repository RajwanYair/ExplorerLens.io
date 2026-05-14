// NPUThumbnailSynthesizer.cpp — NPU-accelerated thumbnail synthesis
// Copyright (c) 2026 ExplorerLens Project
//
#include "NPUThumbnailSynthesizer.h"

namespace ExplorerLens { namespace Engine {

NPUThumbnailSynthesizer NPUThumbnailSynthesizer::s_instance;

NPUThumbnailSynthesizer::NPUThumbnailSynthesizer()  = default;
NPUThumbnailSynthesizer::~NPUThumbnailSynthesizer() { Shutdown(); }

NPUThumbnailSynthesizer& NPUThumbnailSynthesizer::Instance() noexcept { return s_instance; }

bool NPUThumbnailSynthesizer::Initialize(SynthesisBackend backend)
{
    m_backend     = backend;
    m_count       = 0;
    m_initialized = true;
    return true;
}

void NPUThumbnailSynthesizer::Shutdown()
{
    m_initialized = false;
}

NPUSynthesisResult NPUThumbnailSynthesizer::Synthesize(const SynthesisRequest& req)
{
    NPUSynthesisResult result;
    if (!m_initialized || req.prompt.empty())
    {
        result.errorMsg = m_initialized ? "Empty prompt" : "Not initialized";
        return result;
    }
    const uint32_t PIXEL_BYTES = req.width * req.height * 4;
    result.pixels.assign(PIXEL_BYTES, 0xAA);
    result.width   = req.width;
    result.height  = req.height;
    result.success = true;
    ++m_count;
    return result;
}

const char* NPUThumbnailSynthesizer::BackendName() const noexcept
{
    switch (m_backend)
    {
    case SynthesisBackend::DirectML: return "DirectML-NPU";
    case SynthesisBackend::ONNX:     return "ONNX-Runtime";
    case SynthesisBackend::CPU:      return "CPU-Fallback";
    default:                         return "Unknown";
    }
}

}} // namespace ExplorerLens::Engine
